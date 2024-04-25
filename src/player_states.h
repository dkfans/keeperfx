/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_states.h
 *     Header file for player_states.c.
 * @par Purpose:
 *     Player states definitions and functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Nov 2012 - 02 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_STATES_H
#define DK_PLYR_STATES_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PLAYER_STATES_COUNT    PSt_ListEnd
/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;

enum PlayerStates {
    PSt_None = 0,
    PSt_CtrlDungeon,
    PSt_BuildRoom,
    PSt_MkDigger,
    PSt_MkGoodCreatr,
    PSt_HoldInHand, // 5
    PSt_CallToArms,
    PSt_CaveIn,
    PSt_SightOfEvil,
    PSt_Slap,
    PSt_CtrlPassngr, // 10
    PSt_CtrlDirect,
    PSt_CreatrQuery,
    PSt_OrderCreatr,
    PSt_MkBadCreatr,
    PSt_CreatrInfo, // 15
    PSt_PlaceTrap,
    PSt_Lightning,
    PSt_PlaceDoor,
    PSt_SpeedUp,
    PSt_Armour, // 20
    PSt_Conceal,
    PSt_Heal,
    PSt_Sell,
    PSt_CreateDigger,
    PSt_DestroyWalls, // 25
    PSt_CastDisease,
    PSt_TurnChicken,
    PSt_MkGoldPot,
    PSt_TimeBomb,
    PSt_FreeDestroyWalls, // 30
    PSt_FreeCastDisease,
    PSt_FreeTurnChicken,
    PSt_FreeCtrlPassngr,
    PSt_FreeCtrlDirect,
    PSt_StealRoom, // 35
    PSt_DestroyRoom,
    PSt_KillCreatr,
    PSt_ConvertCreatr,
    PSt_StealSlab,
    PSt_LevelCreatureUp, // 40
    PSt_LevelCreatureDown,
    PSt_KillPlayer,
    PSt_HeartHealth,
    PSt_QueryAll,
    PSt_MkHappy, // 45
    PSt_MkAngry,
    PSt_PlaceTerrain,
    PSt_DestroyThing,
    PSt_CreatrInfoAll,
    PSt_Rebound, // 50
    PSt_Freeze,
    PSt_Slow,
    PSt_Flight,
    PSt_Vision,
    PSt_ListEnd
};

#pragma pack()
/******************************************************************************/
extern unsigned short const player_state_to_power_kind[];
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
