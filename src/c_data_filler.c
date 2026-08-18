#include <ultra64.h>
#include <PR/os.h>
#include <PR/gbi.h>
#include <PR/gu.h>

u8 c_data_array[0x11c00] = {0};
u8 _gameSegmentRomStart[4] = {0};

void get_counters(u32 *c0, u32 *c1) {
    if (c0) *c0 = 0;
    if (c1) *c1 = 0;
}

u32 __osGetTLBHi(s32 idx) { return 0; }
u32 tlbRandomGetNext(void) { return 0; }

s32 osPiReadIo(u32 devAddr, u32 *data) {
    if (data) *data = 0;
    return 0;
}

void guLookAtReflect(Mtx *m, LookAt *l, float xEye, float yEye, float zEye,
                     float xAt, float yAt, float zAt,
                     float xUp, float yUp, float zUp) {
    if (m) guMtxIdent(m);
}

void guOrtho(Mtx *m, float l, float r, float b, float t, float n, float f, float scale) {
    if (m) guMtxIdent(m);
}

/* Global Display Lists for Particles / Explosions */
#define DEF_GDL(addr) Gfx globalDL_##addr[2] = {{{(u32)G_ENDDL, 0}}};
DEF_GDL(0x000)
DEF_GDL(0x078)
DEF_GDL(0x120)
DEF_GDL(0x1c8)
DEF_GDL(0x270)
DEF_GDL(0x318)
DEF_GDL(0x3c0)
DEF_GDL(0x468)
DEF_GDL(0x510)
DEF_GDL(0x5b8)
DEF_GDL(0x660)
DEF_GDL(0x708)
DEF_GDL(0x7b0)
DEF_GDL(0x858)
DEF_GDL(0x900)
DEF_GDL(0x9a8)
DEF_GDL(0xa50)

/* Animation Offsets */
#define DEF_ANIM(name) u8 ANIM_DATA_##name[4] = {0};
DEF_ANIM(bond_eye_fire)
DEF_ANIM(bond_eye_walk)
DEF_ANIM(bond_watch)
DEF_ANIM(death_left_leg)
DEF_ANIM(death_neck)
DEF_ANIM(death_stagger_back_to_wall)
DEF_ANIM(extending_left_hand)
DEF_ANIM(fire_hip)
DEF_ANIM(fire_jump_to_side_left)
DEF_ANIM(fire_jump_to_side_right)
DEF_ANIM(fire_kneel_forward_one_handed_weapon_slow)
DEF_ANIM(fire_kneel_left_leg)
DEF_ANIM(fire_standing_draw_one_handed_weapon_fast)
DEF_ANIM(fire_throw_grenade)
DEF_ANIM(hit_butt_long)
DEF_ANIM(hit_butt_short)
DEF_ANIM(idle)
DEF_ANIM(idle_unarmed)
DEF_ANIM(jump_backwards)
DEF_ANIM(look_around)
DEF_ANIM(running)
DEF_ANIM(running_female)
DEF_ANIM(running_one_handed_weapon)
DEF_ANIM(side_step_left)
DEF_ANIM(slide_left)
DEF_ANIM(slide_right)
DEF_ANIM(sneeze)
DEF_ANIM(spotting_bond)
DEF_ANIM(sprinting)
DEF_ANIM(sprinting_one_handed_weapon)
DEF_ANIM(surrendering_armed)
DEF_ANIM(surrendering_armed_drop_weapon)
DEF_ANIM(walking)
DEF_ANIM(walking_female)
DEF_ANIM(walking_unarmed)
/* Global Image Segment and Tables */
u8 _imagesSegmentRomStart[4] = {0};
u32 _GlobalimagetableSegmentStart[4] = {0};
u32 _GlobalimagetableSegmentEnd[4] = {0};
u8 _GlobalimagetableSegmentRomStart[4] = {0};

#define DEF_IMG(name) u8 s_##name[16] = {0};
DEF_IMG(genericimage)
DEF_IMG(impactimages)
DEF_IMG(explosion_smokeimages)
DEF_IMG(scattered_explosions)
DEF_IMG(flareimage1)
DEF_IMG(flareimage2)
DEF_IMG(flareimage3)
DEF_IMG(flareimage4)
DEF_IMG(flareimage5)
DEF_IMG(ammo9mmimage)
DEF_IMG(rifleammoimage)
DEF_IMG(shotgunammoimage)
DEF_IMG(knifeammoimage)
DEF_IMG(glammoimage)
DEF_IMG(rocketammoimage)
DEF_IMG(genericmineammoimage)
DEF_IMG(grenadeammoimage)
DEF_IMG(magnumammoimage)
DEF_IMG(goldengunammoimage)
DEF_IMG(remotemineammoimage)
DEF_IMG(timedmineammoimage)
DEF_IMG(proxmineammoimage)
DEF_IMG(tankammoimage)
DEF_IMG(crosshairimage)
DEF_IMG(betacrosshairimage)
DEF_IMG(glassoverlayimage)
DEF_IMG(monitorimages)
DEF_IMG(skywaterimages)
DEF_IMG(mainfolderimages)
DEF_IMG(mpradarimages)
DEF_IMG(mpcharselimages)
DEF_IMG(mpstageselimages)
Gfx dlFastPipelineSetup[2] = {{{(u32)G_ENDDL, 0}}};
Gfx dlBasicGeometry[2] = {{{(u32)G_ENDDL, 0}}};
Gfx dlZBufferGeometry[2] = {{{(u32)G_ENDDL, 0}}};
u8 _fontdlSegmentRomStart[4] = {0};
u8 _fontdlSegmentRomEnd[4] = {0};
/* Random Seed for ChrObj */
u32 g_chrObjRandomSeed = 0x12345678;

u32 chrObjRandomGetNext(void) {
    g_chrObjRandomSeed = g_chrObjRandomSeed * 0x41C64E6D + 0x3039;
    return (g_chrObjRandomSeed >> 16) & 0x7FFF;
}

/* Microcode data segments */
u32 gsp3DDataStart = 0;
u32 rspbootTextEnd = 0;

/* Fonts and text segments */
u8 _fontbankgothicSegmentStart[4] = {0};
u8 _fontbankgothicSegmentEnd[4] = {0};
u8 _fontbankgothicSegmentRomStart[4] = {0};

u8 _fontocraSegmentStart[4] = {0};
u8 _fontocraSegmentEnd[4] = {0};
u8 _fontocraSegmentRomStart[4] = {0};

u8 _fontnumericSegmentStart[4] = {0};
u8 _fontnumericSegmentEnd[4] = {0};
u8 _fontnumericSegmentRomStart[4] = {0};

u8 _fonthandelgothicSegmentStart[4] = {0};
u8 _fonthandelgothicSegmentEnd[4] = {0};
u8 _fonthandelgothicSegmentRomStart[4] = {0};

u8 _fonthandelgothicsmSegmentStart[4] = {0};
u8 _fonthandelgothicsmSegmentEnd[4] = {0};
u8 _fonthandelgothicsmSegmentRomStart[4] = {0};

u8 _fontbankgothicsmSegmentStart[4] = {0};
u8 _fontbankgothicsmSegmentEnd[4] = {0};
u8 _fontbankgothicsmSegmentRomStart[4] = {0};

/* guScaleF */
void guScaleF(float mf[4][4], float x, float y, float z) {
    memset(mf, 0, sizeof(float) * 16);
    mf[0][0] = x;
    mf[1][1] = y;
    mf[2][2] = z;
    mf[3][3] = 1.0f;
}
void chrObjRandomSetSeed(u32 seed) {
    g_chrObjRandomSeed = seed;
}

u8 _fontzurichboldSegmentStart[4] = {0};
u8 _fontzurichboldSegmentEnd[4] = {0};
u8 _fontzurichboldSegmentRomStart[4] = {0};

/* Title Screen Rareware / Intro Display Lists */
Gfx D_02004758[2] = {{{(u32)G_ENDDL, 0}}};
Gfx D_02005FF0[2] = {{{(u32)G_ENDDL, 0}}};
Gfx DL_RAREWARETEXT[2] = {{{(u32)G_ENDDL, 0}}};
Gfx D_020043E8[2] = {{{(u32)G_ENDDL, 0}}};
Gfx D_02004FE8[2] = {{{(u32)G_ENDDL, 0}}};