/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_memory.c
 *     Memory managing routines.
 * @par Purpose:
 *     Memory related routines - allocation and freeing of memory blocks.
 * @par Comment:
 *     Original SW used different functions for allocating low and extended memory.
 *     This is now outdated way, so functions here are simplified to originals.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_memory.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "post_inc.h"

/******************************************************************************/
char lbEmptyString[] = "";
/******************************************************************************/

void * LbStringToLowerCopy(char *dst, const char *src, const ulong dst_buflen)
{
  if (dst_buflen < 1)
    return dst;
  for (int i = 0; i < dst_buflen; i++)
  {
      char chr = tolower(src[i]);
      dst[i] = chr;
      if (chr == '\0')
          break;
  }
  dst[dst_buflen-1]='\0';
  return dst;
}

void * LbMemoryAlloc(ulong size)
{
    void * ptr = malloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

/******************************************************************************/
