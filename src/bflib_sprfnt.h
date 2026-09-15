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
extern long dbc_colour0;
extern long dbc_colour1;
struct AsianFont;
extern struct AsianFont *active_dbcfont;

/******************************************************************************/


#pragma pack()
/******************************************************************************/
TbBool LbTextDraw(int posx, int posy, const char *text);
#define LbTextDrawFmt(posx, posy, fmt, ...) LbTextDrawResizedFmt(posx, posy, 16, fmt, ##__VA_ARGS__)
TbBool LbTextDrawResized(int posx, int posy, int units_per_px, const char *text);
/** The text draw itself. LbTextDrawResized routes through the renderer first;
 *  the renderer calls this when it is time to actually put pixels down. */
TbBool LbTextDrawResizedImmediate(int posx, int posy, int units_per_px, const char *text);
TbBool LbTextDrawResizedFmt(int posx, int posy, int units_per_px, const char *fmt, ...);
int LbTextHeight(const char *text);
int LbTextLineHeight(void);
int LbTextSetWindow(int posx, int posy, int width, int height);
TbResult LbTextSetJustifyWindow(int pos_x, int pos_y, int width);
TbResult LbTextSetClipWindow(int x1, int y1, int x2, int y2);
void LbTextGetJustifyWindow(int *out_x, int *out_y, int *out_width);
void LbTextGetClipWindow(int *out_x, int *out_y, int *out_width, int *out_height);
unsigned int LbTextGetFontGeneration(void);
/** Call when a font's underlying TbSpriteSheet memory is freed/reloaded --
 *  NOT on mere selection (LbTextSetFont() just changes which already-loaded
 *  font is current, it doesn't invalidate any pointer). See the generation
 *  field's own comment in bflib_sprfnt.c for what this guards against. */
void LbTextInvalidateFontGeneration(void);
TbBool LbTextSetFont(const struct TbSpriteSheet *font);
unsigned char LbTextGetFontFaceColor(void);
unsigned char LbTextGetFontBackColor(void);
int LbTextStringWidth(const char *str);
int LbTextStringPartWidth(const char *text, int part);
int LbTextStringHeight(const char *str);
int LbTextWordWidth(const char *str);
int LbTextCharWidth(const uint32_t chr);
int LbTextCharWidthM(const uint32_t chr, long units_per_px);
int LbTextStringWidthM(const char *str, long units_per_px);
int LbTextWordWidthM(const char *str, long units_per_px);

int LbTextNumberDraw(int pos_x, int pos_y, int units_per_px, long number, unsigned short fdflags);
int LbTextStringDraw(int pos_x, int pos_y, int units_per_px, const char *text, unsigned short fdflags);

// Sub-routines, used for drawing text strings. For use in custom drawing methods.
TbBool LbAlignMethodSet(unsigned short fdflags);
long LbGetJustifiedCharPosX(long startx, long all_chars_width, long spr_width, long mul_width, unsigned short fdflags);
long LbGetJustifiedCharPosY(long starty, long all_lines_height, long spr_height, unsigned short fdflags);
long LbGetJustifiedCharWidth(long all_chars_width, long spr_width, long words_count, int units_per_px, unsigned short fdflags);

// Function which require font sprites as parameter
int LbSprFontCharWidth(const struct TbSpriteSheet * font, const uint32_t chr);
int LbSprFontCharHeight(const struct TbSpriteSheet * font,const uint32_t chr);
const struct TbSprite * LbFontCharSprite(const struct TbSpriteSheet * font, const uint32_t chr);

void LbTextUseByteCoding(TbBool is_enabled);
long text_string_height(int units_per_px, const char *text);
short load_unifont_files();
TbBool is_dbc_language(short language);
unsigned char LbTextGetSpacesPerTab(void);

// Todo : Refactor, these are used by GL, i hate this.
int LbDbcGetGlyphBits(const struct AsianFont *font, uint32_t chr,
                       const unsigned char **out_data, int *out_scanline_bytes,
                       int *out_w, int *out_h, int *out_char_spacing, int *out_v_offset);
int LbDbcCharWidthM(const struct AsianFont *font, uint32_t chr, long units_per_px);
int LbDbcCharHeight(const struct AsianFont *font);
TbBool LbDbcIsDuospaceChar(const struct AsianFont *font, uint32_t chr);
int LbTextWordWidthExplicit(const struct TbSpriteSheet *font, const struct AsianFont *dbcfont,
                             TbBool use_dbc, const char *str, long units_per_px);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
