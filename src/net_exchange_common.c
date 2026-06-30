/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_common.c
 *     Shared multiplayer message exchange for Dungeon Keeper.
 * @par Purpose:
 *     Relay, dispatch and synchronized frame exchange routines.
 * @author   KeeperFX Team
 * @date     09 May 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_exchange_common.h"

#include "bflib_datetm.h"
#include "bflib_video.h"
#include "bflib_inputctrl.h"
#include "front_landview.h"
#include "frontend.h"
#include "game_legacy.h"
#include "net_game.h"
#include "net_exchange_gameplay.h"
#include "net_lobby.h"
#include "packets.h"
#include "player_data.h"
#include <SDL2/SDL.h>
#include "post_inc.h"
/******************************************************************************/

// When set too high the downside is these increase bandwidth usage and possibly congestion. When set too low you get stutters (pay attention to "Stutter Avg").
#define SEND_DUPLICATE_PACKETS 3
#define REDUNDANT_PACKET_BUNDLE 3

// Tested at 10% packet loss:
// 4 duplicate and 4 redundant = Diminishing returns (no difference)
// 3 duplicate and 3 redundant = Good
// 2 duplicate and 3 redundant = Micro stutter
// 3 duplicate and 2 redundant = Micro stutter

extern void network_yield_draw_frontend(void);
extern void network_yield_draw_gameplay(void);

void send_to_active_peers(int send_count, enum NetworkPeerSendMode send_mode, const char *buffer, size_t msg_size, NetUserId first_skip_id, NetUserId second_skip_id)
{
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (id == first_skip_id || id == second_skip_id || !can_send_to_peer(id)) {
            continue;
        }
        if (send_mode == NetSend_Reliable) {
            netstate.sp->sendmsg_single(id, buffer, msg_size);
            continue;
        }
        for (int i = 0; i < send_count; i += 1) {
            netstate.sp->sendmsg_single_unsequenced(id, buffer, msg_size);
        }
    }
}

static void send_exchange_message(enum NetMessageType message_type, size_t message_size, NetUserId skip_peer_id)
{
    switch (message_type) {
    case NETMSG_GAMEPLAY_UNSEQUENCED:
        send_to_active_peers(SEND_DUPLICATE_PACKETS, NetSend_Unsequenced, netstate.msg_buffer, message_size, netstate.my_id, skip_peer_id);
        return;
    default:
        send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, message_size, netstate.my_id, skip_peer_id);
        return;
    }
}

static TbError handle_exchange_message(NetUserId source, void *server_buf, size_t frame_size, enum NetMessageType message_type, enum NetMessageType expected_message_type, NetUserId *frame_peer_id, char *read_pos, size_t message_size)
{
    if (message_type != expected_message_type) {
        return Lb_OK;
    }
    NetUserId peer_id;
    int32_t seq_nbr;
    if (message_size < (size_t)(read_pos - netstate.msg_buffer) + 1 + sizeof(seq_nbr)) {
        WARNLOG("Message type %d from %i is too short", (int)message_type, (int)source);
        return Lb_OK;
    }
    peer_id = (NetUserId)(uint8_t)read_pos[0];
    read_pos += 1;
    if (peer_id >= netstate.max_players) {
        ERRORLOG("Critical error: Out of range peer ID %i received, could be used for buffer overflow attack", peer_id);
        abort();
    }
    memcpy(&seq_nbr, read_pos, sizeof(seq_nbr));
    read_pos += sizeof(seq_nbr);
    if (source != SERVER_ID && source != peer_id) {
        WARNLOG("Peer %i tried to send message type %d for peer %i", (int)source, (int)message_type, (int)peer_id);
        return Lb_OK;
    }
    char *player_frame = (char *)server_buf + peer_id * frame_size;
    netstate.users[peer_id].ack = seq_nbr;
    size_t payload_size = message_size - (read_pos - netstate.msg_buffer);
    if (message_type == NETMSG_GAMEPLAY_UNSEQUENCED) {
        if (frame_size != sizeof(struct Packet)) {
            WARNLOG("Gameplay frame size mismatch (%u != %u)", (unsigned)frame_size, (unsigned)sizeof(struct Packet));
            return Lb_OK;
        }
        if (payload_size < sizeof(unsigned char)) {
            WARNLOG("Invalid gameplay packet bundle from peer %i (%u bytes)", peer_id, (unsigned)payload_size);
            return Lb_OK;
        }
        unsigned char packet_count = (unsigned char)read_pos[0];
        read_pos += 1;
        if (packet_count < 1 || packet_count > REDUNDANT_PACKET_BUNDLE
         || payload_size != sizeof(unsigned char) + packet_count * sizeof(struct Packet)) {
            WARNLOG("Invalid gameplay packet bundle from peer %i (%u bytes)", peer_id, (unsigned)payload_size);
            return Lb_OK;
        }
        const struct Packet *packets = (const struct Packet *)read_pos;
        for (unsigned char i = 0; i < packet_count; i += 1) {
            if (is_packet_empty(&packets[i])) {
                MULTIPLAYER_LOG("process_network_message: Skipping empty packet for player %d turn %lu", peer_id, (unsigned long)packets[i].turn);
                continue;
            }
            store_packet_history((PlayerNumber)peer_id, &packets[i]);
        }
        struct Packet *frame_packet = (struct Packet *)player_frame;
        *frame_packet = packets[0];
    } else {
        if (payload_size != frame_size) {
            WARNLOG("Ignoring frame message type %d from peer %i with %u bytes, expected %u bytes",
                (int)message_type, peer_id, (unsigned)payload_size, (unsigned)frame_size);
            return Lb_OK;
        }
        memcpy(player_frame, read_pos, frame_size);
    }
    if (frame_peer_id != NULL) {
        *frame_peer_id = peer_id;
    }
    if (netstate.my_id == SERVER_ID) {
        send_exchange_message(message_type, message_size, peer_id);
    }
    return Lb_OK;
}

static TbError handle_chat_message(NetUserId source, char *read_pos, size_t message_size, enum NetMessageType expected_frame_type)
{
    int player_id = (int)read_pos[0];
    read_pos += 1;
    const char *message;
    if (!read_network_message_text(&read_pos, &message, sizeof(netstate.msg_buffer) - 1)) {
        ERRORLOG("Chat message too long or not null-terminated");
        return Lb_OK;
    }
    if (expected_frame_type == NETMSG_GAMEPLAY_UNSEQUENCED) {
        process_gameplay_chat_message(player_id, message);
    } else {
        process_frontend_chat_message(player_id, message);
    }
    if (netstate.my_id == SERVER_ID && source != SERVER_ID) {
        send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, message_size, netstate.my_id, source);
    }
    return Lb_OK;
}

TbBool read_network_message_text(char **read_pos, const char **text, size_t max_len)
{
    size_t max_read = sizeof(netstate.msg_buffer) - (*read_pos - netstate.msg_buffer);
    size_t len = strnlen(*read_pos, max_read);
    if (len >= max_read || len > max_len) {
        return false;
    }
    *text = *read_pos;
    *read_pos += len + 1;
    return true;
}

void send_network_chat_message(int player_id, const char *message)
{
    char *write_pos = begin_net_message(NETMSG_CHATMESSAGE);
    *write_pos = player_id;
    write_pos += 1;
    strcpy(write_pos, message);
    write_pos += strlen(message) + 1;
    send_remote_buffer(write_pos);
}

struct PlayerInfo *prepare_network_chat_message(int player_id, const char *message)
{
    struct PlayerInfo *player = get_player(player_id);
    player->allocflags &= ~PlaF_NewMPMessage;
    //LbStopTextInput();
    if (message[0] != '\0') {
        memcpy(player->mp_message_text, message, PLAYER_MP_MESSAGE_LEN);
        memcpy(player->mp_message_text_last, message, PLAYER_MP_MESSAGE_LEN);
    } else {
        memset(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
    }
    return player;
}

TbBool can_send_to_peer(NetUserId peer_id)
{
    return (peer_id != netstate.my_id) &&
        (netstate.users[peer_id].progress != USER_UNUSED) &&
        (my_player_number == get_host_player_id() || peer_id == SERVER_ID);
}

TbBool all_expected_exchange_frames_received(const TbBool has_received_frame[MAX_NET_USERS], TbBool is_host)
{
    for (NetUserId peer_id = 0; peer_id < netstate.max_players; peer_id += 1) {
        if (is_host && !can_send_to_peer(peer_id)) {
            continue;
        }
        if (!is_host && (peer_id == netstate.my_id || !IsUserActive(peer_id))) {
            continue;
        }
        if (!has_received_frame[peer_id]) {
            return false;
        }
    }
    return true;
}

TbError exchange_frame_message(void *send_buf, void *server_buf, size_t frame_size, enum NetMessageType msg_type)
{
    if (netstate.my_id < 0 || netstate.my_id >= netstate.max_players) {
        ERRORLOG("Invalid my_id %i in network exchange (disconnected?)", netstate.my_id);
        return Lb_FAIL;
    }
    netstate.sp->update(OnNewUser);
    char *my_frame = (char *)server_buf + netstate.my_id * frame_size;
    memcpy(my_frame, send_buf, frame_size);
    char *write_pos = begin_net_message(msg_type);
    *write_pos = (char)netstate.my_id;
    write_pos += 1;
    memcpy(write_pos, &netstate.seq_nbr, sizeof(netstate.seq_nbr));
    write_pos += sizeof(netstate.seq_nbr);
    if (msg_type == NETMSG_GAMEPLAY_UNSEQUENCED) {
        const struct Packet *current_packet = (const struct Packet *)send_buf;
        unsigned char *packet_count = (unsigned char *)write_pos;
        *packet_count = 1;
        write_pos += 1;
        memcpy(write_pos, current_packet, sizeof(struct Packet));
        write_pos += sizeof(struct Packet);
        for (GameTurnDelta offset = 1; *packet_count < REDUNDANT_PACKET_BUNDLE; offset += 1) {
            if ((GameTurn)offset > current_packet->turn) {
                break;
            }
            const struct Packet *history_packet = get_history_packet((PlayerNumber)netstate.my_id, current_packet->turn - offset);
            if (history_packet == NULL) {
                continue;
            }
            memcpy(write_pos, history_packet, sizeof(struct Packet));
            write_pos += sizeof(struct Packet);
            *packet_count += 1;
        }
    } else {
        memcpy(write_pos, send_buf, frame_size);
        write_pos += frame_size;
    }
    size_t message_size = write_pos - netstate.msg_buffer;
    send_exchange_message(msg_type, message_size, INVALID_USER_ID);
    return Lb_OK;
}

TbError process_network_message(NetUserId source, void *server_buf, size_t frame_size, enum NetMessageType expected_frame_type, NetUserId *frame_peer_id)
{
    if (frame_peer_id != NULL) {
        *frame_peer_id = INVALID_USER_ID;
    }
    size_t message_size = netstate.sp->readmsg(source, netstate.msg_buffer, sizeof(netstate.msg_buffer));
    if (message_size == 0) {
        ERRORLOG("Problem reading message from %u", source);
        return Lb_FAIL;
    }
    char *read_pos = netstate.msg_buffer;
    enum NetMessageType message_type = (enum NetMessageType)read_pos[0];
    read_pos += 1;
    switch (message_type) {
    case NETMSG_LOGIN:
        return process_login_message(source, read_pos);
    case NETMSG_USERUPDATE:
        return process_user_update_message(source, read_pos);
    case NETMSG_FRONTEND:
    case NETMSG_STARTUP_SYNC:
    case NETMSG_GAMEPLAY_UNSEQUENCED:
        return handle_exchange_message(source, server_buf, frame_size, message_type, expected_frame_type, frame_peer_id, read_pos, message_size);
    case NETMSG_UNPAUSE:
        return process_network_unpause_message();
    case NETMSG_CHATMESSAGE:
        return handle_chat_message(source, read_pos, message_size, expected_frame_type);
    case NETMSG_GAMEPLAY_REPAIR:
        read_repair_packet_history(source, read_pos, message_size - (read_pos - netstate.msg_buffer));
        return Lb_OK;
    case NETMSG_GAMEPLAY_TURN_SYNC:
        return process_network_turn_sync_message(source, read_pos, message_size - (read_pos - netstate.msg_buffer));
    default:
        return Lb_OK;
    }
}

TbError exchange_frame_block(enum NetMessageType msg_type, void *send_buf, void *server_buf, size_t frame_size)
{
    TbBool frontend_exchange;
    if (msg_type == NETMSG_FRONTEND) {
        frontend_exchange = true;
    } else if (msg_type == NETMSG_STARTUP_SYNC) {
        frontend_exchange = false;
    } else {
        ERRORLOG("exchange_frame_block unsupported message type %d", (int)msg_type);
        return Lb_FAIL;
    }
    if (exchange_frame_message(send_buf, server_buf, frame_size, msg_type) != Lb_OK) {
        return Lb_FAIL;
    }

    const struct ScreenPacket *screen_packets = (const struct ScreenPacket *)server_buf;
    TbBool has_received_frame[MAX_NET_USERS] = {false};
    TbBool is_host = my_player_number == get_host_player_id();
    TbClockMSec wait_start_time = LbTimerClock();
    TbBool stop_waiting = false;
    while (LbTimerClock() - wait_start_time < TIMEOUT_LOBBY_EXCHANGE) {
        if (frontend_exchange && frontend_menu_state == FeSt_START_MPLEVEL) {
            break;
        }
        if (frontend_exchange && frame_size == sizeof(struct ScreenPacket)) {
            for (NetUserId peer_id = 0; peer_id < netstate.max_players; peer_id += 1) {
                const struct ScreenPacket *packet = &screen_packets[peer_id];
                if ((packet->networkstatus_flags & NetStat_PlayerConnected) == 0) {
                    continue;
                }
                unsigned char action = screen_packet_action(packet);
                if (action == NetAct_OpenLandView || (action == NetAct_HostStartLevel && packet->action_par1 > 0)) {
                    stop_waiting = true;
                    break;
                }
            }
        }
        for (int pass = 0; pass < 2 && !stop_waiting; pass += 1) {
            if (pass > 0) {
                netstate.sp->update(OnNewUser);
            }
            for (NetUserId peer_id = 0; peer_id < netstate.max_players && !stop_waiting; peer_id += 1) {
                if (!can_send_to_peer(peer_id)) {
                    continue;
                }
                while (netstate.sp->msgready(peer_id, 0)) {
                    NetUserId frame_peer_id;
                    process_network_message(peer_id, server_buf, frame_size, msg_type, &frame_peer_id);
                    if (frame_peer_id != INVALID_USER_ID) {
                        has_received_frame[frame_peer_id] = true;
                    }
                    if (frontend_exchange && frontend_menu_state == FeSt_START_MPLEVEL) {
                        stop_waiting = true;
                        break;
                    }
                }
            }
        }
        if (stop_waiting) {
            break;
        }
        if (all_expected_exchange_frames_received(has_received_frame, is_host)) {
            break;
        }
        if (frontend_exchange) {
            network_yield_draw_frontend();
        } else {
            network_yield_draw_gameplay();
        }
        SDL_Delay(1);
    }
    TbBool frames_received = all_expected_exchange_frames_received(has_received_frame, is_host);
    netstate.seq_nbr += 1;
    if (!frontend_exchange && !frames_received) {
        return Lb_FAIL;
    }
    return Lb_OK;
}

void process_peer_msgs(NetUserId peer_id, void *server_buf, size_t frame_size)
{
    while (netstate.sp->msgready(peer_id, 0)) {
        process_network_message(peer_id, server_buf, frame_size, NETMSG_GAMEPLAY_UNSEQUENCED, NULL);
    }
}

void wait_for_all_players(void)
{
    if (!network_is_active()) {
        return;
    }

    const TbClockMSec resend_interval = 50;
    TbBool has_received_frame[MAX_NET_USERS] = {false};
    TbBool is_host = (my_player_number == get_host_player_id());
    enum NetMessageType send_message_type;
    enum NetMessageType expected_message_type;
    if (is_host) {
        send_message_type = NETMSG_HOST_DECLARES_START;
        expected_message_type = NETMSG_CLIENT_IS_READY;
    } else {
        send_message_type = NETMSG_CLIENT_IS_READY;
        expected_message_type = NETMSG_HOST_DECLARES_START;
    }
    TbClockMSec wait_start_time = LbTimerClock();
    TbClockMSec last_send_time = wait_start_time - resend_interval;
    TbError result = Lb_FAIL;
    while (result != Lb_OK && LbTimerClock() - wait_start_time < TIMEOUT_WAIT_FOR_ALL_PLAYERS) {
        TbClockMSec now = LbTimerClock();
        if (now - last_send_time >= resend_interval) {
            if (!is_host || all_expected_exchange_frames_received(has_received_frame, is_host)) {
                netstate.msg_buffer[0] = send_message_type;
                if (!is_host) {
                    if (!can_send_to_peer(SERVER_ID)) {
                        ERRORLOG("Initial startup wait failed: could not send NETMSG_CLIENT_IS_READY packet");
                        return;
                    }
                    netstate.sp->sendmsg_single_unsequenced(SERVER_ID, netstate.msg_buffer, 1);
                } else {
                    send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, 1, netstate.my_id, INVALID_USER_ID);
                    result = Lb_OK;
                }
            }
            last_send_time = now;
        }
        netstate.sp->update(OnNewUser);
        for (NetUserId peer_id = 0; peer_id < netstate.max_players && result != Lb_OK; peer_id += 1) {
            if (!can_send_to_peer(peer_id)) {
                continue;
            }
            while (result != Lb_OK && netstate.sp->msgready(peer_id, 0)) {
                if (process_network_message(peer_id, NULL, 0, expected_message_type, NULL) != Lb_OK) {
                    ERRORLOG("Initial startup wait failed: could not process startup wait packet from peer %d", (int)peer_id);
                    return;
                }
                enum NetMessageType message_type = (enum NetMessageType)netstate.msg_buffer[0];
                if (message_type != expected_message_type) {
                    continue;
                }
                if (is_host) {
                    if (peer_id == SERVER_ID) {
                        WARNLOG("Peer %i sent invalid NETMSG_CLIENT_IS_READY", (int)peer_id);
                        continue;
                    }
                    has_received_frame[peer_id] = true;
                    continue;
                }
                if (peer_id != SERVER_ID) {
                    WARNLOG("Peer %i sent invalid NETMSG_HOST_DECLARES_START", (int)peer_id);
                    continue;
                }
                result = Lb_OK;
            }
        }
        if (result != Lb_OK) {
            SDL_Delay(1);
        }
    }
    netstate.seq_nbr += 1;
    if (result != Lb_OK) {
        ERRORLOG("Initial startup wait failed: TIMEOUT_WAIT_FOR_ALL_PLAYERS expired after %d ms", TIMEOUT_WAIT_FOR_ALL_PLAYERS);
    }
}
