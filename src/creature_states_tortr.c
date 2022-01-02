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
#include "creature_states_tortr.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_memory.h"
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
    if (!room_initially_valid_as_type_for_thing(room, get_room_for_job(Job_KINKY_TORTURE), thing))
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
    cctrl->assigned_torturer = 0;
    cctrl->turns_at_job = game.play_gameturn;
    cctrl->tortured.start_gameturn = game.play_gameturn;
    cctrl->tortured.long_9Ex = game.play_gameturn;
    cctrl->tortured.vis_state = CTVS_TortureGoToDevice;
    cctrl->tortured.long_A2x = game.play_gameturn;
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
    if (!room_initially_valid_as_type_for_thing(room, get_room_for_job(Job_PAINFUL_TORTURE), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room, Job_PAINFUL_TORTURE))
    {
        output_message_room_related_from_computer_or_player_action(room->owner, room->kind, OMsg_RoomTooSmall);
        set_start_state(thing);
        return 0;
    }
    add_creature_to_torture_room(thing, room);
    cctrl->flgfield_1 |= CCFlg_NoCompControl;
    cctrl->assigned_torturer = 0;
    cctrl->turns_at_job = game.play_gameturn;
    cctrl->tortured.start_gameturn = game.play_gameturn;
    cctrl->tortured.long_9Ex = game.play_gameturn;
    cctrl->tortured.vis_state = CTVS_TortureGoToDevice;
    cctrl->tortured.long_A2x = game.play_gameturn;
    internal_set_thing_state(thing, get_continue_state_for_job(Job_PAINFUL_TORTURE));
    return 1;
}

short cleanup_torturing(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->assigned_torturer > 0)
    {
        struct Thing* thing = thing_get(cctrl->assigned_torturer);
        if (thing_exists(thing)) {
            thing->belongs_to = 0;
            thing->field_4F &= ~TF4F_Unknown01;
        }
        cctrl->assigned_torturer = 0;
    }
    // If the creature has flight ability, return it to flying state
    restore_creature_flight_flag(creatng);
    cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
    remove_creature_from_torture_room(creatng);
    state_cleanup_in_room(creatng);
    return 1;
}

long setup_torture_move_to_device(struct Thing *creatng, struct Room *room, CreatureJob jobpref)
{
    unsigned long k;
    long n = CREATURE_RANDOM(creatng, room->slabs_count);
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
        struct Thing* tortrtng = find_base_thing_on_mapwho(TCls_Object, 125, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (!thing_is_invalid(tortrtng) && (tortrtng->belongs_to == 0))
        {
            if (!setup_person_move_to_coord(creatng, &tortrtng->mappos, NavRtF_Default))
            {
                ERRORLOG("Move failed in torture");
                break;
            }
            struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
            creatng->continue_state = get_continue_state_for_job(jobpref);
            tortrtng->belongs_to = creatng->index;
            tortrtng->word_15 = tortrtng->sprite_size;
            cctrl->assigned_torturer = tortrtng->index;
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
        if (game.play_gameturn - cctrl->tortured.long_9Ex > 100) {
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
            cctrl->tortured.long_9Ex = game.play_gameturn;
            return CrStRet_Unchanged;
        }
        cctrl->tortured.vis_state = CTVS_TortureInDevice;
        cctrl->tortured.long_9Ex = game.play_gameturn;
        return 1;
    case CTVS_TortureInDevice:
        sectng = thing_get(cctrl->assigned_torturer);
        if (creature_turn_to_face_angle(creatng, sectng->move_angle_xy) >= LbFPMath_PI/12) {
            return CrStRet_Unchanged;
        }
        creatng->movement_flags &= ~TMvF_Flying;
        cctrl->spell_flags &= ~CSAfF_Flying;
        creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        if (cctrl->instance_id == CrInst_NULL) {
            set_creature_instance(creatng, CrInst_TORTURED, 1, 0, 0);
        }
        if (thing_exists(sectng)) {
            sectng->field_4F |= TF4F_Unknown01;
        } else {
            ERRORLOG("No device for torture");
        }
        dturn = game.play_gameturn - cctrl->long_A2;
        if ((dturn > 32) || ((cctrl->spell_flags & CSAfF_Speed) && (dturn > 16)))
        {
            play_creature_sound(creatng, CrSnd_Torture, 2, 0);
            cctrl->long_A2 = game.play_gameturn;
        }
        return CrStRet_Unchanged;
    default:
        WARNLOG("Invalid creature state in torture room");
        cctrl->tortured.long_9Ex = game.play_gameturn;
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (game.play_gameturn-cctrl->turns_at_job > crstat->torture_break_time)
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    anger_apply_anger_to_creature(thing, crstat->annoy_in_torture, AngR_Other, 1);
    return CrCkRet_Available;
}

void convert_creature_to_ghost(struct Room *room, struct Thing *thing)
{
    int crmodel = get_room_create_creature_model(room->kind);
    struct Thing* newthing = create_creature(&thing->mappos, crmodel, room->owner);
    if (thing_is_invalid(newthing))
    {
        ERRORLOG("Couldn't create creature %s in %s room",creature_code_name(crmodel),room_code_name(room->kind));
        return;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    init_creature_level(newthing, cctrl->explevel);
    if (creature_model_bleeds(thing->model))
      create_effect_around_thing(newthing, TngEff_Blood5);
    set_start_state(newthing);
    kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects|CrDed_DiedInBattle);
    struct Dungeon* dungeon = get_dungeon(room->owner);
    if (!dungeon_invalid(dungeon)) {
        dungeon->lvstats.ghosts_raised++;
    }
    if (is_my_player_number(room->owner))
        output_message(SMsg_TortureMadeGhost, 0, true);
}

void convert_tortured_creature_owner(struct Thing *creatng, PlayerNumber new_owner)
{
    if (is_my_player_number(new_owner))
    {
        output_message(SMsg_TortureConverted, 0, true);
    } else
    if (is_my_player_number(creatng->owner))
    {
        output_message(SMsg_CreatureJoinedEnemy, 0, true);
    }
    change_creature_owner(creatng, new_owner);
    anger_set_creature_anger_all_types(creatng, 0);
    struct Dungeon* dungeon = get_dungeon(new_owner);
    struct DungeonAdd* dungeonadd = get_dungeonadd(new_owner);
    if (!dungeon_invalid(dungeon)) 
    {
        dungeon->lvstats.creatures_converted++;
        if (((get_creature_model_flags(creatng) & CMF_IsSpectator) == 0) && ((get_creature_model_flags(creatng) & CMF_IsSpecDigger) == 0))
        {
            if (get_creature_model_flags(creatng) & CMF_IsEvil)
            {
                dungeonadd->evil_creatures_converted++;
            }
            else
            {
                dungeonadd->good_creatures_converted++;
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
    if (!thing_is_invalid(heartng))
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

    unsigned char* ownership_map = (unsigned char*)malloc(map_tiles_y * map_tiles_x);
    LbMemorySet(ownership_map,0,map_tiles_y*map_tiles_x);
    for (slb_y=0; slb_y < map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < map_tiles_x; slb_x++)
        {
            slb_num = get_slab_number(slb_x, slb_y);
            slb = get_slabmap_direct(slb_num);
            if (slabmap_owner(slb) != thing->owner)
                ownership_map[slb_num] |= 0x01;
        }
    }
    struct USPOINT_2D* revealed_pts = (struct USPOINT_2D*)malloc((map_tiles_y * map_tiles_x) * sizeof(struct USPOINT_2D));
    unsigned int pts_to_reveal = 32;
    unsigned int pts_count = 0;
    unsigned int pt_idx = 0;

    slb_x = subtile_slab_fast(revealstl_x);
    slb_y = subtile_slab_fast(revealstl_y);
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
          output_message(SMsg_TortureInformation, 0, true);
          return 1;
        }
        if (is_my_player_number(thing->owner)) {
          output_message(SMsg_CreatureRevealInfo, 0, true);
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    long i = ((long)game.play_gameturn - cctrl->tortured.start_gameturn) * room->efficiency / ROOM_EFFICIENCY_MAX;
    if (creature_affected_by_spell(thing, SplK_Speed))
      i = (4 * i) / 3;
    if (creature_affected_by_slap(thing))
      i = (5 * i) / 4;
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long i = ((long)game.play_gameturn - cctrl->tortured.start_gameturn) - (long)crstat->torture_break_time;
    return (i/64 + 1);
}

CrCheckRet process_torture_function(struct Thing *creatng)
{
    long i;
    struct Room* room = get_room_creature_works_in(creatng);
    if ( !room_still_valid_as_type_for_thing(room,RoK_TORTURE,creatng) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng),(int)creatng->owner);
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    anger_apply_anger_to_creature(creatng, crstat->annoy_in_torture, AngR_Other, 1);
    if ((long)game.play_gameturn >= cctrl->turns_at_job + game.turns_per_torture_health_loss)
    {
        i = compute_creature_max_health(game.torture_health_loss,cctrl->explevel);
        remove_health_from_thing_and_display_health(creatng, i);
        cctrl->turns_at_job = (long)game.play_gameturn;
    }
    // Check if we should convert the creature into ghost
    if ((creatng->health < 0) && (game.ghost_convert_chance > 0))
    {
        if (CREATURE_RANDOM(creatng, 100) < game.ghost_convert_chance)
        {
            convert_creature_to_ghost(room, creatng);
            return CrCkRet_Deleted;
        }
    }
    // Other torture functions are available only when torturing enemies
    if (room->owner == creatng->owner)
        return CrCkRet_Available;
    // Torture must take some time before it has any affect
    i = compute_torture_convert_time(creatng,room);
    if ( (i < crstat->torture_break_time) || (cctrl->assigned_torturer == 0) )
        return CrCkRet_Available;
    // After that, every time broke chance is hit, do something
    if (CREATURE_RANDOM(creatng, 100) < compute_torture_broke_chance(creatng))
    {
        if (CREATURE_RANDOM(creatng, 100) >= (int)gameadd.torture_death_chance)
        {
            SYNCDBG(4, "The %s has been broken", thing_model_name(creatng));
            
            if (CREATURE_RANDOM(creatng, 100) < (int)gameadd.torture_convert_chance)
            { // converting creature and ending the torture
                convert_tortured_creature_owner(creatng, room->owner);
                return CrCkRet_Continue;
            }
            else
            { // revealing information about enemy and continuing the torture
                cctrl->tortured.start_gameturn = (long)game.play_gameturn - (long)crstat->torture_break_time / 2;
                reveal_players_map_to_player(creatng, room->owner);
                return CrCkRet_Available;
            }
        } else
        {
            SYNCDBG(4, "The %s died from torture", thing_model_name(creatng));
            if (CREATURE_RANDOM(creatng, 100) < game.ghost_convert_chance)
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
