/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_gameplay.c
 *     Gameplay packet exchange for Dungeon Keeper multiplayer.
 * @par Purpose:
 *     Packet redundancy and repair for multiplayer turns.
 * @author   KeeperFX Team
 * @date     30 Apr 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_exchange_gameplay.h"
#include "bflib_enet.h"
#include "bflib_datetm.h"
#include "bflib_video.h"
#include "config_keeperfx.h"
#include "console_cmd.h"
#include "engine_redraw.h"
#include "globals.h"
#include "gui_msgs.h"
#include "net_exchange_common.h"
#include "net_input_lag.h"
#include "net_main.h"
#include "player_data.h"
#include "net_game.h"
#include "packets.h"
#include "lua_triggers.h"
#include "keeperfx.hpp"
#include <zlib.h>
#include "post_inc.h"

extern void network_yield_waiting_gameplay_packets(void);
extern int32_t multiplayer_speed_adjustment_ns;

/******************************************************************************/

// PACKET_HISTORY_SIZE affects how far apart two player's turns can be (if they divert too far then there's no easy recovering). It should be set as high as possible while still being safe to send, so less than 1300 bytes at once.
// TURN_SYNC_MAX_ADJUSTMENT_NS being set too high causes stutters.
#define PACKET_HISTORY_SIZE 40
#define REPAIR_HISTORY_RESEND_INTERVAL 200
#define FINAL_RESORT_RESYNC_RECOVERY 10000
#define TURN_SYNC_INTERVAL_MS 250
#define TURN_SYNC_MAX_ADJUSTMENT_NS 1000000
#define TURN_SYNC_ADJUST_FACTOR 64

struct PacketHistory {
    struct Packet entries[PACKET_HISTORY_SIZE];
};

#pragma pack(1)
struct RedundantPacketBundle {
    unsigned char valid_count;
    struct Packet packets[PACKET_HISTORY_SIZE];
};

struct PacketHistoryHeader {
    PlayerNumber player;
    unsigned int compressed_length;
    unsigned int original_length;
};
#pragma pack()

static struct PacketHistory packet_history[MAX_NET_USERS];
static TbClockMSec server_turn_received_at = 0;
static int64_t server_turn_position_ns = 0;
static TbClockMSec last_repair_history_send = 0;
static TbClockMSec last_turn_sync_send = 0;

static int64_t get_current_turn_position_ns(void)
{
    if (turns_per_second <= 0) {
        return 0;
    }
    int64_t nanoseconds_per_turn = (1000000000ULL + turns_per_second / 2) / turns_per_second;
    int64_t position_ns = (int64_t)get_gameturn() * (int64_t)nanoseconds_per_turn;
    if (is_feature_on(Ft_DeltaTime) == true && game.process_turn_time > 0) {
        int64_t turn_phase_ns = (int64_t)((game.process_turn_time - 1) * nanoseconds_per_turn + 0.5);
        if (turn_phase_ns >= nanoseconds_per_turn) {
            turn_phase_ns = nanoseconds_per_turn - 1;
        }
        position_ns += (int64_t)turn_phase_ns;
    }
    return position_ns;
}

static void update_turn_speed_adjustment(void)
{
    TbClockMSec sync_age_ms = LbTimerClock() - server_turn_received_at;
    multiplayer_speed_adjustment_ns = 0;
    if (game.frame_skip > 0 || netstate.my_id == SERVER_ID || server_turn_received_at == 0 || turns_per_second <= 0
        || sync_age_ms > TURN_SYNC_INTERVAL_MS * 3) {
        return;
    }
    int64_t server_position_now_ns = server_turn_position_ns + (int64_t)sync_age_ms * 1000000;
    int64_t target_adjustment_ns = (get_current_turn_position_ns() - server_position_now_ns) / TURN_SYNC_ADJUST_FACTOR;
    target_adjustment_ns = clamp(target_adjustment_ns, -TURN_SYNC_MAX_ADJUSTMENT_NS, TURN_SYNC_MAX_ADJUSTMENT_NS);
    multiplayer_speed_adjustment_ns = (int32_t)target_adjustment_ns;
}

static void send_turn_sync_if_due(void)
{
    if (netstate.my_id != SERVER_ID) {
        return;
    }
    multiplayer_speed_adjustment_ns = 0;
    TbClockMSec current_time = LbTimerClock();
    if (last_turn_sync_send != 0 && current_time - last_turn_sync_send < TURN_SYNC_INTERVAL_MS) {
        return;
    }
    last_turn_sync_send = current_time;
    int64_t position_ns = get_current_turn_position_ns();
    char *write_pos = begin_net_message(NETMSG_GAMEPLAY_TURN_SYNC);
    memcpy(write_pos, &position_ns, sizeof(position_ns));
    write_pos += sizeof(position_ns);
    size_t message_size = write_pos - netstate.msg_buffer;
    send_to_active_peers(1, NetSend_Unsequenced, netstate.msg_buffer, message_size, INVALID_USER_ID, INVALID_USER_ID);
}

void LbNetwork_BroadcastUnpause(void)
{
    MULTIPLAYER_LOG("LbNetwork_BroadcastUnpause");
    char *write_pos = begin_net_message(NETMSG_UNPAUSE);
    send_remote_buffer(write_pos);
}

TbError process_network_unpause_message(void)
{
    if ((game.operation_flags & GOF_Paused) == 0) {
        MULTIPLAYER_LOG("ProcessMessage NETMSG_UNPAUSE: ignoring, not paused");
        return Lb_OK;
    }
    MULTIPLAYER_LOG("ProcessMessage NETMSG_UNPAUSE: applying unpause");
    unpausing_in_progress = 1;
    keeper_screen_redraw();
    LbScreenSwap();
    if (my_player_number == get_host_player_id()) {
        LbNetwork_BroadcastUnpause();
    }
    process_pause_packet(0, 0);
    unpausing_in_progress = 0;
    return Lb_OK;
}

void process_gameplay_chat_message(int player_id, const char *message)
{
    struct PlayerInfo *player = prepare_network_chat_message(player_id, message);
    if (message[0] != '\0') {
        lua_on_chatmsg(player_id, player->mp_message_text);
        if (player->mp_message_text[0] != cmd_char || !cmd_exec(player_id, player->mp_message_text + 1) || network_is_active()) {
            message_add(MsgType_Player, player_id, player->mp_message_text);
        }
    }
    memset(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
}

TbError process_network_turn_sync_message(NetUserId source, const char *buffer, size_t buffer_size)
{
    if (buffer_size != sizeof(int64_t)) {
        WARNLOG("Invalid gameplay turn sync message from peer %i (%u bytes)", source, (unsigned)buffer_size);
        return Lb_OK;
    }
    if (netstate.my_id == SERVER_ID) {
        multiplayer_speed_adjustment_ns = 0;
        return Lb_OK;
    }
    if (source != SERVER_ID) {
        WARNLOG("Ignoring gameplay turn sync message from peer %i", source);
        return Lb_OK;
    }
    int64_t received_position_ns;
    memcpy(&received_position_ns, buffer, sizeof(received_position_ns));
    int64_t one_way_latency_ns = (int64_t)(((uint64_t)GetPing(source) * 1000000 + 1) / 2);
    server_turn_position_ns = received_position_ns + one_way_latency_ns;
    server_turn_received_at = LbTimerClock();
    update_turn_speed_adjustment();
    return Lb_OK;
}

void store_packet_history(PlayerNumber player, const struct Packet *packet)
{
    if (player < 0 || player >= MAX_NET_USERS || is_packet_empty(packet)) {
        return;
    }
    struct Packet *entry = &packet_history[player].entries[packet->turn % PACKET_HISTORY_SIZE];
    if (!is_packet_empty(entry) && (GameTurnDelta)(entry->turn - packet->turn) > 0) {
        return;
    }
    *entry = *packet;
}

const struct Packet *get_history_packet(PlayerNumber player, GameTurn turn)
{
    if (player < 0 || player >= MAX_NET_USERS) {
        return NULL;
    }
    const struct Packet *packet = &packet_history[player].entries[turn % PACKET_HISTORY_SIZE];
    if (is_packet_empty(packet) || packet->turn != turn) {
        return NULL;
    }
    return packet;
}

TbBool read_repair_packet_history(NetUserId source, const char *buffer, size_t buffer_size)
{
    size_t header_size = sizeof(struct PacketHistoryHeader);
    if (buffer_size < header_size) {
        WARNLOG("Gameplay repair history from peer %i was too small (%u bytes)", source, (unsigned)buffer_size);
        return false;
    }
    struct PacketHistoryHeader header;
    memcpy(&header, buffer, sizeof(header));
    if (header.player < 0 || header.player >= netstate.max_players) {
        WARNLOG("Gameplay repair history from peer %i had invalid player %d", source, (int)header.player);
        return false;
    }
    if (source != SERVER_ID && source != header.player) {
        WARNLOG("Peer %i tried to send gameplay repair history for peer %i", source, (int)header.player);
        return false;
    }
    if (buffer_size != sizeof(struct PacketHistoryHeader) + header.compressed_length) {
        WARNLOG("Gameplay repair history from peer %i had size mismatch (%u != %u + %u)",
            source, (unsigned)buffer_size, (unsigned)sizeof(struct PacketHistoryHeader), header.compressed_length);
        return false;
    }
    if (header.original_length > sizeof(struct RedundantPacketBundle) || header.original_length < sizeof(unsigned char)) {
        WARNLOG("Gameplay repair history from peer %i had invalid original length %u", source, header.original_length);
        return false;
    }
    char packet_history_buffer[sizeof(struct RedundantPacketBundle)];
    uLongf packet_history_size = header.original_length;
    Bytef *packet_history_data = (Bytef *)packet_history_buffer;
    const Bytef *compressed_data = (const Bytef *)(buffer + header_size);
    int uncompress_result = uncompress(packet_history_data, &packet_history_size, compressed_data, header.compressed_length);
    if (uncompress_result != Z_OK || packet_history_size != header.original_length) {
        WARNLOG("Gameplay repair history decompression from peer %i failed: zlib error %d", source, uncompress_result);
        return false;
    }
    const struct RedundantPacketBundle *packet_bundle = (const struct RedundantPacketBundle *)packet_history_buffer;
    if (packet_bundle->valid_count < 1 || packet_bundle->valid_count > PACKET_HISTORY_SIZE) {
        WARNLOG("Gameplay repair history from peer %i had invalid packet count %u", source, (unsigned)packet_bundle->valid_count);
        return false;
    }
    if (header.original_length < sizeof(unsigned char) + packet_bundle->valid_count * sizeof(struct Packet)) {
        WARNLOG("Gameplay repair history from peer %i was truncated for player %d", source, (int)header.player);
        return false;
    }
    for (unsigned char i = 0; i < packet_bundle->valid_count; i += 1) {
        const struct Packet *packet = &packet_bundle->packets[i];
        if (is_packet_empty(packet)) {
            MULTIPLAYER_LOG("read_repair_packet_history: Skipping empty packet for player %d turn %lu", header.player, (unsigned long)packet->turn);
            continue;
        }
        store_packet_history(header.player, packet);
    }
    return true;
}

void initialize_packet_history(void)
{
    memset(packet_history, 0, sizeof(packet_history));
    server_turn_received_at = 0;
    server_turn_position_ns = 0;
    multiplayer_speed_adjustment_ns = 0;
    last_repair_history_send = 0;
    last_turn_sync_send = 0;
}

static TbBool have_all_turn_packets(PlayerNumber local_packet_player)
{
    GameTurn expected_turn = get_gameturn() - game.input_lag_turns;
    for (PlayerNumber player = 0; player < MAX_NET_USERS; player += 1) {
        const struct Packet *packet = get_history_packet(player, expected_turn);
        if (player != local_packet_player && network_player_active(player) && packet == NULL) {
            return false;
        }
    }
    return true;
}

static TbBool host_lost(GameTurn turn, const char *state)
{
    if (netstate.my_id == SERVER_ID || netstate.users[SERVER_ID].progress != USER_UNUSED) {
        return false;
    }
    MULTIPLAYER_LOG("LbNetwork_ExchangeGameplay: Host disconnected while %s turn=%lu", state, (unsigned long)turn);
    netstate.seq_nbr += 1;
    return true;
}

static void send_player_repair_history(PlayerNumber player)
{
    if (player < 0 || player >= MAX_NET_USERS || !network_player_active(player)) {
        return;
    }
    struct RedundantPacketBundle packet_bundle;
    packet_bundle.valid_count = 0;
    const struct PacketHistory *history = &packet_history[player];
    GameTurn latest_turn = 0;
    TbBool have_turn = false;
    for (int i = 0; i < PACKET_HISTORY_SIZE; i += 1) {
        const struct Packet *packet = &history->entries[i];
        if (is_packet_empty(packet)) {
            continue;
        }
        if (!have_turn || (GameTurnDelta)(packet->turn - latest_turn) > 0) {
            latest_turn = packet->turn;
            have_turn = true;
        }
    }
    if (!have_turn) {
        return;
    }
    for (GameTurnDelta offset = 0; offset < PACKET_HISTORY_SIZE; offset += 1) {
        if ((GameTurn)offset > latest_turn) {
            break;
        }
        GameTurn turn = latest_turn - offset;
        const struct Packet *packet = &history->entries[turn % PACKET_HISTORY_SIZE];
        if (is_packet_empty(packet) || packet->turn != turn) {
            continue;
        }
        packet_bundle.packets[packet_bundle.valid_count] = *packet;
        packet_bundle.valid_count += 1;
    }
    size_t packet_history_size = sizeof(unsigned char) + packet_bundle.valid_count * sizeof(struct Packet);
    char *write_pos = begin_net_message(NETMSG_GAMEPLAY_REPAIR);
    char *header_pos = write_pos;
    write_pos += sizeof(struct PacketHistoryHeader);
    uLongf compressed_size = sizeof(netstate.msg_buffer) - (write_pos - netstate.msg_buffer);
    int compress_result = compress((Bytef *)write_pos, &compressed_size, (const Bytef *)&packet_bundle, packet_history_size);
    if (compress_result != Z_OK) {
        ERRORLOG("Gameplay repair history compression failed for player %d: zlib error %d", (int)player, compress_result);
        return;
    }
    struct PacketHistoryHeader header;
    header.player = player;
    header.compressed_length = (unsigned int)compressed_size;
    header.original_length = (unsigned int)packet_history_size;
    memcpy(header_pos, &header, sizeof(header));
    size_t message_size = (write_pos - netstate.msg_buffer) + compressed_size;
    if (netstate.my_id != SERVER_ID) {
        if (can_send_to_peer(SERVER_ID)) {
            MULTIPLAYER_LOG("Sending unreliable compressed gameplay repair history for player=%d to host (%lu -> %lu bytes)",
                (int)player, (unsigned long)packet_history_size, (unsigned long)compressed_size);
            netstate.sp->sendmsg_single_unsequenced(SERVER_ID, netstate.msg_buffer, message_size);
        }
        return;
    }
    NetUserId skip_peer_id = INVALID_USER_ID;
    if ((NetUserId)player != SERVER_ID) {
        skip_peer_id = (NetUserId)player;
    }
    MULTIPLAYER_LOG("Sending unreliable compressed gameplay repair history for player=%d to clients (skip=%d) (%lu -> %lu bytes)",
        (int)player, (int)skip_peer_id, (unsigned long)packet_history_size, (unsigned long)compressed_size);
    send_to_active_peers(1, NetSend_Unsequenced, netstate.msg_buffer, message_size, skip_peer_id, INVALID_USER_ID);
}

static void send_repair_history_if_due(void)
{
    TbClockMSec current_time = LbTimerClock();
    if (last_repair_history_send != 0 && current_time - last_repair_history_send < REPAIR_HISTORY_RESEND_INTERVAL) {
        return;
    }
    last_repair_history_send = current_time;
    if (netstate.my_id != SERVER_ID) {
        send_player_repair_history((PlayerNumber)netstate.my_id);
        return;
    }
    for (PlayerNumber player = 0; player < netstate.max_players; player += 1) {
        send_player_repair_history(player);
    }
}

static TbError wait_for_missing_packets(void *server_buf, size_t frame_size, PlayerNumber local_packet_player)
{
    GameTurn expected_turn = get_gameturn() - game.input_lag_turns;
    TbClockMSec wait_start_time = LbTimerClock();
    TbBool turn_complete = false;
    TbBool wait_timed_out = false;
    MULTIPLAYER_LOG("LbNetwork_ExchangeGameplay: Missing packets for turn=%lu, collecting...", (unsigned long)expected_turn);
    while (!turn_complete) {
        send_turn_sync_if_due();
        send_repair_history_if_due();
        netstate.sp->update(OnNewUser);
        if (host_lost(expected_turn, "waiting for")) {
            return Lb_OK;
        }
        turn_complete = have_all_turn_packets(local_packet_player);
        for (NetUserId peer_id = 0; peer_id < netstate.max_players && !turn_complete; peer_id += 1) {
            const struct Packet *packet = get_history_packet(peer_id, expected_turn);
            if (!can_send_to_peer(peer_id)) {
                continue;
            }
            if (netstate.my_id == SERVER_ID && packet != NULL) {
                continue;
            }
            process_peer_msgs(peer_id, server_buf, frame_size);
            if (host_lost(expected_turn, "collecting")) {
                return Lb_OK;
            }
            turn_complete = have_all_turn_packets(local_packet_player);
        }
        if (turn_complete) {
            break;
        }
        if (LbTimerClock() - wait_start_time >= FINAL_RESORT_RESYNC_RECOVERY) {
            set_flag(game.system_flags, GSF_NetGameNoSync);
            clear_flag(game.system_flags, GSF_NetSeedNoSync);
            wait_timed_out = true;
            break;
        }
        update_turn_speed_adjustment();
        network_yield_waiting_gameplay_packets();
        if (quit_game || exit_keeper) {
            netstate.seq_nbr += 1;
            return Lb_OK;
        }
    }
    TbClockMSec wait_time = LbTimerClock() - wait_start_time;
    if (wait_timed_out) {
        WARNLOG("LbNetwork_ExchangeGameplay: Timed out waiting for turn=%lu after %dms; continuing so resync can recover",
            (unsigned long)expected_turn, (int32_t)wait_time);
    } else {
        MULTIPLAYER_LOG("LbNetwork_ExchangeGameplay: Completed wait for turn=%lu after %dms", (unsigned long)expected_turn, (int32_t)wait_time);
    }
    netstate.seq_nbr += 1;
    return Lb_OK;
}

void network_update(void *server_buf, size_t frame_size)
{
    if (netstate.sp == NULL) {
        return;
    }
    netstate.sp->update(OnNewUser);
    for (NetUserId peer_id = 0; peer_id < netstate.max_players; peer_id += 1) {
        if (can_send_to_peer(peer_id)) {
            process_peer_msgs(peer_id, server_buf, frame_size);
        }
    }
}

TbError LbNetwork_ExchangeGameplay(void *send_buf, void *server_buf, size_t frame_size)
{
    if (exchange_frame_message(send_buf, server_buf, frame_size, NETMSG_GAMEPLAY_UNSEQUENCED) != Lb_OK) {
        return Lb_FAIL;
    }
    send_turn_sync_if_due();
    send_repair_history_if_due();
    network_update(server_buf, frame_size);
    update_turn_speed_adjustment();
    if (game.skip_initial_input_turns <= 0) {
        struct PlayerInfo *my_player = get_my_player();
        if (player_exists(my_player)) {
            PlayerNumber local_packet_player = my_player->packet_num;
            if (!have_all_turn_packets(local_packet_player)) {
                return wait_for_missing_packets(server_buf, frame_size, local_packet_player);
            }
        }
    }
    netstate.seq_nbr += 1;
    return Lb_OK;
}
