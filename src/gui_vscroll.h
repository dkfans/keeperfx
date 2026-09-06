/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_vscroll.h
 *     Custom vertical scrollbar for the in-game Load/Save Game menus.
 * @par Purpose:
 *     Draws and drives a vertical scrollbar (built entirely from cut-outs of the
 *     std_l / std_r slot-bar sprites, using the in-game palette) for the in-game
 *     GMnu_LOAD / GMnu_SAVE screens, so more than 8 save slots can be reached with
 *     the mouse and mouse wheel.
 * @par Comment:
 *     Every piece of the scrollbar is a rectangular cut-out (mask) of the 
 *     std_l / std_r slot-bar sprites (button_sprites, gui1-64, in-game palette), 
 *     no new sprites are added. A cut-out is drawn by clipping the graphics window
 *     to the wanted rectangle and drawing the whole sprite shifted so that rectangle
 *     lands under the window. All offsets are in native (640x480 / units_per_px==16)
 *     pixels and scaled up by units_per_px.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_VSCROLL_H
#define DK_GUI_VSCROLL_H

#include "bflib_basics.h"
#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/** Number of savegame slots shown on screen at once in the in-game Load/Save menu. */
#define GUI_VSCROLL_VISIBLE 8

/** Current first-visible-slot offset of the in-game Load/Save list. */
extern long gui_vscroll_offset;

/** Total number of slots the in-game Load/Save list can currently scroll through
 *  (always at least GUI_VSCROLL_VISIBLE; grows to reveal one free slot past the used
 *  ones). */
long gui_vscroll_total(void);

/** Largest valid scroll offset (0 when nothing to scroll). */
long gui_vscroll_max_offset(void);

/** GUI button callbacks (wired up from frontmenu_saves_data.cpp). */
void gui_vscroll_draw(struct GuiButton *gbtn);      //< draw_call
void gui_vscroll_input(struct GuiButton *gbtn);     //< click_event
void gui_vscroll_maintain(struct GuiButton *gbtn);  //< maintain_call

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
/******************************************************************************/
