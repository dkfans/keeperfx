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
#define CREATURE_STATES_MAX 256
/** Percentage of creature parameter increase for every experience level.
 *  Used as default value, should be replaced in config file. */
#define CREATURE_PROPERTY_INCREASE_ON_EXP  35
/******************************************************************************/

struct Thing;

enum CreatureModelFlags {
    CMF_IsSpecDigger     = 0x0001, // Imp and Tunneler
    CMF_IsArachnid       = 0x0002, // simply, Spider
    CMF_IsDiptera        = 0x0004, // simply, Fly
    CMF_IsLordOTLand     = 0x0008, // simply, Knight
    CMF_IsSpectator      = 0x0010, // simply, Floating spirit
    CMF_IsEvil           = 0x0020, // All evil creatures
    CMF_NeverChickens    = 0x0040, // Cannot be affected by Chicken (for Avatar)
    CMF_ImmuneToBoulder  = 0x0080, // Boulder traps are destroyed at the moment they touch the creature
    CMF_NoCorpseRotting  = 0x0100, // Corpse cannot rot in graveyard
    CMF_NoEnmHeartAttack = 0x0200, // Creature will not attack enemy heart on sight
    CMF_TremblingFat     = 0x0400, // Creature causes ground to tremble when dropped
    CMF_Female           = 0x0800, // Creature is female
    CMF_Insect           = 0x1000, // Creature is kind of insect
    CMF_OneOfKind        = 0x2000, // Only one creature of that kind may exist on one level
};

enum CreatureJobFlags {
    Job_NULL             = 0x00000000,
    Job_TUNNEL           = 0x00000001,
    Job_DIG              = 0x00000002,
    Job_RESEARCH         = 0x00000004,
    Job_TRAIN            = 0x00000008,
    Job_MANUFACTURE      = 0x00000010,
    Job_SCAVENGE         = 0x00000020,
    Job_KINKY_TORTURE    = 0x00000040,
    Job_JOIN_FIGHT       = 0x00000080,
    Job_SEEK_THE_ENEMY   = 0x00000100,
    Job_GUARD            = 0x00000200,
    Job_GROUP            = 0x00000400, //This job doesn't do anything, should be removed?
    Job_BARRACK          = 0x00000800,
    Job_TEMPLE_PRAY      = 0x00001000,
    Job_FREEZE_PRISONERS = 0x00002000,
    Job_EXPLORE          = 0x00004000,
    // Jobs which can't be assigned to a creature, are only working one time
    Job_EXEMPT           = 0x00008000,
    Job_TEMPLE_SACRIFICE = 0x00010000,
    Job_PAINFUL_TORTURE  = 0x00020000,
    Job_CAPTIVITY        = 0x00040000,
    Job_PLACE_IN_VAULT   = 0x00080000,
    Job_TAKE_SALARY      = 0x00100000,
    Job_TAKE_FEED        = 0x00200000,
    Job_TAKE_SLEEP       = 0x00400000,
};

enum JobKindFlags {
    JoKF_None               = 0x00000000,
    // "Assign" field
    JoKF_AssignHumanDropInRoom    = 0x00000001,
    JoKF_AssignComputerDropInRoom = 0x00000002,
    JoKF_AssignInitInRoom         = 0x00000004,
    JoKF_AssignDropOnRoomBorder   = 0x00000008,
    JoKF_AssignDropOnRoomCenter   = 0x00000010,
    JoKF_OwnedCreatures     = 0x00000020,
    JoKF_EnemyCreatures     = 0x00000040,
    JoKF_OwnedDiggers       = 0x00000080,
    JoKF_EnemyDiggers       = 0x00000100,
    JoKF_AssignOneTime      = 0x00000200,
    JoKF_NeedsHaveJob       = 0x00000400,
    // "Flags" field
    JoKF_NeedsCapacity      = 0x00001000,
    JoKF_WorkOnRoomBorder   = 0x00002000,
    JoKF_WorkOnRoomCenter   = 0x00004000,
    JoKF_NoSelfControl      = 0x00008000,
    JoKF_NoGroups           = 0x00010000,
    JoKF_AllowChickenized   = 0x00020000,
};

enum InstancePropertiesFlags {
    InstPF_None               = 0x00,
    InstPF_RepeatTrigger      = 0x01,
};

enum CreatureDeathKind {
    Death_Unset = 0,
    Death_Normal,
    Death_FleshExplode,
    Death_GasFleshExplode,
    Death_SmokeExplode,
    Death_IceExplode,
};

enum CreatureAttackType {
    AttckT_Unset = 0,
    AttckT_Melee,
    AttckT_Ranged,
};

/******************************************************************************/
#pragma pack(1)

struct CreatureModelConfig {
    char name[COMMAND_WORD_LEN];
    long namestr_idx;
    unsigned short model_flags;
};

struct CreatureStateConfig {
    char name[COMMAND_WORD_LEN];
};

struct CreatureInstanceConfig {
    char name[COMMAND_WORD_LEN];
};

typedef TbBool (*Creature_Job_Player_Check_Func)(const struct Thing *, PlayerNumber, CreatureJob);
typedef TbBool (*Creature_Job_Player_Assign_Func)(struct Thing *, PlayerNumber, CreatureJob);
typedef TbBool (*Creature_Job_Coords_Check_Func)(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob jobpref, unsigned long flags);
typedef TbBool (*Creature_Job_Coords_Assign_Func)(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob jobpref);

struct CreatureJobConfig {
    char name[COMMAND_WORD_LEN];
    Creature_Job_Player_Check_Func func_plyr_check;
    Creature_Job_Player_Assign_Func func_plyr_assign;
    Creature_Job_Coords_Check_Func func_cord_check;
    Creature_Job_Coords_Assign_Func func_cord_assign;
    RoomKind room_kind;
    EventKind event_kind;
    CrtrStateId initial_crstate;
    unsigned long job_flags;
};

struct CreatureAngerJobConfig {
    char name[COMMAND_WORD_LEN];
};

struct CreatureData {
      unsigned char flags;
      short lair_tngmodel;
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

/**
 * Structure which stores levelling up stats.
 */
struct CreatureExperience {
    long pay_increase_on_exp;
    long spell_damage_increase_on_exp;
    long range_increase_on_exp;
    long job_value_increase_on_exp;
    long health_increase_on_exp;
    long strength_increase_on_exp;
    long dexterity_increase_on_exp;
    long defense_increase_on_exp;
    long loyalty_increase_on_exp;
    long armour_increase_on_exp;
};

struct CreatureConfig {
    long model_count;
    struct CreatureModelConfig model[CREATURE_TYPES_MAX];
    long states_count;
    struct CreatureStateConfig states[CREATURE_STATES_MAX];
    long instances_count;
    struct CreatureInstanceConfig instances[INSTANCE_TYPES_MAX];
    long jobs_count;
    struct CreatureJobConfig jobs[INSTANCE_TYPES_MAX];
    long angerjobs_count;
    struct CreatureAngerJobConfig angerjobs[INSTANCE_TYPES_MAX];
    long attackpref_count;
    struct CommandWord attackpref_names[INSTANCE_TYPES_MAX];
    struct CreatureExperience exp;
    ThingModel special_digger_good;
    ThingModel special_digger_evil;
    ThingModel spectator_breed;
};

/******************************************************************************/
extern const char keeper_creaturetp_file[];
extern struct NamedCommand creature_desc[];
extern struct NamedCommand angerjob_desc[];
extern struct NamedCommand creaturejob_desc[];
extern struct NamedCommand attackpref_desc[];
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
const char *creature_own_name(const struct Thing *creatng);
/******************************************************************************/
TbBool load_creaturetypes_config(const char *conf_fname, unsigned short flags);
/******************************************************************************/
unsigned short get_creature_model_flags(const struct Thing *thing);
TbBool set_creature_available(PlayerNumber plyr_idx, ThingModel crtr_model, long can_be_avail, long force_avail);
ThingModel get_players_special_digger_model(PlayerNumber plyr_idx);
ThingModel get_players_spectator_model(PlayerNumber plyr_idx);
ThingModel get_creature_model_with_model_flags(unsigned short needflags);
/******************************************************************************/
struct CreatureInstanceConfig *get_config_for_instance(CrInstance inst_id);
const char *creature_instance_code_name(CrInstance inst_id);
/******************************************************************************/
struct CreatureJobConfig *get_config_for_job(CreatureJob job_flags);
RoomKind get_room_for_job(CreatureJob job_flags);
EventKind get_event_for_job(CreatureJob job_flags);
CrtrStateId get_initial_state_for_job(CreatureJob jobpref);
CrtrStateId get_arrive_at_state_for_job(CreatureJob jobpref);
CrtrStateId get_arrive_at_state_for_room(RoomKind rkind);
unsigned long get_flags_for_job(CreatureJob jobpref);
CreatureJob get_creature_job_causing_going_postal(CreatureJob job_flags, RoomKind rkind);
CreatureJob get_creature_job_causing_stress(CreatureJob job_flags, RoomKind rkind);
CreatureJob get_job_for_subtile(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long drop_kind_flags);
CreatureJob get_job_for_room(RoomKind rkind, unsigned long required_kind_flags, CreatureJob has_jobs);
CreatureJob get_job_which_qualify_for_room(RoomKind rkind, unsigned long qualify_flags, unsigned long prevent_flags);
const char *creature_job_code_name(CreatureJob job_flag);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
