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

#include "creature_instances.h"
#include "creature_states.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "globals.h"
#include "magic.h"
#include "map_data.h"
#include "player_computer.h"
#include "power_hand.h"
#include "slab_data.h"
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
			if (!slab_kind_can_drop_here_now(slab->kind))
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

			if (found_unconscious)
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
	long i;
	int k;
	k = 0;
	i = list_start;
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

			if (thing->health >= max_health)
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
			else if (!crtrstats->humanoid_creature && !cctrl->instance_available[CrInst_HEAL]) //TODO: could add || for creature we want to keep anyway, better than skeleton
			{
				if (!search->can_torture)
				{
					action = PMA_Kill;
				}
				else if (!should_heal_for_torture || !search->can_heal)
				{
					action = PMA_Torture;
				}
				else
				{
					action = PMA_Heal;
				}
			}

			if (action == PMA_Kill) //always prioritize killing to make room in dungeon, starting with low level creatures
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
