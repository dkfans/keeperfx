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
  _DK_lbDisplay.MouseWindowX = x;
  _DK_lbDisplay.MouseWindowY = y;
  _DK_lbDisplay.MouseWindowWidth = width;
  _DK_lbDisplay.MouseWindowHeight = height;
  adjust_point(&_DK_lbDisplay.MMouseX, &_DK_lbDisplay.MMouseY);
  adjust_point(&_DK_lbDisplay.MouseX, &_DK_lbDisplay.MouseY);
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

int __fastcall LbMouseSetup(struct TbSprite *MouseSprite, char x_ratio, char y_ratio)
{
  if ( lbMouseInstalled )
    return -1;
/*
  if ( !mouse_initialised )
  {
    if (segread_(), inregs = v5, int386_(), v11 != 65535)
    {
      return -1;
    }
  }
*/
  lbMouseInstalled = true;
  minfo.XMoveRatio = 1;
  minfo.YMoveRatio = 1;
  minfo.XSpriteOffset = 0;
  minfo.YSpriteOffset = 0;
  memset(minfo.Sprite, 254, 4096);
  _DK_lbDisplay.MouseSprite = NULL;
  redraw_active = 0;
  LbMemorySet(&mbuffer, 0, sizeof(struct mouse_buffer));
  if (LbMouseSetWindow(0,0,_DK_lbDisplay.GraphicsScreenWidth,_DK_lbDisplay.GraphicsScreenHeight)!=1)
  {
      lbMouseInstalled = 0;
      return -1;
  }
  if (LbMouseChangeMoveRatio(x_ratio,y_ratio)!=1)
  {
      lbMouseInstalled = 0;
      return -1;
  }
  if ( LbMouseSetPosition(_DK_lbDisplay.GraphicsScreenWidth>>1, _DK_lbDisplay.GraphicsScreenHeight>>1) != 1 )
  {
      lbMouseInstalled = 0;
      return -1;
  }
  if (LbMouseChangeSprite(MouseSprite)!=1)
  {
      lbMouseInstalled = 0;
      return -1;
  }
  if ( !mouse_initialised )
  {
/*
      sregs = __CS__;
      int386x_();
*/
      mouse_initialised = true;
  }
  return 1;
}

int __fastcall LbMouseReset()
{
  if ( !lbMouseInstalled )
    return -1;
  redraw_active = 1;
  if ( !lbScreenDirectAccessActive )
      screen_remove(1u);
  LbMemorySet(&mbuffer, 0, sizeof(struct mouse_buffer));
/*
  int386_();
*/
  mouse_initialised = 0;
  _DK_lbDisplay.MouseSprite = NULL;
  lbMouseInstalled = false;
  redraw_active = 0;
  return 1;
}

int __fastcall LbMouseSuspend(void)
{
  if ( !lbMouseInstalled )
    return -1;
  redraw_active = 1;
  if ( !lbScreenDirectAccessActive )
      screen_remove(1u);
  LbMemorySet(&mbuffer, 0, sizeof(struct mouse_buffer));
  _DK_lbDisplay.MouseSprite = NULL;
  lbMouseInstalled = false;
  redraw_active = 0;
  return 1;
}

//Places mouse sprite on the screen.
//Requires screen to be locked before calling.
int __fastcall LbMousePlace(void)
{
  redraw_active = 1;
  if ( _DK_lbDisplay.MouseSprite == NULL )
    return 1;
  mbuffer.X = minfo.XSpriteOffset + _DK_lbDisplay.MMouseX;
  mbuffer.Y = minfo.YSpriteOffset + _DK_lbDisplay.MMouseY;
  mbuffer.Valid = mouse_setup_range();
  if ( mbuffer.Valid != true )
    return 1;
  unsigned char *buf_ptr;
  unsigned char *bspr_ptr;
  unsigned char *bscr_ptr;
  buf_ptr = mbuffer.Buffer;
  bspr_ptr = &minfo.Sprite[mbuffer.XOffset + _DK_lbDisplay.MouseSprite->SWidth*mbuffer.YOffset];
  mbuffer.Offset = (mbuffer.YOffset+mbuffer.Y)*_DK_lbDisplay.GraphicsScreenWidth
          + mbuffer.XOffset+mbuffer.X;
  bscr_ptr = &_DK_lbDisplay.WScreen[mbuffer.Offset];
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
        bscr_ptr += _DK_lbDisplay.GraphicsScreenWidth;
        bspr_ptr += _DK_lbDisplay.MouseSprite->SWidth;
  }
  return 1;
}

//Removes mouse from the screen.
//Requires screen to be locked before calling.
int __fastcall LbMouseRemove(void)
{
  if ( (mbuffer.Valid) && (_DK_lbDisplay.WScreen!=NULL) )
  {
    unsigned char *scr_ptr;
    unsigned char *bscr_ptr;
    unsigned char *spr_ptr;
    int c1,c2;
    spr_ptr = mbuffer.Buffer;
    bscr_ptr = &_DK_lbDisplay.WScreen[mbuffer.Offset];
    for (c1=0;c1<mbuffer.Height;c1++)
    {
      scr_ptr = bscr_ptr;
      for (c2=0;c2<mbuffer.Width;c2++)
      {
        *scr_ptr=*spr_ptr;
        spr_ptr++;
        scr_ptr++;
      }
      bscr_ptr += _DK_lbDisplay.GraphicsScreenWidth;
    }
  }
  if ( mouse_pos_change_saved )
  {
    _DK_lbDisplay.MMouseX += mouse_dx;
    _DK_lbDisplay.MMouseY += mouse_dy;
    adjust_point(&_DK_lbDisplay.MMouseX, &_DK_lbDisplay.MMouseY);
    if ( !lbScreenDirectAccessActive )
    {
      screen_remove(0);
      screen_place();
    }
    mouse_pos_change_saved = 0;
  }
  redraw_active = 0;
  return 1;
}

int __fastcall LbMouseSetPosition(long x, long y)
{
  if ( !lbMouseInstalled )
    return -1;
  redraw_active = 1;
  if ( !lbScreenDirectAccessActive )
    screen_remove(1u);
  adjust_point(&x, &y);
  _DK_lbDisplay.MMouseX = x;
  _DK_lbDisplay.MouseX = _DK_lbDisplay.MMouseX;
  _DK_lbDisplay.MMouseY = y;
  _DK_lbDisplay.MouseY = _DK_lbDisplay.MMouseY;
  if ( !lbScreenDirectAccessActive )
    screen_place();
  redraw_active = 0;
  return 1;
}

int __fastcall LbMouseChangeSprite(struct TbSprite *pointer)
{
  if ( !lbMouseInstalled )
      return -1;
  //Setting same pointer more than one
  if ( _DK_lbDisplay.MouseSprite == pointer )
      return 1;
  //Size limitation
  if ( (pointer!=NULL) && ((pointer->SWidth>64)||(pointer->SHeight>64)) )
      return -1;
  redraw_active = 1;
  if ( !lbScreenDirectAccessActive )
    screen_remove(1u);
  _DK_lbDisplay.MouseSprite = pointer;
  memset(minfo.Sprite, 254, 4096);
  if ( pointer != NULL )
  {
      unsigned char *wscr_backup;
      int gwx,gwy;
      int gww,gwh,gsw;
      unsigned short dflags;
      wscr_backup = _DK_lbDisplay.WScreen;
      _DK_lbDisplay.WScreen = minfo.Sprite;
      gwx = _DK_lbDisplay.GraphicsWindowX;
      gwy = _DK_lbDisplay.GraphicsWindowY;
      gww = _DK_lbDisplay.GraphicsWindowWidth;
      gwh = _DK_lbDisplay.GraphicsWindowHeight;
      gsw = _DK_lbDisplay.GraphicsScreenWidth;
      dflags = _DK_lbDisplay.DrawFlags;
      _DK_lbDisplay.GraphicsScreenWidth = pointer->SWidth;
      _DK_lbDisplay.DrawFlags = 0;
      LbScreenSetGraphicsWindow(0, 0, pointer->SWidth, pointer->SHeight);
      _DK_lbDisplay.WScreen = wscr_backup;
      _DK_lbDisplay.GraphicsScreenWidth = gsw;
      _DK_lbDisplay.DrawFlags = dflags;
      LbScreenSetGraphicsWindow(gwx, gwy, gww, gwh);
  }
  if ( !lbScreenDirectAccessActive )
      screen_place();
  redraw_active = 0;
  return 1;
}

inline void __fastcall MouseHandlerMove(int mickey_x,int mickey_y)
{
    int old_mx;
    int old_my;
    static int old_rx;
    static int old_ry;
    old_mx=mouse_mickey_x;
    old_my=mouse_mickey_y;
    mouse_mickey_x=mickey_x;
    mouse_mickey_y=mickey_y;
    int dtx = mouse_mickey_x-old_mx;
    if (dtx!=0)
    {
        mouse_dx=dtx/minfo.XMoveRatio;
        old_rx+=dtx%minfo.XMoveRatio;
        //Note: I'm not sure here...
        if (old_rx<0)
        {
          old_rx=minfo.XMoveRatio + (dtx%minfo.XMoveRatio);
          mouse_dx--;
        }
    mouse_pos_change_saved=1;
    }
    int dty = mouse_mickey_y-old_my;
    if (dty!=0)
    {
        mouse_dy=dty/minfo.YMoveRatio;
        old_ry+=dty%minfo.YMoveRatio;
        //Note: I'm not sure here...
        if (old_ry<0)
        {
          old_ry=minfo.YMoveRatio + (dty%minfo.YMoveRatio);
          mouse_dy--;
        }
    mouse_pos_change_saved=1;
    }
    if (redraw_active==0)
    {
      _DK_lbDisplay.MMouseX+=mouse_dx;
      _DK_lbDisplay.MMouseY+=mouse_dy;
      bool remove=adjust_point(&_DK_lbDisplay.MMouseX,&_DK_lbDisplay.MMouseY);
      int vesa_page=lbVesaPage;
      screen_remove(remove);
      screen_place();
      if (_DK_lbDisplay.VesaIsSetUp)
        LbVesaSetPage(vesa_page);
      mouse_pos_change_saved=0;
    }
}

//DOS interrupt handler for the mouse driver
void __fastcall MouseHandler256(int mx, int my, int event)
{
  if (!lbMouseInstalled)
    return;
  if (event != 0x01)
  {
    if (event & 0x02)
    {
      _DK_lbDisplay.MLeftButton=1;
      if (_DK_lbDisplay.LeftButton==0)
      {
        _DK_lbDisplay.LeftButton=1;
        _DK_lbDisplay.MouseX=_DK_lbDisplay.MMouseX;
        _DK_lbDisplay.MouseY=_DK_lbDisplay.MMouseY;
        _DK_lbDisplay.RLeftButton=0;
      }
    }
    if (event & 0x04)
    {
      _DK_lbDisplay.MLeftButton=0;
      if (_DK_lbDisplay.RLeftButton==0)
      {
        _DK_lbDisplay.RLeftButton=1;
        _DK_lbDisplay.RMouseX=_DK_lbDisplay.MMouseX;
        _DK_lbDisplay.RMouseY=_DK_lbDisplay.MMouseY;
      }
    }
    if (event & 0x08)
    {
      _DK_lbDisplay.MRightButton=1;
      if (_DK_lbDisplay.RightButton==0)
      {
        _DK_lbDisplay.RightButton=1;
        _DK_lbDisplay.MouseX=_DK_lbDisplay.MMouseX;
        _DK_lbDisplay.MouseY=_DK_lbDisplay.MMouseY;
        _DK_lbDisplay.RRightButton=0;
      }
    }
    if (event & 0x10)
    {
      _DK_lbDisplay.MRightButton=0;
      if (_DK_lbDisplay.RRightButton==0)
      {
        _DK_lbDisplay.RRightButton=1;
        _DK_lbDisplay.RMouseX=_DK_lbDisplay.MMouseX;
        _DK_lbDisplay.RMouseY=_DK_lbDisplay.MMouseY;
      }
    }

    if (event & 0x20)
    {
      _DK_lbDisplay.MMiddleButton=1;
      if (_DK_lbDisplay.MiddleButton==0)
      {
        _DK_lbDisplay.MiddleButton=1;
        _DK_lbDisplay.MouseX=_DK_lbDisplay.MMouseX;
        _DK_lbDisplay.MouseY=_DK_lbDisplay.MMouseY;
        _DK_lbDisplay.RMiddleButton=0;
      }
    }
    if (event & 0x40)
    {
      _DK_lbDisplay.MMiddleButton=0;
      if (_DK_lbDisplay.RMiddleButton==0)
      {
        _DK_lbDisplay.RMiddleButton=1;
        _DK_lbDisplay.RMouseX=_DK_lbDisplay.MMouseX;
        _DK_lbDisplay.RMouseY=_DK_lbDisplay.MMouseY;
      }
    }
  }
  if (event & 0x01)
  {
    MouseHandlerMove(mx,my);
  }
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
  if ( (_DK_lbDisplay.MMouseX==mbuffer.X) && (_DK_lbDisplay.MMouseY==mbuffer.Y) && (!force) )
    return 1;
  if ( mbuffer.Valid )
  {
      unsigned char *spr_ptr;
      unsigned char *bscr_ptr;
      int c1;
      unsigned char *scr_ptr;
      spr_ptr = mbuffer.Buffer;
/*TODO: make it write on screen buffer, not the physical one
      if ( _DK_lbDisplay.VesaIsSetUp )
      {
          unsigned int vesa_page = mbuffer.Offset >> 16;
          LbVesaSetPage(vesa_page);
          bscr_ptr = (mbuffer.Offset&0xffff) + _DK_lbDisplay.PhysicalScreen;
          int to_copy;
          int init_copy;
          for (c1=mbuffer.Height-1;c1>=0;c1--)
          {
            scr_ptr = bscr_ptr;
            to_copy = mbuffer.Width;
            if ( mbuffer.Width + bscr_ptr >= byte_B0000 )
            {
              init_copy = &byte_B0000[-scr_ptr];
              if ( (signed int)&byte_B0000[-scr_ptr] > 0 )
              {
                int c2;
                for (c2=0;c2<init_copy;c2++)
                {
                  *scr_ptr=*spr_ptr;
                  spr_ptr++;
                  scr_ptr++;
                }
                to_copy -= init_copy;
              }
              vesa_page++;
              LbVesaSetPage(vesa_page);
              scr_ptr -= byte_10000;
              bscr_ptr -= byte_10000;
            }
            int c2;
            for (c2=0;c2<to_copy;c2++)
            {
              *scr_ptr=*spr_ptr;
              spr_ptr++;
              scr_ptr++;
            }
            bscr_ptr += _DK_lbDisplay.PhysicalScreenWidth;
          }
      } else
      {
          bscr_ptr = _DK_lbDisplay.PhysicalScreen + mbuffer.Offset;
          for (c1=mbuffer.Height-1;c1>=0;c1--)
          {
            scr_ptr = bscr_ptr;
            int c2;
            for (c2=0;c2<mbuffer.Width;c2++)
            {
              *scr_ptr=*spr_ptr;
              spr_ptr++;
              scr_ptr++;
            }
            bscr_ptr += _DK_lbDisplay.PhysicalScreenWidth;
          }
      }
*/
      mbuffer.Valid = 0;
  }
  return 1;
}

bool mouse_setup_range(void)
{
  if (_DK_lbDisplay.MouseSprite==NULL)
    return false;
  mbuffer.Width = _DK_lbDisplay.MouseSprite->SWidth;
  mbuffer.Height = _DK_lbDisplay.MouseSprite->SHeight;
  mbuffer.XOffset = 0;
  mbuffer.YOffset = 0;
  //Basic range checking
  if ( (mbuffer.X<=(-mbuffer.Width)) || (mbuffer.X>=_DK_lbDisplay.GraphicsScreenWidth) )
    return false;
  if ( (mbuffer.Y<=(-mbuffer.Height)) || (mbuffer.Y>=_DK_lbDisplay.GraphicsScreenHeight) )
    return false;
  // Adjusting position
  if ( mbuffer.X < 0 )
  {
      mbuffer.XOffset = -mbuffer.X;
      mbuffer.Width += mbuffer.X;
  }
  if ( mbuffer.Width + mbuffer.X > _DK_lbDisplay.GraphicsScreenWidth )
      mbuffer.Width = _DK_lbDisplay.GraphicsScreenWidth - mbuffer.X;
  if ( mbuffer.Y < 0 )
  {
      mbuffer.YOffset = -mbuffer.Y;
      mbuffer.Height += mbuffer.Y;
  }
  if ( mbuffer.Height + mbuffer.Y > _DK_lbDisplay.GraphicsScreenHeight )
      mbuffer.Height = _DK_lbDisplay.GraphicsScreenHeight - mbuffer.Y;
  return true;
}

//Adjusts point coordinates; returns true if the coordinates have changed.
bool __fastcall adjust_point(long *x, long *y)
{
  bool result = false;
  if ( *x >= _DK_lbDisplay.MouseWindowX )
  {
    if ( _DK_lbDisplay.MouseWindowX + _DK_lbDisplay.MouseWindowWidth <= *x )
    {
      *x = _DK_lbDisplay.MouseWindowX + _DK_lbDisplay.MouseWindowWidth - 1;
      result = true;
    }
  } else
  {
    *x = _DK_lbDisplay.MouseWindowX;
    result = true;
  }
  if ( *y >= _DK_lbDisplay.MouseWindowY )
  {
    if ( _DK_lbDisplay.MouseWindowY + _DK_lbDisplay.MouseWindowHeight <= *y )
    {
      *y = _DK_lbDisplay.MouseWindowY + _DK_lbDisplay.MouseWindowHeight - 1;
      result = true;
    }
  } else
  {
    *y = _DK_lbDisplay.MouseWindowY;
    result = true;
  }
  return result;
}

//Returns if the current mouse position is inside of given rectangle
char __fastcall mouse_in_rect(short x1, short x2, short y1, short y2)
{
  return (x1<=_DK_lbDisplay.MMouseX) && (x2>_DK_lbDisplay.MMouseX) &&
         (y1<=_DK_lbDisplay.MMouseY) && (y2>_DK_lbDisplay.MMouseY);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
