/**
 * GoldenEye 007 Web Port — math_sincos.c (C replacement for math_sincos.s)
 * The original used MIPS assembly for sin/cos lookup table optimization.
 * On modern platforms we just use the standard C math library.
 */
#include <math.h>
#include <ultra64.h>

f32 sinf_custom(f32 x) {
    return sinf(x);
}

f32 cosf_custom(f32 x) {
    return cosf(x);
}

/* The original asm had these as separate functions for the game to call */
f64 sin_custom(f64 x) {
    return sin(x);
}

f64 cos_custom(f64 x) {
    return cos(x);
}
