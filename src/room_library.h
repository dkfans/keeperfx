/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_library.h
 *     Header file for room_library.c.
 * @par Purpose:
 *     Library room maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 05 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_LIBRARY_H
#define DK_ROOM_LIBRARY_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Room;
struct Thing;
struct ResearchVal;
struct Dungeon;

#pragma pack()
/******************************************************************************/
TbBool remove_spell_from_library(struct Room *room, struct Thing *spelltng, PlayerNumber new_owner);
struct Thing *create_spell_in_library(struct Room *room, ThingModel spkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

void init_dungeons_research(void);
TbBool research_needed(const struct ResearchVal *rsrchval, const struct Dungeon *dungeon);
TbBool add_research_to_player(PlayerNumber plyr_idx, long rtyp, long rkind, long amount);
TbBool add_research_to_all_players(long rtyp, long rkind, long amount);
TbBool remove_all_research_from_player(PlayerNumber plyr_idx);
TbBool clear_research_for_all_players(void);
TbBool research_overriden_for_player(PlayerNumber plyr_idx);
TbBool update_players_research_amount(PlayerNumber plyr_idx, long rtyp, long rkind, long amount);
TbBool update_or_add_players_research_amount(PlayerNumber plyr_idx, long rtyp, long rkind, long amount);
void process_player_research(PlayerNumber plyr_idx);

EventIndex update_library_object_pickup_event(struct Thing *creatng, struct Thing *picktng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
