/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_saves.h
 *     Header file for frontmenu_saves.c.
 * @par Purpose:
 *     GUI menus for saved games support (save and load screens).
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTMENU_SAVES_H
#define DK_FRONTMENU_SAVES_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct GuiMenu;
struct GuiButton;

#pragma pack()
/******************************************************************************/
extern struct GuiMenu load_menu;
extern struct GuiMenu save_menu;
#define frontend_load_menu_items_visible  6
extern struct GuiMenu frontend_load_menu;
/******************************************************************************/
void gui_load_game(struct GuiButton *gbtn);
void gui_load_game_maintain(struct GuiButton *gbtn);
void draw_load_button(struct GuiButton *gbtn);
void gui_save_game(struct GuiButton *gbtn);
void frontend_load_game(struct GuiButton *gbtn);
void frontend_draw_load_game_button(struct GuiButton *gbtn);
void frontend_load_game_up(struct GuiButton *gbtn);
void frontend_load_game_down(struct GuiButton *gbtn);
void frontend_load_game_scroll(struct GuiButton *gbtn);
void frontend_load_game_up_maintain(struct GuiButton *gbtn);
void frontend_load_game_down_maintain(struct GuiButton *gbtn);
void frontend_load_game_maintain(struct GuiButton *gbtn);
void frontend_draw_games_scroll_tab(struct GuiButton *gbtn);
void init_load_menu(struct GuiMenu *gmnu);
void init_save_menu(struct GuiMenu *gmnu);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
