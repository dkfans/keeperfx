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

#define TRAPDOOR_TYPES_MAX 128

/******************************************************************************/
#pragma pack(1)
// TODO: join it with TrapConfigStats and DoorConfigStats and TrapStats
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
    TextStringId name_stridx;
    TextStringId tooltip_stridx;
    long panel_tab_idx;
    long bigsym_sprite_idx;
    long medsym_sprite_idx;
    long pointer_sprite_idx;
};

struct TrapConfigStats {
    char code_name[COMMAND_WORD_LEN];
    TextStringId name_stridx;
    TextStringId tooltip_stridx;
    long panel_tab_idx;
    long bigsym_sprite_idx;
    long medsym_sprite_idx;
    long pointer_sprite_idx;
    long hidden;
    long slappable;
    long notify;
};

/**
 * Manufacture types data.
 * Originally was named TrapData, but stores both traps and doors; now no longer matches original.
 */
struct ManufactureData {
      ThingClass tngclass; //< Thing class created when manufactured design is placed
      ThingModel tngmodel; //< Thing model created when manufactured design is placed
      long work_state; //< Work state used to place the manufactured item on map
      TextStringId tooltip_stridx;
      long panel_tab_idx;
      long bigsym_sprite_idx;
      long medsym_sprite_idx;
};

struct TrapDoorConfig {
    long trap_types_count;
    struct TrapConfigStats trap_cfgstats[TRAPDOOR_TYPES_MAX];
    long door_types_count;
    struct DoorConfigStats door_cfgstats[TRAPDOOR_TYPES_MAX];
    ThingModel trap_to_object[TRAPDOOR_TYPES_MAX];
    ThingModel door_to_object[TRAPDOOR_TYPES_MAX];
    long manufacture_types_count;
    /** Stores manufacturable items. Was originally named trap_data. */
    struct ManufactureData manufacture_data[2*TRAPDOOR_TYPES_MAX];
};
/******************************************************************************/
extern const char keeper_trapdoor_file[];
extern struct NamedCommand trap_desc[TRAPDOOR_TYPES_MAX];
extern struct NamedCommand door_desc[TRAPDOOR_TYPES_MAX];
/******************************************************************************/
TbBool load_trapdoor_config(const char *conf_fname,unsigned short flags);

struct TrapConfigStats* get_trap_model_stats(int tngmodel);
struct DoorConfigStats *get_door_model_stats(int tngmodel);
struct ManufactureData *get_manufacture_data(int manufctr_idx);
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel);

ThingModel door_crate_object_model(ThingModel tngmodel);
ThingModel trap_crate_object_model(ThingModel tngmodel);
const char *door_code_name(int tngmodel);
const char *trap_code_name(int tngmodel);
int door_model_id(const char * code_name);
int trap_model_id(const char * code_name);

TbBool is_trap_placeable(PlayerNumber plyr_idx, long trap_idx);
TbBool is_trap_buildable(PlayerNumber plyr_idx, long trap_idx);
TbBool is_trap_built(PlayerNumber plyr_idx, long tngmodel);
TbBool is_door_placeable(PlayerNumber plyr_idx, long door_idx);
TbBool is_door_buildable(PlayerNumber plyr_idx, long door_idx);
TbBool is_door_built(PlayerNumber plyr_idx, long door_idx);
TbBool create_manufacture_array_from_trapdoor_data(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
