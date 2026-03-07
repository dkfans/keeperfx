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
   DKChr_unused6,
   DKChr_unused7,
   DKChr_unused8,
   DKChr_Tab,
   DKChr_NewLine,
   DKChr_Modifier_Underline,
   DKChr_Modifier_OneColor,
   DKChr_Return,
   DKChr_Modifier_Colour,
   DKChr_Cyrillic_Ukrainian_Upper_Ye,
   DKChr_Cyrillic_Ukrainian_Lower_Ye,
   DKChr_Cyrillic_Ukrainian_Upper_GheWithUpturn,
   DKChr_Cyrillic_Ukrainian_Lower_GheWithUpturn,
   DKChr_unused16,
   DKChr_unused17,
   DKChr_unused18,
   DKChr_unused19,
   DKChr_unused20,
   DKChr_unused21,
   DKChr_unused22,
   DKChr_unused23,
   DKChr_unused24,
   DKChr_unused25,
   DKChr_unused26,
   DKChr_unused27,
   DKChr_unused28,
   DKChr_Space,
   DKChr_Exclamation,
   DKChr_Quote,
   DKChr_Hash,
   DKChr_Dollar,
   DKChr_Percent,
   DKChr_Ampersand,
   DKChr_Apostrophe,
   DKChr_LeftParen,
   DKChr_RightParen,
   DKChr_Asterisk,
   DKChr_Plus,
   DKChr_Comma,
   DKChr_Hyphen,
   DKChr_Period,
   DKChr_Slash,
   DKChr_Digit0,
   DKChr_Digit1,
   DKChr_Digit2,
   DKChr_Digit3,
   DKChr_Digit4,
   DKChr_Digit5,
   DKChr_Digit6,
   DKChr_Digit7,
   DKChr_Digit8,
   DKChr_Digit9,
   DKChr_Colon,
   DKChr_Semicolon,
   DKChr_LessThan,
   DKChr_Equals,
   DKChr_GreaterThan,
   DKChr_QuestionMark,
   DKChr_At,
   DKChr_Upper_A,
   DKChr_Upper_B,
   DKChr_Upper_C,
   DKChr_Upper_D,
   DKChr_Upper_E,
   DKChr_Upper_F,
   DKChr_Upper_G,
   DKChr_Upper_H,
   DKChr_Upper_I,
   DKChr_Upper_J,
   DKChr_Upper_K,
   DKChr_Upper_L,
   DKChr_Upper_M,
   DKChr_Upper_N,
   DKChr_Upper_O,
   DKChr_Upper_P,
   DKChr_Upper_Q,
   DKChr_Upper_R,
   DKChr_Upper_S,
   DKChr_Upper_T,
   DKChr_Upper_U,
   DKChr_Upper_V,
   DKChr_Upper_W,
   DKChr_Upper_X,
   DKChr_Upper_Y,
   DKChr_Upper_Z,
   DKChr_LeftBracket,
   DKChr_Backslash,
   DKChr_RightBracket,
   DKChr_Caret,
   DKChr_Underscore,
   DKChr_Grave,
   DKChr_Lower_a,
   DKChr_Lower_b,
   DKChr_Lower_c,
   DKChr_Lower_d,
   DKChr_Lower_e,
   DKChr_Lower_f,
   DKChr_Lower_g,
   DKChr_Lower_h,
   DKChr_Lower_i,
   DKChr_Lower_j,
   DKChr_Lower_k,
   DKChr_Lower_l,
   DKChr_Lower_m,
   DKChr_Lower_n,
   DKChr_Lower_o,
   DKChr_Lower_p,
   DKChr_Lower_q,
   DKChr_Lower_r,
   DKChr_Lower_s,
   DKChr_Lower_t,
   DKChr_Lower_u,
   DKChr_Lower_v,
   DKChr_Lower_w,
   DKChr_Lower_x,
   DKChr_Lower_y,
   DKChr_Lower_z,
   DKChr_LeftBrace,
   DKChr_Pipe,
   DKChr_RightBrace,
   DKChr_Tilde,
   DKChr_Cyrillic_Upper_El,
   DKChr_Upper_C_Cedilla,
   DKChr_Lower_u_Umlaut,
   DKChr_Lower_e_Acute,
   DKChr_Lower_a_Circumflex,
   DKChr_Lower_a_Umlaut,
   DKChr_Lower_a_Grave,
   DKChr_Lower_a_Ring,
   DKChr_Lower_c_Cedilla,
   DKChr_Lower_e_Circumflex,
   DKChr_Lower_e_Umlaut,
   DKChr_Lower_e_Grave,
   DKChr_Lower_i_Umlaut,
   DKChr_Lower_i_Circumflex,
   DKChr_Lower_i_Grave,
   DKChr_Upper_A_Umlaut,
   DKChr_Upper_A_Ring,
   DKChr_Upper_E_Acute,
   DKChr_Ligature_ae,
   DKChr_Cyrillic_Upper_Pe,
   DKChr_Lower_o_Circumflex,
   DKChr_Lower_o_Umlaut,
   DKChr_Lower_o_Grave,
   DKChr_Lower_u_Circumflex,
   DKChr_Lower_u_Grave,
   DKChr_Lower_y_Umlaut,
   DKChr_Upper_O_Umlaut,
   DKChr_Upper_U_Umlaut,
   DKChr_Cyrillic_Upper_E,
   DKChr_Cyrillic_Upper_Yu,
   DKChr_Cyrillic_Upper_Ya,
   DKChr_Cyrillic_Lower_Ka,
   DKChr_Cyrillic_Lower_El,
   DKChr_Lower_a_Acute,
   DKChr_Lower_i_Acute,
   DKChr_Lower_o_Acute,
   DKChr_Lower_u_Acute,
   DKChr_Lower_n_Tilde,
   DKChr_Upper_N_Tilde,
   DKChr_Cyrillic_Lower_Em,
   DKChr_Cyrillic_Lower_En,
   DKChr_InvertedQuestion,
   DKChr_Cyrillic_Lower_Pe,
   DKChr_Cyrillic_Lower_HardSign,
   DKChr_Cyrillic_Lower_Yeru,
   DKChr_Cyrillic_Lower_SoftSign,
   DKChr_InvertedExclamation,
   DKChr_Cyrillic_Lower_E,
   DKChr_Cyrillic_Lower_Yu,
   DKChr_Cyrillic_Lower_Ya,
   DKChr_Cyrillic_Upper_Be,
   DKChr_Cyrillic_Upper_Ghe,
   DKChr_Cyrillic_Upper_De,
   DKChr_Cyrillic_Upper_Zhe,
   DKChr_Upper_A_Acute,
   DKChr_Upper_A_Circumflex,
   DKChr_Upper_A_Grave,
   DKChr_unused29,
   DKChr_unused30,
   DKChr_unused31,
   DKChr_unused32,
   DKChr_unused33,
   DKChr_unused34,
   DKChr_unused35,
   DKChr_unused36,
   DKChr_unused37,
   DKChr_unused38,
   DKChr_Cyrillic_Upper_Ze,
   DKChr_Cyrillic_Upper_I,
   DKChr_Cyrillic_Upper_ShortI,
   DKChr_Cyrillic_Upper_U,
   DKChr_Lower_a_Tilde,
   DKChr_Upper_A_Tilde,
   DKChr_Cyrillic_Upper_Ef,
   DKChr_Cyrillic_Upper_Tse,
   DKChr_Cyrillic_Upper_Che,
   DKChr_Cyrillic_Upper_Sha,
   DKChr_Cyrillic_Upper_Shcha,
   DKChr_Cyrillic_Lower_Be,
   DKChr_Cyrillic_Lower_Ve,
   DKChr_Cyrillic_Lower_Ghe,
   DKChr_Cyrillic_Lower_De,
   DKChr_Cyrillic_Lower_Zhe,
   DKChr_Upper_E_Circumflex,
   DKChr_Upper_E_Umlaut,
   DKChr_Upper_E_Grave,
   DKChr_Cyrillic_Lower_Ze,
   DKChr_Upper_I_Acute,
   DKChr_Upper_I_Circumflex,
   DKChr_Upper_I_Umlaut,
   DKChr_Cyrillic_Lower_I,
   DKChr_Cyrillic_Lower_ShortI,
   DKChr_Cyrillic_Lower_Te,
   DKChr_Cyrillic_Lower_Ef,
   DKChr_Cyrillic_Lower_Tse,
   DKChr_Upper_I_Grave,
   DKChr_Cyrillic_Lower_Che,
   DKChr_Upper_O_Acute,
   DKChr_Lower_SharpS,
   DKChr_Upper_O_Circumflex,
   DKChr_Upper_O_Grave,
   DKChr_Lower_o_Tilde,
   DKChr_Upper_O_Tilde,
   DKChr_Cyrillic_Lower_Sha,
   DKChr_Cyrillic_Lower_Shcha,
   DKChr_Cyrillic_Upper_Yeru,
   DKChr_Upper_U_Acute,
   DKChr_Upper_U_Circumflex,
   DKChr_Upper_U_Grave,
   DKChr_Lower_y_Acute,
   DKChr_Upper_Y_Acute,
   DKChr_Lower_a_Ogonek,
   DKChr_Upper_A_Ogonek,
   DKChr_Lower_c_Acute,
   DKChr_Upper_C_Acute,
   DKChr_Lower_e_Ogonek,
   DKChr_Upper_E_Ogonek,
   DKChr_Lower_l_Stroke,
   DKChr_Upper_L_Stroke,
   DKChr_Lower_n_Acute,
   DKChr_Upper_N_Acute,
   DKChr_Lower_s_Acute,
   DKChr_Upper_S_Acute,
   DKChr_Lower_z_Acute,
   DKChr_Upper_Z_Acute,
   DKChr_Lower_z_Dot,
   DKChr_Upper_Z_Dot,
   DKChr_unused39,
   DKChr_unused40,
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
  unsigned long narrow_width;
  unsigned long narrow_height;
  unsigned long bits_width;
  unsigned long bits_height;
  unsigned long narrow_spacing;
  unsigned long kana_spacing;
  unsigned long wide_spacing;
  unsigned long baseline_offset;
  unsigned long line_spacing;
};

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

extern short dbc_language;
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
int LbTextCharWidth(const long chr);
int LbTextCharWidthM(const long chr, long units_per_px);
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
int LbSprFontWordWidth(const struct TbSpriteSheet * font, const char * text);
int LbSprFontCharWidth(const struct TbSpriteSheet * font, const unsigned long chr);
int LbSprFontCharHeight(const struct TbSpriteSheet * font,const unsigned long chr);
const struct TbSprite * LbFontCharSprite(const struct TbSpriteSheet * font, const unsigned long chr);

void LbTextUseByteCoding(TbBool is_enabled);
long text_string_height(int units_per_px, const char *text);
void dbc_set_language(short ilng);
short dbc_initialize(const char *fpath);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
