/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp.cpp
 *     Network ServiceProvider class declaration.
 * @par Purpose:
 *     Defines ServiceProvider for network library.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     08 Mar 2009 - 09 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_netsp.h"

#include "bflib_basics.h"

/******************************************************************************/
// Nil callbacks declaration
void NilAddMsgCallback(unsigned long , char *, void *);
void NilDeleteMsgCallback(unsigned long, void *);
void NilHostMsgCallback(unsigned long, void *);
void NilUserSysMsgCallback(void *);
void *NilUserDataMsgCallback(unsigned long, unsigned long, unsigned long, void *);
void NilRequestExchangeDataMsgCallback(unsigned long, unsigned long, void *);
void NilRequestCompositeExchangeDataMsgCallback(unsigned long, unsigned long, void *);
void *NilUnidirectionalMsgCallback(unsigned long, unsigned long, void *);
void NilSystemUserMsgCallback(unsigned long, void *, unsigned long, void *);

struct ReceiveCallbacks nilReceiveAspect = {
  NilAddMsgCallback,
  NilDeleteMsgCallback,
  NilHostMsgCallback,
  NilUserSysMsgCallback,
  NilUserDataMsgCallback,
  NilRequestExchangeDataMsgCallback,
  NilRequestCompositeExchangeDataMsgCallback,
  NilUnidirectionalMsgCallback,
  NilSystemUserMsgCallback,
  NULL,
};
/******************************************************************************/
class ServiceProvider *spPtr;
/******************************************************************************/
// Nil callbacks content
void NilAddMsgCallback(unsigned long a1, char *a2, void *a3)
{
  WARNLOG("hit(%d, \"%s\", *)",a1,a2);
}

void NilDeleteMsgCallback(unsigned long a1, void *a2)
{
  WARNLOG("hit(%d, *)",a1);
}

void NilHostMsgCallback(unsigned long a1, void *a2)
{
  WARNLOG("hit(%d, *)",a1);
}

void NilUserSysMsgCallback(void *a1)
{
  WARNLOG("hit(*)");
}

void *NilUserDataMsgCallback(unsigned long a1, unsigned long a2, unsigned long a3, void *a4)
{
  WARNLOG("hit(%d, %d, %d, *)",a1,a2,a3);
  return NULL;
}

void NilRequestExchangeDataMsgCallback(unsigned long a1, unsigned long a2, void *a3)
{
  WARNLOG("hit(%d, %d, *)",a1,a2);
}

void NilRequestCompositeExchangeDataMsgCallback(unsigned long a1, unsigned long a2, void *a3)
{
  WARNLOG("hit(%d, %d, *)",a1,a2);
}

void *NilUnidirectionalMsgCallback(unsigned long a1, unsigned long a2, void *a3)
{
  WARNLOG("hit(%d, %d, *)",a1,a2);
}

void NilSystemUserMsgCallback(unsigned long a1, void *a2, unsigned long a3, void *a4)
{
  WARNLOG("hit(%d, *, %d, *)",a1,a3);
}
/******************************************************************************/
// methods of virtual class ServiceProvider

void ServiceProvider::EncodeMessageStub(void *buf, unsigned long a2, unsigned char a3, unsigned long a4)
{
  if (buf == NULL)
  {
    WARNLOG("NULL ptr");
    return;
  }
  *(unsigned long *)buf = (a3 << 24) | ((a4 & 0xF) << 20) | (a2 & 0xFFFFF);
}

void ServiceProvider::EncodeDeletePlayerMsg(unsigned char *buf, unsigned long val)
{
  if (buf == NULL)
    WARNLOG("NULL ptr");
  else
    *(unsigned long *)buf = 0x2000004;
  memcpy(buf+4, &val, 4);
}

void ServiceProvider::EncodeRequestExchangeDataMsg(unsigned char *buf, unsigned long a1, unsigned long a2)
{
  if (buf == NULL)
  {
    WARNLOG("NULL ptr");
    return;
  }
  *(unsigned long *)buf = ((a2 & 0xF) << 20) | 0x5000004;
  memcpy(buf+4, &a1, 4);
}

void ServiceProvider::EncodeRequestCompositeExchangeDataMsg(unsigned char *buf, unsigned long a1, unsigned long a2)
{
  if (buf == NULL)
  {
    WARNLOG("NULL ptr");
    return;
  }
  *(unsigned long *)buf = ((a2 & 0xF) << 20) | 0x6000004;
  memcpy(buf+4, &a1, 4);
}

unsigned long ServiceProvider::DecodeRequestCompositeExchangeDataMsg(unsigned char *buf, unsigned long &a1)
{
  memcpy(&a1, buf+4, 4);
  return 1;
}

void ServiceProvider::DecodeMessageStub(void *buf, unsigned long *a2, unsigned char *a3, unsigned long *a4)
{
  unsigned long k;
  if (buf == NULL)
  {
    WARNLOG("NULL ptr");
    return;
  }
  if (a2 != NULL)
  {
    k = *(unsigned long *)buf;
    *a2 = k & 0xFFFFF;
  }
  if (a4 != NULL)
  {
    k = *(unsigned long *)buf;
    *a4 = ((k >> 20) & 0xF);
  }
  if (a3 != NULL)
  {
    k = *(unsigned long *)buf;
    *(unsigned char *)a3 = (k >> 24)  & 0xFF;
  }
}

unsigned long ServiceProvider::GetRequestCompositeExchangeDataMsgSize(void)
{
  return 8;
}

ServiceProvider::ServiceProvider()
{
  long i;
  this->local_id = 0;
  for (i=0; i < SESSION_ENTRIES_COUNT; i++)
  {
    nsnames[i].field_1 = 0;
    nsnames[i].field_5 = 0;
    nsnames[i].text[0] = '\0';
  }
  this->field_7A4 = 0;
  this->field_7A8 = 0;
  this->field_7AC = 0;
  strcpy(this->field_D50,"");
  this->field_D74 = 0;
  this->field_D78 = 0;
  this->recvCallbacks = &nilReceiveAspect;
}

ServiceProvider::~ServiceProvider()
{
  NETMSG("Destructing Service Provider");
}

TbError ServiceProvider::Initialise(struct ReceiveCallbacks *nCallbacks, void *a2)
{
  NETMSG("Initialising Service Provider");
  if (this->field_7A4)
    WARNLOG("Service Provider already set up!");
  this->field_7AC = 0;
  ClearSessions();
  this->field_D78 = a2;
  if (nCallbacks != NULL)
    this->recvCallbacks = nCallbacks;
  this->field_7A8 = 0;
  this->field_D74 = 0;
  this->field_7A4++;
  return Lb_OK;
}

TbError ServiceProvider::Send(unsigned long plr_id, void *buf)
{
  unsigned char messageType;
  unsigned long p1,p2,p3;
  void *imsg;
  char str[32];
  long i;
  if (this->field_D74 < 1)
  {
    WARNLOG("not initialised");
    return Lb_FAIL;
  }
  DecodeMessageStub(buf,&p1,&messageType,&p2);
  if (plr_id != this->local_id)
  {
    i = (messageType != 0) && (messageType != 5) && (messageType != 6) && (messageType != 8);
    if (this->SendMessage(plr_id, buf, i) != Lb_OK)
    {
      WARNLOG("failure on SendMessage()");
      return Lb_FAIL;
    }
    return Lb_OK;
  }
  switch (messageType)
  {
  case 0:
      if (recvCallbacks->multiPlayer == NULL)
      {
        WARNLOG("NIL target for userDataMsgCallbackProc");
        break;
      }
      imsg = recvCallbacks->multiPlayer(this->local_id, p1+4, p2, this->field_D78);
      if (imsg == NULL)
        break;
      memcpy(imsg, buf, p1+4);
      break;
  case 1:
      memcpy(&p3, (uchar *)buf+4, sizeof(unsigned long));
      strncpy(str,(char *)buf+8, 32);
      this->AddPlayer(p3, str, 0, 0);
      if (recvCallbacks->addMsg == NULL)
      {
        break;
      }
      recvCallbacks->addMsg(p3, str, this->field_D78);
      break;
  case 2:
      this->CheckForDeletedHost((uchar *)buf);
      memcpy(&p1, (uchar *)buf+4, 4);
      this->DeletePlayer(p1);
      if (recvCallbacks->deleteMsg == NULL)
      {
        break;
      }
      recvCallbacks->deleteMsg(p1, this->field_D78);
      break;
  case 3:
      break;
  case 4:
      if (recvCallbacks->systemUserMsg == NULL)
      {
        WARNLOG("NIL target for systemUserMsgCallbackProc");
        break;
      }
      recvCallbacks->systemUserMsg(this->local_id, (char *)buf+4, p1, this->field_D78);
      break;
  case 5:
      memcpy(&p3, (uchar *)buf+4, sizeof(unsigned long));
      if (recvCallbacks->mpReqExDataMsg == NULL)
      {
        break;
      }
      recvCallbacks->mpReqExDataMsg(p3, p2, this->field_D78);
      break;
  case 6:
      memcpy(&p3, (uchar *)buf+4, sizeof(unsigned long));
      if (recvCallbacks->mpReqCompsExDataMsg == NULL)
      {
        break;
      }
      recvCallbacks->mpReqCompsExDataMsg(p3, p2, this->field_D78);
      break;
  case 8:
      // This callback seems to never be used
      p3 = 0;
      if (recvCallbacks->field_24 == NULL)
      {
        break;
      }
      recvCallbacks->field_24(p3, buf);
      break;
  default:
      WARNLOG("messageType is out of range");
      break;
  }
  return Lb_OK;
}

TbError ServiceProvider::Receive(unsigned long a1)
{
  unsigned long p1,p2,p3;
  unsigned char messageType;
  TbBool keepExchng;
  if (this->field_D74 < 1)
  {
    WARNLOG("not initialised");
    return Lb_FAIL;
  }
  keepExchng = true;
  while (keepExchng)
  {
    p3 = 1028;
    if (!this->PeekMessage(&p1, &p2, &p3))
    {
      keepExchng = false;
      break;
    }
//!!!!!!!!!
/*    DecodeMessageStub(&p2, &v37, &messageType, &38);
    if (a1 & 0x08)
      a1 = -1;
*/
    switch (messageType)
    {
    case 0:
        //TODO
        break;
    default:
        WARNLOG("messageType is out of range");
        break;
    }
  }
  return Lb_OK;
}

TbError ServiceProvider::Release(void)
{
  NETMSG("Releasing Service Provider");
  this->field_7A8 = 0;
  if (this->field_7A4 != 1)
    WARNLOG("Service Provider not set up!");
  this->field_7A4--;
  return Lb_OK;
}

TbError ServiceProvider::AddPlayer(unsigned long a1, char *a2, unsigned long a3, unsigned long a4)
{
  //TODO
  return Lb_FAIL;
}

TbError ServiceProvider::DeletePlayer(unsigned long a1)
{
  //TODO
  return Lb_FAIL;
}

void ServiceProvider::ClearPlayers(void)
{
  this->field_7AC = 0;
}

struct TbNetworkSessionNameEntry *ServiceProvider::AddSession(unsigned long a1, char *a2)
{
  //TODO
  return NULL;
}

void ServiceProvider::ClearSessions(void)
{
  long i;
  for (i=0; i < SESSION_ENTRIES_COUNT; i++)
  {
    nsnames[i].field_5 = 0;
  }
}

TbError ServiceProvider::EnumeratePlayers(TbNetworkCallbackFunc callback, void *a2)
{
  //TODO
  return Lb_FAIL;
}

unsigned long ServiceProvider::GetAddPlayerMsgSize(char *a1)
{
  //TODO
  return 0;
}

void ServiceProvider::EncodeAddPlayerMsg(char *a1, unsigned long a2, char *a3)
{
  //TODO
}

unsigned long ServiceProvider::DecodeAddPlayerMsg(char *a1, unsigned long &a2, char *a3)
{
  //TODO
  return 0;
}

TbError ServiceProvider::SystemAddPlayerHandler(char *a1)
{
  //TODO
  return Lb_FAIL;
}

TbError ServiceProvider::SystemDeletePlayerHandler(char *a1)
{
  //TODO
  return Lb_FAIL;
}

TbError ServiceProvider::CheckForDeletedHost(unsigned char *a1)
{
  //TODO
  return Lb_FAIL;
}

TbError ServiceProvider::LookForSystemMessages(void)
{
  //TODO
  return Lb_FAIL;
}

TbError ServiceProvider::BroadcastSystemMessage(char *a1)
{
  //TODO
  return Lb_FAIL;
}

TbError ServiceProvider::EnumeratePlayersForSessionRunning(TbNetworkCallbackFunc callback, void *a2)
{
  //TODO
  return Lb_FAIL;
}

TbError ServiceProvider::EnableNewPlayers(TbBool allow)
{
  return Lb_OK;
}

/******************************************************************************/
