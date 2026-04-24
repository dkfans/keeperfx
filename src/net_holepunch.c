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

#include <SDL2/SDL.h>
#include <enet6/enet.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "post_inc.h"

#define STUN_SERVER "stun.l.google.com"
#define STUN_PORT 19302
#define STUN_TIMEOUT_MS 500
#define STUN_MAGIC_COOKIE 0x2112A442U
#define STUN_BINDING_REQUEST 0x0001U
#define STUN_BINDING_SUCCESS 0x0101U
#define STUN_ATTRIBUTE_XOR_MAPPED 0x0020U
#define STUN_RESPONSE_BUFFER_SIZE 512
#define HOLE_PUNCH_COUNT 4
#define HOLE_PUNCH_PAYLOAD_SIZE 8
#define HOLE_PUNCH_LOG_INTERVAL_MS 1000

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

uint16_t holepunch_stun_query(ENetHost *host, char *output_ip, size_t output_ip_buffer_size)
{
    ENetAddress stun_server_address;
    if (enet_address_set_host(&stun_server_address, ENET_ADDRESS_TYPE_IPV4, STUN_SERVER) < 0) {
        LbNetLog("STUN: failed to resolve %s\n", STUN_SERVER);
        return 0;
    }
    stun_server_address.port = STUN_PORT;

    static unsigned s_transaction_counter = 0;
    s_transaction_counter++;
    struct StunHeader stun_request = {htons(STUN_BINDING_REQUEST), htons(0), htonl(STUN_MAGIC_COOKIE), {0}};
    memcpy(stun_request.transaction_id, &s_transaction_counter, sizeof(s_transaction_counter));
    ENetBuffer send_buffer = {.data = &stun_request, .dataLength = sizeof(stun_request)};
    ENetSocket send_socket = host->socket;
    ENetSocket fallback_socket = ENET_SOCKET_NULL;
    int send_succeeded = (enet_socket_send(send_socket, &stun_server_address, &send_buffer, 1) >= 0);
    if (!send_succeeded) {
        ENetAddress mapped_stun_address = stun_server_address;
        enet_address_convert_ipv6(&mapped_stun_address);
        send_succeeded = (enet_socket_send(send_socket, &mapped_stun_address, &send_buffer, 1) >= 0);
    }
    if (!send_succeeded) {
        LbNetLog("STUN: host socket send failed, falling back to fresh IPv4 socket\n");
        fallback_socket = enet_socket_create(ENET_ADDRESS_TYPE_IPV4, ENET_SOCKET_TYPE_DATAGRAM);
        if (fallback_socket == ENET_SOCKET_NULL) {
            LbNetLog("STUN: failed to create fallback IPv4 socket\n");
            return 0;
        }
        if (enet_socket_send(fallback_socket, &stun_server_address, &send_buffer, 1) < 0) {
            LbNetLog("STUN: fallback IPv4 socket send failed\n");
            enet_socket_destroy(fallback_socket);
            return 0;
        }
        send_socket = fallback_socket;
    }

    Uint32 timeout_deadline = SDL_GetTicks() + STUN_TIMEOUT_MS;
    uint16_t external_port_result = 0;
    for (;;) {
        Uint32 now = SDL_GetTicks();
        if (now >= timeout_deadline)
            break;
        enet_uint32 socket_wait_flags = ENET_SOCKET_WAIT_RECEIVE;
        if (enet_socket_wait(send_socket, &socket_wait_flags, timeout_deadline - now) < 0
            || !(socket_wait_flags & ENET_SOCKET_WAIT_RECEIVE))
            break;
        uint8_t response_buffer[STUN_RESPONSE_BUFFER_SIZE];
        ENetBuffer receive_buffer = {.data = response_buffer, .dataLength = sizeof(response_buffer)};
        int bytes_received = enet_socket_receive(send_socket, NULL, &receive_buffer, 1);
        if (bytes_received <= 0)
            continue;
        if (bytes_received < (int)sizeof(struct StunHeader))
            break;
        const struct StunHeader *stun_response_header = (const struct StunHeader *)response_buffer;
        if (ntohs(stun_response_header->type) != STUN_BINDING_SUCCESS
            || ntohl(stun_response_header->magic) != STUN_MAGIC_COOKIE
            || memcmp(stun_response_header->transaction_id, stun_request.transaction_id, sizeof(stun_response_header->transaction_id)) != 0)
            continue;
        int attribute_offset = (int)sizeof(struct StunHeader);
        int attributes_end = attribute_offset + (int)ntohs(stun_response_header->length);
        if (attributes_end > bytes_received) attributes_end = bytes_received;
        char mapped_ip[64] = {0};
        uint16_t external_port = 0;
        while (attribute_offset + 4 <= attributes_end) {
            const struct StunAttrHeader *stun_attribute = (const struct StunAttrHeader *)(response_buffer + attribute_offset);
            uint16_t attribute_type = ntohs(stun_attribute->type);
            uint16_t attribute_length = ntohs(stun_attribute->length);
            attribute_offset += 4;
            if (attribute_type == STUN_ATTRIBUTE_XOR_MAPPED && attribute_length >= 8
                    && attribute_offset + attribute_length <= attributes_end && response_buffer[attribute_offset + 1] == 0x01) {
                uint16_t xor_encoded_port = ((uint16_t)response_buffer[attribute_offset + 2] << 8) | response_buffer[attribute_offset + 3];
                external_port = xor_encoded_port ^ (uint16_t)(STUN_MAGIC_COOKIE >> 16);
                uint32_t xor_encoded_address;
                memcpy(&xor_encoded_address, response_buffer + attribute_offset + 4, 4);
                uint32_t decoded_address = ntohl(xor_encoded_address) ^ STUN_MAGIC_COOKIE;
                snprintf(mapped_ip, sizeof(mapped_ip), "%u.%u.%u.%u",
                    (decoded_address >> 24) & 0xFFu, (decoded_address >> 16) & 0xFFu,
                    (decoded_address >> 8) & 0xFFu, decoded_address & 0xFFu);
                break;
            }
            attribute_offset += (attribute_length + 3) & ~3;
        }
        if (!external_port)
            continue;
        if (fallback_socket != ENET_SOCKET_NULL)
            external_port = host->address.port;
        LbNetLog("STUN: external address %s:%u\n", mapped_ip, (unsigned)external_port);
        if (output_ip && output_ip_buffer_size > 0)
            snprintf(output_ip, output_ip_buffer_size, "%s", mapped_ip);
        external_port_result = external_port;
        break;
    }
    if (!external_port_result)
        LbNetLog("STUN: failed to obtain mapped address\n");
    if (fallback_socket != ENET_SOCKET_NULL)
        enet_socket_destroy(fallback_socket);
    return external_port_result;
}

static int send_and_burst(ENetSocket socket_handle, const ENetAddress *address, ENetBuffer *buffer)
{
    if (enet_socket_send(socket_handle, address, buffer, 1) < 0)
        return 0;
    for (int i = 1; i < HOLE_PUNCH_COUNT; i++)
        enet_socket_send(socket_handle, address, buffer, 1);
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
    Uint32 current_time = SDL_GetTicks();
    if (last_failure_log_time == 0 || SDL_TICKS_PASSED(current_time, last_failure_log_time + HOLE_PUNCH_LOG_INTERVAL_MS)) {
        last_failure_log_time = current_time;
        LbNetLog("Holepunch: send failed\n");
    }
}
