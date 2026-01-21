/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_resync.cpp
 *     Network resynchronization for Dungeon Keeper multiplayer.
 * @par Purpose:
 *     Network resynchronization routines for multiplayer games.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     31 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_resync.h"
#include "bflib_network_internal.h"
#include "bflib_network.h"
#include "bflib_datetm.h"
#include "bflib_network_exchange.h"
#include <zlib.h>
#include "globals.h"
#include "frontend.h"
#include "player_data.h"
#include "net_game.h"
#include "game_legacy.h"
#include "lens_api.h"
#include "net_input_lag.h"
#include "net_received_packets.h"
#include "net_redundant_packets.h"
#include "net_checksums.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void draw_out_of_sync_box(long a1, long a2, long box_width);

struct Boing {
  unsigned char active_panel_menu_index;
  unsigned char comp_player_aggressive;
  unsigned char comp_player_defensive;
  unsigned char comp_player_construct;
  unsigned char comp_player_creatrsonly;
  unsigned char creatures_tend_imprison;
  unsigned char creatures_tend_flee;
  unsigned short hand_over_subtile_x;
  unsigned short hand_over_subtile_y;
  unsigned long chosen_room_kind;
  unsigned long chosen_room_spridx;
  unsigned long chosen_room_tooltip;
  unsigned long chosen_spell_type;
  unsigned long chosen_spell_spridx;
  unsigned long chosen_spell_tooltip;
  unsigned long manufactr_element;
  unsigned long manufactr_spridx;
  unsigned long manufactr_tooltip;
};

static struct Boing boing;

TbBool detailed_multiplayer_logging = false;

#define RESYNC_RECEIVE_TIMEOUT_MS 30000
#define RESYNC_TIMESYNC_TIMEOUT_MS 15000
static long g_timesync_offset_ms = 0;
static TbClockMSec g_client_rtt_ms = 0;

struct ResyncHeader {
    unsigned char message_type;
    unsigned int compressed_length;
    unsigned int original_length;
    unsigned int data_checksum;
};

struct ResumeMessage {
    unsigned char message_type;
    TbClockMSec resume_time;
};

struct TimeSyncComplete {
    unsigned char message_type;
    TbClockMSec client_rtt;
};

struct TimeSyncRequest {
    unsigned char message_type;
    TbClockMSec client_send_time;
};

struct TimeSyncResponse {
    unsigned char message_type;
    TbClockMSec client_send_time;
    TbClockMSec host_receive_time;
    TbClockMSec host_send_time;
};

void animate_resync_progress_bar(int current_phase, int total_phases) {
    if (game.play_gameturn == 0) {
        return;
    }
    if ((game.operation_flags & GOF_Paused) != 0) {
        return;
    }
    const long max_progress = (long)(32 * units_per_pixel / 16);
    const long progress_pixels = (long)((double)max_progress * current_phase / total_phases);
    draw_out_of_sync_box(progress_pixels, max_progress, status_panel_width);
}

void store_localised_game_structure(void) {
    boing.active_panel_menu_index = game.active_panel_mnu_idx;
    boing.comp_player_aggressive = game.comp_player_aggressive;
    boing.comp_player_defensive = game.comp_player_defensive;
    boing.comp_player_construct = game.comp_player_construct;
    boing.comp_player_creatrsonly = game.comp_player_creatrsonly;
    boing.creatures_tend_imprison = game.creatures_tend_imprison;
    boing.creatures_tend_flee = game.creatures_tend_flee;
    boing.hand_over_subtile_x = game.hand_over_subtile_x;
    boing.hand_over_subtile_y = game.hand_over_subtile_y;
    boing.chosen_room_kind = game.chosen_room_kind;
    boing.chosen_room_spridx = game.chosen_room_spridx;
    boing.chosen_room_tooltip = game.chosen_room_tooltip;
    boing.chosen_spell_type = game.chosen_spell_type;
    boing.chosen_spell_spridx = game.chosen_spell_spridx;
    boing.chosen_spell_tooltip = game.chosen_spell_tooltip;
    boing.manufactr_element = game.manufactr_element;
    boing.manufactr_spridx = game.manufactr_spridx;
    boing.manufactr_tooltip = game.manufactr_tooltip;
}

void recall_localised_game_structure(void) {
    game.active_panel_mnu_idx = boing.active_panel_menu_index;
    game.comp_player_aggressive = boing.comp_player_aggressive;
    game.comp_player_defensive = boing.comp_player_defensive;
    game.comp_player_construct = boing.comp_player_construct;
    game.comp_player_creatrsonly = boing.comp_player_creatrsonly;
    game.creatures_tend_imprison = boing.creatures_tend_imprison;
    game.creatures_tend_flee = boing.creatures_tend_flee;
    game.hand_over_subtile_x = boing.hand_over_subtile_x;
    game.hand_over_subtile_y = boing.hand_over_subtile_y;
    game.chosen_room_kind = boing.chosen_room_kind;
    game.chosen_room_spridx = boing.chosen_room_spridx;
    game.chosen_room_tooltip = boing.chosen_room_tooltip;
    game.chosen_spell_type = boing.chosen_spell_type;
    game.chosen_spell_spridx = boing.chosen_spell_spridx;
    game.chosen_spell_tooltip = boing.chosen_spell_tooltip;
    game.manufactr_element = boing.manufactr_element;
    game.manufactr_spridx = boing.manufactr_spridx;
    game.manufactr_tooltip = boing.manufactr_tooltip;
}

static TbBool send_resync_data(const void * buffer, size_t total_length) {
    MULTIPLAYER_LOG("Starting to send resync data: %lu bytes uncompressed", (unsigned long)total_length);
    
    uLong data_crc = crc32(0L, Z_NULL, 0);
    data_crc = crc32(data_crc, (const Bytef*)buffer, total_length);

    uLongf compressed_size = compressBound(total_length);
    char * compressed_buffer = (char *) malloc(compressed_size);
    if (!compressed_buffer) {
        ERRORLOG("Failed to allocate buffer for compression (bound: %lu bytes)", (unsigned long)compressed_size);
        return false;
    }

    int compress_result = compress((Bytef*)compressed_buffer, &compressed_size, (const Bytef*)buffer, total_length);
    if (compress_result != Z_OK) {
        ERRORLOG("Compression failed: zlib error %d", compress_result);
        free(compressed_buffer);
        return false;
    }

    NETLOG("Compression successful: %lu -> %lu bytes (ratio: %.2f%%)",
           (unsigned long)total_length, (unsigned long)compressed_size,
           ((double)compressed_size / total_length) * 100.0);

    ResyncHeader header;
    memset(&header, 0, sizeof(header));
    header.message_type = NETMSG_RESYNC_DATA;
    header.compressed_length = compressed_size;
    header.original_length = total_length;
    header.data_checksum = (unsigned int)data_crc;

    size_t message_size = sizeof(ResyncHeader) + compressed_size;
    char * message_buffer = (char *) malloc(message_size);
    if (!message_buffer) {
        ERRORLOG("Failed to allocate message buffer");
        free(compressed_buffer);
        return false;
    }

    memcpy(message_buffer, &header, sizeof(ResyncHeader));
    memcpy(message_buffer + sizeof(ResyncHeader), compressed_buffer, compressed_size);

    NETLOG("Host: Sending resync data to all clients");
    for (NetUserId user_index = 0; user_index < MAX_N_USERS; ++user_index) {
        if (netstate.users[user_index].progress != USER_LOGGEDIN) {
            continue;
        }
        netstate.sp->sendmsg_single(netstate.users[user_index].id, message_buffer, message_size);
    }

    free(compressed_buffer);
    free(message_buffer);
    return true;
}

static TbBool receive_resync_data(void * destination_buffer, size_t expected_total_length) {
    NETLOG("Starting to receive resync data, expecting %lu bytes", (unsigned long)expected_total_length);

    TbClockMSec start_time = LbTimerClock();
    while (LbTimerClock() - start_time < RESYNC_RECEIVE_TIMEOUT_MS) {
        netstate.sp->update(OnNewUser);
        size_t ready = netstate.sp->msgready(SERVER_ID, 0);
        if (ready == 0) {
            continue;
        }

        size_t max_message_size = sizeof(ResyncHeader) + compressBound(expected_total_length);
        char * message_buffer = (char *) malloc(max_message_size);
        if (!message_buffer) {
            ERRORLOG("Failed to allocate message buffer");
            return false;
        }

        size_t received_size = netstate.sp->readmsg(SERVER_ID, message_buffer, max_message_size);
        if (received_size < sizeof(ResyncHeader)) {
            ERRORLOG("Received message too small: %lu bytes", (unsigned long)received_size);
            free(message_buffer);
            continue;
        }

        ResyncHeader header;
        memcpy(&header, message_buffer, sizeof(ResyncHeader));

        if (header.message_type != NETMSG_RESYNC_DATA) {
            MULTIPLAYER_LOG("Received wrong message type: %d", header.message_type);
            free(message_buffer);
            continue;
        }

        if (header.original_length != expected_total_length) {
            ERRORLOG("Received data with wrong size: %lu != %lu", (unsigned long)header.original_length, (unsigned long)expected_total_length);
            free(message_buffer);
            continue;
        }

        if (received_size != sizeof(ResyncHeader) + header.compressed_length) {
            ERRORLOG("Received message size mismatch: %lu != %lu + %lu",
                   (unsigned long)received_size, (unsigned long)sizeof(ResyncHeader), (unsigned long)header.compressed_length);
            free(message_buffer);
            continue;
        }

        MULTIPLAYER_LOG("Client: Received resync message, decompressing %lu bytes", (unsigned long)header.compressed_length);

        uLongf dest_len = expected_total_length;
        int uncompress_result = uncompress((Bytef*)destination_buffer, &dest_len,
                                           (const Bytef*)(message_buffer + sizeof(ResyncHeader)),
                                           header.compressed_length);

        if (uncompress_result != Z_OK || dest_len != expected_total_length) {
            ERRORLOG("Decompression failed: zlib error %d, expected %lu bytes, got %lu bytes",
                   uncompress_result, (unsigned long)expected_total_length, (unsigned long)dest_len);
            free(message_buffer);
            return false;
        }

        uLong verify_crc = crc32(0L, Z_NULL, 0);
        verify_crc = crc32(verify_crc, (const Bytef*)destination_buffer, expected_total_length);
        if ((unsigned long)verify_crc != header.data_checksum) {
            ERRORLOG("Resync data checksum mismatch");
            free(message_buffer);
            return false;
        }

        free(message_buffer);
        NETLOG("Client: Resync data received successfully");
        return true;
    }

    ERRORLOG("Client: Timeout waiting for resync data after %dms", RESYNC_RECEIVE_TIMEOUT_MS);
    return false;
}

TbBool LbNetwork_Resync(void * data_buffer, size_t buffer_length) {
    MULTIPLAYER_LOG("Starting resync, my_id=%d, buffer_length=%lu", netstate.my_id, (unsigned long)buffer_length);
    const TbBool is_host = (my_player_number == get_host_player_id());

    TbBool result;
    if (is_host) {
        MULTIPLAYER_LOG("Resync: I am the server");
        result = send_resync_data(data_buffer, buffer_length);
    } else {
        MULTIPLAYER_LOG("Resync: I am a client, receiving");
        result = receive_resync_data(data_buffer, buffer_length);
    }

    if (!result) {
        ERRORLOG("Resync FAILED");
        return false;
    }

    MULTIPLAYER_LOG("Resync data transfer completed successfully");
    return true;
}

TbBool send_resync_game(void) {
  pack_desync_history_for_resync();
  clear_flag(game.operation_flags, GOF_Paused);
  animate_resync_progress_bar(0, 6);
  NETLOG("Initiating re-synchronization of network game");
  TbBool result = LbNetwork_Resync(&game, sizeof(game));
  if (!result) {
    return false;
  }
  animate_resync_progress_bar(2, 6);
  LbNetwork_TimesyncBarrier();
  animate_resync_progress_bar(6, 6);
  NETLOG("Host: Resync complete");
  return true;
}

TbBool receive_resync_game(void) {
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 6);
    NETLOG("Initiating re-synchronization of network game");
    TbBool result = LbNetwork_Resync(&game, sizeof(game));
    if (!result) {
        return false;
    }
    animate_resync_progress_bar(2, 6);
    LbNetwork_TimesyncBarrier();
    animate_resync_progress_bar(6, 6);
    NETLOG("Client: Resync complete");

    compare_desync_history_from_host();

    return true;
}


void resync_game(void) {
    SYNCDBG(2,"Starting");
    struct PlayerInfo* player = get_my_player();
    draw_out_of_sync_box(0, 32*units_per_pixel/16, player->engine_window_x);
    reset_eye_lenses();
    store_localised_game_structure();
    if (my_player_number == get_host_player_id()) {
        send_resync_game();
    } else {
        receive_resync_game();
    }
    recall_localised_game_structure();
    reinit_level_after_load();

    game.skip_initial_input_turns = calculate_skip_input();
    clear_packet_tracking();
    clear_redundant_packets();
    clear_input_lag_queue();
    NETLOG("Input lag after resync: %d turns", game.input_lag_turns);

    clear_flag(game.system_flags, GSF_NetGameNoSync);
    clear_flag(game.system_flags, GSF_NetSeedNoSync);
}

void LbNetwork_TimesyncBarrier(void) {
    if (game.game_kind != GKind_MultiGame) {
        return;
    }
    const TbBool is_host = (my_player_number == get_host_player_id());
    if (is_host) {
        MULTIPLAYER_LOG("Host: Handling any pending timesync requests");
        TbBool timesynced[MAX_N_USERS];
        for (NetUserId i = 0; i < MAX_N_USERS; ++i) {
            if (i == netstate.my_id) {
                timesynced[i] = 1;
            } else if (netstate.users[i].progress == USER_LOGGEDIN) {
                timesynced[i] = 0;
            } else {
                timesynced[i] = 1;
            }
        }
        g_client_rtt_ms = 0;
        TbClockMSec wait_start = LbTimerClock();
        TbClockMSec last_anim_time = wait_start;
        while (LbTimerClock() - wait_start < RESYNC_TIMESYNC_TIMEOUT_MS) {
            netstate.sp->update(OnNewUser);
            for (NetUserId id = 0; id < MAX_N_USERS; ++id) {
                if (id == netstate.my_id) continue;
                if (netstate.users[id].progress != USER_LOGGEDIN) continue;
                while (netstate.sp->msgready(id, 0)) {
                    size_t sz = netstate.sp->readmsg(id, netstate.msg_buffer, sizeof(netstate.msg_buffer));
                    if (sz >= sizeof(TimeSyncRequest) && netstate.msg_buffer[0] == NETMSG_TIMESYNC_REQUEST) {
                        TimeSyncRequest req;
                        memcpy(&req, netstate.msg_buffer, sizeof(TimeSyncRequest));
                        TbClockMSec receive_time = LbTimerClock();
                        TimeSyncResponse rsp;
                        rsp.message_type = NETMSG_TIMESYNC_REPLY;
                        rsp.client_send_time = req.client_send_time;
                        rsp.host_receive_time = receive_time;
                        rsp.host_send_time = LbTimerClock();
                        netstate.sp->sendmsg_single(netstate.users[id].id, (const char *)&rsp, sizeof(rsp));
                    } else if (sz >= sizeof(TimeSyncComplete) && netstate.msg_buffer[0] == NETMSG_TIMESYNC_COMPLETE) {
                        TimeSyncComplete complete;
                        memcpy(&complete, netstate.msg_buffer, sizeof(TimeSyncComplete));
                        g_client_rtt_ms = complete.client_rtt;
                        MULTIPLAYER_LOG("Host: Received RTT from client %d: %lu ms", id, (unsigned long)g_client_rtt_ms);
                        timesynced[id] = 1;
                    }
                }
            }
            TbBool all_done = 1;
            for (NetUserId id = 0; id < MAX_N_USERS; ++id) {
                if (!timesynced[id]) {
                    all_done = 0;
                    break;
                }
            }
            if (all_done) {
                break;
            }
            TbClockMSec now = LbTimerClock();
            if (now - last_anim_time >= 100) {
                animate_resync_progress_bar(3, 6);
                last_anim_time = now;
            }
        }
        LbSleepFor(100);
        for (NetUserId id = 0; id < MAX_N_USERS; ++id) {
            if (id == netstate.my_id) continue;
            if (netstate.users[id].progress != USER_LOGGEDIN) continue;
            if (!timesynced[id]) {
                ERRORLOG("Host: Timesync timeout for client index %d, dropping", id);
                netstate.sp->drop_user(id);
            }
        }
        TbClockMSec current_time = LbTimerClock();
        TbClockMSec delay = 500;
        if (g_client_rtt_ms > 0) {
            delay = g_client_rtt_ms * 3;
            if (delay < 100) {
                delay = 100;
            }
            if (delay > 1000) {
                delay = 1000;
            }
        }
        TbClockMSec resume_time = current_time + delay;
        ResumeMessage message;
        message.message_type = NETMSG_RESYNC_RESUME;
        message.resume_time = resume_time;
        MULTIPLAYER_LOG("Host: Broadcasting RESUME to all clients (current=%lu, resume=%lu, delay=%lu, client_rtt=%lu)", (unsigned long)current_time, (unsigned long)resume_time, (unsigned long)delay, (unsigned long)g_client_rtt_ms);
        for (NetUserId user_index = 0; user_index < MAX_N_USERS; ++user_index) {
            if (netstate.users[user_index].progress != USER_LOGGEDIN) {
                continue;
            }
            netstate.sp->sendmsg_single(netstate.users[user_index].id, (char*)&message, sizeof(ResumeMessage));
        }
        TbClockMSec now = LbTimerClock();
        if (resume_time > now) {
            TbClockMSec wait_delay = resume_time - now;
            MULTIPLAYER_LOG("Current time %lu, waiting until %lu (delay %lu ms)", (unsigned long)now, (unsigned long)resume_time, (unsigned long)wait_delay);
            TbClockMSec end_time = now + wait_delay;
            if (wait_delay > 10) {
                TbClockMSec sleep_until = end_time - 5;
                while (LbTimerClock() < sleep_until) {
                    LbSleepFor(1);
                }
            }
            while (LbTimerClock() < end_time) {
            }
        }
        MULTIPLAYER_LOG("Host: Timesync barrier complete");
    } else {
        TbClockMSec t1 = LbTimerClock();
        TimeSyncRequest req;
        req.message_type = NETMSG_TIMESYNC_REQUEST;
        req.client_send_time = t1;
        netstate.sp->sendmsg_single(SERVER_ID, (const char *)&req, sizeof(req));
        MULTIPLAYER_LOG("Client: Sent TIMESYNC request at t1=%lu", (unsigned long)t1);
        TbClockMSec start = LbTimerClock();
        TbClockMSec last_anim_time = start;
        while (LbTimerClock() - start < RESYNC_TIMESYNC_TIMEOUT_MS) {
            netstate.sp->update(OnNewUser);
            while (netstate.sp->msgready(SERVER_ID, 0)) {
                size_t got = netstate.sp->readmsg(SERVER_ID, netstate.msg_buffer, sizeof(netstate.msg_buffer));
                TbClockMSec t4 = LbTimerClock();
                if (got >= sizeof(TimeSyncResponse)) {
                    TimeSyncResponse rsp;
                    memcpy(&rsp, netstate.msg_buffer, sizeof(TimeSyncResponse));
                    if (rsp.message_type == NETMSG_TIMESYNC_REPLY) {
                        TbClockMSec rtt = t4 - rsp.client_send_time;
                        long long offset = ((long long)rsp.host_receive_time - (long long)rsp.client_send_time + (long long)rsp.host_send_time - (long long)t4) / 2;
                        g_timesync_offset_ms = (long)offset;
                        MULTIPLAYER_LOG("Client: TIMESYNC complete: t1=%lld t2=%lld t3=%lld t4=%lld rtt=%lu offset=%ld ms (host ahead)", (long long)rsp.client_send_time, (long long)rsp.host_receive_time, (long long)rsp.host_send_time, (long long)t4, (unsigned long)rtt, g_timesync_offset_ms);
                        TimeSyncComplete complete;
                        complete.message_type = NETMSG_TIMESYNC_COMPLETE;
                        complete.client_rtt = rtt;
                        netstate.sp->sendmsg_single(SERVER_ID, (const char *)&complete, sizeof(complete));
                        goto timesync_done;
                    }
                }
            }
            TbClockMSec now = LbTimerClock();
            if (now - last_anim_time >= 100) {
                animate_resync_progress_bar(3, 6);
                last_anim_time = now;
            }
        }
        ERRORLOG("Client: TIMESYNC timeout");
        LbNetwork_Stop();
        return;
timesync_done:
        MULTIPLAYER_LOG("Client: Waiting for RESUME message from host");
        TbClockMSec start_time = LbTimerClock();
        last_anim_time = start_time;
        TbClockMSec resume_time = 0;
        TbBool received = false;
        while (LbTimerClock() - start_time < 10000) {
            netstate.sp->update(OnNewUser);
            while (netstate.sp->msgready(SERVER_ID, 0)) {
                size_t sz = netstate.sp->readmsg(SERVER_ID, netstate.msg_buffer, sizeof(netstate.msg_buffer));
                if (sz >= sizeof(ResumeMessage)) {
                    ResumeMessage msg;
                    memcpy(&msg, netstate.msg_buffer, sizeof(ResumeMessage));
                    if (msg.message_type == NETMSG_RESYNC_RESUME) {
                        long long host_time = (long long)msg.resume_time;
                        long long offset = (long long)g_timesync_offset_ms;
                        long long local_time = host_time - offset;
                        if (local_time < 0) {
                            local_time = 0;
                        }
                        resume_time = (TbClockMSec)local_time;
                        MULTIPLAYER_LOG("Client: Received RESUME from host, host_time=%lld offset=%lld local_time=%lld", host_time, offset, local_time);
                        received = true;
                        break;
                    }
                }
            }
            if (received) {
                break;
            }
            TbClockMSec now = LbTimerClock();
            if (now - last_anim_time >= 100) {
                animate_resync_progress_bar(4, 6);
                last_anim_time = now;
            }
        }
        if (!received) {
            ERRORLOG("Client: Timeout waiting for RESUME from host");
            return;
        }
        TbClockMSec now = LbTimerClock();
        if (resume_time > now) {
            TbClockMSec wait_delay = resume_time - now;
            MULTIPLAYER_LOG("Current time %lu, waiting until %lu (delay %lu ms)", (unsigned long)now, (unsigned long)resume_time, (unsigned long)wait_delay);
            TbClockMSec end_time = now + wait_delay;
            if (wait_delay > 10) {
                TbClockMSec sleep_until = end_time - 5;
                while (LbTimerClock() < sleep_until) {
                    LbSleepFor(1);
                }
            }
            while (LbTimerClock() < end_time) {
            }
        }
        MULTIPLAYER_LOG("Client: Timesync barrier complete");
    }
}

#ifdef __cplusplus
}
#endif
