#ifndef _HAL_GFX_H_
#define _HAL_GFX_H_

#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

typedef struct {
    u16 *pixels;
    u32 width;
    u32 height;
    u32 stride;
    u32 format; /* 0 = RGBA5551 */
    u32 frameNumber;
} hal_framebuffer_info_t;

void hal_gfx_init(void);
void hal_gfx_reset(void);
void hal_gfx_render_display_list(Gfx *gdl);
hal_framebuffer_info_t hal_gfx_get_framebuffer_info(void);
u16 *hal_gfx_get_framebuffer(void);
u32 hal_gfx_get_frame_count(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_GFX_H_ */