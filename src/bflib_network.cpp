/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_network.c
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
#include "bflib_basics.h"
#include "bflib_enet.h"
#include "bflib_datetm.h"
#include "bflib_netsession.h"
#include "bflib_netsp.hpp"
#include "bflib_netsp_ipx.hpp"
#include "bflib_sound.h"
#include <zlib.h>
#include "globals.h"
#include <assert.h>
#include <ctype.h>

//TODO: get rid of the following headers later by refactoring, they're here for testing primarily
#include "frontend.h"
#include "net_game.h"
#include "packets.h"
#include "front_landview.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// External function declarations
extern void draw_out_of_sync_box(long a1, long a2, long box_width);

/******************************************************************************/
// Local functions definition
static void OnDroppedUser(NetUserId id, enum NetDropReason reason);
static void ProcessMessagesUntilNextLoginReply(TbClockMSec timeout, void *server_buf, size_t client_frame_size);
/******************************************************************************/

struct TbNetworkPlayerInfo *localPlayerInfoPtr;
unsigned long hostId;

static int ServerPort = 0;
/******************************************************************************/

// New network code declarations start here ===================================

/**
 * Max wait for a client before we declare client messed up.
 */
#define WAIT_FOR_CLIENT_TIMEOUT_IN_MS   10000
#define WAIT_FOR_SERVER_TIMEOUT_IN_MS   WAIT_FOR_CLIENT_TIMEOUT_IN_MS

#define SESSION_COUNT 32 //not arbitrary, it's what code calling EnumerateSessions expects

enum NetUserProgress
{
	USER_UNUSED = 0,		//array slot unused
    USER_CONNECTED,			//connected user on slot
    USER_LOGGEDIN,          //sent name and password and was accepted
    USER_SERVER             //none of the above states are applicable because this is server
};

struct NetUser
{
    NetUserId               id; //same as array index. server always 0
    char                    name[32];
	enum NetUserProgress	progress;
	int                     ack; //last sequence number processed
};

struct NetFrame
{
    struct NetFrame *       next;
    char *                  buffer;
    int                     seq_nbr;
    size_t                  size;
};

/*
 * This should be squished into TbPacketAction
 */
enum NetMessageType
{
    NETMSG_LOGIN,           //to server: username and pass, from server: assigned id
    NETMSG_USERUPDATE,      //changed player from server
    NETMSG_FRAME,           //to server: ACK of frame + packets, from server: the frame itself
    // Not used: NETMSG_LAGWARNING,      //from server: notice that some client is lagging
    NETMSG_RESYNC,          //from server: re-synchronization is occurring
};

/**
 * Structure for network messages for illustrational purposes.
 * I don't actually load into this structure as it takes too much effort with C.
 */
struct NetworkMessageExample
{
    char         type; //enum NetMessageType
    union NetMessageBody
    {
        struct
        {
            char                password[32];
            char                username[32];
        }                       login_request;

        NetUserId               user;       //in login response or lag warning
        NetUser                 user_update;
        struct NetFrame         frame;
    } body;
};

/**
 * Contains the entire network state.
 */
struct NetState
{
    const struct NetSP *    sp;                 //pointer to service provider in use
    struct NetUser          users[MAX_N_USERS]; //the users
    struct NetFrame *       exchg_queue;        //exchange queue from server
    char                    password[32];       //password for server
    NetUserId               my_id;              //id for user representing this machine
    int                     seq_nbr;            //sequence number of next frame to be issued
    unsigned                max_players;        //max players that will actually be used
    char                    msg_buffer[(sizeof(NetFrame) + sizeof(struct Packet)) * PACKETS_COUNT + 1]; //completely estimated for now
    char                    msg_buffer_null;    //theoretical safe guard vs non-terminated strings
    TbBool                  locked;             //if set, no players may join
};

//the "new" code contained in this struct
static struct NetState netstate;

//sessions placed here for now, would be smarter to store dynamically
static struct TbNetworkSessionNameEntry sessions[SESSION_COUNT]; //using original because enumerate expects static life time

// New network code data definitions end here =================================

//debug function to find out reason for mutating peer ids
static TbBool UserIdentifiersValid(void)
{
    NetUserId i;
    for (i = 0; i < MAX_N_USERS; ++i) {
        if (netstate.users[i].id != i) {
            NETMSG("Bad peer ID on index %i", i);
            return 0;
        }
    }

    return 1;
}

static void SendLoginRequest(const char * name, const char * password)
{
    char * buffer_ptr;

    NETMSG("Logging in as %s", name);

    buffer_ptr = netstate.msg_buffer;
    *buffer_ptr = NETMSG_LOGIN;
    buffer_ptr += 1;

    strcpy(buffer_ptr, password);
    buffer_ptr += strlen(password) + 1;

    strcpy(buffer_ptr, name); //don't want to bother saving length ahead
    buffer_ptr += strlen(name) + 1;

    netstate.sp->sendmsg_single(SERVER_ID, netstate.msg_buffer,
        buffer_ptr - netstate.msg_buffer);
}

static void SendUserUpdate(NetUserId dest, NetUserId updated_user)
{
    char * ptr;

    ptr = netstate.msg_buffer;

    *ptr = NETMSG_USERUPDATE;
    ptr += 1;

    *ptr = updated_user;
    ptr += 1;

    *ptr = netstate.users[updated_user].progress;
    ptr += 1;

    ptr += snprintf(ptr, sizeof(netstate.users[updated_user].name), "%s", netstate.users[updated_user].name) + 1;

    netstate.sp->sendmsg_single(dest, netstate.msg_buffer,
        ptr - netstate.msg_buffer);
}

static void SendClientFrame(const char * send_buf, size_t buf_size, int seq_nbr) //seq_nbr because it isn't necessarily determined
{
    char * ptr;

    NETDBG(9, "Starting");

    ptr = netstate.msg_buffer;

    *ptr = NETMSG_FRAME;
    ptr += 1;

    *(int *) ptr = seq_nbr;
    ptr += 4;

    memcpy(ptr, send_buf, buf_size);
    ptr += buf_size;

    netstate.sp->sendmsg_single(SERVER_ID, netstate.msg_buffer,
        ptr - netstate.msg_buffer);
}

static int CountLoggedInClients()
{
    NetUserId id;
    int count;

    for (count = 0, id = 0; id < netstate.max_players; ++id)
    {
        if (netstate.users[id].progress == USER_LOGGEDIN)
        {
            count++;
        }
    }

    return count;
}

static void SendServerFrame(const void *send_buf, size_t frame_size, int num_frames)
{
    char * ptr;

    NETDBG(9, "Starting");

    ptr = netstate.msg_buffer;
    *ptr = NETMSG_FRAME;
    ptr += sizeof(char);

    *(int *) ptr = netstate.seq_nbr;
    ptr += sizeof(int);

    *ptr = num_frames;
    ptr += sizeof(char);

    memcpy(ptr, send_buf, frame_size * num_frames);
    ptr += frame_size * num_frames;

    netstate.sp->sendmsg_all(netstate.msg_buffer, ptr - netstate.msg_buffer);
}

static void HandleLoginRequest(NetUserId source, char * ptr, char * end)
{
    size_t len;
    NetUserId id;

    NETDBG(7, "Starting");

    if (netstate.users[source].progress != USER_CONNECTED) {
        NETMSG("Peer was not in connected state");
        //TODO NET implement drop
        return;
    }

    if (netstate.password[0] != 0 && strncmp(ptr, netstate.password,
            sizeof(netstate.password)) != 0) {
        NETMSG("Peer chose wrong password");
        //TODO NET implement drop
        return;
    }

    len = strlen(ptr) + 1;
    ptr += len;
    if (len > sizeof(netstate.password)) {
        NETDBG(6, "Connected peer attempted to flood password");
        netstate.sp->drop_user(source);
        return;
    }

    snprintf(netstate.users[source].name, sizeof(netstate.users[source].name), "%s", ptr);
    if (!isalnum(netstate.users[source].name[0])) {
        //TODO NET drop player for bad name
        //also replace isalnum with something that considers foreign non-ASCII chars
        NETDBG(6, "Connected peer had bad name starting with %c",
            netstate.users[source].name[0]);
        netstate.sp->drop_user(source);
        return;
    }

    //presume login successful from here
    NETMSG("User %s successfully logged in", netstate.users[source].name);
    netstate.users[source].progress = USER_LOGGEDIN;
    play_non_3d_sample(76);

    //send reply
    ptr = netstate.msg_buffer;
    ptr += 1; //skip header byte which should still be ok
    memcpy(ptr, &source, 1); //assumes LE
    ptr += 1;
    netstate.sp->sendmsg_single(source, netstate.msg_buffer, ptr - netstate.msg_buffer);

    //send user updates
    ptr = netstate.msg_buffer;
    for (id = 0; id < MAX_N_USERS; ++id) {
        if (netstate.users[id].progress == USER_UNUSED) {
            continue;
        }

        SendUserUpdate(source, id);

        if (id == netstate.my_id || id == source) {
            continue;
        }

        SendUserUpdate(id, source);
    }

    //set up the stuff the other parts of the game expect
    //TODO NET try to get rid of this because it makes understanding code much more complicated
    localPlayerInfoPtr[source].active = 1;
    strcpy(localPlayerInfoPtr[source].name, netstate.users[source].name);
}

static void HandleLoginReply(char * ptr, char * end)
{
    NETDBG(7, "Starting");

    netstate.my_id = (NetUserId) *ptr;
}

static void HandleUserUpdate(NetUserId source, char * ptr, char * end)
{
    NetUserId id;

    NETDBG(7, "Starting");

    id = (NetUserId) *ptr;
    if (id < 0 && id >= MAX_N_USERS) {
        NETLOG("Critical error: Out of range user ID %i received from server, could be used for buffer overflow attack", id);
        abort();
    }
    ptr += 1;

    netstate.users[id].progress = (enum NetUserProgress) *ptr;
    ptr += 1;

    snprintf(netstate.users[id].name, sizeof(netstate.users[id].name), "%s", ptr);

    //send up the stuff the other parts of the game expect
    //TODO NET try to get rid of this because it makes understanding code much more complicated
    localPlayerInfoPtr[id].active = netstate.users[id].progress != USER_UNUSED;
    strcpy(localPlayerInfoPtr[id].name, netstate.users[id].name);
}

static void HandleClientFrame(NetUserId source, char *dst_ptr, const char * ptr, char * end, size_t frame_size)
{
    NETDBG(7, "Starting");

    netstate.users[source].ack = *(int *) ptr;
    ptr += 4;

    memcpy(dst_ptr, ptr, frame_size);
    ptr += frame_size;

    if (ptr >= end) {
        //TODO NET handle bad frame
        NETMSG("Bad frame size from client %u", source);
        return;
    }

    NETDBG(9, "Handled client frame of %u bytes", frame_size);
}

static void HandleServerFrame(char * ptr, char * end, size_t user_frame_size)
{
    int seq_nbr;
    NetFrame * frame;
    NetFrame * it;
    unsigned num_user_frames;

    NETDBG(7, "Starting");

    seq_nbr = *(int *) ptr;
    ptr += 4;

    num_user_frames = *ptr;
    ptr += 1;

    frame = (NetFrame *) calloc(sizeof(*frame), 1);
    if (netstate.exchg_queue == NULL)
    {
        netstate.exchg_queue = frame;
    }
    else
    {
        for (it = netstate.exchg_queue; it->next != NULL; it = it->next)
        {
        }
        it->next = frame;
    }

    frame->next = NULL;
    frame->size = num_user_frames * user_frame_size;
    frame->buffer = (char *) calloc(frame->size, 1);
    frame->seq_nbr = seq_nbr;

    memcpy(frame->buffer, ptr, frame->size);

    NETDBG(9, "Handled server frame of %u bytes", frame->size);
}

static void HandleMessageFromServer(NetUserId source, size_t frame_size)
{
    //this is a very bad way to do network message parsing, but it is what C offers
    //(I could also load into it memory by some complicated system with data description
    //auxiliary structures which I don't got time to code nor do the requirements
    //justify it)

    char * buffer_ptr;
    char * buffer_end;
    size_t buffer_size;
    enum NetMessageType type;

    NETDBG(7, "Handling message from %u", source);

    buffer_ptr = netstate.msg_buffer;
    buffer_size = sizeof(netstate.msg_buffer);
    buffer_end = buffer_ptr + buffer_size;

    //type
    type = (enum NetMessageType) *buffer_ptr;
    buffer_ptr += 1;

    switch (type) {
        case NETMSG_LOGIN:
            HandleLoginReply(buffer_ptr, buffer_end);
            break;
        case NETMSG_USERUPDATE:
            HandleUserUpdate(source, buffer_ptr, buffer_end);
            break;
        case NETMSG_FRAME:
            HandleServerFrame(buffer_ptr, buffer_end, frame_size);
            break;
        default:
            break;
    }
}

static void HandleMessageFromClient(NetUserId source, void *server_buf, size_t frame_size)
{
    //this is a very bad way to do network message parsing, but it is what C offers
    //(I could also load into it memory by some complicated system with data description
    //auxiliary structures which I don't got time to code nor do the requirements
    //justify it)

    char * buffer_ptr;
    char * buffer_end;
    size_t buffer_size;
    enum NetMessageType type;

    NETDBG(7, "Handling message from %u", source);

    buffer_ptr = netstate.msg_buffer;
    buffer_size = sizeof(netstate.msg_buffer);
    buffer_end = buffer_ptr + buffer_size;

    //type
    type = (enum NetMessageType) *buffer_ptr;
    buffer_ptr += 1;

    switch (type) {
        case NETMSG_LOGIN:
            HandleLoginRequest(source, buffer_ptr, buffer_end);
            break;
        case NETMSG_USERUPDATE:
            WARNLOG("Unexpected USERUPDATE");
            break;
        case NETMSG_FRAME:
            HandleClientFrame(source,((char*)server_buf) + source * frame_size,
                              buffer_ptr, buffer_end, frame_size);
            break;
        default:
            break;
    }
}

static TbError ProcessMessage(NetUserId source, void* server_buf, size_t frame_size)
{
    size_t rcount;

    rcount = netstate.sp->readmsg(source, netstate.msg_buffer, sizeof(netstate.msg_buffer));

    if (rcount > 0)
    {
        if (source == SERVER_ID)
        {
            HandleMessageFromServer(source, frame_size);
        }
        else
        {
            HandleMessageFromClient(source, server_buf, frame_size);
        }
    }
    else
    {
        NETLOG("Problem reading message from %u", source);
        return Lb_FAIL;
    }

    return Lb_OK;
}

static void AddSession(const char * str, size_t len)
{
    unsigned i;

    for (i = 0; i < SESSION_COUNT; ++i) {
        if (sessions[i].in_use) {
            continue;
        }

        sessions[i].in_use = 1;
        sessions[i].joinable = 1; //actually we don't know, but keep for now
        net_copy_name_string(sessions[i].text, str, min((size_t)SESSION_NAME_MAX_LEN, len + 1));

        break;
    }
}

void LbNetwork_SetServerPort(int port)
{
    ServerPort = port;
}

void LbNetwork_InitSessionsFromCmdLine(const char * str)
{
    const char* start;
    const char* end;

    NETMSG("Initializing sessions from command line: %s", str);

    start = end = str;

    while (*end != '\0') {
        if (start != end && (*end == ',' || *end == ';')) {
            AddSession(start, end - start);
            start = end + 1;
        }

        ++end;
    }

    if (start != end) {
        AddSession(start, end - start);
    }
}

TbError LbNetwork_Init(unsigned long srvcindex, unsigned long maxplayrs, struct TbNetworkPlayerInfo *locplayr, struct ServiceInitData *init_data)
{
  TbError res;
  NetUserId usr;

  res = Lb_FAIL;

  localPlayerInfoPtr = locplayr; //TODO NET try to get rid of dependency on external player list, makes things 2x more complicated

  //clear network object and init it to neutral config
  memset(&netstate, 0, sizeof(netstate));
  for (usr = 0; usr < MAX_N_USERS; ++usr) {
      netstate.users[usr].id = usr;
  }

  netstate.max_players = maxplayrs;

  // Initialising the service provider object
  switch (srvcindex)
  {
  case NS_TCP_IP:
      NETMSG("Selecting TCP/IP SP");
      netstate.sp = &tcpSP;

      break;
  case NS_ENET_UDP:
      netstate.sp = InitEnetSP();
      NETMSG("Selecting UDP");
      break;
  default:
      WARNLOG("The serviceIndex value of %lu is out of range", srvcindex);
      res = Lb_FAIL;
      break;
  }

  if (netstate.sp) {
      res = netstate.sp->init(OnDroppedUser); //TODO NET supply drop callback
  }

  return res;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, long *plyr_num, void *optns)
{
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }

    if (netstate.sp->join(nsname->text, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }

    netstate.my_id = 23456;

    SendLoginRequest(plyr_name, netstate.password);
    ProcessMessagesUntilNextLoginReply(WAIT_FOR_SERVER_TIMEOUT_IN_MS, &net_screen_packet, sizeof(struct ScreenPacket));
    if (netstate.msg_buffer[0] != NETMSG_LOGIN) {
        fprintf(stderr, "Network login rejected");
        return Lb_FAIL;
    }
    ProcessMessage(SERVER_ID, &net_screen_packet, sizeof (struct ScreenPacket));

    if (netstate.my_id == 23456) {
        fprintf(stderr, "Network login unsuccessful");
        return Lb_FAIL;
    }

    *plyr_num = netstate.my_id;

    return Lb_OK;
}

TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num, void *optns)
{
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }

    if (ServerPort != 0)
    {
        char buf[16] = "";
        snprintf(buf, sizeof(buf), "%d", ServerPort);
        if (netstate.sp->host(buf, optns) == Lb_FAIL) {
            return Lb_FAIL;
        }
    }
    else
    {
        if (netstate.sp->host(":5555", optns) == Lb_FAIL) {
            return Lb_FAIL;
        }
    }

    netstate.my_id = SERVER_ID;
    snprintf(netstate.users[netstate.my_id].name, sizeof(netstate.users[netstate.my_id].name), "%s", plyr_name);
    netstate.users[netstate.my_id].progress = USER_SERVER;

    *plyr_num = netstate.my_id;
    localPlayerInfoPtr[netstate.my_id].active = 1;
    strcpy(localPlayerInfoPtr[netstate.my_id].name, netstate.users[netstate.my_id].name);

    LbNetwork_EnableNewPlayers(true);
    return Lb_OK;
}

TbError LbNetwork_Stop(void)
{
    NetFrame* frame;
    NetFrame* nextframe;

    if (netstate.sp) {
        netstate.sp->exit();
    }

    frame = netstate.exchg_queue;
    while (frame != NULL) {
        nextframe = frame->next;
        free(frame->buffer);
        free(frame);
        frame = nextframe;
    }

    memset(&netstate, 0, sizeof(netstate));

    return Lb_OK;
}

static TbBool OnNewUser(NetUserId * assigned_id)
{
    NetUserId i;

    if (netstate.locked) {
        return 0;
    }

    for (i = 0; i < MAX_N_USERS; ++i) {
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

static void OnDroppedUser(NetUserId id, enum NetDropReason reason)
{
    int i;

    assert(id >= 0);
    assert(id < MAX_N_USERS);

    if (netstate.my_id == id) {
        NETMSG("Warning: Trying to drop local user. There's a bug in code somewhere, probably server trying to send message to itself.");
        return;
    }

    //return;
    if (reason == NETDROP_ERROR) {
        NETMSG("Connection error with user %i %s", id, netstate.users[id].name);
    }
    else if (reason == NETDROP_MANUAL) {
        NETMSG("Dropped user %i %s", id, netstate.users[id].name);
    }


    if (netstate.my_id == SERVER_ID) {
        memset(&netstate.users[id], 0, sizeof(netstate.users[id]));
        netstate.users[id].id = id; //repair effect by LbMemorySet

        for (i = 0; i < MAX_N_USERS; ++i) {
            if (i == netstate.my_id) {
                continue;
            }

            SendUserUpdate(i, id);
        }

        //set up the stuff the other parts of the game expect
        //TODO NET try to get rid of this because it makes understanding code much more complicated
        localPlayerInfoPtr[id].active = 0;
        memset(localPlayerInfoPtr[id].name, 0, sizeof(localPlayerInfoPtr[id].name));
    }
    else {
        NETMSG("Quitting after connection loss");
        LbNetwork_Stop();
    }
}

static void ProcessMessagesUntilNextFrame(NetUserId id, void *serv_buf, size_t frame_size, unsigned timeout)
{
    /*TbClockMSec start;
    start = LbTimerClock();*/

    //read all messages up to next frame
    while (timeout == 0 || netstate.sp->msgready(id,
            timeout /*- (min(LbTimerClock() - start, max(timeout - 1, 0)))*/) != 0)
    {
        if (ProcessMessage(id, serv_buf, frame_size) == Lb_FAIL)
        {
            break;
        }

        if (    netstate.msg_buffer[0] == NETMSG_FRAME ||
                netstate.msg_buffer[0] == NETMSG_RESYNC) {
            break;
        }

        /*if (LbTimerClock() - start > timeout) {
            break;
        }*/
    }
}

static void ProcessMessagesUntilNextLoginReply(TbClockMSec timeout, void *server_buf, size_t client_frame_size)
{
    TbClockMSec start;
    start = LbTimerClock();

    //read all messages up to next frame
    while (timeout == 0 || netstate.sp->msgready(SERVER_ID,
            timeout - (min(LbTimerClock() - start, max(timeout - 1, 0l)))) != 0)
    {
        if (ProcessMessage(SERVER_ID, server_buf, client_frame_size) == Lb_FAIL)
        {
            break;
        }

        if (netstate.msg_buffer[0] == NETMSG_LOGIN) {
            break;
        }

        if (LbTimerClock() - start > timeout)
        {
            break;
        }
    }
}

static void ConsumeServerFrame(void *server_buf, int frame_size)
{
    NetFrame * frame;

    frame = netstate.exchg_queue;
    NETDBG(8, "Consuming Server frame %d of size %u", frame->seq_nbr, frame->size);

    netstate.exchg_queue = frame->next;
    netstate.seq_nbr = frame->seq_nbr;
    memcpy(server_buf, frame->buffer, frame->size);
    free(frame->buffer);
    free(frame);
}

/*
 * Exchange assuming we are at server side
 */
TbError LbNetwork_ExchangeServer(void *server_buf, size_t client_frame_size)
{
    //server needs to be careful about how it reads messages
    for (NetUserId id = 0; id < MAX_N_USERS; ++id)
    {
        if (id == netstate.my_id) {
            continue;
        }

        if (netstate.users[id].progress == USER_UNUSED) {
            continue;
        }

        if (netstate.users[id].progress == USER_LOGGEDIN)
        {
            //if (netstate.seq_nbr >= SCHEDULED_LAG_IN_FRAMES) { //scheduled lag in TCP stream
                //TODO NET take time to detect a lagger which can then be announced
                ProcessMessagesUntilNextFrame(id, server_buf, client_frame_size, WAIT_FOR_CLIENT_TIMEOUT_IN_MS);
            //}

            netstate.seq_nbr += 1;
            SendServerFrame(server_buf, client_frame_size, CountLoggedInClients() + 1);
        }
        else
        {
            ProcessMessagesUntilNextFrame(id, server_buf, client_frame_size, WAIT_FOR_CLIENT_TIMEOUT_IN_MS);
            netstate.seq_nbr += 1;
            SendServerFrame(server_buf, client_frame_size, CountLoggedInClients() + 1);
        }
    }
    //TODO NET deal with case where no new frame is available and game should be stalled
    netstate.sp->update(OnNewUser);

    assert(UserIdentifiersValid());

    return Lb_OK;
}

TbError LbNetwork_ExchangeClient(void *send_buf, void *server_buf, size_t client_frame_size)
{
    SendClientFrame((char *) send_buf, client_frame_size, netstate.seq_nbr);
    ProcessMessagesUntilNextFrame(SERVER_ID, server_buf, client_frame_size, 0);

    if (netstate.exchg_queue == NULL)
    {
        //connection lost
        return Lb_FAIL;
    }

    // most likely overwrites what was sent in SendClientFrame
    ConsumeServerFrame(server_buf, client_frame_size);

    //TODO NET deal with case where no new frame is available and game should be stalled
    netstate.sp->update(OnNewUser);

    if (!UserIdentifiersValid())
    {
        fprintf(stderr, "Bad network peer state\n");
        return Lb_FAIL;
    }
    return Lb_OK;
}

/*
 * send_buf is a buffer inside shared buffer which sent to a server
 * server_buf is a buffer shared between all clients and server
 */
TbError LbNetwork_Exchange(void *send_buf, void *server_buf, size_t client_frame_size)
{
    NETDBG(7, "Starting");

    assert(UserIdentifiersValid());

    if (netstate.users[netstate.my_id].progress == USER_SERVER)
    {
        return LbNetwork_ExchangeServer(server_buf, client_frame_size);
    }
    else
    { // client
        return LbNetwork_ExchangeClient(send_buf, server_buf, client_frame_size);
    }
}

static const size_t CHUNK_SIZE = 1024 * 32; // Size of each data chunk in bytes (32KB) for both sending and receiving
static const int RECEIVE_CHUNKS_PER_LOOP = 10; // Chunks to process per receive loop iteration
static const int RECEIVE_TIMEOUT_ATTEMPTS = 10; // Number of timeout attempts before giving up on receiving resync
static const int RECEIVE_CHUNK_TIMEOUT_MS = 1000; // Network timeout in milliseconds for waiting for incoming chunks

struct ResyncHeader {
    char message_type; // Message type identifier (NETMSG_RESYNC)
    size_t chunk_index; // Index of this chunk (0-based)
    size_t chunk_count; // Total number of chunks in the complete compressed data transmission
    size_t chunk_length; // Length of compressed data in this specific chunk
    size_t original_total_length; // Original uncompressed data length for verification
};

static size_t calculate_chunk_count(size_t len)
{
    return (len + CHUNK_SIZE - 1) / CHUNK_SIZE;
}

static void draw_resync_progress(size_t bytes_processed, size_t total_bytes)
{
    if (game.play_gameturn == 0) { // Prevent resync bar from drawing on loading screen
        return;
    }
    if (total_bytes == 0) {
        return;
    }
    const long max_progress = (long)(32 * units_per_pixel / 16);
    const long progress_pixels = (long)((double)max_progress * bytes_processed / total_bytes);
    draw_out_of_sync_box(progress_pixels, max_progress, get_my_player()->engine_window_x);
}

static TbBool send_chunked_resync_data(const void * buffer, size_t total_length)
{
    // Compress the entire buffer first
    uLongf compressed_size = compressBound(total_length);
    char * compressed_buffer = (char *) malloc(compressed_size);
    if (!compressed_buffer) {
        return false;
    }
    
    int compress_result = compress((Bytef*)compressed_buffer, &compressed_size, (const Bytef*)buffer, total_length);
    if (compress_result != Z_OK) {
        free(compressed_buffer);
        return false;
    }
    
    // Now chunk the compressed data
    const size_t total_chunk_count = calculate_chunk_count(compressed_size);
    char * message_buffer = (char *) malloc(sizeof(ResyncHeader) + CHUNK_SIZE);
    if (!message_buffer) {
        free(compressed_buffer);
        return false;
    }
    
    for (size_t chunk_index = 0; chunk_index < total_chunk_count; chunk_index++) {
        size_t chunk_start_offset = chunk_index * CHUNK_SIZE;
        size_t remaining_bytes = compressed_size - chunk_start_offset;
        size_t current_chunk_length = min(remaining_bytes, CHUNK_SIZE);
        
        ResyncHeader * message_header = (ResyncHeader*)message_buffer;
        message_header->message_type = NETMSG_RESYNC;
        message_header->chunk_index = chunk_index;
        message_header->chunk_count = total_chunk_count;
        message_header->chunk_length = current_chunk_length;
        message_header->original_total_length = total_length;
        
        memcpy(message_buffer + sizeof(ResyncHeader), compressed_buffer + chunk_start_offset, current_chunk_length);
        
        for (size_t user_index = 0; user_index < MAX_N_USERS; ++user_index) {
            if (netstate.users[user_index].progress != USER_LOGGEDIN) {
                continue;
            }
            netstate.sp->sendmsg_single(netstate.users[user_index].id, message_buffer, sizeof(ResyncHeader) + current_chunk_length);
        }
        
        // Progress based on compressed data sent
        size_t bytes_sent_compressed = chunk_start_offset + current_chunk_length;
        draw_resync_progress(bytes_sent_compressed, compressed_size);
    }
    
    free(compressed_buffer);
    free(message_buffer);
    return true;
}

static void cleanup_resync_buffers(char ** chunk_data_array, size_t total_chunk_count, TbBool * chunk_received_flags, size_t * chunk_lengths, char * message_buffer)
{
    if (chunk_data_array) {
        for (size_t chunk_index = 0; chunk_index < total_chunk_count; chunk_index++) {
            free(chunk_data_array[chunk_index]);
        }
        free(chunk_data_array);
    }
    free(chunk_received_flags);
    free(chunk_lengths);
    free(message_buffer);
}

static TbBool receive_chunked_resync_data(void * destination_buffer, size_t expected_total_length)
{
    // We don't know the compressed size yet, so we need to get it from the first chunk
    size_t expected_chunk_count = 0;
    TbBool first_chunk_received = false;
    
    char ** chunk_data_array = NULL;
    TbBool * chunk_received_flags = NULL;
    size_t * chunk_lengths = NULL;
    char * message_buffer = (char *) malloc(sizeof(ResyncHeader) + CHUNK_SIZE);
    
    if (!message_buffer) {
        return false;
    }
    
    size_t chunks_received_count = 0;
    int timeout_attempt_count = 0;
    size_t total_compressed_bytes_received = 0;
    
    while (!first_chunk_received || chunks_received_count < expected_chunk_count) {
        // Process multiple messages per iteration if available
        int messages_processed_this_iteration = 0;
        
        while (messages_processed_this_iteration < RECEIVE_CHUNKS_PER_LOOP &&
               (!first_chunk_received || chunks_received_count < expected_chunk_count)) {
            size_t received_message_size = netstate.sp->readmsg(SERVER_ID, message_buffer, sizeof(ResyncHeader) + CHUNK_SIZE);
            if (received_message_size == 0) {
                break; // No more messages available
            }
            if (received_message_size < sizeof(ResyncHeader)) {
                continue; // Skip invalid messages
            }
            messages_processed_this_iteration++;
            
            ResyncHeader * message_header = (ResyncHeader*)message_buffer;
            if (message_header->message_type != NETMSG_RESYNC ||
                message_header->chunk_length == 0 ||
                message_header->chunk_length > CHUNK_SIZE ||
                message_header->chunk_length + sizeof(ResyncHeader) != received_message_size ||
                message_header->original_total_length != expected_total_length) {
                continue;
            }
            
            // Initialize arrays on first valid chunk
            if (!first_chunk_received) {
                expected_chunk_count = message_header->chunk_count;
                
                chunk_data_array = (char **) calloc(expected_chunk_count, sizeof(char*));
                chunk_received_flags = (TbBool *) calloc(expected_chunk_count, sizeof(TbBool));
                chunk_lengths = (size_t *) calloc(expected_chunk_count, sizeof(size_t));
                
                if (!chunk_data_array || !chunk_received_flags || !chunk_lengths) {
                    cleanup_resync_buffers(chunk_data_array, expected_chunk_count, chunk_received_flags, chunk_lengths, message_buffer);
                    return false;
                }
                first_chunk_received = true;
            }
            
            if (message_header->chunk_index >= expected_chunk_count ||
                chunk_received_flags[message_header->chunk_index] ||
                message_header->chunk_count != expected_chunk_count) {
                continue;
            }
            
            chunk_data_array[message_header->chunk_index] = (char *) malloc(message_header->chunk_length);
            if (!chunk_data_array[message_header->chunk_index]) {
                cleanup_resync_buffers(chunk_data_array, expected_chunk_count, chunk_received_flags, chunk_lengths, message_buffer);
                return false;
            }
            
            memcpy(chunk_data_array[message_header->chunk_index], message_buffer + sizeof(ResyncHeader), message_header->chunk_length);
            chunk_received_flags[message_header->chunk_index] = true;
            chunk_lengths[message_header->chunk_index] = message_header->chunk_length;
            chunks_received_count++;
            total_compressed_bytes_received += message_header->chunk_length;
            timeout_attempt_count = 0;
        }
        
        // Update progress after processing messages
        if (messages_processed_this_iteration > 0 && first_chunk_received && chunks_received_count > 0) {
            size_t estimated_total_compressed_size = (total_compressed_bytes_received * expected_chunk_count) / chunks_received_count;
            draw_resync_progress(total_compressed_bytes_received, estimated_total_compressed_size);
        }
        
        // Only check timeout if no messages were processed
        if (messages_processed_this_iteration == 0) {
            if (netstate.sp->msgready(SERVER_ID, RECEIVE_CHUNK_TIMEOUT_MS) == 0) {
                timeout_attempt_count++;
                if (timeout_attempt_count >= RECEIVE_TIMEOUT_ATTEMPTS) {
                    NETLOG("Timeout waiting for resync chunks after %d attempts", timeout_attempt_count);
                    cleanup_resync_buffers(chunk_data_array, expected_chunk_count, chunk_received_flags, chunk_lengths, message_buffer);
                    return false;
                }
            }
        }
    }
    
    // Reassemble compressed data
    char * compressed_buffer = (char *) malloc(total_compressed_bytes_received);
    if (!compressed_buffer) {
        cleanup_resync_buffers(chunk_data_array, expected_chunk_count, chunk_received_flags, chunk_lengths, message_buffer);
        return false;
    }

    size_t reassembly_offset = 0;
    for (size_t chunk_index = 0; chunk_index < expected_chunk_count; chunk_index++) {
        if (!chunk_received_flags[chunk_index] || !chunk_data_array[chunk_index]) {
            free(compressed_buffer);
            cleanup_resync_buffers(chunk_data_array, expected_chunk_count, chunk_received_flags, chunk_lengths, message_buffer);
            return false;
        }
        if (reassembly_offset + chunk_lengths[chunk_index] > total_compressed_bytes_received) {
            free(compressed_buffer);
            cleanup_resync_buffers(chunk_data_array, expected_chunk_count, chunk_received_flags, chunk_lengths, message_buffer);
            return false;
        }
        memcpy(compressed_buffer + reassembly_offset, chunk_data_array[chunk_index], chunk_lengths[chunk_index]);
        reassembly_offset += chunk_lengths[chunk_index];
    }
    
    // Now decompress the reassembled data
    uLongf dest_len = expected_total_length;
    int uncompress_result = uncompress((Bytef*)destination_buffer, &dest_len, (const Bytef*)compressed_buffer, total_compressed_bytes_received);
    
    free(compressed_buffer);
    cleanup_resync_buffers(chunk_data_array, expected_chunk_count, chunk_received_flags, chunk_lengths, message_buffer);
    
    if (uncompress_result != Z_OK || dest_len != expected_total_length) {
        NETLOG("Decompression failed: zlib error %d, expected %lu bytes, got %lu bytes", 
               uncompress_result, (unsigned long)expected_total_length, (unsigned long)dest_len);
        return false;
    }
    
    return true;
}

TbBool LbNetwork_Resync(void * data_buffer, size_t buffer_length)
{
    NETLOG("Starting");

    if (netstate.users[netstate.my_id].progress == USER_SERVER) {
        return send_chunked_resync_data(data_buffer, buffer_length);
    }
    return receive_chunked_resync_data(data_buffer, buffer_length);
}

TbError LbNetwork_EnableNewPlayers(TbBool allow)
{
    int i;

    if (!netstate.locked && !allow) {
        //throw out partially connected players
        for (i = 0; i < MAX_N_USERS; ++i) {
            if (netstate.users[i].progress == USER_CONNECTED) {
                netstate.sp->drop_user(netstate.users[i].id);
            }
        }
    }

    netstate.locked = !allow;

    if (netstate.locked) {
        NETMSG("New players are NOT allowed to join");
    }
    else {
        NETMSG("New players ARE allowed to join");
    }

    return Lb_OK;
}

TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *ptr)
{
  struct TbNetworkCallbackData netcdat = {};

  SYNCDBG(7, "Starting");
  strcpy(netcdat.svc_name, "TCP");
  callback(&netcdat, ptr);
  strcpy(netcdat.svc_name, "ENET/UDP");
  callback(&netcdat, ptr);
  NETMSG("Enumerate Services called");
  return Lb_OK;
}

/*
 * Enumerates network players.
 * @return Returns Lb_OK on success, Lb_FAIL on error.
 */
TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *buf)
{
    TbNetworkCallbackData data = {};
    NetUserId id;

    SYNCDBG(9, "Starting");

    //for now assume this our session.

    for (id = 0; id < MAX_N_USERS; ++id) {
        if (netstate.users[id].progress != USER_UNUSED &&
                netstate.users[id].progress != USER_CONNECTED) { //no point in showing user if there's no name
            memset(&data, 0, sizeof(data));
            snprintf(data.plyr_name, sizeof(data.plyr_name), "%s", netstate.users[id].name);
            callback(&data, buf);
        }
    }

    return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr)
{
    unsigned i;

    SYNCDBG(9, "Starting");

    for (i = 0; i < SESSION_COUNT; ++i) {
        if (!sessions[i].in_use) {
            continue;
        }

        callback((TbNetworkCallbackData *) &sessions[i], ptr);
    }

    return Lb_OK;
}

unsigned long get_host_player_id(void)
{
  return hostId;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
