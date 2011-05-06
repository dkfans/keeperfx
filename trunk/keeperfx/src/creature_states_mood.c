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
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_creature_moan(struct Thing *thing);
DLLIMPORT short _DK_creature_roar(struct Thing *thing);
DLLIMPORT short _DK_creature_be_happy(struct Thing *thing);
DLLIMPORT short _DK_creature_piss(struct Thing *thing);
DLLIMPORT short _DK_mad_killing_psycho(struct Thing *thing);
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
      if (cctrl->field_D2 == 0)
        set_start_state(thing);
      return 0;
    }
    if (game.play_gameturn - cctrl->long_9A > 32)
    {
      play_creature_sound(thing, 4, 2, 0);
      cctrl->long_9A = game.play_gameturn;
    }
    if (cctrl->field_D2 == 0) {
        set_creature_instance(thing, 44, 1, 0, 0);
    }
    return 1;
}

short creature_roar(struct Thing *thing)
{
  return _DK_creature_roar(thing);
}

short creature_be_happy(struct Thing *thing)
{
  return _DK_creature_be_happy(thing);
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

/******************************************************************************/
