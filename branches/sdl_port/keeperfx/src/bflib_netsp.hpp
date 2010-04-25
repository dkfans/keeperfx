/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp.hpp
 *     Header file for bflib_netsp.cpp.
 * @par Purpose:
 *     Network ServiceProvider class declaration.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     08 Mar 2009 - 09 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_NETSP_H
#define BFLIB_NETSP_H

#include <basetyps.h>
#include "bflib_basics.h"

#include "globals.h"
/******************************************************************************/
#define NETSP_PLAYERS_COUNT 32
#define SESSION_ENTRIES_COUNT 32
#define SESSION_NAME_MAX_LEN  32
#define NETSP_PLAYER_NAME_MAX_LEN 32

struct TbNetworkSessionNameEntry {
  unsigned char joinable; //possibly active or selected is better name
  unsigned long id;
  unsigned long in_use;
  char text[SESSION_NAME_MAX_LEN];
  unsigned char field_29[20]; //does not appear to be a string
};

struct TbNetworkPlayerEntry {
  unsigned char field_0;
  unsigned long id;
  unsigned long field_5;
  unsigned long field_9;
  char name[32];
};

struct ReceiveCallbacks {
  void (*addMsg)(unsigned long, char *, void *);
  void (*deleteMsg)(unsigned long, void *);
  void (*hostMsg)(unsigned long, void *);
  void (*sysMsg)(void *);
  void *(*multiPlayer)(unsigned long, unsigned long, unsigned long, void *);
  void (*mpReqExDataMsg)(unsigned long, unsigned long, void *);
  void (*mpReqCompsExDataMsg)(unsigned long, unsigned long, void *);
  void *(*unidirectionalMsg)(unsigned long, unsigned long, void *);
  void (*systemUserMsg)(unsigned long, void *, unsigned long, void *);
  void *(*field_24)(unsigned long, void *);
};

enum NetMsgType
{
	NETMSGTYPE_MULTIPLAYER		= 0,
	NETMSGTYPE_ADD				= 1,
	NETMSGTYPE_DELETE			= 2,
	NETMSGTYPE_PROBABLYHOST		= 3, //might be incorrect
	NETMSGTYPE_SYSUSER			= 4,
	NETMSGTYPE_MPREQEXDATA		= 5,
	NETMSGTYPE_MPREQCOMPEXDATA	= 6,
	NETMSGTYPE_UNIDIRECTIONAL	= 7,
	NETMSGTYPE_UNKNOWN			= 8
};

class ServiceProvider {
private:
	unsigned long nextSessionId;
protected:
	//see if these can be moved to private later
	bool started;
	unsigned long players_count;
	struct TbNetworkPlayerEntry players[NETSP_PLAYERS_COUNT];
	unsigned long nextPlayerId;
	unsigned long localPlayerId; //local player ID

	TbError Initialise(struct ReceiveCallbacks *nCallbacks, void *a2);

	//session management
	TbNetworkSessionNameEntry *AddSession(unsigned long sess_id, const char *namestr);
	void ClearSessions(void);
	long SessionIndex(unsigned long sess_id);

	TbError EnumeratePlayers(TbNetworkCallbackFunc callback, void *a2);
	long PlayerIndex(unsigned long plyr_id);
	TbError AddPlayer(unsigned long plyr_id, const char *namestr, unsigned long a3, unsigned long a4);
	TbError DeletePlayer(unsigned long plyr_id);
	void ClearPlayers(void);

	/**
	 * Reads a message from some player.
	 * @param playerId Holds the player that the message was sent from on return.
	 * @param msgBuffer The buffer of the message. This must be at least the initial value of len bytes.
	 * @param len The initial value specifies the maximum size of the message that may be received. It is
	 * modified to contain the actual length of the message (including header).
	 * @return True if a message was read (which implies there may be more to read).
	 */
	virtual bool ReadMessage(unsigned long * playerId, void * msgBuffer, unsigned long * len) = 0;

	/**
	 * Same as ReadMessage but does not remove the received message (if any), which means it can still be Read.
	 */
	virtual bool PeekMessage(unsigned long * playerId, void * msgBuffer, unsigned long * len) = 0;

	/**
	 * Sends a message to a specific player.
	 * @param playerId The player that the message should be sent to.
	 * @param msgBuffer The buffer of the message.
	 * @param Not sure... Anyway, it can be deduced from message type.
	 * @return Whether operation was a success or a failure.
	 */
	virtual TbError SendMessage(unsigned long playerId, void * msgBuffer, unsigned char) = 0;
public:
  ServiceProvider();
  virtual ~ServiceProvider();
  static unsigned long GetRequestCompositeExchangeDataMsgSize(void);
  static void EncodeMessageStub(void *enc_msg, unsigned long a2, unsigned char a3, unsigned long a4);
  static void EncodeDeletePlayerMsg(unsigned char *buf, unsigned long val);
  static void EncodeRequestExchangeDataMsg(unsigned char *buf, unsigned long a1, unsigned long a2);
  static void EncodeRequestCompositeExchangeDataMsg(unsigned char *buf, unsigned long a1, unsigned long a2);
  static unsigned long DecodeRequestCompositeExchangeDataMsg(unsigned char *buf, unsigned long &a1);
  static void DecodeMessageStub(const void *enc_msg, unsigned long *a2, unsigned char *a3, unsigned long *a4);
  TbError Send(unsigned long a1, void *a2);
  TbError Receive(unsigned long a1);
  unsigned long GetAddPlayerMsgSize(char *msg_str);
  void EncodeAddPlayerMsg(unsigned char *enc_buf, unsigned long id, const char *msg_str);
  TbBool DecodeAddPlayerMsg(const unsigned char *enc_buf, unsigned long &id, char *msg_str);
  TbError SystemAddPlayerHandler(const void *enc_buf);
  TbError SystemDeletePlayerHandler(const void *enc_buf);
  TbError CheckForDeletedHost(const void *enc_buf);
  TbError LookForSystemMessages(void);
  TbError BroadcastSystemMessage(void *enc_msg);
  TbError EnumeratePlayersForSessionRunning(TbNetworkCallbackFunc callback, void *);
  virtual TbError EnableNewPlayers(TbBool allow);
  virtual TbError Start(struct TbNetworkSessionNameEntry *, char *, void *) = 0;
  virtual TbError Start(char *, char *, unsigned long, void *) = 0;
  virtual TbError Stop(void) = 0;
  virtual TbError Enumerate(TbNetworkCallbackFunc sessionCb, void * ptr) = 0;
  virtual TbError Enumerate(struct TbNetworkSessionNameEntry * sessionEntry, TbNetworkCallbackFunc playerCb, void * ptr) = 0;
  virtual TbError Init(struct _GUID, struct _GUID *, struct ReceiveCallbacks *, void *) = 0;
  virtual TbError Release(void);
  virtual TbError ChangeSettings(unsigned long, void *) = 0;
  virtual void tick() = 0; //in case SP needs execution time once per frame

  struct TbNetworkSessionNameEntry nsnames[SESSION_ENTRIES_COUNT];
  unsigned long field_7A4;
  unsigned long field_7A8;
  char field_D50[32];
  struct ReceiveCallbacks *recvCallbacks;
  void *field_D78;
};

/******************************************************************************/
extern class ServiceProvider *spPtr;
/******************************************************************************/

void net_copy_name_string(char *dst,const char *src,long max_len);

/******************************************************************************/
#endif
