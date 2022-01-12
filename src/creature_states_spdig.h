/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_spdig.h
 *     Header file for creature_states_spdig.c.
 * @par Purpose:
 *     Creature state machine functions for special diggers (imps).
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSTATESPDIG_H
#define DK_CRTRSTATESPDIG_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;
#pragma pack()
/******************************************************************************/
short imp_arrives_at_convert_dungeon(struct Thing *thing);
short imp_arrives_at_dig_or_mine(struct Thing *thing);
short imp_arrives_at_improve_dungeon(struct Thing *thing);
short imp_arrives_at_reinforce(struct Thing *thing);
short imp_birth(struct Thing *thing);
short imp_converts_dungeon(struct Thing *thing);
short imp_digs_mines(struct Thing *thing);
short imp_doing_nothing(struct Thing *thing);
short imp_drops_gold(struct Thing *thing);
short imp_improves_dungeon(struct Thing *thing);
short imp_last_did_job(struct Thing *thing);
short imp_picks_up_gold_pile(struct Thing *thing);
short imp_reinforces(struct Thing *thing);
short imp_toking(struct Thing *thing);
short creature_pick_up_unconscious_body(struct Thing *thing);
short creature_picks_up_corpse(struct Thing *thing);
short creature_picks_up_spell_object(struct Thing *thing);
short creature_picks_up_crate_for_workshop(struct Thing *thing);
short creature_picks_up_trap_object(struct Thing *thing);
short creature_drops_corpse_in_graveyard(struct Thing *thing);
short creature_drops_crate_in_workshop(struct Thing *thing);
short creature_drops_spell_object_in_library(struct Thing *thing);
short creature_arms_trap(struct Thing *thing);
short creature_going_to_safety_for_toking(struct Thing *thing);
long check_out_available_spdigger_drop_tasks(struct Thing *digger);
TbBool creature_is_dragging_or_being_dragged(const struct Thing *thing);
TbBool creature_drop_thing_to_another_room(struct Thing* thing, struct Room* skiproom, signed char rkind);
TbBool set_creature_being_dragged_by(struct Thing *dragtng, struct Thing *thing);
short creature_arms_trap_first_person(struct Thing *creatng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
