/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_creature.h
 *     Header file for config_creature.c.
 * @par Purpose:
 *     Creature names, appearance and parameters configuration loading functions.
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
#ifndef DK_CFGLCRTR_H
#define DK_CFGLCRTR_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CREATURE_TYPES_MAX 64
#define INSTANCE_TYPES_MAX 64
/******************************************************************************/
struct Thing;
/******************************************************************************/
enum CreatureModelFlags {
    MF_IsSpecDigger   = 0x0001, // Imp and Tunneller
    MF_IsArachnid     = 0x0002, // simply, Spider
    MF_IsDiptera      = 0x0004, // simply, Fly
    MF_IsLordOTLand   = 0x0008, // simply, Knight
    MF_IsSpectator    = 0x0010, // simply, Floating spirit
    MF_IsEvil         = 0x0020, // All evil creatures
};

struct CreatureModelConfig {
    char name[COMMAND_WORD_LEN];
    long namestr_idx;
    unsigned short model_flags;
};

struct CreatureData {
      unsigned char flags;
      short field_1;
      short namestr_idx;
};

struct CreatureConfig {
    long model_count;
    struct CreatureModelConfig model[CREATURE_TYPES_MAX];
    long instance_count;
    struct CommandWord instance_names[INSTANCE_TYPES_MAX];
    long job_count;
    struct CommandWord job_names[INSTANCE_TYPES_MAX];
    long angerjob_count;
    struct CommandWord angerjob_names[INSTANCE_TYPES_MAX];
    long attackpref_count;
    struct CommandWord attackpref_names[INSTANCE_TYPES_MAX];
    long special_digger_good;
    long special_digger_evil;
};

/******************************************************************************/
extern const char keeper_creaturetp_file[];
extern struct NamedCommand creature_desc[];
extern const struct NamedCommand angerjob_desc[];
extern const struct NamedCommand creaturejob_desc[];
extern const struct NamedCommand attackpref_desc[];
extern struct NamedCommand instance_desc[];
extern struct CreatureConfig crtr_conf;
/******************************************************************************/
DLLIMPORT struct InstanceInfo _DK_instance_info[48];
#define instance_info _DK_instance_info
extern struct CreatureData creature_data[];
/******************************************************************************/
struct CreatureStats *creature_stats_get(long crstat_idx);
struct CreatureStats *creature_stats_get_from_thing(const struct Thing *thing);
struct CreatureData *creature_data_get(long crstat_idx);
struct CreatureData *creature_data_get_from_thing(const struct Thing *thing);
TbBool creature_stats_invalid(const struct CreatureStats *crstat);
const char *creature_code_name(long crmodel);
/******************************************************************************/
TbBool load_creaturetypes_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
long calculate_correct_creature_maxspeed(const struct Thing *thing);
unsigned short get_creature_model_flags(const struct Thing *thing);
TbBool set_creature_available(long plyr_idx, long crtr_model, long can_be_avail, long force_avail);
long get_players_special_digger_breed(long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
