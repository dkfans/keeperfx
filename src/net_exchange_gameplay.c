/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_gameplay.c
 *     Gameplay packet exchange for Dungeon Keeper multiplayer.
 * @par Purpose:
 *     Gameplay-specific packet bundling and missing-packet recovery.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_exchange_gameplay.h"

#include "bflib_datetm.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "net_exchange_common.h"
#include "net_game.h"
#include "packets.h"
#include "player_data.h"
#include "post_inc.h"

extern void network_yield_draw_gameplay(void);
/******************************************************************************/

#define RECEIVED_PACKETS_HISTORY_TURNS 40
#define RECEIVED_PACKETS_HISTORY_SIZE (RECEIVED_PACKETS_HISTORY_TURNS * PACKETS_COUNT)
#define REDUNDANT_PACKET_HISTORY_SIZE 2

struct ReceivedPacketEntry {
    GameTurn turn;
    PlayerNumber player;
    TbBool valid;
    struct Packet packet;
};

struct ReceivedPacketHistory {
    struct ReceivedPacketEntry entries[RECEIVED_PACKETS_HISTORY_SIZE];
    int head;
};

struct PacketHistory {
    struct Packet packets[REDUNDANT_PACKET_HISTORY_SIZE];
    unsigned char write_index;
    unsigned char valid_count;
};

static struct ReceivedPacketHistory received_packets_history;
static struct PacketHistory packet_history[MAX_NET_USERS];

void clear_packet_tracking(void) {
    for (int i = 0; i < RECEIVED_PACKETS_HISTORY_SIZE; i += 1) {
        struct ReceivedPacketEntry *entry = &received_packets_history.entries[i];
        entry->turn = 10000000;
        entry->player = 0;
        entry->valid = false;
        memset(&entry->packet, 0, sizeof(entry->packet));
    }
    received_packets_history.head = 0;
}

void initialize_packet_tracking(void) {
    clear_packet_tracking();
}

void store_received_packet(GameTurn turn, PlayerNumber player, const struct Packet *packet) {
    if (is_packet_empty(packet)) {
        MULTIPLAYER_LOG("store_received_packet: Skipping empty packet for player %d turn %lu", player, (unsigned long)turn);
        return;
    }
    int index = received_packets_history.head;
    struct ReceivedPacketEntry *entry = &received_packets_history.entries[index];
    entry->turn = turn;
    entry->player = player;
    entry->valid = true;
    memcpy(&entry->packet, packet, sizeof(struct Packet));
    received_packets_history.head = (index + 1) % RECEIVED_PACKETS_HISTORY_SIZE;
}

void store_received_packets(void) {
    MULTIPLAYER_LOG("store_received_packets: RECEIVED packets from network");
    struct PlayerInfo *player = get_my_player();
    PlayerNumber my_packet_num = 0;
    if (player_exists(player)) {
        my_packet_num = player->packet_num;
    }
    for (int i = 0; i < PACKETS_COUNT; i += 1) {
        if (i != my_packet_num) {
            if (is_packet_empty(&game.packets[i])) {
                MULTIPLAYER_LOG("store_received_packets: RECEIVED packet[%d] is EMPTY", i);
            } else {
                MULTIPLAYER_LOG("store_received_packets: RECEIVED packet[%d] turn=%lu checksum=%08lx", i, (unsigned long)game.packets[i].turn, (unsigned long)game.packets[i].checksum);
                store_received_packet(game.packets[i].turn, (PlayerNumber)i, &game.packets[i]);
            }
        }
    }
}

const struct Packet *get_received_packets_for_turn(GameTurn turn) {
    static struct Packet packets_for_turn[PACKETS_COUNT];
    TbBool found = false;
    memset(packets_for_turn, 0, sizeof(packets_for_turn));

    for (int i = 0; i < RECEIVED_PACKETS_HISTORY_SIZE; i += 1) {
        const struct ReceivedPacketEntry *entry = &received_packets_history.entries[i];
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

const struct Packet *get_received_packet_for_player(GameTurn turn, PlayerNumber player) {
    for (int i = 0; i < RECEIVED_PACKETS_HISTORY_SIZE; i += 1) {
        const struct ReceivedPacketEntry *entry = &received_packets_history.entries[i];
        if (entry->valid && entry->turn == turn && entry->player == player) {
            return &entry->packet;
        }
    }
    return NULL;
}

void clear_redundant_packets(void) {
    memset(packet_history, 0, sizeof(packet_history));
}

void initialize_redundant_packets(void) {
    clear_redundant_packets();
}

void store_sent_packet(PlayerNumber player, const struct Packet *packet) {
    if (player < 0 || player >= MAX_NET_USERS) {
        return;
    }
    struct PacketHistory *history = &packet_history[player];
    memcpy(&history->packets[history->write_index], packet, sizeof(struct Packet));
    history->write_index = (history->write_index + 1) % REDUNDANT_PACKET_HISTORY_SIZE;
    if (history->valid_count < REDUNDANT_PACKET_HISTORY_SIZE) {
        history->valid_count += 1;
    }
}

size_t bundle_packets(PlayerNumber player, const struct Packet *current_packet, char *out_buffer) {
    if (player < 0 || player >= MAX_NET_USERS) {
        return 0;
    }
    struct PacketHistory *history = &packet_history[player];
    struct BundledPacket bundled;
    bundled.valid_count = 1;
    memcpy(&bundled.packets[0], current_packet, sizeof(struct Packet));
    if (history->valid_count >= 1) {
        int prev_index = (history->write_index - 1 + REDUNDANT_PACKET_HISTORY_SIZE) % REDUNDANT_PACKET_HISTORY_SIZE;
        memcpy(&bundled.packets[1], &history->packets[prev_index], sizeof(struct Packet));
        bundled.valid_count += 1;
    }
    if (history->valid_count >= 2) {
        int prev_prev_index = (history->write_index - 2 + REDUNDANT_PACKET_HISTORY_SIZE) % REDUNDANT_PACKET_HISTORY_SIZE;
        memcpy(&bundled.packets[2], &history->packets[prev_prev_index], sizeof(struct Packet));
        bundled.valid_count += 1;
    }
    memcpy(out_buffer, &bundled, sizeof(struct BundledPacket));
    return sizeof(struct BundledPacket);
}

void unbundle_packets(const char *bundled_buffer, PlayerNumber source_player) {
    if (source_player < 0 || source_player >= MAX_NET_USERS) {
        return;
    }
    struct BundledPacket bundled;
    memcpy(&bundled, bundled_buffer, sizeof(struct BundledPacket));
    if (bundled.valid_count > REDUNDANT_PACKET_COUNT) {
        return;
    }
    for (int i = 0; i < bundled.valid_count; i += 1) {
        const struct Packet *packet = &bundled.packets[i];
        const struct Packet *existing = get_received_packet_for_player(packet->turn, source_player);
        if (existing == NULL) {
            store_received_packet(packet->turn, source_player, packet);
        }
    }
}

size_t write_gameplay_frame_payload(NetUserId source_id, const void *send_buf, char *write_pos)
{
    return bundle_packets((PlayerNumber)source_id, (const struct Packet *)send_buf, write_pos);
}

void send_gameplay_frame_to_peer(NetUserId peer_id, const char *buffer, size_t message_size)
{
    netstate.sp->sendmsg_single_unsequenced(peer_id, buffer, message_size);
    netstate.sp->sendmsg_single(peer_id, buffer, message_size);
}

void store_sent_gameplay_frame(NetUserId source_id, const void *send_buf)
{
    store_sent_packet((PlayerNumber)source_id, (const struct Packet *)send_buf);
}

TbError process_gameplay_frame_payload(NetUserId peer_id, char *read_pos, void *server_buf, size_t frame_size)
{
    char *peer_buf = ((char *)server_buf) + peer_id * frame_size;
    struct BundledPacket *bundled = (struct BundledPacket *)read_pos;
    memcpy(peer_buf, &bundled->packets[0], frame_size);
    unbundle_packets(read_pos, (PlayerNumber)peer_id);
    return Lb_OK;
}

void LbNetwork_WaitForMissingPackets(void *server_buf, size_t client_frame_size)
{
    if (game.skip_initial_input_turns > 0) {
        return;
    }
    GameTurn historical_turn = get_gameturn() - game.input_lag_turns;
    const struct Packet *received_packets = get_received_packets_for_turn(historical_turn);
    if (received_packets == NULL) {
        MULTIPLAYER_LOG("LbNetwork_WaitForMissingPackets: Missing packets for turn=%lu, waiting...", (unsigned long)historical_turn);
        TbClockMSec start = LbTimerClock();
        while (true) {
            int elapsed = LbTimerClock() - start;
            TbBool has_remote_peer = false;
            if (elapsed >= TIMEOUT_GAMEPLAY_MISSING_PACKET) {
                MULTIPLAYER_LOG("LbNetwork_WaitForMissingPackets: Timeout waiting for turn=%lu packets", (unsigned long)historical_turn);
                break;
            }

            netstate.sp->update(OnNewUser);
            const int wait_time = min(TIMEOUT_GAMEPLAY_MISSING_PACKET - elapsed, 100);
            for (NetUserId peer_id = 0; peer_id < netstate.max_players; peer_id += 1) {
                if (!can_send_to_peer(peer_id)) {
                    continue;
                }

                has_remote_peer = true;
                if (netstate.sp->msgready(peer_id, wait_time)) {
                    while (netstate.sp->msgready(peer_id, 0)) {
                        process_network_message(peer_id, server_buf, client_frame_size);
                    }
                }
            }
            if (!has_remote_peer) {
                break;
            }

            received_packets = get_received_packets_for_turn(historical_turn);
            if (received_packets != NULL) {
                MULTIPLAYER_LOG("LbNetwork_WaitForMissingPackets: Successfully received packets for turn=%lu after %dms", (unsigned long)historical_turn, elapsed);
                break;
            }

            network_yield_draw_gameplay();
        }
    }
}

void LbNetwork_BroadcastUnpauseTimesync(void)
{
    MULTIPLAYER_LOG("LbNetwork_BroadcastUnpauseTimesync");
    begin_net_message(NETMSG_UNPAUSE);
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (id != netstate.my_id && IsUserActive(id)) {
            netstate.sp->sendmsg_single(id, netstate.msg_buffer, 1);
        }
    }
}
