/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_compp.h
 *     Header file for config_compp.c.
 * @par Purpose:
 *     Computer player configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     25 May 2009 - 06 Jan 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGCOMPP_H
#define DK_CFGCOMPP_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define COMPUTER_TASKS_COUNT        100
#define COMPUTER_PROCESSES_COUNT     20
#define COMPUTER_CHECKS_COUNT        32
#define COMPUTER_EVENTS_COUNT        33

#define COMPUTER_PROCESS_TYPES_COUNT 64
#define COMPUTER_CHECKS_TYPES_COUNT  64
#define COMPUTER_EVENTS_TYPES_COUNT  64
#define COMPUTER_MODELS_COUNT        64

#define COMPUTER_ASSIST_TYPES_COUNT 4


typedef unsigned char ComputerType;

struct ComputerProcess {
  char name[COMMAND_WORD_LEN];
  char mnemonic[COMMAND_WORD_LEN];
  long priority;
  // Signed process config values
  long confval_2;
  long confval_3;
  long confval_4; /**< room kind or amount of creatures or gameturn or count of slabs */
  long confval_5;
  FuncIdx func_check;
  FuncIdx func_setup;
  FuncIdx func_task;
  FuncIdx func_complete;
  FuncIdx func_pause;
  unsigned char parent;
  // Unsigned process parameters storage (stores gameturns)
  unsigned long param_1;
  unsigned long param_2;
  unsigned long param_3;
  unsigned long last_run_turn;
  // Signed process parameters storage
  long param_5;
  unsigned long flags; /**< Values from ComProc_* enumeration. */
};

struct ComputerCheck {
  char name[COMMAND_WORD_LEN];
  char mnemonic[COMMAND_WORD_LEN];
  unsigned long flags; /**< Values from ComChk_* enumeration. */
  long turns_interval;
  FuncIdx func;
  long param1;
  long param2;
  long param3;
  long last_run_turn;
};

struct ComputerEvent {
  char name[COMMAND_WORD_LEN];
  char mnemonic[COMMAND_WORD_LEN];
  unsigned long cetype;
  unsigned long mevent_kind;
  FuncIdx func_event;
  FuncIdx func_test;
  long test_interval;
  unsigned char process;
  long param1;
  long param2;
  long param3;
  long last_test_gameturn; /**< event last checked time */
};

struct ComputerType {
  char name[COMMAND_WORD_LEN];
  short tooltip_stridx;
  short sprite_idx;
  long dig_stack_size;
  long processes_time;
  long click_rate;
  long max_room_build_tasks;
  long turn_begin;
  long sim_before_dig;
  GameTurnDelta drop_delay;
  unsigned char processes[COMPUTER_PROCESSES_COUNT];
  unsigned char checks[COMPUTER_CHECKS_COUNT];
  unsigned char events[COMPUTER_EVENTS_COUNT];
};

struct ComputerPlayerConfig {
  long processes_count;
  struct ComputerProcess process_types[COMPUTER_PROCESS_TYPES_COUNT];
  long checks_count;
  struct ComputerCheck check_types[COMPUTER_CHECKS_TYPES_COUNT];
  long events_count;
  struct ComputerEvent event_types[COMPUTER_EVENTS_TYPES_COUNT];
  long computers_count;
  struct ComputerType computer_types[COMPUTER_MODELS_COUNT];
  long skirmish_first;
  long skirmish_last;
  long player_assist_default;
  ComputerType computer_assist_types[COMPUTER_ASSIST_TYPES_COUNT];

};

#pragma pack()
/******************************************************************************/
/******************************************************************************/
struct ComputerType *get_computer_type_template(long cpt_idx);
TbBool load_computer_player_config(unsigned short flags);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

extern struct ComputerPlayerConfig comp_player_conf;

#endif
