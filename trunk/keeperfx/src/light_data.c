/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file light_data.c
 *     light_data support functions.
 * @par Purpose:
 *     Functions to light_data.
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
#include "light_data.h"

#include "globals.h"
#include "bflib_basics.h"

#include "player_data.h"
#include "map_data.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_light_remove_light_from_list(struct Light *lgt, struct StructureList *list);
DLLIMPORT void _DK_light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2);
DLLIMPORT void _DK_light_delete_light(long idx);
DLLIMPORT void _DK_light_initialise_lighting_tables(void);
DLLIMPORT void _DK_light_set_light_minimum_size_to_cache(long a1, long a2, long a3);
DLLIMPORT long _DK_light_is_light_allocated(long lgt_id);
DLLIMPORT void _DK_light_set_light_position(long lgt_id, struct Coord3d *pos);
DLLIMPORT void _DK_light_initialise(void);
DLLIMPORT long _DK_light_create_light(struct InitLight *ilght);
DLLIMPORT void _DK_light_set_light_never_cache(long idx);
DLLIMPORT long _DK_light_get_light_intensity(long idx);
DLLIMPORT long _DK_light_set_light_intensity(long a1, long a2);
DLLIMPORT void _DK_light_render_area(int startx, int starty, int endx, int endy);
DLLIMPORT void _DK_light_stat_light_map_clear_area(long x1, long y1, long x2, long y2);
DLLIMPORT void _DK_light_signal_update_in_area(long sx, long sy, long ex, long ey);

/******************************************************************************/
TbBool light_add_light_to_list(struct Light *lgt, struct StructureList *list)
{
  if ((lgt->field_1 & 0x01) != 0)
  {
    ERRORLOG("Light is already in list");
    return false;
  }
  list->count++;
  lgt->field_1 |= 0x01;
  lgt->field_26 = list->index;
  list->index = lgt->field_E;
  return true;
}

long light_create_light(struct InitLight *ilght)
{
  return _DK_light_create_light(ilght);
}

void light_set_light_never_cache(long idx)
{
  _DK_light_set_light_never_cache(idx);
}

long light_is_light_allocated(long lgt_id)
{
  return _DK_light_is_light_allocated(lgt_id);
}

void light_set_light_position(long lgt_id, struct Coord3d *pos)
{
  _DK_light_set_light_position(lgt_id, pos);
}

void light_remove_light_from_list(struct Light *lgt, struct StructureList *list)
{
  _DK_light_remove_light_from_list(lgt, list);
}

void light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2)
{
  _DK_light_signal_stat_light_update_in_area(x1, y1, x2, y2);
}

void light_signal_update_in_area(long sx, long sy, long ex, long ey)
{
    _DK_light_signal_update_in_area(sx, sy, ex, ey);
}

void light_turn_light_off(long idx)
{
  struct Light *lgt;
  long x1,y1,x2,y2;

  if (idx == 0)
  {
    ERRORLOG("Attempt to turn off light 0");
    return;
  }
  lgt = &game.lights[idx];
  if ((lgt->field_0 & 0x01) == 0)
  {
    ERRORLOG("Attempt to turn off unallocated light structure");
    return;
  }
  if ((lgt->field_0 & 0x02) == 0)
    return;
  lgt->field_0 &= 0xFD;
  if ((lgt->field_0 & 0x04) != 0)
  {
    light_remove_light_from_list(lgt, &game.thing_lists[12]);
    return;
  }
  // Area bounds
  y2 = lgt->field_2B + lgt->field_5;
  if (y2 >= map_subtiles_y)
    y2 = map_subtiles_y;
  x2 = lgt->field_29 + lgt->field_5;
  if (x2 >= map_subtiles_x)
    x2 = map_subtiles_x;
  y1 = lgt->field_2B - lgt->field_5;
  if (y1 <= 0)
    y1 = 0;
  x1 = lgt->field_29 - lgt->field_5;
  if (x1 <= 0)
    x1 = 0;
  if ((x2 <= x1) || (y2 <= y1))
    return;
  light_signal_stat_light_update_in_area(x1, y1, x2, y2);
  light_remove_light_from_list(lgt, &game.thing_lists[11]);
  stat_light_needs_updating = 1;
}

void light_turn_light_on(long idx)
{
  struct Light *lgt;

  if (idx == 0)
  {
    ERRORLOG("Attempt to turn on light 0");
    return;
  }
  lgt = &game.lights[idx];
  if ((lgt->field_0 & 0x01) == 0)
  {
    ERRORLOG("Attempt to turn on unallocated light structure");
    return;
  }
  if ((lgt->field_0 & 0x02) != 0)
    return;
  lgt->field_0 |= 0x02;
  if ((lgt->field_0 & 0x04) == 0)
  {
    light_add_light_to_list(lgt, &game.thing_lists[11]);
    stat_light_needs_updating = 1;
    lgt->field_0 |= 0x08;
  } else
  {
    light_add_light_to_list(lgt, &game.thing_lists[12]);
    lgt->field_0 |= 0x08;
  }
}

long light_get_light_intensity(long idx)
{
  return _DK_light_get_light_intensity(idx);
}

long light_set_light_intensity(long a1, long a2)
{
  return _DK_light_set_light_intensity(a1, a2);
}

void clear_light_system(void)
{
  memset(game.field_1DD41, 0, 0x28416u);
}

void clear_stat_light_map(void)
{
  unsigned long x,y,i;
  game.field_46149 = 32;
  game.field_4614D = 0;
  game.field_4614F = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
  {
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      i = get_subtile_number(x,y);
      game.stat_light_map[i] = 0;
    }
  }
}

void light_delete_light(long idx)
{
  _DK_light_delete_light(idx);
}

void light_initialise_lighting_tables(void)
{
  _DK_light_initialise_lighting_tables();
}

void light_initialise(void)
{
  struct Light *lgt;
  int i;
  for (i=0; i < LIGHTS_COUNT; i++)
  {
    lgt = &game.lights[i];
    if (lgt->field_0 & 0x01)
      light_delete_light(lgt->field_E);
  }
  if (!game.field_4614E)
  {
    light_initialise_lighting_tables();
    for (i=0; i < 32; i++)
    {
      light_bitmask[i] = 1 << (31-i);
    }
    game.field_4614E = 1;
  }
}

void light_stat_light_map_clear_area(long x1, long y1, long x2, long y2)
{
  _DK_light_stat_light_map_clear_area(x1, y1, x2, y2);
}

void light_set_lights_on(char state)
{
  if (state)
  {
    game.field_46149 = 10;
    game.field_4614D = 1;
  } else
  {
    game.field_46149 = 32;
    game.field_4614D = 0;
  }
  // Enable lights on all but bounding subtiles
  light_stat_light_map_clear_area(0, 0, map_subtiles_x, map_subtiles_y);
  light_signal_stat_light_update_in_area(1, 1, map_subtiles_x, map_subtiles_y);
}

void light_render_area(int startx, int starty, int endx, int endy)
{
  _DK_light_render_area(startx, starty, endx, endy);
}

void update_light_render_area(void)
{
  int subtile_x,subtile_y;
  int delta_x,delta_y;
  int startx,endx,starty,endy;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  player=get_my_player();
  if (player->field_37 >= 1)
    if ((player->field_37 <= 2) || (player->field_37 == 5))
    {
        game.field_14BB5D = LIGHT_MAX_RANGE;
        game.field_14BB59 = LIGHT_MAX_RANGE;
    }
  delta_x=abs(game.field_14BB59);
  delta_y=abs(game.field_14BB5D);
  // Prepare the area constraits
  if (player->acamera != NULL)
  {
    subtile_y = player->acamera->mappos.y.stl.num;
    subtile_x = player->acamera->mappos.x.stl.num;
  } else
  {
    subtile_y = 0;
    subtile_x = 0;
  }
//SYNCMSG("LghtRng %d,%d CamTil %d,%d",game.field_14BB59,game.field_14BB5D,tile_x,tile_y);
  if (subtile_y > delta_y)
  {
    starty = subtile_y - delta_y;
    if (starty > map_subtiles_y) starty = map_subtiles_y;
  } else
    starty = 0;
  if (subtile_x > delta_x)
  {
    startx = subtile_x - delta_x;
    if (startx > map_subtiles_x) startx = map_subtiles_x;
  } else
    startx = 0;
  endy = subtile_y + delta_y;
  if (endy < starty) endy = starty;
  if (endy > map_subtiles_y) endy = map_subtiles_y;
  endx = subtile_x + delta_x;
  if (endx < startx) endx = startx;
  if (endx > map_subtiles_x) endx = map_subtiles_x;
  // Set the area
  light_render_area(startx, starty, endx, endy);
}

void light_set_light_minimum_size_to_cache(long a1, long a2, long a3)
{
  _DK_light_set_light_minimum_size_to_cache(a1, a2, a3);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
