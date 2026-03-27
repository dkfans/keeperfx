/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_matchmaking.h
 *     Matchmaking client for the KeeperFX lobby server.
 * @par Purpose:
 *     Connects to the WebSocket-based matchmaking server to list, host, and
 *     join game sessions.  Provides the hole-punch relay flow required for
 *     NAT traversal when direct port-forwarding is unavailable.
 * @author   KeeperFX Team
 * @date     06 Mar 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef NET_MATCHMAKING_H
#define NET_MATCHMAKING_H

#include "bflib_netsession.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MATCHMAKING_URL    "wss://matchmaking.keeperfx.workers.dev/ws"
#define MATCHMAKING_IP_URL "https://matchmaking.keeperfx.workers.dev/ip"
#define MATCHMAKING_ID_MAX 64
#define MATCHMAKING_IP_MAX 64
#define MATCHMAKING_NAME_MAX SESSION_NAME_MAX_LEN
#define MATCHMAKING_SESSIONS_MAX 32

typedef struct {
    char ipv4[MATCHMAKING_IP_MAX];
    char ipv6[MATCHMAKING_IP_MAX];
    int ipv4_port;
    int ipv6_port;
} PunchAddresses;

extern struct TbNetworkSessionNameEntry matchmaking_sessions[MATCHMAKING_SESSIONS_MAX];
extern int matchmaking_session_count;
extern char join_lobby_id[MATCHMAKING_ID_MAX];

void matchmaking_connect_async(void);
int matchmaking_connect(void);
void matchmaking_disconnect(void);
void matchmaking_refresh_sessions(void);
int matchmaking_create(const char *name, int udp_ipv4_port, int udp_ipv6_port);
int matchmaking_punch(const char *lobby_id, int udp_ipv4_port, int udp_ipv6_port, PunchAddresses *output);
int matchmaking_poll_punch(PunchAddresses *output);

#ifdef __cplusplus
}
#endif

#endif
