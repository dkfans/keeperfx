/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_common.h
 *     Header file for net_exchange_common.c.
 * @par Purpose:
 *     Public declarations for shared multiplayer message exchange routines.
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
#ifndef DK_NET_EXCHANGE_COMMON_H
#define DK_NET_EXCHANGE_COMMON_H

#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

enum NetworkPeerSendMode {
    NetSend_Reliable,
    NetSend_Unsequenced,
};

TbBool read_network_message_text(char **read_pos, const char **text, size_t max_len);
void send_network_chat_message(int player_id, const char *message);
struct PlayerInfo *prepare_network_chat_message(int player_id, const char *message);
TbBool can_send_to_peer(NetUserId peer_id);
void send_to_active_peers(int send_count, enum NetworkPeerSendMode send_mode, const char *buffer, size_t msg_size, NetUserId first_skip_id, NetUserId second_skip_id);
TbBool all_expected_exchange_frames_received(const TbBool has_received_frame[MAX_NET_USERS], TbBool is_host);
TbError exchange_frame_message(void *send_buf, void *server_buf, size_t frame_size, enum NetMessageType msg_type);
TbError process_network_message(NetUserId source, void *server_buf, size_t frame_size, enum NetMessageType expected_frame_type, NetUserId *frame_peer_id);
TbError exchange_frame_block(enum NetMessageType msg_type, void *send_buf, void *server_buf, size_t frame_size);
void process_peer_msgs(NetUserId peer_id, void *server_buf, size_t frame_size);
void wait_for_all_players(void);

#ifdef __cplusplus
}
#endif

#endif
