#include <ultra64.h>
#include "random.h"

/**
 * Authentic 64-bit LFSR PRNG from GoldenEye 007 (src/random.s)
 * Initial seed: 0xAB8D9F7781280783ULL
 */
u64 g_randomSeed = 0xAB8D9F7781280783ULL;

u32 randomGetNext(void) {
    u64 a0 = g_randomSeed;
    u64 a2 = ((a0 << 63) >> 31) | ((a0 << 31) >> 32);
    a2 ^= ((a0 << 44) >> 32);
    a0 = ((a2 >> 20) & 0xFFF) ^ a2;
    g_randomSeed = a0;
    return (u32)a0;
}

void randomSetSeed(u32 seed) {
    g_randomSeed = ((u64)seed) + 1;
}

u32 randomGetNextFrom(u64 *param_1) {
    u64 a3 = *param_1;
    u64 a2 = ((a3 << 63) >> 31) | ((a3 << 31) >> 32);
    a2 ^= ((a3 << 44) >> 32);
    a3 = ((a2 >> 20) & 0xFFF) ^ a2;
    *param_1 = a3;
    return (u32)a3;
}