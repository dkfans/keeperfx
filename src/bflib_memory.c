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
#include "bflib_memory.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#if defined(_WIN32)

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef WINBASEAPI
#ifdef __W32API_USE_DLLIMPORT__
#define WINBASEAPI DECLSPEC_IMPORT
#else
#define WINBASEAPI
#endif
#endif

#define WINAPI __stdcall
#define WINAPIV __cdecl
#define APIENTRY __stdcall
#define CALLBACK __stdcall

typedef unsigned long DWORD;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef char CCHAR, *PCCHAR;
typedef unsigned char UCHAR,*PUCHAR;
typedef unsigned short USHORT,*PUSHORT;
typedef unsigned long ULONG,*PULONG;
typedef char *PSZ;

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS,*LPMEMORYSTATUS;

WINBASEAPI VOID WINAPI GlobalMemoryStatus(LPMEMORYSTATUS);

#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
unsigned long lbMemoryAvailable=0;
short lbMemorySetup=0;
char lbEmptyString[] = "";
/******************************************************************************/
/**
 * Updates memory status variables.
 */
short update_memory_constraits(void)
{
  LbMemoryCheck();
  if ( lbMemoryAvailable <= (8*1024*1024) )
      mem_size = 8;
  else
  if ( lbMemoryAvailable <= (16*1024*1024) )
      mem_size = 16;
  else
  if ( lbMemoryAvailable <= (24*1024*1024) )
      mem_size = 24;
  else
//  if ( lbMemoryAvailable <= (32*1024*1024) )
      mem_size = 32;
/*
  else
  if ( lbMemoryAvailable <= (48*1024*1024) )
      mem_size = 48;
  else
      mem_size = 64;
*/
  LbSyncLog("PhysicalMemory %d\n", mem_size);
  return true;
}

void * LbMemorySet(void *dst, uchar c, ulong length)
{
  return memset(dst, c, length);
}

void * LbMemoryCopy(void *in_dst, const void *in_src, ulong len)
{
  return memcpy(in_dst,in_src,len);
}

/**
 *
 * Appends characters of source to destination, plus a terminating null-character.
 * Prevents string in dst of getting bigger than maxlen characters.
 */
void * LbStringConcat(char *dst, const char *src, const ulong dst_buflen)
{
  int max_num=dst_buflen-strlen(dst);
  if (max_num<=0) return dst;
  strncat(dst, src, max_num);
  dst[dst_buflen-1]='\0';
  return dst;
}

void * LbStringCopy(char *dst, const char *src, const ulong dst_buflen)
{
  if (dst_buflen < 1)
    return dst;
  strncpy(dst, src, dst_buflen);
  dst[dst_buflen-1]='\0';
  return dst;
}

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

ulong LbStringLength(const char *str)
{
    if (str == NULL) return 0;
    return strlen(str);
}

void LbMemRegister_Setup(void)
{
    /*memset(lbMemList, 0, 0x800);
    lbMemAllocation = 0;
    lbMemCount = 0;*/
}

int LbMemorySetup()
{
    if (lbMemorySetup != 0)
        return 0;
    lbMemorySetup = 1;
    /* Heap handling by application is only required for some platforms
    if (heap_handle == NULL)
    {
        heap_handle = HeapCreate(heap_handle, heap_handle, heap_handle);
        LbMemRegister_Setup();
    }
    if (heap_handle == NULL)
        return -1;
    */
    return 1;
}

int LbMemoryReset(void)
{
    if (lbMemorySetup == 0)
        return 0;
    lbMemorySetup = 0;
    /* Heap handling by application is only required for some platforms
    if (heap_handle != NULL)
    {
        if (!HeapDestroy(heap_handle))
            return -1;
        heap_handle = NULL;
    }
    LbMemRegister_Setup();
    */
    return 1;
}

unsigned char * LbMemoryAllocLow(ulong size)
{
//Simplified as we no longer need such memory routines
unsigned char* ptr = (unsigned char*)malloc(size);
if (ptr != NULL)
    memset(ptr, 0, size);
return ptr;
}

unsigned char * LbMemoryAlloc(ulong size)
{
    unsigned char* ptr = (unsigned char*)malloc(size);
    if (ptr != NULL)
      memset(ptr,0,size);
    return ptr;
}

int LbMemoryFree(void *mem_ptr)
{
    if (mem_ptr==NULL) return -1;
    free(mem_ptr);
    return 1;
}

short LbMemoryCheck(void)
{
#if defined(_WIN32)
  struct _MEMORYSTATUS msbuffer;
  msbuffer.dwLength = 32;
  GlobalMemoryStatus(&msbuffer);
  lbMemoryAvailable=msbuffer.dwTotalPhys;
#else
  lbMemoryAvailable=536870912;
#endif
  return 1;
}

/** Enlarge previously allocated memory block.
 *  The size of the memory block pointed to by the ptr parameter is
 *  changed to the size bytes, expanding the amount of memory available
 *  in the block. A pointer to the reallocated memory block is returned,
 *  which may be either the same as the ptr argument or a new location.
 *  If the function failed to allocate the requested block of memory,
 *  a NULL pointer is returned.
 *
 * @param ptr The previously allocated memory block.
 * @param size New size of the block.
 */
void * LbMemoryGrow(void *ptr, unsigned long size)
{
    return realloc(ptr,size);
}

/** Reduce previously allocated memory block.
 *  The size of the memory block pointed to by the ptr parameter is
 *  changed to the size bytes, reducing the amount of memory available
 *  in the block. A pointer to the reallocated memory block is returned,
 *  which usually is the same as the ptr argument.
 *
 * @param ptr The previously allocated memory block.
 * @param size New size of the block.
 */
void * LbMemoryShrink(void *ptr, unsigned long size)
{
    return realloc(ptr,size);
}

/** Reduce previously allocated memory block.
 *  The size of the memory block pointed to by the ptr parameter is
 *  changed to the size bytes, reducing the amount of memory available
 *  in the block. A pointer to the reallocated memory block is returned,
 *  which usually is the same as the ptr argument.
 *
 * @param ptr The previously allocated memory block.
 * @param size New size of the block.
 */
int LbMemoryCompare(void *ptr1, void *ptr2, unsigned long size)
{
    return memcmp(ptr1,ptr2,size);
}

/******************************************************************************/
