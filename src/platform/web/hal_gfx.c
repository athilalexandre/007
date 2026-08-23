#include "hal_gfx.h"
#include <PR/gbi.h>
#include "gbi_extension.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define MAX_MODELVIEW_STACK 32
#define VERTEX_CACHE_SIZE 64
#define MAX_DL_DEPTH 32

/* Real Fast3D / GoldenEye GBI command opcodes */
#define OP_SPNOOP            0x00
#define OP_MTX               0x01
#define OP_MOVEMEM           0x03
#define OP_VTX               0x04
#define OP_DL                0x06

#define OP_TRI4              0xB1
#define OP_RDPHALF_CONT      0xB2
#define OP_RDPHALF_2         0xB3
#define OP_RDPHALF_1         0xB4
#define OP_LINE3D            0xB5
#define OP_CLEARGEOMETRYMODE 0xB6
#define OP_SETGEOMETRYMODE   0xB7
#define OP_ENDDL             0xB8
#define OP_SETOTHERMODE_L    0xB9
#define OP_SETOTHERMODE_H    0xBA
#define OP_TEXTURE           0xBB
#define OP_MOVEWORD          0xBC
#define OP_POPMTX            0xBD
#define OP_CULLDL            0xBE
#define OP_TRI1              0xBF

#define OP_SETTEX            0xC0

#define OP_TEXRECT           0xE4
#define OP_TEXRECTFLIP       0xE5
#define OP_RDPLOADSYNC       0xE6
#define OP_RDPPIPESYNC       0xE7
#define OP_RDPTILESYNC       0xE8
#define OP_RDPFULLSYNC       0xE9
#define OP_SETKEYGB          0xEA
#define OP_SETKEYR           0xEB
#define OP_SETCONVERT        0xEC
#define OP_SETSCISSOR        0xED
#define OP_SETPRIMDEPTH      0xEE
#define OP_RDPSETOTHERMODE   0xEF
#define OP_LOADTLUT          0xF0
#define OP_SETTILESIZE       0xF2
#define OP_LOADBLOCK         0xF3
#define OP_LOADTILE          0xF4
#define OP_SETTILE           0xF5
#define OP_FILLRECT          0xF6
#define OP_SETFILLCOLOR      0xF7
#define OP_SETFOGCOLOR       0xF8
#define OP_SETBLENDCOLOR     0xF9
#define OP_SETPRIMCOLOR      0xFA
#define OP_SETENVCOLOR       0xFB
#define OP_SETCOMBINE        0xFC
#define OP_SETTIMG           0xFD
#define OP_SETZIMG           0xFE
#define OP_SETCIMG           0xFF

#define M_COLOR_R(c) (((c) >> 24) & 0xFF)
#define M_COLOR_G(c) (((c) >> 16) & 0xFF)
#define M_COLOR_B(c) (((c) >> 8) & 0xFF)
#define M_COLOR_A(c) ((c) & 0xFF)

static u16 s_ColorBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static u16 s_DepthBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static u8  s_Tmem[4096];
static u32 s_FrameCount = 0;
static HalGfxTelemetry s_Telemetry;

static uintptr_t s_Segments[16];

extern u8 _bssSegmentEnd[];

typedef struct {
    float scale_x, scale_y, scale_z;
    float trans_x, trans_y, trans_z;
} ViewportState;

static ViewportState s_Viewport = {
    SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 32767.0f,
    SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 32767.0f
};

typedef struct {
    float x, y, z, w;
    float screen_x, screen_y, screen_z;
    float u, v;
    u8 r, g, b, a;
    int clipped;
} TransformedVertex;

static TransformedVertex s_VertexCache[VERTEX_CACHE_SIZE];

static float s_ModelviewStack[MAX_MODELVIEW_STACK][4][4];
static int   s_ModelviewTop = 0;
static float s_ProjectionMatrix[4][4];
static float s_MvpMatrix[4][4];
static int   s_MvpDirty = 1;

static u32 s_GeometryMode = 0;
static u32 s_OtherModeH = 0;
static u32 s_OtherModeL = 0;
static u64 s_CombineMode = 0;
static u32 s_PrimColor = 0xFFFFFFFF;
static u32 s_EnvColor = 0xFFFFFFFF;
static u32 s_BlendColor = 0x00000000;
static u32 s_FogColor = 0x00000000;
static u32 s_FillColor = 0x00000000;

static u32 s_ScissorLeft = 0;
static u32 s_ScissorTop = 0;
static u32 s_ScissorRight = SCREEN_WIDTH;
static u32 s_ScissorBottom = SCREEN_HEIGHT;

static int s_TextureEnabled = 1;
static float s_TextureScaleS = 1.0f;
static float s_TextureScaleT = 1.0f;

typedef struct {
    u32 fmt, siz, line, tmem, palette;
    u32 cmt, maskt, shiftt;
    u32 cms, masks, shifts;
    u32 uls, ult, lrs, lrt;
} GbiTile;

static GbiTile s_Tiles[8];
static const u8 *s_CurrentTextureImage = NULL;
static u32 s_TextureImageWidth = 0;

static void mtx_identity(float m[4][4]) {
    memset(m, 0, 16 * sizeof(float));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

static void mtx_mult(float dst[4][4], const float a[4][4], const float b[4][4]) {
    float tmp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            tmp[i][j] = a[i][0] * b[0][j] +
                        a[i][1] * b[1][j] +
                        a[i][2] * b[2][j] +
                        a[i][3] * b[3][j];
        }
    }
    memcpy(dst, tmp, 16 * sizeof(float));
}

static void mtx_fixed_to_float(float dst[4][4], const Mtx *src) {
    const s16 *intPart = (const s16 *)src->m;
    const u16 *fracPart = (const u16 *)&src->m[2][0];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            s32 fixed = (intPart[idx] << 16) | fracPart[idx];
            dst[i][j] = (float)fixed / 65536.0f;
        }
    }
}

static void update_mvp(void) {
    if (s_MvpDirty) {
        mtx_mult(s_MvpMatrix, s_ModelviewStack[s_ModelviewTop], s_ProjectionMatrix);
        s_MvpDirty = 0;
    }
}

static const void *resolve_seg_address(u32 addr) {
    if (addr == 0) return NULL;
    if (addr >= 0x80000000) {
        return (const void *)(uintptr_t)(addr & 0x1FFFFFFF);
    }
    /* If addr is already a valid direct pointer in engine heap/bss, use directly */
    if (addr >= (uintptr_t)_bssSegmentEnd && addr < ((uintptr_t)_bssSegmentEnd + (32 * 1024 * 1024))) {
        return (const void *)(uintptr_t)addr;
    }
    u32 seg = (addr >> 24) & 0x0F;
    u32 off = addr & 0x00FFFFFF;
    if (s_Segments[seg] != 0 && off < 0x00400000) {
        return (const void *)(s_Segments[seg] + off);
    }
    return (const void *)(uintptr_t)addr;
}

static inline u16 rgba32_to_5551(u8 r, u8 g, u8 b, u8 a) {
    return (u16)(((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | (a ? 1 : 0));
}

static inline void sample_texture(const GbiTile *tile, float s, float t, u8 *outR, u8 *outG, u8 *outB, u8 *outA) {
    s_Telemetry.total_textures++;
    int u = (int)s;
    int v = (int)t;

    if (tile->masks > 0) u &= ((1 << tile->masks) - 1);
    else if (u < 0) u = 0;
    if (tile->maskt > 0) v &= ((1 << tile->maskt) - 1);
    else if (v < 0) v = 0;

    u32 tmemOffset = tile->tmem * 8;
    u32 pitch = (tile->line > 0) ? (tile->line * 8) : 64;

    if (tile->fmt == G_IM_FMT_RGBA && tile->siz == G_IM_SIZ_16b) {
        u32 byteIdx = tmemOffset + v * pitch + u * 2;
        if (byteIdx + 1 < 4096) {
            u16 p = (s_Tmem[byteIdx] << 8) | s_Tmem[byteIdx + 1];
            *outR = ((p >> 11) & 0x1F) * 255 / 31;
            *outG = ((p >> 6) & 0x1F) * 255 / 31;
            *outB = ((p >> 1) & 0x1F) * 255 / 31;
            *outA = (p & 1) ? 255 : 0;
            return;
        }
    } else if (tile->fmt == G_IM_FMT_IA && tile->siz == G_IM_SIZ_8b) {
        u32 byteIdx = tmemOffset + v * pitch + u;
        if (byteIdx < 4096) {
            u8 b = s_Tmem[byteIdx];
            u8 intensity = (b >> 4) * 0x11;
            u8 alpha = (b & 0x0F) * 0x11;
            *outR = intensity; *outG = intensity; *outB = intensity; *outA = alpha;
            return;
        }
    } else if (tile->fmt == G_IM_FMT_IA && tile->siz == G_IM_SIZ_4b) {
        u32 byteIdx = tmemOffset + v * pitch + (u >> 1);
        if (byteIdx < 4096) {
            u8 b = s_Tmem[byteIdx];
            u8 nib = (u & 1) ? (b & 0x0F) : (b >> 4);
            u8 intensity = ((nib >> 1) & 7) * 255 / 7;
            u8 alpha = (nib & 1) ? 255 : 0;
            *outR = intensity; *outG = intensity; *outB = intensity; *outA = alpha;
            return;
        }
    } else if (tile->fmt == G_IM_FMT_I && tile->siz == G_IM_SIZ_8b) {
        u32 byteIdx = tmemOffset + v * pitch + u;
        if (byteIdx < 4096) {
            u8 intensity = s_Tmem[byteIdx];
            *outR = intensity; *outG = intensity; *outB = intensity; *outA = intensity;
            return;
        }
    } else if (tile->fmt == G_IM_FMT_I && tile->siz == G_IM_SIZ_4b) {
        u32 byteIdx = tmemOffset + v * pitch + (u >> 1);
        if (byteIdx < 4096) {
            u8 b = s_Tmem[byteIdx];
            u8 nib = (u & 1) ? (b & 0x0F) : (b >> 4);
            u8 intensity = nib * 0x11;
            *outR = intensity; *outG = intensity; *outB = intensity; *outA = intensity;
            return;
        }
    } else if (tile->fmt == G_IM_FMT_CI) {
        u32 idx = 0;
        if (tile->siz == G_IM_SIZ_4b) {
            u32 byteIdx = tmemOffset + v * pitch + (u >> 1);
            if (byteIdx < 4096) {
                u8 b = s_Tmem[byteIdx];
                idx = (u & 1) ? (b & 0x0F) : (b >> 4);
            }
        } else {
            u32 byteIdx = tmemOffset + v * pitch + u;
            if (byteIdx < 4096) idx = s_Tmem[byteIdx];
        }
        u32 palOffset = 2048 + (tile->palette * 16 + idx) * 2;
        if (palOffset + 1 < 4096) {
            u16 p = (s_Tmem[palOffset] << 8) | s_Tmem[palOffset + 1];
            *outR = ((p >> 11) & 0x1F) * 255 / 31;
            *outG = ((p >> 6) & 0x1F) * 255 / 31;
            *outB = ((p >> 1) & 0x1F) * 255 / 31;
            *outA = (p & 1) ? 255 : 0;
            return;
        }
    }

    *outR = 255; *outG = 255; *outB = 255; *outA = 255;
}

static void rasterize_triangle(const TransformedVertex *v0, const TransformedVertex *v1, const TransformedVertex *v2) {
    s_Telemetry.total_triangles++;

    if (v0->clipped && v1->clipped && v2->clipped) return;

    /* Backface culling */
    float cross = (v1->screen_x - v0->screen_x) * (v2->screen_y - v0->screen_y) - (v1->screen_y - v0->screen_y) * (v2->screen_x - v0->screen_x);
    if ((s_GeometryMode & G_CULL_BACK) && cross < 0.0f) return;
    if ((s_GeometryMode & G_CULL_FRONT) && cross > 0.0f) return;

    float fminX = v0->screen_x < v1->screen_x ? (v0->screen_x < v2->screen_x ? v0->screen_x : v2->screen_x) : (v1->screen_x < v2->screen_x ? v1->screen_x : v2->screen_x);
    float fmaxX = v0->screen_x > v1->screen_x ? (v0->screen_x > v2->screen_x ? v0->screen_x : v2->screen_x) : (v1->screen_x > v2->screen_x ? v1->screen_x : v2->screen_x);
    float fminY = v0->screen_y < v1->screen_y ? (v0->screen_y < v2->screen_y ? v0->screen_y : v2->screen_y) : (v1->screen_y < v2->screen_y ? v1->screen_y : v2->screen_y);
    float fmaxY = v0->screen_y > v1->screen_y ? (v0->screen_y > v2->screen_y ? v0->screen_y : v2->screen_y) : (v1->screen_y > v2->screen_y ? v1->screen_y : v2->screen_y);

    int minX = (int)fminX;
    int maxX = (int)fmaxX + 1;
    int minY = (int)fminY;
    int maxY = (int)fmaxY + 1;

    if (minX < (int)s_ScissorLeft) minX = (int)s_ScissorLeft;
    if (maxX > (int)s_ScissorRight) maxX = (int)s_ScissorRight;
    if (minY < (int)s_ScissorTop) minY = (int)s_ScissorTop;
    if (maxY > (int)s_ScissorBottom) maxY = (int)s_ScissorBottom;

    if (minX >= maxX || minY >= maxY) return;

    float denom = (v1->screen_y - v2->screen_y) * (v0->screen_x - v2->screen_x) + (v2->screen_x - v1->screen_x) * (v0->screen_y - v2->screen_y);
    if (__builtin_fabs(denom) < 0.0001f) return;
    float invDenom = 1.0f / denom;

    const GbiTile *tile = &s_Tiles[0];

    for (int y = minY; y < maxY; y++) {
        float py = y + 0.5f;
        for (int x = minX; x < maxX; x++) {
            float px = x + 0.5f;
            float w0 = ((v1->screen_y - v2->screen_y) * (px - v2->screen_x) + (v2->screen_x - v1->screen_x) * (py - v2->screen_y)) * invDenom;
            float w1 = ((v2->screen_y - v0->screen_y) * (px - v2->screen_x) + (v0->screen_x - v2->screen_x) * (py - v2->screen_y)) * invDenom;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                float z = w0 * v0->screen_z + w1 * v1->screen_z + w2 * v2->screen_z;
                u16 zDepth = (z < 0.0f) ? 0 : ((z > 65535.0f) ? 65535 : (u16)z);
                int pixelIdx = y * SCREEN_WIDTH + x;

                if (!(s_GeometryMode & G_ZBUFFER) || zDepth <= s_DepthBuffer[pixelIdx]) {
                    u8 vertR = (u8)(w0 * v0->r + w1 * v1->r + w2 * v2->r);
                    u8 vertG = (u8)(w0 * v0->g + w1 * v1->g + w2 * v2->g);
                    u8 vertB = (u8)(w0 * v0->b + w1 * v1->b + w2 * v2->b);
                    u8 vertA = (u8)(w0 * v0->a + w1 * v1->a + w2 * v2->a);

                    u8 r = vertR, g = vertG, b = vertB, a = vertA;

                    if (s_TextureEnabled && (s_GeometryMode & G_TEXTURE_ENABLE)) {
                        float u = (w0 * v0->u + w1 * v1->u + w2 * v2->u) * s_TextureScaleS;
                        float v = (w0 * v0->v + w1 * v1->v + w2 * v2->v) * s_TextureScaleT;
                        u8 texR, texG, texB, texA;
                        sample_texture(tile, u, v, &texR, &texG, &texB, &texA);

                        r = (u8)(((u32)vertR * (u32)texR) / 255);
                        g = (u8)(((u32)vertG * (u32)texG) / 255);
                        b = (u8)(((u32)vertB * (u32)texB) / 255);
                        a = (u8)(((u32)vertA * (u32)texA) / 255);
                    }

                    if (a > 8) {
                        if (s_GeometryMode & G_ZBUFFER) {
                            s_DepthBuffer[pixelIdx] = zDepth;
                        }
                        s_ColorBuffer[pixelIdx] = rgba32_to_5551(r, g, b, a);
                    }
                }
            }
        }
    }
}

static void draw_fill_rect(int ulx, int uly, int lrx, int lry) {
    ulx >>= 2;
    uly >>= 2;
    lrx >>= 2;
    lry >>= 2;

    if (ulx < (int)s_ScissorLeft) ulx = (int)s_ScissorLeft;
    if (uly < (int)s_ScissorTop) uly = (int)s_ScissorTop;
    if (lrx > (int)s_ScissorRight) lrx = (int)s_ScissorRight;
    if (lry > (int)s_ScissorBottom) lry = (int)s_ScissorBottom;
    if (ulx >= lrx || uly >= lry) return;

    u16 color16 = (u16)(s_FillColor & 0xFFFF);
    for (int y = uly; y < lry && y < SCREEN_HEIGHT; y++) {
        for (int x = ulx; x < lrx && x < SCREEN_WIDTH; x++) {
            s_ColorBuffer[y * SCREEN_WIDTH + x] = color16;
        }
    }
}

static void draw_tex_rect(int ulx, int uly, int lrx, int lry, float s, float t, float dsdx, float dtdy) {
    s_Telemetry.total_textures++;
    if (ulx < (int)s_ScissorLeft) ulx = (int)s_ScissorLeft;
    if (uly < (int)s_ScissorTop) uly = (int)s_ScissorTop;
    if (lrx > (int)s_ScissorRight) lrx = (int)s_ScissorRight;
    if (lry > (int)s_ScissorBottom) lry = (int)s_ScissorBottom;
    if (ulx >= lrx || uly >= lry) return;

    GbiTile *tile = &s_Tiles[0];
    u16 primColor16 = rgba32_to_5551(M_COLOR_R(s_PrimColor), M_COLOR_G(s_PrimColor), M_COLOR_B(s_PrimColor), M_COLOR_A(s_PrimColor));
    if (!primColor16) primColor16 = 0xFFFF;

    float curT = t;
    for (int y = uly; y < lry && y < SCREEN_HEIGHT; y++) {
        float curS = s;
        for (int x = ulx; x < lrx && x < SCREEN_WIDTH; x++) {
            u8 tr, tg, tb, ta;
            sample_texture(tile, curS, curT, &tr, &tg, &tb, &ta);
            if (ta > 0) {
                if (tile->fmt == G_IM_FMT_I || tile->fmt == G_IM_FMT_IA) {
                    s_ColorBuffer[y * SCREEN_WIDTH + x] = primColor16;
                } else {
                    s_ColorBuffer[y * SCREEN_WIDTH + x] = rgba32_to_5551(tr, tg, tb, ta);
                }
            }
            curS += dsdx;
        }
        curT += dtdy;
    }
}

static void execute_dl_internal(const Gfx *dl, const Gfx *end, int depth) {
    if (!dl || depth > MAX_DL_DEPTH) return;
    if ((uintptr_t)dl < 0x1000 || (uintptr_t)dl > 0x10000000) return;

    if ((u32)depth > s_Telemetry.max_dl_depth) {
        s_Telemetry.max_dl_depth = (u32)depth;
    }

    const Gfx *cur = dl;
    while (!end || cur < end) {
        s_Telemetry.total_commands++;
        u32 w0 = cur->words.w0;
        u32 w1 = cur->words.w1;
        u8 opcode = (w0 >> 24) & 0xFF;
        s_Telemetry.commands_by_opcode[opcode]++;

        if (opcode == OP_SPNOOP) {
            /* No-op */
        } else if (opcode == OP_RDPPIPESYNC || opcode == OP_RDPTILESYNC || opcode == OP_RDPLOADSYNC || opcode == OP_RDPFULLSYNC) {
            /* Full RDP pipeline syncs */
        } else if (opcode == OP_SETTIMG) {
            s_CurrentTextureImage = (const u8 *)resolve_seg_address(w1);
            s_TextureImageWidth = (w0 & 0x0FFF) + 1;
        } else if (opcode == OP_SETCOMBINE) {
            s_CombineMode = ((u64)w0 << 32) | w1;
        } else if (opcode == OP_SETENVCOLOR) {
            s_EnvColor = w1;
        } else if (opcode == OP_SETPRIMCOLOR) {
            s_PrimColor = w1;
        } else if (opcode == OP_SETBLENDCOLOR) {
            s_BlendColor = w1;
        } else if (opcode == OP_SETFOGCOLOR) {
            s_FogColor = w1;
        } else if (opcode == OP_SETFILLCOLOR) {
            s_FillColor = w1;
        } else if (opcode == OP_FILLRECT) {
            int lrx = (w0 >> 12) & 0x0FFF;
            int lry = (w0) & 0x0FFF;
            int ulx = (w1 >> 12) & 0x0FFF;
            int uly = (w1) & 0x0FFF;
            draw_fill_rect(ulx, uly, lrx, lry);
        } else if (opcode == OP_SETSCISSOR) {
            s_ScissorLeft = ((w0 >> 12) & 0x0FFF) >> 2;
            s_ScissorTop = (w0 & 0x0FFF) >> 2;
            s_ScissorRight = ((w1 >> 12) & 0x0FFF) >> 2;
            s_ScissorBottom = (w1 & 0x0FFF) >> 2;
            if (s_ScissorRight > SCREEN_WIDTH) s_ScissorRight = SCREEN_WIDTH;
            if (s_ScissorBottom > SCREEN_HEIGHT) s_ScissorBottom = SCREEN_HEIGHT;
        } else if (opcode == OP_SETOTHERMODE_H) {
            s_OtherModeH = w1;
        } else if (opcode == OP_SETOTHERMODE_L) {
            s_OtherModeL = w1;
        } else if (opcode == OP_SETGEOMETRYMODE) {
            s_GeometryMode |= w1;
        } else if (opcode == OP_CLEARGEOMETRYMODE) {
            s_GeometryMode &= ~w1;
        } else if (opcode == OP_TEXTURE) {
            s_TextureEnabled = ((w1 & 0xFFFF) != 0);
            s_TextureScaleS = (float)(w1 >> 16) / 65536.0f;
            s_TextureScaleT = (float)(w1 & 0xFFFF) / 65536.0f;
            if (s_TextureScaleS <= 0.0f) s_TextureScaleS = 1.0f;
            if (s_TextureScaleT <= 0.0f) s_TextureScaleT = 1.0f;
        } else if (opcode == OP_MOVEWORD) {
            u32 offset = (w0 >> 8) & 0xFFFF;
            u32 index = w0 & 0xFF;
            if (index == G_MW_SEGMENT) {
                u32 seg = (offset >> 2) & 0x0F;
                s_Segments[seg] = w1;
            }
        } else if (opcode == OP_MOVEMEM) {
            u32 index = (w0 >> 16) & 0xFF;
            const void *src = resolve_seg_address(w1);
            if (src && index == G_MV_VIEWPORT) {
                const Vp *vp = (const Vp *)src;
                s_Viewport.scale_x = (float)vp->vp.vscale[0] / 4.0f;
                s_Viewport.scale_y = (float)vp->vp.vscale[1] / 4.0f;
                s_Viewport.scale_z = (float)vp->vp.vscale[2] / 4.0f;
                s_Viewport.trans_x = (float)vp->vp.vtrans[0] / 4.0f;
                s_Viewport.trans_y = (float)vp->vp.vtrans[1] / 4.0f;
                s_Viewport.trans_z = (float)vp->vp.vtrans[2] / 4.0f;
            }
        } else if (opcode == OP_SETTILE) {
            u32 idx = (w1 >> 24) & 0x07;
            GbiTile *t = &s_Tiles[idx];
            t->fmt = (w0 >> 21) & 0x07;
            t->siz = (w0 >> 19) & 0x03;
            t->line = (w0 >> 9) & 0x01FF;
            t->tmem = w0 & 0x01FF;
            t->palette = (w1 >> 20) & 0x0F;
            t->cmt = (w1 >> 18) & 0x03;
            t->maskt = (w1 >> 14) & 0x0F;
            t->shiftt = (w1 >> 10) & 0x0F;
            t->cms = (w1 >> 8) & 0x03;
            t->masks = (w1 >> 4) & 0x0F;
            t->shifts = w1 & 0x0F;
        } else if (opcode == OP_SETTILESIZE) {
            u32 idx = (w1 >> 24) & 0x07;
            GbiTile *t = &s_Tiles[idx];
            t->uls = (w0 >> 12) & 0x0FFF;
            t->ult = w0 & 0x0FFF;
            t->lrs = (w1 >> 12) & 0x0FFF;
            t->lrt = w1 & 0x0FFF;
        } else if (opcode == OP_LOADBLOCK) {
            u32 tileIdx = (w1 >> 24) & 0x07;
            u32 tmemOffset = s_Tiles[tileIdx].tmem * 8;
            u32 numDwords = ((w1 >> 12) & 0x0FFF) + 1;
            u32 numBytes = numDwords * 8;
            if (tmemOffset >= 4096) tmemOffset = 0;
            if (tmemOffset + numBytes > 4096) numBytes = 4096 - tmemOffset;
            if (s_CurrentTextureImage && (uintptr_t)s_CurrentTextureImage >= 0x1000) {
                memcpy(s_Tmem + tmemOffset, s_CurrentTextureImage, numBytes);
            }
        } else if (opcode == OP_LOADTILE) {
            u32 tileIdx = (w1 >> 24) & 0x07;
            u32 tmemOffset = s_Tiles[tileIdx].tmem * 8;
            u32 uls = (w0 >> 12) & 0x0FFF;
            u32 ult = w0 & 0x0FFF;
            u32 lrs = (w1 >> 12) & 0x0FFF;
            u32 lrt = w1 & 0x0FFF;
            u32 width = (lrs - uls) / 4 + 1;
            u32 height = (lrt - ult) / 4 + 1;
            u32 bytesToCopy = width * height * 2;
            if (tmemOffset + bytesToCopy > 4096) bytesToCopy = 4096 - tmemOffset;
            if (s_CurrentTextureImage && (uintptr_t)s_CurrentTextureImage >= 0x1000) {
                memcpy(s_Tmem + tmemOffset, s_CurrentTextureImage, bytesToCopy);
            }
        } else if (opcode == OP_LOADTLUT) {
            u32 count = (w1 >> 14) & 0x03FF;
            u32 numBytes = (count + 1) * 2;
            if (numBytes > 512) numBytes = 512;
            if (s_CurrentTextureImage) {
                memcpy(s_Tmem + 2048, s_CurrentTextureImage, numBytes);
            }
        } else if (opcode == OP_TEXRECT || opcode == OP_TEXRECTFLIP) {
            int lrx = ((w0 >> 12) & 0x0FFF) >> 2;
            int lry = (w0 & 0x0FFF) >> 2;
            int ulx = ((w1 >> 12) & 0x0FFF) >> 2;
            int uly = (w1 & 0x0FFF) >> 2;
            cur++;
            float s = 0.0f, t = 0.0f, dsdx = 1.0f, dtdy = 1.0f;
            s = (float)((s16)(cur->words.w1 >> 16)) / 32.0f;
            t = (float)((s16)(cur->words.w1 & 0xFFFF)) / 32.0f;
            cur++;
            dsdx = (float)((s16)(cur->words.w1 >> 16)) / 1024.0f;
            dtdy = (float)((s16)(cur->words.w1 & 0xFFFF)) / 1024.0f;
            draw_tex_rect(ulx, uly, lrx, lry, s, t, dsdx, dtdy);
        } else if (opcode == OP_MTX) {
            const Mtx *srcMtx = (const Mtx *)resolve_seg_address(w1);
            if ((uintptr_t)srcMtx < 0x1000 || (uintptr_t)srcMtx > 0x10000000) srcMtx = NULL;
            if (srcMtx) {
                float mat[4][4];
                mtx_fixed_to_float(mat, srcMtx);
                u32 params = (w0 >> 16) & 0xFF;
                if (params & G_MTX_PROJECTION) {
                    if (params & G_MTX_LOAD) {
                        memcpy(s_ProjectionMatrix, mat, 16 * sizeof(float));
                    } else {
                        mtx_mult(s_ProjectionMatrix, s_ProjectionMatrix, mat);
                    }
                } else {
                    if (params & G_MTX_PUSH) {
                        if (s_ModelviewTop < MAX_MODELVIEW_STACK - 1) {
                            s_ModelviewTop++;
                            memcpy(s_ModelviewStack[s_ModelviewTop], s_ModelviewStack[s_ModelviewTop - 1], 16 * sizeof(float));
                        }
                    }
                    if (params & G_MTX_LOAD) {
                        memcpy(s_ModelviewStack[s_ModelviewTop], mat, 16 * sizeof(float));
                    } else {
                        mtx_mult(s_ModelviewStack[s_ModelviewTop], s_ModelviewStack[s_ModelviewTop], mat);
                    }
                }
                s_MvpDirty = 1;
            }
        } else if (opcode == OP_POPMTX) {
            if (s_ModelviewTop > 0) s_ModelviewTop--;
            s_MvpDirty = 1;
        } else if (opcode == OP_VTX) {
            u32 numVtx = (((w0 >> 20) & 0x0F) + 1);
            u32 v0 = (w0 >> 16) & 0x0F;
            const Vtx *vtxList = (const Vtx *)resolve_seg_address(w1);
            if ((uintptr_t)vtxList < 0x1000 || (uintptr_t)vtxList > 0x10000000) vtxList = NULL;

            if (vtxList) {
                update_mvp();

                for (u32 i = 0; i < numVtx && (v0 + i) < VERTEX_CACHE_SIZE; i++) {
                    const Vtx *src = &vtxList[i];
                    TransformedVertex *dst = &s_VertexCache[v0 + i];
                    s_Telemetry.total_vertices++;

                    float vx = (float)src->v.ob[0];
                    float vy = (float)src->v.ob[1];
                    float vz = (float)src->v.ob[2];

                    dst->x = vx * s_MvpMatrix[0][0] + vy * s_MvpMatrix[1][0] + vz * s_MvpMatrix[2][0] + s_MvpMatrix[3][0];
                    dst->y = vx * s_MvpMatrix[0][1] + vy * s_MvpMatrix[1][1] + vz * s_MvpMatrix[2][1] + s_MvpMatrix[3][1];
                    dst->z = vx * s_MvpMatrix[0][2] + vy * s_MvpMatrix[1][2] + vz * s_MvpMatrix[2][2] + s_MvpMatrix[3][2];
                    dst->w = vx * s_MvpMatrix[0][3] + vy * s_MvpMatrix[1][3] + vz * s_MvpMatrix[2][3] + s_MvpMatrix[3][3];

                    if (dst->w <= 0.0001f) {
                        dst->clipped = 1;
                        continue;
                    }
                    dst->clipped = 0;

                    float invW = 1.0f / dst->w;
                    float ndcX = dst->x * invW;
                    float ndcY = dst->y * invW;
                    float ndcZ = dst->z * invW;

                    dst->screen_x = s_Viewport.trans_x + ndcX * s_Viewport.scale_x;
                    dst->screen_y = s_Viewport.trans_y - ndcY * s_Viewport.scale_y;
                    dst->screen_z = s_Viewport.trans_z + ndcZ * s_Viewport.scale_z;

                    dst->u = (float)src->v.tc[0] / 32.0f;
                    dst->v = (float)src->v.tc[1] / 32.0f;

                    dst->r = src->v.cn[0];
                    dst->g = src->v.cn[1];
                    dst->b = src->v.cn[2];
                    dst->a = src->v.cn[3];
                }
            }
        } else if (opcode == OP_TRI1) {
            u32 idx0 = ((w1 >> 16) & 0xFF) / 10;
            u32 idx1 = ((w1 >> 8) & 0xFF) / 10;
            u32 idx2 = (w1 & 0xFF) / 10;
            if (idx0 < VERTEX_CACHE_SIZE && idx1 < VERTEX_CACHE_SIZE && idx2 < VERTEX_CACHE_SIZE) {
                rasterize_triangle(&s_VertexCache[idx0], &s_VertexCache[idx1], &s_VertexCache[idx2]);
            }
        } else if (opcode == OP_TRI4) {
            u32 x1 = (w1 >> 0) & 0x0F;
            u32 y1 = (w1 >> 4) & 0x0F;
            u32 z1 = (w0 >> 0) & 0x0F;

            u32 x2 = (w1 >> 8) & 0x0F;
            u32 y2 = (w1 >> 12) & 0x0F;
            u32 z2 = (w0 >> 4) & 0x0F;

            u32 x3 = (w1 >> 16) & 0x0F;
            u32 y3 = (w1 >> 20) & 0x0F;
            u32 z3 = (w0 >> 8) & 0x0F;

            u32 x4 = (w1 >> 24) & 0x0F;
            u32 y4 = (w1 >> 28) & 0x0F;
            u32 z4 = (w0 >> 12) & 0x0F;

            if (x1 || y1 || z1) rasterize_triangle(&s_VertexCache[x1], &s_VertexCache[y1], &s_VertexCache[z1]);
            if (x2 || y2 || z2) rasterize_triangle(&s_VertexCache[x2], &s_VertexCache[y2], &s_VertexCache[z2]);
            if (x3 || y3 || z3) rasterize_triangle(&s_VertexCache[x3], &s_VertexCache[y3], &s_VertexCache[z3]);
            if (x4 || y4 || z4) rasterize_triangle(&s_VertexCache[x4], &s_VertexCache[y4], &s_VertexCache[z4]);
        } else if (opcode == OP_DL) {
            const Gfx *nextDl = (const Gfx *)resolve_seg_address(w1);
            if ((uintptr_t)nextDl < 0x1000 || (uintptr_t)nextDl > 0x10000000) nextDl = NULL;
            u32 push = ((w0 >> 16) & 0xFF) == G_DL_PUSH;
            if (nextDl && depth < MAX_DL_DEPTH) {
                execute_dl_internal(nextDl, NULL, depth + 1);
            }
            if (!push) return; /* Branch - do not return to caller */
        } else if (opcode == OP_ENDDL || (w0 == 0 && w1 == 0)) {
            return;
        } else if (opcode == OP_SETTEX || opcode == OP_CULLDL || opcode == OP_LINE3D ||
                   opcode == OP_RDPHALF_1 || opcode == OP_RDPHALF_2 || opcode == OP_RDPHALF_CONT ||
                   opcode == OP_SETCIMG || opcode == OP_SETZIMG || opcode == OP_SETPRIMDEPTH ||
                   opcode == OP_SETKEYGB || opcode == OP_SETKEYR || opcode == OP_SETCONVERT ||
                   opcode == OP_RDPSETOTHERMODE) {
            /* Recognized non-critical display list state commands */
        } else {
            s_Telemetry.unsupported_commands++;
        }

        cur++;
    }
}

void hal_gfx_init(void) {
    memset(s_ColorBuffer, 0, sizeof(s_ColorBuffer));
    memset(s_DepthBuffer, 0xFF, sizeof(s_DepthBuffer));
    memset(s_Tmem, 0, sizeof(s_Tmem));
    memset(s_Segments, 0, sizeof(s_Segments));
    memset(&s_Telemetry, 0, sizeof(s_Telemetry));
    s_FrameCount = 0;

    mtx_identity(s_ProjectionMatrix);
    for (int i = 0; i < MAX_MODELVIEW_STACK; i++) {
        mtx_identity(s_ModelviewStack[i]);
    }
    s_ModelviewTop = 0;
    s_MvpDirty = 1;

    s_Viewport.scale_x = SCREEN_WIDTH / 2.0f;
    s_Viewport.scale_y = SCREEN_HEIGHT / 2.0f;
    s_Viewport.scale_z = 32767.0f;
    s_Viewport.trans_x = SCREEN_WIDTH / 2.0f;
    s_Viewport.trans_y = SCREEN_HEIGHT / 2.0f;
    s_Viewport.trans_z = 32767.0f;
}

void hal_gfx_execute_display_list(Gfx *start, Gfx *end) {
    if (!start || !end || start >= end) return;
    execute_dl_internal(start, end, 0);
    s_FrameCount++;
}

u16 *hal_gfx_get_framebuffer(void) {
    return s_ColorBuffer;
}

u32 hal_gfx_get_frame_count(void) {
    return s_FrameCount;
}

const HalGfxTelemetry *hal_gfx_get_telemetry(void) {
    return &s_Telemetry;
}

u32 hal_gfx_get_telemetry_json(char *buf, u32 maxlen) {
    if (!buf || maxlen == 0) return 0;
    return (u32)snprintf(buf, maxlen,
        "{\"commands\":%u,\"triangles\":%u,\"vertices\":%u,\"textures\":%u,\"unsupported\":%u,\"max_depth\":%u,\"frames\":%u}",
        s_Telemetry.total_commands,
        s_Telemetry.total_triangles,
        s_Telemetry.total_vertices,
        s_Telemetry.total_textures,
        s_Telemetry.unsupported_commands,
        s_Telemetry.max_dl_depth,
        s_FrameCount
    );
}
