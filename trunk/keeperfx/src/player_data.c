/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_data.c
 *     Player data structures definitions.
 * @par Purpose:
 *     Defines functions for player-related structures support.
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
#include "player_data.h"

#include "globals.h"
#include "bflib_basics.h"
#include "keeperfx.hpp"

/******************************************************************************/
/******************************************************************************/
struct PlayerInfo bad_player;
/******************************************************************************/
struct PlayerInfo *get_player(long plyr_idx)
{
    if ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT))
        return &game.players[plyr_idx];
    WARNLOG("Tried to get nonexisting player!");
    return INVALID_PLAYER;
}

TbBool is_my_player(struct PlayerInfo *player)
{
    struct PlayerInfo *myplyr;
    myplyr = &game.players[my_player_number%PLAYERS_COUNT];
    return (player == myplyr);
}

TbBool is_my_player_number(PlayerNumber plyr_num)
{
    struct PlayerInfo *myplyr;
    myplyr = &game.players[my_player_number%PLAYERS_COUNT];
    return (plyr_num == myplyr->index);
}

TbBool player_invalid(struct PlayerInfo *player)
{
    if (player == INVALID_PLAYER)
        return false;
    return (player >= &game.players[0]);
}
/******************************************************************************/
