/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_semphr.hpp
 *     Header file for bflib_semphr.cpp.
 * @par Purpose:
 *     Semaphores wrapper.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     21 May 2009 - 20 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_SEMPHR_H
#define BFLIB_SEMPHR_H

#include "bflib_basics.h"

#include "globals.h"

/******************************************************************************/
#if defined(_WIN32)
//Selected declarations from Win32 API - I don't want to use whole API
// since it influences everything
typedef void *PVOID,*LPVOID;
typedef PVOID HANDLE;
#endif

/******************************************************************************/

class LbSemaphore {
public:
    LbSemaphore(void);
    virtual ~LbSemaphore(void);
    HANDLE sHandle;
};

/******************************************************************************/

class LbSemaLock {
public:
    LbSemaLock(class LbSemaphore *sem, int a2);
    virtual ~LbSemaLock(void);
    int Lock(TbBool wait_forever);
    void Release(void);
    HANDLE sHandle;
    int field_4;
    int field_8;
};

/******************************************************************************/

#endif
