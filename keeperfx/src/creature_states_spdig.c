/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_spdig.c
 *     Creature state machine functions for special diggers (imps).
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_spdig.h"
#include "globals.h"

#include "bflib_math.h"
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
#include "spworker_stack.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_imp_arrives_at_convert_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_arrives_at_dig_or_mine(struct Thing *thing);
DLLIMPORT short _DK_imp_arrives_at_improve_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_arrives_at_reinforce(struct Thing *thing);
DLLIMPORT short _DK_imp_birth(struct Thing *thing);
DLLIMPORT short _DK_imp_converts_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_digs_mines(struct Thing *thing);
DLLIMPORT short _DK_imp_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_imp_drops_gold(struct Thing *thing);
DLLIMPORT short _DK_imp_improves_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_last_did_job(struct Thing *thing);
DLLIMPORT short _DK_imp_picks_up_gold_pile(struct Thing *thing);
DLLIMPORT short _DK_imp_reinforces(struct Thing *thing);
DLLIMPORT short _DK_imp_toking(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short imp_arrives_at_convert_dungeon(struct Thing *thing)
{
    //return _DK_imp_arrives_at_convert_dungeon(thing);
    if (check_place_to_convert_excluding(thing,
           map_to_slab[thing->mappos.x.stl.num],
           map_to_slab[thing->mappos.y.stl.num]) )
    {
      internal_set_thing_state(thing, 74);
    } else
    {
      internal_set_thing_state(thing, 8);
    }
    return 1;
}

short imp_arrives_at_dig_or_mine(struct Thing *thing)
{
  return _DK_imp_arrives_at_dig_or_mine(thing);
}

short imp_arrives_at_improve_dungeon(struct Thing *thing)
{
  //return _DK_imp_arrives_at_improve_dungeon(thing);
  if ( check_place_to_pretty_excluding(thing,
          map_to_slab[thing->mappos.x.stl.num],
          map_to_slab[thing->mappos.y.stl.num]) )
  {
    internal_set_thing_state(thing, 10);
  } else
  {
    internal_set_thing_state(thing, 8);
  }
  return 1;
}

short imp_arrives_at_reinforce(struct Thing *thing)
{
  return _DK_imp_arrives_at_reinforce(thing);
}

short imp_birth(struct Thing *thing)
{
  return _DK_imp_birth(thing);
}

short imp_converts_dungeon(struct Thing *thing)
{
  return _DK_imp_converts_dungeon(thing);
}

TbBool too_much_gold_lies_around_thing(struct Thing *thing)
{
  return gold_pile_with_maximum_at_xy(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

short imp_digs_mines(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct MapTask* mtask;
    struct SlabMap *slb;
    struct Coord3d pos;
    MapSubtlCoord stl_x,stl_y;
    long delta_x,delta_y;
    SYNCDBG(19,"Starting");
    // return _DK_imp_digs_mines(thing);
    cctrl = creature_control_get_from_thing(thing);
    mtask = get_task_list_entry(thing->owner, cctrl->word_91);
    stl_x = stl_num_decode_x(cctrl->word_8F);
    stl_y = stl_num_decode_y(cctrl->word_8F);
    slb = get_slabmap_for_subtile(stl_x, stl_y);

    // Check if we've arrived at the destination
    delta_x = abs(thing->mappos.x.stl.num - cctrl->moveto_pos.x.stl.num);
    delta_y = abs(thing->mappos.y.stl.num - cctrl->moveto_pos.y.stl.num);
    if ((mtask->field_1 != cctrl->word_8F) || (delta_x >= 1) || (delta_y >= 1))
    {
      clear_creature_instance(thing);
      internal_set_thing_state(thing, 8);
      return 1;
    }
    // If gems are marked for digging, but there is too much gold laying around, then don't dig
    if ((slb->kind == SlbT_GEMS) && too_much_gold_lies_around_thing(thing))
    {
      clear_creature_instance(thing);
      internal_set_thing_state(thing, 8);
      return 1;
    }
    // Turn to the correct direction to do the task
    pos.x.stl.num = stl_x;
    pos.y.stl.num = stl_y;
    pos.x.stl.pos = 128;
    pos.y.stl.pos = 128;
    if (creature_turn_to_face(thing, &pos))
    {
      return 1;
    }

    if (mtask->field_0 == 0)
    {
        clear_creature_instance(thing);
        internal_set_thing_state(thing, 8);
        return 1;
    }

    if (cctrl->field_D2 == 0)
    {
        set_creature_instance(thing, 30, 0, 0, 0);
    }

    if (mtask->field_0 == 2)
    {
        crstat = creature_stats_get_from_thing(thing);
        // If the creature holds more gold than its able
        if (thing->long_13 > crstat->gold_hold)
        {
          if (game.play_gameturn - cctrl->field_2C7 > 128)
          {
            if (check_out_imp_has_money_for_treasure_room(thing))
              return 1;
            cctrl->field_2C7 = game.play_gameturn;
          }

          drop_gold_pile(thing->long_13 - crstat->gold_hold, &thing->mappos);
          thing->long_13 = crstat->gold_hold;
        }
    }
    return 1;
}

short imp_doing_nothing(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    SYNCDBG(19,"Starting");
    //return _DK_imp_doing_nothing(thing);
    if (!thing_is_creature_special_digger(thing))
    {
        ERRORLOG("Non digger thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (game.play_gameturn-cctrl->long_9A <= 1)
        return 1;
    if (check_out_imp_last_did(thing))
        return 1;
    if (check_out_available_imp_tasks(thing))
        return 1;
    if (check_out_imp_tokes(thing))
        return 1;
    if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
    {
        thing->continue_state = CrSt_ImpDoingNothing;
        return 1;
    }
    dungeon->lvstats.promises_broken++;
    return 1;
}

short imp_drops_gold(struct Thing *thing)
{
  return _DK_imp_drops_gold(thing);
}

short imp_improves_dungeon(struct Thing *thing)
{
  return _DK_imp_improves_dungeon(thing);
}

short imp_last_did_job(struct Thing *thing)
{
    //return _DK_imp_last_did_job(thing);
    if (check_out_imp_last_did(thing))
    {
        return 1;
    } else
    {
        set_start_state(thing);
        return 0;
    }
}

short imp_picks_up_gold_pile(struct Thing *thing)
{
  return _DK_imp_picks_up_gold_pile(thing);
}

short imp_reinforces(struct Thing *thing)
{
  return _DK_imp_reinforces(thing);
}

short imp_toking(struct Thing *thing)
{
  return _DK_imp_toking(thing);
}

/******************************************************************************/
