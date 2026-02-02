/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_network.cpp
 *     Network support library.
 * @par Purpose:
 *     Network support routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Apr 2009 - 13 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_network.h"
#include "bflib_network_internal.h"
#include "bflib_enet.h"
#include "bflib_datetm.h"
#include "bflib_network_exchange.h"
#include "bflib_netsession.h"
#include "bflib_sound.h"
#include "globals.h"
#include "frontend.h"
#include "net_game.h"
#include "front_landview.h"
#include "front_network.h"
#include "net_received_packets.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
void gameplay_loop_draw();
extern "C" void network_yield_draw_gameplay();
#endif

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// External function declarations

/******************************************************************************/

struct TbNetworkPlayerInfo *localPlayerInfoPtr;
static int ServerPort = 0;
#define SESSION_COUNT 32
struct NetState netstate;
static struct TbNetworkSessionNameEntry sessions[SESSION_COUNT];

TbBool IsUserActive(NetUserId id) {
    return (netstate.users[id].progress == USER_LOGGEDIN);
}

void UpdateLocalPlayerInfo(NetUserId id) {
    localPlayerInfoPtr[id].active = (netstate.users[id].progress != USER_UNUSED);
    if (!localPlayerInfoPtr[id].active) {
        memset(localPlayerInfoPtr[id].name, 0, sizeof(localPlayerInfoPtr[id].name));
        return;
    }
    strcpy(localPlayerInfoPtr[id].name, netstate.users[id].name);
}

void SendUserUpdate(NetUserId dest, NetUserId updated_user) {
    char * ptr = InitMessageBuffer(NETMSG_USERUPDATE);
    *ptr = updated_user;
    ptr += 1;
    *ptr = netstate.users[updated_user].progress;
    ptr += 1;
    strcpy(ptr, netstate.users[updated_user].name);
    ptr += strlen(netstate.users[updated_user].name) + 1;
    SendMessage(dest, ptr);
}

void LbNetwork_SetServerPort(int port) {
    ServerPort = port;
}

static void AddSessionSegment(const char* start, const char* end) {
    if (start == end) { return; }
    unsigned i;
    for (i = 0; i < SESSION_COUNT; i += 1) {
        if (sessions[i].in_use) { continue; }
        sessions[i].in_use = 1;
        sessions[i].joinable = 1;
        size_t seglen = (size_t)(end - start);
        net_copy_name_string(sessions[i].text, start, min((size_t)SESSION_NAME_MAX_LEN, seglen + 1));
        return;
    }
}

void LbNetwork_InitSessionsFromCmdLine(const char * str) {
    NETMSG("Initializing sessions from command line: %s", str);
    const char* start = str;
    const char* end = str;
    while (*end != '\0') {
        if (start != end && (*end == ',' || *end == ';')) {
            AddSessionSegment(start, end);
            start = end + 1;
        }
        end += 1;
    }
    AddSessionSegment(start, end);
}

TbError LbNetwork_Init(unsigned long srvcindex, unsigned long maxplayrs, struct TbNetworkPlayerInfo *locplayr, struct ServiceInitData *init_data) {
    localPlayerInfoPtr = locplayr;
    memset(&netstate, 0, sizeof(netstate));
    netstate.max_players = maxplayrs;
    NetUserId usr;
    for (usr = 0; usr < netstate.max_players; usr += 1) {
        netstate.users[usr].id = usr;
    }
    if (srvcindex == NS_TCP_IP) {
        NETMSG("Selecting TCP/IP SP");
        netstate.sp = &tcpSP;
    } else if (srvcindex == NS_ENET_UDP) {
        netstate.sp = InitEnetSP();
        NETMSG("Selecting UDP");
    } else {
        WARNLOG("The serviceIndex value of %lu is out of range", srvcindex);
    }
    if (!netstate.sp) {
        return Lb_FAIL;
    }
    return netstate.sp->init(OnDroppedUser);
}

TbError LbNetwork_Create(char *nsname_str, char *plyr_name, uint32_t *plyr_num, void *optns) {
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }
    const char *port = ":5555";
    char buf[16] = "";
    if (ServerPort != 0) {
        snprintf(buf, sizeof(buf), "%d", ServerPort);
        port = buf;
    }
    if (netstate.sp->host(port, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }
    netstate.my_id = SERVER_ID;
    snprintf(netstate.users[netstate.my_id].name, sizeof(netstate.users[netstate.my_id].name), "%s", plyr_name);
    netstate.users[netstate.my_id].progress = USER_LOGGEDIN;
    *plyr_num = netstate.my_id;
    UpdateLocalPlayerInfo(netstate.my_id);
    LbNetwork_EnableNewPlayers(true);
    return Lb_OK;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, int32_t *plyr_num, void *optns) {
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }
    if (netstate.sp->join(nsname->text, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }
    netstate.my_id = INVALID_USER_ID;
    if (LbNetwork_ExchangeLogin(plyr_name) == Lb_FAIL) {
        return Lb_FAIL;
    }
    *plyr_num = netstate.my_id;
    return Lb_OK;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow) {
    if (!netstate.locked && !allow) {
        NetUserId i;
        for (i = 0; i < netstate.max_players; i += 1) {
            if (netstate.users[i].progress == USER_CONNECTED) {
                netstate.sp->drop_user(i);
            }
        }
    }
    netstate.locked = !allow;
    if (netstate.locked) {
        NETMSG("New players are NOT allowed to join");
    } else {
        NETMSG("New players ARE allowed to join");
    }
    return Lb_OK;
}

TbError LbNetwork_Stop(void) {
    if (netstate.sp) {
        netstate.sp->exit();
    }
    memset(&netstate, 0, sizeof(netstate));
    netstate.my_id = INVALID_USER_ID;
    return Lb_OK;
}

TbBool OnNewUser(NetUserId * assigned_id) {
    if (netstate.locked) {
        return 0;
    }
    NetUserId i;
    for (i = 0; i < netstate.max_players; i += 1) {
        if (netstate.users[i].progress == USER_UNUSED) {
            *assigned_id = i;
            netstate.users[i].progress = USER_CONNECTED;
            netstate.users[i].ack = -1;
            NETLOG("Assigning new user to ID %u", i);
            return 1;
        }
    }
    return 0;
}

void OnDroppedUser(NetUserId id, enum NetDropReason reason) {
    assert(id >= 0);
    assert(id < (int)netstate.max_players);
    if (netstate.my_id == id) {
        NETMSG("Warning: Trying to drop local user. There's a bug in code somewhere, probably server trying to send message to itself.");
        return;
    }
    if (reason == NETDROP_ERROR) {
        NETMSG("Connection error with user %i %s", id, netstate.users[id].name);
    } else if (reason == NETDROP_MANUAL) {
        NETMSG("Dropped user %i %s", id, netstate.users[id].name);
    }
    if (netstate.my_id != SERVER_ID) {
        NETMSG("Quitting after connection loss");
        LbNetwork_Stop();
        return;
    }
    memset(&netstate.users[id], 0, sizeof(netstate.users[id]));
    netstate.users[id].id = id;
    NetUserId uid;
    for (uid = 0; uid < netstate.max_players; uid += 1) {
        if (uid == netstate.my_id) { continue; }
        SendUserUpdate(uid, id);
    }
    UpdateLocalPlayerInfo(id);
}

TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *ptr) {
    struct TbNetworkCallbackData netcdat = {};
    strcpy(netcdat.svc_name, "TCP");
    callback(&netcdat, ptr);
    strcpy(netcdat.svc_name, "ENET/UDP");
    callback(&netcdat, ptr);
    NETMSG("Enumerate Services called");
    return Lb_OK;
}

TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc callback, void *buf) {
    TbNetworkCallbackData data;
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (!IsUserActive(id)) { continue; }
        memset(&data, 0, sizeof(data));
        snprintf(data.plyr_name, sizeof(data.plyr_name), "%s", netstate.users[id].name);
        callback(&data, buf);
    }
    return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr) {
    unsigned i;
    for (i = 0; i < SESSION_COUNT; i += 1) {
        if (!sessions[i].in_use) { continue; }
        callback((TbNetworkCallbackData *) &sessions[i], ptr);
    }
    return Lb_OK;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
