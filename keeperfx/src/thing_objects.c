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
#include "thing_stats.h"
#include "map_data.h"
#include "map_columns.h"
#include "gui_topmsg.h"
#include "sounds.h"

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

unsigned char object_to_door_or_trap[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

unsigned short gold_hoard_objects[] = {52, 53, 54, 55, 56};

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
DLLIMPORT struct Thing *_DK_create_object(struct Coord3d *pos, unsigned short model, unsigned short owner, long parent_idx);
DLLIMPORT long _DK_thing_is_spellbook(struct Thing *thing);
DLLIMPORT struct Thing *_DK_get_crate_at_position(long x, long y);
DLLIMPORT struct Thing *_DK_get_spellbook_at_position(long x, long y);
DLLIMPORT struct Thing *_DK_get_special_at_position(long x, long y);
/******************************************************************************/
struct Thing *create_object(const struct Coord3d *pos, unsigned short model, unsigned short owner, long parent_idx)
{
    struct Objects *objdat;
    struct ObjectConfig *objconf;
    struct InitLight ilight;
    struct Thing *thing;
    long i,k;
    //thing = _DK_create_object(pos, model, owner, a4);

    if (!i_can_allocate_free_thing_structure(TAF_FreeEffectIfNoSlots))
    {
        ERRORDBG(3,"Cannot create object model %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(TAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate object %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_Object;
    thing->model = model;
    if (parent_idx == -1)
      thing->parent_thing_idx = -1;
    else
      thing->parent_thing_idx = parent_idx;
    LbMemoryCopy(&thing->mappos, pos, sizeof(struct Coord3d));
    objconf = &game.objects_config[model];
    objdat = get_objects_data_for_thing(thing);
    thing->sizexy = objdat->field_9;
    thing->field_58 = objdat->field_B;
    thing->field_5A = objdat->field_9;
    thing->field_5C = objdat->field_B;
    thing->health = saturate_set_signed(objconf->health,16);
    thing->field_20 = objconf->field_4;
    thing->field_23 = 204;
    thing->field_24 = 51;
    thing->field_22 = 0;
    thing->movement_flags |= TMvF_Unknown08;

    set_flag_byte(&thing->movement_flags, TMvF_Unknown40, objconf->field_8);
    thing->owner = owner;
    thing->field_9 = game.play_gameturn;

    if (!objdat->field_2)
    {
      i = convert_td_iso(objdat->field_5);
      k = 0;
    } else
    {
      i = convert_td_iso(objdat->field_5);
      k = -1;
    }
    set_thing_draw(thing, i, objdat->field_7, objdat->field_D, 0, k, objdat->field_11);
    set_flag_byte(&thing->field_4F, 0x02, objconf->field_5);
    set_flag_byte(&thing->field_4F, 0x01, objdat->field_3 & 0x01);
    set_flag_byte(&thing->field_4F, 0x10, objdat->field_F & 0x01);
    set_flag_byte(&thing->field_4F, 0x20, objdat->field_F & 0x02);
    thing->active_state = objdat->field_0;
    if (objconf->ilght.field_0 != 0)
    {
        LbMemorySet(&ilight, 0, sizeof(struct InitLight));
        LbMemoryCopy(&ilight.mappos, &thing->mappos, sizeof(struct Coord3d));
        ilight.field_0 = objconf->ilght.field_0;
        ilight.field_2 = objconf->ilght.field_2;
        ilight.field_3 = objconf->ilght.field_3;
        ilight.is_dynamic = objconf->ilght.is_dynamic;
        thing->light_id = light_create_light(&ilight);
        if (thing->light_id == 0) {
            SYNCDBG(8,"Cannot allocate light to %s.",thing_model_name(thing));
        }
    } else {
        thing->light_id = 0;
    }
    switch (thing->model)
    {
      case 3:
        thing->creature.gold_carried = game.chest_gold_hold;
        break;
      case 5:
        thing->byte_14 = 1;
        light_set_light_minimum_size_to_cache(thing->light_id, 0, 56);
        break;
      case 6:
        thing->creature.gold_carried = game.pot_of_gold_holds;
        break;
      case 33:
        set_flag_byte(&thing->field_4F, 0x10, false);
        set_flag_byte(&thing->field_4F, 0x20, true);
        break;
      case 43:
        thing->creature.gold_carried = game.gold_pile_value;
        break;
      case 49:
        i = get_free_hero_gate_number();
        if (i > 0)
        {
            thing->byte_13 = i;
        } else
        {
            thing->byte_13 = 0;
            ERRORLOG("Could not allocate number for hero gate");
        }
        break;
      default:
        break;
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

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

unsigned int get_workshop_object_class_for_thing(const struct Thing *thing)
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
    if ( (thing->class_id != TCls_Object) || (thing->model >= OBJECT_TYPES_COUNT) )
        return 0;
    return object_to_magic[thing->model];
}

int box_thing_to_door_or_trap(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return 0;
    if ( (thing->class_id != TCls_Object) ||
         (thing->model >= sizeof(object_to_door_or_trap)/sizeof(object_to_door_or_trap[0])) )
      return 0;
    return object_to_door_or_trap[thing->model];
}

TbBool thing_is_special_box(const struct Thing *thing)
{
    return (box_thing_to_special(thing) > 0);
}

TbBool thing_is_door_or_trap_box(const struct Thing *thing)
{
    return (box_thing_to_door_or_trap(thing) > 0);
}

TbBool thing_is_trap_box(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if ((thing->class_id != TCls_Object) || (thing->model >= OBJECT_TYPES_COUNT))
        return false;
    return (workshop_object_class[thing->model] == TCls_Trap);
}

TbBool thing_is_door_box(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if ((thing->class_id != TCls_Object) || (thing->model >= OBJECT_TYPES_COUNT))
        return false;
    return (workshop_object_class[thing->model] == TCls_Door);
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

long thing_is_spellbook(struct Thing *thing)
{
  return _DK_thing_is_spellbook(thing);
}

struct Thing *get_spellbook_at_position(long x, long y)
{
  return _DK_get_spellbook_at_position(x, y);
}

struct Thing *get_special_at_position(long x, long y)
{
  return _DK_get_special_at_position(x, y);
}

struct Thing *get_crate_at_position(long x, long y)
{
  return _DK_get_crate_at_position(x, y);
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
    if (thing->active_state != 3)
    {
        i = (char)thing->byte_14;
        thing->field_3E = 0;
        k = 384 * (long)(game.objects_config[5].health - thing->health) / game.objects_config[5].health;
        k = base_heart_beat_rate / (k + 128);
        light_set_light_intensity(thing->light_id, light_get_light_intensity(thing->light_id) + (i*36/k));
        thing->field_40 += (i*base_heart_beat_rate/k);
        if (thing->field_40 < 0)
        {
            thing->field_40 = 0;
            light_set_light_intensity(thing->light_id, 20);
            thing->byte_14 = 1;
        }
        if (thing->field_40 > base_heart_beat_rate-1)
        {
            thing->field_40 = base_heart_beat_rate-1;
            light_set_light_intensity(thing->light_id, 56);
            thing->byte_14 = (unsigned char)-1;
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
        if ( !S3DEmitterIsPlayingSample(thing->snd_emitter_id, 93, 0) )
          thing_play_sample(thing, 93, 100, -1, 3, 1, 6, 256);
    }
}

long object_update_dungeon_heart(struct Thing *thing)
{
    struct Dungeon *dungeon;
    long i;
    long long k;
    SYNCDBG(18,"Starting");
    //return _DK_object_update_dungeon_heart(thing);
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
      thing->field_46 = i * (long)objects_data[5].field_D >> 8;
      thing->sizexy = i * (long)objects_data[5].field_9 >> 8;
    } else
    if (thing->owner != game.neutral_player_num)
    {
        dungeon = get_players_num_dungeon(thing->owner);
        if (dungeon->field_1060[0] == 0)
        {
            LbMemorySet(dungeon->field_1060, 0, sizeof(dungeon->field_1060));
            dungeon->field_1060[0] = 1;
            dungeon->pos_1065.x.val = thing->mappos.x.val;
            dungeon->pos_1065.y.val = thing->mappos.y.val;
            dungeon->pos_1065.z.val = thing->mappos.z.val;
        }
    }
    process_dungeon_destroy(thing);
    SYNCDBG(18,"Beat update");
    if ((thing->field_0 & 0x01) == 0)
      return 0;
    if ( thing->byte_13 )
      thing->byte_13--;
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
    SYNCDBG(18,"Starting for %s",thing_model_name(thing));
    //return _DK_update_object(thing);

    upcallback = NULL;
    if (thing->model < sizeof(object_update_functions)/sizeof(object_update_functions[0])) {
        upcallback = object_update_functions[thing->model];
    } else {
        ERRORLOG("Object model %d exceeds update_functions dimensions",(int)thing->model);
    }
    if (upcallback != NULL)
    {
        if (upcallback(thing) <= 0) {
            return -1;
        }
    }
    stcallback = NULL;
    if (thing->active_state < sizeof(object_state_functions)/sizeof(object_state_functions[0])) {
        stcallback = object_state_functions[thing->active_state];
    } else {
        ERRORLOG("Object state %d exceeds state_functions dimensions",(int)thing->active_state);
    }
    if (stcallback != NULL)
    {
        SYNCDBG(18,"Updating state");
        if (stcallback(thing) <= 0) {
            return -1;
        }
    }
    SYNCDBG(18,"Updating position");
    thing->movement_flags &= ~TMvF_Unknown01;
    thing->movement_flags &= ~TMvF_Unknown02;
    if ( ((thing->movement_flags & TMvF_Unknown40) == 0) && thing_touching_floor(thing) )
    {
      if ( map_pos_is_lava(thing->mappos.x.stl.num, thing->mappos.y.stl.num) )
      {
        thing->movement_flags |= TMvF_Unknown02;
        objdat = get_objects_data_for_thing(thing);
        if ( (objdat->field_12) && ((thing->field_1 & TF1_Unkn01) == 0) && ((thing->field_0 & 0x80) == 0) )
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
        thing->movement_flags |= TMvF_Unknown01;
      }
    }
    if ((thing->movement_flags & TMvF_Unknown40) != 0)
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
        return INVALID_THING;
    thing->object.gold_stored = game.pot_of_gold_holds;
    return thing;
}

/**
 * Creates a gold hoard object.
 * Note that this function does not create a fully operable object - gold hoard requires room
 * association to be fully functional. This is just a utility sub-function.
 * @param pos Position where the hoard is to be created.
 * @param plyr_idx Player who will own the hoard.
 * @param value The amount of gold to be stored inside the hoard.
 * @return Hoard object thing, which still require to be associated to room.
 */
struct Thing *create_gold_hoard_object(const struct Coord3d *pos, long plyr_idx, long value)
{
    struct Thing *thing;
    long hoard_size_holds,hoard_size,hoard_store;
    hoard_size_holds = 9 * game.pot_of_gold_holds / 5;
    hoard_size = value / hoard_size_holds;
    if (hoard_size >= sizeof(gold_hoard_objects)/sizeof(gold_hoard_objects[0]))
        hoard_size = sizeof(gold_hoard_objects)/sizeof(gold_hoard_objects[0])-1;
    thing = create_object(pos, gold_hoard_objects[hoard_size], plyr_idx, -1);
    if (thing_is_invalid(thing))
        return INVALID_THING;
    hoard_store = 9 * game.pot_of_gold_holds;
    if (hoard_store >= value)
        hoard_store = value;
    thing->long_13 = hoard_store;
    return thing;
}

long remove_gold_from_hoarde(struct Thing *thing, struct Room *room, long amount)
{
    return _DK_remove_gold_from_hoarde(thing, room, amount);
}

TbBool thing_is_gold_hoard(struct Thing *thing)
{
    if (thing->class_id == TCls_Object)
    {
        return (thing->model == 52) || (thing->model == 53) || (thing->model == 54) || (thing->model == 55) || (thing->model == 56);
    }
    return false;
}

struct Thing *find_gold_hoard_at(unsigned short stl_x, unsigned short stl_y)
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
      if (thing_is_gold_hoard(thing))
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

TbBool add_gold_to_pile(struct Thing *thing, long value)
{
    long scaled_val;
    if (thing_is_invalid(thing))
        return false;
    thing->creature.gold_carried += value;
    if (thing->creature.gold_carried < 0)
        thing->creature.gold_carried = LONG_MAX;
    scaled_val = thing->creature.gold_carried / 2 + 150;
    if ((scaled_val > 600) || (thing->creature.gold_carried >= game.gold_pile_maximum))
      scaled_val = 600;
    thing->field_46 = scaled_val;
    return true;
}

struct Thing *create_gold_pile(struct Coord3d *pos, long plyr_idx, long value)
{
    struct Thing *thing;
    thing = create_object(pos, 43, plyr_idx, -1);
    if (thing_is_invalid(thing)) {
        return INVALID_THING;
    }
    thing->creature.gold_carried = 0;
    add_gold_to_pile(thing, value);
    return thing;
}

struct Thing *drop_gold_pile(long value, struct Coord3d *pos)
{
    struct Thing *thing;
    thing = smallest_gold_pile_at_xy(pos->x.stl.num, pos->y.stl.num);
    if (thing_is_invalid(thing)) {
        thing = create_gold_pile(pos, game.neutral_player_num, value);
    } else {
        add_gold_to_pile(thing, value);
    }
    return thing;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
