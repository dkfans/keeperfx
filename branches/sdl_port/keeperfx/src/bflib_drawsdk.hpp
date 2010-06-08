/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_drawsdk.hpp
 *     Header file for bflib_drawsdk.cpp.
 * @par Purpose:
 *     Graphics drawing support sdk class.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     16 Nov 2008 - 21 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_DRAWSDK_H
#define BFLIB_DRAWSDK_H

#include "bflib_basics.h"
#include "bflib_video.h"
#include "bflib_drawbas.hpp"
#include <ddraw.h>
#include <SDL.h>

/******************************************************************************/

// This structure could be rewritten to an OO class, but DK hardly uses it so I let it be.
struct SSurface {
    SDL_Surface* surf;
    unsigned long locks_count; //perhaps not necessary
    long pitch; //can in reality be replaced by surf->pitch if dependent code is changed
};

// Exported class
class TDDrawSdk : public TDDrawBaseClass {
 public:
    TDDrawSdk(void);
    virtual ~TDDrawSdk(void);
    // Virtual methods from superclass
    bool get_palette(void *,unsigned long,unsigned long);
    bool set_palette(void *,unsigned long,unsigned long);
    bool setup_screen(TbScreenMode *);
    bool lock_screen(void);
    bool unlock_screen(void);
    bool clear_screen(unsigned long);
    bool clear_window(long,long,unsigned long,unsigned long,unsigned long);
    bool swap_screen(void);
    bool reset_screen(void);
    bool restore_surfaces(void);
    bool swap_box(struct tagPOINT,struct tagRECT &);
    bool create_surface(struct SSurface *,unsigned long,unsigned long);
    bool release_surface(struct SSurface *);
    bool blt_surface(struct SSurface *,unsigned long,unsigned long,tagRECT *,unsigned long);
    void *lock_surface(struct SSurface *);
    bool unlock_surface(struct SSurface *);
    // Nonvirtual methods
    static bool isModePossible(TbScreenMode * mode);
    void setIcon(void);
 protected:
    LPCTSTR resource_mapping(int index);

    SDL_Surface * screenSurface;
    SDL_Surface * drawSurface; //may or may not be same as screen surface
    int lockCount;
    bool hasSecondSurface;
    };


/******************************************************************************/

#endif
