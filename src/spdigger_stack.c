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
#include "creature_states_combt.h"
#include "creature_states_train.h"
#include "map_blocks.h"
#include "dungeon_data.h"
#include "tasks_list.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_terrain.h"
#include "thing_corpses.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_traps.h"
#include "room_data.h"
#include "room_util.h"
#include "room_list.h"
#include "room_jobs.h"
#include "power_hand.h"
#include "map_utils.h"
#include "map_events.h"
#include "ariadne_wallhug.h"
#include "gui_soundmsgs.h"
#include "front_simple.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_check_out_unreinforced_place(struct Thing *creatng);
DLLIMPORT long _DK_check_out_unreinforced_area(struct Thing *creatng);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_gold(struct Thing *creatng, long stl_x, long stl_y);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_spell(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT struct Thing *_DK_check_place_to_pickup_unconscious_body(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT long _DK_imp_will_soon_be_converting_at_excluding(struct Thing *creatng, long slb_x, long slb_y);
DLLIMPORT long _DK_imp_already_reinforcing_at_excluding(struct Thing *creatng, long stl_x, long stl_y);
/******************************************************************************/
long const dig_pos[] = {0, -1, 1};

/******************************************************************************/
/**
 * Returns if given digger needs to have its task revised due to recent digger tasks list update.
 * The revision of tasks is done by check_out_imp_stack() function.
 * @param creatng The special digger creature to be checked.
 * @return
 * @see check_out_imp_stack()
 */
TbBool creature_task_needs_check_out_after_digger_stack_change(const struct Thing *creatng)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    dungeon = get_dungeon(creatng->owner);
    cctrl = creature_control_get_from_thing(creatng);
    return (dungeon->digger_stack_update_turn != cctrl->digger.stack_update_turn);
}

/**
 * Adds task to imp stack. Returns if the stack still has free space.
 * @param stl_num Map position related to the task.
 * @param task_type Type of the task.
 * @param dungeon The dungeon to which task is to be added.
 * @return True if there is still free slot on the stack after adding, false otherwise.
 */
TbBool add_to_imp_stack_using_pos(SubtlCodedCoords stl_num, SpDiggerTaskType task_type, struct Dungeon *dungeon)
{
    struct DiggerStack *dstack;
    SYNCDBG(19,"Task %d at %d,%d",(int)task_type,(int)stl_num_decode_x(stl_num),(int)stl_num_decode_y(stl_num));
    if (dungeon->digger_stack_length >= DIGGER_TASK_MAX_COUNT)
        return false;
    dstack = &dungeon->digger_stack[dungeon->digger_stack_length];
    dungeon->digger_stack_length++;
    dstack->stl_num = stl_num;
    dstack->task_type = task_type;
    return (dungeon->digger_stack_length < DIGGER_TASK_MAX_COUNT);
}

/**
 * Finds a task of given type which concerns given subtile in the current imp stack.
 * @param stl_num
 * @param task_type
 * @param dungeon
 */
long find_in_imp_stack_using_pos(SubtlCodedCoords stl_num, SpDiggerTaskType task_type, const struct Dungeon *dungeon)
{
    long i;
    for (i=0; i < dungeon->digger_stack_length; i++)
    {
        const struct DiggerStack *dstack;
        dstack = &dungeon->digger_stack[i];
        if ((dstack->stl_num == stl_num) && (dstack->task_type == task_type)) {
            return i;
        }
    }
    return -1;
}

long find_in_imp_stack_task_other_than_starting_at(SpDiggerTaskType excl_task_type, long start_pos, const struct Dungeon *dungeon)
{
    long i;
    long n;
    long stack_len;
    stack_len = dungeon->digger_stack_length;
    n = start_pos;
    for (i=0; i < stack_len; i++)
    {
        const struct DiggerStack *dstack;
        dstack = &dungeon->digger_stack[n];
        if (dstack->task_type != excl_task_type) {
            return n;
        }
        n = (n+1) % stack_len;
    }
    return -1;
}

long find_in_imp_stack_starting_at(SpDiggerTaskType task_type, long start_pos, const struct Dungeon *dungeon)
{
    long i;
    long n;
    long stack_len;
    stack_len = dungeon->digger_stack_length;
    n = start_pos;
    for (i=0; i < stack_len; i++)
    {
        const struct DiggerStack *dstack;
        dstack = &dungeon->digger_stack[n];
        if (dstack->task_type == task_type) {
            return n;
        }
        n = (n+1) % stack_len;
    }
    return -1;
}

void remove_task_from_all_other_players_digger_stacks(PlayerNumber skip_plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    PlayerNumber plyr_idx;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (plyr_idx == skip_plyr_idx) {
            continue;
        }
        long task_id;
        task_id = find_from_task_list(plyr_idx, get_subtile_number(stl_x, stl_y));
        if (task_id >= 0)
        {
            remove_from_task_list(plyr_idx, task_id);
            if (is_my_player_number(plyr_idx)) {
                pretty_map_remove_flags_and_update(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
            }
        }
    }
}

TbBool imp_will_soon_be_working_at_excluding(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    SYNCDBG(19,"Starting");
    TRACE_THING(thing);
    //return _DK_imp_will_soon_be_working_at_excluding(creatng, stl_x, stl_y);
    struct Coord3d pos2;
    pos2.x.val = subtile_coord_center(stl_x);
    pos2.y.val = subtile_coord_center(stl_y);
    pos2.z.val = subtile_coord(1,0);
    struct Dungeon *dungeon;
    unsigned long k;
    int i;
    dungeon = get_players_num_dungeon(creatng->owner);
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        struct CreatureControl *cctrl;
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_being_unconscious(thing) && !creature_is_dying(thing))
        {
            if (thing->index != creatng->index)
            {
              if ((cctrl->moveto_pos.x.stl.num == stl_x) && (cctrl->moveto_pos.y.stl.num == stl_y))
              {
                  MapCoordDelta dist_other;
                  MapCoordDelta dist_creatng;
                  dist_other = get_2d_box_distance(&thing->mappos, &pos2);
                  dist_creatng = get_2d_box_distance(&creatng->mappos, &pos2);
                  if (dist_other <= dist_creatng)
                      return true;
                  if (dist_other - dist_creatng <= subtile_coord(6,0))
                      return true;
              }
            }
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    return false;
}

TbBool imp_will_soon_be_getting_object(PlayerNumber plyr_idx, const struct Thing *objtng)
{
    const struct Thing *spdigtng;
    const struct CreatureControl *cctrl;
    const struct Dungeon *dungeon;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    dungeon = get_players_num_dungeon(plyr_idx);
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        spdigtng = thing_get(i);
        TRACE_THING(spdigtng);
        cctrl = creature_control_get_from_thing(spdigtng);
        if (thing_is_invalid(spdigtng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->pickup_object_id == objtng->index)
        {
            CrtrStateId crstate;
            crstate = get_creature_state_besides_interruptions(spdigtng);
            if (crstate == CrSt_CreaturePicksUpTrapObject)
                return true;
            if (crstate == CrSt_CreatureArmsTrap)
                return true;
            if (crstate == CrSt_CreaturePicksUpCrateForWorkshop)
                return true;
            if (crstate == CrSt_CreaturePicksUpSpellObject)
                return true;
            // Note that picking up gold pile does not currently fill pickup_object_id, so can't be checked here
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
    return false;
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
        if (cctrl->arming_thing_id == traptng->index)
        {
            crstate = get_creature_state_besides_interruptions(thing);
            if (crstate == CrSt_CreaturePicksUpTrapObject) {
                return true;
            }
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

void force_any_creature_dragging_thing_to_drop_it(struct Thing *dragtng)
{
    TRACE_THING(dragtng);
    if (thing_is_dragged_or_pulled(dragtng))
    {
        struct Thing *creatng;
        creatng = find_creature_dragging_thing(dragtng);
        // If found a creature dragging the thing, reset it so it will drop the thing
        if (!thing_is_invalid(creatng)) {
            SYNCDBG(8,"Reset %s index %d",thing_model_name(creatng),(int)creatng->index);
            set_start_state(creatng);
        } else {
            WARNDBG(4,"Can't find creature dragging %s index %d",thing_model_name(dragtng),(int)dragtng->index);
        }
    }
}

struct Thing *check_for_empty_trap_for_imp_not_being_armed(struct Thing *digger, long trpmodel)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Trap);
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ( (thing->model == trpmodel) && (thing->trap.num_shots == 0) && (thing->owner == digger->owner) )
        {
            if ( !imp_will_soon_be_arming_trap(thing) )
            {
                return thing;
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
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
    struct DiggerStack *dstack;
    struct Coord3d navpos;
    SYNCDBG(9,"Starting");
    dungeon = get_dungeon(thing->owner);
    int min_dist;
    int min_taskid;
    struct Coord3d min_pos;
    MapSubtlCoord srcstl_x;
    MapSubtlCoord srcstl_y;
    min_dist = 28;
    srcstl_x = thing->mappos.x.stl.num;
    srcstl_y = thing->mappos.y.stl.num;
    int i;
    for (i=0; i < dungeon->digger_stack_length; i++)
    {
        dstack = &dungeon->digger_stack[i];
        if ((dstack->task_type != DigTsk_ImproveDungeon) && (dstack->task_type != DigTsk_ConvertDungeon)) {
            continue;
        }
        MapSubtlCoord stl_x;
        MapSubtlCoord stl_y;
        MapSlabCoord slb_x;
        MapSlabCoord slb_y;
        stl_x = stl_num_decode_x(dstack->stl_num);
        stl_y = stl_num_decode_y(dstack->stl_num);
        slb_x = map_to_slab[stl_x];
        slb_y = map_to_slab[stl_y];
        int new_dist;
        new_dist = get_2d_box_distance_xy(srcstl_x, srcstl_y, stl_x, stl_y);
        if (new_dist >= min_dist) {
            continue;
        }
        if (dstack->task_type == DigTsk_ImproveDungeon)
        {
            if (!check_place_to_pretty_excluding(thing, slb_x, slb_y)) {
                // Task is no longer valid
                dstack->task_type = DigTsk_None;
                continue;
            }
            if (!imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
            {
                navpos.x.val = subtile_coord_center(stl_x);
                navpos.y.val = subtile_coord_center(stl_y);
                navpos.z.val = get_thing_height_at(thing, &navpos);
                if (creature_can_navigate_to_with_storage(thing, &navpos, NavRtF_Default))
                {
                    min_taskid = 1;
                    min_dist = new_dist;
                    min_pos.x.val = navpos.x.val;
                    min_pos.y.val = navpos.y.val;
                    min_pos.z.val = navpos.z.val;
                }
            }
        } else
        if (dstack->task_type == DigTsk_ConvertDungeon)
        {
          if (!check_place_to_convert_excluding(thing, slb_x, slb_y)) {
              // Task is no longer valid
              dstack->task_type = DigTsk_None;
              continue;
          }
          if (!imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
          {
              navpos.x.val = subtile_coord_center(stl_x);
              navpos.y.val = subtile_coord_center(stl_y);
              navpos.z.val = get_thing_height_at(thing, &navpos);
              if (creature_can_navigate_to_with_storage(thing, &navpos, NavRtF_Default))
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
            dstack->task_type = DigTsk_None;
        }
    }
    if (min_dist == 28)
      return 0;
    if (!setup_person_move_to_coord(thing, &min_pos, NavRtF_Default))
    {
        ERRORLOG("Digger can navigate but not move to.");
        return 0;
    }
    if (min_taskid == 1)
    {
        thing->continue_state = CrSt_ImpArrivesAtImproveDungeon;
        return 1;
    } else
    {
        thing->continue_state = CrSt_ImpArrivesAtConvertDungeon;
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
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    if ((slb_x < 0) || (slb_x >= map_tiles_x)) {
        return false;
    }
    if ((slb_y < 0) || (slb_y >= map_tiles_y)) {
        return false;
    }
    if (!check_place_to_convert_excluding(creatng, slb_x, slb_y))
    {
        return false;
    }
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    if (imp_will_soon_be_converting_at_excluding(creatng, stl_x, stl_y)) {
        return false;
    }
    if (!setup_person_move_to_position(creatng, stl_x, stl_y, NavRtF_Default)) {
        return false;
    }
    creatng->continue_state = CrSt_ImpArrivesAtConvertDungeon;
    return true;
}

long check_out_unconverted_spiral(struct Thing *thing, long nslabs)
{
    const struct Around *arnd;
    long slb_x;
    long slb_y;
    long slabi;
    long arndi;
    long i;
    long imax;
    long k;
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);

    slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    imax = 2;
    arndi = CREATURE_RANDOM(thing, 4);
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
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
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
    if (!setup_person_move_to_position(creatng, stl_x, stl_y, NavRtF_Default)) {
        return false;
    }
    creatng->continue_state = CrSt_ImpArrivesAtImproveDungeon;
    return true;
}

long check_out_unprettied_spiral(struct Thing *thing, long nslabs)
{
    const struct Around *arnd;
    long slb_x;
    long slb_y;
    long slabi;
    long arndi;
    long i;
    long imax;
    long k;
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);

    slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    imax = 2;
    arndi = CREATURE_RANDOM(thing, 4);
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
        SYNCDBG(8,"The slab %d,%d is owned by ally, so cannot be converted",(int)slb_x, (int)slb_y);
        return 0;
    }

    struct Room *room;
    room = room_get(slb->room_index);
    if ((slb->kind != SlbT_CLAIMED) && (room_is_invalid(room) || (room->kind == RoK_DUNGHEART))) {
        SYNCDBG(8,"The slab %d,%d is not a valid kind %d to be converted",(int)slb_x, (int)slb_y, (int)slb->kind);
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
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return 1;
}

long check_place_to_pretty_excluding(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb;
    SYNCDBG(19,"Starting");
    TRACE_THING(creatng);
    slb = get_slabmap_block(slb_x, slb_y);
    if (slb->kind != SlbT_PATH) {
        SYNCDBG(8,"The slab %d,%d is not a valid kind %d",(int)slb_x, (int)slb_y, (int)slb->kind);
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
            break_mapwho_infinite_chain(mapblk);
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
    long stl_x;
    long stl_y;
    long slb_x;
    long slb_y;
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    TRACE_THING(thing);
    slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    if (check_place_to_convert_excluding(thing, slb_x, slb_y)
      && !imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        if (setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
        {
            thing->continue_state = CrSt_ImpArrivesAtConvertDungeon;
            return true;
        }
    }
    if (check_out_unconverted_spiral(thing, 1))
    {
        return true;
    }
    return false;
}

long check_out_unprettied_place(struct Thing *thing)
{
    long stl_x;
    long stl_y;
    long slb_x;
    long slb_y;
    SYNCDBG(19, "Starting for %s index %d", thing_model_name(thing), (int)thing->index);
    TRACE_THING(thing);
    slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    if (check_place_to_pretty_excluding(thing, slb_x, slb_y) && !imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        if (setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
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

/**
 * Checks if given special digger is digging a slab which cannot be destroyed.
 * @param creatng
 * @return
 */
TbBool is_digging_indestructible_place(const struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab_fast(stl_num_decode_x(cctrl->digger.task_stl));
    slb_y = subtile_slab_fast(stl_num_decode_y(cctrl->digger.task_stl));
    SYNCDBG(19,"Starting for %s index %d at %d,%d",thing_model_name(creatng),(int)creatng->index,(int)slb_x,(int)slb_y);
    // Note that digger task position stores the central subtile on slab to be excavated
    // which happens to be the same subtile as one stored in keeper map tasks
    long task_idx;
    task_idx = find_dig_from_task_list(creatng->owner, cctrl->digger.task_stl);
    if (task_idx != -1)
    {
        struct SlabMap *slb;
        slb = get_slabmap_block(slb_x, slb_y);
        if (slab_kind_is_indestructible(slb->kind)) {
            return true;
        }
    }
    return false;
}

long check_out_undug_place(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    MapSubtlCoord base_stl_x;
    MapSubtlCoord base_stl_y;
    long i;
    long n;
    SYNCDBG(19,"Starting");
    cctrl = creature_control_get_from_thing(creatng);
    base_stl_x = stl_num_decode_x(cctrl->digger.task_stl);
    base_stl_y = stl_num_decode_y(cctrl->digger.task_stl);
    n = CREATURE_RANDOM(creatng, 4);
    for (i=0; i < 4; i++)
    {
        struct MapTask* mtask;
        SubtlCodedCoords task_pos;
        MapSlabCoord slb_x;
        MapSlabCoord slb_y;
        long task_idx;
        slb_x = subtile_slab_fast(base_stl_x)+small_around[n].delta_x;
        slb_y = subtile_slab_fast(base_stl_y)+small_around[n].delta_y;
        task_pos = get_subtile_number_at_slab_center(slb_x, slb_y);
        task_idx = find_dig_from_task_list(creatng->owner, task_pos);
        if (task_idx != -1)
        {
            long mv_x;
            long mv_y;
            mv_x = 0; mv_y = 0;
            if (check_place_to_dig_and_get_position(creatng, task_pos, &mv_x, &mv_y)
                && setup_person_move_to_position(creatng, mv_x, mv_y, NavRtF_Default))
            {
                cctrl->digger.task_idx = task_idx;
                cctrl->digger.task_stl = task_pos;
                mtask = get_task_list_entry(creatng->owner, cctrl->digger.task_idx);
                if (mtask->kind == SDDigTask_MineGold)
                {
                  creatng->continue_state = CrSt_ImpArrivesAtMineGold;
                } else
                {
                  creatng->continue_state = CrSt_ImpArrivesAtDigDirt;
                }
                return 1;
            }
        }
        n = (n + 1) % 4;
    }
    return 0;
}

long get_random_mining_undug_area_position_for_digger_drop(PlayerNumber plyr_idx, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    long i;
    long n;
    long tsk_max;
    tsk_max = dungeon->highest_task_number;
    if (tsk_max > MAPTASKS_COUNT)
        tsk_max = MAPTASKS_COUNT;
    if (tsk_max > 1)
        n = PLAYER_RANDOM(plyr_idx, tsk_max);
    else
        n = 0;
    for (i=0; i < tsk_max; i++,n=(n+1)%tsk_max)
    {
        struct MapTask *mtask;
        mtask = &dungeon->task_list[n];
        if (mtask->kind == SDDigTask_None)
            continue;
        if (mtask->kind == SDDigTask_MineGold)
        {
            MapSubtlCoord tsk_stl_x;
            MapSubtlCoord tsk_stl_y;
            if (check_place_to_dig_and_get_drop_position(plyr_idx, mtask->coords, &tsk_stl_x, &tsk_stl_y))
            {
                if (can_drop_thing_here(tsk_stl_x, tsk_stl_y, plyr_idx, 1))
                {
                    *retstl_x = tsk_stl_x;
                    *retstl_y = tsk_stl_y;
                    return n;
                }
            }
        }
    }
    return -1;
}

#define UNDUG_MAX_DIST 24
long get_nearest_undug_area_position_for_digger(struct Thing *thing, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    dungeon = get_dungeon(thing->owner);
    cctrl = creature_control_get_from_thing(thing);
    struct MapTask *mtask;
    long i;
    long tsk_max;
    tsk_max = dungeon->highest_task_number;
    if (tsk_max > MAPTASKS_COUNT)
        tsk_max = MAPTASKS_COUNT;
    MapSubtlCoord digstl_y;
    MapSubtlCoord digstl_x;
    digstl_x = stl_num_decode_x(cctrl->digger.task_stl);
    digstl_y = stl_num_decode_y(cctrl->digger.task_stl);
    MapSubtlCoord best_dist;
    MapSubtlCoord best_stl_x;
    MapSubtlCoord best_stl_y;
    int best_tsk_id;
    best_dist = UNDUG_MAX_DIST;
    best_tsk_id = -1;
    best_stl_x = -1;
    best_stl_y = -1;
    for (i=0; i < tsk_max; i++)
    {
        mtask = &dungeon->task_list[i];
        if (mtask->kind == SDDigTask_None)
            continue;
        if (mtask->kind != SDDigTask_MineGems)
        {
            SubtlCodedCoords tsk_stl_num;
            MapSubtlCoord tsk_dist;
            tsk_stl_num = mtask->coords;
            tsk_dist = get_2d_box_distance_xy(digstl_x, digstl_y, stl_num_decode_x(tsk_stl_num), stl_num_decode_y(tsk_stl_num));
            if (tsk_dist < best_dist)
            {
                MapSubtlCoord tsk_stl_x;
                MapSubtlCoord tsk_stl_y;
                if (check_place_to_dig_and_get_position(thing, tsk_stl_num, &tsk_stl_x, &tsk_stl_y))
                {
                    best_dist = tsk_dist;
                    best_tsk_id = i;
                    best_stl_x = tsk_stl_x;
                    best_stl_y = tsk_stl_y;
                }
            }
        }
    }
    if (best_dist >= UNDUG_MAX_DIST) {
        return -1;
    }
    *retstl_x = best_stl_x;
    *retstl_y = best_stl_y;
    return best_tsk_id;
}
#undef UNDUG_MAX_DIST

long check_out_undug_area(struct Thing *thing)
{
    SYNCDBG(19,"Starting");
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    int tsk_id;
    stl_x = -1;
    stl_y = -1;
    tsk_id = get_nearest_undug_area_position_for_digger(thing, &stl_x, &stl_y);
    if (tsk_id < 0) {
        return 0;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default)) {
        return 0;
    }
    struct Dungeon *dungeon;
    dungeon = get_dungeon(thing->owner);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    struct MapTask *mtask;
    mtask = &dungeon->task_list[tsk_id];
    cctrl->digger.task_idx = tsk_id;
    cctrl->digger.task_stl = mtask->coords;
    if (mtask->kind == SDDigTask_MineGold) {
        thing->continue_state = CrSt_ImpArrivesAtMineGold;
    } else {
        thing->continue_state = CrSt_ImpArrivesAtDigDirt;
    }
    return 1;
}

int add_undug_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    struct MapTask* mtask;
    long stl_x;
    long stl_y;
    long i;
    SYNCDBG(18,"Starting");
    int remain_num;
    remain_num = max_tasks;
    i = -1;
    while ((remain_num > 0) && (dungeon->digger_stack_length < DIGGER_TASK_MAX_COUNT))
    {
        i = find_next_dig_in_dungeon_task_list(dungeon, i);
        if (i < 0)
            break;
        mtask = get_dungeon_task_list_entry(dungeon, i);
        stl_x = stl_num_decode_x(mtask->coords);
        stl_y = stl_num_decode_y(mtask->coords);
        struct SlabMap *slb;
        slb = get_slabmap_for_subtile(stl_x, stl_y);
        if (!slab_kind_is_indestructible(slb->kind)) // Add only blocks which can be destroyed by digging
        {
            if ( block_has_diggable_side(dungeon->owner, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)) )
            {
                add_to_imp_stack_using_pos(mtask->coords, DigTsk_DigOrMine, dungeon);
                remain_num--;
            }
        }
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}
int add_gems_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    struct MapTask* mtask;
    long stl_x;
    long stl_y;
    long i;
    SYNCDBG(18,"Starting");
    int remain_num;
    remain_num = max_tasks;
    i = -1;
    while ((remain_num > 0) && (dungeon->digger_stack_length < DIGGER_TASK_MAX_COUNT))
    {
        i = find_next_dig_in_dungeon_task_list(dungeon, i);
        if (i < 0)
            break;
        mtask = get_dungeon_task_list_entry(dungeon, i);
        stl_x = stl_num_decode_x(mtask->coords);
        stl_y = stl_num_decode_y(mtask->coords);
        if ( subtile_revealed(stl_x, stl_y, dungeon->owner) )
        {
            struct SlabMap *slb;
            slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (slab_kind_is_indestructible(slb->kind)) // Add only blocks which cannot be destroyed by digging
            {
                if ( block_has_diggable_side(dungeon->owner, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)) )
                {
                    add_to_imp_stack_using_pos(mtask->coords, DigTsk_DigOrMine, dungeon);
                    remain_num--;
                }
            }
        }
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

TbBool add_to_reinforce_stack(long slb_x, long slb_y, SpDiggerTaskType task_type)
{
    if (r_stackpos >= DIGGER_TASK_MAX_COUNT) {
        return false;
    }
    struct DiggerStack *rfstack;
    rfstack = &reinforce_stack[r_stackpos];
    r_stackpos++;
    rfstack->stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
    rfstack->task_type = task_type;
    return true;
}

long add_to_reinforce_stack_if_need_to(long slb_x, long slb_y, struct Dungeon *dungeon)
{
    if (r_stackpos < DIGGER_TASK_MAX_COUNT - dungeon->digger_stack_length)
    {
        struct SlabMap *slb;
        slb = get_slabmap_block(slb_x, slb_y);
        if (slab_kind_is_friable_dirt(slb->kind))
        {
            if (subtile_revealed(slab_subtile_center(slb_x), slab_subtile_center(slb_y), dungeon->owner))
            {
                if (slab_by_players_land(dungeon->owner, slb_x, slb_y))
                {
                    add_to_reinforce_stack(slb_x, slb_y, DigTsk_ReinforceWall);
                }
            }
        }
    }
    return (r_stackpos < DIGGER_TASK_MAX_COUNT - dungeon->digger_stack_length);
}

long add_to_pretty_to_imp_stack_if_need_to(long slb_x, long slb_y, struct Dungeon *dungeon, int *remain_num)
{
    //return _DK_add_to_pretty_to_imp_stack_if_need_to(slb_x, slb_y, dungeon);
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    const struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    if (slb->kind == SlbT_PATH)
    {
        if (subtile_revealed(stl_x, stl_y, dungeon->owner) && slab_by_players_land(dungeon->owner, slb_x, slb_y)) {
            (*remain_num)--;
            return add_to_imp_stack_using_pos(get_subtile_number_at_slab_center(slb_x, slb_y), DigTsk_ImproveDungeon, dungeon);
        }
    } else
    if ((slb->kind == SlbT_CLAIMED) || slab_kind_is_room(slb->kind))
    {
        if (!players_are_mutual_allies(dungeon->owner, slabmap_owner(slb)))
        {
            if (subtile_revealed(stl_x, stl_y, dungeon->owner) && slab_by_players_land(dungeon->owner, slb_x, slb_y)) {
                (*remain_num)--;
                return add_to_imp_stack_using_pos(get_subtile_number_at_slab_center(slb_x, slb_y), DigTsk_ConvertDungeon, dungeon);
            }
        }
    }
    if (slab_is_door(slb_x, slb_y)) // Door is in the way of claiming
    {
        struct Thing* doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (!thing_is_invalid(doortng))
        {
            if (players_are_enemies(doortng->owner, dungeon->owner))
            {
                event_create_event_or_update_old_event(doortng->mappos.x.val, doortng->mappos.y.val, EvKind_EnemyDoor, dungeon->owner, doortng->index);
            }
        }
    }
    return (dungeon->digger_stack_length < DIGGER_TASK_MAX_COUNT);
}

/**
 * Array used to determine whether a diagonal slab is blocked by two two slabs around.
 */
struct ExtraSquares spdigger_extra_squares[] = {
    { 0,  0x00},
    { 0,  0x00},
    { 0,  0x00},
    { 1, ~0x03}, // 0x01|0x02 are blocking slab
    { 0,  0x00},
    { 0,  0x00},
    { 2, ~0x06}, // 0x02|0x04 are blocking slab
    { 1, ~0x01}, // 0x01|0x02|0x04 are blocking 2 slabs
    { 0,  0x00},
    { 4, ~0x09}, // 0x01|0x08 are blocking slab
    { 0,  0x00},
    { 1, ~0x02}, // 0x01|0x02|0x08 are blocking 2 slabs
    { 3, ~0x0C}, // 0x04|0x08 are blocking slab
    { 3, ~0x04}, // 0x01|0x04|0x08 are blocking 2 slabs
    { 2, ~0x02}, // 0x02|0x04|0x08 are blocking 2 slabs
    { 1,  0x00}, // all diagonals blocked, special case
};

struct Around spdigger_extra_positions[] = {
    { 0, 0},
    { 1,-1},
    { 1, 1},
    {-1, 1},
    {-1,-1},
};
#define SPDIGGER_EXTRA_POSITIONS_COUNT 5

enum SlabConnectedAreaOptions {
    SlbCAOpt_None      = 0x00,
    SlbCAOpt_Border    = 0x01,
    SlbCAOpt_Processed = 0x02,
};

/**
 * Prepares slab options map used for finding pretty and convert tasks for diggers.
 * @param dungeon Target dungeon for which tasks are to be searched.
 * @param slbopt The slab options map to be initialized.
 */
void add_pretty_and_convert_to_imp_stack_prepare(struct Dungeon *dungeon, unsigned char *slbopt)
{
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    // Clear our slab options array and mark tall slabs with SlbCAOpt_Border
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
            slbopt[slb_num] = 0;
            if ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                slbopt[slb_num] |= SlbCAOpt_Border;
            }
        }
    }
}

/**
 * Adds pretty and convert tasks of areas connected to given position into imp stack.
 * @param dungeon Target dungeon for which tasks should be added.
 * @param slbopt Slab options map.
 * @param slblist Buffer for storing temporary slabs list.
 * @param start_pos The position connected to tasks to find.
 * @param remain_num Limit of tasks which can still be added.
 * @return The amount of slabs checked.
 */
long add_pretty_and_convert_to_imp_stack_starting_from_pos(struct Dungeon *dungeon, unsigned char *slbopt, struct SlabCoord *slblist, const struct Coord3d * start_pos, int *remain_num)
{
    unsigned int slblicount;
    unsigned int slblipos;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slblipos = 0; // Current position in our list of slabs which should be checked around
    slblicount = 0; // Amount of items in our list of slabs which should be checked around
    MapSlabCoord base_slb_x;
    MapSlabCoord base_slb_y;
    base_slb_x = subtile_slab(start_pos->x.stl.num);
    base_slb_y = subtile_slab(start_pos->y.stl.num);
    SlabCodedCoords slb_num;
    slb_num = get_slab_number(base_slb_x, base_slb_y);
    slbopt[slb_num] |= SlbCAOpt_Processed;
    // Verify slabs around; we will add more around slabs to checklist as we progress
    do
    {
        unsigned char around_flags;
        around_flags = 0;
        long i;
        long n;
        n = PLAYER_RANDOM(dugneon->owner, 4);
        for (i=0; i < SMALL_AROUND_LENGTH; i++)
        {
            slb_x = base_slb_x + (long)small_around[n].delta_x;
            slb_y = base_slb_y + (long)small_around[n].delta_y;
            slb_num = get_slab_number(slb_x, slb_y);
            // Per around code
            if ((slbopt[slb_num] & SlbCAOpt_Border) != 0)
            { // Prepare around flags to be used later for ExtraSquares
                around_flags |= (1<<n);
            }
            if ((slbopt[slb_num] & SlbCAOpt_Processed) == 0)
            {
                slbopt[slb_num] |= SlbCAOpt_Processed;
                // For border wall, check if it can be reinforced
                if ((slbopt[slb_num] & SlbCAOpt_Border) != 0)
                {
                    add_to_reinforce_stack_if_need_to(slb_x, slb_y, dungeon);
                } else
                // If not a border, add it to around verification list and check for pretty
                {
                    slblist[slblicount].x = slb_x;
                    slblist[slblicount].y = slb_y;
                    slblicount++;
                    if ((*remain_num) <= 0)
                    {
                        // Even if the remain_num reaches zero and we can't add new tasks, we may still
                        // want to continue the loop if reinforce stack is not filled.
                        if (r_stackpos >= DIGGER_TASK_MAX_COUNT - dungeon->digger_stack_length) {
                            return slblipos;
                        }
                    } else
                    {
                        // The remain_num parameter must go to subfunction - here we don't know if we should decrement it or not
                        if ( !add_to_pretty_to_imp_stack_if_need_to(slb_x, slb_y, dungeon, remain_num) ) {
                            SYNCDBG(6,"Cannot add any more pretty tasks");
                            return slblipos;
                        }
                    }
                }
            }
            // Per around code ends
            n = (n + 1) % SMALL_AROUND_LENGTH;
        }

        // Check if we can already remove diagonal slabs from verification
        struct ExtraSquares  *square;
        for (square = &spdigger_extra_squares[around_flags]; square->index != 0; square = &spdigger_extra_squares[around_flags])
        {
            if (around_flags == (0x01|0x02|0x04|0x08))
            {
                // If whole diagonal around is to be marked, just do it in one go
                for (i=1; i < SPDIGGER_EXTRA_POSITIONS_COUNT; i++)
                {
                    slb_x = base_slb_x + (long)spdigger_extra_positions[i].delta_x;
                    slb_y = base_slb_y + (long)spdigger_extra_positions[i].delta_y;
                    add_to_reinforce_stack_if_need_to(slb_x, slb_y, dungeon);
                    slb_num = get_slab_number(slb_x, slb_y);
                    slbopt[slb_num] |= SlbCAOpt_Processed;
                }
                around_flags = 0;
            } else
            {
                i = square->index;
                {
                    slb_x = base_slb_x + (long)spdigger_extra_positions[i].delta_x;
                    slb_y = base_slb_y + (long)spdigger_extra_positions[i].delta_y;
                    add_to_reinforce_stack_if_need_to(slb_x, slb_y, dungeon);
                    slb_num = get_slab_number(slb_x, slb_y);
                    slbopt[slb_num] |= SlbCAOpt_Processed;
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


/**
 * Adds tasks of claiming unowned and converting enemy land to the digger tasks stack; also fills reinforce tasks.
 * @param dungeon Target dungeon for which tasks should be added.
 * @param max_tasks Max amount of tasks to be added.
 * @return The amount of tasks added.
 */
int add_pretty_and_convert_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    if (dungeon->digger_stack_length >= DIGGER_TASK_MAX_COUNT) {
        WARNLOG("Too many jobs, no place for more");
        return 0;
    }
    SYNCDBG(18,"Starting");
    //TODO SPDIGGER This restricts convert tasks to the area connected to heart, instead of connected to diggers.
    struct Thing *heartng;
    heartng = get_player_soul_container(dungeon->owner);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng)) {
        WARNLOG("The player %d has no heart, no dungeon position available",(int)dungeon->owner);
        return 0;
    }
    int remain_num;
    remain_num = max_tasks;
    unsigned char *slbopt;
    struct SlabCoord *slblist;
    slbopt = scratch;
    slblist = (struct SlabCoord *)(scratch + map_tiles_x*map_tiles_y);
    add_pretty_and_convert_to_imp_stack_prepare(dungeon, slbopt);
    add_pretty_and_convert_to_imp_stack_starting_from_pos(dungeon, slbopt, slblist, &heartng->mappos, &remain_num);
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

/**
 * Returns if thing can be picked by special digger.
 *
 * Things considered "for us" are neutral or own things on neutral ground, and
 * things owned by enemy players on our ground.
 * If either thing owner or ground owner doesn't match, we can't pick that thing.
 * Additionally, we have a special condition in case our thing + our ground, because
 * in that case the thing may already be on a correct position.
 *
 * @param thing
 * @param dungeon
 * @param rkind
 * @return
 */
TbBool thing_can_be_picked_to_place_in_player_room(const struct Thing* thing, PlayerNumber plyr_idx, RoomKind rkind, unsigned short flags)
{
    if (thing_is_object(thing))
    {
        if (!object_is_room_inventory(thing, rkind)) {
            return false;
        }
    } else
    if (thing_is_dead_creature(thing))
    {
        if (!dead_creature_is_room_inventory(thing, rkind)) {
            return false;
        }
    } else
    {
        return false;
    }
    const struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    if (!thing_revealed(thing, dungeon->owner)) {
        return false;
    }
    if (thing_is_dragged_or_pulled(thing)) {
        return false;
    }
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    // Neutral things on either neutral or owned ground should be always pickable
    if ((thing->owner == game.neutral_player_num) && ((slabmap_owner(slb) == game.neutral_player_num) || (slabmap_owner(slb) == dungeon->owner)))
    {
        return true;
    } else
    // Same goes for owned things on neutral ground
    if ((thing->owner == dungeon->owner) && (slabmap_owner(slb) == game.neutral_player_num))
    {
        if (thing_is_object(thing)) {
            WARNLOG("The %s owner %d found on neutral ground instead of owner's %s",thing_model_name(thing),(int)thing->owner,room_code_name(rkind));
        }
        return true;
    } else
    // Things belonging to enemy but laying on our ground can be taken
    if (!players_are_mutual_allies(dungeon->owner, thing->owner) && (slabmap_owner(slb) == dungeon->owner))
    {
        if (thing_is_object(thing)) {
            WARNLOG("The %s owner %d found on own ground instead of owner's %s",thing_model_name(thing),(int)thing->owner,room_code_name(rkind));
        }
        return true;
    } else
    // Owned things on owned ground are only pickable if not already in storage room
    if ((thing->owner == dungeon->owner) && (slabmap_owner(slb) == dungeon->owner))
    {
        if ((flags & TngFRPickF_AllowStoredInOwnedRoom) != 0) {
            // Allow things stored in own room if flags say so
            return true;
        }
        struct Room* room;
        room = get_room_thing_is_on(thing);
        if (room_is_invalid(room) || (room->kind != rkind))
        {
            if (thing_is_object(thing)) {
                WARNLOG("The %s owner %d found on his ground but outside %s",thing_model_name(thing),(int)thing->owner,room_code_name(rkind));
            }
            return true;
        }
    }
    return false;
}

struct Thing *get_next_unclaimed_gold_thing_pickable_by_digger(PlayerNumber owner, int start_idx)
{
    struct Thing *thing;
    int i;
    int k;
    k = 0;
    i = start_idx;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_object(thing) && object_is_gold_pile(thing))
        {
            // TODO DIGGERS Use thing_can_be_picked_to_place_in_player_room() instead of single conditions
            //if (thing_can_be_picked_to_place_in_player_room(thing, owner, RoK_TREASURE, TngFRPickF_Default))
            if (!thing_is_picked_up(thing) && !thing_is_dragged_or_pulled(thing))
            {
                  if (thing_revealed(thing, owner))
                  {
                      PlayerNumber slb_owner;
                      slb_owner = get_slab_owner_thing_is_on(thing);
                      if ((slb_owner == owner) || (slb_owner == game.neutral_player_num)) {
                          struct Room *room;
                          room = find_any_navigable_room_for_thing_closer_than(thing, owner, RoK_TREASURE, NavRtF_Default, map_subtiles_x/2 + map_subtiles_y/2);
                          if (!room_is_invalid(room)) {
                              return thing;
                          }
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
    return INVALID_THING;
}

int add_unclaimed_gold_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    //return _DK_add_unclaimed_gold_to_imp_stack(dungeon);
    struct Room *room;
    room = find_room_with_spare_capacity(dungeon->owner, RoK_TREASURE, 1);
    if (room_is_invalid(room)) {
        return 0;
    }
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    int remain_num;
    remain_num = max_tasks;
    struct Thing *gldtng;
    gldtng = get_next_unclaimed_gold_thing_pickable_by_digger(dungeon->owner, slist->index);
    while ((remain_num > 0) && (dungeon->digger_stack_length < DIGGER_TASK_MAX_COUNT))
    {
        if (thing_is_invalid(gldtng)) {
            break;
        }
        SubtlCodedCoords stl_num;
        stl_num = get_subtile_number(gldtng->mappos.x.stl.num,gldtng->mappos.y.stl.num);
        if (find_in_imp_stack_using_pos(stl_num, DigTsk_PicksUpGoldPile, dungeon) == -1) {
            add_to_imp_stack_using_pos(stl_num, DigTsk_PicksUpGoldPile, dungeon);
            remain_num--;
        }
        gldtng = get_next_unclaimed_gold_thing_pickable_by_digger(dungeon->owner, gldtng->next_of_class);
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

void setup_imp_stack(struct Dungeon *dungeon)
{
    long i;
    for (i = 0; i < dungeon->digger_stack_length; i++)
    {
        dungeon->digger_stack[i].task_type = DigTsk_None;
    }
    dungeon->digger_stack_update_turn = game.play_gameturn;
    dungeon->digger_stack_length = 0;
    r_stackpos = 0;
}

int add_unclaimed_unconscious_bodies_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    struct Thing *thing;
    struct Room *room;
    int remain_num;
    unsigned long k;
    int i;
    if (!dungeon_has_room(dungeon, RoK_PRISON)) {
        SYNCDBG(8,"Dungeon %d has no %s",(int)dungeon->owner,room_code_name(RoK_PRISON));
        return 0;
    }
    if (!player_creature_tends_to(dungeon->owner, CrTend_Imprison)) {
        SYNCDBG(8,"Player %d creatures do not tend to imprison",(int)dungeon->owner);
        return 0;
    }
    room = find_room_with_spare_capacity(dungeon->owner, RoK_PRISON, 1);
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    k = 0;
    i = slist->index;
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
        // Per-thing code
        if ( (dungeon->digger_stack_length >= DIGGER_TASK_MAX_COUNT) || (remain_num <= 0) ) {
            break;
        }
        if (players_are_enemies(dungeon->owner,thing->owner) && creature_is_being_unconscious(thing) && !thing_is_dragged_or_pulled(thing))
        {
            if (thing_revealed(thing, dungeon->owner))
            {
                if (room_is_invalid(room))
                {
                    // Check why the room search failed and inform the player
                    update_cannot_find_room_wth_spare_capacity_event(dungeon->owner, thing, RoK_PRISON);
                    break;
                }
                SubtlCodedCoords stl_num;
                stl_num = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
                if (!add_to_imp_stack_using_pos(stl_num, DigTsk_PickUpUnconscious, dungeon)) {
                    break;
                }
                remain_num--;
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

int add_unclaimed_dead_bodies_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    struct Thing *thing;
    struct Room *room;
    SubtlCodedCoords stl_num;
    int remain_num;
    unsigned long k;
    int i;
    if (!dungeon_has_room(dungeon, RoK_GRAVEYARD)) {
        SYNCDBG(8,"Dungeon %d has no %s",(int)dungeon->owner,room_code_name(RoK_GRAVEYARD));
        return 0;
    }
    room = find_room_with_spare_capacity(dungeon->owner, RoK_GRAVEYARD, 1);
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_DeadCreature);
    k = 0;
    i = slist->index;
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
        // Per-thing code
        if ( (dungeon->digger_stack_length >= DIGGER_TASK_MAX_COUNT) || (remain_num <= 0) ) {
            break;
        }
        if (!thing_is_dragged_or_pulled(thing) && (thing->active_state == DCrSt_RigorMortis)
           && (!corpse_laid_to_rest(thing)) && corpse_is_rottable(thing))
        {
            if (thing_revealed(thing, dungeon->owner))
            {
                if (room_is_invalid(room))
                {
                    // Check why the room search failed and inform the player
                    update_cannot_find_room_wth_spare_capacity_event(dungeon->owner, thing, RoK_GRAVEYARD);
                    return 0;
                }
                stl_num = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
                if (!add_to_imp_stack_using_pos(stl_num, DigTsk_PickUpCorpse, dungeon)) {
                    break;
                }
                remain_num--;
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

int add_unclaimed_spells_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    if (!dungeon_has_room(dungeon, RoK_LIBRARY)) {
        SYNCDBG(8,"Dungeon %d has no %s",(int)dungeon->owner,room_code_name(RoK_LIBRARY));
        return 0;
    }
    struct Room *room;
    room = find_room_with_spare_room_item_capacity(dungeon->owner, RoK_LIBRARY);
    int remain_num;
    remain_num = max_tasks;
    long i;
    unsigned long k;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if ((dungeon->digger_stack_length >= DIGGER_TASK_MAX_COUNT) || (remain_num <= 0)) {
            break;
        }
        if (thing_can_be_picked_to_place_in_player_room(thing, dungeon->owner, RoK_LIBRARY, TngFRPickF_Default))
        {
            if (room_is_invalid(room))
            {
                // Check why the room search failed and inform the player
                update_cannot_find_room_wth_spare_capacity_event(dungeon->owner, thing, RoK_LIBRARY);
                break;
            }
            SubtlCodedCoords stl_num;
            stl_num = get_subtile_number(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            SYNCDBG(18,"Pickup task for dungeon %d at (%d,%d)",
                (int)dungeon->owner,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
            if (!add_to_imp_stack_using_pos(stl_num, DigTsk_PicksUpSpellBook, dungeon)) {
                break;
            }
            remain_num--;
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

TbBool add_object_for_trap_to_imp_stack(struct Dungeon *dungeon, struct Thing *armtng)
{
    unsigned long k;
    int i;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing->model == trap_crate_object_model(armtng->model))
        {
            struct SlabMap *slb;
            slb = get_slabmap_thing_is_on(thing);
            if (slabmap_owner(slb) == dungeon->owner)
            {
                SubtlCodedCoords stl_num;
                stl_num = get_subtile_number(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
                if (find_in_imp_stack_using_pos(stl_num, DigTsk_PicksUpCrateToArm, dungeon) == -1)
                {
                    add_to_imp_stack_using_pos(stl_num, DigTsk_PicksUpCrateToArm, dungeon);
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

int add_empty_traps_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    SYNCDBG(18,"Starting");
    int remain_num;
    remain_num = max_tasks;
    long i;
    unsigned long k;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Trap);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Thing list loop body
        if ((dungeon->digger_stack_length >= DIGGER_TASK_MAX_COUNT) || (remain_num <= 0)) {
            break;
        }
        if ((thing->trap.num_shots == 0) && (thing->owner == dungeon->owner))
        {
            if ( add_object_for_trap_to_imp_stack(dungeon, thing) ) {
                remain_num--;
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
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

int add_unclaimed_traps_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    struct Thing* thing;
    SYNCDBG(18,"Starting");
    // Checking if the workshop exists
    struct Room *room;
    room = find_room_with_spare_room_item_capacity(dungeon->owner, RoK_WORKSHOP);
    int remain_num;
    remain_num = max_tasks;
    long i;
    unsigned long k;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if ((dungeon->digger_stack_length >= DIGGER_TASK_MAX_COUNT) || (remain_num <= 0)) {
            break;
        }
        if (thing_can_be_picked_to_place_in_player_room(thing, dungeon->owner, RoK_WORKSHOP, TngFRPickF_Default))
        {
            if (room_is_invalid(room))
            {
                // Check why the room search failed and inform the player
                update_cannot_find_room_wth_spare_capacity_event(dungeon->owner, thing, RoK_WORKSHOP);
                break;
            }
            SubtlCodedCoords stl_num;
            stl_num = get_subtile_number(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            SYNCDBG(8,"Pickup task for dungeon %d at (%d,%d)",
                (int)dungeon->owner,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
            if (!add_to_imp_stack_using_pos(stl_num, DigTsk_PicksUpCrateForWorkshop, dungeon)) {
                break;
            }
            remain_num--;
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

/**
 * Adds tasks from a pre-made list of reinforce tasks to the digger tasks stack.
 * The reinforce tasks have to be filled by a add_pretty_and_convert_to_imp_stack() call.
 * @param dungeon Target dungeon for which reinforce stack is filled.
 * @param max_tasks Max amount of tasks to be added.
 * @return The amount of tasks added.
 * Only fill up the stack with reinforce task halfway
 */
int add_reinforce_to_imp_stack(struct Dungeon *dungeon, int max_tasks)
{
    int remain_num;
    remain_num = max_tasks;
    long i;
    for (i=0; i < r_stackpos; i++)
    {
        if ((dungeon->digger_stack_length >= 32) || (remain_num <= 0)) {
            break;
        }
        struct DiggerStack *rfstack;
        rfstack = &reinforce_stack[i];
        add_to_imp_stack_using_pos(rfstack->stl_num, rfstack->task_type, dungeon);
        remain_num--;
    }
    SYNCDBG(8,"Done, added %d tasks",(int)(max_tasks-remain_num));
    return (max_tasks-remain_num);
}

TbBool slab_is_players_land(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    if ((slb->kind == SlbT_LAVA) || (slb->kind == SlbT_WATER)) {
        return false;
    }
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    if (!slbattr->is_safe_land) {
        return false;
    }
    return (slabmap_owner(slb) == plyr_idx);
}

long imp_already_reinforcing_at_excluding(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    return _DK_imp_already_reinforcing_at_excluding(creatng, stl_x, stl_y);
}
int get_nearest_small_around_side_of_slab(MapCoord dstcor_x, MapCoord dstcor_y, MapCoord srccor_x, MapCoord srccor_y)
{
    MapCoordDelta delta_x;
    MapCoordDelta delta_y;
    delta_x = dstcor_x - (MapCoordDelta)srccor_x;
    delta_y = dstcor_y - (MapCoordDelta)srccor_y;
    // First check the nearest side
    if (abs(delta_y) > abs(delta_x))
    {
      if (delta_y > 0)
          return 0;
      else
          return 2;
    } else
    {
      if (delta_x > 0)
          return 3;
      else
          return 1;
    }
    ERRORLOG("Impossible reached");
    return 0;
}

long check_out_uncrowded_reinforce_position(struct Thing *thing, SubtlCodedCoords stl_num, long *retstl_x, long *retstl_y)
{
    MapSubtlCoord basestl_x;
    MapSubtlCoord basestl_y;
    basestl_x = stl_num_decode_x(stl_num);
    basestl_y = stl_num_decode_y(stl_num);
    int i;
    int n;
    n = get_nearest_small_around_side_of_slab(subtile_coord_center(basestl_x), subtile_coord_center(basestl_y), thing->mappos.x.val, thing->mappos.y.val);  
    for (i=0; i < SMALL_AROUND_LENGTH; i++)
    {
        MapSubtlCoord stl_x;
        MapSubtlCoord stl_y;
        stl_x = basestl_x + 2 * (long)small_around[n].delta_x;
        stl_y = basestl_y + 2 * (long)small_around[n].delta_y;
        if (slab_is_players_land(thing->owner, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)))
        {
            if (!imp_already_reinforcing_at_excluding(thing, stl_x, stl_y))
            {
                if (!imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
                {
                    struct Coord3d pos;
                    pos.z.val = 0;
                    pos.x.val = subtile_coord_center(stl_x);
                    pos.y.val = subtile_coord_center(stl_y);
                    pos.z.val = get_thing_height_at(thing, &pos);
                    if ( creature_can_navigate_to_with_storage(thing, &pos, NavRtF_Default) != -1 ) {
                        *retstl_x = stl_x;
                        *retstl_y = stl_y;
                        return 1;
                    }
                }
            }
        }
        n = (n + 1) % SMALL_AROUND_LENGTH;
    }
    return 0;
}

long check_place_to_dig_and_get_drop_position(PlayerNumber plyr_idx, SubtlCodedCoords stl_num, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y)
{
    struct SlabMap *place_slb;
    MapSubtlCoord place_x;
    MapSubtlCoord place_y;
    long base_x;
    long base_y;
    long stl_x;
    long stl_y;
    long i;
    long k;
    long n;
    long nstart;
    SYNCDBG(18,"Starting");
    place_x = stl_num_decode_x(stl_num);
    place_y = stl_num_decode_y(stl_num);
    if (!block_has_diggable_side(plyr_idx, subtile_slab_fast(place_x), subtile_slab_fast(place_y)))
        return 0;
    place_slb = get_slabmap_for_subtile(place_x,place_y);
    n = PLAYER_RANDOM(plyr_idx, SMALL_AROUND_SLAB_LENGTH);

    for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
    {
      base_x = place_x + 2 * (long)small_around[n].delta_x;
      base_y = place_y + 2 * (long)small_around[n].delta_y;
      if (valid_dig_position(plyr_idx, base_x, base_y))
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
              if (valid_dig_position(plyr_idx, stl_x, stl_y))
              {
                    if ((place_slb->kind != SlbT_GEMS) || !gold_pile_with_maximum_at_xy(stl_x, stl_y))
                      if (!imp_already_digging_at_excluding(INVALID_THING, stl_x, stl_y))
                      {
                          *retstl_x = stl_x;
                          *retstl_y = stl_y;
                          return 1;
                      }
              }
          }
      }
      n = (n+1) % 4;
    }
    return 0;
}

long check_place_to_dig_and_get_position(struct Thing *thing, SubtlCodedCoords stl_num, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y)
{
    struct SlabMap *place_slb;
    struct Coord3d pos;
    MapSubtlCoord place_x;
    MapSubtlCoord place_y;
    long base_x;
    long base_y;
    long stl_x;
    long stl_y;
    long i;
    long k;
    long n;
    long nstart;
    SYNCDBG(18,"Starting");
    place_x = stl_num_decode_x(stl_num);
    place_y = stl_num_decode_y(stl_num);
    if (!block_has_diggable_side(thing->owner, subtile_slab_fast(place_x), subtile_slab_fast(place_y)))
        return 0;
    nstart = get_nearest_small_around_side_of_slab(subtile_coord_center(place_x), subtile_coord_center(place_y), thing->mappos.x.val, thing->mappos.y.val);
    place_slb = get_slabmap_for_subtile(place_x,place_y);
    n = nstart;

    for (i = 0; i < SMALL_AROUND_LENGTH; i++)
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
                          if (creature_can_navigate_to_with_storage(thing, &pos, NavRtF_Default))
                          {
                              *retstl_x = stl_x;
                              *retstl_y = stl_y;
                              return 1;
                          }
                        }
              }
          }
      }
      n = (n+1) % SMALL_AROUND_LENGTH;
    }
    return 0;
}

struct Thing *check_place_to_pickup_dead_body(struct Thing *creatng, long stl_x, long stl_y)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x,stl_y);
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
        if (corpse_ready_for_collection(thing))
        {
            return thing;
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

struct Thing* check_place_to_pickup_gold(struct Thing* thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
//return _DK_check_place_to_pickup_gold(thing, stl_x, stl_y);
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    for (int i = get_mapwho_thing_index(mapblk); i != 0;)
    {
        struct Thing* ret = thing_get(i);
        i = ret->next_on_mapblk;
        if ((ret->class_id == TCls_Object) && object_is_gold_pile(ret))
        {
            return ret;
        }
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

struct Thing *check_place_to_pickup_spell(struct Thing *thing, long a2, long a3)
{
    return _DK_check_place_to_pickup_spell(thing, a2, a3);
}

struct Thing *check_place_to_pickup_unconscious_body(struct Thing *thing, long a2, long a3)
{
    return _DK_check_place_to_pickup_unconscious_body(thing, a2, a3);
}

long check_place_to_reinforce(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb;
    TRACE_THING(creatng);
    slb = get_slabmap_block(slb_x, slb_y);
    if ((slb->kind != SlbT_EARTH) && (slb->kind != SlbT_TORCHDIRT)) {
        SYNCDBG(8,"The slab %d,%d is not a valid type to be reinforced",(int)slb_x, (int)slb_y);
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
    SubtlCodedCoords task_pos;
    long task_idx;
    task_pos = get_subtile_number_at_slab_center(slb_x, slb_y);
    task_idx = find_dig_from_task_list(creatng->owner, task_pos);
    if (task_idx != -1) {
        return -1;
    }
    return 1;
}

struct Thing *check_place_to_pickup_crate(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short flags, long n)
{
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
        if (thing_can_be_picked_to_place_in_player_room(thing, creatng->owner, RoK_WORKSHOP, flags))
        {
            if (n > 0) {
                n--;
            } else {
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
 * Checks if given digger has money that should be placed in treasure room.
 * If he does, he is ordered to return them into nearest treasure room
 * which has the proper capacity. If there's no proper treasure room,
 * a proper speech message is created.
 * @param thing The digger creature.
 * @return Gives 1 if the digger was ordered to go into treasure room, 0 otherwise.
 */
long check_out_imp_has_money_for_treasure_room(struct Thing *thing)
{
    struct Room *room;
    SYNCDBG(8,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    //If the imp doesn't have any money - then just return
    if (thing->creature.gold_carried <= 0) {
        return 0;
    }
    // Find a treasure room to drop the money
    room = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_TREASURE, NavRtF_Default, 1);
    if (room_is_invalid(room))
    {
        // Check why the treasure room search failed and inform the player
        update_cannot_find_room_wth_spare_capacity_event(thing->owner, thing, RoK_TREASURE);
        return 0;
    }
    if (setup_head_for_empty_treasure_space(thing, room))
    {
        thing->continue_state = CrSt_ImpDropsGold;
        return 1;
    }
    return 0;
}

long check_out_available_imp_tasks(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    cctrl = creature_control_get_from_thing(thing);
    imp_stack_update(thing);
    if (check_out_imp_stack(thing)) {
        return 1;
    }
    if (game.play_gameturn-cctrl->tasks_check_turn > 128)
    {
        check_out_imp_has_money_for_treasure_room(thing);
        cctrl->tasks_check_turn = game.play_gameturn;
        return 1;
    }
    SYNCDBG(9,"No task for %s index %d",thing_model_name(thing),(int)thing->index);
    return 0;
}

long check_out_imp_tokes(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long i;
    SYNCDBG(19,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    i = CREATURE_RANDOM(thing, 64);
    // small chance of changing state
    if (i != 0)
      return 0;
    internal_set_thing_state(thing, CrSt_ImpToking);
    thing->continue_state = CrSt_ImpDoingNothing;
    cctrl->countdown_282 = 200;
    return 1;
}

long check_out_imp_last_did(struct Thing *creatng)
{
  struct CreatureControl *cctrl;
  struct Dungeon *dungeon;
  struct Room *room;
  cctrl = creature_control_get_from_thing(creatng);
  SYNCDBG(19,"Starting for %s index %d, last did %d repeated %d times",thing_model_name(creatng),(int)creatng->index,(int)cctrl->digger.last_did_job,(int)cctrl->digger.task_repeats);
  TRACE_THING(creatng);
  switch (cctrl->digger.last_did_job)
  {
  case SDLstJob_None:
      return false;
  case SDLstJob_DigOrMine:
      if (is_digging_indestructible_place(creatng)) {
          dungeon = get_dungeon(creatng->owner);
      //don't reassign if it's the first gem dig since other tasks
      if (cctrl->digger.task_repeats != 0)
      { 
        // If we were digging gems, after 5 repeats of this job, a 1 in 20 chance to select another dungeon job.
        // This allows to switch to other important tasks and not consuming all the diggers workforce forever
        if ((( rand( ) % 20) == 1) && ((cctrl->digger.task_repeats % 5) == 0) && (dungeon->digger_stack_length > 1))
        {
          // Set position in digger tasks list to a random place
          SYNCDBG(9,"Digger %s index %d reset due to neverending task",thing_model_name(creatng),(int)creatng->index);
          cctrl->digger.stack_update_turn = dungeon->digger_stack_update_turn;
          cctrl->digger.task_stack_pos = CREATURE_RANDOM(creatng, dungeon->digger_stack_length);
          break;
        }
      }
      }
      if (check_out_undug_place(creatng) || check_out_undug_area(creatng))
      {
          cctrl->digger.task_repeats++;
          cctrl->digger.last_did_job = SDLstJob_DigOrMine;
          return true;
      }
      if (check_out_unconverted_place(creatng) || check_out_unprettied_place(creatng))
      {
          cctrl->digger.task_repeats = 0;
          cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
          SYNCDBG(19,"Done on unprettied or unconverted place 1");
          return true;
      }
      imp_stack_update(creatng);
      if (check_out_unprettied_or_unconverted_area(creatng))
      {
          cctrl->digger.task_repeats = 0;
          cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
          SYNCDBG(9,"Done on unprettied or unconverted area 1");
          return true;
      }
      break;
  case SDLstJob_ConvImprDungeon:
      if (check_out_unconverted_place(creatng) || check_out_unprettied_place(creatng))
      {
          cctrl->digger.task_repeats++;
          cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
          SYNCDBG(19,"Done on unprettied or unconverted place 2");
          return true;
      }
      imp_stack_update(creatng);
      if (check_out_unprettied_or_unconverted_area(creatng))
      {
          cctrl->digger.task_repeats++;
          cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
          SYNCDBG(9,"Done on unprettied or unconverted area 2");
          return true;
      }
      if (check_out_undug_area(creatng))
      {
          cctrl->digger.task_repeats = 0;
          cctrl->digger.last_did_job = SDLstJob_DigOrMine;
          return true;
      }
      break;
  case SDLstJob_ReinforceWall3:
      dungeon = get_dungeon(creatng->owner);
      imp_stack_update(creatng);
      if (creature_task_needs_check_out_after_digger_stack_change(creatng))
      {
          // If there are other tasks besides reinforcing, do not continue reinforcing
          //TODO DIGGERS it would be smarter to include priorities for tasks, and use generic priority handling for all tasks
          if (find_in_imp_stack_task_other_than_starting_at(DigTsk_ReinforceWall, 0, dungeon) != -1)
              break;
      }
      if (check_out_unreinforced_place(creatng))
      {
          cctrl->digger.task_repeats++;
          cctrl->digger.last_did_job = SDLstJob_ReinforceWall3;
          return true;
      }
      if (check_out_unreinforced_area(creatng))
      {
          cctrl->digger.task_repeats++;
          cctrl->digger.last_did_job = SDLstJob_ReinforceWall3;
          return true;
      }
      break;
  case SDLstJob_UseTraining4:
      if (!creature_can_be_trained(creatng) || !player_can_afford_to_train_creature(creatng))
        break;
      room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_TRAINING, NavRtF_Default, 1);
      if (!room_is_invalid(room))
      {
          if (creature_setup_random_move_for_job_in_room(creatng, room, Job_TRAIN, NavRtF_Default))
          {
              cctrl->digger.task_repeats++;
              creatng->continue_state = CrSt_AtTrainingRoom;
              cctrl->target_room_id = room->index;
              return true;
          }
      }
      if (is_my_player_number(creatng->owner))
      {
          room = find_room_with_spare_capacity(creatng->owner, RoK_TRAINING, 1);
          if (room_is_invalid(room)) {
              output_message_room_related_from_computer_or_player_action(creatng->owner, RoK_TRAINING, OMsg_RoomTooSmall);
          }
      }
      break;
  case SDLstJob_ReinforceWall9:
      if (check_out_unreinforced_place(creatng))
      {
          cctrl->digger.task_repeats++;
          cctrl->digger.last_did_job = SDLstJob_ReinforceWall9;
          return true;
      }
      if (check_out_unreinforced_area(creatng))
      {
          cctrl->digger.task_repeats++;
          cctrl->digger.last_did_job = SDLstJob_ReinforceWall9;
          return true;
      }
      break;
  default:
      break;
  }
  cctrl->digger.task_repeats = 0;
  cctrl->digger.last_did_job = SDLstJob_None;
  SYNCDBG(9,"No job found");
  return false;
}
//Create list of up to 64 tasks. 
TbBool imp_stack_update(struct Thing *creatng)
{
    struct Dungeon *dungeon;
    SYNCDBG(18,"Starting");
    dungeon = get_dungeon(creatng->owner);
    if ((game.play_gameturn - dungeon->digger_stack_update_turn) < 128)
        return 0;
    SYNCDBG(8,"Updating");
    setup_imp_stack(dungeon);
    if (dungeon_invalid(dungeon)) {
        WARNLOG("Played %d has no dungeon",(int)creatng->owner);
        return false;
    }
    add_unclaimed_unconscious_bodies_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/4 - 1);
    add_unclaimed_dead_bodies_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/4 - 1);
    add_unclaimed_spells_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/4 - 1);
    add_empty_traps_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/6);
    add_pretty_and_convert_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/64);
    add_undug_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/16 - 1);
    add_unclaimed_gold_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/64);
    add_gems_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT*5/8);
    add_unclaimed_traps_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/4);
    add_undug_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT*5/8);
    add_pretty_and_convert_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT*5/8);
    add_unclaimed_gold_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT/3);
    add_reinforce_to_imp_stack(dungeon, DIGGER_TASK_MAX_COUNT);
    return true;
}

long check_out_worker_improve_dungeon(struct Thing *thing, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    SYNCDBG(18,"Starting");
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (!check_place_to_pretty_excluding(thing, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)))
    {
        dstack->task_type = DigTsk_None;
        return 0;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    thing->continue_state = CrSt_ImpArrivesAtImproveDungeon;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
    return 1;
}

long check_out_worker_convert_dungeon(struct Thing *thing, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (!check_place_to_convert_excluding(thing, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)))
    {
        dstack->task_type = DigTsk_None;
        return 0;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    thing->continue_state = CrSt_ImpArrivesAtConvertDungeon;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
    return 1;
}

long check_out_worker_reinforce_wall(struct Thing *thing, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    struct CreatureControl *cctrl;
    SYNCDBG(18,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    if (game.play_gameturn - cctrl->tasks_check_turn > 128)
    {
        cctrl->digger.task_stack_pos--;
        check_out_imp_has_money_for_treasure_room(thing);
        cctrl->tasks_check_turn = game.play_gameturn;
        return 1;
    }
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (check_place_to_reinforce(thing, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)) <= 0)
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (!check_out_uncrowded_reinforce_position(thing, dstack->stl_num, &stl_x, &stl_y))
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    thing->continue_state = CrSt_ImpArrivesAtReinforce;
    cctrl->digger.byte_93 = 0;
    cctrl->word_8D = dstack->stl_num;
    cctrl->digger.last_did_job = SDLstJob_ReinforceWall3;
    return 1;
}

long check_out_worker_pickup_unconscious(struct Thing *thing, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    SYNCDBG(18,"Starting");
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (!player_has_room_of_role(thing->owner, RoRoF_Prison)) {
        return 0;
    }
    if (!player_creature_tends_to(thing->owner, CrTend_Imprison)) {
        return 0;
    }
    struct Thing *sectng;
    sectng = check_place_to_pickup_unconscious_body(thing, stl_x, stl_y);
    if (thing_is_invalid(sectng))
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    struct Room * room;
    room = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, RoK_PRISON, NavRtF_Default, 1);
    if (room_is_invalid(room))
    {
        update_cannot_find_room_wth_spare_capacity_event(thing->owner, thing, RoK_PRISON);
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    thing->continue_state = CrSt_CreaturePickUpUnconsciousBody;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->pickup_creature_id = sectng->index;
    return 1;
}

long check_out_worker_pickup_corpse(struct Thing *creatng, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (!player_has_room_of_role(creatng->owner, RoRoF_DeadStorage)) {
        return 0;
    }
    struct Thing *deadtng;
    deadtng = check_place_to_pickup_dead_body(creatng, stl_x, stl_y);
    if (thing_is_invalid(deadtng))
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(creatng, stl_x, stl_y))
    {
        return 0;
    }
    struct Room * room;
    room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, RoK_GRAVEYARD, NavRtF_Default, 1);
    if (room_is_invalid(room))
    {
        update_cannot_find_room_wth_spare_capacity_event(creatng->owner, creatng, RoK_GRAVEYARD);
        dstack->task_type = DigTsk_None;
        return -1;
    }
    { // Search for enemies around pickup position
        struct Thing *enmtng;
        enmtng = get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(deadtng->mappos.x.val, deadtng->mappos.y.val, 10, creatng->owner);
        if (!thing_is_invalid(enmtng)) {
            // A place with enemies around is not good for picking up, wait
            return 0;
        }
    }
    if (!setup_person_move_to_position(creatng, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    creatng->continue_state = CrSt_CreaturePicksUpCorpse;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->pickup_object_id = deadtng->index;
    return 1;
}

long check_out_worker_pickup_spellbook(struct Thing *thing, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (!player_has_room_of_role(thing->owner, RoRoF_PowersStorage)) {
        return 0;
    }
    struct Thing *sectng;
    sectng = check_place_to_pickup_spell(thing, stl_x, stl_y);
    if (thing_is_invalid(sectng))
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    struct Room *room;
    room = find_nearest_room_for_thing_with_spare_item_capacity(thing, thing->owner, RoK_LIBRARY, NavRtF_Default);
    if (room_is_invalid(room))
    {
        update_cannot_find_room_wth_spare_capacity_event(thing->owner, thing, RoK_LIBRARY);
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    // Do not create event about spellbook or special yet - wait until we pick it up
    thing->continue_state = CrSt_CreaturePicksUpSpellObject;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->pickup_object_id = sectng->index;
    return 1;
}

long check_out_worker_pickup_crate_to_arm(struct Thing *creatng, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    struct Thing *cratng;
    struct Thing *armtng;
    long n;
    for (n=0; true; n++)
    {
        cratng = check_place_to_pickup_crate(creatng, stl_x, stl_y, TngFRPickF_AllowStoredInOwnedRoom, n);
        if (thing_is_invalid(cratng)) {
            armtng = INVALID_THING;
            break;
        }
        // Allow only trap boxes on that subtile which have a corresponding trap to be armed
        if (thing_is_trap_crate(cratng))
        {
            armtng = check_for_empty_trap_for_imp_not_being_armed(creatng, crate_thing_to_workshop_item_model(cratng));
            if (!thing_is_invalid(armtng)) {
                break;
            }
        }
    }
    // Either the crate or thing to arm is gone - remove the task
    if (thing_is_invalid(cratng))
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    // The task is being covered by another creature
    if (imp_will_soon_be_working_at_excluding(creatng, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(creatng, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    creatng->continue_state = CrSt_CreaturePicksUpTrapObject;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->pickup_object_id = cratng->index;
    cctrl->arming_thing_id = armtng->index;
    return 1;
}

long check_out_worker_pickup_trap_for_workshop(struct Thing *thing, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    long i;
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (!player_has_room_of_role(thing->owner, RoRoF_CratesStorage)) {
        return 0;
    }
    struct Thing *sectng;
    sectng = check_place_to_pickup_crate(thing, stl_x, stl_y, TngFRPickF_Default, 0);
    if (thing_is_invalid(sectng))
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    struct Room *room;
    room = find_nearest_room_for_thing_with_spare_item_capacity(thing, thing->owner, RoK_WORKSHOP, NavRtF_Default);
    if (room_is_invalid(room))
    {
        update_cannot_find_room_wth_spare_capacity_event(thing->owner, thing, RoK_WORKSHOP);
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    EventIndex evidx;
    i = crate_thing_to_workshop_item_class(sectng);
    if (i == TCls_Trap)
    {
      evidx = event_create_event_or_update_nearby_existing_event(
          subtile_coord_center(stl_x), subtile_coord_center(stl_y),
          EvKind_TrapCrateFound, thing->owner, sectng->index);
        if (evidx > 0)
        {

            if ( (is_my_player_number(thing->owner)) && (!is_my_player_number(sectng->owner)) )
            {
                if (sectng->owner == game.neutral_player_num)
                {
                    output_message(SMsg_DiscoveredTrap, 0, true);
                }
            }
        }
    } else
    if (i == TCls_Door)
    {
      evidx = event_create_event_or_update_nearby_existing_event(
          subtile_coord_center(stl_x), subtile_coord_center(stl_y),
          EvKind_DoorCrateFound, thing->owner, sectng->index);
        if (evidx > 0)
        {
            if ( (is_my_player_number(thing->owner)) && (!is_my_player_number(sectng->owner)) )
            {
                if (sectng->owner == game.neutral_player_num)
                {
                    output_message(SMsg_DiscoveredDoor, 0, true);
                }
            }
        }
    } else
    {
        WARNLOG("Strange pickup (class %d) - no event",(int)i);
    }
    thing->continue_state = CrSt_CreaturePicksUpCrateForWorkshop;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->pickup_object_id = sectng->index;
    return 1;
}

long check_out_worker_dig_or_mine(struct Thing *thing, struct DiggerStack *dstack)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    long i;
    i = find_dig_from_task_list(thing->owner, dstack->stl_num);
    if (i == -1)
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    stl_x = 0; stl_y = 0;
    if (!check_place_to_dig_and_get_position(thing, dstack->stl_num, &stl_x, &stl_y))
    {
        dstack->task_type = DigTsk_None;
        return -1;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->digger.task_idx = i;
    cctrl->digger.task_stl = dstack->stl_num;
    cctrl->digger.last_did_job = SDLstJob_DigOrMine;
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

long check_out_worker_pickup_gold_pile(struct Thing *thing, struct DiggerStack *dstack)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    if (crstat->gold_hold <= thing->creature.gold_carried)
    {
        if (game.play_gameturn - cctrl->tasks_check_turn > 128)
        {
          check_out_imp_has_money_for_treasure_room(thing);
          cctrl->tasks_check_turn = game.play_gameturn;
        }
        return 1;
    }
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = stl_num_decode_x(dstack->stl_num);
    stl_y = stl_num_decode_y(dstack->stl_num);
    if (!check_place_to_pickup_gold(thing, stl_x, stl_y))
    {
        dstack->task_type = DigTsk_None;
        return 0;
    }
    if (imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y))
    {
        return 0;
    }
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
    {
        // Do not delete the task - another digger might be able to reach it
        return 0;
    }
    thing->continue_state = CrSt_ImpPicksUpGoldPile;
    return 1;
}

TbBool check_out_imp_stack(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct DiggerStack *dstack;
    long ret;
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    cctrl = creature_control_get_from_thing(creatng);
    dungeon = get_dungeon(creatng->owner);
    // If digger stack was re-filled in the meantime, reset current pos
    if (cctrl->digger.stack_update_turn != dungeon->digger_stack_update_turn)
    {
      cctrl->digger.stack_update_turn = dungeon->digger_stack_update_turn;
      cctrl->digger.task_stack_pos = 0;
    }
    if (dungeon->digger_stack_length > DIGGER_TASK_MAX_COUNT)
    {
        ERRORLOG("Digger tasks length %d out of range",(int)dungeon->digger_stack_length);
        dungeon->digger_stack_length = DIGGER_TASK_MAX_COUNT;
    }
    while (cctrl->digger.task_stack_pos < dungeon->digger_stack_length)
    {
        dstack = &dungeon->digger_stack[cctrl->digger.task_stack_pos];
        SYNCDBG(18,"Checking task %d, type %d",(int)cctrl->digger.task_stack_pos,(int)dstack->task_type);
        cctrl->digger.task_stack_pos++;
        SpDiggerTaskType task_type;
        task_type = dstack->task_type;
        switch (task_type)
        {
        case DigTsk_ImproveDungeon:
            ret = check_out_worker_improve_dungeon(creatng, dstack);
            break;
        case DigTsk_ConvertDungeon:
            ret = check_out_worker_convert_dungeon(creatng, dstack);
            break;
        case DigTsk_ReinforceWall:
            ret = check_out_worker_reinforce_wall(creatng, dstack);
            break;
        case DigTsk_PickUpUnconscious:
            ret = check_out_worker_pickup_unconscious(creatng, dstack);
            break;
        case DigTsk_PickUpCorpse:
            ret = check_out_worker_pickup_corpse(creatng, dstack);
            break;
        case DigTsk_PicksUpSpellBook:
            ret = check_out_worker_pickup_spellbook(creatng, dstack);
            break;
        case DigTsk_PicksUpCrateToArm:
            ret = check_out_worker_pickup_crate_to_arm(creatng, dstack);
            break;
        case DigTsk_PicksUpCrateForWorkshop:
            ret = check_out_worker_pickup_trap_for_workshop(creatng, dstack);
            break;
        case DigTsk_PicksUpGoldPile:
            ret = check_out_worker_pickup_gold_pile(creatng, dstack);
            break;
        case DigTsk_DigOrMine:
            ret = check_out_worker_dig_or_mine(creatng, dstack);
            break;
        case DigTsk_None:
            ret = 0;
            break;
        default:
            ret = 0;
            ERRORLOG("Invalid stack task type, %d",(int)task_type);
            dstack->task_type = DigTsk_None;
            break;
        }
        if (ret > 0) {
            SYNCDBG(9,"Assigned task type %d, new state %s",task_type,creature_state_code_name(get_creature_state_besides_interruptions(creatng)));
            return true;
        } else if (ret < 0) {
            SYNCDBG(9,"Task type %d was impossible",(int)task_type);
            return false;
        }
        SYNCDBG(19,"No task");
    }
    return false;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
