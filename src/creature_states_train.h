/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_train.h
 *     Header file for creature_states_train.c.
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
#ifndef DK_CRTRSTATETRAIN_H
#define DK_CRTRSTATETRAIN_H

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
short at_training_room(struct Thing *thing);
short training(struct Thing *thing);
TbBool creature_can_be_trained(const struct Thing *thing);
TbBool player_can_afford_to_train_creature(const struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
