/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_evnt.h
 *     Header file for frontmenu_ingame_evnt.c.
 * @par Purpose:
 *     In-game events GUI, visible during gameplay at bottom.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 03 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTMENU_INGAMEVNT_H
#define DK_FRONTMENU_INGAMEVNT_H

#include "globals.h"

#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct GuiMenu;
struct GuiButton;

#pragma pack()
/******************************************************************************/
DLLIMPORT struct GuiButtonTemplate _DK_text_info_buttons[];
DLLIMPORT struct GuiButtonTemplate _DK_battle_buttons[];
DLLIMPORT extern unsigned short _DK_battle_creature_over;
#define battle_creature_over _DK_battle_creature_over
/******************************************************************************/
extern struct GuiMenu text_info_menu;
extern struct GuiMenu battle_menu;
/******************************************************************************/
void gui_open_event(struct GuiButton *gbtn);
void gui_kill_event(struct GuiButton *gbtn);
void turn_on_event_info_panel_if_necessary(EventIndex evidx);
void activate_event_box(EventIndex evidx);

short zoom_to_fight(PlayerNumber plyr_idx);

void draw_bonus_timer(void);
TbBool bonus_timer_enabled(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
