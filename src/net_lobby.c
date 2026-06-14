/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_lobby.c
 *     Lobby and frontend network support for Dungeon Keeper multiplayer.
 * @par Purpose:
 *     Session lifecycle, login, chat and frontend packet exchange routines.
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
#include "net_lobby.h"

#include "bflib_enet.h"
#include "bflib_datetm.h"
#include "net_exchange_common.h"
#include "bflib_netsession.h"
#include "bflib_sound.h"
#include "config_sounds.h"
#include "front_landview.h"
#include "front_network.h"
#include "frontend.h"
#include "net_lan.h"
#include "net_matchmaking.h"
#include "packets.h"
#include "player_data.h"
#include <SDL2/SDL.h>
#include "post_inc.h"
/******************************************************************************/

#define SESSION_COUNT 32

static struct TbNetworkSessionNameEntry sessions[SESSION_COUNT];
static int32_t server_port = 0;

struct MatchmakingCreateTask {
    uint16_t ipv4_port;
    uint16_t ipv6_port;
    char host_name[32];
};

static int matchmaking_create_thread(void *userdata)
{
    struct MatchmakingCreateTask *task = (struct MatchmakingCreateTask *)userdata;
    if (matchmaking_connect() == 0) {
        matchmaking_create(task->host_name, (int)task->ipv4_port, (int)task->ipv6_port);
    }
    free(task);
    return 0;
}

static void AddSessionSegment(const char *start, const char *end)
{
    if (start == end) {
        return;
    }
    for (unsigned i = 0; i < SESSION_COUNT; i += 1) {
        if (sessions[i].in_use) {
            continue;
        }
        sessions[i].in_use = 1;
        sessions[i].joinable = 1;
        size_t seglen = (size_t)(end - start);
        net_copy_name_string(sessions[i].text, start, min((size_t)SESSION_NAME_MAX_LEN, seglen + 1));
        return;
    }
}

void LbNetwork_SetServerPort(int port)
{
    server_port = port;
}

TbError process_login_message(NetUserId source, char *read_pos)
{
    if (source == SERVER_ID) {
        netstate.my_id = (NetUserId)read_pos[0];
        read_pos += 1;
        netstate.users[netstate.my_id].version = net_current_version;
        const struct GameVersionPacket *server_version = (const struct GameVersionPacket *)read_pos;
        netstate.users[SERVER_ID].version = *server_version;
        return Lb_OK;
    }
    struct NetUser *user = &netstate.users[source];
    if (user->progress != USER_CONNECTED) {
        NETMSG("Peer was not in connected state");
        return Lb_OK;
    }
    const char *password;
    if (!read_network_message_text(&read_pos, &password, sizeof(netstate.password))) {
        NETDBG(6, "Connected peer sent invalid password");
        netstate.sp->drop_user(source);
        return Lb_OK;
    }
    if (netstate.password[0] != 0 && strncmp(password, netstate.password, sizeof(netstate.password)) != 0) {
        NETMSG("Peer chose wrong password");
        return Lb_OK;
    }

    const char *name;
    if (!read_network_message_text(&read_pos, &name, sizeof(user->name) - 1) || name[0] == '\0') {
        NETDBG(6, "Connected peer sent invalid name");
        netstate.sp->drop_user(source);
        return Lb_OK;
    }
    strcpy(user->name, name);
    if (!isalnum(user->name[0])) {
        NETDBG(6, "Connected peer had bad name starting with %c", user->name[0]);
        netstate.sp->drop_user(source);
        return Lb_OK;
    }
    const struct GameVersionPacket *user_version = (const struct GameVersionPacket *)read_pos;
    user->version = *user_version;
    NETMSG("User %s successfully logged in", user->name);
    user->progress = USER_LOGGEDIN;
    play_non_3d_sample(snd_spell_stars);
    char *reply_pos = begin_net_message(NETMSG_LOGIN);
    *reply_pos = source;
    reply_pos += 1;
    memcpy(reply_pos, &netstate.users[SERVER_ID].version, sizeof(netstate.users[SERVER_ID].version));
    reply_pos += sizeof(netstate.users[SERVER_ID].version);
    send_message_buffer(source, reply_pos);
    for (NetUserId user_id = 0; user_id < netstate.max_players; user_id += 1) {
        if (netstate.users[user_id].progress == USER_UNUSED) {
            continue;
        }
        SendUserUpdate(source, user_id);
        if (user_id != netstate.my_id && user_id != source) {
            SendUserUpdate(user_id, source);
        }
    }
    UpdateLocalPlayerInfo(source);
    return Lb_OK;
}

TbError process_user_update_message(NetUserId source, char *read_pos)
{
    if (source != SERVER_ID) {
        WARNLOG("Unexpected USERUPDATE");
        return Lb_OK;
    }
    NetUserId user_id = (NetUserId)read_pos[0];
    read_pos += 1;
    if (user_id < 0 || user_id >= netstate.max_players) {
        ERRORLOG("Critical error: Out of range user ID %i received from server, could be used for buffer overflow attack", user_id);
        abort();
    }
    struct NetUser *user = &netstate.users[user_id];
    user->progress = (enum NetUserProgress)read_pos[0];
    read_pos += 1;
    const char *name;
    if (!read_network_message_text(&read_pos, &name, sizeof(user->name) - 1)) {
        ERRORLOG("Critical error: Unterminated name in USERUPDATE");
        abort();
    }
    strcpy(user->name, name);
    UpdateLocalPlayerInfo(user_id);
    return Lb_OK;
}

void LbNetwork_InitSessionsFromCmdLine(const char *str)
{
    NETMSG("Initializing sessions from command line: %s", str);
    const char *start = str;
    const char *end = str;
    while (*end != '\0') {
        if (start != end && (*end == ',' || *end == ';')) {
            AddSessionSegment(start, end);
            start = end + 1;
        }
        end += 1;
    }
    AddSessionSegment(start, end);
}

TbError LbNetwork_ExchangeLogin(char *player_name)
{
    NETMSG("Logging in as %s", player_name);
    if (1 + strlen(netstate.password) + 1 + strlen(player_name) + 1 + sizeof(net_current_version) >= sizeof(netstate.msg_buffer)) {
        ERRORLOG("Login credentials too long");
        return Lb_FAIL;
    }
    char *write_pos = begin_net_message(NETMSG_LOGIN);
    strcpy(write_pos, netstate.password);
    write_pos += strlen(netstate.password) + 1;
    strcpy(write_pos, player_name);
    write_pos += strlen(player_name) + 1;
    memcpy(write_pos, &net_current_version, sizeof(net_current_version));
    write_pos += sizeof(net_current_version);
    send_message_buffer(SERVER_ID, write_pos);
    TbClockMSec wait_start_time = LbTimerClock();
    while (LbTimerClock() - wait_start_time < TIMEOUT_JOIN_LOBBY) {
        if (!netstate.sp->msgready(SERVER_ID, 0)) {
            netstate.sp->update(NULL);
            if (!netstate.sp->msgready(SERVER_ID, 0)) {
                SDL_Delay(1);
                continue;
            }
        }
        if (process_network_message(SERVER_ID, &net_screen_packet, sizeof(struct ScreenPacket), NETMSG_FRONTEND, NULL) == Lb_FAIL
         || netstate.msg_buffer[0] == NETMSG_LOGIN) {
            break;
        }
    }
    if (netstate.msg_buffer[0] != NETMSG_LOGIN || netstate.my_id == INVALID_USER_ID) {
        return Lb_FAIL;
    }
    if (netstate.sp->msgready(SERVER_ID, TIMEOUT_JOIN_LOBBY)) {
        process_network_message(SERVER_ID, &net_screen_packet, sizeof(struct ScreenPacket), NETMSG_FRONTEND, NULL);
    }
    return Lb_OK;
}

TbError LbNetwork_ExchangeFrontend(void *send_buf, void *server_buf, size_t frame_size)
{
    return exchange_frame_block(NETMSG_FRONTEND, send_buf, server_buf, frame_size);
}

TbError LbNetwork_Create(char *, char *plyr_name, uint32_t *plyr_num, void *optns)
{
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }
    char default_port_buf[16];
    snprintf(default_port_buf, sizeof(default_port_buf), ":%u", (unsigned)ENET_DEFAULT_PORT);
    const char *port = default_port_buf;
    char port_string[16] = "";
    if (server_port != 0) {
        snprintf(port_string, sizeof(port_string), "%d", server_port);
        port = port_string;
    }
    if (netstate.sp->host(port, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }
    uint16_t local_port = ENET_DEFAULT_PORT;
    if (server_port > 0) {
        local_port = (uint16_t)server_port;
    }
    uint16_t ipv4_port = local_port;
    if (external_ipv4_port != 0) {
        ipv4_port = external_ipv4_port;
    }
    const uint16_t ipv6_port = enet_get_bound_ipv6_port();
    if (frontnet_service_selected(FrontendNetSvc_LAN)) {
        lan_host_start(plyr_name, local_port);
    }
    if (frontnet_service_selected(FrontendNetSvc_Online)) {
        struct MatchmakingCreateTask *task = malloc(sizeof(struct MatchmakingCreateTask));
        if (task != NULL) {
            task->ipv4_port = ipv4_port;
            task->ipv6_port = ipv6_port;
            snprintf(task->host_name, sizeof(task->host_name), "%s", plyr_name);
            SDL_Thread *thread = SDL_CreateThread(matchmaking_create_thread, "matchmaking_host", task);
            if (thread != NULL) {
                SDL_DetachThread(thread);
            } else {
                free(task);
            }
        }
    }
    netstate.my_id = SERVER_ID;
    snprintf(netstate.users[netstate.my_id].name, sizeof(netstate.users[netstate.my_id].name), "%s", plyr_name);
    netstate.users[netstate.my_id].progress = USER_LOGGEDIN;
    netstate.users[netstate.my_id].version = net_current_version;
    *plyr_num = netstate.my_id;
    UpdateLocalPlayerInfo(netstate.my_id);
    LbNetwork_EnableNewPlayers(true);
    return Lb_OK;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, int32_t *plyr_num, void *optns)
{
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }
    if (netstate.sp->join(nsname->text, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }
    netstate.my_id = INVALID_USER_ID;
    if (LbNetwork_ExchangeLogin(plyr_name) == Lb_FAIL) {
        return Lb_FAIL;
    }
    *plyr_num = netstate.my_id;
    return Lb_OK;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow)
{
    if (!netstate.locked && !allow) {
        for (NetUserId i = 0; i < netstate.max_players; i += 1) {
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

TbError LbNetwork_Stop(void)
{
    lan_shutdown();
    matchmaking_disconnect();
    if (netstate.sp) {
        netstate.sp->exit();
    }
    memset(&netstate, 0, sizeof(netstate));
    netstate.my_id = INVALID_USER_ID;
    return Lb_OK;
}

TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc callback, void *buf)
{
    struct TbNetworkCallbackData data;
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (!IsUserActive(id)) {
            continue;
        }
        memset(&data, 0, sizeof(data));
        snprintf(data.plyr_name, sizeof(data.plyr_name), "%s", netstate.users[id].name);
        callback(&data, buf);
    }
    return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr)
{
    for (unsigned i = 0; i < SESSION_COUNT; i += 1) {
        if (!sessions[i].in_use) {
            continue;
        }
        callback((struct TbNetworkCallbackData *)&sessions[i], ptr);
    }
    return Lb_OK;
}
