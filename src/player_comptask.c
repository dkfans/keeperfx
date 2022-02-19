/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_comptask.c
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Defines a computer player control variables and events/checks/processes
 *      functions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_computer.h"

#include <limits.h>
#include <string.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_memory.h"
#include "bflib_sound.h"
#include "bflib_math.h"

#include "config.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "creature_jobs.h"
#include "creature_states_lair.h"
#include "creature_states_combt.h"
#include "creature_states_mood.h"
#include "magic.h"
#include "thing_traps.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "player_instances.h"
#include "player_utils.h"
#include "room_jobs.h"
#include "room_workshop.h"

#include "dungeon_data.h"
#include "map_blocks.h"
#include "map_data.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "slab_data.h"
#include "power_hand.h"
#include "power_process.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TrapDoorSelling {
    long category;
    long model;
};

struct MoveToBestJob {
    CreatureJob job_kind;
    long priority;
};

struct MyLookup {
    long delta_x;
    long delta_y;
};

/******************************************************************************/
long task_dig_room_passage(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_room(struct Computer2 *comp, struct ComputerTask *ctask);
long task_check_room_dug(struct Computer2 *comp, struct ComputerTask *ctask);
long task_place_room(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_entrance(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_gold(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_attack(struct Computer2 *comp, struct ComputerTask *ctask);
long task_magic_call_to_arms(struct Computer2 *comp, struct ComputerTask *ctask);
long task_pickup_for_attack(struct Computer2 *comp, struct ComputerTask *ctask);
long task_move_creature_to_room(struct Computer2 *comp, struct ComputerTask *ctask);
long task_move_creature_to_pos(struct Computer2 *comp, struct ComputerTask *ctask);
long task_move_creatures_to_defend(struct Computer2 *comp, struct ComputerTask *ctask);
long task_slap_imps(struct Computer2 *comp, struct ComputerTask *ctask);
long task_dig_to_neutral(struct Computer2 *comp, struct ComputerTask *ctask);
long task_magic_speed_up(struct Computer2 *comp, struct ComputerTask *ctask);
long task_wait_for_bridge(struct Computer2 *comp, struct ComputerTask *ctask);
long task_attack_magic(struct Computer2 *comp, struct ComputerTask *ctask);
long task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask);
long task_move_gold_to_treasury(struct Computer2 *comp, struct ComputerTask *ctask);
long find_next_gold(struct Computer2 *, struct ComputerTask *);
long check_for_gold(MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, long plyr_idx);
/******************************************************************************/
/**
 * Computer tasks definition array.
 * The task position corresponds to ComputerTaskTypes enumeration index.
 */
const struct TaskFunctions task_function[] = {
    {NULL, NULL},
    {"COMPUTER_DIG_ROOM_PASSAGE", task_dig_room_passage},
    {"COMPUTER_DIG_ROOM",         task_dig_room},
    {"COMPUTER_CHECK_ROOM_DUG",   task_check_room_dug},
    {"COMPUTER_PLACE_ROOM",       task_place_room},
    {"COMPUTER_DIG_TO_ENTRANCE",  task_dig_to_entrance},
    {"COMPUTER_DIG_TO_GOLD",      task_dig_to_gold},
    {"COMPUTER_DIG_TO_ATTACK",    task_dig_to_attack},
    {"COMPUTER_MAGIC_CALL_TO_ARMS", task_magic_call_to_arms},
    {"COMPUTER_PICKUP_FOR_ATTACK", task_pickup_for_attack},
    {"COMPUTER_MOVE_CREATURE_TO_ROOM", task_move_creature_to_room},
    {"COMPUTER_MOVE_CREATURE_TO_POS", task_move_creature_to_pos},
    {"COMPUTER_MOVE_CREATURES_TO_DEFEND", task_move_creatures_to_defend},
    {"COMPUTER_SLAP_IMPS",        task_slap_imps},
    {"COMPUTER_DIG_TO_NEUTRAL",   task_dig_to_neutral},
    {"COMPUTER_MAGIC_SPEED_UP",   task_magic_speed_up},
    {"COMPUTER_WAIT_FOR_BRIDGE",  task_wait_for_bridge},
    {"COMPUTER_ATTACK_MAGIC",     task_attack_magic},
    {"COMPUTER_SELL_TRAPS_AND_DOORS", task_sell_traps_and_doors},
    {"COMPUTER_MOVE_GOLD_TO_TREASURY",task_move_gold_to_treasury},
};

const struct TrapDoorSelling trapdoor_sell[] = {
    {TDSC_DoorCrate,  4},
    {TDSC_TrapCrate,  1},
    {TDSC_TrapCrate,  6},
    {TDSC_DoorCrate,  3},
    {TDSC_TrapCrate,  5},
    {TDSC_TrapCrate,  4},
    {TDSC_DoorCrate,  2},
    {TDSC_TrapCrate,  3},
    {TDSC_DoorCrate,  1},
    {TDSC_TrapCrate,  2},
    {TDSC_DoorPlaced, 4},
    {TDSC_TrapPlaced, 1},
    {TDSC_TrapPlaced, 6},
    {TDSC_DoorPlaced, 3},
    {TDSC_TrapPlaced, 5},
    {TDSC_TrapPlaced, 4},
    {TDSC_DoorPlaced, 2},
    {TDSC_TrapPlaced, 3},
    {TDSC_DoorPlaced, 1},
    {TDSC_TrapPlaced, 2},
    {TDSC_EndList,    0},
};

const struct MoveToBestJob move_to_best_job[] = {
    {Job_TRAIN,       60},
    {Job_RESEARCH,    45},
    {Job_MANUFACTURE, 28},
    {Job_SCAVENGE,    20},
    {Job_GUARD,        2},
    {Job_BARRACK,      1},
    {Job_NULL,         0},
};

const struct MyLookup lookup[] = {
    { 0, -3},
    { 3,  0},
    { 0,  3},
    {-3,  0}
};

/******************************************************************************/
DLLIMPORT struct Thing *_DK_find_creature_for_call_to_arms(struct Computer2 *comp, long a2);
DLLIMPORT long _DK_count_creatures_in_call_to_arms(struct Computer2 *comp);
DLLIMPORT struct ComputerTask *_DK_get_free_task(struct Computer2 *comp, long basestl_y);
DLLIMPORT int _DK_search_spiral(struct Coord3d *pos, int owner, int i3, long (*cb)(long, long, long));
DLLIMPORT long _DK_other_build_here(struct Computer2 *comp, long a2, long round_directn, long plyr_idx, long slabs_dist);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct ComputerTask *get_computer_task(long idx)
{
    if ((idx < 1) || (idx >= COMPUTER_TASKS_COUNT))
    {
        return INVALID_COMPUTER_TASK;
    } else
    {
        return &game.computer_task[idx];
    }
}

TbBool computer_task_invalid(const struct ComputerTask *ctask)
{
    if (ctask <= &game.computer_task[0])
        return true;
    return false;
}

int computer_task_index(struct ComputerTask *ctask)
{
    long i;
    i = ((char *)ctask - (char *)&game.computer_task[0]);
    if ( (i < 0) || (i > COMPUTER_TASKS_COUNT*sizeof(struct ComputerTask)) ) {
        ERRORLOG("Task is outside of Game.");
        return 0;
    }
    return i / sizeof(struct ComputerTask);
}

const char *computer_task_code_name(int ctask_type)
{
    const char * ctask_name;
    ctask_name = NULL;
    if ((ctask_type > 0) && (ctask_type < sizeof(task_function)/sizeof(task_function[0]))) {
        ctask_name = task_function[ctask_type].name;
    }
    if (ctask_name == NULL)
        return "INVALID";
    return ctask_name;
}

/** Removes task from Computer2 structure and marks it as unused.
 *
 * @param comp Computer from which task is removed.
 * @param ctask Task to be removed from the computer tasks list.
 * @return
 */
TbBool remove_task(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct ComputerTask *nxctask;
    long i;
    i = comp->task_idx;
    if (computer_task_index(ctask) == i)
    {
        //Removing first task in list
        comp->task_idx = ctask->next_task;
        ctask->next_task = 0;
        ctask->flags &= ~ComTsk_Unkn0001;
        return true;
    }
    nxctask = get_computer_task(i);
    while (!computer_task_invalid(nxctask))
    {
        i = nxctask->next_task;
        if (computer_task_index(ctask) == i)
        {
          nxctask->next_task = ctask->next_task;
          ctask->next_task = 0;
          ctask->flags &= ~ComTsk_Unkn0001;
          return true;
        }
        nxctask = get_computer_task(i);
    }
    return false;
}

void restart_task_process(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct ComputerProcess *cproc;
    cproc = get_computer_process(comp, ctask->field_8C);
    if (cproc != NULL)
    {
        struct ComputerProcess *onproc;
        onproc = get_computer_process(comp, comp->ongoing_process);
        if (onproc != cproc)
        {
            cproc->flags &= ~ComProc_Unkn0020;
            cproc->flags &= ~ComProc_Unkn0008;
        }
    } 
    else 
    {
        ERRORLOG("Invalid computer process %d referenced",(int)ctask->field_8C);
    }
    remove_task(comp, ctask);
}

void suspend_task_process(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct ComputerProcess *cproc;
    cproc = get_computer_process(comp, ctask->field_8C);
    suspend_process(comp, cproc);
    remove_task(comp, ctask);
}

TbResult game_action(PlayerNumber plyr_idx, unsigned short gaction, unsigned short alevel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2)
{
    struct Dungeon *dungeon;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    struct Thing *thing;
    long i;
    long k;
    struct SlabMap *slb;
    struct Room *room;
    SYNCDBG(9,"Starting action %d",(int)gaction);
    if (subtile_has_slab(stl_x, stl_y)) {
        slb_x = subtile_slab_fast(stl_x);
        slb_y = subtile_slab_fast(stl_y);
    } else {
        slb_x = -1;
        slb_y = -1;
    }
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
      return Lb_FAIL;
    }
    switch (gaction)
    {
    case GA_Unk01:
        break;
    case GA_UsePwrHandPick:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_HAND, 0,thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, PwMod_Default);
    case GA_UsePwrHandDrop:
        // Note that we can drop things even if we have no hand power
        if (!dump_first_held_thing_on_map(plyr_idx, stl_x, stl_y, 0))
            return Lb_FAIL;
        return Lb_SUCCESS;
    case GA_UseMkDigger:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_MKDIGGER, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_UsePwrSight:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_SIGHT, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_UsePwrObey:
        return magic_use_available_power_on_level(plyr_idx, PwrK_OBEY, alevel, PwMod_Default);
    case GA_UsePwrHealCrtr:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_HEALCRTR, alevel, stl_x, stl_y, thing, PwMod_Default);
    case GA_UsePwrCall2Arms:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_CALL2ARMS, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_UsePwrCaveIn:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_CAVEIN, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_StopPwrCall2Arms:
        turn_off_power_call_to_arms(plyr_idx);
        return Lb_SUCCESS;
    case GA_StopPwrHoldAudnc:
        dungeon->hold_audience_cast_turn = 0;
        return Lb_SUCCESS;
    case GA_Unk13:
    case GA_MarkDig:
        slb = get_slabmap_block(slb_x, slb_y);
        if ((slb->kind == SlbT_LAVA) || (slb->kind == SlbT_WATER))
        {
            // Computer player can turn water/lava into path; but only in very special circumstances
            // Normally liquid tiles are not marked for digging by the tool function
            place_slab_type_on_map(SlbT_PATH, stl_x, stl_y, plyr_idx, 0);
            do_slab_efficiency_alteration(slb_x, slb_y);
            i = Lb_SUCCESS;
        } else
        {
            i = tag_blocks_for_digging_in_rectangle_around(slab_subtile(slb_x,0), slab_subtile(slb_y,0), plyr_idx) > 0;
        }
        return i;
    case GA_Unk15:
    case GA_PlaceRoom:
        room = player_build_room_at(stl_x, stl_y, plyr_idx, param2);
        if (room_is_invalid(room))
            break;
        return Lb_SUCCESS;
    case GA_SetTendencies:
        dungeon->creature_tendencies = param1;
        return Lb_SUCCESS;
    case GA_PlaceTrap:
        if (!player_place_trap_at(stl_x, stl_y, plyr_idx, param1))
            break;
        return Lb_SUCCESS;
    case GA_PlaceDoor:
        k = tag_cursor_blocks_place_door(plyr_idx, stl_x, stl_y);
        i = packet_place_door(stl_x, stl_y, plyr_idx, param1, k);
        return i;
    case GA_SellTrap:
        return player_sell_trap_at_subtile(plyr_idx, stl_x, stl_y);
    case GA_SellDoor:
        return player_sell_door_at_subtile(plyr_idx, stl_x, stl_y);
    case GA_UsePwrLightning:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_LIGHTNING, alevel, stl_x, stl_y, PwMod_Default);
    case GA_UsePwrSpeedUp:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_SPEEDCRTR, alevel, stl_x, stl_y, thing, PwMod_Default);
    case GA_UsePwrArmour:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_PROTECT, alevel, stl_x, stl_y, thing, PwMod_Default);
    case GA_UsePwrConceal:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_CONCEAL, alevel, stl_x, stl_y, thing, PwMod_Default);
    case GA_UsePwrHoldAudnc:
        return magic_use_available_power_on_level(plyr_idx, PwrK_HOLDAUDNC, alevel, PwMod_Default);
    case GA_UsePwrDisease:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_DISEASE, alevel, stl_x, stl_y, thing, PwMod_Default);
    case GA_UsePwrChicken:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_CHICKEN, alevel, stl_x, stl_y, thing, PwMod_Default);
    case GA_UseSlap:
    case GA_UsePwrSlap:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, alevel, stl_x, stl_y, thing, PwMod_Default);
    default:
        ERRORLOG("Unknown game action %d", (int)gaction);
        break;
    }
    return Lb_OK;
}

TbResult try_game_action(struct Computer2 *comp, PlayerNumber plyr_idx, unsigned short gaction, unsigned short alevel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2)
{
    TbResult result;
    result = game_action(plyr_idx, gaction, alevel, stl_x, stl_y, param1, param2);
    if (result > Lb_OK) {
        comp->tasks_did--;
    }
    SYNCDBG(19,"Returning %d",(int)result);
    return result;
}

/**
 * Returns first task of given type from given computer player in progress tasks list.
 * @param comp The computer player to be checked.
 * @param ttype Task type to search for.
 * @return The task pointer, or invalid task pointer if not found.
 */
struct ComputerTask *get_task_in_progress(struct Computer2 *comp, ComputerTaskType ttype)
{
    struct ComputerTask *ctask;
    long i;
    unsigned long k;
    k = 0;
    i = comp->task_idx;
    while (i != 0)
    {
        if ((i < 0) || (i >= COMPUTER_TASKS_COUNT))
        {
          ERRORLOG("Jump to invalid computer task %ld detected",i);
          break;
        }
        ctask = &game.computer_task[i];
        i = ctask->next_task;
        if ((ctask->flags & ComTsk_Unkn0001) != 0)
        {
            long n;
            n = ctask->ttype;
            // If it's a sub-task, compare the main task behind it
            if (n == CTT_WaitForBridge)
                n = ctask->ottype;
            if (n == ttype) {
                return ctask;
            }
        }
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping computer tasks");
          break;
        }
    }
    // TODO COMPUTER change to INVALID_COMPUTER_TASK when all functions can handle this value correctly
    return NULL;
}

/**
 * Returns first task of given type from given computer player in progress tasks list.
 * @param comp The computer player to be checked.
 * @param ttype Task type to search for.
 * @return The task pointer, or invalid task pointer if not found.
 */
struct ComputerTask *get_task_in_progress_in_list(const struct Computer2 *comp, const ComputerTaskType *ttypes)
{
    struct ComputerTask *ctask;
    long i;
    unsigned long k;
    k = 0;
    i = comp->task_idx;
    while (i != 0)
    {
        if ((i < 0) || (i >= COMPUTER_TASKS_COUNT))
        {
          ERRORLOG("Jump to invalid computer task %ld detected",i);
          break;
        }
        ctask = &game.computer_task[i];
        i = ctask->next_task;
        if ((ctask->flags & ComTsk_Unkn0001) != 0)
        {
            long n;
            n = ctask->ttype;
            // If it's a sub-task, compare the main task behind it
            if (n == CTT_WaitForBridge)
                n = ctask->ottype;
            const ComputerTaskType *ttype;
            for (ttype = ttypes; *ttype > CTT_None; ttype++)
            {
                if (n == *ttype) {
                    return ctask;
                }
            }
        }
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping computer tasks");
          break;
        }
    }
    // TODO COMPUTER change to INVALID_COMPUTER_TASK when all functions can handle this value correctly
    return NULL;
}

/**
 * Checks if given computer player has in progress task of given type.
 * @param comp The computer player to be checked.
 * @param ttype Task type to search for.
 * @return True if computer player has at least one such task, false otherwise.
 */
TbBool is_task_in_progress(struct Computer2 *comp, ComputerTaskType ttype)
{
    const struct ComputerTask *ctask;
    ctask = get_task_in_progress(comp, ttype);
    return !computer_task_invalid(ctask);
}

struct ComputerTask *get_free_task(struct Computer2 *comp, long a2)
{
    return _DK_get_free_task(comp, a2);
}

TbBool is_task_in_progress_using_hand(struct Computer2 *comp)
{
    return is_task_in_progress(comp, CTT_PickupForAttack) ||
        is_task_in_progress(comp, CTT_MoveCreatureToRoom) ||
        is_task_in_progress(comp, CTT_MoveCreatureToPos) ||
        is_task_in_progress(comp, CTT_MoveCreaturesToDefend) ||
        is_task_in_progress(comp, CTT_MoveGoldToTreasury);
}

/**
 * Low level function which unconditionally picks creature by computer player to hand.
 * @param comp
 * @param thing
 */
void computer_pick_thing_by_hand(struct Computer2 *comp, struct Thing *thing)
{
    if (thing_is_creature(thing)) {
        reset_creature_if_affected_by_cta(thing);
        clear_creature_instance(thing);
        external_set_thing_state(thing, CrSt_InPowerHand);
        remove_all_traces_of_combat(thing);
    }
    place_thing_in_limbo(thing);
}

/** Checks if given thing is placed in power hand of given player.
 *
 * @param thing
 * @param plyr_idx
 * @return
 */
TbBool thing_is_in_computer_power_hand_list(const struct Thing *thing, PlayerNumber plyr_idx)
{
    struct Computer2 *comp;
    comp = get_computer_player(plyr_idx);
    return (comp->held_thing_idx == thing->index);
}

/**
 * Dumps computer player held things on map.
 * @param comp
 * @param thing
 * @param pos
 * @note originally named fake_dump_held_creatures_on_map()
 */
short computer_dump_held_things_on_map(struct Computer2 *comp, struct Thing *droptng, struct Coord3d *pos)
{
    if (thing_is_creature(droptng) && (droptng->active_state == CrSt_CreatureUnconscious)) {
        WARNLOG("The %s Held By computer is unconscious",creature_code_name(droptng->model));
    }
    if (!computer_find_non_solid_block(comp, pos)) {
        return 0;
    }
    if (!can_place_thing_here(droptng, pos->x.stl.num, pos->y.stl.num, comp->dungeon->owner)) {
        return 0;
    }
    struct Coord3d locpos;
    locpos.z.val = 0;
    locpos.x.val = subtile_coord_center(pos->x.stl.num);
    locpos.y.val = subtile_coord_center(pos->y.stl.num);
    locpos.z.val = get_thing_height_at(droptng, &locpos);
    int height;
    int max_height;
    max_height = get_ceiling_height_above_thing_at(droptng, &locpos);
    height = locpos.z.val + droptng->clipbox_size_yz;
    if (max_height <= height) {
        ERRORLOG("Ceiling is too low to drop %s at (%d,%d)", thing_model_name(droptng),(int)locpos.x.stl.num,(int)locpos.y.stl.num);
        return 0;
    }
    int i;
    i = max_height - height;
    if (i < 0) {
        i = 0;
    } else
    if (i > subtile_coord(3,0)) {
        i = subtile_coord(3,0);
    }
    locpos.z.val += i;
    if (thing_is_object(droptng) && object_is_gold_pile(droptng))
    {
        drop_gold_coins(pos, droptng->valuable.gold_stored, comp->dungeon->owner);
        if (is_my_player_number(comp->dungeon->owner)) {
            play_non_3d_sample(88);
        }
        delete_thing_structure(droptng, 0);
    } else
    {
        drop_held_thing_on_ground(comp->dungeon, droptng, &locpos);
    }
    comp->held_thing_idx = 0;
    comp->tasks_did--;
    return 1;
}

long computer_place_thing_in_power_hand(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos)
{
    SYNCDBG(9,"Player %d picks %s index %d",(int)comp->dungeon->owner,thing_model_name(thing),(int)thing->index);
    if (!can_thing_be_picked_up_by_player(thing, comp->dungeon->owner)) {
        ERRORLOG("Computer tries to pick up %s index %d which is not pickable", thing_model_name(thing),(int)thing->index);
        return 0;
    }
    if ((thing->alloc_flags & TAlF_IsControlled) != 0) {
        SYNCDBG(7,"Computer tries to pick up %s index %d which is being controlled", thing_model_name(thing),(int)thing->index);
        return 0;
    }
    if (!computer_find_non_solid_block(comp, pos)) {
        SYNCDBG(7,"Computer tries to pick up %s index %d which is to be dropped on solid block", thing_model_name(thing),(int)thing->index);
        return 0;
    }
    if (!can_place_thing_here(thing, pos->x.stl.num, pos->y.stl.num, comp->dungeon->owner)) {
        SYNCDBG(7,"Computer tries to pick up %s index %d which cannot be dropped at given place", thing_model_name(thing),(int)thing->index);
        return 0;
    }
    computer_pick_thing_by_hand(comp, thing);
    comp->held_thing_idx = thing->index;
    comp->tasks_did--;
    return 1;
}

/**
 * Dumps computer player held creatures on map, without checking if target position is valid.
 * @param comp
 * @param thing
 * @param pos
 * @note originally named fake_dump_held_creatures_on_map()
 */
TbBool computer_force_dump_held_things_on_map(struct Computer2 *comp, const struct Coord3d *pos)
{
    SYNCDBG(7,"Starting");
    struct Thing *thing;
    // Remove thing from hand
    thing = thing_get(comp->held_thing_idx);
    if (thing_is_invalid(thing)) {
        return false;
    }
    struct Coord3d locpos;
    locpos.z.val = 0;
    locpos.x.val = subtile_coord_center(pos->x.stl.num);
    locpos.y.val = subtile_coord_center(pos->y.stl.num);
    locpos.z.val = get_thing_height_at(thing, &locpos);
    drop_held_thing_on_ground(comp->dungeon, thing, &locpos);
    comp->held_thing_idx = 0;
    return true;
}

TbBool computer_force_dump_specific_held_thing(struct Computer2 *comp, struct Thing *thing, const struct Coord3d *pos)
{
    if (comp->held_thing_idx != thing->index) {
        return false;
    }
    struct Coord3d locpos;
    locpos.z.val = 0;
    locpos.x.val = subtile_coord_center(pos->x.stl.num);
    locpos.y.val = subtile_coord_center(pos->y.stl.num);
    locpos.z.val = get_thing_height_at(thing, &locpos);
    drop_held_thing_on_ground(comp->dungeon, thing, &locpos);
    comp->held_thing_idx = 0;
    return true;
}

TbBool creature_could_be_placed_in_better_room(const struct Computer2 *comp, const struct Thing *thing)
{
    const struct Dungeon *dungeon;
    struct Room *chosen_room;
    long k;
    TbBool better_job_allowed;
    SYNCDBG(19,"Starting for %s index %d owner %d",thing_model_name(thing),(int)thing->index,(int)thing->owner);
    dungeon = comp->dungeon;
    // Choose the room we're currently working in, and check it on the list
    chosen_room = get_room_creature_works_in(thing);
    if (!room_exists(chosen_room)) {
        return true;
    }
    better_job_allowed = false;
    for (k=0; move_to_best_job[k].job_kind != Job_NULL; k++)
    {
        RoomRole rrole;
        rrole = get_room_role_for_job(move_to_best_job[k].job_kind);
        // If we already have this job
        if ((get_room_roles(chosen_room->kind) & rrole) != 0)
        {
            // Jobs below have smaller priority, so we can return here
            return better_job_allowed;
        }
        if (player_has_room_of_role(dungeon->owner, rrole)
         && creature_can_do_job_for_computer_player_in_room_role(thing, dungeon->owner, rrole)
         && (worker_needed_in_dungeons_room_role(dungeon, rrole) > 0)) {
            better_job_allowed = true;
        }
    }
    // If current job wasn't matched, be careful with trying to find better one
    if (better_job_allowed)
    {
        return !creature_is_celebrating(thing) && !creature_is_being_summoned(thing)
            && !creature_is_doing_garden_activity(thing) && !creature_is_taking_salary_activity(thing)
            && !creature_is_sleeping(thing) && !creature_is_doing_toking(thing)
            && !creature_is_being_sacrificed(thing) && !creature_is_being_scavenged(thing);
    }
    return false;
}

CreatureJob get_job_to_place_creature_in_room(const struct Computer2 *comp, const struct Thing *thing)
{
    const struct Dungeon *dungeon;
    long chosen_priority;
    CreatureJob chosen_job;
    struct Room *room;
    long total_spare_cap;
    long i;
    long k;

    dungeon = comp->dungeon;

    chosen_job = Job_NULL;
    chosen_priority = LONG_MIN;
    for (k=0; move_to_best_job[k].job_kind != Job_NULL; k++)
    {
        const struct MoveToBestJob * mvto;
        mvto = &move_to_best_job[k];
        if (!creature_can_do_job_for_player(thing, dungeon->owner, mvto->job_kind, JobChk_None)) {
            continue;
        }
        RoomRole rrole;
        rrole = get_room_role_for_job(mvto->job_kind);
        long need_value;
        need_value = worker_needed_in_dungeons_room_role(dungeon, rrole);
        if (need_value == 0) {
            SYNCDBG(9,"Cannot assign %s for %s index %d; no worker needed",creature_job_code_name(mvto->job_kind),thing_model_name(thing),(int)thing->index);
            continue;
        }
        RoomKind rkind;
        rkind = get_room_for_job(mvto->job_kind);
        // Find specific room which meets capacity demands
        i = dungeon->room_kind[rkind];
        room = find_room_with_most_spare_capacity_starting_with(i,&total_spare_cap);
        if (room_is_invalid(room)) {
            SYNCDBG(9,"Cannot assign %s for %s index %d; no room with spares",creature_job_code_name(mvto->job_kind),thing_model_name(thing),(int)thing->index);
            continue;
        }
        // Check whether putting the creature to that room will make someone go postal
        if (any_worker_will_go_postal_on_creature_in_room(room, thing)) {
            SYNCDBG(9,"Cannot assign %s for %s index %d; others would go postal",creature_job_code_name(mvto->job_kind),thing_model_name(thing),(int)thing->index);
            continue;
        }
        // Work value affects priority
        long work_value;
        work_value = compute_creature_work_value_for_room_role(thing, rrole, room->efficiency);
        work_value = max(work_value/256,1);
        // Now compute the priority
        long new_priority;
        new_priority = (LbSqrL(total_spare_cap) + need_value*need_value + work_value) * mvto->priority;
        SYNCDBG(9,"Checking job %s for %s index %d, cap %ld needval %ld wrkval %ld priority %ld",creature_job_code_name(mvto->job_kind),thing_model_name(thing),(int)thing->index,(long)total_spare_cap,(long)need_value,(long)work_value,(long)new_priority);
        if (chosen_priority < new_priority)
        {
            chosen_priority = new_priority;
            chosen_job = mvto->job_kind;
        }
    }
    SYNCDBG(9,"Chosen %s as best job for %s index %d, with priority %ld",creature_job_code_name(chosen_job),thing_model_name(thing),(int)thing->index,(long)chosen_priority);
    return chosen_job;
}

CreatureJob find_creature_to_be_placed_in_room_for_job(struct Computer2 *comp, struct Room **roomp, struct Thing ** creatngp)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *thing;
    struct Room *room;
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon in computer player");
        return Job_NULL;
    }
    SYNCDBG(9,"Starting for player %d",(int)dungeon->owner);
    param.ptr1 = (void *)comp;
    param.num2 = Job_NULL; // Our filter function will update that
    filter = player_list_creature_filter_needs_to_be_placed_in_room_for_job;
    thing = get_player_list_random_creature_with_filter(dungeon->creatr_list_start, filter, &param, dungeon->owner);
    if (thing_is_invalid(thing)) {
        return Job_NULL;
    }
    SYNCDBG(9,"Player %d wants to move %s index %d for job %s",(int)dungeon->owner,thing_model_name(thing),(int)thing->index,creature_job_code_name(param.num2));
    // We won't allow the creature to be picked if we want it to be placed in the same room it is now.
    // The filter function took care of most such situations, but it is still possible that the creature
    // won't be able or will not want to work in that room, and will be picked up and dropped over and over.
    // This will prevent such situation, at least to the moment when the creature leaves the room.
    room = get_room_thing_is_on(thing);
    if (!room_is_invalid(room) && ((get_room_roles(room->kind) & get_room_role_for_job(param.num2)) != 0) && (room->owner == thing->owner)) {
        // Do not spam with warnings which we know to be expected
        if (!creature_is_called_to_arms(thing) && !creature_is_celebrating(thing)) {
            WARNDBG(4,"The %s index %d owner %d already is in %s, but goes for %s instead of work there",
                thing_model_name(thing),(int)thing->index,(int)thing->owner,room_code_name(room->kind),creatrtng_realstate_name(thing));
        }
        return Job_NULL;
    }
    room = get_room_of_given_role_for_thing(thing, dungeon, get_room_role_for_job(param.num2), 1);
    if (room_is_invalid(room))
        return Job_NULL;
    *roomp = room;
    *creatngp = thing;
    return param.num2;
}

void setup_computer_dig_room(struct ComputerDig *cdig, const struct Coord3d *pos, long a3)
{
    cdig->pos_begin.x.val = pos->x.val;
    cdig->pos_begin.y.val = pos->y.val;
    cdig->pos_begin.z.val = pos->z.val;
    cdig->subfield_30 = 0;
    cdig->subfield_34 = 0;
    cdig->subfield_38 = 0;
    cdig->subfield_3C = 0;
    cdig->subfield_40 = a3;
    cdig->subfield_44 = 0;
    cdig->subfield_2C = 1;
}

long task_dig_room_passage(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    struct Coord3d pos;
    switch (tool_dig_to_pos2(comp, &ctask->dig, 0, ToolDig_BasicOnly))
    {
    case -5:
        ctask->ottype = ctask->ttype;
        ctask->ttype = CTT_WaitForBridge;
        return CTaskRet_Unk4;
    case -3:
    case -2:
        shut_down_task_process(comp, ctask);
        return 0;
    case -1:
        pos.x.val = ctask->pos_6A.x.val;
        pos.y.val = ctask->pos_6A.y.val;
        pos.z.val = ctask->pos_6A.z.val;
        {
            long round_directn;
            round_directn = small_around_index_towards_destination(ctask->pos_6A.x.stl.num,ctask->pos_6A.y.stl.num,
                ctask->pos_64.x.stl.num,ctask->pos_64.y.stl.num);
            pos_move_in_direction_to_last_allowing_drop(&pos, round_directn, comp->dungeon->owner, ctask->create_room.width+ctask->create_room.height);
        }
        move_imp_to_dig_here(comp, &pos, 1);
        pos.x.val = ctask->pos_64.x.val;
        pos.y.val = ctask->pos_64.y.val;
        pos.z.val = ctask->pos_64.z.val;
        setup_computer_dig_room(&ctask->dig, &pos, ctask->create_room.area);
        ctask->ttype = CTT_DigRoom;
        return 1;
    default:
        if ((ctask->flags & ComTsk_AddTrapLocation) != 0)
        {
            ctask->flags &= ~ComTsk_AddTrapLocation;
            add_to_trap_location(comp, &ctask->dig.pos_next);
        }
        return 2;
    }
}

long task_dig_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    {
        int digger_tasks;
        digger_tasks = dungeon->digger_stack_length;
        if ((digger_tasks > 0) && (comp->field_1C * dungeon->total_area / 100 <= digger_tasks)) {
            return 2;
        }
    }
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = stl_slab_center_subtile(ctask->dig.pos_begin.x.stl.num);
    stl_y = stl_slab_center_subtile(ctask->dig.pos_begin.y.stl.num);
    int i;
    for (i=ctask->dig.subfield_2C; i > 0; i--)
    {
        if (ctask->dig.subfield_38 > 0)
        {
            if ((stl_x < map_subtiles_x) && (stl_y < map_subtiles_y))
            {
                struct SlabMap *slb;
                slb = get_slabmap_for_subtile(stl_x, stl_y);
                const struct SlabAttr *slbattr;
                slbattr = get_slab_attrs(slb);
                struct Map *mapblk;
                mapblk = get_map_block_at(stl_x, stl_y);
                if (slbattr->is_diggable && (slb->kind != SlbT_GEMS))
                {
                    if (((mapblk->flags & SlbAtFlg_Filled) == 0) || (slabmap_owner(slb) == dungeon->owner))
                    {
                        if (find_from_task_list(dungeon->owner, get_subtile_number(stl_x,stl_y)) < 0)
                        {
                            if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, stl_x, stl_y, 1, 1) <= Lb_OK)
                            {
                                shut_down_task_process(comp, ctask);
                                return 0;
                            }
                        }
                    }
                }
                ctask->dig.subfield_44++;
                if (ctask->dig.subfield_44 >= ctask->dig.subfield_40) {
                    ctask->ttype = CTT_CheckRoomDug;
                    return 1;
                }
            }
            const struct MyLookup *lkp;
            lkp = &lookup[ctask->dig.subfield_3C];
            stl_x += lkp->delta_x;
            stl_y += lkp->delta_y;
        }
        ctask->dig.subfield_38--;
        if (ctask->dig.subfield_38 <= 0)
        {
            ctask->dig.subfield_30++;
            if ((ctask->dig.subfield_30 & 1) != 0) {
                ctask->dig.subfield_34++;
            }
            ctask->dig.subfield_38 = ctask->dig.subfield_34;
            ctask->dig.subfield_3C = (ctask->dig.subfield_3C + 1) & 3;
        }
    }
    ctask->dig.pos_begin.x.stl.num = stl_x;
    ctask->dig.pos_begin.y.stl.num = stl_y;
    return 2;
}

/**
 * Counts the slabs which are supposed to be used for room building, and which cannot be used for the building.
 */
void count_slabs_where_room_cannot_be_built(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, RoomKind rkind, long slabs_num, long *waiting_slabs, long *wrong_slabs)
{
    SlabKind room_slbkind;
    room_slbkind = room_corresponding_slab(rkind);
    int m;
    int n;
    long nchecked;
    long nwaiting;
    long nwrong;
    int i;
    int imax;
    n = 0;
    imax = 0;
    m = 0;
    nchecked = 0;
    nwaiting = 0;
    nwrong = 0;
    while (n < slabs_num)
    {
        if (nchecked & 1)
          imax++;
        int lkp_x;
        int lkp_y;
        lkp_x = lookup[m].delta_x;
        lkp_y = lookup[m].delta_y;
        for (i = imax; i > 0; i--)
        {
            struct SlabMap *slb;
            slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (!slabmap_block_invalid(slb))
            {
                if (slab_kind_is_liquid(slb->kind) || slab_kind_is_indestructible(slb->kind)) {
                    nwrong++;
                } else
                if ((slb->kind != room_slbkind) && slab_kind_is_room(slb->kind)) {
                    nwrong++;
                } else
                if (((slb->kind != SlbT_CLAIMED) && (slb->kind != room_slbkind)) || (slabmap_owner(slb) != plyr_idx)) {
                    nwaiting++;
                }
                n++;
                if (n >= slabs_num) {
                    (*waiting_slabs) += nwaiting;
                    (*wrong_slabs) += nwrong;
                    return;
                }
            }
            stl_x += lkp_x;
            stl_y += lkp_y;
        }
        m = (m + 1) & 3;
        nchecked++;
    }
    (*waiting_slabs) += nwaiting;
    (*wrong_slabs) += nwrong;
    return;
}

long task_check_room_dug(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    if (game.play_gameturn - ctask->created_turn > COMPUTER_DIG_ROOM_TIMEOUT) {
        WARNLOG("Task %s couldn't be completed in reasonable time, reset",computer_task_code_name(ctask->ttype));
        restart_task_process(comp, ctask);
        return 0;
    }
    long waiting_slabs;
    long wrong_slabs;
    waiting_slabs = 0; wrong_slabs = 0;
    count_slabs_where_room_cannot_be_built(comp->dungeon->owner, ctask->pos_64.x.stl.num, ctask->pos_64.y.stl.num,
        ctask->create_room.long_80, ctask->create_room.area, &waiting_slabs, &wrong_slabs);
    if (wrong_slabs > 0) {
        WARNLOG("Task %s couldn't be completed as %d wrong slabs are in destination area, reset",computer_task_code_name(ctask->ttype),(int)wrong_slabs);
        restart_task_process(comp, ctask);
        return 0;
    }
    if (waiting_slabs > 0) {
        SYNCDBG(9,"The %d/%d tiles around %d,%d are not ready to place room",(int)wrong_slabs,
            (int)ctask->create_room.area, (int)ctask->pos_64.x.stl.num, (int)ctask->pos_64.y.stl.num);
        return 4;
    }
    // The room digging task is complete - change it to room placing task
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        struct RoomConfigStats *roomst;
        roomst = &slab_conf.room_cfgstats[ctask->rkind];
        message_add_fmt(comp->dungeon->owner, "Now I can place the %s.",get_string(roomst->name_stridx));
    }
    ctask->ttype = CTT_PlaceRoom;
    setup_computer_dig_room(&ctask->dig, &ctask->pos_64, ctask->create_room.area);
    return 1;
}

void shut_down_task_process(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct ComputerProcess *cproc;
    SYNCDBG(9,"Starting");
    cproc = get_computer_process(comp, ctask->field_8C);
    if (cproc != NULL)
    {
        if ((cproc->flags & ComProc_Unkn0020) != 0) {
            shut_down_process(comp, cproc);
        }
    } else {
        ERRORLOG("Invalid computer process %d referenced",(int)ctask->field_8C);
    }
    if (!computer_task_invalid(ctask)) {
        remove_task(comp, ctask);
    }
}

long task_place_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    RoomKind rkind;
    struct RoomStats *rstat;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    int i;
    SYNCDBG(9,"Starting");
    dungeon = comp->dungeon;
    rkind = ctask->create_room.long_80;
    rstat = room_stats_get_for_kind(rkind);
    struct RoomConfigStats *roomst;
    roomst = &slab_conf.room_cfgstats[rkind];
    // If we don't have money for the room - don't even try
    if (rstat->cost + 1000 >= dungeon->total_money_owned)
    {
        // Prefer leaving some gold, unless a flag is forcing us to build
        if (((roomst->flags & RoCFlg_BuildToBroke) == 0) || (rstat->cost >= dungeon->total_money_owned)) {
            return 0;
        }
    }
    // If we've lost the ability to build that room - kill the process and remove task (should we really remove task?)
    if (!is_room_available(dungeon->owner, rkind)) {
        shut_down_task_process(comp, ctask);
        return 1;
    }
    stl_x = ctask->dig.pos_begin.x.stl.num;
    stl_y = ctask->dig.pos_begin.y.stl.num;
    for (i = ctask->dig.subfield_2C; i > 0; i--)
    {
        if (ctask->dig.subfield_38 > 0)
        {
            if (slab_has_trap_on(subtile_slab(stl_x), subtile_slab(stl_y))) {
                try_game_action(comp, dungeon->owner, GA_SellTrap, 0, stl_x, stl_y, 1, 0);
            }
            if (can_build_room_at_slab(dungeon->owner, rkind, subtile_slab(stl_x), subtile_slab(stl_y)))
            {
                if (try_game_action(comp, dungeon->owner, GA_PlaceRoom, 0, stl_x, stl_y, 1, rkind) > Lb_OK)
                {
                    ctask->dig.subfield_44++;
                    if (ctask->dig.subfield_40 <= ctask->dig.subfield_44) {
                        shut_down_task_process(comp, ctask);
                        return 1;
                    }
                }
            }
            const struct MyLookup *lkp;
            lkp = &lookup[ctask->dig.subfield_3C];
            stl_x += lkp->delta_x;
            stl_y += lkp->delta_y;
        }
        ctask->dig.subfield_38--;
        if (ctask->dig.subfield_38 <= 0)
        {
            ctask->dig.subfield_30++;
            if (ctask->dig.subfield_30 & 1)
                ctask->dig.subfield_34++;
            ctask->dig.subfield_38 = ctask->dig.subfield_34;
            ctask->dig.subfield_3C = (ctask->dig.subfield_3C + 1) & 3;
        }
    }
    ctask->dig.pos_begin.x.stl.num = stl_x;
    ctask->dig.pos_begin.y.stl.num = stl_y;
    return 0;
}

long task_dig_to_entrance(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    struct Room *room;
    long dig_ret;
    dig_ret = 0;
    int n;
    for (n=0; n < SMALL_AROUND_LENGTH; n++)
    {
        MapSubtlCoord stl_x;
        MapSubtlCoord stl_y;
        stl_x = stl_slab_center_subtile(ctask->dig.pos_begin.x.stl.num) + small_around[n].delta_x;
        stl_y = stl_slab_center_subtile(ctask->dig.pos_begin.y.stl.num) + small_around[n].delta_y;
        room = subtile_room_get(stl_x, stl_y);
        if (!room_is_invalid(room))
        {
            if (room->index == ctask->dig_to_room.target_room_idx) {
                dig_ret = -1;
                break;
            }
        }
    }
    struct ComputerTask *curtask;
    if (dig_ret == 0)
    {
        dig_ret = tool_dig_to_pos2(comp, &ctask->dig, 0, ToolDig_BasicOnly);
        if ((ctask->flags & ComTsk_AddTrapLocation) != 0) {
            ctask->flags &= ~ComTsk_AddTrapLocation;
            add_to_trap_location(comp, &ctask->dig.pos_next);
        }
    }
    switch ( dig_ret )
    {
    case -5:
        ctask->ottype = ctask->ttype;
        ctask->ttype = CTT_WaitForBridge;
        return 4;
    case -3:
    case -2:
        room = room_get(ctask->dig_to_room.target_room_idx);
        room->player_interested[dungeon->owner] |= 0x02;
        curtask = get_computer_task(comp->task_idx);
        if (!computer_task_invalid(curtask)) {
            remove_task(comp, curtask);
        }
        return dig_ret;
    case -1:
        curtask = get_computer_task(comp->task_idx);
        if (!computer_task_invalid(curtask)) {
            remove_task(comp, curtask);
        }
        return dig_ret;
    default:
        return dig_ret;
    }
}

/**
 * Checks if given room kind is available for building by computer player.
 * @param comp Computer player.
 * @param rkind Room kind.
 * @return Gives IAvail_Never if the room isn't available, IAvail_Now if it's available and IAvail_Later if it's researchable.
 */
ItemAvailability computer_check_room_available(const struct Computer2 * comp, long rkind)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if ((rkind < 1) || (rkind >= ROOM_TYPES_COUNT)) {
        return IAvail_Never;
    }
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon in computer player.");
        return IAvail_Never;
    }
    if (!dungeon->room_resrchable[rkind])
        return IAvail_Never;
    if ((dungeon->room_buildable[rkind] & 1) == 0)
        return IAvail_NeedResearch;
    return IAvail_Now;
}

long xy_walkable(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
    struct SlabAttr *slbattr;
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    slbattr = get_slab_attrs(slb);
    if ((slabmap_owner(slb) == plyr_idx) || (plyr_idx == -1))
    {
        if (((slbattr->block_flags & SlbAtFlg_Blocking) == 0) && (slb->kind != SlbT_LAVA)) {
            return true;
        }
        if ((slbattr->block_flags & SlbAtFlg_IsRoom) != 0) {
            return true;
        }
    }
    return false;
}

long check_for_perfect_buildable(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
    struct SlabAttr *slbattr;
    struct SlabMap *slb;
    SubtlCodedCoords stl_num;
    struct Map *mapblk;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    slbattr = get_slab_attrs(slb);
    if (slb->kind == SlbT_GEMS) {
        return 1;
    }
    if (slbattr->category == SlbAtCtg_RoomInterior) {
        return -1;
    }
    if ((slbattr->block_flags & SlbAtFlg_IsRoom) != 0) {
        return -1;
    }
    if (!slab_good_for_computer_dig_path(slb) || (slb->kind == SlbT_WATER)) {
        return -1;
    }
    stl_num = get_subtile_number(stl_slab_center_subtile(stl_x),stl_slab_center_subtile(stl_y));
    if (find_from_task_list(plyr_idx, stl_num) >= 0) {
        return -1;
    }
    if ((slbattr->block_flags & SlbAtFlg_Valuable) != 0) {
        return -1;
    }
    if (slab_kind_is_liquid(slb->kind)) {
        return 1;
    }
    if ( (slbattr->is_diggable == 0) || (slb->kind == SlbT_GEMS) ) {
        return 1;
    }
    mapblk = get_map_block_at_pos(stl_num);
    return ((mapblk->flags & SlbAtFlg_Filled) != 0) && (slabmap_owner(slb) != plyr_idx);
}

long check_for_buildable(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx)
{
    struct SlabAttr *slbattr;
    struct SlabMap *slb;
    SubtlCodedCoords stl_num;
    struct Map *mapblk;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    slbattr = get_slab_attrs(slb);
    if (slbattr->category == SlbAtCtg_RoomInterior) {
        return -1;
    }
    if ((slbattr->block_flags & SlbAtFlg_IsRoom) != 0) {
        return -1;
    }
    if (slb->kind == SlbT_GEMS) {
        return 1;
    }
    if (slab_kind_is_liquid(slb->kind)) {
        return 1;
    }
    if (!slab_good_for_computer_dig_path(slb) || (slb->kind == SlbT_WATER)) {
        return 0;
    }
    stl_num = get_subtile_number(stl_slab_center_subtile(stl_x),stl_slab_center_subtile(stl_y));
    if (find_from_task_list(plyr_idx, stl_num) >= 0) {
        return 0;
    }
    if ( (slbattr->is_diggable == 0) || (slb->kind == SlbT_GEMS) ) {
        return 1;
    }
    mapblk = get_map_block_at_pos(stl_num);
    return ((mapblk->flags & SlbAtFlg_Filled) != 0) && (slabmap_owner(slb) != plyr_idx);
}

long get_corridor(struct Coord3d *pos1, struct Coord3d * pos2, unsigned char round_directn, PlayerNumber plyr_idx, unsigned short slabs_dist)
{
    //return _DK_get_corridor(pos1, pos2, a3, a4, a5);
    struct Coord3d mvpos;
    mvpos.x.val = pos1->x.val;
    mvpos.y.val = pos1->y.val;
    mvpos.z.val = pos1->z.val;
    int i;
    // If we're on room, move to non-room tile
    i = pos_move_in_direction_to_outside_player_room(&mvpos, round_directn, plyr_idx, slabs_dist);
    if (i < 0) {
        return 0;
    }
    // Update original position with non-room tile
    pos1->x.val = mvpos.x.val;
    pos1->y.val = mvpos.y.val;
    // Move to a blocking tile which is not a room
    i = pos_move_in_direction_to_blocking_wall_or_lava(&mvpos, round_directn, plyr_idx, slabs_dist);
    if (i < 0) {
        return 0;
    }
    if (i == slabs_dist) {
        pos2->x.val = mvpos.x.val;
        pos2->y.val = mvpos.y.val;
        return 1;
    }
    // Update original position with first slab to dig out
    pos1->x.val = mvpos.x.val;
    pos1->y.val = mvpos.y.val;
    // Move to unowned filled or water
    i = pos_move_in_direction_to_unowned_filled_or_water(&mvpos, round_directn, plyr_idx, slabs_dist);
    if (i < 0) {
        return 0;
    }
    if (i == slabs_dist) {
        pos2->x.val = mvpos.x.val;
        pos2->y.val = mvpos.y.val;
        return 1;
    }
    return 0;
}

long other_build_here(struct Computer2 *comp, long a2, long a3, long a4, long a5)
{
    return _DK_other_build_here(comp, a2, a3, a4, a5);
}

struct ComputerTask * able_to_build_room(struct Computer2 *comp, struct Coord3d *pos, RoomKind rkind, long width_slabs, long height_slabs, long max_slabs_dist, long perfect)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    struct Coord3d dstpos;
    struct Coord3d startpos;
    struct Coord3d corpos;

    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    long area_total;
    long area_buildable;
    int i;
    int n;
    n = AI_RANDOM(4);
    if (perfect) {
        area_total = (width_slabs + 1) * (height_slabs + 1);
    } else {
        area_total = width_slabs * height_slabs;
    }
    i = 0;
    dstpos.x.val = 0;
    dstpos.y.val = 0;
    dstpos.z.val = 0;
    while ( 1 )
    {
        startpos.x.val = pos->x.val;
        startpos.y.val = pos->y.val;
        startpos.z.val = pos->z.val;
        if (get_corridor(&startpos, &corpos, n, dungeon->owner, max_slabs_dist))
        {
            stl_x = corpos.x.stl.num;
            stl_y = corpos.y.stl.num;
            if (other_build_here(comp, stl_x, stl_y, width_slabs, height_slabs))
            {
                dstpos.x.stl.num = stl_x;
                dstpos.y.stl.num = stl_y;
                if ( perfect )
                {
                    area_buildable = search_spiral(&dstpos, comp->dungeon->owner, area_total, check_for_perfect_buildable);
                    if (area_buildable >= area_total) {
                        area_buildable = width_slabs * height_slabs;
                        break;
                    }
                } else
                {
                    area_buildable = search_spiral(&dstpos, comp->dungeon->owner, area_total, check_for_buildable);
                    if (area_buildable >= area_total - area_total / 4) {
                        break;
                    }
                }
            }
        }
        i++;
        n = (n + 1) % 4;
        if (i >= 4)
          return 0;
    }
    struct ComputerTask *ctask;
    ctask = get_free_task(comp, 0);
    if (!computer_task_invalid(ctask))
    {
        if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
            struct RoomConfigStats *roomst;
            roomst = &slab_conf.room_cfgstats[rkind];
            message_add_fmt(comp->dungeon->owner, "It is time to build %s.",get_string(roomst->name_stridx));
        }
        ctask->ttype = CTT_DigRoomPassage;
        ctask->rkind = rkind;
        ctask->pos_64.x.val = subtile_coord_center(stl_slab_center_subtile(stl_x));
        ctask->pos_64.y.val = subtile_coord_center(stl_slab_center_subtile(stl_y));
        ctask->pos_64.z.val = subtile_coord(1,0);
        ctask->pos_6A.x.val = pos->x.val;
        ctask->pos_6A.y.val = pos->y.val;
        ctask->pos_6A.z.val = pos->z.val;
        ctask->create_room.startpos.x.val = startpos.x.val;
        ctask->create_room.startpos.y.val = startpos.y.val;
        ctask->create_room.startpos.z.val = startpos.z.val;
        ctask->create_room.width = width_slabs;
        ctask->create_room.height = height_slabs;
        ctask->create_room.area = area_buildable;
        ctask->create_room.long_80 = rkind;
        ctask->flags |= ComTsk_Unkn0002;
        ctask->flags |= ComTsk_AddTrapLocation;
        ctask->flags |= ComTsk_Urgent;
        setup_dig_to(&ctask->dig, ctask->create_room.startpos, ctask->pos_64);
    }
    return ctask;
}

short get_hug_side(struct ComputerDig * cdig, MapSubtlCoord stl1_x, MapSubtlCoord stl1_y, MapSubtlCoord stl2_x, MapSubtlCoord stl2_y, unsigned short direction, PlayerNumber plyr_idx)
{
    SYNCDBG(4,"Starting");
    MapSubtlCoord stl_b_x;
    MapSubtlCoord stl_b_y;
    MapSubtlCoord stl_a_x;
    MapSubtlCoord stl_a_y;
    int i;
    i = get_hug_side_options(stl1_x, stl1_y, stl2_x, stl2_y, direction, plyr_idx, &stl_a_x, &stl_a_y, &stl_b_x, &stl_b_y);
    if ((i == 0) || (i == 1)) {
        return i;
    }
    i = cdig->hug_side;
    if ((i == 0) || (i == 1)) {
        return i;
    }
    int dist_a;
    int dist_b;
    dist_a = abs(stl_a_y - stl2_y) + abs(stl_a_x - stl1_x);
    dist_b = abs(stl_b_y - stl2_y) + abs(stl_b_x - stl1_x);
    if (dist_b > dist_a) {
        return 1;
    }
    if (dist_b < dist_a) {
        return 0;
    }
    // Random hug side
    return ((stl2_x+stl2_y)>>1)%2;
}

short tool_dig_to_pos2_skip_slabs_which_dont_need_digging_f(const struct Computer2 * comp, struct ComputerDig * cdig, unsigned short digflags,
    MapSubtlCoord *nextstl_x, MapSubtlCoord *nextstl_y, const char *func_name)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    MapSlabCoord nextslb_x;
    MapSlabCoord nextslb_y;
    long around_index;
    long i;
    for (i = 0; ; i++)
    {
        struct SlabMap *slb;
        nextslb_x = subtile_slab(*nextstl_x);
        nextslb_y = subtile_slab(*nextstl_y);
        slb = get_slabmap_block(nextslb_x, nextslb_y);
        if (slab_good_for_computer_dig_path(slb) && (slb->kind != SlbT_WATER))
        {
            SubtlCodedCoords stl_num;
            stl_num = get_subtile_number_at_slab_center(nextslb_x,nextslb_y);
            if (find_from_task_list(dungeon->owner, stl_num) < 0) {
                // We've reached a subtile which is good for digging and not in dig tasks list
                break;
            }
        }
        if (slab_kind_is_liquid(slb->kind))
        {
            // We've reached liquid slab - act accordingly
            if ((digflags & ToolDig_AllowLiquidWBridge) != 0) {
                break;
            }
            if (computer_check_room_available(comp, RoK_BRIDGE) != IAvail_Now) {
                break;
            }
        }
        if (slab_kind_is_door(slb->kind) && (slabmap_owner(slb) != dungeon->owner)) {
            // We've reached enemy door
            break;
        }
        cdig->pos_next.x.stl.num = *nextstl_x;
        cdig->pos_next.y.stl.num = *nextstl_y;
        if ((subtile_slab(cdig->pos_dest.x.stl.num) == nextslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == nextslb_y))
        {
            SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)nextslb_x,(int)nextslb_y);
            return -1;
        }
        // Move position towards the target subtile, shortest way
        around_index = small_around_index_towards_destination(*nextstl_x,*nextstl_y,cdig->pos_dest.x.stl.num,cdig->pos_dest.y.stl.num);
        (*nextstl_x) += STL_PER_SLB * small_around[around_index].delta_x;
        (*nextstl_y) += STL_PER_SLB * small_around[around_index].delta_y;
        if (i > map_tiles_x+map_tiles_y)
        {
            ERRORLOG("%s: Infinite loop while finding path to dig gold",func_name);
            return -2;
        }
    }
    return i;
}

/**
 * Moves position towards destination, tagging any slabs which require digging.
 * Stops when the straight road towards destination can no longer be continued.
 * Computer player dig helper function.
 * @see tool_dig_to_pos2_f()
 * @param comp
 * @param cdig
 * @param simulation
 * @param digflags
 * @param nextstl_x
 * @param nextstl_y
 * @param func_name
 */
short tool_dig_to_pos2_do_action_on_slab_which_needs_it_f(struct Computer2 * comp, struct ComputerDig * cdig, TbBool simulation, unsigned short digflags,
    MapSubtlCoord *nextstl_x, MapSubtlCoord *nextstl_y, const char *func_name)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    MapSlabCoord nextslb_x;
    MapSlabCoord nextslb_y;
    long around_index;
    long i;
    for (i = 0; i < cdig->subfield_2C; i++)
    {
        struct SlabAttr *slbattr;
        struct SlabMap *slb;
        struct Map *mapblk;
        nextslb_x = subtile_slab(*nextstl_x);
        nextslb_y = subtile_slab(*nextstl_y);
        slb = get_slabmap_block(nextslb_x, nextslb_y);
        mapblk = get_map_block_at(*nextstl_x, *nextstl_y);
        slbattr = get_slab_attrs(slb);
        if ( (slbattr->is_diggable == 0) || (slb->kind == SlbT_GEMS)
          || (((mapblk->flags & SlbAtFlg_Filled) != 0) && (slabmap_owner(slb) != dungeon->owner)) )
        {
            if ( ((slbattr->block_flags & SlbAtFlg_Valuable) == 0) || (digflags == 0) ) {
                break;
            }
        }
        if ( !simulation )
        {
            if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, *nextstl_x, *nextstl_y, 1, 1) <= Lb_OK) {
                ERRORLOG("%s: Could not do game action at subtile (%d,%d)",func_name,(int)*nextstl_x,(int)*nextstl_y);
                break;
            }
            if (digflags != 0)
            {
                if ((slbattr->block_flags & SlbAtFlg_Valuable) != 0) {
                    cdig->valuable_slabs_tagged++;
                }
            }
        }
        cdig->pos_next.x.stl.num = *nextstl_x;
        cdig->pos_next.y.stl.num = *nextstl_y;
        if ((subtile_slab(cdig->pos_dest.x.stl.num) == nextslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == nextslb_y))
        {
            SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)nextslb_x,(int)nextslb_y);
            return -1;
        }
        // Move position towards the target subtile, shortest way
        around_index = small_around_index_towards_destination(*nextstl_x,*nextstl_y,cdig->pos_dest.x.stl.num,cdig->pos_dest.y.stl.num);
        (*nextstl_x) += STL_PER_SLB * small_around[around_index].delta_x;
        (*nextstl_y) += STL_PER_SLB * small_around[around_index].delta_y;
        if (i > map_tiles_x*map_tiles_y)
        {
            ERRORLOG("%s: Infinite loop while finding path to dig gold",func_name);
            return -2;
        }
    }
    nextslb_x = subtile_slab(*nextstl_x);
    nextslb_y = subtile_slab(*nextstl_y);
    if ((subtile_slab(cdig->pos_dest.x.stl.num) == nextslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == nextslb_y))
    {
        cdig->pos_next.x.stl.num = *nextstl_x;
        cdig->pos_next.y.stl.num = *nextstl_y;
        SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)nextslb_x,(int)nextslb_y);
        return -1;
    }
    return i;
}

/**
 * Tool function to do (or simulate) computer player digging.
 * @param comp Computer player which is doing the task.
 * @param cdig The ComputerDig structure to be changed. Should be dummy if simulating.
 * @param simulation Indicates if we're simulating or doing the real thing.
 * @param digflags These are not really flags, but should be changed into flags when all calls to this func are rewritten. Use values from ToolDigFlags enum.
 * @return
 */
short tool_dig_to_pos2_f(struct Computer2 * comp, struct ComputerDig * cdig, TbBool simulation, unsigned short digflags, const char *func_name)
{
    struct Dungeon *dungeon;
    struct SlabMap *slb;
    struct SlabMap *slbw;
    struct Map *mapblk;
    struct Map *mapblkw;
    MapSubtlCoord gldstl_x;
    MapSubtlCoord gldstl_y;
    MapSubtlCoord digstl_x;
    MapSubtlCoord digstl_y;
    MapSlabCoord digslb_x;
    MapSlabCoord digslb_y;
    long counter1;
    long i;
    SYNCDBG(14,"%s: Starting",func_name);
    dungeon = comp->dungeon;
    // Limit amount of calls
    cdig->calls_count++;
    if (cdig->calls_count >= COMPUTER_TOOL_DIG_LIMIT) {
        WARNLOG("%s: Player %d ComputerDig calls count (%d) exceeds limit for path from (%d,%d) to (%d,%d)",func_name,
            (int)dungeon->owner,(int)cdig->calls_count,(int)coord_slab(cdig->pos_begin.x.val),(int)coord_slab(cdig->pos_begin.y.val),
            (int)coord_slab(cdig->pos_dest.x.val),(int)coord_slab(cdig->pos_dest.y.val));
        return -2;
    }
    gldstl_x = stl_slab_center_subtile(cdig->pos_begin.x.stl.num);
    gldstl_y = stl_slab_center_subtile(cdig->pos_begin.y.stl.num);
    SYNCDBG(4,"%s: Dig slabs from (%d,%d) to (%d,%d)",func_name,subtile_slab(gldstl_x),subtile_slab(gldstl_y),subtile_slab(cdig->pos_dest.x.stl.num),subtile_slab(cdig->pos_dest.y.stl.num));
    if (get_2d_distance(&cdig->pos_begin, &cdig->pos_dest) <= cdig->distance)
    {
        SYNCDBG(4,"%s: Player %d does small distance digging",func_name,(int)dungeon->owner);
        counter1 = tool_dig_to_pos2_skip_slabs_which_dont_need_digging_f(comp, cdig, digflags, &gldstl_x, &gldstl_y, func_name);
        if (counter1 < 0) {
            return counter1;
        }
        // Being here means we didn't reached the destination - we must do some kind of action
        if (slab_is_liquid(subtile_slab(gldstl_x), subtile_slab(gldstl_y)))
        {
            if (computer_check_room_available(comp, RoK_BRIDGE) == IAvail_Now) {
                cdig->pos_next.x.stl.num = gldstl_x;
                cdig->pos_next.y.stl.num = gldstl_y;
                SYNCDBG(5,"%s: Player %d has bridge, so is going through liquid slab (%d,%d)",func_name,
                    (int)dungeon->owner,(int)subtile_slab(gldstl_x),(int)subtile_slab(gldstl_y));
                return -5;
            }
        }
        counter1 = tool_dig_to_pos2_do_action_on_slab_which_needs_it_f(comp, cdig, simulation, digflags, &gldstl_x, &gldstl_y, func_name);
        if (counter1 < 0) {
            return counter1;
        }
        // If the straight road stopped and we were not able to find anything to dig, check other directions
        long around_index;
        around_index = small_around_index_towards_destination(gldstl_x,gldstl_y,cdig->pos_dest.x.stl.num,cdig->pos_dest.y.stl.num);
        if (counter1 > 0)
        {
            cdig->pos_begin.x.stl.num = gldstl_x;
            cdig->pos_begin.y.stl.num = gldstl_y;
            cdig->distance = get_2d_distance(&cdig->pos_next, &cdig->pos_dest);
            // In case we're finishing the easy road, prepare vars for long distance digging
            cdig->hug_side = get_hug_side(cdig, cdig->pos_next.x.stl.num, cdig->pos_next.y.stl.num,
                cdig->pos_dest.x.stl.num, cdig->pos_dest.y.stl.num, around_index, dungeon->owner);
            cdig->direction_around = (around_index + (cdig->hug_side < 1 ? 3 : 1)) & 3;
            SYNCDBG(5,"%s: Going through slab (%d,%d)",func_name,(int)subtile_slab(gldstl_x),(int)subtile_slab(gldstl_y));
            return 0;
        }
        if (cdig->subfield_2C == comp->field_C)
        {
            gldstl_x -= STL_PER_SLB * small_around[around_index].delta_x;
            gldstl_y -= STL_PER_SLB * small_around[around_index].delta_y;
            cdig->pos_begin.x.val = subtile_coord(gldstl_x,0);
            cdig->pos_begin.y.val = subtile_coord(gldstl_y,0);
            cdig->pos_begin.z.val = 0;
            cdig->pos_E.x.val = cdig->pos_begin.x.val;
            cdig->pos_E.y.val = cdig->pos_begin.y.val;
            cdig->pos_E.z.val = cdig->pos_begin.z.val;
        }
        if ((cdig->pos_next.x.val == 0) && (cdig->pos_next.y.val == 0) && (cdig->pos_next.z.val == 0))
        {
            cdig->pos_next.x.val = cdig->pos_E.x.val;
            cdig->pos_next.y.val = cdig->pos_E.y.val;
            cdig->pos_next.z.val = cdig->pos_E.z.val;
        }
        cdig->subfield_48++;
        if ((cdig->subfield_48 > 10) && (cdig->sub4C_stl_x == gldstl_x) && (cdig->sub4C_stl_y == gldstl_y)) {
            SYNCDBG(5,"%s: Positions are equal at subtile (%d,%d)",func_name,(int)gldstl_x,(int)gldstl_y);
            return -2;
        }
        cdig->sub4C_stl_x = gldstl_x;
        cdig->sub4C_stl_y = gldstl_y;
        cdig->distance = get_2d_distance(&cdig->pos_next, &cdig->pos_dest);
        cdig->hug_side = get_hug_side(cdig, cdig->pos_next.x.stl.num, cdig->pos_next.y.stl.num,
                           cdig->pos_dest.x.stl.num, cdig->pos_dest.y.stl.num, around_index, dungeon->owner);
        cdig->direction_around = (around_index + (cdig->hug_side < 1 ? 3 : 1)) & 3;
        i = dig_to_position(dungeon->owner, cdig->pos_next.x.stl.num, cdig->pos_next.y.stl.num,
            cdig->direction_around, cdig->hug_side);
        if (i == -1) {
            SYNCDBG(5,"%s: Player %d short digging to subtile (%d,%d) preparations failed",func_name,(int)dungeon->owner,(int)cdig->pos_next.x.stl.num,(int)cdig->pos_next.y.stl.num);
            return -2;
        }
        digstl_x = stl_num_decode_x(i);
        digstl_y = stl_num_decode_y(i);
        digslb_x = subtile_slab(digstl_x);
        digslb_y = subtile_slab(digstl_y);
        slb = get_slabmap_block(digslb_x, digslb_y);
        if (slab_kind_is_liquid(slb->kind) && (computer_check_room_available(comp, RoK_BRIDGE) == IAvail_Now))
        {
            cdig->pos_next.y.stl.num = digstl_y;
            cdig->pos_next.x.stl.num = digstl_x;
            SYNCDBG(5,"%s: Player %d has bridge, so is going through liquid subtile (%d,%d)",func_name,(int)dungeon->owner,(int)gldstl_x,(int)gldstl_y);
            return -5;
        }
    } else
    {
        SYNCDBG(4,"%s: Player %d does long distance digging",func_name,(int)dungeon->owner);
        i = dig_to_position(dungeon->owner, gldstl_x, gldstl_y, cdig->direction_around, cdig->hug_side);
        if (i == -1) {
            SYNCDBG(5,"%s: Player %d long digging to subtile (%d,%d) preparations failed",func_name,(int)dungeon->owner,(int)gldstl_x,(int)gldstl_y);
            return -2;
        }
        digstl_x = stl_num_decode_x(i);
        digstl_y = stl_num_decode_y(i);
        digslb_x = subtile_slab(digstl_x);
        digslb_y = subtile_slab(digstl_y);
    }
    slb = get_slabmap_block(digslb_x, digslb_y);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    if ((slbattr->is_diggable) && (slb->kind != SlbT_GEMS))
    {
        mapblk = get_map_block_at(digstl_x, digstl_y);
        if (((mapblk->flags & SlbAtFlg_Filled) == 0) || (slabmap_owner(slb) == dungeon->owner))
        {
            i = get_subtile_number_at_slab_center(digslb_x,digslb_y);
            if ((find_from_task_list(dungeon->owner, i) < 0) && (!simulation))
            {
                // Only when the computer has enough gold to cast lvl8, will he consider casting lvl3 power, so he has some gold left.
                if( computer_able_to_use_power(comp, PwrK_DESTRWALLS, 8, 1))
                {
                    mapblkw = get_map_block_at(digstl_x, digstl_y-3);
                    slbw = get_slabmap_block(digslb_x, digslb_y-1);
                    if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                    {
                        magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x, digstl_y-3, PwCast_Unrevealed);
                        return -5;
                    }
                    else
                    {
                        mapblkw = get_map_block_at(digstl_x, digstl_y+3);
                        slbw = get_slabmap_block(digslb_x, digslb_y+1);
                        if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                        {
                            magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x, digstl_y+3, PwCast_Unrevealed);
                            return -5;
                        }
                        else
                        {
                            mapblkw = get_map_block_at(digstl_x-3, digstl_y);
                            slbw = get_slabmap_block(digslb_x-1, digslb_y);
                            if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                            {
                                magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x-3, digstl_y, PwCast_Unrevealed);
                                return -5;
                            }
                            else
                            {
                                mapblkw = get_map_block_at(digstl_x+3, digstl_y);
                                slbw = get_slabmap_block(digslb_x+1, digslb_y);
                                if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                                {
                                    magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x+3, digstl_y, PwCast_Unrevealed);
                                    return -5;
                                }
                            }
                        }
                    }
                }
                if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, digstl_x, digstl_y, 1, 1) <= Lb_OK) 
                {
                    ERRORLOG("%s: Couldn't do game action - cannot dig",func_name);
                    return -2;
                }
            }
        }
    }
    cdig->direction_around = small_around_index_towards_destination(cdig->pos_next.x.stl.num,cdig->pos_next.y.stl.num,digstl_x,digstl_y);
    cdig->pos_next.x.stl.num = digstl_x;
    cdig->pos_next.y.stl.num = digstl_y;
    if ((subtile_slab(cdig->pos_dest.x.stl.num) == digslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == digslb_y))
    {
        SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)digslb_x,(int)digslb_y);
        return -1;
    }
    cdig->pos_begin.x.stl.num = digstl_x;
    cdig->pos_begin.y.stl.num = digstl_y;
    SYNCDBG(5,"%s: Going through slab (%d,%d)",func_name,(int)digslb_x,(int)digslb_y);
    return 0;
}

int find_trap_location_index(const struct Computer2 * comp, const struct Coord3d * coord)
{
    const struct Coord3d * location;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    long i;
    slb_x = subtile_slab(coord->x.stl.num);
    slb_y = subtile_slab(coord->y.stl.num);
    for (i=0; i < COMPUTER_TRAP_LOC_COUNT; i++)
    {
        location = &comp->trap_locations[i];
        if ((subtile_slab(location->x.stl.num) == slb_x) && (subtile_slab(location->y.stl.num) == slb_y)) {
            return i;
        }
    }
    return -1;
}

long add_to_trap_location(struct Computer2 * comp, struct Coord3d * coord)
{
    SYNCDBG(6,"Starting");
    // Avoid duplicating entries
    if (find_trap_location_index(comp, coord) >= 0) {
        return false;
    }
    struct Coord3d * location;
    long i;
    // Find a free place and add the location
    for (i=0; i < COMPUTER_TRAP_LOC_COUNT; i++)
    {
        location = &comp->trap_locations[i];
        if ((location->x.val <= 0) && (location->y.val <= 0)) {
            location->x.val = coord->x.val;
            location->y.val = coord->y.val;
            location->z.val = coord->z.val;
            return true;
        }
    }
    SYNCDBG(7,"No free location");
    return false;
}

long check_for_gold(MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, long plyr_idx)
{
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    SubtlCodedCoords stl_num;
    SYNCDBG(15,"Starting");
    basestl_x = stl_slab_center_subtile(basestl_x);
    basestl_y = stl_slab_center_subtile(basestl_y);
    stl_num = get_subtile_number(basestl_x,basestl_y);
    slb = get_slabmap_for_subtile(basestl_x,basestl_y);
    slbattr = get_slab_attrs(slb);
    if ((slbattr->block_flags & SlbAtFlg_Valuable) != 0) {
        return (find_from_task_list(plyr_idx, stl_num) < 0);
    }
    return 0;
}

int search_spiral_f(struct Coord3d *pos, PlayerNumber owner, int i3, long (*cb)(MapSubtlCoord, MapSubtlCoord, long), const char *func_name)
{
    SYNCDBG(7,"%s: Starting at (%d,%d)",func_name,pos->x.stl.num,pos->y.stl.num);
    long retval = _DK_search_spiral(pos, owner, i3, cb);
    SYNCDBG(8,"%s: Finished with %d",func_name,(int)retval);

    return retval;
}

long find_next_gold(struct Computer2 * comp, struct ComputerTask * ctask)
{
    SYNCDBG(5,"Starting");

    memcpy(&ctask->dig.pos_dest, &ctask->dig.pos_next, sizeof(struct Coord3d));

    // Try to find gold tiles around current position
    if (search_spiral(&ctask->dig.pos_dest, comp->dungeon->owner, 25, check_for_gold) == 25) {
        SYNCDBG(5,"Player %d did not found next gold",(int)comp->dungeon->owner);
        return 0;
    }

    memcpy(&ctask->dig.pos_begin, &ctask->dig.pos_next, sizeof(struct Coord3d));
    ctask->dig.distance = LONG_MAX;
    ctask->dig.calls_count = 0;
    // Make local copy of the dig structure
    struct ComputerDig cdig;
    memcpy(&cdig, &ctask->dig, sizeof(struct ComputerDig));

    long retval;
    do
    {
        retval = tool_dig_to_pos2(comp, &cdig, 1, ToolDig_BasicOnly);
        SYNCDBG(5,"retval=%d, dig.distance=%d, dig.subfield_54=%d",
            retval, cdig.distance, cdig.calls_count);
    } while (retval == 0);

    SYNCDBG(6,"Finished");
    if ((retval != -1) && (retval != -5)) {
        return 0;
    } else {
        return 1;
    }
}

long task_dig_to_gold(struct Computer2 *comp, struct ComputerTask *ctask)
{
    long i;
    SYNCDBG(2,"Starting");
    struct Dungeon* dungeon = comp->dungeon;

    i = dungeon->total_area * comp->field_1C / 100;
    if ((dungeon->digger_stack_length > 0) && (dungeon->digger_stack_length >= i))
    {
        SYNCDBG(6,"Player %d did nothing because digger stack length is over %d",(int)dungeon->owner,(int)i);
        return 0;
    }

    if (ctask->dig.valuable_slabs_tagged >= ctask->dig_to_gold.slabs_dig_count)
    {
        struct SlabMap* slb = get_slabmap_for_subtile(ctask->dig.pos_next.x.stl.num, ctask->dig.pos_next.y.stl.num);

        if ((get_slab_attrs(slb)->block_flags & SlbAtFlg_Valuable) != 0)
        {
            ctask->field_60--;
            if (ctask->field_60 > 0) {
                SYNCDBG(6,"Player %d needs to dig %d more",(int)dungeon->owner,(int)ctask->field_60);
                return 0;
            }
        }
        ctask->dig.valuable_slabs_tagged = 0;
    }

    {
        MapSubtlCoord stl_x = stl_slab_center_subtile(ctask->dig.pos_next.x.stl.num);
        MapSubtlCoord stl_y = stl_slab_center_subtile(ctask->dig.pos_next.y.stl.num);
        MapSlabCoord slb_x = subtile_slab(stl_x);
        MapSlabCoord slb_y = subtile_slab(stl_y);
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);

        if (slb->kind == SlbT_GEMS)
        {
            SYNCDBG(7, "Player %d is digging around gems (%d %d)", (int)dungeon->owner, slb_x, slb_y);
            for (int y = -1; y < 2; y++)
            {
                for (int x = -1; x < 2; x++)
                {
                    if ((x != 0) || (y != 0))
                    {
                        stl_x = slab_subtile_center(slb_x + x);
                        stl_y = slab_subtile_center(slb_y + y);
                        slb = get_slabmap_block(slb_x + x, slb_y + y);

                        struct Map* mapblk = get_map_block_at(stl_x, stl_y);
                        struct SlabAttr *slbattr = get_slab_attrs(slb);
                        if ( (slbattr->is_diggable != 0)
                          || (((mapblk->flags & SlbAtFlg_Filled) != 0) && (slabmap_owner(slb) == dungeon->owner)) )
                        {
                            TbResult res = game_action(dungeon->owner, GA_MarkDig, 0, stl_x, stl_y, 1, 1);
                            if (res <= Lb_OK)
                            {
                                WARNLOG("Game action GA_MarkDig returned code %d - location %d,%d around gem not marked for digging", res, slb_x + x, slb_y + y);
                            }
                        }
                    }
                }
            }
        }
    }

    long retval = tool_dig_to_pos2(comp, &ctask->dig, 0, ToolDig_AllowValuable);

    if ((ctask->flags & ComTsk_AddTrapLocation) != 0)
    {
        ctask->flags &= ~ComTsk_AddTrapLocation;
        add_to_trap_location(comp, &ctask->dig.pos_next);
    }

    if (ctask->dig.valuable_slabs_tagged >= ctask->dig_to_gold.slabs_dig_count)
    {
        ctask->field_60 = 700 / comp->field_18;
    }

    if (retval == -5)
    {
        ctask->ottype = ctask->ttype;
        ctask->ttype = CTT_WaitForBridge;

        SYNCDBG(6,"Player %d is waiting for bridge",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }

    if ((retval < -3) || (retval > -1))
    {
        SYNCDBG(6,"Player %d finished, code %d",(int)dungeon->owner,(int)retval);
        return retval;
    }

    if (find_next_gold(comp, ctask) != 0)
    {
        SYNCDBG(7,"Player %d found next slab",(int)dungeon->owner);
        return 0;
    }

    struct GoldLookup* gold_lookup = get_gold_lookup(ctask->dig_to_gold.target_lookup_idx);

    unsigned short gldstl_x = gold_lookup->x_stl_num;
    unsigned short gldstl_y = gold_lookup->y_stl_num;

    unsigned short ctgstl_x = ctask->dig.pos_begin.x.stl.num;
    unsigned short ctgstl_y = ctask->dig.pos_begin.y.stl.num;

    // While destination isn't reached, continue finding slabs to mark
    if ((gldstl_x != ctgstl_x) || (gldstl_y != ctgstl_y))
    {
        ctask->dig.pos_next.x.stl.num = gldstl_x;
        ctask->dig.pos_next.y.stl.num = gldstl_y;

        ctask->dig.pos_begin.x.stl.num = gldstl_x;
        ctask->dig.pos_begin.y.stl.num = gldstl_y;

        if (find_next_gold(comp, ctask) != 0) // || (retval < -3) -- Already returned
        {
            SYNCDBG(7,"Player %d found next slab",(int)dungeon->owner);
            return retval;
        }
    }

    // move to next task or return to enclosing task or return to try again later
    if ((retval == -3) || (retval == -2))
    {
        gold_lookup = get_gold_lookup(ctask->dig_to_gold.target_lookup_idx);
        gold_lookup->player_interested[dungeon->owner] |= 0x02;
        remove_task(comp, ctask);
    } else
    if (retval == -1) // unnecessary check as retval < -3 and retval > -1 did return before
    {
        remove_task(comp, ctask);
    }

    SYNCDBG(5,"Player %d task finished",(int)dungeon->owner);
    return retval;
}

long task_dig_to_attack(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    if ((game.play_gameturn - ctask->created_turn) > 7500) {
      comp->task_state = CTaskSt_Select;
      remove_task(comp, ctask);
      return CTaskRet_Unk0;
    }
    //return _DK_task_dig_to_attack(comp,ctask);
    if (ctask->dig.pos_next.x.val > 0)
    {
        struct SlabMap *slb;
        slb = get_slabmap_for_subtile(ctask->dig.pos_next.x.stl.num, ctask->dig.pos_next.y.stl.num);
        if (slabmap_owner(slb) != comp->dungeon->owner) {
            return CTaskRet_Unk4;
        }
        if ((ctask->flags & ComTsk_AddTrapLocation) != 0)
        {
          ctask->lastrun_turn++;
          if (ctask->lastrun_turn > 5)
              ctask->flags &= ~ComTsk_AddTrapLocation;
          add_to_trap_location(comp, &ctask->dig.pos_next);
        }
    }
    long dig_ret;
    dig_ret = tool_dig_to_pos2(comp, &ctask->dig, 0, ToolDig_BasicOnly);
    int i;
    switch (dig_ret)
    {
    case -5:
        ctask->ottype = ctask->ttype;
        ctask->ttype = CTT_WaitForBridge;
        dig_ret = CTaskRet_Unk4;
        break;
    case -3:
    case -2:
        comp->task_state = CTaskSt_Select;
        suspend_task_process(comp, ctask);
        break;
    case 0:
        for (i = 0; i < SMALL_AROUND_MID_LENGTH; i++)
        {
            MapSubtlCoord stl_x;
            MapSubtlCoord stl_y;
            stl_x = ctask->dig.pos_next.x.stl.num + (long)slab_subtile(small_around_mid[i].delta_x,0);
            stl_y = ctask->dig.pos_next.y.stl.num + (long)slab_subtile(small_around_mid[i].delta_y,0);
            if (xy_walkable(stl_x, stl_y, ctask->dig_somewhere.target_plyr_idx)) {
                remove_task(comp, ctask);
                break;
            }
        }
        if (i == SMALL_AROUND_MID_LENGTH)
        {
            break;
        }
        // fall through
    case -1:
        {
            struct ComputerProcess *cproc;
            cproc = get_computer_process(comp, ctask->field_8C);
            cproc->param_5 = computer_task_index(ctask);
            cproc->func_complete(comp, cproc);
        }
        suspend_task_process(comp, ctask);
        break;
    default:
        break;
    }
    return dig_ret;
}

long count_creatures_at_call_to_arms(struct Computer2 *comp)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    return count_player_list_creatures_of_model_matching_bool_filter(dungeon->owner, -1, creature_is_called_to_arms);
}

struct Thing *find_creature_for_call_to_arms(struct Computer2 *comp, long a2)
{
    return _DK_find_creature_for_call_to_arms(comp, a2);
}

long count_creatures_in_call_to_arms(struct Computer2 *comp)
{
    return _DK_count_creatures_in_call_to_arms(comp);
}

long task_magic_call_to_arms(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    //return _DK_task_magic_call_to_arms(comp,ctask);
    switch (ctask->task_state)
    {
    case 0:
        if ((game.play_gameturn - ctask->lastrun_turn) < ctask->field_60) {
            return CTaskRet_Unk4;
        }
        ctask->field_60 = 18;
        ctask->lastrun_turn = game.play_gameturn;
        // If gathered enough creatures, go to next task state
        if (count_creatures_in_call_to_arms(comp) >= ctask->magic_cta.repeat_num) {
            ctask->task_state = CTaskSt_Wait;
            return CTaskRet_Unk2;
        }
        // If not, cast CTA on position of next creature
        struct Thing *creatng;
        creatng = find_creature_for_call_to_arms(comp, 1);
        if (!thing_is_invalid(creatng))
        {
          if (try_game_action(comp, dungeon->owner, GA_UsePwrCall2Arms, 8, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, 1, 1) > Lb_OK) {
              return CTaskRet_Unk2;
          }
          // If cannot cast CTA on next creature, skip to state 2
          SYNCDBG(7,"Player %d cannot cast CTA on owned %s index %d",(int)dungeon->owner,thing_model_name(creatng),(int)creatng->index);
          ctask->task_state = CTaskSt_Select;
          return CTaskRet_Unk4;
        }
        // If there is no next creature, but we've gathered at least half of the amount, go to next state with what we have
        if (count_creatures_in_call_to_arms(comp) > ctask->magic_cta.repeat_num - ctask->magic_cta.repeat_num / 2) {
            ctask->task_state = CTaskSt_Wait;
            return CTaskRet_Unk4;
        }
        // We haven't gathered enough creatures - cancel the task
        SYNCDBG(7,"Player %d cannot gather enough creatures around CTA, cancelling task",(int)dungeon->owner);
        if (dungeon->cta_start_turn > 0) {
            try_game_action(comp, dungeon->owner, GA_StopPwrCall2Arms, 5, 0, 0, 0, 0);
        }
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    case 1:
        if ((game.play_gameturn - ctask->lastrun_turn) < ctask->field_60) {
            return CTaskRet_Unk2;
        }
        SYNCDBG(7,"Player %d casts CTA at (%d,%d)",(int)dungeon->owner, (int)ctask->magic_cta.target_pos.x.stl.num, (int)ctask->magic_cta.target_pos.y.stl.num);
        if (try_game_action(comp, dungeon->owner, GA_UsePwrCall2Arms, 5, ctask->magic_cta.target_pos.x.stl.num, ctask->magic_cta.target_pos.y.stl.num, 1, 1) >= Lb_OK) {
            ctask->task_state = CTaskSt_Select;
            ctask->field_60 = ctask->field_8E;
            return CTaskRet_Unk2;
        }
        SYNCDBG(7,"Player %d cannot cast CTA at (%d,%d), cancelling task",(int)dungeon->owner, (int)ctask->magic_cta.target_pos.x.stl.num, (int)ctask->magic_cta.target_pos.y.stl.num);
        if (dungeon->cta_start_turn > 0) {
            try_game_action(comp, dungeon->owner, GA_StopPwrCall2Arms, 5, 0, 0, 0, 0);
        }
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    case 2:
        // Keep CTA running until most creatures are able to reach it
        if (count_creatures_at_call_to_arms(comp) < ctask->magic_cta.repeat_num - ctask->magic_cta.repeat_num / 4) 
        {
            // For a minimum amount of time
            if ((game.play_gameturn - ctask->lastrun_turn) < (ctask->field_60 / 10)) 
            {
                return CTaskRet_Unk1;
            }
        }
        // There's a time limit for how long CTA may run
        if ((game.play_gameturn - ctask->lastrun_turn) < ctask->field_60) 
            {
                return CTaskRet_Unk1;
            }
        // Finish the CTA casting task
        SYNCDBG(7,"Player %d finishes CTA at (%d,%d)",(int)dungeon->owner, (int)ctask->magic_cta.target_pos.x.stl.num, (int)ctask->magic_cta.target_pos.y.stl.num);
        if (dungeon->cta_start_turn > 0) {
            try_game_action(comp, dungeon->owner, GA_StopPwrCall2Arms, 5, 0, 0, 0, 0);
        }
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    default:
        ERRORLOG("Bad state %d", (int)ctask->task_state);
        ctask->task_state = CTaskSt_None;
        break;
    }
    return CTaskRet_Unk4;
}

struct Thing *find_creature_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, CreatureJob new_job, long best_score)
{
    //TODO CREATURE_JOBS This function needs major rework, to base conditions on job, not on room we're dropping into
    //TODO CREATURE_JOBS Rewrite all uses of this function before remaking it
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    SYNCDBG(8,"Starting");
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = 0;
    stl_y = 0;
    if (pos != NULL)
    {
        stl_x = pos->x.stl.num;
        stl_y = pos->y.stl.num;
    } else
    if (!room_is_invalid(room))
    {
        stl_x = room->central_stl_x;
        stl_y = room->central_stl_y;
    }
    struct Thing *pick_thing;
    long pick_score;
    pick_score = LONG_MIN;
    pick_thing = INVALID_THING;

    unsigned long k;
    int i;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        long score;
        if (can_thing_be_picked_up_by_player(thing, dungeon->owner) && !creature_is_being_dropped(thing))
        {
            if ((get_creature_state_type(thing) != CrStTyp_Work) || (thing_is_invalid(pick_thing) && best_score))
            {
                if (!room_is_invalid(room))
                {
                    if (creature_can_do_job_near_position(thing, stl_x, stl_y, new_job, JobChk_None))
                    {
                        struct CreatureStats *crstat;
                        crstat = creature_stats_get_from_thing(thing);
                        switch (room->kind)
                        {
                        case RoK_LIBRARY:
                            score = compute_creature_work_value(crstat->research_value*256, ROOM_EFFICIENCY_MAX, cctrl->explevel);
                            break;
                        case RoK_WORKSHOP:
                            score = compute_creature_work_value(crstat->manufacture_value*256, ROOM_EFFICIENCY_MAX, cctrl->explevel);
                            break;
                        case RoK_TRAINING:
                            score = get_creature_thing_score(thing);
                            break;
                        default:
                            score = 0; // Still more than LONG_MIN
                            break;
                        }
                        if (score >= pick_score)
                        {
                            pick_score = score;
                            pick_thing = thing;
                        }
                        continue;
                    }
                } else
                {
                    long delta_x;
                    long delta_y;
                    delta_x = thing->mappos.x.stl.num - stl_x;
                    delta_y = thing->mappos.y.stl.num - stl_y;
                    if (abs(delta_x) + abs(delta_y) >= 2)
                    {
                        if (best_score) {
                            score = get_creature_thing_score(thing);
                        } else {
                            score = 0; // Still more than LONG_MIN
                        }
                        if (score >= pick_score)
                        {
                            pick_score = score;
                            pick_thing = thing;
                        }
                    }
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
    SYNCDBG(19,"Finished");
    return pick_thing;
}

long count_creatures_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, long a4)
{
    //TODO COMPUTER_EVENT_BREACH needs this function; may be also used somewhere else - not sure
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = 0;
    stl_y = 0;
    if (pos != NULL)
    {
      stl_x = pos->x.stl.num;
      stl_y = pos->y.stl.num;
    }
    int count;
    count = 0;
    k = 0;
    i = comp->dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing))
        {
            if ((thing->active_state != CrSt_CreatureUnconscious) && (cctrl->combat_flags == 0))
            {
                if (!creature_is_called_to_arms(thing) && !creature_is_being_dropped(thing))
                {
                    struct StateInfo *stati;
                    int n;
                    n = get_creature_state_besides_move(thing);
                    stati = get_thing_state_info_num(n);
                    if ((stati->state_type != CrStTyp_Work) || a4 )
                    {
                        if (room_is_invalid(room))
                        {
                            if (abs(thing->mappos.x.stl.num - stl_x) + abs(thing->mappos.y.stl.num - stl_y) < 2 )
                              continue;
                        } else
                        {
                            //This needs finishing
                            //if ( !person_will_do_job_for_room(thing, room) )
                              continue;
                        }
                        count++;
                    }
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
    SYNCDBG(19,"Finished");
    return count;
}

long task_pickup_for_attack(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(19,"Starting");
    if (game.play_gameturn - ctask->created_turn > 7500)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    }
    if (!xy_walkable(ctask->pickup_for_attack.target_pos.x.stl.num, ctask->pickup_for_attack.target_pos.y.stl.num, comp->dungeon->owner))
    {
        return CTaskRet_Unk4;
    }
    struct Thing *thing;
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing))
    {
        if (computer_dump_held_things_on_map(comp, thing, &ctask->pickup_for_attack.target_pos)) {
            return CTaskRet_Unk2;
        }
        computer_force_dump_held_things_on_map(comp, &comp->dungeon->essential_pos);
        return CTaskRet_Unk4;
    }
    if (ctask->pickup_for_attack.repeat_num <= 0)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    ctask->pickup_for_attack.repeat_num--;
    thing = find_creature_for_pickup(comp, &ctask->pickup_for_attack.target_pos, NULL, Job_SEEK_THE_ENEMY, 1);
    if (!thing_is_invalid(thing))
    {
        if (computer_place_thing_in_power_hand(comp, thing, &ctask->pickup_for_attack.target_pos)) {
            SYNCDBG(9,"Player %d picked %s index %d to attack position (%d,%d)",(int)comp->dungeon->owner,thing_model_name(thing),(int)thing->index,
                (int)ctask->pickup_for_attack.target_pos.x.stl.num, (int)ctask->pickup_for_attack.target_pos.y.stl.num);
            return CTaskRet_Unk2;
        }
        return CTaskRet_Unk4;
    }
    return CTaskRet_Unk4;
}

long task_move_creature_to_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Thing *thing;
    struct Room *room;
    struct Coord3d pos;
    long i;
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    room = room_get(ctask->move_to_room.room_idx1);
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing)) // We have no unit in hand
    {
        // 2nd phase - we have specific creature and specific room index, and creature is picked up already
        SYNCDBG(9,"Starting player %d drop",(int)dungeon->owner);
        if (room_is_invalid(room))
        {
            room = room_get(ctask->move_to_room.room_idx2);
        }
        if (thing_is_creature(thing) && room_exists(room))
        {
            struct CreatureStats *crstat;
            crstat = creature_stats_get_from_thing(thing);
            CreatureJob jobpref;
            jobpref = get_job_for_room(room->kind, JoKF_AssignComputerDrop|JoKF_AssignAreaWithinRoom, crstat->job_primary|crstat->job_secondary);
            if (get_drop_position_for_creature_job_in_room(&pos, room, jobpref, thing))
            {
                if (computer_dump_held_things_on_map(comp, thing, &pos) > 0) {
                    return CTaskRet_Unk2;
                }
            }
            ERRORLOG("Could not find valid position in player %d %s for %s to be dropped",(int)dungeon->owner,room_code_name(room->kind),thing_model_name(thing));
        } else
        {
            WARNLOG("Could not move player %d creature by dropping %s into %s",(int)dungeon->owner,thing_model_name(thing),room_code_name(room->kind));
        }
        computer_force_dump_held_things_on_map(comp, &dungeon->essential_pos);
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    }
    // 1st phase - we need to find a room and a creature for pickup, and take it to hand
    SYNCDBG(9,"Starting player %d pickup",(int)dungeon->owner);
    i = ctask->move_to_room.repeat_num;
    ctask->move_to_room.repeat_num--;
    if (i <= 0)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    CreatureJob jobpref;
    jobpref = find_creature_to_be_placed_in_room_for_job(comp, &room, &thing);
    if (jobpref != Job_NULL)
    {
        //TODO CREATURE_AI try to make sure the creature will do proper activity in the room
        //     ie. select a room tile which is far from CTA and enemies
        ctask->move_to_room.room_idx2 = room->index;
        if (get_drop_position_for_creature_job_in_room(&pos, room, jobpref, thing))
        {
            if (computer_place_thing_in_power_hand(comp, thing, &pos)) {
                SYNCDBG(9,"Player %d picked %s index %d to place in %s index %d",(int)dungeon->owner,
                    thing_model_name(thing),(int)thing->index,room_code_name(room->kind),(int)room->index);
                return CTaskRet_Unk2;
            }
        }
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
      }
      SYNCDBG(9,"No thing for player %d pickup",(int)dungeon->owner);
      remove_task(comp, ctask);
      return CTaskRet_Unk0;
}

long task_move_creature_to_pos(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    SYNCDBG(19,"Starting");
    dungeon = comp->dungeon;
    struct Thing *thing;
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing))
    {
        if (ctask->move_to_pos.target_thing_idx == comp->held_thing_idx)
        {
            if (thing_is_creature(thing))
            {
                if (computer_dump_held_things_on_map(comp, thing, &ctask->move_to_pos.pos_86)) {
                    remove_task(comp, ctask);
                    return CTaskRet_Unk2;
                }
                ERRORLOG("Could not dump player %d %s into (%d,%d)",(int)dungeon->owner,
                    thing_model_name(thing),(int)ctask->move_to_pos.pos_86.x.stl.num,(int)ctask->move_to_pos.pos_86.y.stl.num);
            } else
            {
                WARNLOG("Player %d computer hand holds %s instead of creature",(int)dungeon->owner, thing_model_name(thing));
            }
            computer_force_dump_held_things_on_map(comp, &comp->dungeon->essential_pos);
            remove_task(comp, ctask);
            return CTaskRet_Unk0;
        }
        // Being here means we have a thing for different task picked up, so wait until it's dropped
        return CTaskRet_Unk0;
    }
    thing = thing_get(ctask->move_to_pos.target_thing_idx);
    if (can_thing_be_picked_up_by_player(thing, dungeon->owner))
    {
        if (computer_place_thing_in_power_hand(comp, thing, &ctask->move_to_pos.pos_86)) {
            SYNCDBG(9,"Player %d picked %s index %d to move to (%d,%d)",(int)comp->dungeon->owner,thing_model_name(thing),(int)thing->index,
                (int)ctask->move_to_pos.pos_86.x.stl.num, (int)ctask->move_to_pos.pos_86.y.stl.num);
            return CTaskRet_Unk2;
        }
    }
    remove_task(comp, ctask);
    return CTaskRet_Unk0;
}

struct Thing *find_creature_for_defend_pickup(struct Computer2 *comp)
{
    struct Dungeon *dungeon;
    unsigned long k;
    long i;
    dungeon = comp->dungeon;
    long best_factor;
    struct Thing *best_creatng;
    best_creatng = INVALID_THING;
    best_factor = LONG_MIN;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct CreatureControl *cctrl;
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per creature code
        if (can_thing_be_picked_up_by_player(thing, dungeon->owner))
        {
            if (cctrl->combat_flags == 0)
            {
                if (!creature_is_fleeing_combat(thing) && !creature_is_at_alarm(thing))
                {
                    if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
                    {
                        if (!creature_is_doing_lair_activity(thing) && !creature_is_being_dropped(thing))
                        {
                            struct PerExpLevelValues *expvalues;
                            expvalues = &game.creature_scores[thing->model];
                            long expval;
                            long healthprm;
                            long new_factor;
                            expval = expvalues->value[cctrl->explevel];
                            healthprm = get_creature_health_permil(thing);
                            new_factor = healthprm * expval / 1000;
                            if ((new_factor > best_factor) && (healthprm > 20))
                            {
                                best_factor = new_factor;
                                best_creatng = thing;
                            }
                        }
                    }
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return best_creatng;
}

long task_move_creatures_to_defend(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(19,"Starting");
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    struct Thing *thing;
    thing = thing_get(comp->held_thing_idx);
    // If the heart is just being destroyed - dump held thing and finish task
    if (player_cannot_win(dungeon->owner))
    {
        if (!thing_is_invalid(thing)) {
            computer_force_dump_held_things_on_map(comp, &ctask->move_to_defend.target_pos);
        }
        SYNCDBG(8,"No reason to bother, player %d can no longer win",(int)dungeon->owner);
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    }
    // If everything is fine and we're keeping the thing to move in "fake hand"
    if (!thing_is_invalid(thing))
    {
        if (thing_is_creature(thing))
        {
            if (!players_are_mutual_allies(thing->owner, dungeon->owner))
            {
                return CTaskRet_Unk4;
            }
            if (computer_dump_held_things_on_map(comp, thing, &ctask->move_to_defend.target_pos)) {
                return CTaskRet_Unk2;
            }
            ERRORLOG("Could not dump player %d %s into (%d,%d)",(int)dungeon->owner,
                thing_model_name(thing),(int)ctask->move_to_defend.target_pos.x.stl.num,(int)ctask->move_to_defend.target_pos.y.stl.num);
        } else
        {
            WARNLOG("Player %d computer hand holds %s instead of creature",(int)dungeon->owner, thing_model_name(thing));
        }
        computer_force_dump_held_things_on_map(comp, &dungeon->essential_pos);
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    }
    if (game.play_gameturn - ctask->lastrun_turn < ctask->field_60) {
        return CTaskRet_Unk4;
    }
    ctask->lastrun_turn = game.play_gameturn;
    ctask->move_to_defend.repeat_num--;
    if (ctask->move_to_defend.repeat_num <= 0)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    thing = find_creature_for_defend_pickup(comp);
    if (!thing_is_invalid(thing))
    {
        if (!computer_place_thing_in_power_hand(comp, thing, &ctask->move_to_defend.target_pos) )
        {
            remove_task(comp, ctask);
            return CTaskRet_Unk0;
        }
        SYNCDBG(9,"Player %d picked %s index %d to defend position (%d,%d)",(int)dungeon->owner,thing_model_name(thing),(int)thing->index,
            (int)ctask->move_to_defend.target_pos.x.stl.num, (int)ctask->move_to_defend.target_pos.y.stl.num);
        return CTaskRet_Unk2;
    }
    return CTaskRet_Unk2;
}

long task_move_gold_to_treasury(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon in computer player.");
        return CTaskRet_Unk0;
    }
    SYNCDBG(9,"Starting for player %d",(int)dungeon->owner);
    struct Thing *thing;
    struct Coord3d pos;
    long i;
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing))
    {
        struct Room* room = room_get(ctask->move_gold.room_idx);
        if (object_is_gold(thing) && room_exists(room))
        {
            if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
            {
                if (computer_dump_held_things_on_map(comp, thing, &pos) > 0) {
                    return CTaskRet_Unk2;
                }
            }
            ERRORLOG("Could not find valid position in player %d %s for %s to be dropped",(int)dungeon->owner,room_code_name(room->kind),thing_model_name(thing));
        } else
        {
            WARNLOG("Could not move player %d gold by dropping %s into %s",(int)dungeon->owner,thing_model_name(thing),room_code_name(room->kind));
        }
        computer_force_dump_held_things_on_map(comp, &comp->dungeon->essential_pos);
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    }
    i = ctask->move_gold.items_amount;
    ctask->move_gold.items_amount--;
    if (i <= 0)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    thing = find_gold_laying_in_dungeon(comp->dungeon);
    if (!thing_is_invalid(thing))
    {
        struct Room *room;
        room = find_room_with_spare_capacity(comp->dungeon->owner, RoK_TREASURE, 1);
        if (!room_is_invalid(room))
        {
            ctask->move_gold.room_idx = room->index;
            // Get a position somewhere in that room for the check below; we will use random one when dropping anyway
            pos.x.val = subtile_coord_center(slab_subtile_center(slb_num_decode_x(room->slabs_list)));
            pos.y.val = subtile_coord_center(slab_subtile_center(slb_num_decode_y(room->slabs_list)));
            pos.z.val = subtile_coord(1,0);
            if (computer_place_thing_in_power_hand(comp, thing, &pos)) {
                SYNCDBG(9,"Player %d picked %s index %d to place in %s index %d",(int)comp->dungeon->owner,
                    thing_model_name(thing),(int)thing->index,room_code_name(room->kind),(int)room->index);
                return CTaskRet_Unk2;
            } else {
                SYNCDBG(9,"Player %d cannot place %s index %d into power hand",(int)comp->dungeon->owner,
                    thing_model_name(thing),(int)thing->index);
            }
        } else
        {
            SYNCDBG(9,"Player %d has no room to place the gold into",(int)comp->dungeon->owner);
            remove_task(comp, ctask);
        }
    } else
    {
        SYNCDBG(9,"Player %d has no more gold laying around",(int)comp->dungeon->owner);
        remove_task(comp, ctask);
    }
    return CTaskRet_Unk0;
}

long task_slap_imps(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    SYNCDBG(9,"Starting");
    dungeon = comp->dungeon;
    ctask->attack_magic.repeat_num--;
    if (ctask->attack_magic.repeat_num >= 0)
    {
        TbBool allow_slap_to_kill;
        // Make sure we can accept situation where the creature will die because of the slap
        allow_slap_to_kill = computer_able_to_use_power(comp, PwrK_MKDIGGER, 0, 10);
        struct Thing *thing;
        struct CreatureControl *cctrl;
        long i;
        unsigned long k;
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
            // Don't slap if picked up or already slapped
            if (!thing_is_picked_up(thing) && !creature_affected_by_slap(thing))
            {
                // Check if we really can use the spell on that creature, considering its position and state
                if (can_cast_spell(dungeon->owner, PwrK_SLAP, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default))
                {
                    struct CreatureStats *crstat;
                    crstat = creature_stats_get_from_thing(thing);
                    // Check if the slap may cause death
                    if (allow_slap_to_kill || (crstat->slaps_to_kill < 1) || (get_creature_health_permil(thing) >= 2*1000/crstat->slaps_to_kill))
                    {
                        long state_type;
                        state_type = get_creature_state_type(thing);
                        if (state_type == CrStTyp_Work)
                        {
                            if (try_game_action(comp, dungeon->owner, GA_UsePwrSlap, 0, 0, 0, thing->index, 0) > Lb_OK)
                            {
                                return CTaskRet_Unk2;
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
    }
    remove_task(comp, ctask);
    return CTaskRet_Unk0;
}

long task_dig_to_neutral(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    short digret;
    digret = tool_dig_to_pos2(comp, &ctask->dig, 0, ToolDig_BasicOnly);
    if (digret == -5)
    {
        ctask->ottype = ctask->ttype;
        ctask->ttype = CTT_WaitForBridge;
        return CTaskRet_Unk4;
    }
    if ((digret >= -3) && (digret <= -1))
    {
        suspend_task_process(comp,ctask);
        return digret;
    }
    if ((ctask->flags & ComTsk_AddTrapLocation) != 0) {
        ctask->flags &= ~ComTsk_AddTrapLocation;
        add_to_trap_location(comp, &ctask->dig.pos_next);
    }
    return digret;
}

long task_magic_speed_up(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    struct Thing *creatng;
    int k = 0;
    SYNCDBG(9,"Starting");
    dungeon = comp->dungeon;
    creatng = thing_get(ctask->attack_magic.target_thing_idx);
    if (thing_is_invalid(creatng))
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk4;
    }
    if (creature_is_dying(creatng) || creature_is_being_unconscious(creatng) || creature_is_kept_in_custody(creatng))
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk4;
    }
    if (computer_able_to_use_power(comp, PwrK_SPEEDCRTR, ctask->attack_magic.splevel, 1) && !thing_affected_by_spell(creatng, PwrK_SPEEDCRTR))
    {
        if (try_game_action(comp, dungeon->owner, GA_UsePwrSpeedUp, ctask->attack_magic.splevel, 0, 0, ctask->attack_magic.target_thing_idx, 0) > Lb_OK)
        {
            k = 1;
        }
    } 
    else if (computer_able_to_use_power(comp, PwrK_PROTECT, ctask->attack_magic.splevel, 1) && !thing_affected_by_spell(creatng, PwrK_PROTECT))
    {
        if (try_game_action(comp, dungeon->owner, GA_UsePwrArmour, ctask->attack_magic.splevel, 0, 0, ctask->attack_magic.target_thing_idx, 0) > Lb_OK)
        {
            k = 1;
        }
    }
    if (k != 1)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk4;
    }
    else
    {
        return CTaskRet_Unk1;
    }
}

long task_wait_for_bridge(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9, "Starting");
    PlayerNumber plyr_idx;
    plyr_idx = comp->dungeon->owner;
    if (game.play_gameturn - ctask->created_turn > COMPUTER_DIG_ROOM_TIMEOUT)
    {
        //If the task has been active too long, restart the process to try a different approach.
        restart_task_process(comp, ctask);
        return CTaskRet_Unk0;
    }
    if (game.play_gameturn - ctask->created_turn > COMPUTER_URGENT_BRIDGE_TIMEOUT)
    {
        if ((is_room_available(plyr_idx, RoK_BRIDGE)) || (ctask->flags & ComTsk_Urgent))
        {
            //When the player already has the bridge available, or is doing an urgent task, don't keep the task active as long.
            restart_task_process(comp, ctask);
            return CTaskRet_Unk0;
        }
    }

    MapSubtlCoord basestl_x;
    MapSubtlCoord basestl_y;
    basestl_x = ctask->dig.pos_next.x.stl.num;
    basestl_y = ctask->dig.pos_next.y.stl.num;
    if (!is_room_available(plyr_idx, RoK_BRIDGE))
    {
        return CTaskRet_Unk4;
    }
    if (!can_build_room_at_slab(plyr_idx, RoK_BRIDGE, subtile_slab(basestl_x), subtile_slab(basestl_y)))
    {
        return CTaskRet_Unk4;
    }
    if (try_game_action(comp, plyr_idx, GA_PlaceRoom, 0, basestl_x, basestl_y, 1, RoK_BRIDGE) > Lb_OK)
    {
        long i;
        i = ctask->ottype;
        ctask->ttype = i;
        if (i == 0) {
            ERRORLOG("Bad set Task State");
        }
        return CTaskRet_Unk1;
    }
    return CTaskRet_Unk4;
}

long task_attack_magic(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    long i;
    SYNCDBG(9,"Starting");
    dungeon = comp->dungeon;
    thing = thing_get(ctask->attack_magic.target_thing_idx);
    if (thing_is_invalid(thing)) {
        return CTaskRet_Unk1;
    }
    i = ctask->attack_magic.repeat_num;
    ctask->attack_magic.repeat_num--;
    if ((i <= 0) || creature_is_dying(thing) || creature_is_being_unconscious(thing)
     || creature_is_kept_in_custody(thing))
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    // Note that can_cast_spell() shouldn't be needed here - should
    // be checked in the function which adds the task
    if (!computer_able_to_use_power(comp, ctask->attack_magic.pwkind, ctask->attack_magic.splevel, 1)) {
        return CTaskRet_Unk4;
    }
    TbResult ret;
    ret = try_game_action(comp, dungeon->owner, ctask->attack_magic.gaction, ctask->attack_magic.splevel,
        thing->mappos.x.stl.num, thing->mappos.y.stl.num, ctask->attack_magic.target_thing_idx, 0);
    if (ret <= Lb_OK)
        return CTaskRet_Unk4;
    return CTaskRet_Unk2;
}

long task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon = comp->dungeon;
    const struct TrapDoorSelling *tdsell;
    TbBool item_sold;
    long value;
    long model;
    long i;
    struct DungeonAdd *dungeonadd = get_dungeonadd(dungeon->owner);

    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon in computer player");
        return 0;
    }
    SYNCDBG(19,"Starting for player %d",(int)dungeon->owner);
    if ((ctask->sell_traps_doors.gold_gain <= ctask->sell_traps_doors.gold_gain_limit) && (dungeon->total_money_owned <= ctask->sell_traps_doors.total_money_limit))
    {
        i = 0;
        value = 0;
        item_sold = false;
        for (i=0; i < sizeof(trapdoor_sell)/sizeof(trapdoor_sell[0]); i++)
        {
            tdsell = &trapdoor_sell[ctask->sell_traps_doors.sell_idx];
            switch (tdsell->category)
            {
            case TDSC_DoorCrate:
                model = tdsell->model;
                if ((model <= 0) || (model >= gameadd.trapdoor_conf.door_types_count)) {
                    ERRORLOG("Internal error - invalid door model %d in slot %d",(int)model,(int)i);
                    break;
                }
                if (dungeonadd->mnfct_info.door_amount_placeable[model] > 0)
                {
                    int crate_source;
                    crate_source = remove_workshop_item_from_amount_stored(dungeon->owner, TCls_Door, model, WrkCrtF_Default);
                    switch (crate_source)
                    {
                    case WrkCrtS_Offmap:
                        remove_workshop_item_from_amount_placeable(dungeon->owner, TCls_Door, model);
                        item_sold = true;
                        value = compute_value_percentage(gameadd.doors_config[model].selling_value,gameadd.door_sale_percent);
                        SYNCDBG(9,"Offmap door %s crate sold for %d gold",door_code_name(model),(int)value);
                        break;
                    case WrkCrtS_Stored:
                        remove_workshop_item_from_amount_placeable(dungeon->owner, TCls_Door, model);
                        remove_workshop_object_from_player(dungeon->owner, door_crate_object_model(model));
                        item_sold = true;
                        value = compute_value_percentage(gameadd.doors_config[model].selling_value, gameadd.door_sale_percent);
                        SYNCDBG(9,"Stored door %s crate sold for %ld gold by player %d",door_code_name(model),(long)value,(int)dungeon->owner);
                        break;
                    default:
                        WARNLOG("Placeable door %s amount for player %d was incorrect; fixed",door_code_name(model),(int)dungeon->owner);
                        dungeonadd->mnfct_info.door_amount_placeable[model] = 0;
                        break;
                    }
                }
                break;
            case TDSC_TrapCrate:
                model = tdsell->model;
                if ((model <= 0) || (model >= gameadd.trapdoor_conf.trap_types_count)) {
                    ERRORLOG("Internal error - invalid trap model %d in slot %d",(int)model,(int)i);
                    break;
                }
                if (dungeonadd->mnfct_info.trap_amount_placeable[model] > 0)
                {
                    int crate_source;
                    crate_source = remove_workshop_item_from_amount_stored(dungeon->owner, TCls_Trap, model, WrkCrtF_Default);
                    switch (crate_source)
                    {
                    case WrkCrtS_Offmap:
                        remove_workshop_item_from_amount_placeable(dungeon->owner, TCls_Trap, model);
                        item_sold = true;
                        value = compute_value_percentage(gameadd.traps_config[model].selling_value, gameadd.trap_sale_percent);
                        SYNCDBG(9,"Offmap trap %s crate sold for %ld gold",trap_code_name(model),value);
                        break;
                    case WrkCrtS_Stored:
                        remove_workshop_item_from_amount_placeable(dungeon->owner, TCls_Trap, model);
                        remove_workshop_object_from_player(dungeon->owner, trap_crate_object_model(model));
                        item_sold = true;
                        value = compute_value_percentage(gameadd.traps_config[model].selling_value, gameadd.trap_sale_percent);
                        SYNCDBG(9,"Stored trap %s crate sold for %ld gold by player %d",trap_code_name(model),(long)value,(int)dungeon->owner);
                        break;
                    default:
                        WARNLOG("Placeable trap %s amount for player %d was incorrect; fixed",trap_code_name(model),(int)dungeon->owner);
                        dungeonadd->mnfct_info.trap_amount_placeable[model] = 0;
                        break;
                    }
                }
                break;
            case TDSC_DoorPlaced:
                if (!ctask->sell_traps_doors.allow_deployed)
                    break;
                model = tdsell->model;
                if ((model <= 0) || (model >= gameadd.trapdoor_conf.door_types_count)) {
                    ERRORLOG("Internal error - invalid door model %d in slot %d",(int)model,(int)i);
                    break;
                }
                {
                    struct Thing *doortng;
                    doortng = get_random_door_of_model_owned_by_and_locked(model, dungeon->owner, false);
                    if (!thing_is_invalid(doortng)) {
                        MapSubtlCoord stl_x;
                        MapSubtlCoord stl_y;
                        item_sold = true;
                        stl_x = stl_slab_center_subtile(doortng->mappos.x.stl.num);
                        stl_y = stl_slab_center_subtile(doortng->mappos.y.stl.num);
                        value = compute_value_percentage(gameadd.doors_config[model].selling_value, gameadd.door_sale_percent);
                        destroy_door(doortng);
                        if (is_my_player_number(dungeon->owner))
                            play_non_3d_sample(115);
                        dungeon->camera_deviate_jump = 192;
                        if (value != 0)
                        {
                            struct Coord3d pos;
                            set_coords_to_subtile_center(&pos,stl_x,stl_y,1);
                            create_price_effect(&pos, dungeon->owner, value);
                            add_to_trap_location(comp, &pos);
                            SYNCDBG(4,"Placed door at (%d,%d) sold for %d gold by player %d",(int)stl_x,(int)stl_y,(int)value,(int)dungeon->owner);
                        } else
                        {
                            WARNLOG("Sold door at (%d,%d) which didn't cost anything",(int)stl_x,(int)stl_y);
                        }
                    }
                }
                break;
            case TDSC_TrapPlaced:
                if (!ctask->sell_traps_doors.allow_deployed)
                    break;
                model = tdsell->model;
                if ((model <= 0) || (model >= gameadd.trapdoor_conf.trap_types_count)) {
                    ERRORLOG("Internal error - invalid trap model %d in slot %d",(int)model,(int)i);
                    break;
                }
                {
                    struct Thing *traptng;
                    traptng = get_random_trap_of_model_owned_by_and_armed(model, dungeon->owner, true);
                    if (!thing_is_invalid(traptng)) {
                        SYNCDBG(6,"Got %s index %d owner %d",thing_model_name(traptng),(int)traptng->index,(int)traptng->owner);
                        MapSubtlCoord stl_x;
                        MapSubtlCoord stl_y;
                        item_sold = true;
                        stl_x = stl_slab_center_subtile(traptng->mappos.x.stl.num);
                        stl_y = stl_slab_center_subtile(traptng->mappos.y.stl.num);
                        remove_traps_around_subtile(stl_x, stl_y, &value);
                        if (is_my_player_number(dungeon->owner))
                            play_non_3d_sample(115);
                        dungeon->camera_deviate_jump = 192;
                        if (value != 0)
                        {
                            struct Coord3d pos;
                            set_coords_to_subtile_center(&pos,stl_x,stl_y,1);
                            create_price_effect(&pos, dungeon->owner, value);
                            add_to_trap_location(comp, &pos);
                            SYNCDBG(4,"Placed traps at (%d,%d) sold for %d gold by player %d",(int)stl_x,(int)stl_y,(int)value,(int)dungeon->owner);
                        } else
                        {
                            WARNLOG("Sold traps at (%d,%d) which didn't cost anything",(int)stl_x,(int)stl_y);
                        }
                    }
                }
                break;
            default:
                ERRORLOG("Unknown SELL_ITEM type");
                break;
            }
            ctask->sell_traps_doors.sell_idx++;
            if (trapdoor_sell[ctask->sell_traps_doors.sell_idx].category == TDSC_EndList)
                ctask->sell_traps_doors.sell_idx = 0;
            if (item_sold)
            {
                ctask->sell_traps_doors.gold_gain += value;
                player_add_offmap_gold(dungeon->owner, value);
                // Mark that we've sold the item; if enough was sold, end the task
                ctask->sell_traps_doors.items_amount--;
                if (ctask->sell_traps_doors.items_amount <= 0) {
                    remove_task(comp, ctask);
                    return 1;
                }
                return 1;
            }
        }
        SYNCDBG(9,"Could not sell anything, aborting.");
    } else
    {
        SYNCDBG(9,"Initial conditions not met, aborting.");
    }
    remove_task(comp, ctask);
    return 0;
}

void setup_dig_to(struct ComputerDig *cdig, const struct Coord3d startpos, const struct Coord3d endpos)
{
    memset(cdig,0,sizeof(struct ComputerDig));
    cdig->pos_begin.x.val = startpos.x.val;
    cdig->pos_begin.y.val = startpos.y.val;
    cdig->pos_begin.z.val = startpos.z.val;
    cdig->pos_E.x.val = startpos.x.val;
    cdig->pos_E.y.val = startpos.y.val;
    cdig->pos_E.z.val = startpos.z.val;
    cdig->pos_dest.x.val = endpos.x.val;
    cdig->pos_dest.y.val = endpos.y.val;
    cdig->pos_dest.z.val = endpos.z.val;
    cdig->pos_next.x.val = 0;
    cdig->pos_next.y.val = 0;
    cdig->pos_next.z.val = 0;
    cdig->distance = LONG_MAX;
    cdig->subfield_2C = 1;
    cdig->calls_count = 0;
}

TbBool create_task_move_creature_to_subtile(struct Computer2 *comp, const struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CrtrStateId dst_state)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord(stl_x,AI_RANDOM(256));
    pos.y.val = subtile_coord(stl_y,AI_RANDOM(256));
    pos.z.val = subtile_coord(1,0);
    return create_task_move_creature_to_pos(comp, thing, pos, dst_state);
}

TbBool create_task_move_creature_to_pos(struct Computer2 *comp, const struct Thing *thing, const struct Coord3d pos, CrtrStateId dst_state)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksFrequent) != 0) {
        struct CreatureData *crdata;
        crdata = creature_data_get(thing->model);
        switch (dst_state)
        {
        case CrSt_ImpImprovesDungeon:
            message_add_fmt(comp->dungeon->owner, "This %s should go claiming.",get_string(crdata->namestr_idx));
            break;
        case CrSt_ImpDigsDirt:
            message_add_fmt(comp->dungeon->owner, "This %s should go digging.",get_string(crdata->namestr_idx));
            break;
        case CrSt_ImpMinesGold:
            message_add_fmt(comp->dungeon->owner, "This %s should go mining.",get_string(crdata->namestr_idx));
            break;
        case CrSt_CreatureDoingNothing:
        case CrSt_ImpDoingNothing:
            message_add_fmt(comp->dungeon->owner, "This %s should stop doing that.",get_string(crdata->namestr_idx));
            break;
        case CrSt_CreatureSacrifice:
            if (thing->model == gameadd.cheaper_diggers_sacrifice_model) {
                struct PowerConfigStats *powerst;
                powerst = get_power_model_stats(PwrK_MKDIGGER);
                message_add_fmt(comp->dungeon->owner, "Sacrificing %s to reduce %s price.",get_string(crdata->namestr_idx),get_string(powerst->name_stridx));
                break;
            }
            message_add_fmt(comp->dungeon->owner, "This %s will be sacrificed.",get_string(crdata->namestr_idx));
            break;
        case CrSt_Torturing:
            message_add_fmt(comp->dungeon->owner, "This %s should be tortured.", get_string(crdata->namestr_idx));
            break;
        case CrSt_CreatureDoorCombat:
            message_add_fmt(comp->dungeon->owner, "This %s should attack a door.", get_string(crdata->namestr_idx));
            break;
        default:
            message_add_fmt(comp->dungeon->owner, "This %s should go there.",get_string(crdata->namestr_idx));
            break;
        }
    }
    ctask->ttype = CTT_MoveCreatureToPos;
    ctask->move_to_pos.pos_86.x.val = pos.x.val;
    ctask->move_to_pos.pos_86.y.val = pos.y.val;
    ctask->move_to_pos.pos_86.z.val = pos.z.val;
    ctask->move_to_pos.target_thing_idx = thing->index;
    ctask->move_to_pos.word_80 = 0;
    ctask->created_turn = game.play_gameturn;
    return true;
}

TbBool create_task_move_creatures_to_defend(struct Computer2 *comp, struct Coord3d *pos, long repeat_num, unsigned long evflags)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "Minions, defend this place!");
    }
    ctask->ttype = CTT_MoveCreaturesToDefend;
    ctask->move_to_defend.target_pos.x.val = pos->x.val;
    ctask->move_to_defend.target_pos.y.val = pos->y.val;
    ctask->move_to_defend.target_pos.z.val = pos->z.val;
    ctask->move_to_defend.repeat_num = repeat_num;
    ctask->move_to_defend.field_70 = evflags;
    ctask->created_turn = game.play_gameturn;
    ctask->lastrun_turn = game.play_gameturn;
    ctask->field_60 = comp->field_34;
    return true;
}

TbBool create_task_move_creatures_to_room(struct Computer2 *comp, int room_idx, long repeat_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        struct Room *room;
        room = room_get(room_idx);
        if (room_exists(room)) {
            struct RoomConfigStats *roomst;
            roomst = &slab_conf.room_cfgstats[room->kind];
            message_add_fmt(comp->dungeon->owner, "Time to put some creatures into %s.",get_string(roomst->name_stridx));
        } else {
            if ((gameadd.computer_chat_flags & CChat_TasksFrequent) != 0)
                message_add_fmt(comp->dungeon->owner, "Time to put some creatures into rooms.");
        }
    }
    ctask->ttype = CTT_MoveCreatureToRoom;
    ctask->move_to_room.room_idx1 = room_idx;
    ctask->move_to_room.room_idx2 = room_idx;
    ctask->move_to_room.repeat_num = repeat_num;
    ctask->created_turn = game.play_gameturn;
    return true;
}

TbBool create_task_pickup_for_attack(struct Computer2 *comp, struct Coord3d *pos, long par3, long repeat_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "Minions, attack now!");
    }
    ctask->ttype = CTT_PickupForAttack;
    ctask->pickup_for_attack.target_pos.x.val = pos->x.val;
    ctask->pickup_for_attack.target_pos.y.val = pos->y.val;
    ctask->pickup_for_attack.target_pos.z.val = pos->z.val;
    ctask->pickup_for_attack.repeat_num = repeat_num;
    ctask->created_turn = game.play_gameturn;
    ctask->pickup_for_attack.long_86 = par3; // Originally only a word was set here
    return true;
}

TbBool create_task_magic_battle_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long cta_duration, long repeat_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "Minions, call to arms! Join the battle!");
    }
    ctask->ttype = CTT_MagicCallToArms;
    ctask->task_state = CTaskSt_None;
    ctask->created_turn = game.play_gameturn;
    // Initial wait before start of casting
    ctask->field_60 = 25;
    ctask->lastrun_turn = game.play_gameturn - 25;
    ctask->magic_cta.target_pos.x.val = pos->x.val;
    ctask->magic_cta.target_pos.y.val = pos->y.val;
    ctask->magic_cta.target_pos.z.val = pos->z.val;
    ctask->magic_cta.repeat_num = repeat_num;
    ctask->field_8E = cta_duration;
    return true;
}

TbBool create_task_magic_support_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long cta_duration, long par3, long repeat_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "Minions, call to arms! Attack!");
    }
    ctask->ttype = CTT_MagicCallToArms;
    ctask->task_state = CTaskSt_None;
    ctask->created_turn = game.play_gameturn;
    // Initial wait before start of casting
    ctask->field_60 = 25;
    ctask->lastrun_turn = game.play_gameturn - 25;
    ctask->magic_cta.target_pos.x.val = pos->x.val;
    ctask->magic_cta.target_pos.y.val = pos->y.val;
    ctask->magic_cta.target_pos.z.val = pos->z.val;
    ctask->magic_cta.repeat_num = repeat_num;
    ctask->field_8E = cta_duration;
    ctask->magic_cta.word_86 = par3;
    return true;
}

/**
 * Creates task of selling traps and doors to get more gold.
 * @param comp Computer player who will do the task.
 * @param num_to_sell Amount of traps/doors/crates to sell.
 * @param gold_up_to Max amount of gold to be gained by the selling.
 * @return True if the task was created successfully, false otherwise.
 */
TbBool create_task_sell_traps_and_doors(struct Computer2 *comp, long num_to_sell, GoldAmount gold_up_to, TbBool allow_deployed)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting for player %d to sell %d traps up to %d gold",(int)dungeon->owner,(int)num_to_sell,(int)gold_up_to);
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(dungeon->owner, "I will sell some traps and doors.");
    }
    ctask->ttype = CTT_SellTrapsAndDoors;
    ctask->created_turn = game.play_gameturn;
    ctask->lastrun_turn = game.play_gameturn;
    ctask->field_60 = 1;
    ctask->sell_traps_doors.items_amount = num_to_sell;
    ctask->sell_traps_doors.gold_gain = 0;
    ctask->sell_traps_doors.gold_gain_limit = gold_up_to;
    ctask->sell_traps_doors.total_money_limit = dungeon->total_money_owned + gold_up_to;
    ctask->sell_traps_doors.sell_idx = 0;
    ctask->sell_traps_doors.allow_deployed = allow_deployed;
    return true;
}

/**
 * Creates task of moving gold laying in the dungeon to treasure room.
 * @param comp Computer player who will do the task.
 * @param num_to_move Amount of gold piles/pots to move.
 * @param gold_up_to Max amount of gold to be gained by placing the gold in treasure room.
 * @return True if the task was created successfully, false otherwise.
 */
TbBool create_task_move_gold_to_treasury(struct Computer2 *comp, long num_to_move, long gold_up_to)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksFrequent) != 0) {
        message_add_fmt(comp->dungeon->owner, "Gold should not lay around outside treasury.");
    }
    ctask->ttype = CTT_MoveGoldToTreasury;
    ctask->created_turn = game.play_gameturn;
    ctask->lastrun_turn = game.play_gameturn;
    ctask->field_60 = 1;
    ctask->move_gold.items_amount = num_to_move;
    ctask->move_gold.gold_gain = 0;
    ctask->move_gold.gold_gain_limit = gold_up_to;
    ctask->move_gold.total_money_limit = gold_up_to;
    return true;
}

TbBool create_task_dig_to_attack(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, PlayerNumber victim_plyr_idx, long parent_cproc_idx)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "Player %d looks like he need a kick.",(int)victim_plyr_idx);
    }
    ctask->ttype = CTT_DigToAttack;
    ctask->dig_somewhere.startpos.x.val = startpos.x.val;
    ctask->dig_somewhere.startpos.y.val = startpos.y.val;
    ctask->dig_somewhere.startpos.z.val = startpos.z.val;
    ctask->dig_somewhere.endpos.x.val = endpos.x.val;
    ctask->dig_somewhere.endpos.y.val = endpos.y.val;
    ctask->dig_somewhere.endpos.z.val = endpos.z.val;
    ctask->field_8C = parent_cproc_idx;
    ctask->dig_somewhere.target_plyr_idx = victim_plyr_idx;
    ctask->lastrun_turn = 0;
    ctask->flags |= ComTsk_AddTrapLocation;
    // Setup the digging
    setup_dig_to(&ctask->dig, startpos, endpos);
    return true;
}

TbBool create_task_dig_to_neutral(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "Localized neutral place, hopefully with loot.");
    }
    ctask->ttype = CTT_DigToNeutral;
    ctask->dig_somewhere.byte_80 = 0;
    ctask->dig_somewhere.startpos.x.val = startpos.x.val;
    ctask->dig_somewhere.startpos.y.val = startpos.y.val;
    ctask->dig_somewhere.startpos.z.val = startpos.z.val;
    ctask->dig_somewhere.endpos.x.val = endpos.x.val;
    ctask->dig_somewhere.endpos.y.val = endpos.y.val;
    ctask->dig_somewhere.endpos.z.val = endpos.z.val;
    ctask->flags |= ComTsk_AddTrapLocation;
    ctask->created_turn = game.play_gameturn;
    setup_dig_to(&ctask->dig, startpos, endpos);
    return true;
}

TbBool create_task_dig_to_gold(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, long parent_cproc_idx, long count_slabs_to_dig, long gold_lookup_idx)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "Time to dig more gold.");
    }
    ctask->ttype = CTT_DigToGold;
    ctask->flags |= ComTsk_AddTrapLocation;
    ctask->dig_to_gold.startpos.x.val = startpos.x.val;
    ctask->dig_to_gold.startpos.y.val = startpos.y.val;
    ctask->dig_to_gold.startpos.z.val = startpos.z.val;
    ctask->dig_to_gold.endpos.x.val = endpos.x.val;
    ctask->dig_to_gold.endpos.y.val = endpos.y.val;
    ctask->dig_to_gold.endpos.z.val = endpos.z.val;
    ctask->dig_to_gold.slabs_dig_count = count_slabs_to_dig;
    ctask->field_8C = parent_cproc_idx;
    ctask->dig_to_gold.target_lookup_idx = gold_lookup_idx;
    // Setup the digging
    setup_dig_to(&ctask->dig, startpos, endpos);
    return true;
}

TbBool create_task_dig_to_entrance(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, long parent_cproc_idx, long entroom_idx)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        struct RoomConfigStats *roomst;
        roomst = &slab_conf.room_cfgstats[RoK_ENTRANCE];
        message_add_fmt(comp->dungeon->owner, "I will take that %s.",get_string(roomst->name_stridx));
    }
    ctask->ttype = CTT_DigToEntrance;
    ctask->flags |= ComTsk_AddTrapLocation;
    ctask->flags |= ComTsk_Urgent;
    ctask->dig_to_room.startpos.x.val = startpos.x.val;
    ctask->dig_to_room.startpos.y.val = startpos.y.val;
    ctask->dig_to_room.startpos.z.val = startpos.z.val;
    ctask->dig_to_room.endpos.x.val = endpos.x.val;
    ctask->dig_to_room.endpos.y.val = endpos.y.val;
    ctask->dig_to_room.endpos.z.val = endpos.z.val;
    ctask->field_8C = parent_cproc_idx;
    ctask->dig_to_room.target_room_idx = entroom_idx;
    // Setup the digging
    setup_dig_to(&ctask->dig, startpos, endpos);
    return true;
}

TbBool create_task_slap_imps(struct Computer2 *comp, long creatrs_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksFrequent) != 0) {
        message_add_fmt(comp->dungeon->owner, "Work harder, minions!");
    }
    ctask->ttype = CTT_SlapDiggers;
    ctask->attack_magic.repeat_num = creatrs_num;
    ctask->created_turn = game.play_gameturn;
    return true;
}

//task is named 'speed up', but it's generated from 'check fighter' event and all round buffs units. Not to be confused
//with check_for_accelerate which cast speed outside of combat
TbBool create_task_magic_speed_up(struct Computer2 *comp, const struct Thing *creatng, long splevel)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        message_add_fmt(comp->dungeon->owner, "I should buff my fighters.");
    }
    ctask->ttype = CTT_MagicSpeedUp;
    ctask->attack_magic.target_thing_idx = creatng->index;
    ctask->attack_magic.splevel = splevel;
    ctask->created_turn = game.play_gameturn;
    return true;
}

TbBool create_task_attack_magic(struct Computer2 *comp, const struct Thing *creatng, PowerKind pwkind, int repeat_num, int splevel, int gaction)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    if ((gameadd.computer_chat_flags & CChat_TasksScarce) != 0) {
        struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(pwkind);
        struct CreatureData *crdata;
        crdata = creature_data_get(creatng->model);
        message_add_fmt(comp->dungeon->owner, "Casting %s on %s!",get_string(powerst->name_stridx),get_string(crdata->namestr_idx));
    }
    ctask->ttype = CTT_AttackMagic;
    ctask->attack_magic.target_thing_idx = creatng->index;
    ctask->attack_magic.splevel = splevel;
    ctask->attack_magic.repeat_num = repeat_num;
    ctask->attack_magic.gaction = gaction;
    ctask->attack_magic.pwkind = pwkind;
    ctask->created_turn = game.play_gameturn;
    return true;
}

long process_tasks(struct Computer2 *comp)
{
    struct ComputerTask *ctask;
    long ndone;
    long i;
    long n;
    unsigned long k;
    ndone = 0;
    k = 0;
    i = comp->task_idx;
    while (i != 0)
    {
        if ((i < 0) || (i >= COMPUTER_TASKS_COUNT))
        {
          ERRORLOG("Jump to invalid computer task %ld detected",i);
          break;
        }
        if (comp->tasks_did <= 0)
            break;
        ctask = &game.computer_task[i];
        i = ctask->next_task;
        if ((ctask->flags & ComTsk_Unkn0001) != 0)
        {
            n = ctask->ttype;
            if ((n > 0) && (n < sizeof(task_function)/sizeof(task_function[0])))
            {
                SYNCDBG(12,"Task %s",computer_task_code_name(n));
                task_function[n].func(comp, ctask);
                ndone++;
            } else
            {
                ERRORLOG("Bad Computer Task Type %d at index %d, removing",(int)n,(int)i);
                remove_task(comp, ctask);
            }
        }
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping computer tasks");
          break;
        }
    }
    return ndone;
}
/******************************************************************************/
