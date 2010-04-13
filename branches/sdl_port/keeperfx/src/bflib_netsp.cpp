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
#include "bflib_netsp.hpp"

#include "bflib_basics.h"
#include "bflib_memory.h"

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
void net_copy_name_string(char *dst,const char *src,long max_len)
{
  LbMemorySet(dst, 0, max_len);
  if (dst != NULL)
  {
    if (src != NULL)
    {
      strncpy(dst, src, max_len-1);
      dst[max_len-1] = '\0';
    }
  }
}
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
  return NULL;
}

void NilSystemUserMsgCallback(unsigned long a1, void *a2, unsigned long a3, void *a4)
{
  WARNLOG("hit(%d, *, %d, *)",a1,a3);
}
/******************************************************************************/
// methods of virtual class ServiceProvider

void ServiceProvider::EncodeMessageStub(void *enc_msg, unsigned long a2, unsigned char messageType, unsigned long a4)
{
  if (enc_msg == NULL)
  {
    WARNLOG("NULL ptr");
    return;
  }
  *(unsigned long *)enc_msg = (messageType << 24) | ((a4 & 0xF) << 20) | (a2 & 0xFFFFF);
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

void ServiceProvider::DecodeMessageStub(const void *enc_msg, unsigned long *a2, unsigned char *messageType, unsigned long *a4)
{
  unsigned long k;
  if (enc_msg == NULL)
  {
    WARNLOG("NULL ptr");
    return;
  }
  if (a2 != NULL)
  {
    k = *(unsigned long *)enc_msg;
    *a2 = k & 0xFFFFF;
  }
  if (a4 != NULL)
  {
    k = *(unsigned long *)enc_msg;
    *a4 = ((k >> 20) & 0xF);
  }
  if (messageType != NULL)
  {
    k = *(unsigned long *)enc_msg;
    *messageType = (k >> 24)  & 0xFF;
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
    nsnames[i].id = 0;
    nsnames[i].in_use = false;
    LbMemorySet(nsnames[i].text, 0, SESSION_NAME_MAX_LEN);
  }
  this->field_7A4 = 0;
  this->field_7A8 = 0;
  this->players_count = 0;
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
  NETMSG("Initializing Service Provider");
  if (this->field_7A4)
    WARNLOG("Service Provider already set up!");
  this->players_count = 0;
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
    WARNLOG("not initialized");
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
      this->CheckForDeletedHost(buf);
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

TbError ServiceProvider::Receive(unsigned long len) //a1 = length?
{
  unsigned long p1, p2, p3;
  unsigned long decode_20bits,decode_4bits;
  unsigned char messageType;
  unsigned long id;
  TbBool keepExchanging;
  char array1[1024];
  char array2[32];
  char array3[32];
  char * array1Ptr;
  char * array2Ptr;
  char * array3Ptr;
  void * tmpPtr;
  long tmpInt1, tmpInt2;
  TbError result;

  //see what these are
  char wtf;
  unsigned long wtf2;
  unsigned long wtf3;
  unsigned long wtf4;

  result = 0;

  if (this->field_D74 < 1)
  {
    WARNLOG("not initialized");
    return Lb_FAIL;
  }
  keepExchanging = true;
  while (keepExchanging)
  {
    p3 = 1028; //probably a bitmask
    if (PeekMessage(&p1, &p2, &p3))
    {
      keepExchanging = false;
      break;
    }

    DecodeMessageStub(&p2, &decode_20bits, &messageType, &decode_4bits);
    if (len & 0x08)
      len = 0xFF;
    switch (messageType)
    {
    case 0:
        if (len & 1) {
        	tmpPtr = 0;
        	if (recvCallbacks->multiPlayer) {
        		tmpPtr = recvCallbacks->multiPlayer(p1, decode_20bits + 4, decode_4bits, field_D78);
        	}
        	else {
        		NETMSG("NIL target for userDataMsgCallbackProc"); //rename
        	}
        	if (tmpPtr == NULL) {
        		tmpPtr = &p2;
        		p3 = 1028;
        	}
        	if (!(ReadMessage(&p1, tmpPtr, &p3))) {
        		NETMSG("Inconsistency between reads");
				result = wtf;
        	}
        }

        keepExchanging = false;
        break;
    case 1:
        if (!(len & 2)) {
        	keepExchanging = false;
        	break;
        }

        p3 = 1028;
        if (!ReadMessage(&p1, &p2, &p3)) {
        	NETMSG("Inconsistency on receive");
        	break;
        }

        memcpy(&tmpInt1, array1, sizeof(tmpInt1));
        if (array2) {
        	array1Ptr = array1 + 4;
        	array2Ptr = array2;
        	bool cond;
        	do {
        		array2Ptr[0] = array1Ptr[0];
        		if (!array1Ptr[0]) {
        			break;
        		}

        		cond = array1Ptr[1];
        		array1Ptr += 2;
        		array2Ptr[1] = cond;
        		array2Ptr += 2;
        	} while (cond);
        }
        AddPlayer(tmpInt1, array2, 0, 0);

        memcpy(&tmpInt2, array1, sizeof(tmpInt2));
        memcpy(&tmpInt1, array1 + 4, sizeof(tmpInt1));
        if (&p2 != (unsigned long *) -1028) {
        	array1Ptr = array1 + 4;
        	array3Ptr = array3;
        	bool cond;
        	do {
        		array3Ptr[0] = array1Ptr[0];
				if (!array1Ptr[0]) {
					break;
				}

				cond = array1Ptr[1];
				array1Ptr += 2;
				array3Ptr[1] = cond;
				array3Ptr += 2;
        	} while (cond);
        }

        if (recvCallbacks->addMsg) {
        	recvCallbacks->addMsg(tmpInt2, array3, field_D78);
        }

        break;
    case 2:
        if (!(len & 4)) {
        	keepExchanging = false;
        	break;
        }

        p3 = 1028;
        if (!ReadMessage(&p1, &p2, &p3)) {
			NETMSG("Inconsistency on receive");
			break;
		}

        CheckForDeletedHost(&p2);
        memcpy(&id, array1, wtf2);
        DeletePlayer(id);
        memcpy(&tmpInt1, array1, wtf3);
        if (recvCallbacks->deleteMsg) {
        	recvCallbacks->deleteMsg(tmpInt1, field_D78);
        }

        break;
    case 3:
        continue;
    case 4:
        if (!(len & 0x80)) {
        	keepExchanging = false;
        	break;
        }

        p3 = 1028;
		if (!ReadMessage(&p1, &p2, &p3)) {
			NETMSG("Inconsistency on receive");
			break;
		}

		if (&p2 != (unsigned long *) -1124) {
			wtf4 = (unsigned int)((char *) (0xFFFFA + 5)) & p2; //TODO: what does this address point to?.......
		}
		if (recvCallbacks->systemUserMsg) {
			recvCallbacks->systemUserMsg(p1, array1, wtf4, field_D78);
		}

        break;
    case 5:
        //TODO NET
        break;
    case 6:
        //TODO NET
        break;
    case 7:
        //TODO NET
        break;
    case 8:
        //TODO NET
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

long ServiceProvider::PlayerIndex(unsigned long plyr_id)
{
  struct TbNetworkPlayerEntry *netplyr;
  long i;
  for (i=0; i < this->players_count; i++)
  {
    netplyr = &this->players[i];
    if (plyr_id == netplyr->id)
      return i;
  }
  return -1;
}

TbError ServiceProvider::AddPlayer(unsigned long plyr_id, const char *namestr, unsigned long a3, unsigned long a4)
{
  struct TbNetworkPlayerEntry *netplyr;
  long i;
  // Check if we already have the player on list
  if (PlayerIndex(plyr_id) >= 0)
    return Lb_OK;
  // Get a structure for new player
  i = this->players_count;
  if (i >= NETSP_PLAYERS_COUNT)
  {
    WARNLOG("player table is full");
    return Lb_FAIL;
  }
  netplyr = &this->players[i];
  netplyr->id = plyr_id;
  net_copy_name_string(netplyr->name,namestr,NETSP_PLAYER_NAME_MAX_LEN);
  netplyr->field_5 = a3;
  netplyr->field_9 = a4;
  this->players_count++;
  return Lb_OK;
}

TbError ServiceProvider::DeletePlayer(unsigned long plyr_id)
{
  long i;
  // Check if we have the player on list
  i = PlayerIndex(plyr_id);
  if (i < 0)
    return Lb_OK;
  while (i < this->players_count-1)
  {
    memcpy(&this->players[i], &this->players[i+1], sizeof(struct TbNetworkPlayerEntry));
    i++;
  }
  this->players_count--;
  return Lb_OK;
}

void ServiceProvider::ClearPlayers(void)
{
  this->players_count = 0;
}

long ServiceProvider::SessionIndex(unsigned long sess_id)
{
  struct TbNetworkSessionNameEntry *nsname;
  int i;
  for (i=0; i < SESSION_ENTRIES_COUNT; i++)
  {
    nsname = &this->nsnames[i];
    if ((nsname->in_use) && (nsname->id == sess_id))
        return i;
  }
  return -1;
}

/**
 * Adds a session with given sess_id and name string.
 * @param sess_id ID of the session, or -1 if new session should be added.
 * @param namestr Text name of the new session, or NULL if session is unnamed.
 * @return Returns session name structure, or NULL if couldn't add.
 */
struct TbNetworkSessionNameEntry *ServiceProvider::AddSession(unsigned long sess_id, const char *namestr)
{
  struct TbNetworkSessionNameEntry *nsname;
  TbBool got;
  long i;
  // Check if the session i already in list
  i = SessionIndex(sess_id);
  if (i >= 0)
    return &this->nsnames[i];
  // Search for unused slot
  got = false;
  for (i=0; i < SESSION_ENTRIES_COUNT; i++)
  {
    nsname = &this->nsnames[i];
    if (!nsname->in_use)
    {
      got = true;
      break;
    }
  }
  if (!got)
    return NULL;
  // Fill the new entry
  nsname->id = sess_id;
  net_copy_name_string(nsname->text,namestr,SESSION_NAME_MAX_LEN);
  nsname->in_use = true;
  return nsname;
}

/**
 * Sets all sessions as unused.
 */
void ServiceProvider::ClearSessions(void)
{
  long i;
  for (i=0; i < SESSION_ENTRIES_COUNT; i++)
  {
    nsnames[i].in_use = false;
  }
}

TbError ServiceProvider::EnumeratePlayers(TbNetworkCallbackFunc callback, void *a2)
{
  struct TbNetworkPlayerEntry *netplyr;
  TbError result;
  long i;
  result = Lb_OK;
  for (i=0; i < players_count; i++)
  {
    netplyr = &this->players[i];
    callback((struct TbNetworkCallbackData *)netplyr, a2);
  }
  return result;
}

unsigned long ServiceProvider::GetAddPlayerMsgSize(char *msg_str)
{
  return strlen(msg_str) + 1 + sizeof(unsigned long) + sizeof(unsigned long);
}

void ServiceProvider::EncodeAddPlayerMsg(unsigned char *enc_buf, unsigned long id, const char *msg_str)
{
  long len;
  unsigned char *out;
  out = enc_buf;
  if (enc_buf == NULL)
  {
    WARNLOG("Can't write to NULL buffer!");
    return;
  }
  len = strlen(msg_str) + 5;
  *(unsigned long *)out = len & 0xFFFFF | 0x1000000;
  out += sizeof(unsigned long);
  memcpy(out, &id, sizeof(unsigned long));
  out += sizeof(unsigned long);
  strcpy((char *)out,msg_str);
}

TbBool ServiceProvider::DecodeAddPlayerMsg(const unsigned char *enc_buf, unsigned long &id, char *msg_str)
{
  const unsigned char *inp;
  inp = enc_buf;
  inp += sizeof(unsigned long);
  memcpy(&id, inp, sizeof(unsigned long));
  if (msg_str == NULL)
    return true;
  inp += sizeof(unsigned long);
  strcpy(msg_str, (const char *)inp);
  return true;
}

TbError ServiceProvider::SystemAddPlayerHandler(const void *enc_buf)
{
  char name[NETSP_PLAYER_NAME_MAX_LEN];
  const unsigned char *inp;
  unsigned long plyr_id;
  inp = (const unsigned char *)enc_buf;
  inp += sizeof(unsigned long);
  memcpy(&plyr_id, inp, sizeof(unsigned long));
  inp += sizeof(unsigned long);
  net_copy_name_string(name,(const char *)inp,NETSP_PLAYER_NAME_MAX_LEN);
  if (AddPlayer(plyr_id, name, 0, 0) == Lb_OK)
    return Lb_OK;
  return Lb_FAIL;
}

TbError ServiceProvider::SystemDeletePlayerHandler(const void *enc_buf)
{
  const unsigned char *inp;
  unsigned long plyr_id;
  CheckForDeletedHost(enc_buf);
  inp = (const unsigned char *)enc_buf;
  inp += sizeof(unsigned long);
  memcpy(&plyr_id, inp, sizeof(unsigned long));
  if (DeletePlayer(plyr_id) == Lb_OK)
    return Lb_OK;
  return Lb_FAIL;
}

TbError ServiceProvider::CheckForDeletedHost(const void *enc_buf)
{
  struct TbNetworkPlayerEntry *netplyr;
  const unsigned char *inp;
  unsigned long plyr_id;
  unsigned long idx1;
  TbBool got;
  long i;
  inp = (const unsigned char *)enc_buf;
  inp += sizeof(unsigned long);
  memcpy(&plyr_id, inp, sizeof(unsigned long));


  //TODO NET CheckForDeletedHost
  got = 0;
  for (i=0; i < players_count; i++)
  {
    netplyr = &this->players[i];
    if ((plyr_id == netplyr->id) && (netplyr->field_9))
    {
      idx1 = i;
      got = 1;
      break;
    }
  }
  if (!got)
    return Lb_OK;
/*!!!!!!
  got = 0;
  for (i=0; i < players_count; i++)
  {
    netplyr = &this->players[i];
    if (!netplyr->field_9)
    {
      if ( got )
      {
        if ( ebp0 > *(_DWORD *)&v5->field_D50[1] )
        {
          ebp0 = *(_DWORD *)&v5->field_D50[1];
          idx2 = i;
        }
      } else
      {
        got = 1;
        idx2 = i;
        ebp0 = *(_DWORD *)&v5->field_D50[1];
      }
    }
  }

  if ( got )
  {
    netplyr = &this->players[idx1];
    netplyr->field_9 = 1;
    netplyr = &this->players[idx1];
    netplyr->field_9 = 0;
    if (recvCallbacks->deleteMsg != NULL)
      (recvCallbacks->deleteMsg(ebp0, *(_DWORD *)&this1[1].nsnames[23].field_9[16]);
  }
*/
  return Lb_OK;
}

TbError ServiceProvider::LookForSystemMessages(void)
{
  return this->Receive(0xB6);
}

TbError ServiceProvider::BroadcastSystemMessage(void *enc_msg)
{
  struct TbNetworkPlayerEntry *netplyr;
  unsigned char messageType;
  unsigned long id;
  TbError result;
  unsigned char *inp;
  long i;
  inp = (unsigned char *)enc_msg;
  this->DecodeMessageStub(inp, NULL, &messageType, NULL);
  inp += sizeof(unsigned long);
  if ( (messageType < 1) || ((messageType > 1) && (messageType != 2)) )
  {
    WARNLOG("invalid message type: %02X", (int)messageType);
    return Lb_FAIL;
  } else
  {
    memcpy(&id, inp, sizeof(unsigned long));
  }
  inp += sizeof(unsigned long);
  result = Lb_OK;
  for (i=0; i < this->players_count; i++)
  {
    netplyr = &this->players[i];
    if ((netplyr->id != this->local_id) && (netplyr->id != id))
    {
      if (this->SendMessage(netplyr->id,inp,1) != Lb_OK)
      {
        WARNLOG("failure while sending message");
        result = Lb_FAIL;
      }
    }
  }
  return result;
}

TbError ServiceProvider::EnumeratePlayersForSessionRunning(TbNetworkCallbackFunc callback, void *a2)
{
  if (this->LookForSystemMessages() != Lb_OK)
  {
    WARNLOG("failure while looking for system messages");
    return Lb_FAIL;
  }
  if (this->EnumeratePlayers(callback, a2) != Lb_OK)
  {
    WARNLOG("failure while enumerating players");
    return Lb_FAIL;
  }
  return Lb_OK;
}

TbError ServiceProvider::EnableNewPlayers(TbBool allow)
{
  return Lb_OK;
}

/******************************************************************************/
