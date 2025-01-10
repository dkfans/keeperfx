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
#include "creature_control.h"
#include "thing_creature.h"
#include "creature_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CREATURE_NONE            255
#define CREATURE_ANY             254
#define CREATURE_NOT_A_DIGGER    253
#define CREATURE_DIGGER          252

/** Percentage of creature parameter increase for every experience level.
 *  Used as default value, should be replaced in config file. */
#define CREATURE_PROPERTY_INCREASE_ON_EXP  35
/******************************************************************************/
enum CreatureModelFlags {
    CMF_IsSpecDigger     = 0x000001, // Imp and Tunneller.
    CMF_IsArachnid       = 0x000002, // Simply, Spider.
    CMF_IsDiptera        = 0x000004, // Simply, Fly.
    CMF_IsLordOfLand     = 0x000008, // Simply, Knight and Avatar.
    CMF_IsSpectator      = 0x000010, // Simply, Floating Spirit.
    CMF_IsEvil           = 0x000020, // All evil creatures.
    CMF_ImmuneToBoulder  = 0x000040, // Boulder traps are destroyed at the moment they touch the creature.
    CMF_NoCorpseRotting  = 0x000080, // Corpse cannot rot in graveyard.
    CMF_NoEnmHeartAttack = 0x000100, // Creature will not attack enemy heart on sight.
    CMF_Trembling        = 0x000200, // Creature causes ground to tremble when dropped.
    CMF_Fat              = 0x000400, // Creature too fat to walk a full animation.
    CMF_Female           = 0x000800, // Creature is female.
    CMF_Insect           = 0x001000, // Creature is kind of insect.
    CMF_OneOfKind        = 0x002000, // Only one creature of that kind may exist on one level. Unit name is type name.
    CMF_NoImprisonment   = 0x004000, // Creature will not faint.
    CMF_NoResurrect      = 0x008000, // Creature will not resurrect.
    CMF_NoTransfer       = 0x010000, // Creature cannot be transferred.
    CMF_NoStealHero      = 0x020000, // Prevent the creature from being stolen with the Steal Hero special.
    CMF_PreferSteal      = 0x040000, // The creature can be generated from Steal Hero special if there's nothing to steal.
    CMF_EventfulDeath    = 0x080000, // The LAST_DEATH_EVENT[] script location is updated on death.
};

// Before C23 standard, we cannot specify the underlaying type (in this case we want 64bit int) of enum.
// Some compilers may handle it well but some may not. We use #define to avoid the portability issue.
// Refer https://open-std.org/JTC1/SC22/WG14/www/docs/n3030.htm
#define Job_NULL                (0LL)
#define Job_TUNNEL              (1LL << 0)
#define Job_DIG                 (1LL << 1)
#define Job_RESEARCH            (1LL << 2)
#define Job_TRAIN               (1LL << 3)
#define Job_MANUFACTURE         (1LL << 4)
#define Job_SCAVENGE            (1LL << 5)
#define Job_KINKY_TORTURE       (1LL << 6)
#define Job_JOIN_FIGHT          (1LL << 7)
#define Job_SEEK_THE_ENEMY      (1LL << 8)
#define Job_GUARD               (1LL << 9)
#define Job_GROUP               (1LL << 10) //This job doesn't do anything, to be implemented.
#define Job_BARRACK             (1LL << 11)
#define Job_TEMPLE_PRAY         (1LL << 12)
#define Job_FREEZE_PRISONERS    (1LL << 13)
#define Job_EXPLORE             (1LL << 14)
// Jobs which can't be assigned to a creature, are only working one time
#define Job_EXEMPT              (1LL << 15)
#define Job_TEMPLE_SACRIFICE    (1LL << 16)
#define Job_PAINFUL_TORTURE     (1LL << 17)
#define Job_CAPTIVITY           (1LL << 18)
#define Job_PLACE_IN_VAULT      (1LL << 19)
#define Job_TAKE_SALARY         (1LL << 20)
#define Job_TAKE_FEED           (1LL << 21)
#define Job_TAKE_SLEEP          (1LL << 22)
//TODO Nonexisting - TO ADD LATER - digger jobs
#define Job_MINE                (1LL << 23)
#define Job_CONVERT_GROUND      (1LL << 24)
#define Job_IMPROVE_GROUND      (1LL << 25)
#define Job_REINFORCE_WALL      (1LL << 26)
//TODO Nonexisting - TO ADD LATER - anger jobs
#define Job_KILL_OWN_CREATURE   (1LL << 27)
#define Job_DESTROY_OWN_ROOM    (1LL << 28)
#define Job_LEAVE_DUNGEON       (1LL << 29)
#define Job_STEAL_OWN_GOLD      (1LL << 30)
#define Job_DAMAGE_OWN_WALLS    (1LL << 31)
#define Job_MAD_PSYCHO          (1LL << 32)
#define Job_PERSUADE_LEAVE      (1LL << 33)
#define Job_JOIN_ENEMY          (1LL << 34)
//TODO Nonexisting - TO ADD LATER - hero objectives
#define Job_ATTACK_ROOM         (1LL << 35)
#define Job_ATTACK_CREATURE     (1LL << 36)
#define Job_ATTACK_DNHEART      (1LL << 37)
#define Job_STEAL_GOLD          (1LL << 38)
#define Job_STEAL_SPELLS        (1LL << 39)
#define Job_DEFEND_PARTY        (1LL << 40)

enum JobKindFlags {
    JoKF_None                   = 0x00000000,
    // "Assign" field
    JoKF_AssignHumanDrop        = 0x00000001,
    JoKF_AssignComputerDrop     = 0x00000002,
    JoKF_AssignCeatureInit      = 0x00000004,
    JoKF_AssignAreaWithinRoom   = 0x00000008,
    JoKF_AssignAreaOutsideRoom  = 0x00000010,
    JoKF_AssignOnAreaBorder     = 0x00000040,
    JoKF_AssignOnAreaCenter     = 0x00000080,
    JoKF_OwnedCreatures         = 0x00000200,
    JoKF_EnemyCreatures         = 0x00000400,
    JoKF_OwnedDiggers           = 0x00000800,
    JoKF_EnemyDiggers           = 0x00001000,
    JoKF_AssignOneTime          = 0x00002000,
    JoKF_NeedsHaveJob           = 0x00004000,
    // "Flags" field
    JoKF_NeedsCapacity          = 0x00010000,
    JoKF_WorkOnAreaBorder       = 0x00020000,
    JoKF_WorkOnAreaCenter       = 0x00040000,
    JoKF_NoSelfControl          = 0x00080000,
    JoKF_NoGroups               = 0x00100000,
    JoKF_AllowChickenized       = 0x00200000,
};

enum InstancePropertiesFlags {
    InstPF_None               = 0x0000,
    InstPF_RepeatTrigger      = 0x0001,
    InstPF_RangedAttack       = 0x0002,
    InstPF_MeleeAttack        = 0x0004,
    InstPF_SelfBuff           = 0x0008,
    InstPF_RangedDebuff       = 0x0010,
    InstPF_Dangerous          = 0x0020,
    InstPF_Destructive        = 0x0040,
    InstPF_Unused             = 0x0080, //Quick
    InstPF_Disarming          = 0x0100,
    InstPF_UsesSwipe          = 0x0200,
    InstPF_RangedBuff         = 0x0400,
    InstPF_NeedsTarget        = 0x0800,
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

struct Thing;

/******************************************************************************/
extern ThingModel breed_activities[CREATURE_TYPES_MAX];
#pragma pack()
/******************************************************************************/

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
    unsigned char func_plyr_check_idx;
    unsigned char func_plyr_assign_idx;
    unsigned char func_cord_check_idx;
    unsigned char func_cord_assign_idx;
    RoomRole room_role;
    EventKind event_kind;
    /** The state creature should go into when job is started. */
    CrtrStateId initial_crstate;
    /** The state creature should back to after job is interrupted. */
    CrtrStateId continue_crstate;
    unsigned long job_flags;
};

struct CreatureAngerJobConfig {
    char name[COMMAND_WORD_LEN];
};

struct CreatureModelConfig {
    char name[COMMAND_WORD_LEN];
    long namestr_idx;
    unsigned long model_flags;
};

/**
 * Structure which stores levelling up stats.
 */
struct CreatureExperience {
    long size_increase_on_exp;
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
    long exp_on_hitting_increase_on_exp;
    long training_cost_increase_on_exp;
    long scavenging_cost_increase_on_exp;
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
    long attacktypes_count;
    struct CommandWord attacktypes[INSTANCE_TYPES_MAX];
    struct CreatureExperience exp;
    ThingModel special_digger_good;
    ThingModel special_digger_evil;
    ThingModel spectator_breed;
    short creature_graphics[CREATURE_TYPES_MAX][CREATURE_GRAPHICS_INSTANCES];
    long sprite_size;
    struct CreatureSounds creature_sounds[CREATURE_TYPES_MAX];
};

/******************************************************************************/
extern const char keeper_creaturetp_file[];
extern struct NamedCommand creature_desc[];
extern struct NamedCommand angerjob_desc[];
extern struct NamedCommand creaturejob_desc[];
extern struct NamedCommand attackpref_desc[];
extern struct NamedCommand instance_desc[];
extern struct NamedCommand lenses_desc[];
extern const struct NamedCommand creatmodel_attributes_commands[];
extern const struct NamedCommand creatmodel_jobs_commands[];
extern const struct NamedCommand creatmodel_attraction_commands[];
extern const struct NamedCommand creatmodel_sounds_commands[];
extern const struct NamedCommand creatmodel_sprite_commands[];
extern const struct NamedCommand creature_graphics_desc[];
extern const struct NamedCommand creatmodel_annoyance_commands[];
extern const struct NamedCommand creatmodel_experience_commands[];
extern const struct NamedCommand creatmodel_senses_commands[];
extern const struct NamedCommand creatmodel_appearance_commands[];
extern const struct NamedCommand creature_deathkind_desc[];
extern Creature_Job_Player_Check_Func creature_job_player_check_func_list[];
/******************************************************************************/
struct CreatureStats *creature_stats_get(ThingModel crstat_idx);
struct CreatureStats *creature_stats_get_from_thing(const struct Thing *thing);
TbBool creature_stats_invalid(const struct CreatureStats *crstat);
void check_and_auto_fix_stats(void);
void init_creature_model_stats(void);
void init_creature_model_graphics(void);
const char *creature_code_name(ThingModel crmodel);
long creature_model_id(const char * name);
const char *creature_own_name(const struct Thing *creatng);
TbBool is_creature_model_wildcard(ThingModel crmodel);
/******************************************************************************/
TbBool load_creaturetypes_config(const char *conf_fname, unsigned short flags);
/******************************************************************************/
unsigned long get_creature_model_flags(const struct Thing *thing);
TbBool set_creature_available(PlayerNumber plyr_idx, ThingModel crtr_model, long can_be_avail, long force_avail);
ThingModel get_players_special_digger_model(PlayerNumber plyr_idx);
ThingModel get_players_spectator_model(PlayerNumber plyr_idx);
ThingModel get_creature_model_with_model_flags(unsigned long needflags);
/******************************************************************************/
struct CreatureInstanceConfig *get_config_for_instance(CrInstance inst_id);
const char *creature_instance_code_name(CrInstance inst_id);
/******************************************************************************/
struct CreatureJobConfig *get_config_for_job(CreatureJob job_flags);
RoomKind get_first_room_kind_for_job(CreatureJob job_flags);
RoomRole get_room_role_for_job(CreatureJob job_flags);
EventKind get_event_for_job(CreatureJob job_flags);
CrtrStateId get_initial_state_for_job(CreatureJob jobpref);
CrtrStateId get_arrive_at_state_for_job(CreatureJob jobpref);
CrtrStateId get_continue_state_for_job(CreatureJob jobpref);
CreatureJob get_job_for_creature_state(CrtrStateId crstat_id);
CreatureJob get_jobs_enemies_may_do_in_room(RoomKind rkind);
CreatureJob get_jobs_enemies_may_do_in_room_role(RoomRole rrole);
unsigned long get_flags_for_job(CreatureJob jobpref);
int get_required_room_capacity_for_job(CreatureJob jobpref, ThingModel crmodel);
CreatureJob get_creature_job_causing_going_postal(CreatureJob job_flags, RoomKind rkind);
CreatureJob get_creature_job_causing_stress(CreatureJob job_flags, RoomKind rkind);
CreatureJob get_job_for_subtile(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long drop_kind_flags);
CreatureJob get_job_for_room(RoomKind rkind, unsigned long required_kind_flags, CreatureJob has_jobs);
CreatureJob get_job_for_room_role(RoomRole rrole, unsigned long required_kind_flags, CreatureJob has_jobs);
CreatureJob get_job_which_qualify_for_room(RoomKind rkind, unsigned long qualify_flags, unsigned long prevent_flags);
CreatureJob get_job_which_qualify_for_room_role(RoomRole rrole, unsigned long qualify_flags, unsigned long prevent_flags);
const char *creature_job_code_name(CreatureJob job_flag);
struct Thing* thing_death_flesh_explosion(struct Thing* thing);
/******************************************************************************/
const char *attack_type_job_code_name(CrAttackType attack_type);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
