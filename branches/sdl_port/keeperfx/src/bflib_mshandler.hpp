/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mshandler.hpp
 *     Header file for bflib_mshandler.cpp.
 * @par Purpose:
 *     Graphics drawing support sdk class.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     16 Nov 2008 - 21 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_MSHANDLER_H
#define BFLIB_MSHANDLER_H

#include "bflib_basics.h"
#include "bflib_video.h"
#include "bflib_mspointer.hpp"

/******************************************************************************/

// Exported class
class MouseStateHandler {
 public:
    MouseStateHandler(void);
    virtual ~MouseStateHandler(void);
    bool Install(void);
    bool IsInstalled(void);
    bool Release(void);
    struct tagPOINT *GetPosition(void);
    bool SetMousePosition(long x, long y);
    bool SetPosition(long x, long y);
    struct TbSprite *GetPointer(void);
    bool SetMousePointerAndOffset(struct TbSprite *mouseSprite, long x, long y);
    bool SetMousePointer(struct TbSprite *mouseSprite);
    bool SetPointerOffset(long x, long y);
    struct tagPOINT *GetPointerOffset(void);
    bool SetMouseWindow(long x, long y,long width, long height);
    bool PointerBeginSwap(void);
    bool PointerEndSwap(void);
    void updatePosition(int x, int y);
 protected:
    bool SetPointer(struct TbSprite *spr, struct tagPOINT *pt);
    // Properties
    bool installed;
    struct TbSprite *mssprite;
    struct tagPOINT mspos;
    struct tagPOINT hotspot;
    class LbI_PointerHandler pointer;
    bool swap;
    };

/******************************************************************************/
extern class MouseStateHandler winMouseHandler;
extern int volatile lbMouseInstalled;
extern int volatile lbMouseOffline;
/******************************************************************************/

#endif
