/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp.h
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
#define SESSION_ENTRIES_COUNT 32

struct TbNetworkSessionNameEntry {
  unsigned char field_0;
  unsigned long field_1;
  unsigned long field_5;
  unsigned char text[52];
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
public:
  ServiceProvider();
  virtual ~ServiceProvider();
  static unsigned long GetRequestCompositeExchangeDataMsgSize(void);
  static void EncodeMessageStub(void *buf, unsigned long a2, unsigned char a3, unsigned long a4);
  static void EncodeDeletePlayerMsg(unsigned char *buf, unsigned long val);
  static void EncodeRequestExchangeDataMsg(unsigned char *buf, unsigned long a1, unsigned long a2);
  static void EncodeRequestCompositeExchangeDataMsg(unsigned char *buf, unsigned long a1, unsigned long a2);
  static unsigned long DecodeRequestCompositeExchangeDataMsg(unsigned char *buf, unsigned long &a1);
  static void DecodeMessageStub(void *buf, unsigned long *a2, unsigned char *a3, unsigned long *a4);
  TbError Send(unsigned long a1, void *a2);
  TbError Receive(unsigned long a1);
  TbError Initialise(struct ReceiveCallbacks *nCallbacks, void *a2);
  TbError AddPlayer(unsigned long a1, char *a2, unsigned long a3, unsigned long a4);
  TbError DeletePlayer(unsigned long a1);
  void ClearPlayers(void);
  struct TbNetworkSessionNameEntry *AddSession(unsigned long a1, char *a2);
  void ClearSessions(void);
  TbError EnumeratePlayers(TbNetworkCallbackFunc callback, void *a2);
  unsigned long GetAddPlayerMsgSize(char *a1);
  void EncodeAddPlayerMsg(char *a1, unsigned long a2, char *a3);
  unsigned long DecodeAddPlayerMsg(char *a1, unsigned long &a2, char *a3);
  TbError SystemAddPlayerHandler(char *a1);
  TbError SystemDeletePlayerHandler(char *);
  TbError CheckForDeletedHost(unsigned char *);
  TbError LookForSystemMessages(void);
  TbError BroadcastSystemMessage(char *);
  TbError EnumeratePlayersForSessionRunning(TbNetworkCallbackFunc callback, void *);
  virtual TbError Start(struct TbNetworkSessionNameEntry *, char *, void *) = 0;
  virtual TbError Start(char *, char *, unsigned long, void *) = 0;
  virtual TbError Stop(void) = 0;
  virtual TbError Enumerate(TbNetworkCallbackFunc, void *) = 0;
  virtual TbError Enumerate(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc, void *) = 0;
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
  unsigned long field_7AC;
  unsigned char field_7B0[1440];
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
