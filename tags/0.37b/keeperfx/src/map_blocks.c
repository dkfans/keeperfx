/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_blocks.c
 *     Map blocks support functions.
 * @par Purpose:
 *     Functions to manage map blocks.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_blocks.h"

#include "globals.h"
#include "bflib_basics.h"

#include "slab_data.h"
#include "room_data.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_mine_out_block(long a1, long a2, long a3);
DLLIMPORT unsigned char _DK_dig_has_revealed_area(long a1, long a2, unsigned char a3);
DLLIMPORT void _DK_dig_out_block(long a1, long a2, long a3);
DLLIMPORT void _DK_check_map_explored(struct Thing *thing, long a2, long a3);
DLLIMPORT void _DK_create_gold_rubble_for_dug_block(long x, long y, unsigned char a3, unsigned char a4);
DLLIMPORT long _DK_untag_blocks_for_digging_in_area(long slb_x, long slb_y, signed char a3);
DLLIMPORT void _DK_set_slab_explored_flags(unsigned char flag, long slb_x, long slb_y);
DLLIMPORT long _DK_ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey);

/******************************************************************************/
TbBool block_has_diggable_side(long plyr_idx, long slb_x, long slb_y)
{
  long i;
  for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
  {
    if (slab_is_safe_land(plyr_idx, slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y))
      return true;
  }
  return false;
}

void create_gold_rubble_for_dug_block(long x, long y, unsigned char a3, unsigned char a4)
{
    _DK_create_gold_rubble_for_dug_block(x, y, a3, a4);
}

long untag_blocks_for_digging_in_area(long slb_x, long slb_y, long plyr_idx)
{
    return _DK_untag_blocks_for_digging_in_area(slb_x, slb_y, plyr_idx);
}

void all_players_untag_blocks_for_digging_in_area(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct PlayerInfo *player;
    struct Map *map;
    long plyr_idx;
    map = get_map_block_at(3*slb_x+1, 3*slb_y+1);
    for (plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if ((plyr_idx == hero_player_number) || (plyr_idx == game.neutral_player_num))
            continue;
        player = get_player(plyr_idx);
        if (player_exists(player))
        {
            if (map_block_revealed(map, plyr_idx))
            {
                untag_blocks_for_digging_in_area(3*slb_x, 3*slb_y, plyr_idx);
            }
        }
    }
}

void set_slab_explored_flags(unsigned char flag, long slb_x, long slb_y)
{
    _DK_set_slab_explored_flags(flag, slb_x, slb_y);
}

void create_gold_rubble_for_dug_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Column *col;
    MapSubtlCoord stl_x,stl_y;
    long x,y,z;
    stl_x = 3 * slb_x;
    stl_y = 3 * slb_y;
    col = get_column_at(stl_x, stl_y);
    if (column_invalid(col))
        z = 0;
    else
        z = (col->bitfileds >> 4) & 0x0F;
    for (y = stl_y; y < stl_y+3; y++)
    {
      for (x = stl_x; x < stl_x+3; x++)
      {
        if (z > 0)
          create_gold_rubble_for_dug_block(x, y, z, game.neutral_player_num);
      }
    }
}


void mine_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
    MapSlabCoord slb_x,slb_y;
    //_DK_mine_out_block(a1, a2, plyr_idx);
    slb_x = map_to_slab[stl_x];
    slb_y = map_to_slab[stl_y];
    create_gold_rubble_for_dug_slab(slb_x, slb_y);
    all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
    place_slab_type_on_map(10, 3*slb_x, 3*slb_y, game.neutral_player_num, 0);
    do_slab_efficiency_alteration(slb_x, slb_y);
    set_slab_explored_flags(1 << plyr_idx, slb_x, slb_y);
}

unsigned char dig_has_revealed_area(long a1, long a2, unsigned char a3)
{
    return _DK_dig_has_revealed_area(a1, a2, a3);
}

void dig_out_block(long a1, long a2, long a3)
{
    _DK_dig_out_block(a1, a2, a3);
}

void check_map_explored(struct Thing *thing, long a2, long a3)
{
    _DK_check_map_explored(thing, a2, a3);
}

long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey)
{
    return _DK_ceiling_partially_recompute_heights(sx, sy, ex, ey);
}

/*
char point_in_map_is_solid_including_lava_check_ignoring_door(struct Coord3d *pos, struct Thing *thing)
{

}*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
