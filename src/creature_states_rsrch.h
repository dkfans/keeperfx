/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_rsrch.h
 *     Header file for creature_states_rsrch.c.
 * @par Purpose:
 *     Creature state machine functions for their job in various rooms.
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
#ifndef DK_CRTRSTATERSRCH_H
#define DK_CRTRSTATERSRCH_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Dungeon;

#pragma pack()
/******************************************************************************/
TbBool creature_can_do_research(const struct Thing *creatng);
short at_research_room(struct Thing *thing);
CrCheckRet process_research_function(struct Thing *thing);
short researching(struct Thing *thing);
TbBool force_complete_current_research(PlayerNumber plyr_idx);
int get_next_research_item(const struct Dungeon *dungeon);
TbBool has_new_rooms_to_research(const struct Dungeon *dungeon);

struct ResearchVal *get_players_current_research_val(PlayerNumber plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
