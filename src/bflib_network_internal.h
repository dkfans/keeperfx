/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_network_internal.h
 *     Internal network structures shared between network modules.
 * @par Purpose:
 *     Internal network data structures and definitions.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     31 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_NETWORK_INTERNAL_H
#define BFLIB_NETWORK_INTERNAL_H

#include "bflib_network.h"

#ifdef __cplusplus
extern "C" {
#endif

enum NetUserProgress {
    USER_UNUSED = 0,
    USER_CONNECTED,
    USER_LOGGEDIN,
    USER_SERVER
};

struct NetUser {
    NetUserId id;
    char name[32];
    enum NetUserProgress progress;
    int ack;
};

struct NetFrame {
    struct NetFrame *next;
    char *buffer;
    int seq_nbr;
    size_t size;
};

enum NetMessageType {
    NETMSG_LOGIN,
    NETMSG_USERUPDATE,
    NETMSG_FRAME,
    NETMSG_RESYNC_DATA,
    NETMSG_RESYNC_READY,
    NETMSG_RESYNC_RESUME,
    NETMSG_TIMESYNC_REQUEST,
    NETMSG_TIMESYNC_REPLY,
};

#define NET_MSG_BUFFER_SIZE 5000

struct NetState {
    const struct NetSP *sp;
    struct NetUser users[MAX_N_USERS];
    struct NetFrame *exchg_queue;
    char password[32];
    NetUserId my_id;
    int seq_nbr;
    unsigned max_players;
    char msg_buffer[NET_MSG_BUFFER_SIZE];
    char msg_buffer_null;
    TbBool locked;
};

extern struct NetState netstate;

TbBool OnNewUser(NetUserId *assigned_id);
void OnDroppedUser(NetUserId id, enum NetDropReason reason);

#ifdef __cplusplus
}
#endif

#endif
