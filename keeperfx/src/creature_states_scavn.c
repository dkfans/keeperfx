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
DLLIMPORT short _DK_at_scavenger_room(struct Thing *thing);
DLLIMPORT short _DK_creature_being_scavenged(struct Thing *thing);
DLLIMPORT short _DK_creature_scavenged_disappear(struct Thing *thing);
DLLIMPORT short _DK_creature_scavenged_reappear(struct Thing *thing);
DLLIMPORT long _DK_process_scavenge_function(struct Thing *thing);
DLLIMPORT short _DK_scavengering(struct Thing *thing);
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
    cctrl = creature_control_get_from_thing(thing);
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_SCAVENGER) || (room->owner != thing->owner))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (crstat->scavenger_cost >= dungeon->total_money_owned)
    {
        if (is_my_player_number(thing->owner))
            output_message(88, 500, 1);
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
        dungeon->field_98B++;
        if (is_my_player_number(thing->owner))
          output_message(62, 0, 1);
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

long process_scavenge_function(struct Thing *thing)
{
  return _DK_process_scavenge_function(thing);
}

short scavengering(struct Thing *thing)
{
  return _DK_scavengering(thing);
}

/******************************************************************************/
