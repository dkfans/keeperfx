/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_trapdoor.h
 *     Header file for config_trapdoor.c.
 * @par Purpose:
 *     Traps and doors configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 21 Dec 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGTRAPDOOR_H
#define DK_CFGTRAPDOOR_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define TRAPDOOR_ITEMS_MAX 256

/******************************************************************************/
#pragma pack(1)

struct ManfctrConfig { // sizeof=0x14
  int manufct_level;
  int manufct_required;
  int shots;
  int shots_delay;
  long selling_value;
};

#pragma pack()
/******************************************************************************/
struct DoorConfigStats {
    char code_name[COMMAND_WORD_LEN];
    long name_stridx;
};

struct TrapConfigStats {
    char code_name[COMMAND_WORD_LEN];
    long name_stridx;
    long tooltip_stridx;
};

struct TrapDoorConfig {
    long trap_types_count;
    struct TrapConfigStats trap_cfgstats[TRAPDOOR_ITEMS_MAX];
    long door_types_count;
    struct DoorConfigStats door_cfgstats[TRAPDOOR_ITEMS_MAX];
};
/******************************************************************************/
extern const char keeper_trapdoor_file[];
extern struct NamedCommand trap_desc[TRAPDOOR_ITEMS_MAX];
extern struct NamedCommand door_desc[TRAPDOOR_ITEMS_MAX];
/******************************************************************************/
TbBool load_trapdoor_config(const char *conf_fname,unsigned short flags);
struct TrapConfigStats *get_trap_model_stats(int tngmodel);
struct DoorConfigStats *get_door_model_stats(int tngmodel);
const char *door_code_name(int tngmodel);
const char *trap_code_name(int tngmodel);
int door_model_id(const char * code_name);
int trap_model_id(const char * code_name);
TbBool is_trap_placeable(long plyr_idx, long trap_idx);
TbBool is_trap_buildable(long plyr_idx, long trap_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
