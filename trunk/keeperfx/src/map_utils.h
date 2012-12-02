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

struct MapOffset {
  char v;
  char h;
  unsigned short both;
};

#pragma pack()
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
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
