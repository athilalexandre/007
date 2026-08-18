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

u32 randomGetNextFrom(u64 *param_1) {
    if (!param_1) return randomGetNext();
    *param_1 = (*param_1 * 0x41C64E6DULL + 0x3039ULL);
    return (u32)((*param_1 >> 16) & 0x7FFF);
}

void chrObjRandomSetSeed(u32 seed) {
    s_ChrRandomSeed = seed;
}

u32 chrObjRandomGetNext(void) {
    s_ChrRandomSeed = s_ChrRandomSeed * 0x41C64E6D + 0x3039;
    return (s_ChrRandomSeed >> 16) & 0x7FFF;
}

// ROM Segment Symbols required by linker
u8 unknown2[1] __attribute__((aligned(8)));
u8 unknown2_end[1] __attribute__((aligned(8)));

u32 ramrom_Dam_1;
u32 ramrom_Dam_2;
u32 ramrom_Facility_1;
u32 ramrom_Facility_2;
u32 ramrom_Facility_3;
u32 ramrom_Runway_1;
u32 ramrom_Runway_2;
u32 ramrom_BunkerI_1;
u32 ramrom_BunkerI_2;
u32 ramrom_Silo_1;
u32 ramrom_Silo_2;
u32 ramrom_Frigate_1;
u32 ramrom_Frigate_2;
u32 ramrom_Train;