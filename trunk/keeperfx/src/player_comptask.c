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
#include "magic.h"
#include "thing_traps.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "player_instances.h"
#include "room_jobs.h"
#include "room_workshop.h"

#include "dungeon_data.h"
#include "map_blocks.h"
#include "slab_data.h"
#include "power_hand.h"
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

struct MoveToRoom {
    char kind;
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
long add_to_trap_location(struct Computer2 *, struct Coord3d *);
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

const struct MoveToRoom move_to_room[] = {
    {RoK_TRAINING,  40},
    {RoK_LIBRARY,   35},
    {RoK_WORKSHOP,  32},
    {RoK_SCAVENGER, 20},
    {RoK_GUARDPOST,  2},
    {RoK_BARRACKS,   1},
    {RoK_NONE,       0},
};

const struct MyLookup lookup[] = {
    { 0, -3},
    { 3,  0},
    { 0,  3},
    {-3,  0}
};

/******************************************************************************/
DLLIMPORT long _DK_task_dig_room_passage(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_room(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_check_room_dug(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_place_room(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_to_entrance(struct Computer2 *comp, struct ComputerTask *ctask);
//DLLIMPORT long _DK_task_dig_to_gold(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_to_attack(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_magic_call_to_arms(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_pickup_for_attack(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_move_creature_to_room(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_move_creature_to_pos(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_move_creatures_to_defend(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_slap_imps(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_dig_to_neutral(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_magic_speed_up(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_wait_for_bridge(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_attack_magic(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT long _DK_task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask);
DLLIMPORT struct ComputerTask *_DK_get_task_in_progress(struct Computer2 *comp, long basestl_y);
DLLIMPORT struct ComputerTask *_DK_get_free_task(struct Computer2 *comp, long basestl_y);
DLLIMPORT short _DK_fake_dump_held_creatures_on_map(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_fake_place_thing_in_power_hand(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos);
DLLIMPORT struct Thing *_DK_find_creature_to_be_placed_in_room(struct Computer2 *comp, struct Room **roomp);
DLLIMPORT short _DK_game_action(char basestl_x, unsigned short basestl_y, unsigned short plyr_idx, unsigned short a4,
 unsigned short a5, unsigned short a6, unsigned short a7);
DLLIMPORT short _DK_tool_dig_to_pos2(struct Computer2 *, struct ComputerDig *, long, long);
DLLIMPORT long _DK_add_to_trap_location(struct Computer2 *, struct Coord3d *);
//DLLIMPORT long _DK_find_next_gold(struct Computer2 *, struct ComputerTask *);
DLLIMPORT long _DK_check_for_gold(long simulation, long digflags, long l3);
DLLIMPORT int _DK_search_spiral(struct Coord3d *pos, int owner, int i3, long (*cb)(long, long, long));
DLLIMPORT long _DK_dig_to_position(signed char basestl_x, unsigned short basestl_y, unsigned short plyr_idx, unsigned char a4, unsigned char a5);
DLLIMPORT short _DK_get_hug_side(struct ComputerDig * cdig, unsigned short basestl_y, unsigned short plyr_idx, unsigned short a4, unsigned short a5, unsigned short a6, unsigned short a7);
DLLIMPORT struct ComputerTask * _DK_able_to_build_room(struct Computer2 *comp, struct Coord3d *pos, unsigned short width_slabs, unsigned short height_slabs, long a5, long a6, long a7);
DLLIMPORT struct Thing *_DK_find_creature_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, long a4);
DLLIMPORT long _DK_get_ceiling_height_above_thing_at(struct Thing *thing, struct Coord3d *pos);
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
            cproc->flags &= ~0x0020;
            cproc->flags &= ~0x0008;
        }
    } else {
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
    MapSlabCoord slb_x,slb_y;
    struct Thing *thing;
    long i,k;
    struct SlabMap *slb;
    struct Room *room;
    SYNCDBG(9,"Starting action %d",(int)gaction);
    //return _DK_game_action(plyr_idx, gaction, alevel, stl_x, stl_y, param1, param2);
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
        if (!is_power_available(plyr_idx, PwrK_HAND))
            break;
        if (!place_thing_in_power_hand(thing, plyr_idx))
          return Lb_FAIL;
        return Lb_SUCCESS;
    case GA_UsePwrHandDrop:
        // Note that we can drop things even if we have no hand power
        if (!dump_held_things_on_map(plyr_idx, stl_x, stl_y, 0))
            return Lb_FAIL;
        return Lb_SUCCESS;
    case GA_UseMkDigger:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_MKDIGGER, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_UseSlap:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, alevel, stl_x, stl_y, thing);
    case GA_UsePwrSight:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_SIGHT, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_UsePwrObey:
        return magic_use_available_power_on_level(plyr_idx, PwrK_OBEY, alevel);
    case GA_UsePwrHealCrtr:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_HEALCRTR, alevel, stl_x, stl_y, thing);
    case GA_UsePwrCall2Arms:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_CALL2ARMS, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_UsePwrCaveIn:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_CAVEIN, alevel, stl_x, stl_y, PwCast_Unrevealed);
    case GA_StopPwrCall2Arms:
        turn_off_call_to_arms(plyr_idx);
        return Lb_SUCCESS;
    case GA_Unk12:
        dungeon->field_88C[0] = 0;
        return Lb_SUCCESS;
    case GA_Unk13:
    case GA_MarkDig:
        slb = get_slabmap_block(slb_x, slb_y);
        if ((slb->kind == SlbT_LAVA) || (slb->kind == SlbT_WATER))
        {
            place_slab_type_on_map(SlbT_PATH, stl_x, stl_y, plyr_idx, 0);
            do_slab_efficiency_alteration(slb_x, slb_y);
            i = 1;
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
        return room->index;
    case GA_SetTendencies:
        dungeon->creature_tendencies = param1;
        return 1;
    case GA_PlaceTrap:
        if (!player_place_trap_at(stl_x, stl_y, plyr_idx, param1))
            break;
        return 1;
    case GA_PlaceDoor:
        k = tag_cursor_blocks_place_door(plyr_idx, stl_x, stl_y);
        i = packet_place_door(stl_x, stl_y, plyr_idx, param1, k);
        return i;
    case GA_UsePwrLightning:
        return magic_use_available_power_on_subtile(plyr_idx, PwrK_LIGHTNING, alevel, stl_x, stl_y, PwCast_None);
    case GA_UsePwrSpeedUp:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_SPEEDCRTR, alevel, stl_x, stl_y, thing);
    case GA_UsePwrArmour:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_PROTECT, alevel, stl_x, stl_y, thing);
    case GA_UsePwrConceal:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_CONCEAL, alevel, stl_x, stl_y, thing);
    case GA_UsePwrHoldAudnc:
        return magic_use_available_power_on_level(plyr_idx, PwrK_HOLDAUDNC, alevel);
    case GA_UsePwrDisease:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_DISEASE, alevel, stl_x, stl_y, thing);
    case GA_UsePwrChicken:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_CHICKEN, alevel, stl_x, stl_y, thing);
    case GA_UsePwrSlap:
        thing = thing_get(param1);
        return magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, alevel, stl_x, stl_y, thing);
    default:
        ERRORLOG("Unknown game action %d", (int)gaction);
        break;
    }
    return 0;
}

TbResult try_game_action(struct Computer2 *comp, PlayerNumber plyr_idx, unsigned short gaction, unsigned short alevel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2)
{
    TbResult result;
    result = game_action(plyr_idx, gaction, alevel, stl_x, stl_y, param1, param2);
    if (result > 0) {
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
struct ComputerTask *get_task_in_progress(struct Computer2 *comp, long ttype)
{
    struct ComputerTask *ctask;
    long i;
    unsigned long k;
    //return _DK_get_task_in_progress(comp, a2);
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
        if ((ctask->flags & 0x01) != 0)
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
    return NULL;
}

/**
 * Checks if given computer player has in progress task of given type.
 * @param comp The computer player to be checked.
 * @param ttype Task type to search for.
 * @return True if computer player has at least one such task, false otherwise.
 */
TbBool is_task_in_progress(struct Computer2 *comp, long ttype)
{
    struct ComputerTask *ctask;
    ctask = get_task_in_progress(comp, ttype);
    return !computer_task_invalid(ctask);
}

struct ComputerTask *get_free_task(struct Computer2 *comp, long a2)
{
    return _DK_get_free_task(comp, a2);
}

long get_ceiling_height_above_thing_at(struct Thing *thing, struct Coord3d *pos)
{
    //return _DK_get_ceiling_height_above_thing_at(thing, pos);
    int nav_sizexy;
    if (thing_is_creature(thing))
        nav_sizexy = thing_nav_sizexy(thing);
    else
        nav_sizexy = thing->sizexy;

    int nav_radius;
    nav_radius = (nav_sizexy / 2);
    int xstart, ystart, xend, yend;
    xstart = (int)pos->x.val - nav_radius;
    if (xstart < 0)
        xstart = 0;
    ystart = (int)pos->y.val - nav_radius;
    if (ystart < 0)
        ystart = 0;
    xend = (int)pos->x.val + nav_radius;
    if (xend >= 65535)
        xend = 65535;
    yend = (int)pos->y.val + nav_radius;
    if (yend >= 65535)
        yend = 65535;
    // Set initial values for computing floor and ceiling heights
    MapSubtlCoord floor_height, ceiling_height;
    floor_height = 0;
    ceiling_height = 15;
    // Sweep through subtiles and select highest floor and lowest ceiling
    int x,y;
    for (y=ystart; y < yend; y += 256)
    {
        for (x=xstart; x < xend; x += 256)
        {
            update_floor_and_ceiling_heights_at(coord_subtile(x), coord_subtile(y), &floor_height, &ceiling_height);
            SYNCDBG(19,"Ceiling %d after (%d,%d)", (int)ceiling_height,(int)x>>8,(int)y>>8);
        }
    }
    // Assuming xend may not be multiplication of 255, treat it separately
    for (y=ystart; y < yend; y += 256)
    {
        x = xend;
        {
            update_floor_and_ceiling_heights_at(coord_subtile(x), coord_subtile(y), &floor_height, &ceiling_height);
        }
    }
    // Assuming yend may not be multiplication of 255, treat it separately
    y = yend;
    {
        for (x=xstart; x < xend; x += 256)
        {
            update_floor_and_ceiling_heights_at(coord_subtile(x), coord_subtile(y), &floor_height, &ceiling_height);
        }
    }
    // For both xend and yend at max
    x = xend; y = yend;
    update_floor_and_ceiling_heights_at(coord_subtile(x), coord_subtile(y), &floor_height, &ceiling_height);
    // Now we can be sure the value is correct
    SYNCDBG(19,"Ceiling %d after (%d,%d)", (int)ceiling_height,(int)xend>>8,(int)yend>>8);
    return subtile_coord(ceiling_height,0);
}

/**
 * Low level function which unconditionally drops creature held in hand by computer player.
 * @param comp
 * @param thing
 * @param pos
 */
void computer_drop_held_thing_at(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos)
{
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    remove_thing_from_limbo(thing);
    if (thing_is_creature(thing)) {
        initialise_thing_state(thing, CrSt_CreatureBeingDropped);
    }
}

/**
 * Low level function which unconditionally picks creature by computer player to hand.
 * @param comp
 * @param thing
 */
void computer_pick_thing_by_hand(struct Computer2 *comp, struct Thing *thing)
{
    if (thing_is_creature(thing)) {
        clear_creature_instance(thing);
        external_set_thing_state(thing, CrSt_InPowerHand);
    }
    place_thing_in_limbo(thing);
}

short fake_dump_held_creatures_on_map(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos)
{
    if (thing_is_creature(thing) &&(thing->active_state == CrSt_CreatureUnconscious)) {
        WARNLOG("The %s Held By computer is unconscious",creature_code_name(thing->model));
    }
    //return _DK_fake_dump_held_creatures_on_map(comp, thing, pos);
    if (!computer_find_non_solid_block(comp, pos)) {
        return 0;
    }
    if (!can_place_thing_here(thing, pos->x.stl.num, pos->y.stl.num, comp->dungeon->owner)) {
        return 0;
    }
    struct Coord3d locpos;
    locpos.z.val = 0;
    locpos.x.val = subtile_coord_center(pos->x.stl.num);
    locpos.y.val = subtile_coord_center(pos->y.stl.num);
    locpos.z.val = get_thing_height_at(thing, &locpos);
    int height,max_height;
    max_height = get_ceiling_height_above_thing_at(thing, &locpos);
    height = locpos.z.val + thing->field_58;
    if (max_height <= height) {
        ERRORLOG("Ceiling is too low to drop %s at (%d,%d)", thing_model_name(thing),(int)locpos.x.stl.num,(int)locpos.y.stl.num);
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
    computer_drop_held_thing_at(comp, thing, &locpos);
    comp->held_thing_idx = 0;
    comp->tasks_did--;
    return 1;
}

long fake_place_thing_in_power_hand(struct Computer2 *comp, struct Thing *thing, struct Coord3d *pos)
{
    SYNCDBG(9,"Starting");
    //return _DK_fake_place_thing_in_power_hand(comp, thing, pos);
    if (!can_thing_be_picked_up_by_player(thing, comp->dungeon->owner)) {
        ERRORLOG("Computer tries to pick up %s which is not pickable", thing_model_name(thing));
        return 0;
    }
    if ((thing->alloc_flags & 0x20) != 0) {
        return 0;
    }
    if (!computer_find_non_solid_block(comp, pos)) {
        return 0;
    }
    if (!can_place_thing_here(thing, pos->x.stl.num, pos->y.stl.num, comp->dungeon->owner)) {
        return 0;
    }
    computer_pick_thing_by_hand(comp, thing);
    comp->held_thing_idx = thing->index;
    comp->tasks_did--;
    return 1;
}

TbBool fake_force_dump_held_things_on_map(struct Computer2 *comp, struct Coord3d *pos)
{
    struct Thing *thing;
    thing = thing_get(comp->held_thing_idx);
    if (thing_is_invalid(thing)) {
        return false;
    }
    struct Coord3d locpos;
    locpos.z.val = 0;
    locpos.x.val = subtile_coord_center(pos->x.stl.num);
    locpos.y.val = subtile_coord_center(pos->y.stl.num);
    locpos.z.val = get_thing_height_at(thing, &locpos);
    computer_drop_held_thing_at(comp, thing, &locpos);
    comp->held_thing_idx = 0;
    return true;
}

TbBool creature_could_be_placed_in_better_room(const struct Computer2 *comp, const struct Thing *thing)
{
    const struct Dungeon *dungeon;
    struct Room *chosen_room;
    long k,rkind;
    TbBool better_job_allowed;
    SYNCDBG(19,"Starting");
    dungeon = comp->dungeon;
    // Choose the room we're currently working in, and check it on the list
    chosen_room = get_room_creature_works_in(thing);
    if (!room_exists(chosen_room)) {
        return true;
    }
    better_job_allowed = false;
    for (k=0; move_to_room[k].kind != RoK_NONE; k++)
    {
        rkind = move_to_room[k].kind;
        if (rkind == chosen_room->kind)
        {
            return better_job_allowed;
        }
        if (dungeon_has_room(dungeon, rkind)
         && creature_can_do_job_for_player_in_room(thing, dungeon->owner, rkind)
         && worker_needed_in_dungeons_room_kind(dungeon, rkind)) {
            better_job_allowed = true;
        }
    }
    return false;
}
struct Room *get_room_to_place_creature(const struct Computer2 *comp, const struct Thing *thing)
{
    const struct Dungeon *dungeon;
    long chosen_priority;
    struct Room *chosen_room;
    struct Room *room;
    long total_spare_cap;
    long i,k,rkind;

    dungeon = comp->dungeon;

    chosen_room = INVALID_ROOM;
    chosen_priority = LONG_MIN;
    for (k=0; move_to_room[k].kind != RoK_NONE; k++)
    {
        rkind = move_to_room[k].kind;
        if (!creature_can_do_job_for_player_in_room(thing, dungeon->owner, rkind)) {
            continue;
        }
        if (!worker_needed_in_dungeons_room_kind(dungeon, rkind)) {
            continue;
        }
        // Find specific room which meets capacity demands
        i = dungeon->room_kind[rkind];
        room = find_room_with_most_spare_capacity_starting_with(i,&total_spare_cap);
        if (room_is_invalid(room))
            continue;
        if (chosen_priority < total_spare_cap * move_to_room[k].priority)
        {
            chosen_priority = total_spare_cap * move_to_room[k].priority;
            chosen_room = room;
        }
    }
    return chosen_room;
}

struct Thing *find_creature_to_be_placed_in_room(struct Computer2 *comp, struct Room **roomp)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *thing;
    struct Room *room;
    SYNCDBG(9,"Starting");
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon in computer player.");
        return INVALID_THING;
    }
    //return _DK_find_creature_to_be_placed_in_room(comp, roomp);
    param.ptr1 = (void *)comp;
    param.num2 = RoK_NONE; // Our filter function will update that
    filter = player_list_creature_filter_needs_to_be_placed_in_room;
    thing = get_player_list_random_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    if (thing_is_invalid(thing)) {
        return INVALID_THING;
    }
    // We won't allow the creature to be picked if we want it to be placed in the same room it is now.
    // The filter function took care of most such situations, but it is still possible that the creature
    // won't be able or will not want to work in that room, and will be picked up and dropped over and over.
    // This will prevent such situation, at least to the moment when the creature leaves the room.
    room = get_room_thing_is_on(thing);
    if (!room_is_invalid(room) && (room->kind == param.num2) && (room->owner == thing->owner)) {
        WARNDBG(4,"The %s owned by player %d already is in %s, but goes for %s instead of work there",
            thing_model_name(thing),(int)thing->owner,room_code_name(room->kind),creatrtng_realstate_name(thing));
        return INVALID_THING;
    }
    room = get_room_of_given_kind_for_thing(thing,dungeon,param.num2);
    if (room_is_invalid(room))
        return INVALID_THING;
    *roomp = room;
    return thing;
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
    //return _DK_task_dig_room_passage(comp,ctask);
    struct Coord3d pos;
    struct ComputerProcess *cproc;
    switch (tool_dig_to_pos2(comp, &ctask->dig, 0, 0))
    {
    case -5:
        ctask->ottype = ctask->ttype;
        ctask->ttype = 16;
        return 4;
    case -3:
    case -2:
        cproc =  get_computer_process(comp, ctask->field_8C);
        if ((cproc->flags & 0x20) != 0) {
            shut_down_process(comp, cproc);
        }
        if (!computer_task_invalid(ctask)) {
            remove_task(comp, ctask);
        }
        return 0;
    case -1:
        move_imp_to_dig_here(comp, &ctask->pos_6A, 1);
        pos.x.val = ctask->pos_64.x.val;
        pos.y.val = ctask->pos_64.y.val;
        pos.z.val = ctask->pos_64.z.val;
        setup_computer_dig_room(&ctask->dig, &pos, ctask->long_86);
        ctask->ttype = 2;
        return 1;
    default:
        if ((ctask->flags & 0x04) != 0)
        {
            ctask->flags &= ~0x04;
            add_to_trap_location(comp, &ctask->dig.pos_next);
        }
        return 2;
    }
}

long task_dig_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    //return _DK_task_dig_room(comp,ctask);
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    {
        int digger_tasks;
        digger_tasks = dungeon->digger_stack_length;
        if ((digger_tasks > 0) && (comp->field_1C * dungeon->total_area / 100 <= digger_tasks)) {
            return 2;
        }
    }
    MapSubtlCoord stl_x, stl_y;
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
                if (slbattr->is_unknflg14 && (slb->kind != SlbT_GEMS))
                {
                    if (((mapblk->flags & 0x20) == 0) || (slabmap_owner(slb) == dungeon->owner))
                    {
                        if (find_from_task_list(dungeon->owner, get_subtile_number(stl_x,stl_y)) < 0)
                        {
                            if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, stl_x, stl_y, 1, 1) <= 0)
                            {
                                struct ComputerProcess *cproc;
                                cproc =  get_computer_process(comp, ctask->field_8C);
                                if ((cproc->flags & 0x20) != 0) {
                                    shut_down_process(comp, cproc);
                                }
                                if (!computer_task_invalid(ctask)) {
                                    remove_task(comp, ctask);
                                }
                                return 0;
                            }
                        }
                    }
                }
                ctask->dig.subfield_44++;
                if (ctask->dig.subfield_44 >= ctask->dig.subfield_40) {
                    ctask->ttype = 3;
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
    int m, n;
    long nchecked,nwaiting,nwrong;
    int i,imax;
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
        int lkp_x,lkp_y;
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
    if (game.play_gameturn - ctask->field_A > COMPUTER_DIG_ROOM_TIMEOUT) {
        WARNLOG("Task %d couldn't be completed in reasonable time, reset.",(int)ctask->ttype);
        restart_task_process(comp, ctask);
        return 0;
    }
    //return _DK_task_check_room_dug(comp,ctask);
    long waiting_slabs,wrong_slabs;
    waiting_slabs = 0; wrong_slabs = 0;
    count_slabs_where_room_cannot_be_built(comp->dungeon->owner, ctask->pos_64.x.stl.num, ctask->pos_64.y.stl.num,
        ctask->long_80, ctask->long_86, &waiting_slabs, &wrong_slabs);
    if (wrong_slabs > 0) {
        WARNLOG("Task %d couldn't be completed as %d wrong slabs are in destination area, reset.",(int)ctask->ttype,(int)wrong_slabs);
        restart_task_process(comp, ctask);
        return 0;
    }
    if (waiting_slabs > 0) {
        SYNCDBG(9,"The %d/%d tiles around %d,%d are not ready to place room",(int)wrong_slabs,
            (int)ctask->long_86, (int)ctask->pos_64.x.stl.num, (int)ctask->pos_64.y.stl.num);
        return 4;
    }
    ctask->ttype = CTT_PlaceRoom;
    setup_computer_dig_room(&ctask->dig, &ctask->pos_64, ctask->long_86);
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
    MapSubtlCoord stl_x, stl_y;
    int i;
    SYNCDBG(9,"Starting");
    //return _DK_task_place_room(comp,ctask);
    dungeon = comp->dungeon;
    rkind = ctask->long_80;
    rstat = room_stats_get_for_kind(rkind);
    // If we don't have money for the room - don't even try
    if (rstat->cost >= dungeon->total_money_owned) {
        return 0;
    }
    stl_x = ctask->dig.pos_begin.x.stl.num;
    stl_y = ctask->dig.pos_begin.y.stl.num;
    for (i = ctask->dig.subfield_2C; i > 0; i--)
    {
        if (ctask->dig.subfield_38 > 0)
        {
            struct SlabMap *slb;
            slb = get_slabmap_for_subtile(stl_x, stl_y);
            if ((slb->kind == SlbT_CLAIMED) && (slabmap_owner(slb) == dungeon->owner))
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
    //return _DK_task_dig_to_entrance(comp,ctask);
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    struct Room *room;
    long dig_ret;
    dig_ret = 0;
    int n;
    for (n=0; n < SMALL_AROUND_LENGTH; n++)
    {
        MapSubtlCoord stl_x, stl_y;
        stl_x = stl_slab_center_subtile(ctask->dig.pos_begin.x.stl.num) + small_around[n].delta_x;
        stl_y = stl_slab_center_subtile(ctask->dig.pos_begin.y.stl.num) + small_around[n].delta_y;
        room = subtile_room_get(stl_x, stl_y);
        if (!room_is_invalid(room))
        {
            if (room->index == ctask->word_80) {
                dig_ret = -1;
                break;
            }
        }
    }
    struct ComputerTask *curtask;
    if (dig_ret == 0)
    {
        dig_ret = tool_dig_to_pos2(comp, &ctask->dig, 0, 0);
        if ((ctask->flags & 0x04) != 0) {
            ctask->flags &= ~0x04;
            add_to_trap_location(comp, &ctask->dig.pos_next);
        }
    }
    switch ( dig_ret )
    {
    case -5:
        ctask->ottype = ctask->ttype;
        ctask->ttype = 16;
        return 4;
    case -3:
    case -2:
        room = room_get(ctask->word_80);
        room->field_12[dungeon->owner] |= 0x02;
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

TbBool slab_good_for_computer_dig_path(const struct SlabMap *slb)
{
    const struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    if ( ((slbattr->flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) || (slb->kind == SlbT_LAVA) )
        return true;
    return false;
}

TbBool is_valid_hug_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb;
    const struct SlabAttr *slbattr;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    slbattr = get_slab_attrs(slb);
    if ((slbattr->is_unknflg14) && (slb->kind != SlbT_GEMS))
    {
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_x, stl_y);
        if (((mapblk->flags & MapFlg_Unkn20) == 0) || (slabmap_owner(slb) == plyr_idx)) {
            SYNCDBG(17,"Subtile (%d,%d) rejected based on attrs",(int)stl_x,(int)stl_y);
            return false;
        }
    }
    if (!slab_good_for_computer_dig_path(slb)) {
        SYNCDBG(17,"Subtile (%d,%d) rejected as not good for dig",(int)stl_x,(int)stl_y);
        return false;
    }
    return true;
}

long dig_to_position(PlayerNumber plyr_idx, MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, int direction_around, TbBool revside)
{
    long i,n,nchange;
    //return _DK_dig_to_position(a1, a2, a3, start_side, revside);
    SYNCDBG(14,"Starting for subtile (%d,%d)",(int)basestl_x,(int)basestl_y);
    if (revside) {
      nchange = 1;
    } else {
      nchange = 3;
    }
    n = (direction_around + 4 - nchange) & 3;
    for (i=0; i < 4; i++)
    {
        MapSubtlCoord stl_x,stl_y;
        stl_x = basestl_x + STL_PER_SLB * (int)small_around[n].delta_x;
        stl_y = basestl_y + STL_PER_SLB * (int)small_around[n].delta_y;
        if (!is_valid_hug_subtile(stl_x, stl_y, plyr_idx))
        {
            SYNCDBG(7,"Subtile (%d,%d) accepted",(int)stl_x,(int)stl_y);
            SubtlCodedCoords stl_num;
            stl_num = get_subtile_number(stl_x, stl_y);
            return stl_num;
        }
        n = (n + nchange) & 3;
    }
    return -1;
}

short get_hug_side(struct ComputerDig * cdig, MapSubtlCoord stl1_x, MapSubtlCoord stl1_y, MapSubtlCoord stl2_x, MapSubtlCoord stl2_y, unsigned short a6, PlayerNumber plyr_idx)
{
    SYNCDBG(4,"Starting");
    //return _DK_get_hug_side(cdig, stl1_x, stl1_y, stl2_x, stl2_y, a6, plyr_idx);
    MapSubtlCoord stl_b_x, stl_b_y;
    MapSubtlCoord stl_a_x, stl_a_y;
    int maxdist_a,maxdist_b;
    short round_a,round_b;
    char state_a, state_b;
    int dist;
    int i;
    stl_a_x = stl1_x;
    stl_a_y = stl1_y;
    round_a = (a6 + 1) & 3;
    dist = max(abs(stl1_x - stl2_x), abs(stl1_y - stl2_y));
    state_a = 0;
    maxdist_a = dist - 1;
    state_b = 0;
    stl_b_x = stl1_x;
    stl_b_y = stl1_y;
    round_b = (a6 - 1) & 3;
    maxdist_b = dist - 1;
    for (i = 150; i > 0; i--)
    {
        int dx,dy;
        unsigned short round_idx;
        if ((state_a == 2) && (state_b == 2)) {
            break;
        }
        if (state_a != 2)
        {
            round_idx = (((LbArcTanAngle(stl2_x - stl_a_x, stl2_y - stl_a_y) & LbFPMath_AngleMask) + LbFPMath_PI/4) >> 9) & 3;
            dist = max(abs(stl_a_x - stl2_x), abs(stl_a_y - stl2_y));
            dx = small_around[round_idx].delta_x;
            dy = small_around[round_idx].delta_y;
            if ((dist <= maxdist_a) && !is_valid_hug_subtile(stl_a_x + 3*dx, stl_a_y + 3*dy, plyr_idx))
            {
                stl_a_x += 3*dx;
                stl_a_y += 3*dy;
                state_a = 1;
                maxdist_a = max(abs(stl_a_x - stl2_x), abs(stl_a_y - stl2_y));
            } else
            if (state_a == 1)
            {
                state_a = 2;
            } else
            {
                int n;
                round_idx = (round_a - 1) & 3;
                for (n = 0; n < 4; n++)
                {
                    dx = small_around[round_idx].delta_x;
                    dy = small_around[round_idx].delta_y;
                    if (!is_valid_hug_subtile(stl_a_x + 3*dx, stl_a_y + 3*dy, plyr_idx))
                    {
                        round_a = round_idx;
                        stl_a_x += 3*dx;
                        stl_a_y += 3*dy;
                        break;
                    }
                    round_idx = (round_idx + 1) & 3;
                }
            }
            if ((stl_a_x == stl2_x) && (stl_a_y == stl2_y)) {
                return 1;
            }
        }
        if (state_b != 2)
        {
            round_idx = (((LbArcTanAngle(stl2_x - stl_b_x, stl2_y - stl_b_y) & LbFPMath_AngleMask) + LbFPMath_PI/4) >> 9) & 3;
            dist = max(abs(stl_b_x - stl2_x), abs(stl_b_y - stl2_y));
            dx = small_around[round_idx].delta_x;
            dy = small_around[round_idx].delta_y;
            if ((dist <= maxdist_b) && !is_valid_hug_subtile(stl_b_x + 3*dx, stl_b_y + 3*dy, plyr_idx))
            {
                stl_b_x += 3*dx;
                stl_b_y += 3*dy;
                state_b = 1;
                maxdist_b = max(abs(stl_b_x - stl2_x), abs(stl_b_y - stl2_y));
            } else
            if (state_b == 1)
            {
                state_b = 2;
            } else
            {
                int n;
                round_idx = (round_b + 1) & 3;
                for (n = 0; n < 4; n++)
                {
                    dx = small_around[round_idx].delta_x;
                    dy = small_around[round_idx].delta_y;
                    if (!is_valid_hug_subtile(stl_b_x + 3*dx, stl_b_y + 3*dy, plyr_idx))
                    {
                        round_b = round_idx;
                        stl_b_x += 3*dx;
                        stl_b_y += 3*dy;
                        break;
                    }
                    round_idx = (round_idx - 1) & 3;
                }
            }
            if ((stl_b_x == stl2_x) && (stl_b_y == stl2_y)) {
                return 0;
            }
        }
    }

    i = cdig->hug_side;
    if ((i == 0) || (i == 1)) {
        return i;
    }
    int dist_a, dist_b;
    dist_a = abs(stl_a_y - stl2_y) + abs(stl_a_x - stl1_x);
    dist_b = abs(stl_b_y - stl2_y) + abs(stl_b_x - stl1_x);
    if (dist_b > dist_a) {
        return 1;
    }
    if (dist_b < dist_a) {
        return 0;
    }
    return ACTION_RANDOM(2);
}

/**
 * Checks if given room kind is available for building by computer player.
 * @param comp Computer player.
 * @param rkind Room kind.
 * @return Gives IAvail_Never if the room isn't available, IAvail_Now if it's available and IAvail_Later if it's researchable.
 */
ItemAvailability computer_check_room_available(struct Computer2 * comp, long rkind)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    if ((rkind < 1) || (rkind >= ROOM_TYPES_COUNT)) {
        return IAvail_Never;
    }
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon");
        return IAvail_Never;
    }
    if (!dungeon->room_resrchable[rkind])
        return IAvail_Never;
    if (!dungeon->room_buildable[rkind])
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
        if (((slbattr->flags & SlbAtFlg_Blocking) == 0) && (slb->kind != SlbT_LAVA)) {
            return true;
        }
        if ((slbattr->flags & SlbAtFlg_IsRoom) != 0) {
            return true;
        }
    }
    return false;
}

struct ComputerTask * able_to_build_room(struct Computer2 *comp, struct Coord3d *pos, RoomKind rkind, long width_slabs, long height_slabs, long a6, long a7)
{
    return _DK_able_to_build_room(comp, pos, rkind, width_slabs, height_slabs, a6, a7);
}

long check_for_buildable(long stl_x, long stl_y, long plyr_idx)
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
    if ((slbattr->flags & SlbAtFlg_IsRoom) != 0) {
        return -1;
    }
    if (!slab_good_for_computer_dig_path(slb) || (slb->kind == SlbT_WATER)) {
        return -1;
    }
    stl_num = get_subtile_number(stl_slab_center_subtile(stl_x),stl_slab_center_subtile(stl_y));
    if (find_from_task_list(plyr_idx, stl_num) >= 0) {
        return -1;
    }
    if ((slbattr->flags & SlbAtFlg_Valuable) != 0) {
        return -1;
    }
    if (slab_kind_is_liquid(slb->kind)) {
        return 1;
    }
    if ( (slbattr->is_unknflg14 == 0) || (slb->kind == SlbT_GEMS) ) {
        return 1;
    }
    mapblk = get_map_block_at_pos(stl_num);
    return ((mapblk->flags & MapFlg_Unkn20) != 0) && (slabmap_owner(slb) != plyr_idx);
}

/** Retrieves index for small_around[] array which leads to the area closer to given destination.
 *  It uses a bit of randomness when angles are too straight, so it may happen that result for same points will vary.
 *
 * @param curr_x Current position x coord.
 * @param curr_y Current position y coord.
 * @param dest_x Destination position x coord.
 * @param dest_y Destination position y coord.
 * @return Index closer to destination.
 */
unsigned int small_around_index_towards_destination(long curr_x,long curr_y,long dest_x,long dest_y)
{
    long i,n;
    i = LbArcTanAngle(dest_x - curr_x, dest_y - curr_y);
    // Check the angle - we're a bit afraid of angles which are pi/4 multiplications
    if ((i & 0xFF) != 0)
    {
        // Just compute the index
        n = (i + LbFPMath_PI/4) >> 9;
    } else
    {
        //Special case - the angle is exact multiplication of pi/4
        // Add some variant factor to make it little off this value.
        // this should give better results because tangens values are rounded up or down.
        //TODO: maybe it would be even better to get previous around_index as parameter - this way we could avoid taking same path without random factors.
        n = (i + LbFPMath_PI/4 + ACTION_RANDOM(3) - 1) >> 9;
    }
    SYNCDBG(18,"Vector (%ld,%ld) returned ArcTan=%ld, around (%d,%d)",dest_x - curr_x, dest_y - curr_y,i,(int)small_around[n].delta_x,(int)small_around[n].delta_y);
    return n & 3;
}

short tool_dig_to_pos2_skip_slabs_which_dont_need_digging_f(struct Computer2 * comp, struct ComputerDig * cdig, unsigned short digflags,
    MapSubtlCoord *nextstl_x, MapSubtlCoord *nextstl_y, const char *func_name)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    MapSlabCoord nextslb_x,nextslb_y;
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
        (*nextstl_x) += 3 * small_around[around_index].delta_x;
        (*nextstl_y) += 3 * small_around[around_index].delta_y;
        if (i > map_tiles_x*map_tiles_y)
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
    MapSlabCoord nextslb_x,nextslb_y;
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
        if ( (slbattr->is_unknflg14 == 0) || (slb->kind == SlbT_GEMS)
          || (((mapblk->flags & MapFlg_Unkn20) != 0) && (slabmap_owner(slb) != dungeon->owner)) )
        {
            if ( ((slbattr->flags & SlbAtFlg_Valuable) == 0) || (digflags == 0) ) {
                break;
            }
        }
        if ( !simulation )
        {
            if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, *nextstl_x, *nextstl_y, 1, 1) <= Lb_OK) {
                break;
            }
            if (digflags != 0)
            {
                if ((slbattr->flags & SlbAtFlg_Valuable) != 0) {
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
    struct SlabAttr *slbattr;
    struct SlabMap *slb;
    struct Map *mapblk;
    MapSubtlCoord gldstl_x,gldstl_y;
    MapSubtlCoord digstl_x,digstl_y;
    MapSlabCoord digslb_x,digslb_y;
    long counter1;
    long i;
    SYNCDBG(14,"Starting");
    //return _DK_tool_dig_to_pos2(comp, cdig, simulation, digflags);
    dungeon = comp->dungeon;
    cdig->calls_count++;
    if (cdig->calls_count >= 356) {
        WARNLOG("%s: Player %d ComputerDig calls count (%d) exceeds limit",func_name,(int)dungeon->owner,(int)cdig->calls_count);
        return -2;
    }
    gldstl_x = 3 * subtile_slab(cdig->pos_begin.x.stl.num);
    gldstl_y = 3 * subtile_slab(cdig->pos_begin.y.stl.num);
    SYNCDBG(4,"Dig slabs from (%d,%d) to (%d,%d)",subtile_slab(gldstl_x),subtile_slab(gldstl_y),subtile_slab(cdig->pos_dest.x.stl.num),subtile_slab(cdig->pos_dest.y.stl.num));
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
            SYNCDBG(5,"Positions are equal at subtile (%d,%d)",(int)gldstl_x,(int)gldstl_y);
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
            SYNCDBG(5,"%s: Player %d digging to subtile (%d,%d) preparations failed",func_name,(int)dungeon->owner,(int)cdig->pos_next.x.stl.num,(int)cdig->pos_next.y.stl.num);
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
        cdig->direction_around = small_around_index_towards_destination(cdig->pos_next.x.stl.num,cdig->pos_next.y.stl.num,digstl_x,digstl_y);
        slbattr = get_slab_attrs(slb);
        if ((slbattr->is_unknflg14 != 0) && (slb->kind != SlbT_GEMS))
        {
            mapblk = get_map_block_at(digstl_x, digstl_y);
            if ( ((mapblk->flags & MapFlg_Unkn20) == 0) || (slabmap_owner(slb) == dungeon->owner) ) {
                SYNCDBG(5,"%s: Player %d cannot go through subtile (%d,%d)",func_name,(int)dungeon->owner,(int)digstl_x,(int)digstl_y);
                return -2;
            }
        }
        i = get_subtile_number(stl_slab_center_subtile(digstl_x),stl_slab_center_subtile(digstl_y));
        if ((find_from_task_list(dungeon->owner, i) < 0) && (!simulation))
        {
            if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, digstl_x, digstl_y, 1, 1) <= Lb_OK) {
                SYNCDBG(5,"%s: Player %d game action failed at subtile (%d,%d)",func_name,(int)dungeon->owner,(int)digstl_x,(int)digstl_y);
                return -2;
            }
        }
    } else
    {
        SYNCDBG(4,"%s: Player %d does long distance digging",func_name,(int)dungeon->owner);
        i = dig_to_position(dungeon->owner, gldstl_x, gldstl_y, cdig->direction_around, cdig->hug_side);
        if (i == -1) {
            SYNCDBG(5,"%s: Player %d digging to subtile (%d,%d) preparations failed",func_name,(int)dungeon->owner,(int)gldstl_x,(int)gldstl_y);
            return -2;
        }
        digstl_x = stl_num_decode_x(i);
        digstl_y = stl_num_decode_y(i);
        digslb_x = subtile_slab(digstl_x);
        digslb_y = subtile_slab(digstl_y);
        slb = get_slabmap_block(digslb_x, digslb_y);
        slbattr = get_slab_attrs(slb);
        if ((slbattr->is_unknflg14 != 0) && (slb->kind != SlbT_GEMS))
        {
            mapblk = get_map_block_at(digstl_x, digstl_y);
            if (((mapblk->flags & MapFlg_Unkn20) == 0) || (slabmap_owner(slb) == dungeon->owner))
            {
                i = get_subtile_number_at_slab_center(digslb_x,digslb_y);
                if ((find_from_task_list(dungeon->owner, i) < 0) && (!simulation))
                {
                    if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, digstl_x, digstl_y, 1, 1) <= Lb_OK) {
                        ERRORLOG("%s: Couldn't do game action - cannot dig",func_name);
                        return -2;
                    }
                }
            }
        }
        cdig->direction_around = small_around_index_towards_destination(cdig->pos_next.x.stl.num,cdig->pos_next.y.stl.num,digstl_x,digstl_y);
    }
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

long add_to_trap_location(struct Computer2 * comp, struct Coord3d * coord)
{
    struct Coord3d * location;
    MapSlabCoord slb_x, slb_y;
    long i;
    SYNCDBG(6,"Starting");
    //return _DK_add_to_trap_location(comp, coord);
    // Avoid duplicating entries
    slb_x = subtile_slab(coord->x.stl.num);
    slb_y = subtile_slab(coord->y.stl.num);
    for (i=0; i < COMPUTER_TRAP_LOC_COUNT; i++)
    {
        location = &comp->trap_locations[i];
        if ((subtile_slab(location->x.stl.num) == slb_x) && (subtile_slab(location->y.stl.num) == slb_y)) {
            return false;
        }
    }
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
    //return _DK_check_for_gold(l1, l2, l3);
    basestl_x = stl_slab_center_subtile(basestl_x);
    basestl_y = stl_slab_center_subtile(basestl_y);
    stl_num = get_subtile_number(basestl_x,basestl_y);
    slb = get_slabmap_for_subtile(basestl_x,basestl_y);
    slbattr = get_slab_attrs(slb);
    if ((slbattr->flags & SlbAtFlg_Valuable) != 0) {
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
    //return _DK_find_next_gold(comp, ctask);

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
        retval = tool_dig_to_pos2(comp, &cdig, 1, 0);
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
    //return _DK_task_dig_to_gold(comp,ctask);
    struct Dungeon* dungeon = comp->dungeon;

    i = dungeon->total_area * comp->field_1C / 100;
    if ((dungeon->digger_stack_length > 0) && (dungeon->digger_stack_length >= i))
    {
        SYNCDBG(6,"Player %d did nothing because digger stack length is over %d",(int)dungeon->owner,(int)i);
        return 0;
    }

    if (ctask->dig.valuable_slabs_tagged >= ctask->long_86)
    {
        struct SlabMap* slb = get_slabmap_for_subtile(ctask->dig.pos_next.x.stl.num, ctask->dig.pos_next.y.stl.num);

        if ((get_slab_attrs(slb)->flags & SlbAtFlg_Valuable) != 0)
        {
            ctask->field_60--;
            if (ctask->field_60 > 0) {
                SYNCDBG(6,"Player %d needs to dig %d more",(int)dungeon->owner,(int)ctask->field_60);
                return 0;
            }
        }
        ctask->dig.valuable_slabs_tagged = 0;
    }

    long retval = tool_dig_to_pos2(comp, &ctask->dig, 0, 1);

    if ((ctask->flags & ComTsk_Unkn0004) != 0)
    {
        set_flag_byte(&ctask->flags, ComTsk_Unkn0004, false);
        add_to_trap_location(comp, &ctask->dig.pos_next);
    }

    if (ctask->dig.valuable_slabs_tagged >= ctask->long_86)
    {
        ctask->field_60 = 700 / comp->field_18;
    }

    if (retval == -5)
    {
        ctask->ottype = ctask->ttype;
        ctask->ttype = CTT_WaitForBridge;

        SYNCDBG(6,"Player %d is waiting for bridge",(int)dungeon->owner);
        return 4;
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

    struct GoldLookup* gold_lookup = get_gold_lookup(ctask->gold_lookup_idx);

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
        gold_lookup = &game.gold_lookup[ctask->gold_lookup_idx];
        set_flag_byte(&gold_lookup->plyrfield_1[dungeon->owner], 0x02, true);
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
    return _DK_task_dig_to_attack(comp,ctask);
}

long task_magic_call_to_arms(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_magic_call_to_arms(comp,ctask);
}

struct Thing *find_creature_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, long a4)
{
    return _DK_find_creature_for_pickup(comp, pos, room, a4);
}

long task_pickup_for_attack(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    //return _DK_task_pickup_for_attack(comp,ctask);
    if (game.play_gameturn - ctask->field_A > 7500)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    }
    if (!xy_walkable(ctask->pos_76.x.stl.num, ctask->pos_76.y.stl.num, comp->dungeon->owner))
    {
        return CTaskRet_Unk4;
    }
    struct Thing *thing;
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing))
    {
        if (fake_dump_held_creatures_on_map(comp, thing, &ctask->pos_76)) {
            return CTaskRet_Unk2;
        }
        return CTaskRet_Unk4;
    }
    if (ctask->field_7C <= 0)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    ctask->field_7C--;
    thing = find_creature_for_pickup(comp, &ctask->pos_76, 0, 1);
    if (!thing_is_invalid(thing))
    {
        if (fake_place_thing_in_power_hand(comp, thing, &ctask->pos_76)) {
            return CTaskRet_Unk2;
        }
        return CTaskRet_Unk4;
    }
    return CTaskRet_Unk4;
}

TbBool get_drop_position_for_creature_job_in_room(struct Coord3d *pos, const struct Room *room, CreatureJob jobpref)
{
    if (!room_exists(room)) {
        return false;
    }
    // If the job can only be assigned by dropping creature at border - then drop at border
    if ((get_flags_for_job(jobpref) & (JoKF_AssignDropOnRoomBorder|JoKF_AssignDropOnRoomCenter)) == JoKF_AssignDropOnRoomBorder)
    {
        SYNCDBG(9,"Job %s requires dropping at %s border",creature_job_code_name(jobpref),room_code_name(room->kind));
        if (find_random_position_at_border_of_room(pos, room)) {
            SYNCDBG(9,"Will drop at border of %s on (%d,%d)",room_code_name(room->kind),(int)pos->x.stl.num,(int)pos->y.stl.num);
            return true;
        }
    }
    SYNCDBG(9,"Job %s has no %s area preference",creature_job_code_name(jobpref),room_code_name(room->kind));
    pos->x.val = subtile_coord(room->central_stl_x,ACTION_RANDOM(256));
    pos->y.val = subtile_coord(room->central_stl_y,ACTION_RANDOM(256));
    pos->z.val = subtile_coord(1,0);
    SYNCDBG(9,"Will drop at center of %s on (%d,%d)",room_code_name(room->kind),(int)pos->x.stl.num,(int)pos->y.stl.num);
    return true;
}

long task_move_creature_to_room(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Thing *thing;
    struct Room *room;
    struct Coord3d pos;
    long i;
    SYNCDBG(9,"Starting");
    //return _DK_task_move_creature_to_room(comp,ctask);
    room = INVALID_ROOM;
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing))
    {
        room = room_get(ctask->word_80);
        if (get_drop_position_for_creature_job_in_room(&pos,room,get_job_for_room(room->kind, true)))
        {
            if (fake_dump_held_creatures_on_map(comp, thing, &pos) > 0) {
                return CTaskRet_Unk2;
            }
        }
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
    }
    i = ctask->field_7C;
    ctask->field_7C--;
    if (i <= 0)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    thing = find_creature_to_be_placed_in_room(comp, &room);
    if (!thing_is_invalid(thing))
    {
        //TODO CREATURE_AI try to make sure the creature will do proper activity in the room
        //     ie. select a room tile which is far from CTA and enemies
        //TODO CREATURE_AI don't place creatures at center of a temple/portal if we don't want to get rid of them
        //TODO CREATURE_AI make sure to place creatures at "active" portal tile if we do want them to leave
        ctask->word_80 = room->index;
        if (get_drop_position_for_creature_job_in_room(&pos,room,get_job_for_room(room->kind, true)))
        {
            if (fake_place_thing_in_power_hand(comp, thing, &pos)) {
                return CTaskRet_Unk2;
            }
        }
        remove_task(comp, ctask);
        return CTaskRet_Unk0;
      }
      remove_task(comp, ctask);
      return CTaskRet_Unk0;
}

long task_move_creature_to_pos(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    SYNCDBG(9,"Starting");
    //return _DK_task_move_creature_to_pos(comp,ctask);
    dungeon = comp->dungeon;
    struct Thing *thing;
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing))
    {
        if (ctask->word_76 == comp->held_thing_idx)
        {
            if (fake_dump_held_creatures_on_map(comp, thing, &ctask->pos_86)) {
                remove_task(comp, ctask);
                return CTaskRet_Unk2;
            }
            remove_task(comp, ctask);
            return CTaskRet_Unk0;
        }
    }
    thing = thing_get(ctask->word_76);
    if (can_thing_be_picked_up_by_player(thing, dungeon->owner))
    {
        if (fake_place_thing_in_power_hand(comp, thing, &ctask->pos_86)) {
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
                    if (!creature_affected_by_spell(thing,SplK_Chicken))
                    {
                        if (!creature_is_doing_lair_activity(thing) && !creature_is_being_dropped(thing))
                        {
                            struct PerExpLevelValues *expvalues;
                            expvalues = &game.creature_scores[thing->model];
                            long expval, healthprm, new_factor;
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
    SYNCDBG(9,"Starting");
    struct Thing *thing;
    //return _DK_task_move_creatures_to_defend(comp,ctask);
    thing = thing_get(comp->held_thing_idx);
    if (!thing_is_invalid(thing))
    {
        if (!fake_dump_held_creatures_on_map(comp, thing, &ctask->pos_76))
        {
            remove_task(comp, ctask);
            return CTaskRet_Unk0;
        }
        return CTaskRet_Unk2;
    }
    if (game.play_gameturn - ctask->field_5C < ctask->field_60) {
        return CTaskRet_Unk4;
    }
    ctask->field_5C = game.play_gameturn;
    ctask->field_7C--;
    if (ctask->field_7C <= 0)
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    thing = find_creature_for_defend_pickup(comp);
    if (!thing_is_invalid(thing))
    {
        if (!fake_place_thing_in_power_hand(comp, thing, &ctask->pos_76) )
        {
            remove_task(comp, ctask);
            return CTaskRet_Unk0;
        }
        return CTaskRet_Unk2;
    }
    return CTaskRet_Unk2;
}

long task_slap_imps(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    SYNCDBG(9,"Starting");
    //return _DK_task_slap_imps(comp,ctask);
    dungeon = comp->dungeon;
    ctask->field_7C--;
    if (ctask->field_7C >= 0)
    {
        TbBool allow_slap_to_kill;
        // Make sure we can accept situation where the creature will die because of the slap
        allow_slap_to_kill = computer_able_to_use_magic(comp, PwrK_MKDIGGER, 0, 10);
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
            // Don't slap if picked up or affected by speed or already slapped
            if (!thing_is_picked_up(thing) && !thing_affected_by_spell(thing, SplK_Speed) && (cctrl->slap_turns == 0))
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
                        if (state_type == 1)
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
    //return _DK_task_dig_to_neutral(comp,ctask);
    int i;
    short digret;
    digret = tool_dig_to_pos2(comp, &ctask->dig, 0, 0);
    if (digret == -5)
    {
        i = ctask->ttype;
        ctask->ttype = CTT_WaitForBridge;
        ctask->ottype = i;
        return 4;
    }
    if ((digret >= -3) && (digret <= -1))
    {
        suspend_task_process(comp,ctask);
        return digret;
    }
    if ((ctask->flags & 0x04) != 0) {
        ctask->flags &= ~0x04;
        add_to_trap_location(comp, &ctask->dig.pos_next);
    }
    return digret;
}

long task_magic_speed_up(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    return _DK_task_magic_speed_up(comp,ctask);
}

long task_wait_for_bridge(struct Computer2 *comp, struct ComputerTask *ctask)
{
    SYNCDBG(9,"Starting");
    //return _DK_task_wait_for_bridge(comp,ctask);
    if (game.play_gameturn - ctask->field_A > 7500)
    {
        restart_task_process(comp, ctask);
        return 0;
    }
    PlayerNumber plyr_idx;
    MapSubtlCoord basestl_x,basestl_y;
    plyr_idx = comp->dungeon->owner;
    basestl_x = ctask->dig.pos_next.x.stl.num;
    basestl_y = ctask->dig.pos_next.y.stl.num;
    long n;
    for (n=0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        MapSlabCoord slb_x,slb_y;
        slb_x = subtile_slab_fast(basestl_x) + (long)small_around[n].delta_x;
        slb_y = subtile_slab_fast(basestl_y) + (long)small_around[n].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_block(slb_x, slb_y);
        if (slabmap_owner(slb) == plyr_idx)
        {
            if (try_game_action(comp, plyr_idx, GA_PlaceRoom, 0, basestl_x, basestl_y, 1, RoK_BRIDGE) > 0)
            {
                long i;
                i = ctask->ottype;
                ctask->ttype = i;
                if (i == 0) {
                    ERRORLOG("Bad set Task State");
                }
                return 1;
            }
        }
    }
    return 4;
}

long task_attack_magic(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    long i;
    SYNCDBG(9,"Starting");
    //return _DK_task_attack_magic(comp,ctask);
    dungeon = comp->dungeon;
    thing = thing_get(ctask->word_76);
    if (thing_is_invalid(thing)) {
        return CTaskRet_Unk1;
    }
    i = ctask->field_7C;
    ctask->field_7C--;
    if ((i <= 0) || creature_is_dying(thing) || creature_is_being_unconscious(thing)
     || creature_is_kept_in_custody(thing))
    {
        remove_task(comp, ctask);
        return CTaskRet_Unk1;
    }
    // Note that can_cast_spell() shouldn't be needed here - should
    // be checked in the function which adds the task
    if (computer_able_to_use_magic(comp, ctask->long_86, ctask->field_70, 1) != 1) {
        return CTaskRet_Unk4;
    }
    i = try_game_action(comp, dungeon->owner, ctask->long_80, ctask->field_70,
        thing->mappos.x.stl.num, thing->mappos.y.stl.num, ctask->word_76, 0);
    if (i <= Lb_OK)
        return CTaskRet_Unk4;
    return CTaskRet_Unk2;
}

long task_sell_traps_and_doors(struct Computer2 *comp, struct ComputerTask *ctask)
{
    struct Dungeon *dungeon;
    const struct TrapDoorSelling *tdsell;
    TbBool item_sold;
    long value,model;
    long i;
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid dungeon in computer player.");
        return 0;
    }
    SYNCDBG(19,"Starting for player %d",(int)dungeon->owner);
    //return _DK_task_sell_traps_and_doors(comp,ctask);
    if ((ctask->long_76 <= ctask->field_7C) && (dungeon->total_money_owned <= ctask->long_80))
    {
        i = 0;
        value = 0;
        item_sold = false;
        for (i=0; i < sizeof(trapdoor_sell)/sizeof(trapdoor_sell[0]); i++)
        {
            tdsell = &trapdoor_sell[ctask->long_86];
            switch (tdsell->category)
            {
            case TDSC_DoorCrate:
                model = tdsell->model;
                if ((model < 0) || (model >= DOOR_TYPES_COUNT)) {
                    ERRORLOG("Internal error - invalid door model %d in slot %d",(int)model,(int)i);
                    break;
                }
                if (dungeon->door_amount_placeable[model] > 0)
                {
                    if (remove_workshop_item_from_amount_stored(dungeon->owner, TCls_Door, model))
                    {
                        remove_workshop_item_from_amount_placeable(dungeon->owner, TCls_Door, model);
                        remove_workshop_object_from_player(dungeon->owner, door_to_object[model]);
                        item_sold = true;
                        value = game.doors_config[model].selling_value;
                        SYNCDBG(9,"Door %s crate sold for %d gold",door_code_name(model),(int)value);
                    } else
                    {
                        WARNLOG("Placeable door %s amount for player %d was incorrect; fixed",door_code_name(model),(int)dungeon->owner);
                        dungeon->door_amount_placeable[model] = 0;
                    }
                }
                break;
            case TDSC_TrapCrate:
                model = tdsell->model;
                if ((model < 0) || (model >= TRAP_TYPES_COUNT)) {
                    ERRORLOG("Internal error - invalid trap model %d in slot %d",(int)model,(int)i);
                    break;
                }
                if (dungeon->trap_amount_placeable[model] > 0)
                {
                    if (remove_workshop_item_from_amount_stored(dungeon->owner, TCls_Trap, model))
                    {
                        remove_workshop_item_from_amount_placeable(dungeon->owner, TCls_Trap, model);
                        remove_workshop_object_from_player(dungeon->owner, trap_to_object[model]);
                        item_sold = true;
                        value = game.traps_config[model].selling_value;
                        SYNCDBG(9,"Trap %s crate sold for %ld gold",trap_code_name(model),value);
                    } else
                    {
                        WARNLOG("Placeable trap %s amount for player %d was incorrect; fixed",trap_code_name(model),(int)dungeon->owner);
                        dungeon->trap_amount_placeable[model] = 0;
                    }
                }
                break;
            case TDSC_DoorPlaced:
                model = tdsell->model;
                if ((model < 0) || (model >= DOOR_TYPES_COUNT)) {
                    ERRORLOG("Internal error - invalid door model %d in slot %d",(int)model,(int)i);
                    break;
                }
                {
                    struct Thing *doortng;
                    doortng = get_random_door_of_model_owned_by_and_locked(model, dungeon->owner, false);
                    if (!thing_is_invalid(doortng)) {
                        MapSubtlCoord stl_x, stl_y;
                        item_sold = true;
                        stl_x = stl_slab_center_subtile(doortng->mappos.x.stl.num);
                        stl_y = stl_slab_center_subtile(doortng->mappos.y.stl.num);
                        value = game.doors_config[model].selling_value;
                        destroy_door(doortng);
                        if (is_my_player_number(dungeon->owner))
                            play_non_3d_sample(115);
                        dungeon->camera_deviate_jump = 192;
                        if (value != 0)
                        {
                            struct Coord3d pos;
                            set_coords_to_subtile_center(&pos,stl_x,stl_y,1);
                            create_price_effect(&pos, dungeon->owner, value);
                        } else
                        {
                            WARNLOG("Sold door at (%d,%d) which didn't cost anything",(int)stl_x,(int)stl_y);
                        }
                    }
                }
                break;
            case TDSC_TrapPlaced:
                model = tdsell->model;
                if ((model < 0) || (model >= TRAP_TYPES_COUNT)) {
                    ERRORLOG("Internal error - invalid trap model %d in slot %d",(int)model,(int)i);
                    break;
                }
                {
                    struct Thing *traptng;
                    traptng = get_random_trap_of_model_owned_by_and_armed(model, dungeon->owner, true);
                    if (!thing_is_invalid(traptng)) {
                        MapSubtlCoord stl_x, stl_y;
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
            ctask->long_86++;
            if (trapdoor_sell[ctask->long_86].category == TDSC_EndList)
                ctask->long_86 = 0;
            if (item_sold)
            {
                ctask->field_70--;
                if (ctask->field_70 > 0)
                {
                  ctask->long_76 += value;
                  dungeon->offmap_money_owned += value;
                  dungeon->total_money_owned += value;
                  return 1;
                }
                remove_task(comp, ctask);
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

TbBool create_task_move_creatures_to_defend(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num, unsigned long evflags)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    ctask->ttype = CTT_MoveCreaturesToDefend;
    ctask->pos_76.x.val = pos->x.val;
    ctask->pos_76.y.val = pos->y.val;
    ctask->pos_76.z.val = pos->z.val;
    ctask->field_7C = creatrs_num;
    ctask->field_70 = evflags;
    ctask->field_A = game.play_gameturn;
    ctask->field_5C = game.play_gameturn;
    ctask->field_60 = comp->field_34;
    return true;
}

TbBool create_task_move_creatures_to_room(struct Computer2 *comp, int room_idx, long creatrs_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    ctask->ttype = CTT_MoveCreatureToRoom;
    ctask->word_70 = room_idx;
    ctask->word_80 = room_idx;
    ctask->field_7C = creatrs_num;
    ctask->field_A = game.play_gameturn;
    return true;
}

TbBool create_task_pickup_for_attack(struct Computer2 *comp, struct Coord3d *pos, long par3, long creatrs_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    ctask->ttype = CTT_PickupForAttack;
    ctask->pos_76.x.val = pos->x.val;
    ctask->pos_76.y.val = pos->y.val;
    ctask->pos_76.z.val = pos->z.val;
    ctask->field_7C = creatrs_num;
    ctask->field_A = game.play_gameturn;
    ctask->long_86 = par3; // Originally only a word was set here
    return true;
}

TbBool create_task_magic_battle_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long par2, long creatrs_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    ctask->ttype = CTT_MagicCallToArms;
    ctask->field_1 = 0;
    ctask->pos_76.x.val = pos->x.val;
    ctask->pos_76.y.val = pos->y.val;
    ctask->pos_76.z.val = pos->z.val;
    ctask->field_7C = creatrs_num;
    ctask->field_A = game.play_gameturn;
    ctask->field_60 = 25;
    ctask->field_5C = game.play_gameturn - 25;
    ctask->field_8E = par2;
    return true;
}

TbBool create_task_magic_support_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long par2, long par3, long creatrs_num)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    ctask->ttype = CTT_MagicCallToArms;
    ctask->field_1 = 0;
    ctask->pos_76.x.val = pos->x.val;
    ctask->pos_76.y.val = pos->y.val;
    ctask->pos_76.z.val = pos->z.val;
    ctask->field_7C = creatrs_num;
    ctask->field_A = game.play_gameturn;
    ctask->field_60 = 25;
    ctask->field_5C = game.play_gameturn - 25;
    ctask->field_8E = par2;
    ctask->long_86 = par3; // Originally only a word was set here
    return true;
}

TbBool create_task_sell_traps_and_doors(struct Computer2 *comp, long par2, long value)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    ctask->ttype = CTT_SellTrapsAndDoors;
    ctask->field_70 = par2;
    ctask->field_A = game.play_gameturn;
    ctask->field_5C = game.play_gameturn;
    ctask->field_60 = 1;
    ctask->field_70 = 5;
    ctask->long_76 = 0;
    ctask->field_7C = value;
    ctask->long_80 = value;
    ctask->long_86 = 0;
    return true;
}

TbBool create_task_move_creature_to_pos(struct Computer2 *comp, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct ComputerTask *ctask;
    SYNCDBG(7,"Starting");
    ctask = get_free_task(comp, 0);
    if (computer_task_invalid(ctask)) {
        return false;
    }
    ctask->ttype = CTT_MoveCreatureToPos;
    ctask->word_86 = subtile_coord(stl_x,ACTION_RANDOM(256));
    ctask->word_88 = subtile_coord(stl_y,ACTION_RANDOM(256));
    ctask->word_76 = thing->index;
    ctask->word_80 = 0;
    ctask->field_A = game.play_gameturn;
    return true;
}

long process_tasks(struct Computer2 *comp)
{
    struct ComputerTask *ctask;
    long ndone;
    long i,n;
    unsigned long k;
    //return _DK_process_tasks(comp);
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
        if ((ctask->flags & 0x01) != 0)
        {
            n = ctask->ttype;
            if ((n > 0) && (n < sizeof(task_function)/sizeof(task_function[0])))
            {
                SYNCDBG(12,"Computer Task Type %d",(int)n);
                task_function[n].func(comp, ctask);
                ndone++;
            } else
            {
                ERRORLOG("Bad Computer Task Type %d",(int)n);
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
