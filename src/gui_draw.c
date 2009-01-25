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
DLLIMPORT void _DK_draw_button_string(struct GuiButton *gbtn, const char *text);
/******************************************************************************/

/******************************************************************************/

void draw_bar64k(long pos_x, long pos_y, long width)
{
  static const char *func_name="draw_bar64k";
  long body_end;
  long x;
  if (width < 72)
  {
    error(func_name, 5317, "Bar is too small");
    return;
  }
  // Button opening sprite
  LbSpriteDraw(pos_x/pixel_size, pos_y/pixel_size, &button_sprite[1]);
  // Button body
  body_end = pos_x + width - 64;
  for (x = pos_x+32; x<body_end; x+=32)
  {
      LbSpriteDraw(x/pixel_size, pos_y/pixel_size, &button_sprite[2]);
  }
  LbSpriteDraw(body_end/pixel_size, pos_y/pixel_size, &button_sprite[2]);
  // Button ending sprite
  LbSpriteDraw((pos_x + width - 32)/pixel_size, pos_y/pixel_size, &button_sprite[3]);
}

void draw_lit_bar64k(long pos_x, long pos_y, long width)
{
  static const char *func_name="draw_lit_bar64k";
  long body_end;
  long x;
  if (width < 32)
  {
    error(func_name, 5340, "Bar is too small");
    return;
  }
  // opening sprite
  LbSpriteDraw(pos_x/pixel_size, pos_y/pixel_size, &button_sprite[7]);
  // body
  body_end = pos_x+width-64;
  for (x = pos_x+32; x<body_end; x+=32)
  {
      LbSpriteDraw(x/pixel_size, pos_y/pixel_size, &button_sprite[8]);
  }
  LbSpriteDraw(body_end/pixel_size, pos_y/pixel_size, &button_sprite[8]);
  // ending sprite
  LbSpriteDraw((pos_x+width-32)/pixel_size, pos_y/pixel_size, &button_sprite[9]);
}

void draw_button_string(struct GuiButton *gbtn, const char *text)
{
  _DK_draw_button_string(gbtn, text);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
