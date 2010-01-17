/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_doors.c
 *     Door things support functions.
 * @par Purpose:
 *     Functions to create and operate on door things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_doors.h"

#include "globals.h"
#include "bflib_basics.h"

#include "thing_list.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_lock_door(struct Thing *thing);
/******************************************************************************/


/******************************************************************************/
TbBool remove_key_on_door(struct Thing *thing)
{
  struct Thing *keytng;
  keytng = find_base_thing_on_mapwho(1, 44, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
  if (thing_is_invalid(keytng))
    return false;
  delete_thing_structure(keytng, 0);
  return true;
}

TbBool add_key_on_door(struct Thing *thing)
{
  struct Thing *keytng;
  keytng = create_object(&thing->mappos, 44, thing->owner, 0);
  if (thing_is_invalid(keytng))
    return false;
  keytng->mappos.x.stl.pos = 128;
  keytng->mappos.y.stl.pos = 128;
  keytng->mappos.z.stl.num = 4;
  return true;
}

void unlock_door(struct Thing *thing)
{
  thing->byte_17.h = 0;
  game.field_14EA4B = 1;
  update_navigation_triangulation(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1,
    thing->mappos.x.stl.num+1, thing->mappos.y.stl.num+1);
  pannel_map_update(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1, 3, 3);
  if (!remove_key_on_door(thing))
    WARNMSG("Cannot remove keyhole when unlocking door.");
}

void lock_door(struct Thing *thing)
{
  _DK_lock_door(thing);
/*
  int v3;
  v3 = door_stats[0][thing->word_13.w0 + 2 * (unsigned int)thing->model].field_0;
  thing->field_7 = 2;
  *(short *)&thing->byte_13.f3 = 0;
  thing->byte17_h = 1;
  game.field_14EA4B = 1;
  place_animating_slab_type_on_map(v3, 0, thing->mappos.x_stl_num, thing->mappos.y_stl_num, thing->owner);
  update_navigation_triangulation(thing->mappos.x_stl_num-1,  thing->mappos.y_stl_num-1,
    thing->mappos.x_stl_num-1+2,thing->mappos.y_stl_num-1+2);
  pannel_map_update(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1, 3, 3);
  if (!add_key_on_door(thing))
    WARNMSG("Cannot create a keyhole when locking a door.");
*/
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
