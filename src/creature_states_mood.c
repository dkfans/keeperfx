/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_mood.c
 *     Creature state machine functions related to their mood.
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
#include "creature_states_mood.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_sound.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "creature_jobs.h"
#include "creature_states_lair.h"
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
#include "player_utils.h"
#include "game_legacy.h"

/******************************************************************************/
TbBool creature_can_get_angry(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    return (crstat->annoy_level > 0);
}

short creature_moan(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    long i = cctrl->countdown_282;
    if (i > 0) i--;
    cctrl->countdown_282 = i;
    if (i <= 0)
    {
        if (cctrl->instance_id == CrInst_NULL) {
            set_start_state(thing);
        }
        return 0;
    }
    if (game.play_gameturn - cctrl->last_mood_sound_turn > 32)
    {
        play_creature_sound(thing, CrSnd_Sad, 2, 0);
        cctrl->last_mood_sound_turn = game.play_gameturn;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(thing, CrInst_MOAN, 1, 0, 0);
    }
    return 1;
}

short creature_roar(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);

    if (cctrl->countdown_282 > 0) {
        cctrl->countdown_282--;
    }
    if (cctrl->countdown_282 <= 0)
    {
        cctrl->last_roar_turn = game.play_gameturn;
        set_start_state(thing);
        return 0;
    }
    if (game.play_gameturn - cctrl->long_9A > 32)
    {
        play_creature_sound(thing, 4, 2, 0);
        cctrl->long_9A = game.play_gameturn;
    }
    return 1;
}

short creature_be_happy(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    long i = cctrl->countdown_282;
    if (i > 0) i--;
    cctrl->countdown_282 = i;
    if (i <= 0)
    {
      if (cctrl->instance_id == CrInst_NULL) {
          set_start_state(thing);
      }
      return 0;
    }
    if (game.play_gameturn - cctrl->last_mood_sound_turn > 32)
    {
        play_creature_sound(thing, CrSnd_Happy, 2, 0);
        cctrl->last_mood_sound_turn = game.play_gameturn;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(thing, CrInst_CELEBRATE_SHORT, 1, 0, 0);
    }
    return 1;
}

short creature_piss(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ( !S3DEmitterIsPlayingSample(thing->snd_emitter_id, 171, 0) ) {
        thing_play_sample(thing, 171, NORMAL_PITCH, 0, 3, 1, 6, FULL_LOUDNESS);
    }
    long i = cctrl->countdown_282;
    if (i > 0) i--;
    cctrl->countdown_282 = i;
    if (i > 0) {
        return 1;
    }
    cctrl->field_B2 = game.play_gameturn;
    set_start_state(thing);
    return 0;
}

short mad_killing_psycho(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // Find a position for killing - use random dungeon
    struct Coord3d pos;
    int i;
    int n = CREATURE_RANDOM(creatng, PLAYERS_COUNT);
    for (i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(n);
        if (player_exists(player)) {
            if (get_random_position_in_dungeon_for_creature(n, CrWaS_WithinDungeon, creatng, &pos)) {
                if (creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_Default)) {
                    break;
                }
            }
        }
        n = (n+1) % PLAYERS_COUNT;
    }
    if (i >= PLAYERS_COUNT)
      return 1;
    if (setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
    {
        if (game.play_gameturn - cctrl->last_roar_turn <= 200)
        {
            creatng->continue_state = CrSt_MadKillingPsycho;
        } else
        {
            cctrl->countdown_282 = 50;
            creatng->continue_state = CrSt_CreatureRoar;
        }
    } else
    {
        if (game.play_gameturn - cctrl->last_roar_turn > 200)
        {
            cctrl->countdown_282 = 50;
            internal_set_thing_state(creatng, CrSt_CreatureRoar);
        }
    }
    return 1;
}

void anger_calculate_creature_is_angry(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    cctrl->mood_flags &= ~CCMoo_Angry;
    cctrl->mood_flags &= ~CCMoo_Livid;
    for (int i = 1; i < 5; i++)
    {
        if (crstat->annoy_level <= cctrl->annoyance_level[i])
        {
            cctrl->mood_flags |= CCMoo_Angry;
            if (2*crstat->annoy_level <= cctrl->annoyance_level[i])
            {
                cctrl->mood_flags |= CCMoo_Livid;
                break;
            }
        }
    }
}

TbBool anger_free_for_anger_increase(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->combat_flags != 0) {
        return false;
    }
    return !creature_affected_by_call_to_arms(creatng);
}

TbBool anger_free_for_anger_decrease(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // If the creature is mad killing, don't allow it not to be angry
    if ((cctrl->spell_flags & CSAfF_MadKilling) != 0) {
        return false;
    }
    return true;
}

void anger_increase_creature_anger_f(struct Thing *creatng, long anger, AnnoyMotive reason, const char *func_name)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (anger_free_for_anger_increase(creatng))
    {
        struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->lvstats.lies_told++;
        }
        anger_set_creature_anger_f(creatng, anger + cctrl->annoyance_level[reason], reason, func_name);
    }
}

void anger_reduce_creature_anger_f(struct Thing *creatng, long anger, AnnoyMotive reason, const char *func_name)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (anger_free_for_anger_decrease(creatng))
    {
        anger_set_creature_anger_f(creatng, anger + cctrl->annoyance_level[reason], reason, func_name);
    }
}

void anger_set_creature_anger_f(struct Thing *creatng, long annoy_lv, AnnoyMotive reason, const char *func_name)
{
    SYNCDBG(18,"%s: Setting reason %d to %d for %s index %d",func_name,(int)reason,(int)annoy_lv,thing_model_name(creatng),(int)creatng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if ((game.numfield_14 != 0) || !creature_can_get_angry(creatng)) {
        return;
    }
    if (annoy_lv < 0)
    {
        annoy_lv = 0;
    } else
    if (annoy_lv > (3 * crstat->annoy_level)) 
    {
        annoy_lv = (3 * crstat->annoy_level);
    } else
    if (annoy_lv > 65534) 
    {
        annoy_lv = 65534;
    }
    TbBool was_angry = ((cctrl->mood_flags & CCMoo_Angry) != 0);
    cctrl->annoyance_level[reason] = annoy_lv;
    anger_calculate_creature_is_angry(creatng);
    struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
    if (dungeon_invalid(dungeon)) {
        return;
    }
    if ((cctrl->mood_flags & CCMoo_Angry) != 0)
    {
        if (!was_angry) {
            dungeon->creatures_annoyed++;
            event_create_event_or_update_nearby_existing_event(
              creatng->mappos.x.val, creatng->mappos.y.val,
              EvKind_CreatrIsAnnoyed, creatng->owner, creatng->index);
        }
    } else
    {
        if (was_angry) {
          if (dungeon->creatures_annoyed > 0) {
              dungeon->creatures_annoyed--;
          } else {
              ERRORLOG("%s: Removing annoyed creature - none to Remove",func_name);
          }
        }
    }
}

TbBool anger_is_creature_livid(const struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl))
        return false;
    return ((cctrl->mood_flags & CCMoo_Livid) != 0);
}

TbBool anger_is_creature_angry(const struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl))
        return false;
    return ((cctrl->mood_flags & CCMoo_Angry) != 0);
}

AnnoyMotive anger_get_creature_anger_type(const struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if (crstat->annoy_level == 0)
        return AngR_None;
    if ((cctrl->mood_flags & CCMoo_Angry) == 0)
        return AngR_None;
    AnnoyMotive anger_type = AngR_None;
    long anger_level = 0;
    for (long i = 1; i < 5; i++)
    {
        if (anger_level < cctrl->annoyance_level[i])
        {
            anger_level = cctrl->annoyance_level[i];
            anger_type = i;
        }
    }
    if (anger_level < (long)crstat->annoy_level)
        return AngR_None;
    return anger_type;
}

void anger_apply_anger_to_creature_all_types_f(struct Thing *thing, long anger, const char *func_name)
{
    if (!creature_can_get_angry(thing) || anger == 0) {
        return;
    }
    if (anger > 0)
    {
        anger_increase_creature_anger_f(thing, anger, AngR_Other, func_name);
        anger_increase_creature_anger_f(thing, anger, AngR_NotPaid, func_name);
        anger_increase_creature_anger_f(thing, anger, AngR_NoLair, func_name);
        anger_increase_creature_anger_f(thing, anger, AngR_Hungry, func_name);
    } else
    {
        anger_reduce_creature_anger_f(thing, anger, AngR_Other, func_name);
        anger_reduce_creature_anger_f(thing, anger, AngR_NotPaid, func_name);
        anger_reduce_creature_anger_f(thing, anger, AngR_NoLair, func_name);
        anger_reduce_creature_anger_f(thing, anger, AngR_Hungry, func_name);
    }
}

TbBool anger_make_creature_angry(struct Thing *creatng, AnnoyMotive reason)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if ((crstat->annoy_level <= 0) || ((cctrl->mood_flags & CCMoo_Angry) != 0))
        return false;
    anger_set_creature_anger(creatng, crstat->annoy_level, reason);
    return true;
}

TbBool creature_mark_if_woken_up(struct Thing *creatng)
{
    if (creature_is_sleeping(creatng))
    {
        struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
        anger_apply_anger_to_creature(creatng, crstat->annoy_woken_up, AngR_Other, 1);
        return true;
    }
    return false;
}

TbBool creature_will_go_postal_on_victim_during_job(const struct Thing *creatng, const struct Thing *victng, CreatureJob job_kind)
{
    if (thing_is_creature(victng) && (victng->index != creatng->index) && !creature_has_job(victng, job_kind)
        && !creature_is_kept_in_custody(victng) && !creature_is_being_unconscious(victng)
        && !creature_is_dying(victng) && !creature_is_doing_anger_job(victng))
    {
        if (!creature_is_invisible(victng) || creature_can_see_invisible(creatng)) {
            return true;
        }
    }
    return false;
}

TbBool find_combat_target_passing_by_subtile_but_having_unrelated_job(const struct Thing *creatng, CreatureJob job_kind, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long *found_dist, struct Thing **found_thing)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (creature_will_go_postal_on_victim_during_job(creatng, thing, job_kind))
        {
            long dist = get_combat_distance(creatng, thing);
            // If we have combat sight - we want that target, don't search anymore
            if (creature_can_see_combat_path(creatng, thing, dist))
            {
                *found_dist = dist;
                *found_thing = thing;
                return true;
            }
            // No combat sight - but maybe it's at least closer than previous one
            if ( *found_dist > dist )
            {
                *found_dist = dist;
                *found_thing = thing;
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return false;
}

/**
 * Finds a creature passing by a subtile and having job different than given one, which has direct combat sight for given creature.
 * If such creature is not found, the function returns false and gives closest creature passing by a subtile and having job different than given one.
 * @param creatng The creature which should have combat sight of the target.
 * @param job_kind The job target should'n be performing.
 * @param slb_x Subtile on which target creature is searched, X coord.
 * @param slb_y Subtile on which target creature is searched, Y coord.
 * @param found_dist Distance to the target creature found.
 * @param found_thing The target creature found, either closest one or one with combat sight.
 * @return True if a target with combat sight was found. False if closest creature was found, or no creature met the conditions.
 * @note If no creature met the conditions, output variables are not initialized. Therefore, they should be initialized before calling this function.
 */
TbBool find_combat_target_passing_by_slab_but_having_unrelated_job(const struct Thing *creatng, CreatureJob job_kind, MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned long *found_dist, struct Thing **found_thing)
{
    MapSubtlCoord endstl_x = 3 * slb_x + 3;
    MapSubtlCoord endstl_y = 3 * slb_y + 3;
    for (MapSubtlCoord stl_y = 3 * slb_y; stl_y < endstl_y; stl_y++)
    {
        for (MapSubtlCoord stl_x = 3 * slb_x; stl_x < endstl_x; stl_x++)
        {
            if (find_combat_target_passing_by_subtile_but_having_unrelated_job(creatng, job_kind, stl_x, stl_y, found_dist, found_thing))
                return true;
        }
    }
    // If found a creature, but it's not on sight
    return false;
}

/**
 * Finds a creature passing by a room and having job different than given one, which has direct combat sight for given creature.
 * If such creature is not found, the function returns false and gives closest creature passing by a room and having job different than given one.
 * @param creatng The creature which should have combat sight of the target.
 * @param job_kind The job target should'n be performing.
 * @param room The room on which target creature is searched.
 * @param found_dist Distance to the target creature found.
 * @param found_thing The target creature found, either closest one or one with combat sight.
 * @return True if a target with combat sight was found. False if closest creature was found, or no creature met the conditions.
 * @note If no creature met the conditions, output variables are not initialized. Therefore, they should be initialized before calling this function.
 */
TbBool find_combat_target_passing_by_room_but_having_unrelated_job(const struct Thing *creatng, CreatureJob job_kind, const struct Room *room, unsigned long *found_dist, struct Thing **found_thing)
{
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSubtlCoord slb_x = slb_num_decode_x(i);
        MapSubtlCoord slb_y = slb_num_decode_y(i);
        // Per-slab code
        if (find_combat_target_passing_by_slab_but_having_unrelated_job(creatng, job_kind, slb_x, slb_y, found_dist, found_thing)) {
            return true;
        }
        // Per-slab code ends
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    // If found a creature, but it's not on sight
    return false;
}

TbBool process_job_causes_going_postal(struct Thing *creatng, struct Room *room, CreatureJob going_postal_job)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    CrInstance inst_use = get_best_quick_range_instance_to_use(creatng);
    if (inst_use <= 0) {
        SYNCDBG(8,"The %s index %d cannot go postal during %s; no ranged instance",thing_model_name(creatng),(int)creatng->index,creature_job_code_name(going_postal_job));
        return false;
    }
    // Find a target
    unsigned long combt_dist = LONG_MAX;
    struct Thing* combt_thing = INVALID_THING;
    if (find_combat_target_passing_by_room_but_having_unrelated_job(creatng, going_postal_job, room, &combt_dist, &combt_thing))
    {
        SYNCDBG(8,"The %s index %d goes postal on %s index %d during %s",thing_model_name(creatng),(int)creatng->index,thing_model_name(combt_thing),(int)combt_thing->index,creature_job_code_name(going_postal_job));
        EVM_CREATURE_EVENT_WITH_TARGET("postal", creatng->owner, creatng, combt_thing->index);
        set_creature_instance(creatng, inst_use, 0, combt_thing->index, 0);
        external_set_thing_state(combt_thing, CrSt_CreatureEvacuateRoom);
        struct CreatureControl* combctrl = creature_control_get_from_thing(combt_thing);
        combctrl->word_9A = room->index;
        anger_apply_anger_to_creature(creatng, crstat->annoy_going_postal, AngR_Other, 1);
        return true;
    }
    if (thing_is_invalid(combt_thing)) {
        return false;
    }
    if (!setup_person_move_to_coord(creatng, &combt_thing->mappos, NavRtF_Default)) {
        return false;
    }
    // Back to original job - assume the state data is not damaged
    creatng->continue_state = get_continue_state_for_job(going_postal_job);
    return true;
}

/**
 * Processes job stress and going postal due to annoying co-workers.
 * Creatures which aren't performing their primary jobs can be attacked by creatures
 * going postal.
 * Creature doing primary job in room may go postal and attack other creatures walking
 * through the same room which aren't performing the same job (ie. are just passing by),
 * or other workers in that room who does not have the job as primary job.
 *
 * @param creatng The thing being affected by job stress or going postal.
 * @param room The room where target creature should be searched for.
 * @return
 */
TbBool process_job_stress_and_going_postal(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    SYNCDBG(18,"Starting for %s index %d state %s",thing_model_name(creatng),(int)creatng->index,creatrtng_realstate_name(creatng));
    if (cctrl->instance_id != CrInst_NULL) {
        return false;
    }
    // Process the stress once per 20 turns
    //TODO CONFIG export amount of turns to config file
    if (((game.play_gameturn + creatng->index) % 20) != 0) {
        return false;
    }
    struct Room* room = get_room_creature_works_in(creatng);
    if (room_is_invalid(room)) {
        return false;
    }
    // Process the job stress
    if (crstat->annoy_job_stress != 0)
    {
        // Note that this kind of code won't allow one-time jobs, or jobs not related to rooms, to be stressful
        CreatureJob stressful_job = get_creature_job_causing_stress(crstat->job_stress, room->kind);
        if (stressful_job != Job_NULL)
        {
            anger_apply_anger_to_creature(creatng, crstat->annoy_job_stress, AngR_Other, 1);
        }
    }
    // Process going postal
    if (crstat->annoy_going_postal != 0)
    {
        // Make sure we really should go postal in that room
        CreatureJob going_postal_job = get_creature_job_causing_going_postal(crstat->job_primary, room->kind);
        if (going_postal_job != Job_NULL)
        {
            SYNCDBG(18,"The %s index %d has postal job %s",thing_model_name(creatng),(int)creatng->index,creature_job_code_name(going_postal_job));
            if (process_job_causes_going_postal(creatng, room, going_postal_job)) {
                return true;
            }
        }
    }
    return false;
}

TbBool any_worker_will_go_postal_on_creature_in_room(const struct Room *room, const struct Thing *victng)
{
    TRACE_THING(victng);
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (!creature_control_exists(cctrl))
        {
            ERRORLOG("Jump to invalid creature %ld detected",i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
        CreatureJob going_postal_job = Job_NULL;
        if (crstat->annoy_going_postal != 0) {
            going_postal_job = get_creature_job_causing_going_postal(crstat->job_primary,room->kind);
        }
        if (going_postal_job != Job_NULL)
        {
            if (creature_will_go_postal_on_victim_during_job(thing, victng, going_postal_job))
            {
                // We need quick ranged instance to go postal
                if (creature_has_quick_range_weapon(thing)) {
                    return true;
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    return false;
}
/******************************************************************************/
