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

enum IngameButtonDesignationIDs {
    BID_INFO_TAB = BID_DEFAULT+1,
    BID_ROOM_TAB,
    BID_SPELL_TAB,
    BID_MNFCT_TAB,
    BID_CREATR_TAB,//5
    BID_ROOM_TD01,
    BID_ROOM_TD02,
    BID_ROOM_TD03,
    BID_ROOM_TD04,
    BID_ROOM_TD05,//10
    BID_ROOM_TD06,
    BID_ROOM_TD07,
    BID_ROOM_TD08,
    BID_ROOM_TD09,
    BID_ROOM_TD10,//15
    BID_ROOM_TD11,
    BID_ROOM_TD12,
    BID_ROOM_TD13,
    BID_ROOM_TD14,
    //BID_ROOM_TD15, -- no such index
    BID_ROOM_TD16,//20
    BID_POWER_TD01,
    BID_POWER_TD02,
    BID_POWER_TD03,
    BID_POWER_TD04,
    BID_POWER_TD05,//25
    BID_POWER_TD06,
    BID_POWER_TD07,
    BID_POWER_TD08,
    BID_POWER_TD09,
    BID_POWER_TD10,//30
    BID_POWER_TD11,
    BID_POWER_TD12,
    BID_POWER_TD13,
    BID_POWER_TD14,
    BID_POWER_TD15,//35
    BID_POWER_TD16,
    BID_MAP_ZOOM_FS,
    BID_MAP_ZOOM_IN,
    BID_MAP_ZOOM_OU,
    BID_MSG_EV01,//40
    BID_MSG_EV02,
    BID_MSG_EV03,
    BID_MSG_EV04,
    BID_MSG_EV05,
    BID_MSG_EV06,//45
    BID_MSG_EV07,
    BID_MSG_EV08,
    BID_MSG_EV09,
    BID_MSG_EV10,
    BID_MSG_EV11,//50
    BID_MSG_EV12,
    BID_MSG_EV13,
    BID_MNFCT_TD01,
    BID_MNFCT_TD02,
    BID_MNFCT_TD03,//55
    BID_MNFCT_TD04,
    BID_MNFCT_TD05,
    BID_MNFCT_TD06,
    BID_MNFCT_TD07,
    BID_MNFCT_TD08,//60
    BID_MNFCT_TD09,
    BID_MNFCT_TD10,
    BID_MNFCT_TD11,
    BID_MNFCT_TD12,
    BID_MNFCT_TD13,//65
    BID_MNFCT_TD14,
    BID_MNFCT_TD15,
    BID_MNFCT_TD16,
    BID_QRY_IMPRSN,
    BID_QRY_FLEE,//70
    BID_QRY_BTN3,
    BID_CRTR_NXWNDR,
    BID_CRTR_NXWRKR,
    BID_CRTR_NXFIGT,
    BID_QUERY_INFO, //75
    BID_DUNGEON_INFO,
    BID_OPTIONS,
    BID_EVENT_ZOOM,
    BID_OBJ_CLOSE,
    BID_OBJ_SCRL_UP, //80
    BID_OBJ_SCRL_DWN,
    BID_MASTERSERVER_PUBLIC,
    BID_MASTERSERVER_REFRESH,
};

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
extern long activity_list[24];
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
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
