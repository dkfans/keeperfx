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
DLLIMPORT unsigned char _DK_remove_creature_from_work_room(struct Thing *creatng);
DLLIMPORT short _DK_send_creature_to_room(struct Thing *creatng, struct Room *room);

/******************************************************************************/
struct Room *get_room_creature_works_in(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
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
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
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
    struct Dungeon *dungeon;
    if (creatng->light_id != 0) {
        light_delete_light(creatng->light_id);
        creatng->light_id = 0;
    }
    if (creature_affected_by_spell(creatng, SplK_Speed))
        terminate_thing_spell_effect(creatng, SplK_Speed);
    if (creature_affected_by_spell(creatng, SplK_Invisibility))
        terminate_thing_spell_effect(creatng, SplK_Invisibility);
    dungeon = get_dungeon(room->owner);
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
    struct Room *room;
    PlayerNumber plyr_idx;
    room = get_room_creature_works_in(creatng);
    if (!room_exists(room)) {
        SYNCDBG(6,"The %s worked in a room which no longer exists",thing_model_name(creatng));
        return false;
    }
    plyr_idx = room->owner;
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
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

TbBool output_message_room_related_from_computer_or_player_action(long msg_idx)
{
    long delay;
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    if ((dungeon->computer_enabled & 0x01) != 0) {
        delay = 500;
    } else {
        delay = 0;
    }
    return output_message(msg_idx, delay, 1);
}

TbBool creature_move_to_place_in_room(struct Thing *creatng, struct Room *room)
{
    struct Coord3d pos;
    TbBool result;
    if (room->kind == RoK_ENTRANCE)
    {
        pos.x.val = subtile_coord_center(room->central_stl_x);
        pos.y.val = subtile_coord_center(room->central_stl_y);
        result = true;
    } else
    {
        result = find_random_valid_position_for_thing_in_room(creatng, room, &pos);
    }
    if (result)
    {
        if (!setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0)) {
            result = false;
        }
    }
    return result;
}

short send_creature_to_room(struct Thing *creatng, struct Room *room)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(6,"Starting for %s (owner %d) and room %s",thing_model_name(creatng),(int)creatng->owner,room_code_name(room->kind));
    CreatureJob jobpref;
    jobpref = get_job_for_room(room->kind, false);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    struct Coord3d pos;
    //return _DK_send_creature_to_room(creatng, room);
    if ((jobpref & crstat->jobs_not_do) != 0)
    {
        anger_apply_anger_to_creature(creatng, crstat->annoy_will_not_do_job, AngR_Other, 1);
        external_set_thing_state(creatng, 140);
        cctrl->field_282 = 50;
        return 0;
    }
    switch (room->kind)
    {
    case RoK_ENTRANCE:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (creature_move_to_place_in_room(creatng, room))
        {
            creatng->continue_state = CrSt_CreatureFired;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    case RoK_TREASURE:
        if (creatng->owner != room->owner) {
            return 0;
        }
        if (creatng->model == get_players_special_digger_breed(room->owner))
        {
            if (creatng->creature.gold_carried > 0)
            {
                if (creature_move_to_place_in_room(creatng, room))
                {
                    creatng->continue_state = CrSt_ImpDropsGold;
                    cctrl->target_room_id = room->index;
                    return 1;
                }
            }
        } else
        {
            if (room->used_capacity > 0)
            {
                if (creature_move_to_place_in_room(creatng, room))
                {
                  creatng->continue_state = CrSt_CreatureTakeSalary;
                  cctrl->target_room_id = room->index;
                  return 1;
                }
            }
        }
        return 0;
    case RoK_LIBRARY:
        if (creatng->owner != room->owner) {
            return 0;
        }
        if (!creature_can_do_research(creatng))
        {
            struct Dungeon *dungeon;
            dungeon = get_dungeon(room->owner);
            if (!is_neutral_thing(creatng) && (dungeon->field_F78 < 0))
            {
                if (is_my_player_number(dungeon->owner)) {
                    output_message(SMsg_NoMoreReseach, 500, true);
                }
            }
            set_start_state(creatng);
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            if (is_my_player_number(room->owner))
            {
                output_message_room_related_from_computer_or_player_action(SMsg_LibraryTooSmall);
            }
            return 0;
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos))
        {
            if (setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
            {
              creatng->continue_state = CrSt_AtResearchRoom;
              cctrl->target_room_id = room->index;
              return 1;
            }
        }
        return 0;
    case RoK_PRISON:
        if ((creatng->owner == room->owner) && (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (creature_move_to_place_in_room(creatng, room))
        {
            cctrl->flgfield_1 |= 0x02;
            creatng->continue_state = CrSt_CreatureArrivedAtPrison;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    case RoK_TORTURE:
        if ((creatng->owner == room->owner) && (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            if (is_my_player_number(room->owner))
            {
                output_message_room_related_from_computer_or_player_action(SMsg_TortureTooSmall);
            }
            return 0;
        }
        if (creature_move_to_place_in_room(creatng, room))
        {
            if ((creatng->owner == room->owner) && creature_has_job(creatng, Job_KINKY_TORTURE))
                creatng->continue_state = CrSt_AtKinkyTortureRoom;
            else
                creatng->continue_state = CrSt_AtTortureRoom;
            cctrl->target_room_id = room->index;
            cctrl->flgfield_1 |= 0x02;
            return 1;
        }
        return 0;
    case RoK_TRAINING:
        if (creatng->owner != room->owner) {
            return 0;
        }
        if (!creature_can_be_trained(creatng)) {
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            if (is_my_player_number(room->owner))
            {
                output_message_room_related_from_computer_or_player_action(SMsg_TrainingTooSmall);
            }
            return 0;
        }
        if (creatng->model == get_players_special_digger_breed(room->owner))
        {
            if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
              && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
            {
                creatng->continue_state = CrSt_AtTrainingRoom;
                cctrl->target_room_id = room->index;
                cctrl->digger.last_did_job = SDLstJob_UseTraining4;
                return 1;
            }
        }
        else
        {
            if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
              && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
            {
                creatng->continue_state = CrSt_AtTrainingRoom;
                cctrl->target_room_id = room->index;
                return 1;
            }
        }
        return 0;
    case RoK_WORKSHOP:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (!creature_can_do_manufacturing(creatng)) {
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            if (is_my_player_number(room->owner))
            {
                output_message_room_related_from_computer_or_player_action(SMsg_WorkshopTooSmall);
            }
            return 0;
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
          && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
        {
            creatng->continue_state = CrSt_AtWorkshopRoom;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    case RoK_SCAVENGER:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (!creature_can_do_scavenging(creatng)) {
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            if (is_my_player_number(room->owner))
            {
                output_message_room_related_from_computer_or_player_action(SMsg_ScavengeTooSmall);
            }
            return 0;
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
          && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
        {
            creatng->continue_state = CrSt_AtScavengerRoom;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    case RoK_TEMPLE:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (!creature_can_do_manufacturing(creatng)) {
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            if (is_my_player_number(room->owner))
            {
                output_message_room_related_from_computer_or_player_action(SMsg_TempleTooSmall);
            }
            return 0;
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
          && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
        {
            creatng->continue_state = CrSt_AtTemple;
            cctrl->target_room_id = room->index;
            process_temple_cure(creatng);
            return 1;
        }
        return 0;
    case RoK_BARRACKS:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            if (is_my_player_number(room->owner))
            {
                output_message_room_related_from_computer_or_player_action(SMsg_BarracksTooSmall);
            }
            return 0;
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
          && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
        {
            creatng->continue_state = CrSt_AtBarrackRoom;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    case RoK_GARDEN:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
          && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
        {
            creatng->continue_state = CrSt_CreatureArrivedAtGarden;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    case RoK_LAIR:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        cctrl->field_21 = 0;
        cctrl->max_speed = calculate_correct_creature_maxspeed(creatng);
        if (!creature_free_for_sleep(creatng)) {
            return 0;
        }
        if (creature_has_lair_room(creatng) && (room->index == cctrl->lair_room_id))
        {
            if (creature_move_to_home_lair(creatng))
            {
                creatng->continue_state = CrSt_CreatureGoingHomeToSleep;
                return 1;
            }
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
          && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
        {
            creatng->continue_state = CrSt_CreatureChangeLair;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    case RoK_GUARDPOST:
        if ((creatng->owner != room->owner) || (creatng->model == get_players_special_digger_breed(room->owner))) {
            return 0;
        }
        if (!room_has_enough_free_capacity_for_creature(room, creatng))
        {
            return 0;
        }
        if (find_first_valid_position_for_thing_in_room(creatng, room, &pos)
          && setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0))
        {
            creatng->continue_state = CrSt_AtGuardPostRoom;
            cctrl->target_room_id = room->index;
            return 1;
        }
        return 0;
    default:
        break;
    }
    return 0;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
