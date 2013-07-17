/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_compevents.c
 *     Computer player events definitions and routines.
 * @par Purpose:
 *     Defines a computer player events and related functions.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 06 Jan 2013
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
#include "bflib_math.h"

#include "config.h"
#include "player_instances.h"
#include "config_terrain.h"
#include "creature_states_combt.h"

#include "dungeon_data.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
DLLIMPORT long _DK_computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
DLLIMPORT long _DK_computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);

/******************************************************************************/
/******************************************************************************/
long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_breach(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event);

/******************************************************************************/
const struct NamedCommand computer_event_test_func_type[] = {
  {"event_battle_test",       1,},
  {"event_check_fighters",    2,},
  {"event_attack_magic_foe",  3,},
  {"event_check_rooms_full",  4,},
  {"event_check_imps_danger", 5,},
  {"none",                    6,},
  {NULL,                      0,},
};

Comp_EvntTest_Func computer_event_test_func_list[] = {
  NULL,
  computer_event_battle_test,
  computer_event_check_fighters,
  computer_event_attack_magic_foe,
  computer_event_check_rooms_full,
  computer_event_check_imps_in_danger,
  NULL,
  NULL,
};

const struct NamedCommand computer_event_func_type[] = {
  {"event_battle",            1,},
  {"event_find_link",         2,},
  {"event_check_payday",      3,},
  {"none",                    4,},
  {NULL,                      0,},
};

Comp_Event_Func computer_event_func_list[] = {
  NULL,
  computer_event_battle,
  computer_event_find_link,
  computer_event_check_payday,
  NULL,
  NULL,
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event)
{
    //return _DK_computer_event_battle(comp, cevent, event);
    struct Coord3d pos;
    pos.x.stl.num = event->mappos_x;
    pos.x.stl.pos = 0;
    pos.y.stl.num = event->mappos_y;
    pos.y.stl.pos = 0;
    pos.z.val = 0;
    if ((pos.x.val <= 0) || (pos.y.val <= 0))
        return false;
    long creatrs_def, creatrs_num;
    creatrs_def = count_creatures_for_defend_pickup(comp);
    creatrs_num = creatrs_def * (long)cevent->param1 / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0)) {
        creatrs_num = 1;
    }
    if (creatrs_num <= 0) {
        return false;
    }
    if (!computer_find_non_solid_block(comp, &pos)) {
        return false;
    }
    if (!get_task_in_progress(comp, CTT_MoveCreaturesToDefend) || ((cevent->param2 & 0x02) != 0))
    {
        return create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->param2);
    } else
    if (computer_able_to_use_magic(comp, PwrK_CALL2ARMS, 1, 1) == 1)
    {
        if (!get_task_in_progress(comp, CTT_MagicCallToArms) || ((cevent->param2 & 0x02) != 0))
        {
            if ( check_call_to_arms(comp) )
            {
                return create_task_magic_call_to_arms(comp, &pos, creatrs_num);
            }
        }
    }
    return false;
}

long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
  return _DK_computer_event_find_link(comp, cevent, event);
}

/**
 * Finds computer player creature which is currently in a fight.
 * @param comp
 * @return
 */
struct Thing *find_creature_in_fight_with_enemy(struct Computer2 *comp)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *creatng;
    unsigned long k;
    int i;
    dungeon = comp->dungeon;
    // Force leave or kill normal creatures
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        creatng = thing_get(i);
        cctrl = creature_control_get_from_thing(creatng);
        if (thing_is_invalid(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->combat_flags != 0)
        {
            if (creature_is_being_attacked_by_enemy_player(creatng)) {
                return creatng;
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
    // Kill all special workers
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        creatng = thing_get(i);
        cctrl = creature_control_get_from_thing(creatng);
        if (thing_is_invalid(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->combat_flags != 0) {
            if (creature_is_being_attacked_by_enemy_player(creatng)) {
                return creatng;
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
    return INVALID_THING;
}

long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    //return _DK_computer_event_battle_test(comp, cevent);
    if (comp->dungeon->fights_num <= 0) {
        return 4;
    }
    struct Thing *creatng;
    creatng = find_creature_in_fight_with_enemy(comp);
    if (thing_is_invalid(creatng)) {
        return 4;
    }
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    long creatrs_def, creatrs_num;
    creatrs_def = count_creatures_for_defend_pickup(comp);
    creatrs_num = creatrs_def * (long)cevent->param1 / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0)) {
        creatrs_num = 1;
    }
    if (creatrs_num <= 0) {
        return 4;
    }
    if (!computer_find_non_solid_block(comp, &pos)) {
        return 4;
    }
    if (!get_task_in_progress(comp, CTT_MoveCreaturesToDefend))
    {
        if (!create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->param2)) {
            return 4;
        }
        return 1;
    }
    if (computer_able_to_use_magic(comp, PwrK_CALL2ARMS, 8, 1) == 1)
    {
        if (!get_task_in_progress(comp, CTT_MagicCallToArms))
        {
            if (check_call_to_arms(comp))
            {
                if (!create_task_magic_call_to_arms(comp, &pos, creatrs_num)) {
                    return 4;
                }
                return 1;
            }
        }
    }
    return 4;
}

long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_fighters(comp, cevent);
}

long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_attack_magic_foe(comp, cevent);
}

long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    long ret;
    SYNCDBG(18,"Starting");
    //return _DK_computer_event_check_rooms_full(comp, cevent);
    ret = 4;
    struct ValidRooms *bldroom;
    for (bldroom = valid_rooms_to_build; bldroom->rkind > 0; bldroom++)
    {
        if (computer_get_room_kind_free_capacity(comp, bldroom->rkind) > 0) {
            continue;
        }
        // Find the corresponding build process and mark it as needed
        long i;
        for (i=0; i <= COMPUTER_PROCESSES_COUNT; i++)
        {
            struct ComputerProcess *cproc;
            cproc = &comp->processes[i];
            if ((cproc->flags & ComProc_Unkn0002) != 0)
                break;
            if (cproc->parent == bldroom->process)
            {
                SYNCDBG(8,"Need \"%s\"",room_code_name(bldroom->rkind));
                ret = 1;
                cproc->flags &= ~ComProc_Unkn0008;
                cproc->flags &= ~ComProc_Unkn0001;
                cproc->field_3C = 0;
                cproc->field_38 = 0;
            }
        }
    }
    return ret;
}

long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_imps_in_danger(comp, cevent);
}

long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
  return _DK_computer_event_check_payday(comp, cevent, event);
}

long computer_event_breach(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event)
{
    //TODO COMPUTER_EVENT_BREACH is remade from beta; make it work (if it's really needed)
    struct ComputerTask *ctask;
    struct Coord3d pos;
    long i,count;

    //TODO COMPUTER_EVENT_BREACH check why mappos_x and mappos_y isn't used normally
    pos.x.val = ((event->mappos_x & 0xFF) << 8);
    pos.y.val = (((event->mappos_x >> 8) & 0xFF) << 8);
    if ((pos.x.val <= 0) || (pos.y.val <= 0))
    {
        return 0;
    }
    count = count_creatures_for_pickup(comp, &pos, 0, cevent->param2);
    i = count * cevent->param1 / 100;
    if ((i <= 0) && (count > 0))
    {
        i = 1;
    }
    if (i <= 0)
    {
        return 4;
    }
    if (!computer_find_non_solid_block(comp, &pos))
    {
        return 4;
    }
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask))
    {
        return 4;
    }
    ctask->ttype = CTT_MoveCreaturesToDefend;
    ctask->pos_76.x.val = pos.x.val;
    ctask->pos_76.y.val = pos.y.val;
    ctask->pos_76.z.val = pos.z.val;
    ctask->field_7C = i;
    ctask->field_70 = cevent->param2;
    ctask->field_A = game.play_gameturn;
    ctask->field_5C = game.play_gameturn;
    ctask->field_60 = comp->field_34;
    return 1;
}

/******************************************************************************/
