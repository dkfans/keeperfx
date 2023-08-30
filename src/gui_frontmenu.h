/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_frontmenu.h
 *     Header file for gui_frontmenu.c.
 * @par Purpose:
 *     GUI Menus support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     28 May 2010 - 12 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_FRONTMENU_H
#define DK_GUI_FRONTMENU_H

#include "globals.h"
#include "bflib_guibtns.h"

#define ACTIVE_MENUS_COUNT           8

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

enum GUI_Menus {
  GMnu_MAIN               =  1,
  GMnu_ROOM               =  2,
  GMnu_SPELL              =  3,
  GMnu_TRAP               =  4,
  GMnu_CREATURE           =  5,
  GMnu_EVENT              =  6,
  GMnu_QUERY              =  7,
  GMnu_OPTIONS            =  8,
  GMnu_INSTANCE           =  9,
  GMnu_QUIT               = 10,
  GMnu_LOAD               = 11,
  GMnu_SAVE               = 12,
  GMnu_VIDEO              = 13,
  GMnu_SOUND              = 14,
  GMnu_ERROR_BOX          = 15,
  GMnu_TEXT_INFO          = 16,
  GMnu_HOLD_AUDIENCE      = 17,
  GMnu_FEMAIN             = 18,
  GMnu_FELOAD             = 19,
  GMnu_FENET_SERVICE      = 20,
  GMnu_FENET_SESSION      = 21,
  GMnu_FENET_START        = 22,
  GMnu_FESTATISTICS       = 25,
  GMnu_FEHIGH_SCORE_TABLE = 26,
  GMnu_DUNGEON_SPECIAL    = 27,
  GMnu_RESURRECT_CREATURE = 28,
  GMnu_TRANSFER_CREATURE  = 29,
  GMnu_ARMAGEDDON         = 30,
  GMnu_CREATURE_QUERY1    = 31,
  GMnu_CREATURE_QUERY3    = 32,
  GMnu_CREATURE_QUERY4    = 33,
  GMnu_BATTLE             = 34,
  GMnu_CREATURE_QUERY2    = 35,
  GMnu_FEDEFINE_KEYS      = 36,
  GMnu_AUTOPILOT          = 37,
  GMnu_SPELL_LOST         = 38,
  GMnu_FEOPTION           = 39,
  GMnu_FELEVEL_SELECT     = 40,
  GMnu_FECAMPAIGN_SELECT  = 41,
  GMnu_FEERROR_BOX        = 42,
  GMnu_FEADD_SESSION      = 43,
  GMnu_MAPPACK_SELECT     = 44,
  GMnu_MSG_BOX            = 45,
};

#define MENU_INVALID_ID -1
/******************************************************************************/
#pragma pack(1)

struct GuiMenu;
struct GuiButton;

/**
 * Type to store GMnu_* items from GUI_Menus enumeration.
 */
typedef long MenuID;

/**
 * Type to store menu number.
 */
typedef long MenuNumber;

/******************************************************************************/

extern char no_of_active_menus;
extern unsigned char menu_stack[ACTIVE_MENUS_COUNT];
extern struct GuiMenu active_menus[ACTIVE_MENUS_COUNT];

#pragma pack()
/******************************************************************************/
struct GuiMenu *get_active_menu(MenuNumber num);
MenuNumber menu_id_to_number(MenuID menu_id);
int first_monopoly_menu(void);
void update_busy_doing_gui_on_menu(void);

void turn_on_menu(MenuID idx);
void turn_off_menu(MenuID mnu_idx);
void turn_off_query_menus(void);
void turn_off_all_menus(void);
short turn_off_all_window_menus(void);
short turn_off_all_bottom_menus(void);
void turn_on_main_panel_menu(void);
void turn_off_all_panel_menus(void);
void set_menu_mode(long mnu_idx);
void set_menu_visible_on(MenuID menu_id);
void set_menu_visible_off(MenuID menu_id);
void turn_off_event_box_if_necessary(PlayerNumber plyr_idx, unsigned char event_idx);

void kill_menu(struct GuiMenu *gmnu);
void remove_from_menu_stack(short mnu_id);
void add_to_menu_stack(unsigned char mnu_idx);
long first_available_menu(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
