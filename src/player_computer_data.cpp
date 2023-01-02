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
#include "bflib_memory.h"
#include "bflib_math.h"

#include "config.h"
#include "config_compp.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "creature_states.h"
#include "ariadne_wallhug.h"
#include "spdigger_stack.h"
#include "magic.h"
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
extern struct ComputerProcess BuildAllRooms3x3;
extern struct ComputerProcess BuildAllRooms4x4;
extern struct ComputerProcess BuildPrisonRoom;
extern struct ComputerProcess BuildTortureRoom;
extern struct ComputerProcess BuildScavengerRoom;
extern struct ComputerProcess BuildTempleRoom;
extern struct ComputerProcess BuildGraveyardRoom;
extern struct ComputerProcess BuildBarrackRoom;
extern struct ComputerProcess BuildTreasureRoom;
extern struct ComputerProcess BuildResearchRoom;
extern struct ComputerProcess BuildHatcheryRoom;
extern struct ComputerProcess BuildLairRoom;
extern struct ComputerProcess BuildTrainingRoom;
extern struct ComputerProcess BuildWorkshopRoom;
extern struct ComputerProcess DigToEntrance;
extern struct ComputerProcess DigToGoldForMoney;
extern struct ComputerProcess BuildTreasureRoom4x4;
extern struct ComputerProcess BuildLairRoom4x4;
extern struct ComputerProcess DigToCloseGoldForMoney;
extern struct ComputerProcess DigToGoldGreedy;
extern struct ComputerProcess DigToGoldGreedy2;
extern struct ComputerProcess ComputerSightOfEvil;
extern struct ComputerProcess ComputerSightOfEvilScare;
extern struct ComputerProcess ComputerAttack1;
extern struct ComputerProcess ComputerSafeAttack;
/******************************************************************************/

struct ValidRooms valid_rooms_to_build[] = {
  {RoK_TREASURE,  &BuildTreasureRoom},
  {RoK_LAIR,      &BuildLairRoom},
  {RoK_GARDEN,    &BuildHatcheryRoom},
  {RoK_LIBRARY,   &BuildResearchRoom},
  {RoK_TRAINING,  &BuildTrainingRoom},
  {RoK_WORKSHOP,  &BuildWorkshopRoom},
  {RoK_SCAVENGER, &BuildScavengerRoom},
  {RoK_PRISON,    &BuildPrisonRoom},
  {RoK_TEMPLE,    &BuildTempleRoom},
  {RoK_TORTURE,   &BuildTortureRoom},
  {RoK_GRAVEYARD, &BuildGraveyardRoom},
  {RoK_BARRACKS,  &BuildBarrackRoom},
  {-1,            NULL},
};

struct ComputerProcessMnemonic computer_process_config_list[] = {
  {"Unused", NULL,},
  {"", &BuildAllRooms3x3,},
  {"", &BuildAllRooms4x4,},
  {"", &BuildPrisonRoom,},
  {"", &BuildTortureRoom,},
  {"", &BuildScavengerRoom,},
  {"", &BuildTempleRoom,},
  {"", &BuildGraveyardRoom,},
  {"", &BuildBarrackRoom,},
  {"", &BuildTreasureRoom,},
  {"", &BuildResearchRoom,},
  {"", &BuildHatcheryRoom,},
  {"", &BuildLairRoom,},
  {"", &BuildTrainingRoom,},
  {"", &BuildWorkshopRoom,},
  {"", &DigToEntrance,},
  {"", &DigToGoldForMoney,},
  {"", &BuildTreasureRoom4x4,},
  {"", &BuildLairRoom4x4,},
  {"", &DigToCloseGoldForMoney,},
  {"", &DigToGoldGreedy,},
  {"", &DigToGoldGreedy2,},
  {"", &ComputerSightOfEvil,},
  {"", &ComputerSightOfEvilScare,},
  {"", &ComputerAttack1,},
  {"", &ComputerSafeAttack,},
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
