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
#include "thing_creature.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CREATURE_TYPES_MAX 64
#define INSTANCE_TYPES_MAX 64
#define CREATURE_STATES_MAX 147
/******************************************************************************/
struct Thing;

enum CreatureModelFlags {
    MF_IsSpecDigger     = 0x0001, // Imp and Tunneler
    MF_IsArachnid       = 0x0002, // simply, Spider
    MF_IsDiptera        = 0x0004, // simply, Fly
    MF_IsLordOTLand     = 0x0008, // simply, Knight
    MF_IsSpectator      = 0x0010, // simply, Floating spirit
    MF_IsEvil           = 0x0020, // All evil creatures
    MF_NeverChickens    = 0x0040, // Cannot be affected by Chicken (for Avatar)
    MF_ImmuneToBoulder  = 0x0080, // Boulder traps are destroyed at the moment they touch the creature
    MF_NoCorpseRotting  = 0x0100, // Corpse cannot rot in graveyard
};

enum CreatureJobFlags {
    Job_NULL             = 0x0000,
    Job_TUNNEL           = 0x0001,
    Job_DIG              = 0x0002,
    Job_RESEARCH         = 0x0004,
    Job_TRAIN            = 0x0008,
    Job_MANUFACTURE      = 0x0010,
    Job_SCAVENGE         = 0x0020,
    Job_KINKY_TORTURE    = 0x0040,
    Job_FIGHT            = 0x0080,
    Job_SEEK_THE_ENEMY   = 0x0100,
    Job_GUARD            = 0x0200,
    Job_GROUP            = 0x0400,
    Job_BARRACK          = 0x0800,
    Job_TEMPLE           = 0x1000,
    Job_FREEZE_PRISONERS = 0x2000,
    Job_EXPLORE          = 0x4000,
};

enum CreatureDeathKind {
    Death_Unset = 0,
    Death_Normal,
    Death_FleshExplode,
    Death_GasFleshExplode,
    Death_SmokeExplode,
    Death_IceExplode,
};

enum CreaturePreferredAttacks {
    PrefAttck_Unset = 0,
    PrefAttck_Melee,
    PrefAttck_Ranged,
};

enum CreatureTypesLoadFlags {
    CTLd_Standard      =  0x00,
    CTLd_KindListOnly  =  0x01,
};

/******************************************************************************/
#pragma pack(1)

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

struct Creatures { // sizeof = 16
  unsigned short evil_start_state;
  unsigned short good_start_state;
  unsigned char natural_death_kind;
  unsigned char field_5;
  unsigned char field_6;
  unsigned char field_7;
  unsigned char swipe_idx;
  short field_9;
  short field_B;
  short field_D;
  unsigned char field_F;
};

#pragma pack()
/******************************************************************************/

struct CreatureConfig {
    long model_count;
    struct CreatureModelConfig model[CREATURE_TYPES_MAX];
    long states_count;
    struct CommandWord state_names[CREATURE_STATES_MAX];
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
    long spectator_breed;
};

/******************************************************************************/
extern const char keeper_creaturetp_file[];
extern struct NamedCommand creature_desc[];
extern const struct NamedCommand angerjob_desc[];
extern const struct NamedCommand creaturejob_desc[];
extern const struct NamedCommand attackpref_desc[];
extern struct NamedCommand instance_desc[];
extern const struct NamedCommand creature_graphics_desc[];
extern struct CreatureConfig crtr_conf;
/******************************************************************************/
extern struct CreatureData creature_data[];
//extern struct Creatures creatures[];
/******************************************************************************/
DLLIMPORT struct Creatures _DK_creatures[CREATURE_TYPES_COUNT];
#define creatures _DK_creatures
DLLIMPORT unsigned short _DK_breed_activities[CREATURE_TYPES_COUNT];
#define breed_activities _DK_breed_activities
/******************************************************************************/
struct CreatureStats *creature_stats_get(ThingModel crstat_idx);
struct CreatureStats *creature_stats_get_from_thing(const struct Thing *thing);
struct CreatureData *creature_data_get(ThingModel crstat_idx);
struct CreatureData *creature_data_get_from_thing(const struct Thing *thing);
TbBool creature_stats_invalid(const struct CreatureStats *crstat);
void creature_stats_updated(ThingModel crstat_idx);
void check_and_auto_fix_stats(void);
const char *creature_code_name(ThingModel crmodel);
long creature_model_id(const char * name);
/******************************************************************************/
TbBool load_creaturetypes_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
unsigned short get_creature_model_flags(const struct Thing *thing);
TbBool set_creature_available(long plyr_idx, long crtr_model, long can_be_avail, long force_avail);
long get_players_special_digger_breed(long plyr_idx);
long get_players_spectator_breed(long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
