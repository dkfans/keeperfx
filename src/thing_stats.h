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
#define CREATURE_MAX_LEVEL      10
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
const char *thing_model_only_name(ThingClass class_id, ThingModel model);
const char *thing_class_and_model_name(ThingClass class_id, ThingModel model);
const char *thing_class_code_name(ThingClass class_id);
const char *creatrtng_actstate_name(const struct Thing *thing);
const char *creatrtng_realstate_name(const struct Thing *thing);
TbBool things_stats_debug_dump(void);
TbBool is_neutral_thing(const struct Thing *thing);
TbBool is_hero_thing(const struct Thing *thing);
/******************************************************************************/
long compute_creature_kind_score(ThingModel crkind, CrtrExpLevel exp_level);
GoldAmount compute_creature_max_pay(GoldAmount base_pay, CrtrExpLevel exp_level);
GoldAmount compute_creature_max_training_cost(GoldAmount base_training_cost, CrtrExpLevel exp_level);
GoldAmount compute_creature_max_scavenging_cost(GoldAmount base_scavenging_cost, CrtrExpLevel exp_level);
long project_creature_attack_melee_damage(long base_param, short damage_percent, long luck, CrtrExpLevel exp_level, const struct Thing* thing);
long compute_creature_attack_melee_damage(long base_param, long luck, CrtrExpLevel exp_level, struct Thing* thing);
long compute_creature_attack_spell_damage(long base_param, long luck, CrtrExpLevel exp_level, PlayerNumber plyr_idx);
long compute_creature_attack_range(long base_param, long luck, CrtrExpLevel exp_level);
HitPoints compute_creature_spell_damage_over_time(HitPoints spell_damage, CrtrExpLevel caster_level, PlayerNumber caster_owner);
long compute_creature_work_value(long base_param, long efficiency, CrtrExpLevel exp_level);
HitPoints compute_creature_max_health(HitPoints base_health, CrtrExpLevel exp_level);
long compute_creature_max_strength(long base_param, CrtrExpLevel exp_level);
long compute_creature_max_armour(long base_param, CrtrExpLevel exp_level);
long compute_creature_max_defense(long base_param, CrtrExpLevel exp_level);
long compute_creature_max_dexterity(long base_param, CrtrExpLevel exp_level);
long compute_creature_max_loyalty(long base_param, CrtrExpLevel exp_level);
long compute_creature_max_unaffected(long base_param, CrtrExpLevel exp_level);
#define compute_creature_max_luck compute_creature_max_unaffected
long compute_controlled_speed_increase(long prev_speed, long speed_limit);
long compute_controlled_speed_decrease(long prev_speed, long speed_limit);
long compute_value_percentage(long base_val, short npercent);
GoldAmount calculate_correct_creature_pay(const struct Thing *thing);
GoldAmount calculate_correct_creature_training_cost(const struct Thing *thing);
GoldAmount calculate_correct_creature_scavenging_cost(const struct Thing *thing);
HitPoints calculate_correct_creature_max_health(const struct Thing *thing);
long calculate_correct_creature_strength(const struct Thing *thing);
long calculate_correct_creature_armour(const struct Thing *thing);
long calculate_correct_creature_defense(const struct Thing *thing);
long calculate_correct_creature_dexterity(const struct Thing *thing);
long calculate_correct_creature_maxspeed(const struct Thing *thing);
long calculate_correct_creature_loyalty(const struct Thing *thing);
long calculate_correct_creature_scavenge_required(const struct Thing *thing, PlayerNumber callplyr_idx);
long compute_creature_work_value_for_room_role(const struct Thing *creatng, RoomRole rrole, long efficiency);
long compute_creature_weight(const struct Thing* creatng);

const char *creature_statistic_text(const struct Thing *creatng, CreatureLiveStatId cstat_id);

HitPoints reduce_damage_for_midas(PlayerNumber owner, HitPoints damage, short multiplier);
long calculate_damage_did_to_slab_with_single_hit(const struct Thing *diggertng, const struct SlabMap *slb);
GoldAmount calculate_gold_digged_out_of_slab_with_single_hit(long damage_did_to_slab, const struct SlabMap *slb);
HitPoints calculate_shot_real_damage_to_door(const struct Thing *doortng, const struct Thing *shotng);

long get_radially_decaying_value(long magnitude, long decay_start, long decay_length, long distance);
long get_radially_growing_value(long magnitude, long decay_start, long decay_length, long distance, long acceleration);

TbBool update_creature_health_to_max(struct Thing *creatng);
TbBool update_relative_creature_health(struct Thing *creatng);
TbBool set_creature_health_to_max_with_heal_effect(struct Thing *thing);
TbBool apply_health_to_thing(struct Thing *thing, HitPoints amount);
void apply_health_to_thing_and_display_health(struct Thing *thing, HitPoints amount);
HitPoints apply_damage_to_thing(struct Thing *thing, HitPoints dmg, PlayerNumber dealing_plyr_idx);
HitPoints get_thing_max_health(const struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
