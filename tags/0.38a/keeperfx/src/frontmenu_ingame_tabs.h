/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_tabs.h
 *     Header file for frontmenu_ingame_tabs.c.
 * @par Purpose:
 *     Main in-game GUI, visible during gameplay.
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
#ifndef DK_FRONTMENU_INGAMETABS_H
#define DK_FRONTMENU_INGAMETABS_H

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

#define BID_INFO_TAB      1
#define BID_ROOM_TAB      2
#define BID_SPELL_TAB     3
#define BID_TRAP_TAB      4
#define BID_CREATR_TAB    5


DLLIMPORT long _DK_activity_list[24];
#define activity_list _DK_activity_list

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT struct GuiButtonInit _DK_main_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_room_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_spell_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_spell_lost_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_trap_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_creature_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_query_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_creature_query_buttons1[];
DLLIMPORT struct GuiButtonInit _DK_creature_query_buttons2[];
DLLIMPORT struct GuiButtonInit _DK_creature_query_buttons3[];
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
