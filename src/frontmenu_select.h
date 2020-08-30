/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_select.h
 *     Header file for frontmenu_select.c.
 * @par Purpose:
 *     GUI menus for level and campaign select screens.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     07 Dec 2012 - 11 Aug 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTMENU_SELECT_H
#define DK_FRONTMENU_SELECT_H

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
#define frontend_select_level_items_max_visible  7
extern struct GuiMenu frontend_select_level_menu;
#define frontend_select_campaign_items_max_visible  7
extern struct GuiMenu frontend_select_campaign_menu;
#define frontend_select_mappack_items_max_visible  7
extern struct GuiMenu frontend_select_mappack_menu;
/******************************************************************************/
// Level list selection screen
void frontend_draw_levels_scroll_tab(struct GuiButton *gbtn);
void frontend_draw_level_select_button(struct GuiButton *gbtn);
void frontend_level_select(struct GuiButton *gbtn);
void frontend_level_select_up(struct GuiButton *gbtn);
void frontend_level_select_down(struct GuiButton *gbtn);
void frontend_level_select_scroll(struct GuiButton *gbtn);
void frontend_level_select_up_maintain(struct GuiButton *gbtn);
void frontend_level_select_down_maintain(struct GuiButton *gbtn);
void frontend_level_select_maintain(struct GuiButton *gbtn);
void frontend_level_list_load(void);
void frontend_level_list_unload(void);
void frontend_level_select_update(void);
void frontend_draw_level_select_mappack(struct GuiButton *gbtn);

// Campaign selection screen
void frontend_campaign_select_up(struct GuiButton *gbtn);
void frontend_campaign_select_down(struct GuiButton *gbtn);
void frontend_campaign_select_scroll(struct GuiButton *gbtn);
void frontend_campaign_select_up_maintain(struct GuiButton *gbtn);
void frontend_campaign_select_down_maintain(struct GuiButton *gbtn);
void frontend_campaign_select_maintain(struct GuiButton *gbtn);
void frontend_draw_campaign_select_button(struct GuiButton *gbtn);
void frontend_campaign_select(struct GuiButton *gbtn);
void frontend_campaign_select_update(void);
void frontend_draw_campaign_scroll_tab(struct GuiButton *gbtn);
void frontend_campaign_list_load(void);

// Map pack selection screen
void frontend_mappack_select_up(struct GuiButton *gbtn);
void frontend_mappack_select_down(struct GuiButton *gbtn);
void frontend_mappack_select_scroll(struct GuiButton *gbtn);
void frontend_mappack_select_up_maintain(struct GuiButton *gbtn);
void frontend_mappack_select_down_maintain(struct GuiButton *gbtn);
void frontend_mappack_select_maintain(struct GuiButton *gbtn);
void frontend_draw_mappack_select_button(struct GuiButton *gbtn);
void frontend_mappack_select(struct GuiButton *gbtn);
void frontend_mappack_select_update(void);
void frontend_draw_mappack_scroll_tab(struct GuiButton *gbtn);
void frontend_mappack_list_load(void);
void frontend_draw_variable_mappack_exit_button(struct GuiButton *gbtn);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
