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
#include "thing_effects.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_mine_out_block(long a1, long a2, long stl_height);
DLLIMPORT unsigned char _DK_dig_has_revealed_area(long a1, long a2, unsigned char stl_height);
DLLIMPORT void _DK_dig_out_block(long a1, long a2, long stl_height);
DLLIMPORT void _DK_check_map_explored(struct Thing *thing, long a2, long stl_height);
DLLIMPORT void _DK_create_gold_rubble_for_dug_block(long x, long y, unsigned char stl_height, unsigned char a4);
DLLIMPORT long _DK_untag_blocks_for_digging_in_area(long slb_x, long slb_y, signed char stl_height);
DLLIMPORT void _DK_set_slab_explored_flags(unsigned char flag, long slb_x, long slb_y);
DLLIMPORT long _DK_ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey);
DLLIMPORT long _DK_element_top_face_texture(struct Map *map);

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

void create_gold_rubble_for_dug_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_height, PlayerNumber owner)
{
    _DK_create_gold_rubble_for_dug_block(stl_x, stl_y, stl_height, owner);
}

void create_dirt_rubble_for_dug_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_height, PlayerNumber owner)
{
    struct Coord3d pos;
    MapCoord maxpos_z;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = subtile_coord_center(1);
    maxpos_z = subtile_coord(stl_height,0);
    while (pos.z.val < maxpos_z)
    {
        create_effect(&pos, 26, maxpos_z);
        pos.z.val += subtile_coord(1,0);
    }
}

long untag_blocks_for_digging_in_area(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct Map *mapblk;
    MapSubtlCoord x, y;
    long num_untagged;
    long task_idx;
    long i;
    //return _DK_untag_blocks_for_digging_in_area(slb_x, slb_y, plyr_idx);
    x = 3 * (stl_x/3);
    y = 3 * (stl_y/3);
    if ( (x < 0) || (x >= map_subtiles_x) || (y < 0) || (y >= map_subtiles_y) ) {
        ERRORLOG("Attempt to tag area outside of map");
        return 0;
    }
    i = get_subtile_number(x+1,y+1);
    task_idx = find_from_task_list(plyr_idx, i);
    if (task_idx != -1) {
        remove_from_task_list(plyr_idx, task_idx);
    }
    num_untagged = 0;
    if (is_my_player_number(plyr_idx))
    {
        long dx,dy;
        for (dy=0; dy < 3; dy++)
        {
            for (dx=0; dx < 3; dx++)
            {
                mapblk = get_map_block_at(x+dx, y+dy);
                if (map_block_invalid(mapblk))
                    continue;
                if ( mapblk->flags & (MapFlg_Unkn80|MapFlg_Unkn04) )
                  num_untagged++;
                mapblk->flags &= ~MapFlg_Unkn80;
                mapblk->flags &= ~MapFlg_Unkn04;
            }
        }
    }
    pannel_map_update(x, y, 3, 3);
    return num_untagged;
}

void all_players_untag_blocks_for_digging_in_area(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct PlayerInfo *player;
    struct Map *map;
    PlayerNumber plyr_idx;
    map = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
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

TbBool set_slab_explored(long plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    if ( (plyr_idx == game.neutral_player_num) || subtile_revealed(3*slb_x+1, 3*slb_y+1, plyr_idx) )
    {
        return false;
    }
    reveal_map_subtile(3*slb_x,   3*slb_y,   plyr_idx);
    reveal_map_subtile(3*slb_x+1, 3*slb_y,   plyr_idx);
    reveal_map_subtile(3*slb_x+2, 3*slb_y,   plyr_idx);
    reveal_map_subtile(3*slb_x,   3*slb_y+1, plyr_idx);
    reveal_map_subtile(3*slb_x+1, 3*slb_y+1, plyr_idx);
    reveal_map_subtile(3*slb_x+2, 3*slb_y+1, plyr_idx);
    reveal_map_subtile(3*slb_x,   3*slb_y+2, plyr_idx);
    reveal_map_subtile(3*slb_x+1, 3*slb_y+2, plyr_idx);
    reveal_map_subtile(3*slb_x+2, 3*slb_y+2, plyr_idx);
    pannel_map_update(3*slb_x, 3*slb_y, 3, 3);
    return true;
}

void set_slab_explored_flags(unsigned char flag, long slb_x, long slb_y)
{
    _DK_set_slab_explored_flags(flag, slb_x, slb_y);
}

void replacce_map_slab_when_destroyed(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SlabType nslab;
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    switch (slabmap_wlb(slb))
    {
    case 1:
        nslab = SlbT_LAVA;
        break;
    case 2:
        nslab = SlbT_WATER;
        break;
    default:
        nslab = SlbT_PATH;
        break;
    }
    place_slab_type_on_map(nslab, 3*slb_x, 3*slb_y, game.neutral_player_num, 0);
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

void get_floor_and_ceiling_heights_at(const struct Coord3d *pos, unsigned long *heights)
{
    struct Column *col;
    struct Map *mapblk;
    long i;
    unsigned long height,k;
    heights[0] = 0;
    heights[1] = 15;
    mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
    i = get_mapblk_column_index(mapblk);
    col = get_column(i);
    if (col->bitfileds & 0xF0)
        heights[0] = col->bitfileds >> 4;
    k = col->bitfileds & 0xE;
    if (k)
    {
        height = 8 - (k >> 1);
        if (height >= 15)
            height = 15;
        heights[1] = height;
    } else
    {
        height = get_mapblk_filled_subtiles(mapblk);
        if (height >= 15)
            height = 15;
        heights[1] = height;
    }
}

TbBool point_in_map_is_solid(const struct Coord3d *pos)
{
    struct Map *mapblk;
    struct Column *col;
    unsigned long heights[2];
    unsigned long check_h;
    col = get_column_at(pos->x.stl.num, pos->y.stl.num);
    check_h = pos->z.stl.num;
    if (col->bitfileds & 0xE)
    {
        get_floor_and_ceiling_heights_at(pos, heights);
    } else
    {
        mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
        heights[0] = col->bitfileds >> 4;
        heights[1] = get_mapblk_filled_subtiles(mapblk);
    }
    if ((heights[1] <= check_h) || (heights[0] > check_h))
        return 1;
    return 0;
}

/**
 * Destroys a tall gold slab, replacing it with neutral ground.
 * @param stl_x Slab subtile digged out, X coordinate.
 * @param stl_y Slab subtile digged out, Y coordinate.
 * @param plyr_idx Index of the player who does the digging.
 */
void mine_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSlabCoord slb_x,slb_y;
    //_DK_mine_out_block(a1, a2, plyr_idx);
    if (!subtile_has_slab(stl_x, stl_y))
    {
        ERRORLOG("Attempt to mine on invalid coordinates.");
        return;
    }
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    create_gold_rubble_for_dug_slab(slb_x, slb_y);
    all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
    replacce_map_slab_when_destroyed(slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    // Gold slabs are normally visible to all players,
    // so sine we're destroying it - make it invisible
    // TODO MAP Maybe it should be cleared only if sibling non-gold slab are invisible
    set_slab_explored_flags(1 << plyr_idx, slb_x, slb_y);
}

unsigned char dig_has_revealed_area(long a1, long a2, unsigned char a3)
{
    return _DK_dig_has_revealed_area(a1, a2, a3);
}

void create_dirt_rubble_for_dug_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
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
                create_dirt_rubble_for_dug_block(x, y, z, game.neutral_player_num);
        }
    }
}

/**
 * Destroys a tall dirt or wall slab, replacing it with neutral ground.
 * @param stl_x Slab subtile digged out, X coordinate.
 * @param stl_y Slab subtile digged out, Y coordinate.
 * @param plyr_idx Index of the player who does the digging.
 */
void dig_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSlabCoord slb_x,slb_y;
    //_DK_dig_out_block(stl_x, stl_y, plyr_idx);
    if (!subtile_has_slab(stl_x, stl_y))
    {
        ERRORLOG("Attempt to dig on invalid coordinates.");
        return;
    }
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    create_dirt_rubble_for_dug_slab(slb_x, slb_y);
    all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
    replacce_map_slab_when_destroyed(slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(plyr_idx);
        if (!dungeon_invalid(dungeon)) {
            dungeon->lvstats.rock_dug_out++;
        }
    }
}

void check_map_explored(struct Thing *thing, long a2, long a3)
{
    _DK_check_map_explored(thing, a2, a3);
}

long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey)
{
    return _DK_ceiling_partially_recompute_heights(sx, sy, ex, ey);
}

long element_top_face_texture(struct Map *map)
{
  return _DK_element_top_face_texture(map);
}

/*
char point_in_map_is_solid_including_lava_check_ignoring_door(struct Coord3d *pos, struct Thing *thing)
{

}*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
