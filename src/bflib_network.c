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
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT TbError _DK_LbNetwork_Exchange(struct Packet *pckt);
DLLIMPORT TbError _DK_LbNetwork_Stop(void);
DLLIMPORT TbError _DK_LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, unsigned long *plyr_num);
DLLIMPORT TbError _DK_LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num);
DLLIMPORT TbError _DK_LbNetwork_ChangeExchangeBuffer(void *, unsigned long);
DLLIMPORT TbError _DK_LbNetwork_Init(unsigned long,struct _GUID guid, unsigned long, void *, unsigned long, struct TbNetworkPlayerInfo *netplayr, void *);
/******************************************************************************/

/******************************************************************************/
TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data)
{
  return _DK_LbNetwork_Init(srvcp,guid,maxplayrs,exchng_buf,exchng_size,locplayr,init_data);
}

TbError LbNetwork_Join(struct TbNetworkSessionNameEntry *nsname, char *plyr_name, unsigned long *plyr_num)
{
  return _DK_LbNetwork_Join(nsname, plyr_name, plyr_num);
}

TbError LbNetwork_Create(char *nsname_str, char *plyr_name, unsigned long *plyr_num)
{
  return _DK_LbNetwork_Create(nsname_str, plyr_name, plyr_num);
}

TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long a2)
{
  return _DK_LbNetwork_ChangeExchangeBuffer(buf, a2);
}

void LbNetwork_ChangeExchangeTimeout(unsigned long tmout)
{
//  exchangeTimeout = 1000 * tmout;
}

TbError LbNetwork_Stop(void)
{
  return _DK_LbNetwork_Stop();
}

int LbNetwork_Exchange(struct Packet *pckt)
{
  return _DK_LbNetwork_Exchange(pckt);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
