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

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct TbSprite;
struct TbSetupSprite;


/******************************************************************************/
DLLIMPORT extern struct TbSprite *_DK_lbFontPtr;
#define lbFontPtr _DK_lbFontPtr

#pragma pack()
/******************************************************************************/
int LbTextDraw(int posx, int posy, const char *text);
int LbTextHeight(const char *text);

/*
char __fastcall font_height(const unsigned char c);
unsigned long __fastcall my_string_width(const char *str);
*/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
