#include <ultra64.h>
#include <stdio.h>
#include "boss.h"
#include "memp.h"
#include "mema.h"

extern void bossInitMainthreadData(void);
extern void hal_os_set_rom_data(const u8 *data, size_t size);
extern u32 hal_os_get_rom_status(void);
extern void hal_gfx_init(void);

static u32 s_EngineInitialized = 0;

int hal_engine_init(void) {
    hal_gfx_init();
    if (hal_os_get_rom_status() == 1) {
        bossInitMainthreadData();
        s_EngineInitialized = 1;
    } else {
        s_EngineInitialized = 1;
    }
    return s_EngineInitialized;
}

u32 hal_engine_is_initialized(void) {
    return s_EngineInitialized;
}

const char *hal_get_build_info(void) {
    return "GoldenEye 007 (N64) WebAssembly Port - Phase 1 Engine Proof-of-Life (Clean Baseline)";
}