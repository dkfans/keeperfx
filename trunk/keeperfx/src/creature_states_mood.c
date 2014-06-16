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
TbBool creature_can_get_angry(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    return (crstat->annoy_level > 0);
}

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
    if (game.play_gameturn - cctrl->last_mood_sound_turn > 32)
    {
        play_creature_sound(thing, CrSnd_Sad, 2, 0);
        cctrl->last_mood_sound_turn = game.play_gameturn;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(thing, CrInst_MOAN, 1, 0, 0);
    }
    return 1;
}

short creature_roar(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_creature_roar(thing);
    cctrl = creature_control_get_from_thing(thing);

    if (cctrl->field_282 > 0) {
        cctrl->field_282--;
    }
    if (cctrl->field_282 <= 0)
    {
        cctrl->last_roar_turn = game.play_gameturn;
        set_start_state(thing);
        return 0;
    }
    if (game.play_gameturn - cctrl->long_9A > 32)
    {
        play_creature_sound(thing, 4, 2, 0);
        cctrl->long_9A = game.play_gameturn;
    }
    return 1;
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
    if (game.play_gameturn - cctrl->last_mood_sound_turn > 32)
    {
        play_creature_sound(thing, CrSnd_Happy, 2, 0);
        cctrl->last_mood_sound_turn = game.play_gameturn;
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
        thing_play_sample(thing, 171, NORMAL_PITCH, 0, 3, 1, 6, FULL_LOUDNESS);
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

short mad_killing_psycho(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    //return _DK_mad_killing_psycho(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    // Find a position for killing - use random dungeon
    struct Coord3d pos;
    int i,n;
    n = ACTION_RANDOM(PLAYERS_COUNT);
    for (i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo *player;
        player = get_player(n);
        if (player_exists(player)) {
            if (get_random_position_in_dungeon_for_creature(n, 1, creatng, &pos)) {
                if (creature_can_navigate_to_with_storage(creatng, &pos, NavRtF_Default)) {
                    break;
                }
            }
        }
        n = (n+1) % PLAYERS_COUNT;
    }
    if (i >= PLAYERS_COUNT)
      return 1;
    if (setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
    {
        if (game.play_gameturn - cctrl->last_roar_turn <= 200)
        {
            creatng->continue_state = CrSt_MadKillingPsycho;
        } else
        {
            cctrl->field_282 = 50;
            creatng->continue_state = CrSt_CreatureRoar;
        }
    } else
    {
        if (game.play_gameturn - cctrl->last_roar_turn > 200)
        {
            cctrl->field_282 = 50;
            internal_set_thing_state(creatng, CrSt_CreatureRoar);
        }
    }
    return 1;
}

void anger_calculate_creature_is_angry(struct Thing *creatng)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    int i;
    cctrl = creature_control_get_from_thing(creatng);
    crstat = creature_stats_get_from_thing(creatng);
    cctrl->mood_flags &= ~CCMoo_Angry;
    cctrl->mood_flags &= ~CCMoo_Livid;
    for (i = 1; i < 5; i++)
    {
        if (crstat->annoy_level <= cctrl->annoyance_level[i])
        {
            cctrl->mood_flags |= CCMoo_Angry;
            if (2*crstat->annoy_level <= cctrl->annoyance_level[i])
            {
                cctrl->mood_flags |= CCMoo_Livid;
                break;
            }
        }
    }
}

TbBool anger_free_for_anger_increase(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->combat_flags != 0) {
        return false;
    }
    return (cctrl->spell_flags & CSAfF_Unkn0800) == 0;
}

TbBool anger_free_for_anger_decrease(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    // If the creature is mad killing, don't allow it not to be angry
    if ((cctrl->spell_flags & CSAfF_MadKilling) != 0) {
        return false;
    }
    return true;
}

void anger_increase_creature_anger(struct Thing *creatng, long anger, AnnoyMotive reason)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (anger_free_for_anger_increase(creatng))
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(creatng->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->lvstats.lies_told++;
        }
        anger_set_creature_anger(creatng, anger + cctrl->annoyance_level[reason], reason);
    }
}

void anger_reduce_creature_anger(struct Thing *creatng, long anger, AnnoyMotive reason)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (anger_free_for_anger_decrease(creatng))
    {
        anger_set_creature_anger(creatng, anger + cctrl->annoyance_level[reason], reason);
    }
}

void anger_set_creature_anger(struct Thing *creatng, long annoy_lv, AnnoyMotive reason)
{
    SYNCDBG(8,"Setting reason %d to %d for %s index %d",(int)reason,(int)annoy_lv,thing_model_name(creatng),(int)creatng->index);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    //_DK_anger_set_creature_anger(creatng, annoy_lv, reason);
    if ((game.numfield_14 != 0) || !creature_can_get_angry(creatng)) {
        return;
    }
    if (annoy_lv < 0)
    {
        annoy_lv = 0;
    } else
    if (annoy_lv > 65534) {
        annoy_lv = 65534;
    }
    TbBool was_angry;
    was_angry = ((cctrl->mood_flags & CCMoo_Angry) != 0);
    cctrl->annoyance_level[reason] = annoy_lv;
    anger_calculate_creature_is_angry(creatng);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(creatng->owner);
    if (dungeon_invalid(dungeon)) {
        return;
    }
    if ((cctrl->mood_flags & CCMoo_Angry) != 0)
    {
        if (!was_angry) {
            dungeon->creatures_annoyed++;
            event_create_event_or_update_nearby_existing_event(
              creatng->mappos.x.val, creatng->mappos.y.val,
              EvKind_CreatrIsAnnoyed, creatng->owner, creatng->index);
        }
    } else
    {
        if (was_angry) {
          if (dungeon->creatures_annoyed > 0) {
              dungeon->creatures_annoyed--;
          } else {
              ERRORLOG("Removing annoyed creature - non to Remove");
          }
        }
    }
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
    for (i=1; i < 5; i++)
    {
        if (anger_level < cctrl->annoyance_level[i])
        {
            anger_level = cctrl->annoyance_level[i];
            anger_type = i;
        }
    }
    if (anger_level < (long)crstat->annoy_level)
        return AngR_None;
    return anger_type;
}

void anger_apply_anger_to_creature_all_types(struct Thing *thing, long anger)
{
    if (!creature_can_get_angry(thing)) {
        return;
    }
    if (anger > 0)
    {
        anger_increase_creature_anger(thing, anger, AngR_Other);
        anger_increase_creature_anger(thing, anger, AngR_NotPaid);
        anger_increase_creature_anger(thing, anger, AngR_NoLair);
        anger_increase_creature_anger(thing, anger, AngR_Hungry);
    } else
    {
        anger_reduce_creature_anger(thing, -anger, AngR_Other);
        anger_reduce_creature_anger(thing, -anger, AngR_NotPaid);
        anger_reduce_creature_anger(thing, -anger, AngR_NoLair);
        anger_reduce_creature_anger(thing, -anger, AngR_Hungry);
    }
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
