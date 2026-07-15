/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_main.h
 *     Header file for net_main.c.
 * @par Purpose:
 *     Public declarations for shared multiplayer network support routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     09 May 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_MAIN_H
#define DK_NET_MAIN_H

#include "bflib_basics.h"
#include "ver_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIMEOUT_CONNECT_HOLEPUNCH 5000
#define TIMEOUT_CONNECT_DIRECT_IPV6 5000
#define TIMEOUT_CONNECT_DIRECT_IPV4 5000
#define TIMEOUT_JOIN_LOBBY 2000
#define TIMEOUT_LOBBY_EXCHANGE 5000
#define TIMEOUT_WAIT_FOR_ALL_PLAYERS 30000
#define PEER_TIMEOUT_LIMIT 0
#define PEER_TIMEOUT_MIN_MS 5000
#define PEER_TIMEOUT_MAX_MS 30000

#define MAX_NET_USERS 4
#define MAX_NET_PEERS (MAX_NET_USERS - 1)
#define SERVER_ID 0
#define NET_MSG_BUFFER_SIZE 5000
#define INVALID_USER_ID 23456

typedef int NetUserId;

enum NetDropReason {
    NETDROP_MANUAL,
    NETDROP_ERROR,
};

enum NetMessageType {
    NETMSG_LOGIN,
    NETMSG_USERUPDATE,
    NETMSG_FRONTEND,
    NETMSG_CLIENT_IS_READY,
    NETMSG_HOST_DECLARES_START,
    NETMSG_STARTUP_SYNC,
    NETMSG_GAMEPLAY_UNSEQUENCED,
    NETMSG_RESYNC_DATA,
    NETMSG_UNPAUSE,
    NETMSG_CHATMESSAGE,
    NETMSG_GAMEPLAY_REPAIR,
    NETMSG_GAMEPLAY_TURN_SYNC,
};

typedef TbBool (*NetNewUserCallback)(NetUserId *assigned_id);
typedef void (*NetDropCallback)(NetUserId id, enum NetDropReason reason);

struct NetSP
{
    TbError (*init)(NetDropCallback drop_callback);
    void (*exit)();
    TbError (*host)(const char *session, void *options);
    TbError (*join)(const char *session, void *options);
    void (*update)(NetNewUserCallback new_user);
    void (*sendmsg_single)(NetUserId destination, const char *buffer, size_t size);
    void (*sendmsg_single_unsequenced)(NetUserId destination, const char *buffer, size_t size);
    void (*sendmsg_all)(const char *buffer, size_t size);
    size_t (*msgready)(NetUserId source, unsigned timeout);
    size_t (*readmsg)(NetUserId source, char *buffer, size_t max_size);
    void (*drop_user)(NetUserId id);
};

enum NetUserProgress {
    USER_UNUSED = 0,
    USER_CONNECTED,
    USER_LOGGEDIN,
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

struct NetState {
    const struct NetSP *sp;
    struct NetUser users[MAX_NET_USERS];
    struct NetFrame *exchg_queue;
    char password[32];
    NetUserId my_id;
    int seq_nbr;
    unsigned max_players;
    char msg_buffer[NET_MSG_BUFFER_SIZE];
    char msg_buffer_null;
    TbBool locked;
    TbClockMSec frontend_start_pending_end_time;
};

struct TbNetworkPlayerInfo {
    char name[32];
    int32_t network_user_active;
};

struct ServiceInitData {
    int32_t service_flags;
    int32_t max_connections;
    int32_t buffer_size;
    int32_t timeout_value;
};

enum TbNetworkService {
    NS_ENET_UDP,
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

TbError LbNetwork_Init(uint32_t srvcindex, uint32_t maxplayrs, struct TbNetworkPlayerInfo *locplayr, struct ServiceInitData *init_data);
TbBool OnNewUser(NetUserId *assigned_id);
void OnDroppedUser(NetUserId id, enum NetDropReason reason);
TbBool IsUserActive(NetUserId id);
void UpdateLocalPlayerInfo(NetUserId id);
char *begin_net_message(enum NetMessageType msg_type);
void send_message_buffer(NetUserId dest, const char *end_ptr);
void send_remote_buffer(const char *end_ptr);
void SendUserUpdate(NetUserId dest, NetUserId updated_user);

#ifdef __cplusplus
}
#endif

#endif
