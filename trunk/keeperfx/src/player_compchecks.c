/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_compchecks.c
 *     Computer player checks definitions and routines.
 * @par Purpose:
 *     Defines a computer player checks and related functions.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 06 Jan 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_computer.h"

#include <limits.h>
#include <string.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"

#include "config.h"
#include "player_instances.h"
#include "creature_states.h"
#include "magic.h"
#include "dungeon_data.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);

/******************************************************************************/
/******************************************************************************/
long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);

/******************************************************************************/
const struct NamedCommand computer_check_func_type[] = {
  {"checks_hates",            1,},
  {"check_move_to_best_room", 2,},
  {"check_move_to_room",      3,},
  {"check_no_imps",           4,},
  {"check_for_pretty",        5,},
  {"check_for_quick_attack",  6,},
  {"check_for_accelerate",    7,},
  {"check_slap_imps",         8,},
  {"check_enemy_entrances",   9,},
  {"check_for_place_door",   10,},
  {"check_neutral_places",   11,},
  {"check_for_place_trap",   12,},
  {"check_for_expand_room",  13,},
  {"check_for_money",        14,},
  {"none",                   15,},
  {NULL,                      0,},
};

Comp_Check_Func computer_check_func_list[] = {
  NULL,
  computer_checks_hates,
  computer_check_move_creatures_to_best_room,
  computer_check_move_creatures_to_room,
  computer_check_no_imps,
  computer_check_for_pretty,
  computer_check_for_quick_attack,
  computer_check_for_accelerate,
  computer_check_slap_imps,
  computer_check_enemy_entrances,
  computer_check_for_place_door,
  computer_check_neutral_places,
  computer_check_for_place_trap,
  computer_check_for_expand_room,
  computer_check_for_money,
  NULL,
  NULL,
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *compdngn;
    SYNCDBG(8,"Starting");
    //return _DK_computer_checks_hates(comp, check);
    compdngn = comp->dungeon;
    // Reference values for checking hate
    int cdngn_creatrs, cdngn_spwrkrs, cdngn_enrancs;
    cdngn_creatrs = count_creatures_in_dungeon(compdngn);
    cdngn_spwrkrs = count_diggers_in_dungeon(compdngn);
    cdngn_enrancs = count_entrances(comp, compdngn->owner);
    // Now check hate for every player
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo *player;
        struct Dungeon *dungeon;
        struct Comp2_UnkStr1 *rel;
        player = get_player(i);
        dungeon = get_players_dungeon(player);
        rel = &comp->unkarr_A10[i];
        if (!player_exists(player) || (player->id_number == compdngn->owner)
         || (player->id_number == game.neutral_player_num))
            continue;
        if (player->field_2C != 1)
            continue;
        if (players_are_mutual_allies(compdngn->owner, i))
            continue;
        int hdngn_creatrs, hdngn_spwrkrs, hdngn_enrancs;
        int hate_reasons;
        hate_reasons = 0;
        hdngn_creatrs = count_creatures_in_dungeon(dungeon);
        hdngn_spwrkrs = count_diggers_in_dungeon(dungeon);
        if (hdngn_creatrs >= cdngn_creatrs)
        {
            hate_reasons++;
            rel->field_42++;
        }
        if (cdngn_spwrkrs / 6 + cdngn_spwrkrs < hdngn_spwrkrs)
        {
            hate_reasons++;
            rel->field_42++;
        }
        if (((int)compdngn->buildable_rooms_count + (int)compdngn->buildable_rooms_count / 6) < (int)dungeon->buildable_rooms_count)
        {
            hate_reasons++;
            rel->field_42++;
        }
        hdngn_enrancs = count_entrances(comp, i);
        if (hdngn_enrancs > cdngn_enrancs)
        {
            hate_reasons++;
            rel->field_42 += 5;
        }
        // If no reason to hate the player - hate him randomly for just surviving that long
        if ((hate_reasons <= 0) && (check->param2 < game.play_gameturn))
        {
            if (ACTION_RANDOM(100) < 20) {
                rel->field_42++;
            }
        }
    }
    return 4;
}

long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_move_creatures_to_best_room(comp, check);
}

long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_move_creatures_to_room(comp, check);
}

long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_no_imps(comp, check);
}

long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_pretty(comp, check);
}

long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_quick_attack(comp, check);
}

struct Thing *computer_check_creatures_in_room_for_accelerate(struct Computer2 *comp, struct Room *room)
{
    struct Dungeon *dungeon;
    struct StateInfo *stati;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i,n;
    dungeon = comp->dungeon;
    i = room->creatures_list;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature %ld detected",i);
        break;
      }
      i = cctrl->next_in_room;
      // Per creature code
      if (!thing_affected_by_spell(thing, SplK_Speed))
      {
          if (thing->active_state == CrSt_MoveToPosition)
              n = thing->continue_state;
          else
              n = thing->active_state;
          stati = get_thing_state_info_num(n);
          if (stati->state_type == 1)
          {
              if (try_game_action(comp, dungeon->owner, GA_UsePwrSpeedUp, SPELL_MAX_LEVEL, 0, 0, thing->index, 0) > 0)
              {
                  return thing;
              }
          }
      }
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(struct Computer2 *comp, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    struct Thing *thing;
    long i;
    unsigned long k;
    if ((rkind < 1) || (rkind > ROOM_TYPES_COUNT))
    {
        ERRORLOG("Invalid room kind %d",(int)rkind);
        return INVALID_THING;
    }
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid computer players dungeon");
        return INVALID_THING;
    }
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
      room = room_get(i);
      if (room_is_invalid(room))
      {
        ERRORLOG("Jump to invalid room detected");
        break;
      }
      i = room->next_of_owner;
      // Per-room code
      thing = computer_check_creatures_in_room_for_accelerate(comp, room);
      if (!thing_is_invalid(thing))
          return thing;
      // Per-room code ends
      k++;
      if (k > ROOMS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping rooms list");
        break;
      }
    }
    return INVALID_THING;
}

long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check)
{
    static RoomKind workers_in_rooms[] = {RoK_LIBRARY,RoK_LIBRARY,RoK_WORKSHOP,RoK_TRAINING,RoK_SCAVENGER};
    struct Thing *thing;
    long i,n;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_accelerate(comp, check);
    if (computer_able_to_use_magic(comp, 11, 8, 3) != 1)
    {
        return 4;
    }
    n = check->param2 % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    if (n <= 0)
        n = ACTION_RANDOM(sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    for (i=0; i < sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]); i++)
    {
        thing = computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(comp, workers_in_rooms[n]);
        if (!thing_is_invalid(thing))
        {
            SYNCDBG(8,"Cast on thing %d",(int)thing->index);
            return 1;
        }
        n = (n+1) % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    }
    return 4;
}

long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_slap_imps(comp, check);
}

long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_enemy_entrances(comp, check);
}

long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_place_door(comp, check);
}

long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_neutral_places(comp, check);
}

long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_expand_room(comp, check);
}
/******************************************************************************/
