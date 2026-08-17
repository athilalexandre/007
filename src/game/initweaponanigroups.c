#include <ultra64.h>
#include "model.h"

extern void get_ptr_allocated_block_for_vertices(int);

void init_weapon_animation_groups_maybe(void) {
    set_vtxallocator((void*)get_ptr_allocated_block_for_vertices);
    initWeaponAnimGroups();
}