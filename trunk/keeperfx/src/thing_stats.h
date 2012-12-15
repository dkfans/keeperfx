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
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct SlabMap;

#pragma pack()
/******************************************************************************/
extern const char *blood_types[];
/******************************************************************************/
const char *thing_model_name(const struct Thing *thing);
const char *thing_class_code_name(long class_id);
const char *creatrtng_actstate_name(const struct Thing *thing);
TbBool things_stats_debug_dump(void);
TbBool is_neutral_thing(const struct Thing *thing);
/******************************************************************************/
long compute_creature_kind_score(ThingModel crkind,unsigned short crlevel);
long compute_creature_max_pay(long base_pay,unsigned short crlevel);
long compute_creature_max_health(long base_health,unsigned short crlevel);
long compute_creature_attack_damage(long base_param,long luck,unsigned short crlevel);
long project_creature_attack_damage(long base_param,long luck,unsigned short crlevel);
long compute_creature_attack_range(long base_param,long luck,unsigned short crlevel);
long compute_creature_work_value(long base_param,long efficiency,unsigned short crlevel);
long compute_creature_max_sparameter(long base_param,unsigned short crlevel);
long compute_creature_max_dexterity(long base_param,unsigned short crlevel);
long compute_creature_max_defense(long base_param,unsigned short crlevel);
long compute_creature_max_strength(long base_param,unsigned short crlevel);
long compute_creature_max_unaffected(long base_param,unsigned short crlevel);
#define compute_creature_max_luck compute_creature_max_unaffected
#define compute_creature_max_armour compute_creature_max_unaffected
long compute_controlled_speed_increase(long prev_speed, long speed_limit);
long compute_controlled_speed_decrease(long prev_speed, long speed_limit);
long compute_value_percentage(long base_val, short npercent);
long compute_value_8bpercentage(long base_val, short npercent);
long calculate_correct_creature_maxspeed(const struct Thing *thing);
long calculate_correct_creature_pay(const struct Thing *thing);

long calculate_damage_did_to_slab_with_single_hit(const struct Thing *diggertng, const struct SlabMap *slb);
long calculate_gold_digged_out_of_slab_with_single_hit(long damage_did_to_slab, PlayerNumber plyr_idx, unsigned short crlevel, const struct SlabMap *slb);

long get_radially_decaying_value(long magnitude,long decay_start,long decay_length,long distance);

TbBool update_creature_health_to_max(struct Thing *thing);
void apply_health_to_thing_and_display_health(struct Thing *thing, long amount);
void apply_damage_to_thing(struct Thing *thing, long a2, char a3);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
