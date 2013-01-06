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
long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
    long creatrs_def, creatrs_num;
    struct Coord3d pos;
    //return _DK_computer_event_battle(comp, cevent, event);
    pos.x.stl.num = event->mappos_x;
    pos.x.stl.pos = 0;
    pos.y.stl.num = event->mappos_y;
    pos.y.stl.pos = 0;
    pos.z.val = 0;
    if ((pos.x.val <= 0) || (pos.y.val <= 0))
        return false;
    creatrs_def = count_creatures_for_defend_pickup(comp);
    creatrs_num = creatrs_def * (long)cevent->param1 / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0))
        creatrs_num = 1;
    if (creatrs_num <= 0)
        return false;
    if (!computer_find_non_solid_block(comp, &pos))
        return false;
    if (!get_task_in_progress(comp, CTT_MoveCreaturesToDefend) || ((cevent->param2 & 0x02) != 0))
    {
        return create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->param2);
    } else
    if (computer_able_to_use_magic(comp, 6, 1, 1) == 1)
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

long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_battle_test(comp, cevent);
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
  return _DK_computer_event_check_rooms_full(comp, cevent);
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
