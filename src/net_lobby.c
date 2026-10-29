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

#include "game_legacy.h"
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
#include "net_spectator.h"
#include "packets.h"
#include "config_campaigns.h"
#include "game_merge.h"
#include "player_data.h"
#include <SDL3/SDL.h>
#include "post_inc.h"
/******************************************************************************/

#define SESSION_COUNT 32
#define LOBBY_SETTINGS_OFFSET offsetof(struct ConfigInfo, net_lobby_name)
#define LOBBY_SETTINGS_SIZE (sizeof(struct ConfigInfo) - LOBBY_SETTINGS_OFFSET)

static struct TbNetworkSessionNameEntry sessions[SESSION_COUNT];
static int32_t server_port = 0;
static TbClockMSec lobby_ping_last_sample;
uint32_t network_lobby_ping;
static struct LobbyAdvertisement hosted_advertisement;
static SDL_Mutex *advertisement_mutex;
static SDL_Thread *advertisement_thread;
static SDL_AtomicInt advertisement_running;

void net_lobby_advertisement(struct LobbyAdvertisement *output)
{
    if (!advertisement_mutex) {
        output->name[0] = '\0';
        output->metadata[0] = '\0';
        return;
    }
    SDL_LockMutex(advertisement_mutex);
    *output = hosted_advertisement;
    SDL_UnlockMutex(advertisement_mutex);
}

void net_lobby_refresh_metadata(void)
{
    if (netstate.my_id != SERVER_ID || !advertisement_mutex) {
        return;
    }
    struct TbNetworkSessionNameEntry session = {0};
    snprintf(session.text, sizeof(session.text), "%s", net_config_info.net_lobby_name);
    session.phase = netstate.phase;
    session.joinable = !netstate.locked && netstate.phase != NetPhase_InGame && netstate.phase != NetPhase_Loading;
    session.max_players = netstate.max_users;
    session.spectator_support = 1;
    session.spectators_enabled = net_config_info.spectators_enabled;
    session.max_spectators = MAX_NET_SPECTATORS;
    session.spectator_count = net_lobby_spectator_count();
    snprintf(session.version, sizeof(session.version), "%d.%d.%d.%d", VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD);
    struct LevelInformation *level = get_level_info(get_selected_level_number());
    if (netstate.locked && level) {
        session.max_players = min(session.max_players, max(level->players, 2));
    }
    for (int i = 0; i < netstate.max_users && i < SESSION_HUMANS_MAX; i++) {
        if (netstate.users[i].progress == USER_LOGGEDIN) {
            snprintf(session.players[session.player_count++], NETSP_PLAYER_NAME_MAX_LEN, "%s", netstate.users[i].name);
        }
    }
    SDL_LockMutex(advertisement_mutex);
    snprintf(hosted_advertisement.name, sizeof(hosted_advertisement.name), "%s", session.text);
    if (net_session_metadata_json(&session, hosted_advertisement.metadata, sizeof(hosted_advertisement.metadata)) < 0) {
        hosted_advertisement.metadata[0] = '\0';
    }
    SDL_UnlockMutex(advertisement_mutex);
}

static char *write_lobby_settings_message(void)
{
    char *write_pos = begin_net_message(NETMSG_LOBBY_SETTINGS);
    memcpy(write_pos, (const char *)&net_config_info + LOBBY_SETTINGS_OFFSET, LOBBY_SETTINGS_SIZE);
    return write_pos + LOBBY_SETTINGS_SIZE;
}

void net_lobby_apply_settings(void)
{
    if (netstate.my_id != SERVER_ID) {
        return;
    }
    netstate.max_users = net_config_info.max_players;
    net_lobby_settings = net_config_info;
    net_lobby_refresh_metadata();
    send_remote_buffer(write_lobby_settings_message());
}

TbError process_lobby_settings_message(NetUserId source, const char *buffer, size_t size)
{
    if (source != SERVER_ID || size < LOBBY_SETTINGS_SIZE) {
        return Lb_OK;
    }
    memcpy((char *)&net_lobby_settings + LOBBY_SETTINGS_OFFSET, buffer, LOBBY_SETTINGS_SIZE);
    net_lobby_settings.net_lobby_name[sizeof(net_lobby_settings.net_lobby_name) - 1] = '\0';
    return Lb_OK;
}

int32_t net_lobby_min_players(void)
{
    int32_t min_players = MIN_NET_USERS;
    for (NetUserId user = MIN_NET_USERS; user < MAX_NET_USERS; user++) {
        if (netstate.users[user].progress != USER_UNUSED) {
            min_players = user + 1;
        }
    }
    return min_players;
}

void net_lobby_set_phase(enum NetSessionPhase phase)
{
    if (netstate.phase == NetPhase_InGame) {
        return;
    }
    netstate.phase = phase;
    net_lobby_refresh_metadata();
}

enum NetJoinRejection net_lobby_join_rejection(void)
{
    if (netstate.phase == NetPhase_InGame) {
        return NetJoin_InGame;
    }
    if (netstate.locked || netstate.phase == NetPhase_Loading) {
        return NetJoin_Locked;
    }
    return NetJoin_Accepted;
}

int32_t net_lobby_spectator_count(void)
{
    int32_t count = 0;
    for (NetUserId id = MAX_NET_USERS; id < MAX_NET_CONNECTIONS; id++) {
        if (netstate.users[id].progress != USER_UNUSED) {
            count++;
        }
    }
    return count;
}

enum NetJoinRejection net_lobby_spectator_rejection(NetUserId reservation)
{
    if (netstate.phase != NetPhase_InGame) {
        return NetJoin_SpectatorState;
    }
    if (!net_config_info.spectators_enabled) {
        return NetJoin_SpectatorsDisabled;
    }
    int32_t count = net_lobby_spectator_count();
    if (reservation >= MAX_NET_USERS && reservation < MAX_NET_CONNECTIONS && netstate.users[reservation].progress != USER_UNUSED) {
        count--;
    }
    if (count >= MAX_NET_SPECTATORS) {
        return NetJoin_SpectatorsFull;
    }
    return NetJoin_Accepted;
}

static int advertise_host(void *unused)
{
    while (SDL_GetAtomicInt(&advertisement_running)) {
        lan_host_update();
        matchmaking_service();
        SDL_Delay(50);
    }
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

TbError process_login_message(NetUserId source, char *read_pos, const char *end_pos)
{
    if (source == SERVER_ID) {
        if (end_pos - read_pos < 1 + sizeof(struct GameVersionPacket)) {
            return Lb_FAIL;
        }
        NetUserId assigned_id = (uint8_t)read_pos[0];
        if (assigned_id <= SERVER_ID || assigned_id >= MAX_NET_CONNECTIONS || (net_join_role == NetRole_Player && assigned_id >= MAX_NET_USERS) || (net_join_role == NetRole_Spectator && assigned_id < MAX_NET_USERS)) {
            return Lb_FAIL;
        }
        netstate.my_id = assigned_id;
        LbNetLog("Login: assigned local user ID %d\n", (int)netstate.my_id);
        read_pos += 1;
        netstate.users[netstate.my_id].version = net_current_version;
        const struct GameVersionPacket *server_version = (const struct GameVersionPacket *)read_pos;
        netstate.users[SERVER_ID].version = *server_version;
        read_pos += sizeof(*server_version);
        if (network_user_is_spectator(netstate.my_id)) {
            network_spectator_chat_visible = read_pos < end_pos && *read_pos != 0;
        }
        return Lb_OK;
    }
    struct NetUser *user = &netstate.users[source];
    if (user->progress != USER_CONNECTED) {
        NETMSG("Peer was not in connected state");
        return Lb_OK;
    }
    enum NetJoinRejection reason = net_lobby_join_rejection();
    if (network_user_is_spectator(source)) {
        reason = net_lobby_spectator_rejection(source);
    }
    if (reason != NetJoin_Accepted) {
        netstate.sp->drop_user(source, reason);
        return Lb_OK;
    }
    const char *password;
    if (!read_network_message_text(&read_pos, &password, sizeof(netstate.password))) {
        NETDBG(6, "Connected peer sent invalid password");
        netstate.sp->drop_user(source, NetJoin_Accepted);
        return Lb_OK;
    }
    if (netstate.password[0] != 0 && strncmp(password, netstate.password, sizeof(netstate.password)) != 0) {
        NETMSG("Peer chose wrong password");
        netstate.sp->drop_user(source, NetJoin_Locked);
        return Lb_OK;
    }

    const char *name;
    if (!read_network_message_text(&read_pos, &name, sizeof(user->name) - 1) || name[0] == '\0') {
        NETDBG(6, "Connected peer sent invalid name");
        netstate.sp->drop_user(source, NetJoin_Accepted);
        return Lb_OK;
    }
    strcpy(user->name, name);
    if (!isalnum(user->name[0])) {
        NETDBG(6, "Connected peer had bad name starting with %c", user->name[0]);
        netstate.sp->drop_user(source, NetJoin_Accepted);
        return Lb_OK;
    }
    if (end_pos - read_pos < sizeof(user->version)) {
        netstate.sp->drop_user(source, NetJoin_Version);
        return Lb_OK;
    }
    memcpy(&user->version, read_pos, sizeof(user->version));
    read_pos += sizeof(user->version);
    if (network_user_is_spectator(source) && (read_pos >= end_pos || *read_pos != NetRole_Spectator || !net_versions_match(&user->version, &net_current_version))) {
        netstate.sp->drop_user(source, NetJoin_Version);
        return Lb_OK;
    }
    const struct GameVersionPacket stable_version = {1, 4, 0, 5136};
    if (!net_versions_match(&user->version, &net_current_version) && !net_versions_match(&user->version, &stable_version)) {
        netstate.sp->drop_user(source, NetJoin_Version);
        return Lb_OK;
    }
    NETMSG("User %d (%s) successfully logged in", (int)source, user->name);
    user->progress = USER_LOGGEDIN;
    play_non_3d_sample(snd_spell_stars);
    char *reply_pos = begin_net_message(NETMSG_LOGIN);
    *reply_pos = source;
    reply_pos += 1;
    memcpy(reply_pos, &netstate.users[SERVER_ID].version, sizeof(netstate.users[SERVER_ID].version));
    reply_pos += sizeof(netstate.users[SERVER_ID].version);
    if (network_user_is_spectator(source)) {
        *reply_pos++ = net_config_info.spectator_chat;
    }
    send_message_buffer(source, reply_pos);
    SendNewUserUpdates(source);
    UpdateLocalPlayerInfo(source);
    if (network_user_is_spectator(source)) {
        network_spectator_send_bootstrap(source);
    } else {
        send_message_buffer(source, write_lobby_settings_message());
    }
    return Lb_OK;
}

TbError process_user_update_message(NetUserId source, char *read_pos, const char *end_pos)
{
    if (source != SERVER_ID) {
        WARNLOG("Unexpected USERUPDATE");
        return Lb_OK;
    }
    if (end_pos - read_pos < 3) {
        return Lb_FAIL;
    }
    NetUserId user_id = (uint8_t)read_pos[0];
    read_pos += 1;
    if (user_id < 0 || user_id >= MAX_NET_CONNECTIONS) {
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
    if (read_pos + sizeof(user->version) <= end_pos) {
        memcpy(&user->version, read_pos, sizeof(user->version));
    }
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
    *write_pos++ = net_join_role;
    send_message_buffer(SERVER_ID, write_pos);
    TbClockMSec wait_start_time = LbTimerClock();
    while (net_join_rejection == NetJoin_Accepted && LbTimerClock() - wait_start_time < TIMEOUT_JOIN_LOBBY) {
        if (!netstate.sp->msgready(SERVER_ID, 0)) {
            if (attempting_to_join_cancel_requested()) {
                return Lb_FAIL;
            }
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
    if (net_join_rejection != NetJoin_Accepted) {
        return Lb_FAIL;
    }
    if (netstate.msg_buffer[0] != NETMSG_LOGIN || netstate.my_id == INVALID_USER_ID) {
        return Lb_FAIL;
    }
    if (netstate.sp->msgready(SERVER_ID, TIMEOUT_JOIN_LOBBY)) {
        process_network_message(SERVER_ID, &net_screen_packet, sizeof(struct ScreenPacket), NETMSG_FRONTEND, NULL);
    }
    return Lb_OK;
}

static void sample_lobby_ping(void)
{
    TbClockMSec now = LbTimerClock();
    if (network_lobby_ping == 0 || now - lobby_ping_last_sample >= 1000) {
        uint32_t ping = GetPlayersPing();
        if (ping > 0) {
            network_lobby_ping = ping;
        }
        lobby_ping_last_sample = now;
    }
}

TbError LbNetwork_ExchangeFrontend(void *send_buf, void *server_buf, size_t frame_size)
{
    TbError result = exchange_frame_block(NETMSG_FRONTEND, send_buf, server_buf, frame_size);
    sample_lobby_ping();
    return result;
}

TbError LbNetwork_ExchangeLobby(void *send_buf, void *server_buf, size_t frame_size, TbBool send_frame, void (*process_frame)(NetUserId))
{
    TbError result = Lb_OK;
    if (send_frame) {
        result = exchange_frame_message(send_buf, server_buf, frame_size, NETMSG_FRONTEND);
        netstate.seq_nbr += 1;
    }
    netstate.sp->update(OnNewUser);
    int menu_state = frontend_menu_state;
    for (NetUserId peer_id = 0; peer_id < netstate.max_users; peer_id += 1) {
        if (!can_send_to_peer(peer_id)) {
            continue;
        }
        while (frontend_menu_state == menu_state && netstate.sp->msgready(peer_id, 0)) {
            NetUserId frame_peer_id;
            process_network_message(peer_id, server_buf, frame_size, NETMSG_FRONTEND, &frame_peer_id);
            if (frame_peer_id != INVALID_USER_ID) {
                process_frame(frame_peer_id);
            }
        }
    }
    sample_lobby_ping();
    return result;
}

TbError LbNetwork_Create(char *plyr_name, uint32_t *plyr_num, void *optns)
{
    net_join_role = NetRole_Player;
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }
    char default_port_buf[16];
    snprintf(default_port_buf, sizeof(default_port_buf), ":%u", (unsigned)enet_port);
    const char *port = default_port_buf;
    char port_string[16] = "";
    if (server_port != 0) {
        snprintf(port_string, sizeof(port_string), "%d", server_port);
        port = port_string;
    }
    if (netstate.sp->host(port, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }
    if (!advertisement_mutex) {
        advertisement_mutex = SDL_CreateMutex();
    }
    if (!advertisement_mutex) {
        netstate.sp->disconnect();
        return Lb_FAIL;
    }
    netstate.phase = NetPhase_Lobby;
    netstate.my_id = SERVER_ID;
    snprintf(netstate.users[SERVER_ID].name, sizeof(netstate.users[SERVER_ID].name), "%s", plyr_name);
    netstate.users[SERVER_ID].progress = USER_LOGGEDIN;
    netstate.locked = 0;
    net_lobby_apply_settings();
    const uint16_t local_port = enet_get_bound_port();
    uint16_t ipv4_port = local_port;
    if (external_ipv4_port != 0) {
        ipv4_port = external_ipv4_port;
    }
    const uint16_t ipv6_port = enet_get_bound_ipv6_port();
    if (frontnet_service_selected(FrontendNetSvc_LAN)) {
        lan_host_start(local_port);
    }
    if (frontnet_service_selected(FrontendNetSvc_Online) && matchmaking_create(external_ipv4_address, ipv4_port, ipv6_port, local_port) != 0) {
        LbNetwork_Stop();
        return Lb_FAIL;
    }
    netstate.users[netstate.my_id].version = net_current_version;
    *plyr_num = netstate.my_id;
    UpdateLocalPlayerInfo(netstate.my_id);
    LbNetwork_EnableNewPlayers(1);
    SDL_SetAtomicInt(&advertisement_running, 1);
    advertisement_thread = SDL_CreateThread(advertise_host, "lobby_advertising", NULL);
    if (!advertisement_thread) {
        LbNetwork_Stop();
        return Lb_FAIL;
    }
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
    snprintf(net_lobby_settings.net_lobby_name, sizeof(net_lobby_settings.net_lobby_name), "%s", nsname->text);
    netstate.my_id = INVALID_USER_ID;
    if (LbNetwork_ExchangeLogin(plyr_name) == Lb_FAIL) {
        netstate.sp->disconnect();
        return Lb_FAIL;
    }
    *plyr_num = netstate.my_id;
    if (net_join_role == NetRole_Spectator && !network_spectator_wait(NETMSG_SPECTATOR_BOOTSTRAP)) {
        netstate.sp->disconnect();
        return Lb_FAIL;
    }
    return Lb_OK;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow)
{
    int was_locked = netstate.locked;
    netstate.locked = !allow;
    net_lobby_refresh_metadata();
    if (!was_locked && !allow) {
        for (NetUserId i = 0; i < netstate.max_users; i += 1) {
            if (netstate.users[i].progress == USER_CONNECTED) {
                netstate.sp->drop_user(i, net_lobby_join_rejection());
            }
        }
    }
    if (netstate.locked) {
        NETMSG("New players are NOT allowed to join");
    } else {
        NETMSG("New players ARE allowed to join");
    }
    return Lb_OK;
}

TbError LbNetwork_Stop(void)
{
    SDL_SetAtomicInt(&advertisement_running, 0);
    if (advertisement_thread) {
        SDL_WaitThread(advertisement_thread, NULL);
        advertisement_thread = NULL;
    }
    lan_shutdown();
    matchmaking_disconnect(netstate.phase);
    if (netstate.sp) {
        netstate.sp->exit();
    }
    clear_flag(local_system_flags, GSF_NetworkActive);
    network_spectator_clear_roles();
    memset(&netstate, 0, sizeof(netstate));
    memset(&net_lobby_settings, 0, sizeof(net_lobby_settings));
    netstate.my_id = INVALID_USER_ID;
    netstate.pending_spectator_id = INVALID_USER_ID;
    net_join_role = NetRole_Player;
    return Lb_OK;
}

TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc callback, void *buf)
{
    struct TbNetworkCallbackData data;
    for (NetUserId id = 0; id < netstate.max_users; id += 1) {
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
