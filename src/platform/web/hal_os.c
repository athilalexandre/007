/**
 * GoldenEye 007 Web Port ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â OS Abstraction Layer Implementation
 * Real implementations of N64 libultra OS functions for Emscripten/WASM.
 */
#ifdef TARGET_WEB

#include <ultra64.h>
#include <PR/os.h>
#include "sched.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* Global N64 Hardware & OS Variables */
u32 osTvType = OS_TV_NTSC;
u64 osClockRate = 62500000;
u8  _bssSegmentEnd[32];
s32 g_osClockRate = 62500000;

/* Microcode / Segment Symbols */
u32 rspbootTextStart = 0;
u32 gsp3DTextStart = 0;
u32 aspMainTextStart = 0;
u32 aspMainDataStart = 0;

void permit_stderr(u32 flag) { }

/* Fault Handler & Thread Queue Stubs */
void* __osRunQueue = NULL;
void* __osGetCurrFaultedThread(void) { return NULL; }
void  __osEnqueueThread(void *queue, void *thread) { }
void  osSetEventMesg(OSEvent event, OSMesgQueue *mq, OSMesg msg) { }
OSIntMask osSetIntMask(OSIntMask mask) { return 0; }
void  osYieldThread(void) { }

/* ROM data buffer ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â loaded from user-provided .z64 file */
static u8 *g_romData = NULL;
static u32 g_romSize = 0;

/* Timing */
static u64 g_osTimeBase = 0;
static u32 g_osCountBase = 0;

/* === Thread stubs (cooperative on web) === */
void osCreateThread(OSThread *t, s32 id, void (*entry)(void*), void *arg, void *sp, s32 pri) {
    if (t) {
        t->id = id;
        t->priority = pri;
        t->state = 0;
    }
}

void osStartThread(OSThread *t) { }
void osStopThread(OSThread *t) { }
void osSetThreadPri(OSThread *t, OSPri pri) { }
s32  osGetThreadPri(OSThread *t) { return t ? t->priority : 0; }
void osDestroyThread(OSThread *t) { }

/* === Message Queue stubs === */
void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msg, s32 count) {
    if (mq) {
        mq->msg = msg;
        mq->msgCount = count;
        mq->validCount = 0;
        mq->first = 0;
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
    return (OSTime)(emscripten_get_now() * 46875.0);
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

int osSetTimer(OSTimer *t, OSTime countdown, OSTime interval, OSMesgQueue *mq, OSMesg msg) {
    return 0;
}

/* === Init === */
void osInitialize(void) {
    osTvType = OS_TV_NTSC;
    printf("[HAL_OS] osInitialize() ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â Hardware abstraction layer ready\n");
}

/* === Cache / TLB === */
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

/* === PI (ROM Direct DMA Access) === */
void hal_os_set_rom_data(u8 *data, u32 size) {
    g_romData = data;
    g_romSize = size;
    printf("[HAL_OS] ROM buffer registered: %u bytes (%u MB)\n", size, size / (1024 * 1024));
}

s32 osPiRawStartDma(s32 direction, u32 romAddr, void *ramAddr, u32 size) {
    if (direction == OS_READ && g_romData) {
        u32 offset = romAddr;
        if (offset >= 0x10000000) offset -= 0x10000000;

        if (offset + size <= g_romSize) {
            memcpy(ramAddr, g_romData + offset, size);
            return 0;
        }
    }
    return -1;
}

u32 osPiGetStatus(void) {
    return 0;
}

s32 osPiStartDma(OSIoMesg *ioMesg, s32 priority, s32 direction,
                 u32 devAddr, void *dramAddr, u32 size, OSMesgQueue *mq) {
    s32 ret = osPiRawStartDma(direction, devAddr, dramAddr, size);
    if (mq) {
        osSendMesg(mq, NULL, OS_MESG_NOBLOCK);
    }
    return ret;
}

void osCreatePiManager(s32 pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf, s32 cmdMsgCnt) {
}

/* === VI (Video Interface) === */
void osViSetMode(OSViMode *mode) { }
void osViSwapBuffer(void *frameBufPtr) { }
void osViSetSpecialFeatures(u32 func) { }
void osViBlack(u8 on) { }

/* === Scheduler stubs === */
void osCreateScheduler(OSSched *s, void *stack, u8 mode, u32 numFields) { }
void osScAddClient(OSSched *s, OSScClient *c, OSMesgQueue *msgQ, OSScClient *next) { }
OSMesgQueue *osScGetCmdQ(OSSched *s) {
    static OSMesgQueue dummyQ;
    static OSMesg dummyBuf[4];
    osCreateMesgQueue(&dummyQ, dummyBuf, 4);
    return &dummyQ;
}

/* === Controller / Joypad (Supports up to 4 Players) === */
static OSContPad g_webContPad[4];

s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data) {
    if (bitpattern) *bitpattern = 0x0F; /* 4 Controllers connected */
    memset(g_webContPad, 0, sizeof(g_webContPad));
    return 0;
}

s32 osContStartReadData(OSMesgQueue *mq) {
    return 0;
}

void osContGetReadData(OSContPad *pad) {
    memcpy(pad, g_webContPad, sizeof(OSContPad) * 4);
}

void hal_input_set_pad(s32 port, u16 buttons, s8 stick_x, s8 stick_y) {
    if (port >= 0 && port < 4) {
        g_webContPad[port].button = buttons;
        g_webContPad[port].stick_x = stick_x;
        g_webContPad[port].stick_y = stick_y;
    }
}

/* === EEPROM (Save Data) === */
s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, s32 nbytes) {
    memset(buffer, 0, nbytes);
    return 0;
}

s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, s32 nbytes) {
    return 0;
}

#endif /* TARGET_WEB */
/* === VI Mode Table Symbols === */
OSViMode osViModeTable[64];
f32 g_ViXScales[2] = {1.0f, 1.0f};
f32 g_ViYScales[2] = {1.0f, 1.0f};
OSViMode* g_ViModePtrs[2] = {0, 0};
OSViMode g_ViModes[2];

void osViSetYScale(f32 scale) { }
/* Scheduler client */
OSScClient gfxClient[3];

/* Controller Query & PFS (Controller Pak) & Rumble Pak Stubs */
s32 osContStartQuery(OSMesgQueue *mq) { return 0; }
void osContGetQuery(OSContStatus *data) { }
s32 osPfsInit(OSMesgQueue *mq, OSPfs *pfs, s32 channel) { return 0; }
s32 osEepromProbe(OSMesgQueue *mq) { return 1; }

s32 __osContRamWrite(OSMesgQueue *mq, s32 channel, u16 address, u8 *buffer, s32 force) { return 0; }
s32 __osContRamRead(OSMesgQueue *mq, s32 channel, u16 address, u8 *buffer) { return 0; }
u8 __osContAddressCrc(u16 addr) { return 0; }
s32 g_ViChangeVideoModes[2] = {0, 0};
void _Printf(const char *fmt, ...) { }