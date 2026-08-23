
#ifndef SIGNATURES_H
#define SIGNATURES_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <ultra64.h>
#include <bondtypes.h>
#include <bondconstants.h>

#if defined(TARGET_WEB) || defined(__wasm__)
#ifdef PHYS_TO_K0
#undef PHYS_TO_K0
#endif
#define PHYS_TO_K0(x) ((void *)(uintptr_t)(x))

#ifdef K0_TO_PHYS
#undef K0_TO_PHYS
#endif
#define K0_TO_PHYS(x) ((void *)(uintptr_t)(x))

#ifdef K1_TO_PHYS
#undef K1_TO_PHYS
#endif
#define K1_TO_PHYS(x) ((void *)(uintptr_t)(x))

#ifdef PHYS_TO_K1
#undef PHYS_TO_K1
#endif
#define PHYS_TO_K1(x) ((void *)(uintptr_t)(x))
#endif


// Forward declarations of types
struct ChrRecord;
typedef struct ChrRecord ChrRecord;
struct HatRecord;
typedef struct HatRecord HatRecord;
struct MonitorRecord;
typedef struct MonitorRecord MonitorRecord;
struct coord3d;
typedef struct coord3d coord3d;
struct PropRecord;
typedef struct PropRecord PropRecord;
struct StandTilePoint;
typedef struct StandTilePoint StandTilePoint;
struct bondstruct_unk_animation_related;
typedef struct bondstruct_unk_animation_related bondstruct_unk_animation_related;
struct sImageTableEntry;
struct save_data;
typedef struct save_data save_data;
struct DoorRecord;
typedef struct DoorRecord DoorRecord;
struct ShotData;
typedef struct ShotData ShotData;
struct BulletHit;
typedef struct BulletHit BulletHit;
struct ModelFileHeader;
typedef struct ModelFileHeader ModelFileHeader;
struct texpool;
struct Model;
typedef struct Model Model;
struct PropDefHeaderRecord;
typedef struct PropDefHeaderRecord PropDefHeaderRecord;

// Standard C & memory prototypes
void bcopy(const void *src, void *dst, size_t length);
void bzero(void *b, size_t length);

// ROM & Memory Copy Prototypes
void romCopy(void *target, void *source, u32 size);
s32  romCopyAligned(void *target, void *source, s32 length);

// Dyn Prototypes
s32 dynGetFreeGfx(Gfx *gdl);
s32 dynGetFreeVtx(void);

// Texture prototypes
void texLoadFromDisplayList(Gfx *gdl, struct texpool *arg1);
void texSelect(Gfx **gdlptr, struct sImageTableEntry *tconfig, u32 arg2, s32 arg3, u32 ulst);

// Core Engine Function Prototypes with exact signatures
void cleanup_rooms(void);
void skySetStageNum(s32 stagenum);
void init_watch_at_start_of_stage(int stage);
void bondviewRemovePlayerBody(void);
void objectivestatusCheckRoomEntered(s32 room);
void mtxLoadRandomRotation(Mtxf *mtx);
void loop_set_sound_effect_all_slots(void);
f32  get_xrotation_solo_watch_menu_for_item(ITEM_IDS item);
void hatAssignToChr(HatRecord *hat, ChrRecord *chr);
void generate_player_thrown_grenade(s32 weapon);
void generate_player_thrown_object(s32 weapon);
f32  bondinvGetHposWatchForIndex(s32 index);
void debmenuResetBuffer(void);
void replay_recorded_ramrom_from_indy(void);
void sub_GAME_7F05D690(void);
void set_cur_player_look_vertical_inverted(u32 flag);
void gotoAboveDebugOption(void);
void handle_alarm_gas_timer_calldamage(void);
void chrObjRandomSetSeed(u32 seed);
void sub_GAME_7F057DF8(Mtxf *arg0, Mtxf *arg1, s32 count);
void stop_recording_ramrom(void);
void currentPlayerSetXAutoAimEnabled(s32 flag);
void currentPlayerCreateRocket(GUNHAND hand);
void explosionClearBulletImpactRoomByFlag(PropRecord* arg0, s8 arg1);
void generate_player_thrown_knife(s32 weapon);
void explosionClearBulletImpactRoom(PropRecord* arg0);
void indycommHostRamRomLoad(char *filename, u8 *target, s32 size);
void gunSpawnGLGrenade(s32 weapon);
void bondinvSetCurEquippedItem(int item);
void add_ammo_to_inventory(AMMOTYPE ammotype, int amount, int doplaysound, int dodisplaytext);
void used_to_load_1st_person_model_on_demand(GUNHAND hand);
s32  getmusictrack_or_randomtrack(s32 stage);

void initAnimationsBuffer(struct bondstruct_unk_animation_related* animBuffer, OSMesgQueue* mq, s32 unused);
void initWeaponAnimGroups(void);
void matrix_4x4_7F058C64(void);
void matrix_4x4_7F058C88(void);
void matrix_4x4_invert_affine(Mtxf *matrix, Mtxf *result);
void set_obj_collision_flag(s32 flag);
void set_missionstate_zero(void);
void set_missionstate(MISSION_STATE_ID arg0);
void currentPlayerSetCameraMode(s32 mode);
void currentPlayerUnEquipWeaponWrapper(GUNHAND hand, s32 flag);
void set_players_team_or_scenario_item_flag(int player, s32 flag);
void rle_expand_8bit(u8 *src, u8 *dst);
f32  get_horizontal_offset_on_solo_watch_menu_for_item(ITEM_IDS item);
f32  get_depth_offset_solo_watch_menu_inventory_page_for_item(ITEM_IDS item);
f32  get_vertical_offset_on_solo_watch_menu_for_item(ITEM_IDS item);
f32  bondinvGetVoffsetForIndex(s32 index);
void fileCopyDemoSaveToRamRomSave(u32 folder, save_data *save);
void fileCopySave(s32 folder, save_data *out_save);
void fileCopyFolderToFirstFree(s32 foldernum);
void fileClearSavefileForFolder(s32 a);
void fileLoad(ModelFileHeader *header, char *name);
void mpwatchMenuTick(void);
void cleanupExplosions(void);
void gotoBelowDebugOption(void);
void gotoLeftDebugOption(void);
void gotoRightDebugOption(void);
f32  get_yrotation_solo_watch_menu_for_item(ITEM_IDS item);
void sub_GAME_7F0C1310(void);
void sub_GAME_7F0A6A80(void);
void debmenuSetMenu(char **labels, s32 (*positions)[2], s32 *offsets);
void chrpropDeregisterRoom(PropRecord *prop, s16 room);
void memaFree(void *ptr, s32 size);
void setRamRomRecordSlot(s32 slot);
void set_sound_effect_for_weapontype_collection(ITEM_IDS weapontype);
void currentPlayerSetMatrix10C8(Mtx *matrix);
void currentPlayerSetMatrix10C4(Mtx *matrix);
void currentPlayerSetViewToWorldMtxf(Mtxf *mtx);
void save_img_index_to_obj_ani_slot(MonitorRecord *mon, void *unk88);
void cur_player_set_screen_setting(u32 param_1);
void cur_player_set_ammo_onscreen_setting(u32 flag);
void cur_player_set_aim_control(u32 flag);
void cur_player_set_sight_onscreen_control(u32 flag);
void cur_player_set_lookahead(u32 param_1);
void alloc_false_GUARDdata_to_exec_global_action(void);
void setTextOrientation(s32 a);
f32  get_vertical_position_solo_watch_menu_main_page_for_item(ITEM_IDS item);
f32  get_depth_on_solo_watch_menu_page_for_item(ITEM_IDS item);
void set_favorite_weapon_for_every_player(void);
void chrobjSndCreatePostEventDefault(ALSoundState *state, coord3d *pos);
void set_BONDdata_outside_watch_menu_flag(s32 flag);
void set_bondata_invincible_flag(u32 arg0);
void load_ramrom_from_devtool(void);
void cleanup_REMOVED_(void);
void currentPlayerSetYAutoAimEnabled(s32 flag);
void select_ramrom_to_play(void);

// Additional link-resolved prototypes
void set_max_ammo_for_cur_player(void);
void alloc_init_GUARDdata_entries(s32 a);
void chrpropDetach(PropRecord *prop);
void chrSetWeaponFlag4(ChrRecord *chr, GUNHAND hand);
void setSixExplosionAndSmokeEntries(void);
void indycommHostSaveFile(char *filename, s32 size, u8 *data);
void clearChrGunModelInstances(void);
void set_BONDdata_field_10E0(s32 a);
void debug_object_load_all_models(void);
void sub_GAME_7F05E83C(GUNHAND hand);
void sub_GAME_7F05E6B4(GUNHAND hand, s32 arg1);
void sub_GAME_7F05E978(Model *model, s32 val);
void fogLoadLevelEnvironment(s32 a, s32 b);
void save_ramrom_to_devtool(void);
void modelmgrAllocateModelSlots(s32 count);
void modelmgrAllocateAnimModelSlots(s32 numanimated);
void debmenuSetFgColour(s32 r, s32 g, s32 b, s32 a);
void sub_GAME_7F05C614(void);
void cleanupObjectSounds(void);
void cleanupGuardData(void);
void debmenuSetPos(s32 x, s32 y);
void langClearBank(s32 bank);
void fileGenerateCRC(u8 *addressA, u8 *addressB, save_data *retval);
void setMPWeaponSet(s32 set);
void sub_GAME_7F008DE4(u8 **addr, s32 *size);
void initializeGunBarrelIntro(u8 *gfxBuffer, s32 bufferSize);
void fileUpdateSelectedBondInSave(s32 save);
void updateFrameCounters(s32 a);
void sub_GAME_7F078464(s32 a);
void sub_GAME_7F059334(Mtx *src, Mtx *dst);
void sub_GAME_7F04E9BC(PropRecord *prop, ShotData *shotdata);
void increment_num_suicides_display_MP(void);
void increment_num_deaths(void);
void increment_num_times_killed_MwtGC(void);
f32  get_lateral_position_solo_watch_menu_main_page_for_item(ITEM_IDS item);
void setupRarewareLogoData(s32 a, s32 b);
void bondviewSetVisibleToGuardsFlag(s32 flag);
// bondviewSetCurrentPlayerPosition in bondview.h
void bondviewUpdateCameraMatrices(coord3d* cam_pos, coord3d* cam_look_dir, coord3d* cam_up);
void set_cur_player(s32 a);
void initializeDebugCameraPosition(void);
void updateDebugCameraWorldPosition(void);
void resetDebugCameraToPlayerPosition(void);
void zbufSetBuffer(s32 a, s32 b, s32 c);
void store_favorite_weapon_current_player(u32 right, u32 left);
void set_show_patrols_flag(s32 flag);
void viSetColorMode32Bit(void);
void viSetColorMode16Bit(void);
void debug_weapon_load_table(void);
void lightFixtureBreak(Gfx * hit_gfx, u32 tri_type, s32 room_index);
void getRoomPositionScaledByIndex(s32 index, struct coord3d *param_2);
void debmenuPrintString(const unsigned char *str);
void bondviewUpdateFrustumPlanes(void);
void bondviewSetCameraMode(s32 mode);
void bondviewKillCurrentPlayer(void);
void sub_GAME_7F0BD8FC(s32 a);
void doorDeactivatePortal(DoorRecord *door);
void objHit(ShotData *shotdata, BulletHit *hit);
void bondinvIncrementHeldTime(s32 a, s32 b);
void bondinvDetermineEquippedItem(void);
void bondinvSetAllGunsFlag(s32 flag);
// bondinvAddTextOverride in bondinv.h
void bullet_path_from_screen_center(coord3d* arg0, coord3d* arg1, enum GUNHAND arg2);
void gunUpdateAttachedRocket(s32 handIndex);
void removed_debug_roomblocks_feature(void);
void setupUpdateObjectRoomPosition(struct ObjectRecord *);
void display_text_for_weapon_in_lower_left_corner(ITEM_IDS weaponid);
void joyConsumeSamplesWrapper(void);
void bgFindRoomsAlongSegment(coord3d *pos1, coord3d *pos2, u8 *initialRooms, u8 *outRoomSet, s32 *outRoomNums, s32 *outRoomNumsCount, s32 outRoomNumsMax);
void modelGetXYExtents(Model *model, f32 *arg1, f32 *arg2, f32 *arg3, f32 *arg4);
void transform3Dto2DWithZScaling(coord3d *in, coord3d *out);
void ai(PropDefHeaderRecord *Entityp, PROP_TYPE EntityType);
s32  objectiveGetStatus_WEAK(s32 objectiveNum, s32 unused);

// Initialization & Pool Prototypes
void init_path_table_links(void);
void alloc_explosion_smoke_casing_scorch_impact_buffers(void);
void disableOnscreenCheatText(void);
void initCheatTextBuffer(void);
void initCasingPool(void);
void cleanup_window_pieces(void);
void cleanupAlarms(void);
void cleanupObjects(s32 stage);
void cleanupObjectives(void);
void cleanupSFXRelated(void);
void cleanupplayersoundrelated(void);

#endif /* SIGNATURES_H */
