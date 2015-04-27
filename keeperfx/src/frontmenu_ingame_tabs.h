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

#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct GuiMenu;
struct GuiButton;

#define BID_INFO_TAB      1
#define BID_ROOM_TAB      2
#define BID_SPELL_TAB     3
#define BID_TRAP_TAB      4
#define BID_CREATR_TAB    5

struct InstanceButtonInit {  // sizeof=0x6
    long numfield_0;
    short tooltip_str_idx;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT long _DK_activity_list[24];
#define activity_list _DK_activity_list
DLLIMPORT struct TiledSprite _DK_status_panel;
DLLIMPORT char _DK_gui_room_type_highlighted;
#define gui_room_type_highlighted _DK_gui_room_type_highlighted
DLLIMPORT char _DK_gui_door_type_highlighted;
#define gui_door_type_highlighted _DK_gui_door_type_highlighted
DLLIMPORT char _DK_gui_trap_type_highlighted;
#define gui_trap_type_highlighted _DK_gui_trap_type_highlighted
DLLIMPORT char _DK_gui_creature_type_highlighted;
#define gui_creature_type_highlighted _DK_gui_creature_type_highlighted
DLLIMPORT struct InstanceButtonInit _DK_instance_button_init[48];
#define instance_button_init _DK_instance_button_init
DLLIMPORT unsigned long _DK_first_person_instance_top_half_selected;
#define first_person_instance_top_half_selected _DK_first_person_instance_top_half_selected
/******************************************************************************/
extern struct GuiMenu main_menu;
extern struct GuiMenu room_menu;
extern struct GuiMenu spell_menu;
extern struct GuiMenu spell_lost_menu;
extern struct GuiMenu trap_menu;
extern struct GuiMenu creature_menu;
extern struct GuiMenu event_menu;
extern struct GuiMenu query_menu;
extern struct GuiMenu creature_query_menu1;
extern struct GuiMenu creature_query_menu2;
extern struct GuiMenu creature_query_menu3;
extern struct GuiMenu creature_query_menu4;
extern struct TiledSprite status_panel;
/******************************************************************************/
void draw_whole_status_panel(void);
void gui_set_button_flashing(long btn_idx, long gameturns);

void update_room_tab_to_config(void);
void update_trap_tab_to_config(void);
void update_powers_tab_to_config(void);

void go_to_my_next_room_of_type_and_select(RoomKind rkind);
void go_to_my_next_room_of_type(RoomKind rkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
