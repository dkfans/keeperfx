/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_gameplay.h
 *     Header file for net_exchange_gameplay.c.
 * @par Purpose:
 *     Gameplay-specific multiplayer exchange routines.
 */
/******************************************************************************/
#ifndef DK_NET_EXCHANGE_GAMEPLAY_H
#define DK_NET_EXCHANGE_GAMEPLAY_H

#include "bflib_basics.h"
#include "globals.h"
#include "net_main.h"
#include "packets.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REDUNDANT_PACKET_COUNT 3

#pragma pack(1)
struct BundledPacket {
    unsigned char valid_count;
    struct Packet packets[REDUNDANT_PACKET_COUNT];
};
#pragma pack()

void initialize_packet_tracking(void);
void clear_packet_tracking(void);
void store_received_packets(void);
void store_received_packet(GameTurn turn, PlayerNumber player, const struct Packet *packet);
const struct Packet *get_received_packets_for_turn(GameTurn turn);
const struct Packet *get_received_packet_for_player(GameTurn turn, PlayerNumber player);
void initialize_redundant_packets(void);
void clear_redundant_packets(void);
void store_sent_packet(PlayerNumber player, const struct Packet *packet);
size_t bundle_packets(PlayerNumber player, const struct Packet *current_packet, char *out_buffer);
void unbundle_packets(const char *bundled_buffer, PlayerNumber source_player);
size_t write_gameplay_frame_payload(NetUserId source_id, const void *send_buf, char *write_pos);
void send_gameplay_frame_to_peer(NetUserId peer_id, const char *buffer, size_t message_size);
void store_sent_gameplay_frame(NetUserId source_id, const void *send_buf);
TbError process_gameplay_frame_payload(NetUserId peer_id, char *read_pos, void *server_buf, size_t frame_size);
void LbNetwork_WaitForMissingPackets(void *server_buf, size_t client_frame_size);
void LbNetwork_BroadcastUnpauseTimesync(void);

#ifdef __cplusplus
}
#endif

#endif
