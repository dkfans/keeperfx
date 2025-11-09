/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_network.c
 *     Network support library.
 * @par Purpose:
 *     Network support routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Apr 2009 - 13 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_network.h"
#include "bflib_network_internal.h"
#include "bflib_enet.h"
#include "bflib_datetm.h"
#include "bflib_netsession.h"
#include "bflib_sound.h"
#include "globals.h"
#include "frontend.h"
#include "net_game.h"
#include "front_landview.h"
#include "front_network.h"
#include "post_inc.h"

#ifdef __cplusplus
void gameplay_loop_draw();
extern "C" void network_yield_draw();
#endif

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// External function declarations

/******************************************************************************/
// Local functions definition
static void ProcessMessagesUntil(NetUserId id, void *serv_buf, size_t frame_size, enum NetMessageType target_msg, TbBool block_on_first_message, TbClockMSec total_timeout_ms);
/******************************************************************************/

struct TbNetworkPlayerInfo *localPlayerInfoPtr;
unsigned long hostId;
static int ServerPort = 0;
#define SESSION_COUNT 32
struct NetState netstate;
static struct TbNetworkSessionNameEntry sessions[SESSION_COUNT];

static TbBool IsUserActive(NetUserId id) {
    return (netstate.users[id].progress == USER_LOGGEDIN || netstate.users[id].progress == USER_SERVER);
}

static void UpdateLocalPlayerInfo(NetUserId id) {
    localPlayerInfoPtr[id].active = (netstate.users[id].progress != USER_UNUSED);
    if (!localPlayerInfoPtr[id].active) {
        memset(localPlayerInfoPtr[id].name, 0, sizeof(localPlayerInfoPtr[id].name));
        return;
    }
    strcpy(localPlayerInfoPtr[id].name, netstate.users[id].name);
}

static char* InitMessageBuffer(enum NetMessageType msg_type) {
    char* ptr = netstate.msg_buffer;
    *ptr = msg_type;
    return ptr + 1;
}

static void SendMessage(NetUserId dest, const char* end_ptr) {
    netstate.sp->sendmsg_single(dest, netstate.msg_buffer, end_ptr - netstate.msg_buffer);
}

static void SendUserUpdate(NetUserId dest, NetUserId updated_user) {
    char * ptr = InitMessageBuffer(NETMSG_USERUPDATE);
    *ptr = updated_user;
    ptr += 1;
    *ptr = netstate.users[updated_user].progress;
    ptr += 1;
    strcpy(ptr, netstate.users[updated_user].name);
    ptr += strlen(netstate.users[updated_user].name) + 1;
    SendMessage(dest, ptr);
}

static void SendFrameToPeers(NetUserId source_id, const void * send_buf, size_t buf_size, int seq_nbr) {
    char * ptr = InitMessageBuffer(NETMSG_FRAME);
    *ptr = source_id;
    ptr += 1;
    *(int *) ptr = seq_nbr;
    ptr += 4;
    memcpy(ptr, send_buf, buf_size);
    ptr += buf_size;
    NetUserId id;
    for (id = 0; id < netstate.max_players; id += 1) {
        if (id == source_id) { continue; }
        if (!IsUserActive(id)) { continue; }
        SendMessage(id, ptr);
    }
}


static TbError ProcessMessage(NetUserId source, void* server_buf, size_t frame_size) {
    if (netstate.sp->readmsg(source, netstate.msg_buffer, sizeof(netstate.msg_buffer)) <= 0) {
        NETLOG("Problem reading message from %u", source);
        return Lb_FAIL;
    }
    char *ptr = netstate.msg_buffer;
    enum NetMessageType type = (enum NetMessageType)*ptr;
    TbBool from_server = (source == SERVER_ID);
    ptr += 1;
    if (type == NETMSG_LOGIN) {
        if (from_server) {
            netstate.my_id = (NetUserId)*ptr;
            return Lb_OK;
        }
        if (netstate.users[source].progress != USER_CONNECTED) {
            NETMSG("Peer was not in connected state");
            return Lb_OK;
        }
        if (netstate.password[0] != 0 && strncmp(ptr, netstate.password, sizeof(netstate.password)) != 0) {
            NETMSG("Peer chose wrong password");
            return Lb_OK;
        }
        size_t len = strlen(ptr) + 1;
        ptr += len;
        if (len > sizeof(netstate.password)) {
            NETDBG(6, "Connected peer attempted to flood password");
            netstate.sp->drop_user(source);
            return Lb_OK;
        }
        snprintf(netstate.users[source].name, sizeof(netstate.users[source].name), "%s", ptr);
        if (!isalnum(netstate.users[source].name[0])) {
            NETDBG(6, "Connected peer had bad name starting with %c", netstate.users[source].name[0]);
            netstate.sp->drop_user(source);
            return Lb_OK;
        }
        NETMSG("User %s successfully logged in", netstate.users[source].name);
        netstate.users[source].progress = USER_LOGGEDIN;
        play_non_3d_sample(76);
        char * msg_ptr = InitMessageBuffer(NETMSG_LOGIN);
        *msg_ptr = source;
        msg_ptr += 1;
        SendMessage(source, msg_ptr);
        NetUserId uid;
        for (uid = 0; uid < netstate.max_players; uid += 1) {
            if (netstate.users[uid].progress == USER_UNUSED) {
                continue;
            }
            SendUserUpdate(source, uid);
            if (uid != netstate.my_id && uid != source) {
                SendUserUpdate(uid, source);
            }
        }
        UpdateLocalPlayerInfo(source);
        return Lb_OK;
    }
    if (type == NETMSG_USERUPDATE) {
        if (!from_server) {
            WARNLOG("Unexpected USERUPDATE");
            return Lb_OK;
        }
        NetUserId id = (NetUserId)*ptr;
        if (id < 0 || id >= netstate.max_players) {
            NETLOG("Critical error: Out of range user ID %i received from server, could be used for buffer overflow attack", id);
            abort();
        }
        ptr += 1;
        netstate.users[id].progress = (enum NetUserProgress)*ptr;
        ptr += 1;
        snprintf(netstate.users[id].name, sizeof(netstate.users[id].name), "%s", ptr);
        UpdateLocalPlayerInfo(id);
        return Lb_OK;
    }
    if (type == NETMSG_FRAME) {
        NetUserId peer_id = (NetUserId)*ptr;
        char* peer_buf = ((char*)server_buf) + peer_id * frame_size;
        ptr += 1;
        netstate.users[peer_id].ack = *(int *)ptr;
        ptr += 4;
        memcpy(peer_buf, ptr, frame_size);
        if (!from_server) {
            SendFrameToPeers(peer_id, peer_buf, frame_size, netstate.users[peer_id].ack);
        }
        return Lb_OK;
    }
    return Lb_OK;
}

void LbNetwork_SetServerPort(int port) {
    ServerPort = port;
}

static void AddSessionSegment(const char* start, const char* end) {
    if (start == end) { return; }
    unsigned i;
    for (i = 0; i < SESSION_COUNT; i += 1) {
        if (sessions[i].in_use) { continue; }
        sessions[i].in_use = 1;
        sessions[i].joinable = 1;
        size_t seglen = (size_t)(end - start);
        net_copy_name_string(sessions[i].text, start, min((size_t)SESSION_NAME_MAX_LEN, seglen + 1));
        return;
    }
}

void LbNetwork_InitSessionsFromCmdLine(const char * str) {
    NETMSG("Initializing sessions from command line: %s", str);
    const char* start = str;
    const char* end = str;
    while (*end != '\0') {
        if (start != end && (*end == ',' || *end == ';')) {
            AddSessionSegment(start, end);
            start = end + 1;
        }
        end += 1;
    }
    AddSessionSegment(start, end);
}

TbError LbNetwork_Init(unsigned long srvcindex, unsigned long maxplayrs, struct TbNetworkPlayerInfo *locplayr, struct ServiceInitData *init_data) {
    localPlayerInfoPtr = locplayr;
    memset(&netstate, 0, sizeof(netstate));
    netstate.max_players = maxplayrs;
    NetUserId usr;
    for (usr = 0; usr < netstate.max_players; usr += 1) {
        netstate.users[usr].id = usr;
    }
    if (srvcindex == NS_TCP_IP) {
        NETMSG("Selecting TCP/IP SP");
        netstate.sp = &tcpSP;
    } else if (srvcindex == NS_ENET_UDP) {
        netstate.sp = InitEnetSP();
        NETMSG("Selecting UDP");
    } else {
        WARNLOG("The serviceIndex value of %lu is out of range", srvcindex);
    }
    if (!netstate.sp) {
        return Lb_FAIL;
    }
    return netstate.sp->init(OnDroppedUser);
}

TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num, void *optns) {
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }
    const char *port = ":5555";
    char buf[16] = "";
    if (ServerPort != 0) {
        snprintf(buf, sizeof(buf), "%d", ServerPort);
        port = buf;
    }
    if (netstate.sp->host(port, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }
    netstate.my_id = SERVER_ID;
    snprintf(netstate.users[netstate.my_id].name, sizeof(netstate.users[netstate.my_id].name), "%s", plyr_name);
    netstate.users[netstate.my_id].progress = USER_SERVER;
    *plyr_num = netstate.my_id;
    UpdateLocalPlayerInfo(netstate.my_id);
    LbNetwork_EnableNewPlayers(true);
    return Lb_OK;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, long *plyr_num, void *optns) {
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }
    if (netstate.sp->join(nsname->text, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }
    netstate.my_id = 23456;
    NETMSG("Logging in as %s", plyr_name);
    char * ptr = InitMessageBuffer(NETMSG_LOGIN);
    strcpy(ptr, netstate.password);
    ptr += strlen(netstate.password) + 1;
    strcpy(ptr, plyr_name);
    ptr += strlen(plyr_name) + 1;
    SendMessage(SERVER_ID, ptr);
    ProcessMessagesUntil(SERVER_ID, &net_screen_packet, sizeof(struct ScreenPacket), NETMSG_LOGIN, 0, 5000);
    if (netstate.msg_buffer[0] != NETMSG_LOGIN) {
        fprintf(stderr, "Network login rejected");
        return Lb_FAIL;
    }
    ProcessMessage(SERVER_ID, &net_screen_packet, sizeof (struct ScreenPacket));
    if (netstate.my_id == 23456) {
        fprintf(stderr, "Network login unsuccessful");
        return Lb_FAIL;
    }
    *plyr_num = netstate.my_id;
    return Lb_OK;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow) {
    if (!netstate.locked && !allow) {
        NetUserId i;
        for (i = 0; i < netstate.max_players; i += 1) {
            if (netstate.users[i].progress == USER_CONNECTED) {
                netstate.sp->drop_user(i);
            }
        }
    }
    netstate.locked = !allow;
    if (netstate.locked) {
        NETMSG("New players are NOT allowed to join");
    } else {
        NETMSG("New players ARE allowed to join");
    }
    return Lb_OK;
}

TbError LbNetwork_Stop(void) {
    if (netstate.sp) {
        netstate.sp->exit();
    }
    memset(&netstate, 0, sizeof(netstate));
    return Lb_OK;
}

TbBool OnNewUser(NetUserId * assigned_id) {
    if (netstate.locked) {
        return 0;
    }
    NetUserId i;
    for (i = 0; i < netstate.max_players; i += 1) {
        if (netstate.users[i].progress == USER_UNUSED) {
            *assigned_id = i;
            netstate.users[i].progress = USER_CONNECTED;
            netstate.users[i].ack = -1;
            NETLOG("Assigning new user to ID %u", i);
            return 1;
        }
    }
    return 0;
}

void OnDroppedUser(NetUserId id, enum NetDropReason reason) {
    assert(id >= 0);
    assert(id < (int)netstate.max_players);
    if (netstate.my_id == id) {
        NETMSG("Warning: Trying to drop local user. There's a bug in code somewhere, probably server trying to send message to itself.");
        return;
    }
    if (reason == NETDROP_ERROR) {
        NETMSG("Connection error with user %i %s", id, netstate.users[id].name);
    } else if (reason == NETDROP_MANUAL) {
        NETMSG("Dropped user %i %s", id, netstate.users[id].name);
    }
    if (netstate.my_id != SERVER_ID) {
        NETMSG("Quitting after connection loss");
        LbNetwork_Stop();
        return;
    }
    memset(&netstate.users[id], 0, sizeof(netstate.users[id]));
    netstate.users[id].id = id;
    NetUserId uid;
    for (uid = 0; uid < netstate.max_players; uid += 1) {
        if (uid == netstate.my_id) { continue; }
        SendUserUpdate(uid, id);
    }
    UpdateLocalPlayerInfo(id);
}

static void ProcessMessagesUntil(NetUserId id, void *serv_buf, size_t frame_size, enum NetMessageType target_msg, TbBool block_on_first_message, TbClockMSec total_timeout_ms) {
    if (block_on_first_message) {
        if (netstate.sp->msgready(id, 10000)) {
            ProcessMessage(id, serv_buf, frame_size);
        }
        return;
    }
    if (total_timeout_ms > 0) {
        TbClockMSec start = LbTimerClock();
        while (true) {
            TbClockMSec elapsed = LbTimerClock() - start;
            if (elapsed >= total_timeout_ms) { break; }
            unsigned wait_ms = (unsigned)(total_timeout_ms - elapsed);
            if (!netstate.sp->msgready(id, wait_ms)) { break; }
            if (ProcessMessage(id, serv_buf, frame_size) == Lb_FAIL) { break; }
            if (netstate.msg_buffer[0] == target_msg) { break; }
            network_yield_draw();
        }
    } else {
        const int draw_interval = 16; // 60 times per second. If you decrease this to draw faster then that means less time spent checking for new packets. Keep the amount of draws the same between both Host and Client, don't change it dynamically.
        const int timeout_max = (1000 / game_num_fps);
        TbClockMSec start = LbTimerClock();
        TbClockMSec last_draw = start;
        while (true) {
            int elapsed = LbTimerClock() - start;
            if (elapsed >= timeout_max) {
                break;
            }
            int wait = min(timeout_max - elapsed, draw_interval);
            if (netstate.sp->msgready(id, wait)) {
                ProcessMessage(id, serv_buf, frame_size);
                break;
            }
            if (LbTimerClock() - last_draw >= draw_interval) {
                network_yield_draw();
                last_draw = LbTimerClock();
            }
        }
    }
}

TbError LbNetwork_Exchange(void *send_buf, void *server_buf, size_t client_frame_size, TbBool block_on_first_message) {
    netstate.sp->update(OnNewUser);
    memcpy(((char*)server_buf) + netstate.my_id * client_frame_size, send_buf, client_frame_size);
    SendFrameToPeers(netstate.my_id, send_buf, client_frame_size, netstate.seq_nbr);
    NetUserId id;
    for (id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id) { continue; }
        if (netstate.users[id].progress == USER_UNUSED) { continue; }
        if (netstate.users[netstate.my_id].progress != USER_SERVER && id != SERVER_ID) { continue; }
        ProcessMessagesUntil(id, server_buf, client_frame_size, NETMSG_FRAME, block_on_first_message, 0);
    }
    netstate.seq_nbr += 1;
    return Lb_OK;
}

TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *ptr) {
    struct TbNetworkCallbackData netcdat = {};
    strcpy(netcdat.svc_name, "TCP");
    callback(&netcdat, ptr);
    strcpy(netcdat.svc_name, "ENET/UDP");
    callback(&netcdat, ptr);
    NETMSG("Enumerate Services called");
    return Lb_OK;
}

TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc callback, void *buf) {
    TbNetworkCallbackData data;
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (!IsUserActive(id)) { continue; }
        memset(&data, 0, sizeof(data));
        snprintf(data.plyr_name, sizeof(data.plyr_name), "%s", netstate.users[id].name);
        callback(&data, buf);
    }
    return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr) {
    unsigned i;
    for (i = 0; i < SESSION_COUNT; i += 1) {
        if (!sessions[i].in_use) { continue; }
        callback((TbNetworkCallbackData *) &sessions[i], ptr);
    }
    return Lb_OK;
}

unsigned long get_host_player_id(void) {
  return hostId;
}

void LogInputLagChange(int delay_frames, int min_ping, int rounded_latency_ms, int frame_time_ms) {
    static int last_delay_frames = -1;
    static int last_min_ping = -1;
    if (min_ping > 0 && (delay_frames != last_delay_frames || min_ping != last_min_ping)) {
        MULTIPLAYER_LOG("Calculated input lag: %d frames (min ping: %ums rounded to %ums, frame_time: %ums)",
               delay_frames, min_ping, rounded_latency_ms, frame_time_ms);
        last_delay_frames = delay_frames;
        last_min_ping = min_ping;
    }
}

void LbNetwork_UpdateInputLagIfHost(void) {
    static TbClockMSec last_update_ms = 0;
    static TbClockMSec second_player_login_time = 0;
    static int last_player_count = 0;
    static int average_ping = 0;
    static int average_variance = 0;
    static int sample_count = 0;
    if ((game.system_flags & GSF_NetworkActive) == 0) { return; }
    if (frontend_menu_state == FeSt_START_MPLEVEL) { return; }
    if (my_player_number != get_host_player_id()) { return; }
    if (!netstate.sp) { return; }
    TbClockMSec now = LbTimerClock();
    netstate.sp->update(OnNewUser);
    int active_player_count = 0;
    NetUserId id;
    for (id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id) { continue; }
        if (IsUserActive(id)) {
            active_player_count += 1;
        }
    }
    if (active_player_count == 0) {
        second_player_login_time = 0;
        return;
    }
    if (second_player_login_time == 0) {
        second_player_login_time = now;
    }
    if (now - second_player_login_time < WAIT_FOR_STABLE_PLAYER) {
        return;
    }
    if (last_update_ms != 0 && now - last_update_ms < AVERAGE_PING_UPDATE_RATE) { return; }
    last_update_ms = now;
    if (active_player_count != last_player_count) {
        average_ping = 0;
        average_variance = 0;
        sample_count = 0;
        last_player_count = active_player_count;
    }
    int total_latency = 0;
    int total_variance = 0;
    int valid_player_count = 0;
    for (id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id) { continue; }
        if (!IsUserActive(id)) { continue; }
        int ping_ms = GetPing(id);
        if (ping_ms <= 0) {
            NETLOG("Player %d (%s) has no RTT data yet", id, netstate.users[id].name);
            continue;
        }
        int variance_ms = GetPingVariance(id);
        int calculated_ms = GetCalculatedPing(id);
        int calculated_variance = GetCalculatedVariance(id);
        NETLOG("Player %d (%s) Ping: %ums, Variance: %ums, Calculated Ping: %ums, Calculated Variance: %ums",
               id, netstate.users[id].name, ping_ms, variance_ms, calculated_ms, calculated_variance);
        total_latency += calculated_ms;
        total_variance += calculated_variance;
        valid_player_count += 1;
    }
    if (valid_player_count == 0) {
        return;
    }
    int current_average_ping = total_latency / valid_player_count;
    int current_average_variance = total_variance / valid_player_count;
    sample_count += 1;
    average_ping = average_ping + (current_average_ping - average_ping) / sample_count;
    average_variance = average_variance + (current_average_variance - average_variance) / sample_count;
    NETLOG("Average Ping: %ums, Average Variance: %ums (samples: %d)",
           average_ping, average_variance, sample_count);
    int combined_latency = average_ping + average_variance;
    int frame_time_ms = 1000 / game_num_fps;
    int rounded_latency_ms = ((combined_latency + 49) / 50) * 50;
    int delay_frames = (rounded_latency_ms + frame_time_ms - 1) / frame_time_ms;
    if (delay_frames < 1) {
        delay_frames = 1;
    }
    LogInputLagChange(delay_frames, average_ping, rounded_latency_ms, frame_time_ms);
    game.input_lag_turns = delay_frames;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
