/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_gardn.h
 *     Header file for creature_states_gardn.c.
 * @par Purpose:
 *     Creature state machine functions for their job in various rooms.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSTATEGARDN_H
#define DK_CRTRSTATEGARDN_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/
TbBool creature_able_to_eat(const struct Thing *creatng);
TbBool hunger_is_creature_hungry(const struct Thing *creatng);

short creature_arrived_at_garden(struct Thing *thing);
short creature_eat(struct Thing *thing);
short creature_eating_at_garden(struct Thing *thing);
short creature_to_garden(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
