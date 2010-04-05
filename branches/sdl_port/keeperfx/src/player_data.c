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
long neutral_player_number = NEUTRAL_PLAYER;
long hero_player_number = HERO_PLAYER;
struct PlayerInfo bad_player;
/******************************************************************************/
struct PlayerInfo *get_player_ptr(long plyr_idx,const char *func_name)
{
    if ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT))
        return &game.players[plyr_idx];
    ERRORMSG("%s: Tried to get nonexisting player %ld!",func_name,plyr_idx);
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
    return (plyr_num == myplyr->id_number);
}

TbBool player_invalid(struct PlayerInfo *player)
{
    if (player == INVALID_PLAYER)
        return true;
    return (player < &game.players[0]);
}

TbBool player_allied_with(struct PlayerInfo *player, long ally_idx)
{
    if ((ally_idx < 0) || (ally_idx >= PLAYERS_COUNT))
    {
        WARNLOG("Tried to get nonexisting player!");
        return false;
    }
    return ((player->allied_players & (1<<ally_idx)) != 0);
}

void clear_players(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = &game.players[i];
        memset(player, 0, sizeof(struct PlayerInfo));
        player->id_number = PLAYERS_COUNT;
    }
    memset(&bad_player, 0, sizeof(struct PlayerInfo));
    bad_player.id_number = PLAYERS_COUNT;
    game.field_14E496 = hero_player_number;
    game.field_14E495 = 0;
    game.flagfield_14EA4A = 2;
}

/******************************************************************************/
