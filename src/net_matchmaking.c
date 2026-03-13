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
#include "bflib_enet.h"
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
#define CONNECT_TIMEOUT_MS       5000

static CURL *curl_handle = NULL;
static char hosted_lobby_id[MATCHMAKING_ID_MAX] = {0};
static Uint32 last_refresh_tick = 0;
char join_lobby_id[MATCHMAKING_ID_MAX] = {0};
static SDL_mutex *mutex = NULL;
static SDL_atomic_t connect_thread_active = {0};
static int connect_gave_up = 0;

struct TbNetworkSessionNameEntry matchmaking_sessions[MATCHMAKING_SESSIONS_MAX];
int matchmaking_session_count = 0;

static void websocket_cleanup(void)
{
    curl_easy_cleanup(curl_handle);
    curl_handle = NULL;
    hosted_lobby_id[0] = '\0';
    matchmaking_session_count = 0;
    last_refresh_tick = 0;
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

    fd_set readable_sockets;
    FD_ZERO(&readable_sockets);
    FD_SET(raw_socket, &readable_sockets);
    struct timeval timeout_value = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    if (select((int)raw_socket + 1, &readable_sockets, NULL, NULL, &timeout_value) <= 0)
        return 0;

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
    return (int)bytes_received;
}

static int websocket_exchange(const char *request, char *response_buffer, size_t buffer_size)
{
    if (!curl_handle) return -1;
    if (websocket_send(request) != 0) return -1;
    return websocket_receive(response_buffer, buffer_size, WEBSOCKET_RECEIVE_TIMEOUT_MS);
}

static const char *json_parse_string(const char *json, const char *key, char *output, size_t output_buffer_size)
{
    char key_pattern[128];
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
    char key_pattern[128];
    snprintf(key_pattern, sizeof(key_pattern), "\"%s\":", key);
    const char *json_cursor = strstr(json, key_pattern);
    if (!json_cursor)
        return 0;
    json_cursor += strlen(key_pattern);
    *output = atoi(json_cursor);
    return 1;
}

void matchmaking_init(void)
{
    static int s_initialized = 0;
    if (s_initialized)
        return;
    s_initialized = 1;
    mutex = SDL_CreateMutex();
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

static int matchmaking_connect_thread(void *arg)
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
        return curl_handle ? 0 : -1;
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
    websocket_send("{\"action\":\"list\",\"version\":\"" MATCHMAKING_VERSION "\"}");
    last_refresh_tick = SDL_GetTicks();
    SDL_UnlockMutex(mutex);
    return 0;
}

void matchmaking_disconnect(void)
{
    SDL_LockMutex(mutex);
    connect_gave_up = 0;
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
    if (!curl_handle) {
        SDL_UnlockMutex(mutex);
        return;
    }
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

int matchmaking_create(const char *name, int udp_port)
{
    char escaped_lobby_name[MATCHMAKING_NAME_MAX * 2];
    char request_message[SEND_BUFFER_SIZE];
    char response_buffer[WEBSOCKET_BUFFER_SIZE];
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        LbNetLog("Matchmaking: not connected to server, lobby won't be listed online\n");
        SDL_UnlockMutex(mutex);
        return -1;
    }
    int write_pos = 0;
    for (int i = 0; name[i] && write_pos < (int)sizeof(escaped_lobby_name) - 2; i++) {
        if (name[i] == '"' || name[i] == '\\')
            escaped_lobby_name[write_pos++] = '\\';
        escaped_lobby_name[write_pos++] = name[i];
    }
    escaped_lobby_name[write_pos] = '\0';
    snprintf(request_message, sizeof(request_message),
        "{\"action\":\"create\",\"name\":\"%s\",\"port\":%d,\"version\":\"%s\"}",
        escaped_lobby_name, udp_port, MATCHMAKING_VERSION);
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

int matchmaking_punch(const char *lobby_id, int udp_port, char *output_ip, int *output_port)
{
    char request_message[SEND_BUFFER_SIZE];
    char response_buffer[WEBSOCKET_BUFFER_SIZE];
    SDL_LockMutex(mutex);
    if (!curl_handle) {
        LbNetLog("Matchmaking: not connected to server, UDP hole punching unavailable\n");
        SDL_UnlockMutex(mutex);
        return -1;
    }
    snprintf(request_message, sizeof(request_message),
        "{\"action\":\"punch\",\"lobbyId\":\"%s\",\"udpPort\":%d}",
        lobby_id, udp_port);
    if (websocket_send(request_message) != 0) {
        SDL_UnlockMutex(mutex);
        return -1;
    }
    int bytes_received;
    Uint32 timeout_deadline = SDL_GetTicks() + WEBSOCKET_RECEIVE_TIMEOUT_MS;
    for (;;) {
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
        if (strstr(response_buffer, "\"lobbies\""))
            continue;
        break;
    }
    if (!strstr(response_buffer, "\"punch\"")
        || !json_parse_string(response_buffer, "peerIp", output_ip, MATCHMAKING_IP_MAX)
        || !json_parse_int(response_buffer, "peerPort", output_port)) {
        LbNetLog("Matchmaking: punch failed - unexpected response\n");
        SDL_UnlockMutex(mutex);
        return -1;
    }
    LbNetLog("Matchmaking: punch relay -> %s:%d\n", output_ip, *output_port);
    SDL_UnlockMutex(mutex);
    return 0;
}

int matchmaking_poll_punch(char *output_ip, int *output_port)
{
    if (!curl_handle || !mutex || SDL_TryLockMutex(mutex) != 0)
        return 0;
    char response_buffer[WEBSOCKET_BUFFER_SIZE];
    int bytes_received = websocket_receive(response_buffer, sizeof(response_buffer), 0);
    int punch_was_received = 0;
    if (bytes_received > 0 && strstr(response_buffer, "\"punch\"")) {
        punch_was_received = json_parse_string(response_buffer, "peerIp", output_ip, MATCHMAKING_IP_MAX)
            && json_parse_int(response_buffer, "peerPort", output_port);
        if (punch_was_received)
            LbNetLog("Matchmaking: poll_punch -> %s:%d\n", output_ip, *output_port);
        else
            LbNetLog("Matchmaking: poll_punch parse failed\n");
    }
    SDL_UnlockMutex(mutex);
    return punch_was_received;
}
