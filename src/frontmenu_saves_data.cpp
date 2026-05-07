/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_saves_data.cpp
 *     GUI menus for saved games support (save and load screens).
 * @par Purpose:
 *     Structures to show and maintain menus used for saving and loading.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     07 Dec 2012 - 11 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "frontmenu_options.h"
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
#include "frontmenu_options.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GuiButtonInit load_menu_buttons[] = {
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuLoad,0,       {0},               0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               0, 999,  58, 999,  58,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[0]}, 0, gui_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               1, 999,  90, 999,  90,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[1]}, 0, gui_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               2, 999, 122, 999, 122,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[2]}, 0, gui_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               3, 999, 154, 999, 154,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[3]}, 0, gui_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               4, 999, 186, 999, 186,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[4]}, 0, gui_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               5, 999, 218, 999, 218,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[5]}, 0, gui_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               6, 999, 250, 999, 250,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[6]}, 0, gui_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_load_game,      NULL,        NULL,               7, 999, 282, 999, 282,300, 32, draw_load_button,                  1, GUIStr_Empty,  0,{.str = input_string[7]}, 0, gui_load_game_maintain },
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,           0,       {0},               0, NULL },
};

struct GuiButtonInit save_menu_buttons[] = {
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuSave,0,       {0},               0, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               0, 999,  58, 999,  58,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[0]},SAVE_TEXTNAME_LEN, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               1, 999,  90, 999,  90,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[1]},SAVE_TEXTNAME_LEN, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               2, 999, 122, 999, 122,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[2]},SAVE_TEXTNAME_LEN, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               3, 999, 154, 999, 154,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[3]},SAVE_TEXTNAME_LEN, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               4, 999, 186, 999, 186,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[4]},SAVE_TEXTNAME_LEN, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               5, 999, 218, 999, 218,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[5]},SAVE_TEXTNAME_LEN, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               6, 999, 250, 999, 250,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[6]},SAVE_TEXTNAME_LEN, NULL },
  { 5, -2,-1, 1, gui_save_game,      NULL,        NULL,               7, 999, 282, 999, 282,300, 32, gui_area_text,                     1, GUIStr_Empty,  0,{.str = input_string[7]},SAVE_TEXTNAME_LEN, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,           0,       {0},               0, NULL },
};

struct GuiButtonInit frontend_load_menu_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,       {7},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, frontend_draw_scroll_box_tab,      0, GUIStr_Empty,  0,      {28},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,182, frontend_draw_scroll_box,          0, GUIStr_Empty,  0,      {26},            0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_load_game_up,NULL,frontend_over_button,     0, 532, 149, 532, 149, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {17},            0, frontend_load_game_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_load_game_down,NULL,frontend_over_button,   0, 532, 317, 532, 317, 26, 14, frontend_draw_slider_button,       0, GUIStr_Empty,  0,      {18},            0, frontend_load_game_down_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontend_load_game_scroll,NULL,  NULL,               0, 536, 163, 534, 163, 20,154, frontend_draw_games_scroll_tab,    0, GUIStr_Empty,  0,      {40},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 102, 125, 102, 125,220, 26, frontend_draw_text,                0, GUIStr_Empty,  0,      {30},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 157,  95, 157,424, 22, frontend_draw_load_game_button,    0, GUIStr_Empty,  0,      {45},            0, frontend_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 185,  95, 185,424, 22, frontend_draw_load_game_button,    0, GUIStr_Empty,  0,      {46},            0, frontend_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 213,  95, 213,424, 22, frontend_draw_load_game_button,    0, GUIStr_Empty,  0,      {47},            0, frontend_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 241,  95, 241,424, 22, frontend_draw_load_game_button,    0, GUIStr_Empty,  0,      {48},            0, frontend_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 269,  95, 269,424, 22, frontend_draw_load_game_button,    0, GUIStr_Empty,  0,      {49},            0, frontend_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 297,  95, 297,424, 22, frontend_draw_load_game_button,    0, GUIStr_Empty,  0,      {50},            0, frontend_load_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,       {6},            0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,           0,       {0},            0, NULL },
};

struct GuiMenu load_menu =
 {   GMnu_LOAD, 0, 4, load_menu_buttons,          POS_GAMECTR,POS_GAMECTR, 436, 350, gui_pretty_background, 0, NULL,    init_load_menu,          0, 1, 0,};
struct GuiMenu save_menu =
 {   GMnu_SAVE, 0, 4, save_menu_buttons,          POS_GAMECTR,POS_GAMECTR, 436, 350, gui_pretty_background, 0, NULL,    init_save_menu,          0, 1, 0,};
struct GuiMenu frontend_load_menu =
 { GMnu_FELOAD, 0, 1, frontend_load_menu_buttons,  POS_SCRCTR, POS_SCRCTR, 640, 480, NULL,                  0, NULL,    NULL,                    0, 0, 0,};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
