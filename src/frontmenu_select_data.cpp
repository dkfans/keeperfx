/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_select_data.cpp
 *     GUI menus for level and campaign select screens.
 * @par Purpose:
 *     Structures to show and maintain menus used for level and campaign list screens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     07 Dec 2012 - 11 Aug 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "frontmenu_select.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"

#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "frontmenu_saves.h"
#include "config_settings.h"
#include "config_strings.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GuiButtonInit frontend_select_level_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,     {107},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82,  88,  82,  88,220, 26, frontend_draw_scroll_box_tab,      0, GUIStr_Empty,  0,      {28},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 114,  82, 114,450,290, frontend_draw_scroll_box,          0, GUIStr_Empty,  0,      {26},            0, NULL},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_level_select_up,NULL,frontend_over_button,  0, 532, 113, 532, 113, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {17},            0, frontend_level_select_up_maintain},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_level_select_down,NULL,frontend_over_button,0, 532, 391, 532, 391, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {18},            0, frontend_level_select_down_maintain},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_level_select_scroll,NULL,NULL,              0, 536, 127, 536, 127, 20,264, frontend_draw_levels_scroll_tab,   0, GUIStr_Empty,  0,      {40},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 102,  89, 102,  89,220, 26, frontend_draw_level_select_mappack,0, GUIStr_Empty,  0,      {32},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 127,  95, 129,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {45},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 149,  95, 151,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {46},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 171,  95, 173,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {47},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 193,  95, 195,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {48},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 215,  95, 217,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {49},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 237,  95, 239,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {50},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 259,  95, 261,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {51},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 281,  95, 283,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {52},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 303,  95, 305,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {53},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 325,  95, 327,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {54},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 347,  95, 349,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {55},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 369,  95, 371,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {56},            0, frontend_level_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_change_state,NULL,frontend_over_button,    34, 999, 420, 999, 420,371, 46, frontend_draw_variable_mappack_exit_button, 0, GUIStr_Empty,  0,     {111},   0, NULL},
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_select_campaign_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,     {108},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82,  88,  82,  88,220, 26, frontend_draw_scroll_box_tab,      0, GUIStr_Empty,  0,      {28},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 114,  82, 114,450,290, frontend_draw_scroll_box,          0, GUIStr_Empty,  0,      {26},            0, NULL},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_campaign_select_up,NULL,frontend_over_button,0, 532,113, 532, 113, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {17},            0, frontend_campaign_select_up_maintain},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_campaign_select_down,NULL,frontend_over_button,0,532,391,532, 391, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {18},            0, frontend_campaign_select_down_maintain},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_campaign_select_scroll,NULL,NULL,           0, 536, 127, 536, 127, 20,264, frontend_draw_campaign_scroll_tab, 0, GUIStr_Empty,  0,      {40},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 102,  89, 102,  89,220, 26, frontend_draw_text,                0, GUIStr_Empty,  0,     {109},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 127,  95, 129,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {45},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 149,  95, 151,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {46},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 171,  95, 173,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {47},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 193,  95, 195,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {48},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 215,  95, 217,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {49},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 237,  95, 239,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {50},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 259,  95, 261,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {51},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 281,  95, 283,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {52},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 303,  95, 305,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {53},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 325,  95, 327,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {54},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 347,  95, 349,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {55},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 369,  95, 371,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {56},            0, frontend_campaign_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 420, 999, 420,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,       {6},            0, NULL},
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_select_mappack_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,     {107},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82,  88,  82,  88,220, 26, frontend_draw_scroll_box_tab,      0, GUIStr_Empty,  0,      {28},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 114,  82, 114,450,290, frontend_draw_scroll_box,          0, GUIStr_Empty,  0,      {26},            0, NULL},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_mappack_select_up,NULL,frontend_over_button,0, 532, 113, 532, 113, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {17},            0, frontend_mappack_select_up_maintain},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_mappack_select_down,NULL,frontend_over_button,0,532,391,532,  391, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {18},            0, frontend_mappack_select_down_maintain},
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_mappack_select_scroll,NULL,NULL,            0, 536, 127, 536, 127, 20,264, frontend_draw_mappack_scroll_tab,  0, GUIStr_Empty,  0,      {40},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 102,  89, 102,  89,220, 26, frontend_draw_text,                0, GUIStr_Empty,  0,     {112},            0, NULL},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 127,  95, 129,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {45},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 149,  95, 151,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {46},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 171,  95, 173,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {47},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 193,  95, 195,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {48},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 215,  95, 217,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {49},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 237,  95, 239,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {50},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 259,  95, 261,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {51},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 281,  95, 283,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {52},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 303,  95, 305,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {53},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 325,  95, 327,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {54},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 347,  95, 349,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {55},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 369,  95, 371,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {56},            0, frontend_mappack_select_maintain},
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 420, 999, 420,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,       {6},            0, NULL},
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiMenu frontend_select_mappack_menu =
 { GMnu_MAPPACK_SELECT,     0, 1, frontend_select_mappack_buttons,   POS_SCRCTR, POS_SCRCTR, 640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
 struct GuiMenu frontend_select_level_menu =
 { GMnu_FELEVEL_SELECT,     0, 1, frontend_select_level_buttons,   POS_SCRCTR, POS_SCRCTR, 640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_select_campaign_menu =
 { GMnu_FECAMPAIGN_SELECT,  0, 1, frontend_select_campaign_buttons,POS_SCRCTR, POS_SCRCTR, 640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
