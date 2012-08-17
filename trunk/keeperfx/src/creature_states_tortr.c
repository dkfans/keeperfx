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
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_battle.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "map_blocks.h"
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_kinky_torture_room(struct Thing *thing);
DLLIMPORT short _DK_at_torture_room(struct Thing *thing);
DLLIMPORT short _DK_cleanup_torturing(struct Thing *thing);
DLLIMPORT short _DK_kinky_torturing(struct Thing *thing);
DLLIMPORT long _DK_process_torture_function(struct Thing *thing);
DLLIMPORT short _DK_torturing(struct Thing *thing);
DLLIMPORT long _DK_process_torture_visuals(struct Thing *thing, struct Room *room, long a3);
DLLIMPORT long _DK_reveal_players_map_to_player(struct Thing *thing, long benefit_plyr_idx);
DLLIMPORT long _DK_process_kinky_function(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/** State triggered when creature reached torture chamber and is ready to start kinky torture action.
 *
 * @param thing The creature.
 * @return
 */
short at_kinky_torture_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_at_kinky_torture_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_TORTURE) || (room->owner != thing->owner))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        set_start_state(thing);
        return 0;
    }
    add_creature_to_torture_room(thing, room);
    cctrl->word_A6 = 0;
    cctrl->field_82 = game.play_gameturn;
    cctrl->tortured.start_gameturn = game.play_gameturn;
    cctrl->long_9E = game.play_gameturn;
    cctrl->field_A8 = 1;
    cctrl->long_A2 = game.play_gameturn;
    internal_set_thing_state(thing, CrSt_KinkyTorturing);
    return 1;
}

/** State triggered when creature reached (was dropped to) torture chamber and is ready to start torture action.
 *
 * @param thing The creature.
 * @return
 */
short at_torture_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_at_torture_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room) || (room->kind != RoK_TORTURE))
    {
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        if (is_my_player_number(room->owner))
            output_message(SMsg_TortureTooSmall, 0, true);
        set_start_state(thing);
        return 0;
    }
    add_creature_to_torture_room(thing, room);
    cctrl->flgfield_1 |= 0x02;
    cctrl->word_A6 = 0;
    cctrl->field_82 = game.play_gameturn;
    cctrl->tortured.start_gameturn = game.play_gameturn;
    cctrl->long_9E = game.play_gameturn;
    cctrl->field_A8 = 1;
    cctrl->long_A2 = game.play_gameturn;
    internal_set_thing_state(thing, CrSt_Torturing);
    return 1;
}

short cleanup_torturing(struct Thing *thing)
{
  return _DK_cleanup_torturing(thing);
}

long process_torture_visuals(struct Thing *thing, struct Room *room, long a3)
{
  return _DK_process_torture_visuals(thing, room, a3);
}

short kinky_torturing(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_kinky_torturing(thing);
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if (!room_still_valid_as_type_for_thing(room, RoK_TORTURE, thing) || (cctrl->work_room_id != room->index))
    {
        WARNLOG("Room %s index %d is not the one %s worked in",room_code_name(room->kind),(int)room->index,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (game.play_gameturn-cctrl->field_82 > crstat->torture_time)
    {
        set_start_state(thing);
        return 0;
    }
    process_kinky_function(thing);
    process_torture_visuals(thing, room, 110);
    return 1;
}

long process_kinky_function(struct Thing *thing)
{
  struct CreatureStats *crstat;
  //return _DK_process_kinky_function(thing);
  crstat = creature_stats_get_from_thing(thing);
  anger_apply_anger_to_creature(thing, crstat->annoy_in_torture, 4, 1);
  return 0;
}

void convert_creature_to_ghost(struct Room *room, struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *newthing;
    int crmodel;
    crmodel = get_room_create_creature_model(room->kind);
    newthing = create_creature(&thing->mappos, crmodel, room->owner);
    if (thing_is_invalid(newthing))
    {
        ERRORLOG("Couldn't create creature model %d in %s room",crmodel,room_code_name(room->kind));
        return;
    }
    cctrl = creature_control_get_from_thing(thing);
    init_creature_level(newthing, cctrl->explevel);
    if (creature_model_bleeds(thing->model))
      create_effect_around_thing(newthing, TngEff_Unknown10);
    set_start_state(newthing);
    kill_creature(thing, INVALID_THING, -1, 1, 1, 0);
    dungeon = get_dungeon(room->owner);
    if (!dungeon_invalid(dungeon))
        dungeon->lvstats.ghosts_raised++;
    if (is_my_player_number(room->owner))
        output_message(SMsg_TortureMadeGhost, 0, true);
}

void convert_tortured_creature_owner(struct Thing *thing, long new_owner)
{
    struct Dungeon *dungeon;
    if (is_my_player_number(new_owner))
    {
        output_message(SMsg_TortureConverted, 0, true);
    } else
    if (is_my_player_number(thing->owner))
    {
        output_message(SMsg_CreatureJoinedEnemy, 0, true);
    }
    change_creature_owner(thing, new_owner);
    anger_set_creature_anger_all_types(thing, 0);
    dungeon = get_dungeon(new_owner);
    if (!dungeon_invalid(dungeon))
        dungeon->lvstats.creatures_converted++;
}

long reveal_players_map_to_player(struct Thing *thing, long benefit_plyr_idx)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *heartng;
    SlabCodedCoords slb_num;
    struct SlabMap *slb;
    int revealstl_x,revealstl_y;
    MapSubtlCoord slb_x, slb_y;
    unsigned char *ownership_map;
    struct USPOINT_2D *revealed_pts;
    unsigned int pt_idx,pts_count,pts_to_reveal;
    TbBool reveal_success;
    TRACE_THING(thing);
    //return _DK_reveal_players_map_to_player(thing, a2);
    dungeon = get_dungeon(thing->owner);

    if (dungeon->dnheart_idx > 0)
    {
        heartng = thing_get(dungeon->dnheart_idx);
        TRACE_THING(heartng);
        revealstl_x = heartng->mappos.x.stl.num;
        revealstl_y = heartng->mappos.y.stl.num;
    } else
    {
        setup_combat_flee_position(thing);
        cctrl = creature_control_get_from_thing(thing);
        revealstl_x = cctrl->flee_pos.x.stl.num;
        revealstl_y = cctrl->flee_pos.y.stl.num;
    }
    reveal_success = 0;

    ownership_map = (unsigned char *)malloc(map_tiles_y*map_tiles_x);
    memset(ownership_map,0,map_tiles_y*map_tiles_x);
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
    revealed_pts = (struct USPOINT_2D *)malloc((map_tiles_y*map_tiles_x)*sizeof(struct USPOINT_2D));
    pts_to_reveal = 32;
    pts_count = 0;
    pt_idx = 0;

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
          output_message(SMsg_TortureInformation, 0, 1);
          return 1;
        }
        if (is_my_player_number(thing->owner)) {
          output_message(SMsg_CreatureRevealInfo, 0, 1);
          return 1;
        }
    }
    return 1;
}

long process_torture_function(struct Thing *thing)
{
    struct Room *room;
    long i;
    //return _DK_process_torture_function(thing);
    room = get_room_creature_works_in(thing);
    if ( !room_still_valid_as_type_for_thing(room,RoK_TORTURE,thing) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->owner);
        set_start_state(thing);
        return 1;
    }
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    anger_apply_anger_to_creature(thing, crstat->annoy_in_torture, 4, 1);
    if ((long)game.play_gameturn >= cctrl->field_82 + game.turns_per_torture_health_loss)
    {
        i = compute_creature_max_health(game.torture_health_loss,cctrl->explevel);
        remove_health_from_thing_and_display_health(thing, i);
        cctrl->field_82 = (long)game.play_gameturn;
    }
    if ((thing->health < 0) && (game.ghost_convert_chance > 0))
    {
        if (ACTION_RANDOM(game.ghost_convert_chance) == 0)
        {
            convert_creature_to_ghost(room, thing);
            return -1;
        }
    }
    if (room->owner == thing->owner)
        return 0;

    i = ((long)game.play_gameturn - cctrl->tortured.start_gameturn) * room->efficiency >> 8;

    if ((cctrl->spell_flags & CSAfF_Speed) != 0)
      i = (4 * i) / 3;
    if (cctrl->field_21 != 0)
      i = (5 * i) / 4;
    if ( (i < crstat->torture_time) || (cctrl->word_A6 == 0) )
        return 0;
    i = (long)game.play_gameturn - (long)crstat->torture_time - cctrl->tortured.start_gameturn;
    if (ACTION_RANDOM(100) >= (i/64 + 1))
        return 0;
    if (ACTION_RANDOM(3) == 0)
    {
        convert_tortured_creature_owner(thing, room->owner);
        return 1;
    }
    cctrl->tortured.start_gameturn = (long)game.play_gameturn - (long)crstat->torture_time / 2;
    reveal_players_map_to_player(thing, room->owner);
    return 0;
}

short torturing(struct Thing *thing)
{
  return _DK_torturing(thing);
}

/******************************************************************************/
