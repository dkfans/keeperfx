/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_guard.c
 *     Creature state machine functions related to guard post.
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
#include "creature_states_guard.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
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

#include "game_legacy.h"
#include "keeperfx.hpp"

/******************************************************************************/
short at_guard_post_room(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_for_job(Job_GUARD), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room, Job_GUARD))
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, get_continue_state_for_job(Job_GUARD));
    if (!person_get_somewhere_adjacent_in_room(thing, room, &cctrl->moveto_pos))
    {
        cctrl->moveto_pos.x.val = thing->mappos.x.val;
        cctrl->moveto_pos.y.val = thing->mappos.y.val;
        cctrl->moveto_pos.z.val = thing->mappos.z.val;
    }
    return 1;
}

CrStateRet guarding(struct Thing *thing)
{
    TRACE_THING(thing);
    struct Room* room = get_room_thing_is_on(thing);
    if (creature_job_in_room_no_longer_possible(room, Job_GUARD, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_move_to(thing, &cctrl->moveto_pos, cctrl->max_speed, 0, 0) == 0)
    {
        return CrStRet_Unchanged;
    }
    if (!person_get_somewhere_adjacent_in_room(thing, room, &cctrl->moveto_pos))
    {
        cctrl->moveto_pos.x.val = thing->mappos.x.val;
        cctrl->moveto_pos.y.val = thing->mappos.y.val;
        cctrl->moveto_pos.z.val = thing->mappos.z.val;
    }
    return CrStRet_Modified;
}

/******************************************************************************/
