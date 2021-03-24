#include "globals.h"

#include "bflib_basics.h"
#include "event_monitoring.h"

#include <SDL2/SDL_net.h>

static const int MAX_PACKET_SIZE = 1024;
#define SUFFIX_SIZE 4

static char evm_suffix[SUFFIX_SIZE+1] = {0};

static UDPsocket evm_socket = 0;
static UDPpacket * evm_packet = NULL;

void evm_init(char *hostport, int client_no)
{
    int port = 8089;
    char *portptr = strchr(hostport, ':');

    if (SDLNet_Init() < 0)
    {
        WARNLOG("Unable to init SDLNet: %s", SDLNet_GetError());
        return;
    }

    if (client_no < 0 || client_no > 9)
    {
        WARNLOG("Invalid suffix: %d", client_no);
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

    sprintf(evm_suffix, ",s=%d", client_no);

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
    int len, ret;
    if (evm_packet == NULL)
    {
        return;
    }

    char packet_data[MAX_PACKET_SIZE];
    packet_data[MAX_PACKET_SIZE-1] = 0;

    if (force_new && (evm_packet->len > 0))
    {
        // Send old data
        SDLNet_UDP_Send(evm_socket, -1, evm_packet);
        evm_packet->len = 0;
    }

    va_list lst;
    va_start(lst, event_fmt);
    ret = vsnprintf(packet_data, MAX_PACKET_SIZE - 1, event_fmt, lst);
    va_end(lst);

    len = strlen(packet_data);
    if ((ret < 0) || (len >= MAX_PACKET_SIZE))
    {
        ERRORMSG("Packet too big");
        return;
    }
    else if (evm_packet->len + len + 1 >= MAX_PACKET_SIZE)
    {
        // Not enough remaining space. Send old data and try again
        SDLNet_UDP_Send(evm_socket, -1, evm_packet);
        evm_packet->len = 0;
    }
    if (evm_packet->len > 0)
    {
        evm_packet->data[evm_packet->len] = '\n';
        memcpy(evm_packet->data + 1 + evm_packet->len, packet_data, len);
        evm_packet->len += len + 1;
    }
    else
    {
        memcpy(evm_packet->data + evm_packet->len, packet_data, len);
        evm_packet->len += len;
    }

    if (16 + evm_packet->len > MAX_PACKET_SIZE)
    {
        SDLNet_UDP_Send(evm_socket, -1, evm_packet);
        evm_packet->len = 0;
    }
}
