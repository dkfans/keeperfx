/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_network.h
 *     Header file for bflib_network.c.
 * @par Purpose:
 *     Network support routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Apr 2009 - 13 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_NETWRK_H
#define BFLIB_NETWRK_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define CLIENT_TABLE_LEN 32
/******************************************************************************/
#pragma pack(1)

// New Declarations Here ======================================================

#define MAX_N_USERS 4
#define MAX_N_PEERS (MAX_N_USERS - 1)
#define SERVER_ID   0

typedef int NetUserId;

enum NetDropReason
{
    NETDROP_MANUAL, // via drop_user()
    NETDROP_ERROR // connection error
};

enum NetMessageType {
    NETMSG_LOGIN,
    NETMSG_USERUPDATE,
    NETMSG_FRONTEND,
    NETMSG_SMALLDATA,
    NETMSG_GAMEPLAY,
    NETMSG_RESYNC_DATA,
    NETMSG_RESYNC_RESUME,
    NETMSG_TIMESYNC_REQUEST,
    NETMSG_TIMESYNC_REPLY,
    NETMSG_TIMESYNC_COMPLETE,
    NETMSG_PAUSE,
    NETMSG_CHATMESSAGE,
};

typedef TbBool  (*NetNewUserCallback)(NetUserId * assigned_id);
typedef void    (*NetDropCallback)(NetUserId id, enum NetDropReason reason);

struct NetSP // new version
{
    /**
     * Inits this service provider.
     * @return Lb_FAIL or Lb_OK
     */
    TbError (*init)(NetDropCallback drop_callback);

    /**
     * Closes down all activities and cleans up this service provider.
     */
    void    (*exit)();

    /**
     * Sets this service provider up as a host for a new network game.
     * @param session String representing the network game to be hosted.
     *  This could be a hostname:port pair for TCP for instance.
     * @param options
     * @return Lb_FAIL or Lb_OK
     */
    TbError (*host)(const char * session, void * options); //leaving void * for now, analyze meaning later

    /**
     * Sets this service provider as a client for an existing network game.
     * @param session String representing the network game to be hosted.
     *  This could be a hostname:port pair for TCP for instance.
     * @param options
     * @return Lb_FAIL or Lb_OK
     */
    TbError (*join)(const char * session, void * options);

    /**
     * Checks for new connections.
     * @param new_user Call back if a new user has connected.
     */
    void    (*update)(NetNewUserCallback new_user);

    /**
     * Sends a message buffer to a certain user.
     * @param destination Destination user.
     * @param buffer
     * @param size Must be > 0
     */
    void    (*sendmsg_single)(NetUserId destination, const char * buffer, size_t size);

    /**
     * Sends a message buffer to a certain user using unsequenced delivery.
     * @param destination Destination user.
     * @param buffer
     * @param size Must be > 0
     */
    void    (*sendmsg_single_unsequenced)(NetUserId destination, const char * buffer, size_t size);

    /**
     * Sends a message buffer to all remote users.
     * @param buffer
     * @param size Must be > 0
     */
    void    (*sendmsg_all)(const char * buffer, size_t size);

    /**
     * Asks if a message has finished reception and get be read through readmsg.
     * May block under some circumstances but shouldn't unless it can be presumed
     * a whole message is on the way.
     * TODO NET this definition is due to how SDL Net handles sockets.. see if it can
     *  be improved - ideally this function shouldn't block at all
     * @param source The source user.
     * @param timeout If non-zero, this method will wait this number of milliseconds
     *  for a message to arrive before returning.
     * @return The size of the message waiting if there is a message, otherwise 0.
     */
    size_t  (*msgready)(NetUserId source, unsigned timeout);

    /**
     * Completely reads a message. Blocks until entire message has been read.
     * Will not block if msgready has returned > 0.
     * @param source The source user.
     * @param buffer
     * @param max_size The maximum size of the message to be received.
     * @return The actual size of the message received, <= max_size. If 0, an
     *  error occurred.
     */
    size_t  (*readmsg)(NetUserId source, char * buffer, size_t max_size);

    /**
     * Disconnects a user.
     * @param id User to be dropped.
     */
    void (*drop_user)(NetUserId id);
};

extern const struct NetSP tcpSP;

// New Declarations End Here ==================================================

struct TbNetworkSessionNameEntry;

typedef long (*Net_Callback_Func)(void);

enum TbNetworkService {
    NS_TCP_IP,
    NS_ENET_UDP,
};

struct ClientDataEntry {
  unsigned long plyrid;
  TbBool isactive;
  TbBool has_exchanged_data;
  char name[32];
};

struct ConfigInfo {
    char str_join[20];
    char net_player_name[20];
};

struct TbNetworkPlayerInfo {
char name[32];
long active;
};

struct TbNetworkCallbackData {
  char svc_name[12];
  char plyr_name[20];
  char session_data[32];
};

struct TbNetworkPlayerName {
  char name[20];
};

struct TbNetworkPlayerNameEntry {
  unsigned char id;
  unsigned long islocal;
  unsigned long ishost;
  unsigned long is_active;
  char name[19];
  unsigned char player_extra_data[20];
  unsigned char reserved_padding[4];
};

//TODO: find out what this struct really is, and how long is it
struct SystemUserMsg {
  unsigned char type;
  struct ClientDataEntry client_data_table[CLIENT_TABLE_LEN];
};

struct UnidirectionalDataMessage {
  unsigned long message_header;
  unsigned long message_type;
  unsigned long data_length;
  unsigned long sequence_number;
  unsigned long timestamp;
  unsigned char payload_data[492];
  unsigned char message_footer[12];
};

struct UnidirectionalRTSMessage {
  unsigned long message_header;
  unsigned long message_type;
  unsigned long data_length;
  unsigned long sequence_number;
  unsigned long timestamp;
};

/** Structure for storing network service configuration. Used to pass information about configuration into LbNetwork_Init().
 */
struct ServiceInitData {
long service_flags;
    long max_connections;
    long buffer_size;
    long timeout_value;
};

/******************************************************************************/

#pragma pack()
/******************************************************************************/
void    LbNetwork_SetServerPort(int port);
void    LbNetwork_InitSessionsFromCmdLine(const char * str);
TbError LbNetwork_Init(unsigned long srvcindex, unsigned long maxplayrs, struct TbNetworkPlayerInfo *locplayr, struct ServiceInitData *init_data);
TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *playr_name, int32_t *playr_num, void *optns);
TbError LbNetwork_Create(char *nsname_str, char *plyr_name, uint32_t *plyr_num, void *optns);
TbError LbNetwork_EnableNewPlayers(TbBool allow);
TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *user_data);
TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *user_data);
TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr);
TbError LbNetwork_Stop(void);
void    LbNetwork_UpdateInputLagIfHost(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
