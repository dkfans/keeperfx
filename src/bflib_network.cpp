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
 * @date     11 Apr 2009 - 14 Aug 2020
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
// Local functions definition
TbError GetCurrentPlayers(void);
TbError AddAPlayer(struct TbNetworkPlayerNameEntry *plyrname);
static TbError GenericSerialInit(void *init_data);
static TbError GenericModemInit(void *init_data);
static TbError GenericIPXInit(void *init_data);
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

// Send no more bytes than this per one packet
#define MAX_OUTGOING_SIZE     940
// Store no more that this number of unconfirmed actions
#define MAX_STORED_ACTIONS    256
// Move base_seq when stored more than this amount
#define MAX_STORED_ACTIONS_TH 3

unsigned long inside_sr;
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
struct UnidirectionalRTSMessage rtsMessage;
/******************************************************************************/

// New network code declarations start here ===================================

/**
 * Max wait for a client before we declare client messed up.
 */
#define WAIT_FOR_CLIENT_TIMEOUT_IN_MS   1200
#define WAIT_FOR_SERVER_TIMEOUT_IN_MS   WAIT_FOR_CLIENT_TIMEOUT_IN_MS

/* Disconnected if no messages recieved for that interval */
#define DISCONNECT_TIMEOUT              2000

#define SESSION_COUNT 32 //not arbitrary, it's what code calling EnumerateSessions expects

// Should be less than max packet size from bflib_tcpsp.c
#define BF_SYNC_DATA_SIZE 980
#define MAX_FRAME_DATA_SIZE 980

typedef unsigned short SeqType;
#define SEQ_ALWAYS        ((SeqType)0xFFFF)
#define SEQ_WINDOW_LEN    60

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
    TbClockMSec             last_message_time;
};

struct PacketBuffer
{
    short size;
    short max_size;
    char data[MAX_FRAME_DATA_SIZE];
};

/*
  This struct is a memory node to hold and reorder NetBufferItems
*/
struct NetBufferNode
{
    struct NetBufferNode *next;
    struct NetBufferNode *prev;
    unsigned int          delivery_flag;
    SeqType               out_seq;
    short                 size;
    char                  data[];
};

struct NetBufferList
{
    struct NetBufferNode *first = 0;
    struct NetBufferNode *last = 0;

    void append(struct NetBufferNode *node);
    struct NetBufferNode *remove(struct NetBufferNode *node); //return next item
    bool empty() { return first == NULL; }
    void clear();
    void join_list(NetBufferList *list2);
    struct NetBufferNode *rfind_by_data(void *data);
    // Find and remove (starting from the end)
    struct NetBufferNode *rextract_by_data(void *data);
};

/*
    This struct is sent inside packets
*/
struct NetBufferItem
{
    unsigned long           turn;
    short                   size;
    SeqType                 in_seq;
    unsigned char           kind;
    unsigned char           player;
    unsigned char           buffer[0];
};
 
enum NetMessageType
{
    NETMSG_LOGIN = 0,       //to server: username and pass, from server: assigned id
    NETMSG_USERUPDATE,      //changed player from server
    NETMSG_FRAME,           //to server: ACK of frame + packets, from server: the frame itself
    NETMSG_LAGWARNING,      //from server: notice that some client is lagging¨
    NETMSG_RESYNC,          //from server: re-synchronization is occurring
    NETMSG_SYNC_CONFIRM,    //to server
};

/**
 */
#pragma push(pack)
#pragma pack(1)
struct SyncPacket
{
    NetMessageType      message_type;
    unsigned long       sync_turn;
    short               packet_num;     // number of current packet in list
    short               packet_count;   // amount of packets in list
    short               len;
    char                data[];
};

/*
Resync related
*/
struct PacketNode
{
    struct PacketNode   *next;
    short               confirmed; // bitmask for confirmations
    short               len;
    char                data[];
};

/*
  Used on client too for clarity
*/
struct NetServerHeader
{
    unsigned char   packet_kind;
    int             seq_nbr;
    unsigned char   num_clients;
};

#pragma pop(pack)

/**
 * Contains the entire network state.
 */
struct NetState
{
    const struct NetSP *    sp;                 //pointer to service provider in use
    struct NetUser          users[MAX_N_USERS]; //the users

    char                    password[32];       //password for server
    NetUserId               my_id;              //id for user representing this machine
    int                     seq_nbr;            //sequence number of next frame to be issued
    int                     max_players;        //max players that will actually be used
    int                     active_players;     //how many active players

    TbBool                  locked;             //if set, no players may join

    int                     resync_mode;        // == 1 if we are in process of resync and == 2 if post resync
    unsigned long           resync_prev_turn;   //we want to skip old messages
    long                    resync_turn;        //game turn to sync game with
    short                   resync_last_packet; // # of last packet from server
    short                   resync_packet_count;// total amount of packets to receive
    TbClockMSec             resync_next_message_time;
    int                     resync_sent_packets;
    int                     resync_total_packets;
    short                   resync_done_players;

    LbNetwork_Client_Callback client_callback;

    struct PacketNode       *sync_packets;      // list of packets with parts of game state
    struct PacketNode       *cli_sync_packets;  // client list with all received parts of game state

    struct NetServerHeader  *server_header;
    struct NetServerHeader  *client_header;

    // if ( (action->seq - seq_min) > MAX_PACKETS ) <discard action>
    SeqType                 in_seq_min[MAX_N_USERS];

    SeqType                 out_seq;

    unsigned int            delivery_mask; // whenever a packet delivered to all client

    SeqType                 seq_last_sent;
    SeqType                 last_confirmed[MAX_N_USERS];
    unsigned char           confirmed_this_turn[MAX_N_USERS];

    // List of not processed items (sorted by playturn?)
    struct NetBufferList    incoming_list;

    // List of messages created this turn (they should be mixed with incoming)
    struct NetBufferList    created_list;

    // List of items to send
    struct NetBufferList    outgoing_list;
};

//the "new" code contained in this struct
static struct NetState netstate;

//sessions placed here for now, would be smarter to store dynamically
static struct TbNetworkSessionNameEntry sessions[SESSION_COUNT]; //using original because enumerate expects static life time

// Scratch buffer (outgoing)
static char temp_buffer_data[MAX_FRAME_DATA_SIZE];

int nullFn;

const struct NetSP nullSP =
{
    [](NetDropCallback)->TbError{ return 0; },
    []()->void{},
    [](const char*, void *)->TbError{ return 0; },
    [](const char*, void *)->TbError{ return 0; },
    [](NetNewUserCallback)->void{},
    [](int, const char*, unsigned int)->void{},
    [](const char*, size_t)->void{},
    [](NetUserId, unsigned int)->size_t{ return 0; },
    [](NetUserId, char*, unsigned int)->size_t{ return 0; },
    [](NetUserId)->void{}
};
// New network code data definitions end here =================================
/*****/

static SeqType current_packet_id = 0;

/*****/
static void ProcessMessagesUntilNextLoginReply(TbClockMSec timeout, struct PacketBuffer *pbuffer);
static void process_confirmation(bool is_server, unsigned int netuser_idx, SeqType tail_num);
static void create_update_seq_packet(NetUserId id);
static void log_nodes(struct NetBufferList *lst, int player_idx);
/*****/

/**
  allocate NetBufferNode
*/
static struct NetBufferNode *sm_alloc(size_t size)
{
    struct NetBufferNode *ret = (struct NetBufferNode*) malloc(sizeof(struct NetBufferNode) + size);
    ret->delivery_flag = 0;
    ret->out_seq = 0;
    ret->next = NULL;
    ret->prev = NULL;
    ret->size = size;
    memset(ret->data, 0, size);
    return ret;
}

static struct NetBufferNode *sm_append(struct NetBufferList &list, size_t size)
{
    struct NetBufferNode *ret = sm_alloc(size);
    list.append(ret);
    return ret;
}

static void sm_free(struct NetBufferNode *node)
{
    NETDBG(11, "base:%p", node);
    free(node);
}

/*
  Get next seq number
*/
static inline SeqType next_seq()
{
    SeqType ret = netstate.out_seq++;
    if (netstate.out_seq == SEQ_ALWAYS) // -1 and 0 are special case
    {
        fprintf(stderr, "SEQ_ALWAYS");
        netstate.out_seq++;
    }
    if (netstate.out_seq == 0) // -1 and 0 are special case
    {
        fprintf(stderr, "SEQ_0");
        netstate.out_seq++;
    }
    return ret;
}

void NetBufferList::append(struct NetBufferNode *node)
{
    if (last)
    {
        last->next = node;
    }
    node->prev = last;
    last = node;
    if (!first)
    {
        first = node;
    }
}

struct NetBufferNode *NetBufferList::remove(struct NetBufferNode *node)
{
    struct NetBufferNode *ret = node->next;
    if (node->prev)
        node->prev->next = node->next;
    else if (this->first == node)
            this->first = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else if (this->last == node)
            this->last = node->prev;
    sm_free(node);
    return ret;
}

void NetBufferList::clear()
{
    //TODO: delete all
    for (struct NetBufferNode *node = first; node != NULL; )
    {
        struct NetBufferNode *node2 = node->next;
        sm_free(node);
        node = node2;
    }
    first = NULL;
    last = NULL;
}

void NetBufferList::join_list(NetBufferList *list2)
{
    for (struct NetBufferNode *node = list2->first;
        node != NULL;
        )
    {
        struct NetBufferNode *node2 = node->next;
        // TODO: keep nodes sorted

        node->next = NULL;
        node->prev = this->last;
        if (this->last)
            this->last->next = node;
        if (this->first == NULL)
            this->first = node;
        this->last = node;

        node = node2;
    }
    list2->first = NULL;
    list2->last = NULL;
}

struct NetBufferNode *NetBufferList::rextract_by_data(void *data)
{
    struct NetBufferNode *ret = NULL;
    for (struct NetBufferNode *node = this->last;
        node != NULL;
        )
    {
        struct NetBufferNode *node2 = node->prev;
        struct NetBufferItem *item = (struct NetBufferItem *)node->data;
        if (data == item->buffer)
        {
            if (node->prev)
                node->prev->next = node->next;
            if (node->next)
                node->next->prev = node->prev;
            ret = node;
            break;
        }
        node = node2;
    }
    if (ret == NULL)
        return ret;
    if (this->first == ret)
        this->first = ret->next;
    if (this->last == ret)
        this->last = ret->prev;
    ret->next = NULL;
    ret->prev = NULL;
    return ret;
}

struct NetBufferNode *NetBufferList::rfind_by_data(void *data)
{
    for (struct NetBufferNode *node = this->last;
        node != NULL;
        node = node->prev
        )
    {
        struct NetBufferItem *item = (struct NetBufferItem *)node->data;
        if (data == item->buffer)
            return node;
    }
    return NULL;
}
int netlist_size(struct NetBufferList *list)
{
    int cnt = 0;
    for (struct NetBufferNode *node = list->first; node != NULL; node = node->next)
    {
      cnt++;
    }
    return cnt;
}

//debug function to find out reason for mutating peer ids
static TbBool UserIdentifiersValid(void)
{
    NetUserId i;
    for (i = 0; i < MAX_N_USERS; ++i) {
        if (netstate.users[i].id != i) {
            NETMSG("Bad peer ID on index %i", i);
            EVM_GLOBAL_EVENT("mp.bad_peer,id=%d,n=%d cnt=1", netstate.users[i].id, i);
            return 0;
        }
    }

    return 1;
}

static void SendLoginRequest(const char * name, const char * password)
{
    char * buffer_ptr;

    NETMSG("Logging in as %s", name);

    buffer_ptr = temp_buffer_data;
    *buffer_ptr = NETMSG_LOGIN;
    buffer_ptr += 1;

    strcpy(buffer_ptr, password);
    buffer_ptr += LbStringLength(password) + 1;

    strcpy(buffer_ptr, name); //don't want to bother saving length ahead
    buffer_ptr += LbStringLength(name) + 1;

    netstate.sp->sendmsg_single(SERVER_ID, temp_buffer_data, buffer_ptr - temp_buffer_data);
}

static void SendUserUpdate(NetUserId dest, NetUserId updated_user)
{
    char * ptr;

    ptr = temp_buffer_data;

    *ptr = NETMSG_USERUPDATE;
    ptr += 1;

    *ptr = (char)netstate.active_players;
    ptr += 1;

    *ptr = updated_user;
    ptr += 1;

    *ptr = netstate.users[updated_user].progress;
    ptr += 1;

    LbStringCopy(ptr, netstate.users[updated_user].name,
        sizeof(netstate.users[updated_user].name));
    ptr += LbStringLength(netstate.users[updated_user].name) + 1;

    netstate.sp->sendmsg_single(dest, temp_buffer_data,
        ptr - temp_buffer_data);
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
        NETDBG(3, "Connected peer attempted to flood password");
        netstate.sp->drop_user(source);
        return;
    }

    LbStringCopy(netstate.users[source].name, ptr, sizeof(netstate.users[source].name));
    if (!isalnum(netstate.users[source].name[0])) {
        //TODO NET drop player for bad name
        //also replace isalnum with something that considers foreign non-ASCII chars
        NETDBG(3, "Connected peer had bad name starting with %c",
            netstate.users[source].name[0]);
        netstate.sp->drop_user(source);
        return;
    }

    //presume login successful from here
    NETMSG("User %s successfully logged in", netstate.users[source].name);
    netstate.users[source].progress = USER_LOGGEDIN;

    //send reply
    NETDBG(7, "SendUserUpdate");
    ptr = temp_buffer_data;
    *ptr = NETMSG_LOGIN;
    ptr += 1;
    LbMemoryCopy(ptr, &source, 1); //assumes LE
    ptr += 1;
    netstate.sp->sendmsg_single(source, temp_buffer_data, ptr - temp_buffer_data);

    netstate.delivery_mask |= (1 << source);
    create_update_seq_packet(source);
    netstate.active_players++; // All clients should get new number

    //send user updates
    for (id = 0; id < MAX_N_USERS; ++id)
    {
        if (netstate.users[id].progress == USER_UNUSED) {
            continue;
        }

        SendUserUpdate(source, id);

        if (id == netstate.my_id || id == source) {
            continue;
        }

        SendUserUpdate(id, source);
    }

    // TODO: reject player?
    netstate.client_callback(false, source, netstate.active_players, netstate.users[source].name);

    NETDBG(7, "Done");
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

    netstate.active_players = *ptr;
    ptr += 1;
    assert( netstate.active_players <= MAX_N_USERS) ;

    id = (NetUserId) *ptr;
    if (id < 0 && id >= MAX_N_USERS) {
        NETLOG("Critical error: Out of range user ID %i received from server, could be used for buffer overflow attack", id);
        abort();
    }
    ptr += 1;

    netstate.users[id].progress = (enum NetUserProgress) *ptr;
    ptr += 1;

    LbStringCopy(netstate.users[id].name, ptr, sizeof(netstate.users[id].name));

    NETDBG(7, "Got update id:%d active_players:%d", id, netstate.active_players);
    netstate.client_callback(
        netstate.users[id].progress != USER_UNUSED,id, netstate.active_players, netstate.users[id].name);
    
}

// Called from server on first packet
static void create_update_seq_packet(NetUserId id)
{
    struct NetBufferNode *node = sm_append(netstate.outgoing_list, sizeof(struct NetBufferItem) + sizeof(SeqType));
    struct NetBufferItem *buf_struct = (struct NetBufferItem *)node->data;

    node->out_seq = 1; // That is the point - target will accept this
    node->delivery_flag = ~(1 << id);

    buf_struct->turn = 0;
    buf_struct->size = sizeof(SeqType);
    buf_struct->kind = PckA_UpdateBaseSeq;
    buf_struct->in_seq = node->out_seq;
    buf_struct->player = netstate.my_id;

    *((SeqType*)buf_struct->buffer) = netstate.out_seq - (SEQ_WINDOW_LEN / 2);
    NETDBG(6, "sending UpdateSeq player:%d seq:%d mask:%04u",
        id, *((SeqType*)buf_struct->buffer), netstate.delivery_mask);
}

static char * send_confirm_packet(SeqType last_confirmed[], char *ptr, int id, int &cnt)
{
    NETDBG(7, "sending Confirm player:%d seq:%d", id, last_confirmed[id]);
    struct NetBufferItem *buf_struct = (struct NetBufferItem *)ptr;

    buf_struct->turn = 0;
    buf_struct->size = 2;
    buf_struct->kind = PckA_ConfirmSeq;
    buf_struct->in_seq = SEQ_ALWAYS;
    buf_struct->player = (char) netstate.my_id;

    *((SeqType*)buf_struct->buffer) = last_confirmed[id];
    ptr += sizeof(SeqType) + sizeof(struct NetBufferItem);
    cnt++;
    return ptr;
}

/*
Send something like this:

    char type = NETMSG_FRAME;
    int  seq_number = netstate.seq_nbr;
    char sm_count;
    struct NetBufferItem items[sm_count];
*/
static void SendServerFrame()
{
    //netstate.server_header->num_clients = CountLoggedInClients() + 1;

    NETDBG(13, "num_clients:%d", netstate.server_header->num_clients);

    char *ptr, *saved_ptr;
    // We should reserve some space for confirmation packet
    const char *ptr_end = temp_buffer_data + MAX_OUTGOING_SIZE - sizeof(struct NetBufferItem) - 2;
    unsigned char *sm_count;
    int cnt;

    NETDBG(10, "Starting");
    ptr = temp_buffer_data;

    *ptr = NETMSG_FRAME;
    ptr += 1;

    *(int *) ptr = netstate.seq_nbr;
    ptr += sizeof(int);
    sm_count = (unsigned char*)ptr;
    ptr ++;
    saved_ptr = ptr;

    for (int id = 1; id < MAX_N_USERS; ++id)
    {
        if (netstate.users[id].progress != USER_LOGGEDIN)
            continue;

        unsigned int id_mask = 1 << id;

        ptr = saved_ptr;
        cnt = 0;

        for (struct NetBufferNode *node = netstate.outgoing_list.first;
            node != NULL;
            node = node->next
            )
        {
            if ((ptr + node->size) > ptr_end)
            {
                //TODO: start a resync?
                NETLOG("Overflow player:%d cnt:%d", id, cnt);
                log_nodes(&netstate.outgoing_list, id);
                return;
            }

            struct NetBufferItem *item = (struct NetBufferItem *)ptr;

            if (node->delivery_flag & id_mask)
            {
                // Skipping already confirmed messages same id
                NETDBG(8, "skipped seq:%d player:%d ", node->out_seq, id);
                continue;
            }
            memcpy(ptr, node->data, node->size);

            if (node->out_seq == 0)
            {
                if (item->in_seq != SEQ_ALWAYS)
                {
                    node->out_seq = next_seq();
                    NETDBG(8, "reassigned old:%d seq:%d", item->in_seq, node->out_seq);
                }
            }
            item->in_seq = node->out_seq;

            ptr += node->size;
            cnt++;
        }

        if (netstate.confirmed_this_turn[id] > 0)
        {
            ptr = send_confirm_packet(netstate.last_confirmed, ptr, id, cnt);
        }

        assert(cnt < 255);
        sm_count[0] = (unsigned char)cnt;

        netstate.sp->sendmsg_single(id, temp_buffer_data, ptr - temp_buffer_data);
    }
}

/*
Send something like this:

    char type = NETMSG_FRAME;
    int  seq_number = netstate.seq_nbr;
    char sm_count;
    struct NetBufferItem items[sm_count];
*/
static void SendClientFrame(int seq_nbr) //seq_nbr because it isn't necessarily determined
{
    char *ptr;
    // We should reserve some space for confirmation packet
    const char *ptr_end = temp_buffer_data + MAX_OUTGOING_SIZE - sizeof(struct NetBufferItem) - 2;
    unsigned char *sm_count;
    int cnt = 0;

    NETDBG(10, "Starting");
    ptr = temp_buffer_data;

    *ptr = NETMSG_FRAME;
    ptr += 1;

    *(int *) ptr = seq_nbr;
    ptr += sizeof(int);
    sm_count = (unsigned char*)ptr;
    ptr ++;

    unsigned int id = 0;

    for (struct NetBufferNode *node = netstate.outgoing_list.first;
        node != NULL;
        node = node->next
        )
    {
        if ((ptr + node->size) > ptr_end)
        {
            NETLOG("Overflow cnt:%d", cnt);
            log_nodes(&netstate.outgoing_list, id);
            return;
        }
        //assert((ptr + node->size) <= ptr_end);
        if (node->out_seq == 0)
        {
            node->out_seq = ((struct NetBufferItem *)node->data)->in_seq;
        }

        memcpy(ptr, node->data, node->size);
        ptr += node->size;
        cnt++;
    }

    if (netstate.confirmed_this_turn[id] > 0)
    {
        ptr = send_confirm_packet(netstate.last_confirmed, ptr, id, cnt);
    }

    assert(cnt < 255);
    sm_count[0] = (unsigned char)cnt;
    if (netstate.sp == NULL)
    {
        ERRORLOG("sp closed!");
        return;
    }
    netstate.sp->sendmsg_single(SERVER_ID, temp_buffer_data, ptr - temp_buffer_data);
}

static TbBool HandleFrame(NetUserId source, char * ptr, char * end, bool is_server)
{
    int seq_nbr;
    int sm_count;

    NETDBG(8, "Starting");

    seq_nbr = *(int *) ptr;
    ptr += sizeof(int);

    if (seq_nbr < netstate.users[SERVER_ID].ack)
    {
        NETDBG(5, "Old packet source:%d seq:%05d", SERVER_ID, seq_nbr);
        return false;
    }
    netstate.users[source].ack = seq_nbr;

    sm_count = *ptr;
    ptr++;
    for (int i = 0; i < sm_count; i++)
    {
        // TODO here we mix abstraction layers
        struct NetBufferItem *buf_struct = (struct NetBufferItem *)ptr;
        size_t size = sizeof(struct NetBufferItem) + buf_struct->size;
        NETDBG(7, "Got buf kind:%d sz:%d turn:%04ld player:%d", 
            buf_struct->kind, buf_struct->size, buf_struct->turn, (int) buf_struct->player);

        // PckA_ConfirmSeq is "transport level" packet - it should not get into lists
        if (buf_struct->kind == PckA_ConfirmSeq)
        {
            SeqType seq = ((SeqType*)buf_struct->buffer)[0];
            //fprintf(stderr, "got Confirm from:%d seq:%d\n", source, seq);
            process_confirmation(is_server, source, seq);
            ptr += size;
            continue;
        } else if (buf_struct->kind == PckA_UpdateBaseSeq)
        {
            struct NetBufferNode *node = sm_append(netstate.incoming_list, size);
            node->delivery_flag = 0xFFFFFFFF; //We got an update - dont send it to anyone

            memcpy(node->data, ptr, size);
            ptr += size;
            continue;
        }

        struct NetBufferNode *node = sm_append(netstate.incoming_list, size);
        node->delivery_flag |= (1 << source); // This guy send us a packet, so he already know it
        memcpy(node->data, ptr, size);
        ptr += size;
    }

    NETDBG(8, "Handled frame");
    return true;
}

/*
    Here we should put pbuffer in list or just discard it
*/
static TbError UnpackMessage(NetUserId source, struct PacketBuffer *pbuffer)
{
    size_t rcount;
    enum NetMessageType type;
    char * buffer_ptr = pbuffer->data;
    char * buffer_end;
     TbBool ok;
    NETDBG(8, "Blocking read %u", source);

    rcount = netstate.sp->readmsg(source, pbuffer->data, pbuffer->max_size);

    if (rcount > 0) {
        pbuffer->size = (short)rcount;
        buffer_end = buffer_ptr + pbuffer->size;
        struct SyncPacket *sync_packet;

        NETDBG(8, "Handling message from %u", source);

        //type
        type = (enum NetMessageType) *buffer_ptr;
        buffer_ptr += 1;

        switch (type) {
        case NETMSG_LOGIN:
            if (netstate.my_id == SERVER_ID)
            {
                HandleLoginRequest(source, buffer_ptr, buffer_end);
            }
            else
            {
                HandleLoginReply(buffer_ptr, buffer_end);
            }
            break;
        case NETMSG_USERUPDATE:
            if (netstate.my_id != SERVER_ID)
            {
                HandleUserUpdate(source, buffer_ptr, buffer_end);
            }
            break;
        case NETMSG_FRAME:
            ok = HandleFrame(source, buffer_ptr, buffer_end, netstate.my_id == SERVER_ID);
            if (!ok)
            {
                buffer_ptr[-1] = 0; // We have to wait for next frame
            }
            break;
        case NETMSG_LAGWARNING:
            break;
        case NETMSG_RESYNC:
            sync_packet = (struct SyncPacket *)(buffer_ptr-1);
            if (sync_packet->sync_turn < netstate.resync_prev_turn)
            {   // This is a wandering resync packet from the past
                NETDBG(6, "resync message from the past %ld < %ld", sync_packet->sync_turn, netstate.resync_prev_turn);
            }
            else
            {   // Other side want to start resync
                NETDBG(6, "resync message for turn %ld %d/%d", sync_packet->sync_turn, sync_packet->packet_num, sync_packet->packet_count);
                netstate.resync_mode = 1;
                netstate.resync_turn = sync_packet->sync_turn;
            }
            break;
        case NETMSG_SYNC_CONFIRM:
            NETDBG(1, "Sync Confirm?");
            return Lb_OK;
        default:
            NETDBG(1, "Unknown type %02x", type);
        }
    }
    else {
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

static void network_init_header(TbBool server)
{
    if (server)
    {
        netstate.server_header = (struct NetServerHeader *)malloc(sizeof(struct NetServerHeader *));
        netstate.client_header = NULL;
    }
    else
    {
        netstate.server_header = NULL;
        netstate.client_header = (struct NetServerHeader *)malloc(sizeof(struct NetServerHeader *));
    }
}

static void network_init_common(unsigned long maxplayers)
{
    //clear network object and init it to neutral config

    LbMemorySet(&netstate, 0, sizeof(netstate));
    for (NetUserId usr = 0; usr < MAX_N_USERS; ++usr)
    {
        netstate.users[usr].id = usr;
    }

    netstate.out_seq = 1;
    netstate.delivery_mask = 1; // player0 is mine

    netstate.incoming_list.clear();
    netstate.created_list.clear();
    netstate.outgoing_list.clear();

    netstate.max_players = maxplayers;
    netstate.resync_mode = 0;
    netstate.resync_prev_turn = 0;
}

TbError LbNetwork_InitSingleplayer()
{
    network_init_common(1);
    netstate.users[netstate.my_id].progress = USER_SERVER;
    network_init_header(true);
    netstate.sp = &nullSP;
    NETDBG(7, "done")
    return Lb_OK;
}

TbError LbNetwork_Init(unsigned long srvcindex, unsigned long maxplayrs,
    LbNetwork_Client_Callback client_callback, struct ServiceInitData *init_data)
{
    TbError res;

    NETDBG(7, "srvcindex:%ld", srvcindex);

    res = Lb_FAIL;

    network_init_common(maxplayrs);

    netstate.client_callback = client_callback;

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
        NETMSG("Selecting UDP SP");

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

    return res;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, const char *plyr_name, unsigned long *plyr_num, void *optns)
{
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }

    if (netstate.sp->join(nsname->text, optns) == Lb_FAIL) {
        return Lb_FAIL;
    }

    netstate.my_id = 23456;

    struct PacketBuffer temp_node = {0, .max_size=MAX_FRAME_DATA_SIZE, 0};
    NETDBG(8, "Sending login request");
    SendLoginRequest(plyr_name, netstate.password);
    NETDBG(8, "Waiting for response");
    ProcessMessagesUntilNextLoginReply(WAIT_FOR_SERVER_TIMEOUT_IN_MS, &temp_node);
    if (temp_node.data[0] != NETMSG_LOGIN) {
        NETMSG("Network login rejected");
        return Lb_FAIL;
    }
    UnpackMessage(SERVER_ID, &temp_node);

    if (netstate.my_id == 23456) {
        NETMSG("Network login unsuccessful");
        return Lb_FAIL;
    }

    *plyr_num = netstate.my_id;

    return Lb_OK;
}

TbError LbNetwork_Create(char *nsname_str, const char *plyr_name, unsigned long *plyr_num, void *optns)
{
    NETDBG(7, "Starting");
    if (!netstate.sp) {
        ERRORLOG("No network SP selected");
        return Lb_FAIL;
    }

    if (netstate.sp->host(":5555", optns) == Lb_FAIL) {
        return Lb_FAIL;
    }

    network_init_header(true);

    netstate.my_id = SERVER_ID;
    LbStringCopy(netstate.users[netstate.my_id].name, plyr_name,
        sizeof(netstate.users[netstate.my_id].name));
    netstate.users[netstate.my_id].progress = USER_SERVER;

    *plyr_num = netstate.my_id;
    netstate.active_players = 1;
    
    netstate.client_callback(
        true, netstate.my_id, netstate.active_players, netstate.users[netstate.my_id].name);

    LbNetwork_EnableNewPlayers(true);
    return Lb_OK;
}

void LbNetwork_ChangeExchangeTimeout(unsigned long tmout)
{
  exchangeTimeout = 1000 * tmout;
}

TbError LbNetwork_Stop(void)
{
    if (netstate.sp)
    {
        netstate.sp->exit();
    }

    // TODO: clear packet data
    LbMemorySet(&netstate, 0, sizeof(netstate));
    NETDBG(7, "Done");
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

    EVM_GLOBAL_EVENT("mp.dropped_user cnt=1,id=%d,reason=%d", id, reason);

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

        for (i = 0; i < MAX_N_USERS; ++i)
        {
            if (i == netstate.my_id)
                continue;

            SendUserUpdate(i, id);
        }

        netstate.client_callback(false, id, netstate.active_players, NULL);
    }
    else {
        NETMSG("Quitting after connection loss");
        LbNetwork_Stop();
    }
}

static TbBool ProcessMessagesUntilNextFrame(
      NetUserId id, unsigned timeout, struct PacketBuffer *pbuffer)
{
    //read all messages up to next frame
    while (timeout == 0 ||
            netstate.sp->msgready(id, timeout) != 0)
    {
        // TODO: enqueue messages
        if (UnpackMessage(id, pbuffer) == Lb_FAIL)
        {
            NETDBG(8, "Failed");
            return false;
        }

        if (pbuffer->data[0] == NETMSG_FRAME ||
            pbuffer->data[0] == NETMSG_RESYNC || 
            pbuffer->data[0] == NETMSG_SYNC_CONFIRM
            )
        {
            break;
        }
    }
    return true;
}

static void ProcessMessagesUntilNextLoginReply(TbClockMSec timeout, struct PacketBuffer *pbuffer)
{
    TbClockMSec start, now;
    assert(timeout > 0);
    start = LbTimerClock();
    unsigned remain = timeout - (min(LbTimerClock() - start, max(timeout - 1, 0l)));
    //read all messages up to next frame
    while (1)
    {
        if (netstate.sp->msgready(SERVER_ID, remain) != 0)
        {
            if (UnpackMessage(SERVER_ID, pbuffer) == Lb_FAIL)
            {
                NETDBG(6, "Failed");
                break;
            }

            if (pbuffer->data[0] == NETMSG_LOGIN)
            {
                NETDBG(7, "Done");
                break;
            }
        }
        now = LbTimerClock();
        if (now - start > timeout)
        {
            NETDBG(7, "Timeout");
            break;
        }
        remain = timeout - min(now - start, 0L);
    }
}

TbBool LbNetwork_CheckFirstPacket()
{
    TbBool ret = (netstate.created_list.empty())? 1 : 0;
    return ret;
}

/*
    Only for outgoing packets (from my player)
*/
void *LbNetwork_AddPacket_f(unsigned char kind, unsigned long turn, short size, const char *func)
{
    struct NetBufferNode *node = sm_append(netstate.created_list, sizeof(struct NetBufferItem) + size);
    struct NetBufferItem *buf_struct = (struct NetBufferItem *)node->data;

    NETDBG(9, "from:%s size:%d seq:%d base:%p", func, size, netstate.out_seq, node);

    buf_struct->turn = turn;
    buf_struct->size = size;
    buf_struct->kind = kind;
    buf_struct->in_seq = next_seq();
    buf_struct->player = (char) netstate.my_id;

    return buf_struct->buffer;
}

/*
  Called on any side when other side confirm some packet
*/
static void process_confirmation(bool is_server, unsigned int id, SeqType tail_num)
{
    NETDBG(7, "user:%d tail:%d", id, tail_num);
    SeqType min_diff = 0xFFFF, min_val = 0;

    tail_num -= netstate.seq_last_sent;

    // That is used to protect from overflow of seq counter
    for (struct NetBufferNode *node = netstate.outgoing_list.first;
        node != NULL;
        )
    {
        if ((node->out_seq == 0) || (node->out_seq == SEQ_ALWAYS))
        {
            node = node->next;
            continue;
        }
        //TODO: compare with local min
        SeqType seq = node->out_seq - netstate.seq_last_sent;
        if (seq <= tail_num)
        {
            node->delivery_flag |= (1 << id);
            if ((node->delivery_flag & netstate.delivery_mask) == netstate.delivery_mask)
            {
                NETDBG(7, "confirmed packet:%d, deleting", node->out_seq);
                // We are removing some packet. So it may be next base
                if (seq < min_diff)
                {
                    min_diff = seq;
                    min_val = node->out_seq;
                }
                node = netstate.outgoing_list.remove(node);
            }
            else
            {
                NETDBG(8, "confirmed packet:%d flag:%x\n", node->out_seq, node->delivery_flag);
                node = node->next;
            }
        }
        else
        {
            NETDBG(10, "skipping:%d\n", seq);
            node = node->next;
        }
    }
    //fprintf(stderr, "=last_sent:%d seq:%d\n", netstate.seq_last_sent, tail_num);
    if (min_diff != 0xFFFF)
    {
        NETDBG(8, "seq_last_sent:%d\n", min_val);
        netstate.seq_last_sent = min_val;
    }
}

static void process_update_seq(int source, struct NetBufferItem* buf_struct)
{
    SeqType seq = ((SeqType*)buf_struct->buffer)[0];
    NETDBG(6, "update last:%d -> seq:%d", netstate.last_confirmed[source], seq);

    netstate.last_confirmed[source] = seq;
}

unsigned short LbNetwork_Packetid()
{
    return current_packet_id;
}

/*
    This function process messages from `created_list` and `incoming_list`
*/
static void process_lists(
    struct NetBufferList *first_list, struct NetBufferList *second_list,
    bool is_server,
    void *context, LbNetwork_Packet_Callback callback)
{
    // Here we should process all messages and reorder it correctly
    struct NetBufferNode *node1 = first_list->first, *node2 = second_list->first;
    struct NetBufferNode *node;

    TbBool turn = false;

    if ((node1 == NULL) && (node2 == NULL))
        return;
    while (true)
    {
        if (node1 == NULL)
        {
            if (node2 == NULL)
                break;
            else
            {
                node = node2;
                node2 = node2->next;
                NETDBG(9, "from:second node:%p", node);
            }
        }
        else
        {
            //TODO any reasonable logic between lists
            if ((node2 == NULL) || turn)
            {
                turn = false;
                node = node1;
                node1 = node1->next;
                NETDBG(9, "from:first node:%p", node);
            }
            else
            {
                turn = true;
                node = node2;
                node2 = node2->next;
                NETDBG(9, "from:second node:%p", node);
            }
        }
        struct NetBufferItem* item = (struct NetBufferItem*)node->data;

        // That is id of message sender, not originator of event
        unsigned int player_idx;
        if (is_server)
        {
            // orginator is same
            player_idx = item->player;
        }
        else
        {
            // in case of client we may got message from ourself or from server
            player_idx = (item->player == netstate.my_id)? item->player : 0;
        }
        assert(player_idx < MAX_N_USERS);

        if (item->size == 0)
        {
            NETLOG("Unexpected empty packet:%p from:%d", node, player_idx);
            assert(item->size != 0);
        }

        if (item->kind == PckA_UpdateBaseSeq)
        {
            process_update_seq(player_idx, item);
            continue;
        }

        assert (item->kind != PckA_ConfirmSeq);
        if ((item->in_seq == 0) || (item->in_seq == SEQ_ALWAYS))
        {
            // Single shot packets
            callback(context, item->turn, item->player, item->kind, item->buffer, item->size);
            // TODO: delete this node after sending to each peer
            assert(false && "Unexpected");
        }
        else
        {
            // action seq should be in order (skips are possible if sent from server)
            SeqType seq = item->in_seq - netstate.last_confirmed[player_idx];
            if (seq < SEQ_WINDOW_LEN)
            {
                current_packet_id = item->in_seq;
                callback(context, item->turn, item->player, item->kind, item->buffer, item->size);
                current_packet_id = 0;

                // We dont want to send confirmation to ourself
                if (item->player != netstate.my_id)
                {
                    NETDBG(8, "new_last:%d(%d) last:%d(%d)",
                        item->in_seq, seq, netstate.last_confirmed[player_idx]);

                    netstate.confirmed_this_turn[player_idx]++;
                }
                else
                {
                    NETDBG(8, "local:%d(%d) last:%d(%d)",
                        item->in_seq, seq, netstate.last_confirmed[player_idx]);
                }
                netstate.last_confirmed[player_idx] = item->in_seq;
                node->delivery_flag |= (1 << netstate.my_id);
            }
            else
            {
                NETDBG(6, "already received seq:%d(%d) last:%d",
                    item->in_seq, seq, netstate.last_confirmed[player_idx]);
            }
        }
    }
    NETDBG(9, "done");
}

void LbNetwork_SetDestination(void *packet_data, int net_player_idx)
{
    struct NetBufferNode *node = netstate.created_list.rfind_by_data(packet_data);
    if (node == NULL)
    {
        ERRORLOG("Unable to find node");
        return;
    }
    node->delivery_flag = ~(1 << net_player_idx);
}

void LbNetwork_MoveToOutgoingQueue(void *packet_data)
{
    struct NetBufferNode *node = netstate.created_list.rextract_by_data(packet_data);
    if (node == NULL)
    {
        ERRORLOG("Unable to find node");
        return;
    }
    netstate.outgoing_list.append(node);
}

/*
  incoming
  created

  to_process_list = incoming + created

  outgoing += created
  
  
*/
enum NetResponse LbNetwork_Exchange(void *context, LbNetwork_Packet_Callback callback)
{
    NetUserId id;
    static TbClockMSec disconnectTime = 10 * DISCONNECT_TIMEOUT;
    bool is_server = (netstate.users[netstate.my_id].progress == USER_SERVER);
    TbBool ok;

    struct PacketBuffer packet_buffer = {0, .max_size = MAX_FRAME_DATA_SIZE, 0 };

    NETDBG(11, "Starting");
    if (netstate.resync_mode == 1)
    {
        disconnectTime = LbTimerClock() + DISCONNECT_TIMEOUT;
        return NR_RESYNC;
    }

    TbClockMSec now = LbTimerClock();
    assert(UserIdentifiersValid());

    memset(netstate.confirmed_this_turn, 0, sizeof(netstate.confirmed_this_turn));

    if (is_server)
    {
        if (netstate.resync_mode == 2)
        {
            NETDBG(7, "Cleanup after resync");
            // If we are after resync - we should send data BEFORE waiting for it
            netstate.resync_mode = 0;
        }
        else
        {
            //server needs to be careful about how it reads messages
            for (id = 0; id < MAX_N_USERS; ++id)
            {
                if (id == netstate.my_id)
                    continue;

                if (netstate.users[id].progress == USER_UNUSED)
                    continue;

                ok = ProcessMessagesUntilNextFrame(id, WAIT_FOR_CLIENT_TIMEOUT_IN_MS, &packet_buffer);
                if (ok)
                {
                    netstate.users[id].last_message_time = now;
                }
                else
                {
                    JUSTLOG("Timeout %04ld %04ld", now, netstate.users[id].last_message_time);
                }
            } // for
        } // if netstate.resync_mode
    }
    else
    { // client
        if (netstate.resync_mode == 2)
        {
            // If we are after resync on client - do nothing
            netstate.resync_mode = 0;
        }
        if (!ProcessMessagesUntilNextFrame(SERVER_ID, 0, &packet_buffer))
        {
            netstate.sp->update(OnNewUser);
            NETDBG(11, "No message");

            netstate.created_list.clear();

            if (disconnectTime > now)
            {
                return NR_DISCONNECT;
            }
            return NR_FAIL;
        }
        disconnectTime = now + DISCONNECT_TIMEOUT;
    }

    process_lists(&netstate.created_list, &netstate.incoming_list,
        is_server, context, callback);

    netstate.outgoing_list.join_list(&netstate.created_list);

    if (is_server)
    {
        if (netstate.delivery_mask == 1)
            netstate.outgoing_list.clear(); // Single player
        else
          netstate.outgoing_list.join_list(&netstate.incoming_list);

        for (struct NetBufferNode *node = netstate.outgoing_list.first;
            node != NULL;
            )
        {
          // clear outgoing list
            if ((node->delivery_flag & netstate.delivery_mask) == netstate.delivery_mask)
            {
                NETDBG(7, "deleting packet:%d");
                node = netstate.outgoing_list.remove(node);
            }
            else
            {
                node = node->next;
            }
        }

        netstate.seq_nbr += 1;

        /* //TODO: hide with #ifdef
        int sz = netlist_size(&netstate.outgoing_list);
        if (sz > 0)
        {
            fprintf(stderr, "netlist_size:%d\n", sz);
        }
        */

        SendServerFrame();
    }
    else
    {
        /* //TODO: hide with #ifdef
        int sz = netlist_size(&netstate.outgoing_list);
        if (sz > 0)
        {
            fprintf(stderr, "netlist_size:%d\n", sz);
        }
        */

        SendClientFrame(netstate.seq_nbr);
        netstate.incoming_list.clear();
    }

    if (netstate.sp == NULL)
    {
        return NR_DISCONNECT;
    }
    netstate.sp->update(OnNewUser);

    assert(UserIdentifiersValid());

    NETDBG(11, "Ending");

    return NR_OK;
}

void LbNetwork_EmptyQueue()
{
    netstate.incoming_list.clear();
    netstate.created_list.clear();
    netstate.outgoing_list.clear();
}


static void log_nodes(struct NetBufferList *lst, int player_idx)
{
    int total = 0;
    JUSTLOG("= ");
    for (struct NetBufferNode *node = lst->first;
        node != NULL;
        node = node->next
        )
    {
        struct NetBufferItem* item = (struct NetBufferItem*)node->data;
        int sz = item->size + sizeof(struct NetBufferItem);
        total += sz;
        JUSTLOG("   fullsize:%d kind:%d turn:%ld seq:%d src:%d delivery:%02x", sz, item->kind, item->turn, item->in_seq, item->player, node->delivery_flag);
    }
    JUSTLOG("= next_seq:%d total_size:%d", netstate.out_seq, total);
}
/*
    Read data if availiable
*/
static int try_read_buf(NetUserId user_id, void *buf, size_t len)
{
    if (!netstate.sp->msgready(user_id, 0))
        return 0;
    return netstate.sp->readmsg(user_id, (char*)buf, len);
}

void LbNetwork_GetResyncProgress(int *now, int *max)
{
    if (netstate.users[netstate.my_id].progress == USER_SERVER)
    {
        *now = netstate.resync_sent_packets; // TODO: calculate amount. Maybe progress of each players
        *max = netstate.resync_total_packets;
    }
    else
    {
        *now = netstate.resync_last_packet;
        *max = netstate.resync_packet_count;
    }
}

static void resync_server_init(unsigned long game_turn, struct SyncArrayItem sync_data[])
{
    struct SyncPacket *packet = NULL;
    struct PacketNode *node, *prev_node = NULL;
    int i;
    size_t remain = 0;
    unsigned char *byte_ptr;
    short packet_num;

    assert(netstate.sync_packets == NULL);
    netstate.resync_mode = 1;
    packet_num = 0;
    node = (struct PacketNode*)LbMemoryAlloc(BF_SYNC_DATA_SIZE + sizeof(struct SyncPacket) + sizeof(struct PacketNode));
    prev_node = node;
    netstate.sync_packets = node;
    for (struct SyncArrayItem *src_node = sync_data; src_node->buf != NULL; src_node++)
    {   // for each part of incoming data
        byte_ptr = (unsigned char *)src_node->buf;
        remain = *src_node->size;
        if (remain == 0)
        {
            NETDBG(6, "skip next sync_item size:%d", remain);
        }
        else
        NETDBG(7, "next sync_item size:%d", remain);

        while (remain > 0)
        {   // for each acceptable chunk of data
            packet = (struct SyncPacket*)&node->data[0];
            packet->message_type = NETMSG_RESYNC;
            packet->sync_turn = game_turn;
            packet->packet_num = packet_num;
            packet->len = (remain > BF_SYNC_DATA_SIZE)? BF_SYNC_DATA_SIZE : remain;
            remain -= packet->len;
            node->len = packet->len + sizeof(struct SyncPacket);

            LbMemoryCopy(packet->data, byte_ptr, packet->len);
            byte_ptr += packet->len;

            node->next = (struct PacketNode*)LbMemoryAlloc(BF_SYNC_DATA_SIZE + sizeof(struct SyncPacket) + sizeof(struct PacketNode));
            prev_node = node;
            node = node->next;
            packet_num++;
        }
    }
    LbMemoryFree(prev_node->next);
    prev_node->next = NULL;

    netstate.resync_total_packets = packet_num * netstate.active_players;
    netstate.resync_sent_packets = 0;
    netstate.resync_done_players = 0;
    NETDBG(5, "Have %d packets, last:%d", packet_num, packet->len);


    for (node = netstate.sync_packets; node != NULL; node = node->next)
    {
        packet = (struct SyncPacket*)&node->data[0];
        packet->packet_count = packet_num;
    }

    for (i = 0; i < MAX_N_USERS; ++i)
    {
        if (netstate.users[i].progress != USER_LOGGEDIN)
        {
            continue;
        }
        NETDBG(7, "Sending to %d id:%d len:%d", i, netstate.users[i].id, netstate.sync_packets->len);
        netstate.sp->sendmsg_single(netstate.users[i].id, netstate.sync_packets->data, netstate.sync_packets->len);
    }
}

static TbBool resync_server(TbBool first_resync, unsigned long game_turn, struct SyncArrayItem sync_data[])
{
    short         *short_ptr;
    struct SyncPacket *packet;
    struct PacketNode *node, *node2;
    int i;
    char incoming_data[BF_SYNC_DATA_SIZE + sizeof(struct SyncPacket) + sizeof(struct PacketNode)];
    int incoming_size;

    if (first_resync)
    {   // We want to prepare all packets with parts of sync data
        resync_server_init(game_turn, sync_data);
    }
    else
    {
        for (i = 0; i < MAX_N_USERS; ++i)
        {
            if (netstate.users[i].progress != USER_LOGGEDIN)
                continue;
            incoming_size = try_read_buf(netstate.users[i].id, incoming_data, sizeof(incoming_data));
            NETDBG(8, "request from %d size:%d", i, incoming_size);
            if (incoming_size <= (sizeof(short) + sizeof(char)))
                continue;
            if (incoming_data[0] == NETMSG_SYNC_CONFIRM)
            {
                short_ptr = (short *)incoming_data;
                if (short_ptr[2] == 0)
                { // client requesting a repeat
                    node = netstate.sync_packets;
                    NETDBG(7, "request for starting packet from %d", i);
                }
                else
                {
                    node = netstate.sync_packets;
                    for (; node != NULL; node = node->next)
                    {
                        packet = (struct SyncPacket*)&node->data[0];
                        if (short_ptr[1] == packet->packet_num)
                        {
                            break;
                        }
                    }
                    if (node != NULL)
                    {
                        netstate.resync_sent_packets++;
                        NETDBG(7, "got confirmation from %d %d/%d", i, short_ptr[1], short_ptr[2]);
                        node->confirmed |= (1 << i);
                        node = node->next;
                    }
                }
                if (node != NULL)
                {
                    packet = (struct SyncPacket*)&node->data[0];
                    NETDBG(7, "sending part #%d len: %d", packet->packet_num, node->len);
                    netstate.sp->sendmsg_single(netstate.users[i].id, node->data, node->len);
                }
                else
                {
                    netstate.resync_done_players++;
                    NETDBG(6, "all data sent to %d", i);
                }
            }
        }
    }
    if (netstate.resync_done_players == netstate.active_players)
    {
        for (i = 0; i < MAX_N_USERS; ++i)
        {
            if (netstate.users[i].progress != USER_LOGGEDIN)
                continue;
            incoming_data[0] = NETMSG_RESYNC;
            packet = (struct SyncPacket*)incoming_data;
            packet->sync_turn = (long) game_turn;
            packet->packet_num = 0;
            packet->packet_count = 0;
            packet->len = 0;
            // THat is a crtical packet! I hope clients will get it
            netstate.sp->sendmsg_single(netstate.users[i].id, incoming_data, sizeof(struct SyncPacket));
            NETDBG(5, "sync finished for %d", i);
        }
        for (node = netstate.sync_packets; node != NULL;)
        {
            node2 = node;
            node = node->next;
            LbMemoryFree(node2);
        }
        netstate.sync_packets = NULL;
        netstate.resync_mode = 2;
        return true;
    }

    return false;
}
/*
    This is a update function. It is called each tick when game is in resync state.
*/
TbBool LbNetwork_Resync(TbBool first_resync, unsigned long game_turn, struct SyncArrayItem sync_data[])
{
    short *short_ptr;
    unsigned char *byte_ptr;
    struct SyncPacket *packet, *packet2;
    struct PacketNode *node, *node2, *node3 = NULL;
    TbClockMSec now;
    char incoming_data[BF_SYNC_DATA_SIZE + sizeof(struct SyncPacket) + sizeof(struct PacketNode)];
    int incoming_size;

    NETDBG(8, "Starting (%s)", first_resync?"first": "next");

    netstate.sp->update(OnNewUser);

    if (netstate.users[netstate.my_id].progress == USER_SERVER)
    {
        return resync_server(first_resync, game_turn, sync_data);
    }
    else
    {
        now = LbTimerClock();
        if (netstate.resync_next_message_time < now)
        {
            netstate.resync_next_message_time = now + 100;

            // We have to request a repeat (we lost some data)
            incoming_data[0] = NETMSG_SYNC_CONFIRM;
            short_ptr = (short*)incoming_data;
            short_ptr[1] = netstate.resync_last_packet;
            short_ptr[2] = netstate.resync_packet_count;
            netstate.sp->sendmsg_single(SERVER_ID, incoming_data, 6);
            NETDBG(8, "asking for resync data %d/%d", short_ptr[1], short_ptr[2]);
        }
        //discard all frames until next resync frame
        do {
            incoming_size = try_read_buf(SERVER_ID, incoming_data, sizeof(incoming_data));
            if (incoming_size < sizeof(struct SyncPacket)) {
                NETLOG("Unable to get resync message");
                return false;
            }
        } while (incoming_data[0] != NETMSG_RESYNC);

        node = (struct PacketNode*)LbMemoryAlloc(BF_SYNC_DATA_SIZE + sizeof(struct SyncPacket) + sizeof(struct PacketNode));
        packet = (struct SyncPacket*)&node->data[0];
        LbMemoryCopy(packet, incoming_data, incoming_size);

        NETDBG(8, "got packet %d/%d %d/%d len:%d", packet->packet_num, packet->packet_count,
              netstate.resync_last_packet, netstate.resync_packet_count, incoming_size);
        if (incoming_size != (sizeof(struct SyncPacket) + packet->len))
        {
            NETLOG("Invalid size %d expected %d for packet #%03d",
                incoming_size,
                (sizeof(struct SyncPacket) + packet->len),
                packet->packet_num);
        }
 
        if ((netstate.resync_packet_count == (netstate.resync_last_packet+1))
            && (packet->len == 0)
            && (packet->packet_count == 0)) // Got confirmation from server
        {
            netstate.resync_packet_count = 0;
            netstate.resync_last_packet = 0;
            netstate.resync_next_message_time = 0;
            ptrdiff_t prev_offset = 0;
            int prev = -1;

            node2 = netstate.cli_sync_packets;
            for (struct SyncArrayItem *src_node = sync_data; src_node->buf != NULL; src_node++)
            {
                byte_ptr = ((unsigned char *)src_node->buf) + prev_offset;
                int remain = *src_node->size;
                for (; node2 != NULL; node2 = node3)
                {
                    if (remain < 0)
                    {
                        NETLOG("Unexpected sync size remain: %d", remain);
                        break;
                    }
                    else if (remain == 0)
                        break; // TODO: I assume that sizeborder
                    node3 = node2->next;
                    packet = (struct SyncPacket*)&node2->data[0];
                    if ((prev + 1) != packet->packet_num)
                    {
                        NETDBG(5, "unexpected sequence prev:%d curr:%d", prev, packet->packet_num);
                    }
                    prev = packet->packet_num;

                    memcpy(byte_ptr, packet->data, min((int)packet->len, remain));
                    byte_ptr += packet->len;
                    remain -= packet->len;
                    LbMemoryFree(node2);
                } // for (node2 ...
            } // for (src_node ...
            assert (node2 == NULL);
            netstate.resync_mode = 2;
            netstate.cli_sync_packets = NULL;
            NETDBG(5, "resync complete");
            return true;
        }

        short packet_cnt;
        if (netstate.cli_sync_packets == NULL)
        {
            netstate.cli_sync_packets = node;
            packet_cnt = 1;
        }
        else
        {
            packet_cnt = 2;
            for (node2 = netstate.cli_sync_packets; node2 != NULL; node2 = node2->next)
            {
                packet2 = (struct SyncPacket*)&node2->data[0];
                if (packet->packet_num == packet2->packet_num)
                {
                    NETDBG(6, "got duplicate packet %d", packet->packet_num);
                    // Just ignore that packet
                    LbMemoryFree(node);
                    return false;
                }
                else
                {
                    packet_cnt++;
                }
                node3 = node2;
            }
            if (node3 == NULL)
            {
                NETDBG(1, "unexpected packet: %d", packet->packet_num);
                LbMemoryFree(node);
                return false;
            }
            node3->next = node;
        }

        netstate.resync_next_message_time = now + 100;
        netstate.resync_last_packet = packet->packet_num;
        netstate.resync_packet_count = packet->packet_count;

        incoming_data[0] = NETMSG_SYNC_CONFIRM;
        short_ptr = (short*)incoming_data;
        short_ptr[1] = netstate.resync_last_packet;
        short_ptr[2] = netstate.resync_packet_count;
        netstate.sp->sendmsg_single(SERVER_ID, incoming_data, 6);
        NETDBG(7, "sending confirmation for %d %d/%d",
            packet->packet_num, short_ptr[1], short_ptr[2]);
    }

    NETDBG(8, "done");
    return false;
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
  strcpy(netcdat.svc_name, "UDP");
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

    // TODO: list sessions possibly ordered by ping

    for (i = 0; i < SESSION_COUNT; ++i) {
        if (!sessions[i].in_use) {
            continue;
        }

        callback((TbNetworkCallbackData *) &sessions[i], ptr);
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

void GetPlayerInfo(void)
{
    int i;
    NETDBG(7, "Starting");
    for (i=0; i < netstate.max_players; i++)
    {
      bool active = ( netstate.users[i].progress == USER_SERVER || netstate.users[i].progress == USER_LOGGEDIN );
      netstate.client_callback(active, i, netstate.active_players, netstate.users[i].name);
    }
}

TbError AddAPlayer(struct TbNetworkPlayerNameEntry *plyrname)
{
    NETLOG("Not implemented");
    return Lb_FAIL;
}

TbBool LbNetwork_IsServer()
{
    return netstate.users[netstate.my_id].progress == USER_SERVER;
}

/*
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
*/

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
/*  if (spPtr != NULL)
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
  } */
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
    NETLOG("Not implemented");
/*
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
*/
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
