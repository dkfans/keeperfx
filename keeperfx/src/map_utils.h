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

#define SPIRAL_STEPS_COUNT   2500
#define AROUND_TILES_COUNT      9

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
extern const struct Around around[];
/******************************************************************************/
void init_spiral_steps(void);

void get_min_floor_and_ceiling_heights_for_rect(MapSubtlCoord stl_x_beg, MapSubtlCoord stl_y_beg,
    MapSubtlCoord stl_x_end, MapSubtlCoord stl_y_end,
    MapSubtlCoord *floor_height, MapSubtlCoord *ceiling_height);

long near_coord_filter_battle_drop_point(const struct Coord3d *pos, MaxCoordFilterParam param, long maximizer);

TbBool get_position_spiral_near_map_block_with_filter(struct Coord3d *retpos, MapCoord x, MapCoord y,
    long spiral_len, Coord_Maximizer_Filter filter, MaxCoordFilterParam param);

long slabs_count_near(MapSlabCoord tx, MapSlabCoord ty, long rad, SlabKind slbkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
