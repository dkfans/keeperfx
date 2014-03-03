/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_states.h
 *     Header file for player_states.c.
 * @par Purpose:
 *     Player states definitions and functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Nov 2012 - 02 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_STATES_H
#define DK_PLYR_STATES_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PLAYER_STATES_COUNT    32
/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;

#pragma pack()
/******************************************************************************/
extern unsigned short const player_state_to_power_kind[];
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
