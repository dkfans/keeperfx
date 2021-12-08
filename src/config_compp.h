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
#include "player_computer.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct ComputerProcessTypes {
  char *name;
  long field_4;
  long field_8;
  long field_C;
  long max_room_build_tasks;
  long field_14;
  long sim_before_dig;
  long field_1C;
  struct ComputerProcess *processes[COMPUTER_PROCESSES_COUNT];
  struct ComputerCheck checks[COMPUTER_CHECKS_COUNT];
  struct ComputerEvent events[COMPUTER_EVENTS_COUNT];
  long field_460;
};

#pragma pack()
/******************************************************************************/
/******************************************************************************/
struct ComputerProcessTypes *get_computer_process_type_template(long cpt_idx);
TbBool load_computer_player_config(unsigned short flags);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

extern struct ComputerPlayerConfig comp_player_conf;

#endif
