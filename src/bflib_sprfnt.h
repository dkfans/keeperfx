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

#define TEXT_DRAW_MAX_LEN 4096

enum TbFontDrawFlags {
  Fnt_LeftJustify   = 0x00,
  Fnt_RightJustify  = 0x01,
  Fnt_CenterPos     = 0x02,
  Fnt_CenterLeftPos = 0x03,
  };

/******************************************************************************/
#pragma pack(1)

struct TbSprite;
struct TbSetupSprite;

enum DkcodepageLetter {
   DKChr_Null,
   DKChr_Modifier_Transparent4,
   DKChr_Modifier_Transparent8,
   DKChr_Modifier_Outline,
   DKChr_Modifier_FlipHoriz,
   DKChr_Modifier_FlipVertic,
   DKChr_AlignLeft,
   DKChr_AlignRight,
   DKChr_AlignCenter,
   DKChr_AlignJustify = 9, //tab and AlignJustify overlap so Justify can't be reached
   DKChr_Tab = 9,
   DKChr_NewLine,
   DKChr_Modifier_Underline,
   DKChr_Modifier_OneColor,
   DKChr_Return,
   DKChr_Modifier_Colour,
};

// unicode private use area mappings
static const uint32_t white_numbers_start = 0xF000;
static const uint32_t white_numbers_end   = 0xF009;
static const uint32_t colour_modifiers_begin = 0xF100;
static const uint32_t colour_modifiers_end   = 0xF1FF;


extern TbBool dbc_enabled;
extern TbBool dbc_initialized;
extern const struct TbSpriteSheet *lbFontPtr;

/******************************************************************************/


#pragma pack()
/******************************************************************************/
TbBool LbTextDraw(int posx, int posy, const char *text);
#define LbTextDrawFmt(posx, posy, fmt, ...) LbTextDrawResizedFmt(posx, posy, 16, fmt, ##__VA_ARGS__)
TbBool LbTextDrawResized(int posx, int posy, int units_per_px, const char *text);
TbBool LbTextDrawResizedFmt(int posx, int posy, int units_per_px, const char *fmt, ...);
int LbTextHeight(const char *text);
int LbTextLineHeight(void);
int LbTextSetWindow(int posx, int posy, int width, int height);
TbResult LbTextSetJustifyWindow(int pos_x, int pos_y, int width);
TbResult LbTextSetClipWindow(int x1, int y1, int x2, int y2);
TbBool LbTextSetFont(const struct TbSpriteSheet *font);
unsigned char LbTextGetFontFaceColor(void);
unsigned char LbTextGetFontBackColor(void);
int LbTextStringWidth(const char *str);
int LbTextStringPartWidth(const char *text, int part);
int LbTextStringHeight(const char *str);
int LbTextWordWidth(const char *str);
int LbTextCharWidth(const uint32_t chr);
int LbTextCharWidthM(const uint32_t chr, int32_t units_per_px);
int LbTextStringWidthM(const char *str, int32_t units_per_px);
int LbTextWordWidthM(const char *str, int32_t units_per_px);

int LbTextNumberDraw(int pos_x, int pos_y, int units_per_px, int32_t number, unsigned short fdflags);
int LbTextStringDraw(int pos_x, int pos_y, int units_per_px, const char *text, unsigned short fdflags);

// Sub-routines, used for drawing text strings. For use in custom drawing methods.
TbBool LbAlignMethodSet(unsigned short fdflags);
int32_t LbGetJustifiedCharPosX(int32_t startx, int32_t all_chars_width, int32_t spr_width, int32_t mul_width, unsigned short fdflags);
int32_t LbGetJustifiedCharPosY(int32_t starty, int32_t all_lines_height, int32_t spr_height, unsigned short fdflags);
int32_t LbGetJustifiedCharWidth(int32_t all_chars_width, int32_t spr_width, int32_t words_count, int units_per_px, unsigned short fdflags);

// Function which require font sprites as parameter
int LbSprFontCharWidth(const struct TbSpriteSheet * font, const uint32_t chr);
int LbSprFontCharHeight(const struct TbSpriteSheet * font,const uint32_t chr);
const struct TbSprite * LbFontCharSprite(const struct TbSpriteSheet * font, const uint32_t chr);

void LbTextUseByteCoding(TbBool is_enabled);
int32_t text_string_height(int units_per_px, const char *text);
short load_unifont_files();
TbBool is_dbc_language(short language);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
