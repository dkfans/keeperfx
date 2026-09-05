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
#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "bflib_mspointer.hpp"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_planar.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"

#include "keeperfx.hpp"
#include "post_inc.h"
/******************************************************************************/

// Methods

LbI_PointerHandler::LbI_PointerHandler(void)
{
    this->is_active = false;
    this->needs_redraw = false;
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
    std::lock_guard<std::mutex> guard(lock);
    if (this->is_active)
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
    if (!this->is_active)
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
    Release();
    std::lock_guard<std::mutex> guard(lock);
    sprite = spr;
    position = npos;
    spr_offset = noffset;
    ClipHotspot();
    this->is_active = true;
    this->needs_redraw = false;
    NewMousePos();
}

void LbI_PointerHandler::Draw(bool a1)
{
    (void)a1;
}

void LbI_PointerHandler::Backup(bool a1)
{
    (void)a1;
    this->needs_redraw = false;
}

void LbI_PointerHandler::Undraw(bool a1)
{
    (void)a1;
}

void LbI_PointerHandler::Release(void)
{
    std::lock_guard<std::mutex> guard(lock);
    if ( this->is_active )
    {
        this->is_active = false;
        this->needs_redraw = false;
        position = NULL;
        sprite = NULL;
        spr_offset = NULL;
        CursorLayer_Clear();
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
    if (this->draw_pos_x+dstwidth > RendererPhysicalWidth())
    {
        rect_1038.right += RendererPhysicalWidth()-dstwidth-this->draw_pos_x;
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
    std::lock_guard<std::mutex> guard(lock);
    NewMousePos();
    return true;
}

void LbI_PointerHandler::ComputeDrawParams(int32_t *out_x, int32_t *out_y, int *out_units_per_px)
{
    *out_x = position->x - scale_ui_value_lofi(spr_offset->x);
    *out_y = position->y - scale_ui_value_lofi(spr_offset->y);
    *out_units_per_px = (sprite->SWidth > 0)
        ? (int)(scale_ui_value_lofi(sprite->SWidth) * 16 / sprite->SWidth)
        : 16;
}

void LbI_PointerHandler::OnBeginSwap(void)
{
    std::lock_guard<std::mutex> guard(lock);
    if (sprite == NULL || !is_active)
    {
        CursorLayer_Clear();
        return;
    }
    int32_t cx, cy;
    int units_per_px;
    ComputeDrawParams(&cx, &cy, &units_per_px);
    CursorLayer_SubmitPointerSprite(sprite, cx, cy, units_per_px);
}

void LbI_PointerHandler::OnEndSwap(void)
{
    // No-op: for a backend that defers, cursor rendering happens entirely in
    // CursorLayer_Draw() at end of frame. No restore needed.
}

bool LbI_PointerHandler::GetSpriteForDraw(const struct TbSprite **out_spr, int32_t *out_x, int32_t *out_y, int *out_units_per_px)
{
    std::lock_guard<std::mutex> guard(lock);
    if (sprite == NULL || !is_active)
        return false;
    *out_spr = sprite;
    ComputeDrawParams(out_x, out_y, out_units_per_px);
    return true;
}

/******************************************************************************/
