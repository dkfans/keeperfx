/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsession.c
 *     Algorithms and data structures for network sessions.
 * @par Purpose:
 *     Sessions support code for network library.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     08 Mar 2009 - 12 Oct 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_netsession.h"

#include "bflib_basics.h"
#include "bflib_memory.h"

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
