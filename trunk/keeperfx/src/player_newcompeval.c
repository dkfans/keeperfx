/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_newcompeval.c
 *     New computer player evaluation functions.
 * @par Purpose:
 *     Defines functions that look at the game state to determine something, but
 *     that do not take any action themselves.
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

#include "bflib_math.h"

#include "config_terrain.h"
#include "creature_states.h"
#include "game_legacy.h"
#include "magic.h"
#include "player_computer.h"
#include "player_instances.h"
#include "player_newcomp.h"
#include "power_hand.h"
#include "room_list.h"
#include "spdigger_stack.h"
#include "thing_data.h"
#include "thing_objects.h"

/************************************************************************/
/* Influence map stuff.                                                 */
/************************************************************************/

enum InfluenceMetric
{
	HeartDistance,
	DropDistance,
	DigDistance,
};

struct InfluenceNode //4 bytes
{
	char x, y; //happen to know the range is legal right now...
	char player;
	char metric;
};

static struct
{
	struct InfluenceNode* nodes;
	int push_index;
	int pop_index;
	int capacity;
} influence_queue; //very lazy queue implementation that doesn't attempt to reuse popped nodes in same pass. if we ever increase size of map a rework might be needed

static struct SlabInfluence null_influence;
static struct SlabInfluence influence_map[85][85]; //[y][x], static size for now since we're years away from dynamic sized maps

static void visit_influence(MapSlabCoord x, MapSlabCoord y, int player, enum InfluenceMetric metric, short dist);

struct SlabInfluence* get_slab_influence(MapSlabCoord x, MapSlabCoord y)
{
	if (x < 0 || y < 0 || x >= 85 || y >= 85 )
		return &null_influence;

	return &influence_map[y][x];
}

short * get_slab_influence_value(MapSlabCoord x, MapSlabCoord y, int player, enum InfluenceMetric metric)
{
	struct SlabInfluence* influence;
	static short dummy;

	influence = get_slab_influence(x, y);

	switch (metric)
	{
	case HeartDistance:
		return &influence->heart_distance[player];
		break;
	case DropDistance:
		return &influence->drop_distance[player];
		break;
	case DigDistance:
		return &influence->dig_distance[player];
		break;
	default:
		ERRORLOG("Invalid influence metric %d", (int)metric);
		return &dummy;
	}
}

static void queue_influence_visit(MapSlabCoord x, MapSlabCoord y, int player, enum InfluenceMetric metric, short dist)
{
	struct SlabMap* slab;
	struct InfluenceNode* node;
	short* stored_dist = get_slab_influence_value(x, y, player, metric);

	if (*stored_dist >= 0)
		return; //already visited. case of improvement is assumed not to exist (i.e. nodes are assumed to be visited in order of ascending dist)

	*stored_dist = dist;

	slab = get_slabmap_block(x, y);
	if (SlbT_GEMS == slab->kind)
		return; //ugly to do it here, but easiest

	if (influence_queue.push_index == influence_queue.capacity)
	{
		size_t size;

		if (influence_queue.capacity == 0)
			influence_queue.capacity = 100;
		else
			influence_queue.capacity *= 2;

		size = sizeof(*influence_queue.nodes) * influence_queue.capacity;
		influence_queue.nodes = (struct InfluenceNode*)realloc(influence_queue.nodes, size);
		if (NULL == influence_queue.nodes)
		{
			influence_queue.push_index = 0; //force termination
			ERRORLOG("Couldn't alloc %u bytes", size);
			return;
		}
	}

	node = &influence_queue.nodes[influence_queue.push_index++];
	
	node->metric = (char)metric;
	node->player = (char)player;
	node->x = (char)x;
	node->y = (char)y;
};

static void influence_neighbor(MapSlabCoord x, MapSlabCoord y, int player, enum InfluenceMetric metric, short dist)
{
	struct SlabMap* slab;
	int owner;
	slab = get_slabmap_block(x, y);
	owner = slabmap_owner(slab);

	if (slab->kind == SlbT_DUNGHEART && metric == HeartDistance)
	{
		dist = 0;
	}

	if (slab_kind_can_drop_here_now(slab->kind) || slab->kind == SlbT_PATH)
	{
		if (slab->kind != SlbT_PATH && owner == player)
			dist = 0;

		queue_influence_visit(x, y, player, metric, dist);
	}
	else if (metric == DigDistance)
	{
		switch (slab->kind)
		{
		case SlbT_EARTH:
		case SlbT_TORCHDIRT:
		case SlbT_GOLD:
		case SlbT_GEMS:
			queue_influence_visit(x, y, player, metric, dist);
			break;
		case SlbT_WALLDRAPE:
		case SlbT_WALLPAIRSHR:
		case SlbT_WALLTORCH:
		case SlbT_WALLWTWINS:
		case SlbT_WALLWWOMAN:
		case SlbT_DOORWOOD1:
		case SlbT_DOORWOOD2:
		case SlbT_DOORBRACE1:
		case SlbT_DOORBRACE2:
		case SlbT_DOORIRON1:
		case SlbT_DOORIRON2:
		case SlbT_DOORMAGIC1:
		case SlbT_DOORMAGIC2:
			if (owner == player)
				queue_influence_visit(x, y, player, metric, dist);
			break;
		case SlbT_LAVA:
		case SlbT_WATER:
			if (is_room_available(player, RoK_BRIDGE))
				queue_influence_visit(x, y, player, metric, dist);
			break;
		}
	}
};

static void visit_influence(MapSlabCoord x, MapSlabCoord y, int player, enum InfluenceMetric metric, short dist)
{
	dist += 1;
	influence_neighbor(x - 1, y, player, metric, dist);
	influence_neighbor(x + 1, y, player, metric, dist);
	influence_neighbor(x, y - 1, player, metric, dist);
	influence_neighbor(x, y + 1, player, metric, dist);
}

void update_influence_maps(void)
{
	struct SlabMap* slab;
	MapSlabCoord x, y;
	int owner;
	struct InfluenceNode* node;
	short* stored_dist;

	influence_queue.push_index = 0;
	influence_queue.pop_index = 0;
	memset(&null_influence, 0xff, sizeof(null_influence));
	memset(&influence_map, 0xff, sizeof(influence_map));

	SYNCDBG(9, "Seeding from map");

	//seed search on map
	for (y = 1; y < map_tiles_y - 1; ++y)
	{
		for (x = 1; x < map_tiles_x - 1; ++x)
		{
			slab = get_slabmap_block(x, y);
			owner = slabmap_owner(slab);
			if (owner >= KEEPER_COUNT)
				continue;

			if (slab_kind_can_drop_here_now(slab->kind))
			{
				queue_influence_visit(x, y, owner, DropDistance, 0);
				queue_influence_visit(x, y, owner, DigDistance, 0);
			}
			if (slab->kind == SlbT_DUNGHEART)
			{
				queue_influence_visit(x, y, owner, HeartDistance, 0);
			}
		}
	}

	SYNCDBG(9, "Processing node queue of %d seed nodes", influence_queue.push_index);

	while (influence_queue.pop_index < influence_queue.push_index)
	{
		node = &influence_queue.nodes[influence_queue.pop_index++];
		stored_dist = get_slab_influence_value(node->x, node->y, node->player, (enum InfluenceMetric)node->metric);
		if (*stored_dist >= 0)
		{
			visit_influence(node->x, node->y, node->player, (enum InfluenceMetric)node->metric, *stored_dist);
		}
		else
		{
			ERRORLOG("Distance invariant bug");
			return;
		}
	}

	SYNCDBG(8, "Processed %d nodes", influence_queue.pop_index);
}

/************************************************************************/
/* Attitude (overall strategy) stuff.                                    */
/************************************************************************/

static enum PlayerAttitude attitude_map[KEEPER_COUNT][PLAYERS_EXT_COUNT];

void update_attitudes(void)
{
	int i, j;
	long strength[KEEPER_COUNT];
	long allied_strength[KEEPER_COUNT];
	struct Dungeon* dungeon;

	for (i = 0; i < KEEPER_COUNT; ++i)
	{
		strength[i] = 0;
		dungeon = get_dungeon(i);
		if (player_has_heart(i))
			strength[i] = calc_players_strength(dungeon);
	}

	for (i = 0; i < KEEPER_COUNT; ++i)
	{
		allied_strength[i] = strength[i];
		for (j = 0; j < KEEPER_COUNT; ++j)
		{
			if (i != j && players_are_mutual_allies(i, j))
				allied_strength[i] += strength[j];
		}
	}

	for (i = 0; i < KEEPER_COUNT; ++i)
	{
		//hero
		attitude_map[i][PLAYER_GOOD] = Attitude_SuperAggressive;

		//neutrals
		attitude_map[i][PLAYER_NEUTRAL] = Attitude_SuperAggressive;

		//keepers
		for (j = 0; j < KEEPER_COUNT; ++j)
		{
			struct Dungeon* their_dungeon;
			their_dungeon = get_dungeon(i);
			if (i == dungeon->owner || dungeon_invalid(their_dungeon))
				continue;

			if (players_are_mutual_allies(i, j))
				attitude_map[i][j] = Attitude_Friend;
			else
			{
				long my_strength, their_strength;
				my_strength = allied_strength[i];
				their_strength = allied_strength[j];

				if (2 * their_strength < my_strength)
				{
					attitude_map[i][j] = Attitude_SuperAggressive;
				}
				else if (5 * their_strength < 4 * my_strength)
				{
					attitude_map[i][j] = Attitude_Aggressive;
				}
				else if (their_strength < my_strength)
				{
					attitude_map[i][j] = Attitude_Ignore;
				}
				else
				{
					attitude_map[i][j] = Attitude_Avoid;
				}
			}

			if (!players_are_enemies(i, j) && attitude_map[i][j] >= Attitude_Aggressive)
				attitude_map[i][j] = Attitude_Ignore;
		}
	}

	//TODO: log when attitude changes

	//TODO: take into account money and access to gems/workshop -> it might be better delaying engagement because we expect them to go bankrupt long before us
}

enum PlayerAttitude get_attitude_towards_f(int player, int towards_player, const char *func_name)
{
	if (player < 0 || player >= KEEPER_COUNT)
	{
		ERRORLOG("%s: player %d outside valid range", func_name, player);
		return Attitude_Ignore;
	}
	if (towards_player < 0 || towards_player >= PLAYERS_EXT_COUNT)
	{
		ERRORLOG("%s: towards_player %d outside valid range", func_name, towards_player);
		return Attitude_Ignore;
	}
	return attitude_map[player][towards_player];
};

/************************************************************************/
/* Any imps thinking to dig gems right now?                             */
/************************************************************************/
TbBool is_digging_any_gems(struct Dungeon *dungeon)
{
	SYNCDBG(19,"Starting");

	long i;
	unsigned long k;
	k = 0;
	i = dungeon->digger_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		if (cctrl->combat_flags == 0)
		{
			long state_type;
			state_type = get_creature_state_type(thing);

			if (state_type == CrStTyp_Work
				&& cctrl->digger.last_did_job == SDLstJob_DigOrMine
				&& is_digging_indestructible_place(thing))
			{
				SYNCDBG(18, "Gems being dug by player %d", (int)dungeon->owner);
				return 1;
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return 0;
		}
	}

	SYNCDBG(18, "Gems NOT being dug by player %d", (int)dungeon->owner);

	return 0;
}

/************************************************************************/
/* Good imp for sacrifice is level 1 and has low health and money.      */
/************************************************************************/
struct Thing * find_imp_for_sacrifice(struct Dungeon* dungeon)
{
	SYNCDBG(19,"Starting");

	long best_priority;
	struct Thing *best_tng;
	long digger_price;
	TbBool digging_gems;

	digger_price = compute_lowest_digger_price(dungeon->owner);
	best_priority = INT_MAX;
	best_tng = INVALID_THING;
	digging_gems = is_digging_any_gems(dungeon);

	long i;
	unsigned long k;
	k = 0;
	i = dungeon->digger_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		if (cctrl->combat_flags == 0
			&& cctrl->explevel == 0 //at lowest experience level
			&& (digging_gems || thing->creature.gold_carried == 0)) //no gold carried if no gem access
		{
			if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
			{
				if (!creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
				{
					struct CreatureStats* crtrstats;
					crtrstats = creature_stats_get_from_thing(thing);

					long priority;
					long state_type;
					int max_health;
					state_type = get_creature_state_type(thing);

					priority = thing->creature.gold_carried; //base value
					if (state_type == CrStTyp_Work)
						priority += 500; //aborted work valued at this many gold

					max_health = compute_creature_max_health(crtrstats->health, cctrl->explevel);
					priority += digger_price * thing->health / max_health; //full health valued at this many gold

					if (priority < best_priority)
					{
						best_priority = priority;
						best_tng = thing;
					}
				}
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return INVALID_THING;
		}
	}

	return best_tng;
}

/*************************************************************************/
/* Find good imp for claim and ensure not too many are claiming already. */
/*************************************************************************/
struct Thing * find_imp_for_claim(struct Dungeon* dungeon)
{
	SYNCDBG(19,"Starting");

	long max_claimers;
	max_claimers = (dungeon->num_active_diggers - count_player_diggers_not_counting_to_total(dungeon->owner) - 1) / 3; //4th imp can claim
	if (max_claimers <= 0)
		return INVALID_THING;

	long best_priority;
	struct Thing *best_tng;
	TbBool digging_gems;
	best_priority = INT_MIN;
	best_tng = INVALID_THING;
	digging_gems = is_digging_any_gems(dungeon);

	long claimers;
	long i;
	unsigned long k;
	claimers = 0;
	k = 0;
	i = dungeon->digger_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		if (cctrl->combat_flags == 0
			&& (digging_gems || thing->creature.gold_carried == 0)) //no gold carried if no gem access
		{
			if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
			{
				CrtrStateId state;
				state = get_creature_state_besides_move(thing);
				if (state == CrSt_ImpConvertsDungeon || state == CrSt_ImpArrivesAtConvertDungeon)
					claimers += 1;
				else if (!creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
				{
					long priority;
					long state_type;
					state_type = get_creature_state_type(thing);

					priority = 1000 * thing->health / (1000 + thing->creature.gold_carried); //base value
					if (state_type == CrStTyp_Work)
						priority /= 2;

					if (priority > best_priority)
					{
						best_priority = priority;
						best_tng = thing;
					}
				}
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return INVALID_THING;
		}
	}

	if (claimers >= max_claimers)
		best_tng = INVALID_THING;
	return best_tng;
}

/*************************************************************************/
/* Find good imp for urgent digging.                                     */
/*************************************************************************/
struct Thing * find_imp_for_urgent_dig(struct Dungeon* dungeon)
{
	SYNCDBG(19,"Starting");

	long best_priority;
	struct Thing *best_tng;
	best_priority = INT_MIN;
	best_tng = INVALID_THING;

	long i;
	unsigned long k;
	k = 0;
	i = dungeon->digger_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		if (cctrl->combat_flags == 0)
		{
			if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
			{
				if (!creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
				{
					long priority;
					long state_type;
					state_type = get_creature_state_type(thing);

					priority = 100000 * (1 + cctrl->explevel) / (1000 + thing->creature.gold_carried + thing->health); //base value
					if (state_type == CrStTyp_Work)
					{
						priority /= 2;

						CrtrStateId state;
						state = get_creature_state_besides_move(thing);
						switch (state)
						{
						case CrSt_ImpArrivesAtDigDirt:
						case CrSt_ImpDigsDirt:
						case CrSt_ImpConvertsDungeon:
						case CrSt_ImpArrivesAtConvertDungeon:
							goto thing_list_loop_body_ends; //don't override these states
						}
					}

					if (priority > best_priority)
					{
						best_priority = priority;
						best_tng = thing;
					}
				}
			}
		}
		thing_list_loop_body_ends:
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return INVALID_THING;
		}
	}

	return best_tng;
}

/*******************************************************************************************/
/* Find creature that can be considered good for non-combat attacks, e.g. opening doors.   */
/*******************************************************************************************/
struct Thing * find_creature_for_low_priority_attack(struct Dungeon* dungeon, TbBool strong)
{
	SYNCDBG(19,"Starting");

	long max_attackers;
	max_attackers = (dungeon->num_active_creatrs - count_player_creatures_not_counting_to_total(dungeon->owner) + 2) / 3; //1st creature can attack, one third can attack
	if (max_attackers <= 0)
		return INVALID_THING;

	long best_priority;
	struct Thing *best_tng;
	best_priority = INT_MIN;
	best_tng = INVALID_THING;

	long attackers;
	long i;
	unsigned long k;
	attackers = 0;
	k = 0;
	int test;
	test = 0;
	i = dungeon->creatr_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		//if (cctrl->combat_flags != 0)
		{
			if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
			{
				CrtrStateId state;
				state = get_creature_state_besides_move(thing);
				if (state == CrSt_CreatureDoorCombat)
					attackers += 1;
				else if (state != CrSt_CreatureInCombat && state != CrSt_CreatureObjectCombat && !creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
				{
					test += 1;
					struct CreatureStats* crtrstats;
					int max_health;
					crtrstats = creature_stats_get_from_thing(thing);
					max_health = compute_creature_max_health(crtrstats->health, cctrl->explevel);

					if (thing->health * 2 >= max_health)
					{
						long priority;
						long state_type;
						state_type = get_creature_state_type(thing);

						priority = thing->health * (crtrstats->armour + crtrstats->strength);

						if (strong && state_type == CrStTyp_Work)
							priority /= 2;

						if (!strong)
							priority = INT_MAX / 2 - priority;

						if (priority > best_priority)
						{
							best_priority = priority;
							best_tng = thing;
						}
					}
				}
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return INVALID_THING;
		}
	}

	if (attackers >= max_attackers)
		best_tng = INVALID_THING;
	return best_tng;
}

/*******************************************************************************************/
/* Find a chicken somewhere in a room (e.g. to feed prisoner).                             */
/*******************************************************************************************/
static struct Thing * find_any_chicken_in_room(struct Room* room)
{
	struct Thing* thing;
	long k, i;
	if (room->slabs_count <= 0)
	{
		WARNLOG("Room with no slabs detected!");
		return INVALID_THING;
	}
	k = 0;
	i = room->slabs_list;
	while (i != 0)
	{
		MapSubtlCoord stl_x,stl_y;
		stl_x = slab_subtile_center(slb_num_decode_x(i));
		stl_y = slab_subtile_center(slb_num_decode_y(i));
		// Per room tile code
		thing = get_object_around_owned_by_and_matching_bool_filter(
			subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, thing_is_mature_food);
		if (!thing_is_invalid(thing))
			return thing;
		// Per room tile code ends
		i = get_next_slab_number_in_room(i);
		k++;
		if (k > room->slabs_count)
		{
			ERRORLOG("Room slabs list length exceeded when sweeping");
			break;
		}
	}

	return INVALID_THING;
}

/*******************************************************************************************/
/* Find a chicken somewhere in the dungeon's hatcheries (e.g. to feed prisoner).           */
/*******************************************************************************************/
struct Thing * find_any_chicken(struct Dungeon* dungeon)
{
	struct Room* room;
	long i;
	unsigned long k;
	unsigned long n;
	SYNCDBG(18,"Starting");
	n = count_player_rooms_of_type(dungeon->owner, RoK_GARDEN);
	if (n == 0)
		return INVALID_THING; //no hatcheries
	n = ACTION_RANDOM(n); //pick random room
	i = dungeon->room_kind[RoK_GARDEN];
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
		if (k == n)
		{
			//this is the room
			return find_any_chicken_in_room(room);
		}
		// Per-room code ends
		k++;
		if (k > ROOMS_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping rooms list");
			break;
		}
	}

	return INVALID_THING;
}

/************************************************************************/
/* Higher strength => good chance of player winning a battle.           */
/************************************************************************/
long calc_players_strength(struct Dungeon* dungeon)
{
	SYNCDBG(19,"Starting");

	struct CreatureControl *cctrl;
	struct Thing *thing;
	unsigned long k;
	int i;
	long strength;
	k = 0;
	i = dungeon->creatr_list_start;
	strength = 0.0f;
	while (i != 0)
	{
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body

		strength += get_creature_thing_score(thing); //TODO: replace with better estimate

		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			break;
		}
	}
	SYNCDBG(19,"Finished");

	return strength;
}

/************************************************************************/
/* When building a new room, how many tiles do I ideally want?          */
/************************************************************************/
int get_preferred_num_room_tiles(struct Dungeon* dungeon, RoomKind rkind)
{
	//TODO: perhaps this should be loaded from file?

	int existing_num;
	int target;
	existing_num = count_slabs_of_room_type(dungeon->owner, rkind);

	switch (rkind)
	{
	case RoK_BRIDGE:
	case RoK_GUARDPOST:
		return 1;
	case RoK_TEMPLE:
	case RoK_SCAVENGER:
		return min(25, max(9, 6 + existing_num / 2));
	case RoK_TORTURE:
	case RoK_PRISON:
	case RoK_GRAVEYARD:
		return min(25, max(15, 5 + existing_num / 2));
	case RoK_TREASURE:
	case RoK_TRAINING:
	case RoK_LIBRARY:
	case RoK_GARDEN: //TODO: might wish to scale hatchery similar to lair
		return 25;
	case RoK_WORKSHOP:
		return 30;
	case RoK_LAIR:
		if (existing_num == 0)
			return max(12, 4 * max(dungeon->max_creatures_attracted, dungeon->num_active_creatrs) / 3);
		else
		{
			target = dungeon->num_active_creatrs < dungeon->max_creatures_attracted?
				dungeon->max_creatures_attracted + 2 - dungeon->num_active_creatrs :
				3 * dungeon->num_active_creatrs / 5 + 1;
			return max(12, (1000 * target) / (1000 * dungeon->num_active_creatrs / existing_num)); //target creatures / creatures per lair space = target lair space
		}
	default:
		return 9;
	}
}
