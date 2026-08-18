#include <ultra64.h>
#include "bondtypes.h"
#include "initunk_005450.h"

#define MODELHITENTRIES_LEN 600

extern struct ModelHitEntry *g_ModelHitFreeList;

ModelHitEntry g_ModelHitEntriesPool[MODELHITENTRIES_LEN];
char *g_ModelHitEntries = (char *)g_ModelHitEntriesPool;
char *g_ModelHitEntriesPenultimate = (char *)&g_ModelHitEntriesPool[598];

void initModelHitEntryFreeList(void)
{
    s32 i;
    ModelHitEntry *entries = g_ModelHitEntriesPool;

    g_ModelHitFreeList = entries;

    entries[0].next = &entries[1];

    for (i = 1; i < MODELHITENTRIES_LEN - 1; i++)
    {
        entries[i].next = &entries[i + 1];
        entries[i].prev = &entries[i - 1];
    }

    entries[MODELHITENTRIES_LEN - 1].prev = &entries[598];
}
