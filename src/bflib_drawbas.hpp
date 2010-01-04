/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_drawbas.hpp
 *     Header file for bflib_drawbas.cpp.
 * @par Purpose:
 *     Graphics drawing support base class.
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
#ifndef BFLIB_DRAWBAS_H
#define BFLIB_DRAWBAS_H

#include "bflib_basics.h"
#include "bflib_video.h"
#include <windows.h>

#pragma pack(1)

struct TSurface;

#pragma pack()
/******************************************************************************/
/**
 * Base class for graphics support drivers.
 */
class TDDrawBaseClass {
 public:
    TDDrawBaseClass(void);
    virtual ~TDDrawBaseClass(void);
    // Virtual Methods
    virtual bool setup_window(void) = 0;
    virtual long WindowProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam) = 0;
    virtual void find_video_modes(void) = 0;
    virtual bool get_palette(void *,unsigned long,unsigned long) = 0;
    virtual bool set_palette(void *,unsigned long,unsigned long) = 0;
    virtual bool setup_screen(TbScreenMode) = 0;
    virtual bool lock_screen(void) = 0;
    virtual bool unlock_screen(void) = 0;
    virtual bool clear_screen(unsigned long) = 0;
    virtual bool clear_window(long,long,unsigned long,unsigned long,unsigned long) = 0;
    virtual bool swap_screen(void) = 0;
    virtual bool reset_screen(void) = 0;
    virtual bool restore_surfaces(void) = 0;
    virtual void wait_vbi(void) = 0;
    virtual bool swap_box(struct tagPOINT,struct tagRECT &) = 0;
    virtual bool create_surface(struct TSurface *,unsigned long,unsigned long) = 0;
    virtual bool release_surface(struct TSurface *) = 0;
    virtual bool blt_surface(struct TSurface *,unsigned long,unsigned long,tagRECT *,unsigned long) = 0;
    virtual void *lock_surface(struct TSurface *) = 0;
    virtual bool unlock_surface(struct TSurface *) = 0;
    virtual void LoresEmulation(bool);
    // Nonvirtual Methods
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool set_double_buffering_video(bool);
    bool is_double_buffering_video(void);
    bool set_wscreen_in_video(bool);
    bool is_wscreen_in_video(void);
    bool IsActive(void);
    void SetIcon(void);
 protected:
    // Properties
    char *appTitle;
    char *appName;
    unsigned long flags;
    HWND hWindow;
    bool active;
    };
/******************************************************************************/
extern char lbDrawAreaTitle[128];
extern class TDDrawBaseClass *lpDDC;
extern volatile HINSTANCE lbhInstance;
extern volatile TbBool lbInteruptMouse;
extern volatile unsigned long lbIconIndex;
/******************************************************************************/

#endif
