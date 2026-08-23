#include <stdio.h>
#include <ultra64.h>
#include <bondgame.h>
#include "objecthandler.h"
#include <tex.h>
#include <model.h>
#include <memp.h>
#include <deb.h>
#include <bondconstants.h>
#include "ob.h"


void sub_GAME_7F0762E0(ModelFileHeader *objheader, u8 *name, u8 *dst, struct texpool *buffer)
{
    printf("[sub_GAME_7F0762E0] name=%s dst=%p buffer=%p\n", name, dst, buffer); fflush(stdout);
    ModelNode *node;
    s32 romremaining;
    Gfx *gdl;
    s32 pcremaining;
    u32 replacementgdl;
    ModelNode *curnode;
    Gfx *curgdl;
    s32 delta;
    s32 filedata;
    s32 filenum;

    filedata = (s32) objheader->Switches;
    filenum = fileGetIndex((char *) name);

    romremaining = get_rom_remaining_buffer_for_index(filenum);
    pcremaining = get_pc_remaining_buffer_for_index(filenum);
    node = 0;
    modelIterateDisplayLists(objheader, &node, &gdl);
#ifdef TARGET_WEB
    if ((uintptr_t)gdl > 0x01000000 && ((uintptr_t)gdl & 0xFF) == 0x05) {
        gdl = (Gfx *)__builtin_bswap32((uintptr_t)gdl);
    }
#endif

    if (gdl != 0)
    {
        name = (u8 *) ((pcremaining - ((s32) (((u8 *) objheader->Switches) + (((u32) gdl) & 0x00ffffff)))) + ((s32) filedata));
        
        /* The signed lvalue cast is required for the compiler to choose the target registers. */
        replacementgdl = (u32)*(s32 *)&gdl;
        
        delta = ((s32) ((romremaining + filedata) - (s32) name)) - ((s32) (((u8 *) objheader->Switches) + (((u32) gdl) & 0x00ffffff)));
        
        texCopyGdls((Gfx *) (((u8 *) objheader->Switches) + (((u32) gdl) & 0x00ffffff)), (Gfx *) ((romremaining + filedata) - (s32) name), (s32) name);

        printf("[sub_GAME_7F0762E0] before texLoadFromModelFileHeader\n"); fflush(stdout);
        texLoadFromModelFileHeader(objheader, buffer);
        printf("[sub_GAME_7F0762E0] after texLoadFromModelFileHeader\n"); fflush(stdout);

        if (node != 0)
        {
            do
            {
                curnode = node;
                curgdl = gdl;
                modelIterateDisplayLists(objheader, &node, &gdl);
#ifdef TARGET_WEB
                if ((uintptr_t)gdl > 0x01000000 && ((uintptr_t)gdl & 0xFF) == 0x05) {
                    gdl = (Gfx *)__builtin_bswap32((uintptr_t)gdl);
                }
#endif
                
                if (gdl != 0)
                {
                    name = (u8 *) (((s32) gdl) - ((s32) curgdl));
                }
                else
                {
                    name = (u8 *) ((((s32) (filedata + pcremaining)) - ((s32) objheader->Switches)) - (((u32) curgdl) & 0x00ffffff));
                }
                
                modelNodeReplaceGdl((u32) objheader, curnode, curgdl, (Gfx *) replacementgdl);
                printf("[sub_GAME_7F0762E0] calling texLoadFromGdl srcsize=%d replacementgdl=0x%x\n", (s32)name, replacementgdl); fflush(stdout);
                s32 written = texLoadFromGdl( (Gfx *) ((((u8 *) objheader->Switches) + (((u32) curgdl) & 0x00ffffff)) + delta), (s32) name, (Gfx *) (((u8 *) objheader->Switches) + (replacementgdl & 0x00ffffff)), buffer);
                printf("[sub_GAME_7F0762E0] texLoadFromGdl returned written=%d\n", written); fflush(stdout);
                replacementgdl += written;
            } 
            while (node != 0);
            printf("[sub_GAME_7F0762E0] loop finished\n"); fflush(stdout);
        }

        name = (u8 *) (((s32) (((u8 *) objheader->Switches) + (replacementgdl & 0x00ffffff))) - filedata);

        fileSetSize(filenum, (u8 *) filedata, (((s32) name + 0xf) & (~0xf)), dst == 0);
    }
}


void load_object_fill_header(struct ModelFileHeader *objheader, u8 *name, u8* dst, s32 size, struct texpool * buffer)
{
    void *filedata;

    if (dst != 0)
    {
        filedata = _fileNameLoadToAddr(name, 0, dst, size);
    }
    else
    {
        filedata = _fileNameLoadToBank(name, 0, 0x100, 4);
    }
    
    objheader->Switches = (struct ModelNode **)filedata;
    
    // hmmmmmmmmmmmm
    objheader->Textures = (struct ModelFileTextures *)&((s32*)filedata)[objheader->numSwitches];
    
    objheader->RootNode = (struct ModelNode *)&objheader->Textures[objheader->numtextures];
    
    sub_GAME_7F075A90(objheader, 0x5000000, filedata);
    sub_GAME_7F0762E0(objheader, name, dst, buffer);
}

void fileLoad(struct ModelFileHeader *header,char *name)
{
   load_object_fill_header(header,name,0,0,0);
   return;
}

void load_object_into_memory_unused_maybe(struct ModelFileHeader *header,int *recallstring,int *targetloc,int sizeleft)
{
   load_object_fill_header(header,recallstring,targetloc,sizeleft,0);
   return;
}