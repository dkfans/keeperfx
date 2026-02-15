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
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/**
 * Type which contains buffer of a sprite, with RLE-encoded alpha channel.
 */
typedef unsigned char * TbSpriteData;

struct TbSprite {
    TbSpriteData Data;
#ifdef SPRITE_FORMAT_V2
    unsigned short SWidth;
    unsigned short SHeight;
#else
    unsigned char SWidth;
    unsigned char SHeight;
#endif
};

struct TbSpriteSheet;

struct TbSetupSprite {
    struct TbSprite **Start;
    struct TbSprite **End;
    TbSpriteData *Data;
};

struct TbHugeSprite {
    TbSpriteData Data;  //**< Raw sprite data, with RLE coded transparency.
    int32_t * Lines;  //**< Index of line starts in the sprite data.
    unsigned long SWidth;
    unsigned long SHeight;
};

struct TiledSprite {
    unsigned char x_num;
    unsigned char y_num;
    unsigned short spr_idx[10][10];
};

struct TbSpriteSheet * create_spritesheet(void);
struct TbSpriteSheet * load_spritesheet(const char * data_fname, const char * index_fname);
void free_spritesheet(struct TbSpriteSheet **);
const struct TbSprite * get_sprite(const struct TbSpriteSheet *, long index);
#ifdef SPRITE_FORMAT_V2
TbBool add_sprite(struct TbSpriteSheet * sheet, unsigned short width, unsigned short height, int size, const void * data);
#else
TbBool add_sprite(struct TbSpriteSheet * sheet, unsigned char width, unsigned char height, int size, const void * data);
#endif
long num_sprites(const struct TbSpriteSheet *);

#define load_font(data_fname, index_fname) load_spritesheet(data_fname, index_fname)
#define free_font(font) free_spritesheet(font)

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
