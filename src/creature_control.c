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
struct CreatureControl *creature_control_get(CctrlIndex cctrl_idx)
{
  if ((cctrl_idx < 1) || (cctrl_idx >= CREATURES_COUNT))
    return INVALID_CRTR_CONTROL;
  return game.persons.cctrl_lookup[cctrl_idx];
}

/**
 * Returns CreatureControl assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureControl *creature_control_get_from_thing(const struct Thing *thing)
{
  if ((thing->ccontrol_idx < 1) || (thing->ccontrol_idx >= CREATURES_COUNT))
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
  if ((cctrl->creature_control_flags & CCFlg_Exists) == 0)
      return false;
  return true;
}

CctrlIndex i_can_allocate_free_control_structure(void)
{
    for (CctrlIndex i = 1; i < CREATURES_COUNT; i++)
    {
        struct CreatureControl* cctrl = game.persons.cctrl_lookup[i];
        if (!creature_control_invalid(cctrl))
        {
            if ((cctrl->creature_control_flags & CCFlg_Exists) == 0)
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
            if ((cctrl->creature_control_flags & CCFlg_Exists) == 0)
            {
                memset(cctrl, 0, sizeof(struct CreatureControl));
                cctrl->creature_control_flags |= CCFlg_Exists;
                cctrl->index = i;
                return cctrl;
            }
      }
    }
    return NULL;
}

void delete_control_structure(struct CreatureControl *cctrl)
{
    memset(cctrl, 0, sizeof(struct CreatureControl));
}

void delete_all_control_structures(void)
{
    for (long i = 1; i < CREATURES_COUNT; i++)
    {
        struct CreatureControl* cctrl = creature_control_get(i);
        if (!creature_control_invalid(cctrl))
        {
            if ((cctrl->creature_control_flags & CCFlg_Exists) != 0)
                delete_control_structure(cctrl);
      }
    }
}

struct Thing *create_and_control_creature_as_controller(struct PlayerInfo *player, ThingModel crmodel, struct Coord3d *pos)
{
    SYNCDBG(6,"Request for model %d (%s) at (%d,%d,%d)",
        crmodel, creature_code_name(crmodel),(int)pos->x.val,(int)pos->y.val,(int)pos->z.val);
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
    thing->rendering_flags |= TRF_Invisible;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->creature_state_flags |= TF2_Spectator;
    cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    set_player_mode(player, PVT_CreatureContrl);
    set_start_state(thing);
    // Preparing light object
    struct InitLight ilght;
    memset(&ilght, 0, sizeof(struct InitLight));
    ilght.mappos.x.val = thing->mappos.x.val;
    ilght.mappos.y.val = thing->mappos.y.val;
    ilght.mappos.z.val = thing->mappos.z.val;
    ilght.intensity = 36;
    ilght.flags = 1;
    ilght.is_dynamic = 1;
    ilght.radius = 10 * COORD_PER_STL;
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
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
            SYNCDBG(7,"Possessing creature '%s', eye_effect=%d", crconf->name, crconf->eye_effect);
            setup_eye_lens(crconf->eye_effect);
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
    ThingModel cmodel = thing->model;
    if ((cmodel < 1) || (cmodel >= game.conf.crtr_conf.model_count))
    {
        ERRORLOG("Trying to get sound for undefined creature type %d",(int)cmodel);
        // Return dummy element
        return &game.conf.crtr_conf.creature_sounds[0].foot;
    }
    switch (snd_idx)
    {
    case CrSnd_Hurt:
        return &game.conf.crtr_conf.creature_sounds[cmodel].hurt;
    case CrSnd_Hit:
        return &game.conf.crtr_conf.creature_sounds[cmodel].hit;
    case CrSnd_Happy:
        return &game.conf.crtr_conf.creature_sounds[cmodel].happy;
    case CrSnd_Sad:
        return &game.conf.crtr_conf.creature_sounds[cmodel].sad;
    case CrSnd_Hang:
        return &game.conf.crtr_conf.creature_sounds[cmodel].hang;
    case CrSnd_Drop:
        return &game.conf.crtr_conf.creature_sounds[cmodel].drop;
    case CrSnd_Torture:
        return &game.conf.crtr_conf.creature_sounds[cmodel].torture;
    case CrSnd_Slap:
        return &game.conf.crtr_conf.creature_sounds[cmodel].slap;
    case CrSnd_Die:
        return &game.conf.crtr_conf.creature_sounds[cmodel].die;
    case CrSnd_Foot:
        return &game.conf.crtr_conf.creature_sounds[cmodel].foot;
    case CrSnd_Fight:
        return &game.conf.crtr_conf.creature_sounds[cmodel].fight;
    case CrSnd_Piss:
        return &game.conf.crtr_conf.creature_sounds[cmodel].piss;
    default:
        // Return dummy element
        return &game.conf.crtr_conf.creature_sounds[0].foot;
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
        SYNCDBG(19,"No sample %ld for creature %d",snd_idx,thing->model);
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

void play_creature_sound(struct Thing *thing, long snd_idx, long priority, long use_flags)
{
    SYNCDBG(8,"Starting");
    if (playing_creature_sound(thing, snd_idx)) {
      return;
    }
    struct CreatureSound* crsound = get_creature_sound(thing, snd_idx);
    if (crsound->index <= 0) {
        SYNCDBG(19,"No sample %ld for creature %d",snd_idx,thing->model);
        return;
    }
    long i = SOUND_RANDOM(crsound->count);
    SYNCDBG(18,"Playing sample %ld (index %ld) for creature %d",snd_idx,crsound->index+i,thing->model);
    if ( use_flags ) {
        thing_play_sample(thing, crsound->index+i, NORMAL_PITCH, 0, 3, 8, priority, FULL_LOUDNESS);
    } else {
        thing_play_sample(thing, crsound->index+i, NORMAL_PITCH, 0, 3, 0, priority, FULL_LOUDNESS);
    }
}

void play_creature_sound_and_create_sound_thing(struct Thing *thing, long snd_idx, long sound_priority)
{
    if (playing_creature_sound(thing, snd_idx)) {
        return;
    }
    struct CreatureSound* crsound = get_creature_sound(thing, snd_idx);
    if (crsound->index <= 0) {
        SYNCDBG(14,"No sample %ld for creature %d",snd_idx,thing->model);
        return;
    }
    long i = SOUND_RANDOM(crsound->count);
    struct Thing* efftng = create_effect(&thing->mappos, TngEff_Dummy, thing->owner);
    if (!thing_is_invalid(efftng)) {
        thing_play_sample(efftng, crsound->index+i, NORMAL_PITCH, 0, 3, 0, sound_priority, FULL_LOUDNESS);
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
    if (cctrl->exp_level >= dungeon->creature_max_level[thing->model])
        return false;
    // Creatures which reached absolute max level and have no grow up creature
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    if ((cctrl->exp_level >= (CREATURE_MAX_LEVEL-1)) && (crconf->grow_up == 0))
        return false;
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
