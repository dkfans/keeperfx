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
#include "creature_control.h"
#include "config_creature.h"
#include "config_effects.h"
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

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_instf_damage_wall(struct Thing *thing, long *param);
DLLIMPORT long _DK_instf_eat(struct Thing *thing, long *param);
DLLIMPORT long _DK_instf_fart(struct Thing *thing, long *param);

DLLIMPORT struct InstanceInfo _DK_instance_info[48];
#define instance_info _DK_instance_info
/******************************************************************************/
long instf_attack_room_slab(struct Thing *creatng, long *param);
long instf_creature_cast_spell(struct Thing *creatng, long *param);
long instf_creature_fire_shot(struct Thing *creatng, long *param);
long instf_damage_wall(struct Thing *creatng, long *param);
long instf_destroy(struct Thing *creatng, long *param);
long instf_dig(struct Thing *creatng, long *param);
long instf_eat(struct Thing *creatng, long *param);
long instf_fart(struct Thing *creatng, long *param);
long instf_first_person_do_imp_task(struct Thing *creatng, long *param);
long instf_pretty_path(struct Thing *creatng, long *param);
long instf_reinforce(struct Thing *creatng, long *param);
long instf_tortured(struct Thing *creatng, long *param);
long instf_tunnel(struct Thing *creatng, long *param);

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
  instf_fart,
  instf_first_person_do_imp_task,
  instf_pretty_path, //[10]
  instf_reinforce,
  instf_tortured,
  instf_tunnel,
  NULL,
  NULL,
};

/*
struct InstanceInfo instance_info[] = {
    {0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0, NULL,                       0,  0},
    {0,  8,  4,  4,  2,   8,   4,  3,  0,  1,  3, instf_creature_fire_shot,  21,  0},
    {0,  8,  4,  4,  2,   8,   4,  3,  0,  1,  3, instf_creature_fire_shot,  22,  0},
    {0,  0,  0,  0,  0,   0,   0,  3,  0,  0,  3, NULL,                       0,  0},
    {0, 10,  5,  5,  2,  16,   8,  3,  0,  1,  3, instf_creature_fire_shot,  14,  0},
    {0, 10,  4,  6,  2,  32,   4,  3,  0,  1,  3, instf_creature_cast_spell,  1,  0},
    {0, 10,  5,  6,  3, 100,  16,  3,  0,  1,  3, instf_creature_cast_spell,  2,  0},
    {0, 10,  5,  6,  3, 100,  16,  3,  0,  1,  3, instf_creature_cast_spell,  3,  0},
    {1, 10,  5,  6,  3, 400, 200,  3,  0,  1,  3, instf_creature_cast_spell,  4,  0},
    {0, 10,  5,  6,  3,   4,   1,  3,  0,  1,  3, instf_creature_cast_spell,  5,  0},
    {1, 10,  5,  6,  3, 400, 200,  3,  0,  1,  3, instf_creature_cast_spell,  6,  0},
    {1, 10,  5,  6,  3, 400, 200,  3,  0,  1,  3, instf_creature_cast_spell,  7,  0},
    {0, 10,  5,  6,  3,  80,  20,  3,  0,  1,  4, instf_creature_cast_spell,  8,  0},
    {1, 10,  5,  6,  3, 500, 200,  3,  0,  1,  3, instf_creature_cast_spell,  9,  0},
    {1, 10,  5,  6,  3,  10, 200,  3,  0,  1,  3, instf_creature_cast_spell, 10,  0},
    {1, 10,  5,  6,  3, 300, 200,  3,  0,  1,  3, instf_creature_cast_spell, 11,  0},
    {0, 10,  5,  6,  3, 400,  20,  3,  0,  1,  3, instf_creature_cast_spell, 12,  0},
    {0, 10,  5,  6,  3,   5,   3,  3,  0,  1,  3, instf_creature_cast_spell, 13,  0},
    {0, 10,  5,  6,  3,  10,  40,  3,  0,  1,  3, instf_creature_cast_spell, 14,  0},
    {0, 10,  5,  6,  3,  25,   2,  3,  0,  1,  3, instf_creature_cast_spell, 15,  0},
    {0, 10,  5,  6,  3,  50,   2,  3,  0,  1,  3, instf_creature_cast_spell, 16,  0},
    {0, 10,  5, 10,  5,   8,   4,  3,  1,  1,  3, instf_creature_cast_spell, 17,  0},
    {0, 10,  5,  6,  3,  50,  40,  3,  0,  1,  3, instf_creature_cast_spell, 18,  0},
    {0, 10,  5,  6,  3,   8,   3,  3,  0,  1,  3, instf_creature_cast_spell, 19,  0},
    {1, 10,  5,  6,  3, 400, 200,  3,  0,  1,  3, instf_creature_cast_spell, 20,  0},
    {0, 10,  5,  6,  3,   8,   4,  3,  0,  1,  3, instf_creature_cast_spell, 21,  0},
    {0, 10,  5,  6,  3,  60,  20,  3,  0,  1,  3, instf_creature_cast_spell, 22,  0},
    {0, 10,  5,  6,  3, 100,  20,  3,  0,  1,  3, instf_creature_cast_spell, 23,  0},
    {0, 10,  5,  6,  3,   8, 200,  3,  0,  1,  3, instf_creature_cast_spell, 24,  0},
    {0,  8,  4,  4,  2, 100,  10,  3,  0,  1,  3, instf_fart,         0,  0},
    {0,  8,  4,  4,  2,   1,   1,  3,  0,  0,  3, instf_dig,          0,  0},
    {0,  8,  4,  4,  2,   1,   1,  7,  0,  0,  3, instf_pretty_path,  0,  0},
    {0,  8,  4,  4,  2,   1,   1,  7,  0,  0,  3, instf_destroy,      0,  0},
    {0,  8,  4,  4,  2,   1,   1,  3,  0,  0,  3, instf_tunnel,       0,  0},
    {0,  8,  4,  1,  1,   1,   1, 11,  0,  0,  3, NULL,               3,  0},
    {0,  8,  4,  4,  2,   1,   1,  7,  0,  0,  3, instf_reinforce,    0,  0},
    {0, 16,  8,  8,  4,   1,   1, 13,  0,  0,  3, instf_eat,          0,  0},
    {0,  8,  4,  4,  2,   1,   1,  3,  0,  1,  3, instf_attack_room_slab,     0,  0},
    {0,  8,  4,  4,  2,   1,   1,  3,  0,  1,  3, instf_damage_wall,         21,  0},
    {0,  8,  4,  4,  2,   1,   1,  3,  0,  1,  3, instf_first_person_do_imp_task,0,0},
    {0, 10,  5,  6,  3,   6,   3,  3,  0,  1,  6, instf_creature_cast_spell, 29,  0},
    {0, 10,  5,  6,  3,   6,   3,  3,  0,  1,  3, instf_creature_cast_spell, 26,  0},
    {0, 10,  5,  6,  3,   6,   3,  3,  0,  1,  3, instf_creature_cast_spell, 27,  0},
    {0, 10,  5,  6,  3,   6,   3,  3,  0,  1,  3, instf_creature_cast_spell, 28,  0},
    {0,  8,  4,  1,  1,   1,   1, 15,  0,  0,  3, NULL,               4,  0},
    {0, 16,  8,  8,  4,   1,   1, 14,  0,  0,  3, instf_tortured,     0,  0},
    {0, 16,  4,  4,  2,   1,   1,  5,  0,  0,  3, NULL,               0,  0},
    {0,  8,  4,  4,  2,   1,   1,  6,  0,  0,  3, NULL,               0,  0},
};*/

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
    if ((inst_idx < 0) || (inst_idx >= sizeof(instance_info)/sizeof(instance_info[0])))
    {
        ERRORMSG("%s: Tried to get invalid instance info %d!",func_name,(int)inst_idx);
        return &instance_info[0];
    }
    return &instance_info[inst_idx];
}

TbBool creature_instance_info_invalid(const struct InstanceInfo *inst_inf)
{
    return (inst_inf < &instance_info[1]);
}

TbBool creature_instance_is_available(const struct Thing *thing, CrInstance inst_id)
{
    struct CreatureControl *cctrl;
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    return cctrl->instance_available[inst_id];
}

TbBool creature_choose_first_available_instance(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    long i,k;
    for (i = 0; i < CREATURE_MAX_SPELL; i++)
    {
        k = crstat->instance_spell_id[i];
        if (k > 0)
        {
            if (cctrl->instance_available[k])
            {
                cctrl->active_instance = k;
                return true;
            }
        }
    }
    cctrl->active_instance = 0;
    return false;
}

void creature_increase_available_instances(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    int i,k;
    for (i = 0; i < CREATURE_MAX_SPELL; i++)
    {
        k = crstat->instance_spell_id[i];
        if (k > 0)
        {
            if (crstat->instance_required_level[i] <= cctrl->explevel+1)
            {
                cctrl->instance_available[k] = true;
            }
        }
    }
}

/**
 * Given instance ID, returns its position in compacted list of instances.
 * Compacted list of instances is a list of available creature instances without holes.
 * @param thing
 * @param req_inst_id
 * @return
 */
int creature_instance_get_available_pos_for_id(struct Thing *thing, CrInstance req_inst_id)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    int avail_pos;
    avail_pos = 0;
    int avail_num;
    for (avail_num = 0; avail_num < CREATURE_MAX_SPELL; avail_num++)
    {
        CrInstance inst_id;
        inst_id = crstat->instance_spell_id[avail_num];
        if (creature_instance_is_available(thing, inst_id))
        {
            if (inst_id == req_inst_id) {
                return avail_pos;
            }
            avail_pos++;
        }
    }
    return -1;
}

/**
 * For position in compacted list of instances, gives instance position in availability list.
 * Compacted list of instances is a list of available creature instances without holes.
 * @param thing
 * @param req_avail_pos
 * @return
 */
int creature_instance_get_available_number_for_pos(struct Thing *thing, int req_avail_pos)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    int avail_pos;
    avail_pos = 0;
    int avail_num;
    for (avail_num = 0; avail_num < CREATURE_MAX_SPELL; avail_num++)
    {
        CrInstance inst_id;
        inst_id = crstat->instance_spell_id[avail_num];
        if (creature_instance_is_available(thing, inst_id))
        {
            if (avail_pos == req_avail_pos) {
                return avail_num;
            }
            avail_pos++;
        }
    }
    return -1;
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
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    int avail_pos = 0;
    int avail_num = 0;
    for (avail_num = 0; avail_num < CREATURE_MAX_SPELL; avail_num++)
    {
        CrInstance inst_id;
        inst_id = crstat->instance_spell_id[avail_num];
        if (creature_instance_is_available(thing, inst_id))
        {
            if (avail_pos == req_avail_pos)
            {
                return inst_id;
            }
            avail_pos++;
        }
    }
    return CrInst_NULL;
}

TbBool instance_is_ranged_weapon(CrInstance inum)
{
    switch (inum)
    {
      case 4:
      case 5:
      case 6:
      case 9:
      case 12:
      case 17:
      case 19:
      case 20:
      case 27:
        return true;
    }
    return false;
}

TbBool instance_is_ranged_weapon_vs_objects(CrInstance inum)
{
    switch (inum)
    {
      case 4:
      case 5:
      case 6:
      case 9:
      case 17:
      case 19:
      case 20:
      case 27:
        return true;
    }
    return false;
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
    const struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    long inum;
    for (inum = 1; inum < CREATURE_INSTANCES_COUNT; inum++)
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
 * Informs whether the creature has an instance which is ranged weapon useable against objects.
 * The instances currently in use and currently in cooldown are included.
 * @param creatng The creature to be checked.
 * @return True if the creature has ranged weapon, false otherwise.
 */
TbBool creature_has_ranged_object_weapon(const struct Thing *creatng)
{
    TRACE_THING(creatng);
    const struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    long inum;
    for (inum = 1; inum < CREATURE_INSTANCES_COUNT; inum++)
    {
        if (cctrl->instance_available[inum])
        {
            if (instance_is_ranged_weapon_vs_objects(inum))
                return true;
        }
    }
    return false;
}

void process_creature_instance(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct InstanceInfo *inst_inf;
    SYNCDBG(19,"Starting for %s index %d instance %d",thing_model_name(thing),(int)thing->index,(int)cctrl->instance_id);
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->instance_id != CrInst_NULL)
    {
        cctrl->inst_turn++;
        if (cctrl->inst_turn == cctrl->inst_action_turns)
        {
            inst_inf = creature_instance_info_get(cctrl->instance_id);
            if (inst_inf->func_cb != NULL)
            {
                SYNCDBG(18,"Executing %s for %s index %d.",creature_instance_code_name(cctrl->instance_id),thing_model_name(thing),(int)thing->index);
                inst_inf->func_cb(thing, inst_inf->func_params);
            }
        }
        if (cctrl->inst_turn >= cctrl->inst_total_turns)
        {
            if (cctrl->inst_repeat)
            {
                cctrl->inst_turn--;
                cctrl->inst_repeat = 0;
                return;
            }
            cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn;
            cctrl->instance_id = CrInst_NULL;
        }
        cctrl->inst_repeat = 0;
    }
}

long instf_creature_fire_shot(struct Thing *creatng, long *param)
{
    struct CreatureControl *cctrl;
    struct Thing *target;
    int i;
    TRACE_THING(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->targtng_idx <= 0)
    {
        if ((creatng->alloc_flags & TAlF_IsControlled) == 0)
            i = 4;
        else
            i = 1;
    }
    else if ((creatng->alloc_flags & TAlF_IsControlled) != 0)
    {
        target = thing_get(cctrl->targtng_idx);
        TRACE_THING(target);
        if (target->class_id == TCls_Object)
            i = 1;
        else
            i = 2;
    }
    else
    {
        target = thing_get(cctrl->targtng_idx);
        TRACE_THING(target);
        if (target->class_id == TCls_Object)
            i = 1;
        else if (target->owner == creatng->owner)
            i = 2;
        else
            i = 4;
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
    creature_fire_shot(creatng, target, *param, 1, i);
    return 0;
}

long instf_creature_cast_spell(struct Thing *creatng, long *param)
{
    struct CreatureControl *cctrl;
    struct Thing *trthing;
    struct SpellInfo *spinfo;
    long spl_idx;
    TRACE_THING(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    spl_idx = *param;
    spinfo = get_magic_info(spl_idx);
    SYNCDBG(8,"The %s index %d casts %s",thing_model_name(creatng),(int)creatng->index,spell_code_name(spl_idx));
    if (spinfo->cast_at_thing)
    {
        trthing = thing_get(cctrl->targtng_idx);
        if (!thing_is_invalid(trthing))
        {
            creature_cast_spell_at_thing(creatng, trthing, spl_idx, cctrl->explevel);
            return 0;
        }
    }
    creature_cast_spell(creatng, spl_idx, cctrl->explevel, cctrl->targtstl_x, cctrl->targtstl_y);
    return 0;
}

long instf_dig(struct Thing *creatng, long *param)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct SlabMap *slb;
    long stl_x,stl_y;
    long task_idx,taskkind;
    long dig_damage,gold;
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    dungeon = get_dungeon(creatng->owner);
    task_idx = cctrl->digger.task_idx;
    {
      struct MapTask *task;
      task = get_dungeon_task_list_entry(dungeon,task_idx);
      taskkind = task->kind;
      if (task->coords != cctrl->word_8F) {
        return 0;
      }
      stl_x = stl_num_decode_x(cctrl->word_8F);
      stl_y = stl_num_decode_y(cctrl->word_8F);
    }
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb)) {
        return 0;
    }
    dig_damage = calculate_damage_did_to_slab_with_single_hit(creatng, slb);
    if ((slb->health > dig_damage) || slab_kind_is_indestructible(slb->kind))
    {
        if (!slab_kind_is_indestructible(slb->kind))
            slb->health -= dig_damage;
        thing_play_sample(creatng, 63 + UNSYNC_RANDOM(6), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        create_effect(&creatng->mappos, TngEff_Unknown25, creatng->owner);
        if (taskkind == SDDigTask_MineGold)
        {
            gold = calculate_gold_digged_out_of_slab_with_single_hit(dig_damage, creatng->owner, cctrl->explevel, slb);
            creatng->creature.gold_carried += gold;
            dungeon->lvstats.gold_mined += gold;
        }
        return 0;
    }
    // slb->health <= dig_damage - we're going to destroy the slab
    remove_from_task_list(creatng->owner, task_idx);
    if (taskkind == SDDigTask_MineGold)
    {
        gold = calculate_gold_digged_out_of_slab_with_single_hit(slb->health, creatng->owner, cctrl->explevel, slb);
        creatng->creature.gold_carried += gold;
        dungeon->lvstats.gold_mined += gold;
        mine_out_block(stl_x, stl_y, creatng->owner);
        if (dig_has_revealed_area(stl_x, stl_y, creatng->owner))
        {
            EventIndex evidx;
            evidx = event_create_event_or_update_nearby_existing_event(
                subtile_coord_center(stl_x), subtile_coord_center(stl_y),
                EvKind_AreaDiscovered, creatng->owner, 0);
            if ((evidx > 0) && is_my_player_number(creatng->owner))
                output_message(SMsg_DugIntoNewArea, 0, true);
        }
    } else
    if (taskkind == SDDigTask_DigEarth)
    {
        dig_out_block(stl_x, stl_y, creatng->owner);
        if (dig_has_revealed_area(stl_x, stl_y, creatng->owner))
        {
            EventIndex evidx;
            evidx = event_create_event_or_update_nearby_existing_event(
                subtile_coord_center(stl_x), subtile_coord_center(stl_y),
                EvKind_AreaDiscovered, creatng->owner, 0);
            if ((evidx > 0) && is_my_player_number(creatng->owner))
                output_message(SMsg_DugIntoNewArea, 0, true);
        }
    }
    check_map_explored(creatng, stl_x, stl_y);
    thing_play_sample(creatng, 72 + UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    return 1;
}

long instf_destroy(struct Thing *creatng, long *param)
{
    struct Dungeon *dungeon;
    struct Room *room;
    struct SlabMap *slb;
    MapSlabCoord slb_x,slb_y;
    long prev_owner;

    TRACE_THING(creatng);
    slb_x = subtile_slab_fast(creatng->mappos.x.stl.num);
    slb_y = subtile_slab_fast(creatng->mappos.y.stl.num);
    dungeon = get_dungeon(creatng->owner);
    slb = get_slabmap_block(slb_x, slb_y);
    room = room_get(slb->room_index);
    prev_owner = slabmap_owner(slb);

    if ( !room_is_invalid(room) && (prev_owner != creatng->owner) )
    {
        if (room->health > 1)
        {
            room->health--;
            return 0;
        }
        clear_dig_on_room_slabs(room, creatng->owner);
        if (room->owner == game.neutral_player_num)
        {
            claim_room(room, creatng);
        } else
        {
            MapCoord ccor_x,ccor_y;
            ccor_x = subtile_coord_center(room->central_stl_x);
            ccor_y = subtile_coord_center(room->central_stl_y);
            event_create_event_or_update_nearby_existing_event(ccor_x, ccor_y, EvKind_RoomLost, room->owner, 0);
            claim_enemy_room(room, creatng);
        }
        thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        create_effects_on_room_slabs(room, imp_spangle_effects[creatng->owner], 0, creatng->owner);
        return 0;
    }
    if (slb->health > 1)
    {
        slb->health--;
        return 0;
    }
    if (prev_owner != game.neutral_player_num) {
        struct Dungeon *prev_dungeon;
        prev_dungeon = get_dungeon(prev_owner);
        prev_dungeon->lvstats.territory_lost++;
    }
    decrease_dungeon_area(prev_owner, 1);
    neutralise_enemy_block(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, creatng->owner);
    remove_traps_around_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), NULL);
    switch_owned_objects_on_destoyed_slab_to_neutral(slb_x, slb_y, prev_owner);
    dungeon->lvstats.territory_destroyed++;
    return 1;
}

long instf_attack_room_slab(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    struct Room *room;
    room = get_room_thing_is_on(creatng);
    if (room_is_invalid(room))
    {
        ERRORLOG("The %s index %d is not on room",thing_model_name(creatng),(int)creatng->index);
        return 0;
    }
    SYNCDBG(8,"Executing for %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct SlabMap *slb;
    slb = get_slabmap_thing_is_on(creatng);
    if (slb->health > 2)
    {
        //TODO CONFIG damage made to room slabs is constant - doesn't look good
        slb->health -= 2;
        thing_play_sample(creatng, 128 + UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        return 1;
    }
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(room->owner);
        dungeon->rooms_destroyed++;
    }
    if (!delete_room_slab(coord_slab(creatng->mappos.x.val), coord_slab(creatng->mappos.y.val), 1))
    {
        ERRORLOG("Cannot delete %s room tile destroyed by %s index %d",room_code_name(room->kind),thing_model_name(creatng),(int)creatng->index);
        return 0;
    }
    create_effect(&creatng->mappos, TngEff_Unknown03, creatng->owner);
    thing_play_sample(creatng, 47, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return 1;
}

long instf_damage_wall(struct Thing *creatng, long *param)
{
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    //return _DK_instf_damage_wall(creatng, param);
    MapSubtlCoord stl_x, stl_y;
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(creatng);
        stl_x = stl_num_decode_x(cctrl->field_284);
        stl_y = stl_num_decode_y(cctrl->field_284);
    }
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slb->health > 2)
    {
        slb->health -= 2;
    } else
    {
        place_slab_type_on_map(2, stl_x, stl_y, creatng->owner, 0);
        do_slab_efficiency_alteration(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
    }
    thing_play_sample(creatng, 63+UNSYNC_RANDOM(6), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return 1;
}

long instf_eat(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    //return _DK_instf_eat(creatng, param);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->hunger_amount > 0)
        cctrl->hunger_amount--;
    apply_health_to_thing_and_display_health(creatng, game.food_health_gain);
    cctrl->hunger_level = 0;
    return 1;
}

long instf_fart(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    //return _DK_instf_fart(creatng, param);
    struct Thing *efftng;
    efftng = create_effect(&creatng->mappos, 13, creatng->owner);
    if (!thing_is_invalid(efftng))
        efftng->byte_16 = 4;
    thing_play_sample(creatng,94+UNSYNC_RANDOM(6), NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    return 1;
}

long instf_first_person_do_imp_task(struct Thing *creatng, long *param)
{
    MapSlabCoord slb_x,slb_y;
    long locparam;
    TRACE_THING(creatng);
    slb_x = subtile_slab_fast(creatng->mappos.x.stl.num);
    slb_y = subtile_slab_fast(creatng->mappos.y.stl.num);
    if (check_place_to_pretty_excluding(creatng, slb_x, slb_y))
    {
        instf_pretty_path(creatng, NULL);
    } else
    {
        //TODO CONFIG shot model dependency
        locparam = 23;
        instf_creature_fire_shot(creatng, &locparam);
    }
    return 1;
}

long instf_pretty_path(struct Thing *creatng, long *param)
{
    struct Dungeon *dungeon;
    TRACE_THING(creatng);
    SYNCDBG(16,"Starting");
    dungeon = get_dungeon(creatng->owner);
    MapSlabCoord slb_x,slb_y;
    slb_x = subtile_slab_fast(creatng->mappos.x.stl.num);
    slb_y = subtile_slab_fast(creatng->mappos.y.stl.num);
    create_effect(&creatng->mappos, imp_spangle_effects[creatng->owner], creatng->owner);
    thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    place_slab_type_on_map(SlbT_CLAIMED, slab_subtile_center(slb_x), slab_subtile_center(slb_y), creatng->owner, 1);
    do_unprettying(creatng->owner, slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    increase_dungeon_area(creatng->owner, 1);
    dungeon->lvstats.area_claimed++;
    remove_traps_around_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), NULL);
    return 1;
}

long instf_reinforce(struct Thing *creatng, long *param)
{
    struct CreatureControl *cctrl;
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    MapSubtlCoord stl_x,stl_y;
    MapSlabCoord slb_x,slb_y;
    stl_x = stl_num_decode_x(cctrl->digger.working_stl);
    stl_y = stl_num_decode_y(cctrl->digger.working_stl);
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    if (check_place_to_reinforce(creatng, slb_x, slb_y) <= 0) {
        return 0;
    }
    if (cctrl->digger.byte_93 <= 25)
    {
        cctrl->digger.byte_93++;
        if (!S3DEmitterIsPlayingSample(creatng->snd_emitter_id, 172, 0)) {
            thing_play_sample(creatng, 172, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
        return 0;
    }
    cctrl->digger.byte_93 = 0;
    place_and_process_pretty_wall_slab(creatng, slb_x, slb_y);
    struct Coord3d pos;
    pos.x.stl.pos = 128;
    pos.y.stl.pos = 128;
    pos.z.stl.pos = 128;
    long n;
    for (n=0; n < SMALL_AROUND_LENGTH; n++)
    {
        pos.x.stl.num = stl_x + 2 * small_around[n].delta_x;
        pos.y.stl.num = stl_y + 2 * small_around[n].delta_y;
        struct Map *mapblk;
        mapblk = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
        if (map_block_revealed(mapblk, creatng->owner) && ((mapblk->flags & SlbAtFlg_Blocking) == 0))
        {
            pos.z.val = get_floor_height_at(&pos);
            create_effect(&pos, imp_spangle_effects[creatng->owner], creatng->owner);
        }
    }
    thing_play_sample(creatng, 41, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
    return 0;
}

long instf_tortured(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    return 1;
}

long instf_tunnel(struct Thing *creatng, long *param)
{
    struct CreatureControl *cctrl;
    struct SlabMap *slb;
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    MapSubtlCoord stl_x,stl_y;
    stl_x = stl_num_decode_x(cctrl->navi.field_15);
    stl_y = stl_num_decode_y(cctrl->navi.field_15);
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb)) {
        return 0;
    }
    thing_play_sample(creatng, 69+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
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
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->instance_use_turn[CrInst_TELEPORT] = game.play_gameturn + 100;
}
/******************************************************************************/
