/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer_data.cpp
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Defines a computer player control variables and events/checks/processes
 *      functions.
 * @par Comment:  Entire file can be removed once everything is moved to config files.
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
enum compProcTemplate
{
    CPrcT_Null = 0,
    CPrcT_BuildAllRooms3x3,
    CPrcT_BuildAllRooms4x4,
    CPrcT_BuildPrisonRoom,
    CPrcT_BuildTortureRoom,
    CPrcT_BuildScavengerRoom,
    CPrcT_BuildTempleRoom,
    CPrcT_BuildGraveyardRoom,
    CPrcT_BuildBarrackRoom,
    CPrcT_BuildTreasureRoom,
    CPrcT_BuildResearchRoom,
    CPrcT_BuildHatcheryRoom,
    CPrcT_BuildLairRoom,
    CPrcT_BuildTrainingRoom,
    CPrcT_BuildWorkshopRoom,
    CPrcT_DigToEntrance,
    CPrcT_DigToGoldForMoney,
    CPrcT_BuildTreasureRoom4x4,
    CPrcT_BuildLairRoom4x4,
    CPrcT_DigToCloseGoldForMoney,
    CPrcT_DigToGoldGreedy,
    CPrcT_DigToGoldGreedy2,
    CPrcT_ComputerSightOfEvil,
    CPrcT_ComputerSightOfEvilScare,
    CPrcT_ComputerAttack1,
    CPrcT_ComputerSafeAttack,
}


/******************************************************************************/

//TODO move to config file
struct ValidRooms valid_rooms_to_build[] = {
  {RoK_TREASURE,  CPrcT_BuildTreasureRoom},
  {RoK_LAIR,      CPrcT_BuildLairRoom},
  {RoK_GARDEN,    CPrcT_BuildHatcheryRoom},
  {RoK_LIBRARY,   CPrcT_BuildResearchRoom},
  {RoK_TRAINING,  CPrcT_BuildTrainingRoom},
  {RoK_WORKSHOP,  CPrcT_BuildWorkshopRoom},
  {RoK_SCAVENGER, CPrcT_BuildScavengerRoom},
  {RoK_PRISON,    CPrcT_BuildPrisonRoom},
  {RoK_TEMPLE,    CPrcT_BuildTempleRoom},
  {RoK_TORTURE,   CPrcT_BuildTortureRoom},
  {RoK_GRAVEYARD, CPrcT_BuildGraveyardRoom},
  {RoK_BARRACKS,  CPrcT_BuildBarrackRoom},
  {-1,            NULL},
};


/******************************************************************************/
#ifdef __cplusplus
}
#endif
