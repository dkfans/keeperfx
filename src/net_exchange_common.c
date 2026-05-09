/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_exchange_common.c
 *     Shared multiplayer message exchange for Dungeon Keeper.
 * @par Purpose:
 *     Relay, dispatch and synchronized frame exchange routines.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_exchange_common.h"

#include "bflib_datetm.h"
#include "front_landview.h"
#include "game_legacy.h"
#include "globals.h"
#include "keeperfx.hpp"
#include "net_game.h"
#include "net_lobby.h"
#include "net_main.h"
#include "net_received_packets.h"
#include "net_redundant_packets.h"
#include "packets.h"
#include "player_data.h"
#include "post_inc.h"

extern void network_yield_draw_gameplay(void);
extern void network_yield_draw_frontend(void);
extern long double last_draw_completed_time;
extern void LbNetwork_TimesyncBarrier(void);
extern TbBool keeper_screen_redraw(void);
extern TbResult LbScreenSwap(void);
/******************************************************************************/

#define NETWORK_FPS 60

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

TbBool can_send_to_peer(NetUserId peer_id)
{
    return (peer_id != netstate.my_id) &&
        (netstate.users[peer_id].progress != USER_UNUSED) &&
        (my_player_number == get_host_player_id() || peer_id == SERVER_ID);
}

static void send_frame_to_peers(NetUserId source_id, const void *send_buf, size_t buf_size, int seq_nbr, enum NetMessageType msg_type)
{
    char *ptr = begin_net_message(msg_type);
    *ptr = source_id;
    ptr += 1;
    *(int *)ptr = seq_nbr;
    ptr += 4;
    if (msg_type == NETMSG_GAMEPLAY) {
        size_t bundled_size = bundle_packets((PlayerNumber)source_id, (const struct Packet *)send_buf, ptr);
        ptr += bundled_size;
    } else {
        memcpy(ptr, send_buf, buf_size);
        ptr += buf_size;
    }
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (id == source_id) {
            continue;
        }
        if (!IsUserActive(id)) {
            continue;
        }
        if (msg_type == NETMSG_GAMEPLAY) {
            netstate.sp->sendmsg_single_unsequenced(id, netstate.msg_buffer, ptr - netstate.msg_buffer);
            netstate.sp->sendmsg_single(id, netstate.msg_buffer, ptr - netstate.msg_buffer);
        } else {
            send_message_buffer(id, ptr);
        }
    }
    if (msg_type == NETMSG_GAMEPLAY) {
        store_sent_packet((PlayerNumber)source_id, (const struct Packet *)send_buf);
    }
}

static TbError process_frame_message(enum NetMessageType type, char *read_pos, void *server_buf, size_t frame_size)
{
    NetUserId peer_id = (NetUserId)*read_pos;
    if (peer_id < 0 || peer_id >= netstate.max_players) {
        ERRORLOG("Critical error: Out of range peer ID %i received, could be used for buffer overflow attack", peer_id);
        abort();
    }
    read_pos += 1;
    netstate.users[peer_id].ack = *(int *)read_pos;
    read_pos += 4;
    char *peer_buf = ((char *)server_buf) + peer_id * frame_size;
    if (type == NETMSG_GAMEPLAY) {
        struct BundledPacket *bundled = (struct BundledPacket *)read_pos;
        memcpy(peer_buf, &bundled->packets[0], frame_size);
        unbundle_packets(read_pos, (PlayerNumber)peer_id);
    } else {
        memcpy(peer_buf, read_pos, frame_size);
    }
    return Lb_OK;
}

static TbError process_unpause_message(void)
{
    if ((game.operation_flags & GOF_Paused) == 0) {
        MULTIPLAYER_LOG("process_network_message NETMSG_UNPAUSE: ignoring, not paused");
        return Lb_OK;
    }
    MULTIPLAYER_LOG("process_network_message NETMSG_UNPAUSE: initiating timesync");
    unpausing_in_progress = 1;
    keeper_screen_redraw();
    LbScreenSwap();
    if (my_player_number == get_host_player_id()) {
        LbNetwork_BroadcastUnpauseTimesync();
    }
    LbNetwork_TimesyncBarrier();
    process_pause_packet(0, 0);
    unpausing_in_progress = 0;
    return Lb_OK;
}

static TbError process_chat_message(NetUserId source, char *read_pos, size_t message_size)
{
    int player_id = (int)*read_pos;
    read_pos += 1;
    const char *message;
    if (!read_network_message_text(&read_pos, &message, sizeof(netstate.msg_buffer) - 1)) {
        ERRORLOG("Chat message too long or not null-terminated");
        return Lb_OK;
    }
    process_chat_message_end(player_id, message);
    if (netstate.my_id == SERVER_ID && source != SERVER_ID) {
        for (NetUserId id = 0; id < netstate.max_players; id += 1) {
            if (id == netstate.my_id || id == source || !IsUserActive(id)) {
                continue;
            }
            netstate.sp->sendmsg_single(id, netstate.msg_buffer, message_size);
        }
    }
    return Lb_OK;
}

TbError process_network_message(NetUserId source, void *server_buf, size_t frame_size)
{
    size_t message_size = netstate.sp->readmsg(source, netstate.msg_buffer, sizeof(netstate.msg_buffer));
    if (message_size <= 0) {
        ERRORLOG("Problem reading message from %u", source);
        return Lb_FAIL;
    }
    char *read_pos = netstate.msg_buffer;
    enum NetMessageType type = (enum NetMessageType)*read_pos;
    read_pos += 1;
    switch (type) {
    case NETMSG_LOGIN:
        return process_login_message(source, read_pos);
    case NETMSG_USERUPDATE:
        return process_user_update_message(source, read_pos);
    case NETMSG_FRONTEND:
    case NETMSG_STARTUP_SYNC:
    case NETMSG_GAMEPLAY:
        return process_frame_message(type, read_pos, server_buf, frame_size);
    case NETMSG_UNPAUSE:
        return process_unpause_message();
    case NETMSG_CHATMESSAGE:
        return process_chat_message(source, read_pos, message_size);
    default:
        return Lb_OK;
    }
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

TbError LbNetwork_Exchange(enum NetMessageType msg_type, void *send_buf, void *server_buf, size_t client_frame_size)
{
    if (netstate.my_id < 0 || netstate.my_id >= netstate.max_players) {
        ERRORLOG("Invalid my_id %i in LbNetwork_Exchange (disconnected?)", netstate.my_id);
        return Lb_FAIL;
    }
    netstate.sp->update(OnNewUser);
    memcpy(((char *)server_buf) + netstate.my_id * client_frame_size, send_buf, client_frame_size);
    send_frame_to_peers(netstate.my_id, send_buf, client_frame_size, netstate.seq_nbr, msg_type);

    long double draw_interval_nanoseconds = 1000000000.0 / NETWORK_FPS;
    if (msg_type == NETMSG_FRONTEND) {
        draw_interval_nanoseconds = 0;
    }
    int timeout_max = TIMEOUT_LOBBY_EXCHANGE;
    if (msg_type == NETMSG_GAMEPLAY) {
        timeout_max = (1000 / turns_per_second);
    }

    for (NetUserId peer_id = 0; peer_id < netstate.max_players; peer_id += 1) {
        if (!can_send_to_peer(peer_id)) {
            continue;
        }

        TbClockMSec start = LbTimerClock();
        while (true) {
            int elapsed = LbTimerClock() - start;
            if (elapsed >= timeout_max) {
                break;
            }
            if (netstate.users[peer_id].progress == USER_UNUSED) {
                break;
            }

            long long time_since_draw_nanoseconds = get_time_tick_ns() - last_draw_completed_time;
            int remaining_time_until_draw = (int)((draw_interval_nanoseconds - time_since_draw_nanoseconds) / 1000000.0);
            if (remaining_time_until_draw < 0) {
                remaining_time_until_draw = 0;
            }
            int wait = min(timeout_max - elapsed, remaining_time_until_draw);

            if (netstate.sp->msgready(peer_id, wait)) {
                process_network_message(peer_id, server_buf, client_frame_size);
                if (msg_type != NETMSG_GAMEPLAY) {
                    if (msg_type != NETMSG_STARTUP_SYNC || netstate.msg_buffer[0] == msg_type) {
                        break;
                    }
                    continue;
                }
                TbBool received_gameplay_msg = (netstate.msg_buffer[0] == NETMSG_GAMEPLAY);
                while (netstate.sp->msgready(peer_id, 0)) {
                    process_network_message(peer_id, server_buf, client_frame_size);
                    if (netstate.msg_buffer[0] == NETMSG_GAMEPLAY) {
                        received_gameplay_msg = true;
                    }
                }
                if (received_gameplay_msg) {
                    break;
                }
            }

            if (LbTimerClock() - start < timeout_max) {
                if (msg_type == NETMSG_FRONTEND) {
                    network_yield_draw_frontend();
                } else {
                    network_yield_draw_gameplay();
                }
            }
        }
    }
    netstate.seq_nbr += 1;
    return Lb_OK;
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
