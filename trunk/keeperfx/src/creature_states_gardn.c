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
#include "player_instances.h"
#include "room_data.h"
#include "room_jobs.h"
#include "gui_soundmsgs.h"
#include "creature_states_prisn.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_creature_arrived_at_garden(struct Thing *creatng);
DLLIMPORT short _DK_creature_eat(struct Thing *creatng);
DLLIMPORT short _DK_creature_eating_at_garden(struct Thing *creatng);
DLLIMPORT short _DK_creature_to_garden(struct Thing *creatng);
DLLIMPORT void _DK_person_search_for_food_again(struct Thing *creatng, struct Room *room);
DLLIMPORT void _DK_person_eat_food(struct Thing *creatng, struct Thing *foodtng, struct Room *room);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool hunger_is_creature_hungry(const struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    cctrl = creature_control_get_from_thing(creatng);
    crstat = creature_stats_get_from_thing(creatng);
    if (creature_control_invalid(cctrl) || creature_stats_invalid(crstat))
        return false;
    return ((crstat->hunger_rate != 0) && (cctrl->hunger_level > crstat->hunger_rate));
}

void person_eat_food(struct Thing *creatng, struct Thing *foodtng, struct Room *room)
{
    return _DK_person_eat_food(creatng, foodtng, room);
}

void person_search_for_food_again(struct Thing *creatng, struct Room *room)
{
    struct Thing *near_food_tng;
    long near_food_dist;
    //_DK_person_search_for_food_again(thing, room);
    near_food_dist = LONG_MAX;
    near_food_tng = INVALID_THING;
    unsigned long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        MapSubtlCoord slb_x,slb_y;
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        // Per-slab code
        long n;
        for (n=0; n < 9; n++)
        {
            MapSubtlCoord x,y;
            struct Thing *thing;
            x = slab_subtile(slb_x,n%3);
            y = slab_subtile(slb_y,n/3);
            thing = get_food_at_subtile_available_to_eat_and_owned_by(x, y, -1);
            if (!thing_is_invalid(thing))
            {
                long dist;
                dist = get_2d_box_distance(&creatng->mappos, &thing->mappos);
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
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    if ( thing_is_invalid(near_food_tng) || is_thing_passenger_controlled(near_food_tng) )
    {
        // Warn about no food in this room
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_GardenTooSmall, MESSAGE_DELAY_ROOM_SMALL, 1);
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        // Check whether there's a room which does have food
        struct Room *nroom;
        // Try to find one which has plenty of food
        nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_GARDEN, 0, crstat->hunger_fill+1);
        if (room_is_invalid(nroom)) {
            // If not found, maybe at least one chicken?
            nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_GARDEN, 0, 1);
        }
        if (!room_is_invalid(nroom))
        {
            if (setup_random_head_for_room(creatng, nroom, 0)) {
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
            if (person_move_somewhere_adjacent_in_room(creatng, room)) {
                creatng->continue_state = CrSt_CreatureArrivedAtGarden;
            } else {
                set_start_state(creatng);
            }
        }
        return;
    }
    if (!setup_person_move_close_to_position(creatng,
           near_food_tng->mappos.x.stl.num, near_food_tng->mappos.y.stl.num, 0))
    {
        ERRORLOG("Cannot get near to food");
        person_eat_food(creatng, near_food_tng, room);
        return;
    }
    struct CreatureControl *cctrl;
    if (near_food_tng->class_id == TCls_Creature)
    {
        cctrl = creature_control_get_from_thing(near_food_tng);
        cctrl->affected_by_spells |= CCSpl_ChickenRel;
    } else
    {
        near_food_tng->byte_15 = 255;
        near_food_tng->byte_16 = 127;
    }
    creatng->continue_state = CrSt_CreatureEatingAtGarden;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->long_9A = near_food_tng->index;
}

short creature_arrived_at_garden(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_creature_arrived_at_garden(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, RoK_GARDEN, thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",
            room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    person_search_for_food_again(thing, room);
    return 1;
}

short creature_eat(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_creature_eat(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->instance_id != CrInst_EAT)
        internal_set_thing_state(thing, thing->continue_state);
    return 1;
}

short creature_eating_at_garden(struct Thing *thing)
{
  return _DK_creature_eating_at_garden(thing);
}

short creature_to_garden(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct Room *nroom;
    //return _DK_creature_to_garden(thing);
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->hunger_amount == 0) {
        set_start_state(creatng);
        return 0;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    if (!player_has_room(creatng->owner, RoK_GARDEN))
    {
        // No room for feeding creatures
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_RoomGardenNeeded, MESSAGE_DELAY_ROOM_NEED, 1);
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        nroom = INVALID_ROOM;
    } else
    {
        // Try to find one which has plenty of food
        nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_GARDEN, 0, crstat->hunger_fill+1);
        if (room_is_invalid(nroom)) {
            // If not found, maybe at least one chicken?
            nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_GARDEN, 0, 1);
        }
        if (room_is_invalid(nroom)) {
            // No correct room - but check what exactly is the problem
            nroom = find_nearest_room_for_thing(creatng, creatng->owner, RoK_GARDEN, 0);
            if (room_is_invalid(nroom)) {
                // There seem to be a correct room, but we can't reach it
                if (is_my_player_number(creatng->owner))
                    output_message(SMsg_NoRouteToGarden, MESSAGE_DELAY_ROOM_NEED, 1);
            } else
            {
                // The room is reachable, so it probably has just no food
                if (is_my_player_number(creatng->owner))
                    output_message(SMsg_GardenTooSmall, MESSAGE_DELAY_ROOM_SMALL, 1);
                event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
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
    if (!setup_random_head_for_room(creatng, nroom, 0))
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
