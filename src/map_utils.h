/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_utils.h
 *     Header file for map_utils.c.
 * @par Purpose:
 *     Map related utility functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Jul 2010 - 05 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_MAP_UTILS_H
#define DK_MAP_UTILS_H

#include "bflib_basics.h"
#include "globals.h"

#define SPIRAL_STEPS_RANGE     50
#define SPIRAL_STEPS_COUNT   (SPIRAL_STEPS_RANGE*SPIRAL_STEPS_RANGE)

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Coord3d;

struct MapOffset {
  char v;
  char h;
  unsigned short both;
};

#pragma pack()

typedef struct CompoundCoordFilterParam * MaxCoordFilterParam;

/** Definition of a callback type used for selecting best position by maximizing a value. */
typedef long (*Coord_Maximizer_Filter)(const struct Coord3d *, MaxCoordFilterParam, long);

typedef TbBool (*SlabsFillIterAction)(MapSlabCoord, MapSlabCoord, MaxCoordFilterParam);

struct CompoundCoordFilterParam {
     long plyr_idx;
     long slab_kind;
     union {
     long num1;
     void *ptr1;
     };
     union {
     long num2;
     void *ptr2;
     };
     union {
     long num3;
     void *ptr3;
     };
};

/******************************************************************************/
DLLIMPORT struct MapOffset _DK_spiral_step[SPIRAL_STEPS_COUNT];
#define spiral_step _DK_spiral_step
/******************************************************************************/
#define AROUND_TILES_COUNT      9
extern struct Around const around[];
#define MID_AROUND_LENGTH 9
extern struct Around const mid_around[MID_AROUND_LENGTH];
extern struct Around const start_at_around[MID_AROUND_LENGTH];
#define SMALL_AROUND_LENGTH 4
extern struct Around const small_around[];
#define SMALL_AROUND_MID_LENGTH 5
extern struct Around const small_around_mid[];
/******************************************************************************/
void init_spiral_steps(void);

void get_min_floor_and_ceiling_heights_for_rect(MapSubtlCoord stl_x_beg, MapSubtlCoord stl_y_beg,
    MapSubtlCoord stl_x_end, MapSubtlCoord stl_y_end,
    MapSubtlCoord *floor_height, MapSubtlCoord *ceiling_height);

void slabs_fill_iterate_from_slab(MapSlabCoord src_slab_x, MapSlabCoord src_slab_y, SlabsFillIterAction f_action, MaxCoordFilterParam param);

unsigned int small_around_index_towards_destination(long curr_x,long curr_y,long dest_x,long dest_y);

long pos_move_in_direction_to_last_allowing_drop(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, unsigned short slabs_dist);
long pos_move_in_direction_to_outside_player_room(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, unsigned short slabs_dist);
long pos_move_in_direction_to_blocking_wall_or_lava(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, unsigned short slabs_dist);
long pos_move_in_direction_to_unowned_filled_or_water(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, unsigned short slabs_dist);

long near_coord_filter_battle_drop_point(const struct Coord3d *pos, MaxCoordFilterParam param, long maximizer);

TbBool get_position_spiral_near_map_block_with_filter(struct Coord3d *retpos, MapCoord x, MapCoord y,
    long spiral_len, Coord_Maximizer_Filter filter, MaxCoordFilterParam param);

long slabs_count_near(MapSlabCoord tx, MapSlabCoord ty, long rad, SlabKind slbkind);

TbBool subtile_is_blocking_wall_or_lava(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);

enum FillIterType {
    FillIterType_NoFill,
    FillIterType_Match,
    FillIterType_Floor,
    FillIterType_FloorBridge,
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
