#include "pre_inc.h"
#include "net_lan.h"
#include "net_lobby.h"
#include <json-dom.h>
#include "bflib_basics.h"

#include <enet6/enet.h>
#include <SDL3/SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "post_inc.h"

#define LAN_DISCOVERY_PORT               5557
#define LAN_BROADCAST_INTERVAL_MS        1000
#define LAN_SESSION_TIMEOUT_MS           4000
#define LAN_DISCOVER_MSG                 "KEEPERFX_DISCOVER"
#define LAN_HOST_REPLY_PREFIX            "KEEPERFX_HOST:"
#define LAN_IP_MAX                       64
#define LAN_MSG_MAX                      4096

struct TbNetworkSessionNameEntry lan_sessions[LAN_SESSIONS_MAX];
int lan_session_count = 0;

struct LanSessionCache {
    struct TbNetworkSessionNameEntry session;
    Uint32 last_seen_milliseconds;
};

static struct LanSessionCache session_cache[LAN_SESSIONS_MAX];
static int session_cache_count = 0;

static ENetSocket host_socket = ENET_SOCKET_NULL;
static ENetSocket joiner_socket = ENET_SOCKET_NULL;
static int joiner_socket_failed = 0;
static char lan_hosted_name[SESSION_NAME_MAX_LEN] = {0};
static uint16_t lan_hosted_port = 0;
static Uint32 last_broadcast_milliseconds = 0;

static void socket_close(ENetSocket *socket)
{
    if (*socket != ENET_SOCKET_NULL) {
        enet_socket_destroy(*socket);
        *socket = ENET_SOCKET_NULL;
    }
}

static int receive_packet(ENetSocket socket, ENetAddress *sender, char *buffer, int buffer_size)
{
    enet_uint32 wait_flags = ENET_SOCKET_WAIT_RECEIVE;
    if (enet_socket_wait(socket, &wait_flags, 0) < 0 || !(wait_flags & ENET_SOCKET_WAIT_RECEIVE))
        return -1;
    ENetBuffer receive_buffer = {.data = buffer, .dataLength = (size_t)(buffer_size - 1)};
    int bytes = enet_socket_receive(socket, sender, &receive_buffer, 1);
    if (bytes <= 0)
        return -1;
    buffer[bytes] = '\0';
    return bytes;
}

static void strip_port_from_lan_sender_ipv4_address(const ENetAddress *sender, char *address)
{
    if (sender->type != ENET_ADDRESS_TYPE_IPV4)
        return;
    char *port_separator = strrchr(address, ':');
    if (port_separator == NULL)
        return;
    *port_separator = '\0';
}

void lan_host_start(const char *name, uint16_t port)
{
    socket_close(&host_socket);
    host_socket = enet_socket_create(ENET_ADDRESS_TYPE_IPV4, ENET_SOCKET_TYPE_DATAGRAM);
    if (host_socket == ENET_SOCKET_NULL) {
        LbNetLog("LAN: failed to create host discovery socket\n");
        return;
    }
    enet_socket_set_option(host_socket, ENET_SOCKOPT_REUSEADDR, 1);
    enet_socket_set_option(host_socket, ENET_SOCKOPT_NONBLOCK, 1);
    ENetAddress bind_address;
    enet_address_build_any(&bind_address, ENET_ADDRESS_TYPE_IPV4);
    bind_address.port = LAN_DISCOVERY_PORT;
    if (enet_socket_bind(host_socket, &bind_address) < 0) {
        LbNetLog("LAN: failed to bind host discovery socket to port %d\n", LAN_DISCOVERY_PORT);
        socket_close(&host_socket);
        return;
    }
    snprintf(lan_hosted_name, SESSION_NAME_MAX_LEN, "%s", name);
    lan_hosted_port = port;
    LbNetLog("LAN: host discovery socket ready, discovery_port=%d game_port=%d\n", LAN_DISCOVERY_PORT, port);
}

void lan_host_update(void)
{
    if (host_socket == ENET_SOCKET_NULL)
        return;
    char buffer[LAN_MSG_MAX];
    ENetAddress sender;
    while (receive_packet(host_socket, &sender, buffer, sizeof(buffer)) > 0) {
        if (strcmp(buffer, LAN_DISCOVER_MSG) != 0)
            continue;
        char metadata[SESSION_METADATA_MAX];
        net_lobby_metadata(metadata);
        char reply[LAN_MSG_MAX];
        int reply_length = snprintf(reply, sizeof(reply), LAN_HOST_REPLY_PREFIX "|%s:%d", lan_hosted_name, (int)lan_hosted_port);
        if (!metadata[0] || reply_length < 0 || reply_length + strlen(metadata) + 4 >= sizeof(reply)) {
            continue;
        }
        reply_length++;
        reply_length += snprintf(reply + reply_length, sizeof(reply) - reply_length, "{%s}", metadata);
        ENetBuffer send_buffer = {.data = reply, .dataLength = (size_t)reply_length};
        enet_socket_send(host_socket, &sender, &send_buffer, 1);
    }
}

void lan_refresh_sessions(void)
{
    if (joiner_socket == ENET_SOCKET_NULL) {
        if (joiner_socket_failed)
            return;
        joiner_socket = enet_socket_create(ENET_ADDRESS_TYPE_IPV4, ENET_SOCKET_TYPE_DATAGRAM);
        if (joiner_socket == ENET_SOCKET_NULL) {
            LbNetLog("LAN: failed to create joiner discovery socket\n");
            joiner_socket_failed = 1;
            return;
        }
        enet_socket_set_option(joiner_socket, ENET_SOCKOPT_BROADCAST, 1);
        enet_socket_set_option(joiner_socket, ENET_SOCKOPT_NONBLOCK, 1);
        ENetAddress bind_address;
        enet_address_build_any(&bind_address, ENET_ADDRESS_TYPE_IPV4);
        bind_address.port = 0;
        if (enet_socket_bind(joiner_socket, &bind_address) < 0) {
            LbNetLog("LAN: failed to bind joiner discovery socket\n");
            socket_close(&joiner_socket);
            joiner_socket_failed = 1;
            return;
        }
    }
    Uint32 now = (Uint32)SDL_GetTicks();
    if (now - last_broadcast_milliseconds >= LAN_BROADCAST_INTERVAL_MS) {
        ENetAddress broadcast_address;
        enet_address_set_host_ip(&broadcast_address, "255.255.255.255");
        broadcast_address.port = LAN_DISCOVERY_PORT;
        char message[] = LAN_DISCOVER_MSG;
        ENetBuffer send_buffer = {.data = message, .dataLength = sizeof(message) - 1};
        enet_socket_send(joiner_socket, &broadcast_address, &send_buffer, 1);
        last_broadcast_milliseconds = now;
    }
    char buffer[LAN_MSG_MAX];
    ENetAddress sender;
    int packet_length;
    while ((packet_length = receive_packet(joiner_socket, &sender, buffer, sizeof(buffer))) > 0) {
        const char *metadata = "{}";
        size_t legacy_length = strlen(buffer);
        if (legacy_length + 1 < packet_length) {
            metadata = buffer + legacy_length + 1;
        }
        if (strncmp(buffer, LAN_HOST_REPLY_PREFIX, sizeof(LAN_HOST_REPLY_PREFIX) - 1) != 0)
            continue;
        char *payload = buffer + sizeof(LAN_HOST_REPLY_PREFIX) - 1;
        char parsed_lobby_id[SESSION_LOBBY_ID_MAX_LEN] = {0};
        char *pipe_separator = strchr(payload, '|');
        if (pipe_separator) {
            int id_length = (int)(pipe_separator - payload);
            if (id_length >= SESSION_LOBBY_ID_MAX_LEN)
                id_length = SESSION_LOBBY_ID_MAX_LEN - 1;
            snprintf(parsed_lobby_id, sizeof(parsed_lobby_id), "%.*s", id_length, payload);
            payload = pipe_separator + 1;
        }
        char *port_separator = strrchr(payload, ':');
        if (!port_separator)
            continue;
        *port_separator = '\0';
        int game_port = atoi(port_separator + 1);
        if (game_port <= 0 || game_port > 65535) {
            continue;
        }
        char sender_ip[LAN_IP_MAX];
        if (enet_address_get_host_ip(&sender, sender_ip, sizeof(sender_ip)) < 0)
            continue;
        strip_port_from_lan_sender_ipv4_address(&sender, sender_ip);
        if (host_socket != ENET_SOCKET_NULL && game_port == lan_hosted_port && strcmp(payload, lan_hosted_name) == 0)
            continue;
        char join_address[SESSION_LOBBY_ID_MAX_LEN];
        snprintf(join_address, sizeof(join_address), "LAN:%s:%d", sender_ip, game_port);
        struct LanSessionCache *entry = NULL;
        for (int i = 0; i < session_cache_count; i++) {
            if (strcmp(session_cache[i].session.join_address, join_address) == 0 && strcmp(session_cache[i].session.text, payload) == 0) {
                entry = &session_cache[i];
                break;
            }
        }
        if (!entry && session_cache_count < LAN_SESSIONS_MAX) {
            entry = &session_cache[session_cache_count++];
        }
        if (entry) {
            snprintf(entry->session.text, sizeof(entry->session.text), "%s", payload);
            snprintf(entry->session.lobby_id, sizeof(entry->session.lobby_id), "%s", parsed_lobby_id);
            snprintf(entry->session.join_address, sizeof(entry->session.join_address), "%s", join_address);
            entry->session.in_use = 1;
            VALUE root = {0};
            json_dom_parse(metadata, strlen(metadata), NULL, 0, &root, NULL);
            net_session_parse_metadata(&entry->session, &root);
            value_fini(&root);
            entry->last_seen_milliseconds = now;
        }
    }
    lan_session_count = 0;
    int write_index = 0;
    for (int i = 0; i < session_cache_count; i++) {
        if (now - session_cache[i].last_seen_milliseconds > LAN_SESSION_TIMEOUT_MS)
            continue;
        session_cache[write_index++] = session_cache[i];
        struct TbNetworkSessionNameEntry *entry = &lan_sessions[lan_session_count++];
        *entry = session_cache[i].session;
        entry->id = lan_session_count;
    }
    session_cache_count = write_index;
}

void lan_shutdown(void)
{
    if (host_socket != ENET_SOCKET_NULL)
        LbNetLog("LAN: host discovery socket closed\n");
    socket_close(&host_socket);
    lan_hosted_name[0] = '\0';
    lan_hosted_port = 0;
    socket_close(&joiner_socket);
    joiner_socket_failed = 0;
    last_broadcast_milliseconds = 0;
    session_cache_count = 0;
    lan_session_count = 0;
}
