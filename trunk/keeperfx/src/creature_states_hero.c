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
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "gui_soundmsgs.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_good_attack_room(struct Thing *thing);
DLLIMPORT short _DK_good_back_at_start(struct Thing *thing);
DLLIMPORT short _DK_good_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_good_drops_gold(struct Thing *thing);
DLLIMPORT short _DK_good_leave_through_exit_door(struct Thing *thing);
DLLIMPORT short _DK_good_returns_to_start(struct Thing *thing);
DLLIMPORT short _DK_good_wait_in_exit_door(struct Thing *thing);
DLLIMPORT long _DK_good_setup_loot_treasure_room(struct Thing *thing, long dngn_id);
DLLIMPORT short _DK_creature_hero_entering(struct Thing *thing);
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

TbBool good_setup_wander_to_exit(struct Thing *thing)
{
    struct Thing *gatetng;
    SYNCDBG(7,"Starting");
    gatetng = find_hero_door_hero_can_navigate_to(thing);
    if (thing_is_invalid(gatetng))
    {
        SYNCLOG("Can't find any exit gate for hero %s.",thing_model_name(thing));
        return false;
    }
    if (!setup_person_move_to_position(thing, gatetng->mappos.x.stl.num, gatetng->mappos.y.stl.num, 0))
    {
        WARNLOG("Hero %s can't move to exit gate at (%d,%d).",thing_model_name(thing),(int)gatetng->mappos.x.stl.num, (int)gatetng->mappos.y.stl.num);
        return false;
    }
    thing->continue_state = CrSt_GoodLeaveThroughExitDoor;
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
        get_subtile_center_pos(room->stl_x), get_subtile_center_pos(room->stl_y),
        19, room->owner, 0);
    if (is_my_player_number(room->owner))
      output_message(15, 400, 1);
    cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_GoodAttackRoom1;
    cctrl->field_80 = room->index;
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
    if (!setup_person_move_to_position(thing, room->stl_x, room->stl_y, 0))
    {
        WARNLOG("Cannot setup move to player %ld treasure room",dngn_id);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_CreatureSearchForGoldToStealInRoom2;
    cctrl->field_80 = room->index;
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
    if (!setup_person_move_to_position(thing, room->stl_x, room->stl_y, 0))
    {
        WARNLOG("Cannot setup move to player %ld library",dngn_id);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->continue_state = CrSt_CreatureSearchForSpellToStealInRoom;
    cctrl->field_80 = room->index;
    return true;
}

TbBool good_setup_wander_to_creature(struct Thing *wanderer, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *thing;
    long navigable_targets,target_match;
    unsigned long k;
    long i;
    SYNCDBG(7,"Starting");
    cctrl = creature_control_get_from_thing(wanderer);
    dungeon = get_dungeon(dngn_id);
    navigable_targets = 0;
    // Get the amount of possible targets
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature detected");
        break;
      }
      i = cctrl->thing_idx;
      // Thing list loop body
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
            navigable_targets++;
      }
      // Thing list loop body ends
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
    }
    // Select random target
    if (navigable_targets < 1)
    {
        SYNCDBG(4,"No player %d creatures found to wander to",(int)dngn_id);
        return false;
    }
    target_match = ACTION_RANDOM(navigable_targets);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature detected");
        break;
      }
      i = cctrl->thing_idx;
      // Thing list loop body
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
          {
              if (target_match > 0)
              {
                  target_match--;
              } else
              if ( setup_person_move_to_position(wanderer, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
              {
                  thing->continue_state = CrSt_GoodDoingNothing;
                  return true;
              }
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
    WARNLOG("Internal - couldn't wander to player %d creature",(int)dngn_id);
    return false;
}

TbBool good_setup_wander_to_imp(struct Thing *wanderer, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *thing;
    long navigable_targets,target_match;
    unsigned long k;
    long i;
    SYNCDBG(7,"Starting");
    cctrl = creature_control_get_from_thing(wanderer);
    dungeon = get_dungeon(dngn_id);
    navigable_targets = 0;
    // Get the amount of possible targets
    k = 0;
    i = dungeon->worker_list_start;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature detected");
        break;
      }
      i = cctrl->thing_idx;
      // Thing list loop body
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
            navigable_targets++;
      }
      // Thing list loop body ends
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
    }
    // Select random target
    if (navigable_targets < 1)
    {
        SYNCDBG(4,"No player %d creatures found to wander to",(int)dngn_id);
        return false;
    }
    target_match = ACTION_RANDOM(navigable_targets);
    k = 0;
    i = dungeon->worker_list_start;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature detected");
        break;
      }
      i = cctrl->thing_idx;
      // Thing list loop body
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
          {
              if (target_match > 0)
              {
                  target_match--;
              } else
              if ( setup_person_move_to_position(wanderer, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
              {
                  thing->continue_state = CrSt_GoodDoingNothing;
                  return true;
              }
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
    WARNLOG("Internal - couldn't wander to player %d creature",(int)dngn_id);
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

TbBool good_setup_wander_to_dungeon_heart(struct Thing *thing, long dngn_idx)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *heartng;
    dungeon = get_dungeon(dngn_idx);
    if (dungeon_invalid(dungeon) || (thing->owner == dngn_idx))
    {
        ERRORLOG("The %s tried to wander to invalid player (%d) heart", thing_model_name(thing), (int)dngn_idx);
        return false;
    }
    player = get_player(dngn_idx);
    if ((!player_exists(player)) || (dungeon->dnheart_idx < 1))
    {
        WARNLOG("The %s tried to wander to inactive player (%d) heart", thing_model_name(thing), (int)dngn_idx);
        return false;
    }
    heartng = thing_get(dungeon->dnheart_idx);
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
    nturns = game.play_gameturn - cctrl->long_9A;
    if (nturns <= 1)
      return 1;
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
    case 1: // ATTACK_ROOMS
        if (good_setup_attack_rooms(thing, i))
        {
            return 1;
        }
        WARNLOG("Can't attack player %d rooms, switching to attack heart", (int)i);
        cctrl->field_4 = 3;
        return false;
    case 3: // ATTACK_DUNGEON_HEART
        if (good_setup_wander_to_dungeon_heart(thing, i))
        {
            return 1;
        }
        ERRORLOG("Cannot wander to player %d heart", (int)i);
        return false;
    case 4: // STEAL_GOLD
        crstat = creature_stats_get_from_thing(thing);
        if (thing->long_13 < crstat->gold_hold)
        {
            if (good_setup_loot_treasure_room(thing, i))
                return true;
            WARNLOG("Can't loot player %d treasury, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        } else
        {
            if (good_setup_wander_to_exit(thing))
                return true;
            WARNLOG("Can't wander to exit after looting player %d treasury, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        }
        return false;
    case 5: // STEAL_SPELLS
        //TODO STEAL_SPELLS write a correct code for stealing spells, then enable this
        if (true)//!thing->holds_a_spell)
        {
            if (good_setup_loot_research_room(thing, i))
                return true;
            WARNLOG("Can't loot player %d spells, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        } else
        {
            if (good_setup_wander_to_exit(thing))
                return true;
            WARNLOG("Can't wander to exit after looting player %d spells, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        }
        return false;
    case 2: // ATTACK_ENEMIES
    case 0:
    default:
        if (ACTION_RANDOM(2) == 1)
        {
          if (good_setup_wander_to_creature(thing, cctrl->sbyte_89))
          {
              SYNCDBG(17,"Finished - wandering to creature");
              return true;
          }
        }
        if (good_setup_wander_to_imp(thing, cctrl->sbyte_89))
        {
            SYNCDBG(17,"Finished - wandering to worker");
            return true;
        }
        WARNLOG("Can't attack player %d creature, switching to attack heart", (int)cctrl->sbyte_89);
        cctrl->field_4 = 3;
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
    tmptng = find_base_thing_on_mapwho(1, 49, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(tmptng))
    {
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->long_13 = 0;
    cctrl->field_282 = game.hero_door_wait_time;
    cctrl->byte_8A = tmptng->field_9;
    place_thing_in_creature_controlled_limbo(thing);
    internal_set_thing_state(thing, CrSt_GoodWaitInExitDoor);
    return 1;
}

short good_returns_to_start(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    return _DK_good_returns_to_start(thing);
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
        tmptng = find_base_thing_on_mapwho(1, 49, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (!thing_is_invalid(tmptng))
        {
            if (cctrl->byte_8A == tmptng->field_9)
            {
              remove_thing_from_creature_controlled_limbo(thing);
              set_start_state(thing);
              return 1;
            }
        }
        thing->long_13 = 0;
        tmptng = thing_get(cctrl->field_6E);
        if (!thing_is_invalid(tmptng))
        {
            delete_thing_structure(tmptng, 0);
        }
        kill_creature(thing, 0, -1, 1, 0, 0);
    }
    return 0;
}

short creature_hero_entering(struct Thing *thing)
{
  return _DK_creature_hero_entering(thing);
}

/******************************************************************************/
