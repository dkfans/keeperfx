/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_semphr.cpp
 *     Semaphores wrapper.
 * @par Purpose:
 *     Creates unified interface for semaphores, which are used for
 *      sharing access to resources on multiple threads.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     21 May 2009 - 20 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_semphr.hpp"

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#if defined(_WIN32)
//Selected declarations from Win32 API - I don't want to use whole API
// since it influences everything
#ifndef WINBASEAPI
#ifdef __W32API_USE_DLLIMPORT__
#define WINBASEAPI DECLSPEC_IMPORT
#else
#define WINBASEAPI
#endif
#endif
#define WINAPI __stdcall
#ifndef CONST
#define CONST const
#endif
typedef unsigned long DWORD;
typedef long LONG;
typedef char CHAR;
typedef int WINBOOL,*PWINBOOL,*LPWINBOOL;
typedef CONST CHAR *LPCCH,*PCSTR,*LPCSTR;
#define BOOL WINBOOL
#define DECLARE_HANDLE(n) typedef HANDLE n
typedef HANDLE *PHANDLE,*LPHANDLE;
typedef long *LPLONG;
typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES,*PSECURITY_ATTRIBUTES,*LPSECURITY_ATTRIBUTES;
#define INFINITE    0xFFFFFFFF
WINBASEAPI HANDLE WINAPI CreateSemaphoreA(LPSECURITY_ATTRIBUTES,LONG,LONG,LPCSTR);
WINBASEAPI BOOL WINAPI ReleaseSemaphore(HANDLE,LONG,LPLONG);
WINBASEAPI DWORD WINAPI WaitForSingleObject(HANDLE,DWORD);
WINBASEAPI BOOL WINAPI CloseHandle(HANDLE);
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

LbSemaphore::LbSemaphore(void)
{
  this->sHandle = CreateSemaphoreA(NULL, 1, 1, NULL);
}

LbSemaphore::~LbSemaphore(void)
{
  CloseHandle(this->sHandle);
}

/******************************************************************************/

LbSemaLock::LbSemaLock(class LbSemaphore *sem, int a2)
{
  this->sHandle = sem->sHandle;
  this->field_4 = a2;
  this->field_8 = 0;
}

LbSemaLock::~LbSemaLock(void)
{
  Release();
}

void LbSemaLock::Release(void)
{
  if ( this->field_4 )
  {
    if ( !this->field_8 )
    {
      ReleaseSemaphore(this->sHandle, 1, 0);
      this->field_4 = 0;
    }
  }
}

int LbSemaLock::Lock(TbBool wait_forever)
{
  if (wait_forever)
  {
    this->field_4 = WaitForSingleObject(this->sHandle, INFINITE) < 1;
    return this->field_4;
  } else
  {
    this->field_4 = WaitForSingleObject(this->sHandle, 5) < 1;
    return this->field_4;
  }
}

/******************************************************************************/
