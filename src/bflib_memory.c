/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 10 Feb 2008

// Purpose:
//    Memory related routines - allocation and freeing of memory blocks.

// Comment:
//   Original SW used different functions for allocating low and extended memory.
//   This is now outdated way, so functions here are simplified to originals.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#include "bflib_memory.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned long lbMemoryAvailable=0;
/******************************************************************************/
void * __fastcall LbMemorySet(void *dst, uchar c, ulong length)
{
  return memset(dst, c, length);
}

void * __fastcall LbMemoryCopy(void *in_dst, const void *in_src, ulong len)
{
  return memcpy(in_dst,in_src,len);
}

//Appends characters of source to destination, plus a terminating null-character.
// Prevents string in dst of getting bigger than maxlen characters.
void * __fastcall LbStringConcat(char *dst, const char *src, const ulong dst_buflen)
{
  int max_num=dst_buflen-strlen(dst);
  if (max_num<=0) return dst;
  strncat(dst, src, max_num);
  dst[dst_buflen-1]='\0';
  return dst;
}

void * __fastcall LbStringCopy(char *dst, const char *src, const ulong dst_buflen)
{
  strncpy(dst, src, dst_buflen);
  dst[dst_buflen-1]='\0';
  return dst;
}

ulong __fastcall LbStringLength(const char *str)
{
  if (str==NULL) return 0;
  return strlen(str);
}

int __fastcall LbMemorySetup()
{
//Disabled as we no longer need such memory routines
  return 1;
}

unsigned char * __fastcall LbMemoryAllocLow(ulong size)
{
//Simplified as we no longer need such memory routines
  unsigned char *ptr;
  ptr=(unsigned char *)malloc(size);
  if (ptr!=NULL)
    memset(ptr,0,size);
  return ptr;
}

unsigned char * __fastcall LbMemoryAlloc(ulong size)
{
//Simplified as we no longer need such memory routines
  unsigned char *ptr;
  ptr=(unsigned char *)malloc(size);
  if (ptr!=NULL)
    memset(ptr,0,size);
  return ptr;
}

int __fastcall LbMemoryFree(void *mem_ptr)
{
    if (mem_ptr==NULL) return -1;
    free(mem_ptr);
    return 1;
}

int __fastcall LbMemoryCheck()
{
  lbMemoryAvailable=536870912;
  return 1;
}

//The size of the memory block pointed to by the ptr parameter is changed
// to the size bytes, expanding the amount of memory available
// in the block. A pointer to the reallocated memory block is returned,
// which may be either the same as the ptr argument or a new location.
//If the function failed to allocate the requested block of memory,
// a NULL pointer is returned.
void * __fastcall LbMemoryGrow(void *ptr, unsigned long size)
{
    return realloc(ptr,size);
}

//The size of the memory block pointed to by the ptr parameter is changed
// to the size bytes, reducing the amount of memory available
// in the block. A pointer to the reallocated memory block is returned,
// which usually is the same as the ptr argument.
void * __fastcall LbMemoryShrink(void *ptr, unsigned long size)
{
    return realloc(ptr,size);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
