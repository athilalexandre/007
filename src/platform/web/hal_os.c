#include <stdlib.h>
#include <string.h>
#include <ultra64.h>
#include <stdio.h>
#include "hal_os.h"
#include "rom_resolver.h"

#define WEB_HEAP_TOTAL_SIZE (16 * 1024 * 1024)

u8 _bssSegmentEnd[WEB_HEAP_TOTAL_SIZE] __attribute__((aligned(16)));

static u8 *s_RomBuffer = NULL;
static size_t s_RomBufferSize = 0;
static u32 s_RomStatus = 0; /* 0: None, 1: Valid & Mounted, 2: Invalid */
static u32 s_DmaTransferCount = 0;
static size_t s_DmaTotalBytesRead = 0;

void hal_os_set_rom_data(const u8 *data, size_t size) {
    if (s_RomBuffer) {
        free(s_RomBuffer);
        s_RomBuffer = NULL;
        s_RomBufferSize = 0;
        s_RomStatus = 0;
    }

    if (!data || size < 12582912) {
        s_RomStatus = 2; /* Invalid */
        return;
    }

    if (!rom_resolver_validate_layout(data, size)) {
        s_RomStatus = 2; /* Layout validation failure */
        return;
    }

    s_RomBuffer = (u8 *)malloc(size);
    if (!s_RomBuffer) {
        s_RomStatus = 2; /* OOM */
        return;
    }

    memcpy(s_RomBuffer, data, size);
    s_RomBufferSize = size;
    s_RomStatus = 1; /* Valid & Ready */
    s_DmaTransferCount = 0;
    s_DmaTotalBytesRead = 0;
}

u32 hal_os_get_rom_status(void) {
    return s_RomStatus;
}

size_t hal_os_get_rom_size(void) {
    return s_RomBufferSize;
}

const u8 *hal_os_get_rom_buffer(void) {
    return s_RomBuffer;
}

void hal_os_reset(void) {
    if (s_RomBuffer) {
        free(s_RomBuffer);
        s_RomBuffer = NULL;
        s_RomBufferSize = 0;
    }
    s_RomStatus = 0;
    s_DmaTransferCount = 0;
    s_DmaTotalBytesRead = 0;
    memset(_bssSegmentEnd, 0, sizeof(_bssSegmentEnd));
}

/* Libultra PI DMA implementation */
s32 osPiStartDma(OSIoMesg *mb, s32 priority, s32 direction, u32 devAddr, void *dramAddr, u32 size, OSMesgQueue *mq) {
    if (direction != OS_READ || !dramAddr || size == 0) {
        return -1;
    }

    if (!s_RomBuffer || s_RomStatus != 1) {
        return -1;
    }

    /* Bounds and overflow check */
    if (devAddr >= s_RomBufferSize || (devAddr + size) > s_RomBufferSize || (devAddr + size) < devAddr) {
        return -1;
    }

    memcpy(dramAddr, s_RomBuffer + devAddr, size);
    s_DmaTransferCount++;
    s_DmaTotalBytesRead += size;

    if (mq) {
        osSendMesg(mq, (OSMesg)0, OS_MESG_NOBLOCK);
    }
    return 0;
}

s32 osEPiStartDma(OSPiHandle *han, OSIoMesg *mb, s32 direction) {
    if (!mb) return -1;
    return osPiStartDma(mb, mb->hdr.pri, direction, mb->devAddr, mb->dramAddr, mb->size, mb->hdr.retQueue);
}

s32 osPiRawStartDma(s32 direction, u32 devAddr, void *dramAddr, u32 size) {
    return osPiStartDma(NULL, 0, direction, devAddr, dramAddr, size, NULL);
}

/* Message Queue Implementation */
void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msgBuf, s32 count) {
    if (!mq) return;
    mq->mtqueue = NULL;
    mq->fullqueue = NULL;
    mq->validCount = 0;
    mq->first = 0;
    mq->msgCount = count;
    mq->msg = msgBuf;
}

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag) {
    if (!mq || !mq->msg) return -1;
    if (mq->validCount >= mq->msgCount) {
        return -1;
    }
    s32 last = (mq->first + mq->validCount) % mq->msgCount;
    mq->msg[last] = msg;
    mq->validCount++;
    return 0;
}

s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flag) {
    if (!mq || !mq->msg || mq->validCount == 0) {
        return -1;
    }
    if (msg) {
        *msg = mq->msg[mq->first];
    }
    mq->first = (mq->first + 1) % mq->msgCount;
    mq->validCount--;
    return 0;
}

int osSetTimer(OSTimer *t, OSTime countdown, OSTime interval, OSMesgQueue *mq, OSMesg msg) {
    if (mq) {
        osSendMesg(mq, msg, OS_MESG_NOBLOCK);
    }
    return 0;
}

int osStopTimer(OSTimer *t) {
    return 0;
}

static u32 s_VirtualCycleCount = 1000;
OSTime osGetTime(void) {
    s_VirtualCycleCount += 15625;
    return s_VirtualCycleCount;
}

u32 osGetCount(void) {
    s_VirtualCycleCount += 15625;
    return s_VirtualCycleCount;
}

void osInvalDCache(void *vaddr, s32 nbytes) {}
void osInvalICache(void *vaddr, s32 nbytes) {}
void osWritebackDCache(void *vaddr, s32 nbytes) {}
void osWritebackDCacheAll(void) {}

u32 osVirtualToPhysical(void *vaddr) {
    return (u32)(uintptr_t)vaddr;
}
void *setSPToEnd(u8 *stack, u32 size) { return stack ? stack + size : NULL; }
#include <stdarg.h>
void osSyncPrintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
u64 osClockRate = 62500000;
static OSThread s_MainThread;
OSThread *__osRunQueue = &s_MainThread;

void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri p) {}
void osStartThread(OSThread *t) {}
void osYieldThread(void) {}
void osSetEventMesg(OSEvent e, OSMesgQueue *mq, OSMesg msg) {}
OSIntMask osSetIntMask(OSIntMask mask) { return 0; }
OSThread *__osGetCurrFaultedThread(void) { return &s_MainThread; }
void __osEnqueueThread(OSThread **queue, OSThread *t) {}
u32 __osGetTLBHi(s32 index) { return 0; }
void osUnmapTLB(s32 index) {}
u32 tlbRandomGetNext(void) { return 0; }
void osWriteHost(void *dramAddr, u32 nbytes) {}
void osReadHost(void *dramAddr, u32 nbytes) {}
s32 rmonGetToken(void) { return 0; }
