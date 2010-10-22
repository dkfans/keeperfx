/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_frontbtns.h
 *     Header file for gui_frontbtns.c.
 * @par Purpose:
 *     gui_frontbtns functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_FRONTBTNS_H
#define DK_GUI_FRONTBTNS_H

#include "globals.h"
#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif


#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
void gui_clear_buttons_not_over_mouse(int gmbtn_idx);
TbBool gui_button_release_inputs(int gmbtn_idx);
TbBool gui_slider_button_inputs(int gbtn_idx);
TbBool gui_button_click_inputs(int gmbtn_idx);

void kill_button_area_input(void);
void kill_button(struct GuiButton *gbtn);
void setup_radio_buttons(struct GuiMenu *gmnu);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
