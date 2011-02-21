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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct GuiMenu;
struct GuiButton;

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT struct GuiButtonInit _DK_text_info_buttons[];
DLLIMPORT struct GuiButtonInit _DK_battle_buttons[];
/******************************************************************************/
void gui_open_event(struct GuiButton *gbtn);
void gui_kill_event(struct GuiButton *gbtn);
void turn_on_event_info_panel_if_necessary(unsigned short evnt_idx);
void activate_event_box(long evnt_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
