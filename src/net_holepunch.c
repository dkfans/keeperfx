/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_holepunch.cpp
 *     UDP hole punching via STUN.
 * @par Purpose:
 *     holepunch_stun_query() sends a STUN Binding Request (RFC 5389) from the
 *     ENet host's own socket.  This serves two purposes:
 *       1. Creates a NAT table entry so incoming ENet connections reach us even
 *          when UPnP/NAT-PMP is unavailable.
 *       2. Logs our external IP:port so the host can share it with others.
 *
 *     holepunch_punch_to() sends a burst of small UDP datagrams to the server
 *     before enet_host_connect() is called.  This opens a mapping in the
 *     client's cone NAT and primes port-restricted cone NATs on the server.
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
#include "net_holepunch.h"
#include "bflib_basics.h"

#include <SDL3/SDL.h>
#include <enet6/enet.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "post_inc.h"

#define STUN_TIMEOUT_MS 500
#define STUN_MAGIC_COOKIE 0x2112A442U
#define STUN_BINDING_REQUEST 0x0001U
#define STUN_BINDING_SUCCESS 0x0101U
#define STUN_BINDING_INDICATION 0x0011U
#define STUN_KEEPALIVE_MS 1500
#define STUN_ATTRIBUTE_XOR_MAPPED 0x0020U
#define STUN_ATTRIBUTE_MAPPED 0x0001U
#define STUN_RESPONSE_BUFFER_SIZE 512
#define HOLE_PUNCH_COUNT 4
#define HOLE_PUNCH_PAYLOAD_SIZE 8
#define HOLE_PUNCH_LOG_INTERVAL_MS 1000

static ENetAddress keepalive_address;
static ENetSocket keepalive_socket = ENET_SOCKET_NULL;
static Uint32 keepalive_time;

#pragma pack(push, 1)
struct StunHeader {
    uint16_t type;
    uint16_t length;
    uint32_t magic;
    uint8_t  transaction_id[12];
};

struct StunAttrHeader {
    uint16_t type;
    uint16_t length;
};
#pragma pack(pop)

static uint16_t stun_query_server(ENetHost *host, const ENetAddress *server, char *output_ip, size_t output_ip_buffer_size)
{
    ENetAddress stun_server_address = *server;
    static unsigned s_transaction_counter = 0;
    s_transaction_counter++;
    struct StunHeader stun_request = {htons(STUN_BINDING_REQUEST), htons(0), htonl(STUN_MAGIC_COOKIE), {0}};
    memcpy(stun_request.transaction_id, &s_transaction_counter, sizeof(s_transaction_counter));
    ENetBuffer send_buffer = {.data = &stun_request, .dataLength = sizeof(stun_request)};
    ENetSocket send_socket = host->socket;
    int send_succeeded = (enet_socket_send(send_socket, &stun_server_address, &send_buffer, 1) >= 0);
    if (!send_succeeded) {
        ENetAddress mapped_stun_address = stun_server_address;
        enet_address_convert_ipv6(&mapped_stun_address);
        send_succeeded = (enet_socket_send(send_socket, &mapped_stun_address, &send_buffer, 1) >= 0);
    }
    if (!send_succeeded) {
        LbNetLog("STUN: host socket send failed\n");
        return 0;
    }

    Uint32 timeout_deadline = (Uint32)SDL_GetTicks() + STUN_TIMEOUT_MS;
    uint16_t external_port_result = 0;
    for (;;) {
        Uint32 now = (Uint32)SDL_GetTicks();
        if (now >= timeout_deadline)
            break;
        enet_uint32 socket_wait_flags = ENET_SOCKET_WAIT_RECEIVE;
        if (enet_socket_wait(send_socket, &socket_wait_flags, timeout_deadline - now) < 0 || !(socket_wait_flags & ENET_SOCKET_WAIT_RECEIVE))
            break;
        uint8_t response_buffer[STUN_RESPONSE_BUFFER_SIZE];
        ENetBuffer receive_buffer = {.data = response_buffer, .dataLength = sizeof(response_buffer)};
        ENetAddress source;
        int bytes_received = enet_socket_receive(send_socket, &source, &receive_buffer, 1);
        if (bytes_received <= 0)
            continue;
        if (bytes_received < (int)sizeof(struct StunHeader))
            continue;
        ENetAddress expected_source = stun_server_address;
        enet_address_convert_ipv6(&expected_source);
        enet_address_convert_ipv6(&source);
        if (!enet_address_equal(&source, &expected_source))
            continue;
        const struct StunHeader *stun_response_header = (const struct StunHeader *)response_buffer;
        if (ntohs(stun_response_header->type) != STUN_BINDING_SUCCESS || ntohl(stun_response_header->magic) != STUN_MAGIC_COOKIE || memcmp(stun_response_header->transaction_id, stun_request.transaction_id, sizeof(stun_response_header->transaction_id)) != 0)
            continue;
        int attribute_offset = (int)sizeof(struct StunHeader);
        int attributes_end = attribute_offset + (int)ntohs(stun_response_header->length);
        if (attributes_end > bytes_received)
            continue;
        char mapped_ip[64] = {0};
        uint16_t external_port = 0;
        while (attribute_offset + 4 <= attributes_end) {
            const struct StunAttrHeader *stun_attribute = (const struct StunAttrHeader *)(response_buffer + attribute_offset);
            uint16_t attribute_type = ntohs(stun_attribute->type);
            uint16_t attribute_length = ntohs(stun_attribute->length);
            attribute_offset += 4;
            if ((attribute_type == STUN_ATTRIBUTE_XOR_MAPPED || attribute_type == STUN_ATTRIBUTE_MAPPED) && attribute_length >= 8 && attribute_offset + attribute_length <= attributes_end && response_buffer[attribute_offset + 1] == 0x01) {
                external_port = ((uint16_t)response_buffer[attribute_offset + 2] << 8) | response_buffer[attribute_offset + 3];
                uint32_t decoded_address;
                memcpy(&decoded_address, response_buffer + attribute_offset + 4, 4);
                decoded_address = ntohl(decoded_address);
                if (attribute_type == STUN_ATTRIBUTE_XOR_MAPPED) {
                    external_port ^= (uint16_t)(STUN_MAGIC_COOKIE >> 16);
                    decoded_address ^= STUN_MAGIC_COOKIE;
                }
                snprintf(mapped_ip, sizeof(mapped_ip), "%u.%u.%u.%u",
                    (decoded_address >> 24) & 0xFFu, (decoded_address >> 16) & 0xFFu,
                    (decoded_address >> 8) & 0xFFu, decoded_address & 0xFFu);
                break;
            }
            attribute_offset += (attribute_length + 3) & ~3;
        }
        if (!external_port)
            continue;
        LbNetLog("STUN: local port %u -> external %s:%u\n", (unsigned)host->address.port, mapped_ip, (unsigned)external_port);
        if (output_ip && output_ip_buffer_size > 0)
            snprintf(output_ip, output_ip_buffer_size, "%s", mapped_ip);
        external_port_result = external_port;
        break;
    }
    if (!external_port_result)
        LbNetLog("STUN: failed to obtain mapped address\n");
    return external_port_result;
}

uint16_t holepunch_stun_query(ENetHost *host, char *output_ip, size_t output_ip_buffer_size)
{
    static const char *servers[] = {"stun.l.google.com", "stun.antisip.com", "stun1.l.google.com"};
    static const uint16_t ports[] = {19302, 3478, 19302};
    uint16_t result = 0;
    char previous_ip[64] = {0};
    keepalive_socket = ENET_SOCKET_NULL;
    if (output_ip && output_ip_buffer_size > 0)
        output_ip[0] = '\0';
    for (size_t i = 0; i < sizeof(servers) / sizeof(servers[0]); i++) {
        ENetAddress server;
        if (enet_address_set_host(&server, ENET_ADDRESS_TYPE_IPV4, servers[i]) < 0) {
            LbNetLog("STUN: failed to resolve %s\n", servers[i]);
            continue;
        }
        server.port = ports[i];
        char mapped_ip[64] = {0};
        uint16_t port = stun_query_server(host, &server, mapped_ip, sizeof(mapped_ip));
        if (!port) {
            LbNetLog("STUN: %s:%u unavailable, trying another server\n", servers[i], (unsigned)server.port);
            continue;
        }
        keepalive_address = server;
        keepalive_socket = host->socket;
        keepalive_time = (Uint32)SDL_GetTicks();
        if (output_ip && output_ip_buffer_size > 0)
            snprintf(output_ip, output_ip_buffer_size, "%s", mapped_ip);
        if (result) {
            if (result != port || strcmp(previous_ip, mapped_ip) != 0) {
                LbNetLog("STUN: destination-dependent mapping detected (%s:%u -> %s:%u)\n", previous_ip, (unsigned)result, mapped_ip, (unsigned)port);
            } else {
                LbNetLog("STUN: mapping unchanged across two servers\n");
            }
            return port;
        }
        result = port;
        snprintf(previous_ip, sizeof(previous_ip), "%s", mapped_ip);
    }
    return result;
}

void holepunch_stun_keepalive(ENetHost *host)
{
    Uint32 now = (Uint32)SDL_GetTicks();
    if (keepalive_socket == ENET_SOCKET_NULL || host->socket != keepalive_socket || (Uint32)(now - keepalive_time) < STUN_KEEPALIVE_MS)
        return;
    keepalive_time = now;
    struct StunHeader indication = {htons(STUN_BINDING_INDICATION), 0, htonl(STUN_MAGIC_COOKIE), {0}};
    memcpy(indication.transaction_id, &now, sizeof(now));
    ENetBuffer buffer = {.data = &indication, .dataLength = sizeof(indication)};
    if (enet_socket_send(host->socket, &keepalive_address, &buffer, 1) < 0) {
        ENetAddress mapped_address = keepalive_address;
        enet_address_convert_ipv6(&mapped_address);
        if (enet_socket_send(host->socket, &mapped_address, &buffer, 1) < 0)
            LbNetLog("STUN: keepalive send failed\n");
    }
}

static int send_and_burst(ENetSocket socket_handle, const ENetAddress *address, ENetBuffer *buffer)
{
    if (enet_socket_send(socket_handle, address, buffer, 1) < 0)
        return 0;
    for (int i = 1; i < HOLE_PUNCH_COUNT; i++)
        enet_socket_send(socket_handle, address, buffer, 1);
    return 1;
}

static int address_family_is_ipv4(const ENetAddress *address)
{
    if (address->type == ENET_ADDRESS_TYPE_IPV4)
        return 1;
    if (address->type != ENET_ADDRESS_TYPE_IPV6)
        return 0;
    for (int i = 0; i < 5; i++) {
        if (address->host.v6[i] != 0)
            return 0;
    }
    return address->host.v6[5] == 0xFFFF;
}

int holepunch_handle_packet(ENetHost *host, ENetAddress *expected, size_t expected_count, int *received_mask)
{
    static const uint8_t punch_payload[HOLE_PUNCH_PAYLOAD_SIZE] = {0};
    *received_mask = 0;
    if (host->receivedDataLength != HOLE_PUNCH_PAYLOAD_SIZE || memcmp(host->receivedData, punch_payload, HOLE_PUNCH_PAYLOAD_SIZE) != 0)
        return 0;
    ENetAddress source = host->receivedAddress;
    for (size_t i = 0; i < expected_count; i++) {
        if (address_family_is_ipv4(&source) != address_family_is_ipv4(&expected[i]))
            continue;
        ENetAddress mapped_source = source;
        ENetAddress mapped_expected = expected[i];
        enet_address_convert_ipv6(&mapped_source);
        enet_address_convert_ipv6(&mapped_expected);
        if (!enet_address_equal(&mapped_source, &mapped_expected)) {
            char source_address[64] = {0};
            char advertised_address[64] = {0};
            enet_address_get_host_ip(&source, source_address, sizeof(source_address));
            enet_address_get_host_ip(&expected[i], advertised_address, sizeof(advertised_address));
            LbNetLog("Holepunch: peer at %s (advertised %s)\n", source_address, advertised_address);
        }
        expected[i] = source;
        *received_mask |= 1 << i;
        break;
    }
    return 1;
}

void holepunch_punch_to(ENetHost *host, const ENetAddress *target)
{
    static const uint8_t punch_payload[HOLE_PUNCH_PAYLOAD_SIZE] = {0};
    static Uint32 last_failure_log_time = 0;
    ENetBuffer send_buffer = {.data = (void *)punch_payload, .dataLength = sizeof(punch_payload)};
    if (send_and_burst(host->socket, target, &send_buffer))
        return;
    if (target->type == ENET_ADDRESS_TYPE_IPV4) {
        ENetAddress mapped_target = *target;
        enet_address_convert_ipv6(&mapped_target);
        if (send_and_burst(host->socket, &mapped_target, &send_buffer))
            return;
    }
    Uint32 current_time = (Uint32)SDL_GetTicks();
    if (last_failure_log_time == 0 || (Sint32)((last_failure_log_time + HOLE_PUNCH_LOG_INTERVAL_MS) - current_time) <= 0) {
        last_failure_log_time = current_time;
        LbNetLog("Holepunch: send failed\n");
    }
}
