#include <ultra64.h>

static u32 s_RandomSeed = 0x12345678;
static u32 s_ChrRandomSeed = 0x87654321;

void randomSetSeed(u32 seed) {
    s_RandomSeed = seed;
}

u32 randomGetNext(void) {
    s_RandomSeed = s_RandomSeed * 0x41C64E6D + 0x3039;
    return (s_RandomSeed >> 16) & 0x7FFF;
}

void chrObjRandomSetSeed(u32 seed) {
    s_ChrRandomSeed = seed;
}

u32 chrObjRandomGetNext(void) {
    s_ChrRandomSeed = s_ChrRandomSeed * 0x41C64E6D + 0x3039;
    return (s_ChrRandomSeed >> 16) & 0x7FFF;
}