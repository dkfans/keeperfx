#include "player_newcomp.h"

#include "creature_states.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "globals.h"
#include "map_data.h"
#include "player_computer.h"
#include "slab_data.h"
#include "thing_list.h"
#include "thing_stats.h"

extern MapSlabCoord map_tiles_x;
extern MapSlabCoord map_tiles_y;

// CHECK FOR CLAIMS ///////////////////////////////////////////////////////////////////////////////

static TngUpdateRet eval_claim_check_neighbor_callback(struct Thing* thing, ModTngFilterParam filter)
{
	//return/param semantics basically ignored, we used our own
	struct Dungeon* dungeon;
	dungeon = (struct Dungeon*)filter;

	switch (thing->class_id)
	{
	case TCls_Trap:
		return thing->owner != dungeon->owner;
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
			return 0; //always ignore unconscious creatures
		}

		break;
	}

	return 0;
}

static TbBool claim_from_player[PLAYERS_EXT_COUNT] = { 1, 1, 1, 1, 1, 1 };
static int eval_claim_check_neighbor(MapSlabCoord x, MapSlabCoord y, struct Dungeon* my_dungeon)
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
	//using this check to pressure enemy when we have an advantage

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

		their_strength = calc_players_strength(their_dungeon);
		claim_from_player[i] = their_strength < 0.8f * my_strength;
		//SYNCLOG("%d our: %f, their: %f", dungeon->owner, my_strength, their_strength);
	}

	struct Thing* imp;
	imp = find_imp_for_claim(comp->dungeon); //TODO: might wish to parametrize for suicide-on-traps-with-cheap-imps case which we're currently ignoring
	if (thing_is_invalid(imp))
		return CTaskRet_Unk4;

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

			//ignore places with enemy traps, friendly imps, enemy creatures
			if (do_to_things_with_param_on_tile(x, y, eval_claim_check_neighbor_callback, (ModTngFilterParam)dungeon) != 0)
				continue;

			eval = eval_claim_check_neighbor(x, y - 1, dungeon);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = eval;

			eval = eval_claim_check_neighbor(x - 1, y, dungeon);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = max(local_best, eval);

			eval = eval_claim_check_neighbor(x, y + 1, dungeon);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = max(local_best, eval);

			eval = eval_claim_check_neighbor(x + 1, y, dungeon);
			if (eval < 0) //disqualify tiles with neighbors that have imps or traps
				continue;
			local_best = max(local_best, eval);

			//check for enemy creatures in a bit wider area if we reached this far
			//if (!thing_is_invalid(get_creature_near_who_is_enemy_of_and_not_specdigger(x, y, dungeon->owner)))
			//	continue;

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