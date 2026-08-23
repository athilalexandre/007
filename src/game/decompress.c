#include <ultra64.h>
#include "zlib.h"

extern u8 *inbuf;
extern u32 inptr;
extern u32 decompress_entry(void *src, void *dst, void *hlist);

u32 decompressdata(u8 *src, u8 *dst, struct huft *huffman_table)
{
    return decompress_entry(src, dst, (void *)huffman_table);
}

s32 rzipGetSomething(void) {
    return (s32)(inbuf + inptr);
}
