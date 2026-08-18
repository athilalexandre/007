#include <ultra64.h>
#include <PR/gbi.h>

void hal_gfx_init(void) {
}

void hal_gfx_process_gdl(Gfx *gdl) {
    if (!gdl) return;
    Gfx *cmd = gdl;
    while (cmd) {
        u8 opcode = (u8)(cmd->words.w0 >> 24);
        if (opcode == (u8)G_ENDDL) {
            break;
        }
        cmd++;
    }
}