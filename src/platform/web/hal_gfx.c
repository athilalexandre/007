/**
 * GoldenEye 007 Web Port - Graphics HAL (Stub)
 * WebGL 2.0 backend for F3DEX display list rendering.
 * This is a STUB - will be implemented incrementally.
 */
#ifdef TARGET_WEB

#include "hal_gfx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES2/gl2.h>
#endif

static int g_gfxInitialized = 0;
static int g_screenWidth = 960;
static int g_screenHeight = 720;

void hal_gfx_init(s32 width, s32 height) {
    g_screenWidth = width;
    g_screenHeight = height;
    g_gfxInitialized = 1;
    printf("[HAL_GFX] Initialized WebGL context: %dx%d\n", width, height);
}

void hal_gfx_start_frame(void) {
#ifdef __EMSCRIPTEN__
    glViewport(0, 0, g_screenWidth, g_screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
}

void hal_gfx_end_frame(void) {
    /* WebGL auto-presents at end of requestAnimationFrame */
}

void hal_gfx_process_display_list(Gfx *dl) {
    /* TODO: This is the core function that needs to:
     * 1. Walk the display list
     * 2. Parse each 64-bit command word
     * 3. Translate F3DEX commands to WebGL calls
     * This is the most complex part of the entire port.
     */
    if (!dl) return;
    /* Stub - will be implemented in Phase 2 */
}

/* === gu* Matrix helper implementations === */

void guMtxIdent(Mtx *m) {
    memset(m, 0, sizeof(Mtx));
    m->m[0][0] = 0x00010000;
    m->m[1][1] = 0x00010000;
    m->m[2][2] = 0x00010000;
    m->m[3][3] = 0x00010000;
}

void guMtxF2L(f32 mf[4][4], Mtx *m) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            s32 val = (s32)(mf[i][j] * 65536.0f);
            m->m[i][j] = val;
        }
    }
}

void guMtxL2F(f32 mf[4][4], Mtx *m) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            mf[i][j] = (f32)m->m[i][j] / 65536.0f;
        }
    }
}

void guTranslate(Mtx *m, f32 x, f32 y, f32 z) {
    f32 mf[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {x, y, z, 1}
    };
    guMtxF2L(mf, m);
}

void guScale(Mtx *m, f32 x, f32 y, f32 z) {
    f32 mf[4][4] = {
        {x, 0, 0, 0},
        {0, y, 0, 0},
        {0, 0, z, 0},
        {0, 0, 0, 1}
    };
    guMtxF2L(mf, m);
}

void guRotate(Mtx *m, f32 angle, f32 x, f32 y, f32 z) {
    f32 rad = angle * 3.14159265f / 180.0f;
    f32 c = cosf(rad), s = sinf(rad);
    f32 len = sqrtf(x*x + y*y + z*z);
    if (len > 0.0f) { x /= len; y /= len; z /= len; }
    f32 mf[4][4] = {
        {x*x*(1-c)+c,   x*y*(1-c)+z*s, x*z*(1-c)-y*s, 0},
        {y*x*(1-c)-z*s, y*y*(1-c)+c,   y*z*(1-c)+x*s, 0},
        {z*x*(1-c)+y*s, z*y*(1-c)-x*s, z*z*(1-c)+c,   0},
        {0,             0,             0,             1}
    };
    guMtxF2L(mf, m);
}

void guPerspective(Mtx *m, u16 *perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale) {
    f32 rad = fovy * 3.14159265f / 360.0f;
    f32 cot = cosf(rad) / sinf(rad);
    f32 mf[4][4];
    memset(mf, 0, sizeof(mf));
    mf[0][0] = cot / aspect * scale;
    mf[1][1] = cot * scale;
    mf[2][2] = (near + far) / (near - far);
    mf[2][3] = -1.0f;
    mf[3][2] = 2.0f * near * far / (near - far);
    guMtxF2L(mf, m);
    if (perspNorm) *perspNorm = 0xFFFF;
}

void guOrtho(Mtx *m, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far, f32 scale) {
    f32 mf[4][4];
    memset(mf, 0, sizeof(mf));
    mf[0][0] = 2.0f / (right - left) * scale;
    mf[1][1] = 2.0f / (top - bottom) * scale;
    mf[2][2] = -2.0f / (far - near);
    mf[3][0] = -(right + left) / (right - left);
    mf[3][1] = -(top + bottom) / (top - bottom);
    mf[3][2] = -(far + near) / (far - near);
    mf[3][3] = 1.0f;
    guMtxF2L(mf, m);
}

void guLookAt(Mtx *m, f32 xEye, f32 yEye, f32 zEye,
              f32 xAt, f32 yAt, f32 zAt,
              f32 xUp, f32 yUp, f32 zUp) {
    f32 fx = xAt - xEye, fy = yAt - yEye, fz = zAt - zEye;
    f32 len = sqrtf(fx*fx + fy*fy + fz*fz);
    if (len > 0) { fx /= len; fy /= len; fz /= len; }
    f32 sx = fy*zUp - fz*yUp;
    f32 sy = fz*xUp - fx*zUp;
    f32 sz = fx*yUp - fy*xUp;
    len = sqrtf(sx*sx + sy*sy + sz*sz);
    if (len > 0) { sx /= len; sy /= len; sz /= len; }
    f32 ux = sy*fz - sz*fy;
    f32 uy = sz*fx - sx*fz;
    f32 uz = sx*fy - sy*fx;
    f32 mf[4][4] = {
        { sx,  ux, -fx, 0},
        { sy,  uy, -fy, 0},
        { sz,  uz, -fz, 0},
        {-(sx*xEye + sy*yEye + sz*zEye),
         -(ux*xEye + uy*yEye + uz*zEye),
          (fx*xEye + fy*yEye + fz*zEye), 1}
    };
    guMtxF2L(mf, m);
}

void guLookAtReflect(Mtx *m, LookAt *l, f32 xEye, f32 yEye, f32 zEye,
                     f32 xAt, f32 yAt, f32 zAt,
                     f32 xUp, f32 yUp, f32 zUp) {
    guLookAt(m, xEye, yEye, zEye, xAt, yAt, zAt, xUp, yUp, zUp);
}

void guMtxCatF(f32 m1[4][4], f32 m2[4][4], f32 res[4][4]) {
    f32 tmp[4][4];
    int i, j, k;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            tmp[i][j] = 0;
            for (k = 0; k < 4; k++) {
                tmp[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    memcpy(res, tmp, sizeof(tmp));
}

/* === Texture conversion helpers === */
void hal_gfx_convert_rgba16(u16 *src, u8 *dst_rgba, s32 width, s32 height) {
    int i;
    for (i = 0; i < width * height; i++) {
        u16 px = src[i];
        dst_rgba[i*4+0] = ((px >> 11) & 0x1F) << 3;
        dst_rgba[i*4+1] = ((px >> 6) & 0x1F) << 3;
        dst_rgba[i*4+2] = ((px >> 1) & 0x1F) << 3;
        dst_rgba[i*4+3] = (px & 1) ? 255 : 0;
    }
}

void hal_gfx_convert_ia8(u8 *src, u8 *dst_rgba, s32 width, s32 height) {
    int i;
    for (i = 0; i < width * height; i++) {
        u8 intensity = (src[i] >> 4) << 4;
        u8 alpha = (src[i] & 0x0F) << 4;
        dst_rgba[i*4+0] = intensity;
        dst_rgba[i*4+1] = intensity;
        dst_rgba[i*4+2] = intensity;
        dst_rgba[i*4+3] = alpha;
    }
}

void hal_gfx_convert_ci4(u8 *src, u16 *palette, u8 *dst_rgba, s32 width, s32 height) {
    int i;
    for (i = 0; i < width * height; i++) {
        u8 idx;
        if (i & 1) idx = src[i/2] & 0x0F;
        else       idx = (src[i/2] >> 4) & 0x0F;
        hal_gfx_convert_rgba16(&palette[idx], &dst_rgba[i*4], 1, 1);
    }
}

void hal_gfx_convert_ci8(u8 *src, u16 *palette, u8 *dst_rgba, s32 width, s32 height) {
    int i;
    for (i = 0; i < width * height; i++) {
        hal_gfx_convert_rgba16(&palette[src[i]], &dst_rgba[i*4], 1, 1);
    }
}

#endif /* TARGET_WEB */