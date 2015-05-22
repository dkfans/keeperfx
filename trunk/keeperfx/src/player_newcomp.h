/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_newcomp.h
 *     New computer player header.
 * @par Purpose:
 *     Cleanly separates computer player code that wasn't in the original game.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 02 May 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef PLAYER_NEWCOMP_H
#define PLAYER_NEWCOMP_H

#include "bflib_basics.h"

#include "dungeon_data.h"
#include "player_computer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SlabInfluence
{
	short heart_distance[KEEPER_COUNT];
	short drop_distance[KEEPER_COUNT];
	short dig_distance[KEEPER_COUNT];
};

enum PlayerAttitude
{
	Attitude_Friend, //try to help them, treat as ignore otherwise
	Attitude_Avoid, //stay as far away as possible and stall advance
	Attitude_Ignore, //if we meet them, so be it. fight them, but don't actively advance towards dungeon
	Attitude_Aggressive, //actively try to take over their dungeon
	Attitude_SuperAggressive, //actively try to break into their dungeon
};

//eval
void update_influence_maps(void);
struct SlabInfluence* get_slab_influence(MapSlabCoord x, MapSlabCoord y);
void update_attitudes(void);
#define get_attitude_towards(player, towards_player) get_attitude_towards_f(player, towards_player, __func__)
enum PlayerAttitude get_attitude_towards_f(int player, int towards_player, const char* func_name);
TbBool is_digging_any_gems(struct Dungeon *dungeon);
struct Thing * find_imp_for_sacrifice(struct Dungeon* dungeon);
struct Thing * find_imp_for_claim(struct Dungeon* dungeon);
struct Thing * find_imp_for_urgent_dig(struct Dungeon* dungeon);
struct Thing * find_creature_for_low_priority_attack(struct Dungeon* dungeon, TbBool strong);
struct Thing * find_any_chicken(struct Dungeon* dungeon);
long calc_players_strength(struct Dungeon* dungeon);
int get_preferred_num_room_tiles(struct Dungeon* dungeon, RoomKind rkind);

//checks
long computer_check_for_door_attacks(struct Computer2 *comp);
long computer_check_for_claims(struct Computer2 *comp);
long computer_check_for_imprison_tendency(struct Computer2* comp);
long computer_check_prison_management(struct Computer2* comp);
long computer_check_new_digging(struct Computer2* comp);
void computer_setup_new_digging(void);

extern TbBool newdig; //is new digging enabled?

#ifdef __cplusplus
}
#endif

#endif //PLAYER_NEWCOMP_H
