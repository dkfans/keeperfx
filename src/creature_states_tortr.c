/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_tortr.c
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
#include "creature_states_tortr.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_states_prisn.h"
#include "creature_control.h"
#include "creature_battle.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "room_data.h"
#include "room_jobs.h"
#include "map_blocks.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "post_inc.h"

/******************************************************************************/
/** State triggered when creature reached torture chamber and is ready to start kinky torture action.
 *
 * @param thing The creature.
 * @return
 */
short at_kinky_torture_room(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_role_for_job(Job_KINKY_TORTURE), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room, Job_KINKY_TORTURE))
    {
        set_start_state(thing);
        return 0;
    }
    add_creature_to_torture_room(thing, room);
    cctrl->tortured.assigned_torturer = 0;
    cctrl->turns_at_job = game.play_gameturn;
    cctrl->tortured.start_gameturn = game.play_gameturn;
    cctrl->tortured.state_start_turn = game.play_gameturn;
    cctrl->tortured.vis_state = CTVS_TortureGoToDevice;
    cctrl->tortured.torturer_start_turn = game.play_gameturn;
    internal_set_thing_state(thing, get_continue_state_for_job(Job_KINKY_TORTURE));
    return 1;
}

/** State triggered when creature reached (was dropped to) torture chamber and is ready to start torture action.
 *
 * @param thing The creature.
 * @return
 */
short at_torture_room(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_role_for_job(Job_PAINFUL_TORTURE), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room, Job_PAINFUL_TORTURE))
    {
        output_room_message(room->owner, room->kind, OMsg_RoomTooSmall);
        set_start_state(thing);
        return 0;
    }
    add_creature_to_torture_room(thing, room);
    cctrl->creature_control_flags |= CCFlg_NoCompControl;
    cctrl->tortured.assigned_torturer = 0;
    cctrl->turns_at_job = game.play_gameturn;
    cctrl->tortured.start_gameturn = game.play_gameturn;
    cctrl->tortured.state_start_turn = game.play_gameturn;
    cctrl->tortured.vis_state = CTVS_TortureGoToDevice;
    cctrl->tortured.torturer_start_turn = game.play_gameturn;
    internal_set_thing_state(thing, get_continue_state_for_job(Job_PAINFUL_TORTURE));
    return 1;
}

short cleanup_torturing(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->tortured.assigned_torturer > 0)
    {
        struct Thing* thing = thing_get(cctrl->tortured.assigned_torturer);
        if (thing_exists(thing)) {
            thing->torturer.belongs_to = 0;
            thing->rendering_flags &= ~TRF_Invisible;
        }
        cctrl->tortured.assigned_torturer = 0;
    }
    // If the creature has flight ability, return it to flying state
    restore_creature_flight_flag(creatng);
    cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
    remove_creature_from_torture_room(creatng);
    state_cleanup_in_room(creatng);
    return 1;
}

long setup_torture_move_to_device(struct Thing *creatng, struct Room *room, CreatureJob jobpref)
{
    unsigned long k;
    long n = THING_RANDOM(creatng, room->slabs_count);
    SlabCodedCoords slbnum = room->slabs_list;
    for (k = n; k > 0; k--)
    {
        if (slbnum == 0)
            break;
        slbnum = get_next_slab_number_in_room(slbnum);
    }
    if (slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%d) in %s index %d failed - internal inconsistency.",(int)n,(int)room->slabs_count,room_code_name(room->kind),(int)room->index);
        slbnum = room->slabs_list;
    }
    k = 0;
    while (1)
    {
        MapSlabCoord slb_x = slb_num_decode_x(slbnum);
        MapSlabCoord slb_y = slb_num_decode_y(slbnum);
        struct Thing* tortrtng = find_base_thing_on_mapwho(TCls_Object, ObjMdl_Torturer, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (!thing_is_invalid(tortrtng) && (tortrtng->torturer.belongs_to == 0))
        {
            if (!setup_person_move_to_coord(creatng, &tortrtng->mappos, NavRtF_Default))
            {
                ERRORLOG("Move failed in torture");
                break;
            }
            struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
            creatng->continue_state = get_continue_state_for_job(jobpref);
            tortrtng->torturer.belongs_to = creatng->index;
            tortrtng->torturer.cssize = tortrtng->sprite_size;
            cctrl->tortured.assigned_torturer = tortrtng->index;
            return 1;
        }
        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) {
            slbnum = room->slabs_list;
        }
        k++;
        if (k >= room->slabs_count) {
            break;
        }
    }
    return 0;
}

long process_torture_visuals(struct Thing *creatng, struct Room *room, CreatureJob jobpref)
{
    struct CreatureControl *cctrl;
    struct Thing *sectng;
    cctrl = creature_control_get_from_thing(creatng);
    GameTurnDelta dturn;
    switch (cctrl->tortured.vis_state)
    {
    case CTVS_TortureRandMove:
        if (game.play_gameturn - cctrl->tortured.state_start_turn > 100) {
            cctrl->tortured.vis_state = CTVS_TortureGoToDevice;
        }
        if (!creature_setup_adjacent_move_for_job_within_room(creatng, room, jobpref)) {
            return CrStRet_Unchanged;
        }
        creatng->continue_state = get_continue_state_for_job(jobpref);
        return 1;
    case CTVS_TortureGoToDevice:
        if (!setup_torture_move_to_device(creatng, room, jobpref)) {
            cctrl->tortured.vis_state = CTVS_TortureRandMove;
            cctrl->tortured.state_start_turn = game.play_gameturn;
            return CrStRet_Unchanged;
        }
        cctrl->tortured.vis_state = CTVS_TortureInDevice;
        cctrl->tortured.state_start_turn = game.play_gameturn;
        return 1;
    case CTVS_TortureInDevice:
        sectng = thing_get(cctrl->tortured.assigned_torturer);
        if (creature_turn_to_face_angle(creatng, sectng->move_angle_xy) >= DEGREES_15) {
            return CrStRet_Unchanged;
        }
        creatng->movement_flags &= ~TMvF_Flying;
        cctrl->spell_flags &= ~CSAfF_Flying;
        creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        if (cctrl->instance_id == CrInst_NULL) {
            set_creature_instance(creatng, CrInst_TORTURED, 0, 0);
        }
        if (thing_exists(sectng)) {
            sectng->rendering_flags |= TRF_Invisible;
        } else {
            ERRORLOG("No device for torture");
        }
        dturn = game.play_gameturn - cctrl->tortured.torturer_start_turn;
        if ((dturn > 32) || (creature_under_spell_effect(creatng, CSAfF_Speed) && (dturn > 16)))
        {
            play_creature_sound(creatng, CrSnd_Torture, 2, 0);
            cctrl->tortured.torturer_start_turn = game.play_gameturn;
        }
        return CrStRet_Unchanged;
    default:
        WARNLOG("Invalid creature state in torture room");
        cctrl->tortured.state_start_turn = game.play_gameturn;
        cctrl->tortured.vis_state = CTVS_TortureGoToDevice;
        break;
    }
    return CrStRet_Unchanged;
}

short kinky_torturing(struct Thing *thing)
{
    TRACE_THING(thing);
    struct Room* room = get_room_thing_is_on(thing);
    if (creature_job_in_room_no_longer_possible(room, Job_KINKY_TORTURE, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((game.play_gameturn-cctrl->turns_at_job > crconf->torture_break_time) && !is_neutral_thing(thing))
    {
        set_start_state(thing);
        return CrStRet_ResetOk;
    }
    switch (process_kinky_function(thing))
    {
    case CrCkRet_Deleted:
        return CrStRet_Deleted;
    case CrCkRet_Available:
        process_torture_visuals(thing, room, Job_KINKY_TORTURE);
        return CrStRet_Modified;
    default:
        return CrStRet_ResetOk;
    }
}

CrCheckRet process_kinky_function(struct Thing *thing)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    anger_apply_anger_to_creature(thing, crconf->annoy_in_torture, AngR_Other, 1);
    return CrCkRet_Available;
}

void convert_creature_to_ghost(struct Room *room, struct Thing *thing)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    ThingModel crmodel = crconf->torture_kind;
    if ((crmodel > game.conf.crtr_conf.model_count) || (crmodel <= 0))
    {
        // If not assigned or is unknown, default to the room creature creation.
        crmodel = get_room_create_creature_model(room->kind);
    }
    struct Thing* newthing = INVALID_THING;
    if (creature_count_below_map_limit(1))
    {
        newthing = create_creature(&thing->mappos, crmodel, room->owner);
        if (thing_is_invalid(newthing))
        {
            ERRORLOG("Couldn't create creature %s in %s room", creature_code_name(crmodel), room_code_name(room->kind));
            return;
        }
    } else
    {
        WARNLOG("Could not create creature %s to transform %s to due to creature limit", creature_code_name(crmodel), thing_model_name(thing));
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureControl* newcctrl = creature_control_get_from_thing(newthing);
    init_creature_level(newthing, cctrl->exp_level);
    if (creature_model_bleeds(thing->model)) {
        create_effect_around_thing(newthing, TngEff_Blood5); // TODO CONFIG: make this effect configurable?
    }
    set_start_state(newthing);
    strcpy(newcctrl->creature_name, cctrl->creature_name);
    kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects|CrDed_DiedInBattle);
    struct Dungeon* dungeon = get_dungeon(room->owner);
    if (!dungeon_invalid(dungeon)) {
        dungeon->lvstats.ghosts_raised++;
    }
    if (is_my_player_number(room->owner)) {
        output_message(SMsg_TortureMadeGhost, 0);
    }
}

void convert_tortured_creature_owner(struct Thing *creatng, PlayerNumber new_owner)
{
    if (is_my_player_number(new_owner))
    {
        output_message(SMsg_TortureConverted, 0);
    } else
    if (is_my_player_number(creatng->owner))
    {
        output_message(SMsg_CreatureJoinedEnemy, 0);
    }
    change_creature_owner(creatng, new_owner);
    anger_set_creature_anger_all_types(creatng, 0);
    struct Dungeon* dungeon = get_dungeon(new_owner);
    if (!dungeon_invalid(dungeon))
    {
        dungeon->lvstats.creatures_converted++;
        if (((get_creature_model_flags(creatng) & CMF_IsSpectator) == 0) && ((get_creature_model_flags(creatng) & CMF_IsSpecDigger) == 0))
        {
            if (get_creature_model_flags(creatng) & CMF_IsEvil)
            {
                dungeon->evil_creatures_converted++;
            }
            else
            {
                dungeon->good_creatures_converted++;
            }
        }
    }
}

long reveal_players_map_to_player(struct Thing *thing, PlayerNumber benefit_plyr_idx)
{
    SlabCodedCoords slb_num;
    struct SlabMap *slb;
    MapSubtlCoord revealstl_x;
    MapSubtlCoord revealstl_y;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    TRACE_THING(thing);
    struct Thing* heartng = get_player_soul_container(thing->owner);
    if (thing_exists(heartng))
    {
        TRACE_THING(heartng);
        revealstl_x = heartng->mappos.x.stl.num;
        revealstl_y = heartng->mappos.y.stl.num;
    } else
    {
        setup_combat_flee_position(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        revealstl_x = cctrl->flee_pos.x.stl.num;
        revealstl_y = cctrl->flee_pos.y.stl.num;
    }
    TbBool reveal_success = 0;

    unsigned char* ownership_map = (unsigned char*)malloc(game.map_tiles_y * game.map_tiles_x);
    memset(ownership_map,0,game.map_tiles_y*game.map_tiles_x);
    for (slb_y=0; slb_y < game.map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < game.map_tiles_x; slb_x++)
        {
            slb_num = get_slab_number(slb_x, slb_y);
            slb = get_slabmap_direct(slb_num);
            if (slabmap_owner(slb) != thing->owner)
                ownership_map[slb_num] |= 0x01;
        }
    }
    struct USPOINT_2D* revealed_pts = (struct USPOINT_2D*)malloc((game.map_tiles_y * game.map_tiles_x) * sizeof(struct USPOINT_2D));
    unsigned int pts_to_reveal = 32;
    unsigned int pts_count = 0;
    unsigned int pt_idx = 0;

    slb_x = subtile_slab(revealstl_x);
    slb_y = subtile_slab(revealstl_y);
    slb_num = get_slab_number(slb_x, slb_y);
    ownership_map[slb_num] |= 0x02;
    do
    {
        // Reveal given point
        if ( !subtile_revealed(slab_subtile_center(slb_x), slab_subtile_center(slb_y), benefit_plyr_idx) )
        {
            reveal_success = 1;
            clear_slab_dig(slb_x, slb_y, benefit_plyr_idx);
            set_slab_explored(benefit_plyr_idx, slb_x, slb_y);
            pts_to_reveal--;
            if (pts_to_reveal == 0)
              break;
        }
        // Add sibling points to reveal list
        slb_num = get_slab_number(slb_x-1, slb_y);
        if ((ownership_map[slb_num] & 0x03) == 0)
        {
            ownership_map[slb_num] |= 0x02;
            slb = get_slabmap_direct(slb_num);
            if (slabmap_owner(slb) == thing->owner) {
                revealed_pts[pts_count].x = slb_x - 1;
                revealed_pts[pts_count].y = slb_y;
                pts_count++;
            }
        }
        slb_num = get_slab_number(slb_x+1, slb_y);
        if ((ownership_map[slb_num] & 0x03) == 0)
        {
            ownership_map[slb_num] |= 0x02;
            slb = get_slabmap_direct(slb_num);
            if (slabmap_owner(slb) == thing->owner) {
                revealed_pts[pts_count].x = slb_x + 1;
                revealed_pts[pts_count].y = slb_y;
                pts_count++;
            }
        }
        slb_num = get_slab_number(slb_x, slb_y-1);
        if ((ownership_map[slb_num] & 0x03) == 0)
        {
            ownership_map[slb_num] |= 0x02;
            slb = get_slabmap_direct(slb_num);
            if (slabmap_owner(slb) == thing->owner) {
                revealed_pts[pts_count].x = slb_x;
                revealed_pts[pts_count].y = slb_y - 1;
                pts_count++;
            }
        }
        slb_num = get_slab_number(slb_x, slb_y+1);
        if ((ownership_map[slb_num] & 0x03) == 0)
        {
            ownership_map[slb_num] |= 0x02;
            slb = get_slabmap_direct(slb_num);
            if (slabmap_owner(slb) == thing->owner) {
                revealed_pts[pts_count].x = slb_x;
                revealed_pts[pts_count].y = slb_y + 1;
                pts_count++;
            }
        }
        slb_x = revealed_pts[pt_idx].x;
        slb_y = revealed_pts[pt_idx].y;
        pt_idx++;
    }
    while ( pts_count >= pt_idx );
    free(revealed_pts);
    free(ownership_map);

    if (reveal_success)
    {
        if (is_my_player_number(benefit_plyr_idx)) {
          output_message(SMsg_TortureInformation, 0);
          return 1;
        }
        if (is_my_player_number(thing->owner)) {
          output_message(SMsg_CreatureRevealInfo, 0);
          return 1;
        }
    }
    return 1;
}

/**
 * Computes the average time required for a torture victim to be converted in given room.
 *
 * @param thing The victim creature.
 * @param room The torture chamber room.
 */
long compute_torture_convert_time(const struct Thing *thing, const struct Room *room)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    long i = ((long)game.play_gameturn - cctrl->tortured.start_gameturn) * room->efficiency / ROOM_EFFICIENCY_MAX;
    if (creature_under_spell_effect(thing, CSAfF_Speed))
    {
        i = (4 * i) / 3;
    }
    if (creature_affected_by_slap(thing))
    {
        i = (5 * i) / 4;
    }
    return i;
}

/**
 * Computes the average time required for a torture victim to start revealing information.
 *
 * @param thing The victim creature.
 * @param room The torture chamber room.
 */
long compute_torture_broke_chance(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long i = ((long)game.play_gameturn - cctrl->tortured.start_gameturn) - (long)crconf->torture_break_time;
    return (i/64 + 1);
}

CrCheckRet process_torture_function(struct Thing *creatng)
{
    long i;
    struct Room *room = get_room_creature_works_in(creatng);
    if (!room_still_valid_as_type_for_thing(room, RoRoF_Torture, creatng))
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d", room_code_name(room->kind), (int)room->owner, thing_model_name(creatng), (int)creatng->owner);
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    if ((game.conf.rules[room->owner].game.classic_bugs_flags & ClscBug_NeutralTortureConverts) == 0)
    {
        if (room->owner == game.neutral_player_num || is_neutral_thing(creatng))
        {
            return CrCkRet_Available;
        }
    }
    struct CreatureModelConfig *crconf = creature_stats_get_from_thing(creatng);
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    anger_apply_anger_to_creature(creatng, crconf->annoy_in_torture, AngR_Other, 1);
    if ((long)game.play_gameturn >= cctrl->turns_at_job + game.conf.rules[room->owner].health.turns_per_torture_health_loss)
    {
        HitPoints torture_damage = compute_creature_max_health(game.conf.rules[room->owner].health.torture_health_loss, cctrl->exp_level);
        remove_health_from_thing_and_display_health(creatng, torture_damage);
        cctrl->turns_at_job = (long)game.play_gameturn;
    }
    // Check if we should convert the creature into ghost.
    if ((creatng->health < 0) && (game.conf.rules[room->owner].rooms.ghost_convert_chance > 0))
    {
        if (THING_RANDOM(creatng, 100) < game.conf.rules[room->owner].rooms.ghost_convert_chance)
        {
            convert_creature_to_ghost(room, creatng);
            return CrCkRet_Deleted;
        }
    }
    // Other torture functions are available only when torturing enemies.
    if (room->owner == creatng->owner)
    {
        return CrCkRet_Available;
    }
    // Torture must take some time before it has any affect.
    i = compute_torture_convert_time(creatng, room);
    if ((i < crconf->torture_break_time) || (cctrl->tortured.assigned_torturer == 0))
    {
        return CrCkRet_Available;
    }
    // After that, every time broke chance is hit, do something.
    if (THING_RANDOM(creatng, 100) < compute_torture_broke_chance(creatng))
    {
        if (THING_RANDOM(creatng, 100) >= (int)game.conf.rules[room->owner].rooms.torture_death_chance)
        {
            SYNCDBG(4, "The %s has been broken", thing_model_name(creatng));
            if (THING_RANDOM(creatng, 100) < (int)game.conf.rules[room->owner].rooms.torture_convert_chance)
            { // Converting creature and ending the torture.
                convert_tortured_creature_owner(creatng, room->owner);
                return CrCkRet_Continue;
            }
            else
            { // Revealing information about enemy and continuing the torture.
                cctrl->tortured.start_gameturn = (long)game.play_gameturn - (long)crconf->torture_break_time / 2;
                reveal_players_map_to_player(creatng, room->owner);
                return CrCkRet_Available;
            }
        }
        else
        {
            SYNCDBG(4, "The %s died from torture", thing_model_name(creatng));
            if (THING_RANDOM(creatng, 100) < game.conf.rules[room->owner].rooms.ghost_convert_chance)
            {
                convert_creature_to_ghost(room, creatng);
                return CrCkRet_Deleted;
            }
            else
            {
                creatng->health = -1;
            }
        }
    }
    return CrCkRet_Available;
}

CrStateRet torturing(struct Thing *thing)
{
    TRACE_THING(thing);
    struct Room* room = get_room_thing_is_on(thing);
    if (creature_job_in_room_no_longer_possible(room, Job_PAINFUL_TORTURE, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    switch (process_torture_function(thing))
    {
    case CrCkRet_Deleted:
        return CrStRet_Deleted;
    case CrCkRet_Available:
        process_torture_visuals(thing, room, Job_PAINFUL_TORTURE);
        return CrStRet_Modified;
    default:
        return CrStRet_ResetOk;
    }
}

/******************************************************************************/
