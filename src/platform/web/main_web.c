#include <ultra64.h>
#include <stdio.h>
#include "boss.h"
#include "memp.h"
#include "mema.h"

extern void bossInitMainthreadData(void);
extern void hal_os_set_rom_data(const u8 *data, u32 size);
extern u32 hal_os_get_rom_status(void);
extern void hal_gfx_init(void);

static u32 s_EngineInitialized = 0;

void hal_engine_init(void) {
    if (!s_EngineInitialized) {
        hal_gfx_init();
        bossInitMainthreadData();
        s_EngineInitialized = 1;
    }
}

u32 hal_engine_is_initialized(void) {
    return s_EngineInitialized;
}

const char *hal_get_build_info(void) {
    return "GoldenEye 007 (N64) WebAssembly Port - Phase 1 Engine Proof-of-Life (Clean Baseline)";
}