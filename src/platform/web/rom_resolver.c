#include <string.h>
/*
 * GoldenEye 007 (N64) Web ROM Resource Resolver
 * Authoritative numeric ROM segment offsets (USA Retail 12MB layout).
 */
#include "rom_resolver.h"
#include <string.h>

static const rom_segment_entry_t s_RomSegments[] = {
    { "assets/ge007.u.117880.jfont_dl.bin", 0x117880, 192, 0 },
    { "assets/ge007.u.117940.jfont_chardata.bin", 0x117940, 46848, 0 },
    { "assets/ge007.u.123040.efont_chardata.bin", 0x123040, 6784, 0 },
    { "assets/animationtable_entries.bin", 0x124AC0, 1482432, 0 },
    { "assets/animationtable_data.bin", 0x28E980, 59360, 0 },
    { "assets/ge007.u.29D160.Globalimagetable.bin", 0x29D160, 2760, 0 },
    { "assets/rarewarelogo.bin", 0x29E560, 26608, 0 },
    { "assets/ge007.u.2A4D50.usedby7F008DE4.bin", 0x2A4D50, 107904, 0 },
    { "assets/ramrom/ramrom_Dam_1.bin", 0x2BF2D0, 20992, 0 },
    { "assets/ramrom/ramrom_Dam_2.bin", 0x2C44D0, 8144, 0 },
    { "assets/ramrom/ramrom_Facility_1.bin", 0x2C64A0, 6832, 0 },
    { "assets/ramrom/ramrom_Facility_2.bin", 0x2C7F50, 9184, 0 },
    { "assets/ramrom/ramrom_Facility_3.bin", 0x2CA330, 7280, 0 },
    { "assets/ramrom/ramrom_Runway_1.bin", 0x2CBFA0, 10064, 0 },
    { "assets/ramrom/ramrom_Runway_2.bin", 0x2CE6F0, 10512, 0 },
    { "assets/ramrom/ramrom_BunkerI_1.bin", 0x2D1000, 13200, 0 },
    { "assets/ramrom/ramrom_BunkerI_2.bin", 0x2D4390, 21120, 0 },
    { "assets/ramrom/ramrom_Silo_1.bin", 0x2D9610, 8592, 0 },
    { "assets/ramrom/ramrom_Silo_2.bin", 0x2DB7A0, 8144, 0 },
    { "assets/ramrom/ramrom_Frigate_1.bin", 0x2DD770, 6576, 0 },
    { "assets/ramrom/ramrom_Frigate_2.bin", 0x2DF120, 13536, 0 },
    { "assets/ramrom/ramrom_Train.bin", 0x2E2600, 15856, 0 },
    { "assets/font/fontBankGothic_kerning.bin", 0x2E63F0, 676, 0 },
    { "assets/font/fontBankGothic_fontchartable.bin", 0x2E6694, 8716, 0 },
    { "assets/font/fontZurichBold_kerning.bin", 0x2E88A0, 676, 0 },
    { "assets/font/fontZurichBold_fontchartable.bin", 0x2E8B44, 12956, 0 },
    { "assets/music/sfx.ctl", 0x2EBDE0, 23488, 0 },
    { "assets/music/sfx.tbl", 0x2F19A0, 797360, 0 },
    { "assets/music/instruments.ctl", 0x3B4450, 17312, 0 },
    { "assets/music/instruments.tbl", 0x3B87F0, 397216, 0 },
    { "assets/music/music.sbk", 0x419790, 126667, 0 },
    { "assets/music/number_music_samples", 0x419790, 4, 0 },
    { "assets/music/table_music_data.bin", 0x419794, 504, 0 },
    { "assets/music/Mno_music.bin", 0x41998C, 42, 1 },
    { "assets/music/Msolo_death_abrev.bin", 0x4199B6, 470, 1 },
    { "assets/music/Mintro_eye.bin", 0x419B8C, 2222, 1 },
    { "assets/music/Mtrain.bin", 0x41A43A, 3050, 1 },
    { "assets/music/Mdepot.bin", 0x41B024, 3488, 1 },
    { "assets/music/Mjungle_unused.bin", 0x41BDC4, 3480, 1 },
    { "assets/music/Mcitadel.bin", 0x41CB5C, 3520, 1 },
    { "assets/music/Mfacility.bin", 0x41D91C, 2766, 1 },
    { "assets/music/Mcontrol.bin", 0x41E3EA, 2910, 1 },
    { "assets/music/Mdam.bin", 0x41EF48, 3588, 1 },
    { "assets/music/Mfrigate.bin", 0x41FD4C, 3552, 1 },
    { "assets/music/Marchives.bin", 0x420B2C, 2388, 1 },
    { "assets/music/Msilo.bin", 0x421480, 3696, 1 },
    { "assets/music/Mjungle_perimeter_unused.bin", 0x4222F0, 3948, 1 },
    { "assets/music/Mstreets.bin", 0x42325C, 3330, 1 },
    { "assets/music/Mbunker1.bin", 0x423F5E, 1650, 1 },
    { "assets/music/Mbunker2.bin", 0x4245D0, 1664, 1 },
    { "assets/music/Mstatue.bin", 0x424C50, 2456, 1 },
    { "assets/music/Melevator_control.bin", 0x4255E8, 2522, 1 },
    { "assets/music/Mcradle.bin", 0x425FC2, 3424, 1 },
    { "assets/music/Mnull1.bin", 0x426D22, 42, 1 },
    { "assets/music/Melevator_wc.bin", 0x426D4C, 1606, 1 },
    { "assets/music/Megyptian.bin", 0x427392, 3482, 1 },
    { "assets/music/Mfolders.bin", 0x42812C, 994, 1 },
    { "assets/music/Mwatchmusic.bin", 0x42850E, 498, 1 },
    { "assets/music/Maztec.bin", 0x428700, 3186, 1 },
    { "assets/music/Mwatercaverns.bin", 0x429372, 3628, 1 },
    { "assets/music/Mdeathsolo.bin", 0x42A19E, 870, 1 },
    { "assets/music/Msurface2.bin", 0x42A504, 3510, 1 },
    { "assets/music/Mtrainx.bin", 0x42B2BA, 2208, 1 },
    { "assets/music/Mnull2.bin", 0x42BB5A, 42, 1 },
    { "assets/music/Mfacilityx.bin", 0x42BB84, 2392, 1 },
    { "assets/music/Mdepotx.bin", 0x42C4DC, 1980, 1 },
    { "assets/music/Mcontrolx.bin", 0x42CC98, 1352, 1 },
    { "assets/music/Mwatercavernsx.bin", 0x42D1E0, 1876, 1 },
    { "assets/music/Mdamx.bin", 0x42D934, 1312, 1 },
    { "assets/music/Mfrigatex.bin", 0x42DE54, 1474, 1 },
    { "assets/music/Marchivesx.bin", 0x42E416, 1828, 1 },
    { "assets/music/Msilox.bin", 0x42EB3A, 2292, 1 },
    { "assets/music/Mnull3.bin", 0x42F42E, 42, 1 },
    { "assets/music/Mstreetsx.bin", 0x42F458, 1644, 1 },
    { "assets/music/Mbunker1x.bin", 0x42FAC4, 1958, 1 },
    { "assets/music/Mbunker2x.bin", 0x43026A, 1614, 1 },
    { "assets/music/Mjunglex.bin", 0x4308B8, 2070, 1 },
    { "assets/music/Mnint_rare_logo.bin", 0x4310CE, 1074, 1 },
    { "assets/music/Mstatuex.bin", 0x431500, 1720, 1 },
    { "assets/music/Maztecx.bin", 0x431BB8, 2262, 1 },
    { "assets/music/Megyptianx.bin", 0x43248E, 2224, 1 },
    { "assets/music/Mcradlex.bin", 0x432D3E, 1738, 1 },
    { "assets/music/Mcuba.bin", 0x433408, 2122, 1 },
    { "assets/music/Mrunway.bin", 0x433C52, 3358, 1 },
    { "assets/music/Mrunway_plane.bin", 0x434970, 730, 1 },
    { "assets/music/Msurface2x.bin", 0x434C4A, 1832, 1 },
    { "assets/music/Mwindblowing.bin", 0x435372, 1314, 1 },
    { "assets/music/Mmultideath_alt.bin", 0x435894, 524, 1 },
    { "assets/music/Mjungle.bin", 0x435AA0, 1928, 1 },
    { "assets/music/Mrunwayx.bin", 0x436228, 1570, 1 },
    { "assets/music/Msurface1.bin", 0x43684A, 3432, 1 },
    { "assets/music/Mmultiplayerdeath.bin", 0x4375B2, 712, 1 },
    { "assets/music/Msurface1x.bin", 0x43787A, 1832, 1 },
    { "assets/music/Msurface2_ending.bin", 0x437FA2, 668, 1 },
    { "assets/music/Mstatue_ending.bin", 0x43823E, 358, 1 },
    { "assets/music/Mfrigate_outro.bin", 0x4383A4, 700, 1 },
    { "assets/obseg/bg/bg_sev_all_p.bin", 0x438660, 69104, 0 },
    { "assets/obseg/bg/bg_silo_all_p.bin", 0x449450, 331584, 0 },
    { "assets/obseg/bg/bg_stat_all_p.bin", 0x49A390, 139472, 0 },
    { "assets/obseg/bg/bg_arec_all_p.bin", 0x4BC460, 189312, 0 },
    { "assets/obseg/bg/bg_arch_all_p.bin", 0x4EA7E0, 154352, 0 },
    { "assets/obseg/bg/bg_tra_all_p.bin", 0x5102D0, 132464, 0 },
    { "assets/obseg/bg/bg_dest_all_p.bin", 0x530840, 186816, 0 },
    { "assets/obseg/bg/bg_sevb_all_p.bin", 0x55E200, 109984, 0 },
    { "assets/obseg/bg/bg_azt_all_p.bin", 0x578FA0, 137808, 0 },
    { "assets/obseg/bg/bg_pete_all_p.bin", 0x59A9F0, 105520, 0 },
    { "assets/obseg/bg/bg_depo_all_p.bin", 0x5B4620, 182640, 0 },
    { "assets/obseg/bg/bg_ref_all_p.bin", 0x5E0F90, 38416, 0 },
    { "assets/obseg/bg/bg_cryp_all_p.bin", 0x5EA5A0, 87728, 0 },
    { "assets/obseg/bg/bg_dam_all_p.bin", 0x5FFC50, 197024, 0 },
    { "assets/obseg/bg/bg_ark_all_p.bin", 0x62FDF0, 200576, 0 },
    { "assets/obseg/bg/bg_run_all_p.bin", 0x660D70, 41936, 0 },
    { "assets/obseg/bg/bg_sevx_all_p.bin", 0x66B140, 116176, 0 },
    { "assets/obseg/bg/bg_jun_all_p.bin", 0x687710, 86352, 0 },
    { "assets/obseg/bg/bg_dish_all_p.bin", 0x69C860, 18544, 0 },
    { "assets/obseg/bg/bg_cave_all_p.bin", 0x6A10D0, 148720, 0 },
    { "assets/obseg/bg/bg_cat_all_p.bin", 0x6C55C0, 21808, 0 },
    { "assets/obseg/bg/bg_crad_all_p.bin", 0x6CAAF0, 66384, 0 },
    { "assets/obseg/bg/bg_imp_all_p.bin", 0x6DAE40, 0, 0 },
    { "assets/obseg/bg/bg_ash_all_p.bin", 0x6DAE40, 0, 0 },
    { "assets/obseg/bg/bg_sho_all_p.bin", 0x6DAE40, 0, 0 },
    { "assets/obseg/bg/bg_ame_all_p.bin", 0x6DAE40, 40800, 0 },
    { "assets/obseg/bg/bg_rit_all_p.bin", 0x6E4DA0, 0, 0 },
    { "assets/obseg/bg/bg_oat_all_p.bin", 0x6E4DA0, 28240, 0 },
    { "assets/obseg/bg/bg_lip_all_p.bin", 0x6EBBF0, 0, 0 },
    { "assets/obseg/bg/bg_lee_all_p.bin", 0x6EBBF0, 0, 0 },
    { "assets/obseg/bg/bg_ear_all_p.bin", 0x6EBBF0, 0, 0 },
    { "assets/obseg/bg/bg_len_all_p.bin", 0x6EBBF0, 4000, 0 },
    { "assets/obseg/bg/bg_pam_all_p.bin", 0x6ECB90, 0, 0 },
    { "assets/obseg/bg/bg_wax_all_p.bin", 0x6ECB90, 0, 0 },
    { "assets/obseg/chr/CarmourguardZ.bin", 0x6ECB90, 9344, 1 },
    { "assets/obseg/chr/CbaronsamediZ.bin", 0x6EF010, 14064, 1 },
    { "assets/obseg/chr/CbluecamguardZ.bin", 0x6F2700, 9968, 1 },
    { "assets/obseg/chr/CbluemanZ.bin", 0x6F4DF0, 7696, 1 },
    { "assets/obseg/chr/CbluewomanZ.bin", 0x6F6C00, 7872, 1 },
    { "assets/obseg/chr/CboilerbondZ.bin", 0x6F8AC0, 11280, 1 },
    { "assets/obseg/chr/CboilertrevZ.bin", 0x6FB6D0, 13952, 1 },
    { "assets/obseg/chr/CborisZ.bin", 0x6FED50, 12576, 1 },
    { "assets/obseg/chr/CcamguardZ.bin", 0x701E70, 9952, 1 },
    { "assets/obseg/chr/CcardimanZ.bin", 0x704550, 7680, 1 },
    { "assets/obseg/chr/CcheckmanZ.bin", 0x706350, 7808, 1 },
    { "assets/obseg/chr/CcommguardZ.bin", 0x7081D0, 9728, 1 },
    { "assets/obseg/chr/CdjbondZ.bin", 0x70A7D0, 11920, 1 },
    { "assets/obseg/chr/CfattechwomanZ.bin", 0x70D660, 7968, 1 },
    { "assets/obseg/chr/Cgreatguard2Z.bin", 0x70F580, 10016, 1 },
    { "assets/obseg/chr/CgreatguardZ.bin", 0x711CA0, 9856, 1 },
    { "assets/obseg/chr/CgreyguardZ.bin", 0x714320, 9936, 1 },
    { "assets/obseg/chr/CgreymanZ.bin", 0x7169F0, 7616, 1 },
    { "assets/obseg/chr/CheadalanZ.bin", 0x7187B0, 1488, 1 },
    { "assets/obseg/chr/CheadbZ.bin", 0x718D80, 1392, 1 },
    { "assets/obseg/chr/CheadbalaclavaZ.bin", 0x7192F0, 976, 1 },
    { "assets/obseg/chr/CheadbikeZ.bin", 0x7196C0, 1696, 1 },
    { "assets/obseg/chr/CheadbrosnanZ.bin", 0x719D60, 3408, 1 },
    { "assets/obseg/chr/CheadbrosnanboilerZ.bin", 0x71AAB0, 2976, 1 },
    { "assets/obseg/chr/CheadbrosnansnowZ.bin", 0x71B650, 3712, 1 },
    { "assets/obseg/chr/CheadbrosnansuitZ.bin", 0x71C4D0, 3456, 1 },
    { "assets/obseg/chr/CheadbrosnantimberZ.bin", 0x71D250, 3008, 1 },
    { "assets/obseg/chr/CheadchrisZ.bin", 0x71DE10, 1344, 1 },
    { "assets/obseg/chr/CheaddaveZ.bin", 0x71E350, 1408, 1 },
    { "assets/obseg/chr/CheaddesZ.bin", 0x71E8D0, 1328, 1 },
    { "assets/obseg/chr/CheadduncanZ.bin", 0x71EE00, 1312, 1 },
    { "assets/obseg/chr/CheaddwayneZ.bin", 0x71F320, 1408, 1 },
    { "assets/obseg/chr/CheadgrahamZ.bin", 0x71F8A0, 1392, 1 },
    { "assets/obseg/chr/CheadgrantZ.bin", 0x71FE10, 1328, 1 },
    { "assets/obseg/chr/CheadjimZ.bin", 0x720340, 1328, 1 },
    { "assets/obseg/chr/Cheadjoe2Z.bin", 0x720870, 1056, 1 },
    { "assets/obseg/chr/CheadjoeZ.bin", 0x720C90, 1392, 1 },
    { "assets/obseg/chr/CheadjoelZ.bin", 0x721200, 1296, 1 },
    { "assets/obseg/chr/CheadkarlZ.bin", 0x721710, 1376, 1 },
    { "assets/obseg/chr/CheadkenZ.bin", 0x721C70, 1360, 1 },
    { "assets/obseg/chr/CheadleeZ.bin", 0x7221C0, 1408, 1 },
    { "assets/obseg/chr/CheadmandyZ.bin", 0x722740, 1008, 1 },
    { "assets/obseg/chr/CheadmarionZ.bin", 0x722B30, 1040, 1 },
    { "assets/obseg/chr/CheadmarkZ.bin", 0x722F40, 1328, 1 },
    { "assets/obseg/chr/CheadmartinZ.bin", 0x723470, 1376, 1 },
    { "assets/obseg/chr/CheadmishkinZ.bin", 0x7239D0, 1376, 1 },
    { "assets/obseg/chr/CheadneilZ.bin", 0x723F30, 1312, 1 },
    { "assets/obseg/chr/CheadpeteZ.bin", 0x724450, 1424, 1 },
    { "assets/obseg/chr/CheadrobinZ.bin", 0x7249E0, 1296, 1 },
    { "assets/obseg/chr/CheadsallyZ.bin", 0x724EF0, 1024, 1 },
    { "assets/obseg/chr/CheadscottZ.bin", 0x7252F0, 1408, 1 },
    { "assets/obseg/chr/CheadshaunZ.bin", 0x725870, 1504, 1 },
    { "assets/obseg/chr/CheadsteveeZ.bin", 0x725E50, 1360, 1 },
    { "assets/obseg/chr/CheadstevehZ.bin", 0x7263A0, 1360, 1 },
    { "assets/obseg/chr/CheadvivienZ.bin", 0x7268F0, 1168, 1 },
    { "assets/obseg/chr/CjawsZ.bin", 0x726D80, 11328, 1 },
    { "assets/obseg/chr/CjeanwomanZ.bin", 0x7299C0, 8208, 1 },
    { "assets/obseg/chr/CmaydayZ.bin", 0x72B9D0, 11168, 1 },
    { "assets/obseg/chr/CmoonfemaleZ.bin", 0x72E570, 8528, 1 },
    { "assets/obseg/chr/CmoonguardZ.bin", 0x7306C0, 9712, 1 },
    { "assets/obseg/chr/CnatalyaZ.bin", 0x732CB0, 14528, 1 },
    { "assets/obseg/chr/CnavyguardZ.bin", 0x736570, 9952, 1 },
    { "assets/obseg/chr/CoddjobZ.bin", 0x738C50, 12592, 1 },
    { "assets/obseg/chr/ColiveguardZ.bin", 0x73BD80, 9808, 1 },
    { "assets/obseg/chr/CorumovZ.bin", 0x73E3D0, 13360, 1 },
    { "assets/obseg/chr/CpilotZ.bin", 0x741800, 12416, 1 },
    { "assets/obseg/chr/CredmanZ.bin", 0x744880, 10032, 1 },
    { "assets/obseg/chr/CrusguardZ.bin", 0x746FB0, 7216, 1 },
    { "assets/obseg/chr/CsnowbondZ.bin", 0x748BE0, 13104, 1 },
    { "assets/obseg/chr/CsnowguardZ.bin", 0x74BF10, 11392, 1 },
    { "assets/obseg/chr/CspicebondZ.bin", 0x74EB90, 14624, 1 },
    { "assets/obseg/chr/Csuit_lf_handZ.bin", 0x7524B0, 12832, 1 },
    { "assets/obseg/chr/CsuitbondZ.bin", 0x7556D0, 11664, 1 },
    { "assets/obseg/chr/CtechmanZ.bin", 0x758460, 9904, 1 },
    { "assets/obseg/chr/CtechwomanZ.bin", 0x75AB10, 8176, 1 },
    { "assets/obseg/chr/CtimberbondZ.bin", 0x75CB00, 11568, 1 },
    { "assets/obseg/chr/CtrevelyanZ.bin", 0x75F830, 14288, 1 },
    { "assets/obseg/chr/CtrevguardZ.bin", 0x763000, 9744, 1 },
    { "assets/obseg/chr/CvalentinZ.bin", 0x765610, 12144, 1 },
    { "assets/obseg/chr/CxeniaZ.bin", 0x768580, 14832, 1 },
    { "assets/obseg/gun/Gak47Z.bin", 0x76BF70, 2576, 1 },
    { "assets/obseg/gun/GaudiotapeZ.bin", 0x76C980, 912, 1 },
    { "assets/obseg/gun/GautoshotZ.bin", 0x76CD10, 6160, 1 },
    { "assets/obseg/gun/GblackboxZ.bin", 0x76E520, 1536, 1 },
    { "assets/obseg/gun/GblueprintsZ.bin", 0x76EB20, 256, 1 },
    { "assets/obseg/gun/GbombcaseZ.bin", 0x76EC20, 1936, 1 },
    { "assets/obseg/gun/GbombdefuserZ.bin", 0x76F3B0, 1520, 1 },
    { "assets/obseg/gun/GbriefcaseZ.bin", 0x76F9A0, 1936, 1 },
    { "assets/obseg/gun/GbugZ.bin", 0x770130, 2416, 1 },
    { "assets/obseg/gun/GbugdetectorZ.bin", 0x770AA0, 848, 1 },
    { "assets/obseg/gun/GbungeeZ.bin", 0x770DF0, 848, 1 },
    { "assets/obseg/gun/GcameraZ.bin", 0x771140, 1152, 1 },
    { "assets/obseg/gun/GcartblueZ.bin", 0x7715C0, 608, 1 },
    { "assets/obseg/gun/GcartridgeZ.bin", 0x771820, 304, 1 },
    { "assets/obseg/gun/GcartrifleZ.bin", 0x771950, 528, 1 },
    { "assets/obseg/gun/GcartshellZ.bin", 0x771B60, 512, 1 },
    { "assets/obseg/gun/GcircuitboardZ.bin", 0x771D60, 320, 1 },
    { "assets/obseg/gun/GclipboardZ.bin", 0x771EA0, 496, 1 },
    { "assets/obseg/gun/GcreditcardZ.bin", 0x772090, 848, 1 },
    { "assets/obseg/gun/GdarkglassesZ.bin", 0x7723E0, 848, 1 },
    { "assets/obseg/gun/GdatathiefZ.bin", 0x772730, 320, 1 },
    { "assets/obseg/gun/GdattapeZ.bin", 0x772870, 368, 1 },
    { "assets/obseg/gun/GdoordecoderZ.bin", 0x7729E0, 1408, 1 },
    { "assets/obseg/gun/GdoorexploderZ.bin", 0x772F60, 848, 1 },
    { "assets/obseg/gun/GdossierredZ.bin", 0x7732B0, 864, 1 },
    { "assets/obseg/gun/GdynamiteZ.bin", 0x773610, 848, 1 },
    { "assets/obseg/gun/GexplosivefloppyZ.bin", 0x773960, 592, 1 },
    { "assets/obseg/gun/GexplosivepenZ.bin", 0x773BB0, 848, 1 },
    { "assets/obseg/gun/GextinguisherZ.bin", 0x773F00, 2032, 1 },
    { "assets/obseg/gun/GfingergunZ.bin", 0x7746F0, 848, 1 },
    { "assets/obseg/gun/GfistZ.bin", 0x774A40, 5888, 1 },
    { "assets/obseg/gun/GflarepistolZ.bin", 0x776140, 848, 1 },
    { "assets/obseg/gun/Gfnp90Z.bin", 0x776490, 3232, 1 },
    { "assets/obseg/gun/GgaskeyringZ.bin", 0x777130, 2608, 1 },
    { "assets/obseg/gun/GgoldbarZ.bin", 0x777B60, 848, 1 },
    { "assets/obseg/gun/GgoldeneyekeyZ.bin", 0x777EB0, 2480, 1 },
    { "assets/obseg/gun/GgoldengunZ.bin", 0x778860, 6112, 1 },
    { "assets/obseg/gun/GgoldwppkZ.bin", 0x77A040, 6496, 1 },
    { "assets/obseg/gun/GgrenadeZ.bin", 0x77B9A0, 2608, 1 },
    { "assets/obseg/gun/GgrenadelaunchZ.bin", 0x77C3D0, 4224, 1 },
    { "assets/obseg/gun/GheroinZ.bin", 0x77D450, 848, 1 },
    { "assets/obseg/gun/GjoypadZ.bin", 0x77D7A0, 7856, 1 },
    { "assets/obseg/gun/GkeyanalysercaseZ.bin", 0x77F650, 1936, 1 },
    { "assets/obseg/gun/GkeyboltZ.bin", 0x77FDE0, 2544, 1 },
    { "assets/obseg/gun/GkeycardZ.bin", 0x7807D0, 304, 1 },
    { "assets/obseg/gun/GkeyyaleZ.bin", 0x780900, 3408, 1 },
    { "assets/obseg/gun/GknifeZ.bin", 0x781650, 6864, 1 },
    { "assets/obseg/gun/GlaserZ.bin", 0x783120, 3568, 1 },
    { "assets/obseg/gun/GlectreZ.bin", 0x783F10, 848, 1 },
    { "assets/obseg/gun/GlockexploderZ.bin", 0x784260, 848, 1 },
    { "assets/obseg/gun/Gm16Z.bin", 0x7845B0, 2592, 1 },
    { "assets/obseg/gun/GmapZ.bin", 0x784FD0, 240, 1 },
    { "assets/obseg/gun/GmicrocameraZ.bin", 0x7850C0, 1600, 1 },
    { "assets/obseg/gun/GmicrocodeZ.bin", 0x785700, 848, 1 },
    { "assets/obseg/gun/GmicrofilmZ.bin", 0x785A50, 848, 1 },
    { "assets/obseg/gun/GmoneyZ.bin", 0x785DA0, 848, 1 },
    { "assets/obseg/gun/Gmp5kZ.bin", 0x7860F0, 3040, 1 },
    { "assets/obseg/gun/Gmp5ksilZ.bin", 0x786CD0, 3328, 1 },
    { "assets/obseg/gun/GpitongunZ.bin", 0x7879D0, 848, 1 },
    { "assets/obseg/gun/GplansZ.bin", 0x787D20, 464, 1 },
    { "assets/obseg/gun/GplastiqueZ.bin", 0x787EF0, 848, 1 },
    { "assets/obseg/gun/GpolarizedglassesZ.bin", 0x788240, 1376, 1 },
    { "assets/obseg/gun/GproximitymineZ.bin", 0x7887A0, 2032, 1 },
    { "assets/obseg/gun/GremotemineZ.bin", 0x788F90, 2496, 1 },
    { "assets/obseg/gun/GrocketlaunchZ.bin", 0x789950, 4640, 1 },
    { "assets/obseg/gun/GrugerZ.bin", 0x78AB70, 7568, 1 },
    { "assets/obseg/gun/GsafecrackercaseZ.bin", 0x78C900, 1936, 1 },
    { "assets/obseg/gun/GshotgunZ.bin", 0x78D090, 3808, 1 },
    { "assets/obseg/gun/GsilverwppkZ.bin", 0x78DF70, 6496, 1 },
    { "assets/obseg/gun/GskorpionZ.bin", 0x78F8D0, 4608, 1 },
    { "assets/obseg/gun/GsniperrifleZ.bin", 0x790AD0, 4208, 1 },
    { "assets/obseg/gun/GspectreZ.bin", 0x791B40, 3200, 1 },
    { "assets/obseg/gun/GspooltapeZ.bin", 0x7927C0, 848, 1 },
    { "assets/obseg/gun/GspyfileZ.bin", 0x792B10, 848, 1 },
    { "assets/obseg/gun/GstafflistZ.bin", 0x792E60, 416, 1 },
    { "assets/obseg/gun/GtaserZ.bin", 0x793000, 7776, 1 },
    { "assets/obseg/gun/GthrowknifeZ.bin", 0x794E60, 6896, 1 },
    { "assets/obseg/gun/GtimedmineZ.bin", 0x796950, 2752, 1 },
    { "assets/obseg/gun/GtriggerZ.bin", 0x797410, 13312, 1 },
    { "assets/obseg/gun/Gtt33Z.bin", 0x79A810, 6944, 1 },
    { "assets/obseg/gun/GuziZ.bin", 0x79C330, 2320, 1 },
    { "assets/obseg/gun/GvideotapeZ.bin", 0x79CC40, 528, 1 },
    { "assets/obseg/gun/GwatchcommunicatorZ.bin", 0x79CE50, 5216, 1 },
    { "assets/obseg/gun/GwatchgeigercounterZ.bin", 0x79E2B0, 5216, 1 },
    { "assets/obseg/gun/GwatchidentifierZ.bin", 0x79F710, 5216, 1 },
    { "assets/obseg/gun/GwatchlaserZ.bin", 0x7A0B70, 13312, 1 },
    { "assets/obseg/gun/GwatchmagnetattractZ.bin", 0x7A3F70, 5200, 1 },
    { "assets/obseg/gun/GwatchmagnetrepelZ.bin", 0x7A53C0, 5216, 1 },
    { "assets/obseg/gun/GweaponcaseZ.bin", 0x7A6820, 1936, 1 },
    { "assets/obseg/gun/GwppkZ.bin", 0x7A6FB0, 7312, 1 },
    { "assets/obseg/gun/GwppksilZ.bin", 0x7A8C40, 7488, 1 },
    { "assets/obseg/gun/GwristdartZ.bin", 0x7AA980, 848, 1 },
    { "assets/obseg/prop/PICBMZ.bin", 0x7AACD0, 9600, 1 },
    { "assets/obseg/prop/PICBM_noseZ.bin", 0x7AD250, 1968, 1 },
    { "assets/obseg/prop/Pak47magZ.bin", 0x7ADA00, 480, 1 },
    { "assets/obseg/prop/Palarm1Z.bin", 0x7ADBE0, 352, 1 },
    { "assets/obseg/prop/Palarm2Z.bin", 0x7ADD40, 416, 1 },
    { "assets/obseg/prop/Pammo_crate1Z.bin", 0x7ADEE0, 576, 1 },
    { "assets/obseg/prop/Pammo_crate2Z.bin", 0x7AE120, 576, 1 },
    { "assets/obseg/prop/Pammo_crate3Z.bin", 0x7AE360, 592, 1 },
    { "assets/obseg/prop/Pammo_crate4Z.bin", 0x7AE5B0, 624, 1 },
    { "assets/obseg/prop/Pammo_crate5Z.bin", 0x7AE820, 704, 1 },
    { "assets/obseg/prop/PapcZ.bin", 0x7AEAE0, 7264, 1 },
    { "assets/obseg/prop/Parchsecdoor1Z.bin", 0x7B0740, 480, 1 },
    { "assets/obseg/prop/Parchsecdoor2Z.bin", 0x7B0920, 464, 1 },
    { "assets/obseg/prop/ParticZ.bin", 0x7B0AF0, 3584, 1 },
    { "assets/obseg/prop/PartictrailerZ.bin", 0x7B18F0, 2320, 1 },
    { "assets/obseg/prop/PbarricadeZ.bin", 0x7B2200, 576, 1 },
    { "assets/obseg/prop/Pbin1Z.bin", 0x7B2440, 848, 1 },
    { "assets/obseg/prop/Pblotter1Z.bin", 0x7B2790, 224, 1 },
    { "assets/obseg/prop/PbodyarmourZ.bin", 0x7B2870, 1184, 1 },
    { "assets/obseg/prop/PbodyarmourvestZ.bin", 0x7B2D10, 1056, 1 },
    { "assets/obseg/prop/PbollardZ.bin", 0x7B3130, 512, 1 },
    { "assets/obseg/prop/PbombZ.bin", 0x7B3330, 368, 1 },
    { "assets/obseg/prop/Pbook1Z.bin", 0x7B34A0, 400, 1 },
    { "assets/obseg/prop/Pbookshelf1Z.bin", 0x7B3630, 1776, 1 },
    { "assets/obseg/prop/Pborg_crateZ.bin", 0x7B3D20, 368, 1 },
    { "assets/obseg/prop/PboxcartridgesZ.bin", 0x7B3E90, 512, 1 },
    { "assets/obseg/prop/Pboxes2x4Z.bin", 0x7B4090, 1072, 1 },
    { "assets/obseg/prop/Pboxes3x4Z.bin", 0x7B44C0, 1088, 1 },
    { "assets/obseg/prop/Pboxes4x4Z.bin", 0x7B4900, 1632, 1 },
    { "assets/obseg/prop/PbrakeunitZ.bin", 0x7B4F60, 880, 1 },
    { "assets/obseg/prop/Pbridge_console1aZ.bin", 0x7B52D0, 1408, 1 },
    { "assets/obseg/prop/Pbridge_console1bZ.bin", 0x7B5850, 1376, 1 },
    { "assets/obseg/prop/Pbridge_console2aZ.bin", 0x7B5DB0, 1408, 1 },
    { "assets/obseg/prop/Pbridge_console2bZ.bin", 0x7B6330, 1264, 1 },
    { "assets/obseg/prop/Pbridge_console3aZ.bin", 0x7B6820, 1360, 1 },
    { "assets/obseg/prop/Pbridge_console3bZ.bin", 0x7B6D70, 1424, 1 },
    { "assets/obseg/prop/PcarbmwZ.bin", 0x7B7300, 3296, 1 },
    { "assets/obseg/prop/Pcard_box1Z.bin", 0x7B7FE0, 512, 1 },
    { "assets/obseg/prop/Pcard_box2Z.bin", 0x7B81E0, 576, 1 },
    { "assets/obseg/prop/Pcard_box3Z.bin", 0x7B8420, 496, 1 },
    { "assets/obseg/prop/Pcard_box4_lgZ.bin", 0x7B8610, 432, 1 },
    { "assets/obseg/prop/Pcard_box5_lgZ.bin", 0x7B87C0, 512, 1 },
    { "assets/obseg/prop/Pcard_box6_lgZ.bin", 0x7B89C0, 496, 1 },
    { "assets/obseg/prop/PcarescortZ.bin", 0x7B8BB0, 3072, 1 },
    { "assets/obseg/prop/PcargolfZ.bin", 0x7B97B0, 3120, 1 },
    { "assets/obseg/prop/PcarweirdZ.bin", 0x7BA3E0, 4416, 1 },
    { "assets/obseg/prop/PcarzilZ.bin", 0x7BB520, 5632, 1 },
    { "assets/obseg/prop/PcctvZ.bin", 0x7BCB20, 896, 1 },
    { "assets/obseg/prop/PchraudiotapeZ.bin", 0x7BCEA0, 1376, 1 },
    { "assets/obseg/prop/PchrautoshotZ.bin", 0x7BD400, 864, 1 },
    { "assets/obseg/prop/PchrblackboxZ.bin", 0x7BD760, 2176, 1 },
    { "assets/obseg/prop/PchrblueprintsZ.bin", 0x7BDFE0, 336, 1 },
    { "assets/obseg/prop/PchrbombcaseZ.bin", 0x7BE130, 496, 1 },
    { "assets/obseg/prop/PchrbombdefuserZ.bin", 0x7BE320, 2272, 1 },
    { "assets/obseg/prop/PchrbriefcaseZ.bin", 0x7BEC00, 400, 1 },
    { "assets/obseg/prop/PchrbugZ.bin", 0x7BED90, 3504, 1 },
    { "assets/obseg/prop/PchrbugdetectorZ.bin", 0x7BFB40, 368, 1 },
    { "assets/obseg/prop/PchrbungeeZ.bin", 0x7BFCB0, 368, 1 },
    { "assets/obseg/prop/PchrcameraZ.bin", 0x7BFE20, 1680, 1 },
    { "assets/obseg/prop/PchrcircuitboardZ.bin", 0x7C04B0, 416, 1 },
    { "assets/obseg/prop/PchrclipboardZ.bin", 0x7C0650, 640, 1 },
    { "assets/obseg/prop/PchrcreditcardZ.bin", 0x7C08D0, 368, 1 },
    { "assets/obseg/prop/PchrdarkglassesZ.bin", 0x7C0A40, 368, 1 },
    { "assets/obseg/prop/PchrdatathiefZ.bin", 0x7C0BB0, 416, 1 },
    { "assets/obseg/prop/PchrdattapeZ.bin", 0x7C0D50, 496, 1 },
    { "assets/obseg/prop/PchrdoordecoderZ.bin", 0x7C0F40, 2144, 1 },
    { "assets/obseg/prop/PchrdoorexploderZ.bin", 0x7C17A0, 368, 1 },
    { "assets/obseg/prop/PchrdossierredZ.bin", 0x7C1910, 1232, 1 },
    { "assets/obseg/prop/PchrdynamiteZ.bin", 0x7C1DE0, 368, 1 },
    { "assets/obseg/prop/PchrexplosivepenZ.bin", 0x7C1F50, 368, 1 },
    { "assets/obseg/prop/PchrextinguisherZ.bin", 0x7C20C0, 1280, 1 },
    { "assets/obseg/prop/PchrfingergunZ.bin", 0x7C25C0, 368, 1 },
    { "assets/obseg/prop/PchrflarepistolZ.bin", 0x7C2730, 368, 1 },
    { "assets/obseg/prop/Pchrfnp90Z.bin", 0x7C28A0, 1120, 1 },
    { "assets/obseg/prop/PchrgaskeyringZ.bin", 0x7C2D00, 3856, 1 },
    { "assets/obseg/prop/PchrgoldbarZ.bin", 0x7C3C10, 368, 1 },
    { "assets/obseg/prop/PchrgoldenZ.bin", 0x7C3D80, 624, 1 },
    { "assets/obseg/prop/PchrgoldeneyekeyZ.bin", 0x7C3FF0, 3744, 1 },
    { "assets/obseg/prop/PchrgoldwppkZ.bin", 0x7C4E90, 368, 1 },
    { "assets/obseg/prop/PchrgrenadeZ.bin", 0x7C5000, 880, 1 },
    { "assets/obseg/prop/PchrgrenadelaunchZ.bin", 0x7C5370, 912, 1 },
    { "assets/obseg/prop/PchrgrenaderoundZ.bin", 0x7C5700, 624, 1 },
    { "assets/obseg/prop/PchrheroinZ.bin", 0x7C5970, 368, 1 },
    { "assets/obseg/prop/PchrkalashZ.bin", 0x7C5AE0, 1008, 1 },
    { "assets/obseg/prop/PchrkeyanalysercaseZ.bin", 0x7C5ED0, 496, 1 },
    { "assets/obseg/prop/PchrkeyboltZ.bin", 0x7C60C0, 3744, 1 },
    { "assets/obseg/prop/PchrkeyyaleZ.bin", 0x7C6F60, 5216, 1 },
    { "assets/obseg/prop/PchrknifeZ.bin", 0x7C83C0, 512, 1 },
    { "assets/obseg/prop/PchrlaserZ.bin", 0x7C85C0, 960, 1 },
    { "assets/obseg/prop/PchrlectreZ.bin", 0x7C8980, 368, 1 },
    { "assets/obseg/prop/PchrlockexploderZ.bin", 0x7C8AF0, 368, 1 },
    { "assets/obseg/prop/Pchrm16Z.bin", 0x7C8C60, 976, 1 },
    { "assets/obseg/prop/PchrmapZ.bin", 0x7C9030, 336, 1 },
    { "assets/obseg/prop/PchrmicrocameraZ.bin", 0x7C9180, 2288, 1 },
    { "assets/obseg/prop/PchrmicrocodeZ.bin", 0x7C9A70, 368, 1 },
    { "assets/obseg/prop/PchrmicrofilmZ.bin", 0x7C9BE0, 368, 1 },
    { "assets/obseg/prop/PchrmoneyZ.bin", 0x7C9D50, 368, 1 },
    { "assets/obseg/prop/Pchrmp5kZ.bin", 0x7C9EC0, 896, 1 },
    { "assets/obseg/prop/Pchrmp5ksilZ.bin", 0x7CA240, 1040, 1 },
    { "assets/obseg/prop/PchrpitongunZ.bin", 0x7CA650, 368, 1 },
    { "assets/obseg/prop/PchrplansZ.bin", 0x7CA7C0, 656, 1 },
    { "assets/obseg/prop/PchrplastiqueZ.bin", 0x7CAA50, 1120, 1 },
    { "assets/obseg/prop/PchrpolarizedglassesZ.bin", 0x7CAEB0, 2240, 1 },
    { "assets/obseg/prop/PchrproximitymineZ.bin", 0x7CB770, 1120, 1 },
    { "assets/obseg/prop/PchrremotemineZ.bin", 0x7CBBD0, 1120, 1 },
    { "assets/obseg/prop/PchrrocketZ.bin", 0x7CC030, 1456, 1 },
    { "assets/obseg/prop/PchrrocketlaunchZ.bin", 0x7CC5E0, 992, 1 },
    { "assets/obseg/prop/PchrrugerZ.bin", 0x7CC9C0, 992, 1 },
    { "assets/obseg/prop/PchrsafecrackercaseZ.bin", 0x7CCDA0, 496, 1 },
    { "assets/obseg/prop/PchrshotgunZ.bin", 0x7CCF90, 848, 1 },
    { "assets/obseg/prop/PchrsilverwppkZ.bin", 0x7CD2E0, 368, 1 },
    { "assets/obseg/prop/PchrskorpionZ.bin", 0x7CD450, 896, 1 },
    { "assets/obseg/prop/PchrsniperrifleZ.bin", 0x7CD7D0, 912, 1 },
    { "assets/obseg/prop/PchrspectreZ.bin", 0x7CDB60, 880, 1 },
    { "assets/obseg/prop/PchrspooltapeZ.bin", 0x7CDED0, 368, 1 },
    { "assets/obseg/prop/PchrspyfileZ.bin", 0x7CE040, 368, 1 },
    { "assets/obseg/prop/PchrstafflistZ.bin", 0x7CE1B0, 544, 1 },
    { "assets/obseg/prop/PchrtesttubeZ.bin", 0x7CE3D0, 448, 1 },
    { "assets/obseg/prop/PchrthrowknifeZ.bin", 0x7CE590, 544, 1 },
    { "assets/obseg/prop/PchrtimedmineZ.bin", 0x7CE7B0, 1328, 1 },
    { "assets/obseg/prop/Pchrtt33Z.bin", 0x7CECE0, 656, 1 },
    { "assets/obseg/prop/PchruziZ.bin", 0x7CEF70, 720, 1 },
    { "assets/obseg/prop/PchrvideotapeZ.bin", 0x7CF240, 720, 1 },
    { "assets/obseg/prop/PchrweaponcaseZ.bin", 0x7CF510, 512, 1 },
    { "assets/obseg/prop/PchrwppkZ.bin", 0x7CF710, 576, 1 },
    { "assets/obseg/prop/PchrwppksilZ.bin", 0x7CF950, 736, 1 },
    { "assets/obseg/prop/PchrwristdartZ.bin", 0x7CFC30, 368, 1 },
    { "assets/obseg/prop/Pconsole1Z.bin", 0x7CFDA0, 1664, 1 },
    { "assets/obseg/prop/Pconsole2Z.bin", 0x7D0420, 1664, 1 },
    { "assets/obseg/prop/Pconsole3Z.bin", 0x7D0AA0, 1680, 1 },
    { "assets/obseg/prop/Pconsole_sev2aZ.bin", 0x7D1130, 1056, 1 },
    { "assets/obseg/prop/Pconsole_sev2bZ.bin", 0x7D1550, 1216, 1 },
    { "assets/obseg/prop/Pconsole_sev2cZ.bin", 0x7D1A10, 1088, 1 },
    { "assets/obseg/prop/Pconsole_sev2dZ.bin", 0x7D1E50, 1072, 1 },
    { "assets/obseg/prop/Pconsole_sev_GEaZ.bin", 0x7D2280, 1072, 1 },
    { "assets/obseg/prop/Pconsole_sev_GEbZ.bin", 0x7D26B0, 1072, 1 },
    { "assets/obseg/prop/Pconsole_sevaZ.bin", 0x7D2AE0, 1152, 1 },
    { "assets/obseg/prop/Pconsole_sevbZ.bin", 0x7D2F60, 1136, 1 },
    { "assets/obseg/prop/Pconsole_sevcZ.bin", 0x7D33D0, 1072, 1 },
    { "assets/obseg/prop/Pconsole_sevdZ.bin", 0x7D3800, 1072, 1 },
    { "assets/obseg/prop/Pcryptdoor1aZ.bin", 0x7D3C30, 400, 1 },
    { "assets/obseg/prop/Pcryptdoor1bZ.bin", 0x7D3DC0, 400, 1 },
    { "assets/obseg/prop/Pcryptdoor2aZ.bin", 0x7D3F50, 400, 1 },
    { "assets/obseg/prop/Pcryptdoor2bZ.bin", 0x7D40E0, 400, 1 },
    { "assets/obseg/prop/Pcryptdoor3Z.bin", 0x7D4270, 624, 1 },
    { "assets/obseg/prop/Pcryptdoor4Z.bin", 0x7D44E0, 384, 1 },
    { "assets/obseg/prop/PdamchaindoorZ.bin", 0x7D4660, 640, 1 },
    { "assets/obseg/prop/PdamgatedoorZ.bin", 0x7D48E0, 544, 1 },
    { "assets/obseg/prop/PdamtundoorZ.bin", 0x7D4B00, 880, 1 },
    { "assets/obseg/prop/Pdepot_door_steelZ.bin", 0x7D4E70, 416, 1 },
    { "assets/obseg/prop/Pdepot_gate_entryZ.bin", 0x7D5010, 576, 1 },
    { "assets/obseg/prop/Pdesk1Z.bin", 0x7D5250, 384, 1 },
    { "assets/obseg/prop/Pdesk2Z.bin", 0x7D53D0, 384, 1 },
    { "assets/obseg/prop/Pdesk_arecibo1Z.bin", 0x7D5550, 576, 1 },
    { "assets/obseg/prop/Pdesk_lamp2Z.bin", 0x7D5790, 768, 1 },
    { "assets/obseg/prop/Pdest_engineZ.bin", 0x7D5A90, 6384, 1 },
    { "assets/obseg/prop/Pdest_exocetZ.bin", 0x7D7380, 1632, 1 },
    { "assets/obseg/prop/Pdest_gunZ.bin", 0x7D79E0, 1648, 1 },
    { "assets/obseg/prop/Pdest_harpoonZ.bin", 0x7D8050, 2208, 1 },
    { "assets/obseg/prop/Pdest_seawolfZ.bin", 0x7D88F0, 4016, 1 },
    { "assets/obseg/prop/Pdisc_readerZ.bin", 0x7D98A0, 448, 1 },
    { "assets/obseg/prop/Pdisk_drive1Z.bin", 0x7D9A60, 400, 1 },
    { "assets/obseg/prop/Pdoor_azt_chairZ.bin", 0x7D9BF0, 384, 1 },
    { "assets/obseg/prop/Pdoor_azt_deskZ.bin", 0x7D9D70, 1088, 1 },
    { "assets/obseg/prop/Pdoor_azt_desk_topZ.bin", 0x7DA1B0, 912, 1 },
    { "assets/obseg/prop/Pdoor_aztecZ.bin", 0x7DA540, 560, 1 },
    { "assets/obseg/prop/Pdoor_dest1Z.bin", 0x7DA770, 768, 1 },
    { "assets/obseg/prop/Pdoor_dest2Z.bin", 0x7DAA70, 960, 1 },
    { "assets/obseg/prop/Pdoor_eyelidZ.bin", 0x7DAE30, 1376, 1 },
    { "assets/obseg/prop/Pdoor_irisZ.bin", 0x7DB390, 2640, 1 },
    { "assets/obseg/prop/Pdoor_mfZ.bin", 0x7DBDE0, 752, 1 },
    { "assets/obseg/prop/Pdoor_roller1Z.bin", 0x7DC0D0, 880, 1 },
    { "assets/obseg/prop/Pdoor_roller2Z.bin", 0x7DC440, 576, 1 },
    { "assets/obseg/prop/Pdoor_roller3Z.bin", 0x7DC680, 576, 1 },
    { "assets/obseg/prop/Pdoor_roller4Z.bin", 0x7DC8C0, 608, 1 },
    { "assets/obseg/prop/Pdoor_rollertrainZ.bin", 0x7DCB20, 304, 1 },
    { "assets/obseg/prop/Pdoor_st_arec1Z.bin", 0x7DCC50, 608, 1 },
    { "assets/obseg/prop/Pdoor_st_arec2Z.bin", 0x7DCEB0, 736, 1 },
    { "assets/obseg/prop/Pdoor_winZ.bin", 0x7DD190, 416, 1 },
    { "assets/obseg/prop/PdoorconsoleZ.bin", 0x7DD330, 1136, 1 },
    { "assets/obseg/prop/PdoorpanelZ.bin", 0x7DD7A0, 880, 1 },
    { "assets/obseg/prop/Pdoorprison1Z.bin", 0x7DDB10, 336, 1 },
    { "assets/obseg/prop/PdoorstatgateZ.bin", 0x7DDC60, 512, 1 },
    { "assets/obseg/prop/PexplosionbitZ.bin", 0x7DDE60, 288, 1 },
    { "assets/obseg/prop/Pfiling_cabinet1Z.bin", 0x7DDF80, 384, 1 },
    { "assets/obseg/prop/PflagZ.bin", 0x7DE100, 304, 1 },
    { "assets/obseg/prop/PfloppyZ.bin", 0x7DE230, 800, 1 },
    { "assets/obseg/prop/Pfnp90magZ.bin", 0x7DE550, 416, 1 },
    { "assets/obseg/prop/Pgas_plant_met1_do1Z.bin", 0x7DE6F0, 896, 1 },
    { "assets/obseg/prop/Pgas_plant_sw2_do1Z.bin", 0x7DEA70, 480, 1 },
    { "assets/obseg/prop/Pgas_plant_sw3_do1Z.bin", 0x7DEC50, 512, 1 },
    { "assets/obseg/prop/Pgas_plant_sw4_do1Z.bin", 0x7DEE50, 352, 1 },
    { "assets/obseg/prop/Pgas_plant_sw_do1Z.bin", 0x7DEFB0, 656, 1 },
    { "assets/obseg/prop/Pgas_plant_wc_cub1Z.bin", 0x7DF240, 528, 1 },
    { "assets/obseg/prop/PgasbarrelZ.bin", 0x7DF450, 528, 1 },
    { "assets/obseg/prop/PgasbarrelsZ.bin", 0x7DF660, 1344, 1 },
    { "assets/obseg/prop/Pgasplant_clear_doorZ.bin", 0x7DFBA0, 1376, 1 },
    { "assets/obseg/prop/PgastankZ.bin", 0x7E0100, 1456, 1 },
    { "assets/obseg/prop/Pglassware1Z.bin", 0x7E06B0, 352, 1 },
    { "assets/obseg/prop/Pglassware2Z.bin", 0x7E0810, 656, 1 },
    { "assets/obseg/prop/Pglassware3Z.bin", 0x7E0AA0, 528, 1 },
    { "assets/obseg/prop/Pglassware4Z.bin", 0x7E0CB0, 1408, 1 },
    { "assets/obseg/prop/PgoldeneyelogoZ.bin", 0x7E1230, 3760, 1 },
    { "assets/obseg/prop/PgoldenshellsZ.bin", 0x7E20E0, 512, 1 },
    { "assets/obseg/prop/PgroundgunZ.bin", 0x7E22E0, 2000, 1 },
    { "assets/obseg/prop/Pgun_runway1Z.bin", 0x7E2AB0, 1856, 1 },
    { "assets/obseg/prop/PhatberetZ.bin", 0x7E31F0, 672, 1 },
    { "assets/obseg/prop/PhatberetblueZ.bin", 0x7E3490, 720, 1 },
    { "assets/obseg/prop/PhatberetredZ.bin", 0x7E3760, 736, 1 },
    { "assets/obseg/prop/PhatchboltZ.bin", 0x7E3A40, 208, 1 },
    { "assets/obseg/prop/PhatchdoorZ.bin", 0x7E3B10, 544, 1 },
    { "assets/obseg/prop/PhatchsevxZ.bin", 0x7E3D30, 368, 1 },
    { "assets/obseg/prop/PhatfurryZ.bin", 0x7E3EA0, 560, 1 },
    { "assets/obseg/prop/PhatfurryblackZ.bin", 0x7E40D0, 544, 1 },
    { "assets/obseg/prop/PhatfurrybrownZ.bin", 0x7E42F0, 528, 1 },
    { "assets/obseg/prop/PhathelmetZ.bin", 0x7E4500, 560, 1 },
    { "assets/obseg/prop/PhathelmetgreyZ.bin", 0x7E4730, 560, 1 },
    { "assets/obseg/prop/PhatmoonZ.bin", 0x7E4960, 992, 1 },
    { "assets/obseg/prop/PhatpeakedZ.bin", 0x7E4D40, 784, 1 },
    { "assets/obseg/prop/PhattbirdZ.bin", 0x7E5050, 592, 1 },
    { "assets/obseg/prop/PhattbirdbrownZ.bin", 0x7E52A0, 624, 1 },
    { "assets/obseg/prop/PhelicopterZ.bin", 0x7E5510, 16928, 1 },
    { "assets/obseg/prop/PhindZ.bin", 0x7E9730, 6000, 1 },
    { "assets/obseg/prop/PjeepZ.bin", 0x7EAEA0, 4448, 1 },
    { "assets/obseg/prop/Pjerry_can1Z.bin", 0x7EC000, 608, 1 },
    { "assets/obseg/prop/Pjungle3_treeZ.bin", 0x7EC260, 1920, 1 },
    { "assets/obseg/prop/Pjungle5_treeZ.bin", 0x7EC9E0, 1328, 1 },
    { "assets/obseg/prop/Pkey_holderZ.bin", 0x7ECF10, 848, 1 },
    { "assets/obseg/prop/Pkeyboard1Z.bin", 0x7ED260, 368, 1 },
    { "assets/obseg/prop/Pkit_units1Z.bin", 0x7ED3D0, 672, 1 },
    { "assets/obseg/prop/PlabbenchZ.bin", 0x7ED670, 976, 1 },
    { "assets/obseg/prop/PlandmineZ.bin", 0x7EDA40, 624, 1 },
    { "assets/obseg/prop/PlegalpageZ.bin", 0x7EDCB0, 4032, 1 },
    { "assets/obseg/prop/Pletter_tray1Z.bin", 0x7EEC70, 352, 1 },
    { "assets/obseg/prop/Plocker3Z.bin", 0x7EEDD0, 400, 1 },
    { "assets/obseg/prop/Plocker4Z.bin", 0x7EEF60, 400, 1 },
    { "assets/obseg/prop/Pm16magZ.bin", 0x7EF0F0, 320, 1 },
    { "assets/obseg/prop/PmagnumshellsZ.bin", 0x7EF230, 512, 1 },
    { "assets/obseg/prop/Pmainframe1Z.bin", 0x7EF430, 768, 1 },
    { "assets/obseg/prop/Pmainframe2Z.bin", 0x7EF730, 720, 1 },
    { "assets/obseg/prop/Pmetal_chair1Z.bin", 0x7EFA00, 832, 1 },
    { "assets/obseg/prop/Pmetal_crate1Z.bin", 0x7EFD40, 448, 1 },
    { "assets/obseg/prop/Pmetal_crate2Z.bin", 0x7EFF00, 448, 1 },
    { "assets/obseg/prop/Pmetal_crate3Z.bin", 0x7F00C0, 448, 1 },
    { "assets/obseg/prop/Pmetal_crate4Z.bin", 0x7F0280, 448, 1 },
    { "assets/obseg/prop/PmilcopterZ.bin", 0x7F0440, 6368, 1 },
    { "assets/obseg/prop/PmiltruckZ.bin", 0x7F1D20, 8960, 1 },
    { "assets/obseg/prop/Pmissile_rack2Z.bin", 0x7F4020, 2576, 1 },
    { "assets/obseg/prop/Pmissile_rackZ.bin", 0x7F4A30, 992, 1 },
    { "assets/obseg/prop/PmodemboxZ.bin", 0x7F4E10, 832, 1 },
    { "assets/obseg/prop/PmotorbikeZ.bin", 0x7F5150, 3776, 1 },
    { "assets/obseg/prop/Pmp5kmagZ.bin", 0x7F6010, 336, 1 },
    { "assets/obseg/prop/PnintendologoZ.bin", 0x7F6160, 10976, 1 },
    { "assets/obseg/prop/Poil_drum1Z.bin", 0x7F8C40, 624, 1 },
    { "assets/obseg/prop/Poil_drum2Z.bin", 0x7F8EB0, 752, 1 },
    { "assets/obseg/prop/Poil_drum3Z.bin", 0x7F91A0, 752, 1 },
    { "assets/obseg/prop/Poil_drum5Z.bin", 0x7F9490, 752, 1 },
    { "assets/obseg/prop/Poil_drum6Z.bin", 0x7F9780, 784, 1 },
    { "assets/obseg/prop/Poil_drum7Z.bin", 0x7F9A90, 768, 1 },
    { "assets/obseg/prop/PpadlockZ.bin", 0x7F9D90, 2640, 1 },
    { "assets/obseg/prop/PpalmZ.bin", 0x7FA7E0, 1104, 1 },
    { "assets/obseg/prop/PpalmtreeZ.bin", 0x7FAC30, 1232, 1 },
    { "assets/obseg/prop/Pphone1Z.bin", 0x7FB100, 320, 1 },
    { "assets/obseg/prop/PplaneZ.bin", 0x7FB240, 9696, 1 },
    { "assets/obseg/prop/Pplant11Z.bin", 0x7FD820, 960, 1 },
    { "assets/obseg/prop/Pplant1Z.bin", 0x7FDBE0, 912, 1 },
    { "assets/obseg/prop/Pplant2Z.bin", 0x7FDF70, 864, 1 },
    { "assets/obseg/prop/Pplant2bZ.bin", 0x7FE2D0, 1040, 1 },
    { "assets/obseg/prop/Pplant3Z.bin", 0x7FE6E0, 1104, 1 },
    { "assets/obseg/prop/Pradio_unit1Z.bin", 0x7FEB30, 432, 1 },
    { "assets/obseg/prop/Pradio_unit2Z.bin", 0x7FECE0, 448, 1 },
    { "assets/obseg/prop/Pradio_unit3Z.bin", 0x7FEEA0, 448, 1 },
    { "assets/obseg/prop/Pradio_unit4Z.bin", 0x7FF060, 448, 1 },
    { "assets/obseg/prop/ProofgunZ.bin", 0x7FF220, 1632, 1 },
    { "assets/obseg/prop/PsafeZ.bin", 0x7FF880, 848, 1 },
    { "assets/obseg/prop/PsafedoorZ.bin", 0x7FFBD0, 1264, 1 },
    { "assets/obseg/prop/Psat1_reflectZ.bin", 0x8000C0, 5488, 1 },
    { "assets/obseg/prop/PsatboxZ.bin", 0x801630, 288, 1 },
    { "assets/obseg/prop/PsatdishZ.bin", 0x801750, 1120, 1 },
    { "assets/obseg/prop/Psec_panelZ.bin", 0x801BB0, 416, 1 },
    { "assets/obseg/prop/Psev_door3Z.bin", 0x801D50, 656, 1 },
    { "assets/obseg/prop/Psev_door3_windZ.bin", 0x801FE0, 912, 1 },
    { "assets/obseg/prop/Psev_door4_windZ.bin", 0x802370, 992, 1 },
    { "assets/obseg/prop/Psev_doorZ.bin", 0x802750, 848, 1 },
    { "assets/obseg/prop/Psev_door_v1Z.bin", 0x802AA0, 816, 1 },
    { "assets/obseg/prop/Psev_trislideZ.bin", 0x802DD0, 944, 1 },
    { "assets/obseg/prop/PsevdishZ.bin", 0x803180, 3872, 1 },
    { "assets/obseg/prop/PsevdoormetslideZ.bin", 0x8040A0, 736, 1 },
    { "assets/obseg/prop/PsevdoornowindZ.bin", 0x804380, 368, 1 },
    { "assets/obseg/prop/PsevdoorwindZ.bin", 0x8044F0, 1072, 1 },
    { "assets/obseg/prop/PsevdoorwoodZ.bin", 0x804920, 944, 1 },
    { "assets/obseg/prop/PshuttleZ.bin", 0x804CD0, 10752, 1 },
    { "assets/obseg/prop/Pshuttle_door_lZ.bin", 0x8076D0, 3120, 1 },
    { "assets/obseg/prop/Pshuttle_door_rZ.bin", 0x808300, 3328, 1 },
    { "assets/obseg/prop/PsilencerZ.bin", 0x809000, 416, 1 },
    { "assets/obseg/prop/Psilo_lift_doorZ.bin", 0x8091A0, 576, 1 },
    { "assets/obseg/prop/PsilotopdoorZ.bin", 0x8093E0, 752, 1 },
    { "assets/obseg/prop/PskorpionmagZ.bin", 0x8096D0, 352, 1 },
    { "assets/obseg/prop/PspectremagZ.bin", 0x809830, 368, 1 },
    { "assets/obseg/prop/PspeedboatZ.bin", 0x8099A0, 3392, 1 },
    { "assets/obseg/prop/Pst_pete_room_1iZ.bin", 0x80A6E0, 12608, 1 },
    { "assets/obseg/prop/Pst_pete_room_2iZ.bin", 0x80D820, 12768, 1 },
    { "assets/obseg/prop/Pst_pete_room_3tZ.bin", 0x810A00, 12096, 1 },
    { "assets/obseg/prop/Pst_pete_room_5cZ.bin", 0x813940, 13712, 1 },
    { "assets/obseg/prop/Pst_pete_room_6cZ.bin", 0x816ED0, 13328, 1 },
    { "assets/obseg/prop/Psteel_door1Z.bin", 0x81A2E0, 624, 1 },
    { "assets/obseg/prop/Psteel_door2Z.bin", 0x81A550, 688, 1 },
    { "assets/obseg/prop/Psteel_door2bZ.bin", 0x81A800, 720, 1 },
    { "assets/obseg/prop/Psteel_door3Z.bin", 0x81AAD0, 720, 1 },
    { "assets/obseg/prop/Pstool1Z.bin", 0x81ADA0, 704, 1 },
    { "assets/obseg/prop/Pswipe_card2Z.bin", 0x81B060, 400, 1 },
    { "assets/obseg/prop/Pswivel_chair1Z.bin", 0x81B1F0, 656, 1 },
    { "assets/obseg/prop/PtankZ.bin", 0x81B480, 6816, 1 },
    { "assets/obseg/prop/PtigerZ.bin", 0x81CF20, 7824, 1 },
    { "assets/obseg/prop/Ptorpedo_rackZ.bin", 0x81EDB0, 2176, 1 },
    { "assets/obseg/prop/Ptrain_door2Z.bin", 0x81F630, 976, 1 },
    { "assets/obseg/prop/Ptrain_door3Z.bin", 0x81FA00, 1056, 1 },
    { "assets/obseg/prop/Ptrain_doorZ.bin", 0x81FE20, 624, 1 },
    { "assets/obseg/prop/PtrainextdoorZ.bin", 0x820090, 832, 1 },
    { "assets/obseg/prop/Ptt33magZ.bin", 0x8203D0, 320, 1 },
    { "assets/obseg/prop/Ptuning_console1Z.bin", 0x820510, 1312, 1 },
    { "assets/obseg/prop/Ptv1Z.bin", 0x820A30, 464, 1 },
    { "assets/obseg/prop/Ptv4screenZ.bin", 0x820C00, 416, 1 },
    { "assets/obseg/prop/Ptv_holderZ.bin", 0x820DA0, 1744, 1 },
    { "assets/obseg/prop/PtvscreenZ.bin", 0x821470, 208, 1 },
    { "assets/obseg/prop/PuzimagZ.bin", 0x821540, 320, 1 },
    { "assets/obseg/prop/PvertdoorZ.bin", 0x821680, 1552, 1 },
    { "assets/obseg/prop/PwalletbondZ.bin", 0x821C90, 5552, 1 },
    { "assets/obseg/prop/PwindowZ.bin", 0x823240, 240, 1 },
    { "assets/obseg/prop/Pwindow_cor11Z.bin", 0x823330, 224, 1 },
    { "assets/obseg/prop/Pwindow_lib_lg1Z.bin", 0x823410, 224, 1 },
    { "assets/obseg/prop/Pwindow_lib_sm1Z.bin", 0x8234F0, 240, 1 },
    { "assets/obseg/prop/Pwood_lg_crate1Z.bin", 0x8235E0, 640, 1 },
    { "assets/obseg/prop/Pwood_lg_crate2Z.bin", 0x823860, 544, 1 },
    { "assets/obseg/prop/Pwood_md_crate3Z.bin", 0x823A80, 544, 1 },
    { "assets/obseg/prop/Pwood_sm_crate4Z.bin", 0x823CA0, 608, 1 },
    { "assets/obseg/prop/Pwood_sm_crate5Z.bin", 0x823F00, 608, 1 },
    { "assets/obseg/prop/Pwood_sm_crate6Z.bin", 0x824160, 544, 1 },
    { "assets/obseg/prop/Pwooden_table1Z.bin", 0x824380, 880, 1 },
    { "assets/obseg/prop/PwppkmagZ.bin", 0x8246F0, 320, 1 },
    { "assets/obseg/stan/Tbg_ame_all_p_stanZ.bin", 0x824830, 6448, 1 },
    { "assets/obseg/stan/Tbg_arch_all_p_stanZ.bin", 0x826160, 23792, 1 },
    { "assets/obseg/stan/Tbg_arec_all_p_stanZ.bin", 0x82BE50, 33616, 1 },
    { "assets/obseg/stan/Tbg_ark_all_p_stanZ.bin", 0x8341A0, 36800, 1 },
    { "assets/obseg/stan/Tbg_ash_all_p_stanZ.bin", 0x83D160, 6448, 1 },
    { "assets/obseg/stan/Tbg_azt_all_p_stanZ.bin", 0x83EA90, 21888, 1 },
    { "assets/obseg/stan/Tbg_cat_all_p_stanZ.bin", 0x844010, 10032, 1 },
    { "assets/obseg/stan/Tbg_cave_all_p_stanZ.bin", 0x846740, 20208, 1 },
    { "assets/obseg/stan/Tbg_crad_all_p_stanZ.bin", 0x84B630, 10512, 1 },
    { "assets/obseg/stan/Tbg_cryp_all_p_stanZ.bin", 0x84DF40, 12400, 1 },
    { "assets/obseg/stan/Tbg_dam_all_p_stanZ.bin", 0x850FB0, 41952, 1 },
    { "assets/obseg/stan/Tbg_depo_all_p_stanZ.bin", 0x85B390, 28480, 1 },
    { "assets/obseg/stan/Tbg_dest_all_p_stanZ.bin", 0x8622D0, 26864, 1 },
    { "assets/obseg/stan/Tbg_dish_all_p_stanZ.bin", 0x868BC0, 2832, 1 },
    { "assets/obseg/stan/Tbg_imp_all_p_stanZ.bin", 0x8696D0, 6448, 1 },
    { "assets/obseg/stan/Tbg_jun_all_p_stanZ.bin", 0x86B000, 29008, 1 },
    { "assets/obseg/stan/Tbg_len_all_p_stanZ.bin", 0x872150, 2752, 1 },
    { "assets/obseg/stan/Tbg_oat_all_p_stanZ.bin", 0x872C10, 6400, 1 },
    { "assets/obseg/stan/Tbg_pete_all_p_stanZ.bin", 0x874510, 18064, 1 },
    { "assets/obseg/stan/Tbg_ref_all_p_stanZ.bin", 0x878BA0, 7632, 1 },
    { "assets/obseg/stan/Tbg_run_all_p_stanZ.bin", 0x87A970, 6784, 1 },
    { "assets/obseg/stan/Tbg_sev_all_p_stanZ.bin", 0x87C3F0, 15824, 1 },
    { "assets/obseg/stan/Tbg_sevb_all_p_stanZ.bin", 0x8801C0, 20288, 1 },
    { "assets/obseg/stan/Tbg_sevx_all_p_stanZ.bin", 0x885100, 37680, 1 },
    { "assets/obseg/stan/Tbg_silo_all_p_stanZ.bin", 0x88E430, 37024, 1 },
    { "assets/obseg/stan/Tbg_stat_all_p_stanZ.bin", 0x8974D0, 20160, 1 },
    { "assets/obseg/stan/Tbg_tra_all_p_stanZ.bin", 0x89C390, 9168, 1 },
    { "assets/obseg/brief/UbriefarchZ", 0x89E760, 32, 1 },
    { "assets/obseg/brief/UbriefarkZ", 0x89E780, 32, 1 },
    { "assets/obseg/brief/UbriefaztZ", 0x89E7A0, 32, 1 },
    { "assets/obseg/brief/UbriefcaveZ", 0x89E7C0, 32, 1 },
    { "assets/obseg/brief/UbriefcontrolZ", 0x89E7E0, 32, 1 },
    { "assets/obseg/brief/UbriefcradZ", 0x89E800, 32, 1 },
    { "assets/obseg/brief/UbriefcrypZ", 0x89E820, 32, 1 },
    { "assets/obseg/brief/UbriefdamZ", 0x89E840, 32, 1 },
    { "assets/obseg/brief/UbriefdepoZ", 0x89E860, 32, 1 },
    { "assets/obseg/brief/UbriefdestZ", 0x89E880, 32, 1 },
    { "assets/obseg/brief/UbriefjunZ", 0x89E8A0, 32, 1 },
    { "assets/obseg/brief/UbriefpeteZ", 0x89E8C0, 32, 1 },
    { "assets/obseg/brief/UbriefrunZ", 0x89E8E0, 32, 1 },
    { "assets/obseg/brief/UbriefsevbZ", 0x89E900, 32, 1 },
    { "assets/obseg/brief/UbriefsevbunkerZ", 0x89E920, 32, 1 },
    { "assets/obseg/brief/UbriefsevxZ", 0x89E940, 32, 1 },
    { "assets/obseg/brief/UbriefsevxbZ", 0x89E960, 32, 1 },
    { "assets/obseg/brief/UbriefsiloZ", 0x89E980, 32, 1 },
    { "assets/obseg/brief/UbriefstatueZ", 0x89E9A0, 32, 1 },
    { "assets/obseg/brief/UbrieftraZ", 0x89E9C0, 32, 1 },
    { "assets/obseg/setup/Ump_setupameZ.bin", 0x89E9E0, 1824, 1 },
    { "assets/obseg/setup/u/Ump_setuparchZ.bin", 0x89F100, 11680, 1 },
    { "assets/obseg/setup/Ump_setuparkZ.bin", 0x8A1EA0, 7488, 1 },
    { "assets/obseg/setup/Ump_setupashZ.bin", 0x8A3BE0, 1776, 1 },
    { "assets/obseg/setup/Ump_setupcaveZ.bin", 0x8A42D0, 9568, 1 },
    { "assets/obseg/setup/Ump_setupcradZ.bin", 0x8A6830, 2400, 1 },
    { "assets/obseg/setup/Ump_setupcrypZ.bin", 0x8A7190, 3424, 1 },
    { "assets/obseg/setup/Ump_setupdishZ.bin", 0x8A7EF0, 1008, 1 },
    { "assets/obseg/setup/Ump_setupimpZ.bin", 0x8A82E0, 1600, 1 },
    { "assets/obseg/setup/Ump_setupoatZ.bin", 0x8A8920, 848, 1 },
    { "assets/obseg/setup/Ump_setuprefZ.bin", 0x8A8C70, 1040, 1 },
    { "assets/obseg/setup/Ump_setupsevbZ.bin", 0x8A9080, 4880, 1 },
    { "assets/obseg/setup/Ump_setupstatueZ.bin", 0x8AA390, 3712, 1 },
    { "assets/obseg/setup/UsetuparchZ.bin", 0x8AB210, 17936, 1 },
    { "assets/obseg/setup/UsetuparkZ.bin", 0x8AF820, 15248, 1 },
    { "assets/obseg/setup/UsetupaztZ.bin", 0x8B33B0, 10496, 1 },
    { "assets/obseg/setup/UsetupcaveZ.bin", 0x8B5CB0, 15968, 1 },
    { "assets/obseg/setup/UsetupcontrolZ.bin", 0x8B9B10, 15104, 1 },
    { "assets/obseg/setup/u/UsetupcradZ.bin", 0x8BD610, 7216, 1 },
    { "assets/obseg/setup/UsetupcrypZ.bin", 0x8BF240, 7824, 1 },
    { "assets/obseg/setup/UsetupdamZ.bin", 0x8C10D0, 17104, 1 },
    { "assets/obseg/setup/UsetupdepoZ.bin", 0x8C53A0, 12176, 1 },
    { "assets/obseg/setup/u/UsetupdestZ.bin", 0x8C8330, 9040, 1 },
    { "assets/obseg/setup/u/UsetupjunZ.bin", 0x8CA680, 14080, 1 },
    { "assets/obseg/setup/u/UsetuplenZ.bin", 0x8CDD80, 1488, 1 },
    { "assets/obseg/setup/UsetuppeteZ.bin", 0x8CE350, 12160, 1 },
    { "assets/obseg/setup/UsetuprunZ.bin", 0x8D12D0, 6240, 1 },
    { "assets/obseg/setup/UsetupsevbZ.bin", 0x8D2B30, 9824, 1 },
    { "assets/obseg/setup/UsetupsevbunkerZ.bin", 0x8D5190, 6704, 1 },
    { "assets/obseg/setup/UsetupsevxZ.bin", 0x8D6BC0, 17168, 1 },
    { "assets/obseg/setup/UsetupsevxbZ.bin", 0x8DAED0, 16624, 1 },
    { "assets/obseg/setup/u/UsetupsiloZ.bin", 0x8DEFC0, 10832, 1 },
    { "assets/obseg/setup/u/UsetupstatueZ.bin", 0x8E1A10, 10192, 1 },
    { "assets/obseg/setup/u/UsetuptraZ.bin", 0x8E41E0, 12848, 1 },
    { "assets/obseg/text/LameE", 0x8E7410, 16, 1 },
    { "assets/obseg/text/LameJ", 0x8E7420, 16, 1 },
    { "assets/obseg/text/LarchE", 0x8E7430, 1584, 1 },
    { "assets/obseg/text/LarchJ.bin", 0x8E7A60, 1632, 1 },
    { "assets/obseg/text/LarecE", 0x8E80C0, 1488, 1 },
    { "assets/obseg/text/u/LarecJ.bin", 0x8E8690, 1424, 1 },
    { "assets/obseg/text/LarkE", 0x8E8C20, 1696, 1 },
    { "assets/obseg/text/u/LarkJ.bin", 0x8E92C0, 1712, 1 },
    { "assets/obseg/text/LashE", 0x8E9970, 16, 1 },
    { "assets/obseg/text/LashJ", 0x8E9980, 16, 1 },
    { "assets/obseg/text/LaztE", 0x8E9990, 1088, 1 },
    { "assets/obseg/text/u/LaztJ.bin", 0x8E9DD0, 1200, 1 },
    { "assets/obseg/text/LcatE", 0x8EA280, 16, 1 },
    { "assets/obseg/text/LcatJ", 0x8EA290, 16, 1 },
    { "assets/obseg/text/LcaveE", 0x8EA2A0, 1024, 1 },
    { "assets/obseg/text/LcaveJ.bin", 0x8EA6A0, 1120, 1 },
    { "assets/obseg/text/LcradE", 0x8EAB00, 1232, 1 },
    { "assets/obseg/text/u/LcradJ.bin", 0x8EAFD0, 1200, 1 },
    { "assets/obseg/text/LcrypE", 0x8EB480, 592, 1 },
    { "assets/obseg/text/u/LcrypJ.bin", 0x8EB6D0, 704, 1 },
    { "assets/obseg/text/LdamE", 0x8EB990, 1104, 1 },
    { "assets/obseg/text/u/LdamJ.bin", 0x8EBDE0, 1136, 1 },
    { "assets/obseg/text/LdepoE", 0x8EC250, 880, 1 },
    { "assets/obseg/text/u/LdepoJ.bin", 0x8EC5C0, 832, 1 },
    { "assets/obseg/text/LdestE", 0x8EC900, 1168, 1 },
    { "assets/obseg/text/LdestJ.bin", 0x8ECD90, 1120, 1 },
    { "assets/obseg/text/LdishE", 0x8ED1F0, 16, 1 },
    { "assets/obseg/text/LdishJ", 0x8ED200, 16, 1 },
    { "assets/obseg/text/LearE", 0x8ED210, 16, 1 },
    { "assets/obseg/text/LearJ", 0x8ED220, 16, 1 },
    { "assets/obseg/text/LeldE", 0x8ED230, 16, 1 },
    { "assets/obseg/text/LeldJ", 0x8ED240, 16, 1 },
    { "assets/obseg/text/LgunE", 0x8ED250, 1824, 1 },
    { "assets/obseg/text/u/LgunJ.bin", 0x8ED970, 1872, 1 },
    { "assets/obseg/text/LimpE", 0x8EE0C0, 16, 1 },
    { "assets/obseg/text/LimpJ", 0x8EE0D0, 16, 1 },
    { "assets/obseg/text/LjunE", 0x8EE0E0, 1312, 1 },
    { "assets/obseg/text/u/LjunJ.bin", 0x8EE600, 1344, 1 },
    { "assets/obseg/text/LleeE", 0x8EEB40, 16, 1 },
    { "assets/obseg/text/LleeJ", 0x8EEB50, 16, 1 },
    { "assets/obseg/text/LlenE", 0x8EEB60, 1600, 1 },
    { "assets/obseg/text/u/LlenJ.bin", 0x8EF1A0, 688, 1 },
    { "assets/obseg/text/LlipE", 0x8EF450, 16, 1 },
    { "assets/obseg/text/LlipJ", 0x8EF460, 16, 1 },
    { "assets/obseg/text/LlueE", 0x8EF470, 16, 1 },
    { "assets/obseg/text/LlueJ", 0x8EF480, 16, 1 },
    { "assets/obseg/text/LmiscE", 0x8EF490, 672, 1 },
    { "assets/obseg/text/u/LmiscJ.bin", 0x8EF730, 736, 1 },
    { "assets/obseg/text/LmpmenuE", 0x8EFA10, 416, 1 },
    { "assets/obseg/text/u/LmpmenuJ.bin", 0x8EFBB0, 400, 1 },
    { "assets/obseg/text/LmpweaponsE", 0x8EFD40, 192, 1 },
    { "assets/obseg/text/u/LmpweaponsJ.bin", 0x8EFE00, 224, 1 },
    { "assets/obseg/text/LoatE", 0x8EFEE0, 16, 1 },
    { "assets/obseg/text/LoatJ", 0x8EFEF0, 16, 1 },
    { "assets/obseg/text/LoptionsE", 0x8EFF00, 560, 1 },
    { "assets/obseg/text/u/LoptionsJ.bin", 0x8F0130, 592, 1 },
    { "assets/obseg/text/LpamE", 0x8F0380, 16, 1 },
    { "assets/obseg/text/LpamJ", 0x8F0390, 16, 1 },
    { "assets/obseg/text/LpeteE", 0x8F03A0, 1152, 1 },
    { "assets/obseg/text/u/LpeteJ.bin", 0x8F0820, 1136, 1 },
    { "assets/obseg/text/LpropobjE", 0x8F0C90, 672, 1 },
    { "assets/obseg/text/u/LpropobjJ.bin", 0x8F0F30, 704, 1 },
    { "assets/obseg/text/LrefE", 0x8F11F0, 16, 1 },
    { "assets/obseg/text/LrefJ", 0x8F1200, 16, 1 },
    { "assets/obseg/text/LritE", 0x8F1210, 16, 1 },
    { "assets/obseg/text/LritJ", 0x8F1220, 16, 1 },
    { "assets/obseg/text/LrunE", 0x8F1230, 624, 1 },
    { "assets/obseg/text/LrunJ.bin", 0x8F14A0, 656, 1 },
    { "assets/obseg/text/LsevE", 0x8F1730, 1376, 1 },
    { "assets/obseg/text/u/LsevJ.bin", 0x8F1C90, 1296, 1 },
    { "assets/obseg/text/LsevbE", 0x8F21A0, 1872, 1 },
    { "assets/obseg/text/u/LsevbJ.bin", 0x8F28F0, 2032, 1 },
    { "assets/obseg/text/LsevxE", 0x8F30E0, 1120, 1 },
    { "assets/obseg/text/u/LsevxJ.bin", 0x8F3540, 960, 1 },
    { "assets/obseg/text/LsevxbE", 0x8F3900, 1168, 1 },
    { "assets/obseg/text/LsevxbJ.bin", 0x8F3D90, 1104, 1 },
    { "assets/obseg/text/LshoE", 0x8F41E0, 16, 1 },
    { "assets/obseg/text/LshoJ", 0x8F41F0, 16, 1 },
    { "assets/obseg/text/LsiloE", 0x8F4200, 1456, 1 },
    { "assets/obseg/text/u/LsiloJ.bin", 0x8F47B0, 1504, 1 },
    { "assets/obseg/text/LstatE", 0x8F4D90, 2336, 1 },
    { "assets/obseg/text/u/LstatJ.bin", 0x8F56B0, 2160, 1 },
    { "assets/obseg/text/LtitleE", 0x8F5F20, 2752, 1 },
    { "assets/obseg/text/u/LtitleJ.bin", 0x8F69E0, 2960, 1 },
    { "assets/obseg/text/LtraE", 0x8F7570, 1072, 1 },
    { "assets/obseg/text/u/LtraJ.bin", 0x8F79A0, 1056, 1 },
    { "assets/obseg/text/LwaxE", 0x8F7DC0, 16, 1 },
    { "assets/obseg/text/LwaxJ", 0x8F7DD0, 16, 1 },
    { "assets/obseg/ob__ob_end.seg", 0x8F7DE0, 16, 0 },
};

static const size_t s_RomSegmentCount = sizeof(s_RomSegments) / sizeof(s_RomSegments[0]);

int rom_resolver_validate_layout(const u8 *romData, size_t romSize) {
    if (!romData || romSize < 12582912) return 0;
    
    /* Check that all title-path critical segments are within ROM range */
    if (ROM_OFFSET_GLOBALIMAGETABLE + 2760 > romSize) return 0;
    if (ROM_OFFSET_RAREWARELOGO + 26608 > romSize) return 0;
    if (ROM_OFFSET_FONTDL + 192 > romSize) return 0;
    if (ROM_OFFSET_FONTBANKGOTHIC + 676 > romSize) return 0;
    if (ROM_OFFSET_FONTZURICHBOLD + 676 > romSize) return 0;

    return 1;
}

int rom_resolver_get_offset_by_name(const char *name, u32 *outOffset, u32 *outSize) {
    if (!name) return 0;
    for (size_t i = 0; i < s_RomSegmentCount; i++) {
        if (strcmp(s_RomSegments[i].name, name) == 0) {
            if (outOffset) *outOffset = s_RomSegments[i].offset;
            if (outSize) *outSize = s_RomSegments[i].size;
            return 1;
        }
    }
    return 0;
}

const rom_segment_entry_t *rom_resolver_get_entry(size_t index) {
    if (index >= s_RomSegmentCount) return NULL;
    return &s_RomSegments[index];
}

size_t rom_resolver_get_entry_count(void) {
    return s_RomSegmentCount;
}

/* Linker symbols matching authentic ROM layout */
u8 _fontdlSegmentRomStart[1] = {0};
u8 _fontdlSegmentRomEnd[1] = {0};
u8 _jfontchardataSegmentRomStart[1] = {0};
u8 _efontchardataSegmentRomStart[1] = {0};
u8 _animation_entriesSegmentRomStart[1] = {0};
u8 _animation_dataSegmentRomStart[1] = {0};
u8 _imagesSegmentRomStart[1] = {0};
u8 _rarewarelogoSegmentRomStart[1] = {0};
u8 _rarewarelogoSegmentRomEnd[1] = {0};
u8 _rarewarelogoSegmentStart[1] = {0};
u8 _rarewarelogoSegmentEnd[1] = {0};
u8 _gameSegmentRomStart[1] = {0};
u8 _animation_dataSegmentStart[1] = {0};
u8 _animation_dataSegmentEnd[1] = {0};
u8 _GlobalimagetableSegmentRomStart[1] = {0};
u8 _GlobalimagetableSegmentStart[1] = {0};
u8 _GlobalimagetableSegmentEnd[1] = {0};
u8 _fontbankgothicSegmentRomStart[1] = {0};
u8 _fontbankgothicSegmentStart[1] = {0};
u8 _fontbankgothicSegmentEnd[1] = {0};
u8 _fontzurichboldSegmentRomStart[1] = {0};
u8 _fontzurichboldSegmentStart[1] = {0};
u8 _fontzurichboldSegmentEnd[1] = {0};
static const u32 s_ResourceSizes[] = {
    0, /* NULLFILE */
    69104, /* BG_SEV_ALL_P (bg/bg_sev_all_p.seg) */
    331584, /* BG_SILO_ALL_P (bg/bg_silo_all_p.seg) */
    139472, /* BG_STAT_ALL_P (bg/bg_stat_all_p.seg) */
    189312, /* BG_AREC_ALL_P (bg/bg_arec_all_p.seg) */
    154352, /* BG_ARCH_ALL_P (bg/bg_arch_all_p.seg) */
    132464, /* BG_TRA_ALL_P (bg/bg_tra_all_p.seg) */
    186816, /* BG_DEST_ALL_P (bg/bg_dest_all_p.seg) */
    109984, /* BG_SEVB_ALL_P (bg/bg_sevb_all_p.seg) */
    137808, /* BG_AZT_ALL_P (bg/bg_azt_all_p.seg) */
    105520, /* BG_PETE_ALL_P (bg/bg_pete_all_p.seg) */
    182640, /* BG_DEPO_ALL_P (bg/bg_depo_all_p.seg) */
    38416, /* BG_REF_ALL_P (bg/bg_ref_all_p.seg) */
    87728, /* BG_CRYP_ALL_P (bg/bg_cryp_all_p.seg) */
    197024, /* BG_DAM_ALL_P (bg/bg_dam_all_p.seg) */
    200576, /* BG_ARK_ALL_P (bg/bg_ark_all_p.seg) */
    41936, /* BG_RUN_ALL_P (bg/bg_run_all_p.seg) */
    116176, /* BG_SEVX_ALL_P (bg/bg_sevx_all_p.seg) */
    86352, /* BG_JUN_ALL_P (bg/bg_jun_all_p.seg) */
    18544, /* BG_DISH_ALL_P (bg/bg_dish_all_p.seg) */
    148720, /* BG_CAVE_ALL_P (bg/bg_cave_all_p.seg) */
    21808, /* BG_CAT_ALL_P (bg/bg_cat_all_p.seg) */
    66384, /* BG_CRAD_ALL_P (bg/bg_crad_all_p.seg) */
    0, /* BG_SHO_ALL_P (bg/bg_sho_all_p.seg) */
    0, /* BG_ELD_ALL_P (bg/bg_eld_all_p.seg) */
    0, /* BG_IMP_ALL_P (bg/bg_imp_all_p.seg) */
    0, /* BG_ASH_ALL_P (bg/bg_ash_all_p.seg) */
    0, /* BG_LUE_ALL_P (bg/bg_lue_all_p.seg) */
    40800, /* BG_AME_ALL_P (bg/bg_ame_all_p.seg) */
    0, /* BG_RIT_ALL_P (bg/bg_rit_all_p.seg) */
    28240, /* BG_OAT_ALL_P (bg/bg_oat_all_p.seg) */
    0, /* BG_EAR_ALL_P (bg/bg_ear_all_p.seg) */
    0, /* BG_LEE_ALL_P (bg/bg_lee_all_p.seg) */
    0, /* BG_LIP_ALL_P (bg/bg_lip_all_p.seg) */
    4000, /* BG_LEN_ALL_P (bg/bg_len_all_p.seg) */
    0, /* BG_WAX_ALL_P (bg/bg_wax_all_p.seg) */
    0, /* BG_PAM_ALL_P (bg/bg_pam_all_p.seg) */
    9344, /* ARMOURGUARD (CarmourguardZ) */
    14064, /* BARONSAMEDI (CbaronsamediZ) */
    9968, /* BLUECAMGUARD (CbluecamguardZ) */
    7696, /* BLUEMAN (CbluemanZ) */
    7872, /* BLUEWOMAN (CbluewomanZ) */
    11280, /* BOILERBOND (CboilerbondZ) */
    13952, /* BOILERTREV (CboilertrevZ) */
    12576, /* BORIS (CborisZ) */
    9952, /* CAMGUARD (CcamguardZ) */
    7680, /* CARDIMAN (CcardimanZ) */
    7808, /* CHECKMAN (CcheckmanZ) */
    9728, /* COMMGUARD (CcommguardZ) */
    11920, /* DJBOND (CdjbondZ) */
    7968, /* FATTECHWOMAN (CfattechwomanZ) */
    10016, /* GREATGUARD2 (Cgreatguard2Z) */
    9856, /* GREATGUARD (CgreatguardZ) */
    9936, /* GREYGUARD (CgreyguardZ) */
    7616, /* GREYMAN (CgreymanZ) */
    1488, /* HEADALAN (CheadalanZ) */
    1392, /* HEADB (CheadbZ) */
    976, /* HEADBALACLAVA (CheadbalaclavaZ) */
    1696, /* HEADBIKE (CheadbikeZ) */
    3408, /* HEADBROSNAN (CheadbrosnanZ) */
    2976, /* HEADBROSNANBOILER (CheadbrosnanboilerZ) */
    3712, /* HEADBROSNANSNOW (CheadbrosnansnowZ) */
    3456, /* HEADBROSNANSUIT (CheadbrosnansuitZ) */
    3008, /* HEADBROSNANTIMBER (CheadbrosnantimberZ) */
    1344, /* HEADCHRIS (CheadchrisZ) */
    1408, /* HEADDAVE (CheaddaveZ) */
    1328, /* HEADDES (CheaddesZ) */
    1312, /* HEADDUNCAN (CheadduncanZ) */
    1408, /* HEADDWAYNE (CheaddwayneZ) */
    1392, /* HEADGRAHAM (CheadgrahamZ) */
    1328, /* HEADGRANT (CheadgrantZ) */
    1328, /* HEADJIM (CheadjimZ) */
    1056, /* HEADJOE2 (Cheadjoe2Z) */
    1392, /* HEADJOE (CheadjoeZ) */
    1296, /* HEADJOEL (CheadjoelZ) */
    1376, /* HEADKARL (CheadkarlZ) */
    1360, /* HEADKEN (CheadkenZ) */
    1408, /* HEADLEE (CheadleeZ) */
    1008, /* HEADMANDY (CheadmandyZ) */
    1040, /* HEADMARION (CheadmarionZ) */
    1328, /* HEADMARK (CheadmarkZ) */
    1376, /* HEADMARTIN (CheadmartinZ) */
    1376, /* HEADMISHKIN (CheadmishkinZ) */
    1312, /* HEADNEIL (CheadneilZ) */
    1424, /* HEADPETE (CheadpeteZ) */
    1296, /* HEADROBIN (CheadrobinZ) */
    1024, /* HEADSALLY (CheadsallyZ) */
    1408, /* HEADSCOTT (CheadscottZ) */
    1504, /* HEADSHAUN (CheadshaunZ) */
    1360, /* HEADSTEVEE (CheadsteveeZ) */
    1360, /* HEADSTEVEH (CheadstevehZ) */
    1168, /* HEADVIVIEN (CheadvivienZ) */
    11328, /* JAWS (CjawsZ) */
    8208, /* JEANWOMAN (CjeanwomanZ) */
    11168, /* MAYDAY (CmaydayZ) */
    8528, /* MOONFEMALE (CmoonfemaleZ) */
    9712, /* MOONGUARD (CmoonguardZ) */
    14528, /* NATALYA (CnatalyaZ) */
    9952, /* NAVYGUARD (CnavyguardZ) */
    12592, /* ODDJOB (CoddjobZ) */
    9808, /* OLIVEGUARD (ColiveguardZ) */
    13360, /* ORUMOV (CorumovZ) */
    12416, /* PILOT (CpilotZ) */
    10032, /* REDMAN (CredmanZ) */
    7216, /* RUSGUARD (CrusguardZ) */
    13104, /* SNOWBOND (CsnowbondZ) */
    11392, /* SNOWGUARD (CsnowguardZ) */
    14624, /* SPICEBOND (CspicebondZ) */
    12832, /* SUIT_LF_HAND (Csuit_lf_handZ) */
    11664, /* SUITBOND (CsuitbondZ) */
    9904, /* TECHMAN (CtechmanZ) */
    8176, /* TECHWOMAN (CtechwomanZ) */
    11568, /* TIMBERBOND (CtimberbondZ) */
    14288, /* TREVELYAN (CtrevelyanZ) */
    9744, /* TREVGUARD (CtrevguardZ) */
    12144, /* VALENTIN (CvalentinZ) */
    14832, /* XENIA (CxeniaZ) */
    2576, /* AK47 (Gak47Z) */
    912, /* AUDIOTAPE (GaudiotapeZ) */
    6160, /* AUTOSHOT (GautoshotZ) */
    1536, /* BLACKBOX (GblackboxZ) */
    256, /* BLUEPRINTS (GblueprintsZ) */
    1936, /* BOMBCASE (GbombcaseZ) */
    1520, /* BOMBDEFUSER (GbombdefuserZ) */
    1936, /* BRIEFCASE (GbriefcaseZ) */
    2416, /* BUG (GbugZ) */
    848, /* BUGDETECTOR (GbugdetectorZ) */
    848, /* BUNGEE (GbungeeZ) */
    1152, /* CAMERA (GcameraZ) */
    608, /* CARTBLUE (GcartblueZ) */
    304, /* CARTRIDGE (GcartridgeZ) */
    528, /* CARTRIFLE (GcartrifleZ) */
    512, /* CARTSHELL (GcartshellZ) */
    320, /* CIRCUITBOARD (GcircuitboardZ) */
    496, /* CLIPBOARD (GclipboardZ) */
    848, /* CREDITCARD (GcreditcardZ) */
    848, /* DARKGLASSES (GdarkglassesZ) */
    320, /* DATATHIEF (GdatathiefZ) */
    368, /* DATTAPE (GdattapeZ) */
    1408, /* DOORDECODER (GdoordecoderZ) */
    848, /* DOOREXPLODER (GdoorexploderZ) */
    864, /* DOSSIERRED (GdossierredZ) */
    848, /* DYNAMITE (GdynamiteZ) */
    592, /* EXPLOSIVEFLOPPY (GexplosivefloppyZ) */
    848, /* EXPLOSIVEPEN (GexplosivepenZ) */
    2032, /* EXTINGUISHER (GextinguisherZ) */
    848, /* FINGERGUN (GfingergunZ) */
    5888, /* FIST (GfistZ) */
    848, /* FLAREPISTOL (GflarepistolZ) */
    3232, /* FNP90 (Gfnp90Z) */
    2608, /* GASKEYRING (GgaskeyringZ) */
    848, /* GOLDBAR (GgoldbarZ) */
    2480, /* GOLDENEYEKEY (GgoldeneyekeyZ) */
    6112, /* GOLDENGUN (GgoldengunZ) */
    6496, /* GOLDWPPK (GgoldwppkZ) */
    2608, /* GRENADE (GgrenadeZ) */
    4224, /* GRENADELAUNCH (GgrenadelaunchZ) */
    848, /* HEROIN (GheroinZ) */
    7856, /* JOYPAD (GjoypadZ) */
    1936, /* KEYANALYSERCASE (GkeyanalysercaseZ) */
    2544, /* KEYBOLT (GkeyboltZ) */
    304, /* KEYCARD (GkeycardZ) */
    3408, /* KEYYALE (GkeyyaleZ) */
    6864, /* KNIFE (GknifeZ) */
    3568, /* LASER (GlaserZ) */
    848, /* LECTRE (GlectreZ) */
    848, /* LOCKEXPLODER (GlockexploderZ) */
    2592, /* M16 (Gm16Z) */
    240, /* MAP (GmapZ) */
    1600, /* MICROCAMERA (GmicrocameraZ) */
    848, /* MICROCODE (GmicrocodeZ) */
    848, /* MICROFILM (GmicrofilmZ) */
    848, /* MONEY (GmoneyZ) */
    3040, /* MP5K (Gmp5kZ) */
    3328, /* MP5KSIL (Gmp5ksilZ) */
    848, /* PITONGUN (GpitongunZ) */
    464, /* PLANS (GplansZ) */
    848, /* PLASTIQUE (GplastiqueZ) */
    1376, /* POLARIZEDGLASSES (GpolarizedglassesZ) */
    2032, /* PROXIMITYMINE (GproximitymineZ) */
    2496, /* REMOTEMINE (GremotemineZ) */
    4640, /* ROCKETLAUNCH (GrocketlaunchZ) */
    7568, /* RUGER (GrugerZ) */
    1936, /* SAFECRACKERCASE (GsafecrackercaseZ) */
    3808, /* SHOTGUN (GshotgunZ) */
    6496, /* SILVERWPPK (GsilverwppkZ) */
    4608, /* SKORPION (GskorpionZ) */
    4208, /* SNIPERRIFLE (GsniperrifleZ) */
    3200, /* SPECTRE (GspectreZ) */
    848, /* SPOOLTAPE (GspooltapeZ) */
    848, /* SPYFILE (GspyfileZ) */
    416, /* STAFFLIST (GstafflistZ) */
    7776, /* TASER (GtaserZ) */
    6896, /* THROWKNIFE (GthrowknifeZ) */
    2752, /* TIMEDMINE (GtimedmineZ) */
    13312, /* TRIGGER (GtriggerZ) */
    6944, /* TT33 (Gtt33Z) */
    2320, /* UZI (GuziZ) */
    528, /* VIDEOTAPE (GvideotapeZ) */
    5216, /* WATCHCOMMUNICATOR (GwatchcommunicatorZ) */
    5216, /* WATCHGEIGERCOUNTER (GwatchgeigercounterZ) */
    5216, /* WATCHIDENTIFIER (GwatchidentifierZ) */
    13312, /* WATCHLASER (GwatchlaserZ) */
    5200, /* WATCHMAGNETATTRACT (GwatchmagnetattractZ) */
    5216, /* WATCHMAGNETREPEL (GwatchmagnetrepelZ) */
    1936, /* WEAPONCASE (GweaponcaseZ) */
    7312, /* WPPK (GwppkZ) */
    7488, /* WPPKSIL (GwppksilZ) */
    848, /* WRISTDART (GwristdartZ) */
    9600, /* ICBM (PICBMZ) */
    1968, /* ICBM_NOSE (PICBM_noseZ) */
    480, /* AK47MAG (Pak47magZ) */
    352, /* ALARM1 (Palarm1Z) */
    416, /* ALARM2 (Palarm2Z) */
    576, /* AMMO_CRATE1 (Pammo_crate1Z) */
    576, /* AMMO_CRATE2 (Pammo_crate2Z) */
    592, /* AMMO_CRATE3 (Pammo_crate3Z) */
    624, /* AMMO_CRATE4 (Pammo_crate4Z) */
    704, /* AMMO_CRATE5 (Pammo_crate5Z) */
    7264, /* APC (PapcZ) */
    480, /* ARCHSECDOOR1 (Parchsecdoor1Z) */
    464, /* ARCHSECDOOR2 (Parchsecdoor2Z) */
    3584, /* ARTIC (ParticZ) */
    2320, /* ARTICTRAILER (PartictrailerZ) */
    576, /* BARRICADE (PbarricadeZ) */
    848, /* BIN1 (Pbin1Z) */
    224, /* BLOTTER1 (Pblotter1Z) */
    1184, /* BODYARMOUR (PbodyarmourZ) */
    1056, /* BODYARMOURVEST (PbodyarmourvestZ) */
    512, /* BOLLARD (PbollardZ) */
    368, /* BOMB (PbombZ) */
    400, /* BOOK1 (Pbook1Z) */
    1776, /* BOOKSHELF1 (Pbookshelf1Z) */
    368, /* BORG_CRATE (Pborg_crateZ) */
    512, /* BOXCARTRIDGES (PboxcartridgesZ) */
    1072, /* BOXES2X4 (Pboxes2x4Z) */
    1088, /* BOXES3X4 (Pboxes3x4Z) */
    1632, /* BOXES4X4 (Pboxes4x4Z) */
    880, /* BRAKEUNIT (PbrakeunitZ) */
    1408, /* BRIDGE_CONSOLE1A (Pbridge_console1aZ) */
    1376, /* BRIDGE_CONSOLE1B (Pbridge_console1bZ) */
    1408, /* BRIDGE_CONSOLE2A (Pbridge_console2aZ) */
    1264, /* BRIDGE_CONSOLE2B (Pbridge_console2bZ) */
    1360, /* BRIDGE_CONSOLE3A (Pbridge_console3aZ) */
    1424, /* BRIDGE_CONSOLE3B (Pbridge_console3bZ) */
    3296, /* CARBMW (PcarbmwZ) */
    512, /* CARD_BOX1 (Pcard_box1Z) */
    576, /* CARD_BOX2 (Pcard_box2Z) */
    496, /* CARD_BOX3 (Pcard_box3Z) */
    432, /* CARD_BOX4_LG (Pcard_box4_lgZ) */
    512, /* CARD_BOX5_LG (Pcard_box5_lgZ) */
    496, /* CARD_BOX6_LG (Pcard_box6_lgZ) */
    3072, /* CARESCORT (PcarescortZ) */
    3120, /* CARGOLF (PcargolfZ) */
    4416, /* CARWEIRD (PcarweirdZ) */
    5632, /* CARZIL (PcarzilZ) */
    896, /* CCTV (PcctvZ) */
    1376, /* CHRAUDIOTAPE (PchraudiotapeZ) */
    864, /* CHRAUTOSHOT (PchrautoshotZ) */
    2176, /* CHRBLACKBOX (PchrblackboxZ) */
    336, /* CHRBLUEPRINTS (PchrblueprintsZ) */
    496, /* CHRBOMBCASE (PchrbombcaseZ) */
    2272, /* CHRBOMBDEFUSER (PchrbombdefuserZ) */
    400, /* CHRBRIEFCASE (PchrbriefcaseZ) */
    3504, /* CHRBUG (PchrbugZ) */
    368, /* CHRBUGDETECTOR (PchrbugdetectorZ) */
    368, /* CHRBUNGEE (PchrbungeeZ) */
    1680, /* CHRCAMERA (PchrcameraZ) */
    416, /* CHRCIRCUITBOARD (PchrcircuitboardZ) */
    640, /* CHRCLIPBOARD (PchrclipboardZ) */
    368, /* CHRCREDITCARD (PchrcreditcardZ) */
    368, /* CHRDARKGLASSES (PchrdarkglassesZ) */
    416, /* CHRDATATHIEF (PchrdatathiefZ) */
    496, /* CHRDATTAPE (PchrdattapeZ) */
    2144, /* CHRDOORDECODER (PchrdoordecoderZ) */
    368, /* CHRDOOREXPLODER (PchrdoorexploderZ) */
    1232, /* CHRDOSSIERRED (PchrdossierredZ) */
    368, /* CHRDYNAMITE (PchrdynamiteZ) */
    368, /* CHREXPLOSIVEPEN (PchrexplosivepenZ) */
    1280, /* CHREXTINGUISHER (PchrextinguisherZ) */
    368, /* CHRFINGERGUN (PchrfingergunZ) */
    368, /* CHRFLAREPISTOL (PchrflarepistolZ) */
    1120, /* CHRFNP90 (Pchrfnp90Z) */
    3856, /* CHRGASKEYRING (PchrgaskeyringZ) */
    368, /* CHRGOLDBAR (PchrgoldbarZ) */
    624, /* CHRGOLDEN (PchrgoldenZ) */
    3744, /* CHRGOLDENEYEKEY (PchrgoldeneyekeyZ) */
    368, /* CHRGOLDWPPK (PchrgoldwppkZ) */
    880, /* CHRGRENADE (PchrgrenadeZ) */
    912, /* CHRGRENADELAUNCH (PchrgrenadelaunchZ) */
    624, /* CHRGRENADEROUND (PchrgrenaderoundZ) */
    368, /* CHRHEROIN (PchrheroinZ) */
    1008, /* CHRKALASH (PchrkalashZ) */
    496, /* CHRKEYANALYSERCASE (PchrkeyanalysercaseZ) */
    3744, /* CHRKEYBOLT (PchrkeyboltZ) */
    5216, /* CHRKEYYALE (PchrkeyyaleZ) */
    512, /* CHRKNIFE (PchrknifeZ) */
    960, /* CHRLASER (PchrlaserZ) */
    368, /* CHRLECTRE (PchrlectreZ) */
    368, /* CHRLOCKEXPLODER (PchrlockexploderZ) */
    976, /* CHRM16 (Pchrm16Z) */
    336, /* CHRMAP (PchrmapZ) */
    2288, /* CHRMICROCAMERA (PchrmicrocameraZ) */
    368, /* CHRMICROCODE (PchrmicrocodeZ) */
    368, /* CHRMICROFILM (PchrmicrofilmZ) */
    368, /* CHRMONEY (PchrmoneyZ) */
    896, /* CHRMP5K (Pchrmp5kZ) */
    1040, /* CHRMP5KSIL (Pchrmp5ksilZ) */
    368, /* CHRPITONGUN (PchrpitongunZ) */
    656, /* CHRPLANS (PchrplansZ) */
    1120, /* CHRPLASTIQUE (PchrplastiqueZ) */
    2240, /* CHRPOLARIZEDGLASSES (PchrpolarizedglassesZ) */
    1120, /* CHRPROXIMITYMINE (PchrproximitymineZ) */
    1120, /* CHRREMOTEMINE (PchrremotemineZ) */
    1456, /* CHRROCKET (PchrrocketZ) */
    992, /* CHRROCKETLAUNCH (PchrrocketlaunchZ) */
    992, /* CHRRUGER (PchrrugerZ) */
    496, /* CHRSAFECRACKERCASE (PchrsafecrackercaseZ) */
    848, /* CHRSHOTGUN (PchrshotgunZ) */
    368, /* CHRSILVERWPPK (PchrsilverwppkZ) */
    896, /* CHRSKORPION (PchrskorpionZ) */
    912, /* CHRSNIPERRIFLE (PchrsniperrifleZ) */
    880, /* CHRSPECTRE (PchrspectreZ) */
    368, /* CHRSPOOLTAPE (PchrspooltapeZ) */
    368, /* CHRSPYFILE (PchrspyfileZ) */
    544, /* CHRSTAFFLIST (PchrstafflistZ) */
    448, /* CHRTESTTUBE (PchrtesttubeZ) */
    544, /* CHRTHROWKNIFE (PchrthrowknifeZ) */
    1328, /* CHRTIMEDMINE (PchrtimedmineZ) */
    656, /* CHRTT33 (Pchrtt33Z) */
    720, /* CHRUZI (PchruziZ) */
    720, /* CHRVIDEOTAPE (PchrvideotapeZ) */
    512, /* CHRWEAPONCASE (PchrweaponcaseZ) */
    576, /* CHRWPPK (PchrwppkZ) */
    736, /* CHRWPPKSIL (PchrwppksilZ) */
    368, /* CHRWRISTDART (PchrwristdartZ) */
    1664, /* CONSOLE1 (Pconsole1Z) */
    1664, /* CONSOLE2 (Pconsole2Z) */
    1680, /* CONSOLE3 (Pconsole3Z) */
    1056, /* CONSOLE_SEV2A (Pconsole_sev2aZ) */
    1216, /* CONSOLE_SEV2B (Pconsole_sev2bZ) */
    1088, /* CONSOLE_SEV2C (Pconsole_sev2cZ) */
    1072, /* CONSOLE_SEV2D (Pconsole_sev2dZ) */
    1072, /* CONSOLE_SEV_GEA (Pconsole_sev_GEaZ) */
    1072, /* CONSOLE_SEV_GEB (Pconsole_sev_GEbZ) */
    1152, /* CONSOLE_SEVA (Pconsole_sevaZ) */
    1136, /* CONSOLE_SEVB (Pconsole_sevbZ) */
    1072, /* CONSOLE_SEVC (Pconsole_sevcZ) */
    1072, /* CONSOLE_SEVD (Pconsole_sevdZ) */
    400, /* CRYPTDOOR1A (Pcryptdoor1aZ) */
    400, /* CRYPTDOOR1B (Pcryptdoor1bZ) */
    400, /* CRYPTDOOR2A (Pcryptdoor2aZ) */
    400, /* CRYPTDOOR2B (Pcryptdoor2bZ) */
    624, /* CRYPTDOOR3 (Pcryptdoor3Z) */
    384, /* CRYPTDOOR4 (Pcryptdoor4Z) */
    640, /* DAMCHAINDOOR (PdamchaindoorZ) */
    544, /* DAMGATEDOOR (PdamgatedoorZ) */
    880, /* DAMTUNDOOR (PdamtundoorZ) */
    416, /* DEPOT_DOOR_STEEL (Pdepot_door_steelZ) */
    576, /* DEPOT_GATE_ENTRY (Pdepot_gate_entryZ) */
    384, /* DESK1 (Pdesk1Z) */
    384, /* DESK2 (Pdesk2Z) */
    576, /* DESK_ARECIBO1 (Pdesk_arecibo1Z) */
    768, /* DESK_LAMP2 (Pdesk_lamp2Z) */
    6384, /* DEST_ENGINE (Pdest_engineZ) */
    1632, /* DEST_EXOCET (Pdest_exocetZ) */
    1648, /* DEST_GUN (Pdest_gunZ) */
    2208, /* DEST_HARPOON (Pdest_harpoonZ) */
    4016, /* DEST_SEAWOLF (Pdest_seawolfZ) */
    448, /* DISC_READER (Pdisc_readerZ) */
    400, /* DISK_DRIVE1 (Pdisk_drive1Z) */
    384, /* DOOR_AZT_CHAIR (Pdoor_azt_chairZ) */
    1088, /* DOOR_AZT_DESK (Pdoor_azt_deskZ) */
    912, /* DOOR_AZT_DESK_TOP (Pdoor_azt_desk_topZ) */
    560, /* DOOR_AZTEC (Pdoor_aztecZ) */
    768, /* DOOR_DEST1 (Pdoor_dest1Z) */
    960, /* DOOR_DEST2 (Pdoor_dest2Z) */
    1376, /* DOOR_EYELID (Pdoor_eyelidZ) */
    2640, /* DOOR_IRIS (Pdoor_irisZ) */
    752, /* DOOR_MF (Pdoor_mfZ) */
    880, /* DOOR_ROLLER1 (Pdoor_roller1Z) */
    576, /* DOOR_ROLLER2 (Pdoor_roller2Z) */
    576, /* DOOR_ROLLER3 (Pdoor_roller3Z) */
    608, /* DOOR_ROLLER4 (Pdoor_roller4Z) */
    304, /* DOOR_ROLLERTRAIN (Pdoor_rollertrainZ) */
    608, /* DOOR_ST_AREC1 (Pdoor_st_arec1Z) */
    736, /* DOOR_ST_AREC2 (Pdoor_st_arec2Z) */
    416, /* DOOR_WIN (Pdoor_winZ) */
    1136, /* DOORCONSOLE (PdoorconsoleZ) */
    880, /* DOORPANEL (PdoorpanelZ) */
    336, /* DOORPRISON1 (Pdoorprison1Z) */
    512, /* DOORSTATGATE (PdoorstatgateZ) */
    288, /* EXPLOSIONBIT (PexplosionbitZ) */
    384, /* FILING_CABINET1 (Pfiling_cabinet1Z) */
    304, /* FLAG (PflagZ) */
    800, /* FLOPPY (PfloppyZ) */
    416, /* FNP90MAG (Pfnp90magZ) */
    896, /* GAS_PLANT_MET1_DO1 (Pgas_plant_met1_do1Z) */
    480, /* GAS_PLANT_SW2_DO1 (Pgas_plant_sw2_do1Z) */
    512, /* GAS_PLANT_SW3_DO1 (Pgas_plant_sw3_do1Z) */
    352, /* GAS_PLANT_SW4_DO1 (Pgas_plant_sw4_do1Z) */
    656, /* GAS_PLANT_SW_DO1 (Pgas_plant_sw_do1Z) */
    528, /* GAS_PLANT_WC_CUB1 (Pgas_plant_wc_cub1Z) */
    528, /* GASBARREL (PgasbarrelZ) */
    1344, /* GASBARRELS (PgasbarrelsZ) */
    1376, /* GASPLANT_CLEAR_DOOR (Pgasplant_clear_doorZ) */
    1456, /* GASTANK (PgastankZ) */
    352, /* GLASSWARE1 (Pglassware1Z) */
    656, /* GLASSWARE2 (Pglassware2Z) */
    528, /* GLASSWARE3 (Pglassware3Z) */
    1408, /* GLASSWARE4 (Pglassware4Z) */
    3760, /* GOLDENEYELOGO (PgoldeneyelogoZ) */
    512, /* GOLDENSHELLS (PgoldenshellsZ) */
    2000, /* GROUNDGUN (PgroundgunZ) */
    1856, /* GUN_RUNWAY1 (Pgun_runway1Z) */
    672, /* HATBERET (PhatberetZ) */
    720, /* HATBERETBLUE (PhatberetblueZ) */
    736, /* HATBERETRED (PhatberetredZ) */
    208, /* HATCHBOLT (PhatchboltZ) */
    544, /* HATCHDOOR (PhatchdoorZ) */
    368, /* HATCHSEVX (PhatchsevxZ) */
    560, /* HATFURRY (PhatfurryZ) */
    544, /* HATFURRYBLACK (PhatfurryblackZ) */
    528, /* HATFURRYBROWN (PhatfurrybrownZ) */
    560, /* HATHELMET (PhathelmetZ) */
    560, /* HATHELMETGREY (PhathelmetgreyZ) */
    992, /* HATMOON (PhatmoonZ) */
    784, /* HATPEAKED (PhatpeakedZ) */
    592, /* HATTBIRD (PhattbirdZ) */
    624, /* HATTBIRDBROWN (PhattbirdbrownZ) */
    16928, /* HELICOPTER (PhelicopterZ) */
    6000, /* HIND (PhindZ) */
    4448, /* JEEP (PjeepZ) */
    608, /* JERRY_CAN1 (Pjerry_can1Z) */
    1920, /* JUNGLE3_TREE (Pjungle3_treeZ) */
    1328, /* JUNGLE5_TREE (Pjungle5_treeZ) */
    848, /* KEY_HOLDER (Pkey_holderZ) */
    368, /* KEYBOARD1 (Pkeyboard1Z) */
    672, /* KIT_UNITS1 (Pkit_units1Z) */
    976, /* LABBENCH (PlabbenchZ) */
    624, /* LANDMINE (PlandmineZ) */
    4032, /* LEGALPAGE (PlegalpageZ) */
    352, /* LETTER_TRAY1 (Pletter_tray1Z) */
    400, /* LOCKER3 (Plocker3Z) */
    400, /* LOCKER4 (Plocker4Z) */
    320, /* M16MAG (Pm16magZ) */
    512, /* MAGNUMSHELLS (PmagnumshellsZ) */
    768, /* MAINFRAME1 (Pmainframe1Z) */
    720, /* MAINFRAME2 (Pmainframe2Z) */
    832, /* METAL_CHAIR1 (Pmetal_chair1Z) */
    448, /* METAL_CRATE1 (Pmetal_crate1Z) */
    448, /* METAL_CRATE2 (Pmetal_crate2Z) */
    448, /* METAL_CRATE3 (Pmetal_crate3Z) */
    448, /* METAL_CRATE4 (Pmetal_crate4Z) */
    6368, /* MILCOPTER (PmilcopterZ) */
    8960, /* MILTRUCK (PmiltruckZ) */
    2576, /* MISSILE_RACK2 (Pmissile_rack2Z) */
    992, /* MISSILE_RACK (Pmissile_rackZ) */
    832, /* MODEMBOX (PmodemboxZ) */
    3776, /* MOTORBIKE (PmotorbikeZ) */
    336, /* MP5KMAG (Pmp5kmagZ) */
    10976, /* NINTENDOLOGO (PnintendologoZ) */
    624, /* OIL_DRUM1 (Poil_drum1Z) */
    752, /* OIL_DRUM2 (Poil_drum2Z) */
    752, /* OIL_DRUM3 (Poil_drum3Z) */
    752, /* OIL_DRUM5 (Poil_drum5Z) */
    784, /* OIL_DRUM6 (Poil_drum6Z) */
    768, /* OIL_DRUM7 (Poil_drum7Z) */
    2640, /* PADLOCK (PpadlockZ) */
    1104, /* PALM (PpalmZ) */
    1232, /* PALMTREE (PpalmtreeZ) */
    320, /* PHONE1 (Pphone1Z) */
    9696, /* PLANE (PplaneZ) */
    960, /* PLANT11 (Pplant11Z) */
    912, /* PLANT1 (Pplant1Z) */
    864, /* PLANT2 (Pplant2Z) */
    1040, /* PLANT2B (Pplant2bZ) */
    1104, /* PLANT3 (Pplant3Z) */
    432, /* RADIO_UNIT1 (Pradio_unit1Z) */
    448, /* RADIO_UNIT2 (Pradio_unit2Z) */
    448, /* RADIO_UNIT3 (Pradio_unit3Z) */
    448, /* RADIO_UNIT4 (Pradio_unit4Z) */
    1632, /* ROOFGUN (ProofgunZ) */
    848, /* SAFE (PsafeZ) */
    1264, /* SAFEDOOR (PsafedoorZ) */
    5488, /* SAT1_REFLECT (Psat1_reflectZ) */
    288, /* SATBOX (PsatboxZ) */
    1120, /* SATDISH (PsatdishZ) */
    416, /* SEC_PANEL (Psec_panelZ) */
    656, /* SEV_DOOR3 (Psev_door3Z) */
    912, /* SEV_DOOR3_WIND (Psev_door3_windZ) */
    992, /* SEV_DOOR4_WIND (Psev_door4_windZ) */
    848, /* SEV_DOOR (Psev_doorZ) */
    816, /* SEV_DOOR_V1 (Psev_door_v1Z) */
    944, /* SEV_TRISLIDE (Psev_trislideZ) */
    3872, /* SEVDISH (PsevdishZ) */
    736, /* SEVDOORMETSLIDE (PsevdoormetslideZ) */
    368, /* SEVDOORNOWIND (PsevdoornowindZ) */
    1072, /* SEVDOORWIND (PsevdoorwindZ) */
    944, /* SEVDOORWOOD (PsevdoorwoodZ) */
    10752, /* SHUTTLE (PshuttleZ) */
    3120, /* SHUTTLE_DOOR_L (Pshuttle_door_lZ) */
    3328, /* SHUTTLE_DOOR_R (Pshuttle_door_rZ) */
    416, /* SILENCER (PsilencerZ) */
    576, /* SILO_LIFT_DOOR (Psilo_lift_doorZ) */
    752, /* SILOTOPDOOR (PsilotopdoorZ) */
    352, /* SKORPIONMAG (PskorpionmagZ) */
    368, /* SPECTREMAG (PspectremagZ) */
    3392, /* SPEEDBOAT (PspeedboatZ) */
    12608, /* ST_PETE_ROOM_1I (Pst_pete_room_1iZ) */
    12768, /* ST_PETE_ROOM_2I (Pst_pete_room_2iZ) */
    12096, /* ST_PETE_ROOM_3T (Pst_pete_room_3tZ) */
    13712, /* ST_PETE_ROOM_5C (Pst_pete_room_5cZ) */
    13328, /* ST_PETE_ROOM_6C (Pst_pete_room_6cZ) */
    624, /* STEEL_DOOR1 (Psteel_door1Z) */
    688, /* STEEL_DOOR2 (Psteel_door2Z) */
    720, /* STEEL_DOOR2B (Psteel_door2bZ) */
    720, /* STEEL_DOOR3 (Psteel_door3Z) */
    704, /* STOOL1 (Pstool1Z) */
    400, /* SWIPE_CARD2 (Pswipe_card2Z) */
    656, /* SWIVEL_CHAIR1 (Pswivel_chair1Z) */
    6816, /* TANK (PtankZ) */
    7824, /* TIGER (PtigerZ) */
    2176, /* TORPEDO_RACK (Ptorpedo_rackZ) */
    976, /* TRAIN_DOOR2 (Ptrain_door2Z) */
    1056, /* TRAIN_DOOR3 (Ptrain_door3Z) */
    624, /* TRAIN_DOOR (Ptrain_doorZ) */
    832, /* TRAINEXTDOOR (PtrainextdoorZ) */
    320, /* TT33MAG (Ptt33magZ) */
    1312, /* TUNING_CONSOLE1 (Ptuning_console1Z) */
    464, /* TV1 (Ptv1Z) */
    416, /* TV4SCREEN (Ptv4screenZ) */
    1744, /* TV_HOLDER (Ptv_holderZ) */
    208, /* TVSCREEN (PtvscreenZ) */
    320, /* UZIMAG (PuzimagZ) */
    1552, /* VERTDOOR (PvertdoorZ) */
    5552, /* WALLETBOND (PwalletbondZ) */
    240, /* WINDOW (PwindowZ) */
    224, /* WINDOW_COR11 (Pwindow_cor11Z) */
    224, /* WINDOW_LIB_LG1 (Pwindow_lib_lg1Z) */
    240, /* WINDOW_LIB_SM1 (Pwindow_lib_sm1Z) */
    640, /* WOOD_LG_CRATE1 (Pwood_lg_crate1Z) */
    544, /* WOOD_LG_CRATE2 (Pwood_lg_crate2Z) */
    544, /* WOOD_MD_CRATE3 (Pwood_md_crate3Z) */
    608, /* WOOD_SM_CRATE4 (Pwood_sm_crate4Z) */
    608, /* WOOD_SM_CRATE5 (Pwood_sm_crate5Z) */
    544, /* WOOD_SM_CRATE6 (Pwood_sm_crate6Z) */
    880, /* WOODEN_TABLE1 (Pwooden_table1Z) */
    320, /* WPPKMAG (PwppkmagZ) */
    6448, /* BG_AME_ALL_P_STAN (Tbg_ame_all_p_stanZ) */
    23792, /* BG_ARCH_ALL_P_STAN (Tbg_arch_all_p_stanZ) */
    33616, /* BG_AREC_ALL_P_STAN (Tbg_arec_all_p_stanZ) */
    36800, /* BG_ARK_ALL_P_STAN (Tbg_ark_all_p_stanZ) */
    6448, /* BG_ASH_ALL_P_STAN (Tbg_ash_all_p_stanZ) */
    21888, /* BG_AZT_ALL_P_STAN (Tbg_azt_all_p_stanZ) */
    10032, /* BG_CAT_ALL_P_STAN (Tbg_cat_all_p_stanZ) */
    20208, /* BG_CAVE_ALL_P_STAN (Tbg_cave_all_p_stanZ) */
    10512, /* BG_CRAD_ALL_P_STAN (Tbg_crad_all_p_stanZ) */
    12400, /* BG_CRYP_ALL_P_STAN (Tbg_cryp_all_p_stanZ) */
    41952, /* BG_DAM_ALL_P_STAN (Tbg_dam_all_p_stanZ) */
    28480, /* BG_DEPO_ALL_P_STAN (Tbg_depo_all_p_stanZ) */
    26864, /* BG_DEST_ALL_P_STAN (Tbg_dest_all_p_stanZ) */
    2832, /* BG_DISH_ALL_P_STAN (Tbg_dish_all_p_stanZ) */
    0, /* BG_EAR_ALL_P_STAN (Tbg_ear_all_p_stanZ) */
    0, /* BG_ELD_ALL_P_STAN (Tbg_eld_all_p_stanZ) */
    6448, /* BG_IMP_ALL_P_STAN (Tbg_imp_all_p_stanZ) */
    29008, /* BG_JUN_ALL_P_STAN (Tbg_jun_all_p_stanZ) */
    0, /* BG_LEE_ALL_P_STAN (Tbg_lee_all_p_stanZ) */
    2752, /* BG_LEN_ALL_P_STAN (Tbg_len_all_p_stanZ) */
    0, /* BG_LIP_ALL_P_STAN (Tbg_lip_all_p_stanZ) */
    0, /* BG_LUE_ALL_P_STAN (Tbg_lue_all_p_stanZ) */
    6400, /* BG_OAT_ALL_P_STAN (Tbg_oat_all_p_stanZ) */
    0, /* BG_PAM_ALL_P_STAN (Tbg_pam_all_p_stanZ) */
    18064, /* BG_PETE_ALL_P_STAN (Tbg_pete_all_p_stanZ) */
    7632, /* BG_REF_ALL_P_STAN (Tbg_ref_all_p_stanZ) */
    0, /* BG_RIT_ALL_P_STAN (Tbg_rit_all_p_stanZ) */
    6784, /* BG_RUN_ALL_P_STAN (Tbg_run_all_p_stanZ) */
    15824, /* BG_SEV_ALL_P_STAN (Tbg_sev_all_p_stanZ) */
    20288, /* BG_SEVB_ALL_P_STAN (Tbg_sevb_all_p_stanZ) */
    37680, /* BG_SEVX_ALL_P_STAN (Tbg_sevx_all_p_stanZ) */
    37024, /* BG_SILO_ALL_P_STAN (Tbg_silo_all_p_stanZ) */
    20160, /* BG_STAT_ALL_P_STAN (Tbg_stat_all_p_stanZ) */
    9168, /* BG_TRA_ALL_P_STAN (Tbg_tra_all_p_stanZ) */
    0, /* BG_WAX_ALL_P_STAN (Tbg_wax_all_p_stanZ) */
    32, /* BRIEFARCH (UbriefarchZ) */
    32, /* BRIEFARK (UbriefarkZ) */
    32, /* BRIEFAZT (UbriefaztZ) */
    32, /* BRIEFCAVE (UbriefcaveZ) */
    32, /* BRIEFCONTROL (UbriefcontrolZ) */
    32, /* BRIEFCRAD (UbriefcradZ) */
    32, /* BRIEFCRYP (UbriefcrypZ) */
    32, /* BRIEFDAM (UbriefdamZ) */
    32, /* BRIEFDEPO (UbriefdepoZ) */
    32, /* BRIEFDEST (UbriefdestZ) */
    32, /* BRIEFJUN (UbriefjunZ) */
    32, /* BRIEFPETE (UbriefpeteZ) */
    32, /* BRIEFRUN (UbriefrunZ) */
    32, /* BRIEFSEVB (UbriefsevbZ) */
    32, /* BRIEFSEVBUNKER (UbriefsevbunkerZ) */
    32, /* BRIEFSEVX (UbriefsevxZ) */
    32, /* BRIEFSEVXB (UbriefsevxbZ) */
    32, /* BRIEFSILO (UbriefsiloZ) */
    32, /* BRIEFSTATUE (UbriefstatueZ) */
    32, /* BRIEFTRA (UbrieftraZ) */
    1824, /* MP_SETUPAME (Ump_setupameZ) */
    11680, /* MP_SETUPARCH (Ump_setuparchZ) */
    7488, /* MP_SETUPARK (Ump_setuparkZ) */
    1776, /* MP_SETUPASH (Ump_setupashZ) */
    9568, /* MP_SETUPCAVE (Ump_setupcaveZ) */
    2400, /* MP_SETUPCRAD (Ump_setupcradZ) */
    3424, /* MP_SETUPCRYP (Ump_setupcrypZ) */
    1008, /* MP_SETUPDISH (Ump_setupdishZ) */
    1600, /* MP_SETUPIMP (Ump_setupimpZ) */
    848, /* MP_SETUPOAT (Ump_setupoatZ) */
    1040, /* MP_SETUPREF (Ump_setuprefZ) */
    4880, /* MP_SETUPSEVB (Ump_setupsevbZ) */
    3712, /* MP_SETUPSTATUE (Ump_setupstatueZ) */
    17936, /* SETUPARCH (UsetuparchZ) */
    15248, /* SETUPARK (UsetuparkZ) */
    10496, /* SETUPAZT (UsetupaztZ) */
    15968, /* SETUPCAVE (UsetupcaveZ) */
    15104, /* SETUPCONTROL (UsetupcontrolZ) */
    7216, /* SETUPCRAD (UsetupcradZ) */
    7824, /* SETUPCRYP (UsetupcrypZ) */
    17104, /* SETUPDAM (UsetupdamZ) */
    12176, /* SETUPDEPO (UsetupdepoZ) */
    9040, /* SETUPDEST (UsetupdestZ) */
    14080, /* SETUPJUN (UsetupjunZ) */
    1488, /* SETUPLEN (UsetuplenZ) */
    12160, /* SETUPPETE (UsetuppeteZ) */
    6240, /* SETUPRUN (UsetuprunZ) */
    9824, /* SETUPSEVB (UsetupsevbZ) */
    6704, /* SETUPSEVBUNKER (UsetupsevbunkerZ) */
    17168, /* SETUPSEVX (UsetupsevxZ) */
    16624, /* SETUPSEVXB (UsetupsevxbZ) */
    10832, /* SETUPSILO (UsetupsiloZ) */
    10192, /* SETUPSTATUE (UsetupstatueZ) */
    12848, /* SETUPTRA (UsetuptraZ) */
    16, /* AMEE (LameE) */
    16, /* AMEJ (LameJ) */
    16, /* AMEP (LameP) */
    1584, /* ARCHE (LarchE) */
    1632, /* ARCHJ (LarchJ) */
    1584, /* ARCHP (LarchP) */
    1488, /* ARECE (LarecE) */
    1424, /* ARECJ (LarecJ) */
    1488, /* ARECP (LarecP) */
    1696, /* ARKE (LarkE) */
    1712, /* ARKJ (LarkJ) */
    1696, /* ARKP (LarkP) */
    16, /* ASHE (LashE) */
    16, /* ASHJ (LashJ) */
    16, /* ASHP (LashP) */
    1088, /* AZTE (LaztE) */
    1200, /* AZTJ (LaztJ) */
    1088, /* AZTP (LaztP) */
    16, /* CATE (LcatE) */
    16, /* CATJ (LcatJ) */
    16, /* CATP (LcatP) */
    1024, /* CAVEE (LcaveE) */
    1120, /* CAVEJ (LcaveJ) */
    1024, /* CAVEP (LcaveP) */
    1232, /* CRADE (LcradE) */
    1200, /* CRADJ (LcradJ) */
    1232, /* CRADP (LcradP) */
    592, /* CRYPE (LcrypE) */
    704, /* CRYPJ (LcrypJ) */
    592, /* CRYPP (LcrypP) */
    1104, /* DAME (LdamE) */
    1136, /* DAMJ (LdamJ) */
    1104, /* DAMP (LdamP) */
    880, /* DEPOE (LdepoE) */
    832, /* DEPOJ (LdepoJ) */
    880, /* DEPOP (LdepoP) */
    1168, /* DESTE (LdestE) */
    1120, /* DESTJ (LdestJ) */
    1168, /* DESTP (LdestP) */
    16, /* DISHE (LdishE) */
    16, /* DISHJ (LdishJ) */
    16, /* DISHP (LdishP) */
    16, /* EARE (LearE) */
    16, /* EARJ (LearJ) */
    16, /* EARP (LearP) */
    16, /* ELDE (LeldE) */
    16, /* ELDJ (LeldJ) */
    16, /* ELDP (LeldP) */
    1824, /* GUNE (LgunE) */
    1872, /* GUNJ (LgunJ) */
    1824, /* GUNP (LgunP) */
    16, /* IMPE (LimpE) */
    16, /* IMPJ (LimpJ) */
    16, /* IMPP (LimpP) */
    1312, /* JUNE (LjunE) */
    1344, /* JUNJ (LjunJ) */
    1312, /* JUNP (LjunP) */
    16, /* LEEE (LleeE) */
    16, /* LEEJ (LleeJ) */
    16, /* LEEP (LleeP) */
    1600, /* LENE (LlenE) */
    688, /* LENJ (LlenJ) */
    1600, /* LENP (LlenP) */
    16, /* LIPE (LlipE) */
    16, /* LIPJ (LlipJ) */
    16, /* LIPP (LlipP) */
    16, /* LUEE (LlueE) */
    16, /* LUEJ (LlueJ) */
    16, /* LUEP (LlueP) */
    672, /* MISCE (LmiscE) */
    736, /* MISCJ (LmiscJ) */
    672, /* MISCP (LmiscP) */
    416, /* MPMENUE (LmpmenuE) */
    400, /* MPMENUJ (LmpmenuJ) */
    416, /* MPMENUP (LmpmenuP) */
    192, /* MPWEAPONSE (LmpweaponsE) */
    224, /* MPWEAPONSJ (LmpweaponsJ) */
    192, /* MPWEAPONSP (LmpweaponsP) */
    16, /* OATE (LoatE) */
    16, /* OATJ (LoatJ) */
    16, /* OATP (LoatP) */
    560, /* OPTIONSE (LoptionsE) */
    592, /* OPTIONSJ (LoptionsJ) */
    560, /* OPTIONSP (LoptionsP) */
    16, /* PAME (LpamE) */
    16, /* PAMJ (LpamJ) */
    16, /* PAMP (LpamP) */
    1152, /* PETEE (LpeteE) */
    1136, /* PETEJ (LpeteJ) */
    1152, /* PETEP (LpeteP) */
    672, /* PROPOBJE (LpropobjE) */
    704, /* PROPOBJJ (LpropobjJ) */
    672, /* PROPOBJP (LpropobjP) */
    16, /* REFE (LrefE) */
    16, /* REFJ (LrefJ) */
    16, /* REFP (LrefP) */
    16, /* RITE (LritE) */
    16, /* RITJ (LritJ) */
    16, /* RITP (LritP) */
    624, /* RUNE (LrunE) */
    656, /* RUNJ (LrunJ) */
    624, /* RUNP (LrunP) */
    1376, /* SEVE (LsevE) */
    1296, /* SEVJ (LsevJ) */
    1376, /* SEVP (LsevP) */
    1872, /* SEVBE (LsevbE) */
    2032, /* SEVBJ (LsevbJ) */
    1872, /* SEVBP (LsevbP) */
    1120, /* SEVXE (LsevxE) */
    960, /* SEVXJ (LsevxJ) */
    1120, /* SEVXP (LsevxP) */
    1168, /* SEVXBE (LsevxbE) */
    1104, /* SEVXBJ (LsevxbJ) */
    1168, /* SEVXBP (LsevxbP) */
    16, /* SHOE (LshoE) */
    16, /* SHOJ (LshoJ) */
    16, /* SHOP (LshoP) */
    1456, /* SILOE (LsiloE) */
    1504, /* SILOJ (LsiloJ) */
    1456, /* SILOP (LsiloP) */
    2336, /* STATE (LstatE) */
    2160, /* STATJ (LstatJ) */
    2336, /* STATP (LstatP) */
    2752, /* TITLEE (LtitleE) */
    2960, /* TITLEJ (LtitleJ) */
    2752, /* TITLEP (LtitleP) */
    1072, /* TRAE (LtraE) */
    1056, /* TRAJ (LtraJ) */
    1072, /* TRAP (LtraP) */
    16, /* WAXE (LwaxE) */
    16, /* WAXJ (LwaxJ) */
    16, /* WAXP (LwaxP) */
    0, /* OBENDSEG (ob/ob_end.seg) */
};

u32 rom_resolver_get_resource_size(s32 index) {
    if (index >= 0 && (size_t)index < (sizeof(s_ResourceSizes) / sizeof(s_ResourceSizes[0]))) {
        return s_ResourceSizes[index];
    }
    return 0;
}

u8 _sfxctlSegmentRomStart[1] = {0};
u8 _sfxtblSegmentRomStart[1] = {0};
u8 _instrumentsctlSegmentRomStart[1] = {0};
u8 _instrumentstblSegmentRomStart[1] = {0};
u8 _musicsampletblSegmentRomStart[1] = {0};
