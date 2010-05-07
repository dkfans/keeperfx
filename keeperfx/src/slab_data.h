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
#include "bflib_basics.h"

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
#ifdef __cplusplus
#pragma pack(1)
#endif

struct PlayerInfo;
typedef unsigned char SlabType;

struct SlabMap {
      SlabType slab;
      short next_in_room;
      unsigned char room_index;
      unsigned char field_4;
      unsigned char field_5;
};

struct SlabSet { // sizeof = 18
  short col_idx[9];
};

struct SlabObj { // sizeof = 13
  unsigned char field_0[11];
  unsigned short field_B;
};

#define INVALID_SLABMAP_BLOCK (&bad_slabmap_block)
extern const short around_slab[];
#define SMALL_AROUND_SLAB_LENGTH 4
extern const short small_around_slab[];

#ifdef __cplusplus
#pragma pack()
#endif

/******************************************************************************/
unsigned long get_slab_number(long slb_x, long slb_y);
long slb_num_decode_x(unsigned long slb_num);
long slb_num_decode_y(unsigned long slb_num);

struct SlabMap *get_slabmap_block(long slab_x, long slab_y);
struct SlabMap *get_slabmap_for_subtile(long stl_x, long stl_y);
struct SlabMap *get_slabmap_direct(long slab_num);
TbBool slabmap_block_invalid(struct SlabMap *slb);
long slabmap_owner(struct SlabMap *slb);
void slabmap_set_owner(struct SlabMap *slb, long owner);
void set_whole_slab_owner(long slb_x, long slb_y, long owner);
long get_next_slab_number_in_room(long slab_num);
long calculate_effeciency_score_for_room_slab(long slab_num, long plyr_idx);
TbBool slab_is_safe_land(long plyr_idx, long slb_x, long slb_y);

void clear_slabs(void);
void reveal_whole_map(struct PlayerInfo *player);
void update_blocks_around_slab(long slb_x, long slb_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
