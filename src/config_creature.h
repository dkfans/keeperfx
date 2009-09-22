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

struct CreatureConfig {
    long kind_count;
    struct CommandWord kind_names[CREATURE_TYPES_MAX];
    long instance_count;
    struct CommandWord instance_names[INSTANCE_TYPES_MAX];
    long job_count;
    struct CommandWord job_names[INSTANCE_TYPES_MAX];
    long angerjob_count;
    struct CommandWord angerjob_names[INSTANCE_TYPES_MAX];
    long attackpref_count;
    struct CommandWord attackpref_names[INSTANCE_TYPES_MAX];
};

/******************************************************************************/
#pragma pack(1)

DLLIMPORT struct InstanceInfo _DK_instance_info[48];
#define instance_info _DK_instance_info

#pragma pack()
/******************************************************************************/
extern const char keeper_creaturetp_file[];
extern struct NamedCommand creature_desc[];
extern const struct NamedCommand angerjob_desc[];
extern const struct NamedCommand creaturejob_desc[];
extern const struct NamedCommand attackpref_desc[];
extern struct NamedCommand instance_desc[];
extern struct CreatureConfig crtr_conf;
/******************************************************************************/
struct CreatureStats *creature_stats_get(long crstat_idx);
struct CreatureStats *creature_stats_get_from_thing(struct Thing *thing);
TbBool creature_stats_invalid(struct CreatureStats *crstat);
/******************************************************************************/
TbBool load_creaturetypes_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
long calculate_correct_creature_maxspeed(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
