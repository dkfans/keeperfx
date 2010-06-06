/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mspointer.hpp
 *     Header file for bflib_mspointer.cpp.
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
#ifndef BFLIB_MSPOINTER_H
#define BFLIB_MSPOINTER_H

#include "bflib_basics.h"
#include "bflib_semphr.hpp"
#include "bflib_video.h"
#include "bflib_drawsdk.hpp"

#ifdef __cplusplus
#pragma pack(1)
#endif
/******************************************************************************/

// Exported class
class LbI_PointerHandler {
 public:
    LbI_PointerHandler(void);
    ~LbI_PointerHandler(void);
    void SetHotspot(long x, long y);
    void Initialise(struct TbSprite *spr, struct tagPOINT *, struct tagPOINT *);
    void Release(void);
    void NewMousePos(int x, int y);
    bool OnMove(int x, int y);
    void OnBeginPartialUpdate(void);
    void OnEndPartialUpdate(void);
    void OnBeginSwap(void);
    void OnEndSwap(void);
    void OnBeginFlip(void);
    void OnEndFlip(void);
 protected:
    void ClipHotspot(void);
    void Draw(bool);
    void Undraw(bool);
    void Backup(bool);
    // Properties
    struct SSurface surf1;
    struct SSurface surf2;
    unsigned char sprite_data[4096];
    struct tagPOINT *position;
    struct tagPOINT *spr_offset;
    struct tagRECT drawRect;
    long draw_pos_x;
    long draw_pos_y;
    bool field_1050;
    bool field_1054;
    struct TbSprite *sprite;
    };

#ifdef __cplusplus
#pragma pack()
#endif


/******************************************************************************/

#endif
