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
#include "pre_inc.h"
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
#include "creature_jobs.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "room_jobs.h"
#include "room_list.h"
#include "power_hand.h"
#include "spdigger_stack.h"
#include "player_instances.h"
#include "player_utils.h"
#include "game_legacy.h"
#include "gui_soundmsgs.h"

#include "creature_states_prisn.h"
#include "creature_states_rsrch.h"
#include "creature_states_scavn.h"
#include "creature_states_spdig.h"
#include "creature_states_tortr.h"
#include "creature_states_train.h"
#include "creature_states_wrshp.h"
#include "creature_states_lair.h"
#include "creature_states_pray.h"
#include "post_inc.h"

/******************************************************************************/
TbBool creature_can_do_job_always_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_training_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_research_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_manufacturing_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_scavenging_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_freeze_prisoners_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_join_fight_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool creature_can_do_barracking_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);

TbBool attempt_job_work_in_room_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool attempt_job_in_state_on_room_content_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool attempt_job_move_to_event_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool attempt_job_in_state_internal_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);

TbBool creature_can_do_job_always_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_research_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_training_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_manufacturing_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_scavenging_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_place_in_vault_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_take_salary_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool creature_can_take_sleep_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);

TbBool attempt_job_work_in_room_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);
TbBool attempt_job_work_in_room_and_cure_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);
TbBool attempt_job_sleep_in_lair_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);
TbBool attempt_job_in_state_internal_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);

const struct NamedCommand creature_job_player_check_func_type[] = {
  {"can_do_job_always",        1},
  {"can_do_training",          2},
  {"can_do_research",          3},
  {"can_do_manufacturing",     4},
  {"can_do_scavenging",        5},
  {"can_freeze_prisoners",     6},
  {"can_join_fight",           7},
  {"can_do_barracking",        8},
  {"none",                     9},
  {NULL,                       0},
};

Creature_Job_Player_Check_Func creature_job_player_check_func_list[] = {
  NULL,
  creature_can_do_job_always_for_player,
  creature_can_do_training_for_player,
  creature_can_do_research_for_player,
  creature_can_do_manufacturing_for_player,
  creature_can_do_scavenging_for_player,
  creature_can_freeze_prisoners_for_player,
  creature_can_join_fight_for_player,
  creature_can_do_barracking_for_player,
  NULL,
  NULL,
};

const struct NamedCommand creature_job_player_assign_func_type[] = {
  {"work_in_room",             1},
  {"in_state_on_room_content", 2},
  {"move_to_event",            3},
  {"in_state_internal",        4},
  {"none",                     5},
  {NULL,                       0},
};

Creature_Job_Player_Assign_Func creature_job_player_assign_func_list[] = {
  NULL,
  attempt_job_work_in_room_for_player,
  attempt_job_in_state_on_room_content_for_player,
  attempt_job_move_to_event_for_player,
  attempt_job_in_state_internal_for_player,
  NULL,
  NULL,
};

const struct NamedCommand creature_job_coords_check_func_type[] = {
  {"can_do_job_always",        1},
  {"can_do_research",          2},
  {"can_do_training",          3},
  {"can_do_manufacturing",     4},
  {"can_do_scavenging",        5},
  {"can_place_in_vault",       6},
  {"can_take_salary",          7},
  {"can_take_sleep",           8},
  {"none",                     9},
  {NULL,                       0},
};

Creature_Job_Coords_Check_Func creature_job_coords_check_func_list[] = {
  NULL,
  creature_can_do_job_always_near_pos,
  creature_can_do_research_near_pos,
  creature_can_do_training_near_pos,
  creature_can_do_manufacturing_near_pos,
  creature_can_do_scavenging_near_pos,
  creature_can_place_in_vault_near_pos,
  creature_can_take_salary_near_pos,
  creature_can_take_sleep_near_pos,
  NULL,
  NULL,
};

const struct NamedCommand creature_job_coords_assign_func_type[] = {
  {"work_in_room",             1},
  {"work_in_room_and_cure",    2},
  {"sleep_in_lair",            3},
  {"in_state_internal",        4},
  {"none",                     5},
  {NULL,                       0},
};

Creature_Job_Coords_Assign_Func creature_job_coords_assign_func_list[] = {
  NULL,
  attempt_job_work_in_room_near_pos,
  attempt_job_work_in_room_and_cure_near_pos,
  attempt_job_sleep_in_lair_near_pos,
  attempt_job_in_state_internal_near_pos,
  NULL,
  NULL,
};

/******************************************************************************/
TbBool set_creature_assigned_job(struct Thing *thing, CreatureJob new_job)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("The %s index %d has invalid control",thing_model_name(thing),(int)thing->index);
        return false;
    }
    cctrl->job_assigned = new_job;
    SYNCDBG(6,"Assigned job %s for %s index %d owner %d",creature_job_code_name(new_job),thing_model_name(thing),(int)thing->index,(int)thing->owner);
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
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    return ((crconf->job_primary & job_kind) != 0) || ((crconf->job_secondary & job_kind) != 0);
}

TbBool creature_free_for_anger_job(struct Thing *creatng)
{
    return !creature_affected_by_call_to_arms(creatng)
    && !player_uses_power_obey(creatng->owner)
    && !creature_under_spell_effect(creatng, CSAfF_Chicken)
    && !thing_is_picked_up(creatng) && !is_thing_directly_controlled(creatng);
}

TbBool attempt_anger_job_destroy_rooms(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureVandaliseRooms)) {
        return false;
    }
    struct Coord3d pos;
    struct Room* room = find_nearest_room_to_vandalise(creatng, creatng->owner, NavRtF_NoOwner);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        return false;
    }
    if (!creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_NoOwner)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureVandaliseRooms)) {
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_NoOwner)) {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = CrSt_CreatureVandaliseRooms;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_anger_job_steal_gold(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureStealGold)) {
        return false;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if (creatng->creature.gold_carried >= crconf->gold_hold) {
        return false;
    }
    struct Coord3d pos;
    struct Room* room = find_nearest_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, RoRoF_GoldStorage, NavRtF_NoOwner, 1);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        return false;
    }
    if (!creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_NoOwner)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureStealGold)) {
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_NoOwner))
    {
        ERRORLOG("Cannot setup head for treasury.");
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = CrSt_CreatureSearchForGoldToStealInRoom1;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_anger_job_kill_creatures(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureKillCreatures)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureKillCreatures)) {
        return false;
    }
    return true;
}

TbBool attempt_anger_job_kill_diggers(struct Thing* creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureKillDiggers)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureKillDiggers)) {
        return false;
    }
    return true;
}

TbBool attempt_anger_job_leave_dungeon(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureLeaves)) {
        return false;
    }
    struct Room* room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, get_room_role_for_job(Job_EXEMPT), NavRtF_Default);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureLeaves)) {
        return false;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, Job_EXEMPT, NavRtF_Default)) {
        return false;
    }
    creatng->continue_state = CrSt_CreatureLeaves;
    return true;
}

TbBool attempt_anger_job_damage_walls(struct Thing *creatng)
{
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureDamageWalls)) {
        return false;
    }
    struct Coord3d pos;
    if (!get_random_position_in_dungeon_for_creature(creatng->owner, CrWaS_WithinDungeon, creatng, &pos)) {
        return false;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureAttemptToDamageWalls)) {
        return false;
    }
    setup_person_move_to_coord(creatng, &pos, NavRtF_Default);
    creatng->continue_state = CrSt_CreatureAttemptToDamageWalls;
    return true;
}

TbBool attempt_anger_job_mad_psycho(struct Thing *creatng)
{
    TRACE_THING(creatng);
    if (!external_set_thing_state(creatng, CrSt_MadKillingPsycho))
    {
        return false;
    }
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    // Mad Psycho's anger job bypasses immunity.
    set_flag(cctrl->spell_flags, CSAfF_MadKilling);
    return true;
}

TbBool attempt_anger_job_persuade(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->exp_level <= 5) {
        return false;
    }
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreaturePersuade)) {
        return false;
    }
    struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
    int persuade_count = min(dungeon->num_active_creatrs - 1, 5);
    if (persuade_count <= 0) {
        return false;
    }
    persuade_count = THING_RANDOM(creatng, persuade_count) + 1;
    if (!external_set_thing_state(creatng, CrSt_CreaturePersuade)) {
        return false;
    }
    cctrl->persuade.persuade_count = persuade_count;
    return true;
}

TbBool attempt_anger_job_join_enemy(struct Thing *creatng)
{
    int n = THING_RANDOM(creatng, PLAYERS_COUNT);
    for (int i = 0; i < PLAYERS_COUNT; i++, n = (n + 1) % PLAYERS_COUNT)
    {
        if ((n == game.neutral_player_num) || (n == creatng->owner))
            continue;
        struct PlayerInfo* player = get_player(n);
        if (!player_exists(player) || (player->is_active != 1))
            continue;
        struct Thing* heartng = get_player_soul_container(n);
        if (thing_exists(heartng) && (heartng->active_state != 3))
        {
            TRACE_THING(heartng);
            if (creature_can_navigate_to(creatng, &heartng->mappos, NavRtF_Default)) {
                change_creature_owner(creatng, n);
                anger_set_creature_anger_all_types(creatng, 0);
            }
        }
    }
    return false;
}

long attempt_anger_job(struct Thing *creatng, long ajob_kind)
{
    switch (ajob_kind)
    {
    case 1:
        if (!attempt_anger_job_kill_creatures(creatng))
            break;
        return true;
    case 2:
        if (!attempt_anger_job_destroy_rooms(creatng))
            break;
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_CreatrDestroyRooms, MESSAGE_DURATION_CRTR_MOOD);
        return true;
    case 4:
        if (!attempt_anger_job_leave_dungeon(creatng))
            break;
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_CreatureLeaving, MESSAGE_DURATION_CRTR_MOOD);
        return true;
    case 8:
        if (!attempt_anger_job_steal_gold(creatng))
            break;
        return true;
    case 16:
        if (!attempt_anger_job_damage_walls(creatng))
            break;
        if (is_my_player_number(creatng->owner))
            output_message(SMsg_CreatrDestroyRooms, MESSAGE_DURATION_CRTR_MOOD);
        return true;
    case 32:
        if (!attempt_anger_job_mad_psycho(creatng))
            break;
        return true;
    case 64:
        if (!attempt_anger_job_persuade(creatng)) {
            // If can't init persuade, then leave alone
            if (!attempt_anger_job_leave_dungeon(creatng))
                break;
            if (is_my_player_number(creatng->owner))
                output_message(SMsg_CreatureLeaving, MESSAGE_DURATION_CRTR_MOOD);
        }
        return true;
    case 128:
        if (!attempt_anger_job_join_enemy(creatng))
            break;
        return true;
    case 256:
        if (!attempt_anger_job_kill_diggers(creatng))
            break;
        return true;
    default:
        break;
    }
    return false;
}

TbBool creature_find_and_perform_anger_job(struct Thing *creatng)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    // Count the amount of jobs set
    int i = 0;
    int k = crconf->jobs_anger;
    while (k != 0)
    {
        if ((k & 1) != 0)
            i++;
        k >>= 1;
    }
    if (i <= 0) {
        return false;
    }
    // Select a random job as a starting point
    int n = THING_RANDOM(creatng, i) + 1;
    i = 0;
    for (k = 0; k < game.conf.crtr_conf.angerjobs_count; k++)
    {
        if ((crconf->jobs_anger & (1 << k)) != 0) {
            n--;
        }
        if (n <= 0) {
            i = k;
            break;
        }
    }
    // Go through all jobs, starting at randomly selected one, attempting to start each one
    for (k = 0; k < game.conf.crtr_conf.angerjobs_count; k++)
    {
        if ((crconf->jobs_anger & (1 << i)) != 0)
        {
          if (attempt_anger_job(creatng, 1 << i))
              return 1;
        }
        i = (i+1) % game.conf.crtr_conf.angerjobs_count;
    }
    return 0;
}

/** Returns if a creature will refuse to do specific job.
 *
 * @param creatng The creature which is planned for the job.
 * @param jobpref The job to be checked.
 * @return
 */
TbBool creature_will_reject_job(const struct Thing *creatng, CreatureJob jobpref)
{
    if (player_uses_power_obey(creatng->owner) && ((game.conf.rules[creatng->owner].game.classic_bugs_flags & ClscBug_MustObeyKeepsNotDoJobs) == 0)) {
        return false;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    return (jobpref & crconf->jobs_not_do) != 0;
}

TbBool creature_dislikes_job(const struct Thing *creatng, CreatureJob jobpref)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    return (jobpref & crconf->jobs_not_do) != 0;
}

TbBool is_correct_owner_to_perform_job(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    // Note that room required for job is not checked here on purpose.
    // We need to check for it later in upper function, because lack of related room may generate message for the player
    if (creatng->owner == plyr_idx)
    {
        if (creature_is_for_dungeon_diggers_list(creatng)) {
            if ((get_flags_for_job(new_job) & JoKF_OwnedDiggers) == 0)
                return false;
        } else {
            if ((get_flags_for_job(new_job) & JoKF_OwnedCreatures) == 0)
                return false;
        }
    } else
    {
        if (creature_is_for_dungeon_diggers_list(creatng)) {
            if ((get_flags_for_job(new_job) & JoKF_EnemyDiggers) == 0)
                return false;
        } else {
            if ((get_flags_for_job(new_job) & JoKF_EnemyCreatures) == 0)
                return false;
        }
    }
    return true;
}

TbBool is_correct_position_to_perform_job(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    const struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    const struct Room* room = subtile_room_get(stl_x, stl_y);
    RoomRole job_rrole = get_room_role_for_job(new_job);
    if (job_rrole != RoRoF_None)
    {
        if (room_is_invalid(room)) {
            return false;
        }
        if (!room_role_matches(room->kind,job_rrole)) {
            return false;
        }
    }
    if (!is_correct_owner_to_perform_job(creatng, slabmap_owner(slb), new_job)) {
        return false;
    }
    return true;
}

TbBool creature_can_do_job_always_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return true;
}

TbBool creature_can_do_training_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_be_trained(creatng) && player_can_afford_to_train_creature(creatng);
}

TbBool creature_can_do_research_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_do_research(creatng);
}

TbBool creature_can_do_manufacturing_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_do_manufacturing(creatng);
}

TbBool creature_can_do_scavenging_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    return creature_can_do_scavenging(creatng) && player_can_afford_to_scavenge_creature(creatng);
}

TbBool creature_can_freeze_prisoners_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    // To freeze prisoners, our prison can't be empty
    struct Room* room = find_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, get_room_role_for_job(Job_FREEZE_PRISONERS), NavRtF_Default, 1);
    return creature_instance_is_available(creatng, CrInst_FREEZE) && !room_is_invalid(room);
}

TbBool creature_can_join_fight_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    struct Event* event = get_event_of_type_for_player(EvKind_EnemyFight, creatng->owner);
    if (!event_exists(event)) {
        event = get_event_of_type_for_player(EvKind_HeartAttacked, creatng->owner);
    }
    return event_exists(event);
}

TbBool creature_can_do_barracking_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    // Grouping or barracking only makes sense if we have more than one creature
    const struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->num_active_creatrs > 1);
}

/**
 * Returns a drop position within room for a creature to do given job.
 * @param pos
 * @param room
 * @param jobpref
 * @return
 * @see creature_move_to_place_in_room()
 */
TbBool get_drop_position_for_creature_job_in_room(struct Coord3d *pos, const struct Room *room, CreatureJob jobpref,
        struct Thing *thing)
{
    if (!room_exists(room)) {
        return false;
    }
    TbBool result;
    unsigned long room_area = get_flags_for_job(jobpref) & (JoKF_AssignOnAreaBorder|JoKF_AssignOnAreaCenter);
    switch (room_area)
    {
    case JoKF_AssignOnAreaBorder:
        SYNCDBG(9,"Job %s requires dropping at %s border",creature_job_code_name(jobpref),room_code_name(room->kind));
        result = find_random_position_at_area_of_room(pos, room, RoArC_BORDER, thing);
        break;
    case JoKF_AssignOnAreaCenter:
        SYNCDBG(9,"Job %s requires dropping at %s center",creature_job_code_name(jobpref),room_code_name(room->kind));
        result = find_random_position_at_area_of_room(pos, room, RoArC_CENTER, thing);
        result = true;
        break;
    case (JoKF_AssignOnAreaBorder|JoKF_AssignOnAreaCenter):
        SYNCDBG(9,"Job %s has no %s area preference",creature_job_code_name(jobpref),room_code_name(room->kind));
        result = find_random_position_at_area_of_room(pos, room, RoArC_ANY, thing);
        break;
    default:
        WARNLOG("Invalid drop area flags 0x%04x for job %s",(int)room_area,creature_job_code_name(jobpref));
        result = find_random_position_at_area_of_room(pos, room, RoArC_ANY, thing);
        break;
    }
    return result;
}

/**
 * Returns a position on which creature could be set to init or dropped to start doing specific job.
 * @param creatng
 * @param new_job
 * @param drop_kind_flags Flags to select whether we want to set to init or drop the creature.
 * @return
 */
TbBool get_drop_position_for_creature_job_in_dungeon(struct Coord3d *pos, const struct Dungeon *dungeon, struct Thing *creatng, CreatureJob new_job, unsigned long drop_kind_flags)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(new_job);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    SYNCDBG(16,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    if (((jobcfg->job_flags & JoKF_NeedsHaveJob) != 0) && (((crconf->job_primary|crconf->job_secondary) & new_job) == 0))
    {
        SYNCDBG(3,"Cannot assign %s for %s index %d owner %d; NEEDS_HAVE_JOB",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    if (((jobcfg->job_flags & JoKF_AssignAreaWithinRoom) != 0) && player_has_room_of_role(dungeon->owner, jobcfg->room_role))
    {
        int needed_capacity;
        if ((jobcfg->job_flags & JoKF_NeedsCapacity) != 0) {
            needed_capacity = 1;
        } else {
            needed_capacity = 0;
        }
        struct Room* room = get_room_of_given_role_for_thing(creatng, dungeon, jobcfg->room_role, needed_capacity);
        // Returns position, either on border on within room center
        if (get_drop_position_for_creature_job_in_room(pos, room, new_job, creatng)) {
            return true;
        }
        SYNCDBG(3,"No place to assign %s for %s index %d owner %d within room",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    }
    if ((jobcfg->job_flags & JoKF_AssignAreaOutsideRoom) != 0)
    {
        WARNDBG(3,"No place to assign %s for %s index %d owner %d; not implemented",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    }
    return false;
}

/** Returns if a creature can do specific job for the player.
 *
 * @param creatng The creature which is planned for the job.
 * @param plyr_idx Player for whom the job is to be done.
 * @param new_job Job selection with single job flag set.
 * @param flags Function behavior adjustment flags.
 * @return True if the creature can do the job specified, false otherwise.
 * @note this should be used instead of person_will_do_job_for_room()
 * @note this function will never change state of the input thing, even if appropriate flags are set
 * @see creature_can_do_job_near_position() similar function for use when target position is known
 */
TbBool creature_can_do_job_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job, unsigned long flags)
{
    SYNCDBG(16,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    if (creature_will_reject_job(creatng, new_job))
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; in not do jobs list",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    if (!is_correct_owner_to_perform_job(creatng, plyr_idx, new_job))
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; not correct owner for job",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Don't allow creatures changed to chickens to have any job assigned, besides those specifically marked
    if (creature_under_spell_effect(creatng, CSAfF_Chicken)
    && !flag_is_set(get_flags_for_job(new_job), JoKF_AllowChickenized))
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; under chicken spell",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Check if the job is related to correct player
    struct CreatureJobConfig* jobcfg = get_config_for_job(new_job);
    if (creature_job_player_check_func_list[jobcfg->func_plyr_check_idx] == NULL)
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; no check callback",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    if (!creature_job_player_check_func_list[jobcfg->func_plyr_check_idx](creatng, plyr_idx, new_job))
    {
        SYNCDBG(13,"Cannot assign %s for %s index %d owner %d; check callback failed",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    RoomRole job_rrole = get_room_role_for_job(new_job);
    if (job_rrole != RoRoF_None)
    {
        if (!player_has_room_of_role(plyr_idx, job_rrole))
        {
            SYNCDBG(3,"Cannot assign %s in player %d room for %s index %d owner %d; no required room built",creature_job_code_name(new_job),(int)plyr_idx,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
            if ((flags & JobChk_PlayMsgOnFail) != 0) {
                output_room_message(plyr_idx, get_first_room_kind_for_job(new_job), OMsg_RoomNeeded);
            }
            return false;
        }
        if ((get_flags_for_job(new_job) & JoKF_NeedsCapacity) != 0)
        {
            struct Room* room = find_room_of_role_with_spare_capacity(plyr_idx, job_rrole, 1);
            if (room_is_invalid(room))
            {
                SYNCDBG(3,"Cannot assign %s in player %d room for %s index %d owner %d; not enough room capacity",creature_job_code_name(new_job),(int)plyr_idx,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
                if ((flags & JobChk_PlayMsgOnFail) != 0) {
                    output_room_message(plyr_idx, get_first_room_kind_for_job(new_job), OMsg_RoomTooSmall);
                }
                return false;
            }
        }
    }
    return true;
}

TbBool send_creature_to_job_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    SYNCDBG(6,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    struct CreatureJobConfig* jobcfg = get_config_for_job(new_job);
    if (creature_job_player_assign_func_list[jobcfg->func_plyr_assign_idx] != NULL)
    {
        if (creature_job_player_assign_func_list[jobcfg->func_plyr_assign_idx](creatng, plyr_idx, new_job))
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
            // Set computer control accordingly to job flags
            if ((get_flags_for_job(new_job) & JoKF_NoSelfControl) != 0) {
                cctrl->creature_control_flags |= CCFlg_NoCompControl;
            } else {
                cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
            }
            // If a new task isn't a work-in-group thing, remove the creature from group
            if ((get_flags_for_job(new_job) & JoKF_NoGroups) != 0)
            {
                if (creature_is_group_member(creatng)) {
                    SYNCDBG(3,"Removing %s index %d owned by player %d from group",
                        thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
                    remove_creature_from_group(creatng);
                }
            }
            return true;
        }
    } else
    {
        ERRORLOG("Cannot start %s for %s index %d owner %d; job has no player-based assign",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    }
    return false;
}

TbBool creature_can_do_job_always_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    return true;
}

TbBool creature_can_do_research_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_do_research(creatng))
    {
        struct Room* room = subtile_room_get(stl_x, stl_y);
        struct Dungeon* dungeon = get_dungeon(room->owner);
        if (!is_neutral_thing(creatng) && (dungeon->current_research_idx < 0))
        {
            if (is_my_player_number(dungeon->owner) && ((flags & JobChk_PlayMsgOnFail) != 0)) {
                output_message(SMsg_NoMoreReseach, MESSAGE_DURATION_KEEPR_TAUNT);
            }
        }
        return false;
    }
    return true;
}

TbBool creature_can_do_training_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_be_trained(creatng)) {
        return false;
    }
    return true;
}

TbBool creature_can_do_manufacturing_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_do_manufacturing(creatng)) {
        return false;
    }
    return true;
}

TbBool creature_can_do_scavenging_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_can_do_scavenging(creatng)) {
        return false;
    }
    return true;
}

TbBool creature_can_place_in_vault_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (creatng->creature.gold_carried < 1) {
        return false;
    }
    return true;
}

TbBool creature_can_take_salary_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    struct Room* room = subtile_room_get(stl_x, stl_y);
    // If there is any gold in the room - give it a shot
    // We're checking if the amount of gold is enough for the creature pay only if room seem empty
    if (room->used_capacity < 1) {
        struct Dungeon* dungeon = get_dungeon(creatng->owner);
        GoldAmount pay = calculate_correct_creature_pay(creatng);
        if (room->capacity_used_for_storage + dungeon->offmap_money_owned < pay) {
            return false;
        }
    }
    return true;
}

TbBool creature_can_take_sleep_near_pos(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingHomeToSleep)) {
        return false;
    }
    return true;
}

 /** Returns if a creature can do specific job at given map position.
 *
 * @param creatng The creature which is planned for the job.
 * @param stl_x Target map position, x coord.
 * @param stl_y Target map position, y coord.
 * @param new_job Job selection with single job flag set.
 * @return True if the creature can do the job specified, false otherwise.
 * @see creature_can_do_job_for_player() similar function for use when only target player is known
 */
TbBool creature_can_do_job_near_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(6,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if (creature_will_reject_job(creatng, new_job))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; in not-do-jobs list",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        if ((flags & JobChk_SetStateOnFail) != 0) {
            anger_apply_anger_to_creature(creatng, crconf->annoy_will_not_do_job, AngR_Other, 1);
            external_set_thing_state(creatng, CrSt_CreatureMoan);
            cctrl->countdown = 50;
        }
        return false;
    }
    // Don't allow creatures changed to chickens to have any job assigned, besides those specifically marked
    if (creature_under_spell_effect(creatng, CSAfF_Chicken)
    && !flag_is_set(get_flags_for_job(new_job), JoKF_AllowChickenized))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; under chicken spell",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Check if the job is related to correct map place (room,slab)
    if (!is_correct_position_to_perform_job(creatng, stl_x, stl_y, new_job))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; not correct place for job",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    struct CreatureJobConfig* jobcfg = get_config_for_job(new_job);
    if (creature_job_coords_check_func_list[jobcfg->func_cord_check_idx] == NULL)
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; job has no coord check function",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    if (!creature_job_coords_check_func_list[jobcfg->func_cord_check_idx](creatng, stl_x, stl_y, new_job, flags))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; coord check not passed",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // If other tests pass, check if related room (if is needed) has capacity to be used for that job
    if ((get_flags_for_job(new_job) & JoKF_NeedsCapacity) != 0)
    {
        struct Room* room = subtile_room_get(stl_x, stl_y);
        if (!room_has_enough_free_capacity_for_creature_job(room, creatng, new_job))
        {
            SYNCDBG(3,"Cannot assign %s at (%d,%d) for %s index %d owner %d; not enough room capacity",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
            if ((flags & JobChk_PlayMsgOnFail) != 0) {
                output_room_message(room->owner, room->kind, OMsg_RoomTooSmall);
            }
            return false;
        }
    }
    return true;
}

TbBool send_creature_to_job_near_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    SYNCDBG(6,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    struct CreatureJobConfig* jobcfg = get_config_for_job(new_job);
    if (creature_job_coords_assign_func_list[jobcfg->func_cord_assign_idx] != NULL)
    {
        if (creature_job_coords_assign_func_list[jobcfg->func_cord_assign_idx](creatng, stl_x, stl_y, new_job))
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
            // Set computer control accordingly to job flags
            if ((get_flags_for_job(new_job) & JoKF_NoSelfControl) != 0) {
                cctrl->creature_control_flags |= CCFlg_NoCompControl;
            } else {
                cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
            }
            // If a new task isn't a work-in-group thing, remove the creature from group
            if ((get_flags_for_job(new_job) & JoKF_NoGroups) != 0)
            {
                if (creature_is_group_member(creatng)) {
                    SYNCDBG(3,"Removing %s index %d owned by player %d from group",
                        thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
                    remove_creature_from_group(creatng);
                }
            }
            return true;
        }
    } else
    {
        ERRORLOG("Cannot start %s for %s index %d owner %d; job has no coord-based assign",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    }
    return false;
}

/**
 * Retirns whether given creature can do job in a room having given role.
 *
 * @param creatng
 * @param plyr_idx
 * @param rrole
 * @return
 */
TbBool creature_can_do_job_for_computer_player_in_room_role(const struct Thing *creatng, PlayerNumber plyr_idx, RoomRole rrole)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    CreatureJob jobpref = get_job_for_room_role(rrole, JoKF_AssignComputerDrop | JoKF_AssignAreaWithinRoom, crconf->job_primary | crconf->job_secondary);
    return creature_can_do_job_for_player(creatng, plyr_idx, jobpref, JobChk_None);
}

TbBool attempt_job_work_in_room_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    struct Room *room;
    RoomRole rrole = get_room_role_for_job(new_job);
    SYNCDBG(6,"Starting for %s index %d owner %d and job %s in %s room",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job),room_role_code_name(rrole));
    if ((get_flags_for_job(new_job) & JoKF_NeedsCapacity) != 0) {
        room = find_nearest_room_of_role_for_thing_with_spare_capacity(creatng, creatng->owner, rrole, NavRtF_Default, 1);
    } else {
        room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, rrole, NavRtF_Default);
    }
    if (room_is_invalid(room)) {
        return false;
    }
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, new_job, NavRtF_Default)) {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = get_arrive_at_state_for_job(new_job);
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_job_work_in_room_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(16,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    struct Room* room = subtile_room_get(stl_x, stl_y);
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, new_job, NavRtF_Default)) {
        WARNLOG("Could not move in room %s to perform job %s by %s index %d owner %d",room_code_name(room->kind),creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    creatng->continue_state = get_arrive_at_state_for_job(new_job);
    cctrl->target_room_id = room->index;
    if (thing_is_creature_digger(creatng))
    {
        cctrl->digger.task_repeats = 0;
        cctrl->job_assigned = new_job;
        cctrl->digger.last_did_job = SDLstJob_NonDiggerTask;
    }
    return true;
}

TbBool attempt_job_work_in_room_and_cure_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    SYNCDBG(16,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    struct Room* room = subtile_room_get(stl_x, stl_y);
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, new_job, NavRtF_Default)) {
        WARNLOG("Could not move in room %s to perform job %s by %s index %d owner %d",room_code_name(room->kind),creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = get_arrive_at_state_for_job(new_job);
    cctrl->target_room_id = room->index;
    process_temple_cure(creatng);
    if (thing_is_creature_digger(creatng))
    {
        cctrl->digger.task_repeats = 0;
        cctrl->job_assigned = new_job;
        cctrl->digger.last_did_job = SDLstJob_NonDiggerTask;
    }
    return true;
}

TbBool attempt_job_sleep_in_lair_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(16,"Starting for %s index %d owner %d and job %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,creature_job_code_name(new_job));
    struct Room* room = subtile_room_get(stl_x, stl_y);
    if (get_arrive_at_state_for_job(new_job) == CrSt_Unused) {
        ERRORLOG("No arrive at state for job %s in %s room",creature_job_code_name(new_job),room_code_name(room->kind));
        return false;
    }
    if(!creature_can_do_healing_sleep(creatng)){
        return false;
    }
    cctrl->slap_turns = 0;
    cctrl->max_speed = calculate_correct_creature_maxspeed(creatng);
    if (creature_has_lair_room(creatng) && (room->index == cctrl->lair_room_id))
    {
        if (creature_move_to_home_lair(creatng))
        {
            creatng->continue_state = CrSt_CreatureGoingHomeToSleep;
            return true;
        }
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, new_job, NavRtF_Default)) {
        WARNLOG("Could not move in room %s to perform job %s by %s index %d owner %d",room_code_name(room->kind),creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    creatng->continue_state = CrSt_CreatureChangeLair;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool attempt_job_in_state_on_room_content_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    RoomRole rrole = get_room_role_for_job(new_job);
    struct Room* room = find_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, rrole, NavRtF_Default, 1);
    if (room_is_invalid(room)) {
        WARNLOG("Could not find room %s to perform job %s by %s index %d owner %d",room_role_code_name(rrole),creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    internal_set_thing_state(creatng, get_initial_state_for_job(new_job));
    return true;
}

TbBool attempt_job_move_to_event_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    EventKind evkind = get_event_for_job(new_job);
    struct Event* event = get_event_of_type_for_player(evkind, creatng->owner);
    // Treat heart attack as enemy fight, too
    if (event_is_invalid(event) && (evkind == EvKind_EnemyFight)) {
        event = get_event_of_type_for_player(EvKind_HeartAttacked, creatng->owner);
    }
    if (event_is_invalid(event)) {
        WARNLOG("Could not find event to perform job %s by %s index %d owner %d",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    if (!setup_person_move_to_position(creatng, coord_subtile(event->mappos_x), coord_subtile(event->mappos_y), NavRtF_Default)) {
        WARNLOG("Could not reach event to perform job %s by %s index %d owner %d",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    creatng->continue_state = get_initial_state_for_job(new_job);
    return true;
}

TbBool attempt_job_in_state_internal_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job)
{
    CrtrStateId crstate = get_initial_state_for_job(new_job);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    internal_set_thing_state(creatng, crstate);
    // Some states need additional initialization
    if (crstate == CrSt_SeekTheEnemy) {
        cctrl->seek_enemy.enemy_idx = 0;
    }
    return true;
}

TbBool attempt_job_in_state_internal_near_pos(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job)
{
    return attempt_job_in_state_internal_for_player(creatng, creatng->owner, new_job);
}

/**
 * Tries to assign one of creature's preferred jobs to it.
 * Starts at random job, to make sure all the jobs have equal chance of being selected.
 * @param creatng The creature to assign a job to.
 * @param jobpref Job preference flags.
 */
TbBool attempt_job_preference(struct Thing *creatng, long jobpref)
{
    // Start checking at random job
    if (game.conf.crtr_conf.jobs_count < 1) {
        return false;
    }
    long n = THING_RANDOM(creatng, game.conf.crtr_conf.jobs_count);
    for (long i = 0; i < game.conf.crtr_conf.jobs_count; i++, n = (n + 1) % game.conf.crtr_conf.jobs_count)
    {
        if (n == 0)
            continue;
        CreatureJob new_job = 1ULL << (n - 1);
        if ((jobpref & new_job) != 0)
        {
            SYNCDBG(19,"Check job %s",creature_job_code_name(new_job));
            if (creature_can_do_job_for_player(creatng, creatng->owner, new_job, JobChk_None))
            {
                if (send_creature_to_job_for_player(creatng, creatng->owner, new_job)) {
                    return true;
                }
            }
        }
    }
    return false;
}

TbBool attempt_job_secondary_preference(struct Thing *creatng, long jobpref)
{
    // Count the amount of jobs set
    long i = 0;
    unsigned long k = jobpref;
    while (k)
    {
        k >>= 1;
        i++;
    }
    if (i <= 0) {
        return false;
    }
    unsigned long select_val = THING_RANDOM(creatng, 512);
    unsigned long select_delta = 512 / i;
    unsigned long select_curr = select_delta;
    // For some reason, this is a bit different than attempt_job_preference().
    // Probably needs unification
    for (i=1; i < game.conf.crtr_conf.jobs_count; i++)
    {
        CreatureJob new_job = 1ULL << (i-1);
        if ((jobpref & new_job) == 0) {
            continue;
        }
        SYNCDBG(19,"Check job %s",creature_job_code_name(new_job));
        if (select_val <= select_curr)
        {
            select_curr += select_delta;
        } else
        if (creature_can_do_job_for_player(creatng, creatng->owner, new_job, JobChk_None))
        {
            if (send_creature_to_job_for_player(creatng, creatng->owner, new_job)) {
                return true;
            }
        }
    }
    // If no job, give 1% chance of going to temple
    if (THING_RANDOM(creatng, 100) == 0)
    {
        CreatureJob new_job = Job_TEMPLE_PRAY;
        if (creature_can_do_job_for_player(creatng, creatng->owner, new_job, JobChk_None))
        {
            if (send_creature_to_job_for_player(creatng, creatng->owner, new_job))
            {
                if (!creature_dislikes_job(creatng, new_job))
                {
                    return true;
                }
            }
        }
    }
    return 0;
}

TbBool creature_try_doing_secondary_job(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (game.play_gameturn - cctrl->job_secondary_check_turn <= 128) {
        return false;
    }
    cctrl->job_secondary_check_turn = game.play_gameturn;
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    return attempt_job_secondary_preference(creatng, crconf->job_secondary);
}
/******************************************************************************/
