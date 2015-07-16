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

struct SlabPathNode;
struct SlabPathContext;

struct SlabPoint
{
	MapSlabCoord x;
	MapSlabCoord y;
};

struct SlabPathEntry
{
	struct SlabPoint position;
	int cost;
};

typedef int (*SlabPathNodeFunc)(struct SlabPoint* parent, struct SlabPoint* pos, void* userdata);

#define SLABPATH_NONODE	INT_MIN
#define SLABPATH_GOAL	(INT_MIN + 1)

struct SlabInfluence
{
	short heart_distance[KEEPER_COUNT];
	short drop_distance[KEEPER_COUNT];
	short dig_distance[KEEPER_COUNT];
	short hero_walk_region;
	short hero_fly_region;
};

struct HeroRegion
{
	int hero_strength;
	int hero_humanoids_strength; //needed separate due to prison
	int neutral_strength;
	int goldpot_gold;
	unsigned char has_heart[KEEPER_COUNT];
	struct SlabPoint any_pos;
	TbBool is_continous_walkable;
	union
	{
		int score;
	} temp; //for singlethreaded algorithms
};

enum PlayerAttitude
{
	Attitude_Friend, //try to help them, treat as ignore otherwise
	Attitude_Avoid, //stay as far away as possible and stall advance
	Attitude_Ignore, //if we meet them, so be it. fight them, but don't actively advance towards dungeon
	Attitude_Aggressive, //actively try to take over their dungeon
	Attitude_SuperAggressive, //actively try to break into their dungeon
};

//slabpath
//these return length of path including start and goal, nodes are returned from goal to start as path_capacity allows
//path_capacity can be zero and in that case path parameter is ignored. a path with capacity 1 is useful for getting goal only
//for a path of arbitrary length, a default attempt can be made on a fixed size path, and reattempted with dynamic array once actual path length is known
//cost for goal node will not be correct as SlabPathNodeFunc terminates through a special return value, caller must fix it
//likewise, if goal is start node, the routines don't guarantee detection of this
int slabpath_dfs(struct SlabPoint* start, struct SlabPathEntry* path, int path_capacity, SlabPathNodeFunc cost, void* userdata, struct SlabPathContext* context);
int slabpath_ucs(struct SlabPoint* start, struct SlabPathEntry* path, int path_capacity, SlabPathNodeFunc isnode, void* userdata, struct SlabPathContext* context);
int slabpath_bfs(struct SlabPoint* start, struct SlabPathEntry* path, int path_capacity, SlabPathNodeFunc cost, void* userdata, struct SlabPathContext* context);

//eval
void update_influence_maps(void);
struct SlabInfluence* get_slab_influence(MapSlabCoord x, MapSlabCoord y);
struct HeroRegion* get_hero_region(int region_index);
int get_hero_region_count(void);
void calc_player_strengths(void);
void update_attitudes(void);
#define get_attitude_towards(player, towards_player) get_attitude_towards_f(player, towards_player, __func__)
enum PlayerAttitude get_attitude_towards_f(int player, int towards_player, const char* func_name);
TbBool is_digging_any_gems(struct Dungeon *dungeon);
struct Thing * find_imp_for_sacrifice(struct Dungeon* dungeon);
struct Thing * find_imp_for_claim(struct Dungeon* dungeon);
struct Thing * find_imp_for_urgent_dig(struct Dungeon* dungeon);
struct Thing * find_creature_for_low_priority_attack(struct Dungeon* dungeon, TbBool strong);
struct Thing * find_any_chicken(struct Dungeon* dungeon);
long get_players_strength(struct Dungeon* dungeon);
int get_preferred_num_room_tiles(struct Dungeon* dungeon, RoomKind rkind);

//checks
TbBool player_has_marked_for_digging(int plyr_idx, MapSlabCoord x, MapSlabCoord y); //ugly outside module exposure of internal to fix multi tile paths
long computer_check_for_door_attacks(struct Computer2 *comp);
long computer_check_for_claims(struct Computer2 *comp);
long computer_check_for_imprison_tendency(struct Computer2* comp);
long computer_check_prison_management(struct Computer2* comp);
void computer_check_doortrap_management(struct Computer2* comp);
long computer_check_new_digging(struct Computer2* comp);
void computer_setup_new_digging(void);
//TODO: clean up long return values, chances are we will not put into old code anyway

TbBool is_newdig_enabled(struct Computer2* comp);
TbBool any_newdig_enabled(void);

#ifdef __cplusplus
}
#endif

#endif //PLAYER_NEWCOMP_H
