/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_prisn.h
 *     Header file for creature_states_prisn.c.
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
#ifndef DK_CRTRSTATEPRISN_H
#define DK_CRTRSTATEPRISN_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;

#pragma pack()
/******************************************************************************/
short cleanup_prison(struct Thing *thing);
short creature_arrived_at_prison(struct Thing *thing);
short creature_drop_body_in_prison(struct Thing *thing);
short creature_freeze_prisonors(struct Thing *thing);
CrStateRet creature_in_prison(struct Thing *thing);
CrCheckRet process_prison_function(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
