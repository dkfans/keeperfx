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

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    if ((inst_idx < 0) || (inst_idx >= gameadd.crtr_conf.instances_count))
    {
        ERRORMSG("%s: Tried to get invalid instance info %d!",func_name,(int)inst_idx);
        return &magic_conf.instance_info[0];
    }
    return &magic_conf.instance_info[inst_idx];
}

TbBool creature_instance_info_invalid(const struct InstanceInfo *inst_inf)
{
    return (inst_inf < &magic_conf.instance_info[1]);
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    for (long i = 0; i < LEARNED_INSTANCES_COUNT; i++)
    {
        long k = crstat->learned_instance_id[i];
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    for (int i = 0; i < LEARNED_INSTANCES_COUNT; i++)
    {
        int k = crstat->learned_instance_id[i];
        if (k > 0)
        {
            if (crstat->learned_instance_level[i] <= cctrl->explevel+1) {
                cctrl->instance_available[k] = true;
            }
            else if ( (crstat->learned_instance_level[i] > cctrl->explevel+1) && !(gameadd.classic_bugs_flags & ClscBug_RebirthKeepsSpells) )
            {
                cctrl->instance_available[k] = false;   
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    int avail_pos = 0;
    for (int avail_num = 0; avail_num < LEARNED_INSTANCES_COUNT; avail_num++)
    {
        CrInstance inst_id = crstat->learned_instance_id[avail_num];
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    int avail_pos = 0;
    for (int avail_num = 0; avail_num < LEARNED_INSTANCES_COUNT; avail_num++)
    {
        CrInstance inst_id = crstat->learned_instance_id[avail_num];
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    int avail_pos = 0;
    for (int avail_num = 0; avail_num < LEARNED_INSTANCES_COUNT; avail_num++)
    {
        CrInstance inst_id = crstat->learned_instance_id[avail_num];
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
    struct InstanceInfo* inst_inf;
    inst_inf = creature_instance_info_get(inum);
    if (inst_inf->flags & InstPF_Disarming)
    {
        return true;
    }
    return false;
}

TbBool instance_draws_possession_swipe(CrInstance inum)
{
    struct InstanceInfo* inst_inf;
    inst_inf = creature_instance_info_get(inum);
    if (inst_inf->flags & InstPF_UsesSwipe)
    {
        return true;
    }
    return false;
}

TbBool instance_is_ranged_weapon(CrInstance inum)
{
    struct InstanceInfo* inst_inf;
    inst_inf = creature_instance_info_get(inum);
    if (inst_inf->flags & InstPF_RangedAttack)
    {
        return true;
    }
    return false;
}

TbBool instance_is_ranged_weapon_vs_objects(CrInstance inum)
{
    struct InstanceInfo* inst_inf;
    inst_inf = creature_instance_info_get(inum);
    if ((inst_inf->flags & InstPF_RangedAttack) && (inst_inf->flags & InstPF_Destructive) && !(inst_inf->flags & InstPF_Dangerous))
    {
        return true;
    }
    return false;
}

TbBool instance_is_quick_range_weapon(CrInstance inum)
{
    struct InstanceInfo* inst_inf;
    inst_inf = creature_instance_info_get(inum);
    if ((inst_inf->flags & InstPF_RangedAttack) && (inst_inf->flags & InstPF_Quick))
    {
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
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    for (long inum = 1; inum < gameadd.crtr_conf.instances_count; inum++)
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
    for (long inum = 1; inum < gameadd.crtr_conf.instances_count; inum++)
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
    for (long inum = 1; inum < gameadd.crtr_conf.instances_count; inum++)
    {
        if (cctrl->instance_available[inum])
        {
            if (instance_is_ranged_weapon_vs_objects(inum))
                return true;
        }
    }
    return false;
}

TbBool creature_has_quick_range_weapon(const struct Thing *creatng)
{
    TRACE_THING(creatng);
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    for (long inum = 1; inum < gameadd.crtr_conf.instances_count; inum++)
    {
        if (cctrl->instance_available[inum])
        {
            if (instance_is_quick_range_weapon(inum))
                return true;
        }
    }
    return false;
}

void process_creature_instance(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    SYNCDBG(19,"Starting for %s index %d instance %d",thing_model_name(thing),(int)thing->index,(int)cctrl->instance_id);
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->instance_id != CrInst_NULL)
    {
        cctrl->inst_turn++;
        if (cctrl->inst_turn == cctrl->inst_action_turns)
        {
            struct InstanceInfo* inst_inf = creature_instance_info_get(cctrl->instance_id);
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
            // Instances sometimes failed to reach this. More reliable to set instance_use_turn sooner
            // cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn; // so this code has been moved to another location
            cctrl->instance_id = CrInst_NULL;
        }
        cctrl->inst_repeat = 0;
    }
}

long instf_creature_fire_shot(struct Thing *creatng, long *param)
{
    struct Thing *target;
    int hittype;
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->targtng_idx == 0)
    {
        if ((creatng->alloc_flags & TAlF_IsControlled) == 0)
            hittype = THit_CrtrsOnlyNotOwn;
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
            hittype = THit_CrtrsNObjcts;
        else if (thing_is_destructible_trap(target) > 0)
            hittype = THit_CrtrsNObjcts;
        else if (target->class_id == TCls_Trap)
            hittype = THit_TrapsAll;
        else if (target->owner == creatng->owner)
            hittype = THit_CrtrsOnly;
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
    creature_fire_shot(creatng, target, *param, 1, hittype);
    // Start cooldown after shot is fired
    cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn;
    return 0;
}

long instf_creature_cast_spell(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long spl_idx = param[0];
    struct SpellConfig* spconf = get_spell_config(spl_idx);
    SYNCDBG(8,"The %s index %d casts %s",thing_model_name(creatng),(int)creatng->index,spell_code_name(spl_idx));
    if (spconf->cast_at_thing)
    {
        struct Thing* trthing = thing_get(cctrl->targtng_idx);
        if (!thing_is_invalid(trthing))
        {
            creature_cast_spell_at_thing(creatng, trthing, spl_idx, cctrl->explevel);
            // Start cooldown after spell is cast
            cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn;
            return 0;
        }
    }
    creature_cast_spell(creatng, spl_idx, cctrl->explevel, cctrl->targtstl_x, cctrl->targtstl_y);
    // Start cooldown after spell effect activates
    cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn;
    return 0;
}


long process_creature_self_spell_casting(struct Thing* creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (((creatng->alloc_flags & TAlF_IsControlled) != 0)
        || (cctrl->conscious_back_turns != 0)
        || ((cctrl->stateblock_flags & CCSpl_Freeze) != 0)) {
        return 0;
    }
    if (cctrl->instance_id != CrInst_NULL) {
        return 0;
    }
    if (cctrl->combat_flags != 0) {
        return 0;
    }

    long inst_idx = get_self_spell_casting(creatng);
    if (inst_idx <= 0) {
        return 0;
    }
    set_creature_instance(creatng, inst_idx, creatng->index, 0);
    return 1;
}

long instf_dig(struct Thing *creatng, long *param)
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
        thing_play_sample(creatng, shotst->dig.sndsample_idx + UNSYNC_RANDOM(shotst->dig.sndsample_range), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        create_effect(&creatng->mappos, shotst->dig.effect_model, creatng->owner);
        if (taskkind == SDDigTask_MineGold)
        {
            gold = calculate_gold_digged_out_of_slab_with_single_hit(dig_damage, creatng->owner, cctrl->explevel, slb);
            creatng->creature.gold_carried += gold;
            dungeon->lvstats.gold_mined += gold;
            EVM_CREATURE_STAT("gold_mined", creatng->owner, creatng, "gold", gold);
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
        EVM_CREATURE_STAT("gold_mined", creatng->owner, creatng, "gold", gold);
        EVM_MAP_EVENT("dig", creatng->owner, stl_x, stl_y, "gold");
        mine_out_block(stl_x, stl_y, creatng->owner);
        if (dig_has_revealed_area(stl_x, stl_y, creatng->owner))
        {
            EventIndex evidx = event_create_event_or_update_nearby_existing_event(
                subtile_coord_center(stl_x), subtile_coord_center(stl_y),
                EvKind_AreaDiscovered, creatng->owner, 0);
            if ((evidx > 0) && is_my_player_number(creatng->owner))
                output_message(SMsg_DugIntoNewArea, 0, true);
        }
    } else
    if (taskkind == SDDigTask_DigEarth)
    {
        dig_out_block(stl_x, stl_y, creatng->owner);
        EVM_MAP_EVENT("dig", creatng->owner, stl_x, stl_y, "");

        if (dig_has_revealed_area(stl_x, stl_y, creatng->owner))
        {
            EventIndex evidx = event_create_event_or_update_nearby_existing_event(
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
            thing_play_sample(creatng, 5 + UNSYNC_RANDOM(2), 200, 0, 3, 0, 2, volume);
            return 0;
        }
        clear_dig_on_room_slabs(room, creatng->owner);
        if (room->owner == game.neutral_player_num)
        {
            claim_room(room, creatng);
        } else
        {
            MapCoord ccor_x = subtile_coord_center(room->central_stl_x);
            MapCoord ccor_y = subtile_coord_center(room->central_stl_y);
            event_create_event_or_update_nearby_existing_event(ccor_x, ccor_y, EvKind_RoomLost, room->owner, room->kind);
            claim_enemy_room(room, creatng);
        }
        thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        create_effects_on_room_slabs(room, imp_spangle_effects[creatng->owner], 0, creatng->owner);
        return 0;
    }
    if (slb->health > 1)
    {
        slb->health--;
        if ((player->view_type == PVT_CreatureContrl) || (player->view_type == PVT_CreaturePasngr))
        {
            volume = FULL_LOUDNESS;
        }
        thing_play_sample(creatng, 128 + UNSYNC_RANDOM(3), 200, 0, 3, 0, 2, volume);
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
    thing_play_sample(creatng, 128 + UNSYNC_RANDOM(3), 200, 0, 3, 0, 2, volume);
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
    struct Room* room = get_room_thing_is_on(creatng);
    if (room_is_invalid(room))
    {
        ERRORLOG("The %s index %d is not on room",thing_model_name(creatng),(int)creatng->index);
        return 0;
    }
    SYNCDBG(8,"Executing for %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct SlabMap* slb = get_slabmap_thing_is_on(creatng);
    if (slb->health > 2)
    {
        //TODO CONFIG damage made to room slabs is constant - doesn't look good
        slb->health -= 2;
        thing_play_sample(creatng, 128 + UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
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
    if (!delete_room_slab(coord_slab(creatng->mappos.x.val), coord_slab(creatng->mappos.y.val), 1))
    {
        ERRORLOG("Cannot delete %s room tile destroyed by %s index %d", room_code_name(room->kind), thing_model_name(creatng), (int)creatng->index);
        return 0;
    }
    create_effect(&creatng->mappos, TngEff_Explosion3, creatng->owner);
    thing_play_sample(creatng, 47, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return 1;
}

long instf_damage_wall(struct Thing *creatng, long *param)
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
    thing_play_sample(creatng, 63+UNSYNC_RANDOM(6), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return 1;
}

long instf_eat(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->hunger_amount > 0)
        cctrl->hunger_amount--;
    apply_health_to_thing_and_display_health(creatng, game.food_health_gain);
    cctrl->hunger_level = 0;
    return 1;
}

long instf_fart(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    struct Thing* efftng = create_effect(&creatng->mappos, TngEff_Gas3, creatng->owner);
    if (!thing_is_invalid(efftng))
        efftng->shot_effect.hit_type = THit_CrtrsOnlyNotOwn;
    thing_play_sample(creatng,94+UNSYNC_RANDOM(6), NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    // Start cooldown after fart created
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->instance_use_turn[cctrl->instance_id] = game.play_gameturn;
    return 1;
}

long instf_first_person_do_imp_task(struct Thing *creatng, long *param)
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
    struct PlayerInfoAdd* playeradd = get_playeradd(player->id_number);
    if ( (playeradd->selected_fp_thing_pickup != 0) || (cctrl->dragtng_idx != 0) )
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
        if (!playeradd->first_person_dig_claim_mode)
        {
            struct SlabAttr* slbattr = get_slab_attrs(slb);
            instf_destroy(creatng, NULL);
            if (slbattr->block_flags & SlbAtFlg_IsRoom)
            {
                room = room_get(slb->room_index);
                if (!room_is_invalid(room))
                {
                    char id = ((~room->kind) + 1) - 78;
                    if (room->owner != creatng->owner)
                    {
                        MapCoord coord_x = subtile_coord_center(room->central_stl_x);
                        MapCoord coord_y = subtile_coord_center(room->central_stl_y);
                        event_create_event_or_update_nearby_existing_event(coord_x, coord_y,
                            EvKind_RoomUnderAttack, room->owner, 0);
                        if (is_my_player_number(room->owner))
                        {
                            output_message(SMsg_EnemyDestroyRooms, MESSAGE_DELAY_FIGHT, true);
                        }
                        if (game.active_messages_count > 0)
                        {
                            clear_messages_from_player(id);
                        }
                        targeted_message_add(id, player->id_number, 50, "%d/%d", room->health, compute_room_max_health(room->slabs_count, room->efficiency));
                    }
                    else
                    {
                        if (game.active_messages_count > 0)
                        {
                            clear_messages_from_player(id);
                        }
                    }
                }
            }
            return 1;
        }
    }
    else if (playeradd->first_person_dig_claim_mode)
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
        long locparam = ShM_Dig;
        instf_creature_fire_shot(creatng, &locparam);
    }
    return 1;
}

long instf_pretty_path(struct Thing *creatng, long *param)
{
    TRACE_THING(creatng);
    SYNCDBG(16,"Starting");
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    MapSlabCoord slb_x = subtile_slab(creatng->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab(creatng->mappos.y.stl.num);
    create_effect(&creatng->mappos, imp_spangle_effects[creatng->owner], creatng->owner);
    thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    place_slab_type_on_map(SlbT_CLAIMED, slab_subtile_center(slb_x), slab_subtile_center(slb_y), creatng->owner, 1);
    do_unprettying(creatng->owner, slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    increase_dungeon_area(creatng->owner, 1);
    dungeon->lvstats.area_claimed++;
    EVM_MAP_EVENT("claimed", creatng->owner, slb_x, slb_y, "");
    return 1;
}

long instf_reinforce(struct Thing *creatng, long *param)
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
            thing_play_sample(creatng, 1005 + UNSYNC_RANDOM(7), NORMAL_PITCH, 0, 3, 0, 2, volume);
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
    SYNCDBG(16,"Starting");
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    MapSubtlCoord stl_x = stl_num_decode_x(cctrl->navi.first_colliding_block);
    MapSubtlCoord stl_y = stl_num_decode_y(cctrl->navi.first_colliding_block);
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->instance_use_turn[CrInst_TELEPORT] = game.play_gameturn + 100;
}

void delay_heal_sleep(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->healing_sleep_check_turn = game.play_gameturn + 600;
}
/******************************************************************************/
