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
#include "net_received_packets.h"
#include "net_redundant_packets.h"
#include "packets.h"
#include "player_data.h"
#include "post_inc.h"

extern void network_yield_draw_gameplay(void);
/******************************************************************************/

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
