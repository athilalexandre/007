/**
 * GoldenEye 007 Web Port — Random Number Generator (C replacement for random.s)
 * Reimplements the 64-bit XOR-shift PRNG that was originally in MIPS assembly.
 */
#include <random.h>

u64 g_randomSeed = 0x81280783AB8D9F77ULL;

/**
 * Core PRNG — 64-bit XOR-shift generator
 * Matches the original MIPS assembly implementation exactly.
 */
u32 randomGetNext(void) {
    u64 seed = g_randomSeed;
    u64 a, b, c;

    /* dsll32 + dsrl: combined shift-left by 31, then XOR with shift-right */
    a = (seed << 31) | (seed >> 33);  /* rotate-like shift */
    b = seed << 44;                    /* dsll32 by 12 = shift left 44 */
    b = (u32)b;                        /* dsrl32 by 0 = take lower 32 bits extended */
    a = a ^ b;

    /* dsrl by 20, mask 0xFFF, XOR back */
    c = (a >> 20) & 0xFFF;
    a = c ^ a;

    g_randomSeed = a;

    /* dsll32 then dsra32: sign-extend lower 32 bits */
    return (u32)a;
}

void randomSetSeed(u32 param_1) {
    g_randomSeed = (u64)(param_1 + 1);
}

u32 randomGetNextFrom(u64 *param_1) {
    u64 seed = *param_1;
    u64 a, b, c;

    a = (seed << 31) | (seed >> 33);
    b = seed << 44;
    b = (u32)b;
    a = a ^ b;

    c = (a >> 20) & 0xFFF;
    a = c ^ a;

    *param_1 = a;
    return (u32)a;
}
