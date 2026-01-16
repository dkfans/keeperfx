/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @author   KeeperFX Team
 * @date     18 Oct 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "pre_inc.h"
#include "bflib_enet.h"
#include "bflib_network.h"
#include "bflib_math.h"
#include "net_portforward.h"
#include "game_legacy.h"
#include "player_data.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <enet/enet.h>
#include <cstddef>
#include <climits>

#include "post_inc.h"

#define NUM_CHANNELS 2
#define DEFAULT_PORT 5556

namespace
{
    NetDropCallback g_drop_callback = nullptr;
    ENetHost *host = nullptr;
    ENetPeer *client_peer = nullptr;

    // List
    ENetPacket *oldest_packet = nullptr;
    ENetPacket *newest_packet = nullptr;
    int incoming_queue_size = 0;

    TbError bf_enet_init(NetDropCallback drop_callback)
    {
        if (enet_initialize())
            return Lb_FAIL;
        g_drop_callback = drop_callback;
        return Lb_OK;
    }

    void host_destroy()
    {
        if (oldest_packet)
        {
            for (ENetPacket *p = oldest_packet; p != nullptr;)
            {
                ENetPacket *pp = p;
                p = static_cast<ENetPacket *>(p->userData);
                enet_packet_destroy(pp);
            }

            oldest_packet = nullptr;
            newest_packet = nullptr;
            incoming_queue_size = 0;
        }
        if (client_peer)
        {
            client_peer = nullptr;
        }
        if (host)
        {
            enet_host_destroy(host);
            host = nullptr;
        }
    }

    void bf_enet_exit()
    {
        port_forward_remove_mapping();
        host_destroy();
        g_drop_callback = nullptr;
        enet_deinitialize();
    }

    /**
     * Sets this service provider up as a host for a new network game.
     * @param session String representing the network game to be hosted.
     *  This could be a hostname:port pair for TCP for instance.
     * @param options
     * @return Lb_FAIL or Lb_OK
     */
    TbError bf_enet_host(const char *session, void *options)
    {
        ENetAddress address = {.host = 0,
                            .port = DEFAULT_PORT };
        if (!*session)
            return Lb_FAIL;
        int port = atoi(session);
        if (port > 0)
            address.port = port;
        host = enet_host_create(&address, 4, NUM_CHANNELS, 0, 0);
        if (!host) {
            return Lb_FAIL;
        }
        enet_host_compress_with_range_coder(host);
        port_forward_add_mapping(address.port);
        return Lb_OK;
    }

    int wait_for_connect(int timeout)
    {
        ENetEvent ev;
        int ret;
        while (
                (ret = enet_host_service(host, &ev, timeout)) > 0
                )
        {
            if (ev.type == ENET_EVENT_TYPE_CONNECT)
            {
                return 0;
            }
            else
            {
                fprintf(stderr, "Unexpected event %d\n", ev.type);
            }
        }
        if (ret < 0)
        {
            fprintf(stderr, "Unable to connect! %d\n", ret);
            ERRORLOG("Unable to connect: %d", ret);
        }
        return 1;
    }

    TbError bf_enet_join(const char *session, void *options)
    {
        char buf[64] = {0};
        const char *P;
        char *E;
        ENetAddress address = {ENET_HOST_ANY, ENET_PORT_ANY};
        host = enet_host_create(&address, 4, NUM_CHANNELS, 0, 0);
        if (!host)
        {
            return Lb_FAIL;
        }
        enet_host_compress_with_range_coder(host);
        P = strchr(session,':');
        if (P)
        {
            strncpy(buf, session, P-session);
            address.port = strtoul(P+1, &E, 10);
            if (address.port == 0)
            {
                host_destroy();
                return Lb_FAIL;
            }
        }
        else
        {
            strncpy(buf, session, sizeof(buf) - 1);
            address.port = DEFAULT_PORT;
        }
        if (enet_address_set_host(&address, buf) < 0)
        {
            host_destroy();
            return Lb_FAIL;
        }
        client_peer = enet_host_connect(host, &address, NUM_CHANNELS, 0);
        if (!client_peer)
        {
            return Lb_FAIL;
        }
        if (wait_for_connect(TIMEOUT_ENET_CONNECT))
        {
            host_destroy();
            return Lb_FAIL;
        }
        return Lb_OK;
    }

    /*
     * @returns -1 if error, +1 if there is a packet, 0 if timeoout or no events
     */
    int bf_enet_read_event(NetNewUserCallback new_user, uint timeout)
    {
        ENetEvent ev;
        int ret;
        NetUserId user_id;
        if (!host)
        {
            return -1;
        }
        if ((ret = enet_host_service(host, &ev, timeout)) != 0)
        {
            if (ret < 0)
            {
                NETDBG(1, "enet_host -> %d", ret);
                return ret;
            }
            switch (ev.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    if (new_user(&user_id))
                    {
                        ev.peer->data = reinterpret_cast<void *>(user_id);
                    }
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    user_id = NetUserId(reinterpret_cast<ptrdiff_t>(ev.peer->data));
                    g_drop_callback(user_id, NETDROP_ERROR);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    if (oldest_packet == nullptr)
                    {
                        newest_packet = ev.packet;
                        oldest_packet = newest_packet;
                        incoming_queue_size = 1;
                    }
                    else
                    {
                        newest_packet->userData = ev.packet;
                        newest_packet = ev.packet;
                        newest_packet->userData = nullptr;
                        incoming_queue_size +=1;
                        if (incoming_queue_size > 50)
                        {
                            fprintf(stderr, "Too many packets %d\n", incoming_queue_size);
                            WARNLOG("Too many packets %d", incoming_queue_size);
                        }
                    }
                    return 1;
                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
        return 0;
    }

    /**
     * Checks for new connections.
     * @param new_user Call back if a new user has connected.
     */
    void bf_enet_update(NetNewUserCallback new_user)
    {
        int packets_read = 0;
        const int MAX_PACKETS_PER_UPDATE = 100;
        while (packets_read < MAX_PACKETS_PER_UPDATE && bf_enet_read_event(new_user, 0))
        {
            packets_read++;
        }
    }

    /**
     * Sends a message buffer to a certain user.
     * @param destination Destination user.
     * @param buffer
     * @param size Must be > 0
     */
    void bf_enet_sendmsg_single(NetUserId destination, const char *buffer, size_t size)
    {
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_RELIABLE);
        if (client_peer) // Just send to server
        {
            enet_peer_send(client_peer, ENET_CHANNEL_RELIABLE, packet);
        }
        else
        {
            for (ENetPeer *currentPeer = host->peers; currentPeer < &host->peers[host -> peerCount]; ++currentPeer)
            {
                if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
                    continue;
                if (NetUserId(reinterpret_cast<ptrdiff_t>(currentPeer->data)) == destination)
                {
                    enet_peer_send(currentPeer, ENET_CHANNEL_RELIABLE, packet);
                }
            }
        }
        enet_host_flush(host);
    }

    /**
     * Sends a message buffer to a certain user using unsequenced delivery.
     * @param destination Destination user.
     * @param buffer
     * @param size Must be > 0
     */
    void bf_enet_sendmsg_single_unsequenced(NetUserId destination, const char *buffer, size_t size)
    {
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_UNSEQUENCED);
        if (client_peer) // Just send to server
        {
            enet_peer_send(client_peer, ENET_CHANNEL_UNSEQUENCED, packet);
        }
        else
        {
            for (ENetPeer *currentPeer = host->peers; currentPeer < &host->peers[host -> peerCount]; ++currentPeer)
            {
                if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
                    continue;
                if (NetUserId(reinterpret_cast<ptrdiff_t>(currentPeer->data)) == destination)
                {
                    enet_peer_send(currentPeer, ENET_CHANNEL_UNSEQUENCED, packet);
                }
            }
        }
        enet_host_flush(host);
    }

    /**
     * Sends a message buffer to all remote users.
     * @param buffer
     * @param size Must be > 0
     */
    void bf_enet_sendmsg_all(const char *buffer, size_t size)
    {
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(host, ENET_CHANNEL_RELIABLE, packet);
        enet_host_flush(host);
    }

    TbBool not_expected_user(NetUserId *user_id)
    {
        ERRORLOG("Unexpected connected user\n");
        fprintf(stderr, "Unexpected connected user\n");
        return false;
    }
    /**
     * Completely reads a message. Blocks until entire message has been read.
     * Will not block if msgready has returned > 0.
     * @param source The source user.
     * @param buffer
     * @param max_size The maximum size of the message to be received.
     * @return The actual size of the message received, <= max_size. If 0, an
     *  error occurred.
     */
    size_t bf_enet_readmsg(NetUserId source, char *buffer, size_t max_size)
    {
        size_t sz;
        while (!oldest_packet)
        {
            bf_enet_read_event(not_expected_user, 0);
        }
        ENetPacket *packet = oldest_packet;
        oldest_packet = static_cast<ENetPacket *>(oldest_packet->userData);
        incoming_queue_size--;

        sz = min(packet->dataLength, max_size);
        memcpy(buffer, packet->data, sz);
        enet_packet_destroy(packet);
        return sz;
    }

    /**
     * Asks if a message has finished reception and get be read through readmsg.
     * May block under some circumstances but shouldn't unless it can be presumed
     * a whole message is on the way.
     * TODO NET this definition is due to how SDL Net handles sockets.. see if it can
     *  be improved - ideally this function shouldn't block at all
     * @param source The source user.
     * @param timeout If non-zero, this method will wait this number of milliseconds
     *  for a message to arrive before returning.
     * @return The size of the message waiting if there is a message, otherwise 0.
     */
    size_t bf_enet_msgready(NetUserId source, unsigned timeout)
    {
        if (!oldest_packet)
        {
            bf_enet_read_event(not_expected_user, timeout);
        }
        return oldest_packet? oldest_packet->dataLength : 0;
    }

    /**
     * Disconnects a user.
     * @param id User to be dropped.
     */
    void bf_enet_drop_user(NetUserId id)
    {
        fprintf(stderr, "enet_drop_user not implemented\n");
        ERRORLOG("enet_drop_user not implemented");
    }

}

static bool IsPeerConnected(const ENetPeer *peer) {
    return peer && peer->state == ENET_PEER_STATE_CONNECTED;
}

static bool IsLocalPeer(NetUserId id) {
    return id == SERVER_ID || id == my_player_number;
}

static unsigned int ClampSizeToUInt(size_t value) {
    if (value > UINT_MAX) {
        return UINT_MAX;
    }
    return static_cast<unsigned int>(value);
}

unsigned long GetPing(NetUserId id) {
    const bool requesting_local_peer = IsLocalPeer(id);

    if (IsPeerConnected(client_peer)) {
        if (!requesting_local_peer) {
            return 0;
        }
        enet_uint32 value = client_peer->roundTripTime;
        if (value == 0) {
            value = client_peer->lastRoundTripTime;
        }
        return static_cast<unsigned long>(value);
    }

    if (!host) {
        return 0;
    }

    unsigned long best_value = 0;

    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }

        enet_uint32 peer_round_trip = peer->roundTripTime;
        if (peer_round_trip == 0) {
            peer_round_trip = peer->lastRoundTripTime;
        }
        unsigned long value = static_cast<unsigned long>(peer_round_trip);
        if (!requesting_local_peer) {
            NetUserId peer_id = NetUserId(reinterpret_cast<ptrdiff_t>(peer->data));
            if (peer_id == id) {
                return value;
            }
            continue;
        }
        if (value > best_value) {
            best_value = value;
        }
    }

    if (requesting_local_peer) {
        return best_value;
    }

    return 0;
}

unsigned long GetPingVariance(NetUserId id) {
    const bool requesting_local_peer = IsLocalPeer(id);

    if (IsPeerConnected(client_peer)) {
        if (!requesting_local_peer) {
            return 0;
        }
        enet_uint32 value = client_peer->roundTripTimeVariance;
        if (value == 0) {
            value = client_peer->lastRoundTripTimeVariance;
        }
        return static_cast<unsigned long>(value);
    }

    if (!host) {
        return 0;
    }

    unsigned long best_value = 0;

    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }

        enet_uint32 peer_round_trip_variance = peer->roundTripTimeVariance;
        if (peer_round_trip_variance == 0) {
            peer_round_trip_variance = peer->lastRoundTripTimeVariance;
        }
        unsigned long value = static_cast<unsigned long>(peer_round_trip_variance);
        if (!requesting_local_peer) {
            NetUserId peer_id = NetUserId(reinterpret_cast<ptrdiff_t>(peer->data));
            if (peer_id == id) {
                return value;
            }
            continue;
        }
        if (value > best_value) {
            best_value = value;
        }
    }

    if (requesting_local_peer) {
        return best_value;
    }

    return 0;
}

unsigned int GetPacketLoss(NetUserId id) {
    const bool requesting_local_peer = IsLocalPeer(id);

    if (IsPeerConnected(client_peer)) {
        if (!requesting_local_peer) {
            return 0;
        }
        enet_uint32 value = client_peer->packetLoss;
        unsigned int percent = static_cast<unsigned int>((static_cast<unsigned long long>(value) * 100ULL) / ENET_PEER_PACKET_LOSS_SCALE);
        return percent;
    }

    if (!host) {
        return 0;
    }

    unsigned int best_value = 0;

    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }

        enet_uint32 peer_packet_loss = peer->packetLoss;
        unsigned int value = static_cast<unsigned int>((static_cast<unsigned long long>(peer_packet_loss) * 100ULL) / ENET_PEER_PACKET_LOSS_SCALE);
        if (!requesting_local_peer) {
            NetUserId peer_id = NetUserId(reinterpret_cast<ptrdiff_t>(peer->data));
            if (peer_id == id) {
                return value;
            }
            continue;
        }
        if (value > best_value) {
            best_value = value;
        }
    }

    if (requesting_local_peer) {
        return best_value;
    }

    return 0;
}

unsigned int GetClientDataInTransit() {
    if (IsPeerConnected(client_peer)) {
        return client_peer->reliableDataInTransit;
    }

    if (!host) {
        return 0;
    }

    unsigned int result = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        if (peer->reliableDataInTransit > result) {
            result = peer->reliableDataInTransit;
        }
    }
    return result;
}

unsigned int GetIncomingPacketQueueSize() {
    if (incoming_queue_size <= 0) {
        return 0;
    }
    return static_cast<unsigned int>(incoming_queue_size);
}

unsigned int GetClientPacketsLost() {
    if (IsPeerConnected(client_peer)) {
        return static_cast<unsigned int>(client_peer->packetsLost);
    }
    if (!host) {
        return 0;
    }
    unsigned long long total = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        total += static_cast<unsigned long long>(peer->packetsLost);
        if (total > UINT_MAX) {
            return UINT_MAX;
        }
    }
    return static_cast<unsigned int>(total);
}

unsigned int GetClientOutgoingDataTotal() {
    if (IsPeerConnected(client_peer)) {
        return static_cast<unsigned int>(client_peer->outgoingDataTotal);
    }
    if (!host) {
        return 0;
    }
    unsigned long long total = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        total += static_cast<unsigned long long>(peer->outgoingDataTotal);
        if (total > UINT_MAX) {
            return UINT_MAX;
        }
    }
    return static_cast<unsigned int>(total);
}

unsigned int GetClientIncomingDataTotal() {
    if (IsPeerConnected(client_peer)) {
        return static_cast<unsigned int>(client_peer->incomingDataTotal);
    }
    if (!host) {
        return 0;
    }
    unsigned long long total = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        total += static_cast<unsigned long long>(peer->incomingDataTotal);
        if (total > UINT_MAX) {
            return UINT_MAX;
        }
    }
    return static_cast<unsigned int>(total);
}

unsigned int GetClientReliableCommandsInFlight() {
    if (IsPeerConnected(client_peer)) {
        size_t value = enet_list_size(&client_peer->sentReliableCommands);
        return ClampSizeToUInt(value);
    }
    if (!host) {
        return 0;
    }
    size_t best_value = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        size_t current = enet_list_size(&peer->sentReliableCommands);
        if (current > best_value) {
            best_value = current;
            if (best_value > UINT_MAX) {
                return UINT_MAX;
            }
        }
    }
    return ClampSizeToUInt(best_value);
}

struct NetSP *InitEnetSP()
{
    static struct NetSP ret =
    {
            .init = &bf_enet_init,
            .exit = &bf_enet_exit,
            .host = &bf_enet_host,
            .join = &bf_enet_join,
            .update = &bf_enet_update,
            .sendmsg_single = &bf_enet_sendmsg_single,
            .sendmsg_single_unsequenced = &bf_enet_sendmsg_single_unsequenced,
            .sendmsg_all = &bf_enet_sendmsg_all,
            .msgready = &bf_enet_msgready,
            .readmsg = &bf_enet_readmsg,
            .drop_user = &bf_enet_drop_user,
    };
    return &ret;
}
