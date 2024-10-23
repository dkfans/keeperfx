/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_guibtns.c
 *     Graphics sprites support library.
 * @par Purpose:
 *     Functions for reading/writing, decoding/encodeing of sprites.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Nov 2008 - 09 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_sprite.h"
#include "bflib_filelst.h"
#include "bflib_basics.h"
#include "bflib_dernc.h"
#include "bflib_memory.h"
#include "globals.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct TbSpriteSheet {
    struct TbSprite * sprites;
    unsigned long count;
};

struct TbSpriteSheet * load_spritesheet(const char * datafile, const char * indexfile)
{
    // Sprite sheets are allocated as one, contiguous chunk.
    // Data elements are then organized as follows:
    // +----------------------+
    // | TbSpriteSheet itself |
    // +----------------------+
    // | array of TbSprite    |
    // +----------------------+
    // | sprite data          |
    // +----------------------+

    #pragma pack(1)
#ifdef SPRITE_FORMAT_V2
    struct sprite_entry {
        uint32_t offset;
        uint16_t width;
        uint16_t height;
    };
#else
    struct sprite_entry {
        uint32_t offset;
        uint8_t width;
        uint8_t height;
    };
#endif
    #pragma pack(0)

    // load index file
    const char * fname = modify_data_load_filename_function(indexfile);
    const int index_size = LbFileLengthRnc(fname);
    if (index_size <= 0) return NULL;
    const int num_sprites = index_size / sizeof(struct sprite_entry);
    const int sprites_offset = sizeof(struct TbSpriteSheet);
    const int sprites_size = sizeof(struct TbSprite) * num_sprites;
    const int data_offset = sprites_offset + sprites_size;
    void * ptr = LbMemoryAlloc(data_offset + index_size);
    if (!ptr) return NULL;
    struct TbSpriteSheet * sheet = ptr;
    struct sprite_entry * entries = (struct sprite_entry *) &((char *)ptr)[data_offset];
    if (LbFileLoadAt(fname, entries) != index_size) {
        LbMemoryFree(sheet);
        return NULL;
    }

    // populate sprite fields
    struct TbSprite * sprites = (struct TbSprite *) &((char *)ptr)[sprites_offset];
    for (int i = 0; i < num_sprites; ++i) {
        sprites[i].Data = (unsigned char *)(entries[i].offset);
        sprites[i].SWidth = entries[i].width;
        sprites[i].SHeight = entries[i].height;
    }

    // load data file
    fname = modify_data_load_filename_function(datafile);
    const int data_size = LbFileLengthRnc(fname);
    if (data_size <= 0) {
        LbMemoryFree(sheet);
        return NULL;
    }
    ptr = LbMemoryGrow(sheet, data_offset + data_size);
    if (!ptr) {
        LbMemoryFree(sheet);
    }
    sheet = ptr;
    void * data = &((char *)ptr)[data_offset];
    if (LbFileLoadAt(fname, data) != data_size) {
        LbMemoryFree(sheet);
        return NULL;
    }

    // populate sheet fields
    sheet->sprites = sprites = (struct TbSprite *) &((char *)ptr)[sprites_offset];
    sheet->count = num_sprites;

    // convert offsets to pointers
    for (int i = 0; i < num_sprites; ++i) {
        sprites[i].Data = &((unsigned char *)data)[(uintptr_t)sprites[i].Data];
    }
    return sheet;
}

void free_spritesheet(struct TbSpriteSheet ** sheet)
{
    if (sheet) {
        LbMemoryFree(*sheet);
        *sheet = NULL;
    }
}

const struct TbSprite * get_sprite(const struct TbSpriteSheet * sheet, const long index)
{
    if (!sheet) {
        return NULL;
    } else if (index >= sheet->count) {
        return NULL;
    }
    return &sheet->sprites[index];
}

long num_sprites(const struct TbSpriteSheet * sheet)
{
    if (!sheet) {
        return 0;
    }
    return sheet->count;
}

/******************************************************************************/
short LbSpriteSetup(struct TbSprite *start, const struct TbSprite *end, const unsigned char * data)
{
    int n = 0;
    struct TbSprite* sprt = start;
    while (sprt < end)
    {
      if ((unsigned long)sprt->Data < (unsigned long)data)
      {
        sprt->Data += (unsigned long)data;
        n++;
      }
      sprt++;
    }
#ifdef __DEBUG
    LbSyncLog("%s: initied %d of %d sprites\n",func_name,n,(sprt-start));
#endif
    return 1;
}

int LbSpriteSetupAll(struct TbSetupSprite t_setup[])
{
    int idx = 0;
    struct TbSetupSprite* stp_sprite = &t_setup[idx];
    while (stp_sprite->Data != NULL)
    {
      if ((stp_sprite->Start != NULL) && (stp_sprite->End != NULL))
        LbSpriteSetup(*(stp_sprite->Start), *(stp_sprite->End), (unsigned char *)*(stp_sprite->Data));
      idx++;
      stp_sprite=&t_setup[idx];
    }
#ifdef __DEBUG
    LbSyncLog("%s: Initiated %d SetupSprite lists\n",func_name,idx);
#endif
    return 1;
}

int LbSpriteClearAll(struct TbSetupSprite t_setup[])
{
    int idx = 0;
    struct TbSetupSprite* stp_sprite = &t_setup[idx];
    while (stp_sprite->Data != NULL)
    {
        if ((stp_sprite->Start != NULL) && (stp_sprite->End != NULL))
        {
            *(stp_sprite->Start) = NULL;
            *(stp_sprite->End) = NULL;
            *(stp_sprite->Data) = 0;
        }
        idx++;
        stp_sprite = &t_setup[idx];
  }
#ifdef __DEBUG
  LbSyncLog("%s: Cleaned %d SetupSprite lists\n",func_name,idx);
#endif
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
