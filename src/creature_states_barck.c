/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_barck.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     02 Dec 2011 - 24 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_barck.h"
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
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

/******************************************************************************/
short at_barrack_room(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(creatng);
    if (!room_initially_valid_as_type_for_thing(room, get_room_for_job(Job_BARRACK), creatng))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return 0;
    }
    if (!add_creature_to_work_room(creatng, room, Job_BARRACK))
    {
        set_start_state(creatng);
        return 0;
    }
    internal_set_thing_state(creatng, get_continue_state_for_job(Job_BARRACK));
    return 1;
}

short barracking(struct Thing *creatng)
{
    struct Room* room = get_room_thing_is_on(creatng);
    if (!room_still_valid_as_type_for_thing(room, get_room_for_job(Job_BARRACK), creatng))
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s index %d owner %d",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        remove_creature_from_work_room(creatng);
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    if (!creature_setup_adjacent_move_for_job_within_room(creatng, room, Job_BARRACK)) {
        return CrStRet_Unchanged;
    }
    creatng->continue_state = get_continue_state_for_job(Job_BARRACK);
    return CrStRet_Modified;
}
/******************************************************************************/
