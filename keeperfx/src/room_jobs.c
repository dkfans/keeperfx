/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_jobs.c
 *     List of things in room maintain functions.
 * @par Purpose:
 *     Functions to create and use lists of things for specific rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Dec 2010 - 02 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_jobs.h"

#include "globals.h"
#include "bflib_basics.h"

#include "creature_control.h"
#include "dungeon_data.h"
#include "light_data.h"
#include "thing_data.h"
#include "thing_list.h"
#include "config_terrain.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned char _DK_remove_creature_from_work_room(struct Thing *thing);

/******************************************************************************/
struct Room *get_room_creature_works_in(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    return room_get(cctrl->work_room_id);
}

TbBool add_creature_to_torture_room(struct Thing *crtng, struct Room *room)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    cctrl = creature_control_get_from_thing(crtng);
    if (crtng->light_id != 0) {
        light_delete_light(crtng->light_id);
        crtng->light_id = 0;
    }
    if ((cctrl->spell_flags & CSF_Speed) != 0)
        terminate_thing_spell_effect(crtng, 11);
    if ((cctrl->spell_flags & CSF_Conceal) != 0)
        terminate_thing_spell_effect(crtng, 9);
    dungeon = get_dungeon(room->owner);
    dungeon->lvstats.creatures_tortured++;
    if (dungeon->tortured_creatures[crtng->model] == 0)
    {
        // Torturing changes speed of creatures of that kind, so let's update
        update_speed_of_player_creatures_of_model(room->owner, crtng->model);
    }
    dungeon->tortured_creatures[crtng->model]++;
    return true;
}

TbBool add_creature_to_work_room(struct Thing *crtng, struct Room *room)
{
    struct CreatureControl *cctrl;
    struct Thing *nxtng;
    struct CreatureControl *nxctrl;
    cctrl = creature_control_get_from_thing(crtng);
    if (cctrl->work_room_id != 0)
    {
        WARNLOG("Attempt to add creature to room kind %d when he is a member of kind %d",
            (int)room->kind, (int)room_get(cctrl->work_room_id)->kind);
        remove_creature_from_work_room(crtng);
    }
    if ((cctrl->flgfield_1 & 0x20) != 0)
    {
        ERRORLOG("Attempt to add creature to a room when he is in the list of another");
        return false;
    }
    if (room->total_capacity < room->used_capacity + 1)
        return false;
    room->used_capacity++;
    cctrl->work_room_id = room->index;
    cctrl->prev_in_room = 0;
    if (room->creatures_list != 0)
    {
        nxtng = thing_get(room->creatures_list);
        nxctrl = creature_control_get_from_thing(nxtng);
    } else
    {
        nxctrl = INVALID_CRTR_CONTROL;
    }
    if (!creature_control_invalid(nxctrl))
    {
        cctrl->next_in_room = room->creatures_list;
        nxctrl->prev_in_room = crtng->index;
    } else
    {
        cctrl->next_in_room = 0;
    }
    room->creatures_list = crtng->index;
    cctrl->flgfield_1 |= 0x20;
    return true;
}

TbBool remove_creature_from_specific_room(struct Thing *crtng, struct Room *room)
{
    struct CreatureControl *cctrl;
    struct Thing *sectng;
    struct CreatureControl *sectrl;
    cctrl = creature_control_get_from_thing(crtng);
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) == 0)
    {
        ERRORLOG("Attempt to remove a creature from room, but it isn't in any");
        return false;
    }
    if (room->used_capacity > 0) {
        room->used_capacity--;
    } else {
        WARNLOG("Attempt to remove a creature from room %s with too little used space", room_code_name(room->kind));
    }
    if (cctrl->prev_in_room != 0) {
        sectng = thing_get(cctrl->prev_in_room);
        sectrl = creature_control_get_from_thing(sectng);
        if (!creature_control_invalid(sectrl)) {
            sectrl->next_in_room = cctrl->next_in_room;
        } else {
            ERRORLOG("Linked list of rooms has invalid previous element on thing %d",(int)crtng->index);
        }
    } else {
        room->creatures_list = cctrl->next_in_room;
    }
    if (cctrl->next_in_room != 0) {
        sectng = thing_get(cctrl->next_in_room);
        sectrl = creature_control_get_from_thing(sectng);
        if (!creature_control_invalid(sectrl)) {
            sectrl->prev_in_room = cctrl->prev_in_room;
        } else {
            ERRORLOG("Linked list of rooms has invalid next element on thing %d",(int)crtng->index);
        }
    }
    cctrl->last_work_room_id = cctrl->work_room_id;
    cctrl->work_room_id = 0;
    cctrl->flgfield_1 &= ~CCFlg_IsInRoomList;
    cctrl->next_in_room = 0;
    cctrl->prev_in_room = 0;
    return true;
}

TbBool remove_creature_from_work_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_remove_creature_from_work_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) == 0)
        return false;
    room = room_get(cctrl->work_room_id);
    if (room_is_invalid(room))
    {
        WARNLOG("Creature had invalid room index %d",(int)cctrl->work_room_id);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    return remove_creature_from_specific_room(thing, room);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
