/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file slab_data.c
 *     Map Slabs support functions.
 * @par Purpose:
 *     Definitions and functions to maintain map slabs.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Apr 2009 - 12 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "slab_data.h"
#include "globals.h"

#include "player_instances.h"
#include "config_terrain.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const short around_slab[] = {-86, -85, -84,  -1,   0,   1,  84,  85,  86};
const short small_around_slab[] = {-85,   1,  85,  -1};
struct SlabMap bad_slabmap_block;
/******************************************************************************/
DLLIMPORT long _DK_calculate_effeciency_score_for_room_slab(long a1, long plyr_idx);
DLLIMPORT void _DK_update_blocks_in_area(long sx, long sy, long ex, long ey);
/******************************************************************************/
/**
 * Returns slab number, which stores both X and Y coords in one number.
 */
unsigned long get_slab_number(long slb_x, long slb_y)
{
  if (slb_x > map_tiles_x) slb_x = map_tiles_x;
  if (slb_y > map_tiles_y) slb_y = map_tiles_y;
  if (slb_x < 0)  slb_x = 0;
  if (slb_y < 0) slb_y = 0;
  return slb_y*(map_tiles_x) + slb_x;
}

/**
 * Decodes X coordinate from slab number.
 */
long slb_num_decode_x(unsigned long slb_num)
{
  return slb_num % (map_tiles_x);
}

/**
 * Decodes Y coordinate from slab number.
 */
long slb_num_decode_y(unsigned long slb_num)
{
  return (slb_num/(map_tiles_x))%map_tiles_y;
}

/**
 * Returns SlabMap struct for given slab number.
 */
struct SlabMap *get_slabmap_direct(long slab_num)
{
  if ((slab_num < 0) || (slab_num >= map_tiles_x*map_tiles_y))
      return INVALID_SLABMAP_BLOCK;
  return &game.slabmap[slab_num];
}

/**
 * Returns SlabMap struct for given (X,Y) slab coords.
 */
struct SlabMap *get_slabmap_block(long slab_x, long slab_y)
{
  if ((slab_x < 0) || (slab_x >= map_tiles_x))
      return INVALID_SLABMAP_BLOCK;
  if ((slab_y < 0) || (slab_y >= map_tiles_y))
      return INVALID_SLABMAP_BLOCK;
  return &game.slabmap[slab_y*(map_tiles_x) + slab_x];
}

/**
 * Returns SlabMap struct for given (X,Y) subtile coords.
 */
struct SlabMap *get_slabmap_for_subtile(long stl_x, long stl_y)
{
  if ((stl_x < 0) || (stl_x >= map_subtiles_x))
      return INVALID_SLABMAP_BLOCK;
  if ((stl_y < 0) || (stl_y >= map_subtiles_y))
      return INVALID_SLABMAP_BLOCK;
  return &game.slabmap[map_to_slab[stl_y]*(map_tiles_x) + map_to_slab[stl_x]];
}

/**
 * Returns if given SlabMap is not a part of the map.
 */
TbBool slabmap_block_invalid(struct SlabMap *slb)
{
  if (slb == NULL)
    return true;
  if (slb == INVALID_SLABMAP_BLOCK)
    return true;
  return (slb < &game.slabmap[0]);
}

/**
 * Returns owner index of given SlabMap.
 */
long slabmap_owner(struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return 5;
    return slb->field_5 & 0x07;
}

/**
 * Sets owner of given SlabMap.
 */
void slabmap_set_owner(struct SlabMap *slb, long owner)
{
    if (slabmap_block_invalid(slb))
        return;
    slb->field_5 ^= (slb->field_5 ^ owner) & 0x07;
}

/**
 * Sets owner of a slab on given position.
 */
void set_whole_slab_owner(long slb_x, long slb_y, long owner)
{
    struct SlabMap *slb;
    long stl_x,stl_y;
    long i,k;
    stl_x = 3 * slb_x;
    stl_y = 3 * slb_y;
    for (i = 0; i < 3; i++)
    {
      for (k = 0; k < 3; k++)
      {
          slb = get_slabmap_for_subtile(stl_x + k, stl_y + i);
          slabmap_set_owner(slb, owner);
      }
    }
}

/**
 * Returns slab number of the next tile in a room, after the given one.
 */
long get_next_slab_number_in_room(long slab_num)
{
    if ((slab_num < 0) || (slab_num >= map_tiles_x*map_tiles_y))
        return 0;
    return game.slabmap[slab_num].next_in_room;
}

TbBool slab_is_safe_land(long plyr_idx, long slb_x, long slb_y)
{
  struct SlabMap *slb;
  struct SlabAttr *slbattr;
  int slb_owner;
  slb = get_slabmap_block(slb_x, slb_y);
  slbattr = get_slab_attrs(slb);
  slb_owner = slabmap_owner(slb);
  if ((slb_owner == plyr_idx) || (slb_owner == game.neutral_player_num))
  {
      return slbattr->is_safe_land;
  }
  return false;
}

/**
 * Clears all SlabMap structures in the map.
 */
void clear_slabs(void)
{
  struct SlabMap *slb;
  unsigned long x,y;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = &game.slabmap[y*map_tiles_x + x];
      memset(slb, 0, sizeof(struct SlabMap));
      slb->slab = SlbT_ROCK;
    }
}

long calculate_effeciency_score_for_room_slab(long slab_num, long plyr_idx)
{
    return _DK_calculate_effeciency_score_for_room_slab(slab_num, plyr_idx);
}

/**
 * Reveals the whole map for specific player.
 */
void reveal_whole_map(struct PlayerInfo *player)
{
  clear_dig_for_map_rect(player->id_number,0,map_tiles_x,0,map_tiles_y);
  reveal_map_rect(player->id_number,1,map_subtiles_x,1,map_subtiles_y);
  pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
}

void update_blocks_in_area(long sx, long sy, long ex, long ey)
{
    _DK_update_blocks_in_area(sx, sy, ex, ey);
}

void update_blocks_around_slab(long slb_x, long slb_y)
{
    long stl_x,stl_y;
    long sx,sy,ex,ey;
    stl_x = 3 * slb_x;
    stl_y = 3 * slb_y;

    ey = stl_y + 5;
    if (ey >= 256)
        ey = 256;
    ex = stl_x + 5;
    if (ex >= 256)
        ex = 256;
    sy = stl_y - 3;
    if (sy <= 0)
        sy = 0;
    sx = stl_x - 3;
    if (sx <= 0)
        sx = 0;
    update_blocks_in_area(sx, sy, ex, ey);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
