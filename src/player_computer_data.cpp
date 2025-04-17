/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer_data.cpp
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Defines a computer player control variables and events/checks/processes
 *      functions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "player_computer.h"

#include <limits.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_math.h"

#include "config.h"
#include "config_compp.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "creature_states.h"
#include "ariadne_wallhug.h"
#include "spdigger_stack.h"
#include "magic_powers.h"
#include "thing_traps.h"
#include "thing_navigate.h"
#include "player_complookup.h"
#include "power_hand.h"
#include "room_data.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

struct ValidRooms valid_rooms_to_build[] = {
  {RoK_TREASURE,  9},
  {RoK_LAIR,      12},
  {RoK_GARDEN,    11},
  {RoK_LIBRARY,   10},
  {RoK_TRAINING,  13},
  {RoK_WORKSHOP,  14},
  {RoK_SCAVENGER, 5},
  {RoK_PRISON,    3},
  {RoK_TEMPLE,    6},
  {RoK_TORTURE,   4},
  {RoK_GRAVEYARD, 7},
  {RoK_BARRACKS,  8},
  {-1,            0},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
