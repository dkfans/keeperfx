/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mshandler.cpp
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
#include "bflib_mshandler.hpp"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"

#include "keeperfx.hpp"
/******************************************************************************/
// Global variables
int volatile lbMouseSpriteInstalled = false;
int volatile lbMouseOffline = false;

class MouseStateHandler pointerHandler;

MouseStateHandler::MouseStateHandler(void)
{
    Release();
}

MouseStateHandler::~MouseStateHandler(void)
{
}

bool MouseStateHandler::Install(void)
{
    this->_mouseSpriteInstalled = true;
    return true;
}

bool MouseStateHandler::IsInstalled(void)
{
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
    {
        return false;
    }
    return true;
}

bool MouseStateHandler::Release(void)
{
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
    {
        return false;
    }

    lbMouseSpriteInstalled = false;
    lbDisplay.MouseSprite = NULL;
    this->_mouseSpriteInstalled = false;
    _mouseSprite = NULL;

    _pointer.Release();
    _mouseSpritePos.x = 0;
    _mouseSpritePos.y = 0;
    _hotspot.x = 0;
    _hotspot.y = 0;

    return true;
}

bool MouseStateHandler::SetMouseWindow(long x, long y, long width, long height)
{
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
    {
        return false;
    }

    lbDisplay.MouseWindowX = x;
    lbDisplay.MouseWindowY = y;
    lbDisplay.MouseWindowWidth = width;
    lbDisplay.MouseWindowHeight = height;
    return true;
}

// Clip the coordinate into mouse window, and redraw the sprite.
bool MouseStateHandler::_setSpritePosition(long x, long y)
{
    long prev_x, prev_y;
    long mx, my;
    if (!this->_mouseSpriteInstalled)
    {
        return false;
    }

    // Clip coordinates into our mouse window
    mx = x;
    if (x < lbDisplay.MouseWindowX)
    {
        mx = lbDisplay.MouseWindowX;
    }
    else if (x >= lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth)
    {
        mx = lbDisplay.MouseWindowWidth + lbDisplay.MouseWindowX - 1;
    }

    my = y;
    if (y < lbDisplay.MouseWindowY)
    {
        my = lbDisplay.MouseWindowY;
    }
    else if (y >= lbDisplay.MouseWindowHeight + lbDisplay.MouseWindowY)
    {
        my = lbDisplay.MouseWindowHeight + lbDisplay.MouseWindowY - 1;
    }

    // If the coords are unchanged
    if ((mx == lbDisplay.MMouseX) && (my == lbDisplay.MMouseY))
    {
        return true;
    }

    // Change the sprite position.
    prev_x = _mouseSpritePos.x;
    prev_y = _mouseSpritePos.y;
    _mouseSpritePos.x = mx;
    _mouseSpritePos.y = my;

    if ((_mouseSprite != NULL) && (this->_mouseSpriteInstalled))
    {
        // show_onscreen_msg(5, "POS %3d x %3d CLIP %3d x %3d WINDOW %3d x %3d", x,y,mx,my,lbDisplay.MouseWindowX,lbDisplay.MouseWindowY);
        if (!_pointer.OnMove())
        {
            _mouseSpritePos.x = prev_x;
            _mouseSpritePos.y = prev_y;
            return false;
        }
    }
    return true;
}

// Sets the position of logical mouse pointer, and draw the sprite accordingly.
bool MouseStateHandler::SetMousePosition(long x, long y)
{
    long mx, my;
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
    {
        return false;
    }
    if (!this->_setSpritePosition(x, y))
    {
        return false;
    }
    if (this->_mouseSpriteInstalled)
    {
        mx = _mouseSpritePos.x;
        my = _mouseSpritePos.y;
    }
    else
    {
        mx = x;
        my = y;
    }
    lbDisplay.MMouseX = mx;
    lbDisplay.MMouseY = my;
    return true;
}

bool MouseStateHandler::_setPointer(struct TbSprite *sprite, struct TbPoint *hotspot)
{
    if (!this->_mouseSpriteInstalled)
    {
        return false;
    }
    _mouseSprite = sprite;
    _hotspot.y = 0;
    _hotspot.x = 0;
    if ((sprite != NULL) && (sprite->SWidth != 0) && (sprite->SHeight != 0))
    {
        if (hotspot != NULL)
        {
            _hotspot.x = hotspot->x;
            _hotspot.y = hotspot->y;
        }

        _pointer.Initialise(sprite, &_mouseSpritePos, &_hotspot);

        if ((_mouseSprite != NULL) && (this->_mouseSpriteInstalled))
        {
            _pointer.OnMove();
        }
    }
    else
    {
        _pointer.Release();
        _mouseSprite = NULL;
    }
    return true;
}

bool MouseStateHandler::SetMouseSpriteAndOffset(struct TbSprite *mouseSprite, long x, long y)
{
    struct TbPoint point;
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
        return false;
    if (mouseSprite == lbDisplay.MouseSprite)
        return true;
    if (mouseSprite != NULL)
        if ((mouseSprite->SWidth > 64) || (mouseSprite->SHeight > 64))
        {
            WARNLOG("Mouse _pointer too large");
            return false;
        }
    lbDisplay.MouseSprite = mouseSprite;
    point.x = x;
    point.y = y;
    return this->_setPointer(mouseSprite, &point);
}

bool MouseStateHandler::SetMouseSprite(struct TbSprite *mouseSprite)
{
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
        return false;
    if (mouseSprite == lbDisplay.MouseSprite)
        return true;
    if (mouseSprite != NULL)
        if ((mouseSprite->SWidth > 64) || (mouseSprite->SHeight > 64))
        {
            WARNLOG("Mouse _pointer too large");
            return false;
        }
    lbDisplay.MouseSprite = mouseSprite;
    this->_setPointer(mouseSprite, NULL);
    return true;
}

// We don't always use upleft corner of sprite as 'hotspot'(accurate point of cursor),
// This method sets the offset of this hotspot.
bool MouseStateHandler::SetMouseSpriteOffset(long x, long y)
{
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
        return false;
    if (this->_mouseSpriteInstalled)
        _pointer.SetHotspot(x, y);
    return true;
}

// We don't always use upleft corner of sprite as 'hotspot'(accurate point of cursor),
// This method gets the offset of this hotspot.
struct TbPoint *MouseStateHandler::GetMouseSpriteOffset(void)
{
    return &_hotspot;
}

bool MouseStateHandler::PointerBeginSwap(void)
{
    LbSemaLock semlock(&_semaphore, 0);
    if (!semlock.Lock(true))
        return false;
    if ((!lbMouseSpriteInstalled) || (lbMouseOffline))
        return true;
    if ((_mouseSprite != NULL) && (this->_mouseSpriteInstalled))
    {
        _swap = true;
        _pointer.OnBeginSwap();
    }
    return true;
}

bool MouseStateHandler::PointerEndSwap(void)
{
    LbSemaLock semlock(&_semaphore, 1);
    if (!lbMouseSpriteInstalled)
        return true;
    if ((_mouseSprite != NULL) && (this->_mouseSpriteInstalled))
    {
        if (_swap)
        {
            _swap = false;
            _pointer.OnEndSwap();
        }
    }
    semlock.Release();
    return true;
}
/******************************************************************************/
