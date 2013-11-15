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
#include "thing_corpses.h"
#include "thing_stats.h"
#include "thing_creature.h"
#include "creature_senses.h"
#include "power_hand.h"
#include "map_utils.h"
#include "config_objects.h"
#include "config_creature.h"
#include "creature_states.h"
#include "player_instances.h"
#include "engine_camera.h"
#include "gui_topmsg.h"
#include "game_legacy.h"
#include "engine_redraw.h"
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
DLLIMPORT long _DK_thing_is_shootable_by_any_player_including_objects(const struct Thing *thing);
DLLIMPORT long _DK_thing_is_shootable_by_any_player_except_own_including_objects(struct Thing *shooter, struct Thing *thing);
DLLIMPORT long _DK_thing_is_shootable_by_any_player_except_own_excluding_objects(struct Thing *shooter, struct Thing *thing);
DLLIMPORT long _DK_thing_is_shootable_by_any_player_excluding_objects(struct Thing *thing);
DLLIMPORT void _DK_add_thing_to_list(struct Thing *thing, struct StructureList *list);
DLLIMPORT void _DK_remove_thing_from_list(struct Thing *thing, struct StructureList *slist);
DLLIMPORT struct Thing *_DK_get_nearest_object_at_position(long stl_x, long stl_y);
DLLIMPORT struct Thing *_DK_find_base_thing_on_mapwho(unsigned char oclass, unsigned short model, unsigned short x, unsigned short y);
DLLIMPORT void _DK_remove_thing_from_mapwho(struct Thing *thing);
DLLIMPORT void _DK_place_thing_in_mapwho(struct Thing *thing);
/******************************************************************************/
/**
 * Adds thing at beginning of a StructureList.
 * @param thing
 * @param list
 */
void add_thing_to_list(struct Thing *thing, struct StructureList *list)
{
    //_DK_add_thing_to_list(thing, list);
    if ((thing->alloc_flags & TAlF_IsInStrucList) != 0)
    {
        ERRORLOG("Thing is already in list");
        return;
    }
    struct Thing *prevtng;
    prevtng = INVALID_THING;
    if (list->index > 0) {
        prevtng = thing_get(list->index);
    }
    list->count++;
    thing->alloc_flags |= TAlF_IsInStrucList;
    thing->prev_of_class = 0;
    thing->next_of_class = list->index;
    if (!thing_is_invalid(prevtng)) {
        prevtng->prev_of_class = thing->index;
    }
    list->index = thing->index;
}

void remove_thing_from_list(struct Thing *thing, struct StructureList *slist)
{
    struct Thing *sibtng;
    //_DK_remove_thing_from_list(thing, slist);
    if ((thing->alloc_flags & TAlF_IsInStrucList) == 0)
        return;
    if (thing->index == slist->index)
    {
        slist->index = thing->next_of_class;
        if (thing->next_of_class > 0)
        {
            sibtng = thing_get(thing->next_of_class);
            if (!thing_is_invalid(sibtng))
            {
                sibtng->prev_of_class = 0;
            }
        }
        thing->next_of_class = 0;
        thing->prev_of_class = 0;
    } else
    {
        if (thing->prev_of_class > 0)
        {
            sibtng = thing_get(thing->prev_of_class);
            if (!thing_is_invalid(sibtng))
            {
                sibtng->next_of_class = thing->next_of_class;
            }
        }
        if (thing->next_of_class > 0)
        {
            sibtng = thing_get(thing->next_of_class);
            if (!thing_is_invalid(sibtng))
            {
                sibtng->prev_of_class = thing->prev_of_class;
            }
        }
        thing->prev_of_class = 0;
        thing->next_of_class = 0;
    }
    thing->alloc_flags &= ~TAlF_IsInStrucList;
    if (slist->count <= 0) {
        ERRORLOG("List has < 0 structures");
        return;
    }
    slist->count--;
}

struct StructureList *get_list_for_thing_class(ThingClass class_id)
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
    if (slist != NULL) {
        remove_thing_from_list(thing, slist);
    }
}

ThingIndex get_thing_class_list_head(ThingClass class_id)
{
    struct StructureList *slist;
    slist = get_list_for_thing_class(class_id);
    if (slist != NULL) {
        return slist->index;
    }
    return 0;
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

long near_map_block_thing_filter_not_specdigger(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
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

long near_map_block_thing_filter_call_bool_filter(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    long dist_x,dist_y;
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if ((param->model_id == -1) || (thing->model == param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                Thing_Bool_Filter matcher_cb = (Thing_Bool_Filter)param->ptr3;
                if ((matcher_cb != NULL) && matcher_cb(thing))
                {
                    // note that abs() is not required because we're computing square of the values
                    dist_x = param->num1-(MapCoord)thing->mappos.x.val;
                    dist_y = param->num2-(MapCoord)thing->mappos.y.val;
                    // This function should return max value when the distance is minimal, so:
                    return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long creature_near_filter_is_enemy_of_and_not_specdigger(const struct Thing *thing, FilterParam plyr_idx)
{
  if (thing->owner == plyr_idx)
    return false;
  return ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0);
}

long near_map_block_thing_filter_is_enemy_of_and_not_specdigger(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
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

long near_map_block_thing_filter_is_slappable(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    long dist_x,dist_y;
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (!thing_is_picked_up(thing) && thing_slappable(thing, param->plyr_idx))
        {
            dist_x = param->num1-(MapCoord)thing->mappos.x.val;
            dist_y = param->num2-(MapCoord)thing->mappos.y.val;
            // This function should return max value when the distance is minimal, so:
            return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long near_map_block_thing_filter_is_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
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

long map_block_thing_filter_is_of_class_and_model_and_owned_by_or_allied_with(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == param->class_id)
    {
        if ((param->model_id == -1) || (thing->model == param->model_id))
        {
            if ((param->plyr_idx == -1) || players_are_mutual_allies(thing->owner,param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return LONG_MAX;
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long map_block_thing_filter_is_of_class_and_model_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == param->class_id)
    {
        if ((param->model_id == -1) || (thing->model == param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return LONG_MAX;
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long map_block_thing_filter_is_food_available_to_eat_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == TCls_Object)
    {
        if (thing->model == 10)
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return LONG_MAX;
            }
        }
    }
    if (thing->class_id == TCls_Creature)
    {
        if (creature_affected_by_spell(thing,SplK_Chicken) && (thing->health > 0))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return LONG_MAX;
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long map_block_creature_filter_of_model_training_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    if (thing->class_id == TCls_Creature)
    {
      if ((thing->model == param->model_id) || (param->model_id == -1))
      {
          if ((thing->owner == param->plyr_idx) || (param->plyr_idx == -1))
          {
              if (((int)thing->index != param->num1) || (param->num1 == -1))
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
      // Per-thing code
      if ((thing->alloc_flags & TAlF_IsInGroup) == 0)
      {
          if ((thing->alloc_flags & TAlF_IsInLimbo) != 0) {
              update_thing_animation(thing);
          } else {
              update_thing(thing);
          }
      }
      sum += get_thing_checksum(thing);
      // Per-thing code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    SYNCDBG(19,"Finished, %d items, checksum %06lX",(int)k,(unsigned long)sum);
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
  i = game.thing_lists[TngList_CaveIns].index;
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
    update_cave_in(thing);
    // Per-thing code ends
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
        // Per-thing code
        update_thing_sound(thing);
        // Per-thing code ends
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
  i = game.thing_lists[TngList_Creatures].index;
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
    if (thing->index == 0)
    {
      ERRORLOG("Some THING has been deleted during the processing of another thing");
      break;
    }
    if ((thing->alloc_flags & TAlF_IsInGroup) != 0)
    {
      if ((thing->alloc_flags & TAlF_IsInLimbo) != 0) {
        update_thing_animation(thing);
      } else {
        update_thing(thing);
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
    do_lights = game.lish.field_4614D;
    sum = 0;
    sum += update_things_in_list(&game.thing_lists[TngList_Creatures]);
    update_creatures_not_in_list();
    sum += update_things_in_list(&game.thing_lists[TngList_Traps]);
    sum += update_things_in_list(&game.thing_lists[TngList_Shots]);
    sum += update_things_in_list(&game.thing_lists[TngList_Objects]);
    sum += update_things_in_list(&game.thing_lists[TngList_Effects]);
    sum += update_things_in_list(&game.thing_lists[TngList_EffectElems]);
    sum += update_things_in_list(&game.thing_lists[TngList_DeadCreatrs]);
    sum += update_things_in_list(&game.thing_lists[TngList_EffectGens]);
    sum += update_things_in_list(&game.thing_lists[TngList_Doors]);
    update_things_sounds_in_list(&game.thing_lists[TngList_AmbientSnds]);
    update_cave_in_things();
    sum += compute_players_checksum();
    sum += game.action_rand_seed;
    set_player_packet_checksum(my_player_number,sum);
    SYNCDBG(9,"Finished");
}

struct Thing *find_players_dungeon_heart(PlayerNumber plyridx)
{
    struct Thing *thing;
    int i,k;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
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
        if (thing_is_dungeon_heart(thing) && (thing->owner == plyridx))
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
    SYNCDBG(6,"No heart for player %d",(int)plyridx);
    return INVALID_THING;
}

/**
 * Initializes start position of the player.
 * Finds players dungeon heart and initializes players start position.
 * @param player The player to be initialized.
 * @note Replaces init_dungeon_owner().
 */
void init_player_start(struct PlayerInfo *player, TbBool keep_prev)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    thing = find_players_dungeon_heart(player->id_number);
    dungeon = get_players_dungeon(player);
    if (dungeon_invalid(dungeon)) {
        WARNLOG("Tried to init player %d which has no dungeon",(int)player->id_number);
        return;
    }
    if (!thing_is_invalid(thing))
    {
        dungeon->dnheart_idx = thing->index;
        dungeon->mappos.x.val = thing->mappos.x.val;
        dungeon->mappos.y.val = thing->mappos.y.val;
        dungeon->mappos.z.val = thing->mappos.z.val;
    } else
    {
        dungeon->dnheart_idx = 0;
        // If the player had a heart at it was destroyed, we shouldn't replace
        // the heart position - it's needed for Floating Spirit
        if (!keep_prev)
        {
            dungeon->mappos.x.val = subtile_coord_center(map_subtiles_x/2);
            dungeon->mappos.y.val = subtile_coord_center(map_subtiles_y/2);
            dungeon->mappos.z.val = subtile_coord_center(map_subtiles_z/2);
        }
    }
}

void setup_computer_player(int plr_idx)
{
    struct Thing *thing;
    SYNCDBG(5,"Starting for player %d",plr_idx);
    thing = find_players_dungeon_heart(plr_idx);// cannot use player->id_number, as it isn't set yet
    if (!thing_is_invalid(thing))
    {
        script_support_setup_player_as_computer_keeper(plr_idx, 0);
    } else
    {
        script_support_setup_player_as_zombie_keeper(plr_idx);
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

void setup_zombie_players(void)
{
  struct PlayerInfo *player;
  int plr_idx;
  for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
  {
      player = get_player(plr_idx);
      if (!player_exists(player))
      {
          script_support_setup_player_as_zombie_keeper(plr_idx);
      }
  }
}

void init_all_creature_states(void)
{
    struct Thing *thing;
    int i,k;
    k = 0;
    i = game.thing_lists[TngList_Creatures].index;
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
        init_creature_state(thing);
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
}

void remove_thing_from_mapwho(struct Thing *thing)
{
    struct Map *mapblk;
    struct Thing *mwtng;
    SYNCDBG(18,"Starting");
    //_DK_remove_thing_from_mapwho(thing);
    if ((thing->alloc_flags & TAlF_IsInMapWho) == 0)
        return;
    if (thing->prev_on_mapblk > 0)
    {
        mwtng = thing_get(thing->prev_on_mapblk);
        mwtng->next_on_mapblk = thing->next_on_mapblk;
    } else
    {
        mapblk = get_map_block_at(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
        set_mapwho_thing_index(mapblk, thing->next_on_mapblk);
    }
    if (thing->next_on_mapblk > 0)
    {
        mwtng = thing_get(thing->next_on_mapblk);
        mwtng->prev_on_mapblk = thing->prev_on_mapblk;
    }
    thing->next_on_mapblk = 0;
    thing->prev_on_mapblk = 0;
    thing->alloc_flags &= ~TAlF_IsInMapWho;
}

void place_thing_in_mapwho(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  _DK_place_thing_in_mapwho(thing);
}

struct Thing *find_base_thing_on_mapwho(ThingClass oclass, ThingModel model, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    //return _DK_find_base_thing_on_mapwho(oclass, model, stl_x, stl_y);
    struct Map *mapblk;
    long i;
    unsigned long k;
    mapblk = get_map_block_at(stl_x,stl_y);
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing->class_id == oclass)
        {
            if ((thing->model == model) || (model == 0)) {
                return thing;
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
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
    i = game.thing_lists[TngList_Objects].index;
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
      if ((object_is_hero_gate(thing)) && (thing->byte_13 == num))
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

/**
 * Returns a creature lair from given subtile.
 */
struct Thing *find_creature_lair_at_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, ThingModel crmodel)
{
    struct Map *mapblk;
    struct Thing *thing;
    unsigned long k;
    long i;
    mapblk = get_map_block_at(stl_x, stl_y);
    i = get_mapwho_thing_index(mapblk);
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per-thing code
        if (thing->class_id == TCls_Object)
        {
            struct Objects *objdat;
            objdat = get_objects_data_for_thing(thing);
            if (objdat->related_creatr_model > 0)
            {
                if ((crmodel <= 0) || (objdat->related_creatr_model == crmodel))
                    return thing;
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
    return INVALID_THING;
}

struct Thing *get_thing_of_class_with_filter(Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    unsigned long k;
    long i;
    struct Thing *retng;
    long maximizer;
    maximizer = 0;
    retng = INVALID_THING;
    SYNCDBG(19,"Starting");
    struct StructureList *slist;
    slist = get_list_for_thing_class(param->class_id);
    if (slist == NULL) {
        return INVALID_THING;
    }
    i = slist->index;
    k = 0;
    while (i != 0)
    {
        struct Thing *thing;
        long n;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        n = filter(thing, param, maximizer);
        if (n > maximizer)
        {
            retng = thing;
            maximizer = n;
            if (maximizer == LONG_MAX)
                break;
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return retng;
}

/** Finds on whole map a thing owned by given player, which matches given bool filter.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose things will be searched. Allies are not included, use -1 to select all.
 * @return The target thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_nearest_object_owned_by_and_matching_bool_filter(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, Thing_Bool_Filter matcher_cb)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = near_map_block_thing_filter_call_bool_filter;
    param.class_id = TCls_Object;
    param.model_id = -1;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    param.ptr3 = (void *)matcher_cb;
    return get_thing_of_class_with_filter(filter, &param);
}

struct Thing *find_nearest_enemy_creature(struct Thing *creatng)
{
  struct Thing *thing;
  unsigned long k;
  long i;
  struct Thing *neartng;
  long neardist,dist;
  neardist = LONG_MAX;
  neartng = INVALID_THING;
  i = game.thing_lists[TngList_Creatures].index;
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
    if (players_are_enemies(creatng->owner, thing->owner))
    {
      if (creature_will_attack_creature(creatng, thing))
      {
          dist = get_2d_box_distance(&creatng->mappos, &thing->mappos);
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

long creature_of_model_in_prison_or_tortured(ThingModel crmodel)
{
  struct Thing *thing;
  long i,k;
  i = game.thing_lists[TngList_Creatures].index;
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
    if ((crmodel <= 0) || (thing->model == crmodel))
    {
      if (creature_is_kept_in_prison(thing) || creature_is_being_tortured(thing))
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

TbBool lord_of_the_land_in_prison_or_tortured(void)
{
    struct CreatureModelConfig *crconf;
    long crtr_model;
    for (crtr_model=0; crtr_model < crtr_conf.model_count; crtr_model++)
    {
        crconf = &crtr_conf.model[crtr_model];
        if ((crconf->model_flags & MF_IsLordOTLand) != 0)
        {
            if (creature_of_model_in_prison_or_tortured(crtr_model) > 0)
                return true;
        }
    }
    return false;
}

TbBool perform_action_on_all_creatures_in_group(struct Thing *thing, Thing_Bool_Modifier action)
{
    struct CreatureControl *cctrl;
    TbBool result;
    struct Thing *ntng;
    struct Thing *ctng;
    long k;
    cctrl = creature_control_get_from_thing(thing);
    if (!creature_is_group_member(thing))
        return false;
    // Find the last creature in group
    ctng = get_group_last_member(thing);
    result = true;
    // Do the action for every creature in the group, starting from end
    // This allows the creatures to be removed from group or deleted during the update
    k = 0;
    while (!thing_is_invalid(ctng))
    {
        cctrl = creature_control_get_from_thing(ctng);
        ntng = thing_get(cctrl->prev_in_group);
        TRACE_THING(ntng);
        if (!thing_is_invalid(ntng))
        {
            result &= action(ctng);
            ctng = ntng;
        } else
        {
            ctng = INVALID_THING;
        }
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures group");
            return false;
        }
    }
    return result;
}

/**
 * Affects a thing with electric shock.
 *
 * @param tngsrc The thing which caused the affect.
 * @param tngdst The thing being affected by the effect.
 * @param pos Position of the effect epicenter.
 * @param max_dist Max distance at which creatures are affected, in map coordinates.
 * @param max_damage Damage at epicenter of the explosion.
 * @param owner The owning player index of the explosion.
 * @return Gives true if the target thing was affected by the spell, false otherwise.
 * @note If the function returns true, the effect might have caused death of the target.
 */
TbBool electricity_affecting_thing(struct Thing *tngsrc, struct Thing *tngdst, const struct Coord3d *pos,
    MapCoord max_dist, HitPoints max_damage, PlayerNumber owner)
{
    MapCoord distance;
    TbBool affected;
    affected = false;
    if (!line_of_sight_3d(pos, &tngdst->mappos)) {
        max_dist /= 3;
    }
    if (tngdst->owner == owner) {
        max_dist /= 3;
    }
    distance = get_2d_box_distance(pos, &tngdst->mappos);
    if (distance < max_dist)
    {
        HitPoints damage;
        if (tngdst->class_id == TCls_Creature)
        {
            damage = get_radially_decaying_value(max_damage, max_dist/2, max_dist/2, distance);
            if (damage != 0)
            {
                apply_damage_to_thing_and_display_health(tngdst, damage, owner);
                affected = true;
            }
        }
        // If the thing is a dying creature
        if ((tngdst->class_id == TCls_Creature) && (tngdst->health < 0))
        {
            kill_creature(tngdst, tngsrc, owner, CrDed_DiedInBattle);
            affected = true;
        }
    }
    return affected;
}

long electricity_affecting_area(const struct Coord3d *pos, PlayerNumber immune_plyr_idx, long range, long max_damage)
{
    struct Thing *thing;
    unsigned long k;
    long i;
    long naffected;
    naffected = 0;
    i = game.thing_lists[TngList_Creatures].index;
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
        if (!thing_is_picked_up(thing))
        {
            if (thing->owner != immune_plyr_idx)
            {
              if (!creature_affected_by_spell(thing, SplK_Armour))
              {
                  if (electricity_affecting_thing(INVALID_THING, thing, pos, range, max_damage, immune_plyr_idx))
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

/** Does a function on all creatures in players list of given model.
 * @return Count of creatures for which the callback returned true.
 */
long do_on_player_list_all_creatures_of_model(long thing_idx, ThingModel crmodel,
    Thing_Bool_Modifier do_cb)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i, n;
    n = 0;
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
        i = cctrl->players_next_creature_idx;
        // Per creature code
        if (do_cb(thing))
            n++;
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}

/** Does a function on all player creatures of given model.
 * @param plyr_idx Target player.
 * @param crmodel Creature model, or -1 for all (except special diggers).
 *
 * @return Count of creatures for which the callback returned true.
 */
long do_on_players_all_creatures_of_model(PlayerNumber plyr_idx, ThingModel crmodel, Thing_Bool_Modifier do_cb)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    return do_on_player_list_all_creatures_of_model(dungeon->creatr_list_start, crmodel, do_cb);
}


/** Counts creatures of given model belonging to given player.
 * @param plyr_idx Target player.
 * @param crmodel Creature model, or -1 for all (except special diggers).
 *
 * @return Count of players creatures.
 */
long count_player_creatures_of_model(PlayerNumber plyr_idx, ThingModel crmodel)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    return count_player_list_creatures_of_model(dungeon->creatr_list_start, crmodel);
}

long count_player_list_creatures_of_model(long thing_idx, ThingModel crmodel)
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
      i = cctrl->players_next_creature_idx;
      // Per creature code
      if ((crmodel <= 0) || (thing->model == crmodel))
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

struct Thing *get_player_list_nth_creature_of_model(long thing_idx, ThingModel crmodel, long crtr_idx)
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
      i = cctrl->players_next_creature_idx;
      // Per creature code
      if (crtr_idx <= 0)
          return thing;
      if ((crmodel <= 0) || (thing->model == crmodel))
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

long count_player_creatures_not_counting_to_total(PlayerNumber plyr_idx)
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
        i = cctrl->players_next_creature_idx;
        // Per creature code
        if (creature_is_kept_in_prison(thing))
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

struct Thing *get_random_players_creature_of_model(PlayerNumber plyr_idx, ThingModel crmodel)
{
    struct Dungeon *dungeon;
    long total_count,crtr_idx;
    dungeon = get_players_num_dungeon(plyr_idx);
    total_count = count_player_list_creatures_of_model(dungeon->creatr_list_start, crmodel);
    if (total_count < 1)
        return INVALID_THING;
    crtr_idx = ACTION_RANDOM(total_count);
    return get_player_list_nth_creature_of_model(dungeon->creatr_list_start, crmodel, crtr_idx);
}

/**
 * Returns amount of filtered creatures from the players creature list starting from thing_idx.
 * Only creatures for whom the filter function will return LONG_MAX, are counted.
 * @return Gives the amount of things which matched the filter.
 */
long count_player_list_creatures_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    long count;
    long maximizer;
    unsigned long k;
    long i,n;
    SYNCDBG(9,"Starting");
    count = 0;
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
        i = cctrl->players_next_creature_idx;
        // Per creature code
        n = filter(thing, param, maximizer);
        if (n >= maximizer)
        {
            maximizer = n;
            if (maximizer == LONG_MAX)
                count++;
        }
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

/**
 * Returns filtered creature from the players creature list starting from thing_idx.
 * The creature which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return LONG_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Gives the thing, or invalid thing pointer if not found.
 */
struct Thing *get_player_list_creature_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
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
        i = cctrl->players_next_creature_idx;
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
 * @return Gives the thing, or invalid thing pointer if not found.
 * @see get_player_list_creature_with_filter()
 */
struct Thing *get_player_list_random_creature_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
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
  total_count = count_player_list_creatures_of_model(thing_idx, 0);
  retng = INVALID_THING;
  maximizer = 0;
  if (total_count < 1)
      return retng;
  k = 0;
  // Get random index of a thing in list
  thing = get_player_list_nth_creature_of_model(thing_idx, 0, ACTION_RANDOM(total_count));
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
    i = cctrl->players_next_creature_idx;
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
 * @return Gives the thing, or invalid thing pointer if not found.
 */
struct Thing *get_thing_on_map_block_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param, long *maximizer)
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
      i = thing->next_on_mapblk;
      // Begin per-loop code
      n = filter(thing, param, *maximizer);
      if (n > *maximizer)
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
struct Thing *get_thing_near_revealed_map_block_with_filter(MapCoord x, MapCoord y, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
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
            if (!thing_is_invalid(thing) && (n > maximizer))
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

/**
 * Returns filtered creature from slabs around given coordinates.
 * Uses "spiral" checking of surrounding subtiles, up to given number of subtiles.
 * The thing which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return LONG_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *get_thing_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    struct MapOffset *sstep;
    struct Thing *retng;
    long maximizer;
    struct Map *mapblk;
    MapSubtlCoord sx,sy;
    int around;
    SYNCDBG(19,"Starting");
    retng = INVALID_THING;
    maximizer = 0;
    for (around=0; around < spiral_len; around++)
    {
      sstep = &spiral_step[around];
      sx = coord_subtile(x) + (MapSubtlCoord)sstep->h;
      sy = coord_subtile(y) + (MapSubtlCoord)sstep->v;
      mapblk = get_map_block_at(sx, sy);
      if (!map_block_invalid(mapblk))
      {
          struct Thing *thing;
          long i,n;
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
    return retng;
}

/**
 * Returns count of filtered creatures from subtiles around given coordinates.
 * Uses "spiral" checking of surrounding subtiles, up to given number of subtiles.
 * Amount of things for whom the filter function returns LONG_MAX, is returned.
 * @return Gives count of things which matched the filter.
 */
long count_things_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    struct MapOffset *sstep;
    struct Thing *thing;
    long count;
    long maximizer;
    struct Map *mapblk;
    MapSubtlCoord sx,sy;
    int around;
    long i,n;
    SYNCDBG(19,"Starting");
    count = 0;
    maximizer = 0;
    for (around=0; around < spiral_len; around++)
    {
      sstep = &spiral_step[around];
      sx = coord_subtile(x) + (MapSubtlCoord)sstep->h;
      sy = coord_subtile(y) + (MapSubtlCoord)sstep->v;
      mapblk = get_map_block_at(sx, sy);
      if (!map_block_invalid(mapblk))
      {
          i = get_mapwho_thing_index(mapblk);
          n = maximizer;
          thing = get_thing_on_map_block_with_filter(i, filter, param, &n);
          if (!thing_is_invalid(thing) && (n >= maximizer))
          {
              maximizer = n;
              if (maximizer == LONG_MAX)
                  count++;
          }
      }
    }
    return count;
}

void stop_all_things_playing_samples(void)
{
  struct Thing *thing;
  long i;
  for (i=0; i < THINGS_COUNT; i++)
  {
    thing = thing_get(i);
    if ((thing->alloc_flags & TAlF_Exists) != 0)
    {
      if (thing->snd_emitter_id)
      {
        S3DDestroySoundEmitterAndSamples(thing->snd_emitter_id);
        thing->snd_emitter_id = 0;
      }
    }
  }
}

TbBool update_thing(struct Thing *thing)
{
    Thing_Class_Func classfunc;
    struct Coord3d pos;
    SYNCDBG(18,"Thing index %d, class %d",(int)thing->index,(int)thing->class_id);
    TRACE_THING(thing);
    if (thing_is_invalid(thing))
        return false;
    if ((thing->movement_flags & TMvF_Unknown40) == 0)
    {
        if ((thing->field_1 & 0x04) != 0)
        {
            thing->pos_2C.x.val += thing->acceleration.x.val;
            thing->pos_2C.y.val += thing->acceleration.y.val;
            thing->pos_2C.z.val += thing->acceleration.z.val;
            thing->acceleration.x.val = 0;
            thing->acceleration.y.val = 0;
            thing->acceleration.z.val = 0;
            set_flag_byte(&thing->field_1, 0x04, false);
        }
        thing->velocity.x.val = thing->pos_2C.x.val;
        thing->velocity.y.val = thing->pos_2C.y.val;
        thing->velocity.z.val = thing->pos_2C.z.val;
        if ((thing->field_1 & 0x08) != 0)
        {
          thing->velocity.x.val += thing->pos_26.x.val;
          thing->velocity.y.val += thing->pos_26.y.val;
          thing->velocity.z.val += thing->pos_26.z.val;
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
    if (classfunc(thing) == TUFRet_Deleted) {
        return false;
    }
    SYNCDBG(18,"Class function end ok");
    if ((thing->movement_flags & TMvF_Unknown40) == 0)
    {
        if (thing->mappos.z.val > thing->field_60)
        {
            if (thing->pos_2C.x.val != 0)
              thing->pos_2C.x.val = thing->pos_2C.x.val * (256 - (int)thing->field_24) / 256;
            if (thing->pos_2C.y.val != 0)
              thing->pos_2C.y.val = thing->pos_2C.y.val * (256 - (int)thing->field_24) / 256;
            if ((thing->movement_flags & TMvF_Flying) == 0)
            {
                thing->acceleration.z.val -= thing->field_20;
                thing->field_1 |= 0x04;
            }
        } else
        {
            if (thing->pos_2C.x.val != 0)
              thing->pos_2C.x.val = thing->pos_2C.x.val * (256 - (int)thing->field_23) / 256;
            if (thing->pos_2C.y.val != 0)
              thing->pos_2C.y.val = thing->pos_2C.y.val * (256 - (int)thing->field_23) / 256;
            thing->mappos.z.val = thing->field_60;
            if ((thing->movement_flags & TMvF_Unknown08) != 0)
            {
              thing->pos_2C.z.val = 0;
            }
        }
    }
    update_thing_animation(thing);
    update_thing_sound(thing);
    if ((do_lights) && (thing->light_id != 0))
    {
        if (light_is_light_allocated(thing->light_id))
        {
            pos.x.val = thing->mappos.x.val;
            pos.y.val = thing->mappos.y.val;
            pos.z.val = thing->mappos.z.val + thing->field_58;
            light_set_light_position(thing->light_id, &pos);
        } else
        {
            WARNLOG("The %s tries to use non-existing light %d",thing_model_name(thing),(int)thing->light_id);
            thing->light_id = 0;
        }
    }
    SYNCDBG(18,"Finished");
    return true;
}

TbBigChecksum get_thing_checksum(const struct Thing *thing)
{
    TbBigChecksum csum;
    SYNCDBG(18,"Starting");
    //return _DK_get_thing_checksum(thing);
    if (!thing_exists(thing))
        return 0;
    csum = (ulong)thing->class_id +
        (ulong)thing->mappos.z.val +
        (ulong)thing->mappos.x.val +
        (ulong)thing->mappos.y.val +
        (ulong)thing->health + (ulong)thing->model + (ulong)thing->owner;
    if (thing->class_id == 5)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        csum += (ulong)cctrl->field_D4 + (ulong)cctrl->instance_id
            + (ulong)thing->field_49 + (ulong)thing->field_48;
    }
    return csum * thing->index;
}

short update_thing_sound(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  if (thing->snd_emitter_id)
  {
    if ( S3DEmitterHasFinishedPlaying(thing->snd_emitter_id) )
    {
      S3DDestroySoundEmitter(thing->snd_emitter_id);
      thing->snd_emitter_id = 0;
    } else
    {
      S3DMoveSoundEmitterTo(thing->snd_emitter_id,
          thing->mappos.x.val, thing->mappos.y.val, thing->mappos.z.val);
    }
  }
  return true;
}

TbBool thing_is_shootable_by_any_player_including_objects(const struct Thing *thing, PlayerNumber shot_owner)
{
    //return _DK_thing_is_shootable_by_any_player_including_objects(thing);
    if (thing_is_creature(thing))
    {
        // spectators are not shootable
        if ((get_creature_model_flags(thing) & MF_IsSpectator) != 0)
            return false;
        /* This would disallow killing unconscious creatures, so we can't
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->flgfield_1 & CCFlg_Immortal) != 0)
            return false;*/
        return true;
    }
    if (thing_is_object(thing))
    {
        if (thing_is_dungeon_heart(thing))
            return true;
        if (object_is_growing_food(thing))
            return true;
        if (object_is_mature_food(thing) && !is_thing_passenger_controlled(thing))
            return true;
        if (object_is_gold_pile(thing))
            return true;
        if (thing_is_door_or_trap_box(thing))
            return true;
        if (thing_is_spellbook(thing) || thing_is_special_box(thing))
            return true;
        return false;
    }
    return false;
}

TbBool thing_is_shootable_by_any_player_except_own_including_objects(const struct Thing *thing, PlayerNumber shot_owner)
{
    //return _DK_thing_is_shootable_by_any_player_except_own_including_objects(shooter, thing);
    if (thing_is_creature(thing))
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if ((get_creature_model_flags(thing) & MF_IsSpectator) != 0)
            return false;
        if (((cctrl->flgfield_1 & CCFlg_Immortal) != 0) || (thing->owner == shot_owner))
            return false;
        return true;
    }
    if (thing_is_object(thing))
    {
        if (thing->owner == shot_owner)
            return false;
        if (thing_is_dungeon_heart(thing))
            return true;
        if (object_is_growing_food(thing))
            return true;
        if (object_is_mature_food(thing) && !is_thing_passenger_controlled(thing))
            return true;
        if (object_is_gold_pile(thing))
            return true;
        if (thing_is_door_or_trap_box(thing))
            return true;
        if (thing_is_spellbook(thing) || thing_is_special_box(thing))
            return true;
        return false;
    }
    return false;
}

TbBool thing_is_shootable_by_any_player_except_own_excluding_objects(const struct Thing *thing, PlayerNumber shot_owner)
{
    //return _DK_thing_is_shootable_by_any_player_except_own_excluding_objects(shooter, thing);
    if (thing_is_creature(thing))
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if ((get_creature_model_flags(thing) & MF_IsSpectator) != 0)
            return false;
        if (((cctrl->flgfield_1 & CCFlg_Immortal) != 0) || (thing->owner == shot_owner))
            return false;
        return true;
    }
    return false;
}

TbBool thing_is_shootable_by_any_player_except_own_excluding_objects_and_not_under_spell(const struct Thing *thing, PlayerNumber shot_owner, SpellKind spkind)
{
    if (thing_is_creature(thing))
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if ((get_creature_model_flags(thing) & MF_IsSpectator) != 0)
            return false;
        if (((cctrl->flgfield_1 & CCFlg_Immortal) != 0) || (thing->owner == shot_owner))
            return false;
        if (creature_affected_by_spell(thing, spkind))
            return false;
        return true;
    }
    return false;
}

TbBool thing_is_shootable_by_any_player_excluding_objects(const struct Thing *thing, PlayerNumber shot_owner)
{
    //return _DK_thing_is_shootable_by_any_player_excluding_objects(thing);
    if (thing_is_creature(thing))
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        // spectators are not shootable
        if ((get_creature_model_flags(thing) & MF_IsSpectator) != 0)
            return false;
        if ((cctrl->flgfield_1 & CCFlg_Immortal) != 0)
            return false;
        return true;
    }
    return false;
}

/**
 * Returns if there's a creature digging at given map coordinates.
 * @param excltng The thing to exclude from search (to be ignored even if it's digging there).
 * @param stl_x Dig site X coordinate.
 * @param stl_y Dig site Y coordinate.
 * @return
 */
TbBool imp_already_digging_at_excluding(struct Thing *excltng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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
    i = thing->next_on_mapblk;
    // Per thing processing block
    if ((thing->class_id == TCls_Creature) && (thing->index != excltng->index))
    {
        if (!thing_is_picked_up(thing))
        {
            if ((thing->active_state == CrSt_ImpDigsDirt) || (thing->active_state == CrSt_ImpMinesGold))
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

struct Thing *smallest_gold_pile_at_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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
    i = thing->next_on_mapblk;
    // Per thing processing block
    if ((thing->class_id == TCls_Object) && (thing->model == 43))
    {
        if (thing->creature.gold_carried < chosen_gold)
        {
            chosen_thing = thing;
            chosen_gold = thing->creature.gold_carried;
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

TbBool update_speed_of_player_creatures_of_model(PlayerNumber plyr_idx, ThingModel crmodel)
{
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  int i;
  SYNCDBG(8,"Starting");
  dungeon = get_players_num_dungeon(plyr_idx);
  k = 0;
  i = dungeon->creatr_list_start;
  while (i != 0)
  {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature detected");
        break;
      }
      i = cctrl->players_next_creature_idx;
      // Thing list loop body
      if ((crmodel <= 0) || (thing->model == crmodel))
      {
          cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
      }
      // Thing list loop body ends
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
  }
  k = 0;
  i = dungeon->digger_list_start;
  while (i != 0)
  {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature detected");
        break;
      }
      i = cctrl->players_next_creature_idx;
      // Thing list loop body
      if ((crmodel <= 0) || (thing->model == crmodel))
      {
          cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
      }
      // Thing list loop body ends
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
  }
  SYNCDBG(19,"Finished");
  return true;
}

TbBool apply_anger_to_all_players_creatures_excluding(PlayerNumber plyr_idx, long anger, long reason, const struct Thing *excltng)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    //return _DK_make_all_players_creatures_angry(plyr_idx);
    dungeon = get_players_num_dungeon(plyr_idx);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (thing->index != excltng->index) {
            anger_apply_anger_to_creature(thing, anger, reason, 1);
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19,"Finished");
    return true;
}

TbBool gold_pile_with_maximum_at_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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
    i = thing->next_on_mapblk;
    // Per thing processing block
    if ((thing->class_id == TCls_Object) && (thing->model == 43))
    {
        if (thing->creature.gold_carried >= game.gold_pile_maximum)
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
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    //return get_creature_near_with_filter(x, y, creature_near_filter_not_imp, plyr_idx);
    filter = near_map_block_thing_filter_not_specdigger;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds thing on revealed subtiles around given position, which matches given bool filter.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose things will be searched. Alies are not included.
 * @return The target thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_object_around_owned_by_and_matching_bool_filter(MapCoord pos_x, MapCoord pos_y, long plyr_idx, Thing_Bool_Filter matcher_cb)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = near_map_block_thing_filter_call_bool_filter;
    param.class_id = TCls_Object;
    param.model_id = -1;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    param.ptr3 = (void *)matcher_cb;
    return get_thing_spiral_near_map_block_with_filter(pos_x, pos_y, 9, filter, &param);
}

/** Finds creature on revealed subtiles around given position, who is not special digger and is enemy to given player.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose revealed subtiles around will be searched.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near_who_is_enemy_of_and_not_specdigger(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    //return get_creature_near_with_filter(x, y, creature_near_filter_is_enemy_of_and_not_specdigger, plyr_idx);
    filter = near_map_block_thing_filter_is_enemy_of_and_not_specdigger;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds creature on revealed subtiles around given position, which can be slapped by given player.
 *
 * @param plyr_idx Player whose creature from revealed position will be returned.
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_nearest_thing_for_slap(PlayerNumber plyr_idx, MapCoord pos_x, MapCoord pos_y)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    //return _DK_get_nearest_thing_for_slap(plyr_idx, pos_x, pos_y);
    filter = near_map_block_thing_filter_is_slappable;
    param.class_id = -1;
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
struct Thing *get_creature_near_and_owned_by(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    //return get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
    filter = near_map_block_thing_filter_is_owned_by;
    param.class_id = TCls_Creature;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds creature on all subtiles around given position, who belongs to given player or allied one.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose creature or allied creature will be returned.
 * @param distance_stl Max. distance, in subtiles. Will work properly only for odd numbers (1,3,5,7...).
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, long distance_stl, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = map_block_thing_filter_is_of_class_and_model_and_owned_by_or_allied_with;
    param.class_id = TCls_Creature;
    param.model_id = -1;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_spiral_near_map_block_with_filter(pos_x, pos_y, distance_stl*distance_stl, filter, &param);
}

/** Counts creatures on all subtiles around given position, who belongs to given player or allied one.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose creatures and allied creatures count will be returned.
 * @param distance_stl Max. distance, in subtiles. Will work properly only for odd numbers (1,3,5,7...).
 * @return The count of matching creatures on given coordinate range.
 */
long count_creatures_near_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, long distance_stl, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = map_block_thing_filter_is_of_class_and_model_and_owned_by_or_allied_with;
    param.class_id = TCls_Creature;
    param.model_id = -1;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return count_things_spiral_near_map_block_with_filter(pos_x, pos_y, distance_stl*distance_stl, filter, &param);
}

// use this (or make similar one) instead of find_base_thing_on_mapwho_at_pos()
struct Thing *get_object_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
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

struct Thing *get_food_at_subtile_available_to_eat_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    const struct Map *mapblk;
    long i,n;
    SYNCDBG(19,"Starting");
    filter = map_block_thing_filter_is_food_available_to_eat_and_owned_by;
    param.class_id = -1;
    param.model_id = -1;
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

struct Thing *get_trap_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    const struct Map *mapblk;
    long i,n;
    SYNCDBG(19,"Starting");
    filter = map_block_thing_filter_is_of_class_and_model_and_owned_by;
    param.class_id = TCls_Trap;
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

/** Finds trap on all subtiles around given one, which belongs to given player and is of given model.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose creature or allied creature will be returned.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_trap_around_of_model_and_owned_by(MapCoord pos_x, MapCoord pos_y, long model, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = map_block_thing_filter_is_of_class_and_model_and_owned_by;
    param.class_id = TCls_Trap;
    param.model_id = model;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_spiral_near_map_block_with_filter(pos_x, pos_y, 9, filter, &param);
}

struct Thing *get_door_for_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    const struct Map *mapblk;
    MapSlabCoord slb_x,slb_y;
    long i,n;
    SYNCDBG(19,"Starting");
    //return _DK_get_door_for_position(pos_x, pos_y);
    filter = map_block_thing_filter_is_of_class_and_model_and_owned_by;
    param.class_id = TCls_Door;
    param.model_id = -1;
    param.plyr_idx = -1;
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
    mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    i = get_mapwho_thing_index(mapblk);
    n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_creature_of_model_training_at_subtile_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model_id, long plyr_idx, long skip_thing_id)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    const struct Map *mapblk;
    long i,n;
    SYNCDBG(19,"Starting");
    filter = map_block_creature_filter_of_model_training_and_owned_by;
    param.class_id = TCls_Creature;
    param.model_id = model_id;
    param.plyr_idx = plyr_idx;
    param.num1 = skip_thing_id;
    mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    i = get_mapwho_thing_index(mapblk);
    n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_nearest_object_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  return _DK_get_nearest_object_at_position(stl_x, stl_y);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
