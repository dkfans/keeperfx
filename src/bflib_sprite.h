/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sprite.h
 *     Header file for bflib_sprite.c.
 * @par Purpose:
 *     Graphics sprites support library.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_SPRITE_H
#define BFLIB_SPRITE_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

/**
 * Type which contains buffer of a sprite, with RLE-encoded alpha channel.
 */
typedef uint8_t * TbSpriteData;

struct TbSprite {
    const uint8_t * Data;
#ifdef SPRITE_FORMAT_V2
    uint16_t SWidth;
    uint16_t SHeight;
#else
    uint8_t SWidth;
    uint8_t SHeight;
#endif
};

struct HugeSprite;

struct TiledSprite {
    unsigned char x_num;
    unsigned char y_num;
    unsigned short spr_idx[10][10];
};

#pragma pack()

struct SpriteSheet;

/******************************************************************************/

struct SpriteSheet * LoadSprites(const char * basename);
void DeleteSprites(struct SpriteSheet **);
const struct TbSprite * GetSprite(const struct SpriteSheet *, size_t index);
size_t CountSprites(const struct SpriteSheet *);
struct HugeSprite * LoadHugeSprite(const char * filename, uint32_t width, uint32_t height);
void DeleteHugeSprite(struct HugeSprite **);
const uint8_t * HugeSpriteLine(const struct HugeSprite *, int row);
uint32_t HugeSpriteWidth(const struct HugeSprite *);
uint32_t HugeSpriteHeight(const struct HugeSprite *);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
