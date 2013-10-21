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
#include "config_terrain.h"
#include "player_instances.h"
#include "creature_states.h"
#include "spdigger_stack.h"
#include "magic.h"
#include "dungeon_data.h"
#include "room_data.h"
#include "power_hand.h"
#include "gui_soundmsgs.h"
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
DLLIMPORT struct Thing * _DK_find_imp_for_pickup(struct Computer2 *comp, long stl_x, long stl_y);

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

struct ExpandRooms expand_rooms[] = {
  {RoK_TREASURE, 45},
  {RoK_LAIR, 45},
  {RoK_GARDEN, 45},
  {RoK_LIBRARY, 45},
  {RoK_TRAINING, 35},
  {RoK_WORKSHOP, 45},
  {RoK_SCAVENGER, 30},
  {RoK_PRISON, 30},
  {RoK_TEMPLE, 25},
  {RoK_TORTURE, 35},
  {RoK_GRAVEYARD, 30},
  {RoK_BARRACKS, 35},
  {RoK_NONE,0},
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
    int cdngn_creatrs, cdngn_spdiggrs, cdngn_enrancs;
    cdngn_creatrs = count_creatures_in_dungeon(compdngn);
    cdngn_spdiggrs = count_diggers_in_dungeon(compdngn);
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
        int hdngn_creatrs, hdngn_spdiggrs, hdngn_enrancs;
        int hate_reasons;
        hate_reasons = 0;
        hdngn_creatrs = count_creatures_in_dungeon(dungeon);
        hdngn_spdiggrs = count_diggers_in_dungeon(dungeon);
        // Computers hate players who have more creatures than them
        if (hdngn_creatrs >= cdngn_creatrs)
        {
            hate_reasons++;
            rel->field_42++;
        }
        // Computers hate players who have more special diggers than them
        if (cdngn_spdiggrs / 6 + cdngn_spdiggrs < hdngn_spdiggrs)
        {
            hate_reasons++;
            rel->field_42++;
        }
        // Computers hate players who can build more rooms than them
        if (((int)compdngn->buildable_rooms_count + (int)compdngn->buildable_rooms_count / 6) < (int)dungeon->buildable_rooms_count)
        {
            hate_reasons++;
            rel->field_42++;
        }
        // Computers highly hate players who claimed more entrances than them
        hdngn_enrancs = count_entrances(comp, i);
        if (hdngn_enrancs > cdngn_enrancs)
        {
            hate_reasons++;
            rel->field_42 += 5;
        }
        // If no reason to hate the player - hate him randomly for just surviving that long
        if ((hate_reasons <= 0) && (check->param1 < game.play_gameturn))
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
    struct Dungeon *dungeon;
    struct ComputerTask *ctask;
    dungeon = comp->dungeon;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_move_creatures_to_best_room(comp, check);
    //TODO check if should be changed to computer_able_to_use_magic()
    if (!is_power_available(dungeon->owner, PwrK_HAND)) {
        return 4;
    }
    int num_to_move;
    num_to_move = check->param1 * dungeon->num_active_creatrs / 100;
    if (num_to_move <= 0) {
        SYNCDBG(8,"No creatures to move, active %d percentage %d", (int)dungeon->num_active_creatrs, (int)check->param1);
        return 4;
    }
    if (get_task_in_progress(comp, CTT_MoveCreatureToRoom) != NULL) {
        return 4;
    }
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return 4;
    }
    ctask->ttype = CTT_MoveCreatureToRoom;
    ctask->word_70 = 0;
    ctask->word_80 = 0;
    ctask->field_7C = num_to_move;
    ctask->field_A = game.play_gameturn;
    SYNCDBG(8,"Added task to move %d creatures to best room", (int)num_to_move);
    return 1;
}

long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    struct Room *room;
    dungeon = comp->dungeon;
    SYNCDBG(8,"Checking player %d for move to %s", (int)dungeon->owner, room_code_name(check->param2));
    //return _DK_computer_check_move_creatures_to_room(comp, check);
    //TODO check if should be changed to computer_able_to_use_magic()
    if (!is_power_available(dungeon->owner, PwrK_HAND)) {
        return 4;
    }
    int num_to_move;
    num_to_move = check->param1 * dungeon->num_active_creatrs / 100;
    if (num_to_move <= 0) {
        SYNCDBG(8,"No creatures to move, active %d percentage %d", (int)dungeon->num_active_creatrs, (int)check->param1);
        return 4;
    }
    if (get_task_in_progress(comp, CTT_MoveCreatureToRoom) != NULL) {
        return 4;
    }
    struct ComputerTask *ctask;
    unsigned long k;
    long i;
    k = 0;
    i = dungeon->room_kind[check->param2];
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
        if (room->total_capacity > room->used_capacity)
        {
            ctask = get_free_task(comp, 1);
            if (ctask != NULL) {
                ctask->ttype = CTT_MoveCreatureToRoom;
                ctask->word_70 = room->index;
                ctask->word_80 = room->index;
                ctask->field_7C = num_to_move;
                ctask->field_A = game.play_gameturn;
                SYNCDBG(8,"Added task to move %d creatures to room %d", (int)num_to_move,(int)room->index);
                return 1;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return 4;
}

long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_no_imps(comp, check);
    dungeon = comp->dungeon;
    if (dungeon->num_active_diggers >= check->param1) {
        return 4;
    }
    long able;
    able = computer_able_to_use_magic(comp, PwrK_MKDIGGER, 0, 1);
    if (able == 1)
    {
        struct Thing *heartng;
        MapSubtlCoord stl_x, stl_y;
        heartng = thing_get(dungeon->dnheart_idx);
        stl_x = heartng->mappos.x.stl.num;
        stl_y = heartng->mappos.y.stl.num;
        if (xy_walkable(stl_x, stl_y, dungeon->owner))
        {
            if (try_game_action(comp, dungeon->owner, GA_UseMkDigger, 0, stl_x, stl_y, 1, 1) > 0) {
                able = 1;
            }
        }
    }
    return able;
}

struct Thing * find_imp_for_pickup(struct Computer2 *comp, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Dungeon *dungeon;
    int pick1_dist;
    struct Thing *pick1_tng;
    int pick2_dist;
    struct Thing *pick2_tng;
    //return _DK_find_imp_for_pickup(comp, stl_x, stl_y);
    dungeon = comp->dungeon;
    pick1_dist = INT_MAX;
    pick2_dist = INT_MAX;
    pick2_tng = INVALID_THING;
    pick1_tng = INVALID_THING;
    long i;
    unsigned long k;
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        struct Thing *thing;
        struct CreatureControl *cctrl;
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
          ERRORLOG("Jump to invalid creature detected");
          break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->combat_flags == 0)
        {
            if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
            {
                if (!creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
                {
                    int dist;
                    long state_type;
                    dist = abs(stl_x - thing->mappos.x.stl.num) + abs(stl_y - thing->mappos.y.stl.num);
                    state_type = get_creature_state_type(thing);
                    if (state_type == CrStTyp_Value1)
                    {
                        if (dist < pick1_dist)
                        {
                            pick1_dist = dist;
                            pick1_tng = thing;
                        }
                    }
                    else
                    {
                        if (dist < pick2_dist)
                        {
                            pick2_dist = dist;
                            pick2_tng = thing;
                        }
                    }
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
    if (!thing_is_invalid(pick2_tng)) {
        return pick2_tng;
    } else {
        return pick1_tng;
    }
}

long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_pretty(comp, check);
    dungeon = comp->dungeon;
    MapSubtlCoord stl_x, stl_y;
    {
        long stack_len;
        stack_len = dungeon->digger_stack_length;
        if (stack_len <= check->param1 * dungeon->total_area / 100) {
            return 4;
        }
        long n;
        n = find_in_imp_stack_starting_at(DigTsk_ImproveDungeon, ACTION_RANDOM(stack_len), dungeon);
        if (n < 0) {
            return 4;
        }
        const struct DiggerStack *istack;
        istack = &dungeon->imp_stack[n];
        stl_x = stl_num_decode_x(istack->field_0);
        stl_y = stl_num_decode_y(istack->field_0);
    }
    struct Thing * creatng;
    creatng = find_imp_for_pickup(comp, stl_x, stl_y);
    if (thing_is_invalid(creatng)) {
        return 4;
    }
    struct ComputerTask *ctask;
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return 4;
    }
    ctask->ttype = CTT_MoveCreatureToPos;
    ctask->word_86 = subtile_coord_center(stl_x);
    ctask->word_88 = subtile_coord_center(stl_y);
    ctask->word_76 = creatng->index;
    ctask->word_80 = 0;
    ctask->field_A = game.play_gameturn;
    return 1;
}

struct Room *get_opponent_room(struct Computer2 *comp, PlayerNumber plyr_idx)
{
    static const RoomKind opponent_room_kinds[] = {RoK_DUNGHEART, RoK_PRISON, RoK_LIBRARY, RoK_TREASURE};
    struct Dungeon *dungeon;
    struct Room *room;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon) || (slab_conf.room_types_count < 1)) {
        return INVALID_ROOM;
    }
    int i,n;
    n = opponent_room_kinds[ACTION_RANDOM(sizeof(opponent_room_kinds)/sizeof(opponent_room_kinds[0]))];
    for (i=0; i < slab_conf.room_types_count; i++)
    {
        room = room_get(dungeon->room_kind[n]);
        if (room_exists(room)) {
            return room;
        }
        n = (n + 1) % slab_conf.room_types_count;
    }
    return INVALID_ROOM;
}

struct Room *get_hated_room_for_quick_attack(struct Computer2 *comp, long min_hate)
{
    struct THate hate[PLAYERS_COUNT];
    long plyr_idx;
    get_opponent(comp, hate);
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (players_are_enemies(comp->dungeon->owner,plyr_idx))
        {
            if ((hate[plyr_idx].value[2]) && (hate[plyr_idx].value[0] > min_hate))
            {
                struct Room *room;
                room = get_opponent_room(comp, plyr_idx);
                if (!room_is_invalid(room)) {
                    return room;
                }
            }
        }
    }
    return INVALID_ROOM;
}

/**
 * Quick attack is just putting CTA spell on enemy room.

 * @param comp
 * @param check
 */
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_quick_attack(comp, check);
    dungeon = comp->dungeon;
    int creatrs_factor;
    creatrs_factor = check->param1 * dungeon->num_active_creatrs / 100;
    if (check->param3 >= creatrs_factor) {
        return 4;
    }
    if (computer_able_to_use_magic(comp, PwrK_CALL2ARMS, 1, 3) != 1) {
        return 4;
    }
    if ((check_call_to_arms(comp) != 1) || is_there_an_attack_task(comp)) {
        return 4;
    }
    struct Room *room;
    room = get_hated_room_for_quick_attack(comp, check->param3);
    if (room_is_invalid(room)) {
        return 4;
    }
    struct Coord3d pos;
    // TODO COMPUTER_AI We should make sure the place of cast is accessible for creatures
    pos.x.val = subtile_coord_center(room->central_stl_x);
    pos.y.val = subtile_coord_center(room->central_stl_y);
    pos.z.val = subtile_coord(1,0);
    if (check->param3 >= count_creatures_availiable_for_fight(comp, &pos)) {
        return 4;
    }
    struct ComputerTask *ctask;
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return 4;
    }
    output_message(SMsg_EnemyHarassments+ACTION_RANDOM(8), 500, 1);
    ctask->ttype = CTT_MagicCallToArms;
    ctask->field_1 = 0;
    ctask->pos_76.x.val = pos.x.val;
    ctask->pos_76.y.val = pos.y.val;
    ctask->pos_76.z.val = pos.z.val;
    ctask->field_7C = creatrs_factor;
    ctask->field_A = game.play_gameturn;
    ctask->field_60 = 25;
    ctask->field_5C = game.play_gameturn - 25;
    ctask->field_8E = check->param2;
    return 1;
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
    if (computer_able_to_use_magic(comp, PwrK_SPEEDCRTR, 8, 3) != 1)
    {
        return 4;
    }
    n = check->param1 % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
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

/**
 * This function generates "expand room" action on a tile which is claimed ground and could have a room placed on.
 * It is used to fix vandalized or not fully built rooms, so that they will cover the whole area digged for them.
 *
 * @param comp
 * @param check Computer check data.
 * @param room The room to be checked for expand.
 * @param around_start Random value used for setting starting point of the check process.
 * @return
 */
TbBool computer_check_for_expand_specific_room(struct Computer2 *comp, struct ComputerCheck * check, struct Room *room, long around_start)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    unsigned long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x,slb_y;
        struct SlabMap *slb;
        slb = get_slabmap_direct(i);
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code
        unsigned long m,n;
        m = around_start % SMALL_AROUND_SLAB_LENGTH;
        for (n=0; n < SMALL_AROUND_SLAB_LENGTH; n++)
        {
            MapSlabCoord arslb_x, arslb_y;
            int available_slabs;
            available_slabs = 0;
            arslb_x = slb_x + small_around[m].delta_x;
            arslb_y = slb_y + small_around[m].delta_y;
            slb = get_slabmap_block(arslb_x, arslb_y);
            if ((slb->kind == SlbT_CLAIMED) && (slabmap_owner(slb) == dungeon->owner))
            {
                available_slabs++;
                if (available_slabs >= 2)
                {
                    if (try_game_action(comp, dungeon->owner, GA_PlaceRoom, 0,
                        slab_subtile_center(arslb_x), slab_subtile_center(arslb_y), 1, room->kind) > 0) {
                        return true;
                    }
                }
            }
            m = (m+1) % SMALL_AROUND_SLAB_LENGTH;
        }
        // Per-slab code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    return false;
}

long computer_check_for_expand_room_kind(struct Computer2 *comp, struct ComputerCheck * check, RoomKind rkind, long max_slabs, long around_start)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid computer players dungeon");
        return 0;
    }
    struct Room *room;
    long i;
    unsigned long k;
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
        if ((room->slabs_count > 0) && (room->slabs_count < max_slabs)) {
            if (computer_check_for_expand_specific_room(comp, check, room, around_start)) {
                SYNCDBG(6,"The %s index %d will be expanded",room_code_name(room->kind),(int)room->index);
                return 1;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return 0;
}

long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_expand_room(comp, check);
    long around_start;
    around_start = ACTION_RANDOM(119);
    if (get_task_in_progress(comp, CTT_PlaceRoom))
    {
        SYNCDBG(8,"No rooms expansion - task already in progress");
        return 0;
    }
    const struct ExpandRooms *expndroom;
    for (expndroom = &expand_rooms[0]; expndroom->rkind != RoK_NONE; expndroom++)
    {
        if (computer_check_for_expand_room_kind(comp, check, expndroom->rkind, expndroom->max_slabs, around_start)) {
            return 1;
        }
    }
    SYNCDBG(8,"No rooms found for expansion");
    return 0;
}
/******************************************************************************/
