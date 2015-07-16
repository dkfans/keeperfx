/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_newcompchecks.c
 *     New computer player digging.
 * @par Purpose:
 *     Cleanly separates computer player checks that weren't in the original
 *     game.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 15 Jul 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "player_newcomp.h"

#include "bflib_math.h"

#include "config_terrain.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "creature_states_lair.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "globals.h"
#include "magic.h"
#include "map_data.h"
#include "player_computer.h"
#include "power_hand.h"
#include "room_data.h"
#include "slab_data.h"
#include "spdigger_stack.h"
#include "thing_list.h"
#include "thing_stats.h"

extern MapSlabCoord map_tiles_x;
extern MapSlabCoord map_tiles_y;

struct ExpandRoomPos
{
	MapSlabCoord min_x, min_y;
	MapSlabCoord max_x, max_y;
	MapSlabCoord access_x, access_y;
	MapSlabCoord access_dx, access_dy;
};

struct ExpandRoom //expand room (digging prior if necessary)
{
	TbBool active;
	TbBool for_dig_gold;
	RoomKind rkind;
	int preferred_size;
	TbBool predug; //temporary in algorithm, not valid for final output
	struct ExpandRoomPos room_pos;
	int room_score;
	int drop_count;
};

struct DigToGold
{
	TbBool active;
	MapSlabCoord target_x, target_y;
	MapSlabCoord access_x, access_y;
};

struct DigToAttack
{
	struct SlabPoint entry_pos;
	TbBool active;
};

struct DigToSecure
{
	TbBool active;
};

#define CONCURRENT_GOLD_DIGS		10
#define TREASURE_ROOM_SEARCH_RADIUS	9
#define TREASURE_ROOM_SEARCH_TRACK	2

struct BridgeNode
{
	struct BridgeNode* next;
	MapSlabCoord x, y;
	//might need more meta info here later on
};

struct Digging
{
	struct ExpandRoom expand_room;
	struct DigToGold dig_gold[CONCURRENT_GOLD_DIGS];
	struct DigToAttack dig_attack;
	struct DigToSecure dig_secure;
	unsigned char marked_for_dig[85][85]; //need instant lookup, so maintaining additional struct. [y][x] in case we move to ptr later
	struct BridgeNode* bridge_list;
	long hatchery_built_turn;
};

static struct Digging comp_digging[KEEPER_COUNT];
static unsigned char danger_map[85][85]; //similar as marked_for_dig

static void initiate_expand_room(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand);
static int eval_expand_room(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand, TbBool allow_move, int upscale, int downscale, MapSlabCoord x, MapSlabCoord y, MapSlabCoord dx, MapSlabCoord dy);

static TbBool is_dangerous_digging(MapSlabCoord x, MapSubtlCoord y)
{
	struct SlabMap* slab;
	slab = get_slabmap_block(x, y);
	if (SlbT_GEMS != slab->kind)
		return danger_map[y][x];
	return 0;
}

static TbBool is_marked_for_digging(struct Digging* digging, MapSlabCoord x, MapSlabCoord y)
{
	return digging->marked_for_dig[y][x];
}

TbBool player_has_marked_for_digging(int plyr_idx, MapSlabCoord x, MapSlabCoord y)
{
	//TODO: I don't like exposing this outside of the module, but it's only quick option to fix multi tile wide paths right now

	if (plyr_idx < 0 || plyr_idx >= KEEPER_COUNT)
	{
		ERRORLOG("Access of invalid keeper %d", plyr_idx);
		return false;
	}

	return is_marked_for_digging(&comp_digging[plyr_idx], x, y);
}

static void debug_digging_map(const char* filename, struct Dungeon* dungeon, struct Digging* digging)
{
	MapSlabCoord x, y;
	FILE* file;
	struct SlabMap* slab;

	if (NULL == filename)
		return; //used to avoid unused function warning

	file = fopen(filename, "w");
	if (NULL == file)
	{
		ERRORLOG("Unable to open %s for debug printing digging map", filename);
		return;
	}

	for (y = 0; y < map_tiles_y; ++y)
	{
		for (x = 0; x < map_tiles_x; ++x)
		{
			char c;
			slab = get_slabmap_block(x, y);

			if (is_marked_for_digging(digging, x, y))
				c = slab_kind_is_liquid(slab->kind)? 'x' : 'X';
			else
			{
				switch (slab->kind)
				{
				case SlbT_LAVA:
					c = 'L';
					break;
				case SlbT_WATER:
					c = 'W';
					break;
				case SlbT_WALLDRAPE:
				case SlbT_WALLPAIRSHR:
				case SlbT_WALLTORCH:
				case SlbT_WALLWTWINS:
				case SlbT_WALLWWOMAN:
					c = '=';
					break;
				case SlbT_EARTH:
					c = '.';
					break;
				case SlbT_PATH:
					c = ' ';
					break;
				case SlbT_CLAIMED:
					c = '#';
					break;
				case SlbT_ROCK:
					c = 'O';
					break;
				case SlbT_GOLD:
					c = 'G';
					break;
				case SlbT_GEMS:
					c = 'g';
					break;
				default:
					if (slab_kind_can_drop_here_now(slab->kind))
						c = '~';
					else
						c = 'Z';
					break;
				}
			}

			fputc(c, file);
		}
		fputc('\n', file);
	}

	fclose(file);
}

static TbBool is_marked_for_digging_and_undug(struct Digging* digging, MapSlabCoord x, MapSlabCoord y)
{
	struct SlabMap* slab;

	if (x < 0 || y < 0 || x >= map_tiles_x || y >= map_tiles_y)
		return 0;
	if (!is_marked_for_digging(digging, x, y))
		return 0;

	slab = get_slabmap_block(x, y);
	switch (slab->kind)
	{
	case SlbT_EARTH:
	case SlbT_GOLD:
	case SlbT_GEMS:
	case SlbT_TORCHDIRT:
	case SlbT_WALLDRAPE:
	case SlbT_WALLPAIRSHR:
	case SlbT_WALLTORCH:
	case SlbT_WALLWTWINS:
	case SlbT_WALLWWOMAN:
	case SlbT_LAVA:
	case SlbT_WATER:
		return 1;
	default:
		return 0;
	}
}

void computer_setup_new_digging(void)
{
	struct Digging* digging;
	struct BridgeNode* next;
	int plyr_idx;

	//free memory in case it was used earlier
	for (plyr_idx = 0; plyr_idx < KEEPER_COUNT; ++plyr_idx)
	{
		digging = &comp_digging[plyr_idx];
		
		while (digging->bridge_list)
		{
			next = digging->bridge_list->next;
			free(digging->bridge_list);
			digging->bridge_list = next;
		}
	}

	//after this everything will be reset
	memset(&comp_digging, 0, sizeof(comp_digging));
	
	//init new player state
	for (plyr_idx = 0; plyr_idx < KEEPER_COUNT; ++plyr_idx)
	{
		struct Dungeon *dungeon;
		struct MapTask *mtask;
		long i,max;
		digging = &comp_digging[plyr_idx];

		//misc
		digging->hatchery_built_turn = LONG_MIN / 2;

		//rebuild digging map from gamestate
		dungeon = get_dungeon(plyr_idx);
		max = dungeon->field_AF7;
		if (max > MAPTASKS_COUNT)
			max = MAPTASKS_COUNT;

		//mark digging in process
		for (i = 0; i < max; i++)
		{
			mtask = &dungeon->task_list[i];
			if (mtask->kind != SDDigTask_Unknown3)
			{
				MapSlabCoord x, y;
				x = subtile_slab(stl_num_decode_x(mtask->coords));
				y = subtile_slab(stl_num_decode_y(mtask->coords));
				digging->marked_for_dig[y][x] = 1;
			}
		}
	}
}


static TbBool is_diggable_or_buildable(struct Dungeon* dungeon, struct Digging* digging, MapSlabCoord x, MapSubtlCoord y)
{
	struct SlabMap* slab;
	slab = get_slabmap_block(x, y);
	switch (slab->kind)
	{
	case SlbT_PATH:
	case SlbT_TORCHDIRT:
	case SlbT_EARTH:
	case SlbT_GOLD:
		return 1;
	case SlbT_WALLDRAPE:
	case SlbT_WALLPAIRSHR:
	case SlbT_WALLTORCH:
	case SlbT_WALLWTWINS:
	case SlbT_WALLWWOMAN:
	case SlbT_CLAIMED:
		return slabmap_owner(slab) == dungeon->owner;
	default:
		return 0;
	}
	//TODO: might wish to include replacing existing rooms, however that must in that case give negative score in other parts of algorithm
}

static TbResult bridge_later_if_needed(struct Computer2* comp, struct Digging* digging, MapSlabCoord x, MapSlabCoord y)
{
	int player;
	struct SlabMap* slab;
	player = comp->dungeon->owner;

	if (is_marked_for_digging(digging, x, y))
		return Lb_OK;

	if (!is_room_available(player, RoK_BRIDGE))
		return Lb_FAIL;

	slab = get_slabmap_block(x, y);
	if (slab->kind == SlbT_WATER || slab->kind == SlbT_LAVA)
	{
		struct BridgeNode* bridge;
		digging->marked_for_dig[y][x] = 1;
		bridge = (struct BridgeNode*)calloc(1, sizeof(*bridge));
		bridge->next = digging->bridge_list;
		bridge->x = x;
		bridge->y = y;
		digging->bridge_list = bridge;
		return Lb_SUCCESS;
	}

	return Lb_OK;
}

static TbResult dig_if_needed(struct Computer2* comp, struct Digging* digging, MapSlabCoord x, MapSlabCoord y)
{
	int player;
	struct SlabMap* slab;
	TbResult result;
	player = comp->dungeon->owner;

	if (is_marked_for_digging(digging, x, y))
		return Lb_OK;

	slab = get_slabmap_block(x, y);
	switch (slab->kind)
	{
	case SlbT_LAVA:
	case SlbT_WATER:
		return bridge_later_if_needed(comp, digging, x, y);
	case SlbT_EARTH:
	case SlbT_GOLD:
	case SlbT_GEMS:
	case SlbT_TORCHDIRT:
	case SlbT_WALLDRAPE:
	case SlbT_WALLPAIRSHR:
	case SlbT_WALLTORCH:
	case SlbT_WALLWTWINS:
	case SlbT_WALLWWOMAN:
		result = try_game_action(comp, player, GA_MarkDig, 0,
			x * STL_PER_SLB + 1, y * STL_PER_SLB + 1, 1, 1);
		if (result != Lb_FAIL)
			digging->marked_for_dig[y][x] = 1;
		return result;
	default:
		return Lb_OK;
	}
}

static void try_drop_imp_to_reveal(struct Computer2* comp, TbBool* do_build, MapSlabCoord x, MapSlabCoord y)
{
	MapSubtlCoord stl_x, stl_y;
	if (!*do_build)
		return;

	stl_x = slab_subtile_center(x);
	stl_y = slab_subtile_center(y);
	if (can_drop_thing_here(stl_x, stl_y, comp->dungeon->owner, 1))
	{
		struct Thing* thing;
		thing = find_imp_for_urgent_dig(comp->dungeon);
		if (!thing_is_invalid(thing))
		{
			SYNCDBG(9, "Dropping imp to reveal");
			*do_build = 0;
			create_task_move_creature_to_subtile(comp, thing, stl_x, stl_y, CrSt_ImpDigsDirt);
		}
		else
		{
			SYNCDBG(9, "Found no imp to reveal");
		}
	}
}

static void process_bridging(struct Computer2* comp, struct Digging* digging)
{
	struct BridgeNode* bridge;
	struct BridgeNode* next;
	struct BridgeNode** prev;
	struct SlabMap* slab;
	struct Dungeon* dungeon;
	int player;
	TbResult result;
	TbBool do_build;
	struct RoomStats* rstats;
	int count;
	
	SYNCDBG(8, "Starting");
	dungeon = comp->dungeon;
	player = dungeon->owner;
	rstats = room_stats_get_for_kind(RoK_BRIDGE);
	do_build = is_room_available(player, RoK_BRIDGE) && dungeon->total_money_owned >= rstats->cost;
	
	count = 0;
	prev = &digging->bridge_list;
	bridge = digging->bridge_list;
	while (bridge)
	{
		MapSubtlCoord stl_x, stl_y;

		++count;
		next = bridge->next;
		result = Lb_OK;
		stl_x = slab_subtile_center(bridge->x);
		stl_y = slab_subtile_center(bridge->y);
		slab = get_slabmap_block(bridge->x, bridge->y);
		if (slab->kind == SlbT_BRIDGE)
		{
			SYNCLOG("bridge");
			if (slabmap_owner(slab) == player)
			{
				result = Lb_SUCCESS;
			}
			else
			{
				result = Lb_FAIL;
			}
		}
		else if (do_build && !subtile_revealed(stl_x, stl_y, player))
		{
			try_drop_imp_to_reveal(comp, &do_build, bridge->x - 1, bridge->y);
			try_drop_imp_to_reveal(comp, &do_build, bridge->x + 1, bridge->y);
			try_drop_imp_to_reveal(comp, &do_build, bridge->x, bridge->y - 1);
			try_drop_imp_to_reveal(comp, &do_build, bridge->x, bridge->y + 1);
		}
		else if (do_build && slab_by_players_land(player, bridge->x, bridge->y))
		{
			SYNCLOG("try to build");
			result = try_game_action(comp, player, GA_PlaceRoom, 0, stl_x, stl_y, 1, RoK_BRIDGE);
			do_build = 0;
		}

		if (result != Lb_OK)
		{
			//remove node
			*prev = next;
			free(bridge);
			bridge = NULL;
		}

		if (bridge)
			prev = &bridge->next;

		bridge = next;
	}

	if (count > 0)
		SYNCLOG("Processed %d bridge nodes", count);
	//debug_digging_map("process_bridging.txt", dungeon, digging);
}

struct FindBestEntryToHeroRegion
{
	int player;
	int region_index;
	int best_score;
	struct SlabPoint best_pos;
};

static void try_get_closer(int player, TbBool on_dug, int* best_dist, MapSlabCoord* best_x, MapSlabCoord* best_y, MapSlabCoord x, MapSlabCoord y)
{
	int dist;
	struct SlabInfluence* influence;
	influence = get_slab_influence(x, y);
	dist = on_dug? influence->heart_distance[player] : influence->dig_distance[player];
	if (dist >= 0 && dist < *best_dist)
	{
		*best_dist = dist;
		*best_x = x;
		*best_y = y;
	}
}

static TbBool backtrack_dig_path(int x, int y, struct Computer2* comp, struct Digging* digging, TbBool fake_run, TbBool check_target)
{
	struct SlabInfluence* influence;
	struct Dungeon* dungeon = comp->dungeon;
	TbBool abort = 0;

	for (;;)
	{
		int best_dist;
		int current_dist;
		MapSlabCoord best_x, best_y;

		if (check_target && (is_dangerous_digging(x, y) || !is_diggable_or_buildable(dungeon, digging, x, y) ||
			(!fake_run && !is_marked_for_digging(digging, x, y) && dig_if_needed(comp, digging, x, y) == Lb_FAIL)))
		{
			abort = 1;
			break;
		}

		influence = get_slab_influence(x, y);
		current_dist = influence->dig_distance[dungeon->owner];
		if (current_dist <= 0)
		{
			if (current_dist < 0)
			{
				abort = 1;
			}
			break;
		}

		best_dist = current_dist;
		best_x = x;
		best_y = y;
		try_get_closer(dungeon->owner, false, &best_dist, &best_x, &best_y, x - 1, y);
		try_get_closer(dungeon->owner, false, &best_dist, &best_x, &best_y, x + 1, y);
		try_get_closer(dungeon->owner, false, &best_dist, &best_x, &best_y, x, y - 1);
		try_get_closer(dungeon->owner, false, &best_dist, &best_x, &best_y, x, y + 1);
		check_target = 1; //basically means check everything remaining (but the target which has at this point been checked if necessary)

		if (best_dist < current_dist)
		{
			x = best_x;
			y = best_y;
		}
		else
		{
			abort = 1;
			break;
		}
	}

	return !abort;
}

static int find_best_entry_to_hero_region(struct SlabPoint* parent, struct SlabPoint* pos, void* data)
{
	MapSlabCoord x, y;
	int score;
	struct Thing* heart;
	struct FindBestEntryToHeroRegion* state = (struct FindBestEntryToHeroRegion*)data;
	struct SlabInfluence* influence = get_slab_influence(pos->x, pos->y);

	if (influence->hero_walk_region != state->region_index && influence->hero_fly_region != state->region_index)
		return SLABPATH_NONODE;

	if (is_dangerous_digging(pos->x, pos->y))
		return SLABPATH_NONODE; //whole region is likely dangerous if just one tile is

	score = -influence->dig_distance[state->player];
	heart = get_player_soul_container(state->player);
	if (!thing_is_invalid(heart))
	{
		x = subtile_slab(heart->mappos.x.stl.num);
		y = subtile_slab(heart->mappos.y.stl.num);
		score += abs(x - pos->x) + abs(y - pos->y); //TODO: could use influence distance but heart distance is just indoors atm
	}

	if (score > state->best_score)
	{
		state->best_score = score;
		state->best_pos.x = pos->x;
		state->best_pos.y = pos->y;
	}

	return 1;
}

static void process_dig_to_attack(struct Computer2* comp, struct Digging* digging)
{
	struct Dungeon* dungeon;
	struct SlabInfluence* influence;
	dungeon = comp->dungeon;
	SYNCDBG(9, "Starting");

	influence = get_slab_influence(digging->dig_attack.entry_pos.x, digging->dig_attack.entry_pos.y);
	if (influence->unblocked_drop_distance[dungeon->owner] == 0 || influence->unblocked_drop_distance[dungeon->owner] == 1)
	{
		SYNCLOG("Player %d finished dig for attack at %dx%d", (int)dungeon->owner,
			digging->dig_attack.entry_pos.x, digging->dig_attack.entry_pos.y);
		digging->dig_attack.active = 0;
	}
}

static void initiate_dig_attack(struct Computer2* comp, struct Digging* digging, struct SlabPoint* entry_pos)
{
	struct Dungeon* dungeon = comp->dungeon;
	TbBool aborted;
	
	aborted = !backtrack_dig_path(entry_pos->x, entry_pos->y, comp, digging, false, false);

	if (aborted)
	{
		SYNCLOG("Player %d aborted attack dig attempt at %dx%d", (int)dungeon->owner, entry_pos->x, entry_pos->y);
	}
	else
	{
		SYNCLOG("Player %d decided to dig for attack at %dx%d", (int)dungeon->owner, entry_pos->x, entry_pos->y);
		digging->dig_attack.active = 1;
		digging->dig_attack.entry_pos.x = entry_pos->x;
		digging->dig_attack.entry_pos.y = entry_pos->y;
	}
}

static void check_dig_to_attack(struct Computer2* comp, struct Digging* digging)
{
	int i, j;
	int best_score, score;
	struct SlabPoint entry_pos;
	struct Dungeon* dungeon;
	struct FindBestEntryToHeroRegion state;
	struct SlabInfluence* influence;
	TbBool has_heart;

	SYNCDBG(9, "Starting");
	dungeon = comp->dungeon;

	//find best target
	best_score = INT_MIN;
	for (i = 0; i < get_hero_region_count(); ++i)
	{
		struct HeroRegion* region = get_hero_region(i);
		if (!region->is_continous_walkable && !is_room_available(dungeon->owner, RoK_BRIDGE))
			continue;

		has_heart = false;
		for (j = 0; j < KEEPER_COUNT; ++j)
		{
			if (region->has_heart[j])
			{
				has_heart = true;
				break;
			}
		}
		if (has_heart) continue;

		//analyze region's tiles to find best entry tile
		state.player = dungeon->owner;
		state.region_index = i;
		state.best_score = INT_MIN;
		slabpath_dfs(&region->any_pos, NULL, 0, find_best_entry_to_hero_region, &state, NULL);

		if (INT_MIN == state.best_score)
			continue; //no entry found

		influence = get_slab_influence(state.best_pos.x, state.best_pos.y);
		if (influence->unblocked_drop_distance[dungeon->owner] == 0 || influence->unblocked_drop_distance[dungeon->owner] == 1)
			continue; //this should always be the case (no way to drop creatures near this), but checked anyway since it's a termination condition

		score = state.best_score;

		if (score > best_score)
		{
			//SYNCLOG("looking for path");
			if (backtrack_dig_path(state.best_pos.x, state.best_pos.y, comp, digging, true, false))
			{
				//SYNCLOG("found path");
				best_score = score;
				entry_pos.x = state.best_pos.x;
				entry_pos.y = state.best_pos.y;
			}
		}
	}

	if (INT_MIN != best_score)
	{
		initiate_dig_attack(comp, digging, &entry_pos);
	}
}

static void process_dig_to_secure(struct Computer2* comp, struct Digging* digging)
{

}

static void check_dig_to_secure(struct Computer2* comp, struct Digging* digging)
{

}

static void process_dig_to_gold(struct Computer2* comp, struct Digging* digging, struct DigToGold* dig_gold)
{
	struct Dungeon* dungeon;
	struct SlabMap* slab;
	TbBool finished;

	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;
	finished = 0;
	
	slab = get_slabmap_block(dig_gold->target_x, dig_gold->target_y);
	if (slab->kind == SlbT_GEMS) //TODO: need to track gems differently once processing has ended
		;//finished = 1; //DO NOTHING DELIBERATELY ATM, either need to monitor number of gem digs active vs gold or do it outside this process
	else if (slab->kind != SlbT_GOLD)
		finished = 1;

	if (!finished)
	{
		struct SlabInfluence* influence;
		influence = get_slab_influence(dig_gold->target_x, dig_gold->target_y);
		if (influence->dig_distance[dungeon->owner] < 0)
			finished = 1; //TODO: abort
	}
	
	if (finished)
	{
		dig_gold->active = 0;
	}
}

static void initiate_dig_gold(struct Computer2* comp, struct Digging* digging, struct DigToGold* dig_gold)
{
	MapSlabCoord x, y;
	struct Dungeon* dungeon;
	struct SlabInfluence* influence;
	struct SlabMap* slab;
	TbBool abort;
	dungeon = comp->dungeon;
	TbBool wants_treasure_room;
	int treasure_search_steps;
	TbBool on_dug;

	wants_treasure_room = !digging->expand_room.active; //check if we have treasure room somewhere in vicinity
	if (wants_treasure_room)
	{
		int num_gold_tiles, num_gem_tiles;
		num_gold_tiles = num_gem_tiles = 0;
		//TODO: use BFS instead of consider accessibility of treasure room. see if it's worth building a new path
		for (y = dig_gold->target_y - TREASURE_ROOM_SEARCH_RADIUS; y <= dig_gold->target_y + TREASURE_ROOM_SEARCH_RADIUS; ++y)
		{
			for (x = dig_gold->target_x - TREASURE_ROOM_SEARCH_RADIUS; x <= dig_gold->target_x + TREASURE_ROOM_SEARCH_RADIUS; ++x)
			{
				if (abs(x - dig_gold->target_x) + abs(y - dig_gold->target_y) > TREASURE_ROOM_SEARCH_RADIUS)
					continue;
				if (is_dangerous_digging(x, y))
					continue;

				slab = get_slabmap_block(x, y);
				if (SlbT_GOLD == slab->kind) //TODO: count accessible faces instead
				{
					num_gold_tiles += 1;
				}
				else if (SlbT_GEMS == slab->kind)
				{
					num_gem_tiles += 1;
				}
				if (SlbT_TREASURE == slab->kind && slabmap_owner(slab) == dungeon->owner)
				{
					wants_treasure_room = 0;
					break;
				}
			}
		}

		if (num_gem_tiles == 0 && num_gold_tiles < 5)
			wants_treasure_room = 0;

		if (wants_treasure_room)
		{
			digging->expand_room.rkind = RoK_TREASURE;
			digging->expand_room.room_score = INT_MIN;
			digging->expand_room.preferred_size = min(36, max(12, num_gold_tiles / 2 + 8 * num_gem_tiles));
		}
	}

	//back track to own territory //TODO: use A* instead so that we can e.g. prefer not to wreck our walls, also this has gotten complicated due to mixing two different decisions (path and room build)
	x = dig_gold->target_x;
	y = dig_gold->target_y;
	treasure_search_steps = 0;
	abort = 0;
	on_dug = 0;
	for (;;)
	{
		int best_dist;
		int current_dist;
		MapSlabCoord best_x, best_y;

		if (!on_dug && (is_dangerous_digging(x, y) || (!is_marked_for_digging(digging, x, y) && dig_if_needed(comp, digging, x, y) == Lb_FAIL)))
		{
			abort = 1;
			break;
		}

		if (wants_treasure_room && treasure_search_steps < TREASURE_ROOM_SEARCH_TRACK)
		{
			treasure_search_steps += 1;
			eval_expand_room(comp, digging, &digging->expand_room, 0, 1, 1, x, y, -1, 0);
			eval_expand_room(comp, digging, &digging->expand_room, 0, 1, 1, x, y, 1, 0);
			eval_expand_room(comp, digging, &digging->expand_room, 0, 1, 1, x, y, 0, -1);
			eval_expand_room(comp, digging, &digging->expand_room, 0, 1, 1, x, y, 0, 1);
		}
		else if (on_dug)
			break;

		influence = get_slab_influence(x, y);
		current_dist = on_dug? influence->heart_distance[dungeon->owner] : influence->dig_distance[dungeon->owner];
		if (current_dist <= 0)
		{
			if (!on_dug && current_dist < 0)
			{
				abort = 1;
				break;
			}
			if (on_dug || !wants_treasure_room)
				break;
			on_dug = 1;
		}

		best_dist = current_dist;
		best_x = x;
		best_y = y;
		try_get_closer(dungeon->owner, on_dug, &best_dist, &best_x, &best_y, x - 1, y);
		try_get_closer(dungeon->owner, on_dug, &best_dist, &best_x, &best_y, x + 1, y);
		try_get_closer(dungeon->owner, on_dug, &best_dist, &best_x, &best_y, x, y - 1);
		try_get_closer(dungeon->owner, on_dug, &best_dist, &best_x, &best_y, x, y + 1);

		if (best_dist < current_dist)
		{
			x = best_x;
			y = best_y;
		}
		else
		{
			if (!on_dug)
				abort = 1;
			break;
		}
	}

	if (abort)
	{
		SYNCLOG("Player %d aborted gold dig attempt at %dx%d", (int)dungeon->owner, dig_gold->target_x, dig_gold->target_y);
	}
	else
	{
		if (wants_treasure_room && digging->expand_room.room_score >= 0)
		{
			initiate_expand_room(comp, digging, &digging->expand_room);
		}

		SYNCLOG("Player %d decided to dig for gold at %dx%d", (int)dungeon->owner, dig_gold->target_x, dig_gold->target_y);
		dig_gold->active = 1;
	}
}

static TbBool check_dig_to_gold(struct Computer2* comp, struct Digging* digging, struct DigToGold* dig_gold)
{
	struct Dungeon* dungeon;
	MapSlabCoord x, y;
	struct SlabMap* slab;
	struct SlabInfluence* influence;
	int best_score;
	MapSlabCoord best_x, best_y;

	SYNCDBG(8,"Starting");
	//evaluate all tiles we can dig

	dungeon = comp->dungeon;
	best_score = INT_MIN;
	best_x = best_y = 0; //make compiler shut up

	for (y = 1; y < map_tiles_y - 1; ++y)
	{
		for (x = 1; x < map_tiles_x - 1; ++x)
		{
			int score;
			int nx, ny;
			TbBool any_neighbors_digging;

			if (is_marked_for_digging(digging, x, y) || is_dangerous_digging(x, y))
				continue;

			influence = get_slab_influence(x, y);
			if (influence->dig_distance[dungeon->owner] < 0)
				continue; //don't bother evaluating blocks we can't reach

			score = 0;
			slab = get_slabmap_block(x, y);
			if (slab->kind == SlbT_GOLD)
			{
				score = 100000; //TODO: multiply by block (health / max_health) in future
			}
			else if (slab->kind == SlbT_GEMS)
			{
				score = 150000;
			}
			else
				continue;

			any_neighbors_digging = 0;
			for (ny = y - 2; ny <= y + 2; ++ny)
			{
				for (nx = x - 2; nx <= x + 2; ++nx)
				{
					if (is_marked_for_digging_and_undug(digging, nx, ny))
					{
						any_neighbors_digging = 1;
						break;
					}
				}
			}

			score /= 5 + influence->dig_distance[dungeon->owner];

			if (any_neighbors_digging)
				score *= 2;

			//TODO: add score for blocks far away from enemies, especially strong ones

			if (score > best_score)
			{
				best_score = score;
				best_x = x;
				best_y = y;
			}
		}
	}

	if (best_score >= 0)
	{
		dig_gold->target_x = best_x;
		dig_gold->target_y = best_y;
		initiate_dig_gold(comp, digging, dig_gold);

		return 1;
	}
	return 0;
}

static TbResult build_room_if_possible(struct Computer2* comp, RoomKind rkind, MapSlabCoord x, MapSlabCoord y)
{
	struct Dungeon* dungeon;
	struct RoomConfigStats *roomst;
	struct RoomStats* rstat;
	MapSubtlCoord stl_x, stl_y;
	dungeon = comp->dungeon;
	rstat = room_stats_get_for_kind(rkind);
	roomst = &slab_conf.room_cfgstats[rkind];

	// If we don't have money for the room - don't even try
	if (rstat->cost + 1000 >= dungeon->total_money_owned)
	{
		// Prefer leaving some gold, unless a flag is forcing us to build
		if (((roomst->flags & RoCFlg_BuildToBroke) == 0) || (rstat->cost >= dungeon->total_money_owned)) {
			return Lb_OK;
		}
	}
	// If we've lost the ability to build that room - kill the process and remove task (should we really remove task?)
	if (!is_room_available(dungeon->owner, rkind))
		return Lb_FAIL;

	stl_x = x * STL_PER_SLB + 1;
	stl_y = y * STL_PER_SLB + 1;
	if (slab_has_trap_on(x, y)) {
		if (try_game_action(comp, dungeon->owner, GA_SellTrap, 0, stl_x, stl_y, 1, 0) == Lb_SUCCESS)
			return Lb_SUCCESS;
	}
	if (can_build_room_at_slab(dungeon->owner, rkind, x, y))
	{
		return try_game_action(comp, dungeon->owner, GA_PlaceRoom, 0, stl_x, stl_y, 1, rkind);
	}

	return Lb_FAIL;
}

static void process_expand_room(struct Computer2* comp, struct Digging* digging)
{
	//see if we can still build, otherwise sell stuff in way, otherwise give up

	MapSlabCoord x, y;
	struct ExpandRoomPos* pos;
	struct Dungeon *dungeon;
	TbResult result;
	TbBool finished;
	TbBool aborted;

	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;
	pos = &digging->expand_room.room_pos;
	finished = 1;
	aborted = 0;

	//try to drop imps (not working very well since old CP is overriding, but better than nothing for now)
	if (digging->expand_room.drop_count < 3)
	{
		struct Thing* imp;
		imp = find_imp_for_urgent_dig(dungeon);
		if (thing_is_invalid(imp) ||
				create_task_move_creature_to_subtile(comp, imp, pos->access_x * STL_PER_SLB + 1, pos->access_y * STL_PER_SLB + 1, CrSt_ImpDigsDirt) == Lb_FAIL)
		{
			digging->expand_room.drop_count = 9999;
		}
		else
			digging->expand_room.drop_count += 1;
	}

	//quickfix for temple sacrifice, set central slab first (because first slab apparently is central_stl)
	if (digging->expand_room.rkind == RoK_TEMPLE)
	{
		x = (pos->max_x + pos->min_x) / 2;
		y = (pos->max_y + pos->min_y) / 2;

		struct SlabMap* slab;
		slab = get_slabmap_block(x, y);
		switch (slab->kind)
		{
		case SlbT_CLAIMED:
			finished = 0;
			//if has money -> try build
			result = build_room_if_possible(comp, digging->expand_room.rkind, x, y);
			if (result == Lb_SUCCESS) return;
			if (result == Lb_FAIL)
			{
				aborted = 1;
				goto exit_loops;
			}
			break;
		default:
			if (is_diggable_or_buildable(dungeon, digging, x, y))
				return;
		}
	}

	//check each slab in room
	for (y = pos->min_y; y <= pos->max_y; ++y)
	{
		for (x = pos->min_x; x <= pos->max_x; ++x)
		{
			struct SlabMap* slab;
			slab = get_slabmap_block(x, y);
			switch (slab->kind)
			{
			case SlbT_CLAIMED:
				finished = 0;
				//if has money -> try build
				result = build_room_if_possible(comp, digging->expand_room.rkind, x, y);
				if (result == Lb_SUCCESS) return;
				if (result == Lb_FAIL)
				{
					aborted = 1;
					goto exit_loops;
				}
				break;
			default:
				if (is_diggable_or_buildable(dungeon, digging, x, y))
					finished = 0;
			}
		}
	}
	exit_loops:

	//if finished, stop process
	if (finished || aborted)
	{
		if (aborted)
			SYNCLOG("Player %d aborted room %s", (int)dungeon->owner, room_code_name(digging->expand_room.rkind));
		else
			SYNCLOG("Player %d finished room %s", (int)dungeon->owner, room_code_name(digging->expand_room.rkind));

		digging->expand_room.active = 0;
		if (!aborted && digging->expand_room.rkind == RoK_GARDEN)
			digging->hatchery_built_turn = game.play_gameturn;
	}
}

static RoomKind decide_room_to_expand(struct Computer2* comp, struct Digging* digging)
{
	struct Dungeon *dungeon;
	dungeon = comp->dungeon;

	struct ValidRooms *bldroom;
	for (bldroom = valid_rooms_to_build; bldroom->rkind > 0; bldroom++)
	{
		if (bldroom->rkind == RoK_BRIDGE || bldroom->rkind == RoK_GUARDPOST || bldroom->rkind == RoK_UNKN17)
			continue;

		if (computer_check_room_available(comp, bldroom->rkind) != IAvail_Now) {
			continue;
		}

		if (!dungeon_has_room(dungeon, bldroom->rkind))
		{
			return bldroom->rkind;
		}

		if (bldroom->rkind == RoK_GARDEN && game.play_gameturn - digging->hatchery_built_turn < 20 * 40) //need to wait for chickens to spawn before evaluating again
			continue;
		
		long used_capacity;
		long total_capacity;
		get_room_kind_total_and_used_capacity(dungeon, bldroom->rkind, &total_capacity, &used_capacity);
		long free_capacity;
		free_capacity = computer_get_room_kind_free_capacity(comp, bldroom->rkind);
		if (bldroom->rkind == RoK_GARDEN)
		{
			if (4 * used_capacity > total_capacity)
				continue;
		}
		else if (free_capacity != 9999)
		{
			// The "+1" is to better handle cases when the existing room is very small (capacity lower than 10)
			// On higher capacities it doesn't make much difference, but highly increases chance
			// of building new room if existing capacity is low.
			if (free_capacity > 10*total_capacity/100 + 1) {
				continue;
			}
		}

		return bldroom->rkind;
	}
	
	return RoK_NONE;
}

static TbBool is_accessible(struct Dungeon* dungeon, struct SlabMap* slab)
{
	switch (slab->kind)
	{
	case SlbT_LAVA:
	case SlbT_WATER:
		return 0; //TODO: mark liquid as accessible once new digging knows bridge
	case SlbT_WALLDRAPE:
	case SlbT_WALLPAIRSHR:
	case SlbT_WALLTORCH:
	case SlbT_WALLWTWINS:
	case SlbT_WALLWWOMAN:
		return slabmap_owner(slab) == dungeon->owner;
	case SlbT_PATH:
	case SlbT_TORCHDIRT:
	case SlbT_EARTH:
	case SlbT_GOLD:
		return 1;
	default:
		return slab_kind_can_drop_here_now(slab->kind);
	}
}

static int eval_expand_room_wall(struct Digging* digging, int player, MapSlabCoord x, MapSlabCoord y)
{
	struct SlabMap* slab;
	slab = get_slabmap_block(x, y);
	switch (slab->kind)
	{
	case SlbT_WALLDRAPE:
	case SlbT_WALLPAIRSHR:
	case SlbT_WALLTORCH:
	case SlbT_WALLWTWINS:
	case SlbT_WALLWWOMAN:
		return slabmap_owner(slab) == player? 25 : -100;
	case SlbT_EARTH:
	case SlbT_TORCHDIRT:
		return is_marked_for_digging(digging, x, y)? 5 : 20; //treated as claimed if will be dig
	case SlbT_ROCK:
	case SlbT_GEMS:
		return 15;
	case SlbT_CLAIMED:
		return 5; //traps are good, right
	case SlbT_GOLD:
		return is_marked_for_digging(digging, x, y)? 5 : 1; //better than nothing, avoids opening
	case SlbT_LAVA:
	case SlbT_WATER:
	case SlbT_PATH:
		return -50; //might be able to reduce this malus once there's dangerous area detection
	case SlbT_DOORWOOD1:
	case SlbT_DOORWOOD2:
	case SlbT_DOORBRACE1:
	case SlbT_DOORBRACE2:
	case SlbT_DOORIRON1:
	case SlbT_DOORIRON2:
	case SlbT_DOORMAGIC1:
	case SlbT_DOORMAGIC2:
		return slabmap_owner(slab) == player? -25 : -100;
	default:
		return -10;
	}
}

static TbBool is_floor_for(MapSlabCoord x, MapSlabCoord y, int player)
{
	struct SlabMap* slab;
	slab = get_slabmap_block(x, y);
	if (SlbT_CLAIMED == slab->kind)
	{
		return slabmap_owner(slab) == player;
	}
	else if (SlbT_PATH == slab->kind)
	{
		return 1;
	}
	return 0;
}

static int eval_expand_room_pos(struct Dungeon* dungeon, struct Digging* digging, struct ExpandRoom* expand, struct ExpandRoomPos* pos)
{
	struct SlabMap* slab;
	MapSlabCoord x, y;
	int score;
	int num_tiles;
	int w, h, d, i;
	int center_x, center_y;
	struct SlabInfluence* influence;
	int enemy_distance;

	score = 0;

	//detect no straight access
	if (pos->access_dx == 0 && (pos->access_x < pos->min_x || pos->access_x > pos->max_x))
		return INT_MIN;
	if (pos->access_dy == 0 && (pos->access_y < pos->min_y || pos->access_y > pos->max_y))
		return INT_MIN;

	//detect access at wrong side (it should be just at room edge or we can dig straight tunnel)
	if (pos->access_dx < 0 && pos->access_x < pos->max_x)
		return INT_MIN;
	if (pos->access_dx > 0 && pos->access_x > pos->min_x)
		return INT_MIN;
	if (pos->access_dy < 0 && pos->access_y < pos->max_y)
		return INT_MIN;
	if (pos->access_dy > 0 && pos->access_y > pos->min_y)
		return INT_MIN;

	//detect out of bounds
	if (pos->min_x < 1 || pos->min_y < 1 || pos->max_x >= map_tiles_x - 1 || pos->max_y >= map_tiles_y - 1)
		return INT_MIN;

	//TODO: reduce score for library access

	//check access tunnel
	x = pos->access_x;
	y = pos->access_y;
	if (pos->access_dx < 0)
	{
		while (x != pos->max_x)
		{
			slab = get_slabmap_block(x, y);
			if (!is_accessible(dungeon, slab))
				return INT_MIN;
			if (slab->kind == SlbT_GOLD)
				score -= 5;

			x += pos->access_dx;
		}
	}
	else if (pos->access_dx > 0)
	{
		while (x != pos->min_x)
		{
			slab = get_slabmap_block(x, y);
			if (!is_accessible(dungeon, slab))
				return INT_MIN;
			if (slab->kind == SlbT_GOLD)
				score -= 5;

			x += pos->access_dx;
		}
	}
	if (pos->access_dy < 0)
	{
		while (y != pos->max_y)
		{
			slab = get_slabmap_block(x, y);
			if (!is_accessible(dungeon, slab))
				return INT_MIN;
			if (slab->kind == SlbT_GOLD)
				score -= 5;

			y += pos->access_dy;
		}
	}
	else if (pos->access_dy > 0)
	{
		while (y != pos->min_y)
		{
			slab = get_slabmap_block(x, y);
			if (!is_accessible(dungeon, slab))
				return INT_MIN;
			if (slab->kind == SlbT_GOLD)
				score -= 5;

			y += pos->access_dy;
		}
	}

	//check room
	num_tiles = 0;
	for (y = pos->min_y; y <= pos->max_y; ++y)
	{
		for (x = pos->min_x; x <= pos->max_x; ++x)
		{
			if (is_dangerous_digging(x, y))
				return INT_MIN;

			if (expand->predug)
			{
				if (!is_floor_for(x, y, dungeon->owner))
					return INT_MIN;
			}
			else
			{
				if (is_marked_for_digging_and_undug(digging, x, y)) //in case of multiple expansions
					return INT_MIN;

				//if outside walls or otherwise not treasure chamber, demand access (TODO: see if any other rooms worth having non-accessible interiors for)
				if (expand->rkind != RoK_TREASURE ||
					x == pos->min_x || x == pos->max_x || y == pos->min_y || y == pos->max_y)
				{
					if (!is_diggable_or_buildable(dungeon, digging, x, y)) //don't want overlapping expansions
						return INT_MIN;
				} //TODO: deduce points for non-conquerable interior in case where it passes through
			}

			//give score
			num_tiles += 1;
			if (num_tiles < expand->preferred_size)
				score += 1000;

			struct SlabMap* slab;
			slab = get_slabmap_block(x, y);
			switch (slab->kind)
			{
			case SlbT_EARTH:
				score -= 1; //to prefer path
				break;
			case SlbT_CLAIMED:
				score += 3;
				break;
			case SlbT_GOLD:
				if (expand->rkind == RoK_TREASURE)
					score += 5;
				else
					score -= 5; //prefer to avoid gold but it's not a big deal by itself
				break;
			case SlbT_WALLDRAPE:
			case SlbT_WALLPAIRSHR:
			case SlbT_WALLTORCH:
			case SlbT_WALLWTWINS:
			case SlbT_WALLWWOMAN:
				score -= 10; //prefer avoid digging through walls, breaks efficiency and is slow
				break;
			}

			//TODO: increase score for prison, barracks, graveyard, guard post close to enemy, opposite for other rooms
		}
	}

	//score walls
	for (y = pos->min_y - 1; y <= pos->max_y + 1; ++y)
	{
		score += eval_expand_room_wall(digging, dungeon->owner, pos->min_x - 1, y);
		score += eval_expand_room_wall(digging, dungeon->owner, pos->max_x + 1, y);
	}
	for (x = pos->min_x; x <= pos->max_x; ++x)
	{
		score += eval_expand_room_wall(digging, dungeon->owner, x, pos->min_y - 1);
		score += eval_expand_room_wall(digging, dungeon->owner, x, pos->max_y + 1);
	}

	w = pos->max_x - pos->min_x + 1;
	h = pos->max_y - pos->min_y + 1;
	d = abs(w - h);
	if (d > 1)
		score -= 200 * (d - 1) * max(w, h);
	else if (d == 1 && min(w, h) <= 2) //guiding heuristic to avoid sweeping around long useless rooms
		score -= 200;

	influence = get_slab_influence(pos->access_x, pos->access_y);
	score -= influence->heart_distance[dungeon->owner] * 4;

	center_x = (pos->max_x + pos->min_x) / 2;
	center_y = (pos->max_y + pos->min_y) / 2;
	influence = get_slab_influence(center_x, center_y);
	
	enemy_distance = INT_MAX;
	for (i = 0; i < KEEPER_COUNT; ++i)
	{
		if (i != dungeon->owner && !players_are_mutual_allies(dungeon->owner, i) && influence->dig_distance[i] >= 0)
			enemy_distance = min(enemy_distance, influence->dig_distance[i]);
	}
	if (enemy_distance != INT_MAX)
	{
		switch (expand->rkind)
		{
		case RoK_GUARDPOST:
		case RoK_BARRACKS:
		case RoK_GRAVEYARD:
		case RoK_PRISON:
			//score -= enemy_distance * 2;
			//not weighted now, add proxy prisons instead in future
			break;
		default:
			score += enemy_distance * 4;
		}
	}

	if (num_tiles > expand->preferred_size)
	{
		struct RoomStats* rstat;
		rstat = room_stats_get_for_kind(expand->rkind);
		score -= (num_tiles - expand->preferred_size) * rstat->cost;
	}

	return score;
}

static int eval_expand_room_moved(struct Dungeon* dungeon, struct Digging* digging, struct ExpandRoom* expand, struct ExpandRoomPos* room_pos, MapSlabCoord dx, MapSlabCoord dy)
{
	room_pos->min_x += dx;
	room_pos->max_x += dx;
	room_pos->min_y += dy;
	room_pos->max_y += dy;

	return eval_expand_room_pos(dungeon, digging, expand, room_pos);
}

static int eval_expand_room_enlarged(struct Dungeon* dungeon, struct Digging* digging, struct ExpandRoom* expand, struct ExpandRoomPos* room_pos, MapSlabCoord dx, MapSlabCoord dy)
{
	if (dx < 0) room_pos->min_x += dx;
	if (dy < 0) room_pos->min_y += dy;
	if (dx > 0) room_pos->max_x += dx;
	if (dy > 0) room_pos->max_y += dy;

	return eval_expand_room_pos(dungeon, digging, expand, room_pos);
}

static int eval_expand_room_rotated(struct Dungeon* dungeon, struct Digging* digging, struct ExpandRoom* expand, struct ExpandRoomPos* room_pos)
{
	int w, h;
	w = room_pos->max_x - room_pos->min_x;
	h = room_pos->max_y - room_pos->min_y;
	if (w == h)
		return INT_MIN;
	room_pos->max_x = room_pos->min_x + h;
	room_pos->max_y = room_pos->min_y + w;

	return eval_expand_room_pos(dungeon, digging, expand, room_pos);
}

static int adjust_room_score(int score, struct ExpandRoomPos* pos, struct ExpandRoom* expand, struct Dungeon* dungeon)
{
	MapSlabCoord x, y;

	if (pos->max_x - pos->min_x < 2 || pos->max_y - pos->min_y < 2)
	{
		score = INT_MIN;
	}
	else if (expand->rkind == RoK_GARDEN)
	{
		//reward lots of accessible free space near hatchery
		for (y = pos->min_y - 7; y <= pos->max_y + 7; ++y)
		{
			for (x = pos->min_x - 7; x <= pos->max_x + 7; ++x)
			{
				struct SlabInfluence* influence;
				struct SlabMap* slab;
				slab = get_slabmap_block(x, y);
				switch (slab->kind)
				{
				case SlbT_CLAIMED:
				case SlbT_EARTH:
				case SlbT_PATH:
				case SlbT_GOLD:
					influence = get_slab_influence(x, y);
					if (influence->dig_distance[dungeon->owner] >= 0)
					{
						if ((x >= pos->min_x && x <= pos->max_x) || (y >= pos->min_y && y <= pos->max_y))
							score += 3;
						else
							score += 1;
					}
					break;
				}
			}
		}
	}
	else if (expand->rkind == RoK_TREASURE)
	{
		//reward gold/gems near treasure
		for (y = pos->min_y - TREASURE_ROOM_SEARCH_RADIUS; y <= pos->max_y + TREASURE_ROOM_SEARCH_RADIUS; ++y)
		{
			for (x = pos->min_x - TREASURE_ROOM_SEARCH_RADIUS; x <= pos->max_x + TREASURE_ROOM_SEARCH_RADIUS; ++x)
			{
				int dist;
				struct SlabMap* slab;
				slab = get_slabmap_block(x, y);
				switch (slab->kind)
				{
				case SlbT_GOLD:
				case SlbT_GEMS:
					dist = min(abs(pos->min_x - x), abs(pos->max_x - x)) +
						min(abs(pos->min_y - y), abs(pos->max_y - y));
					score += 1 + 3 * max(0, TREASURE_ROOM_SEARCH_RADIUS - dist) / TREASURE_ROOM_SEARCH_RADIUS;
					break;
				}
			}
		}
	}

	return score;
}

static int eval_expand_room(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand, TbBool allow_move, int upscale, int downscale, MapSlabCoord x, MapSlabCoord y, MapSlabCoord dx, MapSlabCoord dy)
{
	struct Dungeon* dungeon;
	struct ExpandRoomPos best_pos;
	int best_score;
	int i;
	TbBool changed;

	SYNCDBG(18, "Starting");
	dungeon = comp->dungeon;
	expand->predug = 0;

	best_pos.access_x = x;
	best_pos.access_y = y;
	best_pos.access_dx = dx;
	best_pos.access_dy = dy;

	//find first tile we can expand on
	for (;;)
	{
		struct SlabMap* slab;
		slab = get_slabmap_block(x, y);
		if (slab->kind == SlbT_LAVA || slab->kind == SlbT_WATER)
		{
			if (!is_room_available(dungeon->owner, RoK_BRIDGE))
				return INT_MIN;
		}
		else if (!slab_kind_can_drop_here_now(slab->kind) || slab->kind == SlbT_CLAIMED)
			break;

		x += dx;
		y += dy;
	}

	best_pos.min_x = best_pos.max_x = x;
	best_pos.min_y = best_pos.max_y = y;
	
	best_score = eval_expand_room_pos(dungeon, digging, expand, &best_pos);

	//try different actions (enlarge/translate room and see if score grows, follow best gradient)
	changed = 1;
	for (i = 0; changed && i < 50; ++i) //max iterations to prevent infinite loop if bugged for some reason
	{
		struct ExpandRoomPos pos, iteration_pos;
		int score;
		changed = 0;
		memcpy(&iteration_pos, &best_pos, sizeof(pos));

		//try enlarging in any direction
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, -1, 0);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, 1, 0);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, 0, -1);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, 0, 1);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}

		//try to rotate
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_rotated(dungeon, digging, expand, &pos);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}

		//try to translate in any position
		if (allow_move)
		{
			memcpy(&pos, &iteration_pos, sizeof(pos));
			score = eval_expand_room_moved(dungeon, digging, expand, &pos, -1, 0);
			if (score > best_score)
			{
				changed = 1;
				best_score = score;
				memcpy(&best_pos, &pos, sizeof(pos));
			}

			memcpy(&pos, &iteration_pos, sizeof(pos));
			score = eval_expand_room_moved(dungeon, digging, expand, &pos, 1, 0);
			if (score > best_score)
			{
				changed = 1;
				best_score = score;
				memcpy(&best_pos, &pos, sizeof(pos));
			}

			memcpy(&pos, &iteration_pos, sizeof(pos));
			score = eval_expand_room_moved(dungeon, digging, expand, &pos, 0, -1);
			if (score > best_score)
			{
				changed = 1;
				best_score = score;
				memcpy(&best_pos, &pos, sizeof(pos));
			}

			memcpy(&pos, &iteration_pos, sizeof(pos));
			score = eval_expand_room_moved(dungeon, digging, expand, &pos, 0, 1);
			if (score > best_score)
			{
				changed = 1;
				best_score = score;
				memcpy(&best_pos, &pos, sizeof(pos));
			}
		}
	}

	best_score *= upscale;
	best_score /= downscale;

	//SYNCLOG("best WxH for room: %dx%d", best_pos.max_x - best_pos.min_x + 1, best_pos.max_y - best_pos.min_y + 1);

	//SYNCLOG("quitting after %d iterations with score %d", i, best_score);
	best_score = adjust_room_score(best_score, &best_pos, expand, dungeon);

	if (best_score > expand->room_score)
	{
		expand->room_score = best_score;
		memcpy(&expand->room_pos, &best_pos, sizeof(best_pos));
	}

	return best_score;
}

static int eval_expand_predug_room(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand, char* visited, MapSlabCoord x, MapSlabCoord y)
{
	struct Dungeon *dungeon;
	struct SlabInfluence* influence;
	struct ExpandRoomPos best_pos;
	int best_score;
	int player;
	int i;
	int changed;

	SYNCDBG(18, "Starting");
	dungeon = comp->dungeon;
	player = dungeon->owner;
	expand->predug = 1;

	if (!is_floor_for(x, y, player)) //does bounds check in practice
		return INT_MIN;

	if( visited[y * map_tiles_x + x] )
		return INT_MIN;

	influence = get_slab_influence(x, y);
	if( influence->heart_distance[player] < 0) //TODO: might wish to avoid this later if CP is able to manage disconnected dungeon
		return INT_MIN;

	//require 3x3 open at start
	if (	!is_floor_for(x - 1, y - 1, player) || !is_floor_for(x, y - 1, player) || !is_floor_for(x + 1, y - 1, player)
		||	!is_floor_for(x - 1, y, player) || !is_floor_for(x + 1, y, player)
		||	!is_floor_for(x - 1, y + 1, player) || !is_floor_for(x, y + 1, player) || !is_floor_for(x + 1, y + 1, player)
		)
	{
		return INT_MIN;
	}

	best_pos.min_x = x - 1;
	best_pos.min_y = y - 1;
	best_pos.max_x = x + 1;
	best_pos.max_y = y + 1;
	best_pos.access_x = best_pos.min_x; //TODO: unsure if this matters
	best_pos.access_y = y;
	best_pos.access_dx = 0;
	best_pos.access_dy = 0;
	best_score = eval_expand_room_pos(dungeon, digging, expand, &best_pos);

	//try different actions (enlarge/rotate room and see if score grows, follow best gradient)
	changed = 1;
	for (i = 0; changed && i < 50; ++i) //max iterations to prevent infinite loop if bugged for some reason
	{
		struct ExpandRoomPos pos, iteration_pos;
		int score;
		changed = 0;
		memcpy(&iteration_pos, &best_pos, sizeof(pos));

		//try enlarging in any direction
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, -1, 0);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, 1, 0);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, 0, -1);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_enlarged(dungeon, digging, expand, &pos, 0, 1);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}

		//try to rotate
		memcpy(&pos, &iteration_pos, sizeof(pos));
		score = eval_expand_room_rotated(dungeon, digging, expand, &pos);
		if (score > best_score)
		{
			changed = 1;
			best_score = score;
			memcpy(&best_pos, &pos, sizeof(pos));
		}
	}

	//mark visited on best room
	for (y = best_pos.min_y; y <= best_pos.max_y; ++y)
	{
		for (x = best_pos.min_x; x <= best_pos.max_x; ++x)
		{
			visited[y * map_tiles_x + x] = 1;
		}
	}

	best_score = adjust_room_score(best_score, &best_pos, expand, dungeon);

	if (best_score > expand->room_score)
	{
		//SYNCLOG("Found better predug room sized %dx%d", (best_pos.max_x - best_pos.min_x + 1), (best_pos.max_y - best_pos.min_y + 1));

		expand->room_score = best_score;
		memcpy(&expand->room_pos, &best_pos, sizeof(best_pos));
	}
	//else
		//SYNCLOG("Found worse predug room sized %dx%d", (best_pos.max_x - best_pos.min_x + 1), (best_pos.max_y - best_pos.min_y + 1));

	return best_score;
}

static TbBool find_expand_location(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand)
{
	struct Dungeon *dungeon;
	struct Thing* heart;
	struct Room* room;
	long i;
	RoomKind rkind;
	unsigned long k;
	MapSlabCoord x, y, r;
	int tolerance_counter;
	MapSlabCoord center_x, center_y;
	char visited[85][85]; //map size assumed

	SYNCDBG(18, "Starting");
	dungeon = comp->dungeon;
	expand->room_score = INT_MIN;
	heart = get_player_soul_container(dungeon->owner);
	if (thing_is_invalid(heart))
	{
		ERRORLOG("Called without heart");
		return false;
	}

	//check existing rooms' sides
	for (rkind = 1; rkind < sizeof(dungeon->room_kind) / sizeof(*dungeon->room_kind); ++rkind)
	{
		i = dungeon->room_kind[rkind];
		k = 0;
		while (i != 0)
		{
			room = room_get(i);
			if (room_is_invalid(room))
			{
				ERRORLOG("Jump to invalid room detected");
				break;
			}
			i = room->next_of_owner;
			// Per-room code
			{
				int upscale, downscale;
				MapSlabCoord x, y;
				x = subtile_slab(room->central_stl_x);
				y = subtile_slab(room->central_stl_y);

				upscale = downscale = 1;
				if (room->kind == RoK_LIBRARY)
				{
					upscale = 4;
					downscale = 5;
				}
				else if (room->kind == RoK_GARDEN)
				{
					switch (expand->rkind)
					{
					case RoK_WORKSHOP:
					case RoK_TRAINING:
					case RoK_LIBRARY:
					case RoK_SCAVENGER:
						upscale = 11;
						downscale = 10;
						break;
					default:
						upscale = 9;
						downscale = 10; //don't want to needlessly block hatchery with rooms that don't need it
						break;
					}
				}
				//SYNCLOG("evaluating expand %d %d", x, y);
				eval_expand_room(comp, digging, expand, 1, upscale, downscale, x, y, -1, 0);
				eval_expand_room(comp, digging, expand, 1, upscale, downscale, x, y, 1, 0);
				eval_expand_room(comp, digging, expand, 1, upscale, downscale, x, y, 0, -1);
				eval_expand_room(comp, digging, expand, 1, upscale, downscale, x, y, 0, 1);
			}
			// Per-room code ends
			k++;
			if (k > ROOMS_COUNT)
			{
				ERRORLOG("Infinite loop detected when sweeping rooms list");
				break;
			}
		}
	}

	//check unbuilt claimed map locations
	memset(visited, 0, sizeof (visited));
	center_x = subtile_slab(heart->mappos.x.stl.num);
	center_y = subtile_slab(heart->mappos.y.stl.num);
	tolerance_counter = -1;
	for (r = 4; r < 85; ++r)
	{
		int score_before;
		score_before = expand->room_score;
		//from heart, keep trying from an increasing radius until score no longer improves significantly
		//(it didn't improve for tolerance_counter > K radius steps)

		for (x = center_x - r; x <= center_x + r; ++x)
		{
			y = center_y - r;
			eval_expand_predug_room(comp, digging, expand, (char*)visited, x, y);
			y = center_y + r;
			eval_expand_predug_room(comp, digging, expand, (char*)visited, x, y);
		}

		for (y = center_y - r - 1; y <= center_y + r + 1; ++y)
		{
			x = center_x - r;
			eval_expand_predug_room(comp, digging, expand, (char*)visited, x, y);
			x = center_x + r;
			eval_expand_predug_room(comp, digging, expand, (char*)visited, x, y);
		}

		if (tolerance_counter >= 0)
		{
			if (expand->room_score <= score_before)
			{
				if (++tolerance_counter >= 20)
					break;
			}
			else
				tolerance_counter = 0;
		}
		else
		{
			if (expand->room_score >= 0)
				tolerance_counter = 0;
		}
	}

	//TODO: flood fill backup attempt

	//good score min size check
	return expand->room_score >= 0;
}

static void initiate_expand_room(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand)
{
	TbBool aborted;
	TbResult result;
	struct Dungeon *dungeon;
	struct ExpandRoomPos* pos;
	MapSlabCoord x, y;

	dungeon = comp->dungeon;
	pos = &expand->room_pos;
	aborted = 0;

	//dig access
	x = pos->access_x;
	y = pos->access_y;
	if (pos->access_dx < 0)
	{
		while (x != pos->max_x && x > 0) //second check not necessary technically, but paranoid about infinite loops
		{
			result = dig_if_needed(comp, digging, x, y);
			if (result == Lb_FAIL) aborted = 1;
			x += pos->access_dx;
		}
	}
	else if (pos->access_dx > 0)
	{
		while (x != pos->min_x && x < map_tiles_x - 1) //second check not necessary technically, but paranoid about infinite loops
		{
			result = dig_if_needed(comp, digging, x, y);
			if (result == Lb_FAIL) aborted = 1;
			x += pos->access_dx;
		}
	}
	if (pos->access_dy < 0)
	{
		while (y != pos->max_y && y > 0) //second check not necessary technically, but paranoid about infinite loops
		{
			result = dig_if_needed(comp, digging, x, y);
			if (result == Lb_FAIL) aborted = 1;
			y += pos->access_dy;
		}
	}
	else if (pos->access_dy > 0)
	{
		while (y != pos->min_y && x < map_tiles_y - 1) //second check not necessary technically, but paranoid about infinite loops
		{
			result = dig_if_needed(comp, digging, x, y);
			if (result == Lb_FAIL) aborted = 1;
			y += pos->access_dy;
		}
	}

	//dig room
	for (y = pos->min_y; y <= pos->max_y; ++y)
	{
		for (x = pos->min_x; x <= pos->max_x; ++x)
		{
			result = dig_if_needed(comp, digging, x, y);
			if (result == Lb_FAIL) aborted = 1;
		}
	}

	if (aborted)
	{
		SYNCLOG("Player %d aborting build room %s because can't order dig", (int)dungeon->owner, room_code_name(expand->rkind));
	}
	else
	{
		SYNCLOG("Player %d decided to build room %s of size %dx%d with score %d", (int)dungeon->owner, room_code_name(expand->rkind),
			(expand->room_pos.max_x - expand->room_pos.min_x + 1), (expand->room_pos.max_y - expand->room_pos.min_y + 1), expand->room_score);
		expand->drop_count = 0;
		expand->active = 1;

		//debug_digging_map("initiate_expand_room.txt", dungeon, digging);
	}
}

static void check_expand_room(struct Computer2* comp, struct Digging* digging)
{
	RoomKind rkind;
	struct Dungeon *dungeon;
	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;

	rkind = decide_room_to_expand(comp, digging);
	if (rkind == RoK_NONE)
	{
		SYNCLOG("Player %d doesn't want to expand right now", (int)dungeon->owner);
		return;
	}

	digging->expand_room.rkind = rkind;
	digging->expand_room.preferred_size = get_preferred_num_room_tiles(dungeon, rkind);
	if (find_expand_location(comp, digging, &digging->expand_room))
	{
		initiate_expand_room(comp, digging, &digging->expand_room);
	}
	else
	{
		SYNCLOG("Player %d wanted to build room %s but couldn't expand", (int)dungeon->owner, room_code_name(rkind));
	}
}

static void update_danger_map(struct Dungeon* dungeon)
{
	int x, y, i;
	long my_strength;
	long danger;
	struct SlabInfluence* influence;
	struct HeroRegion* hero_region;
	TbBool avoid;
	SYNCDBG(9, "Starting");

	memset(danger_map, 0, sizeof(danger_map));

	if (players_are_mutual_allies(dungeon->owner, HERO_PLAYER))
	{
		return;
	}

	my_strength = get_players_strength(dungeon);

	for (y = 1; y < map_tiles_y - 1; ++y)
	{
		for (x = 1; x < map_tiles_x - 1; ++x)
		{
			influence = get_slab_influence(x, y);

			avoid = 0;
			for (i = 0; i < KEEPER_COUNT; ++i)
			{
				if (i != dungeon->owner &&
					get_attitude_towards(dungeon->owner, i) == Attitude_Avoid &&
					influence->blocked_drop_distance[i] == 0)
				{
					avoid = 1;
					break;
				}
			}

			danger = 0;
			if (!avoid)
			{
				if (influence->hero_walk_region >= 0)
				{
					hero_region = get_hero_region(influence->hero_walk_region);
					if (!hero_region->has_heart[dungeon->owner]) //if not danger already here (don't pointlessly avoid it)
						danger += hero_region->hero_strength;
				}
				if (influence->hero_fly_region >= 0)
				{
					hero_region = get_hero_region(influence->hero_fly_region);
					if (!hero_region->has_heart[dungeon->owner]) //if not danger already here (don't pointlessly avoid it)
						danger += hero_region->hero_strength;
				}
			}

			if (avoid || 5 * danger > my_strength * 2)
			{
				danger_map[y - 1][x - 1] = 1;
				danger_map[y - 1][x] = 1;
				danger_map[y - 1][x + 1] = 1;
				danger_map[y][x - 1] = 1;
				danger_map[y][x] = 1;
				danger_map[y][x + 1] = 1;
				danger_map[y + 1][x - 1] = 1;
				danger_map[y + 1][x] = 1;
				danger_map[y + 1][x + 1] = 1;
			}
		}
	}
}

long computer_check_new_digging(struct Computer2* comp)
{
	int i;
	struct Dungeon *dungeon;
	struct Digging* digging;
	int max_gold_digs;
	int num_gold_digs;
	int controlled_diggers;

	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;
	digging = &comp_digging[dungeon->owner];

	update_danger_map(dungeon);

	debug_digging_map(NULL, NULL, NULL); //avoiding unused function warning

	process_bridging(comp, digging);

	if (digging->dig_attack.active)
	{
		process_dig_to_attack(comp, digging);
	}
	else
	{
		check_dig_to_attack(comp, digging);
	}

	if (digging->dig_secure.active)
	{
		process_dig_to_secure(comp, digging);
	}
	else
	{
		check_dig_to_secure(comp, digging);
	}

	controlled_diggers = dungeon->num_active_diggers - count_player_diggers_not_counting_to_total(dungeon->owner);
	max_gold_digs = min(CONCURRENT_GOLD_DIGS, max(1, 1 + controlled_diggers / 5 - (digging->expand_room.active? 1 : 0))); //TODO: count gem faces
	num_gold_digs = 0;
	for (i = 0; i < CONCURRENT_GOLD_DIGS; ++i)
	{
		if (digging->dig_gold[i].active)
			process_dig_to_gold(comp, digging, &digging->dig_gold[i]);
		if (digging->dig_gold[i].active)
			num_gold_digs += 1;
	}
	if (num_gold_digs < max_gold_digs)
	{
		for (i = 0; i < CONCURRENT_GOLD_DIGS; ++i)
		{
			if (!digging->dig_gold[i].active)
			{
				if (check_dig_to_gold(comp, digging, &digging->dig_gold[i]))
					num_gold_digs += 1;
				break; //don't consider more than one per check turn
			}
		}
	}

	if (digging->expand_room.active)
	{
		process_expand_room(comp, digging);
	}
	else
	{
		check_expand_room(comp, digging); //TODO: delay if this fails since it's an expensive check
	}

	return CTaskRet_Unk4;
}
