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
#include "front_network.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKETS_COUNT           9

/******************************************************************************/
#pragma pack(1)

struct TbNetworkSessionNameEntry;
struct PlayerInfo;

/******************************************************************************/
extern struct TbNetworkPlayerInfo net_player_info[MAX_NET_USERS];

#pragma pack()
/******************************************************************************/
short setup_network_service(enum FrontendNetService service);
int setup_old_network_service(void);
TbBool init_players_network_game(void);
void setup_count_players(void);
void are_disconnect_victories_allowed(void);

int32_t network_session_join(void);

TbBool network_player_active(int plyr_idx);
const char *network_player_name(int plyr_idx);
TbBool network_human_contenders_remain(void);
void process_player_leave_game_packet(struct PlayerInfo *player);
void process_disconnected_network_players(void);
void sync_initial_network_seed(void);
uint32_t get_host_player_id(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
