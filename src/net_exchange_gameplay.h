/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_gameplay.h
 *     Header file for net_exchange_gameplay.c.
 * @par Purpose:
 *     Gameplay packet exchange for Dungeon Keeper multiplayer.
 * @par Comment:
 *     Public declarations for multiplayer gameplay exchange routines.
 * @author   KeeperFX Team
 * @date     09 May 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_EXCHANGE_GAMEPLAY_H
#define DK_NET_EXCHANGE_GAMEPLAY_H

#include "bflib_basics.h"
#include "globals.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

void initialize_packet_history(void);
void store_packet_history(PlayerNumber player, const struct Packet *packet);
const struct Packet *get_history_packet(PlayerNumber player, GameTurn turn);
TbBool read_repair_packet_history(NetUserId source, const char *buffer, size_t buffer_size);
TbError LbNetwork_ExchangeGameplay(void *send_buf, void *server_buf, size_t frame_size);
void LbNetwork_BroadcastUnpause(void);
TbError process_network_unpause_message(void);
TbError process_network_turn_sync_message(NetUserId source, const char *buffer, size_t buffer_size);
void process_gameplay_chat_message(int player_id, const char *message);

#ifdef __cplusplus
}
#endif

#endif
