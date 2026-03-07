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

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
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
#define STUN_ATTR_XOR_MAPPED 0x0020U
#define STUN_RESPONSE_BUF_SIZE 512
#define HOLE_PUNCH_COUNT 5
#define HOLE_PUNCH_PAYLOAD_SIZE 8

#pragma pack(push, 1)
struct StunHeader {
    uint16_t type;
    uint16_t length;
    uint32_t magic;
    uint8_t  txid[12];
};

struct StunAttrHeader {
    uint16_t type;
    uint16_t length;
};
#pragma pack(pop)

uint16_t holepunch_stun_query(ENetHost *host, char *ip_out, size_t ip_len)
{
    ENetAddress stun_addr;
    LbNetLog("STUN: resolving %s\n", STUN_SERVER);
    if (enet_address_set_host(&stun_addr, ENET_ADDRESS_TYPE_IPV4, STUN_SERVER) < 0) {
        LbNetLog("STUN: failed to resolve %s\n", STUN_SERVER);
        return 0;
    }
    stun_addr.port = STUN_PORT;
    LbNetLog("STUN: resolved, sending binding request to port %u\n", (unsigned)STUN_PORT);

    static unsigned s_counter = 0;
    s_counter++;
    struct StunHeader req = {htons(STUN_BINDING_REQUEST), htons(0), htonl(STUN_MAGIC_COOKIE), {0}};
    memcpy(req.txid, &s_counter, sizeof(s_counter));
    LbNetLog("STUN: txid counter=%u, host socket=%d\n", s_counter, (int)host->socket);
    ENetBuffer send_buf = {sizeof(req), &req};
    ENetSocket sock = host->socket;
    ENetSocket tmp_sock = ENET_SOCKET_NULL;
    if (enet_socket_send(sock, &stun_addr, &send_buf, 1) < 0) {
        LbNetLog("STUN: host socket send failed, falling back to fresh IPv4 socket\n");
        tmp_sock = enet_socket_create(ENET_ADDRESS_TYPE_IPV4, ENET_SOCKET_TYPE_DATAGRAM);
        if (tmp_sock == ENET_SOCKET_NULL) {
            LbNetLog("STUN: failed to create fallback IPv4 socket\n");
            return 0;
        }
        if (enet_socket_send(tmp_sock, &stun_addr, &send_buf, 1) < 0) {
            LbNetLog("STUN: fallback IPv4 socket send failed\n");
            enet_socket_destroy(tmp_sock);
            return 0;
        }
        LbNetLog("STUN: sent via fallback socket (port will be host port %u)\n", (unsigned)host->address.port);
        sock = tmp_sock;
    } else {
        LbNetLog("STUN: sent via host socket\n");
    }

    DWORD deadline = GetTickCount() + STUN_TIMEOUT_MS;
    uint16_t result = 0;
    LbNetLog("STUN: waiting up to %d ms for response\n", STUN_TIMEOUT_MS);
    for (;;) {
        DWORD now = GetTickCount();
        if (now >= deadline) {
            LbNetLog("STUN: deadline exceeded\n");
            break;
        }
        enet_uint32 wait_flags = ENET_SOCKET_WAIT_RECEIVE;
        if (enet_socket_wait(sock, &wait_flags, deadline - now) < 0
            || !(wait_flags & ENET_SOCKET_WAIT_RECEIVE)) {
            LbNetLog("STUN: no response within %d ms\n", STUN_TIMEOUT_MS);
            break;
        }
        uint8_t resp[STUN_RESPONSE_BUF_SIZE];
        ENetAddress from;
        ENetBuffer recv_buf = {sizeof(resp), resp};
        int n = enet_socket_receive(sock, &from, &recv_buf, 1);
        LbNetLog("STUN: received %d bytes\n", n);
        if (n <= 0)
            continue;
        if (n < (int)sizeof(struct StunHeader)) {
            LbNetLog("STUN: packet too short (%d bytes), ignoring\n", n);
            break;
        }
        const struct StunHeader *hdr = (const struct StunHeader *)resp;
        uint16_t pkt_type = ntohs(hdr->type);
        uint32_t pkt_magic = ntohl(hdr->magic);
        int txid_match = (memcmp(hdr->txid, req.txid, sizeof(hdr->txid)) == 0);
        LbNetLog("STUN: pkt type=0x%04x magic=0x%08x txid_match=%d\n",
            (unsigned)pkt_type, (unsigned)pkt_magic, txid_match);
        if (pkt_type != STUN_BINDING_SUCCESS
            || pkt_magic != STUN_MAGIC_COOKIE
            || !txid_match) {
            LbNetLog("STUN: packet discarded (expected type=0x%04x magic=0x%08x)\n",
                (unsigned)STUN_BINDING_SUCCESS, (unsigned)STUN_MAGIC_COOKIE);
            continue;
        }
        int offset = (int)sizeof(struct StunHeader);
        int attrs_end = offset + (int)ntohs(hdr->length);
        if (attrs_end > n) attrs_end = n;
        LbNetLog("STUN: parsing attributes (attrs_end=%d)\n", attrs_end);
        char ip[64] = {0};
        uint16_t ext_port = 0;
        while (offset + 4 <= attrs_end) {
            const struct StunAttrHeader *attr = (const struct StunAttrHeader *)(resp + offset);
            uint16_t atype = ntohs(attr->type);
            uint16_t alen = ntohs(attr->length);
            LbNetLog("STUN: attribute type=0x%04x len=%u at offset=%d\n",
                (unsigned)atype, (unsigned)alen, offset);
            offset += 4;
            if (atype == STUN_ATTR_XOR_MAPPED && alen >= 8
                    && offset + alen <= attrs_end && resp[offset + 1] == 0x01) {
                uint16_t xor_port = ((uint16_t)resp[offset + 2] << 8) | resp[offset + 3];
                ext_port = xor_port ^ (uint16_t)(STUN_MAGIC_COOKIE >> 16);
                uint32_t xaddr;
                memcpy(&xaddr, resp + offset + 4, 4);
                uint32_t addr = ntohl(xaddr) ^ STUN_MAGIC_COOKIE;
                snprintf(ip, sizeof(ip), "%u.%u.%u.%u",
                    (addr >> 24) & 0xFFu, (addr >> 16) & 0xFFu,
                    (addr >> 8) & 0xFFu, addr & 0xFFu);
                LbNetLog("STUN: XOR-MAPPED-ADDRESS raw xor_port=0x%04x -> ext_port=%u, ip=%s\n",
                    (unsigned)xor_port, (unsigned)ext_port, ip);
                break;
            }
            offset += (alen + 3) & ~3;
        }
        if (!ext_port) {
            LbNetLog("STUN: no XOR-MAPPED-ADDRESS found in response\n");
            continue;
        }
        if (tmp_sock != ENET_SOCKET_NULL) {
            LbNetLog("STUN: used fallback socket, overriding ext_port %u -> host port %u\n",
                (unsigned)ext_port, (unsigned)host->address.port);
            ext_port = host->address.port;
        }
        LbNetLog("STUN: external address %s:%u\n", ip, (unsigned)ext_port);
        if (ip_out && ip_len > 0)
            snprintf(ip_out, ip_len, "%s", ip);
        result = ext_port;
        break;
    }
    if (!result)
        LbNetLog("STUN: failed to obtain mapped address\n");
    if (tmp_sock != ENET_SOCKET_NULL)
        enet_socket_destroy(tmp_sock);
    return result;
}

void holepunch_punch_to(ENetHost *host, const ENetAddress *target)
{
    LbNetLog("Holepunch: sending %d punch packets to port %u\n",
        HOLE_PUNCH_COUNT, (unsigned)target->port);
    static const uint8_t payload[HOLE_PUNCH_PAYLOAD_SIZE] = {0};
    ENetBuffer buf = {sizeof(payload), (void *)payload};
    for (int i = 0; i < HOLE_PUNCH_COUNT; i++) {
        int r = enet_socket_send(host->socket, target, &buf, 1);
        LbNetLog("Holepunch: punch %d/%d result=%d\n", i + 1, HOLE_PUNCH_COUNT, r);
    }
}
