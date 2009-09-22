/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_magic.h
 *     Header file for config_magic.c.
 * @par Purpose:
 *     Support of configuration files for magic spells.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGMAGIC_H
#define DK_CFGMAGIC_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MAGIC_ITEMS_MAX 64
#define MAGIC_OVERCHARGE_LEVELS 9

struct MagicConfig {
    long spell_types_count;
    struct CommandWord spell_names[MAGIC_ITEMS_MAX];
    long shot_types_count;
    struct CommandWord shot_names[MAGIC_ITEMS_MAX];
    long power_types_count;
    struct CommandWord power_names[MAGIC_ITEMS_MAX];
};

/******************************************************************************/
#pragma pack(1)

struct SpellConfig { // sizeof=4??
  int duration;
};

struct ShotStats // sizeof = 101
{
  short numfield_0;
  short numfield_2;
  unsigned char field_4[2];
  unsigned char field_6;
  unsigned char field_7;
  unsigned char field_8;
  short field_9;
  short field_B;
  unsigned char field_D;
  unsigned char field_E;
  unsigned char field_F;
  unsigned char field_10;
  unsigned char field_11;
  unsigned char field_12;
  unsigned char field_13;
  short health;
  short damage;
  unsigned char field_18;
  short speed;
  short field_1B;
  unsigned char field_1D;
  short field_1E;
  short field_20;
  short field_22;
  unsigned char field_24;
  short field_25;
  unsigned char field_27;
  unsigned char field_28;
  unsigned char field_29;
  unsigned char field_2A;
  short field_2B;
  short field_2D;
  unsigned char field_2F;
  unsigned char field_30;
  short field_31;
  short field_33;
  unsigned char field_35;
  unsigned char field_36;
  short field_37;
  short field_39;
  unsigned char field_3B;
  short field_3C;
  short field_3E;
  unsigned char field_40;
  short field_41;
  short field_43;
  short field_45;
  unsigned char field_47;
  unsigned char field_48;
  unsigned char field_49;
  unsigned char field_4A;
  unsigned char field_4B;
  unsigned char field_4C;
  unsigned char numfield_4D;
  short numfield_4E;
  short field_50;
  unsigned char field_52;
  unsigned char field_53;
  unsigned char field_54[4];
  unsigned char field_58[8];
  unsigned char field_60[4];
  unsigned char field_64;
};

struct MagicStats {  // sizeof=0x4C
  long cost[MAGIC_OVERCHARGE_LEVELS];
  long time;
  long power[MAGIC_OVERCHARGE_LEVELS];
};

/******************************************************************************/
DLLIMPORT struct ShotStats _DK_shot_stats[30];
#define shot_stats _DK_shot_stats

#pragma pack()
/******************************************************************************/
extern const char keeper_magic_file[];
extern struct NamedCommand spell_desc[];
extern struct NamedCommand shot_desc[];
extern struct NamedCommand power_desc[];
/******************************************************************************/
TbBool load_magic_config(const char *conf_fname,unsigned short flags);
TbBool make_all_powers_free(void);
TbBool make_all_powers_researchable(long plyr_idx);
TbBool set_power_available(long plyr_idx, long spl_idx, long resrch, long avail);
TbBool make_available_all_researchable_powers(long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
