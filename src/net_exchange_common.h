/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_common.h
 *     Header file for net_exchange_common.c.
 * @par Purpose:
 *     Shared multiplayer message exchange routines.
 */
/******************************************************************************/
#ifndef DK_NET_EXCHANGE_COMMON_H
#define DK_NET_EXCHANGE_COMMON_H

#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool read_network_message_text(char **read_pos, const char **text, size_t max_len);
TbBool can_send_to_peer(NetUserId peer_id);
TbError process_network_message(NetUserId source, void *server_buf, size_t frame_size);
TbError LbNetwork_Exchange(enum NetMessageType msg_type, void *send_buf, void *server_buf, size_t buf_size);
void LbNetwork_WaitForMissingPackets(void *server_buf, size_t client_frame_size);
void LbNetwork_BroadcastUnpauseTimesync(void);

#ifdef __cplusplus
}
#endif

#endif
