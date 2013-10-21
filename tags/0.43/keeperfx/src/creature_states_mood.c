/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_mood.c
 *     Creature state machine functions related to their mood.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_mood.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_sound.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_creature_moan(struct Thing *thing);
DLLIMPORT short _DK_creature_roar(struct Thing *thing);
DLLIMPORT short _DK_creature_be_happy(struct Thing *thing);
DLLIMPORT short _DK_creature_piss(struct Thing *thing);
DLLIMPORT short _DK_mad_killing_psycho(struct Thing *thing);
DLLIMPORT void _DK_anger_set_creature_anger(struct Thing *creatng, long a1, long wandr_select);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short creature_moan(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long i;
    //return _DK_creature_moan(thing);
    cctrl = creature_control_get_from_thing(thing);
    i = cctrl->field_282;
    if (i > 0) i--;
    cctrl->field_282 = i;
    if (i <= 0)
    {
        if (cctrl->instance_id == CrInst_NULL) {
            set_start_state(thing);
        }
        return 0;
    }
    if (game.play_gameturn - cctrl->long_9A > 32)
    {
        play_creature_sound(thing, 4, 2, 0);
        cctrl->long_9A = game.play_gameturn;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(thing, CrInst_MOAN, 1, 0, 0);
    }
    return 1;
}

short creature_roar(struct Thing *thing)
{
  return _DK_creature_roar(thing);
}

short creature_be_happy(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long i;
    //return _DK_creature_be_happy(thing);
    cctrl = creature_control_get_from_thing(thing);
    i = cctrl->field_282;
    if (i > 0) i--;
    cctrl->field_282 = i;
    if (i <= 0)
    {
      if (cctrl->instance_id == CrInst_NULL) {
          set_start_state(thing);
      }
      return 0;
    }
    if (game.play_gameturn - cctrl->long_9A > 32)
    {
        play_creature_sound(thing, 3, 2, 0);
        cctrl->long_9A = game.play_gameturn;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(thing, CrInst_CELEBRATE_SHORT, 1, 0, 0);
    }
    return 1;
}

short creature_piss(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long i;
    //return _DK_creature_piss(thing);
    cctrl = creature_control_get_from_thing(thing);
    if ( !S3DEmitterIsPlayingSample(thing->snd_emitter_id, 171, 0) ) {
        thing_play_sample(thing, 171, 100, 0, 3, 1, 6, 256);
    }
    i = cctrl->field_282;
    if (i > 0) i--;
    cctrl->field_282 = i;
    if (i > 0) {
        return 1;
    }
    cctrl->field_B2 = game.play_gameturn;
    set_start_state(thing);
    return 0;
}

short mad_killing_psycho(struct Thing *thing)
{
  return _DK_mad_killing_psycho(thing);
}

void anger_set_creature_anger(struct Thing *creatng, long annoy_lv, AnnoyMotive reason)
{
    SYNCDBG(8,"Setting to %d",(int)annoy_lv);
    _DK_anger_set_creature_anger(creatng, annoy_lv, reason);
}

TbBool anger_is_creature_livid(const struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl))
        return false;
    return ((cctrl->mood_flags & CCMoo_Livid) != 0);
}

TbBool anger_is_creature_angry(const struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl))
        return false;
    return ((cctrl->mood_flags & CCMoo_Angry) != 0);
}

AnnoyMotive anger_get_creature_anger_type(const struct Thing *creatng)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    AnnoyMotive anger_type;
    long anger_level;
    long i;
    cctrl = creature_control_get_from_thing(creatng);
    crstat = creature_stats_get_from_thing(creatng);
    if (crstat->annoy_level == 0)
        return AngR_None;
    if ((cctrl->mood_flags & CCMoo_Angry) == 0)
        return AngR_None;
    anger_type = AngR_None;
    anger_level = 0;
    for (i=0; i < 4; i++)
    {
        if (anger_level < cctrl->annoyance_level[i])
        {
            anger_level = cctrl->annoyance_level[i];
            anger_type = i+1;
        }
    }
    if (anger_level < (long)crstat->annoy_level)
        return AngR_None;
    return anger_type;
}

TbBool anger_make_creature_angry(struct Thing *creatng, AnnoyMotive reason)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    crstat = creature_stats_get_from_thing(creatng);
    if ((crstat->annoy_level <= 0) || ((cctrl->mood_flags & CCMoo_Angry) != 0))
        return false;
    anger_set_creature_anger(creatng, crstat->annoy_level, reason);
    return true;
}
/******************************************************************************/
