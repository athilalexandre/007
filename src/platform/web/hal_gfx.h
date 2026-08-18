#ifndef HAL_GFX_H
#define HAL_GFX_H

#include <ultra64.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

typedef struct {
    u32 total_commands;
    u32 total_triangles;
    u32 total_vertices;
    u32 total_textures;
    u32 unsupported_commands;
    u32 max_dl_depth;
    u32 commands_by_opcode[256];
} HalGfxTelemetry;

void hal_gfx_init(void);
void hal_gfx_execute_display_list(Gfx *start, Gfx *end);
u16 *hal_gfx_get_framebuffer(void);
u32 hal_gfx_get_frame_count(void);
const HalGfxTelemetry *hal_gfx_get_telemetry(void);
u32 hal_gfx_get_telemetry_json(char *buf, u32 maxlen);

#endif /* HAL_GFX_H */