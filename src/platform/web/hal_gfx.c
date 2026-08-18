#include <ultra64.h>
#include "gbi_extension.h"

static u32 s_GfxCommandCount = 0;

void hal_gfx_init(void) {
    s_GfxCommandCount = 0;
}

u32 hal_gfx_get_command_count(void) {
    return s_GfxCommandCount;
}

void hal_gfx_process_display_list(Gfx *gdl) {
    if (!gdl) return;
    Gfx *cmd = gdl;
    while (1) {
        u8 opcode = (u8)(cmd->words.w0 >> 24);
        s_GfxCommandCount++;
        if (opcode == G_ENDDL) {
            break;
        }
        cmd++;
        if (s_GfxCommandCount > 50000) {
            break; // safety limiter
        }
    }
}