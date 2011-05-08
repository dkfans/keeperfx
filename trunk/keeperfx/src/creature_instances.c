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
 * @date     11 Mar 2010 - 12 May 2010
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

#include "bflib_math.h"
#include "thing_list.h"
#include "thing_creature.h"
#include "thing_effects.h"
#include "creature_control.h"
#include "map_blocks.h"
#include "config_magic.hpp"
#include "gui_soundmsgs.h"
#include "sounds.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_process_creature_instance(struct Thing *thing);
DLLIMPORT long _DK_instf_dig(struct Thing *thing, long *param);

DLLIMPORT struct InstanceInfo _DK_instance_info[48];
#define instance_info _DK_instance_info
/******************************************************************************/
long instf_creature_fire_shot(struct Thing *thing, long *param);
long instf_creature_cast_spell(struct Thing *thing, long *param);
long instf_dig(struct Thing *thing, long *param);

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
  NULL,//instf_attack_room_slab,
  instf_creature_cast_spell,
  instf_creature_fire_shot,
  NULL,//instf_damage_wall,
  NULL,//instf_destroy,
  instf_dig,
  NULL,//instf_eat,
  NULL,//instf_fart,
  NULL,//instf_first_person_do_imp_task,
  NULL,//instf_pretty_path, //[10]
  NULL,//instf_reinforce,
  NULL,//instf_tortured,
  NULL,//instf_tunnel,
  NULL,
  NULL,
};

/******************************************************************************/

/** Returns creature instance info structure for given instance index.
 *
 * @param inst_idx Instance index.
 * @param func_name Name of the caller function, for logging purposes.
 * @return Instance Info struct is returned.
 */
struct InstanceInfo *creature_instance_info_get_ptr(long inst_idx,const char *func_name)
{
    if ((inst_idx < 0) || (inst_idx >= sizeof(instance_info)/sizeof(instance_info[0])))
    {
        ERRORMSG("%s: Tried to get invalid instance info %ld!",func_name,inst_idx);
        return &instance_info[0];
    }
    return &instance_info[inst_idx];
}

TbBool creature_instance_info_invalid(const struct InstanceInfo *inst_inf)
{
    return (inst_inf < &instance_info[1]);
}

long creature_instance_is_available(struct Thing *thing, long inum)
{
  struct CreatureControl *cctrl;
  cctrl = creature_control_get_from_thing(thing);
  if (creature_control_invalid(cctrl))
    return 0;
  return cctrl->instances[inum];
}

void process_creature_instance(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct InstanceInfo *inst_inf;
  cctrl = creature_control_get_from_thing(thing);
  //_DK_process_creature_instance(thing);
  if (cctrl->instance_id != CrInst_NULL)
  {
    cctrl->field_D4++;
    if (cctrl->field_D6 == cctrl->field_D4)
    {
        inst_inf = creature_instance_info_get(cctrl->instance_id);
        if (inst_inf->func_cb != NULL)
        {
            inst_inf->func_cb(thing, &inst_inf->field_22);
        }
    }
    if (cctrl->field_D8 == cctrl->field_D4)
    {
      if ( cctrl->field_D3 )
      {
        cctrl->field_D4--;
        cctrl->field_D3 = 0;
        return;
      }
      cctrl->field_DE[cctrl->instance_id] = game.play_gameturn;
      cctrl->instance_id = CrInst_NULL;
    }
    cctrl->field_D3 = 0;
  }
}

long instf_creature_fire_shot(struct Thing *thing, long *param)
{
  struct CreatureControl *cctrl;
  struct Thing *target;
  int i;
  cctrl = creature_control_get_from_thing(thing);
  if (cctrl->field_DA <= 0)
  {
    if ((thing->field_0 & 0x20) == 0)
      i = 4;
    else
      i = 1;
  } else
  if ((thing->field_0 & 0x20) != 0)
  {
    target = thing_get(cctrl->field_DA);
    if (target->class_id == TCls_Object)
      i = 1;
    else
      i = 2;
  } else
  {
    target = thing_get(cctrl->field_DA);
    if (target->class_id == TCls_Object)
      i = 1;
    else
    if (target->owner == thing->owner)
      i = 2;
    else
      i = 4;
  }
  if (cctrl->field_DA > 0)
    target = thing_get(cctrl->field_DA);
  else
    target = NULL;
  creature_fire_shot(thing, target, *param, 1, i);
  return 0;
}

long instf_creature_cast_spell(struct Thing *thing, long *param)
{
  struct CreatureControl *cctrl;
  struct Thing *trthing;
  struct SpellInfo *magicinf;
  long mgc_idx;
  cctrl = creature_control_get_from_thing(thing);
  mgc_idx = *param;
  magicinf = get_magic_info(mgc_idx);
  if (magicinf->field_0)
  {
    trthing = thing_get(cctrl->field_DA);
    if (!thing_is_invalid(trthing))
    {
      creature_cast_spell_at_thing(thing, trthing, mgc_idx, 1);
      return 0;
    }
  }
  creature_cast_spell(thing, mgc_idx, 1, cctrl->target_x, cctrl->target_y);
  return 0;
}

long instf_dig(struct Thing *thing, long *param)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct SlabMap *slb;
    long stl_x,stl_y;
    long task_idx,taskkind;
    long dig_damage,gold;
    SYNCDBG(16,"Starting");
    //return _DK_instf_dig(thing, param);
    cctrl = creature_control_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    task_idx = cctrl->word_91;
    {
      struct MapTask *task;
      task = get_dungeon_task_list_entry(dungeon,task_idx);
      taskkind = task->field_0;
      if (task->field_1 != cctrl->word_8F)
        return 0;
      stl_x = stl_num_decode_x(cctrl->word_8F);
      stl_y = stl_num_decode_y(cctrl->word_8F);
    }
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_owner(slb) == thing->owner)
        dig_damage = game.default_imp_dig_own_damage;
    else
        dig_damage = game.default_imp_dig_damage;
    if (slb->field_4 > dig_damage)
    {
        if (slb->kind != SlbT_GEMS)
          slb->field_4 -= dig_damage;
        thing_play_sample(thing, 63 + UNSYNC_RANDOM(6), 100, 0, 3, 0, 2, 256);
        create_effect(&thing->mappos, 25, thing->owner);
        if (taskkind == 2)
        {
          gold = (dig_damage * (long)game.gold_per_gold_block) / game.block_health[1];
          if (slb->kind == SlbT_GEMS)
          {
            gold /= 6;
            if (gold <= 1)
              gold = 1;
          }
          thing->long_13 += gold;
          dungeon->lvstats.gold_mined += gold;
        }
        return 0;
    }
    remove_from_task_list(thing->owner, task_idx);
    if (taskkind == 2)
    {
        gold = (game.gold_per_gold_block * (long)slb->field_4) / game.block_health[1];
        thing->long_13 += gold;
        dungeon->lvstats.gold_mined += gold;
        mine_out_block(stl_x, stl_y, thing->owner);
        if (dig_has_revealed_area(stl_x, stl_y, thing->owner))
        {
            event_create_event_or_update_nearby_existing_event(
                get_subtile_center_pos(stl_x), get_subtile_center_pos(stl_y),
                13, thing->owner, 0);
            if (is_my_player_number(thing->owner))
                output_message(SMsg_DugIntoNewArea, 0, true);
        }
    } else
    if (taskkind == 1)
    {
        dig_out_block(stl_x, stl_y, thing->owner);
        if (dig_has_revealed_area(stl_x, stl_y, thing->owner))
        {
            event_create_event_or_update_nearby_existing_event(
                get_subtile_center_pos(stl_x), get_subtile_center_pos(stl_y),
                13, thing->owner, 0);
            if (is_my_player_number(thing->owner))
                output_message(SMsg_DugIntoNewArea, 0, true);
        }
    }
    check_map_explored(thing, stl_x, stl_y);
    thing_play_sample(thing, 72 + UNSYNC_RANDOM(3), 100, 0, 3, 0, 4, 256);
    return 1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
