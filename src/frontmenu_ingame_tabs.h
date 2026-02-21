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

enum IngameButtonGroupIDs {
    GID_NONE = 0,
    GID_MINIMAP_AREA,
    GID_TABS_AREA,
    GID_INFO_PANE,
    GID_ROOM_PANE,
    GID_POWER_PANE,
    GID_TRAP_PANE,
    GID_DOOR_PANE,
    GID_CREATR_PANE,
    GID_MESSAGE_AREA,
};

/******************************************************************************/
extern int32_t activity_list[24];
extern char gui_room_type_highlighted;
extern char gui_door_type_highlighted;
extern char gui_trap_type_highlighted;
extern char gui_creature_type_highlighted;
extern unsigned long first_person_instance_top_half_selected;

#pragma pack()
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
extern struct Around const draw_square[];
extern struct GuiMenu spell_menu2;
extern struct GuiMenu room_menu2;
extern struct GuiMenu trap_menu2;

#define AROUND_2x2_PIXEL      4
#define AROUND_3x3_PIXEL      9
#define AROUND_4x4_PIXEL      16
#define AROUND_5x5_PIXEL      25
#define AROUND_6x6_PIXEL      36

#define ONE_PIXEL       2048
#define TWO_PIXELS      1024
#define THREE_PIXELS     512
#define FOUR_PIXELS      256

extern const short pixels_needed[];

/******************************************************************************/
short get_pixels_scaled_and_zoomed(long basic_zoom);
short scale_pixel(long basic_zoom);
void draw_whole_status_panel(void);
void gui_set_button_flashing(long btn_idx, long gameturns);
short button_designation_to_tab_designation(short btn_designt_id);
short get_button_designation(short btn_group, short btn_item);
void draw_placefiller(long scr_x, long scr_y, long units_per_px);

void gui_over_creature_button(struct GuiButton* gbtn);

void update_room_tab_to_config(void);
void update_trap_tab_to_config(void);
void update_powers_tab_to_config(void);

void go_to_my_next_room_of_type_and_select(RoomKind rkind);
void go_to_my_next_room_of_type(RoomKind rkind);
RoomIndex find_my_next_room_of_type(RoomKind rkind);
RoomIndex find_next_room_of_type(PlayerNumber plyr_idx, RoomKind rkind);

void gui_query_next_creature_of_owner_and_model(struct GuiButton *gbtn);
void gui_query_next_creature_of_owner(struct GuiButton *gbtn);

void maintain_spell_next_page_button(struct GuiButton *gbtn);
void maintain_room_next_page_button(struct GuiButton *gbtn);
void maintain_trap_next_page_button(struct GuiButton *gbtn);
void gui_switch_players_visible(struct GuiButton* gbtn);

void go_to_adjacent_menu_tab(int direction);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
