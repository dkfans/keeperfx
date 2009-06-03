/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_data.c
 *     Map array data management functions.
 * @par Purpose:
 *     Functions to support the map data array, which stores map blocks information.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     15 May 2009 - 12 Apr 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_data.h"
#include "globals.h"

#include "slab_data.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Map bad_map_block;
const long map_to_slab[] = {
   0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,  5,  5,
   6,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11,
  12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17,
  18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
  24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29,
  30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35,
  36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41,
  42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47,
  48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53,
  54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59,
  60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65,
  66, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71,
  72, 72, 72, 73, 73, 73, 74, 74, 74, 75, 75, 75, 76, 76, 76, 77, 77, 77,
  78, 78, 78, 79, 79, 79, 80, 80, 80, 81, 81, 81, 82, 82, 82, 83, 83, 83,
  84, 84, 84, 85, 85, 85, 86, 86, 86, 87, 87, 87, 88, 88, 88, 89, 89, 89,
};
/******************************************************************************/
struct Map *get_map_block(long stl_x, long stl_y)
{
  if ((stl_x < 0) || (stl_x > map_subtiles_x))
      return &bad_map_block;
  if ((stl_y < 0) || (stl_y > map_subtiles_y))
      return &bad_map_block;
  return &game.map[get_subtile_number(stl_x,stl_y)];
}

TbBool map_block_invalid(struct Map *map)
{
  if (map == NULL)
    return true;
  if (map == &bad_map_block)
    return true;
  return (map < &game.map[0]);
}

/*
 * Subtile number - stores both X and Y coords in one number.
 */
unsigned long get_subtile_number(long stl_x, long stl_y)
{
  if (stl_x > map_subtiles_x+1) stl_x = map_subtiles_x+1;
  if (stl_y > map_subtiles_y+1) stl_y = map_subtiles_y+1;
  if (stl_x < 0)  stl_x = 0;
  if (stl_y < 0) stl_y = 0;
  return stl_y*(map_subtiles_x+1) + stl_x;
}

/*
 * Decodes X coordinate from subtile number.
 */
long stl_num_decode_x(unsigned long stl_num)
{
  return stl_num % (map_subtiles_x+1);
}

/*
 * Decodes Y coordinate from subtile number.
 */
long stl_num_decode_y(unsigned long stl_num)
{
  return (stl_num/(map_subtiles_x+1))%map_subtiles_y;
}

/*
 * Returns subtile coordinate for central subtile on given slab.
 */
long slab_center_subtile(long stl_v)
{
  return map_to_slab[stl_v]*3+1;
}

/*
 * Returns subtile coordinate for starting subtile on given slab.
 */
long slab_starting_subtile(long stl_v)
{
  return map_to_slab[stl_v]*3;
}
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
