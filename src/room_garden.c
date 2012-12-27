/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_garden.c
 *     Hatchery room maintain functions.
 * @par Purpose:
 *     Functions to create and use garden rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_garden.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_remove_food_from_food_room_if_possible(struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long remove_food_from_food_room_if_possible(struct Thing *thing)
{
  return _DK_remove_food_from_food_room_if_possible(thing);
}
/******************************************************************************/
