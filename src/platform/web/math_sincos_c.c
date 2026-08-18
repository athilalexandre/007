#include <PR/gu.h>
#include <ultra64.h>
#include <PR/os.h>
#include <PR/gbi.h>
#include <math.h>

#define FTOFIX32(x) ((s32)((x) * 65536.0f))

f32 sinf(f32 x) {
    return sin(x);
}

f32 cosf(f32 x) {
    return cos(x);
}

s16 sins(u16 angle) {
    f32 rad = (f32)angle * (3.14159265358979323846f / 32768.0f);
    return (s16)(sin(rad) * 32767.0f);
}

s16 coss(u16 angle) {
    f32 rad = (f32)angle * (3.14159265358979323846f / 32768.0f);
    return (s16)(cos(rad) * 32767.0f);
}

void guMtxF2L(float mf[4][4], Mtx *m) {
    int i, j;
    s32 e1, e2;
    s32 *ai, *af;

    ai = (s32 *)m->m[0];
    af = (s32 *)m->m[2];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            e1 = FTOFIX32(mf[i][j * 2]);
            e2 = FTOFIX32(mf[i][j * 2 + 1]);
            *(ai++) = (e1 & 0xffff0000) | ((e2 >> 16) & 0xffff);
            *(af++) = ((e1 << 16) & 0xffff0000) | (e2 & 0xffff);
        }
    }
}

void guMtxIdentF(float mf[4][4]) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            mf[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

void guMtxIdent(Mtx *m) {
    float mf[4][4];
    guMtxIdentF(mf);
    guMtxF2L(mf, m);
}

void guTranslateF(float mf[4][4], float x, float y, float z) {
    guMtxIdentF(mf);
    mf[3][0] = x;
    mf[3][1] = y;
    mf[3][2] = z;
}

void guTranslate(Mtx *m, float x, float y, float z) {
    float mf[4][4];
    guTranslateF(mf, x, y, z);
    guMtxF2L(mf, m);
}

void guScaleF(float mf[4][4], float x, float y, float z) {
    guMtxIdentF(mf);
    mf[0][0] = x;
    mf[1][1] = y;
    mf[2][2] = z;
}

void guScale(Mtx *m, float x, float y, float z) {
    float mf[4][4];
    guScaleF(mf, x, y, z);
    guMtxF2L(mf, m);
}

void guRotateF(float mf[4][4], float a, float x, float y, float z) {
    float s, c, t;
    float len = sqrtf(x * x + y * y + z * z);
    if (len != 0.0f) {
        x /= len;
        y /= len;
        z /= len;
    }
    a *= (3.14159265358979323846f / 180.0f);
    s = sinf(a);
    c = cosf(a);
    t = 1.0f - c;

    guMtxIdentF(mf);
    mf[0][0] = t * x * x + c;
    mf[0][1] = t * x * y + s * z;
    mf[0][2] = t * x * z - s * y;

    mf[1][0] = t * x * y - s * z;
    mf[1][1] = t * y * y + c;
    mf[1][2] = t * y * z + s * x;

    mf[2][0] = t * x * z + s * y;
    mf[2][1] = t * y * z - s * x;
    mf[2][2] = t * z * z + c;
}

void guRotate(Mtx *m, float a, float x, float y, float z) {
    float mf[4][4];
    guRotateF(mf, a, x, y, z);
    guMtxF2L(mf, m);
}

void guPerspectiveF(float mf[4][4], u16 *perspNorm, float fovy, float aspect, float near, float far, float scale) {
    float cot;
    guMtxIdentF(mf);
    fovy *= (3.14159265358979323846f / 360.0f);
    cot = cosf(fovy) / sinf(fovy);

    mf[0][0] = cot / aspect;
    mf[1][1] = cot;
    mf[2][2] = (near + far) / (near - far);
    mf[2][3] = -1.0f;
    mf[3][2] = (2.0f * near * far) / (near - far);
    mf[3][3] = 0.0f;

    if (perspNorm) {
        if (near + far <= 2.0f) {
            *perspNorm = 65535;
        } else {
            *perspNorm = (u16)((scale * 2.0f * 65536.0f) / (near + far));
            if (*perspNorm <= 0) *perspNorm = 1;
        }
    }
}

void guPerspective(Mtx *m, u16 *perspNorm, float fovy, float aspect, float near, float far, float scale) {
    float mf[4][4];
    guPerspectiveF(mf, perspNorm, fovy, aspect, near, far, scale);
    guMtxF2L(mf, m);
}

void guLookAtF(float mf[4][4], float xEye, float yEye, float zEye,
               float xAt, float yAt, float zAt,
               float xUp, float yUp, float zUp) {
    float len;
    float xLook, yLook, zLook;
    float xRight, yRight, zRight;

    xLook = xAt - xEye;
    yLook = yAt - yEye;
    zLook = zAt - zEye;
    len = -1.0f / sqrtf(xLook * xLook + yLook * yLook + zLook * zLook);
    xLook *= len;
    yLook *= len;
    zLook *= len;

    /* Right = Up x Look */
    xRight = yUp * zLook - zUp * yLook;
    yRight = zUp * xLook - xUp * zLook;
    zRight = xUp * yLook - yUp * xLook;
    len = 1.0f / sqrtf(xRight * xRight + yRight * yRight + zRight * zRight);
    xRight *= len;
    yRight *= len;
    zRight *= len;

    /* Up = Look x Right */
    xUp = yLook * zRight - zLook * yRight;
    yUp = zLook * xRight - xLook * zRight;
    zUp = xLook * yRight - yLook * xRight;

    guMtxIdentF(mf);
    mf[0][0] = xRight;
    mf[1][0] = yRight;
    mf[2][0] = zRight;
    mf[3][0] = -(xEye * xRight + yEye * yRight + zEye * zRight);

    mf[0][1] = xUp;
    mf[1][1] = yUp;
    mf[2][1] = zUp;
    mf[3][1] = -(xEye * xUp + yEye * yUp + zEye * zUp);

    mf[0][2] = zLook;
    mf[1][2] = yLook;
    mf[2][2] = zLook;
    mf[3][2] = -(xEye * zLook + yEye * yLook + zEye * zLook);

    mf[0][3] = 0.0f;
    mf[1][3] = 0.0f;
    mf[2][3] = 0.0f;
    mf[3][3] = 1.0f;
}

void guLookAt(Mtx *m, float xEye, float yEye, float zEye,
              float xAt, float yAt, float zAt,
              float xUp, float yUp, float zUp) {
    float mf[4][4];
    guLookAtF(mf, xEye, yEye, zEye, xAt, yAt, zAt, xUp, yUp, zUp);
    guMtxF2L(mf, m);
}
void guNormalize(float *x, float *y, float *z) {
    float len;
    if (!x || !y || !z) return;
    len = sqrtf((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
    if (len != 0.0f) {
        *x /= len;
        *y /= len;
        *z /= len;
    }
}
void guOrthoF(float mf[4][4], float l, float r, float b, float t, float n, float f, float scale) {
    guMtxIdentF(mf);
    mf[0][0] = 2.0f / (r - l);
    mf[1][1] = 2.0f / (t - b);
    mf[2][2] = -2.0f / (f - n);
    mf[3][0] = -(r + l) / (r - l);
    mf[3][1] = -(t + b) / (t - b);
    mf[3][2] = -(f + n) / (f - n);
    mf[3][3] = 1.0f;
}

void guOrtho(Mtx *m, float l, float r, float b, float t, float n, float f, float scale) {
    float mf[4][4];
    guOrthoF(mf, l, r, b, t, n, f, scale);
    guMtxF2L(mf, m);
}

void guLookAtReflectF(float mf[4][4], LookAt *l, float xEye, float yEye, float zEye,
                      float xAt, float yAt, float zAt, float xUp, float yUp, float zUp) {
    guLookAtF(mf, xEye, yEye, zEye, xAt, yAt, zAt, xUp, yUp, zUp);
}

void guLookAtReflect(Mtx *m, LookAt *l, float xEye, float yEye, float zEye,
                     float xAt, float yAt, float zAt, float xUp, float yUp, float zUp) {
    float mf[4][4];
    guLookAtReflectF(mf, l, xEye, yEye, zEye, xAt, yAt, zAt, xUp, yUp, zUp);
    guMtxF2L(mf, m);
}

void guAlignF(float mf[4][4], float a, float x, float y, float z) {
    guRotateF(mf, a, x, y, z);
}