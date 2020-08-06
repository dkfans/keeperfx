#include "globals.h"

#include "bflib_basics.h"
#include "event_monitoring.h"

#include <SDL/SDL_net.h>

const int MAX_PACKET_SIZE = 1024;

static UDPsocket evm_socket = 0;
static UDPpacket * evm_packet = NULL;

void evm_init(char *hostport)
{
    int port = 8089;
    char *portptr = strchr(hostport, ':');

    if (SDLNet_Init() < 0)
    {
        WARNLOG("Unable to init SDL: %s", SDLNet_GetError());
        return;
    }

    evm_socket = SDLNet_UDP_Open(0);
    if (evm_socket == NULL)
    {
        WARNMSG("Failed to open monitoring socket: %s", SDLNet_GetError());
        return;
    }
    evm_packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (evm_packet == NULL)
    {
        SDLNet_UDP_Close(evm_socket);
        evm_socket = 0;
        return;
    }

    if (portptr != NULL)
    {
        port = atoi(portptr + 1);
        *portptr = 0;
    }

    if (SDLNet_ResolveHost(&evm_packet->address, hostport, port) != 0)
    {
        evm_done();
        return;
    }
    evm_packet->channel = -1;
    evm_packet->len = 0;
}

void evm_done()
{
    if (evm_packet)
    {
        SDLNet_FreePacket(evm_packet);
        evm_packet = NULL;
    }
    if (evm_socket)
    {
        SDLNet_UDP_Close(evm_socket);
        evm_socket = 0;
    }
}

void evm_stat(int force_new, const char *event_fmt, ...)
{
    int len, remain;
    if (evm_packet == NULL)
    {
        return;
    }

    char *packet_data = ((char*)evm_packet->data) + evm_packet->len;

    if (force_new && (evm_packet->len > 0))
    {
        // Send old data
        SDLNet_UDP_Send(evm_socket, -1, evm_packet);
        evm_packet->len = 0;
        packet_data = (char*)evm_packet->data;
    }

    va_list lst, lst2;
    va_start(lst, event_fmt);
    va_copy(lst2, lst);

    remain = MAX_PACKET_SIZE - evm_packet->len - 1;
    len = vsnprintf(packet_data, remain, event_fmt, lst);
    if (len >= MAX_PACKET_SIZE)
    {
        ERRORMSG("Packet too big");
        return;
    }
    else if (len >= remain)
    {
        // Not enough remaining space. Send old data and try again
        SDLNet_UDP_Send(evm_socket, -1, evm_packet);
        remain = MAX_PACKET_SIZE - 1;
        evm_packet->len = 0;
        packet_data = (char*)evm_packet->data;

        len = vsnprintf(packet_data, remain, event_fmt, lst2);
    }
    va_end(lst);
    va_end(lst2);

    packet_data[len] = '\n';
    evm_packet->len += len + 1;

    if (MAX_PACKET_SIZE - evm_packet->len < 16)
    {
        SDLNet_UDP_Send(evm_socket, -1, evm_packet);
        evm_packet->len = 0;
    }
}
