/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontend.h
 *     Header file for frontend.cpp.
 * @par Purpose:
 *     Functions to display and maintain the game menu.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Nov 2008 - 01 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTEND_H
#define DK_FRONTEND_H

#include "globals.h"
#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
// Limits for GUI arrays
#define ACTIVE_BUTTONS_COUNT        86
#define ACTIVE_MENUS_COUNT           8
#define MENU_LIST_ITEMS_COUNT       43
#define FRONTEND_BUTTON_INFO_COUNT 110
#define NET_MESSAGES_COUNT           8
#define NET_MESSAGE_LEN             64
// Sprite limits
#define PANEL_SPRITES_COUNT 514
#define FRONTEND_FONTS_COUNT 4
#define TORTURE_DOORS_COUNT 9
// Positioning constants for menus
#define POS_AUTO -9999
#define POS_MOUSMID -999
#define POS_MOUSPRV -998
#define POS_SCRCTR  -997
#define POS_SCRBTM  -996
#define POS_GAMECTR  999
// After that much milliseconds in main menu, demo is started
#define MNU_DEMO_IDLE_TIME 30000
/******************************************************************************/
enum DemoItem_Kind {
    DIK_PlaySmkVideo,
    DIK_LoadPacket,
    DIK_SwitchState,
    DIK_ListEnd,
};

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
  GMnu_FENET_MODEM        = 23,
  GMnu_FENET_SERIAL       = 24,
  GMnu_FESTATISTICS       = 25,
  GMnu_FEHIGH_SCORE_TABLE = 26,
  GMnu_DUNGEON_SPECIAL    = 27,
  GMnu_RESURRECT_CREATURE = 28,
  GMnu_TRANSFER_CREATURE  = 29,
  GMnu_ARMAGEDDON         = 30,
  GMnu_CREATURE_QUERY1    = 31,
  GMnu_CREATURE_QUERY3    = 32,
  GMnu_BATTLE             = 34,
  GMnu_CREATURE_QUERY2    = 35,
  GMnu_FEDEFINE_KEYS      = 36,
  GMnu_AUTOPILOT          = 37,
  GMnu_SPELL_LOST         = 38,
  GMnu_FEOPTION           = 39,
  GMnu_FELEVEL_SELECT     = 40,
  GMnu_FECAMPAIGN_SELECT  = 41,
};

enum FrontendMenuState {
  FeSt_MAIN_MENU          =  1,
  FeSt_FELOAD_GAME        =  2,
  FeSt_LAND_VIEW          =  3,
  FeSt_NET_SERVICE        =  4,
  FeSt_NET_SESSION        =  5,
  FeSt_NET_START          =  6,
  FeSt_LOAD_GAME          = 10,
  FeSt_INTRO              = 11,
  FeSt_STORY_POEM         = 12,
  FeSt_CREDITS            = 13,
  FeSt_DEMO               = 14,
  FeSt_NET_MODEM          = 15,
  FeSt_NET_SERIAL         = 16,
  FeSt_LEVEL_STATS        = 17,
  FeSt_HIGH_SCORES        = 18,
  FeSt_TORTURE            = 19,
  FeSt_OUTRO              = 21,
  FeSt_NETLAND_VIEW       = 24,
  FeSt_PACKET_DEMO        = 25,
  FeSt_FEDEFINE_KEYS      = 26,
  FeSt_FEOPTIONS          = 27,
  FeSt_STORY_BIRTHDAY     = 29,
  FeSt_LEVEL_SELECT       = 30,
  FeSt_CAMPAIGN_SELECT    = 31,
  // Special testing states
  FeSt_FONT_TEST          = 255,
};


#ifdef __cplusplus
#pragma pack(1)
#endif

struct GuiMenu;
struct GuiButton;

struct DemoItem { //sizeof = 5
    unsigned char numfield_0;
    const char *fname;
};

struct StatsData { // sizeof = 12
  unsigned long field_0;
  void *field_4;
  void *field_8;
};

struct DoorSoundState { // sizeof = 8
  long field_0;
  long field_4;
};

struct DoorDesc { // sizeof = 44
  long field_0;
  long field_4;
  long pos_x;
  long pos_y;
  long width;
  long height;
  struct TbSprite *sprites;
  struct TbSprite *sprites_end;
  unsigned long data;
  unsigned char *data_end;
  long field_28;
};

struct NetMessage { // sizeof = 0x41
  unsigned char plyr_idx;
  char text[NET_MESSAGE_LEN];
};

typedef long TortureState;

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT struct GuiButtonInit _DK_main_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_room_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_spell_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_spell_lost_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_trap_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_creature_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_event_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_options_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_query_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_instance_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_quit_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_load_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_save_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_video_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_sound_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_error_box_buttons[];
DLLIMPORT struct GuiButtonInit _DK_text_info_buttons[];
DLLIMPORT struct GuiButtonInit _DK_pause_buttons[];
DLLIMPORT struct GuiButtonInit _DK_hold_audience_buttons[];
DLLIMPORT struct GuiButtonInit _DK_armageddon_buttons[];
DLLIMPORT struct GuiButtonInit _DK_dungeon_special_buttons[];
DLLIMPORT struct GuiButtonInit _DK_battle_buttons[];
DLLIMPORT struct GuiButtonInit _DK_resurrect_creature_buttons[];
DLLIMPORT struct GuiButtonInit _DK_transfer_creature_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_main_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_load_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_net_service_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_net_session_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_net_start_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_net_modem_buttons[37];
DLLIMPORT struct GuiButtonInit _DK_frontend_net_serial_buttons[22];
DLLIMPORT struct GuiButtonInit _DK_frontend_statistics_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_high_score_score_buttons[];
DLLIMPORT struct GuiButtonInit _DK_creature_query_buttons1[];
DLLIMPORT struct GuiButtonInit _DK_creature_query_buttons2[];
DLLIMPORT struct GuiButtonInit _DK_creature_query_buttons3[];
DLLIMPORT struct GuiButtonInit _DK_frontend_define_keys_buttons[];
DLLIMPORT struct GuiButtonInit _DK_autopilot_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_option_buttons[];
DLLIMPORT struct GuiMenu _DK_main_menu;
DLLIMPORT struct GuiMenu _DK_room_menu;
DLLIMPORT struct GuiMenu _DK_spell_menu;
DLLIMPORT struct GuiMenu _DK_spell_lost_menu;
DLLIMPORT struct GuiMenu _DK_trap_menu;
DLLIMPORT struct GuiMenu _DK_creature_menu;
DLLIMPORT struct GuiMenu _DK_event_menu;
DLLIMPORT struct GuiMenu _DK_options_menu;
DLLIMPORT struct GuiMenu _DK_instance_menu;
DLLIMPORT struct GuiMenu _DK_query_menu;
DLLIMPORT struct GuiMenu _DK_quit_menu;
DLLIMPORT struct GuiMenu _DK_load_menu;
DLLIMPORT struct GuiMenu _DK_save_menu;
DLLIMPORT struct GuiMenu _DK_video_menu;
DLLIMPORT struct GuiMenu _DK_sound_menu;
DLLIMPORT struct GuiMenu _DK_error_box;
DLLIMPORT struct GuiMenu _DK_text_info_menu;
DLLIMPORT struct GuiMenu _DK_hold_audience_menu;
DLLIMPORT struct GuiMenu _DK_dungeon_special_menu;
DLLIMPORT struct GuiMenu _DK_resurrect_creature_menu;
DLLIMPORT struct GuiMenu _DK_transfer_creature_menu;
DLLIMPORT struct GuiMenu _DK_armageddon_menu;
DLLIMPORT struct GuiMenu _DK_frontend_main_menu;
DLLIMPORT struct GuiMenu _DK_frontend_load_menu;
DLLIMPORT struct GuiMenu _DK_frontend_net_service_menu;
DLLIMPORT struct GuiMenu _DK_frontend_net_session_menu;
DLLIMPORT struct GuiMenu _DK_frontend_net_start_menu;
DLLIMPORT struct GuiMenu _DK_frontend_net_modem_menu;
DLLIMPORT struct GuiMenu _DK_frontend_net_serial_menu;
DLLIMPORT struct GuiMenu _DK_frontend_statistics_menu;
DLLIMPORT struct GuiMenu _DK_frontend_high_score_table_menu;
DLLIMPORT struct GuiMenu _DK_creature_query_menu1;
DLLIMPORT struct GuiMenu _DK_creature_query_menu2;
DLLIMPORT struct GuiMenu _DK_creature_query_menu3;
DLLIMPORT struct GuiMenu _DK_battle_menu;
DLLIMPORT struct GuiMenu _DK_frontend_define_keys_menu;
DLLIMPORT struct GuiMenu _DK_autopilot_menu;
DLLIMPORT struct GuiMenu _DK_frontend_option_menu;
DLLIMPORT struct GuiMenu *_DK_menu_list[40];

DLLIMPORT char _DK_info_tag;
#define info_tag _DK_info_tag
DLLIMPORT char _DK_room_tag;
#define room_tag _DK_room_tag
DLLIMPORT char _DK_spell_tag;
#define spell_tag _DK_spell_tag
DLLIMPORT char _DK_trap_tag;
#define trap_tag _DK_trap_tag
DLLIMPORT char _DK_creature_tag;
#define creature_tag _DK_creature_tag
DLLIMPORT char _DK_input_string[8][16];
#define input_string _DK_input_string
DLLIMPORT char _DK_gui_error_text[256];
#define gui_error_text _DK_gui_error_text
DLLIMPORT long _DK_net_service_scroll_offset;
#define net_service_scroll_offset _DK_net_service_scroll_offset
DLLIMPORT long _DK_net_number_of_services;
#define net_number_of_services _DK_net_number_of_services
DLLIMPORT long _DK_net_comport_index_active;
#define net_comport_index_active _DK_net_comport_index_active
DLLIMPORT long _DK_net_speed_index_active;
#define net_speed_index_active _DK_net_speed_index_active
DLLIMPORT long _DK_net_number_of_players;
#define net_number_of_players _DK_net_number_of_players
DLLIMPORT long _DK_net_number_of_enum_players;
#define net_number_of_enum_players _DK_net_number_of_enum_players
DLLIMPORT long _DK_net_map_slap_frame;
#define net_map_slap_frame _DK_net_map_slap_frame
DLLIMPORT long _DK_net_level_hilighted;
#define net_level_hilighted _DK_net_level_hilighted
DLLIMPORT struct NetMessage _DK_net_message[NET_MESSAGES_COUNT];
#define net_message _DK_net_message
DLLIMPORT long _DK_net_number_of_messages;
#define net_number_of_messages _DK_net_number_of_messages
DLLIMPORT long _DK_net_message_scroll_offset;
#define net_message_scroll_offset _DK_net_message_scroll_offset
DLLIMPORT long _DK_net_session_index_active_id;
#define net_session_index_active_id _DK_net_session_index_active_id
DLLIMPORT long _DK_net_session_scroll_offset;
#define net_session_scroll_offset _DK_net_session_scroll_offset
DLLIMPORT long _DK_net_player_scroll_offset;
#define net_player_scroll_offset _DK_net_player_scroll_offset
DLLIMPORT char _DK_no_of_active_menus;
#define no_of_active_menus _DK_no_of_active_menus
DLLIMPORT unsigned char _DK_menu_stack[ACTIVE_MENUS_COUNT];
#define menu_stack _DK_menu_stack
DLLIMPORT extern struct GuiMenu _DK_active_menus[ACTIVE_MENUS_COUNT];
#define active_menus _DK_active_menus
DLLIMPORT extern struct GuiButton _DK_active_buttons[ACTIVE_BUTTONS_COUNT];
#define active_buttons _DK_active_buttons
DLLIMPORT long _DK_frontend_mouse_over_button_start_time;
#define frontend_mouse_over_button_start_time _DK_frontend_mouse_over_button_start_time
DLLIMPORT short _DK_old_menu_mouse_x;
#define old_menu_mouse_x _DK_old_menu_mouse_x
DLLIMPORT short _DK_old_menu_mouse_y;
#define old_menu_mouse_y _DK_old_menu_mouse_y
DLLIMPORT unsigned char _DK_menu_ids[3];
#define menu_ids _DK_menu_ids
DLLIMPORT unsigned char _DK_new_objective;
#define new_objective _DK_new_objective
DLLIMPORT char _DK_gui_error_text[256];
#define gui_error_text _DK_gui_error_text
DLLIMPORT extern int _DK_frontend_menu_state;
#define frontend_menu_state _DK_frontend_menu_state
DLLIMPORT extern int _DK_load_game_scroll_offset;
#define load_game_scroll_offset _DK_load_game_scroll_offset
DLLIMPORT extern unsigned char *_DK_frontend_background;
#define frontend_background _DK_frontend_background
DLLIMPORT extern long _DK_high_score_entry_input_active;
#define high_score_entry_input_active _DK_high_score_entry_input_active
DLLIMPORT extern long _DK_high_score_entry_index;
#define high_score_entry_index _DK_high_score_entry_index
DLLIMPORT extern char _DK_high_score_entry[64];
#define high_score_entry _DK_high_score_entry
DLLIMPORT unsigned char _DK_video_gamma_correction;
#define video_gamma_correction _DK_video_gamma_correction

// *** SPRITES ***
DLLIMPORT struct TbSprite *_DK_font_sprites;
#define font_sprites _DK_font_sprites
DLLIMPORT struct TbSprite *_DK_end_font_sprites;
#define end_font_sprites _DK_end_font_sprites
DLLIMPORT unsigned long _DK_font_data;
#define font_data _DK_font_data

DLLIMPORT extern struct TbSprite *_DK_frontend_font[FRONTEND_FONTS_COUNT];
#define frontend_font _DK_frontend_font
DLLIMPORT extern struct TbSprite *_DK_frontend_end_font[FRONTEND_FONTS_COUNT];
#define frontend_end_font _DK_frontend_end_font
DLLIMPORT extern unsigned long _DK_frontend_font_data[FRONTEND_FONTS_COUNT];
#define frontend_font_data _DK_frontend_font_data
DLLIMPORT extern unsigned long _DK_frontend_end_font_data[FRONTEND_FONTS_COUNT];
#define frontend_end_font_data _DK_frontend_end_font_data

DLLIMPORT extern struct TbSprite *_DK_frontend_sprite;
#define frontend_sprite _DK_frontend_sprite

DLLIMPORT extern struct TbSprite *_DK_button_sprite;
#define button_sprite _DK_button_sprite
DLLIMPORT extern struct TbSprite *_DK_end_button_sprites;
#define end_button_sprites _DK_end_button_sprites
DLLIMPORT extern unsigned long _DK_button_sprite_data;
#define button_sprite_data _DK_button_sprite_data
DLLIMPORT extern unsigned long _DK_end_button_sprite_data;
#define end_button_sprite_data _DK_end_button_sprite_data

DLLIMPORT extern struct TbSprite *_DK_frontstory_font;
#define frontstory_font _DK_frontstory_font

DLLIMPORT extern struct TbSprite *_DK_winfont;
#define winfont _DK_winfont
DLLIMPORT extern struct TbSprite *_DK_end_winfonts;
#define end_winfonts _DK_end_winfonts
DLLIMPORT unsigned long _DK_winfont_data;
#define winfont_data _DK_winfont_data
DLLIMPORT unsigned long _DK_end_winfont_data;
#define end_winfont_data _DK_end_winfont_data

DLLIMPORT struct TbSprite *_DK_edit_icon_sprites;
#define edit_icon_sprites _DK_edit_icon_sprites
DLLIMPORT struct TbSprite *_DK_end_edit_icon_sprites;
#define end_edit_icon_sprites _DK_end_edit_icon_sprites
DLLIMPORT unsigned long _DK_edit_icon_data;
#define edit_icon_data _DK_edit_icon_data

DLLIMPORT extern struct TbSprite *_DK_port_sprite;
#define port_sprite _DK_port_sprite
DLLIMPORT extern struct TbSprite *_DK_end_port_sprites;
#define end_port_sprites _DK_end_port_sprites
DLLIMPORT extern unsigned long _DK_port_sprite_data;
#define port_sprite_data _DK_port_sprite_data

DLLIMPORT extern struct StatsData _DK_scrolling_stats_data[];
#define scrolling_stats_data _DK_scrolling_stats_data
DLLIMPORT extern struct LevelStats _DK_frontstats_data;
#define frontstats_data _DK_frontstats_data
DLLIMPORT extern TbClockMSec _DK_frontstats_timer;
#define frontstats_timer _DK_frontstats_timer
DLLIMPORT extern unsigned long _DK_playing_bad_descriptive_speech;
#define playing_bad_descriptive_speech _DK_playing_bad_descriptive_speech
DLLIMPORT extern unsigned long _DK_playing_good_descriptive_speech;
#define playing_good_descriptive_speech _DK_playing_good_descriptive_speech
DLLIMPORT extern long _DK_scrolling_index;
#define scrolling_index _DK_scrolling_index
DLLIMPORT extern long _DK_scrolling_offset;
#define scrolling_offset _DK_scrolling_offset
DLLIMPORT extern long _DK_packet_left_button_double_clicked[6];
#define packet_left_button_double_clicked _DK_packet_left_button_double_clicked
DLLIMPORT extern long _DK_packet_left_button_click_space_count[6];
#define packet_left_button_click_space_count _DK_packet_left_button_click_space_count

DLLIMPORT extern char _DK_frontend_alliances;
#define frontend_alliances _DK_frontend_alliances

DLLIMPORT extern long _DK_torture_left_button;
#define torture_left_button _DK_torture_left_button
DLLIMPORT extern long _DK_torture_sprite_direction;
#define torture_sprite_direction _DK_torture_sprite_direction
DLLIMPORT extern long _DK_torture_end_sprite;
#define torture_end_sprite _DK_torture_end_sprite
DLLIMPORT extern long _DK_torture_sprite_frame;
#define torture_sprite_frame _DK_torture_sprite_frame
DLLIMPORT extern long _DK_torture_door_selected;
#define torture_door_selected _DK_torture_door_selected
DLLIMPORT extern struct DoorSoundState _DK_door_sound_state[TORTURE_DOORS_COUNT];
#define door_sound_state _DK_door_sound_state
DLLIMPORT extern struct DoorDesc _DK_doors[TORTURE_DOORS_COUNT];
#define doors _DK_doors
DLLIMPORT extern TortureState _DK_torture_state;
#define torture_state _DK_torture_state
DLLIMPORT extern unsigned char *_DK_torture_background;
#define torture_background _DK_torture_background
DLLIMPORT extern unsigned char *_DK_torture_palette;
#define torture_palette _DK_torture_palette
DLLIMPORT extern struct TbSprite *_DK_fronttor_sprites;
#define fronttor_sprites _DK_fronttor_sprites
DLLIMPORT extern struct TbSprite *_DK_fronttor_end_sprites;
#define fronttor_end_sprites _DK_fronttor_end_sprites
DLLIMPORT extern unsigned long _DK_fronttor_data;
#define fronttor_data _DK_fronttor_data
DLLIMPORT extern unsigned long _DK_fronttor_end_data;
#define fronttor_end_data _DK_fronttor_end_data
/******************************************************************************/
// Variables - no linger imported
extern struct GuiMenu main_menu;
extern struct GuiMenu room_menu;
extern struct GuiMenu spell_menu;
extern struct GuiMenu spell_lost_menu;
extern struct GuiMenu trap_menu;
extern struct GuiMenu creature_menu;
extern struct GuiMenu event_menu;
extern struct GuiMenu options_menu;
extern struct GuiMenu instance_menu;
extern struct GuiMenu query_menu;
extern struct GuiMenu quit_menu;
extern struct GuiMenu load_menu;
extern struct GuiMenu save_menu;
extern struct GuiMenu video_menu;
extern struct GuiMenu sound_menu;
extern struct GuiMenu error_box;
extern struct GuiMenu text_info_menu;
extern struct GuiMenu hold_audience_menu;
extern struct GuiMenu dungeon_special_menu;
extern struct GuiMenu resurrect_creature_menu;
extern struct GuiMenu transfer_creature_menu;
extern struct GuiMenu armageddon_menu;
extern struct GuiMenu frontend_main_menu;
extern struct GuiMenu frontend_load_menu;
extern struct GuiMenu frontend_net_service_menu;
extern struct GuiMenu frontend_net_session_menu;
extern struct GuiMenu frontend_net_start_menu;
extern struct GuiMenu frontend_net_modem_menu;
extern struct GuiMenu frontend_net_serial_menu;
extern struct GuiMenu frontend_statistics_menu;
extern struct GuiMenu frontend_high_score_table_menu;
extern struct GuiMenu creature_query_menu1;
extern struct GuiMenu creature_query_menu2;
extern struct GuiMenu creature_query_menu3;
extern struct GuiMenu battle_menu;
extern struct GuiMenu frontend_define_keys_menu;
extern struct GuiMenu autopilot_menu;
extern struct GuiMenu frontend_option_menu;
extern struct FrontEndButtonData frontend_button_info[FRONTEND_BUTTON_INFO_COUNT];

extern struct GuiMenu *menu_list[MENU_LIST_ITEMS_COUNT];

extern int status_panel_width;

#if (BFDEBUG_LEVEL > 0)
#define TESTFONTS_COUNT 12
extern struct TbSprite *testfont[TESTFONTS_COUNT];
extern struct TbSprite *testfont_end[TESTFONTS_COUNT];
extern unsigned long testfont_data[TESTFONTS_COUNT];
extern unsigned char *testfont_palette[3];
#endif
/******************************************************************************/

// Reworked functions

short check_if_mouse_is_over_button(struct GuiButton *gbtn);
void gui_area_text(struct GuiButton *gbtn);
char get_button_area_input(struct GuiButton *gbtn, int a2);
void menu_tab_maintain(struct GuiButton *gbtn);
void maintain_turn_on_autopilot(struct GuiButton *gbtn);
void maintain_room(struct GuiButton *gbtn);
void maintain_big_room(struct GuiButton *gbtn);
void maintain_spell(struct GuiButton *gbtn);
void maintain_big_spell(struct GuiButton *gbtn);
void maintain_trap(struct GuiButton *gbtn);
void maintain_door(struct GuiButton *gbtn);
void maintain_big_trap(struct GuiButton *gbtn);
void maintain_activity_up(struct GuiButton *gbtn);
void maintain_activity_down(struct GuiButton *gbtn);
void maintain_activity_pic(struct GuiButton *gbtn);
void maintain_activity_row(struct GuiButton *gbtn);
void maintain_loadsave(struct GuiButton *gbtn);
void maintain_prison_bar(struct GuiButton *gbtn);
void maintain_room_and_creature_button(struct GuiButton *gbtn);
void maintain_ally(struct GuiButton *gbtn);
void gui_load_game_maintain(struct GuiButton *gbtn);
void gui_video_cluedo_maintain(struct GuiButton *gbtn);
void maintain_zoom_to_event(struct GuiButton *gbtn);
void maintain_scroll_up(struct GuiButton *gbtn);
void maintain_scroll_down(struct GuiButton *gbtn);
void maintain_resurrect_creature_select(struct GuiButton *gbtn);
void maintain_resurrect_creature_scroll(struct GuiButton *gbtn);
void maintain_transfer_creature_select(struct GuiButton *gbtn);
void maintain_transfer_creature_scroll(struct GuiButton *gbtn);
void frontend_continue_game_maintain(struct GuiButton *gbtn);
void frontend_main_menu_load_game_maintain(struct GuiButton *gbtn);
void frontend_main_menu_netservice_maintain(struct GuiButton *gbtn);
void frontend_main_menu_highscores_maintain(struct GuiButton *gbtn);
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
void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn);
void maintain_instance(struct GuiButton *gbtn);
void frontend_define_key_up_maintain(struct GuiButton *gbtn);
void frontend_define_key_down_maintain(struct GuiButton *gbtn);
void frontend_define_key_maintain(struct GuiButton *gbtn);
void gui_zoom_in(struct GuiButton *gbtn);
void gui_zoom_out(struct GuiButton *gbtn);
void gui_go_to_map(struct GuiButton *gbtn);
void gui_area_new_normal_button(struct GuiButton *gbtn);
void gui_area_autopilot_button(struct GuiButton *gbtn);
void gui_set_menu_mode(struct GuiButton *gbtn);
void gui_draw_tab(struct GuiButton *gbtn);
void gui_open_event(struct GuiButton *gbtn);
void gui_kill_event(struct GuiButton *gbtn);
void gui_area_event_button(struct GuiButton *gbtn);
void gui_choose_room(struct GuiButton *gbtn);
void gui_go_to_next_room(struct GuiButton *gbtn);
void gui_over_room_button(struct GuiButton *gbtn);
void gui_area_room_button(struct GuiButton *gbtn);
void gui_area_new_null_button(struct GuiButton *gbtn);
void gui_area_new_no_anim_button(struct GuiButton *gbtn);
void gui_remove_area_for_rooms(struct GuiButton *gbtn);
void gui_area_big_room_button(struct GuiButton *gbtn);
void gui_choose_spell(struct GuiButton *gbtn);
void gui_go_to_next_spell(struct GuiButton *gbtn);
void gui_area_spell_button(struct GuiButton *gbtn);
void gui_choose_special_spell(struct GuiButton *gbtn);
void gui_area_big_spell_button(struct GuiButton *gbtn);
void gui_choose_trap(struct GuiButton *gbtn);
void gui_go_to_next_trap(struct GuiButton *gbtn);
void gui_over_trap_button(struct GuiButton *gbtn);
void maintain_trap(struct GuiButton *gbtn);
void gui_area_trap_button(struct GuiButton *gbtn);
void gui_go_to_next_door(struct GuiButton *gbtn);
void maintain_door(struct GuiButton *gbtn);
void gui_over_door_button(struct GuiButton *gbtn);
void gui_remove_area_for_traps(struct GuiButton *gbtn);
void gui_area_big_trap_button(struct GuiButton *gbtn);
void maintain_big_trap(struct GuiButton *gbtn);
void gui_area_no_anim_button(struct GuiButton *gbtn);
void maintain_loadsave(struct GuiButton *gbtn);
void gui_area_normal_button(struct GuiButton *gbtn);
void gui_area_new_normal_button(struct GuiButton *gbtn);
void gui_set_tend_to(struct GuiButton *gbtn);
void gui_area_flash_cycle_button(struct GuiButton *gbtn);
void maintain_prison_bar(struct GuiButton *gbtn);
void gui_area_flash_cycle_button(struct GuiButton *gbtn);
void gui_set_query(struct GuiButton *gbtn);
void gui_area_payday_button(struct GuiButton *gbtn);
void gui_area_research_bar(struct GuiButton *gbtn);
void gui_area_workshop_bar(struct GuiButton *gbtn);
void gui_area_player_creature_info(struct GuiButton *gbtn);
void maintain_room_and_creature_button(struct GuiButton *gbtn);
void gui_area_player_room_info(struct GuiButton *gbtn);
void gui_toggle_ally(struct GuiButton *gbtn);
void maintain_ally(struct GuiButton *gbtn);
void gui_quit_game(struct GuiButton *gbtn);
void gui_area_ally(struct GuiButton *gbtn);
void gui_save_game(struct GuiButton *gbtn);
void gui_video_shadows(struct GuiButton *gbtn);
void gui_video_view_distance_level(struct GuiButton *gbtn);
void gui_video_rotate_mode(struct GuiButton *gbtn);
void gui_video_cluedo_mode(struct GuiButton *gbtn);
void gui_video_gamma_correction(struct GuiButton *gbtn);
void gui_set_sound_volume(struct GuiButton *gbtn);
void gui_set_music_volume(struct GuiButton *gbtn);
void gui_video_cluedo_maintain(struct GuiButton *gbtn);
void gui_area_slider(struct GuiButton *gbtn);
void gui_area_smiley_anger_button(struct GuiButton *gbtn);
void gui_area_experience_button(struct GuiButton *gbtn);
void gui_area_instance_button(struct GuiButton *gbtn);
void maintain_instance(struct GuiButton *gbtn);
void gui_area_stat_button(struct GuiButton *gbtn);
void frontend_define_key_up(struct GuiButton *gbtn);
void frontend_define_key_down(struct GuiButton *gbtn);
void frontend_define_key(struct GuiButton *gbtn);
void frontend_define_key_up_maintain(struct GuiButton *gbtn);
void frontend_define_key_down_maintain(struct GuiButton *gbtn);
void frontend_define_key_maintain(struct GuiButton *gbtn);
void frontend_draw_define_key_scroll_tab(struct GuiButton *gbtn);
void frontend_draw_define_key(struct GuiButton *gbtn);
void frontend_draw_icon(struct GuiButton *gbtn);
void frontend_draw_slider(struct GuiButton *gbtn);
void frontend_set_mouse_sensitivity(struct GuiButton *gbtn);
void frontend_draw_small_slider(struct GuiButton *gbtn);
void frontend_invert_mouse(struct GuiButton *gbtn);
void frontend_draw_invert_mouse(struct GuiButton *gbtn);
void frontstats_draw_main_stats(struct GuiButton *gbtn);
void frontstats_draw_scrolling_stats(struct GuiButton *gbtn);
void frontstats_leave(struct GuiButton *gbtn);
void frontend_draw_vlarge_menu_button(struct GuiButton *gbtn);
void frontend_draw_high_score_table(struct GuiButton *gbtn);
void frontend_quit_high_score_table(struct GuiButton *gbtn);
void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn);
short is_toggleable_menu(short mnu_idx);

void pick_up_next_wanderer(struct GuiButton *gbtn);
void gui_go_to_next_wanderer(struct GuiButton *gbtn);
void pick_up_next_worker(struct GuiButton *gbtn);
void gui_go_to_next_worker(struct GuiButton *gbtn);
void pick_up_next_fighter(struct GuiButton *gbtn);
void gui_go_to_next_fighter(struct GuiButton *gbtn);
void gui_scroll_activity_up(struct GuiButton *gbtn);
void gui_scroll_activity_up(struct GuiButton *gbtn);
void gui_scroll_activity_down(struct GuiButton *gbtn);
void gui_scroll_activity_down(struct GuiButton *gbtn);
void maintain_activity_up(struct GuiButton *gbtn);
void maintain_activity_down(struct GuiButton *gbtn);
void maintain_activity_pic(struct GuiButton *gbtn);
void pick_up_next_creature(struct GuiButton *gbtn);
void gui_go_to_next_creature(struct GuiButton *gbtn);
void pick_up_creature_doing_activity(struct GuiButton *gbtn);
void gui_go_to_next_creature_activity(struct GuiButton *gbtn);
void gui_area_anger_button(struct GuiButton *gbtn);
void maintain_activity_row(struct GuiButton *gbtn);

// Campaign selection screen
void frontend_campaign_select_up(struct GuiButton *gbtn);
void frontend_campaign_select_down(struct GuiButton *gbtn);
void frontend_campaign_select_up_maintain(struct GuiButton *gbtn);
void frontend_campaign_select_down_maintain(struct GuiButton *gbtn);
void frontend_campaign_select_maintain(struct GuiButton *gbtn);
void frontend_draw_campaign_select_button(struct GuiButton *gbtn);
void frontend_campaign_select(struct GuiButton *gbtn);
void frontend_campaign_select_update(void);
void frontend_draw_campaign_scroll_tab(struct GuiButton *gbtn);

void gui_activity_background(struct GuiMenu *gmnu);
void gui_pretty_background(struct GuiMenu *gmnu);
void gui_round_glass_background(struct GuiMenu *gmnu);
void gui_creature_query_background1(struct GuiMenu *gmnu);
void gui_creature_query_background2(struct GuiMenu *gmnu);
void reset_scroll_window(struct GuiMenu *gmnu);
void init_load_menu(struct GuiMenu *gmnu);
void init_save_menu(struct GuiMenu *gmnu);
void init_video_menu(struct GuiMenu *gmnu);
void init_audio_menu(struct GuiMenu *gmnu);
int frontend_load_data(void);
void frontend_draw_scroll_tab(struct GuiButton *gbtn, long a2, long a3, long a4);
TbBool frontend_should_all_players_quit(void);
long frontnet_number_of_players_in_session(void);
void frontnet_serial_reset(void);
void frontnet_modem_reset(void);
void fronttorture_unload(void);
void fronttorture_load(void);
void frontnet_service_setup(void);
void frontnet_session_setup(void);
void frontnet_start_setup(void);
void frontnet_modem_setup(void);
void frontnet_serial_setup(void);
void frontstats_set_timer(void);
void frontnet_start_input(void);
TbBool frontend_high_score_table_input(void);
void fronttorture_input(void);
TbBool fronttorture_draw(void);
void frontstats_update(void);
void fronttorture_update(void);
void frontend_init_options_menu(struct GuiMenu *gmnu);
void frontend_draw_large_menu_button(struct GuiButton *gbtn);
void frontnet_draw_scroll_box_tab(struct GuiButton *gbtn);
void frontnet_draw_scroll_box(struct GuiButton *gbtn);
void frontnet_draw_slider_button(struct GuiButton *gbtn);
void frontnet_draw_services_scroll_tab(struct GuiButton *gbtn);
void frontend_draw_text(struct GuiButton *gbtn);
void frontnet_draw_service_button(struct GuiButton *gbtn);
void frontend_draw_large_menu_button(struct GuiButton *gbtn);
void frontnet_service_maintain(struct GuiButton *gbtn);
void frontnet_service_up_maintain(struct GuiButton *gbtn);
void frontnet_service_down_maintain(struct GuiButton *gbtn);
void frontnet_service_up(struct GuiButton *gbtn);
void frontnet_service_down(struct GuiButton *gbtn);
void frontnet_service_select(struct GuiButton *gbtn);
void frontend_change_state(struct GuiButton *gbtn);
void frontend_over_button(struct GuiButton *gbtn);
void frontnet_session_set_player_name(struct GuiButton *gbtn);
void frontnet_draw_text_bar(struct GuiButton *gbtn);
void frontend_draw_enter_text(struct GuiButton *gbtn);
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
void frontnet_session_join(struct GuiButton *gbtn);
void frontnet_session_create(struct GuiButton *gbtn);
void frontnet_return_to_main_menu(struct GuiButton *gbtn);
void frontend_draw_small_menu_button(struct GuiButton *gbtn);
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
void frontend_toggle_computer_players(struct GuiButton *gbtn);
void frontend_draw_computer_players(struct GuiButton *gbtn);
void frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn);
void frontnet_draw_current_message(struct GuiButton *gbtn);
void frontnet_draw_messages(struct GuiButton *gbtn);
void set_packet_start(struct GuiButton *gbtn);
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
void gui_load_game(struct GuiButton *gbtn);
void gui_load_game_maintain(struct GuiButton *gbtn);
void gui_area_scroll_window(struct GuiButton *gbtn);
void gui_go_to_event(struct GuiButton *gbtn);
void maintain_zoom_to_event(struct GuiButton *gbtn);
void gui_close_objective(struct GuiButton *gbtn);
void gui_scroll_text_up(struct GuiButton *gbtn);
void gui_scroll_text_down(struct GuiButton *gbtn);
void maintain_scroll_up(struct GuiButton *gbtn);
void maintain_scroll_down(struct GuiButton *gbtn);
void gui_scroll_text_down(struct GuiButton *gbtn);
void choose_hold_audience(struct GuiButton *gbtn);
void choose_armageddon(struct GuiButton *gbtn);
void frontend_draw_levels_scroll_tab(struct GuiButton *gbtn);
void frontend_draw_level_select_button(struct GuiButton *gbtn);
void frontend_level_select(struct GuiButton *gbtn);
void frontend_level_select_up(struct GuiButton *gbtn);
void frontend_level_select_down(struct GuiButton *gbtn);
void frontend_level_select_up_maintain(struct GuiButton *gbtn);
void frontend_level_select_down_maintain(struct GuiButton *gbtn);
void frontend_level_select_maintain(struct GuiButton *gbtn);
void frontend_load_game_up(struct GuiButton *gbtn);
void frontend_load_game_down(struct GuiButton *gbtn);
void frontend_load_game_up_maintain(struct GuiButton *gbtn);
void frontend_load_game_down_maintain(struct GuiButton *gbtn);
void frontend_load_game_maintain(struct GuiButton *gbtn);
void frontend_draw_games_scroll_tab(struct GuiButton *gbtn);
void frontend_load_game(struct GuiButton *gbtn);
void frontend_draw_load_game_button(struct GuiButton *gbtn);
void frontend_ldcampaign_change_state(struct GuiButton *gbtn);
void frontend_netservice_change_state(struct GuiButton *gbtn);
void frontend_start_new_game(struct GuiButton *gbtn);
void frontend_load_continue_game(struct GuiButton *gbtn);
short frontend_save_continue_game(short allow_lvnum_grow);
void frontend_continue_game_maintain(struct GuiButton *gbtn);
void frontend_main_menu_load_game_maintain(struct GuiButton *gbtn);
void frontend_main_menu_netservice_maintain(struct GuiButton *gbtn);
void frontend_main_menu_highscores_maintain(struct GuiButton *gbtn);
void gui_previous_battle(struct GuiButton *gbtn);
void gui_next_battle(struct GuiButton *gbtn);
void gui_get_creature_in_battle(struct GuiButton *gbtn);
void gui_go_to_person_in_battle(struct GuiButton *gbtn);
void gui_setup_friend_over(struct GuiButton *gbtn);
void gui_area_friendly_battlers(struct GuiButton *gbtn);
void gui_setup_enemy_over(struct GuiButton *gbtn);
void gui_area_enemy_battlers(struct GuiButton *gbtn);
void select_resurrect_creature(struct GuiButton *gbtn);
void maintain_resurrect_creature_select(struct GuiButton *gbtn);
void draw_resurrect_creature(struct GuiButton *gbtn);
void select_resurrect_creature_up(struct GuiButton *gbtn);
void select_resurrect_creature_down(struct GuiButton *gbtn);
void maintain_resurrect_creature_scroll(struct GuiButton *gbtn);
void select_transfer_creature(struct GuiButton *gbtn);
void draw_transfer_creature(struct GuiButton *gbtn);
void maintain_transfer_creature_select(struct GuiButton *gbtn);
void select_transfer_creature_up(struct GuiButton *gbtn);
void select_transfer_creature_down(struct GuiButton *gbtn);
void maintain_transfer_creature_scroll(struct GuiButton *gbtn);
void frontend_load_data_from_cd(void);
void frontend_load_data_reset(void);
void gui_area_null(struct GuiButton *gbtn);
void draw_load_button(struct GuiButton *gbtn);
void gui_activity_background(struct GuiMenu *gmnu);
void gui_pretty_background(struct GuiMenu *gmnu);
void frontend_draw_large_menu_button(struct GuiButton *gbtn);
void frontend_copy_mnu_background(struct GuiMenu *gmnu);
void frontend_copy_background(void);
void frontend_copy_background_at(int rect_x,int rect_y,int rect_w,int rect_h);
void gui_round_glass_background(struct GuiMenu *gmnu);
void gui_creature_query_background1(struct GuiMenu *gmnu);
void gui_creature_query_background2(struct GuiMenu *gmnu);
void maintain_event_button(struct GuiButton *gbtn);
void init_load_menu(struct GuiMenu *gmnu);
void init_save_menu(struct GuiMenu *gmnu);
void init_video_menu(struct GuiMenu *gmnu);
void init_audio_menu(struct GuiMenu *gmnu);
void frontend_init_options_menu(struct GuiMenu *gmnu);
void frontnet_service_select(struct GuiButton *gbtn);
void frontnet_service_up_maintain(struct GuiButton *gbtn);
void frontnet_service_down_maintain(struct GuiButton *gbtn);
void frontnet_service_up(struct GuiButton *gbtn);
void frontnet_service_down(struct GuiButton *gbtn);
void frontnet_service_maintain(struct GuiButton *gbtn);
void frontnet_draw_service_button(struct GuiButton *gbtn);
TbBool frontend_is_player_allied(long idx1, long idx2);
void frontend_set_alliance(long idx1, long idx2);
long menu_id_to_number(short menu_id);
char update_menu_fade_level(struct GuiMenu *gmnu);
void draw_menu_buttons(struct GuiMenu *gmnu);
char create_menu(struct GuiMenu *mnu);
void do_button_release_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
void draw_gui(void);
void init_gui(void);
void reinit_all_menus(void);
void kill_button_area_input(void);
void kill_menu(struct GuiMenu *gmnu);

void spell_lost_first_person(struct GuiButton *gbtn);
void gui_turn_on_autopilot(struct GuiButton *gbtn);
void gui_set_autopilot(struct GuiButton *gbtn);

int frontend_set_state(long nstate);
void frontstats_initialise(void);
int get_startup_menu_state(void);
void frontend_input(void);
void turn_on_menu(short idx);
void turn_off_menu(short mnu_idx);
void turn_off_query_menus(void);
void turn_off_all_menus(void);
short turn_off_all_window_menus(void);
short turn_off_all_bottom_menus(void);
void turn_on_main_panel_menu(void);
void turn_off_all_panel_menus(void);
void set_menu_mode(long mnu_idx);
void frontend_update(short *finish_menu);
short frontend_draw(void);
int frontend_font_char_width(int fnt_idx,char c);
int frontend_font_string_width(int fnt_idx,char *str);
short menu_is_active(short idx);
TbBool a_menu_window_is_active(void);
struct GuiMenu *get_active_menu(int id);
void turn_on_event_info_panel_if_necessary(unsigned short evnt_idx);
void get_player_gui_clicks(void);
short game_is_busy_doing_gui(void);
void turn_off_event_box_if_necessary(long plridx, char val);
void set_gui_visible(short visible);
void toggle_gui(void);
void add_message(long plyr_idx, char *msg);
TbBool validate_versions(void);
void versions_different_error(void);
void fake_button_click(long btn_idx);
unsigned long toggle_status_menu(short visib);
TbBool toggle_first_person_menu(TbBool visible);
void toggle_gui_overlay_map(void);
void display_objectives(long a1,long a2,long a3);
short toggle_main_cheat_menu(void);
short toggle_instance_cheat_menu(void);
TbBool open_creature_cheat_menu(void);
TbBool close_creature_cheat_menu(void);
TbBool toggle_creature_cheat_menu(void);
void initialise_tab_tags(long menu_id);
void initialise_tab_tags_and_menu(long menu_id);
void turn_off_roaming_menus(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
