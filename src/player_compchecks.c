/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_compchecks.c
 *     Computer player checks definitions and routines.
 * @par Purpose:
 *     Defines a computer player checks and related functions.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 06 Jan 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_computer.h"

#include <limits.h>
#include <string.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"

#include "config.h"
#include "config_terrain.h"
#include "player_instances.h"
#include "creature_states.h"
#include "creature_states_mood.h"
#include "spdigger_stack.h"
#include "magic.h"
#include "map_utils.h"
#include "dungeon_data.h"
#include "room_data.h"
#include "ariadne_wallhug.h"
#include "power_hand.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);

/******************************************************************************/
const struct NamedCommand computer_check_func_type[] = {
  {"checks_hates",            1,},
  {"check_move_to_best_room", 2,},
  {"check_move_to_room",      3,},
  {"check_no_imps",           4,},
  {"check_for_pretty",        5,},
  {"check_for_quick_attack",  6,},
  {"check_for_accelerate",    7,},
  {"check_slap_imps",         8,},
  {"check_enemy_entrances",   9,},
  {"check_for_place_door",   10,},
  {"check_neutral_places",   11,},
  {"check_for_place_trap",   12,},
  {"check_for_expand_room",  13,},
  {"check_for_money",        14,},
  {"none",                   15,},
  {NULL,                      0,},
};

Comp_Check_Func computer_check_func_list[] = {
  NULL,
  computer_checks_hates,
  computer_check_move_creatures_to_best_room,
  computer_check_move_creatures_to_room,
  computer_check_no_imps,
  computer_check_for_pretty,
  computer_check_for_quick_attack,
  computer_check_for_accelerate,
  computer_check_slap_imps,
  computer_check_enemy_entrances,
  computer_check_for_place_door,
  computer_check_neutral_places,
  computer_check_for_place_trap,
  computer_check_for_expand_room,
  computer_check_for_money,
  NULL,
  NULL,
};

struct ExpandRooms expand_rooms[] = {
  {RoK_TREASURE,  45},
  {RoK_LAIR,      45},
  {RoK_GARDEN,    45},
  {RoK_LIBRARY,   45},
  {RoK_TRAINING,  35},
  {RoK_WORKSHOP,  45},
  {RoK_SCAVENGER, 30},
  {RoK_PRISON,    30},
  {RoK_TEMPLE,    25},
  {RoK_TORTURE,   35},
  {RoK_GRAVEYARD, 30},
  {RoK_BARRACKS,  35},
  {RoK_NONE,       0},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *compdngn;
    SYNCDBG(8,"Starting");
    compdngn = comp->dungeon;
    // Reference values for checking hate
    int cdngn_creatrs, cdngn_spdiggrs, cdngn_enrancs;
    cdngn_creatrs = count_creatures_in_dungeon(compdngn);
    cdngn_spdiggrs = count_diggers_in_dungeon(compdngn);
    cdngn_enrancs = count_entrances(comp, compdngn->owner);
    // Now check hate for every player
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo *player;
        struct Dungeon *dungeon;
        struct OpponentRelation *oprel;
        player = get_player(i);
        dungeon = get_players_dungeon(player);
        oprel = &comp->opponent_relations[i];
        if (!player_exists(player) || (player->id_number == compdngn->owner)
         || (player->id_number == game.neutral_player_num))
            continue;
        if (player->field_2C != 1)
            continue;
        if (players_are_mutual_allies(compdngn->owner, i))
            continue;
        int hdngn_creatrs, hdngn_spdiggrs, hdngn_enrancs;
        int hate_reasons;
        hate_reasons = 0;
        hdngn_creatrs = count_creatures_in_dungeon(dungeon);
        hdngn_spdiggrs = count_diggers_in_dungeon(dungeon);
        // Computers hate players who have more creatures than them
        if (hdngn_creatrs >= cdngn_creatrs)
        {
            hate_reasons++;
            oprel->hate_amount++;
        }
        // Computers hate players who have more special diggers than them
        if (cdngn_spdiggrs / 6 + cdngn_spdiggrs < hdngn_spdiggrs)
        {
            hate_reasons++;
            oprel->hate_amount++;
        }
        // Computers hate players who can build more rooms than them
        if (((int)compdngn->total_rooms + (int)compdngn->total_rooms / 6) < (int)dungeon->total_rooms)
        {
            hate_reasons++;
            oprel->hate_amount++;
        }
        // Computers highly hate players who claimed more entrances than them
        hdngn_enrancs = count_entrances(comp, i);
        if (hdngn_enrancs > cdngn_enrancs)
        {
            hate_reasons++;
            oprel->hate_amount += 5;
        }
        // If no reason to hate the player - hate him randomly for just surviving that long
        if ((hate_reasons <= 0) && (check->param1 < game.play_gameturn))
        {
            if (ACTION_RANDOM(100) < 20) {
                oprel->hate_amount++;
            }
        }
    }
    return CTaskRet_Unk4;
}

long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    SYNCDBG(8,"Starting for player %d",(int)dungeon->owner);
    int num_to_move;
    num_to_move = check->param1 * dungeon->num_active_creatrs / 100;
    if (num_to_move <= 0) {
        SYNCDBG(8,"No player %d creatures to move, active %d percentage %d",
            (int)dungeon->owner,(int)dungeon->num_active_creatrs,(int)check->param1);
        return CTaskRet_Unk4;
    }
    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, num_to_move)) {
        return CTaskRet_Unk4;
    }
    // If there's already task in progress which uses hand, then don't add more
    // content of the hand could be used by wrong task by mistake
    if (is_task_in_progress_using_hand(comp)) {
        return CTaskRet_Unk4;
    }
    if (!create_task_move_creatures_to_room(comp, 0, num_to_move)) {
        return CTaskRet_Unk4;
    }
    SYNCDBG(8,"Added player %d task to move %d creatures to best room",(int)dungeon->owner,(int)num_to_move);
    return CTaskRet_Unk1;
}

long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    struct Room *room;
    dungeon = comp->dungeon;
    SYNCDBG(8,"Checking player %d for move to %s", (int)dungeon->owner, room_code_name(check->param2));
    int num_to_move;
    num_to_move = check->param1 * dungeon->num_active_creatrs / 100;
    if (num_to_move <= 0) {
        SYNCDBG(8,"No creatures to move, active %d percentage %d", (int)dungeon->num_active_creatrs, (int)check->param1);
        return CTaskRet_Unk4;
    }
    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, num_to_move)) {
        return CTaskRet_Unk4;
    }
    // If there's already task in progress which uses hand, then don't add more
    // content of the hand could be used by wrong task by mistake
    if (is_task_in_progress_using_hand(comp)) {
        return CTaskRet_Unk4;
    }
    unsigned long k;
    long i;
    k = 0;
    i = dungeon->room_kind[check->param2];
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
        if (room->total_capacity > room->used_capacity)
        {
            int num_to_move_fit;
            num_to_move_fit = min(num_to_move, room->total_capacity - room->used_capacity);
            if (create_task_move_creatures_to_room(comp, room->index, num_to_move_fit)) {
                SYNCDBG(8,"Added task to move %d creatures to %s index %d", (int)num_to_move_fit,room_code_name(room->kind),(int)room->index);
                return CTaskRet_Unk1;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return CTaskRet_Unk4;
}

/**
 * Returns whether special diggers in given dungeon are actually digging indestructible valuables.
 * In standard configuration, indestructible valuables are simply slabs with gems.
 * @param dungeon
 * @return
 */
static TbBool any_digger_is_digging_indestructible_valuables(struct Dungeon *dungeon)
{
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

			if ((state_type == CrStTyp_Work)
				&& (cctrl->digger.last_did_job == SDLstJob_DigOrMine)
				&& is_digging_indestructible_place(thing))
			{
				SYNCDBG(18, "Indestructible valuables being dug by player %d", (int)dungeon->owner);
				return true;
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return false;
		}
	}
	SYNCDBG(18, "Indestructible valuables NOT being dug by player %d", (int)dungeon->owner);
	return false;
}

/**
 * Filter function for selecting creature which is best candidate for being sacrificed.
 * A specific thing can be selected either by class, model and owner.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with specific thing which is dragged.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_best_for_sacrifice(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);

    if ((cctrl->combat_flags == 0) && (param->num2 || thing->creature.gold_carried == 0)) //no gold carried if no gem access
    {
        if (creature_is_being_unconscious(thing) || creature_affected_by_spell(thing, SplK_Chicken))
            return -1;
        if (creature_is_being_dropped(thing) || !can_thing_be_picked_up_by_player(thing, param->plyr_idx))
            return -1;
        if ((param->plyr_idx >= 0) && (thing->owner != param->plyr_idx))
            return -1;
        if ((param->model_id > 0) && (thing->model != param->model_id))
            return -1;
        if ((param->class_id > 0) && (thing->class_id != param->class_id))
            return -1;
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(thing);
        long priority;
        // Let us estimate value of the creature in gold
        priority = thing->creature.gold_carried; // base value
        priority += param->num1 * thing->health / crstat->health; // full health valued at this many gold
        priority += 10000 * cctrl->explevel; // experience earned by the creature has a big value
        if (get_creature_state_type(thing) == CrStTyp_Work)
            priority += 500; // aborted work valued at this many gold
        if (anger_is_creature_angry(thing))
            priority /= 2; // angry creatures have lower value
        if (anger_is_creature_livid(thing))
            priority /= 3; // livid creatures have minimal value
         // Return maximizer based on our evaluated gold value
        return LONG_MAX - priority;
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

static struct Thing *find_creature_for_sacrifice(struct Computer2 *comp, ThingModel crmodel)
{
	struct Dungeon *dungeon;
    dungeon = comp->dungeon;

    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    param.plyr_idx = dungeon->owner;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.num1 = compute_lowest_power_price(dungeon->owner, PwrK_MKDIGGER, 0);
    param.num2 = any_digger_is_digging_indestructible_valuables(dungeon);
    filter = player_list_creature_filter_best_for_sacrifice;
    TbBool is_spec_digger;
    is_spec_digger = false;
    if (crmodel > 0) {
        //TODO DIGGERS For now, only player-specific special diggers are on the diggers list
        is_spec_digger = (crmodel == get_players_special_digger_model(dungeon->owner));
        //struct CreatureModelConfig *crconf;
        //crconf = &crtr_conf.model[crmodel];
        //is_spec_digger = ((crconf->model_flags & MF_IsSpectator) != 0);
    }
    struct Thing *thing;
    thing = INVALID_THING;
    if ((is_spec_digger) || (crmodel == -1))
    {
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
    }
    if (((!is_spec_digger) || (crmodel == -1)) && thing_is_invalid(thing))
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    return thing;
}

/**
 * Checks if a computer player can sacrifice imps to reduce price.
 * @param comp
 * @param check
 */
long computer_check_sacrifice_for_cheap_diggers(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    if (gameadd.cheaper_diggers_sacrifice_model == 0) {
        return CTaskRet_Unk0;
    }

    GoldAmount power_price, lowest_price;
	power_price = compute_power_price(dungeon->owner, PwrK_MKDIGGER, 0);
	lowest_price = compute_lowest_power_price(dungeon->owner, PwrK_MKDIGGER, 0);
	SYNCDBG(18, "Digger creation power price: %d, lowest: %d", power_price, lowest_price);

	if ((power_price > lowest_price) && !is_task_in_progress_using_hand(comp)
		&& computer_able_to_use_power(comp, PwrK_MKDIGGER, 0, 2) //TODO COMPUTER_PLAYER add amount of imps to afford to the checks config params
		&& dungeon_has_room(dungeon, RoK_TEMPLE))
	{
		struct Thing* creatng;
		creatng = find_creature_for_sacrifice(comp, gameadd.cheaper_diggers_sacrifice_model);
		if (!thing_is_invalid(creatng))
		{
			long dist;
			struct Room* room;
			room = find_room_nearest_to_position(dungeon->owner, RoK_TEMPLE, &creatng->mappos, &dist);
			if (!room_is_invalid(room))
			{
				if (create_task_move_creature_to_subtile(comp, creatng, room->central_stl_x, room->central_stl_y, CrSt_CreatureSacrifice))
					return CTaskRet_Unk1;
			}
		}
	}
    return CTaskRet_Unk4;
}

/**
 * Checks if a computer player has not enough imps.
 * @param comp
 * @param check The check structure; param1 is preferred amount of imps, param2 is minimal amount.
 */
long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    // TODO COMPUTER_PLAYER create a separate check for imps sacrificing diggers
    if (computer_check_sacrifice_for_cheap_diggers(comp, check) == CTaskRet_Unk1) {
        return CTaskRet_Unk1;
    }
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    long controlled_diggers;
    controlled_diggers = dungeon->num_active_diggers - count_player_diggers_not_counting_to_total(dungeon->owner);
    if (controlled_diggers >= check->param1) {
        return CTaskRet_Unk4;
    }
    TbBool able_to_use_power;
    if (controlled_diggers >= check->param2) {
        // We have less than preferred amount, but higher than minimal; allow building if we've got spare money
        able_to_use_power = computer_able_to_use_power(comp, PwrK_MKDIGGER, 0, 3 + (controlled_diggers - check->param2)/4);
    } else {
        able_to_use_power = computer_able_to_use_power(comp, PwrK_MKDIGGER, 0, 1);
    }
    if (able_to_use_power)
    {
        struct Thing *heartng;
        MapSubtlCoord stl_x, stl_y;
        heartng = get_player_soul_container(dungeon->owner);
        stl_x = heartng->mappos.x.stl.num;
        stl_y = heartng->mappos.y.stl.num;
        if (xy_walkable(stl_x, stl_y, dungeon->owner))
        {
            if (try_game_action(comp, dungeon->owner, GA_UseMkDigger, 0, stl_x, stl_y, 1, 1) > Lb_OK) {
                return CTaskRet_Unk1;
            }
        }
        return CTaskRet_Unk1;
    }
    return CTaskRet_Unk0;
}

struct Thing * find_imp_for_pickup(struct Computer2 *comp, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Dungeon *dungeon;
    int pick1_dist;
    struct Thing *pick1_tng;
    int pick2_dist;
    struct Thing *pick2_tng;
    dungeon = comp->dungeon;
    pick1_dist = INT_MAX;
    pick2_dist = INT_MAX;
    pick2_tng = INVALID_THING;
    pick1_tng = INVALID_THING;
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
                    MapSubtlDelta dist;
                    long state_type;
                    dist = abs(stl_x - (MapSubtlDelta)thing->mappos.x.stl.num) + abs(stl_y - (MapSubtlDelta)thing->mappos.y.stl.num);
                    state_type = get_creature_state_type(thing);
                    if (state_type == CrStTyp_Work)
                    {
                        if (dist < pick1_dist)
                        {
                            pick1_dist = dist;
                            pick1_tng = thing;
                        }
                    }
                    else
                    {
                        if (dist < pick2_dist)
                        {
                            pick2_dist = dist;
                            pick2_tng = thing;
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
          break;
        }
    }
    if (!thing_is_invalid(pick2_tng)) {
        return pick2_tng;
    } else {
        return pick1_tng;
    }
}

long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    dungeon = comp->dungeon;
    MapSubtlCoord stl_x, stl_y;
    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, 1)) {
        return CTaskRet_Unk4;
    }
    {
        long stack_len;
        stack_len = dungeon->digger_stack_length;
        if (stack_len <= check->param1 * dungeon->total_area / 100) {
            return CTaskRet_Unk4;
        }
        long n;
        n = find_in_imp_stack_starting_at(DigTsk_ImproveDungeon, ACTION_RANDOM(stack_len), dungeon);
        if (n < 0) {
            return CTaskRet_Unk4;
        }
        const struct DiggerStack *dstack;
        dstack = &dungeon->digger_stack[n];
        stl_x = stl_num_decode_x(dstack->stl_num);
        stl_y = stl_num_decode_y(dstack->stl_num);
    }
    struct Thing * creatng;
    creatng = find_imp_for_pickup(comp, stl_x, stl_y);
    if (thing_is_invalid(creatng)) {
        return CTaskRet_Unk4;
    }
    if (!create_task_move_creature_to_subtile(comp, creatng, stl_x, stl_y, CrSt_ImpImprovesDungeon)) {
        return CTaskRet_Unk4;
    }
    return CTaskRet_Unk1;
}

struct Room *get_opponent_room(struct Computer2 *comp, PlayerNumber plyr_idx)
{
    static const RoomKind opponent_room_kinds[] = {RoK_DUNGHEART, RoK_PRISON, RoK_LIBRARY, RoK_TREASURE};
    struct Dungeon *dungeon;
    struct Room *room;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon) || (slab_conf.room_types_count < 1)) {
        return INVALID_ROOM;
    }
    int i,n;
    n = opponent_room_kinds[ACTION_RANDOM(sizeof(opponent_room_kinds)/sizeof(opponent_room_kinds[0]))];
    for (i=0; i < slab_conf.room_types_count; i++)
    {
        room = room_get(dungeon->room_kind[n]);
        if (room_exists(room)) {
            return room;
        }
        n = (n + 1) % slab_conf.room_types_count;
    }
    return INVALID_ROOM;
}

struct Room *get_hated_room_for_quick_attack(struct Computer2 *comp, long min_hate)
{
    struct THate hates[PLAYERS_COUNT];
    long i;
    SYNCDBG(8,"Starting for player %d",(int)comp->dungeon->owner);
    get_opponent(comp, hates);
    // note that 'i' is not player index, player index is inside THate struct
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct THate *hate;
        hate = &hates[i];
        if (players_are_enemies(comp->dungeon->owner, hate->plyr_idx))
        {
            if ((hate->pos_near != NULL) && (hate->amount > min_hate))
            {
                struct Room *room;
                room = get_opponent_room(comp, hate->plyr_idx);
                if (!room_is_invalid(room)) {
                    return room;
                }
            }
        }
    }
    return INVALID_ROOM;
}

/**
 * Quick attack is just putting CTA spell on enemy room.

 * @param comp
 * @param check
 */
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    dungeon = comp->dungeon;
    int creatrs_num;
    creatrs_num = check->param1 * dungeon->num_active_creatrs / 100;
    if (check->param3 >= creatrs_num) {
        return CTaskRet_Unk4;
    }
    if (!computer_able_to_use_power(comp, PwrK_CALL2ARMS, 1, 3)) {
        return CTaskRet_Unk4;
    }
    if ((check_call_to_arms(comp) != 1) || is_there_an_attack_task(comp)) {
        return CTaskRet_Unk4;
    }
    struct Room *room;
    room = get_hated_room_for_quick_attack(comp, check->param3);
    if (room_is_invalid(room)) {
        return CTaskRet_Unk4;
    }
    struct Coord3d pos;
    // TODO COMPUTER_PLAYER We should make sure the place of cast is accessible for creatures
    pos.x.val = subtile_coord_center(room->central_stl_x);
    pos.y.val = subtile_coord_center(room->central_stl_y);
    pos.z.val = subtile_coord(1,0);
    if (count_creatures_availiable_for_fight(comp, &pos) <= check->param3) {
        return CTaskRet_Unk4;
    }
    if (!create_task_magic_support_call_to_arms(comp, &pos, check->param2, 0, creatrs_num)) {
        return CTaskRet_Unk4;
    }
    SYNCLOG("Player %d decided to attack %s owned by player %d",(int)dungeon->owner,room_code_name(room->kind),(int)room->owner);
    output_message(SMsg_EnemyHarassments+ACTION_RANDOM(8), 500, 1);
    return CTaskRet_Unk1;
}

struct Thing *computer_check_creatures_in_room_for_accelerate(struct Computer2 *comp, struct Room *room)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i,n;
    dungeon = comp->dungeon;
    i = room->creatures_list;
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature %ld detected",i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (!thing_affected_by_spell(thing, SplK_Speed))
        {
            n = get_creature_state_besides_move(thing);
            struct StateInfo *stati;
            stati = get_thing_state_info_num(n);
            if (stati->state_type == CrStTyp_Work)
            {
                if (try_game_action(comp, dungeon->owner, GA_UsePwrSpeedUp, SPELL_MAX_LEVEL, 0, 0, thing->index, 0) > Lb_OK)
                {
                    return thing;
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(struct Computer2 *comp, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    struct Thing *thing;
    long i;
    unsigned long k;
    if ((rkind < 1) || (rkind > ROOM_TYPES_COUNT))
    {
        ERRORLOG("Invalid room kind %d",(int)rkind);
        return INVALID_THING;
    }
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid computer players dungeon");
        return INVALID_THING;
    }
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
        thing = computer_check_creatures_in_room_for_accelerate(comp, room);
        if (!thing_is_invalid(thing))
            return thing;
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

long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check)
{
    static RoomKind workers_in_rooms[] = {RoK_LIBRARY,RoK_LIBRARY,RoK_WORKSHOP,RoK_TRAINING,RoK_SCAVENGER};
    struct Thing *thing;
    long i,n;
    SYNCDBG(8,"Starting");
    if (!computer_able_to_use_power(comp, PwrK_SPEEDCRTR, 8, 3))
    {
        return CTaskRet_Unk4;
    }
    n = check->param1 % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    if (n <= 0)
        n = ACTION_RANDOM(sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    for (i=0; i < sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]); i++)
    {
        thing = computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(comp, workers_in_rooms[n]);
        if (!thing_is_invalid(thing))
        {
            SYNCDBG(8,"Cast on thing %d",(int)thing->index);
            return CTaskRet_Unk1;
        }
        n = (n+1) % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    }
    return CTaskRet_Unk4;
}

long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    dungeon = comp->dungeon;
    if (!is_power_available(dungeon->owner, PwrK_SLAP)) {
        return CTaskRet_Unk4;
    }
    long creatrs_num;
    creatrs_num = check->param1 * dungeon->num_active_diggers / 100;
    if (!is_task_in_progress(comp, CTT_SlapDiggers))
    {
        if (create_task_slap_imps(comp, creatrs_num)) {
            return CTaskRet_Unk1;
        }
    }
    return CTaskRet_Unk4;
}

long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    long result;
    result = CTaskRet_Unk4;
    PlayerNumber plyr_idx;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (comp->dungeon->owner == plyr_idx) {
            continue;
        }
        if (players_are_mutual_allies(comp->dungeon->owner, plyr_idx)) {
            continue;
        }
        struct PlayerInfo *player;
        struct Dungeon *dungeon;
        player = get_player(plyr_idx);
        dungeon = get_players_dungeon(player);
        long i;
        unsigned long k;
        i = dungeon->room_kind[RoK_ENTRANCE];
        k = 0;
        while (i != 0)
        {
            struct Room *room;
            room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            struct OpponentRelation *oprel;
            oprel = &comp->opponent_relations[(int)plyr_idx];
            long n;
            for (n = 0; n < 64; n++)
            {
                struct Coord3d *pos;
                pos = &oprel->pos_A[n];
                if ((pos->x.val == subtile_coord(room->central_stl_x,0)) && (pos->y.val == subtile_coord(room->central_stl_y,0))) {
                    break;
                }
            }
            if (n == 64)
            {
                struct Coord3d *pos;
                n = oprel->field_4;
                oprel->field_4 = (n + 1) % 64;
                oprel->field_0 = game.play_gameturn;
                pos = &oprel->pos_A[n];
                pos->x.val = subtile_coord(room->central_stl_x,0);
                pos->y.val = subtile_coord(room->central_stl_y,0);
                pos->z.val = subtile_coord(1,0);
                result = CTaskRet_Unk2;
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
    return result;
}

TbBool find_place_to_put_door_around_room(const struct Room *room, struct Coord3d *pos)
{
    long m,n;
    m = ACTION_RANDOM(SMALL_AROUND_SLAB_LENGTH);
    for (n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        // Get position containing room center
        MapSlabCoord slb_x,slb_y;
        slb_x = subtile_slab_fast(room->central_stl_x);
        slb_y = subtile_slab_fast(room->central_stl_y);
        // Move the position to edge of the room
        struct Room *sibroom;
        sibroom = slab_room_get(slb_x, slb_y);
        while (!room_is_invalid(sibroom) && (sibroom->index == room->index))
        {
            slb_x += small_around[m].delta_x;
            slb_y += small_around[m].delta_y;
            sibroom = slab_room_get(slb_x, slb_y);
        }
        // Move the position a few tiles further in that direction searching for a place to put door
        //TODO COMPUTER_PLAYER Why we can only have doors if corridor is at center of the room? This should be fixed to allow doors everywhere around room.
        int i;
        for (i = 4; i > 0; i--)
        {
            struct SlabMap *slb;
            slb = get_slabmap_block(slb_x, slb_y);
            if ((slabmap_owner(slb) != room->owner) || (slb->kind != SlbT_CLAIMED)) {
                i = 0;
                break;
            }
            if (tag_cursor_blocks_place_door(room->owner, slab_subtile_center(slb_x), slab_subtile_center(slb_y))) {
                break;
            }
            if (!subtile_has_door_thing_on(slab_subtile_center(slb_x), slab_subtile_center(slb_y))) {
                // No door - the position looks ok
                break;
            }
            slb_x += small_around[m].delta_x;
            slb_y += small_around[m].delta_y;
        }
        // Now if we were able to move, then the position seem ok. One last check - make sure the corridor is not dead end and doesn't already have a door
        if (i > 0)
        {
            MapSlabCoord nxslb_x,nxslb_y;
            nxslb_x = slb_x + small_around[m].delta_x;
            nxslb_y = slb_y + small_around[m].delta_y;
            struct SlabMap *nxslb;
            nxslb = get_slabmap_block(nxslb_x, nxslb_y);
            if ((slabmap_owner(nxslb) == room->owner) && (nxslb->kind == SlbT_CLAIMED))
            {
                if (!subtile_has_door_thing_on(slab_subtile_center(nxslb_x), slab_subtile_center(nxslb_y))) {
                    pos->x.val = subtile_coord_center(slab_subtile_center(slb_x));
                    pos->y.val = subtile_coord_center(slab_subtile_center(slb_y));
                    pos->z.val = subtile_coord(1,0);
                    return true;
                }
            }
        }
        m = (m + 1) % SMALL_AROUND_SLAB_LENGTH;
    }
    return false;
}

long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    ThingModel doorkind;
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    //TODO COMPUTER_PLAYER Computer prefers to put wooden doors, and uses other types only if wooden are depleted. That should be changed.
    for (doorkind = 1; doorkind < DOOR_TYPES_COUNT; doorkind++)
    {
        if (dungeon->door_amount_stored[doorkind] <= 0) {
            continue;
        }
        long rkind;
        rkind = check->param1;
        if (rkind == 0)
        {
            rkind = (check->param2 + 1) % ROOM_TYPES_COUNT;
            check->param2 = rkind;
        }
        unsigned long k;
        k = 0;
        long i;
        i = dungeon->room_kind[rkind];
        while (i != 0)
        {
            struct Room *room;
            room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            struct Coord3d pos;
            pos.x.val = 0;
            pos.y.val = 0;
            pos.z.val = 0;
            if (find_place_to_put_door_around_room(room, &pos))
            {
                if (try_game_action(comp, dungeon->owner, GA_PlaceDoor, 0, pos.x.stl.num, pos.y.stl.num, doorkind, 0) > Lb_OK) {
                    return CTaskRet_Unk1;
                }
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
    return CTaskRet_Unk4;
}

long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    struct OpponentRelation *oprel;
    oprel = &comp->opponent_relations[game.neutral_player_num];
    struct Room *near_room;
    struct Coord3d *near_pos;
    int near_dist;
    near_room = INVALID_ROOM;
    near_dist = LONG_MAX;
    near_pos = &oprel->pos_A[0];
    int i;
    for (i=0; i < COMPUTER_SPARK_POSITIONS_COUNT; i++)
    {
        struct Coord3d *place;
        place = &oprel->pos_A[i];
        if ((place->x.val == 0) || (place->y.val == 0)) {
            continue;
        }
        struct Room *room;
        room = INVALID_ROOM;
        if (computer_finds_nearest_room_to_pos(comp, &room, place))
        {
            MapSubtlDelta dx,dy;
            dx = abs((int)room->central_stl_x - (MapSubtlDelta)place->x.stl.num);
            dy = abs((int)room->central_stl_y - (MapSubtlDelta)place->y.stl.num);
            if (near_dist > dx+dy)
            {
                near_room = room;
                near_pos = place;
                near_dist = dx+dy;
            }
        }
    }
    if (room_is_invalid(near_room)) {
        return CTaskRet_Unk4;
    }
    struct Coord3d endpos;
    struct Coord3d startpos;
    endpos.x.val = near_pos->x.val;
    endpos.y.val = near_pos->y.val;
    endpos.z.val = near_pos->z.val;
    startpos.x.val = subtile_coord_center(stl_slab_center_subtile(near_room->central_stl_x));
    startpos.y.val = subtile_coord_center(stl_slab_center_subtile(near_room->central_stl_y));
    startpos.z.val = subtile_coord(1,0);
    if (!create_task_dig_to_neutral(comp, startpos, endpos)) {
        return CTaskRet_Unk4;
    }
    near_pos->x.val = 0;
    near_pos->y.val = 0;
    near_pos->z.val = 0;
    return CTaskRet_Unk1;
}

/**
 * Counts amount of slabs around given slab which have given kind and owner.
 * @param slb_x Target slab position, X coordinate.
 * @param slb_y Target slab position, Y coordinate.
 * @param slbkind Kind of the slabs to count.
 * @param owner Owner of the slabs to count.
 * @return Amount of matched slabs around given coordinates, 0..8.
 */
int count_slabs_around_of_kind(MapSlabCoord slb_x, MapSlabCoord slb_y, SlabKind slbkind, PlayerNumber owner)
{
    unsigned long n;
    int matched_slabs;
    matched_slabs = 0;
    for (n = 1; n < MID_AROUND_LENGTH; n++)
    {
        MapSlabCoord arslb_x, arslb_y;
        arslb_x = slb_x + mid_around[n].delta_x;
        arslb_y = slb_y + mid_around[n].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_block(arslb_x, arslb_y);
        if ((slb->kind == slbkind) && (slabmap_owner(slb) == owner)) {
            matched_slabs++;
        }
    }
    return matched_slabs;
}

/**
 * This function generates "expand room" action on a tile which is claimed ground and could have a room placed on.
 * It is used to fix vandalized or not fully built rooms, so that they will cover the whole area digged for them.
 *
 * @param comp
 * @param check Computer check data.
 * @param room The room to be checked for expand.
 * @param max_radius The max distance of the slab being put to the center of the room, in subtiles.
 * @param around_start Random value used for setting starting point of the check process.
 * @return
 */
TbBool computer_check_for_expand_specific_room(struct Computer2 *comp, struct ComputerCheck * check, struct Room *room, MapSubtlCoord max_radius, long around_start)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (!is_room_available(dungeon->owner, room->kind)) {
        return false;
    }
    unsigned long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x,slb_y;
        struct SlabMap *slb;
        slb = get_slabmap_direct(i);
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code
        int room_around, claimed_around;
        room_around = count_slabs_around_of_kind(slb_x, slb_y, slb->kind, dungeon->owner);
        claimed_around = 0;
        if (room_around < 8) {
            claimed_around = count_slabs_around_of_kind(slb_x, slb_y, SlbT_CLAIMED, dungeon->owner);
        }
        if (((room_around >= 3) && (claimed_around >= 2) && (room_around+claimed_around < 8)) // If there's something besides room and claimed, then grow more aggressively
         || ((room_around >= 4) && (claimed_around >= 2) && (room_around+claimed_around >= 8)) // If we're in open space, don't expand that much
         || ((room_around >= 6) && (claimed_around >= 1))) // Allow fixing one-slab holes inside rooms
        {
            unsigned long m,n;
            m = around_start % SMALL_AROUND_SLAB_LENGTH;
            for (n=0; n < SMALL_AROUND_SLAB_LENGTH; n++)
            {
                MapSlabCoord arslb_x, arslb_y;
                arslb_x = slb_x + small_around[m].delta_x;
                arslb_y = slb_y + small_around[m].delta_y;
                MapSubtlCoord arstl_x, arstl_y;
                arstl_x = slab_subtile_center(arslb_x);
                arstl_y = slab_subtile_center(arslb_y);
                long dist;
                dist = abs(room->central_stl_x - arstl_x) + abs(room->central_stl_y - arstl_y);
                if (dist <= max_radius)
                {
                    if (can_build_room_at_slab(dungeon->owner, room->kind, arslb_x, arslb_y))
                    {
                        if (try_game_action(comp, dungeon->owner, GA_PlaceRoom, 0, arstl_x, arstl_y, 1, room->kind) > Lb_OK) {
                            return true;
                        }
                    }
                }
                m = (m+1) % SMALL_AROUND_SLAB_LENGTH;
            }
        }
        // Per-slab code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    return false;
}

TbBool computer_check_for_expand_room_kind(struct Computer2 *comp, struct ComputerCheck * check, RoomKind rkind, long max_slabs, long around_start)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    {
        struct RoomStats *rstat;
        rstat = room_stats_get_for_kind(rkind);
        // If we don't have money for the room - don't even try
        // Check price for two slabs - after all, we don't want to end up having nothing
        if (2*rstat->cost >= dungeon->total_money_owned) {
            return false;
        }
    }
    MapSubtlCoord max_radius;
    // Don't allow the room to be made into long, narrow shape
    max_radius = 3 * slab_subtile(LbSqrL(max_slabs),2) / 4;
    struct Room *room;
    long i;
    unsigned long k;
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
        if ((room->slabs_count > 0) && (room->slabs_count < max_slabs)) {
            if (computer_check_for_expand_specific_room(comp, check, room, max_radius, around_start)) {
                SYNCDBG(6,"The %s index %d will be expanded",room_code_name(room->kind),(int)room->index);
                return true;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return false;
}

long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    long around_start;
    around_start = ACTION_RANDOM(119);
    // Don't work when placing rooms; we could place in an area for room by mistake
    if (is_task_in_progress(comp, CTT_PlaceRoom) || is_task_in_progress(comp, CTT_CheckRoomDug)) {
        SYNCDBG(8,"No rooms expansion - colliding task already in progress");
        return CTaskRet_Unk0;
    }
    if (computer_player_in_emergency_state(comp)) {
        SYNCDBG(8,"No rooms expansion - emergency state");
        return CTaskRet_Unk0;
    }
    if (get_computer_money_less_cost(comp) < dungeon->creatures_total_pay / 3) {
        SYNCDBG(8,"No rooms expansion - not enough money buffer");
        return CTaskRet_Unk0;
    }
    const struct ExpandRooms *expndroom;
    for (expndroom = &expand_rooms[0]; expndroom->rkind != RoK_NONE; expndroom++)
    {
        if (computer_check_for_expand_room_kind(comp, check, expndroom->rkind, expndroom->max_slabs, around_start)) {
            return CTaskRet_Unk1;
        }
    }
    SYNCDBG(8,"No rooms found for expansion");
    return CTaskRet_Unk0;
}
/******************************************************************************/
