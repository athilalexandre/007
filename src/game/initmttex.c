#include <ultra64.h>
#include <memp.h>
#include <stdlib.h>
#include <stdio.h>
#include "image.h"
#include "initmttex.h"

void set_mt_tex_alloc(void)
{  
    s32 bytes = 0x100000; // 1MB Texture pool for stage assets
    g_TexCacheCount = 0;

    if (tokenFind(1, "-mt"))
    {
        bytes = strtol(tokenFind(1, "-mt"), 0x0, 0) * 1024; //get KB
    }

    u8 *poolMem = mempAllocBytesInBank(bytes, MEMPOOL_STAGE);
    printf("[set_mt_tex_alloc] poolMem=%p bytes=0x%x (%d KB)\n", poolMem, bytes, bytes / 1024);
    fflush(stdout);
    texInitPool(&ptr_texture_alloc_start, poolMem, bytes);
}
