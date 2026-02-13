/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_matchmaking.c
 *     Matchmaking server integration for multiplayer games.
 * @par Purpose:
 *     Register and discover multiplayer lobbies through a matchmaking server.
 * @author   KeeperFX Team
 * @date     24 Jan 2026
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
#include "bflib_network.h"
#include "globals.h"

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#else
#include <curl/curl.h>
#endif

#include <json.h>
#include <json-dom.h>
#include <stdio.h>
#include <string.h>

#include "post_inc.h"

#define MATCHMAKING_SERVER_URL "https://matchmaking.keeperfx.workers.dev"
#define MATCHMAKING_HTTP_TIMEOUT_MS 5000
#define MATCHMAKING_RESPONSE_BUFFER_SIZE 4096

static char current_lobby_id[64] = "";
static TbBool lobby_registered = 0;
static char response_buffer[MATCHMAKING_RESPONSE_BUFFER_SIZE];

#ifdef __WIN32__

static HINTERNET http_session = NULL;
static HINTERNET http_connection = NULL;

static int http_request(const char *method, const char *path, const char *body) {
    if (http_session == NULL) {
        http_session = WinHttpOpen(L"KeeperFX/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (http_session == NULL) {
            return 0;
        }
        int timeout = MATCHMAKING_HTTP_TIMEOUT_MS;
        WinHttpSetOption(http_session, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(http_session, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(http_session, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        http_connection = WinHttpConnect(http_session, L"matchmaking.keeperfx.workers.dev", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (http_connection == NULL) {
            WinHttpCloseHandle(http_session);
            http_session = NULL;
            return 0;
        }
    }
    wchar_t wmethod[16] = {0};
    wchar_t wpath[128] = {0};
    mbstowcs(wmethod, method, 15);
    mbstowcs(wpath, path, 127);
    HINTERNET request = WinHttpOpenRequest(http_connection, wmethod, wpath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (request == NULL) {
        return 0;
    }
    DWORD body_len = 0;
    if (body) {
        body_len = (DWORD)strlen(body);
        WinHttpAddRequestHeaders(request, L"Content-Type: application/json", -1, WINHTTP_ADDREQ_FLAG_ADD);
    }
    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)body, body_len, body_len, 0) || !WinHttpReceiveResponse(request, NULL)) {
        WinHttpCloseHandle(request);
        return 0;
    }
    DWORD status_code = 0, size = sizeof(status_code);
    WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status_code, &size, WINHTTP_NO_HEADER_INDEX);
    if (status_code >= 400) {
        WinHttpCloseHandle(request);
        return 0;
    }
    DWORD total = 0, avail, read;
    while (WinHttpQueryDataAvailable(request, &avail) && avail > 0 && total < MATCHMAKING_RESPONSE_BUFFER_SIZE - 1) {
        DWORD remaining = MATCHMAKING_RESPONSE_BUFFER_SIZE - 1 - total;
        if (avail > remaining) {
            avail = remaining;
        }
        if (!WinHttpReadData(request, response_buffer + total, avail, &read)) {
            break;
        }
        total += read;
    }
    response_buffer[total] = '\0';
    WinHttpCloseHandle(request);
    return (int)total;
}

#else

static CURL *curl_handle = NULL;
static size_t curl_response_pos = 0;

static size_t matchmaking_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    if (curl_response_pos + realsize >= MATCHMAKING_RESPONSE_BUFFER_SIZE - 1) {
        realsize = MATCHMAKING_RESPONSE_BUFFER_SIZE - 1 - curl_response_pos;
    }
    memcpy(response_buffer + curl_response_pos, contents, realsize);
    curl_response_pos += realsize;
    response_buffer[curl_response_pos] = '\0';
    return size * nmemb;
}

static int http_request(const char *method, const char *path, const char *body) {
    if (curl_handle == NULL) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle = curl_easy_init();
        if (curl_handle == NULL) {
            return 0;
        }
    }
    curl_response_pos = 0;
    response_buffer[0] = '\0';
    char url[256];
    snprintf(url, sizeof(url), "%s%s", MATCHMAKING_SERVER_URL, path);
    curl_easy_reset(curl_handle);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, matchmaking_write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, (long)MATCHMAKING_HTTP_TIMEOUT_MS);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "KeeperFX/1.0");
    struct curl_slist *headers = NULL;
    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
        if (body) {
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body);
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
        }
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    CURLcode res = curl_easy_perform(curl_handle);
    curl_slist_free_all(headers);
    if (res != CURLE_OK) {
        return 0;
    }
    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400) {
        return 0;
    }
    return (int)curl_response_pos;
}

#endif

TbBool matchmaking_register_lobby(const char *name, uint16_t port) {
    if (lobby_registered) {
        matchmaking_unregister_lobby();
    }
    char escaped_name[128];
    int j = 0;
    for (int i = 0; name[i] && j < (int)sizeof(escaped_name) - 2; i++) {
        if (name[i] == '"' || name[i] == '\\') {
            escaped_name[j++] = '\\';
        }
        escaped_name[j++] = name[i];
    }
    escaped_name[j] = '\0';
    char body[256];
    snprintf(body, sizeof(body), "{\"name\":\"%s\",\"port\":%u}", escaped_name, port);
    if (!http_request("POST", "/lobbies", body) || sscanf(response_buffer, "{\"id\":\"%63[^\"]\"}", current_lobby_id) != 1) {
        ERRORLOG("Failed to register lobby: %s", response_buffer);
        return 0;
    }
    lobby_registered = 1;
    SYNCMSG("Registered lobby, ID: %s", current_lobby_id);
    return 1;
}

void matchmaking_unregister_lobby(void) {
    if (!lobby_registered) {
        return;
    }
    char path[80];
    snprintf(path, sizeof(path), "/lobby/%s", current_lobby_id);
    http_request("DELETE", path, NULL);
    lobby_registered = 0;
}

void matchmaking_ping_lobby(void) {
    if (!lobby_registered) {
        return;
    }
    char path[80];
    snprintf(path, sizeof(path), "/lobby/%s/ping", current_lobby_id);
    if (!http_request("POST", path, NULL) || strstr(response_buffer, "\"success\":true") == NULL) {
        WARNLOG("Lobby ping failed, lobby may have expired");
        lobby_registered = 0;
    }
}

int matchmaking_fetch_lobbies(struct TbNetworkSessionNameEntry *sessions, int max_sessions) {
    int len = http_request("GET", "/lobbies", NULL);
    if (!len) {
        return 0;
    }
    VALUE json;
    if (json_dom_parse(response_buffer, len, NULL, 0, &json, NULL) != 0) {
        return 0;
    }
    if (value_type(&json) != VALUE_ARRAY) {
        value_fini(&json);
        return 0;
    }
    int count = 0;
    for (size_t i = 0; i < value_array_size(&json) && count < max_sessions; i++) {
        VALUE *item = value_array_get(&json, i);
        if (value_type(item) != VALUE_DICT) {
            continue;
        }
        struct TbNetworkSessionNameEntry *s = &sessions[count++];
        memset(s, 0, sizeof(*s));
        s->in_use = 1;
        s->joinable = 1;
        VALUE *v;
        const char *name = "Unknown";
        if ((v = value_dict_get(item, "name")) && value_type(v) == VALUE_STRING) {
            name = value_string(v);
        }
        snprintf(s->text, SESSION_NAME_MAX_LEN, "%s's lobby", name);
        const char *ip = "";
        uint16_t port = 0;
        if ((v = value_dict_get(item, "ip")) && value_type(v) == VALUE_STRING) {
            ip = value_string(v);
        }
        if ((v = value_dict_get(item, "port")) && value_is_compatible(v, VALUE_INT32)) {
            port = (uint16_t)value_int32(v);
        }
        if (port != 0 && port != DEFAULT_SERVER_PORT) {
            TbBool is_ipv6 = (strchr(ip, ':') != NULL);
            if (is_ipv6) {
                snprintf(s->ip, sizeof(s->ip), "[%s]:%u", ip, port);
            } else {
                snprintf(s->ip, sizeof(s->ip), "%s:%u", ip, port);
            }
        } else {
            snprintf(s->ip, sizeof(s->ip), "%s", ip);
        }
    }
    value_fini(&json);
    return count;
}
