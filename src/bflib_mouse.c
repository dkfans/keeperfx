/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mouse.c
 *     Mouse related routines.
 * @par Purpose:
 *     Mouse and mouse cursor support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 18 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_mouse.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
//#include <SDL/SDL.h>

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*
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
*/
/******************************************************************************/
DLLIMPORT int _DK_LbMouseChangeSpriteAndHotspot(struct TbSprite *spr, int a, int b);
DLLIMPORT int _DK_LbMouseSetup(struct TbSprite *MouseSprite);
DLLIMPORT int _DK_LbMouseSetPointerHotspot(int x, int y);
/******************************************************************************/
int LbMouseChangeSpriteAndHotspot(struct TbSprite *spr, int a, int b)
{
  return _DK_LbMouseChangeSpriteAndHotspot(spr, a, b);
}

int LbMouseSetup(struct TbSprite *MouseSprite)
{
  return _DK_LbMouseSetup(MouseSprite);
}

int LbMouseSetPointerHotspot(int x, int y)
{
  return _DK_LbMouseSetPointerHotspot(x, y);
}

/*
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

int __fastcall LbMouseSetup(struct TbSprite *MouseSprite, char x_ratio, char y_ratio)
{
  if ( lbMouseInstalled )
    return -1;
  if ( !mouse_initialised )
  {
    if (segread_(), inregs = v5, int386_(), v11 != 65535)
    {
      return -1;
    }
  }
  lbMouseInstalled = true;
  SDL_ShowCursor(0);
  minfo.XMoveRatio = 1;
  minfo.YMoveRatio = 1;
  minfo.XSpriteOffset = 0;
  minfo.YSpriteOffset = 0;
  memset(minfo.Sprite, 254, 4096);
  lbDisplay.MouseSprite = NULL;
  redraw_active = 0;
  LbMemorySet(&mbuffer, 0, sizeof(struct mouse_buffer));
  if (LbMouseSetWindow(0,0,lbDisplay.GraphicsScreenWidth,lbDisplay.GraphicsScreenHeight)!=1)
  {
      lbMouseInstalled = 0;
      return -1;
  }
  if (LbMouseChangeMoveRatio(x_ratio,y_ratio)!=1)
  {
      lbMouseInstalled = 0;
      return -1;
  }
  if ( LbMouseSetPosition(lbDisplay.GraphicsScreenWidth>>1, lbDisplay.GraphicsScreenHeight>>1) != 1 )
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
      sregs = __CS__;
      int386x_();
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
  int386_();
  mouse_initialised = 0;
  lbDisplay.MouseSprite = NULL;
  lbMouseInstalled = false;
  SDL_ShowCursor(1);
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
  lbDisplay.MouseSprite = NULL;
  lbMouseInstalled = false;
  redraw_active = 0;
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

//Removes mouse from the screen.
//Requires screen to be locked before calling.
int __fastcall LbMouseRemove(void)
{
  if ( (mbuffer.Valid) && (lbDisplay.WScreen!=NULL) )
  {
    unsigned char *scr_ptr;
    unsigned char *bscr_ptr;
    unsigned char *spr_ptr;
    int c1,c2;
    spr_ptr = mbuffer.Buffer;
    bscr_ptr = &lbDisplay.WScreen[mbuffer.Offset];
    for (c1=0;c1<mbuffer.Height;c1++)
    {
      scr_ptr = bscr_ptr;
      for (c2=0;c2<mbuffer.Width;c2++)
      {
        *scr_ptr=*spr_ptr;
        spr_ptr++;
        scr_ptr++;
      }
      bscr_ptr += lbDisplay.GraphicsScreenWidth;
    }
  }
  if ( mouse_pos_change_saved )
  {
    lbDisplay.MMouseX += mouse_dx;
    lbDisplay.MMouseY += mouse_dy;
    adjust_point(&lbDisplay.MMouseX, &lbDisplay.MMouseY);
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
  lbDisplay.MMouseX = x;
  lbDisplay.MouseX = lbDisplay.MMouseX;
  lbDisplay.MMouseY = y;
  lbDisplay.MouseY = lbDisplay.MMouseY;
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
  if ( lbDisplay.MouseSprite == pointer )
      return 1;
  //Size limitation
  if ( (pointer!=NULL) && ((pointer->SWidth>64)||(pointer->SHeight>64)) )
      return -1;
  redraw_active = 1;
  if ( !lbScreenDirectAccessActive )
    screen_remove(1u);
  lbDisplay.MouseSprite = pointer;
  memset(minfo.Sprite, 254, 4096);
  if ( pointer != NULL )
  {
      unsigned char *wscr_backup;
      int gwx,gwy;
      int gww,gwh,gsw;
      unsigned short dflags;
      wscr_backup = lbDisplay.WScreen;
      lbDisplay.WScreen = minfo.Sprite;
      gwx = lbDisplay.GraphicsWindowX;
      gwy = lbDisplay.GraphicsWindowY;
      gww = lbDisplay.GraphicsWindowWidth;
      gwh = lbDisplay.GraphicsWindowHeight;
      gsw = lbDisplay.GraphicsScreenWidth;
      dflags = lbDisplay.DrawFlags;
      lbDisplay.GraphicsScreenWidth = pointer->SWidth;
      lbDisplay.DrawFlags = 0;
      LbScreenSetGraphicsWindow(0, 0, pointer->SWidth, pointer->SHeight);
      LbSpriteDraw(0, 0, pointer);
      lbDisplay.WScreen = wscr_backup;
      lbDisplay.GraphicsScreenWidth = gsw;
      lbDisplay.DrawFlags = dflags;
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
      lbDisplay.MMouseX+=mouse_dx;
      lbDisplay.MMouseY+=mouse_dy;
      bool remove=adjust_point(&lbDisplay.MMouseX,&lbDisplay.MMouseY);
      int vesa_page=lbVesaPage;
      screen_remove(remove);
      screen_place();
      if (lbDisplay.VesaIsSetUp)
        LbVesaSetPage(vesa_page);
      mouse_pos_change_saved=0;
    }
}

//DOS interrupt handler for the mouse driver, not used with SDL
void __fastcall MouseHandler256(int mx, int my, int event)
{
  if (!lbMouseInstalled)
    return;
  if (event != 0x01)
  {
    if (event & 0x02)
    {
      lbDisplay.MLeftButton=1;
      if (lbDisplay.LeftButton==0)
      {
        lbDisplay.LeftButton=1;
        lbDisplay.MouseX=lbDisplay.MMouseX;
        lbDisplay.MouseY=lbDisplay.MMouseY;
        lbDisplay.RLeftButton=0;
      }
    }
    if (event & 0x04)
    {
      lbDisplay.MLeftButton=0;
      if (lbDisplay.RLeftButton==0)
      {
        lbDisplay.RLeftButton=1;
        lbDisplay.RMouseX=lbDisplay.MMouseX;
        lbDisplay.RMouseY=lbDisplay.MMouseY;
      }
    }
    if (event & 0x08)
    {
      lbDisplay.MRightButton=1;
      if (lbDisplay.RightButton==0)
      {
        lbDisplay.RightButton=1;
        lbDisplay.MouseX=lbDisplay.MMouseX;
        lbDisplay.MouseY=lbDisplay.MMouseY;
        lbDisplay.RRightButton=0;
      }
    }
    if (event & 0x10)
    {
      lbDisplay.MRightButton=0;
      if (lbDisplay.RRightButton==0)
      {
        lbDisplay.RRightButton=1;
        lbDisplay.RMouseX=lbDisplay.MMouseX;
        lbDisplay.RMouseY=lbDisplay.MMouseY;
      }
    }

    if (event & 0x20)
    {
      lbDisplay.MMiddleButton=1;
      if (lbDisplay.MiddleButton==0)
      {
        lbDisplay.MiddleButton=1;
        lbDisplay.MouseX=lbDisplay.MMouseX;
        lbDisplay.MouseY=lbDisplay.MMouseY;
        lbDisplay.RMiddleButton=0;
      }
    }
    if (event & 0x40)
    {
      lbDisplay.MMiddleButton=0;
      if (lbDisplay.RMiddleButton==0)
      {
        lbDisplay.RMiddleButton=1;
        lbDisplay.RMouseX=lbDisplay.MMouseX;
        lbDisplay.RMouseY=lbDisplay.MMouseY;
      }
    }
  }
  if (event & 0x01)
  {
    MouseHandlerMove(mx,my);
  }
}

void __fastcall LbProcessMouseMove(SDL_MouseMotionEvent *motion)
{
  MouseHandlerMove(motion->x,motion->y);
}

void __fastcall LbProcessMouseClick(SDL_MouseButtonEvent *button)
{
  switch (button->state)
  {
  case SDL_PRESSED:
    switch (button->button)
    {
    case SDL_BUTTON_LEFT:
        lbDisplay.MLeftButton=1;
        if (lbDisplay.LeftButton==0)
        {
          lbDisplay.LeftButton=1;
          lbDisplay.MouseX=lbDisplay.MMouseX;
          lbDisplay.MouseY=lbDisplay.MMouseY;
          lbDisplay.RLeftButton=0;
        }
       break;
    case SDL_BUTTON_MIDDLE:
        lbDisplay.MMiddleButton=1;
        if (lbDisplay.MiddleButton==0)
        {
          lbDisplay.MiddleButton=1;
          lbDisplay.MouseX=lbDisplay.MMouseX;
          lbDisplay.MouseY=lbDisplay.MMouseY;
          lbDisplay.RMiddleButton=0;
        }
       break;
    case SDL_BUTTON_RIGHT:
        lbDisplay.MRightButton=1;
        if (lbDisplay.RightButton==0)
        {
          lbDisplay.RightButton=1;
          lbDisplay.MouseX=lbDisplay.MMouseX;
          lbDisplay.MouseY=lbDisplay.MMouseY;
          lbDisplay.RRightButton=0;
        }
       break;
    };break;
  case SDL_RELEASED:
    switch (button->button)
    {
    case SDL_BUTTON_LEFT:
        lbDisplay.MLeftButton=0;
        if (lbDisplay.RLeftButton==0)
        {
          lbDisplay.RLeftButton=1;
          lbDisplay.RMouseX=lbDisplay.MMouseX;
          lbDisplay.RMouseY=lbDisplay.MMouseY;
        }
       break;
    case SDL_BUTTON_MIDDLE:
        lbDisplay.MMiddleButton=0;
        if (lbDisplay.RMiddleButton==0)
        {
          lbDisplay.RMiddleButton=1;
          lbDisplay.RMouseX=lbDisplay.MMouseX;
          lbDisplay.RMouseY=lbDisplay.MMouseY;
        }
       break;
    case SDL_BUTTON_RIGHT:
        lbDisplay.MRightButton=0;
        if (lbDisplay.RRightButton==0)
        {
          lbDisplay.RRightButton=1;
          lbDisplay.RMouseX=lbDisplay.MMouseX;
          lbDisplay.RMouseY=lbDisplay.MMouseY;
        }
       break;
    };break;
  }
  //MouseHandlerMove(button->x,button->y);
}


int __fastcall screen_place(void)
{
//todo
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
      unsigned char *spr_ptr;
      unsigned char *bscr_ptr;
      int c1;
      unsigned char *scr_ptr;
      spr_ptr = mbuffer.Buffer;
//TODO: make it write on screen buffer, not the physical one
      if ( lbDisplay.VesaIsSetUp )
      {
          unsigned int vesa_page = mbuffer.Offset >> 16;
          LbVesaSetPage(vesa_page);
          bscr_ptr = (mbuffer.Offset&0xffff) + lbDisplay.PhysicalScreen;
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
            bscr_ptr += lbDisplay.PhysicalScreenWidth;
          }
      } else
      {
          bscr_ptr = lbDisplay.PhysicalScreen + mbuffer.Offset;
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
            bscr_ptr += lbDisplay.PhysicalScreenWidth;
          }
      }
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
*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
