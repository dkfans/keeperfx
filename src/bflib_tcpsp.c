/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_tcpsp.c
 *     Part of network support library.
 * @par Purpose:
 *     Low level TCP service provider routines.
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

#include "bflib_network.h"

#include "bflib_memory.h"

#include <assert.h>
#include <SDL2/SDL_net.h>

enum MsgReadState
{
    READ_HEADER,
    READ_BODY,
    READ_FINISHED
};

struct Msg
{
    enum MsgReadState state;
    size_t msg_size;
    //size_t bytes_left; //not necessary with SDL_net
    char * buffer;
    size_t buffer_size;
};

struct Peer
{
    TCPsocket   socket;
    NetUserId   id; //corresponding user
    struct Msg  msg; //last msg received
};

struct SPState
{
    TbBool              ishost;
    struct Msg          servermsg; //message from server
    TCPsocket           socket; //server socket or client peer
    SDLNet_SocketSet    socketset;
    struct Peer         peers[MAX_N_PEERS];
    NetDropCallback     drop_callback;
};

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

static TbBool clear_peer(NetUserId id)
{
    if (spstate.ishost) {
        for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
        {
            if (spstate.peers[i].id == id) {
                if (spstate.peers[i].socket != NULL) {
                    SDLNet_TCP_DelSocket(spstate.socketset, spstate.peers[i].socket);
                    SDLNet_TCP_Close(spstate.peers[i].socket);
                }

                LbMemoryFree(spstate.peers[i].msg.buffer);
                LbMemorySet(&spstate.peers[i], 0, sizeof(spstate.peers[i]));

                return 1;
            }
        }

        return 0;
    }
    else {
        if (spstate.socket != NULL) {
            SDLNet_TCP_Close(spstate.socket);
            spstate.socket = NULL; //assume disconnected
        }

        LbMemoryFree(spstate.servermsg.buffer);
        LbMemorySet(&spstate.servermsg, 0, sizeof(spstate.servermsg));

        return 1;
    }
}

static TCPsocket find_peer_socket(NetUserId id)
{
    if (spstate.ishost) {
        for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
        {
            if (spstate.peers[i].id == id) {
                return spstate.peers[i].socket;
            }
        }
    }
    else {
        assert(id == SERVER_ID);

        return spstate.socket;
    }

    ERRORLOG("No user with ID %u", id);
    return NULL;
}

static struct Msg * find_peer_message(NetUserId id)
{
    if (spstate.ishost) {
        for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
        {
            if (spstate.peers[i].id == id) {
                return &spstate.peers[i].msg;
            }
        }
    }
    else {
        assert(id == SERVER_ID);

        return &spstate.servermsg;
    }

    ERRORLOG("No user with ID %u", id);
    return NULL;
}

static TbError send_buffer(TCPsocket socket, const char * buffer, size_t size)
{
    int retval;

    assert(buffer);

    if (socket == NULL) {
        return Lb_OK;
    }

    NETDBG(9, "Trying to send %u bytes", size);

    if ((retval = SDLNet_TCP_Send(socket, buffer, size)) != size) {
        NETMSG("Failure to send to socket: %d", retval);
        return Lb_FAIL;
    }

    return Lb_OK;
}

static void reset_msg(struct Msg * msg)
{
    NETDBG(9, "Starting");
    msg->state = READ_HEADER;
    msg->msg_size = 0;
}

static void inflate_msg(struct Msg * msg)
{
    NETDBG(9, "Inflating to %u bytes", msg->msg_size);

    msg->state = READ_BODY;

    if (msg->msg_size > msg->buffer_size) {
        msg->buffer_size = msg->msg_size;
        msg->buffer = (char*) LbMemoryGrow(msg->buffer, msg->buffer_size);
    }
}

static TbError read_stage(TCPsocket socket, char * buffer, size_t size)
{
    int retval;

    assert(buffer);
    assert(socket);

    NETDBG(9, "Trying to read %u bytes", size);

    if ((retval = SDLNet_TCP_Recv(socket, buffer, size)) != size) {
        NETMSG("Failure to read from socket: %d", retval);
        return Lb_FAIL;
    }

    return Lb_OK;
}

static TbError read_full(TCPsocket socket, struct Msg * msg)
{
    NETDBG(8, "Starting");

    assert(msg);

    if (socket == NULL) {
        return Lb_OK;
    }

    if (msg->state == READ_FINISHED) {
        reset_msg(msg);
    }

    if (msg->state == READ_HEADER) {
        if (read_stage(socket, (char*) &msg->msg_size, 4) == Lb_FAIL) {
            return Lb_FAIL;
        }

        inflate_msg(msg);
    }

    if (msg->state == READ_BODY) {
        if (read_stage(socket, msg->buffer, msg->msg_size) == Lb_FAIL) {
            return Lb_FAIL;
        }
    }

    assert(msg->state == READ_BODY);
    msg->state = READ_FINISHED;

    NETDBG(8, "Read of %u bytes finished", msg->msg_size);

    return Lb_OK;
}

static TbError read_partial(TCPsocket socket, struct Msg * msg, unsigned timeout)
{
    NETDBG(8, "Starting");

    assert(msg);

    if (socket == NULL) {
        return Lb_OK;
    }

    if (msg->state == READ_FINISHED) {
        reset_msg(msg);
    }

    if (msg->state == READ_HEADER) {
        SDLNet_CheckSockets(spstate.socketset, timeout);
        if (!SDLNet_SocketReady(socket)) {
            return Lb_OK;
        }

        if (read_stage(socket, (char*) &msg->msg_size, 4) == Lb_FAIL) {
            return Lb_FAIL;
        }

        inflate_msg(msg);
    }

    if (msg->state == READ_BODY) {
        SDLNet_CheckSockets(spstate.socketset, timeout);
        if (!SDLNet_SocketReady(socket)) {
            return Lb_OK;
        }

        if (read_stage(socket, msg->buffer, msg->msg_size) == Lb_FAIL) {
            return Lb_FAIL;
        }
    }

    assert(msg->state == READ_BODY);
    msg->state = READ_FINISHED;

    NETDBG(8, "Read of %u bytes finished", msg->msg_size);

    return Lb_OK;
}

static TbError tcpSP_init(NetDropCallback drop_callback)
{
    NETDBG(3, "Starting");

    LbMemorySet(&spstate, 0, sizeof(spstate));

    if (SDLNet_Init() < 0) {
        return Lb_FAIL;
    }

    spstate.drop_callback = drop_callback;

    return Lb_OK;
}

static void tcpSP_exit(void)
{
    if (spstate.ishost) {
        for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
        {
            SDLNet_TCP_Close(spstate.peers[i].socket);
            LbMemoryFree(spstate.peers[i].msg.buffer);
        }
    }

    SDLNet_TCP_Close(spstate.socket);
    LbMemoryFree(spstate.servermsg.buffer);

    SDLNet_FreeSocketSet(spstate.socketset);

    SDLNet_Quit();

    LbMemorySet(&spstate, 0, sizeof(spstate));
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

    IPaddress addr;
    addr.host = INADDR_ANY;
    SDLNet_Write16(atoi(portstr), &addr.port);

    spstate.socket = SDLNet_TCP_Open(&addr);
    if (spstate.socket == NULL) {
        ERRORLOG("Failed to initialize TCP server socket to port %s", portstr);
        return Lb_FAIL;
    }

    spstate.socketset = SDLNet_AllocSocketSet(MAX_N_PEERS);
    spstate.ishost = 1;

    return Lb_OK;
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

    IPaddress addr;
    SDLNet_ResolveHost(&addr, hostname, atoi(portstr));

    spstate.socket = SDLNet_TCP_Open(&addr);
    if (spstate.socket == NULL) {
        LbMemoryFree(hostname);
        NETMSG("Failed to initialize TCP client socket to host %s and port %s", hostname, portstr);
        return Lb_FAIL;
    }
    LbMemoryFree(hostname);

    spstate.socketset = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(spstate.socketset, spstate.socket);
    reset_msg(&spstate.servermsg);
    spstate.ishost = 0;

    return Lb_OK;
}

static void tcpSP_update(NetNewUserCallback new_user)
{
    assert(new_user);
    NETDBG(8, "Starting");

    if (!spstate.ishost) {
        return; //clients don't need to do anything here
    }

    TCPsocket new_socket;
    while ((new_socket = SDLNet_TCP_Accept(spstate.socket)) != NULL)
    { //does not block
        unsigned index;
        for (index = 0; index < MAX_N_PEERS; ++index)
        {
            if (spstate.peers[index].socket == NULL) {
                break;
            }
        }

        NetUserId id;
        if (index < MAX_N_PEERS && new_user(&id))
        {
            spstate.peers[index].id = id;
            spstate.peers[index].socket = new_socket;
            SDLNet_TCP_AddSocket(spstate.socketset, new_socket);
            reset_msg(&spstate.peers[index].msg);
        }
        else {
            SDLNet_TCP_Close(new_socket);
            NETMSG("Socket dropped because server is full");
        }
    }
}

static void tcpSP_sendmsg_single(NetUserId destination, const char * buffer, size_t size)
{
    assert(buffer);
    assert(size > 0);
    assert(!(spstate.ishost && destination == SERVER_ID));

    NETDBG(9, "Starting for buffer of %u bytes to user %u", size, destination);

    if (    send_buffer(find_peer_socket(destination), (const char*) &size, 4) == Lb_FAIL ||
            send_buffer(find_peer_socket(destination), buffer, size) == Lb_FAIL) {
        clear_peer(destination);
        if (spstate.drop_callback) {
            spstate.drop_callback(destination, NETDROP_ERROR);
        }
    }
}

static void tcpSP_sendmsg_all(const char * buffer, size_t size)
{
    assert(buffer);
    assert(size > 0);
    NETDBG(9, "Starting for buffer of %u bytes", size);

    if (spstate.ishost) {
        for (unsigned int i = 0; i < MAX_N_PEERS; ++i)
        {
            if (spstate.peers[i].socket == NULL) {
                continue;
            }

            if (    send_buffer(spstate.peers[i].socket, (const char*) &size, 4) == Lb_FAIL ||
                    send_buffer(spstate.peers[i].socket, buffer, size) == Lb_FAIL) {
                NetUserId id = spstate.peers[i].id;
                clear_peer(id);
                if (spstate.drop_callback) {
                    spstate.drop_callback(id, NETDROP_ERROR);
                }
            }
        }
    }
    else {
        if (    send_buffer(spstate.socket, (const char*) &size, 4) == Lb_FAIL ||
                send_buffer(spstate.socket, buffer, size) == Lb_FAIL) {
            clear_peer(SERVER_ID);
            if (spstate.drop_callback) {
                spstate.drop_callback(SERVER_ID, NETDROP_ERROR);
            }
        }
    }
}

static size_t tcpSP_msgready(NetUserId source, unsigned timeout)
{
    NETDBG(9, "Starting message ready check for user %u", source);

    struct Msg* msg = find_peer_message(source);
    if (!msg) {
        return 0;
    }

    if (msg->state == READ_FINISHED) {
        return msg->msg_size;
    }

    if (read_partial(find_peer_socket(source), msg, timeout) == Lb_FAIL) {
        clear_peer(source);
        if (spstate.drop_callback) {
            spstate.drop_callback(source, NETDROP_ERROR);
        }

        return 0;
    }

    if (msg->state == READ_FINISHED) {
        return msg->msg_size;
    }

    return 0;
}

static size_t tcpSP_readmsg(NetUserId source, char * buffer, size_t max_size)
{
    size_t size;

    assert(buffer);
    assert(max_size > 0);
    NETDBG(9, "Starting read from user %u", source);

    struct Msg* msg = find_peer_message(source);
    if (!msg) {
        return 0;
    }

    if (msg->state == READ_FINISHED) {
        size = min(msg->msg_size, max_size);
        LbMemoryCopy(buffer, msg->buffer, size);
        reset_msg(msg);
        return size;
    }

    if (read_full(find_peer_socket(source), msg) == Lb_FAIL) {
        clear_peer(source);
        if (spstate.drop_callback) {
            spstate.drop_callback(source, NETDROP_ERROR);
        }

        return 0;
    }

    assert(msg->state == READ_FINISHED);

    size = min(msg->msg_size, max_size);
    LbMemoryCopy(buffer, msg->buffer, size);
    reset_msg(msg);
    return size;
}

static void tcpSP_drop_user(NetUserId id)
{
    if (clear_peer(id) && spstate.drop_callback) {
        spstate.drop_callback(id, NETDROP_MANUAL);
    }
}
