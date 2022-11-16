//
// Created by Sim on 23/10/22.
//
#include <stdio.h>
#include <enet/enet.h>

extern "C" {
}

int run_client(int argc, char **argv)
{
    ENetEvent event;
    ENetHost *client;
    const char *SRC_ADDR = "192.168.0.63";
    const char *DST_ADDR = "192.168.0.63";
    ENetAddress address = {ENET_HOST_ANY, ENET_PORT_ANY};
    int ret;
    enet_initialize();
    if (enet_address_set_host(&address, "127.0.0.1") < 0)
    {
        fprintf(stderr, "Unable to listen");
        return 1;
    }
    client = enet_host_create(&address,
                              4,
                              2 /* allow up 2 channels to be used, 0 and 1 */,
                              0,
                              0);
    if (client == NULL)
    {
        fprintf(stderr,
                "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    if (enet_address_set_host(&address, DST_ADDR) < 0)
    {
        enet_host_destroy(client);
        return 1;
    }
    address.port = 5556;

    ENetPeer *peer = enet_host_connect(client, &address, 2, 0);
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
                printf("A packet of length %u containing %s was received from %s on channel %u.\n",
                       event.packet->dataLength,
                       event.packet->data,
                       (const char *) event.peer->data,
                       event.channelID);
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
