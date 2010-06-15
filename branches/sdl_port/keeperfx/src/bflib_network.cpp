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
 * @author   Tomasz Lis
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
#include "bflib_netsp.hpp"
#include "bflib_netsp_ipx.hpp"
#include "bflib_netsp_tcp.hpp"
#include "globals.h"

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
TbError GenericSerialInit(struct _GUID guid, void *init_data);
TbError GenericModemInit(struct _GUID guid, void *init_data);
TbError GenericIPXInit(struct _GUID guid);
static TbError genericTCPInit(struct _GUID guid);
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

#define IS_HOST	(hostId == localPlayerId)
/******************************************************************************/

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

TbError LbNetwork_Init(unsigned long srvcIndex,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data)
{
  TbError res;
  //return _DK_LbNetwork_Init(srvcp,guid,maxplayrs,exchng_buf,exchng_size,locplayr,init_data);
  exchangeSize = exchng_size;
  maximumPlayers = maxplayrs;
  //thread_data_mem = _wint_thread_data;
  basicTimeout = 250;
  localDataPtr = 0;
  compositeBuffer = 0;
  sequenceNumber = 0;
  timeCount = 0;
  maxTime = 0;
  runningTwoPlayerModel = false;
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
  GetPlayerInfo();
  // Initialising the service provider object
  switch (srvcIndex)
  {
  case NS_Serial:
      NETMSG("Selecting Serial SP");
      if (GenericSerialInit(guid,init_data) == Lb_OK)
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
      if (GenericModemInit(guid,init_data) == Lb_OK)
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
      if (GenericIPXInit(guid) == Lb_OK)
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
	  if (genericTCPInit(guid) == Lb_OK) {
		  res = Lb_OK;
	  }
	  else {
		  WARNLOG("Failure on TCP/IP Initialization");
		  res = Lb_FAIL;
	  }
	  break;
  default:
      WARNLOG("The serviceIndex value of %d is out of range", srvcIndex);
      res = Lb_FAIL;
      break;
  }
  //_wint_thread_data = thread_data_mem;
  return res;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, unsigned long *plyr_num, void *optns)
{
  TbError ret;
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
  }
  else {
	localPlayerId = PLAYERID_UNASSIGNED;
  }
  sequenceNumber = 15;
  if (spPtr->Start(nsname, plyr_name, optns))
  {
    NETMSG("Failure on Join");
    return Lb_FAIL;
  }
  if (!runningTwoPlayerModel)
  {
    spPtr->EncodeMessageStub(&systemUserBuffer, 1, NETMSGTYPE_SYSUSER, runningTwoPlayerModel);
    systemUserBuffer[4] = 0;
    spPtr->Send(PLAYERID_HOSTDEFAULT, systemUserBuffer);
    waitingForPlayerMapResponse = 1;
    while (waitingForPlayerMapResponse)
    {
      spPtr->Receive(8);
      if ( waitingForPlayerMapResponse )
      {
        if (LbTimerClock()-tmStart > 10000)
        {
          waitingForPlayerMapResponse = 0;
          //exits due to time out
          return ret;
        }
      }
    }
  }

  //we have received "player map" = clientDataTable
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
  }
  return Lb_OK;
}

TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num, void *optns)
{

  //return _DK_LbNetwork_Create(nsname_str, plyr_name, plyr_num);
  if (spPtr == NULL)
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
  }
  else {
	localPlayerId = PLAYERID_HOSTDEFAULT;
	localPlayerIndex = 0;
	hostId = PLAYERID_HOSTDEFAULT;
  }
  if (spPtr->Start(nsname_str, plyr_name, maximumPlayers, optns) != Lb_OK)
  {
    WARNLOG("Failure on SP::Start()");
    return Lb_FAIL;
  }
  if (GetCurrentPlayers() != Lb_OK)
  {
    WARNLOG("Cannot get current players");
    return Lb_FAIL;
  }
  *plyr_num = localPlayerIndex;
  if (GetPlayerInfo() != Lb_OK)
  {
    WARNLOG("Cannot get player info");
    return Lb_FAIL;
  }
  LbNetwork_EnableNewPlayers(true);
  return Lb_OK;
}

TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long buf_size)
{
  void *cbuf;
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
  compositeBufferSize = comps_size;
  return Lb_OK;
}

void LbNetwork_ChangeExchangeTimeout(unsigned long tmout)
{
  exchangeTimeout = 1000 * tmout;
}

TbError LbNetwork_Stop(void)
{
  //return _DK_LbNetwork_Stop();
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
  exchangeTimeout = 0;
  return Lb_OK;
}

TbError LbNetwork_Exchange(void *buf)
{
  SYNCDBG(7, "Starting");
  //return _DK_LbNetwork_Exchange(buf);
  spPtr->update();

  if (LbNetwork_StartExchange(buf) != Lb_OK)
  {
    WARNLOG("Failure when Starting Exchange");
    return Lb_FAIL;
  }
  if (LbNetwork_CompleteExchange(buf) != Lb_OK)
  {
    WARNLOG("Failure when Completing Exchange");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow)
{
  //return _DK_LbNetwork_EnableNewPlayers(allow);
  if (spPtr == NULL)
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
  }
}

TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *ptr)
{
//  TbBool local_init;
  struct TbNetworkCallbackData netcdat;
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
  char ret;
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
  }
  return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr)
{
  char ret;
  //return _DK_LbNetwork_EnumerateSessions(callback, ptr);
  if (spPtr == NULL)
  {
    ERRORLOG("ServiceProvider ptr is NULL");
    return Lb_FAIL;
  }
  ret = spPtr->Enumerate(callback, ptr);
  if (ret != Lb_OK)
  {
    WARNLOG("Failure on Enumerate");
    return ret;
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
    clientDataTable[i].playerActive = 0;
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

void __stdcall GetCurrentPlayersCallback(struct TbNetworkCallbackData *netcdat, void * ptr)
{
	SYNCDBG(9, "Starting");
	AddAPlayer((struct TbNetworkPlayerNameEntry *)netcdat);
}

TbError GetPlayerInfo(void)
{
  struct ClientDataEntry  *clidat;
  struct TbNetworkPlayerInfo *lpinfo;
  long i;
  NETLOG("Starting");
  for (i=0; i < maximumPlayers; i++)
  {
    clidat = &clientDataTable[i];
    lpinfo = &localPlayerInfoPtr[i];
    if ( clidat->playerActive )
    {
      lpinfo->active = 1;
      strncpy(lpinfo->name, clidat->playerName, 32);
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
  unsigned long playerIndex;
  long i;

  SYNCDBG(7, "Starting");

  if (plyrname == NULL)
  {
	ERRORLOG("Got null player");
    return Lb_FAIL;
  }
  playerIndex = 0;
  found_id = false;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].playerActive) && (clientDataTable[i].playerId == plyrname->id))
    {
      found_id = true;
      playerIndex = i;
    }
  }
  if (!found_id)
  {
    found_id = false;
    for (i=0; i < maximumPlayers; i++)
    {
      if (!clientDataTable[i].playerActive)
      {
        found_id = true;
        playerIndex = i;
      }
    }
    if (found_id)
    {
      clientDataTable[playerIndex].playerId = plyrname->id;
      clientDataTable[playerIndex].playerActive = 1;
      strcpy(clientDataTable[playerIndex].playerName,plyrname->name);
      localPlayerInfoPtr[i].active = 1;
      strcpy(localPlayerInfoPtr[i].name,plyrname->name);
    }
  }
  if (!found_id)
  {
    return Lb_FAIL;
  }
  if (plyrname->isHost)
    hostId = plyrname->id;
  if (plyrname->isLocal)
  {
    localPlayerId = plyrname->id;
    localPlayerIndex = playerIndex;
  }
  return Lb_OK;
}

TbError GenericSerialInit(struct _GUID guid, void *init_data)
{
  struct SerialInitData *sp_init;
  if (spPtr != NULL)
  {
    spPtr->Release();
    delete spPtr;
    spPtr = NULL;
  }
  sp_init = (struct SerialInitData *)init_data;
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
  //TODO: set when SerialSP is ready
  spPtr = NULL;//new SerialSP(...);
  if (spPtr == NULL)
  {
    WARNLOG("Failure on SP construction");
    return Lb_FAIL;
  }
  if (spPtr->Init(guid, 0, &receiveCallbacks, 0) != Lb_OK)
  {
    WARNLOG("Failure on SP::Init()");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError GenericModemInit(struct _GUID guid, void *init_data)
{
  struct SerialInitData *sp_init;
  if (spPtr != NULL)
  {
    spPtr->Release();
    delete spPtr;
    spPtr = NULL;
  }
  sp_init = (struct SerialInitData *)init_data;
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
//TODO: set when ModemSP is ready
  spPtr = NULL;//new ModemSP(...);
  if (spPtr == NULL)
  {
    WARNLOG("Failure on SP construction");
    return Lb_FAIL;
  }
  if (spPtr->Init(guid, 0, &receiveCallbacks, 0) != Lb_OK)
  {
    WARNLOG("Failure on SP::Init()");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError GenericIPXInit(struct _GUID guid)
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
  if (spPtr->Init(guid, 0, &receiveCallbacks, 0) != Lb_OK)
  {
    WARNLOG("Failure on SP::Init()");
    return Lb_FAIL;
  }
  return Lb_OK;
}

static TbError genericTCPInit(struct _GUID guid)
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
	if (spPtr->Init(guid, 0, &receiveCallbacks, 0) != Lb_OK) {
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
    if ((clientDataTable[i].playerActive) && (!clientDataTable[i].field_8))
    {
      if (spPtr->Send(clientDataTable[i].playerId,requestExchangeDataBuffer))
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
  long i,k;
  ret = Lb_FAIL;

  keepExchng = true;
  nRetries = 0;
  while ( keepExchng )
  {
    exchngNeeded = 1;
    for (i=0; i < maximumPlayers; i++)
    {
      if ((clientDataTable[i].playerActive) && (!clientDataTable[i].field_8))
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
            if ((clientDataTable[i].playerActive) && (!clientDataTable[i].field_8))
            {
              spPtr->EncodeDeletePlayerMsg(deletePlayerBuffer, clientDataTable[i].playerId);
              for (k=0; k < maximumPlayers; k++)
              {
                if ((clientDataTable[k].playerActive) && (clientDataTable[k].playerId != clientDataTable[i].playerId))
                {
                  if ( spPtr->Send(clientDataTable[i].playerId,deletePlayerBuffer) )
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
    if ((clientDataTable[i].playerActive) && (clientDataTable[i].playerId != hostId))
    {
      if ( spPtr->Send(clientDataTable[i].playerId, exchangeBuffer) )
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
  if (!clientDataTable[remotePlayerIndex].playerActive)
    spPtr->Receive(2);
  gotCompositeData = 0;
  if (clientDataTable[remotePlayerIndex].playerActive)
  {
    spPtr->Receive(8);
    spPtr->Receive(16);
  }
  memcpy((uchar *)exchangeBuffer + exchangeSize * localPlayerIndex, buf, exchangeSize);
  if (clientDataTable[remotePlayerIndex].playerActive)
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
    if (clidat->playerActive)
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
  if (!clientDataTable[remotePlayerIndex].playerActive )
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
    if (!clientDataTable[remotePlayerIndex].playerActive)
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

TbError SendSystemUserMessage(unsigned long plr_id, int type, void *ibuf, unsigned long ibuf_len)
{
  if (ibuf_len+5 > sizeof(systemUserBuffer))
  {
    WARNLOG("systemUserBuffer is too small");
    return Lb_FAIL;
  }
  spPtr->EncodeMessageStub(systemUserBuffer, ibuf_len+1, 4, 0);
  systemUserBuffer[4] = type;
  if ((ibuf != NULL) && (ibuf_len > 0))
  {
    memcpy(&systemUserBuffer[5], ibuf, ibuf_len);
  }
  return spPtr->Send(plr_id, systemUserBuffer);
}

void PlayerMapMsgHandler(unsigned long plr_id, void *msg, unsigned long msg_len)
{
  unsigned long len = sizeof(struct ClientDataEntry)*maximumPlayers;
  if (msg_len == 0 && IS_HOST)
  {
    SendSystemUserMessage(PLAYERID_ALLBUTSELF, 0, clientDataTable, len);
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
      if ((clientDataTable[i].playerActive) && (clientDataTable[i].playerId == plr_id))
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
      if ((clientDataTable[i].playerActive) && (clientDataTable[i].playerId == plr_id))
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
    if ((clientDataTable[i].playerActive) && (clientDataTable[i].playerId == plr_id))
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

void AddMsgCallback(unsigned long playerId, char *nmstr, void *a3)
{
  struct TbNetworkPlayerNameEntry npname;
  npname.id = playerId;
  strcpy(npname.name,nmstr);
  npname.isLocal = 0;
  npname.isHost = 0;
  AddAPlayer(&npname);
}

void DeleteMsgCallback(unsigned long plr_id, void *a2)
{
  long i;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].playerActive) && (clientDataTable[i].playerId == plr_id))
    {
      clientDataTable[i].playerActive = 0;
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

void SystemUserMsgCallback(unsigned long plr_id, void *msgdata, unsigned long msglen, void *a4)
{
  struct SystemUserMsg *msg;
  msg = (struct SystemUserMsg *) msgdata;
  if ((msgdata = NULL) || (msglen <= 0))
    return;
  if (msg->type != 0)
  {
    WARNLOG("Illegal sysMsgType %d",msg->type);
  }
  PlayerMapMsgHandler(plr_id, msg->clientDataTable, msglen-1);
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
