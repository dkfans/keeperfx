/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_hero.c
 *     Creature state machine functions for heroes.
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
#include "creature_states_hero.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_planar.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "creature_states_spdig.h"
#include "creature_states_combt.h"
#include "creature_jobs.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "power_hand.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_list.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "player_utils.h"
#include "gui_soundmsgs.h"
#include "gui_topmsg.h"
#include "game_legacy.h"
#include "map_locations.h"
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
 * Return index of a dungeon which the hero may attack.
 * @todo CREATURE_AI Shouldn't we support allies with heroes?
 *
 * @param thing The hero searching for target.
 * @return Player index, or -1 if no dungeon to attack found.
 */
TbBool has_available_enemy_dungeon_heart(struct Thing *thing, PlayerNumber plyr_idx)
{
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->hero.ready_for_attack_flag != 0) || (cctrl->hero.hero_state_reset_flag != 0))
    {
        cctrl->hero.ready_for_attack_flag = 0;
        cctrl->hero.hero_state_reset_flag = 0;
    }
    // Try accessing dungeon heart of undefeated enemy players
    if (!player_is_friendly_or_defeated(plyr_idx, thing->owner) && (creature_can_get_to_dungeon_heart(thing, plyr_idx)))
    {
        return true;
    }
    return false;
}

long good_find_best_enemy_dungeon(struct Thing* creatng)
{
    PlayerNumber best_plyr_idx = -1;
    PlayerNumber backup_plyr_idx = -1;
    struct PlayerInfo* player;
    struct Dungeon* dungeon;
    long best_score = INT32_MIN;
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (player_is_friendly_or_defeated(plyr_idx, creatng->owner)) {
            continue;
        }
        player = get_player(plyr_idx);
        if (flag_is_set(game.conf.rules[creatng->owner].game.classic_bugs_flags,ClscBug_AlwaysTunnelToRed))
        {
            if (creature_can_get_to_dungeon_heart(creatng, plyr_idx))
            {
                return plyr_idx;
            }
        }
        else
        {

            dungeon = get_players_dungeon(player);
            long score;
            if (player_exists(player) && !dungeon_invalid(dungeon) && (creatng->owner != plyr_idx))
            {
                score = dungeon->total_score;
                if (score <= 0)
                {
                    score = 0;
                }
                if (has_available_enemy_dungeon_heart(creatng, plyr_idx))
                {
                    if (best_score < score)
                    {
                        best_score = score;
                        best_plyr_idx = plyr_idx;
                    }
                }
            }
        }
    }
    if (best_plyr_idx == -1)
    {
        best_plyr_idx = backup_plyr_idx;
    }
    return best_plyr_idx;
}

/**
 * Checks if given hero has money that should be placed in treasure room.
 * If he does, he is ordered to return them into nearest treasure room
 * which has the proper capacity.
 * @param thing The hero.
 * @return Gives 1 if the hero was ordered to go into treasure room, 0 otherwise.
 */
long check_out_hero_has_money_for_treasure_room(struct Thing* thing)
{
    struct Room* room;
    SYNCDBG(8, "Starting for %s index %d", thing_model_name(thing), (int)thing->index);
    //If the hero doesn't have any money - then just return
    if (thing->creature.gold_carried <= 0) {
        return 0;
    }
    // Find a treasure room to drop the money
    room = find_nearest_room_of_role_for_thing_with_spare_capacity(thing, thing->owner, RoRoF_GoldStorage, NavRtF_Default, 1);
    if (room_is_invalid(room))
    {
        return 0;
    }
    if (setup_head_for_empty_treasure_space(thing, room))
    {
        thing->continue_state = CrSt_ImpDropsGold; //todo: when more is rewritten, see if there are other states possible here.
        return 1;
    }
    return 0;
}

TbBool good_setup_wander_to_exit(struct Thing *creatng)
{
    SYNCDBG(7,"Starting");
    if (creature_is_dragging_spellbook(creatng))
    {
        struct Coord3d pos;
        struct Room* dstroom = find_nearest_room_of_role_for_thing_with_spare_capacity(creatng, creatng->owner, RoRoF_PowersStorage, NavRtF_Default, 1);
        if (!(room_is_invalid(dstroom)) && find_random_valid_position_for_thing_in_room_avoiding_object(creatng, dstroom, &pos))
        {
            SYNCLOG("Can't find a library for hero %s index %d to place stolen spellbook", thing_model_name(creatng), (int)creatng->index);
            if (dstroom == get_room_thing_is_on(creatng))
            {
                if (dstroom->owner == creatng->owner)
                {
                    creature_drops_spell_object_in_library(creatng);
                    return true;
                }
                else
                {
                    if (creature_drop_thing_to_another_room(creatng, dstroom, RoRoF_PowersStorage))
                    {
                        return true;
                    }
                }
            }
            else
            {
                if (setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
                {
                    return true;
                }
            }
        }
        else
        {
            if (creature_drop_thing_to_another_room(creatng, dstroom, RoRoF_PowersStorage))
            {
                return true;
            }
        }
    }

    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->party.objective == CHeroTsk_StealGold) && (creatng->creature.gold_carried > 0))
    {
        if (check_out_hero_has_money_for_treasure_room(creatng))
        {
            return true;
        }
    }

    struct Thing* gatetng = find_best_hero_gate_to_navigate_to(creatng);
    if (thing_is_invalid(gatetng))
    {
        SYNCLOG("Can't find any exit gate for hero %s index %d",thing_model_name(creatng),(int)creatng->index);
        return false;
    }
    if (!setup_person_move_to_coord(creatng, &gatetng->mappos, NavRtF_Default))
    {
        WARNLOG("Hero %s index %d can't move to exit gate %d at (%d,%d).",thing_model_name(creatng),(int)creatng->index,
            (int)gatetng->index, (int)gatetng->mappos.x.stl.num, (int)gatetng->mappos.y.stl.num);
        return false;
    }
    creatng->continue_state = CrSt_GoodLeaveThroughExitDoor;
    return true;
}

TbBool good_setup_attack_rooms(struct Thing *creatng, long dngn_id)
{
    struct Room* room = find_nearest_room_to_vandalise(creatng, dngn_id, NavRtF_NoOwner);
    if (room_is_invalid(room))
    {
        return false;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos) || !creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_NoOwner))
    {
        ERRORLOG("The %s index %d cannot destroy %s because it cannot reach position within it",thing_model_name(creatng),(int)creatng->index,room_code_name(room->kind));
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_NoOwner))
    {
        ERRORLOG("The %s index %d cannot destroy %s because it cannot head for it",thing_model_name(creatng),(int)creatng->index,room_code_name(room->kind));
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = CrSt_GoodArrivedAtAttackRoom;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool good_setup_sabotage_rooms(struct Thing* creatng, short dngn_id)
{
    struct Room* room = find_nearest_room_to_vandalise(creatng, dngn_id, NavRtF_NoOwner);
    if (room_is_invalid(room))
    {
        return false;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos) || !creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_NoOwner))
    {
        ERRORLOG("The %s index %d cannot destroy %s because it cannot reach position within it", thing_model_name(creatng), (int)creatng->index, room_code_name(room->kind));
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_NoOwner))
    {
        ERRORLOG("The %s index %d cannot destroy %s because it cannot head for it", thing_model_name(creatng), (int)creatng->index, room_code_name(room->kind));
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = CrSt_GoodArrivedAtSabotageRoom;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool good_setup_defend_rooms(struct Thing* creatng)
{
    struct Room* room = find_nearest_room_to_vandalise(creatng, creatng->owner, NavRtF_Default);
    if (room_is_invalid(room))
    {
        return false;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos) || !creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_Default))
    {
        ERRORLOG("The %s index %d cannot defend %s because it cannot reach position within it", thing_model_name(creatng), (int)creatng->index, room_code_name(room->kind));
        return false;
    }
    if (!setup_random_head_for_room(creatng, room, NavRtF_Default))
    {
        ERRORLOG("The %s index %d cannot defend %s because it cannot head for it", thing_model_name(creatng), (int)creatng->index, room_code_name(room->kind));
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creatng->continue_state = CrSt_PatrolHere;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool good_setup_loot_treasure_room(struct Thing *thing, long dngn_id)
{
    struct Room* room = find_random_room_of_role_with_used_capacity_creature_can_navigate_to(thing, dngn_id, RoRoF_GoldStorage, NavRtF_Default);
    if (room_is_invalid(room))
    {
        SYNCDBG(6,"No accessible player %d treasure room found",(int)dngn_id);
        return false;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        SYNCDBG(6,"No position for %s index %d in %s owned by player %d",
            thing_model_name(thing),(int)thing->index,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    if (!setup_person_move_to_coord(thing, &pos, NavRtF_Default))
    {
        WARNLOG("Cannot setup move to player %d treasure room",(int)dngn_id);
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_CreatureSearchForGoldToStealInRoom2;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool good_setup_loot_research_room(struct Thing *thing, long dngn_id)
{
    struct Room* room = find_random_room_of_role_with_used_capacity_creature_can_navigate_to(thing, dngn_id, RoRoF_PowersStorage, NavRtF_Default);
    if (room_is_invalid(room))
    {
        SYNCDBG(6,"No accessible player %d library found",(int)dngn_id);
        return false;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        SYNCDBG(6,"No position for %s index %d in %s owned by player %d",
            thing_model_name(thing),(int)thing->index,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    if (!setup_person_move_to_coord(thing, &pos, NavRtF_Default))
    {
        SYNCDBG(6,"Cannot setup move %s index %d to %s owned by player %d",
            thing_model_name(thing),(int)thing->index,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_CreatureSearchForSpellToStealInRoom;
    cctrl->target_room_id = room->index;
    return true;
}

long get_wanderer_possible_targets_count_in_list(long first_thing_idx, struct Thing *wanderer)
{
    long victims_count = 0;
    // Get the amount of possible targets
    unsigned long k = 0;
    long i = first_thing_idx;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody_by_enemy(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            // Don't check for being navigable - it's too CPU-expensive to check all creatures
            //if ( creature_can_navigate_to(wanderer, &thing->mappos, NavTF_Default) )
            {
                victims_count++;
            }
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    return victims_count;
}

TbBool wander_to_specific_possible_target_in_list(long first_thing_idx, struct Thing *wanderer, long specific_target)
{
    long target_match = specific_target;
    // Find the target
    unsigned long k = 0;
    long i = first_thing_idx;
    long matched_thing_idx = i;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody_by_enemy(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            // If it's not the one we want, continue sweeping
            if (target_match > 0)
            {
                target_match--;
                // Store the last unmatched thing, so we know where to stop when wrapped
                matched_thing_idx = thing->index;
            } else
            // If it is the one, try moving to it
            if (setup_person_move_to_coord(wanderer, &thing->mappos, NavRtF_Default))
            {
                SYNCDBG(8,"The %s index %d wanders towards %s index %d",thing_model_name(wanderer),(int)wanderer->index,thing_model_name(thing),(int)thing->index);
                return true;
            }
            // If we've got the right creature, but moving failed for some reason, try next one.
        }
        // Wrap to first thing if reached end of list.
        if (i == 0) {
            i = first_thing_idx;
            if (target_match != 0)
                WARNLOG("Wrapping to start of the list shouldn't occur before target_match reaches 0!");
        }
        // When wrapped, process things only to the start index
        if (i == matched_thing_idx)
            break;
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    return false;
}

/**
 * Setups a wanderer creature to move to a random thing in given list.
 * @param first_thing_idx
 * @param wanderer
 * @return
 */
TbBool setup_wanderer_move_to_random_creature_from_list(long first_thing_idx, struct Thing *wanderer)
{
    long possible_targets = get_wanderer_possible_targets_count_in_list(first_thing_idx, wanderer);
    // Select random target
    if (possible_targets < 1) {
        SYNCDBG(4,"The %s index %d cannot wander to creature, there are no targets",thing_model_name(wanderer),(int)wanderer->index);
        return false;
    }
    long target_match = THING_RANDOM(wanderer, possible_targets);
    if ( wander_to_specific_possible_target_in_list(first_thing_idx, wanderer, target_match) )
    {
        return true;
    }
    WARNLOG("The %s index %d cannot wander to creature, it seem all %d creatures were not navigable",
        thing_model_name(wanderer),(int)wanderer->index,(int)possible_targets);
    return false;
}

TbBool good_setup_wander_to_creature(struct Thing *wanderer, long dngn_id)
{
    SYNCDBG(7,"Starting");
    struct Dungeon* dungeon = get_dungeon(dngn_id);
    if ( setup_wanderer_move_to_random_creature_from_list(dungeon->creatr_list_start,wanderer) )
    {
        wanderer->continue_state = CrSt_GoodWanderToCreatureCombat;
        return true;
    }
    SYNCDBG(4,"The %s index %d cannot wander to player %d creatures",thing_model_name(wanderer),
        (int)wanderer->index,(int)dngn_id);
    return false;
}

/**
 * Wander the given creature to a random digger belonging to given player.
 * Originally was good_setup_wander_to_imp.
 * @param wanderer
 * @param dngn_id
 * @return
 */
TbBool good_setup_wander_to_spdigger(struct Thing *wanderer, long dngn_id)
{
    SYNCDBG(7,"Starting");
    struct Dungeon* dungeon = get_dungeon(dngn_id);
    if ( setup_wanderer_move_to_random_creature_from_list(dungeon->digger_list_start,wanderer) )
    {
        wanderer->continue_state = CrSt_GoodWanderToCreatureCombat;
        return true;
    }
    SYNCDBG(4,"Cannot wander to player %d creatures",(int)dngn_id);
    return false;
}

short good_arrived_at_attack_room(struct Thing *thing)
{
    struct Room* room = get_room_thing_is_on(thing);
    // If the current tile can be destroyed
    if (room_exists(room) && !players_creatures_tolerate_each_other(thing->owner, room->owner) && !room_cannot_vandalise(room->kind))
    {
        internal_set_thing_state(thing, CrSt_GoodAttackRoom1);
        MapCoord ev_coord_x = subtile_coord_center(room->central_stl_x);
        MapCoord ev_coord_y = subtile_coord_center(room->central_stl_y);
        event_create_event_or_update_nearby_existing_event(ev_coord_x, ev_coord_y, EvKind_RoomUnderAttack, room->owner, 0);
        if (is_my_player_number(room->owner))
          output_message(SMsg_EnemyDestroyRooms, MESSAGE_DURATION_FIGHT);
        return 1;
    }
    set_start_state(thing);
    return 0;
}

short good_attack_room(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero %s index %d owner %d - reset",thing_model_name(thing),(int)thing->index,(int)thing->owner);
        set_start_state(thing);
        return 0;
    }
    MapSlabCoord base_slb_x = subtile_slab(thing->mappos.x.stl.num);
    MapSlabCoord base_slb_y = subtile_slab(thing->mappos.y.stl.num);
    struct Room* room = slab_room_get(base_slb_x, base_slb_y);
    // If the current tile can be destroyed
    if (room_exists(room) && !players_creatures_tolerate_each_other(thing->owner, room->owner) && !room_cannot_vandalise(room->kind))
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (cctrl->instance_id == CrInst_NULL)
        {
            set_creature_instance(thing, CrInst_ATTACK_ROOM_SLAB, 0, 0);
            MapCoord ev_coord_x = subtile_coord_center(room->central_stl_x);
            MapCoord ev_coord_y = subtile_coord_center(room->central_stl_y);
            event_create_event_or_update_nearby_existing_event(ev_coord_x, ev_coord_y, EvKind_RoomUnderAttack, room->owner, 0);
            if (is_my_player_number(room->owner))
                output_message(SMsg_EnemyDestroyRooms, MESSAGE_DURATION_FIGHT);
        }
        return 1;
    }
    // Otherwise, search around for a tile to destroy
    long m = THING_RANDOM(thing, SMALL_AROUND_SLAB_LENGTH);
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        MapSlabCoord slb_x = base_slb_x + (long)small_around[m].delta_x;
        MapSlabCoord slb_y = base_slb_y + (long)small_around[m].delta_y;
        room = slab_room_get(slb_x, slb_y);
        if (room_exists(room) && !players_creatures_tolerate_each_other(thing->owner, room->owner) && !room_cannot_vandalise(room->kind))
        {
            if (setup_person_move_to_position(thing, slb_x, slb_y, NavRtF_Default))
            {
                thing->continue_state = CrSt_GoodAttackRoom1;
                return 1;
            }
        }
        m = (m+1) % SMALL_AROUND_SLAB_LENGTH;
    }
    set_start_state(thing);
    return 0;
}

short good_back_at_start(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        return false;
    }
    if (thing->creature.gold_carried <= 0)
    {
        set_start_state(thing);
        return 1;
    }
    SubtlCodedCoords stl_num = get_subtile_number(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    long m = THING_RANDOM(thing, AROUND_MAP_LENGTH);
    for (long n = 0; n < AROUND_MAP_LENGTH; n++)
    {
        struct Map* mapblk = get_map_block_at_pos(stl_num + game.around_map[m]);
        // Per-block code
        if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
        {
            MapSubtlCoord stl_x = stl_num_decode_x(stl_num + game.around_map[m]);
            MapSubtlCoord stl_y = stl_num_decode_y(stl_num + game.around_map[m]);
            if (setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default)) {
                thing->continue_state = CrSt_GoodDropsGold;
                return 1;
            }
        }
        // Per-block code ends
        m = (m + 1) % AROUND_MAP_LENGTH;
    }
    set_start_state(thing);
    return 1;

}

TbBool good_can_move_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(creatng);
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player))
    {
        SYNCDBG(3,"The %s index %d cannot move to inactive player %d heart", thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (!thing_exists(heartng))
    {
        SYNCDBG(3,"The %s index %d cannot move to player %d which has no heart", thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    return creature_can_navigate_to(creatng, &heartng->mappos, NavRtF_Default);
}

short good_arrived_at_attack_dungeon_heart(struct Thing* creatng)
{
    creatng->continue_state = CrSt_GoodDoingNothing;
    if (creature_look_for_enemy_heart_combat(creatng))
    {
        return CrStRet_Modified;
    }
    return move_to_position(creatng);
}

short good_arrived_at_combat(struct Thing* creatng)
{
    creatng->continue_state = CrSt_GoodDoingNothing;
    if (creature_look_for_combat(creatng))
    {
        return CrStRet_Modified;
    }
    return move_to_position(creatng);
}

TbBool good_setup_wander_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(creatng);
    if (creatng->owner == plyr_idx)
    {
        ERRORLOG("The %s index %d tried to wander to own (%d) heart", thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player))
    {
        WARNLOG("The %s index %d tried to wander to inactive player (%d) heart", thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (!thing_exists(heartng))
    {
        WARNLOG("The %s index %d tried to wander to player %d which has no heart", thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    if (!setup_person_move_to_coord(creatng, &heartng->mappos, NavRtF_Default))
    {
        WARNLOG("Hero %s index %d can't move to heart %d at (%d,%d).", thing_model_name(creatng), (int)creatng->index,
            (int)heartng->index, (int)heartng->mappos.x.stl.num, (int)heartng->mappos.y.stl.num);
        return false;
    }
    creatng->continue_state = CrSt_GoodWanderToObjectCombat;
    return true;
}

TbBool good_setup_rush_to_dungeon_heart(struct Thing* creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(18, "Starting");
    TRACE_THING(creatng);
    if (creatng->owner == plyr_idx)
    {
        ERRORLOG("The %s index %d tried to wander to own (%d) heart", thing_model_name(creatng), (int)creatng->index, (int)plyr_idx);
        return false;
    }
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player))
    {
        WARNLOG("The %s index %d tried to wander to inactive player (%d) heart", thing_model_name(creatng), (int)creatng->index, (int)plyr_idx);
        return false;
    }
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (!thing_exists(heartng))
    {
        WARNLOG("The %s index %d tried to wander to player %d which has no heart", thing_model_name(creatng), (int)creatng->index, (int)plyr_idx);
        return false;
    }
    set_creature_object_snipe(creatng, heartng);
    return true;
}

TbBool good_setup_wander_to_own_heart(struct Thing* creatng)
{
    SYNCDBG(7, "Starting");
    struct Thing* heartng = get_player_soul_container(creatng->owner);
    TRACE_THING(heartng);
    if (!thing_exists(heartng))
    {
        WARNLOG("The %s index %d tried to wander to player %d which has no heart", thing_model_name(creatng), (int)creatng->index, creatng->owner);
        return false;
    }

    if (!setup_person_move_to_coord(creatng, &heartng->mappos, NavRtF_Default))
    {
        WARNLOG("Hero %s index %d can't move to heart %d at (%d,%d).", thing_model_name(creatng), (int)creatng->index,
            (int)heartng->index, (int)heartng->mappos.x.stl.num, (int)heartng->mappos.y.stl.num);
        return false;
    }
    creatng->continue_state = CrSt_PatrolHere;
    return true;
}

TbBool good_creature_setup_task_in_dungeon(struct Thing *creatng, PlayerNumber target_plyr_idx)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(8,"The %s index %d performing task %d",thing_model_name(creatng),(int)creatng->index,(int)cctrl->party.objective);
    switch (cctrl->party.objective)
    {
    case CHeroTsk_AttackRooms:
        if (good_setup_attack_rooms(creatng, target_plyr_idx)) {
            return true;
        }
        SYNCDBG(8,"Can't attack player %d rooms, switching to attack heart", (int)target_plyr_idx);
        cctrl->party.objective = CHeroTsk_AttackDnHeart;
        return false;
    case CHeroTsk_SabotageRooms:
        if (good_setup_sabotage_rooms(creatng, target_plyr_idx)) {
            return true;
        }
        SYNCDBG(8,"Can't attack player %d rooms, switching to attack heart", (int)target_plyr_idx);
        cctrl->party.objective = CHeroTsk_AttackDnHeart;
        return false;
    case CHeroTsk_AttackDnHeart:
        if (good_setup_wander_to_dungeon_heart(creatng, target_plyr_idx))
        {
            return true;
        }
        SYNCDBG(8,"Cannot wander to player %d heart", (int)target_plyr_idx);
        return false;
    case CHeroTsk_SnipeDnHeart:
        if (good_setup_rush_to_dungeon_heart(creatng, target_plyr_idx))
        {
            return true;
        }
        SYNCDBG(8,"Cannot rush to player %d heart", (int)target_plyr_idx);
        return false;
    case CHeroTsk_StealGold:
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
        if (creatng->creature.gold_carried < crconf->gold_hold)
        {
            if (good_setup_loot_treasure_room(creatng, target_plyr_idx)) {
                return true;
            }
            SYNCDBG(8,"Can't loot player %d treasury, switching to attack heart", (int)target_plyr_idx);
            cctrl->party.objective = CHeroTsk_AttackDnHeart;
        } else
        {
            if (good_setup_wander_to_exit(creatng)) {
                return true;
            }
            SYNCDBG(8, "Can't wander to exit after looting player %d treasury, switching to attack heart", (int)target_plyr_idx);
            cctrl->party.objective = CHeroTsk_AttackDnHeart;
        }
        return false;
    }
    case CHeroTsk_StealSpells:
        if (!creature_is_dragging_spellbook(creatng))
        {
            if (good_setup_loot_research_room(creatng, target_plyr_idx)) {
                return true;
            }
            SYNCDBG(8,"Can't loot player %d spells, switching to attack heart", (int)target_plyr_idx);
            cctrl->party.objective = CHeroTsk_AttackDnHeart;
        } else
        {
            if (good_setup_wander_to_exit(creatng)) {
                return true;
            }
            SYNCDBG(8,"Can't wander to exit after looting player %d spells, switching to attack heart", (int)target_plyr_idx);
            cctrl->party.objective = CHeroTsk_AttackDnHeart;
        }
        return false;
    case CHeroTsk_AttackEnemies:
        // Randomly select if we will first try to wander to creature, or to special digger
        if (THING_RANDOM(creatng, 2) == 1)
        {
            // Try wander to creature
            if (good_setup_wander_to_creature(creatng, cctrl->party.target_plyr_idx))
            {
                SYNCDBG(17,"Finished - wandering to player %d creature", (int)cctrl->party.target_plyr_idx);
                return true;
            }
            // If the wander failed, try wander to special digger
            if (good_setup_wander_to_spdigger(creatng, cctrl->party.target_plyr_idx))
            {
                SYNCDBG(17,"Finished - wandering to player %d worker", (int)cctrl->party.target_plyr_idx);
                return true;
            }
        } else
        {
            // Try wander to special digger
            if (good_setup_wander_to_spdigger(creatng, cctrl->party.target_plyr_idx))
            {
                SYNCDBG(17,"Finished - wandering to player %d worker", (int)cctrl->party.target_plyr_idx);
                return true;
            }
            // If the wander failed, try wander to creature
            if (good_setup_wander_to_creature(creatng, cctrl->party.target_plyr_idx))
            {
                SYNCDBG(17,"Finished - wandering to player %d creature", (int)cctrl->party.target_plyr_idx);
                return true;
            }
        }
        SYNCDBG(8,"Can't attack player %d creature, switching to attack heart", (int)cctrl->party.target_plyr_idx);
        cctrl->party.objective = CHeroTsk_AttackDnHeart;
        return false;
    case CHeroTsk_DefendSpawn:
        if (patrol_here(creatng))
        {
            return true;
        }
        if (patrolling(creatng))
        {
            return true;
        }
        SYNCDBG(8,"Can't patrol location, switching to defending rooms");
        cctrl->party.objective = CHeroTsk_DefendRooms;
        return false;
    case CHeroTsk_DefendHeart:
        if (good_setup_wander_to_own_heart(creatng))
        {
            return true;
        }
        SYNCDBG(8,"Can't defend own heart, switching to attack player %d heart", (int)cctrl->party.target_plyr_idx);
        cctrl->party.objective = CHeroTsk_AttackEnemies;
        return false;
    case CHeroTsk_DefendRooms:
        if (good_setup_defend_rooms(creatng))
        {
            return true;
        }
        SYNCDBG(8,"Can't defend rooms, switching to defending heart");
        cctrl->party.objective = CHeroTsk_AttackEnemies;
        return false;
    case CHeroTsk_Default:
    default:
        SYNCDBG(4,"Wrong task, switching to attack enemies");
        cctrl->party.objective = CHeroTsk_AttackEnemies;
        return false;
    }
}

short good_doing_nothing(struct Thing *creatng)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(creatng);
    // Debug code to find incorrect states
    if (!is_hero_thing(creatng))
    {
        ERRORLOG("Non hero %s index %d owned by player %d - reset",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        set_start_state(creatng);
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return 0;
    }
    // Respect the idle time - just wander around some time
    long nturns = game.play_gameturn - cctrl->idle.start_gameturn;
    if (nturns <= 1) {
        return 1;
    }
    // Do some wandering also if can't find any task to do
    if ((long)cctrl->wait_to_turn > (long)game.play_gameturn)
    {
        if (creature_choose_random_destination_on_valid_adjacent_slab(creatng)) {
            creatng->continue_state = CrSt_GoodDoingNothing;
        }
        return 1;
    }
    // Done wandering - if we had job assigned, get back to it
    if ((cctrl->job_assigned != Job_NULL) && (game.play_gameturn - cctrl->job_assigned_check_turn > 128))
    {
        if (attempt_job_preference(creatng, cctrl->job_assigned)) {
            SYNCDBG(8,"The %s index %d will do assigned job with state %s",thing_model_name(creatng),
                (int)creatng->index,creature_state_code_name(get_creature_state_besides_interruptions(creatng)));
            return 1;
        }
        cctrl->job_assigned_check_turn = game.play_gameturn;
    }
    // Now go the standard hero path - find a target player
    PlayerNumber target_plyr_idx = cctrl->party.target_plyr_idx;
    if (target_plyr_idx != -1)
    {
        struct PlayerInfo* player = get_player(target_plyr_idx);
        if (player_invalid(player))
        {
            ERRORLOG("Invalid target player in %s index %d owned by player %d - reset",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
            cctrl->party.target_plyr_idx = -1;
            return 0;
        }
        if ((player->victory_state != VicS_LostLevel) && players_are_enemies(creatng->owner, target_plyr_idx))
        {
            nturns = game.play_gameturn - cctrl->hero.wait_time;
            if (nturns > 400)
            {
                // Go to the previously chosen dungeon
                if (!creature_can_get_to_dungeon_heart(creatng,target_plyr_idx))
                {
                    if (!creature_can_get_to_any_of_players_rooms(creatng, target_plyr_idx) || (cctrl->party.objective != CHeroTsk_AttackRooms))
                    {
                        // Cannot get to the originally selected dungeon - reset it
                        cctrl->party.target_plyr_idx = -1;
                    }
                }
            } else
            if (nturns >= 0)
            {
                // Waiting - move around a bit
                if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
                {
                    creatng->continue_state = CrSt_GoodDoingNothing;
                    return 0;
                }
            } else
            {
                // Value lower than 0 would mean it is invalid
                WARNLOG("Invalid wait time detected for %s index %d, value %ld",thing_model_name(creatng),(int)creatng->index,(long)cctrl->hero.wait_time);
                cctrl->hero.wait_time = 0;
            }
        } else
        {
            // The player we've chosen has lost or is not an enemy - we'll have to find other target
            cctrl->party.target_plyr_idx = -1;
        }
    }
    if (cctrl->party.objective > CHeroTsk_DefendParty) // Defensive objectives don't need a target
    {
        if (good_creature_setup_task_in_dungeon(creatng, target_plyr_idx)) {
            return 1;
        }
    }
    else
    {
        target_plyr_idx = cctrl->party.target_plyr_idx;
    }
    if (target_plyr_idx == -1)
    {
        nturns = game.play_gameturn - cctrl->hero.wait_time;
        if ((nturns > 400) && (cctrl->hero.look_for_enemy_dungeon_turn != 0))
        {
            cctrl->hero.wait_time = game.play_gameturn;
            cctrl->hero.ready_for_attack_flag = 1;
        }
        nturns = game.play_gameturn - cctrl->hero.look_for_enemy_dungeon_turn;
        if (nturns > 64)
        {
            cctrl->hero.look_for_enemy_dungeon_turn = game.play_gameturn;
            cctrl->party.target_plyr_idx = good_find_best_enemy_dungeon(creatng);
        }
        target_plyr_idx = cctrl->party.target_plyr_idx;
        if (target_plyr_idx == -1)
        {
            SYNCDBG(4,"No enemy dungeon to perform %s index %d task",
                thing_model_name(creatng),(int)creatng->index);
            if (cctrl->party.original_objective > CHeroTsk_Default)
            {
                cctrl->party.objective = cctrl->party.original_objective;
            }
            cctrl->wait_to_turn = game.play_gameturn + 16;
            if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
            {
                creatng->continue_state = CrSt_GoodDoingNothing;
                return 1;
            }
        }
        return 1;
    }
    if (good_creature_setup_task_in_dungeon(creatng, target_plyr_idx)) {
        return 1;
    }
    // If there are problems with the task, do a break before re-trying
    cctrl->wait_to_turn = game.play_gameturn + 200;
    return 0;
}

short good_drops_gold(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero %s index %d, owner %d - reset",thing_model_name(thing),(int)thing->index,(int)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    GoldAmount amount = game.conf.rules[thing->owner].game.pot_of_gold_holds;
    if (thing->creature.gold_carried <= game.conf.rules[thing->owner].game.pot_of_gold_holds)
        amount = thing->creature.gold_carried;
    struct Thing* gldtng = create_object(&thing->mappos, ObjMdl_GoldPot, thing->owner, -1);
    if (thing_is_invalid(gldtng)) {
        return 0;
    }
    gldtng->valuable.gold_stored = amount;
    thing->creature.gold_carried -= amount;
    // Update size of the gold object
    add_gold_to_pile(gldtng, 0);
    internal_set_thing_state(thing, CrSt_GoodBackAtStart);
    return 1;
}

short good_leave_through_exit_door(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero %s index %d, owner %d - reset",thing_model_name(thing),(int)thing->index,(int)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    struct Thing* tmptng = find_object_of_genre_on_mapwho(OCtg_HeroGate, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(tmptng))
    {
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    thing->creature.gold_carried = 0;
    cctrl->countdown = game.conf.rules[thing->owner].game.hero_door_wait_time;
    cctrl->hero.hero_gate_creation_turn = tmptng->creation_turn;
    struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
    if (cctrl->dragtng_idx != 0)
    {
        creature_drop_dragged_object(thing, dragtng);
        destroy_object(dragtng);
    }
    place_thing_in_creature_controlled_limbo(thing);
    internal_set_thing_state(thing, CrSt_GoodWaitInExitDoor);
    return 1;
}

short good_returns_to_start(struct Thing *thing)
{
    // Debug code to find incorrect states
    SYNCDBG(7,"Starting");
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero %s index %d, owner %d - reset",thing_model_name(thing),(int)thing->index,(int)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    struct Thing* heartng = get_player_soul_container(thing->owner);
    TRACE_THING(heartng);
    //TODO CREATURE_AI Heroes don't usually have hearts; maybe they should also go back to hero gates, or any room?
    if (!setup_person_move_to_coord(thing, &heartng->mappos, NavRtF_Default))
    {
        return 0;
    }
    thing->continue_state = CrSt_GoodBackAtStart;
    return 1;
}

short good_wait_in_exit_door(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero thing %s index %d, owner %d - reset",
            thing_model_name(thing), (int)thing->index, (int)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->countdown <= 0)
        return 0;
    cctrl->countdown--;
    if (cctrl->countdown == 0)
    {
        struct Thing* tmptng = find_object_of_genre_on_mapwho(OCtg_HeroGate, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (!thing_is_invalid(tmptng))
        {
            if (cctrl->hero.hero_gate_creation_turn == tmptng->creation_turn)
            {
                remove_thing_from_creature_controlled_limbo(thing);
                set_start_state(thing);
                return 1;
            }
        }
        thing->creature.gold_carried = 0;
        tmptng = thing_get(cctrl->dragtng_idx);
        TRACE_THING(tmptng);
        if (!thing_is_invalid(tmptng))
        {
            delete_thing_structure(tmptng, 0);
        }
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects|CrDed_NotReallyDying);
    }
    return 0;
}

short creature_hero_entering(struct Thing *thing)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->countdown > 0)
    {
        cctrl->countdown--;
        return CrStRet_Unchanged;
    }
    if (cctrl->countdown == 0)
    {
        thing->mappos.z.val = get_ceiling_height(&thing->mappos) - (long)thing->clipbox_size_z - 1;
        cctrl->countdown--;
        return CrStRet_Modified;
    }
    if ( thing_touching_floor(thing) || (((thing->movement_flags & TMvF_Flying) != 0) && thing_touching_flight_altitude(thing)))
    {
        set_start_state(thing);
        return CrStRet_ResetOk;
    }
    if (cctrl->countdown < -500)
    {
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    cctrl->countdown--;
    return CrStRet_Modified;
}

long get_best_dungeon_to_tunnel_to(struct Thing *creatng)
{
    PlayerNumber best_plyr_idx = -1;
    long best_score = INT32_MIN;
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        struct Dungeon* dungeon = get_players_dungeon(player);
        if (player_exists(player) && !dungeon_invalid(dungeon) && players_are_enemies(creatng->owner,plyr_idx))
        {
            long score = dungeon->total_score; //Original code: = dungeon->total_score -20 * dungeon->total_score * dungeon->field_F7D / 100;
            if ((score <= 0) || (game.conf.rules[creatng->owner].game.classic_bugs_flags & ClscBug_AlwaysTunnelToRed))
            {
                score = 0;
            }
            if (best_score < score)
            {
                best_score = score;
                best_plyr_idx = plyr_idx;
            }
        }
    }
    return best_plyr_idx;
}

short setup_person_tunnel_to_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char unusedparam)
{
    if ( internal_set_thing_state(creatng, CrSt_Tunnelling) )
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        cctrl->moveto_pos.x.val = subtile_coord_center(stl_x);
        cctrl->moveto_pos.y.val = subtile_coord_center(stl_y);
        cctrl->moveto_pos.z.val = get_thing_height_at(creatng, &cctrl->moveto_pos);
    }
    return 0;
}

long send_tunneller_to_point(struct Thing *thing, struct Coord3d *pos)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->party.target_plyr_idx = -1;
    setup_person_tunnel_to_position(thing, pos->x.stl.num, pos->y.stl.num, 0);
    thing->continue_state = CrSt_TunnellerDoingNothing;
    return 1;
}

TbBool script_support_send_tunneller_to_action_point(struct Thing *thing, long apt_idx)
{
    SYNCDBG(7,"Starting");
    struct ActionPoint* apt = action_point_get(apt_idx);
    struct Coord3d pos;
    if (action_point_exists(apt)) {
        pos.x.val = apt->mappos.x.val;
        pos.y.val = apt->mappos.y.val;
    } else {
        ERRORLOG("Attempt to send to non-existing action point %d",(int)apt_idx);
        pos.x.val = subtile_coord_center(game.map_subtiles_x/2);
        pos.y.val = subtile_coord_center(game.map_subtiles_y/2);
    }
    pos.z.val = subtile_coord(1,0);
    send_tunneller_to_point(thing, &pos);
    return true;
}

TbBool script_support_send_tunneller_to_dungeon(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Send %s to player %d",thing_model_name(creatng),(int)plyr_idx);
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (!thing_exists(heartng))
    {
        WARNLOG("Tried to send %s to player %d which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    struct Coord3d pos;
    if (!get_random_position_in_dungeon_for_creature(plyr_idx, CrWaS_WithinDungeon, creatng, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't find position", thing_model_name(creatng), (int)plyr_idx);
        return send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &heartng->mappos);
    }
    if (!send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't start the task", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    SYNCDBG(17,"Moving %s to (%d,%d)",thing_model_name(creatng),(int)pos.x.stl.num,(int)pos.y.stl.num);
    return true;
}

TbBool script_support_send_tunneller_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Send %s to player %d",thing_model_name(creatng),(int)plyr_idx);
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (!thing_exists(heartng)) {
        WARNLOG("Tried to send %s to player %d which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    if (!send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &heartng->mappos)) {
        WARNLOG("Tried to send %s to player %d but can't start the task", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    SYNCDBG(17,"Moving %s to (%d,%d)",thing_model_name(creatng),(int)heartng->mappos.x.stl.num,(int)heartng->mappos.y.stl.num);
    return true;
}

TbBool script_support_send_tunneller_to_appropriate_dungeon(struct Thing *creatng)
{
    SYNCDBG(7,"Starting");
    PlayerNumber plyr_idx;
    struct Coord3d pos;
    plyr_idx = get_best_dungeon_to_tunnel_to(creatng);
    if (plyr_idx == -1) {
        ERRORLOG("Could not find appropriate dungeon to send %s to",thing_model_name(creatng));
        return false;
    }
    if (!get_random_position_in_dungeon_for_creature(plyr_idx, CrWaS_WithinDungeon, creatng, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't find position", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    return send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos);
}

struct Thing *script_process_new_tunneler(unsigned char plyr_idx, TbMapLocation location, TbMapLocation heading, CrtrExpLevel exp_level, unsigned long carried_gold)
{
    ThingModel diggerkind = get_players_special_digger_model(plyr_idx);
    struct Thing* creatng = script_create_creature_at_location(plyr_idx, diggerkind, location, SpwnT_Default);
    if (thing_is_invalid(creatng))
        return INVALID_THING;
    creatng->creature.gold_carried = carried_gold;
    init_creature_level(creatng, exp_level);
    switch (get_map_location_type(heading))
    {
    case MLoc_ACTIONPOINT:
        script_support_send_tunneller_to_action_point(creatng, get_map_location_longval(heading));
        break;
    case MLoc_PLAYERSDUNGEON:
        script_support_send_tunneller_to_dungeon(creatng, get_map_location_longval(heading));
        break;
    case MLoc_PLAYERSHEART:
        script_support_send_tunneller_to_dungeon_heart(creatng, get_map_location_longval(heading));
        break;
    case MLoc_APPROPRTDUNGEON:
        script_support_send_tunneller_to_appropriate_dungeon(creatng);
        break;
    default:
        ERRORLOG("Invalid Heading objective %d",(int)get_map_location_type(heading));
        break;
    }
    return creatng;
}

TbBool send_tunneller_to_point_in_dungeon(struct Thing *creatng, PlayerNumber plyr_idx, struct Coord3d *pos)
{
    SYNCDBG(17,"Move %s index %d to (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)pos->x.stl.num,(int)pos->y.stl.num);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->party.target_plyr_idx = plyr_idx;
    setup_person_tunnel_to_position(creatng, pos->x.stl.num, pos->y.stl.num, 0);
    creatng->continue_state = CrSt_TunnellerDoingNothing;
    return true;
}

short tunneller_doing_nothing(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    PlayerNumber CurrentTarget = cctrl->party.target_plyr_idx;
    // Wait for some time
    if (game.play_gameturn - cctrl->idle.start_gameturn <= 1) {
        return 1;
    }
    /* Sometimes we may have no target dungeon. In that case, destination dungeon
     * index is negative. */
    if (cctrl->party.target_plyr_idx == -1)
    {
        script_support_send_tunneller_to_appropriate_dungeon(creatng);
        return 0;
    }
    if (!player_cannot_win(cctrl->party.target_plyr_idx))
    {
        if (good_can_move_to_dungeon_heart(creatng, cctrl->party.target_plyr_idx))
        {
            internal_set_thing_state(creatng, CrSt_GoodDoingNothing);
            return 1;
        }
    }
    cctrl->party.target_plyr_idx = good_find_best_enemy_dungeon(creatng);
    if ( (cctrl->party.target_plyr_idx != -1) && (cctrl->party.target_plyr_idx != CurrentTarget) )
    {
        internal_set_thing_state(creatng, CrSt_GoodDoingNothing);
        return 1;
    }

    int plyr_idx = get_best_dungeon_to_tunnel_to(creatng);
    if (CurrentTarget != -1)
    {
        plyr_idx = CurrentTarget;
    }
    if (plyr_idx == -1) {
      return 1;
    }
    struct PlayerInfo* player = get_player(plyr_idx);
    if ( (player_exists(player)) && (player_has_heart(plyr_idx)) )
    {
        struct Dungeon* dungeon = get_dungeon(plyr_idx);
        struct Coord3d pos;
        if ((dungeon->num_active_creatrs > 0) || (dungeon->num_active_diggers > 0))
        {
            get_random_position_in_dungeon_for_creature(plyr_idx, CrWaS_WithinDungeon, creatng, &pos);
            send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos);
        } else
        {
            struct Thing* heartng = get_player_soul_container(plyr_idx);
            if (creature_can_navigate_to_with_storage(creatng, &heartng->mappos, NavRtF_Default))
            {
                good_setup_wander_to_dungeon_heart(creatng, plyr_idx);
            }
            else
            {
                get_random_position_in_dungeon_for_creature(plyr_idx, CrWaS_WithinDungeon, creatng, &pos);
                send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos);
            }
        }
    }
    return 1;
}

long creature_tunnel_to(struct Thing *creatng, struct Coord3d *pos, short speed)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(6,"Move %s index %d from (%d,%d) to (%d,%d) with speed %d",thing_model_name(creatng),(int)creatng->index,(int)creatng->mappos.x.stl.num,(int)creatng->mappos.y.stl.num,(int)pos->x.stl.num,(int)pos->y.stl.num,(int)speed);
    cctrl->navi.owner_flags[0] = 0;
    if (get_chessboard_distance(&creatng->mappos, pos) <= 32)
    {
        // We've reached the destination
        creature_set_speed(creatng, 0);
        return 1;
    }
    long i = cctrl->party.tunnel_steps_counter;
    if ((i > 0) && (i < INT32_MAX))
    {
        cctrl->party.tunnel_steps_counter++;
    }
    if ((pos->x.val != cctrl->navi.pos_final.x.val)
     || (pos->y.val != cctrl->navi.pos_final.y.val)
     || (pos->z.val != cctrl->navi.pos_final.z.val))
    {
        pos->z.val = get_thing_height_at(creatng, pos);
        initialise_wallhugging_path_from_to(&cctrl->navi, &creatng->mappos, pos);
    }
    long tnlret = get_next_position_and_angle_required_to_tunnel_creature_to(creatng, pos, cctrl->party.tunnel_dig_direction);
    if (tnlret == 2)
    {
        i = cctrl->navi.first_colliding_block;
        if (cctrl->navi.second_colliding_block != i)
        {
            cctrl->navi.second_colliding_block = i;
        } else
        if (cctrl->instance_id == CrInst_NULL)
        {
            MapSubtlCoord stl_x = stl_num_decode_x(cctrl->navi.first_colliding_block);
            MapSubtlCoord stl_y = stl_num_decode_y(cctrl->navi.first_colliding_block);
            struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
            if ( (slabmap_owner(slb) == creatng->owner) || (slb->kind == SlbT_EARTH || (slb->kind == SlbT_TORCHDIRT)) ) { // if this is false, that means the current tile must have changed to an undiggable wall
                set_creature_instance(creatng, CrInst_TUNNEL, 0, 0);
            }
        else {
            return 1;
        }
        }
    }
    MapCoordDelta dist = get_2d_distance(&creatng->mappos, &cctrl->navi.pos_next);
    if (dist <= 16)
    {
        creature_turn_to_face_angle(creatng, cctrl->navi.angle);
        creature_set_speed(creatng, 0);
        return 0;
    }
    if (dist > 768)
    {
        if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
        {
            creatng->continue_state = CrSt_TunnellerDoingNothing;
        }
        ERRORLOG("Move %s index %d to (%d,%d) reset - wallhug distance %d too large",thing_model_name(creatng),(int)creatng->index,(int)pos->x.stl.num,(int)pos->y.stl.num,(int)dist);
        return 0;
    }
    // If the tunneler tries to tunnel the same distance for 150 times, he must be stuck. So push him.
    static struct TunnelDistance tunnel;
    tunnel.creatid = creatng->index;
    tunnel.newdist = dist;
    static unsigned long identical[CREATURES_COUNT];
    if (tunnel.olddist == tunnel.newdist)
    {
        identical[creatng->ccontrol_idx] += 1;
    }
    else
    {
        tunnel.olddist = tunnel.newdist;
        identical[creatng->ccontrol_idx] = 0;
    }
    if ( identical[creatng->ccontrol_idx] >= 150)
    {
        if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
        {
            creatng->continue_state = CrSt_TunnellerDoingNothing;
            ERRORLOG("%s index %d stuck - attempt %lu to dislodge",thing_model_name(creatng),(int)creatng->index,identical[creatng->ccontrol_idx]-149);
        }
        return 0;
    }
    if (creature_turn_to_face(creatng, &cctrl->navi.pos_next) > 0)
    {
        creature_set_speed(creatng, 0);
        return 0;
    }
    cctrl->moveaccel.x.val = cctrl->navi.pos_next.x.val - (MapCoordDelta)creatng->mappos.x.val;
    cctrl->moveaccel.y.val = cctrl->navi.pos_next.y.val - (MapCoordDelta)creatng->mappos.y.val;
    cctrl->moveaccel.z.val = 0;
    cctrl->creature_state_flags |= TF2_CreatureIsMoving;
    creature_set_speed(creatng, min(speed,dist));
    return 0;
}

short tunnelling(struct Thing *creatng)
{
    SYNCDBG(7,"Move %s index %d from (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)creatng->mappos.x.stl.num,(int)creatng->mappos.y.stl.num);
    long speed = get_creature_speed(creatng);
    struct SlabMap* slb = get_slabmap_for_subtile(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Coord3d* pos = &cctrl->moveto_pos;
    if (slabmap_owner(slb) == cctrl->party.target_plyr_idx)
    {
        internal_set_thing_state(creatng, CrSt_GoodDoingNothing);
        return 1;
    }
    long move_result = creature_tunnel_to(creatng, pos, speed);
    if (move_result == 1)
    {
        internal_set_thing_state(creatng, CrSt_TunnellerDoingNothing);
        return 1;
    }
    if (move_result == -1)
    {
        ERRORLOG("Bad place to tunnel to!");
        set_start_state(creatng);
        creatng->continue_state = CrSt_Unused;
        return 0;
    }
    // Once per 128 turns, check if we've done digging and can now walk to the place
    if (((game.play_gameturn + creatng->index) & 0x7F) == 0)
    {
        if (creature_can_navigate_to(creatng, pos, NavRtF_Default))
        {
            SYNCDBG(7,"The %s index %d can now walk to (%d,%d), no need to tunnel",thing_model_name(creatng),(int)creatng->index,(int)pos->x.stl.num,(int)pos->y.stl.num);
            return 1;
        }
    }
    SYNCDBG(7,"The %s index %d cannot reach (%d,%d) by walk",thing_model_name(creatng),(int)creatng->index,(int)pos->x.stl.num,(int)pos->y.stl.num);
    return 0;
}

/**
 * Returns if given creature is a hero tunneller currently digging to attack a player.
 * @param creatng The creature to be checked.
 * @return Gives true if the creature is hero tunneller at tunnelling, false otherwise.
 */
TbBool is_hero_tunnelling_to_attack(struct Thing *creatng)
{
    if (creatng->model != get_players_special_digger_model(creatng->owner))
        return false;
    CrtrStateId crstate = get_creature_state_besides_move(creatng);
    if ((crstate != CrSt_Tunnelling) && (crstate != CrSt_TunnellerDoingNothing))
        return false;
    return true;
}
/******************************************************************************/
