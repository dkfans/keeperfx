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
#include <assert.h>
#include "globals.h"

#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_mouse.h"
#include "bflib_render.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbSpriteDrawData {
    char *sp;
    short Wd;
    short Ht;
    unsigned char *r;
    int nextRowDelta;
    short startShift;
    TbBool mirror;
};
/******************************************************************************/
long xsteps_array[2*SPRITE_SCALING_XSTEPS];
long ysteps_array[2*SPRITE_SCALING_YSTEPS];
long alpha_xsteps_array[2*SPRITE_SCALING_XSTEPS];
long alpha_ysteps_array[2*SPRITE_SCALING_YSTEPS];
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
unsigned char *dither_map=NULL;
unsigned char *dither_end=NULL;
struct StartScreenPoint proj_origin = { (640>>1)-1, ((480+60)>>1)-1 };
struct StartScreenPoint *hotspot_buffer=hots;
unsigned char *lbSpriteReMapPtr;
*/
/******************************************************************************/
/**  Prints horizontal or vertical line on current graphics window.
 *  Does no screen locking - screen must be lock before and unlocked
 *  after a call to this function.
 *
 * @param xpos1
 * @param ypos1
 * @param xpos2
 * @param ypos2
 * @param colour
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
    if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4)
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
      if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8)
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
    if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4)
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
      if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8)
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

/** Draws a filled box on current graphic window.
 *  Performs clipping if needed to stay inside the window.
 *  Does no screen locking.
 *
 * @param x
 * @param y
 * @param width
 * @param height
 * @param colour
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
  if ( lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4 )
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
  if ( lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8 )
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

/** Draws a rectangular box on current graphics window.
 *  Does no screen locking.
 *
 * @param x Box left border coordinate.
 * @param y Box top border coordinate.
 * @param width Box width.
 * @param height Box height.
 * @param colour Colour index used to draw the box.
 * @return If wrong dimensions gives Lb_FAIL. On success gives Lb_SUCCESS.
 */
TbResult LbDrawBox(long x, long y, unsigned long width, unsigned long height, TbPixel colour)
{
    if (lbDisplay.DrawFlags & Lb_SPRITE_OUTLINE)
    {
        if ((width < 1) || (height < 1))
          return Lb_FAIL;
        LbDrawHVLine(x, y, width + x - 1, y, colour);
        LbDrawHVLine(x, height + y - 1, width + x - 1, height + y - 1, colour);
        if (height > 2)
        {
          LbDrawHVLine(x, y + 1, x, height + y - 2, colour);
          LbDrawHVLine(width + x - 1, y + 1, width + x - 1, height + y - 2, colour);
        }
    } else
    {
        LbDrawBoxClip(x, y, width, height, colour);
    }
    return Lb_SUCCESS;
}

/** Internal function used to prepare sprite drawing.
 *  Fills TbSpriteDrawData struct with values accepted by drawing routines.
 *
 * @param spd The TbSpriteDrawData struct to be filled.
 * @param x Drawing position x coordinate.
 * @param y Drawing position y coordinate.
 * @param spr Sprite to be drawn.
 * @return Gives Lb_SUCCESS if the data was prepared.
 */
static inline TbResult LbSpriteDrawPrepare(struct TbSpriteDrawData *spd, long x, long y, const struct TbSprite *spr)
{
    if (spr == NULL)
    {
        SYNCDBG(19,"NULL sprite");
        return Lb_FAIL;
    }
    if ((spr->SWidth < 1) || (spr->SHeight < 1))
    {
        SYNCDBG(19,"Zero size sprite (%d,%d)",spr->SWidth,spr->SHeight);
        return Lb_OK;
    }
    if ((lbDisplay.GraphicsWindowWidth == 0) || (lbDisplay.GraphicsWindowHeight == 0))
    {
        SYNCDBG(19,"Invalid graphics window dimensions");
        return Lb_FAIL;
    }
    x += lbDisplay.GraphicsWindowX;
    y += lbDisplay.GraphicsWindowY;
    short left;
    short right;
    short top;
    short btm;
    short sprWd = spr->SWidth;
    short sprHt = spr->SHeight;
    //Coordinates range checking - x coords
    int delta;
    delta = lbDisplay.GraphicsWindowX - x;
    if (delta <= 0)
    {
        left = 0;
    } else
    {
        if (sprWd <= delta)
            return Lb_OK;
        left = delta;
    }
    delta = x + sprWd - (lbDisplay.GraphicsWindowWidth+lbDisplay.GraphicsWindowX);
    if ( delta <= 0 )
    {
        right = sprWd;
    } else
    {
        if (sprWd <= delta)
            return Lb_OK;
        right = sprWd - delta;
    }
    //Coordinates range checking - y coords
    delta = lbDisplay.GraphicsWindowY - y;
    if (delta <= 0)
    {
      top = 0;
    } else
    {
      if (sprHt <= delta)
        return Lb_OK;
      top = delta;
    }
    delta = y + sprHt - (lbDisplay.GraphicsWindowHeight + lbDisplay.GraphicsWindowY);
    if (y + sprHt - (lbDisplay.GraphicsWindowHeight + lbDisplay.GraphicsWindowY) <= 0)
    {
      btm = sprHt;
    } else
    {
      if (sprHt <= delta)
        return Lb_OK;
      btm = sprHt - delta;
    }
    if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_VERTIC) != 0)
    {
        spd->r = &lbDisplay.WScreen[x + (y+btm-1)*lbDisplay.GraphicsScreenWidth + left];
        spd->nextRowDelta = -lbDisplay.GraphicsScreenWidth;
        short tmp_btm = btm;
        btm = sprHt - top;
        top = sprHt - tmp_btm;
    } else
    {
        spd->r = &lbDisplay.WScreen[x + (y+top)*lbDisplay.GraphicsScreenWidth + left];
        spd->nextRowDelta = lbDisplay.GraphicsScreenWidth;
    }
    spd->Ht = btm - top;
    spd->Wd = right - left;
    spd->sp = (char *)spr->Data;
    SYNCDBG(19,"Sprite coords X=%d...%d Y=%d...%d data=%08x",left,right,top,btm,spd->sp);
    long htIndex;
    if ( top )
    {
        htIndex = top;
        while ( 1 )
        {
            char chr = *(spd->sp);
            while (chr > 0)
            {
                spd->sp += chr + 1;
                chr = *(spd->sp);
            }
            spd->sp++;
            if (chr == 0)
            {
              htIndex--;
              if (htIndex <= 0) break;
            }
        }
    }
    SYNCDBG(19,"Drawing sprite of size (%d,%d)",(int)spd->Ht,(int)spd->Wd);
    if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
    {
        spd->r += spd->Wd - 1;
        spd->mirror = true;
        short tmpwidth = spr->SWidth;
        short tmpright = right;
        right = tmpwidth - left;
        spd->startShift = tmpwidth - tmpright;
    } else
    {
        spd->mirror = false;
        spd->startShift = left;
    }
    return Lb_SUCCESS;
}

/** Internal function used to skip some of sprite data before drawing is started.
 *
 * @param sp Sprite data buffer pointer.
 * @param r Output buffer pointer.
 * @param x1 Width to be drawn.
 * @param left Width of the area to skip.
 */
static inline short LbSpriteDrawLineSkipLeft(const char **sp, short *x1, short left)
{
    char schr;
    // Cut the left side of the sprite, if needed
    if (left != 0)
    {
        short lpos = left;
        while (lpos > 0)
        {
            schr = *(*sp);
            // Value > 0 means count of filled characters, < 0 means skipped characters
            // Equal to 0 means EOL
            if (schr == 0)
            {
              (*x1) = 0;
              break;
            }
            if (schr < 0)
            {
                if (-schr <= lpos)
                {
                    lpos += schr;
                    (*sp)++;
                } else
                // If we have more empty spaces than we want to skip
                {
                    // Return remaining part to skip, so that we can do it outside
                    return lpos;
                }
            } else
            //if (schr > 0)
            {
                if (schr <= lpos)
                // If we have less than we want to skip
                {
                    lpos -= schr;
                    (*sp) += (*(*sp)) + 1;
                } else
                // If we have more characters than we want to skip
                {
                    // Return remaining part to skip, so that we can draw it
                    return lpos;
                }
            }
        }
    }
    return 0;
}

/** Internal function used to skip to next line after drawing a requested area.
 *
 * @param sp Sprite data buffer pointer.
 * @param x1 Width difference after draw.
 */
static inline void LbSpriteDrawLineSkipToEol(const char **sp, short *x1)
{
    char schr;
    if ((*x1) <= 0)
    {
      do {
        schr = *(*sp);
        while (schr > 0)
        {
          (*sp) += schr+1;
          schr = *(*sp);
        }
        (*sp)++;
      } while (schr);
    } else
    {
        (*sp)++;
    }
}

/** Internal function used to draw part of sprite line.
 *
 * @param buf_out
 * @param buf_inp
 * @param buf_len
 * @param mirror
 */
static inline void LbDrawBufferTranspr(unsigned char **buf_out,const char *buf_inp,
        const int buf_len, const TbBool mirror)
{
  int i;
  unsigned int val;
  if ( mirror )
  {
    if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
    {
        for (i=0; i<buf_len; i++ )
        {
            val = *(const unsigned char *)buf_inp;
            **buf_out = lbDisplay.GlassMap[(val<<8) + **buf_out];
            buf_inp++;
            (*buf_out)--;
        }
    } else
    {
        for (i=0; i<buf_len; i++ )
        {
            val = *(const unsigned char *)buf_inp;
            **buf_out = lbDisplay.GlassMap[((**buf_out)<<8) + val];
            buf_inp++;
            (*buf_out)--;
        }
    }
  } else
  {
    if ( lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4 )
    {
        for (i=0; i<buf_len; i++ )
        {
            val = *(const unsigned char *)buf_inp;
            **buf_out = lbDisplay.GlassMap[(val<<8) + **buf_out];
            buf_inp++;
            (*buf_out)++;
        }
    } else
    {
        for (i=0; i<buf_len; i++ )
        {
            val = *(const unsigned char *)buf_inp;
            **buf_out = lbDisplay.GlassMap[((**buf_out)<<8) + val];
            buf_inp++;
            (*buf_out)++;
        }
    }
  }
}

/** Internal function used to draw part of sprite line.
 *  Draws by copying pixels from input buffer into output buffer, without any kind of blending
 *  or altering the color values. Palette for both buffers must be identical.
 *
 * @param buf_out
 * @param buf_inp
 * @param buf_len
 * @param mirror
 */
static inline void LbDrawBufferSolid(unsigned char **buf_out,const char *buf_inp,
        const int buf_len, const TbBool mirror)
{
    int i;
    if ( mirror )
    {
        for (i=0; i < buf_len; i++)
        {
            **buf_out = *(const unsigned char *)buf_inp;
            buf_inp++;
            (*buf_out)--;
        }
    } else
    {
        for (i=0; i < buf_len; i++)
        {
            **buf_out = *(const unsigned char *)buf_inp;
            buf_inp++;
            (*buf_out)++;
        }
    }
}

/** Internal function used to draw part of sprite line with single colour.
 *
 * @param buf_scr
 * @param colour
 * @param buf_len
 * @param mirror
 */
static inline void LbDrawBufferOneColour(unsigned char **buf_out,const TbPixel colour,
        const int buf_len, const TbBool mirror)
{
    int i;
    if ( mirror )
    {
        if ( lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4 )
        {
            for (i=0; i<buf_len; i++ )
            {
                **buf_out = lbDisplay.GlassMap[(colour<<8) + **buf_out];
                (*buf_out)--;
            }
        } else
        {
            for (i=0; i<buf_len; i++ )
            {
                **buf_out = lbDisplay.GlassMap[((**buf_out)<<8) + colour];
                (*buf_out)--;
            }
        }
    } else
    {
        if ( lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4 )
        {
            for (i=0; i<buf_len; i++ )
            {
                **buf_out = lbDisplay.GlassMap[(colour<<8) + **buf_out];
                (*buf_out)++;
            }
        } else
        {
            for (i=0; i<buf_len; i++ )
            {
                **buf_out = lbDisplay.GlassMap[((**buf_out)<<8) + colour];
                (*buf_out)++;
            }
        }
    }
}

/** Internal function used to draw part of sprite line with single colour.
 *
 * @param buf_out
 * @param colour
 * @param buf_len
 */
static inline void LbDrawBufferOneColorSolid(unsigned char **buf_out,const TbPixel colour,
        const int buf_len, const TbBool mirror)
{
    int i;
    if ( mirror )
    {
        for (i=0; i < buf_len; i++)
        {
            **buf_out = colour;
            (*buf_out)--;
        }
    } else
    {
        for (i=0; i < buf_len; i++)
        {
            **buf_out = colour;
            (*buf_out)++;
        }
    }
}

/** Internal function used to draw part of sprite line.
 *
 * @param buf_out
 * @param buf_inp
 * @param buf_len
 * @param mirror
 */
static inline void LbDrawBufferTrRemap(unsigned char **buf_out,const char *buf_inp,
        const int buf_len, const unsigned char *cmap, const TbBool mirror)
{
  int i;
  unsigned int val;
  if ( mirror )
  {
    if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
    {
        for (i=0; i<buf_len; i++ )
        {
            val = cmap[*(const unsigned char *)buf_inp];
            **buf_out = lbDisplay.GlassMap[(val<<8) + **buf_out];
            buf_inp++;
            (*buf_out)--;
        }
    } else
    {
        for (i=0; i<buf_len; i++ )
        {
            val = cmap[*(const unsigned char *)buf_inp];
            **buf_out = lbDisplay.GlassMap[((**buf_out)<<8) + val];
            buf_inp++;
            (*buf_out)--;
        }
    }
  } else
  {
    if ( lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4 )
    {
        for (i=0; i<buf_len; i++ )
        {
            val = cmap[*(const unsigned char *)buf_inp];
            **buf_out = lbDisplay.GlassMap[(val<<8) + **buf_out];
            buf_inp++;
            (*buf_out)++;
        }
    } else
    {
        for (i=0; i<buf_len; i++ )
        {
            val = cmap[*(const unsigned char *)buf_inp];
            **buf_out = lbDisplay.GlassMap[((**buf_out)<<8) + val];
            buf_inp++;
            (*buf_out)++;
        }
    }
  }
}

/** Internal function used to draw part of sprite line.
 *
 * @param buf_out
 * @param buf_inp
 * @param buf_len
 * @param mirror
 */
static inline void LbDrawBufferSlRemap(unsigned char **buf_out,const char *buf_inp,
        const int buf_len, const unsigned char *cmap, const TbBool mirror)
{
    int i;
    for (i=0; i < buf_len; i++)
    {
        **buf_out = cmap[*(const unsigned char *)buf_inp];
        buf_inp++;
        (*buf_out)--;
    }
}

/** Internal function used to draw part of sprite line.
 *
 * @param buf_out
 * @param buf_inp
 * @param buf_len
 * @param mirror
 */
static inline void LbDrawBufferFCRemap(unsigned char **buf_out,const char *buf_inp,
        const int buf_len, const unsigned char *cmap)
{
    int i;
    for (i=0; i < buf_len; i++)
    {
        **buf_out = cmap[*(const unsigned char *)buf_inp];
        buf_inp++;
        (*buf_out)++;
    }
}

/** Internal routine to draw one line of a transparent sprite.
 *
 * @param sp
 * @param r
 * @param x1
 * @param lpos
 * @param mirror
 */
static inline void LbSpriteDrawLineTranspr(const char **sp, unsigned char **r, short *x1,
    short lpos, const TbBool mirror)
{
    char schr;
    unsigned char drawOut;
    // Draw any unfinished block, which should be only partially visible
    if (lpos > 0)
    {
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            if ( mirror )
                (*r) -= drawOut;
            else
                (*r) += drawOut;
            (*sp)++;

        } else
        {
            // Draw the part of current block which exceeds value of 'lpos'
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferTranspr(r,(*sp)+(lpos+1),drawOut,mirror);
            // Update positions and break the skipping loop
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            if ( mirror )
               (*r) += *(*sp);
            else
               (*r) -= *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            LbDrawBufferTranspr(r,(*sp)+1,drawOut,mirror);
            (*x1) -= schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

static inline TbResult LbSpriteDrawTranspr(const char *sp,short sprWd,short sprHt,unsigned char *r,
    int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineTranspr(&sp,&r,&x1,lpos,mirror);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

/** Internal routine to draw one line of a solid sprite.
 *  Supports only mirrored sprites.
 *
 * @param sp
 * @param r
 * @param x1
 * @param lpos
 * @param mirror
 */
static inline void LbSpriteDrawLineSolid(const char **sp, unsigned char **r, short *x1, short lpos, const TbBool mirror)
{
    char schr;
    unsigned char drawOut;
    // Draw any unfinished block, which should be only partially visible
    if (lpos > 0)
    {
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            (*r) -= drawOut;
            (*sp)++;
        } else
        {
            // Draw the part of current block which exceeds value of 'lpos'
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferSolid(r,(*sp)+(lpos+1),drawOut,mirror);
            // Update positions and break the skipping loop
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            (*r) += *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            LbDrawBufferSolid(r,(*sp)+1,drawOut,mirror);
            (*x1) -= schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

/** Solid sprite drawing routine. Optimized for mirrored ones, without transparency.
 *
 * @param sp
 * @param sprWd
 * @param sprHt
 * @param r
 * @param nextRowDelta
 * @param left
 * @param mirror
 * @return
 */
static inline TbResult LbSpriteDrawSolid(const char *sp,short sprWd,short sprHt,unsigned char *r,
    int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineSolid(&sp,&r,&x1,lpos,mirror);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

static inline void LbSpriteDrawLineFastCpy(const char **sp, unsigned char **r, short *x1, short lpos)
{
    char schr;
    unsigned char drawOut;
    if (lpos > 0)
    {
        // Draw the part of current block which exceeds value of 'lpos'
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            (*r) += drawOut;
            (*sp)++;
        } else
        {
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferSolid(r, (*sp)+(lpos+1), drawOut, false);
            (*r) += drawOut;
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            (*r) -= *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            memcpy((*r), (*sp)+1, drawOut);
            (*x1) -= schr;
            (*r) += schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

/** Fast copy sprite drawing routine. Does not support transparency nor mirroring.
 *
 * @param sp
 * @param sprWd
 * @param sprHt
 * @param r
 * @param nextRowDelta
 * @param left
 * @param mirror
 * @return
 */
static inline TbResult LbSpriteDrawFastCpy(const char *sp,short sprWd,short sprHt,unsigned char *r,
    int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineFastCpy(&sp,&r,&x1,lpos);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

TbResult LbSpriteDraw(long x, long y, const struct TbSprite *spr)
{
    struct TbSpriteDrawData spd;
    TbResult ret;
    SYNCDBG(19,"At (%ld,%ld)",x,y);
    ret = LbSpriteDrawPrepare(&spd, x, y, spr);
    if (ret != Lb_SUCCESS)
        return ret;
    if ((lbDisplay.DrawFlags & (Lb_SPRITE_TRANSPAR4|Lb_SPRITE_TRANSPAR8)) != 0)
        return LbSpriteDrawTranspr(spd.sp,spd.Wd,spd.Ht,spd.r,spd.nextRowDelta,spd.startShift,spd.mirror);
    else
    if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
        return LbSpriteDrawSolid(spd.sp,spd.Wd,spd.Ht,spd.r,spd.nextRowDelta,spd.startShift,spd.mirror);
    else
        return LbSpriteDrawFastCpy(spd.sp,spd.Wd,spd.Ht,spd.r,spd.nextRowDelta,spd.startShift,spd.mirror);
}

/** Internal routine to draw one line of a remapped transparent sprite.
 *
 * @param sp
 * @param r
 * @param x1
 * @param lpos
 * @param mirror
 */
static inline void LbSpriteDrawLineTrRemap(const char **sp, unsigned char **r, short *x1,
    const unsigned char *cmap, short lpos,const TbBool mirror)
{
    char schr;
    unsigned char drawOut;
    // Draw any unfinished block, which should be only partially visible
    if (lpos > 0)
    {
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            if ( mirror )
                (*r) -= drawOut;
            else
                (*r) += drawOut;
            (*sp)++;

        } else
        {
            // Draw the part of current block which exceeds value of 'lpos'
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferTrRemap(r,(*sp)+(lpos+1),drawOut,cmap,mirror);
            // Update positions and break the skipping loop
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            if ( mirror )
               (*r) += *(*sp);
            else
               (*r) -= *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            LbDrawBufferTrRemap(r,(*sp)+1,drawOut,cmap,mirror);
            (*x1) -= schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

static inline TbResult LbSpriteDrawTrRemap(const char *sp,short sprWd,short sprHt,
        unsigned char *r,const unsigned char *cmap,int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineTrRemap(&sp,&r,&x1,cmap,lpos,mirror);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

static inline void LbSpriteDrawLineSlRemap(const char **sp, unsigned char **r, short *x1,
    const unsigned char *cmap, short lpos,const TbBool mirror)
{
    char schr;
    unsigned char drawOut;
    // Draw any unfinished block, which should be only partially visible
    if (lpos > 0)
    {
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            (*r) -= drawOut;
            (*sp)++;
        } else
        {
            // Draw the part of current block which exceeds value of 'lpos'
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferSlRemap(r,(*sp)+(lpos+1),drawOut,cmap,mirror);
            // Update positions and break the skipping loop
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            (*r) += *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            LbDrawBufferSlRemap(r,(*sp)+1,drawOut,cmap,mirror);
            (*x1) -= schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

static inline TbResult LbSpriteDrawSlRemap(const char *sp,short sprWd,short sprHt,
        unsigned char *r,const unsigned char *cmap,int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineSlRemap(&sp,&r,&x1,cmap,lpos,mirror);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

static inline void LbSpriteDrawLineFCRemap(const char **sp, unsigned char **r, short *x1,
    const unsigned char *cmap, short lpos,const TbBool mirror)
{
    char schr;
    unsigned char drawOut;
    // Draw any unfinished block, which should be only partially visible
    if (lpos > 0)
    {
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            if ( mirror )
                (*r) -= drawOut;
            else
                (*r) += drawOut;
            (*sp)++;

        } else
        {
            // Draw the part of current block which exceeds value of 'lpos'
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferFCRemap(r,(*sp)+(lpos+1),drawOut,cmap);
            // Update positions and break the skipping loop
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            if ( mirror )
               (*r) += *(*sp);
            else
               (*r) -= *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            LbDrawBufferFCRemap(r,(*sp)+1,drawOut,cmap);
            (*x1) -= schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

/** Fast copy sprite drawing routine with colour remap. Does not support transparency nor mirroring.
 *
 * @param sp
 * @param sprWd
 * @param sprHt
 * @param r
 * @param nextRowDelta
 * @param left
 * @param mirror
 * @return
 */
static inline TbResult LbSpriteDrawFCRemap(const char *sp,short sprWd,short sprHt,unsigned char *r,
    const unsigned char *cmap,int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineFCRemap(&sp,&r,&x1,cmap,lpos,mirror);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

int LbSpriteDrawRemap(long x, long y, const struct TbSprite *spr,const unsigned char *cmap)
{
    struct TbSpriteDrawData spd;
    TbResult ret;
    SYNCDBG(19,"At (%ld,%ld)",x,y);
    ret = LbSpriteDrawPrepare(&spd, x, y, spr);
    if (ret != Lb_SUCCESS)
        return ret;
    if ((lbDisplay.DrawFlags & (Lb_SPRITE_TRANSPAR4|Lb_SPRITE_TRANSPAR8)) != 0) {
        return LbSpriteDrawTrRemap(spd.sp,spd.Wd,spd.Ht,spd.r,cmap,spd.nextRowDelta,spd.startShift,spd.mirror);
    } else
    if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0) {
        return LbSpriteDrawSlRemap(spd.sp,spd.Wd,spd.Ht,spd.r,cmap,spd.nextRowDelta,spd.startShift,spd.mirror);
    } else
    {
        return LbSpriteDrawFCRemap(spd.sp,spd.Wd,spd.Ht,spd.r,cmap,spd.nextRowDelta,spd.startShift,spd.mirror);
    }
}

/** Internal routine to draw one line of a transparent sprite.
 *
 * @param sp
 * @param r
 * @param x1
 * @param lpos
 * @param mirror
 */
static inline void LbSpriteDrawLineTrOneColour(const char **sp, unsigned char **r, short *x1,
    TbPixel colour, short lpos,const TbBool mirror)
{
    char schr;
    unsigned char drawOut;
    // Draw any unfinished block, which should be only partially visible
    if (lpos > 0)
    {
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            if ( mirror )
                (*r) -= drawOut;
            else
                (*r) += drawOut;
            (*sp)++;

        } else
        {
            // Draw the part of current block which exceeds value of 'lpos'
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferOneColour(r,colour,drawOut,mirror);
            // Update positions and break the skipping loop
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            if ( mirror )
               (*r) += *(*sp);
            else
               (*r) -= *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            LbDrawBufferOneColour(r,colour,drawOut,mirror);
            (*x1) -= schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

static inline TbResult LbSpriteDrawTrOneColour(const char *sp,short sprWd,short sprHt,
        unsigned char *r,TbPixel colour,int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineTrOneColour(&sp,&r,&x1,colour,lpos,mirror);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

static inline void LbSpriteDrawLineSlOneColour(const char **sp, unsigned char **r, short *x1,
    TbPixel colour, short lpos,const TbBool mirror)
{
    char schr;
    unsigned char drawOut;
    // Draw any unfinished block, which should be only partially visible
    if (lpos > 0)
    {
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            (*r) -= drawOut;
            (*sp)++;
        } else
        {
            // Draw the part of current block which exceeds value of 'lpos'
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferOneColorSolid(r,colour,drawOut,mirror);
            // Update positions and break the skipping loop
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            (*r) += *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            LbDrawBufferOneColorSolid(r,colour,drawOut,mirror);
            (*x1) -= schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

static inline TbResult LbSpriteDrawSlOneColour(const char *sp,short sprWd,short sprHt,
        unsigned char *r,TbPixel colour,int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineSlOneColour(&sp,&r,&x1,colour,lpos,mirror);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

static inline void LbSpriteDrawLineFCOneColour(const char **sp, unsigned char **r, short *x1, TbPixel colour, short lpos)
{
    char schr;
    unsigned char drawOut;
    if (lpos > 0)
    {
        // Draw the part of current block which exceeds value of 'lpos'
        schr = *(*sp);
        if (schr < 0)
        {
            drawOut = -schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            (*r) += drawOut;
            (*sp)++;
        } else
        {
            drawOut = schr - lpos;
            if (drawOut > (*x1))
              drawOut = (*x1);
            LbDrawBufferOneColorSolid(r, colour, drawOut, false);
            (*r) += drawOut;
            (*sp) += (*(*sp)) + 1;
        }
        (*x1) -= drawOut;
    }
    // Draw the visible part of a sprite
    while ((*x1) > 0)
    {
        schr = *(*sp);
        if (schr == 0)
        { // EOL, breaking line loop
            break;
        }
        if (schr < 0)
        { // Skipping some pixels
            (*x1) += schr;
            (*r) -= *(*sp);
            (*sp)++;
        } else
        //if ( schr > 0 )
        { // Drawing some pixels
            drawOut = schr;
            if (drawOut >= (*x1))
                drawOut = (*x1);
            memset((*r), colour, drawOut);
            (*x1) -= schr;
            (*r) += schr;
            (*sp) += (*(*sp)) + 1;
        }
    } //end while
}

/** Fast copy one color sprite drawing routine. Does not support transparency nor mirroring.
 *
 * @param sp
 * @param sprWd
 * @param sprHt
 * @param r
 * @param nextRowDelta
 * @param left
 * @param mirror
 * @return
 */
static inline TbResult LbSpriteDrawFCOneColour(const char *sp,short sprWd,short sprHt,unsigned char *r,
    TbPixel colour,int nextRowDelta,short left,const TbBool mirror)
{
    unsigned char *nextRow;
    long htIndex;
    nextRow = &(r[nextRowDelta]);
    htIndex = sprHt;
    // For all lines of the sprite
    while (1)
    {
        short x1;
        short lpos;
        x1 = sprWd;
        // Skip the pixels left before drawing area
        lpos = LbSpriteDrawLineSkipLeft(&sp,&x1,left);
        // Do the actual drawing
        LbSpriteDrawLineFCOneColour(&sp,&r,&x1,colour,lpos);
        // Go to next line
        htIndex--;
        if (htIndex == 0)
            return Lb_SUCCESS;
        LbSpriteDrawLineSkipToEol(&sp,&x1);
        r = nextRow;
        nextRow += nextRowDelta;
    } //end while
    return Lb_SUCCESS;
}

TbResult LbSpriteDrawOneColour(long x, long y, const struct TbSprite *spr, const TbPixel colour)
{
    struct TbSpriteDrawData spd;
    TbResult ret;
    SYNCDBG(19,"At (%ld,%ld)",x,y);
    ret = LbSpriteDrawPrepare(&spd, x, y, spr);
    if (ret != Lb_SUCCESS)
        return ret;
    if ((lbDisplay.DrawFlags & (Lb_SPRITE_TRANSPAR4|Lb_SPRITE_TRANSPAR8)) != 0) {
        return LbSpriteDrawTrOneColour(spd.sp,spd.Wd,spd.Ht,spd.r,colour,spd.nextRowDelta,spd.startShift,spd.mirror);
    } else
    if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0) {
        return LbSpriteDrawSlOneColour(spd.sp,spd.Wd,spd.Ht,spd.r,colour,spd.nextRowDelta,spd.startShift,spd.mirror);
    } else {
        return LbSpriteDrawFCOneColour(spd.sp,spd.Wd,spd.Ht,spd.r,colour,spd.nextRowDelta,spd.startShift,spd.mirror);
    }
}

void LbPixelBlockCopyForward(TbPixel * dst, const TbPixel * src, long len)
{
    TbPixel px;
    unsigned long pxquad;
    if ( !((int)dst & 3) || ((px = *src, ++src, *dst = px, ++dst, --len, len)
     && (!((int)dst & 3) || ((px = *src, ++src, *dst = px, ++dst, --len, len)
     && (!((int)dst & 3) ||  (px = *src, ++src, *dst = px, ++dst, --len, len))))) )
    {
        long l;
        for ( l = len>>2; l > 0; l--)
        {
            pxquad = *(unsigned long *)src;
            src += sizeof(unsigned long);
            *(unsigned long *)dst = pxquad;
            dst += sizeof(unsigned long);
        }
        if (len & 3)
        {
          *dst = *src;
          if ((len & 3) != 1)
          {
            *(dst + 1) = *(src + 1);
            if ((len & 3) != 2)
              *(dst + 2) = *(src + 2);
          }
        }
    }
}

/**
 * Sets X scaling array for drawing scaled sprites.
 * The X scaling array contains position and length of each pixel of the sprite on destination buffer.
 * @param xsteps_arr The destination X scaling array.
 * @param x Position of the sprite in output buffer, X coord.
 * @param swidth Source sprite original width.
 * @param dwidth Width which the sprite should have on destination buffer.
 * @param gwidth Graphics buffer visible window line width.
 */
void LbSpriteSetScalingWidthClippedArray(long * xsteps_arr, long x, long swidth, long dwidth, long gwidth)
{
    long *pwidth;
    long pxpos;
    pwidth = xsteps_arr;
    long factor = (dwidth<<16)/swidth;
    long tmp = (factor >> 1) + (x << 16);
    pxpos = tmp >> 16;
    long w = swidth;
    do {
        tmp += factor;
        long pxstart;
        long pxend;
        pxstart = pxpos;
        pxend = tmp>>16;
        // Remember unclipped difference
        long wdiff = pxend - pxstart;
        // Now clip to graphics line bounds
        if (pxstart < 0) {
            pxstart = 0;
            pxend = pxstart;
        } else
        if (pxstart >= gwidth) {
            pxstart = gwidth-1;
            pxend = pxstart;
        } else
        if (pxend < 0) {
            pxend = 0;
        } else
        if (pxend > gwidth) {
            pxend = gwidth;
        }
        // Set clipped difference to be drawn
        pwidth[0] = pxstart;
        pwidth[1] = pxend - pxstart;
        // But store position for the unclipped difference
        pxpos += wdiff;
        w--;
        pwidth += 2;
    } while (w > 0);
}

void LbSpriteSetScalingWidthClipped(long x, long swidth, long dwidth, long gwidth)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)swidth,(int)dwidth,(int)x);
    if (swidth > SPRITE_SCALING_XSTEPS)
        swidth = SPRITE_SCALING_XSTEPS;
    LbSpriteSetScalingWidthClippedArray(xsteps_array, x, swidth, dwidth, gwidth);
}

void LbSpriteSetAlphaScalingWidthClipped(long x, long swidth, long dwidth, long gwidth)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)swidth,(int)dwidth,(int)x);
    if (swidth > SPRITE_SCALING_XSTEPS)
        swidth = SPRITE_SCALING_XSTEPS;
    LbSpriteSetScalingWidthClippedArray(alpha_xsteps_array, x, swidth, dwidth, gwidth);
}

void LbSpriteSetScalingWidthSimpleArray(long * xsteps_arr, long x, long swidth, long dwidth)
{
    long *pwidth;
    long cwidth;
    pwidth = xsteps_arr;
    long factor = (dwidth<<16)/swidth;
    long tmp = (factor >> 1) + (x << 16);
    cwidth = tmp >> 16;
    long w = swidth;
    do {
      int i;
      for (i=0; i < 16; i+=2)
      {
          pwidth[i] = cwidth;
          tmp += factor;
          pwidth[i+1] = (tmp>>16) - cwidth;
          cwidth = (tmp>>16);
          w--;
          if (w <= 0)
              break;
      }
      pwidth += 16;
    } while (w > 0);
}

void LbSpriteSetScalingWidthSimple(long x, long swidth, long dwidth)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)swidth,(int)dwidth,(int)x);
    if (swidth > SPRITE_SCALING_XSTEPS)
        swidth = SPRITE_SCALING_XSTEPS;
    LbSpriteSetScalingWidthSimpleArray(xsteps_array, x, swidth, dwidth);
}

void LbSpriteSetAlphaScalingWidthSimple(long x, long swidth, long dwidth)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)swidth,(int)dwidth,(int)x);
    if (swidth > SPRITE_SCALING_XSTEPS)
        swidth = SPRITE_SCALING_XSTEPS;
    LbSpriteSetScalingWidthSimpleArray(alpha_xsteps_array, x, swidth, dwidth);
}

void LbSpriteClearScalingWidthArray(long * xsteps_arr, long swidth)
{
    int i;
    long *pwidth;
    pwidth = xsteps_arr;
    for (i=0; i < swidth; i++)
    {
        pwidth[0] = 0;
        pwidth[1] = 0;
        pwidth += 2;
    }
}

void LbSpriteClearScalingWidth(void)
{
    LbSpriteClearScalingWidthArray(xsteps_array, SPRITE_SCALING_XSTEPS);
}

void LbSpriteClearAlphaScalingWidth(void)
{
    LbSpriteClearScalingWidthArray(alpha_xsteps_array, SPRITE_SCALING_XSTEPS);
}

/**
 * Sets Y scaling array for drawing scaled sprites.
 * The Y scaling array contains position and length of each line of pixels of the sprite on destination buffer.
 * @param ysteps_arr The destination X scaling array.
 * @param y Position of the sprite in output buffer, Y coord.
 * @param sheight Source sprite original height.
 * @param dheight Height which the sprite should have on destination buffer.
 * @param gheight Graphics buffer visible window lines count.
 */
void LbSpriteSetScalingHeightClippedArray(long * ysteps_arr, long y, long sheight, long dheight, long gheight)
{
    long *pheight;
    long lnpos;
    pheight = ysteps_arr;
    long factor = (dheight<<16)/sheight;
    long tmp = (factor >> 1) + (y << 16);
    lnpos = tmp >> 16;
    if (lnpos < 0)
        lnpos = 0;
    if (lnpos >= gheight)
        lnpos = gheight;
    long h = sheight;
    do {
        tmp += factor;
        long lnstart;
        long lnend;
        lnstart = lnpos;
        lnend = tmp>>16;
        // Remember unclipped difference
        long hdiff = lnend - lnstart;
        // Now clip to graphics line bounds
        if (lnstart < 0) {
            lnstart = 0;
            lnend = lnstart;
        } else
        if (lnstart >= gheight) {
            lnstart = gheight-1;
            lnend = lnstart;
        } else
        if (lnend < 0) {
            lnend = 0;
        } else
        if (lnend > gheight) {
            lnend = gheight;
        }
        // Set clipped difference to be drawn
        pheight[0] = lnstart;
        pheight[1] = lnend - lnstart;
        // But store position for the unclipped difference
        lnpos += hdiff;
        h--;
        pheight += 2;
    } while (h > 0);
}

void LbSpriteSetScalingHeightClipped(long y, long sheight, long dheight, long gheight)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)sheight,(int)dheight,(int)y);
    if (sheight > SPRITE_SCALING_YSTEPS)
        sheight = SPRITE_SCALING_YSTEPS;
    LbSpriteSetScalingHeightClippedArray(ysteps_array, y, sheight, dheight, gheight);
}

void LbSpriteSetAlphaScalingHeightClipped(long y, long sheight, long dheight, long gheight)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)sheight,(int)dheight,(int)y);
    if (sheight > SPRITE_SCALING_YSTEPS)
        sheight = SPRITE_SCALING_YSTEPS;
    LbSpriteSetScalingHeightClippedArray(alpha_ysteps_array, y, sheight, dheight, gheight);
}

void LbSpriteSetScalingHeightSimpleArray(long * ysteps_arr, long y, long sheight, long dheight)
{
    long *pheight;
    long cheight;
    pheight = ysteps_arr;
    long factor = (dheight<<16)/sheight;
    long tmp = (factor >> 1) + (y << 16);
    cheight = tmp >> 16;
    long h = sheight;
    do {
      int i=0;
      for (i=0; i < 16; i+=2)
      {
        pheight[i] = cheight;
        tmp += factor;
        pheight[i+1] = (tmp>>16) - cheight;
        cheight = (tmp>>16);
        h--;
        if (h <= 0)
          break;
      }
      pheight += 16;
    } while (h > 0);
}

void LbSpriteSetScalingHeightSimple(long y, long sheight, long dheight)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)sheight,(int)dheight,(int)y);
    if (sheight > SPRITE_SCALING_YSTEPS)
        sheight = SPRITE_SCALING_YSTEPS;
    LbSpriteSetScalingHeightSimpleArray(ysteps_array, y, sheight, dheight);
}

void LbSpriteSetAlphaScalingHeightSimple(long y, long sheight, long dheight)
{
    SYNCDBG(17,"Starting %d -> %d at %d",(int)sheight,(int)dheight,(int)y);
    if (sheight > SPRITE_SCALING_YSTEPS)
        sheight = SPRITE_SCALING_YSTEPS;
    LbSpriteSetScalingHeightSimpleArray(alpha_ysteps_array, y, sheight, dheight);
}

void LbSpriteClearScalingHeightArray(long * ysteps_arr, long sheight)
{
    int i;
    long *pheight;
    pheight = ysteps_arr;
    for (i=0; i < sheight; i++)
    {
        pheight[0] = 0;
        pheight[1] = 0;
        pheight += 2;
    }
}

void LbSpriteClearScalingHeight(void)
{
    LbSpriteClearScalingHeightArray(ysteps_array, SPRITE_SCALING_YSTEPS);
}

void LbSpriteClearAlphaScalingHeight(void)
{
    LbSpriteClearScalingHeightArray(alpha_ysteps_array, SPRITE_SCALING_YSTEPS);
}

/**
 * Sets scaling data for drawing scaled sprites.
 * @param x Position of the sprite in output buffer, X coord.
 * @param y Position of the sprite in output buffer, Y coord.
 * @param swidth Source sprite original width.
 * @param sheight Source sprite original height.
 * @param dwidth Width which the sprite should have on destination buffer.
 * @param dheight Height which the sprite should have on destination buffer.
 */
void LbSpriteSetScalingData(long x, long y, long swidth, long sheight, long dwidth, long dheight)
{
    long gwidth = lbDisplay.GraphicsWindowWidth;
    long gheight = lbDisplay.GraphicsWindowHeight;
    scale_up = true;
    if ((dwidth <= swidth) && (dheight <= sheight))
        scale_up = false;
    // Checking whether to select simple scaling creation, or more comprehensive one - with clipping
    if ((swidth <= 0) || (dwidth <= 0)) {
        WARNLOG("Tried scaling width %ld -> %ld", swidth, dwidth);
        LbSpriteClearScalingWidth();
    } else
    // Normally it would be enough to check if ((dwidth+y) >= gwidth), but due to rounding we need to add swidth
    if ((x < 0) || ((dwidth+swidth+x) >= gwidth))
    {
        LbSpriteSetScalingWidthClipped(x, swidth, dwidth, gwidth);
    } else {
        LbSpriteSetScalingWidthSimple(x, swidth, dwidth);
    }
    if ((sheight <= 0) || (dheight <= 0)) {
        WARNLOG("Tried scaling height %ld -> %ld", sheight, dheight);
        LbSpriteClearScalingHeight();
    } else
    // Normally it would be enough to check if ((dheight+y) >= gheight), but our simple rounding may enlarge the image
    if ((y < 0) || ((dheight+sheight+y) >= gheight))
    {
        LbSpriteSetScalingHeightClipped(y, sheight, dheight, gheight);
    } else {
        LbSpriteSetScalingHeightSimple(y, sheight, dheight);
    }
}

void SetAlphaScalingData(long x, long y, long swidth, long sheight, long dwidth, long dheight)
{
    long gwidth = lbDisplay.GraphicsWindowWidth;
    long gheight = lbDisplay.GraphicsWindowHeight;
    alpha_scale_up = true;
    if ((dwidth <= swidth) && (dheight <= sheight))
        alpha_scale_up = false;
    if ((swidth <= 0) || (dwidth <= 0)) {
        WARNLOG("Tried scaling width %ld -> %ld", swidth, dwidth);
        LbSpriteClearAlphaScalingWidth();
    } else
    if ((x < 0) || ((dwidth+x) >= gwidth))
    {
        LbSpriteSetAlphaScalingWidthClipped(x, swidth, dwidth, gwidth);
    } else {
        LbSpriteSetAlphaScalingWidthSimple(x, swidth, dwidth);
    }
    if ((sheight <= 0) || (dheight <= 0)) {
        WARNLOG("Tried scaling height %ld -> %ld", sheight, dheight);
        LbSpriteClearAlphaScalingHeight();
    } else
    if ((y < 0) || ((dheight+y) >= gheight))
    {
        LbSpriteSetAlphaScalingHeightClipped(y, sheight, dheight, gheight);
    } else {
        LbSpriteSetAlphaScalingHeightSimple(y, sheight, dheight);
    }
}

TbResult LbSpriteDrawScaled(long xpos, long ypos, const struct TbSprite *sprite, long dest_width, long dest_height)
{
    SYNCDBG(19,"At (%ld,%ld) size (%ld,%ld)",xpos,ypos,dest_width,dest_height);
    if ((dest_width <= 0) || (dest_height <= 0))
      return 1;
    if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLNSHADOW) != 0)
        lbSpriteReMapPtr = lbDisplay.FadeTable + ((lbDisplay.FadeStep & 0x3F) << 8);
    LbSpriteSetScalingData(xpos, ypos, sprite->SWidth, sprite->SHeight, dest_width, dest_height);
    return LbSpriteDrawUsingScalingData(0, 0, sprite);
}

TbResult LbSpriteDrawScaledOneColour(long xpos, long ypos, const struct TbSprite *sprite, long dest_width, long dest_height, const TbPixel colour)
{
    SYNCDBG(19,"At (%ld,%ld) size (%ld,%ld)",xpos,ypos,dest_width,dest_height);
    if ((dest_width <= 0) || (dest_height <= 0))
      return 1;
    if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLNSHADOW) != 0)
        lbSpriteReMapPtr = lbDisplay.FadeTable + ((lbDisplay.FadeStep & 0x3F) << 8);
    LbSpriteSetScalingData(xpos, ypos, sprite->SWidth, sprite->SHeight, dest_width, dest_height);
    return LbSpriteDrawOneColourUsingScalingData(0, 0, sprite, colour);
}

int LbSpriteDrawScaledRemap(long xpos, long ypos, const struct TbSprite *sprite, long dest_width, long dest_height, const unsigned char *cmap)
{
    SYNCDBG(19,"At (%ld,%ld) size (%ld,%ld)",xpos,ypos,dest_width,dest_height);
    if ((dest_width <= 0) || (dest_height <= 0))
      return 1;
    if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLNSHADOW) != 0)
        lbSpriteReMapPtr = lbDisplay.FadeTable + ((lbDisplay.FadeStep & 0x3F) << 8);
    LbSpriteSetScalingData(xpos, ypos, sprite->SWidth, sprite->SHeight, dest_width, dest_height);
    return LbSpriteDrawRemapUsingScalingData(0, 0, sprite, cmap);
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

/**
 * Draws a scaled up big sprite on given buffer, with original colours, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @return Gives 0 on success.
 */
TbResult LbHugeSpriteDrawUsingScalingUpData(uchar *outbuf, int scanline, int outheight,
    long *xstep, long *ystep, const struct TbHugeSprite *sprite)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    const unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    ycurstep = ystep;

    int h;
    for (h=0; h < sprite->SHeight; h++)
    {
        if (ycurstep[1] != 0)
        {
            int ycur;
            int solid_len;
            TbPixel * out_line;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            xcurstep = xstep;
            sprdata = &sprite->Data[sprite->Lines[h]];
            TbPixel *out_end;
            out_end = outbuf;
            while (out_end - outbuf < scanline)
            {
                int pxlen;
                pxlen = *(unsigned long *)sprdata;
                sprdata += 4;
                TbPixel *out_start;
                out_start = out_end;
                for(;pxlen > 0; pxlen--)
                {
                    xdup = xcurstep[1];
                    if (xcurstep[0]+xdup > abs(scanline))
                        xdup = abs(scanline)-xcurstep[0];
                    if (xdup > 0)
                    {
                        unsigned char pxval;
                        pxval = *sprdata;
                        for (;xdup > 0; xdup--)
                        {
                            *out_end = pxval;
                            out_end++;
                        }
                    }
                    sprdata++;
                    xcurstep += 2;
                }
                ycur = ydup - 1;
                if (ycur > 0)
                {
                    solid_len = out_end - out_start;
                    out_line = out_start + scanline;
                    for (;ycur > 0; ycur--)
                    {
                        if (solid_len > 0) {
                            LbPixelBlockCopyForward(out_line, out_start, solid_len);
                        }
                        out_line += scanline;
                    }
                }
                // Transparent bytes count
                pxlen = *(unsigned long *)sprdata;
                sprdata += 4;
                out_end -= xcurstep[0];
                xcurstep += 2 * pxlen;
                // In case we've exceeded sprite width, don't try to access xcurstep[] any more
                if ((xcurstep - xstep)/2 >= sprite->SWidth)
                    break;
                out_end += xcurstep[0];
            }
            outbuf += scanline;
            ycur = ydup - 1;
            for (;ycur > 0; ycur--)
            {
                outbuf += scanline;
            }
        }
        ycurstep += ystep_delta;
    }
    return Lb_SUCCESS;
}

/** Draws a huge sprite, used ie. as frame in land view.
 *  What differs huge sprite from standard one is the index of y line starts, which
 *  speeds up finding a specific line to be drawn.
 * @param spr Sprite data struct.
 * @param sp_len Length of the sprite data.
 * @param r Destination buffer.
 * @param r_row_delta Row interline in the destination buffer.
 * @param r_height Height of the destination buffer.
 * @param xshift Shift of the drawing, X coord.
 * @param yshift Shift of the drawing, Y coord.
 * @return
 */
TbResult LbHugeSpriteDraw(const struct TbHugeSprite * spr, long sp_len,
    unsigned char *r, int r_row_delta, int r_height, short xshift, short yshift, int units_per_px)
{
    LbSpriteSetScalingData(-xshift, -yshift, spr->SWidth, spr->SHeight, spr->SWidth*units_per_px/16, spr->SHeight*units_per_px/16);
    return LbHugeSpriteDrawUsingScalingUpData(r, r_row_delta, r_height, xsteps_array, ysteps_array, spr);
}

/**
 * Draws a tiled sprite, which consists of multiple sprites.
 * @param start_x
 * @param start_y
 * @param units_per_px
 * @param bigspr
 * @param sprite
 * @note originally named DrawBigSprite()
 */
void LbTiledSpriteDraw(long start_x, long start_y, long units_per_px, struct TiledSprite *bigspr, struct TbSprite *sprite)
{
    long x;
    long y;
    int delta_x;
    int delta_y;
    int spnum_x;
    int spnum_y;
    delta_y = 0;
    y = start_y;
    for (spnum_y = 0; spnum_y < bigspr->y_num; spnum_y++)
    {
        unsigned short *spr_idx;
        x = start_x;
        spr_idx = &bigspr->spr_idx[spnum_y][0];
        for (spnum_x = 0; spnum_x < bigspr->x_num; spnum_x++)
        {
            delta_x = sprite[*spr_idx].SWidth * units_per_px / 16;
            delta_y = sprite[*spr_idx].SHeight * units_per_px / 16;
            if (*spr_idx)
            {
                LbSpriteDrawScaled(x, y, &sprite[*spr_idx], delta_x, delta_y);
            } else
            {
                unsigned short *prev_spr_idx;
                prev_spr_idx = (spr_idx - 10);
                signed int spnum_p;
                for (spnum_p = 1; spnum_p <= spnum_y; spnum_p++)
                {
                    if (*prev_spr_idx) {
                        delta_x = sprite[bigspr->spr_idx[(spnum_y - spnum_p)][spnum_x]].SWidth * units_per_px / 16;
                        break;
                    }
                    prev_spr_idx -= 10;
                }
            }
            spr_idx++;
            x += delta_x;
        }
        y += delta_y;
    }
}

int LbTiledSpriteHeight(struct TiledSprite *bigspr, struct TbSprite *sprite)
{
    long height;
    int spnum_y;
    height = 0;
    for (spnum_y = 0; spnum_y < bigspr->y_num; spnum_y++)
    {
        unsigned short *spr_idx;
        spr_idx = &bigspr->spr_idx[spnum_y][0];
        height += sprite[*spr_idx].SHeight;
    }
    return height;
}

void LbDrawPixel(long x, long y, TbPixel colour)
{
    lbDisplay.GraphicsWindowPtr[x + lbDisplay.GraphicsScreenWidth * y] = colour;
}

void LbDrawPixelClip(long x, long y, TbPixel colour)
{
    if ( (x < 0) || (x >= lbDisplay.GraphicsWindowWidth) )
        return;
    if ( (y < 0) || (y >= lbDisplay.GraphicsWindowHeight) )
        return;
    TbPixel *buf;
    int val;
    buf = lbDisplay.GraphicsWindowPtr + lbDisplay.GraphicsScreenWidth * y + x;
    val = 0;
    if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
    {
        val = (colour << 8) + (*buf);
        *buf = lbDisplay.GlassMap[val];
    } else
    if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8) != 0)
    {
        val = ((*buf) << 8) + colour;
        *buf = lbDisplay.GlassMap[val];
    } else
    {
        *buf = colour;
    }
}

void LbDrawCircleFilled(long x, long y, long radius, TbPixel colour)
{
    long r;
    long i;
    long n;
    long dx;
    long dy;
    if (radius < 1)
    {
        LbDrawPixelClip(x, y, colour);
        return;
    }
    if (radius == 1)
    {
        LbDrawPixelClip(x - 1, y, colour);
        LbDrawPixelClip(x, y - 1, colour);
        LbDrawPixelClip(x + 1, y, colour);
        LbDrawPixelClip(x, y + 1, colour);
        LbDrawPixelClip(x, y, colour);
        return;
    }
    n = 3 - 2 * radius;
    LbDrawHVLine(x - radius, y, radius + x, y, colour);
    if (n >= 0)
    {
        LbDrawHVLine(x, y - radius, x, y - radius, colour);
        LbDrawHVLine(x, radius + y, x, radius + y, colour);
        r = radius - 1;
        n += 10 - (4 * (radius - 1) + 4);
    } else
    {
        r = radius;
        n += 10 - 4;
    }
    dx = 1;
    dy = 1;
    while (dx < r)
    {
        LbDrawHVLine(x - r, y - dx, x + r, y - dx, colour);
        LbDrawHVLine(x - r, dx + y, x + r, dx + y, colour);
        if (n >= 0)
        {
            LbDrawHVLine(x - dy, y - r, x + dy, y - r, colour);
            LbDrawHVLine(x - dy, r + y, x + dy, r + y, colour);
            i = dx - r;
            r--;
            n += 4 * i + 10;
        } else
        {
            n += 4 * dx + 6;
        }
        dx++;
        dy = dx;
    }
    if (r == dx)
    {
        LbDrawHVLine(x - r, y - dx, x + r, y - dx, colour);
        LbDrawHVLine(x - r, dx + y, x + r, dx + y, colour);
    }
}

static inline void LbDrawPixelClipOpaq1(long x, long y, TbPixel colour)
{
    if ( (x < 0) || (x >= lbDisplay.GraphicsWindowWidth) )
        return;
    if ( (y < 0) || (y >= lbDisplay.GraphicsWindowHeight) )
        return;
    TbPixel *buf;
    int val;
    buf = lbDisplay.GraphicsWindowPtr + lbDisplay.GraphicsScreenWidth * y + x;
    val = (colour << 8) + (*buf);
    *buf = lbDisplay.GlassMap[val];
}

static inline void LbDrawPixelClipOpaq2(long x, long y, TbPixel colour)
{
    if ( (x < 0) || (x >= lbDisplay.GraphicsWindowWidth) )
        return;
    if ( (y < 0) || (y >= lbDisplay.GraphicsWindowHeight) )
        return;
    TbPixel *buf;
    int val;
    buf = lbDisplay.GraphicsWindowPtr + lbDisplay.GraphicsScreenWidth * y + x;
    val = ((*buf) << 8) + colour;
    *buf = lbDisplay.GlassMap[val];
}

static inline void LbDrawPixelClipSolid(long x, long y, TbPixel colour)
{
    if ( (x < 0) || (x >= lbDisplay.GraphicsWindowWidth) )
        return;
    if ( (y < 0) || (y >= lbDisplay.GraphicsWindowHeight) )
        return;
    TbPixel *buf;
    buf = lbDisplay.GraphicsWindowPtr + lbDisplay.GraphicsScreenWidth * y + x;
    *buf = colour;
}

void LbDrawCircleOutline(long x, long y, long radius, TbPixel colour)
{
    int na;
    int nb;
    int n;
    if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
    {
        nb = radius;
        n = 3 - 2 * radius;
        if (radius < 1)
        {
            LbDrawPixelClipOpaq1(x, y, colour);
            return;
        }
        for (na=0; na < nb; na++)
        {
            LbDrawPixelClipOpaq1(x - na, y - nb, colour);
            LbDrawPixelClipOpaq1(x + na, y - nb, colour);
            LbDrawPixelClipOpaq1(x - na, y + nb, colour);
            LbDrawPixelClipOpaq1(x + na, y + nb, colour);
            LbDrawPixelClipOpaq1(x - nb, y - na, colour);
            LbDrawPixelClipOpaq1(x + nb, y - na, colour);
            LbDrawPixelClipOpaq1(x - nb, y + na, colour);
            LbDrawPixelClipOpaq1(x + nb, y + na, colour);
            if (n >= 0)
            {
                n += 10 + 4 * (na - nb);
                nb--;
            } else
            {
                n += 6 + 4 * na;
            }
        }
        if (nb == na)
        {
            LbDrawPixelClipOpaq1(x - na, y - nb, colour);
            LbDrawPixelClipOpaq1(x + na, y - nb, colour);
            LbDrawPixelClipOpaq1(x - na, y + nb, colour);
            LbDrawPixelClipOpaq1(x + na, y + nb, colour);
            LbDrawPixelClipOpaq1(x - nb, y - na, colour);
            LbDrawPixelClipOpaq1(x + nb, y - na, colour);
            LbDrawPixelClipOpaq1(x - nb, y + na, colour);
            LbDrawPixelClipOpaq1(x + nb, y + na, colour);
        }
    } else
    if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8) != 0)
    {
        nb = radius;
        n = 3 - 2 * radius;
        if (radius < 1)
        {
            LbDrawPixelClipOpaq2(x, y, colour);
            return;
        }
        for (na=0; na < nb; na++)
        {
            LbDrawPixelClipOpaq2(x - na, y - nb, colour);
            LbDrawPixelClipOpaq2(x + na, y - nb, colour);
            LbDrawPixelClipOpaq2(x - na, y + nb, colour);
            LbDrawPixelClipOpaq2(x + na, y + nb, colour);
            LbDrawPixelClipOpaq2(x - nb, y - na, colour);
            LbDrawPixelClipOpaq2(x + nb, y - na, colour);
            LbDrawPixelClipOpaq2(x - nb, y + na, colour);
            LbDrawPixelClipOpaq2(x + nb, y + na, colour);
            if (n >= 0)
            {
                n += 10 + 4 * (na - nb);
                nb--;
            } else
            {
                n += 6 + 4 * na;
            }
        }
        if (nb == na)
        {
            LbDrawPixelClipOpaq2(x - na, y - nb, colour);
            LbDrawPixelClipOpaq2(x + na, y - nb, colour);
            LbDrawPixelClipOpaq2(x - na, y + nb, colour);
            LbDrawPixelClipOpaq2(x + na, y + nb, colour);
            LbDrawPixelClipOpaq2(x - nb, y - na, colour);
            LbDrawPixelClipOpaq2(x + nb, y - na, colour);
            LbDrawPixelClipOpaq2(x - nb, y + na, colour);
            LbDrawPixelClipOpaq2(x + nb, y + na, colour);
        }
    } else
    {
        nb = radius;
        n = 3 - 2 * radius;
        if (radius < 1)
        {
            LbDrawPixelClipSolid(x, y, colour);
            return;
        }
        for (na=0; na < nb; na++)
        {
            LbDrawPixelClipSolid(x - na, y - nb, colour);
            LbDrawPixelClipSolid(x + na, y - nb, colour);
            LbDrawPixelClipSolid(x - na, y + nb, colour);
            LbDrawPixelClipSolid(x + na, y + nb, colour);
            LbDrawPixelClipSolid(x - nb, y - na, colour);
            LbDrawPixelClipSolid(x + nb, y - na, colour);
            LbDrawPixelClipSolid(x - nb, y + na, colour);
            LbDrawPixelClipSolid(x + nb, y + na, colour);
            if (n >= 0)
            {
                n += 10 + 4 * (na - nb);
                nb--;
            } else
            {
                n += 6 + 4 * na;
            }
        }
        if (nb == na)
        {
            LbDrawPixelClipSolid(x - na, y - nb, colour);
            LbDrawPixelClipSolid(x + na, y - nb, colour);
            LbDrawPixelClipSolid(x - na, y + nb, colour);
            LbDrawPixelClipSolid(x + na, y + nb, colour);
            LbDrawPixelClipSolid(x - nb, y - na, colour);
            LbDrawPixelClipSolid(x + nb, y - na, colour);
            LbDrawPixelClipSolid(x - nb, y + na, colour);
            LbDrawPixelClipSolid(x + nb, y + na, colour);
        }
    }


}

void LbDrawCircle(long x, long y, long radius, TbPixel colour)
{
    if ((lbDisplay.DrawFlags & Lb_SPRITE_OUTLINE) != 0)
        LbDrawCircleOutline(x, y, radius, colour);
    else
        LbDrawCircleFilled(x, y, radius, colour);
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
}*/

void draw_b_line(long x1, long y1, long x2, long y2, TbPixel colour)
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
  if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4)
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
  if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8)
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

/** Draws a line on current graphics window. Truncates the coordinates
 *  if they go off the window. Does no screen locking.
 */
TbResult LbDrawLine(long x1, long y1, long x2, long y2, TbPixel colour)
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
/*
//Draws any triangle on the current graphics window.
//Does no screen locking.
void __fastcall LbDrawTriangle(long x1, long y1, long x2, long y2, long x3, long y3, TbPixel colour)
{
  if ( lbDisplay.DrawFlags & Lb_SPRITE_UNKNOWN0010 )
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
  if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4)
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
  if (lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8)
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

*/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
