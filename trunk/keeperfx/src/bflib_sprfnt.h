/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sprfnt.h
 *     Header file for bflib_sprfnt.c.
 * @par Purpose:
 *     Bitmap sprite fonts support library.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     29 Dec 2008 - 11 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_SPRFNT_H
#define BFLIB_SPRFNT_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct TbSprite;
struct TbSetupSprite;

enum TbFontDrawFlags {
  Fnt_LeftJustify   = 0x00,
  Fnt_RightJustify  = 0x01,
  Fnt_CenterPos     = 0x02,
  };

struct AsianFont {
  const char *fname;
  unsigned char *data;
  unsigned long data_length;
  unsigned long chars_count;
  unsigned long ndata_shift;
  unsigned long ndata_scanline;
  unsigned long sdata_shift;
  unsigned long sdata_scanline;
  unsigned long field_20;
  unsigned long field_24;
  unsigned long field_28;
  unsigned long bits_width;
  unsigned long field_30;
  unsigned long field_34;
  unsigned long field_38;
  unsigned long field_3C;
  unsigned long field_40;
  unsigned long field_44;
};

struct AsianDraw {
  unsigned long draw_char;
  unsigned long bits_width;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  unsigned char *sprite_data;
  unsigned long field_1C;
  unsigned long field_20;
};

struct AsianFontWindow {
  unsigned long width;
  unsigned long height;
  unsigned long scanline;
  unsigned char *buf_ptr;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT extern const struct TbSprite *_DK_lbFontPtr;
#define lbFontPtr _DK_lbFontPtr
DLLIMPORT extern unsigned char _DK_lbSpacesPerTab;
#define lbSpacesPerTab _DK_lbSpacesPerTab
/******************************************************************************/
TbBool LbTextDraw(int posx, int posy, const char *text);
int LbTextHeight(const char *text);
int LbTextLineHeight(void);
int LbTextSetWindow(int posx, int posy, int width, int height);
TbResult LbTextSetJustifyWindow(int pos_x, int pos_y, int width);
TbResult LbTextSetClipWindow(int x1, int y1, int x2, int y2);
TbBool LbTextSetFont(const struct TbSprite *font);
unsigned char LbTextGetFontFaceColor(void);
unsigned char LbTextGetFontBackColor(void);
int LbTextStringWidth(const char *str);
int LbTextStringPartWidth(const char *text, int part);
int LbTextStringHeight(const char *str);
int LbTextWordWidth(const char *str);
int LbTextCharWidth(const long chr);
int LbTextCharHeight(const long chr);

int LbTextNumberDraw(int pos_x, int pos_y, long number, unsigned short fdflags);
int LbTextStringDraw(int pos_x, int pos_y, const char *text, unsigned short fdflags);

// Sub-routines, used for drawing text strings. For use in custom drawing methods.
TbBool LbAlignMethodSet(unsigned short fdflags);
long LbGetJustifiedCharPosX(long startx, long all_chars_width, long spr_width, long mul_width, unsigned short fdflags);
long LbGetJustifiedCharPosY(long starty, long all_lines_height, long spr_height, unsigned short fdflags);
long LbGetJustifiedCharWidth(long all_chars_width, long spr_width, long words_count, unsigned short fdflags);
long LbGetJustifiedCharHeight(long all_lines_height, long spr_height, long lines_count, unsigned short fdflags);

// Function which require font sprites as parameter
int LbSprFontWordWidth(const struct TbSprite *font,const char *text);
int LbSprFontCharWidth(const struct TbSprite *font,const unsigned long chr);
int LbSprFontCharHeight(const struct TbSprite *font,const unsigned long chr);
const struct TbSprite *LbFontCharSprite(const struct TbSprite *font,const unsigned long chr);

long text_string_height(const char *text);
void dbc_set_language(short ilng);
short dbc_initialize(const char *fpath);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
