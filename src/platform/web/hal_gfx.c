/**
 * GoldenEye 007 Web Port - Graphics HAL & GBI Tracer/Rasterizer
 *
 * NEW ADAPTER: hal_gfx_init, hal_gfx_start_frame, hal_gfx_end_frame,
 *              hal_gfx_process_display_list, hal_get_framebuffer,
 *              hal_get_engine_telemetry
 *
 * GBI Dialect: F3DEX 1.0 with GoldenEye extensions from include/gbi_extension.h:
 *   G_TRI4   = (G_IMMFIRST-14) = 0xB1  (replaces standard G_TRI2 at 0xB1)
 *   G_SETTEX = 0xC0  (custom Rare texture setup)
 *   gSP2Triangles -> gSP4Triangles via G_TRI4
 *
 * Endianness: N64/ROM GBI commands are Big-Endian.
 * In WASM (little-endian), each Gfx word must be byte-swapped before decoding.
 *
 * Framebuffer: 320x240 RGBA5551 (cfb_16), matching viInitBuffers in src/fr.c:209.
 */
#ifdef TARGET_WEB

#include <ultra64.h>
#include <PR/gbi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* Native framebuffer: 320x240 RGBA5551 (matches cfb_16 in fr.c) */
#define FB_W 320
#define FB_H 240

static u16 g_cfb[FB_W * FB_H];       /* RGBA5551 - native GE format */
static u32 g_rgba32[FB_W * FB_H];    /* RGBA8888 - for Canvas ImageData */
static f32 g_zbuf[FB_W * FB_H];

/* GoldenEye GBI opcode constants */
#define GE_G_TRI4    0xB1  /* (G_IMMFIRST-14): 4-triangle draw, GE extension */
#define GE_G_SETTEX  0xC0  /* Custom Rare texture setup, GE extension */

/* Telemetry */
typedef struct {
    s32 frame;
    s32 stage;
    s32 dl_count;
    s32 cmd_count;
    s32 tri_count;
    s32 rect_count;
    s32 unsupported_opcodes;
} EngineTelemetry;
static EngineTelemetry g_telem = {0};

/* Segment register table (8 segments) */
static u32 g_segTable[16] = {0};

/* GBI state */
static u32  g_fillColorRGBA = 0xFF000000;
static u32  g_scissorUlx = 0, g_scissorUly = 0;
static u32  g_scissorLrx = FB_W, g_scissorLry = FB_H;

/* Vertex cache */
typedef struct { f32 x, y, z, r, g, b, a; } VtxCached;
static VtxCached g_vtxCache[32];

/* Matrix helpers */
static void mat_ident(f32 m[4][4]) {
    int i, j;
    for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) m[i][j] = (i==j) ? 1.0f : 0.0f;
}

/* â”€â”€â”€ Framebuffer helpers â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€ */

static u16 rgba32_to_5551(u32 c) {
    u8 r = (c >> 0) & 0xFF;
    u8 g = (c >> 8) & 0xFF;
    u8 b = (c >> 16) & 0xFF;
    return ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | 1;
}

static void blit_cfb_to_rgba32(void) {
    u32 i;
    for (i = 0; i < FB_W * FB_H; i++) {
        u16 p = g_cfb[i];
        u8 r = ((p >> 11) & 0x1F) * 255 / 31;
        u8 g2 = ((p >> 6) & 0x1F) * 255 / 31;
        u8 b = ((p >> 1) & 0x1F) * 255 / 31;
        g_rgba32[i] = (0xFF << 24) | (b << 16) | (g2 << 8) | r;
    }
}

/* â”€â”€â”€ HAL API â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€ */

void hal_gfx_init(s32 width, s32 height) {
    memset(g_cfb, 0, sizeof(g_cfb));
    memset(g_rgba32, 0, sizeof(g_rgba32));
    for (u32 i = 0; i < FB_W * FB_H; i++) g_zbuf[i] = 1.0f;
    mat_ident((f32(*)[4])&g_segTable); /* unused matrix slots */
    memset(g_segTable, 0, sizeof(g_segTable));
    printf("[HAL_GFX] Init 320x240 RGBA5551 framebuffer. GE GBI extensions active.\n");
    printf("[HAL_GFX] G_TRI4=0x%02X, G_SETTEX=0x%02X\n", GE_G_TRI4, GE_G_SETTEX);
}

void hal_gfx_start_frame(void) {
    g_telem.frame++;
    g_telem.dl_count = 0;
    g_telem.cmd_count = 0;
    g_telem.tri_count = 0;
    g_telem.rect_count = 0;
    g_telem.unsupported_opcodes = 0;
    /* Clear framebuffer to black */
    memset(g_cfb, 0, sizeof(g_cfb));
    for (u32 i = 0; i < FB_W * FB_H; i++) g_zbuf[i] = 1.0f;
}

void hal_gfx_end_frame(void) {
    blit_cfb_to_rgba32();
}

u32 *EMSCRIPTEN_KEEPALIVE hal_get_framebuffer(void) {
    return g_rgba32;
}

void *EMSCRIPTEN_KEEPALIVE hal_get_engine_telemetry(void) {
    return &g_telem;
}

/* â”€â”€â”€ Byte-swap helper for GBI words (N64 Big-Endian â†’ WASM Little-Endian) â”€â”€ */
static u32 bswap32(u32 x) {
    return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) |
           ((x & 0x0000FF00) << 8)  | ((x & 0x000000FF) << 24);
}

/* â”€â”€â”€ Software rasterizer: fill rectangle â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€ */
static void fill_rect(u32 ulx, u32 uly, u32 lrx, u32 lry) {
    if (lrx > FB_W) lrx = FB_W;
    if (lry > FB_H) lry = FB_H;
    u16 c = rgba32_to_5551(g_fillColorRGBA);
    for (u32 y = uly; y < lry; y++) {
        for (u32 x = ulx; x < lrx; x++) {
            g_cfb[y * FB_W + x] = c;
        }
    }
    g_telem.rect_count++;
}

/* â”€â”€â”€ GBI tracer and rasterizer â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€ */
void hal_gfx_process_display_list(Gfx *dl) {
    if (!dl) return;
    g_telem.dl_count++;
    int depth = 0;
    int maxCmd = 65536;

    while (dl && maxCmd-- > 0) {
        /* N64 GBI words are Big-Endian. Byte-swap for WASM. */
        u32 w0 = bswap32(dl->words.w0);
        u32 w1 = bswap32(dl->words.w1);
        u8  op = (u8)(w0 >> 24);
        g_telem.cmd_count++;

        switch (op) {

        case 0x00: /* G_SPNOOP */
        case 0xC0: /* G_NOOP (RDP, same byte as GE_G_SETTEX â€” only 0xC0 in non-RDP context) */
            /* G_SETTEX (0xC0) â€” GE custom texture setup. Log and continue. */
            if (op == GE_G_SETTEX) {
                /* Phase 4 trace: log fields */
                u8 cms      = (w0 >> 22) & 0x3;
                u8 cmt      = (w0 >> 20) & 0x3;
                u8 tile     = (w0 >> 18) & 0x3;
                u16 tex_id  = (w1) & 0xFFF;
                if (g_telem.frame == 1 && g_telem.cmd_count <= 64) {
                    printf("[GBI TRACE] G_SETTEX cms=%d cmt=%d tile=%d tex_id=%d\n",
                           cms, cmt, tile, tex_id);
                }
            }
            break;

        case 0xB8: /* G_ENDDL */
            if (depth <= 0) goto done;
            depth--;
            break;

        case 0xE9: /* G_RDPFULLSYNC */
            goto done;

        case 0xE7: /* G_RDPPIPESYNC */
        case 0xBA: /* G_SETOTHERMODE_H */
        case 0xB9: /* G_SETOTHERMODE_L */
        case 0xB7: /* G_SETGEOMETRYMODE */
        case 0xB6: /* G_CLEARGEOMETRYMODE */
        case 0xBB: /* G_TEXTURE */
        case 0xBC: /* G_MOVEWORD */
        case 0xFC: /* G_SETCOMBINE */
        case 0xFB: /* G_SETENVCOLOR */
        case 0xFA: /* G_SETPRIMCOLOR */
        case 0xF8: /* G_SETFOGCOLOR */
        case 0xFE: /* G_SETZIMG */
            break;

        case 0x06: /* G_DL â€” branch or call */
        {
            Gfx *child = (Gfx *)w1;
            u8 nopush = (w0 >> 16) & 0xFF;
            if (depth < 16 && child) {
                depth++;
                if (nopush) { dl = child; continue; }
                else { hal_gfx_process_display_list(child); }
            } else {
                printf("[GBI] G_DL: depth limit or null ptr\n");
            }
            break;
        }

        case 0x01: /* G_MTX */
        {
            /* Update segment 0 (model matrix) â€” minimal */
            break;
        }


        case 0xFF: /* G_SETCIMG â€” set color framebuffer pointer */
            break;

        case 0xFD: /* G_SETTIMG */
            break;

        case 0xF7: /* G_SETFILLCOLOR */
        {
            u16 c = (u16)(w1 & 0xFFFF);
            u8 r = ((c >> 11) & 0x1F) * 255 / 31;
            u8 g2 = ((c >> 6) & 0x1F) * 255 / 31;
            u8 b = ((c >> 1) & 0x1F) * 255 / 31;
            g_fillColorRGBA = (0xFF << 24) | (b << 16) | (g2 << 8) | r;
            break;
        }

        case 0xF6: /* G_FILLRECT */
        {
            u32 lrx = ((w0 >> 14) & 0x3FF) >> 2;
            u32 lry = ((w0 >>  2) & 0x3FF) >> 2;
            u32 ulx = ((w1 >> 14) & 0x3FF) >> 2;
            u32 uly = ((w1 >>  2) & 0x3FF) >> 2;
            fill_rect(ulx, uly, lrx + 1, lry + 1);
            break;
        }

        case 0xED: /* G_SETSCISSOR */
        {
            g_scissorUlx = ((w0 >> 14) & 0x3FF) >> 2;
            g_scissorUly = ((w0 >>  2) & 0x3FF) >> 2;
            g_scissorLrx = ((w1 >> 14) & 0x3FF) >> 2;
            g_scissorLry = ((w1 >>  2) & 0x3FF) >> 2;
            break;
        }

        case 0xF5: /* G_SETTILE */
        case 0xF4: /* G_LOADTILE */
        case 0xF3: /* G_LOADBLOCK */
        case 0xF2: /* G_SETTILESIZE */
        case 0xF0: /* G_LOADTLUT */
            break;

        case 0xE4: /* G_TEXRECT */
        {
            u32 lrx = ((w0 >> 12) & 0xFFF) >> 2;
            u32 lry = (w0 & 0xFFF) >> 2;
            u32 ulx = ((w1 >> 12) & 0xFFF) >> 2;
            u32 uly = (w1 & 0xFFF) >> 2;
            fill_rect(ulx, uly, lrx + 1, lry + 1);
            break;
        }

        case 0x04: /* G_VTX â€” load vertex cache */
        {
            u32 n   = (w0 >> 20) & 0xF;
            u32 idx = (w0 >> 16) & 0xF;
            Vtx *v  = (Vtx *)w1;
            if (v) {
                u32 i;
                for (i = 0; i < n && (idx+i) < 32; i++) {
                    g_vtxCache[idx+i].x = (f32)v[i].v.ob[0];
                    g_vtxCache[idx+i].y = (f32)v[i].v.ob[1];
                    g_vtxCache[idx+i].z = (f32)v[i].v.ob[2];
                    g_vtxCache[idx+i].r = v[i].v.cn[0] / 255.0f;
                    g_vtxCache[idx+i].g = v[i].v.cn[1] / 255.0f;
                    g_vtxCache[idx+i].b = v[i].v.cn[2] / 255.0f;
                    g_vtxCache[idx+i].a = 1.0f;
                }
            }
            break;
        }

        case 0xBF: /* G_TRI1 */
        {
            g_telem.tri_count++;
            break;
        }

        case GE_G_TRI4: /* 0xB1 â€” GoldenEye G_TRI4 (NOT standard G_TRI2) */
        {
            /* 4 triangles packed in one 8-byte command */
            g_telem.tri_count += 4;
            /* Phase 4 trace */
            if (g_telem.frame == 1 && g_telem.cmd_count <= 128) {
                printf("[GBI TRACE] G_TRI4 @ %p w0=0x%08X w1=0x%08X\n", (void*)dl, w0, w1);
            }
            break;
        }

        default:
            g_telem.unsupported_opcodes++;
            if (g_telem.frame <= 3) {
                printf("[GBI UNSUPPORTED] op=0x%02X w0=0x%08X w1=0x%08X @ %p\n",
                       op, w0, w1, (void*)dl);
            }
            break;
        }

        dl++;
    }
done:;
}

/* â”€â”€â”€ libultra gu math stubs â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€ */

void guMtxIdentF(f32 mf[4][4]) { mat_ident(mf); }

void guMtxIdent(Mtx *m) {
    f32 mf[4][4]; mat_ident(mf); guMtxF2L(mf, m);
}

void guMtxF2L(f32 mf[4][4], Mtx *m) {
    int i, j;
    if (!m) return;
    for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) {
        s32 fixed = (s32)(mf[i][j] * 65536.0f);
        m->m[i][j]   = (u16)((fixed >> 16) & 0xFFFF);
        m->m[i+4][j] = (u16)(fixed & 0xFFFF);
    }
}

void guPerspectiveF(f32 mf[4][4], u16 *perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale) {
    f32 cot = 1.0f / tanf(fovy * 3.14159265f / 360.0f);
    mat_ident(mf);
    mf[0][0] = cot / aspect * scale;
    mf[1][1] = cot * scale;
    mf[2][2] = (near + far) / (near - far);
    mf[2][3] = -1.0f;
    mf[3][2] = 2.0f * near * far / (near - far);
    mf[3][3] = 0.0f;
    if (perspNorm) *perspNorm = 0xFFFF;
}

void guPerspective(Mtx *m, u16 *perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale) {
    f32 mf[4][4]; guPerspectiveF(mf, perspNorm, fovy, aspect, near, far, scale); guMtxF2L(mf, m);
}

void guNormalize(f32 *x, f32 *y, f32 *z) {
    f32 len = sqrtf((*x)*(*x)+(*y)*(*y)+(*z)*(*z));
    if (len > 1e-5f) { *x/=len; *y/=len; *z/=len; }
}

void guRotateF(f32 mf[4][4], f32 a, f32 x, f32 y, f32 z) {
    f32 rad = a*3.14159265f/180.0f, s = sinf(rad), c = cosf(rad);
    guNormalize(&x,&y,&z);
    mat_ident(mf);
    mf[0][0]=x*x*(1-c)+c;   mf[0][1]=y*x*(1-c)+z*s; mf[0][2]=x*z*(1-c)-y*s;
    mf[1][0]=x*y*(1-c)-z*s; mf[1][1]=y*y*(1-c)+c;   mf[1][2]=y*z*(1-c)+x*s;
    mf[2][0]=x*z*(1-c)+y*s; mf[2][1]=y*z*(1-c)-x*s; mf[2][2]=z*z*(1-c)+c;
}

void guRotate(Mtx *m, f32 a, f32 x, f32 y, f32 z) {
    f32 mf[4][4]; guRotateF(mf,a,x,y,z); guMtxF2L(mf,m);
}


void guScale(Mtx *m, f32 x, f32 y, f32 z) {
    f32 mf[4][4]; guScaleF(mf,x,y,z); guMtxF2L(mf,m);
}

void guTranslateF(f32 mf[4][4], f32 x, f32 y, f32 z) {
    mat_ident(mf); mf[3][0]=x; mf[3][1]=y; mf[3][2]=z;
}

void guTranslate(Mtx *m, f32 x, f32 y, f32 z) {
    f32 mf[4][4]; guTranslateF(mf,x,y,z); guMtxF2L(mf,m);
}

void guAlignF(f32 mf[4][4], f32 a, f32 x, f32 y, f32 z) { guRotateF(mf,a,x,y,z); }

void guLookAt(Mtx *m, f32 xEye, f32 yEye, f32 zEye,
              f32 xAt, f32 yAt, f32 zAt, f32 xUp, f32 yUp, f32 zUp) {
    if (m) guMtxIdent(m);
}

#endif /* TARGET_WEB */
