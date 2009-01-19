/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_draw.h
 *     Header file for gui_draw.c.
 * @par Purpose:
 *     GUI elements drawing functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_GUIDRAW_H
#define DK_GUIDRAW_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern char gui_textbuf[TEXT_BUFFER_LENGTH];
/******************************************************************************/
DLLIMPORT extern struct TbSprite *_DK_button_sprite;
#define button_sprite _DK_button_sprite
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
