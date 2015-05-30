/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_net.h
 *     Header file for frontmenu_net.c.
 * @par Purpose:
 *     GUI menus for network support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTMENU_NET_H
#define DK_FRONTMENU_NET_H

#include "globals.h"

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define frontend_services_menu_items_visible  6

#pragma pack(1)

struct GuiMenu;
struct GuiButton;

#pragma pack()
/******************************************************************************/
extern struct GuiMenu frontend_net_service_menu;
extern struct GuiMenu frontend_net_session_menu;
extern struct GuiMenu frontend_net_start_menu;
extern struct GuiMenu frontend_net_modem_menu;
extern struct GuiMenu frontend_net_serial_menu;
extern struct GuiMenu frontend_add_session_box;
/******************************************************************************/
void frontnet_session_up_maintain(struct GuiButton *gbtn);
void frontnet_session_down_maintain(struct GuiButton *gbtn);
void frontnet_session_maintain(struct GuiButton *gbtn);
void frontnet_players_up_maintain(struct GuiButton *gbtn);
void frontnet_players_down_maintain(struct GuiButton *gbtn);
void frontnet_join_game_maintain(struct GuiButton *gbtn);
void frontnet_maintain_alliance(struct GuiButton *gbtn);
void frontnet_messages_up_maintain(struct GuiButton *gbtn);
void frontnet_messages_down_maintain(struct GuiButton *gbtn);
void frontnet_start_game_maintain(struct GuiButton *gbtn);
void frontnet_comport_up_maintain(struct GuiButton *gbtn);
void frontnet_comport_down_maintain(struct GuiButton *gbtn);
void frontnet_comport_select_maintain(struct GuiButton *gbtn);
void frontnet_comport_select_maintain(struct GuiButton *gbtn);
void frontnet_speed_up_maintain(struct GuiButton *gbtn);
void frontnet_speed_down_maintain(struct GuiButton *gbtn);
void frontnet_speed_select_maintain(struct GuiButton *gbtn);
void frontnet_speed_select_maintain(struct GuiButton *gbtn);
void frontnet_net_modem_start_maintain(struct GuiButton *gbtn);
void frontnet_comport_up_maintain(struct GuiButton *gbtn);
void frontnet_comport_down_maintain(struct GuiButton *gbtn);
void frontnet_comport_select_maintain(struct GuiButton *gbtn);
void frontnet_comport_select_maintain(struct GuiButton *gbtn);
void frontnet_speed_up_maintain(struct GuiButton *gbtn);
void frontnet_speed_down_maintain(struct GuiButton *gbtn);
void frontnet_speed_select_maintain(struct GuiButton *gbtn);
void frontnet_net_serial_start_maintain(struct GuiButton *gbtn);
long frontnet_number_of_players_in_session(void);
void frontnet_serial_reset(void);
void frontnet_modem_reset(void);
TbBool frontnet_start_input(void);
void frontnet_draw_services_scroll_tab(struct GuiButton *gbtn);
void frontnet_draw_service_button(struct GuiButton *gbtn);
void frontnet_service_maintain(struct GuiButton *gbtn);
void frontnet_service_up_maintain(struct GuiButton *gbtn);
void frontnet_service_down_maintain(struct GuiButton *gbtn);
void frontnet_service_up(struct GuiButton *gbtn);
void frontnet_service_down(struct GuiButton *gbtn);
void frontnet_service_select(struct GuiButton *gbtn);
void frontnet_session_set_player_name(struct GuiButton *gbtn);
void frontnet_draw_text_bar(struct GuiButton *gbtn);
void frontnet_session_up(struct GuiButton *gbtn);
void frontnet_session_up_maintain(struct GuiButton *gbtn);
void frontnet_session_down(struct GuiButton *gbtn);
void frontnet_session_down_maintain(struct GuiButton *gbtn);
void frontnet_session_maintain(struct GuiButton *gbtn);
void frontnet_draw_sessions_scroll_tab(struct GuiButton *gbtn);
void frontnet_draw_session_selected(struct GuiButton *gbtn);
void frontnet_session_select(struct GuiButton *gbtn);
void frontnet_draw_session_button(struct GuiButton *gbtn);
void frontnet_players_up(struct GuiButton *gbtn);
void frontnet_players_up_maintain(struct GuiButton *gbtn);
void frontnet_players_down(struct GuiButton *gbtn);
void frontnet_players_down_maintain(struct GuiButton *gbtn);
void frontnet_draw_players_scroll_tab(struct GuiButton *gbtn);
void frontnet_draw_net_session_players(struct GuiButton *gbtn);
void frontnet_session_add(struct GuiButton *gbtn);
void frontnet_session_join(struct GuiButton *gbtn);
void frontnet_session_create(struct GuiButton *gbtn);
void frontnet_return_to_main_menu(struct GuiButton *gbtn);
void frontnet_add_session_done(struct GuiButton *gbtn);
void frontnet_add_session_back(struct GuiButton *gbtn);
void frontnet_join_game_maintain(struct GuiButton *gbtn);
void frontnet_draw_alliance_box_tab(struct GuiButton *gbtn);
void frontnet_draw_net_start_players(struct GuiButton *gbtn);
void frontnet_select_alliance(struct GuiButton *gbtn);
void frontnet_draw_alliance_grid(struct GuiButton *gbtn);
void frontnet_draw_alliance_button(struct GuiButton *gbtn);
void frontnet_maintain_alliance(struct GuiButton *gbtn);
void frontnet_messages_up(struct GuiButton *gbtn);
void frontnet_messages_up_maintain(struct GuiButton *gbtn);
void frontnet_messages_down(struct GuiButton *gbtn);
void frontnet_messages_down_maintain(struct GuiButton *gbtn);
void frontnet_draw_bottom_scroll_box_tab(struct GuiButton *gbtn);
void frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn);
void frontnet_draw_current_message(struct GuiButton *gbtn);
void frontnet_draw_messages(struct GuiButton *gbtn);
void frontnet_start_game_maintain(struct GuiButton *gbtn);
void frontnet_return_to_session_menu(struct GuiButton *gbtn);
void frontnet_draw_small_scroll_box_tab(struct GuiButton *gbtn);
void frontnet_draw_small_scroll_box(struct GuiButton *gbtn);
void frontnet_comport_up(struct GuiButton *gbtn);
void frontnet_comport_up_maintain(struct GuiButton *gbtn);
void frontnet_comport_down(struct GuiButton *gbtn);
void frontnet_comport_down_maintain(struct GuiButton *gbtn);
void frontnet_draw_comport_scroll_tab(struct GuiButton *gbtn);
void frontnet_draw_comport_selected(struct GuiButton *gbtn);
void frontnet_comport_select(struct GuiButton *gbtn);
void frontnet_draw_comport_button(struct GuiButton *gbtn);
void frontnet_comport_select_maintain(struct GuiButton *gbtn);
void frontnet_speed_up(struct GuiButton *gbtn);
void frontnet_speed_up_maintain(struct GuiButton *gbtn);
void frontnet_speed_down(struct GuiButton *gbtn);
void frontnet_speed_down_maintain(struct GuiButton *gbtn);
void frontnet_draw_speed_scroll_tab(struct GuiButton *gbtn);
void frontnet_draw_speed_selected(struct GuiButton *gbtn);
void frontnet_speed_select(struct GuiButton *gbtn);
void frontnet_draw_speed_button(struct GuiButton *gbtn);
void frontnet_speed_select_maintain(struct GuiButton *gbtn);
void frontnet_draw_text_cont_bar(struct GuiButton *gbtn);
void frontnet_net_set_modem_init(struct GuiButton *gbtn);
void frontnet_net_set_modem_hangup(struct GuiButton *gbtn);
void frontnet_net_set_modem_dial(struct GuiButton *gbtn);
void frontnet_net_set_phone_number(struct GuiButton *gbtn);
void frontnet_net_modem_start(struct GuiButton *gbtn);
void frontnet_net_modem_start_maintain(struct GuiButton *gbtn);
void frontnet_net_set_modem_answer(struct GuiButton *gbtn);
void frontnet_net_serial_start(struct GuiButton *gbtn);
void frontnet_net_serial_start_maintain(struct GuiButton *gbtn);
void frontnet_service_select(struct GuiButton *gbtn);
void frontnet_service_up_maintain(struct GuiButton *gbtn);
void frontnet_service_down_maintain(struct GuiButton *gbtn);
void frontnet_service_up(struct GuiButton *gbtn);
void frontnet_service_down(struct GuiButton *gbtn);
void frontnet_service_maintain(struct GuiButton *gbtn);
void frontnet_draw_service_button(struct GuiButton *gbtn);

#define frontnet_draw_scroll_box_tab frontend_draw_scroll_box_tab
#define frontnet_draw_scroll_box frontend_draw_scroll_box
#define frontnet_draw_slider_button frontend_draw_slider_button
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
