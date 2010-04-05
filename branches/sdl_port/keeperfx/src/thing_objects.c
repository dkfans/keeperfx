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
#include "bflib_memory.h"
#include "bflib_sound.h"
#include "map_data.h"
#include "map_columns.h"
#include "keeperfx.hpp"

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
DLLIMPORT long _DK_object_is_gold_pile(struct Thing *thing);
DLLIMPORT long _DK_object_is_gold(struct Thing *thing);
DLLIMPORT long _DK_remove_gold_from_hoarde(struct Thing *thing, struct Room *room, long amount);
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

TbBool thing_is_dungeon_heart(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  return (thing->class_id == TCls_Object) && (thing->model == 5);
}

TbBool thing_is_mature_food(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  return (thing->class_id == TCls_Object) && (thing->model == 10);
}

TbBool object_is_mature_food(const struct Thing *thing)
{
  return (thing->model == 10);
}

TbBool object_is_gold(const struct Thing *thing)
{
  //return _DK_object_is_gold(thing);
  switch (thing->model)
  {
    case 3:
    case 6:
    case 43:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
        return true;
    default:
        return false;
  }
}

TbBool object_is_gold_pile(const struct Thing *thing)
{
  //return _DK_object_is_gold_pile(thing);
  switch (thing->model)
  {
    case 3:
    case 6:
    case 43:
    case 128:
        return true;
    default:
        return false;
  }
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

void update_dungeon_heart_beat(struct Thing *thing)
{
  long i;
  long long k;
  const long base_heart_beat_rate = 2304;
  static long bounce = 0;
  if (thing->field_7 != 3)
  {
    i = (char)thing->byte_13.h;
    thing->field_3E = 0;
    k = 384 * (game.objects_config[5].health - thing->health) / game.objects_config[5].health;
    k = base_heart_beat_rate / (k + 128);
    light_set_light_intensity(thing->field_62, light_get_light_intensity(thing->field_62) + (i*36/k));
    thing->field_40 += (i*base_heart_beat_rate/k);
    if (thing->field_40 < 0)
    {
      thing->field_40 = 0;
      light_set_light_intensity(thing->field_62, 20);
      thing->byte_13.h = 1;
    }
    if (thing->field_40 > base_heart_beat_rate-1)
    {
      thing->field_40 = base_heart_beat_rate-1;
      light_set_light_intensity(thing->field_62, 56);
      thing->byte_13.h = (unsigned char)-1;
      if ( bounce )
      {
        thing_play_sample(thing, 151, 100, 0, 3, 1, 6, 256);
      } else
      {
        thing_play_sample(thing, 150, 100, 0, 3, 1, 6, 256);
      }
      bounce = !bounce;
    }
    k = (((unsigned long long)thing->field_40 >> 32) & 0xFF) + thing->field_40;
    thing->field_48 = (k >> 8) & 0xFF;
    if ( !S3DEmitterIsPlayingSample(thing->field_66, 93, 0) )
      thing_play_sample(thing, 93, 100, -1, 3, 1, 6, 256);
  }
}

long object_update_dungeon_heart(struct Thing *thing)
{
  struct Dungeon *dungeon;
  long i;
  long long k;
  SYNCDBG(8,"Starting");
  //return _DK_object_update_dungeon_heart(thing);
  dungeon = get_players_num_dungeon(thing->owner);
  if ((thing->health > 0) && (game.dungeon_heart_heal_time != 0))
  {
    if ((game.play_gameturn % game.dungeon_heart_heal_time) == 0)
    {
        thing->health += game.dungeon_heart_heal_health;
        if (thing->health < 0)
        {
          thing->health = 0;
        } else
        if (thing->health > game.objects_config[5].health)
        {
          thing->health = game.objects_config[5].health;
        }
    }
    k = ((thing->health << 8) / game.objects_config[5].health) << 7;
    i = (saturate_set_signed(k,32) >> 8) + 128;
    thing->field_46 = i * objects_data[5].field_D >> 8;
    thing->field_56 = i * objects_data[5].field_9 >> 8;
  } else
  if (dungeon->field_1060[0] == 0)
  {
      LbMemorySet(dungeon->field_1060, 0, sizeof(dungeon->field_1060));
      dungeon->field_1060[0] = 1;
      dungeon->pos_1065.x.val = thing->mappos.x.val;
      dungeon->pos_1065.y.val = thing->mappos.y.val;
      dungeon->pos_1065.z.val = thing->mappos.z.val;
  }
  process_dungeon_destroy(thing);
  if ((thing->field_0 & 0x01) == 0)
    return 0;
  if ( thing->byte_13.l )
    thing->byte_13.l--;
  update_dungeon_heart_beat(thing);
  return 1;
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
  Thing_State_Func upcallback;
  Thing_State_Func stcallback;
  struct Objects *objdat;
  SYNCDBG(18,"Starting for model %d",(int)thing->model);
  //return _DK_update_object(thing);

  upcallback = object_update_functions[thing->model];
  if (upcallback != NULL)
  {
    if (upcallback(thing) <= 0)
      return -1;
  }
  stcallback = object_state_functions[thing->field_7];
  if (stcallback != NULL)
  {
    if (stcallback(thing) <= 0)
      return -1;
  }
  thing->field_25 &= 0xFE;
  thing->field_25 &= 0xFD;
  if ( ((thing->field_25 & 0x40) == 0) && thing_touching_floor(thing) )
  {
    if ( map_pos_is_lava(thing->mappos.x.stl.num, thing->mappos.y.stl.num) )
    {
      thing->field_25 |= 0x02;
      objdat = get_objects_data_for_thing(thing);
      if ( (objdat->field_12) && ((thing->field_1 & 0x01) == 0) && ((thing->field_0 & 0x80) == 0) )
      {
            if (thing->model == 10)
            {
              destroy_food(thing);
            } else
            {
              delete_thing_structure(thing, 0);
            }
            return -1;
      }
    } else
    if (get_top_cube_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num) == 39)
    {
      thing->field_25 |= 0x01;
    }
  }
  if ((thing->field_25 & 0x40) != 0)
    return 1;
  return move_object(thing);
}

struct Thing *create_gold_pot_at(long pos_x, long pos_y, long plyr_idx)
{
    struct Thing *thing;
    struct Coord3d pos;
    pos.x.val = pos_x;
    pos.y.val = pos_y;
    pos.z.val = (3 << 8);
    thing = create_object(&pos, 6, plyr_idx, -1);
    if (thing_is_invalid(thing))
        return NULL;
    thing->long_13 = game.pot_of_gold_holds;
    return thing;
}

long remove_gold_from_hoarde(struct Thing *thing, struct Room *room, long amount)
{
    return _DK_remove_gold_from_hoarde(thing, room, amount);
}

TbBool thing_is_gold_hoarde(struct Thing *thing)
{
    if (thing->class_id == TCls_Object)
    {
        return (thing->model == 52) || (thing->model == 53) || (thing->model == 54) || (thing->model == 55) || (thing->model == 56);
    }
    return false;
}

struct Thing *find_gold_hoarde_at(unsigned short stl_x, unsigned short stl_y)
{
    struct Thing *thing;
    struct Map *mapblk;
    long i;
    unsigned long k;
    k = 0;
    mapblk = get_map_block_at(stl_x,stl_y);
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
      // Per-thing block
      if (thing_is_gold_hoarde(thing))
          return thing;
      // Per-thing block ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    return INVALID_THING;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
