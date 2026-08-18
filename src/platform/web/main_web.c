#include "game/textrelated.h"
#include <ultra64.h>

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#include "hal_os.h"
#include "hal_gfx.h"
#include "hal_input.h"
#include "hal_audio.h"

// Original GoldenEye decompilation includes
#include "boss.h"
#include "joy.h"
#include "memp.h"
#include "mema.h"
#include "debugmenu.h"
#include "game/lv.h"
#include "game/dyn.h"
#include "game/front.h"
#include "game/frametiming.h"

static int s_EngineInitialized = 0;
static uint32_t s_FrameCount = 0;

// Declarations of engine globals and internal functions
extern s32 g_StageNum;
extern s32 g_MainStageNum;
extern u32 g_CurentMaMallocValue;

extern void obBlankResourcesLoadedInBank(u8 bank);
extern void reset_play_data_ptrs(void);
extern void init_player_data_ptrs_construct_viewports(s32 playernum);
extern void lvlManageMpGame(void);
extern void shuffle_player_ids(void);

extern void osSyncPrintf(const char *fmt, ...);
extern void bossInitMainthreadData(void);
extern void viInitBuffers(void);
extern void tokenSetString(const char *string);
extern const char *tokenFind(s32 arg0, const char *arg1);
extern long int strtol(const char *str, char **endptr, int base);

#define LOG_STEP(msg) do { osSyncPrintf(msg); } while(0)

int hal_engine_init(void) {
    if (s_EngineInitialized) {
        return 1;
    }

    if (hal_os_get_rom_status() != 1) {
        osSyncPrintf("[GE007-CORE] Error: ROM not mounted (status=%d)\n", hal_os_get_rom_status());
        return -1;
    }

    LOG_STEP("[GE007-CORE] 1. Initializing Web HAL subsystems...\n");
    hal_gfx_init();
    hal_input_init();
    hal_audio_init();

    LOG_STEP("[GE007-CORE] 2. Calling bossInitMainthreadData()...\n");
    bossInitMainthreadData();

    LOG_STEP("[GE007-CORE] 3. Setting stage to LEVELID_TITLE (90 / 0x5A)...\n");
    g_StageNum = LEVELID_TITLE;

    LOG_STEP("[GE007-CORE] 4.1 tokenSetString\n");
    tokenSetString("-ml0 -me0 -mgfx80 -mvtx20 -mt646 -ma001");
    LOG_STEP("[GE007-CORE] 4.2 mempResetBank\n");
    mempResetBank(MEMPOOL_STAGE);
    LOG_STEP("[GE007-CORE] 4.3 obBlankResourcesLoadedInBank\n");
    obBlankResourcesLoadedInBank(MEMPOOL_STAGE);
    if (tokenFind(1, "-ma")) {
        g_CurentMaMallocValue = (s32)(strtol(tokenFind(1, "-ma"), NULL, 0) * 1024);
    }
        osSyncPrintf("[GE007-CORE] 4.4 Calling mempAllocBytesInBank(bytes=0x%x, bank=%d)...\n", g_CurentMaMallocValue, MEMPOOL_STAGE);
    void *stageHeap = mempAllocBytesInBank(g_CurentMaMallocValue, MEMPOOL_STAGE);
    LOG_STEP("[GE007-CORE] 4.5 memaReset\n");
    memaReset(stageHeap, g_CurentMaMallocValue);
    LOG_STEP("[GE007-CORE] 4.6 reset_play_data_ptrs\n");
    reset_play_data_ptrs();
    LOG_STEP("[GE007-CORE] 4.7 init_player_data_ptrs_construct_viewports\n");
    init_player_data_ptrs_construct_viewports(0);
    LOG_STEP("[GE007-CORE] 4.8 dynInitMemory\n");
    dynInitMemory();
    LOG_STEP("[GE007-CORE] 4.9 joyCheckStatusThreadSafe\n");
    joyCheckStatusThreadSafe();

    LOG_STEP("[GE007-CORE] 5. Calling lvlStageLoad(LEVELID_TITLE)...\n");
    lvlStageLoad(LEVELID_TITLE);
    
    LOG_STEP("[GE007-CORE] 6. Refreshing VI buffers and debug menu...\n");
        debmenuRefresh();     waitForNextFrame();

    s_EngineInitialized = 1;
    s_FrameCount = 0;
    LOG_STEP("[GE007-CORE] Authentic GoldenEye 007 engine initialization complete!\n");
    return 0;
}

int hal_engine_step(void) {
    if (!s_EngineInitialized) {
        return -1;
    }

    waitForNextFrame();
    joyConsumeSamplesWrapper();

    Gfx *firstGdl = dynGetMasterDisplayList();
    Gfx *gdl = firstGdl;

    if (gdl != NULL) {
                shuffle_player_ids();

        gdl = lvlRender(gdl);

        gDPFullSync(gdl++);
        gSPEndDisplayList(gdl++);

        hal_gfx_execute_display_list(firstGdl, gdl);
    }

    dynSwapBuffers();
    s_FrameCount++;
    return 0;
}

int hal_engine_is_initialized(void) {
    return s_EngineInitialized;
}

uint32_t hal_engine_get_frame_count(void) {
    return s_FrameCount;
}

int32_t hal_engine_get_stage_num(void) {
    return g_StageNum;
}

const char *hal_get_build_info(void) {
    return "GoldenEye 007 (NTSC-U / USA) Clean Baseline Decompilation Web Port";
}