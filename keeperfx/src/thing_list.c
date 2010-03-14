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
#include "bflib_sound.h"
#include "packets.h"
#include "lvl_script.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "config_creature.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
Thing_Class_Func class_functions[] = {
  NULL,
  update_object,
  update_shot,
  update_effect_element,
  update_dead_creature,
  update_creature,
  update_effect,
  process_effect_generator,
  update_trap,
  process_door,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

/******************************************************************************/
DLLIMPORT long _DK_update_thing(struct Thing *thing);
DLLIMPORT long _DK_get_thing_checksum(struct Thing *thing);
DLLIMPORT long _DK_update_thing_sound(struct Thing *thing);
DLLIMPORT void _DK_update_creatures_not_in_list(void);
DLLIMPORT long _DK_update_things_in_list(struct StructureList *list);
DLLIMPORT void _DK_update_things(void);
DLLIMPORT long _DK_thing_is_shootable_by_any_player_including_objects(struct Thing *thing);
DLLIMPORT long _DK_thing_is_shootable_by_any_player_except_own_including_objects(struct Thing *shooter, struct Thing *thing);
DLLIMPORT long _DK_thing_is_shootable_by_any_player_except_own_excluding_objects(struct Thing *shooter, struct Thing *thing);
DLLIMPORT long _DK_thing_is_shootable_by_any_player_excluding_objects(struct Thing *thing);
/******************************************************************************/
long creature_near_filter_not_imp(const struct Thing *thing, FilterParam val)
{
  return ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0);
}

long creature_near_filter_is_enemy_of_and_not_imp(const struct Thing *thing, FilterParam plyr_idx)
{
  if (thing->owner == plyr_idx)
    return false;
  return ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0);
}
long creature_near_filter_is_owned_by(const struct Thing *thing, FilterParam plyr_idx)
{
  struct SlabMap *slb;
  int i;
  if (thing->owner == plyr_idx)
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
    if (slabmap_owner(slb) == plyr_idx)
      return true;
  }
  return false;
}

/**
 * Makes per game turn update of all things in given StructureList.
 * @param list List of things to process.
 * @return Returns checksum computed from status of all things in list.
 */
TbBigChecksum update_things_in_list(struct StructureList *list)
{
  struct Thing *thing;
  unsigned long k;
  TbBigChecksum sum;
  int i;
  SYNCDBG(18,"Starting");
  sum = 0;
  k = 0;
  i = list->index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
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
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  SYNCDBG(19,"Finished");
  return sum;
}

/**
 * Makes per game turn update of cave in things, using proper StructureList.
 * @return Returns amount of cave in things in list.
 */
unsigned long update_cave_in_things(void)
{
  struct Thing *thing;
  unsigned long k;
  int i;
  k = 0;
  i = game.thing_lists[10].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    update_cave_in(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
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
  struct Thing *thing;
  unsigned long k;
  int i;
  SYNCDBG(18,"Starting");
  k = 0;
  i = list->index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    update_thing_sound(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return k;
}

unsigned long update_creatures_not_in_list(void)
{
  struct Thing *thing;
  unsigned long k;
  int i;
  SYNCDBG(18,"Starting");
  //_DK_update_creatures_not_in_list();
  k = 0;
  i = game.thing_lists[0].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    if (thing->index == 0)
    {
      ERRORLOG("Some THING has been deleted during the processing of another thing");
      break;
    }
    if ((thing->field_0 & 0x40) != 0)
    {
      if ((thing->field_0 & 0x10) != 0)
        update_thing_animation(thing);
      else
        update_thing(thing);
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  SYNCDBG(18,"Finished");
  return k;
}

void update_things(void)
{
  SYNCDBG(7,"Starting");
  //_DK_update_things(); return;
  TbBigChecksum sum;
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
  sum += game.action_rand_seed;
  set_player_packet_checksum(my_player_number,sum);
  SYNCDBG(9,"Finished");
}

void init_player_start(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[2].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    if ((game.objects_config[thing->model].field_6) && (thing->owner == player->id_number))
    {
      dungeon = get_players_dungeon(player);
      dungeon->dnheart_idx = thing->index;
      memcpy(&dungeon->mappos,&thing->mappos,sizeof(struct Coord3d));
      break;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void init_traps(void)
{
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[7].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    if (thing->byte_13.l == 0)
    {
      thing->byte_13.l = game.traps_config[thing->model].shots;
      thing->field_4F ^= (thing->field_4F ^ (trap_stats[thing->model].field_12 << 4)) & 0x30;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void setup_computer_player(int plr_idx)
{
  struct Thing *thing;
  int i,k;
  struct PlayerInfo *player;
  SYNCDBG(5,"Starting for player %d",plr_idx);
  player = get_player(plr_idx);
  k = 0;
  i = game.thing_lists[2].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    if ((game.objects_config[thing->model].field_6) && (thing->owner == plr_idx))
    {
      script_support_setup_player_as_computer_keeper(plr_idx, 0);
      break;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void setup_computer_players(void)
{
  struct PlayerInfo *player;
  int plr_idx;
  for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
  {
      player = get_player(plr_idx);
      if ((player->field_0 & 0x01) == 0)
        setup_computer_player(plr_idx);
  }
}

void init_all_creature_states(void)
{
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[0].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    init_creature_state(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
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
  struct Thing *thing;
  unsigned long k;
  long i;
  i = game.thing_lists[2].index;
  k = 0;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    if ((thing->model == 49) && (thing->byte_13.l == num))
    {
      return thing;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return INVALID_THING;
}

long creature_of_model_in_prison(int model)
{
  struct Thing *thing;
  long i,k,n;
  i = game.thing_lists[0].index;
  k = 0;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    // Thing list loop body
    if (thing->model == model)
    {
      n = thing->field_7;
      if (n == 14)
        n = thing->field_8;
      if ((n == 41) || (n == 40))
        return i;
    }
    // Thing list loop body ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return 0;
}

TbBool knight_in_prison(void)
{
  return (creature_of_model_in_prison(6) > 0);
}

long get_free_hero_gate_number(void)
{
  struct Thing *thing;
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
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  long i;
  int count;
  dungeon = get_players_num_dungeon(plyr_idx);
  count = 0;
  i = dungeon->creatr_list_start;
  k = 0;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    cctrl = creature_control_get_from_thing(thing);
    i = cctrl->thing_idx;
    // Per creature code
    if (thing->model == model)
      count++;
    // Per creature code ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return count;
}

long count_player_creatures_not_counting_to_total(long plyr_idx)
{
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  long i,n;
  int count;
  dungeon = get_players_num_dungeon(plyr_idx);
  count = 0;
  k = 0;
  i = dungeon->creatr_list_start;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
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
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return count;
}

/**
 * Returns filtered creature from the players creature list starting from thing_idx.
 * The creature which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return LONG_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *get_player_list_creature_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param)
{
  struct CreatureControl *cctrl;
  struct Thing *thing;
  struct Thing *retng;
  long maximizer;
  unsigned long k;
  long i,n;
  SYNCDBG(9,"Starting");
  retng = INVALID_THING;
  maximizer = 0;
  k = 0;
  i = thing_idx;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    cctrl = creature_control_get_from_thing(thing);
    i = cctrl->thing_idx;
    // Per creature code
    n = filter(thing, param, maximizer);
    if (n >= maximizer)
    {
        retng = thing;
        maximizer = n;
        if (maximizer == LONG_MAX)
            break;
    }
    // Per creature code ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return retng;
}

/**
 * Returns thing of given array index.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *thing_get(long tng_idx)
{
  if ((tng_idx > 0) && (tng_idx < THINGS_COUNT))
    return game.things_lookup[tng_idx];
  if ((tng_idx < -1) || (tng_idx >= THINGS_COUNT))
    ERRORLOG("Request of invalid thing (no %ld) intercepted",tng_idx);
  return INVALID_THING;
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

TbBool thing_exists_idx(long tng_idx)
{
  struct Thing *thing;
  thing = thing_get(tng_idx);
  if (thing_is_invalid(thing))
    return false;
  return ((thing->field_0 & 0x01) != 0);
}

TbBool thing_exists(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  return ((thing->field_0 & 0x01) != 0);
}

TbBool thing_touching_floor(const struct Thing *thing)
{
  return (thing->field_60 == thing->mappos.z.val);
}

void stop_all_things_playing_samples(void)
{
  struct Thing *thing;
  long i;
  for (i=0; i < THINGS_COUNT; i++)
  {
    thing = thing_get(i);
    if ((thing->field_0 & 0x01) != 0)
    {
      if (thing->field_66)
      {
        S3DDestroySoundEmitterAndSamples(thing->field_66);
        thing->field_66 = 0;
      }
    }
  }
}

TbBool update_thing(struct Thing *thing)
{
  Thing_Class_Func classfunc;
  struct Coord3d pos;
  SYNCDBG(18,"Starting for thing class %d",(int)thing->class_id);
  if (thing_is_invalid(thing))
      return false;
  if ((thing->field_25 & 0x40) == 0)
  {
    if ((thing->field_1 & 0x04) != 0)
    {
      thing->pos_2C.x.val += thing->pos_32.x.val;
      thing->pos_2C.y.val += thing->pos_32.y.val;
      thing->pos_2C.z.val += thing->pos_32.z.val;
      thing->pos_32.x.val = 0;
      thing->pos_32.y.val = 0;
      thing->pos_32.z.val = 0;
      set_flag_byte(&thing->field_1, 0x04, false);
    }
    thing->pos_38.x.val = thing->pos_2C.x.val;
    thing->pos_38.y.val = thing->pos_2C.y.val;
    thing->pos_38.z.val = thing->pos_2C.z.val;
    if ((thing->field_1 & 0x08) != 0)
    {
      thing->pos_38.x.val += thing->pos_26.x.val;
      thing->pos_38.y.val += thing->pos_26.y.val;
      thing->pos_38.z.val += thing->pos_26.z.val;
      thing->pos_26.x.val = 0;
      thing->pos_26.y.val = 0;
      thing->pos_26.z.val = 0;
      set_flag_byte(&thing->field_1, 0x08, false);
    }
  }
  classfunc = class_functions[thing->class_id%THING_CLASSES_COUNT];
  if (classfunc == NULL)
      return false;
  if (classfunc(thing) < 0)
      return false;
  SYNCDBG(18,"Class function end ok");
  if ((thing->field_25 & 0x40) == 0)
  {
      if (thing->mappos.z.val > thing->field_60)
      {
        if (thing->pos_2C.x.val != 0)
          thing->pos_2C.x.val = thing->pos_2C.x.val * (256 - thing->field_24) / 256;
        if (thing->pos_2C.y.val != 0)
          thing->pos_2C.y.val = thing->pos_2C.y.val * (256 - thing->field_24) / 256;
        if ((thing->field_25 & 0x20) == 0)
        {
          thing->pos_32.z.val -= thing->field_20;
          thing->field_1 |= 0x04;
        }
      } else
      {
        if (thing->pos_2C.x.val != 0)
          thing->pos_2C.x.val = thing->pos_2C.x.val * (256 - thing->field_23) / 256;
        if (thing->pos_2C.y.val != 0)
          thing->pos_2C.y.val = thing->pos_2C.y.val * (256 - thing->field_23) / 256;
        thing->mappos.z.val = thing->field_60;
        if ((thing->field_25 & 0x08) != 0)
        {
          thing->pos_2C.z.val = 0;
        }
      }
  }
  update_thing_animation(thing);
  update_thing_sound(thing);
  if ((do_lights) && (thing->field_62 != 0))
  {
      if (light_is_light_allocated(thing->field_62))
      {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val + thing->field_58;
        light_set_light_position(thing->field_62, &pos);
      } else
      {
        thing->field_62 = 0;
      }
  }
  SYNCDBG(18,"Finished");
  return true;
}

TbBigChecksum get_thing_checksum(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  return _DK_get_thing_checksum(thing);
}

short update_thing_sound(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  if (thing->field_66)
  {
    if ( S3DEmitterHasFinishedPlaying(thing->field_66) )
    {
      S3DDestroySoundEmitter(thing->field_66);
      thing->field_66 = 0;
    } else
    {
      S3DMoveSoundEmitterTo(thing->field_66,
          thing->mappos.x.val, thing->mappos.y.val, thing->mappos.z.val);
    }
  }
  return true;
}

long thing_is_shootable_by_any_player_including_objects(struct Thing *thing)
{
  return _DK_thing_is_shootable_by_any_player_including_objects(thing);
}

long thing_is_shootable_by_any_player_except_own_including_objects(struct Thing *shooter, struct Thing *thing)
{
  return _DK_thing_is_shootable_by_any_player_except_own_including_objects(shooter, thing);
}

long thing_is_shootable_by_any_player_except_own_excluding_objects(struct Thing *shooter, struct Thing *thing)
{
  return _DK_thing_is_shootable_by_any_player_except_own_excluding_objects(shooter, thing);
}

long thing_is_shootable_by_any_player_excluding_objects(struct Thing *thing)
{
  return _DK_thing_is_shootable_by_any_player_excluding_objects(thing);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
