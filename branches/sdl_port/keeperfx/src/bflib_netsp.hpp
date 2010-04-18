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
  unsigned char field_0;
  unsigned long id;
  unsigned long in_use;
  char text[SESSION_NAME_MAX_LEN];
  unsigned char field_29[20];
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

class ServiceProvider {
private:
	unsigned long nextSessionId;
protected:
	TbError Initialise(struct ReceiveCallbacks *nCallbacks, void *a2);

	//session management
	TbNetworkSessionNameEntry *AddSession(unsigned long sess_id, const char *namestr);
	void ClearSessions(void);
	long SessionIndex(unsigned long sess_id);
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
  long PlayerIndex(unsigned long plyr_id);
  TbError AddPlayer(unsigned long plyr_id, const char *namestr, unsigned long a3, unsigned long a4);
  TbError DeletePlayer(unsigned long plyr_id);
  void ClearPlayers(void);
  TbError EnumeratePlayers(TbNetworkCallbackFunc callback, void *a2);
  unsigned long GetAddPlayerMsgSize(char *msg_str);
  void EncodeAddPlayerMsg(unsigned char *enc_buf, unsigned long id, const char *msg_str);
  TbBool DecodeAddPlayerMsg(const unsigned char *enc_buf, unsigned long &id, char *msg_str);
  TbError SystemAddPlayerHandler(const void *enc_buf);
  TbError SystemDeletePlayerHandler(const void *enc_buf);
  TbError CheckForDeletedHost(const void *enc_buf);
  TbError LookForSystemMessages(void);
  TbError BroadcastSystemMessage(void *enc_msg);
  TbError EnumeratePlayersForSessionRunning(TbNetworkCallbackFunc callback, void *);
  virtual TbError Start(struct TbNetworkSessionNameEntry *, char *, void *) = 0;
  virtual TbError Start(char *, char *, unsigned long, void *) = 0;
  virtual TbError Stop(void) = 0;
  virtual TbError Enumerate(TbNetworkCallbackFunc sessionCb, void * ptr) = 0;
  virtual TbError Enumerate(struct TbNetworkSessionNameEntry * sessionEntry, TbNetworkCallbackFunc playerCb, void * ptr) = 0;
  virtual TbError Init(struct _GUID, struct _GUID *, struct ReceiveCallbacks *, void *) = 0;
  virtual TbError Release(void);
  virtual TbError ChangeSettings(unsigned long, void *) = 0;
  virtual TbError EnableNewPlayers(TbBool allow);
  virtual unsigned long ReadMessage(unsigned long *, void *, unsigned long *) = 0;
  virtual unsigned long PeekMessage(unsigned long *, void *, unsigned long *) = 0;
  virtual TbError SendMessage(unsigned long, void *, unsigned char) = 0;

  unsigned long local_id;
  struct TbNetworkSessionNameEntry nsnames[SESSION_ENTRIES_COUNT];
  unsigned long field_7A4;
  unsigned long field_7A8;
  unsigned long players_count;
  struct TbNetworkPlayerEntry players[NETSP_PLAYERS_COUNT];
  char field_D50[32];
  struct ReceiveCallbacks *recvCallbacks;
  unsigned long field_D74;
  void *field_D78;
};

/******************************************************************************/
extern class ServiceProvider *spPtr;
/******************************************************************************/

/******************************************************************************/
#endif
