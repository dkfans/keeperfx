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
int32_t compute_creature_kind_score(ThingModel crkind, CrtrExpLevel exp_level);
GoldAmount compute_creature_max_pay(GoldAmount base_pay, CrtrExpLevel exp_level);
GoldAmount compute_creature_max_training_cost(GoldAmount base_training_cost, CrtrExpLevel exp_level);
GoldAmount compute_creature_max_scavenging_cost(GoldAmount base_scavenging_cost, CrtrExpLevel exp_level);
int32_t project_creature_attack_melee_damage(int32_t base_param, short damage_percent, int32_t luck, CrtrExpLevel exp_level, const struct Thing* thing);
int32_t compute_creature_attack_melee_damage(int32_t base_param, int32_t luck, CrtrExpLevel exp_level, struct Thing* thing);
int32_t compute_creature_attack_spell_damage(int32_t base_param, int32_t luck, CrtrExpLevel exp_level, PlayerNumber plyr_idx);
int32_t compute_creature_attack_range(int32_t base_param, int32_t luck, CrtrExpLevel exp_level);
HitPoints compute_creature_spell_damage_over_time(HitPoints spell_damage, CrtrExpLevel caster_level, PlayerNumber caster_owner);
int32_t compute_creature_work_value(int32_t base_param, int32_t efficiency, CrtrExpLevel exp_level);
HitPoints compute_creature_max_health(HitPoints base_health, CrtrExpLevel exp_level);
int32_t compute_creature_max_strength(int32_t base_param, CrtrExpLevel exp_level);
int32_t compute_creature_max_armour(int32_t base_param, CrtrExpLevel exp_level);
int32_t compute_creature_max_defense(int32_t base_param, CrtrExpLevel exp_level);
int32_t compute_creature_max_dexterity(int32_t base_param, CrtrExpLevel exp_level);
int32_t compute_creature_max_loyalty(int32_t base_param, CrtrExpLevel exp_level);
int32_t compute_creature_max_unaffected(int32_t base_param, CrtrExpLevel exp_level);
#define compute_creature_max_luck compute_creature_max_unaffected
int32_t compute_controlled_speed_increase(int32_t prev_speed, int32_t speed_limit);
int32_t compute_controlled_speed_decrease(int32_t prev_speed, int32_t speed_limit);
int32_t compute_value_percentage(int32_t base_val, short npercent);
GoldAmount calculate_correct_creature_pay(const struct Thing *thing);
GoldAmount calculate_correct_creature_training_cost(const struct Thing *thing);
GoldAmount calculate_correct_creature_scavenging_cost(const struct Thing *thing);
HitPoints calculate_correct_creature_max_health(const struct Thing *thing);
int32_t calculate_correct_creature_strength(const struct Thing *thing);
int32_t calculate_correct_creature_armour(const struct Thing *thing);
int32_t calculate_correct_creature_defense(const struct Thing *thing);
int32_t calculate_correct_creature_dexterity(const struct Thing *thing);
int32_t calculate_correct_creature_maxspeed(const struct Thing *thing);
int32_t calculate_correct_creature_loyalty(const struct Thing *thing);
int32_t calculate_correct_creature_scavenge_required(const struct Thing *thing, PlayerNumber callplyr_idx);
int32_t compute_creature_work_value_for_room_role(const struct Thing *creatng, RoomRole rrole, int32_t efficiency);
int32_t compute_creature_weight(const struct Thing* creatng);

const char *creature_statistic_text(const struct Thing *creatng, CreatureLiveStatId cstat_id);

HitPoints reduce_damage_for_midas(PlayerNumber owner, HitPoints damage, short multiplier);
int32_t calculate_damage_did_to_slab_with_single_hit(const struct Thing *diggertng, const struct SlabMap *slb);
GoldAmount calculate_gold_digged_out_of_slab_with_single_hit(int32_t damage_did_to_slab, const struct SlabMap *slb);
HitPoints calculate_shot_real_damage_to_door(const struct Thing *doortng, const struct Thing *shotng);
HitPoints collide_door_and_boulder(struct Thing* doortng, struct Thing *boulder);

int32_t get_radially_decaying_value(int32_t magnitude, int32_t decay_start, int32_t decay_length, int32_t distance);
int32_t get_radially_growing_value(int32_t magnitude, int32_t decay_start, int32_t decay_length, int32_t distance, int32_t acceleration);

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
