/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dungeon_data.c
 *     Dungeon data structures definitions.
 * @par Purpose:
 *     Defines functions for dungeon-related structures support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "dungeon_data.h"

#include "globals.h"
#include "bflib_basics.h"
#include "keeperfx.hpp"

/******************************************************************************/
/******************************************************************************/
struct Dungeon bad_dungeon;
/******************************************************************************/
struct Dungeon *get_players_num_dungeon(long plyr_idx)
{
    struct PlayerInfo *player;
    PlayerNumber plyr_num;
    player = get_player(plyr_idx);
    plyr_num = player->index;
    if ((plyr_num >= 0) && (plyr_num < DUNGEONS_COUNT))
        return &(game.dungeon[plyr_num]);
    return INVALID_DUNGEON;
}

struct Dungeon *get_dungeon(long dngn_idx)
{
    if ((dngn_idx >= 0) && (dngn_idx < DUNGEONS_COUNT))
        return &(game.dungeon[dngn_idx]);
    return INVALID_DUNGEON;
}

TbBool dungeon_invalid(struct Dungeon *dungeon)
{
    if (dungeon == INVALID_DUNGEON)
        return false;
    return (dungeon >= &game.dungeon[0]);
}
/******************************************************************************/
