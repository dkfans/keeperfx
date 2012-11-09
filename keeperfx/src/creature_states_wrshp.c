/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_wrshp.c
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
#include "creature_states_wrshp.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_workshop_room(struct Thing *thing);
DLLIMPORT short _DK_manufacturing(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool setup_workshop_move(struct Thing *thing, SubtlCodedCoords stl_num)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->moveto_pos.x.stl.num = stl_num_decode_x(stl_num);
    cctrl->moveto_pos.x.stl.pos = 128;
    cctrl->moveto_pos.y.stl.num = stl_num_decode_y(stl_num);
    cctrl->moveto_pos.y.stl.pos = 128;
    cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
    if ( thing_in_wall_at(thing, &cctrl->moveto_pos) )
    {
        ERRORLOG("Illegal setup to subtile (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
        set_start_state(thing);
        return false;
    }
    return true;
}

TbBool setup_move_to_new_workshop_position(struct Thing *thing, struct Room *room, unsigned long a3)
{
    struct CreatureControl *cctrl;
    SubtlCodedCoords stl_num;
    cctrl = creature_control_get_from_thing(thing);
    if ( a3 )
        cctrl->byte_9E = 50;
    cctrl->byte_9A = 1;
    stl_num = find_position_around_in_room(&thing->mappos, thing->owner, RoK_WORKSHOP);
    if (stl_num <= 0)
    {
        WARNLOG("Couldn't find position around in workshop of %d slabs",(int)room->slabs_count);
        return false;
    }
    return setup_workshop_move(thing,stl_num);
}

short at_workshop_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_at_workshop_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, RoK_WORKSHOP, thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (room->total_capacity <= room->used_capacity)
    {
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Manufacturing);
    setup_move_to_new_workshop_position(thing, room, 1);
    return 1;
}

short manufacturing(struct Thing *thing)
{
  return _DK_manufacturing(thing);
}

/******************************************************************************/
