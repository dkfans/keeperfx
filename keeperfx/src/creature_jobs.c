/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_jobs.c
 *     Creature job assign and verify functions.
 * @par Purpose:
 *     Defines creature jobs configuration and various job-related routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     27 Aug 2013 - 07 Oct 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_jobs.h"
#include "globals.h"

#include "bflib_math.h"

#include "thing_data.h"
#include "thing_stats.h"
#include "thing_navigate.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_magic.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "room_jobs.h"
#include "power_hand.h"
#include "player_instances.h"
#include "game_legacy.h"

#include "creature_states_prisn.h"
#include "creature_states_rsrch.h"
#include "creature_states_scavn.h"
#include "creature_states_spdig.h"
#include "creature_states_tortr.h"
#include "creature_states_train.h"
#include "creature_states_wrshp.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_creature_find_and_perform_anger_job(struct Thing *creatng);
DLLIMPORT long _DK_attempt_job_preference(struct Thing *creatng, long jobpref);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool attempt_job_work_in_room(struct Thing *creatng, CreatureJob jobpref);
TbBool attempt_job_in_state_on_room_content(struct Thing *creatng, CreatureJob jobpref);
TbBool attempt_job_move_to_event(struct Thing *creatng, CreatureJob jobpref);
TbBool attempt_job_in_state_internal(struct Thing *creatng, CreatureJob jobpref);

const struct NamedCommand creature_job_assign_func_type[] = {
  {"work_in_room",             1},
  {"in_state_on_room_content", 2},
  {"move_to_event",            3},
  {"in_state_internal",        4},
  {"none",                     5},
  {NULL,                       0},
};

Creature_Job_Assign_Func creature_job_assign_func_list[] = {
  NULL,
  attempt_job_work_in_room,
  attempt_job_in_state_on_room_content,
  attempt_job_move_to_event,
  attempt_job_in_state_internal,
  NULL,
  NULL,
};
/******************************************************************************/
TbBool set_creature_assigned_job(struct Thing *thing, CreatureJob new_job)
{
    struct CreatureControl *cctrl;
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("The %s index %d has invalid control",thing_model_name(thing),(int)thing->index);
        return false;
    }
    cctrl->job_assigned = new_job;
    SYNCLOG("Assigned job %s for %s index %d owner %d",creature_job_code_name(new_job),thing_model_name(thing),(int)thing->index,(int)thing->owner);
    return true;
}

/**
 * Returns if the given job is creature's primary or secondary job.
 * @param thing
 * @param job_kind
 * @return
 */
TbBool creature_has_job(const struct Thing *thing, CreatureJob job_kind)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (crstat->job_primary & job_kind) || (crstat->job_secondary & job_kind);
}

TbBool creature_free_for_anger_job(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    cctrl = creature_control_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    return ((cctrl->spell_flags & CSAfF_Unkn0800) == 0)
        && (dungeon->must_obey_turn == 0)
        && ((cctrl->spell_flags & CSAfF_Speed) == 0)
        && !thing_is_picked_up(thing) && !is_thing_passenger_controlled(thing);
}

long creature_find_and_perform_anger_job(struct Thing *thing)
{
    return _DK_creature_find_and_perform_anger_job(thing);
}

TbBool creature_can_do_job_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob jobpref)
{
    if (jobpref & Job_TRAIN)
    {
        return creature_can_be_trained(creatng) && player_can_afford_to_train_creature(creatng);
    }
    if (jobpref & Job_RESEARCH)
    {
        return creature_can_do_research(creatng);
    }
    if (jobpref & Job_MANUFACTURE)
    {
        return creature_can_do_manufacturing(creatng);
    }
    if (jobpref & Job_SCAVENGE)
    {
        return creature_can_do_scavenging(creatng) && player_can_afford_to_scavenge_creature(creatng);
    }
    if (jobpref & Job_FREEZE_PRISONERS)
    {
        struct Room *room;
        room = find_room_for_thing_with_used_capacity(creatng, creatng->owner, get_room_for_job(Job_FREEZE_PRISONERS), 0, 1);
        return creature_instance_is_available(creatng, 7) && !room_is_invalid(room);
    }
    if (jobpref & Job_GUARD)
    {
        return true;
    }
    if (jobpref & Job_TEMPLE_PRAY)
    {
        return true;
    }
    if (jobpref & Job_KINKY_TORTURE)
    {
        return true;
    }
    if (jobpref & Job_SEEK_THE_ENEMY)
    {
        return true;
    }
    if (jobpref & Job_EXPLORE)
    {
        return true;
    }
    if (jobpref & Job_FIGHT)
    {
        struct Event *event;
        event = get_event_of_type_for_player(EvKind_Fight, creatng->owner);
        return !event_is_invalid(event);
    }
    return false;
}

TbBool attempt_job_work_in_room(struct Thing *creatng, CreatureJob jobpref)
{
    struct Coord3d pos;
    struct Room *room;
    RoomKind rkind;
    rkind = get_room_for_job(jobpref);
    room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, rkind, 0, 1);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        return false;
    }
    if (!get_arrive_at_state_for_room(room->kind))
    {
        ERRORLOG("No arrive at state for %s room", room_code_name(room->kind));
        return false;
    }
    if (!setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, 0)) {
        return false;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = get_arrive_at_state_for_room(room->kind);
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_job_in_state_on_room_content(struct Thing *creatng, CreatureJob jobpref)
{
    struct Room *room;
    RoomKind rkind;
    rkind = get_room_for_job(jobpref);
    room = find_room_for_thing_with_used_capacity(creatng, creatng->owner, rkind, 0, 1);
    if (room_is_invalid(room)) {
        return false;
    }
    internal_set_thing_state(creatng, get_initial_state_for_job(jobpref));
    return true;
}

TbBool attempt_job_move_to_event(struct Thing *creatng, CreatureJob jobpref)
{
    EventKind evkind;
    struct Event *event;
    evkind = get_event_for_job(jobpref);
    event = get_event_of_type_for_player(evkind, creatng->owner);
    if (event_is_invalid(event)) {
        return false;
    }
    if (!setup_person_move_to_position(creatng, event->mappos_x, event->mappos_y, 0)) {
        return false;
    }
    creatng->continue_state = get_initial_state_for_job(jobpref);
    return true;
}

TbBool attempt_job_in_state_internal(struct Thing *creatng, CreatureJob jobpref)
{
    struct CreatureControl *cctrl;
    CrtrStateId crstate;
    crstate = get_initial_state_for_job(jobpref);
    cctrl = creature_control_get_from_thing(creatng);
    internal_set_thing_state(creatng, crstate);
    if (crstate == CrSt_SeekTheEnemy) {
        cctrl->word_9A = 0;
    }
    return true;
}

long attempt_job_preference(struct Thing *creatng, long jobpref)
{
    //return _DK_attempt_job_preference(creatng, jobpref);
    long i,n;
    unsigned long k;
    // Start checking at random job
    if (crtr_conf.jobs_count < 1) {
        return false;
    }
    n = ACTION_RANDOM(crtr_conf.jobs_count);
    for (i=0; i < crtr_conf.jobs_count; i++, n = (n+1)%crtr_conf.jobs_count)
    {
        if (n == 0)
            continue;
        k = 1 << (n-1);
        if (jobpref & k)
        {
            if (creature_can_do_job_for_player(creatng, creatng->owner, k))
            {
                struct CreatureJobConfig *jobcfg;
                jobcfg = get_config_for_job(k);
                if (jobcfg->func_assign != NULL)
                {
                    if (jobcfg->func_assign(creatng, k)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

TbBool attempt_job_secondary_preference(struct Thing *creatng, long jobpref)
{
    long i;
    unsigned long k;
    // Count the amount of jobs set
    i = 0;
    k = jobpref;
    while (k)
    {
        k >>= 1;
        i++;
    }
    if (i <= 0) {
        return false;
    }
    unsigned long select_val,select_curr,select_delta;
    select_val = ACTION_RANDOM(512);
    select_delta = 512 / i;
    select_curr = select_delta;
    // For some reason, this is a bit different than attempt_job_preference().
    // Probably needs unification
    if (jobpref & Job_TRAIN)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        if (creature_can_be_trained(creatng))
        {
            struct Room *room;
            room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_TRAINING, 0, 1);
            if (!room_is_invalid(room))
            {
                if (send_creature_to_room(creatng, room)) {
                  return true;
                }
            }
        }
    }
    if (jobpref & Job_RESEARCH)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        {
            if (creature_can_do_research(creatng))
            {
                struct Room *room;
                room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_LIBRARY, 0, 1);
                if (!room_is_invalid(room))
                {
                    if (send_creature_to_room(creatng, room)) {
                      return true;
                    }
                }
            }
        }
    }
    if (jobpref & Job_MANUFACTURE)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        if (creature_can_do_manufacturing(creatng))
        {
            struct Room *room;
            room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_WORKSHOP, 0, 1);
            if (!room_is_invalid(room))
            {
                if (send_creature_to_room(creatng, room)) {
                  return true;
                }
            }
        }
    }
    if (jobpref & Job_SCAVENGE)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        if (creature_can_do_scavenging(creatng) && player_can_afford_to_scavenge_creature(creatng))
        {
            struct Room *room;
            room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_SCAVENGER, 0, 1);
            if (!room_is_invalid(room))
            {
                if (send_creature_to_room(creatng, room)) {
                  return true;
                }
            }
        }
    }
    if (jobpref & Job_KINKY_TORTURE)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        {
            struct Room *room;
            room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_TORTURE, 0, 1);
            if (!room_is_invalid(room))
            {
                if (send_creature_to_room(creatng, room)) {
                  return true;
                }
            }
        }
    }
    if (jobpref & Job_GUARD)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        {
            struct Room *room;
            room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_GUARDPOST, 0, 1);
            if (!room_is_invalid(room))
            {
                if (send_creature_to_room(creatng, room)) {
                  return true;
                }
            }
        }
    }
    if (jobpref & Job_FREEZE_PRISONERS)
    {
      if ( select_curr < select_val && creature_instance_is_available(creatng, 7) && find_room_for_thing_with_used_capacity(creatng, creatng->owner, 4, 0, 1) )
      {
        internal_set_thing_state(creatng, CrSt_CreatureFreezePrisoners);
        return 1;
      }
      select_curr += select_delta;
    }
    if (jobpref & Job_SEEK_THE_ENEMY)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        {
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(creatng);
            internal_set_thing_state(creatng, CrSt_SeekTheEnemy);
            cctrl->word_9A = 0;
            return true;
        }
    }
    if (jobpref & Job_EXPLORE)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        {
            internal_set_thing_state(creatng, CrSt_CreatureExploreDungeon);
            return true;
        }
    }
    if (jobpref & Job_TEMPLE_PRAY)
    {
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        {
            struct Room *room;
            room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_TEMPLE, 0, 1);
            if (!room_is_invalid(room))
            {
                if (send_creature_to_room(creatng, room)) {
                  return true;
                }
            }
        }
    }

    // If no job, give 1% chance of going to temple
    if (ACTION_RANDOM(100) == 0)
    {
        struct Room *room;
        room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_TEMPLE, 0, 1);
        if (!room_is_invalid(room))
        {
            if (send_creature_to_room(creatng, room)) {
              return true;
            }
        }
    }
    return 0;
}

TbBool creature_try_doing_secondary_job(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (game.play_gameturn - cctrl->field_2D3 <= 128) {
        return false;
    }
    cctrl->field_2D3 = game.play_gameturn;
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    return attempt_job_secondary_preference(creatng, crstat->job_secondary);
}

/******************************************************************************/
