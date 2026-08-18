/*
 * GoldenEye 007 (N64) Web ROM Resource Resolver
 * Authoritative numeric ROM segment offsets derived from clean N64 decompilation layout.
 */
#ifndef _ROM_RESOLVER_H_
#define _ROM_RESOLVER_H_

#include <ultra64.h>

typedef struct {
    const char *name;
    u32 offset;
    u32 size;
    u32 compressed;
} rom_segment_entry_t;

int rom_resolver_validate_layout(const u8 *romData, size_t romSize);
int rom_resolver_get_offset_by_name(const char *name, u32 *outOffset, u32 *outSize);
const rom_segment_entry_t *rom_resolver_get_entry(size_t index);
size_t rom_resolver_get_entry_count(void);

/* Direct ROM offsets for core engine segments (USA Retail) */
#define ROM_OFFSET_FONTDL              0x117880
#define ROM_OFFSET_JFONTCHARDATA       0x117940
#define ROM_OFFSET_EFONTCHARDATA       0x123040
#define ROM_OFFSET_ANIMATION_ENTRIES   0x124AC0
#define ROM_OFFSET_ANIMATION_DATA      0x28E980
#define ROM_OFFSET_GLOBALIMAGETABLE    0x29D160
#define ROM_OFFSET_RAREWARELOGO        0x29E560
#define ROM_OFFSET_FONTBANKGOTHIC      0x2E63B0
#define ROM_OFFSET_FONTZURICHBOLD      0x2E8860
#define ROM_OFFSET_SFXCTL              0x2EBDC0
#define ROM_OFFSET_SFXTBL              0x2F19A0
#define ROM_OFFSET_INSTRUMENTSCTL      0x3B4490
#define ROM_OFFSET_INSTRUMENTSTBL      0x3B8830
#define ROM_OFFSET_MUSICSAMPLETBL      0x419790

u32 rom_resolver_get_resource_size(s32 index);
#endif /* _ROM_RESOLVER_H_ */
