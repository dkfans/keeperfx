/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 12 Feb 2008

// Purpose:
//    Mouse related routines.

// Comment:
//   None yet.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#include "bflib_mouse.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
bool lbMouseInstalled=false;
struct mouse_buffer mbuffer;
struct mouse_info minfo;
char ptr[4096];
unsigned int redraw_active=0;
unsigned int mouse_initialised=0;
short volatile mouse_mickey_x;
short volatile mouse_mickey_y;
long volatile mouse_dx;
long volatile mouse_dy;
unsigned long mouse_pos_change_saved;
struct DevInput joy;
/******************************************************************************/
int __fastcall LbMouseSetWindow(int x, int y, int width, int height)
{
  if ( !lbMouseInstalled )
    return -1;
  lbDisplay.MouseWindowX = x;
  lbDisplay.MouseWindowY = y;
  lbDisplay.MouseWindowWidth = width;
  lbDisplay.MouseWindowHeight = height;
  adjust_point(&lbDisplay.MMouseX, &lbDisplay.MMouseY);
  adjust_point(&lbDisplay.MouseX, &lbDisplay.MouseY);
  return 1;
}

int __fastcall LbMouseChangeMoveRatio(int x, int y)
{
  if ( !lbMouseInstalled )
    return -1;
  if ( (x<1) || (x>63) )
    return -1;
  if ( (y<1) || (y>63) )
    return -1;
  minfo.XMoveRatio = x;
  minfo.YMoveRatio = y;
  return 1;
}

//Places mouse sprite on the screen.
//Requires screen to be locked before calling.
int __fastcall LbMousePlace(void)
{
  redraw_active = 1;
  if ( lbDisplay.MouseSprite == NULL )
    return 1;
  mbuffer.X = minfo.XSpriteOffset + lbDisplay.MMouseX;
  mbuffer.Y = minfo.YSpriteOffset + lbDisplay.MMouseY;
  mbuffer.Valid = mouse_setup_range();
  if ( mbuffer.Valid != true )
    return 1;
  unsigned char *buf_ptr;
  unsigned char *bspr_ptr;
  unsigned char *bscr_ptr;
  buf_ptr = mbuffer.Buffer;
  bspr_ptr = &minfo.Sprite[mbuffer.XOffset + lbDisplay.MouseSprite->SWidth*mbuffer.YOffset];
  mbuffer.Offset = (mbuffer.YOffset+mbuffer.Y)*lbDisplay.GraphicsScreenWidth
          + mbuffer.XOffset+mbuffer.X;
  bscr_ptr = &lbDisplay.WScreen[mbuffer.Offset];
  unsigned int c1,c2;
  for (c1=0;c1<mbuffer.Height;c1++)
  {
        unsigned char *scr_ptr;
        unsigned char *spr_ptr;
        scr_ptr = bscr_ptr;
        spr_ptr = bspr_ptr;
        c2 = 0;
        while ( c2 < mbuffer.Width )
        {
          *buf_ptr = *scr_ptr;
          if ( *spr_ptr != 254 )
            *scr_ptr = *spr_ptr;
          c2++;
          buf_ptr++;
          scr_ptr++;
          spr_ptr++;
        }
        bscr_ptr += lbDisplay.GraphicsScreenWidth;
        bspr_ptr += lbDisplay.MouseSprite->SWidth;
  }
  return 1;
}

int __fastcall screen_place(void)
{
/*todo
*/
  return 1;
}

int __fastcall screen_remove(unsigned long force)
{
  if ( !lbMouseInstalled )
    return -1;
  if ( (lbDisplay.MMouseX==mbuffer.X) && (lbDisplay.MMouseY==mbuffer.Y) && (!force) )
    return 1;
  if ( mbuffer.Valid )
  {
      mbuffer.Valid = 0;
  }
  return 1;
}

bool mouse_setup_range(void)
{
  if (lbDisplay.MouseSprite==NULL)
    return false;
  mbuffer.Width = lbDisplay.MouseSprite->SWidth;
  mbuffer.Height = lbDisplay.MouseSprite->SHeight;
  mbuffer.XOffset = 0;
  mbuffer.YOffset = 0;
  //Basic range checking
  if ( (mbuffer.X<=(-mbuffer.Width)) || (mbuffer.X>=lbDisplay.GraphicsScreenWidth) )
    return false;
  if ( (mbuffer.Y<=(-mbuffer.Height)) || (mbuffer.Y>=lbDisplay.GraphicsScreenHeight) )
    return false;
  // Adjusting position
  if ( mbuffer.X < 0 )
  {
      mbuffer.XOffset = -mbuffer.X;
      mbuffer.Width += mbuffer.X;
  }
  if ( mbuffer.Width + mbuffer.X > lbDisplay.GraphicsScreenWidth )
      mbuffer.Width = lbDisplay.GraphicsScreenWidth - mbuffer.X;
  if ( mbuffer.Y < 0 )
  {
      mbuffer.YOffset = -mbuffer.Y;
      mbuffer.Height += mbuffer.Y;
  }
  if ( mbuffer.Height + mbuffer.Y > lbDisplay.GraphicsScreenHeight )
      mbuffer.Height = lbDisplay.GraphicsScreenHeight - mbuffer.Y;
  return true;
}

//Adjusts point coordinates; returns true if the coordinates have changed.
bool __fastcall adjust_point(long *x, long *y)
{
  bool result = false;
  if ( *x >= lbDisplay.MouseWindowX )
  {
    if ( lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth <= *x )
    {
      *x = lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth - 1;
      result = true;
    }
  } else
  {
    *x = lbDisplay.MouseWindowX;
    result = true;
  }
  if ( *y >= lbDisplay.MouseWindowY )
  {
    if ( lbDisplay.MouseWindowY + lbDisplay.MouseWindowHeight <= *y )
    {
      *y = lbDisplay.MouseWindowY + lbDisplay.MouseWindowHeight - 1;
      result = true;
    }
  } else
  {
    *y = lbDisplay.MouseWindowY;
    result = true;
  }
  return result;
}

//Returns if the current mouse position is inside of given rectangle
char __fastcall mouse_in_rect(short x1, short x2, short y1, short y2)
{
  return (x1<=lbDisplay.MMouseX) && (x2>lbDisplay.MMouseX) &&
         (y1<=lbDisplay.MMouseY) && (y2>lbDisplay.MMouseY);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
