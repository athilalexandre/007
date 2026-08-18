#include <stdlib.h>
#include <string.h>
#include <ultra64.h>
#include <stdio.h>
#include "hal_os.h"
#include "hal_gfx.h"
#include "hal_input.h"
#include "hal_audio.h"

extern void bossInitMainthreadData(void);
extern Gfx *dynGetMasterDisplayList(void);
extern Gfx *lvlRender(Gfx *gdl);
extern void joyConsumeSamplesWrapper(void);

static u32 s_EngineInitialized = 0;
static u32 s_FrameCount = 0;

int hal_engine_init(void) {
    if (s_EngineInitialized) {
        return 1;
    }

    if (hal_os_get_rom_status() != 1) {
        return 0; /* Fail: No valid ROM mounted */
    }

    hal_gfx_init();
    hal_input_init();
    hal_audio_init();

    bossInitMainthreadData();
    s_EngineInitialized = 1;
    s_FrameCount = 0;

    return 1;
}

u32 hal_engine_is_initialized(void) {
    return s_EngineInitialized;
}

u32 hal_engine_step(void) {
    if (!s_EngineInitialized) {
        return 0;
    }

    joyConsumeSamplesWrapper();

    Gfx *gdl = dynGetMasterDisplayList();
    if (gdl) {
        gdl = lvlRender(gdl);
        hal_gfx_render_display_list(gdl);
    }

    s_FrameCount++;
    return s_FrameCount;
}

const char *hal_get_build_info(void) {
    return "GoldenEye 007 (N64) WebAssembly Port - Authentic Engine Proof-of-Life";
}