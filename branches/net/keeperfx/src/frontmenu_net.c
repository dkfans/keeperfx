/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_net.c
 *     GUI menus for network support.
 * @par Purpose:
 *     Functions to show and maintain network screens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontmenu_net.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_datetm.h"
#include "bflib_guibtns.h"
#include "front_network.h"
#include "bflib_netsp.hpp"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "frontend.h"
#include "net_game.h"
#include "game_merge.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_frontnet_start_input(void);
DLLIMPORT void _DK_frontnet_serial_reset(void);
DLLIMPORT void _DK_frontnet_modem_reset(void);
DLLIMPORT void _DK_frontnet_draw_services_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_service_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_select(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_set_player_name(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_text_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_sessions_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_session_selected(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_select(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_session_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_players_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_players_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_players_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_players_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_players_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_net_session_players(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_join(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_create(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_return_to_main_menu(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_join_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_alliance_box_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_net_start_players(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_select_alliance(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_alliance_grid(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_alliance_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_maintain_alliance(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_messages_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_messages_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_messages_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_messages_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_bottom_scroll_box_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_current_message(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_messages(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_start_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_return_to_session_menu(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_small_scroll_box_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_small_scroll_box(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_comport_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_comport_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_comport_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_comport_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_comport_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_comport_selected(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_comport_select(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_comport_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_comport_select_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_speed_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_speed_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_speed_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_speed_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_speed_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_speed_selected(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_speed_select(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_speed_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_speed_select_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_text_cont_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_set_modem_init(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_set_modem_hangup(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_set_modem_dial(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_set_phone_number(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_modem_start(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_modem_start_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_set_modem_answer(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_serial_start(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_net_serial_start_maintain(struct GuiButton *gbtn);
/******************************************************************************/
struct GuiButtonInit frontend_net_service_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {10},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {12},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,180, frontnet_draw_scroll_box,          0, 201,  0,      {26},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_service_up,NULL,frontend_over_button,       0, 532, 149, 532, 149, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontnet_service_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_service_down,NULL,frontend_over_button,     0, 532, 317, 532, 317, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontnet_service_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 163, 536, 163, 10,154, frontnet_draw_services_scroll_tab, 0, 201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 125, 102, 125,220, 26, frontend_draw_text,                0, 201,  0,      {33},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 158,  95, 158,424, 26, frontnet_draw_service_button,      0, 201,  0,      {45},            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 184,  95, 184,424, 26, frontnet_draw_service_button,      0, 201,  0,      {46},            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 210,  95, 210,424, 26, frontnet_draw_service_button,      0, 201,  0,      {47},            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 236,  95, 236,424, 26, frontnet_draw_service_button,      0, 201,  0,      {48},            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 262,  95, 262,424, 26, frontnet_draw_service_button,      0, 201,  0,      {49},            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,frontend_over_button,   0,  95, 288,  95, 288,424, 26, frontnet_draw_service_button,      0, 201,  0,      {50},            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_net_session_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {12},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82,  79,  82,  79,165, 29, frontnet_draw_text_bar,            0, 201,  0,      {27},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95,  81,  91,  81,165, 25, frontend_draw_text,                0, 201,  0,      {19},            0, 0, NULL },
  { 5, -1,-1,-1, 0, frontnet_session_set_player_name,NULL,frontend_over_button,19,200,81,95,81,432, 25, frontend_draw_enter_text,          0, 201,  0,{(long)tmp_net_player_name}, 20, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 112,  82, 112,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 138,  82, 138,450,180, frontnet_draw_scroll_box,          0, 201,  0,      {25},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_session_up,NULL,       frontend_over_button,0, 532, 137, 532, 137, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontnet_session_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_session_down,NULL,     frontend_over_button,0, 532, 217, 532, 217, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontnet_session_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 151, 536, 151, 10, 66, frontnet_draw_sessions_scroll_tab, 0, 201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 113, 102, 113,220, 26, frontend_draw_text,                0, 201,  0,      {29},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 230,  82, 230,450, 23, frontnet_draw_session_selected,    0, 201,  0,      {35},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_session_select,NULL,   frontend_over_button,0,  95, 141,  95, 141,424, 26, frontnet_draw_session_button,      0, 201,  0,      {45},            0, 0, frontnet_session_maintain },
  { 0,  0, 0, 0, 0, frontnet_session_select,NULL,   frontend_over_button,0,  95, 167,  95, 167,424, 26, frontnet_draw_session_button,      0, 201,  0,      {46},            0, 0, frontnet_session_maintain },
  { 0,  0, 0, 0, 0, frontnet_session_select,NULL,   frontend_over_button,0,  95, 193,  95, 193,424, 26, frontnet_draw_session_button,      0, 201,  0,      {47},            0, 0, frontnet_session_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 261,  82, 261,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 287,  82, 287,450, 74, frontnet_draw_scroll_box,          0, 201,  0,      {24},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_players_up,NULL,       frontend_over_button,0, 532, 286, 532, 286, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {36},            0, 0, frontnet_players_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_players_down,NULL,     frontend_over_button,0, 532, 344, 532, 344, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {37},            0, 0, frontnet_players_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 300, 536, 300, 10, 44, frontnet_draw_players_scroll_tab,  0, 201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 262,  95, 262,220, 22, frontend_draw_text,                0, 201,  0,      {31},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 291,  82, 291,450, 52, frontnet_draw_net_session_players, 0, 201,  0,      {21},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_session_join,NULL,     frontend_over_button,0,  72, 360,  72, 360,247, 46, frontend_draw_small_menu_button,   0, 201,  0,      {13},            0, 0, frontnet_join_game_maintain },
  { 0,  0, 0, 0, 0, frontnet_session_create,NULL,   frontend_over_button,0, 321, 360, 321, 360,247, 46, frontend_draw_small_menu_button,   0, 201,  0,      {14},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_return_to_main_menu,NULL,frontend_over_button,0,999,404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_net_start_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 999,  30, 999,  30, 371, 46, frontend_draw_large_menu_button,   0, 201,  0,  {12}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82,  78,  82,  78, 220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 421,  81, 421,  81, 100, 27, frontnet_draw_alliance_box_tab,    0, 201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 104,  82, 104, 450, 70, frontnet_draw_scroll_box,          0, 201,  0,  {90}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 102,  79, 102,  79, 220, 26, frontend_draw_text,                0, 201,  0,  {31}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 105,  82, 105, 432,104, frontnet_draw_net_start_players,   0, 201,  0,  {21}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 107, 431, 116, 432, 88, frontnet_draw_alliance_grid,       0, 201,  0,  {74}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 108, 431, 108,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {74}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 108, 453, 108,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {74}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 108, 475, 108,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {74}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 108, 497, 108,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {74}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 134, 431, 134,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {75}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 134, 453, 134,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {75}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 134, 475, 134,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {75}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 134, 497, 134,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {75}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 160, 431, 160,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {76}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 160, 453, 160,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {76}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 160, 475, 160,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {76}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 160, 497, 160,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {76}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  0, 431, 186, 431, 183,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {77}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  1, 453, 186, 453, 186,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {77}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  2, 475, 186, 475, 186,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {77}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, frontnet_select_alliance,NULL,frontend_over_button,  3, 497, 186, 497, 186,  22, 26, frontnet_draw_alliance_button,     0, 201,  0,  {77}, 0, 0, frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 284, 217, 284, 217,   0,  0, frontnet_draw_bottom_scroll_box_tab,0,201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_toggle_computer_players,NULL,frontend_over_button,0,297,214,297,214,220,26, frontend_draw_computer_players,    0, 201,  0, {103}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 246,  82, 246, 220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 272,  82, 272, 450,111, frontnet_draw_scroll_box,          0, 201,  0,  {91}, 0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_messages_up,NULL,  frontend_over_button,    0, 532, 271, 532, 271,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {38}, 0, 0, frontnet_messages_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_messages_down,NULL,frontend_over_button,    0, 532, 373, 532, 373,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {39}, 0, 0, frontnet_messages_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 102, 247, 102, 247, 220, 26, frontend_draw_text,                0, 201,  0,  {34}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 536, 285, 536, 285,  10, 88, frontnet_draw_messages_scroll_tab, 0, 201,  0,  {40}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 386,  82, 386, 459, 23, frontnet_draw_current_message,     0, 201,  0,  {43}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  89, 273,  89, 273, 438,104, frontnet_draw_messages,            0, 201,  0,  {44}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, set_packet_start,   NULL,   frontend_over_button,    0,  49, 412,  49, 412, 247, 46, frontend_draw_small_menu_button,   0, 201,  0,  {15}, 0, 0, frontnet_start_game_maintain },
  { 0,  0, 0, 0, 0, frontnet_return_to_session_menu,NULL,frontend_over_button,1, 345,412,345,412,247,46, frontend_draw_small_menu_button,   0, 201,  0,  {16}, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,   0,  0,   {0}, 0, 0, NULL },
};

struct GuiButtonInit frontend_net_modem_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 999,  30, 999,  30, 371, 46, frontend_draw_large_menu_button,   0, 201,  0,  {53}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  41, 102,  41, 102, 212, 26, frontnet_draw_small_scroll_box_tab,0, 201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  41, 128,  41, 128, 268, 70, frontnet_draw_small_scroll_box,    0, 201,  0,  {24}, 0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_comport_up,NULL,   frontend_over_button,    0, 275, 128, 275, 128,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {17}, 0, 0, frontnet_comport_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_comport_down,NULL, frontend_over_button,    0, 275, 186, 275, 186,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {18}, 0, 0, frontnet_comport_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 279, 142, 279, 142,  22, 44, frontnet_draw_comport_scroll_tab,  0, 201,  0,  {40}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  61, 103,  61, 103, 172, 25, frontend_draw_text,                0, 201,  0,  {55}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  41, 198,  41, 198, 268, 23, frontnet_draw_comport_selected,    0, 201,  0,  {57}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_comport_select,NULL,frontend_over_button,   0,  54, 136,  54, 136, 190, 26, frontnet_draw_comport_button,      0, 201,  0,  {45}, 0, 0, frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, frontnet_comport_select,NULL,frontend_over_button,   0,  54, 164,  54, 164, 190, 26, frontnet_draw_comport_button,      0, 201,  0,  {46}, 0, 0, frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 331, 102, 331, 102, 212, 26, frontnet_draw_small_scroll_box_tab,0, 201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 331, 128, 331, 128, 268, 70, frontnet_draw_small_scroll_box,    0, 201,  0,  {24}, 0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_speed_up,  NULL,   frontend_over_button,    0, 565, 128, 565, 128,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {36}, 0, 0, frontnet_speed_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_speed_down,NULL,   frontend_over_button,    0, 565, 186, 565, 186,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {37}, 0, 0, frontnet_speed_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 569, 142, 569, 142,  22, 44, frontnet_draw_speed_scroll_tab,    0, 201,  0,  {40}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 351, 103, 351, 103, 172, 25, frontend_draw_text,                0, 201,  0,  {56}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 331, 198, 331, 198, 450, 23, frontnet_draw_speed_selected,      0, 201,  0,  {58}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_speed_select,NULL, frontend_over_button,    0, 344, 136, 344, 136, 190, 14, frontnet_draw_speed_button,        0, 201,  0,  {47}, 0, 0, frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, frontnet_speed_select,NULL, frontend_over_button,    0, 344, 164, 344, 164, 190, 14, frontnet_draw_speed_button,        0, 201,  0,  {48}, 0, 0, frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 254,  82, 254, 165, 28, frontnet_draw_text_cont_bar,       0, 201,  0,  {27}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 255,  91, 255, 165, 25, frontend_draw_text,                0, 201,  0,  {71}, 0, 0, NULL },
  { 5, -3,-1,-1, 0, frontnet_net_set_phone_number,NULL,frontend_over_button,71,280,255,95, 255, 432, 25, frontend_draw_enter_text,          0, 201,  0, {(long)tmp_net_phone_number}, 20, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 282,  82, 282, 165, 28, frontnet_draw_text_cont_bar,       0, 201,  0,  {27}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 283,  91, 283, 165, 25, frontend_draw_text,                0, 201,  0,  {66}, 0, 0, NULL },
  { 5, -1,-1,-1, 0, frontnet_net_set_modem_init,NULL,frontend_over_button,66,280,283,  95, 283, 432, 25, frontend_draw_enter_text,          0, 201,  0, {(long)tmp_net_modem_init}, -20, -1, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 310,  82, 310, 165, 28, frontnet_draw_text_cont_bar,       0, 201,  0,  {27}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 311,  91, 311, 165, 25, frontend_draw_text,                0, 201,  0,  {67}, 0, 0, NULL },
  { 5, -1,-1,-1, 0, frontnet_net_set_modem_hangup,NULL,frontend_over_button,67,280,311,95, 311, 432, 25, frontend_draw_enter_text,          0, 201,  0, {(long)tmp_net_modem_hangup}, -20, -1, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 338,  82, 338, 165, 28, frontnet_draw_text_cont_bar,       0, 201,  0,  {27}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 339,  91, 339, 165, 25, frontend_draw_text,                0, 201,  0,  {68}, 0, 0, NULL },
  { 5, -1,-1,-1, 0, frontnet_net_set_modem_dial,NULL,frontend_over_button,68,280,339,  95, 339, 432, 25, frontend_draw_enter_text,          0, 201,  0, {(long)tmp_net_modem_dial}, -20, -1, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 366,  82, 366, 165, 28, frontnet_draw_text_cont_bar,       0, 201,  0,  {27}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 367,  91, 367, 165, 25, frontend_draw_text,                0, 201,  0,  {69}, 0, 0, NULL },
  { 5, -1,-1,-1, 0, frontnet_net_set_modem_answer,NULL,frontend_over_button,69,280,367,95, 367, 432, 25, frontend_draw_enter_text,          0, 201,  0, {(long)tmp_net_modem_answer}, -20, -1, NULL },
  { 0,  0, 0, 0, 0, frontnet_net_modem_start,NULL,frontend_over_button,  0,  49, 412,  49, 412, 247, 46, frontend_draw_small_menu_button,   0, 201,  0,  {72}, 0, 0, frontnet_net_modem_start_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 345, 412, 345, 412, 247, 46, frontend_draw_small_menu_button,   0, 201,  0,  {16}, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,   0,  0,   {0}, 0, 0, NULL },
};

struct GuiButtonInit frontend_net_serial_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 999,  30, 999,  30, 371, 46, frontend_draw_large_menu_button,   0, 201,  0,  {54}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  41, 178,  41, 178, 212, 26, frontnet_draw_small_scroll_box_tab,0, 201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  41, 204,  41, 204, 268, 70, frontnet_draw_small_scroll_box,    0, 201,  0,  {24}, 0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_comport_up,NULL,   frontend_over_button,    0, 275, 204, 275, 204,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {17}, 0, 0, frontnet_comport_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_comport_down,NULL, frontend_over_button,    0, 275, 262, 275, 262,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {18}, 0, 0, frontnet_comport_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 279, 218, 279, 218,  22, 44, frontnet_draw_comport_scroll_tab,  0, 201,  0,  {40}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  61, 179,  61, 179, 172, 25, frontend_draw_text,                0, 201,  0,  {55}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  41, 274,  41, 274, 268, 23, frontnet_draw_comport_selected,    0, 201,  0,  {57}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_comport_select,NULL,frontend_over_button,   0,  54, 212,  54, 212, 190, 26, frontnet_draw_comport_button,      0, 201,  0,  {45}, 0, 0, frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, frontnet_comport_select,NULL,frontend_over_button,   0,  54, 240,  54, 240, 190, 26, frontnet_draw_comport_button,      0, 201,  0,  {46}, 0, 0, frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 331, 178, 331, 178, 212, 26, frontnet_draw_small_scroll_box_tab,0, 201,  0,  {28}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 331, 204, 331, 204, 268, 70, frontnet_draw_small_scroll_box,    0, 201,  0,  {24}, 0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_speed_up,NULL,     frontend_over_button,    0, 565, 204, 565, 204,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {36}, 0, 0, frontnet_speed_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_speed_down,NULL,   frontend_over_button,    0, 565, 262, 565, 262,  26, 14, frontnet_draw_slider_button,       0, 201,  0,  {37}, 0, 0, frontnet_speed_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 569, 218, 569, 218,  22, 44, frontnet_draw_speed_scroll_tab,    0, 201,  0,  {40}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 351, 179, 351, 179, 172, 25, frontend_draw_text,                0, 201,  0,  {56}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 331, 274, 331, 274, 450, 23, frontnet_draw_speed_selected,      0, 201,  0,  {58}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_speed_select,NULL, frontend_over_button,    0, 344, 212, 344, 212, 190, 26, frontnet_draw_speed_button,        0, 201,  0,  {47}, 0, 0, frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, frontnet_speed_select,NULL, frontend_over_button,    0, 344, 240, 344, 240, 190, 26, frontnet_draw_speed_button,        0, 201,  0,  {48}, 0, 0, frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, frontnet_net_serial_start,NULL,frontend_over_button, 0,  49, 412,  49, 412, 247, 46, frontend_draw_small_menu_button,   0, 201,  0,  {73}, 0, 0, frontnet_net_serial_start_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,    1, 345, 412, 345, 412, 247, 46, frontend_draw_small_menu_button,   0, 201,  0,  {16}, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,   0,  0,   {0}, 0, 0, NULL },
};

struct GuiMenu frontend_net_service_menu =
 { 20, 0, 1, frontend_net_service_buttons,        0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_session_menu =
 { 21, 0, 1, frontend_net_session_buttons,        0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_start_menu =
 { 22, 0, 1, frontend_net_start_buttons,          0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_modem_menu =
 { 23, 0, 1, frontend_net_modem_buttons,          0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_net_serial_menu =
 { 24, 0, 1, frontend_net_serial_buttons,         0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
/******************************************************************************/
const char *get_net_speed_text(int id)
{
  static const char *net_speed[] = {
     "9600",
    "14400",
    "19200",
    "28800",
    "38400",
    "57600",
   "115200",
     "ISDN",
  };
  const int limit = sizeof(net_speed)/sizeof(*net_speed) - 1;
  if (id < 0)
    id = 0;
  if (id > limit)
    id = limit;
  return net_speed[id];
}

long frontnet_number_of_players_in_session(void)
{
  long i,nplyr;
  nplyr = 0;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    if (network_player_active(i))
      nplyr++;
  }
  return nplyr;
}

void frontnet_session_up_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_session_up_maintain(gbtn);
}

void frontnet_session_down_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_session_down_maintain(gbtn);
}

void frontnet_session_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_session_maintain(gbtn);
}

void frontnet_players_up_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_players_up_maintain(gbtn);
}

void frontnet_players_down_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_players_down_maintain(gbtn);
}

void frontnet_join_game_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_join_game_maintain(gbtn);
}

void frontnet_maintain_alliance(struct GuiButton *gbtn)
{
  _DK_frontnet_maintain_alliance(gbtn);
}

void frontnet_messages_up_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_messages_up_maintain(gbtn);
}

void frontnet_messages_down_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_messages_down_maintain(gbtn);
}

void frontnet_start_game_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_start_game_maintain(gbtn);
}

void frontnet_comport_down_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_comport_down_maintain(gbtn);
}

void frontnet_comport_select_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_comport_select_maintain(gbtn);
}

void frontnet_speed_up_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_speed_up_maintain(gbtn);
}

void frontnet_speed_down_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_speed_down_maintain(gbtn);
}

void frontnet_speed_select_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_speed_select_maintain(gbtn);
}

void frontnet_net_modem_start_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_net_modem_start_maintain(gbtn);
}

void frontnet_comport_up_maintain(struct GuiButton *gbtn)
{
  _DK_frontnet_comport_up_maintain(gbtn);
}

void frontnet_net_serial_start_maintain(struct GuiButton *gbtn)
{
  if ((net_comport_index_active == -1) || (net_speed_index_active == -1))
    gbtn->field_0 &= 0xF7u;
  else
    gbtn->field_0 |= 0x08;
}

void frontnet_serial_reset(void)
{
  _DK_frontnet_serial_reset();
}

void frontnet_modem_reset(void)
{
  _DK_frontnet_modem_reset();
}

void frontnet_start_input(void)
{
  _DK_frontnet_start_input();
}

void frontnet_draw_services_scroll_tab(struct GuiButton *gbtn)
{
  frontend_draw_scroll_tab(gbtn, net_service_scroll_offset, 0, net_number_of_services);
}

void frontnet_session_set_player_name(struct GuiButton *gbtn)
{
  _DK_frontnet_session_set_player_name(gbtn);
}

void frontnet_draw_text_bar(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_text_bar(gbtn);
}

void frontnet_session_up(struct GuiButton *gbtn)
{
  _DK_frontnet_session_up(gbtn);
}

void frontnet_session_down(struct GuiButton *gbtn)
{
  _DK_frontnet_session_down(gbtn);
}

void frontnet_draw_sessions_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_sessions_scroll_tab(gbtn);
}

void frontnet_draw_session_selected(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_session_selected(gbtn);
}

void frontnet_session_select(struct GuiButton *gbtn)
{
  _DK_frontnet_session_select(gbtn);
}

void frontnet_draw_session_button(struct GuiButton *gbtn)
{
  //_DK_frontnet_draw_session_button(gbtn);
  long sessionIndex;
  long fontIndex;
  long btnIndex;
  long height;

  btnIndex = (long)gbtn->field_33;
  sessionIndex = net_session_scroll_offset + btnIndex - 45;
  if ((sessionIndex < 0) || (sessionIndex >= net_number_of_sessions))
      return;
  fontIndex = frontend_button_info[btnIndex%FRONTEND_BUTTON_INFO_COUNT].font_index;
  if ((btnIndex > 0) && (frontend_mouse_over_button == btnIndex)) {
      fontIndex = 2;
  }
  lbDisplay.DrawFlags = 0;
  LbTextSetFont(frontend_font[fontIndex]);
  height = LbTextLineHeight();
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, height);
  LbTextDraw(0, 0, net_session[sessionIndex]->text);
}

void frontnet_players_up(struct GuiButton *gbtn)
{
  _DK_frontnet_players_up(gbtn);
}

void frontnet_players_down(struct GuiButton *gbtn)
{
  _DK_frontnet_players_down(gbtn);
}

void frontnet_draw_players_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_players_scroll_tab(gbtn);
}

void frontnet_draw_net_session_players(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_net_session_players(gbtn);
}

void frontnet_session_join(struct GuiButton *gbtn)
{
  long plyr_num;
  plyr_num = network_session_join();
  if (plyr_num < 0)
      return;
  frontend_set_player_number(plyr_num);
  frontend_set_state(FeSt_NET_START);
}

void frontnet_session_create(struct GuiButton *gbtn)
{
  struct TbNetworkSessionNameEntry *nsname;
  unsigned long plyr_num;
  void *conn_options;
  char *text;
  char *txpos;
  long i,idx;
  idx = 0;
  for (i=0; i < net_number_of_sessions; i++)
  {
      nsname = net_session[i];
      if (nsname == NULL)
        continue;
      text = buf_sprintf("%s",nsname->text);
      txpos = strchr(text, '\'');
      if (txpos != NULL)
        *txpos = '\0';
      if (strcmp(text, net_player_name) != 0)
        idx++;
  }
  if (idx > 0)
    text = buf_sprintf("%s (%d)", net_player_name, idx+1);
  else
    text = buf_sprintf("%s", net_player_name);
  switch (net_service_index_selected)
  {
  case 1:
      modem_dev.field_0 = 0;
      modem_dev.field_4 = 0;
      strcpy(modem_dev.field_58, net_config_info.str_join);
      modem_dev.field_AC = modem_initialise_callback;
      modem_dev.field_B0 = modem_connect_callback;
      conn_options = &modem_dev;
      break;
  default:
      conn_options = NULL;
      break;
  }
  if (LbNetwork_Create(text, net_player_name, &plyr_num, conn_options))
  {
    if (net_service_index_selected == 1)
      process_network_error(modem_dev.field_A8);
    else
      process_network_error(-801);
    return;
  }
  frontend_set_player_number(plyr_num);
  fe_computer_players = 0;
  frontend_set_state(FeSt_NET_START);
}

void frontnet_return_to_main_menu(struct GuiButton *gbtn)
{
  if ( LbNetwork_Stop() )
  {
    ERRORLOG("LbNetwork_Stop() failed");
    return;
  }
  frontend_set_state(FeSt_MAIN_MENU);
}

void frontnet_draw_alliance_box_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_alliance_box_tab(gbtn);
}

void frontnet_draw_net_start_players(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_net_start_players(gbtn);
}

void frontnet_select_alliance(struct GuiButton *gbtn)
{
  _DK_frontnet_select_alliance(gbtn);
}

void frontnet_draw_alliance_grid(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_alliance_grid(gbtn);
}

void frontnet_draw_alliance_button(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_alliance_button(gbtn);
}

void frontnet_messages_up(struct GuiButton *gbtn)
{
  _DK_frontnet_messages_up(gbtn);
}

void frontnet_messages_down(struct GuiButton *gbtn)
{
  _DK_frontnet_messages_down(gbtn);
}

void frontnet_draw_bottom_scroll_box_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_bottom_scroll_box_tab(gbtn);
}

void frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_messages_scroll_tab(gbtn);
}

void frontnet_draw_current_message(struct GuiButton *gbtn)
{
  //_DK_frontnet_draw_current_message(gbtn);

  struct TbSprite * sprite;
  int draw_x;
  int i;
  unsigned char height;
  const char * format_string;
  char * text_to_print;
  int button_info_font_index;
  char text[2048];

  static int last_time = 0;
  static unsigned print_with_underscore = 1; //might in fact be a global, see original function, either way doesn't matter too much

  if ( LbTimerClock() >= last_time + 100 )
  {
    //weird code (why print with underscore after a certain time?)
    print_with_underscore = print_with_underscore < 1;
    last_time = LbTimerClock();
  }

  if ( print_with_underscore )
  {
    text_to_print = game.players[my_player_number].strfield_463;
    format_string = "%s_";
  }
  else
  {
    text_to_print = game.players[my_player_number].strfield_463;
    format_string = "%s";
  }

  snprintf(text, sizeof(text), format_string, text_to_print);
  sprite = &frontend_sprite[55];
  draw_x = gbtn->scr_pos_x;
  button_info_font_index = frontend_button_info[(unsigned) gbtn->field_33].font_index;
  for (i = 6; i > 0; --i)
  {
    LbSpriteDraw(draw_x, gbtn->scr_pos_y, sprite);
    draw_x += sprite->SWidth;
    ++sprite;
  }

  if ( text ) //hmm why?
  {
    lbDisplay.DrawFlags = 0;
    lbFontPtr = frontend_font[button_info_font_index];
    if ( frontend_font[button_info_font_index] )
      height = frontend_font[button_info_font_index][1].SHeight;
    else
      height = 0;
    LbTextSetWindow(gbtn->scr_pos_x + 13, gbtn->scr_pos_y, gbtn->width - 26, height);
    LbTextDraw(0, 0, text);
  }
}

void frontnet_draw_messages(struct GuiButton *gbtn)
{
  //_DK_frontnet_draw_messages(gbtn);

  struct TbSprite *font;
  int y;
  int font_index;
  struct NetMessage *message_ptr;
  int num_active;
  unsigned char font_height;
  struct TbSprite *player_sprite;
  long long height_diff;
  int scroll_offset;
  int i;

  y = 0;
  scroll_offset = net_message_scroll_offset;
  font_index = frontend_button_info[(unsigned) gbtn->field_33].font_index;
  lbDisplay.DrawFlags = 0;
  font = frontend_font[font_index];
  lbFontPtr = frontend_font[font_index];
  if ( gbtn->height )
  {
    message_ptr = &net_message[net_message_scroll_offset];
    do
    {
      *(short *) font = scroll_offset; //check this, seems weird
      if ( scroll_offset >= net_number_of_messages )
        break;
      num_active = 0;
      for (i = message_ptr->plyr_idx; i > 0; --i)
      {
        if ( net_player_info[i].active)
          ++num_active;
      }

      player_sprite = &frontend_sprite[num_active + 21];
      font_height = 0;
      if ( lbFontPtr )
        font_height = lbFontPtr[1].SHeight;

      height_diff = font_height - player_sprite->SHeight;
      LbSpriteDraw(gbtn->scr_pos_x, y + gbtn->scr_pos_y + (((unsigned)height_diff - (unsigned) (height_diff >> 32)) >> 1), player_sprite);

      LbTextSetWindow(gbtn->scr_pos_x, y + gbtn->scr_pos_y, gbtn->width, font_height);
      LbTextDraw(player_sprite->SWidth, 0, message_ptr->text);

      ++message_ptr;
      y += font_height;
      ++scroll_offset;
    }
    while ( y < gbtn->height );
  }
}

void frontnet_return_to_session_menu(struct GuiButton *gbtn)
{
  if ( LbNetwork_Stop() )
  {
    ERRORLOG("LbNetwork_Stop() failed");
    return;
  }
  if ( setup_network_service(net_service_index_selected) )
    frontend_set_state(FeSt_NET_SESSION);
  else
    frontend_set_state(FeSt_MAIN_MENU);
}

void frontnet_draw_small_scroll_box_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_small_scroll_box_tab(gbtn);
}

void frontnet_draw_small_scroll_box(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_small_scroll_box(gbtn);
}

void frontnet_comport_up(struct GuiButton *gbtn)
{
  _DK_frontnet_comport_up(gbtn);
}

void frontnet_comport_down(struct GuiButton *gbtn)
{
  _DK_frontnet_comport_down(gbtn);
}

void frontnet_draw_comport_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_comport_scroll_tab(gbtn);
}

void frontnet_draw_comport_selected(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_comport_selected(gbtn);
}

void frontnet_comport_select(struct GuiButton *gbtn)
{
  _DK_frontnet_comport_select(gbtn);
}

void frontnet_draw_comport_button(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_comport_button(gbtn);
}

void frontnet_speed_up(struct GuiButton *gbtn)
{
  _DK_frontnet_speed_up(gbtn);
}

void frontnet_speed_down(struct GuiButton *gbtn)
{
  _DK_frontnet_speed_down(gbtn);
}

void frontnet_draw_speed_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_speed_scroll_tab(gbtn);
}

void frontnet_draw_speed_selected(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_speed_selected(gbtn);
}

void frontnet_speed_select(struct GuiButton *gbtn)
{
  _DK_frontnet_speed_select(gbtn);
}

void frontnet_draw_speed_button(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_speed_button(gbtn);
}

void frontnet_draw_text_cont_bar(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_text_cont_bar(gbtn);
}

void frontnet_net_set_modem_init(struct GuiButton *gbtn)
{
  _DK_frontnet_net_set_modem_init(gbtn);
}

void frontnet_net_set_modem_hangup(struct GuiButton *gbtn)
{
  _DK_frontnet_net_set_modem_hangup(gbtn);
}

void frontnet_net_set_modem_dial(struct GuiButton *gbtn)
{
  _DK_frontnet_net_set_modem_dial(gbtn);
}

void frontnet_net_set_phone_number(struct GuiButton *gbtn)
{
  _DK_frontnet_net_set_phone_number(gbtn);
}

void frontnet_net_modem_start(struct GuiButton *gbtn)
{
  _DK_frontnet_net_modem_start(gbtn);
}

void frontnet_net_set_modem_answer(struct GuiButton *gbtn)
{
  _DK_frontnet_net_set_modem_answer(gbtn);
}

void frontnet_net_serial_start(struct GuiButton *gbtn)
{
    const char *net_speed_text;
    net_serial_data.field_0 = net_config_info.numfield_0;
    net_speed_text = get_net_speed_text(net_config_info.numfield_9);
    if (strcmp(net_speed_text, "ISDN") != 0)
    {
      net_serial_data.numfield_4 = atoi(net_speed_text);
    } else
    {
    ERRORLOG("ISDN not supported by Serial");
    }
  net_serial_data.field_8 = net_config_info.numfield_1[(unsigned char)net_config_info.numfield_0];
  net_serial_data.str_dial = NULL;
  net_serial_data.str_phone = NULL;
  net_serial_data.str_hang = NULL;
  net_serial_data.str_answr = NULL;
  setup_network_service(0);
}

void frontnet_service_up_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (net_service_scroll_offset != 0));
}

void frontnet_service_down_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (net_number_of_services-1 > net_service_scroll_offset));
}

void frontnet_service_up(struct GuiButton *gbtn)
{
  if ( net_service_scroll_offset>0 )
    net_service_scroll_offset--;
}

void frontnet_service_down(struct GuiButton *gbtn)
{
  if ( net_number_of_services-1 > net_service_scroll_offset )
    net_service_scroll_offset++;
}

void frontnet_service_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (net_service_scroll_offset+(long)gbtn->field_33-45 < net_number_of_services));
}

void frontnet_draw_service_button(struct GuiButton *gbtn)
{
  int srvidx;
  long fbinfo_idx;
  int fntidx;
  // Find and verify selected network service
  fbinfo_idx = (long)(gbtn->field_33);
  srvidx = fbinfo_idx + net_service_scroll_offset - 45;
  if (srvidx >= net_number_of_services)
    return;
  // Select font to draw
  fntidx = frontend_button_info[fbinfo_idx%FRONTEND_BUTTON_INFO_COUNT].font_index;
  if ((fbinfo_idx != 0) && (frontend_mouse_over_button == fbinfo_idx))
      fntidx = 2;
  LbTextSetFont(frontend_font[fntidx]);
  // Set drawing windsow
  int height;
  lbDisplay.DrawFlags = 0x0020;
  height = LbTextHeight(net_service[srvidx]);
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, height);
  //Draw the text
  LbTextDraw(0, 0, net_service[srvidx]);
}

void frontnet_service_select(struct GuiButton *gbtn)
{
  int srvidx;
  srvidx = (long)(gbtn->field_33) + net_service_scroll_offset - 45;
  if ( ((game.system_flags & GSF_AllowOnePlayer) != 0)
     && (srvidx+1 >= net_number_of_services) )
  {
    fe_network_active = 0;
    frontend_set_state(FeSt_NETLAND_VIEW);
  } else
  if (srvidx <= 0)
  {
    frontend_set_state(FeSt_NET_SERIAL);
  } else
// Special condition to skip 'modem' connection
  if (srvidx == 1)
  {
    setup_network_service(2);
  } else
  {
    setup_network_service(srvidx);
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
