/**
 * GoldenEye 007 Web Port — OS Abstraction Layer Implementation
 * Stub implementations of N64 libultra OS functions for Emscripten/WASM.
 */
#ifdef TARGET_WEB

#include "hal_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* === Global State === */
s32 osTvType = OS_TV_NTSC;

/* ROM data buffer — loaded from user-provided .z64 file */
static u8 *g_romData = NULL;
static u32 g_romSize = 0;

/* Timing */
static u64 g_osTimeBase = 0;
static u32 g_osCountBase = 0;

/* === Thread stubs (single-threaded on web) === */
void osCreateThread(OSThread *t, s32 id, void (*entry)(void*), void *arg, void *sp, s32 pri) {
    if (t) {
        t->id = id;
        t->priority = pri;
        t->state = 0;
    }
    /* In the real port, the main thread entry will be called directly */
}

void osStartThread(OSThread *t) {
    /* No-op: threads don't exist on web, functions are called directly */
}

void osStopThread(s32 id) { }
void osSetThreadPri(s32 id, s32 pri) { }
s32  osGetThreadPri(OSThread *t) { return t ? t->priority : 0; }
void osDestroyThread(OSThread *t) { }

/* === Message Queue stubs === */
void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msg, s32 count) {
    if (mq) {
        mq->msg = msg;
        mq->msgCount = count;
        mq->validCount = 0;
        mq->first = 0;
        mq->mtqueue = NULL;
        mq->fullqueue = NULL;
    }
}

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flags) {
    if (!mq) return -1;
    if (mq->validCount < mq->msgCount) {
        s32 idx = (mq->first + mq->validCount) % mq->msgCount;
        mq->msg[idx] = msg;
        mq->validCount++;
        return 0;
    }
    return -1;
}

s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flags) {
    if (!mq || mq->validCount == 0) return -1;
    if (msg) {
        *msg = mq->msg[mq->first];
    }
    mq->first = (mq->first + 1) % mq->msgCount;
    mq->validCount--;
    return 0;
}

s32 osJamMesg(OSMesgQueue *mq, OSMesg msg, s32 flags) {
    if (!mq || mq->validCount >= mq->msgCount) return -1;
    mq->first = (mq->first - 1 + mq->msgCount) % mq->msgCount;
    mq->msg[mq->first] = msg;
    mq->validCount++;
    return 0;
}

/* === Timing === */
OSTime osGetTime(void) {
#ifdef __EMSCRIPTEN__
    return (OSTime)(emscripten_get_now() * 46875.0); /* Approximate N64 clock ticks */
#else
    return 0;
#endif
}

u32 osGetCount(void) {
#ifdef __EMSCRIPTEN__
    return (u32)(emscripten_get_now() * 46875.0);
#else
    return 0;
#endif
}

void osSetTimer(void *t, OSTime countdown, OSTime interval, OSMesgQueue *mq, OSMesg msg) {
    /* TODO: implement with emscripten_set_timeout if needed */
}

/* === Init === */
void osInitialize(void) {
    osTvType = OS_TV_NTSC;
    printf("[HAL_OS] osInitialize() — Web platform ready\n");
}

/* === Cache / TLB — No-ops on web === */
void osWritebackDCacheAll(void) { }
void osWritebackDCache(void *vaddr, s32 len) { }
void osInvalDCache(void *vaddr, s32 len) { }
void osInvalICache(void *vaddr, s32 len) { }
void osUnmapTLB(s32 index) { }
void osMapTLB(s32 index, u32 pageSize, void *vaddr, u32 even, u32 odd, s32 asid) { }

/* === FPC === */
static u32 g_fpcCsr = 0;
u32 __osGetFpcCsr(void) { return g_fpcCsr; }
u32 __osSetFpcCsr(u32 flags) { u32 old = g_fpcCsr; g_fpcCsr = flags; return old; }

/* === PI (ROM Access) === */
void hal_os_set_rom_data(u8 *data, u32 size) {
    g_romData = data;
    g_romSize = size;
    printf("[HAL_OS] ROM data set: %u bytes\n", size);
}

s32 osPiRawStartDma(s32 direction, u32 romAddr, void *ramAddr, u32 size) {
    if (direction == OS_READ && g_romData && romAddr + size <= g_romSize) {
        memcpy(ramAddr, g_romData + romAddr, size);
        return 0;
    }
    return -1;
}

s32 osPiGetStatus(void) {
    return 0; /* Never busy — DMA is instant (synchronous memcpy) */
}

s32 osPiStartDma(void *ioMesg, s32 priority, s32 direction,
                 u32 devAddr, void *dramAddr, u32 size, OSMesgQueue *mq) {
    s32 ret = osPiRawStartDma(direction, devAddr, dramAddr, size);
    if (mq) {
        osSendMesg(mq, NULL, OS_MESG_NOBLOCK);
    }
    return ret;
}

void osCreatePiManager(s32 pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf, s32 cmdMsgCnt) {
    /* No-op on web — PI DMA is synchronous */
}

/* === VI (Video Interface) === */
void osViSetMode(void *mode) { }
void osViSwapBuffer(void *frameBufPtr) {
    /* TODO: trigger WebGL present / requestAnimationFrame */
}
void osViSetSpecialFeatures(u32 func) { }
void osViBlack(s32 on) { }

/* === Scheduler stubs === */
void osCreateScheduler(OSSched *s, OSThread *t, void *mode, s32 numFields) { }
void osScAddClient(OSSched *s, OSScClient *c, OSMesgQueue *mq, void *data) { }
OSMesgQueue *osScGetCmdQ(OSSched *s) {
    static OSMesgQueue dummyQ;
    static OSMesg dummyBuf[4];
    osCreateMesgQueue(&dummyQ, dummyBuf, 4);
    return &dummyQ;
}

/* === Controller / Joypad === */
static OSContPad g_webContPad[4];

void osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data) {
    if (bitpattern) *bitpattern = 0x01; /* Controller 1 connected */
    memset(g_webContPad, 0, sizeof(g_webContPad));
}

void osContStartReadData(OSMesgQueue *mq) {
    /* Input is polled from JS side and written to g_webContPad */
}

void osContGetReadData(OSContPad *pad) {
    memcpy(pad, g_webContPad, sizeof(OSContPad) * 4);
}

/* Called from JavaScript to update controller state */
void hal_input_set_pad(s32 port, u16 buttons, s8 stick_x, s8 stick_y) {
    if (port >= 0 && port < 4) {
        g_webContPad[port].button = buttons;
        g_webContPad[port].stick_x = stick_x;
        g_webContPad[port].stick_y = stick_y;
    }
}

/* === EEPROM (Save Data) — use localStorage via JS === */
s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, s32 nbytes) {
    memset(buffer, 0, nbytes);
    /* TODO: read from localStorage via EM_ASM */
    return 0;
}

s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, s32 nbytes) {
    /* TODO: write to localStorage via EM_ASM */
    return 0;
}

#endif /* TARGET_WEB */
