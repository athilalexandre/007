#ifndef HAL_GFX_H
#define HAL_GFX_H

/**
 * GoldenEye 007 Web Port — Graphics Abstraction Layer
 * Provides WebGL 2.0 rendering backend that interprets N64 F3DEX display lists.
 *
 * The N64 GBI (Graphics Binary Interface) generates display list commands
 * for the RSP (Reality Signal Processor). On the web, we intercept these
 * at the GBI macro level and translate them to WebGL calls.
 *
 * Key systems to implement:
 * - Vertex loading and transformation (gSPVertex)
 * - Triangle rasterization (gSP1Triangle, gSP2Triangles)
 * - Texture loading and management (gDPLoadBlock, gDPSetTile)
 * - Combine mode / fragment shading (gDPSetCombineMode)
 * - Render mode / blending (gDPSetRenderMode)
 * - Matrix stack operations (gSPMatrix)
 * - Fog, lighting, geometry modes
 */

#include "hal_os.h"

/* === GFX Display List Types === */

/* Gfx command — 64-bit display list word (matches N64 Gfx union) */
typedef struct {
    u32 w0;
    u32 w1;
} Gfx;

/* Vertex structure — matches N64 Vtx union */
typedef struct {
    s16 ob[3]; /* position */
    u16 flag;
    s16 tc[2]; /* texture coords */
    u8  cn[4]; /* color / normal */
} Vtx_t;

typedef union {
    Vtx_t v;
    long long int force_structure_alignment;
} Vtx;

/* Matrix type — 4x4 fixed-point (matching N64 Mtx) */
typedef struct {
    s32 m[4][4];
} Mtx;

/* Texture Tile descriptor */
typedef struct {
    u8 fmt;
    u8 siz;
    u16 line;
    u16 tmem;
    u8 palette;
    u8 ct, mt, cs, ms;
    u8 shifts, shiftt;
    u8 masks, maskt;
} TileDescriptor;

/* Light structure */
typedef struct {
    u8 col[3];
    u8 pad1;
    u8 colc[3];
    u8 pad2;
    s8 dir[3];
    u8 pad3;
} Light_t;

typedef struct {
    Light_t l;
} Light;

typedef struct {
    u8 col[3];
    u8 pad1;
    u8 colc[3];
    u8 pad2;
} Ambient_t;

typedef struct {
    Ambient_t l;
} Ambient;

typedef struct {
    Ambient a;
    Light   l[7];
} Lights7;

typedef struct {
    Ambient a;
    Light   l[1];
} Lights1;

/* Viewport */
typedef struct {
    struct {
        s16 vscale[4];
        s16 vtrans[4];
    } vp;
} Vp;

/* LookAt (for specular highlight) */
typedef struct {
    s32 dummy; /* simplified for now */
} LookAt;

/* === GFX Initialization and Frame Management === */
void hal_gfx_init(s32 width, s32 height);
void hal_gfx_start_frame(void);
void hal_gfx_end_frame(void);
void hal_gfx_process_display_list(Gfx *dl);

/* === Matrix helpers (gu* functions) === */
void guMtxIdent(Mtx *m);
void guMtxF2L(f32 mf[4][4], Mtx *m);
void guMtxL2F(f32 mf[4][4], Mtx *m);
void guTranslate(Mtx *m, f32 x, f32 y, f32 z);
void guScale(Mtx *m, f32 x, f32 y, f32 z);
void guRotate(Mtx *m, f32 angle, f32 x, f32 y, f32 z);
void guPerspective(Mtx *m, u16 *perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale);
void guOrtho(Mtx *m, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far, f32 scale);
void guLookAt(Mtx *m, f32 xEye, f32 yEye, f32 zEye, f32 xAt, f32 yAt, f32 zAt, f32 xUp, f32 yUp, f32 zUp);
void guLookAtReflect(Mtx *m, LookAt *l, f32 xEye, f32 yEye, f32 zEye, f32 xAt, f32 yAt, f32 zAt, f32 xUp, f32 yUp, f32 zUp);
void guMtxCatF(f32 m1[4][4], f32 m2[4][4], f32 res[4][4]);

/* === Texture conversion helpers === */
void hal_gfx_convert_rgba16(u16 *src, u8 *dst_rgba, s32 width, s32 height);
void hal_gfx_convert_ia8(u8 *src, u8 *dst_rgba, s32 width, s32 height);
void hal_gfx_convert_ci4(u8 *src, u16 *palette, u8 *dst_rgba, s32 width, s32 height);
void hal_gfx_convert_ci8(u8 *src, u16 *palette, u8 *dst_rgba, s32 width, s32 height);

#endif /* HAL_GFX_H */
