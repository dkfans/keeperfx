/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_utils.c
 *     Map related utility functions.
 * @par Purpose:
 *     Helper functions for various simple map-related tasks.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Jul 2010 - 05 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_utils.h"

#include "globals.h"
#include "bflib_basics.h"

#include "map_blocks.h"
#include "slab_data.h"
#include "room_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
const struct Around around[] = {
  {-1,-1},
  {-1, 0},
  {-1, 1},
  { 0,-1},
  { 0, 0},
  { 0, 1},
  { 1,-1},
  { 1, 0},
  { 1, 1},
  { 0, 0}, // this entry shouldn't be used
};
/******************************************************************************/
DLLIMPORT long _DK_get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void init_spiral_steps(void)
{
    struct MapOffset *sstep;
    long x,y;
    long i;
    y = 0;
    x = 0;
    sstep = &spiral_step[0];
    sstep->h = y;
    sstep->v = x;
    sstep->both = (short)y + ((short)x << 8);
    y = -1;
    x = -1;
    for (i=1; i < SPIRAL_STEPS_COUNT; i++)
    {
      sstep = &spiral_step[i];
      sstep->h = y;
      sstep->v = x;
      sstep->both = (short)y + ((short)x << 8);
      if ((y < 0) && (x-y == 1))
      {
          y--;
          x -= 2;
      } else
      if (x == y)
      {
          if (y < 0)
            y++;
          else
            y--;
      } else
      if (y+x == 0)
      {
          if (x >= 0)
            x--;
          else
            x++;
      } else
      if (abs(x) >= abs(y))
      {
          if (x < 0)
            y++;
          else
            y--;
      } else
      {
          if (y >= 0)
            x++;
          else
            x--;
      }
    }
}

long get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos)
{
  return _DK_get_floor_height_under_thing_at(thing, pos);
}
/******************************************************************************/
