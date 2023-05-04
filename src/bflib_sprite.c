/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sprite.c
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
// #include "bflib_basics.h"
#include "bflib_memory.h"
// #include "globals.h"
#include "post_inc.h"

#pragma packed(1)
struct TbDiskSprite {
    uint32_t offset;
#ifdef SPRITE_FORMAT_V2
    uint16_t width;
    uint16_t height;
#else
    uint8_t width;
    uint8_t height;
#endif
};
#pragma packed()

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
int LbSpriteSetup(struct TbSprite ** start, struct TbSprite ** end, const uint8_t * data)
{
    // The on-disk sprites have a 32-bit offset whereas the in-memory sprites have a
    // pointer to RLE-encoded data. This causes all kinds of breakage in 64-bit builds.
    // In addition, LbDataLoad does not understand sprites are being loaded and
    // allocates insufficient memory to hold the sprite descriptors.

    // Lots of pointer shenanigans ahead because of above and compiler
    // really wants to align TbDiskSprite on 8-byte boundaries.

    const long sprite_size = sizeof(struct TbSprite);
#ifdef SPRITE_FORMAT_V2
    const long disk_sprite_size = 8;
#else
    const long disk_sprite_size = 6;
#endif
    struct TbDiskSprite * disk_sprites = (struct TbDiskSprite*) *start;
    const long num_sprites = (((char *)*end) - ((char *)*start)) / disk_sprite_size;
    if (num_sprites == 0) {
        return 1;
    }
    struct TbSprite * sprites = (struct TbSprite *) LbMemoryAlloc(num_sprites * sprite_size);
    if (sprites == NULL) {
        return 0;
    }
    for (long i = 0; i < num_sprites; ++i)
    {
        struct TbDiskSprite * src = (struct TbDiskSprite *) &((char *) disk_sprites)[i * disk_sprite_size];
        struct TbSprite * dst = &sprites[i];
        dst->Data = &data[src->offset];
        dst->SWidth = src->width;
        dst->SHeight = src->height;
    }
    LbMemoryFree(*start);
    *start = sprites;
    *end = &sprites[num_sprites];
#ifdef __DEBUG
    SYNCLOG("Initialized %d sprites", num_sprites);
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
        LbSpriteSetup(stp_sprite->Start, stp_sprite->End, (unsigned char *)*(stp_sprite->Data));
      idx++;
      stp_sprite=&t_setup[idx];
    }
#ifdef __DEBUG
    SYNCLOG("Initiated %d SetupSprite lists",idx);
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
  SYNCLOG("Cleaned %d SetupSprite lists",idx);
#endif
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
