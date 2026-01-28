/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_matchmaking.h
 *     Matchmaking server integration for multiplayer games.
 * @par Purpose:
 *     Register and discover multiplayer lobbies through a matchmaking server.
 * @author   KeeperFX Team
 * @date     24 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef NET_MATCHMAKING_H
#define NET_MATCHMAKING_H

#include "bflib_basics.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MATCHMAKING_MAX_LOBBIES 32
#define MATCHMAKING_LOBBY_NAME_LEN 64
#define MATCHMAKING_LOBBY_ID_LEN 64
#define MATCHMAKING_IP_LEN 64

struct MatchmakingLobby {
    char id[MATCHMAKING_LOBBY_ID_LEN];
    char name[MATCHMAKING_LOBBY_NAME_LEN];
    char ip[MATCHMAKING_IP_LEN];
    uint16_t port;
};

TbBool matchmaking_register_lobby(const char *name, uint16_t port);
void matchmaking_unregister_lobby(void);
void matchmaking_ping_lobby(void);
int matchmaking_list_lobbies(struct MatchmakingLobby *lobbies, int max_lobbies);
TbBool matchmaking_is_registered(void);

#ifdef __cplusplus
}
#endif

#endif
