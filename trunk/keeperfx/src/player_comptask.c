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

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_memory.h"

#include "config.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
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
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
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
    SYNCDBG(9,"Starting");
    return _DK_task_move_creature_to_room(comp,ctask);
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
    SYNCDBG(9,"Starting");
    return _DK_task_attack_magic(comp,ctask);
}

long task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_sell_traps_and_doors(comp,ctask);
}

long process_tasks(struct Computer2 *comp)
{
    //return _DK_process_tasks(comp);
    struct ComputerTask *ctask;
    long ndone;
    long i,n;
    unsigned long k;
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
            n = ctask->field_2;
            if ((n > 0) && (n < sizeof(task_function)/sizeof(task_function[0])))
            {
                SYNCDBG(12,"Computer Task State %ld",n);
                task_function[n].func(comp, ctask);
                ndone++;
            } else
            {
                ERRORLOG("Bad Computer Task State %ld",n);
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
