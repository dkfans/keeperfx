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
#include "bflib_network.h"

#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "bflib_memory.h"
#include "bflib_netsession.h"
#include "bflib_netsp.hpp"
#include "bflib_netsp_ipx.hpp"
#include "bflib_netsp_tcp.hpp"
#include "globals.h"
#include <assert.h>
#include <ctype.h>

//TODO: get rid of the following headers later by refactoring, they're here for testing primarily
#include "frontend.h"
#include "net_game.h"
#include "packets.h"
#include "front_landview.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT TbError _DK_LbNetwork_Exchange(void *buf);
DLLIMPORT TbError _DK_LbNetwork_Startup(void);
DLLIMPORT TbError _DK_LbNetwork_Shutdown(void);
DLLIMPORT TbError _DK_LbNetwork_Stop(void);
DLLIMPORT TbError _DK_LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, unsigned long *plyr_num);
DLLIMPORT TbError _DK_LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num);
DLLIMPORT TbError _DK_LbNetwork_ChangeExchangeBuffer(void *, unsigned long);
DLLIMPORT TbError _DK_LbNetwork_Init(unsigned long,struct _GUID guid, unsigned long, void *, unsigned long, struct TbNetworkPlayerInfo *netplayr, void *);
DLLIMPORT TbError _DK_LbNetwork_EnableNewPlayers(unsigned long allow);
DLLIMPORT TbError _DK_LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *a2);
DLLIMPORT TbError _DK_LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *a2);
DLLIMPORT TbError _DK_LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr);
/******************************************************************************/
// Local functions definition
TbError ClearClientData(void);
TbError GetPlayerInfo(void);
TbError GetCurrentPlayers(void);
TbError AddAPlayer(struct TbNetworkPlayerNameEntry *plyrname);
static TbError GenericSerialInit(void *init_data);
static TbError GenericModemInit(void *init_data);
static TbError GenericIPXInit(void *init_data);
//static TbError GenericTCPInit(void *init_data); // this is not being used
TbError StartTwoPlayerExchange(void *buf);
TbError StartMultiPlayerExchange(void *buf);
TbError CompleteTwoPlayerExchange(void *buf);
TbError CompleteMultiPlayerExchange(void *buf);
TbError HostDataCollection(void);
TbError HostDataBroadcast(void);
void __stdcall GetCurrentPlayersCallback(struct TbNetworkCallbackData *netcdat, void *a2);
void *MultiPlayerCallback(unsigned long a1, unsigned long a2, unsigned long a3, void *a4);
void MultiPlayerReqExDataMsgCallback(unsigned long a1, unsigned long a2, void *a3);
void AddMsgCallback(unsigned long, char *, void *);
void DeleteMsgCallback(unsigned long, void *);
void HostMsgCallback(unsigned long, void *);
void RequestCompositeExchangeDataMsgCallback(unsigned long, unsigned long, void *);
void *UnidirectionalMsgCallback(unsigned long, unsigned long, void *);
void SystemUserMsgCallback(unsigned long, void *, unsigned long, void *);
void TwoPlayerReqExDataMsgCallback(unsigned long, unsigned long, void *);
void *TwoPlayerCallback(unsigned long, unsigned long, unsigned long, void *);
TbError LbNetwork_StartExchange(void *buf);
TbError LbNetwork_CompleteExchange(void *buf);
static void OnDroppedUser(NetUserId id, enum NetDropReason reason);
static void ProcessMessagesUntilNextLoginReply(TbClockMSec timeout);
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
struct TbNetworkPlayerInfo *localPlayerInfoPtr;
unsigned long actualTimeout;
void *localDataPtr;
void *compositeBuffer;
unsigned long basicTimeout;
unsigned long maxTime;
unsigned long startTime;
unsigned long waitingForPlayerMapResponse;
unsigned long compositeBufferSize;
unsigned long maximumPlayers;
unsigned long localPlayerIndex;
unsigned long localPlayerId;
unsigned long gotCompositeData;
void *exchangeBuffer;
unsigned long exchangeSize;
unsigned long sequenceNumber;
unsigned long timeCount;
unsigned long hostId;
unsigned long runningTwoPlayerModel;
struct ClientDataEntry clientDataTable[CLIENT_TABLE_LEN];
unsigned long exchangeTimeout;
unsigned char deletePlayerBuffer[8];
unsigned char requestExchangeDataBuffer[8];
unsigned char requestCompositeExchangeDataBuffer[8];
unsigned char systemUserBuffer[1028];
unsigned char lastMessage[1028];
unsigned char lastButOneMessage[1028];
unsigned long remotePlayerIndex;
unsigned long remotePlayerId;
unsigned long unidirectionalMsgReceived;
struct UnidirectionalDataMessage incomingUnidirectionalMessage;
struct UnidirectionalDataMessage dataMessage;
//struct UnidirectionalHeader endMessage;
//struct UnidirectionalHeader abortMessage;
struct UnidirectionalRTSMessage rtsMessage;
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
	int                     ack; //last sequence number processed
};

struct NetFrame
{
    struct NetFrame *       next;
    char *                  buffer;
    int                     seq_nbr;
    size_t                  size;
};

enum NetMessageType
{
    NETMSG_LOGIN,           //to server: username and pass, from server: assigned id
    NETMSG_USERUPDATE,      //changed player from server
    NETMSG_FRAME,           //to server: ACK of frame + packets, from server: the frame itself
    NETMSG_LAGWARNING,      //from server: notice that some client is lagging¨
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
    size_t                  user_frame_size;    //sizeof(Packet) most probably
    char *                  exchg_buffer;
    TbBool                  enable_lag;         //enable scheduled lag mode in exchange (in the best case this would always be true but other parts of code expects perfect sync for now)
    char                    msg_buffer[(sizeof(NetFrame) + sizeof(struct Packet)) * PACKETS_COUNT + 1]; //completely estimated for now
    char                    msg_buffer_null;    //theoretical safe guard vs non-terminated strings
    TbBool                  locked;             //if set, no players may join
};

//the "new" code contained in this struct
static struct NetState netstate;

//sessions placed here for now, would be smarter to store dynamically
static struct TbNetworkSessionNameEntry sessions[SESSION_COUNT]; //using original because enumerate expects static life time

// New network code data definitions end here =================================

/*
 * The following two functions are not exported from this module.
 *
TbError LbNetwork_Startup(void)
{
  return _DK_LbNetwork_Startup();
}

TbError LbNetwork_Shutdown(void)
{
  return _DK_LbNetwork_Shutdown();
}
*/

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
    buffer_ptr += LbStringLength(password) + 1;

    strcpy(buffer_ptr, name); //don't want to bother saving length ahead
    buffer_ptr += LbStringLength(name) + 1;

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

    LbStringCopy(ptr, netstate.users[updated_user].name,
        sizeof(netstate.users[updated_user].name));
    ptr += LbStringLength(netstate.users[updated_user].name) + 1;

    netstate.sp->sendmsg_single(dest, netstate.msg_buffer,
        ptr - netstate.msg_buffer);
}

static void SendClientFrame(const char * frame_buffer, int seq_nbr) //seq_nbr because it isn't necessarily determined
{
    char * ptr;

    NETDBG(9, "Starting");

    ptr = netstate.msg_buffer;

    *ptr = NETMSG_FRAME;
    ptr += 1;

    *(int *) ptr = seq_nbr;
    ptr += 4;

    LbMemoryCopy(ptr, frame_buffer, netstate.user_frame_size);
    ptr += netstate.user_frame_size;

    netstate.sp->sendmsg_single(SERVER_ID, netstate.msg_buffer,
        ptr - netstate.msg_buffer);
}

static unsigned CountLoggedInClients(void)
{
    NetUserId id;
    unsigned count;

    for (count = 0, id = 0; id < netstate.max_players; ++id) {
        if (netstate.users[id].progress == USER_LOGGEDIN) {
            ++count;
        }
    }

    return count;
}

static void SendServerFrame(void)
{
    char * ptr;
    size_t size;

    NETDBG(9, "Starting");

    ptr = netstate.msg_buffer;
    *ptr = NETMSG_FRAME;
    ptr += sizeof(char);

    *(int *) ptr = netstate.seq_nbr;
    ptr += sizeof(int);

    *ptr = CountLoggedInClients() + 1;
    ptr += sizeof(char);

    size = (CountLoggedInClients() + 1) * netstate.user_frame_size;
    LbMemoryCopy(ptr, netstate.exchg_buffer, size);
    ptr += size;

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

    len = LbStringLength(ptr) + 1;
    ptr += len;
    if (len > sizeof(netstate.password)) {
        NETDBG(6, "Connected peer attempted to flood password");
        netstate.sp->drop_user(source);
        return;
    }

    LbStringCopy(netstate.users[source].name, ptr, sizeof(netstate.users[source].name));
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

    //send reply
    ptr = netstate.msg_buffer;
    ptr += 1; //skip header byte which should still be ok
    LbMemoryCopy(ptr, &source, 1); //assumes LE
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

    LbStringCopy(netstate.users[id].name, ptr, sizeof(netstate.users[id].name));

    //send up the stuff the other parts of the game expect
    //TODO NET try to get rid of this because it makes understanding code much more complicated
    localPlayerInfoPtr[id].active = netstate.users[id].progress != USER_UNUSED;
    strcpy(localPlayerInfoPtr[id].name, netstate.users[id].name);
}

static void HandleClientFrame(NetUserId source, char * ptr, char * end)
{
    NETDBG(7, "Starting");

    netstate.users[source].ack = *(int *) ptr;
    ptr += 4;

    LbMemoryCopy(&netstate.exchg_buffer[source * netstate.user_frame_size],
        ptr, netstate.user_frame_size);
    ptr += netstate.user_frame_size;

    if (ptr >= end) {
        //TODO NET handle bad frame
        NETMSG("Bad frame size from client %u", source);
        return;
    }

    NETDBG(9, "Handled client frame of %u bytes", netstate.user_frame_size);
}

static void HandleServerFrame(char * ptr, char * end)
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
    if (netstate.exchg_queue == NULL) {
        netstate.exchg_queue = frame;
    }
    else {
        for (it = netstate.exchg_queue; it->next != NULL; it = it->next);
        it->next = frame;
    }

    frame->next = NULL;
    frame->size = num_user_frames * netstate.user_frame_size;
    frame->buffer = (char *) LbMemoryAlloc(frame->size);
    frame->seq_nbr = seq_nbr;

    LbMemoryCopy(frame->buffer, ptr, frame->size);

    NETDBG(9, "Handled server frame of %u bytes", frame->size);
}

static void HandleMessage(NetUserId source)
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
        if (netstate.my_id == SERVER_ID) {
            HandleLoginRequest(source, buffer_ptr, buffer_end);
        }
        else {
            HandleLoginReply(buffer_ptr, buffer_end);
        }
        break;
    case NETMSG_USERUPDATE:
        if (netstate.my_id != SERVER_ID) {
            HandleUserUpdate(source, buffer_ptr, buffer_end);
        }
        break;
    case NETMSG_FRAME:
        if (netstate.my_id == SERVER_ID) {
            HandleClientFrame(source, buffer_ptr, buffer_end);
        }
        else {
            HandleServerFrame(buffer_ptr, buffer_end);
        }
        break;
    case NETMSG_LAGWARNING:
        break;
    default:
        break;
    }
}

static TbError ProcessMessage(NetUserId source)
{
    size_t rcount;

    rcount = netstate.sp->readmsg(source, netstate.msg_buffer,
        sizeof(netstate.msg_buffer));

    if (rcount > 0) {
        HandleMessage(source);
    }
    else {
        NETLOG("Problem reading message from %u", source);
        return Lb_FAIL;
    }

    return Lb_OK;
}

static void VerifyBufferSize(void)
{
    size_t required_msg_buffer_size;

    required_msg_buffer_size = (netstate.user_frame_size + sizeof(unsigned)) * netstate.max_players + 1;

    if (required_msg_buffer_size > sizeof(netstate.msg_buffer)) { //frame data + seq nbr
        ERRORLOG("Too small message buffer size: %u bytes required, %u bytes available. Will ABORT: Force programmer to fix error",
            required_msg_buffer_size, sizeof(netstate.msg_buffer));
        abort(); //no point in continuing, code bug
    }
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

TbError LbNetwork_Init(unsigned long srvcindex, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct ServiceInitData *init_data)
{
  TbError res;
  NetUserId usr;

  res = Lb_FAIL;

  localPlayerInfoPtr = locplayr; //TODO NET try to get rid of dependency on external player list, makes things 2x more complicated

  /*//return _DK_LbNetwork_Init(srvcp,guid,maxplayrs,exchng_buf,exchng_size,locplayr,init_data);
  exchangeSize = exchng_size;
  maximumPlayers = maxplayrs;
  //thread_data_mem = _wint_thread_data;
  basicTimeout = 250;
  localDataPtr = 0;
  compositeBuffer = 0;
  sequenceNumber = 0;
  timeCount = 0;
  maxTime = 0;
  runningTwoPlayerModel = 0;
  waitingForPlayerMapResponse = 0;
  compositeBufferSize = 0;
  //_wint_thread_data = &thread_data_mem;
  receiveCallbacks.multiPlayer = MultiPlayerCallback;
  receiveCallbacks.field_24 = NULL;
  exchangeBuffer = exchng_buf;
  receiveCallbacks.mpReqExDataMsg = MultiPlayerReqExDataMsgCallback;
  localPlayerInfoPtr = locplayr;
  compositeBufferSize = exchng_size * maxplayrs;
  if (compositeBufferSize > 0)
  {
    compositeBuffer = LbMemoryAlloc(compositeBufferSize);
  }
  if ((compositeBufferSize <= 0) || (compositeBuffer == NULL))
  {
    WARNLOG("Failure on buffer allocation");
    //_wint_thread_data = thread_data_mem;
    return Lb_FAIL;
  }
  ClearClientData();
  GetPlayerInfo();*/

  //clear network object and init it to neutral config
  LbMemorySet(&netstate, 0, sizeof(netstate));
  for (usr = 0; usr < MAX_N_USERS; ++usr) {
      netstate.users[usr].id = usr;
  }

  netstate.max_players = maxplayrs;
  netstate.exchg_buffer = (char *) exchng_buf;
  netstate.user_frame_size = exchng_size;
  VerifyBufferSize();

  // Initialising the service provider object
  switch (srvcindex)
  {
  case NS_Serial:
      NETMSG("Selecting Serial SP");
      if (GenericSerialInit(init_data) == Lb_OK)
      {
        res = Lb_OK;
      } else
      {
        WARNLOG("Failure on Serial Initialization");
        res = Lb_FAIL;
      }
      break;
  case NS_Modem:
      NETMSG("Selecting Modem SP");
      if (GenericModemInit(init_data) == Lb_OK)
      {
        res = Lb_OK;
      } else
      {
        WARNLOG("Failure on Modem Initialization");
        res = Lb_FAIL;
      }
      break;
  case NS_IPX:
      NETMSG("Selecting IPX SP");
      if (GenericIPXInit(init_data) == Lb_OK)
      {
        res = Lb_OK;
      } else
      {
        WARNLOG("Failure on IPX Initialization");
        res = Lb_FAIL;
      }
      break;
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
  default:
      WARNLOG("The serviceIndex value of %d is out of range", srvcindex);
      res = Lb_FAIL;
      break;
  }

  if (netstate.sp) {
      res = netstate.sp->init(OnDroppedUser); //TODO NET supply drop callback
  }

  //_wint_thread_data = thread_data_mem;
  return res;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, unsigned long *plyr_num, void *optns)
{
  /*TbError ret;
  TbClockMSec tmStart;
  //return _DK_LbNetwork_Join(nsname, plyr_name, plyr_num);
  ret = Lb_FAIL;
  tmStart = LbTimerClock();
  if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  if (runningTwoPlayerModel)
  {
    remotePlayerId = 0;
    remotePlayerIndex = 0;
    localPlayerId = 1;
    localPlayerIndex = 1;
  } else
  {
    localPlayerId = (unsigned) -1;
  }
  sequenceNumber = 15;
  if (spPtr->Start(nsname, plyr_name, optns))
  {
    WARNLOG("Failure on Join");
    return Lb_FAIL;
  }
  if (!runningTwoPlayerModel)
  {
    spPtr->EncodeMessageStub(&systemUserBuffer, 1, 4, runningTwoPlayerModel);
    systemUserBuffer[4] = 0;
    spPtr->Send(0,systemUserBuffer);
    waitingForPlayerMapResponse = 1;
    while (waitingForPlayerMapResponse)
    {
      spPtr->Receive(8);
      if ( waitingForPlayerMapResponse )
      {
        if (LbTimerClock()-tmStart > 10000)
        {
          waitingForPlayerMapResponse = 0;
          return ret;
        }
      }
    }
  }
  ret = GetCurrentPlayers();
  if (ret != Lb_OK)
  {
    WARNLOG("Cannot get current players");
    return ret;
  }
  *plyr_num = localPlayerIndex;
  ret = GetPlayerInfo();
  if (ret != Lb_OK)
  {
    WARNLOG("Cannot get player info");
    return ret;
  }*/

    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }

    if (netstate.sp->join(nsname->text, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }

    netstate.my_id = 23456;

    SendLoginRequest(plyr_name, netstate.password);
    ProcessMessagesUntilNextLoginReply(WAIT_FOR_SERVER_TIMEOUT_IN_MS);
    if (netstate.msg_buffer[0] != NETMSG_LOGIN) {
        NETMSG("Network login rejected");
        return Lb_FAIL;
    }
    ProcessMessage(SERVER_ID);

    if (netstate.my_id == 23456) {
        NETMSG("Network login unsuccessful");
        return Lb_FAIL;
    }

    *plyr_num = netstate.my_id;

    return Lb_OK;
}

TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num, void *optns)
{
  //return _DK_LbNetwork_Create(nsname_str, plyr_name, plyr_num);
  /*if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  if ( runningTwoPlayerModel )
  {
    localPlayerId = 0;
    localPlayerIndex = 0;
    remotePlayerId = 1;
    remotePlayerIndex = 1;
  } else
  {
    localPlayerId = 0;
    localPlayerIndex = 0;
    hostId = 0;
  }
  if (spPtr->Start(nsname_str, plyr_name, maximumPlayers, optns) != Lb_OK)
  {
    WARNLOG("Failure on SP::Start()");
    return Lb_FAIL;
  }
  *plyr_num = localPlayerIndex;
  if (GetCurrentPlayers() != Lb_OK)
  {
    WARNLOG("Cannot get current players");
    return Lb_FAIL;
  }
  if (GetPlayerInfo() != Lb_OK)
  {
    WARNLOG("Cannot get player info");
    return Lb_FAIL;
  }*/

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
    localPlayerInfoPtr[netstate.my_id].active = 1;
    strcpy(localPlayerInfoPtr[netstate.my_id].name, netstate.users[netstate.my_id].name);

    LbNetwork_EnableNewPlayers(true);
    return Lb_OK;
}

TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long buf_size)
{
  /*void *cbuf;
  long comps_size;
  //return _DK_LbNetwork_ChangeExchangeBuffer(buf, buf_size);
  exchangeBuffer = buf;
  exchangeSize = buf_size;
  comps_size = buf_size * maximumPlayers;
  if (compositeBuffer == NULL)
  {
    cbuf = LbMemoryAlloc(comps_size);
    if (cbuf == NULL)
    {
      WARNLOG("Failure on buffer allocation");
      compositeBuffer = NULL;
      return Lb_FAIL;
    }
    compositeBuffer = cbuf;
    compositeBufferSize = comps_size;
    return Lb_OK;
  }
  if (comps_size <= compositeBufferSize)
    return Lb_OK;
  cbuf = LbMemoryAlloc(comps_size);
  if (cbuf == NULL)
  {
    WARNLOG("Failure on buffer reallocation");
    return Lb_FAIL;
  }
  LbMemoryCopy(cbuf, compositeBuffer, compositeBufferSize);
  LbMemoryFree(compositeBuffer);
  compositeBuffer = cbuf;
  compositeBufferSize = comps_size;*/

    NETDBG(2, "Setting user frame buffer size to %u", buf_size);

    netstate.user_frame_size = buf_size;
    netstate.exchg_buffer = (char *) buf;

    VerifyBufferSize();

    return Lb_OK;
}

void LbNetwork_EnableLag(TbBool lag)
{
    netstate.enable_lag = lag;
}

void LbNetwork_ChangeExchangeTimeout(unsigned long tmout)
{
  exchangeTimeout = 1000 * tmout;
}

TbError LbNetwork_Stop(void)
{
    NetFrame* frame;
    NetFrame* nextframe;

    /*//return _DK_LbNetwork_Stop();
  if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  if (spPtr->Release())
    WARNLOG("Failure on Release");
  if (spPtr != NULL)
    delete spPtr;
  spPtr = NULL;
  if (compositeBuffer != NULL)
    LbMemoryFree(compositeBuffer);
  actualTimeout = 0;
  localDataPtr = 0;
  compositeBuffer = NULL;
  maxTime = 0;
  startTime = 0;
  waitingForPlayerMapResponse = 0;
  compositeBufferSize = 0;
  maximumPlayers = 0;
  localPlayerIndex = 0;
  localPlayerId = 0;
  gotCompositeData = 0;
  exchangeBuffer = NULL;
  exchangeSize = 0;
  sequenceNumber = 0;
  spPtr = 0;
  basicTimeout = 250;
  timeCount = 0;
  hostId = 0;
  runningTwoPlayerModel = 0;
  ClearClientData();
  exchangeTimeout = 0;*/

    if (netstate.sp) {
        netstate.sp->exit();
    }

    frame = netstate.exchg_queue;
    while (frame != NULL) {
        nextframe = frame->next;
        LbMemoryFree(frame->buffer);
        LbMemoryFree(frame);
        frame = nextframe;
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
        LbMemorySet(&netstate.users[id], 0, sizeof(netstate.users[id]));
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
        LbMemorySet(localPlayerInfoPtr[id].name, 0, sizeof(localPlayerInfoPtr[id].name));
    }
    else {
        NETMSG("Quitting after connection loss");
        LbNetwork_Stop();
    }
}

static void ProcessMessagesUntilNextFrame(NetUserId id, unsigned timeout)
{
    /*TbClockMSec start;
    start = LbTimerClock();*/

    //read all messages up to next frame
    while (timeout == 0 || netstate.sp->msgready(id,
            timeout /*- (min(LbTimerClock() - start, max(timeout - 1, 0)))*/) != 0) {
        if (ProcessMessage(id) == Lb_FAIL) {
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

static void ProcessMessagesUntilNextLoginReply(TbClockMSec timeout)
{
    TbClockMSec start;
    start = LbTimerClock();

    //read all messages up to next frame
    while (timeout == 0 || netstate.sp->msgready(SERVER_ID,
            timeout - (min(LbTimerClock() - start, max(timeout - 1, 0l)))) != 0) {
        if (ProcessMessage(SERVER_ID) == Lb_FAIL) {
            break;
        }

        if (netstate.msg_buffer[0] == NETMSG_LOGIN) {
            break;
        }

        if (LbTimerClock() - start > timeout) {
            break;
        }
    }
}

static void ProcessPendingMessages(NetUserId id)
{
    //process as many messages as possible
    while (netstate.sp->msgready(id, 0) != 0) {
        if (ProcessMessage(id) == Lb_FAIL) {
            return;
        }
    }
}

static void ConsumeServerFrame(void)
{
    NetFrame * frame;

    frame = netstate.exchg_queue;
    NETDBG(8, "Consuming Server frame %d of size %u", frame->seq_nbr, frame->size);

    netstate.exchg_queue = frame->next;
    netstate.seq_nbr = frame->seq_nbr;
    LbMemoryCopy(netstate.exchg_buffer, frame->buffer, frame->size);
    LbMemoryFree(frame->buffer);
    LbMemoryFree(frame);
}

TbError LbNetwork_Exchange(void *buf)
{
    NetUserId id;

    NETDBG(7, "Starting");
  //return _DK_LbNetwork_Exchange(buf);
  /*spPtr->update();
  if (LbNetwork_StartExchange(buf) != Lb_OK)
  {
    WARNLOG("Failure when Starting Exchange");
    return Lb_FAIL;
  }
  if (LbNetwork_CompleteExchange(buf) != Lb_OK)
  {
    WARNLOG("Failure when Completing Exchange");
    return Lb_FAIL;
  }*/
    assert(UserIdentifiersValid());

    if (netstate.users[netstate.my_id].progress == USER_SERVER) {
        //server needs to be careful about how it reads messages
        for (id = 0; id < MAX_N_USERS; ++id) {
            if (id == netstate.my_id) {
                continue;
            }

            if (netstate.users[id].progress == USER_UNUSED) {
                continue;
            }

            if (netstate.users[id].progress == USER_LOGGEDIN) {
                if (!netstate.enable_lag ||
                        netstate.seq_nbr >= SCHEDULED_LAG_IN_FRAMES) { //scheduled lag in TCP stream
                    //TODO NET take time to detect a lagger which can then be announced
                    ProcessMessagesUntilNextFrame(id, WAIT_FOR_CLIENT_TIMEOUT_IN_MS);
                }

                netstate.seq_nbr += 1;
                SendServerFrame();
            }
            else {
                ProcessMessagesUntilNextFrame(id, WAIT_FOR_CLIENT_TIMEOUT_IN_MS);
                netstate.seq_nbr += 1;
                SendServerFrame();
            }
        }
    }
    else { //client
        if (netstate.enable_lag) {
            ProcessPendingMessages(SERVER_ID);

            if (netstate.exchg_queue == NULL) {
                //we need at least one frame so block
                ProcessMessagesUntilNextFrame(SERVER_ID, 0);
            }

            if (netstate.exchg_queue == NULL) {
                //connection lost
                return Lb_FAIL;
            }

            SendClientFrame((char *) buf, netstate.exchg_queue->seq_nbr);
        }

        if (!netstate.enable_lag) {
            SendClientFrame((char *) buf, netstate.seq_nbr);
            ProcessMessagesUntilNextFrame(SERVER_ID, 0);

            if (netstate.exchg_queue == NULL) {
                //connection lost
                return Lb_FAIL;
            }
        }

        ConsumeServerFrame(); //most likely overwrites what is sent in SendClientFrame
    }

    //TODO NET deal with case where no new frame is available and game should be stalled

    netstate.sp->update(OnNewUser);

    assert(UserIdentifiersValid());

    NETDBG(7, "Ending");

    return Lb_OK;
}

TbBool LbNetwork_Resync(void * buf, size_t len)
{
    char * full_buf;
    int i;

    NETLOG("Starting");

    full_buf = (char *) LbMemoryAlloc(len + 1);

    if (netstate.users[netstate.my_id].progress == USER_SERVER) {
        full_buf[0] = NETMSG_RESYNC;
        LbMemoryCopy(full_buf + 1, buf, len);

        for (i = 0; i < MAX_N_USERS; ++i) {
            if (netstate.users[i].progress != USER_LOGGEDIN) {
                continue;
            }

            netstate.sp->sendmsg_single(netstate.users[i].id, full_buf, len + 1);
        }
    }
    else {
        //discard all frames until next resync frame
        do {
            if (netstate.sp->readmsg(SERVER_ID, full_buf, len + 1) < 1) {
                NETLOG("Bad reception of resync message");
                return false;
            }
        } while (full_buf[0] != NETMSG_RESYNC);

        LbMemoryCopy(buf, full_buf + 1, len);
    }

    LbMemoryFree(full_buf);

    return true;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow)
{
  //return _DK_LbNetwork_EnableNewPlayers(allow);
  /*if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  if (allow)
  {
    NETMSG("New players ARE allowed to join");
    return spPtr->EnableNewPlayers(true);
  } else
  {
    NETMSG("New players are NOT allowed to join");
    return spPtr->EnableNewPlayers(false);
  }*/

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
//  TbBool local_init;

  struct TbNetworkCallbackData netcdat;

  SYNCDBG(7, "Starting");

  //return _DK_LbNetwork_EnumerateServices(callback, ptr);
/*
  local_init = false;
  if (!network_initialized)
  {
    if (LbNetwork_Startup() != Lb_OK)
      local_init = true;
  }
  if (network_initialized)
  {
    strcpy(netcdat.svc_name, "SERIAL");
    callback(&netcdat, ptr);
    strcpy(netcdat.svc_name, "MODEM");
    callback(&netcdat, ptr);
    strcpy(netcdat.svc_name, "IPX");
    callback(&netcdat, ptr);
    NETMSG("Enumerate Services called");
  }
  if (local_init)
    LbNetwork_Shutdown();
  return Lb_OK;
*/
  strcpy(netcdat.svc_name, "Serial");
  callback(&netcdat, ptr);
  strcpy(netcdat.svc_name, "Modem");
  callback(&netcdat, ptr);
  strcpy(netcdat.svc_name, "IPX");
  callback(&netcdat, ptr);
  strcpy(netcdat.svc_name, "TCP");
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
    TbNetworkCallbackData data;
    NetUserId id;

    SYNCDBG(9, "Starting");

  /*char ret;
  //return _DK_LbNetwork_EnumeratePlayers(sesn, callback, a2);
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
            callback(&data, buf);
        }
    }

    return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr)
{
    unsigned i;

    SYNCDBG(9, "Starting");

  //char ret;
  //return _DK_LbNetwork_EnumerateSessions(callback, ptr);
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

        callback((TbNetworkCallbackData *) &sessions[i], ptr);
    }

    return Lb_OK;
}

TbError LbNetwork_StartExchange(void *buf)
{
  if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  if (runningTwoPlayerModel)
    return StartTwoPlayerExchange(buf);
  else
    return StartMultiPlayerExchange(buf);
}

TbError LbNetwork_CompleteExchange(void *buf)
{
  if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  if ( runningTwoPlayerModel )
    return CompleteTwoPlayerExchange(buf);
  else
    return CompleteMultiPlayerExchange(buf);
}

TbError ClearClientData(void)
{
  long i;
  LbMemorySet(clientDataTable, 0, 32*sizeof(struct ClientDataEntry));
  for (i=0; i < maximumPlayers; i++)
  {
    clientDataTable[i].isactive = 0;
  }
  return Lb_OK;
}

TbError GetCurrentPlayers(void)
{
  if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  NETLOG("Starting");
  localPlayerIndex = maximumPlayers;
  if (spPtr->Enumerate(0, GetCurrentPlayersCallback, 0))
  {
    WARNLOG("Failure on SP::Enumerate()");
    return Lb_FAIL;
  }
  if (localPlayerIndex >= maximumPlayers)
  {
    WARNLOG("localPlayerIndex not updated, max players %d",maximumPlayers);
    return Lb_FAIL;
  }
  return Lb_OK;
}

void __stdcall GetCurrentPlayersCallback(struct TbNetworkCallbackData *netcdat, void *a2)
{
  AddAPlayer((struct TbNetworkPlayerNameEntry *)netcdat);
}

TbError GetPlayerInfo(void)
{
  struct TbNetworkPlayerInfo *lpinfo;
  long i;
  NETLOG("Starting");
  for (i=0; i < netstate.max_players; i++)
  {
    lpinfo = &localPlayerInfoPtr[i];
    if ( netstate.users[i].progress == USER_SERVER ||
            netstate.users[i].progress == USER_LOGGEDIN )
    {
      lpinfo->active = 1;
      LbStringCopy(lpinfo->name, netstate.users[i].name, 32);
    } else
    {
      lpinfo->active = 0;
    }
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
      localPlayerInfoPtr[i].active = 1;
      strcpy(localPlayerInfoPtr[i].name,plyrname->name);
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
    localPlayerIndex = plr_id;
  }
  return Lb_OK;
}

TbError GenericSerialInit(void *init_data)
{
  struct ServiceInitData *sp_init;
  if (spPtr != NULL)
  {
    spPtr->Release();
    delete spPtr;
    spPtr = NULL;
  }
  sp_init = (struct ServiceInitData *)init_data;
  LbMemorySet(lastMessage, 0, sizeof(lastMessage));
  LbMemorySet(lastButOneMessage, 0, sizeof(lastMessage));
  basicTimeout = 250;
  receiveCallbacks.mpReqExDataMsg = TwoPlayerReqExDataMsgCallback;
  startTime = 0;
  actualTimeout = 0;
  remotePlayerIndex = 0;
  remotePlayerId = 0;
  sequenceNumber = 0;
  runningTwoPlayerModel = true;
  receiveCallbacks.multiPlayer = TwoPlayerCallback;
  if (sp_init != NULL)
    sp_init->field_C = 1;
  //TODO: NET set when SerialSP is ready
  spPtr = NULL;//new SerialSP(...);
  if (spPtr == NULL)
  {
    WARNLOG("Failure on SP construction");
    return Lb_FAIL;
  }
  if (spPtr->Init(&receiveCallbacks, 0) != Lb_OK)
  {
    WARNLOG("Failure on SP::Init()");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError GenericModemInit(void *init_data)
{
  struct ServiceInitData *sp_init;
  if (spPtr != NULL)
  {
    spPtr->Release();
    delete spPtr;
    spPtr = NULL;
  }
  sp_init = (struct ServiceInitData *)init_data;
  LbMemorySet(lastMessage, 0, sizeof(lastMessage));
  LbMemorySet(lastButOneMessage, 0, sizeof(lastMessage));
  basicTimeout = 250;
  receiveCallbacks.mpReqExDataMsg = TwoPlayerReqExDataMsgCallback;
  startTime = 0;
  actualTimeout = 0;
  remotePlayerIndex = 0;
  remotePlayerId = 0;
  sequenceNumber = 0;
  runningTwoPlayerModel = true;
  receiveCallbacks.multiPlayer = TwoPlayerCallback;
  if (sp_init != NULL)
    sp_init->field_C = 2;
//TODO: NET set when ModemSP is ready
  spPtr = NULL;//new ModemSP(...);
  if (spPtr == NULL)
  {
    WARNLOG("Failure on SP construction");
    return Lb_FAIL;
  }
  if (spPtr->Init(&receiveCallbacks, 0) != Lb_OK)
  {
    WARNLOG("Failure on SP::Init()");
    return Lb_FAIL;
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
  if (spPtr->Init(&receiveCallbacks, 0) != Lb_OK)
  {
    WARNLOG("Failure on SP::Init()");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError GenericTCPInit(void *init_data)
{
    if (spPtr != NULL) {
        spPtr->Release();
        delete spPtr;
        spPtr = NULL;
    }

    spPtr = new TCPServiceProvider();
    if (spPtr == NULL) {
        WARNLOG("Failure on SP construction");
        return Lb_FAIL;
    }
    if (spPtr->Init(&receiveCallbacks, 0) != Lb_OK) {
        WARNLOG("Failure on SP::Init()");
        return Lb_FAIL;
    }

  return Lb_OK;
}

TbError SendRequestCompositeExchangeDataMsg(const char *func_name)
{
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(requestCompositeExchangeDataBuffer))
  {
    WARNMSG("%s: requestCompositeExchangeDataBuffer is too small",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeRequestCompositeExchangeDataMsg(requestCompositeExchangeDataBuffer,localPlayerId,sequenceNumber);
  if (spPtr->Send(hostId, requestCompositeExchangeDataBuffer) != 0)
  {
    WARNMSG("%s: Failure on SP::Send()",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError SendRequestToAllExchangeDataMsg(unsigned long src_id,unsigned long seq, const char *func_name)
{
  long i;
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(requestExchangeDataBuffer))
  {
    WARNMSG("%s: requestCompositeExchangeDataBuffer is too small",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, src_id, seq);
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].isactive) && (!clientDataTable[i].field_8))
    {
      if (spPtr->Send(clientDataTable[i].plyrid,requestExchangeDataBuffer))
        WARNMSG("%s: Failure on SP::Send()",func_name);
    }
  }
  return Lb_OK;
}

TbError SendRequestExchangeDataMsg(unsigned long dst_id,unsigned long src_id,unsigned long seq, const char *func_name)
{
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(requestExchangeDataBuffer))
  {
    WARNMSG("%s: requestCompositeExchangeDataBuffer is too small",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, src_id, seq);
  if (spPtr->Send(dst_id,requestExchangeDataBuffer))
  {
    WARNMSG("%s: Failure on SP::Send()",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError SendDeletePlayerMsg(unsigned long dst_id,unsigned long del_id,const char *func_name)
{
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(deletePlayerBuffer))
  {
    WARNMSG("%s: deletePlayerBuffer is too small",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeDeletePlayerMsg(deletePlayerBuffer, del_id);
  if (spPtr->Send(dst_id, deletePlayerBuffer) != Lb_OK)
  {
    WARNLOG("Failure on SP::Send()");
    return Lb_FAIL;
  }
  NETMSG("%s: Sent delete player message",func_name);
  return Lb_OK;
}

TbError HostDataCollection(void)
{
  TbError ret;
  TbClockMSec tmPassed;
  int exchngNeeded;
  TbBool keepExchng;
  unsigned long nRetries;
  long i;
  long k;
  ret = Lb_FAIL;

  keepExchng = true;
  nRetries = 0;
  while ( keepExchng )
  {
    exchngNeeded = 1;
    for (i=0; i < maximumPlayers; i++)
    {
      if ((clientDataTable[i].isactive) && (!clientDataTable[i].field_8))
      {
        exchngNeeded = clientDataTable[i].field_8;
      }
    }
    if (exchngNeeded)
    {
      keepExchng = false;
      if (nRetries == 0)
      {
        tmPassed = LbTimerClock()-startTime;
        if ((timeCount == 0) || (tmPassed > maxTime))
          maxTime = tmPassed;
        timeCount++;
        if (timeCount >= 50)
        {
          timeCount = 0;
          basicTimeout = 4 * maxTime;
          if (basicTimeout < 250)
            basicTimeout = 250;
        }
      }
      ret = Lb_OK;
      continue;
    }
    tmPassed = LbTimerClock()-startTime;
    if (tmPassed > actualTimeout)
    {
      NETMSG("Timed out waiting for client");
      nRetries++;
      if (nRetries <= 10)
      {
        SendRequestToAllExchangeDataMsg(hostId, sequenceNumber, __func__);
      } else
      {
        if (spPtr->GetRequestCompositeExchangeDataMsgSize() <= sizeof(deletePlayerBuffer))
        {
          for (i=0; i < maximumPlayers; i++)
          {
            if ((clientDataTable[i].isactive) && (!clientDataTable[i].field_8))
            {
              spPtr->EncodeDeletePlayerMsg(deletePlayerBuffer, clientDataTable[i].plyrid);
              for (k=0; k < maximumPlayers; k++)
              {
                if ((clientDataTable[k].isactive) && (clientDataTable[k].plyrid != clientDataTable[i].plyrid))
                {
                  if ( spPtr->Send(clientDataTable[i].plyrid,deletePlayerBuffer) )
                    WARNLOG("failure on SP::Send()");
                }
              }
            }
          }
        } else
        {
          WARNLOG("deletePlayerBuffer is too small");
        }
      }
      startTime = LbTimerClock();
      actualTimeout = (nRetries + 1) * basicTimeout;
      basicTimeout += 100;
    }
    spPtr->Receive(8);
  }
  return ret;
}

TbError HostDataBroadcast(void)
{
  TbError ret;
  long i;
  ret = Lb_OK;
  spPtr->EncodeMessageStub(exchangeBuffer, maximumPlayers*exchangeSize-4, 0, sequenceNumber);
  LbMemoryCopy(compositeBuffer, exchangeBuffer, maximumPlayers*exchangeSize);
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].isactive) && (clientDataTable[i].plyrid != hostId))
    {
      if ( spPtr->Send(clientDataTable[i].plyrid, exchangeBuffer) )
      {
        WARNLOG("Failure on SP::Send()");
          ret = Lb_FAIL;
      }
    }
  }
  return ret;
}

TbError SendSequenceNumber(void *buf, const char *func_name)
{
  if (hostId == localPlayerId)
  {
    if (HostDataCollection() != Lb_OK)
    {
      WARNMSG("%s: Failure on HostDataCollection()",func_name);
      return Lb_FAIL;
    }
    if (HostDataBroadcast() != Lb_OK)
    {
      WARNMSG("%s: Failure on HostDataBroadcast()",func_name);
      return Lb_FAIL;
    }
  } else
  {
    spPtr->EncodeMessageStub(buf, exchangeSize-4, 0, sequenceNumber);
    if (spPtr->Send(hostId, buf) != Lb_OK)
    {
      WARNMSG("%s: Failure on SP::Send()",func_name);
      return Lb_FAIL;
    }
  }
  return Lb_OK;
}

TbError StartTwoPlayerExchange(void *buf)
{
  if (!clientDataTable[remotePlayerIndex].isactive)
    spPtr->Receive(2);
  gotCompositeData = 0;
  if (clientDataTable[remotePlayerIndex].isactive)
  {
    spPtr->Receive(8);
    spPtr->Receive(16);
  }
  memcpy((uchar *)exchangeBuffer + exchangeSize * localPlayerIndex, buf, exchangeSize);
  if (clientDataTable[remotePlayerIndex].isactive)
  {
    spPtr->EncodeMessageStub(buf, exchangeSize-4, 0, sequenceNumber);
    if (spPtr->Send(remotePlayerId, buf) != Lb_OK)
    {
      WARNLOG("Failure on SP::Send()");
      return Lb_FAIL;
    }
    startTime = LbTimerClock();
    if (exchangeTimeout)
      actualTimeout = exchangeTimeout;
    else
      actualTimeout = basicTimeout;
    memcpy(lastButOneMessage, lastMessage, exchangeSize);
    memcpy(lastMessage, buf, exchangeSize);
  }
  return Lb_OK;
}

TbError StartMultiPlayerExchange(void *buf)
{
  struct ClientDataEntry  *clidat;
  long i;
  localDataPtr = buf;
  spPtr->Receive(6);
  for (i=0; i < maximumPlayers; i++)
  {
    clidat = &clientDataTable[i];
    if (clidat->isactive)
      clidat->field_8 = 0;
  }
  LbMemoryCopy((uchar *)exchangeBuffer + exchangeSize * localPlayerIndex, buf, exchangeSize);
  clientDataTable[localPlayerIndex].field_8 = 1;
  startTime = LbTimerClock();
  actualTimeout = basicTimeout;
  if (hostId == localPlayerId)
    return Lb_OK;
  spPtr->EncodeMessageStub(buf, exchangeSize-4, 0, exchangeSize-4);
  if (spPtr->Send(hostId, buf) != Lb_OK)
  {
    WARNLOG("Failure on SP::Send()");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError CompleteTwoPlayerExchange(void *buf)
{
  TbError ret;
  TbBool keepExchng;
  TbClockMSec tmPassed;
  long nRetries;
  ret = Lb_FAIL;
  keepExchng = true;
  if (!clientDataTable[remotePlayerIndex].isactive )
    return 0;
  nRetries = 0;
  while ( keepExchng )
  {
    spPtr->Receive(8);
    if (gotCompositeData)
    {
      keepExchng = false;
      if (nRetries == 0)
      {
        tmPassed = LbTimerClock()-startTime;
        if (tmPassed < 0)
          tmPassed = -tmPassed;
        if ((timeCount == 0) || (tmPassed > maxTime))
          maxTime = tmPassed;
        timeCount++;
        if (timeCount >= 50)
        {
          timeCount = 0;
          basicTimeout = 3 * maxTime;
          if (basicTimeout < 250)
            basicTimeout = 250;
        }
      }
      ret = 0;
    }
    if (keepExchng)
    {
      tmPassed = LbTimerClock()-startTime;
      if (tmPassed < 0)
        tmPassed = -tmPassed;
      if (tmPassed > actualTimeout)
      {
        NETMSG("Timed out (%d) waiting for seq %d - %d ms", tmPassed, sequenceNumber, actualTimeout);
        nRetries++;
        if (nRetries <= 10)
        {
          NETMSG("Requesting %d resend of packet (%d)", nRetries, sequenceNumber);
          SendRequestExchangeDataMsg(remotePlayerId, localPlayerId, sequenceNumber, __func__);
        } else
        {
          NETMSG("Tried to resend %d times, aborting", nRetries);
          SendDeletePlayerMsg(localPlayerId, remotePlayerId, __func__);
          return Lb_FAIL;
        }
        startTime = LbTimerClock();
        actualTimeout = exchangeTimeout;
        if (actualTimeout == 0)
        {
          if (nRetries < 3)
            actualTimeout = basicTimeout;
          else
          if (nRetries == 3)
            actualTimeout = 2 * basicTimeout;
          else
            actualTimeout = (nRetries-3) * 4 * basicTimeout;
        }
      }
    }
    if (!clientDataTable[remotePlayerIndex].isactive)
    {
      keepExchng = false;
      ret = 0;
    }
  }
  if (sequenceNumber != 15)
  {
    sequenceNumber++;
    if (sequenceNumber >= 15)
      sequenceNumber = 0;
  }
  return ret;
}

TbError CompleteMultiPlayerExchange(void *buf)
{
  TbError ret;
  TbBool hostChange;
  TbBool keepExchng;
  TbClockMSec tmPassed;
  long nRetries;
  long i;
  ret = Lb_FAIL;
  if (hostId != localPlayerId)
  {
    gotCompositeData = 0;
    keepExchng = true;
    hostChange = false;
    nRetries = 0;
    while (keepExchng)
    {
      i = hostId;
      spPtr->Receive(8);
      if (i != hostId)
        hostChange = true;
      if (hostChange)
      {
        ret = SendSequenceNumber(buf,__func__);
        if (hostId == localPlayerId)
        {
          keepExchng = 0;
          break;
        }
      } else
      if (gotCompositeData)
      {
        if (nRetries == 0)
        {
          tmPassed = LbTimerClock()-startTime;
          if ((timeCount == 0) || (tmPassed > maxTime))
            maxTime = tmPassed;
          timeCount++;
          if (timeCount >= 50)
          {
            timeCount = 0;
            basicTimeout = 4 * maxTime;
            if (basicTimeout < 250)
              basicTimeout = 250;
          }
        }
        keepExchng = 0;
        ret = Lb_OK;
      }
      tmPassed = LbTimerClock()-startTime;
      if (!keepExchng)
        break;
      // Now the time out code
      if (tmPassed <= actualTimeout)
        continue;
      NETMSG("Timed out waiting for host");
      nRetries++;
      if (nRetries <= 10)
      {
        SendRequestCompositeExchangeDataMsg(__func__);
      } else
      {
        SendDeletePlayerMsg(localPlayerId, hostId, __func__);
      }
      startTime = LbTimerClock();
      actualTimeout = (nRetries+1) * basicTimeout;
      basicTimeout += 100;
    }
  } else
  {
    HostDataCollection();
    ret = HostDataBroadcast();
  }
  localDataPtr = 0;
  if (sequenceNumber != 15)
  {
    sequenceNumber++;
    if (sequenceNumber >= 15)
      sequenceNumber = 0;
  }
  return ret;
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
  gotCompositeData = true;
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
      if (localPlayerInfoPtr != NULL)
      {
        localPlayerInfoPtr[i].active = 0;
      } else
      {
        WARNLOG("NULL localPlayerInfoPtr");
      }
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
  unidirectionalMsgReceived = 1;
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

void TwoPlayerReqExDataMsgCallback(unsigned long, unsigned long, void *)
{
//TODO NET rewrite (less importand - used only for modem and serial)
}

void *TwoPlayerCallback(unsigned long, unsigned long, unsigned long, void *)
{
//TODO NET (less importand - used only for modem and serial)
  return NULL;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
