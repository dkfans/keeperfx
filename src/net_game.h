/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_game.h
 *     Header file for net_game.c.
 * @par Purpose:
 *     Network game support for Dungeon Keeper.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NETGAME_H
#define DK_NETGAME_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_coroutine.h"
#include "bflib_network.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NET_PLAYERS_COUNT       4
#define NET_SERVICES_COUNT     16
#define NET_SERVICE_LEN        64
#define PACKETS_COUNT           5

/******************************************************************************/
#pragma pack(1)

struct TbNetworkSessionNameEntry;

/******************************************************************************/
extern struct TbNetworkPlayerInfo net_player_info[NET_PLAYERS_COUNT];
extern struct TbNetworkSessionNameEntry *net_session[32];
extern long net_number_of_sessions;
extern long net_session_index_active;
extern struct TbNetworkPlayerName net_player[NET_PLAYERS_COUNT];
extern struct ConfigInfo net_config_info;
extern char net_service[16][NET_SERVICE_LEN];
extern char net_player_name[20];

#pragma pack()
/******************************************************************************/
extern char net_current_message[64];
extern long net_current_message_index;
/******************************************************************************/
short setup_network_service(int srvidx);
int setup_old_network_service(void);
void init_players_network_game(CoroutineLoop *context);
void setup_count_players(void);

long network_session_join(void);

TbBool network_player_active(int plyr_idx);
const char *network_player_name(int plyr_idx);
void set_network_player_name(int plyr_idx, const char *name);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
