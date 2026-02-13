/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file redundant_packets.c
 *     Redundant packet bundling for network game packet loss prevention.
 * @par Purpose:
 *     Bundles current packet with previous 2 packets to prevent packet loss.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_redundant_packets.h"
#include "packets.h"
#include "net_received_packets.h"
#include "bflib_network.h"
#include <string.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define HISTORY_SIZE 2

struct PacketHistory {
    struct Packet packets[HISTORY_SIZE];
    unsigned char write_index;
    unsigned char valid_count;
};

static struct PacketHistory packet_history[NET_PLAYERS_COUNT];

/******************************************************************************/

void initialize_redundant_packets(void) {
    memset(packet_history, 0, sizeof(packet_history));
}

void clear_redundant_packets(void) {
    memset(packet_history, 0, sizeof(packet_history));
}

void store_sent_packet(PlayerNumber player, const struct Packet* packet) {
    if (player < 0 || player >= NET_PLAYERS_COUNT) {
        return;
    }
    struct PacketHistory* history = &packet_history[player];
    memcpy(&history->packets[history->write_index], packet, sizeof(struct Packet));
    history->write_index = (history->write_index + 1) % HISTORY_SIZE;
    if (history->valid_count < HISTORY_SIZE) {
        history->valid_count += 1;
    }
}

size_t bundle_packets(PlayerNumber player, const struct Packet* current_packet, char* out_buffer) {
    if (player < 0 || player >= NET_PLAYERS_COUNT) {
        return 0;
    }
    struct PacketHistory* history = &packet_history[player];
    struct BundledPacket bundled;
    bundled.valid_count = 1;
    memcpy(&bundled.packets[0], current_packet, sizeof(struct Packet));
    if (history->valid_count >= 1) {
        int prev_index = (history->write_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        memcpy(&bundled.packets[1], &history->packets[prev_index], sizeof(struct Packet));
        bundled.valid_count += 1;
    }
    if (history->valid_count >= 2) {
        int prev_prev_index = (history->write_index - 2 + HISTORY_SIZE) % HISTORY_SIZE;
        memcpy(&bundled.packets[2], &history->packets[prev_prev_index], sizeof(struct Packet));
        bundled.valid_count += 1;
    }
    memcpy(out_buffer, &bundled, sizeof(struct BundledPacket));
    return sizeof(struct BundledPacket);
}

void unbundle_packets(const char* bundled_buffer, PlayerNumber source_player) {
    if (source_player < 0 || source_player >= NET_PLAYERS_COUNT) {
        return;
    }
    struct BundledPacket bundled;
    memcpy(&bundled, bundled_buffer, sizeof(struct BundledPacket));
    if (bundled.valid_count > REDUNDANT_PACKET_COUNT) {
        return;
    }
    int i;
    for (i = 0; i < bundled.valid_count; i += 1) {
        const struct Packet* packet = &bundled.packets[i];
        const struct Packet* existing = get_received_packet_for_player(packet->turn, source_player);
        if (existing == NULL) {
            store_received_packet(packet->turn, source_player, packet);
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
