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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct TbNetworkPlayerInfo {
char field_0[32];
long field_20;
};

struct TbNetworkCallbackData {
  char svc_name[20];
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

#pragma pack()
/******************************************************************************/
typedef void (*TbNetworkCallbackFunc)(struct TbNetworkCallbackData *, void *);
/******************************************************************************/
DLLIMPORT extern int _DK_network_initialized;
#define network_initialized _DK_network_initialized
/******************************************************************************/
TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data);
TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *playr_name, unsigned long *playr_num);
TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num);
int LbNetwork_Exchange(void *buf);
void LbNetwork_ChangeExchangeTimeout(unsigned long tmout);
TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long a2);
TbError LbNetwork_EnableNewPlayers(unsigned long allow);
TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *a2);
TbError LbNetwork_Stop(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
