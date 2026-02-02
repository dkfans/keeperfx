/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_rsrch.c
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
#include "pre_inc.h"
#include "creature_states_rsrch.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_jobs.h"
#include "creature_states_combt.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_library.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "post_inc.h"

/******************************************************************************/
TbBool creature_can_do_research(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    return (crconf->research_value > 0) && (dungeon->current_research_idx >= 0);
}

short at_research_room(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    if (!creature_can_do_research(thing))
    {
        if (!is_neutral_thing(thing) && (dungeon->current_research_idx < 0))
        {
            if (is_my_player_number(dungeon->owner))
                output_message(SMsg_NoMoreReseach, 500);
        }
        set_start_state(thing);
        return 0;
    }
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_role_for_job(Job_RESEARCH), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room, Job_RESEARCH))
    {
        set_start_state(thing);
        return 0;
    }
    if (!creature_setup_random_move_for_job_in_room(thing, room, Job_RESEARCH, NavRtF_Default))
    {
        ERRORLOG("The %s index %d can not move in research room", thing_model_name(thing),(int)thing->index);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = get_continue_state_for_job(Job_RESEARCH);
    cctrl->turns_at_job = 0;
    cctrl->research.job_stage = JobStage_MovingToPosition;
    return 1;
}

/**
 * Returns new research item index, or -1 if nothing to research.
 * @param dungeon
 * @return
 */
int get_next_research_item(const struct Dungeon *dungeon)
{
    if (dungeon->research_num == 0)
        return -1;
    for (long resnum = 0; resnum < dungeon->research_num; resnum++)
    {
        const struct ResearchVal* rsrchval = &dungeon->research[resnum];
        switch (rsrchval->rtyp)
        {
       case RsCat_Power:
            if ( (dungeon->magic_resrchable[rsrchval->rkind]) && (dungeon->magic_level[rsrchval->rkind] == 0) )
            {
                return resnum;
            }
            break;
        case RsCat_Room:
            if ((dungeon->room_buildable[rsrchval->rkind] & 1) == 0)
            {
                /// Need research
                if (dungeon->room_resrchable[rsrchval->rkind] == 1)
                    return resnum;
                /// Need research but may find room instantly
                else if (dungeon->room_resrchable[rsrchval->rkind] == 2)
                    return resnum;
                /// Need research AND already captured
                else if ((dungeon->room_resrchable[rsrchval->rkind] == 4) &&
                    (dungeon->room_buildable[rsrchval->rkind] & 2))
                {
                    return resnum;
                }
            }
            break;
        case RsCat_Creature:
            if (dungeon->creature_allowed[rsrchval->rkind] == 0)
            {
                return resnum;
            }
            break;
        case RsCat_None:
            break;
        default:
            ERRORLOG("Illegal research type %d while getting next research item",(int)rsrchval->rtyp);
            break;
        }
    }
    return -1;
}

TbBool has_new_rooms_to_research(const struct Dungeon *dungeon)
{    
    for (long resnum = 0; resnum < dungeon->research_num; resnum++)
    {
        const struct ResearchVal* rsrchval = &dungeon->research[resnum];
        if (rsrchval->rtyp == RsCat_Room)
        {
            if ((dungeon->room_buildable[rsrchval->rkind] & 1) == 0)
            {
                if ((dungeon->room_resrchable[rsrchval->rkind] == 1)
                    || (dungeon->room_resrchable[rsrchval->rkind] == 2)
                    || ((dungeon->room_resrchable[rsrchval->rkind] == 4) && (dungeon->room_buildable[rsrchval->rkind] & 2))
                    )
                {
                    return true;
                }
            }
        }
    }
    return false;
}

struct ResearchVal *get_players_current_research_val(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if ((dungeon->current_research_idx < 0) || (dungeon->current_research_idx >= DUNGEON_RESEARCH_COUNT))
        return NULL;
    return &dungeon->research[dungeon->current_research_idx];
}

TbBool force_complete_current_research(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    struct ResearchVal* rsrchval = get_players_current_research_val(plyr_idx);
    if (rsrchval != NULL)
    {
        if ( research_needed(rsrchval, dungeon) ) {
            dungeon->research_progress = rsrchval->req_amount << 8;
            return true;
        }
    }
    dungeon->current_research_idx = get_next_research_item(dungeon);
    rsrchval = get_players_current_research_val(plyr_idx);
    if (rsrchval != NULL)
    {
        dungeon->research_progress = rsrchval->req_amount << 8;
        return true;
    }
    return false;
}

/**
 * Does a step of researching.
 * Informs if the research cycle should end.
 * @param thing
 */
CrCheckRet process_research_function(struct Thing *creatng)
{
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (dungeon_invalid(dungeon)) {
        SYNCDBG(9,"The %s index %d cannot work as player %d has no dungeon",
            thing_model_name(creatng), (int)creatng->index, (int)creatng->owner);
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    if (!creature_can_do_research(creatng)) {
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    struct Room* room = get_room_creature_works_in(creatng);
    if ( !room_still_valid_as_type_for_thing(room, get_room_role_for_job(Job_RESEARCH), creatng) ) {
        WARNLOG("Room %s owned by player %d is bad work place for %s index %d owner %d",
            room_code_name(room->kind), (int)room->owner, thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        remove_creature_from_work_room(creatng);
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    long work_value = compute_creature_work_value_for_room_role(creatng, RoRoF_Research, room->efficiency);
    SYNCDBG(19,"The %s index %d produced %d research points",thing_model_name(creatng),(int)creatng->index,(int)work_value);
    dungeon->total_research_points += work_value;
    dungeon->research_progress += work_value;
    return CrCkRet_Available;
}

short researching(struct Thing *thing)
{
    TRACE_THING(thing);
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    if (is_neutral_thing(thing))
    {
        ERRORLOG("Neutral %s index %d cannot do research",thing_model_name(thing),(int)thing->index);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_Unchanged;
    }
    if (!creature_can_do_research(thing))
    {
        if (!is_neutral_thing(thing) && (dungeon->current_research_idx < 0))
        {
            if (is_my_player_number(dungeon->owner))
                output_message(SMsg_NoMoreReseach, 500);
        }
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_Unchanged;
    }
    // Get and verify working room
    struct Room* room = get_room_thing_is_on(thing);
    if (creature_job_in_room_no_longer_possible(room, Job_RESEARCH, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }

    if (room->used_capacity > room->total_capacity)
    {
        output_room_message(room->owner, room->kind, OMsg_RoomTooSmall);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetOk;
    }
    process_research_function(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ( (game.play_gameturn - dungeon->last_research_complete_gameturn < 50)
      && ((game.play_gameturn + thing->index) & 0x03) == 0)
    {
        external_set_thing_state(thing, CrSt_CreatureBeHappy);
        cctrl->countdown = 50;
        cctrl->mood.last_mood_sound_turn = 0;
        return CrStRet_Modified;
    }
    if (cctrl->instance_id != CrInst_NULL)
      return 1;
    cctrl->turns_at_job++;
    // Shall we do some "Standing and thinking"
    if (cctrl->turns_at_job <= 128)
    {
      if (cctrl->research.job_stage == JobStage_MovingToPosition)
      {
          // Do some random thinking
          if ((cctrl->turns_at_job % 16) == 0)
          {
              long i = THING_RANDOM(thing, DEGREES_180) - DEGREES_90;
              cctrl->research.random_thinking_angle = ((long)thing->move_angle_xy + i) & ANGLE_MASK;
              cctrl->research.job_stage = JobStage_TurningToFace;
          }
      } else
      {
          // Look at different direction while thinking
          if (creature_turn_to_face_angle(thing, cctrl->research.random_thinking_angle) < DEGREES_10)
          {
              cctrl->research.job_stage = JobStage_MovingToPosition;
          }
      }
      return 1;
    }
    // Finished "Standing and thinking" - make "new idea" effect and go to next position
    if (!creature_setup_random_move_for_job_in_room(thing, room, Job_RESEARCH, NavRtF_Default))
    {
        ERRORLOG("Cannot move %s index %d in %s room", thing_model_name(thing),(int)thing->index,room_code_name(room->kind));
        set_start_state(thing);
        return 1;
    }
    thing->continue_state = get_continue_state_for_job(Job_RESEARCH);
    cctrl->turns_at_job = 0;
    cctrl->research.job_stage = JobStage_MovingToPosition;
    if (cctrl->exp_level < 3)
    {
        create_effect(&thing->mappos, TngEff_RoomSparkeSmall, thing->owner);
    } else
    if (cctrl->exp_level < 6)
    {
        create_effect(&thing->mappos, TngEff_RoomSparkeMedium, thing->owner);
    } else
    {
        create_effect(&thing->mappos, TngEff_RoomSparkeLarge, thing->owner);
    }
    return 1;
}

/******************************************************************************/
