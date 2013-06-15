/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file spdigger_stack.c
 *     Special diggers task stack support functions.
 * @par Purpose:
 *     Functions to create and maintain list of tasks for special diggers (imps).
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 04 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "spdigger_stack.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"

#include "creature_states.h"
#include "creature_states_train.h"
#include "map_blocks.h"
#include "dungeon_data.h"
#include "tasks_list.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "thing_corpses.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_traps.h"
#include "room_data.h"
#include "power_hand.h"
#include "map_events.h"
#include "gui_soundmsgs.h"
#include "front_simple.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_imp_stack_update(struct Thing *creatng);
DLLIMPORT long _DK_add_unclaimed_unconscious_bodies_to_imp_stack(struct Dungeon *dungeon, long slb_x);
DLLIMPORT long _DK_add_unclaimed_dead_bodies_to_imp_stack(struct Dungeon *dungeon, long slb_x);
DLLIMPORT long _DK_add_unclaimed_spells_to_imp_stack(struct Dungeon *dungeon, long slb_x);
DLLIMPORT void _DK_add_pretty_and_convert_to_imp_stack(struct Dungeon *dungeon);
DLLIMPORT long _DK_add_unclaimed_gold_to_imp_stack(struct Dungeon *dungeon);
DLLIMPORT long _DK_add_object_for_trap_to_imp_stack(struct Dungeon *dungeon, struct Thing *creatng);
DLLIMPORT long _DK_check_out_imp_stack(struct Thing *creatng);
DLLIMPORT struct Thing *_DK_check_for_empty_trap_for_imp_not_being_armed(struct Thing *creatng, long slb_x);
DLLIMPORT long _DK_imp_will_soon_be_working_at_excluding(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT long _DK_check_out_imp_last_did(struct Thing *creatng);
DLLIMPORT long _DK_check_place_to_convert_excluding(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT long _DK_check_out_unconverted_spiral(struct Thing *creatng, long slb_x);
DLLIMPORT long _DK_check_place_to_pretty_excluding(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT long _DK_check_out_unprettied_spiral(struct Thing *creatng, long slb_x);
DLLIMPORT long _DK_check_out_undug_place(struct Thing *creatng);
DLLIMPORT long _DK_check_out_undug_area(struct Thing *creatng);
DLLIMPORT long _DK_check_out_unprettied_or_unconverted_area(struct Thing *creatng);
DLLIMPORT long _DK_check_out_unreinforced_place(struct Thing *creatng);
DLLIMPORT long _DK_check_out_unreinforced_area(struct Thing *creatng);
DLLIMPORT long _DK_check_out_uncrowded_reinforce_position(struct Thing *creatng, unsigned short slb_x, long *slb_y, long *a4);
DLLIMPORT long _DK_check_place_to_dig_and_get_position(struct Thing *creatng, unsigned short slb_x, long *slb_y, long *a4);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_dead_body(struct Thing *creatng, long stl_x, long stl_y);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_gold(struct Thing *creatng, long stl_x, long stl_y);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_spell(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_unconscious_body(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT long _DK_check_place_to_reinforce(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_crate(struct Thing *creatng, long stl_x, long stl_y);
DLLIMPORT long _DK_add_to_pretty_to_imp_stack_if_need_to(long creatng, long slb_x, struct Dungeon *dungeon);
DLLIMPORT long _DK_imp_will_soon_be_converting_at_excluding(struct Thing *creatng, long slb_x, long slb_y);
/******************************************************************************/
long const dig_pos[] = {0, -1, 1};

/******************************************************************************/
TbBool add_to_imp_stack_using_pos(long stl_num, long task_type, struct Dungeon *dungeon)
{
    struct DiggerStack *istack;
    if (dungeon->digger_stack_length >= IMP_TASK_MAX_COUNT)
        return false;
    istack = &dungeon->imp_stack[dungeon->digger_stack_length];
    dungeon->digger_stack_length++;
    istack->field_0 = stl_num;
    istack->task_id = task_type;
    return (dungeon->digger_stack_length < IMP_TASK_MAX_COUNT);
}

/**
 * Finds a task of given type which concerns given subtile in the current imp stack.
 * @param stl_num
 * @param task_type
 * @param dungeon
 */
long find_in_imp_stack_using_pos(long stl_num, long task_type, const struct Dungeon *dungeon)
{
    long i;
    for (i=0; i < dungeon->digger_stack_length; i++)
    {
        const struct DiggerStack *istack;
        istack = &dungeon->imp_stack[i];
        if ((istack->field_0 == stl_num) && (istack->task_id == task_type)) {
            return i;
        }
    }
    return -1;
}

long imp_will_soon_be_working_at_excluding(struct Thing *thing, long a2, long a3)
{
    SYNCDBG(19,"Starting");
    TRACE_THING(thing);
    return _DK_imp_will_soon_be_working_at_excluding(thing, a2, a3);
}

/** Returns if the player owns any digger who is working on re-arming given trap.
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

void force_any_creature_dragging_owned_thing_to_drop_it(struct Thing *dragtng)
{
    if (thing_is_dragged_or_pulled(dragtng))
    {
        struct Thing *creatng;
        creatng = find_players_creature_dragging_thing(dragtng->owner, dragtng);
        // If found a creature dragging the thing, reset it so it will drop the thing
        if (!thing_is_invalid(creatng)) {
            set_start_state(creatng);
        }
    }
}

struct Thing *check_for_empty_trap_for_imp_not_being_armed(struct Thing *digger, long trpmodel)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    //return _DK_check_for_empty_trap_for_imp_not_being_armed(thing, a2);
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ( (thing->model == trpmodel) && (thing->byte_13 == 0) && (thing->owner == digger->owner) )
        {
            if ( !imp_will_soon_be_arming_trap(thing) )
            {
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

long check_out_unprettied_or_unconverted_area(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct DiggerStack *stck;
    struct Coord3d navpos;
    SYNCDBG(9,"Starting");
    //return _DK_check_out_unprettied_or_unconverted_area(thing);
    dungeon = get_dungeon(thing->owner);
    int min_dist;
    int min_taskid;
    struct Coord3d min_pos;
    MapSubtlCoord srcstl_x, srcstl_y;
    min_dist = 28;
    srcstl_x = thing->mappos.x.stl.num;
    srcstl_y = thing->mappos.y.stl.num;
    int i;
    for (i=0; i < dungeon->digger_stack_length; i++)
    {
        stck = &dungeon->imp_stack[i];
        if ((stck->task_id != 1) && (stck->task_id != 2)) {
            continue;
        }
        MapSubtlCoord stl_x, stl_y;
        MapSlabCoord slb_x, slb_y;
        stl_x = stl_num_decode_x(stck->field_0);
        stl_y = stl_num_decode_y(stck->field_0);
        slb_x = map_to_slab[stl_x];
        slb_y = map_to_slab[stl_y];
        int new_dist;
        new_dist = get_2d_box_distance_xy(srcstl_x, srcstl_y, stl_x, stl_y);
        if (new_dist >= min_dist) {
            continue;
        }
        if (stck->task_id == 1)
        {
            if (!check_place_to_pretty_excluding(thing, slb_x, slb_y)) {
                // Task is no longer valid
                stck->task_id = 0;
                continue;
            }
            if (!imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
            {
                navpos.x.val = subtile_coord_center(stl_x);
                navpos.y.val = subtile_coord_center(stl_y);
                navpos.z.val = get_thing_height_at(thing, &navpos);
                if (creature_can_navigate_to_with_storage(thing, &navpos, 0))
                {
                    min_taskid = 1;
                    min_dist = new_dist;
                    min_pos.x.val = navpos.x.val;
                    min_pos.y.val = navpos.y.val;
                    min_pos.z.val = navpos.z.val;
                }
            }
        } else
        if (stck->task_id == 2)
        {
          if (!check_place_to_convert_excluding(thing, slb_x, slb_y)) {
              // Task is no longer valid
              stck->task_id = 0;
              continue;
          }
          if (!imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
          {
              navpos.x.val = subtile_coord_center(stl_x);
              navpos.y.val = subtile_coord_center(stl_y);
              navpos.z.val = get_thing_height_at(thing, &navpos);
              if (creature_can_navigate_to_with_storage(thing, &navpos, 0))
              {
                  min_taskid = 2;
                  min_dist = new_dist;
                  min_pos.x.val = navpos.x.val;
                  min_pos.y.val = navpos.y.val;
                  min_pos.z.val = navpos.z.val;
              }
          }
        } else
        {
            ERRORLOG("Invalid stack type; cleared");
            stck->task_id = 0;
        }
    }
    if (min_dist == 28)
      return 0;
    if (!setup_person_move_to_coord(thing, &min_pos, 0))
    {
        ERRORLOG("Digger can navigate but not move to.");
        return 0;
    }
    if (min_taskid == 1)
    {
        thing->continue_state = 9;
        return 1;
    } else
    {
        thing->continue_state = 73;
        return 1;
    }
    return 0;
}

long imp_will_soon_be_converting_at_excluding(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    return _DK_imp_will_soon_be_converting_at_excluding(creatng, slb_x, slb_y);
}

TbBool check_out_unconverted_spot(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapSubtlCoord stl_x,stl_y;
    if ((slb_x < 0) || (slb_x >= map_tiles_x)) {
        return false;
    }
    if ((slb_y < 0) || (slb_y >= map_tiles_y)) {
        return false;
    }
    if (!check_place_to_convert_excluding(creatng, slb_x, slb_y)) {
        return false;
    }
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    if (imp_will_soon_be_converting_at_excluding(creatng, stl_x, stl_y)) {
        return false;
    }
    if (!setup_person_move_to_position(creatng, stl_x, stl_y, 0)) {
        return false;
    }
    creatng->continue_state = CrSt_ImpArrivesAtConvertDungeon;
    return true;
}

long check_out_unconverted_spiral(struct Thing *thing, long nslabs)
{
    const struct Around *arnd;
    long slb_x,slb_y;
    long slabi,arndi;
    long i,imax,k;
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);
    //return _DK_check_out_unconverted_spiral(thing, nslabs);

    slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    imax = 2;
    arndi = ACTION_RANDOM(4);
    for (slabi = 0; slabi < nslabs; slabi++)
    {
        {
          arnd = &small_around[arndi];
          {
              slb_x += arnd->delta_x;
              slb_y += arnd->delta_y;
              if (check_out_unconverted_spot(thing, slb_x, slb_y)) {
                  return 1;
              }
          }
          arndi = (arndi + 1) & 3;
          i = 1;
        }
        for (k = 0; k < 4; k++)
        {
          arnd = &small_around[arndi];
          for (; i < imax; i++)
          {
              slb_x += arnd->delta_x;
              slb_y += arnd->delta_y;
              if (check_out_unconverted_spot(thing, slb_x, slb_y)) {
                  return 1;
              }
          }
          arndi = (arndi + 1) & 3;
          i = 0;
        }
        imax += 2;
    }
    return 0;
}

TbBool check_out_unprettied_spot(struct Thing *creatng, long slb_x, long slb_y)
{
    MapSubtlCoord stl_x,stl_y;
    if ((slb_x < 0) || (slb_x >= map_tiles_x)) {
        return false;
    }
    if ((slb_y < 0) || (slb_y >= map_tiles_y)) {
        return false;
    }
    if (!check_place_to_pretty_excluding(creatng, slb_x, slb_y)) {
        return false;
    }
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    if (imp_will_soon_be_working_at_excluding(creatng, stl_x, stl_y)) {
        return false;
    }
    if (!setup_person_move_to_position(creatng, stl_x, stl_y, 0)) {
        return false;
    }
    creatng->continue_state = CrSt_ImpArrivesAtImproveDungeon;
    return true;
}

long check_out_unprettied_spiral(struct Thing *thing, long nslabs)
{
    const struct Around *arnd;
    long slb_x,slb_y;
    long slabi,arndi;
    long i,imax,k;
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);
    //return _DK_check_out_unprettied_spiral(thing, nslabs);

    slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    imax = 2;
    arndi = ACTION_RANDOM(4);
    for (slabi = 0; slabi < nslabs; slabi++)
    {
        {
          arnd = &small_around[arndi];
          {
              slb_x += arnd->delta_x;
              slb_y += arnd->delta_y;
              if (check_out_unprettied_spot(thing, slb_x, slb_y))
              {
                  return 1;
              }
          }
          arndi = (arndi + 1) & 3;
          i = 1;
        }
        for (k = 0; k < 4; k++)
        {
          arnd = &small_around[arndi];
          for (; i < imax; i++)
          {
              slb_x += arnd->delta_x;
              slb_y += arnd->delta_y;
              if (check_out_unprettied_spot(thing, slb_x, slb_y))
              {
                  return 1;
              }
          }
          arndi = (arndi + 1) & 3;
          i = 0;
        }
        imax += 2;
    }
    return 0;
}

long check_place_to_convert_excluding(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb;
    PlayerNumber prev_owner;
    TRACE_THING(creatng);
    slb = get_slabmap_block(slb_x, slb_y);
    prev_owner = slabmap_owner(slb);
    if (prev_owner == creatng->owner)
        return 0;
    if (players_are_mutual_allies(creatng->owner, prev_owner)) {
        SYNCDBG(8,"The slab %d,%d is owned by ally, so cennot be converted",(int)slb_x, (int)slb_y);
        return 0;
    }
    //return _DK_check_place_to_convert_excluding(creatng, slb_x, slb_y);

    struct Room *room;
    room = room_get(slb->room_index);
    if ((slb->kind != SlbT_CLAIMED) && (room_is_invalid(room) || (room->kind == RoK_DUNGHEART))) {
        SYNCDBG(8,"The slab %d,%d is not a valid type to be converted",(int)slb_x, (int)slb_y);
        return 0;
    }
    struct Map *mapblk;
    mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (!map_block_revealed(mapblk, creatng->owner)) {
        SYNCDBG(8,"The slab %d,%d is not revealed",(int)slb_x, (int)slb_y);
        return 0;
    }
    if (!slab_by_players_land(creatng->owner, slb_x, slb_y)) {
        SYNCDBG(8,"The slab %d,%d is not by players land",(int)slb_x, (int)slb_y);
        return 0;
    }
    struct Thing *thing;
    long i;
    unsigned long k;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ( thing_is_creature(thing) && (thing->index != creatng->index) )
        {
            if (!thing_is_picked_up(thing) && (thing->active_state == CrSt_ImpConvertsDungeon)) {
                SYNCDBG(8,"The slab %d,%d is already being converted by %s index %d",
                    (int)slb_x,(int)slb_y,thing_model_name(thing),(int)thing->index);
                return 0;
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return 1;
}

long check_place_to_pretty_excluding(struct Thing *creatng, long slb_x, long slb_y)
{
    struct SlabMap *slb;
    SYNCDBG(19,"Starting");
    TRACE_THING(creatng);
    //return _DK_check_place_to_pretty_excluding(creatng, slb_x, slb_y);
    slb = get_slabmap_block(slb_x, slb_y);
    if (slb->kind != SlbT_PATH) {
        SYNCDBG(8,"The slab %d,%d is not a valid type",(int)slb_x, (int)slb_y);
        return 0;
    }
    struct Map *mapblk;
    mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (!map_block_revealed(mapblk, creatng->owner)) {
        SYNCDBG(8,"The slab %d,%d is not revealed",(int)slb_x, (int)slb_y);
        return 0;
    }
    if (!slab_by_players_land(creatng->owner, slb_x, slb_y)) {
        SYNCDBG(8,"The slab %d,%d is not by players land",(int)slb_x, (int)slb_y);
        return 0;
    }
    struct Thing *thing;
    long i;
    unsigned long k;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ( thing_is_creature(thing) && (thing->index != creatng->index) )
        {
            if (!thing_is_picked_up(thing) && (thing->active_state == CrSt_ImpImprovesDungeon)) {
                SYNCDBG(8,"The slab %d,%d is already being improved by %s index %d",
                    (int)slb_x,(int)slb_y,thing_model_name(thing),(int)thing->index);
                return 0;
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return 1;
}

long check_out_unreinforced_place(struct Thing *thing)
{
  return _DK_check_out_unreinforced_place(thing);
}

long check_out_unreinforced_area(struct Thing *thing)
{
  return _DK_check_out_unreinforced_area(thing);
}

TbBool check_out_unconverted_place(struct Thing *thing)
{
    long stl_x,stl_y;
    long slb_x,slb_y;
    SYNCDBG(19,"Starting");
    TRACE_THING(thing);
    slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    if ( check_place_to_convert_excluding(thing, slb_x, slb_y)
      && !imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y) )
    {
        if (setup_person_move_to_position(thing, stl_x, stl_y, 0))
        {
            thing->continue_state = CrSt_ImpArrivesAtConvertDungeon;
            return true;
        }
    }
    if ( check_out_unconverted_spiral(thing, 1) )
    {
        return true;
    }
    return false;
}

long check_out_unprettied_place(struct Thing *thing)
{
  long stl_x,stl_y;
  long slb_x,slb_y;
  SYNCDBG(19,"Starting");
  TRACE_THING(thing);
  slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
  slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
  stl_x = slab_subtile_center(slb_x);
  stl_y = slab_subtile_center(slb_y);
  if ( check_place_to_pretty_excluding(thing, slb_x, slb_y)
    && !imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y) )
  {
      if (setup_person_move_to_position(thing, stl_x, stl_y, 0))
      {
          thing->continue_state = CrSt_ImpArrivesAtImproveDungeon;
          return true;
      }
  }
  if ( check_out_unprettied_spiral(thing, 1) )
  {
      return true;
  }
  return false;
}

long check_out_undug_place(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    MapSubtlCoord base_stl_x,base_stl_y;
    long i,n;
    SYNCDBG(19,"Starting");
    //return _DK_check_out_undug_place(thing);
    cctrl = creature_control_get_from_thing(thing);
    base_stl_x = stl_num_decode_x(cctrl->word_8F);
    base_stl_y = stl_num_decode_y(cctrl->word_8F);
    n = ACTION_RANDOM(4);
    for (i=0; i < 4; i++)
    {
        struct MapTask* mtask;
        SubtlCodedCoords task_pos;
        MapSlabCoord slb_x,slb_y;
        long task_idx;
        slb_x = subtile_slab_fast(base_stl_x)+small_around[n].delta_x;
        slb_y = subtile_slab_fast(base_stl_y)+small_around[n].delta_y;
        task_pos = get_subtile_number(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        task_idx = find_dig_from_task_list(thing->owner, task_pos);
        if (task_idx != -1)
        {
            long mv_x,mv_y;
            mv_x = 0; mv_y = 0;
            if (check_place_to_dig_and_get_position(thing, task_pos, &mv_x, &mv_y)
                && setup_person_move_to_position(thing, mv_x, mv_y, 0))
            {
                cctrl->word_91 = task_idx;
                cctrl->word_8F = task_pos;
                mtask = get_task_list_entry(thing->owner, cctrl->word_91);
                if (mtask->kind == SDDigTask_MineGold)
                {
                  thing->continue_state = CrSt_ImpArrivesAtMineGold;
                } else
                {
                  thing->continue_state = CrSt_ImpArrivesAtDigDirt;
                }
                return 1;
            }
        }
        n = (n + 1) % 4;
    }
    return 0;
}

long check_out_undug_area(struct Thing *thing)
{
    SYNCDBG(19,"Starting");
    return _DK_check_out_undug_area(thing);
}

long add_undug_to_imp_stack(struct Dungeon *dungeon, long num)
{
    struct MapTask* mtask;
    long stl_x, stl_y;
    long i,nused;
    SYNCDBG(18,"Starting");
    nused = 0;
    i = -1;
    while ((num > 0) && (dungeon->digger_stack_length < IMP_TASK_MAX_COUNT))
    {
        i = find_next_dig_in_dungeon_task_list(dungeon, i);
        if (i < 0)
            break;
        mtask = get_dungeon_task_list_entry(dungeon, i);
        stl_x = stl_num_decode_x(mtask->field_1);
        stl_y = stl_num_decode_y(mtask->field_1);
        if ( subtile_revealed(stl_x, stl_y, dungeon->owner) )
        {
          if ( block_has_diggable_side(dungeon->owner, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)) )
          {
            add_to_imp_stack_using_pos(mtask->field_1, DigTsk_DigOrMine, dungeon);
            num--;
            nused++;
          }
        }
    }
    return nused;
}

TbBool add_to_reinforce_stack(long slb_x, long slb_y, long task_id)
{
    if (r_stackpos >= 64) {
        return false;
    }
    struct DiggerStack *rfstack;
    rfstack = &reinforce_stack[r_stackpos];
    r_stackpos++;
    rfstack->field_0 = get_subtile_number_at_slab_center(slb_x, slb_y);
    rfstack->task_id = task_id;
    return true;
}

long add_to_pretty_to_imp_stack_if_need_to(long a1, long a2, struct Dungeon *dungeon)
{
    return _DK_add_to_pretty_to_imp_stack_if_need_to(a1, a2, dungeon);
}

struct ExtraSquares spdigger_extra_squares[] = {
    { 0,  0x00},
    { 0,  0x00},
    { 0,  0x00},
    { 1, ~0x03},
    { 0,  0x00},
    { 0,  0x00},
    { 2, ~0x06},
    { 1, ~0x01},
    { 0,  0x00},
    { 4, ~0x09},
    { 0,  0x00},
    { 1, ~0x02},
    { 3, ~0x0C},
    { 3, ~0x04},
    { 2, ~0x02},
    { 1,  0x00},
};

struct Around spdigger_extra_positions[] = {
    { 0, 0},
    { 1,-1},
    { 1, 1},
    {-1, 1},
    {-1,-1},
};

long add_pretty_and_convert_to_imp_stack_starting_from_pos(struct Dungeon *dungeon, const struct Coord3d * start_pos)
{
    unsigned char *slbopt;
    struct SlabCoord *slblist;
    unsigned int slblicount;
    unsigned int slblipos;
    slbopt = scratch;
    slblist = (struct SlabCoord *)(scratch + map_tiles_x*map_tiles_y);
    MapSlabCoord slb_x, slb_y;
    for (slb_y=0; slb_y < map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < map_tiles_x; slb_x++)
        {
            SlabCodedCoords slb_num;
            struct SlabMap *slb;
            slb_num = get_slab_number(slb_x, slb_y);
            slb = get_slabmap_direct(slb_num);
            struct SlabAttr *slbattr;
            slbattr = get_slab_attrs(slb);
            slbopt[slb_num] = ((slbattr->flags & 0x29) != 0);
        }
    }
    slblipos = 0;
    slblicount = 0;
    MapSlabCoord base_slb_x, base_slb_y;
    base_slb_x = start_pos->x.stl.num / 3;
    base_slb_y = start_pos->y.stl.num / 3;
    SlabCodedCoords slb_num;
    slb_num = get_slab_number(base_slb_x, base_slb_y);
    slbopt[slb_num] |= 0x02;
    do
    {
        unsigned char around_flags;
        around_flags = 0;

        long i,n;
        n = 0;//ACTION_RANDOM(4);
        for (i=0; i < SMALL_AROUND_COUNT; i++)
        {
            slb_x = base_slb_x + (long)small_around[n].delta_x;
            slb_y = base_slb_y + (long)small_around[n].delta_y;
            slb_num = get_slab_number(slb_x, slb_y);
            // Per around code
            if ((slbopt[slb_num] & 0x01) != 0)
            {
                struct SlabMap *slb;
                around_flags |= (1<<n);
                slbopt[slb_num] |= 0x02;
                slb = get_slabmap_direct(slb_num);
                if ( 64 - dungeon->digger_stack_length > r_stackpos )
                {
                  if (slab_kind_is_friable_dirt(slb->kind))
                  {
                      struct Map *mapblk;
                      mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
                      if ( (mapblk->data >> 28) & (1 << dungeon->owner) )
                      {
                          if ( slab_by_players_land(dungeon->owner, slb_x, slb_y) )
                          {
                              add_to_reinforce_stack(slb_x, slb_y, 3);
                          }
                      }
                  }
                }
            } else
            if ((slbopt[slb_num] & 0x02) == 0)
            {
                slbopt[slb_num] |= 0x02;
                slblist[slblicount].x = slb_x;
                slblist[slblicount].y = slb_y;
                slblicount++;
                if ( !add_to_pretty_to_imp_stack_if_need_to(slb_x, slb_y, dungeon) ) {
                    SYNCDBG(6,"Can't add any more pretty tasks");
                    return slblipos;
                }
            }
            // Per around code ends
            n = (n + 1) % 4;
        }

        struct ExtraSquares  *square;
        for (square = &spdigger_extra_squares[around_flags]; square->index != 0; square = &spdigger_extra_squares[around_flags])
        {
            if (around_flags == 0x0F)
            {
                // If whole around is to be set, just do it in one go
                for (i=1; i < 5; i++)
                {
                    slb_x = base_slb_x + (long)spdigger_extra_positions[i].delta_x;
                    slb_y = base_slb_y + (long)spdigger_extra_positions[i].delta_y;
                    slb_num = get_slab_number(slb_x, slb_y);
                    slbopt[slb_num] |= 0x02;
                }
                around_flags = 0;
            } else
            {
                i = square->index;
                {
                    slb_x = base_slb_x + (long)spdigger_extra_positions[i].delta_x;
                    slb_y = base_slb_y + (long)spdigger_extra_positions[i].delta_y;
                    slb_num = get_slab_number(slb_x, slb_y);
                    slbopt[slb_num] |= 0x02;
                }
                around_flags &= square->flgmask;
            }
        }
        base_slb_x = slblist[slblipos].x;
        base_slb_y = slblist[slblipos].y;
        slblipos++;
    }
    while (slblipos <= slblicount);
    return slblipos;
}


void add_pretty_and_convert_to_imp_stack(struct Dungeon *dungeon)
{
    if (dungeon->digger_stack_length >= 64) {
        WARNLOG("Too many jobs, no place for more");
        return;
    }
    SYNCDBG(18,"Starting");
    //TODO SPDIGGER This restricts convert tasks to the area connected to heart, instead of connected to diggers.
    //_DK_add_pretty_and_convert_to_imp_stack(dungeon); return;
    struct Thing *heartng;
    heartng = INVALID_THING;
    if (!dungeon_invalid(dungeon))
        heartng = thing_get(dungeon->dnheart_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng)) {
        WARNLOG("Dungeon has no heart, no dungeon position available");
        return;
    }
    add_pretty_and_convert_to_imp_stack_starting_from_pos(dungeon, &heartng->mappos);
}

long add_unclaimed_gold_to_imp_stack(struct Dungeon *dungeon)
{
  return _DK_add_unclaimed_gold_to_imp_stack(dungeon);
}

void setup_imp_stack(struct Dungeon *dungeon)
{
  long i;
  for (i = 0; i < dungeon->digger_stack_length; i++)
  {
    dungeon->imp_stack[i].task_id = DigTsk_None;
  }
  dungeon->digger_stack_update_turn = game.play_gameturn;
  dungeon->digger_stack_length = 0;
  r_stackpos = 0;
}

long add_unclaimed_unconscious_bodies_to_imp_stack(struct Dungeon *dungeon, long a2)
{
  return _DK_add_unclaimed_unconscious_bodies_to_imp_stack(dungeon, a2);
}

TbBool add_unclaimed_dead_bodies_to_imp_stack(struct Dungeon *dungeon, long max_tasks)
{
    struct Thing *thing;
    struct Room *room;
    SubtlCodedCoords stl_num;
    int remain_num;
    unsigned long k;
    int i;
    //return _DK_add_unclaimed_dead_bodies_to_imp_stack(dungeon, max_tasks);
    if (!dungeon_has_room(dungeon, RoK_GRAVEYARD)) {
        SYNCDBG(8,"Dungeon %d has no graveyard",(int)dungeon->owner);
        return 1;
    }
    room = find_room_with_spare_capacity(dungeon->owner, RoK_GRAVEYARD, 1);
    k = 0;
    i = game.thing_lists[TCls_DeadCreature].index;
    remain_num = max_tasks;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        if ( (dungeon->digger_stack_length >= IMP_TASK_MAX_COUNT) || (remain_num <= 0) ) {
            break;
        }
        if ( ((thing->field_1 & TF1_IsDragged1) == 0) && (thing->active_state == DCrSt_Unknown02)
           && (thing->byte_14 == 0) && corpse_is_rottable(thing) )
        {
            if (room_is_invalid(room))
            {
                SYNCDBG(8,"Dungeon %d has no free graveyard space",(int)dungeon->owner);
                if (is_my_player_number(dungeon->owner)) {
                    output_message(SMsg_GraveyardTooSmall, 1000, true);
                }
                return 0;
            }
            if ( subtile_revealed(thing->mappos.x.stl.num,thing->mappos.y.stl.num,dungeon->owner) )
            {
                stl_num = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
                add_to_imp_stack_using_pos(stl_num, DigTsk_PickUpCorpse, dungeon);
                remain_num--;
            }
        }
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return 1;
}

long add_unclaimed_spells_to_imp_stack(struct Dungeon *dungeon, long a2)
{
  return _DK_add_unclaimed_spells_to_imp_stack(dungeon, a2);
}

TbBool add_object_for_trap_to_imp_stack(struct Dungeon *dungeon, struct Thing *boxtng)
{
    struct Thing *thing;
    unsigned long k;
    int i;
    //return _DK_add_object_for_trap_to_imp_stack(dungeon, thing);
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
    while (i > 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing->model == trap_to_object[boxtng->model])
        {
            struct SlabMap *slb;
            slb = get_slabmap_thing_is_on(thing);
            if (slabmap_owner(slb) == dungeon->owner)
            {
                SubtlCodedCoords stl_num;
                stl_num = get_subtile_number(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
                if (find_in_imp_stack_using_pos(stl_num, DigTsk_PicksUpTrapBox, dungeon) == -1)
                {
                    add_to_imp_stack_using_pos(stl_num, DigTsk_PicksUpTrapBox, dungeon);
                    return true;
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
    return false;
}

TbBool add_empty_traps_to_imp_stack(struct Dungeon *dungeon, long num)
{
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Thing list loop body
        if ((num <= 0) || (dungeon->digger_stack_length >= IMP_TASK_MAX_COUNT))
          break;
        if ((thing->byte_13 == 0) && (thing->owner == dungeon->owner))
        {
            if ( add_object_for_trap_to_imp_stack(dungeon, thing) ) {
                num--;
            }
        }
        // Thing list loop body ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(19,"Finished");
    return true;
}

TbBool add_unclaimed_traps_to_imp_stack(struct Dungeon *dungeon)
{
  struct SlabMap* slb;
  struct Room* room;
  unsigned long stl_num;
  struct Thing* thing;
  unsigned long k;
  int i;
  SYNCDBG(18,"Starting");
  // Checking if the workshop exists
  room = find_room_with_spare_room_item_capacity(dungeon->owner, RoK_WORKSHOP);
  if ( !dungeon_has_room(dungeon, RoK_WORKSHOP) || room_is_invalid(room) )
    return false;
  k = 0;
  i = game.thing_lists[TngList_Objects].index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    // Thing list loop body
    if (dungeon->digger_stack_length >= IMP_TASK_MAX_COUNT)
      break;
    if ( thing_is_door_or_trap_box(thing) )
    {
        if ((thing->field_1 & TF1_IsDragged1) == 0)
        {
            if ((thing->owner == dungeon->owner) || (thing->owner == game.neutral_player_num))
            {
                slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
                if (slabmap_owner(slb) == dungeon->owner)
                {
                    room = get_room_thing_is_on(thing);
                    if (room_is_invalid(room) || (room->kind != RoK_WORKSHOP))
                    {
                      stl_num = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
                      add_to_imp_stack_using_pos(stl_num, DigTsk_PicksUpTrapForWorkshop, dungeon);
                    }
                }
            }
        }
    }
    // Thing list loop body ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  SYNCDBG(19,"Finished");
  return true;
}

void add_reinforce_to_imp_stack(struct Dungeon *dungeon)
{
    struct DiggerStack *rfstack;
    long i;
    for (i=0; i < r_stackpos; i++)
    {
        if (dungeon->digger_stack_length >= IMP_TASK_MAX_COUNT)
          break;
        rfstack = &reinforce_stack[i];
        add_to_imp_stack_using_pos(rfstack->field_0, rfstack->task_id, dungeon);
    }
}

long check_out_uncrowded_reinforce_position(struct Thing *thing, unsigned short a2, long *a3, long *a4)
{
    return _DK_check_out_uncrowded_reinforce_position(thing, a2, a3, a4);
}

long check_place_to_dig_and_get_position(struct Thing *thing, SubtlCodedCoords stl_num, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y)
{
    struct SlabMap *place_slb;
    struct Coord3d pos;
    MapSubtlCoord place_x,place_y;
    long distance_x,distance_y;
    long base_x,base_y;
    long stl_x,stl_y;
    long i,k,n,nstart;
    SYNCDBG(18,"Starting");
    //return _DK_check_place_to_dig_and_get_position(thing, stl_num, retstl_x, retstl_y);
    place_x = stl_num_decode_x(stl_num);
    place_y = stl_num_decode_y(stl_num);
    if (!block_has_diggable_side(thing->owner, subtile_slab_fast(place_x), subtile_slab_fast(place_y)))
        return 0;
    distance_x = place_x - thing->mappos.x.stl.num;
    distance_y = place_y - thing->mappos.y.stl.num;
    if (abs(distance_y) >= abs(distance_x))
    {
      if (distance_y > 0)
          nstart = 0;
      else
          nstart = 2;
    } else
    {
      if (distance_x > 0)
          nstart = 3;
      else
          nstart = 1;
    }
    place_slb = get_slabmap_for_subtile(place_x,place_y);
    n = nstart;

    for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
    {
      base_x = place_x + 2 * (long)small_around[n].delta_x;
      base_y = place_y + 2 * (long)small_around[n].delta_y;
      if (valid_dig_position(thing->owner, base_x, base_y))
      {
          for (k = 0; k < sizeof(dig_pos)/sizeof(dig_pos[0]); k++)
          {
              if ( k )
              {
                nstart = ((n + dig_pos[k]) & 3);
                stl_x = base_x + small_around[nstart].delta_x;
                stl_y = base_y + small_around[nstart].delta_y;
              } else
              {
                stl_x = base_x;
                stl_y = base_y;
              }
              if (valid_dig_position(thing->owner, stl_x, stl_y))
              {
                    if ((place_slb->kind != SlbT_GEMS) || !gold_pile_with_maximum_at_xy(stl_x, stl_y))
                      if (!imp_already_digging_at_excluding(thing, stl_x, stl_y))
                        if (!imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
                        {
                          set_coords_to_subtile_center(&pos, stl_x, stl_y, 0);
                          pos.z.val = get_thing_height_at(thing, &pos);
                          if (creature_can_navigate_to_with_storage(thing, &pos, 0))
                          {
                              *retstl_x = stl_x;
                              *retstl_y = stl_y;
                              return 1;
                          }
                        }
              }
          }
      }
      n = (n+1) % 4;
    }
    return 0;
}

struct Thing *check_place_to_pickup_dead_body(struct Thing *thing, long stl_x, long stl_y)
{
    return _DK_check_place_to_pickup_dead_body(thing, stl_x, stl_y);
}

struct Thing *check_place_to_pickup_gold(struct Thing *thing, long stl_x, long stl_y)
{
    return _DK_check_place_to_pickup_gold(thing, stl_x, stl_y);
}

struct Thing *check_place_to_pickup_spell(struct Thing *thing, long a2, long a3)
{
    return _DK_check_place_to_pickup_spell(thing, a2, a3);
}

struct Thing *check_place_to_pickup_unconscious_body(struct Thing *thing, long a2, long a3)
{
    return _DK_check_place_to_pickup_unconscious_body(thing, a2, a3);
}

long check_place_to_reinforce(struct Thing *thing, long a2, long a3)
{
    return _DK_check_place_to_reinforce(thing, a2, a3);
}

struct Thing *check_place_to_pickup_crate(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long n)
{
    //return _DK_check_place_to_pickup_crate(thing, stl_x, stl_y);
    struct Map *mapblk;
    long i;
    unsigned long k;
    mapblk = get_map_block_at(stl_x,stl_y);
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(creatng);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing_is_door_or_trap_box(thing))
        {
          if ((thing->owner == creatng->owner) || is_neutral_thing(thing))
          {
            if ((thing->field_1 & TF1_IsDragged1) == 0) {
                if (n > 0) {
                    n--;
                } else {
                    return thing;
                }
            }
          }
        }
        // Per thing code end
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
 * Checks if given digger has money that should be placed in treasure room.
 * If he does, he is ordered to return them into nearest treasure room
 * which has the proper capacity. If there's no proper treasure room,
 * a proper speech message is created.
 * @param thing The digger creature.
 * @return Gives 1 if the digger was ordered to go into treasure room, 0 otherwise.
 */
long check_out_imp_has_money_for_treasure_room(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct Room *room;
    SYNCDBG(8,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    //If the imp doesn't have any money - then just return
    if (thing->creature.gold_carried <= 0) {
        return 0;
    }
    // Find a treasure room to drop the money
    room = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_TREASURE, 0, 1);
    if (!room_is_invalid(room))
    {
        if (setup_head_for_empty_treasure_space(thing, room))
        {
            thing->continue_state = CrSt_ImpDropsGold;
            return 1;
        }
        return 0;
    }
    dungeon = get_dungeon(thing->owner);
    // Check why the treasure room search failed. Maybe we don't have treasure room?
    if (!dungeon_has_room(dungeon, RoK_TREASURE))
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_RoomTreasrNeeded, 1000, true);
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_NeedTreasureRoom, thing->owner, 0);
        return 0;
    }
    // If we have it, is it unreachable, or just too small?
    room = find_room_with_spare_capacity(thing->owner, RoK_TREASURE, 1);
    if (room_is_invalid(room))
    {
        // No room with spare capacity - treasury is too small
        if (is_my_player_number(thing->owner))
            output_message(SMsg_TreasuryTooSmall, 1000, true);
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_TreasureRoomFull, thing->owner, 0);
    } else
    {
        // There are rooms with spare capacity - they must be just unreachable
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoRouteToTreasury, 1000, true);
    }
    return 0;
}

long check_out_available_imp_tasks(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    SYNCDBG(9,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    imp_stack_update(thing);
    if ( check_out_imp_stack(thing) ) {
        return 1;
    }
    if (game.play_gameturn-cctrl->field_2C7 > 128)
    {
        check_out_imp_has_money_for_treasure_room(thing);
        cctrl->field_2C7 = game.play_gameturn;
        return 1;
    }
    return 0;
}

long check_out_imp_tokes(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long i;
    SYNCDBG(19,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    i = ACTION_RANDOM(64);
    // small chance of changing state
    if (i != 0)
      return 0;
    internal_set_thing_state(thing, CrSt_ImpToking);
    thing->continue_state = CrSt_ImpDoingNothing;
    cctrl->field_282 = 200;
    return 1;
}

long check_out_imp_last_did(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct Dungeon *dungeon;
  struct Room *room;
  //return _DK_check_out_imp_last_did(thing);
  cctrl = creature_control_get_from_thing(thing);
  SYNCDBG(19,"Starting for %s index %d, last did %d",thing_model_name(thing),(int)thing->index,(int)cctrl->digger.last_did_job);
  TRACE_THING(thing);
  switch (cctrl->digger.last_did_job)
  {
  case 0:
      return false;
  case 1:
      if ( check_out_undug_place(thing) || check_out_undug_area(thing) )
      {
          cctrl->digger.last_did_job = 1;
          return true;
      }
      if ( check_out_unconverted_place(thing) || check_out_unprettied_place(thing) )
      {
          cctrl->digger.last_did_job = 2;
          SYNCDBG(19,"Done on unprettied or unconverted place 1");
          return true;
      }
      imp_stack_update(thing);
      if ( check_out_unprettied_or_unconverted_area(thing) )
      {
          cctrl->digger.last_did_job = 2;
          SYNCDBG(9,"Done on unprettied or unconverted area 1");
          return true;
      }
      break;
  case 2:
      if ( check_out_unconverted_place(thing) || check_out_unprettied_place(thing) )
      {
          cctrl->digger.last_did_job = 2;
          SYNCDBG(19,"Done on unprettied or unconverted place 2");
          return true;
      }
      imp_stack_update(thing);
      if ( check_out_unprettied_or_unconverted_area(thing) )
      {
        cctrl->digger.last_did_job = 2;
        SYNCDBG(9,"Done on unprettied or unconverted area 2");
        return true;
      }
      if ( check_out_undug_area(thing) )
      {
        cctrl->digger.last_did_job = 1;
        return true;
      }
      break;
  case 3:
      dungeon = get_dungeon(thing->owner);
      imp_stack_update(thing);
      if ((dungeon->digger_stack_update_turn != cctrl->digger.stack_update_turn) && (dungeon->digger_stack_length != 3))
        break;
      if ( check_out_unreinforced_place(thing) )
      {
        cctrl->digger.last_did_job = 3;
        return true;
      }
      if ( check_out_unreinforced_area(thing) )
      {
        cctrl->digger.last_did_job = 3;
        return true;
      }
      break;
  case 4:
      if ( !creature_can_be_trained(thing) || !player_can_afford_to_train_creature(thing) )
        break;
      room = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_TRAINING, 0, 1);
      if (!room_is_invalid(room))
      {
          if ( setup_random_head_for_room(thing, room, 0) )
          {
            thing->continue_state = CrSt_AtTrainingRoom;
            cctrl->target_room_id = room->index;
            return true;
          }
      }
      if (is_my_player_number(thing->owner))
      {
        if ( !find_room_with_spare_capacity(thing->owner, RoK_TRAINING, 1) )
          output_message(SMsg_TrainingTooSmall, 0, true);
      }
      break;
  case 9:
      if ( check_out_unreinforced_place(thing) )
      {
        cctrl->digger.last_did_job = 9;
        return true;
      }
      if ( check_out_unreinforced_area(thing) )
      {
        cctrl->digger.last_did_job = 9;
        return true;
      }
      break;
  default:
      break;
  }
  cctrl->digger.last_did_job = 0;
  SYNCDBG(9,"No job found");
  return false;
}

long imp_stack_update(struct Thing *thing)
{
    struct Dungeon *dungeon;
    SYNCDBG(18,"Starting");
    //return _DK_imp_stack_update(thing);
    dungeon = get_dungeon(thing->owner);
    if ((game.play_gameturn - dungeon->digger_stack_update_turn) < 128)
        return 0;
    SYNCDBG(8,"Updating");
    setup_imp_stack(dungeon);
    add_unclaimed_unconscious_bodies_to_imp_stack(dungeon, IMP_TASK_MAX_COUNT/4 - 1);
    add_unclaimed_dead_bodies_to_imp_stack(dungeon, IMP_TASK_MAX_COUNT/4 - 1);
    add_unclaimed_spells_to_imp_stack(dungeon, IMP_TASK_MAX_COUNT/12);
    add_empty_traps_to_imp_stack(dungeon, IMP_TASK_MAX_COUNT/6);
    add_undug_to_imp_stack(dungeon, IMP_TASK_MAX_COUNT*5/8);
    add_pretty_and_convert_to_imp_stack(dungeon);
    add_unclaimed_gold_to_imp_stack(dungeon);
    add_unclaimed_traps_to_imp_stack(dungeon);
    add_reinforce_to_imp_stack(dungeon);
    return 1;
}

long check_out_worker_improve_dungeon(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    SYNCDBG(18,"Starting");
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if ( !check_place_to_pretty_excluding(thing, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)) )
    {
        istack->task_id = DigTsk_None;
        return 0;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    thing->continue_state = CrSt_ImpArrivesAtImproveDungeon;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->digger.last_did_job = 2;
    return 1;
}

long check_out_worker_convert_dungeon(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if (!check_place_to_convert_excluding(thing, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)))
    {
        istack->task_id = DigTsk_None;
        return 0;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    thing->continue_state = CrSt_ImpArrivesAtConvertDungeon;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->digger.last_did_job = 2;
    return 1;
}

long check_out_worker_reinforce_wall(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    struct CreatureControl *cctrl;
    SYNCDBG(18,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    if (game.play_gameturn - cctrl->field_2C7 > 128)
    {
        cctrl->field_95--;
        check_out_imp_has_money_for_treasure_room(thing);
        cctrl->field_2C7 = game.play_gameturn;
        return 1;
    }
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if (check_place_to_reinforce(thing, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)) <= 0)
    {
        istack->task_id = DigTsk_None;
        return 0;
    }
    if (!check_out_uncrowded_reinforce_position(thing, istack->field_0, &stl_x, &stl_y))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0) )
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    thing->continue_state = CrSt_ImpArrivesAtReinforce;
    cctrl->digger.byte_93 = 0;
    cctrl->word_8D = istack->field_0;
    cctrl->digger.last_did_job = 3;
    return 1;
}

long check_out_worker_pickup_unconscious(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    SYNCDBG(18,"Starting");
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if (!player_has_room(thing->owner, RoK_PRISON)) {
        return 0;
    }
    if (!player_creature_tends_to(thing->owner, CrTend_Imprison)) {
        return 0;
    }
    struct Room * room;
    room = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_PRISON, 0, 1);
    if (room_is_invalid(room))
    {
      if (is_my_player_number(thing->owner))
      {
          room = find_room_with_spare_capacity(thing->owner, RoK_PRISON, 1);
          if (room_is_invalid(room)) {
              output_message(SMsg_PrisonTooSmall, 1000, true);
          }
      }
      istack->task_id = DigTsk_None;
      return -1;
    }
    struct Thing *sectng;
    sectng = check_place_to_pickup_unconscious_body(thing, stl_x, stl_y);
    if (thing_is_invalid(sectng))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0) )
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    thing->continue_state = CrSt_CreaturePickUpUnconsciousBody;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->pickup_creature_id = sectng->index;
    return 1;
}

long check_out_worker_pickup_corpse(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if (!player_has_room(thing->owner, RoK_GRAVEYARD)) {
        return 0;
    }
    struct Room * room;
    room = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_GRAVEYARD, 0, 1);
    if (room_is_invalid(room))
    {
        if (is_my_player_number(thing->owner))
        {
            room = find_room_with_spare_capacity(thing->owner, RoK_GRAVEYARD, 1);
            if (room_is_invalid(room)) {
                output_message(SMsg_GraveyardTooSmall, 1000, true);
            }
        }
        istack->task_id = DigTsk_None;
        return -1;
    }
    struct Thing *sectng;
    sectng = check_place_to_pickup_dead_body(thing, stl_x, stl_y);
    if (thing_is_invalid(sectng))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    thing->continue_state = CrSt_CreaturePicksUpCorpse;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->pickup_object_id = sectng->index;
    return 1;
}

long check_out_worker_pickup_spellbook(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if (!player_has_room(thing->owner, RoK_LIBRARY)) {
        return 0;
    }
    if (!find_nearest_room_for_thing_with_spare_item_capacity(thing, thing->owner, RoK_LIBRARY, 0))
    {
        if (is_my_player_number(thing->owner))
        {
          if (!find_room_with_spare_room_item_capacity(thing->owner, RoK_LIBRARY))
            output_message(SMsg_LibraryTooSmall, 1000, true);
        }
        istack->task_id = DigTsk_None;
        return -1;
    }
    struct Thing *sectng;
    sectng = check_place_to_pickup_spell(thing, stl_x, stl_y);
    if (thing_is_invalid(sectng))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0) )
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (thing_is_spellbook(sectng))
    {
        event_create_event_or_update_nearby_existing_event(
            get_subtile_center_pos(stl_x), get_subtile_center_pos(stl_y),
            EvKind_SpellPickedUp, thing->owner, sectng->index);
    } else
    if (thing_is_special_box(sectng))
    {
        event_create_event_or_update_nearby_existing_event(
            get_subtile_center_pos(stl_x), get_subtile_center_pos(stl_y),
            EvKind_DnSpecialFound, thing->owner, sectng->index);
    } else
    {
        WARNLOG("Strange pickup (model %d) - no event",(int)sectng->model);
    }
    thing->continue_state = CrSt_CreaturePicksUpSpellObject;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->pickup_object_id = sectng->index;
    return 1;
}

long check_out_worker_pickup_trapbox(struct Thing *creatng, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    struct Thing *cratng;
    struct Thing *trdtng;
    long n;
    for (n=0; true; n++)
    {
        cratng = check_place_to_pickup_crate(creatng, stl_x, stl_y, n);
        if (thing_is_invalid(cratng)) {
            trdtng = INVALID_THING;
            break;
        }
        // Allow only trap boxes on that subtile which have a corresponding trap to be armed
        if (thing_is_trap_box(cratng))
        {
            trdtng = check_for_empty_trap_for_imp_not_being_armed(creatng, box_thing_to_door_or_trap(cratng));
            if (!thing_is_invalid(trdtng)) {
                break;
            }
        }
    }
    if (thing_is_invalid(cratng))
    {
        istack->task_id = DigTsk_None;
        return 0;
    }
    if (imp_will_soon_be_working_at_excluding(creatng, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(creatng, stl_x, stl_y, 0))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    creatng->continue_state = CrSt_CreaturePicksUpTrapObject;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->pickup_object_id = cratng->index;
    cctrl->field_70 = trdtng->index;
    return 1;
}

long check_out_worker_pickup_trap_for_workshop(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    long i;
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if (!player_has_room(thing->owner, RoK_WORKSHOP)) {
        return 0;
    }
    if (!find_nearest_room_for_thing_with_spare_item_capacity(thing, thing->owner, RoK_WORKSHOP, 0))
    {
      if (is_my_player_number(thing->owner))
      {
        if (!find_room_with_spare_room_item_capacity(thing->owner, RoK_WORKSHOP))
          output_message(SMsg_WorkshopTooSmall, 1000, true);
      }
      istack->task_id = DigTsk_None;
      return -1;
    }
    struct Thing *sectng;
    sectng = check_place_to_pickup_crate(thing, stl_x, stl_y, 0);
    if (thing_is_invalid(sectng))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    i = workshop_object_class[sectng->model];
    if ( i == 8 )
    {
      event_create_event_or_update_nearby_existing_event(
          get_subtile_center_pos(stl_x), get_subtile_center_pos(stl_y),
          EvKind_TrapCrateFound, thing->owner, sectng->index);
    } else
    if (i == 9)
    {
      event_create_event_or_update_nearby_existing_event(
          get_subtile_center_pos(stl_x), get_subtile_center_pos(stl_y),
          EvKind_DoorCrateFound, thing->owner, sectng->index);
    } else
    {
        WARNLOG("Strange pickup (class %d) - no event",(int)i);
    }
    thing->continue_state = CrSt_CreaturePicksUpTrapForWorkshop;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->pickup_object_id = sectng->index;
    return 1;
}

long check_out_worker_pickup_dig_or_mine(struct Thing *thing, struct DiggerStack *istack)
{
    MapSubtlCoord stl_x,stl_y;
    long i;
    i = find_dig_from_task_list(thing->owner, istack->field_0);
    if (i == -1)
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    stl_x = 0; stl_y = 0;
    if (!check_place_to_dig_and_get_position(thing, istack->field_0, &stl_x, &stl_y)
      || !setup_person_move_to_position(thing, stl_x, stl_y, 0))
    {
      istack->task_id = DigTsk_None;
      return -1;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->word_91 = i;
    cctrl->word_8F = istack->field_0;
    cctrl->digger.last_did_job = 1;
    struct MapTask *task;
    task = get_task_list_entry(thing->owner, i);
    if (task->kind == SDDigTask_MineGold)
    {
      thing->continue_state = CrSt_ImpArrivesAtMineGold;
    } else
    {
      thing->continue_state = CrSt_ImpArrivesAtDigDirt;
    }
    return 1;
}

long check_out_worker_pickup_gold_pile(struct Thing *thing, struct DiggerStack *istack)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    if (crstat->gold_hold <= thing->creature.gold_carried)
    {
        if (game.play_gameturn - cctrl->field_2C7 > 128)
        {
          check_out_imp_has_money_for_treasure_room(thing);
          cctrl->field_2C7 = game.play_gameturn;
        }
        return 1;
    }
    MapSubtlCoord stl_x,stl_y;
    stl_x = stl_num_decode_x(istack->field_0);
    stl_y = stl_num_decode_y(istack->field_0);
    if (!check_place_to_pickup_gold(thing, stl_x, stl_y))
    {
        istack->task_id = DigTsk_None;
        return 0;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, 0))
    {
        istack->task_id = DigTsk_None;
        return -1;
    }
    thing->continue_state = CrSt_ImpPicksUpGoldPile;
    return 1;
}

long check_out_imp_stack(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct DiggerStack *istack;
    long ret;
    SYNCDBG(18,"Starting");
    //return _DK_check_out_imp_stack(thing);
    cctrl = creature_control_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (cctrl->digger.stack_update_turn != dungeon->digger_stack_update_turn)
    {
      cctrl->digger.stack_update_turn = dungeon->digger_stack_update_turn;
      cctrl->field_95 = 0;
    }
    if (dungeon->digger_stack_length > IMP_TASK_MAX_COUNT)
    {
        ERRORLOG("Imp tasks length %d out of range",(int)dungeon->digger_stack_length);
        dungeon->digger_stack_length = IMP_TASK_MAX_COUNT;
    }
    while (cctrl->field_95 < dungeon->digger_stack_length)
    {
        istack = &dungeon->imp_stack[cctrl->field_95];
        cctrl->field_95++;
        SYNCDBG(18,"Checking task %d",(int)istack->task_id);
        switch (istack->task_id)
        {
        case DigTsk_ImproveDungeon:
            ret = check_out_worker_improve_dungeon(thing, istack);
            break;
        case DigTsk_ConvertDungeon:
            ret = check_out_worker_convert_dungeon(thing, istack);
            break;
        case DigTsk_ReinforceWall:
            ret = check_out_worker_reinforce_wall(thing, istack);
            break;
        case DigTsk_PickUpUnconscious:
            ret = check_out_worker_pickup_unconscious(thing, istack);
            break;
        case DigTsk_PickUpCorpse:
            ret = check_out_worker_pickup_corpse(thing, istack);
            break;
        case DigTsk_PicksUpSpellBook:
            ret = check_out_worker_pickup_spellbook(thing, istack);
            break;
        case DigTsk_PicksUpTrapBox:
            ret = check_out_worker_pickup_trapbox(thing, istack);
            break;
        case DigTsk_PicksUpTrapForWorkshop:
            ret = check_out_worker_pickup_trap_for_workshop(thing, istack);
            break;
        case DigTsk_DigOrMine:
            ret = check_out_worker_pickup_dig_or_mine(thing, istack);
            break;
        case DigTsk_PicksUpGoldPile:
            ret = check_out_worker_pickup_gold_pile(thing, istack);
            break;
        case DigTsk_None:
            ret = 0;
            break;
        default:
            ret = 0;
            ERRORLOG("Invalid stack type, %d",(int)istack->task_id);
            istack->task_id = DigTsk_None;
            break;
        }
        if (ret != 0) {
            return ret;
        }
    }
    return 0;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
