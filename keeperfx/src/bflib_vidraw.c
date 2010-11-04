/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_vidraw.c
 *     Graphics canvas drawing library.
 * @par Purpose:
 *    Screen drawing routines; draws half-transparent boxes and other elements.
 * @par Comment:
 *     Medium level library, draws on screen buffer used in bflib_video.
 *     Used for drawing screen components.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 10 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_vidraw.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"

#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_mouse.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT int _DK_LbSpriteDraw(long x, long y, const struct TbSprite *spr);
DLLIMPORT int _DK_LbSpriteDrawRemap(long x, long y, const struct TbSprite *spr,unsigned char *map);
DLLIMPORT int _DK_LbSpriteDrawOneColour(long x, long y, const struct TbSprite *spr, const TbPixel colour);
/******************************************************************************/
/*
bool sprscale_enlarge;
long  sprscale_wbuf[512];
long  sprscale_hbuf[512];
struct PurpleDrawItem p_list[NUM_DRAWITEMS];
unsigned short purple_draw_index;
struct PurpleDrawItem *purple_draw_list=p_list;
TbSprite *lbFontPtr;
unsigned short text_window_x1, text_window_y1;
unsigned short text_window_x2, text_window_y2;
char my_line_spacing;
TbPixel vec_colour=0x70;
unsigned char vec_tmap[0x10000];
struct StartScreenPoint hots[50];
unsigned char *poly_screen=NULL;
unsigned char *vec_screen=NULL;
unsigned char *vec_map=NULL;
unsigned char *vec_pal=NULL;
unsigned long vec_screen_width=0;
unsigned long vec_window_width=0;
unsigned long vec_window_height=0;
unsigned char vec_mode=0;
unsigned char *dither_map=NULL;
unsigned char *dither_end=NULL;
struct StartScreenPoint proj_origin = { (640>>1)-1, ((480+60)>>1)-1 };
struct StartScreenPoint *hotspot_buffer=hots;
unsigned char *lbSpriteReMapPtr;
*/
/******************************************************************************/
/**
 * Prints horizontal or vertical line on current graphics window.
 * Does no screen locking - screen must be lock before and unlocked
 * after a call to this function.
 */
void LbDrawHVLine(long xpos1, long ypos1, long xpos2, long ypos2, TbPixel colour)
{
  long width_max = lbDisplay.GraphicsWindowWidth - 1;
  long height_max = lbDisplay.GraphicsWindowHeight - 1;
  if ( xpos1 > xpos2 )
  { //Switching & clipping x coordinates
    if (xpos1 < 0) return;
    if (xpos2 > width_max) return;
    long nxpos1=xpos2;
    long nxpos2=xpos1;
    if ( xpos2 < 0 )
      nxpos1 = 0;
    if ( xpos1 > width_max )
      nxpos2 = lbDisplay.GraphicsWindowWidth - 1;
    xpos1 = nxpos1;
    xpos2 = nxpos2;
  } else
  { //Clipping x coordinates
    if (xpos2 < 0) return;
    if (xpos1 > width_max) return;
    if ( xpos1 < 0 )
      xpos1 = 0;
    if ( xpos2 > width_max )
      xpos2 = lbDisplay.GraphicsWindowWidth - 1;
  }
  if ( ypos1 > ypos2 )
  { //Switching & clipping y coordinates
    if (ypos1 < 0) return;
    if (ypos2 > height_max) return;
    long nxpos1=xpos2;
    long nxpos2=xpos1;
    if ( ypos2 < 0 )
      nxpos1 = 0;
    if ( ypos1 > height_max )
      nxpos2 = lbDisplay.GraphicsWindowHeight - 1;
    ypos1 = nxpos1;
    ypos2 = nxpos2;
  } else
  { //Clipping y coordinates
    if (ypos2 < 0) return;
    if (ypos1 > height_max) return;
    if (ypos1 < 0)
      ypos1 = 0;
    if ( ypos2 > height_max )
      ypos2 = lbDisplay.GraphicsWindowHeight - 1;
  }
  //And now to drawing
  unsigned char *screen_ptr = lbDisplay.GraphicsWindowPtr + xpos1 +
          lbDisplay.GraphicsScreenWidth * ypos1;
  if ( xpos2 == xpos1 )
  {//Vertical line
    long idx = ypos2 - ypos1 + 1;
    if ( lbDisplay.DrawFlags & 4 )
    {
      unsigned short glass_idx = (unsigned char)colour << 8;
      do {
        glass_idx&=0xff00;
        glass_idx |= *screen_ptr;
        *screen_ptr = lbDisplay.GlassMap[glass_idx];
        screen_ptr += lbDisplay.GraphicsScreenWidth;
        idx--;
      } while ( idx>0 );
    } else
    {
      if ( lbDisplay.DrawFlags & 8 )
      {
        unsigned short glass_idx = (unsigned char)colour;
        do
        {
          glass_idx&=0x00ff;
          glass_idx |= ((*screen_ptr)<<8);
          *screen_ptr = lbDisplay.GlassMap[glass_idx];
          screen_ptr += lbDisplay.GraphicsScreenWidth;
          idx--;
        }
        while ( idx>0 );
      } else
      {
        unsigned char col_idx = colour;
        do
        {
          *screen_ptr = col_idx;
          screen_ptr += lbDisplay.GraphicsScreenWidth;
          idx--;
        }
        while ( idx>0 );
      }
    }
  } else
  {//Horizontal line
    long idx = xpos2 - xpos1 + 1;
    if ( lbDisplay.DrawFlags & 4 )
    {
      unsigned short glass_idx = (unsigned char)colour << 8;
      do
      {
        glass_idx&=0xff00;
        glass_idx |= *screen_ptr;
        *screen_ptr = lbDisplay.GlassMap[glass_idx];
        screen_ptr++;
        idx--;
      }
      while ( idx>0 );
    }
    else
    {
      if ( lbDisplay.DrawFlags & 8 )
      {
        unsigned short glass_idx = (unsigned char)colour;
        do
        {
          glass_idx&=0x00ff;
          glass_idx |= (((unsigned short)*screen_ptr)<<8);
          *screen_ptr = lbDisplay.GlassMap[glass_idx];
          screen_ptr++;
          idx--;
        }
        while ( idx>0 );
      }
      else
      {
        unsigned char col_idx = colour;
        while ( idx>0 )
        {
          *screen_ptr = col_idx;
          screen_ptr++;
          idx--;
        }
      }
    }
  }
}

/**
 * Draws a filled box on current graphic window.
 * Performs clipping if needed to stay inside the window.
 * Does no screen locking.
 */
void LbDrawBoxClip(long x, long y, unsigned long width, unsigned long height, TbPixel colour)
{
  long ypos = y;
  //Checking and clipping coordinates
  if ( y >= lbDisplay.GraphicsWindowHeight )
      return;
  if ( y < 0 )
  {
      height += y;
      ypos = 0;
  }
  if ( (long)(height + ypos) > lbDisplay.GraphicsWindowHeight )
      height -= height + ypos - lbDisplay.GraphicsWindowHeight;
  if ( (long)height <= 0 )
      return;

  ypos = lbDisplay.GraphicsScreenWidth * (lbDisplay.GraphicsWindowY + ypos);
  long xpos = x;
  if ( x >= lbDisplay.GraphicsWindowWidth )
      return;
  if ( x < 0 )
  {
      width += x;
      xpos = 0;
  }
  if ( (long)(width + xpos) > lbDisplay.GraphicsWindowWidth )
      width -= width + xpos - lbDisplay.GraphicsWindowWidth;
  if ( (long)width <= 0 )
      return;
  //And now let's start drawing
  unsigned char *screen_ptr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX] + xpos + ypos;
  unsigned long idxh = height;
  //Space between lines in video buffer
  unsigned long screen_delta = lbDisplay.GraphicsScreenWidth - width;
  if ( lbDisplay.DrawFlags & 0x0004 )
  {
      unsigned short glass_idx = (unsigned char)colour << 8;
      do {
          unsigned long idxw = width;
          do {
                glass_idx&=0xff00;
                glass_idx |= *screen_ptr;
                *screen_ptr = lbDisplay.GlassMap[glass_idx];
                screen_ptr++;
                idxw--;
          } while ( idxw>0 );
          screen_ptr += screen_delta;
          idxh--;
      } while ( idxh>0 );
  } else
  if ( lbDisplay.DrawFlags & 0x0008 )
  {
      unsigned short glass_idx = (unsigned char)colour;
      do {
            unsigned long idxw = width;
            do {
              glass_idx&=0x00ff;
              glass_idx |= (((unsigned short)*screen_ptr)<<8);
              *screen_ptr = lbDisplay.GlassMap[glass_idx];
              screen_ptr++;
              idxw--;
            } while ( idxw>0 );
            screen_ptr += screen_delta;
            idxh--;
      } while ( idxh>0 );
  } else
  {
      unsigned char col_idx = colour;
      do {
            unsigned long idxw = width;
            do {
              *screen_ptr = col_idx;
              screen_ptr++;
              idxw--;
            } while ( idxw>0 );
            screen_ptr += screen_delta;
            idxh--;
      } while ( idxh>0 );
  }
}

/**
 * Draws a rectangular box on current graphics window.
 * Does no screen locking.
 * @return If wrong dimensions returns -1. On success returns 1.
*/
int LbDrawBox(long x, long y, unsigned long width, unsigned long height, TbPixel colour)
{
  if (lbDisplay.DrawFlags & 0x0010)
  {
    if ( width < 1 || height < 1 )
      return -1;
    LbDrawHVLine(x, y, width + x - 1, y, colour);
    LbDrawHVLine(x, height + y - 1, width + x - 1, height + y - 1, colour);
    if ( height > 2 )
    {
      LbDrawHVLine(x, y + 1, x, height + y - 2, colour);
      LbDrawHVLine(width + x - 1, y + 1, width + x - 1, height + y - 2, colour);
    }
  }
  else
  {
    LbDrawBoxClip(x, y, width, height, colour);
  }
  return 1;
}

/** Internal function used to draw part of sprite line.
 *
 * @param buf_scr
 * @param buf_inp
 * @param buf_len
 * @param mirror
 */
inline void LbDrawBufferLine(unsigned char **buf_scr,unsigned char **buf_inp,
        const unsigned int buf_len, const bool mirror)
{
  unsigned int i;
  if ( mirror )
  {
    if ((lbDisplay.DrawFlags & 0x0004) != 0)
    {
        SYNCDBG(19,"Drawing M4, %d points",buf_len);
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[((**buf_inp)<<8) + **buf_scr];
            (*buf_inp)++;
            (*buf_scr)--;
        }
    } else
    {
        SYNCDBG(19,"Drawing MX, %d points",buf_len);
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[((**buf_scr)<<8) + **buf_inp];
            (*buf_inp)++;
            (*buf_scr)--;
        }
    }
  } else
  {
    if ( lbDisplay.DrawFlags & 0x0004 )
    {
        SYNCDBG(19,"Drawing S4, %d points",buf_len);
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[((**buf_inp)<<8) + **buf_scr];
            (*buf_inp)++;
            (*buf_scr)++;
        }
    } else
    {
        SYNCDBG(19,"Drawing SX, %d points",buf_len);
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[((**buf_scr)<<8) + **buf_inp];
            (*buf_inp)++;
            (*buf_scr)++;
        }
    }
  }
}

/** Internal function used to draw part of sprite line.
 *
 * @param buf_scr
 * @param buf_inp
 * @param buf_len
 * @param mirror
 */
inline void LbDrawBufferLineSolid(unsigned char **buf_scr,unsigned char **buf_inp,
        const unsigned int buf_len, const bool mirror)
{
    unsigned int i;
    for (i=0;i<buf_len;i++)
    {
        **buf_scr = **buf_inp;
        (*buf_inp)++;
        (*buf_scr)--;
    }
}

inline int LbSpriteDrawTranspr(const char *sp,short sprWd,short sprHt,unsigned char *r,int nextRowDelta,short left,bool mirror)
{
    int x1;
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    x1 = sprWd;
    htIndex = sprHt;
    SYNCDBG(19,"Drawing %ld lines of %hd points",htIndex,sprWd);
    while (1)
    {
        SYNCDBG(19,"Line %ld",htIndex);
        if ( left != 0 )
        {
              unsigned char drawOut=0;
              short lpos = left;
              do {
                char chr = *sp;
                if ( chr == 0 )
                {
                  x1 = *sp;
                  break;
                } else
                if ( chr > lpos )
                {
                  unsigned char *nchr = (unsigned char *)&sp[lpos+1];
                  drawOut = *sp - lpos;
                  if ( drawOut > sprWd )
                    drawOut = sprWd;
                  SYNCDBG(19,"Left line, screen buf pos=%ld, len=%d",r-lbDisplay.WScreen,(int)drawOut);
                  LbDrawBufferLine(&r,&nchr,drawOut,mirror);
                  sp += chr + 1;
                  x1 -= drawOut;
                  break;
                } else
                if ( chr < -lpos )
                {
                  drawOut = -(*sp) - lpos;
                  if ( drawOut > sprWd )
                    drawOut = sprWd;
                  if ( mirror )
                     r -= drawOut;
                  else
                     r += drawOut;
                  x1 -= drawOut;
                  sp++;
                  break;
                } else
                if ( chr > 0 )
                {
                    lpos -= *sp;
                    sp += *sp + 1;
                } else
                // if ( chr < 0 )
                {
                  lpos += *sp;
                  sp++;
                }
              } while (lpos > 0);
        }

        while (x1 > 0)
        {
              char schr = *sp;
              if ( schr == 0 )
                break;
              if ( schr < 0 )
              {
                x1 += schr;
                if ( mirror )
                   r += *sp;
                else
                   r -= *sp;
                sp++;
              } else
              {
                if ( schr >= x1 )
                  schr = x1;
                unsigned char *nchr = (unsigned char *)(sp+1);
                SYNCDBG(19,"X1 line, screen buf pos=%ld, len=%d",r-lbDisplay.WScreen,(int)schr);
                LbDrawBufferLine(&r,&nchr,schr,mirror);
                x1 -= schr;
                sp += schr+1;
              }
        }//end while

        //Drawing loop end condition
        htIndex--;
        if ( htIndex<=0 )
            break;

        if ( x1 > 0 )
        {
            sp++;
        } else
        {
            char schr;
            do {
              schr=*sp;
              while ( schr>0 )
              {
                  sp+=schr+1;
                  schr=*sp;
              }
              sp++;
            } while ( schr != 0 );
        }
        r = nextRow;
        nextRow += nextRowDelta;
        x1 = sprWd;
    } //end while
    return 1;
}

inline int LbSpriteDrawSolid(const char *sp,short sprWd,short sprHt,unsigned char *r,int nextRowDelta,short left,bool mirror)
{
    int x1;
    unsigned char *nextRow;
    long htIndex;
    nextRow = r;
    htIndex = sprHt;
    while (1)
    {
        r = nextRow;
        nextRow += nextRowDelta;
        x1 = sprWd;
        if ( left != 0 )
        {
            unsigned char drawOut=0;
            short lpos = left;
            do {
                char chr=*sp;

                if ( chr == 0 )
                {
                  x1 = *sp;
                  break;
                } else
                if ( chr > lpos )
                {
                    unsigned char *nchr = (unsigned char *)&sp[lpos+1];
                    drawOut = *sp - lpos;
                    if ( drawOut > sprWd )
                      drawOut = sprWd;
                    LbDrawBufferLineSolid(&r,&nchr,drawOut,mirror);
                    sp += *sp + 1;
                    x1 -= drawOut;
                    break;
                } else
                if ( chr < -lpos )
                {
                  drawOut = -*sp -lpos;
                  if ( drawOut > sprWd )
                    drawOut = sprWd;
                  r -= drawOut;
                  x1 -= drawOut;
                  sp++;
                  break;
                } else
                if ( chr > 0 )
                {
                  lpos -= *sp;
                  sp += *sp + 1;
                } else
                //if ( chr < 0 )
                {
                  lpos += *sp;
                  sp++;
                }
            } while (lpos > 0);

        }
        while ( x1 > 0 )
        {
          char schr;
          schr = *sp;
          if ( schr == 0 )
          {
              break;
          }
          if ( schr < 0 )
          {
              x1 += schr;
              r += *sp;
              sp++;
          } else
          {
            if ( schr >= x1 )
              schr = x1;
            unsigned char *nchr = (unsigned char *)&sp[1];
            LbDrawBufferLineSolid(&r,&nchr,schr,mirror);
            x1 -= schr;
            sp += schr + 1;
          }
        }

        htIndex--;
        if ( htIndex==0 )
            break;

        if ( x1 <= 0 )
        {
            char schr;
            do {
              schr=*sp;
              while ( schr>0 )
              {
                  sp+=schr+1;
                  schr=*sp;
              }
              sp++;
            } while ( schr != 0 );
        } else
        {
            sp++;
        }
    } //end while
    return 1;
}

inline int LbSpriteDrawFastCpy(const char *sp,short sprWd,short sprHt,unsigned char *r,int nextRowDelta,short left,bool mirror)
{
    short x1;
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        x1 = sprWd;
        char schr;
        unsigned char drawOut;
        // Cut the left side of the sprite, if needed
        if (left != 0)
        {
            short lpos = left;
            while (lpos > 0)
            {
                schr = *sp;
                // Value > 0 means count of filled characters, < 0 means skipped characters
                // Equal to 0 means EOL
                if (schr == 0)
                {
                  x1 = 0;
                  break;
                }
                if (schr < 0)
                {
                    if (-schr > lpos)
                    {
                        drawOut = -schr - lpos;
                        if (drawOut > x1)
                          drawOut = x1;
                        sp++;
                        r += drawOut;
                        x1 -= drawOut;
                        lpos = 0;
                    } else
                    {
                        lpos += schr;
                        sp++;
                    }
                } else
                //if (schr > 0)
                {
                    if (schr > lpos)
                    // If we have more characters than we want to skip
                    {
                        // Draw the part that exceeds value of 'left'
                        drawOut = schr - lpos;
                        if (drawOut > x1)
                          drawOut = x1;
                        memcpy( r, sp+(lpos+1), drawOut);
                        // Update positions and break the skipping loop
                        sp += (*sp) + 1;
                        r += drawOut;
                        x1 -= drawOut;
                        lpos = 0;
                    } else
                    // If we have less than we want to skip
                    {
                        lpos -= schr;
                        sp += (*sp) + 1;
                    }
                }
            }
        }
        // Draw the visible part of a sprite
        while (x1 > 0)
        {
            schr = *sp;
            if (schr == 0)
            { // EOL, breaking line loop
                break;
            }
            if (schr < 0)
            { // Skipping some pixels
                x1 += schr;
                r -= *sp;
                sp++;
            } else
            //if ( schr > 0 )
            { // Drawing some pixels
                drawOut = schr;
                if (drawOut >= x1)
                    drawOut = x1;
                memcpy(r, sp+1, drawOut);
                x1 -= schr;
                r += schr;
                sp += (*sp) + 1;
            }
        } //end while
        htIndex--;
        if (htIndex==0)
          return 1;
        if (x1 <= 0)
        {
          do {
            schr=*sp;
            while (schr > 0)
            {
              sp += schr+1;
              schr = *sp;
            }
            sp++;
          } while (schr);
        } else
        {
          sp++;
        }

        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return 1;
}

int LbSpriteDraw(long x, long y, const struct TbSprite *spr)
{
    //TODO SPRITES Fix, then enable the rewritten code. Works incorrectly if image starts before left corner of the screen.
    SYNCDBG(19,"At (%ld,%ld)",x,y);
    //return _DK_LbSpriteDraw(x, y, spr);
    if (spr == NULL)
    {
        SYNCDBG(19,"NULL sprite");
        return 1;
    }
    if ((spr->SWidth < 1) || (spr->SHeight < 1))
    {
        SYNCDBG(19,"Zero size sprite (%d,%d)",spr->SWidth,spr->SHeight);
        return 1;
    }
    if ((lbDisplay.GraphicsWindowWidth == 0) || (lbDisplay.GraphicsWindowHeight == 0))
    {
        SYNCDBG(19,"Invalid graphics window dimensions");
        return 1;
    }
    x += lbDisplay.GraphicsWindowX;
    y += lbDisplay.GraphicsWindowY;
    short left,right,top,btm;
    short sprWd = spr->SWidth;
    short sprHt = spr->SHeight;
    //Coordinates range checking
    int delta;
    delta = lbDisplay.GraphicsWindowX - x;
    if ( delta <= 0 )
    {
      left = 0;
    } else
    {
      if (sprWd <= delta)
        return 1;
      left = delta;
    }
    delta = x + sprWd - (lbDisplay.GraphicsWindowWidth+lbDisplay.GraphicsWindowX);
    if ( delta <= 0 )
    {
      right = sprWd;
    } else
    {
      if ( sprWd <= delta )
        return 1;
      right = sprWd - delta;
    }
    delta = lbDisplay.GraphicsWindowY - y;
    if ( delta <= 0 )
    {
      top = 0;
    } else
    {
      if ( sprHt <= delta )
        return 1;
      top = delta;
    }
    delta = y + sprHt - (lbDisplay.GraphicsWindowHeight + lbDisplay.GraphicsWindowY);
    if ( y + sprHt - (lbDisplay.GraphicsWindowHeight + lbDisplay.GraphicsWindowY) <= 0 )
    {
      btm = sprHt;
    } else
    {
      if ( sprHt <= delta )
        return 1;
      btm = sprHt - delta;
    }
    unsigned char *r;
    int nextRowDelta;
    if ((lbDisplay.DrawFlags & 0x0002) != 0)
    {
        r = &lbDisplay.WScreen[x + (y+btm-1)*lbDisplay.GraphicsScreenWidth + left];
        nextRowDelta = -lbDisplay.GraphicsScreenWidth;
        short tmp_btm = btm;
        btm = sprHt - top;
        top = sprHt - tmp_btm;
    } else
    {
        r = &lbDisplay.WScreen[x + (y+top)*lbDisplay.GraphicsScreenWidth + left];
        nextRowDelta = lbDisplay.GraphicsScreenWidth;
    }
    sprHt = btm - top;
    sprWd = right - left;
    const char *sp = (const char *)spr->Data;
    SYNCDBG(19,"Sprite coords X=%d...%d Y=%d...%d data=%08x",left,right,top,btm,sp);
    long htIndex;
    if ( top )
    {
      htIndex = top;
      while ( 1 )
      {
        char chr = *sp;
        while ( chr>0 )
        {
          sp += chr + 1;
          chr = *sp;
        }
        sp++;
        if ( chr == 0 )
        {
          htIndex--;
          if (htIndex<=0) break;
        }
      }
    }
    SYNCDBG(19,"Drawing sprite of size (%ld,%ld)",sprHt,sprWd);
    bool mirror;
    if ( lbDisplay.DrawFlags & (0x0004|0x0008|0x0001) )
    {
      if ((lbDisplay.DrawFlags & 0x0001) != 0)
      {
         r += sprWd - 1;
        mirror = true;
        short tmpwidth = spr->SWidth;
        short tmpright = right;
        right = tmpwidth - left;
        left = tmpwidth - tmpright;
      }
      else
      {
        mirror = false;
      }
      if ((lbDisplay.DrawFlags & (0x0004|0x0008)) != 0)
        return LbSpriteDrawTranspr(sp,sprWd,sprHt,r,nextRowDelta,left,mirror);
      else
        return LbSpriteDrawSolid(sp,sprWd,sprHt,r,nextRowDelta,left,mirror);
    }
    return LbSpriteDrawFastCpy(sp,sprWd,sprHt,r,nextRowDelta,left,mirror);
}

int LbSpriteDrawRemap(long x, long y, const struct TbSprite *spr,unsigned char *map)
{
  return _DK_LbSpriteDrawRemap(x, y, spr,map);
}

int LbSpriteDrawOneColour(long x, long y, const struct TbSprite *spr, const TbPixel colour)
{
  return _DK_LbSpriteDrawOneColour(x, y, spr, colour);
}

void setup_vecs(unsigned char *screenbuf, unsigned char *nvec_map,
        unsigned int line_len, unsigned int width, unsigned int height)
{
  if ( line_len > 0 )
    vec_screen_width = line_len;
  if (screenbuf != NULL)
  {
    vec_screen = screenbuf;
    poly_screen = screenbuf - vec_screen_width;
  }
  if (nvec_map != NULL)
  {
    vec_map = nvec_map;
    dither_map = nvec_map;
    dither_end = nvec_map + 16;
  }
  if (height > 0)
    vec_window_height = height;
  if (width > 0)
    vec_window_width = width;
}

/*
unsigned short __fastcall is_it_clockwise(struct EnginePoint *point1,
      struct EnginePoint *point2, struct EnginePoint *point3)
{
  long vx;
  long wx;
  long vy;
  long wy;
  vx = point2->X - point1->X;
  wx = point3->X - point2->X;
  vy = point2->Y - point1->Y;
  wy = point3->Y - point2->Y;
  return (wy * vx - wx * vy) > 0;
}

void LbSpriteSetScalingData(long x, long y, long swidth, long sheight, long dwidth, long dheight)
{
  sprscale_enlarge = true;
  if ( (dwidth<=swidth) && (dheight<=sheight) )
      sprscale_enlarge = false;
  long gwidth = lbDisplay.GraphicsWindowWidth;
  long gheight = lbDisplay.GraphicsWindowHeight;
  long *pwidth;
  long cwidth;
  if ( (x < 0) || ((dwidth+x) >= gwidth) )
  {
    pwidth = sprscale_wbuf;
    long factor = (dwidth<<16)/swidth;
    long tmp = (factor >> 1) + (x << 16);
    cwidth = tmp >> 16;
    if ( cwidth < 0 )
      cwidth = 0;
    if ( cwidth >= gwidth )
      cwidth = gwidth;
    long w = swidth;
    do {
      pwidth[0] = cwidth;
      tmp += factor;
      long cwidth2 = tmp>>16;
      if ( cwidth2 < 0 )
        cwidth2 = 0;
      if ( cwidth2 >= gwidth )
        cwidth2 = gwidth;
      long wdiff = cwidth2 - cwidth;
      pwidth[1] = wdiff;
      cwidth += wdiff;
      pwidth += 2;
      w--;
    } while (w>0);
  } else
  {
    pwidth = sprscale_wbuf;
    long factor = (dwidth<<16)/swidth;
    long tmp = (factor >> 1) + (x << 16);
    cwidth = tmp >> 16;
    long w=swidth;
    while ( 1 )
    {
      int i=0;
      for (i=0;i<16;i+=2)
      {
        pwidth[i] = cwidth;
        tmp += factor;
        pwidth[i+1] = (tmp>>16) - cwidth;
        cwidth = (tmp>>16);
        w--;
        if (w<=0)
          break;
      }
      if (w<=0)
        break;
      pwidth += 16;
    }
  }
  long *pheight;
  long cheight;
  //Note: the condition in "if" is suspicious
  if ( ((long)pwidth<0) || ((long)pwidth + cwidth) >= gheight )
  {
    long factor = (dheight<<16)/sheight;
    pheight = sprscale_hbuf;
    long h = sheight;
    long tmp = (factor>>1) + (y<<16);
    cheight = tmp>>16;
    if ( cheight < 0 )
      cheight = 0;
    if ( cheight >= gheight )
      cheight = gheight;
    do
    {
      pheight[0] = cheight;
      tmp += factor;
      long cheight2 = tmp>>16;
      if ( cheight2 < 0 )
        cheight2 = 0;
      if ( cheight2 >= gheight )
        cheight2 = gheight;
      long hdiff = cheight2 - cheight;
      pheight[1] = hdiff;
      cheight += hdiff;
      pheight += 2;
      h--;
    }
    while (h>0);
  }
  else
  {
    pheight = sprscale_hbuf;
    long factor = (dheight<<16)/sheight;
    long tmp = (factor>>1) + (y<<16);
    cheight = tmp >> 16;
    long h = sheight;
    while ( 1 )
    {
      int i=0;
      for (i=0;i<16;i+=2)
      {
        pheight[i] = cheight;
        tmp += factor;
        pheight[i+1] = (tmp>>16) - cheight;
        cheight = (tmp>>16);
        h--;
        if (h<=0)
          break;
      }
      if (h<=0)
        break;
      pheight += 16;
    }
  }
}

// Internal function used to draw part of sprite line with single colour.
inline void LbDrawOneColorLine(unsigned char **buf_scr,const TbPixel colour,
        const unsigned int buf_len, const bool mirror)
{
  unsigned int i;
  if ( mirror )
  {
    if ( lbDisplay.DrawFlags & 0x0004 )
    {
#ifdef __DEBUG
    LbSyncLog("LbDrawOneColorLine: Drawing M4, %d points\n",buf_len);
#endif
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[(colour<<8) + **buf_scr];
            (*buf_scr)--;
        }
    } else
    {
#ifdef __DEBUG
    LbSyncLog("LbDrawOneColorLine: Drawing MX, %d points\n",buf_len);
#endif
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[((**buf_scr)<<8) + colour];
            (*buf_scr)--;
        }
    }
  } else
  {
    if ( lbDisplay.DrawFlags & 0x0004 )
    {
#ifdef __DEBUG
    LbSyncLog("LbDrawOneColorLine: Drawing S4, %d points\n",buf_len);
#endif
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[(colour<<8) + **buf_scr];
            (*buf_scr)++;
        }
    } else
    {
#ifdef __DEBUG
    LbSyncLog("LbDrawOneColorLine: Drawing SX, %d points\n",buf_len);
#endif
        for (i=0; i<buf_len; i++ )
        {
            **buf_scr = lbDisplay.GlassMap[((**buf_scr)<<8) + colour];
            (*buf_scr)++;
        }
    }
  }
}

// Internal function used to draw part of sprite line with single colour.
inline void LbDrawOneColorLineSolid(unsigned char **buf_scr,const TbPixel colour,
        const unsigned int buf_len, const bool mirror)
{
    unsigned int i;
    for (i=0;i<buf_len;i++)
    {
        **buf_scr = colour;
        (*buf_scr)--;
    }
}


int __fastcall LbSpriteDrawUsingScalingData(long posx, long posy, struct TbSprite *sprite)
{
  LbSyncLog("LbSpriteDrawUsingScalingData: UNFINISHED!\n");
  return 1;
}

int __fastcall LbSpriteDrawScaled(long xpos, long ypos, struct TbSprite *sprite, long dest_width, long dest_height)
{
#ifdef __DEBUG
    LbSyncLog("LbSpriteDrawScaled: Requested to draw at (%ld,%ld) sprite of size (%ld,%ld).\n",
            xpos,ypos,dest_width,dest_height);
#endif
  if ( (dest_width <= 0) || (dest_height <= 0) )
    return 1;
  lbSpriteReMapPtr = lbDisplay.FadeTable + ((lbDisplay.FadeStep & 0x3F) << 8);
  LbSpriteSetScalingData(xpos, ypos, sprite->SWidth, sprite->SHeight, dest_width, dest_height);
  return LbSpriteDrawUsingScalingData(0,0,sprite);
}

inline int LbSpriteDrawTrOneColor(char *sp,short sprWd,short sprHt,
        unsigned char *r,TbPixel colour,int nextRowDelta,short left,bool mirror)
{
  int x1;
  unsigned char *nextRow;
  long htIndex;
  nextRow = &(r[nextRowDelta]);
  x1 = sprWd;
  htIndex = sprHt;
#ifdef __DEBUG
    LbSyncLog("LbSpriteDrawTrOneColor: Drawing %ld lines of %hd points\n",htIndex,sprWd);
#endif
  while (1)
  {
#ifdef __DEBUG
    LbSyncLog("LbSpriteDrawTrOneColor: Line %ld\n",htIndex);
#endif
      if ( left != 0 )
      {
            unsigned char drawOut=0;
            short lpos = left;
            do {
              char chr = *sp;
              if ( chr == 0 )
              {
                x1 = *sp;
                break;
              } else
              if ( chr > lpos )
              {
                drawOut = *sp - lpos;
                if ( drawOut > sprWd )
                  drawOut = sprWd;
#ifdef __DEBUG
    LbSyncLog("LbSpriteDrawTrOneColor: Left line, screen buf pos=%ld, len=%d\n",r-lbDisplay.WScreen,(int)drawOut);
#endif
                LbDrawOneColorLine(&r,colour,drawOut,mirror);
                sp += chr + 1;
                x1 -= drawOut;
                break;
              } else
              if ( chr < -lpos )
              {
                drawOut = -(*sp) - lpos;
                if ( drawOut > sprWd )
                  drawOut = sprWd;
                if ( mirror )
                   r -= drawOut;
                else
                   r += drawOut;
                x1 -= drawOut;
                sp++;
                break;
              } else
              if ( chr > 0 )
              {
                  lpos -= *sp;
                  sp += *sp + 1;
              } else
              // if ( chr < 0 )
              {
                lpos += *sp;
                sp++;
              }
            } while (lpos > 0);
      }

      while (x1 > 0)
      {
            char schr = *sp;
            if ( schr == 0 )
              break;
            if ( schr < 0 )
            {
              x1 += schr;
              if ( mirror )
                 r += *sp;
              else
                 r -= *sp;
              sp++;
            } else
            {
              if ( schr >= x1 )
                schr = x1;
#ifdef __DEBUG
              LbSyncLog("LbSpriteDrawTrOneColor: X1 line, screen buf pos=%ld, len=%d\n",r-lbDisplay.WScreen,(int)schr);
#endif
              LbDrawOneColorLine(&r,colour,schr,mirror);
              x1 -= schr;
              sp += schr+1;
            }
      }//end while

      //Drawing loop end condition
      htIndex--;
      if ( htIndex<=0 )
          break;

      if ( x1 > 0 )
      {
          sp++;
      } else
      {
          char schr;
          do {
            schr=*sp;
            while ( schr>0 )
            {
                sp+=schr+1;
                schr=*sp;
            }
            sp++;
          } while ( schr != 0 );
      }
      r = nextRow;
      nextRow += nextRowDelta;
      x1 = sprWd;
  } //end while
  return 1;
}

inline int LbSpriteDrawSlOneColor(char *sp,short sprWd,short sprHt,
        unsigned char *r,TbPixel colour,int nextRowDelta,short left,bool mirror)
{
  int x1;
  unsigned char *nextRow;
  long htIndex;
  nextRow = r;
  htIndex = sprHt;
  while (1)
  {
      r = nextRow;
      nextRow += nextRowDelta;
      x1 = sprWd;
      if ( left != 0 )
      {
          unsigned char drawOut=0;
          short lpos = left;
          do {
              char chr=*sp;

              if ( chr == 0 )
              {
                x1 = *sp;
                break;
              } else
              if ( chr > lpos )
              {
                  drawOut = *sp - lpos;
                  if ( drawOut > sprWd )
                    drawOut = sprWd;
                  LbDrawOneColorLineSolid(&r,colour,drawOut,mirror);
                  sp += *sp + 1;
                  x1 -= drawOut;
                  break;
              } else
              if ( chr < -lpos )
              {
                drawOut = -*sp -lpos;
                if ( drawOut > sprWd )
                  drawOut = sprWd;
                r -= drawOut;
                x1 -= drawOut;
                sp++;
                break;
              } else
              if ( chr > 0 )
              {
                lpos -= *sp;
                sp += *sp + 1;
              } else
              //if ( chr < 0 )
              {
                lpos += *sp;
                sp++;
              }
          } while (lpos > 0);

      }
      while ( x1 > 0 )
      {
        char schr;
        schr = *sp;
        if ( schr == 0 )
        {
            break;
        }
        if ( schr < 0 )
        {
            x1 += schr;
            r += *sp;
            sp++;
        } else
        {
          if ( schr >= x1 )
            schr = x1;
          LbDrawOneColorLineSolid(&r,colour,schr,mirror);
          x1 -= schr;
          sp += schr + 1;
        }
      }

      htIndex--;
      if ( htIndex==0 )
          break;

      if ( x1 <= 0 )
      {
          char schr;
          do {
            schr=*sp;
            while ( schr>0 )
            {
                sp+=schr+1;
                schr=*sp;
            }
            sp++;
          } while ( schr != 0 );
      } else
      {
          sp++;
      }
  } //end while
  return 1;
}

inline int LbSpriteDrawFCOneColor(char *sp,short sprWd,short sprHt,
        unsigned char *r,TbPixel colour,int nextRowDelta,short left,bool mirror)
{
  int x1;
  unsigned char *nextRow;
  long htIndex;
  nextRow = &(r[nextRowDelta]);
  x1 = sprWd;
  htIndex = sprHt;
    while (1)
    {
        if ( left != 0 )
        {
          unsigned char drawOut=0;
          short lpos = left;
          do {
            char schr = *sp;
            while ( schr>0 )
            {
              if ( schr > lpos )
              {
                drawOut = *sp - lpos;
                if ( drawOut > sprWd )
                  drawOut = sprWd;
                memset( r, colour, drawOut);
                r += drawOut;
                sp += *sp + 1;
                x1 -= drawOut;
                break;
              } else
              {
                lpos -= *sp;
                sp += *sp + 1;
                if ( !lpos )
                  break;
              }
              schr = *sp;
            }

            if ( schr == 0 )
            {
              x1 = *sp;
              break;
            }
            if ( *sp < -lpos )
            {
              drawOut = -*sp - lpos;
              if ( drawOut > sprWd )
                drawOut = sprWd;
              sp++;
               r += drawOut;
              x1 -= drawOut;
              break;
            }
            lpos += *sp;
            sp++;
          } while (lpos != 0);
        }

      while ( x1 > 0 )
      {
        char schr = *sp;
        if ( schr == 0 )
          break;
        if ( schr < 0 )
        {
          x1 += schr;
          r -= *sp;
          sp++;
        } else
        //if ( schr > 0 )
        {
          if ( schr >= x1 )
            schr = x1;
          memset(r, colour, schr);
          x1 -= *sp;
          r += *sp;
          sp += (*sp)+1;
        }
      } //end while
      htIndex--;
      if (htIndex==0)
        return 1;
      if ( x1 <= 0 )
      {
        char schr;
        do {
          schr=*sp;
          while ( schr>0 )
          {
            sp+=schr+1;
            schr=*sp;
          }
          sp++;
        } while ( schr );
      } else
      {
        sp++;
      }
      r = nextRow;
      nextRow += nextRowDelta;
      x1 = sprWd;
    } //end while
}

int __fastcall LbSpriteDrawOneColour(long x, long y, struct TbSprite *spr, TbPixel colour)
{
#ifdef __DEBUG
    LbSyncLog("LbSpriteDrawOneColour: Requested to draw sprite at (%ld,%ld)\n",x,y);
#endif
  if ( (spr->SWidth<=0) || (spr->SHeight<=0) )
    return 1;
  if ( (lbDisplay.GraphicsWindowWidth<=0) || (lbDisplay.GraphicsWindowHeight<=0) )
    return 1;
  x += lbDisplay.GraphicsWindowX;
  y += lbDisplay.GraphicsWindowY;
  short left,right,top,btm;
  short sprWd = spr->SWidth;
  short sprHt = spr->SHeight;
  //Coordinates range checking
  int delta;
  delta = lbDisplay.GraphicsWindowX - x;
  if ( delta <= 0 )
  {
    left = 0;
  } else
  {
    if ( sprWd <= delta )
      return 1;
    left = delta;
  }
  delta = x + sprWd - (lbDisplay.GraphicsWindowWidth+lbDisplay.GraphicsWindowX);
  if ( delta <= 0 )
  {
    right = sprWd;
  } else
  {
    if ( sprWd <= delta )
      return 1;
    right = sprWd - delta;
  }
  delta = lbDisplay.GraphicsWindowY - y;
  if ( delta <= 0 )
  {
    top = 0;
  } else
  {
    if ( sprHt <= delta )
      return 1;
    top = delta;
  }
  delta = y + sprHt - (lbDisplay.GraphicsWindowHeight + lbDisplay.GraphicsWindowY);
  if ( y + sprHt - (lbDisplay.GraphicsWindowHeight + lbDisplay.GraphicsWindowY) <= 0 )
  {
    btm = sprHt;
  } else
  {
    if ( sprHt <= delta )
      return 1;
    btm = sprHt - delta;
  }

  unsigned char *tgt;
  int nextRowDelta;
  if ( lbDisplay.DrawFlags & 0x0002 )
  {
    tgt = &lbDisplay.WScreen[x + (y+btm-1)*lbDisplay.GraphicsScreenWidth + left];
    nextRowDelta = -lbDisplay.GraphicsScreenWidth;
    short tmp_btm = btm;
    btm = sprHt - top;
    top = sprHt - tmp_btm;
  } else
  {
    tgt = &lbDisplay.WScreen[x + (y+top)*lbDisplay.GraphicsScreenWidth + left];
    nextRowDelta = lbDisplay.GraphicsScreenWidth;
  }
  sprHt = btm - top;
  sprWd = right - left;
  char *sp = spr->Data;
#ifdef __DEBUG
    LbSyncLog("LbSpriteDrawOneColour: Sprite coords X=%d...%d Y=%d...%d data=%08x\n",left,right,top,btm,sp);
#endif
  long htIndex;
  if ( top )
  {
    htIndex = top;
    while ( 1 )
    {
      char chr = *sp;
      while ( chr>0 )
      {
        sp += chr + 1;
        chr = *sp;
      }
      sp++;
      if ( chr == 0 )
      {
        htIndex--;
        if (htIndex<=0) break;
      }
    }
  }
#ifdef __DEBUG
    LbSyncLog("LbSpriteDrawOneColour: Drawing sprite of size (%ld,%ld)\n",sprHt,sprWd);
#endif
  unsigned char *nextRow;
  bool mirror;
  int x1;
  if ( lbDisplay.DrawFlags & (0x0004|0x0008|0x0001) )
  {
    if ( lbDisplay.DrawFlags & 0x0001 )
    {
      tgt += sprWd - 1;
      mirror = true;
      short tmpwidth = spr->SWidth;
      short tmpright = right;
      right = tmpwidth - left;
      left = tmpwidth - tmpright;
    }
    else
    {
      mirror = false;
    }
    if ( lbDisplay.DrawFlags & (0x0004|0x0008) )
      return LbSpriteDrawTrOneColor(sp,sprWd,sprHt,tgt,colour,nextRowDelta,left,mirror);
    else
      return LbSpriteDrawSlOneColor(sp,sprWd,sprHt,tgt,colour,nextRowDelta,left,mirror);
  }
  return LbSpriteDrawFCOneColor(sp,sprWd,sprHt,tgt,colour,nextRowDelta,left,mirror);
}

void __fastcall draw_b_line(long x1, long y1, long x2, long y2, TbPixel colour)
{
  long apx = 2 * abs(x2 - x1);
  long spx;
  if ( x2-x1 <= 0 )
    spx = -1;
  else
    spx = 1;
  long apy = 2 * abs(y2 - y1);
  long spy;
  if ( y2-y1 <= 0 )
    spy = -1;
  else
    spy = 1;
  long doffy = spy * lbDisplay.GraphicsScreenWidth;
  long offset = lbDisplay.GraphicsScreenWidth * y1 + x1;
  long x = x1;
  long y = y1;
  if ( lbDisplay.DrawFlags & 4 )
  {
    if (apx <= apy)
    {
      long d = apx - (apy>>1);
      while ( true )
      {
        unsigned short glass_idx = lbDisplay.GraphicsWindowPtr[offset]
                + ((unsigned char)colour<<8);
        lbDisplay.GraphicsWindowPtr[offset] = lbDisplay.GlassMap[glass_idx];
        if (y==y2) break;
        if (d>=0)
        {
          offset += spx;
          d -= apy;
        }
        y += spy;
        offset += doffy;
        d += apx;
      }
    } else
    {
      long d = apy - (apx >> 1);
      while ( true )
      {
        unsigned short glass_idx = lbDisplay.GraphicsWindowPtr[offset]
                + ((unsigned char)colour<<8);
        lbDisplay.GraphicsWindowPtr[offset] = lbDisplay.GlassMap[glass_idx];
        if (x==x2) break;
        if (d>=0)
        {
          offset += doffy;
          d -= apx;
        }
        x += spx;
        offset += spx;
        d += apy;
      }
    }
  } else
  if ( lbDisplay.DrawFlags & 8 )
  {
      if ( apx <= apy )
      {
        long d = apx - (apy >> 1);
        while ( 1 )
        {
          unsigned short glass_idx = (lbDisplay.GraphicsWindowPtr[offset]<<8)
                + ((unsigned char)colour);
          lbDisplay.GraphicsWindowPtr[offset] = lbDisplay.GlassMap[glass_idx];
          if (y==y2) break;
          if (d>=0)
          {
            offset += spx;
            d -= apy;
          }
          y += spy;
          offset += doffy;
          d += apx;
        }
      } else
      {
        long d = apy - (apx >> 1);
        while ( 1 )
        {
          unsigned short glass_idx = (lbDisplay.GraphicsWindowPtr[offset]<<8)
                + ((unsigned char)colour);
          lbDisplay.GraphicsWindowPtr[offset] = lbDisplay.GlassMap[glass_idx];
          if (x==x2) break;
          if (d>=0)
          {
            offset += doffy;
            d -= apx;
          }
          x += spx;
          offset += spx;
          d += apy;
        }
      }
  } else
  {
      if ( apx <= apy )
      {
        long d = apx - (apy >> 1);
        while ( true )
        {
          lbDisplay.GraphicsWindowPtr[offset] = ((unsigned char)colour);
          if (y==y2) break;
          if ( d >= 0 )
          {
            offset += spx;
            d -= apy;
          }
          y += spy;
          offset += doffy;
          d += apx;
        }
      }
      else
      {
        long d = apy - (apx >> 1);
        while ( 1 )
        {
          lbDisplay.GraphicsWindowPtr[offset] = ((unsigned char)colour);
          if ( x == x2 )
            break;
          if ( d >= 0 )
          {
            offset += doffy;
            d -= apx;
          }
          x += spx;
          offset += spx;
          d += apy;
        }
      }
  }
}

//Draws a line on current graphics window. Truncates the coordinates
// if they go off the window. Does no screen locking.
char __fastcall LbDrawLine(long x1, long y1, long x2, long y2, TbPixel colour)
{
  char result=0;
  // Adjusting X-dimension coordinates
  long width_max = lbDisplay.GraphicsWindowWidth - 1;
  if ( x1 >= 0 )
  {
    if ( x1 <= width_max )
    {
      if ( x2 >= 0 )
      {
        if ( x2 > width_max )
        {
          y2 -= (x2-width_max)*(y2-y1) / (x2-x1);
          x2 = width_max;
          result = 1;
        }
      } else
      {
        y2 -= x2 * (y2-y1) / (x2-x1);
        x2 = 0;
        result = 1;
      }
    } else
    {
      if ( x2 > width_max ) return 1;
      y1 -= (x1-width_max) * (y1-y2) / (x1-x2);
      x1 = width_max;
      result = 1;
      if ( x2 < 0 )
      {
        y2 -= x2 * (y2-y1) / (x2-x1);
        x2 = 0;
      }
    }
  }
  else
  {
    if ( x2 < 0 ) return 1;
    y1 -= x1 * (y1-y2) / (x1-x2);
    x1 = 0;
    result = 1;
    if ( x2 > width_max )
    {
      y2 -= (x2-width_max) * (y2-y1) / (x2-x1);
      x2 = lbDisplay.GraphicsWindowWidth - 1;
    }
  }
  // Adjusting Y-dimension coordinates
  long height_max = lbDisplay.GraphicsWindowHeight - 1;
  if ( y1 < 0 )
  {
    if ( y2 < 0 ) return 1;
    x1 -= y1 * (x1-x2) / (y1-y2);
    y1 = 0;
    result = 1;
    if ( y2 > height_max )
    {
      x2 -= (y2-height_max) * (x2-x1) / (y2-y1);
      y2 = height_max;
    }
  } else
  if ( y1 <= height_max )
  {
    if ( y2 >= 0 )
    {
      if ( y2 > height_max )
      {
        x2 -= (y2-height_max) * (x2-x1) / (y2-y1);
        y2 = height_max;
        result = 1;
      }
    } else
    {
      x2 -= y2 * (x2-x1) / (y2 - y1);
      y2 = 0;
      result = 1;
    }
  } else
  {
    if ( y2 > height_max )
      return 1;
    x1 -= (y1-height_max) * (x1-x2) / (y1-y2);
    y1 = height_max;
    result = 1;
    if ( y2 < 0 )
    {
      x2 -= y2 * (x2-x1) / (y2-y1);
      y2 = 0;
    }
  }
  draw_b_line(x1, y1, x2, y2, colour);
  return result;
}

//Draws any triangle on the current graphics window.
//Does no screen locking.
void __fastcall LbDrawTriangle(long x1, long y1, long x2, long y2, long x3, long y3, TbPixel colour)
{
  if ( lbDisplay.DrawFlags & 0x0010 )
  {
    LbDrawLine(x1, y1, x2, y2, colour);
    LbDrawLine(x2, y2, x3, y3, colour);
    LbDrawLine(x3, y3, x1, y1, colour);
  }
  else
  {
    LbDrawTriangleFilled(x1, y1, x2, y2, x3, y3, colour);
  }
}

// Draws a filled box on current graphic window.
// Does not perform any clipping to input variables.
// Does no screen locking.
void __fastcall LbDrawBoxNoClip(long x, long y, unsigned long width, unsigned long height, TbPixel colour)
{
  unsigned long idxh = height;
  //Space between lines in video buffer
  unsigned long screen_delta = lbDisplay.GraphicsScreenWidth - width;
  unsigned char *screen_ptr = lbDisplay.GraphicsWindowPtr + x + lbDisplay.GraphicsScreenWidth * y;
  if ( lbDisplay.DrawFlags & 4 )
  {
      unsigned short glass_idx = (unsigned char)colour << 8;
      do {
          unsigned long idxw = width;
          do {
                glass_idx&=0xff00;
                glass_idx |= *screen_ptr;
                *screen_ptr = lbDisplay.GlassMap[glass_idx];
                screen_ptr++;
                idxw--;
          } while ( idxw>0 );
          screen_ptr += screen_delta;
          idxh--;
      } while ( idxh>0 );
  } else
  if ( lbDisplay.DrawFlags & 8 )
  {
      unsigned short glass_idx = (unsigned char)colour;
      do {
            unsigned long idxw = width;
            do {
              glass_idx&=0x00ff;
              glass_idx |= ((*screen_ptr)<<8);
              *screen_ptr = lbDisplay.GlassMap[glass_idx];
              screen_ptr++;
              idxw--;
            } while ( idxw>0 );
            screen_ptr += screen_delta;
            idxh--;
      } while ( idxh>0 );
  } else
  {
      unsigned char col_idx = colour;
      do {
            unsigned long idxw = width;
            do {
              *screen_ptr = col_idx;
              screen_ptr++;
              idxw--;
            } while ( idxw>0 );
            screen_ptr += screen_delta;
            idxh--;
      } while ( idxh>0 );
  }
}

// Draws a rectangular box on current graphics window.
// Locks and unlocks screen as needed. If wrong dimensions or can't lock,
// returns -1. On success returns 1. Gets two point coords as parameters.
int __fastcall LbDrawBoxCoords(long xpos1, long ypos1, long xpos2, long ypos2, TbPixel colour)
{
  if ( xpos1 > xpos2 )
  {
    xpos1 ^= xpos2;
    xpos2 ^= xpos1;
    xpos1 ^= xpos2;
  }
  if ( ypos1 > ypos2 )
  {
    ypos1 ^= ypos2;
    ypos2 ^= ypos1;
    ypos1 ^= ypos2;
  }
  return LbDrawBox(xpos1, ypos1, xpos2 - xpos1 + 1, ypos2 - ypos1 + 1, colour);
}

//Copies rectangle from current screen buffer into back buffer
// (from WScreen to back_buffer). Does no screen locking.
void __fastcall block_screen_copy(int x, int y, unsigned int width, unsigned int height)
{
  unsigned int i;
  for (i=0;i<height;i++)
  {
    unsigned long bufpos=(y+i)*lbDisplay.GraphicsScreenWidth + x;
    memcpy((uchar *)back_buffer+bufpos,(uchar *)lbDisplay.WScreen+bufpos,width);
  }
}

inline void parse_text_line(const char *text, const long lastlinepos, const long linelength,
        const bool draw, long *x, long *y)
{
    long j;
    for (j=lastlinepos; j<linelength ;j++)
    {
      unsigned char cj=text[j];
      switch (cj)
      {
      case  1:
          if ( lbDisplay.DrawFlags & 0x0004 )
              lbDisplay.DrawFlags &= 0xFFFB;
          else
              lbDisplay.DrawFlags |= 0x0004;
          break;
      case 12:
          if ( lbDisplay.DrawFlags & 0x0040 )
              lbDisplay.DrawFlags &= 0xFFBF;
          else
              lbDisplay.DrawFlags |= 0x0040;
          break;
      case 14: {
          j++;
          unsigned char col_idx=text[j];
          lbDisplay.DrawColour = text_colours[col_idx-1];
          };break;
      case 27:
          lbDisplay.DrawFlags |= 0x0040;
          break;
      case 28:
          lbDisplay.DrawFlags &= 0xFFBF;
          break;
      case 30:
          lbDisplay.DrawFlags &= 0xFFFB;
          break;
      default: {
          if (!draw) break;
          if ((cj<=31) || (lbFontPtr==NULL)) break;
          //Now we can draw a character
          if ( (lbFontPtr!=sprites.Font3) || (lang_selection!=1) )
              cj = my_to_upper(cj);
          struct TbSprite *chrsprite = &lbFontPtr[cj-31];
          if ((lbDisplay.DrawFlags & 0x0040) != 0)
          {
              LbSpriteDrawOneColour(*x, (*y)-font_offset(cj), chrsprite,
                  lbDisplay.DrawColour);
          } else
          {
              LbSpriteDraw(*x, (*y)-font_offset(cj), chrsprite);
          }
              *x += chrsprite->SWidth;
          };break;
      }
    } //end for
}

//Draws a bitmap text on screen. You must lock the screen before drawing.
unsigned short __fastcall my_draw_text(long x, long y, const char *text, const long startline)
{
  if ( text == NULL )
  {
#ifdef __DEBUG
      LbSyncLog("my_draw_text: Text is null, skipped drawing at (%ld,%ld).\n",
            x,y);
#endif
    return 0;
  }
#ifdef __DEBUG
      LbSyncLog("my_draw_text: Request for drawing line %d of text at (%ld,%ld).\n",
            startline,x,y);
#endif
  x += text_window_x1;
  y += text_window_y1;
  long start_x = x;
  long linecount = 0;
  long last_word_width = 0;
  long width_so_far = 0;
  long line_width = text_window_x2 - x;
  long lastwordpos = 0;
  long lastlinepos = 0;

  long i;
  unsigned char ci=text[0];

  for (i=0;ci!=0;i++)
  {
      ci=text[i];
      if ( (lbFontPtr!=sprites.Font3) || (lang_selection!=1) )
        ci = my_to_upper(ci);
      switch (ci)
      {
      case 1:
        break;
      case 10:
            if ( linecount < startline )
            {
              parse_text_line(text,lastlinepos,i,false,&x,&y);
            } else
            {
              if ( y >= text_window_y1 )
              {
                if ( lbDisplay.DrawFlags & 0x0080 )
                  x += line_width-width_so_far;
                else
                if ( lbDisplay.DrawFlags & 0x0100 )
                    x += ((line_width-width_so_far)>>1);
#ifdef __DEBUG
      LbSyncLog("my_draw_text: Drawing case 10 at (%ld,%ld).\n",
            x,y);
#endif
                parse_text_line(text,lastlinepos,i,true,&x,&y);
              }
              y += font_height('A')+my_line_spacing;
              if ( font_height('A')+y > text_window_y2 )
                return linecount;
            }
            x = start_x;
            linecount++;
            lastlinepos = i+1;
            lastwordpos = i+1;
            width_so_far = 0;
        break;
      case 14:
        i++;
        break;
      case 27:
        break;
      case 28:
        break;
      //Further cases come through to default case
      case ' ':
      case '-':
        lastwordpos = i+1;
        if ( ci == '-' )
        {
          int widthinc;
          if ( lbFontPtr )
            widthinc = lbFontPtr[ci-31].SWidth;
          else
            widthinc = 0;
          last_word_width = widthinc + width_so_far;
        } else
        {
          last_word_width = width_so_far;
        }
      default: {
          if (ci<=31) break;
          int widthinc;
          if ( lbFontPtr )
            widthinc = lbFontPtr[ci-31].SWidth;
          else
            widthinc = 0;
          width_so_far += widthinc;
        };break;
      }
      if ( width_so_far <= line_width )
          continue;

      if ( lastwordpos == lastlinepos )
      {
          int widthinc;
          if ( lbFontPtr )
            widthinc = lbFontPtr[text[i+1]-31].SWidth;
          else
            widthinc = 0;
          width_so_far -= widthinc;
          do
            i--;
          while ( text[i+1] < 32 );
          if ( lbFontPtr )
            widthinc = lbFontPtr[text[i+1]-31].SWidth;
          else
            widthinc = 0;
          width_so_far -= widthinc;
          do
            i--;
          while ( text[i+1] < 32 );
          if ( lbFontPtr )
            widthinc = lbFontPtr[text[i]-31].SWidth;
          else
            widthinc = 0;
          width_so_far -= widthinc;
          if ( lbFontPtr )
            widthinc = lbFontPtr['-'-31].SWidth;
          else
            widthinc = 0;
          width_so_far += widthinc;

          if ( linecount < startline )
          {
              parse_text_line(text,lastlinepos,i,false,&x,&y);
          } else
          {
            if ( y >= text_window_y1 )
            {
              if ( lbDisplay.DrawFlags & 0x0080 )
              {
                x += line_width - width_so_far;
              } else
              {
                if ( lbDisplay.DrawFlags & 0x0100 )
                  x = ((line_width-width_so_far)>>1) + x;
              }
#ifdef __DEBUG
      LbSyncLog("my_draw_text: Drawing P1 char '%c' at (%ld,%ld).\n",
            ci,x,y);
#endif
              parse_text_line(text,lastlinepos,i,true,&x,&y);
              if ( lbFontPtr != NULL)
              {
                struct TbSprite *chrsprite = &lbFontPtr['-'-31];
                if ( lbDisplay.DrawFlags & 0x0040 )
                {
                    LbSpriteDrawOneColour(x, y-font_offset('-'), chrsprite,
                        lbDisplay.DrawColour);
                 } else
                 {
                     LbSpriteDraw(x, y-font_offset('-'), chrsprite);
                 }
                 x += chrsprite->SWidth;
              }
            }
            y += font_height('A')+my_line_spacing;
            if ( font_height('A')+y > text_window_y2 )
              return linecount;
          }
          linecount++;
          lastwordpos = i+1;
          lastlinepos = i+1;
          last_word_width = 0;
          width_so_far = 0;
          x = start_x;
      } else
      {
          if ( linecount < startline )
          {
            parse_text_line(text,lastlinepos,i,false,&x,&y);
          } else
          {
            if ( y >= text_window_y1 )
            {
              if ( lbDisplay.DrawFlags & 0x0080 )
              {
                x += line_width - last_word_width;
              }
              else
              {
                if ( lbDisplay.DrawFlags & 0x0100 )
                  x = ((line_width-last_word_width)>>1) + x;
              }
#ifdef __DEBUG
      LbSyncLog("my_draw_text: Drawing P2 char '%c' at (%ld,%ld).\n",
            ci,x,y);
#endif
              parse_text_line(text,lastlinepos,i,true,&x,&y);
            }
            y += font_height('A')+my_line_spacing;
            if ( font_height('A')+y > text_window_y2 )
              return linecount;
          }
          linecount++;
          lastlinepos = lastwordpos;
          width_so_far -= last_word_width;
          if ( text[lastwordpos-1] == ' ' )
          {
            int widthinc;
            if ( lbFontPtr )
              widthinc = lbFontPtr[' '-31].SWidth;
            else
              widthinc = 0;
            width_so_far -= widthinc;
          }
          x = start_x;
      }
  }//loop end

  //Setting proper draw flags
  if ( linecount < startline )
  {//Previous lines - skip drawing, just adjust variables
    parse_text_line(text,lastlinepos,i,false,&x,&y);
    return linecount;
  }
  //Now the lines we want to draw
  if ( y >= text_window_y1 )
  {
      if ( lbDisplay.DrawFlags & 0x0080 )
      {
        x += line_width - width_so_far;
      } else
      {
        if ( lbDisplay.DrawFlags & 0x0100 )
            x += ((line_width-width_so_far)>>1);
      }
#ifdef __DEBUG
      LbSyncLog("my_draw_text: Drawing P3 char '%c' at (%ld,%ld).\n",
            ci,x,y);
#endif
      parse_text_line(text,lastlinepos,i,true,&x,&y);
  }
  linecount++;
  return linecount;
}

// Menu engine screen drawing function
void __fastcall draw_purple_screen(void)
{
  static char joy_move;
  LbScreenSetGraphicsWindow(0,0,640,480);
  my_set_text_window(0,0,640,480);
  setup_vecs(lbDisplay.WScreen,vec_tmap,lbDisplay.PhysicalScreenWidth,
       lbDisplay.PhysicalScreenWidth,lbDisplay.PhysicalScreenHeight);
  // Setup engine points
  struct EnginePoint origin;
  struct EnginePoint point2;
  struct EnginePoint point3;
  origin.X=proj_origin.X;
  origin.Y=proj_origin.Y;
  origin.Shade=0x200000;
  point3.Shade=0x200000;
  point2.Shade=0x8000;
  vec_mode=17;
  int hs=0; //hotspot index
  int draw_item_idx=0;
  if (LbScreenLock() == Lb_SUCCESS)
  {
    while (draw_item_idx < purple_draw_index)
    {
      PurpleDrawItem *draw_item=&purple_draw_list[draw_item_idx];
      lbDisplay.DrawFlags=draw_item->Flags;
      unsigned char itm_type=draw_item->Type;
      switch (itm_type)
      {
      case 1:
        LbDrawBox(draw_item->U.Box.X,draw_item->U.Box.Y,draw_item->U.Box.Width,
            draw_item->U.Box.Height,draw_item->U.Box.Colour);
        if (lbDisplay.DrawFlags & 0x8000)
        {
          hotspot_buffer[hs].X = draw_item->U.Box.X + (draw_item->U.Box.Width>>1);
          hotspot_buffer[hs].Y = draw_item->U.Box.Y + (draw_item->U.Box.Height>>1);
          hs++;
        }
        break;
      case 2:
        lbDisplay.DrawColour=draw_item->U.Text.Colour;
        lbFontPtr=draw_item->U.Text.Font;
        my_set_text_window(draw_item->U.Text.WindowX,draw_item->U.Text.WindowY,
                draw_item->U.Text.Width,draw_item->U.Text.Height);
        my_draw_text(draw_item->U.Text.X,draw_item->U.Text.Y,
                draw_item->U.Text.Text,draw_item->U.Text.Line);
        if (lbDisplay.DrawFlags & 0x8000)
        {
          int strwidth=my_string_width(draw_item->U.Text.Text);
          if ((draw_item->U.Text.Width>=strwidth) || (lbDisplay.DrawFlags & 0x0100))
          {
            hotspot_buffer[hs].X = draw_item->U.Text.WindowX
                    + (draw_item->U.Text.Width>>1);
          } else
          {
            hotspot_buffer[hs].X = draw_item->U.Text.WindowX+draw_item->U.Text.X
                    + (strwidth>>1);
          }
          hotspot_buffer[hs].Y = draw_item->U.Text.WindowY+draw_item->U.Text.Y
                  + (font_height('A')>>1);
          hs++;
        }
        break;
      case 4:
         block_screen_copy(draw_item->U.Box.X,draw_item->U.Box.Y,
                 draw_item->U.Box.Width,draw_item->U.Box.Height);
        break;
      case 5:
        lbDisplay.DrawColour=draw_item->U.Sprite.Colour;
        if (lbDisplay.DrawFlags & 0x0040)
        {
          LbSpriteDrawOneColour(draw_item->U.Sprite.X,draw_item->U.Sprite.Y,
                  draw_item->U.Sprite.Sprite,lbDisplay.DrawColour);
        } else
        {
          LbSpriteDraw(draw_item->U.Sprite.X,draw_item->U.Sprite.Y,
                  draw_item->U.Sprite.Sprite);
        }
        if (lbDisplay.DrawFlags & 0x8000)
        {
          struct TbSprite *spr=draw_item->U.Sprite.Sprite;
          hotspot_buffer[hs].X = draw_item->U.Sprite.X + (spr->SWidth>>1);
          hotspot_buffer[hs].Y = draw_item->U.Sprite.Y + (spr->SHeight>>1);
          hs++;
        }
        break;
      case 6:
        vec_colour=draw_item->U.Trig.Colour;
        point2.X=draw_item->U.Trig.X2;
        point2.Y=draw_item->U.Trig.Y2;
        point3.X=draw_item->U.Trig.X3;
        point3.Y=draw_item->U.Trig.Y3;
        if (is_it_clockwise(&origin,&point2,&point3))
          trig(&origin,&point2,&point3);
        else
          trig(&origin,&point3,&point2);
        break;
      case 7:
        draw_item->U.Flic.Function();
      break;
      case 9:
        LbDrawLine(draw_item->U.Line.X1,draw_item->U.Line.Y1,
                draw_item->U.Line.X2,draw_item->U.Line.Y2,draw_item->U.Line.Colour);
        break;
      case 10:
        LbDrawHVLine(draw_item->U.Line.X1,draw_item->U.Line.Y1,
                draw_item->U.Line.X2,draw_item->U.Line.Y2,draw_item->U.Line.Colour);
        break;
      case 11:
        LbDrawTriangle(draw_item->U.Triangle.X1,draw_item->U.Triangle.Y1,
            draw_item->U.Triangle.X2,draw_item->U.Triangle.Y2,
            draw_item->U.Triangle.X3,draw_item->U.Triangle.Y3,draw_item->U.Triangle.Colour);
        break;
      case 12:
        hotspot_buffer[hs].X=draw_item->U.Hotspot.X;
        hotspot_buffer[hs].Y=draw_item->U.Hotspot.Y;
        hs++;
        break;
      case 3:
      case 8:
      default:
        break;
      }
      draw_item_idx++;
    }
    LbScreenUnlock();
  }
  purple_draw_index=0;
  lbDisplay.DrawFlags=0;
  ulong nearest;
  int new_hs=0; //hotspot index
  //Note: there may be error in the code below - it isn't verified
  if (joy_move!=0)
  {
    if ( (joy.DigitalY[0]==0) && (joy.DigitalX[0]==0) )
        joy_move=0;
  } else
  {// so (joy_move==0)
    if (joy.DigitalY[0] == 1)
    {
            nearest=0x80000000;
            int i;
            for (i=0;i<hs;i++)
            {
              if (hotspot_buffer[i].Y > lbDisplay.MMouseY)
              {
                uint diff_y=hotspot_buffer[i].Y - lbDisplay.MMouseY;
                uint diff_x=abs(hotspot_buffer[i].X - lbDisplay.MMouseX);
                if (diff_y > diff_x)
                {
                  ulong distance=diff_y + (diff_x>>1);
                  if ( (distance>nearest) && (distance>0))
                  {
                    nearest=distance;
                    new_hs=i;
                  }
                }
              }
            }//end for
            if (nearest!=0x80000000)
            {
              LbMouseSetPosition(hotspot_buffer[new_hs].X,hotspot_buffer[new_hs].Y);
            }
            joy_move=0;
    } else
    if (joy.DigitalY[0] == -1)
    {
            nearest=0x80000000;
            int i;
            for (i=0;i<hs;i++)
            {
              if (hotspot_buffer[i].Y < lbDisplay.MMouseY)
              {
                uint diff_y=lbDisplay.MMouseY-hotspot_buffer[i].Y;
                uint diff_x=abs(hotspot_buffer[i].X - lbDisplay.MMouseX);
                if (diff_y>diff_x)
                {
                  ulong distance=diff_y+(diff_x>>1);
                  if ( (distance>nearest) && (distance>0))
                  {
                    nearest=distance;
                    new_hs=i;
                  }
                }
              }
            }//end for
            if (nearest!=0x80000000)
              LbMouseSetPosition(hotspot_buffer[new_hs].X,hotspot_buffer[new_hs].Y);
            joy_move=1;
    } else
    if (joy.DigitalX[0] == 1)
    {
            nearest=0x80000000;
            int i;
            for (i=0;i<hs;i++)
            {
              if (hotspot_buffer[i].X > lbDisplay.MMouseX)
              {
                uint diff_x=hotspot_buffer[i].X - lbDisplay.MMouseX;
                uint diff_y=abs(hotspot_buffer[i].Y - lbDisplay.MMouseY);
                if (diff_x>diff_y)
                {
                  ulong distance=diff_x+(diff_y>>1);
                  if ( (distance>nearest) && (distance>0))
                  {
                    nearest=distance;
                    new_hs=i;
                  }
                }
              }
            }//end for
            if (nearest!=0x80000000)
              LbMouseSetPosition(hotspot_buffer[new_hs].X,hotspot_buffer[new_hs].Y);
            joy_move=1;
    } else
    if (joy.DigitalX[0] == -1)
    {
            nearest=0x80000000;
            int i;
            for (i=0;i<hs;i++)
            {
              if (hotspot_buffer[i].X < lbDisplay.MMouseX)
              {
                uint diff_x=lbDisplay.MMouseX - hotspot_buffer[i].X;
                uint diff_y=abs(hotspot_buffer[i].Y - lbDisplay.MMouseY);
                if (diff_x>diff_y)
                {
                  ulong distance=diff_x+(diff_y>>1);
                  if ( (distance>nearest) && (distance>0))
                  {
                    nearest=distance;
                    new_hs=i;
                  }
                }
              }
            }//end for
            if (nearest!=0x80000000)
              LbMouseSetPosition(hotspot_buffer[new_hs].X,hotspot_buffer[new_hs].Y);
            joy_move=1;
    }
  }
}
*/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
