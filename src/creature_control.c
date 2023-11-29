/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_control.c
 *     CreatureControl structure support functions.
 * @par Purpose:
 *     Functions to use CreatureControl for controlling creatures.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     23 Apr 2009 - 16 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "creature_control.h"
#include "globals.h"

#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "config_creature.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "player_instances.h"
#include "frontend.h"
#include "lens_api.h"
#include "light_data.h"
#include "sounds.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
/**
 * Returns CreatureControl of given index.
 */
struct CreatureControl *creature_control_get(long cctrl_idx)
{
  if ((cctrl_idx < 1) || (cctrl_idx > CREATURES_COUNT))
    return INVALID_CRTR_CONTROL;
  return game.persons.cctrl_lookup[cctrl_idx];
}

/**
 * Returns CreatureControl assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureControl *creature_control_get_from_thing(const struct Thing *thing)
{
  if ((thing->ccontrol_idx < 1) || (thing->ccontrol_idx > CREATURES_COUNT))
    return INVALID_CRTR_CONTROL;
  return game.persons.cctrl_lookup[thing->ccontrol_idx];
}

/**
 * Returns if given CreatureControl pointer is incorrect.
 */
TbBool creature_control_invalid(const struct CreatureControl *cctrl)
{
  return (cctrl <= game.persons.cctrl_lookup[0]) || (cctrl == NULL);
}

TbBool creature_control_exists(const struct CreatureControl *cctrl)
{
  if (creature_control_invalid(cctrl))
      return false;
  if ((cctrl->flgfield_1 & CCFlg_Exists) == 0)
      return false;
  return true;
}

TbBool creature_control_exists_in_thing(const struct Thing *thing)
{
    return creature_control_exists(creature_control_get_from_thing(thing));
}

long i_can_allocate_free_control_structure(void)
{
    for (long i = 1; i < CREATURES_COUNT; i++)
    {
        struct CreatureControl* cctrl = game.persons.cctrl_lookup[i];
        if (!creature_control_invalid(cctrl))
        {
            if ((cctrl->flgfield_1 & CCFlg_Exists) == 0)
                return i;
        }
  }
  return 0;
}

struct CreatureControl *allocate_free_control_structure(void)
{
    for (long i = 1; i < CREATURES_COUNT; i++)
    {
        struct CreatureControl* cctrl = game.persons.cctrl_lookup[i];
        if (!creature_control_invalid(cctrl))
        {
            if ((cctrl->flgfield_1 & CCFlg_Exists) == 0)
            {
                LbMemorySet(cctrl, 0, sizeof(struct CreatureControl));
                cctrl->flgfield_1 |= CCFlg_Exists;
                cctrl->index = i;
                return cctrl;
            }
      }
    }
    return NULL;
}

void delete_control_structure(struct CreatureControl *cctrl)
{
    LbMemorySet(cctrl, 0, sizeof(struct CreatureControl));
}

void delete_all_control_structures(void)
{
    for (long i = 1; i < CREATURES_COUNT; i++)
    {
        struct CreatureControl* cctrl = creature_control_get(i);
        if (!creature_control_invalid(cctrl))
        {
            if ((cctrl->flgfield_1 & CCFlg_Exists) != 0)
                delete_control_structure(cctrl);
      }
    }
}

struct Thing *create_and_control_creature_as_controller(struct PlayerInfo *player, long crmodel, struct Coord3d *pos)
{
    SYNCDBG(6,"Request for model %ld (%s) at (%d,%d,%d)",crmodel, creature_code_name(crmodel),(int)pos->x.val,(int)pos->y.val,(int)pos->z.val);
    struct Thing* thing = create_creature(pos, crmodel, player->id_number);
    if (thing_is_invalid(thing))
      return INVALID_THING;
    // Do not count the spectator creature as real creature
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    dungeon->num_active_creatrs--;
    dungeon->owned_creatures_of_model[thing->model]--;
    if (is_my_player(player))
    {
        toggle_status_menu(0);
        turn_off_roaming_menus();
    }
    const struct Camera* cam = player->acamera;
    set_selected_creature(player, thing);
    player->view_mode_restore = cam->view_mode;
    thing->alloc_flags |= TAlF_IsControlled;
    thing->rendering_flags |= TRF_Unknown01;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->flgfield_2 |= TF2_Spectator;
    cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    set_player_mode(player, PVT_CreatureContrl);
    set_start_state(thing);
    // Preparing light object
    struct InitLight ilght;
    LbMemorySet(&ilght, 0, sizeof(struct InitLight));
    ilght.mappos.x.val = thing->mappos.x.val;
    ilght.mappos.y.val = thing->mappos.y.val;
    ilght.mappos.z.val = thing->mappos.z.val;
    ilght.intensity = 36;
    ilght.field_3 = 1;
    ilght.is_dynamic = 1;
    ilght.radius = 2560;
    thing->light_id = light_create_light(&ilght);
    if (thing->light_id != 0)
    {
        light_set_light_never_cache(thing->light_id);
    } else
    {
        ERRORLOG("Cannot allocate light to new hero");
    }
    if (is_my_player_number(thing->owner))
    {
        if (thing->class_id == TCls_Creature)
        {
            struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
            setup_eye_lens(crstat->eye_effect);
        }
    }
    return thing;
}

void clear_creature_instance(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->instance_id = CrInst_NULL;
    cctrl->inst_turn = 0;
}

struct Thing *get_group_last_member(struct Thing *thing)
{
    struct Thing* ctng = thing;
    struct CreatureControl* cctrl = creature_control_get_from_thing(ctng);
    long k = 0;
    while (cctrl->next_in_group > 0)
    {
        ctng = thing_get(cctrl->next_in_group);
        cctrl = creature_control_get_from_thing(ctng);
        k++;
        if (k > CREATURES_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures group");
          break;
        }
    }
    return ctng;
}

TbBool disband_creatures_group(struct Thing *thing)
{
    // Disband the group, removing creatures from end
    SYNCDBG(3,"Removing %s index %d owned by player %d",thing_model_name(thing),(int)thing->index,(int)thing->owner);
    return perform_action_on_all_creatures_in_group(thing, remove_creature_from_group_without_leader_consideration);
}

struct CreatureSound *get_creature_sound(struct Thing *thing, long snd_idx)
{
    unsigned int cmodel = thing->model;
    if ((cmodel < 1) || (cmodel >= gameadd.crtr_conf.model_count))
    {
        ERRORLOG("Trying to get sound for undefined creature type %d",(int)cmodel);
        // Return dummy element
        return &gameadd.crtr_conf.creature_sounds[0].foot;
    }
    switch (snd_idx)
    {
    case CrSnd_Hurt:
        return &gameadd.crtr_conf.creature_sounds[cmodel].hurt;
    case CrSnd_Hit:
        return &gameadd.crtr_conf.creature_sounds[cmodel].hit;
    case CrSnd_Happy:
        return &gameadd.crtr_conf.creature_sounds[cmodel].happy;
    case CrSnd_Sad:
        return &gameadd.crtr_conf.creature_sounds[cmodel].sad;
    case CrSnd_Hang:
        return &gameadd.crtr_conf.creature_sounds[cmodel].hang;
    case CrSnd_Drop:
        return &gameadd.crtr_conf.creature_sounds[cmodel].drop;
    case CrSnd_Torture:
        return &gameadd.crtr_conf.creature_sounds[cmodel].torture;
    case CrSnd_Slap:
        return &gameadd.crtr_conf.creature_sounds[cmodel].slap;
    case CrSnd_Die:
        return &gameadd.crtr_conf.creature_sounds[cmodel].die;
    case CrSnd_Foot:
        return &gameadd.crtr_conf.creature_sounds[cmodel].foot;
    case CrSnd_Fight:
        return &gameadd.crtr_conf.creature_sounds[cmodel].fight;
    default:
        // Return dummy element
        return &gameadd.crtr_conf.creature_sounds[0].foot;
    }
}

TbBool playing_creature_sound(struct Thing *thing, long snd_idx)
{
    struct CreatureSound* crsound = get_creature_sound(thing, snd_idx);
    for (long i = 0; i < crsound->count; i++)
    {
        if (S3DEmitterIsPlayingSample(thing->snd_emitter_id, crsound->index+i, 0))
          return true;
    }
    return false;
}

void stop_creature_sound(struct Thing *thing, long snd_idx)
{
    struct CreatureSound* crsound = get_creature_sound(thing, snd_idx);
    if (crsound->index <= 0) {
        SYNCDBG(19,"No sample %d for creature %d",snd_idx,thing->model);
        return;
    }

    for (int i = 0; i < crsound->count; i++)
    {
        if (S3DEmitterIsPlayingSample(thing->snd_emitter_id, crsound->index+i, 0))
        {
            S3DDeleteSampleFromEmitter(thing->snd_emitter_id, crsound->index+i, 0);
        }
    }
}

void play_creature_sound(struct Thing *thing, long snd_idx, long a3, long a4)
{
    SYNCDBG(8,"Starting");
    if (playing_creature_sound(thing, snd_idx)) {
      return;
    }
    struct CreatureSound* crsound = get_creature_sound(thing, snd_idx);
    if (crsound->index <= 0) {
        SYNCDBG(19,"No sample %d for creature %d",snd_idx,thing->model);
        return;
    }
    long i = UNSYNC_RANDOM(crsound->count);
    SYNCDBG(18,"Playing sample %d (index %d) for creature %d",snd_idx,crsound->index+i,thing->model);
    if ( a4 ) {
        thing_play_sample(thing, crsound->index+i, NORMAL_PITCH, 0, 3, 8, a3, FULL_LOUDNESS);
    } else {
        thing_play_sample(thing, crsound->index+i, NORMAL_PITCH, 0, 3, 0, a3, FULL_LOUDNESS);
    }
}

void play_creature_sound_and_create_sound_thing(struct Thing *thing, long snd_idx, long a2)
{
    if (playing_creature_sound(thing, snd_idx)) {
        return;
    }
    struct CreatureSound* crsound = get_creature_sound(thing, snd_idx);
    if (crsound->index <= 0) {
        SYNCDBG(14,"No sample %d for creature %d",snd_idx,thing->model);
        return;
    }
    long i = UNSYNC_RANDOM(crsound->count);
    struct Thing* efftng = create_effect(&thing->mappos, TngEff_DamageBlood, thing->owner);
    if (!thing_is_invalid(efftng)) {
        thing_play_sample(efftng, crsound->index+i, NORMAL_PITCH, 0, 3, 0, a2, FULL_LOUDNESS);
    }
}

void reset_creature_eye_lens(struct Thing *thing)
{
    setup_eye_lens(0);
}

TbBool creature_can_gain_experience(const struct Thing *thing)
{
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // Creatures which reached players max level can't be trained
    if (cctrl->explevel >= dungeon->creature_max_level[thing->model])
        return false;
    // Creatures which reached absolute max level and have no grow up creature
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    if ((cctrl->explevel >= (CREATURE_MAX_LEVEL-1)) && (crstat->grow_up == 0))
        return false;
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
