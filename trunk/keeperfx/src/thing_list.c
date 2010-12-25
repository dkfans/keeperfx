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
 * @date     12 Feb 2009 - 24 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_list.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "globals.h"
#include "bflib_sound.h"
#include "packets.h"
#include "lvl_script.h"
#include "light_data.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_traps.h"
#include "thing_shots.h"
#include "thing_creature.h"
#include "config_creature.h"
#include "creature_states.h"
#include "engine_camera.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
Thing_Class_Func class_functions[] = {
  NULL,//TCls_Empty
  update_object,
  update_shot,
  update_effect_element,
  update_dead_creature,
  update_creature,
  update_effect,
  process_effect_generator,
  update_trap,
  process_door,//TCls_Door
  NULL,
  NULL,
  NULL,
  NULL,//TCls_CaveIn
  NULL,
  NULL,
  NULL,
};

unsigned long thing_create_errors = 0;

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
DLLIMPORT void _DK_add_thing_to_list(struct Thing *thing, struct StructureList *list);
DLLIMPORT void _DK_remove_thing_from_list(struct Thing *thing, struct StructureList *slist);
DLLIMPORT struct Thing *_DK_get_nearest_object_at_position(long x, long y);
/******************************************************************************/
void add_thing_to_list(struct Thing *thing, struct StructureList *list)
{
  _DK_add_thing_to_list(thing, list);
}

void remove_thing_from_list(struct Thing *thing, struct StructureList *slist)
{
    _DK_remove_thing_from_list(thing, slist);
}

struct StructureList *get_list_for_thing_class(long class_id)
{
    switch (class_id)
    {
    case TCls_Object:
        return &game.thing_lists[TngList_Objects];
    case TCls_Shot:
        return &game.thing_lists[TngList_Shots];
    case TCls_EffectElem:
        return &game.thing_lists[TngList_EffectElems];
    case TCls_DeadCreature:
        return &game.thing_lists[TngList_DeadCreatrs];
    case TCls_Creature:
        return &game.thing_lists[TngList_Creatures];
    case TCls_Effect:
        return &game.thing_lists[TngList_Effects];
    case TCls_EffectGen:
        return &game.thing_lists[TngList_EffectGens];
    case TCls_Trap:
        return &game.thing_lists[TngList_Traps];
    case TCls_Door:
        return &game.thing_lists[TngList_Doors];
    case TCls_AmbientSnd:
        return &game.thing_lists[TngList_AmbientSnds];
    case TCls_CaveIn:
        return &game.thing_lists[TngList_CaveIns];
    default:
        return NULL;
    }
}

/** Removes the given thing from a linked list which contains all things of the same class.
 *
 * @param thing The thing to be unlinked from list chain.
 */
void remove_thing_from_its_class_list(struct Thing *thing)
{
    struct StructureList *slist;
    slist = get_list_for_thing_class(thing->class_id);
    if (slist != NULL)
        remove_thing_from_list(thing, slist);
}

/** Adds the given thing to a linked list which contains all things of the same class.
 *
 * @param thing The thing to be linked with list chain.
 */
void add_thing_to_its_class_list(struct Thing *thing)
{
    struct StructureList *slist;
    slist = get_list_for_thing_class(thing->class_id);
    if (slist != NULL)
        add_thing_to_list(thing, slist);
}

long creature_near_filter_not_imp(const struct Thing *thing, FilterParam val)
{
  return ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0);
}

long near_map_block_thing_filter_not_specdigger(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    long dist_x,dist_y;
    if (thing->class_id == TCls_Creature)
    {
      if ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0)
      {
          // note that abs() is not required because we're computing square of the values
          dist_x = param->num1-(MapCoord)thing->mappos.x.val;
          dist_y = param->num2-(MapCoord)thing->mappos.y.val;
          // This function should return max value when the distance is minimal, so:
          return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long creature_near_filter_is_enemy_of_and_not_imp(const struct Thing *thing, FilterParam plyr_idx)
{
  if (thing->owner == plyr_idx)
    return false;
  return ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0);
}

long near_map_block_thing_filter_is_enemy_of_and_not_specdigger(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    long dist_x,dist_y;
    if ((thing->class_id == TCls_Creature) && (thing->owner != param->plyr_idx))
    {
      if ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0)
      {
          // note that abs() is not required because we're computing square of the values
          dist_x = param->num1-(MapCoord)thing->mappos.x.val;
          dist_y = param->num2-(MapCoord)thing->mappos.y.val;
          // This function should return max value when the distance is minimal, so:
          return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long creature_near_filter_is_owned_by(const struct Thing *thing, FilterParam plyr_idx)
{
  struct SlabMap *slb;
  if (thing->owner == plyr_idx)
  {
    return true;
  }
  if (creature_is_kept_in_prison(thing) || creature_is_being_tortured(thing))
  {
      slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
      if (slabmap_owner(slb) == plyr_idx)
          return true;
  }
  return false;
}

long near_map_block_thing_filter_is_owned_by(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    long dist_x,dist_y;
    if (thing->class_id == param->class_id)
    {
        switch(param->class_id)
        {
        case TCls_Creature:
            if (creature_near_filter_is_owned_by(thing, param->plyr_idx))
            {
                // note that abs() is not required because we're computing square of the values
                dist_x = param->num1-(MapCoord)thing->mappos.x.val;
                dist_y = param->num2-(MapCoord)thing->mappos.y.val;
                // This function should return max value when the distance is minimal, so:
                return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
            }
            break;
        default:
            if ((thing->owner == param->plyr_idx) || (param->plyr_idx == -1))
            {
                dist_x = param->num1-(MapCoord)thing->mappos.x.val;
                dist_y = param->num2-(MapCoord)thing->mappos.y.val;
                // This function should return max value when the distance is minimal, so:
                return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
            }
            break;
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long map_block_thing_filter_is_of_class_and_model_and_owned_by(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    if (thing->class_id == param->class_id)
    {
      if ((thing->model == param->model_id) || (param->model_id == -1))
      {
          if ((thing->owner == param->plyr_idx) || (param->plyr_idx == -1))
          {
              // Return the largest value to stop sweeping
              return LONG_MAX;
          }
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long map_block_creature_filter_of_model_training_and_owned_by(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    if (thing->class_id == TCls_Creature)
    {
      if ((thing->model == param->model_id) || (param->model_id == -1))
      {
          if ((thing->owner == param->plyr_idx) || (param->plyr_idx == -1))
          {
              cctrl = creature_control_get_from_thing(thing);
              if ((thing->active_state == CrSt_Training) && (cctrl->byte_9A > 1))
              {
                  // Return the largest value to stop sweeping
                  return LONG_MAX;
              }
          }
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
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

/**
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
      if (!player_exists(player))
      {
        setup_computer_player(plr_idx);
      }
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

/**
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
    // Per-thing code
    if ((thing->model == 49) && (thing->byte_13 == num))
    {
      return thing;
    }
    // Per-thing code ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return INVALID_THING;
}

struct Thing *find_nearest_enemy_creature(struct Thing *crtng)
{
  struct Thing *thing;
  unsigned long k;
  long i;
  struct Thing *neartng;
  long neardist,dist;
  neardist = LONG_MAX;
  neartng = NULL;
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
    // Per-thing code
    if (players_are_enemies(crtng->owner, thing->owner))
    {
      if (creature_will_attack_creature(crtng, thing))
      {
          dist = get_2d_box_distance(&crtng->mappos, &thing->mappos);
          if (dist < neardist)
          {
              neartng = thing;
              neardist = dist;
          }
      }
    }
    // Per-thing code ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return neartng;
}

long creature_of_model_in_prison(int model)
{
  struct Thing *thing;
  long i,k;
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
      if (creature_is_kept_in_prison(thing))
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

long electricity_affecting_area(struct Coord3d *pos, long immune_plyr_idx, long range, long max_damage)
{
  struct Thing *thing;
  struct CreatureControl *cctrl;
  unsigned long k;
  long i;
  long dist,damage,naffected;
  naffected = 0;
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
    // Per-thing code
    cctrl = creature_control_get_from_thing(thing);
    if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
    {
        if (thing->owner != immune_plyr_idx)
        {
          if ((cctrl->spell_flags & 0x04) == 0)
          {
              dist = get_2d_box_distance(&thing->mappos, pos);
              damage = get_radially_decaying_value(max_damage, range/2, range/2, dist);
              apply_damage_to_thing_and_display_health(thing, damage, immune_plyr_idx);
              naffected++;
          }
        }
    }
    // Per-thing code ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return naffected;
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

/** Counts creatures of given model belonging to given player.
 * @param plyr_idx Target player.
 * @param model Creature model, or -1 for all (except special diggers).
 *
 * @return Count of players creatures.
 */
long count_player_creatures_of_model(long plyr_idx, long model)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(plyr_idx);
  return count_player_list_creatures_of_model(dungeon->creatr_list_start, model);
}

long count_player_list_creatures_of_model(long thing_idx, long model)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i;
    int count;
    count = 0;
    i = thing_idx;
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
      if ((thing->model == model) || (model == -1))
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

struct Thing *get_player_list_nth_creature_of_model(long thing_idx, long model, long crtr_idx)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i;
    i = thing_idx;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      if (thing_is_invalid(thing))
      {
        ERRORLOG("Jump to invalid thing detected");
        return INVALID_THING;
      }
      cctrl = creature_control_get_from_thing(thing);
      i = cctrl->thing_idx;
      // Per creature code
      if (crtr_idx <= 0)
          return thing;
      if ((thing->model == model) || (model == -1))
          crtr_idx--;
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        return INVALID_THING;
      }
    }
    ERRORLOG("Tried to get creature of index exceeding list");
    return INVALID_THING;
}

long count_player_creatures_not_counting_to_total(long plyr_idx)
{
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  long i;
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
    if (creature_is_kept_in_prison(thing))
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

struct Thing *get_random_players_creature_of_model(long plyr_idx, long model)
{
    struct Dungeon *dungeon;
    long total_count,crtr_idx;
    dungeon = get_players_num_dungeon(plyr_idx);
    total_count = count_player_list_creatures_of_model(dungeon->creatr_list_start, model);
    if (total_count < 1)
        return INVALID_THING;
    crtr_idx = ACTION_RANDOM(total_count);
    return get_player_list_nth_creature_of_model(dungeon->creatr_list_start, model, crtr_idx);
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
 * Returns filtered creature from the players creature list starting at random index from thing_idx.
 * The creature which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return LONG_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * Unlike get_player_list_creature_with_filter(), this function doesn't start checking at thing_idx,
 * but at random index in the list starting at thing_idx. When list end is reached, the function
 * starts checking things of index lower than randomly selected starting index, so all things in list
 * are checked.
 * @return Returns thing, or invalid thing pointer if not found.
 * @see get_player_list_creature_with_filter()
 */
struct Thing *get_player_list_random_creature_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param)
{
  struct CreatureControl *cctrl;
  struct Thing *thing;
  struct Thing *retng;
  long maximizer;
  long total_count;
  unsigned long k;
  long i,n;
  SYNCDBG(9,"Starting");
  // Count all creatures in list, so that we can know range for our random index
  total_count = count_player_list_creatures_of_model(thing_idx, -1);
  retng = INVALID_THING;
  maximizer = 0;
  if (total_count < 1)
      return retng;
  k = 0;
  // Get random index of a thing in list
  thing = get_player_list_nth_creature_of_model(thing_idx, -1, ACTION_RANDOM(total_count));
  i = thing->index;
  while (k < total_count)
  {
    if (i == 0)
        i = thing_idx;
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
 * Returns filtered creature from the map block list, starting from thing_idx.
 * The thing which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return LONG_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *get_thing_on_map_block_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param, long *maximizer)
{
    struct Thing *thing;
    struct Thing *retng;
    unsigned long k;
    long i,n;
    SYNCDBG(19,"Starting");
    retng = INVALID_THING;
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
      i = thing->field_2;
      // Begin per-loop code
      n = filter(thing, param, *maximizer);
      if (n >= *maximizer)
      {
          retng = thing;
          *maximizer = n;
          if (*maximizer == LONG_MAX)
              break;
      }
      // End of per-loop code
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
 * Returns filtered creature from slabs around given coordinates.
 * Skips slabs which are not revealed to player provided in MaxFilterParam.
 * The thing which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return LONG_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *get_thing_near_revealed_map_block_with_filter(MapCoord x, MapCoord y, Thing_Maximizer_Filter filter, MaxFilterParam param)
{
    struct Thing *thing;
    struct Thing *retng;
    long maximizer;
    struct Map *mapblk;
    MapSubtlCoord sx,sy;
    int around;
    long i,n;
    SYNCDBG(19,"Starting");
    retng = INVALID_THING;
    maximizer = 0;
    for (around=0; around < sizeof(mid_around)/sizeof(mid_around[0]); around++)
    {
      sx = coord_subtile(x) + (MapSubtlCoord)mid_around[around].delta_x;
      sy = coord_subtile(y) + (MapSubtlCoord)mid_around[around].delta_y;
      mapblk = get_map_block_at(sx, sy);
      if (!map_block_invalid(mapblk))
      {
        if (map_block_revealed(mapblk, param->plyr_idx))
        {
            i = get_mapwho_thing_index(mapblk);
            n = maximizer;
            thing = get_thing_on_map_block_with_filter(i, filter, param, &n);
            if (!thing_is_invalid(thing) && (n >= maximizer))
            {
                retng = thing;
                maximizer = n;
                if (maximizer == LONG_MAX)
                    break;
            }
        }
      }
    }
    return retng;
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
  SYNCDBG(18,"Thing index %d, class %d",(int)thing->index,(int)thing->class_id);
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
  if (thing->class_id < sizeof(class_functions)/sizeof(class_functions[0]))
      classfunc = class_functions[thing->class_id];
  else
      classfunc = NULL;
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
          thing->pos_2C.x.val = thing->pos_2C.x.val * (long)(256 - thing->field_24) / 256;
        if (thing->pos_2C.y.val != 0)
          thing->pos_2C.y.val = thing->pos_2C.y.val * (long)(256 - thing->field_24) / 256;
        if ((thing->field_25 & 0x20) == 0)
        {
          thing->pos_32.z.val -= thing->field_20;
          thing->field_1 |= 0x04;
        }
      } else
      {
        if (thing->pos_2C.x.val != 0)
          thing->pos_2C.x.val = thing->pos_2C.x.val * (long)(256 - thing->field_23) / 256;
        if (thing->pos_2C.y.val != 0)
          thing->pos_2C.y.val = thing->pos_2C.y.val * (long)(256 - thing->field_23) / 256;
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

TbBool imp_already_digging_at_excluding(struct Thing *excltng, long stl_x, long stl_y)
{
  const struct Map *mapblk;
  struct Thing *thing;
  unsigned long k;
  long i;
  mapblk = get_map_block_at(stl_x, stl_y);
  if (map_block_invalid(mapblk))
      return false;
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      WARNLOG("Jump out of things array");
      break;
    }
    i = thing->field_2;
    // Per thing processing block
    if ((thing->class_id == TCls_Creature) && (thing != excltng))
    {
        if ( ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0) )
        {
            if ((thing->active_state == CrSt_ImpDigsMines1) || (thing->active_state == CrSt_ImpDigsMines2))
            {
                return true;
            }
        }
    }
    // Per thing processing block ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return false;
}

struct Thing *smallest_gold_pile_at_xy(long stl_x, long stl_y)
{
  const struct Map *mapblk;
  struct Thing *thing;
  unsigned long k;
  struct Thing *chosen_thing;
  long chosen_gold;
  long i;
  chosen_thing = INVALID_THING;
  chosen_gold = LONG_MAX;
  mapblk = get_map_block_at(stl_x, stl_y);
  if (map_block_invalid(mapblk))
      return chosen_thing;
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      WARNLOG("Jump out of things array");
      break;
    }
    i = thing->field_2;
    // Per thing processing block
    if ((thing->class_id == TCls_Object) && (thing->model == 43))
    {
        if (thing->long_13 < chosen_gold)
        {
            chosen_thing = thing;
            chosen_gold = thing->long_13;
        }
    }
    // Per thing processing block ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return chosen_thing;
}

TbBool gold_pile_with_maximum_at_xy(long stl_x, long stl_y)
{
  const struct Map *mapblk;
  struct Thing *thing;
  unsigned long k;
  long i;
  mapblk = get_map_block_at(stl_x, stl_y);
  if (map_block_invalid(mapblk))
      return false;
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      WARNLOG("Jump out of things array");
      break;
    }
    i = thing->field_2;
    // Per thing processing block
    if ((thing->class_id == TCls_Object) && (thing->model == 43))
    {
        if (thing->long_13 >= game.gold_pile_maximum)
        {
            return true;
        }
    }
    // Per thing processing block ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return false;
}

/** Finds creature on revealed subtiles around given position, who is not special digger.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose revealed subtiles around will be searched.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near_but_not_specdigger(MapCoord pos_x, MapCoord pos_y, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    SYNCDBG(19,"Starting");
    //return get_creature_near_with_filter(x, y, creature_near_filter_not_imp, plyr_idx);
    filter = near_map_block_thing_filter_not_specdigger;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds creature on revealed subtiles around given position, who is not special digger and is enemy to given player.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose revealed subtiles around will be searched.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near_who_is_enemy_of_and_not_specdigger(MapCoord pos_x, MapCoord pos_y, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    SYNCDBG(19,"Starting");
    //return get_creature_near_with_filter(x, y, creature_near_filter_is_enemy_of_and_not_imp, plyr_idx);
    filter = near_map_block_thing_filter_is_enemy_of_and_not_specdigger;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds creature on revealed subtiles around given position, who belongs to given player.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose creature from revealed position will be returned.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near_and_owned_by(MapCoord pos_x, MapCoord pos_y, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    SYNCDBG(19,"Starting");
    //return get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
    filter = near_map_block_thing_filter_is_owned_by;
    param.class_id = TCls_Creature;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

struct Thing *get_object_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    const struct Map *mapblk;
    long i,n;
    SYNCDBG(19,"Starting");
    filter = map_block_thing_filter_is_of_class_and_model_and_owned_by;
    param.class_id = TCls_Object;
    param.model_id = model;
    param.plyr_idx = plyr_idx;
    mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    i = get_mapwho_thing_index(mapblk);
    n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_door_for_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    const struct Map *mapblk;
    MapSlabCoord slb_x,slb_y;
    long i,n;
    SYNCDBG(19,"Starting");
    //return _DK_get_door_for_position(pos_x, pos_y);
    filter = map_block_thing_filter_is_of_class_and_model_and_owned_by;
    param.class_id = TCls_Door;
    param.model_id = -1;
    param.plyr_idx = -1;
    slb_x = (stl_x/3);
    slb_y = (stl_y/3);
    mapblk = get_map_block_at(3*slb_x+1, 3*slb_y+1);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    i = get_mapwho_thing_index(mapblk);
    n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_creature_of_model_training_at_subtile_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model_id, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    const struct Map *mapblk;
    long i,n;
    SYNCDBG(19,"Starting");
    filter = map_block_creature_filter_of_model_training_and_owned_by;
    param.class_id = TCls_Creature;
    param.model_id = model_id;
    param.plyr_idx = plyr_idx;
    mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    i = get_mapwho_thing_index(mapblk);
    n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_nearest_object_at_position(MapSubtlCoord x, MapSubtlCoord y)
{
  return _DK_get_nearest_object_at_position(x, y);
}

TbBool thing_slappable(const struct Thing *thing, long plyr_idx)
{
  switch (thing->class_id)
  {
  case TCls_Object:
      return object_is_slappable(thing, plyr_idx);
  case TCls_Shot:
      return shot_is_slappable(thing, plyr_idx);
  case TCls_Creature:
      return creature_is_slappable(thing, plyr_idx);
  case TCls_Trap:
      return trap_is_slappable(thing, plyr_idx);
  default:
      return false;
  }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
