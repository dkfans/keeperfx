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
#include "bflib_netsp.h"
#include "bflib_netsp_ipx.h"
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
  static const char *func_name="LbNetwork_Init";
  struct TbNetworkPlayerInfo *lpinfo;
  TbError res;
  long i;
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
    LbWarnLog("%s: failure on buffer allocation\n",func_name);
    //_wint_thread_data = thread_data_mem;
    return Lb_FAIL;
  }
  ClearClientData();
  GetPlayerInfo();
  // Initialising the service provider object
  switch (srvcIndex)
  {
  case 0:
      LbNetLog("Selecting Serial SP\n");
      if (GenericSerialInit(guid,init_data) == Lb_OK)
      {
        res = Lb_OK;
      } else
      {
        LbWarnLog("%s: failure on Serial Initialization\n",func_name);
        res = Lb_FAIL;
      }
      break;
  case 1:
      LbNetLog("Selecting Modem SP\n");
      if (GenericModemInit(guid,init_data) == Lb_OK)
      {
        res = Lb_OK;
      } else
      {
        LbWarnLog("%s: failure on Modem Initialization\n",func_name);
        res = Lb_FAIL;
      }
      break;
  case 2:
      LbNetLog("Selecting IPX SP\n");
      if (GenericIPXInit(guid) == Lb_OK)
      {
        res = Lb_OK;
      } else
      {
        LbWarnLog("%s: failure on IPX Initialization\n",func_name);
        res = Lb_FAIL;
      }
      break;
  default:
      LbWarnLog("The serviceIndex value of %d is out of range\n", srvcIndex);
      res = Lb_FAIL;
      break;
  }
  //_wint_thread_data = thread_data_mem;
  return res;
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, unsigned long *plyr_num, void *optns)
{
  static const char *func_name="LbNetwork_Join";
  TbError ret;
  TbClockMSec tmStart;
  //return _DK_LbNetwork_Join(nsname, plyr_name, plyr_num);
  ret = Lb_FAIL;
  tmStart = LbTimerClock();
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  if (runningTwoPlayerModel)
  {
    remotePlayerId = 0;
    remotePlayerIndex = 0;
    localPlayerId = 1;
    localPlayerIndex = 1;
  }
  sequenceNumber = 15;
  if (spPtr->Start(nsname, plyr_name, optns))
  {
    LbWarnLog("%s: failure on Join\n",func_name);
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
    LbWarnLog("%s: cannot get current players\n",func_name);
    return ret;
  }
  *plyr_num = localPlayerIndex;
  ret = GetPlayerInfo();
  if (ret != Lb_OK)
  {
    LbWarnLog("%s: cannot get player info\n",func_name);
    return ret;
  }
  return Lb_OK;
}

TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num, void *optns)
{
  static const char *func_name="LbNetwork_Create";
  //return _DK_LbNetwork_Create(nsname_str, plyr_name, plyr_num);
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  if ( runningTwoPlayerModel )
  {
    localPlayerId = 0;
    localPlayerIndex = 0;
    remotePlayerId = 1;
    remotePlayerIndex = 1;
  }
  if (spPtr->Start(nsname_str, plyr_name, maximumPlayers, optns) != Lb_OK)
  {
    LbWarnLog("%s: failure on SP::Start()\n",func_name);
    return Lb_FAIL;
  }
  if (GetCurrentPlayers() != Lb_OK)
  {
    LbWarnLog("%s: cannot get current players\n",func_name);
    return Lb_FAIL;
  }
  if (GetPlayerInfo() != Lb_OK)
  {
    LbWarnLog("%s: cannot get player info\n",func_name);
    return Lb_FAIL;
  }
  LbNetwork_EnableNewPlayers(true);
  return Lb_OK;
}

TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long buf_size)
{
  static const char *func_name="LbNetwork_ChangeExchangeBuffer";
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
      LbWarnLog("%s: failure on buffer allocation\n",func_name);
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
    LbWarnLog("%s: failure on buffer reallocation\n",func_name);
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
  static const char *func_name="LbNetwork_Stop";
  //return _DK_LbNetwork_Stop();
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  if (spPtr->Release())
    LbWarnLog("%s: failure on Release\n",func_name);
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
  static const char *func_name="LbNetwork_Exchange";
  LbNetLog("%s: Starting\n",func_name);
  //return _DK_LbNetwork_Exchange(buf);
  if (LbNetwork_StartExchange(buf) != Lb_OK)
  {
    LbWarnLog("%s failure when Starting Exchange\n",func_name);
    return Lb_FAIL;
  }
  if (LbNetwork_CompleteExchange(buf) != Lb_OK)
  {
    LbWarnLog("%s: failure when Completing Exchange\n",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow)
{
  static const char *func_name="LbNetwork_EnableNewPlayers";
  //return _DK_LbNetwork_EnableNewPlayers(allow);
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  if (allow)
  {
    LbNetLog("New players ARE allowed to join\n");
    return spPtr->EnableNewPlayers(true);
  } else
  {
    LbNetLog("New players are NOT allowed to join\n");
    return spPtr->EnableNewPlayers(false);
  }
}

TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *ptr)
{
  static const char *func_name="LbNetwork_EnumerateServices";
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
    LbNetLog("Enumerate Services called\n");
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
  LbNetLog("Enumerate Services called\n");
  return Lb_OK;
}

/*
 * Enumerates network players.
 * @return Returns Lb_OK on success, Lb_FAIL on error.
 */
TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *buf)
{
  static const char *func_name="LbNetwork_EnumeratePlayers";
  char ret;
  //return _DK_LbNetwork_EnumeratePlayers(sesn, callback, a2);
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  ret = spPtr->Enumerate(sesn, callback, buf);
  if (ret != Lb_OK)
  {
    LbWarnLog("%s: failure on Enumerate\n",func_name);
    return ret;
  }
  return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr)
{
  static const char *func_name="LbNetwork_EnumerateSessions";
  char ret;
  //return _DK_LbNetwork_EnumerateSessions(callback, ptr);
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  ret = spPtr->Enumerate(callback, ptr);
  if (ret != Lb_OK)
  {
    LbWarnLog("%s: failure on Enumerate\n",func_name);
    return ret;
  }
  return Lb_OK;
}

TbError LbNetwork_StartExchange(void *buf)
{
  static const char *func_name="LbNetwork_StartExchange";
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  if (runningTwoPlayerModel)
    return StartTwoPlayerExchange(buf);
  else
    return StartMultiPlayerExchange(buf);
}

TbError LbNetwork_CompleteExchange(void *buf)
{
  static const char *func_name="LbNetwork_CompleteExchange";
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  if ( runningTwoPlayerModel )
    return CompleteTwoPlayerExchange(buf);
  else
    return CompleteMultiPlayerExchange(buf);
}

TbError ClearClientData(void)
{
  static const char *func_name="ClearClientData";
  long i;
  LbMemorySet(clientDataTable, 0, 32*sizeof(struct ClientDataEntry));
  for (i=0; i < maximumPlayers; i++)
  {
    clientDataTable[i].field_4 = 0;
  }
}  

TbError GetCurrentPlayers(void)
{
  static const char *func_name="GetCurrentPlayers";
  if (spPtr == NULL)
  {
    LbErrorLog("%s: ServiceProvider ptr is NULL\n",func_name);
    return Lb_FAIL;
  }
  LbNetLog("%s: Starting\n",func_name);
  localPlayerIndex = maximumPlayers;
  if (spPtr->Enumerate(0, GetCurrentPlayersCallback, 0))
  {
    LbWarnLog("%s: failure on SP::Enumerate()\n",func_name);
    return Lb_FAIL;
  }
  if (localPlayerIndex >= maximumPlayers)
  {
    LbWarnLog("%s: localPlayerIndex not updated, max players %d\n",func_name,maximumPlayers);
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
  static const char *func_name="GetPlayerInfo";
  struct ClientDataEntry  *clidat;
  struct TbNetworkPlayerInfo *lpinfo;
  long i;
  LbNetLog("%s: Starting\n",func_name);
  for (i=0; i < maximumPlayers; i++)
  {
    clidat = &clientDataTable[i];
    lpinfo = &localPlayerInfoPtr[i];
    if ( clidat->field_4 )
    {
      lpinfo->field_20 = 1;
      strncpy(lpinfo->field_0, clidat->field_C, 32);
    } else
    {
      lpinfo->field_20 = 0;
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
  found_id = false;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].field_4) && (clientDataTable[i].field_0 == plyrname->field_1))
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
      if (clientDataTable[i].field_4)
      {
        found_id = true;
        plr_id = i;
      }
    }
    if (found_id)
    {
      clientDataTable[plr_id].field_0 = plyrname->field_1;
      clientDataTable[plr_id].field_4 = 1;
      strcpy(clientDataTable[plr_id].field_C,plyrname->field_D);
      localPlayerInfoPtr[i].field_20 = 1;
      strcpy(localPlayerInfoPtr[i].field_0,plyrname->field_D);
    }
  }
  if (!found_id)
  {
    return Lb_FAIL;
  }
  if (plyrname->field_9)
    hostId = plyrname->field_1;
  if (plyrname->field_5)
  {
    localPlayerId = plyrname->field_1;
    localPlayerIndex = plr_id;
  }
  return Lb_OK;
}

TbError GenericSerialInit(struct _GUID guid, void *init_data)
{
  static const char *func_name="GenericSerialInit";
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
//TODO
  spPtr = NULL;//new SerialSP(...);
  if (spPtr == NULL)
  {
    LbWarnLog("%s: failure on SP construction\n",func_name);
    return Lb_FAIL;
  }
  if (spPtr->Init(guid, 0, &receiveCallbacks, 0) != Lb_OK)
  {
    LbWarnLog("%s: failure on SP::Init()\n",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError GenericModemInit(struct _GUID guid, void *init_data)
{
  static const char *func_name="GenericSerialInit";
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
//TODO
  spPtr = NULL;//new ModemSP(...);
  if (spPtr == NULL)
  {
    LbWarnLog("%s: failure on SP construction\n",func_name);
    return Lb_FAIL;
  }
  if (spPtr->Init(guid, 0, &receiveCallbacks, 0) != Lb_OK)
  {
    LbWarnLog("%s: failure on SP::Init()\n",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError GenericIPXInit(struct _GUID guid)
{
  static const char *func_name="GenericIPXInit";
  if (spPtr != NULL)
  {
    spPtr->Release();
    delete spPtr;
    spPtr = NULL;
  }
  spPtr = new IPXServiceProvider();
  if (spPtr == NULL)
  {
    LbWarnLog("%s: failure on SP construction\n",func_name);
    return Lb_FAIL;
  }
  if (spPtr->Init(guid, 0, &receiveCallbacks, 0) != Lb_OK)
  {
    LbWarnLog("%s: failure on SP::Init()\n",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError SendRequestCompositeExchangeDataMsg(const char *func_name)
{
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(requestCompositeExchangeDataBuffer))
  {
    LbWarnLog("%s: requestCompositeExchangeDataBuffer is too small\n",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeRequestCompositeExchangeDataMsg(requestCompositeExchangeDataBuffer,localPlayerId,sequenceNumber);
  if (spPtr->Send(hostId, requestCompositeExchangeDataBuffer) != 0)
  {
    LbWarnLog("%s: failure on SP::Send()\n",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError SendRequestToAllExchangeDataMsg(unsigned long src_id,unsigned long seq, const char *func_name)
{
  long i;
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(requestExchangeDataBuffer))
  {
    LbWarnLog("%s: requestCompositeExchangeDataBuffer is too small\n",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, src_id, seq);
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].field_4) && (!clientDataTable[i].field_8))
    {
      if (spPtr->Send(clientDataTable[i].field_0,requestExchangeDataBuffer))
        LbWarnLog("%s: failure on SP::Send()\n",func_name);
    }
  }
  return Lb_OK;
}

TbError SendRequestExchangeDataMsg(unsigned long dst_id,unsigned long src_id,unsigned long seq, const char *func_name)
{
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(requestExchangeDataBuffer))
  {
    LbWarnLog("%s: requestCompositeExchangeDataBuffer is too small\n",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, src_id, seq);
  if (spPtr->Send(dst_id,requestExchangeDataBuffer))
  {
    LbWarnLog("%s: failure on SP::Send()\n",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError SendDeletePlayerMsg(unsigned long dst_id,unsigned long del_id,const char *func_name)
{
  if (spPtr->GetRequestCompositeExchangeDataMsgSize() > sizeof(deletePlayerBuffer))
  {
    LbWarnLog("%s: deletePlayerBuffer is too small\n",func_name);
    return Lb_FAIL;
  }
  spPtr->EncodeDeletePlayerMsg(deletePlayerBuffer, del_id);
  if (spPtr->Send(dst_id, deletePlayerBuffer) != Lb_OK)
  {
    LbWarnLog("%s failure on SP::Send()\n",func_name);
    return Lb_FAIL;
  }
  LbNetLog("%s: Sent delete player message\n",func_name);
  return Lb_OK;
}

TbError HostDataCollection(void)
{
  static const char *func_name="HostDataCollection";
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
      if ((clientDataTable[i].field_4) && (!clientDataTable[i].field_8))
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
      LbNetLog("Timed out waiting for client\n");
      nRetries++;
      if (nRetries <= 10)
      {
        SendRequestToAllExchangeDataMsg(hostId, sequenceNumber, func_name);
      } else
      {
        if (spPtr->GetRequestCompositeExchangeDataMsgSize() <= sizeof(deletePlayerBuffer))
        {
          for (i=0; i < maximumPlayers; i++)
          {
            if ((clientDataTable[i].field_4) && (!clientDataTable[i].field_8))
            {
              spPtr->EncodeDeletePlayerMsg(deletePlayerBuffer, clientDataTable[i].field_0);
              for (k=0; k < maximumPlayers; k++)
              {
                if ((clientDataTable[k].field_4) && (clientDataTable[k].field_0 != clientDataTable[i].field_0))
                {
                  if ( spPtr->Send(clientDataTable[i].field_0,deletePlayerBuffer) )
                    LbWarnLog("%s: failure on SP::Send()\n",func_name);
                }
              }
            }
          }
        } else
        {
          LbWarnLog("%s: deletePlayerBuffer is too small\n",func_name);
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
  static const char *func_name="HostDataBroadcast";
  TbError ret;
  long i;
  ret = Lb_OK;
  spPtr->EncodeMessageStub(exchangeBuffer, maximumPlayers*exchangeSize-4, 0, sequenceNumber);
  LbMemoryCopy(compositeBuffer, exchangeBuffer, maximumPlayers*exchangeSize);
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].field_4) && (clientDataTable[i].field_0 != hostId))
    {
      if ( spPtr->Send(clientDataTable[i].field_0, exchangeBuffer) )
      {
        LbWarnLog("%s: failure on SP::Send()\n",func_name);
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
      LbWarnLog("%s: failure on HostDataCollection()\n",func_name);
      return Lb_FAIL;
    }
    if (HostDataBroadcast() != Lb_OK)
    {
      LbWarnLog("%s: failure on HostDataBroadcast()\n",func_name);
      return Lb_FAIL;
    }
  } else
  {
    spPtr->EncodeMessageStub(buf, exchangeSize-4, 0, sequenceNumber);
    if (spPtr->Send(hostId, buf) != Lb_OK)
    {
      LbWarnLog("%s: failure on SP::Send()\n",func_name);
      return Lb_FAIL;
    }
  }
  return Lb_OK;
}

TbError StartTwoPlayerExchange(void *buf)
{
  static const char *func_name="StartTwoPlayerExchange";
  if (!clientDataTable[remotePlayerIndex].field_4)
    spPtr->Receive(2);
  gotCompositeData = 0;
  if (clientDataTable[remotePlayerIndex].field_4)
  {
    spPtr->Receive(8);
    spPtr->Receive(16);
  }
  memcpy((uchar *)exchangeBuffer + exchangeSize * localPlayerIndex, buf, exchangeSize);
  if (clientDataTable[remotePlayerIndex].field_4)
  {
    spPtr->EncodeMessageStub(buf, exchangeSize-4, 0, sequenceNumber);
    if (spPtr->Send(remotePlayerId, buf) != Lb_OK)
    {
      LbWarnLog("%s: failure on SP::Send()\n",func_name);
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
  static const char *func_name="StartMultiPlayerExchange";
  struct ClientDataEntry  *clidat;
  long i;
  localDataPtr = buf;
  spPtr->Receive(6);
  for (i=0; i < maximumPlayers; i++)
  {
    clidat = &clientDataTable[i];
    if (clidat->field_4)
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
    LbWarnLog("%s: failure on SP::Send()\n",func_name);
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError CompleteTwoPlayerExchange(void *buf)
{
  static const char *func_name="CompleteTwoPlayerExchange";
  TbError ret;
  TbBool keepExchng;
  TbClockMSec tmPassed;
  long nRetries;
  long i;
  ret = Lb_FAIL;
  keepExchng = true;
  if (!clientDataTable[remotePlayerIndex].field_4 )
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
        LbNetLog("Timed out (%d) waiting for seq %d - %d ms\n", tmPassed, sequenceNumber, actualTimeout);
        nRetries++;
        if (nRetries <= 10)
        {
          LbNetLog("Requesting %d resend of packet (%d)\n", nRetries, sequenceNumber);
          SendRequestExchangeDataMsg(remotePlayerId, localPlayerId, sequenceNumber, func_name);
        } else
        {
          LbNetLog("Tried to resend %d times, aborting\n", nRetries);
          SendDeletePlayerMsg(localPlayerId, remotePlayerId, func_name);
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
    if (!clientDataTable[remotePlayerIndex].field_4)
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
  static const char *func_name="CompleteMultiPlayerExchange";
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
        ret = SendSequenceNumber(buf,func_name);
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
      LbNetLog("Timed out waiting for host\n");
      nRetries++;
      if (nRetries <= 10)
      {
        SendRequestCompositeExchangeDataMsg(func_name);
      } else
      {
        SendDeletePlayerMsg(localPlayerId, hostId, func_name);
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
  static const char *func_name="SendSystemUserMessage";
  if (ibuf_len+5 > sizeof(systemUserBuffer))
  {
    LbWarnLog("%s: systemUserBuffer is too small\n", func_name);
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
  static const char *func_name="PlayerMapMsgHandler";
  unsigned long len;
  len = sizeof(struct ClientDataEntry)*maximumPlayers;
  if (msg_len == 0)
  {
    SendSystemUserMessage(plr_id, 0, clientDataTable, len);
    return;
  }
  if (!waitingForPlayerMapResponse)
  {
    LbWarnLog("%s: received unexpected SU_PLAYER_MAP_MSG\n",func_name);
    return;
  }
  if (msg_len != len)
  {
    LbWarnLog("%s: invalid length %d\n",func_name,msg_len);
    return;
  }
  LbMemoryCopy(clientDataTable, msg, len);
  waitingForPlayerMapResponse = 0;
}

void *MultiPlayerCallback(unsigned long plr_id, unsigned long xch_size, unsigned long seq, void *a4)
{
  static const char *func_name="MultiPlayerCallback";
  TbBool found_id;
  long i;
  if (inside_sr)
    LbNetLog("%s: Got a request\n",func_name);
  if (localPlayerId == hostId)
  {
    if (xch_size != exchangeSize)
    {
      LbWarnLog("%s: invalid length: %d\n",func_name,xch_size);
      return NULL;
    }
    if (plr_id == localPlayerId)
    {
      LbWarnLog("%s: host got data from itself\n",func_name);
      return NULL;
    }
    found_id = false;
    for (i=0; i < maximumPlayers; i++)
    {
      if ((clientDataTable[i].field_4) && (clientDataTable[i].field_0 == plr_id))
      {
        found_id = true;
        plr_id = i;
      }
    }
    if (!found_id)
    {
      LbWarnLog("%s: invalid id: %d\n",func_name,plr_id);
      return NULL;
    }
    if ((seq != sequenceNumber) && (seq != 15))
    {
      LbWarnLog("%s: Unexpected sequence number: Got %d, expected %d\n",func_name,seq,sequenceNumber);
      return NULL;
    }
    clientDataTable[plr_id].field_8 = 1;
    return (uchar *)exchangeBuffer + plr_id * exchangeSize;
  }
  if (xch_size != maximumPlayers * exchangeSize)
  {
    if (xch_size != exchangeSize)
    {
      LbWarnLog("%s: invalid length: %d\n",func_name,xch_size);
      return NULL;
    }
    if (plr_id == localPlayerId)
    {
      LbWarnLog("%s: client acting as host got data from itself\n",func_name);
      return NULL;
    }
    found_id = false;
    for (i=0; i < maximumPlayers; i++)
    {
      if ((clientDataTable[i].field_4) && (clientDataTable[i].field_0 == plr_id))
      {
        found_id = true;
        plr_id = i;
      }
    }
    if (!found_id)
    {
      LbWarnLog("%s: invalid id: %d\n",func_name,plr_id);
      return NULL;
    }
    clientDataTable[plr_id].field_8 = 1;
    return (uchar *)exchangeBuffer + plr_id * exchangeSize;
  }
  if (hostId != plr_id)
  {
    LbWarnLog("%s: data is not from host\n",func_name);
    return NULL;
  }
  found_id = false;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].field_4) && (clientDataTable[i].field_0 == plr_id))
    {
      found_id = true;
      plr_id = i;
    }
  }
  if (!found_id)
  {
    LbWarnLog("%s: invalid id: %d\n",func_name,plr_id);
    return 0;
  }
  if (sequenceNumber == 15)
  {
    sequenceNumber = seq;
  } else
  if (sequenceNumber != seq)
  {
    LbWarnLog("%s: Unexpected sequence number: Got %d, expected %d\n", seq, sequenceNumber);
    return NULL;
  }
  gotCompositeData = true;
  return exchangeBuffer;
}

void MultiPlayerReqExDataMsgCallback(unsigned long plr_id, unsigned long seq, void *a3)
{
  static const char *func_name="MultiPlayerReqExDataMsgCallback";
  if (inside_sr)
    LbNetLog("%s: Got a request\n",func_name);
  if (localDataPtr == NULL)
  {
    LbWarnLog("%s: NULL data pointer\n",func_name);
    return;
  }
  if (sequenceNumber == 15)
    sequenceNumber = seq;
  if (seq != sequenceNumber)
  {
    LbWarnLog("%s: unexpected sequence number, got %d, expected %d\n",func_name,seq,sequenceNumber);
    return;
  }
  spPtr->EncodeMessageStub(localDataPtr, exchangeSize-4, 0, sequenceNumber);
  if (spPtr->Send(plr_id, localDataPtr) != Lb_OK)
  {
    LbWarnLog("%s: failure on SP::Send()\n",func_name);
  }
}

void AddMsgCallback(unsigned long a1, char *nmstr, void *a3)
{
  struct TbNetworkPlayerNameEntry npname;
  npname.field_1 = a1;
  strcpy(npname.field_D,nmstr);
  npname.field_5 = 0;
  npname.field_9 = 0;
  AddAPlayer(&npname);
}

void DeleteMsgCallback(unsigned long plr_id, void *a2)
{
  static const char *func_name="DeleteMsgCallback";
  long i;
  for (i=0; i < maximumPlayers; i++)
  {
    if ((clientDataTable[i].field_4) && (clientDataTable[i].field_0 == plr_id))
    {
      clientDataTable[i].field_4 = 0;
      if (localPlayerInfoPtr != NULL)
      {
        localPlayerInfoPtr[i].field_20 = 0;
      } else
      {
        LbWarnLog("%s: NULL localPlayerInfoPtr\n",func_name);
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
  static const char *func_name="RequestCompositeExchangeDataMsgCallback";
  if (inside_sr)
    LbNetLog("%s: Got sequence %d, expected %d\n",func_name,seq,sequenceNumber);
  if ((seq != sequenceNumber) && (seq != ((sequenceNumber - 1) & 0xF)))
  {
    LbWarnLog("%s: unexpected sequence number, got %d, expected %d\n",func_name,seq,sequenceNumber);
    return;
  }
  if (spPtr->Send(plr_id, compositeBuffer) != Lb_OK)
  {
    LbWarnLog("%s: failure on SP::Send()\n",func_name);
    return;
  }
}

void *UnidirectionalMsgCallback(unsigned long a1, unsigned long msg_len, void *a3)
{
  static const char *func_name="UnidirectionalMsgCallback";
  if (msg_len > 524)
  {
    LbWarnLog("%s: invalid length, %d vs %d\n", msg_len, 524);
    return NULL;
  }
  unidirectionalMsgReceived = 1;
  return &incomingUnidirectionalMessage;
}

void SystemUserMsgCallback(unsigned long plr_id, void *msgbuf, unsigned long msglen, void *a4)
{
  static const char *func_name="SystemUserMsgCallback";
  struct SystemUserMsg *msg;
  msg = (struct SystemUserMsg *)msgbuf;
  if ((msgbuf = NULL) || (msglen <= 0))
    return;
  if (msg->field_0)
  {
    LbWarnLog("%s: illegal sysMsgType %d\n",func_name,msg->field_0);
  }
  PlayerMapMsgHandler(plr_id, msg->field_1, msglen-1);
}

void TwoPlayerReqExDataMsgCallback(unsigned long, unsigned long, void *)
{
//TODO (less importand - used only for modem and serial)
}

void *TwoPlayerCallback(unsigned long, unsigned long, unsigned long, void *)
{
//TODO (less importand - used only for modem and serial)
  return NULL;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
