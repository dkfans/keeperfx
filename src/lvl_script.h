/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.h
 *     Header file for lvl_script.c.
 * @par Purpose:
 *     Level script commands support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
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
#define CONDITIONS_COUNT         512
#define TUNNELLER_TRIGGERS_COUNT 256
#define SCRIPT_VALUES_COUNT      2048
#define WIN_CONDITIONS_COUNT      12

#define CONDITION_ALWAYS (CONDITIONS_COUNT)

#define SENSIBLE_GOLD 99999999

#define EXTERNAL_SOUNDS_COUNT 32

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

struct TunnellerTrigger {
  unsigned char flags;
  unsigned short condit_idx;
  unsigned char plyr_idx;
  unsigned long location;
  unsigned long heading; // originally was 'target'
  long carried_gold;
  unsigned char crtr_level;
  char party_id;
};

struct PartyTrigger {
  unsigned char flags;
  unsigned short condit_idx;
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

struct ScriptValue {
  unsigned char flags;
  unsigned short condit_idx;
  unsigned char valtype;
  unsigned char plyr_range;
  union
  {
    struct
    {
      union
      {
          long arg0;
          char* str0;
      };
      union
      {
          long arg1;
          char* str1;
      };
      union
      {
          long arg2;
          char* str2;
      };
    };
    struct
    {
      union
      {
          unsigned long uarg0;
          unsigned char* ustr0;
      };
      union
      {
          unsigned long uarg1;
          unsigned char* ustr1;
      };
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

struct Condition {
  short condit_idx;
  unsigned char status;
  unsigned char plyr_range;
  unsigned char variabl_type;
  unsigned short variabl_idx;
  unsigned char operation;
  unsigned long rvalue;
  unsigned char plyr_range_right;
  unsigned char variabl_type_right;
  unsigned short variabl_idx_right;
  TbBool use_second_variable;
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
extern unsigned char next_command_reusable;


#pragma pack()
/******************************************************************************/

extern const struct NamedCommand player_desc[];
/******************************************************************************/
short clear_script(void);
short load_script(long lvl_num);
TbBool preload_script(long lvnum);
/******************************************************************************/

long get_condition_value(PlayerNumber plyr_idx, unsigned char valtype, unsigned char a3);
void process_level_script(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
