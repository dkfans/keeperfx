/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_traps.c
 *     Traps support functions.
 * @par Purpose:
 *     Functions to support trap things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_traps.h"

#include "globals.h"
#include "bflib_basics.h"
#include "thing_data.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct Thing *_DK_create_trap(struct Coord3d *pos, unsigned short a1, unsigned short a2);
DLLIMPORT struct Thing *_DK_get_trap_for_position(long pos_x, long pos_y);
DLLIMPORT struct Thing *_DK_get_trap_for_slab_position(long slb_x, long slb_y);
DLLIMPORT long _DK_update_trap(struct Thing *thing);

/******************************************************************************/
TbBool destroy_trap(struct Thing *thing)
{
  delete_thing_structure(thing, 0);
  return true;
}

TbBool trap_is_active(const struct Thing *thing)
{
  return ((thing->byte_13 > 0) && (thing->long_14 <= game.play_gameturn));
}

TbBool trap_is_slappable(const struct Thing *thing, long plyr_idx)
{
  if (thing->owner == plyr_idx)
  {
    return (thing->model == 1) && trap_is_active(thing);
  }
  return false;
}

struct Thing *get_trap_for_position(long pos_x, long pos_y)
{
  return _DK_get_trap_for_position(pos_x, pos_y);
}

struct Thing *get_trap_for_slab_position(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    //return _DK_get_trap_for_slab_position(slb_x, slb_y);
    return get_trap_around_of_model_and_owned_by(subtile_coord_center(3*slb_x+1), subtile_coord_center(3*slb_y+1), -1, -1);
}

long update_trap(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  return _DK_update_trap(thing);
}

struct Thing *create_trap(struct Coord3d *pos, unsigned short a1, unsigned short a2)
{
  SYNCDBG(7,"Starting");
  return _DK_create_trap(pos, a1, a2);
}

void init_traps(void)
{
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[7].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    if (thing->byte_13 == 0)
    {
      thing->byte_13 = game.traps_config[thing->model].shots;
      thing->field_4F ^= (thing->field_4F ^ (trap_stats[thing->model].field_12 << 4)) & 0x30;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
