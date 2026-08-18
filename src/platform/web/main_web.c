/**
 * GoldenEye 007 Web Port - Engine Boot & Per-Frame Adapter
 *
 * New adapter symbols (not present in n64decomp/007 upstream c4356466):
 *   hal_engine_init        - global subsystem init
 *   hal_engine_load_stage  - stage setup (extracted from bossMainloop:331-443)
 *   hal_engine_step        - per-frame tick (extracted from bossMainloop:480-)
 *   web_load_rom           - JS entry point
 */
#ifdef TARGET_WEB

#include <ultra64.h>
#include <PR/os.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "bondconstants.h"
#include "boss.h"
#include "memp.h"
#include "mema.h"
#include "fr.h"
#include "joy.h"
#include "game/dyn.h"
#include "game/lv.h"
#include "game/frametiming.h"
#include "debugmenu.h"

extern s32 g_CurentMaMallocValue;
extern void obBlankResourcesLoadedInBank(u8 bank);
extern void reset_play_data_ptrs(void);

extern void hal_os_set_rom_data(u8 *data, u32 size);
extern void hal_gfx_init(s32 width, s32 height);
extern void hal_gfx_start_frame(void);
extern void hal_gfx_process_display_list(Gfx *dl);
extern void hal_gfx_end_frame(void);
extern u32 *hal_get_framebuffer(void);

extern s32 g_StageNum;

static int g_engineInitialized = 0;
static int g_stageLoaded       = 0;
static u32 g_frameTick         = 0;

/* NEW ADAPTER: Global subsystem initialization */
void EMSCRIPTEN_KEEPALIVE hal_engine_init(void) {
    printf("[GE007] hal_engine_init: bossInitMainthreadData()...\n");
    /* bossInitMainthreadData() calls mempInit() internally at boss.c:172.
     * Do NOT call mempInit() separately before this. */
    bossInitMainthreadData();
    rspAllocateBuffers();
    hal_gfx_init(320, 240);
    g_engineInitialized = 1;
    printf("[GE007] hal_engine_init: done. Stage=%d\n", g_StageNum);
}

/* NEW ADAPTER: Stage setup extracted from bossMainloop:331-443 */
void EMSCRIPTEN_KEEPALIVE hal_engine_load_stage(s32 stage) {
    if (!g_engineInitialized) return;
    printf("[GE007] Loading stage %d (LEVELID_TITLE=%d)\n", stage, LEVELID_TITLE);
    g_StageNum = stage;

    reset_mem_bank_5();                           /* bossMainloop:331 */
    mempResetBank(MEMPOOL_STAGE);                 /* bossMainloop:416 - param is u8 */
    obBlankResourcesLoadedInBank(MEMPOOL_STAGE);
    memaReset(mempAllocBytesInBank(g_CurentMaMallocValue, MEMPOOL_STAGE), g_CurentMaMallocValue);
    reset_play_data_ptrs();
    init_player_data_ptrs_construct_viewports(0); /* 0 players for LEVELID_TITLE */
    dynInitMemory();                              /* bossMainloop:437 - once per stage, not per frame */
    joyCheckStatusThreadSafe();
    lvlStageLoad(stage);
    viInitBuffers();
    debmenuRefresh();
    waitForNextFrame();
    speedgraphMarkerCommit();

    g_stageLoaded = 1;
    printf("[GE007] Stage load complete.\n");
}

/* NEW ADAPTER: Per-frame tick extracted from bossMainloop:480- */
void EMSCRIPTEN_KEEPALIVE hal_engine_step(void) {
    Gfx *gdl, *firstGdl;
    if (!g_engineInitialized || !g_stageLoaded) return;
    g_frameTick++;
    hal_gfx_start_frame();

    waitForNextFrame();                /* bossMainloop:486 */
    speedgraphRenderGraph();
    speedgraphMarkerCommit();
    speedgraphMarkerHandler(0x20000);
    joyConsumeSamplesWrapper();        /* bossMainloop:492 */

    gdl = firstGdl = dynGetMasterDisplayList(); /* bossMainloop:495 */

    lvlManageMpGame();                 /* bossMainloop:519 */
    shuffle_player_ids();

    /* bossMainloop:522: if (g_StageNum != LEVELID_TITLE) { lvlViewMoveTick() }
     * We are always LEVELID_TITLE in proof-of-life. Do NOT call lvlViewMoveTick. */

    gdl = lvlRender(gdl);             /* bossMainloop:538 */
    gdl = debmenuDraw(gdl);           /* bossMainloop:553 */

    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);

    hal_gfx_process_display_list(firstGdl);

    dynSwapBuffers();
    hal_gfx_end_frame();
}

void EMSCRIPTEN_KEEPALIVE web_load_rom(u8 *data, u32 size) {
    printf("[GE007] ROM: %u bytes (SHA-1 validated by JS)\n", size);
    osInitialize();
    hal_os_set_rom_data(data, size);
    hal_engine_init();
    hal_engine_load_stage(LEVELID_TITLE);
}

int main(int argc, char *argv[]) {
    printf("[GE007] GoldenEye 007 WebAssembly Core - n64decomp/007 (US 1.0)\n");
    return 0;
}

#endif /* TARGET_WEB */