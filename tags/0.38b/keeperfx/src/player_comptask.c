/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer.c
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Defines a computer player control variables and events/checks/processes
 *      functions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_computer.h"

#include <limits.h>
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_memory.h"
#include "bflib_sound.h"

#include "config.h"
#include "config_creature.h"
#include "magic.h"
#include "thing_traps.h"
#include "player_instances.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TrapDoorSelling {
    long category;
    long model;
};

struct MoveToRoom {
    char kind;
    long field_1;
};

/******************************************************************************/
long task_dig_room_passage(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_room(struct Computer2 *comp, struct ComputerTask *ctask);
long task_check_room_dug(struct Computer2 *comp, struct ComputerTask *ctask);
long task_place_room(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_entrance(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_gold(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_attack(struct Computer2 *comp, struct ComputerTask *ctask);
long task_magic_call_to_arms(struct Computer2 *comp, struct ComputerTask *ctask);
long task_pickup_for_attack(struct Computer2 *comp, struct ComputerTask *ctask);
long task_move_creature_to_room(struct Computer2 *comp, struct ComputerTask *ctask);
long task_move_creature_to_pos(struct Computer2 *comp, struct ComputerTask *ctask);
long task_move_creatures_to_defend(struct Computer2 *comp, struct ComputerTask *ctask);
long task_slap_imps(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_neutral(struct Computer2 *comp, struct ComputerTask *ctask);
long task_magic_speed_up(struct Computer2 *comp, struct ComputerTask *ctask);
long task_wait_for_bridge(struct Computer2 *comp, struct ComputerTask *ctask);
long task_attack_magic(struct Computer2 *comp, struct ComputerTask *ctask);
long task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask);
/******************************************************************************/
const struct TaskFunctions task_function[] = {
    {NULL, NULL},
    {"COMPUTER_DIG_ROOM_PASSAGE", task_dig_room_passage},
    {"COMPUTER_DIG_ROOM",         task_dig_room},
    {"COMPUTER_CHECK_ROOM_DUG",   task_check_room_dug},
    {"COMPUTER_PLACE_ROOM",       task_place_room},
    {"COMPUTER_DIG_TO_ENTRANCE",  task_dig_to_entrance},
    {"COMPUTER_DIG_TO_GOLD",      task_dig_to_gold},
    {"COMPUTER_DIG_TO_ATTACK",    task_dig_to_attack},
    {"COMPUTER_MAGIC_CALL_TO_ARMS", task_magic_call_to_arms},
    {"COMPUTER_PICKUP_FOR_ATTACK", task_pickup_for_attack},
    {"COMPUTER_MOVE_CREATURE_TO_ROOM", task_move_creature_to_room},
    {"COMPUTER_MOVE_CREATURE_TO_POS", task_move_creature_to_pos},
    {"COMPUTER_MOVE_CREATURES_TO_DEFEND", task_move_creatures_to_defend},
    {"COMPUTER_SLAP_IMPS",        task_slap_imps},
    {"COMPUTER_DIG_TO_NEUTRAL",   task_dig_to_neutral},
    {"COMPUTER_MAGIC_SPEED_UP",   task_magic_speed_up},
    {"COMPUTER_WAIT_FOR_BRIDGE",  task_wait_for_bridge},
    {"COMPUTER_ATTACK_MAGIC",     task_attack_magic},
    {"COMPUTER_SELL_TRAPS_AND_DOORS", task_sell_traps_and_doors},
};

const struct TrapDoorSelling trapdoor_sell[] = {
    {TDSC_Door, 4},
    {TDSC_Trap, 1},
    {TDSC_Trap, 6},
    {TDSC_Door, 3},
    {TDSC_Trap, 5},
    {TDSC_Trap, 4},
    {TDSC_Door, 2},
    {TDSC_Trap, 3},
    {TDSC_Door, 1},
    {TDSC_Trap, 2},
    {TDSC_EndList, 0},
};

const struct MoveToRoom move_to_room[] = {
    {RoK_TRAINING,  40},
    {RoK_LIBRARY,   35},
    {RoK_WORKSHOP,  32},
    {RoK_SCAVENGER, 20},
    {RoK_NONE,       0},
};
/******************************************************************************/
DLLIMPORT long _DK_task_dig_room_passage(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_room(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_check_room_dug(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_place_room(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_to_entrance(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_to_gold(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_to_attack(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_magic_call_to_arms(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_pickup_for_attack(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_move_creature_to_room(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_move_creature_to_pos(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_move_creatures_to_defend(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_slap_imps(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_to_neutral(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_magic_speed_up(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_wait_for_bridge(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_attack_magic(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT struct ComputerTask *_DK_get_task_in_progress(struct Computer2 *comp, long a2);
DLLIMPORT struct ComputerTask *_DK_get_free_task(struct Computer2 *comp, long a2);
DLLIMPORT short _DK_fake_dump_held_creatures_on_map(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_fake_place_thing_in_power_hand(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos);
DLLIMPORT struct Thing *_DK_find_creature_to_be_placed_in_room(struct Computer2 *comp, struct Room **roomp);
DLLIMPORT short _DK_game_action(char a1, unsigned short a2, unsigned short a3, unsigned short a4,
 unsigned short a5, unsigned short a6, unsigned short a7);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct ComputerTask *get_computer_task(long idx)
{
    if ((idx < 1) || (idx >= COMPUTER_TASKS_COUNT))
    {
        return &game.computer_task[0];
    } else
    {
        return &game.computer_task[idx];
    }
}

TbBool computer_task_invalid(struct ComputerTask *ctask)
{
    if (ctask <= &game.computer_task[0])
        return true;
    return false;
}

TbBool remove_task(struct Computer2 *comp, struct ComputerTask *ctask)
{
  struct ComputerTask *nxctask;
  long i;
  i = comp->field_14C6;
  if (&game.computer_task[i] == ctask)
  {
    comp->field_14C6 = ctask->next_task;
    ctask->next_task = 0;
    set_flag_byte(&ctask->field_0, 0x01, false);
    return false;
  }
  nxctask = &game.computer_task[i];
  while (!computer_task_invalid(nxctask))
  {
      i = nxctask->next_task;
      if (&game.computer_task[i] == ctask)
      {
        nxctask->next_task = ctask->next_task;
        ctask->next_task = 0;
        set_flag_byte(&ctask->field_0, 0x01, false);
        return true;
      }
      nxctask = &game.computer_task[i];
  }
  return false;
}

short game_action(char plyr_idx, unsigned short gaction, unsigned short a3,
    unsigned short stl_x, unsigned short stl_y, unsigned short param1, unsigned short param2)
{
    //TODO: may hang if out of things
    struct Dungeon *dungeon;
    MapSlabCoord slb_x,slb_y;
    struct Thing *thing;
    long i,k;
    struct SlabMap *slb;
    struct Room *room;
    struct Coord3d pos;
    SYNCDBG(9,"Starting action %d",(int)gaction);
    //return _DK_game_action(plyr_idx, gaction, a3, stl_x, stl_y, param1, param2);
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
      return 0;
    slb_x = map_to_slab[stl_x];
    slb_y = map_to_slab[stl_y];
    switch (gaction)
    {
    case 1:
        break;
    case 2:
        if (dungeon->magic_level[1] == 0)
          break;
        thing = thing_get(param1);
        i = place_thing_in_power_hand(thing, plyr_idx);
        if (i < 0)
          return -1;
        return 1;
    case 3:
        if (dungeon->magic_level[1] == 0)
          break;
        i = dump_held_things_on_map(plyr_idx, stl_x, stl_y, 0);
        if (i < 0)
          return -1;
        return 1;
    case 4:
        if (dungeon->magic_level[2] == 0)
          break;
        i = magic_use_power_imp(plyr_idx, stl_x, stl_y);
        return 1;
    case 5:
        if (dungeon->magic_level[4] == 0)
          break;
        i = magic_use_power_slap(plyr_idx, stl_x, stl_y);
        return i;
    case 6:
        if (dungeon->magic_level[5] == 0)
          break;
        i = magic_use_power_sight(plyr_idx, stl_x, stl_y, a3);
        return i;
    case 7:
        if (dungeon->magic_level[3] == 0)
          break;
        i = magic_use_power_obey(plyr_idx);
        return 1;
    case 8:
        if (dungeon->magic_level[8] == 0)
          break;
        thing = thing_get(param1);
        magic_use_power_heal(plyr_idx, thing, thing->mappos.x.stl.num,thing->mappos.y.stl.num, a3);
        return 1;
    case 9:
        if (dungeon->magic_level[6] == 0)
          break;
        i = magic_use_power_call_to_arms(plyr_idx, stl_x, stl_y, a3, 1);
        return i;
    case 10:
        if (dungeon->magic_level[7] == 0)
          break;
        magic_use_power_cave_in(plyr_idx, stl_x, stl_y, a3);
        return 1;
    case 11:
        turn_off_call_to_arms(plyr_idx);
        return 1;
    case 12:
        dungeon->field_88C[0] = 0;
        return 1;
    case 13:
    case 14:
        slb = get_slabmap_block(slb_x, slb_y);
        if ((slb->kind == SlbT_LAVA) || (slb->kind == SlbT_WATER))
        {
            place_slab_type_on_map(10, stl_x, stl_y, plyr_idx, 0);
            do_slab_efficiency_alteration(slb_x, slb_y);
            i = 1;
        } else
        {
            i = tag_blocks_for_digging_in_rectangle_around(3 * (stl_x / 3), 3 * (stl_y / 3), plyr_idx) > 0;
        }
        return i;
    case 15:
    case 16:
        room = player_build_room_at(stl_x, stl_y, plyr_idx, param2);
        if (room_is_invalid(room))
          break;
        return room->index;
    case 17:
        dungeon->creature_tendencies = param1;
        return 1;
    case 18:
        if (dungeon->trap_amount[param1] == 0)
            break;
        pos.x.stl.pos = 128;
        pos.x.stl.num = 3 * (stl_x / 3) + 1;
        pos.y.stl.pos = 128;
        pos.y.stl.num = 3 * (stl_y / 3) + 1;
        pos.z.val = 0;
        pos.z.val = get_floor_height_at(&pos);
        thing = create_trap(&pos, param1, plyr_idx);
        if (remove_workshop_item(plyr_idx, 8, param1))
          dungeon->lvstats.traps_used++;
        dungeon->field_EA4 = 192;
        if (is_my_player_number(plyr_idx))
            play_non_3d_sample(117);
        return 1;
    case 19:
        k = tag_cursor_blocks_place_door(plyr_idx, stl_x, stl_y);
        i = packet_place_door(stl_x, stl_y, plyr_idx, param1, k);
        return i;
    case 20:
        magic_use_power_lightning(plyr_idx, stl_x, stl_y, a3);
        return 1;
    case 21:
        thing = thing_get(param1);
        magic_use_power_speed(plyr_idx, thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num, a3);
        return 1;
    case 22:
        thing = thing_get(param1);
        magic_use_power_armour(plyr_idx, thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num, a3);
        return 1;
    case 23:
        thing = thing_get(param1);
        magic_use_power_conceal(plyr_idx, thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num, a3);
        return 1;
    case 24:
        magic_use_power_hold_audience(plyr_idx);
        break;
    case 25:
        thing = thing_get(param1);
        magic_use_power_disease(plyr_idx, thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num, a3);
        return 1;
    case 26:
        thing = thing_get(param1);
        magic_use_power_chicken(plyr_idx, thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num, a3);
        return 1;
    case 28:
        if (dungeon->magic_level[4] == 0)
          break;
        thing = thing_get(param1);
        i = magic_use_power_slap_thing(plyr_idx, thing);
        return i;
    default:
        ERRORLOG("Unknown game action %d", (int)gaction);
        break;
    }
    return 0;
}

long try_game_action(struct Computer2 *comp, char a2, unsigned short a3, unsigned short a4,
 unsigned short a5, unsigned short a6, unsigned short a7, unsigned short a8)
{
  long result;
  result = game_action(a2, a3, a4, a5, a6, a7, a8);
  if (result > 0)
    comp->field_10--;
  SYNCDBG(19,"Returning %ld",result);
  return result;
}

struct ComputerTask *get_task_in_progress(struct Computer2 *comp, long a2)
{
    return _DK_get_task_in_progress(comp, a2);
}

struct ComputerTask *get_free_task(struct Computer2 *comp, long a2)
{
    return _DK_get_free_task(comp, a2);
}

short fake_dump_held_creatures_on_map(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos)
{
    return _DK_fake_dump_held_creatures_on_map(comp, thing, pos);
}

long fake_place_thing_in_power_hand(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos)
{
    SYNCDBG(9,"Starting");
    return _DK_fake_place_thing_in_power_hand(comp, thing, pos);
}

TbBool worker_needed_in_dungeons_room_kind(const struct Dungeon *dungeon, long rkind)
{
    long i;
    switch (rkind)
    {
    case RoK_LIBRARY:
        if (dungeon->field_F78 < 0)
            return false;
        return true;
    case RoK_TRAINING:
        if (2 * dungeon->field_14B8 >= dungeon->money)
            return false;
        return true;
    case RoK_WORKSHOP:
        for (i = 1; i < TRAP_TYPES_COUNT; i++)
        {
            if ((dungeon->trap_buildable[i]) && (dungeon->trap_amount[i] == 0))
            {
              break;
            }
        }
        if (i == TRAP_TYPES_COUNT)
            return false;
        return true;
    default:
        return true;
    }
}

long get_job_for_room(long rkind)
{
    switch (rkind)
    {
    case RoK_LIBRARY:
        return Job_RESEARCH;
    case RoK_TRAINING:
        return Job_TRAIN;
    case RoK_WORKSHOP:
        return Job_MANUFACTURE;
    case RoK_SCAVENGER:
        return Job_SCAVENGE;
    case RoK_TEMPLE:
        return Job_TEMPLE;
    case RoK_GUARDPOST:
        return Job_GUARD;
//    case RoK_TORTURE: -- no 'bad jobs' should be listed here
//        return Job_KINKY_TORTURE;
    default:
        return Job_NULL;
    }
}

TbBool person_will_do_job_for_room(const struct Thing *thing, const struct Room *room)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (get_job_for_room(room->kind) & crstat->jobs_not_do) == 0;
}

TbBool person_will_do_job_for_room_kind(const struct Thing *thing, long rkind)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (get_job_for_room(rkind) & crstat->jobs_not_do) == 0;
}

struct Room *get_room_to_place_creature(const struct Computer2 *comp, const struct Thing *thing)
{
  const struct Dungeon *dungeon;
  long chosen_priority;
  struct Room *chosen_room;
  struct Room *room;
  long total_spare_cap;
  long i,k,rkind;

    dungeon = comp->dungeon;

    chosen_room = NULL;
    chosen_priority = LONG_MIN;
    for (k=0; move_to_room[k].kind != RoK_NONE; k++)
    {
        rkind = move_to_room[k].kind;
        if (person_will_do_job_for_room_kind(thing,rkind))
        {
            if (!worker_needed_in_dungeons_room_kind(dungeon,rkind))
                continue;
        }
        // Find specific room which meets capacity demands
        i = dungeon->room_kind[rkind];
        room = find_room_with_most_spare_capacity_starting_with(i,&total_spare_cap);
        if (room_is_invalid(room))
            continue;
        if (chosen_priority < total_spare_cap * move_to_room[k].field_1)
        {
            chosen_priority = total_spare_cap * move_to_room[k].field_1;
            chosen_room = room;
        }
    }
  return chosen_room;
}

struct Thing *find_creature_to_be_placed_in_room(struct Computer2 *comp, struct Room **roomp)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *thing;
    struct Room *room;
    SYNCDBG(9,"Starting");
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid dungeon in computer player.");
        return INVALID_THING;
    }
    //return _DK_find_creature_to_be_placed_in_room(comp, roomp);
    param.ptr1 = (void *)comp;
    filter = player_list_creature_filter_needs_to_be_placed_in_room;
    thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    if (thing_is_invalid(thing))
        return INVALID_THING;
    room = get_room_of_given_kind_for_thing(thing,dungeon,param.num2);
    if (room_is_invalid(room))
        return INVALID_THING;
    *roomp = room;
    return thing;
}

long task_dig_room_passage(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_dig_room_passage(comp,ctask);
}

long task_dig_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_dig_room(comp,ctask);
}

long task_check_room_dug(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_check_room_dug(comp,ctask);
}

long task_place_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_place_room(comp,ctask);
}

long task_dig_to_entrance(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_dig_to_entrance(comp,ctask);
}

long task_dig_to_gold(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_dig_to_gold(comp,ctask);
}

long task_dig_to_attack(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_dig_to_attack(comp,ctask);
}

long task_magic_call_to_arms(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_magic_call_to_arms(comp,ctask);
}

long task_pickup_for_attack(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_pickup_for_attack(comp,ctask);
}

long task_move_creature_to_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Thing *thing;
    struct Room *room;
    struct Coord3d pos;
    long i;
    SYNCDBG(9,"Starting");
    //return _DK_task_move_creature_to_room(comp,ctask);
    room = INVALID_ROOM;
    thing = thing_get(comp->field_14C8);
    if (!thing_is_invalid(thing))
    {
      room = room_get(ctask->word_80);
      pos.x.val = room->central_stl_x << 8;
      pos.y.val = room->central_stl_y << 8;
      pos.z.val = 256;
      if (fake_dump_held_creatures_on_map(comp, thing, &pos) > 0)
        return 2;
      remove_task(comp, ctask);
      return 0;
    }
    i = ctask->field_7C;
    ctask->field_7C--;
    if (i <= 0)
    {
      remove_task(comp, ctask);
      return 1;
    }
    thing = find_creature_to_be_placed_in_room(comp, &room);
    if (!thing_is_invalid(thing))
    {
        //TODO CREATURE_AI try to make sure the creature will do proper activity in the room
        //     ie. select a room tile which is far from CTA and enemies
        //TODO CREATURE_AI don't place creatures at center of a temple/portal if we don't want to get rid of them
        //TODO CREATURE_AI make sure to place creatures at "active" portal tile if we do want them to leave
        ctask->word_80 = room->index;
        pos.x.val = room->central_stl_x << 8;
        pos.y.val = room->central_stl_y << 8;
        pos.z.val = 256;
        if ( fake_place_thing_in_power_hand(comp, thing, &pos) )
          return 2;
        remove_task(comp, ctask);
        return 0;
      }
      remove_task(comp, ctask);
      return 0;
}

long task_move_creature_to_pos(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_move_creature_to_pos(comp,ctask);
}

long task_move_creatures_to_defend(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_move_creatures_to_defend(comp,ctask);
}

long task_slap_imps(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_slap_imps(comp,ctask);
}

long task_dig_to_neutral(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_dig_to_neutral(comp,ctask);
}

long task_magic_speed_up(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_magic_speed_up(comp,ctask);
}

long task_wait_for_bridge(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_wait_for_bridge(comp,ctask);
}

long task_attack_magic(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    long i;
    SYNCDBG(9,"Starting");
    //return _DK_task_attack_magic(comp,ctask);
    dungeon = comp->dungeon;
    thing = thing_get(ctask->word_76);
    if (thing_is_invalid(thing))
    {
        return 1;
    }
    i = ctask->field_7C;
    ctask->field_7C--;
    if ((i <= 0) || (thing->health <= 0))
    {
        remove_task(comp, ctask);
        return 1;
    }
    if (computer_able_to_use_magic(comp, ctask->long_86, ctask->field_70, 1) != 1)
      return 4;
    i = try_game_action(comp, dungeon->owner, ctask->field_80, ctask->field_70,
        thing->mappos.x.stl.num, thing->mappos.y.stl.num, ctask->word_76, 0);
    if (i <= 0)
      return 4;
    return 2;
}

long task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    const struct TrapDoorSelling *tdsell;
    TbBool item_sold;
    long value,model;
    long i;
    SYNCDBG(19,"Starting");
    //return _DK_task_sell_traps_and_doors(comp,ctask);
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid dungeon in computer player.");
        return 0;
    }
    if ((ctask->field_7C >= ctask->long_76) && (ctask->field_80 >= dungeon->money))
    {
        i = 0;
        value = 0;
        item_sold = false;
        for (i=0; i < sizeof(trapdoor_sell)/sizeof(trapdoor_sell[0]); i++)
        {
            tdsell = &trapdoor_sell[ctask->long_86];
            switch (tdsell->category)
            {
            case TDSC_Door:
                model = tdsell->model;
                if ((model < 0) || (model >= DOOR_TYPES_COUNT))
                {
                    ERRORLOG("Internal error - invalid model %ld in slot %ld",model,i);
                    break;
                }
                if (dungeon->door_amount[model] > 0)
                {
                  item_sold = true;
                  value = game.doors_config[model].selling_value;
                  if (remove_workshop_item(dungeon->owner, 9, model))
                  {
                    remove_workshop_object_from_player(dungeon->owner, door_to_object[model]);
                  }
                  SYNCDBG(9,"Door model %ld sold for %ld gold",model,value);
                }
                break;
            case TDSC_Trap:
                model = tdsell->model;
                if ((model < 0) || (model >= TRAP_TYPES_COUNT))
                {
                    ERRORLOG("Internal error - invalid model %ld in slot %ld",model,i);
                    break;
                }
                if (dungeon->trap_amount[model] > 0)
                {
                  item_sold = true;
                  value = game.traps_config[model].selling_value;
                  if (remove_workshop_item(dungeon->owner, 8, model))
                  {
                    remove_workshop_object_from_player(dungeon->owner, trap_to_object[model]);
                  }
                  SYNCDBG(9,"Trap model %ld sold for %ld gold",model,value);
                }
                break;
            default:
                ERRORLOG("Unknown SELL_ITEM type");
                break;
            }
            ctask->long_86++;
            if (trapdoor_sell[ctask->long_86].category == TDSC_EndList)
                ctask->long_86 = 0;
            if (item_sold)
            {
                ctask->field_70--;
                if (ctask->field_70 > 0)
                {
                  ctask->long_76 += value;
                  dungeon->field_AFD += value;
                  dungeon->money += value;
                  return 1;
                }
                remove_task(comp, ctask);
                return 1;
            }
        }
    }
    SYNCDBG(9,"Couldn't sell anything, aborting.");
    remove_task(comp, ctask);
    return 0;
}

TbBool create_task_move_creatures_to_defend(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num, unsigned long evflags)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (ctask == NULL)
        return false;
    ctask->ttype = CTT_MoveCreaturesToDefend;
    ctask->pos_76.x.val = pos->x.val;
    ctask->pos_76.y.val = pos->y.val;
    ctask->pos_76.z.val = pos->z.val;
    ctask->field_7C = creatrs_num;
    ctask->field_70 = evflags;
    ctask->field_A = game.play_gameturn;
    ctask->field_5C = game.play_gameturn;
    ctask->field_60 = comp->field_34;
    return true;
}

TbBool create_task_magic_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (ctask == NULL)
        return false;
    ctask->ttype = CTT_MagicCallToArms;
    ctask->field_1 = 0;
    ctask->pos_76.x.val = pos->x.val;
    ctask->pos_76.y.val = pos->y.val;
    ctask->pos_76.z.val = pos->z.val;
    ctask->field_7C = creatrs_num;
    ctask->field_A = game.play_gameturn;
    ctask->field_60 = 25;
    ctask->field_5C = game.play_gameturn - 25;
    ctask->field_8E = 2500;
    return true;
}

TbBool create_task_sell_traps_and_doors(struct Computer2 *comp, long value)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (ctask == NULL)
        return false;
    ctask->ttype = CTT_SellTrapsAndDoors;
    ctask->field_70 = 0;
    ctask->field_A = game.play_gameturn;
    ctask->field_5C = game.play_gameturn;
    ctask->field_60 = 1;
    ctask->field_70 = 5;
    ctask->long_76 = 0;
    ctask->field_7C = value;
    ctask->field_80 = value;
    ctask->long_86 = 0;
    return true;
}

TbBool create_task_move_creature_to_pos(struct Computer2 *comp, struct Thing *thing, long a2, long a3)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (ctask == NULL)
        return false;
    ctask->ttype = CTT_MoveCreatureToPos;
    ctask->word_86 = a2 << 8;
    ctask->word_88 = a3 << 8;
    ctask->word_76 = thing->index;
    ctask->word_80 = 0;
    ctask->field_A = game.play_gameturn;
    return true;
}

long process_tasks(struct Computer2 *comp)
{
    struct ComputerTask *ctask;
    long ndone;
    long i,n;
    unsigned long k;
    //return _DK_process_tasks(comp);
    ndone = 0;
    k = 0;
    i = comp->field_14C6;
    while (i != 0)
    {
        if ((i < 0) || (i >= COMPUTER_TASKS_COUNT))
        {
          ERRORLOG("Jump to invalid computer task %ld detected",i);
          break;
        }
        if (comp->field_10 <= 0)
            break;
        ctask = &game.computer_task[i];
        i = ctask->next_task;
        if ((ctask->field_0 & 0x01) != 0)
        {
            n = ctask->ttype;
            if ((n > 0) && (n < sizeof(task_function)/sizeof(task_function[0])))
            {
                SYNCDBG(12,"Computer Task Type %ld",n);
                task_function[n].func(comp, ctask);
                ndone++;
            } else
            {
                ERRORLOG("Bad Computer Task Type %ld",n);
            }
        }
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping computer tasks");
          break;
        }
    }
    return ndone;
}
/******************************************************************************/
