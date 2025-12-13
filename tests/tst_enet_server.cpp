//
// Created by Sim on 23/10/22.
//
#include <stdio.h>
#include <enet/enet.h>

#include "bflib_enet.h"
#include "front_landview.h"

enum NetMessageType
{
    NETMSG_LOGIN,           //to server: username and pass, from server: assigned id
    NETMSG_USERUPDATE,      //changed player from server
    NETMSG_FRAME,           //to server: ACK of frame + packets, from server: the frame itself
    NETMSG_LAGWARNING,      //from server: notice that some client is lagging¨
    NETMSG_RESYNC,          //from server: re-synchronization is occurring
};

namespace
{
    void send_message(ENetPeer *client_peer, char prefix, const char *msgpack, ...)
    {
        const char *P = msgpack;
        const char *E = P + strlen(P);
        va_list lst;

        char buffer[256];
        size_t sz = 1;
        char *dst = buffer;
        *dst = prefix;
        dst++;

        va_start(lst, msgpack);
        for(int i = 0; P < E; P++, i++)
        {
            switch (*P)
            {
                case 'c':
                    sz += 1;
                    *((char*)dst) = va_arg(lst, int);
                    dst += 1;
                    break;
                case 's':
                    sz += 2;
                    *((short*)dst) = va_arg(lst, int);
                    dst += 2;
                    break;
                default:
                    fprintf(stderr, "Unknown symbol: %c", *P);
            }
        }
        va_end(lst);
        ENetPacket *packet = enet_packet_create(buffer, sz, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(client_peer, ENET_CHANNEL_RELIABLE, packet);
        fprintf(stderr, "sending %d bytes to %p\n", sz, client_peer);
    }

#pragma pack(1)
    int seq_nbr = 1;
    void process_packet(ENetPeer *peer, ENetPacket *packet)
    {
        struct ScreenPacket *screen_packet;
        struct
        {
            char header;
            int seq;
            char logged_clients;
            struct ScreenPacket packet;
        } out = {
                NETMSG_FRAME,
                seq_nbr,
                2,
                {

                }
        };
        switch (packet->data[0])
        {
            case NETMSG_LOGIN:
                send_message(peer, NETMSG_LOGIN, "c", 1);
                break;
            case NETMSG_FRAME:
            {
                seq_nbr++;
                screen_packet = reinterpret_cast<struct ScreenPacket *>(packet->data);
                out.packet.reserved_padding[0] = 0x15;
                ENetPacket *out_packet = enet_packet_create(&out, sizeof(out), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, ENET_CHANNEL_RELIABLE, out_packet);
                break;
            }
            default:
                printf("= packet = %d, %x\n", packet->dataLength, packet->data[0]);
                fprintf(stderr, "Unexpected packet type: %d", packet->data[0]);
        }
    }
}

int run_serv(int argc, char **argv)
{
    ENetEvent event;
    ENetHost *client;
    const char *SRC_ADDR = "192.168.0.63";
    ENetAddress addr = {ENET_HOST_ANY, 5556};
    int ret;
    enet_initialize();
    if (enet_address_set_host(&addr, SRC_ADDR) < 0)
    {
        fprintf(stderr, "Unable to listen");
        return 1;
    }
    client = enet_host_create(&addr /* create a client host */,
                              4 /* only allow 1 outgoing connection */,
                              2 /* allow up 2 channels to be used, 0 and 1 */,
                              0 /* assume any amount of incoming bandwidth */,
                              0 /* assume any amount of outgoing bandwidth */);
    if (client == NULL)
    {
        fprintf(stderr,
                "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }
    while ((ret = enet_host_service(client, &event, 1000)) >= 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n",
                       event.peer->address.host,
                       event.peer->address.port);
                /* Store any relevant client information here. */
                event.peer->data = (void*)"Client information";
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                process_packet(event.peer, event.packet);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconnected.\n", (const char *) event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
        }
    }
    if (ret < 0)
    {
        fprintf(stderr, "err: %d", ret);
    }
    enet_host_destroy(client);
    return 0;
}
