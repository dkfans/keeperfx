/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_tortr.h
 *     Header file for creature_states_tortr.c.
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
#ifndef DK_CRTRSTATETORTR_H
#define DK_CRTRSTATETORTR_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;

enum CreatureTortureVisualStates {
    CTVS_TortureRandMove,
    CTVS_TortureGoToDevice,
    CTVS_TortureInDevice,
};

#pragma pack()
/******************************************************************************/
short at_kinky_torture_room(struct Thing *thing);
CrStateRet kinky_torturing(struct Thing *thing);
CrCheckRet process_kinky_function(struct Thing *thing);

short at_torture_room(struct Thing *thing);
CrStateRet torturing(struct Thing *thing);
CrCheckRet process_torture_function(struct Thing *thing);
short cleanup_torturing(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
