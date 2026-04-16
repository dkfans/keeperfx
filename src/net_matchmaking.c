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
#include "net_lan.h"
#include "ver_defs.h"

#include <SDL2/SDL.h>
#ifndef _WIN32
#include <sys/select.h>
#endif
#include <curl/curl.h>
#include <curl/websockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "post_inc.h"

#define STR_(x) #x
#define STR(x) STR_(x)
#define MATCHMAKING_VERSION STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_RELEASE)

#define WEBSOCKET_BUFFER_SIZE         8192
#define WEBSOCKET_RECEIVE_TIMEOUT_MS  3000
#define SEND_BUFFER_SIZE              512
#define CONNECT_TIMEOUT_MS            5000
#define JSON_KEY_PATTERN_SIZE         128

static CURL *curl_handle = NULL;
static char hosted_lobby_id[MATCHMAKING_ID_MAX] = {0};
char join_lobby_id[MATCHMAKING_ID_MAX] = {0};
static SDL_mutex *mutex = NULL;
static SDL_atomic_t connect_thread_active = {0};
static SDL_atomic_t ips_resolved = {0};
static SDL_atomic_t ips_resolving = {0};
static int connect_gave_up = 0;
static char local_ipv4[MATCHMAKING_IP_MAX] = {0};
static char local_ipv6[MATCHMAKING_IP_MAX] = {0};

struct TbNetworkSessionNameEntry matchmaking_sessions[MATCHMAKING_SESSIONS_MAX];
int matchmaking_session_count = 0;

static void matchmaking_init(void);

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

static void resolve_public_address(long address_family, char *output)
{
    output[0] = '\0';
    CURL *handle = curl_easy_init();
    if (!handle) return;
    curl_easy_setopt(handle, CURLOPT_URL, MATCHMAKING_IP_URL);
    curl_easy_setopt(handle, CURLOPT_IPRESOLVE, address_family);
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, (long)CONNECT_TIMEOUT_MS);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, (long)CONNECT_TIMEOUT_MS);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_to_buffer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, output);
    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK)
        LbNetLog("Matchmaking: resolve_public_address failed (%s)\n", curl_easy_strerror(result));
    curl_easy_cleanup(handle);
}

typedef struct {
    long address_family;
    char *output;
} PublicAddressResolveTask;

static int resolve_public_address_thread(void *userdata)
{
    PublicAddressResolveTask *task = userdata;
    resolve_public_address(task->address_family, task->output);
    return 0;
}

static void wait_for_public_ip_resolution(void)
{
    while (SDL_AtomicGet(&ips_resolving)) {
        SDL_Delay(10);
    }
}

static int resolve_public_ips_thread(void *userdata)
{
    (void)userdata;
    char resolved_ipv4[MATCHMAKING_IP_MAX] = {0};
    char resolved_ipv6[MATCHMAKING_IP_MAX] = {0};
    PublicAddressResolveTask ipv6_task = { CURL_IPRESOLVE_V6, resolved_ipv6 };
    SDL_Thread *ipv6_thread = SDL_CreateThread(resolve_public_address_thread, "resolve_ipv6", &ipv6_task);
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
    SDL_AtomicSet(&ips_resolved, 1);
    SDL_UnlockMutex(mutex);
    SDL_AtomicSet(&ips_resolving, 0);
    return 0;
}

static void resolve_public_ips_async(void)
{
    if (SDL_AtomicGet(&ips_resolved))
        return;
    if (SDL_AtomicCAS(&ips_resolving, 0, 1) == SDL_FALSE)
        return;
    SDL_Thread *thread = SDL_CreateThread(resolve_public_ips_thread, "resolve_ips", NULL);
    if (thread) {
        SDL_DetachThread(thread);
    } else {
        SDL_AtomicSet(&ips_resolving, 0);
    }
}

static int copy_public_ip(int ipv6, char *output, int output_buffer_size)
{
    const char *source = local_ipv4;
    if (ipv6) {
        source = local_ipv6;
    }
    if (output == NULL || output_buffer_size <= 0) {
        return 0;
    }
    output[0] = '\0';
    SDL_LockMutex(mutex);
    snprintf(output, output_buffer_size, "%s", source);
    SDL_UnlockMutex(mutex);
    return output[0] != '\0';
}

static void websocket_cleanup(void)
{
    curl_easy_cleanup(curl_handle);
    curl_handle = NULL;
    hosted_lobby_id[0] = '\0';
    matchmaking_session_count = 0;
}

static int websocket_send(const char *request)
{
    size_t bytes_sent = 0;
    CURLcode curl_result = curl_ws_send(curl_handle, request, strlen(request), &bytes_sent, 0, CURLWS_TEXT);
    if (curl_result != CURLE_OK) {
        LbNetLog("Matchmaking: websocket_send failed (%s)\n", curl_easy_strerror(curl_result));
        websocket_cleanup();
        return -1;
    }
    return 0;
}

static int websocket_receive(char *response_buffer, size_t buffer_size, int timeout_ms)
{
    curl_socket_t raw_socket = CURL_SOCKET_BAD;
    if (curl_easy_getinfo(curl_handle, CURLINFO_ACTIVESOCKET, &raw_socket) != CURLE_OK || raw_socket == CURL_SOCKET_BAD) {
        LbNetLog("Matchmaking: websocket_receive failed to get active socket\n");
        return -1;
    }
    Uint32 timeout_deadline = SDL_GetTicks() + timeout_ms;
    while (1) {
        if (timeout_ms > 0) {
            int time_remaining = (int)(timeout_deadline - SDL_GetTicks());
            if (time_remaining <= 0)
                return 0;
            fd_set readable_sockets;
            FD_ZERO(&readable_sockets);
            FD_SET(raw_socket, &readable_sockets);
            struct timeval timeout_value = { time_remaining / 1000, (time_remaining % 1000) * 1000 };
            if (select((int)raw_socket + 1, &readable_sockets, NULL, NULL, &timeout_value) <= 0)
                return 0;
        }

        size_t bytes_received = 0;
        const struct curl_ws_frame *websocket_frame = NULL;
        CURLcode curl_result = curl_ws_recv(curl_handle, response_buffer, buffer_size - 1, &bytes_received, &websocket_frame);
        if (curl_result == CURLE_AGAIN)
            return 0;
        if (curl_result != CURLE_OK) {
            LbNetLog("Matchmaking: websocket_receive failed (%s)\n", curl_easy_strerror(curl_result));
            websocket_cleanup();
            return -1;
        }
        response_buffer[bytes_received] = '\0';
        if (!strstr(response_buffer, "\"type\":\"ping\""))
            return (int)bytes_received;
        if (hosted_lobby_id[0] == '\0') {
            LbNetLog("Matchmaking: ignoring ping while not hosting\n");
            continue;
        }
        if (websocket_send("{\"action\":\"pong\"}") != 0) {
            LbNetLog("Matchmaking: ping response failed\n");
            return -1;
        }
        LbNetLog("Matchmaking: ping replied\n");
    }
}

static int websocket_exchange(const char *request, char *response_buffer, size_t buffer_size)
{
    if (!curl_handle) return -1;
    if (websocket_send(request) != 0) return -1;
    return websocket_receive(response_buffer, buffer_size, WEBSOCKET_RECEIVE_TIMEOUT_MS);
}

int matchmaking_request_list(void)
{
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        SDL_UnlockMutex(mutex);
        return -1;
    }
    matchmaking_session_count = 0;
    int result = websocket_send("{\"action\":\"list\",\"version\":\"" MATCHMAKING_VERSION "\"}");
    SDL_UnlockMutex(mutex);
    return result;
}

static const char *json_parse_string(const char *json, const char *key, char *output, size_t output_buffer_size)
{
    char key_pattern[JSON_KEY_PATTERN_SIZE];
    snprintf(key_pattern, sizeof(key_pattern), "\"%s\":\"", key);
    const char *json_cursor = strstr(json, key_pattern);
    if (!json_cursor)
        return NULL;
    json_cursor += strlen(key_pattern);
    size_t output_length = 0;
    while (*json_cursor && *json_cursor != '"' && output_length < output_buffer_size - 1)
        output[output_length++] = *json_cursor++;
    output[output_length] = '\0';
    if (*json_cursor != '"')
        return NULL;
    return json_cursor + 1;
}

static int json_parse_int(const char *json, const char *key, int *output)
{
    char key_pattern[JSON_KEY_PATTERN_SIZE];
    snprintf(key_pattern, sizeof(key_pattern), "\"%s\":", key);
    const char *json_cursor = strstr(json, key_pattern);
    if (!json_cursor)
        return 0;
    json_cursor += strlen(key_pattern);
    *output = atoi(json_cursor);
    return 1;
}

static void parse_punch_addresses(const char *json, PunchAddresses *output)
{
    *output = (PunchAddresses){0};
    json_parse_string(json, "peerIpv4", output->ipv4, MATCHMAKING_IP_MAX);
    json_parse_string(json, "peerIpv6", output->ipv6, MATCHMAKING_IP_MAX);
    json_parse_int(json, "peerIpv4Port", &output->ipv4_port);
    output->ipv6_port = output->ipv4_port;
    json_parse_int(json, "peerIpv6Port", &output->ipv6_port);
}

static int punch_addresses_valid(const PunchAddresses *addresses)
{
    return (addresses->ipv4_port && addresses->ipv4[0] != '\0') || (addresses->ipv6_port && addresses->ipv6[0] != '\0');
}

static void matchmaking_init(void)
{
    static int s_initialized = 0;
    if (s_initialized)
        return;
    s_initialized = 1;
    mutex = SDL_CreateMutex();
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

static void load_published_public_ips(int udp_ipv4_port, int udp_ipv6_port, PunchAddresses *published_addresses)
{
    *published_addresses = (PunchAddresses){0};
    if (udp_ipv4_port > 0)
        copy_public_ip(0, published_addresses->ipv4, sizeof(published_addresses->ipv4));
    if (udp_ipv6_port > 0)
        copy_public_ip(1, published_addresses->ipv6, sizeof(published_addresses->ipv6));
}

static int matchmaking_connect_thread(void *)
{
    matchmaking_connect();
    SDL_AtomicSet(&connect_thread_active, 0);
    return 0;
}

void matchmaking_connect_async(void)
{
    if (SDL_AtomicCAS(&connect_thread_active, 0, 1) == SDL_FALSE)
        return;
    matchmaking_init();
    resolve_public_ips_async();
    SDL_Thread *thread = SDL_CreateThread(matchmaking_connect_thread, "matchmaking", NULL);
    if (thread) {
        SDL_DetachThread(thread);
    } else {
        SDL_AtomicSet(&connect_thread_active, 0);
    }
}

int matchmaking_connect(void)
{
    SDL_LockMutex(mutex);
    if (curl_handle || connect_gave_up) {
        SDL_UnlockMutex(mutex);
        if (curl_handle) return 0;
        return -1;
    }
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        SDL_UnlockMutex(mutex);
        return -1;
    }
    LbNetLog("Matchmaking: connecting to %s\n", MATCHMAKING_URL);
    curl_easy_setopt(curl_handle, CURLOPT_URL, MATCHMAKING_URL);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECT_ONLY, 2L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, (long)CONNECT_TIMEOUT_MS);
    curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    CURLcode curl_result = curl_easy_perform(curl_handle);
    if (curl_result != CURLE_OK) {
        LbNetLog("Matchmaking: connect to %s failed: %s\n", MATCHMAKING_URL, curl_easy_strerror(curl_result));
        connect_gave_up = 1;
        websocket_cleanup();
        SDL_UnlockMutex(mutex);
        return -1;
    }
    LbNetLog("Matchmaking: connected\n");
    SDL_UnlockMutex(mutex);
    return matchmaking_request_list();
}

void matchmaking_disconnect(void)
{
    Uint32 wait_deadline = SDL_GetTicks() + CONNECT_TIMEOUT_MS * 3;
    while (SDL_AtomicGet(&connect_thread_active) && SDL_GetTicks() < wait_deadline) {
        SDL_Delay(10);
    }
    wait_for_public_ip_resolution();
    SDL_LockMutex(mutex);
    connect_gave_up = 0;
    SDL_AtomicSet(&ips_resolved, 0);
    if (curl_handle) {
        if (hosted_lobby_id[0] != '\0') {
            char delete_message[SEND_BUFFER_SIZE];
            snprintf(delete_message, sizeof(delete_message), "{\"action\":\"delete\",\"id\":\"%s\"}", hosted_lobby_id);
            websocket_send(delete_message);
        }
        websocket_cleanup();
        LbNetLog("Matchmaking: disconnected\n");
    }
    SDL_UnlockMutex(mutex);
}

void matchmaking_refresh_sessions(void)
{
    if (!curl_handle || !mutex || SDL_TryLockMutex(mutex) != 0)
        return;
    char response_buffer[WEBSOCKET_BUFFER_SIZE];
    int bytes_received = websocket_receive(response_buffer, sizeof(response_buffer), 0);
    if (bytes_received > 0)
        LbNetLog("Matchmaking: list response (%d bytes): %s\n", bytes_received, response_buffer);
    if (bytes_received > 0 && strstr(response_buffer, "\"lobbies\"")) {
        int count = 0;
        const char *json_cursor = response_buffer;
        while (count < MATCHMAKING_SESSIONS_MAX) {
            char id[MATCHMAKING_ID_MAX];
            char name[MATCHMAKING_NAME_MAX];
            json_cursor = json_parse_string(json_cursor, "id", id, sizeof(id));
            if (!json_cursor) break;
            json_cursor = json_parse_string(json_cursor, "name", name, sizeof(name));
            if (!json_cursor) break;
            if (hosted_lobby_id[0] != '\0' && strcmp(id, hosted_lobby_id) == 0)
                continue;
            struct TbNetworkSessionNameEntry *session = &matchmaking_sessions[count++];
            memset(session, 0, sizeof(*session));
            session->joinable = 1;
            session->in_use = 1;
            session->id = (unsigned long)count;
            snprintf(session->text, SESSION_NAME_MAX_LEN, "%s", name);
            snprintf(session->join_address, SESSION_LOBBY_ID_MAX_LEN, "%s", id);
            snprintf(session->lobby_id, SESSION_LOBBY_ID_MAX_LEN, "%s", id);
        }
        matchmaking_session_count = count;
        LbNetLog("Matchmaking: parsed %d session(s)\n", count);
    }
    SDL_UnlockMutex(mutex);
}

int matchmaking_create(const char *name, int udp_ipv4_port, int udp_ipv6_port)
{
    char escaped_lobby_name[MATCHMAKING_NAME_MAX * 2];
    char request_message[SEND_BUFFER_SIZE];
    char response_buffer[WEBSOCKET_BUFFER_SIZE];
    PunchAddresses published_addresses;
    load_published_public_ips(udp_ipv4_port, udp_ipv6_port, &published_addresses);
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        LbNetLog("Matchmaking: not connected to server, lobby won't be listed online\n");
        SDL_UnlockMutex(mutex);
        return -1;
    }
    int write_position = 0;
    for (int i = 0; name[i] && write_position < (int)sizeof(escaped_lobby_name) - 2; i++) {
        if (name[i] == '"' || name[i] == '\\')
            escaped_lobby_name[write_position++] = '\\';
        escaped_lobby_name[write_position++] = name[i];
    }
    escaped_lobby_name[write_position] = '\0';
    snprintf(request_message, sizeof(request_message),
        "{\"action\":\"create\",\"name\":\"%s\",\"ipv4Port\":%d,\"ipv6Port\":%d,\"version\":\"%s\",\"ipv4\":\"%s\",\"ipv6\":\"%s\"}",
        escaped_lobby_name, udp_ipv4_port, udp_ipv6_port, MATCHMAKING_VERSION,
        published_addresses.ipv4, published_addresses.ipv6);
    int bytes_received = websocket_exchange(request_message, response_buffer, sizeof(response_buffer));
    if (bytes_received > 0)
        LbNetLog("Matchmaking: create response (%d bytes): %s\n", bytes_received, response_buffer);
    if (bytes_received <= 0) {
        SDL_UnlockMutex(mutex);
        return -1;
    }
    if (!strstr(response_buffer, "\"created\"") || !json_parse_string(response_buffer, "id", hosted_lobby_id, MATCHMAKING_ID_MAX)) {
        LbNetLog("Matchmaking: create failed - unexpected response\n");
        SDL_UnlockMutex(mutex);
        return -1;
    }
    LbNetLog("Matchmaking: created lobby id=%s\n", hosted_lobby_id);
    lan_set_lobby_id(hosted_lobby_id);
    SDL_UnlockMutex(mutex);
    return 0;
}

int matchmaking_punch(const char *lobby_id, int udp_ipv4_port, int udp_ipv6_port, PunchAddresses *output)
{
    char request_message[SEND_BUFFER_SIZE];
    char response_buffer[WEBSOCKET_BUFFER_SIZE];
    PunchAddresses published_addresses;
    load_published_public_ips(udp_ipv4_port, udp_ipv6_port, &published_addresses);
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        LbNetLog("Matchmaking: not connected to server, UDP hole punching unavailable\n");
        SDL_UnlockMutex(mutex);
        return -1;
    }
    snprintf(request_message, sizeof(request_message),
        "{\"action\":\"punch\",\"lobbyId\":\"%s\",\"myIpv4Port\":%d,\"myIpv6Port\":%d,\"myIpv4\":\"%s\",\"myIpv6\":\"%s\"}",
        lobby_id, udp_ipv4_port, udp_ipv6_port, published_addresses.ipv4, published_addresses.ipv6);
    if (websocket_send(request_message) != 0) {
        SDL_UnlockMutex(mutex);
        return -1;
    }
    int bytes_received;
    Uint32 timeout_deadline = SDL_GetTicks() + WEBSOCKET_RECEIVE_TIMEOUT_MS;
    while (1) {
        int time_remaining = (int)(timeout_deadline - SDL_GetTicks());
        if (time_remaining <= 0) {
            LbNetLog("Matchmaking: punch failed - timeout\n");
            SDL_UnlockMutex(mutex);
            return -1;
        }
        bytes_received = websocket_receive(response_buffer, sizeof(response_buffer), time_remaining);
        if (bytes_received > 0)
            LbNetLog("Matchmaking: punch response (%d bytes): %s\n", bytes_received, response_buffer);
        if (bytes_received <= 0) {
            SDL_UnlockMutex(mutex);
            return -1;
        }
        if (strstr(response_buffer, "\"punch\""))
            break;
        if (!strstr(response_buffer, "\"lobbies\"")) {
            LbNetLog("Matchmaking: punch failed - server error response\n");
            SDL_UnlockMutex(mutex);
            return -1;
        }
    }
    parse_punch_addresses(response_buffer, output);
    if (!punch_addresses_valid(output)) {
        LbNetLog("Matchmaking: punch failed - unexpected response\n");
        SDL_UnlockMutex(mutex);
        return -1;
    }
    LbNetLog("Matchmaking: punch relay -> ipv4=%s ipv6=%s ipv4_port=%d ipv6_port=%d\n", output->ipv4, output->ipv6, output->ipv4_port, output->ipv6_port);
    SDL_UnlockMutex(mutex);
    return 0;
}

int matchmaking_poll_punch(PunchAddresses *output)
{
    if (!curl_handle || !mutex || SDL_TryLockMutex(mutex) != 0)
        return 0;
    char response_buffer[WEBSOCKET_BUFFER_SIZE];
    int bytes_received = websocket_receive(response_buffer, sizeof(response_buffer), 0);
    int punch_was_received = 0;
    if (bytes_received > 0 && strstr(response_buffer, "\"punch\"")) {
        parse_punch_addresses(response_buffer, output);
        punch_was_received = punch_addresses_valid(output);
        if (punch_was_received)
            LbNetLog("Matchmaking: poll_punch -> ipv4=%s ipv6=%s ipv4_port=%d ipv6_port=%d\n", output->ipv4, output->ipv6, output->ipv4_port, output->ipv6_port);
        else
            LbNetLog("Matchmaking: poll_punch parse failed\n");
    }
    SDL_UnlockMutex(mutex);
    return punch_was_received;
}
