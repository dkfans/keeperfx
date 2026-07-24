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
  uint16_t both;
};

#pragma pack()

typedef struct CompoundCoordFilterParam * MaxCoordFilterParam;

/** Definition of a callback type used for selecting best position by maximizing a value. */
typedef int32_t (*Coord_Maximizer_Filter)(const struct Coord3d *, MaxCoordFilterParam, int32_t);

typedef TbBool (*SlabsFillIterAction)(MapSlabCoord, MapSlabCoord, MaxCoordFilterParam);

struct CompoundCoordFilterParam {
     int32_t plyr_idx;
     int32_t slab_kind;
     union {
     int32_t primary_number;
     void *primary_pointer;
     };
     union {
     int32_t secondary_number;
     void *secondary_pointer;
     };
     union {
     int32_t tertiary_number;
     void *tertiary_pointer;
     };
};

/******************************************************************************/
extern struct MapOffset spiral_step[SPIRAL_STEPS_COUNT];
/******************************************************************************/
#define AROUND_TILES_COUNT      9
extern struct Around const around[];
#define MID_AROUND_LENGTH 9
#define LARGE_AROUND_MAX 36
#define LARGE_AROUND_LIMITED 25
extern struct Around const mid_around[MID_AROUND_LENGTH];
extern struct Around const large_around[LARGE_AROUND_MAX];
extern struct Around const start_at_around[MID_AROUND_LENGTH];
#define SMALL_AROUND_LENGTH 4
extern struct Around const small_around[];
#define SMALL_AROUND_MID_LENGTH 5
extern struct Around const small_around_mid[];

#define AROUND_EIGHT_LENGTH 8
extern struct Around const my_around_eight[];
#define AROUND_NINE_LENGTH 9
extern struct Around const my_around_nine[];


/******************************************************************************/
void init_spiral_steps(void);

void get_min_floor_and_ceiling_heights_for_rect(MapSubtlCoord stl_x_beg, MapSubtlCoord stl_y_beg,
    MapSubtlCoord stl_x_end, MapSubtlCoord stl_y_end,
    MapSubtlCoord *floor_height, MapSubtlCoord *ceiling_height);

void slabs_fill_iterate_from_slab(MapSlabCoord src_slab_x, MapSlabCoord src_slab_y, SlabsFillIterAction f_action, MaxCoordFilterParam param);

SmallAroundIndex small_around_index_towards_destination(int32_t curr_x, int32_t curr_y, int32_t dest_x, int32_t dest_y);
SmallAroundIndex small_around_index_in_direction(int32_t srcpos_x, int32_t srcpos_y, int32_t dstpos_x, int32_t dstpos_y);

int32_t pos_move_in_direction_to_last_allowing_drop(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, uint16_t slabs_dist);
int32_t pos_move_in_direction_to_outside_player_room(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, uint16_t slabs_dist);
int32_t pos_move_in_direction_to_blocking_wall_or_lava(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, uint16_t slabs_dist);
int32_t pos_move_in_direction_to_unowned_filled_or_water(struct Coord3d *mvpos, unsigned char round_directn, PlayerNumber plyr_idx, uint16_t slabs_dist);

int32_t near_coord_filter_battle_drop_point(const struct Coord3d *pos, MaxCoordFilterParam param, int32_t maximizer);

TbBool get_position_spiral_near_map_block_with_filter(struct Coord3d *retpos, MapCoord x, MapCoord y,
    int32_t spiral_len, Coord_Maximizer_Filter filter, MaxCoordFilterParam param);
TbBool get_position_next_to_map_block_with_filter(struct Coord3d* retpos, MapCoord x, MapCoord y, Coord_Maximizer_Filter filter, MaxCoordFilterParam param);

int32_t slabs_count_near(MapSlabCoord tx, MapSlabCoord ty, int32_t rad, SlabKind slbkind);

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
