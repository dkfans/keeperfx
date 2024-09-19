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
    CrInst_LISTEND,
};

/******************************************************************************/
#pragma pack(1)

struct Thing;

typedef long (*Creature_Instf_Func)(struct Thing *, long *);
typedef TbBool (*Creature_Validate_Func)(struct Thing *, struct Thing *, CrInstance);
typedef TbBool (*Creature_Target_Search_Func)(struct Thing *, CrInstance, struct Thing **, short *);

struct InstanceInfo {
    TbBool instant;
    long time;
    long fp_time;
    long action_time;
    long fp_action_time;
    long reset_time;
    long fp_reset_time;
    unsigned char graphics_idx;
    short instance_property_flags;
    short force_visibility;
    unsigned char primary_target;
    unsigned char func_idx;
    long func_params[2];
    long range_min;
    long range_max;
    long symbol_spridx;
    short tooltip_stridx;
    // [0] for source, [1] for target. Refer to creature_instances_validate_func_list
    unsigned char validate_func_idx[2];
    // Refer to creature_instances_search_targets_func_list
    unsigned char search_func_idx;
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
long process_creature_self_spell_casting(struct Thing* thing);
CrInstance process_creature_ranged_buff_spell_casting(struct Thing* thing);

TbBool creature_instance_info_invalid(const struct InstanceInfo *inst_inf);
TbBool creature_instance_is_available(const struct Thing *thing, CrInstance inum);

TbBool creature_choose_first_available_instance(struct Thing *thing);
void creature_increase_available_instances(struct Thing *thing);
TbBool creature_has_ranged_weapon(const struct Thing *thing);
TbBool creature_has_disarming_weapon(const struct Thing* creatng);
TbBool creature_has_ranged_object_weapon(const struct Thing *creatng);
TbBool creature_has_quick_range_weapon(const struct Thing *creatng);
TbBool creature_has_melee_attack(const struct Thing *creatng);

int creature_instance_get_available_pos_for_id(struct Thing *thing, CrInstance req_inst_id);
int creature_instance_get_available_number_for_pos(struct Thing *thing, int req_avail_pos);
CrInstance creature_instance_get_available_id_for_pos(struct Thing *thing, int req_avail_pos);

TbBool instance_draws_possession_swipe(CrInstance inum);

void delay_teleport(struct Thing *creatng);
void delay_heal_sleep(struct Thing *creatng);
/******************************************************************************/
TbBool validate_source_generic(struct Thing *source, struct Thing *target, CrInstance inst_idx);
TbBool validate_target_generic(struct Thing *source, struct Thing *target, CrInstance inst_idx);
TbBool validate_source_ranged_heal(struct Thing *source, struct Thing *target, CrInstance inst_idx);
TbBool validate_target_ranged_heal(struct Thing *source, struct Thing *target, CrInstance inst_idx);

TbBool search_target_generic(struct Thing *source, CrInstance inst_idx, struct Thing **targets, short *found_count);
TbBool search_target_ranged_heal(struct Thing *source, CrInstance inst_idx, struct Thing **targets, short *found_count);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
