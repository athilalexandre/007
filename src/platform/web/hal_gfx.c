#include "hal_gfx.h"
#include <ultra64.h>
#include <PR/gbi.h>
#include "gbi_extension.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define MAX_MODELVIEW_STACK 32

/* Authentic F3DEX 1 (GoldenEye 007) Command Opcodes */
#undef G_SPNOOP
#undef G_MTX
#undef G_MOVEMEM
#undef G_VTX
#undef G_DL
#undef G_TRI1
#undef G_TRI2
#undef G_TRI4
#undef G_LINE3D
#undef G_CLEARGEOMETRYMODE
#undef G_SETGEOMETRYMODE
#undef G_ENDDL
#undef G_SETOTHERMODE_L
#undef G_SETOTHERMODE_H
#undef G_TEXTURE
#undef G_MOVEWORD
#undef G_POPMTX
#undef G_CULLDL

#define G_SPNOOP            0x00
#define G_MTX               0x01
#define G_MOVEMEM           0x03
#define G_VTX               0x04
#define G_DL                0x06
#define G_TRI4              0xB1
#define G_TRI2              0xB1
#define G_LINE3D            0xB5
#define G_CLEARGEOMETRYMODE 0xB6
#define G_SETGEOMETRYMODE   0xB7
#define G_ENDDL             0xB8
#define G_SETOTHERMODE_L    0xB9
#define G_SETOTHERMODE_H    0xBA
#define G_TEXTURE           0xBB
#define G_MOVEWORD          0xBC
#define G_POPMTX            0xBD
#define G_CULLDL            0xBE
#define G_TRI1              0xBF

/* RDP Command Opcodes */
#undef G_TEXRECT
#undef G_TEXRECTFLIP
#undef G_RDPLOADSYNC
#undef G_RDPPIPESYNC
#undef G_RDPTILESYNC
#undef G_RDPFULLSYNC
#undef G_SETKEYGB
#undef G_SETKEYR
#undef G_SETCONVERT
#undef G_SETSCISSOR
#undef G_SETPRIMDEPTH
#undef G_RDPSETOTHERMODE
#undef G_LOADTLUT
#undef G_SETTILESIZE
#undef G_LOADBLOCK
#undef G_LOADTILE
#undef G_SETTILE
#undef G_FILLRECT
#undef G_SETFILLCOLOR
#undef G_SETFOGCOLOR
#undef G_SETBLENDCOLOR
#undef G_SETPRIMCOLOR
#undef G_SETENVCOLOR
#undef G_SETCOMBINE
#undef G_SETTIMG

#define G_TEXRECT           0xE4
#define G_TEXRECTFLIP       0xE5
#define G_RDPLOADSYNC       0xE6
#define G_RDPPIPESYNC       0xE7
#define G_RDPTILESYNC       0xE8
#define G_RDPFULLSYNC       0xE9
#define G_SETKEYGB          0xEA
#define G_SETKEYR           0xEB
#define G_SETCONVERT        0xEC
#define G_SETSCISSOR        0xED
#define G_SETPRIMDEPTH      0xEE
#define G_RDPSETOTHERMODE   0xEF
#define G_LOADTLUT          0xF0
#define G_SETTILESIZE       0xF2
#define G_LOADBLOCK         0xF3
#define G_LOADTILE          0xF4
#define G_SETTILE           0xF5
#define G_FILLRECT          0xF6
#define G_SETFILLCOLOR      0xF7
#define G_SETFOGCOLOR       0xF8
#define G_SETBLENDCOLOR     0xF9
#define G_SETPRIMCOLOR      0xFA
#define G_SETENVCOLOR       0xFB
#define G_SETCOMBINE        0xFC
#define G_SETTIMG           0xFD

#define M_COLOR_R(c) (((c) >> 24) & 0xFF)
#define M_COLOR_G(c) (((c) >> 16) & 0xFF)
#define M_COLOR_B(c) (((c) >> 8) & 0xFF)
#define M_COLOR_A(c) ((c) & 0xFF)

static u16 s_ColorBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static u16 s_DepthBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static u8  s_Tmem[4096];
static u32 s_FrameCount = 0;
static HalGfxTelemetry s_Telemetry;

typedef struct {
    float x, y, z, w;
    float screen_x, screen_y;
    float u, v;
    u8 r, g, b, a;
} TransformedVertex;

static TransformedVertex s_VertexCache[64];

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

typedef struct {
    u32 fmt, siz, line, tmem, palette;
    u32 cmt, maskt, shiftt;
    u32 cms, masks, shifts;
    u32 uls, ult, lrs, lrt;
} GbiTile;

static GbiTile s_Tiles[8];
static const u8 *s_CurrentTextureImage = NULL;
static u32 s_TextureImageWidth = 0;

static const void *s_Segments[16] = {0};

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
    u32 seg = (addr >> 24) & 0x0F;
    u32 off = addr & 0x00FFFFFF;
    if (seg > 0 && seg < 16) {
        if (s_Segments[seg] == NULL) {
            return NULL;
        }
        return (const void *)((uintptr_t)s_Segments[seg] + off);
    }
    return (const void *)(uintptr_t)addr;
}

static inline u16 rgba32_to_5551(u8 r, u8 g, u8 b, u8 a) {
    return (u16)(((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | (a ? 1 : 0));
}

static void rasterize_triangle(const TransformedVertex *v0, const TransformedVertex *v1, const TransformedVertex *v2) {
    s_Telemetry.total_triangles++;

    float fminX = v0->screen_x < v1->screen_x ? (v0->screen_x < v2->screen_x ? v0->screen_x : v2->screen_x) : (v1->screen_x < v2->screen_x ? v1->screen_x : v2->screen_x);
    float fmaxX = v0->screen_x > v1->screen_x ? (v0->screen_x > v2->screen_x ? v0->screen_x : v2->screen_x) : (v1->screen_x > v2->screen_x ? v1->screen_x : v2->screen_x);
    float fminY = v0->screen_y < v1->screen_y ? (v0->screen_y < v2->screen_y ? v0->screen_y : v2->screen_y) : (v1->screen_y < v2->screen_y ? v1->screen_y : v2->screen_y);
    float fmaxY = v0->screen_y > v1->screen_y ? (v0->screen_y > v2->screen_y ? v0->screen_y : v2->screen_y) : (v1->screen_y > v2->screen_y ? v1->screen_y : v2->screen_y);

    int minX = (int)fminX;
    int maxX = (int)fmaxX + 1;
    int minY = (int)fminY;
    int maxY = (int)fmaxY + 1;

    if (minX < 0) minX = 0;
    if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;
    if (minY < 0) minY = 0;
    if (maxY > SCREEN_HEIGHT) maxY = SCREEN_HEIGHT;

    if (minX >= maxX || minY >= maxY) return;

    float denom = (v1->screen_y - v2->screen_y) * (v0->screen_x - v2->screen_x) + (v2->screen_x - v1->screen_x) * (v0->screen_y - v2->screen_y);
    if (__builtin_fabs(denom) < 0.0001) return;
    float invDenom = 1.0f / denom;

    for (int y = minY; y < maxY; y++) {
        float py = y + 0.5f;
        for (int x = minX; x < maxX; x++) {
            float px = x + 0.5f;
            float w0 = ((v1->screen_y - v2->screen_y) * (px - v2->screen_x) + (v2->screen_x - v1->screen_x) * (py - v2->screen_y)) * invDenom;
            float w1 = ((v2->screen_y - v0->screen_y) * (px - v2->screen_x) + (v0->screen_x - v2->screen_x) * (py - v2->screen_y)) * invDenom;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                float z = w0 * v0->z + w1 * v1->z + w2 * v2->z;
                u16 zDepth = (u16)(z * 65535.0f);
                int pixelIdx = y * SCREEN_WIDTH + x;

                if (zDepth < s_DepthBuffer[pixelIdx] || !(s_GeometryMode & G_ZBUFFER)) {
                    if (s_GeometryMode & G_ZBUFFER) {
                        s_DepthBuffer[pixelIdx] = zDepth;
                    }

                    u8 r = (u8)(w0 * v0->r + w1 * v1->r + w2 * v2->r);
                    u8 g = (u8)(w0 * v0->g + w1 * v1->g + w2 * v2->g);
                    u8 b = (u8)(w0 * v0->b + w1 * v1->b + w2 * v2->b);
                    u8 a = (u8)(w0 * v0->a + w1 * v1->a + w2 * v2->a);

                    s_ColorBuffer[pixelIdx] = rgba32_to_5551(r, g, b, a);
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

    if (ulx < 0) ulx = 0;
    if (uly < 0) uly = 0;
    if (lrx > SCREEN_WIDTH) lrx = SCREEN_WIDTH;
    if (lry > SCREEN_HEIGHT) lry = SCREEN_HEIGHT;
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
    if (ulx < 0) ulx = 0;
    if (uly < 0) uly = 0;
    if (lrx > SCREEN_WIDTH) lrx = SCREEN_WIDTH;
    if (lry > SCREEN_HEIGHT) lry = SCREEN_HEIGHT;
    if (ulx >= lrx || uly >= lry) return;

    GbiTile *tile = &s_Tiles[0];
    u32 tmemByteOffset = tile->tmem * 8;
    if (tmemByteOffset >= 4096) tmemByteOffset = 0;

    const u8 *tmem8 = s_Tmem + tmemByteOffset;
    u16 primColor16 = rgba32_to_5551(M_COLOR_R(s_PrimColor), M_COLOR_G(s_PrimColor), M_COLOR_B(s_PrimColor), M_COLOR_A(s_PrimColor));
    if (!primColor16) primColor16 = 0xFFFF;

    float curT = t;
    for (int y = uly; y < lry && y < SCREEN_HEIGHT; y++) {
        float curS = s;
        int texY = (int)curT;
        if (texY < 0) texY = 0;
        for (int x = ulx; x < lrx && x < SCREEN_WIDTH; x++) {
            int texX = (int)curS;
            if (texX < 0) texX = 0;
            
            u32 byteIdx = (texY * ((tile->line > 0) ? (tile->line * 8) : (lrx - ulx)) + texX);
            u8 intensity = (byteIdx < 4096) ? tmem8[byteIdx] : 0;
            
            if (intensity > 0) {
                s_ColorBuffer[y * SCREEN_WIDTH + x] = primColor16;
            }
            curS += dsdx;
        }
        curT += dtdy;
    }
}

static void execute_dl_internal(const Gfx *dl, const Gfx *end, int depth) {
    if (!dl || depth > 16) return;
    if ((uintptr_t)dl < 0x1000 || (uintptr_t)dl > 0x10000000) return;

    const Gfx *cur = dl;
    while (!end || cur < end) {
        s_Telemetry.total_commands++;
        u32 w0 = cur->words.w0;
        u32 w1 = cur->words.w1;
        u8 opcode = (w0 >> 24) & 0xFF;

        if (opcode == G_SPNOOP) {
            /* No-op */
        } else if (opcode == G_RDPPIPESYNC || opcode == G_RDPTILESYNC || opcode == G_RDPLOADSYNC || opcode == G_RDPFULLSYNC) {
            /* Syncs handled */
        } else if (opcode == (u8)G_MOVEWORD) {
            u8 index = w0 & 0xFF;
            u16 offset = (w0 >> 8) & 0xFFFF;
            if (index == G_MW_SEGMENT) {
                u32 seg = (offset >> 2) & 0x0F;
                if (seg < 16) {
                    s_Segments[seg] = (const void *)(uintptr_t)w1;
                }
            }
        } else if (opcode == G_SETTIMG) {
            s_CurrentTextureImage = (const u8 *)resolve_seg_address(w1);
            s_TextureImageWidth = (w0 & 0x0FFF) + 1;
        } else if (opcode == G_SETCOMBINE) {
            s_CombineMode = ((u64)w0 << 32) | w1;
        } else if (opcode == G_SETENVCOLOR) {
            s_EnvColor = w1;
        } else if (opcode == G_SETPRIMCOLOR) {
            s_PrimColor = w1;
        } else if (opcode == G_SETBLENDCOLOR) {
            s_BlendColor = w1;
        } else if (opcode == G_SETFOGCOLOR) {
            s_FogColor = w1;
        } else if (opcode == G_SETFILLCOLOR) {
            s_FillColor = w1;
        } else if (opcode == G_FILLRECT) {
            int lrx = (w0 >> 12) & 0x0FFF;
            int lry = (w0) & 0x0FFF;
            int ulx = (w1 >> 12) & 0x0FFF;
            int uly = (w1) & 0x0FFF;
            draw_fill_rect(ulx, uly, lrx, lry);
        } else if (opcode == G_SETSCISSOR) {
            s_ScissorLeft = ((w0 >> 12) & 0x0FFF) >> 2;
            s_ScissorTop = (w0 & 0x0FFF) >> 2;
            s_ScissorRight = ((w1 >> 12) & 0x0FFF) >> 2;
            s_ScissorBottom = (w1 & 0x0FFF) >> 2;
        } else if (opcode == (u8)G_SETOTHERMODE_H) {
            s_OtherModeH = w1;
        } else if (opcode == (u8)G_SETOTHERMODE_L) {
            s_OtherModeL = w1;
        } else if (opcode == (u8)G_SETGEOMETRYMODE) {
            s_GeometryMode |= w1;
        } else if (opcode == (u8)G_CLEARGEOMETRYMODE) {
            s_GeometryMode &= ~w1;
        } else if (opcode == G_SETTILE) {
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
        } else if (opcode == G_SETTILESIZE) {
            u32 idx = (w1 >> 24) & 0x07;
            GbiTile *t = &s_Tiles[idx];
            t->uls = (w0 >> 12) & 0x0FFF;
            t->ult = w0 & 0x0FFF;
            t->lrs = (w1 >> 12) & 0x0FFF;
            t->lrt = w1 & 0x0FFF;
        } else if (opcode == G_LOADBLOCK) {
            u32 tileIdx = (w1 >> 24) & 0x07;
            u32 tmemOffset = s_Tiles[tileIdx].tmem * 8;
            u32 numDwords = ((w1 >> 12) & 0x0FFF) + 1;
            u32 numBytes = numDwords * 8;
            if (tmemOffset >= 4096) tmemOffset = 0;
            if (tmemOffset + numBytes > 4096) numBytes = 4096 - tmemOffset;
            if (s_CurrentTextureImage && (uintptr_t)s_CurrentTextureImage >= 0x1000) {
                memcpy(s_Tmem + tmemOffset, s_CurrentTextureImage, numBytes);
            }
        } else if (opcode == G_LOADTLUT) {
            u32 count = (w1 >> 14) & 0x03FF;
            u32 numBytes = (count + 1) * 2;
            if (numBytes > 512) numBytes = 512;
            if (s_CurrentTextureImage && (uintptr_t)s_CurrentTextureImage >= 0x1000) {
                memcpy(s_Tmem + 0x800, s_CurrentTextureImage, numBytes);
            }
        } else if (opcode == G_TEXRECT || opcode == G_TEXRECTFLIP) {
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
        } else if (opcode == (u8)G_MTX) {
            const Mtx *srcMtx = (const Mtx *)resolve_seg_address(w1);
            if (!srcMtx || (uintptr_t)srcMtx < 0x1000 || (uintptr_t)srcMtx > 0x10000000) { cur++; continue; }
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
                if (params & G_MTX_LOAD) {
                    memcpy(s_ModelviewStack[s_ModelviewTop], mat, 16 * sizeof(float));
                } else {
                    mtx_mult(s_ModelviewStack[s_ModelviewTop], s_ModelviewStack[s_ModelviewTop], mat);
                }
            }
            s_MvpDirty = 1;
        } else if (opcode == (u8)G_POPMTX) {
            if (s_ModelviewTop > 0) s_ModelviewTop--;
            s_MvpDirty = 1;
        } else if (opcode == (u8)G_VTX) {
            u32 numVtx = (w0 >> 10) & 0x3F;
            u32 v0 = ((w0 >> 16) & 0xFF) / 2;
            if (numVtx == 0) {
                numVtx = (w0 >> 19) & 0x1F;
                v0 = (w0 >> 24) & 0x0F;
            }
            const Vtx *vtxList = (const Vtx *)resolve_seg_address(w1);
            if (!vtxList || (uintptr_t)vtxList < 0x1000 || (uintptr_t)vtxList > 0x10000000) { cur++; continue; }

            update_mvp();

            for (u32 i = 0; i < numVtx && (v0 + i) < 64; i++) {
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

                float invW = (__builtin_fabs(dst->w) > 0.0001) ? (1.0f / dst->w) : 1.0f;
                dst->screen_x = (dst->x * invW + 1.0f) * 0.5f * SCREEN_WIDTH;
                dst->screen_y = (1.0f - dst->y * invW) * 0.5f * SCREEN_HEIGHT;

                dst->u = (float)src->v.tc[0] / 32.0f;
                dst->v = (float)src->v.tc[1] / 32.0f;

                dst->r = src->v.cn[0];
                dst->g = src->v.cn[1];
                dst->b = src->v.cn[2];
                dst->a = src->v.cn[3];
            }
        } else if (opcode == (u8)G_TRI1) {
            u32 idx0 = ((w1 >> 16) & 0xFF) / 2;
            u32 idx1 = ((w1 >> 8) & 0xFF) / 2;
            u32 idx2 = (w1 & 0xFF) / 2;
            if (idx0 == 0 && idx1 == 0 && idx2 == 0) {
                idx0 = ((w0 >> 16) & 0xFF) / 10;
                idx1 = ((w0 >> 8) & 0xFF) / 10;
                idx2 = (w0 & 0xFF) / 10;
            }
            if (idx0 < 64 && idx1 < 64 && idx2 < 64) {
                rasterize_triangle(&s_VertexCache[idx0], &s_VertexCache[idx1], &s_VertexCache[idx2]);
            }
        } else if (opcode == (u8)G_TRI4) {
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
        } else if (opcode == (u8)G_DL) {
            const Gfx *nextDl = (const Gfx *)resolve_seg_address(w1);
            if (!nextDl || (uintptr_t)nextDl < 0x1000 || (uintptr_t)nextDl > 0x10000000) { cur++; continue; }
            if (depth < 16) {
                execute_dl_internal(nextDl, NULL, depth + 1);
            }
            if (w0 & 0x00010000) return; /* Branch */
        } else if (opcode == (u8)G_ENDDL || (w0 == 0 && w1 == 0)) {
            return;
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
    memset(&s_Telemetry, 0, sizeof(s_Telemetry));
    memset(s_Segments, 0, sizeof(s_Segments));
    s_FrameCount = 0;

    mtx_identity(s_ProjectionMatrix);
    for (int i = 0; i < MAX_MODELVIEW_STACK; i++) {
        mtx_identity(s_ModelviewStack[i]);
    }
    s_ModelviewTop = 0;
    s_MvpDirty = 1;
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
        "{\"commands\":%u,\"triangles\":%u,\"vertices\":%u,\"textures\":%u,\"unsupported\":%u,\"frames\":%u}",
        s_Telemetry.total_commands,
        s_Telemetry.total_triangles,
        s_Telemetry.total_vertices,
        s_Telemetry.total_textures,
        s_Telemetry.unsupported_commands,
        s_FrameCount
    );
}
