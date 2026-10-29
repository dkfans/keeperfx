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
#include "bflib_enet.h"
#include "bflib_datetm.h"
#include "bflib_guibtns.h"
#include "bflib_video.h"
#include "vidmode.h"
#include "vidfade.h"
#include "bflib_vidraw.h"
#include "bflib_render.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"

#include "config_strings.h"
#include "config_campaigns.h"
#include "front_network.h"
#include "front_input.h"
#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "front_landview.h"
#include "front_simple.h"
#include "net_game.h"
#include "net_lobby.h"
#include "player_data.h"
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
#define LOBBY_HEADER_INSET 6
#define LOBBY_PANEL_TILE_COUNT 4
#define LOBBY_COLUMN_TAB_PADDING 19
#define LOBBY_PLAYER_ROW_INSET 7
#define LOBBY_COLUMN_TEXT_GAP 12
#define LOBBY_LEFT_WIDTH 324
#define LOBBY_RIGHT_X (LOBBY_LEFT_WIDTH + 12)
#define LOBBY_RIGHT_WIDTH 268
#define LOBBY_OPTION_X (LOBBY_RIGHT_X + 10)
#define LOBBY_OPTION_WIDTH (LOBBY_RIGHT_WIDTH - 17)
#define LOBBY_MESSAGES_WIDTH (LOBBY_RIGHT_X + LOBBY_RIGHT_WIDTH - 10)
#define LOBBY_SCROLLBAR_WIDTH 26
#define LOBBY_HALF_WIDTH ((LOBBY_MESSAGES_WIDTH + LOBBY_SCROLLBAR_WIDTH) / 2)
#define LOBBY_BUTTON_WIDTH 247
#define LOBBY_START_BUTTON_X (10 + (LOBBY_HALF_WIDTH - LOBBY_BUTTON_WIDTH) / 2)
#define LOBBY_CANCEL_BUTTON_X (LOBBY_START_BUTTON_X + LOBBY_HALF_WIDTH)
#define LOBBY_NUMBER_TEXT_LENGTH 12
#define LOBBY_PALETTE_MAX_INTENSITY 63
#define LOBBY_SECTION_TEXT_INDENT 20
#define LOBBY_SELECTOR_TEXT_RIGHT_INSET 8
#define LOBBY_ROW_HEIGHT 26
#define LOBBY_PANEL_PADDING 4
#define LOBBY_SCROLL_BUTTON_HEIGHT 14
#define LOBBY_SCROLL_BUTTON_OVERLAP 13
#define LOBBY_OPTION_TEXT_GAP 6
#define LOBBY_MAPPACK_Y 274
#define LOBBY_MAPPACK_SELECTOR_ID 56
#define BID_LOBBY_MAPPACK_SELECTOR (BID_ASSIST + 1)
#define LOBBY_MAPPACK_LIST_Y 86
#define LOBBY_MAPPACK_LIST_HEIGHT 190
#define LOBBY_MAPPACK_LIST_MAX_ROWS ((LOBBY_MAPPACK_LIST_HEIGHT - 2 * LOBBY_PANEL_PADDING) / LOBBY_ROW_HEIGHT)

struct GuiButtonInit frontend_net_service_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,      {10},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, frontnet_draw_scroll_box_tab,      0, GUIStr_Empty, 0,      {12},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,180, frontnet_draw_scroll_box,          0, GUIStr_Empty, 0,      {26},            0, frontnet_services_wheel_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_service_up,NULL,frontend_over_button,       0, 532, 149, 532, 149, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {17},            0, frontnet_service_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_service_down,NULL,frontend_over_button,     0, 532, 317, 532, 317, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {18},            0, frontnet_service_down_maintain },
  { LbBtnT_DraggableBtn,BID_DEFAULT, 0, 0, frontnet_service_scroll,               NULL,        NULL,               0, 536, 163, 536, 163, 20,154, frontnet_draw_services_scroll_tab, 0, GUIStr_Empty, 0,      {40},            0, NULL },
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
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  10, 154,  10, 154,594,180, frontnet_draw_lobby_panel,         0, GUIStr_Empty, 0,      {26},            0, frontnet_sessions_wheel_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_session_up,NULL,       frontend_over_button,0, 604, 153, 604, 153, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {17},            0, frontnet_session_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_session_down,NULL,     frontend_over_button,0, 604, 321, 604, 321, 26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,      {18},            0, frontnet_session_down_maintain },
  { LbBtnT_DraggableBtn,BID_DEFAULT, 0, 0, frontnet_session_scroll,               NULL,        NULL,               0, 608, 167, 608, 167, 20,154, frontnet_draw_sessions_scroll_tab, 0, GUIStr_Empty, 0,      {40},            0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,158,19,158,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {45}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,182,19,182,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {46}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,206,19,206,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {47}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,230,19,230,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {48}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,254,19,254,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {49}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,278,19,278,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {50}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, frontnet_session_join, NULL, frontend_over_button, 0, 19,302,19,302,578,24, frontnet_draw_session_button, 0, GUIStr_Empty, 0, {51}, 0, frontnet_session_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_session_create,NULL,frontend_over_button,0,999,358,999,358,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,      {14},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_return_to_main_menu,NULL,frontend_over_button,0,999,404,999,404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,       {6},            0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 0, 455, 0, 455,371,46, frontend_draw_product_version, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, LbBFeF_NoMouseOver, 19, 158, 19, 158,578,168, frontnet_draw_lobby_tooltip, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,           0,  0,       {0},            0, NULL },
};


struct GuiButtonInit frontend_net_start_buttons[] = {
  { LbBtnT_NormalBtn,  BID_MENU_TITLE, 0, 0, NULL,               NULL,   NULL,                    0, 999,  12, 999,  12, 371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty, 0,  {11},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, 10,  60, 10,  60, LOBBY_LEFT_WIDTH - LOBBY_HEADER_INSET, 26, frontnet_draw_lobby_players_header, 0, GUIStr_Empty, 0, {31},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  10,  86,  10,  86, LOBBY_LEFT_WIDTH,190, frontnet_draw_lobby_panel_without_scrollbar,         0, GUIStr_Empty, 0,   {0},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_player_setting,frontnet_change_player_setting,frontend_over_button,0,  10,  90,  10,  90, LOBBY_LEFT_WIDTH - LOBBY_HEADER_INSET,104, frontnet_draw_net_start_players,   0, GUIStr_Empty, 0,  {21},    0, frontnet_maintain_net_start_players },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, LOBBY_RIGHT_X,  60, LOBBY_RIGHT_X,  60, LOBBY_RIGHT_WIDTH - LOBBY_HEADER_INSET, 26, frontnet_draw_lobby_settings_header, 0, GUIStr_Empty, 0, {117},    0, NULL },
  { LbBtnT_NormalBtn,  BID_LOBBY_MAPPACK_SELECTOR, 0, 0, frontnet_open_mappack_list,frontnet_open_mappack_list,frontend_over_button,0, LOBBY_RIGHT_X, LOBBY_MAPPACK_Y, LOBBY_RIGHT_X, LOBBY_MAPPACK_Y, LOBBY_RIGHT_WIDTH, LOBBY_ROW_HEIGHT, NULL, 0, GUIStr_Empty, 0, {LOBBY_MAPPACK_SELECTOR_ID},    0, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, LOBBY_RIGHT_X,  86, LOBBY_RIGHT_X,  86, LOBBY_RIGHT_WIDTH,LOBBY_MAPPACK_LIST_HEIGHT, frontnet_draw_lobby_settings_panel,       0, GUIStr_Empty, 0,   {0},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_lobby_option,frontnet_change_lobby_option,frontend_over_button,0, LOBBY_OPTION_X,  90, LOBBY_OPTION_X,  90, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0,  {45},    0, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_lobby_option,frontnet_change_lobby_option,frontend_over_button,1, LOBBY_OPTION_X, 116, LOBBY_OPTION_X, 116, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0,  {46},    0, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_lobby_option,frontnet_change_lobby_option,frontend_over_button,2, LOBBY_OPTION_X, 142, LOBBY_OPTION_X, 142, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0,  {47},    0, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_lobby_option,frontnet_change_lobby_option,frontend_over_button,3, LOBBY_OPTION_X, 168, LOBBY_OPTION_X, 168, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0,  {48},    0, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_lobby_option,frontnet_change_lobby_option,frontend_over_button,4, LOBBY_OPTION_X, 194, LOBBY_OPTION_X, 194, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0,  {49},    0, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_lobby_option,frontnet_change_lobby_option,frontend_over_button,5, LOBBY_OPTION_X, 220, LOBBY_OPTION_X, 220, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0,  {50},    0, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_change_lobby_option,frontnet_change_lobby_option,frontend_over_button,6, LOBBY_OPTION_X, 246, LOBBY_OPTION_X, 246, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_option, 0, GUIStr_Empty, 0,  {51},    0, frontnet_maintain_host_setting },
  { LbBtnT_EditBox, -1,-1, 0, frontnet_session_set_lobby_name, NULL, frontend_over_button,     45, LOBBY_OPTION_X,  90, LOBBY_OPTION_X,  90, LOBBY_OPTION_WIDTH, 26, frontnet_draw_lobby_name,          0, GUIStr_Empty, 0, {.str = net_lobby_settings.net_lobby_name}, sizeof(net_lobby_settings.net_lobby_name) - 1, frontnet_maintain_host_setting },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0, 10, 278, 10, 278, LOBBY_LEFT_WIDTH - LOBBY_HEADER_INSET, 26, frontnet_draw_lobby_section_header, 0, GUIStr_Empty, 0, {34},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  10, 304,  10, 304, LOBBY_MESSAGES_WIDTH, 86, frontnet_draw_lobby_panel,         0, GUIStr_Empty, 0,   {0},    0, frontnet_messages_wheel_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_messages_up,NULL,  frontend_over_button,    0, 10 + LOBBY_MESSAGES_WIDTH, 303, 10 + LOBBY_MESSAGES_WIDTH, 303,  26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,  {38},    0, frontnet_messages_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_messages_down,NULL,frontend_over_button,    0, 10 + LOBBY_MESSAGES_WIDTH, 377, 10 + LOBBY_MESSAGES_WIDTH, 377,  26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,  {39},    0, frontnet_messages_down_maintain },
  { LbBtnT_DraggableBtn,BID_DEFAULT, 0, 0, frontnet_messages_scroll,               NULL,   NULL,                    0, 14 + LOBBY_MESSAGES_WIDTH, 317, 14 + LOBBY_MESSAGES_WIDTH, 317,  20, 60, frontnet_draw_messages_scroll_tab, 0, GUIStr_Empty, 0,  {40},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  10, 393,  10, 393, LOBBY_MESSAGES_WIDTH, 28, frontnet_draw_current_message,     0, GUIStr_Empty, 0,  {43},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,  17, 308,  17, 308, LOBBY_MESSAGES_WIDTH - 12, 78, frontnet_draw_messages,            0, GUIStr_Empty, 0,  {44},    0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, set_packet_start,   NULL,   frontend_over_button,    0, LOBBY_START_BUTTON_X, 422, LOBBY_START_BUTTON_X, 422, LOBBY_BUTTON_WIDTH, 46, frontend_draw_small_menu_button,   0, GUIStr_Empty, 0,  {15},    0, frontnet_start_game_maintain },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_return_to_session_menu,NULL,frontend_over_button,1, LOBBY_CANCEL_BUTTON_X, 422, LOBBY_CANCEL_BUTTON_X, 422, LOBBY_BUTTON_WIDTH, 46, frontend_draw_small_menu_button,   0, GUIStr_Empty, 0,  {16},    0, NULL },
  { LbBtnT_NormalBtn, BID_DEFAULT, 0, 0, NULL, NULL, NULL, 0, 0, 455, 0, 455,371,46, frontend_draw_product_version, 0, GUIStr_Empty, 0, {0}, 0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, frontnet_mappack_list_click,frontnet_mappack_list_click,NULL,0, 0, 0, 0, 0, 640, 480, frontnet_draw_mappack_list, 0, GUIStr_Empty, 0, {0}, 0, frontnet_maintain_mappack_list },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_mappack_up,NULL,frontend_over_button,0, 604, LOBBY_MAPPACK_LIST_Y - 1, 604, LOBBY_MAPPACK_LIST_Y - 1,  26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,  {54},    0, frontnet_mappack_up_maintain },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, frontnet_mappack_down,NULL,frontend_over_button,0, 604, 403, 604, 403,  26, 14, frontnet_draw_slider_button,       0, GUIStr_Empty, 0,  {55},    0, frontnet_mappack_down_maintain },
  { LbBtnT_DraggableBtn,BID_DEFAULT, 0, 0, frontnet_mappack_scroll,NULL,  NULL,                    0, 608, 317, 608, 317,  20, 68, frontnet_draw_mappack_scroll_tab, 0, GUIStr_Empty, 0,  {40},    0, frontnet_maintain_mappack_scroll_tab },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    LbBFeF_NoMouseOver, LOBBY_OPTION_X + LOBBY_OPTION_WIDTH / 2,  90, LOBBY_OPTION_X + LOBBY_OPTION_WIDTH / 2,  90, LOBBY_OPTION_WIDTH / 2, 26, frontnet_draw_lobby_tooltips, 0, GUIStr_Empty, 0, {0},    0, NULL },
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,   NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,           0,  0,   {0},    0, NULL },
};

struct GuiMenu frontend_net_service_menu =
 { GMnu_FENET_SERVICE, 0, 1, frontend_net_service_buttons, POS_SCRCTR, POS_SCRCTR,  640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_session_menu =
 { GMnu_FENET_SESSION, 0, 1, frontend_net_session_buttons, POS_SCRCTR, POS_SCRCTR,  640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_start_menu =
 { GMnu_FENET_START,   0, 1, frontend_net_start_buttons,   POS_SCRCTR, POS_SCRCTR,  640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
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

static void draw_lobby_text(int x, int y, int width, int height, int font, unsigned short draw_flags, int32_t text_x, const char *text)
{
    LbTextSetFont(frontend_font[font]);
    RendererSetDrawFlags(draw_flags);
    int scale = height * 16 / LbTextLineHeight();
    LbTextSetWindow(x, y, width, min(height, (int)(lbDisplay.GraphicsWindowY + lbDisplay.GraphicsWindowHeight - y)));
    LbTextDrawResized(text_x, (height - LbTextLineHeight() * scale / 16) / 2, scale, text);
}

static int lobby_font(struct GuiButton *gbtn, TbBool highlighted)
{
    TbBool enabled = (gbtn->flags & LbBtnF_Enabled) != 0;
    if (!enabled && network_is_host()) {
        return 3;
    }
    if (enabled && highlighted) {
        return 2;
    }
    return 1;
}

static int lobby_menu_scale(struct GuiButton *gbtn)
{
    struct GuiMenu *gmnu = get_active_menu(gbtn->gmenu_idx);
    return gmnu->width * 16 / gmnu->menu_init->width;
}

static TbBool mouse_is_over_lobby_area(struct GuiButton *gbtn, TbBool include_scrollbar)
{
    int32_t right = gbtn->scr_pos_x + gbtn->width;
    if (include_scrollbar) {
        right += LOBBY_SCROLLBAR_WIDTH * lobby_menu_scale(gbtn) / 16;
    }
    int32_t mouse_x = GetMouseX();
    int32_t mouse_y = GetMouseY();
    return (mouse_x >= gbtn->scr_pos_x) && (mouse_x < right) && (mouse_y >= gbtn->scr_pos_y) && (mouse_y < gbtn->scr_pos_y + gbtn->height);
}

static int32_t lobby_text_width(int height, int font, const char *text)
{
    LbTextSetFont(frontend_font[font]);
    int scale = height * 16 / LbTextLineHeight();
    return LbTextStringPartWidthM(text, INT_MAX, scale);
}

static int32_t draw_lobby_text_right_aligned(int right, int y, int max_width, int height, int font, TbBool show_end, const char *text)
{
    int32_t text_width = lobby_text_width(height, font, text);
    int32_t width = min(text_width, max_width);
    int32_t text_x = 0;
    if (show_end) {
        text_x = width - text_width;
    }
    draw_lobby_text(right - width, y, width, height, font, 0, text_x, text);
    return width;
}

enum LobbyPlayerColumn {
    LobbyPlayerColumn_Name = 0,
    LobbyPlayerColumn_Team,
    LobbyPlayerColumn_Ping,
    LobbyPlayerColumn_Count,
};

static const int lobby_columns[] = {10, 277, 377, 478, 578};
static const int lobby_player_columns[] = {LOBBY_COLUMN_TAB_PADDING, LOBBY_LEFT_WIDTH - 112, LOBBY_LEFT_WIDTH - 54, LOBBY_LEFT_WIDTH - LOBBY_HEADER_INSET};
static int lobby_tooltip_index = -1;

static unsigned char *get_frontend_lobby_glass_map(void)
{
    static unsigned char *glass_map = [] {
        static struct TbColorTables tables;
        compute_fade_tables(&tables, frontend_palette, frontend_palette);
        return tables.ghost;
    }();
    if (RendererGetActiveType() == RENDERER_SOFTWARE) {
        return glass_map;
    }
    return lbDisplay.GlassMap;
}

static void draw_lobby_glass_box(int x, int y, int width, int height, TbPixel colour, unsigned short draw_flags)
{
    unsigned char *previous_glass_map = lbDisplay.GlassMap;
    lbDisplay.GlassMap = get_frontend_lobby_glass_map();
    RendererSetDrawFlags(draw_flags);
    LbDrawBox(x, y, width, height, colour);
    RendererSetDrawFlags(0);
    lbDisplay.GlassMap = previous_glass_map;
}

static void draw_lobby_separator(int x, int y, int width, int thickness)
{
    RendererSetDrawFlags(0);
    LbDrawBox(x, y, width, thickness, 6);
    LbDrawBox(x, y + thickness, width, thickness, 105);
    draw_lobby_glass_box(x, y + 2 * thickness, width, thickness, 37, Lb_SPRITE_TRANSPAR8);
    draw_lobby_glass_box(x, y + 3 * thickness, width, thickness, 0, Lb_SPRITE_TRANSPAR8);
}

static void draw_lobby_columns(struct GuiButton *gbtn, int font, const char *const text[4])
{
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
    draw_lobby_glass_box(separator_x, panel_y, separator_width, separator_y - panel_y, 37, Lb_SPRITE_TRANSPAR8);
    for (int i = 0; i < 4; i++) {
        int x = gbtn->scr_pos_x + lobby_columns[i] * gbtn->width / lobby_columns[4];
        int width = (lobby_columns[i + 1] - lobby_columns[i] - LOBBY_COLUMN_TEXT_GAP) * gbtn->width / lobby_columns[4];
        draw_lobby_text(x, y, width, height, font, 0, 0, text[i]);
    }
    draw_lobby_separator(separator_x, separator_y, separator_width, thickness);
}

static void draw_lobby_column_tabs(struct GuiButton *gbtn, const int *columns, int count, const char *const *text)
{
    const struct TbSprite *left = get_frontend_sprite(GFS_hugearea_thc_cor_tl);
    const struct TbSprite *middle = get_frontend_sprite(GFS_hugearea_thc_tx1_tc);
    const struct TbSprite *right = get_frontend_sprite(GFS_hugearea_thc_cor_tr);
    int scale = simple_frontend_sprite_height_units_per_px(gbtn, GFS_hugearea_thc_tx1_tc, 100);
    int left_width = left->SWidth * scale / 16;
    int right_width = right->SWidth * scale / 16;
    int padding = LOBBY_COLUMN_TAB_PADDING * gbtn->width / columns[count];
    RendererSetDrawFlags(0);
    for (int i = 0; i < count; i++) {
        int x = gbtn->scr_pos_x + columns[i] * gbtn->width / columns[count];
        int width = gbtn->scr_pos_x + columns[i + 1] * gbtn->width / columns[count] - x;
        int tab_width = width;
        int tab_x = x - padding;
        if (i == count - 1) {
            tab_width += 20 * gbtn->width / columns[count];
        } else {
            tab_width += left_width - right_width + 2 * scale / 16;
        }
        LbSpriteDrawResized(tab_x, gbtn->scr_pos_y, scale, left);
        LbSpriteDrawScaled(tab_x + left_width, gbtn->scr_pos_y, middle, tab_width - left_width - right_width, gbtn->height);
        LbSpriteDrawResized(tab_x + tab_width - right_width, gbtn->scr_pos_y, scale, right);
        draw_lobby_text(x, gbtn->scr_pos_y + scale / 16, width - LOBBY_COLUMN_TEXT_GAP * gbtn->width / columns[count], gbtn->height, 2, 0, 0, text[i]);
    }
}

void frontnet_draw_lobby_columns(struct GuiButton *gbtn)
{
    lobby_tooltip_index = -1;
    const char *text[] = {get_string(GUIStr_NetSessions), get_string(GUIStr_NetState), get_string(GUIStr_MnuPlayers), get_string(GUIStr_NetVersion)};
    draw_lobby_column_tabs(gbtn, lobby_columns, 4, text);
}

void frontnet_draw_session_button(struct GuiButton *gbtn)
{
    int index = net_session_scroll_offset + gbtn->content.lval - 45;
    if (index < 0 || index >= net_number_of_sessions) {
        return;
    }
    if (mouse_is_over_lobby_area(gbtn, false)) {
        lobby_tooltip_index = index;
    }
    const struct TbNetworkSessionNameEntry *session = net_session[index];
    int font = 1;
    if (frontend_mouse_over_button == gbtn->content.lval) {
        font = 2;
    }
    const char *phase = get_string(GUIStr_NetUnknown);
    if (session->phase == NetPhase_Lobby) {
        phase = get_string(GUIStr_NetInLobby);
    } else if (session->phase == NetPhase_InGame) {
        phase = get_string(GUIStr_NetInGame);
    } else if (session->phase == NetPhase_InLandview) {
        phase = get_string(GUIStr_NetInLandview);
    } else if (session->phase == NetPhase_Loading) {
        phase = get_string(GUIStr_NetLoading);
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
    char version_number[sizeof(session->version)];
    snprintf(version_number, sizeof(version_number), "%.*s", (int)strspn(session->version, "0123456789."), session->version);
    const char *version = version_number;
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
    int tile_x = 0;
    for (int column = 0; tile_x < middle_width; column++) {
        const struct TbSprite *tile = get_frontend_sprite(sprite_index + 1 + column % LOBBY_PANEL_TILE_COUNT);
        LbSpriteDrawResized(tile_x, 0, scale, tile);
        tile_x += max(1, tile->SWidth * scale / 16);
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
    int middle_height = max(1, get_frontend_sprite(GFS_hugearea_thc_cor_ml)->SHeight * scale / 16);
    while (remaining > 0) {
        int row_height = min(remaining, middle_height);
        draw_lobby_panel_row(x, y, width, row_height, scale, GFS_hugearea_thc_cor_ml);
        y += row_height;
        remaining -= row_height;
    }
}

void frontnet_draw_lobby_panel_without_scrollbar(struct GuiButton *gbtn)
{
    draw_lobby_panel(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height, lobby_menu_scale(gbtn));
}

void frontnet_draw_lobby_panel(struct GuiButton *gbtn)
{
    frontnet_draw_lobby_panel_without_scrollbar(gbtn);
    int scale = lobby_menu_scale(gbtn);
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
    int width = lobby_text_width(line_height, 2, get_string(GUIStr_MnuPlayers));
    if (!session->roster_known) {
        width = max(width, (int)lobby_text_width(line_height, 1, get_string(GUIStr_NetPlayerListUnavailable)));
    }
    for (int i = 0; i < session->player_count && session->roster_known; i++) {
        width = max(width, (int)lobby_text_width(line_height, 1, session->players[i]));
    }
    width = min(width + padding * 2, (int)lbDisplay.GraphicsScreenWidth - padding * 2);
    int height = lines * line_height + padding * 2;
    int x = min(mouse_x + padding, (int)lbDisplay.GraphicsScreenWidth - width);
    int y = min(mouse_y + padding, (int)lbDisplay.GraphicsScreenHeight - height);
    x = max(0, x);
    y = max(0, y);
    draw_lobby_panel(x, y, width, height, scale);
    draw_lobby_text(x + padding, y + padding, width - padding * 2, line_height, 2, 0, 0, get_string(GUIStr_MnuPlayers));
    if (!session->roster_known) {
        draw_lobby_text(x + padding, y + padding + line_height, width - padding * 2, line_height, 1, 0, 0, get_string(GUIStr_NetPlayerListUnavailable));
    }
    for (int i = 0; i < session->player_count && session->roster_known; i++) {
        draw_lobby_text(x + padding, y + padding + (i + 1) * line_height, width - padding * 2, line_height, 1, 0, 0, session->players[i]);
    }
}

enum LobbyOption {
    LobbyOption_LobbyName = 0,
    LobbyOption_MaximumPlayers,
    LobbyOption_ComputerPlayers,
    LobbyOption_ComputerAssistant,
    LobbyOption_Cheats,
    LobbyOption_OpenToSpectators,
    LobbyOption_ShowSpectatorChat,
    LobbyOption_Count,
};

struct LobbyMappackListLayout {
    int32_t rows;
    int32_t max_scroll_offset;
};

static const enum GUIStrings lobby_option_labels[] = {
    GUIStr_NetLobbyName,
    GUIStr_NetMaximumPlayers,
    GUIStr_NetFillEmptySlots,
    GUIStr_NetAllowAssistant,
    GUIStr_NetAllowCheats,
    GUIStr_NetOpenToSpectators,
    GUIStr_NetShowSpectatorChat,
};

int32_t net_lobby_mappack_scroll_offset;
TbBool net_lobby_mappack_list_open;

static struct LobbyMappackListLayout lobby_mappack_list_layout(void)
{
    struct LobbyMappackListLayout layout;
    layout.rows = min((int32_t)mp_mappacks_list.items_num, LOBBY_MAPPACK_LIST_MAX_ROWS);
    layout.max_scroll_offset = (int32_t)mp_mappacks_list.items_num - layout.rows;
    return layout;
}

static void place_lobby_button(struct GuiButton *gbtn, TbBool visible, int32_t pos_y, int32_t height)
{
    struct GuiMenu *gmnu = get_active_menu(gbtn->gmenu_idx);
    int scale = lobby_menu_scale(gbtn);
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Visible * visible) & LbBtnF_Visible;
    gbtn->scr_pos_y = gmnu->pos_y + (pos_y * scale + 8) / 16;
    gbtn->pos_y = gbtn->scr_pos_y;
    gbtn->height = (height * scale + 8) / 16;
}

static uint8_t *lobby_option_toggle(struct ConfigInfo *config, int32_t option)
{
    switch (option) {
    case LobbyOption_OpenToSpectators:
        return &config->spectators_enabled;
    case LobbyOption_ShowSpectatorChat:
        return &config->spectator_chat;
    case LobbyOption_Cheats:
        return &config->cheats_enabled;
    case LobbyOption_ComputerAssistant:
        return &config->computer_assistant;
    default:
        return NULL;
    }
}

static const char *lobby_option_value(int32_t option, char *value, size_t value_size)
{
    uint8_t *toggle = lobby_option_toggle(&net_lobby_settings, option);
    if (toggle != NULL) {
        return get_on_off_string(*toggle);
    }
    switch (option) {
    case LobbyOption_MaximumPlayers:
        snprintf(value, value_size, "%d", net_lobby_max_players);
        return value;
    case LobbyOption_ComputerPlayers:
        return get_on_off_string(fe_computer_players);
    default:
        return NULL;
    }
}

void frontnet_maintain_host_setting(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * network_is_host()) & LbBtnF_Enabled;
}

static void draw_lobby_section_header(struct GuiButton *gbtn, const char *text)
{
    int scale = lobby_menu_scale(gbtn);
    int indent = LOBBY_SECTION_TEXT_INDENT * scale / 16;
    frontnet_draw_tab_bar(gbtn);
    draw_lobby_text(gbtn->scr_pos_x + indent, gbtn->scr_pos_y + scale / 16, gbtn->width - 2 * indent, gbtn->height, 2, 0, 0, text);
}

void frontnet_draw_lobby_section_header(struct GuiButton *gbtn)
{
    draw_lobby_section_header(gbtn, frontend_button_caption_text(gbtn));
}

void frontnet_draw_lobby_settings_header(struct GuiButton *gbtn)
{
    const char *text = frontend_button_caption_text(gbtn);
    if (net_lobby_mappack_list_open) {
        text = get_string(GUIStr_MnuMapPacks);
    }
    draw_lobby_section_header(gbtn, text);
}

void frontnet_draw_lobby_players_header(struct GuiButton *gbtn)
{
    const char *text[] = {get_string(GUIStr_MnuPlayers), get_string(GUIStr_NetTeam), get_string(GUIStr_NetPing)};
    draw_lobby_column_tabs(gbtn, lobby_player_columns, LobbyPlayerColumn_Count, text);
}

void frontnet_draw_lobby_name(struct GuiButton *gbtn)
{
    int font_index = lobby_font(gbtn, (gbtn == input_button) || (gbtn->btype_value == frontend_mouse_over_button));
    const char *cursor = "";
    if (gbtn == input_button) {
        cursor = "_";
    }
    char text[sizeof(net_lobby_settings.net_lobby_name) + 1];
    snprintf(text, sizeof(text), "%s%s", gbtn->content.str, cursor);
    draw_lobby_text_right_aligned(gbtn->scr_pos_x + gbtn->width, gbtn->scr_pos_y, gbtn->width / 2, gbtn->height, font_index, gbtn == input_button, text);
}

static const unsigned char *lobby_highlight_remap(TbBool highlighted)
{
    static unsigned char remaps[2][PALETTE_COLORS];
    static TbBool remaps_initialized;
    if (!remaps_initialized) {
        unsigned char highlight_palette[PALETTE_SIZE];
        for (int i = 0; i < PALETTE_SIZE; i++) {
            highlight_palette[i] = min(LOBBY_PALETTE_MAX_INTENSITY, frontend_palette[i] * 5 / 4);
        }
        make_palette_remap(remaps[0], frontend_palette, frontend_palette);
        make_palette_remap(remaps[1], highlight_palette, frontend_palette);
        remaps_initialized = true;
    }
    return remaps[highlighted];
}

static struct GuiButton mappack_selector_text_area(struct GuiButton *gbtn)
{
    int scale = lobby_menu_scale(gbtn);
    struct GuiButton text_area = *gbtn;
    text_area.scr_pos_x += LOBBY_SECTION_TEXT_INDENT * scale / 16;
    text_area.width -= (LOBBY_SECTION_TEXT_INDENT + LOBBY_SELECTOR_TEXT_RIGHT_INSET) * scale / 16;
    return text_area;
}

static void draw_mappack_selector(struct GuiButton *gbtn)
{
    RendererSetDrawFlags(0);
    frontnet_draw_scroll_selection_box(gbtn, 0, NULL);
    int font_index = lobby_font(gbtn, net_lobby_mappack_list_open || (frontend_mouse_over_button == gbtn->content.lval));
    struct GuiButton text_area = mappack_selector_text_area(gbtn);
    draw_lobby_text(text_area.scr_pos_x, text_area.scr_pos_y, text_area.width, text_area.height, font_index, 0, 0, campaign.display_name);
}

void frontnet_draw_lobby_settings_panel(struct GuiButton *gbtn)
{
    draw_mappack_selector(get_gui_button(BID_LOBBY_MAPPACK_SELECTOR));
    frontnet_draw_lobby_panel(gbtn);
}

void frontnet_draw_lobby_option(struct GuiButton *gbtn)
{
    int font_index = lobby_font(gbtn, frontend_mouse_over_button == gbtn->content.lval);
    char value_text[LOBBY_NUMBER_TEXT_LENGTH];
    const char *value = lobby_option_value(gbtn->btype_value, value_text, sizeof(value_text));
    int32_t value_width = gbtn->width / 2;
    if (value != NULL) {
        value_width = draw_lobby_text_right_aligned(gbtn->scr_pos_x + gbtn->width, gbtn->scr_pos_y, gbtn->width / 2, gbtn->height, font_index, false, value);
    }
    int32_t label_width = gbtn->width - value_width - LOBBY_OPTION_TEXT_GAP * lobby_menu_scale(gbtn) / 16;
    draw_lobby_text(gbtn->scr_pos_x, gbtn->scr_pos_y, label_width, gbtn->height, font_index, 0, 0, get_string(lobby_option_labels[gbtn->btype_value]));
}

void frontnet_mappack_up(struct GuiButton *gbtn)
{
    if (net_lobby_mappack_scroll_offset > 0) {
        net_lobby_mappack_scroll_offset--;
    }
}

void frontnet_mappack_down(struct GuiButton *gbtn)
{
    if (net_lobby_mappack_scroll_offset < lobby_mappack_list_layout().max_scroll_offset) {
        net_lobby_mappack_scroll_offset++;
    }
}

void frontnet_mappack_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Visible * net_lobby_mappack_list_open) & LbBtnF_Visible;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_lobby_mappack_scroll_offset != 0)) & LbBtnF_Enabled;
}

void frontnet_mappack_down_maintain(struct GuiButton *gbtn)
{
    struct LobbyMappackListLayout layout = lobby_mappack_list_layout();
    place_lobby_button(gbtn, net_lobby_mappack_list_open, LOBBY_MAPPACK_LIST_Y + LOBBY_MAPPACK_LIST_HEIGHT - LOBBY_SCROLL_BUTTON_OVERLAP, LOBBY_SCROLL_BUTTON_HEIGHT);
    net_lobby_mappack_scroll_offset = min(net_lobby_mappack_scroll_offset, layout.max_scroll_offset);
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_lobby_mappack_scroll_offset < layout.max_scroll_offset)) & LbBtnF_Enabled;
}

void frontnet_mappack_scroll(struct GuiButton *gbtn)
{
    net_lobby_mappack_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), 0, lobby_mappack_list_layout().max_scroll_offset + 1);
}

void frontnet_maintain_mappack_scroll_tab(struct GuiButton *gbtn)
{
    place_lobby_button(gbtn, net_lobby_mappack_list_open && (lobby_mappack_list_layout().max_scroll_offset > 0), LOBBY_MAPPACK_LIST_Y + LOBBY_SCROLL_BUTTON_OVERLAP, LOBBY_MAPPACK_LIST_HEIGHT - 2 * LOBBY_SCROLL_BUTTON_OVERLAP);
}

void frontnet_draw_mappack_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_lobby_mappack_scroll_offset, 0, lobby_mappack_list_layout().max_scroll_offset + 1);
}

static void scroll_section_with_wheel(struct GuiButton *gbtn, Gf_Btn_Callback scroll_up, Gf_Btn_Callback scroll_down)
{
    if (!mouse_is_over_lobby_area(gbtn, true)) {
        return;
    }
    if (wheel_scrolled_up) {
        scroll_up(gbtn);
    }
    if (wheel_scrolled_down) {
        scroll_down(gbtn);
    }
}

void frontnet_services_wheel_maintain(struct GuiButton *gbtn)
{
    scroll_section_with_wheel(gbtn, frontnet_service_up, frontnet_service_down);
}

void frontnet_sessions_wheel_maintain(struct GuiButton *gbtn)
{
    scroll_section_with_wheel(gbtn, frontnet_session_up, frontnet_session_down);
}

void frontnet_messages_wheel_maintain(struct GuiButton *gbtn)
{
    scroll_section_with_wheel(gbtn, frontnet_messages_up, frontnet_messages_down);
}

static int32_t lobby_player_column_at_mouse(struct GuiButton *gbtn)
{
    int32_t mouse_x = (GetMouseX() - gbtn->scr_pos_x) * lobby_player_columns[LobbyPlayerColumn_Count] / gbtn->width + LOBBY_COLUMN_TAB_PADDING;
    int32_t column = LobbyPlayerColumn_Name;
    while (column + 1 < LobbyPlayerColumn_Count && mouse_x >= lobby_player_columns[column + 1]) {
        column++;
    }
    return column;
}

static TbBool lobby_player_cell_editable(NetUserId user, int32_t column)
{
    TbBool player_editable = net_player_colour[user] >= 0 && (network_is_host() || user == my_player_number);
    if (column == LobbyPlayerColumn_Name) {
        return player_editable;
    }
    if (column == LobbyPlayerColumn_Team) {
        return frontend_alliances != -1 && (player_editable || (network_is_host() && net_lobby_computer_player(user)));
    }
    return false;
}

static NetUserId lobby_editable_player_at_mouse(struct GuiButton *gbtn)
{
    if (!mouse_is_over_lobby_area(gbtn, false)) {
        return -1;
    }
    NetUserId user = (GetMouseY() - gbtn->scr_pos_y) * 16 / (LOBBY_ROW_HEIGHT * lobby_menu_scale(gbtn));
    if (user >= net_lobby_max_players || !lobby_player_cell_editable(user, lobby_player_column_at_mouse(gbtn))) {
        return -1;
    }
    return user;
}

void frontnet_maintain_net_start_players(struct GuiButton *gbtn)
{
    gbtn->height = (net_lobby_max_players * LOBBY_ROW_HEIGHT * lobby_menu_scale(gbtn) + 8) / 16;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (lobby_editable_player_at_mouse(gbtn) >= 0)) & LbBtnF_Enabled;
}

void frontnet_change_player_setting(struct GuiButton *gbtn)
{
    NetUserId user = lobby_editable_player_at_mouse(gbtn);
    if (user < 0) {
        return;
    }
    enum NetPlayerSettingKind kind = NetPlayerSetting_Colour;
    if (lobby_player_column_at_mouse(gbtn) == LobbyPlayerColumn_Team) {
        kind = NetPlayerSetting_Team;
    }
    frontnet_cycle_player_setting(kind, user, gbtn->button_state_right_pressed);
}

void frontnet_draw_net_start_players(struct GuiButton *gbtn)
{
    const struct TbSprite *icon = get_frontend_sprite(GFS_bullfrog_red_med);
    int height = (LOBBY_ROW_HEIGHT * lobby_menu_scale(gbtn) + 8) / 16;
    int icon_scale = height * 16 / (icon->SHeight * 13 / 8);
    int icon_width = icon->SWidth * icon_scale / 16;
    int team_scale = height * 16 / get_frontend_sprite(GFS_team_1)->SHeight;
    int text_gap = LOBBY_COLUMN_TEXT_GAP * gbtn->width / lobby_player_columns[LobbyPlayerColumn_Count];
    int column_x[LobbyPlayerColumn_Count + 1];
    for (int32_t column = 0; column <= LobbyPlayerColumn_Count; column++) {
        column_x[column] = gbtn->scr_pos_x + lobby_player_columns[column] * gbtn->width / lobby_player_columns[LobbyPlayerColumn_Count];
    }
    column_x[LobbyPlayerColumn_Name] = gbtn->scr_pos_x + LOBBY_PLAYER_ROW_INSET * gbtn->width / lobby_player_columns[LobbyPlayerColumn_Count];
    NetUserId user_at_mouse = lobby_editable_player_at_mouse(gbtn);
    int32_t column_at_mouse = lobby_player_column_at_mouse(gbtn);
    int font = frontend_button_caption_font(gbtn, 0);
    unsigned char *previous_glass_map = lbDisplay.GlassMap;
    unsigned char *previous_render_ghost = render_ghost;
    lbDisplay.GlassMap = get_frontend_lobby_glass_map();
    render_ghost = lbDisplay.GlassMap;
    RendererSetDrawFlags(0);
    for (NetUserId user = 0; user < net_lobby_max_players; user++) {
        int y = gbtn->scr_pos_y + user * height;
        frontnet_draw_player_icon(user, column_x[LobbyPlayerColumn_Name], y + (height - icon->SHeight * icon_scale / 16) / 2, icon_scale);
        TbBool computer = net_lobby_computer_player(user);
        if (net_player_colour[user] < 0 && !computer) {
            continue;
        }
        char ping[LOBBY_NUMBER_TEXT_LENGTH] = "-";
        const char *name = network_user_name(user);
        unsigned short draw_flags = 0;
        if (computer) {
            name = get_string(GUIStr_MnuComputer);
            draw_flags = Lb_SPRITE_TRANSPAR4;
        } else if (user != my_player_number && GetPing(user) > 0) {
            snprintf(ping, sizeof(ping), "%u", GetPing(user));
        }
        int name_font = font;
        if (user == user_at_mouse && column_at_mouse == LobbyPlayerColumn_Name) {
            name_font = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
        }
        int name_x = column_x[LobbyPlayerColumn_Name] + icon_width;
        draw_lobby_text(name_x, y, column_x[LobbyPlayerColumn_Team] - name_x - text_gap, height, name_font, draw_flags, 0, name);
        draw_lobby_text(column_x[LobbyPlayerColumn_Ping], y, column_x[LobbyPlayerColumn_Count] - column_x[LobbyPlayerColumn_Ping] - text_gap, height, font, draw_flags, 0, ping);
        if (net_player_team[user] >= 0) {
            const struct TbSprite *team_icon = get_frontend_sprite(GFS_random_team);
            if (net_player_team[user] < NET_TEAM_RANDOM) {
                team_icon = get_frontend_sprite(GFS_team_1 + net_player_team[user]);
            }
            RendererSetDrawFlags(draw_flags);
            int team_x = (column_x[LobbyPlayerColumn_Team] + column_x[LobbyPlayerColumn_Ping]) / 2 - text_gap - team_icon->SWidth * team_scale / 32;
            int team_y = y + (height - team_icon->SHeight * team_scale / 16) / 2;
            LbSpriteDrawResizedRemap(team_x, team_y, team_scale, team_icon, lobby_highlight_remap(user == user_at_mouse && column_at_mouse == LobbyPlayerColumn_Team));
        }
        RendererSetDrawFlags(0);
    }
    lbDisplay.GlassMap = previous_glass_map;
    render_ghost = previous_render_ghost;
}

static void select_mp_mappack(int32_t index)
{
    const char *fname = mp_mappacks_list.items[index].fname;
    if (strcasecmp(fname, campaign.fname) == 0) {
        return;
    }
    frontnet_send_campaign_change_message(fname);
    change_campaign(CampgnT_MultiplayerMappack, fname);
}

static struct GuiButton lobby_mappack_list_panel(struct GuiButton *gbtn)
{
    struct GuiMenu *gmnu = get_active_menu(gbtn->gmenu_idx);
    int scale = lobby_menu_scale(gbtn);
    struct GuiButton panel = *gbtn;
    panel.scr_pos_x = gmnu->pos_x + LOBBY_RIGHT_X * scale / 16;
    panel.scr_pos_y = gmnu->pos_y + (LOBBY_MAPPACK_LIST_Y * scale + 8) / 16;
    panel.width = LOBBY_RIGHT_WIDTH * scale / 16;
    panel.height = (LOBBY_MAPPACK_LIST_HEIGHT * scale + 8) / 16;
    return panel;
}

static int32_t lobby_mappack_list_item_at_mouse(struct GuiButton *panel)
{
    int32_t row_y = (GetMouseY() - panel->scr_pos_y) * 16 / lobby_menu_scale(panel) - LOBBY_PANEL_PADDING;
    int32_t rows = lobby_mappack_list_layout().rows;
    if (GetMouseX() < panel->scr_pos_x || GetMouseX() >= panel->scr_pos_x + panel->width || row_y < 0 || row_y >= rows * LOBBY_ROW_HEIGHT) {
        return -1;
    }
    return net_lobby_mappack_scroll_offset + row_y / LOBBY_ROW_HEIGHT;
}

void frontnet_open_mappack_list(struct GuiButton *gbtn)
{
    const struct GameCampaign *selected = find_campaign_in_list(campaign.fname, &mp_mappacks_list);
    net_lobby_mappack_scroll_offset = 0;
    if (selected != NULL) {
        net_lobby_mappack_scroll_offset = max(0, (int32_t)(selected - mp_mappacks_list.items) - lobby_mappack_list_layout().rows + 1);
    }
    net_lobby_mappack_list_open = true;
}

void frontnet_maintain_mappack_list(struct GuiButton *gbtn)
{
    if (!network_is_host() || (mp_mappacks_list.items_num == 0)) {
        net_lobby_mappack_list_open = false;
    }
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Visible * net_lobby_mappack_list_open) & LbBtnF_Visible;
    if (net_lobby_mappack_list_open) {
        scroll_section_with_wheel(gbtn, frontnet_mappack_up, frontnet_mappack_down);
    }
}

static struct GuiButton lobby_mappack_list_row(struct GuiButton *gbtn)
{
    struct GuiMenu *gmnu = get_active_menu(gbtn->gmenu_idx);
    int scale = lobby_menu_scale(gbtn);
    struct GuiButton row = *gbtn;
    row.scr_pos_x = gmnu->pos_x + LOBBY_OPTION_X * scale / 16;
    row.width = LOBBY_OPTION_WIDTH * scale / 16;
    row.height = (LOBBY_ROW_HEIGHT * scale + 8) / 16;
    return row;
}

static void draw_lobby_tooltip(struct GuiButton *text_area, const char *text)
{
    int32_t text_width = lobby_text_width(text_area->height, 1, text);
    if (text_width <= text_area->width) {
        return;
    }
    int scale = lobby_menu_scale(text_area);
    int padding = 7 * scale / 16;
    int width = min(text_width + padding * 2, (int)lbDisplay.GraphicsScreenWidth);
    int height = text_area->height + padding * 2;
    int x = max(0, min(GetMouseX() + padding, (int)lbDisplay.GraphicsScreenWidth - width));
    int y = max(0, GetMouseY() - height - padding);
    draw_lobby_panel(x, y, width, height, scale);
    draw_lobby_text(x + padding, y + padding, width - padding * 2, text_area->height, 1, 0, 0, text);
}

void frontnet_draw_lobby_tooltips(struct GuiButton *gbtn)
{
    if (input_button != NULL) {
        return;
    }
    if (net_lobby_mappack_list_open) {
        struct GuiButton panel = lobby_mappack_list_panel(gbtn);
        int32_t hovered = lobby_mappack_list_item_at_mouse(&panel);
        if (hovered >= 0) {
            struct GuiButton row = lobby_mappack_list_row(gbtn);
            draw_lobby_tooltip(&row, mp_mappacks_list.items[hovered].display_name);
        }
        return;
    }
    if (mouse_is_over_lobby_area(gbtn, false)) {
        draw_lobby_tooltip(gbtn, net_lobby_settings.net_lobby_name);
    }
    struct GuiButton *selector = get_gui_button(BID_LOBBY_MAPPACK_SELECTOR);
    if (mouse_is_over_lobby_area(selector, false)) {
        struct GuiButton text_area = mappack_selector_text_area(selector);
        draw_lobby_tooltip(&text_area, campaign.display_name);
    }
}

void frontnet_draw_mappack_list(struct GuiButton *gbtn)
{
    struct GuiButton panel = lobby_mappack_list_panel(gbtn);
    int scale = lobby_menu_scale(gbtn);
    frontnet_draw_lobby_panel(&panel);
    int32_t hovered = lobby_mappack_list_item_at_mouse(&panel);
    struct GuiButton row = lobby_mappack_list_row(gbtn);
    int32_t rows = lobby_mappack_list_layout().rows;
    for (int32_t i = 0; i < rows; i++) {
        int32_t index = net_lobby_mappack_scroll_offset + i;
        const struct GameCampaign *mappack = &mp_mappacks_list.items[index];
        int font_index = 1;
        if ((index == hovered) || (strcasecmp(mappack->fname, campaign.fname) == 0)) {
            font_index = 2;
        }
        row.scr_pos_y = panel.scr_pos_y + ((LOBBY_PANEL_PADDING + i * LOBBY_ROW_HEIGHT) * scale + 8) / 16;
        draw_lobby_text(row.scr_pos_x, row.scr_pos_y, row.width, row.height, font_index, 0, 0, mappack->display_name);
    }
}

void frontnet_mappack_list_click(struct GuiButton *gbtn)
{
    struct GuiButton panel = lobby_mappack_list_panel(gbtn);
    int32_t index = lobby_mappack_list_item_at_mouse(&panel);
    if (index >= 0) {
        select_mp_mappack(index);
    }
    if ((index >= 0) || !mouse_is_over_lobby_area(&panel, true)) {
        net_lobby_mappack_list_open = false;
    }
}

void frontnet_change_lobby_option(struct GuiButton *gbtn)
{
    int32_t option = gbtn->btype_value;
    if (option == LobbyOption_ComputerPlayers) {
        frontend_toggle_computer_players(gbtn);
        return;
    }
    if (option == LobbyOption_MaximumPlayers) {
        int32_t min_players = net_lobby_min_players();
        if (gbtn->button_state_right_pressed) {
            net_config_info.max_players--;
        } else {
            net_config_info.max_players++;
        }
        if (net_config_info.max_players > MAX_NET_USERS) {
            net_config_info.max_players = min_players;
        } else if (net_config_info.max_players < min_players) {
            net_config_info.max_players = MAX_NET_USERS;
        }
    } else {
        uint8_t *toggle = lobby_option_toggle(&net_config_info, option);
        if (toggle == NULL) {
            return;
        }
        *toggle = !*toggle;
    }
    net_write_config_file();
    net_lobby_apply_settings();
}

void frontnet_session_set_lobby_name(struct GuiButton *gbtn)
{
    const char *name = net_lobby_settings.net_lobby_name;
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
    net_write_config_file();
    net_lobby_apply_settings();
}

void frontnet_session_create(struct GuiButton *gbtn)
{
    if (net_config_info.net_lobby_name[0] == '\0') {
        snprintf(net_config_info.net_lobby_name, sizeof(net_config_info.net_lobby_name), "Dungeon%03d", rand() % 1000);
        net_write_config_file();
    }
    uint32_t plyr_num;
    if (LbNetwork_Create(net_player_name, &plyr_num, nullptr)) {
        process_network_error(-801);
        return;
    }
    frontend_set_player_number(plyr_num);
    fe_computer_players = net_config_info.computer_players;
    frontend_set_state(FeSt_NET_START);
}
