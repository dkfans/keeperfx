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
#include "pre_inc.h"
#include "thing_list.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "globals.h"
#include "bflib_sound.h"
#include "packets.h"
#include "light_data.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_traps.h"
#include "thing_shots.h"
#include "thing_corpses.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_creature.h"
#include "thing_navigate.h"
#include "creature_senses.h"
#include "spdigger_stack.h"
#include "power_hand.h"
#include "magic_powers.h"
#include "map_utils.h"
#include "config_creature.h"
#include "config_magic.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "player_instances.h"
#include "engine_camera.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "bflib_planar.h"
#include "post_inc.h"

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

const struct NamedCommand class_commands[] = {
  {"Object",        TCls_Object},
  {"Shot",          TCls_Shot},
  {"EffectElem",    TCls_EffectElem},
  {"DeadCreature",  TCls_DeadCreature},
  {"Creature",      TCls_Creature},
  {"Effect",        TCls_Effect},
  {"EffectGen",     TCls_EffectGen},
  {"Trap",          TCls_Trap},
  {"Door",          TCls_Door},
  {"AmbientSnd",    TCls_AmbientSnd},
  {"CaveIn",        TCls_CaveIn},
  {NULL,              0},
  };

/******************************************************************************/

void set_previous_thing_position(struct Thing *thing) {
    thing->previous_mappos = thing->mappos;
    thing->previous_floor_height = thing->floor_height;
}

/**
 * Adds thing at beginning of a StructureList.
 * @param thing
 * @param list
 */
void add_thing_to_list(struct Thing *thing, struct StructureList *list)
{
    if ((thing->alloc_flags & TAlF_IsInStrucList) != 0)
    {
        ERRORLOG("Thing is already in list");
        return;
    }
    struct Thing* prevtng = INVALID_THING;
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
    struct StructureList* slist = get_list_for_thing_class(thing->class_id);
    if (slist != NULL) {
        remove_thing_from_list(thing, slist);
    }
}

ThingIndex get_thing_class_list_head(ThingClass class_id)
{
    struct StructureList* slist = get_list_for_thing_class(class_id);
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
    struct StructureList* slist = get_list_for_thing_class(thing->class_id);
    if (slist != NULL)
        add_thing_to_list(thing, slist);
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_map_block_thing_filter_call_bool_filter(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                Thing_Bool_Filter matcher_cb = (Thing_Bool_Filter)param->tertiary_pointer;
                if ((matcher_cb != NULL) && matcher_cb(thing))
                {
                    // Prepare reference Coord3d struct for distance computation
                    struct Coord3d refpos;
                    refpos.x.val = param->primary_number;
                    refpos.y.val = param->secondary_number;
                    refpos.z.val = 0;
                    // This function should return max value when the distance is minimal, so:
                    return INT32_MAX-get_2d_distance(&thing->mappos, &refpos);
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}


/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_thing_pos_thing_filter_is_enemy_which_can_be_shot_by_trap(const struct Thing* thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                struct Thing* traptng = thing_get(param->primary_number);
                if (players_are_enemies(traptng->owner, thing->owner) || is_neutral_thing(traptng))
                {
                    if (!creature_is_being_unconscious(thing) && !thing_is_dragged_or_pulled(thing) && !thing_is_picked_up(thing)
                        && !creature_is_kept_in_custody_by_enemy(thing) && !creature_is_dying(thing) && !creature_is_leaving_and_cannot_be_stopped(thing)
                        && ((get_creature_model_flags(thing) & CMF_IsSpectator) == 0))
                    {
                        MapCoordDelta distance = get_2d_distance(&thing->mappos, &traptng->mappos);
                        MapCoordDelta max_range = param->secondary_number;
                        if ((distance <= max_range) || (max_range <= 0))
                        {
                            if (line_of_sight_2d(&traptng->mappos, &thing->mappos))
                            {
                                if (creature_is_invisible(thing))
                                {
                                    struct TrapConfigStats *trapst = get_trap_model_stats(traptng->model);
                                    if (trapst->detect_invisible == 0)
                                    {
                                        return -1;
                                    }
                                }
                                // This function should return max value when the distance is minimal.
                                return INT32_MAX - distance;
                            }
                        }
                    }
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_thing_pos_thing_filter_is_enemy_which_can_be_attacked_by_creature(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                struct Thing* creatng = thing_get(param->primary_number);
                if (players_are_enemies(creatng->owner, thing->owner))
                {
                    if (creature_will_attack_creature(creatng, thing))
                    {
                        // This function should return max value when the distance is minimal, so:
                        return INT32_MAX-get_2d_distance(&thing->mappos, &creatng->mappos);
                    }
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_thing_pos_thing_filter_is_enemy_object_which_can_be_attacked_by_creature(const struct Thing* objtng, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (objtng->class_id == param->class_id))
    {
        if (thing_matches_model(objtng, param->model_id))
        {
            if ((param->plyr_idx == -1) || (objtng->owner == param->plyr_idx))
            {
                struct Thing* creatng = thing_get(param->primary_number);
                if (players_are_enemies(creatng->owner, objtng->owner))
                {
                    MapCoordDelta distance = get_2d_distance(&creatng->mappos, &objtng->mappos);
                    //Just dungeon hearts now. Todo: expand with other types of destructible objects
                    if (thing_is_dungeon_heart(objtng) && creature_can_have_combat_with_creature(creatng, (struct Thing*)objtng, distance,1,0) && !(get_creature_model_flags(creatng) & CMF_NoEnmHeartAttack))
                    {
                        return INT32_MAX - distance;
                    }
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long highest_score_thing_filter_is_enemy_within_distance_which_can_be_attacked_by_creature(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (thing_matches_model(thing,param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                struct Thing* creatng = thing_get(param->primary_number);
                if (creature_will_attack_creature(creatng, thing) && !creature_has_creature_in_combat(creatng, thing))
                {
                    long distance = get_combat_distance(creatng, thing);
                    if (distance >= param->secondary_number) {
                        return -1;
                    }
                    CrAttackType attack_type = creature_can_have_combat_with_creature(creatng, (struct Thing*)thing, distance, param->tertiary_number, 0);
                    if (attack_type > AttckT_Unset)
                    {
                        long score = get_combat_score(creatng, thing, attack_type, distance);
                        return score;
                    }
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long highest_score_thing_filter_is_enemy_object_within_distance_which_can_be_attacked_by_creature(const struct Thing* thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                struct Thing* objtng = thing_get(param->primary_number);
                if (trap_is_valid_combat_target_for_creature(objtng, thing))
                {
                    long distance = get_combat_distance(objtng, thing);
                    if (distance >= param->secondary_number) {
                        return -1;
                    }
                    CrAttackType attack_type = creature_can_have_combat_with_object(objtng, (struct Thing*)thing, distance, param->tertiary_number, 0);
                    if (attack_type > AttckT_Unset)
                    {
                        long score = get_combat_score(objtng, thing, attack_type, distance);
                        return score;
                    }
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_map_block_thing_filter_is_enemy_of_able_to_attack_and_not_specdigger(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((thing->class_id == TCls_Creature) && players_are_enemies(param->plyr_idx, thing->owner))
    {
        if (!creature_is_being_unconscious(thing) && !thing_is_picked_up(thing) && !creature_is_kept_in_custody_by_enemy(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            if ((get_creature_model_flags(thing) & (CMF_IsSpecDigger|CMF_IsDiggingCreature)) == 0)
            {
                // Prepare reference Coord3d struct for distance computation
                struct Coord3d refpos;
                refpos.x.val = param->primary_number;
                refpos.y.val = param->secondary_number;
                refpos.z.val = 0;
                // This function should return max value when the distance is minimal, so:
                return INT32_MAX-get_2d_distance(&thing->mappos, &refpos);
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function. Random around AP
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_map_block_creature_filter_diagonal_random(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == TCls_Creature)
    {
        if (creature_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == ALL_PLAYERS) || (thing->owner == param->plyr_idx))
            {
                if (!thing_is_picked_up(thing))
                {
                    MapCoordDelta dist = get_distance_xy(thing->mappos.x.val, thing->mappos.y.val, param->primary_number, param->secondary_number);
                    if (dist > param->tertiary_number) // Too far away
                        return -1;
                    // It is not "correct" randomness (pick random N from list) but rolling a dice on each creature found
                    uint32_t tmp = maximizer + dist + 1;
                    return (long)LbRandomSeries(INT32_MAX, &tmp, __func__, __LINE__);
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_map_block_thing_filter_is_thing_of_class_and_model_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                if (!thing_is_picked_up(thing))
                {
                    // Prepare reference Coord3d struct for distance computation
                    struct Coord3d refpos;
                    refpos.x.val = param->primary_number;
                    refpos.y.val = param->secondary_number;
                    refpos.z.val = 0;
                    MapCoordDelta dist = get_2d_distance(&thing->mappos, &refpos);
                    if (dist > param->tertiary_number) // Too far away
                        return -1;
                    // This function should return max value when the distance is minimal, so:
                    return INT32_MAX-dist;
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_map_block_thing_filter_can_be_keeper_power_target(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (can_cast_power_on_thing(param->plyr_idx, thing, param->tertiary_number))
    {
        // Prepare reference Coord3d struct for distance computation
        struct Coord3d refpos;
        refpos.x.val = param->primary_number;
        refpos.y.val = param->secondary_number;
        refpos.z.val = 0;
        // This function should return max value when the distance is minimal, so:
        return INT32_MAX-get_2d_distance(&thing->mappos, &refpos);
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

long creature_near_filter_is_owned_by(const struct Thing *thing, FilterParam plyr_idx)
{
    if (thing->owner == plyr_idx)
    {
        return true;
    }
    if (creature_is_kept_in_custody(thing))
    {
        struct SlabMap* slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (slabmap_owner(slb) == plyr_idx)
            return true;
    }
    return false;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_map_block_thing_filter_is_slappable(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if (!thing_is_picked_up(thing) && thing_slappable(thing, param->plyr_idx))
        {
            // Prepare reference Coord3d struct for distance computation
            struct Coord3d refpos;
            refpos.x.val = param->primary_number;
            refpos.y.val = param->secondary_number;
            refpos.z.val = 0;
            // This function should return max value when the distance is minimal, so:
            return INT32_MAX-get_2d_distance(&thing->mappos, &refpos);
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long near_map_block_thing_filter_is_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == param->class_id)
    {
        switch(param->class_id)
        {
        case TCls_Creature:
            if ((param->plyr_idx == -1) || creature_near_filter_is_owned_by(thing, param->plyr_idx))
            {
                // Prepare reference Coord3d struct for distance computation
                struct Coord3d refpos;
                refpos.x.val = param->primary_number;
                refpos.y.val = param->secondary_number;
                refpos.z.val = 0;
                // This function should return max value when the distance is minimal, so:
                return INT32_MAX-get_2d_distance(&thing->mappos, &refpos);
            }
            break;
        default:
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                // Prepare reference Coord3d struct for distance computation
                struct Coord3d refpos;
                refpos.x.val = param->primary_number;
                refpos.y.val = param->secondary_number;
                refpos.z.val = 0;
                // This function should return max value when the distance is minimal, so:
                return INT32_MAX-get_2d_distance(&thing->mappos, &refpos);
            }
            break;
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_is_of_class_and_model_and_owned_by_or_allied_with(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == param->class_id)
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || players_are_mutual_allies(thing->owner,param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return INT32_MAX;
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_is_of_class_and_model_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == param->class_id)
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return INT32_MAX;
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long in_action_point_thing_filter_is_of_class_and_model_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == param->class_id)
    {
        if (thing_matches_model(thing, param->model_id))
        {
            if (!thing_is_in_limbo(thing))
            {
                if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
                {
                    struct Coord3d refpos;
                    refpos.x.val = param->primary_number;
                    refpos.y.val = param->secondary_number;
                    refpos.z.val = 0;
                    MapCoordDelta dist = get_2d_distance(&thing->mappos, &refpos);
                    if (dist <= param->tertiary_number) {
                        // Return the largest value to stop sweeping
                        return INT32_MAX;
                    }
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_is_food_available_to_eat_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == TCls_Object)
    {
        if (object_is_mature_food(thing))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return INT32_MAX;
            }
        }
    }
    if (thing->class_id == TCls_Creature)
    {
        if (creature_under_spell_effect(thing, CSAfF_Chicken) && (thing->health > 0))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                // Return the largest value to stop sweeping
                return INT32_MAX;
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_is_creature_of_model_training_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == TCls_Creature)
    {
      if (creature_matches_model(thing, param->model_id))
      {
          if ((thing->owner == param->plyr_idx) || (param->plyr_idx == -1))
          {
              if (((int)thing->index != param->primary_number) || (param->primary_number == -1))
              {
                  struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
                  if ((thing->active_state == CrSt_Training) && (cctrl->training.mode > 1))
                  {
                      // Return the largest value to stop sweeping
                      return INT32_MAX;
                  }
              }
          }
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * returns if a specific creature model matches a target model, considering all the wildcards.
 * @param creatng_model The creature model being checked.
 * @param plyr_idx The player id so we can exclude diggers and spectators.
 * @param target_model model to compare it to, possible wildcard.
 */
TbBool creature_model_matches_model(ThingModel creatng_model, PlayerNumber plyr_idx, ThingModel target_model)
{
    if (!is_creature_model_wildcard(target_model))
        return target_model == creatng_model;
    else if (target_model == CREATURE_ANY)
        return (creatng_model != get_players_spectator_model(plyr_idx)); //we can assume we do not want to cast on the floating spirit
    else if (target_model == CREATURE_NONE)
        return false;
    if (target_model == CREATURE_DIGGER)
        return creature_kind_is_for_dungeon_diggers_list(plyr_idx, creatng_model);
    else if (target_model == CREATURE_NOT_A_DIGGER)
        return ((!creature_kind_is_for_dungeon_diggers_list(plyr_idx, creatng_model)) && (creatng_model != get_players_spectator_model(plyr_idx)));
    else
        ERRORLOG("Invalid model wildcard detected: %d", target_model);
    return false;
}

/**
 * returns if the creature thing matches the model from the filter, considering possible wildcards.
 * @param creatng The creature being checked.
 * @param crmodel model to compare it to, possible wildcard.
 */
TbBool creature_matches_model(const struct Thing* creatng, ThingModel crmodel)
{
    if (creatng->class_id != TCls_Creature)
    {
        return false;
    }
    return creature_model_matches_model(creatng->model, creatng->owner, crmodel);
}

/**
 * returns if the thing matches the model from the filter, considering possible wildcards.
 * @param thing The thing being checked.
 * @param crmodel model to compare it to, possible wildcard.
  */
TbBool thing_matches_model(const struct Thing* thing, long tngmodel)
{
    if (thing->class_id == TCls_Creature)
    {
        return creature_matches_model(thing, tngmodel);
    }
    else if ((tngmodel == -1) || (thing->model == tngmodel))
    {
        return true;
    }
    return false;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_call_bool_filter(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
        if(thing_matches_model(thing, param->model_id))
        {
            if ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
            {
                Thing_Bool_Filter matcher_cb = (Thing_Bool_Filter)param->tertiary_pointer;
                if ((matcher_cb != NULL) && matcher_cb(thing))
                {
                    return INT32_MAX;
                }
            }
        }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_is_trap_of_model_armed_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == TCls_Trap)
    {
      if (thing_matches_model(thing, param->model_id))
      {
          if ((thing->owner == param->plyr_idx) || (param->plyr_idx == -1))
          {
              if ((param->primary_number && (thing->trap.num_shots != 0))
              || (!param->primary_number && (thing->trap.num_shots == 0)))
              {
                  // Return the largest value to stop sweeping
                  return INT32_MAX;
              }
          }
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_is_door_of_model_locked_and_owned_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if (thing->class_id == TCls_Door)
    {
      if (thing_matches_model(thing,param->model_id))
      {
          if ((thing->owner == param->plyr_idx) || (param->plyr_idx == -1))
          {
              if ((param->primary_number && (thing->door.is_locked))
              || (!param->primary_number && (!thing->door.is_locked)))
              {
                  // Return the largest value to stop sweeping
                  return INT32_MAX;
              }
          }
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function.
 * @param thing The thing being checked.
 * @param param Parameters exchanged between filter calls.
 * @param maximizer Previous value which made a thing pass the filter.
 */
long anywhere_thing_filter_is_gold_on_owned_ground_pickable_by(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ((param->class_id == -1) || (thing->class_id == param->class_id))
    {
      if (((param->model_id == -1) && object_is_gold_pile(thing)) || (thing->model == param->model_id))
      {
          if (!thing_is_dragged_or_pulled(thing))
          {
              if ((param->plyr_idx == -1) || ((get_slab_owner_thing_is_on(thing) == param->plyr_idx) && can_thing_be_picked_up_by_player(thing, param->plyr_idx)))
              {
                  // Return the largest value to stop sweeping
                  return INT32_MAX;
              }
          }
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

TbBool delete_if_dead_creature(struct Thing *thing)
{
    if (thing->class_id == TCls_DeadCreature) {
        delete_thing_structure(thing, 0);
        return true;
    }
    return false;
}

TngUpdateRet switch_object_on_destoyed_slab_to_new_owner(struct Thing *thing, ModTngFilterParam param)
{
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    if (thing_is_picked_up(thing) || thing_is_dragged_or_pulled(thing))
    {
        return TUFRet_Unchanged;
    }
    if (thing_is_object(thing))
    {
        if (object_is_gold_pile(thing) && (thing->owner != param->primary_number))
        {
            change_object_owner(thing, param->primary_number);
            return TUFRet_Modified;
        }
    }
    return TUFRet_Unchanged;
}

/**
 * Makes per game turn update of all things in given StructureList.
 * @param list List of things to process.
 */
void update_things_in_list(struct StructureList *list)
{
    SYNCDBG(18,"Starting");
    unsigned long k = 0;
    int i = list->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
      }
      i = thing->next_of_class;
      // Per-thing code
      if ((thing->alloc_flags & TAlF_IsFollowingLeader) == 0)
      {
          if ((thing->alloc_flags & TAlF_IsInLimbo) != 0) {
              update_thing_animation(thing);
          } else {
              update_thing(thing);
          }
      }
      set_previous_thing_position(thing);
      // Per-thing code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    SYNCDBG(19,"Finished, %d items",(int)k);
}

/**
 * Makes per game turn update of cave in things, using proper StructureList.
 * @return Returns amount of cave in things in list.
 */
unsigned long update_cave_in_things(void)
{
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_CaveIn);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
    SYNCDBG(18,"Starting");
    unsigned long k = 0;
    int i = list->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
  SYNCDBG(18,"Starting");
  unsigned long k = 0;
  int i = game.thing_lists[TngList_Creatures].index;
  while (i != 0)
  {
      struct Thing* thing = thing_get(i);
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
    if ((thing->alloc_flags & TAlF_IsFollowingLeader) != 0)
    {
      if ((thing->alloc_flags & TAlF_IsInLimbo) != 0) {
        update_thing_animation(thing);
      } else {
        update_thing(thing);
      }
    }
    set_previous_thing_position(thing);
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
    optimised_lights = 0;
    total_lights = 0;
    do_lights = game.lish.light_enabled;
    update_things_in_list(&game.thing_lists[TngList_Creatures]);
    update_creatures_not_in_list();
    update_things_in_list(&game.thing_lists[TngList_Traps]);
    update_things_in_list(&game.thing_lists[TngList_Shots]);
    update_things_in_list(&game.thing_lists[TngList_Objects]);
    update_things_in_list(&game.thing_lists[TngList_Effects]);
    update_things_in_list(&game.thing_lists[TngList_EffectElems]);
    update_things_in_list(&game.thing_lists[TngList_DeadCreatrs]);
    update_things_in_list(&game.thing_lists[TngList_EffectGens]);
    update_things_in_list(&game.thing_lists[TngList_Doors]);
    update_things_sounds_in_list(&game.thing_lists[TngList_AmbientSnds]);
    update_cave_in_things();
    game.map_changed_for_nagivation = 0;
    SYNCDBG(9,"Finished");
}

struct Thing *find_players_dungeon_heart(PlayerNumber plyridx)
{
    int k = 0;
    int i = game.thing_lists[TngList_Objects].index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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

struct Thing* find_players_backup_dungeon_heart(PlayerNumber plyridx)
{
    struct Dungeon* dungeon = get_dungeon(plyridx);
    int k = 0;
    int i = game.thing_lists[TngList_Objects].index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_dungeon_heart(thing) && (thing->owner == plyridx) && (thing->index != dungeon->dnheart_idx))
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
    SYNCDBG(6, "No secondary heart for player %d", (int)plyridx);
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
    struct Thing* thing = find_players_dungeon_heart(player->id_number);
    struct Dungeon* dungeon = get_players_dungeon(player);
    if (dungeon_invalid(dungeon)) {
        WARNLOG("Tried to init player %d which has no dungeon",(int)player->id_number);
        return;
    }
    if (thing_exists(thing))
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
            dungeon->mappos.x.val = subtile_coord_center(game.map_subtiles_x/2);
            dungeon->mappos.y.val = subtile_coord_center(game.map_subtiles_y/2);
            dungeon->mappos.z.val = subtile_coord_center(map_subtiles_z/2);
        }
    }

    dungeon->backup_heart_idx = 0;
    struct Thing* scndthing = find_players_backup_dungeon_heart(player->id_number);
    {
        if (thing_exists(scndthing))
        {
            dungeon->backup_heart_idx = scndthing->index;
        }
    }
}

TbBool script_support_setup_player_as_zombie_keeper(PlayerNumber plyr_idx)
{
    SYNCDBG(8,"Starting for player %d",(int)plyr_idx);
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player)) {
        SCRPTWRNLOG("Tried to set up invalid player %d",(int)plyr_idx);
        return false;
    }
    player->allocflags &= ~PlaF_Allocated; // mark as non-existing
    player->id_number = plyr_idx;
    player->is_active = 0;
    player->allocflags &= ~PlaF_CompCtrl;
    init_player_start(player, false);
    return true;
}

void setup_computer_player(int plr_idx)
{
    SYNCDBG(5,"Starting for player %d",plr_idx);
    struct Thing* thing = find_players_dungeon_heart(plr_idx); // cannot use player->id_number, as it isn't set yet
    if (thing_exists(thing))
    {
        script_support_setup_player_as_computer_keeper(plr_idx, 0);
    } else
    {
        script_support_setup_player_as_zombie_keeper(plr_idx);
    }
}

void setup_computer_players(void)
{
    for (int plr_idx = 0; plr_idx < PLAYERS_COUNT; plr_idx++)
    {
        struct PlayerInfo* player = get_player(plr_idx);
        if (!player_exists(player))
        {
          setup_computer_player(plr_idx);
        }
    }
}

void setup_zombie_players(void)
{
    for (int plr_idx = 0; plr_idx < PLAYERS_COUNT; plr_idx++)
    {
        struct PlayerInfo* player = get_player(plr_idx);
        if (!player_exists(player))
        {
            script_support_setup_player_as_zombie_keeper(plr_idx);
        }
  }
}

void init_all_creature_states(void)
{
    int k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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

void init_creature_states_for_player(PlayerNumber plyr_idx)
{
    int k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing->owner == plyr_idx)
        {
            init_creature_state(thing);
        }
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
    struct Thing *mwtng;
    SYNCDBG(18,"Starting");
    if ((thing->alloc_flags & TAlF_IsInMapWho) == 0)
        return;
    if (thing->prev_on_mapblk > 0)
    {
        mwtng = thing_get(thing->prev_on_mapblk);
        if (thing_exists(mwtng)) {
            mwtng->next_on_mapblk = thing->next_on_mapblk;
        } else {
            ERRORLOG("Non-existing thing index %d in mapwho before index %d!",(int)thing->prev_on_mapblk,(int)thing->index);
            thing->prev_on_mapblk = 0;
        }
    } else
    {
        struct Map* mapblk = get_map_block_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (get_mapwho_thing_index(mapblk) != thing->index)
        {
            WARNLOG("Moving lost %s %d from %d, %d", thing_class_and_model_name(thing->class_id, thing->model),
                    thing->index, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        }
        set_mapwho_thing_index(mapblk, thing->next_on_mapblk);
    }
    if (thing->next_on_mapblk > 0)
    {
        mwtng = thing_get(thing->next_on_mapblk);
        if (thing_exists(mwtng)) {
            mwtng->prev_on_mapblk = thing->prev_on_mapblk;
        } else {
            ERRORLOG("Non-existing thing index %d in mapwho after index %d!",(int)thing->next_on_mapblk,(int)thing->index);
            thing->next_on_mapblk = 0;
        }
    }
    thing->next_on_mapblk = 0;
    thing->prev_on_mapblk = 0;
    thing->alloc_flags &= ~TAlF_IsInMapWho;
}

void place_thing_in_mapwho(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    if ((thing->alloc_flags & TAlF_IsInMapWho) != 0)
        return;
    struct Map* mapblk = get_map_block_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    thing->next_on_mapblk = get_mapwho_thing_index(mapblk);
    if (thing->next_on_mapblk > 0)
    {
        struct Thing* mwtng = thing_get(thing->next_on_mapblk);
        if (thing_exists(mwtng)) {
            mwtng->prev_on_mapblk = thing->index;
        } else {
            ERRORLOG("Non-existing thing index %d in mapwho!",(int)thing->next_on_mapblk);
            thing->next_on_mapblk = 0;
        }
    }
    set_mapwho_thing_index(mapblk, thing->index);
    thing->prev_on_mapblk = 0;
    thing->alloc_flags |= TAlF_IsInMapWho;
}

struct Thing *find_base_thing_on_mapwho(ThingClass oclass, ThingModel model, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return INVALID_THING;
}

/**
 * Checks the subtiles for a thing. Only considers objects, checks against genre, accepts genre 0 for any.
 * @return Returns INVALID_THING, or a thing of class 'object' of the matching genre.
 */
struct Thing* find_object_of_genre_on_mapwho(long genre, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    struct ObjectConfigStats* objst;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing->class_id == TCls_Object)
        {
            objst = get_object_model_stats(thing->model);
            if ((objst->genre == genre) || (genre == 0)) {
                return thing;
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
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
    long i = game.thing_lists[TngList_Objects].index;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
      }
      i = thing->next_of_class;
      // Per-thing code
      if ((object_is_hero_gate(thing)) && (thing->hero_gate.number == num))
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
 * Returns a creatures lair totem from given subtile.
 */
struct Thing *find_creature_lair_totem_at_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, ThingModel crmodel)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    long i = get_mapwho_thing_index(mapblk);
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per-thing code
        if (thing->class_id == TCls_Object)
        {
            struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
            if (objst->related_creatr_model > 0)
            {
                if ((crmodel == 0) || (crmodel == CREATURE_ANY) || (objst->related_creatr_model == crmodel))
                    return thing;
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return INVALID_THING;
}

struct Thing *find_random_thing_in_room(ThingClass tngclass, ThingModel tngmodel,struct Room *room)
{

    if ( room_is_invalid(room) )
    {
        ERRORLOG("Invalid Room passed");
        return INVALID_THING;
    }

    if ( room->slabs_count == 0)
    {
        return INVALID_THING;
    }

    SlabCodedCoords current_slb = room->slabs_list;
    unsigned int current_slab_idx = GAME_RANDOM(room->slabs_count);

    for (size_t i = 0; i < current_slab_idx; i++)
    {
        current_slb = get_next_slab_number_in_room(current_slb);
    }

    static const int STL_PER_SLB_2D = STL_PER_SLB * STL_PER_SLB;

    for (size_t i = 0; i < room->slabs_count; i++)
    {
        if ( room->slabs_count == current_slab_idx )
        {
            current_slab_idx = 0;
            current_slb = room->slabs_list;
        }

        MapSlabCoord slb_x = slb_num_decode_x(current_slb);
        MapSlabCoord slb_y = slb_num_decode_y(current_slb);

        unsigned char subtile = GAME_RANDOM(STL_PER_SLB_2D);

        for (size_t j = 0; j < STL_PER_SLB_2D; j++)
        {
            MapSubtlCoord stl_x = STL_PER_SLB * (slb_x) + subtile % STL_PER_SLB;
            MapSubtlCoord stl_y = STL_PER_SLB * (slb_y) + subtile / STL_PER_SLB;

            struct Thing *thing = find_base_thing_on_mapwho(tngclass,tngmodel,stl_x,stl_y);

            if ( !thing_is_invalid(thing) )
                return thing;
            subtile = (subtile + 1) % STL_PER_SLB_2D;
        }

        current_slb = get_next_slab_number_in_room(current_slb);
        ++current_slab_idx;
    }
    return INVALID_THING;
}

/**
 * Counts things best matching given filter.
 * Only things for which filter function returns max value are counted.
 * @param filter Filter function reference.
 * @param param Filter function parameters struct.
 * @return Count of best matched things.
 */
long count_things_of_class_with_filter(Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    long maximizer = 0;
    long match_count = 0;
    SYNCDBG(19,"Starting");
    const struct StructureList* slist = get_list_for_thing_class(param->class_id);
    if (slist == NULL) {
        return 0;
    }
    long i = slist->index;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        long n = filter(thing, param, maximizer);
        if (n > maximizer)
        {
            maximizer = n;
            match_count = 1;
        } else
        if (n == maximizer)
        {
            match_count++;
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return match_count;
}

/**
 * Out of things best matching given filter, returns the one of given index.
 * Only things for which filter function returns max value are counted.
 * If the amount of such things is lower than tngindex, then last matching thing is returned.
 * @param filter Filter function reference.
 * @param param Filter function parameters struct.
 * @param tngindex Best matched thing index to be returned.
 * @return
 */
struct Thing *get_nth_thing_of_class_with_filter(Thing_Maximizer_Filter filter, MaxTngFilterParam param, long tngindex)
{
    long maximizer = 0;
    long curindex = 0;
    struct Thing* retng = INVALID_THING;
    SYNCDBG(19,"Starting");
    struct StructureList* slist = get_list_for_thing_class(param->class_id);
    if (slist == NULL) {
        return INVALID_THING;
    }
    long i = slist->index;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        long n = filter(thing, param, maximizer);
        if (n > maximizer)
        {
            retng = thing;
            maximizer = n;
            curindex = 0;
        } else
        if (n == maximizer)
        {
            if (curindex <= tngindex) {
                retng = thing;
            }
            // Only break if we can't get any higher with the filter function result
            if ((maximizer == INT32_MAX) && (curindex >= tngindex)) {
                break;
            }
            curindex++;
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

struct Thing *get_random_thing_of_class_with_filter(Thing_Maximizer_Filter filter, MaxTngFilterParam param, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    long match_count = count_things_of_class_with_filter(filter, param);
    if (match_count < 1) {
        return INVALID_THING;
    }
    return get_nth_thing_of_class_with_filter(filter, param, PLAYER_RANDOM(plyr_idx, match_count));
}

long do_to_all_things_of_class_and_model(int tngclass, int tngmodel, Thing_Bool_Modifier do_cb)
{
    SYNCDBG(19,"Starting");
    struct StructureList* slist = get_list_for_thing_class(tngclass);
    if (slist == NULL) {
        return 0;
    }
    long n = 0;
    long i = slist->index;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing->model == tngmodel)
        {
            if (do_cb(thing))
                n++;
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}

/** Finds on whole map a thing owned by given player, which matches given criteria.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose things will be searched. Allies are not included, use -1 to select all.
 * @return The target thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_nearest_thing_of_class_and_model_owned_by(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, int tngclass, int tngmodel)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = near_map_block_thing_filter_is_thing_of_class_and_model_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = tngclass;
    param.model_id = tngmodel;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    param.tertiary_number = INT32_MAX;
    return get_nth_thing_of_class_with_filter(filter, &param, 0);
}

struct Thing* get_nearest_enemy_creature_in_sight_and_range_of_trap(struct Thing* traptng)
{
    struct TrapConfigStats *trapst = get_trap_model_stats(traptng->model);
    struct ShotConfigStats* shotst = get_shot_model_stats(trapst->created_itm_model);

    SYNCDBG(19, "Starting");
    Thing_Maximizer_Filter filter = near_thing_pos_thing_filter_is_enemy_which_can_be_shot_by_trap;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = CREATURE_ANY;
    param.plyr_idx = -1;
    param.primary_number = traptng->index;
    param.secondary_number = shotst->max_range;
    param.tertiary_number = -1;
    return get_nth_thing_of_class_with_filter(filter, &param, 0);
}

struct Thing *get_nearest_enemy_creature_possible_to_attack_by(struct Thing *creatng)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = near_thing_pos_thing_filter_is_enemy_which_can_be_attacked_by_creature;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = CREATURE_ANY;
    param.plyr_idx = -1;
    param.primary_number = creatng->index;
    param.secondary_number = -1;
    param.tertiary_number = -1;
    return get_nth_thing_of_class_with_filter(filter, &param, 0);
}

struct Thing* get_nearest_enemy_object_possible_to_attack_by(struct Thing* creatng)
{
    SYNCDBG(19, "Starting");
    Thing_Maximizer_Filter filter = near_thing_pos_thing_filter_is_enemy_object_which_can_be_attacked_by_creature;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Object;
    param.model_id = -1;
    param.plyr_idx = -1;
    param.primary_number = creatng->index;
    param.secondary_number = -1;
    param.tertiary_number = -1;
    return get_nth_thing_of_class_with_filter(filter, &param, 0);
}

struct Thing *get_highest_score_enemy_creature_within_distance_possible_to_attack_by(struct Thing *creatng, MapCoordDelta dist, long move_on_ground)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = highest_score_thing_filter_is_enemy_within_distance_which_can_be_attacked_by_creature;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = CREATURE_ANY;
    param.plyr_idx = -1;
    param.primary_number = creatng->index;
    param.secondary_number = dist;
    param.tertiary_number = move_on_ground;
    return get_nth_thing_of_class_with_filter(filter, &param, 0);
}

struct Thing* get_highest_score_enemy_object_within_distance_possible_to_attack_by(struct Thing* creatng, MapCoordDelta dist, long move_on_ground)
{
    SYNCDBG(19, "Starting");
    Thing_Maximizer_Filter filter = highest_score_thing_filter_is_enemy_object_within_distance_which_can_be_attacked_by_creature;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Trap;
    param.model_id = -1;
    param.plyr_idx = -1;
    param.primary_number = creatng->index;
    param.secondary_number = dist;
    param.tertiary_number = move_on_ground;
    return get_nth_thing_of_class_with_filter(filter, &param, 0);
}

struct Thing *get_random_trap_of_model_owned_by_and_armed(ThingModel tngmodel, PlayerNumber plyr_idx, TbBool armed)
{
    SYNCDBG(19,"Starting");
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_trap_of_model_armed_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Trap;
    param.model_id = tngmodel;
    param.plyr_idx = plyr_idx;
    param.primary_number = armed;
    param.secondary_number = -1;
    param.tertiary_number = -1;
    long match_count = count_things_of_class_with_filter(filter, &param);
    if (match_count < 1) {
        return INVALID_THING;
    }
    return get_nth_thing_of_class_with_filter(filter, &param, PLAYER_RANDOM(plyr_idx, match_count));
}

struct Thing *get_random_door_of_model_owned_by_and_locked(ThingModel tngmodel, PlayerNumber plyr_idx, TbBool locked)
{
    SYNCDBG(19,"Starting");
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_door_of_model_locked_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Door;
    param.model_id = tngmodel;
    param.plyr_idx = plyr_idx;
    param.primary_number = locked;
    param.secondary_number = -1;
    param.tertiary_number = -1;
    long match_count = count_things_of_class_with_filter(filter, &param);
    if (match_count < 1) {
        return INVALID_THING;
    }
    return get_nth_thing_of_class_with_filter(filter, &param, PLAYER_RANDOM(plyr_idx, match_count));
}

struct Thing *find_gold_laying_in_dungeon(const struct Dungeon *dungeon)
{
    SYNCDBG(19,"Starting");
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_gold_on_owned_ground_pickable_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Object;
    param.model_id = -1;
    param.plyr_idx = dungeon->owner;
    param.primary_number = -1;
    param.secondary_number = -1;
    param.tertiary_number = -1;
    return get_random_thing_of_class_with_filter(filter, &param, dungeon->owner);
}

long creature_of_model_find_first(ThingModel crmodel)
{
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    long i = slist->index;
    long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Thing list loop body
        if (thing_matches_model(thing,crmodel))
        {
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

struct Thing *creature_of_model_in_prison_or_tortured(ThingModel crmodel)
{
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    long i = slist->index;
    long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Thing list loop body
        if (thing_matches_model(thing,crmodel))
        {
          if (creature_is_kept_in_prison(thing) || creature_is_being_tortured(thing))
              return thing;
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
    for (long crtr_model = 0; crtr_model < game.conf.crtr_conf.model_count; crtr_model++)
    {
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[crtr_model];
        if ((crconf->model_flags & CMF_IsLordOfLand) != 0)
        {
            struct Thing* thing = creature_of_model_in_prison_or_tortured(crtr_model);
            if (!thing_is_invalid(thing))
            {
                if (player_keeping_creature_in_custody(thing) == my_player_number)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

struct Thing *lord_of_the_land_find(void)
{
    for (long crtr_model = 1; crtr_model < game.conf.crtr_conf.model_count; crtr_model++)
    {
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[crtr_model];
        if ((crconf->model_flags & CMF_IsLordOfLand) != 0)
        {
            int i = creature_of_model_find_first(crtr_model);
            if (i > 0)
                return thing_get(i);
        }
    }
    return INVALID_THING;
}

TbBool perform_action_on_all_creatures_in_group(struct Thing *thing, Thing_Bool_Modifier action)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (!creature_is_group_member(thing))
        return false;
    // Find the last creature in group
    struct Thing* ctng = get_group_last_member(thing);
    TbBool result = true;
    // Do the action for every creature in the group, starting from end
    // This allows the creatures to be removed from group or deleted during the update
    long k = 0;
    while (!thing_is_invalid(ctng))
    {
        cctrl = creature_control_get_from_thing(ctng);
        struct Thing* ntng = thing_get(cctrl->prev_in_group);
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
    TbBool affected = false;
    if (!line_of_sight_3d(pos, &tngdst->mappos)) {
        max_dist /= 3;
    }
    // Friendly fire usually causes less damage and at smaller distance
    if ((tngdst->class_id == TCls_Creature) && (tngdst->owner == owner)) {
        max_dist = max_dist * game.conf.rules[tngsrc->owner].magic.friendly_fight_area_range_percent / 100;
        max_damage = max_damage * game.conf.rules[tngsrc->owner].magic.friendly_fight_area_damage_percent / 100;
    }
    MapCoordDelta distance = get_chessboard_distance(pos, &tngdst->mappos);
    if (distance < max_dist)
    {
        if (tngdst->class_id == TCls_Creature)
        {
            HitPoints damage = get_radially_decaying_value(max_damage, max_dist / 2, max_dist / 2, distance);
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
    long naffected = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    long i = slist->index;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
              if (!creature_under_spell_effect(thing, CSAfF_Armour))
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
    for (long n = 1; n < HERO_GATES_COUNT; n++)
    {
        struct Thing* thing = find_hero_gate_of_number(n);
        if (thing_is_invalid(thing))
          return n;
    }
    return 0;
}

/** Does a function on all creatures in players list of given model.
 *
 * @param thing_idx Initial thing index  of players linked list.
 * @param crmodel Creature model to affect, or -1 for all.
 * @param do_cb The callback function to be executed.
 * @return Count of creatures for which the callback returned true.
 */
long do_on_player_list_all_creatures_of_model(long thing_idx, int crmodel,
    Thing_Bool_Modifier do_cb)
{
    long n = 0;
    long i = thing_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per creature code
        if (thing_matches_model(thing, crmodel))
        {
            if (do_cb(thing))
                n++;
        }
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
 * @param crmodel Creature model, or CREATURE_ANY for all, or CREATURE_NOT_A_DIGGER for all except special diggers, CREATURE_DIGGER for special diggers only.
 * @param do_cb The callback function to be executed.
 * @return Count of creatures for which the callback returned true.
 */
long do_to_players_all_creatures_of_model(PlayerNumber plyr_idx, int crmodel, Thing_Bool_Modifier do_cb)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    TbBool is_spec_digger = ((!is_creature_model_wildcard(crmodel) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel)) || (crmodel == CREATURE_DIGGER));
    long count = 0;
    if ((!is_creature_model_wildcard(crmodel) && !is_spec_digger) || (is_creature_model_wildcard(crmodel) && !(crmodel == CREATURE_DIGGER)))
    {
        count += do_on_player_list_all_creatures_of_model(dungeon->creatr_list_start, (is_creature_model_wildcard(crmodel)) ? CREATURE_ANY : crmodel, do_cb);
    }
    if ((!is_creature_model_wildcard(crmodel) && is_spec_digger) || (is_creature_model_wildcard(crmodel) && !(crmodel == CREATURE_NOT_A_DIGGER)))
    {
        count += do_on_player_list_all_creatures_of_model(dungeon->digger_list_start, (is_creature_model_wildcard(crmodel)) ? CREATURE_ANY : crmodel, do_cb);
    }
    return count;
}

/** Counts creatures of given model belonging to given player.
 * @param plyr_idx Target player.
 * @param crmodel Creature model, or CREATURE_ANY for all (except special diggers).
 *
 * @return Count of players creatures.
 */
long count_player_creatures_of_model(PlayerNumber plyr_idx, int crmodel)
{
    SYNCDBG(19,"Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = (is_creature_model_wildcard(crmodel)) ? CREATURE_ANY : crmodel;
    param.plyr_idx = plyr_idx;
    param.primary_number = -1;
    param.secondary_number = -1;
    param.tertiary_number = -1;
    if (dungeon_invalid(dungeon) || player_is_neutral(plyr_idx)) {
        // Invalid dungeon - use list of creatures not associated to any dungeon
        return count_player_list_creatures_with_filter(game.nodungeon_creatr_list_start, filter, &param);
    }
    TbBool is_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    long count = 0;
    if (((crmodel > 0) && (!is_creature_model_wildcard(crmodel)) && !is_spec_digger) ||
        (crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER))
    {
        count += count_player_list_creatures_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    if (((crmodel > 0) && (!is_creature_model_wildcard(crmodel)) && is_spec_digger) ||
        (crmodel == CREATURE_ANY) || (crmodel == CREATURE_DIGGER))
    {
        count += count_player_list_creatures_with_filter(dungeon->digger_list_start, filter, &param);
    }
    return count;
}

long count_player_creatures_of_model_in_action_point(PlayerNumber plyr_idx, int crmodel, long apt_index)
{
    struct ActionPoint* apt = action_point_get(apt_index);
    if (!action_point_exists(apt))
    {
        WARNLOG("Action point is invalid:%d", apt->num);
        return 0;
    }
    if (apt->range == 0)
    {
        WARNLOG("Action point with zero range:%d", apt->num);
        return 0;
    }

    SYNCDBG(19,"Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    Thing_Maximizer_Filter filter = in_action_point_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = (is_creature_model_wildcard(crmodel)) ? CREATURE_ANY : crmodel;
    param.plyr_idx = plyr_idx;
    param.primary_number = apt->mappos.x.val;
    param.secondary_number = apt->mappos.y.val;
    param.tertiary_number = apt->range;
    if (dungeon_invalid(dungeon) || player_is_neutral(plyr_idx)) {
        // Invalid dungeon - use list of creatures not associated to any dungeon
        return count_player_list_creatures_with_filter(game.nodungeon_creatr_list_start, filter, &param);
    }
    TbBool is_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    long count = 0;
    if (((crmodel > 0) && (!is_creature_model_wildcard(crmodel)) && !is_spec_digger) ||
        (crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER))
    {
        count += count_player_list_creatures_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    if (((crmodel > 0) && (!is_creature_model_wildcard(crmodel)) && is_spec_digger) ||
        (crmodel == CREATURE_ANY) || (crmodel == CREATURE_DIGGER))
    {
        count += count_player_list_creatures_with_filter(dungeon->digger_list_start, filter, &param);
    }
    return count;
}

long count_player_list_creatures_of_model(long thing_idx, ThingModel crmodel)
{
    int count = 0;
    long i = thing_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = cctrl->players_next_creature_idx;
        // Per creature code
        if (thing_matches_model(thing,crmodel))
        {
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

long count_player_list_creatures_of_model_on_territory(long thing_idx, ThingModel crmodel, int friendly)
{
    int count = 0;
    long i = thing_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = cctrl->players_next_creature_idx;
        // Per creature code
        int slbwnr = get_slab_owner_thing_is_on(thing);
        if ( thing_matches_model(thing, crmodel) &&
            ( (players_are_enemies(thing->owner,slbwnr) && (friendly == 0)) ||
              (players_are_mutual_allies(thing->owner,slbwnr) && (friendly == 1)) ||
              (slbwnr == game.neutral_player_num && (friendly == 2)) ) )
        {
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

TbBool reset_all_players_creatures_affected_by_cta(PlayerNumber plyr_idx)
{
    SYNCDBG(3,"Processing all player %d creatures",plyr_idx);
    int n = do_to_players_all_creatures_of_model(plyr_idx, CREATURE_ANY, reset_creature_if_affected_by_cta);
    return (n > 0);
}

struct Thing *get_player_list_nth_creature_of_model(long thing_idx, ThingModel crmodel, long crtr_idx)
{
    long i = thing_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            return INVALID_THING;
      }
      struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
      i = cctrl->players_next_creature_idx;
      // Per creature code
      if (thing_matches_model(thing,crmodel))
          crtr_idx--;
      if (crtr_idx == -1)
          return thing;
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

struct Thing* get_player_list_nth_creature_with_property(long thing_idx, unsigned long crmodelflag, long crtr_idx)
{
    long i = thing_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            return INVALID_THING;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per creature code
        if ((get_creature_model_flags(thing) & crmodelflag) == 0)
            crtr_idx--;
        if (crtr_idx == -1)
            return thing;
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

struct Thing *get_player_list_nth_creature_of_model_on_territory(long thing_idx, ThingModel crmodel, long crtr_idx, int friendly)
{
    long i = thing_idx;
    unsigned long k = 0;
    struct Thing *nth_creature = INVALID_THING;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            return INVALID_THING;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        if ((i == -1) || (crtr_idx ==-1))
        {
            return nth_creature;
        }
        // Per creature code
        int slbwnr = get_slab_owner_thing_is_on(thing);
        int match = 0;
        if (friendly == 1)
        {
            if (players_are_mutual_allies(thing->owner,slbwnr))
            {
                match = 1;
            }
        }
        else if (friendly == 0)
        {
            if (players_are_enemies(thing->owner,slbwnr))
            {
                match = 1;
            }
        }
        else
        {
            if (slbwnr == game.neutral_player_num)
            {
                match = 1;
            }
        }

        if ((thing_matches_model(thing, crmodel)) && (match == 1))
        {
            nth_creature = thing;
            crtr_idx--;
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            return INVALID_THING;
        }
    }
    return nth_creature;
}

/**
 * Counts player creatures (not diggers) which are kept out of players control.
 * @param plyr_idx
 */
long count_player_creatures_for_transfer(PlayerNumber plyr_idx)
{
    return count_player_list_creatures_of_model_matching_bool_filter(plyr_idx, CREATURE_NOT_A_DIGGER, creature_can_be_transferred);
}

/**
 * Counts player creatures (not diggers) which are kept out of players control.
 * @param plyr_idx
 */
long count_player_creatures_not_counting_to_total(PlayerNumber plyr_idx)
{
    return count_player_list_creatures_of_model_matching_bool_filter(plyr_idx, CREATURE_NOT_A_DIGGER, creature_is_kept_in_custody_by_enemy_or_dying);
}

/**
 * Counts player diggers which are kept out of players control.
 * @param plyr_idx
 */
long count_player_diggers_not_counting_to_total(PlayerNumber plyr_idx)
{
    return count_player_list_creatures_of_model_matching_bool_filter(plyr_idx, CREATURE_DIGGER, creature_is_kept_in_custody_by_enemy_or_dying);
}

GoldAmount compute_player_payday_total(const struct Dungeon *dungeon)
{
    SYNCDBG(18,"Starting");
    GoldAmount total_pay = 0;
    unsigned long k = 0;
    int i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature (%d) detected for %s.",i, player_code_name(dungeon->owner));
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        total_pay += calculate_correct_creature_pay(thing);
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19,"Finished");
    return total_pay;
}

struct Thing *get_random_players_creature_of_model(PlayerNumber plyr_idx, ThingModel crmodel)
{
    long total_count;
    TbBool is_spec_digger = ((crmodel == CREATURE_DIGGER) || creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel));
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (is_spec_digger)
    {
        total_count = count_player_list_creatures_of_model(dungeon->digger_list_start, crmodel);
    }
    else
    {
        total_count = count_player_list_creatures_of_model(dungeon->creatr_list_start, crmodel);
    }
    if (total_count < 1)
    {
        return INVALID_THING;
    }
    long crtr_idx = PLAYER_RANDOM(plyr_idx, total_count);
    if (is_spec_digger)
    {
        return get_player_list_nth_creature_of_model(dungeon->digger_list_start, crmodel, crtr_idx);
    }
    else
    {
        return get_player_list_nth_creature_of_model(dungeon->creatr_list_start, crmodel, crtr_idx);
    }
}

struct Thing *get_random_players_creature_of_model_on_territory(PlayerNumber plyr_idx, ThingModel crmodel, int friendly)
{
    long model_count;
    long total_count;
    TbBool is_spec_digger = ((crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel));
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (is_spec_digger)
    {
        total_count = count_player_list_creatures_of_model_on_territory(dungeon->digger_list_start, crmodel, friendly);
        model_count = count_player_list_creatures_of_model(dungeon->digger_list_start, crmodel);
    }
    else
    {
        total_count = count_player_list_creatures_of_model_on_territory(dungeon->creatr_list_start, crmodel, friendly);
        model_count = count_player_list_creatures_of_model(dungeon->creatr_list_start, crmodel);
    }
    if (total_count < 1)
    {
      return INVALID_THING;
    }
    long crtr_idx = PLAYER_RANDOM(plyr_idx, model_count);
    if (is_spec_digger)
    {
        return get_player_list_nth_creature_of_model_on_territory(dungeon->digger_list_start, crmodel, crtr_idx, friendly);
    }
    else
    {
        return get_player_list_nth_creature_of_model_on_territory(dungeon->creatr_list_start, crmodel, crtr_idx, friendly);
    }
}

/**
 * Returns amount of filtered creatures from the players creature list starting from thing_idx.
 * Only creatures for whom the filter function will return INT32_MAX, are counted.
 * @return Gives the amount of things which matched the filter.
 */
long count_player_list_creatures_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    SYNCDBG(9,"Starting");
    long count = 0;
    long maximizer = 0;
    unsigned long k = 0;
    long i = thing_idx;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per creature code
        long n = filter(thing, param, maximizer);
        if (n >= maximizer)
        {
            maximizer = n;
            if (maximizer == INT32_MAX)
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

/** Counts on whole map creatures owned by given player, which match given bool filter.
 *
 * @param plyr_idx Player whose things will be searched. Allies are not included, use -1 to select all.
 * @param crmodel Creature model, or CREATURE_ANY for all, or CREATURE_NOT_A_DIGGER for all except special diggers, CREATURE_DIGGER for special diggers only.
 * @param matcher_cb The test callback function to be executed.
 * @return Amount of matching things.
 */
long count_player_list_creatures_of_model_matching_bool_filter(PlayerNumber plyr_idx, int crmodel, Thing_Bool_Filter matcher_cb)
{
    SYNCDBG(19,"Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    Thing_Maximizer_Filter filter = anywhere_thing_filter_call_bool_filter;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = (crmodel <= 0) ? CREATURE_ANY : crmodel;
    param.plyr_idx = plyr_idx;
    param.primary_number = -1;
    param.secondary_number = -1;
    param.tertiary_pointer = (void *)matcher_cb;
    if (dungeon_invalid(dungeon) || player_is_neutral(plyr_idx)) {
        // Invalid dungeon - use list of creatures not associated to any dungeon
        return count_player_list_creatures_with_filter(game.nodungeon_creatr_list_start, filter, &param);
    }
    TbBool is_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    long count = 0;
    if (((crmodel > 0) && !is_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER)) {
        count += count_player_list_creatures_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    if (((crmodel > 0) && is_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_DIGGER)) {
        count += count_player_list_creatures_with_filter(dungeon->digger_list_start, filter, &param);
    }
    return count;
}

/**
 * Returns filtered creature from the players creature list starting from thing_idx.
 * The creature which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return INT32_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Gives the thing, or invalid thing pointer if not found.
 */
struct Thing *get_player_list_creature_with_filter(ThingIndex thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    SYNCDBG(9,"Starting");
    struct Thing* retng = INVALID_THING;
    long maximizer = 0;
    unsigned long k = 0;
    long i = thing_idx;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per creature code
        long n = filter(thing, param, maximizer);
        if (n >= maximizer)
        {
            retng = thing;
            maximizer = n;
            if (maximizer == INT32_MAX)
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
 * If the filter function will return INT32_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * Unlike get_player_list_creature_with_filter(), this function doesn't start checking at thing_idx,
 * but at random index in the list starting at thing_idx. When list end is reached, the function
 * starts checking things of index lower than randomly selected starting index, so all things in list
 * are checked.
 * @return Gives the thing, or invalid thing pointer if not found.
 * @see get_player_list_creature_with_filter()
 */
struct Thing *get_player_list_random_creature_with_filter(ThingIndex thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    // Count all creatures in list, so that we can know range for our random index
    long total_count = count_player_list_creatures_of_model(thing_idx, CREATURE_ANY);
    struct Thing* retng = INVALID_THING;
    long maximizer = 0;
    if (total_count < 1)
        return retng;
    unsigned long k = 0;
    // Get random index of a thing in list
    struct Thing* thing = get_player_list_nth_creature_of_model(thing_idx, CREATURE_ANY, PLAYER_RANDOM(plyr_idx, total_count));
    long i = thing->index;
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
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per creature code
        long n = filter(thing, param, maximizer);
        if (n >= maximizer)
        {
            retng = thing;
            maximizer = n;
            if (maximizer == INT32_MAX)
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
 * If the filter function will return INT32_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Gives the thing, or invalid thing pointer if not found.
 */
struct Thing *get_thing_on_map_block_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param, int32_t *maximizer)
{
    SYNCDBG(19,"Starting");
    struct Thing* retng = INVALID_THING;
    unsigned long k = 0;
    long i = thing_idx;
    while (i != 0)
    {
      struct Thing* thing = thing_get(i);
      if (thing_is_invalid(thing))
      {
          ERRORLOG("Jump to invalid thing detected");
          break;
      }
      i = thing->next_on_mapblk;
      // Begin per-loop code
      long n = filter(thing, param, *maximizer);
      if (n > *maximizer)
      {
          retng = thing;
          *maximizer = n;
          if (*maximizer == INT32_MAX)
          {
              break;
          }
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

struct Thing* get_other_thing_on_map_block_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param, int32_t * maximizer)
{
    SYNCDBG(19, "Starting");
    struct Thing* retng = INVALID_THING;
    unsigned long k = 0;
    long i = thing_idx;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Begin per-loop code
        long n = filter(thing, param, *maximizer);
        if (n >= *maximizer)
        {
            retng = thing;
            *maximizer = n;
            if (*maximizer == INT32_MAX)
            {
                break;
            }
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

long do_to_things_on_map_block(long thing_idx, Thing_Bool_Modifier do_cb)
{
    SYNCDBG(19,"Starting");
    long n = 0;
    unsigned long k = 0;
    long i = thing_idx;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Begin per-loop code
        if (do_cb(thing))
            n++;
        // End of per-loop code
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}

long do_to_things_with_param_on_map_block(ThingIndex thing_idx, Thing_Modifier_Func do_cb, ModTngFilterParam param)
{
    SYNCDBG(19,"Starting");
    long n = 0;
    unsigned long k = 0;
    long i = thing_idx;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Begin per-loop code
        if (do_cb(thing, param) != TUFRet_Unchanged)
            n++;
        // End of per-loop code
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}

/**
 * Returns filtered creature from slab around given coordinates.
 * Skips subtiles which are not revealed to player provided in MaxFilterParam.
 * The thing which will return highest nonnegative value from given filter function
 * will be returned.
 * If the filter function will return INT32_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *get_thing_near_revealed_map_block_with_filter(MapCoord x, MapCoord y, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    SYNCDBG(19,"Starting");
    struct Thing* retng = INVALID_THING;
    long maximizer = 0;
    for (int around_val = 0; around_val < sizeof(mid_around) / sizeof(mid_around[0]); around_val++)
    {
        MapSubtlCoord sx = coord_subtile(x) + (MapSubtlCoord)mid_around[around_val].delta_x;
        MapSubtlCoord sy = coord_subtile(y) + (MapSubtlCoord)mid_around[around_val].delta_y;
        struct Map* mapblk = get_map_block_at(sx, sy);
        if (!map_block_invalid(mapblk))
        {
            if ((param->plyr_idx == -1) || map_block_revealed(mapblk, param->plyr_idx))
            {
                long i = get_mapwho_thing_index(mapblk);
                int32_t n = maximizer;
                struct Thing* thing = get_thing_on_map_block_with_filter(i, filter, param, &n);
                if (!thing_is_invalid(thing) && (n > maximizer))
                {
                    retng = thing;
                    maximizer = n;
                    if (maximizer == INT32_MAX)
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
 * If the filter function will return INT32_MAX, the current creature will be returned
 * immediately and no further things will be checked.
 * @return Returns thing, or invalid thing pointer if not found.
 */
struct Thing *get_thing_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    SYNCDBG(19,"Starting");
    struct Thing* retng = INVALID_THING;
    long maximizer = 0;
    for (int around_val = 0; around_val < spiral_len; around_val++)
    {
        struct MapOffset* sstep = &spiral_step[around_val];
        MapSubtlCoord sx = coord_subtile(x) + (MapSubtlCoord)sstep->h;
        MapSubtlCoord sy = coord_subtile(y) + (MapSubtlCoord)sstep->v;
        struct Map* mapblk = get_map_block_at(sx, sy);
        if (!map_block_invalid(mapblk))
        {
            long i = get_mapwho_thing_index(mapblk);
            int32_t n = maximizer;
            struct Thing* thing = get_thing_on_map_block_with_filter(i, filter, param, &n);
            if (!thing_is_invalid(thing) && (n >= maximizer))
            {
                retng = thing;
                maximizer = n;
                if (maximizer == INT32_MAX)
                    break;
            }
      }
    }
    return retng;
}

/**
 * Returns count of filtered creatures from subtiles around given coordinates.
 * Uses "spiral" checking of surrounding subtiles, up to given number of subtiles.
 * Amount of things for whom the filter function returns INT32_MAX, is returned.
 * @return Gives count of things which matched the filter.
 */
long count_things_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxTngFilterParam param)
{
    SYNCDBG(19,"Starting");
    long count = 0;
    long maximizer = 0;
    for (int around_val = 0; around_val < spiral_len; around_val++)
    {
        struct MapOffset* sstep = &spiral_step[around_val];
        MapSubtlCoord sx = coord_subtile(x) + (MapSubtlCoord)sstep->h;
        MapSubtlCoord sy = coord_subtile(y) + (MapSubtlCoord)sstep->v;
        struct Map* mapblk = get_map_block_at(sx, sy);
        if (!map_block_invalid(mapblk))
        {
            long i = get_mapwho_thing_index(mapblk);
            int32_t n = maximizer;
            struct Thing* thing = get_other_thing_on_map_block_with_filter(i, filter, param, &n);
            if (!thing_is_invalid(thing) && (n >= maximizer))
            {
                maximizer = n;
                if (maximizer == INT32_MAX)
                {
                    count++;
                }
            }
        }
    }
    return count;
}

/**
 * Executes callback for all things on subtiles around given position up to given spiral length.
 * @return Gives amount of things for which callback returned true.
 */
long do_to_things_spiral_near_map_block(MapCoord x, MapCoord y, long spiral_len, Thing_Bool_Modifier do_cb)
{
    SYNCDBG(19,"Starting");
    long count = 0;
    for (int around_val = 0; around_val < spiral_len; around_val++)
    {
        struct MapOffset* sstep = &spiral_step[around_val];
        MapSubtlCoord sx = coord_subtile(x) + (MapSubtlCoord)sstep->h;
        MapSubtlCoord sy = coord_subtile(y) + (MapSubtlCoord)sstep->v;
        struct Map* mapblk = get_map_block_at(sx, sy);
        if (!map_block_invalid(mapblk))
        {
            long i = get_mapwho_thing_index(mapblk);
            count += do_to_things_on_map_block(i, do_cb);
      }
    }
    return count;
}

/**
 * Executes callback for all things on slab around given position.
 * @return Gives amount of things for which callback returned true.
 */
long do_to_things_with_param_around_map_block(const struct Coord3d *center_pos, Thing_Modifier_Func do_cb, ModTngFilterParam param)
{
    SYNCDBG(19,"Starting");
    long count = 0;
    for (int around_val = 0; around_val < sizeof(mid_around) / sizeof(mid_around[0]); around_val++)
    {
        const struct Around* caround = &mid_around[around_val];
        MapSubtlCoord sx = coord_subtile(center_pos->x.val) + caround->delta_x;
        MapSubtlCoord sy = coord_subtile(center_pos->y.val) + caround->delta_y;
        SYNCDBG(18,"Doing on (%d,%d)",(int)sx,(int)sy);
        struct Map* mapblk = get_map_block_at(sx, sy);
        if (!map_block_invalid(mapblk))
        {
            long i = get_mapwho_thing_index(mapblk);
            count += do_to_things_with_param_on_map_block(i, do_cb, param);
        }
    }
    return count;
}

long do_to_things_with_param_spiral_near_map_block(const struct Coord3d *center_pos, MapCoordDelta max_dist, Thing_Modifier_Func do_cb, ModTngFilterParam param)
{
    long spiral_range = coord_subtile(max_dist + COORD_PER_STL - 1);
    if (spiral_range > SPIRAL_STEPS_RANGE) {
        WARNLOG("Spiral range %d trimmed to max %d",(int)spiral_range,SPIRAL_STEPS_RANGE);
        spiral_range = SPIRAL_STEPS_RANGE;
    }
    SYNCDBG(19,"Starting");
    long count = 0;
    for (int around_val = 0; around_val < spiral_range * spiral_range; around_val++)
    {
        struct MapOffset* sstep = &spiral_step[around_val];
        MapSubtlCoord sx = coord_subtile(center_pos->x.val) + sstep->h;
        MapSubtlCoord sy = coord_subtile(center_pos->y.val) + sstep->v;
        SYNCDBG(18,"Doing on (%d,%d)",(int)sx,(int)sy);
        struct Map* mapblk = get_map_block_at(sx, sy);
        if (!map_block_invalid(mapblk))
        {
            long i = get_mapwho_thing_index(mapblk);
            count += do_to_things_with_param_on_map_block(i, do_cb, param);
        }
    }
    return count;
}

void stop_all_things_playing_samples(void)
{
    for (long i = 0; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
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
    SYNCDBG(18,"Thing index %d, class %d",(int)thing->index,(int)thing->class_id);
    TRACE_THING(thing);
    if (thing_is_invalid(thing))
        return false;

    if ((thing->movement_flags & TMvF_Immobile) == 0)
    {
        if ((thing->state_flags & TF1_PushAdd) != 0)
        {
            thing->veloc_base.x.val += thing->veloc_push_add.x.val;
            thing->veloc_base.y.val += thing->veloc_push_add.y.val;
            thing->veloc_base.z.val += thing->veloc_push_add.z.val;
            thing->veloc_push_add.x.val = 0;
            thing->veloc_push_add.y.val = 0;
            thing->veloc_push_add.z.val = 0;
            thing->state_flags &= ~TF1_PushAdd;
        }
        thing->velocity.x.val = thing->veloc_base.x.val;
        thing->velocity.y.val = thing->veloc_base.y.val;
        thing->velocity.z.val = thing->veloc_base.z.val;
        if ((thing->state_flags & TF1_PushOnce) != 0)
        {
          thing->velocity.x.val += thing->veloc_push_once.x.val;
          thing->velocity.y.val += thing->veloc_push_once.y.val;
          thing->velocity.z.val += thing->veloc_push_once.z.val;
          thing->veloc_push_once.x.val = 0;
          thing->veloc_push_once.y.val = 0;
          thing->veloc_push_once.z.val = 0;
          thing->state_flags &= ~TF1_PushOnce;
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
    if ((thing->movement_flags & TMvF_Immobile) == 0)
    {
        if (thing->mappos.z.val > thing->floor_height)
        {
            if (thing->veloc_base.x.val != 0)
                thing->veloc_base.x.val = thing->veloc_base.x.val * (256 - thing->inertia_air) / 256;
            if (thing->veloc_base.y.val != 0)
                thing->veloc_base.y.val = thing->veloc_base.y.val * (256 - thing->inertia_air) / 256;
            if ((thing->movement_flags & TMvF_Flying) == 0)
            {
                thing->veloc_push_add.z.val -= thing->fall_acceleration;
                thing->state_flags |= TF1_PushAdd;
            } else
            {
                // For flying creatures, the Z velocity should also decrease over time
                if (thing->veloc_base.z.val != 0)
                {
                    thing->veloc_base.z.val = thing->veloc_base.z.val * (256 - thing->inertia_air) / 256;
                }
                else
                {
                    if (thing_above_flight_altitude(thing) && ((thing->alloc_flags & TAlF_IsControlled) == 0))
                    {
                        thing->veloc_push_add.z.val -= thing->fall_acceleration;
                        thing->state_flags |= TF1_PushAdd;
                    }
                }
            }
        } else
        {
            if (thing->veloc_base.x.val != 0)
              thing->veloc_base.x.val = thing->veloc_base.x.val * (256 - thing->inertia_floor) / 256;
            if (thing->veloc_base.y.val != 0)
              thing->veloc_base.y.val = thing->veloc_base.y.val * (256 - thing->inertia_floor) / 256;
            thing->mappos.z.val = thing->floor_height;
            if ((thing->movement_flags & TMvF_ZeroVerticalVelocity) != 0)
            {
              thing->veloc_base.z.val = 0;
            }
        }
    }
    update_thing_animation(thing);
    update_thing_sound(thing);
    if ((do_lights) && (thing->light_id != 0))
    {
        if (light_is_light_allocated(thing->light_id))
        {
            struct Coord3d pos;
            pos.x.val = thing->mappos.x.val;
            pos.y.val = thing->mappos.y.val;
            pos.z.val = thing->mappos.z.val + thing->clipbox_size_z;
            light_set_light_position(thing->light_id, &pos);
        } else
        {
            WARNLOG("The %s index %d tries to use non-existing light %d",thing_model_name(thing),(int)thing->index,(int)thing->light_id);
            thing->light_id = 0;
        }
    }
    SYNCDBG(18,"Finished");
    return true;
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

HitTargetFlags collide_filter_thing_is_of_type(const struct Thing *thing, const struct Thing *sectng, HitTargetFlags tngclass, long tngmodel)
{
    if (tngmodel >= 0)
    {
        if (thing->model != tngmodel)
          return false;
    }
    if ((long)tngclass >= 0)
    {
        if (thing->class_id != (long)tngclass)
          return false;
    }
    return true;
}

HitTargetFlags hit_type_to_hit_targets(long hit_type)
{
    switch (hit_type)
    {
    case THit_All:
    case THit_CrtrsNObjctsNShot:
        return HitTF_EnemyCreatures|HitTF_AlliedCreatures|HitTF_OwnedCreatures|HitTF_ArmourAffctdCreatrs|HitTF_PreventDmgCreatrs|
            HitTF_EnemySoulContainer|HitTF_AlliedSoulContainer|HitTF_OwnedSoulContainer|
            HitTF_AnyWorkshopBoxes|HitTF_AnySpellbooks|HitTF_AnyDnSpecialBoxes|
            HitTF_EnemyShotsCollide|HitTF_AlliedShotsCollide|HitTF_OwnedShotsCollide|
            HitTF_EnemyDestructibleTraps|HitTF_AlliedDestructibleTraps|HitTF_OwnedDestructibleTraps|
            HitTF_AnyFoodObjects|HitTF_AnyGoldPiles;
    case THit_CrtrsNObjcts:
        return HitTF_EnemyCreatures|HitTF_AlliedCreatures|HitTF_OwnedCreatures|HitTF_ArmourAffctdCreatrs|HitTF_PreventDmgCreatrs|
            HitTF_EnemySoulContainer|HitTF_AlliedSoulContainer|HitTF_OwnedSoulContainer|
            HitTF_AnyWorkshopBoxes|HitTF_AnySpellbooks|HitTF_AnyDnSpecialBoxes|
            HitTF_EnemyDestructibleTraps|HitTF_AlliedDestructibleTraps|HitTF_OwnedDestructibleTraps|
            HitTF_AnyFoodObjects|HitTF_AnyGoldPiles;
    case THit_CrtrsOnly:
        return HitTF_EnemyCreatures|HitTF_AlliedCreatures|HitTF_OwnedCreatures|HitTF_ArmourAffctdCreatrs;
    case THit_CrtrsOnlyNotOwn:
        return HitTF_EnemyCreatures | HitTF_AlliedCreatures | HitTF_ArmourAffctdCreatrs;
    case THit_CrtrsOnlyOwn:
        return HitTF_OwnedCreatures | HitTF_ArmourAffctdCreatrs;
    case THit_CrtrsNObjctsNotOwn:
        return HitTF_EnemyCreatures|HitTF_AlliedCreatures|HitTF_ArmourAffctdCreatrs|
        HitTF_EnemySoulContainer|HitTF_AlliedSoulContainer|
        HitTF_AnyWorkshopBoxes|HitTF_AnySpellbooks|HitTF_AnyDnSpecialBoxes|
        HitTF_EnemyDestructibleTraps | HitTF_AlliedDestructibleTraps|
        HitTF_AnyFoodObjects|HitTF_AnyGoldPiles;
    case THit_CrtrsNotArmourNotOwn:
        return HitTF_EnemyCreatures|HitTF_AlliedCreatures;
    case THit_HeartOnly:
        return HitTF_EnemySoulContainer|HitTF_AlliedSoulContainer|HitTF_OwnedSoulContainer;
    case THit_HeartOnlyNotOwn:
        return HitTF_EnemySoulContainer|HitTF_AlliedSoulContainer;
    case THit_TrapsAll:
        return HitTF_EnemyDestructibleTraps|HitTF_AlliedDestructibleTraps|HitTF_OwnedDestructibleTraps|
            HitTF_OwnedDeployedTraps|HitTF_AlliedDeployedTraps|HitTF_EnemyDeployedTraps;
    case THit_None:
        return HitTF_None;
    default:
        WARNLOG("Illegal hit thing type %d",(int)hit_type);
        return HitTF_None;
    }
}

/**
 * Returns  whether a thing can be shot by given players shot.
 * @param thing The thing to be checked.
 * @param shot_owner Shot owner to be checked, or -1 if any enemy player should be considered.
 * @param hit_targets Target hit configuration flags.
 * @return
 */
TbBool thing_is_shootable(const struct Thing *thing, PlayerNumber shot_owner, HitTargetFlags hit_targets)
{
    if (thing_is_creature(thing))
    {
        // spectators are not shootable
        if ((get_creature_model_flags(thing) & CMF_IsSpectator) != 0)
            return false;
        // Armour spell may prevent from hitting
        if ((hit_targets & HitTF_ArmourAffctdCreatrs) == 0) {
            if (creature_under_spell_effect(thing, CSAfF_Armour))
                return false;
        }
        // Prevent Damage flag may be either respected or ignored
        if ((hit_targets & HitTF_PreventDmgCreatrs) == 0) {
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            if ((cctrl->creature_control_flags & CCFlg_PreventDamage) != 0)
                return false;
        }
        if (shot_owner == thing->owner) {
            return ((hit_targets & HitTF_OwnedCreatures) != 0);
        }
        if ((shot_owner < 0) || players_are_enemies(shot_owner, thing->owner)) {
            return ((hit_targets & HitTF_EnemyCreatures) != 0);
        }
        return ((hit_targets & HitTF_AlliedCreatures) != 0);
    }
    if (thing_is_shot(thing))
    {
        struct ShotConfigStats* shotst = get_shot_model_stats(thing->model);
        if (shotst->model_flags & ShMF_CanCollide)
        {
            if (shot_owner == thing->owner) {
                return ((hit_targets & HitTF_OwnedShotsCollide) != 0);
            }
            if ((shot_owner < 0) || players_are_enemies(shot_owner, thing->owner)) {
                return ((hit_targets & HitTF_EnemyShotsCollide) != 0);
            }
            return ((hit_targets & HitTF_AlliedShotsCollide) != 0);
        }
        return false;
    }
    if (thing_is_object(thing))
    {
        if (thing_is_dungeon_heart(thing))
        {
            if (shot_owner == thing->owner) {
                return ((hit_targets & HitTF_OwnedSoulContainer) != 0);
            }
            if ((shot_owner < 0) || players_are_enemies(shot_owner, thing->owner)) {
                return ((hit_targets & HitTF_EnemySoulContainer) != 0);
            }
            return ((hit_targets & HitTF_AlliedSoulContainer) != 0);
        }
        if (object_is_growing_food(thing) ||
           (object_is_mature_food(thing) && !is_thing_directly_controlled(thing) && !is_thing_passenger_controlled(thing)))
        {
            return ((hit_targets & HitTF_AnyFoodObjects) != 0);
        }
        if (thing_is_workshop_crate(thing))
        {
            return ((hit_targets & HitTF_AnyWorkshopBoxes) != 0);
        }
        if (thing_is_spellbook(thing))
        {
            return ((hit_targets & HitTF_AnySpellbooks) != 0);
        }
        if (thing_is_special_box(thing))
        {
            return ((hit_targets & HitTF_AnyDnSpecialBoxes) != 0);
        }
        if (thing_is_gold_hoard(thing))
        {
            return ((hit_targets & HitTF_AnyGoldHoards) != 0);
        }
        if (object_is_gold_pile(thing))
        {
            return ((hit_targets & HitTF_AnyGoldPiles) != 0);
        }
        //TODO implement hitting decorations flag
        /*if (object_is_decoration(thing))
        {
            return ((hit_targets & HitTF_AnyDecorations) != 0);
        }*/
        return false;
    }
    if (thing_is_deployed_door(thing))
    {
        if (shot_owner == thing->owner) {
            return ((hit_targets & HitTF_OwnedDeployedDoors) != 0);
        }
        if ((shot_owner < 0) || players_are_enemies(shot_owner, thing->owner)) {
            return ((hit_targets & HitTF_EnemyDeployedDoors) != 0);
        }
        return ((hit_targets & HitTF_AlliedDeployedDoors) != 0);
    }
    if (thing_is_deployed_trap(thing))
    {
        if (thing_is_destructible_trap(thing) > 0)
        {
            if (shot_owner == thing->owner) {
                return ((hit_targets & HitTF_OwnedDestructibleTraps) != 0);
            }
            if ((shot_owner < 0) || players_are_enemies(shot_owner, thing->owner)) {
                return ((hit_targets & HitTF_EnemyDestructibleTraps) != 0);
            }
            return ((hit_targets & HitTF_AlliedDestructibleTraps) != 0);
        }
        if (shot_owner == thing->owner) {
            return ((hit_targets & HitTF_OwnedDeployedTraps) != 0);
        }
        if ((shot_owner < 0) || players_are_enemies(shot_owner, thing->owner)) {
            return ((hit_targets & HitTF_EnemyDeployedTraps) != 0);
        }
        return ((hit_targets & HitTF_AlliedDeployedTraps) != 0);
    }
    if (thing_is_dead_creature(thing))
    {
        return ((hit_targets & HitTF_CreatureDeadBodies) != 0);
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
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return false;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
            break_mapwho_infinite_chain(mapblk);
            break;
        }
  }
  return false;
}

struct Thing *smallest_gold_pile_at_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* chosen_thing = INVALID_THING;
    long chosen_gold = INT32_MAX;
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return chosen_thing;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing processing block
        if ((thing->class_id == TCls_Object) && (object_is_gold_laying_on_ground(thing)))
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
            break_mapwho_infinite_chain(mapblk);
            break;
        }
  }
  return chosen_thing;
}

/**
 * Re-computes speed of a creature and uses it.
 * @param thing
 * @return
 */
TbBool update_creature_speed(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    return true;
}


TbBool update_speed_of_player_creatures_of_model(PlayerNumber plyr_idx, int crmodel)
{
    SYNCDBG(3,"Processing player %d creatures of model %d",plyr_idx,(int)crmodel);
    int n = do_to_players_all_creatures_of_model(plyr_idx, CREATURE_ANY, update_creature_speed);
    return (n > 0);
}

TbBool apply_anger_to_all_players_creatures_excluding(PlayerNumber plyr_idx, long anger, long reason, const struct Thing *excltng)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    unsigned long k = 0;
    int i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
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
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return false;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing processing block
        if ((thing->class_id == TCls_Object) && (object_is_gold_laying_on_ground(thing)))
        {
            if (thing->valuable.gold_stored >= game.conf.rules[thing->owner].game.gold_pile_maximum)
            {
                return true;
            }
        }
        // Per thing processing block ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
  }
  return false;
}

/** Finds thing on revealed subtiles around given position, which matches given bool filter.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose things will be searched. Alies are not included.
 * @return The target thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_object_around_owned_by_and_matching_bool_filter(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, Thing_Bool_Filter matcher_cb)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = near_map_block_thing_filter_call_bool_filter;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Object;
    param.model_id = -1;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    param.tertiary_pointer = (void *)matcher_cb;
    return get_thing_spiral_near_map_block_with_filter(pos_x, pos_y, 9, filter, &param);
}

/** Finds creature on subtiles in range around given position, who is not special digger and is enemy to given player, able to attack.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param distance_stl Max distance, in subtiles. Will work properly only for odd numbers (1,3,5,7...).
 * @param plyr_idx Player whose revealed subtiles around will be searched.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(MapCoord pos_x, MapCoord pos_y, long distance_stl, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = near_map_block_thing_filter_is_enemy_of_able_to_attack_and_not_specdigger;
    struct CompoundTngFilterParam param;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    return get_thing_spiral_near_map_block_with_filter(pos_x, pos_y, distance_stl*distance_stl, filter, &param);
}

/** Finds thing on revealed subtiles around given position, on which given player can cast given spell.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param pwmode Keeper power to be casted.
 * @param plyr_idx Player whose revealed subtiles around will be searched.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near_to_be_keeper_power_target(MapCoord pos_x, MapCoord pos_y, PowerKind pwmodel, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = near_map_block_thing_filter_can_be_keeper_power_target;
    struct CompoundTngFilterParam param;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    param.tertiary_number = pwmodel;
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
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = near_map_block_thing_filter_is_slappable;
    struct CompoundTngFilterParam param;
    param.class_id = -1;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds creature on revealed subtiles around given position, who belongs to given player.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose creature from revealed position will be returned.
 * @param crmodel Creature model or 0 for any
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near_and_owned_by(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, long crmodel)
{
    SYNCDBG(19,"Starting");
    //return get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
    Thing_Maximizer_Filter filter = near_map_block_thing_filter_is_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.plyr_idx = plyr_idx;
    param.model_id = crmodel;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds creature on revealed subtiles around given position.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_near(MapCoord pos_x, MapCoord pos_y)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = near_map_block_thing_filter_is_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.plyr_idx = -1;
    param.model_id = CREATURE_ANY;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

/** Finds creature on all subtiles around given position, who belongs to given player or allied one.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose creature or allied creature will be returned.
 * @param distance_stl Max distance, in subtiles. Will work properly only for odd numbers (1,3,5,7...).
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_creature_in_range_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, MapSubtlDelta distance_stl, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by_or_allied_with;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = CREATURE_ANY;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
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
long count_creatures_near_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, long distance_stl, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by_or_allied_with;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = CREATURE_ANY;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    return count_things_spiral_near_map_block_with_filter(pos_x, pos_y, distance_stl*distance_stl, filter, &param);
}

// use this (or make similar one) instead of find_base_thing_on_mapwho_at_pos()
struct Thing *get_object_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long tngmodel, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Object;
    param.model_id = tngmodel;
    param.plyr_idx = plyr_idx;
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    long i = get_mapwho_thing_index(mapblk);
    int32_t n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_cavein_at_subtile_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_CaveIn;
    param.model_id = -1;
    param.plyr_idx = plyr_idx;
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    long i = get_mapwho_thing_index(mapblk);
    int32_t n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_food_at_subtile_available_to_eat_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_food_available_to_eat_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = -1;
    param.model_id = -1;
    param.plyr_idx = plyr_idx;
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    long i = get_mapwho_thing_index(mapblk);
    int32_t n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_trap_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, ThingModel model, long plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Trap;
    param.model_id = model;
    param.plyr_idx = plyr_idx;
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    long i = get_mapwho_thing_index(mapblk);
    int32_t n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

/** Finds trap on all subtiles around given one, which belongs to given player and is of given model.
 *
 * @param pos_x Position to search around X coord.
 * @param pos_y Position to search around Y coord.
 * @param plyr_idx Player whose creature or allied creature will be returned.
 * @return The creature thing pointer, or invalid thing pointer if not found.
 */
struct Thing *get_trap_around_of_model_and_owned_by(MapCoord pos_x, MapCoord pos_y, ThingModel model, PlayerNumber plyr_idx)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Trap;
    param.model_id = model;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    return get_thing_spiral_near_map_block_with_filter(pos_x, pos_y, 9, filter, &param);
}

struct Thing *get_door_for_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Door;
    param.model_id = -1;
    param.plyr_idx = -1;
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    const struct Map* mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    // const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    long i = get_mapwho_thing_index(mapblk);
    int32_t n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing *get_door_for_position_for_trap_placement(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_of_class_and_model_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Door;
    param.model_id = -1;
    param.plyr_idx = -1;
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    long i = get_mapwho_thing_index(mapblk);
    int32_t n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

TbBool slab_has_door_thing_on(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Thing* doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    return !thing_is_invalid(doortng);
}

struct Thing *get_creature_of_model_training_at_subtile_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model_id, PlayerNumber plyr_idx, long skip_thing_id)
{
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = anywhere_thing_filter_is_creature_of_model_training_and_owned_by;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = model_id;
    param.plyr_idx = plyr_idx;
    param.primary_number = skip_thing_id;
    const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
    {
        return INVALID_THING;
    }
    long i = get_mapwho_thing_index(mapblk);
    int32_t n = 0;
    return get_thing_on_map_block_with_filter(i, filter, &param, &n);
}

struct Thing* get_nearest_object_with_tooltip_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool optional)
{
    if (optional)
    {
        return get_object_around_owned_by_and_matching_bool_filter(
            subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, thing_is_object_with_optional_tooltip);
    }
    else
    {
        return get_object_around_owned_by_and_matching_bool_filter(
            subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, thing_is_object_with_mandatory_tooltip);
    }
}

struct Thing *get_nearest_thing_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  long OldDistance = INT32_MAX;
  struct Thing *thing;
  unsigned char n,k = 0;
  struct Thing *result = NULL;
  MapSubtlCoord x,y;
  do
  {
    n = 0;
    y = stl_y + k;
    if ( (y >= 0) && (y < game.map_subtiles_y) )
    {
      do
      {
        x = stl_x + n;
        if ( (x >= 0) && (x < game.map_subtiles_x) )
        {
          struct Map *blk = get_map_block_at(x, y);
          thing = thing_get(get_mapwho_thing_index(blk));
          while (!thing_is_invalid(thing))
          {
            TRACE_THING(thing);
            long NewDistance = chessboard_distance(stl_x, stl_y, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            if ( NewDistance < OldDistance )
            {
                OldDistance = NewDistance;
                result = thing;
            }
            thing = thing_get(thing->next_on_mapblk);
          }
        }
      n++;
      }
      while ( n < STL_PER_SLB );
    }
    k++;
  }
  while ( k < STL_PER_SLB );
  return result;
}

void remove_dead_creatures_from_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    do_to_things_spiral_near_map_block(subtile_coord_center(stl_x), subtile_coord_center(stl_y), 9, delete_if_dead_creature);
}

long switch_owned_objects_on_destoyed_slab_to_neutral(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber prev_owner)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
    pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
    pos.z.val = 0;
    struct CompoundTngFilterParam param;
    param.plyr_idx = prev_owner;
    param.class_id = 0;
    param.model_id = 0;
    param.primary_number = game.neutral_player_num;
    param.secondary_number = 0;
    param.tertiary_pointer = 0;
    Thing_Modifier_Func do_cb = switch_object_on_destoyed_slab_to_new_owner;
    return do_to_things_with_param_around_map_block(&pos, do_cb, &param);
}

TbBool setup_creature_leave_or_die_if_possible(struct Thing *thing)
{
    if (!is_thing_some_way_controlled(thing) && !creature_is_dying(thing))
    {
        if (!creature_is_kept_in_custody_by_enemy(thing) && !creature_is_being_unconscious(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            SYNCDBG(9,"Forcing on %s index %d",thing_model_name(thing),(int)thing->index);
            // Drop creature if it's being dragged
            force_any_creature_dragging_thing_to_drop_it(thing);
            // Drop creature if it's in hand
            if (thing_is_picked_up(thing)) {
                if ((game.conf.rules[thing->owner].game.classic_bugs_flags & ClscBug_NoHandPurgeOnDefeat) != 0) {
                    SYNCDBG(19,"Skipped %s index %d due to classic bug",thing_model_name(thing),(int)thing->index);
                    return false;
                }
                dump_thing_held_by_any_player(thing);
            }
            // Setup leave state or kill the creature
            setup_creature_leaves_or_dies(thing);
            return true;
        }
    }
    SYNCDBG(19,"Skipped %s index %d",thing_model_name(thing),(int)thing->index);
    return false;
}

TbBool setup_creature_die_if_not_in_custody(struct Thing *thing)
{
    if (!creature_is_kept_in_custody_by_enemy(thing))
    {
        SYNCDBG(19,"Forcing on %s index %d",thing_model_name(thing),(int)thing->index);
        // Drop creature if it's being dragged
        force_any_creature_dragging_thing_to_drop_it(thing);
        // Drop creature if it's in hand
        if (thing_is_picked_up(thing)) {
            if ((game.conf.rules[thing->owner].game.classic_bugs_flags & ClscBug_NoHandPurgeOnDefeat) != 0) {
                SYNCDBG(19,"Skipped %s index %d due to classic bug",thing_model_name(thing),(int)thing->index);
                return false;
            }
            dump_thing_held_by_any_player(thing);
        }
        // And kill it
        kill_creature(thing, INVALID_THING, -1, CrDed_Default);
        return true;
    }
    SYNCDBG(19,"Skipped %s index %d",thing_model_name(thing),(int)thing->index);
    return false;
}

void setup_all_player_creatures_and_diggers_leave_or_die(PlayerNumber plyr_idx)
{
    if (!player_is_keeper(plyr_idx)) {
        // Don't affect heroes and neutral creatures
        return;
    }
    // Force leave or kill normal creatures
    do_to_players_all_creatures_of_model(plyr_idx, CREATURE_NOT_A_DIGGER, setup_creature_leave_or_die_if_possible);
    // Kill all special diggers
    do_to_players_all_creatures_of_model(plyr_idx, CREATURE_DIGGER, setup_creature_die_if_not_in_custody);
}

unsigned short setup_excess_creatures_to_leave_or_die(short max_remain)
{
    struct CreatureControl* cctrl;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    unsigned long k = 0;
    int i = slist->index;
    short count = 0;
    if (slist->count <= max_remain)
    {
        return count;
    }
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        cctrl = creature_control_get_from_thing(thing);
        if (cctrl->index > max_remain)
        {
            count++;
            cleanup_creature_state_and_interactions(thing);
            force_any_creature_dragging_thing_to_drop_it(thing);

            if (is_thing_some_way_controlled(thing) || thing_is_picked_up(thing))
            {
                kill_creature(thing, INVALID_THING, -1, CrDed_Default);
            }
            else
            {
                setup_creature_leaves_or_dies(thing);
            }
        }
        // Per-thing code ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return count;
}

long count_creatures_in_dungeon_of_model_flags(const struct Dungeon *dungeon, unsigned long need_mdflags, unsigned long excl_mdflags)
{
    long count = 0;
    for (ThingModel crmodel = 1; crmodel < game.conf.crtr_conf.model_count; crmodel++)
    {
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[crmodel];
        if (((crconf->model_flags & need_mdflags) == need_mdflags) &&
           ((crconf->model_flags & excl_mdflags) == 0))
        {
            count += dungeon->owned_creatures_of_model[crmodel];
        }
    }
    return count;
}

long count_creatures_in_dungeon_controlled_and_of_model_flags(const struct Dungeon *dungeon, unsigned long need_mdflags, unsigned long excl_mdflags)
{
    long count = 0;
    for (ThingModel crmodel = 1; crmodel < game.conf.crtr_conf.model_count; crmodel++)
    {
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[crmodel];
        if (((crconf->model_flags & need_mdflags) == need_mdflags) &&
           ((crconf->model_flags & excl_mdflags) == 0))
        {
            count += dungeon->owned_creatures_of_model[crmodel]
              - count_player_list_creatures_of_model_matching_bool_filter(dungeon->owner, crmodel, creature_is_kept_in_custody_by_enemy_or_dying);
        }
    }
    return count;
}

void break_mapwho_infinite_chain(const struct Map *mapblk)
{
    SYNCDBG(8,"Starting");
    long i_first = get_mapwho_thing_index(mapblk);
    long i_prev[2];
    i_prev[1] = 0;
    i_prev[0] = 0;
    while (i_first != 0)
    {
        // Per thing code start
        unsigned long k = 0;
        long i = i_first;
        while (i != 0)
        {
            struct Thing* thing = thing_get(i);
            TRACE_THING(thing);
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Jump to invalid thing detected");
                break;
            }
            i_prev[1] = i_prev[0];
            i_prev[0] = i;
            i = thing->next_on_mapblk;
            // Per thing code start
            if (i == i_first) {
                thing->next_on_mapblk = 0;
                thing->prev_on_mapblk = i_prev[1];
                WARNLOG("The things chain has been cut");
                return;
            }
            // Per thing code end
            k++;
            if (k > THINGS_COUNT)
            {
                break;
            }
        }
        // Per thing code end
        struct Thing* thing = thing_get(i_first);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i_prev[0] = i_first;
        i_first = thing->next_on_mapblk;
    }
    WARNLOG("No change performed");
}

ThingIndex get_index_of_next_creature_of_owner_and_model(struct Thing *current_creature, PlayerNumber owner, ThingModel crmodel, struct PlayerInfo *player)
{
    unsigned long k = 0;
    ThingIndex i = current_creature->index;
    do
    {
        i++;
        if (i >= THINGS_COUNT)
        {
            i = 1;
        }
        else if (i == current_creature->index)
        {
            break;
        }
        struct Thing* thing = thing_get(i);
        if (thing_is_creature(thing))
        {
            if ( (thing->owner == owner) || (owner == -1) )
            {
                if ( (thing->model == crmodel) || (crmodel == 0) )
                {
                    if (creature_can_be_queried(player, thing))
                    {
                        return thing->index;
                    }
                }
            }
        }
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    while (1);
    return current_creature->index;
}

struct Thing* get_timebomb_target(struct Thing *creatng)
{
    struct Thing *retng = NULL;
    MapCoordDelta dist, new_dist;
    struct Thing* thing;
    struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    if (slist != NULL)
    {
        dist = INT32_MAX;
        unsigned long i = slist->index;
        unsigned long k = 0;
        while (i != 0)
        {
            thing = thing_get(i);
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Jump to invalid thing detected");
                break;
            }
            if (players_are_enemies(creatng->owner, thing->owner))
            {
                if (creature_will_attack_creature(creatng, thing))
                {
                    new_dist = get_2d_distance(&creatng->mappos, &thing->mappos);
                    if (new_dist < dist)
                    {
                        if (creature_can_navigate_to(creatng, &thing->mappos, NavRtF_Default))
                        {
                            dist = new_dist;
                            retng = thing;
                        }
                    }
                }
            }
            // Per-thing code ends
            k++;
            if (k > slist->count)
            {
                ERRORLOG("Infinite loop detected when sweeping things list");
                break;
            }
            i = thing->next_of_class;
        }
    }
    if (thing_is_invalid(retng))
    {
        dist = INT32_MAX;
        for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
        {
            if (players_are_enemies(creatng->owner, plyr_idx))
            {
                thing = get_player_soul_container(plyr_idx);
                if (thing_exists(thing))
                {
                    new_dist = get_2d_distance(&creatng->mappos, &thing->mappos);
                    if (new_dist < dist)
                    {
                        if (creature_can_navigate_to(creatng, &thing->mappos, NavRtF_Default))
                        {
                            dist = new_dist;
                            retng = thing;
                        }
                    }
                }
            }
        }
    }
    return retng;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
