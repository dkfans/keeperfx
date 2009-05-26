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

/******************************************************************************/
TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data);
TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *playr_name, unsigned long *playr_num);
TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num);
int LbNetwork_Exchange(struct Packet *pckt);
void LbNetwork_ChangeExchangeTimeout(unsigned long tmout);
TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long a2);
TbError LbNetwork_Stop(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
