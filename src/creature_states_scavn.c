/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_scavn.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "creature_states_scavn.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_sound.h"

#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_states_prisn.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "player_utils.h"
#include "player_instances.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_scavenge.h"
#include "room_entrance.h"
#include "room_lair.h"
#include "power_hand.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_can_do_scavenging(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    return (crconf->scavenge_value > 0);
}

short at_scavenger_room(struct Thing *thing)
{
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_role_for_job(Job_SCAVENGE), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    GoldAmount scavenger_cost = calculate_correct_creature_scavenging_cost(thing);
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    if (scavenger_cost >= dungeon->total_money_owned)
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToScavenge, MESSAGE_DURATION_TREASURY);
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room, Job_SCAVENGE))
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, get_continue_state_for_job(Job_SCAVENGE));
    cctrl->turns_at_job = 0;
    return 1;
}

struct Thing *get_random_fellow_not_hated_creature(struct Thing *creatng)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
    if (dungeon->num_active_creatrs <= 1)
    {
        SYNCDBG(19,"No other creatures");
        return INVALID_THING;
    }
    int n = THING_RANDOM(creatng, dungeon->num_active_creatrs - 1);
    unsigned long k = 0;
    int i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if ((n <= 0) && (thing->index != creatng->index))
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
            if (!creature_model_is_lair_enemy(crconf->lair_enemy, creatng->model))
            {
                return thing;
            }
        }
        n--;
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    return INVALID_THING;
}

short creature_being_scavenged(struct Thing *creatng)
{
    SYNCDBG(8,"Starting");
    struct Thing* fellowtng = get_random_fellow_not_hated_creature(creatng);
    if (thing_is_invalid(fellowtng)) {
        fellowtng = get_player_soul_container(creatng->owner);
    }
    if (!thing_exists(fellowtng)) {
        SYNCDBG(19,"Cannot get thing to follow");
        return 0;
    }
    struct Coord3d locpos;
    locpos.x.val = fellowtng->mappos.x.val;
    locpos.y.val = fellowtng->mappos.y.val;
    locpos.z.val = fellowtng->mappos.z.val;
    {
        int angle = (((game.play_gameturn - creatng->creation_turn) >> 6) & 7) * DEGREES_45;
        locpos.x.val += -LbSinL(angle)/128;
        locpos.y.val += LbCosL(angle)/128;
    }
    if (setup_person_move_to_coord(creatng, &locpos, NavRtF_Default) <= 0)
    {
        SYNCDBG(19,"Cannot move %s index %d to pos near %s index %d",thing_model_name(creatng),(int)creatng->index,thing_model_name(fellowtng),(int)fellowtng->index);
        return 0;
    }
    creatng->continue_state = CrSt_CreatureBeingScavenged;
    if (!S3DEmitterIsPlayingSample(creatng->snd_emitter_id, 156, 0))
        thing_play_sample(creatng, 156, NORMAL_PITCH, 0, 3, 1, 2, FULL_LOUDNESS);
    SYNCDBG(19,"Finished");
    return 1;
}

short creature_scavenged_disappear(struct Thing *thing)
{
    struct Coord3d pos;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->scavenge.job_stage--;
    if (cctrl->scavenge.job_stage > 0)
    {
      if ((cctrl->scavenge.job_stage == JobStage_ScavengedDisappearing) && (cctrl->scavenge.effect_id < PLAYERS_COUNT))
      {
          //TODO EFFECTS Verify what is wrong here - we want either effect or effect element
          create_effect(&thing->mappos, get_scavenge_effect(cctrl->scavenge.effect_id), thing->owner);
      }
      return 0;
    }
    struct Room* room = subtile_room_get(cctrl->scavenge.stl_9D_x, cctrl->scavenge.stl_9D_y);
    if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_CrScavenge))
    {
        ERRORLOG("Room %s at subtile (%d,%d) disappeared",room_role_code_name(RoRoF_CrScavenge),(int)cctrl->scavenge.stl_9D_x,(int)cctrl->scavenge.stl_9D_y);
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects);
        return -1;
    }
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        move_thing_in_map(thing, &pos);
        reset_interpolation_of_thing(thing);
        anger_set_creature_anger_all_types(thing, 0);
        if (is_my_player_number(thing->owner))
          output_message(SMsg_MinionScanvenged, 0);
        cctrl->scavenge.previous_owner = thing->owner;
        change_creature_owner(thing, cctrl->scavenge.effect_id);
        internal_set_thing_state(thing, CrSt_CreatureScavengedReappear);
        return 0;
    } else
    {
        ERRORLOG("No valid position inside %s room for %s index %d",room_code_name(room->kind),thing_model_name(thing),(int)thing->index);
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects);
        return -1;
    }
}

short creature_scavenged_reappear(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    create_effect(&thing->mappos, get_scavenge_effect(cctrl->scavenge.previous_owner), thing->owner);
    set_start_state(thing);
    return 0;
}

/**
 * Returns if a player to whom the creature belongs can afford the creature to go scavenging.
 * @param creatng
 * @return
 */
TbBool player_can_afford_to_scavenge_creature(const struct Thing *creatng)
{
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    GoldAmount scavenger_cost = calculate_correct_creature_scavenging_cost(creatng);
    return (scavenger_cost < dungeon->total_money_owned);
}

TbBool reset_scavenge_counts(struct Dungeon *dungeon)
{
    memset(dungeon->creatures_scavenging, 0, game.conf.crtr_conf.model_count);
    dungeon->scavenge_counters_turn = game.play_gameturn;
    return true;
}

TbBool can_scavenge_creature_from_pool(const struct Dungeon * dungeon, ThingModel crtr_kind)
{
    SYNCDBG(11, "Starting for creature kind %s", creature_code_name(crtr_kind));

    if (game.pool.crtr_kind[crtr_kind] <= 0) {
        return false;
    }
    if (dungeon->num_active_creatrs >= dungeon->max_creatures_attracted) {
        return false;
    }
    return true;
}

TbBool thing_is_valid_scavenge_target(const struct Thing *calltng, const struct Thing *scavtng)
{
    // Only creatures of same model can be scavenged
    if (!thing_is_creature(scavtng) || (scavtng->model != calltng->model)) {
        return false;
    }
    // Only neutral and enemy creatures can be scavenged
    if (!is_neutral_thing(scavtng))
    {
        if (!players_are_enemies(calltng->owner, scavtng->owner)) {
            return false;
        }
    }
    if (thing_is_picked_up(scavtng)) {
        return false;
    }
    if (is_thing_directly_controlled(scavtng) || creature_is_kept_in_custody(scavtng) || creature_is_leaving_and_cannot_be_stopped(scavtng)) {
        return false;
    }
    if (is_hero_thing(scavtng) && (!game.conf.rules[calltng->owner].rooms.scavenge_good_allowed)) {
        return false;
    }
    if (is_neutral_thing(scavtng) && (!game.conf.rules[calltng->owner].rooms.scavenge_neutral_allowed)) {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(scavtng);
    if (game.play_gameturn - cctrl->temple_cure_gameturn > game.conf.rules[calltng->owner].rooms.temple_scavenge_protection_turns)
    {
        return true;
    }
    return false;
}

struct Thing *select_scavenger_target(const struct Thing *calltng)
{
    struct Thing* weaktng = INVALID_THING;
    long weakpts = INT32_MAX;
    SYNCDBG(18,"Starting");
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    unsigned long k = 0;
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_valid_scavenge_target(calltng, thing))
        {
            SYNCDBG(18,"The %s index %d owner %d is valid target for %s index %d owner %d",
                thing_model_name(thing),(int)thing->index,(int)thing->owner,
                thing_model_name(calltng),(int)calltng->index,(int)calltng->owner);
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            if (game.play_gameturn - cctrl->temple_cure_gameturn > game.conf.rules[calltng->owner].rooms.temple_scavenge_protection_turns)
            {
                long thingpts = calculate_correct_creature_scavenge_required(thing, calltng->owner);
                if (weakpts > thingpts)
                {
                    weakpts = thingpts;
                    weaktng = thing;
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(8,"The weakest valid target for %s index %d owner %d is %s index %d owner %d",
        thing_model_name(calltng),(int)calltng->index,(int)calltng->owner,
        thing_model_name(weaktng),(int)weaktng->index,(int)weaktng->owner);
    return weaktng;
}

struct Thing *get_scavenger_target(const struct Thing *calltng)
{
    struct Dungeon* dungeon = get_dungeon(calltng->owner);
    { // Check if last scavenged creature of that kind is still good for scavenging
        struct Thing *lastng;
        if (dungeon->scavenge_targets[calltng->model] != 0) {
            lastng = thing_get(dungeon->scavenge_targets[calltng->model]);
        } else {
            lastng = INVALID_THING;
        }
        if (thing_is_valid_scavenge_target(calltng, lastng))
        {
            SYNCDBG(8,"The last target, %s index %d owner %d, is still valid",thing_model_name(lastng),(int)lastng->index,(int)lastng->owner);
            return lastng;
        }
    }
    return select_scavenger_target(calltng);
}

long turn_creature_to_scavenger(struct Thing *scavtng, struct Thing *calltng)
{
    struct Room* room = get_room_thing_is_on(calltng);
    if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_CrScavenge) || (room->owner != calltng->owner))
    {
      ERRORLOG("The %s index %d is scavenging not on owned %s",thing_model_name(calltng),(int)calltng->index,room_code_name(RoK_SCAVENGER));
      return 0;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(scavtng, room, &pos))
    {
        ERRORLOG("Could not find valid position for thing");
        return 0;
    }
    {
        struct Dungeon* calldngn = get_dungeon(calltng->owner);
        calldngn->creatures_scavenge_gain++;
        calldngn->creatures_scavenged[scavtng->model]++;
    }
    if (!is_neutral_thing(scavtng))
    {
        struct Dungeon* scavdngn = get_dungeon(scavtng->owner);
        scavdngn->creatures_scavenge_lost++;
    }
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(scavtng);
        cctrl->scavenge.job_stage = JobStage_BeingScavenged;
        cctrl->scavenge.effect_id = calltng->owner;
        cctrl->scavenge.stl_9D_x = pos.x.stl.num;
        cctrl->scavenge.stl_9D_y = pos.y.stl.num;
    }
    external_set_thing_state(scavtng, CrSt_CreatureScavengedDisappear);
    return 1;
}

TbBool process_scavenge_creature_from_level(struct Thing *scavtng, struct Thing *calltng, long work_value)
{
    long num_prayers;
    struct Dungeon* calldngn = get_dungeon(calltng->owner);
    if (dungeon_invalid(calldngn)) {
        ERRORLOG("The %s index %d owner %d can't do scavenging - has no dungeon",thing_model_name(calltng),(int)calltng->index,(int)calltng->owner);
        return false;
    }
    // Compute amount of creatures praying against the scavenge
    if (!is_neutral_thing(scavtng)) {
        struct Dungeon* scavdngn = get_dungeon(scavtng->owner);
        num_prayers = scavdngn->creatures_praying[scavtng->model];
    } else {
        num_prayers = 0;
    }
    // Increase scavenging counter, used to break the prayers counter
    calldngn->creatures_scavenging[scavtng->model]++;
    // If scavenge is blocked by prayers, return
    if (calldngn->creatures_scavenging[calltng->model] < 2 * num_prayers) {
        SYNCDBG(8, "Player %d prayers (%d) are blocking player %d scavenging (%d) of %s index %d", (int)scavtng->owner,
            (int)num_prayers, (int)calltng->owner, (int)calldngn->creatures_scavenging[calltng->model], thing_model_name(calltng),(int)calltng->index);
        return false;
    }
    SYNCDBG(18,"The %s index %d scavenges %s index %d",thing_model_name(calltng),(int)calltng->index,thing_model_name(scavtng),(int)scavtng->index);
    // If we're starting to scavenge a new creature, do the switch
    if (calldngn->scavenge_targets[calltng->model] != scavtng->index)
    {
        calldngn->scavenge_turn_points[calltng->model] += work_value;
        if (calldngn->scavenge_targets[calltng->model] > 0)
        {
            // Stop scavenging old creature
            struct Thing* thing = thing_get(calldngn->scavenge_targets[calltng->model]);
            if (thing_is_creature(thing) && (thing->model == calltng->model))
            {
                if (creature_is_being_scavenged(thing)) {
                    set_start_state(thing);
                }
            }
        }
        // Start the new scavenging
        calldngn->scavenge_targets[calltng->model] = scavtng->index;
        if (is_my_player_number(scavtng->owner)) {
            output_message(SMsg_CreatureScanvenged, 500);
        }
        event_create_event(scavtng->mappos.x.val, scavtng->mappos.y.val, EvKind_CreatrScavenged, scavtng->owner, scavtng->index);
    } else
    {
        calldngn->scavenge_turn_points[calltng->model] += work_value;
    }
    // Make sure the scavenged creature is in correct state
    if (!creature_is_being_scavenged(scavtng))
    {
        if (!is_neutral_thing(scavtng)) {
            external_set_thing_state(scavtng, CrSt_CreatureBeingScavenged);
        }
    }
    long scavpts = calculate_correct_creature_scavenge_required(scavtng, calltng->owner);
    if ((scavpts << 8) < calldngn->scavenge_turn_points[calltng->model])
    {
        SYNCDBG(8,"The %s index %d owner %d accumulated enough points to turn to scavenger",thing_model_name(scavtng),(int)scavtng->index,(int)scavtng->owner);
        if (turn_creature_to_scavenger(scavtng, calltng))
        {
            calldngn->scavenge_turn_points[calltng->model] = 0;
            return true;
        }
    }
    return false;
}

TbBool creature_scavenge_from_creature_pool(struct Thing *calltng)
{
    struct Coord3d pos;
    struct Room* room = get_room_thing_is_on(calltng);
    if (!room_initially_valid_as_type_for_thing(room, RoRoF_CrScavenge, calltng)) {
        WARNLOG("Room %s owned by player %d is bad work place for %s index %d owner %d",room_code_name(room->kind),(int)room->owner,thing_model_name(calltng),(int)calltng->index,(int)calltng->owner);
        return false;
    }
    if (game.pool.crtr_kind[calltng->model] <= 0) {
        ERRORLOG("Tried to generate %s but it is not in pool",thing_model_name(calltng));
        return false;
    }
    if ( !find_random_valid_position_for_thing_in_room(calltng, room, &pos) ) {
        ERRORLOG("Could not find valid position in %s for %s to be generated",room_code_name(room->kind),creature_code_name(calltng->model));
        return false;
    }
    if (!creature_count_below_map_limit(0))
    {
        SYNCDBG(7,"Scavenge creature %s from portal failed to due to map creature limit", thing_model_name(calltng));
        return false;
    }
    struct Thing* scavtng = create_creature(&pos, calltng->model, calltng->owner);
    if (thing_is_invalid(scavtng)) {
        ERRORLOG("Tried to generate %s but creation failed",thing_model_name(calltng));
        return false;
    }
    if (!remove_creature_from_generate_pool(calltng->model))
    {
        ERRORLOG("Could not remove %s from pool",thing_model_name(calltng));
        return false;
    }
    {
        struct Dungeon* dungeon = get_dungeon(calltng->owner);
        dungeon->creatures_scavenge_gain++;
    }
    internal_set_thing_state(scavtng, CrSt_CreatureScavengedReappear);
    return true;
}

TbBool process_scavenge_creature_from_pool(struct Thing *calltng, long work_value)
{
    struct Dungeon* calldngn = get_dungeon(calltng->owner);
    calldngn->scavenge_turn_points[calltng->model] += work_value;
    long scavpts = calculate_correct_creature_scavenge_required(calltng, calltng->owner);
    if ((scavpts << 8) < calldngn->scavenge_turn_points[calltng->model])
    {
        if (creature_scavenge_from_creature_pool(calltng))
        {
            calldngn->scavenge_turn_points[calltng->model] = 0;
            return true;
        }
    }
    return false;
}

CrCheckRet process_scavenge_function(struct Thing *calltng)
{
    SYNCDBG(18,"Starting for %s owner %d",thing_model_name(calltng),(int)calltng->owner);
    struct CreatureControl* callctrl = creature_control_get_from_thing(calltng);
    struct Dungeon* calldngn = get_dungeon(calltng->owner);
    struct Room* room = get_room_creature_works_in(calltng);
    if ( !room_still_valid_as_type_for_thing(room, RoRoF_CrScavenge, calltng) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(calltng),(int)calltng->owner);
        set_start_state(calltng);
        return CrCkRet_Continue;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(calltng);
    if (!player_can_afford_to_scavenge_creature(calltng))
    {
        if (is_my_player_number(calltng->owner))
            output_message(SMsg_NoGoldToScavenge, 500);
        set_start_state(calltng);
        return CrCkRet_Continue;
    }
    if (calldngn->scavenge_counters_turn != game.play_gameturn)
    {
        reset_scavenge_counts(calldngn);
    }
    long work_value = compute_creature_work_value_for_room_role(calltng, RoRoF_CrScavenge, room->efficiency);
    SYNCDBG(9,"The %s index %d owner %d produced %d scavenge points",thing_model_name(calltng),(int)calltng->index,(int)calltng->owner,(int)work_value);
    struct Thing* scavtng = get_scavenger_target(calltng);
    if (!thing_is_invalid(scavtng))
    {
        process_scavenge_creature_from_level(scavtng, calltng, work_value);
    } else
    if (can_scavenge_creature_from_pool(calldngn, calltng->model))
    {
        process_scavenge_creature_from_pool(calltng, work_value);
    } else
    {
        if (crconf->entrance_force) {
          calldngn->portal_scavenge_boost++;
        }
        return 0;
    }
    callctrl->turns_at_job++;
    if (callctrl->turns_at_job > game.conf.rules[calltng->owner].rooms.scavenge_cost_frequency)
    {
        callctrl->turns_at_job -= game.conf.rules[calltng->owner].rooms.scavenge_cost_frequency;
        GoldAmount scavenger_cost = calculate_correct_creature_scavenging_cost(calltng);
        if (take_money_from_dungeon(calltng->owner, scavenger_cost, 1) < 0) {
            ERRORLOG("Cannot take %d gold from dungeon %d",(int)scavenger_cost,(int)calltng->owner);
        }
        create_price_effect(&calltng->mappos, calltng->owner, scavenger_cost);
    }
    return 0;
}

CrStateRet scavengering(struct Thing *creatng)
{
    // Check if we're in correct room
    struct Room* room = get_room_thing_is_on(creatng);
    if (creature_job_in_room_no_longer_possible(room, Job_SCAVENGE, creatng))
    {
        remove_creature_from_work_room(creatng);
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }

    if (process_scavenge_function(creatng))
    {
        return CrStRet_Modified;
    }
    if (!creature_setup_adjacent_move_for_job_within_room(creatng, room, Job_SCAVENGE)) {
        return CrStRet_Unchanged;
    }
    creatng->continue_state = get_continue_state_for_job(Job_SCAVENGE);
    return CrStRet_Modified;
}

/******************************************************************************/
