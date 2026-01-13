/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file bflib_network_exchange.h
 *     Header file for bflib_network_exchange.cpp.
 * @par Purpose:
 *     Network data exchange for Dungeon Keeper multiplayer.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     11 Apr 2009 - 13 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_EXCHANGE_H
#define DK_NET_EXCHANGE_H

#include "bflib_basics.h"
#include "bflib_network.h"

#ifdef __cplusplus
extern "C" {
#endif

TbError LbNetwork_Exchange(enum NetMessageType msg_type, void *send_buf, void *server_buf, size_t buf_size);
TbError LbNetwork_ExchangeLogin(char *plyr_name);
void LbNetwork_WaitForMissingPackets(void* server_buf, size_t client_frame_size);
void LbNetwork_SendPauseImmediate(TbBool pause_state, unsigned long delay_milliseconds);
void LbNetwork_SendChatMessageImmediate(int player_id, const char *message);

#ifdef __cplusplus
}
#endif

#endif
