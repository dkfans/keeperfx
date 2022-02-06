/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_conditions.h
 *     Header file for lvl_script_conditions.c.
 * @par Purpose:
 *     should only be used by files under lvl_script_*
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_LVLSCRIPTCOND_H
#define DK_LVLSCRIPTCOND_H


#include "globals.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ScriptVariables {
  SVar_MONEY                           =  1,
  SVar_GAME_TURN                       =  5,
  SVar_BREAK_IN                        =  6,
  SVar_CREATURE_NUM                    =  7,
  SVar_TOTAL_DIGGERS                   =  8,
  SVar_TOTAL_CREATURES                 =  9,
  SVar_TOTAL_RESEARCH                  = 10,
  SVar_TOTAL_DOORS                     = 11,
  SVar_TOTAL_AREA                      = 12,
  SVar_TOTAL_CREATURES_LEFT            = 13,
  SVar_CREATURES_ANNOYED               = 14,
  SVar_BATTLES_LOST                    = 15,
  SVar_BATTLES_WON                     = 16,
  SVar_ROOMS_DESTROYED                 = 17,
  SVar_SPELLS_STOLEN                   = 18,
  SVar_ACTION_POINT_TRIGGERED          = 19,
  SVar_GOLD_POTS_STOLEN                = 20,
  SVar_TIMER                           = 21,
  SVar_DUNGEON_DESTROYED               = 22,
  SVar_TIMES_BROKEN_INTO               = 23,
  SVar_TOTAL_GOLD_MINED                = 24,
  SVar_FLAG                            = 25,
  SVar_ROOM_SLABS                      = 26,
  SVar_DOORS_DESTROYED                 = 27,
  SVar_CREATURES_SCAVENGED_LOST        = 28,
  SVar_CREATURES_SCAVENGED_GAINED      = 29,
  SVar_AVAILABLE_MAGIC                 = 30,
  SVar_AVAILABLE_TRAP                  = 31,
  SVar_AVAILABLE_DOOR                  = 32,
  SVar_AVAILABLE_ROOM                  = 33,
  SVar_AVAILABLE_CREATURE              = 34,
  SVar_CONTROLS_CREATURE               = 35,
  SVar_CONTROLS_TOTAL_CREATURES        = 36,
  SVar_CONTROLS_TOTAL_DIGGERS          = 37,
  SVar_ALL_DUNGEONS_DESTROYED          = 38,
  SVar_DOOR_NUM                        = 39,
  SVar_TRAP_NUM                        = 40,
  SVar_GOOD_CREATURES                  = 41,
  SVar_EVIL_CREATURES                  = 42,
  SVar_CONTROLS_GOOD_CREATURES         = 43,
  SVar_CONTROLS_EVIL_CREATURES         = 44,
  SVar_CAMPAIGN_FLAG                   = 45,
  SVar_SLAB_OWNER                      = 46,
  SVar_SLAB_TYPE                       = 47,
  SVar_HEART_HEALTH                    = 48,
  SVar_GHOSTS_RAISED                   = 49,
  SVar_SKELETONS_RAISED                = 50,
  SVar_VAMPIRES_RAISED                 = 51,
  SVar_CREATURES_CONVERTED             = 52,
  SVar_TIMES_ANNOYED_CREATURE          = 53,
  SVar_TIMES_TORTURED_CREATURE         = 54,
  SVar_TOTAL_DOORS_MANUFACTURED        = 55,
  SVar_TOTAL_TRAPS_MANUFACTURED        = 56,
  SVar_TOTAL_MANUFACTURED              = 57,
  SVar_TOTAL_TRAPS_USED                = 58,
  SVar_TOTAL_DOORS_USED                = 59,
  SVar_KEEPERS_DESTROYED               = 60,
  SVar_CREATURES_SACRIFICED            = 61, // Total
  SVar_CREATURES_FROM_SACRIFICE        = 62, // Total
  SVar_TIMES_LEVELUP_CREATURE          = 63,
  SVar_TOTAL_SALARY                    = 64,
  SVar_CURRENT_SALARY                  = 65,
  SVar_BOX_ACTIVATED                   = 66,
  SVar_SACRIFICED                      = 67,  // Per model
  SVar_REWARDED                        = 68,  // Per model
  SVar_EVIL_CREATURES_CONVERTED        = 69,
  SVar_GOOD_CREATURES_CONVERTED        = 70,
  SVar_TRAPS_SOLD                      = 71,
  SVar_DOORS_SOLD                      = 72,
  SVar_MANUFACTURED_SOLD               = 73,
  SVar_MANUFACTURE_GOLD                = 74,
  SVar_TOTAL_SCORE                     = 75,
  SVar_BONUS_TIME                      = 76,
 };

extern const struct NamedCommand variable_desc[];
extern const struct NamedCommand dk1_variable_desc[];


long get_condition_value(PlayerNumber plyr_idx, unsigned char valtype, unsigned char a3);
void process_conditions(void);


#ifdef __cplusplus
}
#endif
#endif