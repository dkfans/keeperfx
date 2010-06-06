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
#include "bflib_mouse.h"
#include "bflib_sprite.h"

#include "keeperfx.hpp"
/******************************************************************************/
struct SSurface;

#ifdef __cplusplus
extern "C" {
#endif
DLLIMPORT __cdecl long _DK_PointerDraw(long x, long y, struct TbSprite *spr, unsigned char *a4, unsigned long a5);
#ifdef __cplusplus
}
#endif
/******************************************************************************/
// Global variables

/******************************************************************************/
void ClearSurface(struct SSurface *surf) // this was originally TSurface constructor
{
  surf->surf = NULL;
  surf->pitch= 0;
}

/**
 * Draws the mouse pointer sprite on a display buffer.
 */
long PointerDraw(long x, long y, struct TbSprite *spr, TbPixel *buf, unsigned long a5)
{
  //show_onscreen_msg(5, "POS %3d x %3d", x,y);
  return _DK_PointerDraw(x,y,spr,buf,a5);
}

// Methods

LbI_PointerHandler::LbI_PointerHandler(void)
{
  ClearSurface(&surf1);
  ClearSurface(&surf2);
  this->field_1050 = false;
  this->field_1054 = false;
  this->sprite = NULL;
  this->position = NULL;
  this->spr_offset = NULL;
}

LbI_PointerHandler::~LbI_PointerHandler(void)
{
  Release();
}

void LbI_PointerHandler::SetHotspot(long x, long y)
{
  long prev_x,prev_y;
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
      NewMousePos(position->x, position->y);
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

void LbI_PointerHandler::Initialise(struct TbSprite *spr, struct tagPOINT *npos, struct tagPOINT *noffset)
{
  void *surfbuf;
  TbPixel *buf;
  long i;

  Release();
  if (lpDDC == NULL)
    return;
  sprite = spr;
  lpDDC->create_surface(&surf1, sprite->SWidth, sprite->SHeight);
  lpDDC->create_surface(&surf2, sprite->SWidth, sprite->SHeight);

  surfbuf = lpDDC->lock_surface(&surf1);
  if (surfbuf == NULL)
  {
    lpDDC->release_surface(&surf1);
    lpDDC->release_surface(&surf2);
    sprite = NULL;
    return;
  }
  buf = (TbPixel *)surfbuf;
  for (i=0; i < sprite->SHeight; i++)
  {
    memset(buf, 255, surf1.pitch);
    buf += surf1.pitch;
  }
  PointerDraw(0, 0, this->sprite, (TbPixel *)surfbuf, surf1.pitch);
  lpDDC->unlock_surface(&surf1);
  this->position = npos;
  this->spr_offset = noffset;
  ClipHotspot();
  this->field_1050 = true;
  NewMousePos(npos->x, npos->y);
  this->field_1054 = false;

  lpDDC->blt_surface(&surf2, this->draw_pos_x, this->draw_pos_y, &drawRect, 0x10|0x02);
}

void LbI_PointerHandler::Draw(bool a1)
{
  unsigned long flags;
  flags = 0x10 | 0x08 | 0x04;
  if ( a1 )
    flags |= 0x02;
  if (lpDDC == NULL)
    return;
  lpDDC->blt_surface(&this->surf1, this->draw_pos_x, this->draw_pos_y, &drawRect, flags);
}

void LbI_PointerHandler::Backup(bool a1)
{
  unsigned long flags;
  flags = 0x10;
  if ( a1 )
    flags |= 0x02;
  if (lpDDC == NULL)
    return;
  this->field_1054 = false;
  lpDDC->blt_surface(&this->surf2, this->draw_pos_x, this->draw_pos_y, &drawRect, flags);
}

void LbI_PointerHandler::Undraw(bool a1)
{
  unsigned long flags;
  flags = 0x10 | 0x08;
  if ( a1 )
    flags |= 0x02;
  if (lpDDC == NULL)
    return;
  lpDDC->blt_surface(&this->surf2, this->draw_pos_x, this->draw_pos_y, &drawRect, flags);
}

void LbI_PointerHandler::Release(void)
{
  if ( this->field_1050 )
  {
    if ( lbInteruptMouse )
      Undraw(true);
    this->field_1050 = false;
    this->field_1054 = false;
    position = NULL;
    sprite = NULL;
    spr_offset = NULL;
    if (lpDDC != NULL)
    {
      lpDDC->release_surface(&surf1);
      lpDDC->release_surface(&surf2);
    }
  }
}

void LbI_PointerHandler::NewMousePos(int x, int y)
{
	if (sprite == NULL) {
		return;
	}

	draw_pos_x = x - spr_offset->x;
	draw_pos_y = y - spr_offset->y;

	SetRect(&drawRect, 0, 0, sprite->SWidth, sprite->SHeight);
	if (draw_pos_x < 0) {
		drawRect.left -= draw_pos_x;
		draw_pos_x = 0;
	}
	else if (draw_pos_x+sprite->SWidth > lbDisplay.PhysicalScreenWidth) {
		drawRect.right += lbDisplay.PhysicalScreenWidth - sprite->SWidth - draw_pos_x;
	}
	if (draw_pos_y < 0) {
		drawRect.top -= draw_pos_y;
		draw_pos_y = 0;
	}
	else if (draw_pos_y+sprite->SHeight > lbDisplay.PhysicalScreenHeight) {
		drawRect.bottom += lbDisplay.PhysicalScreenHeight - sprite->SHeight - draw_pos_y;
	}
}

bool LbI_PointerHandler::OnMove(int x, int y)
{
  if (lbUseSdk && lbInteruptMouse)
  {
      Undraw(true);
      NewMousePos(x, y);
      Backup(true);
      Draw(true);
  } else
  {
    NewMousePos(x, y);
  }
  return true;
}

void LbI_PointerHandler::OnBeginPartialUpdate(void)
{
  Backup(false);
  Draw(false);
}

void LbI_PointerHandler::OnEndPartialUpdate(void)
{
	Undraw(false);
	this->field_1054 = true;
}

void LbI_PointerHandler::OnBeginSwap(void)
{
	Draw(false);
}

void LbI_PointerHandler::OnEndSwap(void) //not called any longer
{
	Undraw(false);
	this->field_1054 = true;
}

void LbI_PointerHandler::OnBeginFlip(void)
{
  Backup(false);
  Draw(false);
}

void LbI_PointerHandler::OnEndFlip(void)
{
}

/******************************************************************************/
