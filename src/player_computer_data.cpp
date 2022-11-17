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
DKIMPORT struct ComputerProcess _DK_BuildAllRooms3x3;
#define BuildAllRooms3x3 _DK_BuildAllRooms3x3
DKIMPORT struct ComputerProcess _DK_BuildAllRooms4x4;
#define BuildAllRooms4x4 _DK_BuildAllRooms4x4
DKIMPORT struct ComputerProcess _DK_BuildPrisonRoom;
#define BuildPrisonRoom _DK_BuildPrisonRoom
DKIMPORT struct ComputerProcess _DK_BuildTortureRoom;
#define BuildTortureRoom _DK_BuildTortureRoom
DKIMPORT struct ComputerProcess _DK_BuildScavengerRoom;
#define BuildScavengerRoom _DK_BuildScavengerRoom
DKIMPORT struct ComputerProcess _DK_BuildTempleRoom;
#define BuildTempleRoom _DK_BuildTempleRoom
DKIMPORT struct ComputerProcess _DK_BuildGraveyardRoom;
#define BuildGraveyardRoom _DK_BuildGraveyardRoom
DKIMPORT struct ComputerProcess _DK_BuildBarrackRoom;
#define BuildBarrackRoom _DK_BuildBarrackRoom
DKIMPORT struct ComputerProcess _DK_BuildTreasureRoom;
#define BuildTreasureRoom _DK_BuildTreasureRoom
DKIMPORT struct ComputerProcess _DK_BuildResearchRoom;
#define BuildResearchRoom _DK_BuildResearchRoom
DKIMPORT struct ComputerProcess _DK_BuildHatcheryRoom;
#define BuildHatcheryRoom _DK_BuildHatcheryRoom
DKIMPORT struct ComputerProcess _DK_BuildLairRoom;
#define BuildLairRoom _DK_BuildLairRoom
DKIMPORT struct ComputerProcess _DK_BuildTrainingRoom;
#define BuildTrainingRoom _DK_BuildTrainingRoom
DKIMPORT struct ComputerProcess _DK_BuildWorkshopRoom;
#define BuildWorkshopRoom _DK_BuildWorkshopRoom
DKIMPORT struct ComputerProcess _DK_DigToEntrance;
#define DigToEntrance _DK_DigToEntrance
DKIMPORT struct ComputerProcess _DK_DigToGoldForMoney;
#define DigToGoldForMoney _DK_DigToGoldForMoney
DKIMPORT struct ComputerProcess _DK_BuildTreasureRoom4x4;
#define BuildTreasureRoom4x4 _DK_BuildTreasureRoom4x4
DKIMPORT struct ComputerProcess _DK_BuildLairRoom4x4;
#define BuildLairRoom4x4 _DK_BuildLairRoom4x4
DKIMPORT struct ComputerProcess _DK_DigToCloseGoldForMoney;
#define DigToCloseGoldForMoney _DK_DigToCloseGoldForMoney
DKIMPORT struct ComputerProcess _DK_DigToGoldGreedy;
#define DigToGoldGreedy _DK_DigToGoldGreedy
DKIMPORT struct ComputerProcess _DK_DigToGoldGreedy2;
#define DigToGoldGreedy2 _DK_DigToGoldGreedy2
DKIMPORT struct ComputerProcess _DK_ComputerSightOfEvil;
#define ComputerSightOfEvil _DK_ComputerSightOfEvil
DKIMPORT struct ComputerProcess _DK_ComputerSightOfEvilScare;
#define ComputerSightOfEvilScare _DK_ComputerSightOfEvilScare
DKIMPORT struct ComputerProcess _DK_ComputerAttack1;
#define ComputerAttack1 _DK_ComputerAttack1
DKIMPORT struct ComputerProcess _DK_ComputerSafeAttack;
#define ComputerSafeAttack _DK_ComputerSafeAttack
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
