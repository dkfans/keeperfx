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
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82,  61,  82,  61,165, 29, frontnet_draw_text_bar,            0, GUIStr_Empty, 0,      {27},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  95,  63,  91,  63,165, 25, frontend_draw_text,                0, GUIStr_Empty, 0,      {19},            0, NULL },
  { 5, -1,-1, 0, frontnet_session_set_player_name,NULL,frontend_over_button,19,200,63,95,63,432, 26, frontend_draw_enter_text,          0, GUIStr_Empty, 0,{.str = tmp_net_player_name}, 20, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 91, 128, 91, 128,434,26, frontnet_draw_lobby_columns, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 154,  82, 154,450,180, frontnet_draw_scroll_box,          0, GUIStr_Empty, 0,      {26},            0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_session_up,NULL,       frontend_over_button,0, 532, 153, 532, 153, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {17},            0, frontnet_session_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_session_down,NULL,     frontend_over_button,0, 532, 321, 532, 321, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {18},            0, frontnet_session_down_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 536, 167, 536, 167, 20,154, frontnet_draw_sessions_scroll_tab, 0, GUIStr_Empty, 0,      {40},            0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 91,158,91,158,434,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {45}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 91,182,91,182,434,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {46}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 91,206,91,206,434,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {47}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 91,230,91,230,434,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {48}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 91,254,91,254,434,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {49}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 91,278,91,278,434,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {50}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 91,302,91,302,434,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {51}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_session_add,NULL,   frontend_over_button,0,999,358,999,358,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,      {14},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_return_to_main_menu,NULL,frontend_over_button,0,999,404,999,404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,       {6},            0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 0, 455, 0, 455,371,46, frontend_draw_product_version, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, LbBFeF_NoMouseOver, 91, 158, 91, 158,434,168, frontnet_draw_lobby_tooltip, 0, GUIStr_Empty, 0, {0}, 0, NULL },
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
    int text_width = LbTextStringWidthM(text, scale);
    int offset = 0;
    if (text_width > width) {
        static TbClockMSec started = LbTimerClock();
        int overflow = text_width - width;
        int speed = max(1, 24 * scale / 16);
        int travel_time = max(1, overflow * 1000 / speed);
        int elapsed = (LbTimerClock() - started) % (2 * travel_time + 2000);
        if (elapsed > travel_time + 1000) {
            offset = overflow - min(overflow, max(0, elapsed - travel_time - 2000) * speed / 1000);
        } else if (elapsed > 1000) {
            offset = min(overflow, (elapsed - 1000) * speed / 1000);
        }
    }
    LbTextSetWindow(x, y, width, height);
    LbTextSetJustifyWindow(x, y, max(width, text_width));
    LbTextDrawResized(-offset, 0, scale, text);
}

static const int lobby_columns[] = {10, 184, 274, 364, 434};
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
    int highlight_height = 2 * thickness;
    int separator_height = highlight_height + thickness;
    int shadow_height = 2 * thickness;
    int height = 26 * gbtn->width / lobby_columns[4];
    int y = gbtn->scr_pos_y + (gbtn->height - height) / 2;
    int separator_y = gbtn->scr_pos_y + gbtn->height - separator_height - thickness;
    int border_inset = 2 * gbtn->width / lobby_columns[4];
    int separator_x = gbtn->scr_pos_x - 9 * gbtn->width / lobby_columns[4] + border_inset;
    int separator_width = 450 * gbtn->width / lobby_columns[4] - 2 * border_inset;
    RendererSetDrawFlags(Lb_SPRITE_TRANSPAR8);
    LbDrawBox(separator_x, gbtn->scr_pos_y + thickness, separator_width, separator_y - gbtn->scr_pos_y - thickness, 37);
    RendererSetDrawFlags(0);
    for (int i = 0; i < 4; i++) {
        int x = gbtn->scr_pos_x + lobby_columns[i] * gbtn->width / lobby_columns[4];
        int width = (lobby_columns[i + 1] - lobby_columns[i] - 4) * gbtn->width / lobby_columns[4];
        draw_lobby_text(x, y, width, height, font, text[i]);
    }
    RendererSetDrawFlags(0);
    LbDrawBox(separator_x, separator_y + separator_height, separator_width, shadow_height, 6);
    RendererSetDrawFlags(Lb_SPRITE_TRANSPAR8);
    LbDrawBox(separator_x, separator_y + highlight_height, separator_width, thickness, 37);
    LbDrawBox(separator_x, separator_y, separator_width, thickness, 37);
    LbDrawBox(separator_x, separator_y + thickness, separator_width, thickness, 105);
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
        if (i == 3) {
            tab_width += 20 * gbtn->width / lobby_columns[4];
        } else {
            tab_width += left_width - right_width + 2 * scale / 16;
        }
        LbSpriteDrawResized(x - padding, gbtn->scr_pos_y, scale, left);
        LbSpriteDrawScaled(x - padding + left_width, gbtn->scr_pos_y, middle, tab_width - left_width - right_width, gbtn->height);
        LbSpriteDrawResized(x - padding + tab_width - right_width, gbtn->scr_pos_y, scale, right);
        draw_lobby_text(x, gbtn->scr_pos_y + scale / 16, tab_width - padding - right_width, gbtn->height, 2, text[i]);
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

static void draw_lobby_tooltip_panel(int x, int y, int width, int height, int scale)
{
    const int sprites[3][3] = {
        {GFS_hugearea_thn_cor_tl, GFS_hugearea_thn_tx1_tc, GFS_hugearea_thn_cor_tr},
        {GFS_hugearea_thc_cor_ml, GFS_hugearea_thc_tx1_mc, GFS_hugearea_thc_cor_mr},
        {GFS_hugearea_thn_cor_bl, GFS_hugearea_thn_tx1_bc, GFS_hugearea_thn_cor_br},
    };
    const struct TbSprite *corner = get_frontend_sprite(sprites[0][0]);
    int edge_x = corner->SWidth * scale / 16;
    int edge_y = corner->SHeight * scale / 16;
    const int widths[] = {edge_x, width - 2 * edge_x, edge_x};
    const int heights[] = {edge_y, height - 2 * edge_y, edge_y};
    RendererSetDrawFlags(0);
    for (int row = 0; row < 3; row++) {
        int column_x = x;
        for (int column = 0; column < 3; column++) {
            LbSpriteDrawScaled(column_x, y, get_frontend_sprite(sprites[row][column]), widths[column], heights[row]);
            column_x += widths[column];
        }
        y += heights[row];
    }
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
    int line_height = 16 * scale / 16;
    int padding = 7 * scale / 16;
    LbTextSetFont(frontend_font[1]);
    int text_scale = line_height * 16 / LbTextLineHeight();
    int width = 70 * scale / 16;
    if (!session->roster_known) {
        width = LbTextStringWidth("Player list unavailable") * text_scale / 16 + padding * 2;
    }
    for (int i = 0; i < session->player_count; i++) {
        width = max(width, LbTextStringWidth(session->players[i]) * text_scale / 16 + padding * 2);
    }
    width = min(width, (int)lbDisplay.GraphicsScreenWidth - padding * 2);
    int height = lines * line_height + padding * 2;
    int x = min(mouse_x + padding, (int)lbDisplay.GraphicsScreenWidth - width);
    int y = min(mouse_y + padding, (int)lbDisplay.GraphicsScreenHeight - height);
    x = max(0, x);
    y = max(0, y);
    draw_lobby_tooltip_panel(x, y, width, height, scale);
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
