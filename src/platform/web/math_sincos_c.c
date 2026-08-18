#include <math.h>
#include <ultra64.h>

s16 sins(u16 angle) {
    float rad = (float)angle * (2.0f * 3.1415926535f / 65536.0f);
    return (s16)(sinf(rad) * 32767.0f);
}

s16 coss(u16 angle) {
    float rad = (float)angle * (2.0f * 3.1415926535f / 65536.0f);
    return (s16)(cosf(rad) * 32767.0f);
}

f32 sinf_custom(f32 x) { return sinf(x); }
f32 cosf_custom(f32 x) { return cosf(x); }
f64 sin_custom(f64 x) { return sin(x); }
f64 cos_custom(f64 x) { return cos(x); }