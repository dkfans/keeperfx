/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_steam.h
 *     Header file for net_steam.cpp.
 * @par Purpose:
 *     Network service provider that carries the game over Steam (lobbies for finding
 *     each other, ISteamNetworkingMessages for the traffic, Steam relays the packets).
 * @par Comment:
 *     Uses the Steamworks flat C API resolved at runtime from steam_api.dll, no SDK is needed to build.
 *     Intended for the Spacewar test app (AppID 480).
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_STEAM_H
#define DK_NET_STEAM_H

#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/** -steamhost: host the game through a Steam lobby instead of a UDP port. */
extern TbBool steam_host_requested;
/** Lobby id to join on startup (-steamjoin <id>, or +connect_lobby <id> from Steam). Empty when none. */
extern char steam_join_lobby[32];
/** -steamjoin / -steamhost given: the frontend polls Steam for invites while the main menu is shown. */
extern TbBool steam_wait_for_invite;

struct NetSP *InitSteamSP(void);

/** Text describing why Steam networking is not available (valid after a failed init). */
const char *steam_net_last_error(void);
/** The error from a failed background init, once (NULL when there is none or it was already taken). */
const char *steam_net_take_error(void);
/** Lobby id of the hosted game as text, or "" when there is none. */
const char *steam_net_lobby_text(void);

/**
 * Called every frame by the main menu when waiting for an invite.
 * @param lobby_out receives the lobby id of a join request (invite accepted / Join Game clicked), if one arrived
 * @return true when a join request was returned
 */
TbBool steam_net_poll_join_request(char *lobby_out, size_t lobby_out_size);

/** Parses a connect string such as "+connect_lobby 1234" or "1234" into a decimal lobby id. */
TbBool steam_net_parse_lobby(const char *text, char *lobby_out, size_t lobby_out_size);

#ifdef __cplusplus
}
#endif

#endif
