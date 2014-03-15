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
#include "bflib_basics.h"
#include "bflib_sound.h"
#include "bflib_memory.h"
#include "config_creature.h"
#include "config_effects.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_delete_thing_structure(struct Thing *creatng, long a2);
DLLIMPORT struct Thing *_DK_allocate_free_thing_structure(unsigned char a1);
DLLIMPORT unsigned char _DK_i_can_allocate_free_thing_structure(unsigned char a1);
DLLIMPORT unsigned char _DK_creature_remove_lair_from_room(struct Thing *creatng, struct Room *room);

/******************************************************************************/
struct Thing *allocate_free_thing_structure_f(unsigned char allocflags, const char *func_name)
{
    struct Thing *thing;
    long i;
    //return _DK_allocate_free_thing_structure(allocflags);

    // Get a thing from "free things list"
    i = game.free_things_start_index;
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
    //return _DK_i_can_allocate_free_thing_structure(allocflags);
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
    int i;
    for (i=game.free_things_start_index; i < THINGS_COUNT-1; i++)
    {
        if (game.free_things[i] == tng_idx)
            return true;
    }
    return false;
}

TbBool delete_lair_totem(struct Thing *lairtng)
{
    struct Thing *creatng;
    creatng = thing_get(lairtng->word_13);
    if (!thing_is_creature(creatng)) {
        ERRORLOG("No totem owner");
        return false;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->lair_room_id = 0;
    cctrl->lairtng_idx = 0;
    delete_thing_structure(lairtng, 0);
    return true;
}

TbBool creature_remove_lair_from_room(struct Thing *creatng, struct Room *room)
{
    //return _DK_creature_remove_lair_from_room(creatng, room);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->lair_room_id != room->index)
    {
        ERRORLOG("Attempt to remove a lair which belongs to %s index %d from room index %d he didn't think he was in",thing_model_name(creatng),(int)creatng->index,(int)room->index);
        return false;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    TbBool result;
    result = true;
    // Remove lair from room capacity
    if (room->content_per_model[creatng->model] <= 0)
    {
        ERRORLOG("Attempt to remove a lair which belongs to %s index %d from room index %d not containing this creature model",thing_model_name(creatng),(int)creatng->index,(int)room->index);
        result = false;
    } else
    if ( room->used_capacity < crstat->lair_size)
    {
        ERRORLOG("Attempt to remove creature lair from room with too little used space");
        result = false;
    } else
    {
        room->used_capacity -= crstat->lair_size;
        room->content_per_model[creatng->model]--;
    }
    cctrl->lair_room_id = 0;
    //Remove the totem thing
    if (cctrl->lairtng_idx > 0)
    {
        struct Thing *lairtng;
        lairtng = thing_get(cctrl->lairtng_idx);
        TRACE_THING(lairtng);
        create_effect(&lairtng->mappos, imp_spangle_effects[creatng->owner], creatng->owner);
        delete_lair_totem(lairtng);
    }
    return result;
}

void delete_thing_structure_f(struct Thing *thing, long a2, const char *func_name)
{
    //_DK_delete_thing_structure(thing, a2); return;
    TRACE_THING(thing);
    if ((thing->alloc_flags & TAlF_Unkn08) != 0) {
        remove_first_creature(thing);
    }
    if (!a2)
    {
        if (thing->light_id != 0) {
            light_delete_light(thing->light_id);
            thing->light_id = 0;
        }
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
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
    long tng_idx;
    tng_idx = (thing - game.things.lookup[0]);
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
    return ((thing->field_1 & TF1_IsDragged1) != 0) || ((thing->alloc_flags & TAlF_IsDragged) != 0);
}

struct PlayerInfo *get_player_thing_is_controlled_by(const struct Thing *thing)
{
    if ((thing->alloc_flags & TAlF_IsControlled) == 0)
        return INVALID_PLAYER;
    return get_player(thing->owner);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
