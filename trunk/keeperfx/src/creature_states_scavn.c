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
#include "creature_states_scavn.h"
#include "globals.h"

#include "bflib_math.h"
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
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_scavenger_room(struct Thing *calltng);
DLLIMPORT short _DK_creature_being_scavenged(struct Thing *calltng);
DLLIMPORT short _DK_creature_scavenged_disappear(struct Thing *calltng);
DLLIMPORT short _DK_creature_scavenged_reappear(struct Thing *calltng);
DLLIMPORT long _DK_process_scavenge_function(struct Thing *calltng);
DLLIMPORT short _DK_scavengering(struct Thing *calltng);
DLLIMPORT long _DK_turn_creature_to_scavenger(struct Thing *scavtng, struct Thing *calltng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short at_scavenger_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_scavenger_room(thing);
    room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, RoK_SCAVENGER, thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (crstat->scavenger_cost >= dungeon->total_money_owned)
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToScavenge, MESSAGE_DELAY_TREASURY, true);
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Scavengering);
    cctrl->field_82 = 0;
    return 1;
}

short creature_being_scavenged(struct Thing *thing)
{
  return _DK_creature_being_scavenged(thing);
}

short creature_scavenged_disappear(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Room *room;
    struct Coord3d pos;
    long stl_x, stl_y;
    long i;
    //return _DK_creature_scavenged_disappear(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->byte_9A--;
    if (cctrl->byte_9A > 0)
    {
      if ((cctrl->byte_9A == 7) && (cctrl->byte_9B < PLAYERS_COUNT))
      {
        create_effect(&thing->mappos, get_scavenge_effect_element(cctrl->byte_9B), thing->owner);
      }
      return 0;
    }
    // We don't really have to convert coordinates into numbers and back to XY.
    i = get_subtile_number(cctrl->byte_9D, cctrl->byte_9E);
    stl_x = stl_num_decode_x(i);
    stl_y = stl_num_decode_y(i);
    room = subtile_room_get(stl_x, stl_y);
    if (room_is_invalid(room) || (room->kind != RoK_SCAVENGER))
    {
        ERRORLOG("Scavenger room disappeared.");
        kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
        return -1;
    }
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        move_thing_in_map(thing, &pos);
        anger_set_creature_anger_all_types(thing, 0);
        dungeon = get_dungeon(cctrl->byte_9B);
        dungeon->creatures_scavenge_gain++;
        if (is_my_player_number(thing->owner))
          output_message(SMsg_MinionScanvenged, 0, true);
        cctrl->byte_9C = thing->owner;
        change_creature_owner(thing, cctrl->byte_9B);
        internal_set_thing_state(thing, 94);
        return 0;
    } else
    {
        ERRORLOG("No valid position inside scavenger room for %s.",thing_model_name(thing));
        kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
        return -1;
    }
}

short creature_scavenged_reappear(struct Thing *thing)
{
    return _DK_creature_scavenged_reappear(thing);
}

TbBool player_can_afford_to_scavenge_creature(const struct Thing *thing)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(thing->owner);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (crstat->scavenger_cost >= dungeon->total_money_owned);
}

TbBool reset_scavenge_counts(struct Dungeon *dungeon)
{
    memset(dungeon->field_894, 0, 32);
    dungeon->field_88C[1] = game.play_gameturn;
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
    if (!thing_is_creature(scavtng) || (scavtng->model != calltng->model)) {
        return false;
    }
    //TODO [config] Add an option whether scavenging neutrals is possible
    if (scavtng->owner != game.neutral_player_num)
    {
        if (!players_are_enemies(calltng->owner, scavtng->owner)) {
            return false;
        }
    }
    if ( ((scavtng->alloc_flags & 0x10) != 0) || ((scavtng->field_1 & 0x02) != 0) ) {
        return false;
    }
    if (is_thing_passenger_controlled(scavtng) || creature_is_kept_in_custody(scavtng)) {
        return false;
    }
    //TODO [config] Add an option whether scavenging heroes is possible
    if (scavtng->owner == game.hero_player_num) {
        return false;
    }
    struct PlayerInfo *scavplyr;
    scavplyr = get_player(scavtng->owner);
    if (scavplyr->controlled_thing_idx != scavtng->index)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(scavtng);
        if (game.play_gameturn - cctrl->field_3D > game.temple_scavenge_protection_time)
        {
            return true;
        }
    }
    return false;
}

struct Thing *get_scavenger_target(const struct Thing *calltng)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(calltng->owner);
    { // Check if last scavenged creature of that kind is still good for scavenging
        struct Thing *lastng;
        if (dungeon->field_A0F[calltng->model] != 0) {
            lastng = thing_get(dungeon->field_A0F[calltng->model]);
        } else {
            lastng = INVALID_THING;
        }
        if (thing_is_valid_scavenge_target(calltng, lastng))
        {
            return lastng;
        }
    }
    long weakpts;
    struct Thing *weaktng;
    weaktng = INVALID_THING;
    weakpts = LONG_MAX;
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    k = 0;
    i = game.thing_lists[TngList_Creatures].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_valid_scavenge_target(calltng, thing))
        {
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(thing);
            if (game.play_gameturn - cctrl->field_3D > game.temple_scavenge_protection_time)
            {
                long thingpts;
                thingpts = calculate_correct_creature_scavenge_required(thing, calltng->owner);
                if (weakpts > thingpts)
                {
                    weakpts = thingpts;
                    weaktng = thing;
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return weaktng;
}

long turn_creature_to_scavenger(struct Thing *scavtng, struct Thing *calltng)
{
    return _DK_turn_creature_to_scavenger(scavtng, calltng);
}

TbBool process_scavenge_creature_from_level(struct Thing *scavtng, struct Thing *calltng, long work_value)
{
    struct Dungeon *calldngn;
    long num_prayers;
    calldngn = get_dungeon(calltng->owner);
    if (scavtng->owner != game.neutral_player_num) {
        calldngn->field_894[scavtng->model]++;
        struct Dungeon *scavdngn;
        scavdngn = get_dungeon(scavtng->owner);
        num_prayers = scavdngn->creatures_praying[scavtng->model];
    } else {
        num_prayers = 0;
    }
    if (calldngn->field_894[calltng->model] > 2 * num_prayers)
    {
        // If we're starting to scavenge a new creature, do the switch
        if (calldngn->field_A0F[calltng->model] != scavtng->index)
        {
            calldngn->field_98F[calltng->model] = work_value;
            if (calldngn->field_A0F[calltng->model] > 0)
            {
                struct Thing *thing;
                thing = thing_get(calldngn->field_A0F[calltng->model]);
                if (thing_is_creature(thing) && (thing->model == calltng->model))
                {
                    if (creature_is_being_scavenged(thing)) {
                        set_start_state(thing);
                    }
                }
            }
            calldngn->field_A0F[calltng->model] = scavtng->index;
            if (is_my_player_number(scavtng->owner)) {
                output_message(61, 500, 1);
            }
            event_create_event(scavtng->mappos.x.val, scavtng->mappos.y.val, 10, scavtng->owner, scavtng->index);
        } else
        {
            calldngn->field_98F[calltng->model] += work_value;
        }
        // Make sure the scavenged creature is in correct state
        if (!creature_is_being_scavenged(scavtng))
        {
            if (scavtng->owner != game.neutral_player_num) {
                external_set_thing_state(scavtng, CrSt_CreatureBeingScavenged);
            }
        }
        long scavpts;
        scavpts = calculate_correct_creature_scavenge_required(scavtng, calltng->owner);
        if (scavpts << 8 < calldngn->field_98F[calltng->model])
        {
            turn_creature_to_scavenger(scavtng, calltng);
            calldngn->field_98F[calltng->model] -= scavpts << 8;
            return true;
        }
    }
    return false;
}

TbBool creature_scavenge_from_creature_pool(struct Thing *calltng)
{
    struct Room *room;
    struct Coord3d pos;
    room = get_room_thing_is_on(calltng);
    if (!room_initially_valid_as_type_for_thing(room, RoK_SCAVENGER, calltng)) {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(calltng),(int)calltng->owner);
        return false;
    }
    if (game.pool.crtr_kind[calltng->model] <= 0) {
        ERRORLOG("Tried to generate %s but it is not in pool",thing_model_name(calltng));
        return false;
    }
    if ( !find_random_valid_position_for_thing_in_room(calltng, room, &pos) ) {
        ERRORLOG("Could not find valid position for thing to be generated");
        return false;
    }
    struct Thing *scavtng;
    scavtng = create_creature(&pos, calltng->model, calltng->owner);
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
        struct Dungeon *dungeon;
        dungeon = get_dungeon(calltng->owner);
        dungeon->creatures_scavenge_gain++;
    }
    internal_set_thing_state(scavtng, CrSt_CreatureScavengedReappear);
    return true;
}

TbBool process_scavenge_creature_from_pool(struct Thing *calltng, long work_value)
{
    struct Dungeon *calldngn;
    calldngn = get_dungeon(calltng->owner);
    calldngn->field_98F[calltng->model] += work_value;
    long scavpts;
    scavpts = calculate_correct_creature_scavenge_required(calltng, calltng->owner);
    if (scavpts << 8 < calldngn->field_98F[calltng->model])
    {
        creature_scavenge_from_creature_pool(calltng);
        calldngn->field_98F[calltng->model] -= scavpts;
        return true;
    }
    return false;
}

CrCheckRet process_scavenge_function(struct Thing *calltng)
{
    SYNCDBG(8,"Starting %s",thing_model_name(calltng));
    //return _DK_process_scavenge_function(thing);
    struct CreatureControl *callctrl;
    callctrl = creature_control_get_from_thing(calltng);
    struct Dungeon *calldngn;
    struct Room *room;
    calldngn = get_dungeon(calltng->owner);
    room = get_room_creature_works_in(calltng);
    if ( !room_still_valid_as_type_for_thing(room, RoK_SCAVENGER, calltng) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(calltng),(int)calltng->owner);
        set_start_state(calltng);
        return CrCkRet_Continue;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(calltng);
    if (player_can_afford_to_scavenge_creature(calltng))
    {
        if (is_my_player_number(calltng->owner))
            output_message(88, 500, 1);
        set_start_state(calltng);
        return CrCkRet_Continue;
    }
    if (calldngn->field_88C[1] != game.play_gameturn)
    {
        reset_scavenge_counts(calldngn);
    }
    long work_value;
    work_value = compute_creature_work_value(crstat->scavenge_value<<8, room->efficiency, callctrl->explevel);
    work_value = process_work_speed_on_work_value(calltng, work_value);
    struct Thing *scavtng;
    scavtng = get_scavenger_target(calltng);
    if (!thing_is_invalid(scavtng))
    {
        process_scavenge_creature_from_level(scavtng, calltng, work_value);
    } else
    if (can_scavenge_creature_from_pool(calldngn, calltng->model))
    {
        process_scavenge_creature_from_pool(calltng, work_value);
    } else
    {
        if (crstat->entrance_force) {
          calldngn->field_1485++;
        }
        return 0;
    }
    callctrl->field_82++;
    if (callctrl->field_82 > game.scavenge_cost_frequency)
    {
        callctrl->field_82 -= game.scavenge_cost_frequency;
        if (take_money_from_dungeon(calltng->owner, crstat->scavenger_cost, 1) < 0) {
            ERRORLOG("Cannot take %d gold from dungeon %d",(int)crstat->scavenger_cost,(int)calltng->owner);
        }
        create_price_effect(&calltng->mappos, calltng->owner, crstat->scavenger_cost);
    }
    return 0;
}

CrStateRet scavengering(struct Thing *thing)
{
    // Check if we're in correct room
    struct Room *room;
    room = get_room_thing_is_on(thing);
    if (creature_work_in_room_no_longer_possible(room, RoK_SCAVENGER, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    //return _DK_scavengering(thing);
    if (process_scavenge_function(thing))
    {
        return CrStRet_Modified;
    }
    if ( setup_prison_move(thing, room) )
    {
        thing->continue_state = CrSt_Scavengering;
        return CrStRet_Modified;
    }
    return CrStRet_Unchanged;
}

/******************************************************************************/
