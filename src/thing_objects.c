/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_objects.c
 *     Things of class 'object' handling functions.
 * @par Purpose:
 *     Functions to maintain object things in the game.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Nov 2009 - 01 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_objects.h"

#include "globals.h"
#include "bflib_basics.h"
#include "map_data.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long food_moves(struct Thing *thing);
long food_grows(struct Thing *thing);
long object_being_dropped(struct Thing *thing);
long object_update_dungeon_heart(struct Thing *thing);
long object_update_call_to_arms(struct Thing *thing);
long object_update_armour(struct Thing *thing);
long object_update_object_scale(struct Thing *thing);
long object_update_armour2(struct Thing *thing);
long object_update_power_sight(struct Thing *thing);
long object_update_power_lightning(struct Thing *thing);

Thing_Class_Func object_state_functions[] = {
    NULL,
    food_moves,
    food_grows,
    NULL,
    object_being_dropped,
    NULL,
};

Thing_Class_Func object_update_functions[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_dungeon_heart,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_call_to_arms,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_armour,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_armour2,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_power_sight,
    object_update_power_lightning,
    object_update_object_scale,
    object_update_object_scale,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

/******************************************************************************/
DLLIMPORT long _DK_move_object(struct Thing *thing);
DLLIMPORT long _DK_update_object(struct Thing *thing);
DLLIMPORT long _DK_food_moves(struct Thing *thing);
DLLIMPORT long _DK_food_grows(struct Thing *thing);
DLLIMPORT long _DK_object_being_dropped(struct Thing *thing);
DLLIMPORT long _DK_object_update_dungeon_heart(struct Thing *thing);
DLLIMPORT long _DK_object_update_call_to_arms(struct Thing *thing);
DLLIMPORT long _DK_object_update_armour(struct Thing *thing);
DLLIMPORT long _DK_object_update_object_scale(struct Thing *thing);
DLLIMPORT long _DK_object_update_armour2(struct Thing *thing);
DLLIMPORT long _DK_object_update_power_sight(struct Thing *thing);
DLLIMPORT long _DK_object_update_power_lightning(struct Thing *thing);
/******************************************************************************/
struct Objects *get_objects_data_for_thing(struct Thing *thing)
{
  unsigned int tmodel;
  tmodel = thing->model;
  if (tmodel >= OBJECT_TYPES_COUNT)
    return &objects_data[0];
  return &objects_data[tmodel];
}

struct Objects *get_objects_data(unsigned int tmodel)
{
  if (tmodel >= OBJECT_TYPES_COUNT)
    return &objects_data[0];
  return &objects_data[tmodel];
}

unsigned int get_workshop_object_class_for_thing(struct Thing *thing)
{
  unsigned int tmodel;
  tmodel = thing->model;
  if (tmodel >= OBJECT_TYPES_COUNT)
    return workshop_object_class[0];
  return workshop_object_class[tmodel];
}

unsigned int get_workshop_object_class(unsigned int tmodel)
{
  if (tmodel >= OBJECT_TYPES_COUNT)
    return workshop_object_class[0];
  return workshop_object_class[tmodel];
}

int box_thing_to_special(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return 0;
  if ((thing->class_id != TCls_Object) || (thing->model >= OBJECT_TYPES_COUNT))
    return 0;
  return object_to_special[thing->model];
}

int book_thing_to_magic(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return 0;
  if ((thing->class_id != TCls_Object) || (thing->model >= OBJECT_TYPES_COUNT))
    return 0;
  return object_to_magic[thing->model];
}

int box_thing_to_door_or_trap(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return 0;
  if ((thing->class_id != TCls_Object) || (thing->model >= OBJECT_TYPES_COUNT))
    return 0;
  return object_to_door_or_trap[thing->model];
}

TbBool thing_is_special(const struct Thing *thing)
{
  return (box_thing_to_special(thing) > 0);
}

TbBool thing_is_door_or_trap(const struct Thing *thing)
{
  return (box_thing_to_door_or_trap(thing) > 0);
}

long food_moves(struct Thing *thing)
{
  return _DK_food_moves(thing);
}

long food_grows(struct Thing *thing)
{
  return _DK_food_grows(thing);
}

long object_being_dropped(struct Thing *thing)
{
  return _DK_object_being_dropped(thing);
}

long object_update_dungeon_heart(struct Thing *thing)
{
  return _DK_object_update_dungeon_heart(thing);
}

long object_update_call_to_arms(struct Thing *thing)
{
  return _DK_object_update_call_to_arms(thing);
}

long object_update_armour(struct Thing *thing)
{
  return _DK_object_update_armour(thing);
}

long object_update_object_scale(struct Thing *thing)
{
  return _DK_object_update_object_scale(thing);
}

long object_update_armour2(struct Thing *thing)
{
  return _DK_object_update_armour2(thing);
}

long object_update_power_sight(struct Thing *thing)
{
  return _DK_object_update_power_sight(thing);
}

long object_update_power_lightning(struct Thing *thing)
{
  return _DK_object_update_power_lightning(thing);
}

long move_object(struct Thing *thing)
{
  return _DK_move_object(thing);
}

long update_object(struct Thing *thing)
{
  SYNCDBG(18,"Starting for model %d",(int)thing->model);
  return _DK_update_object(thing);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
