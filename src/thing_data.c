/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_data.c
 *     Thing struct support functions.
 * @par Purpose:
 *     Functions to maintain thing structure.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_data.h"

#include "globals.h"
#include "bflib_keybrd.h"
#include "bflib_basics.h"
#include "bflib_sound.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "frontend.h"
#include "config_creature.h"
#include "config_effects.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "creature_graphics.h"
#include "game_legacy.h"
#include "engine_arrays.h"
#include "kjm_input.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Thing *allocate_free_thing_structure_f(unsigned char allocflags, const char *func_name)
{
    struct Thing *thing;
    // Get a thing from "free things list"
    long i = game.free_things_start_index;
    // If there is no free thing, try to free an effect
    if (i >= THINGS_COUNT-1)
    {
        if ((allocflags & FTAF_FreeEffectIfNoSlots) != 0)
        {
            thing = thing_get(game.thing_lists[TngList_EffectElems].index);
            if (!thing_is_invalid(thing))
            {
                delete_thing_structure(thing, 0);
            } else
            {
#if (BFDEBUG_LEVEL > 0)
                ERRORMSG("%s: Cannot free up effect element to allocate new thing!",func_name);
#endif
            }
        }
        i = game.free_things_start_index;
    }
    // Now, if there is still no free thing (we couldn't free any)
    if (i >= THINGS_COUNT-1)
    {
#if (BFDEBUG_LEVEL > 0)
        ERRORMSG("%s: Cannot allocate new thing, no free slots!",func_name);
#endif
        return INVALID_THING;
    }
    // And if there is free one, allocate it
    thing = thing_get(game.free_things[i]);
#if (BFDEBUG_LEVEL > 0)
    if (thing_exists(thing)) {
        ERRORMSG("%s: Found existing thing %d in free things list at pos %d!",func_name,(int)game.free_things[i],(int)i);
    }
#endif
    LbMemorySet(thing, 0, sizeof(struct Thing));
    if (thing_is_invalid(thing)) {
        ERRORMSG("%s: Got invalid thing slot instead of free one!",func_name);
        return INVALID_THING;
    }
    thing->alloc_flags |= TAlF_Exists;
    thing->index = game.free_things[i];
    game.free_things[game.free_things_start_index] = 0;
    game.free_things_start_index++;
    TRACE_THING(thing);
    return thing;
}

TbBool i_can_allocate_free_thing_structure(unsigned char allocflags)
{
    // Check if there are free slots
    if (game.free_things_start_index < THINGS_COUNT-1)
        return true;
    // Check if there are effect slots that could be freed
    if ((allocflags & FTAF_FreeEffectIfNoSlots) != 0)
    {
        if (game.thing_lists[TngList_EffectElems].index > 0)
            return true;
    }
    // Couldn't find free slot - fail
    if ((allocflags & FTAF_LogFailures) != 0)
    {
        ERRORLOG("Cannot allocate thing structure.");
        things_stats_debug_dump();
    }
    return false;
}

/**
 * Returns if a thing of given index is in free things list.
 * @param tng_idx Index of the thing to be checked.
 * @return
 */
TbBool is_in_free_things_list(long tng_idx)
{
    for (int i = game.free_things_start_index; i < THINGS_COUNT - 1; i++)
    {
        if (game.free_things[i] == tng_idx)
            return true;
    }
    return false;
}

void delete_thing_structure_f(struct Thing *thing, long a2, const char *func_name)
{
    TRACE_THING(thing);
    if ((thing->alloc_flags & TAlF_InDungeonList) != 0) {
        remove_first_creature(thing);
    }
    if (!a2)
    {
        if (thing->light_id != 0) {
            light_delete_light(thing->light_id);
            thing->light_id = 0;
        }
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (!creature_control_invalid(cctrl))
    {
      if ( !a2 )
      {
          remove_creature_lair(thing);
          if (creature_is_group_member(thing)) {
              remove_creature_from_group(thing);
          }
      }
      delete_control_structure(cctrl);
    }
    if (thing->snd_emitter_id != 0) {
        S3DDestroySoundEmitterAndSamples(thing->snd_emitter_id);
        thing->snd_emitter_id = 0;
    }
    remove_thing_from_its_class_list(thing);
    remove_thing_from_mapwho(thing);
    if (thing->index > 0) {
        game.free_things_start_index--;
        game.free_things[game.free_things_start_index] = thing->index;
    } else {
#if (BFDEBUG_LEVEL > 0)
        ERRORMSG("%s: Performed deleting of thing with bad index %d!",func_name,(int)thing->index);
#endif
    }
    LbMemorySet(thing, 0, sizeof(struct Thing));
}

/**
 * Returns thing of given array index.
 * @param tng_idx
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *thing_get_f(long tng_idx, const char *func_name)
{
    if ((tng_idx > 0) && (tng_idx < THINGS_COUNT)) {
        return game.things.lookup[tng_idx];
    }
    if ((tng_idx < -1) || (tng_idx >= THINGS_COUNT)) {
        ERRORMSG("%s: Request of invalid thing (no %d) intercepted",func_name,(int)tng_idx);
    }
    return INVALID_THING;
}

long thing_get_index(const struct Thing *thing)
{
    long tng_idx = (thing - game.things.lookup[0]);
    if ((tng_idx > 0) && (tng_idx < THINGS_COUNT))
        return tng_idx;
    return 0;
}

short thing_is_invalid(const struct Thing *thing)
{
    return (thing <= game.things.lookup[0]) || (thing > game.things.lookup[THINGS_COUNT-1]) || (thing == NULL);
}

TbBool thing_exists_idx(long tng_idx)
{
    return thing_exists(thing_get(tng_idx));
}

TbBool thing_exists(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if ((thing->alloc_flags & TAlF_Exists) == 0)
        return false;
#if (BFDEBUG_LEVEL > 0)
    if (thing->index != (thing-thing_get(0)))
        WARNLOG("Incorrectly indexed thing (%d) at pos %d",(int)thing->index,(int)(thing-thing_get(0)));
    if ((thing->class_id < 1) || (thing->class_id >= THING_CLASSES_COUNT))
        WARNLOG("Thing %d is of invalid class %d",(int)thing->index,(int)thing->class_id);
#endif
    return true;
}

TbBool thing_is_dragged_or_pulled(const struct Thing *thing)
{
    return ((thing->state_flags & TF1_IsDragged1) != 0) || ((thing->alloc_flags & TAlF_IsDragged) != 0);
}

struct PlayerInfo *get_player_thing_is_controlled_by(const struct Thing *thing)
{
    if ((thing->alloc_flags & TAlF_IsControlled) == 0)
        return INVALID_PLAYER;
    return get_player(thing->owner);
}

void set_thing_draw(struct Thing *thing, long anim, long speed, long scale, char a5, char start_frame, unsigned char draw_class)
{
    unsigned long i;
    thing->anim_sprite = convert_td_iso(anim);
    thing->field_50 &= 0x03;
    thing->field_50 |= (draw_class << 2);
    thing->field_49 = keepersprite_frames(thing->anim_sprite);
    if (speed != -1) {
        thing->anim_speed = speed;
    }
    if (scale != -1) {
        thing->sprite_size = scale;
    }
    if (a5 != -1) {
        set_flag_byte(&thing->field_4F, TF4F_Unknown40, a5);
    }
    if (start_frame == -2)
    {
      i = keepersprite_frames(thing->anim_sprite) - 1;
      thing->field_48 = i;
      thing->field_40 = i << 8;
    } else
    if (start_frame == -1)
    {
      i = CREATURE_RANDOM(thing, thing->field_49);
      thing->field_48 = i;
      thing->field_40 = i << 8;
    } else
    {
      i = start_frame;
      thing->field_48 = i;
      thing->field_40 = i << 8;
    }
}

void query_thing(struct Thing *thing)
{
    struct Thing *querytng;
    if ( (thing->class_id == TCls_Object) && (thing->model == 44) && (!is_key_pressed(KC_LALT, KMod_DONTCARE)) )
    {
        querytng = get_door_for_position(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    }   
    else
    {
        querytng = thing;
    }
    if (!thing_is_invalid(querytng))
    {
        const char title[24];
        const char* name = thing_model_name(querytng);
        const char owner[24]; 
        const char health[24];
        const char position[24];
        const char amount[24] = "\0";
        char output[36];
        sprintf((char*)title, "Thing ID: %d", querytng->index);
        sprintf((char*)owner, "Owner: %d", querytng->owner);
        sprintf((char*)position, "Pos: X:%d Y:%d Z:%d", querytng->mappos.x.stl.num, querytng->mappos.y.stl.num, querytng->mappos.z.stl.num);
        if (querytng->class_id == TCls_Trap)
        {
            struct ManfctrConfig *mconf = &gameadd.traps_config[querytng->model];
            sprintf((char*)health, "Shots: %d/%d", querytng->trap.num_shots, mconf->shots);
        }
        else
        {
            if (querytng->class_id == TCls_Object)
            {
                if (object_is_gold(querytng))
                {
                    sprintf((char*)amount, "Amount: %ld", querytng->valuable.gold_stored);   
                }
            }  
            sprintf((char*)health, "Health: %d", querytng->health);
            if (querytng->class_id == TCls_Door)
            {
                sprintf(output, "%s/%ld", health, door_stats[querytng->model][0].health);
            }
            else if (querytng->class_id == TCls_Object)
            {
                if (querytng->model == 5)
                {
                    sprintf(output, "%s/%ld", health, game.dungeon_heart_health);
                }
            }
        }
        create_message_box((const char*)&title, name, (const char*)&owner, (const char*)&health, (const char*)&position, (const char*)&amount);
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
