/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_stats.h
 *     Header file for thing_stats.c.
 * @par Purpose:
 *     thing_stats functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THINGSTATS_H
#define DK_THINGSTATS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CREATURE_MAX_LEVEL     10
#define LEARNED_INSTANCES_COUNT 10

enum CreatureLiveStatistics {
    CrLStat_FirstName,
    CrLStat_BloodType,
    CrLStat_ExpLevel,
    CrLStat_Health,
    CrLStat_MaxHealth,
    CrLStat_Strength,
    CrLStat_Armour,
    CrLStat_Defence,
    CrLStat_Dexterity,
    CrLStat_Luck,
    CrLStat_Speed,
    CrLStat_Loyalty,
    CrLStat_AgeTime,
    CrLStat_Kills,
    CrLStat_GoldHeld,
    CrLStat_GoldWage,
    CrLStat_ResearchSkill,
    CrLStat_ManufactureSkill,
    CrLStat_TrainingSkill,
    CrLStat_ScavengeSkill,
    CrLStat_TrainingCost,
    CrLStat_ScavengeCost,
    CrLStat_BestDamage,
    CrLStat_Weight,
    CrLStat_Score,
};

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct SlabMap;

typedef short CreatureLiveStatId;

#pragma pack()
/******************************************************************************/
#define BLOOD_TYPES_COUNT (sizeof(blood_types)/sizeof(blood_types[0]))
extern const char *blood_types[15];
/******************************************************************************/
const char *thing_model_name(const struct Thing *thing);
const char *thing_class_and_model_name(int class_id, int model);
const char *thing_class_code_name(int class_id);
const char *creatrtng_actstate_name(const struct Thing *thing);
const char *creatrtng_realstate_name(const struct Thing *thing);
TbBool things_stats_debug_dump(void);
TbBool is_neutral_thing(const struct Thing *thing);
TbBool is_hero_thing(const struct Thing *thing);
/******************************************************************************/
long compute_creature_kind_score(ThingModel crkind,unsigned short crlevel);
long compute_creature_max_pay(long base_pay,unsigned short crlevel);
long compute_creature_max_health(long base_health,unsigned short crlevel);
long compute_creature_attack_melee_damage(long base_param, long luck, unsigned short crlevel, struct Thing* thing);
long compute_creature_attack_spell_damage(long base_param,long luck,unsigned short crlevel, struct Thing* thing);
long project_creature_attack_melee_damage(long base_param,long luck,unsigned short crlevel);
long project_creature_attack_spell_damage(long base_param,long luck,unsigned short crlevel);
long compute_creature_attack_range(long base_param,long luck,unsigned short crlevel);
long compute_creature_work_value(long base_param,long efficiency,unsigned short crlevel);
long compute_creature_max_dexterity(long base_param,unsigned short crlevel);
long compute_creature_max_defense(long base_param,unsigned short crlevel);
long compute_creature_max_strength(long base_param,unsigned short crlevel);
long compute_creature_max_loyalty(long base_param,unsigned short crlevel);
long compute_creature_max_armour(long base_param, unsigned short crlevel, TbBool armour_spell);
long compute_creature_max_unaffected(long base_param,unsigned short crlevel);
#define compute_creature_max_luck compute_creature_max_unaffected
long compute_controlled_speed_increase(long prev_speed, long speed_limit);
long compute_controlled_speed_decrease(long prev_speed, long speed_limit);
long compute_value_percentage(long base_val, short npercent);
long compute_value_8bpercentage(long base_val, short npercent);
long calculate_correct_creature_maxspeed(const struct Thing *thing);
long calculate_correct_creature_pay(const struct Thing *thing);
long calculate_correct_creature_scavenge_required(const struct Thing *thing, PlayerNumber callplyr_idx);
long compute_creature_work_value_for_room_role(const struct Thing *creatng, RoomRole rrole, long efficiency);

const char *creature_statistic_text(const struct Thing *creatng, CreatureLiveStatId cstat_id);

long calculate_damage_did_to_slab_with_single_hit(const struct Thing *diggertng, const struct SlabMap *slb);
long calculate_gold_digged_out_of_slab_with_single_hit(long damage_did_to_slab, PlayerNumber plyr_idx, unsigned short crlevel, const struct SlabMap *slb);
HitPoints calculate_shot_real_damage_to_door(const struct Thing *doortng, const struct Thing *shotng);

long get_radially_decaying_value(long magnitude,long decay_start,long decay_length,long distance);

TbBool update_creature_health_to_max(struct Thing *thing);
TbBool apply_health_to_thing(struct Thing *thing, long amount);
void apply_health_to_thing_and_display_health(struct Thing *thing, long amount);
HitPoints apply_damage_to_thing(struct Thing *thing, HitPoints dmg, DamageType damage_type, PlayerNumber dealing_plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
