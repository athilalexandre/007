#include <stdlib.h>
#include <string.h>
#include "hal_gfx.h"
#include <gbi_extension.h>
#include <string.h>
#include <stdio.h>

static u16 s_Framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT] __attribute__((aligned(64)));
static u16 s_DepthBuffer[SCREEN_WIDTH * SCREEN_HEIGHT] __attribute__((aligned(64)));
static u32 s_FrameCounter = 0;

static u32 s_FillColor = 0;
static u32 s_PrimColor = 0xFFFFFFFF;
static u32 s_EnvColor = 0xFFFFFFFF;
static u16 *s_CurrentColorBuffer = s_Framebuffer;

void hal_gfx_init(void) {
    memset(s_Framebuffer, 0, sizeof(s_Framebuffer));
    memset(s_DepthBuffer, 0, sizeof(s_DepthBuffer));
    s_FrameCounter = 0;
    s_CurrentColorBuffer = s_Framebuffer;
}

void hal_gfx_reset(void) {
    hal_gfx_init();
}

hal_framebuffer_info_t hal_gfx_get_framebuffer_info(void) {
    hal_framebuffer_info_t info;
    info.pixels = s_Framebuffer;
    info.width = SCREEN_WIDTH;
    info.height = SCREEN_HEIGHT;
    info.stride = SCREEN_WIDTH * sizeof(u16);
    info.format = 0; /* RGBA5551 */
    info.frameNumber = s_FrameCounter;
    return info;
}

u16 *hal_gfx_get_framebuffer(void) {
    return s_Framebuffer;
}

u32 hal_gfx_get_frame_count(void) {
    return s_FrameCounter;
}

static void execute_fill_rect(s32 ulx, s32 uly, s32 lrx, s32 lry) {
    ulx = (ulx < 0) ? 0 : ulx;
    uly = (uly < 0) ? 0 : uly;
    lrx = (lrx >= SCREEN_WIDTH) ? (SCREEN_WIDTH - 1) : lrx;
    lry = (lry >= SCREEN_HEIGHT) ? (SCREEN_HEIGHT - 1) : lry;

    u16 color16 = (u16)(s_FillColor & 0xFFFF);
    for (s32 y = uly; y <= lry; y++) {
        for (s32 x = ulx; x <= lrx; x++) {
            s_CurrentColorBuffer[y * SCREEN_WIDTH + x] = color16;
        }
    }
}

void hal_gfx_render_display_list(Gfx *gdl) {
    if (!gdl) return;

    Gfx *p = gdl;
    u32 cmdCount = 0;
    const u32 MAX_CMDS = 50000;

    while (cmdCount++ < MAX_CMDS) {
        u32 w0 = p->words.w0;
        u32 w1 = p->words.w1;
        u8 opcode = (u8)(w0 >> 24);

        if ((s8)opcode == (s8)G_ENDDL) {
            break;
        }

        switch (opcode) {
            case G_SETFILLCOLOR:
                s_FillColor = w1;
                break;

            case G_SETPRIMCOLOR:
                s_PrimColor = w1;
                break;

            case G_SETENVCOLOR:
                s_EnvColor = w1;
                break;

            case G_FILLRECT: {
                s32 lrx = (s32)((w0 >> 14) & 0x3FF);
                s32 lry = (s32)((w0 >> 2) & 0x3FF);
                s32 ulx = (s32)((w1 >> 14) & 0x3FF);
                s32 uly = (s32)((w1 >> 2) & 0x3FF);
                execute_fill_rect(ulx, uly, lrx, lry);
                break;
            }

            case G_SETCIMG: {
                /* Set color image buffer */
                s_CurrentColorBuffer = s_Framebuffer;
                break;
            }

            case G_SETZIMG: {
                /* Depth buffer pointer */
                break;
            }

            case G_RDPFULLSYNC:
                s_FrameCounter++;
                return;

            case (u8)G_TRI4:
            case G_SETTEX:
            case G_VTX:
            case G_MTX:
            case (u8)G_POPMTX:
            case G_DL:
            case (u8)G_MOVEWORD:
            case G_MOVEMEM:
            case G_LOADBLOCK:
            case G_SETTILE:
            case G_SETTILESIZE:
            case G_LOADTLUT:
            case G_RDPPIPESYNC:
            case G_RDPTILESYNC:
            case G_RDPLOADSYNC:
            case G_TEXRECT:
            case G_TEXRECTFLIP:
            default:
                break;
        }

        p++;
    }
    s_FrameCounter++;
}