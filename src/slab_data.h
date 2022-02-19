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
#include "config_terrain.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define SLABSET_COUNT        1304
#define SLABOBJS_COUNT        512

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
    SlbT_DAMAGEDWALL        =   9,
    SlbT_PATH               =  10,
    SlbT_CLAIMED            =  11,
    SlbT_LAVA               =  12,
    SlbT_WATER              =  13,
    SlbT_ENTRANCE           =  14,
    SlbT_ENTRANCE_WALL      =  15,
    SlbT_TREASURE           =  16,
    SlbT_TREASURE_WALL      =  17,
    SlbT_LIBRARY            =  18,
    SlbT_LIBRARY_WALL       =  19,
    SlbT_PRISON             =  20,
    SlbT_PRISON_WALL        =  21,
    SlbT_TORTURE            =  22,
    SlbT_TORTURE_WALL       =  23,
    SlbT_TRAINING           =  24,
    SlbT_TRAINING_WALL      =  25,
    SlbT_DUNGHEART          =  26,
    SlbT_DUNGHEART_WALL     =  27,
    SlbT_WORKSHOP           =  28,
    SlbT_WORKSHOP_WALL      =  29,
    SlbT_SCAVENGER          =  30,
    SlbT_SCAVENGER_WALL     =  31,
    SlbT_TEMPLE             =  32,
    SlbT_TEMPLE_WALL        =  33,
    SlbT_GRAVEYARD          =  34,
    SlbT_GRAVEYARD_WALL     =  35,
    SlbT_GARDEN             =  36,
    SlbT_GARDEN_WALL        =  37,
    SlbT_LAIR               =  38,
    SlbT_LAIR_WALL          =  39,
    SlbT_BARRACKS           =  40,
    SlbT_BARRACKS_WALL      =  41,
    SlbT_DOORWOOD1          =  42,
    SlbT_DOORWOOD2          =  43,
    SlbT_DOORBRACE1         =  44,
    SlbT_DOORBRACE2         =  45,
    SlbT_DOORIRON1          =  46,
    SlbT_DOORIRON2          =  47,
    SlbT_DOORMAGIC1         =  48,
    SlbT_DOORMAGIC2         =  49,
    SlbT_SLAB50             =  50, //Has special properties and is known as slab50 until a better name is found
    SlbT_BRIDGE             =  51,
    SlbT_GEMS               =  52,
    SlbT_GUARDPOST          =  53,
    SlbT_PURPLE             =  54,
};

/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;
struct Thing;

struct SlabMap {
      SlabKind kind;
      short next_in_room;
      unsigned char room_index;
      unsigned char health;
      unsigned char field_5;
};

struct SlabSet { // sizeof = 18
  short col_idx[9];
};

struct SlabObj { // sizeof = 13
  unsigned char field_0;
  short field_1;
  unsigned char field_3;
  short field_4;
  short field_6;
  short field_8;
  unsigned char field_A;
  unsigned char sofield_B;
  unsigned char sofield_C;
};

#pragma pack()
/******************************************************************************/
#define INVALID_SLABMAP_BLOCK (&bad_slabmap_block)
#define AROUND_SLAB_LENGTH 9
extern const short around_slab[];
#define SMALL_AROUND_SLAB_LENGTH 4
extern const short small_around_slab[];
/******************************************************************************/
SlabCodedCoords get_slab_number(MapSlabCoord slb_x, MapSlabCoord slb_y);
MapSlabCoord slb_num_decode_x(SlabCodedCoords slb_num);
MapSlabCoord slb_num_decode_y(SlabCodedCoords slb_num);

TbBool slab_kind_is_animated(SlabKind slbkind);

struct SlabMap *get_slabmap_block(MapSlabCoord slb_x, MapSlabCoord slb_y);
struct SlabMap *get_slabmap_for_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct SlabMap *get_slabmap_direct(SlabCodedCoords slab_num);
struct SlabMap *get_slabmap_thing_is_on(const struct Thing *thing);
TbBool slabmap_block_invalid(const struct SlabMap *slb);
TbBool slab_coords_invalid(MapSlabCoord slb_x, MapSlabCoord slb_y);
long slabmap_owner(const struct SlabMap *slb);
void slabmap_set_owner(struct SlabMap *slb, PlayerNumber owner);
void set_whole_slab_owner(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber owner);
PlayerNumber get_slab_owner_thing_is_on(const struct Thing *thing);
unsigned long slabmap_wlb(struct SlabMap *slb);
void slabmap_set_wlb(struct SlabMap *slb, unsigned long wlbflag);
long get_next_slab_number_in_room(SlabCodedCoords slab_num);
long calculate_effeciency_score_for_room_slab(SlabCodedCoords slab_num, PlayerNumber plyr_idx);

TbBool slab_is_safe_land(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool slab_is_door(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool slab_is_liquid(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool slab_is_wall(MapSlabCoord slb_x, MapSlabCoord slb_y);

TbBool can_build_room_at_slab(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y);

TbBool can_build_room_at_slab_fast(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y);

int check_room_at_slab_loose(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y, int looseness);

void clear_slabs(void);
void reveal_whole_map(struct PlayerInfo *player);
void update_blocks_in_area(MapSubtlCoord sx, MapSubtlCoord sy, MapSubtlCoord ex, MapSubtlCoord ey);
void update_blocks_around_slab(MapSlabCoord slb_x, MapSlabCoord slb_y);
void update_map_collide(SlabKind slbkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void copy_block_with_cube_groups(short itm_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void do_slab_efficiency_alteration(MapSlabCoord slb_x, MapSlabCoord slb_y);
void do_unprettying(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);

/******************************************************************************/
#include "roomspace.h"
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
