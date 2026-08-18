const fs = require('fs');
const path = require('path');

const csvPath = path.join(__dirname, 'filelist.u.csv');
const csvContent = fs.readFileSync(csvPath, 'utf-8');
const lines = csvContent.trim().split('\n');

let entries = [];
for (const line of lines) {
    const parts = line.trim().split(',');
    if (parts.length >= 3) {
        const offset = parseInt(parts[0], 10);
        const size = parseInt(parts[1], 10);
        const name = parts[2];
        const compressed = parseInt(parts[3] || '0', 10);
        entries.push({ offset, size, name, compressed });
    }
}

console.log('Parsed ' + entries.length + ' entries from filelist.u.csv');

let hContent = `/*
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

#endif /* _ROM_RESOLVER_H_ */
`;

fs.writeFileSync(path.join(__dirname, '../src/platform/web/rom_resolver.h'), hContent);

let cContent = `/*
 * GoldenEye 007 (N64) Web ROM Resource Resolver
 * Authoritative numeric ROM segment offsets (USA Retail 12MB layout).
 */
#include "rom_resolver.h"
#include <string.h>

static const rom_segment_entry_t s_RomSegments[] = {
`;

for (const e of entries) {
    cContent += `    { "${e.name}", 0x${e.offset.toString(16).toUpperCase()}, ${e.size}, ${e.compressed} },\n`;
}

cContent += `};\n\n`;
cContent += `static const size_t s_RomSegmentCount = sizeof(s_RomSegments) / sizeof(s_RomSegments[0]);\n\n`;

cContent += `int rom_resolver_validate_layout(const u8 *romData, size_t romSize) {
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
`;

fs.writeFileSync(path.join(__dirname, '../src/platform/web/rom_resolver.c'), cContent);
console.log('Successfully generated rom_resolver.h and rom_resolver.c');