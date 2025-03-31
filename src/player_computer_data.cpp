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
  {RoK_TREASURE,  &comp_player_conf.process_types[9]},
  {RoK_LAIR,      &comp_player_conf.process_types[12]},
  {RoK_GARDEN,    &comp_player_conf.process_types[11]},
  {RoK_LIBRARY,   &comp_player_conf.process_types[10]},
  {RoK_TRAINING,  &comp_player_conf.process_types[13]},
  {RoK_WORKSHOP,  &comp_player_conf.process_types[14]},
  {RoK_SCAVENGER, &comp_player_conf.process_types[5]},
  {RoK_PRISON,    &comp_player_conf.process_types[3]},
  {RoK_TEMPLE,    &comp_player_conf.process_types[6]},
  {RoK_TORTURE,   &comp_player_conf.process_types[4]},
  {RoK_GRAVEYARD, &comp_player_conf.process_types[7]},
  {RoK_BARRACKS,  &comp_player_conf.process_types[8]},
  {-1,            NULL},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
