/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_magic_data.cpp
 *     Keeper and creature spells configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for magic spells.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_magic.h"
#include "globals.h"

#include "bflib_basics.h"
#include "power_process.h"
#include "player_instances.h"
#include "player_states.h"
#include "packets.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct SpellData spell_data[] = {
  {PckA_SetPlyrState,  PSt_CtrlDirect, 0,   0,   0,   0,   0,   0,  0, NULL,                 PwCast_None},      //[0] None
  {        PckA_None,        PSt_None, 0,   0,   0,   0,   0,   0,  0, NULL,                 PwCast_Anywhere}, //[1] Power Hand
  {PckA_SetPlyrState,PSt_CreateDigger, 0,  95, 118, 631, 648, 831,  5, NULL,                 PwCast_Anywhere}, //[2] Make Digger
  {  PckA_UsePwrObey,        PSt_None, 0, 394, 452, 636, 653, 834,  0, NULL,                 PwCast_Anywhere}, //[3] Must Obey
  {        PckA_None,        PSt_None, 0,   0,   0,   0,   0,   0,  0, NULL,                 PwCast_Anywhere}, //[4] Slap
  {PckA_SetPlyrState, PSt_SightOfEvil, 1,  85, 108, 632, 649, 828, 12, sight_of_evil_expand_check, PwCast_Anywhere}, //[5] Sight of Evil
  {PckA_SetPlyrState,  PSt_CallToArms, 1,  93, 116, 633, 650, 826,  0, call_to_arms_expand_check,  PwCast_Anywhere}, //[6] Call To Arms
  {PckA_SetPlyrState,      PSt_CaveIn, 1,  97, 120, 635, 652, 837, 10, general_expand_check, PwCast_Anywhere}, //[7] Cave in
  {PckA_SetPlyrState,        PSt_Heal, 0,  87, 110, 644, 661, 829,  8, general_expand_check, PwCast_Anywhere}, //[8] Heal Creature
  {PckA_HoldAudience,        PSt_None, 0,  89, 112, 634, 651, 830,  0, general_expand_check, PwCast_Anywhere}, //[9] Hold Audience
  {PckA_SetPlyrState,   PSt_Lightning, 0, 101, 124, 640, 657, 833,  6, general_expand_check, PwCast_Anywhere}, //[10] Lightning
  {PckA_SetPlyrState,     PSt_SpeedUp, 0,  99, 122, 637, 654, 838, 11, general_expand_check, PwCast_Anywhere}, //[11] Speed Creature
  {PckA_SetPlyrState,      PSt_Armour, 0, 103, 126, 638, 655, 825,  9, general_expand_check, PwCast_Anywhere}, //[12] Protect
  {PckA_SetPlyrState,     PSt_Conceal, 0, 105, 128, 639, 656, 832,  1, general_expand_check, PwCast_Anywhere}, //[13] Conceal
  {PckA_SetPlyrState, PSt_CastDisease, 0, 310, 319, 642, 659, 835,  3, general_expand_check, PwCast_Anywhere}, //[14] Disease
  {PckA_SetPlyrState, PSt_TurnChicken, 0, 306, 314, 641, 658, 827,  2, general_expand_check, PwCast_Anywhere}, //[15] Chicken
  {PckA_SetPlyrState,PSt_DestroyWalls, 0, 308, 317, 643, 660, 839,  4, general_expand_check, PwCast_Anywhere}, //[16] Destroy Walls
  {PckA_SetPlyrState,    PSt_TimeBomb, 0, 105, 128, 645, 662,   0,  0, NULL,                 PwCast_Anywhere}, //[17] Time Bomb
  {PckA_SetPlyrState,  PSt_CtrlDirect, 0,  91, 114, 630, 647, 836,  7, NULL,                 PwCast_Anywhere}, //[18] Possession
  { PckA_UsePwrArmageddon,   PSt_None, 0, 312, 321, 646, 663, 824,  0, NULL,                 PwCast_Anywhere}, //[19] Armageddon
  {        PckA_None,        PSt_None, 0,   0,   0,   0,   0,   0,  0, NULL,                 PwCast_None},      //[20]
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
