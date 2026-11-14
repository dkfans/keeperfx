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
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int32_t multiplayer_speed_adjustment_ns;

// PACKET_HISTORY_SIZE affects how far apart two users' turns can be (if they divert too far then there's no easy recovering). It should be set as high as possible while still being safe to send, so less than 1300 bytes at once.
#define PACKET_HISTORY_SIZE 40
#define GAMEPLAY_CHAT_QUEUE_LEN 16

struct QueuedGameplayChat {
    NetUserId user;
    MapCoord cursor_x;
    MapCoord cursor_y;
    char message[PLAYER_MP_MESSAGE_LEN];
};

struct Packet;

void initialize_packet_history(void);
/** Once a turn on every machine, before its packets are exchanged (exchange_packets): notes the turn each Daimon slot
 *  starts being played, the first turn the host writes a packet for it. */
void track_daimon_slot_packets(void);
/** A Daimon slot played since the turn in `*first_turn` (track_daimon_slot_packets); false: not played. */
TbBool daimon_slot_first_packet_turn(NetUserId slot, GameTurn *first_turn);
void store_packet_history(NetUserId user, const struct Packet *packet);
const struct Packet *get_history_packet(NetUserId user, GameTurn turn);
const struct Packet *get_latest_history_packet(NetUserId user);
TbBool read_repair_packet_history(NetUserId source, const char *buffer, size_t buffer_size);
void network_update(void *server_buf, size_t frame_size);
TbError LbNetwork_ExchangeGameplay(void *send_buf, void *server_buf, size_t frame_size);
void LbNetwork_BroadcastUnpause(void);
TbError process_network_unpause_message(NetUserId source, size_t buffer_size);
TbError process_network_turn_sync_message(NetUserId source, const char *buffer, size_t buffer_size);
void process_gameplay_chat_message(NetUserId user, const char *message, MapCoord cursor_x, MapCoord cursor_y);
void queue_gameplay_chat_message(NetUserId user, const char *message, MapCoord cursor_x, MapCoord cursor_y);
void load_gameplay_chat_messages(const struct QueuedGameplayChat *messages, int32_t count);
void process_queued_chat_messages(void);
TbBool is_waiting_on_user_packets(NetUserId user);
TbClockMSec get_peer_timeout_remaining(void);

#ifdef __cplusplus
}
#endif

#endif
