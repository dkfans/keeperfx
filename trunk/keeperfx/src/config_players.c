/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_players.c
 *     Players configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for players.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     17 Sep 2012 - 06 Mar 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_players.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"

#include "config.h"
#include "player_states.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const struct NamedCommand player_state_commands[] = {
    {"PLAYER_STATE_NONE",             PSt_None},
    {"PLAYER_STATE_CTRLDUNGEON",      PSt_CtrlDungeon},
    {"PLAYER_STATE_BUILDROOM",        PSt_BuildRoom},
    {"PLAYER_STATE_MKGOODDIGGER",     PSt_MkGoodDigger},
    {"PLAYER_STATE_MKGOODCREATR",     PSt_MkGoodCreatr},
    {"PLAYER_STATE_HOLDINHAND",       PSt_HoldInHand},
    {"PLAYER_STATE_CALLTOARMS",       PSt_CallToArms},
    {"PLAYER_STATE_CAVEIN",           PSt_CaveIn},
    {"PLAYER_STATE_SIGHTOFEVIL",      PSt_SightOfEvil},
    {"PLAYER_STATE_SLAP",             PSt_Slap},
    {"PLAYER_STATE_CTRLPASSNGR",      PSt_CtrlPassngr},
    {"PLAYER_STATE_CTRLDIRECT",       PSt_Possession},
    {"PLAYER_STATE_CREATRQUERY",      PSt_CreatrQuery},
    {"PLAYER_STATE_ORDERCREATR",      PSt_OrderCreatr},
    {"PLAYER_STATE_MKBADCREATR",      PSt_MkBadCreatr},
    {"PLAYER_STATE_CREATRINFO",       PSt_CreatrInfo},
    {"PLAYER_STATE_PLACETRAP",        PSt_PlaceTrap},
    {"PLAYER_STATE_LIGHTNING",        PSt_Lightning},
    {"PLAYER_STATE_PLACEDOOR",        PSt_PlaceDoor},
    {"PLAYER_STATE_SPEEDUP",          PSt_SpeedUp},
    {"PLAYER_STATE_ARMOUR",           PSt_Armour},
    {"PLAYER_STATE_CONCEAL",          PSt_Conceal},
    {"PLAYER_STATE_HEAL",             PSt_Heal},
    {"PLAYER_STATE_SELL",             PSt_Sell},
    {"PLAYER_STATE_CREATEDIGGER",     PSt_CreateDigger},
    {"PLAYER_STATE_DESTROYWALLS",     PSt_DestroyWalls},
    {"PLAYER_STATE_CASTDISEASE",      PSt_CastDisease},
    {"PLAYER_STATE_TURNCHICKEN",      PSt_TurnChicken},
    {"PLAYER_STATE_MKGOLDPOT",        PSt_MkGoldPot},
    {"PLAYER_STATE_TIMEBOMB",         PSt_TimeBomb},
    {"PLAYER_STATE_FREEDESTROYWALLS", PSt_FreeDestroyWalls},
    {"PLAYER_STATE_FREECASTDISEASE",  PSt_FreeCastDisease},
    {"PLAYER_STATE_FREETURNCHICKEN",  PSt_FreeTurnChicken},
    {"PLAYER_STATE_FREECTRLPASSNGR",  PSt_FreeCtrlPassngr},
    {"PLAYER_STATE_FREECTRLDIRECT",   PSt_FreePossession},
    {NULL,                            0},
};
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
#ifdef __cplusplus
}
#endif
