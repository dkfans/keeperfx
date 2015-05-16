/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_newcompchecks.c
 *     New computer player checks.
 * @par Purpose:
 *     Cleanly separates computer player checks that weren't in the original
 *     game.
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

// CHECK FOR DOOR ATTACKS /////////////////////////////////////////////////////////////////////////

static TngUpdateRet eval_any_traps_callback(struct Thing* thing, ModTngFilterParam filter)
{
	//return/param semantics basically ignored, we used our own
	struct Dungeon* dungeon;
	dungeon = (struct Dungeon*)filter;

	if (thing->class_id == TCls_Trap)
	{
		return thing->owner != dungeon->owner;
	}

	return 0;
}

static TngUpdateRet eval_door_attack_check_any_friendlies_callback(struct Thing* thing, ModTngFilterParam filter)
{
	//return/param semantics basically ignored, we use our own
	struct Dungeon* dungeon;
	dungeon = (struct Dungeon*)filter;

	if (thing->class_id == TCls_Creature
		&& !creature_is_being_unconscious(thing)
		&& thing->owner == dungeon->owner)
	{
		if (thing_is_creature_special_digger(thing))
			return 0;
		return 1; //non-digger player creature, the kind we wanted to identify
	}

	return 0;
}

static TbBool attack_door_from_player[PLAYERS_EXT_COUNT] = { 1, 1, 1, 1, 1, 1 };
static int eval_door_attack_check_neighbor(MapSlabCoord x, MapSlabCoord y, MapSlabCoord trap_x, MapSlabCoord trap_y, struct Dungeon* my_dungeon, TbBool* trap)
{
	struct SlabMap* slab;
	long owner;
	int value;

	slab = get_slabmap_block(x, y);

	switch (slab->kind) //high value for low doors because they're easier to attack. could look up based on door HP instead later
	{
	case SlbT_DUNGHEART:
		value = INT_MAX;
		break;
	case SlbT_DOORWOOD1:
	case SlbT_DOORWOOD2:
		value = 400;
		break;
	case SlbT_DOORBRACE1:
	case SlbT_DOORBRACE2:
		value = 300;
		break;
	case SlbT_DOORIRON1:
	case SlbT_DOORIRON2:
		value = 200;
		break;
	case SlbT_DOORMAGIC1:
	case SlbT_DOORMAGIC2:
		value = 100;
		break;
	default:
		return 0; //don't consider
	}

	owner = slabmap_owner(slab);
	if (owner == my_dungeon->owner)
		return 0; //don't consider our own
	if (!attack_door_from_player[owner])
		return 0; //don't care about this player

	if (slab->kind == SlbT_DUNGHEART)
	{
		struct Dungeon* their_dungeon;
		if (owner <= HERO_PLAYER && (their_dungeon = get_dungeon(owner)) && !dungeon_invalid(their_dungeon))
		{
			//prioritize killing dungeon hearts above anything else
			return value - (int)(100 * calc_players_strength(their_dungeon));
		}
		return -1; //no interested messing with hero dungeon hearts as they are likely script influenced anyway
	}

	//ignore places with friendly creatures already
	if (do_to_things_with_param_on_tile(x, y, eval_door_attack_check_any_friendlies_callback, (ModTngFilterParam)my_dungeon) != 0)
		return -1; //already covering this

	//SYNCLOG("%d will try to attack against %d (kind = %d)", my_dungeon->owner, owner, slab->kind);

	//TODO: should have an influence map flood fill pass instead where rooms flood so AI can distinguish with more than dungeon heart path

	//modify value by distance to their dungeon heart
	struct Dungeon* their_dungeon;
	if (owner <= HERO_PLAYER && (their_dungeon = get_dungeon(owner)) && !dungeon_invalid(their_dungeon) && their_dungeon->dnheart_idx > 0)
	{
		struct Thing* heart;
		MapSlabCoord heart_x, heart_y;
		heart = thing_get(their_dungeon->dnheart_idx);
		heart_x = subtile_slab_fast(heart->mappos.x.stl.num);
		heart_y = subtile_slab_fast(heart->mappos.y.stl.num);
		value -= abs(heart_x - x) + abs(heart_y - y);
	}

	if (do_to_things_with_param_on_tile(trap_x, trap_y, eval_any_traps_callback, (ModTngFilterParam)my_dungeon) != 0)
	{
		value /= 2;
		*trap = 1;
	}

	if (owner == HERO_PLAYER)
		value /= 3;

	//add random component
	value += ACTION_RANDOM(10) * ACTION_RANDOM(10);

	return value;
}

long computer_check_for_door_attacks(struct Computer2 *comp)
{
	//using this check to break through enemy doors and destroy dungeon heart when we have an advantage

	struct Dungeon *dungeon;
	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;

	int i;
	float my_strength, their_strength;
	my_strength = calc_players_strength(dungeon);
	attack_door_from_player[HERO_PLAYER] = 1;
	for (i = 0; i < KEEPER_COUNT; ++i)
	{
		struct Dungeon* their_dungeon;
		their_dungeon = get_dungeon(i);
		if (i == dungeon->owner || dungeon_invalid(their_dungeon))
			continue;

		if (players_are_enemies(dungeon->owner, i))
		{
			their_strength = calc_players_strength(their_dungeon);
			attack_door_from_player[i] = their_strength < 0.8f * my_strength;
			if (!attack_door_from_player[i])
				attack_door_from_player[HERO_PLAYER] = 0; //to avoid opening frequent early blocking doors by level designers
			//SYNCLOG("%d our: %f, their: %f", dungeon->owner, my_strength, their_strength);
		}
		else
			attack_door_from_player[i] = 0;
	}

	struct Thing* strongest;
	struct Thing* weakest;
	strongest = find_creature_for_low_priority_attack(comp->dungeon, 1);
	if (thing_is_invalid(strongest))
		return CTaskRet_Unk4;
	weakest = find_creature_for_low_priority_attack(comp->dungeon, 0);
	// strongest exists so weakest should exist as well, even if it is the same creature

	//we only try to find the "best" door and assign creature to it
	//we only attempt to have one creature attacking a door at once in case of losses related to traps

	MapSlabCoord x, y;
	struct SlabMap* slab;
	long owner;
	int eval;
	int local_best, global_best;
	TbBool local_trap, global_trap;
	MapSlabCoord best_x, best_y;
	global_best = 0;
	for (y = 1; y < map_tiles_y - 1; ++y)
	{
		for (x = 1; x < map_tiles_x - 1; ++x)
		{
			slab = get_slabmap_block(x, y);
			if (!slab_kind_can_drop_here_now(slab->kind) || slab->kind == SlbT_PRISON)
				continue;

			owner = slabmap_owner(slab);
			if (owner != dungeon->owner)
				continue;

			//ignore places with friendly creatures already
			if (do_to_things_with_param_on_tile(x, y, eval_door_attack_check_any_friendlies_callback, (ModTngFilterParam)dungeon) != 0)
				continue;

			local_trap = 0;
			eval = eval_door_attack_check_neighbor(x, y - 1, x, y - 2, dungeon, &local_trap);
			if (eval < 0) //disqualify tiles with unsuitable neighbors
				continue;
			local_best = eval;

			eval = eval_door_attack_check_neighbor(x - 1, y, x - 2, y, dungeon, &local_trap);
			if (eval < 0) //disqualify tiles with unsuitable neighbors
				continue;
			local_best = max(local_best, eval);

			eval = eval_door_attack_check_neighbor(x, y + 1, x, y + 2, dungeon, &local_trap);
			if (eval < 0) //disqualify tiles with unsuitable neighbors
				continue;
			local_best = max(local_best, eval);

			eval = eval_door_attack_check_neighbor(x + 1, y, x + 2, y, dungeon, &local_trap);
			if (eval < 0) //disqualify tiles with unsuitable neighbors
				continue;
			local_best = max(local_best, eval);

			//TODO: guard against creatures of players other than the one we have greenlit

			if (local_best > global_best)
			{
				global_best = local_best;
				global_trap = local_trap;
				best_x = x;
				best_y = y;
			}
		}
	}

	if (global_best > 0)
	{
		struct Thing* crtr;
		crtr = global_trap? weakest : strongest;

		//try to drop creature
		if (!create_task_move_creature_to_subtile(comp, crtr,
				best_x * STL_PER_SLB + 1, best_y * STL_PER_SLB + 1, CrSt_CreatureDoorCombat))
			return CTaskRet_Unk4;

		SYNCDBG(18, "Dropped creature to attack door for %d", dungeon->owner);
		return CTaskRet_Unk1;
	}

	return CTaskRet_Unk4;
}

// CHECK FOR CLAIMS ///////////////////////////////////////////////////////////////////////////////

static TbBool found_unconscious; //ugly, and prevents using OpenMP later
static TngUpdateRet eval_claim_check_neighbor_callback(struct Thing* thing, ModTngFilterParam filter)
{
	//return/param semantics basically ignored, we used our own
	struct Dungeon* dungeon;
	dungeon = (struct Dungeon*)filter;

	switch (thing->class_id)
	{
	case TCls_Creature:
		if (!creature_is_being_unconscious(thing))
		{
			if (thing->owner == dungeon->owner)
			{
				//if digger, cancel because we already have imp working closeby
				if (thing_is_creature_special_digger(thing))
					return 1;
				return 0;
			}
			else
			{
				//if enemy non-digger, cancel
				if (is_hero_thing(thing))
					return 1;
				if (is_neutral_thing(thing))
					return 0;
				if (thing_is_creature_special_digger(thing))
					return 0;
				return 1;
			}
		}
		else
		{
			if (thing->owner != dungeon->owner)
				found_unconscious = 1;
			return 0; //always ignore unconscious creatures
		}

		break;
	}

	return 0;
}

static TbBool claim_from_player[PLAYERS_EXT_COUNT] = { 1, 1, 1, 1, 1, 1 };
static int eval_claim_check_neighbor(MapSlabCoord x, MapSlabCoord y, struct Dungeon* my_dungeon, TbBool cheap_diggers)
{
	struct SlabMap* slab;
	long owner;

	slab = get_slabmap_block(x, y);
	if ((!slab_kind_can_drop_here_now(slab->kind) && slab->kind != SlbT_PATH) || slab->kind == SlbT_DUNGHEART)
		return 0; //don't consider

	owner = slabmap_owner(slab);
	if (owner == my_dungeon->owner && slab->kind != SlbT_PATH)
		return 0; //don't consider

	if (!claim_from_player[owner] && slab->kind != SlbT_PATH)
		return -1; //avoid this player

	//SYNCLOG("%d will try to drop against %d (kind = %d)", my_dungeon->owner, owner, slab->kind);

	//ignore places with enemy traps, friendly imps, enemy creatures
	if (do_to_things_with_param_on_tile(x, y, eval_claim_check_neighbor_callback, (ModTngFilterParam)my_dungeon) != 0)
		return -1;

	if (!cheap_diggers && do_to_things_with_param_on_tile(x, y, eval_any_traps_callback, (ModTngFilterParam)my_dungeon) != 0)
		return -1;

	if( slab->kind == SlbT_PATH)
		return 100; //TODO: better to use influence map as discussed below in future

	int value;
	struct Room* room;
	room = room_get(slab->room_index);
	if (room_is_invalid(room))
		value = 150;
	else if (room->kind == RoK_ENTRANCE)
	{
		value = 250;
	}
	else
	{
		//base_value = C * (room cost + contents value)
		value = room->slabs_count * room_stats_get_for_kind(room->kind)->cost / 10; //TODO: add treasure
	}

	//TODO: should have an influence map flood fill pass instead where rooms flood so AI can distinguish with more than dungeon heart path
	
	//modify value by distance to their dungeon heart
	struct Dungeon* their_dungeon;
	if (owner <= HERO_PLAYER && (their_dungeon = get_dungeon(owner)) && !dungeon_invalid(their_dungeon) && their_dungeon->dnheart_idx > 0)
	{
		struct Thing* heart;
		MapSlabCoord heart_x, heart_y;
		heart = thing_get(their_dungeon->dnheart_idx);
		heart_x = subtile_slab_fast(heart->mappos.x.stl.num);
		heart_y = subtile_slab_fast(heart->mappos.y.stl.num);
		value -= abs(heart_x - x) + abs(heart_y - y);
	}
	
	//add random component
	
	value += ACTION_RANDOM(10) * ACTION_RANDOM(10);

	return value;
}

long computer_check_for_claims(struct Computer2 *comp)
{
	//check for claiming land and capturing prisoners

	struct Dungeon *dungeon;
	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;

	int i;
	float my_strength, their_strength;
	my_strength = calc_players_strength(dungeon);
	for (i = 0; i < KEEPER_COUNT; ++i)
	{
		struct Dungeon* their_dungeon;
		their_dungeon = get_dungeon(i);
		if (i == dungeon->owner || dungeon_invalid(their_dungeon))
			continue;
		if (players_are_enemies(dungeon->owner, i))
		{
			their_strength = calc_players_strength(their_dungeon);
			claim_from_player[i] = their_strength < 0.8f * my_strength;
			//SYNCLOG("%d our: %f, their: %f", dungeon->owner, my_strength, their_strength);
		}
		else
			claim_from_player[i] = 0;
	}

	struct Thing* imp;
	imp = find_imp_for_claim(comp->dungeon); //TODO: might wish to parametrize for suicide-on-traps-with-cheap-imps case which we're currently ignoring
	if (thing_is_invalid(imp))
		return CTaskRet_Unk4;

	TbBool cheap_diggers;
	cheap_diggers = is_power_available(dungeon->owner, PwrK_MKDIGGER)
		&& get_computer_money_less_cost(comp) >= 100 * compute_power_price(dungeon->owner, PwrK_MKDIGGER, 0);

	//we only try to find the "best" block and assign imp to it

	MapSlabCoord x, y;
	struct SlabMap* slab;
	long owner;
	int eval;
	int local_best, global_best;
	MapSlabCoord best_x, best_y;
	global_best = 0;
	for (y = 1; y < map_tiles_y - 1; ++y)
	{
		for (x = 1; x < map_tiles_x - 1; ++x)
		{
			slab = get_slabmap_block(x, y);
			if (!slab_kind_can_drop_here_now(slab->kind))
				continue;

			owner = slabmap_owner(slab);
			if (owner != dungeon->owner)
				continue;

			//ignore places with friendly imps, enemy creatures
			found_unconscious = 0;
			if (do_to_things_with_param_on_tile(x, y, eval_claim_check_neighbor_callback, (ModTngFilterParam)dungeon) != 0)
				continue;

			eval = eval_claim_check_neighbor(x, y - 1, dungeon, cheap_diggers);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = eval;

			eval = eval_claim_check_neighbor(x - 1, y, dungeon, cheap_diggers);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = max(local_best, eval);

			eval = eval_claim_check_neighbor(x, y + 1, dungeon, cheap_diggers);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = max(local_best, eval);

			eval = eval_claim_check_neighbor(x + 1, y, dungeon, cheap_diggers);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = max(local_best, eval);

			if (found_unconscious && player_creature_tends_to(dungeon->owner, CrTend_Imprison))
				local_best += 500;

			//check for enemy creatures in a bit wider area if we reached this far
			if (!thing_is_invalid(get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(
					COORD_PER_STL * (x * STL_PER_SLB + 1), COORD_PER_STL * (y * STL_PER_SLB + 1), 21, dungeon->owner)))
				continue;

			if (local_best > global_best)
			{
				global_best = local_best;
				best_x = x;
				best_y = y;
			}
		}
	}

	if (global_best > 0)
	{
		//try to drop imp
		if (!create_task_move_creature_to_subtile(comp, imp,
			best_x * STL_PER_SLB + 1, best_y * STL_PER_SLB + 1, CrSt_ImpConvertsDungeon))
			return CTaskRet_Unk4;

		SYNCDBG(18, "Dropped imp to claim for %d", dungeon->owner);
		return CTaskRet_Unk1;
	}

	return CTaskRet_Unk4;
}

// CHECK FOR IMPRISON TENDENCY ////////////////////////////////////////////////////////////////////

long computer_check_for_imprison_tendency(struct Computer2* comp)
{
	//small check to dynamically toggle imprison tendency based on available prison space

	struct Dungeon *dungeon;
	TbBool current_state, desired_state;
	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;

	current_state = player_creature_tends_to(dungeon->owner, CrTend_Imprison);

	if (dungeon_has_room(dungeon, RoK_PRISON))
	{
		long total, used;
		get_room_kind_total_and_used_capacity(dungeon, RoK_PRISON, &total, &used);
		desired_state = used < total;
	}
	else
		desired_state = 0;

	if (current_state != desired_state)
	{
		//ugly and breaks encapsulation but GA_SetTendencies expects final mask
		unsigned char new_tendencies;
		new_tendencies = dungeon->creature_tendencies ^ 0x01;

		if (try_game_action(comp, dungeon->owner, GA_SetTendencies, 0, 0, 0, new_tendencies, 0) == Lb_OK)
			return CTaskRet_Unk1;
	}

	return CTaskRet_Unk4;
}

//CHECK PRISON MANAGEMENT /////////////////////////////////////////////////////////////////////////

enum PrisonManageAction
{
	PMA_Nothing,
	PMA_Heal,
	PMA_Torture,
	PMA_Kill,
};

struct PrisonManageSearch
{
	int captor;
	struct Thing* best_thing;
	long best_priority;
	enum PrisonManageAction best_action;
	TbBool can_torture;
	TbBool can_heal;
};

static void search_list_for_good_prison_manage_action(short list_start, struct Dungeon* victim, struct PrisonManageSearch* search, TbBool should_heal_for_torture)
{
	struct Dungeon* captor_dungeon;
	long i;
	int k;
	k = 0;
	i = list_start;
	captor_dungeon = get_dungeon(search->captor);
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
		if (creature_is_kept_in_prison(thing)
				&& !creature_is_being_dropped(thing)
				&& can_thing_be_picked_up_by_player(thing, search->captor)) {
			struct CreatureStats* crtrstats;
			long priority;
			int max_health;
			enum PrisonManageAction action;
			action = PMA_Nothing;
			crtrstats = creature_stats_get_from_thing(thing);
			priority = get_creature_thing_score(thing);
			max_health = compute_creature_max_health(crtrstats->health, cctrl->explevel);

			if (thing->health * 10 >= max_health * 9)
			{
				if (search->can_torture)
				{
					action = PMA_Torture;
					priority *= 2;
				}
				else
				{
					action = PMA_Kill;
				}
			}
			else if (cctrl->instance_available[CrInst_HEAL])
			{
				if (!creature_requires_healing(thing) //it will not heal self otherwise
					&& search->can_torture)
				{
					if (search->can_heal)
					{
						action = PMA_Heal;
					}
					else
					{
						action = PMA_Torture;
						priority *= 2;
					}
				}
			}
			else if (!crtrstats->humanoid_creature) //TODO: could add || for creature we want to keep anyway, better than skeleton
			{
				if (!search->can_torture)
				{
					action = PMA_Kill;
				}
				else if (!should_heal_for_torture || !search->can_heal)
				{
					action = PMA_Torture;
					priority *= 2;
				}
				else
				{
					action = PMA_Heal;
				}
			}

			if (action == PMA_Nothing && !crtrstats->humanoid_creature && !dungeon_has_room(captor_dungeon, RoK_TORTURE))
			{
				action = PMA_Kill;
				if (cctrl->instance_available[CrInst_HEAL])
					priority /= 4;
			}

			if (action == PMA_Kill && thing->health * 7 < max_health) //always prioritize easy killing to make room in dungeon, starting with low level creatures
				priority = INT_MAX / 2 - priority;

			if (action != PMA_Nothing && priority > search->best_priority)
			{
				search->best_priority = priority;
				search->best_thing = thing;
				search->best_action = action;
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return;
		}
	}
}

long computer_check_prison_management(struct Computer2* comp)
{
	//check to manage creatures in prison: heal, transfer to torture chamber
	//TODO: could add transfer to better prison if prison is in danger
	//TODO: could heal tortured prisoners
	
	struct Dungeon *dungeon;
	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;

	if (!dungeon_has_room(dungeon, RoK_PRISON))
		return CTaskRet_Unk4;

	struct PrisonManageSearch search;
	TbBool can_feed_chicken, can_heal_spell;

	can_heal_spell = get_computer_money_less_cost(comp) > 10000;
	can_feed_chicken = 0;
	if (dungeon_has_room(dungeon, RoK_GARDEN))
	{
		long total, used;
		get_room_kind_total_and_used_capacity(dungeon, RoK_GARDEN, &total, &used);
		can_feed_chicken = used * 2 >= total;
	}

	search.can_heal = can_feed_chicken || can_heal_spell;
	search.can_torture = 0;
	if (dungeon_has_room(dungeon, RoK_TORTURE))
	{
		long total, used;
		get_room_kind_total_and_used_capacity(dungeon, RoK_TORTURE, &total, &used);
		search.can_torture = used < total;
	}

	search.captor = dungeon->owner;
	search.best_thing = INVALID_THING;
	search.best_priority = INT_MIN;
	search.best_action = PMA_Nothing;
	int i;
	for (i = 0; i < PLAYERS_COUNT; ++i)
	{
		if (i == dungeon->owner || !player_exists(get_player(i)) || !players_are_enemies(dungeon->owner, i))
			continue;

		struct Dungeon* their_dungeon;
		their_dungeon = get_dungeon(i);
		if (dungeon_invalid(their_dungeon))
			continue;

		//go through all creatures of enemy player, see which best to take action for
		search_list_for_good_prison_manage_action(their_dungeon->creatr_list_start, their_dungeon, &search, 1);
		search_list_for_good_prison_manage_action(their_dungeon->digger_list_start, their_dungeon, &search, 0);
	}

	if (!thing_is_invalid(search.best_thing))
	{
		SYNCDBG(18, "Prison management action for player %d: %d", dungeon->owner, search.best_action);

		struct Room* room;

		perform_action:
		switch (search.best_action)
		{
		case PMA_Torture:
			room = find_room_with_spare_capacity(dungeon->owner, RoK_TORTURE, 1);
			if (!room_is_invalid(room))
			{
				MapSlabCoord x, y;
				i = room->slabs_list;
				x = slb_num_decode_x(i);
				y = slb_num_decode_y(i);
				if (create_task_move_creature_to_subtile(comp, search.best_thing,
						x * STL_PER_SLB + 1, y * STL_PER_SLB + 1, CrSt_Torturing))
					return CTaskRet_Unk1;
			}
			break;
		case PMA_Heal:
			if (can_feed_chicken && can_heal_spell && is_digging_any_gems(dungeon))
				can_feed_chicken = ACTION_RANDOM(6) != 0; //use heal spell sometimes

			if (can_feed_chicken)
			{
				struct Thing* chicken;
				chicken = find_any_chicken(dungeon);
				if (!thing_is_invalid(chicken))
				{
					MapSubtlCoord x, y;
					x = search.best_thing->mappos.x.stl.num;
					y = search.best_thing->mappos.y.stl.num;
					if (ACTION_RANDOM(3) == 0)
					{
						//random offset for amusement
						x += ACTION_RANDOM(3) - 1;
						y += ACTION_RANDOM(3) - 1;
					}
					if (create_task_move_creature_to_subtile(comp, chicken, x, y, 0))
						return CTaskRet_Unk1;
				}
				else
				{
					WARNLOG("Hatchery full despite there being no chickens");
					//workaround for bug where hatchery is bugged and entirely at full capacity despite no chickens
					if (can_heal_spell)
						can_feed_chicken = 0;
					else
						search.best_action = search.can_torture? PMA_Torture : PMA_Kill;
					goto perform_action;
				}
			}
			else
			{
				if (try_game_action(comp, dungeon->owner, GA_UsePwrHealCrtr, SPELL_MAX_LEVEL, 0, 0, search.best_thing->index, 0) > Lb_OK)
					return CTaskRet_Unk1;
			}
			break;
		case PMA_Kill:
			if (can_cast_spell(dungeon->owner, PwrK_SLAP, search.best_thing->mappos.x.stl.num, search.best_thing->mappos.y.stl.num, search.best_thing, CastChk_Default)
					&& try_game_action(comp, dungeon->owner, GA_UsePwrSlap, 0, 0, 0, search.best_thing->index, 0) > Lb_OK)
				return CTaskRet_Unk2;
			break;
		case PMA_Nothing: //avoid untested enum warning
			break;
		}
	}

	return CTaskRet_Unk4;
}

//CHECK NEW DIGGING ///////////////////////////////////////////////////////////////////////////////

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
	struct ExpandRoomPos room_pos;
	int room_score;
};

struct DigToGold
{
	TbBool active;
	MapSlabCoord target_x, target_y;
	MapSlabCoord access_x, access_y;
};

struct DigToAttack
{
	TbBool active;
};

struct DigToSecure
{
	TbBool active;
};

#define CONCURRENT_GOLD_DIGS	10

struct Digging
{
	struct ExpandRoom expand_room;
	struct DigToGold dig_gold[CONCURRENT_GOLD_DIGS];
	struct DigToAttack dig_attack;
	struct DigToSecure dig_secure;
	unsigned char marked_for_dig[85][85]; //need instant lookup, so maintaining additional struct. [y][x] in case we move to ptr later
};

static struct Digging comp_digging[KEEPER_COUNT];

static TbBool marked_for_dig_and_undug(struct Digging* digging, MapSlabCoord x, MapSlabCoord y)
{
	struct SlabMap* slab;

	if (!digging->marked_for_dig[y][x])
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
		return 1;
	default:
		return 0;
	}
}

void computer_setup_new_digging(void)
{
	int plyr_idx;
	memset(&comp_digging, 0, sizeof(comp_digging));

	for (plyr_idx = 0; plyr_idx < KEEPER_COUNT; ++plyr_idx)
	{
		struct Dungeon *dungeon;
		struct MapTask *mtask;
		long i,max;
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
				MapSubtlCoord x, y;
				x = stl_num_decode_x(mtask->coords);
				y = stl_num_decode_y(mtask->coords);
				comp_digging[plyr_idx].marked_for_dig[subtile_slab(y)][subtile_slab(x)] = 1;
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

static TbResult dig_if_needed(struct Computer2* comp, struct Digging* digging, MapSlabCoord x, MapSlabCoord y)
{
	if (digging->marked_for_dig[y][x])
		return Lb_OK;

	struct SlabMap* slab;
	TbResult result;

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
		result = try_game_action(comp, comp->dungeon->owner, GA_MarkDig, 0,
			x * STL_PER_SLB + 1, y * STL_PER_SLB + 1, 1, 1);
		if (result != Lb_FAIL)
			digging->marked_for_dig[y][x] = 1;
		return result;
	default:
		return Lb_OK;
	}
}

static void process_dig_to_attack(struct Computer2* comp, struct Digging* digging)
{

}

static void check_dig_to_attack(struct Computer2* comp, struct Digging* digging)
{

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

static void try_get_closer(int player, int* best_dist, MapSlabCoord* best_x, MapSlabCoord* best_y, MapSlabCoord x, MapSlabCoord y)
{
	int dist;
	struct SlabInfluence* influence;
	influence = get_slab_influence(x, y);
	dist = influence->dig_distance[player];
	if (dist >= 0 && dist < *best_dist)
	{
		*best_dist = dist;
		*best_x = x;
		*best_y = y;
	}
}

static void initiate_dig_gold(struct Computer2* comp, struct Digging* digging, struct DigToGold* dig_gold)
{
	MapSlabCoord x, y;
	struct Dungeon* dungeon;
	struct SlabInfluence* influence;
	TbBool abort;
	dungeon = comp->dungeon;

	//back track to own territory //TODO: use A* instead so that we can e.g. prefer not to wreck our walls
	x = dig_gold->target_x;
	y = dig_gold->target_y;
	abort = 0;
	for (;;)
	{
		int best_dist;
		int current_dist;
		MapSlabCoord best_x, best_y;

		if (!digging->marked_for_dig[y][x] && dig_if_needed(comp, digging, x, y) == Lb_FAIL)
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
		try_get_closer(dungeon->owner, &best_dist, &best_x, &best_y, x - 1, y);
		try_get_closer(dungeon->owner, &best_dist, &best_x, &best_y, x + 1, y);
		try_get_closer(dungeon->owner, &best_dist, &best_x, &best_y, x, y - 1);
		try_get_closer(dungeon->owner, &best_dist, &best_x, &best_y, x, y + 1);

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

	if (abort)
	{
		SYNCLOG("Player %d aborted gold dig attempt at %dx%d", (int)dungeon->owner, dig_gold->target_x, dig_gold->target_y);
	}
	else
	{
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

			if (digging->marked_for_dig[y][x])
				continue;

			influence = get_slab_influence(x, y);
			if (influence->dig_distance[dungeon->owner] < 0)
				continue; //don't bother evaluating blocks we can't reach

			//TODO: disqualify blocks that would open up dangerous areas we can't handle

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

			score /= 5 + influence->dig_distance[dungeon->owner];

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
	}
}

static RoomKind decide_room_to_expand(struct Computer2* comp)
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

		long used_capacity;
		long total_capacity;
		get_room_kind_total_and_used_capacity(dungeon, bldroom->rkind, &total_capacity, &used_capacity);
		if (total_capacity > 0) {
			continue;
		}
		long free_capacity;
		free_capacity = computer_get_room_kind_free_capacity(comp, bldroom->rkind);
		if (free_capacity == 9999)
		{
			if (bldroom->rkind == RoK_GARDEN) {
				continue;
			}
		} else
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
		return digging->marked_for_dig[y][x]? 5 : 20; //treated as claimed if will be dig
	case SlbT_ROCK:
	case SlbT_GEMS:
		return 15;
	case SlbT_CLAIMED:
		return 5; //traps are good, right
	case SlbT_GOLD:
		return digging->marked_for_dig[y][x]? 5 : 1; //better than nothing, avoids opening
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

static int eval_expand_room_pos(struct Dungeon* dungeon, struct Digging* digging, struct ExpandRoom* expand, struct ExpandRoomPos* pos)
{
	struct SlabMap* slab;
	MapSlabCoord x, y;
	int score;
	int num_tiles;

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
			if (marked_for_dig_and_undug(digging, x, y)) //in case of multiple expansions
				return INT_MIN;

			//if outside walls or otherwise not treasure chamber, demand access (TODO: see if any other rooms worth having non-accessible interiors for)
			if (expand->rkind != RoK_TREASURE ||
				x == pos->min_x || x == pos->max_x || y == pos->min_y || y == pos->max_y)
			{
				if (!is_diggable_or_buildable(dungeon, digging, x, y)) //don't want overlapping expansions
					return INT_MIN;
			} //TODO: deduce points for non-conquerable interior in case where it passes through

			//give score
			num_tiles += 1;
			if (num_tiles < expand->preferred_size)
				score += 1000;

			struct SlabMap* slab;
			slab = get_slabmap_block(x, y);
			switch (slab->kind)
			{
			case SlbT_EARTH:
				score -= 1; //to prefer claimed
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

			//TODO: reduce score for unreinforcable walls

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

	//TODO: penalize opening dungeon to dangerous area

	int w, h, d;
	w = pos->max_x - pos->min_x + 1;
	h = pos->max_y - pos->min_y + 1;
	d = abs(w - h);
	if (d > 1)
		score -= 200 * (d - 1) * max(w, h);
	else if (d == 1 && min(w, h) <= 2) //guiding heuristic to avoid sweeping around long useless rooms
		score -= 200;

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

static int eval_expand_room(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand, MapSlabCoord x, MapSlabCoord y, MapSlabCoord dx, MapSlabCoord dy)
{
	struct Dungeon* dungeon;
	struct ExpandRoomPos best_pos;
	int best_score;
	int i;
	TbBool changed;

	dungeon = comp->dungeon;

	//find first tile we can expand on
	for (;;)
	{
		struct SlabMap* slab;
		slab = get_slabmap_block(x, y);
		if (!slab_kind_can_drop_here_now(slab->kind) || slab->kind == SlbT_CLAIMED)
			break;

		x += dx;
		y += dy;
	}

	best_pos.min_x = best_pos.max_x = best_pos.access_x = x;
	best_pos.min_y = best_pos.max_y = best_pos.access_y = y;
	best_pos.access_dx = dx;
	best_pos.access_dy = dy;
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

		//try to translate in any position
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

	//SYNCLOG("best WxH for room: %dx%d", best_pos.max_x - best_pos.min_x + 1, best_pos.max_y - best_pos.min_y + 1);

	//SYNCLOG("quitting after %d iterations with score %d", i, best_score);
	if (best_pos.max_x - best_pos.min_x < 2 || best_pos.max_y - best_pos.min_y < 2)
	{
		best_score = INT_MIN;
	}

	if (best_score > expand->room_score)
	{
		expand->room_score = best_score;
		memcpy(&expand->room_pos, &best_pos, sizeof(best_pos));
	}

	return best_score;
}

static TbBool find_expand_location(struct Computer2* comp, struct Digging* digging, struct ExpandRoom* expand)
{
	struct Dungeon *dungeon;
	struct Room* room;
	long i;
	RoomKind rkind;
	unsigned long k;

	SYNCDBG(18, "Starting");
	dungeon = comp->dungeon;
	expand->room_score = INT_MIN;

	//check room sides
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
				MapSlabCoord x, y;
				x = subtile_slab(room->central_stl_x);
				y = subtile_slab(room->central_stl_y);
				//SYNCLOG("evaluating expand %d %d", x, y);
				eval_expand_room(comp, digging, expand, x, y, -1, 0);
				eval_expand_room(comp, digging, expand, x, y, 1, 0);
				eval_expand_room(comp, digging, expand, x, y, 0, -1);
				eval_expand_room(comp, digging, expand, x, y, 0, 1);
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
		SYNCLOG("Player %d decided to build room %s of size %dx%d", (int)dungeon->owner, room_code_name(expand->rkind),
			(expand->room_pos.max_x - expand->room_pos.min_x + 1), (expand->room_pos.max_y - expand->room_pos.min_y + 1));
		expand->active = 1;
	}
}

static void check_expand_room(struct Computer2* comp, struct Digging* digging)
{
	RoomKind rkind;
	struct Dungeon *dungeon;
	SYNCDBG(8,"Starting");
	dungeon = comp->dungeon;

	rkind = decide_room_to_expand(comp);
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
