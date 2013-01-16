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
#include "room_data.h"
#include "room_jobs.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_creature_arrived_at_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_eat(struct Thing *thing);
DLLIMPORT short _DK_creature_eating_at_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_to_garden(struct Thing *thing);
DLLIMPORT void _DK_person_search_for_food_again(struct Thing *thing, struct Room *room);
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

void person_search_for_food_again(struct Thing *thing, struct Room *room)
{
    _DK_person_search_for_food_again(thing, room);
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

short creature_to_garden(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_creature_to_garden(thing);
    cctrl = creature_control_get_from_thing(thing);
    if ( !cctrl->field_41[0] ) {
        set_start_state(thing);
        return 0;
    }
    if (!player_has_room(thing->owner, RoK_GARDEN))
    {
        // No room for feeding creatures
        if (is_my_player_number(thing->owner))
            output_message(SMsg_RoomGardenNeeded, MESSAGE_DELAY_ROOM_NEED, 1);
        event_create_event_or_update_nearby_existing_event(0, 0, 23, thing->owner, 0);
        room = INVALID_ROOM;
    } else
    {
        room = find_nearest_room_for_thing_with_used_capacity(thing, thing->owner, RoK_GARDEN, 0, 1);
        if (room_is_invalid(room)) {
            // No correct room - but check what exactly is the problem
            room = find_nearest_room_for_thing(thing, thing->owner, RoK_GARDEN, 0);
            if (room_is_invalid(room)) {
                // There seem to be a correct room, but we can't reach it
                if (is_my_player_number(thing->owner))
                    output_message(SMsg_NoRouteToGarden, MESSAGE_DELAY_ROOM_NEED, 1);
            } else
            {
                // The room is reachable, so it probably has just no food
                if (is_my_player_number(thing->owner))
                    output_message(SMsg_GardenTooSmall, MESSAGE_DELAY_ROOM_SMALL, 1);
                event_create_event_or_update_nearby_existing_event(0, 0, 23, thing->owner, 0);
            }
        }
    }
    // Apply anger if there's no room (note that anger isn't applied if room is just empty)
    if (room_is_invalid(room))
    {
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(thing);
        anger_apply_anger_to_creature(thing, crstat->annoy_no_hatchery, AngR_Hungry, 1);
        set_start_state(thing);
        return 0;
    }
    if (!setup_random_head_for_room(thing, room, 0))
    {
        ERRORLOG("Attempting to move to garden we cannot navigate to - this should not be possible");
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = CrSt_CreatureArrivedAtGarden;
    cctrl->target_room_id = room->index;
    return 1;
}

/******************************************************************************/
