#include <ultra64.h>
#include "model.h"

//uncomment when actor is worked on
//#include "chr.h"
extern u8 *get_ptr_allocated_block_for_vertices(void);

void init_weapon_animation_groups_maybe(void) {
    set_vtxallocator((s32)get_ptr_allocated_block_for_vertices);
    initWeaponAnimGroups();
}
