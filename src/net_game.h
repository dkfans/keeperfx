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
DLLIMPORT struct TbNetworkPlayerInfo _DK_net_player_info[NET_PLAYERS_COUNT];
#define net_player_info _DK_net_player_info
DLLIMPORT struct TbNetworkSessionNameEntry *_DK_net_session[32];
#define net_session _DK_net_session
DLLIMPORT long _DK_net_number_of_sessions;
#define net_number_of_sessions _DK_net_number_of_sessions
DLLIMPORT long _DK_net_session_index_active;
#define net_session_index_active _DK_net_session_index_active
DLLIMPORT struct TbNetworkPlayerName _DK_net_player[NET_PLAYERS_COUNT];
#define net_player _DK_net_player
DLLIMPORT struct ConfigInfo _DK_net_config_info;
#define net_config_info _DK_net_config_info
DLLIMPORT char _DK_net_service[16][NET_SERVICE_LEN];
#define net_service _DK_net_service
DLLIMPORT char _DK_net_player_name[20];
#define net_player_name _DK_net_player_name
DLLIMPORT struct ServiceInitData _DK_net_serial_data;
#define net_serial_data _DK_net_serial_data
DLLIMPORT struct ServiceInitData _DK_net_modem_data;
#define net_modem_data _DK_net_modem_data
DLLIMPORT struct TbModemDev _DK_modem_dev;
#define modem_dev _DK_modem_dev

#pragma pack()
/******************************************************************************/
extern long number_of_comports;
extern long number_of_speeds;
extern long net_comport_scroll_offset;
extern long net_speed_scroll_offset;
extern char tmp_net_irq[8];
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
