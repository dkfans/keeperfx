/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file slab_data.h
 *     Header file for slab_data.c.
 * @par Purpose:
 *     Map Slabs support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Apr 2009 - 12 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_SLABDATA_H
#define DK_SLABDATA_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

enum SlabTypes {
    SlbT_ROCK               =   0,
    SlbT_GOLD               =   1,
    SlbT_EARTH              =   2,
    SlbT_TORCHDIRT          =   3,
    SlbT_WALLDRAPE          =   4,
    SlbT_WALLTORCH          =   5,
    SlbT_WALLWTWINS         =   6,
    SlbT_WALLWWOMAN         =   7,
    SlbT_WALLPAIRSHR        =   8,
    SlbT_PATH               =  10,
    SlbT_CLAIMED            =  11,
    SlbT_LAVA               =  12,
    SlbT_WATER              =  13,
    SlbT_ENTRANCE           =  14,
    SlbT_TREASURE           =  16,
    SlbT_LIBRARY            =  18,
    SlbT_PRISON             =  20,
    SlbT_TORTURE            =  22,
    SlbT_TRAINING           =  24,
    SlbT_DUNGHEART          =  26,
    SlbT_WORKSHOP           =  28,
    SlbT_SCAVENGER          =  30,
    SlbT_TEMPLE             =  32,
    SlbT_GRAVEYARD          =  34,
    SlbT_GARDEN             =  36,
    SlbT_LAIR               =  38,
    SlbT_BARRACKS           =  40,
    SlbT_DOORWOOD1          =  42,
    SlbT_DOORWOOD2          =  43,
    SlbT_DOORBRACE1         =  44,
    SlbT_DOORBRACE2         =  45,
    SlbT_DOORIRON1          =  46,
    SlbT_DOORIRON2          =  47,
    SlbT_DOORMAGIC1         =  48,
    SlbT_DOORMAGIC2         =  49,
    SlbT_BRIDGE             =  51,
    SlbT_GEMS               =  52,
    SlbT_GUARDPOST          =  53,
};

/******************************************************************************/
#pragma pack(1)

typedef unsigned char SlabType;

struct SlabMap {
      SlabType slab;
      short field_1;
      unsigned char room_index;
      unsigned char field_4;
      unsigned char field_5;
};

struct SlabAttr {
    short field_0;
    short field_2;
    short field_4;
    long field_6;
    long field_A;
    unsigned char field_E;
    unsigned char field_F;
    unsigned char field_10;
    unsigned char field_11;
    unsigned char field_12;
    unsigned char field_13;
    unsigned char field_14;
    unsigned char field_15;
};

struct SlabSet { // sizeof = 18
  short col_idx[9];
};

struct SlabObj { // sizeof = 13
  unsigned char field_0[11];
  unsigned short field_B;
};

#pragma pack()
/******************************************************************************/
void reveal_whole_map(struct PlayerInfo *player);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
