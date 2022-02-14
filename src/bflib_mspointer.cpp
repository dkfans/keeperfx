/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mspointer.cpp
 *     Graphics drawing support sdk class.
 * @par Purpose:
 *     A link between game engine and the DirectDraw library.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     16 Nov 2008 - 21 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_mspointer.hpp"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_planar.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"
#include "bflib_vidsurface.h"
#include "bflib_vidraw.h"

#include "keeperfx.hpp"
/******************************************************************************/
struct SSurface;
/******************************************************************************/
// Global variables
volatile TbBool lbPointerAdvancedDraw;
long cursor_xsteps_array[2*CURSOR_SCALING_XSTEPS];
long cursor_ysteps_array[2*CURSOR_SCALING_YSTEPS];
/******************************************************************************/
// function used for actual drawing
extern "C" {
TbResult LbSpriteDrawUsingScalingUpDataSolidLR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite);
}
/******************************************************************************/

void LbCursorSpriteSetScalingWidthClipped(long x, long swidth, long dwidth, long gwidth)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)swidth,(int)dwidth,(int)x);
    if (swidth > CURSOR_SCALING_XSTEPS)
        swidth = CURSOR_SCALING_XSTEPS;
    LbSpriteSetScalingWidthClippedArray(cursor_xsteps_array, x, swidth, dwidth, gwidth);
}

void LbCursorSpriteSetScalingWidthSimple(long x, long swidth, long dwidth)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)swidth,(int)dwidth,(int)x);
    if (swidth > CURSOR_SCALING_XSTEPS)
        swidth = CURSOR_SCALING_XSTEPS;
    LbSpriteSetScalingWidthSimpleArray(cursor_xsteps_array, x, swidth, dwidth);
}

void LbCursorSpriteSetScalingHeightClipped(long y, long sheight, long dheight, long gheight)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)sheight,(int)dheight,(int)y);
    if (sheight > CURSOR_SCALING_YSTEPS)
        sheight = CURSOR_SCALING_YSTEPS;
    LbSpriteSetScalingHeightClippedArray(cursor_ysteps_array, y, sheight, dheight, gheight);
}

void LbCursorSpriteSetScalingHeightSimple(long y, long sheight, long dheight)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)sheight,(int)dheight,(int)y);
    if (sheight > CURSOR_SCALING_YSTEPS)
        sheight = CURSOR_SCALING_YSTEPS;
    LbSpriteSetScalingHeightSimpleArray(cursor_ysteps_array, y, sheight, dheight);
}

/**
 * Draws the mouse pointer sprite on a display buffer.
 */
static long PointerDraw(long x, long y, const struct TbSprite *spr, TbPixel *outbuf, unsigned long scanline)
{
    unsigned int dwidth;
    unsigned int dheight;
    // Prepare bounds
    dwidth = scale_ui_value_lofi(spr->SWidth);
    dheight = scale_ui_value_lofi(spr->SHeight);
    if ( (dwidth <= 0) || (dheight <= 0) )
        return 1;
    if ( (lbDisplay.MouseWindowWidth <= 0) || (lbDisplay.MouseWindowHeight <= 0) )
        return 1;
    // Normally it would be enough to check if ((dwidth+x) >= gwidth), but due to rounding we need to add swidth
    if ((x < 0) || ((dwidth + spr->SWidth + x) >= lbDisplay.MouseWindowWidth))
    {
        LbCursorSpriteSetScalingWidthClipped(x, spr->SWidth, dwidth, lbDisplay.MouseWindowWidth);
    } else {
        LbCursorSpriteSetScalingWidthSimple(x, spr->SWidth, dwidth);
    }
    // Normally it would be enough to check if ((dheight+y) >= gheight), but our simple rounding may enlarge the image
    if ((y < 0) || ((dheight + spr->SHeight + y) >= lbDisplay.MouseWindowHeight))
    {
        LbCursorSpriteSetScalingHeightClipped(y, spr->SHeight, dheight, lbDisplay.MouseWindowHeight);
    } else {
        LbCursorSpriteSetScalingHeightSimple(y, spr->SHeight, dheight);
    }
    long *xstep;
    long *ystep;
    {
        xstep = &cursor_xsteps_array[0];
        ystep = &cursor_ysteps_array[0];
    }
    outbuf = &outbuf[xstep[0] + scanline * ystep[0]];
    return LbSpriteDrawUsingScalingUpDataSolidLR(outbuf, scanline, lbDisplay.MouseWindowHeight, xstep, ystep, spr);
}

// Methods

LbI_PointerHandler::LbI_PointerHandler(void)
{
    LbScreenSurfaceInit(&surf1);
    LbScreenSurfaceInit(&surf2);
    this->field_1050 = false;
    this->field_1054 = false;
    this->sprite = NULL;
    this->position = NULL;
    this->spr_offset = NULL;
    draw_pos_x = 0;
    draw_pos_y = 0;
}

LbI_PointerHandler::~LbI_PointerHandler(void)
{
    Release();
}

void LbI_PointerHandler::SetHotspot(long x, long y)
{
    long prev_x;
    long prev_y;
    LbSemaLock semlock(&sema_rel,0);
    semlock.Lock(true);
    if (this->field_1050)
    {
        // Set new coords, and backup previous ones
        prev_x = spr_offset->x;
        spr_offset->x = x;
        prev_y = spr_offset->y;
        spr_offset->y = y;
        ClipHotspot();
        // If the coords were changed, then update the pointer
        if ((spr_offset->x != prev_x) || (spr_offset->y != prev_y))
        {
            Undraw(true);
            NewMousePos();
            Backup(true);
            Draw(true);
        }
    }
}

void LbI_PointerHandler::ClipHotspot(void)
{
    if (!this->field_1050)
        return;
    if ((sprite != NULL) && (spr_offset != NULL))
    {
        if (spr_offset->x < 0)
        {
          spr_offset->x = 0;
        } else
        if (sprite->SWidth <= spr_offset->x)
        {
          spr_offset->x = sprite->SWidth - 1;
        }
        if (spr_offset->y < 0)
        {
          spr_offset->y = 0;
        } else
        if (spr_offset->y >= sprite->SHeight)
        {
          spr_offset->y = sprite->SHeight - 1;
        }
    }
}

void LbI_PointerHandler::Initialise(const struct TbSprite *spr, struct TbPoint *npos, struct TbPoint *noffset)
{
    void *surfbuf;
    TbPixel *buf;
    long i;
    int dstwidth;
    int dstheight;
    Release();
    LbSemaLock semlock(&sema_rel,0);
    semlock.Lock(true);
    sprite = spr;
    dstwidth = scale_ui_value_lofi(sprite->SWidth) + 1;
    dstheight = scale_ui_value_lofi(sprite->SHeight) + 1;
    LbScreenSurfaceCreate(&surf1, dstwidth, dstheight);
    LbScreenSurfaceCreate(&surf2, dstwidth, dstheight);
    surfbuf = LbScreenSurfaceLock(&surf1);
    if (surfbuf == NULL)
    {
        LbScreenSurfaceRelease(&surf1);
        LbScreenSurfaceRelease(&surf2);
        sprite = NULL;
        return;
    }
    buf = (TbPixel *)surfbuf;
    for (i=0; i < dstheight; i++)
    {
        memset(buf, 255, surf1.pitch);
        buf += surf1.pitch;
    }
    PointerDraw(0, 0, this->sprite, (TbPixel *)surfbuf, surf1.pitch);
    LbScreenSurfaceUnlock(&surf1);
    this->position = npos;
    this->spr_offset = noffset;
    ClipHotspot();
    this->field_1050 = true;
    NewMousePos();
    this->field_1054 = false;
    LbScreenSurfaceBlit(&surf2, this->draw_pos_x, this->draw_pos_y, &rect_1038, 0x10|0x02);
}

void LbI_PointerHandler::Draw(bool a1)
{
    unsigned long flags;
    flags = 0x10 | 0x08 | 0x04;
    if ( a1 )
      flags |= 0x02;
    LbScreenSurfaceBlit(&this->surf1, this->draw_pos_x, this->draw_pos_y, &rect_1038, flags);
}

void LbI_PointerHandler::Backup(bool a1)
{
    unsigned long flags;
    flags = 0x10;
    if ( a1 )
      flags |= 0x02;
    this->field_1054 = false;
    LbScreenSurfaceBlit(&this->surf2, this->draw_pos_x, this->draw_pos_y, &rect_1038, flags);
}

void LbI_PointerHandler::Undraw(bool a1)
{
    unsigned long flags;
    flags = 0x10 | 0x08;
    if ( a1 )
      flags |= 0x02;
    LbScreenSurfaceBlit(&this->surf2, this->draw_pos_x, this->draw_pos_y, &rect_1038, flags);
}

void LbI_PointerHandler::Release(void)
{
    LbSemaLock semlock(&sema_rel,0);
    semlock.Lock(true);
    if ( this->field_1050 )
    {
        if ( lbInteruptMouse )
            Undraw(true);
        this->field_1050 = false;
        this->field_1054 = false;
        position = NULL;
        sprite = NULL;
        spr_offset = NULL;
        LbScreenSurfaceRelease(&surf1);
        LbScreenSurfaceRelease(&surf2);
    }
}

void LbI_PointerHandler::NewMousePos(void)
{
    this->draw_pos_x = position->x - scale_ui_value_lofi(spr_offset->x);
    this->draw_pos_y = position->y - scale_ui_value_lofi(spr_offset->y);
    int dstwidth;
    int dstheight;
    dstwidth = scale_ui_value_lofi(sprite->SWidth);
    dstheight = scale_ui_value_lofi(sprite->SHeight);
    LbSetRect(&rect_1038, 0, 0, dstwidth, dstheight);
    if (this->draw_pos_x < 0)
    {
        rect_1038.left -= this->draw_pos_x;
        this->draw_pos_x = 0;
    } else
    if (this->draw_pos_x+dstwidth > lbDisplay.PhysicalScreenWidth)
    {
        rect_1038.right += lbDisplay.PhysicalScreenWidth-dstwidth-this->draw_pos_x;
    }
    if (this->draw_pos_y < 0)
    {
        rect_1038.top -= this->draw_pos_y;
        this->draw_pos_y = 0;
    } else
    if (this->draw_pos_y+dstheight > lbDisplay.PhysicalScreenHeight)
    {
        rect_1038.bottom += lbDisplay.PhysicalScreenHeight - dstheight - this->draw_pos_y;
    }
}

bool LbI_PointerHandler::OnMove(void)
{
    LbSemaLock semlock(&sema_rel,0);
    if (!semlock.Lock(true))
        return false;
    if (lbPointerAdvancedDraw && lbInteruptMouse)
    {
        Undraw(true);
        NewMousePos();
        Backup(true);
        Draw(true);
    } else
    {
        NewMousePos();
    }
    return true;
}

void LbI_PointerHandler::OnBeginPartialUpdate(void)
{
    LbSemaLock semlock(&sema_rel,0);
    if (!semlock.Lock(true))
        return;
    Backup(false);
    Draw(false);
}

void LbI_PointerHandler::OnEndPartialUpdate(void)
{
    LbSemaLock semlock(&sema_rel,1);
    Undraw(false);
    this->field_1054 = true;
    semlock.Release();
}

void LbI_PointerHandler::OnBeginSwap(void)
{
    LbSemaLock semlock(&sema_rel,0);
    if (!semlock.Lock(true))
      return;
    if ( lbPointerAdvancedDraw )
    {
        Backup(false);
        Draw(false);
    } else
    if (LbScreenLock() == Lb_SUCCESS)
    {
      PointerDraw(position->x - scale_ui_value_lofi(spr_offset->x), position->y - scale_ui_value_lofi(spr_offset->y),
          sprite, lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth);
      LbScreenUnlock();
    }
}

void LbI_PointerHandler::OnEndSwap(void)
{
    LbSemaLock semlock(&sema_rel,1);
    if ( lbPointerAdvancedDraw )
    {
        Undraw(false);
        this->field_1054 = true;
    }
    semlock.Release();
}

void LbI_PointerHandler::OnBeginFlip(void)
{
    LbSemaLock semlock(&sema_rel,0);
    if (!semlock.Lock(true))
        return;
    Backup(false);
    Draw(false);
}

void LbI_PointerHandler::OnEndFlip(void)
{
    LbSemaLock semlock(&sema_rel,1);
    semlock.Release();
}

/******************************************************************************/
