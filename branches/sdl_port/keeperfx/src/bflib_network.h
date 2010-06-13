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

#include <basetyps.h>
#include "bflib_basics.h"
#include "globals.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define CLIENT_TABLE_LEN 32
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct TbNetworkSessionNameEntry;

typedef long (*Net_Callback_Func)(void);

enum TbNetworkService {
    NS_Serial,
    NS_Modem,
    NS_IPX,
    NS_TCP_IP,
};


struct ClientDataEntry {
  unsigned long playerId;
  unsigned long playerActive;
  unsigned long field_8;
  char playerName[32];
};

struct ConfigInfo { // sizeof = 130
  char numfield_0;
  unsigned char numfield_1[8];
  char numfield_9;
  char str_atz[20];
  char str_atdt[20];
  char str_ath[20];
  char str_ats[20];
  char str_join[20];
  char str_u2[20];
};

struct TbModemDev { // sizeof = 180
  unsigned long field_0;
  unsigned long field_4;
  char field_8[80];
  char field_58[80];
  unsigned long field_A8;
  Net_Callback_Func field_AC;
  Net_Callback_Func field_B0;
};

struct ModemResponse {
  unsigned char field_0[8];
  unsigned char field_8[8];
  unsigned char field_10[8];
  unsigned char field_18[8];
  unsigned char field_20[8];
  unsigned char field_28[8];
  unsigned char field_30[8];
  unsigned char field_38[8];
};

struct TbNetworkPlayerInfo {
char name[32];
long active;
};

struct TbNetworkCallbackData {
  char svc_name[12];
  char field_C[20];
  char field_20[32];
};

struct TbNetworkPlayerName {
  char name[NET_PLAYER_NAME_LENGTH + 1];
};

struct TbNetworkPlayerNameEntry {
  unsigned char field_0;
  unsigned long id;
  unsigned long isLocal;
  unsigned long isHost;
  char name[19];
  unsigned char field_20[20];
  unsigned char field_34[4];
};

struct SystemUserMsg {
  unsigned char type;
  ClientDataEntry clientDataTable[CLIENT_TABLE_LEN];
};

struct UnidirectionalDataMessage {
  unsigned long field_0;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned char field_14[492];
  unsigned char field_200[12];
};

struct UnidirectionalRTSMessage {
  unsigned long field_0;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
};

struct SerialInitData {
long field_0;
    long numfield_4;
    long field_8;
long field_C;
    char *str_phone;
    char *str_dial;
    char *str_hang;
    char *str_answr;
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT extern int _DK_network_initialized;
#define network_initialized _DK_network_initialized
/******************************************************************************/
TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data);
TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *playr_name, unsigned long *playr_num, void *optns);
TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num, void *optns);
TbError LbNetwork_Exchange(void *buf);
void LbNetwork_ChangeExchangeTimeout(unsigned long tmout);
TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long a2);
TbError LbNetwork_EnableNewPlayers(TbBool allow);
TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *a2);
TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *a2);
TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr);
TbError LbNetwork_Stop(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
