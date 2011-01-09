/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_hero.h
 *     Header file for creature_states_hero.c.
 * @par Purpose:
 *     Creature state machine functions for heroes.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSTATEHERO_H
#define DK_CRTRSTATEHERO_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
short good_attack_room(struct Thing *thing);
short good_back_at_start(struct Thing *thing);
short good_doing_nothing(struct Thing *thing);
short good_drops_gold(struct Thing *thing);
short good_leave_through_exit_door(struct Thing *thing);
short good_returns_to_start(struct Thing *thing);
short good_wait_in_exit_door(struct Thing *thing);
short creature_hero_entering(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
