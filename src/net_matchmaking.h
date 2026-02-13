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
#include "bflib_netsession.h"
#include <stdint.h>

#define MATCHMAKING_FETCH_INTERVAL_MS 5000
#define MATCHMAKING_PING_INTERVAL_MS 30000

#ifdef __cplusplus
extern "C" {
#endif

TbBool matchmaking_register_lobby(const char *name, uint16_t port);
void matchmaking_unregister_lobby(void);
void matchmaking_ping_lobby(void);
int matchmaking_fetch_lobbies(struct TbNetworkSessionNameEntry *sessions, int max_sessions);

#ifdef __cplusplus
}
#endif

#endif
