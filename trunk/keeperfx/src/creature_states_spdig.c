/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_spdig.c
 *     Creature state machine functions for special diggers (imps).
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
#include "thing_traps.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_workshop.h"
#include "spworker_stack.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"

/** Effects used when creating new imps. Every player color has different index. */
const int birth_effect_element[] = { 54, 79, 80, 81, 82, 82, };
const unsigned char reinforce_edges[] = { 3, 0, 0, 3, 0, 1, 2, 2, 1, };

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_imp_arrives_at_convert_dungeon(struct Thing *digger);
DLLIMPORT short _DK_imp_arrives_at_dig_or_mine(struct Thing *digger);
DLLIMPORT short _DK_imp_arrives_at_improve_dungeon(struct Thing *digger);
DLLIMPORT short _DK_imp_arrives_at_reinforce(struct Thing *digger);
DLLIMPORT short _DK_imp_birth(struct Thing *digger);
DLLIMPORT short _DK_imp_converts_dungeon(struct Thing *digger);
DLLIMPORT short _DK_imp_digs_mines(struct Thing *digger);
DLLIMPORT short _DK_imp_doing_nothing(struct Thing *digger);
DLLIMPORT short _DK_imp_drops_gold(struct Thing *digger);
DLLIMPORT short _DK_imp_improves_dungeon(struct Thing *digger);
DLLIMPORT short _DK_imp_last_did_job(struct Thing *digger);
DLLIMPORT short _DK_imp_picks_up_gold_pile(struct Thing *digger);
DLLIMPORT short _DK_imp_reinforces(struct Thing *digger);
DLLIMPORT short _DK_imp_toking(struct Thing *digger);
DLLIMPORT short _DK_creature_pick_up_unconscious_body(struct Thing *digger);
DLLIMPORT short _DK_creature_picks_up_corpse(struct Thing *digger);
DLLIMPORT short _DK_creature_picks_up_spell_object(struct Thing *digger);
DLLIMPORT short _DK_creature_picks_up_trap_for_workshop(struct Thing *digger);
DLLIMPORT short _DK_creature_picks_up_trap_object(struct Thing *digger);
DLLIMPORT short _DK_creature_drops_corpse_in_graveyard(struct Thing *digger);
DLLIMPORT short _DK_creature_drops_crate_in_workshop(struct Thing *digger);
DLLIMPORT short _DK_creature_drops_spell_object_in_library(struct Thing *digger);
DLLIMPORT short _DK_creature_arms_trap(struct Thing *digger);
DLLIMPORT long _DK_check_out_unclaimed_unconscious_bodies(struct Thing *digger, long a1);
DLLIMPORT long _DK_check_out_unclaimed_dead_bodies(struct Thing *digger, long a1);
DLLIMPORT long _DK_check_out_unclaimed_spells(struct Thing *digger, long a1);
DLLIMPORT long _DK_check_out_unclaimed_traps(struct Thing *digger, long a1);
DLLIMPORT long _DK_check_out_unconverted_drop_place(struct Thing *digger);
DLLIMPORT long _DK_check_out_undug_drop_place(struct Thing *digger);
DLLIMPORT long _DK_check_out_unprettied_drop_place(struct Thing *digger);
DLLIMPORT long _DK_check_out_unclaimed_gold(struct Thing *digger, long a1);
DLLIMPORT long _DK_imp_will_soon_be_arming_trap(struct Thing *digger);
DLLIMPORT long _DK_check_out_object_for_trap(struct Thing *traptng, struct Thing *digger);
DLLIMPORT struct Thing *_DK_check_for_empty_trap_for_imp(struct Thing *traptng, long a2);
DLLIMPORT long _DK_imp_will_soon_be_getting_object(long a2, struct Thing *digger);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long check_out_unclaimed_unconscious_bodies(struct Thing *digger, long range)
{
    return _DK_check_out_unclaimed_unconscious_bodies(digger, range);
}

long check_out_unclaimed_dead_bodies(struct Thing *digger, long range)
{
    return _DK_check_out_unclaimed_dead_bodies(digger, range);
}

long check_out_unclaimed_spells(struct Thing *digger, long range)
{
    return _DK_check_out_unclaimed_spells(digger, range);
}

long check_out_unclaimed_traps(struct Thing *digger, long range)
{
    return _DK_check_out_unclaimed_traps(digger, range);
}

long check_out_unconverted_drop_place(struct Thing *thing)
{
    return _DK_check_out_unconverted_drop_place(thing);
}

long check_out_undug_drop_place(struct Thing *thing)
{
    return _DK_check_out_undug_drop_place(thing);
}

long check_out_unclaimed_gold(struct Thing *thing, long a1)
{
    return _DK_check_out_unclaimed_gold(thing, a1);
}

long check_out_unprettied_drop_place(struct Thing *thing)
{
    return _DK_check_out_unprettied_drop_place(thing);
}

long imp_will_soon_be_getting_object(long a2, struct Thing *thing)
{
    return _DK_imp_will_soon_be_getting_object(a2, thing);
}

/** Returns if the player owns any digger who is working on re-arming it.
 *
 * @param traptng The trap that needs re-arming.
 * @return
 */
TbBool imp_will_soon_be_arming_trap(struct Thing *traptng)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    struct CreatureControl *cctrl;
    long crstate;
    long i;
    unsigned long k;
    //return _DK_imp_will_soon_be_arming_trap(digger);
    dungeon = get_dungeon(traptng->owner);
    k = 0;
    i = dungeon->digger_list_start;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per-thing code
        if (cctrl->field_70 == traptng->index)
        {
            crstate = get_creature_state_besides_move(thing);
            if (crstate == CrSt_CreaturePicksUpTrapObject) {
                return true;
            }
            crstate = get_creature_state_besides_drag(thing);
            if (crstate == CrSt_CreatureArmsTrap) {
                return true;
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
    return false;
}

long check_out_object_for_trap(struct Thing *digger, struct Thing *traptng)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    struct SlabMap *slb;
    struct Room *room;
    long find_model,find_owner;
    long i;
    unsigned long k;
    //return _DK_check_out_object_for_trap(digger, traptng);
    cctrl = creature_control_get_from_thing(digger);
    room = get_room_thing_is_on(digger);
    if (room_is_invalid(room)) {
        return 0;
    }
    if ( (room->kind != RoK_WORKSHOP) || (room->owner != digger->owner) ) {
        return 0;
    }
    find_model = trap_to_object[traptng->model];
    find_owner = digger->owner;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing->model == find_model)
        {
            slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            if ( ((slb->field_5 & 0x07) == find_owner) && ((thing->field_1 & 0x01) == 0) )
            {
                if ( !imp_will_soon_be_getting_object(find_owner, thing) )
                {
                    if ( setup_person_move_to_position(digger, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
                    {
                        digger->continue_state = CrSt_CreaturePicksUpTrapObject;
                        cctrl->field_72 = thing->index;
                        cctrl->field_70 = traptng->index;
                        return 1;
                    }
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
    return 0;
}

long check_out_empty_traps(struct Thing *digger, long range)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ( (thing->byte_13 == 0) && (thing->owner == digger->owner) )
        {
            if ( (range < 0) || (get_2d_box_distance(&thing->mappos, &digger->mappos) < range) )
            {

                if ( !imp_will_soon_be_arming_trap(thing) && check_out_object_for_trap(digger, thing) ) {
                    return 1;
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
    return 0;
}

long check_out_unreinforced_drop_place(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    MapSubtlCoord stl_x,stl_y;
    MapSlabCoord slb_x,slb_y;
    long stl_num;
    long pos_x,pos_y;
    long i,n;
    stl_x = thing->mappos.x.stl.num;
    stl_y = thing->mappos.y.stl.num;
    cctrl = creature_control_get_from_thing(thing);
    n = reinforce_edges[3 * (stl_y % 3) + (stl_x % 3)];
    for (i=0; i < 4; i++)
    {
        slb_x = map_to_slab[stl_x] + (long)small_around[n].delta_x;
        slb_y = map_to_slab[stl_y] + (long)small_around[n].delta_y;
        if ( check_place_to_reinforce(thing, slb_x, slb_y) > 0 )
        {
            stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
            if ( check_out_uncrowded_reinforce_position(thing, stl_num, &pos_x, &pos_y) )
            {
                if ( setup_person_move_to_position(thing, pos_x, pos_y, 0) )
                {
                    thing->continue_state = CrSt_ImpArrivesAtReinforce;
                    cctrl->word_8D = stl_num;
                    cctrl->byte_93 = 0;
                    return 1;
                }
            }
        }
        n = (n + 1) % 4;
    }
    return 0;
}

struct Thing *check_for_empty_trap_for_imp(struct Thing *digger, long tngmodel)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    //return _DK_check_for_empty_trap_for_imp(digger, tngmodel);
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ( (thing->model == tngmodel) && (thing->byte_13 == 0) && (thing->owner == digger->owner) )
        {
            if ( !imp_will_soon_be_arming_trap(thing) ) {
                return thing;
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
    return INVALID_THING;
}

long check_out_crates_to_arm_trap_in_room(struct Thing *digger)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    struct Thing *traptng;
    struct Room *room;
    long i;
    unsigned long k;
    cctrl = creature_control_get_from_thing(digger);
    room = get_room_thing_is_on(digger);
    if (room_is_invalid(room)) {
        return 0;
    }
    if ( (room->kind != RoK_WORKSHOP) || (room->owner != digger->owner) ) {
        return 0;
    }

    k = 0;
    i = game.thing_lists[TngList_Objects].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ( thing_is_trap_box(thing) )
        {
          if ( ((thing->field_1 & 0x01) == 0) && (get_room_thing_is_on(thing) == room) )
          {
              traptng = check_for_empty_trap_for_imp(digger, box_thing_to_door_or_trap(thing));
              if (thing_is_invalid(traptng))
              {
                  if ( !imp_will_soon_be_getting_object(digger->owner, thing)
                    && setup_person_move_to_position(digger, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
                  {
                      digger->continue_state = CrSt_CreaturePicksUpTrapObject;
                      cctrl->field_72 = thing->index;
                      cctrl->field_70 = traptng->index;
                      return 1;
                  }
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
    return 0;
}

long check_out_available_imp_drop_tasks(struct Thing *digger)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(digger);

    if ( check_out_unclaimed_unconscious_bodies(digger, 768)
      || check_out_unclaimed_dead_bodies(digger, 768)
      || check_out_unclaimed_spells(digger, 768)
      || check_out_unclaimed_traps(digger, 768)
      || check_out_empty_traps(digger, 768) )
    {
        return 1;
    }
    if ( check_out_undug_drop_place(digger) )
    {
        cctrl->byte_94 = 1;
        return 1;
    }
    if ( check_out_unconverted_drop_place(digger) )
    {
        cctrl->byte_94 = 2;
        return 1;
    }
    if ( check_out_unprettied_drop_place(digger) )
    {
        cctrl->byte_94 = 2;
        return 1;
    }
    if ( check_out_unclaimed_gold(digger, 768) )
    {
        return 1;
    }
    if ( check_out_unreinforced_drop_place(digger) )
    {
        cctrl->byte_94 = 9;
        return 1;
    }
    if ( check_out_crates_to_arm_trap_in_room(digger) )
    {
        return 1;
    }
    cctrl->byte_94 = 0;
    return 0;
}

short imp_arrives_at_convert_dungeon(struct Thing *thing)
{
    //return _DK_imp_arrives_at_convert_dungeon(thing);
    if (check_place_to_convert_excluding(thing,
           map_to_slab[thing->mappos.x.stl.num],
           map_to_slab[thing->mappos.y.stl.num]) )
    {
      internal_set_thing_state(thing, CrSt_ImpConvertsDungeon);
    } else
    {
      internal_set_thing_state(thing, CrSt_ImpLastDidJob);
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
    internal_set_thing_state(thing, CrSt_ImpImprovesDungeon);
  } else
  {
    internal_set_thing_state(thing, CrSt_ImpLastDidJob);
  }
  return 1;
}

short imp_arrives_at_reinforce(struct Thing *thing)
{
  return _DK_imp_arrives_at_reinforce(thing);
}

short imp_birth(struct Thing *thing)
{
    struct CreatureStats *crstat;
    long i;
    //return _DK_imp_birth(thing);
    if ( thing_touching_floor(thing) )
    {
      if (!check_out_available_imp_drop_tasks(thing)) {
          set_start_state(thing);
      }
      return 1;
    }
    i = game.play_gameturn - thing->field_9;
    if ((i % 2) == 0) {
      create_effect_element(&thing->mappos, birth_effect_element[thing->owner], thing->owner);
    }
    crstat = creature_stats_get_from_thing(thing);
    creature_turn_to_face_angle(thing, i * (long)crstat->max_angle_change);
    return 0;
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
      internal_set_thing_state(thing, CrSt_ImpLastDidJob);
      return 1;
    }
    // If gems are marked for digging, but there is too much gold laying around, then don't dig
    if ((slb->kind == SlbT_GEMS) && too_much_gold_lies_around_thing(thing))
    {
      clear_creature_instance(thing);
      internal_set_thing_state(thing, CrSt_ImpLastDidJob);
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
        internal_set_thing_state(thing, CrSt_ImpLastDidJob);
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

short creature_pick_up_unconscious_body(struct Thing *thing)
{
  return _DK_creature_pick_up_unconscious_body(thing);
}

short creature_picks_up_corpse(struct Thing *thing)
{
  return _DK_creature_picks_up_corpse(thing);
}

short creature_picks_up_spell_object(struct Thing *thing)
{
    struct Room *enmroom, *ownroom;
    struct CreatureControl *cctrl;
    struct Thing *spelltng;
    struct Coord3d pos;
    //return _DK_creature_picks_up_spell_object(thing);
    cctrl = creature_control_get_from_thing(thing);
    spelltng = thing_get(cctrl->field_72);
    if ( thing_is_invalid(spelltng) || ((spelltng->field_1 & 0x01) != 0)
      || (get_2d_box_distance(&thing->mappos, &spelltng->mappos) >= 512))
    {
        set_start_state(thing);
        return 0;
    }
    enmroom = subtile_room_get(spelltng->mappos.x.stl.num,spelltng->mappos.y.stl.num);
    ownroom = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, 3, 0, 1);
    if ( room_is_invalid(ownroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(thing, ownroom, &pos) )
    {
        WARNLOG("Player %d can't pick spell - doesn't have proper library to store it",(int)thing->owner);
        set_start_state(thing);
        return 0;
    }
    // Check if we're stealing the spell from a library
    if (!room_is_invalid(enmroom))
    {
        remove_spell_from_library(enmroom, spelltng, thing->owner);
    }
    creature_drag_object(thing, spelltng);
    if (!setup_person_move_to_position(thing, pos.x.stl.num, pos.y.stl.num, 0))
    {
        SYNCDBG(8,"Cannot move to (%d,%d)",(int)pos.x.stl.num, (int)pos.y.stl.num);
    }
    thing->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
    return 1;
}

short creature_picks_up_trap_for_workshop(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *cratetng;
    struct Room *dstroom;
    struct Coord3d pos;
    //return _DK_creature_picks_up_trap_for_workshop(thing);
    // Get the crate thing
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->field_72);
    // Check if everything is right
    if ( thing_is_invalid(cratetng) || ((cratetng->field_1 & 0x01) != 0)
      || (get_2d_box_distance(&thing->mappos, &cratetng->mappos) >= 512) )
    {
        set_start_state(thing);
        return 0;
    }
    // Find room to drag the crate to
    dstroom = find_nearest_room_for_thing_with_spare_item_capacity(thing, thing->owner, 8, 0);
    if ( room_is_invalid(dstroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(thing, dstroom, &pos) )
    {
        set_start_state(thing);
        return 0;
    }
    // Initialize dragging
    if ( !setup_person_move_backwards_to_position(thing, pos.x.stl.num, pos.y.stl.num, 0) )
    {
        set_start_state(thing);
        return 0;
    }
    creature_drag_object(thing, cratetng);
    thing->continue_state = CrSt_CreatureDropsCrateInWorkshop;
    return 1;
}

/** Being in workshop, pick up a trap crate to be dragged to a trap that needs re-arming.
 *
 * @param thing Special worker creature.
 * @return
 */
short creature_picks_up_trap_object(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    struct Thing *cratetng;
    struct Thing *traptng;
    //return _DK_creature_picks_up_trap_object(thing);
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->field_72);
    room = get_room_thing_is_on(cratetng);
    traptng = thing_get(cctrl->field_70);
    if ( !thing_exists(cratetng) || !thing_exists(traptng) )
    {
        cctrl->field_70 = 0;
        set_start_state(thing);
        return 0;
    }
    if ( ((cratetng->field_1 & 0x01) != 0)
      || (get_2d_box_distance(&thing->mappos, &cratetng->mappos) >= 512)
      || (traptng->class_id != TCls_Trap) || (box_thing_to_door_or_trap(cratetng) != traptng->model))
    {
        cctrl->field_70 = 0;
        set_start_state(thing);
        return 0;
    }
    if ( !setup_person_move_backwards_to_position(thing, traptng->mappos.x.stl.num, traptng->mappos.y.stl.num, 0) )
    {
        cctrl->field_70 = 0;
        set_start_state(thing);
        return 0;
    }
    if (!room_is_invalid(room))
    {
        if ( (room->kind == RoK_WORKSHOP) && (room->owner == cratetng->owner) )
        {
            remove_workshop_object_from_workshop(room);
            if (cratetng->owner < 5)
            {
                remove_workshop_item(cratetng->owner,
                    get_workshop_object_class_for_thing(cratetng),
                    box_thing_to_door_or_trap(cratetng));
            }
        }
    }
    creature_drag_object(thing, cratetng);
    thing->continue_state = CrSt_CreatureArmsTrap;
    return 1;
}

short creature_drops_corpse_in_graveyard(struct Thing *thing)
{
  return _DK_creature_drops_corpse_in_graveyard(thing);
}

short creature_drops_crate_in_workshop(struct Thing *thing)
{
    struct Thing *cratetng;
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_creature_drops_crate_in_workshop(thing);
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->field_6E);
    // Check if crate is ok
    if ( !thing_exists(cratetng) )
    {
        set_start_state(thing);
        return 0;
    }
    // Check if we're on correct room
    room = get_room_thing_is_on(thing);
    if ( room_is_invalid(room) )
    {
        set_start_state(thing);
        return 0;
    }

    if ( (room->kind != RoK_WORKSHOP) || (room->owner != thing->owner)
        || (room->used_capacity >= room->total_capacity) )
    {
        set_start_state(thing);
        return 0;
    }
    creature_drop_dragged_object(thing, cratetng);
    cratetng->owner = thing->owner;
    add_workshop_object_to_workshop(room);
    add_workshop_item(room->owner, get_workshop_object_class_for_thing(cratetng),
        box_thing_to_door_or_trap(cratetng));
    set_start_state(thing);
    return 1;
}

short creature_drops_spell_object_in_library(struct Thing *thing)
{
  return _DK_creature_drops_spell_object_in_library(thing);
}

short creature_arms_trap(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *traptng;
    struct Thing *postng;
    struct Thing *cratetng;
    //return _DK_creature_arms_trap(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Creature has invalid control structure!");
        return 0;
    }
    dungeon = get_dungeon(thing->owner);
    cratetng = thing_get(cctrl->field_6E);
    traptng = thing_get(cctrl->field_70);
    if ( !thing_exists(cratetng) || !thing_exists(traptng) )
    {
        set_start_state(thing);
        return 0;
    }
    postng = get_trap_at_subtile_of_model_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, traptng->model, thing->owner);
    // Note that this means there can be only one trap of given kind at a subtile.
    // Otherwise it won't be possible to re-arm it, as the condition below will fail.
    if ( (postng != traptng) || (traptng->byte_13 > 0) )
    {
        ERRORLOG("The %s has moved or been already rearmed",thing_model_name(traptng));
        set_start_state(thing);
        return 0;
    }
    traptng->byte_13 = game.traps_config[traptng->model].shots;
    traptng->field_4F ^= (traptng->field_4F ^ (trap_stats[traptng->model].field_12 << 4)) & 0x30;
    dungeon->lvstats.traps_armed++;
    creature_drop_dragged_object(thing, cratetng);
    delete_thing_structure(cratetng, 0);
    set_start_state(thing);
    return 1;
}

/******************************************************************************/
