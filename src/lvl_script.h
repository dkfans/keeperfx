/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.h
 *     Header file for lvl_script.c.
 * @par Purpose:
 *     Level script commands support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LVLSCRIPT_H
#define DK_LVLSCRIPT_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"
#include "config_rules.h"
#include "creature_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define PARTY_TRIGGERS_COUNT     256
#define CREATURE_PARTYS_COUNT    256
#define CONDITIONS_COUNT         255
#define TUNNELLER_TRIGGERS_COUNT 256
#define SCRIPT_VALUES_COUNT      256
#define WIN_CONDITIONS_COUNT      4

#define CONDITION_ALWAYS (CONDITIONS_COUNT)

#define PARTY_TRIGGERS_COUNT_OLD     48
#define CREATURE_PARTYS_COUNT_OLD    16
#define CONDITIONS_COUNT_OLD         48
#define TUNNELLER_TRIGGERS_COUNT_OLD 16
#define SCRIPT_VALUES_COUNT_OLD      64
#define WIN_CONDITIONS_COUNT_OLD      4

#define SENSIBLE_GOLD 99999999




enum MapLocationTypes {
    MLoc_NONE = 0,
    MLoc_ACTIONPOINT,
    MLoc_HEROGATE,
    MLoc_PLAYERSHEART,
    MLoc_CREATUREKIND, // 4
    MLoc_OBJECTKIND,
    MLoc_ROOMKIND,
    MLoc_THING,
    MLoc_PLAYERSDUNGEON, // 8
    MLoc_APPROPRTDUNGEON,
    MLoc_DOORKIND,
    MLoc_TRAPKIND,
    MLoc_METALOCATION, // 12 // Triggered box, Combat, Last entered creature etc
};

enum MetaLocation {
  MML_LAST_EVENT = 1,
  MML_RECENT_COMBAT,
};

enum ScriptOperator {
    SOpr_SET = 1,
    SOpr_INCREASE,
    SOpr_DECREASE,
    SOpr_MULTIPLY,
};

enum {
    CurrentPlayer = 15
};

/******************************************************************************/
#pragma pack(1)

struct Condition;
struct Party;
typedef unsigned long TbMapLocation;
struct ScriptLine;
struct ScriptValue;
struct PartyTrigger;

struct ScriptContext
{
    int plr_start;
    int plr_end;
    int player_idx;

    union {
      struct ScriptValue *value;
      struct PartyTrigger *pr_trig;
    };
};

struct TunnellerTrigger { // sizeof = 18
  unsigned char flags;
  unsigned char condit_idx;
  unsigned char plyr_idx;
  unsigned long location;
  unsigned char heading_OLD;//no longer used
  unsigned long heading; // originally was 'target'
  long carried_gold;
  unsigned char crtr_level;
  char party_id;
};

struct PartyTrigger { // sizeof = 13
  unsigned char flags;
  unsigned char condit_idx;
  char creatr_id;
  union
  {
      unsigned char plyr_idx;
      char party_id; // for add_to_party
  };
  union
  {
      unsigned long location;
      unsigned long countdown;
  };
  unsigned char crtr_level;
  unsigned short carried_gold;
  union
  {
      unsigned short ncopies;
      unsigned char objectv;
  };
};

struct ScriptValue { // sizeof = 16
  unsigned char flags;
  unsigned char condit_idx;
  unsigned char valtype;
  unsigned char plyr_range;
  union
  {
    struct
    {
      long arg0;
      long arg1;
      union
      {
          long arg2;
          char* str2;
      };
    };
    struct
    {
      unsigned long uarg0;
      unsigned long uarg1;
      union
      {
          unsigned long uarg2;
          unsigned char* ustr2;
      };
    };
    struct
    {
        char action;
        char param;
        char victims[MAX_SACRIFICE_VICTIMS];
    } sac;
    unsigned char bytes[12];
    char chars[12];
    short shorts[6];
  };
};

struct Condition { // sizeof = 12
  short condit_idx;
  unsigned char status;
  unsigned char plyr_range;
  unsigned char variabl_type;
  unsigned short variabl_idx;
  unsigned char operation;
  unsigned long rvalue;
};

struct PartyMember { // sizeof = 13
  unsigned char flags;
  unsigned char field_65;
  unsigned char crtr_kind;
  unsigned char objectv;
  long countdown;
  unsigned char crtr_level;
  unsigned short carried_gold;
  unsigned short field_6F;
};


struct Party { // sizeof = 208
  char prtname[100];
  struct PartyMember members[GROUP_MEMBERS_COUNT];
  unsigned long members_num;
};

struct ScriptFxLine
{
    int used;
    struct Coord3d from;
    struct Coord3d here;
    struct Coord3d to;

    int cx, cy; // midpoint

    int curvature;
    int spatial_step;
    int steps_per_turn;
    int partial_steps;
    int effect;

    int total_steps;
    int step;
};

struct LevelScriptOld { // sizeof = 5884
    struct TunnellerTrigger tunneller_triggers[TUNNELLER_TRIGGERS_COUNT_OLD];
    unsigned long tunneller_triggers_num;
    struct PartyTrigger party_triggers[PARTY_TRIGGERS_COUNT_OLD];
    unsigned long party_triggers_num;
    struct ScriptValue values[SCRIPT_VALUES_COUNT_OLD];
    unsigned long values_num;
    struct Condition conditions[CONDITIONS_COUNT_OLD];
    unsigned long conditions_num;
    struct Party creature_partys[CREATURE_PARTYS_COUNT_OLD];
    unsigned long creature_partys_num;
    unsigned short win_conditions[WIN_CONDITIONS_COUNT_OLD];
    unsigned long win_conditions_num;
    unsigned short lose_conditions[WIN_CONDITIONS_COUNT_OLD];
    unsigned long lose_conditions_num;
};

struct LevelScript {
    struct TunnellerTrigger tunneller_triggers[TUNNELLER_TRIGGERS_COUNT];
    unsigned long tunneller_triggers_num;
    struct PartyTrigger party_triggers[PARTY_TRIGGERS_COUNT];
    unsigned long party_triggers_num;
    struct ScriptValue values[SCRIPT_VALUES_COUNT];
    unsigned long values_num;
    struct Condition conditions[CONDITIONS_COUNT];
    unsigned long conditions_num;
    struct Party creature_partys[CREATURE_PARTYS_COUNT];
    unsigned long creature_partys_num;
    unsigned short win_conditions[WIN_CONDITIONS_COUNT];
    unsigned long win_conditions_num;
    unsigned short lose_conditions[WIN_CONDITIONS_COUNT];
    unsigned long lose_conditions_num;

    // Store strings used at level here
    char strings[2048];
    char *next_string;
};

/******************************************************************************/
// DLLIMPORT short _DK_script_current_condition;
// #define script_current_condition _DK_script_current_condition
DLLIMPORT unsigned long _DK_script_line_number;
DLLIMPORT unsigned char _DK_next_command_reusable;
#define next_command_reusable _DK_next_command_reusable
DLLIMPORT unsigned short _DK_condition_stack_pos;
#define condition_stack_pos _DK_condition_stack_pos
DLLIMPORT unsigned short _DK_condition_stack[48];
#define condition_stack _DK_condition_stack

#pragma pack()
/******************************************************************************/

extern const struct NamedCommand player_desc[];
/******************************************************************************/
TbBool script_support_setup_player_as_computer_keeper(PlayerNumber plyridx, long comp_model);
TbBool script_support_setup_player_as_zombie_keeper(unsigned short plyridx);
long script_scan_line(char *line,TbBool preloaded);
const struct CommandDesc *get_next_word(char **line, char *param, int *para_level, const struct CommandDesc *cmdlist_desc);
const char *script_get_command_name(long cmnd_index);

void command_add_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4);
void command_message(const char *msgtext, unsigned char kind);
unsigned short get_map_location_type(TbMapLocation location);
unsigned long get_map_location_longval(TbMapLocation location);
unsigned long get_map_location_plyrval(TbMapLocation location);
unsigned short get_map_location_plyridx(TbMapLocation location);
TbBool get_map_location_code_name(TbMapLocation location, char *name);
TbBool get_coords_at_meta_action(struct Coord3d *pos, PlayerNumber target_plyr_idx, long i);

short clear_script(void);
short load_script(long lvl_num);
short preload_script(long lvnum);
/******************************************************************************/
void script_process_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4, struct ScriptValue *value);
void script_process_win_game(PlayerNumber plyr_idx);
void script_process_lose_game(PlayerNumber plyr_idx);
struct Thing *script_process_new_tunneler(unsigned char plyr_idx, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold);
struct Thing *script_process_new_party(struct Party *party, PlayerNumber plyr_idx, TbMapLocation location, long copies_num);
void script_process_new_tunneller_party(PlayerNumber plyr_idx, long prty_id, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold);
long script_support_create_thing_at_hero_door(long gate_num, ThingClass tngclass, ThingModel tngmodel, unsigned char tngowner, unsigned char random_factor);
long script_support_create_thing_at_action_point(long apt_idx, ThingClass tngclass, ThingModel tngmodel, PlayerNumber tngowner, unsigned char random_factor);
long script_support_create_thing_at_dungeon_heart(ThingClass tngclass, ThingModel tngmodel, PlayerNumber tngowner, PlayerNumber plyr_idx);
struct Thing *script_create_new_creature(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long carried_gold, long crtr_level);
TbBool process_activation_status(struct Condition *condt);
TbBool get_condition_status(unsigned char opkind, long val1, long val2);
TbBool condition_inactive(long cond_idx);
TbBool action_point_activated_by_player(ActionPointId apt_idx, PlayerNumber plyr_idx);
void process_conditions(void);
void process_values(void);
void process_win_and_lose_conditions(PlayerNumber plyr_idx);
void script_process_new_creatures(PlayerNumber plyr_idx, long crtr_breed, long location, long copies_num, long carried_gold, long crtr_level);
void process_check_new_creature_partys(void);
void process_check_new_tunneller_partys(void);
char get_player_number_from_value(const char* txt);
long get_condition_value(PlayerNumber plyr_idx, unsigned char valtype, unsigned char a3);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
