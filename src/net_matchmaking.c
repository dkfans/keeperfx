/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_matchmaking.c
 *     Matchmaking client for the KeeperFX lobby server.
 * @par Purpose:
 *     Manages a WebSocket connection to the matchmaking server.
 *     Hosts register their lobby; clients list and join via hole-punch relay.
 * @author   KeeperFX Team
 * @date     06 Mar 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_matchmaking.h"
#include "bflib_basics.h"
#include "ver_defs.h"

#include <SDL3/SDL.h>
#ifndef _WIN32
#include <sys/select.h>
#endif
#include <curl/curl.h>
#include <curl/websockets.h>
#include <string.h>
#include <stdio.h>
#include "net_lobby.h"
#include <json-dom.h>
#include "post_inc.h"

#define WEBSOCKET_BUFFER_SIZE         262144
#define WEBSOCKET_RECEIVE_TIMEOUT_MS  3000
#define SEND_BUFFER_SIZE              4096
#define CONNECT_TIMEOUT_MS            5000
#define HEARTBEAT_INTERVAL_MS          30000
#define HEARTBEAT_TIMEOUT_MS           5000
#define HEARTBEAT_FAILURE_LIMIT        3

#define MATCHMAKING_WS_PREFIX "wss://"
#define MATCHMAKING_WS_SUFFIX "/ws"
#define MATCHMAKING_IP_PREFIX "https://"
#define MATCHMAKING_IP_SUFFIX "/ip"
#define MATCHMAKING_HOST_DEFAULT "matchmaking.keeperfx.workers.dev"

TbBool matchmaking_enabled = true;
char matchmaking_ws_url[MATCHMAKING_URL_MAX] = MATCHMAKING_WS_PREFIX MATCHMAKING_HOST_DEFAULT MATCHMAKING_WS_SUFFIX;
char matchmaking_ip_url[MATCHMAKING_URL_MAX] = MATCHMAKING_IP_PREFIX MATCHMAKING_HOST_DEFAULT MATCHMAKING_IP_SUFFIX;

static CURL *curl_handle = NULL;
static char hosted_lobby_id[MATCHMAKING_ID_MAX] = {0};
char join_lobby_id[MATCHMAKING_ID_MAX] = {0};
static SDL_Mutex *mutex = NULL;
static SDL_Thread *connection_thread;
static SDL_Thread *create_thread;
static SDL_Thread *resolve_thread;
static char local_ipv4[MATCHMAKING_IP_MAX] = {0};
static char local_ipv6[MATCHMAKING_IP_MAX] = {0};
static char create_name[MATCHMAKING_NAME_MAX * 6];
static PunchAddresses create_addresses;
static const char list_request[] = "{\"action\":\"list\",\"version\":\"" VER_STRING "\"}";
static Uint32 heartbeat_time;
static int heartbeat_attempts;
static Uint32 host_retry_time;
static char start_metadata[LINEMSG_SIZE * 6 + 64];
static PunchAddresses queued_punch[MATCHMAKING_SESSIONS_MAX];
static int queued_punch_read;
static int queued_punch_count;
static char last_metadata[SESSION_METADATA_MAX];
static char receive_message[WEBSOCKET_BUFFER_SIZE];
static size_t receive_length;
static struct TbNetworkSessionNameEntry listed_sessions[MATCHMAKING_SESSIONS_MAX];
static int listed_session_count;

struct TbNetworkSessionNameEntry matchmaking_sessions[MATCHMAKING_SESSIONS_MAX];
int matchmaking_session_count = 0;

static int parse_punch_addresses(const VALUE *message, PunchAddresses *output);
static void read_lobbies(const VALUE *message);

void matchmaking_set_server(const char *host)
{
    if (!host || !*host) {
        matchmaking_enabled = false;
        matchmaking_ws_url[0] = 0;
        matchmaking_ip_url[0] = 0;
        return;
    }
    const char *scheme_end = strstr(host, "://");
    if (scheme_end) {
        host = scheme_end + 3;
    }
    size_t length = strlen(host);
    if (length > 0 && host[length - 1] == '/') {
        length--;
    }
    int lws = snprintf(matchmaking_ws_url, sizeof(matchmaking_ws_url), MATCHMAKING_WS_PREFIX "%.*s" MATCHMAKING_WS_SUFFIX, (int)length, host);
    int lip = snprintf(matchmaking_ip_url, sizeof(matchmaking_ip_url), MATCHMAKING_IP_PREFIX "%.*s" MATCHMAKING_IP_SUFFIX, (int)length, host);
    if (!length || lws < 0 || lip < 0 || lws >= sizeof(matchmaking_ws_url) || lip >= sizeof(matchmaking_ip_url)) {
        LbNetLog("Matchmaking: invalid server URL\n");
        matchmaking_set_server(NULL);
        return;
    }
    matchmaking_enabled = true;
}

static size_t write_to_buffer(char *data, size_t element_size, size_t element_count, void *userdata)
{
    char *buffer = userdata;
    size_t incoming_size = element_size * element_count;
    size_t existing_size = strlen(buffer);
    if (existing_size + incoming_size >= MATCHMAKING_IP_MAX - 1) {
        LbNetLog("Matchmaking: write_to_buffer response too large, aborting\n");
        return 0;
    }
    memcpy(buffer + existing_size, data, incoming_size);
    buffer[existing_size + incoming_size] = '\0';
    return incoming_size;
}

static void resolve_public_address(int32_t address_family, char *output)
{
    output[0] = '\0';
    CURL *handle = curl_easy_init();
    if (!handle) return;
    LbNetLog("Matchmaking: resolving address via \"%s\"\n", matchmaking_ip_url);
    curl_easy_setopt(handle, CURLOPT_URL, matchmaking_ip_url);
    curl_easy_setopt(handle, CURLOPT_IPRESOLVE, (long)address_family);
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, (long)CONNECT_TIMEOUT_MS);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, (long)CONNECT_TIMEOUT_MS);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_to_buffer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, output);
    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK)
        LbNetLog("Matchmaking: resolve_public_address failed (%s)\n", curl_easy_strerror(result));
    curl_easy_cleanup(handle);
}

static int resolve_ipv6_thread(void *output)
{
    resolve_public_address(CURL_IPRESOLVE_V6, output);
    return 0;
}

static int resolve_public_ips_thread(void *userdata)
{
    (void)userdata;
    char resolved_ipv4[MATCHMAKING_IP_MAX] = {0};
    char resolved_ipv6[MATCHMAKING_IP_MAX] = {0};
    SDL_Thread *ipv6_thread = SDL_CreateThread(resolve_ipv6_thread, "resolve_ipv6", resolved_ipv6);
    resolve_public_address(CURL_IPRESOLVE_V4, resolved_ipv4);
    if (ipv6_thread) {
        SDL_WaitThread(ipv6_thread, NULL);
    } else {
        resolve_public_address(CURL_IPRESOLVE_V6, resolved_ipv6);
    }
    SDL_LockMutex(mutex);
    if (resolved_ipv4[0] != '\0')
        snprintf(local_ipv4, sizeof(local_ipv4), "%s", resolved_ipv4);
    if (resolved_ipv6[0] != '\0')
        snprintf(local_ipv6, sizeof(local_ipv6), "%s", resolved_ipv6);
    LbNetLog("Matchmaking: public IPs: ipv4=%s ipv6=%s\n", local_ipv4, local_ipv6);
    SDL_UnlockMutex(mutex);
    return 0;
}

static void websocket_cleanup(void)
{
    curl_easy_cleanup(curl_handle);
    curl_handle = NULL;
    hosted_lobby_id[0] = '\0';
    listed_session_count = 0;
    receive_length = 0;
    queued_punch_read = 0;
    queued_punch_count = 0;
    start_metadata[0] = '\0';
    last_metadata[0] = '\0';
}

static int websocket_send(const char *request)
{
    size_t remaining = strlen(request);
    Uint32 timeout_deadline = (Uint32)SDL_GetTicks() + CONNECT_TIMEOUT_MS;
    CURLcode curl_result = CURLE_OK;
    while (remaining && (curl_result == CURLE_OK || curl_result == CURLE_AGAIN) && (int)(timeout_deadline - (Uint32)SDL_GetTicks()) > 0) {
        size_t bytes_sent = 0;
        curl_result = curl_ws_send(curl_handle, request, remaining, &bytes_sent, 0, CURLWS_TEXT);
        request += bytes_sent;
        remaining -= bytes_sent;
        if (curl_result == CURLE_AGAIN || !bytes_sent)
            SDL_Delay(1);
    }
    if (!remaining && curl_result == CURLE_OK)
        return 0;
    if (remaining && (curl_result == CURLE_OK || curl_result == CURLE_AGAIN))
        curl_result = CURLE_OPERATION_TIMEDOUT;
    LbNetLog("Matchmaking: websocket_send failed (%s)\n", curl_easy_strerror(curl_result));
    websocket_cleanup();
    return -1;
}

static int websocket_receive(const char *expected_type, VALUE *response, int timeout_ms)
{
    curl_socket_t raw_socket = CURL_SOCKET_BAD;
    if (curl_easy_getinfo(curl_handle, CURLINFO_ACTIVESOCKET, &raw_socket) != CURLE_OK || raw_socket == CURL_SOCKET_BAD) {
        LbNetLog("Matchmaking: websocket_receive failed to get active socket\n");
        websocket_cleanup();
        return -1;
    }
    Uint32 timeout_deadline = (Uint32)SDL_GetTicks() + timeout_ms;
    while (1) {
        int remaining = (int)(timeout_deadline - (Uint32)SDL_GetTicks());
        if (timeout_ms > 0 && remaining <= 0) {
            return 0;
        }
        size_t bytes_received = 0;
        const struct curl_ws_frame *frame = NULL;
        CURLcode result = curl_ws_recv(curl_handle, receive_message + receive_length, sizeof(receive_message) - receive_length - 1, &bytes_received, &frame);
        if (result == CURLE_AGAIN) {
            if (timeout_ms <= 0 || remaining <= 0) {
                return 0;
            }
            fd_set readable;
            FD_ZERO(&readable);
            FD_SET(raw_socket, &readable);
            struct timeval timeout = { remaining / 1000, (remaining % 1000) * 1000 };
            if (select((int)raw_socket + 1, &readable, NULL, NULL, &timeout) <= 0) {
                return 0;
            }
            continue;
        }
        if (result != CURLE_OK || !frame || (frame->flags & CURLWS_CLOSE)) {
            break;
        }
        if (frame->flags & (CURLWS_PING | CURLWS_PONG)) {
            continue;
        }
        receive_length += bytes_received;
        if (receive_length >= sizeof(receive_message) - 1) {
            break;
        }
        if (frame->bytesleft || (frame->flags & CURLWS_CONT)) {
            continue;
        }
        bytes_received = receive_length;
        receive_length = 0;
        VALUE message = {0};
        if (json_dom_parse(receive_message, bytes_received, NULL, 0, &message, NULL) != 0) {
            value_fini(&message);
            break;
        }
        const char *type = value_string(value_dict_get(&message, "type"));
        if (type && expected_type && (strcmp(type, expected_type) == 0 || strcmp(type, "error") == 0)) {
            *response = message;
            if (strcmp(type, expected_type) == 0) {
                return 1;
            }
            return -1;
        }
        int failed = 0;
        if (!type) {
            failed = 1;
        } else if (strcmp(type, "ping") == 0) {
            if (hosted_lobby_id[0] && websocket_send("{\"action\":\"pong\"}") != 0) {
                failed = 1;
            }
        } else if (strcmp(type, "pong") == 0) {
            heartbeat_time = (Uint32)SDL_GetTicks() + HEARTBEAT_INTERVAL_MS;
            heartbeat_attempts = 0;
        } else if (strcmp(type, "lobbies") == 0) {
            read_lobbies(&message);
        } else if (hosted_lobby_id[0] && strcmp(type, "punch") == 0) {
            PunchAddresses addresses;
            if (parse_punch_addresses(&message, &addresses)) {
                if (queued_punch_count < MATCHMAKING_SESSIONS_MAX) {
                    queued_punch[(queued_punch_read + queued_punch_count) % MATCHMAKING_SESSIONS_MAX] = addresses;
                    queued_punch_count++;
                }
            }
        } else if (hosted_lobby_id[0] && strcmp(type, "game_started") == 0) {
            VALUE *success = value_dict_get(&message, "success");
            if (value_type(success) == VALUE_BOOL && value_bool(success)) {
                start_metadata[0] = '\0';
            }
        } else if (hosted_lobby_id[0] && (strcmp(type, "error") == 0 || strcmp(type, "timed_out") == 0)) {
            failed = 1;
        }
        value_fini(&message);
        if (failed) {
            break;
        }
    }
    websocket_cleanup();
    return -1;
}

static int websocket_exchange(const char *request, const char *expected_type, VALUE *response)
{
    if (websocket_send(request) != 0) {
        return -1;
    }
    int received = websocket_receive(expected_type, response, WEBSOCKET_RECEIVE_TIMEOUT_MS);
    if (received == 0) {
        websocket_cleanup();
    }
    return received;
}

int matchmaking_request_list(void)
{
    if (!matchmaking_enabled || !mutex) {
        return 0;
    }
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        SDL_UnlockMutex(mutex);
        matchmaking_connect_async();
        return 0;
    }
    listed_session_count = 0;
    int result = websocket_send(list_request);
    SDL_UnlockMutex(mutex);
    return result;
}

static int parse_punch_addresses(const VALUE *message, PunchAddresses *output)
{
    *output = (PunchAddresses){0};
    const char *ipv4 = value_string(value_dict_get(message, "peerIpv4"));
    const char *ipv6 = value_string(value_dict_get(message, "peerIpv6"));
    if (ipv4) {
        snprintf(output->ipv4, sizeof(output->ipv4), "%s", ipv4);
    }
    if (ipv6) {
        snprintf(output->ipv6, sizeof(output->ipv6), "%s", ipv6);
    }
    output->ipv4_port = value_int32(value_dict_get(message, "peerIpv4Port"));
    output->ipv6_port = output->ipv4_port;
    VALUE *port = value_dict_get(message, "peerIpv6Port");
    if (port) {
        output->ipv6_port = value_int32(port);
    }
    output->direct_ipv4_port = value_int32(value_dict_get(message, "peerDirectIpv4Port"));
    return (output->ipv4_port > 0 && output->ipv4_port <= 65535 && output->ipv4[0]) || (output->ipv6_port > 0 && output->ipv6_port <= 65535 && output->ipv6[0]);
}

static void matchmaking_init(void)
{
    if (mutex) {
        return;
    }
    mutex = SDL_CreateMutex();
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

static void load_published_public_ips(const char *udp_ipv4, int udp_ipv4_port, int udp_ipv6_port, PunchAddresses *published_addresses)
{
    *published_addresses = (PunchAddresses){0};
    SDL_LockMutex(mutex);
    if (udp_ipv4_port > 0) {
        const char *ipv4 = local_ipv4;
        if (udp_ipv4[0]) {
            ipv4 = udp_ipv4;
        }
        snprintf(published_addresses->ipv4, sizeof(published_addresses->ipv4), "%s", ipv4);
    }
    if (udp_ipv6_port > 0) {
        snprintf(published_addresses->ipv6, sizeof(published_addresses->ipv6), "%s", local_ipv6);
    }
    SDL_UnlockMutex(mutex);
}

static int matchmaking_connect_thread(void *request)
{
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        curl_handle = curl_easy_init();
        if (!curl_handle) {
            SDL_UnlockMutex(mutex);
            return -1;
        }
        LbNetLog("Matchmaking: connecting to \"%s\"\n", matchmaking_ws_url);
        curl_easy_setopt(curl_handle, CURLOPT_URL, matchmaking_ws_url);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECT_ONLY, 2L);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, (int32_t)CONNECT_TIMEOUT_MS);
        curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        CURLcode curl_result = curl_easy_perform(curl_handle);
        if (curl_result != CURLE_OK) {
            LbNetLog("Matchmaking: connect to \"%s\" failed: %s\n", matchmaking_ws_url, curl_easy_strerror(curl_result));
            websocket_cleanup();
            SDL_UnlockMutex(mutex);
            return -1;
        }
    }
    int result = -1;
    if (!request) {
        result = websocket_send(list_request);
    } else {
        VALUE response = {0};
        int received = websocket_exchange(request, "created", &response);
        const char *id = value_string(value_dict_get(&response, "id"));
        if (received > 0 && id && *id && strlen(id) < sizeof(hosted_lobby_id)) {
            snprintf(hosted_lobby_id, sizeof(hosted_lobby_id), "%s", id);
            heartbeat_time = (Uint32)SDL_GetTicks();
            heartbeat_attempts = 0;
            result = 0;
        } else {
            websocket_cleanup();
        }
        value_fini(&response);
    }
    SDL_UnlockMutex(mutex);
    return result;
}

void matchmaking_connect_async(void)
{
    if (!matchmaking_enabled) {
        return;
    }
    matchmaking_init();
    if (!mutex || SDL_GetThreadState(create_thread) == SDL_THREAD_ALIVE || SDL_GetThreadState(connection_thread) == SDL_THREAD_ALIVE) {
        return;
    }
    SDL_WaitThread(connection_thread, NULL);
    if (!resolve_thread) {
        resolve_thread = SDL_CreateThread(resolve_public_ips_thread, "resolve_ips", NULL);
    }
    connection_thread = SDL_CreateThread(matchmaking_connect_thread, "matchmaking", NULL);
}

void matchmaking_disconnect(enum NetSessionPhase phase)
{
    if (!mutex) {
        return;
    }
    SDL_WaitThread(create_thread, NULL);
    create_thread = NULL;
    SDL_WaitThread(connection_thread, NULL);
    connection_thread = NULL;
    SDL_WaitThread(resolve_thread, NULL);
    resolve_thread = NULL;
    SDL_LockMutex(mutex);
    if (curl_handle && hosted_lobby_id[0]) {
        const char *action = "cancel";
        if (phase == NetPhase_InGame) {
            action = "delete";
        }
        char request[SEND_BUFFER_SIZE];
        snprintf(request, sizeof(request), "{\"action\":\"%s\",\"id\":\"%s\"}", action, hosted_lobby_id);
        VALUE response = {0};
        websocket_exchange(request, "deleted", &response);
        value_fini(&response);
    }
    websocket_cleanup();
    matchmaking_session_count = 0;
    SDL_UnlockMutex(mutex);
}

void matchmaking_start_game(int map_number, const char *map_name)
{
    if (!mutex) {
        return;
    }
    char escaped_map_name[LINEMSG_SIZE * 6 + 1];
    net_json_escape(escaped_map_name, sizeof(escaped_map_name), map_name);
    SDL_LockMutex(mutex);
    snprintf(start_metadata, sizeof(start_metadata), "\"mapNumber\":%d,\"mapName\":\"%s\",", map_number, escaped_map_name);
    host_retry_time = (Uint32)SDL_GetTicks();
    SDL_UnlockMutex(mutex);
}

static void read_lobbies(const VALUE *message)
{
    VALUE *lobbies = value_dict_get(message, "lobbies");
    if (value_type(lobbies) != VALUE_ARRAY) {
        return;
    }
    listed_session_count = 0;
    for (int i = 0; i < value_array_size(lobbies) && listed_session_count < MATCHMAKING_SESSIONS_MAX; i++) {
        VALUE *lobby = value_array_get(lobbies, i);
        const char *id = value_string(value_dict_get(lobby, "id"));
        const char *name = value_string(value_dict_get(lobby, "name"));
        if (!id || !name || !*id || strlen(id) >= SESSION_LOBBY_ID_MAX_LEN || strcmp(id, hosted_lobby_id) == 0) {
            continue;
        }
        struct TbNetworkSessionNameEntry *session = &listed_sessions[listed_session_count++];
        memset(session, 0, sizeof(*session));
        session->in_use = 1;
        session->id = listed_session_count;
        snprintf(session->text, sizeof(session->text), "%s", name);
        snprintf(session->join_address, sizeof(session->join_address), "%s", id);
        snprintf(session->lobby_id, sizeof(session->lobby_id), "%s", id);
        net_session_parse_metadata(session, lobby);
    }
}

static int matchmaking_create_thread(void *previous_thread)
{
    SDL_WaitThread(previous_thread, NULL);
    char metadata[SESSION_METADATA_MAX];
    net_lobby_metadata(metadata);
    PunchAddresses published_addresses;
    load_published_public_ips(create_addresses.ipv4, create_addresses.ipv4_port, create_addresses.ipv6_port, &published_addresses);
    char request[SEND_BUFFER_SIZE];
    int used = snprintf(request, sizeof(request), "{\"action\":\"create\",\"name\":\"%s\",\"ipv4Port\":%d,\"ipv6Port\":%d,\"directIpv4Port\":%d,%s,\"ipv4\":\"%s\",\"ipv6\":\"%s\",\"resultActions\":true}", create_name, create_addresses.ipv4_port, create_addresses.ipv6_port, create_addresses.direct_ipv4_port, metadata, published_addresses.ipv4, published_addresses.ipv6);
    if (!metadata[0] || used < 0 || used >= sizeof(request)) {
        return -1;
    }
    SDL_LockMutex(mutex);
    queued_punch_read = 0;
    queued_punch_count = 0;
    snprintf(last_metadata, sizeof(last_metadata), "%s", metadata);
    SDL_UnlockMutex(mutex);
    return matchmaking_connect_thread(request);
}

int matchmaking_create(const char *name, const char *udp_ipv4, int udp_ipv4_port, int udp_ipv6_port, int direct_ipv4_port)
{
    if (!matchmaking_enabled) {
        return 0;
    }
    matchmaking_init();
    if (!mutex || SDL_GetThreadState(create_thread) == SDL_THREAD_ALIVE) {
        return -1;
    }
    SDL_WaitThread(create_thread, NULL);
    net_json_escape(create_name, sizeof(create_name), name);
    snprintf(create_addresses.ipv4, sizeof(create_addresses.ipv4), "%s", udp_ipv4);
    create_addresses.ipv4_port = udp_ipv4_port;
    create_addresses.ipv6_port = udp_ipv6_port;
    create_addresses.direct_ipv4_port = direct_ipv4_port;
    create_thread = SDL_CreateThread(matchmaking_create_thread, "matchmaking_host", connection_thread);
    if (!create_thread) {
        return -1;
    }
    connection_thread = NULL;
    return 0;
}

int matchmaking_punch(const char *lobby_id, const char *udp_ipv4, int udp_ipv4_port, int udp_ipv6_port, PunchAddresses *output)
{
    if (!matchmaking_enabled || !mutex) {
        return -1;
    }
    char request[SEND_BUFFER_SIZE];
    PunchAddresses published_addresses;
    load_published_public_ips(udp_ipv4, udp_ipv4_port, udp_ipv6_port, &published_addresses);
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        SDL_UnlockMutex(mutex);
        return -1;
    }
    snprintf(request, sizeof(request), "{\"action\":\"punch\",\"lobbyId\":\"%s\",\"myIpv4Port\":%d,\"myIpv6Port\":%d,\"myIpv4\":\"%s\",\"myIpv6\":\"%s\",\"version\":\"%d.%d.%d.%d\"}", lobby_id, udp_ipv4_port, udp_ipv6_port, published_addresses.ipv4, published_addresses.ipv6, VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD);
    VALUE response = {0};
    int received = websocket_exchange(request, "punch", &response);
    int result = -1;
    if (received > 0) {
        if (parse_punch_addresses(&response, output)) {
            result = 0;
        }
    } else {
        VALUE *reason = value_dict_get(&response, "reason");
        if (value_type(reason) == VALUE_INT32 && value_int32(reason) >= NetJoin_InGame && value_int32(reason) <= NetJoin_Version) {
            net_join_rejection = value_int32(reason);
        }
    }
    value_fini(&response);
    SDL_UnlockMutex(mutex);
    return result;
}

void matchmaking_service(void)
{
    if (!matchmaking_enabled || !mutex) {
        return;
    }
    char metadata[SESSION_METADATA_MAX];
    net_lobby_metadata(metadata);
    if (!SDL_TryLockMutex(mutex)) {
        return;
    }
    if (!curl_handle) {
        SDL_UnlockMutex(mutex);
        return;
    }
    Uint32 now = (Uint32)SDL_GetTicks();
    int starting = start_metadata[0] && (int32_t)(now - host_retry_time) >= 0;
    if (hosted_lobby_id[0] && metadata[0] && (starting || strcmp(metadata, last_metadata) != 0)) {
        char request[SEND_BUFFER_SIZE];
        const char *action = "update";
        const char *fields = "";
        if (starting) {
            action = "game_started";
            fields = start_metadata;
            host_retry_time = now + HEARTBEAT_TIMEOUT_MS;
        }
        int used = snprintf(request, sizeof(request), "{\"action\":\"%s\",\"id\":\"%s\",%s%s}", action, hosted_lobby_id, fields, metadata);
        if (used < 0 || used >= sizeof(request)) {
            websocket_cleanup();
        } else if (websocket_send(request) == 0) {
            snprintf(last_metadata, sizeof(last_metadata), "%s", metadata);
        }
    }
    if (curl_handle) {
        websocket_receive(NULL, NULL, 0);
    }
    now = (Uint32)SDL_GetTicks();
    if (curl_handle && hosted_lobby_id[0] && (int)(heartbeat_time - now) <= 0) {
        if (heartbeat_attempts >= HEARTBEAT_FAILURE_LIMIT || websocket_send("{\"action\":\"ping\"}") != 0) {
            websocket_cleanup();
        } else {
            heartbeat_attempts++;
            heartbeat_time = now + HEARTBEAT_TIMEOUT_MS;
        }
    }
    SDL_UnlockMutex(mutex);
}

void matchmaking_refresh_sessions(void)
{
    if (!mutex || !SDL_TryLockMutex(mutex)) {
        return;
    }
    matchmaking_session_count = listed_session_count;
    memcpy(matchmaking_sessions, listed_sessions, listed_session_count * sizeof(*listed_sessions));
    SDL_UnlockMutex(mutex);
}

int matchmaking_poll_punch(PunchAddresses *output)
{
    if (!mutex || !SDL_TryLockMutex(mutex)) {
        return 0;
    }
    int result = 0;
    if (!hosted_lobby_id[0] && SDL_GetThreadState(create_thread) != SDL_THREAD_ALIVE) {
        result = -1;
    }
    if (queued_punch_count > 0) {
        *output = queued_punch[queued_punch_read];
        queued_punch_read = (queued_punch_read + 1) % MATCHMAKING_SESSIONS_MAX;
        queued_punch_count--;
        result = 1;
    }
    SDL_UnlockMutex(mutex);
    return result;
}
