#ifndef HAL_OS_H
#define HAL_OS_H

/**
 * GoldenEye 007 Web Port — OS Abstraction Layer
 * Replaces N64 libultra OS functions with web/Emscripten equivalents.
 *
 * The N64 OS provides:
 * - Cooperative multithreading (osCreateThread, osStartThread, etc.)
 * - Message queues (osSendMesg, osRecvMesg)
 * - Timer and timing (osGetTime, osGetCount)
 * - Interrupt management
 * - Video Interface control
 * - PI (Peripheral Interface) for ROM/cartridge DMA
 * - TLB (Translation Lookaside Buffer) management
 *
 * For the web port:
 * - Threads become a single-threaded cooperative loop via emscripten_set_main_loop
 * - Message queues become simple function call dispatching
 * - ROM access becomes ArrayBuffer reads from user-provided ROM file
 * - TLB/cache operations become no-ops
 * - VI control drives the WebGL swap chain
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* === Basic Types (matching N64 ultratypes.h) === */
#ifndef _ULTRATYPES_DEFINED_
#define _ULTRATYPES_DEFINED_

typedef signed char             s8;
typedef unsigned char           u8;
typedef signed short            s16;
typedef unsigned short          u16;
typedef signed int              s32;
typedef unsigned int            u32;
typedef signed long long        s64;
typedef unsigned long long      u64;
typedef float                   f32;
typedef double                  f64;

typedef volatile u8             vu8;
typedef volatile u16            vu16;
typedef volatile u32            vu32;
typedef volatile u64            vu64;
typedef volatile s8             vs8;
typedef volatile s16            vs16;
typedef volatile s32            vs32;
typedef volatile s64            vs64;
typedef volatile f32            vf32;
typedef volatile f64            vf64;

#endif /* _ULTRATYPES_DEFINED_ */

/* === OS Thread stubs === */
typedef struct OSThread {
    struct OSThread *next;
    s32 priority;
    s32 id;
    s32 state;
    /* ... simplified */
} OSThread;

typedef void *OSMesg;

typedef struct OSMesgQueue {
    OSThread    *mtqueue;
    OSThread    *fullqueue;
    s32         validCount;
    s32         first;
    s32         msgCount;
    OSMesg      *msg;
} OSMesgQueue;

typedef u64 OSTime;

/* OS Thread functions — stubs for web (single-threaded) */
void osCreateThread(OSThread *t, s32 id, void (*entry)(void*), void *arg, void *sp, s32 pri);
void osStartThread(OSThread *t);
void osStopThread(s32 id);
void osSetThreadPri(s32 id, s32 pri);
s32  osGetThreadPri(OSThread *t);
void osDestroyThread(OSThread *t);

/* OS Message Queue functions */
void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msg, s32 count);
s32  osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flags);
s32  osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flags);
s32  osJamMesg(OSMesgQueue *mq, OSMesg msg, s32 flags);

/* OS Timing */
OSTime osGetTime(void);
u32    osGetCount(void);
void   osSetTimer(void *t, OSTime countdown, OSTime interval, OSMesgQueue *mq, OSMesg msg);

/* OS Init */
void osInitialize(void);

/* Cache / TLB — no-ops on web */
void osWritebackDCacheAll(void);
void osWritebackDCache(void *vaddr, s32 len);
void osInvalDCache(void *vaddr, s32 len);
void osInvalICache(void *vaddr, s32 len);
void osUnmapTLB(s32 index);
void osMapTLB(s32 index, u32 pageSize, void *vaddr, u32 even, u32 odd, s32 asid);

/* FPC (Floating Point Control) */
u32  __osGetFpcCsr(void);
u32  __osSetFpcCsr(u32 flags);

/* PI (ROM/Cartridge DMA) — mapped to ArrayBuffer reads */
s32  osPiRawStartDma(s32 direction, u32 romAddr, void *ramAddr, u32 size);
s32  osPiGetStatus(void);
s32  osPiStartDma(void *ioMesg, s32 priority, s32 direction,
                  u32 devAddr, void *dramAddr, u32 size, OSMesgQueue *mq);
void osCreatePiManager(s32 pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf, s32 cmdMsgCnt);

/* VI (Video Interface) */
void osViSetMode(void *mode);
void osViSwapBuffer(void *frameBufPtr);
void osViSetSpecialFeatures(u32 func);
void osViBlack(s32 on);

/* TV Type global */
extern s32 osTvType;
#define OS_TV_PAL     0
#define OS_TV_NTSC    1
#define OS_TV_MPAL    2

/* Scheduler stubs */
typedef struct {
    /* simplified scheduler struct */
    s32 dummy;
} OSSched;

typedef struct {
    s32 dummy;
} OSScClient;

typedef struct {
    s32 type;
} OSScMsg;

void osCreateScheduler(OSSched *s, OSThread *t, void *mode, s32 numFields);
void osScAddClient(OSSched *s, OSScClient *c, OSMesgQueue *mq, void *data);
OSMesgQueue *osScGetCmdQ(OSSched *s);

/* Controller / Joypad */
typedef struct {
    u16 button;
    s8  stick_x;
    s8  stick_y;
    u8  errno_pad;
    u8  status;
} OSContPad;

typedef struct {
    u8 type;
    u8 status;
    u8 errno_pad;
} OSContStatus;

void osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data);
void osContStartReadData(OSMesgQueue *mq);
void osContGetReadData(OSContPad *pad);

/* Controller button masks */
#define CONT_A          0x8000
#define CONT_B          0x4000
#define CONT_G          0x2000
#define CONT_START      0x1000
#define CONT_UP         0x0800
#define CONT_DOWN       0x0400
#define CONT_LEFT       0x0200
#define CONT_RIGHT      0x0100
#define CONT_L          0x0020
#define CONT_R          0x0010
#define CONT_E          0x0008
#define CONT_D          0x0004
#define CONT_C          0x0002
#define CONT_F          0x0001

/* Message flags */
#define OS_MESG_NOBLOCK  0
#define OS_MESG_BLOCK    1

/* OS read/write */
#define OS_READ          0
#define OS_WRITE         1

/* PI status bits */
#define PI_STATUS_DMA_BUSY  0x01
#define PI_CLR_INTR         0x02
#define PI_SET_RESET        0x01
#define PI_DOM1_ADDR2       0x10000000

#define DEVICE_TYPE_CART    0

/* EEPROM stubs */
s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, s32 nbytes);
s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, s32 nbytes);

#endif /* HAL_OS_H */
