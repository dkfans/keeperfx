/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_compprocs.c
 *     Computer player processes definitions and routines.
 * @par Purpose:
 *     Defines a computer player processes and related functions.
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
#include "room_lair.h"

#include "gui_soundmsgs.h"
#include "dungeon_data.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_process_task(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process);
DLLIMPORT long _DK_set_next_process(struct Computer2 *comp);
DLLIMPORT long _DK_computer_finds_nearest_entrance2(struct Computer2 *comp, struct Coord3d *pos, struct Room **retroom, short plyr_idx);
DLLIMPORT long _DK_move_imp_to_dig_here(struct Computer2 *comp, struct Coord3d *pos, long a3);
DLLIMPORT struct ComputerProcess * _DK_find_best_process(struct Computer2 *comp);
/******************************************************************************/
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process);
long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_process_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process);
/******************************************************************************/
/*TODO DLL_CLEANUP enable ComputerProcess structs when there are no references to those in DLL
struct ComputerProcess BuildAllRooms3x3 = {
  "BUILD ALL ROOM 3x3", 0, 3, 3, 0, -1, computer_check_build_all_rooms,
  computer_setup_any_room_continue, computer_process_task,
  computer_completed_build_a_room, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildAllRooms4x4 = {
  "BUILD ALL ROOM 4x4", 0, 4, 4, 0, -1, computer_check_build_all_rooms,
  computer_setup_any_room_continue, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildPrisonRoom = {
  "BUILD A PRISON ROOM", 0, 3, 4, 4, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTortureRoom = {
  "BUILD A TORTURE ROOM", 0, 3, 4, 5, 4, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildScavengerRoom = {
  "BUILD A SCAVENGER ROOM", 0, 3, 3, 9, 4, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTempleRoom = {
  "BUILD A TEMPLE ROOM", 0, 3, 3, 10, 8, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildGraveyardRoom = {
  "BUILD A GRAVEYARD ROOM", 0, 4, 5, 11, 5, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildBarrackRoom = {
  "BUILD A BARRACK ROOM", 0, 3, 4, 12, 6, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTreasureRoom = {
  "BUILD A TREASURE ROOM", 10, 5, 5, 2, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildResearchRoom = {
  "BUILD A RESEARCH ROOM", 0, 5, 5, 3, 2, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildHatcheryRoom = {
  "BUILD A HATCHERY", 0, 6, 5, 13, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildLairRoom = {
 "BUILD A LAIR ROOM", 0, 5, 5, 14, 7, computer_check_any_room,
 computer_setup_any_room, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTrainingRoom = {
  "BUILD A TRAINING ROOM", 0, 4, 5, 6, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildWorkshopRoom = {
 "BUILD A WORKSHOP ROOM", 0, 6, 6, 8, 3, computer_check_any_room,
 computer_setup_any_room, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToEntrance = {
 "DIG TO AN ENTRANCE", 0, 1700, 0, 0, 0, computer_check_dig_to_entrance,
 computer_setup_dig_to_entrance, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldForMoney = {
 "DIG TO GOLD", 0, 10999, 150, 7, 0, computer_check_dig_to_gold,
 computer_setup_dig_to_gold, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTreasureRoom4x4 = {
  "BUILD A TREASURE ROOM 4x4", 10, 4, 4, 2, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildLairRoom4x4 = {
  "BUILD A LAIR ROOM 4x4", 0, 4, 4, 14, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToCloseGoldForMoney = {
 "DIG TO CLOSE GOLD", 0, 30999, 500, 5, 71, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldGreedy = {
 "DIG TO GREEDY GOLD", 0, 40999, 400, 7, 900, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldGreedy2 = {
  "DIG TO GREEDY GOLD2", 0, 40999, 50, 7, 900, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSightOfEvil = {
  "SIGHT OF EVIL", 0, 8, 64, 1500, 0, computer_check_sight_of_evil,
  computer_setup_sight_of_evil, computer_process_sight_of_evil,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSightOfEvilScare = {
  "SIGHT OF EVIL SCARE", 0, 8, 10, 5000, 0, computer_check_sight_of_evil,
  computer_setup_sight_of_evil, computer_process_sight_of_evil,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerAttack1 = {
  "ATTACK PLAN 1", 0, 55, 6, 80, 0, computer_check_attack1,
  computer_setup_attack1, computer_process_task,
  computer_completed_attack1, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSafeAttack = {
  "ATTACK SAFE ATTACK", 0, 25, 4, 80, 0, computer_check_safe_attack,
  computer_setup_attack1, computer_process_task,
  computer_completed_attack1, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};
*/
/******************************************************************************/
const struct NamedCommand computer_process_func_type[] = {
  {"check_build_all_rooms",   1,},
  {"setup_any_room_continue", 2,},
  {"check_any_room",          3,},
  {"setup_any_room",          4,},
  {"check_dig_to_entrance",   5,},
  {"setup_dig_to_entrance",   6,},
  {"check_dig_to_gold",       7,},
  {"setup_dig_to_gold",       8,},
  {"check_sight_of_evil",     9,},
  {"setup_sight_of_evil",    10,},
  {"process_sight_of_evil",  11,},
  {"check_attack1",          12,},
  {"setup_attack1",          13,},
  {"completed_attack1",      14,},
  {"check_safe_attack",      15,},
  {"process_task",           16,},
  {"completed_build_a_room", 17,},
  {"paused_task",            18,},
  {"completed_task",         19,},
  {"none",                   20,},
  {NULL,                      0,},
};

Comp_Process_Func computer_process_func_list[] = {
  NULL,
  computer_check_build_all_rooms,
  computer_setup_any_room_continue,
  computer_check_any_room,
  computer_setup_any_room,
  computer_check_dig_to_entrance,
  computer_setup_dig_to_entrance,
  computer_check_dig_to_gold,
  computer_setup_dig_to_gold,
  computer_check_sight_of_evil,
  computer_setup_sight_of_evil,
  computer_process_sight_of_evil,
  computer_check_attack1,
  computer_setup_attack1,
  computer_completed_attack1,
  computer_check_safe_attack,
  computer_process_task,
  computer_completed_build_a_room,
  computer_paused_task,
  computer_completed_task,
  NULL,
  NULL,
};
/******************************************************************************/
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process)
{
    //return _DK_computer_setup_any_room(comp, process);
    struct ComputerTask *ctask;
    long i;
    ctask = computer_setup_build_room(comp, process->field_10, process->field_8, process->field_C, process->field_14);
    if (!computer_task_invalid(ctask))
    {
        SYNCDBG(8,"Task for \"%s\" has been created",process->name);
        process->flags |= ComProc_Unkn0020;
        i = (long)((char *)process - (char *)&comp->processes[0]) / sizeof(struct ComputerProcess);
        if ((i < 0) || (i > COMPUTER_PROCESSES_COUNT))
        {
          ERRORLOG("Process \"%s\" is outside of Computer Player.",process->name);
          i = COMPUTER_PROCESSES_COUNT;
        }
        ctask->field_8C = i;
        shut_down_process(comp, process);
        return 2;
    }
    if (process->field_8 > process->field_C)
    {
        if (process->field_8 <= 2) {
            return 0;
        }
        process->field_8--;
    } else
    {
        if (process->field_C <= 2) {
            return 0;
        }
        process->field_C--;
    }
    reset_process(comp, process);
    return 2;
}

long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct ComputerTask *ctask;
    long i;
    //return _DK_computer_setup_any_room_continue(comp, process);
    ctask = computer_setup_build_room(comp, process->field_10, process->field_8, process->field_C, process->field_14);
    if (!computer_task_invalid(ctask))
    {
        SYNCDBG(8,"Task for \"%s\" has been created",process->name);
        process->flags |= ComProc_Unkn0020;
        i = (long)((char *)process - (char *)&comp->processes[0]) / sizeof(struct ComputerProcess);
        if ((i < 0) || (i > COMPUTER_PROCESSES_COUNT))
        {
          ERRORLOG("Process \"%s\" is outside of Computer Player.",process->name);
          i = COMPUTER_PROCESSES_COUNT;
        }
        ctask->field_8C = i;
        shut_down_process(comp, process);
        process->flags &= ~ComProc_Unkn0008;
        return 2;
    }
    if (process->field_8 > process->field_C)
    {
        if (process->field_8 <= 2) {
            return 0;
        }
        process->field_8--;
    } else
    {
        if (process->field_C <= 2) {
            return 0;
        }
        process->field_C--;
    }
    reset_process(comp, process);
    return 2;
}

long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
    //return _DK_computer_setup_sight_of_evil(comp, process);
    process->field_40++;
    if (process->field_C >= process->field_40) {
        return 1;
    }
    process->flags |= 0x0001;
    shut_down_process(comp, process);
    comp->task_state = 2;
    return 0;
}

long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
    //return _DK_computer_setup_attack1(comp, process);
    output_message(SMsg_EnemyHarassments + ACTION_RANDOM(8), 500, 1);
    return 1;
}

long count_no_room_build_tasks(const struct Computer2 *comp)
{
    const struct ComputerTask *ctask;
    long count;
    count = 0;
    long i;
    unsigned long k;
    i = comp->task_idx;
    k = 0;
    while (i != 0)
    {
        ctask = get_computer_task(i);
        if (computer_task_invalid(ctask))
        {
            ERRORLOG("Jump to invalid task detected");
            break;
        }
        i = ctask->next_task;
        // Per-task code
        if ((ctask->flags & ComTsk_Unkn0001) != 0)
        {
            unsigned char ttype;
            ttype = ctask->ttype;
            if ((ttype == CTT_DigRoomPassage) || (ttype == CTT_DigRoom)
             || (ttype == CTT_CheckRoomDug) || (ttype == CTT_PlaceRoom)) {
                SYNCDBG(19,"Task %d is matching type %d",(int)i,(int)ttype);
                count++;
            }
        }
        // Per-task code ends
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping tasks list");
            break;
        }
    }
    return count;
}

long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct Dungeon *dungeon;
    //return _DK_computer_check_build_all_rooms(comp, process);
    dungeon = comp->dungeon;
    if (count_no_room_build_tasks(comp) >= comp->max_room_build_tasks) {
        return 4;
    }
    struct ValidRooms *bldroom;
    for (bldroom = valid_rooms_to_build; bldroom->rkind > 0; bldroom++)
    {
        if (!dungeon_has_room(dungeon, bldroom->rkind))
        {
            if (computer_check_room_available(comp, bldroom->rkind) == IAvail_Now) {
                SYNCDBG(8,"Going to build %s",room_code_name(bldroom->rkind));
                process->field_10 = bldroom->rkind;
                return 1;
            }
        }
    }
    return 4;
}

long computer_get_room_kind_total_capacity(struct Computer2 *comp, RoomKind room_kind)
{
  struct Dungeon *dungeon;
  dungeon = comp->dungeon;
  long used_capacity;
  long total_capacity;
  get_room_kind_total_and_used_capacity(dungeon, room_kind, &total_capacity, &used_capacity);
  return total_capacity;
}

long computer_get_room_kind_free_capacity(struct Computer2 *comp, RoomKind room_kind)
{
  struct Dungeon *dungeon;
  dungeon = comp->dungeon;
  if (room_kind == RoK_GARDEN) {
      return 9999;
  }
  if (room_kind == RoK_LAIR)
  {
      if (!dungeon_has_room(dungeon, room_kind)) {
          return 9999;
      }
      return calculate_free_lair_space(comp->dungeon);
  }
  long used_capacity;
  long total_capacity;
  get_room_kind_total_and_used_capacity(dungeon, room_kind, &total_capacity, &used_capacity);
  if (total_capacity <= 0) {
      return 9999;
  }
  return total_capacity - used_capacity;
}

long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct Dungeon *dungeon;
    //return _DK_computer_check_any_room(comp, process);
    dungeon = comp->dungeon;
    ItemAvailability is_avail;
    is_avail = computer_check_room_available(comp, process->field_10);
    if (is_avail != IAvail_Now)
    {
        if (is_avail == IAvail_Never) {
            process->flags |= ComProc_Unkn0004;
            return 0;
        }
        return 4;
    }
    long num_build_tasks = count_no_room_build_tasks(comp);
    if (num_build_tasks >= comp->max_room_build_tasks) {
        SYNCDBG(19,"Not building \"%s\" because already doing %d build tasks",room_code_name(process->field_10),(int)num_build_tasks);
        return 4;
    }
    long used_capacity;
    long total_capacity;
    get_room_kind_total_and_used_capacity(dungeon, process->field_10, &total_capacity, &used_capacity);
    if (total_capacity <= 0) {
        SYNCDBG(8,"Need \"%s\" because do not have any",room_code_name(process->field_10));
        return 1;
    }
    long free_capacity;
    free_capacity = computer_get_room_kind_free_capacity(comp, process->field_10);
    if (free_capacity == 9999)
    {
        if (process->field_10 != RoK_GARDEN) {
            SYNCDBG(8,"Need \"%s\" because of undetermined capacity",room_code_name(process->field_10));
            return 1;
        }
    } else
    {
        // The "+1" is to better handle cases when the existing room is very small (capacity lower than 10)
        // On higher capacities it doesn't make much difference, but highly increases chance
        // of building new room if existing capacity is low.
        if (free_capacity <= 10*total_capacity/100 + 1) {
            SYNCDBG(8,"Need \"%s\" because of low free capacity",room_code_name(process->field_10));
            return 1;
        }
    }
    SYNCDBG(9,"Not building \"%s\" because free capacity is %d/%d",room_code_name(process->field_10),(int)free_capacity, (int)total_capacity);
    return 4;
}

PlayerNumber get_player_with_more_entrances_than_computer(const struct Computer2 *comp, int *max_entr_count)
{
    PlayerNumber max_plyr_idx;
    PlayerNumber plyr_idx;
    const struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    max_plyr_idx = -1;
    *max_entr_count = dungeon->room_slabs_count[RoK_ENTRANCE];
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        int entr_count;
        if (plyr_idx == dungeon->owner)
            continue;
        entr_count = count_entrances(comp, plyr_idx);
        if ((entr_count > 0) && (entr_count >= *max_entr_count))
        {
            *max_entr_count = entr_count;
            max_plyr_idx = plyr_idx;
        }
    }
    return max_plyr_idx;
}

/**
 * Does a check if there are entrances with specific properties.
 * @param comp
 * @return
 */
TbBool there_is_virgin_entrance_for_computer(const struct Computer2 *comp)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    long i;
    unsigned long k;
    i = game.entrance_room_id;
    k = 0;
    while (i != 0)
    {
        struct Room *room;
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_kind;
        // Per-room code
        if (((room->field_12[dungeon->owner] & 0x01) != 0) &&
          (room->owner != dungeon->owner))
        {
            return true;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return false;
}

long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct Dungeon *dungeon;
    SYNCDBG(18,"Starting");
    //return _DK_computer_check_dig_to_entrance(comp, process);
    int neutral_entrances;
    dungeon = comp->dungeon;
    neutral_entrances = count_entrances(comp, game.neutral_player_num);
    if (is_task_in_progress(comp, CTT_DigToEntrance)) {
        return 4;
    }
    if ((there_is_virgin_entrance_for_computer(comp)) &&
        (game.play_gameturn - process->field_34 < 2000))
    {
        return 4;
    }
    PlayerNumber better_plyr_idx;
    int better_entr_count;
    better_plyr_idx = get_player_with_more_entrances_than_computer(comp, &better_entr_count);
    int entr_count;
    entr_count = dungeon->room_slabs_count[RoK_ENTRANCE];
    if ((better_plyr_idx >= 0) && (better_entr_count > entr_count))
    {
        return 1;
    }
    if ((entr_count > 0) && (neutral_entrances/2 <= entr_count))
    {
        return 4;
    }
    long turns;
    int trn_mul, trn_div;
    trn_mul = process->field_8;
    turns = game.play_gameturn - process->field_34;
    if (turns >= trn_mul)
        turns = trn_mul;
    trn_div = neutral_entrances - entr_count;
    if (trn_div <= 0)
      trn_div = 1;
    return trn_mul / trn_div <= turns;
}

long computer_finds_nearest_entrance2(struct Computer2 *comp, struct Coord3d *pos, struct Room **retroom, short plyr_idx)
{
    return _DK_computer_finds_nearest_entrance2(comp, pos, retroom, plyr_idx);
}

long move_imp_to_dig_here(struct Computer2 *comp, struct Coord3d *pos, long a3)
{
    return _DK_move_imp_to_dig_here(comp, pos, a3);
}

TbBool right_time_to_choose_target_entrance(struct ComputerProcess *process, long neutral_entrances, long own_entrances, long targplyr_entrances)
{
    GameTurnDelta turns_to_capture, turns_delta;
    turns_to_capture = process->field_8;
    turns_delta = game.play_gameturn - process->field_34;
    if (turns_delta >= turns_to_capture)
      turns_delta = turns_to_capture;
    long entrances_div;
    entrances_div = neutral_entrances - own_entrances + targplyr_entrances;
    if (entrances_div <= 0)
        entrances_div = 1;
    return (turns_to_capture/entrances_div <= turns_delta);
}

long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    //return _DK_computer_setup_dig_to_entrance(comp, process);
    int i;
    PlayerNumber targplyr_idx;
    long targplyr_entrances;
    // Let's find a player with highest number of entrance rooms
    targplyr_idx = -1;
    targplyr_entrances = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      if (dungeon->owner == i) {
          continue;
      }
      long num_entrances;
      num_entrances = count_entrances(comp, i);
      if (num_entrances >= targplyr_entrances) {
          targplyr_idx = i;
          targplyr_entrances = num_entrances;
      }
    }
    // Count our own entrance rooms, and neutral ones
    long own_entrances, neutral_entrances;
    own_entrances = count_entrances(comp, dungeon->owner);
    neutral_entrances = count_entrances(comp, game.neutral_player_num);
    // Prepare for selecting entrance
    MapCoordDelta entdist;
    struct Room *entroom;
    struct Coord3d startpos;
    entroom = INVALID_ROOM;
    entdist = LONG_MAX;
    // Check if it's time to choose the entrance. That depends on how many entrances are claimed.
    if (right_time_to_choose_target_entrance(process, neutral_entrances, own_entrances, targplyr_entrances))
    {
        MapCoordDelta dist;
        struct Room *room;
        struct Coord3d pos;
        if ((targplyr_idx >= 0) && (own_entrances < targplyr_entrances))
        {
            room = INVALID_ROOM;
            dist = computer_finds_nearest_entrance2(comp, &pos, &room, targplyr_idx);
            if (dist > 0) {
                entdist = dist;
                entroom = room;
                startpos.x.val = subtile_coord_center(stl_slab_center_subtile(pos.x.stl.num));
                startpos.y.val = subtile_coord_center(stl_slab_center_subtile(pos.y.stl.num));
                startpos.z.val = pos.z.val;
            }
        }
        if (neutral_entrances > 0)
        {
            room = INVALID_ROOM;
            dist = computer_finds_nearest_entrance2(comp, &pos, &room, game.neutral_player_num);
            if (dist > 0)
            {
                struct Dungeon *targdngn;
                targdngn = get_players_num_dungeon(targplyr_idx);
                if ((dist < entdist) || (targdngn->num_active_creatrs > dungeon->num_active_creatrs)) {
                    entdist = dist;
                    entroom = room;
                    startpos.x.val = subtile_coord_center(stl_slab_center_subtile(pos.x.stl.num));
                    startpos.y.val = subtile_coord_center(stl_slab_center_subtile(pos.y.stl.num));
                    startpos.z.val = pos.z.val;
                }
            }
        }
    }
    // If no entrance was selected, that's all
    if (room_is_invalid(entroom)) {
        return 0;
    }
    // Set the end position
    struct Coord3d endpos;
    endpos.x.val = subtile_coord_center(stl_slab_center_subtile(entroom->central_stl_x));
    endpos.y.val = subtile_coord_center(stl_slab_center_subtile(entroom->central_stl_y));
    endpos.z.val = subtile_coord(1,0);
    // If we are supposed to dig there, then do it
    if (comp->field_20)
    {
        struct ComputerDig cdig;
        setup_dig_to(&cdig, startpos, endpos);
        long digres;
        do {
            digres = tool_dig_to_pos2(comp, &cdig, true, 0);
        } while (digres == 0);
        if ((digres != -1) && (digres != -5))
        {
            entroom->field_12[dungeon->owner] |= 0x02;
            return 0;
        }
    }
    // Now everything is ready - start the task
    struct ComputerTask *ctask;
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return 0;
    }
    ctask->ttype = CTT_DigToEntrance;
    ctask->flags |= ComTsk_Unkn0004;
    ctask->pos_70.x.val = startpos.x.val;
    ctask->pos_70.y.val = startpos.y.val;
    ctask->pos_70.z.val = startpos.z.val;
    ctask->pos_76.x.val = endpos.x.val;
    ctask->pos_76.y.val = endpos.y.val;
    ctask->pos_76.z.val = endpos.z.val;
    ctask->field_8C = computer_process_index(comp, process);
    ctask->word_80 = entroom->index;
    entroom->field_12[dungeon->owner] |= 0x01;
    // Setup the digging
    setup_dig_to(&ctask->dig, startpos, endpos);
    process->func_complete(comp, process);
    suspend_process(comp, process);
    move_imp_to_dig_here(comp, &startpos, 1);
    return 2;
}

long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct GoldLookup *gldlook;
    struct Dungeon *dungeon;
    struct ComputerTask *ctask;
    struct Coord3d startpos;
    struct Coord3d endpos;
    unsigned long dig_distance;
    unsigned long max_distance;
    long digres;
    SYNCDBG(18,"Starting");
    //return _DK_computer_setup_dig_to_gold(comp, process);
    dig_distance = 0;
    dungeon = comp->dungeon;
    gldlook = 0;
    digres = computer_finds_nearest_room_to_gold(comp, &startpos, &gldlook);
    if (digres == -1)
    {
        process->flags |= ComProc_Unkn0004;
        SYNCDBG(8,"Can't find nearest room to gold; will refresh gold map");
        return 0;
    }
    if (digres <= 0)
    {
        SYNCDBG(8,"Finding gold to dig didn't worked out");
        return 0;
    }
    max_distance = game.play_gameturn / process->field_C + process->field_14;
    if (digres > max_distance)
    {
        SYNCDBG(8,"Gold is out of distance (%lu > %lu)",digres,max_distance);
        return 4;
    }
    endpos.x.val = subtile_coord_center(stl_slab_center_subtile(gldlook->x_stl_num));
    endpos.y.val = subtile_coord_center(stl_slab_center_subtile(gldlook->y_stl_num));
    endpos.z.val = subtile_coord(1,0);
    startpos.x.val = subtile_coord_center(stl_slab_center_subtile(startpos.x.stl.num));
    startpos.y.val = subtile_coord_center(stl_slab_center_subtile(startpos.y.stl.num));
    startpos.z.val = subtile_coord(1,0);
    if ( comp->field_20 )
    {
        struct ComputerDig cdig;
        // Setup the digging on dummy ComputerDig, just to compute distance
        setup_dig_to(&cdig, startpos, endpos);
        while ( 1 )
        {
            digres = tool_dig_to_pos2(comp, &cdig, true, 1);
            if (digres != 0)
              break;
            dig_distance++;
        }
        if ( (digres != -1) && (digres != -5) )
        {
            SYNCDBG(8,"Dig evaluation didn't worked out, code %d",digres);
            gldlook->plyrfield_1[dungeon->owner] |= 0x02;
            return 0;
        }
        if (dig_distance > max_distance)
        {
            SYNCDBG(8,"Gold is out of evaluation distance (%lu > %lu)",digres,max_distance);
            return 0;
        }
        SYNCDBG(8,"Dig evaluation distance %d, result %d",dig_distance,digres);
    }
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        SYNCDBG(8,"No free task; won't dig");
        return 4;
    }
    ctask->ttype = CTT_DigToGold;
    ctask->flags |= ComTsk_Unkn0004;
    ctask->pos_70.x.val = startpos.x.val;
    ctask->pos_70.y.val = startpos.y.val;
    ctask->pos_70.z.val = startpos.z.val;
    ctask->pos_76.x.val = endpos.x.val;
    ctask->pos_76.y.val = endpos.y.val;
    ctask->pos_76.z.val = endpos.z.val;
    ctask->long_86 = process->field_10;
    ctask->field_8C = computer_process_index(comp, process);
    ctask->word_80 = gold_lookup_index(gldlook);
    gldlook->plyrfield_1[dungeon->owner] |= 0x01;
    // Setup the digging
    setup_dig_to(&ctask->dig, startpos, endpos);
    process->func_complete(comp, process);
    suspend_process(comp, process);
    comp->task_state = CTaskSt_Select;
    return 2;
}

/**
 * Check for gold digging.
 * This function address is compared in computer_check_for_money(); but it is already rewritten.
 */
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process)
{
    //return _DK_computer_check_dig_to_gold(comp, process);
    // If we have treasure room
    if ( !dungeon_has_room(comp->dungeon, RoK_TREASURE) )
    {
        SYNCDBG(18,"Computer player model %d won't dig for gold - no treasure room.",(int)comp->model);
        return 4;
    }
    // And we're lacking money
    if ( process->field_8 <= get_computer_money_less_cost(comp) )
    {
        SYNCDBG(18,"Computer player model %d won't dig for gold - has over %d gold.",(int)comp->model,(int)process->field_8);
        return 4;
    }
    // And we're not already digging for gold
    if (is_task_in_progress(comp, CTT_DigToGold)) {
        SYNCDBG(18,"Computer player model %d is already digging for gold.",(int)comp->model);
        return 4;
    }
    // Then do dig for gold
    SYNCDBG(8,"Computer player model %d is going to start digging for gold.",(int)comp->model);
    return 1;
}

long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct Dungeon *dungeon;
    //return _DK_computer_check_sight_of_evil(comp, process);
    dungeon = comp->dungeon;
    if (process->field_10 >= game.play_gameturn) {
        return 4;
    }
    long able;
    able = computer_able_to_use_magic(comp, 5, process->field_8, 5);
    if (able == 1) {
        if (dungeon->sight_casted_thing_idx >= 1u) {
            return 4;
        }
        return 1;
    }
    if (able != 0) {
        return 4;
    }
    process->flags |= 0x04;
    return 0;
}

long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
    //return _DK_computer_check_attack1(comp, process);
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    int max_crtrs;
    max_crtrs = dungeon->max_creatures_attracted;
    if (max_crtrs <= 0) {
        suspend_process(comp, process);
        return 4;
    }
    if ((100 * dungeon->num_active_creatrs / max_crtrs < process->field_10) || is_there_an_attack_task(comp)) {
        suspend_process(comp, process);
        return 4;
    }
    if (process->field_8 * count_creatures_availiable_for_fight(comp, 0) / 100 < process->field_C) {
        suspend_process(comp, process);
        return 4;
    }
    struct THate hates[PLAYERS_COUNT];
    get_opponent(comp, hates);
    long i;
    // note that 'i' is not player index, player index is inside THate struct
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct THate *hate;
        hate = &hates[i];
        if (hate->pos_near != NULL)
        {
            // If we can attack - do it, don't hesitate
            if (setup_computer_attack(comp, process, hate->pos_near, hate->plyr_idx) == 1) {
                hate->pos_near->x.val = 0;
                return 1;
            }
        }
    }
    suspend_process(comp, process);
    return 4;
}

long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process)
{
    //return _DK_computer_check_safe_attack(comp, process);
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    int max_crtrs;
    max_crtrs = dungeon->max_creatures_attracted;
    if (max_crtrs <= 0) {
        suspend_process(comp, process);
        return 4;
    }
    if ((100 * dungeon->num_active_creatrs / max_crtrs < process->field_10) || is_there_an_attack_task(comp)) {
        suspend_process(comp, process);
        return 4;
    }
    if (process->field_8 * count_creatures_availiable_for_fight(comp, 0) / 100 < process->field_C) {
        suspend_process(comp, process);
        return 4;
    }
    struct THate hates[PLAYERS_COUNT];
    get_opponent(comp, hates);
    long i;
    // note that 'i' is not player index, player index is inside THate struct
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct THate *hate;
        hate = &hates[i];
        if (hate->pos_near != NULL)
        {
            struct Dungeon *enmdngn;
            enmdngn = get_players_num_dungeon(hate->plyr_idx);
            if (enmdngn->num_active_creatrs * process->field_8 / 100 + enmdngn->num_active_creatrs < dungeon->num_active_creatrs)
            {
                if (setup_computer_attack(comp, process, hate->pos_near, hate->plyr_idx) == 1) {
                    hate->pos_near->x.val = 0;
                    return 1;
                }
            }
        }
    }
    suspend_process(comp, process);
    return 4;
}

long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_process_sight_of_evil(comp, process);
}

long computer_process_task(struct Computer2 *comp, struct ComputerProcess *process)
{
  return 0;
}

long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process)
{
  comp->task_state = CTaskSt_Select;
  return 0;
}

long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process)
{
    SYNCDBG(8,"Completed process \"%s\"",process->name);
    process->field_34 = game.play_gameturn;
    comp->task_state = CTaskSt_Select;
    return 0;
}

long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_completed_attack1(comp, process);
}

long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process)
{
    process->flags &= ~ComProc_Unkn0008;
    comp->task_state = CTaskSt_Select;
    return 0;
}

void shut_down_process(struct Computer2 *comp, struct ComputerProcess *process)
{
    Comp_Process_Func callback;
    if (process != NULL)
    {
        process->flags |= ComProc_Unkn0008;
        process->flags &= ~ComProc_Unkn0020;
        process->field_34 = game.play_gameturn;
        callback = process->func_complete;
        if (callback != NULL) {
            callback(comp, process);
        }
    }
}

struct ComputerProcess *get_computer_process(struct Computer2 *comp, int cproc_idx)
{
    if ((cproc_idx < 0) || (cproc_idx > COMPUTER_PROCESSES_COUNT)) {
        return NULL;//&comp->processes[0]
    }
    return &comp->processes[cproc_idx];
}

long computer_process_index(const struct Computer2 *comp, const struct ComputerProcess *process)
{
    long i;
    i = ((char *)process - (char *)&comp->processes[0]);
    if ( (i < 0) || (i > COMPUTER_PROCESSES_COUNT*sizeof(struct ComputerProcess)) ) {
        ERRORLOG("Process \"%s\" is outside of Computer Player.",process->name);
        return 0;
    }
    return i / sizeof(struct ComputerProcess);
}

void suspend_process(struct Computer2 *comp, struct ComputerProcess *process)
{
    if (process != NULL)
    {
        process->flags &= ~ComProc_Unkn0020;
        process->field_38 = 0;
        process->field_3C = game.play_gameturn;
        process->field_34 = game.play_gameturn;
    } else {
        WARNLOG("Invalid computer process referenced");
    }
}

void reset_process(struct Computer2 *comp, struct ComputerProcess *process)
{
  if (process != NULL)
  {
    process->field_3C = 0;
    process->field_38 = 0;
    process->flags &= ~ComProc_Unkn0020;
    process->field_34 = game.play_gameturn;
  }
}

struct ComputerProcess * find_best_process(struct Computer2 *comp)
{
    return _DK_find_best_process(comp);
}

long set_next_process(struct Computer2 *comp)
{
    struct ComputerProcess *process;
    long chkres;
    //return _DK_set_next_process(comp);
    chkres = 0;
    process = find_best_process(comp);
    if (process != NULL)
    {
        SYNCDBG(8,"Checking \"%s\"",process->name);
        chkres = process->func_check(comp, process);
        if (chkres == 1)
        {
            comp->ongoing_process = computer_process_index(comp, process); // This should give index of the process
            SYNCDBG(8,"Setting up process %d",(int)comp->ongoing_process);
            chkres = process->func_setup(comp, process);
            if ( chkres == 1 )
            {
                process->field_30 = game.play_gameturn;
                comp->task_state = CTaskSt_Perform;
            }
        }
        if (chkres == 4)
        {
            process->field_3C = game.play_gameturn;
            process->field_38 = 0;
        }
        if (chkres == 0)
        {
            process->field_3C = 0;
            process->field_38 = game.play_gameturn;
        }
    }
    if (chkres != 1)
    {
        SYNCDBG(17,"No new process");
        comp->ongoing_process = 0;
    } else
    {
        SYNCDBG(7,"Undertaking new process");
    }
    return chkres;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
