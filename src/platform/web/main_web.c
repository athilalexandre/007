#include <stdio.h>
#include <string.h>
#include <ultra64.h>


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
#include "game/player.h"
#include "game/gun.h"
#include "game/bondview.h"
#include "game/chr.h"
#include "game/chrai.h"

static int s_EngineInitialized = 0;
static uint32_t s_FrameCount = 0;

// Declarations of engine globals and internal functions
extern s32 g_StageNum;
extern s32 g_MainStageNum;
extern u32 g_CurentMaMallocValue;
extern struct player *g_CurrentPlayer;
extern s32 g_NumChrSlots;
extern ChrRecord *g_ChrSlots;
extern struct memallocstring memallocstringtable[];

extern void obBlankResourcesLoadedInBank(u8 bank);
extern void reset_play_data_ptrs(void);
extern void init_player_data_ptrs_construct_viewports(s32 playernum);
extern void lvlManageMpGame(void);
extern void shuffle_player_ids(void);
extern void lvlViewMoveTick(void);
extern void stop_demo_playback(void);
extern void lvlUnloadStageTextData(void);
extern void mempNullNextEntryInBank(u8 bank);
extern s32 getPlayerCount(void);
extern void set_cur_player(s32 n);
extern void viSetViewSize(s32 x, s32 y);
extern void viSetViewPosition(s32 x, s32 y);
extern void memaSingleDefragPass(void);

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
    extern void test_tex1(void);
    test_tex1();

    LOG_STEP("[GE007-CORE] 3. Setting stage to LEVELID_TITLE (90 / 0x5A)...\n");
    g_StageNum = LEVELID_TITLE;
    g_MainStageNum = LEVELID_NONE;

    LOG_STEP("[GE007-CORE] 4.1 tokenSetString\n");
    tokenSetString("-ml0 -me0 -mgfx80 -mvtx20 -mt646 -ma001");
    LOG_STEP("[GE007-CORE] 4.2 mempResetBank\n");
    mempResetBank(MEMPOOL_STAGE);
    LOG_STEP("[GE007-CORE] 4.3 obBlankResourcesLoadedInBank\n");
    obBlankResourcesLoadedInBank(MEMPOOL_STAGE);
    if (tokenFind(1, "-ma")) {
        g_CurentMaMallocValue = (s32)(strtol(tokenFind(1, "-ma"), NULL, 0) * 1024);
    }
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
    debmenuRefresh();
    waitForNextFrame();

    s_EngineInitialized = 1;
    s_FrameCount = 0;
    LOG_STEP("[GE007-CORE] Authentic GoldenEye 007 engine initialization complete!\n");
    return 0;
}

static void handle_stage_transition(void) {
    if (g_MainStageNum < 0) return;

    osSyncPrintf("[GE007-CORE] Stage Transition: 0x%02X -> 0x%02X\n", g_StageNum, g_MainStageNum);

    lvlUnloadStageTextData();
    stop_demo_playback();
    mempNullNextEntryInBank(MEMPOOL_STAGE);
    obBlankResourcesLoadedInBank(MEMPOOL_STAGE);

    g_StageNum = g_MainStageNum;
    g_MainStageNum = LEVELID_NONE;

    /* Apply authentic memory allocation token string for new stage */
    int stringIndex = 0;
    while (memallocstringtable[stringIndex].id) {
        if (memallocstringtable[stringIndex].id == g_StageNum) {
            break;
        }
        stringIndex++;
    }
    if (memallocstringtable[stringIndex].id != 0) {
        tokenSetString((const char *)memallocstringtable[stringIndex].string);
    } else {
        tokenSetString("-ml0 -me0 -mgfx100 -mvtx50 -mt700 -ma400");
    }

    mempResetBank(MEMPOOL_STAGE);
    obBlankResourcesLoadedInBank(MEMPOOL_STAGE);
    if (tokenFind(1, "-ma")) {
        g_CurentMaMallocValue = (s32)(strtol(tokenFind(1, "-ma"), NULL, 0) * 1024);
    }

    void *stageHeap = mempAllocBytesInBank(g_CurentMaMallocValue, MEMPOOL_STAGE);
    memaReset(stageHeap, g_CurentMaMallocValue);
    reset_play_data_ptrs();

    s32 numPlayers = (g_StageNum != LEVELID_TITLE) ? 1 : 0;
    init_player_data_ptrs_construct_viewports(numPlayers);
    dynInitMemory();
    joyCheckStatusThreadSafe();
    lvlStageLoad(g_StageNum);
    viInitBuffers();
    debmenuRefresh();
    waitForNextFrame();
}

int hal_engine_step(void) {
    if (!s_EngineInitialized) {
        return -1;
    }

    /* Check for stage transitions (e.g. Title -> Dam mission launch) */
    if (g_MainStageNum >= 0) {
        handle_stage_transition();
    }

    waitForNextFrame();
    joyConsumeSamplesWrapper();

    /* Manage authentic per-frame game, AI, and menu state updates */
    lvlManageMpGame();
    shuffle_player_ids();

    /* In gameplay stages, update viewports and player state (movement, looking, shooting, doors) */
    if (g_StageNum != LEVELID_TITLE) {
        for (s32 i = 0; i < getPlayerCount(); i++) {
            set_cur_player(get_nth_player_from_shuffled(i));
            if (g_CurrentPlayer) {
                viSetViewSize(g_CurrentPlayer->viewx, g_CurrentPlayer->viewy);
                viSetViewPosition(g_CurrentPlayer->viewleft, g_CurrentPlayer->viewtop);
                lvlViewMoveTick();
            }
        }
    }

    /* Construct authentic display list */
    Gfx *firstGdl = dynGetMasterDisplayList();
    Gfx *gdl = firstGdl;

    if (gdl != NULL) {
        gdl = lvlRender(gdl);

        gDPFullSync(gdl++);
        gSPEndDisplayList(gdl++);

        hal_gfx_execute_display_list(firstGdl, gdl);
    }

    dynSwapBuffers();
    memaSingleDefragPass();
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

/* Rich Telemetry Exports for Testing and Modern HUD */
float hal_player_get_pos_x(void) {
    return (g_CurrentPlayer && g_CurrentPlayer->prop) ? g_CurrentPlayer->prop->pos.x : 0.0f;
}

float hal_player_get_pos_y(void) {
    return (g_CurrentPlayer && g_CurrentPlayer->prop) ? g_CurrentPlayer->prop->pos.y : 0.0f;
}

float hal_player_get_pos_z(void) {
    return (g_CurrentPlayer && g_CurrentPlayer->prop) ? g_CurrentPlayer->prop->pos.z : 0.0f;
}

float hal_player_get_health(void) {
    return g_CurrentPlayer ? g_CurrentPlayer->bondhealth : 0.0f;
}

float hal_player_get_armor(void) {
    return g_CurrentPlayer ? g_CurrentPlayer->bondarmour : 0.0f;
}

int32_t hal_player_get_weapon(void) {
    return g_CurrentPlayer ? getCurrentPlayerWeaponId(GUNRIGHT) : 0;
}

int32_t hal_player_get_ammo(void) {
    return g_CurrentPlayer ? g_CurrentPlayer->hands[0].weapon_ammo_in_magazine : 0;
}

int32_t hal_engine_get_guard_count(void) {
    return g_NumChrSlots;
}

uint32_t hal_engine_get_telemetry_json(char *buf, u32 maxlen) {
    if (!buf || maxlen == 0) return 0;
    float px = hal_player_get_pos_x();
    float py = hal_player_get_pos_y();
    float pz = hal_player_get_pos_z();
    float health = hal_player_get_health();
    float armor = hal_player_get_armor();
    int32_t wep = hal_player_get_weapon();
    int32_t ammo = hal_player_get_ammo();
    int32_t guards = hal_engine_get_guard_count();
    uint32_t dmaTransfers = hal_os_get_dma_transfers();
    uint32_t dmaBytes = hal_os_get_dma_bytes();

    return (uint32_t)snprintf(buf, maxlen,
        "{\"stage\":%d,\"frame\":%u,\"pos\":[%.2f,%.2f,%.2f],\"health\":%.2f,\"armor\":%.2f,\"weapon\":%d,\"ammo\":%d,\"guards\":%d,\"dma_transfers\":%u,\"dma_bytes\":%u}",
        g_StageNum, s_FrameCount, px, py, pz, health, armor, wep, ammo, guards, dmaTransfers, dmaBytes
    );
}
#include "src/game/image.h"
extern struct texpool ptr_texture_alloc_start;

void test_tex1(void) {
    printf("[test_tex1] testing texLoad for texture 1 (X)...\n");
    s32 texword = 1;
    texLoad(&texword, NULL);
    printf("[test_tex1] test complete! tex1 word=0x%x\n", texword);
    fflush(stdout);
}
