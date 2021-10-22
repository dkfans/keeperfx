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
#include "creature_states.h"
#include "creature_jobs.h"
#include "dungeon_data.h"
#include "light_data.h"
#include "thing_data.h"
#include "thing_list.h"
#include "thing_navigate.h"
#include "spdigger_stack.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "room_workshop.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#include "creature_states_rsrch.h"
#include "creature_states_train.h"
#include "creature_states_wrshp.h"
#include "creature_states_scavn.h"
#include "creature_states_lair.h"
#include "creature_states_pray.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Room *get_room_creature_works_in(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    return room_get(cctrl->work_room_id);
}

/**
 * Informs in the creature is working in given room.
 * @param creatng The creature thing.
 * @param room The room to be checked if creature works in it.
 * @return True if the creature is working in the room, false otherwise.
 */
TbBool creature_is_working_in_room(const struct Thing *creatng, const struct Room *room)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl)) {
        return false;
    }
    if (room_is_invalid(room)) {
        return false;
    }
    return (cctrl->work_room_id == room->index);
}

/**
 * Does tasks required to add a creature to torture room.
 * @note This function only does the part of adding which is required
 *  for torture room. Other tasks required when adding a creature to room,
 *  like add_creature_to_work_room(), have to be invoked separately.
 * @param creatng
 * @param room
 * @return
 * @see add_creature_to_work_room()
 * @see remove_creature_from_torture_room()
 */
TbBool add_creature_to_torture_room(struct Thing *creatng, const struct Room *room)
{
    TRACE_THING(creatng);
    if (creatng->light_id != 0) {
        light_delete_light(creatng->light_id);
        creatng->light_id = 0;
    }
    if (creature_affected_by_spell(creatng, SplK_Speed))
        terminate_thing_spell_effect(creatng, SplK_Speed);
    if (creature_affected_by_spell(creatng, SplK_Invisibility))
        terminate_thing_spell_effect(creatng, SplK_Invisibility);
    struct Dungeon* dungeon = get_dungeon(room->owner);
    dungeon->lvstats.creatures_tortured++;
    if (dungeon->tortured_creatures[creatng->model] == 0)
    {
        dungeon->tortured_creatures[creatng->model]++;
        // Torturing changes speed of creatures of that kind, so let's update
        update_speed_of_player_creatures_of_model(room->owner, creatng->model);
    } else
    {
        dungeon->tortured_creatures[creatng->model]++;
    }
    return true;
}

/**
 * Does tasks required to remove a creature from torture room.
 * @note This function only does the part of removing which is required
 *  for torture room. Other tasks required when removing a creature from room,
 *  like remove_creature_from_work_room(), have to be invoked separately.
 * @see remove_creature_from_work_room()
 * @param creatng The creature thing being removed from room.
 * @return
 */
TbBool remove_creature_from_torture_room(struct Thing *creatng)
{
    struct Room* room = get_room_creature_works_in(creatng);
    if (!room_exists(room)) {
        SYNCDBG(6,"The %s worked in a room which no longer exists",thing_model_name(creatng));
        return false;
    }
    PlayerNumber plyr_idx = room->owner;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon) || (dungeon->tortured_creatures[creatng->model] < 1)) {
        ERRORLOG("The %s is tortured by wrong player %d",thing_model_name(creatng),(int)plyr_idx);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    dungeon->tortured_creatures[creatng->model]--;
    if (dungeon->tortured_creatures[creatng->model] == 0)
    {
        // Torturing changes speed of creatures of that kind, so let's update
        update_speed_of_player_creatures_of_model(plyr_idx, creatng->model);
        // It also changes their pay, but thet's not updated here
    }
    return true;
}

TbBool add_creature_to_work_room(struct Thing *creatng, struct Room *room, CreatureJob jobpref)
{
    struct CreatureControl *nxctrl;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->work_room_id != 0)
    {
        const struct Room* wrkroom = room_get(cctrl->work_room_id);
        WARNLOG("Attempt to add creature to %s index %d when he is a member of %s index %d",
            room_code_name(room->kind), (int)room->index, room_code_name(wrkroom->kind), (int)wrkroom->index);
        remove_creature_from_work_room(creatng);
    }
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) != 0)
    {
        ERRORLOG("Attempt to add creature to a room when he is in the list of another");
        return false;
    }
    int required_cap = get_required_room_capacity_for_job(jobpref, creatng->model);
    if (room->used_capacity + required_cap > room->total_capacity)
        return false;
    room->used_capacity += required_cap;
    cctrl->work_room_id = room->index;
    cctrl->prev_in_room = 0;
    if (room->creatures_list != 0)
    {
        struct Thing* nxtng = thing_get(room->creatures_list);
        nxctrl = creature_control_get_from_thing(nxtng);
    } else
    {
        nxctrl = INVALID_CRTR_CONTROL;
    }
    if (!creature_control_invalid(nxctrl))
    {
        cctrl->next_in_room = room->creatures_list;
        nxctrl->prev_in_room = creatng->index;
    } else
    {
        cctrl->next_in_room = 0;
    }
    room->creatures_list = creatng->index;
    cctrl->flgfield_1 |= CCFlg_IsInRoomList;
    return true;
}

TbBool remove_creature_from_specific_room(struct Thing *creatng, struct Room *room, CreatureJob jobpref)
{
    struct Thing *sectng;
    struct CreatureControl *sectrl;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) == 0)
    {
        ERRORLOG("Attempt to remove a creature from room, but it isn't in any");
        return false;
    }
    int required_cap = get_required_room_capacity_for_job(jobpref, creatng->model);
    if (room->used_capacity >= required_cap) {
        room->used_capacity -= required_cap;
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) == 0)
        return false;
    struct Room* room = room_get(cctrl->work_room_id);
    if (room_is_invalid(room))
    {
        WARNLOG("Creature had invalid room index %d",(int)cctrl->work_room_id);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    CreatureJob jobpref = get_job_for_creature_state(get_creature_state_besides_interruptions(creatng));
    return remove_creature_from_specific_room(creatng, room, jobpref);
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
    struct Thing* rettng = INVALID_THING;
    if (room->slabs_count <= 0)
    {
        WARNLOG("Room with no slabs detected!");
        return rettng;
    }
    long selected = CREATURE_RANDOM(creatng, room->slabs_count);
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i != 0)
    {
        MapSubtlCoord stl_x = slab_subtile_center(slb_num_decode_x(i));
        MapSubtlCoord stl_y = slab_subtile_center(slb_num_decode_y(i));
        // Per room tile code
        struct Thing* tmptng = get_object_around_owned_by_and_matching_bool_filter(
            subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, matcher_cb);
        if (!thing_is_invalid(tmptng))
        {
            if (creature_can_navigate_to_with_storage(creatng, &tmptng->mappos, NavRtF_Default))
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

/**
 * Provides functionality of small random movements within a room where creature works.
 * @param creatng Creature to be moved within room.
 * @param room The room which bounds creature moves.
 * @param jobpref Job flag identifying the job given creature does in given room.
 * @return True if move setup was successful, false otherwise.
 */
TbBool creature_setup_adjacent_move_for_job_within_room_f(struct Thing *creatng, struct Room *room, CreatureJob jobpref, const char *func_name)
{
    struct Coord3d pos;
    TbBool result;
    unsigned long room_area = get_flags_for_job(jobpref) & (JoKF_WorkOnAreaBorder|JoKF_WorkOnAreaCenter);
    switch (room_area)
    {
    case JoKF_WorkOnAreaBorder:
        result = person_get_somewhere_adjacent_in_room_around_borders_f(creatng, room, &pos, func_name);
        break;
    //TODO CREATURE_MOVE Add support of central room_area
    case JoKF_WorkOnAreaCenter:
    case (JoKF_WorkOnAreaBorder|JoKF_WorkOnAreaCenter):
        result = person_get_somewhere_adjacent_in_room_f(creatng, room, &pos, func_name);
        break;
    default:
        WARNLOG("%s: Invalid room area flags 0x%04x for job %s.",func_name,(int)room_area,creature_job_code_name(jobpref));
        result = person_get_somewhere_adjacent_in_room_f(creatng, room, &pos, func_name);
        break;
    }
    if (result)
    {
        if (!setup_person_move_to_position_f(creatng, pos.x.stl.num, pos.y.stl.num, NavRtF_Default, func_name)) {
            ERRORLOG("%s: Cannot move %s index %d in %s room",func_name,thing_model_name(creatng),(int)creatng->index,room_code_name(room->kind));
            result = false;
        }
    } else {
        WARNLOG("%s: No position to move %s index %d in %s room",func_name,thing_model_name(creatng),(int)creatng->index,room_code_name(room->kind));
    }
    return result;
}

/**
 * Provides functionality of large random movements within a room, or a path into it.
 * @param creatng Creature to be moved into room.
 * @param room The room within which final position is selected.
 * @param jobpref Job flag identifying the job given creature does in given room.
 * @return True if move setup was successful, false otherwise.
 */
TbBool creature_setup_random_move_for_job_in_room_f(struct Thing *creatng, struct Room *room, CreatureJob jobpref, NaviRouteFlags nav_flags, const char *func_name)
{
    struct Coord3d pos;
    TbBool result;
    unsigned long room_area = get_flags_for_job(jobpref) & (JoKF_WorkOnAreaBorder|JoKF_WorkOnAreaCenter);
    switch (room_area)
    {
    case JoKF_WorkOnAreaBorder:
        result = find_random_position_at_area_of_room(&pos, room, RoArC_BORDER, creatng);
        break;
    case JoKF_WorkOnAreaCenter:
        result = find_random_position_at_area_of_room(&pos, room, RoArC_CENTER, creatng);
        break;
    case (JoKF_WorkOnAreaBorder|JoKF_WorkOnAreaCenter):
        result = find_random_valid_position_for_thing_in_room(creatng, room, &pos);
        break;
    default:
        WARNLOG("%s: Invalid room area flags 0x%04x for job %s.",func_name,(int)room_area,creature_job_code_name(jobpref));
        result = find_random_valid_position_for_thing_in_room(creatng, room, &pos);
        break;
    }
    if (result)
    {
        if (!setup_person_move_to_position_f(creatng, pos.x.stl.num, pos.y.stl.num, nav_flags, func_name)) {
            SYNCDBG(4,"%s: Cannot move %s index %d in %s room",func_name,thing_model_name(creatng),(int)creatng->index,room_code_name(room->kind));
            result = false;
        }
    } else {
        SYNCDBG(4,"%s: No position to move %s index %d in %s room",func_name,thing_model_name(creatng),(int)creatng->index,room_code_name(room->kind));
    }
    return result;
}

TbBool room_is_correct_to_perform_job(const struct Thing *creatng, const struct Room *room, CreatureJob jobpref)
{
    if ((get_room_for_job(jobpref) != RoK_NONE) && (room->kind != get_room_for_job(jobpref))) {
        return false;
    }
    if (creatng->owner == room->owner)
    {
        if (creatng->model == get_players_special_digger_model(creatng->owner)) {
            if ((get_flags_for_job(jobpref) & JoKF_OwnedDiggers) == 0)
                return false;
        } else {
            if ((get_flags_for_job(jobpref) & JoKF_OwnedCreatures) == 0)
                return false;
        }
    } else
    {
        if (creatng->model == get_players_special_digger_model(creatng->owner)) {
            if ((get_flags_for_job(jobpref) & JoKF_EnemyDiggers) == 0)
                return false;
        } else {
            if ((get_flags_for_job(jobpref) & JoKF_EnemyCreatures) == 0)
                return false;
        }
    }
    return true;
}

/**
 * Sends creature to a room where it should be performing a job.
 * @param creatng The creature to be sent.
 * @param room Target room.
 * @param jobpref The job to be performed in that room.
 */
short send_creature_to_room(struct Thing *creatng, struct Room *room, CreatureJob jobpref)
{
    SYNCDBG(16,"Starting for %s (owner %d) and room %s",thing_model_name(creatng),(int)creatng->owner,room_code_name(room->kind));
    // Job selection is based on subtile, not on room - so select a subtile within the room
    MapSubtlCoord stl_x = slab_subtile(slb_num_decode_x(room->slabs_list), 0);
    MapSubtlCoord stl_y = slab_subtile(slb_num_decode_y(room->slabs_list), 0);
    if (!creature_can_do_job_near_position(creatng, stl_x, stl_y, jobpref, JobChk_SetStateOnFail|JobChk_PlayMsgOnFail)) {
        SYNCDBG(16,"Cannot assign job %s in room %s to %s (owner %d)",creature_job_code_name(jobpref),room_code_name(room->kind),thing_model_name(creatng),(int)creatng->owner);
        return 0;
    }
    return send_creature_to_job_near_position(creatng, stl_x, stl_y, jobpref);
}

/**
 *
 * @param thing
 * @param room
 * @param move_flags
 * @return
 * @deprecated To be replaced by creature_setup_random_move_for_job_in_room().
 */
TbBool setup_random_head_for_room(struct Thing *thing, struct Room *room, unsigned char move_flags)
{
    struct Coord3d pos;
    if (room->kind == RoK_ENTRANCE)
    {
        pos.x.val = subtile_coord_center(room->central_stl_x);
        pos.y.val = subtile_coord_center(room->central_stl_y);
        pos.z.val = subtile_coord(1,0);
    } else
    {
        if (!find_random_valid_position_for_thing_in_room(thing, room, &pos)) {
            return false;
        }
    }
    return setup_person_move_to_coord(thing, &pos, move_flags);
}

/**
 * Returns need strength of new creature doing work on given room role in dungeon.
 * @param dungeon
 * @param rrole
 * @return Gives integer value; 0 if no worker is needed; 1 if worker is not recommended;
 *     2 if worker is recommended; 3 if worker is badly needed.
 * @note This was worker_needed_in_dungeons_room_kind() before roles were introduced.
 */
int worker_needed_in_dungeons_room_role(const struct Dungeon *dungeon, RoomRole rrole)
{
    if ((rrole & RoRoF_Research) != 0)
    {
        if (dungeon->current_research_idx < 0)
            return 0;
        if (has_new_rooms_to_research(dungeon))
            return 2;
        return 1;
    }
    if ((rrole & RoRoF_CratesManufctr) != 0)
    {
        // When we have low gold, allow working on any manufacture - we'll sell the crates
        long amount = get_doable_manufacture_with_minimal_amount_available(dungeon, NULL, NULL);
        GoldAmount net_gold = get_dungeon_money_less_cost(dungeon);
        if (amount >= MANUFACTURED_ITEMS_LIMIT)
            return 0;
        if (net_gold < 0)
            return 3;
        if (net_gold < dungeon->creatures_total_pay)
            return 2;
        // If gold amount is fine, do manufacture only if there are items which we don't have in stock
        if (amount > 0)
            return 1;
        return 2;
    }
    if ((rrole & RoRoF_CrTrainExp) != 0)
    {
        GoldAmount net_gold = get_dungeon_money_less_cost(dungeon);
        if (net_gold < 0)
            return 0;
        if (net_gold < dungeon->creatures_total_pay)
            return 1;
        return 2;
    }
    if ((rrole & RoRoF_CrScavenge) != 0)
    {
        GoldAmount net_gold = get_dungeon_money_less_cost(dungeon);
        if (net_gold < dungeon->creatures_total_pay)
            return 0;
        if (net_gold < 2 * dungeon->creatures_total_pay)
            return 1;
        return 2;
    }
    return 1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
