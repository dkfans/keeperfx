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
#include "pre_inc.h"
#include "player_states.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "player_data.h"
#include "player_instances.h"
#include "dungeon_data.h"
#include "power_hand.h"
#include "thing_objects.h"
#include "post_inc.h"
#include "config_magic.h"

/******************************************************************************/
/******************************************************************************/
// originally was player_state_to_spell
PowerKind const player_state_to_power_kind[PLAYER_STATES_COUNT] = {
  PwrK_None, // PSt_None
  PwrK_None, // PSt_CtrlDungeon 
  PwrK_None, // PSt_BuildRoom
  PwrK_None, // PSt_MkDigger
  PwrK_None, // PSt_MkGoodCreatr 
  PwrK_None, // PSt_HoldInHand
  PwrK_CALL2ARMS, // PSt_CallToArms 
  PwrK_CAVEIN,  // PSt_CaveIn
  PwrK_SIGHT, // PSt_SightOfEvil 
  PwrK_None, // PSt_Slap
  PwrK_POSSESS, // PSt_CtrlPassngr
  PwrK_POSSESS, // PSt_CtrlDirect
  PwrK_None, // PSt_CreatrQuery  
  PwrK_None, // PSt_OrderCreatr 
  PwrK_None, // PSt_MkBadCreatr 
  PwrK_None, // PSt_CreatrInfo
  PwrK_None, // PSt_PlaceTrap
  PwrK_LIGHTNING, // PSt_Lightning
  PwrK_None,  // PSt_PlaceDoor
  PwrK_SPEEDCRTR, // PSt_SpeedUp
  PwrK_PROTECT, // PSt_Armour
  PwrK_CONCEAL, // PSt_Conceal  
  PwrK_HEALCRTR, // PSt_Heal  
  PwrK_None, // PSt_Sell 
  PwrK_MKDIGGER, // PSt_CreateDigger 
  PwrK_DESTRWALLS, // PSt_DestroyWalls 
  PwrK_DISEASE, // PSt_CastDisease 
  PwrK_CHICKEN, // PSt_TurnChicken  
  PwrK_None, // PSt_MkGoldPot 
  PwrK_TIMEBOMB, // PSt_TimeBomb
  PwrK_DESTRWALLS, // PSt_FreeDestroyWalls
  PwrK_DISEASE, // PSt_FreeCastDisease
  PwrK_CHICKEN, // PSt_FreeTurnChicken
  PwrK_POSSESS, // PSt_FreeCtrlPassngr
  PwrK_POSSESS, // PSt_FreeCtrlDirect
};
/******************************************************************************/

/******************************************************************************/
