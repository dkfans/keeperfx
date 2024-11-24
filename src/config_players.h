/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_players.h
 *     Header file for config_players.c.
 * @par Purpose:
 *     Players configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     17 Sep 2012 - 06 Mar 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGPLAYERS_H
#define DK_CFGPLAYERS_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define PLAYER_STATES_COUNT_MAX    255

struct PlayerStateConfigStats {
    char code_name[COMMAND_WORD_LEN];
    unsigned char pointer_group;
    TbBool stop_own_units;
};

struct PlayerStateConfig {
    struct PlayerStateConfigStats plrst_cfg_stats[PLAYER_STATES_COUNT_MAX];
};

enum PlayerStates {
    PSt_None = 0,
    PSt_CtrlDungeon,
    PSt_BuildRoom,
    PSt_MkDigger,
    PSt_MkGoodCreatr,
    PSt_HoldInHand, // 5
    PSt_CallToArms,
    PSt_CastPowerOnSubtile,
    PSt_SightOfEvil,
    PSt_Slap,
    PSt_CtrlPassngr, // 10
    PSt_CtrlDirect,
    PSt_CreatrQuery,
    PSt_OrderCreatr,
    PSt_MkBadCreatr,
    PSt_CreatrInfo, // 15
    PSt_PlaceTrap,
    PSt_PlaceDoor = 18,
    PST_CastPowerOnTarget,
    PSt_Sell = 23,
    PSt_MkGoldPot = 28,
    PSt_FreeDestroyWalls = 30, // 30
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
    PSt_ListEnd
};

enum PlayerStatePointerGroup {
    PsPg_None,
    PsPg_CtrlDungeon,
    PsPg_BuildRoom,
    PsPg_Invisible,
    PsPg_Spell,
    PsPg_Query,
    PsPg_PlaceTrap,
    PsPg_PlaceDoor,
    PsPg_Sell,
    PsPg_PlaceTerrain,
    PsPg_MkDigger,
    PsPg_MkCreatr,
    PsPg_OrderCreatr
};

/******************************************************************************/
extern struct NamedCommand player_state_commands[];
extern const char keeper_playerstates_file[];

/******************************************************************************/
const char *player_state_code_name(int wrkstate);
struct PlayerStateConfigStats *get_player_state_stats(PlayerState plr_state);
TbBool load_playerstate_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
