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
#include "pre_inc.h"
#include "bflib_sprfnt.h"

#include <stdarg.h>
#include "bflib_basics.h"
#include "globals.h"

#include "bflib_sprite.h"
#include "bflib_fileio.h"
#include "bflib_vidraw.h"
#include "bflib_text.h"
#include "bflib_string.h"

//TODO: this breaks my convention - non-bflib call from bflib (used for asian fonts)
#include "frontend.h"
#include "front_credits.h"
#include "config_keeperfx.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct AsianDraw {
  unsigned long draw_char;
  unsigned long bits_width;
  unsigned long bits_height;
  unsigned long character_spacing;
  unsigned long vertical_offset;
  unsigned long y_spacing;
  unsigned char *sprite_data;
};

/**
 * Defines a font drawing window.
 * Values are signed to ease comparison with negative values.
 */
struct AsianFontWindow {
  long width;
  long height;
  long scanline;
  unsigned char *buf_ptr;
};

/******************************************************************************/

#define DOUBLE_UNDERLINE_BOUND 16

long dbc_colour0 = 0;
long dbc_colour1 = 0;
short dbc_language = 0;
TbBool dbc_initialized = false;
TbBool dbc_enabled = true;
const struct TbSpriteSheet *lbFontPtr;

#define UNIFONT_INDEX_COUNT 65536
#define UNIFONT_INDEX_SIZE 6
#define UNIFONT_HEIGHT 16

static unsigned char *unifont_data = NULL;
static unsigned short *unifont_widths = NULL;
static unsigned int *unifont_offsets = NULL;
static TbBool unifont_loaded = false;

static void free_unifont_file(void);

static TbGraphicsWindow lbTextJustifyWindow;
static TbGraphicsWindow lbTextClipWindow;
static unsigned char lbSpacesPerTab;
/******************************************************************************/

  static const uint16_t narrow_spacing = 0;
  static const uint16_t kana_spacing = 1;
  static const uint16_t wide_spacing = 1;
  static const uint16_t baseline_offset = 4;
  static const uint16_t line_spacing = 2;


static long dbc_char_height(unsigned long chr)
{
  if (unifont_loaded)
  {
    return UNIFONT_HEIGHT + line_spacing + baseline_offset;
  }
  return 0;
}

static long dbc_char_width(unsigned long chr)
{
    if (chr > 0xFFFF)
      return 0;
    return unifont_widths[(unsigned int)chr] + wide_spacing;
}

/** Returns if the given char starts a wide charcode.
 * @param chr
 */
static TbBool is_duospace_char(unsigned long chr)
{
    if (chr < 0xFF)
        return false;
    return dbc_char_width(chr) >= 16;
    
    return false;
}

/**
 * Draws an underline below the character.
 * @param pos_x
 * @param pos_y
 * @param width
 * @param height
 * @param draw_colr
 * @param shadow_colr
 */
static void LbDrawCharUnderline(long pos_x, long pos_y, long width, long height, uchar draw_colr, uchar shadow_colr)
{
    long h = height;
    long w = width;
    // Draw shadow
    if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLNSHADOW) != 0) {
        long shadow_x = pos_x + 1;
        if (height > 2*DOUBLE_UNDERLINE_BOUND)
            shadow_x++;
        LbDrawHVLine(shadow_x, pos_y+h, shadow_x+w, pos_y+h, shadow_colr);
        h--;
        if (height > DOUBLE_UNDERLINE_BOUND) {
            LbDrawHVLine(shadow_x, pos_y+h, shadow_x+w, pos_y+h, shadow_colr);
            h--;
        }
    }
    // Draw underline
    LbDrawHVLine(pos_x, pos_y+h, pos_x+w, pos_y+h, draw_colr);
    h--;
    if (height > DOUBLE_UNDERLINE_BOUND) {
        LbDrawHVLine(pos_x, pos_y+h, pos_x+w, pos_y+h, draw_colr);
        h--;
    }
}

static TbBool is_kana_char(unsigned long chr)
{
    return (chr >= 0xFF61) && (chr <= 0xFF9F);
}

static int dbc_get_sprite_for_char(struct AsianDraw *adraw, unsigned long chr)
{
    SYNCDBG(19,"Starting");
    if (adraw == NULL)
        return 4;
    if (unifont_loaded)
    {
        if (chr > 0xFFFF)
            return 6;
        const unsigned int codepoint = (unsigned int)chr;
        unsigned short width = unifont_widths[codepoint];
        if (width == 0)
            return 6;
        unsigned int offset = unifont_offsets[codepoint];
        adraw->draw_char = chr;
        adraw->bits_width = width;
        adraw->bits_height = UNIFONT_HEIGHT;

        if (is_kana_char(chr))
          adraw->character_spacing = kana_spacing;
        else if (is_duospace_char(chr))
          adraw->character_spacing = wide_spacing;
        else
          adraw->character_spacing = narrow_spacing;

        adraw->vertical_offset = baseline_offset;
        adraw->y_spacing = line_spacing;
        adraw->sprite_data = unifont_data + offset;
        return 0;
    }
    return 1;

}

static int dbc_draw_font_sprite(unsigned char *dst_buf, long dst_scanline, unsigned char *src_buf,
      unsigned short src_bitwidth, short start_x, short start_y, short width, short height,
      short colr1, short colr2)
{
    SYNCDBG(19,"Starting at %d,%d size %d,%d",(int)start_x, (int)start_y, (int)width, (int)height);
    // Computing width in bytes from the number of bits
    unsigned short src_scanline = src_bitwidth >> 3;
    if ((src_bitwidth & 7) != 0)
        src_scanline++;
    if (start_y != 0)
        src_buf += src_scanline * (long)start_y;
    unsigned short src_val = 0;
    for (int y = height; y > 0; y--)
    {
        unsigned char* src = src_buf;
        unsigned char* dst = dst_buf;
        short skip_count = start_x;
        for (int x = 0; x < start_x + width; x++)
        {
          if ((x & 7) == 0)
            src_val = *src++;
          src_val <<= 1;
          short colour;
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

static int dbc_draw_font_sprite_text(const struct AsianFontWindow *awind, const struct AsianDraw *adraw,
      long pos_x, long pos_y, short colr1, short colr2, short colr3)
{
    long scr_x;
    long scr_y;
    unsigned char *dst_buf;
    long width;
    long height;
    long x;
    long y;
    SYNCDBG(19,"Starting");
    if ((adraw == NULL) || (awind == NULL))
      return 4;
    if ((adraw->sprite_data == NULL) || (awind->buf_ptr == NULL))
      return 4;
    if (colr3 >= 0)
    {
      x = 0;
      y = 0;
      scr_y = adraw->vertical_offset + pos_y + 1;
      scr_x = pos_x + 1;
      width = adraw->bits_width;
      height = adraw->bits_height;
      if (scr_x < 0)
      {
        width += scr_x;
        if (width <= 0)
          goto skip_sprite_draw;
        x = -scr_x;
        scr_x = 0;
        if (width > awind->width)
          width = awind->width;
      } else
      if ((long) (scr_x + adraw->bits_width) > awind->width)
      {
        if (scr_x >= awind->width)
          goto skip_sprite_draw;
        width = awind->width - scr_x;
      }
      if (scr_y < 0)
      {
        height += scr_y;
        if (height > 0)
        {
          y = -scr_y;
          scr_y = 0;
          if (height > awind->height)
            height = awind->height;
          if ((width != 0) && (height != 0))
          {
            if ((scr_x < 0) || (scr_x >= awind->width) || (scr_y < 0) || (scr_y >= awind->height))
              goto skip_sprite_draw;
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
            if ((scr_x < 0) || (scr_x >= awind->width) || (scr_y < 0) || (scr_y >= awind->height))
              goto skip_sprite_draw;
            dst_buf = &awind->buf_ptr[awind->scanline * scr_y + scr_x];
            dbc_draw_font_sprite(dst_buf, awind->scanline, adraw->sprite_data, adraw->bits_width, x, y, width, height, colr3, -1);
          }
        }
      }
    }
skip_sprite_draw:
    if ((colr1 >= 0) || (colr2 >= 0))
    {
      y = 0;
      x = 0;
      width = adraw->bits_width;
      height = adraw->bits_height;
      scr_y = pos_y + adraw->vertical_offset;
      scr_x = pos_x;
      if (pos_x >= 0)
      {
        if (width + pos_x > awind->width)
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
        if (width > awind->width)
          width = awind->width;
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
        if (height > awind->height)
          height = awind->height;
      }
      if ((width != 0) && (height != 0))
      {
        if ((scr_x < 0) || (scr_x >= awind->width) || (scr_y < 0) || (scr_y >= awind->height))
          return 4;
        dst_buf = &awind->buf_ptr[awind->scanline * scr_y + scr_x];
        dbc_draw_font_sprite(dst_buf, awind->scanline, adraw->sprite_data, adraw->bits_width, x, y, width, height, colr1, colr2);
      }
    }
    return 0;
}

static int get_bit_to_array(unsigned char* arrD, int iX, int iY, int iMax)
{
    int iRet = 0;
    int iBytePos = 0;
    int iModBitPos = 0;
    int iPos = (iY * iMax + iX);

    iBytePos = iPos / 8;
    iModBitPos = iPos % 8;

    iRet = (*(arrD + iBytePos) & (0x80 >> iModBitPos)) == (0x80 >> iModBitPos) ? 1 : 0;

    return iRet;
}

static void set_bit_to_array(unsigned char* arrD, int iX, int iY, int iMax, int iValue)
{
    int iBytePos = 0;
    int iModBitPos = 0;
    int iPos = (iY * iMax + iX);

    iBytePos = iPos / 8;
    iModBitPos = iPos % 8;

    if (iValue == 1)
        *(arrD + iBytePos) |= 0x80 >> iModBitPos;
    else
        *(arrD + iBytePos) &= ~(0x80 >> iModBitPos);
}

static int8_t draw_dbc_char(uint32_t chr, struct AsianFontWindow *awind, long *pos_x, long pos_y, long  int units_per_px)
{
    SYNCDBG(19,"Got needs_draw");
    struct AsianDraw adraw;
    unsigned long colour;
    if (dbc_get_sprite_for_char(&adraw, chr) == 0)
    {
        if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) == 0)
          colour = dbc_colour0;
        else
          colour = lbDisplay.DrawColour;

        #define MAX_DBC_SPRITE_SIZE 8192
        unsigned char dest_pixel[MAX_DBC_SPRITE_SIZE] = { 0 };
        if (units_per_px != 16)
        {            
            // Needs to be a multiple of 8
            int iDstSizeH = (units_per_px / 8) * 8;
            int iDstSizeW = (units_per_px * adraw.bits_width / 16 / 8) * 8;
            
            float scale_factorX = (float)adraw.bits_width / (float)iDstSizeW;
            float scale_factorY = (float)adraw.bits_height / (float)iDstSizeH;

            if ((iDstSizeW * iDstSizeH) > MAX_DBC_SPRITE_SIZE)
            {
                ERRORLOG("DBC sprite size %d,%d exceeds max %d",iDstSizeW,iDstSizeH,MAX_DBC_SPRITE_SIZE);
                return -1;
            }
            for (int sY = 0; sY < iDstSizeH; sY++)
            {
                for (int sX = 0; sX < iDstSizeW; sX++)
                {
                    set_bit_to_array(dest_pixel, sX, sY, iDstSizeW, get_bit_to_array(adraw.sprite_data, (int)(sX * scale_factorX), (int)(sY * scale_factorY), adraw.bits_width));
                }
            }

            adraw.bits_width = iDstSizeW;
            adraw.bits_height = iDstSizeH;
            adraw.sprite_data = dest_pixel;
            adraw.character_spacing = adraw.character_spacing * units_per_px / 16;
            adraw.vertical_offset = adraw.vertical_offset * units_per_px / 16;
            adraw.y_spacing = adraw.y_spacing * units_per_px / 16;
        }

        dbc_draw_font_sprite_text(awind, &adraw, *pos_x, pos_y, colour, -1, dbc_colour1);

        int w;
        if (adraw.bits_height == 16)
        {
           w = (adraw.character_spacing + adraw.bits_width) * units_per_px / 16;
        }
        else
        {
            w = (adraw.character_spacing + adraw.bits_width);
        }
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            int h = adraw.bits_height * units_per_px / 16;
            LbDrawCharUnderline(*pos_x,pos_y,w,h,colour,lbDisplayEx.ShadowColour);
        }
        *pos_x += w;
        if (*pos_x >= awind->width)
        {
          return -1;
        }
    }
    return 0;
}

static int8_t draw_simpletext_char(uint32_t chr, long *pos_x, long pos_y, int units_per_px)
{
    const struct TbSprite *spr = LbFontCharSprite(lbFontPtr, chr);
    if (spr != NULL)
    {
        if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0) {
            LbSpriteDrawResizedOneColour(*pos_x, pos_y, units_per_px, spr, lbDisplay.DrawColour);
        }
        else {
            LbSpriteDrawResized(*pos_x, pos_y, units_per_px, spr);
        }
        int w = spr->SWidth * units_per_px / 16;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            int h = LbTextLineHeight() * units_per_px / 16;
            LbDrawCharUnderline(*pos_x, pos_y, w, h, lbDisplay.DrawColour, lbDisplayEx.ShadowColour);
        }
        *pos_x += w;
        return 1;
    }
    return 0;
}

static int8_t draw_char(uint32_t chr, struct AsianFontWindow *awind, long *pos_x, long pos_y, int units_per_px)
{
    if ((dbc_initialized) && (dbc_enabled))
    {
        return draw_dbc_char(chr, awind, pos_x, pos_y, units_per_px);
    } else
    {
        if (draw_simpletext_char(chr, pos_x, pos_y, units_per_px) == 0)
            return draw_dbc_char(chr, awind, pos_x, pos_y, units_per_px);
    }
    return 0;
}


/**
 * Puts scaled simple text sprites on screen.
 * @param sbuf
 * @param ebuf
 * @param x
 * @param y
 * @param len
 */
static void put_down_sprites(const char *sbuf, const char *ebuf, long x, long y, long space_len, int units_per_px)
{
  const char *c;
  long w;
  long h;
    struct AsianFontWindow awind;
    awind.buf_ptr = lbDisplay.GraphicsWindowPtr;
    awind.width = lbDisplay.GraphicsWindowWidth;
    awind.height = lbDisplay.GraphicsWindowHeight;
    awind.scanline = lbDisplay.GraphicsScreenWidth;
  for (c=sbuf; c < ebuf; )
  {
    size_t seq_len;
    uint32_t chr = read_utf_8_codepoint((const char *)c, &seq_len);
    c += seq_len;

    if (chr == 0xA0 || chr == ' ') //NO-BREAK SPACE or SPACE
    {
        w = space_len;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight() * units_per_px / 16;
            LbDrawCharUnderline(x,y,w,h,lbDisplay.DrawColour,lbDisplayEx.ShadowColour);
        }
        x += w;
    } else
    if (chr > 32)
    {
        if (draw_char(chr, &awind, &x, y, units_per_px) < 0)
        {
            return;
        }
    } else
    if (chr == '\t')
    {
        w = space_len*(long)lbSpacesPerTab;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight() * units_per_px / 16;
            LbDrawCharUnderline(x,y,w,h,lbDisplay.DrawColour,lbDisplayEx.ShadowColour);
        }
        x += w;
    } else
    {
      switch (chr)
      {
        case DKChr_Modifier_Transparent4:
          lbDisplay.DrawFlags ^= Lb_SPRITE_TRANSPAR4;
          break;
        case DKChr_Modifier_Transparent8:
          lbDisplay.DrawFlags ^= Lb_SPRITE_TRANSPAR8;
          break;
        case DKChr_Modifier_Outline:
          lbDisplay.DrawFlags ^= Lb_SPRITE_OUTLINE;
          break;
        case DKChr_Modifier_FlipHoriz:
          lbDisplay.DrawFlags ^= Lb_SPRITE_FLIP_HORIZ;
          break;
        case DKChr_Modifier_FlipVertic:
          lbDisplay.DrawFlags ^= Lb_SPRITE_FLIP_VERTIC;
          break;
        case DKChr_Modifier_Underline:
          lbDisplay.DrawFlags ^= Lb_TEXT_UNDERLINE;
          break;
        case DKChr_Modifier_OneColor:
          lbDisplay.DrawFlags ^= Lb_TEXT_ONE_COLOR;
          break;
        case DKChr_Modifier_Colour:
          c++;
          lbDisplay.DrawColour = (unsigned char)(*c);
          break;
        default:
          break;
      }
    }
  }
}

/**
 * Given text and its scale, returns unscaled height which the text would occupy
 * if drawn with current fornt on current text window.
 *
 * @param units_per_px
 * @param text
 */
long text_string_height(int units_per_px, const char *text)
{
    long nlines = 0;
    if (lbFontPtr == NULL)
      return 0;
    long lnwidth_clip = lbTextJustifyWindow.x - lbTextClipWindow.x;
    long lnwidth = lnwidth_clip;
    for (const char* pchr = text; *pchr != '\0'; )
    {

        size_t seq_len;
        uint32_t chr = read_utf_8_codepoint((const char *)pchr, &seq_len);
        pchr += seq_len;

      long w;
      if (chr > 32)
      {
          w = LbTextCharWidthM(chr, units_per_px);
          if (lnwidth + w - lnwidth_clip > lbTextJustifyWindow.width)
          {
            lnwidth = lnwidth_clip + w;
            nlines++;
          } else
          {
            lnwidth += w;
          }
      } else

      if (chr == ' ')
      {
        if (lnwidth > 0)
        {
          w = LbTextCharWidth(' ') * units_per_px / 16;
          if (lnwidth + w + LbTextWordWidthM(pchr, units_per_px) - lnwidth_clip > lbTextJustifyWindow.width)
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
      case DKChr_Return:
          lnwidth = lnwidth_clip;
          nlines++;
          if (pchr[0] == DKChr_NewLine) pchr++;
          break;
      case DKChr_NewLine:
          lnwidth = lnwidth_clip;
          nlines++;
          break;
      case DKChr_Tab:
          w = LbTextCharWidth(' ') * units_per_px / 16;
          lnwidth += lbSpacesPerTab * w;
          if (lnwidth + LbTextWordWidthM(pchr, units_per_px) - lnwidth_clip > lbTextJustifyWindow.width)
          {
            lnwidth = lnwidth_clip;
            nlines++;
          }
          break;
      case DKChr_Modifier_Colour:
          pchr++;
          break;
      }
    }
    nlines++;
    return nlines * (LbTextLineHeight() * units_per_px / 16);
}

/**
 * Draws a string in the current text window in given scale.
 * @param posx Position of the text, X coord.
 * @param posy Position of the text, Y coord.
 * @param units_per_px Scale in pixels; 16 is 100%.
 * @param text The text to be drawn.
 * @return
 */
TbBool LbTextDrawResized(int posx, int posy, int units_per_px, const char *text)
{
    // Counter for amount of blank characters in a line
    const char *ebuf;
    long x;
    long y;
    long len;
    if ((lbFontPtr == NULL) || (text == NULL))
        return true;
    TbGraphicsWindow grwnd;
    LbScreenStoreGraphicsWindow(&grwnd);
    LbScreenLoadGraphicsWindow(&lbTextClipWindow);
    long count = 0;
    long justifyx = lbTextJustifyWindow.x - lbTextClipWindow.x;
    long justifyy = lbTextJustifyWindow.y - lbTextClipWindow.y;
    posx += justifyx;
    long startx = posx;
    long starty = posy + justifyy;

    long h = LbTextLineHeight() * units_per_px / 16;
    const char *draw_buffer = text;

    const char* sbuf = draw_buffer;
    
    for (ebuf=draw_buffer; *ebuf != '\0'; )
    {
        const char* text_backup_pointer = ebuf;

        size_t seq_len;
        uint32_t chr = read_utf_8_codepoint((const char *)ebuf, &seq_len);
        ebuf += seq_len;

        long w;
        if ((chr > 32))
        {
            // Align when ansi and unicode are mixed on one screen
            w = LbTextCharWidthM(chr, units_per_px);

            if ((posx+w-justifyx <= lbTextJustifyWindow.width) || (count > 0) || !LbAlignMethodSet(lbDisplay.DrawFlags))
            {
                posx += w;
                continue;
            }
            // If the char exceeds screen, and there were no spaces in that line, and alignment is set - divide the line here
            w = LbTextCharWidthM(' ', units_per_px);
            posx += w;
            x = LbGetJustifiedCharPosX(startx, posx, w, 1, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbGetJustifiedCharWidth(posx, w, count, units_per_px, lbDisplay.DrawFlags);
            put_down_sprites(sbuf, text_backup_pointer, x, y, len, units_per_px);
            // We already know that alignment is set - don't re-check
            {
                posx = startx;
                sbuf = text_backup_pointer; // sbuf points at start of char for next loop. text_backup_pointer points at unprocessed char.
                ebuf = text_backup_pointer;
                starty += h;
            }
            count = 0;
        } else
        if (chr == ' ')
        {
            w = LbTextCharWidthM(' ', units_per_px);
            len = LbTextWordWidthM(ebuf, units_per_px);
            if (posx+w+len-justifyx <= lbTextJustifyWindow.width)
            {
                count++;
                posx += w;
                continue;
            }
            posx += w;
            x = LbGetJustifiedCharPosX(startx, posx, w, 1, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbGetJustifiedCharWidth(posx, w, count, units_per_px, lbDisplay.DrawFlags);
            put_down_sprites(sbuf, ebuf, x, y, len, units_per_px);
            // End the line only if align method is set
            if (LbAlignMethodSet(lbDisplay.DrawFlags))
            {
              posx = startx;
              sbuf = ebuf; // sbuf should start at the next character, not skip it
              starty += h;
            }
            count = 0;
        } else
        if (chr == '\n')
        {
            w = 0;
            x = LbGetJustifiedCharPosX(startx, posx, w, 1, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbTextCharWidthM(' ', units_per_px);
            y = starty;
            put_down_sprites(sbuf, ebuf, x, y, len, units_per_px);
            // We've got EOL sign - end the line
            sbuf = ebuf;
            posx = startx;
            starty += h;
            count = 0;
        } else
        if (chr == '\t')
        {
            w = LbTextCharWidthM(' ', units_per_px);
            posx += lbSpacesPerTab*w;
            len = LbTextWordWidthM(ebuf, units_per_px);
            if (posx+len-justifyx <= lbTextJustifyWindow.width)
            {
              count += lbSpacesPerTab;
              continue;
            }
            x = LbGetJustifiedCharPosX(startx, posx, w, lbSpacesPerTab, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbGetJustifiedCharWidth(posx, w, count, units_per_px, lbDisplay.DrawFlags);
            put_down_sprites(sbuf, ebuf, x, y, len, units_per_px);
            if (LbAlignMethodSet(lbDisplay.DrawFlags))
            {
              posx = startx;
              sbuf = ebuf;
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
              len = LbTextCharWidthM(' ', units_per_px);
              put_down_sprites(sbuf, ebuf, x, y, len, units_per_px);
              posx = startx;
              sbuf = ebuf;
              count = 0;
              starty += h;
            }
            switch (*ebuf)
            {
            case 6:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_LEFT;
              break;
            case 7:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_RIGHT;
              break;
            case 8:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_CENTER;
              break;
            case 9:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_JUSTIFY;
              break;
            }
        } else
        if (chr == DKChr_Modifier_Colour)
        {
            ebuf++;
            if (*ebuf == '\0')
              break;
        }
    }
    x = LbGetJustifiedCharPosX(startx, posx, 0, 1, lbDisplay.DrawFlags);
    y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
    len = LbTextCharWidthM(' ', units_per_px);
    put_down_sprites(sbuf, ebuf, x, y, len, units_per_px);
    LbScreenLoadGraphicsWindow(&grwnd);
    return true;
}

/**
 * Draws a string in the current text window.
 * @param posx Position of the text, X coord.
 * @param posy Position of the text, Y coord.
 * @param text The text to be drawn.
 * @return
 */
TbBool LbTextDraw(int posx, int posy, const char *text)
{
    // Using resized version - it will end up with version optimized for no resize anyway
    return LbTextDrawResized(posx, posy, 16, text);
}

/**
 * Draws a formatted string in the current text window.
 * @param posx Position of the text, X coord.
 * @param posy Position of the text, Y coord.
 * @param fmt The text format to be drawn.
 * @return
 */
TbBool LbTextDrawResizedFmt(int posx, int posy, int units_per_px, const char *fmt, ...)
{
    char * text = (char *)malloc(8192);
    if (text == NULL) return false;
    va_list val;
    va_start(val, fmt);
    vsnprintf(text, TEXT_DRAW_MAX_LEN, fmt, val);
    va_end(val);
    TbBool result = LbTextDrawResized(posx, posy, units_per_px, text);
    free(text);
    return result;
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

static long dbc_char_widthM(unsigned long chr, long units_per_px)
{
    if (chr == 0)
    {
        return 0;
    }

    struct AsianDraw adraw = { 0 };
    if (dbc_get_sprite_for_char(&adraw, chr) == 0)
    {
       return (adraw.bits_width + adraw.character_spacing) * units_per_px / 16;
    }

    return 0;
}

int LbTextCharWidthM(const uint32_t chr, long units_per_px)
{
    if ((dbc_initialized) && (dbc_enabled))
    {
        return dbc_char_widthM(chr, units_per_px);
    }
    else
    {
        return LbSprFontCharWidth(lbFontPtr, chr) * units_per_px / 16;
    }
}

int LbTextCharWidth(const uint32_t chr)
{
    if ((dbc_initialized) && (dbc_enabled))
    {
      return dbc_char_width(chr);
    } else
    {
      return LbSprFontCharWidth(lbFontPtr,(unsigned char)chr);
    }
}

int LbTextWordWidth(const char *str)
{
    return LbTextWordWidthM(str,16);
}

void LbTextUseByteCoding(TbBool is_enabled)
{
    dbc_enabled = is_enabled;
}

int LbTextSetWindow(int posx, int posy, int width, int height)
{
    lbTextJustifyWindow.x = posx;
    lbTextJustifyWindow.y = posy;
    lbTextJustifyWindow.width = width;
    lbTextJustifyWindow.ptr = &lbDisplay.WScreen[posx + posy * lbDisplay.GraphicsScreenWidth];
    LbTextSetClipWindow(posx, posy, width, height);
    return 1;
}

TbBool LbTextSetFont(const struct TbSpriteSheet *font)
{
    lbFontPtr = font;
    dbc_colour0 = LbTextGetFontFaceColor();
    dbc_colour1 = LbTextGetFontBackColor();
    
    return true;
}

unsigned char LbTextGetFontFaceColor(void)
{
    if (lbFontPtr == frontend_font[0]) {
      return 238;
    } else if (lbFontPtr == frontend_font[1]) {
      return 243;
    } else if (lbFontPtr == frontend_font[2]) {
      return 248;
    } else if (lbFontPtr == frontend_font[3]) {
      return 119;
    } else if (lbFontPtr == winfont) {
      return 73;
    } else if (lbFontPtr == font_sprites) {
      return 1;
    } else if (lbFontPtr == frontstory_font) {
      return 237;
    } else {
      return 70;
    }
}

unsigned char LbTextGetFontBackColor(void)
{
    if (lbFontPtr == font_sprites) {
      return 0;
    } else if (lbFontPtr == frontstory_font) {
        return 232;
    } else {
        return 1;
    }
}

int LbTextStringPartWidthM(const char *text, int part, long units_per_px)
{
    if (lbFontPtr == NULL)
        return 0;
    int max_len = 0;
    int len = 0;
    for (const char* ebuf = text; *ebuf != '\0'; )
    {
        size_t seq_len;
        uint32_t chr = read_utf_8_codepoint((const char *)ebuf, &seq_len);
        ebuf += seq_len;


        if (part <= 0) break;
        part--;

        if (chr > 31)
        {
            len += LbTextCharWidthM(chr, units_per_px);
        }
        else
            if (chr == '\r')
            {
                if (len > max_len)
                {
                    max_len = len;
                    len = 0;
                }
            }
            else
                if (chr == '\t')
                {
                    len += lbSpacesPerTab * LbTextCharWidthM(' ', units_per_px);
                }
                else
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

/**
 * Returns length of part of a text if drawn on screen.
 * @param text The text to be probed.
 * @param part Amount of characters to be probed.
 * @return Width of the text image, in pixels.
 */
int LbTextStringPartWidth(const char *text, int part)
{
    return LbTextStringPartWidthM(text, part, 16);
}

/**
 * Returns length of given text if drawn on screen.
 * @param text The text to be probed.
 * @return Width of the text image, in pixels.
 */
int LbTextStringWidth(const char *text)
{
    return LbTextStringPartWidth(text, INT_MAX);
}

int LbTextStringWidthM(const char *text, long units_per_px)
{
    if ((dbc_initialized) && (dbc_enabled))
    {
        return LbTextStringPartWidthM(text, INT_MAX, units_per_px);
    }
    else
    {
        return LbTextStringWidth(text) * units_per_px / 16;
    }
}

/* @function
 *   Get the scaled length of word for multiple encodings, that is, compatible with dbc or non-dbc.
 *   Like LbTextCharWidthM, but change from one char to one word.
 *   One word defined as continuous and uninterrupted letters.
 *
 * @param units_per_px Scale in pixels.
 */
int LbTextWordWidthM(const char *str, long units_per_px)
{
    if (str == NULL || str[0] == 0)
        return 0;


    int len = 0;
    const char *sbuf = str;
    while (true)
    {
        size_t seq_len;
        uint32_t chr = read_utf_8_codepoint((const char *)sbuf, &seq_len);
        sbuf += seq_len;

        if ((chr == ' ') || (chr == '\t') || (chr == '\0') || (chr == '\r') || (chr == '\n'))
            break;

        if ((dbc_initialized) && (dbc_enabled))
        {
            if (is_duospace_char(chr))
            {
                if (len != 0)
                    break; // letters before, need to stop.
                return dbc_char_widthM(chr, units_per_px);
            }

            len += dbc_char_widthM(chr, units_per_px);
        }
        else
        {
           len += LbSprFontCharWidth(lbFontPtr, chr) * units_per_px / 16; 
        }
    }
    return len;
}

int LbTextStringHeight(const char *str)
{
    int lines = 1;
    if ((lbFontPtr == NULL) || (str == NULL))
        return 0;
    for (int i = 0; i < MAX_TEXT_LENGTH; i++)
    {
        if (str[i]=='\0') break;
        if (str[i]=='\r') lines++;
    }
    int h = LbTextLineHeight();
    return h*lines;
}

int LbTextNumberDraw(int pos_x, int pos_y, int units_per_px, long number, unsigned short fdflags)
{
    if (lbFontPtr == NULL)
      return 0;
    char text[16] = "";
    snprintf(text, sizeof(text), "%ld", number);
    int h = LbTextLineHeight() * units_per_px / 16;
    int w = LbTextStringWidthM(text, units_per_px);
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
    LbTextDrawResized(0, 0, units_per_px, text);
    return w;
}

int LbTextStringDraw(int pos_x, int pos_y, int units_per_px, const char *text, unsigned short fdflags)
{
    if (lbFontPtr == NULL)
      return 0;
    if (text == NULL)
      return 0;
    int h = LbTextLineHeight() * units_per_px / 16;
    int w = LbTextStringWidthM(text, units_per_px);
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
    LbTextDrawResized(0, 0, units_per_px, text);
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

TbResult LbTextSetJustifyWindow(int pos_x, int pos_y, int width)
{
    lbTextJustifyWindow.x = pos_x;
    lbTextJustifyWindow.y = pos_y;
    lbTextJustifyWindow.width = width;
    /* Note: DON'T USE lbTextJustifyWindow_window_ptr in KeeperFX!
    if (lbDisplay.WScreen != NULL)
    {
        lbTextJustifyWindow_window_ptr = lbDisplay.WScreen + pos_x + lbDisplay.GraphicsScreenWidth * pos_y;
    } else
    {
        lbTextJustifyWindow_window_ptr = NULL;
    }*/
    return Lb_SUCCESS;
}

TbResult LbTextSetClipWindow(int pos_x, int pos_y, int width, int height)
{
    int i;
    int start_x = pos_x;
    int start_y = pos_y;
    int end_x = pos_x + width;
    int end_y = pos_y + height;
    if (pos_x > end_x)
    {
      i = pos_x ^ end_x;
      start_x = i ^ pos_x;
      end_x = i ^ pos_x ^ i;
    }
    if ( pos_y > end_y )
    {
      i = pos_y ^ end_y;
      start_y = i ^ pos_y;
      end_y = i ^ pos_y ^ i;
    }
    if ( start_x < 0 )
        start_x = 0;
    if ( end_x < 0 )
      end_x = 0;
    if ( start_y < 0 )
        start_y = 0;
    if ( end_y < 0 )
      end_y = 0;
    if (start_x > lbDisplay.GraphicsScreenWidth)
        start_x = lbDisplay.GraphicsScreenWidth;
    if (end_x > lbDisplay.GraphicsScreenWidth)
      end_x = lbDisplay.GraphicsScreenWidth;
    if (start_y > lbDisplay.GraphicsScreenHeight)
        start_y = lbDisplay.GraphicsScreenHeight;
    if (end_y > lbDisplay.GraphicsScreenHeight)
      end_y = lbDisplay.GraphicsScreenHeight;
    lbTextClipWindow.x = start_x;
    lbTextClipWindow.y = start_y;
    lbTextClipWindow.width = end_x - start_x;
    lbTextClipWindow.height = end_y - start_y;
    /* Note: DON'T USE lbTextClipWindow_window_ptr in KeeperFX!
    lbTextClipWindow_window_ptr = lbDisplay.WScreen + pos_x + lbDisplay.GraphicsScreenWidth * pos_y;
    */
    return Lb_SUCCESS;
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
long LbGetJustifiedCharWidth(long all_chars_width, long spr_width, long words_count, int units_per_px, unsigned short fdflags)
{
    if ((fdflags & Lb_TEXT_HALIGN_JUSTIFY) != 0)
    {
        long space_width = LbTextCharWidth(' ') * units_per_px / 16;
        long justifyx = lbTextJustifyWindow.x - lbTextClipWindow.x;
        if (words_count > 0)
            return spr_width + (lbTextJustifyWindow.width + justifyx + space_width - all_chars_width) / words_count;
        return spr_width;
    }
    return spr_width;
}


/**
 * Computes width of a single character in given font.
 * For characters that don't have a sprite (like tab), returns 0.
 * @note Works only for characters stored in the sprite list.
 *       Multibyte characters are usually stored somewhere else.
 */
int LbSprFontCharWidth(const struct TbSpriteSheet * font, const uint32_t chr)
{
    const struct TbSprite* spr = LbFontCharSprite(font, chr);
    if (spr == NULL)
        return dbc_char_width(chr);
    return spr->SWidth;
}

/**
 * Computes height of a single character in given font.
 * For characters that don't have a sprite (like tab), returns 0.
 * @note Works only for characters stored in the sprite list.
 *       Multibyte characters are usually stored somewhere else.
 */
int LbSprFontCharHeight(const struct TbSpriteSheet * font, const uint32_t chr)
{
    const struct TbSprite* spr = LbFontCharSprite(font, chr);
    if (spr == NULL)
        return dbc_char_height(chr);
    return spr->SHeight;
}

/**
 * Returns sprite of a single character in given font.
 * For characters that don't have a sprite, returns NULL.
 */
const struct TbSprite * LbFontCharSprite(const struct TbSpriteSheet * font, const uint32_t codepoint)
{
    if (font == NULL)
        return NULL;

    uint32_t sprite_index = 0;

    if (codepoint < 0x80){
        sprite_index = codepoint - 31;
    } else if (codepoint >= white_numbers_start && codepoint <= white_numbers_end){
        // white numbers mapped to private use area in unicode
        sprite_index = codepoint - white_numbers_start + 153;
    } else {
      static const struct
      {
          uint32_t unicode;
          uint16_t sprite_idx;
      } codepage_map[] = {
        {0x0410, 34 }, {0x0411, 146}, {0x0412, 35 }, {0x0413, 147},
        {0x0414, 148}, {0x0415, 38 }, {0x0416, 149}, {0x0417, 163},
        {0x0418, 164}, {0x0419, 165}, {0x041A, 44 }, {0x041B, 96},
        {0x041C, 46 }, {0x041D, 41 }, {0x041E, 48 }, {0x041F, 115},
        {0x0420, 49 }, {0x0421, 36 }, {0x0422, 53 }, {0x0423, 166},
        {0x0424, 169}, {0x0425, 57 }, {0x0426, 170}, {0x0427, 171},
        {0x0428, 172}, {0x0429, 173}, {0x042A, 67 }, {0x042B, 201},
        {0x042C, 67 }, {0x042D, 124}, {0x042E, 125}, {0x042F, 126},
        {0x0430, 66 }, {0x0431, 174}, {0x0432, 175}, {0x0433, 176},
        {0x0434, 177}, {0x0435, 70 }, {0x0436, 178}, {0x0437, 182},
        {0x0438, 186}, {0x0439, 187}, {0x043A, 127}, {0x043B, 128},
        {0x043C, 135}, {0x043D, 136}, {0x043E, 80 }, {0x043F, 138},
        {0x0440, 81 }, {0x0441, 68 }, {0x0442, 188}, {0x0443, 90 },
        {0x0444, 189}, {0x0445, 89 }, {0x0446, 190}, {0x0447, 192},
        {0x0448, 199}, {0x0449, 200}, {0x044A, 139}, {0x044B, 140},
        {0x044C, 141}, {0x044D, 143}, {0x044E, 144}, {0x044F, 145},
        {0x0407, 185}, {0x0457, 108}, {0x0456, 74 }, {0x0406, 42 },
        {0x0404, 223}, {0x0454, 224}, {0x0490, 225}, {0x0491, 226},
        {0x0451, 106}, {0x0401, 180},
        {0x2019, 8  }, {0x2014, 14 }, 
        {0x00C7, 97 }, {0x00FC, 98 }, {0x00E9, 99 }, {0x00E2, 100},
        {0x00E4, 101}, {0x00E0, 102}, {0x00E5, 103}, {0x00E7, 104},
        {0x00EA, 105}, {0x00EB, 106}, {0x00E8, 107}, {0x00EF, 108},
        {0x00EE, 109}, {0x00EC, 110}, {0x00C4, 111}, {0x00C5, 112},
        {0x00C9, 113}, {0x00E6, 114}, {0x00F4, 116}, {0x00F6, 117},
        {0x00F2, 118}, {0x00FB, 119}, {0x00F9, 120}, {0x00FF, 121},
        {0x00D6, 122}, {0x00DC, 123}, {0x00E1, 129}, {0x00ED, 130},
        {0x00F3, 131}, {0x00FA, 132}, {0x00F1, 133}, {0x00D1, 134},
        {0x00BF, 137}, {0x00A1, 142}, {0x00C1, 150}, {0x00C2, 151},
        {0x00C0, 152}, {0x00E3, 167}, {0x00C3, 168}, {0x00CA, 179},
        {0x00CB, 180}, {0x00C8, 181}, {0x00CD, 183}, {0x00CE, 184},
        {0x00CF, 185}, {0x00CC, 191}, {0x00D3, 193}, {0x00DF, 194},
        {0x00D4, 195}, {0x00D2, 196}, {0x00F5, 197}, {0x00D5, 198},
        {0x00DA, 202}, {0x00DB, 203}, {0x00D9, 204}, {0x00FD, 205},
        {0x00DD, 206}, {0x0105, 207}, {0x0104, 208}, {0x0107, 209},
        {0x0106, 210}, {0x0119, 211}, {0x0118, 212}, {0x0142, 213},
        {0x0141, 214}, {0x0144, 215}, {0x0143, 216}, {0x015B, 217},
        {0x015A, 218}, {0x017A, 219}, {0x0179, 220}, {0x017C, 221},
        {0x017B, 222},
        {0x011B, 70 }, {0x010D, 68 }, {0x010F, 69}, {0x0159, 83},
        {0x0161, 84 }, {0x017E, 91 }, {0x0165, 85}, {0x0148, 79},
        {0x016F, 86 }, {0x010C, 36 }, {0x010E, 37}, {0x0158, 51},
        {0x0160, 52 }, {0x017D, 59 }, {0x0164, 53}, {0x0147, 47},
        {0x016E, 54 }, {0x011A, 38 }, {0x1804, 27}, {0xFE55, 27},
        {0xFE54, 28 }, {0x00C6, 114},

      };
      for (size_t i = 0; i < sizeof(codepage_map) / sizeof(codepage_map[0]); ++i)
      {
          if (codepage_map[i].unicode == codepoint)
          {
              sprite_index = codepage_map[i].sprite_idx;
              break;
          }
      }
    }
    if (sprite_index == 0)
    {
        return NULL;
    }
    return get_sprite(font, sprite_index);
}

static void free_unifont_file(void)
{
    if (unifont_data != NULL)
    {
        free(unifont_data);
        unifont_data = NULL;
    }
    if (unifont_widths != NULL)
    {
        free(unifont_widths);
        unifont_widths = NULL;
    }
    if (unifont_offsets != NULL)
    {
        free(unifont_offsets);
        unifont_offsets = NULL;
    }
    unifont_loaded = false;
}

void dbc_shutdown(void)
{
  free_unifont_file();
  dbc_initialized = 0;
}

char * prepare_font_filename(const char * fpath, const char * fname) {
  if (fpath == NULL || fpath[0] == 0)
  {
    // current folder, copy font filename as-is
    const int fname_len = strlen(fname);
    const int buffer_size = fname_len + 1;
    char * buffer = malloc(buffer_size);
    if (buffer == NULL)
    {
      return NULL;
    }
    memcpy(buffer, fname, buffer_size);
    return buffer;
  }
  const int fpath_len = strlen(fpath);
  const int fname_len = strlen(fname);
  const int buffer_size = fpath_len + fname_len + 2;
  char * buffer = malloc(buffer_size);
  if (buffer == NULL)
  {
    return NULL;
  }
  if (fpath[fpath_len - 1] != '/')
  {
    // path does not end with /
    snprintf(buffer, buffer_size, "%s/%s", fpath, fname);
  }
  else
  {
    // path ends with /
    snprintf(buffer, buffer_size, "%s%s", fpath, fname);
  }
  return buffer;
}

short load_unifont_file()
{
    char* fname = prepare_file_fmtpath(FGrp_FxData, "unifont_%s.fxfont", get_language_lwrstr(install_info.lang_id));

    if ( !LbFileExists(fname) )
    {
        fname = prepare_file_path(FGrp_FxData, "unifont.fxfont");
    }

    long filelen = LbFileLength(fname);
    if (filelen < UNIFONT_INDEX_COUNT * UNIFONT_INDEX_SIZE)
    {
        free(fname);
        return 1;
    }
    TbFileHandle fhandle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (!fhandle)
    {
        free(fname);
        return 1;
    }
    long index_bytes = UNIFONT_INDEX_COUNT * UNIFONT_INDEX_SIZE;
    unsigned char *index_buf = (unsigned char *)malloc(index_bytes);
    if (index_buf == NULL)
    {
        ERRORLOG("Can't allocate memory for unifont index");
        LbFileClose(fhandle);
        free(fname);
        return 2;
    }
    if (LbFileRead(fhandle, index_buf, index_bytes) != index_bytes)
    {
        ERRORLOG("Error reading unifont index from \"%s\"", fname);
        free(index_buf);
        LbFileClose(fhandle);
        free(fname);
        return 3;
    }
    long data_len = filelen - index_bytes;
    unsigned char *data_buf = (unsigned char *)malloc(data_len > 0 ? data_len : 1);
    if (data_buf == NULL)
    {
        ERRORLOG("Can't allocate memory for unifont data");
        free(index_buf);
        LbFileClose(fhandle);
        free(fname);
        return 2;
    }
    if (data_len > 0 && LbFileRead(fhandle, data_buf, data_len) != data_len)
    {
        ERRORLOG("Error reading unifont data from \"%s\"", fname);
        free(index_buf);
        free(data_buf);
        LbFileClose(fhandle);
        free(fname);
        return 3;
    }
    LbFileClose(fhandle);
    free(fname);

    unsigned short *widths = (unsigned short *)malloc(UNIFONT_INDEX_COUNT * sizeof(*widths));
    unsigned int *offsets = (unsigned int *)malloc(UNIFONT_INDEX_COUNT * sizeof(*offsets));
    if (widths == NULL || offsets == NULL)
    {
        ERRORLOG("Can't allocate memory for unifont index arrays");
        free(index_buf);
        free(data_buf);
        free(widths);
        free(offsets);
        return 2;
    }
    for (unsigned int i = 0; i < UNIFONT_INDEX_COUNT; ++i)
    {
        unsigned int pos = i * UNIFONT_INDEX_SIZE;
        widths[i] = (unsigned short)index_buf[pos] | ((unsigned short)index_buf[pos + 1] << 8);
        offsets[i] = (unsigned int)index_buf[pos + 2] | ((unsigned int)index_buf[pos + 3] << 8)
                     | ((unsigned int)index_buf[pos + 4] << 16) | ((unsigned int)index_buf[pos + 5] << 24);
        if (widths[i] != 0)
        {
            unsigned int row_bytes = (widths[i] + 7) >> 3;
            unsigned long max_offset = offsets[i] + row_bytes * UNIFONT_HEIGHT;
            if ((long)max_offset > data_len)
            {
                ERRORLOG("Invalid unifont offset for codepoint %u", i);
                free(index_buf);
                free(data_buf);
                free(widths);
                free(offsets);
                return 3;
            }
        }
    }
    free(index_buf);

    free_unifont_file();
    unifont_data = data_buf;
    unifont_widths = widths;
    unifont_offsets = offsets;
    unifont_loaded = true;
    return 0;
}

/**
 * Loads Double Byte Coding fonts from disk.
 */
short dbc_initialize(const char *fpath)
{

  if (dbc_initialized)
  {
    dbc_shutdown();
  }

  if (load_unifont_file(fpath) == 0)
  {
    dbc_initialized = 1;
    dbc_language = 1;
    return 0;
  }

  return 0;
}

TbBool is_dbc_language(short language)
{
    return (language == Lang_Japanese) || (language == Lang_ChineseInt) || 
           (language == Lang_ChineseTra) || (language == Lang_Korean);
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
