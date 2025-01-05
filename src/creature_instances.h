/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_instances.h
 *     Header file for creature_instances.c.
 * @par Purpose:
 *     creature_instances functions.
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
#ifndef DK_CRTRINSTANCE_H
#define DK_CRTRINSTANCE_H

#include "globals.h"
#include "creature_control.h"
#include "bflib_basics.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
enum CreatureInstances {
    CrInst_NULL = 0,
    CrInst_SWING_WEAPON_SWORD,
    CrInst_SWING_WEAPON_FIST,
    CrInst_ESCAPE,
    CrInst_FIRE_ARROW,
    CrInst_FIREBALL,
    CrInst_FIRE_BOMB,
    CrInst_FREEZE,
    CrInst_ARMOUR,
    CrInst_LIGHTNING,
    CrInst_REBOUND, // 10
    CrInst_HEAL,
    CrInst_POISON_CLOUD,
    CrInst_INVISIBILITY,
    CrInst_TELEPORT,
    CrInst_SPEED,
    CrInst_SLOW,
    CrInst_DRAIN,
    CrInst_FEAR,
    CrInst_MISSILE,
    CrInst_NAVIGATING_MISSILE, // 20
    CrInst_FLAME_BREATH,
    CrInst_WIND,
    CrInst_LIGHT,
    CrInst_FLY,
    CrInst_SIGHT,
    CrInst_GRENADE,
    CrInst_HAILSTORM,
    CrInst_WORD_OF_POWER,
    CrInst_FART,
    CrInst_DIG, // 30
    CrInst_PRETTY_PATH,
    CrInst_DESTROY_AREA,
    CrInst_TUNNEL,
    CrInst_CELEBRATE_SHORT,
    CrInst_REINFORCE,
    CrInst_EAT,
    CrInst_ATTACK_ROOM_SLAB,
    CrInst_DAMAGE_WALL,
    CrInst_FIRST_PERSON_DIG,
    CrInst_LIZARD, // 40
    CrInst_CAST_SPELL_DISEASE, // 41
    CrInst_CAST_SPELL_CHICKEN,
    CrInst_CAST_SPELL_TIME_BOMB,
    CrInst_MOAN,
    CrInst_TORTURED,
    CrInst_TOKING,
    CrInst_RELAXING,
    CrInst_FAMILIAR,
    CrInst_SUMMON,
    CrInst_RANGED_HEAL, // 50
    CrInst_RANGED_SPEED,
    CrInst_RANGED_ARMOUR,
    CrInst_RANGED_REBOUND,
    CrInst_CLEANSE,
    CrInst_LISTEND,
};

/******************************************************************************/
#pragma pack(1)

struct Thing;

typedef long (*Creature_Instf_Func)(struct Thing *, long *);
typedef TbBool (*Creature_Validate_Func)(struct Thing *, struct Thing *, CrInstance, int32_t, int32_t);
typedef TbBool (*Creature_Target_Search_Func)(struct Thing *, CrInstance, ThingIndex **, uint16_t *, int32_t, int32_t);

struct InstanceInfo {
    TbBool instant;
    long time;
    long fp_time;
    long action_time;
    long fp_action_time;
    long reset_time;
    long fp_reset_time;
    unsigned char graphics_idx;
    char postal_priority;
    short instance_property_flags;
    short force_visibility;
    unsigned char primary_target;
    unsigned char func_idx;
    long func_params[2];
    long range_min;
    long range_max;
    long symbol_spridx;
    short tooltip_stridx;
    // Refer to creature_instances_validate_func_list
    uint8_t validate_source_func;
    int32_t validate_source_func_params[2];
    uint8_t validate_target_func;
    int32_t validate_target_func_params[2];
    // Refer to creature_instances_search_targets_func_list
    uint8_t search_func;
    int32_t search_func_params[2];
};

/******************************************************************************/

#pragma pack()
/******************************************************************************/
extern const struct NamedCommand creature_instances_func_type[];
extern Creature_Instf_Func creature_instances_func_list[];
extern const struct NamedCommand creature_instances_validate_func_type[];
extern Creature_Validate_Func creature_instances_validate_func_list[];
extern const struct NamedCommand creature_instances_search_targets_func_type[];
extern Creature_Target_Search_Func creature_instances_search_targets_func_list[];
/******************************************************************************/
/** Returns creature instance info structure for given instance index. */
#define creature_instance_info_get(inst_idx) creature_instance_info_get_f(inst_idx,__func__)
struct InstanceInfo *creature_instance_info_get_f(CrInstance inst_idx,const char *func_name);
void process_creature_instance(struct Thing *thing);
TbBool process_creature_self_spell_casting(struct Thing* thing);
CrInstance process_creature_ranged_buff_spell_casting(struct Thing* thing);

TbBool creature_instance_info_invalid(const struct InstanceInfo *inst_inf);
TbBool creature_instance_is_available(const struct Thing *thing, CrInstance inum);

TbBool creature_choose_first_available_instance(struct Thing *thing);
void creature_increase_available_instances(struct Thing *thing);
TbBool creature_has_ranged_weapon(const struct Thing *thing);
TbBool creature_has_disarming_weapon(const struct Thing* creatng);
TbBool creature_has_ranged_object_weapon(const struct Thing *creatng);
TbBool creature_has_weapon_for_postal(const struct Thing *creatng);
TbBool creature_has_melee_attack(const struct Thing *creatng);

int creature_instance_get_available_pos_for_id(struct Thing *thing, CrInstance req_inst_id);
int creature_instance_get_available_number_for_pos(struct Thing *thing, int req_avail_pos);
CrInstance creature_instance_get_available_id_for_pos(struct Thing *thing, int req_avail_pos);

TbBool instance_draws_possession_swipe(CrInstance inum);

void delay_teleport(struct Thing *creatng);
void delay_heal_sleep(struct Thing *creatng);
/******************************************************************************/
TbBool validate_source_basic(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_source_generic(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_source_even_in_prison(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);

TbBool validate_target_basic(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_generic(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_even_in_prison(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_benefits_from_missile_defense(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_benefits_from_defensive(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_benefits_from_higher_altitude(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_benefits_from_offensive(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_benefits_from_wind(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_benefits_from_healing(struct Thing *source, struct Thing *target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_non_idle(struct Thing* source, struct Thing* target, CrInstance inst_idx, int32_t param1, int32_t param2);
TbBool validate_target_takes_gas_damage(struct Thing* source, struct Thing* target, CrInstance inst_idx, int32_t param1, int32_t param2);

TbBool search_target_generic(struct Thing *source, CrInstance inst_idx, ThingIndex **targets, uint16_t *found_count, int32_t param1, int32_t param2);
TbBool search_target_ranged_heal(struct Thing *source, CrInstance inst_idx, ThingIndex **targets, uint16_t *found_count, int32_t param1, int32_t param2);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
