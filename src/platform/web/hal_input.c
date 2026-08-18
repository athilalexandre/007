#include <ultra64.h>
#include <PR/os.h>
#include "hal_input.h"

static OSContPad s_Controllers[4];

void hal_input_set_buttons(u16 buttons, s8 stickX, s8 stickY) {
    s_Controllers[0].button = buttons;
    s_Controllers[0].stick_x = stickX;
    s_Controllers[0].stick_y = stickY;
}

s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *status) {
    if (bitpattern) *bitpattern = 0x0F;
    if (status) {
        for (int i = 0; i < 4; i++) {
            status[i].type = CONT_TYPE_NORMAL;
            status[i].status = 0;
            status[i].errno = 0;
        }
    }
    return 0;
}

s32 osContStartQuery(OSMesgQueue *mq) {
    if (mq) osSendMesg(mq, (OSMesg)0, OS_MESG_NOBLOCK);
    return 0;
}

void osContGetQuery(OSContStatus *status) {
    if (status) {
        for (int i = 0; i < 4; i++) {
            status[i].type = CONT_TYPE_NORMAL;
            status[i].status = 0;
            status[i].errno = 0;
        }
    }
}

s32 osContStartReadData(OSMesgQueue *mq) {
    if (mq) osSendMesg(mq, (OSMesg)0, OS_MESG_NOBLOCK);
    return 0;
}

void osContGetReadData(OSContPad *pad) {
    if (pad) {
        for (int i = 0; i < 4; i++) {
            pad[i] = s_Controllers[i];
        }
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
void hal_input_init(void) {}
