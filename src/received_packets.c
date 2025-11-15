/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file received_packets.c
 *     Received packets list tracking for network game synchronization.
 * @par Purpose:
 *     Stores and retrieves received packet data for multiplayer desync debugging.
 * @par Comment:
 *     Uses a circular buffer to store the last N turns of received packets.
 * @author   KeeperFX Team
 * @date     24 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "received_packets.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_network.h"
#include "packets.h"
#include "player_data.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define RECEIVED_PACKETS_HISTORY_TURNS 40
#define RECEIVED_PACKETS_HISTORY_SIZE (RECEIVED_PACKETS_HISTORY_TURNS * PACKETS_COUNT)

struct ReceivedPacketEntry {
    GameTurn turn;
    PlayerNumber player;
    TbBool valid;
    struct Packet packet;
};

static struct {
    struct ReceivedPacketEntry entries[RECEIVED_PACKETS_HISTORY_SIZE];
    int head;
} received_packets_history;

/******************************************************************************/
static void clear_received_packet_entry(struct ReceivedPacketEntry* entry) {
    entry->turn = 10000000;
    entry->player = 0;
    entry->valid = false;
    memset(&entry->packet, 0, sizeof(entry->packet));
}

void clear_packet_tracking(void) {
    for (int i = 0; i < RECEIVED_PACKETS_HISTORY_SIZE; ++i) {
        clear_received_packet_entry(&received_packets_history.entries[i]);
    }
    received_packets_history.head = 0;
}

void initialize_packet_tracking(void) {
    clear_packet_tracking();
}

void store_received_packet(GameTurn turn, PlayerNumber player, const struct Packet* packet) {
    if (is_packet_empty(packet)) {
        MULTIPLAYER_LOG("store_received_packet: Skipping empty packet for player %d turn %lu", player, (unsigned long)turn);
        return;
    }
    int index = received_packets_history.head;
    struct ReceivedPacketEntry* entry = &received_packets_history.entries[index];
    entry->turn = turn;
    entry->player = player;
    entry->valid = true;
    memcpy(&entry->packet, packet, sizeof(struct Packet));
    received_packets_history.head = (index + 1) % RECEIVED_PACKETS_HISTORY_SIZE;
}

void store_received_packets(void) {
    MULTIPLAYER_LOG("store_received_packets: RECEIVED packets from network");
    struct PlayerInfo* player = get_my_player();
    PlayerNumber my_packet_num = 0;
    if (player_exists(player)) {
        my_packet_num = player->packet_num;
    }
    for (int i = 0; i < PACKETS_COUNT; ++i) {
        if (i != my_packet_num) {
            if (is_packet_empty(&game.packets[i])) {
                MULTIPLAYER_LOG("store_received_packets: RECEIVED packet[%d] is EMPTY", i);
            } else {
                MULTIPLAYER_LOG("store_received_packets: RECEIVED packet[%d] turn=%lu checksum=%08lx",
                        i, (unsigned long)game.packets[i].turn,
                        (unsigned long)game.packets[i].checksum);
                store_received_packet(game.packets[i].turn, (PlayerNumber)i, &game.packets[i]);
            }
        }
    }
}

const struct Packet* get_received_packets_for_turn(GameTurn turn) {
    static struct Packet packets_for_turn[PACKETS_COUNT];
    TbBool found = false;
    memset(packets_for_turn, 0, sizeof(packets_for_turn));

    for (int i = 0; i < RECEIVED_PACKETS_HISTORY_SIZE; ++i) {
        const struct ReceivedPacketEntry* entry = &received_packets_history.entries[i];
        if (entry->valid && entry->turn == turn && entry->player >= 0 && entry->player < PACKETS_COUNT && entry->turn != 10000000) {
            packets_for_turn[entry->player] = entry->packet;
            found = true;
        }
    }

    if (!found) {
        return NULL;
    }
    return packets_for_turn;
}

const struct Packet* get_received_packet_for_player(GameTurn turn, PlayerNumber player) {
    for (int i = 0; i < RECEIVED_PACKETS_HISTORY_SIZE; ++i) {
        const struct ReceivedPacketEntry* entry = &received_packets_history.entries[i];
        if (entry->valid && entry->turn == turn && entry->player == player) {
            return &entry->packet;
        }
    }
    return NULL;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
