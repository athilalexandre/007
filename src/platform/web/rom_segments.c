#include <ultra64.h>

/**
 * Authentic ROM segment anchor symbols corresponding to ge007.ld.
 * These symbols provide linker addresses for romCopy / osPiStartDma
 * to fetch genuine data from the user-provided ROM buffer at runtime.
 */
u8 _headerSegmentRomStart = 0;
u8 _bootSegmentRomStart = 0;
u8 _codeSegmentRomStart = 0;
u8 _cdataSegmentRomStart = 0;
u8 _inflateSegmentRomStart = 0;
u8 _gameSegmentRomStart = 0;
u8 _alt_startSegmentRomStart = 0;
u8 _alt_startSegmentStart = 0;
u8 _animation_dataSegmentRomStart = 0;
u32 _animation_entriesSegmentRomStart[1] = {0};
u8 _fontdlSegmentRomStart = 0;
u8 _fontdlSegmentRomEnd = 0;
u8 _efontchardataSegmentRomStart = 0;
u8 _jfontchardataSegmentRomStart = 0;
u8 _fontbankgothicSegmentRomStart = 0;
u8 _fontzurichboldSegmentRomStart = 0;
u8 _rarewarelogoSegmentRomStart = 0;
u8 _rarewarelogoSegmentStart = 0;
u8 _rarewarelogoSegmentEnd = 0;
u8 _musicfilesSegmentRomStart = 0;
u8 _obsegSegmentRomStart = 0;
u8 _imagesSegmentRomStart = 0;
u8 _GlobalimagetableSegmentRomStart = 0;
u32 _sfxtblSegmentRomStart = 0;
u32 _sfxctlSegmentRomStart = 0;
u32 _instrumentstblSegmentRomStart = 0;
u32 _instrumentsctlSegmentRomStart = 0;
u32 _musicsampletblSegmentRomStart = 0;
u8 _romfiles2SegmentRomStart = 0;
u8 _ramromfilesSegmentRomStart = 0;