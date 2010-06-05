/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontend.cpp
 *     Frontend menu implementation for Dungeon Keeper.
 * @par Purpose:
 *     Functions to display and maintain the game menu.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2008 - 01 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontend.h"

#include <string.h>
#include "bflib_basics.h"
#include "globals.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_dernc.h"
#include "bflib_datetm.h"
#include "bflib_keybrd.h"
#include "bflib_sndlib.h"
#include "bflib_mouse.h"
#include "bflib_vidraw.h"
#include "bflib_fileio.h"
#include "bflib_memory.h"
#include "bflib_filelst.h"
#include "bflib_sound.h"
#include "bflib_network.h"
#include "bflib_netsp.hpp"
#include "config.h"
#include "config_campaigns.h"
#include "config_creature.h"
#include "scrcapt.h"
#include "gui_draw.h"
#include "kjm_input.h"
#include "vidmode.h"
#include "front_simple.h"
#include "front_input.h"
#include "game_saves.h"
#include "engine_render.h"
#include "front_landview.h"
#include "front_credits.h"
#include "lvl_filesdk1.h"
#include "player_instances.h"

#include <windows.h>
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_frontnet_service_update(void);
DLLIMPORT void _DK_frontnet_session_update(void);
DLLIMPORT void _DK_frontnet_start_update(void);
DLLIMPORT void _DK_frontnet_modem_update(void);
DLLIMPORT void _DK_frontnet_serial_update(void);
//DLLIMPORT void * __cdecl _DK_frontnet_session_join(GuiButton); (may be incorrect)
DLLIMPORT void _DK_add_message(long plyr_idx, char *msg);
DLLIMPORT unsigned long _DK_validate_versions(void);
DLLIMPORT void _DK_versions_different_error(void);
DLLIMPORT char _DK_get_button_area_input(struct GuiButton *gbtn, int);
DLLIMPORT void _DK_fake_button_click(long btn_idx);
DLLIMPORT void _DK_turn_off_roaming_menus(void);
DLLIMPORT void _DK_display_objectives(long,long,long);
DLLIMPORT unsigned long _DK_toggle_status_menu(unsigned long);
DLLIMPORT void _DK_frontstats_update(void);
DLLIMPORT void _DK_fronttorture_update(void);
DLLIMPORT void _DK_frontcredits_draw(void);
DLLIMPORT void _DK_fronttorture_draw(void);
DLLIMPORT void _DK_frontnet_start_input(void);
DLLIMPORT void _DK_frontend_high_score_table_input(void);
DLLIMPORT void _DK_fronttorture_input(void);
DLLIMPORT void _DK_frontstats_set_timer(void);
DLLIMPORT int _DK_frontend_load_data(void);
DLLIMPORT void _DK_frontend_set_player_number(long plr_num);
DLLIMPORT void _DK_frontnet_serial_reset(void);
DLLIMPORT void _DK_frontnet_modem_reset(void);
DLLIMPORT void _DK_fronttorture_unload(void);
DLLIMPORT void _DK_fronttorture_load(void);
DLLIMPORT void _DK_frontnet_service_setup(void);
DLLIMPORT void _DK_frontnet_session_setup(void);
DLLIMPORT void _DK_frontnet_start_setup(void);
DLLIMPORT void _DK_frontnet_modem_setup(void);
DLLIMPORT void _DK_frontnet_serial_setup(void);
DLLIMPORT void _DK_turn_off_menu(char);
DLLIMPORT void _DK_turn_on_menu(int);//char);
DLLIMPORT void _DK_initialise_tab_tags_and_menu(long menu_id);
DLLIMPORT void _DK_turn_off_event_box_if_necessary(long plridx, char val);
DLLIMPORT void _DK_frontstats_initialise(void);
DLLIMPORT void _DK_frontend_save_continue_game(long lv_num, int a2);
DLLIMPORT unsigned char _DK_a_menu_window_is_active(void);
DLLIMPORT unsigned long _DK_turn_off_all_window_menus(void);
DLLIMPORT char _DK_game_is_busy_doing_gui(void);
DLLIMPORT char _DK_menu_is_active(char idx);
DLLIMPORT void _DK_get_player_gui_clicks(void);
DLLIMPORT void _DK_init_gui(void);
DLLIMPORT void _DK_gui_area_text(struct GuiButton *gbtn);
DLLIMPORT void _DK_turn_off_all_panel_menus(void);
DLLIMPORT void _DK_spell_lost_first_person(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_turn_on_autopilot(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_autopilot(struct GuiButton *gbtn);
DLLIMPORT char _DK_update_menu_fade_level(struct GuiMenu *gmnu);
DLLIMPORT void _DK_draw_menu_buttons(struct GuiMenu *gmnu);
DLLIMPORT void _DK_draw_load_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_null(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_button(struct GuiButton *gbtn, long a2, const char *text, long a4);
DLLIMPORT char _DK_create_menu(struct GuiMenu *mnu);
DLLIMPORT char _DK_create_button(struct GuiMenu *gmnu, struct GuiButtonInit *gbinit);
DLLIMPORT void _DK_maintain_event_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_menu_tab_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_turn_on_autopilot(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_zoom_in(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_zoom_out(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_map(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_new_normal_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_autopilot_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_menu_mode(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_draw_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_open_event(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_kill_event(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_event_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_over_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_new_null_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_new_no_anim_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_remove_area_for_rooms(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_big_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_spell_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_special_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_big_spell_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_over_trap_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_trap_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_door(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_door(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_over_door_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_remove_area_for_traps(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_big_trap_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_no_anim_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_loadsave(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_normal_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_new_normal_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_tend_to(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_flash_cycle_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_prison_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_flash_cycle_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_query(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_payday_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_research_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_workshop_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_player_creature_info(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_room_and_creature_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_player_room_info(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_toggle_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_quit_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_save_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_shadows(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_view_distance_level(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_rotate_mode(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_cluedo_mode(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_gamma_correction(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_sound_volume(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_music_volume(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_cluedo_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_slider(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_smiley_anger_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_experience_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_instance_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_instance(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_stat_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_define_key_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_define_key(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_icon(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_slider(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_set_mouse_sensitivity(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_small_slider(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_invert_mouse(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_invert_mouse(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontstats_draw_main_stats(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontstats_draw_scrolling_stats(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontstats_leave(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_vlarge_menu_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_high_score_table(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_quit_high_score_table(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_maintain_high_score_ok_button(struct GuiButton *gbtn);

DLLIMPORT void _DK_pick_up_next_wanderer(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_wanderer(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_worker(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_worker(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_fighter(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_fighter(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_pic(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_creature_doing_activity(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_creature_activity(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_anger_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_row(struct GuiButton *gbtn);

DLLIMPORT void _DK_gui_activity_background(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_pretty_background(struct GuiMenu *gmnu);
DLLIMPORT void _DK_frontend_copy_background(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_round_glass_background(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_creature_query_background1(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_creature_query_background2(struct GuiMenu *gmnu);
DLLIMPORT void _DK_reset_scroll_window(struct GuiMenu *gmnu);
DLLIMPORT void _DK_init_load_menu(struct GuiMenu *gmnu);
DLLIMPORT void _DK_init_save_menu(struct GuiMenu *gmnu);
DLLIMPORT void _DK_init_video_menu(struct GuiMenu *gmnu);
DLLIMPORT void _DK_init_audio_menu(struct GuiMenu *gmnu);
DLLIMPORT void _DK_frontend_init_options_menu(struct GuiMenu *gmnu);
DLLIMPORT void _DK_frontend_draw_large_menu_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_scroll_box_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_scroll_box(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_slider_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_services_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_text(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_service_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_large_menu_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_service_select(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_change_state(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_over_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_session_set_player_name(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_text_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_enter_text(struct GuiButton *gbtn);
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
DLLIMPORT void _DK_frontend_draw_small_menu_button(struct GuiButton *gbtn);
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
DLLIMPORT void _DK_frontend_toggle_computer_players(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_computer_players(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_current_message(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_messages(struct GuiButton *gbtn);
DLLIMPORT void _DK_set_packet_start(struct GuiButton *gbtn);
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
DLLIMPORT void _DK_gui_load_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_load_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_scroll_window(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_event(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_zoom_to_event(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_close_objective(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_text_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_text_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_scroll_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_scroll_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_text_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_choose_hold_audience(struct GuiButton *gbtn);
DLLIMPORT void _DK_choose_armageddon(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_games_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_load_game_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_start_new_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_continue_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_continue_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_main_menu_load_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_main_menu_netservice_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_main_menu_highscores_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_previous_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_next_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_get_creature_in_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_person_in_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_setup_friend_over(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_friendly_battlers(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_setup_enemy_over(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_enemy_battlers(struct GuiButton *gbtn);
DLLIMPORT void _DK_select_resurrect_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_resurrect_creature_select(struct GuiButton *gbtn);
DLLIMPORT void _DK_draw_resurrect_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_select_resurrect_creature_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_select_resurrect_creature_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_resurrect_creature_scroll(struct GuiButton *gbtn);
DLLIMPORT void _DK_select_transfer_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_draw_transfer_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_transfer_creature_select(struct GuiButton *gbtn);
DLLIMPORT void _DK_select_transfer_creature_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_select_transfer_creature_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_transfer_creature_scroll(struct GuiButton *gbtn);
/******************************************************************************/
long gf_change_player_state(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_change_player_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_research_rooms(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_make_everything_free(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_give_all_creatures_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_explore_everywhere(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_research_magic(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_all_researchable(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gfa_can_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag);
long gfa_controlled_creature_has_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag);
long gf_decide_victory(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);

struct GuiBoxOption gui_main_cheat_list[] = { //gui_main_option_list in beta
  {"Null mode",                1,           NULL,      gf_change_player_state, 0, 0, 0,        PSt_None, 0, 0, 0, 0},
  {"Place tunneller mode",     1,           NULL,      gf_change_player_state, 0, 0, 0,PSt_MkGoodWorker, 0, 0, 0, 0},
  {"Place creature mode",      1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_MkBadCreatr, 0, 0, 0, 0},
  {"Place hero mode",          1,           NULL,      gf_change_player_state, 0, 0, 0,PSt_MkGoodCreatr, 0, 0, 0, 0},
  {"Destroy walls mode",       1,           NULL,      gf_change_player_state, 0, 0, 0,PSt_DestroyWalls, 0, 0, 0, 0},
  {"Disease mode",             1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_CastDisease, 0, 0, 0, 0},
  {"Peter mode",               1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_TurnChicken, 0, 0, 0, 0},
  {"Create gold mode",         1,           NULL,      gf_change_player_state, 0, 0, 0,   PSt_MkGoldPot, 0, 0, 0, 0},
  {"",                         2,           NULL,                        NULL, 0, 0, 0,        PSt_None, 0, 0, 0, 0},
  {"Passenger control mode",   1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_CtrlPassngr, 0, 0, 0, 0},
  {"Direct control mode",      1,           NULL,      gf_change_player_state, 0, 0, 0,  PSt_CtrlDirect, 0, 0, 0, 0},
  {"Order creature mode",      1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_OrderCreatr, 0, 0, 0, 0},
  {"",                         2,           NULL,                        NULL, 0, 0, 0,        PSt_None, 0, 0, 0, 0},
  {"!",                        0,           NULL,                        NULL, 0, 0, 0,        PSt_None, 0, 0, 0, 0},
};

struct GuiBoxOption gui_creature_cheat_option_list[] = {
 {"Everything is free",        1,           NULL,     gf_make_everything_free, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Give controlled creature spells",1,gfa_can_give_controlled_creature_spells,gf_give_controlled_creature_spells, 0, 0, 0, 0, 0, 0, 0, 0},
 {"Give all creatures spells", 1,           NULL,gf_give_all_creatures_spells, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Explore everywhere",        1,           NULL,       gf_explore_everywhere, 0, 0, 0,               0, 0, 0, 0, 0},
 {"All rooms and magic researchable",1,     NULL,         gf_all_researchable, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Research all magic",        1,           NULL,           gf_research_magic, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Research all rooms",        1,           NULL,           gf_research_rooms, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Win the level instantly",   1,           NULL,           gf_decide_victory, 0, 0, 0,               1, 0, 0, 0, 0},
 {"Lose the level instantly",  1,           NULL,           gf_decide_victory, 0, 0, 0,               0, 0, 0, 0, 0},
 {"!",                         0,           NULL,                        NULL, 0, 0, 0,               0, 0, 0, 0, 0},
};

struct GuiBoxOption gui_instance_option_list[] = {
 {"Fireball",1,gfa_controlled_creature_has_instance,gf_change_player_instance, 5, 0, 0,  5, 0, 0, 0, 0},
 {"Meteor",1, gfa_controlled_creature_has_instance, gf_change_player_instance, 6, 0, 0,  6, 0, 0, 0, 0},
 {"Freeze",1, gfa_controlled_creature_has_instance, gf_change_player_instance, 7, 0, 0,  7, 0, 0, 0, 0},
 {"Armour",1, gfa_controlled_creature_has_instance, gf_change_player_instance, 8, 0, 0,  8, 0, 0, 0, 0},
 {"Lightning",1,gfa_controlled_creature_has_instance,gf_change_player_instance,9, 0, 0,  9, 0, 0, 0, 0},
 {"Rebound",1,gfa_controlled_creature_has_instance, gf_change_player_instance,10, 0, 0, 10, 0, 0, 0, 0},
 {"Heal",1,   gfa_controlled_creature_has_instance, gf_change_player_instance,11, 0, 0, 11, 0, 0, 0, 0},
 {"Poison Cloud",1,gfa_controlled_creature_has_instance,gf_change_player_instance,12,0,0,12,0, 0, 0, 0},
 {"Invisibility",1,gfa_controlled_creature_has_instance,gf_change_player_instance,13,0,0,13,0, 0, 0, 0},
 {"Teleport",1,gfa_controlled_creature_has_instance,gf_change_player_instance,14, 0, 0, 14, 0, 0, 0, 0},
 {"Speed", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,15, 0, 0, 15, 0, 0, 0, 0},
 {"Slow",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,16, 0, 0, 16, 0, 0, 0, 0},
 {"Drain", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,17, 0, 0, 17, 0, 0, 0, 0},
 {"Fear",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,18, 0, 0, 18, 0, 0, 0, 0},
 {"Missile",1,gfa_controlled_creature_has_instance, gf_change_player_instance,19, 0, 0, 19, 0, 0, 0, 0},
 {"Homer", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,20, 0, 0, 20, 0, 0, 0, 0},
 {"Breath",1, gfa_controlled_creature_has_instance, gf_change_player_instance,21, 0, 0, 21, 0, 0, 0, 0},
 {"Wind",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,22, 0, 0, 22, 0, 0, 0, 0},
 {"Light", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,23, 0, 0, 23, 0, 0, 0, 0},
 {"Fly",   1, gfa_controlled_creature_has_instance, gf_change_player_instance,24, 0, 0, 24, 0, 0, 0, 0},
 {"Sight", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,25, 0, 0, 25, 0, 0, 0, 0},
 {"Grenade",1,gfa_controlled_creature_has_instance, gf_change_player_instance,26, 0, 0, 26, 0, 0, 0, 0},
 {"Hail",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,27, 0, 0, 27, 0, 0, 0, 0},
 {"WOP",   1, gfa_controlled_creature_has_instance, gf_change_player_instance,28, 0, 0, 28, 0, 0, 0, 0},
 {"Fart",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,29, 0, 0, 29, 0, 0, 0, 0},
 {"Dig",   1, gfa_controlled_creature_has_instance, gf_change_player_instance,39, 0, 0, 39, 0, 0, 0, 0},
 {"Arrow", 1, gfa_controlled_creature_has_instance, gf_change_player_instance, 4, 0, 0,  4, 0, 0, 0, 0},
 {"Group", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,40, 0, 0, 40, 0, 0, 0, 0},
 {"Disease",1,gfa_controlled_creature_has_instance, gf_change_player_instance,41, 0, 0, 41, 0, 0, 0, 0},
 {"Chicken",1,gfa_controlled_creature_has_instance, gf_change_player_instance,42, 0, 0, 42, 0, 0, 0, 0},
 {"!",     0,                          NULL,                             NULL, 0, 0, 0,  0, 0, 0, 0, 0},
};

#define BID_INFO_TAB      1
#define BID_ROOM_TAB      2
#define BID_SPELL_TAB     3
#define BID_TRAP_TAB      4
#define BID_CREATR_TAB    5

int select_level_scroll_offset = 0;
int number_of_freeplay_levels = 0;

struct GuiButtonInit main_menu_buttons[] = {
  { 0,             38, 0, 0, 0,          gui_zoom_in,           NULL,  NULL,               0, 110,   4, 114,   4, 26, 64, gui_area_new_normal_button,      237, 321,  0,       {0},            0, 0, NULL },
  { 0,             39, 0, 0, 0,         gui_zoom_out,           NULL,  NULL,               0, 110,  70, 114,  70, 26, 64, gui_area_new_normal_button,      239, 322,  0,       {0},            0, 0, NULL },
  { 0,             37, 0, 0, 0,        gui_go_to_map,           NULL,  NULL,               0,   0,   0,   0,   0, 30, 30, gui_area_new_normal_button,      304, 323,  0,       {0},            0, 0, NULL },
  { 0,              0, 0, 0, 0,gui_turn_on_autopilot,           NULL,  NULL,               0,   0,  70,   0,  70, 16, 68, gui_area_autopilot_button,       492, 201,  0,       {0},            0, 0, maintain_turn_on_autopilot },
  { 0,              0, 0, 0, 0,                 NULL,           NULL,  NULL,               0,  68,   0,  68,   0, 68, 16, gui_area_new_normal_button,      499, 722,&options_menu, {0},        0, 0, NULL },
  { 3,   BID_INFO_TAB, 0, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               7,   0, 154,   0, 154, 28, 34, gui_draw_tab,                      7, 447,  0,{(long)&info_tag},     0, 0, menu_tab_maintain },
  { 3,   BID_ROOM_TAB, 0, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               2,  28, 154,  28, 154, 28, 34, gui_draw_tab,                      9, 448,  0,{(long)&room_tag},     0, 0, menu_tab_maintain },
  { 3,  BID_SPELL_TAB, 0, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               3,  56, 154,  56, 154, 28, 34, gui_draw_tab,                     11, 449,  0,{(long)&spell_tag},    0, 0, menu_tab_maintain },
  { 3,   BID_TRAP_TAB, 0, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               4,  84, 154,  84, 154, 28, 34, gui_draw_tab,                     13, 450,  0,{(long)&trap_tag},     0, 0, menu_tab_maintain },
  { 3, BID_CREATR_TAB, 0, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               5, 112, 154, 112, 154, 28, 34, gui_draw_tab,                     15, 451,  0,{(long)&creature_tag}, 0, 0,menu_tab_maintain },
  { 0,             40, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 360, 138, 360, 24, 30, gui_area_event_button,             0, 201,  0,       {0},            0, 0, maintain_event_button },
  { 0,             41, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 330, 138, 330, 24, 30, gui_area_event_button,             0, 201,  0,       {1},            0, 0, maintain_event_button },
  { 0,             42, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 300, 138, 300, 24, 30, gui_area_event_button,             0, 201,  0,       {2},            0, 0, maintain_event_button },
  { 0,             43, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 270, 138, 270, 24, 30, gui_area_event_button,             0, 201,  0,       {3},            0, 0, maintain_event_button },
  { 0,             44, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 240, 138, 240, 24, 30, gui_area_event_button,             0, 201,  0,       {4},            0, 0, maintain_event_button },
  { 0,             45, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 210, 138, 210, 24, 30, gui_area_event_button,             0, 201,  0,       {5},            0, 0, maintain_event_button },
  { 0,             46, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 180, 138, 180, 24, 30, gui_area_event_button,             0, 201,  0,       {6},            0, 0, maintain_event_button },
  { 0,             47, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 150, 138, 150, 24, 30, gui_area_event_button,             0, 201,  0,       {7},            0, 0, maintain_event_button },
  { 0,             48, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 120, 138, 120, 24, 30, gui_area_event_button,             0, 201,  0,       {8},            0, 0, maintain_event_button },
  { 0,             49, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  90, 138,  90, 24, 30, gui_area_event_button,             0, 201,  0,       {9},            0, 0, maintain_event_button },
  { 0,             50, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  60, 138,  60, 24, 30, gui_area_event_button,             0, 201,  0,      {10},            0, 0, maintain_event_button },
  { 0,             51, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  30, 138,  30, 24, 30, gui_area_event_button,             0, 201,  0,      {11},            0, 0, maintain_event_button },
  { 0,             52, 0, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,   0, 138,   0, 24, 30, gui_area_event_button,             0, 201,  0,      {12},            0, 0, maintain_event_button },
  { 0,              0, 0, 0, 0,                 NULL,           NULL,  NULL,               0,  22, 122,  22, 122, 94, 40, NULL,                              0, 441,  0,       {0},            0, 0, NULL },
  {-1,              0, 0, 0, 0,                 NULL,           NULL,  NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit room_menu_buttons[] = {
  { 0,  6, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,238,  6, 242, 32, 36, gui_area_room_button,             57, 615,  0,       {2},            0, 0, maintain_room },
  { 0,  8, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,238, 38, 242, 32, 36, gui_area_room_button,             79, 625,  0,      {14},            0, 0, maintain_room },
  { 0,  7, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,238, 70, 242, 32, 36, gui_area_room_button,             59, 624,  0,      {13},            0, 0, maintain_room },
  { 0, 10, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,238,102, 242, 32, 36, gui_area_room_button,             67, 618,  0,       {6},            0, 0, maintain_room },
  { 0,  9, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,276,  6, 280, 32, 36, gui_area_room_button,             61, 616,  0,       {3},            0, 0, maintain_room },
  { 0, 18, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,276, 38, 280, 32, 36, gui_area_room_button,             81, 626,  0,      {15},            0, 0, maintain_room },
  { 0, 19, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,276, 70, 280, 32, 36, gui_area_room_button,             83, 627,  0,      {16},            0, 0, maintain_room },
  { 0, 13, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,276,102, 280, 32, 36, gui_area_room_button,             75, 621,  0,       {8},            0, 0, maintain_room },
  { 0, 11, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,314,  6, 318, 32, 36, gui_area_room_button,             65, 617,  0,       {4},            0, 0, maintain_room },
  { 0, 17, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,314, 38, 318, 32, 36, gui_area_room_button,             63, 619,  0,       {5},            0, 0, maintain_room },
  { 0, 16, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,314, 70, 318, 32, 36, gui_area_room_button,             69, 623,  0,      {12},            0, 0, maintain_room },
  { 0, 12, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,314,102, 318, 32, 36, gui_area_room_button,             73, 628,  0,      {10},            0, 0, maintain_room },
  { 0, 15, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,352,  6, 356, 32, 36, gui_area_room_button,             71, 622,  0,      {11},            0, 0, maintain_room },
  { 0, 14, 0, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,352, 38, 356, 32, 36, gui_area_room_button,             77, 629,  0,       {9},            0, 0, maintain_room },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, maintain_room },
  { 0, 20, 0, 0, 0, gui_remove_area_for_rooms,NULL,NULL,                 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button,     107, 462,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194, 46, 44, gui_area_big_room_button,          0, 201,  0,       {0},            0, 0, maintain_big_room },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit spell_menu_buttons[] = {
  { 0, 36, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 238,   6, 242, 32, 36, gui_area_spell_button,           114, 647,  0,      {18},            0, 0, maintain_spell },
  { 0, 21, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 238,  38, 242, 32, 36, gui_area_spell_button,           118, 648,  0,       {2},            0, 0, maintain_spell },
  { 0, 22, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 238,  70, 242, 32, 36, gui_area_spell_button,           108, 649,  0,       {5},            0, 0, maintain_spell },
  { 0, 27, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 238, 102, 242, 32, 36, gui_area_spell_button,           122, 654,  0,      {11},            0, 0, maintain_spell },
  { 0, 35, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 276,   6, 280, 32, 36, gui_area_spell_button,           452, 653,  0,       {3},            0, 0, maintain_spell },
  { 0, 23, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 276,  38, 280, 32, 36, gui_area_spell_button,           116, 650,  0,       {6},            0, 0, maintain_spell },
  { 0, 29, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 276,  70, 280, 32, 36, gui_area_spell_button,           128, 656,  0,      {13},            0, 0, maintain_spell },
  { 0, 34, 0, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  98, 276, 102, 280, 32, 36, gui_area_spell_button,           112, 651,  0,       {9},            0, 0, maintain_spell },
  { 0, 24, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 314,   6, 318, 32, 36, gui_area_spell_button,           120, 652,  0,       {7},            0, 0, maintain_spell },
  { 0, 26, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 314,  38, 318, 32, 36, gui_area_spell_button,           110, 661,  0,       {8},            0, 0, maintain_spell },
  { 0, 25, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 314,  70, 318, 32, 36, gui_area_spell_button,           124, 657,  0,      {10},            0, 0, maintain_spell },
  { 0, 28, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 314, 102, 318, 32, 36, gui_area_spell_button,           126, 655,  0,      {12},            0, 0, maintain_spell },
  { 0, 30, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 352,   6, 356, 32, 36, gui_area_spell_button,           314, 658,  0,      {15},            0, 0, maintain_spell },
  { 0, 31, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 352,  38, 356, 32, 36, gui_area_spell_button,           319, 659,  0,      {14},            0, 0, maintain_spell },
  { 0, 33, 0, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_spell_button,           321, 663,  0,      {19},            0, 0, maintain_spell },
  { 0, 32, 0, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 352, 102, 356, 32, 36, gui_area_spell_button,           317, 660,  0,      {16},            0, 0, maintain_spell },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194, 46, 44, gui_area_big_spell_button,         0, 201,  0,       {0},            0, 0, maintain_big_spell },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit spell_lost_menu_buttons[] = {
  { 0, 36, 0, 0, 0, spell_lost_first_person,NULL,    NULL,               0,   2, 238,   8, 250, 24, 24, gui_area_new_null_button,        114, 647,  0,      {18},            0, 0, maintain_spell },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 238,  40, 250, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 238,  72, 250, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 238, 104, 250, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 276,   8, 288, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 276,  40, 288, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  72, 288, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 104, 288, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 314,   8, 326, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 314,  40, 326, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 314,  72, 326, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 314, 104, 326, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   8, 364, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  40, 364, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  72, 364, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 352, 104, 364, 24, 24, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194, 46, 44, gui_area_big_spell_button,         0, 201,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit trap_menu_buttons[] = {
  { 0, 54, 0, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0, 2,238,  6, 242, 32, 36, gui_area_trap_button,            154, 585,  0,       {2},            0, 0, maintain_trap },
  { 0, 55, 0, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,34,238, 38, 242, 32, 36, gui_area_trap_button,            156, 586,  0,       {3},            0, 0, maintain_trap },
  { 0, 56, 0, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,66,238, 70, 242, 32, 36, gui_area_trap_button,            158, 587,  0,       {4},            0, 0, maintain_trap },
  { 0, 67, 0, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,98,238,102, 242, 32, 36, gui_area_trap_button,            162, 589,  0,       {6},            0, 0, maintain_trap },
  { 0, 53, 0, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0, 2,276,  6, 280, 32, 36, gui_area_trap_button,            152, 584,  0,       {1},            0, 0, maintain_trap },
  { 0, 57, 0, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,34,276, 38, 280, 32, 36, gui_area_trap_button,            160, 588,  0,       {5},            0, 0, maintain_trap },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  70, 280, 32, 36, gui_area_trap_button,             24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 102, 280, 32, 36, gui_area_trap_button,             24, 201,  0,       {0},            0, 0, NULL },
  { 0, 58, 0, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0, 2,314,  6, 318, 32, 36, gui_area_trap_button,            166, 594,  0,       {7},            0, 0, maintain_door },
  { 0, 59, 0, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0,34,314, 38, 318, 32, 36, gui_area_trap_button,            168, 595,  0,       {8},            0, 0, maintain_door },
  { 0, 60, 0, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0,66,314, 70, 318, 32, 36, gui_area_trap_button,            170, 596,  0,       {9},            0, 0, maintain_door },
  { 0, 61, 0, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0,98,314,102, 318, 32, 36, gui_area_trap_button,            172, 597,  0,      {10},            0, 0, maintain_door },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   6, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  38, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0, 62, 0, 0, 0, gui_remove_area_for_traps,NULL,  NULL,               0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button,     107, 463,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194, 46, 44, gui_area_big_trap_button,          0, 201,  0,       {0},            0, 0, maintain_big_trap },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_menu_buttons[] = {
  { 0, 72, 0, 0, 0, pick_up_next_wanderer,gui_go_to_next_wanderer,NULL,  0,  26, 192,  26, 192, 38, 24, gui_area_new_normal_button,      284, 302,  0,       {0},            0, 0, NULL },
  { 0, 73, 0, 0, 0, pick_up_next_worker,gui_go_to_next_worker,NULL,      0,  62, 192,  62, 192, 38, 24, gui_area_new_normal_button,      282, 303,  0,       {0},            0, 0, NULL },
  { 0, 74, 0, 0, 0, pick_up_next_fighter,gui_go_to_next_fighter,NULL,    0,  98, 192,  98, 192, 38, 24, gui_area_new_normal_button,      286, 304,  0,       {0},            0, 0, NULL },
  { 1,  0, 0, 0, 0, gui_scroll_activity_up,gui_scroll_activity_up,NULL,  0,   4, 192,   4, 192, 22, 24, gui_area_new_normal_button,      278, 201,  0,       {0},            0, 0, maintain_activity_up },
  { 1,  0, 0, 0, 0, gui_scroll_activity_down,gui_scroll_activity_down,NULL,0, 4, 364,   4, 364, 22, 24, gui_area_new_normal_button,      280, 201,  0,       {0},            0, 0, maintain_activity_down },
  { 0,  0, 0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  0,   0, 196,   0, 218, 22, 22, gui_area_new_no_anim_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,26,220,26,220,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[0]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,62,220,62,220,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[1]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,98,220,98,220,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[2]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  1,   0, 220,   0, 242, 22, 22, gui_area_new_no_anim_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,26,244,26,244,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[4]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,62,244,62,244,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[5]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,98,244,98,244,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[6]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  2,   0, 244,   0, 266, 22, 22, gui_area_new_no_anim_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,26,268,26,268,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[8]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,62,268,62,268,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[9]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,98,268,98,268,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[10]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  3,   0, 268,   0, 290, 22, 22, gui_area_new_no_anim_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,26,292,26,292,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[12]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,62,292,62,292,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[13]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,98,292,98,292,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[14]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  4,   0, 292,   0, 314, 22, 22, gui_area_new_no_anim_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,26,316,26,316,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[16]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,62,316,62,316,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[17]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,98,316,98,316,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[18]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  5,   0, 314,   0, 338, 22, 22, gui_area_new_no_anim_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,26,340,26,340,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[20]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,62,340,62,340,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[21]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,98,340,98,340,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[22]},0,0, maintain_activity_row },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit event_menu_buttons[] = {
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit options_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 716,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  12,  36,  12,  36, 46, 64, gui_area_no_anim_button,          23, 725, &load_menu, {0},          0, 0, maintain_loadsave },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  60,  36,  60,  36, 46, 64, gui_area_no_anim_button,          22, 726, &save_menu, {0},          0, 0, maintain_loadsave },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 108,  36, 108,  36, 46, 64, gui_area_no_anim_button,          25, 723, &video_menu,{0},          0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 156,  36, 156,  36, 46, 64, gui_area_no_anim_button,          24, 724, &sound_menu,{0},          0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 204,  36, 204,  36, 46, 64, gui_area_new_no_anim_button,     501, 728, &autopilot_menu,{0},      0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 252,  36, 252,  36, 46, 64, gui_area_no_anim_button,          26, 727, &quit_menu,{0},           0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit query_menu_buttons[] = {
  { 0,  0, 0, 0, 0, gui_set_query,      NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      475, 432,  0,       {0},            0, 0, NULL },
  { 2, 69, 0, 0, 0, gui_set_tend_to,    NULL,        NULL,               1,  36, 190,  36, 190, 32, 26, gui_area_flash_cycle_button,     350, 307,  0,{(long)&game.creatures_tend_1}, 1, 0, maintain_prison_bar },
  { 2, 70, 0, 0, 0, gui_set_tend_to,    NULL,        NULL,               2,  74, 190,  74, 190, 32, 26, gui_area_flash_cycle_button,     346, 306,  0,{(long)&game.creatures_tend_2}, 1, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 216,   4, 222,130, 24, gui_area_payday_button,          341, 454,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 246,   2, 246, 60, 24, gui_area_research_bar,            61, 452,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 246,  74, 246, 60, 24, gui_area_workshop_bar,            75, 453,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 274,  74, 274, 60, 24, gui_area_player_creature_info,   323, 456,  0,       {0},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 298,  74, 298, 60, 24, gui_area_player_creature_info,   325, 456,  0,       {1},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 322,  74, 322, 60, 24, gui_area_player_creature_info,   327, 456,  0,       {2},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 346,  74, 346, 60, 24, gui_area_player_creature_info,   329, 456,  0,       {3},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 274,   4, 274, 60, 24, gui_area_player_room_info,       324, 455,  0,       {0},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 298,   4, 298, 60, 24, gui_area_player_room_info,       326, 455,  0,       {1},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 322,   4, 322, 60, 24, gui_area_player_room_info,       328, 455,  0,       {2},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_player_room_info,       330, 455,  0,       {3},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 274,  62, 274, 14, 22, gui_area_ally,                     0, 469,  0,       {0},            0, 0, maintain_ally },
  { 0,  0, 0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 298,  62, 298, 14, 22, gui_area_ally,                     0, 469,  0,       {1},            0, 0, maintain_ally },
  { 0,  0, 0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 322,  62, 322, 14, 22, gui_area_ally,                     0, 469,  0,       {2},            0, 0, maintain_ally },
  { 0,  0, 0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 346,  62, 346, 14, 22, gui_area_ally,                     0, 469,  0,       {3},            0, 0, maintain_ally },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit quit_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,210, 32, gui_area_text,                     1, 309,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  70,  24,  72,  58, 46, 32, gui_area_normal_button,           46, 311,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, gui_quit_game,      NULL,        NULL,               0, 136,  24, 138,  58, 46, 32, gui_area_normal_button,           48, 310,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit load_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 719,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               0, 999,  58, 999,  58,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[0]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               1, 999,  90, 999,  90,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[1]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               2, 999, 122, 999, 122,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[2]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               3, 999, 154, 999, 154,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[3]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               4, 999, 186, 999, 186,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[4]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               5, 999, 218, 999, 218,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[5]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               6, 999, 250, 999, 250,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[6]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               7, 999, 282, 999, 282,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[7]}, 0, 0, gui_load_game_maintain },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit save_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 720,  0,       {0},            0, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               0, 999,  58, 999,  58,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[0]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               1, 999,  90, 999,  90,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[1]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               2, 999, 122, 999, 122,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[2]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               3, 999, 154, 999, 154,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[3]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               4, 999, 186, 999, 186,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[4]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               5, 999, 218, 999, 218,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[5]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               6, 999, 250, 999, 250,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[6]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               7, 999, 282, 999, 282,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[7]},15, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit video_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 717,  0,       {0},            0, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_shadows,  NULL,        NULL,               0,   8,  38,  10,  38, 46, 64, gui_area_no_anim_button,          27, 313,  0,{(long)&video_shadows}, 4, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_view_distance_level,NULL,NULL,             0,  56,  38,  58,  38, 46, 64, gui_area_no_anim_button,          36, 316,  0,{(long)&video_view_distance_level}, 3, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_rotate_mode,NULL,      NULL,               0, 104,  38, 106,  38, 46, 64, gui_area_no_anim_button,          32, 314,  0,{(long)&settings.field_3}, 1, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_cluedo_mode,NULL,      NULL,               0,  32,  90,  32,  90, 46, 64, gui_area_no_anim_button,          42, 315,  0,{(long)&_DK_video_cluedo_mode},1, 0, gui_video_cluedo_maintain },
  { 0,  0, 0, 0, 0, gui_video_gamma_correction,NULL, NULL,               0,  80,  90,  80,  90, 46, 64, gui_area_no_anim_button,          44, 317,  0,{(long)&video_gamma_correction}, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit sound_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 718,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8,  28,  10,  28, 46, 64, gui_area_no_anim_button,          41, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8,  80,  10,  80, 46, 64, gui_area_no_anim_button,          40, 201,  0,       {0},            0, 0, NULL },
  { 4,  0, 0, 0, 0, gui_set_sound_volume,NULL,       NULL,               0,  66,  58,  66,  58,190, 32, gui_area_slider,                   0, 340,  0,{(long)&sound_level}, 127, 0, NULL },
  { 4,  0, 0, 0, 0, gui_set_music_volume,NULL,       NULL,               0,  66, 110,  66, 110,190, 32, gui_area_slider,                   0, 341,  0,{(long)&music_level}, 127, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit error_box_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 670,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,   0, 999,   0,155,155, gui_area_text,                     0, 201,  0,{(long)&gui_error_text},0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 999, 100, 999, 132, 46, 34, gui_area_normal_button,           48, 201,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit instance_menu_buttons[] = {
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit text_info_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,   4, 999,   4,400, 78, gui_area_scroll_window,            0, 201,  0,{(long)&game.evntbox_scroll_window},0,0, NULL },
  { 1, 63, 0, 0, 0, gui_go_to_event,    NULL,        NULL,               0,   4,   4,   4,   4, 30, 24, gui_area_new_normal_button,      276, 466,  0,       {0},             0,0, maintain_zoom_to_event },
  { 0, 64, 0, 0, 1, gui_close_objective,gui_close_objective,NULL,        0,   4,  56,   4,  56, 30, 24, gui_area_new_normal_button,      274, 465,  0,       {0},             0,0, NULL },
  { 1, 66, 0, 0, 0, gui_scroll_text_up, NULL,        NULL,               0, 446,   4, 446,   4, 30, 24, gui_area_new_normal_button,      486, 201,  0,{(long)&game.evntbox_scroll_window},0,0, maintain_scroll_up },
  { 1, 65, 0, 0, 0, gui_scroll_text_down,NULL,       NULL,               0, 446,  56, 446,  56, 30, 24, gui_area_new_normal_button,      272, 201,  0,{(long)&game.evntbox_scroll_window},0,0, maintain_scroll_down },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit pause_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999, 999, 999, 999,140,100, gui_area_text,                     0, 320,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit battle_buttons[] = {
  { 0,  0, 0, 0, 1, gui_close_objective,NULL,        NULL,               0,   4,  72,   4,  72, 30, 24, gui_area_new_normal_button,      274, 465,  0,       {0},            0, 0, NULL },
  { 1,  0, 0, 0, 0, gui_previous_battle,NULL,        NULL,               0, 446,   4, 446,   4, 30, 24, gui_area_new_normal_button,      486, 464,  0,       {0},            0, 0, NULL },
  { 1,  0, 0, 0, 0, gui_next_battle,NULL,            NULL,               0, 446,  72, 446,  72, 30, 24, gui_area_new_normal_button,      272, 464,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_friend_over,0, 42,12, 42,12,160,24,gui_area_friendly_battlers,0,201,0,   {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_enemy_over, 0,260,12,260,12,160,24,gui_area_enemy_battlers,   0,201,0,   {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_friend_over,1, 42,42, 42,42,160,24,gui_area_friendly_battlers,0,201,0,   {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_enemy_over, 1,260,42,260,42,160,24,gui_area_enemy_battlers,   0,201,0,   {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_friend_over,2, 42,72, 42,72,160,24,gui_area_friendly_battlers,0,201,0,   {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_enemy_over, 2,260,72,260,72,160,24,gui_area_enemy_battlers,   0,201,0,   {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 214,  34, 214,  34, 32, 32, gui_area_null,                   175, 201,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit resurrect_creature_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, gui_area_text,                     1, 428,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, select_resurrect_creature,NULL,  NULL,               0, 999,  62, 999,  62,250, 26, draw_resurrect_creature,           0, 201,  0,       {0},            0, 0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, select_resurrect_creature,NULL,  NULL,               1, 999,  90, 999,  90,250, 26, draw_resurrect_creature,           0, 201,  0,       {0},            0, 0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, select_resurrect_creature,NULL,  NULL,               2, 999, 118, 999, 118,250, 26, draw_resurrect_creature,           0, 201,  0,       {0},            0, 0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, select_resurrect_creature,NULL,  NULL,               3, 999, 146, 999, 146,250, 26, draw_resurrect_creature,           0, 201,  0,       {0},            0, 0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, select_resurrect_creature,NULL,  NULL,               4, 999, 174, 999, 174,250, 26, draw_resurrect_creature,           0, 201,  0,       {0},            0, 0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, select_resurrect_creature,NULL,  NULL,               5, 999, 202, 999, 202,250, 26, draw_resurrect_creature,           0, 201,  0,       {0},            0, 0, maintain_resurrect_creature_select },
  { 1,  0, 0, 0, 0, select_resurrect_creature_up,NULL,NULL,              1, 305,  62, 305,  62, 22, 24, gui_area_new_normal_button,      278, 201,  0,       {0},            0, 0, maintain_resurrect_creature_scroll },
  { 1,  0, 0, 0, 0, select_resurrect_creature_down,NULL,NULL,            2, 305, 204, 305, 204, 22, 24, gui_area_new_normal_button,      280, 201,  0,       {0},            0, 0, maintain_resurrect_creature_scroll },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 999, 258, 999, 258,100, 32, gui_area_text,                     1, 403,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit transfer_creature_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, gui_area_text,                     1, 429,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, select_transfer_creature,NULL,   NULL,               0, 999,  62, 999,  62,250, 26, draw_transfer_creature,            0, 201,  0,       {0},            0, 0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, select_transfer_creature,NULL,   NULL,               1, 999,  90, 999,  90,250, 26, draw_transfer_creature,            1, 201,  0,       {0},            0, 0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, select_transfer_creature,NULL,   NULL,               2, 999, 118, 999, 118,250, 26, draw_transfer_creature,            2, 201,  0,       {0},            0, 0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, select_transfer_creature,NULL,   NULL,               3, 999, 146, 999, 146,250, 26, draw_transfer_creature,            3, 201,  0,       {0},            0, 0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, select_transfer_creature,NULL,   NULL,               4, 999, 174, 999, 174,250, 26, draw_transfer_creature,            4, 201,  0,       {0},            0, 0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, select_transfer_creature,NULL,   NULL,               5, 999, 202, 999, 202,250, 26, draw_transfer_creature,            5, 201,  0,       {0},            0, 0, maintain_transfer_creature_select },
  { 1,  0, 0, 0, 0, select_transfer_creature_up,NULL,NULL,               1, 305,  62, 305,  62, 22, 24, gui_area_new_normal_button,      278, 201,  0,       {0},            0, 0, maintain_transfer_creature_scroll },
  { 1,  0, 0, 0, 0, select_transfer_creature_down,NULL,NULL,             2, 305, 204, 305, 204, 22, 24, gui_area_new_normal_button,      280, 201,  0,       {0},            0, 0, maintain_transfer_creature_scroll },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 999, 258, 999, 258,100, 32, gui_area_text,                     1, 403,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit hold_audience_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 634,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, gui_area_normal_button,           46, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, choose_hold_audience,NULL,       NULL,               0, 116,  24, 118,  58, 46, 32, gui_area_normal_button,           48, 201,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit armageddon_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 646,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, gui_area_normal_button,           46, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, choose_armageddon,  NULL,        NULL,               0, 116,  24, 118,  58, 46, 32, gui_area_normal_button,           48, 201,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit dungeon_special_buttons[] = {
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_main_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  26, 999,  26,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {1},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_start_new_game,NULL,frontend_over_button,   3, 999,  92, 999,  92,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {2},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_load_continue_game,NULL,frontend_over_button,0,999, 138, 999, 138,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {8},            0, 0, frontend_continue_game_maintain },
  { 0,  0, 0, 0, 0, frontend_ldcampaign_change_state,NULL,frontend_over_button,30,999,184, 999, 184,371, 46, frontend_draw_large_menu_button,0,201, 0,     {106},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,    2, 999, 230, 999, 230,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {3},            0, 0, frontend_main_menu_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_netservice_change_state,NULL, frontend_over_button,  4, 999, 276, 999, 276,371, 46, frontend_draw_large_menu_button,0,201,0,    {4},            0, 0, frontend_main_menu_netservice_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,   27, 999, 322, 999, 322,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {97},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_ldcampaign_change_state,NULL, frontend_over_button,   18, 999, 368, 999, 368,371, 46, frontend_draw_large_menu_button,0,201,0,{104},            0, 0, frontend_main_menu_highscores_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,    9, 999, 414, 999, 414,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {5},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};
/*
struct GuiButtonInit frontend_main_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {1},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_start_new_game,NULL,frontend_over_button,   3, 999, 104, 999, 104,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {2},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_load_continue_game,NULL,frontend_over_button,0,999, 154, 999, 154,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {8},            0, 0, frontend_continue_game_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,    2, 999, 204, 999, 204,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {3},            0, 0, frontend_main_menu_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,    4, 999, 254, 999, 254,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {4},            0, 0, frontend_main_menu_netservice_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,   27, 999, 304, 999, 304,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {97},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,   18, 999, 354, 999, 354,371, 46, frontend_draw_large_menu_button,   0, 201,  0,     {104},            0, 0, frontend_main_menu_highscores_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL, frontend_over_button,    9, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {5},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};
*/
#define frontend_load_menu_items_visible  6
struct GuiButtonInit frontend_load_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {7},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,180, frontnet_draw_scroll_box,          0, 201,  0,      {26},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontend_load_game_up,NULL,frontend_over_button,     0, 532, 149, 532, 149, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontend_load_game_up_maintain },
  { 1,  0, 0, 0, 0, frontend_load_game_down,NULL,frontend_over_button,   0, 532, 317, 532, 317, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontend_load_game_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 163, 536, 163, 10,154, frontend_draw_games_scroll_tab,    0, 201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 125, 102, 125,220, 26, frontend_draw_text,                0, 201,  0,      {30},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 157,  95, 157,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {45},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 185,  95, 185,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {46},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 213,  95, 213,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {47},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 241,  95, 241,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {48},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 269,  95, 269,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {49},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 297,  95, 297,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {50},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

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

struct GuiButtonInit frontend_statistics_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {84},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  90, 999,  90,450,158, frontstats_draw_main_stats,        0, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999, 260, 999, 260,450,136, frontstats_draw_scrolling_stats,   0, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontstats_leave,NULL,frontend_over_button,         18, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {83},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_high_score_score_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,495, 46, frontend_draw_vlarge_menu_button,  0, 201,  0,      {85},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  97, 999,  97,450,286, frontend_draw_high_score_table,    0, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_quit_high_score_table,NULL,frontend_over_button,3,999,404,999,404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {83},            0, 0, frontend_maintain_high_score_ok_button },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons1[] = {
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, 433,&creature_query_menu2,{0}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button,    466, 291,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button,      467, 223,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, 222,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {0},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {1},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {2},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {3},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {4},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {5},            0, 0, maintain_instance },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons2[] = {
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, 433,&creature_query_menu3,{0}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button,    466, 291,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button,      467, 223,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, 222,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {4},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {5},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {6},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {7},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {8},            0, 0, maintain_instance },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {9},            0, 0, maintain_instance },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons3[] = {
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, 433,&creature_query_menu1,{0}, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 226,   4, 226, 60, 24, gui_area_stat_button,            331, 292,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 226,  72, 226, 60, 24, gui_area_stat_button,            332, 293,  0,       {1},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 256,   4, 256, 60, 24, gui_area_stat_button,            333, 295,  0,       {2},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 256,  72, 256, 60, 24, gui_area_stat_button,            334, 294,  0,       {3},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 286,   4, 286, 60, 24, gui_area_stat_button,            335, 296,  0,       {4},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 286,  72, 286, 60, 24, gui_area_stat_button,            336, 297,  0,       {5},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 316,   4, 316, 60, 24, gui_area_stat_button,            337, 298,  0,       {6},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 316,  72, 316, 60, 24, gui_area_stat_button,            338, 299,  0,       {7},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_stat_button,            339, 300,  0,       {8},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_stat_button,            340, 301,  0,       {9},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_define_keys_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {92},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 117,  82, 117,450,246, frontnet_draw_scroll_box,          0, 201,  0,      {94},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontend_define_key_up,NULL, frontend_over_button,   0, 532, 116, 532, 116, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontend_define_key_up_maintain },
  { 1,  0, 0, 0, 0, frontend_define_key_down,NULL,frontend_over_button,  0, 532, 350, 532, 350, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontend_define_key_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 130, 536, 130, 10,220, frontend_draw_define_key_scroll_tab,0,201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 130,  95, 130,424, 22, frontend_draw_define_key,          0, 201,  0,      {-1},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 152,  95, 152,424, 22, frontend_draw_define_key,          0, 201,  0,      {-2},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 174,  95, 174,424, 22, frontend_draw_define_key,          0, 201,  0,      {-3},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 196,  95, 196,424, 22, frontend_draw_define_key,          0, 201,  0,      {-4},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 218,  95, 218,424, 22, frontend_draw_define_key,          0, 201,  0,      {-5},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 240,  95, 240,424, 22, frontend_draw_define_key,          0, 201,  0,      {-6},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 262,  95, 262,424, 22, frontend_draw_define_key,          0, 201,  0,      {-7},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 284,  95, 284,424, 22, frontend_draw_define_key,          0, 201,  0,      {-8},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 306,  95, 306,424, 22, frontend_draw_define_key,          0, 201,  0,      {-9},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 328,  95, 328,424, 22, frontend_draw_define_key,          0, 201,  0,     {-10},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,     frontend_over_button,27,999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {98},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit autopilot_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 845,  0,       {0},            0, 0, NULL },
  { 3,  0, 0, 0, 0, gui_set_autopilot,  NULL,        NULL,               0,  12,  36,  12,  36, 46, 64, gui_area_new_normal_button,      503, 729,  0,{(long)&game.comp_player_aggressive}, 0, 0, NULL },
  { 3,  0, 0, 0, 0, gui_set_autopilot,  NULL,        NULL,               0,  60,  36,  60,  36, 46, 64, gui_area_new_normal_button,      505, 730,  0,{(long)&game.comp_player_defensive}, 0, 0, NULL },
  { 3,  0, 0, 0, 0, gui_set_autopilot,  NULL,        NULL,               0, 108,  36, 108,  36, 46, 64, gui_area_new_normal_button,      507, 731,  0,{(long)&game.comp_player_construct}, 0, 0, NULL },
  { 3,  0, 0, 0, 0, gui_set_autopilot,  NULL,        NULL,               0, 156,  36, 156,  36, 46, 64, gui_area_new_normal_button,      509, 732,  0,{(long)&game.comp_player_creatrsonly}, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_option_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {96},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 107,  95, 107,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 133,  95, 133,  2, 88, frontnet_draw_scroll_box,          0, 201,  0,      {89},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 108, 115, 108,220, 26, frontend_draw_text,                0, 201,  0,      {99},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 142, 146, 142, 48, 32, frontend_draw_icon,               90, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 182, 146, 182, 48, 32, frontend_draw_icon,               89, 201,  0,       {0},            0, 0, NULL },
  { 4, 75, 0, 0, 0, gui_set_sound_volume,NULL,       NULL,               0, 194, 147, 194, 147,300, 22, frontend_draw_slider,              0, 201,  0,{(long)&sound_level}, 127, 0, NULL },
  { 4,  0, 0, 0, 0, gui_set_music_volume,NULL,       NULL,               0, 194, 187, 194, 187,300, 22, frontend_draw_slider,              0, 201,  0,{(long)&music_level}, 127, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 231,  95, 231,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 257,  95, 257,  0, 88, frontnet_draw_scroll_box,          0, 201,  0,      {89},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 232, 115, 232,220, 26, frontend_draw_text,                0, 201,  0,     {100},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 271, 102, 271,190, 22, frontend_draw_text,                0, 201,  0,     {101},            0, 0, NULL },
  { 4,  0, 0, 0, 0, frontend_set_mouse_sensitivity,NULL,NULL,            0, 304, 271, 304, 271,190, 22, frontend_draw_small_slider,        0, 201,  0,{(long)&fe_mouse_sensitivity}, 7, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_invert_mouse,NULL,     frontend_over_button,0, 102, 303, 102, 303,380, 22, frontend_draw_text,                0, 201,  0,     {102},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 320, 303,   0,   0,100, 22, frontend_draw_invert_mouse,        0, 201,  0,     {102},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,    frontend_over_button,26, 999, 357, 999, 357,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {95},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,    frontend_over_button, 1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

#define frontend_select_level_items_visible  7
struct GuiButtonInit frontend_select_level_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,     {107},            0, 0, NULL},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 128,  82, 128,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 154,  82, 154,450,180, frontnet_draw_scroll_box,          0, 201,  0,      {26},            0, 0, NULL},
  { 1,  0, 0, 0, 0, frontend_level_select_up,NULL,frontend_over_button,  0, 532, 153, 532, 153, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontend_level_select_up_maintain},
  { 1,  0, 0, 0, 0, frontend_level_select_down,NULL,frontend_over_button,0, 532, 321, 532, 321, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontend_level_select_down_maintain},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 167, 536, 167, 10,154, frontend_draw_levels_scroll_tab,   0, 201,  0,      {40},            0, 0, NULL},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 129, 102, 129,220, 26, frontend_draw_text,                0, 201,  0,      {32},            0, 0, NULL},
  { 0,  0, 0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 167,  95, 169,424, 14, frontend_draw_level_select_button, 0, 201,  0,      {45},            0, 0, frontend_level_select_maintain},
  { 0,  0, 0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 189,  95, 191,424, 14, frontend_draw_level_select_button, 0, 201,  0,      {46},            0, 0, frontend_level_select_maintain},
  { 0,  0, 0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 211,  95, 213,424, 14, frontend_draw_level_select_button, 0, 201,  0,      {47},            0, 0, frontend_level_select_maintain},
  { 0,  0, 0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 233,  95, 235,424, 14, frontend_draw_level_select_button, 0, 201,  0,      {48},            0, 0, frontend_level_select_maintain},
  { 0,  0, 0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 255,  95, 257,424, 14, frontend_draw_level_select_button, 0, 201,  0,      {49},            0, 0, frontend_level_select_maintain},
  { 0,  0, 0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 277,  95, 279,424, 14, frontend_draw_level_select_button, 0, 201,  0,      {50},            0, 0, frontend_level_select_maintain},
  { 0,  0, 0, 0, 0, frontend_level_select,NULL,frontend_over_button,     0,  95, 299,  95, 301,424, 14, frontend_draw_level_select_button, 0, 201,  0,      {51},            0, 0, frontend_level_select_maintain},
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL},
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

#define frontend_select_campaign_items_visible  7
struct GuiButtonInit frontend_select_campaign_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,     {108},            0, 0, NULL},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 128,  82, 128,220, 26, frontnet_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 154,  82, 154,450,180, frontnet_draw_scroll_box,          0, 201,  0,      {26},            0, 0, NULL},
  { 1,  0, 0, 0, 0, frontend_campaign_select_up,NULL,frontend_over_button,0, 532,153, 532, 153, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontend_campaign_select_up_maintain},
  { 1,  0, 0, 0, 0, frontend_campaign_select_down,NULL,frontend_over_button,0,532,321,532, 321, 26, 14, frontnet_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontend_campaign_select_down_maintain},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 167, 536, 167, 10,154, frontend_draw_campaign_scroll_tab, 0, 201,  0,      {40},            0, 0, NULL},
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 129, 102, 129,220, 26, frontend_draw_text,                0, 201,  0,     {109},            0, 0, NULL},
  { 0,  0, 0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 167,  95, 169,424, 14, frontend_draw_campaign_select_button,0,201, 0,      {45},            0, 0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 189,  95, 191,424, 14, frontend_draw_campaign_select_button,0,201, 0,      {46},            0, 0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 211,  95, 213,424, 14, frontend_draw_campaign_select_button,0,201, 0,      {47},            0, 0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 233,  95, 235,424, 14, frontend_draw_campaign_select_button,0,201, 0,      {48},            0, 0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 255,  95, 257,424, 14, frontend_draw_campaign_select_button,0,201, 0,      {49},            0, 0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 277,  95, 279,424, 14, frontend_draw_campaign_select_button,0,201, 0,      {50},            0, 0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, 0, frontend_campaign_select,NULL,frontend_over_button,  0,  95, 299,  95, 301,424, 14, frontend_draw_campaign_select_button,0,201, 0,      {51},            0, 0, frontend_campaign_select_maintain},
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL},
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiMenu main_menu =
 { 1, 0, 1, main_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu room_menu =
 { 2, 0, 1, room_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu spell_menu =
 { 3, 0, 1, spell_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu spell_lost_menu =
 { 38, 0, 1, spell_lost_menu_buttons,                    0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu trap_menu =
 { 4, 0, 1, trap_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu creature_menu =
 { 5, 0, 1, creature_menu_buttons,                       0,   0, 140, 400, gui_activity_background,     0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu event_menu =
 { 6, 0, 1, event_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu options_menu =
 { 8, 0, 1, options_menu_buttons,       POS_GAMECTR,POS_GAMECTR, 308, 120, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu instance_menu =
 { 9, 0, 1, instance_menu_buttons,      POS_GAMECTR,POS_GAMECTR, 318, 120, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu query_menu =
 { 7, 0, 1, query_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu quit_menu =
 { 10, 0, 1, quit_menu_buttons,          POS_GAMECTR,POS_GAMECTR,264, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu load_menu =
 { 11, 0, 4, load_menu_buttons,          POS_GAMECTR,POS_GAMECTR,436, 350, gui_pretty_background,       0, NULL,    init_load_menu,          0, 1, 0,};
struct GuiMenu save_menu =
 { 12, 0, 4, save_menu_buttons,          POS_GAMECTR,POS_GAMECTR,436, 350, gui_pretty_background,       0, NULL,    init_save_menu,          0, 1, 0,};
struct GuiMenu video_menu =
 { 13, 0, 4, video_menu_buttons,         POS_GAMECTR,POS_GAMECTR,160, 170, gui_pretty_background,       0, NULL,    init_video_menu,         0, 1, 0,};
struct GuiMenu sound_menu =
 { 14, 0, 4, sound_menu_buttons,         POS_GAMECTR,POS_GAMECTR,280, 170, gui_pretty_background,       0, NULL,    init_audio_menu,         0, 1, 0,};
struct GuiMenu error_box =
 { 15, 0, 1, error_box_buttons,          POS_GAMECTR,POS_GAMECTR,280, 180, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu text_info_menu =
// { 16, 0, 4, text_info_buttons,                        160, 316, 480,  86, gui_round_glass_background,  0, NULL,    reset_scroll_window,     0, 0, 0,};
 { 16, 0, 4, text_info_buttons,                  160, POS_SCRBTM,480,  86, gui_round_glass_background,  0, NULL,    reset_scroll_window,     0, 0, 0,};
struct GuiMenu hold_audience_menu =
 { 17, 0, 4, hold_audience_buttons,      POS_GAMECTR,POS_GAMECTR,200, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu dungeon_special_menu =
 { 27, 0, 4, dungeon_special_buttons,            160, POS_SCRBTM,480, 86, gui_round_glass_background,  0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu resurrect_creature_menu =
 { 28, 0, 4, resurrect_creature_buttons, POS_GAMECTR,POS_GAMECTR,350, 300, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu transfer_creature_menu =
 { 29, 0, 4, transfer_creature_buttons,  POS_GAMECTR,POS_GAMECTR,350, 300, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu armageddon_menu =
 { 30, 0, 4, armageddon_buttons,         POS_GAMECTR,POS_GAMECTR,200, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu frontend_main_menu =
 { 18, 0, 1, frontend_main_menu_buttons,          0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_load_menu =
 { 19, 0, 1, frontend_load_menu_buttons,          0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
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
struct GuiMenu frontend_statistics_menu =
 { 25, 0, 1, frontend_statistics_buttons,         0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_high_score_table_menu =
 { 26, 0, 1, frontend_high_score_score_buttons,   0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu creature_query_menu1 =
 { 31, 0, 1, creature_query_buttons1,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu2 =
 { 35, 0, 1, creature_query_buttons2,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu3 =
 { 32, 0, 1, creature_query_buttons3,             0,          0, 140, 400, gui_creature_query_background2,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu battle_menu =
// { 34, 0, 4, battle_buttons,                    160,        300, 480, 102, gui_round_glass_background,  0, NULL,    NULL,                    0, 0, 0,};
 { 34, 0, 4, battle_buttons,                    160, POS_SCRBTM, 480, 102, gui_round_glass_background,  0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_define_keys_menu =
 { 36, 0, 1, frontend_define_keys_buttons,        0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu autopilot_menu =
 { 37, 0, 4, autopilot_menu_buttons,     POS_GAMECTR,POS_GAMECTR,224, 120, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu frontend_option_menu =
 { 39, 0, 1, frontend_option_buttons,             0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    frontend_init_options_menu,0,0,0,};
struct GuiMenu frontend_select_level_menu =
 { 40, 0, 1, frontend_select_level_buttons,       0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_select_campaign_menu =
 { 41, 0, 1, frontend_select_campaign_buttons,    0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};

// Note: update size in .h file when changing this array.
struct GuiMenu *menu_list[] = {
    NULL,
    &main_menu,
    &room_menu,
    &spell_menu,
    &trap_menu,
    &creature_menu,
    &event_menu,
    &query_menu,
    &options_menu,
    &instance_menu,
    &quit_menu,
    &load_menu,
    &save_menu,
    &video_menu,
    &sound_menu,
    &error_box,
    &text_info_menu,
    &hold_audience_menu,
    &frontend_main_menu,
    &frontend_load_menu,
    &frontend_net_service_menu,
    &frontend_net_session_menu,
    &frontend_net_start_menu,
    &frontend_net_modem_menu,
    &frontend_net_serial_menu,
    &frontend_statistics_menu,
    &frontend_high_score_table_menu,
    &dungeon_special_menu,
    &resurrect_creature_menu,
    &transfer_creature_menu,
    &armageddon_menu,
    &creature_query_menu1,
    &creature_query_menu3,
    NULL,
    &battle_menu,
    &creature_query_menu2,
    &frontend_define_keys_menu,
    &autopilot_menu,
    &spell_lost_menu,
    &frontend_option_menu,
    &frontend_select_level_menu,
    &frontend_select_campaign_menu,
    NULL,
};

// If adding entries here, you should also update FRONTEND_BUTTON_INFO_COUNT.
struct FrontEndButtonData frontend_button_info[] = {
    {0,   0}, // [0]
    {343, 0},
    {360, 1},
    {345, 1},
    {347, 1},
    {359, 1},
    {348, 1},
    {345, 0},
    {346, 1},
    {349, 1},
    {350, 0}, // [10]
    {351, 0},
    {402, 0},
    {400, 1},
    {399, 1},
    {401, 1},
    {403, 1},
    {201, 1},
    {201, 1},
    {396, 1},
    {201, 1}, // [20]
    {201, 1},
    {406, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {395, 2},
    {408, 2}, // [30]
    {405, 2},
    {407, 2},
    {397, 2},
    {398, 2},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1}, // [40]
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1}, // [50]
    {201, 1},
    {201, 1},
    {409, 0},
    {410, 0},
    {353, 2},
    {352, 2},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1}, // [60]
    {355, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {411, 1},
    {412, 1},
    {533, 1},
    {414, 1},
    {201, 1}, // [70]
    {354, 1},
    {534, 1},
    {534, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1},
    {201, 1}, // [80]
    {201, 1},
    {418, 1},
    {419, 1},
    {356, 0},
    {431, 0},
    {457, 0},
    {458, 2},
    {415, 1},
    {201, 1},
    {201, 1}, // [90]
    {201, 1},
    {468, 0},
    {201, 1},
    {201, 1},
    {468, 1},
    {716, 0},
    {716, 1},
    {840, 1},
    {718, 1},
    {850, 1}, // [100]
    {849, 1},
    {843, 1},
    {845, 1},
    {431, 1},
    {201, 0},
    {941, 1},
    {941, 0},
    {942, 0}, // [108] "Land selection"
    {943, 2}, // [109] "Campaigns"
};

struct EventTypeInfo event_button_info[] = {
  {260, 201, 201,    1,   1},
  {260, 698, 672,  300, 250},
  {262, 699, 673,   -1,   0},
  {258, 674, 201,   -1,   0},
  {260, 701, 675,  300,   0},
  {250, 703, 677, 1200,   0},
  {256, 706, 680, 1200,   0},
  {252, 702, 676, 1200,   0},
  {254, 704, 678, 1200,   0},
  {254, 705, 679, 1200,   0},
  {260, 691, 665, 1200,   0},
  {266, 690, 664, 1200, 500},
  {266, 692, 666, 1200,   0},
  {266, 695, 669, 1200,   0},
  {266, 693, 667, 1200,   0},
  {266, 694, 668, 1200,   0},
  {260, 707, 681, 1200,   0},
  {260, 708, 682, 1200, 500},
  {260, 709, 683,  300, 200},
  {260, 710, 684,  300, 250},
  {260, 711, 685,  300, 500},
  {268, 696, 201, 1200,   0},
  {260, 697, 671, 1200,   0},
  {260, 712, 686,  300, 500},
  {266, 713, 687,  300,   0},
  {266, 714, 688,  300,   0},
  {266, 715, 689,  300,   0},
  {268, 696, 201, 1200,   0},
};
/*
struct DoorDesc doors[] = {
  {102,  13, 102,  20,  97, 155, 0, 0, 0, 0, 200},
  {253,   0, 257,   0, 103, 118, 0, 0, 0, 0, 201},
  {399,   0, 413,   0, 114, 144, 0, 0, 0, 0, 202},
  {511,  65, 546,  85,  94, 160, 0, 0, 0, 0, 203},
  {149, 211, 153, 232,  55,  84, 0, 0, 0, 0, 204},
  {258, 176, 262, 178,  60,  84, 0, 0, 0, 0, 205},
  {364, 183, 375, 191,  70,  95, 0, 0, 0, 0, 206},
  {466, 257, 473, 261,  67,  94, 0, 0, 0, 0, 207},
  {254, 368, 260, 391, 128,  80, 0, 0, 0, 0, 208},
};
*/

struct TbLoadFiles torture_load_files[] = {
  {"ldata/fronttor.tab", (unsigned char **)&fronttor_sprites, (unsigned char **)&fronttor_end_sprites, 0, 0, 0},
  {"ldata/fronttor.dat", (unsigned char **)&fronttor_data,    (unsigned char **)&fronttor_end_data,    0, 0, 0},
  {"",                    NULL,                                NULL,                                   0, 0, 0},
};

struct TbSetupSprite setup_torture_sprites[] = {
  {&doors[0].sprites, &doors[0].sprites_end, &doors[0].data},
  {&doors[1].sprites, &doors[1].sprites_end, &doors[1].data},
  {&doors[2].sprites, &doors[2].sprites_end, &doors[2].data},
  {&doors[3].sprites, &doors[3].sprites_end, &doors[3].data},
  {&doors[4].sprites, &doors[4].sprites_end, &doors[4].data},
  {&doors[5].sprites, &doors[5].sprites_end, &doors[5].data},
  {&doors[6].sprites, &doors[6].sprites_end, &doors[6].data},
  {&doors[7].sprites, &doors[7].sprites_end, &doors[7].data},
  {&doors[8].sprites, &doors[8].sprites_end, &doors[8].data},
  {&fronttor_sprites, &fronttor_end_sprites, &fronttor_data},
  {NULL,              NULL,                  NULL,}
};

const struct DemoItem demo_item[] = {
    {DIK_SwitchState, (char *)13},
/*
    {DIK_LoadPacket, "PACKET1.SAV"},
    {DIK_LoadPacket, "PACKET2.SAV"},
*/
    {DIK_PlaySmkVideo, "intromix.smk"},
    {DIK_ListEnd, NULL},
};

const unsigned long alliance_grid[4][4] = {
  {0x00, 0x01, 0x02, 0x04,},
  {0x01, 0x00, 0x08, 0x10,},
  {0x02, 0x08, 0x00, 0x20,},
  {0x04, 0x10, 0x20, 0x00,},
};

const char *keeper_netconf_file = "fxconfig.net";

const struct ConfigInfo default_net_config_info = {
    -1, {4, 3, 4, 3, 4, 3, 4, 3, }, -1,
    "ATZ",
    "ATDT",
    "ATH",
    "ATS0=1",
    "",
    "",
};

#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)
struct TbSprite *testfont[TESTFONTS_COUNT];
struct TbSprite *testfont_end[TESTFONTS_COUNT];
unsigned long testfont_data[TESTFONTS_COUNT];
unsigned char *testfont_palette[3];
long num_chars_in_font = 128;
#endif

long torture_doors_available = TORTURE_DOORS_COUNT;


// Boxes used for cheat menu
struct GuiBox *gui_box=NULL;
struct GuiBox *gui_cheat_box=NULL;

struct GuiBox *first_box=NULL;
struct GuiBox *last_box=NULL;
struct GuiBox gui_boxes[3];
//struct TbSprite *font_sprites=NULL;
//struct TbSprite *end_font_sprites=NULL;
//unsigned char *font_data=NULL;
struct DraggingBox dragging_box;

int status_panel_width = 140;

/******************************************************************************/
long stat_return_c_slong(long *ptr)
{
  if (ptr == NULL) return 0;
  return *ptr;
}

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
short menu_is_active(short idx)
{
  return (menu_id_to_number(idx) >= 0);
}

TbBool a_menu_window_is_active(void)
{
  if (no_of_active_menus <= 0)
    return false;
  int i,k;
  for (i=0; i<no_of_active_menus; i++)
  {
      k = menu_stack[i];
      if (!is_toggleable_menu(k))
        return true;
  }
  return false;
}

struct GuiMenu *get_active_menu(int id)
{
    if (id < 0)
        id = 0;
    if (id >= ACTIVE_MENUS_COUNT)
        id = 0;
    return &active_menus[id];
}

void get_player_gui_clicks(void)
{
  struct PlayerInfo *player;
  struct Thing *thing;
  player = get_my_player();

  if ( ((game.numfield_C & 0x01) != 0) && ((game.numfield_C & 0x80) == 0))
    return;

  switch (player->view_type)
  {
  case 3:
      if (right_button_released)
      {
        thing = thing_get(player->field_2F);
        if (thing->class_id == TCls_Creature)
        {
          if (a_menu_window_is_active())
          {
            game.numfield_D &= 0xF7u;
            player->field_0 &= 0xF7u;
            turn_off_all_window_menus();
          } else
          {
            game.numfield_D |= 0x08;
            player->field_0 |= 0x08;
            turn_on_menu(GMnu_QUERY);
          }
        }
      }
      break;
  case 2:
  case 4:
  case 5:
  case 6:
      break;
  default:
      if (right_button_released)
      {
        if ((player->work_state != 5) || power_hand_is_empty(player))
        {
          if ( !turn_off_all_window_menus() )
          {
            if (player->work_state == 12)
            {
              turn_off_query_menus();
              set_players_packet_action(player, PckA_SetPlyrState, 1, 0, 0, 0);
              right_button_released = 0;
            } else
            if ((player->work_state != 15) && (player->work_state != 1))
            {
              set_players_packet_action(player, PckA_SetPlyrState, 1, 0, 0, 0);
              right_button_released = 0;
            }
          }
        }
      } else
      if (lbKeyOn[KC_ESCAPE])
      {
        lbKeyOn[KC_ESCAPE] = 0;
        if ( a_menu_window_is_active() )
        {
          turn_off_all_window_menus();
        } else
        {
          turn_on_menu(GMnu_OPTIONS);
        }
      }
      break;
  }

  if ( game_is_busy_doing_gui() )
  {
    set_players_packet_control(player, 0x4000u);
  }
}

void add_message(long plyr_idx, char *msg)
{
  struct NetMessage *nmsg;
  long i,k;
  i = net_number_of_messages;
  if (i >= NET_MESSAGES_COUNT)
  {
    for (k=0; k < (NET_MESSAGES_COUNT-1); k++)
    {
      memcpy(&net_message[k], &net_message[k+1], sizeof(struct NetMessage));
    }
    i = NET_MESSAGES_COUNT-1;
  }
  nmsg = &net_message[i];
  nmsg->plyr_idx = plyr_idx;
  strncpy(nmsg->text, msg, NET_MESSAGE_LEN-1);
  nmsg->text[NET_MESSAGE_LEN-1] = '\0';
  i++;
  net_number_of_messages = i;
  if (net_message_scroll_offset+4 < i)
    net_message_scroll_offset = i-4;
}

/*
 * Checks if all the network players are using compatible version of DK.
 */
TbBool validate_versions(void)
{
  struct PlayerInfo *player;
  long i,ver;
  ver = -1;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if ((net_screen_packet[i].field_4 & 0x01) != 0)
    {
      if (ver == -1)
        ver = player->field_4E7;
      if (player->field_4E7 != ver)
        return false;
    }
  }
  return true;
}

void versions_different_error(void)
{
  struct TbNetworkPlayerName *plyr_nam;
  struct ScreenPacket *nspckt;
  char text[MESSAGE_TEXT_LEN];
  char *str;
  int i;
  if (LbNetwork_Stop())
  {
    ERRORLOG("LbNetwork_Stop() failed");
  }
  lbKeyOn[KC_ESCAPE] = 0;
  lbKeyOn[KC_SPACE] = 0;
  lbKeyOn[KC_RETURN] = 0;
  text[0] = '\0';
  // Preparing message
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    plyr_nam = &net_player[i];
    nspckt = &net_screen_packet[i];
    if ((nspckt->field_4 & 0x01) != 0)
    {
      str = buf_sprintf("%s(%d.%02d) ", plyr_nam->name, nspckt->field_6, nspckt->field_8);
      strncat(text, str, MESSAGE_TEXT_LEN-strlen(text));
      text[MESSAGE_TEXT_LEN-1] = '\0';
    }
  }
  // Waiting for users reaction
  while ( 1 )
  {
    if (lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_SPACE] || lbKeyOn[KC_RETURN])
      break;
    LbWindowsControl();
    if (LbScreenLock() == Lb_SUCCESS)
    {
      draw_text_box(text);
      LbScreenUnlock();
    }
    LbScreenSwap();
  }
  // Checking where to go back
  if (setup_old_network_service())
    frontend_set_state(FeSt_NET_SESSION);
  else
    frontend_set_state(FeSt_MAIN_MENU);
}

void create_error_box(unsigned short msg_idx)
{
  if ( !game.packet_load_enable )
  {
    //change the length into  when gui_error_text will not be exported
    strncpy(gui_error_text, gui_strings[msg_idx],TEXT_BUFFER_LENGTH-1);
    turn_on_menu(GMnu_ERROR_BOX);
  }
}

/*
 * Checks if mouse pointer is currently over a specific button.
 * @return Returns true it mouse is over the button.
 */
short check_if_mouse_is_over_button(struct GuiButton *gbtn)
{
  int mouse_x = GetMouseX();
  int mouse_y = GetMouseY();
  int x = gbtn->pos_x;
  int y = gbtn->pos_y;
  if ( (mouse_x >= x) && (mouse_x < x + gbtn->width)
    && (mouse_y >= y) && (mouse_y < y + gbtn->height)
    && (gbtn->field_0 & 0x04) )
    return true;
  return false;
}

void demo(void)
{
  static long index = 0;
  char *fname;
  switch (demo_item[index].numfield_0)
  {
  case DIK_PlaySmkVideo:
      fname = prepare_file_path(FGrp_LoData,demo_item[index].fname);
      play_smacker_file(fname, 1);
      break;
  case DIK_LoadPacket:
      fname = prepare_file_path(FGrp_FxData,demo_item[index].fname);
      wait_for_cd_to_be_available();
      if ( LbFileExists(fname) )
      {
        strcpy(game.packet_fname, fname);
        game.packet_load_enable = 1;
        game.turns_fastforward = 0;
        frontend_set_state(FeSt_PACKET_DEMO);
      }
      break;
  case DIK_SwitchState:
      frontend_set_state((long)demo_item[index].fname);
      break;
  }
  index++;
  if (demo_item[index].numfield_0 == DIK_ListEnd)
    index = 0;
}

void turn_on_event_info_panel_if_necessary(unsigned short evnt_idx)
{
  if (game.event[evnt_idx%EVENTS_COUNT].kind == 2)
  {
    if (!menu_is_active(GMnu_BATTLE))
      turn_on_menu(GMnu_BATTLE);
  } else
  {
    if (!menu_is_active(GMnu_TEXT_INFO))
      turn_on_menu(GMnu_TEXT_INFO);
  }
}

void activate_event_box(long evnt_idx)
{
  struct PlayerInfo *player;
  player = get_my_player();
  set_players_packet_action(player, 115, evnt_idx, 0,0,0);
}

void kill_button(struct GuiButton *gbtn)
{
  if (gbtn != NULL)
    set_flag_byte(&gbtn->field_0, 0x01, false);
}

void kill_button_area_input(void)
{
  if (input_button != NULL)
    strcpy((char *)input_button->field_33, backup_input_field);
  input_button = NULL;
}

void kill_menu(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  int i;
  if (gmnu->field_1)
  {
    gmnu->field_1 = 0;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
      gbtn = &active_buttons[i];
      if ((gbtn->field_0 & 0x01) && (gbtn->gmenu_idx == gmnu->field_14))
        kill_button(gbtn);
    }
  }
}

void remove_from_menu_stack(short mnu_id)
{
  unsigned short i;
  for (i=0; i<no_of_active_menus; i++)
  {
    if (menu_stack[i] == mnu_id)
    {
      while (i < no_of_active_menus-1)
      {
        menu_stack[i] = menu_stack[i+1];
        i++;
      }
      break;
    }
  }
  if (i < no_of_active_menus)
    no_of_active_menus--;
}

void turn_off_menu(short mnu_idx)
{
  struct GuiMenu *gmnu;
  long menu_num;
  if ((mnu_idx >= 13) && (mnu_idx <= 14))
    save_settings();
  menu_num = menu_id_to_number(mnu_idx);
  if (menu_num >= 0)
  {
    if (game_is_busy_doing_gui_string_input())
    {
      if (input_button->gmenu_idx == menu_num)
        kill_button_area_input();
    }
    gmnu = get_active_menu(menu_num);
    gmnu->field_1 = 3;
    if (update_menu_fade_level(gmnu) == -1)
    {
      kill_menu(gmnu);
      remove_from_menu_stack(gmnu->field_0);
    }
  }
}

short game_is_busy_doing_gui(void)
{
  struct PlayerInfo *player;
  struct SpellData *pwrdata;
  struct Thing *thing;
  long spl_idx;
  if (!busy_doing_gui)
    return false;
  if (battle_creature_over <= 0)
    return true;
  player = get_my_player();
  spl_idx = -1;
  if (player->work_state < PLAYER_STATES_COUNT)
    spl_idx = player_state_to_spell[player->work_state];
  pwrdata = get_power_data(spl_idx);
  if (!power_data_is_invalid(pwrdata))
  {
    if (!pwrdata->flag_19)
      return true;
    thing = thing_get(battle_creature_over);
    return  (thing->owner != player->id_number) && (!pwrdata->flag_1A);
  }
  return false;
}

char get_button_area_input(struct GuiButton *gbtn, int a2)
{
  return _DK_get_button_area_input(gbtn, a2);
}

void gui_activity_background(struct GuiMenu *gmnu)
{
  SYNCDBG(9,"Starting");
  _DK_gui_activity_background(gmnu);
}

void gui_pretty_background(struct GuiMenu *gmnu)
{
  SYNCDBG(9,"Starting");
  _DK_gui_pretty_background(gmnu);
}

void frontend_copy_mnu_background(struct GuiMenu *gmnu)
{
  SYNCDBG(9,"Starting");
  frontend_copy_background_at(gmnu->pos_x,gmnu->pos_y,gmnu->width,gmnu->height);
}

void draw_load_button(struct GuiButton *gbtn)
{
  if (gbtn == NULL) return;
  gbtn->height = 32;
  if ((gbtn->field_1) || (gbtn->field_2))
  {
    draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width);
    draw_lit_bar64k(gbtn->scr_pos_x - 6, gbtn->scr_pos_y - 6, gbtn->width + 6);
  } else
  {
    draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width);
  }
  if (gbtn->field_33 != NULL)
  {
    sprintf(gui_textbuf, "%s", (const char *)gbtn->field_33);
    draw_button_string(gbtn, gui_textbuf);
  }
}

int frontend_font_char_width(int fnt_idx,char c)
{
  struct TbSprite *fnt;
  int i;
  fnt = frontend_font[fnt_idx];
  i = (unsigned short)c - 31;
  if (i >= 0)
    return fnt[i].SWidth;
  return 0;
}

int frontend_font_string_width(int fnt_idx,char *str)
{
  LbTextSetFont(frontend_font[fnt_idx]);
  return LbTextStringWidth(str);
}

void frontend_draw_button(struct GuiButton *gbtn, unsigned short btntype, const char *text, unsigned int drw_flags)
{
  static const long large_button_sprite_anims[] =
      { 2, 5, 8, 11, 14, 11, 8, 5, };
  unsigned int fbinfo_idx;
  unsigned int spridx;
  int fntidx;
  long x,y;
  int h;
  SYNCDBG(9,"Drawing type %d, text \"%s\"",(int)btntype,text);
  fbinfo_idx = (unsigned int)gbtn->field_33;
  if ((gbtn->field_0 & 0x08) == 0)
  {
    fntidx = 3;
    spridx = 14;
  } else
  if ((fbinfo_idx>0) && (frontend_mouse_over_button == fbinfo_idx))
  {
    fntidx = 2;
    spridx = large_button_sprite_anims[((timeGetTime()-frontend_mouse_over_button_start_time)/100) & 7];
  } else
  {
    fntidx = frontend_button_info[fbinfo_idx%FRONTEND_BUTTON_INFO_COUNT].field_2;
    spridx = 14;
  }
  x = gbtn->scr_pos_x;
  y = gbtn->scr_pos_y;
  switch (btntype)
  {
   case 1:
      LbSpriteDraw(x, y, &frontend_sprite[spridx]);
      x += frontend_sprite[spridx].SWidth;
      LbSpriteDraw(x, y, &frontend_sprite[spridx+1]);
      x += frontend_sprite[spridx+1].SWidth;
      break;
  case 2:
      LbSpriteDraw(x, y, &frontend_sprite[spridx]);
      x += frontend_sprite[spridx].SWidth;
      LbSpriteDraw(x, y, &frontend_sprite[spridx+1]);
      x += frontend_sprite[spridx+1].SWidth;
      LbSpriteDraw(x, y, &frontend_sprite[spridx+1]);
      x += frontend_sprite[spridx+1].SWidth;
      break;
  default:
      LbSpriteDraw(x, y, &frontend_sprite[spridx]);
      x += frontend_sprite[spridx].SWidth;
      break;
  }
  LbSpriteDraw(x, y, &frontend_sprite[spridx+2]);
  if (text != NULL)
  {
    lbDisplay.DrawFlags = drw_flags;
    LbTextSetFont(frontend_font[fntidx]);
    h = LbTextHeight(text);
    x = gbtn->scr_pos_x + ((40) >> 1);
    y = gbtn->scr_pos_y + ((frontend_sprite[spridx].SHeight-h) >> 1);
    LbTextSetWindow(x, y, gbtn->width-40, h);
    LbTextDraw(0, 0, text);
  }
}

void frontend_draw_large_menu_button(struct GuiButton *gbtn)
{
  unsigned long btninfo_idx;
  int idx;
  char *text;
  btninfo_idx = (unsigned long)gbtn->field_33;
  if (btninfo_idx < FRONTEND_BUTTON_INFO_COUNT)
    idx = frontend_button_info[btninfo_idx].capstr_idx;
  else
    idx = -1;
  if ((idx >= 0) && (idx < STRINGS_MAX))
    text = gui_strings[idx];
  else
    text = NULL;
  frontend_draw_button(gbtn, 1, text, 0x0100);
}

void gui_area_null(struct GuiButton *gbtn)
{
  if (gbtn->field_0 & 0x08)
  {
    LbSpriteDraw(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
      &button_sprite[gbtn->field_29]);
  } else
  {
    LbSpriteDraw(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
      &button_sprite[gbtn->field_29]);
  }
}

void gui_round_glass_background(struct GuiMenu *gmnu)
{
  SYNCDBG(19,"Starting");
  _DK_gui_round_glass_background(gmnu);
}

void gui_creature_query_background1(struct GuiMenu *gmnu)
{
  SYNCDBG(19,"Starting");
  _DK_gui_creature_query_background1(gmnu);
}

void gui_creature_query_background2(struct GuiMenu *gmnu)
{
  SYNCDBG(19,"Starting");
  _DK_gui_creature_query_background2(gmnu);
}

void reset_scroll_window(struct GuiMenu *gmnu)
{
  _DK_reset_scroll_window(gmnu);
}

void update_loadsave_input_strings(struct CatalogueEntry *game_catalg)
{
    struct CatalogueEntry *centry;
    long slot_num;
    char *text;
    SYNCDBG(6,"Starting");
    for (slot_num=0; slot_num < SAVE_SLOTS_COUNT; slot_num++)
    {
        centry = &game_catalg[slot_num];
        if ((centry->flags & CEF_InUse) != 0)
          text = centry->textname;
        else
          text = gui_strings[342]; // UNUSED
        strncpy(input_string[slot_num], text, SAVE_TEXTNAME_LEN);
    }
}

void init_load_menu(struct GuiMenu *gmnu)
{
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  player = get_my_player();
  set_players_packet_action(player, 82, 1, 1, 0, 0);
  load_game_save_catalogue();
  update_loadsave_input_strings(save_game_catalogue);
}

void init_save_menu(struct GuiMenu *gmnu)
{
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  player = get_my_player();
  set_players_packet_action(player, 82, 1, 1, 0, 0);
  load_game_save_catalogue();
  update_loadsave_input_strings(save_game_catalogue);
}

void init_video_menu(struct GuiMenu *gmnu)
{
  _DK_init_video_menu(gmnu);
}

void init_audio_menu(struct GuiMenu *gmnu)
{
  _DK_init_audio_menu(gmnu);
}

void maintain_event_button(struct GuiButton *gbtn)
{
  struct Dungeon *dungeon;
  struct Event *event;
  unsigned short evnt_idx;
  unsigned long i;

  dungeon = get_players_num_dungeon(my_player_number);
  i = (unsigned long)gbtn->field_33;
  evnt_idx = dungeon->field_13A7[i&0xFF];

  if ((dungeon->field_1173 != 0) && (evnt_idx == dungeon->field_1173))
  {
      turn_on_event_info_panel_if_necessary(dungeon->field_1173);
  }

  if (evnt_idx == 0)
  {
    gbtn->field_1B |= 0x4000u;
    gbtn->field_29 = 0;
    set_flag_byte(&gbtn->field_0, 0x08, false);
    gbtn->field_1 = 0;
    gbtn->field_2 = 0;
    gbtn->tooltip_id = 201;
    return;
  }
  event = &game.event[evnt_idx];
  if ((event->kind == 3) && (new_objective))
  {
    activate_event_box(evnt_idx);
  }
  gbtn->field_29 = event_button_info[event->kind].field_0;
  if ((event->kind == 2) && ((event->mappos_x != 0) || (event->mappos_y != 0))
      && ((game.play_gameturn & 0x01) != 0))
  {
    gbtn->field_29 += 2;
  } else
  if ((event->kind == 21) && (event->target < 0)
     && ((game.play_gameturn & 0x01) != 0))
  {
    gbtn->field_29 += 2;
  }
  gbtn->tooltip_id = event_button_info[event->kind].field_4;
  set_flag_byte(&gbtn->field_0, 0x08, true);
  gbtn->field_1B = 0;
}

void menu_tab_maintain(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  set_flag_byte(&gbtn->field_0, 0x08, (player->victory_state != VicS_LostLevel));
}

void maintain_turn_on_autopilot(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  unsigned long cplr_model;
  player = get_my_player();
  cplr_model = game.computer[player->id_number%PLAYERS_COUNT].model;
  if ((cplr_model >= 0) && (cplr_model < 10))
    gbtn->tooltip_id = computer_types[cplr_model];
  else
    ERRORLOG("Illegal computer player");
}

void maintain_room(struct GuiButton *gbtn)
{
  _DK_maintain_room(gbtn);
}

void maintain_big_room(struct GuiButton *gbtn)
{
  _DK_maintain_big_room(gbtn);
}

void maintain_spell(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  long i;
  player = get_my_player();
  dungeon = get_players_dungeon(player);
  i = (unsigned long)(gbtn->field_33) & 0xff;
  if (!is_power_available(player->id_number,i))
  {
    gbtn->field_1B |= 0x8000u;
    gbtn->field_0 &= 0xF7;
  } else
  if (i == 19)
  {
      if (game.field_150356 != 0)
      {
        gbtn->field_1B |= 0x8000u;
        gbtn->field_0 &= 0xF7;
      } else
      {
        gbtn->field_1B = 0;
        gbtn->field_0 |= 0x08;
      }
  } else
  if (i == 9)
  {
      if (dungeon->field_88C[0])
      {
        gbtn->field_1B |= 0x8000u;
        gbtn->field_0 &= 0xF7;
      } else
      {
        gbtn->field_1B = 0;
        gbtn->field_0 |= 0x08;
      }
  } else
  {
    gbtn->field_1B = 0;
    gbtn->field_0 |= 0x08;
  }
}

void maintain_big_spell(struct GuiButton *gbtn)
{
  _DK_maintain_big_spell(gbtn);
}

void maintain_trap(struct GuiButton *gbtn)
{
  _DK_maintain_trap(gbtn);
}

void maintain_door(struct GuiButton *gbtn)
{
  struct TrapData *trap_dat;
  struct Dungeon *dungeon;
  int i;
  i = (unsigned int)gbtn->field_33;
  trap_dat = &trap_data[i%MANUFCTR_TYPES_COUNT];
  dungeon = get_players_num_dungeon(my_player_number);
  if (dungeon->door_placeable[trap_dat->field_4%DOOR_TYPES_COUNT])
  {
    gbtn->field_1B = 0;
    set_flag_byte(&gbtn->field_0, 0x08, true);
  } else
  {
    gbtn->field_1B |= 0x8000u;
    set_flag_byte(&gbtn->field_0, 0x08, false);
  }
}

void maintain_big_trap(struct GuiButton *gbtn)
{
  _DK_maintain_big_trap(gbtn);
}

void maintain_activity_up(struct GuiButton *gbtn)
{
  _DK_maintain_activity_up(gbtn);
}

void maintain_activity_down(struct GuiButton *gbtn)
{
  _DK_maintain_activity_down(gbtn);
}

void maintain_activity_pic(struct GuiButton *gbtn)
{
  _DK_maintain_activity_pic(gbtn);
}

void maintain_activity_row(struct GuiButton *gbtn)
{
  _DK_maintain_activity_row(gbtn);
}

void maintain_loadsave(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, ((game.system_flags & GSF_NetworkActive) == 0));
}

void maintain_prison_bar(struct GuiButton *gbtn)
{
  _DK_maintain_prison_bar(gbtn);
}

void maintain_room_and_creature_button(struct GuiButton *gbtn)
{
  _DK_maintain_room_and_creature_button(gbtn);
}

void maintain_ally(struct GuiButton *gbtn)
{
  _DK_maintain_ally(gbtn);
}

void gui_load_game_maintain(struct GuiButton *gbtn)
{
  long slot_num;
  struct CatalogueEntry *centry;
  if (gbtn != NULL)
      slot_num = gbtn->field_1B;
  else
      slot_num = 0;
  centry = &save_game_catalogue[slot_num];
  set_flag_byte(&gbtn->field_0, 0x08, ((centry->flags & CEF_InUse) != 0));
}

void gui_video_cluedo_maintain(struct GuiButton *gbtn)
{
  _DK_gui_video_cluedo_maintain(gbtn);
}

void fake_button_click(long btn_idx)
{
  _DK_fake_button_click(btn_idx);
}

void maintain_zoom_to_event(struct GuiButton *gbtn)
{
  struct Dungeon *dungeon;
  struct Event *event;
  dungeon = get_players_num_dungeon(my_player_number);
  if (dungeon->field_1173)
  {
    event = &(game.event[dungeon->field_1173]);
    if ((event->mappos_x != 0) || (event->mappos_y != 0))
    {
      gbtn->field_0 |= 0x08;
      return;
    }
  }
  gbtn->field_0 &= 0xF7u;
}

void maintain_scroll_up(struct GuiButton *gbtn)
{
  _DK_maintain_scroll_up(gbtn);
}

void maintain_scroll_down(struct GuiButton *gbtn)
{
  _DK_maintain_scroll_down(gbtn);
}

void maintain_resurrect_creature_select(struct GuiButton *gbtn)
{
  _DK_maintain_resurrect_creature_select(gbtn);
}

void maintain_resurrect_creature_scroll(struct GuiButton *gbtn)
{
  _DK_maintain_resurrect_creature_scroll(gbtn);
}

void maintain_transfer_creature_select(struct GuiButton *gbtn)
{
  _DK_maintain_transfer_creature_select(gbtn);
}

void maintain_transfer_creature_scroll(struct GuiButton *gbtn)
{
  _DK_maintain_transfer_creature_scroll(gbtn);
}

void frontend_continue_game_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (continue_game_option_available != 0));
}

void frontend_main_menu_load_game_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (number_of_saved_games > 0));
}

void frontend_main_menu_netservice_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, true);
}

void frontend_main_menu_highscores_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, true);
}

void frontend_load_game_up_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (load_game_scroll_offset != 0));
}

void frontend_load_game_down_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (load_game_scroll_offset < number_of_saved_games-frontend_load_menu_items_visible+1));
}

long frontnet_number_of_players_in_session(void)
{
  long i,nplyr;
  nplyr = 0;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    if (net_player_info[i].active != 0)
      nplyr++;
  }
  return nplyr;
}

TbBool frontend_should_all_players_quit(void)
{
  return (net_service_index_selected <= 1);
}

TbBool frontend_is_player_allied(long idx1, long idx2)
{
  if (idx1 == idx2)
    return true;
  if ((idx1 < 0) || (idx1 >= PLAYERS_COUNT))
    return false;
  if ((idx2 < 0) || (idx2 >= PLAYERS_COUNT))
    return false;
  return ((frontend_alliances & alliance_grid[idx1][idx2]) != 0);
}

void frontend_set_alliance(long idx1, long idx2)
{
  if (frontend_is_player_allied(idx1, idx2))
    frontend_alliances &= ~alliance_grid[idx1][idx2];
  else
    frontend_alliances |= alliance_grid[idx1][idx2];
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

int frontend_load_data(void)
{
  return _DK_frontend_load_data();
}

void frontnet_serial_reset(void)
{
  _DK_frontnet_serial_reset();
}

void frontnet_modem_reset(void)
{
  _DK_frontnet_modem_reset();
}

void torture_play_sound(long door_id, TbBool state)
{
  if ((door_id < 0) || (door_id >= TORTURE_DOORS_COUNT))
    return;
  if (state)
  {
    play_sample_using_heap(0, doors[door_id].field_28, 0, 64, 100, -1, 2, 0);
    door_sound_state[door_id].field_0 = 0;
    door_sound_state[door_id].field_4 = 16;
  }
  else
  {
    door_sound_state[door_id].field_4 = -16;
  }
}

long torture_door_over_point(long x,long y)
{
  struct DoorDesc *door;
  long i;
  for (i=0; i < torture_doors_available; i++)
  {
    door = &doors[i];
    if ((x >= door->pos_x) && (x < door->pos_x+door->width))
      if ((y >= door->pos_y) && (y < door->pos_y+door->height))
        return i;
  }
  return -1;
}

void fronttorture_unload(void)
{
  LbDataFreeAll(torture_load_files);
  memcpy(&frontend_palette, frontend_backup_palette, PALETTE_SIZE);
  StopAllSamples();
  // Clearing the space used for torture graphics
  clear_light_system();
  clear_computer();
  clear_things_and_persons_data();
  clear_mapmap();
  clear_slabs();
  clear_rooms();
  clear_dungeons();
}

void fronttorture_load(void)
{
  struct PlayerInfo *player;
  char *fname;
  unsigned char *ptr;
  long i,k;
  wait_for_cd_to_be_available();
  frontend_load_data_from_cd();
  memcpy(frontend_backup_palette, &frontend_palette, PALETTE_SIZE);
  ptr = block_mem;
  // Load RAW/PAL background
  fname = prepare_file_path(FGrp_LoData,"torture.raw");
  torture_background = ptr;
  i = LbFileLoadAt(fname, ptr);
  ptr += i;
  fname = prepare_file_path(FGrp_LoData,"torture.pal");
  torture_palette = ptr;
  i = LbFileLoadAt(fname, ptr);
  ptr += i;

  // Load DAT/TAB sprites for doors
  k = 0;
  {
    fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.dat",k+1);
    i = LbFileLoadAt(fname, ptr);
    doors[k].data = (unsigned long)ptr;
    ptr += i;
    doors[k].data_end = ptr;

    fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.tab",k+1);
    i = LbFileLoadAt(fname, ptr);
    doors[k].sprites = (struct TbSprite *)ptr;
    ptr += i;
    doors[k].sprites_end =(struct TbSprite *) ptr;
  }
  ptr = &game.land_map_start;
  for (k=1; k < 8; k++)
  {
    fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.dat",k+1);
    i = LbFileLoadAt(fname, ptr);
    doors[k].data = (unsigned long)ptr;
    ptr += i;
    doors[k].data_end = ptr;
    fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.tab",k+1);
    i = LbFileLoadAt(fname, ptr);
    doors[k].sprites = (struct TbSprite *)ptr;
    ptr += i;
    doors[k].sprites_end = (struct TbSprite *)ptr;
  }
  ptr = poly_pool;
  k = 8;
  {
    fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.dat",k+1);
    i = LbFileLoadAt(fname, ptr);
    doors[k].data = (unsigned long)ptr;
    ptr += i;
    doors[k].data_end = ptr;
    fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.tab",k+1);
    i = LbFileLoadAt(fname, ptr);
    doors[k].sprites = (struct TbSprite *)ptr;
    ptr += i;
    doors[k].sprites_end = (struct TbSprite *)ptr;
  }

  if ( LbDataLoadAll(torture_load_files) )
    ERRORLOG("Unable to load torture load files");
  LbSpriteSetupAll(setup_torture_sprites);
  frontend_load_data_reset();
  memcpy(&frontend_palette, torture_palette, PALETTE_SIZE);
  torture_state = 0;
  torture_door_selected = -1;
  torture_end_sprite = -1;
  torture_sprite_direction = 0;
  memset(door_sound_state, 0, 0x48u);

  player = get_my_player();
  if (player->victory_state == VicS_WonLevel)
  {
    LbMouseChangeSpriteAndHotspot(&fronttor_sprites[1], 0, 0);
  } else
  {
    LbMouseChangeSpriteAndHotspot(0, 0, 0);
  }
  torture_left_button = 0;
}

void __stdcall enum_services_callback(struct TbNetworkCallbackData *netcdat, void *a2)
{
  if (net_number_of_services >= NET_SERVICES_COUNT)
  {
    ERRORLOG("Too many services in enumeration");
    return;
  }

  if (stricmp("SERIAL", netcdat->svc_name) == 0)
  {
    strcpy(net_service[net_number_of_services], gui_strings[874]);
    net_number_of_services++;
  }
  else if (stricmp("MODEM", netcdat->svc_name) == 0)
  {
    strcpy(net_service[net_number_of_services], gui_strings[875]);
    net_number_of_services++;
  }
  else if (stricmp("IPX", netcdat->svc_name) == 0)
  {
    strcpy(net_service[net_number_of_services], gui_strings[876]);
    net_number_of_services++;
  }
  else if (stricmp("TCP", netcdat->svc_name) == 0) {
    strcpy(net_service[net_number_of_services], "TCP/IP");
    net_number_of_services++;
  }
  else
  {
    ERRORLOG("Unrecognised Network Service");
  }
}

void __stdcall enum_players_callback(struct TbNetworkCallbackData *netcdat, void *a2)
{
  if (net_number_of_enum_players >= 4)
  {
    ERRORLOG("Too many players in enumeration");
    return;
  }
  strncpy(net_player[net_number_of_enum_players].name, netcdat->field_C, sizeof(struct TbNetworkPlayerName));
  net_number_of_enum_players++;
}

void __stdcall enum_sessions_callback(struct TbNetworkCallbackData *netcdat, void *ptr)
{
  if (net_number_of_sessions >= 32)
  {
    ERRORLOG("Too many sessions in enumeration");
    return;
  }
  if (net_service_index_selected == 0)
  {
    net_session[net_number_of_sessions] = (struct TbNetworkSessionNameEntry *)netcdat;
    net_number_of_sessions++;
  } else
  if (net_service_index_selected != 1)
  {
    net_session[net_number_of_sessions] = (struct TbNetworkSessionNameEntry *)netcdat;
    net_number_of_sessions++;
  } else
  if (net_number_of_sessions == 0)
  {
    net_session[net_number_of_sessions] = (struct TbNetworkSessionNameEntry *)netcdat;
    strcpy(&netcdat->svc_name[8],gui_strings[875]);
    net_number_of_sessions++;
  }
}

void net_load_config_file(void)
{
  TbFileHandle handle;
  char *fname;
  // Try to load the config file
  fname = prepare_file_path(FGrp_Save,keeper_netconf_file);
  handle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
  if (handle != -1)
  {
    if (LbFileRead(handle, &net_config_info, sizeof(net_config_info)) == sizeof(net_config_info))
    {
      LbFileClose(handle);
      return;
    }
    LbFileClose(handle);
  }
  // If can't load, then use default config
  LbMemoryCopy(&net_config_info, &default_net_config_info, sizeof(net_config_info));
  LbStringCopy(net_config_info.str_u2, gui_strings[404], 20);
}

void frontnet_service_setup(void)
{
  net_number_of_services = 0;
  LbMemorySet(net_service, 0, sizeof(net_service));
  // Create list of available services
  if (LbNetwork_EnumerateServices(enum_services_callback, NULL))
    ERRORLOG("LbNetwork_EnumerateServices() failed");
  // Create skirmish option if it should be enabled
  if ((game.system_flags & GSF_AllowOnePlayer) != 0)
  {
    LbStringCopy(net_service[net_number_of_services], gui_strings[870], 64);
    net_number_of_services++;
  }
  frontnet_init_level_descriptions();
  net_load_config_file();
}

void frontnet_session_setup(void)
{
  _DK_frontnet_session_setup();
}

void frontnet_start_setup(void)
{
  _DK_frontnet_start_setup();
}

void frontnet_modem_setup(void)
{
  _DK_frontnet_modem_setup();
}

void frontnet_serial_setup(void)
{
  _DK_frontnet_serial_setup();
}

void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (high_score_entry_input_active == -1));
}

void maintain_instance(struct GuiButton *gbtn)
{
  _DK_maintain_instance(gbtn);
}

void frontend_define_key_up_maintain(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_up_maintain(gbtn);
}

void frontend_define_key_down_maintain(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_down_maintain(gbtn);
}

void frontend_define_key_maintain(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_maintain(gbtn);
}

void gui_zoom_in(struct GuiButton *gbtn)
{
  _DK_gui_zoom_in(gbtn);
}

void gui_zoom_out(struct GuiButton *gbtn)
{
  _DK_gui_zoom_out(gbtn);
}

void gui_go_to_map(struct GuiButton *gbtn)
{
  zoom_to_map();
}

void gui_area_new_normal_button(struct GuiButton *gbtn)
{
  SYNCDBG(10,"Starting");
  _DK_gui_area_new_normal_button(gbtn);
  SYNCDBG(12,"Finished");
}

void gui_area_autopilot_button(struct GuiButton *gbtn)
{
  _DK_gui_area_autopilot_button(gbtn);
}

void gui_set_menu_mode(struct GuiButton *gbtn)
{
  set_menu_mode(gbtn->field_1B);
}

void gui_draw_tab(struct GuiButton *gbtn)
{
  if (gbtn->gbtype == Lb_CYCLEBTN)
    ERRORLOG("Cycle button cannot use this draw function!");
  if ((gbtn->field_1) || (gbtn->field_2))
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
  else
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29+1);
}

void turn_off_event_box_if_necessary(long plridx, char val)
{
  _DK_turn_off_event_box_if_necessary(plridx, val);
}

void frontstats_initialise(void)
{
  _DK_frontstats_initialise();
}

void gui_open_event(struct GuiButton *gbtn)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(my_player_number);
  unsigned int idx;
  unsigned int evnt_idx;
  SYNCDBG(5,"Starting");
  idx = (unsigned long)gbtn->field_33;
  if (idx < 121) //size of the field_13A7 array (I can't be completely sure of it)
    evnt_idx = dungeon->field_13A7[idx];
  else
    evnt_idx = 0;
  if (evnt_idx == dungeon->field_1173)
  {
    gui_close_objective(gbtn);
  } else
  if (evnt_idx != 0)
  {
    activate_event_box(evnt_idx);
  }
}

void gui_kill_event(struct GuiButton *gbtn)
{
  _DK_gui_kill_event(gbtn);
}

void gui_area_event_button(struct GuiButton *gbtn)
{
  struct Dungeon *dungeon;
  unsigned long i;
  if ((gbtn->field_0 & 0x08) != 0)
  {
    dungeon = get_players_num_dungeon(my_player_number);
    i = (unsigned long)gbtn->field_33;
    if ((gbtn->field_1) || (gbtn->field_2))
    {
      draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
    } else
    if (dungeon->field_13A7[i&0xFF] == dungeon->field_1173)
    {
      draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
    } else
    {
      draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29+1);
    }
  }
}

void gui_choose_room(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  long i;
  player = get_my_player();
  i = (long)gbtn->field_33;
  set_players_packet_action(player, PckA_SetPlyrState, PSt_BuildRoom, i, 0, 0);
  game.field_151801 = i;
  game.field_151805 = room_info[i].field_0;
  game.field_151809 = gbtn->tooltip_id;
}

void gui_go_to_next_room(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_room(gbtn);
}

void gui_over_room_button(struct GuiButton *gbtn)
{
  _DK_gui_over_room_button(gbtn);
}

void gui_area_room_button(struct GuiButton *gbtn)
{
  _DK_gui_area_room_button(gbtn);
}

void gui_area_new_null_button(struct GuiButton *gbtn)
{
  _DK_gui_area_new_null_button(gbtn);
}

void gui_area_new_no_anim_button(struct GuiButton *gbtn)
{
  SYNCDBG(10,"Starting");
  _DK_gui_area_new_no_anim_button(gbtn);
  SYNCDBG(12,"Finished");
}

void gui_remove_area_for_rooms(struct GuiButton *gbtn)
{
  _DK_gui_remove_area_for_rooms(gbtn);
}

void gui_area_big_room_button(struct GuiButton *gbtn)
{
  _DK_gui_area_big_room_button(gbtn);
}

TbBool set_players_packet_change_spell(struct PlayerInfo *player,int sptype)
{
  struct SpellData *pwrdata;
  long k;
  if (spell_is_stupid(game.chosen_spell_type))
    return false;
  pwrdata = get_power_data(sptype);
  k = pwrdata->field_4;
  if ((k == PSt_CallToArms) && (player->work_state == PSt_CallToArms))
  {
    set_players_packet_action(player, PckA_SpellCTADis, 0, 0, 0, 0);
  } else
  if ((k == PSt_SightOfEvil) && (player->work_state == PSt_SightOfEvil))
  {
    set_players_packet_action(player, PckA_SpellSOEDis, 0, 0, 0, 0);
  } else
  {
    set_players_packet_action(player, pwrdata->field_0, k, 0, 0, 0);
    play_non_3d_sample(pwrdata->field_11);
  }
  return true;
}

/*
 * Sets a new chosen spell.
 * Fills packet with the spell disable action.
 */
void gui_choose_spell(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  long i;
//  _DK_gui_choose_spell(gbtn); return;
  player = get_my_player();
  i = (long)gbtn->field_33;
  // Disable previous spell
  if (!set_players_packet_change_spell(player,i))
    WARNLOG("Inconsistency when switching spell %d to %d",(int)game.chosen_spell_type,i);
  set_chosen_spell(i,gbtn->tooltip_id);
}

void gui_go_to_next_spell(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_spell(gbtn);
}

void gui_area_spell_button(struct GuiButton *gbtn)
{
  _DK_gui_area_spell_button(gbtn);
}

void gui_choose_special_spell(struct GuiButton *gbtn)
{
  struct Dungeon *dungeon;
  struct SpellData *pwrdata;
  long idx;
  dungeon = get_players_num_dungeon(my_player_number);
  idx = (long)gbtn->field_33 % POWER_TYPES_COUNT;
  set_chosen_spell(idx, gbtn->tooltip_id);
  if (dungeon->field_AF9 >= game.magic_stats[idx].cost[0])
  {
    pwrdata = get_power_data(idx);
    play_non_3d_sample(pwrdata->field_11); // Play the spell speech
    switch (idx)
    {
    case 19:
        turn_on_menu(GMnu_ARMAGEDDON);
        break;
    case 9:
        turn_on_menu(GMnu_HOLD_AUDIENCE);
        break;
    }
  }
}

void frontend_draw_scroll_tab(struct GuiButton *gbtn, long scroll_offset, long first_elem, long last_elem)
{
  struct TbSprite *spr;
  long i,k,n;
  spr = &frontend_sprite[78];
  i = last_elem - first_elem;
  k = gbtn->height - spr->SHeight;
  if (i <= 1)
    n = (gbtn->height - spr->SHeight) ^ k;
  else
    n = scroll_offset * (k << 8) / (i - 1) >> 8;
  LbSpriteDraw(gbtn->scr_pos_x, n+gbtn->scr_pos_y, spr);
}

void gui_area_big_spell_button(struct GuiButton *gbtn)
{
  _DK_gui_area_big_spell_button(gbtn);
}

void gui_choose_trap(struct GuiButton *gbtn)
{
  _DK_gui_choose_trap(gbtn);
}

void gui_go_to_next_trap(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_trap(gbtn);
}

void gui_over_trap_button(struct GuiButton *gbtn)
{
  _DK_gui_over_trap_button(gbtn);
}

void gui_area_trap_button(struct GuiButton *gbtn)
{
  _DK_gui_area_trap_button(gbtn);
}

void gui_go_to_next_door(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_door(gbtn);
}

void gui_over_door_button(struct GuiButton *gbtn)
{
  _DK_gui_over_door_button(gbtn);
}

void gui_remove_area_for_traps(struct GuiButton *gbtn)
{
  _DK_gui_remove_area_for_traps(gbtn);
}

void gui_area_big_trap_button(struct GuiButton *gbtn)
{
  _DK_gui_area_big_trap_button(gbtn);
}

void gui_area_no_anim_button(struct GuiButton *gbtn)
{
  _DK_gui_area_no_anim_button(gbtn);
}

void gui_area_normal_button(struct GuiButton *gbtn)
{
  _DK_gui_area_normal_button(gbtn);
}

void gui_set_tend_to(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  set_players_packet_action(player, PckA_ToggleTendency, gbtn->field_1B, 0, 0, 0);
}

void gui_area_flash_cycle_button(struct GuiButton *gbtn)
{
  _DK_gui_area_flash_cycle_button(gbtn);
}

void gui_set_query(struct GuiButton *gbtn)
{
  _DK_gui_set_query(gbtn);
}

void gui_area_payday_button(struct GuiButton *gbtn)
{
  _DK_gui_area_payday_button(gbtn);
}

void gui_area_research_bar(struct GuiButton *gbtn)
{
  _DK_gui_area_research_bar(gbtn);
}

void gui_area_workshop_bar(struct GuiButton *gbtn)
{
  _DK_gui_area_workshop_bar(gbtn);
}

void gui_area_player_creature_info(struct GuiButton *gbtn)
{
  _DK_gui_area_player_creature_info(gbtn);
}

void gui_area_player_room_info(struct GuiButton *gbtn)
{
  _DK_gui_area_player_room_info(gbtn);
}

void gui_toggle_ally(struct GuiButton *gbtn)
{
  _DK_gui_toggle_ally(gbtn);
}

void gui_quit_game(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  set_players_packet_action(player, 1, 0, 0, 0, 0);
}

void gui_area_ally(struct GuiButton *gbtn)
{
  _DK_gui_area_ally(gbtn);
}

void gui_save_game(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  long slot_num;
  player = get_my_player();
  if (strcasecmp((char *)gbtn->field_33, gui_strings[342]) != 0) // "UNUSED"
  {
      slot_num = gbtn->field_1B%SAVE_SLOTS_COUNT;
      fill_game_catalogue_entry(slot_num,(char *)gbtn->field_33);
      if (save_game(slot_num))
      {
        output_message(103, 0, 1);
      } else
      {
        ERRORLOG("Error in save!");
        create_error_box(536);
      }
  }
  set_players_packet_action(player, PckA_TogglePause, 0, 0, 0, 0);
}

void gui_video_shadows(struct GuiButton *gbtn)
{
  _DK_gui_video_shadows(gbtn);
}

void gui_video_view_distance_level(struct GuiButton *gbtn)
{
  _DK_gui_video_view_distance_level(gbtn);
}

void gui_video_rotate_mode(struct GuiButton *gbtn)
{
  _DK_gui_video_rotate_mode(gbtn);
}

void gui_video_cluedo_mode(struct GuiButton *gbtn)
{
  _DK_gui_video_cluedo_mode(gbtn);
}

void gui_video_gamma_correction(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  video_gamma_correction = (video_gamma_correction + 1) % GAMMA_LEVELS_COUNT;
  set_players_packet_action(player, PckA_SetGammaLevel, video_gamma_correction, 0, 0, 0);
}

void gui_set_sound_volume(struct GuiButton *gbtn)
{
  _DK_gui_set_sound_volume(gbtn);
}

void gui_set_music_volume(struct GuiButton *gbtn)
{
  _DK_gui_set_music_volume(gbtn);
}

void gui_area_slider(struct GuiButton *gbtn)
{
  _DK_gui_area_slider(gbtn);
}

void gui_area_smiley_anger_button(struct GuiButton *gbtn)
{
  _DK_gui_area_smiley_anger_button(gbtn);
}

void gui_area_experience_button(struct GuiButton *gbtn)
{
  _DK_gui_area_experience_button(gbtn);
}

void gui_area_instance_button(struct GuiButton *gbtn)
{
  _DK_gui_area_instance_button(gbtn);
}

void gui_area_stat_button(struct GuiButton *gbtn)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  struct Dungeon *dungeon;
  struct PlayerInfo *player;
  struct Thing *thing;
  char *text;
  long i;
  draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 459);
  player = get_my_player();
  thing = thing_get(player->field_2F);
  if (thing == NULL)
    return;
  if (thing->class_id == TCls_Creature)
  {
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    switch ((long)gbtn->field_33)
    {
    case 0: // kills
        i = cctrl->field_C2;
        text = buf_sprintf("%ld", i);
        break;
    case 1: // strength
        i = compute_creature_max_strength(crstat->strength,cctrl->explevel);
        text = buf_sprintf("%ld", i);
        break;
    case 2: // gold held
        i = thing->long_13;
        text = buf_sprintf("%ld", i);
        break;
    case 3: // payday wage
        dungeon = get_players_num_dungeon(thing->owner);
        if (dungeon->field_1420[thing->model])
          i = compute_creature_max_pay(crstat->pay,cctrl->explevel)/2;
        else
          i = compute_creature_max_pay(crstat->pay,cctrl->explevel);
        text = buf_sprintf("%ld", i);
        break;
    case 4: // armour
        i = compute_creature_max_armour(crstat->armour,cctrl->explevel);
        text = buf_sprintf("%ld", i);
        break;
    case 5: // defence
        i = compute_creature_max_defence(crstat->defence,cctrl->explevel);
        text = buf_sprintf("%ld", i);
        break;
    case 6: // time in dungeon
        i = (game.play_gameturn-thing->field_9) / 2000 + cctrl->field_286;
        if (i >= 99)
          i = 99;
        text = buf_sprintf("%ld", i);
        break;
    case 7: // dexterity
        i = compute_creature_max_dexterity(crstat->dexterity,cctrl->explevel);
        text = buf_sprintf("%ld", i);
        break;
    case 8: // luck
        i = compute_creature_max_luck(crstat->luck,cctrl->explevel);
        text = buf_sprintf("%ld", i);
        break;
    case 9: // blood type
        i = cctrl->field_287;
        text = buf_sprintf("%s", blood_types[i%BLOOD_TYPES_COUNT]);
        break;
    default:
        return;
    }
    draw_gui_panel_sprite_left(gbtn->scr_pos_x-6, gbtn->scr_pos_y-12, gbtn->field_29);
    draw_button_string(gbtn, text);
  }
}

#if (BFDEBUG_LEVEL > 0)
// Code for font testing screen (debug version only)
TbBool fronttestfont_draw(void)
{
  const struct TbSprite *spr;
  unsigned long i,k;
  long w,h;
  long x,y;
  SYNCDBG(9,"Starting");
  for (y=0; y < lbDisplay.GraphicsScreenHeight; y++)
    for (x=0; x < lbDisplay.GraphicsScreenWidth; x++)
    {
        lbDisplay.WScreen[y*lbDisplay.GraphicsScreenWidth+x] = 0;
    }
  LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenHeight/pixel_size, MyScreenWidth/pixel_size);
  // Drawing
  w = 32;
  h = 48;
  for (i=31; i < num_chars_in_font+31; i++)
  {
    k = (i-31);
    SYNCDBG(9,"Drawing char %d",i);
    x = (k%32)*w + 2;
    y = (k/32)*h + 2;
    if (lbFontPtr != NULL)
      spr = LbFontCharSprite(lbFontPtr,i);
    else
      spr = NULL;
    if (spr != NULL)
    {
      LbDrawBox(x, y, spr->SWidth+2, spr->SHeight+2, 255);
      LbSpriteDraw(x+1, y+1, spr);
    }
//    !!!!!!!!!!!!!!!!
  }
  // Displaying the new frame
  return true;
}

TbBool fronttestfont_input(void)
{
  const unsigned int keys[] = {KC_Z,KC_1,KC_2,KC_3,KC_4,KC_5,KC_6,KC_7,KC_8,KC_9,KC_0};
  int i;
  if (lbKeyOn[KC_Q])
  {
    lbKeyOn[KC_Q] = 0;
    frontend_set_state(FeSt_MAIN_MENU);
    return true;
  }
  for (i=0; i < sizeof(keys)/sizeof(keys[0]); i++)
  {
    if (lbKeyOn[keys[i]])
    {
      lbKeyOn[keys[i]] = 0;
      num_chars_in_font = testfont_end[i]-testfont[i];
      SYNCDBG(9,"Characters in font %d: %d",i,num_chars_in_font);
      if (i < 4)
        LbPaletteSet(frontend_palette);//testfont_palette[0]
      else
        LbPaletteSet(testfont_palette[1]);
      LbTextSetFont(testfont[i]);
      return true;
    }
  }
  // Handle GUI inputs
  return get_gui_inputs(0);
}
#endif


void frontend_define_key_up(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_up(gbtn);
}

void frontend_define_key_down(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_down(gbtn);
}

void frontend_define_key(struct GuiButton *gbtn)
{
  _DK_frontend_define_key(gbtn);
}

void frontend_draw_define_key_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontend_draw_define_key_scroll_tab(gbtn);
}

void frontend_draw_define_key(struct GuiButton *gbtn)
{
  _DK_frontend_draw_define_key(gbtn);
}

void frontend_draw_icon(struct GuiButton *gbtn)
{
  _DK_frontend_draw_icon(gbtn);
}

void frontend_draw_slider(struct GuiButton *gbtn)
{
  _DK_frontend_draw_slider(gbtn);
}

void frontend_set_mouse_sensitivity(struct GuiButton *gbtn)
{
  _DK_frontend_set_mouse_sensitivity(gbtn);
}

void frontend_draw_small_slider(struct GuiButton *gbtn)
{
  _DK_frontend_draw_small_slider(gbtn);
}

void frontend_invert_mouse(struct GuiButton *gbtn)
{
  _DK_frontend_invert_mouse(gbtn);
}

void frontend_draw_invert_mouse(struct GuiButton *gbtn)
{
  _DK_frontend_draw_invert_mouse(gbtn);
}

void frontstats_draw_main_stats(struct GuiButton *gbtn)
{
  _DK_frontstats_draw_main_stats(gbtn);
}

void frontstats_draw_scrolling_stats(struct GuiButton *gbtn)
{
  _DK_frontstats_draw_scrolling_stats(gbtn);
}

/*
 * Loads next menu state after leaving frontstats.
 */
void frontstats_leave(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  LevelNumber lvnum;
  if ((game.system_flags & GSF_NetworkActive) != 0)
  {
    if ( setup_old_network_service() )
    {
      frontend_set_state(FeSt_NET_SESSION);
      fe_high_score_table_from_main_menu = false;
    } else
    {
      frontend_set_state(FeSt_MAIN_MENU);
    }
  } else
  {
    player = get_my_player();
    lvnum = get_loaded_level_number();
    if (player->victory_state == VicS_WonLevel)
    {
      frontend_set_state(FeSt_HIGH_SCORES);
      fe_high_score_table_from_main_menu = false;
    } else
    if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum) || is_extra_level(lvnum))
    {
      frontend_set_state(FeSt_LAND_VIEW);
    } else
    if (is_freeplay_level(lvnum))
    {
      frontend_set_state(FeSt_LEVEL_SELECT);
    } else
    {
      frontend_set_state(FeSt_MAIN_MENU);
    }
  }
}

void frontend_draw_vlarge_menu_button(struct GuiButton *gbtn)
{
  unsigned int fbinfo_idx;
  const char *text;
  int i;
  //_DK_frontend_draw_vlarge_menu_button(gbtn);
  fbinfo_idx = (unsigned long)gbtn->field_33;
  i = frontend_button_info[fbinfo_idx%FRONTEND_BUTTON_INFO_COUNT].capstr_idx;
  if (i > 0)
    text = gui_strings[i];
  else
    text = NULL;
  frontend_draw_button(gbtn, 2, text, 0x100);
}

void draw_high_score_entry(int idx, long pos_x, long pos_y, int col1_width, int col2_width, int col3_width, int col4_width)
{
    struct HighScore *hscore;
    int i;
    if ((idx >= campaign.hiscore_count) || (campaign.hiscore_table == NULL))
      return;
    hscore = &campaign.hiscore_table[idx];
    lbDisplay.DrawFlags = 0x80;
    i = pos_x + col1_width;
    LbTextNumberDraw(i, pos_y, idx+1, Fnt_RightJustify);
    i += col2_width;
    LbTextNumberDraw(i, pos_y, hscore->score, Fnt_RightJustify);
    i += col3_width;
    LbTextNumberDraw(i, pos_y, hscore->lvnum, Fnt_RightJustify);
    i += col4_width;
    if (idx == high_score_entry_input_active)
    {
      i += LbTextStringDraw(i, pos_y, high_score_entry, Fnt_LeftJustify);
      // Blinking cursor
      if ((LbTimerClock() & 0x0100) != 0)
      {
        LbTextStringDraw(i, pos_y, "_", Fnt_LeftJustify);
      }
    } else
    {
      LbTextStringDraw(i, pos_y, hscore->name, Fnt_LeftJustify);
    }
}

void frontend_draw_high_score_table(struct GuiButton *gbtn)
{
  struct TbSprite *spr;
  struct TbSprite *swpspr;
  long pos_x,pos_y;
  long col1_width,col2_width,col3_width,col4_width;
  long i,k;
//  _DK_frontend_draw_high_score_table(gbtn); return;
  // Draw the high scores area - top
  pos_x = gbtn->scr_pos_x;
  pos_y = gbtn->scr_pos_y;
  spr = &frontend_sprite[25];
  swpspr = spr;
  for (i=6; i > 0; i--)
  {
    LbSpriteDraw(pos_x, pos_y, swpspr);
    pos_x += swpspr->SWidth;
    swpspr++;
  }
  pos_y += spr->SHeight;
  // Draw the high scores area - filling
  k = 12;
  while (k > 0)
  {
    if (k < 3)
      i = 33;
    else
      i = 40;
    spr = &frontend_sprite[i];
    pos_x = gbtn->scr_pos_x;
    swpspr = spr;
    for (i=6; i > 0; i--)
    {
      LbSpriteDraw(pos_x, pos_y, swpspr);
      pos_x += swpspr->SWidth;
      swpspr++;
    }
    pos_y += spr->SHeight;
    if (k < 3)
      k--;
    else
      k -= 3;
  }
  // Draw the high scores area - bottom
  pos_x = gbtn->scr_pos_x;
  spr = &frontend_sprite[47];
  swpspr = spr;
  for (i=6; i > 0; i--)
  {
    LbSpriteDraw(pos_x, pos_y, swpspr);
    pos_x += swpspr->SWidth;
    swpspr++;
  }
  LbTextSetFont(frontend_font[1]);
  lbDisplay.DrawFlags = 0;
  spr = &frontend_sprite[33];
  pos_x = gbtn->scr_pos_x + spr->SWidth;
  spr = &frontend_sprite[25];
  pos_y = spr->SHeight + gbtn->scr_pos_y + 3;
  col1_width = LbTextStringWidth("99");
  col2_width = LbTextStringWidth(" 99999");
  col3_width = LbTextStringWidth(" 999");
  col4_width = LbTextCharWidth('-');
  for (k=0; k < VISIBLE_HIGH_SCORES_COUNT-1; k++)
  {
    draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width);
    pos_y += LbTextLineHeight();
  }
  if (high_score_entry_input_active > k)
    draw_high_score_entry(high_score_entry_input_active, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width);
  else
    draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width);
}

void frontend_quit_high_score_table(struct GuiButton *gbtn)
{
  LevelNumber lvnum;
  lvnum = get_loaded_level_number();
  if (fe_high_score_table_from_main_menu)
  {
    frontend_set_state(FeSt_MAIN_MENU);
  } else
  if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum) || is_extra_level(lvnum))
  {
    frontend_set_state(FeSt_LAND_VIEW);
  } else
  if (is_multiplayer_level(lvnum))
  {
    frontend_set_state(FeSt_MAIN_MENU);
  } else
  if (is_freeplay_level(lvnum))
  {
    frontend_set_state(FeSt_LEVEL_SELECT);
  } else
  {
    frontend_set_state(FeSt_MAIN_MENU);
  }
}

void frontnet_start_input(void)
{
  _DK_frontnet_start_input();
}

TbBool frontend_high_score_table_input(void)
{
  struct HighScore *hscore;
  char chr;
  long i;
  if (high_score_entry_input_active >= campaign.hiscore_count)
    return false;
  if (lbInkey == KC_BACK)
  {
    if (high_score_entry_index > 0)
    {
      i = high_score_entry_index-1;
      high_score_entry[i] = '\0';
      high_score_entry_index = i;
      lbInkey = KC_UNASSIGNED;
      return true;
    }
  }
  if (lbInkey == KC_RETURN)
  {
    hscore = &campaign.hiscore_table[high_score_entry_input_active];
    strncpy(hscore->name, high_score_entry, HISCORE_NAME_LENGTH);
    high_score_entry_input_active = -1;
    save_high_score_table();
    lbInkey = KC_UNASSIGNED;
    return true;
  }
  if (high_score_entry_index < HISCORE_NAME_LENGTH)
  {
    chr = key_to_ascii(lbInkey, key_modifiers);
    if (chr != 0)
    {
      LbTextSetFont(frontend_font[1]);
      i = LbTextCharWidth(chr);
      if ((i > 0) && (i+LbTextStringWidth(high_score_entry) < 308))
      {
        high_score_entry[high_score_entry_index] = chr;
        i = high_score_entry_index+1;
        high_score_entry[i] = 0;
        high_score_entry_index = i;
        lbInkey = KC_UNASSIGNED;
        return true;
      }
    }
  }
  return false;
}

void pick_up_next_wanderer(struct GuiButton *gbtn)
{
  _DK_pick_up_next_wanderer(gbtn);
}

void gui_go_to_next_wanderer(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_wanderer(gbtn);
}

void pick_up_next_worker(struct GuiButton *gbtn)
{
  _DK_pick_up_next_worker(gbtn);
}

void gui_go_to_next_worker(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_worker(gbtn);
}

void pick_up_next_fighter(struct GuiButton *gbtn)
{
  _DK_pick_up_next_fighter(gbtn);
}

void gui_go_to_next_fighter(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_fighter(gbtn);
}

void gui_scroll_activity_up(struct GuiButton *gbtn)
{
  _DK_gui_scroll_activity_up(gbtn);
}

void gui_scroll_activity_down(struct GuiButton *gbtn)
{
  _DK_gui_scroll_activity_down(gbtn);
}

void pick_up_next_creature(struct GuiButton *gbtn)
{
  _DK_pick_up_next_creature(gbtn);
}

void gui_go_to_next_creature(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_creature(gbtn);
}

void pick_up_creature_doing_activity(struct GuiButton *gbtn)
{
    long i,job_idx,kind;
    unsigned char pick_flags;
    SYNCDBG(8,"Starting");
    //_DK_pick_up_creature_doing_activity(gbtn); return;
    i = gbtn->field_1B;
    // Get index from pointer
    job_idx = ((long *)gbtn->field_33 - &activity_list[0]);
    if (i > 0)
        kind = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    else
        kind = get_players_special_digger_breed(my_player_number);
    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(kind, (job_idx & 0x03), my_player_number, pick_flags);
}

void gui_go_to_next_creature_activity(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_creature_activity(gbtn);
}

void turn_off_roaming_menus(void)
{
  _DK_turn_off_roaming_menus();
}

void gui_area_anger_button(struct GuiButton *gbtn)
{
    long i,job_idx,kind;
    SYNCDBG(10,"Starting");
    i = gbtn->field_1B;
    // Get index from pointer
    job_idx = ((long *)gbtn->field_33 - &activity_list[0]);
    if (i > 0)
        kind = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    else
        kind = 23;
    // Now draw the button
    struct Dungeon *dungeon;
    int spridx;
    long cr_total;
    cr_total = 0;
    if ((kind > 0) && (kind < CREATURE_TYPES_COUNT) && (gbtn->field_0 & 0x08))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        spridx = gbtn->field_29;
        if (gbtn->field_33 != NULL)
        {
          cr_total = *(long *)gbtn->field_33;
          if (cr_total > 0)
          {
            i = dungeon->field_4E4[kind][(job_idx & 0x03)];
            if (i > cr_total)
            {
              WARNDBG(7,"Creature %d stats inconsistency; total=%d, doing activity%d=%d",kind,cr_total,(job_idx & 0x03),i);
              i = cr_total;
            }
            if (i < 0)
            {
              i = 0;
            }
            spridx += 14 * i / cr_total;
          }
        }
        if ((gbtn->field_1) || (gbtn->field_2))
        {
          draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y-2, spridx, 3072);
        } else
        {
          draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y-2, spridx);
        }
        if (gbtn->field_33 != NULL)
        {
          sprintf(gui_textbuf, "%ld", cr_total);
          if ((cr_total > 0) && (dungeon->job_breeds_count[kind][(job_idx & 0x03)] ))
          {
              for (i=0; gui_textbuf[i] != '\0'; i++)
                  gui_textbuf[i] -= 120;
          }
          draw_button_string(gbtn, gui_textbuf);
        }
    }
    SYNCDBG(12,"Finished");
}

void gui_area_text(struct GuiButton *gbtn)
{
  _DK_gui_area_text(gbtn);
}

void frontend_init_options_menu(struct GuiMenu *gmnu)
{
  _DK_frontend_init_options_menu(gmnu);
}

void frontend_set_player_number(long plr_num)
{
  struct PlayerInfo *player;
  my_player_number = plr_num;
  player = get_my_player();
  player->id_number = plr_num;
//  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight); - maybe better?
  setup_engine_window(0, 0, 640, 480);

}

void frontnet_draw_scroll_box_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_scroll_box_tab(gbtn);
}

void frontnet_draw_scroll_box(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_scroll_box(gbtn);
}

void frontnet_draw_slider_button(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_slider_button(gbtn);
}

void frontnet_draw_services_scroll_tab(struct GuiButton *gbtn)
{
  frontend_draw_scroll_tab(gbtn, net_service_scroll_offset, 0, net_number_of_services);
}

void frontend_draw_text(struct GuiButton *gbtn)
{
  struct FrontEndButtonData *febtn_data;
  long i;
  i = (long)gbtn->field_33;
  lbDisplay.DrawFlags = 0x20;
  febtn_data = &frontend_button_info[i%FRONTEND_BUTTON_INFO_COUNT];
  if ((gbtn->field_0 & 0x08) == 0)
    LbTextSetFont(frontend_font[3]);
  else
  if ((i != 0) && (frontend_mouse_over_button == i))
    LbTextSetFont(frontend_font[2]);
  else
    LbTextSetFont(frontend_font[febtn_data->field_2]);
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
  LbTextDraw(0, 0, gui_strings[febtn_data->capstr_idx%STRINGS_MAX]);
}

void frontend_change_state(struct GuiButton *gbtn)
{
  frontend_set_state(gbtn->field_1B);
}

void frontend_over_button(struct GuiButton *gbtn)
{
  _DK_frontend_over_button(gbtn);
}

void frontnet_session_set_player_name(struct GuiButton *gbtn)
{
  _DK_frontnet_session_set_player_name(gbtn);
}

void frontnet_draw_text_bar(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_text_bar(gbtn);
}

void frontend_draw_enter_text(struct GuiButton *gbtn)
{
  _DK_frontend_draw_enter_text(gbtn);
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

	long sessionIndex; // esi@1
	long fontIndex; // edx@2
	TbSprite * font; // edx@5
	int height; // ecx@6

	sessionIndex = net_session_scroll_offset + (long) gbtn->field_33 - 45;
	if ( sessionIndex < net_number_of_sessions ) {
		fontIndex = frontend_button_info[(long) gbtn->field_33].field_2;
		if ( gbtn->field_33 ) {
			if ( frontend_mouse_over_button == (long) gbtn->field_33 )
				fontIndex = 2;
		}
		font = frontend_font[fontIndex];
		lbDisplay.DrawFlags = 0;
		lbFontPtr = font;
		if ( font )
			height = *(char *)(font + 11);
		else
			height = 0;
		LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, height);
		LbTextDraw(0, 0, net_session[sessionIndex]->text);
	}
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

void display_attempting_to_join_message(void)
{
  if (LbScreenLock() == Lb_SUCCESS)
  {
    draw_text_box(gui_strings[868]); // "Attempting To Join"
    LbScreenUnlock();
  }
  LbScreenSwap();
}

void frontnet_session_join(struct GuiButton *gbtn)
{
  unsigned long plyr_num;
  void *conn_options;
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
    display_attempting_to_join_message();
    conn_options = NULL;
    break;
  }
  if ( LbNetwork_Join(net_session[net_session_index_active], net_player_name, &plyr_num, conn_options) )
  {
    if (net_service_index_selected == 1)
      process_network_error(modem_dev.field_A8);
    else
      process_network_error(-802);
    return;
  }
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

void frontend_draw_small_menu_button(struct GuiButton *gbtn)
{
  _DK_frontend_draw_small_menu_button(gbtn);
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

void frontend_toggle_computer_players(struct GuiButton *gbtn)
{
  _DK_frontend_toggle_computer_players(gbtn);
}

void frontend_draw_computer_players(struct GuiButton *gbtn)
{
  _DK_frontend_draw_computer_players(gbtn);
}

void frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_messages_scroll_tab(gbtn);
}

void frontnet_draw_current_message(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_current_message(gbtn);
}

void frontnet_draw_messages(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_messages(gbtn);
}

void set_packet_start(struct GuiButton *gbtn)
{
  _DK_set_packet_start(gbtn);
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

void gui_load_game(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player=get_my_player();
  if (!load_game(gbtn->field_1B))
  {
      ERRORLOG("Error in load!");
      quit_game = 1;
      return;
  }
  set_players_packet_action(player, 22, 0, 0, 0, 0);
}

void draw_scrolling_button_string(struct GuiButton *gbtn, const char *text)
{
  struct TextScrollWindow *scrollwnd;
  unsigned short flg_mem;
  long text_height,area_height;
  flg_mem = lbDisplay.DrawFlags;
  lbDisplay.DrawFlags &= 0xFFBF;
  lbDisplay.DrawFlags |= 0x0100;
  LbTextSetWindow(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
        gbtn->width/pixel_size, gbtn->height/pixel_size);
  scrollwnd = (struct TextScrollWindow *)gbtn->field_33;
  if (scrollwnd == NULL)
  {
    ERRORLOG("Cannot have a TEXT_SCROLLING box type without a pointer to a TextScrollWindow");
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenHeight/pixel_size, MyScreenWidth/pixel_size);
    return;
  }
  area_height = gbtn->height;
  scrollwnd->window_height = area_height;
  text_height = scrollwnd->text_height;
  if (text_height == 0)
  {
    text_height = text_string_height(text);
    SYNCDBG(18,"Computed message height %ld for \"%s\"",text_height,text);
    scrollwnd->text_height = text_height;
  }
  SYNCDBG(18,"Message h=%ld Area h=%d",text_height,area_height);
  // If the text is smaller that the area we have for it - just place it at center
  if (text_height <= area_height)
  {
    scrollwnd->start_y = (area_height - text_height) / 2;
  } else
  // Otherwise - we must take scrollbars into account
  {
    // Maintain scrolling actions
    switch ( scrollwnd->action )
    {
    case 1:
      scrollwnd->start_y += 8;
      break;
    case 2:
      scrollwnd->start_y -= 8;
      break;
    case 3:
      scrollwnd->start_y += area_height;
      break;
    case 4:
    case 5:
      scrollwnd->start_y -= area_height;
      break;
    }
    if (scrollwnd->action == 5)
    {
      if (scrollwnd->start_y < -text_height)
      {
        scrollwnd->start_y = 0;
      }
    } else
    if (scrollwnd->action != 0)
    {
      if (scrollwnd->start_y < gbtn->height-text_height)
      {
        scrollwnd->start_y = gbtn->height-text_height;
      } else
      if (scrollwnd->start_y > 0)
      {
        scrollwnd->start_y = 0;
      }
    }
    scrollwnd->action = 0;
  }
  // Finally, draw the text
  LbTextDraw(0/pixel_size, scrollwnd->start_y/pixel_size, text);
  // And restore default drawing options
  LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenHeight/pixel_size, MyScreenWidth/pixel_size);
  lbDisplay.DrawFlags = flg_mem;
}

void gui_area_scroll_window(struct GuiButton *gbtn)
{
  struct TextScrollWindow *scrollwnd;
  char *text;
  //_DK_gui_area_scroll_window(gbtn); return;
  if ((gbtn->field_0 & 8) == 0)
    return;
  scrollwnd = (struct TextScrollWindow *)gbtn->field_33;
  if (scrollwnd == NULL)
  {
    ERRORLOG("Button doesn't point to a TextScrollWindow data item");
    return;
  }
  text = buf_sprintf("%s", scrollwnd->text);
  draw_scrolling_button_string(gbtn, text);
}

void gui_go_to_event(struct GuiButton *gbtn)
{
  _DK_gui_go_to_event(gbtn);
}

void gui_close_objective(struct GuiButton *gbtn)
{
  _DK_gui_close_objective(gbtn);
}

void gui_scroll_text_up(struct GuiButton *gbtn)
{
  _DK_gui_scroll_text_up(gbtn);
}

void gui_scroll_text_down(struct GuiButton *gbtn)
{
  _DK_gui_scroll_text_down(gbtn);
}

void choose_hold_audience(struct GuiButton *gbtn)
{
  _DK_choose_hold_audience(gbtn);
}

void choose_armageddon(struct GuiButton *gbtn)
{
  _DK_choose_armageddon(gbtn);
}

void frontend_load_game_up(struct GuiButton *gbtn)
{
  if (load_game_scroll_offset > 0)
    load_game_scroll_offset--;
}

void frontend_load_game_down(struct GuiButton *gbtn)
{
  if (load_game_scroll_offset < number_of_saved_games-frontend_load_menu_items_visible+1)
    load_game_scroll_offset++;
}

void frontend_draw_games_scroll_tab(struct GuiButton *gbtn)
{
  frontend_draw_scroll_tab(gbtn, load_game_scroll_offset, frontend_load_menu_items_visible-2, number_of_saved_games);
}

void frontend_draw_levels_scroll_tab(struct GuiButton *gbtn)
{
  frontend_draw_scroll_tab(gbtn, select_level_scroll_offset, frontend_select_level_items_visible-2, number_of_freeplay_levels);
}

int frontend_load_game_button_to_index(struct GuiButton *gbtn)
{
  struct CatalogueEntry *centry;
  long gbidx;
  int i,k;
  gbidx = (unsigned long)gbtn->field_33;
  k = -1;
  for (i=gbidx+load_game_scroll_offset-45; i >= 0; i--)
  {
    do
    {
      k++;
      if (k >= SAVE_SLOTS_COUNT)
        return -1;
      centry = &save_game_catalogue[k];
    } while ((centry->flags & CEF_InUse) == 0);
  }
  return k;
}

void frontend_load_game(struct GuiButton *gbtn)
{
  int i;
  i = frontend_load_game_button_to_index(gbtn);
  if (i < 0)
    return;
  game.numfield_15 = i;
  if (is_save_game_loadable(i))
  {
    frontend_set_state(FeSt_LOAD_GAME);
  } else
  {
    save_catalogue_slot_disable(i);
    if (!initialise_load_game_slots())
      frontend_set_state(FeSt_MAIN_MENU);
  }
}

void frontend_draw_load_game_button(struct GuiButton *gbtn)
{
  int nfont;
  long gbidx;
  int i,h;
  gbidx = (unsigned long)gbtn->field_33;
  nfont = frontend_button_info[gbidx%FRONTEND_BUTTON_INFO_COUNT].field_2;
  if ((gbidx != 0) && (frontend_mouse_over_button == gbidx))
      nfont = 2;
  lbDisplay.DrawFlags = 0x20;
  LbTextSetFont(frontend_font[nfont]);
  h = LbTextLineHeight();
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, h);
  i = frontend_load_game_button_to_index(gbtn);
  if (i < 0)
    return;
  LbTextDraw(0, 0, save_game_catalogue[i].textname);
}

/*
 * Changes state based on a parameter inside GuiButton.
 * But first, loads the default campaign if no campaign is loaded yet.
 */
void frontend_ldcampaign_change_state(struct GuiButton *gbtn)
{
  if (!is_campaign_loaded())
  {
    if (!change_campaign(""))
      return;
  }
  frontend_change_state(gbtn);
}

/*
 * Changes state based on a parameter inside GuiButton.
 * But first, loads the default campaign if no campaign is loaded,
 * or the loaded one has no MP maps.
 */
void frontend_netservice_change_state(struct GuiButton *gbtn)
{
  TbBool set_cmpg;
  set_cmpg = false;
  if (!is_campaign_loaded())
  {
    set_cmpg = true;
  } else
  if (campaign.multi_levels_count < 1)
  {
    set_cmpg = true;
  }
  if (set_cmpg)
  {
    if (!change_campaign(""))
      return;
  }
  frontend_change_state(gbtn);
}

TbBool frontend_start_new_campaign(const char *cmpgn_fname)
{
  struct PlayerInfo *player;
  int i;
  SYNCDBG(7,"Starting");
  if (!change_campaign(cmpgn_fname))
    return false;
  set_continue_level_number(first_singleplayer_level());
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    player->field_6 &= 0xFD;
  }
  player = get_my_player();
  clear_transfered_creature();
  calculate_moon_phase(false,false);
  hide_all_bonus_levels(player);
  update_extra_levels_visibility();
  return true;
}

void frontend_start_new_game(struct GuiButton *gbtn)
{
  char *cmpgn_fname;
  SYNCDBG(6,"Clicked");
  // Check if we can just start the game without campaign selection screen
  if (campaigns_list.items_num < 1)
    cmpgn_fname = "";
  else
  if (campaigns_list.items_num == 1)
    cmpgn_fname = campaigns_list.items[0].fname;
  else
    cmpgn_fname = NULL;
  if (cmpgn_fname != NULL)
  { // If there's only one campaign, then start it
    if (!frontend_start_new_campaign(cmpgn_fname))
    {
      ERRORLOG("Unable to start new campaign");
      return;
    }
    frontend_set_state(FeSt_LAND_VIEW);
  } else
  { // If there's more campaigns, go to selection screen
    frontend_set_state(FeSt_CAMPAIGN_SELECT);
  }
}

/*
 * Writes the continue game file.
 * If allow_lvnum_grow is true and my_player has won the singleplayer level,
 * then next level is written into continue file. This should be the case
 * if complete_level() wasn't called yet.
 */
short frontend_save_continue_game(short allow_lvnum_grow)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  unsigned short victory_state;
  short flg_mem;
  LevelNumber lvnum;
  SYNCDBG(6,"Starting");
  player = get_my_player();
  dungeon = get_players_dungeon(player);
  // Save some of the data from clearing
  victory_state = player->victory_state;
  memcpy(scratch, &dungeon->lvstats, sizeof(struct LevelStats));
  flg_mem = ((player->field_3 & 0x10) != 0);
  // clear all data
  clear_game_for_save();
  // Restore saved data
  player->victory_state = victory_state;
  memcpy(&dungeon->lvstats, scratch, sizeof(struct LevelStats));
  set_flag_byte(&player->field_3,0x10,flg_mem);
  // Only save continue if level was won, and not in packet mode
  if (((game.system_flags & GSF_NetworkActive) != 0)
   || ((game.numfield_C & 0x02) != 0)
   || (game.packet_load_enable))
    return false;
  lvnum = get_continue_level_number();
  if ((allow_lvnum_grow) && (player->victory_state == VicS_WonLevel))
  {
    if (is_singleplayer_like_level(lvnum))
      lvnum = next_singleplayer_level(lvnum);
    if (lvnum == LEVELNUMBER_ERROR)
      lvnum = get_continue_level_number();
  }
  return save_continue_game(lvnum);
}

void frontend_load_continue_game(struct GuiButton *gbtn)
{
  if (!load_continue_game())
  {
    continue_game_option_available = 0;
    return;
  }
  frontend_set_state(FeSt_LAND_VIEW);
}

TbBool fronttorture_draw(void)
{
	SYNCDBG(10, "Entering");

	TbScreenMode * mode = getActiveScreenMode();
  struct TbSprite *spr;
  const int img_width = 640;
  const int img_height = 480;
  int w,h,m,i;
  int spx,spy;

  w=0;
  h=0;
  m=1;
  {
    w+=img_width;
    h+=img_height;
  }
  // Starting point coords
  //TODO: temporarely set to top left corner because input function is not rewritten
  spx = 0;//(mdinfo->Width-m*img_width)>>1;
  spy = 0;//(mdinfo->Height-m*img_height)>>1;
  copy_raw8_image_buffer(lbDisplay.WScreen, mode->width , mode->height,
      spx,spy,torture_background,img_width,img_height,m);

  for (i=0; i < torture_doors_available; i++)
  {
    if (i == torture_door_selected)
    {
      spr = &doors[i].sprites[torture_sprite_frame];
    } else
    {
      spr = &doors[i].sprites[1];
    }
    LbSpriteDraw(spx+doors[i].field_0, spy+doors[i].field_4, spr);
  }
  return true;
}

void gui_previous_battle(struct GuiButton *gbtn)
{
  _DK_gui_previous_battle(gbtn);
}

void gui_next_battle(struct GuiButton *gbtn)
{
  _DK_gui_next_battle(gbtn);
}

void gui_get_creature_in_battle(struct GuiButton *gbtn)
{
  _DK_gui_get_creature_in_battle(gbtn);
}

void gui_go_to_person_in_battle(struct GuiButton *gbtn)
{
  _DK_gui_go_to_person_in_battle(gbtn);
}

void gui_setup_friend_over(struct GuiButton *gbtn)
{
  _DK_gui_setup_friend_over(gbtn);
}

void gui_area_friendly_battlers(struct GuiButton *gbtn)
{
  _DK_gui_area_friendly_battlers(gbtn);
}

void gui_setup_enemy_over(struct GuiButton *gbtn)
{
  _DK_gui_setup_enemy_over(gbtn);
}

void gui_area_enemy_battlers(struct GuiButton *gbtn)
{
  _DK_gui_area_enemy_battlers(gbtn);
}

void select_resurrect_creature(struct GuiButton *gbtn)
{
  _DK_select_resurrect_creature(gbtn);
}

void draw_resurrect_creature(struct GuiButton *gbtn)
{
  _DK_draw_resurrect_creature(gbtn);
}

void select_resurrect_creature_up(struct GuiButton *gbtn)
{
  _DK_select_resurrect_creature_up(gbtn);
}

void select_resurrect_creature_down(struct GuiButton *gbtn)
{
  _DK_select_resurrect_creature_down(gbtn);
}

void select_transfer_creature(struct GuiButton *gbtn)
{
  _DK_select_transfer_creature(gbtn);
}

void draw_transfer_creature(struct GuiButton *gbtn)
{
  _DK_draw_transfer_creature(gbtn);
}

void select_transfer_creature_up(struct GuiButton *gbtn)
{
  _DK_select_transfer_creature_up(gbtn);
}

void select_transfer_creature_down(struct GuiButton *gbtn)
{
  _DK_select_transfer_creature_down(gbtn);
}

void fronttorture_input(void)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  long x,y;
  long plyr_idx,door_id;
  clear_packets();
  player = get_my_player();
  pckt = get_packet(my_player_number);
  // Get inputs and create packet
  if (player->victory_state == 1)
  {
    if (left_button_clicked)
    {
      torture_left_button = 1;
      left_button_clicked = 0;
    }
    if ((lbKeyOn[KC_SPACE]) || (lbKeyOn[KC_RETURN]) || (lbKeyOn[KC_ESCAPE]))
    {
      lbKeyOn[KC_SPACE] = 0;
      lbKeyOn[KC_RETURN] = 0;
      lbKeyOn[KC_ESCAPE] = 0;
      pckt->action |= 0x01;
    }
    if (torture_left_button)
      pckt->action |= 0x02;
    if (left_button_held)
      pckt->action |= 0x04;
    pckt->field_6 = GetMouseX();
    pckt->field_8 = GetMouseY();
  }
  // Exchange packet with other players
  if ((game.system_flags & GSF_NetworkActive) != 0)
  {
    if (LbNetwork_Exchange(pckt))
      ERRORLOG("LbNetwork_Exchange failed");
  }
  // Determine the controlling player and get his mouse coords
  for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
  {
    player = get_player(plyr_idx);
    pckt = get_packet(plyr_idx);
    if ((pckt->action != 0) && (player->victory_state == 1))
      break;
  }
  if (plyr_idx < PLAYERS_COUNT)
  {
    x = pckt->field_6;
    y = pckt->field_8;
  } else
  {
    plyr_idx = my_player_number;
    player = get_player(plyr_idx);
    pckt = get_packet(plyr_idx);
    x = 0;
    y = 0;
  }
  if ((pckt->action & 0x01) != 0)
  {
    frontend_set_state(FeSt_LEVEL_STATS);
    if ((game.system_flags & GSF_NetworkActive) != 0)
      LbNetwork_Stop();
    return;
  }
  // Get active door
  door_id = torture_door_over_point(x,y);
  if ((torture_door_selected != -1) && (torture_door_selected != door_id))
    door_id = -1;
  // Make the action
  if (door_id == -1)
    torture_left_button = 0;
  switch (torture_state)
  {
  case 0:
      if (door_id != -1)
      {
        torture_state = 1;
        torture_sprite_direction = 1;
        torture_door_selected = door_id;
        torture_sprite_frame = 3;
        torture_end_sprite = 7;
      }
      break;
  case 1:
      if (torture_sprite_frame == torture_end_sprite)
      {
        if (door_id == -1)
        {
          torture_state = 2;
          torture_sprite_frame = 8;
          torture_end_sprite = 4;
          torture_sprite_direction = -1;
        } else
        if ((pckt->action & 6) != 0)
        {
          torture_state = 3;
          torture_left_button = 0;
          torture_sprite_frame = 7;
          torture_end_sprite = 11;
          torture_sprite_direction = 1;
          torture_play_sound(torture_door_selected, true);
        }
      }
      break;
  case 2:
      if (torture_sprite_frame == torture_end_sprite)
      {
        torture_state = 0;
        torture_door_selected = -1;
      }
      break;
  case 3:
      if (torture_sprite_frame == torture_end_sprite)
      {
        if (((pckt->action & 0x04) == 0) || (door_id == -1))
        {
          torture_state = 4;
          torture_sprite_frame = 12;
          torture_end_sprite = 8;
          torture_sprite_direction = -1;
          torture_play_sound(torture_door_selected, false);
        }
      }
      break;
  case 4:
      if (torture_sprite_frame == torture_end_sprite)
      {
        torture_state = 1;
        torture_sprite_frame = 7;
        torture_end_sprite = 7;
      }
      break;
  }
}

long gf_change_player_state(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  // Note: reworked from beta and unchecked
  struct PlayerInfo *player=get_my_player();
  set_players_packet_action(player, PckA_SetPlyrState, tag[0], tag[1], 0, 0);
  struct GuiBoxOption *guop;
  guop=gbox->optn_list;
  while (guop->label[0] != '!')
  {
    guop->active = 0;
    guop++;
  }
  goptn->active = 1;
  return 1;
}

long gf_decide_victory(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  //TODO: we should use packets! This way is unacceptable!
  struct PlayerInfo *player;
  player = get_my_player();
  if (tag[0])
    set_player_as_won_level(player);
  else
    set_player_as_lost_level(player);
  return 1;
}

long gf_change_player_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player=get_my_player();
  set_players_packet_action(player, 39, *tag, 0, 0, 0);
  return 1;
}

long gf_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  if ((player->field_2F <= 0) || (player->field_2F >= THINGS_COUNT))
    return 0;
  set_players_packet_action(player, PckA_CheatCrtSpells, 0, 0, 0, 0);
  return 1;
}

long gf_research_rooms(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  set_players_packet_action(player, PckA_CheatAllRooms, 0, 0, 0, 0);
  return 1;
}

long gf_all_researchable(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  set_players_packet_action(player, PckA_CheatAllResrchbl, 0, 0, 0, 0);
  return 1;
}

long gf_make_everything_free(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  set_players_packet_action(player, PckA_CheatAllFree, 0, 0, 0, 0);
  return 1;
}

long gf_give_all_creatures_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  set_players_packet_action(player, PckA_CheatCrAllSpls, 0, 0, 0, 0);
  return 1;
}

long gf_explore_everywhere(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  set_players_packet_action(player, PckA_CheatRevealMap, 0, 0, 0, 0);
  return 1;
}

long gf_research_magic(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  set_players_packet_action(player, PckA_CheatAllMagic, 0, 0, 0, 0);
  return 1;
}

long gfa_can_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag)
{
  struct PlayerInfo *player;
  player = get_my_player();
//  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
  if ((player->field_2F <= 0) || (player->field_2F >= THINGS_COUNT))
    return false;
  return true;
}

long gfa_controlled_creature_has_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag)
{
  struct PlayerInfo *player;
  struct Thing *thing;
  player = get_my_player();
  if ((player->field_2F <= 0) || (player->field_2F >= THINGS_COUNT))
    return false;
  thing = thing_get(player->field_2F);
  return creature_instance_is_available(thing, *tag);
}

/*
 * Draws parchment view background, used for in-game level map screen.
 */
void draw_map_parchment(void)
{
  parchment_copy_background_at(0,0,POS_AUTO,POS_AUTO);
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
  fntidx = frontend_button_info[fbinfo_idx%FRONTEND_BUTTON_INFO_COUNT].field_2;
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
     && (srvidx+1>=net_number_of_services) )
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

void frontend_load_game_maintain(struct GuiButton *gbtn)
{
  long game_index=load_game_scroll_offset+(long)(gbtn->field_33)-45;
  set_flag_byte(&gbtn->field_0, 0x08, (game_index < number_of_saved_games));
}

void add_score_to_high_score_table(void)
{
  struct Dungeon *dungeon;
  struct PlayerInfo *player;
  int idx;
  player = get_my_player();
  dungeon = get_players_dungeon(player);
  idx = add_high_score_entry(dungeon->lvstats.player_score, get_loaded_level_number(), "");
  if (idx >= 0)
  {
    // Preparing input in the new entry
    // Note that we're not clearing previous name - this way it may be easily kept unchanged
    high_score_entry_input_active = idx;
    high_score_entry_index = 0;
  } else
  {
    high_score_entry_input_active = -1;
    high_score_entry_index = 0;
  }
}

void do_button_release_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
  SYNCDBG(17,"Starting");
  int i;
  struct GuiMenu *gmnu;
  switch ( gbtn->gbtype )
  {
  case 0:
  case 1:
      if ((*s!=0) && (callback!=NULL))
      {
        do_sound_button_click(gbtn);
        callback(gbtn);
      }
      *s = 0;
      break;
  case 2:
      i = *(unsigned char *)gbtn->field_33;
      i++;
      if (gbtn->field_2D < i)
        i = 0;
      *(unsigned char *)gbtn->field_33 = i;
      if ((*s!=0) && (callback!=NULL))
      {
        do_sound_button_click(gbtn);
        callback(gbtn);
      }
      *s = 0;
      break;
  case Lb_RADIOBTN:
      if ( (char *)gbtn - (char *)s == -2 )
        return;
      break;
  case 5:
      input_button = gbtn;
      setup_input_field(input_button);
      break;
  default:
      break;
  }

  if ((char *)gbtn - (char *)s == -1)
  {
    gmnu = get_active_menu(gbtn->gmenu_idx);
    if (gbtn->field_2F != NULL)
      create_menu(gbtn->field_2F);
    if ((gbtn->field_0 & 0x02) && (gbtn->gbtype != 5))
    {
      if (callback == NULL)
        do_sound_menu_click();
      gmnu->field_1 = 3;
    }
  }
  SYNCDBG(17,"Finished");
}

/*
 * Returns if the menu is toggleable. If it's not, then it is always
 * visible until it's deleted.
 */
short is_toggleable_menu(short mnu_idx)
{
  switch (mnu_idx)
  {
  case GMnu_MAIN:
  case GMnu_ROOM:
  case GMnu_SPELL:
  case GMnu_TRAP:
  case GMnu_CREATURE:
  case GMnu_EVENT:
  case GMnu_QUERY:
      return true;
  case GMnu_TEXT_INFO:
  case GMnu_DUNGEON_SPECIAL:
  case GMnu_CREATURE_QUERY1:
  case GMnu_CREATURE_QUERY3:
  case GMnu_BATTLE:
  case GMnu_CREATURE_QUERY2:
  case GMnu_SPELL_LOST:
      return true;
  case GMnu_OPTIONS:
  case GMnu_INSTANCE:
  case GMnu_QUIT:
  case GMnu_LOAD:
  case GMnu_SAVE:
  case GMnu_VIDEO:
  case GMnu_SOUND:
  case GMnu_ERROR_BOX:
  case GMnu_HOLD_AUDIENCE:
  case GMnu_FEMAIN:
  case GMnu_FELOAD:
  case GMnu_FENET_SERVICE:
  case GMnu_FENET_SESSION:
  case GMnu_FENET_START:
  case GMnu_FENET_MODEM:
  case GMnu_FENET_SERIAL:
  case GMnu_FESTATISTICS:
  case GMnu_FEHIGH_SCORE_TABLE:
  case GMnu_RESURRECT_CREATURE:
  case GMnu_TRANSFER_CREATURE:
  case GMnu_ARMAGEDDON:
  case 33:
  case GMnu_FEDEFINE_KEYS:
  case GMnu_AUTOPILOT:
  case GMnu_FEOPTION:
  case GMnu_FELEVEL_SELECT:
  case GMnu_FECAMPAIGN_SELECT:
      return false;
  default:
      return true;
  }
}

void add_to_menu_stack(unsigned char mnu_idx)
{
  short i;
  if (no_of_active_menus >= ACTIVE_MENUS_COUNT)
  {
    ERRORLOG("No more room on menu stack");
    return;
  }

  for (i=0; i<no_of_active_menus; i++)
  {
    if (menu_stack[i] == mnu_idx)
    { // If already in stack, move it at end of the stack.
      while (i < no_of_active_menus-1)
      {
        menu_stack[i] = menu_stack[i+1];
        i++;
      }
      menu_stack[(int)no_of_active_menus-1] = mnu_idx;
      //SYNCMSG("Menu %d moved to end of stack, at position %d.",mnu_idx,no_of_active_menus-1);
      return;
    }
  }
  // If not in stack, add at end
  menu_stack[(unsigned char)no_of_active_menus] = mnu_idx;
  no_of_active_menus++;
  SYNCDBG(9,"Menu %d put on stack, at position %d.",mnu_idx,no_of_active_menus-1);
}

long first_available_menu(void)
{
  short i;
  for (i=0; i<ACTIVE_MENUS_COUNT; i++)
  {
    if (active_menus[i].field_1 == 0)
      return i;
  }
  return -1;
}

void turn_off_query_menus(void)
{
  turn_off_menu(31);
  turn_off_menu(35);
  turn_off_menu(32);
}

void setup_radio_buttons(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  int i;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    if ((gbtn->field_33) && (gmnu->field_14 == gbtn->gmenu_idx))
    {
      if (gbtn->gbtype == Lb_RADIOBTN)
      {
        if ( *(unsigned char *)gbtn->field_33 )
          gbtn->field_1 = 1;
        else
          gbtn->field_1 = 0;
      }
    }
  }
}

void turn_off_all_panel_menus(void)
{
  int mnu_num;
  struct GuiMenu *gmnu;
  mnu_num = menu_id_to_number(1);
  if (mnu_num >= 0)
  {
    gmnu = get_active_menu(mnu_num);
    setup_radio_buttons(gmnu);
  }
  if ( menu_is_active(2) )
  {
    turn_off_menu(2);
  }
  if ( menu_is_active(3) )
  {
    turn_off_menu(3);
  }
  if ( menu_is_active(4) )
  {
    turn_off_menu(4);
  }
  if ( menu_is_active(7) )
  {
    turn_off_menu(7);
  }
  if ( menu_is_active(5) )
  {
    turn_off_menu(5);
  }
  if ( menu_is_active(31) )
  {
    turn_off_menu(31);
  }
  if ( menu_is_active(35) )
  {
    turn_off_menu(35);
  }
  if ( menu_is_active(32) )
  {
    turn_off_menu(32);
  }
  if ( menu_is_active(38) )
  {
    turn_off_menu(38);
  }
}

void set_menu_mode(long mnu_idx)
{
  if (!menu_is_active(mnu_idx))
  {
    turn_off_all_panel_menus();
    turn_on_menu(mnu_idx);
  }
}

short turn_off_all_window_menus(void)
{
  short result;
  result = false;
  if (menu_is_active(10))
  {
    result = true;
    turn_off_menu(10);
  }
  if (menu_is_active(11))
  {
    result = true;
    set_packet_pause_toggle();
    turn_off_menu(11);
  }
  if (menu_is_active(GMnu_SAVE))
  {
    result = true;
    set_packet_pause_toggle();
    turn_off_menu(GMnu_SAVE);
  }
  if (menu_is_active(GMnu_OPTIONS))
  {
    result = true;
    turn_off_menu(GMnu_OPTIONS);
  }
  if (menu_is_active(GMnu_VIDEO))
  {
    result = true;
    turn_off_menu(GMnu_VIDEO);
  }
  if (menu_is_active(GMnu_SOUND))
  {
    result = true;
    turn_off_menu(GMnu_SOUND);
  }
  if (menu_is_active(GMnu_ERROR_BOX))
  {
    result = true;
    turn_off_menu(GMnu_ERROR_BOX);
  }
  if (menu_is_active(GMnu_INSTANCE))
  {
    result = true;
    turn_off_menu(GMnu_INSTANCE);
  }
  if (menu_is_active(GMnu_RESURRECT_CREATURE))
  {
    result = true;
    turn_off_menu(GMnu_RESURRECT_CREATURE);
  }
  if (menu_is_active(GMnu_TRANSFER_CREATURE))
  {
    result = true;
    turn_off_menu(GMnu_TRANSFER_CREATURE);
  }
  if (menu_is_active(GMnu_ARMAGEDDON))
  {
    result = true;
    turn_off_menu(GMnu_ARMAGEDDON);
  }
  if (menu_is_active(GMnu_AUTOPILOT))
  {
    result = true;
    turn_off_menu(GMnu_AUTOPILOT);
  }
  if (menu_is_active(GMnu_SPELL_LOST))
  {
    result = true;
    turn_off_menu(GMnu_SPELL_LOST);
  }
  return result;
}

void turn_on_main_panel_menu(void)
{
  if (menu_id_to_number(GMnu_MAIN) == -1)
  {
    turn_on_menu(GMnu_MAIN);
  }
  if (info_tag != 0)
  {
    turn_on_menu(GMnu_QUERY);
  } else
  if (room_tag != 0)
  {
    turn_on_menu(GMnu_ROOM);
  } else
  if (spell_tag != 0)
  {
    turn_on_menu(GMnu_SPELL);
  } else
  if (trap_tag != 0)
  {
    turn_on_menu(GMnu_TRAP);
  } else
  if (creature_tag != 0)
  {
    turn_on_menu(GMnu_CREATURE);
  }
}

short turn_off_all_bottom_menus(void)
{
  short result;
  result = false;
  if (menu_is_active(16))
  {
    result = true;
    turn_off_menu(16);
  }
  if (menu_is_active(34))
  {
    result = true;
    turn_off_menu(34);
  }
  if (menu_is_active(27))
  {
    result = true;
    turn_off_menu(27);
  }
  return result;
}

void turn_off_all_menus(void)
{
  turn_off_all_panel_menus();
  turn_off_all_window_menus();
  turn_off_all_bottom_menus();
}

void update_radio_button_data(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  unsigned char *rbstate;
  int i;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    rbstate = (unsigned char *)gbtn->field_33;
    if ((rbstate != NULL) && (gbtn->gmenu_idx == gmnu->field_14))
    {
      if (gbtn->gbtype == Lb_RADIOBTN)
      {
          if (gbtn->field_1)
            *rbstate = 1;
          else
            *rbstate = 0;
      }
    }
  }
}

void init_slider_bars(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  long sldpos;
  int i;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    if ((gbtn->field_33) && (gbtn->gmenu_idx == gmnu->field_14))
    {
      if (gbtn->gbtype == Lb_SLIDER)
      {
          sldpos = *(long *)gbtn->field_33;
          if (sldpos < 0)
            sldpos = 0;
          else
          if (sldpos > gbtn->field_2D)
            sldpos = gbtn->field_2D;
          gbtn->slide_val = (sldpos << 8) / (gbtn->field_2D + 1);
      }
    }
  }
}

void init_menu_buttons(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  Gf_Btn_Callback callback;
  int i;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    callback = gbtn->field_17;
    if ((callback != NULL) && (gbtn->gmenu_idx == gmnu->field_14))
      callback(gbtn);
  }
}

int create_button(struct GuiMenu *gmnu, struct GuiButtonInit *gbinit)
{
  //struct GuiButton *gbtn;
  int i;
  i=_DK_create_button(gmnu, gbinit);

  //gbtn = &active_buttons[i];
  //SYNCMSG("Created button %d at (%d,%d) size (%d,%d)",i,
  //    gbtn->pos_x,gbtn->pos_y,gbtn->width,gbtn->height);

  return i;
}

long compute_menu_position_x(long desired_pos,int menu_width)
{
  struct PlayerInfo *player;
  player = get_my_player();
  long pos;
  switch (desired_pos)
  {
  case POS_MOUSMID: // Place menu centered over mouse
      pos = GetMouseX() - (menu_width >> 1);
      break;
  case POS_GAMECTR: // Player-based positioning
      pos = (player->engine_window_x) + (player->engine_window_width >> 1) - (menu_width >> 1);
      break;
  case POS_MOUSPRV: // Place menu centered over previous mouse position
      pos = old_menu_mouse_x - (menu_width >> 1);
      break;
  case POS_SCRCTR:
      pos = (MyScreenWidth >> 1) - (menu_width >> 1);
      break;
  case POS_SCRBTM:
      pos = MyScreenWidth - menu_width;
      break;
  default: // Desired position have direct coordinates
      pos = ((desired_pos*units_per_pixel)>>4)*pixel_size;
      if (pos+menu_width > lbDisplay.PhysicalScreenWidth*pixel_size)
        pos = lbDisplay.PhysicalScreenWidth*pixel_size-menu_width;
/* Helps not to touch left panel - disabling, as needs additional conditions
      if (pos < 140)
        pos = 140;
*/
      break;
  }
  // Clipping position X
  if (desired_pos == POS_GAMECTR)
  {
    if (pos+menu_width > MyScreenWidth)
      pos = MyScreenWidth-menu_width;
    if (pos < player->engine_window_x)
      pos = player->engine_window_x;
  } else
  {
    if (pos+menu_width > MyScreenWidth)
      pos = MyScreenWidth-menu_width;
    if (pos < 0)
      pos = 0;
  }
  return pos;
}

long compute_menu_position_y(long desired_pos,int menu_height)
{
  struct PlayerInfo *player;
  player = get_my_player();
  long pos;
  switch (desired_pos)
  {
  case POS_MOUSMID: // Place menu centered over mouse
      pos = GetMouseY() - (menu_height >> 1);
      break;
  case POS_GAMECTR: // Player-based positioning
      pos = (player->engine_window_height >> 1) - ((menu_height+20) >> 1);
      break;
  case POS_MOUSPRV: // Place menu centered over previous mouse position
      pos = old_menu_mouse_y - (menu_height >> 1);
      break;
  case POS_SCRCTR:
      pos = (MyScreenHeight >> 1) - (menu_height >> 1);
      break;
  case POS_SCRBTM:
      pos = MyScreenHeight - menu_height;
      break;
  default: // Desired position have direct coordinates
      pos = ((desired_pos*units_per_pixel)>>4)*pixel_size;
      if (pos+menu_height > lbDisplay.PhysicalScreenHeight*pixel_size)
        pos = lbDisplay.PhysicalScreenHeight*pixel_size-menu_height;
      break;
  }
  // Clipping position Y
  if (pos+menu_height > MyScreenHeight)
    pos = MyScreenHeight-menu_height;
  if (pos < 0)
    pos = 0;
  return pos;
}

char create_menu(struct GuiMenu *gmnu)
{
  int mnu_num;
  struct GuiMenu *amnu;
  struct PlayerInfo *player;
  Gf_Mnu_Callback callback;
  struct GuiButtonInit *btninit;
  int i;
  SYNCDBG(18,"Starting menu %d",gmnu->field_0);
  mnu_num = menu_id_to_number(gmnu->field_0);
  if (mnu_num >= 0)
  {
    amnu = get_active_menu(mnu_num);
    amnu->field_1 = 1;
    amnu->numfield_2 = gmnu->numfield_2;
    amnu->flgfield_1D = ((game.numfield_C & 0x20) != 0) || (!is_toggleable_menu(gmnu->field_0));
    return mnu_num;
  }
  add_to_menu_stack(gmnu->field_0);
  mnu_num = first_available_menu();
  if (mnu_num == -1)
  {
      ERRORLOG("Too many menus open");
      return -1;
  }
  player = get_my_player();
  amnu = get_active_menu(mnu_num);
  amnu->field_1 = 1;
  amnu->field_14 = mnu_num;
  amnu->ptrfield_15 = gmnu;
  amnu->field_0 = gmnu->field_0;
  if (amnu->field_0 == 1)
  {
    old_menu_mouse_x = GetMouseX();
    old_menu_mouse_y = GetMouseY();
  }
  // Setting position X
  amnu->pos_x = compute_menu_position_x(gmnu->pos_x,gmnu->width);
  // Setting position Y
  amnu->pos_y = compute_menu_position_y(gmnu->pos_y,gmnu->height);

/* TODO what's this? remove?
  for (i=0; i<3; i++)
  {
    if ((menu_ids[i] == gmnu->field_0) && (MyScreenHeight == 480))
    {
      amnu->pos_y += 80;
      break;
    }
  }*/
  amnu->numfield_2 = gmnu->numfield_2;
  if (amnu->numfield_2 < 1)
    ERRORLOG("Oi! There is a fade time less than 1. Idiot.");
  amnu->ptrfield_4 = gmnu->ptrfield_4;
  amnu->width = gmnu->width;
  amnu->height = gmnu->height;
  amnu->draw_cb = gmnu->draw_cb;
  amnu->ptrfield_19 = gmnu->ptrfield_19;
  amnu->flgfield_1E = gmnu->flgfield_1E;
  amnu->field_1F = gmnu->field_1F;
  amnu->flgfield_1D = ((game.numfield_C & 0x20) != 0) || (!is_toggleable_menu(gmnu->field_0));
  callback = amnu->ptrfield_19;
  if (callback != NULL)
    callback(amnu);
  btninit = gmnu->ptrfield_4;
  for (i=0; btninit[i].field_0 != -1; i++)
  {
    if (create_button(amnu, &btninit[i]) == -1)
    {
      ERRORLOG("Cannot Allocate button");
      return -1;
    }
  }
  update_radio_button_data(amnu);
  init_slider_bars(amnu);
  init_menu_buttons(amnu);
  SYNCMSG("Created menu at slot %d, pos (%d,%d) size (%d,%d)",mnu_num,
      amnu->pos_x,amnu->pos_y,amnu->width,amnu->height);
  return mnu_num;
}

void turn_on_menu(short idx)
{
  if (create_menu(menu_list[idx]))
  {
    if (menu_list[idx]->field_1F)
      game.field_1517F6 = idx;
  }
}

void set_menu_visible_on(long menu_id)
{
  long menu_num;
  menu_num = menu_id_to_number(menu_id);
  if (menu_num < 0)
    return;
  get_active_menu(menu_num)->flgfield_1D = 1;
  int idx;
  for (idx=0; idx<ACTIVE_BUTTONS_COUNT; idx++)
  {
    struct GuiButton *gbtn = &active_buttons[idx];
    if (gbtn->field_0 & 1)
    {
      Gf_Btn_Callback callback;
      callback = gbtn->field_17;
      if ((gbtn->gmenu_idx == menu_num) && (callback != NULL))
        callback(gbtn);
    }
  }
}

void set_menu_visible_off(long menu_id)
{
  long menu_num;
  menu_num = menu_id_to_number(menu_id);
  if (menu_num < 0)
    return;
  get_active_menu(menu_num)->flgfield_1D = 0;
}

//TODO: Remove when original toggle_status_menu() won't be used anymore.
DLLIMPORT unsigned char _DK_room_on;
#define room_on _DK_room_on
DLLIMPORT unsigned char _DK_spell_on;
#define spell_on _DK_spell_on
DLLIMPORT unsigned char _DK_spell_lost_on;
#define spell_lost_on _DK_spell_lost_on
DLLIMPORT unsigned char _DK_trap_on;
#define trap_on _DK_trap_on
DLLIMPORT unsigned char _DK_creat_on;
#define creat_on _DK_creat_on
DLLIMPORT unsigned char _DK_event_on;
#define event_on _DK_event_on
DLLIMPORT unsigned char _DK_query_on;
#define query_on _DK_query_on
DLLIMPORT unsigned char _DK_creature_query1_on;
#define creature_query1_on _DK_creature_query1_on
DLLIMPORT unsigned char _DK_creature_query2_on;
#define creature_query2_on _DK_creature_query2_on
DLLIMPORT unsigned char _DK_creature_query3_on;
#define creature_query3_on _DK_creature_query3_on
DLLIMPORT unsigned char _DK_objective_on;
#define objective_on _DK_objective_on
DLLIMPORT unsigned char _DK_battle_on;
#define battle_on _DK_battle_on
DLLIMPORT unsigned char _DK_special_on;
#define special_on _DK_special_on

unsigned long toggle_status_menu(short visible)
{
/*
  static unsigned char room_on = 0;
  static unsigned char spell_on = 0;
  static unsigned char spell_lost_on = 0;
  static unsigned char trap_on = 0;
  static unsigned char creat_on = 0;
  static unsigned char event_on = 0;
  static unsigned char query_on = 0;
  static unsigned char creature_query1_on = 0;
  static unsigned char creature_query2_on = 0;
  static unsigned char creature_query3_on = 0;
  static unsigned char objective_on = 0;
  static unsigned char battle_on = 0;
  static unsigned char special_on = 0;
*/
  long k;
  unsigned long i;
  k = menu_id_to_number(1);
  if (k < 0) return 0;
  i = get_active_menu(k)->flgfield_1D;
  if (visible != i)
  {
    if ( visible )
    {
      set_menu_visible_on(GMnu_MAIN);
      if ( room_on )
        set_menu_visible_on(GMnu_ROOM);
      if ( spell_on )
        set_menu_visible_on(GMnu_SPELL);
      if ( spell_lost_on )
        set_menu_visible_on(GMnu_SPELL_LOST);
      if ( trap_on )
        set_menu_visible_on(GMnu_TRAP);
      if ( event_on )
        set_menu_visible_on(GMnu_EVENT);
      if ( query_on )
        set_menu_visible_on(GMnu_QUERY);
      if ( creat_on )
        set_menu_visible_on(GMnu_CREATURE);
      if ( creature_query1_on )
        set_menu_visible_on(GMnu_CREATURE_QUERY1);
      if ( creature_query2_on )
        set_menu_visible_on(GMnu_CREATURE_QUERY2);
      if ( creature_query3_on )
        set_menu_visible_on(GMnu_CREATURE_QUERY3);
      if ( battle_on )
        set_menu_visible_on(GMnu_BATTLE);
      if ( objective_on )
        set_menu_visible_on(GMnu_TEXT_INFO);
      if ( special_on )
        set_menu_visible_on(GMnu_DUNGEON_SPECIAL);
    } else
    {
      set_menu_visible_off(GMnu_MAIN);
      k = menu_id_to_number(GMnu_ROOM);
      if (k >= 0)
        room_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_ROOM);
      k = menu_id_to_number(GMnu_SPELL);
      if (k >= 0)
        spell_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_SPELL);
      k = menu_id_to_number(GMnu_SPELL_LOST);
      if (k >= 0)
        spell_lost_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_SPELL_LOST);
      k = menu_id_to_number(GMnu_TRAP);
      if (k >= 0)
      trap_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_TRAP);
      k = menu_id_to_number(GMnu_CREATURE);
      if (k >= 0)
        creat_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_CREATURE);
      k = menu_id_to_number(GMnu_EVENT);
      if (k >= 0)
        event_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_EVENT);
      k = menu_id_to_number(GMnu_QUERY);
      if (k >= 0)
        query_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_QUERY);
      k = menu_id_to_number(GMnu_CREATURE_QUERY1);
      if (k >= 0)
        creature_query1_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_CREATURE_QUERY1);
      k = menu_id_to_number(GMnu_CREATURE_QUERY2);
      if (k >= 0)
        creature_query2_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_CREATURE_QUERY2);
      k = menu_id_to_number(GMnu_CREATURE_QUERY3);
      if (k >= 0)
        creature_query3_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_CREATURE_QUERY3);
      k = menu_id_to_number(GMnu_TEXT_INFO);
      if (k >= 0)
        objective_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_TEXT_INFO);
      k = menu_id_to_number(GMnu_BATTLE);
      if (k >= 0)
        battle_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_BATTLE);
      k = menu_id_to_number(GMnu_DUNGEON_SPECIAL);
      if (k >= 0)
        special_on = get_active_menu(k)->flgfield_1D;
      set_menu_visible_off(GMnu_DUNGEON_SPECIAL);
    }
  }
  return i;
}

TbBool toggle_first_person_menu(TbBool visible)
{
  static unsigned char creature_query_on = 0;
  if (visible)
  {
    if (creature_query_on & 0x01)
        set_menu_visible_on(GMnu_CREATURE_QUERY1);
    else
    if ( creature_query_on & 0x02)
      set_menu_visible_on(GMnu_CREATURE_QUERY2);
    else
    {
      WARNMSG("No active query for first person menu; assuming query 1.");
      set_menu_visible_on(GMnu_CREATURE_QUERY1);
    }
    return true;
  } else
  {
    long menu_num;
    // CREATURE_QUERY1
    menu_num = menu_id_to_number(GMnu_CREATURE_QUERY1);
    if (menu_num >= 0)
      set_flag_byte(&creature_query_on, 0x01, get_active_menu(menu_num)->flgfield_1D);
    set_menu_visible_off(GMnu_CREATURE_QUERY1);
    // CREATURE_QUERY2
    menu_num = menu_id_to_number(GMnu_CREATURE_QUERY2);
    if (menu_num >= 0)
      set_flag_byte(&creature_query_on, 0x02, get_active_menu(menu_num)->flgfield_1D);
    set_menu_visible_off(GMnu_CREATURE_QUERY2);
    return true;
  }
}

void set_gui_visible(short visible)
{
  SYNCDBG(6,"Starting");
  set_flag_byte(&game.numfield_C,0x20,visible);
  struct PlayerInfo *player=get_my_player();
  unsigned char is_visbl = ((game.numfield_C & 0x20) != 0);
  switch (player->view_type)
  {
  case PVT_CreatureContrl:
      toggle_first_person_menu(is_visbl);
      break;
  case PVT_MapScreen:
      toggle_status_menu(false);
      break;
  case PVT_DungeonTop:
  default:
      toggle_status_menu(is_visbl);
      break;
  }
  if (((game.numfield_D & 0x20) != 0) && ((game.numfield_C & 0x20) != 0))
    setup_engine_window(status_panel_width, 0, MyScreenWidth, MyScreenHeight);
  else
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
}

void toggle_gui(void)
{
  short visible=((game.numfield_C & 0x20) == 0);
  set_gui_visible(visible);
}

void frontend_load_data_from_cd(void)
{
    LbDataLoadSetModifyFilenameFunction(_DK_mdlf_for_cd);
}

void frontend_load_data_reset(void)
{
  LbDataLoadSetModifyFilenameFunction(_DK_mdlf_default);
}

void frontstory_load(void)
{
    wait_for_cd_to_be_available();
    frontend_load_data_from_cd();
    if ( LbDataLoadAll(frontstory_load_files) )
    {
        ERRORLOG("Unable to Load FRONT STORY FILES");
    } else
    {
        LbDataLoadSetModifyFilenameFunction(_DK_mdlf_default);
        LbSpriteSetupAll(frontstory_setup_sprites);
        LbPaletteSet(frontend_palette);
        srand(LbTimerClock());
        frontstory_text_no = rand() % 26 + 803;
    }
}
void inline frontstory_unload(void)
{
    LbDataFreeAll(frontstory_load_files);
}

void frontstats_set_timer(void)
{
  frontstats_timer = LbTimerClock() + 3000;
}

void frontstats_save_high_score(void)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(my_player_number);
  if (dungeon->lvstats.allow_save_score)
  {
    dungeon->lvstats.allow_save_score = false;
    add_score_to_high_score_table();
  }
  lbInkey = 0;
}

void frontend_level_select_up(struct GuiButton *gbtn)
{
  if (select_level_scroll_offset > 0)
    select_level_scroll_offset--;
}

void frontend_level_select_down(struct GuiButton *gbtn)
{
  if (select_level_scroll_offset < number_of_freeplay_levels-frontend_select_level_items_visible+1)
    select_level_scroll_offset++;
}

void frontend_level_select_up_maintain(struct GuiButton *gbtn)
{
  if (gbtn != NULL)
    set_flag_byte(&gbtn->field_0, 0x08, (select_level_scroll_offset != 0));
}

void frontend_level_select_down_maintain(struct GuiButton *gbtn)
{
  if (gbtn != NULL)
    set_flag_byte(&gbtn->field_0, 0x08, (select_level_scroll_offset < number_of_freeplay_levels-frontend_select_level_items_visible+1));
}

void frontend_level_select_maintain(struct GuiButton *gbtn)
{
  long i;
  if (gbtn != NULL)
  {
    i = (long)gbtn->field_33 - 45;
    set_flag_byte(&gbtn->field_0, 0x08, (select_level_scroll_offset+i < number_of_freeplay_levels));
  }
}

void frontend_draw_level_select_button(struct GuiButton *gbtn)
{
  struct LevelInformation *lvinfo;
  long btn_idx;
  long lvnum;
  long i;
  btn_idx = (long)gbtn->field_33;
  i = btn_idx + select_level_scroll_offset - 45;
  lvnum = 0;
  if ((i >= 0) && (i < campaign.freeplay_levels_count))
    lvnum = campaign.freeplay_levels[i];
  lvinfo = get_level_info(lvnum);
  if (lvinfo == NULL)
    return;
  if ((btn_idx > 0) && (frontend_mouse_over_button == btn_idx))
    i = 2;
  else
  if (get_level_highest_score(lvnum))
    i = 3;
  else
    i = 1;
  lbDisplay.DrawFlags = 0x20;
  LbTextSetFont(frontend_font[i]);
  i = LbTextLineHeight();
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, i);
  LbTextDraw(0, 0, lvinfo->name);
}

void frontend_level_select(struct GuiButton *gbtn)
{
  long i;
  long lvnum;
  i = (long)gbtn->field_33 + select_level_scroll_offset - 45;
  lvnum = 0;
  if (i < campaign.freeplay_levels_count)
    lvnum = campaign.freeplay_levels[i];
  if (lvnum > 0)
  {
    game.selected_level_number = lvnum;
    game.flags_font |= FFlg_unk80;
    frontend_set_state(7);
  }
}

void frontend_level_list_unload(void)
{
  // Nothing needs to be really unloaded; just menu cleanup here
  number_of_freeplay_levels = 0;
}

void frontend_level_list_load(void)
{
  find_and_load_lif_files();
  find_and_load_lof_files();
  number_of_freeplay_levels = campaign.freeplay_levels_count;
}

void frontend_level_select_update(void)
{
  if (number_of_freeplay_levels <= 0)
  {
    select_level_scroll_offset = 0;
  } else
  if (select_level_scroll_offset < 0)
  {
    select_level_scroll_offset = 0;
  } else
  if (select_level_scroll_offset > number_of_freeplay_levels-1)
  {
    select_level_scroll_offset = number_of_freeplay_levels - 1;
  }
}

void frontend_campaign_select_up(struct GuiButton *gbtn)
{
  if (select_level_scroll_offset > 0)
    select_level_scroll_offset--;
}

void frontend_campaign_select_down(struct GuiButton *gbtn)
{
  if (select_level_scroll_offset < campaigns_list.items_num-frontend_select_campaign_items_visible+1)
    select_level_scroll_offset++;
}

void frontend_campaign_select_up_maintain(struct GuiButton *gbtn)
{
  if (gbtn != NULL)
    set_flag_byte(&gbtn->field_0, 0x08, (select_level_scroll_offset != 0));
}

void frontend_campaign_select_down_maintain(struct GuiButton *gbtn)
{
  if (gbtn != NULL)
    set_flag_byte(&gbtn->field_0, 0x08, (select_level_scroll_offset < campaigns_list.items_num-frontend_select_campaign_items_visible+1));
}

void frontend_campaign_select_maintain(struct GuiButton *gbtn)
{
  long btn_idx;
  long i;
  if (gbtn == NULL)
    return;
  btn_idx = (long)gbtn->field_33;
  i = select_level_scroll_offset + btn_idx-45;
  set_flag_byte(&gbtn->field_0, 0x08, (i < campaigns_list.items_num));
}

void frontend_draw_campaign_select_button(struct GuiButton *gbtn)
{
  struct GameCampaign *campgn;
  long btn_idx;
  long i;
  if (gbtn == NULL)
    return;
  btn_idx = (long)gbtn->field_33;
  i = select_level_scroll_offset + btn_idx-45;
  campgn = NULL;
  if ((i >= 0) && (i < campaigns_list.items_num))
    campgn = &campaigns_list.items[i];
  if (campgn == NULL)
    return;
  if ((btn_idx > 0) && (frontend_mouse_over_button == btn_idx))
    i = 2;
  else
/*  if (campaign has been passed)
    i = 3;
  else*/
    i = 1;
  lbDisplay.DrawFlags = 0x20;
  LbTextSetFont(frontend_font[i]);
  i = LbTextLineHeight();
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, i);
  LbTextDraw(0, 0, campgn->name);
}

void frontend_campaign_select(struct GuiButton *gbtn)
{
  long i;
  struct GameCampaign *campgn;
  i = (long)gbtn->field_33 + select_level_scroll_offset - 45;
  campgn = NULL;
  if ((i >= 0) && (i < campaigns_list.items_num))
    campgn = &campaigns_list.items[i];
  if (campgn == NULL)
    return;
  if (!frontend_start_new_campaign(campgn->fname))
  {
    ERRORLOG("Unable to start new campaign");
    return;
  }
  frontend_set_state(FeSt_LAND_VIEW);
}

void frontend_campaign_select_update(void)
{
  if (campaigns_list.items_num <= 0)
  {
    select_level_scroll_offset = 0;
  } else
  if (select_level_scroll_offset < 0)
  {
    select_level_scroll_offset = 0;
  } else
  if (select_level_scroll_offset > campaigns_list.items_num-1)
  {
    select_level_scroll_offset = campaigns_list.items_num-1;
  }
}

void frontend_draw_campaign_scroll_tab(struct GuiButton *gbtn)
{
  frontend_draw_scroll_tab(gbtn, select_level_scroll_offset, frontend_select_campaign_items_visible-2, campaigns_list.items_num);
}

void initialise_tab_tags(long menu_id)
{
  info_tag =  (menu_id == 7) || (menu_id == 31) || (menu_id == 35) || (menu_id == 32);
  room_tag = (menu_id == 2);
  spell_tag = (menu_id == 3);
  trap_tag = (menu_id == 4);
  creature_tag = (menu_id == 5);
}

void initialise_tab_tags_and_menu(long menu_id)
{
  long menu_num;
  initialise_tab_tags(menu_id);
  menu_num = menu_id_to_number(menu_id);
  if (menu_num >= 0)
    setup_radio_buttons(get_active_menu(menu_num));
}

void init_gui(void)
{
  LbMemorySet(breed_activities, 0, CREATURE_TYPES_COUNT*sizeof(unsigned short));
  LbMemorySet(menu_stack, 0, ACTIVE_MENUS_COUNT*sizeof(unsigned char));
  LbMemorySet(active_menus, 0, ACTIVE_MENUS_COUNT*sizeof(struct GuiMenu));
  LbMemorySet(active_buttons, 0, ACTIVE_BUTTONS_COUNT*sizeof(struct GuiButton));
  breed_activities[0] = 23;
  no_of_breeds_owned = 1;
  top_of_breed_list = 0;
  old_menu_mouse_x = -999;
  old_menu_mouse_y = -999;
  drag_menu_x = -999;
  drag_menu_y = -999;
  initialise_tab_tags(2);
  new_objective = 0;
  input_button = 0;
  busy_doing_gui = 0;
  no_of_active_menus = 0;
}

int frontend_set_state(long nstate)
{
  char *fname;
  SYNCDBG(8,"State %d will be switched to %d",frontend_menu_state,nstate);
  switch (frontend_menu_state)
  {
  case 0:
      init_gui();
      wait_for_cd_to_be_available();
      fname = prepare_file_path(FGrp_LoData,"front.pal");
      if (LbFileLoadAt(fname, frontend_palette) != PALETTE_SIZE)
        ERRORLOG("Unable to load FRONTEND PALETTE");
      wait_for_cd_to_be_available();
      LbMouseSetPosition(lbDisplay.PhysicalScreenWidth>>1, lbDisplay.PhysicalScreenHeight>>1);
      update_mouse();
      break;
  case FeSt_MAIN_MENU: // main menu state
      turn_off_menu(18);
      break;
  case 2:
      turn_off_menu(19);
      break;
  case FeSt_LAND_VIEW:
      frontmap_unload();
      frontend_load_data();
      break;
  case 4:
      turn_off_menu(20);
      break;
  case FeSt_NET_SESSION: // Network play mode
      turn_off_menu(GMnu_FENET_SESSION);
      break;
  case FeSt_NET_START:
      turn_off_menu(GMnu_FENET_START);
      break;
  case FeSt_STORY_POEM:
  case FeSt_STORY_BIRTHDAY:
      frontstory_unload();
      break;
  case FeSt_CREDITS:
      if ((game.flags_cd & MFlg_NoMusic) == 0)
        StopRedbookTrack();
      break;
  case FeSt_NET_MODEM:
      turn_off_menu(23);
      frontnet_modem_reset();
      break;
  case FeSt_NET_SERIAL:
      turn_off_menu(24);
      frontnet_serial_reset();
      break;
  case FeSt_LEVEL_STATS:
      StopStreamedSample();
      turn_off_menu(25);
      break;
  case FeSt_HIGH_SCORES:
      turn_off_menu(26);
      break;
  case FeSt_TORTURE:
      fronttorture_unload();
      frontend_load_data();
      break;
  case FeSt_NETLAND_VIEW:
      frontnetmap_unload();
      frontend_load_data();
      break;
  case FeSt_FEDEFINE_KEYS:
      turn_off_menu(36);
      save_settings();
      break;
  case FeSt_FEOPTIONS:
      turn_off_menu(39);
      if ((game.flags_cd & MFlg_NoMusic) == 0)
        StopRedbookTrack();
      break;
  case FeSt_LEVEL_SELECT:
      turn_off_menu(GMnu_FELEVEL_SELECT);
      frontend_level_list_unload();
      break;
  case FeSt_CAMPAIGN_SELECT:
      turn_off_menu(GMnu_FECAMPAIGN_SELECT);
      break;
  case 7:
  case 8:
  case 9:
  case 10:
  case FeSt_INTRO:
  case FeSt_DEMO: //demo state (intro/credits)
  case FeSt_OUTRO:
  case 25:
      break;
#if (BFDEBUG_LEVEL > 0)
  case FeSt_FONT_TEST:
      free_testfont_fonts();
      break;
#endif
  default:
      ERRORLOG("Unhandled FRONTEND previous state");
      break;
  }
  if ( frontend_menu_state )
    fade_out();
  fade_palette_in = 1;
  SYNCMSG("Frontend state change from %u into %u",frontend_menu_state,nstate);
  switch ( nstate )
  {
    case 0:
      set_pointer_graphic_none();
      break;
    case FeSt_MAIN_MENU:
      set_pointer_graphic_menu();
      continue_game_option_available = continue_game_available();
      turn_on_menu(GMnu_FEMAIN);
      last_mouse_x = GetMouseX();
      last_mouse_y = GetMouseY();
      time_last_played_demo = LbTimerClock();
      fe_high_score_table_from_main_menu = true;
      set_flag_byte(&game.system_flags, GSF_NetworkActive, false);
      break;
    case FeSt_FELOAD_GAME:
      turn_on_menu(GMnu_FELOAD);
      set_pointer_graphic_menu();
      break;
    case FeSt_LAND_VIEW:
      if ( !frontmap_load() )
        nstate = 7;
      break;
    case FeSt_NET_SERVICE:
      turn_on_menu(GMnu_FENET_SERVICE);
      frontnet_service_setup();
      break;
    case FeSt_NET_SESSION:
      turn_on_menu(GMnu_FENET_SESSION);
      frontnet_session_setup();
      set_pointer_graphic_menu();
      set_flag_byte(&game.system_flags, GSF_NetworkActive, false);
      break;
    case FeSt_NET_START:
      turn_on_menu(GMnu_FENET_START);
      frontnet_start_setup();
      set_pointer_graphic_menu();
      set_flag_byte(&game.system_flags, GSF_NetworkActive, true);
      break;
    case 7:
    case 9:
    case 10:
    case FeSt_INTRO:
    case FeSt_DEMO:
    case FeSt_OUTRO:
    case 25:
      fade_palette_in = 0;
      break;
    case 8:
      if ((game.flags_font & FFlg_unk10) != 0)
        LbNetwork_ChangeExchangeTimeout(30);
      fade_palette_in = 0;
      break;
    case FeSt_STORY_POEM:
    case FeSt_STORY_BIRTHDAY:
      frontstory_load();
      break;
    case FeSt_CREDITS:
      credits_offset = lbDisplay.PhysicalScreenHeight;
      credits_end = 0;
      LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
      lbDisplay.DrawFlags = 0x0100;
      break;
    case FeSt_NET_MODEM:
      turn_on_menu(GMnu_FENET_MODEM);
      frontnet_modem_setup();
      break;
    case FeSt_NET_SERIAL:
      turn_on_menu(GMnu_FENET_SERIAL);
      frontnet_serial_setup();
      break;
    case FeSt_LEVEL_STATS:
      turn_on_menu(GMnu_FESTATISTICS);
      set_pointer_graphic_menu();
      frontstats_set_timer();
      break;
    case FeSt_HIGH_SCORES:
      turn_on_menu(GMnu_FEHIGH_SCORE_TABLE);
      frontstats_save_high_score();
      set_pointer_graphic_menu();
      break;
    case FeSt_TORTURE:
      set_pointer_graphic_menu();
      fronttorture_load();
      break;
    case FeSt_NETLAND_VIEW:
      set_pointer_graphic_menu();
      frontnetmap_load();
      break;
    case FeSt_FEDEFINE_KEYS:
      defining_a_key = 0;
      define_key_scroll_offset = 0;
      turn_on_menu(GMnu_FEDEFINE_KEYS);
      break;
    case FeSt_FEOPTIONS:
      turn_on_menu(GMnu_FEOPTION);
      break;
  case FeSt_LEVEL_SELECT:
      set_pointer_graphic_menu();
      turn_on_menu(GMnu_FELEVEL_SELECT);
      frontend_level_list_load();
      set_pointer_graphic_menu();
      break;
  case FeSt_CAMPAIGN_SELECT:
      turn_on_menu(GMnu_FECAMPAIGN_SELECT);
      break;
#if (BFDEBUG_LEVEL > 0)
  case FeSt_FONT_TEST:
      fade_palette_in = 0;
      load_testfont_fonts();
      set_pointer_graphic_menu();
      break;
#endif
    default:
      ERRORLOG("Unhandled FRONTEND new state");
      break;
  }
  frontend_menu_state = nstate;
  return frontend_menu_state;
}

short frontstory_input(void)
{
  return false;
}

TbBool frontmainmnu_input(void)
{
  int mouse_x,mouse_y;
  // check if mouse position has changed
  mouse_x = GetMouseX();
  mouse_y = GetMouseY();
  if ((mouse_x != last_mouse_x) || (mouse_y != last_mouse_y))
  {
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;
    time_last_played_demo = LbTimerClock();
  }
  // Handle key inputs
  if (lbKeyOn[KC_G] && lbKeyOn[KC_LSHIFT])
  {
    lbKeyOn[KC_G] = 0;
    frontend_set_state(FeSt_CREDITS);
    return true;
  }
  if (lbKeyOn[KC_T] && lbKeyOn[KC_LSHIFT])
  {
    if ((game.flags_font & FFlg_AlexCheat) != 0)
    {
      lbKeyOn[KC_T] = 0;
      set_player_as_won_level(get_my_player());
      frontend_set_state(FeSt_TORTURE);
      return true;
    }
  }
#if (BFDEBUG_LEVEL > 0)
  if (lbKeyOn[KC_F] && lbKeyOn[KC_LSHIFT])
  {
    if ((game.flags_font & FFlg_AlexCheat) != 0)
    {
      lbKeyOn[KC_F] = 0;
      frontend_set_state(FeSt_FONT_TEST);
      return true;
    }
  }
#endif
  // Handle GUI inputs
  return get_gui_inputs(0);
}

short end_input(void)
{
  if (lbKeyOn[KC_SPACE])
  {
    lbKeyOn[KC_SPACE] = 0;
    frontend_set_state(FeSt_MAIN_MENU);
    return true;
  }
  if (lbKeyOn[KC_RETURN])
  {
    lbKeyOn[KC_RETURN] = 0;
    frontend_set_state(FeSt_MAIN_MENU);
    return true;
  }
  if (lbKeyOn[KC_ESCAPE])
  {
    lbKeyOn[KC_ESCAPE] = 0;
    frontend_set_state(FeSt_MAIN_MENU);
    return true;
  }
  if (left_button_clicked)
  {
    left_button_clicked = 0;
    frontend_set_state(FeSt_MAIN_MENU);
    return true;
  }
  return false;
}

short get_frontend_global_inputs(void)
{
  if (is_key_pressed(KC_X, KM_ALT))
  {
    clear_key_pressed(KC_X);
    exit_keeper = true;
  } else
    return false;
  return true;
}

void frontend_input(void)
{
  SYNCDBG(7,"Starting");
  switch (frontend_menu_state)
  {
      case FeSt_MAIN_MENU:
        frontmainmnu_input();
        break;
      case FeSt_LAND_VIEW:
        frontmap_input();
        break;
      case FeSt_NET_START:
        get_gui_inputs(0);
        frontnet_start_input();
        break;
      case FeSt_STORY_POEM:
      case FeSt_STORY_BIRTHDAY:
        end_input();
        frontstory_input();
        break;
      case FeSt_CREDITS:
        if (!end_input())
        {
          if ( credits_end )
            frontend_set_state(FeSt_MAIN_MENU);
        }
        frontcredits_input();
        break;
      case FeSt_HIGH_SCORES:
        get_gui_inputs(0);
         frontend_high_score_table_input();
        break;
      case FeSt_TORTURE:
        fronttorture_input();
        break;
      case FeSt_NETLAND_VIEW:
        frontnetmap_input();
        break;
      case FeSt_FEDEFINE_KEYS:
        if ( !defining_a_key )
          get_gui_inputs(0);
        else
          define_key_input();
        break;
#if (BFDEBUG_LEVEL > 0)
      case FeSt_FONT_TEST:
        fronttestfont_input();
        break;
#endif
      default:
        get_gui_inputs(0);
        break;
  } // end switch
  get_frontend_global_inputs();
  get_screen_capture_inputs();
  SYNCDBG(19,"Finished");
}

int get_bitmap_max_scale(int img_w,int img_h,int rect_w,int rect_h)
{
  int w,h,m;
  w = 0;
  h = 0;
  for (m=0; m < 5; m++)
  {
    w += img_w;
    h += img_h;
    if (w > rect_w) break;
    if (h > rect_h) break;
  }
  // The image width can't be larger than video resolution
  if (m < 1)
  {
    if (w > lbDisplay.PhysicalScreenWidth)
      return 0;
    m = 1;
  }
  return m;
}

void frontend_copy_background_at(int rect_x,int rect_y,int rect_w,int rect_h)
{
  const int img_width = 640;
  const int img_height = 480;
  const unsigned char *srcbuf=frontend_background;
  TbScreenMode * mode = getActiveScreenMode();
  int m;
  int spx,spy;

  if (rect_w == POS_AUTO)
    rect_w = mode->width - rect_x;
  if (rect_h == POS_AUTO)
    rect_h = mode->height - rect_y;
  if (rect_w<0) rect_w=0;
  if (rect_h<0) rect_h=0;
  m = get_bitmap_max_scale(img_width, img_height, rect_w, rect_h);
  if (m < 1)
  {
    SYNCMSG("The %dx%d frontend image does not fit in %dx%d window, skipped.", img_width, img_height,rect_w,rect_h);
    return;
  }
  // Starting point coords
  spx = rect_x + ((rect_w-m*img_width)>>1);
  spy = rect_y + ((rect_h-m*img_height)>>1);
  // Do the drawing
  copy_raw8_image_buffer(lbDisplay.WScreen, mode->width, mode->height,
      spx,spy,srcbuf,img_width,img_height,m);
}

void parchment_copy_background_at(int rect_x,int rect_y,int rect_w,int rect_h)
{
  int img_width;
  int img_height;
  unsigned char *srcbuf;
  if (lbDisplay.GraphicsScreenWidth < 640)
  {
    img_width = 320;
    img_height = 200;
    srcbuf = poly_pool;
  } else
  {
    img_width = 640;
    img_height = 480;
    srcbuf = hires_parchment;
  }
  TbScreenMode * mode = getActiveScreenMode();

  int m;
  int spx,spy;

  if (rect_w == POS_AUTO)
    rect_w = mode->width - rect_x;
  if (rect_h == POS_AUTO)
    rect_h = mode->height - rect_y;
  if (rect_w<0) rect_w=0;
  if (rect_h<0) rect_h=0;
  // Parchment bitmap can't be scaled
  m = 1;
  // Starting point coords
  spx=0;spy=0; // disabled, for now
/*
  spx = rect_x + ((rect_w-m*img_width)>>1);
  spy = rect_y + ((rect_h-m*img_height)>>1);
  if (spy<0) spy=0;
*/
  // Do the drawing
  copy_raw8_image_buffer(lbDisplay.WScreen, mode->width, mode->height,
      spx,spy,srcbuf,img_width,img_height,m);
  // Burning candle flames
  LbSpriteDraw(spx+(36/pixel_size),(spy+0/pixel_size), &button_sprite[198+(game.play_gameturn & 3)]);
  LbSpriteDraw(spx+(574/pixel_size),(spy+0/pixel_size), &button_sprite[202+(game.play_gameturn & 3)]);
}

void frontend_copy_background(void)
{
  frontend_copy_background_at(0,0,POS_AUTO,POS_AUTO);
}

void frontstory_draw(void)
{
  frontend_copy_background();
  LbTextSetWindow(70, 70, 500, 340);
  LbTextSetFont(frontstory_font);
  lbDisplay.DrawFlags = 0x0100;
  LbTextDraw(0, 0, gui_strings[frontstory_text_no%STRINGS_MAX]);
}

void draw_defining_a_key_box(void)
{
    draw_text_box(gui_strings[470]); // "Press a key"
}

char update_menu_fade_level(struct GuiMenu *gmnu)
{
  return _DK_update_menu_fade_level(gmnu);
}

void toggle_gui_overlay_map(void)
{
  toggle_flag_byte(&game.numfield_C,0x20);
}

void draw_menu_buttons(struct GuiMenu *gmnu)
{
  int i;
  struct GuiButton *gbtn;
  Gf_Btn_Callback callback;
  SYNCDBG(18,"Starting phase one");
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    callback = gbtn->field_13;
    if ((callback != NULL) && (gbtn->field_0 & 0x04) && (gbtn->field_0 & 0x01) && (gbtn->gmenu_idx == gmnu->field_14))
    {
      if ((gbtn->field_1 == 0) && (gbtn->field_2 == 0) || (gbtn->gbtype == Lb_SLIDER) || (callback == gui_area_null))
        callback(gbtn);
    }
  }
  SYNCDBG(18,"Starting phase two");
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    callback = gbtn->field_13;
    if ((callback != NULL) && (gbtn->field_0 & 0x04) && (gbtn->field_0 & 0x01) && (gbtn->gmenu_idx == gmnu->field_14))
    {
      if (((gbtn->field_1) || (gbtn->field_2)) && (gbtn->gbtype != Lb_SLIDER) && (callback != gui_area_null))
        callback(gbtn);
    }
  }
  SYNCDBG(19,"Finished");
}

long menu_id_to_number(short menu_id)
{
  int idx;
  struct GuiMenu *gmnu;
  for(idx=0; idx < ACTIVE_MENUS_COUNT; idx++)
  {
    gmnu = &active_menus[idx];
    if ((gmnu->field_1 != 0) && (gmnu->field_0 == menu_id))
      return idx;
  }
  return -1;
}

void update_fade_active_menus(void)
{
  SYNCDBG(8,"Starting");
  struct GuiMenu *gmnu;
  int k;
  for (k=0; k < ACTIVE_MENUS_COUNT; k++)
  {
    gmnu = &active_menus[k];
    if (update_menu_fade_level(gmnu) == -1)
    {
      kill_menu(gmnu);
      remove_from_menu_stack(gmnu->field_0);
    }
  }
  SYNCDBG(19,"Finished");
}

void draw_active_menus_buttons(void)
{
  struct GuiMenu *gmnu;
  int k;
  long menu_num;
  Gf_Mnu_Callback callback;
  SYNCDBG(8,"Starting with %d active menus",no_of_active_menus);
  for (k=0; k < no_of_active_menus; k++)
  {
    menu_num = menu_id_to_number(menu_stack[k]);
    if (menu_num < 0) continue;
    gmnu = &active_menus[menu_num];
//SYNCMSG("DRAW menu %d, fields %d, %d",menu_num,gmnu->field_1,gmnu->flgfield_1D);
    if ((gmnu->field_1 != 0) && (gmnu->flgfield_1D))
    {
        if ((gmnu->field_1 != 2) && (gmnu->numfield_2))
        {
          if (gmnu->ptrfield_15 != NULL)
            if (gmnu->ptrfield_15->numfield_2)
              lbDisplay.DrawFlags |= 0x04;
        }
        callback = gmnu->draw_cb;
        if (callback != NULL)
          callback(gmnu);
        if (gmnu->field_1 == 2)
          draw_menu_buttons(gmnu);
        lbDisplay.DrawFlags &= 0xFFFBu;
    }
  }
  SYNCDBG(9,"Finished");
}

void spangle_button(struct GuiButton *gbtn)
{
  long x,y;
  unsigned long i;
  x = ((gbtn->width >> 1) - pixel_size * button_sprite[176].SWidth / 2 + gbtn->pos_x);
  y = ((gbtn->height >> 1) - pixel_size * button_sprite[176].SHeight / 2 + gbtn->pos_y);
  i = 176+((game.play_gameturn >> 1) & 7);
  LbSpriteDraw(x/pixel_size, y/pixel_size, &button_sprite[i]);
}

void draw_menu_spangle(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  int i;
  short in_range;
  if (gmnu->flgfield_1D == 0)
    return;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    if ((!gbtn->field_13) || ((gbtn->field_0 & 0x04) == 0) || ((gbtn->field_0 & 0x01) == 0) || (game.flash_button_index == 0))
      continue;
    in_range = 0;
    switch (gbtn->id_num)
    {
    case BID_INFO_TAB:
      if ((game.flash_button_index >= 68) && (game.flash_button_index <= 71))
        in_range = 1;
      break;
    case BID_ROOM_TAB:
      if ((game.flash_button_index >= 6) && (game.flash_button_index <= 20))
        in_range = 1;
      break;
    case BID_SPELL_TAB:
      if ((game.flash_button_index >= 21) && (game.flash_button_index <= 36))
        in_range = 1;
      break;
    case BID_TRAP_TAB:
      if ((game.flash_button_index >= 53) && (game.flash_button_index <= 61))
        in_range = 1;
      break;
    case BID_CREATR_TAB:
      if ((game.flash_button_index >= 72) && (game.flash_button_index <= 74))
        in_range = 1;
      break;
    default:
      break;
    }
    if (in_range)
    {
      if (!menu_is_active(gbtn->field_1B))
        spangle_button(gbtn);
    } else
    if ((gbtn->id_num > 0) && (gbtn->id_num == game.flash_button_index))
    {
      spangle_button(gbtn);
    }
  }
}

void draw_active_menus_highlights(void)
{
  struct GuiMenu *gmnu;
  int k;
  SYNCDBG(8,"Starting");
  for (k=0; k<ACTIVE_MENUS_COUNT; k++)
  {
    gmnu = &active_menus[k];
    if ((gmnu->field_1) && (gmnu->field_0 == 1))
      draw_menu_spangle(gmnu);
  }
}

void draw_gui(void)
{
  SYNCDBG(6,"Starting");
  unsigned int flg_mem;
  LbTextSetFont(winfont);
  flg_mem = lbDisplay.DrawFlags;
  LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  update_fade_active_menus();
  draw_active_menus_buttons();
  if (game.flash_button_index != 0)
  {
    draw_active_menus_highlights();
    if (game.flash_button_gameturns != -1)
    {
      game.flash_button_gameturns--;
      if (game.flash_button_gameturns <= 0)
        game.flash_button_index = 0;
    }
  }
  lbDisplay.DrawFlags = flg_mem;
  SYNCDBG(8,"Finished");
}

struct TbBirthday {
    unsigned char day;
    unsigned char month;
    const char *name;
    };

const struct TbBirthday team_birthdays[] = {
    {13, 1,"Mark Healey"},
    {21, 3,"Jonty Barnes"},
    { 3, 5,"Simon Carter"},
    { 5, 5,"Peter Molyneux"},
    {13,11,"Alex Peters"},
    { 1,12,"Dene Carter"},
    {25, 5,"Tomasz Lis"},
    {29,11,"Michael Chateauneuf"},
    {0,0,NULL},
    };

const char *get_team_birthday()
{
  struct TbDate curr_date;
  LbDate(&curr_date);
  int i;
  for (i=0;team_birthdays[i].day!=0;i++)
  {
      if ((team_birthdays[i].day==curr_date.Day) &&
          (team_birthdays[i].month==curr_date.Month))
      {
          return team_birthdays[i].name;
      }
  }
  return NULL;
}

void frontbirthday_draw()
{
  frontend_copy_background();
  LbTextSetWindow(70, 70, 500, 340);
  LbTextSetFont(frontstory_font);
  lbDisplay.DrawFlags = 0x0100;
  const char *name=get_team_birthday();
  if ( name != NULL )
  {
      unsigned short line_pos;
      line_pos = LbTextLineHeight();
      LbTextDraw(0, 170-line_pos, gui_strings[885]); // "Happy Birthday"
      LbTextDraw(0, 170, name);
  } else
  {
      frontend_set_state(FeSt_INTRO);
  }
}

/**
 * Frontend drawing function.
 * @return Gives 0 if a movie has started, 1 if normal draw occured, 2 on error.
 */
short frontend_draw(void)
{
    short result;
    switch (frontend_menu_state)
    {
    case FeSt_INTRO:
        intro();
        return 0;
    case FeSt_DEMO:
        demo();
        return 0;
    case FeSt_OUTRO:
        outro();
        return 0;
    }

    if (LbScreenLock() != Lb_SUCCESS)
        return 2;

    result = 1;
    switch ( frontend_menu_state )
    {
    case FeSt_MAIN_MENU:
    case FeSt_FELOAD_GAME:
    case FeSt_NET_SERVICE:
    case FeSt_NET_SESSION:
    case FeSt_NET_MODEM:
    case FeSt_NET_SERIAL:
    case FeSt_LEVEL_STATS:
    case FeSt_HIGH_SCORES:
    case 20:
    case FeSt_FEOPTIONS:
    case FeSt_LEVEL_SELECT:
    case FeSt_CAMPAIGN_SELECT:
        draw_gui();
        break;
    case FeSt_LAND_VIEW:
        frontmap_draw();
        break;
    case FeSt_NET_START:
        draw_gui();
        break;
    case FeSt_STORY_POEM:
        frontstory_draw();
        break;
    case FeSt_CREDITS:
        frontcredits_draw();
        break;
    case FeSt_TORTURE:
        fronttorture_draw();
        break;
    case FeSt_NETLAND_VIEW:
        frontnetmap_draw();
        break;
    case FeSt_FEDEFINE_KEYS:
        draw_gui();
        if ( defining_a_key )
            draw_defining_a_key_box();
        break;
    case FeSt_STORY_BIRTHDAY:
        frontbirthday_draw();
        break;
#if (BFDEBUG_LEVEL > 0)
    case FeSt_FONT_TEST:
        fronttestfont_draw();
        break;
#endif
    default:
        break;
    }
    // In-Menu information, for debugging messages
    //char text[255];
    //sprintf(text, "time %7d, mode %d",LbTimerClock(),frontend_menu_state);
    //lbDisplay.DrawFlags=0;LbTextSetWindow(0,0,640,200);LbTextSetFont(frontend_font[0]);
    //LbTextDraw(200/pixel_size, 8/pixel_size, text);text[0]='\0';
    perform_any_screen_capturing();
    LbScreenUnlock();
    return result;
}

void load_game_update(void)
{
    if ((number_of_saved_games>0) && (load_game_scroll_offset>=0))
    {
        if ( load_game_scroll_offset > number_of_saved_games-1 )
          load_game_scroll_offset = number_of_saved_games-1;
    } else
    {
        load_game_scroll_offset = 0;
    }
}

void spell_lost_first_person(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player=get_my_player();
  set_players_packet_action(player, 110, 0, 0, 0, 0);
}

void gui_turn_on_autopilot(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  if (player->victory_state != VicS_LostLevel)
  {
    set_players_packet_action(player, 107, 0, 0, 0, 0);
  }
}

void gui_set_autopilot(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  int ntype;
  if (game.comp_player_aggressive)
  {
    ntype = 1;
  } else
  if (game.comp_player_defensive)
  {
    ntype = 2;
  } else
  if (game.comp_player_construct)
  {
    ntype = 3;
  } else
  if (game.comp_player_creatrsonly)
  {
    ntype = 4;
  } else
  {
    ERRORLOG("Illegal Autopilot type, resetting to default");
    ntype = 1;
  }
  set_players_packet_action(player, PckA_SetComputerKind, ntype, 0, 0, 0);
}

void display_objectives(long plyr_idx,long x,long y)
{
  _DK_display_objectives(plyr_idx,x,y);
}

void frontnet_service_update(void)
{
  _DK_frontnet_service_update();
}

void frontnet_session_update(void)
{
//  _DK_frontnet_session_update();
  static long last_enum_players = 0;
  static long last_enum_sessions = 0;
  long i;

  if (LbTimerClock() >= last_enum_sessions)
  {
    net_number_of_sessions = 0;
    memset(net_session, 0, sizeof(net_session));
    if ( LbNetwork_EnumerateSessions(enum_sessions_callback, 0) )
      ERRORLOG("LbNetwork_EnumerateSessions() failed");
    last_enum_sessions = LbTimerClock();

    if (net_number_of_sessions == 0)
    {
      net_session_index_active = -1;
      net_session_index_active_id = -1;
    } else
    if (net_session_index_active != -1)
    {
        if ((net_session_index_active >= net_number_of_sessions)
          || (!net_session[net_session_index_active]->joinable))
        {
          net_session_index_active = -1;
          for (i=0; i < net_number_of_sessions; i++)
          {
            if ( net_session[i]->joinable)
            {
              net_session_index_active = i;
              break;
            }
          }
        }
        if (net_session_index_active == -1)
          net_session_index_active_id = -1;
    }
  }

  if ((net_number_of_sessions == 0) || (net_session_scroll_offset < 0))
  {
    net_session_scroll_offset = 0;
  } else
  if (net_session_scroll_offset > net_number_of_sessions-1)
  {
    net_session_scroll_offset = net_number_of_sessions-1;
  }

  if (net_session_index_active == -1)
  {
    net_number_of_enum_players = 0;
  } else
  if (LbTimerClock() >= last_enum_players)
  {
    net_number_of_enum_players = 0;
    memset(net_player, 0, sizeof(net_player));
    if ( LbNetwork_EnumeratePlayers(net_session[net_session_index_active], enum_players_callback, 0) )
    {
      net_session_index_active = -1;
      net_session_index_active_id = -1;
      return;
    }
    last_enum_players = LbTimerClock();
  }

  if (net_number_of_enum_players == 0)
  {
    net_player_scroll_offset = 0;
  } else
  if (net_player_scroll_offset < 0)
  {
    net_player_scroll_offset = 0;
  } else
  if (net_player_scroll_offset > net_number_of_enum_players-1)
  {
    net_player_scroll_offset = net_number_of_enum_players-1;
  }
}

void frontnet_modem_update(void)
{
  _DK_frontnet_modem_update();
}

void frontnet_serial_update(void)
{
  _DK_frontnet_serial_update();
}

void frontnet_rewite_net_messages(void)
{
  struct NetMessage lmsg[NET_MESSAGES_COUNT];
  struct NetMessage *nmsg;
  long i,k;
  k = 0;
  i = net_number_of_messages;
  for (i=0; i < NET_MESSAGES_COUNT; i++)
    LbMemorySet(&lmsg[i], '\0', sizeof(struct NetMessage));
  for (i=0; i < net_number_of_messages; i++)
  {
    nmsg = &net_message[i];
    if (net_player_info[nmsg->plyr_idx].active)
    {
      memcpy(&lmsg[k], nmsg, sizeof(struct NetMessage));
      k++;
    }
  }
  net_number_of_messages = k;
  for (i=0; i < NET_MESSAGES_COUNT; i++)
    memcpy(&net_message[i], &lmsg[i], sizeof(struct NetMessage));
}

void frontnet_start_update(void)
{
  static TbClockMSec player_last_time = 0;
  SYNCDBG(18,"Starting");
  if (LbTimerClock() >= player_last_time+200)
  {
    net_number_of_enum_players = 0;
    LbMemorySet(net_player, 0, sizeof(net_player));
    if ( LbNetwork_EnumeratePlayers(net_session[net_session_index_active], enum_players_callback, 0) )
    {
      ERRORLOG("LbNetwork_EnumeratePlayers() failed");
      return;
    }
    player_last_time = LbTimerClock();
  }
  if ((net_number_of_messages <= 0) || (net_message_scroll_offset < 0))
  {
    net_message_scroll_offset = 0;
  } else
  if (net_message_scroll_offset > net_number_of_messages-1)
  {
    net_message_scroll_offset = net_number_of_messages-1;
  }
  process_frontend_packets();
  frontnet_rewite_net_messages();
  return;
}

void frontstats_update(void)
{
  LevelNumber lvnum;
  int h;
  scrolling_offset++;
  LbTextSetFont(frontend_font[1]);
  h = LbTextLineHeight();
  if (h+4 < scrolling_offset)
  {
    scrolling_offset -= h+4;
    scrolling_index++;
    if (!scrolling_stats_data[scrolling_index].field_0)
      scrolling_index = 0;
  }
  lvnum = get_loaded_level_number();
  if (frontstats_timer != 0)
    if (LbTimerClock() > frontstats_timer)
    {
      play_description_speech(lvnum,0);
      frontstats_timer = 0;
    }
}

void fronttorture_update(void)
{
  _DK_fronttorture_update();
}

void frontend_update(short *finish_menu)
{
    SYNCDBG(18,"Starting for menu state %d", (int)frontend_menu_state);
    switch ( frontend_menu_state )
    {
      case FeSt_MAIN_MENU:
        frontend_button_info[8].field_2 = (continue_game_option_available?1:3);
        //this uses original timing function for compatibility with frontend_set_state()
        if ( abs(LbTimerClock()-time_last_played_demo) > MNU_DEMO_IDLE_TIME )
          frontend_set_state(FeSt_DEMO);
        break;
      case FeSt_FELOAD_GAME:
        load_game_update();
        break;
      case FeSt_LAND_VIEW:
        *finish_menu = frontmap_update();
        break;
      case FeSt_NET_SERVICE:
        frontnet_service_update();
        break;
      case FeSt_NET_SESSION:
        frontnet_session_update();
        break;
      case FeSt_NET_START:
        frontnet_start_update();
        break;
      case 7:
      case 8:
      case FeSt_LOAD_GAME:
      case FeSt_PACKET_DEMO:
        *finish_menu = 1;
        break;
      case 9:
        *finish_menu = 1;
        exit_keeper = 1;
        break;
      case FeSt_CREDITS:
        if ((game.flags_cd & MFlg_NoMusic) == 0)
          PlayRedbookTrack(7);
        break;
      case FeSt_NET_MODEM:
        frontnet_modem_update();
        break;
      case FeSt_NET_SERIAL:
        frontnet_serial_update();
        break;
      case FeSt_LEVEL_STATS:
        frontstats_update();
        break;
      case FeSt_TORTURE:
        fronttorture_update();
        break;
      case FeSt_NETLAND_VIEW:
        *finish_menu = frontnetmap_update();
        break;
      case FeSt_FEOPTIONS:
        if ((game.flags_cd & MFlg_NoMusic) == 0)
          PlayRedbookTrack(3);
        break;
      case FeSt_LEVEL_SELECT:
        frontend_level_select_update();
        break;
      case FeSt_CAMPAIGN_SELECT:
        frontend_campaign_select_update();
        break;
      default:
        break;
    }
  SYNCDBG(17,"Finished");
}

/*
 * Chooses initial frontend menu state.
 * Used when game is first run, or player exits from gameplay.
 */
int get_startup_menu_state(void)
{
  struct PlayerInfo *player;
  LevelNumber lvnum;
  if (game.flags_cd & 0x40)
  {
    if (is_full_moon)
    {
        SYNCLOG("Full moon state selected");
        return FeSt_STORY_POEM;
    } else
    if (get_team_birthday() != NULL)
    {
        SYNCLOG("Birthday state selected");
        return FeSt_STORY_BIRTHDAY;
    } else
    {
        SYNCLOG("Standard startup state selected");
        return 1;
    }
  } else
  {
    SYNCLOG("Player-based state selected");
    player = get_my_player();
    lvnum = get_loaded_level_number();
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
      if ((player->field_3 & 0x10) != 0)
      {
        player->field_3 &= 0xEF;
        return FeSt_TORTURE;
      } else
      if ((player->field_6 & 0x02) == 0)
      {
        return FeSt_LEVEL_STATS;
      } else
      if ( setup_old_network_service() )
      {
        return FeSt_NET_SESSION;
      } else
      {
        return FeSt_MAIN_MENU;
      }
    } else
    if ((player->field_6 & 0x02) || (player->victory_state == VicS_Undecided))
    {
      if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum) || is_extra_level(lvnum))
      {
        return FeSt_LAND_VIEW;
      } else
      if (is_freeplay_level(lvnum)) // note that this will only work if LIFs are loaded
      {
        return FeSt_LEVEL_SELECT;
      } else
      {
          return FeSt_MAIN_MENU;
      }
    } else
    if (game.flags_cd & 0x01)
    {
      game.flags_cd &= 0xFEu;
      return FeSt_MAIN_MENU;
    } else
    if (player->victory_state == VicS_WonLevel)
    {
      if (is_singleplayer_level(lvnum))
      {
        if ((player->field_3 & 0x10) != 0)
        {
           player->field_3 &= 0xEF;
           return FeSt_TORTURE;
        } else
        if (get_continue_level_number() == SINGLEPLAYER_FINISHED)
        {
          return FeSt_OUTRO;
        } else
        {
          return FeSt_LEVEL_STATS;
        }
      } else
      if (is_bonus_level(lvnum) || is_extra_level(lvnum))
      {
        return FeSt_LAND_VIEW;
      } else
      {
        return FeSt_LEVEL_STATS;
      }
    } else
    if (player->victory_state == VicS_State3)
    {
        return FeSt_LEVEL_STATS;
    } else
    if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum) || is_extra_level(lvnum))
    {
      return FeSt_LAND_VIEW;
    } else
    if (is_freeplay_level(lvnum)) // note that this will only work if LIFs are loaded
    {
      return FeSt_LEVEL_SELECT;
    } else
    {
        return FeSt_MAIN_MENU;
    }
  }
  ERRORLOG("Unresolved menu state");
  return FeSt_MAIN_MENU;
}

void gui_draw_all_boxes(void)
{
  struct GuiBox *gbox;
  SYNCDBG(5,"Starting");
  lbDisplay.DrawFlags = 0x0040;
  LbTextSetFont(font_sprites);
  gbox = gui_get_lowest_priority_box();
  while (gbox != NULL)
  {
    gui_draw_box(gbox);
    gbox = gui_get_next_highest_priority_box(gbox);
  }
}

short gui_box_is_not_valid(struct GuiBox *gbox)
{
  if (gbox == NULL) return true;
  return (gbox->field_0 & 0x01) == 0;
}

void gui_insert_box_at_list_top(struct GuiBox *gbox)
{
  if (gbox->field_0 & 0x02)
  {
    ERRORLOG("GuiBox is already in list");
    return;
  }
  gbox->field_0 |= 0x02;
  gbox->next_box = first_box;
  if (first_box != NULL)
      first_box->prev_box = gbox;
  else
      last_box = gbox;
  first_box = gbox;
}

struct GuiBox *gui_allocate_box_structure(void)
{
  int i;
  struct GuiBox *gbox;
  for (i=1;i<3;i++)
  {
    gbox = &gui_boxes[i];
    if (gui_box_is_not_valid(gbox))
    {
      gbox->field_1 = i;
      gbox->field_0 |= 0x01;
      gui_insert_box_at_list_top(gbox);
      return gbox;
    }
  }
  return NULL;
}

long gui_calculate_box_width(struct GuiBox *gbox)
{
  struct GuiBoxOption *goptn;
  int w,maxw;
  maxw = 0;
  goptn = gbox->optn_list;
  while (goptn->label[0] != '!')
  {
    w = pixel_size * LbTextStringWidth(goptn->label);
    if (w > maxw)
      maxw = w;
    goptn++;
  }
  return maxw+16;
}

long gui_calculate_box_height(struct GuiBox *gbox)
{
  struct GuiBoxOption *goptn;
  int i;
  i = 0;
  goptn = gbox->optn_list;
  while (goptn->label[0] != '!')
  {
    i++;
    goptn++;
  }
  return i*(pixel_size*LbTextLineHeight()+2) + 16;
}

void gui_remove_box_from_list(struct GuiBox *gbox)
{
  if ((gbox->field_0 & 0x02) == 0)
  {
    ERRORLOG("Cannot remove box from list when it is not in one!");
    return;
  }
  gbox->field_0 &= 0xFDu;
  if ( gbox->prev_box )
      gbox->prev_box->next_box = gbox->next_box;
  else
      first_box = gbox->next_box;
  if ( gbox->next_box )
      gbox->next_box->prev_box = gbox->prev_box;
  else
      last_box = gbox->prev_box;
  gbox->prev_box = 0;
  gbox->next_box = 0;
}

void gui_delete_box(struct GuiBox *gbox)
{
  gui_remove_box_from_list(gbox);
  memset(gbox, 0, sizeof(struct GuiBox));
}

struct GuiBox *gui_create_box(long x, long y, struct GuiBoxOption *optn_list)
{
  struct GuiBox *gbox;
  gbox = gui_allocate_box_structure();
  if (gbox == NULL)
    return NULL;
  // Setting gui font - will be required to properly calculate box dimensions
  LbTextSetFont(font_sprites);
  gbox->optn_list = optn_list;
  gbox->pos_x = x;
  gbox->pos_y = y;
  gbox->width = gui_calculate_box_width(gbox);
  gbox->height = gui_calculate_box_height(gbox);
  return gbox;
}

short gui_move_box(struct GuiBox *gbox, long x, long y, unsigned short fdflags)
{
  short result;
  switch (fdflags)
  {
  case Fnt_LeftJustify:
      gbox->pos_x = x;
      gbox->pos_y = y;
      result = true;
      break;
  case Fnt_RightJustify:
      gbox->pos_x = x - gbox->width;
      gbox->pos_y = y - gbox->height;
      result = true;
      break;
  case Fnt_CenterPos:
      gbox->pos_x = x - (gbox->width >> 1);
      gbox->pos_y = y - (gbox->height >> 1);
      result = true;
      break;
  default:
      result = false;
      break;
  }
  if (gbox->pos_x+gbox->width > MyScreenWidth)
    gbox->pos_x = MyScreenWidth-gbox->width;
  if (gbox->pos_x < 0)
    gbox->pos_x = 0;
  if (gbox->pos_y+gbox->height > MyScreenHeight)
    gbox->pos_y = MyScreenHeight-gbox->height;
  if (gbox->pos_y < 0)
    gbox->pos_y = 0;
  return result;
}

/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * @return Gives true if the menu was toggled, false if cheat is not allowed.
 */
short toggle_main_cheat_menu(void)
{
  long mouse_x = GetMouseX();
  long mouse_y = GetMouseY();
  if ((gui_box==NULL) || (gui_box_is_not_valid(gui_box)))
  {
    if ((game.flags_font & FFlg_AlexCheat) == 0)
      return false;
    gui_box = gui_create_box(mouse_x,mouse_y,gui_main_cheat_list);
    gui_move_box(gui_box, mouse_x, mouse_y, Fnt_CenterPos);
  } else
  {
    gui_delete_box(gui_box);
    gui_box=NULL;
  }
  return true;
}

/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * @return Gives true if the menu was toggled, false if cheat is not allowed.
 */
short toggle_instance_cheat_menu(void)
{
  // Toggle cheat menu
  if ((gui_box==NULL) || (gui_box_is_not_valid(gui_box)))
  {
    if ((game.flags_font & FFlg_AlexCheat) == 0)
      return false;
    gui_box = gui_create_box(200,20,gui_instance_option_list);
/*
        player->unknownbyte  |= 0x08;
        game.unknownbyte |= 0x08;
*/
  } else
  {
    gui_delete_box(gui_box);
    gui_box=NULL;
/*
        player->unknownbyte &= 0xF7;
        game.unknownbyte &= 0xF7;
*/
  }
  return true;
}

/*
 * Opens cheat menu. It should not allow cheats in Network mode.
 * Returns true if the menu was toggled, false if cheat is not allowed.
 */
TbBool open_creature_cheat_menu(void)
{
  if ((game.flags_font & FFlg_AlexCheat) == 0)
    return false;
  if (!gui_box_is_not_valid(gui_cheat_box))
    return false;
  gui_cheat_box = gui_create_box(150,20,gui_creature_cheat_option_list);
  return (!gui_box_is_not_valid(gui_cheat_box));
}

/*
 * Closes cheat menu.
 * Returns true if the menu was closed.
 */
TbBool close_creature_cheat_menu(void)
{
  if (gui_box_is_not_valid(gui_cheat_box))
    return false;
  gui_delete_box(gui_cheat_box);
  gui_cheat_box = NULL;
  return true;
}

/*
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * Returns true if the menu was toggled, false if cheat is not allowed.
 */
TbBool toggle_creature_cheat_menu(void)
{
  // Cheat sub-menus
  if (gui_box_is_not_valid(gui_cheat_box))
  {
    return open_creature_cheat_menu();
  } else
  {
    return close_creature_cheat_menu();
  }
}


struct GuiBox *gui_get_highest_priority_box(void)
{
  return first_box;
}

struct GuiBox *gui_get_lowest_priority_box(void)
{
  return last_box;
}

struct GuiBox *gui_get_next_highest_priority_box(struct GuiBox *gbox)
{
  return gbox->prev_box;
}

struct GuiBox *gui_get_next_lowest_priority_box(struct GuiBox *gbox)
{
  return gbox->next_box;
}

struct GuiBox *gui_get_box_point_over(long x, long y)
{
  struct GuiBox *gbox;
  gbox = gui_get_highest_priority_box();
  while (gbox != NULL)
  {
    if ((y >= gbox->pos_y) && (y < gbox->pos_y+gbox->height))
      if ((x >= gbox->pos_x) && (x < gbox->pos_x+gbox->width))
        return gbox;
    gbox = gui_get_next_lowest_priority_box(gbox);
  }
  return NULL;
}

/*
 * Returns box option under given position.
 * Requires text font to be set properly before running.
 */
struct GuiBoxOption *gui_get_box_option_point_over(struct GuiBox *gbox, long x, long y)
{
  long sx,sy,lnheight;
  long width,height;
  struct GuiBoxOption *gboptn;
  sx = gbox->pos_x + 8;
  sy = gbox->pos_y + 8;
  gboptn = gbox->optn_list;
  lnheight = LbTextLineHeight()*pixel_size + 2;
  while (gboptn->label[0] != '!')
  {
    height = LbTextStringHeight(gboptn->label)*pixel_size;
    if ((y >= sy) && (y < sy+height))
    {
      width = LbTextStringWidth(gboptn->label)*pixel_size;
      if ((x >= sx) && (x < sx+width))
      {
        if ((gboptn->numfield_4 == 2) || (gboptn->field_26 == 0))
          return NULL;
        return gboptn;
      }
    }
    gboptn++;
    sy += lnheight;
  }
  return NULL;
}

struct GuiBoxOption *gui_move_active_box_option(struct GuiBox *gbox, int val)
{
  int opt_num;
  int opt_total;
  struct GuiBoxOption *goptn;
  if (gbox == NULL)
    return NULL;
  opt_num = -1;
  opt_total = 0;
  goptn = gbox->optn_list;
  while (goptn->label[0] != '!')
  {
    if (goptn->active)
    {
      opt_num = opt_total;
//      goptn->active = 0;
//TODO: deactivate
    }
    goptn++;
    opt_total++;
  }
  opt_num += val;
  if ((opt_num >= 0) && (opt_num < opt_total))
  {
    goptn = &gbox->optn_list[opt_num];
    if (goptn->callback != NULL)
      goptn->callback(gbox, goptn, 1, &goptn->field_19);
//TODO: activate
    return goptn;
  }
  return NULL;
}

void gui_draw_box(struct GuiBox *gbox)
{
  SYNCDBG(6,"Drawing box, first optn \"%s\"",gbox->optn_list->label);
  struct GuiBox *gbox_over;
  struct GuiBoxOption *goptn_over;
  struct GuiBoxOption *goptn;
  long lnheight;
  long mouse_x,mouse_y;
  long pos_x,pos_y;
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  mouse_x = GetMouseX();
  mouse_y = GetMouseY();
  goptn_over = NULL;
  gbox_over = gui_get_box_point_over(mouse_x, mouse_y);
  if (gbox_over != NULL)
  {
    goptn_over = gui_get_box_option_point_over(gbox_over, mouse_x, mouse_y);
  }

  LbTextSetFont(font_sprites);
  lnheight = pixel_size * LbTextLineHeight() + 2;
  pos_y = gbox->pos_y + 8;
  pos_x = gbox->pos_x + 8;
  if (gbox != gui_get_highest_priority_box())
  {
    lbDisplay.DrawFlags |= 0x0004;
    LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[6][0][0]);
    if (lbDisplay.DrawFlags & 0x0010)
    {
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[0][0][0]);
    } else
    {
      lbDisplay.DrawFlags ^= 0x0010;
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[0][0][0]);
      lbDisplay.DrawFlags ^= 0x0010;
    }
    lbDisplay.DrawFlags ^= 0x0004;
    lbDisplay.DrawColour = colours[3][3][3];
    goptn = gbox->optn_list;
    while (goptn->label[0] != '!')
    {
      if (goptn->active_cb != NULL)
        goptn->field_26 = (goptn->active_cb)(gbox, goptn, &goptn->field_D);
      else
        goptn->field_26 = 1;
      if (!goptn->field_26)
        lbDisplay.DrawColour = colours[0][0][0];
      else
        lbDisplay.DrawColour = colours[3][3][3];
      if (LbScreenIsLocked())
      {
        LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, goptn->label);
      }
      goptn++;
      pos_y += lnheight;
    }
  } else
  {
    lbDisplay.DrawFlags |= 0x0004;
    LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[12][0][0]);
    if (lbDisplay.DrawFlags & 0x0010)
    {
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[2][0][0]);
    } else
    {
      lbDisplay.DrawFlags ^= 0x0010;
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[2][0][0]);
      lbDisplay.DrawFlags ^= 0x0010;
    }
    lbDisplay.DrawFlags ^= 0x0004;
    goptn = gbox->optn_list;
    while (goptn->label[0] != '!')
    {
      if (goptn->active_cb != NULL)
        goptn->field_26 = (goptn->active_cb)(gbox, goptn, &goptn->field_D);
      else
        goptn->field_26 = 1;
      if (!goptn->field_26)
        lbDisplay.DrawColour = colours[0][0][0];
      else
      if ((gbox == gbox_over) && (goptn == goptn_over) && (gbox != dragging_box.gbox) ||
           (gbox != NULL) && (goptn->active != 0))
        lbDisplay.DrawColour = colours[15][15][15];
      else
        lbDisplay.DrawColour = colours[9][9][9];
      if (LbScreenIsLocked())
      {
        LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, goptn->label);
      }
      goptn++;
      pos_y += lnheight;
    }
  }
}

TbBool gui_process_option_inputs(struct GuiBox *gbox, struct GuiBoxOption *goptn)
{
  short button_num;
  if (left_button_released || right_button_released)
  {
    if (left_button_released)
    {
      left_button_released = 0;
      button_num = 1;
    } else
    {
      right_button_released = 0;
      button_num = 2;
    }
    if (goptn->numfield_4 == 1)
    {
      if (goptn->callback != NULL)
        goptn->callback(gbox, goptn, button_num, &goptn->field_19);
    }
    return true;
  }
  return false;
}

/*
 * Processes GUI Bixes inputs.
 * @return Returns true if the input event was captured by a GUI Box.
 */
short gui_process_inputs(void)
{
  long mouse_y,mouse_x;
  struct GuiBox *gbox;
  struct GuiBox *hpbox;
  struct GuiBoxOption *goptn;
  short result;
  SYNCDBG(8,"Starting");
  mouse_x = GetMouseX();
  mouse_y = GetMouseY();
  result = false;
  hpbox = gui_get_highest_priority_box();
  goptn = NULL;
  if (dragging_box.gbox != NULL)
  {
    if (left_button_held)
    {
      if (hpbox != dragging_box.gbox)
      {
        gui_remove_box_from_list(dragging_box.gbox);
        gui_insert_box_at_list_top(dragging_box.gbox);
      }
      dragging_box.gbox->pos_x = mouse_x - dragging_box.start_x;
      dragging_box.gbox->pos_y = mouse_y - dragging_box.start_y;
    } else
    {
      dragging_box.gbox = NULL;
      left_button_released = 0;
    }
    result = true;
  } else
  if (left_button_clicked)
  {
    LbTextSetFont(font_sprites);
    gbox = gui_get_box_point_over(left_button_clicked_x, left_button_clicked_y);
    if (gbox != NULL)
    {
      goptn = gui_get_box_option_point_over(gbox, left_button_clicked_x, left_button_clicked_y);
      if (gbox != hpbox)
      {
        gui_remove_box_from_list(gbox);
        gui_insert_box_at_list_top(gbox);
        left_button_clicked = 0;
      } else
      if (goptn == NULL)
      {
        dragging_box.gbox = hpbox;
        dragging_box.start_x = left_button_clicked_x - gbox->pos_x;
        dragging_box.start_y = left_button_clicked_y - gbox->pos_y;
        left_button_clicked = 0;
      }
      result = true;
    }
  } else
  if (left_button_released)
  {
    gbox = gui_get_box_point_over(left_button_clicked_x, left_button_clicked_y);
    if (gbox != NULL)
    {
      LbTextSetFont(font_sprites);
      goptn = gui_get_box_option_point_over(gbox, left_button_clicked_x, left_button_clicked_y);
      if ((gbox == hpbox) && (goptn != NULL))
      {
        gui_process_option_inputs(hpbox, goptn);
      }
      result = true;
    }
  }
  if (right_button_released)
  {
    gbox = gui_get_box_point_over(left_button_clicked_x, left_button_clicked_y);
    if (gbox != NULL)
    {
      LbTextSetFont(font_sprites);
      goptn = gui_get_box_option_point_over(gbox, left_button_clicked_x, left_button_clicked_y);
      if ((gbox == hpbox) && (goptn != NULL))
      {
        gui_process_option_inputs(hpbox, goptn);
      }
      result = true;
    }
  }
/* These are making incorrect mouse function in possesion - thus disabled
  if (hpbox != NULL)
  {
    if (is_key_pressed(KC_UP,KM_NONE))
    {
      goptn = gui_move_active_box_option(hpbox,-1);
      clear_key_pressed(KC_UP);
      result = true;
    } else
    if (is_key_pressed(KC_DOWN,KM_NONE))
    {
      goptn = gui_move_active_box_option(hpbox,1);
      clear_key_pressed(KC_DOWN);
      result = true;
    }
    if (is_key_pressed(KC_PGUP,KM_NONE))
    {
      goptn = gui_move_active_box_option(hpbox,-2);
      clear_key_pressed(KC_PGUP);
      result = true;
    }
    if (is_key_pressed(KC_PGDOWN,KM_NONE))
    {
      goptn = gui_move_active_box_option(hpbox,2);
      clear_key_pressed(KC_PGDOWN);
      result = true;
    }
  }
*/
  SYNCDBG(9,"Returning %s",result?"true":"false");
  return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
