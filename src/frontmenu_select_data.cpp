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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GuiButtonInit frontend_select_level_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,     {107},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  82, 128,  82, 128,220, 26, frontend_draw_scroll_box_tab,      0, GUIStr_Empty,  0,      {28},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  82, 154,  82, 154,450,180, frontend_draw_scroll_box,          0, GUIStr_Empty,  0,      {26},            0, NULL},
  { 1,  0, 0, 0, frontend_level_select_up,NULL,frontend_over_button,  0, 532, 153, 532, 153, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {17},            0, frontend_level_select_up_maintain},
  { 1,  0, 0, 0, frontend_level_select_down,NULL,frontend_over_button,0, 532, 321, 532, 321, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {18},            0, frontend_level_select_down_maintain},
  { 1,  0, 0, 0, frontend_level_select_scroll,NULL,NULL,              0, 536, 167, 536, 167, 20,154, frontend_draw_levels_scroll_tab,   0, GUIStr_Empty,  0,      {40},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 102, 129, 102, 129,220, 26, frontend_draw_level_select_mappack,0, GUIStr_Empty,  0,      {32},            0, NULL},
  { 0,  0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 167,  95, 169,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {45},            0, frontend_level_select_maintain},
  { 0,  0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 189,  95, 191,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {46},            0, frontend_level_select_maintain},
  { 0,  0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 211,  95, 213,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {47},            0, frontend_level_select_maintain},
  { 0,  0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 233,  95, 235,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {48},            0, frontend_level_select_maintain},
  { 0,  0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 255,  95, 257,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {49},            0, frontend_level_select_maintain},
  { 0,  0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 277,  95, 279,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {50},            0, frontend_level_select_maintain},
  { 0,  0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 299,  95, 301,424, 22, frontend_draw_level_select_button, 0, GUIStr_Empty,  0,      {51},            0, frontend_level_select_maintain},
  { 0,  0, 0, 0, frontend_change_state,NULL,frontend_over_button,    34, 999, 404, 999, 404,371, 46, frontend_draw_variable_mappack_exit_button, 0, GUIStr_Empty,  0,     {111},   0, NULL},
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_select_campaign_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,     {108},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  82, 128,  82, 128,220, 26, frontend_draw_scroll_box_tab,      0, GUIStr_Empty,  0,      {28},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  82, 154,  82, 154,450,180, frontend_draw_scroll_box,          0, GUIStr_Empty,  0,      {26},            0, NULL},
  { 1,  0, 0, 0, frontend_campaign_select_up,NULL,frontend_over_button,0, 532,153, 532, 153, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {17},            0, frontend_campaign_select_up_maintain},
  { 1,  0, 0, 0, frontend_campaign_select_down,NULL,frontend_over_button,0,532,321,532, 321, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {18},            0, frontend_campaign_select_down_maintain},
  { 1,  0, 0, 0, frontend_campaign_select_scroll,NULL,NULL,           0, 536, 167, 536, 167, 20,154, frontend_draw_campaign_scroll_tab, 0, GUIStr_Empty,  0,      {40},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 102, 129, 102, 129,220, 26, frontend_draw_text,                0, GUIStr_Empty,  0,     {109},            0, NULL},
  { 0,  0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 167,  95, 169,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {45},            0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 189,  95, 191,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {46},            0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 211,  95, 213,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {47},            0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 233,  95, 235,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {48},            0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 255,  95, 257,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {49},            0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 277,  95, 279,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {50},            0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 299,  95, 301,424, 22, frontend_draw_campaign_select_button,0,GUIStr_Empty, 0,      {51},            0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,       {6},            0, NULL},
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_select_mappack_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,     {107},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  82, 128,  82, 128,220, 26, frontend_draw_scroll_box_tab,      0, GUIStr_Empty,  0,      {28},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  82, 154,  82, 154,450,180, frontend_draw_scroll_box,          0, GUIStr_Empty,  0,      {26},            0, NULL},
  { 1,  0, 0, 0, frontend_mappack_select_up,NULL,frontend_over_button,0, 532, 153, 532, 153, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {17},            0, frontend_mappack_select_up_maintain},
  { 1,  0, 0, 0, frontend_mappack_select_down,NULL,frontend_over_button,0,532,321,532,  321, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {18},            0, frontend_mappack_select_down_maintain},
  { 1,  0, 0, 0, frontend_mappack_select_scroll,NULL,NULL,            0, 536, 167, 536, 167, 20,154, frontend_draw_mappack_scroll_tab,  0, GUIStr_Empty,  0,      {40},            0, NULL},
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 102, 129, 102, 129,220, 26, frontend_draw_text,                0, GUIStr_Empty,  0,     {112},            0, NULL},
  { 0,  0, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 167,  95, 169,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {45},            0, frontend_mappack_select_maintain},
  { 0,  0, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 189,  95, 191,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {46},            0, frontend_mappack_select_maintain},
  { 0,  0, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 211,  95, 213,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {47},            0, frontend_mappack_select_maintain},
  { 0,  0, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 233,  95, 235,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {48},            0, frontend_mappack_select_maintain},
  { 0,  0, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 255,  95, 257,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {49},            0, frontend_mappack_select_maintain},
  { 0,  0, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 277,  95, 279,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {50},            0, frontend_mappack_select_maintain},
  { 0,  0, 0, 0, frontend_mappack_select,NULL,frontend_over_button,   0,  95, 299,  95, 301,424, 22, frontend_draw_mappack_select_button,0,GUIStr_Empty,  0,      {51},            0, frontend_mappack_select_maintain},
  { 0,  0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,       {6},            0, NULL},
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
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
