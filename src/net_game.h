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
#include "user_prefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKETS_COUNT           9

/******************************************************************************/
#pragma pack(1)

struct TbNetworkSessionNameEntry;
struct PlayerInfo;

/******************************************************************************/
extern struct TbNetworkUserInfo net_user_info[MAX_NET_USERS];

#pragma pack()
/******************************************************************************/
short setup_network_service(enum FrontendNetService service);
int setup_old_network_service(void);
TbBool init_players_network_game(void);
void setup_count_players(void);

long network_session_join(void);

TbBool network_user_active(NetUserId);
const char *network_user_name(NetUserId);
TbBool network_human_contenders_remain(void);
void resolve_placeholders(void);
void process_player_leave_game_packet(struct PlayerInfo *player);
void process_disconnected_network_players(void);
TbBool user_present(NetUserId user);
void host_spoof_dropped_user_packets(void);
void sync_initial_network_seed(void);
void build_local_user_preferences(UserPreferences prefs);
void apply_user_preferences(NetUserId user, const UserPreferences prefs, unsigned char flags);
void apply_local_user_preferences(NetUserId user, unsigned char flags);
void apply_user_start_tendencies(struct PlayerInfo *player);
TbBool user_cheats_allowed(NetUserId user);
TbBool game_censorship_enabled(void);
void apply_recorded_network_stop(void);
TbBool get_startup_user_preferences(NetUserId user, UserPreferences prefs);
TbBool network_is_host(void);
PlayerNumber get_net_user_player_number(NetUserId user);
void set_net_user_player_number(NetUserId user, PlayerNumber plyr_idx);
void remap_user_to_solo(struct PlayerInfo *myplyr);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
