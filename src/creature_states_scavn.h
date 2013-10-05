/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_scavn.h
 *     Header file for creature_states_scavn.c.
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
#ifndef DK_CRTRSTATESCAVN_H
#define DK_CRTRSTATESCAVN_H

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
TbBool creature_can_do_scavenging(const struct Thing *creatng);
TbBool player_can_afford_to_scavenge_creature(const struct Thing *creatng);
short at_scavenger_room(struct Thing *thing);
short creature_being_scavenged(struct Thing *thing);
short creature_scavenged_disappear(struct Thing *thing);
short creature_scavenged_reappear(struct Thing *thing);
CrCheckRet process_scavenge_function(struct Thing *thing);
CrStateRet scavengering(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
