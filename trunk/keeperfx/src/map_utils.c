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
#include "map_data.h"
#include "slab_data.h"
#include "room_data.h"
#include "power_hand.h"

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

/**
 * Returns minimal floor and ceiling heights for subtiles in given range.
 * @param stl_x_beg First subtile to be checked, X coord.
 * @param stl_y_beg First subtile to be checked, Y coord.
 * @param stl_x_end Last subtile to be checked, X coord.
 * @param stl_y_end Last subtile to be checked, Y coord.
 * @param floor_height Floor height value reference. Set to max floor height in range.
 * @param ceiling_height Ceiling height value reference. Set to min ceiling height in range.
 */
void get_min_floor_and_ceiling_heights_for_rect(MapSubtlCoord stl_x_beg, MapSubtlCoord stl_y_beg,
    MapSubtlCoord stl_x_end, MapSubtlCoord stl_y_end,
    MapSubtlCoord *floor_height, MapSubtlCoord *ceiling_height)
{
    MapSubtlCoord stl_x, stl_y;
    *floor_height = 0;
    *ceiling_height = 15;
    for (stl_y = stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (stl_x = stl_x_beg; stl_x <= stl_x_end; stl_x++)
        {
            update_floor_and_ceiling_heights_at(stl_x, stl_y,
                floor_height, ceiling_height);
        }
    }
}

long near_coord_filter_battle_drop_point(const struct Coord3d *pos, MaxCoordFilterParam param, long maximizer)
{
    if (can_drop_thing_here(pos->x.stl.num, pos->y.stl.num, param->plyr_idx, 1))
    {
        if (!is_dangerous_drop_subtile(pos->x.stl.num, pos->y.stl.num))
        {
            // This function should return max value when the place is good for dropping.
            return LONG_MAX;
        }
    }
    // If conditions are not met, return -1 to be sure the position will not be returned.
    return -1;
}


/**
 * Returns filtered position from subtiles around given coordinates.
 * Uses "spiral" checking of surrounding subtiles, up to given number of subtiles.
 * The position which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return LONG_MAX, the current position will be returned
 * immediately and no further subtiles will be checked.
 * @return Returns true if coordinates were found, false otherwise.
 */
TbBool get_position_spiral_near_map_block_with_filter(struct Coord3d *retpos, MapCoord x, MapCoord y, long spiral_len, Coord_Maximizer_Filter filter, MaxCoordFilterParam param)
{
    struct MapOffset *sstep;
    long maximizer;
    struct Map *mapblk;
    MapSubtlCoord sx,sy;
    int around;
    SYNCDBG(19,"Starting");
    maximizer = 0;
    for (around=0; around < spiral_len; around++)
    {
      sstep = &spiral_step[around];
      sx = coord_subtile(x) + (MapSubtlCoord)sstep->h;
      sy = coord_subtile(y) + (MapSubtlCoord)sstep->v;
      mapblk = get_map_block_at(sx, sy);
      if (!map_block_invalid(mapblk))
      {
          long n;
          n = maximizer;
          struct Coord3d newpos;
          newpos.x.val = subtile_coord_center(sx);
          newpos.y.val = subtile_coord_center(sy);
          newpos.z.val = 0;
          n = filter(&newpos, param, n);
          if (n >= maximizer)
          {
              retpos->x.val = newpos.x.val;
              retpos->y.val = newpos.y.val;
              retpos->z.val = newpos.z.val;
              maximizer = n;
              if (maximizer == LONG_MAX)
                  break;
          }
      }
    }
    return (maximizer > 0);
}

long slabs_count_near(MapSlabCoord tx, MapSlabCoord ty, long rad, SlabKind slbkind)
{
    long dx,dy;
    long x,y;
    long count;
    count=0;
    struct SlabMap *slb;
    for (dy=-rad; dy <= rad; dy++)
    {
        y = ty+dy;
        if ((y < 0) || (y >= map_tiles_y))
            continue;
        for (dx=-rad; dx <= rad; dx++)
        {
          x = tx+dx;
          if ((x < 0) || (x >= map_tiles_x))
              continue;
          slb = get_slabmap_block(x, y);
          if (slb->kind == slbkind)
            count++;
        }
    }
    return count;
}
/******************************************************************************/
