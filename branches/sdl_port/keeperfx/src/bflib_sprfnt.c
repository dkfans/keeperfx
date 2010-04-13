/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sprfnt.c
 *     Bitmap sprite fonts support library.
 * @par Purpose:
 *     Functions for reading bitmap sprite fonts and using them to display text.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Dec 2008 - 11 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_sprfnt.h"

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_sprite.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_vidraw.h"

//TODO: this breaks my convention - non-bflib call from bflib
#include "frontend.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct AsianFont dbcJapFonts[] = {
  {"font12j.fon", 0, 215136, 0x2284, 0, 12, 0x0C00, 24, 1, 6, 12, 12, 12, 0, 1, 1, 1, 1},
  {"font16j.fon", 0, 286848, 0x2284, 0, 16, 0x1000, 32, 1, 8, 16, 16, 16, 0, 1, 1, 4, 2},
  {"font24j.fon", 0, 561744, 0x2284, 0, 24,      0, 72, 1, 8, 24, 24, 24, 0, 1, 1, 4, 2},
};

struct AsianFont dbcChiFonts[] = {
  {"font12c.fon", 0, 199344, 0x1FF2, 0, 12, 0x0C00, 24, 1, 6, 12, 12, 12, 0, 1, 1, 1, 1},
  {"font16c.fon", 0, 271712, 0x20AB, 0, 16, 0x1000, 32, 1, 8, 16, 16, 16, 0, 1, 1, 4, 2},
  {"font16c.fon", 0, 271712, 0x20AB, 0, 16, 0x1000, 32, 1, 8, 16, 16, 16, 0, 1, 1, 4, 2},
};

struct AsianFont dbcChtFonts[] = {
  {"font12f.fon", 0, 215700, 0x1FF2, 0, 12, 0x0C00, 26, 1, 6, 12, 15, 13, 0, 1, 1, 1, 1},
  {"font16f.fon", 0, 265792, 0x1FF2, 0, 16, 0x1000, 32, 1, 8, 16, 16, 16, 0, 1, 1, 4, 2},
  {"font16f.fon", 0, 265792, 0x1FF2, 0, 16, 0x1000, 32, 1, 8, 16, 16, 16, 0, 1, 1, 4, 2},
};

struct AsianFont *active_dbcfont = &dbcJapFonts[0];
long dbc_colour0 = 0;
long dbc_colour1 = 0;
short dbc_language = 0;
TbBool dbc_initialized = false;
TbBool dbc_enabled = true;
/******************************************************************************/
DLLIMPORT int _DK_LbTextDraw(int posx, int posy, const char *text);
DLLIMPORT int _DK_LbTextSetWindow(int, int, int, int);
DLLIMPORT int _DK_LbTextStringWidth(const char *str);
DLLIMPORT int _DK_LbTextStringHeight(const char *str);
DLLIMPORT void _DK_put_down_sprites(const char *a1, const char *a2, long a3, long a4, long a5);
DLLIMPORT long _DK_text_string_height(const char *text);
/******************************************************************************/

/** Returns if the given char starts a wide charcode.
 */
TbBool is_wide_charcode(unsigned long chr)
{
  if (chr > 0xFF)
    return true;
  if ((dbc_initialized) && (dbc_enabled))
  {
    switch (dbc_language)
    {
    case 1:
        return ((chr >= 0x81) && (chr <= 0x9F)) || ((chr >= 0xE0) && (chr <= 0xFC));
    case 2:
        return ((chr > 0xA0) && (chr <= 0xFF));
    case 3:
        return ((chr > 0xA0) && (chr <= 0xFF));
    }
  }
  return false;
}

unsigned short dbc_char_to_font_char(unsigned long chr)
{
  unsigned char i,k;
  unsigned short n;
  unsigned short font_char;
  switch (dbc_language)
  {
  default:
  case 1:
      i = ((chr)&0xFF);
      if (i >= 128)
        i-=32;
      else
        i-=31;
      k = ((chr>>8)&0xFF) - 129;
      if (k >= 31)
        k -= 64;
      k *= 2;
      if (i >= 127)
      {
        k++;
        i -= 94;
      }
      n = ((k + 33) << 8) + i - (33<<8) - 33;
      font_char = 94 * ((n >> 8)&0xFF) + ((n)&0xFF);
      break;
  case 2:
  case 3:
      i = ((chr)&0xFF);
      k = ((chr>>8)&0xFF);
      font_char = 94 * k + i - 15295;
      break;
  }
  SYNCDBG(19,"Char %04X converted to %d",(int)chr,(int)font_char);
  return font_char;
}

int dbc_get_sprite_for_char(struct AsianDraw *adraw, unsigned long chr)
{
  long c,i;
  SYNCDBG(19,"Starting");
  if (active_dbcfont->data == 0)
    return 5;
  if (adraw == NULL)
    return 4;
  if (chr >= 0xFF)
  {
    c = dbc_char_to_font_char(chr);
    if ((c < 0) || (c >= active_dbcfont->field_C))
      return 6;
    adraw->field_0 = chr;
    adraw->bits_width = active_dbcfont->bits_width;
    adraw->field_8 = active_dbcfont->field_30;
    i = active_dbcfont->field_3C;
    adraw->field_C = i;
    adraw->field_10 = active_dbcfont->field_40;
    adraw->field_14 = active_dbcfont->field_44;
    i = c * active_dbcfont->sdata_scanline + active_dbcfont->sdata_shift;
    adraw->sprite_data = active_dbcfont->data + i;
    return 0;
  } else
  {
    adraw->field_0 = chr;
    c = chr;
    adraw->bits_width = active_dbcfont->field_24;
    adraw->field_8 = active_dbcfont->field_28;
    if ((c < 0xA0) || (c > 0xDF))
      i = active_dbcfont->field_34;
    else
      i = active_dbcfont->field_38;
    adraw->field_C = i;
    adraw->field_10 = active_dbcfont->field_40;
    adraw->field_14 = active_dbcfont->field_44;
    i = c * active_dbcfont->ndata_scanline + active_dbcfont->ndata_shift;
    adraw->sprite_data = active_dbcfont->data + i;
    return 0;
  }
}

long dbc_char_height(unsigned long chr)
{
  if (is_wide_charcode(chr))
  {
    return active_dbcfont->field_44 + active_dbcfont->field_40 + active_dbcfont->field_30;
  } else
  {
    return active_dbcfont->field_44 + active_dbcfont->field_40 + active_dbcfont->field_28;
  }
}

long dbc_char_width(unsigned long chr)
{
  if (chr == 0)
  {
    return 0;
  } else
  if (is_wide_charcode(chr))
  {
    return active_dbcfont->field_3C + active_dbcfont->bits_width;
  } else
  {
    return active_dbcfont->field_34 + active_dbcfont->field_24;
  }
}

int dbc_draw_font_sprite(unsigned char *dst_buf, long dst_scanline, unsigned char *src_buf,
      unsigned short src_bitwidth, short start_x, short start_y, short width, short height,
      short colr1, short colr2)
{
  unsigned short src_scanline;
  unsigned char *src;
  unsigned char *dst;
  short colour;
  short skip_count;
  unsigned short src_val;
  int x,y;
  SYNCDBG(19,"Starting at %d,%d size %d,%d",(int)start_x, (int)start_y, (int)width, (int)height);
  // Computing width in bytes from the number of bits
  src_scanline = src_bitwidth >> 3;
  if ((src_bitwidth & 7) != 0)
    src_scanline++;
  if (start_y != 0)
    src_buf += src_scanline * start_y;
  src_val = 0;
  for (y=height; y > 1; y--)
  {
    src = src_buf;
    dst = dst_buf;
    skip_count = start_x;
    for (x=0; x < start_x+width; x++)
    {
      if ((x & 7) == 0)
        src_val = *src++;
      src_val <<= 1;
      if ((src_val & 0x100) != 0)
        colour = colr1;
      else
        colour = colr2;
      if (skip_count > 0)
      {
        skip_count--;
        continue;
      }
      if ((colour & 0xFF00) == 0)
        *dst = colour;
      dst++;
    }
    src_buf += src_scanline;
    dst_buf += dst_scanline;
  }
  return 0;
}

int dbc_fonts_count(void)
{
  switch (dbc_language)
  {
  case 1:
       return (sizeof(dbcJapFonts)/sizeof(dbcJapFonts[0]));
  case 2:
       return (sizeof(dbcChiFonts)/sizeof(dbcChiFonts[0]));
  case 3:
       return (sizeof(dbcChtFonts)/sizeof(dbcChtFonts[0]));
  }
  return 0;
}

struct AsianFont *dbc_fonts_list(void)
{
  switch (dbc_language)
  {
  case 1:
       return dbcJapFonts;
  case 2:
       return dbcChiFonts;
  case 3:
       return dbcChtFonts;
  }
  return NULL;
}

int dbc_draw_font_sprite_text(struct AsianFontWindow *awind, struct AsianDraw *adraw,
      long pos_x, long pos_y, short colr1, short colr2, short colr3)
{
  long scr_x,scr_y;
  unsigned char *dst_buf;
  long width,height;
  long x,y;
  SYNCDBG(19,"Starting");
  if ((adraw == NULL) || (awind == NULL))
    return 4;
  if ((adraw->sprite_data == NULL) || (awind->buf_ptr == NULL))
    return 4;
  if (colr3 >= 0)
  {
    x = 0;
    y = 0;
    scr_y = adraw->field_10 + pos_y + 1;
    scr_x = pos_x + 1;
    width = adraw->bits_width;
    height = adraw->field_8;
    if (scr_x < 0)
    {
      width += scr_x;
      if (width <= 0)
        goto LABEL_21;
      x = -scr_x;
      scr_x = 0;
    } else
    if (scr_x+adraw->bits_width > awind->width)
    {
      if (scr_x >= awind->width)
        goto LABEL_21;
      width = awind->width - scr_x;
    }
    if (scr_y < 0)
    {
      height += scr_y;
      if (height > 0)
      {
        y = -scr_y;
        scr_y = 0;
        if ((width != 0) && (height != 0))
        {
          dst_buf = &awind->buf_ptr[awind->scanline * scr_y + scr_x];
          dbc_draw_font_sprite(dst_buf, awind->scanline, adraw->sprite_data, adraw->bits_width, x, y, width, height, colr3, -1);
        }
      }
    } else
    if (height+scr_y > awind->height)
    {
      if (scr_y < awind->height)
      {
        height = awind->height - scr_y;
        if ((width != 0) && (height != 0))
        {
          dst_buf = &awind->buf_ptr[awind->scanline * scr_y + scr_x];
          dbc_draw_font_sprite(dst_buf, awind->scanline, adraw->sprite_data, adraw->bits_width, x, y, width, height, colr3, -1);
        }
      }
    }
  }
LABEL_21:
  if ((colr1 >= 0) || (colr2 >= 0))
  {
    y = 0;
    x = 0;
    width = adraw->bits_width;
    height = adraw->field_8;
    scr_y = pos_y + adraw->field_10;
    scr_x = pos_x;
    if (pos_x >= 0)
    {
      if ( adraw->bits_width + pos_x > awind->width )
      {
        if (pos_x >= awind->width)
          return 0;
        width = awind->width - pos_x;
      }
    } else
    {
      width += pos_x;
      if (width <= 0)
        return 0;
      scr_x = 0;
      x = -pos_x;
    }
    if (scr_y >= 0)
    {
      if (height + scr_y > awind->height)
      {
        if (scr_y >= awind->height)
          return 0;
        height = awind->height - scr_y;
      }
    } else
    {
      height += scr_y;
      if (height <= 0)
        return 0;
      y = -scr_y;
      scr_y = 0;
    }
    if ((width != 0) && (height != 0))
    {
      dst_buf = &awind->buf_ptr[awind->scanline * scr_y + scr_x];
      dbc_draw_font_sprite(dst_buf, awind->scanline, adraw->sprite_data, adraw->bits_width, x, y, width, height, colr1, colr2);
    }
  }
  return 0;
}

void put_down_dbctext_sprites(const char *sbuf, const char *ebuf, long x, long y, long len)
{
  const char *c;
  unsigned long chr;
  long w,h;
  struct AsianFontWindow awind;
  TbBool needs_draw;
  awind.buf_ptr = lbDisplay.GraphicsWindowPtr;
  awind.width = lbDisplay.GraphicsWindowWidth;
  awind.height = lbDisplay.GraphicsWindowHeight;
  awind.scanline = lbDisplay.PhysicalScreenWidth;
  needs_draw = false;
  for (c=sbuf; c < ebuf; c++)
  {
      chr = (unsigned char)(*c);
      if (is_wide_charcode(chr))
      {
        c++;
        chr = (chr << 8) | (unsigned char)(*c);
        needs_draw = true;
      } else
      if (chr > 32)
      {
        needs_draw = true;
      } else
      if (chr == ' ')
      {
        w = len;
        if ((lbDisplay.DrawFlags & 0x0400) != 0)
        {
          h = dbc_char_height(' ');
          LbDrawHVLine(x, y+h, x + w, y+h, lbDisplay.DrawColour);
        }
        x += w;
      } else
      if (chr == '\t')
      {
        w = len*lbSpacesPerTab;
        if ((lbDisplay.DrawFlags & 0x0400) != 0)
        {
          h = dbc_char_height(' ');
          LbDrawHVLine(x, y+h, x + w, y+h, lbDisplay.DrawColour);
        }
        x += w;
      } else
      {
        switch (chr)
        {
        case 1:
          lbDisplay.DrawFlags ^= 0x0004;
          break;
        case 2:
          lbDisplay.DrawFlags ^= 0x0008;
          break;
        case 3:
          lbDisplay.DrawFlags ^= 0x0010;
          break;
        case 4:
          lbDisplay.DrawFlags ^= 0x0001;
          break;
        case 5:
          lbDisplay.DrawFlags ^= 0x0002;
          break;
        case 11:
          lbDisplay.DrawFlags ^= 0x0400;
          break;
        case 12:
          lbDisplay.DrawFlags ^= 0x0040;
          break;
        case 14:
          c++;
          lbDisplay.DrawColour = (unsigned char)(*c);
          break;
        default:
          break;
        }
      }
      if (needs_draw)
      {
          SYNCDBG(19,"Got needs_draw");
          struct AsianDraw adraw;
          unsigned long colour;
          if (dbc_get_sprite_for_char(&adraw, chr) == 0)
          {
            if ((lbDisplay.DrawFlags & 0x0040) == 0)
              colour = dbc_colour0;
            else
              colour = lbDisplay.DrawColour;
            dbc_draw_font_sprite_text(&awind, &adraw, x, y, colour, -1, dbc_colour1);
            if ((lbDisplay.DrawFlags & 0x0400) != 0)
              LbDrawHVLine(x, y + adraw.field_8, x + adraw.field_C + adraw.bits_width, y + adraw.field_8, lbDisplay.DrawColour);
            x += adraw.field_C + adraw.bits_width;
            if (x >= awind.width)
              return;
          }
          needs_draw = 0;
      }
  }
}

void put_down_simpletext_sprites(const char *sbuf, const char *ebuf, long x, long y, long len)
{
  const char *c;
  const struct TbSprite *spr;
  unsigned char chr;
  long h;
//  _DK_put_down_sprites(sbuf, ebuf, x, y, len);
  for (c=sbuf; c < ebuf; c++)
  {
    chr = (unsigned char)(*c);
    if (chr > 32)
    {
      spr = LbFontCharSprite(lbFontPtr,chr);
      if (spr != NULL)
      {
        if ((lbDisplay.DrawFlags & 0x0040) != 0)
          LbSpriteDrawOneColour(x, y, spr, lbDisplay.DrawColour);
        else
          LbSpriteDraw(x, y, spr);
        if ((lbDisplay.DrawFlags & 0x0400) != 0)
        {
          h = LbTextLineHeight();
          LbDrawHVLine(x, y+h, x+spr->SWidth, y+h, lbDisplay.DrawColour);
        }
        x += spr->SWidth;
      }
    } else
    if (chr == ' ')
    {
      if ((lbDisplay.DrawFlags & 0x0400) != 0)
      {
        h = LbTextLineHeight();
        LbDrawHVLine(x, y+h, x+len, y+h, lbDisplay.DrawColour);
      }
      x += len;
    } else
    if (chr == '\t')
    {
      if ((lbDisplay.DrawFlags & 0x0400) != 0)
      {
        h = LbTextLineHeight();
        LbDrawHVLine(x, y+h, x+len*lbSpacesPerTab, y+h, lbDisplay.DrawColour);
      }
      x += len*lbSpacesPerTab;
    } else
    {
      switch (chr)
      {
        case 1:
          lbDisplay.DrawFlags ^= 0x0004;
          break;
        case 2:
          lbDisplay.DrawFlags ^= 0x0008;
          break;
        case 3:
          lbDisplay.DrawFlags ^= 0x0010;
          break;
        case 4:
          lbDisplay.DrawFlags ^= 0x0001;
          break;
        case 5:
          lbDisplay.DrawFlags ^= 0x0002;
          break;
        case 11:
          lbDisplay.DrawFlags ^= 0x0400;
          break;
        case 12:
          lbDisplay.DrawFlags ^= 0x0040;
          break;
        case 14:
          c++;
          lbDisplay.DrawColour = (unsigned char)(*c);
          break;
        default:
          break;
      }
    }
  }
}

void put_down_sprites(const char *sbuf, const char *ebuf, long x, long y, long len)
{
  if ((dbc_initialized) && (dbc_enabled))
  {
    put_down_dbctext_sprites(sbuf, ebuf, x, y, len);
  } else
  {
    put_down_simpletext_sprites(sbuf, ebuf, x, y, len);
  }
}

long text_string_height(const char *text)
{
  long nlines;
  long lnwidth,lnwidth_clip;
  long w;
  const char *pchr;
  long chr;
  nlines = 0;
  //return _DK_text_string_height(text);
  if (lbFontPtr == NULL)
    return 0;
  lnwidth_clip = lbTextJustifyWindow.x - lbTextClipWindow.x;
  lnwidth = lnwidth_clip;
  for (pchr=text; *pchr != '\0'; pchr++)
  {
    chr = (unsigned char)(*pchr);
    if (is_wide_charcode(chr))
    {
      pchr++;
      if (*pchr == '\0') break;
      chr = (chr<<8) + (unsigned char)*pchr;
    }
    if (chr > 32)
    {
      w = LbTextCharWidth(chr);
      lnwidth += w;
    } else
    if (chr == ' ')
    {
      if (lnwidth > 0)
      {
        w = LbTextCharWidth(' ');
        if (lnwidth+w+LbTextWordWidth(pchr+1)-lnwidth_clip > lbTextJustifyWindow.width)
        {
          lnwidth = lnwidth_clip;
          nlines++;
        } else
        {
          lnwidth += w;
        }
      }
    } else
    switch (chr)
    {
    case '\r':
        lnwidth = lnwidth_clip;
        nlines++;
        if (pchr[1] == '\n') pchr++;
        break;
    case '\n':
        lnwidth = lnwidth_clip;
        nlines++;
        break;
    case '\t':
        w = LbTextCharWidth(' ');
        lnwidth += lbSpacesPerTab * w;
        if (lnwidth+LbTextWordWidth(pchr+1)-lnwidth_clip > lbTextJustifyWindow.width)
        {
          lnwidth = lnwidth_clip;
          nlines++;
        }
        break;
    case 14:
        pchr++;
        break;
    }
  }
  nlines++;
  return nlines * pixel_size * LbTextLineHeight();
}

TbBool LbTextDraw(int posx, int posy, const char *text)
{
  struct TbGraphicsWindow grwnd;
  long count;
  long justifyx,justifyy;
  long startx,starty;
  const char *sbuf;
  const char *ebuf;
  long chr;
  long x,y,len;
  long w,h;
  //return _DK_LbTextDraw(posx, posy, text);
  if ((lbFontPtr == NULL) || (text == NULL))
    return true;
  LbScreenStoreGraphicsWindow(&grwnd);
  LbScreenLoadGraphicsWindow(&lbTextClipWindow);
  count = 0;
  justifyx = lbTextJustifyWindow.x - lbTextClipWindow.x;
  justifyy = lbTextJustifyWindow.y - lbTextClipWindow.y;
  posx += justifyx;
  startx = posx;
  starty = posy + justifyy;

  h = LbTextLineHeight();
  sbuf = text;
  for (ebuf=text; *ebuf != '\0'; ebuf++)
  {
      chr = (unsigned char)*ebuf;
      if (is_wide_charcode(chr))
      {
        ebuf++;
        if (*ebuf == '\0') break;
        chr = (chr<<8) + (unsigned char)*ebuf;
      }

      if (chr > 32)
      {
        w = LbTextCharWidth(chr);
        posx += w;
      } else

      if (chr == ' ')
      {
        w = LbTextCharWidth(' ');
        len = LbSprFontWordWidth(lbFontPtr,ebuf+1);
        if (posx+w+len-justifyx <= lbTextJustifyWindow.width)
        {
          count++;
          posx += w;
          continue;
        }
        posx += w;
        x = LbGetJustifiedCharPosX(startx, posx, w, 1, lbDisplay.DrawFlags);
        y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
        len = LbGetJustifiedCharWidth(posx, w, count, lbDisplay.DrawFlags);
        put_down_sprites(sbuf, ebuf, x, y, len);
        // End the line only if align method is set
        if (LbAlignMethodSet(lbDisplay.DrawFlags))
        {
          posx = startx;
          sbuf = ebuf + 1;
          starty += h;
        }
        count = 0;
      } else

      if (chr == '\r')
      {
        x = LbGetJustifiedCharPosX(startx, posx, 0, 1, lbDisplay.DrawFlags);
        y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
        len = LbTextCharWidth(' ');
        y = starty;
        put_down_sprites(sbuf, ebuf, x, y, len);
        // We've got EOL sign - end the line
        sbuf = ebuf;
        posx = startx;
        starty += h;
        count = 0;
      } else

      if (chr == '\t')
      {
        w = LbTextCharWidth(' ');
        posx += lbSpacesPerTab*w;
        len = LbSprFontWordWidth(lbFontPtr,ebuf+1);
        if (posx+len-justifyx <= lbTextJustifyWindow.width)
        {
          count += lbSpacesPerTab;
          continue;
        }
        x = LbGetJustifiedCharPosX(startx, posx, w, lbSpacesPerTab, lbDisplay.DrawFlags);
        y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
        len = LbGetJustifiedCharWidth(posx, w, count, lbDisplay.DrawFlags);
        put_down_sprites(sbuf, ebuf, x, y, len);
        if (LbAlignMethodSet(lbDisplay.DrawFlags))
        {
          posx = startx;
          sbuf = ebuf + 1;
          starty += h;
        }
        count = 0;
        continue;

      } else

      if ((chr == 6) || (chr == 7) || (chr == 8) || (chr == 9))
      {
        if (posx-justifyx > lbTextJustifyWindow.width)
        {
          x = startx;
          y = starty;
          len = LbTextCharWidth(' ');
          put_down_sprites(sbuf, ebuf, x, y, len);
          posx = startx;
          sbuf = ebuf;
          count = 0;
          starty += h;
        }
        switch (*ebuf)
        {
        case 6:
          lbDisplay.DrawFlags ^= 0x0020;
          break;
        case 7:
          lbDisplay.DrawFlags ^= 0x0080;
          break;
        case 8:
          lbDisplay.DrawFlags ^= 0x0100;
          break;
        case 9:
          lbDisplay.DrawFlags ^= 0x0200;
          break;
        }
      } else

      if (chr == 14)
      {
          ebuf++;
          if (*ebuf == '\0')
            break;
      }
  }
  x = LbGetJustifiedCharPosX(startx, posx, 0, 1, lbDisplay.DrawFlags);
  y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
  len = LbTextCharWidth(' ');
  put_down_sprites(sbuf, ebuf, x, y, len);
  LbScreenLoadGraphicsWindow(&grwnd);
  return true;
}

/** Returns standard height of a line of text, in currently active font.
 *  Supports both sprite fonts and dbc fonts.
 */
int LbTextLineHeight(void)
{
  if ((dbc_initialized) && (dbc_enabled))
  {
    return dbc_char_height(0xFFFF);
  } else
  {
    return LbSprFontCharHeight(lbFontPtr,' ');
  }
}

int LbTextHeight(const char *text)
{
  if ((dbc_initialized) && (dbc_enabled))
  {
    return dbc_char_height(0xFFFF);
  } else
  {
    return LbSprFontCharHeight(lbFontPtr,' ');
  }
}

int LbTextCharWidth(const long chr)
{
  if ((dbc_initialized) && (dbc_enabled))
  {
    return dbc_char_width(chr);
  } else
  {
    return LbSprFontCharWidth(lbFontPtr,(unsigned char)chr);
  }
}

int LbTextCharHeight(const long chr)
{
  if ((dbc_initialized) && (dbc_enabled))
  {
    return dbc_char_height(chr);
  } else
  {
    return LbSprFontCharHeight(lbFontPtr,(unsigned char)chr);
  }
}

int LbTextWordWidth(const char *str)
{
  if ((dbc_initialized) && (dbc_enabled))
  {
    //TODO: make proper function
    return LbSprFontWordWidth(lbFontPtr,str);
  } else
  {
    return LbSprFontWordWidth(lbFontPtr,str);
  }
}

int LbTextSetWindow(int posx, int posy, int width, int height)
{
  return _DK_LbTextSetWindow(posx, posy, width, height);
}

TbBool change_dbcfont(int nfont)
{
  const long fonts_count = dbc_fonts_count();
  struct AsianFont *dbcfonts = dbc_fonts_list();
  if ((nfont >= 0) && (nfont < fonts_count) && (dbcfonts != NULL))
  {
    active_dbcfont = &dbcfonts[nfont];
    return true;
  }
  return false;
}

TbBool LbTextSetFont(const struct TbSprite *font)
{
  TbBool result;
  lbFontPtr = font;
  result = true;
  if (dbc_initialized)
  {
    result = false;
    if (font == frontend_font[0])
    {
      dbc_colour0 = 238;
      dbc_colour1 = 1;
      result = change_dbcfont(2);
    } else
    if (font == frontend_font[1])
    {
      dbc_colour0 = 243;
      dbc_colour1 = 1;
      if (lbDisplay.PhysicalScreenWidth == 320)
        result = change_dbcfont(0);
      else
        result = change_dbcfont(1);
    } else
    if (font == frontend_font[2])
    {
      dbc_colour0 = 248;
      dbc_colour1 = 1;
      if (lbDisplay.PhysicalScreenWidth == 320)
        result = change_dbcfont(0);
      else
        result = change_dbcfont(1);
    } else
    if (font == frontend_font[3])
    {
      dbc_colour0 = 119;
      dbc_colour1 = 1;
      if (lbDisplay.PhysicalScreenWidth == 320)
        result = change_dbcfont(0);
      else
        result = change_dbcfont(1);
    } else
    if (font == winfont)
    {
      dbc_colour0 = 70;
      dbc_colour1 = 1;
      if (lbDisplay.PhysicalScreenWidth == 320)
        result = change_dbcfont(0);
      else
        result = change_dbcfont(1);
    } else
    if (font == font_sprites)
    {
      dbc_colour0 = 1;
      dbc_colour1 = 0;
      if (lbDisplay.PhysicalScreenWidth == 320)
        result = change_dbcfont(0);
      else
        result = change_dbcfont(1);
    } else
    if (font == frontstory_font)
    {
      dbc_colour0 = 237;
      dbc_colour1 = 232;
      if (lbDisplay.PhysicalScreenWidth == 320)
        result = change_dbcfont(0);
      else
        result = change_dbcfont(1);
    } else
    {
      dbc_colour0 = 70;
      dbc_colour1 = 1;
      if (lbDisplay.PhysicalScreenWidth == 320)
        result = change_dbcfont(0);
      else
        result = change_dbcfont(1);
    }
  }
  return result;
}

int LbTextStringWidth(const char *text)
{
  const char *ebuf;
  long chr;
  int len;
  int max_len;
  //return _DK_LbTextStringWidth(str);
  if (lbFontPtr == NULL)
    return 0;
  max_len = 0;
  len = 0;
  for (ebuf=text; *ebuf != '\0'; ebuf++)
  {
      chr = (unsigned char)*ebuf;
      if (is_wide_charcode(chr))
      {
        ebuf++;
        if (*ebuf == '\0') break;
        chr = (chr<<8) + (unsigned char)*ebuf;
      }
      if (chr > 31)
      {
        len += LbTextCharWidth(chr);
      } else
      if (chr == '\r')
      {
        if (len > max_len)
          max_len = len;
          len = 0;
      } else
      if (chr == '\t')
      {
        len += lbSpacesPerTab*LbTextCharWidth(' ');
      } else
      if ((chr == 6) || (chr == 7) || (chr == 8) || (chr == 9) || (chr == 14))
      {
        ebuf++;
        if (*ebuf == '\0')
          break;
      }
  }
  if (len > max_len)
    max_len = len;
  return max_len;
}

int LbTextStringHeight(const char *str)
{
  //return _DK_LbTextStringHeight(str);
  int i,h,lines;
  lines=1;
  if ((lbFontPtr==NULL) || (str==NULL))
    return 0;
  for (i=0;i<MAX_TEXT_LENGTH;i++)
  {
    if (str[i]=='\0') break;
    if (str[i]=='\r') lines++;
  }
  h = LbTextLineHeight();
  return h*lines;
}

int LbTextNumberDraw(int pos_x, int pos_y, long number, unsigned short fdflags)
{
  char text[16];
  int w,h;
  if (lbFontPtr == NULL)
    return 0;
  sprintf(text,"%ld",number);
  h = LbTextLineHeight();
  w = LbTextStringWidth(text);
  switch (fdflags & 0x03)
  {
  case Fnt_LeftJustify:
    LbTextSetWindow(pos_x, pos_y, w, h);
    break;
  case Fnt_RightJustify:
    LbTextSetWindow(pos_x-w, pos_y, w, h);
    break;
  case Fnt_CenterPos:
    LbTextSetWindow(pos_x-(w>>1), pos_y, w, h);
    break;
  }
  LbTextDraw(0, 0, text);
  return w;
}

int LbTextStringDraw(int pos_x, int pos_y, const char *text, unsigned short fdflags)
{
  int w,h;
  if (lbFontPtr == NULL)
    return 0;
  if (text == NULL)
    return 0;
  h = LbTextLineHeight();
  w = LbTextStringWidth(text);
  switch (fdflags & 0x03)
  {
  case Fnt_LeftJustify:
    LbTextSetWindow(pos_x, pos_y, w, h);
    break;
  case Fnt_RightJustify:
    LbTextSetWindow(pos_x-w, pos_y, w, h);
    break;
  case Fnt_CenterPos:
    LbTextSetWindow(pos_x-(w>>1), pos_y, w, h);
    break;
  }
  LbTextDraw(0, 0, text);
  return w;
}

TbBool LbAlignMethodSet(unsigned short fdflags)
{
  const unsigned short align_flags =
        Lb_TEXT_HALIGN_LEFT | Lb_TEXT_HALIGN_RIGHT
      | Lb_TEXT_HALIGN_CENTER | Lb_TEXT_HALIGN_JUSTIFY;
  if ((fdflags & align_flags) != 0)
    return true;
  return false;
}

/**
 * Returns X coordinate for a text character on screen.
 * Takes into account the current text window and justification settings.
 */
long LbGetJustifiedCharPosX(long startx, long all_chars_width, long spr_width, long mul_width, unsigned short fdflags)
{
  long justifyx;
  if ((fdflags & Lb_TEXT_HALIGN_LEFT) != 0)
  {
    return startx;
  } else
  if ((fdflags & Lb_TEXT_HALIGN_RIGHT) != 0)
  {
    justifyx = lbTextJustifyWindow.x - lbTextClipWindow.x;
    return startx + (lbTextJustifyWindow.width + justifyx + mul_width*spr_width - all_chars_width);
  } else
  if ((fdflags & Lb_TEXT_HALIGN_CENTER) != 0)
  {
    justifyx = lbTextJustifyWindow.x - lbTextClipWindow.x;
    return startx + (lbTextJustifyWindow.width + justifyx + mul_width*spr_width - all_chars_width) / 2;
  } else
  if ((fdflags & Lb_TEXT_HALIGN_JUSTIFY) != 0)
  {
    return startx;
  }
  return startx;
}

/**
 * Returns Y coordinate for a text character on screen.
 * Takes into account the current text window and justification settings.
 */
long LbGetJustifiedCharPosY(long starty, long all_lines_height, long spr_height, unsigned short fdflags)
{
  // No vertical justification supported - so the decision is simple
  return starty;
}

/**
 * Returns width for an empty space between words in text on screen.
 * Takes into account the current text window and justification settings.
 */
long LbGetJustifiedCharWidth(long all_chars_width, long spr_width, long words_count, unsigned short fdflags)
{
  long justifyx;
  long space_width;
  if ((fdflags & Lb_TEXT_HALIGN_JUSTIFY) != 0)
  {
    space_width = LbTextCharWidth(' ');
    justifyx = lbTextJustifyWindow.x - lbTextClipWindow.x;
    if (words_count > 0)
      return spr_width + (lbTextJustifyWindow.width+justifyx+space_width-all_chars_width) / words_count;
    return spr_width;
  }
  return spr_width;
}

/**
 * Returns height for an empty space between lines in text on screen.
 * Takes into account the current text window and justification settings.
 */
long LbGetJustifiedCharHeight(long all_lines_height, long spr_height, long lines_count, unsigned short fdflags)
{
  // No vertical justification supported - so the decision is simple
  return spr_height;
}

/**
 * Computes width of one word in given string, starting at given pointer.
 * The word may end with NULL character, space, tab or line end / return carret.
 * @note Works only for characters stored in the sprite list.
 *       Multibyte characters are usually stored somewhere else.
 */
int LbSprFontWordWidth(const struct TbSprite *font,const char *text)
{
  int len;
  const char *c;
  if ((font == NULL) || (text == NULL))
    return 0;
  c = text;
  len = 0;
  while ((*c != ' ') && (*c != '\t') && (*c != '\0') && (*c != '\r') && (*c != '\n'))
  {
    if ((unsigned char)(*c) > 32)
      len += LbSprFontCharWidth(font,(unsigned char)*c);
    c++;
  }
  return len;
}

/**
 * Computes width of a single character in given font.
 * For characters that don't have a sprite (like tab), returns 0.
 * @note Works only for characters stored in the sprite list.
 *       Multibyte characters are usually stored somewhere else.
 */
int LbSprFontCharWidth(const struct TbSprite *font,const unsigned long chr)
{
  const struct TbSprite *spr;
  spr = LbFontCharSprite(font,chr);
  if (spr == NULL)
    return 0;
  return spr->SWidth;
}

/**
 * Computes height of a single character in given font.
 * For characters that don't have a sprite (like tab), returns 0.
 * @note Works only for characters stored in the sprite list.
 *       Multibyte characters are usually stored somewhere else.
 */
int LbSprFontCharHeight(const struct TbSprite *font,const unsigned long chr)
{
  const struct TbSprite *spr;
  spr = LbFontCharSprite(font,chr);
  if (spr == NULL)
    return 0;
  return spr->SHeight;
}

/**
 * Returns sprite of a single character in given font.
 * For characters that don't have a sprite, returns NULL.
 */
const struct TbSprite *LbFontCharSprite(const struct TbSprite *font,const unsigned long chr)
{
  if (font == NULL)
    return NULL;
  if ((chr >= 31) && (chr < 256))
    return &font[(chr-31)];
  return NULL;
}

void dbc_shutdown(void)
{
  const long fonts_count = dbc_fonts_count();
  struct AsianFont *dbcfonts = dbc_fonts_list();
  long i;
  for (i=0; i < fonts_count; i++)
  {
    active_dbcfont = &dbcfonts[i];
    if (active_dbcfont->data != NULL)
    {
      LbMemoryFree(active_dbcfont->data);
      active_dbcfont->data = NULL;
    }
  }
  dbc_initialized = 0;
}

/**
 * Sets a DBC Language for font initialization.
 */
void dbc_set_language(short ilng)
{
  if (!dbc_initialized)
    dbc_language = ilng;
}

/**
 * Loads Double Byte Coding fonts from disk.
 */
short dbc_initialize(const char *fpath)
{
  const long fonts_count = dbc_fonts_count();
  struct AsianFont *dbcfonts = dbc_fonts_list();
  short ret;
  char fname[DISKPATH_SIZE];
  long i,k;
  TbFileHandle fhandle;

  ret = 0;
  if (dbc_initialized)
    dbc_shutdown();
  for (i=0; i < fonts_count; i++)
  {
    active_dbcfont = &dbcfonts[i];
    // Allocate memory for the font
    active_dbcfont->data = LbMemoryAlloc(active_dbcfont->data_length);
    if (active_dbcfont->data == NULL)
    {
      ERRORLOG("Can't allocate memory for font %d",i);
      ret = 2;
      break;
    }
    // Prepare font file name
    if ((fpath != NULL) && (fpath[0] != 0))
    {
      strncpy(fname, fpath, DISKPATH_SIZE);
      fname[DISKPATH_SIZE-1] = '\0';
      if (fname[strlen(fname)-1] != '/')
        strcat(fname, "/");
    } else
    {
      strcpy(fname, lbEmptyString);
    }
    strcat(fname, active_dbcfont->fname);
    // Load font file
    SYNCDBG(9,"Loading font %ld/%ld, file \"%s\"",i,fonts_count,fname);
    fhandle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (fhandle != -1)
    {
      k = active_dbcfont->data_length;
      if (LbFileRead(fhandle, active_dbcfont->data, k) != k)
      {
        ERRORLOG("Error reading %ld bytes from \"%s\"",i,active_dbcfont->fname);
        ret = 3;
      }
      LbFileClose(fhandle);
    } else
    {
      ERRORLOG("Cannot open \"%s\"",fname);
      ret = 1;
    }
    if (ret != 0)
      break;
  }
  if (ret != 0)
  {
    dbc_shutdown();
    return ret;
  }
  dbc_initialized = 1;
  return ret;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
