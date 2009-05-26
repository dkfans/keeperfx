/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_control.c
 *     CreatureControl structure support functions.
 * @par Purpose:
 *     Functions to use CreatureControl for controlling creatures.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     23 Apr 2009 - 16 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_control.h"
#include "globals.h"

#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/*
 * Returns CreatureControl of given index.
 */
struct CreatureControl *creature_control_get(long cctrl_idx)
{
  if ((cctrl_idx < 1) || (cctrl_idx > CREATURES_COUNT))
    return game.persons.cctrl_lookup[0];
  return game.persons.cctrl_lookup[cctrl_idx];
}

/*
 * Returns CreatureControl assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureControl *creature_control_get_from_thing(struct Thing *thing)
{
  if ((thing->field_64 < 1) || (thing->field_64 > CREATURES_COUNT))
    return game.persons.cctrl_lookup[0];
  return game.persons.cctrl_lookup[thing->field_64];
}

/*
 * Returns if given CreatureControl pointer is incorrect.
 */
TbBool creature_control_invalid(struct CreatureControl *cctrl)
{
  return (cctrl <= game.persons.cctrl_lookup[0]) || (cctrl == NULL);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
