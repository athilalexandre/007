#include <ultra64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static u8 *g_RomBuffer = NULL;
static u32 g_RomSize = 0;
static u32 g_RomValid = 0;

void osInitialize(void) {
}

void hal_os_set_rom_data(const u8 *data, u32 size) {
    if (g_RomBuffer) {
        free(g_RomBuffer);
        g_RomBuffer = NULL;
        g_RomSize = 0;
        g_RomValid = 0;
    }
    if (data && size >= 4096) {
        g_RomBuffer = (u8 *)malloc(size);
        if (g_RomBuffer) {
            memcpy(g_RomBuffer, data, size);
            g_RomSize = size;
            g_RomValid = 1;
        }
    }
}

u32 hal_os_get_rom_status(void) {
    return g_RomValid;
}

void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msg, s32 count) {
    if (!mq) return;
    mq->msg = msg;
    mq->msgCount = count;
    mq->validCount = 0;
    mq->first = 0;
}

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag) {
    if (!mq || mq->validCount >= mq->msgCount) return -1;
    s32 index = (mq->first + mq->validCount) % mq->msgCount;
    mq->msg[index] = msg;
    mq->validCount++;
    return 0;
}

s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flag) {
    if (!mq || mq->validCount <= 0) return -1;
    if (msg) {
        *msg = mq->msg[mq->first];
    }
    mq->first = (mq->first + 1) % mq->msgCount;
    mq->validCount--;
    return 0;
}

void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri p) {
}

void osStartThread(OSThread *t) {
}

void osSetThreadPri(OSThread *t, OSPri p) {
}

OSPri osGetThreadPri(OSThread *t) {
    return 0;
}

OSTime osGetTime(void) {
    static OSTime s_FakeTime = 0;
    s_FakeTime += 1000000;
    return s_FakeTime;
}

void osSetTime(OSTime t) {
}

void osSetTimer(OSTimer *t, OSTime countdown, OSTime interval, OSMesgQueue *mq, OSMesg msg) {
    if (mq) {
        osSendMesg(mq, msg, OS_MESG_NOBLOCK);
    }
}

void osStopTimer(OSTimer *t) {
}

void osInvalDCache(void *vaddr, s32 nbytes) {
}

void osInvalICache(void *vaddr, s32 nbytes) {
}

void osWritebackDCache(void *vaddr, s32 nbytes) {
}

void osWritebackDCacheAll(void) {
}

s32 osPiStartDma(OSIoMesg *mb, s32 priority, s32 direction, u32 devAddr, void *vAddr, u32 nbytes, OSMesgQueue *mq) {
    if (direction == OS_READ && vAddr) {
        u32 romOffset = devAddr;
        if (romOffset >= 0x10000000) {
            romOffset -= 0x10000000;
        }
        if (g_RomBuffer && g_RomValid && (romOffset + nbytes <= g_RomSize)) {
            memcpy(vAddr, g_RomBuffer + romOffset, nbytes);
        } else {
            memset(vAddr, 0, nbytes);
        }
    }
    if (mq) {
        osSendMesg(mq, (OSMesg)mb, OS_MESG_NOBLOCK);
    }
    return 0;
}

s32 osPiRawStartDma(s32 dir, u32 devAddr, void *dramAddr, size_t size) {
    return osPiStartDma(NULL, OS_MESG_PRI_NORMAL, dir, devAddr, dramAddr, size, NULL);
}

void osSyncPrintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

s32 osEepromProbe(OSMesgQueue *mq) {
    return EEPROM_TYPE_4K;
}

s32 osEepromRead(OSMesgQueue *mq, u8 address, u8 *buffer) {
    memset(buffer, 0, 8);
    return 0;
}

s32 osEepromWrite(OSMesgQueue *mq, u8 address, u8 *buffer) {
    return 0;
}

s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes) {
    memset(buffer, 0, nbytes);
    return 0;
}

s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes) {
    return 0;
}