/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_instances.c
 *     creature_instances support functions.
 * @par Purpose:
 *     Functions to creature_instances.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 11 Sep 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "creature_instances.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sound.h"

#include "bflib_math.h"
#include "thing_list.h"
#include "thing_creature.h"
#include "thing_effects.h"
#include "thing_traps.h"
#include "thing_stats.h"
#include "thing_shots.h"
#include "thing_navigate.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_effects.h"
#include "power_specials.h"
#include "room_data.h"
#include "room_util.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "spdigger_stack.h"
#include "config_magic.h"
#include "config_terrain.h"
#include "gui_soundmsgs.h"
#include "sounds.h"
#include "game_legacy.h"
#include "player_instances.h"
#include "gui_msgs.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CREATURES_SEARCHED 100

/******************************************************************************/
long instf_attack_room_slab(struct Thing *creatng, int32_t *param);
long instf_creature_cast_spell(struct Thing *creatng, int32_t *param);
long instf_creature_fire_shot(struct Thing *creatng, int32_t *param);
long instf_damage_wall(struct Thing *creatng, int32_t *param);
long instf_destroy(struct Thing *creatng, int32_t *param);
long instf_dig(struct Thing *creatng, int32_t *param);
long instf_eat(struct Thing *creatng, int32_t *param);
long instf_fart(struct Thing *creatng, int32_t *param);
long instf_first_person_do_imp_task(struct Thing *creatng, int32_t *param);
long instf_pretty_path(struct Thing *creatng, int32_t *param);
long instf_reinforce(struct Thing *creatng, int32_t *param);
long instf_tortured(struct Thing *creatng, int32_t *param);
long instf_tunnel(struct Thing *creatng, int32_t *param);

const struct NamedCommand creature_instances_func_type[] = {
  {"attack_room_slab",         1},
  {"creature_cast_spell",      2},
  {"creature_fire_shot",       3},
  {"creature_damage_wall",     4},
  {"creature_destroy",         5},
  {"creature_dig",             6},
  {"creature_eat",             7},
  {"creature_fart",            8},
  {"first_person_do_imp_task", 9},
  {"creature_pretty_path",     10},
  {"creature_reinforce",       11},
  {"creature_tortured",        12},
  {"creature_tunnel",          13},
  {"none",                     14},
  {NULL,                       0},
};

Creature_Instf_Func creature_instances_func_list[] = {
  NULL,
  instf_attack_room_slab,
  instf_creature_cast_spell,
  instf_creature_fire_shot,
  instf_damage_wall,
  instf_destroy,
  instf_dig,
  instf_eat,
  NULL,
  instf_first_person_do_imp_task,
  instf_pretty_path, //[10]
  instf_reinforce,
  instf_tortured,
  instf_tunnel,
  NULL,
  NULL,
};

const struct NamedCommand creature_instances_validate_func_type[] = {
    {"validate_source_generic",                                 1},
    {"validate_source_even_in_prison",                          2},
    {"validate_target_generic",                                 3},
    {"validate_target_even_in_prison",                          4},
    {"validate_target_benefits_from_missile_defense",           5},
    {"validate_target_benefits_from_defensive",                 6},
    {"validate_target_benefits_from_healing",                   7},
    {"validate_target_benefits_from_higher_altitude",           8},
    {"validate_target_benefits_from_offensive",                 9},
    {"validate_target_benefits_from_wind",                      10},
    {"validate_target_non_idle",                                11},
    {"validate_target_takes_gas_damage",                        12},
    {NULL, 0},
};

Creature_Validate_Func creature_instances_validate_func_list[] = {
    NULL,
    validate_source_generic,
    validate_source_even_in_prison,
    validate_target_generic,
    validate_target_even_in_prison,
    validate_target_benefits_from_missile_defense,
    validate_target_benefits_from_defensive,
    validate_target_benefits_from_healing,
    validate_target_benefits_from_higher_altitude,
    validate_target_benefits_from_offensive,
    validate_target_benefits_from_wind,
    validate_target_non_idle,
    validate_target_takes_gas_damage,
    NULL,
};

const struct NamedCommand creature_instances_search_targets_func_type[] = {
    {"search_target_generic",        1},
    {"search_target_ranged_heal",    2},
    {NULL,                           0},
};

Creature_Target_Search_Func creature_instances_search_targets_func_list[] = {
    NULL,
    search_target_generic,
    search_target_ranged_heal,
    NULL,
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/** Returns creature instance info structure for given instance index.
 *
 * @param inst_idx Instance index.
 * @param func_name Name of the caller function, for logging purposes.
 * @return Instance Info struct is returned.
 */
struct InstanceInfo *creature_instance_info_get_f(CrInstance inst_idx,const char *func_name)
{
    if ((inst_idx < 0) || (inst_idx >= game.conf.crtr_conf.instances_count))
    {
        ERRORMSG("%s: Tried to get invalid instance info %d!",func_name,(int)inst_idx);
        return &game.conf.magic_conf.instance_info[0];
    }
    return &game.conf.magic_conf.instance_info[inst_idx];
}

TbBool creature_instance_info_invalid(const struct InstanceInfo *inst_inf)
{
    return (inst_inf < &game.conf.magic_conf.instance_info[1]);
}

TbBool creature_instance_is_available(const struct Thing *thing, CrInstance inst_id)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    return cctrl->instance_available[inst_id];
}

TbBool creature_choose_first_available_instance(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    for (long i = 0; i < LEARNED_INSTANCES_COUNT; i++)
    {
        long k = crconf->learned_instance_id[i];
        if (k > 0)
        {
            if (cctrl->instance_available[k]) {
                cctrl->active_instance_id = k;
                return true;
            }
        }
    }
    cctrl->active_instance_id = CrInst_NULL;
    return false;
}

void creature_increase_available_instances(struct Thing *thing)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    for (int i = 0; i < LEARNED_INSTANCES_COUNT; i++)
    {
        int k = crconf->learned_instance_id[i];
        if (k > 0)
        {
            if (crconf->learned_instance_level[i] <= cctrl->exp_level+1) {
                cctrl->instance_available[k] = true;
            }
            else if ( (crconf->learned_instance_level[i] > cctrl->exp_level+1) && !(game.conf.rules[thing->owner].game.classic_bugs_flags & ClscBug_RebirthKeepsSpells) )
            {
                cctrl->instance_available[k] = false;
            }
        }
    }
}

/**
 * For position in compacted list of instances, gives instance ID from availability list.
 * Compacted list of instances is a list of available creature instances without holes.
 * @param thing
 * @param req_avail_pos
 * @return
 */
CrInstance creature_instance_get_available_id_for_pos(struct Thing *thing, int req_avail_pos)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    int avail_pos = 0;
    for (int avail_num = 0; avail_num < LEARNED_INSTANCES_COUNT; avail_num++)
    {
        CrInstance inst_id = crconf->learned_instance_id[avail_num];
        if (creature_instance_is_available(thing, inst_id))
        {
            if (avail_pos == req_avail_pos) {
                return inst_id;
            }
            avail_pos++;
        }
    }
    return CrInst_NULL;
}

TbBool instance_is_disarming_weapon(CrInstance inum)
{
    struct InstanceInfo* inst_inf = creature_instance_info_get(inum);
    return ((inst_inf->instance_property_flags & InstPF_Disarming) != 0);
}

TbBool instance_draws_possession_swipe(CrInstance inum)
{
    struct InstanceInfo* inst_inf = creature_instance_info_get(inum);
    return ((inst_inf->instance_property_flags & InstPF_UsesSwipe) != 0);
}

TbBool instance_is_ranged_weapon(CrInstance inum)
{
    struct InstanceInfo* inst_inf = creature_instance_info_get(inum);
    return ((inst_inf->instance_property_flags & InstPF_RangedAttack) != 0);
}

TbBool instance_is_ranged_weapon_vs_objects(CrInstance inum)
{
    struct InstanceInfo* inst_inf = creature_instance_info_get(inum);
    return (((inst_inf->instance_property_flags & InstPF_RangedAttack) != 0) && ((inst_inf->instance_property_flags & InstPF_Destructive) != 0) && !(inst_inf->instance_property_flags & InstPF_Dangerous));
}

/**
 * Informs whether the creature has an instance which can be used when going postal.
 * Going Postal is the behavior where creatures attack others at their job, like warlocks in the library
  * @return True if it has a postal_priority value > 0.
 */
TbBool instance_is_used_for_going_postal(CrInstance inum)
{
    struct InstanceInfo* inst_inf = creature_instance_info_get(inum);
    return (inst_inf->postal_priority > 0);
}

TbBool instance_is_melee_attack(CrInstance inum)
{
    struct InstanceInfo* inst_inf = creature_instance_info_get(inum);
    return ((inst_inf->instance_property_flags & InstPF_MeleeAttack) != 0);
}

/**
 * Informs whether the creature has an instance which is ranged weapon useable against other creatures.
 * The instances currently in use and currently in cooldown are included.
 * @param creatng The creature to be checked.
 * @return True if the creature has ranged weapon, false otherwise.
 */
TbBool creature_has_ranged_weapon(const struct Thing *creatng)
{
    TRACE_THING(creatng);
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    for (long inum = 1; inum < game.conf.crtr_conf.instances_count; inum++)
    {
        if (cctrl->instance_available[inum] > 0)
        {
            if (instance_is_ranged_weapon(inum))
                return true;
        }
    }
    return false;
}

/**
 * Informs whether the creature has an instance which is ranged weapon useable against other creatures.
 * The instances currently in use and currently in cooldown are included.
 * @param creatng The creature to be checked.
 * @return True if the creature has ranged weapon, false otherwise.
 */
TbBool creature_has_disarming_weapon(const struct Thing* creatng)
{
    TRACE_THING(creatng);
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    for (long inum = 1; inum < game.conf.crtr_conf.instances_count; inum++)
    {
        if (cctrl->instance_available[inum] > 0)
        {
            if (instance_is_disarming_weapon(inum))
                return true;
        }
    }
    return false;
}

/**
 * Informs whether the creature has an instance which is ranged weapon useable against objects.
 * The instances currently in use and currently in cooldown are included.
 * @param creatng The creature to be checked.
 * @return True if the creature has ranged weapon, false otherwise.
 */
TbBool creature_has_ranged_object_weapon(const struct Thing *creatng)
{
    TRACE_THING(creatng);
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    for (long inum = 1; inum < game.conf.crtr_conf.instances_count; inum++)
    {
        if (cctrl->instance_available[inum])
        {
            if (instance_is_ranged_weapon_vs_objects(inum))
                return true;
        }
    }
    return false;
}

TbBool creature_has_weapon_for_postal(const struct Thing *creatng)
{
    TRACE_THING(creatng);
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    for (long inum = 1; inum < game.conf.crtr_conf.instances_count; inum++)
    {
        if (cctrl->instance_available[inum])
        {
            if (instance_is_used_for_going_postal(inum))
                return true;
        }
    }
    return false;
}

/**
 * Informs whether the creature has a mêlée attack.
 * The instances currently in use and currently in cooldown are included.
 * @param creatng The creature to be checked.
 * @return True if the creature has mêlée attack, false otherwise.
 */
TbBool creature_has_melee_attack(const struct Thing *creatng)
{
    TRACE_THING(creatng);
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    for (long inum = 1; inum < game.conf.crtr_conf.instances_count; inum++)
    {
        if (cctrl->instance_available[inum] > 0)
        {
            if (instance_is_melee_attack(inum))
                return true;
        }
    }
    return false;
}

void process_creature_instance(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    SYNCDBG(19, "Starting for %s index %d instance %d", thing_model_name(thing), (int)thing->index, (int)cctrl->instance_id);
    if (cctrl->inst_turn >= cctrl->inst_total_turns)
    {
        if (!cctrl->inst_repeat)
        {
            SYNCDBG(18,"Finalize %s for %s index %d.",creature_instance_code_name(cctrl->instance_id),thing_model_name(thing),(int)thing->index);
            cctrl->instance_id = CrInst_NULL;
            thing->creature.volley_fire = false;
            return;
        }
    }
    cctrl->inst_repeat = 0;
    if (cctrl->instance_id != CrInst_NULL)
    {
        cctrl->inst_turn++;
        if (cctrl->inst_turn == cctrl->inst_action_turns)
        {
            struct InstanceInfo* inst_inf = creature_instance_info_get(cctrl->instance_id);
            if (creature_instances_func_list[inst_inf->func_idx] != NULL)
            {
                SYNCDBG(18,"Executing %s for %s index %d.",creature_instance_code_name(cctrl->instance_id),thing_model_name(thing),(int)thing->index);
                creature_instances_func_list[inst_inf->func_idx](thing, inst_inf->func_params);
                if (thing->creature.volley_repeat > 0)
                {
                    return;
                }
            }
        }
        if (cctrl->inst_repeat)
        {
            cctrl->inst_turn--;
        }
        cctrl->inst_repeat = 0;
    }
}

long instf_creature_fire_shot(struct Thing *creatng, int32_t *param)
{
    struct Thing *target;
    int hittype;
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->targtng_idx == 0)
    {
        if ((creatng->alloc_flags & TAlF_IsControlled) == 0)
        {
            hittype = THit_CrtrsOnlyNotOwn;
        }
        else
            hittype = THit_CrtrsNObjcts;
    }
    else if ((creatng->alloc_flags & TAlF_IsControlled) != 0)
    {
        target = thing_get(cctrl->targtng_idx);
        TRACE_THING(target);
        if (target->class_id == TCls_Object)
            hittype = THit_CrtrsNObjcts;
        else if (target->class_id == TCls_Trap)
            hittype = THit_TrapsAll;
        else
            hittype = THit_CrtrsOnly;
    }
    else
    {
        target = thing_get(cctrl->targtng_idx);
        TRACE_THING(target);
        if (target->class_id == TCls_Object)
            hittype = THit_CrtrsNObjctsNotOwn;
        else if (thing_is_destructible_trap(target) > 0)
            hittype = THit_CrtrsNObjctsNotOwn;
        else if (target->class_id == TCls_Trap)
            hittype = THit_TrapsAll;
        else if (target->owner == creatng->owner)
            hittype = THit_CrtrsOnlyOwn;
        else
            hittype = THit_CrtrsOnlyNotOwn;
    }
    if (cctrl->targtng_idx > 0)
    {
        target = thing_get(cctrl->targtng_idx);
        SYNCDBG(8,"The %s index %d fires %s at %s index %d",thing_model_name(creatng),(int)creatng->index,shot_code_name(*param),thing_model_name(target),(int)target->index);
        TRACE_THING(target);
    } else
    {
        target = NULL;
        SYNCDBG(8,"The %s index %d fires %s",thing_model_name(creatng),(int)creatng->index,shot_code_name(*param));
    }
    thing_fire_shot(creatng, target, *param, 1, hittype);
    // Start cooldown after shot is fired
    cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn;
    return 0;
}

long instf_creature_cast_spell(struct Thing *creatng, int32_t *param)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long spl_idx = param[0];
    struct SpellConfig* spconf = get_spell_config(spl_idx);
    struct Thing* target = INVALID_THING;

    SYNCDBG(8,"The %s(%d) casts %s at %d", thing_model_name(creatng), (int)creatng->index,
        spell_code_name(spl_idx), cctrl->targtng_idx);

    if (spconf->cast_at_thing && cctrl->targtng_idx != creatng->index)
    {
        // If the targtng_idx is just the caster itself, we can call creature_cast_spell
        // instead of creature_cast_spell_at_thing.
        target = thing_get(cctrl->targtng_idx);
    }

    if (!thing_is_invalid(target))
    {
        creature_cast_spell_at_thing(creatng, target, spl_idx, cctrl->exp_level);
    }
    else
    {
        creature_cast_spell(creatng, spl_idx, cctrl->exp_level, cctrl->targtstl_x, cctrl->targtstl_y);
    }

    // Start cooldown after spell effect activates
    cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn;
    return 0;
}

TbBool process_creature_self_spell_casting(struct Thing* creatng)
{
    TRACE_THING(creatng);
    CrInstance inst_idx = get_self_spell_casting(creatng);
    if (inst_idx == CrInst_NULL) {
        return false;
    }
    SYNCDBG(9, "%s(%d) use %s(%d) on itself.", thing_model_name(creatng), creatng->index,
        creature_instance_code_name(inst_idx), inst_idx);
    set_creature_instance(creatng, inst_idx, creatng->index, 0);
    return true;
}

/**
 * @brief Check whether the given creature is suitable to cast ranged buff spell.
 * This function is used for both combat and non-combat situations.
 *
 * @param creatng The creature being checked.
 * @return CrInstance The instance index being set.
 */
CrInstance process_creature_ranged_buff_spell_casting(struct Thing* creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(8,"Processing %s(%d), act.st: %s, con.st: %s", thing_model_name(creatng), creatng->index,
        creature_state_code_name(creatng->active_state), creature_state_code_name(creatng->continue_state));
    CrInstance i = CrInst_NULL + 1;
    for(; i < game.conf.crtr_conf.instances_count; i++ )
    {
        const struct InstanceInfo* inst_inf = creature_instance_info_get(i);
        if(creature_instance_info_invalid(inst_inf) || (inst_inf->instance_property_flags & InstPF_RangedBuff) == 0)
        {
            continue;
        }
        if((inst_inf->validate_source_func == 0) || (inst_inf->validate_target_func == 0) ||
           (inst_inf->search_func == 0))
        {
            ERRORLOG("The instance %d has no validate function or search function.", i);
            continue;
        }
        if(!creature_instances_validate_func_list[inst_inf->validate_source_func]
            (creatng, NULL, i, inst_inf->validate_source_func_params[0], inst_inf->validate_source_func_params[1]))
        {
            // The input creature is not a legal source.
            continue;
        }

        ThingIndex *targets = NULL;
        unsigned short found_count = 0;
        TbBool ok = creature_instances_search_targets_func_list[inst_inf->search_func](creatng, i, &targets,
            &found_count, inst_inf->search_func_params[0], inst_inf->search_func_params[1]);
        if(ok && targets && (found_count > 0))
        {
            struct Thing* target = thing_get(targets[0]);
            SYNCDBG(8, "Set instance %s(%d) on %s(%d)(%d) for %s(%d)(%d).",
                creature_instance_code_name(i), i, thing_model_name(creatng), creatng->index, creatng->owner,
                thing_model_name(target), target->index, target->owner);

            struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
            // Enter the temporary state.
            cctrl->active_state_bkp = creatng->active_state;
            cctrl->continue_state_bkp = creatng->continue_state;
            internal_set_thing_state(creatng, CrSt_CreatureCastingPreparation);

            // Apply the spell instance to the first one since we have no group buff yet.
            set_creature_instance(creatng, i, target->index, NULL);
            free(targets);
            break; // No need to check the next spell instance.

        }
        free(targets);
    }

    return (i < game.conf.crtr_conf.instances_count) ? i : CrInst_NULL;
}

long instf_dig(struct Thing *creatng, int32_t *param)
{
    long stl_x;
    long stl_y;
    long taskkind;
    long gold;
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    long task_idx = cctrl->digger.task_idx;
    {
        struct MapTask* task = get_dungeon_task_list_entry(dungeon, task_idx);
        taskkind = task->kind;
        if (task->coords != cctrl->digger.task_stl)
        {
            return 0;
      }
      stl_x = stl_num_decode_x(cctrl->digger.task_stl);
      stl_y = stl_num_decode_y(cctrl->digger.task_stl);
    }
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb)) {
        return 0;
    }
    long dig_damage = calculate_damage_did_to_slab_with_single_hit(creatng, slb);
    if ((slb->health > dig_damage) || slab_kind_is_indestructible(slb->kind))
    {
        if (!slab_kind_is_indestructible(slb->kind))
            slb->health -= dig_damage;
        struct ShotConfigStats* shotst = get_shot_model_stats(ShM_Dig);
        thing_play_sample(creatng, shotst->dig.sndsample_idx + SOUND_RANDOM(shotst->dig.sndsample_range), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        create_effect(&creatng->mappos, shotst->dig.effect_model, creatng->owner);
        if (taskkind == SDDigTask_MineGold)
        {
            gold = calculate_gold_digged_out_of_slab_with_single_hit(dig_damage, slb);
            creatng->creature.gold_carried += gold;
            dungeon->lvstats.gold_mined += gold;
        }
        return 0;
    }
    // slb->health <= dig_damage - we're going to destroy the slab
    remove_from_task_list(creatng->owner, task_idx);
    if (taskkind == SDDigTask_MineGold)
    {
        if (!slab_kind_is_indestructible(slb->kind))
            slb->health -= dig_damage; // otherwise, we won't get the final lot of gold
        gold = calculate_gold_digged_out_of_slab_with_single_hit(dig_damage, slb);
        creatng->creature.gold_carried += gold;
        dungeon->lvstats.gold_mined += gold;
        mine_out_block(stl_x, stl_y, creatng->owner);
        if (dig_has_revealed_area(stl_x, stl_y, creatng->owner))
        {
            EventIndex evidx = event_create_event_or_update_nearby_existing_event(
                subtile_coord_center(stl_x), subtile_coord_center(stl_y),
                EvKind_AreaDiscovered, creatng->owner, 0);
            if ((evidx > 0) && is_my_player_number(creatng->owner))
                output_message(SMsg_DugIntoNewArea, 0);
        }
    } else
    if (taskkind == SDDigTask_DigEarth)
    {
        dig_out_block(stl_x, stl_y, creatng->owner);

        if (dig_has_revealed_area(stl_x, stl_y, creatng->owner))
        {
            EventIndex evidx = event_create_event_or_update_nearby_existing_event(
                subtile_coord_center(stl_x), subtile_coord_center(stl_y),
                EvKind_AreaDiscovered, creatng->owner, 0);
            if ((evidx > 0) && is_my_player_number(creatng->owner))
                output_message(SMsg_DugIntoNewArea, 0);
        }
    }
    check_map_explored(creatng, stl_x, stl_y);
    thing_play_sample(creatng, 72 + SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    return 1;
}

long instf_destroy(struct Thing *creatng, int32_t *param)
{
    TRACE_THING(creatng);
    MapSlabCoord slb_x = subtile_slab(creatng->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab(creatng->mappos.y.stl.num);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct Room* room = room_get(slb->room_index);
    long prev_owner = slabmap_owner(slb);
    struct PlayerInfo* player;
    player = get_my_player();
    int volume = 32;

    if ( !room_is_invalid(room) && (prev_owner != creatng->owner) )
    {
        if (room->health > 1)
        {
            room->health--;
            if ((player->view_type == PVT_CreatureContrl) || (player->view_type == PVT_CreaturePasngr))
            {
                volume = FULL_LOUDNESS;
            }
            thing_play_sample(creatng, 5 + SOUND_RANDOM(2), 200, 0, 3, 0, 2, volume);
            return 0;
        }
        clear_dig_on_room_slabs(room, creatng->owner);
        if (room->owner == game.neutral_player_num)
        {
            remove_room_from_players_list(room, game.neutral_player_num);
            claim_room(room, creatng);
        } else
        {
            MapCoord ccor_x = subtile_coord_center(room->central_stl_x);
            MapCoord ccor_y = subtile_coord_center(room->central_stl_y);
            event_create_event_or_update_nearby_existing_event(ccor_x, ccor_y, EvKind_RoomLost, room->owner, room->kind);
            claim_enemy_room(room, creatng);
        }
        thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        create_effects_on_room_slabs(room, imp_spangle_effects[get_player_color_idx(creatng->owner)], 0, creatng->owner);
        return 0;
    }
    if (slb->health > 1)
    {
        slb->health--;
        if ((player->view_type == PVT_CreatureContrl) || (player->view_type == PVT_CreaturePasngr))
        {
            volume = FULL_LOUDNESS;
        }
        thing_play_sample(creatng, 128 + SOUND_RANDOM(3), 200, 0, 3, 0, 2, volume);
        return 0;
    }
    if (prev_owner != game.neutral_player_num) {
        struct Dungeon* prev_dungeon = get_dungeon(prev_owner);
        prev_dungeon->lvstats.territory_lost++;
    }
    if ((player->view_type == PVT_CreatureContrl) || (player->view_type == PVT_CreaturePasngr))
    {
        volume = FULL_LOUDNESS;
    }
    thing_play_sample(creatng, 128 + SOUND_RANDOM(3), 200, 0, 3, 0, 2, volume);
    decrease_dungeon_area(prev_owner, 1);
    neutralise_enemy_block(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, creatng->owner);
    remove_traps_around_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), NULL);
    switch_owned_objects_on_destoyed_slab_to_neutral(slb_x, slb_y, prev_owner);
    dungeon->lvstats.territory_destroyed++;
    return 1;
}

long instf_attack_room_slab(struct Thing *creatng, int32_t *param)
{
    TRACE_THING(creatng);
    struct Room* room = get_room_thing_is_on(creatng);
    if (room_is_invalid(room))
    {
        ERRORLOG("The %s index %d is not on room",thing_model_name(creatng),(int)creatng->index);
        return 0;
    }
    if (room_cannot_vandalise(room->kind))
    {
        set_start_state(creatng);
        return 0; // Stop the creature from vandalizing the room if the player managed to move it from a breakable room to one that cannot be vandalized.
    }
    SYNCDBG(8,"Executing for %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct SlabMap* slb = get_slabmap_thing_is_on(creatng);
    if (slb->health > 2)
    {
        //TODO CONFIG damage made to room slabs is constant - doesn't look good
        slb->health -= 2;
        thing_play_sample(creatng, 128 + SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        return 1;
    }
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_dungeon(room->owner);
        dungeon->rooms_destroyed++;
    }
    if (count_slabs_of_room_type(room->owner, room->kind) <= 1)
    {
        event_create_event_or_update_nearby_existing_event(coord_slab(creatng->mappos.x.val), coord_slab(creatng->mappos.y.val), EvKind_RoomLost, room->owner, room->kind);
    }
    long z = get_floor_filled_subtiles_at(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    if (!delete_room_slab(coord_slab(creatng->mappos.x.val), coord_slab(creatng->mappos.y.val), 1))
    {
        ERRORLOG("Cannot delete %s room tile destroyed by %s index %d", room_code_name(room->kind), thing_model_name(creatng), (int)creatng->index);
        return 0;
    }
    create_effect(&creatng->mappos, TngEff_Explosion3, creatng->owner);
    thing_play_sample(creatng, 47, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    if (z > 0)
    {
        for (long k = 0; k < AROUND_TILES_COUNT; k++)
        {
            create_dirt_rubble_for_dug_block(creatng->mappos.x.stl.num + around[k].delta_x, creatng->mappos.y.stl.num + around[k].delta_y, z, room->owner);
        }
    }
    return 1;
}

long instf_damage_wall(struct Thing *creatng, int32_t *param)
{
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        stl_x = stl_num_decode_x(cctrl->damage_wall_coords);
        stl_y = stl_num_decode_y(cctrl->damage_wall_coords);
    }
    struct Coord3d pos = creatng->mappos;
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slb->health > 2)
    {
        create_effect(&pos, TngEff_RockChips, creatng->owner);
        slb->health -= 2;
    } else
    {
        MapSlabCoord slb_x = subtile_slab(stl_x);
        MapSlabCoord slb_y = subtile_slab(stl_y);
        place_slab_type_on_map(SlbT_EARTH, stl_x, stl_y, creatng->owner, 0);
        do_slab_efficiency_alteration(slb_x, slb_y);
        create_dirt_rubble_for_dug_slab(slb_x, slb_y);
        thing_play_sample(creatng, 73, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    thing_play_sample(creatng, 63+SOUND_RANDOM(6), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return 1;
}

long instf_eat(struct Thing *creatng, int32_t *param)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->hunger_amount > 0)
        cctrl->hunger_amount--;
    apply_health_to_thing_and_display_health(creatng, game.conf.rules[creatng->owner].health.food_health_gain);
    cctrl->hunger_level = 0;
    return 1;
}

long instf_first_person_do_imp_task(struct Thing *creatng, int32_t *param)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct PlayerInfo* player = get_player(get_appropriate_player_for_creature(creatng));
    TRACE_THING(creatng);
    struct SlabMap* slb;
    MapSubtlCoord ahead_stl_x = creatng->mappos.x.stl.num;
    MapSubtlCoord ahead_stl_y = creatng->mappos.y.stl.num;
    MapSlabCoord slb_x = subtile_slab(creatng->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab(creatng->mappos.y.stl.num);
    if (check_place_to_pretty_excluding(creatng, slb_x, slb_y))
    {
        if (cctrl->dragtng_idx == 0)
        {
            instf_pretty_path(creatng, NULL);
            return 1;
        }
    }
    MapSlabCoord ahead_slb_x = slb_x;
    MapSlabCoord ahead_slb_y = slb_y;
    if ( (creatng->move_angle_xy >= ANGLE_NORTHWEST) || (creatng->move_angle_xy < ANGLE_NORTHEAST) )
    {
        ahead_stl_y--;
        ahead_slb_y--;
    }
    else if ( (creatng->move_angle_xy >= ANGLE_SOUTHEAST) && (creatng->move_angle_xy <= ANGLE_SOUTHWEST) )
    {
        ahead_stl_y++;
        ahead_slb_y++;
    }
    else if ( (creatng->move_angle_xy >= ANGLE_SOUTHWEST) && (creatng->move_angle_xy <= ANGLE_NORTHWEST) )
    {
        ahead_stl_x--;
        ahead_slb_x--;
    }
    else if ( (creatng->move_angle_xy >= ANGLE_NORTHEAST) && (creatng->move_angle_xy <= ANGLE_SOUTHEAST) )
    {
        ahead_stl_x++;
        ahead_slb_x++;
    }
    if ( (player->selected_fp_thing_pickup != 0) || (cctrl->dragtng_idx != 0) )
    {
        set_players_packet_action(player, PckA_DirectCtrlDragDrop, 0, 0, 0, 0);
        return 1;
    }
    struct Room* room;
    TbBool subtile_diggable = subtile_is_diggable_for_player(creatng->owner, ahead_stl_x, ahead_stl_y, true);
    if (!subtile_diggable)
    {
        room = get_room_thing_is_on(creatng);
        if (!room_is_invalid(room))
        {
            if (room_role_matches(room->kind, RoRoF_GoldStorage))
            {
                if (room->owner == creatng->owner)
                {
                    TbBool slab_diggable = subtile_is_diggable_for_player(creatng->owner, slab_subtile_center(ahead_slb_x), slab_subtile_center(ahead_slb_y), true);
                    if (!slab_diggable)
                    {
                        if (creatng->creature.gold_carried > 0)
                        {
                            set_players_packet_action(player, PckA_DirectCtrlDragDrop, 0, 0, 0, 0);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    TbBool dig = true;
    slb = get_slabmap_block(slb_x, slb_y);
    if ( check_place_to_convert_excluding(creatng, slb_x, slb_y) )
    {
        if (!player->first_person_dig_claim_mode)
        {
            struct SlabConfigStats* slabst = get_slab_stats(slb);
            instf_destroy(creatng, NULL);
            if (slabst->block_flags & SlbAtFlg_IsRoom)
            {
                room = room_get(slb->room_index);
                if (!room_is_invalid(room))
                {
                    if (room->owner != creatng->owner)
                    {
                        MapCoord coord_x = subtile_coord_center(room->central_stl_x);
                        MapCoord coord_y = subtile_coord_center(room->central_stl_y);
                        event_create_event_or_update_nearby_existing_event(coord_x, coord_y,
                            EvKind_RoomUnderAttack, room->owner, 0);
                        if (is_my_player_number(room->owner))
                        {
                            output_message(SMsg_EnemyDestroyRooms, MESSAGE_DURATION_FIGHT);
                        }
                        if (game.active_messages_count > 0)
                        {
                            clear_messages_from_player(MsgType_Room, room->kind);
                        }
                        targeted_message_add(MsgType_Room, room->kind, player->id_number, 50, "%d/%d", room->health, compute_room_max_health(room->slabs_count, room->efficiency));
                    }
                    else
                    {
                        if (game.active_messages_count > 0)
                        {
                            clear_messages_from_player(MsgType_Room, room->kind);
                        }
                    }
                }
            }
            return 1;
        }
    }
    else if (player->first_person_dig_claim_mode)
    {
        if (slabmap_owner(slb) == creatng->owner)
        {
            TbBool reinforce = true;
            MapSlabCoord ahead_sslb_x = subtile_slab(ahead_stl_x);
            MapSlabCoord ahead_sslb_y = subtile_slab(ahead_stl_y);
            if (!check_place_to_reinforce(creatng, ahead_sslb_x, ahead_sslb_y))
            {
                struct ShotConfigStats* shotst = get_shot_model_stats(ShM_Dig);
                unsigned char subtiles = 0;
                do
                {
                    subtiles++;
                    if (subtiles > (shotst->health - 1))
                    {
                        reinforce = false;
                        break;
                    }
                    if ( (creatng->move_angle_xy >= ANGLE_NORTHWEST) || (creatng->move_angle_xy < ANGLE_NORTHEAST) )
                    {
                        ahead_stl_y--;
                    }
                    else if ( (creatng->move_angle_xy >= ANGLE_SOUTHEAST) && (creatng->move_angle_xy <= ANGLE_SOUTHWEST) )
                    {
                        ahead_stl_y++;
                    }
                    else if ( (creatng->move_angle_xy >= ANGLE_SOUTHWEST) && (creatng->move_angle_xy <= ANGLE_NORTHWEST) )
                    {
                        ahead_stl_x--;
                    }
                    else if ( (creatng->move_angle_xy >= ANGLE_NORTHEAST) && (creatng->move_angle_xy <= ANGLE_SOUTHEAST) )
                    {
                        ahead_stl_x++;
                    }
                    ahead_sslb_x = subtile_slab(ahead_stl_x);
                    ahead_sslb_y = subtile_slab(ahead_stl_y);
                }
                while (!check_place_to_reinforce(creatng, ahead_sslb_x, ahead_sslb_y));
            }
            if (reinforce)
            {
                cctrl->digger.working_stl = get_subtile_number(ahead_stl_x, ahead_stl_y);
                instf_reinforce(creatng, NULL);
                return 1;
            }
        }
        dig = false;
    }
    if (dig)
    {
        //TODO CONFIG shot model dependency
        int32_t locparam = ShM_Dig;
        instf_creature_fire_shot(creatng, &locparam);
    }
    return 1;
}

long instf_pretty_path(struct Thing *creatng, int32_t *param)
{
    TRACE_THING(creatng);
    SYNCDBG(16,"Starting");
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    MapSlabCoord slb_x = subtile_slab(creatng->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab(creatng->mappos.y.stl.num);
    create_effect(&creatng->mappos, imp_spangle_effects[get_player_color_idx(creatng->owner)], creatng->owner);
    thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    place_slab_type_on_map(SlbT_CLAIMED, slab_subtile_center(slb_x), slab_subtile_center(slb_y), creatng->owner, 1);
    do_unprettying(creatng->owner, slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    increase_dungeon_area(creatng->owner, 1);
    dungeon->lvstats.area_claimed++;
    return 1;
}

long instf_reinforce(struct Thing *creatng, int32_t *param)
{
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    MapSubtlCoord stl_x = stl_num_decode_x(cctrl->digger.working_stl);
    MapSubtlCoord stl_y = stl_num_decode_y(cctrl->digger.working_stl);
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    if (check_place_to_reinforce(creatng, slb_x, slb_y) <= 0) {
        return 0;
    }
    if (cctrl->digger.consecutive_reinforcements <= 25)
    {
        cctrl->digger.consecutive_reinforcements++;
        if (!S3DEmitterIsPlayingSample(creatng->snd_emitter_id, 63, 0))
        {
            struct PlayerInfo* player;
            player = get_my_player();
            int volume = 32;
            if ((player->view_type == PVT_CreatureContrl) || (player->view_type == PVT_CreaturePasngr))
            {
                volume = FULL_LOUDNESS;
            }
            thing_play_sample(creatng, 1005 + SOUND_RANDOM(7), NORMAL_PITCH, 0, 3, 0, 2, volume);
        }
        return 0;
    }
    cctrl->digger.consecutive_reinforcements = 0;
    place_and_process_pretty_wall_slab(creatng, slb_x, slb_y);
    struct Coord3d pos;
    for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        pos.x.val = subtile_coord_center(stl_x + 2 * small_around[n].delta_x);
        pos.y.val = subtile_coord_center(stl_y + 2 * small_around[n].delta_y);
        struct Map* mapblk = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
        if (map_block_revealed(mapblk, creatng->owner) && ((mapblk->flags & SlbAtFlg_Blocking) == 0))
        {
            pos.z.val = get_floor_height_at(&pos);
            create_effect(&pos, imp_spangle_effects[get_player_color_idx(creatng->owner)], creatng->owner);
        }
    }
    thing_play_sample(creatng, 41, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
    return 0;
}

long instf_tortured(struct Thing *creatng, int32_t *param)
{
    TRACE_THING(creatng);
    return 1;
}

long instf_tunnel(struct Thing *creatng, int32_t *param)
{
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    MapSubtlCoord stl_x = stl_num_decode_x(cctrl->navi.first_colliding_block);
    MapSubtlCoord stl_y = stl_num_decode_y(cctrl->navi.first_colliding_block);
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb)) {
        return 0;
    }
    thing_play_sample(creatng, 69+SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        if (slb->health > 1) {
        slb->health--;
        } else {
        dig_out_block(stl_x, stl_y, creatng->owner);
        }
    return 1;
}

/**
 * Delays the possibility to use teleport spell, to make sure creature won't leave the area too soon.
 * @param creatng The creature to be affected.
 */
void delay_teleport(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->instance_use_turn[CrInst_TELEPORT] = game.play_gameturn + 100;
}

void delay_heal_sleep(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->healing_sleep_check_turn = game.play_gameturn + 600;
}

/**
 * @brief Check if the given creature can cast the specified spell by examining basic conditions.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_source_basic
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if ((source->alloc_flags & TAlF_IsControlled) != 0)
    {
        // If this creature is under player's control (Possession).
        return false;
    }
    // We assume we usually don't want to overwrite the original instance.
    struct CreatureControl* cctrl = creature_control_get_from_thing(source);
    if (cctrl->instance_id != CrInst_NULL) {
        SYNCDBG(15, "%s(%d) already has an instance %s.", thing_model_name(source), source->index,
            creature_instance_code_name(cctrl->instance_id));
        return false;
    }

    if (!creature_instance_is_available(source, inst_idx) ||
        !creature_instance_has_reset(source, inst_idx) ||
        creature_under_spell_effect(source, CSAfF_Freeze) ||
        creature_is_fleeing_combat(source) || creature_under_spell_effect(source, CSAfF_Chicken) ||
        creature_is_being_unconscious(source) || creature_is_dying(source) ||
        thing_is_picked_up(source) || creature_is_being_dropped(source) ||
        creature_is_being_sacrificed(source) || creature_is_being_summoned(source))
    {
        return false;
    }

    return true;
}

/**
 * @brief Check if the given creature can cast the specified spell by examining general conditions.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_source_generic
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_source_basic(source, target, inst_idx, param1, param2))
    {
        return false;
    }

    if (creature_is_being_tortured(source) || creature_is_kept_in_prison(source))
    {
        return false;
    }

    return true;
}

/**
 * @brief Check if the given creature can be the target of the specified spell by examining basic conditions.
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can be, false if otherwise.
 */
TbBool validate_target_basic
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (creature_is_dying(target) || thing_is_picked_up(target) || creature_is_being_dropped(target) ||
        creature_is_being_sacrificed(target) || creature_is_being_summoned(target))
    {
        return false;
    }

    struct InstanceInfo* inst_inf = creature_instance_info_get(inst_idx);
    if ((inst_inf->instance_property_flags & InstPF_SelfBuff) == 0 && source->index == target->index)
    {
        // If this spell doesn't have SELF_BUFF flag, exclude itself.
        WARNDBG(8, "%s(%d) try to cast %s(%d) on itself but this instance has no SELF_BUFF flag",
            thing_model_name(target), target->index, creature_instance_code_name(inst_idx), inst_idx);
        return false;
    }

    if (// Creature who is leaving doesn't deserve buff from allies.
        target->continue_state == CrSt_CreatureLeaves ||
        target->active_state == CrSt_CreatureLeavingDungeon ||
        target->active_state == CrSt_CreatureScavengedDisappear ||
        target->active_state ==  CrSt_LeavesBecauseOwnerLost ||
        // Target shouldn't be fighting with the caster.
        creature_has_creature_in_combat(source, target) ||
        creature_has_creature_in_combat(target, source))
    {
        return false;
    }

    return true;
}

/**
 * @brief Check if the given creature can be the target of the specified spell by examining general conditions.
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can be, false if otherwise.
 */
TbBool validate_target_generic
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_target_even_in_prison(source, target, inst_idx, param1, param2) ||
        creature_is_being_tortured(target) || creature_is_kept_in_prison(target))
    {
        return false;
    }

    return true;
}


/**
 * @brief Check if the given creature can be the target of the specified spell by checking if it is non-idle.
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can be, false if otherwise.
 */
TbBool validate_target_non_idle(struct Thing* source, struct Thing* target, CrInstance inst_idx, int32_t param1,int32_t param2)
{
    if (!validate_target_generic(source, target, inst_idx, param1, param2))
    {
        return false;
    }
    struct InstanceInfo* inst_inf = creature_instance_info_get(inst_idx);
    struct SpellConfig *spconf = get_spell_config(inst_inf->func_params[0]);
    long state_type = get_creature_state_type(target);
    if ((state_type != CrStTyp_Idle)
    && !creature_under_spell_effect(target, spconf->spell_flags)
    && !creature_is_immune_to_spell_effect(target, spconf->spell_flags))
    {
        return true;
    }
    return false;
}

/**
 * @brief Check if the given creature can be the target of the specified spell when the creature
 * is in prison/torture room.
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can be, false if otherwise.
 */
TbBool validate_target_even_in_prison
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    // We don't check the spatial conditions, such as distacne, angle, and sight here, because
    // they should be checked in the search function.
    if (!validate_target_basic(source, target, inst_idx, param1, param2) || creature_is_being_unconscious(target))
    {
        return false;
    }

    struct InstanceInfo* inst_inf = creature_instance_info_get(inst_idx);
    struct SpellConfig *spconf = get_spell_config(inst_inf->func_params[0]);
    if (spell_config_is_invalid(spconf)
    || creature_under_spell_effect(target, spconf->spell_flags)
    || creature_is_immune_to_spell_effect(target, spconf->spell_flags))
    {
        // If this instance has wrong spell, or the target has been affected by this spell, return false.
        SYNCDBG(12, "%s(%d) is not a valid target for %s because it has been affected by the spell.",
            thing_model_name(target), target->index, creature_instance_code_name(inst_idx));
        return false;
    }

    return true;
}

/**
 * @brief Check if the given creature can cast spell in prison or torture room.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_source_even_in_prison
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_source_basic(source, target, inst_idx, param1, param2))
    {
        return false;
    }

    return true;
}

/**
 * @brief Check if the target creature can benefits from buff that provides missile protection.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_target_benefits_from_missile_defense
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_target_generic(source, target, inst_idx, param1, param2))
    {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(target);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control");
        return false;
    }
    if (!has_ranged_combat_attackers(target))
    {
        return false;
    }

    return true;
}

/**
 * @brief Check if the target creature can benefits from general defensive buffs.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_target_benefits_from_defensive
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_target_generic(source, target, inst_idx, param1, param2))
    {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(target);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control");
        return false;
    }
    // When the target is fighting creatures, return true because it needs defensive buffs.
    // Doors and Hearts do not fight back, and keepers only defend by dropping units.
    if (any_flag_is_set(cctrl->combat_flags, (CmbtF_Melee|CmbtF_Ranged|CmbtF_Waiting)))
    {
        return true; // In combat with creatures.
    }
    return false;
}

/**
 * @brief Check if the target creature can benefits from higher altitude.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_target_benefits_from_higher_altitude
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_target_generic(source, target, inst_idx, param1, param2))
    {
        return false;
    }
    long state_type = get_creature_state_type(target);
    //Flyin in water has no advantage, since creatures will not fly over guardposts anyway.
    if ((state_type != CrStTyp_Idle) || terrain_toxic_for_creature_at_position(source, coord_subtile(source->mappos.x.val), coord_subtile(source->mappos.y.val)))
    {
        return true;
    }
    return false;
}

/**
 * @brief Check if the target creature can benefits from general offensive buffs.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_target_benefits_from_offensive
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_target_generic(source, target, inst_idx, param1, param2))
    {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(target);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control");
        return false;
    }
    if (cctrl->combat_flags != 0)
    {
        return true; // In any combat.
    }
    return false;
}

/**
 * @brief Check if the target creature can benefits from using Wind.
 * Wind can disperse gas and push away enemies.
 * It has more than one merit. It deserve one dedicated function.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_target_benefits_from_wind
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    // Note that we don't need to call validate_target_generic or validate_target_basic because
    // Wind isn't SELF_BUFF. It doesn't require a target, the target parameter is just the source.
    struct CreatureControl* cctrl = creature_control_get_from_thing(target);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control");
        return false;
    }
    if (creature_under_spell_effect(target, CSAfF_PoisonCloud))
    {
        return true;
    }
    struct CreatureModelConfig* stats = creature_stats_get_from_thing(target);
    if (stats->attack_preference == AttckT_Ranged && cctrl->opponents_melee_count >= 2)
    {
        // Surrounded by 2+ enemies. This could be definitely smarter but not now.
        return true;
    }

    return false;
}


/**
 * @brief The classic condition to determine if wind is used.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_target_takes_gas_damage(struct Thing* source, struct Thing* target, CrInstance inst_idx, int32_t param1, int32_t param2)
{
    // Note that we don't need to call validate_target_generic or validate_target_basic because
    // Wind isn't SELF_BUFF. It doesn't require a target, the target parameter is just the source.
    struct CreatureControl* cctrl = creature_control_get_from_thing(target);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control");
        return false;
    }
    if (creature_under_spell_effect(target, CSAfF_PoisonCloud))
    {
        return true;
    }
    return false;
}

/**
 * @brief Check if the given creature can be the target of healing spells.
 *
 * @param source The source creature
 * @param target The target creature
 * @param inst_idx  The spell instance index
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if the creature can, false if otherwise.
 */
TbBool validate_target_benefits_from_healing
    (
    struct Thing *source,
    struct Thing *target,
    CrInstance inst_idx,
    int32_t param1,
    int32_t param2
    )
{
    if (!validate_target_basic(source, target, inst_idx, param1, param2) || creature_is_being_unconscious(target) ||
        !creature_would_benefit_from_healing(target))
    {
        return false;
    }

    if (source->index == target->index)
    {
        // Special case. The healer is always allowed to heal itself even if
        // it's being tortured or imprisoned.
        return true;
    }
    else
    {
        if (creature_is_being_tortured(target) || creature_is_kept_in_prison(target) ||
            creature_is_being_tortured(source) || creature_is_kept_in_prison(source))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief Search the suitable targets for given spell.
 *
 * @param source The source creature
 * @param inst_idx The spell instance index
 * @param targets The list of the found creatures. Caller must free this.
 * @param found_count The number of the found creatures.
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if there is no error, false if otherwise.
 */
TbBool search_target_generic
    (
    struct Thing *source,
    CrInstance inst_idx,
    ThingIndex **targets,
    uint16_t *found_count,
    int32_t param1,
    int32_t param2
    )
{
    if (!targets || !found_count)
    {
        ERRORLOG("Invalid parameters!");
        return false;
    }

    TbBool ok = true;
    // To improve performance, use a smaller number than CREATURES_COUNT.
    ThingIndex* results = (ThingIndex*)malloc(MAX_CREATURES_SEARCHED * sizeof(ThingIndex));
    memset(results, 0, MAX_CREATURES_SEARCHED * sizeof(ThingIndex));
    *found_count = 0;
    // Note that we only support buff right now, so we only search source's owner's creature.
    // For offensive debuff, we need another loop to iterate all enemies.
    struct Dungeon* dungeon = get_players_num_dungeon(source->owner);
    int creature_idx = dungeon->creatr_list_start;
    int k = 0;
    while (creature_idx != 0 && (*found_count) < MAX_CREATURES_SEARCHED)
    {
        struct Thing* candidate = thing_get(creature_idx);
        struct CreatureControl* cctrl = creature_control_get_from_thing(candidate);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Invalid creature control");
            ok = false;
            break;
        }

        creature_idx = cctrl->players_next_creature_idx;
        const struct InstanceInfo* inst_inf = creature_instance_info_get(inst_idx);
        if (inst_inf->validate_target_func > 0)
        {
            if(!creature_instances_validate_func_list[inst_inf->validate_target_func]
                (source, candidate, inst_idx, inst_inf->validate_target_func_params[0],
                inst_inf->validate_target_func_params[1]))
            {
                // This candidate is out.
                continue;
            }
        }

        if (source->index != candidate->index)
        {
            // @todo Consider checking thing_in_field_of_view() in the future, now it is buggy.
            // We assume that the source must see the target before it can cast the spell.
            int range = get_combat_distance(source, candidate);
            if (range < inst_inf->range_min || range > inst_inf->range_max ||
                !creature_can_see_combat_path(source, candidate, range))
            {
                continue;
            }
        }

        results[(*found_count)] = candidate->index;
        (*found_count)++;

        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            ok = false;
            break;
        }
    }

    *targets = results;
    return ok;
}

/**
 * @brief Search the suitable targets for CrInst_RANGED_HEAL
 *
 * @param source The source creature
 * @param inst_idx The spell instance index
 * @param targets The list of the found creatures. Caller must free this.
 * @param found_count The number of the found creatures.
 * @param param1 Optional 1st parameter.
 * @param param2 Optional 2nd parameter.
 * @return TbBool True if there is no error, false if otherwise
 */
TbBool search_target_ranged_heal
    (
    struct Thing *source,
    CrInstance inst_idx,
    ThingIndex **targets,
    uint16_t *found_count,
    int32_t param1,
    int32_t param2
    )
{
    if (!targets || !found_count)
    {
        ERRORLOG("Invalid parameters!");
        return false;
    }

    if (!search_target_generic(source, inst_idx, targets, found_count, param1, param2))
    {
        return false;
    }

    ThingIndex* results = *targets;
    struct Thing *best_choice = NULL;
    TbBool ok = true;
    int i = 0;
    for (; i < *found_count; i++)
    {
        struct Thing *candidate = thing_get(results[i]);
        if (thing_is_invalid(candidate))
        {
            ERRORLOG("Creature at index %d is invalid", i);
            continue;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(candidate);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Control of creature at index %d is invalid", i);
            continue;
        }
        // Note that we only allow one target. No group buff are allowed yet.
        // So, we only pick the weakest one creature here.
        HitPoints hp_p_best = 0;
        if (best_choice)
        {
            struct CreatureControl* cctrl_best = creature_control_get_from_thing(best_choice);
            hp_p_best = 256L * best_choice->health / cctrl_best->max_health;
        }
        HitPoints hp_p_candiate = 256L * candidate->health / cctrl->max_health;
        if (!best_choice || hp_p_candiate < hp_p_best)
        {
            best_choice = candidate;
        }
    }

    if (best_choice)
    {
        results[0] = best_choice->index;
        *found_count = 1;
    }

    return ok;
}

void script_set_creature_instance(ThingModel crmodel, short slot, int instance, short level)
{
    struct CreatureModelConfig *crconf = creature_stats_get(crmodel);

    if (!creature_stats_invalid(crconf))
    {
        CrInstance old_instance = crconf->learned_instance_id[slot - 1];
        crconf->learned_instance_id[slot - 1] = instance;
        crconf->learned_instance_level[slot - 1] = level;
        const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
        unsigned long k = 0;
        int i = slist->index;
        while (i != 0)
        {
            struct Thing* thing = thing_get(i);
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Jump to invalid thing detected");
                break;
            }
            i = thing->next_of_class;
            // Per-thing code
            if (thing->model == crmodel)
            {
                if (old_instance != CrInst_NULL)
                {
                    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
                    cctrl->instance_available[old_instance] = false;
                }
                creature_increase_available_instances(thing);
            }
            // Per-thing code ends
            k++;
            if (k > slist->count)
            {
                ERRORLOG("Infinite loop detected when sweeping things list");
                break;
            }
        }


    }
}

/******************************************************************************/
