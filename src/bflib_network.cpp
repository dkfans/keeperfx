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
#include "bflib_memory.h"
#include "bflib_netsession.h"
#include "bflib_netsp.hpp"
#include "bflib_netsp_ipx.hpp"
#include "bflib_sound.h"
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
// Local functions definition
TbError AddAPlayer(struct TbNetworkPlayerNameEntry *plyrname);
void *MultiPlayerCallback(unsigned long a1, unsigned long a2, unsigned long a3, void *a4);
void MultiPlayerReqExDataMsgCallback(unsigned long a1, unsigned long a2, void *a3);
void AddMsgCallback(unsigned long, char *, void *);
void DeleteMsgCallback(unsigned long, void *);
void HostMsgCallback(unsigned long, void *);
void RequestCompositeExchangeDataMsgCallback(unsigned long, unsigned long, void *);
void *UnidirectionalMsgCallback(unsigned long, unsigned long, void *);
void SystemUserMsgCallback(unsigned long, void *, unsigned long, void *);
static void OnDroppedUser(NetUserId id, enum NetDropReason reason);
/******************************************************************************/
struct ReceiveCallbacks receiveCallbacks = {
  AddMsgCallback,
  DeleteMsgCallback,
  HostMsgCallback,
  NULL,
  MultiPlayerCallback,
  MultiPlayerReqExDataMsgCallback,
  RequestCompositeExchangeDataMsgCallback,
  UnidirectionalMsgCallback,
  SystemUserMsgCallback,
  NULL,
};

unsigned long inside_sr;
void *localDataPtr;
void *compositeBuffer;
unsigned long waitingForPlayerMapResponse;
unsigned long maximumPlayers;
unsigned long localPlayerId;
void *exchangeBuffer;
unsigned long exchangeSize;
unsigned long sequenceNumber;
unsigned long hostId;
struct ClientDataEntry clientDataTable[CLIENT_TABLE_LEN];
unsigned char systemUserBuffer[1028];
struct UnidirectionalDataMessage incomingUnidirectionalMessage;
/******************************************************************************/

// New network code declarations start here ===================================

/**
 * Max wait for a client before we declare client messed up.
 */
#define WAIT_FOR_CLIENT_TIMEOUT_IN_MS   10000
#define WAIT_FOR_SERVER_TIMEOUT_IN_MS   WAIT_FOR_CLIENT_TIMEOUT_IN_MS

/**
 * If queued frames on client exceed > SCHEDULED_LAG_IN_FRAMES/2 game speed should
 * be faster, if queued frames < SCHEDULED_LAG_IN_FRAMES/2 game speed should be slower.
 * Server also expects there to be SCHEDULED_LAG_IN_FRAMES in TCP stream.
 */
#define SCHEDULED_LAG_IN_FRAMES 12

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
	TbBool                  is_new;
};

struct NetFrame
{
    struct NetFrame *       next;
    char *                  buffer;
    int                     seq_nbr;
    size_t                  size;
};

// sizeof(kind)
#define PACKET_HEAD_SZ 1
struct NetPacket
{
    struct NetPacket        *next;
    uint16_t                size; // From ENET

    uint8_t                 kind;
    // uint16_t                turn; //TODO
    uint8_t                 data[];
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
    int                     login_attempts;

    struct NetPacket        *outpacket, *outpacket_tail; // List of outgoing packets (in order)
    struct NetPacket        *midpacket; // List of packets to move to receiving queue

    TbBool                  locked;             //if set, no players may join
    char                    input_buffer[260];
    char                    input_buffer_null;    //theoretical safe guard vs non-terminated strings

    char                    msg_buffer[(sizeof(NetFrame) + sizeof(struct Packet)) * PACKETS_COUNT + 1]; //completely estimated for now
    char                    msg_buffer_null;    //theoretical safe guard vs non-terminated strings
};

struct NetCallback
{
    void *context;
    LbNetwork_Packet_Callback callback;
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

static int SendLoginRequest(const char * name, const char * password)
{
    char msg_buffer[256];
    char *buffer_ptr;

    NETMSG("Logging in as %s", name);

    if (strlen(msg_buffer) + strlen(password) + 4 > sizeof(msg_buffer))
    {
        NETMSG("Name and/or Password are too big\n");
        return -1;
    }

    buffer_ptr = msg_buffer;
    *buffer_ptr = PckA_Login;
    buffer_ptr += 1;

    strcpy(buffer_ptr, password);
    buffer_ptr += LbStringLength(password) + 1;

    strcpy(buffer_ptr, name); //don't want to bother saving length ahead
    buffer_ptr += LbStringLength(name) + 1;

    netstate.sp->sendmsg_single(SERVER_ID, msg_buffer, buffer_ptr - msg_buffer);
    return 0;
}

static void SendUserUpdate(NetUserId dest, NetUserId updated_user)
{
    char * ptr;

    ptr = netstate.msg_buffer;

    *ptr = PckA_UserUpdate;
    ptr += 1;

    *ptr = updated_user;
    ptr += 1;

    *ptr = netstate.users[updated_user].progress;
    ptr += 1;

    LbStringCopy(ptr, netstate.users[updated_user].name,
        sizeof(netstate.users[updated_user].name));
    ptr += LbStringLength(netstate.users[updated_user].name) + 1;

    netstate.sp->sendmsg_single(dest, netstate.msg_buffer,
        ptr - netstate.msg_buffer);
}

static void HandleLoginRequest(NetUserId source, char * ptr, const char *end)
{
    size_t len;
    NetUserId id;

    NETDBG(7, "Starting");

    if (netstate.locked)
    {
        ERRORLOG("Netstate is locked. New player discarded");
        netstate.sp->drop_user(source);
        return;
    }

    if (netstate.users[source].progress != USER_CONNECTED) {
        NETMSG("Peer was not in connected state");
        //TODO NET implement drop
        netstate.sp->drop_user(source);
        return;
    }

    if (netstate.password[0] != 0 && strncmp(ptr, netstate.password,
            sizeof(netstate.password)) != 0)
    {
        //TODO: Should we send Password failed?
        NETMSG("Peer chose wrong password");
        //TODO NET implement drop
        netstate.sp->drop_user(source);
        return;
    }
    len = LbStringLength(ptr) + 1;
    ptr += len;
    if (ptr > end)
    {
        ERRORLOG("Invalid packet");
        netstate.sp->drop_user(source);
        return;
    }
    if (len > sizeof(netstate.password))
    {
        NETDBG(6, "Connected peer attempted to flood password");
        netstate.sp->drop_user(source);
        return;
    }

    if (!isalnum(ptr[0]))
    {
        //TODO NET drop player for bad name
        //also replace isalnum with something that considers foreign non-ASCII chars
        NETDBG(6, "Connected peer had bad name starting with %c",
            netstate.users[source].name[0]);
        netstate.sp->drop_user(source);
        return;
    }

    LbStringCopy(netstate.users[source].name, ptr, sizeof(netstate.users[source].name));
    //presume login successful from here
    NETMSG("User %s successfully logged in", netstate.users[source].name);
    netstate.users[source].progress = USER_LOGGEDIN;
    play_non_3d_sample(76);

    //send reply
    char reply_packet[2];
    reply_packet[0] = PckA_Login;
    LbMemoryCopy(reply_packet + 1, &source, 1); //assumes LE
    netstate.sp->sendmsg_single(source, reply_packet, sizeof(reply_packet));

    //send user updates
    for (id = 0; id < MAX_N_USERS; ++id)
    {
        if (netstate.users[id].progress == USER_UNUSED)
        {
            continue;
        }

        SendUserUpdate(source, id);

        if (id == netstate.my_id || id == source)
        {
            continue;
        }

        SendUserUpdate(id, source);
    }

    //set up the stuff the other parts of the game expect
}

static void HandleUserUpdate(NetUserId source, char * ptr, const char * end)
{
    NetUserId id;
    NETDBG(7, "Starting");
    if (source != SERVER_ID)
    {
        NETLOG("Error: UserUpdate not from server! Source:%d", source);
        return;
    }
    if (end - ptr < 3)
    {
        NETLOG("Error: Invalid UserUpdate len:%d", end - ptr);
        return;
    }
    id = (uint8_t) *ptr;
    if (id < 0 && id >= MAX_N_USERS)
    {
        NETLOG("Error: Out of range user ID %i received from server", id);
        return;
    }
    ptr += 1;

    netstate.users[id].progress = (enum NetUserProgress) *ptr;
    ptr += 1;

    LbStringCopy(netstate.users[id].name, ptr, min((int)sizeof(netstate.users[id].name), end - ptr));
}

static void HandleServerFrame(char *ptr, const char *end, size_t user_frame_size)
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

    frame = (NetFrame *) LbMemoryAlloc(sizeof(*frame));
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
    frame->buffer = (char *) LbMemoryAlloc(frame->size);
    frame->seq_nbr = seq_nbr;

    LbMemoryCopy(frame->buffer, ptr, frame->size);

    NETDBG(9, "Handled server frame of %u bytes", frame->size);
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

  //clear network object and init it to neutral config
  LbMemorySet(&netstate, 0, sizeof(netstate));
  for (usr = 0; usr < MAX_N_USERS; ++usr) {
      netstate.users[usr].id = usr;
  }

  netstate.max_players = maxplayrs;

  // Initialising the service provider object
  switch (srvcindex)
  {
  case NS_TCP_IP:
      NETMSG("Selecting TCP/IP SP");
      /*if (GenericTCPInit(init_data) == Lb_OK) {
          res = Lb_OK;
      }
      else {
          WARNLOG("Failure on TCP/IP Initialization");
          res = Lb_FAIL;
      }*/

      netstate.sp = &tcpSP;

      break;
  case NS_ENET_UDP:
      netstate.sp = InitEnetSP();
      NETMSG("Selecting UDP");
      break;
  default:
      WARNLOG("The serviceIndex value of %d is out of range", srvcindex);
      res = Lb_FAIL;
      break;
  }

  if (netstate.sp) {
      res = netstate.sp->init(OnDroppedUser); //TODO NET supply drop callback
  }

  return res;
}

static TbBool UpdateCallbackNull(NetUserId * assigned_id)
{
    ERRORLOG("Unexpected login request");
    return false;
}

static TbBool ProcessMessagesUntilNextLoginReply(TbClockMSec timeout)
{
    char data_buf[32];
    size_t data_size = sizeof(data_buf);
    TbClockMSec start;
    start = LbTimerClock();
    netstate.sp->update(&UpdateCallbackNull);
    //read all messages up to next frame
    while (timeout == 0 || netstate.sp->msgready(timeout - (min(LbTimerClock() - start, max(timeout - 1, 0l)))) != 0)
    {
        NetUserId user_id = -1;
        size_t rcount = netstate.sp->readmsg(&user_id, data_buf, data_size);
        assert (user_id == SERVER_ID);

        if (rcount > 0)
        {
            uint8_t kind;

            //type
            kind = *(uint8_t *)data_buf;

            if (kind == PckA_Login)
            {
                // "Handle login response" here
                assert (rcount == sizeof(uint8_t) + sizeof(uint8_t));
                netstate.my_id = (uint8_t)data_buf[1];
                return true;
            }
        }

        if (LbTimerClock() - start > timeout)
        {
            break;
        }
        netstate.sp->update(&UpdateCallbackNull);
    }
    return false;
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
    if (!ProcessMessagesUntilNextLoginReply(WAIT_FOR_SERVER_TIMEOUT_IN_MS))
    {
        fprintf(stderr, "Network login rejected\n");
        return Lb_FAIL;
    }
    if (netstate.my_id == 23456) {
        fprintf(stderr, "Network login unsuccessful\n");
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

    if (netstate.sp->host(":5555", optns) == Lb_FAIL) {
        return Lb_FAIL;
    }

    netstate.my_id = SERVER_ID;
    LbStringCopy(netstate.users[netstate.my_id].name, plyr_name,
        sizeof(netstate.users[netstate.my_id].name));
    netstate.users[netstate.my_id].progress = USER_SERVER;

    *plyr_num = netstate.my_id;

    LbNetwork_EnableNewPlayers(true);
    return Lb_OK;
}

TbBool LbNetwork_IsServer()
{
    return netstate.my_id == SERVER_ID;
}

void LbNetwork_ChangeExchangeTimeout(unsigned long tmout)
{
}

TbError LbNetwork_Stop(void)
{
    if (netstate.sp)
    {
        netstate.sp->exit();
    }

    for (NetFrame* frame = netstate.exchg_queue;frame != NULL;)
    {
        NetFrame *nextframe = frame->next;
        LbMemoryFree(frame->buffer);
        LbMemoryFree(frame);
        frame = nextframe;
    }

    for (auto outframe = netstate.outpacket; outframe != nullptr; )
    {
        auto nextframe = outframe->next;
        free(outframe);
        outframe = nextframe;
    }

    LbMemorySet(&netstate, 0, sizeof(netstate));

    return Lb_OK;
}

static TbBool OnNewUser(NetUserId * assigned_id)
{
    NetUserId i;

    if (netstate.locked) {
        return 0;
    }

    for (i = 0; i < MAX_N_USERS; ++i)
    {
        if (netstate.users[i].progress == USER_UNUSED) {
            *assigned_id = i;
            netstate.users[i].progress = USER_CONNECTED;
            netstate.users[i].is_new = 1;
            netstate.login_attempts++;
            NETLOG("Assigning new user to ID %u", i);
            return 1;
        }
    }

    return 0;
}

static void CleanupUser(NetUserId id)
{
    //set up the stuff the other parts of the game expect
    LbMemorySet(&netstate.users[id], 0, sizeof(netstate.users[id]));
    netstate.users[id].id = id; //repair effect by LbMemorySet
}

static void OnDroppedUser(NetUserId id, enum NetDropReason reason)
{
    int i;

    assert(id >= 0);
    assert(id < MAX_N_USERS);

    if (netstate.my_id == id)
    {
        NETMSG("Warning: Trying to drop local user. There's a bug in code somewhere, probably server trying to send message to itself.");
        return;
    }

    //return;
    if (reason == NETDROP_ERROR)
    {
        NETMSG("Connection error with user %i %s", id, netstate.users[id].name);
    }
    else if (reason == NETDROP_MANUAL)
    {
        NETMSG("Dropped user %i %s", id, netstate.users[id].name);
    }


    if (LbNetwork_IsServer())
    {
        CleanupUser(id);

        for (i = 0; i < MAX_N_USERS; ++i)
        {
            if (i == netstate.my_id)
            {
                continue;
            }

            SendUserUpdate(i, id);
        }
    }
    else
    {
        NETMSG("Quitting after connection loss");
        // TODO: Callback
        LbNetwork_Stop();
    }
}

static void ConsumeServerFrame(void *server_buf, int frame_size)
{
    NetFrame * frame;

    frame = netstate.exchg_queue;
    NETDBG(8, "Consuming Server frame %d of size %u", frame->seq_nbr, frame->size);

    netstate.exchg_queue = frame->next;
    netstate.seq_nbr = frame->seq_nbr;
    LbMemoryCopy(server_buf, frame->buffer, frame->size);
    LbMemoryFree(frame->buffer);
    LbMemoryFree(frame);
}

void *LbNetwork_AddPacket_f(unsigned char kind, unsigned long turn, short size, const char* func)
{
    // TODO optimize somehow
    auto ret = static_cast<NetPacket *>(malloc(sizeof(struct NetPacket) + size));
    if (netstate.outpacket)
    {
        netstate.outpacket_tail->next = ret;
        netstate.outpacket_tail = ret;
    }
    else
    {
        netstate.outpacket = ret;
        netstate.outpacket_tail = ret;
    }
    ret->next = nullptr;
    ret->kind = kind;
    ret->size = size;
    return &ret->data[0];
}

static void SendFrames()
{
    //SendClientFrame((char *) send_buf, client_frame_size, netstate.seq_nbr);
    //SendServerFrame(server_buf, client_frame_size, CountLoggedInClients() + 1);
    for (struct NetPacket *packet = netstate.outpacket; packet != nullptr;)
    {
        netstate.sp->sendmsg_all(((const char *)packet) + offsetof(struct NetPacket, kind), packet->size + PACKET_HEAD_SZ);
        packet = packet->next;
    }
    netstate.midpacket = netstate.outpacket;
    netstate.outpacket = nullptr;
    netstate.outpacket_tail = nullptr;
}
/*
 * send_buf is a buffer inside shared buffer which sent to a server
 * server_buf is a buffer shared between all clients and server
 */
TbError LbNetwork_Exchange(void *context, LbNetwork_Packet_Callback callback)
{
    NETDBG(7, "Starting");

    if (!UserIdentifiersValid())
    {
        ERRORLOG("!UserIdentifiersValid()");
        return Lb_FAIL;
    }

    SendFrames();
    auto midpacket = netstate.midpacket;
    while (true)
    {
        //  TODO NET: Now we are steplocked
        short rcount = 0;
        NetUserId source = -1;
        if (netstate.sp->msgready(0))
        {
            rcount = netstate.sp->readmsg(&source, netstate.input_buffer, sizeof(netstate.input_buffer));
            assert((source >= 0) && (source < MAX_N_USERS));
        }
        else if (midpacket)
        {
            auto curpacket = midpacket;
            midpacket = midpacket->next;
            source = netstate.my_id;
            rcount = min(sizeof(netstate.input_buffer), (size_t)curpacket->size + PACKET_HEAD_SZ);
            memcpy(netstate.input_buffer, &curpacket->kind, // Kind + all data
                    rcount);
            free(curpacket);
        }

        if (rcount > 0)
        {
            /*if (source == SERVER_ID)
            {
                HandleMessageFromServer(source, frame_size);
            }
            else
            {
                HandleMessageFromClient(source, server_buf, frame_size);
            }*/
            int turn = 0;
            uint8_t kind = netstate.input_buffer[0];
            switch (kind)
            {
            case PckA_Login:
                HandleLoginRequest(source, netstate.input_buffer + 1, netstate.input_buffer + rcount);
                break;
            case PckA_UserUpdate:
                HandleUserUpdate(source, netstate.input_buffer + 1, netstate.input_buffer + rcount);
                break;
            default:
                callback(context, turn,
                         source, // NetPlayerIdx? or Player?
                         kind,
                         netstate.input_buffer + PACKET_HEAD_SZ,
                         (short) (rcount - PACKET_HEAD_SZ)
                );
            }
        }
        else
        {
            // No message
            break;
        }
    } // while (true)
    netstate.midpacket = nullptr;

    // most likely overwrites what was sent in SendClientFrame
    //ConsumeServerFrame(server_buf, client_frame_size);

    netstate.sp->update(OnNewUser);

    if (!UserIdentifiersValid())
    {
        fprintf(stderr, "Bad network peer state\n");
        return Lb_FAIL;
    }
    return Lb_OK;
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
  callback(&netcdat, 1);
  strcpy(netcdat.svc_name, "ENET/UDP");
  callback(&netcdat, 1);
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

  /*char ret;
  if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  ret = spPtr->Enumerate(sesn, callback, buf);
  if (ret != Lb_OK)
  {
    WARNLOG("Failure on Enumerate");
    return ret;
  }*/

    //for now assume this our session.

    for (id = 0; id < MAX_N_USERS; ++id) {
        if (netstate.users[id].progress != USER_UNUSED &&
                netstate.users[id].progress != USER_CONNECTED) { //no point in showing user if there's no name
            LbMemorySet(&data, 0, sizeof(data));
            LbStringCopy(data.plyr_name, netstate.users[id].name,
                sizeof(data.plyr_name));
            callback(&data, netstate.users[id].is_new);
            netstate.users[id].is_new = false; //TODO: stateful is bad
        }
    }

    return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr)
{
    unsigned i;

    SYNCDBG(9, "Starting");

  //char ret;
  /*if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  ret = spPtr->Enumerate(callback, ptr);
  if (ret != Lb_OK)
  {
    WARNLOG("Failure on Enumerate");
    return ret;
  }*/

    for (i = 0; i < SESSION_COUNT; ++i) {
        if (!sessions[i].in_use) {
            continue;
        }

        callback((TbNetworkCallbackData *) &sessions[i], 1);
    }

    return Lb_OK;
}

TbError AddAPlayer(struct TbNetworkPlayerNameEntry *plyrname)
{
  TbBool found_id;
  unsigned long plr_id;
  long i;
  if (plyrname == NULL)
  {
    return Lb_FAIL;
  }
  plr_id = 0;
  found_id = false;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].isactive) && (clientDataTable[i].plyrid == plyrname->islocal))
    {
      found_id = true;
      plr_id = i;
    }
  }
  if (!found_id)
  {
    found_id = false;
    for (i=0; i < maximumPlayers; i++)
    {
      if (clientDataTable[i].isactive)
      {
        found_id = true;
        plr_id = i;
      }
    }
    if (found_id)
    {
      clientDataTable[plr_id].plyrid = plyrname->islocal;
      clientDataTable[plr_id].isactive = 1;
      strcpy(clientDataTable[plr_id].name,plyrname->name);
    }
  }
  if (!found_id)
  {
    return Lb_FAIL;
  }
  if (plyrname->field_9)
    hostId = plyrname->islocal;
  if (plyrname->ishost)
  {
    localPlayerId = plyrname->islocal;
  }
  return Lb_OK;
}

TbError GenericIPXInit(void *init_data)
{
  if (spPtr != NULL)
  {
    spPtr->Release();
    delete spPtr;
    spPtr = NULL;
  }
  spPtr = new IPXServiceProvider();
  if (spPtr == NULL)
  {
    WARNLOG("Failure on SP construction");
    return Lb_FAIL;
  }
  if (spPtr->Init(&receiveCallbacks, nullptr) != Lb_OK)
  {
    WARNLOG("Failure on SP::Init()");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError SendSystemUserMessage(unsigned long plr_id, int te, void *ibuf, unsigned long ibuf_len)
{
  if (ibuf_len+5 > sizeof(systemUserBuffer))
  {
    WARNLOG("systemUserBuffer is too small");
    return Lb_FAIL;
  }
  spPtr->EncodeMessageStub(systemUserBuffer, ibuf_len+1, 4, 0);
  systemUserBuffer[4] = te;
  if ((ibuf != NULL) && (ibuf_len > 0))
  {
    memcpy(&systemUserBuffer[5], ibuf, ibuf_len);
  }
  return spPtr->Send(plr_id, systemUserBuffer);
}

void PlayerMapMsgHandler(unsigned long plr_id, void *msg, unsigned long msg_len)
{
  unsigned long len;
  len = sizeof(struct ClientDataEntry)*maximumPlayers;
  if (msg_len == 0)
  {
    SendSystemUserMessage(plr_id, 0, clientDataTable, len);
    return;
  }
  if (!waitingForPlayerMapResponse)
  {
    WARNLOG("Received unexpected SU_PLAYER_MAP_MSG");
    return;
  }
  if (msg_len != len)
  {
    WARNLOG("Invalid length, %d",msg_len);
    return;
  }
  LbMemoryCopy(clientDataTable, msg, len);
  waitingForPlayerMapResponse = 0;
}

void *MultiPlayerCallback(unsigned long plr_id, unsigned long xch_size, unsigned long seq, void *a4)
{
  TbBool found_id;
  long i;
  if (inside_sr)
    NETLOG("Got a request");
  if (localPlayerId == hostId)
  {
    if (xch_size != exchangeSize)
    {
      WARNLOG("Invalid length, %d",xch_size);
      return NULL;
    }
    if (plr_id == localPlayerId)
    {
      WARNLOG("host got data from itself");
      return NULL;
    }
    found_id = false;
    for (i=0; i < maximumPlayers; i++)
    {
      if ((clientDataTable[i].isactive) && (clientDataTable[i].plyrid == plr_id))
      {
        found_id = true;
        plr_id = i;
      }
    }
    if (!found_id)
    {
      WARNLOG("Invalid id: %d",plr_id);
      return NULL;
    }
    if ((seq != sequenceNumber) && (seq != 15))
    {
      WARNLOG("Unexpected sequence number: Got %d, expected %d",seq,sequenceNumber);
      return NULL;
    }
    clientDataTable[plr_id].field_8 = 1;
    return (uchar *)exchangeBuffer + plr_id * exchangeSize;
  }
  if (xch_size != maximumPlayers * exchangeSize)
  {
    if (xch_size != exchangeSize)
    {
      WARNLOG("Invalid length: %d",xch_size);
      return NULL;
    }
    if (plr_id == localPlayerId)
    {
      WARNLOG("Client acting as host got data from itself");
      return NULL;
    }
    found_id = false;
    for (i=0; i < maximumPlayers; i++)
    {
      if ((clientDataTable[i].isactive) && (clientDataTable[i].plyrid == plr_id))
      {
        found_id = true;
        plr_id = i;
      }
    }
    if (!found_id)
    {
      WARNLOG("Invalid id: %d",plr_id);
      return NULL;
    }
    clientDataTable[plr_id].field_8 = 1;
    return (uchar *)exchangeBuffer + plr_id * exchangeSize;
  }
  if (hostId != plr_id)
  {
    WARNLOG("Data is not from host");
    return NULL;
  }
  found_id = false;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].isactive) && (clientDataTable[i].plyrid == plr_id))
    {
      found_id = true;
      plr_id = i;
    }
  }
  if (!found_id)
  {
    WARNLOG("Invalid id: %d",plr_id);
    return 0;
  }
  if (sequenceNumber == 15)
  {
    sequenceNumber = seq;
  } else
  if (sequenceNumber != seq)
  {
    WARNLOG("Unexpected sequence number: Got %d, expected %d", seq, sequenceNumber);
    return NULL;
  }
  return exchangeBuffer;
}

void MultiPlayerReqExDataMsgCallback(unsigned long plr_id, unsigned long seq, void *a3)
{
  if (inside_sr)
    NETLOG("Got a request");
  if (localDataPtr == NULL)
  {
    WARNLOG("NULL data pointer");
    return;
  }
  if (sequenceNumber == 15)
    sequenceNumber = seq;
  if (seq != sequenceNumber)
  {
    WARNLOG("Unexpected sequence number, got %d, expected %d",seq,sequenceNumber);
    return;
  }
  spPtr->EncodeMessageStub(localDataPtr, exchangeSize-4, 0, sequenceNumber);
  if (spPtr->Send(plr_id, localDataPtr) != Lb_OK)
  {
    WARNLOG("Failure on SP::Send()");
  }
}

void AddMsgCallback(unsigned long a1, char *nmstr, void *a3)
{
  struct TbNetworkPlayerNameEntry npname;
  npname.islocal = a1;
  strcpy(npname.name,nmstr);
  npname.ishost = 0;
  npname.field_9 = 0;
  AddAPlayer(&npname);
}

void DeleteMsgCallback(unsigned long plr_id, void *a2)
{
  long i;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].isactive) && (clientDataTable[i].plyrid == plr_id))
    {
      clientDataTable[i].isactive = 0;
    }
  }
}

void HostMsgCallback(unsigned long plr_id, void *a2)
{
  hostId = plr_id;
}

void RequestCompositeExchangeDataMsgCallback(unsigned long plr_id, unsigned long seq, void *a3)
{
  if (inside_sr)
    NETLOG("Got sequence %d, expected %d",seq,sequenceNumber);
  if ((seq != sequenceNumber) && (seq != ((sequenceNumber - 1) & 0xF)))
  {
    WARNLOG("Unexpected sequence number, got %d, expected %d",seq,sequenceNumber);
    return;
  }
  if (spPtr->Send(plr_id, compositeBuffer) != Lb_OK)
  {
    WARNLOG("Failure on SP::Send()");
    return;
  }
}

void *UnidirectionalMsgCallback(unsigned long a1, unsigned long msg_len, void *a3)
{
  if (msg_len > 524)
  {
    WARNLOG("Invalid length, %d vs %d", msg_len, 524);
    return NULL;
  }
  return &incomingUnidirectionalMessage;
}

void SystemUserMsgCallback(unsigned long plr_id, void *msgbuf, unsigned long msglen, void *a4)
{
  struct SystemUserMsg *msg;
  msg = (struct SystemUserMsg *)msgbuf;
  if ((msgbuf = NULL) || (msglen <= 0))
    return;
  if (msg->type)
  {
    WARNLOG("Illegal sysMsgType %d",msg->type);
  }
  PlayerMapMsgHandler(plr_id, msg->client_data_table, msglen-1);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
