/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_tcpsp.c
 *     Part of network support library.
 * @par Purpose:
 *     Low level TCP service provider routines. Implements abstract network API.
 * @par Comment:
 *     None.
 * @author   The KeeperFX Team
 * @date     16 Oct 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#define WIN32_LEAN_AND_MEAN 1
#include <assert.h>
#include <errno.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <SDL2/SDL.h>

#include "bflib_network.h"
#include "bflib_memory.h"


#define MAX_PACKET_SIZE   1024
#define MAX_PACKETS       32
#define MAX_UNBLOCKED_READ 1000
#define TOKEN_SIZE        2

typedef struct sockaddr_in IPaddress;

struct UdpPacket
{
    unsigned short len;
    unsigned short maxlen;
    struct sockaddr_in address;
    unsigned char data[];
};

struct PacketListNode
{
    struct PacketListNode *next;
    struct UdpPacket *packet;
};

enum SPMode
{
    SPM_INACTIVE = 0,
    SPM_WAITING_FOR_TOKEN,
    SPM_CONNECTED,
};

typedef short Token;

struct Peer
{
    NetUserId               id; //corresponding user
    Token                   token;
    Uint32                  last_packet_time_ms;
    struct PacketListNode   *head;
    struct PacketListNode   *tail;
    enum SPMode             mode;
    IPaddress               address;
};

struct SPState
{
    TbBool                  ishost;
    SOCKET                  sock;
    struct Peer             peers[MAX_N_PEERS];
    NetDropCallback         drop_callback;

    struct PacketListNode   *msg_head; // not parsed yet packets goes here
    struct PacketListNode   *msg_tail;

    struct PacketListNode   *free_node;

    IPaddress               address; // client only
    short                   client_token;
    enum SPMode             mode;

    struct UdpPacket        *outpacket;
};

struct PacketListNode all_nodes[MAX_PACKETS] = {0};

static struct SPState spstate;

static TbError  tcpSP_init(NetDropCallback drop_callback);
static void     tcpSP_exit(void);
static TbError  tcpSP_host(const char * session, void * options);
static TbError  tcpSP_join(const char * session, void * options);
static void     tcpSP_update(NetNewUserCallback new_user);
static void     tcpSP_sendmsg_single(NetUserId destination, const char * buffer, size_t size);
static void     tcpSP_sendmsg_all(const char * buffer, size_t size);
static size_t   tcpSP_msgready(NetUserId source, unsigned timeout);
static size_t   tcpSP_readmsg(NetUserId source, char * buffer, size_t max_size);
static void     tcpSP_drop_user(NetUserId id);

const struct NetSP tcpSP =
{
    tcpSP_init,
    tcpSP_exit,
    tcpSP_host,
    tcpSP_join,
    tcpSP_update,
    tcpSP_sendmsg_single,
    tcpSP_sendmsg_all,
    tcpSP_msgready,
    tcpSP_readmsg,
    tcpSP_drop_user,
};

static const SOCKET U_INVALID = 0;

#define U_Send(s, packet) U_Send_f(s, packet, __func__)
static int U_Send_f(SOCKET s, struct UdpPacket *packet, const char *from)
{
    int r;
    r = sendto(s, (char*) packet->data, packet->len, 0, 
        (struct sockaddr*)&packet->address, sizeof(packet->address));
    NETDBG(8, "%s: sending r:%d l:%d",
        from, r, packet->len);
    if (r != packet->len)
        return 0;
    return 1;
}

static TbBool U_Valid(SOCKET s)
{
    return s != 0;
}

static TbBool U_Init()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        ERRORLOG("wsastartup failed");
        return 0;
    }
    return 1;
}

static void U_Done()
{
    WSACleanup();
}

static void U_FreePacket(struct UdpPacket *packet)
{
    LbMemoryFree(packet);
}

static struct UdpPacket * U_AllocPacket(size_t max_size)
{
    struct UdpPacket *ret = (struct UdpPacket *)LbMemoryAlloc(
      sizeof(struct UdpPacket *) + max_size);
    ret->maxlen = max_size;
    return ret;
}

static SOCKET U_Open(unsigned short port)
{
    SOCKET ret;
    struct sockaddr_in server;
    unsigned long one = 1;
    int max_size = 65536;
    ret = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(ret, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        ERRORLOG("bind failed err:%lx", WSAGetLastError());
        closesocket(ret);
        return U_INVALID;
    }
    NETDBG(9, "bind ok");
    setsockopt(ret, SOL_SOCKET, SO_RCVBUF, (const char*)&max_size, sizeof(int));
    ioctlsocket(ret, FIONBIO, &one);
    return ret;
}

static void U_Close(SOCKET s)
{
    closesocket(s);
}

static TbBool U_ResolveHost(IPaddress *addr, const char *hostname, unsigned short port)
{
    struct addrinfo hints = {0};
    struct addrinfo *result = NULL;
    struct addrinfo *next;
    char port_buf[16];

    sprintf(port_buf, "%d", (int)port);
    if (0 != getaddrinfo(hostname, port_buf, &hints, &result))
    {
        ERRORLOG("getaddrinfo failed on %s %d", hostname, port);
        return 0;
    }
    for (next = result; next != NULL; next = next->ai_next)
    {
        if (next->ai_family == AF_INET)
        {
            memcpy(addr, result->ai_addr, sizeof(*addr));
            NETDBG(7, "getaddrinfo ok on %s %d -> %s",
                hostname, port, inet_ntoa(addr->sin_addr));
            freeaddrinfo(result);
            return 1;
        }
    }
    freeaddrinfo(result);
    ERRORLOG("getaddrinfo failed on %s %d (no IPv4?)", hostname, port);
    return 1;
}

static int U_Recv(SOCKET s, struct UdpPacket *packet)
{
      int size = sizeof(packet->address);
      assert(packet != NULL);

      int read = recvfrom(s, (char*)packet->data, packet->maxlen, 0,
          (struct sockaddr *)&packet->address, &size);
      if (read == 0)
      {
          ERRORLOG("== read 0? ==\n");
          return 0;
      }
      else if (read < 0)
      {
          DWORD err = WSAGetLastError();
          if (err == WSAEWOULDBLOCK)
          {
              return 0;
          }
          ERRORLOG("WSAGetLastError: %lx, packet->maxlen:%d size:%d",
              err, (int)packet->maxlen, size);
          return -1;
      }
      else
      {
          packet->len = read;
          assert(size <= sizeof(packet->address));
          return read;
      }
}

//////////////////

static TbBool clear_peer(NetUserId id)
{
    if (spstate.ishost) {
        for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
        {
            if (spstate.peers[i].id == id)
            {
                spstate.peers[i].tail->next = spstate.free_node->next; // Join lists
                spstate.free_node = spstate.peers[i].head;
                LbMemorySet(&spstate.peers[i], 0, sizeof(spstate.peers[i]));
                return 1;
            }
        }
        return 0;
    }
    else {
        if (U_Valid(spstate.sock)) {
            U_Close(spstate.sock);
            spstate.sock = U_INVALID; //assume disconnected
            spstate.client_token = 0;
        }
        return 1;
    }
}

static TbError tcpSP_init(NetDropCallback drop_callback)
{
    NETDBG(3, "Starting");

    LbMemorySet(&spstate, 0, sizeof(spstate));

    if (U_Init() == 0)
    {
        return Lb_FAIL;
    }

    for (unsigned int i = 0; i < MAX_PACKETS; ++i)
    {
        all_nodes[i].packet = U_AllocPacket(MAX_PACKET_SIZE);
        all_nodes[i].next = &all_nodes[i+1];
    }
    spstate.free_node = &all_nodes[0];
    all_nodes[MAX_PACKETS - 1].next = NULL;

    spstate.outpacket = U_AllocPacket(MAX_PACKET_SIZE);

    spstate.drop_callback = drop_callback;

    return Lb_OK;
}

static void tcpSP_exit(void)
{
    NETDBG(9, "Starting");
    if (spstate.ishost)
    {
        for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
        {
            spstate.peers[i].head = NULL;
            spstate.peers[i].tail = NULL;
        }
    }
    for (unsigned int i = 0; i < MAX_PACKETS; ++i)
    {
        if (all_nodes[i].packet)
        {
            U_FreePacket(all_nodes[i].packet);
            all_nodes[i].packet = NULL;
        }
    }
    if (spstate.outpacket)
    {
        U_FreePacket(spstate.outpacket);
        spstate.outpacket = NULL;
    }
    if (spstate.sock)
    {
        U_Close(spstate.sock);
        spstate.sock = U_INVALID;
    }

    U_Done();

    LbMemorySet(&spstate, 0, sizeof(spstate));
    NETDBG(9, "Done");
}

static TbError tcpSP_host(const char * session, void * options)
{
    assert(session);
    NETDBG(4, "Creating TCP server SP for session %s", session);

    const char* portstr = session;

    while (*portstr != 0 && *portstr != ':') {
        ++portstr;
    }
    if (*portstr == ':') {
        ++portstr;
    }

    spstate.sock = U_Open(atoi(portstr));
    if (!U_Valid(spstate.sock)) {
        ERRORLOG("Failed to initialize server socket to port %s", portstr);
        return Lb_FAIL;
    }

    spstate.ishost = 1;

    return Lb_OK;
}

static TbBool null_fn(NetUserId *id)
{
    return 0;
}

static TbError tcpSP_join(const char * session, void * options)
{
    assert(session);
    NETDBG(4, "Creating TCP client SP for session %s", session);

    size_t size = LbStringLength(session) + 1;
    char* hostname = (char*)LbMemoryAlloc(size);
    LbStringCopy(hostname, session, size);

    char* portstr = hostname;
    while (*portstr != 0 && *portstr != ':') {
        ++portstr;
    }
    if (*portstr == ':') {
        *portstr = 0;
        ++portstr;
    }

    if (U_ResolveHost(&spstate.address, hostname, atoi(portstr)) == 0)
    {
        NETMSG("Failed to resolve %s: %d", hostname, atoi(portstr));
        LbMemoryFree(hostname);
        return Lb_FAIL;
    }

    spstate.sock = U_Open(0);
    if (spstate.sock == U_INVALID) {
        LbMemoryFree(hostname);
        NETMSG("Failed to initialize UDP client socket");
        return Lb_FAIL;
    }
    LbMemoryFree(hostname);

    spstate.ishost = 0;
    spstate.mode = SPM_WAITING_FOR_TOKEN;

    spstate.peers[0].id = SERVER_ID; // server
    spstate.peers[0].token = (short) -1; // server
    spstate.peers[0].mode = SPM_CONNECTED;
    spstate.peers[0].last_packet_time_ms = SDL_GetTicks() + 5000; // We have to connect after 5 sec

    short *short_ptr = (short*)spstate.outpacket->data;
    short_ptr[0] = 0;
    spstate.outpacket->address = spstate.address;
    spstate.outpacket->len = TOKEN_SIZE;
    U_Send(spstate.sock, spstate.outpacket);
    for (int i = 0; i < 20; i++)
    {
        SDL_Delay(25);
        tcpSP_update(&null_fn); // A bit of hope
        if (spstate.mode != SPM_WAITING_FOR_TOKEN)
        {
            NETDBG(4, "got token 0x%x", spstate.client_token);
            return Lb_OK;
        }
    }
    NETDBG(4, "no token");
    return Lb_FAIL;
}

static short get_token(Uint32 now)
{
    short token = (short)(9377 * (now) + 9439); // pseudorandom token
    int index;
    if (((short)now) == 0)
        now = 103; // any number;

    while (1)
    {
        for (index = 0; index < MAX_N_PEERS; ++index)
        {
            if ((spstate.peers[index].mode != SPM_INACTIVE)
                && (spstate.peers[index].token == token))
            {
                token += (short)now;
                break;
            }
        }
        if (index != MAX_N_PEERS)
            continue;
        if (token == 0)
        {
            token += (short)now;
            continue;
        }
        if (token == (short)-1)
        {
            token += (short)now;
            continue;
        }
        break;
    }
    return token;
}

static void process_packet(Uint32 now, struct PacketListNode *node, NetNewUserCallback new_user)
{
    NetUserId id;
    int index;
    short *short_ptr;
    short_ptr = (short *)node->packet->data;
    if (short_ptr[0] == 0)
    {
        assert(spstate.ishost);

        // This is a new client
        for (index = 0; index < MAX_N_PEERS; ++index)
        {
            if (spstate.peers[index].mode == SPM_INACTIVE)
                break;
        }

        if (index < MAX_N_PEERS && new_user(&id))
        {
            spstate.peers[index].last_packet_time_ms = now;
            spstate.peers[index].id = id;
            spstate.peers[index].token = get_token(now);
            spstate.peers[index].address = node->packet->address;
            spstate.peers[index].mode = SPM_WAITING_FOR_TOKEN;
            NETDBG(8, "Assigning new id:%d for index:%d, token:%x", id, index, spstate.peers[index].token);

            short_ptr = (short*)spstate.outpacket->data;
            short_ptr[0] = (short) -1;
            short_ptr[1] = spstate.peers[index].token;
            spstate.outpacket->len = 2 * TOKEN_SIZE;
            spstate.outpacket->address = node->packet->address;
            U_Send(spstate.sock, spstate.outpacket);
        }
        else {
            NETMSG("Socket dropped because server is full");
        }
    }
    else
    {
        if (spstate.mode == SPM_WAITING_FOR_TOKEN)
        {
            NETDBG(8, "Storing a token:0x%0x", short_ptr[1]);
            spstate.client_token = short_ptr[1];
            node->next = spstate.free_node;
            spstate.free_node = node;
            spstate.mode = SPM_CONNECTED;
            return;
        }
        for (int i = 0; i < MAX_N_PEERS; ++i)
        {
            if (spstate.peers[i].token == short_ptr[0])
            {
                NETDBG(9, "Packet for token:0x%0x i:%d id:%d len=%d",
                    short_ptr[0], i, spstate.peers[i].id, node->packet->len);

                if (spstate.peers[i].mode == SPM_WAITING_FOR_TOKEN)
                {   // we already assigned a token, but we should not spam to client 
                    // BEFORE it send a message with payload
                    NETDBG(9, "Client is now connected");
                    spstate.peers[i].mode = SPM_CONNECTED;
                }
                spstate.peers[i].address = node->packet->address;
                spstate.peers[i].last_packet_time_ms = now;
                // Add to list of packets
                if (spstate.peers[i].tail == NULL)
                {
                    spstate.peers[i].head = node;
                    spstate.peers[i].tail = node;
                    node->next = NULL;
                }
                else
                {
                    spstate.peers[i].tail->next = node;
                    spstate.peers[i].tail = node;
                }
            }
        }
    }
}

// Here we drain and dispatch packets from network
static void tcpSP_update(NetNewUserCallback new_user)
{
    assert(new_user);
    Uint32 now;
    now = SDL_GetTicks();

    struct PacketListNode *node, *next;
    // Lets first drain all stored packets
    next = spstate.msg_head;
    if (next != NULL)
    {   // Lets check all buffered messages first
        NETDBG(9, "Draining stored packets");

        while (next != NULL)
        {
            node = next;
            next = next->next;
            node->next = NULL;
            process_packet(now, node, new_user);
        }
        // free all buffered packets
        spstate.msg_head = NULL;
        spstate.msg_tail = NULL;
    }
    NETDBG(10, "Waiting for more packets");
    if (spstate.free_node == NULL)
    {
        ERRORLOG("Too many packets are in queue");
        return;
    }
    node = spstate.free_node;
    spstate.free_node = node->next;
    for (
        int ret = U_Recv(spstate.sock, node->packet);
        ret != 0;
        ret = U_Recv(spstate.sock, node->packet))
    {
        NETDBG(8, "Got a packet len=%d", node->packet->len);
        if (ret < 0)
        {
            NETMSG("Recv error");
            break;
        }
        node->next = NULL;
        process_packet(now, node, new_user);

        if (spstate.free_node == NULL)
        {
            ERRORLOG("Too many packets are waiting");
            return;
        }
        node = spstate.free_node;
        spstate.free_node = node->next;
    }
    node->next = spstate.free_node; // return node to the list
    spstate.free_node = node;
    NETDBG(10, "done");
}

/*
    timeout 0 means wait infinite
    returns NULL or "floating node" - not contained in any list
*/
static struct PacketListNode *wait_for_message(Token token, unsigned timeout)
{
    NETDBG(9, "Starting for 0x%0x timeout=%u", token, timeout);
    Token *short_ptr;
    struct PacketListNode *node, *prev;
    Uint32 late = SDL_GetTicks() + timeout;

    node = spstate.msg_head;
    if (node != NULL)
    {   // Lets check all buffered messages first
        short_ptr = (Token*)node->packet->data;
        if (short_ptr[0] == token)
        {
            spstate.msg_head = node->next;
            node->next = NULL;
            if (spstate.msg_tail == node)
            {
                spstate.msg_tail = NULL;
            }
            NETDBG(9, "first from buffer");
            return node;
        }
        prev = node;
        node = node->next;
        while (node != NULL)
        {
            short_ptr = (Token*)node->packet->data;
            if (short_ptr[0] == token)
            {
                if (spstate.msg_tail == node)
                {
                    spstate.msg_tail = prev;
                }
                prev->next = node->next;
                node->next = NULL;
                NETDBG(9, "some from buffer");
                return node;
            }
            prev = node;
            node = node->next;
        }
    }

    if (spstate.free_node == NULL)
    {
        ERRORLOG("Too many packets are waiting");
        return NULL;
    }
    node = spstate.free_node;
    spstate.free_node = node->next;
    while  (true)
    {
        int ret = U_Recv(spstate.sock, node->packet);
        if (ret == 0)
        {
            if ((timeout > 0)  && (SDL_GetTicks() > late))
            {
                break;
            }
            SDL_Delay(0);
            continue;
        }
        else if (ret < 0)
        {
            NETMSG("Recv error");
            break;
        }
        node->next = NULL;
        short_ptr = (Token*)node->packet->data;
        NETDBG(8, "Got a packet len=%d token=0x%0x", node->packet->len, short_ptr[0]);
        if (short_ptr[0] == token)
        {
            NETDBG(9, "processing this packet");
            return node;
        }
        else
        {
            NETDBG(9, "storing this packet");
            if (spstate.msg_head == NULL)
                spstate.msg_head = node;
            if (spstate.msg_tail)
                spstate.msg_tail->next = node;
            spstate.msg_tail = node;
        }

        node = spstate.free_node;
        spstate.free_node = node->next;
        if (spstate.free_node == NULL)
        {
            ERRORLOG("Too many packets are waiting");
            break;
        }
    }
    node->next = spstate.free_node; // return node to the list
    spstate.free_node = node;
    NETDBG(8, "Timeout?");
    return NULL;
}

static void tcpSP_sendmsg_single(NetUserId destination, const char * buffer, size_t size)
{
    assert(buffer);
    assert(size > 0);
    assert(size + TOKEN_SIZE < MAX_PACKET_SIZE);
    assert(!(spstate.ishost && destination == SERVER_ID));

    NETDBG(9, "buffer of %u bytes to user %u", size, destination);

    memcpy(spstate.outpacket->data + TOKEN_SIZE, buffer, size);
    spstate.outpacket->len = size + TOKEN_SIZE;

    if (spstate.ishost)
    {
        ((short *)spstate.outpacket->data)[0] = (short)-1;
        int i;
        for (i = 0; i < MAX_N_PEERS; ++i)
        {
            if ((spstate.peers[i].id == destination)
                && (spstate.peers[i].mode != SPM_INACTIVE)
                )
            {
                if (spstate.peers[i].mode == SPM_WAITING_FOR_TOKEN)
                {
                    WARNLOG("Semiconnected response for id:%d token:0x%0x",
                        spstate.peers[i].id, spstate.peers[i].token);
                }
                NETDBG(9, "Token:0x%0x i:%d id:%d",
                    spstate.peers[i].token, i, spstate.peers[i].id);
                spstate.outpacket->address = spstate.peers[i].address;
                break;
            }
        }
        if (i == MAX_N_PEERS)
        {
            ERRORLOG("Unable to find dest:%d", destination);
            return;
        }
    }
    else
    {
        ((short *)spstate.outpacket->data)[0] = spstate.client_token;
        spstate.outpacket->address = spstate.address;
    }
    if (0 == U_Send(spstate.sock, spstate.outpacket))
    {
        ERRORLOG("Unable to send packet");
    }
}

static void tcpSP_sendmsg_all(const char * buffer, size_t size)
{
    assert(buffer);
    assert(size > 0);
    assert(size + TOKEN_SIZE < MAX_PACKET_SIZE);
    NETDBG(10, "Starting for buffer of %u bytes", size);

    memcpy(spstate.outpacket->data + TOKEN_SIZE, buffer, size);
    spstate.outpacket->len = size + TOKEN_SIZE;

    if (spstate.ishost)
    {
        ((short *)spstate.outpacket->data)[0] = (short)-1;
        for (int i = 0; i < MAX_N_PEERS; ++i)
        {
            if ((spstate.peers[i].mode == SPM_INACTIVE)
                || (spstate.peers[i].mode == SPM_WAITING_FOR_TOKEN))
            {
                continue;
            }
            NETDBG(9, "Token:0x%0x i:%d id:%d", spstate.peers[i].token, i, spstate.peers[i].id);

            spstate.outpacket->address = spstate.peers[i].address;
            if (0 == U_Send(spstate.sock, spstate.outpacket))
            {
                ERRORLOG("Unable to send packet");
            }
        }
    }
    else
    {
        ((short *)spstate.outpacket->data)[0] = spstate.client_token;
        spstate.outpacket->address = spstate.address;
        if (0 == U_Send(spstate.sock, spstate.outpacket))
        {
            ERRORLOG("Unable to send packet");
        }
    }
}
/*
    timeout 0 means nonblocking
*/
static size_t tcpSP_msgready(NetUserId source, unsigned timeout)
{
    NETDBG(10, "Starting message ready check for user %u", source);

    for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
    {
        if ((spstate.peers[i].id == source) && (spstate.peers[i].mode != SPM_INACTIVE))
        {
            struct PacketListNode *node = spstate.peers[i].head;
            if (node != NULL)
            {
                return node->packet->len - TOKEN_SIZE;
            }
            else if (timeout == 0)
            {
                NETDBG(9, "No data from i:%d id:%u", i, source);
                return 0;
            }
            else
            {   // No data but we have to wait
                node = wait_for_message(spstate.peers[i].token, timeout);
                if (node == NULL)
                {
                    NETDBG(9, "No data from id:%u", source);
                    return 0;
                }
                if (spstate.peers[i].mode == SPM_WAITING_FOR_TOKEN)
                {   // we already assigned a token, but we should not spam to client 
                    // BEFORE it send a message with payload
                    spstate.peers[i].mode = SPM_CONNECTED;
                    NETDBG(9, "Client id:%u is now connected token:%0x", source, spstate.peers[i].token);
                }
                else
                {
                    NETDBG(9, "Client id:%u token:%0x is in mode %d", source, spstate.peers[i].token, spstate.peers[i].mode);
                }
                // We already knew that there is no nodes in list
                spstate.peers[i].head = node;
                spstate.peers[i].tail = node;
                return node->packet->len - TOKEN_SIZE;
            }
        }
    }
    NETDBG(8, "Source %u not found", source);
    return 0;
}

static size_t tcpSP_readmsg(NetUserId source, char * buffer, size_t max_size)
{
    assert(buffer);
    assert(max_size > 0);
    NETDBG(10, "Starting read from user %u", source);

    for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
    {
        if ((spstate.peers[i].id == source) && (spstate.peers[i].mode != SPM_INACTIVE))
        {
            struct PacketListNode *node = spstate.peers[i].head;
            if (node == NULL)
            {
                NETDBG(9, "No data for %u, waiting", source);
                node = wait_for_message(spstate.peers[i].token, MAX_UNBLOCKED_READ);

                if (node == 0)
                {
                    JUSTLOG("net read blocked for %d ms", MAX_UNBLOCKED_READ);
                    return 0;
                }
            }
            memcpy(buffer, node->packet->data + TOKEN_SIZE, min(max_size, node->packet->len - TOKEN_SIZE));
            if (node == spstate.peers[i].tail)
                spstate.peers[i].tail = node->next;
            spstate.peers[i].head = node->next;
            node->next = spstate.free_node;
            spstate.free_node = node;
            return node->packet->len - TOKEN_SIZE;
        }
    }
    NETDBG(8, "Source %u not found", source);
    return 0;
}

static void tcpSP_drop_user(NetUserId id)
{
    if (clear_peer(id) && spstate.drop_callback) {
        spstate.drop_callback(id, NETDROP_MANUAL);
    }
}
