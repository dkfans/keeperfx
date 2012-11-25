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
#include "room_data.h"
#include "room_jobs.h"
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
        for (i = 0; i < PLAYERS_COUNT; i++)
        {
          if ( creature_can_get_to_dungeon(thing, i) )
          {
              SYNCDBG(18,"Returning enemy player %ld",i);
              return i;
          }
        }
    }
    SYNCDBG(18,"No enemy found");
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
      output_message(SMsg_EnemyDestroyRooms, 400, true);
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
        SYNCDBG(6,"No accessible player %ld treasure room found",dngn_id);
        return false;
    }
    if (!setup_person_move_to_position(thing, room->central_stl_x, room->central_stl_y, 0))
    {
        WARNLOG("Cannot setup move to player %ld treasure room",dngn_id);
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
        SYNCDBG(6,"No accessible player %ld library found",dngn_id);
        return false;
    }
    if (!setup_person_move_to_position(thing, room->central_stl_x, room->central_stl_y, 0))
    {
        WARNLOG("Cannot setup move to player %ld library",dngn_id);
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
        if (((thing->alloc_flags & TAlF_IsInLimbo) == 0) && ((thing->field_1 & TF1_InCtrldLimbo) == 0))
        {
            if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) ) {
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
    unsigned long k;
    long i;
    target_match = specific_target;
    // Find the target
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
        if (((thing->alloc_flags & TAlF_IsInLimbo) == 0) && ((thing->field_1 & TF1_InCtrldLimbo) == 0))
        {
            if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
            {
                // If it's not the one we want, continue sweeping
                if (target_match > 0)
                {
                    target_match--;
                } else
                // If it is the one, try moving to it
                if ( setup_person_move_to_position(wanderer, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
                {
                    return true;
                }
                // If we've got the right creature, but moving failed for some reason, try next one.
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
    return false;
}

TbBool setup_wanderer_move_to_random_creature_from_list(long first_thing_idx, struct Thing *wanderer)
{
    long navigable_targets,target_match;
    navigable_targets = get_wanderer_possible_targets_count_in_list(first_thing_idx,wanderer);
    // Select random target
    if (navigable_targets < 1) {
        return false;
    }
    target_match = ACTION_RANDOM(navigable_targets);
    if ( wander_to_specific_possible_target_in_list(first_thing_idx, wanderer, target_match) )
    {
        return true;
    }
    WARNLOG("Internal - couldn't wander to creature which was in list a while ago");
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
    SYNCDBG(4,"Cannot wander to player %d creatures",(int)dngn_id);
    return false;
}

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
    if (thing->owner != hero_player_number)
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
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        return false;
    }
    return _DK_good_back_at_start(thing);
}

TbBool good_setup_wander_to_dungeon_heart(struct Thing *thing, long plyr_idx)
{
    struct PlayerInfo *player;
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    if (thing->owner == plyr_idx)
    {
        ERRORLOG("The %s tried to wander to own (%d) heart", thing_model_name(thing), (int)plyr_idx);
        return false;
    }
    player = get_player(plyr_idx);
    if (!player_exists(player))
    {
        WARNLOG("The %s tried to wander to inactive player (%d) heart", thing_model_name(thing), (int)plyr_idx);
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
        WARNLOG("The %s tried to wander to player (%d) which has no heart", thing_model_name(thing), (int)plyr_idx);
        return false;
    }
    set_creature_object_combat(thing, heartng);
    return true;
}


short good_doing_nothing(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct PlayerInfo *player;
    long nturns;
    long i;
    //return _DK_good_doing_nothing(thing);
    SYNCDBG(18,"Starting");
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
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
        if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
            thing->continue_state = CrSt_GoodDoingNothing;
        return 1;
    }
    i = cctrl->sbyte_89;
    if (i != -1)
    {
      player = get_player(i);
      if (player_invalid(player))
      {
          ERRORLOG("Invalid target player in thing no %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
          cctrl->sbyte_89 = -1;
          return 0;
      }
      if (player->victory_state != VicS_LostLevel)
      {
          nturns = game.play_gameturn - cctrl->long_91;
          if (nturns <= 400)
          {
              if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
              {
                thing->continue_state = CrSt_GoodDoingNothing;
                return 0;
              }
          } else
          {
              if (!creature_can_get_to_dungeon(thing,i))
              {
                cctrl->sbyte_89 = -1;
              }
          }
      } else
      {
        cctrl->sbyte_89 = -1;
      }
    }
    i = cctrl->sbyte_89;
    if (i == -1)
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
          cctrl->sbyte_89 = good_find_enemy_dungeon(thing);
      }
      i = cctrl->sbyte_89;
      if (i == -1)
      {
          SYNCDBG(4,"No enemy dungeon to perform task");
          if ( creature_choose_random_destination_on_valid_adjacent_slab(thing) )
          {
              thing->continue_state = CrSt_GoodDoingNothing;
              return 1;
          }
          cctrl->field_5 = game.play_gameturn + 16;
      }
      return 1;
    }
    SYNCDBG(8,"Performing task %d",(int)cctrl->field_4);
    switch (cctrl->field_4)
    {
    case CHeroTsk_AttackRooms:
        if (good_setup_attack_rooms(thing, i)) {
            return 1;
        }
        WARNLOG("Can't attack player %d rooms, switching to attack heart", (int)i);
        cctrl->field_4 = CHeroTsk_AttackDnHeart;
        return false;
    case CHeroTsk_AttackDnHeart:
        if (good_setup_wander_to_dungeon_heart(thing, i)) {
            return 1;
        }
        ERRORLOG("Cannot wander to player %d heart", (int)i);
        return false;
    case CHeroTsk_StealGold:
        crstat = creature_stats_get_from_thing(thing);
        if (thing->creature.gold_carried < crstat->gold_hold)
        {
            if (good_setup_loot_treasure_room(thing, i)) {
                return true;
            }
            WARNLOG("Can't loot player %d treasury, switching to attack heart", (int)i);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        } else
        {
            if (good_setup_wander_to_exit(thing)) {
                return true;
            }
            WARNLOG("Can't wander to exit after looting player %d treasury, switching to attack heart", (int)i);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        }
        return false;
    case CHeroTsk_StealSpells:
        //TODO STEAL_SPELLS write a correct code for stealing spells, then enable this
        if (true)//!thing->holds_a_spell)
        {
            if (good_setup_loot_research_room(thing, i))
                return true;
            WARNLOG("Can't loot player %d spells, switching to attack heart", (int)i);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        } else
        {
            if (good_setup_wander_to_exit(thing))
                return true;
            WARNLOG("Can't wander to exit after looting player %d spells, switching to attack heart", (int)i);
            cctrl->field_4 = CHeroTsk_AttackDnHeart;
        }
        return false;
    case CHeroTsk_AttackEnemies:
    case CHeroTsk_Default:
    default:
        // Randomly select if we will first try to wander to creature, or to special digger
        if (ACTION_RANDOM(2) == 1)
        {
            // Try wander to creature
            if (good_setup_wander_to_creature(thing, cctrl->sbyte_89))
            {
                SYNCDBG(17,"Finished - wandering to creature");
                return true;
            }
            // If the wander failed, try wander to special digger
            if (good_setup_wander_to_spdigger(thing, cctrl->sbyte_89))
            {
                SYNCDBG(17,"Finished - wandering to worker");
                return true;
            }
        } else
        {
            // Try wander to special digger
            if (good_setup_wander_to_spdigger(thing, cctrl->sbyte_89))
            {
                SYNCDBG(17,"Finished - wandering to worker");
                return true;
            }
            // If the wander failed, try wander to creature
            if (good_setup_wander_to_creature(thing, cctrl->sbyte_89))
            {
                SYNCDBG(17,"Finished - wandering to creature");
                return true;
            }
        }
        WARNLOG("Can't attack player %d creature, switching to attack heart", (int)cctrl->sbyte_89);
        cctrl->field_4 = CHeroTsk_AttackDnHeart;
        return 0;
    }
}

short good_drops_gold(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
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
    if (thing->owner != hero_player_number)
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
    cctrl->byte_8A = tmptng->field_9;
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
    if (thing->owner != hero_player_number)
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
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
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
            if (cctrl->byte_8A == tmptng->field_9)
            {
              remove_thing_from_creature_controlled_limbo(thing);
              set_start_state(thing);
              return 1;
            }
        }
        thing->creature.gold_carried = 0;
        tmptng = thing_get(cctrl->field_6E);
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
        if (thing->owner != game.neutral_player_num)
        {
            set_start_state(thing);
            cctrl->field_282--;
            return 0;
        }
        initialise_thing_state(thing, CrSt_CreatureDormant);
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
    if ( internal_set_thing_state(creatng, 28) )
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
    cctrl->sbyte_89 = plyr_idx;
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
    struct Thing *heartng;
    heartng = INVALID_THING;
    {
        /* Sometimes we may have no target dungeon. In that case, destination dungeon
         * index is negative. This code will handle this case, as well as non-existing
         * dungeons.
         */
        struct Dungeon *dungeon;
        dungeon = get_dungeon(cctrl->sbyte_89);
        if (!dungeon_invalid(dungeon))
            heartng = thing_get(dungeon->dnheart_idx);
    }
    if (!thing_exists(heartng))
    {
        script_support_send_tunneller_to_appropriate_dungeon(creatng);
        return 0;
    }
    if ((heartng->active_state != 3) && creature_can_navigate_to(creatng, &heartng->mappos, 0))
    {
        internal_set_thing_state(creatng, CrSt_GoodDoingNothing);
        return 1;
    }
    cctrl->sbyte_89 = good_find_enemy_dungeon(creatng);
    if (cctrl->sbyte_89 != -1)
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
    if (slabmap_owner(slb) == cctrl->sbyte_89)
    {
        internal_set_thing_state(creatng, 34);
        return 1;
    }
    struct Coord3d *pos;
    long move_result;
    pos = &cctrl->moveto_pos;
    move_result = creature_tunnel_to(creatng, pos, speed);
    if (move_result == 1)
    {
        internal_set_thing_state(creatng, 77);
        return 1;
    }
    if (move_result == -1)
    {
        ERRORLOG("Bad place to tunnel to!");
        set_start_state(creatng);
        creatng->continue_state = 0;
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
