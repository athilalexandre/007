#include <ultra64.h>

extern void *malloc(size_t);
extern void free(void *);
extern void *memset(void *, int, size_t);
extern void *memcpy(void *, const void *, size_t);

// Standard N64 hardware symbols
u64 osClockRate = 62500000;
#define WEB_HEAP_TOTAL_SIZE (16 * 1024 * 1024)
u8 _bssSegmentEnd[WEB_HEAP_TOTAL_SIZE];
#define g_WebEngineHeap _bssSegmentEnd
OSThread *__osRunQueue = NULL;

static u8 *g_RomBuffer = NULL;
static size_t g_RomBufferSize = 0;
static u32 g_RomStatus = 0; // 0 = uninit, 1 = loaded

void hal_os_set_rom_data(const u8 *data, size_t size) {
    if (!data || size == 0) {
        return;
    }
    if (g_RomBuffer) {
        free(g_RomBuffer);
        g_RomBuffer = NULL;
        g_RomBufferSize = 0;
    }
    g_RomBuffer = (u8 *)malloc(size);
    if (g_RomBuffer) {
        memcpy(g_RomBuffer, data, size);
        g_RomBufferSize = size;
        g_RomStatus = 1;
    }
}

u32 hal_os_get_rom_status(void) {
    return g_RomStatus;
}

u32 osVirtualToPhysical(void *addr) {
    return (u32)(uintptr_t)addr & 0x1FFFFFFF;
}

void osInvalDCache(void *vaddr, s32 nbytes) {
}

void osInvalICache(void *vaddr, s32 nbytes) {
}

void osWritebackDCache(void *vaddr, s32 nbytes) {
}

void osWritebackDCacheAll(void) {
}

// Minimal OS thread / scheduler stubs
void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri p) {
    if (t) {
        t->id = id;
        t->priority = p;
    }
}

void osStartThread(OSThread *t) {
}

void osYieldThread(void) {
}

void osSetThreadPri(OSThread *t, OSPri pri) {
    if (t) {
        t->priority = pri;
    }
}

OSPri osGetThreadPri(OSThread *t) {
    return t ? t->priority : 0;
}

OSId osGetThreadId(OSThread *t) {
    return t ? t->id : 0;
}

OSIntMask osSetIntMask(OSIntMask mask) {
    return 0;
}

OSThread *__osGetCurrFaultedThread(void) {
    return NULL;
}

void __osEnqueueThread(OSThread **queue, OSThread *t) {
}

void osSetEventMesg(OSEvent e, OSMesgQueue *mq, OSMesg msg) {
}

void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msgBuf, s32 count) {
    if (mq) {
        mq->mtqueue = NULL;
        mq->fullqueue = NULL;
        mq->validCount = 0;
        mq->first = 0;
        mq->msgCount = count;
        mq->msg = msgBuf;
    }
}

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag) {
    if (!mq || mq->validCount >= mq->msgCount) {
        return -1;
    }
    s32 last = (mq->first + mq->validCount) % mq->msgCount;
    mq->msg[last] = msg;
    mq->validCount++;
    return 0;
}

s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flag) {
    if (!mq || mq->validCount == 0) {
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
    if (t) {
        t->mq = mq;
        t->msg = msg;
    }
    if (mq) {
        osSendMesg(mq, msg, OS_MESG_NOBLOCK);
    }
    return 0;
}

int osStopTimer(OSTimer *t) {
    return 0;
}

OSTime osGetTime(void) {
    return 0;
}

void osSetTime(OSTime time) {
}

// Authentic ROM DMA simulation: copy requested segment from loaded user ROM buffer
s32 osEPiStartDma(OSPiHandle *han, OSIoMesg *mb, s32 direction) {
    if (!mb) return -1;
    u32 devAddr = mb->devAddr;
    void *dramAddr = mb->dramAddr;
    u32 size = mb->size;

    if (direction == OS_READ && dramAddr && size > 0 && g_RomBuffer) {
        if (devAddr + size <= g_RomBufferSize) {
            memcpy(dramAddr, g_RomBuffer + devAddr, size);
        }
    }
    if (mb->hdr.retQueue) {
        osSendMesg(mb->hdr.retQueue, NULL, OS_MESG_NOBLOCK);
    }
    return 0;
}

s32 osPiStartDma(OSIoMesg *mb, s32 priority, s32 direction, u32 devAddr, void *dramAddr, u32 size, OSMesgQueue *mq) {
    if (direction == OS_READ && dramAddr && size > 0 && g_RomBuffer) {
        if (devAddr + size <= g_RomBufferSize) {
            memcpy(dramAddr, g_RomBuffer + devAddr, size);
        }
    }
    if (mq) {
        osSendMesg(mq, NULL, OS_MESG_NOBLOCK);
    }
    return 0;
}

s32 osPiRawStartDma(s32 dir, u32 devAddr, void *dramAddr, u32 size) {
    if (dir == OS_READ && dramAddr && size > 0) {
        if (g_RomBuffer && (devAddr + size <= g_RomBufferSize)) {
            memcpy(dramAddr, g_RomBuffer + devAddr, size);
        } else {
            memset(dramAddr, 0, size);
        }
    }
    return 0;
}

s32 osPiRawReadIo(u32 devAddr, u32 *data) {
    if (data) *data = 0;
    return 0;
}

s32 osPiRawWriteIo(u32 devAddr, u32 data) {
    return 0;
}

s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *status) {
    if (bitpattern) *bitpattern = 0x01; // Controller 1 connected
    if (status) {
        memset(status, 0, sizeof(OSContStatus) * 4);
        status[0].type = CONT_TYPE_NORMAL;
        status[0].status = 0;
        status[0].errno = 0;
    }
    return 0;
}

s32 osContStartQuery(OSMesgQueue *mq) {
    if (mq) osSendMesg(mq, NULL, OS_MESG_NOBLOCK);
    return 0;
}

void osContGetQuery(OSContStatus *status) {
    if (status) {
        memset(status, 0, sizeof(OSContStatus) * 4);
        status[0].type = CONT_TYPE_NORMAL;
    }
}

s32 osContStartReadData(OSMesgQueue *mq) {
    if (mq) osSendMesg(mq, NULL, OS_MESG_NOBLOCK);
    return 0;
}

void osContGetReadData(OSContPad *pad) {
    if (pad) {
        memset(pad, 0, sizeof(OSContPad) * 4);
    }
}

s32 osPfsInit(OSMesgQueue *mq, OSPfs *pfs, s32 channel) {
    return 0;
}

s32 __osContRamWrite(OSMesgQueue *mq, s32 channel, u16 address, u8 *buffer, s32 force) {
    return 0;
}

s32 __osContRamRead(OSMesgQueue *mq, s32 channel, u16 address, u8 *buffer) {
    return 0;
}

u8 __osContAddressCrc(u16 addr) {
    return 0;
}

// TLB Stubs for 32-bit flat memory WebAssembly environment
u32 __osGetTLBHi(s32 index) {
    return 0;
}

void osUnmapTLB(s32 index) {
}

s32 tlbRandomGetNext(void) {
    return 0;
}
u32 osGetCount(void) {
    return 0;
}