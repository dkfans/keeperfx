/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_utils.h
 *     Header file for player_utils.c.
 * @par Purpose:
 *     Player data structures definitions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_UTILS_H
#define DK_PLYR_UTILS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum CreatureWanderingSlots {
    CrWaS_OutsideDungeon = 0,
    CrWaS_WithinDungeon,
};
/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;

#pragma pack()
/******************************************************************************/
/******************************************************************************/
TbBool player_has_won(PlayerNumber plyr_idx);
TbBool player_has_lost(PlayerNumber plyr_idx);
TbBool player_cannot_win(PlayerNumber plyr_idx);
void set_player_as_won_level(struct PlayerInfo *player);
void set_player_as_lost_level(struct PlayerInfo *player);
PlayerNumber get_selected_player_for_cheat(PlayerNumber defplayer);

long compute_player_final_score(struct PlayerInfo *player, long gameplay_score);

#define take_money_from_dungeon(plyr_idx, amount_take, only_whole_sum) take_money_from_dungeon_f(plyr_idx, amount_take, only_whole_sum, __func__)
long take_money_from_dungeon_f(PlayerNumber plyr_idx, GoldAmount amount_take, TbBool only_whole_sum, const char *func_name);
long update_dungeon_generation_speeds(void);
void compute_and_update_player_payday_total(PlayerNumber plyr_idx);
void calculate_dungeon_area_scores(void);

TbBool player_sell_trap_at_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool player_sell_door_at_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

void init_players(void);
void init_player(struct PlayerInfo *player, short no_explore);
void post_init_players(void);
void init_players_local_game(void);
void init_keeper_map_exploration_by_terrain(struct PlayerInfo *player);
void init_keeper_map_exploration_by_creatures(struct PlayerInfo *player);
void process_players(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
