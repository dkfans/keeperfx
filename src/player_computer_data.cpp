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

//TODO COMPUTER_PLAYER move to config file
unsigned short computer_types_tooltip_stridx[] = {
    GUIStr_Empty, GUIStr_Empty,
    GUIStr_Empty, GUIStr_Empty,
    GUIStr_Empty, GUIStr_Empty,
    GUIStr_AggressiveAssistDesc, GUIStr_DefensiveAssistDesc,
    GUIStr_ConstructionAssistDesc, GUIStr_MoveOnlyAssistDesc, };

/******************************************************************************/
extern struct ComputerProcess processes_list[];
/******************************************************************************/

struct ValidRooms valid_rooms_to_build[] = {
  {RoK_TREASURE,  &processes_list[9]},
  {RoK_LAIR,      &processes_list[12]},
  {RoK_GARDEN,    &processes_list[11]},
  {RoK_LIBRARY,   &processes_list[10]},
  {RoK_TRAINING,  &processes_list[13]},
  {RoK_WORKSHOP,  &processes_list[14]},
  {RoK_SCAVENGER, &processes_list[5]},
  {RoK_PRISON,    &processes_list[3]},
  {RoK_TEMPLE,    &processes_list[6]},
  {RoK_TORTURE,   &processes_list[4]},
  {RoK_GRAVEYARD, &processes_list[7]},
  {RoK_BARRACKS,  &processes_list[8]},
  {-1,            NULL},
};

struct ComputerProcessMnemonic computer_process_config_list[] = {
  {"Unused", &processes_list[0],},
  {"", &processes_list[1],},
  {"", &processes_list[2],},
  {"", &processes_list[3],},
  {"", &processes_list[4],},
  {"", &processes_list[5],},
  {"", &processes_list[6],},
  {"", &processes_list[7],},
  {"", &processes_list[8],},
  {"", &processes_list[9],},
  {"", &processes_list[10],},
  {"", &processes_list[11],},
  {"", &processes_list[12],},
  {"", &processes_list[13],},
  {"", &processes_list[14],},
  {"", &processes_list[15],},
  {"", &processes_list[16],},
  {"", &processes_list[17],},
  {"", &processes_list[18],},
  {"", &processes_list[19],},
  {"", &processes_list[20],},
  {"", &processes_list[21],},
  {"", &processes_list[22],},
  {"", &processes_list[23],},
  {"", &processes_list[24],},
  {"", &processes_list[25],},
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
