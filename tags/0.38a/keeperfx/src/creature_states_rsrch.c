/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_rsrch.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_rsrch.h"
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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_research_room(struct Thing *thing);
DLLIMPORT long _DK_process_research_function(struct Thing *thing);
DLLIMPORT short _DK_researching(struct Thing *thing);
DLLIMPORT void _DK_force_complete_current_research(long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short at_research_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_research_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    if (thing->owner == game.neutral_player_num)
    {
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if ((crstat->research_value <= 0) || (dungeon->field_F78 < 0))
    {
        if ((thing->owner != game.neutral_player_num) && (dungeon->field_F78 < 0))
        {
            if (is_my_player_number(dungeon->field_E9F))
                output_message(46, 500, 1);
        }
        set_start_state(thing);
        return 0;
    }
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_LIBRARY) || (room->owner != thing->owner))
    {
        WARNLOG("Room of kind %d and owner %d is invalid for %s",(int)room->kind,(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room))
    {
        set_start_state(thing);
        return 0;
    }
    if ( !setup_random_head_for_room(thing, room, 0) )
    {
        ERRORLOG("The %s can not move in research room", thing_model_name(thing));
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = CrSt_Researching;
    cctrl->field_82 = 0;
    cctrl->byte_9A = 3;
    return 1;
}

void force_complete_current_research(long plyr_idx)
{
  _DK_force_complete_current_research(plyr_idx);
}

long process_research_function(struct Thing *thing)
{
  return _DK_process_research_function(thing);
}

short researching(struct Thing *thing)
{
  return _DK_researching(thing);
}

/******************************************************************************/
