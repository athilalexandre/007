#include <ultra64.h>

u64 g_randomSeed = 0x12345678ULL;
u64 g_chrObjRandomSeed = 0x87654321ULL;

void randomSetSeed(u32 seed) {
    g_randomSeed = seed;
}

u32 randomGetNext(void) {
    g_randomSeed = g_randomSeed * 0x41C64E6DULL + 0x3039ULL;
    return (u32)((g_randomSeed >> 16) & 0x7FFF);
}

u32 randomGetNextFrom(u64 *param_1) {
    if (!param_1) return randomGetNext();
    *param_1 = (*param_1 * 0x41C64E6DULL + 0x3039ULL);
    return (u32)((*param_1 >> 16) & 0x7FFF);
}

void chrObjRandomSetSeed(u32 seed) {
    g_chrObjRandomSeed = seed;
}

u32 chrObjRandomGetNext(void) {
    g_chrObjRandomSeed = g_chrObjRandomSeed * 0x41C64E6DULL + 0x3039ULL;
    return (u32)((g_chrObjRandomSeed >> 16) & 0x7FFF);
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