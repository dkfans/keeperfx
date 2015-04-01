/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_states.c
 *     Player states definitions and functions.
 * @par Purpose:
 *     Defines functions for player-related structures support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Nov 2012 - 02 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_states.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "player_data.h"
#include "player_instances.h"
#include "dungeon_data.h"
#include "power_hand.h"
#include "thing_objects.h"

/******************************************************************************/
/******************************************************************************/
// originally was player_state_to_spell
unsigned short const player_state_to_power_kind[PLAYER_STATES_COUNT] = {
  0,  0,  0,  0,  0,  0,  6,  7,  5,  0, 18, 18,  0,  0,  0,  0,
  0, 10,  0, 11, 12, 13,  8,  0,  2, 16, 14, 15,  0,  3, 16, 14,
 15, 18, 18,
};
/******************************************************************************/

/******************************************************************************/
