/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_gardn.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_gardn.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_creature_arrived_at_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_eat(struct Thing *thing);
DLLIMPORT short _DK_creature_eating_at_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_to_garden(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short creature_arrived_at_garden(struct Thing *thing)
{
  return _DK_creature_arrived_at_garden(thing);
}

short creature_eat(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  //return _DK_creature_eat(thing);
  cctrl = creature_control_get_from_thing(thing);
  if (cctrl->instance_id != 36)
    internal_set_thing_state(thing, thing->continue_state);
  return 1;
}

short creature_eating_at_garden(struct Thing *thing)
{
  return _DK_creature_eating_at_garden(thing);
}

short creature_to_garden(struct Thing *thing)
{
  return _DK_creature_to_garden(thing);
}

/******************************************************************************/
