/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_main.c
 *     Shared network support for Dungeon Keeper multiplayer.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_main.h"

#include "bflib_enet.h"
#include "post_inc.h"

static struct TbNetworkPlayerInfo *local_player_info;

struct NetState netstate;

TbBool IsUserActive(NetUserId id)
{
    return (netstate.users[id].progress == USER_LOGGEDIN);
}

void UpdateLocalPlayerInfo(NetUserId id)
{
    local_player_info[id].network_user_active = (netstate.users[id].progress != USER_UNUSED);
    if (!local_player_info[id].network_user_active) {
        memset(local_player_info[id].name, 0, sizeof(local_player_info[id].name));
        return;
    }
    strcpy(local_player_info[id].name, netstate.users[id].name);
}

char *begin_net_message(enum NetMessageType msg_type)
{
    char *write_pos = netstate.msg_buffer;
    *write_pos = msg_type;
    return write_pos + 1;
}

void send_message_buffer(NetUserId dest, const char *end_ptr)
{
    size_t message_size = end_ptr - netstate.msg_buffer;
    netstate.sp->sendmsg_single(dest, netstate.msg_buffer, message_size);
}

void send_remote_buffer(const char *end_ptr)
{
    size_t message_size = end_ptr - netstate.msg_buffer;
    if (netstate.my_id != SERVER_ID) {
        if (IsUserActive(SERVER_ID)) {
            netstate.sp->sendmsg_single(SERVER_ID, netstate.msg_buffer, message_size);
        }
        return;
    }
    for (NetUserId id = 0; id < (NetUserId)netstate.max_players; id += 1) {
        if (id == netstate.my_id || !IsUserActive(id)) {
            continue;
        }
        netstate.sp->sendmsg_single(id, netstate.msg_buffer, message_size);
    }
}

void SendUserUpdate(NetUserId dest, NetUserId updated_user)
{
    char *write_pos = begin_net_message(NETMSG_USERUPDATE);
    *write_pos = updated_user;
    write_pos += 1;
    *write_pos = netstate.users[updated_user].progress;
    write_pos += 1;
    strcpy(write_pos, netstate.users[updated_user].name);
    write_pos += strlen(netstate.users[updated_user].name) + 1;
    send_message_buffer(dest, write_pos);
}

TbError LbNetwork_Init(uint32_t srvcindex, uint32_t maxplayrs, struct TbNetworkPlayerInfo *locplayr, struct ServiceInitData *)
{
    local_player_info = locplayr;
    memset(&netstate, 0, sizeof(netstate));
    netstate.max_players = maxplayrs;
    for (NetUserId user_id = 0; user_id < (NetUserId)netstate.max_players; user_id += 1) {
        netstate.users[user_id].id = user_id;
    }
    if (srvcindex == NS_ENET_UDP) {
        netstate.sp = InitEnetSP();
        NETMSG("Selecting UDP");
    } else {
        WARNLOG("The serviceIndex value of %u is out of range", srvcindex);
    }
    if (!netstate.sp) {
        return Lb_FAIL;
    }
    return netstate.sp->init(OnDroppedUser);
}

TbBool OnNewUser(NetUserId *assigned_id)
{
    if (netstate.locked) {
        return false;
    }
    for (NetUserId id = 0; id < (NetUserId)netstate.max_players; id += 1) {
        if (netstate.users[id].progress == USER_UNUSED) {
            *assigned_id = id;
            netstate.users[id].progress = USER_CONNECTED;
            netstate.users[id].ack = -1;
            NETLOG("Assigning new user to ID %u", id);
            return true;
        }
    }
    return false;
}

void OnDroppedUser(NetUserId id, enum NetDropReason reason)
{
    assert(id >= 0);
    assert(id < (int)netstate.max_players);
    if (netstate.my_id == id) {
        NETMSG("Warning: Trying to drop local user. There's a bug in code somewhere, probably server trying to send message to itself.");
        return;
    }
    if (netstate.users[id].progress == USER_UNUSED) {
        return;
    }
    if (reason == NETDROP_ERROR) {
        NETMSG("User left (or connection error): %i %s", id, netstate.users[id].name);
    } else if (reason == NETDROP_MANUAL) {
        NETMSG("Dropped user %i %s", id, netstate.users[id].name);
    }
    memset(&netstate.users[id], 0, sizeof(netstate.users[id]));
    netstate.users[id].id = id;
    if (id != SERVER_ID) {
        for (NetUserId user_id = 0; user_id < (NetUserId)netstate.max_players; user_id += 1) {
            if (user_id == netstate.my_id) {
                continue;
            }
            SendUserUpdate(user_id, id);
        }
    }
    UpdateLocalPlayerInfo(id);
}
