/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_list.c
 *     Things list support.
 * @par Purpose:
 *     Create and maintain list of things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_list.h"

#include "bflib_basics.h"
#include "globals.h"
#include "packets.h"
#include "lvl_script.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
DLLIMPORT void _DK_update_creatures_not_in_list(void);
DLLIMPORT long _DK_update_things_in_list(struct StructureList *list);
DLLIMPORT void _DK_update_things(void);
/******************************************************************************/
long creature_near_filter_not_imp(struct Thing *thing, long val)
{
  return (thing->model != 23);
}

long creature_near_filter_is_enemy_of_and_not_imp(struct Thing *thing, long val)
{
  return (thing->owner != val) && (thing->model != 23);
}

long creature_near_filter_is_owned_by(struct Thing *thing, long val)
{
  struct SlabMap *slb;
  int i;
  if (thing->owner == val)
  {
    return 1;
  }
  if (thing->field_7 == 14)
    i = thing->field_8;
  else
    i = thing->field_7;
  if ((i == 41) || (i == 40) || (i == 43) || (i == 42))
  {
    slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if ((slb->field_5 & 0x07) == val);
      return true;
  }
  return false;
}

TbBigChecksum update_things_in_list(struct StructureList *list)
{
  static const char *func_name="update_things_in_list";
  struct Thing *thing;
  unsigned long k;
  TbBigChecksum sum;
  int i;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_update_things_in_list(list);
  sum = 0;
  i = list->index;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    if ((thing->field_0 & 0x40) == 0)
    {
        if ((thing->field_0 & 0x10) != 0)
          update_thing_animation(thing);
        else
          update_thing(thing);
    }
    sum += get_thing_checksum(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
#if (BFDEBUG_LEVEL > 19)
    LbSyncLog("%s: Finished\n",func_name);
#endif
  return sum;
}

/*
 * Updates cave in things, using proper StructureList.
 * Returns amount of items in the list.
 */
unsigned long update_cave_in_things(void)
{
  static const char *func_name="update_cave_in_things";
  struct Thing *thing;
  unsigned long k;
  int i;
  i = game.thing_lists[10].index;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4576,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    update_cave_in(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4577,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return k;
}

/*
 * Updates sounds of things from given StructureList.
 * Returns amount of items in the list.
 */
unsigned long update_things_sounds_in_list(struct StructureList *list)
{
  static const char *func_name="update_things_sounds_in_list";
  struct Thing *thing;
  unsigned long k;
  int i;
  i = list->index;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    update_thing_sound(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return k;
}

void update_creatures_not_in_list(void)
{
  static const char *func_name="update_creatures_not_in_list";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_update_creatures_not_in_list();
}

void update_things(void)
{
  static const char *func_name="update_things";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_update_things(); return;
  TbBigChecksum sum;
  struct PlayerInfo *player;
  optimised_lights = 0;
  total_lights = 0;
  do_lights = game.field_4614D;
  sum = 0;
  sum += update_things_in_list(&game.thing_lists[0]);
  update_creatures_not_in_list();
  sum += update_things_in_list(&game.thing_lists[7]);
  sum += update_things_in_list(&game.thing_lists[1]);
  sum += update_things_in_list(&game.thing_lists[2]);
  sum += update_things_in_list(&game.thing_lists[5]);
  sum += update_things_in_list(&game.thing_lists[3]);
  sum += update_things_in_list(&game.thing_lists[4]);
  sum += update_things_in_list(&game.thing_lists[6]);
  sum += update_things_in_list(&game.thing_lists[8]);
  update_things_sounds_in_list(&game.thing_lists[9]);
  update_cave_in_things();
  sum += compute_players_checksum();
  sum += game.field_14BB4A;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  game.packets[player->packet_num%PACKETS_COUNT].chksum = sum;
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void init_player_start(struct PlayerInfo *player)
{
  static const char *func_name="init_player_start";
  struct Dungeon *dungeon;
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[2].index;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    if ((game.objects_config[thing->model].field_6) && (thing->owner == player->field_2B))
    {
      dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
      dungeon->field_0 = thing->field_1B;
      memcpy(&dungeon->mappos,&thing->mappos,sizeof(struct Coord3d));
      break;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void init_traps(void)
{
  static const char *func_name="init_traps";
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[7].index;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    if (thing->byte_13.l == 0)
    {
      thing->byte_13.l = game.traps_config[thing->model].shots;
      thing->field_4F ^= (thing->field_4F ^ (trap_stats[thing->model].field_12 << 4)) & 0x30;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void setup_computer_players(void)
{
  static const char *func_name="setup_computer_players";
  struct PlayerInfo *player;
  struct Thing *thing;
  int plr_idx;
  int i,k;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
  {
      player=&(game.players[plr_idx]);
      if ((player->field_0 & 0x01) == 0)
      {
        k = 0;
        i = game.thing_lists[2].index;
        while (i>0)
        {
          if (i >= THINGS_COUNT)
          {
            error(func_name,4578,"Jump out of things array bounds detected");
            break;
          }
          thing = game.things_lookup[i];
          if (thing_is_invalid(thing))
            break;
          i = thing->next_of_class;
          if ((game.objects_config[thing->model].field_6) && (thing->owner == plr_idx))
          {
            script_support_setup_player_as_computer_keeper(plr_idx, 0);
            break;
          }
          k++;
          if (k > THINGS_COUNT)
          {
            error(func_name,4579,"Infinite loop detected when sweeping things list");
            break;
          }
        }
      }
  }
}

void init_all_creature_states(void)
{
  static const char *func_name="init_all_creature_states";
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[0].index;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    init_creature_state(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

/*
 * Returns hero gate thing of given gate number.
 * @return Returns hero gate object, or invalid thing pointer if not found.
 */
struct Thing *find_hero_gate_of_number(long num)
{
  static const char *func_name="find_hero_gate_of_number";
  struct Thing *thing;
  unsigned long k;
  long i;
  i = game.thing_lists[2].index;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,1953,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    if ((thing->model == 49) && (thing->byte_13.l == num))
    {
      return thing;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,7641,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return game.things_lookup[0];
}

long creature_of_model_in_prison(int model)
{
  static const char *func_name="creature_of_model_in_prison";
  struct Thing *thing;
  long i,k,n;
  i = game.thing_lists[0].index;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4576,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    i = thing->next_of_class;
    if (thing->model == model)
    {
      n = thing->field_7;
      if (n == 14)
        n = thing->field_8;
      if ((n == 41) || (n == 40))
        return i;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4577,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return 0;
}

short knight_in_prison(void)
{
  return (creature_of_model_in_prison(6) > 0);
}

long get_free_hero_gate_number(void)
{
  struct Thing *thing;
  short found;
  long n;
  for (n=1; n < 256; n++)
  {
    thing = find_hero_gate_of_number(n);
    if (thing_is_invalid(thing))
      return n;
  }
  return 0;
}

long count_player_creatures_of_model(long plyr_idx, long model)
{
  static const char *func_name="count_player_creatures_of_model";
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  long i;
  int count;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  count = 0;
  i = dungeon->field_2D;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,1953,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    cctrl = creature_control_get_from_thing(thing);
    i = cctrl->thing_idx;
    if (thing->model == model)
      count++;
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,7641,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return count;
}

long count_player_creatures_not_counting_to_total(long plyr_idx)
{
  static const char *func_name="count_player_creatures_total";
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  long i,n;
  int count;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  count = 0;
  i = dungeon->field_2D;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,1953,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if (thing_is_invalid(thing))
      break;
    cctrl = creature_control_get_from_thing(thing);
    i = cctrl->thing_idx;
    n = thing->field_7;
    if (n == 14)
      n = thing->field_8;
    if ((n == 41) || (n == 40))
      count++;
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,7641,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return count;
}

/*
 * Returns thing of given array index.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *thing_get(long tng_idx)
{
  if ((tng_idx > 0) && (tng_idx < THINGS_COUNT))
    return game.things_lookup[tng_idx];
  return game.things_lookup[0];
}

long thing_get_index(const struct Thing *thing)
{
  long tng_idx;
  tng_idx = (thing - game.things_lookup[0]);
  if ((tng_idx > 0) && (tng_idx < THINGS_COUNT))
    return tng_idx;
  return 0;
}

short thing_is_invalid(const struct Thing *thing)
{
  return (thing <= game.things_lookup[0]) || (thing == NULL);
}

short thing_exists_idx(long tng_idx)
{
  struct Thing *thing;
  thing = thing_get(tng_idx);
  if (thing_is_invalid(thing))
    return false;
  return ((thing->field_0 & 0x01) != 0);
}

short thing_exists(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  return ((thing->field_0 & 0x01) != 0);
}

int thing_to_special(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return 0;
  if ((thing->class_id != 1) || (thing->model >= OBJECT_TYPES_COUNT))
    return 0;
  return object_to_special[thing->model];
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
