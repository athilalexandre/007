#include <ultra64.h>
#include <deb.h>
#include "memp.h"
#include "game/language.h"

/**
 * EU .data, offset from start of data_seg : 0x3640
*/

/**
 * @file memp.c
 * This file contains code for memp.
 */

//bss
MemoryPool g_mempPools[MEMPOOL_COUNT];

//data
void *ptr_memp_c_debug_debug_notice_list = 0;
s32 needmemallocation = 0;
s32 D_80024408 = 0;
s32 D_8002440C = 0;
s32 D_80024410 = 0;

//overloaded
struct s_mempMVALS sdefaultmvals = {
    MEMPOOL_MF + 1,    0,  // MEMPOOL_MF
    MEMPOOL_ML + 1,    82, // MEMPOOL_ML
    MEMPOOL_ME + 1,    15, // MEMPOOL_ME
    0,                 0   // MEMPOOL_END
};

void mempInit(void)
{
    debTryAdd(&ptr_memp_c_debug_debug_notice_list, "memp_c_debug");
}

const char *tokenFind(s32 arg0, const char *arg1);
long int strtol(const char *str, char **endptr, int base);
#include <stdlib.h>

void mempCheckMemflagTokens(s32 poolAreaStart, s32 poolAreaSize)
{
    s_mempMVALS poolSizes;
#ifdef TARGET_WEB
    static u8 *s_WebEngineHeap = NULL;
    if (s_WebEngineHeap == NULL) {
        u8 *raw = (u8 *)malloc(16 * 1024 * 1024 + 64);
        s_WebEngineHeap = (u8 *)(((uintptr_t)raw + 15) & ~15);
    }
    poolAreaStart = (s32)(uintptr_t)s_WebEngineHeap;
    poolAreaSize = 16 * 1024 * 1024;
#endif

    g_mempPools[MEMPOOL_TOTAL].start = (u8 *)(uintptr_t)poolAreaStart;
    g_mempPools[MEMPOOL_TOTAL].end = (u8 *)(uintptr_t)(poolAreaStart + poolAreaSize);

    poolSizes = sdefaultmvals;

    if (tokenFind(1, "-mf"))
    {
        poolSizes.mf = strtol(tokenFind(1, "-mf"), NULL, 0);
    }
    if (tokenFind(1, "-ml"))
    {
        poolSizes.ml = strtol(tokenFind(1, "-ml"), NULL, 0);
    }
    if (tokenFind(1, "-me"))
    {
        poolSizes.me = strtol(tokenFind(1, "-me"), NULL, 0);
    }
    if (poolSizes.me == 0)
    {
        poolSizes.mf = 0;
        poolSizes.me = 2 * 1024 * 1024; // 2MB permanent pool for web port
        poolSizes.ml = poolAreaSize - poolSizes.me;
    }

    mempSetBankStarts((s32*)&poolSizes);
}

void mempSetBankStarts(s32 poolSizes[MEMPOOL_COUNT+1])
{
    s32 i;
    s32 bankstarts[MEMPOOL_COUNT] = {0};
    s32 mempLen;
    s32 mempRequested;
    s32 mempStart;

    i = 0;
    do
    {
        bankstarts[poolSizes[i]] = poolSizes[i+1];
        i += 2;
    } while (poolSizes[i] != 0);

    for (i = MEMPOOL_TOTAL; i < MEMPOOL_COUNT - 1; i++)
    {
        bankstarts[i + 1] += bankstarts[i];
    }

    mempRequested = bankstarts[MEMPOOL_COUNT - 1];
    mempLen = (g_mempPools[MEMPOOL_TOTAL].end - g_mempPools[MEMPOOL_TOTAL].start);

    for (i = MEMPOOL_TOTAL; i < MEMPOOL_COUNT; i++)
    {
        bankstarts[i] = ((s64)bankstarts[i] * mempLen) / mempRequested;
    }

    for (i = MEMPOOL_TOTAL; i < MEMPOOL_COUNT; i++)
    {
        bankstarts[i] = (bankstarts[i] + 15) & ~15;
    }

    mempStart = (uintptr_t)g_mempPools[MEMPOOL_TOTAL].start;
    mempStart = (mempStart + 15) & ~15;

    for (i = MEMPOOL_TOTAL; i < MEMPOOL_COUNT - 1; i++)
    {
        g_mempPools[i + 1].start = (u8 *)(uintptr_t)(((bankstarts[i] + 15) & ~15) + mempStart);
        g_mempPools[i + 1].pos   = 0;
        g_mempPools[i + 1].end   = (u8 *)(uintptr_t)(((bankstarts[i + 1] + 15) & ~15) + mempStart);
    }
}


void *mempAllocBytesInBank(u32 bytes, u8 poolnum)
{
    MemoryPool *pool = &g_mempPools[poolnum];
    bytes = (bytes + 15) & ~15;
    u8 *allocation = (u8 *)(((uintptr_t)pool->pos + 15) & ~15);

    if (pool->pos == NULL)
    {
        osSyncPrintf("[mempAllocBytesInBank] ERROR: pool %d pos is NULL\n", poolnum);
        return NULL;
    }

    if (allocation + bytes > (u8 *)pool->end)
    {
        if (poolnum != MEMPOOL_PERMANENT && g_mempPools[MEMPOOL_PERMANENT].pos != NULL)
        {
            u8 *perm_alloc = (u8 *)(((uintptr_t)g_mempPools[MEMPOOL_PERMANENT].pos + 15) & ~15);
            if (perm_alloc + bytes <= (u8 *)g_mempPools[MEMPOOL_PERMANENT].end)
            {
                needmemallocation = TRUE;
                return mempAllocBytesInBank(bytes, MEMPOOL_PERMANENT);
            }
        }
        osSyncPrintf("[mempAllocBytesInBank] ERROR: pool %d OOM (pos=%p alloc=%p + 0x%x > end=%p)\n", poolnum, pool->pos, allocation, bytes, pool->end);
        return NULL;
    }

    pool->pos = (void *)(allocation + bytes);
    pool->prevpos = (void *)allocation;

    return allocation;
}


/**
 * Resize the most recent allocation in a pool without moving it.
 */
MEMP_ADD_ENTRY_RESULT mempAddEntryOfSizeToBank(void *allocation, s32 newsize, u8 poolnum)
{
    MemoryPool *pool;
    s32 origsize;
    s32 growsize;

    newsize = (newsize + 15) & ~15;

    if (needmemallocation && allocation == g_mempPools[MEMPOOL_PERMANENT].prevpos)
    {
        poolnum = MEMPOOL_PERMANENT;
    }

    pool = &g_mempPools[poolnum];

    if (pool->pos == 0)
    {
        osSyncPrintf("[mempAddEntryOfSizeToBank] pool %d pos is 0\n", poolnum);
        return MEMP_ADD_ENTRY_NOT_LAST_ALLOCATION;
    }

    if (allocation != pool->prevpos)
    {
        osSyncPrintf("[mempAddEntryOfSizeToBank] alloc=%p != prevpos=%p (pos=%p)\n", allocation, pool->prevpos, pool->pos);
        return MEMP_ADD_ENTRY_NOT_LAST_ALLOCATION;
    }

    origsize = (u8 *)pool->pos - (u8 *)pool->prevpos;
    growsize = newsize - origsize;

    osSyncPrintf("[mempAddEntryOfSizeToBank] pool %d: alloc=%p origsize=0x%x newsize=0x%x growsize=%d pos=%p -> %p\n",
        poolnum, allocation, origsize, newsize, growsize, pool->pos, (u8 *)pool->pos + growsize);

    if (growsize <= 0)
    {
        pool->pos = (void *)((u8 *)pool->pos + growsize);
        return MEMP_ADD_ENTRY_SUCCESS;
    }

    if ((u8 *)pool->pos + growsize > (u8 *)pool->end)
    {
        return MEMP_ADD_ENTRY_NOT_LAST_ALLOCATION;
    }

    pool->pos = (void *)((u8 *)pool->pos + growsize);
    return MEMP_ADD_ENTRY_SUCCESS;
}

void nulled_mempLoopAllMemBanks(void) {
    u8 bank;
    for (bank = MEMPOOL_MF; bank < MEMPOOL_COUNT; bank++)
    {
    }
}

s32 mempGetBankSizeLeft(u8 bank) {
    if (needmemallocation) {
        bank = MEMPOOL_PERMANENT;
    }

    if ((bank == MEMPOOL_STAGE) && (g_mempPools[MEMPOOL_STAGE].start == g_mempPools[MEMPOOL_STAGE].end))
    {
        bank = MEMPOOL_PERMANENT;
    }

    uintptr_t cur_alloc = ((uintptr_t)g_mempPools[bank].pos + 15) & ~15;
    uintptr_t end_pos = (uintptr_t)g_mempPools[bank].end;
    if (cur_alloc >= end_pos) {
        return 0;
    }
    return (s32)(end_pos - cur_alloc);
}

// Last three bits contains the bank, the rest contains the size.
u32 mempAllocPackedBytesInBank(u32 sizeandbank) {
    return (u32)(uintptr_t)mempAllocBytesInBank((sizeandbank >> 3), (sizeandbank & 7));
}

void mempResetBank(u8 bank) {
    g_mempPools[bank].prevpos = 0;
    g_mempPools[bank].pos = g_mempPools[bank].start;
}

void mempNullNextEntryInBank(u8 bank) {
    nulled_mempLoopAllMemBanks();
    if (g_mempPools[bank].pos != 0) {
        g_mempPools[bank].pos = 0;
    }
}
