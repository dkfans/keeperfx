/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_lobby.h
 *     Header file for net_lobby.c.
 * @par Purpose:
 *     Public declarations for multiplayer lobby and frontend exchange routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     09 May 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_LOBBY_H
#define DK_NET_LOBBY_H

#include "bflib_netsession.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t network_lobby_ping;

TbError LbNetwork_ExchangeLogin(char *player_name);
TbError LbNetwork_ExchangeFrontend(void *send_buf, void *server_buf, size_t frame_size);
TbError process_login_message(NetUserId source, char *read_pos);
TbError process_user_update_message(NetUserId source, char *read_pos, const char *end_pos);

void LbNetwork_SetServerPort(int port);
void LbNetwork_InitSessionsFromCmdLine(const char *str);
TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *playr_name, int32_t *playr_num, void *optns);
TbError LbNetwork_Create(char *nsname_str, char *plyr_name, uint32_t *plyr_num, void *optns);
TbError LbNetwork_EnableNewPlayers(TbBool allow);
TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *user_data);
TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr);
TbError LbNetwork_Stop(void);

#ifdef __cplusplus
}
#endif

#endif
