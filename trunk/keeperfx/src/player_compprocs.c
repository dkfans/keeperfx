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
#include "room_list.h"
#include "creature_states.h"
#include "power_hand.h"

#include "gui_soundmsgs.h"
#include "dungeon_data.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_computer_look_for_opponent(struct Computer2 *comp, long stl_x, long stl_y, long a4);
/******************************************************************************/
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_process_task(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *cproc);
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
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    struct ComputerTask *ctask;
    long i;
    ctask = computer_setup_build_room(comp, cproc->confval_4, cproc->confval_2, cproc->confval_3, cproc->confval_5);
    if (!computer_task_invalid(ctask))
    {
        SYNCDBG(8,"Computer %d created task for \"%s\"",(int)comp->dungeon->owner,cproc->name);
        cproc->flags |= ComProc_Unkn0020;
        i = (long)((char *)cproc - (char *)&comp->processes[0]) / sizeof(struct ComputerProcess);
        if ((i < 0) || (i > COMPUTER_PROCESSES_COUNT))
        {
          ERRORLOG("Process \"%s\" is outside of Computer Player",cproc->name);
          i = COMPUTER_PROCESSES_COUNT;
        }
        ctask->field_8C = i;
        shut_down_process(comp, cproc);
        return 2;
    }
    if (cproc->confval_2 > cproc->confval_3)
    {
        if (cproc->confval_2 <= 2) {
            return 0;
        }
        cproc->confval_2--;
    } else
    {
        if (cproc->confval_3 <= 2) {
            return 0;
        }
        cproc->confval_3--;
    }
    reset_process(comp, cproc);
    return 2;
}

long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    struct ComputerTask *ctask;
    long i;
    ctask = computer_setup_build_room(comp, cproc->confval_4, cproc->confval_2, cproc->confval_3, cproc->confval_5);
    if (!computer_task_invalid(ctask))
    {
        SYNCDBG(8,"Computer %d created task for \"%s\"",(int)comp->dungeon->owner,cproc->name);
        cproc->flags |= ComProc_Unkn0020;
        i = (long)((char *)cproc - (char *)&comp->processes[0]) / sizeof(struct ComputerProcess);
        if ((i < 0) || (i > COMPUTER_PROCESSES_COUNT))
        {
          ERRORLOG("Process \"%s\" is outside of Computer Player",cproc->name);
          i = COMPUTER_PROCESSES_COUNT;
        }
        ctask->field_8C = i;
        shut_down_process(comp, cproc);
        cproc->flags &= ~ComProc_Unkn0008;
        return 2;
    }
    if (cproc->confval_2 > cproc->confval_3)
    {
        if (cproc->confval_2 <= 2) {
            return 0;
        }
        cproc->confval_2--;
    } else
    {
        if (cproc->confval_3 <= 2) {
            return 0;
        }
        cproc->confval_3--;
    }
    reset_process(comp, cproc);
    return 2;
}

long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    cproc->param_5++;
    if (cproc->confval_3 >= cproc->param_5) {
        return 1;
    }
    cproc->flags |= ComProc_Unkn0001;
    shut_down_process(comp, cproc);
    comp->task_state = 2;
    return 0;
}

long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *cproc)
{
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
                SYNCDBG(9,"Task %d is matching, type %d, building %s",(int)i,(int)ttype,room_code_name(ctask->rkind));
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

struct ComputerTask *get_room_build_task_nearest_to(const struct Computer2 *comp, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *retdist)
{
    struct ComputerTask *nearest_ctask;
    long nearest_dist;
    nearest_dist = LONG_MAX;
    nearest_ctask = INVALID_COMPUTER_TASK;
    struct ComputerTask *ctask;
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
        if (((ctask->flags & ComTsk_Unkn0001) != 0) && ((ctask->flags & ComTsk_Unkn0002) != 0))
        {
            long dist;
            dist = abs((MapSubtlCoord)ctask->pos_64.x.stl.num - stl_x) + abs((MapSubtlCoord)ctask->pos_64.y.stl.num - stl_y);
            if (dist < nearest_dist)
            {
                nearest_dist = dist;
                nearest_ctask = ctask;
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
    if (retdist != NULL)
        *retdist = nearest_dist;
    return nearest_ctask;
}

/**
 * Checks for rooms which the player doesn't have, and triggers building them.
 * @param comp
 * @param cproc
 */
long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *cproc)
{
#ifdef NEW_DIGBUILD
	return 4;
#endif
    struct Dungeon *dungeon;
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
                cproc->confval_4 = bldroom->rkind;
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

long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *cproc)
{
#ifdef NEW_DIGBUILD
	return 4;
#endif
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    ItemAvailability is_avail;
    is_avail = computer_check_room_available(comp, cproc->confval_4);
    if (is_avail != IAvail_Now)
    {
        if (is_avail == IAvail_Never) {
            cproc->flags |= ComProc_Unkn0004;
            return 0;
        }
        return 4;
    }
    long num_build_tasks = count_no_room_build_tasks(comp);
    if (num_build_tasks >= comp->max_room_build_tasks) {
        SYNCDBG(19,"Not building \"%s\" because already doing %d build tasks",room_code_name(cproc->confval_4),(int)num_build_tasks);
        return 4;
    }
    long used_capacity;
    long total_capacity;
    get_room_kind_total_and_used_capacity(dungeon, cproc->confval_4, &total_capacity, &used_capacity);
    if (total_capacity <= 0) {
        SYNCDBG(8,"Need \"%s\" because do not have any",room_code_name(cproc->confval_4));
        return 1;
    }
    long free_capacity;
    free_capacity = computer_get_room_kind_free_capacity(comp, cproc->confval_4);
    if (free_capacity == 9999)
    {
        if (cproc->confval_4 != RoK_GARDEN) {
            SYNCDBG(8,"Need \"%s\" because of undetermined capacity",room_code_name(cproc->confval_4));
            return 1;
        }
    } else
    {
        // The "+1" is to better handle cases when the existing room is very small (capacity lower than 10)
        // On higher capacities it doesn't make much difference, but highly increases chance
        // of building new room if existing capacity is low.
        if (free_capacity <= 10*total_capacity/100 + 1) {
            SYNCDBG(8,"Need \"%s\" because of low free capacity",room_code_name(cproc->confval_4));
            return 1;
        }
    }
    SYNCDBG(9,"Not building \"%s\" because free capacity is %d/%d",room_code_name(cproc->confval_4),(int)free_capacity, (int)total_capacity);
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
        if (((room->player_interested[dungeon->owner] & 0x01) != 0) &&
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

long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    struct Dungeon *dungeon;
    SYNCDBG(18,"Starting");
    int neutral_entrances;
    dungeon = comp->dungeon;
    neutral_entrances = count_entrances(comp, game.neutral_player_num);
    if (is_task_in_progress(comp, CTT_DigToEntrance)) {
        return 4;
    }
    if ((there_is_virgin_entrance_for_computer(comp)) &&
        (game.play_gameturn - cproc->param_2 < 2000))
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
    trn_mul = cproc->confval_2;
    turns = game.play_gameturn - cproc->param_2;
    if (turns >= trn_mul)
        turns = trn_mul;
    trn_div = neutral_entrances - entr_count;
    if (trn_div <= 0)
      trn_div = 1;
    return trn_mul / trn_div <= turns;
}

long computer_finds_nearest_entrance2(struct Computer2 *comp, struct Coord3d *startpos, struct Room **retroom, short from_plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (from_plyr_idx < 0)
        from_plyr_idx = game.neutral_player_num;
    struct Coord3d locpos;
    struct Room *near_entroom;
    long near_dist;
    struct Coord3d *near_startpos;
    near_entroom = NULL;
    near_startpos = NULL;
    near_dist = LONG_MAX;
    *retroom = NULL;
    unsigned long k;
    long i;
    k = 0;
    if (from_plyr_idx == game.neutral_player_num) {
        i = game.entrance_room_id;
    } else {
        struct Dungeon *fromdngn;
        fromdngn = get_dungeon(from_plyr_idx);
        i = fromdngn->room_kind[RoK_ENTRANCE];
    }
    while (i != 0)
    {
        struct Room *entroom;
        entroom = room_get(i);
        if (room_is_invalid(entroom))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        if (from_plyr_idx == game.neutral_player_num)
            i = entroom->next_of_kind;
        else
            i = entroom->next_of_owner;
        // Per-room code
        MapSubtlCoord from_stl_x, from_stl_y;
        from_stl_x = entroom->central_stl_x;
        from_stl_y = entroom->central_stl_y;
        if ((entroom->owner == from_plyr_idx) && ((entroom->player_interested[dungeon->owner] & 3) == 0))
        {
            long dist;
            struct Room *nearoom;
            nearoom = get_player_room_any_kind_nearest_to(dungeon->owner, from_stl_x, from_stl_y,  &dist);
            if (!room_is_invalid(nearoom) && (dist < near_dist)) {
                near_dist = dist;
                near_entroom = entroom;
                near_startpos = &locpos;
                locpos.x.val = subtile_coord_center(nearoom->central_stl_x);
                locpos.y.val = subtile_coord_center(nearoom->central_stl_y);
                locpos.z.val = subtile_coord(1,0);
            }
        }
        long n;
        n = comp->task_idx;
        while (n > 0)
        {
            struct ComputerTask *ctask;
            ctask = get_computer_task(n);
            n = ctask->next_task;
        }
        long dist;
        struct ComputerTask *ctask;
        ctask = get_room_build_task_nearest_to(comp, from_stl_x, from_stl_y, &dist);
        if (!computer_task_invalid(ctask) && (dist < near_dist)) {
            near_dist = dist;
            near_entroom = entroom;
            near_startpos = &ctask->pos_64;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    if (room_is_invalid(near_entroom)) {
        return 0;
    }
    *retroom = near_entroom;
    startpos->x.val = near_startpos->x.val;
    startpos->y.val = near_startpos->y.val;
    startpos->z.val = near_startpos->z.val;
    return near_dist;
}

TbBool imp_can_be_moved_to_dig(const struct Thing *creatng)
{
    CrtrStateId curr_state;
    curr_state = get_creature_state_besides_move(creatng);
    struct StateInfo *curr_stati;
    curr_stati = get_thing_state_info_num(curr_state);
    switch (curr_stati->state_type)
    {
      case CrStTyp_Work:
      case CrStTyp_Escape:
        switch (curr_state)
        {
          case CrSt_ImpArrivesAtMineGold:
          case CrSt_ImpDigsDirt:
          case CrSt_ImpMinesGold:
          case CrSt_ImpDropsGold:
          case CrSt_ImpPicksUpGoldPile:
          case CrSt_ImpArrivesAtReinforce:
          case CrSt_ImpReinforces:
              return true;
        }
        break;
      case CrStTyp_Idle:
      case CrStTyp_OwnNeeds:
      case CrStTyp_Sleep:
      case CrStTyp_Feed:
      case CrStTyp_Move:
          return true;
    }
    return false;
}

TbBool imp_can_be_moved_to_mine(const struct Thing *creatng)
{
    CrtrStateId curr_state;
    curr_state = get_creature_state_besides_move(creatng);
    struct StateInfo *curr_stati;
    curr_stati = get_thing_state_info_num(curr_state);
    switch (curr_stati->state_type)
    {
      case CrStTyp_Work:
      case CrStTyp_Escape:
        switch (curr_state)
        {
          case CrSt_ImpDigsDirt:
          case CrSt_ImpArrivesAtReinforce:
          case CrSt_ImpReinforces:
              return true;
        }
        break;
      case CrStTyp_Idle:
      case CrStTyp_OwnNeeds:
      case CrStTyp_Sleep:
      case CrStTyp_Feed:
      case CrStTyp_Move:
          return true;
    }
    return false;
}

long move_imp_to_dig_here(struct Computer2 *comp, struct Coord3d *pos, long max_amount)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;

    long amount_did;
    amount_did = 0;

    unsigned long k;
    int i;
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        const struct Thing *creatng;
        const struct CreatureControl *cctrl;
        creatng = thing_get(i);
        TRACE_THING(creatng);
        cctrl = creature_control_get_from_thing(creatng);
        if (thing_is_invalid(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (amount_did >= max_amount)
            break;
        if (can_thing_be_picked_up_by_player(creatng, dungeon->owner) && imp_can_be_moved_to_dig(creatng))
        {
            if (!create_task_move_creature_to_pos(comp, creatng, *pos, CrSt_ImpDigsDirt)) {
                break;
            }
            amount_did++;
        }
        // Thing list loop body ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return amount_did;
}

long move_imp_to_mine_here(struct Computer2 *comp, struct Coord3d *pos, long max_amount)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;

    long amount_did;
    amount_did = 0;

    unsigned long k;
    int i;
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        const struct Thing *creatng;
        const struct CreatureControl *cctrl;
        creatng = thing_get(i);
        TRACE_THING(creatng);
        cctrl = creature_control_get_from_thing(creatng);
        if (thing_is_invalid(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (amount_did >= max_amount)
            break;
        if (can_thing_be_picked_up_by_player(creatng, dungeon->owner) && imp_can_be_moved_to_mine(creatng))
        {
            if (!create_task_move_creature_to_pos(comp, creatng, *pos, CrSt_ImpMinesGold)) {
                break;
            }
            amount_did++;
        }
        // Thing list loop body ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return amount_did;
}

TbBool right_time_to_choose_target_entrance(struct ComputerProcess *cproc, long neutral_entrances, long own_entrances, long targplyr_entrances)
{
    GameTurnDelta turns_to_capture, turns_delta;
    turns_to_capture = cproc->confval_2;
    turns_delta = game.play_gameturn - cproc->param_2;
    if (turns_delta >= turns_to_capture)
      turns_delta = turns_to_capture;
    long entrances_div;
    entrances_div = neutral_entrances - own_entrances + targplyr_entrances;
    if (entrances_div <= 0)
        entrances_div = 1;
    return (turns_to_capture/entrances_div <= turns_delta);
}

long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
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
    if (right_time_to_choose_target_entrance(cproc, neutral_entrances, own_entrances, targplyr_entrances))
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
            entroom->player_interested[dungeon->owner] |= 0x02;
            return 0;
        }
    }
    long parent_cproc_idx;
    parent_cproc_idx = computer_process_index(comp, cproc);
    // Now everything is ready - start the task
    if (!create_task_dig_to_entrance(comp, startpos, endpos, parent_cproc_idx, entroom->index)) {
        return 0;
    }
    entroom->player_interested[dungeon->owner] |= 0x01;
    cproc->func_complete(comp, cproc);
    suspend_process(comp, cproc);
    move_imp_to_dig_here(comp, &startpos, 1);
    return 2;
}

long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    struct GoldLookup *gldlook;
    struct Dungeon *dungeon;
    struct Coord3d startpos;
    struct Coord3d endpos;
    unsigned long dig_distance;
    unsigned long max_distance;
    long digres;
    SYNCDBG(18,"Starting");
    dig_distance = 0;
    dungeon = comp->dungeon;
    gldlook = NULL;
    digres = computer_finds_nearest_room_to_gold(comp, &startpos, &gldlook);
    if (digres == -1)
    {
        cproc->flags |= ComProc_Unkn0004;
        SYNCDBG(8,"Can't find nearest room to gold; will refresh gold map");
        return 0;
    }
    if (digres <= 0)
    {
        SYNCDBG(8,"Finding gold to dig didn't worked out");
        return 0;
    }
    max_distance = game.play_gameturn / cproc->confval_3 + cproc->confval_5;
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
        // Setup the digging on dummy ComputerDig, to compute distance and move start position near to wall
        setup_dig_to(&cdig, startpos, endpos);
        while ( 1 )
        {
            digres = tool_dig_to_pos2(comp, &cdig, true, 1);
            if (digres != 0)
              break;
            //TODO COMPUTER_PLAYER If the slab we've got from digging belongs to our player, use it as starting position
            // But don't change distance - it should be computed from our rooms
            dig_distance++;
        }
        if ( (digres != -1) && (digres != -5) )
        {
            SYNCDBG(8,"Dig evaluation didn't worked out, code %d",digres);
            gldlook->player_interested[dungeon->owner] |= 0x02;
            return 0;
        }
        if (dig_distance > max_distance)
        {
            SYNCDBG(8,"Gold is out of evaluation distance (%lu > %lu)",dig_distance,max_distance);
            return 0;
        }
        SYNCDBG(8,"Dig evaluation distance %lu, result %d",dig_distance,digres);
    }
    long parent_cproc_idx;
    long gold_lookup_idx;
    parent_cproc_idx = computer_process_index(comp, cproc);
    gold_lookup_idx = gold_lookup_index(gldlook);
    if (!create_task_dig_to_gold(comp, startpos, endpos, parent_cproc_idx, cproc->confval_4, gold_lookup_idx)) {
        SYNCDBG(8,"No free task; won't dig");
        return 4;
    }
    gldlook->player_interested[dungeon->owner] |= 0x01;
    cproc->func_complete(comp, cproc);
    suspend_process(comp, cproc);
    comp->task_state = CTaskSt_Select;
    return 2;
}

/**
 * Check for gold digging.
 *
 * This function address is compared in computer_check_for_money(); but it is already rewritten.
 */
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *cproc)
{
#ifdef NEW_DIGBUILD
	return 4;
#endif
    // If we have treasure room
    if (!dungeon_has_room(comp->dungeon, RoK_TREASURE))
    {
        SYNCDBG(18,"Computer player model %d won't dig for gold - no treasure room.",(int)comp->model);
        return 4;
    }
    // And we're lacking money
    if (cproc->confval_2 <= get_computer_money_less_cost(comp))
    {
        SYNCDBG(18,"Computer player model %d won't dig for gold - has over %d gold.",(int)comp->model,(int)cproc->confval_2);
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

long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (cproc->confval_4 >= game.play_gameturn) {
        return 4;
    }
    long able;
    able = computer_able_to_use_magic(comp, PwrK_SIGHT, cproc->confval_2, 5);
    if (able == 1) {
        if (dungeon->sight_casted_thing_idx > 0) {
            return 4;
        }
        return 1;
    }
    if (able != 0) {
        return 4;
    }
    cproc->flags |= ComProc_Unkn0004;
    return 0;
}

TbBool hate_filter_any_enemy_no_matter_how_strong(const struct Computer2 *comp, const struct ComputerProcess *cproc, const struct THate *hate)
{
    if (!players_are_enemies(comp->dungeon->owner, hate->plyr_idx))
        return false;
    return true;
}

TbBool hate_filter_enemy_with_not_many_creatures(const struct Computer2 *comp, const struct ComputerProcess *cproc, const struct THate *hate)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (!players_are_enemies(dungeon->owner, hate->plyr_idx))
        return false;
    struct Dungeon *enmdngn;
    enmdngn = get_players_num_dungeon(hate->plyr_idx);
    return (enmdngn->num_active_creatrs * cproc->confval_2 / 100 + enmdngn->num_active_creatrs < dungeon->num_active_creatrs);
}

long computer_check_attack_with_filter(struct Computer2 *comp, struct ComputerProcess *cproc, Comp_HateTest_Func hate_filter)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    SYNCDBG(8,"Starting for player %d",(int)dungeon->owner);
    int max_crtrs;
    max_crtrs = dungeon->max_creatures_attracted;
    if (max_crtrs <= 0) {
        suspend_process(comp, cproc);
        return 4;
    }
    if (100 * dungeon->num_active_creatrs / max_crtrs < cproc->confval_4) {
        SYNCDBG(7,"Not enough active creatures to fight, suspending");
        suspend_process(comp, cproc);
        return 4;
    }
    if (is_there_an_attack_task(comp)) {
        SYNCDBG(7,"Attack task already exists, suspending");
        suspend_process(comp, cproc);
        return 4;
    }
    if (cproc->confval_2 * count_creatures_availiable_for_fight(comp, 0) / 100 < cproc->confval_3) {
        SYNCDBG(7,"Not enough available creatures to fight, suspending");
        suspend_process(comp, cproc);
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
            if (hate_filter(comp, cproc, hate))
            {
                if (setup_computer_attack(comp, cproc, hate->pos_near, hate->plyr_idx) == 1) {
                    SYNCLOG("Player %d decided to attack player %d",(int)dungeon->owner,(int)hate->plyr_idx);
                    hate->pos_near->x.val = 0;
                    return 1;
                }
            }
        }
    }
    suspend_process(comp, cproc);
    return 4;
}

long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    return computer_check_attack_with_filter(comp, cproc, hate_filter_any_enemy_no_matter_how_strong);
}

long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    return computer_check_attack_with_filter(comp, cproc, hate_filter_enemy_with_not_many_creatures);
}

long computer_look_for_opponent(struct Computer2 *comp, long stl_x, long stl_y, long a4)
{
   return _DK_computer_look_for_opponent(comp, stl_x, stl_y, a4);
}

long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    int range;

    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (comp->tasks_did <= 0) {
        return 4;
    }
    // Compute range from power level
    range = 12 * cproc->confval_2;

    MapSubtlCoord stl_x, stl_y;
    {
#define GRID COMPUTER_SOE_GRID_SIZE
        MapSlabCoord slb_x, slb_y;
        slb_x = map_tiles_x/2;
        slb_y = map_tiles_y/2;
        struct SlabMap *slb;
        int n;
        n = ACTION_RANDOM(GRID*GRID);
        unsigned int grid_x, grid_y;
        int i;
        for (i=0; i < GRID*GRID; i++)
        {
            grid_x = n % GRID;
            grid_y = n / GRID;
            if ((comp->soe_targets[grid_y] & (1 << grid_x)) == 0)
            {
                slb_x = (unsigned long)map_tiles_x * grid_x / GRID + map_tiles_x/(2*GRID);
                slb_y = (unsigned long)map_tiles_y * grid_y / GRID + map_tiles_y/(2*GRID);
                comp->soe_targets[grid_y] |= (1 << grid_x);
                slb = get_slabmap_block(slb_x, slb_y);
                if ((slabmap_owner(slb) != dungeon->owner) && (slb->kind != SlbT_ROCK)) {
                    break;
                }
            }
            n = (n + 1) % (GRID*GRID);
        }
        if (i == GRID*GRID) {
            cproc->flags |= ComProc_Unkn0004;
            return 3;
        }
        stl_x = slab_subtile_center(slb_x);
        stl_y = slab_subtile_center(slb_y);
#undef GRID
    }
    if (try_game_action(comp, dungeon->owner, GA_UsePwrSight, cproc->confval_2, stl_x, stl_y, 0, 0) > Lb_OK)
    {
        computer_look_for_opponent(comp, stl_x, stl_y, range);
    }
    cproc->func_complete(comp, cproc);
    suspend_process(comp, cproc);
    return 1;
}

long computer_process_task(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    return 0;
}

long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    comp->task_state = CTaskSt_Select;
    return 0;
}

long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    SYNCDBG(8,"Completed process \"%s\"",cproc->name);
    cproc->param_2 = game.play_gameturn;
    comp->task_state = CTaskSt_Select;
    return 0;
}

long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    int creatrs_num;
    creatrs_num = cproc->confval_2 * dungeon->num_active_creatrs / 100;
    comp->task_state = 2;
    struct ComputerTask *ctask;
    ctask = get_computer_task(cproc->param_5);
    struct Coord3d  *pos;
    pos = &ctask->dig.pos_begin;
    long par1;
    par1 = ctask->pickup_for_attack.long_86;
    if (xy_walkable(pos->x.stl.num, pos->y.stl.num, dungeon->owner))
    {
        if (!create_task_pickup_for_attack(comp, pos, par1, creatrs_num)) {
            return 4;
        }
        return 1;
    } else
    if (cproc->confval_3 <= creatrs_num)
    {
        if ((computer_able_to_use_magic(comp, PwrK_CALL2ARMS, 5, 2) == 1) && check_call_to_arms(comp))
        {
            if (!create_task_magic_support_call_to_arms(comp, pos, 2500, par1, creatrs_num)) {
                return 4;
            }
            return 1;
        }
    }
    return 4;
}

long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    cproc->flags &= ~ComProc_Unkn0008;
    comp->task_state = CTaskSt_Select;
    return 0;
}

void shut_down_process(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    Comp_Process_Func callback;
    if (cproc != NULL)
    {
        cproc->flags |= ComProc_Unkn0008;
        cproc->flags &= ~ComProc_Unkn0020;
        cproc->param_2 = game.play_gameturn;
        callback = cproc->func_complete;
        if (callback != NULL) {
            callback(comp, cproc);
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

long computer_process_index(const struct Computer2 *comp, const struct ComputerProcess *cproc)
{
    long i;
    i = ((char *)cproc - (char *)&comp->processes[0]);
    if ( (i < 0) || (i > COMPUTER_PROCESSES_COUNT*sizeof(struct ComputerProcess)) ) {
        ERRORLOG("Process \"%s\" is outside of Computer Player.",cproc->name);
        return 0;
    }
    return i / sizeof(struct ComputerProcess);
}

void suspend_process(struct Computer2 *comp, struct ComputerProcess *cproc)
{
    if (cproc != NULL)
    {
        cproc->flags &= ~ComProc_Unkn0020;
        cproc->param_3 = 0;
        cproc->last_run_turn = game.play_gameturn;
        cproc->param_2 = game.play_gameturn;
    } else {
        WARNLOG("Invalid computer process referenced");
    }
}

void reset_process(struct Computer2 *comp, struct ComputerProcess *cproc)
{
  if (cproc != NULL)
  {
    cproc->last_run_turn = 0;
    cproc->param_3 = 0;
    cproc->flags &= ~ComProc_Unkn0020;
    cproc->param_2 = game.play_gameturn;
  }
}

struct ComputerProcess * find_best_process(struct Computer2 *comp)
{
    long best_prior;
    struct ComputerProcess *best_cproc;
    best_cproc = INVALID_COMPUTER_PROCESS;
    best_prior = LONG_MIN;
    // Computer players without heart can't start any process
    if (dungeon_invalid(comp->dungeon) || !player_has_heart(comp->dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)comp->dungeon->owner);
        return best_cproc;
    }
    GameTurnDelta g2max_prior;
    struct ComputerProcess *g2max_cproc;
    g2max_cproc = INVALID_COMPUTER_PROCESS;
    g2max_prior = 150;

    GameTurnDelta g1max_prior;
    struct ComputerProcess *g1max_cproc;
    g1max_cproc = INVALID_COMPUTER_PROCESS;
    g1max_prior = 100;

    int i;
    for (i=0; i < COMPUTER_PROCESSES_COUNT+1; i++)
    {
        struct ComputerProcess *cproc;
        cproc = &comp->processes[i];
        if ((cproc->flags & ComProc_Unkn0002) != 0)
            break;
        if ((cproc->flags & (ComProc_Unkn0020|ComProc_Unkn0010|ComProc_Unkn0008|ComProc_Unkn0004|ComProc_Unkn0001)) != 0)
            continue;
        if (cproc->last_run_turn > 0)
        {
            GameTurnDelta prior;
            prior = (GameTurnDelta)game.play_gameturn - (GameTurnDelta)cproc->last_run_turn;
            if (g1max_prior < prior) {
                g1max_prior = prior;
                g1max_cproc = cproc;
            }
        } else
        if (cproc->param_3 > 0)
        {
            GameTurnDelta prior;
            prior = (GameTurnDelta)game.play_gameturn - (GameTurnDelta)cproc->param_3;
            if (g2max_prior < prior) {
                g2max_prior = prior;
                g2max_cproc = cproc;
            }
        } else
        {
            if (best_prior < cproc->priority) {
                best_prior = cproc->priority;
                best_cproc = cproc;
            }
        }
    }

    if (g1max_cproc != NULL)
    {
        if (best_cproc == NULL)
        {
            best_cproc = g1max_cproc;
        } else
        if (best_cproc->priority < g1max_cproc->priority)
        {
            best_cproc = g1max_cproc;
        }
    }
    if (g2max_cproc != NULL)
    {
        if (best_cproc == NULL)
        {
            best_cproc = g2max_cproc;
        } else
        if (best_cproc->priority < g2max_cproc->priority)
        {
            best_cproc = g2max_cproc;
        }
    }
    return best_cproc;
}

long set_next_process(struct Computer2 *comp)
{
    struct ComputerProcess *cproc;
    long chkres;
    chkres = 0;
    cproc = find_best_process(comp);
    if (cproc != INVALID_COMPUTER_PROCESS)
    {
        SYNCDBG(8,"Checking \"%s\" for player %d",cproc->name,(int)comp->dungeon->owner);
        chkres = cproc->func_check(comp, cproc);
        if (chkres == 1)
        {
            comp->ongoing_process = computer_process_index(comp, cproc); // This should give index of the process
            SYNCDBG(8,"Setting up process %d",(int)comp->ongoing_process);
            chkres = cproc->func_setup(comp, cproc);
            if ( chkres == 1 )
            {
                cproc->param_1 = game.play_gameturn;
                comp->task_state = CTaskSt_Perform;
            }
        }
        if (chkres == 4)
        {
            cproc->last_run_turn = game.play_gameturn;
            cproc->param_3 = 0;
        }
        if (chkres == 0)
        {
            cproc->last_run_turn = 0;
            cproc->param_3 = game.play_gameturn;
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
