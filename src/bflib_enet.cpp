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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <enet/enet.h>
#include <cstddef>

#include "post_inc.h"

#define NUM_CHANNELS 2
#define CONNECT_TIMEOUT 3000
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
        ENetAddress addr = {.host = 0,
                            .port = DEFAULT_PORT };
        if (!*session)
            return Lb_FAIL;
        host = enet_host_create(&addr, 4, NUM_CHANNELS, 0, 0);
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
            NETMSG("Unable to join: failed to create host");
            return Lb_FAIL;
        }
        P = strchr(session,':');
        if (P)
        {
            strncpy(buf, session, P-session);
            address.port = strtoul(P+1, &E, 10);
            if (address.port == 0)
            {
                host_destroy();
                return Lb_FAIL;
                NETMSG("Unable to join: did not find a port");
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
            NETMSG("Unable to join: found no address %s", address.host);
            return Lb_FAIL;
        }
        client_peer = enet_host_connect(host, &address, NUM_CHANNELS, 0);
        if (!client_peer)
        {
            NETMSG("Unable to join: unable to connect to host");
            return Lb_FAIL;
        }
        if (wait_for_connect(CONNECT_TIMEOUT))
        {
            host_destroy();
            NETMSG("Unable to join: connection timed out");
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
        while (bf_enet_read_event(new_user, 0))
        {
            // Loop
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
        // TODO: allocations could become a performance problem
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_RELIABLE);
        if (client_peer) // Just send to server
        {
            enet_peer_send(client_peer, 0, packet);
        }
        else
        {
            for (ENetPeer *currentPeer = host->peers; currentPeer < &host->peers[host -> peerCount]; ++currentPeer)
            {
                if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
                    continue;
                if (NetUserId(reinterpret_cast<ptrdiff_t>(currentPeer->data)) == destination)
                {
                    enet_peer_send(currentPeer, 0, packet);
                }
            }
        }
    }

    /**
     * Sends a message buffer to all remote users.
     * @param buffer
     * @param size Must be > 0
     */
    void bf_enet_sendmsg_all(const char *buffer, size_t size)
    {
        // TODO: allocations could become a performance problem
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(host, 0, packet);
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
        if ((!oldest_packet) && timeout > 0)
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
            .sendmsg_all = &bf_enet_sendmsg_all,
            .msgready = &bf_enet_msgready,
            .readmsg = &bf_enet_readmsg,
            .drop_user = &bf_enet_drop_user,
    };
    return &ret;
}
