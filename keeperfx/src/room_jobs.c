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

#include "bflib_math.h"
#include "creature_control.h"
#include "dungeon_data.h"
#include "light_data.h"
#include "thing_data.h"
#include "thing_list.h"
#include "thing_navigate.h"
#include "config_terrain.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned char _DK_remove_creature_from_work_room(struct Thing *creatng);

/******************************************************************************/
struct Room *get_room_creature_works_in(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    return room_get(cctrl->work_room_id);
}

TbBool add_creature_to_torture_room(struct Thing *creatng, const struct Room *room)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    cctrl = creature_control_get_from_thing(creatng);
    if (creatng->light_id != 0) {
        light_delete_light(creatng->light_id);
        creatng->light_id = 0;
    }
    if ((cctrl->spell_flags & CSAfF_Speed) != 0)
        terminate_thing_spell_effect(creatng, SplK_Speed);
    if ((cctrl->spell_flags & CSAfF_Invisibility) != 0)
        terminate_thing_spell_effect(creatng, SplK_Invisibility);
    dungeon = get_dungeon(room->owner);
    dungeon->lvstats.creatures_tortured++;
    if (dungeon->tortured_creatures[creatng->model] == 0)
    {
        // Torturing changes speed of creatures of that kind, so let's update
        update_speed_of_player_creatures_of_model(room->owner, creatng->model);
    }
    dungeon->tortured_creatures[creatng->model]++;
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

TbBool remove_creature_from_specific_room(struct Thing *creatng, struct Room *room)
{
    struct CreatureControl *cctrl;
    struct Thing *sectng;
    struct CreatureControl *sectrl;
    cctrl = creature_control_get_from_thing(creatng);
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
            ERRORLOG("Linked list of rooms has invalid previous element on thing %d",(int)creatng->index);
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
            ERRORLOG("Linked list of rooms has invalid next element on thing %d",(int)creatng->index);
        }
    }
    cctrl->last_work_room_id = cctrl->work_room_id;
    cctrl->work_room_id = 0;
    cctrl->flgfield_1 &= ~CCFlg_IsInRoomList;
    cctrl->next_in_room = 0;
    cctrl->prev_in_room = 0;
    return true;
}

TbBool remove_creature_from_work_room(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_remove_creature_from_work_room(thing);
    cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) == 0)
        return false;
    room = room_get(cctrl->work_room_id);
    if (room_is_invalid(room))
    {
        WARNLOG("Creature had invalid room index %d",(int)cctrl->work_room_id);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    return remove_creature_from_specific_room(creatng, room);
}

/**
 * Gives an object in given room which matches given filter and to which the creature can navigate.
 * @param creatng Creature which should be able to access object to be returned.
 * @param room The room which tiles are to be searched.
 * @param matcher_cb Filter which objects must match to be returned.
 * @note This function may be used for selecting an object to be picked by a player's own creature,
 *  but also for selecting a target for stealing by enemy. Because of that, the function does not
 *  check an owner of the object thing.
 * @return The object thing which matches given criteria.
 */
struct Thing *find_object_in_room_for_creature_matching_bool_filter(struct Thing *creatng, const struct Room *room, Thing_Bool_Filter matcher_cb)
{
    struct Thing *rettng,*tmptng;
    long selected;
    unsigned long k;
    long i;
    rettng = INVALID_THING;
    if (room->slabs_count <= 0)
    {
        WARNLOG("Room with no slabs detected!");
        return rettng;
    }
    selected = ACTION_RANDOM(room->slabs_count);
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        MapSubtlCoord stl_x,stl_y;
        stl_x = slab_subtile_center(slb_num_decode_x(i));
        stl_y = slab_subtile_center(slb_num_decode_y(i));
        // Per room tile code
        tmptng = get_object_around_owned_by_and_matching_bool_filter(
            subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, matcher_cb);
        if (!thing_is_invalid(tmptng))
        {
            if (creature_can_navigate_to_with_storage(creatng, &tmptng->mappos, 0))
            {
                rettng = tmptng;
                if (selected > 0)
                {
                    selected--;
                } else
                {
                    break;
                }
            }
        }
        // Per room tile code ends
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > room->slabs_count)
        {
          ERRORLOG("Room slabs list length exceeded when sweeping");
          break;
        }
    }
    return rettng;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
