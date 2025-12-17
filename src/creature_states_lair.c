/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_lair.c
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
#include "creature_states_lair.h"
#include "globals.h"

#include "bflib_math.h"
#include "thing_physics.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_states_mood.h"
#include "creature_states_spdig.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "config_effects.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_lair.h"
#include "room_util.h"
#include "map_utils.h"
#include "engine_arrays.h"
#include "game_legacy.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Returns if given creature is able to heal by sleeping.
 * Does not take into consideration if the creature has a lair, checks only if
 * the creature model is able to heal in its lair in general.
 * @param creatng
 * @return
 */
TbBool creature_can_do_healing_sleep(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    return ((crconf->heal_requirement > 0) && (crconf->lair_size > 0));
}

TbBool creature_is_sleeping(const struct Thing *thing)
{
    long i = thing->active_state;
    if (i == CrSt_CreatureSleep)
        return true;
    return false;
}

TbBool creature_is_doing_toking(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_CreatureGoingToSafetyForToking) || (i == CrSt_ImpToking))
        return true;
    return false;
}

TbBool creature_is_doing_lair_activity(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_CreatureSleep) || (i == CrSt_CreatureGoingHomeToSleep) || (i == CrSt_AtLairToSleep)
      || (i == CrSt_CreatureChooseRoomForLairSite) || (i == CrSt_CreatureAtNewLair) || (i == CrSt_CreatureWantsAHome)
      || (i == CrSt_CreatureChangeLair) || (i == CrSt_CreatureAtChangedLair))
        return true;
    return false;
}

TbBool creature_requires_healing(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    HitPoints minhealth = crconf->heal_requirement * cctrl->max_health / 256;
    if ((long)thing->health <= minhealth)
        return true;
    return false;
}

TbBool creature_move_to_home_lair(struct Thing *creatng)
{
    if (!creature_has_lair_room(creatng)) {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if (!thing_exists(lairtng)) {
        return false;
    }
    return setup_person_move_to_coord(creatng, &lairtng->mappos, NavRtF_Default);

}

long creature_will_sleep(struct Thing *thing)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    TRACE_THING(lairtng);
    if (!thing_exists(lairtng))
        return false;
    long dist_x = (long)thing->mappos.x.stl.num - (long)lairtng->mappos.x.stl.num;
    long dist_y = (long)thing->mappos.y.stl.num - (long)lairtng->mappos.y.stl.num;
    return (abs(dist_x) < 1) && (abs(dist_y) < 1);
}

/**
 * @brief special digger drop unconscious creatures in their lair
 *
 * only if drag_to_lair rule in activated
 *
 * @param thing special digger who drag the creature
 * @return returns 1 if creature successfully arrived at its lair and woke up
 */
short creature_drop_unconscious_in_lair(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* dragtng = thing_get(cctrl->dragtng_idx);

    if (!thing_exists(dragtng) || !creature_is_being_unconscious(dragtng))
    {
        set_start_state(thing);
        return 0;
    }
    struct CreatureControl* dragctrl = creature_control_get_from_thing(dragtng);
    struct Room* room = get_room_thing_is_on(thing);
    struct Thing* totemtng = find_lair_totem_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    // if place is not a room
    if  (!subtile_is_room(thing->mappos.x.stl.num, thing->mappos.y.stl.num)
            // or room is not a lair
        || (!room_role_matches(room->kind, RoRoF_LairStorage)
            //or room owner is not creature owner
            || room->owner != dragtng->owner
            //or creature has no lair room
            || (dragctrl->lair_room_id == 0
                // and the lair has no capacity
                && (room->used_capacity >= room ->total_capacity)))
        // or there is a lair already but it doesn't belong to the creature
        || ((totemtng->index > 0) && (totemtng->index != dragctrl->lairtng_idx)))
    {
        //just drop the creature
        creature_drop_dragged_object(thing, dragtng);
        set_start_state(thing);
        return 0;
    }

    make_creature_conscious(dragtng);
    // if the creature already has a lair here it's going to sleep
    if (dragctrl->lair_room_id == room->index)
    {
        initialise_thing_state(dragtng, CrSt_CreatureGoingHomeToSleep);
    }
    // if the creature dont has a lair here make a new one
    else
    {
        initialise_thing_state(dragtng, CrSt_CreatureAtNewLair);
    }
    set_flag(dragctrl->creature_control_flags,CCFlg_NoCompControl);
    set_start_state(thing);
    return 1;

}

long process_lair_enemy(struct Thing *thing, struct Room *room)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // Shouldn't be possible. But just for sure.
    if (room_is_invalid(room))
    {
        return 0;
    }
    // If the room changed during creature's journey, end
    if (!room_role_matches(room->kind, RoRoF_CrHealSleep) || (room->owner != thing->owner) || (room->index != cctrl->lair_room_id))
    {
        return 0;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    // End if the creature has no lair enemy
    if (crconf->lair_enemy[0] == 0)
    {
        return 0;
    }
    // Search for enemies no often than every 64 turns
    if (((game.play_gameturn + thing->index) & 0x3F) != 0)
    {
        return 0;
    }
    struct Thing* enemytng;
    long combat_factor = find_fellow_creature_to_fight_in_room(thing, room, crconf->lair_enemy, &enemytng);
    if (combat_factor < 1)
        return 0;
    if (!set_creature_in_combat_to_the_death(thing, enemytng, combat_factor))
        return 0;
    return 1;
}

CrStateRet creature_add_lair_to_room(struct Thing *creatng, struct Room *room)
{
    if (!room_has_enough_free_capacity_for_creature_job(room, creatng, Job_TAKE_SLEEP))
        return CrStRet_ResetFail;
    // Make sure we don't already have a lair on that position
    struct Thing* lairtng = find_creature_lair_totem_at_subtile(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, 0);
    if (!thing_is_invalid(lairtng))
        return CrStRet_Unchanged;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    room->content_per_model[creatng->model]++;
    room->used_capacity += get_required_room_capacity_for_object(RoRoF_LairStorage, 0, creatng->model);
    if ((cctrl->lair_room_id > 0) && (cctrl->lairtng_idx > 0))
    {
        struct Room* origroom = room_get(cctrl->lair_room_id);
        creature_remove_lair_totem_from_room(creatng, origroom);
    }
    cctrl->lair_room_id = room->index;
    // Create the lair thing
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    struct CreatureModelConfig* crconf = creature_stats_get(creatng->model);
    lairtng = create_object(&pos, crconf->lair_object, creatng->owner, -1);
    if (thing_is_invalid(lairtng))
    {
        ERRORLOG("Could not create lair totem");
        remove_thing_from_mapwho(creatng);
        place_thing_in_mapwho(creatng);
        return CrStRet_Modified; // Return that so we won't try to redo the action over and over
    }
    lairtng->move_angle_xy = THING_RANDOM(creatng, DEGREES_360);
    lairtng->mappos.z.val = get_thing_height_at(lairtng, &lairtng->mappos);
    // Associate creature with the lair
    cctrl->lairtng_idx = lairtng->index;
    lairtng->lair.belongs_to = creatng->index;
    lairtng->lair.cssize = 1;
    // Lair size depends on creature level
    lairtng->lair.spr_size = game.conf.crtr_conf.sprite_size + (game.conf.crtr_conf.sprite_size * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100;
    lairtng->move_angle_xy = THING_RANDOM(creatng, DEGREES_360);
    struct ObjectConfigStats* objst = get_object_model_stats(lairtng->model);
    unsigned long i = convert_td_iso(objst->sprite_anim_idx);
    set_thing_draw(lairtng, i, objst->anim_speed, lairtng->lair.cssize, 0, -1, objst->draw_class);
    thing_play_sample(creatng, 158, NORMAL_PITCH, 0, 3, 1, 2, FULL_LOUDNESS);
    create_effect(&pos, imp_spangle_effects[get_player_color_idx(creatng->owner)], creatng->owner);
    anger_set_creature_anger(creatng, 0, AngR_NoLair);
    remove_thing_from_mapwho(creatng);
    place_thing_in_mapwho(creatng);
    return CrStRet_ResetOk;
}

CrStateRet creature_at_changed_lair(struct Thing *creatng)
{
    TRACE_THING(creatng);
    if (!thing_is_on_own_room_tile(creatng))
    {
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    struct Room* room = get_room_thing_is_on(creatng);
    if (!room_initially_valid_as_type_for_thing(room, get_room_role_for_job(Job_TAKE_SLEEP), creatng))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }

    CrStateRet laircreated = creature_add_lair_to_room(creatng, room);
    if (laircreated != CrStRet_ResetOk)
    {
        if (laircreated != CrStRet_Modified)
            internal_set_thing_state(creatng, CrSt_CreatureChooseRoomForLairSite);
        return laircreated;
    }
    // All done - finish the state
    set_start_state(creatng);
    return CrStRet_ResetOk;
}

CrStateRet creature_at_new_lair(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Room* room = get_room_thing_is_on(creatng);
    if ( !room_still_valid_as_type_for_thing(room, get_room_role_for_job(Job_TAKE_SLEEP), creatng) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s index %d owner %d",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }

    CrStateRet laircreated = creature_add_lair_to_room(creatng, room);
    if (laircreated != CrStRet_ResetOk)
    {
        if (laircreated != CrStRet_Modified)
            internal_set_thing_state(creatng, CrSt_CreatureChooseRoomForLairSite);
        return laircreated;
    }
    // All done - finish the state
    set_start_state(creatng);
    return CrStRet_ResetOk;
}

TbBool setup_head_for_random_unused_lair_subtile(struct Thing *creatng, struct Room *room)
{
    unsigned long k;
    long n = THING_RANDOM(creatng, room->slabs_count);
    SlabCodedCoords start_slbnum = room->slabs_list;
    for (k = n; k > 0; k--)
    {
        if (start_slbnum == 0)
            break;
        start_slbnum = get_next_slab_number_in_room(start_slbnum);
    }
    if (start_slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%d) in %s index %d failed - internal inconsistency.",(int)n,(int)room->slabs_count,room_code_name(room->kind),(int)room->index);
        start_slbnum = room->slabs_list;
    }
    SlabCodedCoords slbnum = start_slbnum;
    // Loop for subtiles on a slab; first check the central one, hopefully we will not get to checking other subtiles
    // It is very rare to have more than one lair on a slab, as it will look overlapping; but may be needed if efficiency is large enough
    for (n = 0; n < MID_AROUND_LENGTH; n++)
    {
        MapSubtlDelta ssub_x = 1 + start_at_around[n].delta_x;
        MapSubtlDelta ssub_y = 1 + start_at_around[n].delta_y;
        for (k = 0; k < room->slabs_count; k++)
        {
            MapSlabCoord slb_x = slb_num_decode_x(slbnum);
            MapSlabCoord slb_y = slb_num_decode_y(slbnum);
            struct Thing* lairtng = find_creature_lair_totem_at_subtile(slab_subtile(slb_x, ssub_x), slab_subtile(slb_y, ssub_y), 0);
            if (thing_is_invalid(lairtng))
            {
                if (setup_person_move_to_position(creatng, slab_subtile(slb_x, ssub_x), slab_subtile(slb_y, ssub_y), NavRtF_Default)) {
                    if (n > 0) {
                        WARNDBG(2,"More than one lair totem will be placed on a %s index %d slab %d,%d",room_code_name(room->kind),(int)room->index,(int)slb_x,(int)slb_y);
                    }
                    return true;
                }
                WARNLOG("Cannot get somewhere in room");
            }
            slbnum = get_next_slab_number_in_room(slbnum);
            if (slbnum == 0) {
                slbnum = room->slabs_list;
            }
        }
    }
    ERRORLOG("Could not find valid RANDOM point in room for creature");
    return false;
}

short creature_change_lair(struct Thing *thing)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_role_for_job(Job_TAKE_SLEEP), thing))
    {
        set_start_state(thing);
        return 0;
    }
    if (!room_has_enough_free_capacity_for_creature_job(room, thing, Job_TAKE_SLEEP))
    {
        set_start_state(thing);
        return 0;
    }
    if (!setup_head_for_random_unused_lair_subtile(thing, room))
    {
        set_start_state(thing);
        return 0;
    }
    if (cctrl->lair_room_id != 0) {
        thing->continue_state = CrSt_CreatureAtChangedLair;
    } else {
        thing->continue_state = CrSt_CreatureAtNewLair;
    }
    return 1;
}

short creature_choose_room_for_lair_site(struct Thing *thing)
{
    TRACE_THING(thing);
    struct Room* room = get_best_new_lair_for_creature(thing);
    if (room_is_invalid(room))
    {
        update_cannot_find_room_of_role_wth_spare_capacity_event(thing->owner, thing, get_room_role_for_job(Job_TAKE_SLEEP));
        set_start_state(thing);
        return 0;
    }
    if (!setup_head_for_random_unused_lair_subtile(thing, room))
    {
        ERRORLOG("Chosen lair is not valid, internal inconsistency.");
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = CrSt_CreatureAtNewLair;
    return 1;
}

short at_lair_to_sleep(struct Thing *thing)
{
    TRACE_THING(thing);
    reset_interpolation_of_thing(thing); // Fixes rendering bug 'Creatures behind their lair in straight view'
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    TRACE_THING(lairtng);
    cctrl->target_room_id = 0;
    if (!thing_exists(lairtng) || creature_affected_by_slap(thing))
    {
        set_start_state(thing);
        return 0;
    }
    if (!creature_will_sleep(thing))
    {
        set_start_state(thing);
        return 0;
    }
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_role_for_job(Job_TAKE_SLEEP), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d owner %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->index,(int)thing->owner);
        set_start_state(thing);
        return 0;
    }
    if ((cctrl->lair_room_id != room->index))
    {
        set_start_state(thing);
        return 0;
    }
    if (creature_turn_to_face_angle(thing, lairtng->move_angle_xy) <= 0)
    {
        internal_set_thing_state(thing, CrSt_CreatureSleep);
        cctrl->turns_at_job = 200;
        thing->movement_flags &= ~TMvF_Flying;
    }
    process_lair_enemy(thing, room);
    return 1;
}

short cleanup_sleep(struct Thing *creatng)
{
    restore_creature_flight_flag(creatng);
    return 1;
}

short creature_going_home_to_sleep(struct Thing *thing)
{
    if (creature_move_to_home_lair(thing))
    {
        thing->continue_state = CrSt_AtLairToSleep;
        return 1;
    }
    internal_set_thing_state(thing, CrSt_CreatureWantsAHome);
    return 1;
}

long room_has_slab_adjacent(const struct Room *room, long slbkind)
{
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i > 0)
    {
        // Per room tile code
        for (long n = 0; n < AROUND_SLAB_LENGTH; n++)
        {
            long slab_num = i + game.around_slab[n];
            struct SlabMap* slb = get_slabmap_direct(slab_num);
            if (!slabmap_block_invalid(slb))
            {
                if (slb->kind == slbkind) {
                    return true;
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
    return 0;
}

short creature_sleep(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_affected_by_slap(thing) || !creature_will_sleep(thing))
    {
        set_start_state(thing);
        return 0;
    }
    struct Room *room = get_room_thing_is_on(thing);
    if (room_is_invalid(room)
    || (!room_role_matches(room->kind, get_room_role_for_job(Job_TAKE_SLEEP)))
    || (cctrl->lair_room_id != room->index)
    || (room->owner != thing->owner))
    {
        set_start_state(thing);
        return 0;
    }
    thing->movement_flags &= ~0x0020;
    struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
    // Recovery is disabled if frequency is set to 0 on rules.cfg.
    if (game.conf.rules[thing->owner].creature.recovery_frequency > 0)
    {
        if (((game.play_gameturn + thing->index) % game.conf.rules[thing->owner].creature.recovery_frequency) == 0)
        {
            HitPoints recover = compute_creature_max_health(crconf->sleep_recovery, cctrl->exp_level);
            apply_health_to_thing_and_display_health(thing, recover);
        }
    }
    anger_set_creature_anger(thing, 0, AngR_NoLair);
    anger_apply_anger_to_creature(thing, crconf->annoy_sleeping, AngR_Other, 1);
    if (cctrl->turns_at_job > 0)
    {
        cctrl->turns_at_job--;
    }
    if (((game.play_gameturn + thing->index) & 0x3F) == 0)
    {
        if (THING_RANDOM(thing, 100) < 5)
        {
            struct Dungeon *dungeon = get_dungeon(thing->owner);
            dungeon->lvstats.backs_stabbed++;
        }
    }
    long XP = 0;
    for (unsigned int i = 0; i < SLEEP_XP_COUNT; i++)
    {
        if (crconf->sleep_experience[i] > XP)
        {
            if (room_has_slab_adjacent(room, crconf->sleep_exp_slab[i]))
            {
                XP = crconf->sleep_experience[i];
            }
        }
    }
    if (XP != 0)
    {
        if (creature_can_gain_experience(thing))
        {
            cctrl->exp_points += XP;
            check_experience_upgrade(thing);
        }
    }
    {
        if ((thing->health >= crconf->heal_threshold * cctrl->max_health / 256) && (!cctrl->turns_at_job))
        {
            set_start_state(thing);
            return 1;
        }
    }
    process_lair_enemy(thing, room);
    return 0;
}

/******************************************************************************/
