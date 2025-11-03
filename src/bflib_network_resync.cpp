/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_network_resync.cpp
 *     Network resynchronization support.
 * @par Purpose:
 *     Network resynchronization routines.
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
#include "bflib_network_resync.h"
#include "bflib_network_internal.h"
#include "bflib_network.h"
#include "bflib_datetm.h"
#include <zlib.h>
#include "globals.h"
#include "frontend.h"
#include "player_data.h"
#include "game_legacy.h"
#include "input_lag.h"
#include "received_packets.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void draw_out_of_sync_box(long a1, long a2, long box_width);

#define RESYNC_RECEIVE_TIMEOUT_MS 30000
#define RESYNC_TIMESYNC_TIMEOUT_MS 15000
static long g_timesync_offset_ms = 0;

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

static void animate_resync_progress_bar(void) {
    if (game.play_gameturn == 0) {
        return;
    }
    const long max_progress = (long)(32 * units_per_pixel / 16);
    const long progress_pixels = (long)((double)max_progress * (LbTimerClock() % 1000) / 1000);
    draw_out_of_sync_box(progress_pixels, max_progress, status_panel_width);
}

static void host_handle_timesync_request(NetUserId src_id, const char *buf, size_t sz);
static TbBool client_timesync_with_host(void);
static void host_process_pending_timesync_requests(TbClockMSec duration_ms);

void LbNetwork_ClientSendReadyMessage(void) {
    char message = NETMSG_RESYNC_READY;
    netstate.sp->sendmsg_single(SERVER_ID, &message, 1);
    MULTIPLAYER_LOG("Client: Sent READY to host");
}

static void wait_for_delay(TbClockMSec delay_ms) {
    TbClockMSec start_time = LbTimerClock();
    TbClockMSec end_time = start_time + delay_ms;
    MULTIPLAYER_LOG("Waiting %lu ms (from %lu to %lu)", (unsigned long)delay_ms, (unsigned long)start_time, (unsigned long)end_time);
    while (true) {
        if (LbTimerClock() >= end_time) {
            break;
        }
    }
    MULTIPLAYER_LOG("Wait complete at %lu", (unsigned long)LbTimerClock());
}

void LbNetwork_WaitUntilResumeTime(TbClockMSec resume_time) {
    TbClockMSec now = LbTimerClock();
    if (resume_time > now) {
        TbClockMSec delay = resume_time - now;
        MULTIPLAYER_LOG("Current time %lu, waiting until %lu (delay %lu ms)", (unsigned long)now, (unsigned long)resume_time, (unsigned long)delay);
        wait_for_delay(delay);
    } else {
        MULTIPLAYER_LOG("Resume time %lu already passed (current %lu), resuming immediately", (unsigned long)resume_time, (unsigned long)now);
    }
}

TbBool LbNetwork_HostWaitForAllClientsReady(void) {
    MULTIPLAYER_LOG("Host: Waiting for all clients to send READY");
    TbBool ready_flags[MAX_N_USERS];
    for (NetUserId i = 0; i < MAX_N_USERS; ++i) {
        if (netstate.users[i].progress == USER_LOGGEDIN) {
            ready_flags[i] = 0;
        } else {
            ready_flags[i] = 1;
        }
    }

    TbClockMSec start_time = LbTimerClock();
    while (LbTimerClock() - start_time < 10000) {
        animate_resync_progress_bar();
        netstate.sp->update(OnNewUser);
        for (NetUserId id = 0; id < MAX_N_USERS; ++id) {
            if (id == netstate.my_id) continue;
            if (netstate.users[id].progress != USER_LOGGEDIN) continue;
            while (netstate.sp->msgready(id, 0)) {
                size_t sz = netstate.sp->readmsg(id, netstate.msg_buffer, sizeof(netstate.msg_buffer));
                if (sz > 0 && netstate.msg_buffer[0] == NETMSG_RESYNC_READY) {
                    MULTIPLAYER_LOG("Host: Received READY from client index %d", id);
                    ready_flags[id] = 1;
                }
            }
        }
        TbBool all_ready = true;
        for (NetUserId id = 0; id < MAX_N_USERS; ++id) {
            if (!ready_flags[id]) {
                all_ready = false;
                break;
            }
        }
        if (all_ready) {
            MULTIPLAYER_LOG("Host: All clients ready");
            return true;
        }
    }
    ERRORLOG("Host: Timeout waiting for clients to be READY");
    return false;
}

static void host_process_pending_timesync_requests(TbClockMSec duration_ms) {
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
    TbClockMSec wait_start = LbTimerClock();
    while (LbTimerClock() - wait_start < duration_ms) {
        animate_resync_progress_bar();
        netstate.sp->update(OnNewUser);
        for (NetUserId id = 0; id < MAX_N_USERS; ++id) {
            if (id == netstate.my_id) continue;
            if (netstate.users[id].progress != USER_LOGGEDIN) continue;
            while (netstate.sp->msgready(id, 0)) {
                size_t sz = netstate.sp->readmsg(id, netstate.msg_buffer, sizeof(netstate.msg_buffer));
                if (sz > 0 && netstate.msg_buffer[0] == NETMSG_TIMESYNC_REQUEST) {
                    host_handle_timesync_request(id, netstate.msg_buffer, sz);
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
    }
    LbSleepFor(100);
    for (NetUserId id = 0; id < MAX_N_USERS; ++id) {
        if (id == netstate.my_id) continue;
        if (netstate.users[id].progress != USER_LOGGEDIN) continue;
        if (!timesynced[id]) {
            MULTIPLAYER_LOG("Host: Timesync timeout for client index %d, dropping", id);
            netstate.sp->drop_user(id);
        }
    }
}

TbClockMSec LbNetwork_HostSendResumeToAllClients(void) {
    TbClockMSec current_time = LbTimerClock();
    TbClockMSec host_resume_time = current_time + 500;
    TbClockMSec client_resume_time = host_resume_time;
    ResumeMessage message;
    message.message_type = NETMSG_RESYNC_RESUME;
    message.resume_time = client_resume_time;
    MULTIPLAYER_LOG("Host: Broadcasting RESUME to all clients (current=%lu, resume=%lu)", (unsigned long)current_time, (unsigned long)host_resume_time);
    for (NetUserId user_index = 0; user_index < MAX_N_USERS; ++user_index) {
        if (netstate.users[user_index].progress != USER_LOGGEDIN) {
            continue;
        }
        netstate.sp->sendmsg_single(netstate.users[user_index].id, (char*)&message, sizeof(ResumeMessage));
    }
    return host_resume_time;
}

TbBool LbNetwork_ClientWaitForResumeMessage(TbClockMSec *resume_time) {
    MULTIPLAYER_LOG("Client: Waiting for RESUME message from host");
    TbClockMSec start_time = LbTimerClock();
    while (LbTimerClock() - start_time < 10000) {
        animate_resync_progress_bar();
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
                    *resume_time = (TbClockMSec)local_time;
                    MULTIPLAYER_LOG("Client: Received RESUME from host, host_time=%lld offset=%lld local_time=%lld", host_time, offset, local_time);
                    return true;
                }
            }
        }
    }
    ERRORLOG("Client: Timeout waiting for RESUME from host");
    return false;
}

static void host_handle_timesync_request(NetUserId src_id, const char *buf, size_t sz) {
    if (sz < sizeof(TimeSyncRequest)) {
        return;
    }
    TimeSyncRequest req;
    memcpy(&req, buf, sizeof(TimeSyncRequest));
    if (req.message_type != NETMSG_TIMESYNC_REQUEST) {
        return;
    }
    TbClockMSec receive_time = LbTimerClock();
    TimeSyncResponse rsp;
    rsp.message_type = NETMSG_TIMESYNC_REPLY;
    rsp.client_send_time = req.client_send_time;
    rsp.host_receive_time = receive_time;
    rsp.host_send_time = LbTimerClock();
    netstate.sp->sendmsg_single(netstate.users[src_id].id, (const char *)&rsp, sizeof(rsp));
}

static TbBool client_timesync_with_host(void) {
    TbClockMSec t1 = LbTimerClock();
    TimeSyncRequest req;
    req.message_type = NETMSG_TIMESYNC_REQUEST;
    req.client_send_time = t1;
    netstate.sp->sendmsg_single(SERVER_ID, (const char *)&req, sizeof(req));
    MULTIPLAYER_LOG("Client: Sent TIMESYNC request at t1=%lu", (unsigned long)t1);
    TbClockMSec start = LbTimerClock();
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
                    MULTIPLAYER_LOG("Client: TIMESYNC complete: t1=%lld t2=%lld t3=%lld t4=%lld rtt=%lu offset=%ld ms (host ahead)",
                        (long long)rsp.client_send_time, (long long)rsp.host_receive_time, (long long)rsp.host_send_time, (long long)t4, (unsigned long)rtt, g_timesync_offset_ms);
                    return true;
                }
            }
        }
        animate_resync_progress_bar();
    }
    ERRORLOG("Client: TIMESYNC timeout");
    return false;
}

static TbBool send_resync_data(const void * buffer, size_t total_length) {
    NETLOG("Starting to send resync data: %lu bytes uncompressed", (unsigned long)total_length);

    uLong data_crc = crc32(0L, Z_NULL, 0);
    data_crc = crc32(data_crc, (const Bytef*)buffer, total_length);

    uLongf compressed_size = compressBound(total_length);
    char * compressed_buffer = (char *) malloc(compressed_size);
    if (!compressed_buffer) {
        NETLOG("Failed to allocate buffer for compression (bound: %lu bytes)", (unsigned long)compressed_size);
        return false;
    }

    int compress_result = compress((Bytef*)compressed_buffer, &compressed_size, (const Bytef*)buffer, total_length);
    if (compress_result != Z_OK) {
        NETLOG("Compression failed: zlib error %d", compress_result);
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
        NETLOG("Failed to allocate message buffer");
        free(compressed_buffer);
        return false;
    }

    memcpy(message_buffer, &header, sizeof(ResyncHeader));
    memcpy(message_buffer + sizeof(ResyncHeader), compressed_buffer, compressed_size);

    MULTIPLAYER_LOG("Host: Sending resync data to all clients");
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
        animate_resync_progress_bar();
        netstate.sp->update(OnNewUser);
        size_t ready = netstate.sp->msgready(SERVER_ID, 0);
        if (ready == 0) {
            continue;
        }

        size_t max_message_size = sizeof(ResyncHeader) + compressBound(expected_total_length);
        char * message_buffer = (char *) malloc(max_message_size);
        if (!message_buffer) {
            NETLOG("Failed to allocate message buffer");
            return false;
        }

        size_t received_size = netstate.sp->readmsg(SERVER_ID, message_buffer, max_message_size);
        if (received_size < sizeof(ResyncHeader)) {
            NETLOG("Received message too small: %lu bytes", (unsigned long)received_size);
            free(message_buffer);
            continue;
        }

        ResyncHeader header;
        memcpy(&header, message_buffer, sizeof(ResyncHeader));

        if (header.message_type != NETMSG_RESYNC_DATA) {
            NETLOG("Received wrong message type: %d", header.message_type);
            free(message_buffer);
            continue;
        }

        if (header.original_length != expected_total_length) {
            NETLOG("Received data with wrong size: %lu != %lu", (unsigned long)header.original_length, (unsigned long)expected_total_length);
            free(message_buffer);
            continue;
        }

        if (received_size != sizeof(ResyncHeader) + header.compressed_length) {
            NETLOG("Received message size mismatch: %lu != %lu + %lu",
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
            NETLOG("Decompression failed: zlib error %d, expected %lu bytes, got %lu bytes",
                   uncompress_result, (unsigned long)expected_total_length, (unsigned long)dest_len);
            free(message_buffer);
            return false;
        }

        uLong verify_crc = crc32(0L, Z_NULL, 0);
        verify_crc = crc32(verify_crc, (const Bytef*)destination_buffer, expected_total_length);
        if ((unsigned long)verify_crc != header.data_checksum) {
            NETLOG("Resync data checksum mismatch");
            free(message_buffer);
            return false;
        }

        free(message_buffer);
        MULTIPLAYER_LOG("Client: Resync data received successfully");
        return true;
    }

    ERRORLOG("Client: Timeout waiting for resync data after %dms", RESYNC_RECEIVE_TIMEOUT_MS);
    return false;
}

TbBool LbNetwork_Resync(void * data_buffer, size_t buffer_length) {
    NETLOG("Starting resync, my_id=%d, buffer_length=%lu", netstate.my_id, (unsigned long)buffer_length);
    const TbBool is_server = (netstate.users[netstate.my_id].progress == USER_SERVER);

    TbBool result;
    if (is_server) {
        NETLOG("Resync: I am the server");
        result = send_resync_data(data_buffer, buffer_length);
    } else {
        NETLOG("Resync: I am a client, receiving");
        result = receive_resync_data(data_buffer, buffer_length);
    }

    if (!result) {
        NETLOG("Resync FAILED");
        return false;
    }

    NETLOG("Resync data transfer completed successfully");
    return true;
}

void LbNetwork_TimesyncBarrier(void)
{
    if (game.game_kind != GKind_MultiGame) {
        return;
    }
    const TbBool is_server = (netstate.users[netstate.my_id].progress == USER_SERVER);
    if (is_server) {
        host_process_pending_timesync_requests(RESYNC_TIMESYNC_TIMEOUT_MS);
        TbClockMSec resume_time = LbNetwork_HostSendResumeToAllClients();
        LbNetwork_WaitUntilResumeTime(resume_time);
        MULTIPLAYER_LOG("Host: Timesync barrier complete");
    } else {
        if (!client_timesync_with_host()) {
            MULTIPLAYER_LOG("Client: Timesync failed, disconnecting");
            LbNetwork_Stop();
            return;
        }
        TbClockMSec resume_time = 0;
        if (!LbNetwork_ClientWaitForResumeMessage(&resume_time)) {
            return;
        }
        LbNetwork_WaitUntilResumeTime(resume_time);
        MULTIPLAYER_LOG("Client: Timesync barrier complete");
    }
}

#ifdef __cplusplus
}
#endif
