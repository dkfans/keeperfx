/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_lobby.h
 *     Header file for net_lobby.c.
 * @par Purpose:
 *     Multiplayer lobby and session lifecycle routines.
 */
/******************************************************************************/
#ifndef DK_NET_LOBBY_H
#define DK_NET_LOBBY_H

#include "bflib_netsession.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

TbError process_login_message(NetUserId source, char *read_pos);
TbError process_user_update_message(NetUserId source, char *read_pos);
TbError LbNetwork_ExchangeLogin(char *plyr_name);

void LbNetwork_SetServerPort(int port);
void LbNetwork_InitSessionsFromCmdLine(const char *str);
TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *playr_name, int32_t *playr_num, void *optns);
TbError LbNetwork_Create(char *nsname_str, char *plyr_name, uint32_t *plyr_num, void *optns);
TbError LbNetwork_EnableNewPlayers(TbBool allow);
TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *user_data);
TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr);
TbError LbNetwork_Stop(void);
void LbNetwork_SendChatMessageImmediate(int player_id, const char *message);

/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
