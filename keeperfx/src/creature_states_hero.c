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
#include "creature_states_hero.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
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
#include "player_utils.h"
#include "gui_soundmsgs.h"
#include "gui_topmsg.h"
#include "lvl_script.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_good_attack_room(struct Thing *creatng);
DLLIMPORT short _DK_good_back_at_start(struct Thing *creatng);
DLLIMPORT short _DK_good_doing_nothing(struct Thing *creatng);
DLLIMPORT short _DK_good_drops_gold(struct Thing *creatng);
DLLIMPORT short _DK_good_leave_through_exit_door(struct Thing *creatng);
DLLIMPORT short _DK_good_returns_to_start(struct Thing *creatng);
DLLIMPORT short _DK_good_wait_in_exit_door(struct Thing *creatng);
DLLIMPORT long _DK_good_setup_loot_treasure_room(struct Thing *creatng, long dngn_id);
DLLIMPORT short _DK_creature_hero_entering(struct Thing *creatng);
DLLIMPORT long _DK_get_best_dungeon_to_tunnel_to(struct Thing *creatng);
DLLIMPORT long _DK_creature_tunnel_to(struct Thing *creatng, struct Coord3d *pos, short a3);
DLLIMPORT short _DK_tunneller_doing_nothing(struct Thing *creatng);
DLLIMPORT short _DK_tunnelling(struct Thing *creatng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Return index of a dungeon which the hero may attack.
 * @todo HERO_AI Shouldn't we support allies with heroes?
 *
 * @param thing The hero searching for target.
 * @return Player index, or -1 if no dungeon to attack found.
 */
long good_find_enemy_dungeon(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long i;
    SYNCDBG(18,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->byte_8C != 0) || (cctrl->byte_8B != 0))
    {
        cctrl->byte_8C = 0;
        cctrl->byte_8B = 0;
        // Try accessing dungeon heart of undefeated enemy players
        for (i = 0; i < PLAYERS_COUNT; i++)
        {
            if (player_is_friendly_or_defeated(i, thing->owner)) {
                continue;
            }
            if (creature_can_get_to_dungeon(thing, i))
            {
                SYNCDBG(8,"The %s index %d can get to enemy player %d",thing_model_name(thing),(int)thing->index,(int)i);
                return i;
            }
        }
        // Try accessing any room of any non allied players
        for (i = 0; i < PLAYERS_COUNT; i++)
        {
            if (!players_are_enemies(thing->owner, i)) {
                continue;
            }
            if (creature_can_get_to_any_of_players_rooms(thing, i))
            {
                SYNCDBG(8,"The %s index %d can get to room of player %d",thing_model_name(thing),(int)thing->index,(int)i);
                return i;
            }
        }
    }
    SYNCDBG(8,"The %s index %d cannot find an enemy",thing_model_name(thing),(int)thing->index);
    return -1;
}

TbBool good_setup_wander_to_exit(struct Thing *creatng)
{
    struct Thing *gatetng;
    SYNCDBG(7,"Starting");
    gatetng = find_hero_door_hero_can_navigate_to(creatng);
    if (thing_is_invalid(gatetng))
    {
        SYNCLOG("Can't find any exit gate for hero %s.",thing_model_name(creatng));
        return false;
    }
    if (!setup_person_move_to_position(creatng, gatetng->mappos.x.stl.num, gatetng->mappos.y.stl.num, 0))
    {
        WARNLOG("Hero %s can't move to exit gate at (%d,%d).",thing_model_name(creatng),(int)gatetng->mappos.x.stl.num, (int)gatetng->mappos.y.stl.num);
        return false;
    }
    creatng->continue_state = CrSt_GoodLeaveThroughExitDoor;
    return true;
}

TbBool good_setup_attack_rooms(struct Thing *thing, long dngn_id)
{
    struct Room *room;
    struct CreatureControl *cctrl;
    struct Coord3d pos;
    room = find_nearest_room_for_thing_excluding_two_types(thing, dngn_id, 7, 1, 1);
    if (room_is_invalid(room))
    {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(thing, room, &pos)
      || !creature_can_navigate_to_with_storage(thing, &pos, 1) )
    {
        return false;
    }
    if (!setup_random_head_for_room(thing, room, 1))
    {
        ERRORLOG("setup random head for room failed");
        return false;
    }
    event_create_event_or_update_nearby_existing_event(
        get_subtile_center_pos(room->central_stl_x), get_subtile_center_pos(room->central_stl_y),
        EvKind_RoomUnderAttack, room->owner, 0);
    if (is_my_player_number(room->owner))
      output_message(SMsg_EnemyDestroyRooms, MESSAGE_DELAY_FIGHT, true);
    cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_GoodAttackRoom1;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool good_setup_loot_treasure_room(struct Thing *thing, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_good_setup_loot_treasure_room(thing, dngn_id);
    room = find_random_room_creature_can_navigate_to(thing, dngn_id, RoK_TREASURE, 0);
    if (room_is_invalid(room))
    {
        SYNCDBG(6,"No accessible player %d treasure room found",(int)dngn_id);
        return false;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        SYNCDBG(6,"No position for %s in %s owned by player %d",
            thing_model_name(thing),room_code_name(room->kind),(int)room->owner);
        return false;
    }
    if (!setup_person_move_to_position(thing, pos.x.stl.num, pos.y.stl.num, 0))
    {
        WARNLOG("Cannot setup move to player %d treasure room",(int)dngn_id);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_CreatureSearchForGoldToStealInRoom2;
    cctrl->target_room_id = room->index;
    return true;
}

TbBool good_setup_loot_research_room(struct Thing *thing, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    room = find_random_room_creature_can_navigate_to(thing, dngn_id, RoK_LIBRARY, 0);
    if (room_is_invalid(room))
    {
        SYNCDBG(6,"No accessible player %d library found",(int)dngn_id);
        return false;
    }
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        SYNCDBG(6,"No position for %s in %s owned by player %d",
            thing_model_name(thing),room_code_name(room->kind),(int)room->owner);
        return false;
    }
    if (!setup_person_move_to_position(thing, pos.x.stl.num, pos.y.stl.num, 0))
    {
        SYNCDBG(6,"Cannot setup move %s to %s owned by player %d",
            thing_model_name(thing),room_code_name(room->kind),(int)room->owner);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_CreatureSearchForSpellToStealInRoom;
    cctrl->target_room_id = room->index;
    return true;
}

long get_wanderer_possible_targets_count_in_list(long first_thing_idx, struct Thing *wanderer)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    long victims_count;
    unsigned long k;
    long i;
    victims_count = 0;
    // Get the amount of possible targets
    k = 0;
    i = first_thing_idx;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody_by_enemy(thing))
        {
            // Don't check for being navigable - it's too CPU-expensive to check all creatures
            //if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
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
    struct CreatureControl *cctrl;
    struct Thing *thing;
    long target_match;
    long matched_thing_idx;
    unsigned long k;
    long i;
    target_match = specific_target;
    // Find the target
    k = 0;
    i = first_thing_idx;
    matched_thing_idx = i;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody_by_enemy(thing))
        {
            // If it's not the one we want, continue sweeping
            if (target_match > 0)
            {
                target_match--;
                // Store the last unmatched thing, so we know where to stop when wrapped
                matched_thing_idx = thing->index;
            } else
            // If it is the one, try moving to it
            if ( setup_person_move_to_position(wanderer, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
            {
                SYNCDBG(8,"The %s wanders towards %s",thing_model_name(wanderer),thing_model_name(thing));
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
    long possible_targets,target_match;
    possible_targets = get_wanderer_possible_targets_count_in_list(first_thing_idx,wanderer);
    // Select random target
    if (possible_targets < 1) {
        SYNCDBG(4,"The %s cannot wander to creature, there are no targets",thing_model_name(wanderer));
        return false;
    }
    target_match = ACTION_RANDOM(possible_targets);
    if ( wander_to_specific_possible_target_in_list(first_thing_idx, wanderer, target_match) )
    {
        return true;
    }
    WARNLOG("The %s cannot wander to creature, it seem all %d creatures were not navigable",thing_model_name(wanderer),(int)possible_targets);
    return false;
}

TbBool good_setup_wander_to_creature(struct Thing *wanderer, long dngn_id)
{
    struct Dungeon *dungeon;
    SYNCDBG(7,"Starting");
    dungeon = get_dungeon(dngn_id);
    if ( setup_wanderer_move_to_random_creature_from_list(dungeon->creatr_list_start,wanderer) )
    {
        wanderer->continue_state = CrSt_GoodDoingNothing;
        return true;
    }
    SYNCDBG(4,"The %s cannot wander to player %d creatures",thing_model_name(wanderer), (int)dngn_id);
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
    struct Dungeon *dungeon;
    SYNCDBG(7,"Starting");
    dungeon = get_dungeon(dngn_id);
    if ( setup_wanderer_move_to_random_creature_from_list(dungeon->digger_list_start,wanderer) )
    {
        wanderer->continue_state = CrSt_GoodDoingNothing;
        return true;
    }
    SYNCDBG(4,"Cannot wander to player %d creatures",(int)dngn_id);
    return false;
}

short good_attack_room(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        return false;
    }
    return _DK_good_attack_room(thing);
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
    return _DK_good_back_at_start(thing);
}

TbBool good_can_move_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    player = get_player(plyr_idx);
    if (!player_exists(player))
    {
        SYNCDBG(3,"The %s cannot move to inactive player %d heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    struct Thing *heartng;
    heartng = INVALID_THING;
    {
        struct Dungeon *dungeon;
        dungeon = get_players_dungeon(player);
        if (!dungeon_invalid(dungeon))
            heartng = thing_get(dungeon->dnheart_idx);
    }
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng))
    {
        SYNCDBG(3,"The %s cannot move to player %d which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    return creature_can_navigate_to(creatng, &heartng->mappos, 0);
}

TbBool good_setup_wander_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    if (creatng->owner == plyr_idx)
    {
        ERRORLOG("The %s tried to wander to own (%d) heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    player = get_player(plyr_idx);
    if (!player_exists(player))
    {
        WARNLOG("The %s tried to wander to inactive player (%d) heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    struct Thing *heartng;
    heartng = INVALID_THING;
    {
        struct Dungeon *dungeon;
        dungeon = get_players_dungeon(player);
        if (!dungeon_invalid(dungeon))
            heartng = thing_get(dungeon->dnheart_idx);
    }
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng))
    {
        WARNLOG("The %s tried to wander to player (%d) which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    set_creature_object_combat(creatng, heartng);
    return true;
}

TbBool good_creature_setup_task_in_dungeon(struct Thing *creatng, PlayerNumber target_plyr_idx)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(8,"The %s performing task %d",thing_model_name(creatng), (int)cctrl->field_4);
    switch (cctrl->field_4)
    {
    case CHeroTsk_AttackRooms:
        if (good_setup_attack_rooms(creatng, target_plyr_idx)) {
            return true;
        }
        WARNLOG("Can't attack player %d rooms, switching to attack heart", (int)target_plyr_idx);
        cctrl->field_4 = CHeroTsk_AttackDnHeart;
        return false;
    case CHeroTsk_AttackDnHeart:
        if (good_setup_wander_to_dungeon_heart(creatng, target_plyr_idx)) {
            return true;
        }
        ERRORLOG("Cannot wander to player %d heart", (int)target_plyr_idx);
        return false;
    case CHeroTsk_StealGold:
        crstat = creature_stats_get_from_thing(creatng);
        if (creatng->creature.gold_carried < crstat->gold_hold)
        {
            if (good_setup_loot_treasure_room(creatng, target_plyr_idx)) {
                return true;
            }
            WARNLOG("Can't loot player %d treasury, switching to attack heart", (int)target_plyr_idx);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        } else
        {
            if (good_setup_wander_to_exit(creatng)) {
                return true;
            }
            WARNLOG("Can't wander to exit after looting player %d treasury, switching to attack heart", (int)target_plyr_idx);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        }
        return false;
    case CHeroTsk_StealSpells:
        if (!creature_is_dragging_spellbook(creatng))
        {
            if (good_setup_loot_research_room(creatng, target_plyr_idx)) {
                return true;
            }
            WARNLOG("Can't loot player %d spells, switching to attack heart", (int)target_plyr_idx);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        } else
        {
            if (good_setup_wander_to_exit(creatng)) {
                return true;
            }
            WARNLOG("Can't wander to exit after looting player %d spells, switching to attack heart", (int)target_plyr_idx);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        }
        return false;
    case CHeroTsk_AttackEnemies:
        // Randomly select if we will first try to wander to creature, or to special digger
        if (ACTION_RANDOM(2) == 1)
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
        WARNLOG("Can't attack player %d creature, switching to attack heart", (int)cctrl->party.target_plyr_idx);
        cctrl->field_4 = CHeroTsk_AttackDnHeart;
        return false;
    case CHeroTsk_Default:
    default:
        SYNCDBG(4,"Wrong task, switching to attack enemies");
        cctrl->field_4 = CHeroTsk_AttackEnemies;
        return false;
    }
}

short good_doing_nothing(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct PlayerInfo *player;
    long nturns;
    PlayerNumber target_plyr_idx;
    //return _DK_good_doing_nothing(thing);
    SYNCDBG(18,"Starting");
    // Debug code to find incorrect states
    if (!is_hero_thing(creatng))
    {
        ERRORLOG("Non hero %s index %d owned by player %d - reset",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        set_start_state(creatng);
        return 0;
    }
    cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return 0;
    }
    nturns = game.play_gameturn - cctrl->idle.start_gameturn;
    if (nturns <= 1) {
        return 1;
    }
    if (cctrl->field_5 > (long)game.play_gameturn)
    {
        if (creature_choose_random_destination_on_valid_adjacent_slab(creatng)) {
            creatng->continue_state = CrSt_GoodDoingNothing;
        }
        return 1;
    }
    target_plyr_idx = cctrl->party.target_plyr_idx;
    if (target_plyr_idx != -1)
    {
        player = get_player(target_plyr_idx);
        if (player_invalid(player))
        {
            ERRORLOG("Invalid target player in %s index %d owned by player %d - reset",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
            cctrl->party.target_plyr_idx = -1;
            return 0;
        }
        if (player->victory_state != VicS_LostLevel)
        {
            nturns = game.play_gameturn - cctrl->long_91;
            if (nturns > 400)
            {
                // Go to the previously chosen dungeon
                if (!creature_can_get_to_dungeon(creatng,target_plyr_idx))
                {
                    // Cannot get to the originally selected dungeon - reset it
                    cctrl->party.target_plyr_idx = -1;
                }
            } else
            {
                // Waiting - move around a bit
                if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
                {
                    creatng->continue_state = CrSt_GoodDoingNothing;
                    return 0;
                }
            }
        } else
        {
            // The player we've chosen has lost - we'll have to find other target
            cctrl->party.target_plyr_idx = -1;
        }
    }
    target_plyr_idx = cctrl->party.target_plyr_idx;
    if (target_plyr_idx == -1)
    {
        nturns = game.play_gameturn - cctrl->long_91;
        if (nturns > 400)
        {
            cctrl->long_91 = game.play_gameturn;
            cctrl->byte_8C = 1;
        }
        nturns = game.play_gameturn - cctrl->long_8D;
        if (nturns > 64)
        {
            cctrl->long_8D = game.play_gameturn;
            cctrl->party.target_plyr_idx = good_find_enemy_dungeon(creatng);
        }
        target_plyr_idx = cctrl->party.target_plyr_idx;
        if (target_plyr_idx == -1)
        {
            SYNCDBG(4,"No enemy dungeon to perform %s index %d task",
                thing_model_name(creatng),(int)creatng->index);
            if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
            {
                creatng->continue_state = CrSt_GoodDoingNothing;
                return 1;
            }
            cctrl->field_5 = game.play_gameturn + 16;
        }
        return 1;
    }
    if (good_creature_setup_task_in_dungeon(creatng, target_plyr_idx))
        return 1;
    // If there are problems with the task, do a break before re-trying
    cctrl->field_5 = game.play_gameturn + 200;
    return 0;
}

short good_drops_gold(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    return _DK_good_drops_gold(thing);
}

short good_leave_through_exit_door(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *tmptng;
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    //return _DK_good_leave_through_exit_door(thing);
    tmptng = find_base_thing_on_mapwho(TCls_Object, 49, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(tmptng))
    {
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->creature.gold_carried = 0;
    cctrl->field_282 = game.hero_door_wait_time;
    cctrl->byte_8A = tmptng->creation_turn;
    place_thing_in_creature_controlled_limbo(thing);
    internal_set_thing_state(thing, CrSt_GoodWaitInExitDoor);
    return 1;
}

short good_returns_to_start(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct Thing *heartng;
    // Debug code to find incorrect states
    SYNCDBG(7,"Starting");
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    //return _DK_good_returns_to_start(thing);
    dungeon = get_dungeon(thing->owner);
    heartng = INVALID_THING;
    if (!dungeon_invalid(dungeon))
        heartng = thing_get(dungeon->dnheart_idx);
    TRACE_THING(heartng);
    //TODO CREATURE_AI Heroes don't usually have hearts; maybe they should also go back to hero gates?
    if ( !setup_person_move_to_position(thing, heartng->mappos.x.stl.num, heartng->mappos.y.stl.num, 0) )
    {
        return 0;
    }
    thing->continue_state = CrSt_GoodBackAtStart;
    return 1;
}

short good_wait_in_exit_door(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *tmptng;
    // Debug code to find incorrect states
    if (!is_hero_thing(thing))
    {
        ERRORLOG("Non hero thing %s index %d, owner %d - reset",
            thing_model_name(thing), (int)thing->index, (int)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    //return _DK_good_wait_in_exit_door(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->field_282 <= 0)
        return 0;
    cctrl->field_282--;
    if (cctrl->field_282 == 0)
    {
        tmptng = find_base_thing_on_mapwho(TCls_Object, 49, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (!thing_is_invalid(tmptng))
        {
            if (cctrl->byte_8A == tmptng->creation_turn)
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
        kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
    }
    return 0;
}

short creature_hero_entering(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    TRACE_THING(thing);
    //return _DK_creature_hero_entering(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->field_282 > 0)
    {
        cctrl->field_282--;
        return 0;
    }
    if (cctrl->field_282 == 0)
    {
        thing->mappos.z.val = get_ceiling_height(&thing->mappos) - (long)thing->field_58 - 1;
        cctrl->field_282--;
        return 0;
    }
    if ( thing_touching_floor(thing) || ((thing->movement_flags & TMvF_Flying) != 0) )
    {
        if (is_neutral_thing(thing))
        {
            initialise_thing_state(thing, CrSt_CreatureDormant);
            cctrl->field_282--;
            return 0;
        }
        set_start_state(thing);
    }
    cctrl->field_282--;
    return 0;
}

long get_best_dungeon_to_tunnel_to(struct Thing *creatng)
{
    return _DK_get_best_dungeon_to_tunnel_to(creatng);
}

short setup_person_tunnel_to_position(struct Thing *creatng, long stl_x, long stl_y, unsigned char a4)
{
    struct CreatureControl *cctrl;
    if ( internal_set_thing_state(creatng, CrSt_Tunnelling) )
    {
        cctrl = creature_control_get_from_thing(creatng);
        cctrl->moveto_pos.x.stl.num = stl_x;
        cctrl->moveto_pos.y.stl.num = stl_y;
        cctrl->moveto_pos.x.stl.pos = 128;
        cctrl->moveto_pos.y.stl.pos = 128;
        cctrl->moveto_pos.z.val = get_thing_height_at(creatng, &cctrl->moveto_pos);
    }
    return 0;
}

long send_tunneller_to_point_in_dungeon(struct Thing *creatng, long plyr_idx, struct Coord3d *pos)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->party.target_plyr_idx = plyr_idx;
    setup_person_tunnel_to_position(creatng, pos->x.stl.num, pos->y.stl.num, 0);
    creatng->continue_state = CrSt_TunnellerDoingNothing;
    return 1;
}

short tunneller_doing_nothing(struct Thing *creatng)
{
    //return _DK_tunneller_doing_nothing(creatng);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (game.play_gameturn - cctrl->long_9A <= 1)
    {
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
    cctrl->party.target_plyr_idx = good_find_enemy_dungeon(creatng);
    if (cctrl->party.target_plyr_idx != -1)
    {
        internal_set_thing_state(creatng, CrSt_GoodDoingNothing);
        return 1;
    }

    int plyr_idx;
    plyr_idx = get_best_dungeon_to_tunnel_to(creatng);
    if ( plyr_idx == -1 )
      return 1;
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if ( dungeon->num_active_creatrs || dungeon->num_active_diggers )
    {
        struct Coord3d pos;
        get_random_position_in_dungeon_for_creature(plyr_idx, 1, creatng, &pos);
        send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos);
    } else
    {
        good_setup_wander_to_dungeon_heart(creatng, plyr_idx);
    }
    return 1;
}

long creature_tunnel_to(struct Thing *creatng, struct Coord3d *pos, short a3)
{
    return _DK_creature_tunnel_to(creatng, pos, a3);
}

short tunnelling(struct Thing *creatng)
{
    struct SlabMap *slb;
    long speed;
    //return _DK_tunnelling(creatng);
    speed = get_creature_speed(creatng);
    slb = get_slabmap_for_subtile(creatng->mappos.x.stl.num,creatng->mappos.y.stl.num);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (slabmap_owner(slb) == cctrl->party.target_plyr_idx)
    {
        internal_set_thing_state(creatng, CrSt_GoodDoingNothing);
        return 1;
    }
    struct Coord3d *pos;
    long move_result;
    pos = &cctrl->moveto_pos;
    move_result = creature_tunnel_to(creatng, pos, speed);
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
    if (((game.play_gameturn + creatng->index) & 0x7F) != 0)
    {
        return 0;
    }
    if (!creature_can_navigate_to(creatng, pos, 0))
    {
        return 0;
    }
    return 1;
}
/******************************************************************************/
