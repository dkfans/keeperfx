/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_net_data.cpp
 *     GUI menus for network support.
 * @par Purpose:
 *     Structures to show and maintain network screens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 15 Jun 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "frontmenu_net.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_netsp.hpp"
#include "bflib_datetm.h"
#include "bflib_guibtns.h"
#include "bflib_video.h"
#include "vidmode.h"
#include "vidfade.h"
#include "bflib_vidraw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"

#include "config_strings.h"
#include "front_network.h"
#include "front_input.h"
#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "front_landview.h"
#include "net_game.h"
#include "net_lobby.h"
#include "sprites.h"
#include "custom_sprites.h"
#include "bflib_inputctrl.h"
#include "kjm_input.h"
#include "gui_frontmenu.h"
#include <ctype.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GuiButtonInit frontend_net_service_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,      {10},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, frontnet_draw_scroll_box_tab,      0, GUIStr_Empty, 0,      {12},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,180, frontnet_draw_scroll_box,          0, GUIStr_Empty, 0,      {26},            0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_service_up,NULL,frontend_over_button,       0, 532, 149, 532, 149, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {17},            0, frontnet_service_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_service_down,NULL,frontend_over_button,     0, 532, 317, 532, 317, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {18},            0, frontnet_service_down_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 536, 163, 536, 163, 20,154, frontnet_draw_services_scroll_tab, 0, GUIStr_Empty, 0,      {40},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 102, 125, 102, 125,220, 26, frontend_draw_text,                0, GUIStr_Empty, 0,      {33},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 158,  95, 158,424, 26, frontnet_draw_service_button,      0, GUIStr_Empty, 0,      {45},            0, frontnet_service_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 184,  95, 184,424, 26, frontnet_draw_service_button,      0, GUIStr_Empty, 0,      {46},            0, frontnet_service_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 210,  95, 210,424, 26, frontnet_draw_service_button,      0, GUIStr_Empty, 0,      {47},            0, frontnet_service_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 236,  95, 236,424, 26, frontnet_draw_service_button,      0, GUIStr_Empty, 0,      {48},            0, frontnet_service_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 262,  95, 262,424, 26, frontnet_draw_service_button,      0, GUIStr_Empty, 0,      {49},            0, frontnet_service_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 288,  95, 288,424, 26, frontnet_draw_service_button,      0, GUIStr_Empty, 0,      {50},            0, frontnet_service_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,       {6},            0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 0, 455, 0, 455,371,46, frontend_draw_product_version, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,           0,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_net_session_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,        NULL,               0, 999,  12, 999,  12,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,      {12},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  61, 999,  61,371, 29, frontnet_draw_text_bar,            0, GUIStr_Empty, 0,      {27},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 147,  63, 147,  63, 83, 25, frontend_draw_text,                0, GUIStr_Empty, 0,      {19},            0, NULL },
  { 5, -1,-1, 0, frontnet_session_set_player_name,NULL,frontend_over_button,19,243,63,147,63,345, 26, frontend_draw_enter_text,          0, GUIStr_Empty, 0,{.str = tmp_net_player_name}, 20, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 19, 128, 19, 128,578,26, frontnet_draw_lobby_columns, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  10, 154,  10, 154,594,180, frontnet_draw_lobby_panel,         0, GUIStr_Empty, 0,      {26},            0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_session_up,NULL,       frontend_over_button,0, 604, 153, 604, 153, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {17},            0, frontnet_session_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_session_down,NULL,     frontend_over_button,0, 604, 321, 604, 321, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {18},            0, frontnet_session_down_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 608, 167, 608, 167, 20,154, frontnet_draw_sessions_scroll_tab, 0, GUIStr_Empty, 0,      {40},            0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,158,19,158,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {45}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,182,19,182,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {46}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,206,19,206,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {47}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,230,19,230,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {48}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,254,19,254,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {49}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,278,19,278,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {50}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,302,19,302,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {51}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_session_add,NULL,   frontend_over_button,0,999,358,999,358,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,      {14},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_return_to_main_menu,NULL,frontend_over_button,0,999,404,999,404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,       {6},            0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 0, 455, 0, 455,371,46, frontend_draw_product_version, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, LbBFeF_NoMouseOver, 19, 158, 19, 158,578,168, frontnet_draw_lobby_tooltip, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,           0,  0,       {0},            0, NULL },
};


struct GuiButtonInit frontend_net_start_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,   NULL,                    0, 999,  30, 999,  30, 371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,  {11},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  82,  78,  82,  78, 220, 26, frontnet_draw_scroll_box_tab,      0, GUIStr_Empty, 0,  {28},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, 421,  81, 421,  81, 100, 27, frontnet_draw_alliance_box_tab,    0, GUIStr_Empty, 0,  {28},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  82, 104,  82, 104, 450, 70, frontnet_draw_scroll_box,          0, GUIStr_Empty, 0,  {90},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, 102,  79, 102,  79, 220, 26, frontend_draw_text,                0, GUIStr_Empty, 0,  {31},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  95, 105,  82, 105, 432,104, frontnet_draw_net_start_players,   0, GUIStr_Empty, 0,  {21},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 107, 431, 116, 432, 88, frontnet_draw_alliance_grid,       0, GUIStr_Empty, 0,  {74},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 108, 431, 108,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {74},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 108, 453, 108,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {74},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 108, 475, 108,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {74},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 108, 497, 108,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {74},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 134, 431, 134,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {75},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 134, 453, 134,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {75},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 134, 475, 134,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {75},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 134, 497, 134,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {75},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 160, 431, 160,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {76},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 160, 453, 160,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {76},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 160, 475, 160,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {76},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 160, 497, 160,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {76},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 186, 431, 183,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {77},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 186, 453, 186,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {77},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 186, 475, 186,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {77},    0, frontnet_maintain_alliance },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 186, 497, 186,  22, 26, frontnet_draw_alliance_button,     0, GUIStr_Empty, 0,  {77},    0, frontnet_maintain_alliance },
  
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, NULL,                    NULL,   NULL,                     0,305,217,305,217,220,26, frontnet_draw_bottom_scroll_box_tab,0,GUIStr_Empty, 0,  {28},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_toggle_computer_players,NULL,frontend_over_button,0,315,214,315,214,200,26, frontend_draw_computer_players,    0, GUIStr_Empty, 0, {103},    0, NULL },
  
  
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, NULL,               NULL,  NULL,                     0, 85,217,85,217, 220, 26, frontnet_draw_bottom_scroll_box_tab,0,GUIStr_Empty, 0,  {28},    0, NULL }, //pack select background
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontend_load_mp_mappacks,NULL,frontend_over_button,0, 95,214,95,214, 200, 26, frontend_draw_mp_mappack,           0,GUIStr_Empty, 0, {100},    0, NULL }, //pack select
 
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  82, 246,  82, 246, 220, 26, frontnet_draw_scroll_box_tab,      0, GUIStr_Empty, 0,  {28},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  82, 272,  82, 272, 450,111, frontnet_draw_scroll_box,          0, GUIStr_Empty, 0,  {91},    0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_messages_up,NULL,  frontend_over_button,    0, 532, 271, 532, 271,  26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,  {38},    0, frontnet_messages_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_messages_down,NULL,frontend_over_button,    0, 532, 373, 532, 373,  26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,  {39},    0, frontnet_messages_down_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, 102, 247, 102, 247, 220, 26, frontend_draw_text,                0, GUIStr_Empty, 0,  {34},    0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, 536, 285, 536, 285,  20, 88, frontnet_draw_messages_scroll_tab, 0, GUIStr_Empty, 0,  {40},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  82, 386,  82, 386, 459, 28, frontnet_draw_current_message,     0, GUIStr_Empty, 0,  {43},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  89, 273,  89, 273, 438,104, frontnet_draw_messages,            0, GUIStr_Empty, 0,  {44},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, set_packet_start,   NULL,   frontend_over_button,    0,  49, 412,  49, 412, 247, 46, frontend_draw_small_menu_button,   0, GUIStr_Empty, 0,  {15},    0, frontnet_start_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_return_to_session_menu,NULL,frontend_over_button,1, 345,412,345,412,247,46, frontend_draw_small_menu_button,   0, GUIStr_Empty, 0,  {16},    0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 0, 455, 0, 455,371,46, frontend_draw_product_version, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,           0,  0,   {0},    0, NULL },
};

char net_lobby_name[64];

struct GuiButtonInit frontend_add_session_buttons[] = {
  { LbBtnT_NormalBtn, BID_MENU_TITLE, 0, 0, NULL, NULL, NULL, 0, 999, 30, 999, 30,371,46, frontend_draw_large_menu_button, 0, GUIStr_Empty, 0, {14}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 95,124,95,124,220,26, frontnet_draw_scroll_box_tab, 0, GUIStr_Empty, 0, {28}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 95,150,95,150,450,111, frontnet_draw_scroll_box, 0, GUIStr_Empty, 0, {90}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 3, 115,125,115,125,200,26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0, {28}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 115,164,115,164,187,26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0, {19}, 0, NULL },
  { LbBtnT_EditBox, -1,-1, 0, frontnet_session_set_lobby_name, NULL, frontend_over_button, 19,320,164,320,164,211,26, frontend_draw_enter_text, 0, GUIStr_Empty, 0, {.str = net_lobby_name}, 63, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_change_lobby_option, frontnet_change_lobby_option, frontend_over_button, 1, 115,196,115,196,416,26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0, {20}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 2, 115,228,115,228,416,26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0, {21}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_create, NULL, frontend_over_button, 0,72,310,72,310,247,46, frontend_draw_small_menu_button, 0, GUIStr_Empty, 0, {117}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_add_session_back, NULL, frontend_over_button, 0,321,310,321,310,247,46, frontend_draw_small_menu_button, 0, GUIStr_Empty, 0, {16}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 0, 455, 0, 455,371,46, frontend_draw_product_version, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  {-1, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0,0,0,0,0,0,0, NULL, 0,0,0,{0},0,NULL },
};

struct GuiMenu frontend_net_service_menu =
 { GMnu_FENET_SERVICE, 0, 1, frontend_net_service_buttons, POS_SCRCTR, POS_SCRCTR,  640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_session_menu =
 { GMnu_FENET_SESSION, 0, 1, frontend_net_session_buttons, POS_SCRCTR, POS_SCRCTR,  640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_start_menu =
 { GMnu_FENET_START,   0, 1, frontend_net_start_buttons,   POS_SCRCTR, POS_SCRCTR,  640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_add_session_box =
 { GMnu_FEADD_SESSION, 0, 1, frontend_add_session_buttons, POS_SCRCTR, POS_SCRCTR,  640, 480, NULL, 0, NULL,    NULL,                    0, 1, 0,};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void frontnet_draw_session_selected(struct GuiButton *gbtn)
{
    const struct TbSprite *spr;
    long pos_x;
    long pos_y;
    int i;
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    int fs_units_per_px;
    fs_units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, GFS_largearea_xts_tx1_c, 100);
    spr = get_frontend_sprite(GFS_largearea_xts_cor_l);
    for (i=0; i < 6; i++)
    {
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
        spr++;
    }
    if (net_session_index_active >= 0)
    {
        const char *text;
        text = net_session[net_session_index_active]->text;
        i = frontend_button_caption_font(gbtn, 0);
        if (text != NULL)
        {
            RendererSetDrawFlags(0);
            LbTextSetFont(frontend_font[i]);
            // Set drawing window and draw the text
            int tx_units_per_px;
            tx_units_per_px = (gbtn->height*13/14) * 16 / LbTextLineHeight();
            int h;
            h = LbTextLineHeight()*tx_units_per_px/16;
            LbTextSetWindow(gbtn->scr_pos_x + 13*fs_units_per_px/16, gbtn->scr_pos_y, gbtn->width - 26*fs_units_per_px/16, h);
            LbTextDrawResized(0, 0, tx_units_per_px, text);
        }
    }
}

static void draw_lobby_text(int x, int y, int width, int height, int font, const char *text)
{
    LbTextSetFont(frontend_font[font]);
    RendererSetDrawFlags(0);
    int scale = height * 16 / LbTextLineHeight();
    LbTextSetWindow(x, y, width, height);
    LbTextDrawResized(0, (height - LbTextLineHeight() * scale / 16) / 2, scale, text);
}

static const int lobby_columns[] = {10, 277, 377, 478, 578};
static int lobby_tooltip_index = -1;

static unsigned char *get_frontend_lobby_glass_map(void)
{
    static unsigned char *glass_map = [] {
        static struct TbColorTables tables;
        compute_fade_tables(&tables, frontend_palette, frontend_palette);
        return tables.ghost;
    }();
    return glass_map;
}

static void draw_lobby_columns(struct GuiButton *gbtn, int font, const char *const text[4])
{
    if (RendererGetActiveType() == RENDERER_SOFTWARE) {
        lbDisplay.GlassMap = get_frontend_lobby_glass_map();
    }
    int thickness = max(1, gbtn->width / lobby_columns[4]);
    int height = 26 * gbtn->width / lobby_columns[4];
    int y = gbtn->scr_pos_y + (gbtn->height - height) / 2;
    int separator_y = gbtn->scr_pos_y + gbtn->height - thickness;
    int border_inset = 2 * gbtn->width / lobby_columns[4];
    int separator_x = gbtn->scr_pos_x - 9 * gbtn->width / lobby_columns[4] + border_inset;
    int separator_width = 594 * gbtn->width / lobby_columns[4] - 2 * border_inset;
    int panel_y = gbtn->scr_pos_y + 2 * thickness;
    if (gbtn->content.lval == 45) {
        panel_y -= 3 * thickness;
    }
    RendererSetDrawFlags(Lb_SPRITE_TRANSPAR8);
    LbDrawBox(separator_x, panel_y, separator_width, separator_y - panel_y, 37);
    RendererSetDrawFlags(0);
    for (int i = 0; i < 4; i++) {
        int x = gbtn->scr_pos_x + lobby_columns[i] * gbtn->width / lobby_columns[4];
        int width = (lobby_columns[i + 1] - lobby_columns[i] - 12) * gbtn->width / lobby_columns[4];
        draw_lobby_text(x, y, width, height, font, text[i]);
    }
    RendererSetDrawFlags(0);
    LbDrawBox(separator_x, separator_y, separator_width, thickness, 6);
    LbDrawBox(separator_x, separator_y + thickness, separator_width, thickness, 105);
    RendererSetDrawFlags(Lb_SPRITE_TRANSPAR8);
    LbDrawBox(separator_x, separator_y + 2 * thickness, separator_width, thickness, 37);
    LbDrawBox(separator_x, separator_y + 3 * thickness, separator_width, thickness, 0);
    RendererSetDrawFlags(0);
    lbDisplay.GlassMap = pixmap.ghost;
}

void frontnet_draw_lobby_columns(struct GuiButton *gbtn)
{
    lobby_tooltip_index = -1;
    const char *text[] = {get_string(GUIStr_NetSessions), get_string(GUIStr_NetState), get_string(GUIStr_MnuPlayers), get_string(GUIStr_NetVersion)};
    const struct TbSprite *left = get_frontend_sprite(GFS_hugearea_thc_cor_tl);
    const struct TbSprite *middle = get_frontend_sprite(GFS_hugearea_thc_tx1_tc);
    const struct TbSprite *right = get_frontend_sprite(GFS_hugearea_thc_cor_tr);
    int scale = simple_frontend_sprite_height_units_per_px(gbtn, GFS_hugearea_thc_tx1_tc, 100);
    int left_width = left->SWidth * scale / 16;
    int right_width = right->SWidth * scale / 16;
    int padding = 19 * gbtn->width / lobby_columns[4];
    RendererSetDrawFlags(0);
    for (int i = 0; i < 4; i++) {
        int x = gbtn->scr_pos_x + lobby_columns[i] * gbtn->width / lobby_columns[4];
        int width = gbtn->scr_pos_x + lobby_columns[i + 1] * gbtn->width / lobby_columns[4] - x;
        int tab_width = width;
        int tab_x = x - padding;
        if (i == 3) {
            tab_width += 20 * gbtn->width / lobby_columns[4];
        } else {
            tab_width += left_width - right_width + 2 * scale / 16;
        }
        LbSpriteDrawResized(tab_x, gbtn->scr_pos_y, scale, left);
        LbSpriteDrawScaled(tab_x + left_width, gbtn->scr_pos_y, middle, tab_width - left_width - right_width, gbtn->height);
        LbSpriteDrawResized(tab_x + tab_width - right_width, gbtn->scr_pos_y, scale, right);
        draw_lobby_text(x, gbtn->scr_pos_y + scale / 16, width - 12 * gbtn->width / lobby_columns[4], gbtn->height, 2, text[i]);
    }
}

void frontnet_draw_session_button(struct GuiButton *gbtn)
{
    int index = net_session_scroll_offset + gbtn->content.lval - 45;
    if (index < 0 || index >= net_number_of_sessions) {
        return;
    }
    if (GetMouseX() >= gbtn->scr_pos_x && GetMouseX() < gbtn->scr_pos_x + gbtn->width && GetMouseY() >= gbtn->scr_pos_y && GetMouseY() < gbtn->scr_pos_y + gbtn->height) {
        lobby_tooltip_index = index;
    }
    const struct TbNetworkSessionNameEntry *session = net_session[index];
    int font = 1;
    if (frontend_mouse_over_button == gbtn->content.lval) {
        font = 2;
    }
    if (net_session_incompatible(session)) {
        font = 3;
    }
    const char *phase = get_string(GUIStr_NetUnknown);
    if (session->phase == NetPhase_Lobby) {
        phase = get_string(GUIStr_NetInLobby);
    } else if (session->phase == NetPhase_InGame) {
        phase = get_string(GUIStr_NetInGame);
    } else if (session->phase == NetPhase_InLandview) {
        phase = get_string(GUIStr_NetInLandview);
    }
    char count[8] = "?";
    char capacity[8] = "?";
    if (session->roster_known) {
        snprintf(count, sizeof(count), "%d", session->player_count);
    }
    if (session->max_players) {
        snprintf(capacity, sizeof(capacity), "%d", session->max_players);
    }
    char players[20];
    snprintf(players, sizeof(players), "%s/%s", count, capacity);
    const char *version = session->version;
    if (!*version) {
        version = get_string(GUIStr_NetUnknown);
    }
    const char *text[] = {session->text, phase, players, version};
    draw_lobby_columns(gbtn, font, text);
}

static void draw_lobby_panel_row(int x, int y, int width, int height, int scale, int sprite_index)
{
    const struct TbSprite *left = get_frontend_sprite(sprite_index);
    const struct TbSprite *right = get_frontend_sprite(sprite_index + 5);
    int left_width = left->SWidth * scale / 16;
    int right_width = right->SWidth * scale / 16;
    int middle_width = width - left_width - right_width;
    struct GraphicsWindow window;
    LbScreenStoreGraphicsWindow(&window);
    LbScreenSetGraphicsWindow(x, y, width, height);
    LbSpriteDrawResized(0, 0, scale, left);
    LbSpriteDrawResized(width - right_width, 0, scale, right);
    LbScreenSetGraphicsWindow(x + left_width, y, middle_width, height);
    int column = 1;
    for (int tile_x = 0; tile_x < middle_width; ) {
        const struct TbSprite *tile = get_frontend_sprite(sprite_index + column);
        LbSpriteDrawResized(tile_x, 0, scale, tile);
        tile_x += max(1, tile->SWidth * scale / 16);
        column++;
        if (column == 5) {
            column = 1;
        }
    }
    LbScreenLoadGraphicsWindow(&window);
}

static void draw_lobby_panel(int x, int y, int width, int height, int scale)
{
    const struct TbSprite *top = get_frontend_sprite(GFS_hugearea_thn_cor_tl);
    const struct TbSprite *bottom = get_frontend_sprite(GFS_hugearea_thn_cor_bl);
    int top_height = top->SHeight * scale / 16;
    int bottom_height = bottom->SHeight * scale / 16;
    RendererSetDrawFlags(0);
    draw_lobby_panel_row(x, y, width, top_height, scale, GFS_hugearea_thn_cor_tl);
    draw_lobby_panel_row(x, y + height - bottom_height, width, bottom_height, scale, GFS_hugearea_thn_cor_bl);
    y += top_height;
    int remaining = height - top_height - bottom_height;
    while (remaining > 0) {
        int sprite_index = GFS_hugearea_thc_cor_ml;
        const struct TbSprite *sprite = get_frontend_sprite(sprite_index);
        if (remaining < sprite->SHeight * scale / 16) {
            sprite_index = GFS_hugearea_thn_cor_ml;
            sprite = get_frontend_sprite(sprite_index);
        }
        int row_height = min(remaining, max(1, sprite->SHeight * scale / 16));
        draw_lobby_panel_row(x, y, width, row_height, scale, sprite_index);
        y += row_height;
        remaining -= row_height;
    }
}

void frontnet_draw_lobby_panel(struct GuiButton *gbtn)
{
    int scale = gbtn->height * 16 / 180;
    draw_lobby_panel(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height, scale);
    const struct TbSprite *top = get_frontend_sprite(GFS_scrollbar_toparrow_std);
    const struct TbSprite *middle = get_frontend_sprite(GFS_scrollbar_vert_ct_long);
    const struct TbSprite *bottom = get_frontend_sprite(GFS_scrollbar_btmarrow_std);
    int x = gbtn->scr_pos_x + gbtn->width;
    int top_y = gbtn->scr_pos_y - scale / 16;
    int middle_y = top_y + top->SHeight * scale / 16;
    int bottom_y = gbtn->scr_pos_y + gbtn->height - bottom->SHeight * scale / 16 + scale / 16;
    LbSpriteDrawResized(x, top_y, scale, top);
    LbSpriteDrawScaled(x, middle_y, middle, middle->SWidth * scale / 16, bottom_y - middle_y);
    LbSpriteDrawResized(x, bottom_y, scale, bottom);
}

void frontnet_draw_lobby_tooltip(struct GuiButton *gbtn)
{
    static int hovered_index = -1;
    static TbClockMSec hover_started = 0;
    static int hover_x = 0;
    static int hover_y = 0;
    set_pointer_graphic_menu();
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    if (lobby_tooltip_index < 0 || lobby_tooltip_index >= net_number_of_sessions) {
        hovered_index = -1;
        return;
    }
    TbClockMSec now = LbTimerClock();
    if (hovered_index != lobby_tooltip_index || mouse_x != hover_x || mouse_y != hover_y) {
        hovered_index = lobby_tooltip_index;
        hover_started = now;
        hover_x = mouse_x;
        hover_y = mouse_y;
    }
    if (now - hover_started < 500) {
        return;
    }
    set_pointer_graphic_none();
    const struct TbNetworkSessionNameEntry *session = net_session[lobby_tooltip_index];
    int lines = 2;
    if (session->roster_known) {
        lines = 1 + session->player_count;
    }
    int scale = gbtn->width * 16 / lobby_columns[4];
    int line_height = 26 * gbtn->width / lobby_columns[4];
    int padding = 7 * scale / 16;
    LbTextSetFont(frontend_font[2]);
    int text_scale = line_height * 16 / LbTextLineHeight();
    int width = LbTextStringWidth(get_string(GUIStr_MnuPlayers)) * text_scale / 16;
    LbTextSetFont(frontend_font[1]);
    text_scale = line_height * 16 / LbTextLineHeight();
    if (!session->roster_known) {
        width = max(width, LbTextStringWidth(get_string(GUIStr_NetPlayerListUnavailable)) * text_scale / 16);
    }
    for (int i = 0; i < session->player_count && session->roster_known; i++) {
        width = max(width, LbTextStringWidth(session->players[i]) * text_scale / 16);
    }
    width = min(width + padding * 2, (int)lbDisplay.GraphicsScreenWidth - padding * 2);
    int height = lines * line_height + padding * 2;
    int x = min(mouse_x + padding, (int)lbDisplay.GraphicsScreenWidth - width);
    int y = min(mouse_y + padding, (int)lbDisplay.GraphicsScreenHeight - height);
    x = max(0, x);
    y = max(0, y);
    draw_lobby_panel(x, y, width, height, scale);
    draw_lobby_text(x + padding, y + padding, width - padding * 2, line_height, 2, get_string(GUIStr_MnuPlayers));
    if (!session->roster_known) {
        draw_lobby_text(x + padding, y + padding + line_height, width - padding * 2, line_height, 1, get_string(GUIStr_NetPlayerListUnavailable));
    }
    for (int i = 0; i < session->player_count && session->roster_known; i++) {
        draw_lobby_text(x + padding, y + padding + (i + 1) * line_height, width - padding * 2, line_height, 1, session->players[i]);
    }
}

void frontnet_draw_lobby_option(struct GuiButton *gbtn)
{
    int font_index = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
    const char *label = get_string(GUIStr_NetLobbyName);
    if (gbtn->btype_value == 1) {
        label = get_string(GUIStr_NetMaximumPlayers);
    } else if (gbtn->btype_value == 2) {
        label = get_string(GUIStr_NetAllowObservers);
        font_index = 3;
    } else if (gbtn->btype_value == 3) {
        label = get_string(GUIStr_NetSettings);
        font_index = 2;
    }
    draw_lobby_text(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height, font_index, label);
    char value[12];
    const char *text = NULL;
    if (gbtn->btype_value == 1) {
        snprintf(value, sizeof(value), "%d", net_lobby_max_players);
        text = value;
    } else if (gbtn->btype_value == 2) {
        text = get_string(GUIStr_Off);
    }
    if (text != NULL) {
        int value_offset = gbtn->width * 205 / 416;
        draw_lobby_text(gbtn->scr_pos_x + value_offset, gbtn->scr_pos_y, gbtn->width - value_offset, gbtn->height, font_index, text);
    }
    RendererSetDrawFlags(0);
}

void frontnet_change_lobby_option(struct GuiButton *gbtn)
{
    if (gbtn->btype_value == 1) {
        if (gbtn->button_state_right_pressed) {
            net_lobby_max_players--;
        } else {
            net_lobby_max_players++;
        }
        if (net_lobby_max_players > MAX_NET_USERS) {
            net_lobby_max_players = MIN_NET_USERS;
        } else if (net_lobby_max_players < MIN_NET_USERS) {
            net_lobby_max_players = MAX_NET_USERS;
        }
    }
}

void frontnet_session_set_lobby_name(struct GuiButton *gbtn)
{
    const char *name = net_lobby_name;
    while (isspace((unsigned char)*name)) {
        name++;
    }
    if (!*name) {
        name = net_player_name;
    }
    snprintf(net_config_info.net_lobby_name, sizeof(net_config_info.net_lobby_name), "%s", name);
    size_t length = strlen(net_config_info.net_lobby_name);
    while (length > 0 && isspace((unsigned char)net_config_info.net_lobby_name[length - 1])) {
        net_config_info.net_lobby_name[--length] = '\0';
    }
    snprintf(net_lobby_name, sizeof(net_lobby_name), "%s", net_config_info.net_lobby_name);
    net_write_config_file();
}

void frontnet_session_create(struct GuiButton *gbtn)
{
    frontnet_session_set_lobby_name(gbtn);
    uint32_t plyr_num;
    if (LbNetwork_Create(net_lobby_name, net_player_name, &plyr_num, nullptr)) {
        process_network_error(-801);
        return;
    }
    turn_off_menu(GMnu_FEADD_SESSION);
    frontend_set_player_number(plyr_num);
    fe_computer_players = 0;
    frontend_set_state(FeSt_NET_START);
}
