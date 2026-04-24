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
#include "ver_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

enum NetUserProgress {
    USER_UNUSED = 0,
    USER_CONNECTED,
    USER_LOGGEDIN
};

struct GameVersionPacket {
    int32_t major;
    int32_t minor;
    int32_t release;
    int32_t build;
};

struct NetUser {
    NetUserId id;
    char name[32];
    enum NetUserProgress progress;
    int ack;
    struct GameVersionPacket version;
};

struct NetFrame {
    struct NetFrame *next;
    char *buffer;
    int seq_nbr;
    size_t size;
};

#define NET_MSG_BUFFER_SIZE 5000
#define INVALID_USER_ID 23456

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

static const struct GameVersionPacket net_current_version = { VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD };

static inline TbBool net_versions_match(const struct GameVersionPacket *version_a, const struct GameVersionPacket *version_b)
{
    return (version_a->major == version_b->major) &&
        (version_a->minor == version_b->minor) &&
        (version_a->release == version_b->release) &&
        (version_a->build == version_b->build);
}

TbBool OnNewUser(NetUserId *assigned_id);
void OnDroppedUser(NetUserId id, enum NetDropReason reason);
TbBool IsUserActive(NetUserId id);
void UpdateLocalPlayerInfo(NetUserId id);
char* InitMessageBuffer(enum NetMessageType msg_type);
void SendMessage(NetUserId dest, const char* end_ptr);
void SendUserUpdate(NetUserId dest, NetUserId updated_user);

#ifdef __cplusplus
}
#endif

#endif
