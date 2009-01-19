/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_draw.c
 *     GUI elements drawing functions.
 * @par Purpose:
 *     On-screen drawing of GUI elements, like buttons, menus and panels.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_draw.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char gui_textbuf[TEXT_BUFFER_LENGTH];
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
