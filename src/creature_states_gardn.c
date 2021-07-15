/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_gardn.c
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
#include "creature_states_gardn.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_creature.h"
#include "player_instances.h"
#include "power_hand.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_list.h"
#include "gui_soundmsgs.h"
#include "creature_states_prisn.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Returns if given creature has the ability to eat.
 */
TbBool creature_able_to_eat(const struct Thing *creatng)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if (creature_stats_invalid(crstat))
        return false;
    return (crstat->hunger_rate != 0);
}

TbBool hunger_is_creature_hungry(const struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if (creature_control_invalid(cctrl) || creature_stats_invalid(crstat))
        return false;
    return ((crstat->hunger_rate != 0) && (cctrl->hunger_level > crstat->hunger_rate));
}

void person_eat_food(struct Thing *creatng, struct Thing *foodtng, struct Room *room)
{
    thing_play_sample(creatng, 112+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    internal_set_thing_state(creatng, CrSt_CreatureEat);
    set_creature_instance(creatng, CrInst_EAT, 1, 0, 0);
    creatng->continue_state = CrSt_CreatureToGarden;
    {
        // TODO ANGER Maybe better either zero the hunger anger or apply annoy_eat_food points, based on the annoy_eat_food value?
        /*struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(creatng);
        anger_apply_anger_to_creature(creatng, crstat->annoy_eat_food, AngR_Other, 1);*/
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        anger_apply_anger_to_creature(creatng, -cctrl->annoyance_level[AngR_Hungry], AngR_Hungry, 1);
    }
    if (thing_is_creature(foodtng))
    {
        thing_death_flesh_explosion(foodtng);
    } else
    {
        int required_cap = get_required_room_capacity_for_object(RoRoF_FoodStorage, foodtng->model, 0);
        if (room->used_capacity >= required_cap) {
            room->used_capacity -= required_cap;
        } else {
            ERRORLOG("Trying to remove some food not in room");
        }
        delete_thing_structure(foodtng, 0);
    }
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    dungeon->lvstats.chickens_eaten++;
}

void person_search_for_food_again(struct Thing *creatng, struct Room *room)
{
    long near_food_dist = LONG_MAX;
    struct Thing* near_food_tng = INVALID_THING;
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x = slb_num_decode_x(i);
        MapSlabCoord slb_y = slb_num_decode_y(i);
        // Per-slab code
        for (long n = 0; n < 9; n++)
        {
            MapSubtlCoord x = slab_subtile(slb_x, n % 3);
            MapSubtlCoord y = slab_subtile(slb_y, n / 3);
            struct Thing* thing = get_food_at_subtile_available_to_eat_and_owned_by(x, y, -1);
            if (!thing_is_invalid(thing))
            {
                long dist = get_2d_box_distance(&creatng->mappos, &thing->mappos);
                if (near_food_dist > dist)
                {
                    near_food_dist = dist;
                    near_food_tng = thing;
                }
            }
        }
        // Per-slab code ends
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if (thing_is_invalid(near_food_tng) || is_thing_directly_controlled(near_food_tng) || is_thing_passenger_controlled(near_food_tng))
    {
        RoomKind job_rkind = get_room_for_job(Job_TAKE_FEED);
        // Warn about no food in this room
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        output_message_room_related_from_computer_or_player_action(creatng->owner, job_rkind, OMsg_RoomTooSmall);
        // Check whether there's a room which does have food
        // Try to find one which has plenty of food
        struct Room* nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, job_rkind, NavRtF_Default, crstat->hunger_fill + 1);
        if (room_is_invalid(nroom)) {
            // If not found, maybe at least one chicken?
            nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, job_rkind, NavRtF_Default, 1);
        }
        if (!room_is_invalid(nroom))
        {
            if (creature_setup_random_move_for_job_in_room(creatng, nroom, Job_TAKE_FEED, NavRtF_Default)) {
                creatng->continue_state = CrSt_CreatureArrivedAtGarden;
            } else {
                ERRORLOG("Attempting to move to garden we cannot navigate to - this should not be possible");
                set_start_state(creatng);
            }
        }
        else
        {
            anger_apply_anger_to_creature(creatng, crstat->annoy_no_hatchery, AngR_Hungry, 1);
            // Try to find food in the original room
            if (creature_setup_adjacent_move_for_job_within_room(creatng, room, Job_TAKE_FEED)) {
                creatng->continue_state = CrSt_CreatureArrivedAtGarden;
            } else {
                set_start_state(creatng);
            }
        }
        return;
    }
    if (!setup_person_move_close_to_position(creatng,
           near_food_tng->mappos.x.stl.num, near_food_tng->mappos.y.stl.num, NavRtF_Default))
    {
        ERRORLOG("Cannot get near to food");
        person_eat_food(creatng, near_food_tng, room);
        return;
    }
    struct CreatureControl *cctrl;
    if (near_food_tng->class_id == TCls_Creature)
    {
        cctrl = creature_control_get_from_thing(near_food_tng);
        cctrl->stateblock_flags |= CCSpl_ChickenRel;
    } else
    {
        near_food_tng->food.byte_15 = 255;
        near_food_tng->food.byte_16 = 127;
    }
    creatng->continue_state = CrSt_CreatureEatingAtGarden;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->long_9A = near_food_tng->index;
}

short creature_arrived_at_garden(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_for_job(Job_TAKE_FEED), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",
            room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    person_search_for_food_again(thing, room);
    return 1;
}

short creature_eat(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->instance_id != CrInst_EAT)
        internal_set_thing_state(thing, thing->continue_state);
    return 1;
}

short creature_eating_at_garden(struct Thing *creatng)
{
    //return _DK_creature_eating_at_garden(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* foodtng = thing_get(cctrl->long_9A);
    if (!thing_exists(foodtng)) {
        set_start_state(creatng);
        return 0;
    }
    struct Room* room = INVALID_ROOM;
    room = get_room_thing_is_on(foodtng);
    if (room_is_invalid(room) || (room->kind != get_room_for_job(Job_TAKE_FEED))) {
        set_start_state(creatng);
        return 0;
    }
    if (!thing_can_be_eaten(foodtng))
    {
        WARNLOG("Tried to eat %s index %d which cannot be eaten now but is in %s",
            thing_model_name(foodtng),(int)foodtng->index,room_code_name(get_room_for_job(Job_TAKE_FEED)));
        set_start_state(creatng);
        return 0;
    }
    person_eat_food(creatng, foodtng, room);
    cctrl->long_9A = 0;
    return 1;
}

short creature_to_garden(struct Thing *creatng)
{
    struct Room *nroom;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->hunger_amount == 0) {
        set_start_state(creatng);
        return 0;
    }
    RoomKind job_rkind = get_room_for_job(Job_TAKE_FEED);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if (!player_has_room(creatng->owner, job_rkind))
    {
        // No room for feeding creatures
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        output_message_room_related_from_computer_or_player_action(creatng->owner, job_rkind, OMsg_RoomNeeded);
        nroom = INVALID_ROOM;
    } else
    {
        // Try to find one which has plenty of food
        nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, job_rkind, NavRtF_Default, crstat->hunger_fill+1);
        if (room_is_invalid(nroom)) {
            // If not found, maybe at least one chicken?
            nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, job_rkind, NavRtF_Default, 1);
        }
        if (room_is_invalid(nroom)) {
            // No correct room - but check what exactly is the problem
            nroom = find_nearest_room_for_thing(creatng, creatng->owner, job_rkind, NavRtF_Default);
            if (room_is_invalid(nroom)) {
                // There seem to be a correct room, but we can't reach it
                output_message_room_related_from_computer_or_player_action(creatng->owner, job_rkind, OMsg_RoomNoRoute);
            } else
            {
                // The room is reachable, so it probably has just no food
                event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
                output_message_room_related_from_computer_or_player_action(creatng->owner, job_rkind, OMsg_RoomTooSmall);
            }
        }
    }
    // Apply anger if there's no room (note that anger isn't applied if room is just empty)
    if (room_is_invalid(nroom))
    {
        anger_apply_anger_to_creature(creatng, crstat->annoy_no_hatchery, AngR_Hungry, 1);
        set_start_state(creatng);
        return 0;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, nroom, Job_TAKE_FEED, NavRtF_Default))
    {
        ERRORLOG("Attempting to move to garden we cannot navigate to - this should not be possible");
        set_start_state(creatng);
        return 0;
    }
    creatng->continue_state = CrSt_CreatureArrivedAtGarden;
    cctrl->target_room_id = nroom->index;
    return 1;
}

/******************************************************************************/
