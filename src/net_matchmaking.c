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

#define WS_BUF_SIZE          8192
#define WS_RECV_TIMEOUT_MS   3000
#define SEND_BUF_SIZE        512
#define IP_FIELD_SIZE        80
#define CONNECT_TIMEOUT_MS   5000

static CURL *g_curl = NULL;
static char g_hosted_lobby_id[MATCHMAKING_ID_MAX] = {0};
static Uint32 g_last_refresh_tick = 0;
char g_join_lobby_id[MATCHMAKING_ID_MAX] = {0};
static SDL_mutex *g_cs = NULL;

struct TbNetworkSessionNameEntry g_mm_sessions[MATCHMAKING_SESSIONS_MAX];
int g_mm_session_count = 0;

static void ws_cleanup(void)
{
    LbNetLog("Matchmaking: ws_cleanup\n");
    curl_easy_cleanup(g_curl);
    g_curl = NULL;
    g_hosted_lobby_id[0] = '\0';
    g_mm_session_count = 0;
    g_last_refresh_tick = 0;
}

static int ws_send(const char *msg)
{
    size_t sent = 0;
    CURLcode rc = curl_ws_send(g_curl, msg, strlen(msg), &sent, 0, CURLWS_TEXT);
    if (rc != CURLE_OK) {
        LbNetLog("Matchmaking: ws_send failed (%s)\n", curl_easy_strerror(rc));
        ws_cleanup();
        return -1;
    }
    return 0;
}

static int ws_recv(char *buf, size_t bufsz, int timeout_ms)
{
    curl_socket_t sock = CURL_SOCKET_BAD;
    if (curl_easy_getinfo(g_curl, CURLINFO_ACTIVESOCKET, &sock) != CURLE_OK || sock == CURL_SOCKET_BAD) {
        LbNetLog("Matchmaking: ws_recv failed to get active socket\n");
        return -1;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    if (select((int)sock + 1, &fds, NULL, NULL, &tv) <= 0)
        return 0;

    size_t recvd = 0;
    const struct curl_ws_frame *frame = NULL;
    CURLcode rc = curl_ws_recv(g_curl, buf, bufsz - 1, &recvd, &frame);
    if (rc == CURLE_AGAIN)
        return 0;
    if (rc != CURLE_OK) {
        LbNetLog("Matchmaking: ws_recv failed (%s)\n", curl_easy_strerror(rc));
        ws_cleanup();
        return -1;
    }
    buf[recvd] = '\0';
    return (int)recvd;
}

static int ws_exchange(const char *msg, char *buf, size_t bufsz)
{
    if (!g_curl) return -1;
    if (ws_send(msg) != 0) return -1;
    return ws_recv(buf, bufsz, WS_RECV_TIMEOUT_MS);
}

static const char *json_str(const char *json, const char *key, char *out, size_t outsz)
{
    char search[128];
    snprintf(search, sizeof(search), "\"%s\":\"", key);
    const char *p = strstr(json, search);
    if (!p)
        return NULL;
    p += strlen(search);
    size_t len = 0;
    while (*p && *p != '"' && len < outsz - 1)
        out[len++] = *p++;
    out[len] = '\0';
    if (*p != '"')
        return NULL;
    return p + 1;
}

static int json_int(const char *json, const char *key, int *out)
{
    char search[128];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *p = strstr(json, search);
    if (!p)
        return 0;
    p += strlen(search);
    *out = atoi(p);
    return 1;
}

void matchmaking_init(void)
{
    static int s_done = 0;
    if (s_done)
        return;
    s_done = 1;
    g_cs = SDL_CreateMutex();
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

static int connect_thread(void *arg)
{
    matchmaking_connect();
    return 0;
}

void matchmaking_connect_async(void)
{
    matchmaking_init();
    SDL_Thread *t = SDL_CreateThread(connect_thread, "matchmaking", NULL);
    if (t)
        SDL_DetachThread(t);
}

int matchmaking_connect(void)
{
    SDL_LockMutex(g_cs);
    if (g_curl) {
        SDL_UnlockMutex(g_cs);
        return 0;
    }
    g_curl = curl_easy_init();
    if (!g_curl) {
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    LbNetLog("Matchmaking: connecting to %s\n", MATCHMAKING_URL);
    curl_easy_setopt(g_curl, CURLOPT_URL, MATCHMAKING_URL);
    curl_easy_setopt(g_curl, CURLOPT_CONNECT_ONLY, 2L);
    curl_easy_setopt(g_curl, CURLOPT_CONNECTTIMEOUT_MS, (long)CONNECT_TIMEOUT_MS);
    CURLcode rc = curl_easy_perform(g_curl);
    if (rc != CURLE_OK) {
        LbNetLog("Matchmaking: connect to %s failed: %s\n", MATCHMAKING_URL, curl_easy_strerror(rc));
        ws_cleanup();
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    LbNetLog("Matchmaking: connected\n");
    SDL_UnlockMutex(g_cs);
    return 0;
}

void matchmaking_disconnect(void)
{
    SDL_LockMutex(g_cs);
    if (g_curl) {
        if (g_hosted_lobby_id[0] != '\0') {
            char msg[SEND_BUF_SIZE];
            snprintf(msg, sizeof(msg), "{\"action\":\"delete\",\"id\":\"%s\"}", g_hosted_lobby_id);
            ws_send(msg);
        }
        ws_cleanup();
        LbNetLog("Matchmaking: disconnected\n");
    }
    SDL_UnlockMutex(g_cs);
}

void matchmaking_refresh_sessions(void)
{
    SDL_LockMutex(g_cs);
    if (!g_curl) {
        SDL_UnlockMutex(g_cs);
        return;
    }
    char buf[WS_BUF_SIZE];
    int list_n;
    if (g_last_refresh_tick == 0) {
        list_n = ws_exchange("{\"action\":\"list\",\"version\":\"" MATCHMAKING_VERSION "\"}", buf, sizeof(buf));
        g_last_refresh_tick = SDL_GetTicks();
    } else {
        list_n = ws_recv(buf, sizeof(buf), 0);
    }
    if (list_n > 0)
        LbNetLog("Matchmaking: list response (%d bytes): %s\n", list_n, buf);
    if (list_n > 0 && strstr(buf, "\"lobbies\"")) {
        int count = 0;
        const char *p = buf;
        while (count < MATCHMAKING_SESSIONS_MAX) {
            char id[MATCHMAKING_ID_MAX];
            char name[MATCHMAKING_NAME_MAX];
            p = json_str(p, "id", id, sizeof(id));
            if (!p) break;
            p = json_str(p, "name", name, sizeof(name));
            if (!p) break;
            struct TbNetworkSessionNameEntry *s = &g_mm_sessions[count++];
            memset(s, 0, sizeof(*s));
            s->joinable = 1;
            s->in_use = 1;
            s->id = (unsigned long)count;
            snprintf(s->text, SESSION_NAME_MAX_LEN, "%s", name);
            snprintf(s->lobby_id, SESSION_LOBBY_ID_MAX_LEN, "%s", id);
        }
        g_mm_session_count = count;
        LbNetLog("Matchmaking: parsed %d session(s)\n", count);
    }
    SDL_UnlockMutex(g_cs);
}

int matchmaking_create(const char *name, int udp_port, const char *ip)
{
    char escaped_name[MATCHMAKING_NAME_MAX * 2];
    char ip_field[IP_FIELD_SIZE];
    char msg[SEND_BUF_SIZE];
    char buf[WS_BUF_SIZE];
    SDL_LockMutex(g_cs);
    if (!g_curl) {
        LbNetLog("Matchmaking: not connected to server, lobby won't be listed online\n");
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    int pos = 0;
    for (int i = 0; name[i] && pos < (int)sizeof(escaped_name) - 2; i++) {
        if (name[i] == '"' || name[i] == '\\')
            escaped_name[pos++] = '\\';
        escaped_name[pos++] = name[i];
    }
    escaped_name[pos] = '\0';
    ip_field[0] = '\0';
    if (ip && ip[0])
        snprintf(ip_field, sizeof(ip_field), ",\"ip\":\"%s\"", ip);
    snprintf(msg, sizeof(msg),
        "{\"action\":\"create\",\"name\":\"%s\",\"port\":%d%s,\"version\":\"%s\"}",
        escaped_name, udp_port, ip_field, MATCHMAKING_VERSION);
    int create_n = ws_exchange(msg, buf, sizeof(buf));
    if (create_n > 0)
        LbNetLog("Matchmaking: create response (%d bytes): %s\n", create_n, buf);
    if (create_n <= 0) {
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    if (!strstr(buf, "\"created\"") || !json_str(buf, "id", g_hosted_lobby_id, MATCHMAKING_ID_MAX)) {
        LbNetLog("Matchmaking: create failed - unexpected response\n");
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    LbNetLog("Matchmaking: created lobby id=%s\n", g_hosted_lobby_id);
    SDL_UnlockMutex(g_cs);
    return 0;
}

int matchmaking_punch(const char *lobby_id, int udp_port, const char *udp_ip, char *out_ip, int *out_port)
{
    char msg[SEND_BUF_SIZE];
    char buf[WS_BUF_SIZE];
    SDL_LockMutex(g_cs);
    if (!g_curl) {
        LbNetLog("Matchmaking: not connected to server, UDP hole punching unavailable\n");
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    char ip_field[IP_FIELD_SIZE] = "";
    if (udp_ip && udp_ip[0])
        snprintf(ip_field, sizeof(ip_field), ",\"udpIp\":\"%s\"", udp_ip);
    snprintf(msg, sizeof(msg),
        "{\"action\":\"punch\",\"lobbyId\":\"%s\",\"udpPort\":%d%s}",
        lobby_id, udp_port, ip_field);
    if (ws_send(msg) != 0) {
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    int punch_n;
    Uint32 deadline = SDL_GetTicks() + WS_RECV_TIMEOUT_MS;
    for (;;) {
        int remaining = (int)(deadline - SDL_GetTicks());
        if (remaining <= 0) {
            LbNetLog("Matchmaking: punch failed - timeout\n");
            SDL_UnlockMutex(g_cs);
            return -1;
        }
        punch_n = ws_recv(buf, sizeof(buf), remaining);
        if (punch_n > 0)
            LbNetLog("Matchmaking: punch response (%d bytes): %s\n", punch_n, buf);
        if (punch_n <= 0) {
            SDL_UnlockMutex(g_cs);
            return -1;
        }
        if (strstr(buf, "\"lobbies\""))
            continue;
        break;
    }
    if (!strstr(buf, "\"punch\"")
        || !json_str(buf, "peerIp", out_ip, MATCHMAKING_IP_MAX)
        || !json_int(buf, "peerPort", out_port)) {
        LbNetLog("Matchmaking: punch failed - unexpected response\n");
        SDL_UnlockMutex(g_cs);
        return -1;
    }
    LbNetLog("Matchmaking: punch relay -> %s:%d\n", out_ip, *out_port);
    SDL_UnlockMutex(g_cs);
    return 0;
}

int matchmaking_poll_punch(char *out_ip, int *out_port)
{
    if (!g_curl || !g_cs || SDL_TryLockMutex(g_cs) != 0)
        return 0;
    char buf[WS_BUF_SIZE];
    int n = ws_recv(buf, sizeof(buf), 0);
    int got_punch = 0;
    if (n > 0 && strstr(buf, "\"punch\"")) {
        got_punch = json_str(buf, "peerIp", out_ip, MATCHMAKING_IP_MAX)
            && json_int(buf, "peerPort", out_port);
        if (got_punch)
            LbNetLog("Matchmaking: poll_punch -> %s:%d\n", out_ip, *out_port);
        else
            LbNetLog("Matchmaking: poll_punch parse failed\n");
    }
    SDL_UnlockMutex(g_cs);
    return got_punch;
}
