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
#include "gui_frontmenu.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
// Limits for GUI arrays
#define ACTIVE_BUTTONS_COUNT        86
#define MENU_LIST_ITEMS_COUNT       48
#define FRONTEND_BUTTON_INFO_COUNT 119
#define NET_MESSAGES_COUNT           8
#define NET_MESSAGE_LEN             64
// Sprite limits
#define PANEL_SPRITES_COUNT 514
#define FRONTEND_FONTS_COUNT 4
// After that much milliseconds in main menu, demo is started
#define MNU_DEMO_IDLE_TIME 30000
/******************************************************************************/
#pragma pack(1)

enum DemoItem_Kind {
    DIK_PlaySmkVideo,
    DIK_LoadPacket,
    DIK_SwitchState,
    DIK_ListEnd,
};

enum FrontendMenuStates {
  FeSt_INITIAL = 0,
  FeSt_MAIN_MENU,
  FeSt_FELOAD_GAME,
  FeSt_LAND_VIEW,
  FeSt_NET_SERVICE, /**< Network service selection, wgere player can select Serial/Modem/IPX/TCP IP/1 player. */
  FeSt_NET_SESSION, /**< Network session selection screen, where list of games is displayed, with possibility to join or create own game. */
  FeSt_NET_START, /**< Network game start screen (the menu with chat), when created new session or joined existing session. */
  FeSt_START_KPRLEVEL,
  FeSt_START_MPLEVEL,
  FeSt_UNKNOWN09,
  FeSt_LOAD_GAME, // 10
  FeSt_INTRO,
  FeSt_STORY_POEM,
  FeSt_CREDITS,
  FeSt_DEMO,
  FeSt_NET_MODEM,
  FeSt_NET_SERIAL,
  FeSt_LEVEL_STATS,
  FeSt_HIGH_SCORES,
  FeSt_TORTURE,
  FeSt_UNKNOWN20, // 20
  FeSt_OUTRO,
  FeSt_UNKNOWN22,
  FeSt_UNKNOWN23,
  FeSt_NETLAND_VIEW,
  FeSt_PACKET_DEMO,
  FeSt_FEDEFINE_KEYS,
  FeSt_FEOPTIONS,
  FeSt_UNKNOWN28,
  FeSt_STORY_BIRTHDAY,
  FeSt_LEVEL_SELECT, //30
  FeSt_CAMPAIGN_SELECT,
  FeSt_DRAG,
  FeSt_CAMPAIGN_INTRO,
  FeSt_MAPPACK_SELECT,
  // Special testing states
  FeSt_FONT_TEST          = 255,
};

struct GuiMenu;
struct GuiButton;
struct TbLoadFiles;

struct DemoItem { //sizeof = 5
    unsigned char numfield_0;
    const char *fname;
};

struct NetMessage { // sizeof = 0x41
  unsigned char plyr_idx;
  char text[NET_MESSAGE_LEN];
};

/******************************************************************************/
DLLIMPORT struct GuiButtonInit _DK_event_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_options_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_instance_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_quit_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_load_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_save_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_video_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_sound_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_error_box_buttons[];
DLLIMPORT struct GuiButtonInit _DK_pause_buttons[];
DLLIMPORT struct GuiButtonInit _DK_hold_audience_buttons[];
DLLIMPORT struct GuiButtonInit _DK_armageddon_buttons[];
DLLIMPORT struct GuiButtonInit _DK_dungeon_special_buttons[];
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
DLLIMPORT struct GuiButtonInit _DK_autopilot_menu_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_define_keys_buttons[];
DLLIMPORT struct GuiButtonInit _DK_frontend_option_buttons[];

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
DLLIMPORT unsigned char _DK_video_gamma_correction;
#define video_gamma_correction _DK_video_gamma_correction

// *** SPRITES ***
DLLIMPORT struct TbSprite *_DK_font_sprites;
#define font_sprites _DK_font_sprites
DLLIMPORT struct TbSprite *_DK_end_font_sprites;
#define end_font_sprites _DK_end_font_sprites
DLLIMPORT unsigned char * _DK_font_data;
#define font_data _DK_font_data

DLLIMPORT extern struct TbSprite *_DK_frontend_font[FRONTEND_FONTS_COUNT];
#define frontend_font _DK_frontend_font
DLLIMPORT extern struct TbSprite *_DK_frontend_end_font[FRONTEND_FONTS_COUNT];
#define frontend_end_font _DK_frontend_end_font
DLLIMPORT extern unsigned char * _DK_frontend_font_data[FRONTEND_FONTS_COUNT];
#define frontend_font_data _DK_frontend_font_data
DLLIMPORT extern unsigned char * _DK_frontend_end_font_data[FRONTEND_FONTS_COUNT];
#define frontend_end_font_data _DK_frontend_end_font_data

DLLIMPORT extern struct TbSprite *_DK_button_sprite;
#define button_sprite _DK_button_sprite
DLLIMPORT extern struct TbSprite *_DK_end_button_sprites;
#define end_button_sprites _DK_end_button_sprites
DLLIMPORT extern unsigned char * _DK_button_sprite_data;
#define button_sprite_data _DK_button_sprite_data
DLLIMPORT extern unsigned long _DK_end_button_sprite_data;
#define end_button_sprite_data _DK_end_button_sprite_data

DLLIMPORT extern struct TbSprite *_DK_winfont;
#define winfont _DK_winfont
DLLIMPORT extern struct TbSprite *_DK_end_winfonts;
#define end_winfonts _DK_end_winfonts
DLLIMPORT unsigned char * _DK_winfont_data;
#define winfont_data _DK_winfont_data
DLLIMPORT unsigned char * _DK_end_winfont_data;
#define end_winfont_data _DK_end_winfont_data

DLLIMPORT struct TbSprite *_DK_edit_icon_sprites;
#define edit_icon_sprites _DK_edit_icon_sprites
DLLIMPORT struct TbSprite *_DK_end_edit_icon_sprites;
#define end_edit_icon_sprites _DK_end_edit_icon_sprites
DLLIMPORT unsigned char * _DK_edit_icon_data;
#define edit_icon_data _DK_edit_icon_data

DLLIMPORT extern struct TbSprite *_DK_port_sprite;
#define port_sprite _DK_port_sprite
DLLIMPORT extern struct TbSprite *_DK_end_port_sprites;
#define end_port_sprites _DK_end_port_sprites
DLLIMPORT extern unsigned char * _DK_port_sprite_data;
#define port_sprite_data _DK_port_sprite_data

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

DLLIMPORT extern char _DK_busy_doing_gui;
#define busy_doing_gui _DK_busy_doing_gui
DLLIMPORT extern long _DK_gui_last_left_button_pressed_id;
#define gui_last_left_button_pressed_id _DK_gui_last_left_button_pressed_id
DLLIMPORT extern long _DK_gui_last_right_button_pressed_id;
#define gui_last_right_button_pressed_id _DK_gui_last_right_button_pressed_id
DLLIMPORT int _DK_fe_computer_players;
#define fe_computer_players _DK_fe_computer_players
DLLIMPORT extern long _DK_old_mouse_over_button;
#define old_mouse_over_button _DK_old_mouse_over_button
DLLIMPORT extern long _DK_frontend_mouse_over_button;
#define frontend_mouse_over_button _DK_frontend_mouse_over_button

#pragma pack()
/******************************************************************************/
// Variables - no longer imported
extern struct GuiMenu frontend_main_menu;
extern struct GuiMenu frontend_statistics_menu;
extern struct GuiMenu frontend_high_score_table_menu;
extern struct FrontEndButtonData frontend_button_info[FRONTEND_BUTTON_INFO_COUNT];
extern char gui_message_text[];

extern struct GuiMenu *menu_list[MENU_LIST_ITEMS_COUNT];

extern int status_panel_width;
extern const unsigned long alliance_grid[4][4];

#if (BFDEBUG_LEVEL > 0)
#define TESTFONTS_COUNT 12
extern struct TbSprite *testfont[TESTFONTS_COUNT];
extern struct TbSprite *testfont_end[TESTFONTS_COUNT];
extern unsigned char * testfont_data[TESTFONTS_COUNT];
extern unsigned char *testfont_palette[3];
#endif
/******************************************************************************/
DLLIMPORT char *_DK_mdlf_for_cd(struct TbLoadFiles *);
DLLIMPORT char *_DK_mdlf_default(struct TbLoadFiles *);
/******************************************************************************/
int frontend_font_char_width(int fnt_idx,char c);
int frontend_font_string_width(int fnt_idx, const char *str);
TbBool frontend_font_string_draw(int scr_x, int scr_y, int dst_width, int dst_height, int fnt_idx, const char *str, unsigned short fdflags);

void create_error_box(TextStringId msg_idx);
void create_message_box(const char *title, const char *line1, const char *line2, const char *line3, const char *line4, const char* line5);
void gui_area_text(struct GuiButton *gbtn);
TbBool get_button_area_input(struct GuiButton *gbtn, int a2);
const char *frontend_button_caption_text(const struct GuiButton *gbtn);
int frontend_button_caption_font(const struct GuiButton *gbtn, long mouse_over_btn_idx);
void maintain_loadsave(struct GuiButton *gbtn);
void gui_video_cluedo_maintain(struct GuiButton *gbtn);
void maintain_zoom_to_event(struct GuiButton *gbtn);
void maintain_scroll_up(struct GuiButton *gbtn);
void maintain_scroll_down(struct GuiButton *gbtn);
void frontend_continue_game_maintain(struct GuiButton *gbtn);
void frontend_main_menu_load_game_maintain(struct GuiButton *gbtn);
void frontend_main_menu_netservice_maintain(struct GuiButton *gbtn);
void frontend_main_menu_highscores_maintain(struct GuiButton *gbtn);
void maintain_loadsave(struct GuiButton *gbtn);
void gui_quit_game(struct GuiButton *gbtn);
void gui_area_slider(struct GuiButton *gbtn);
void frontend_draw_icon(struct GuiButton *gbtn);
void frontend_draw_error_text_box(struct GuiButton *gbtn);
void frontend_maintain_error_text_box(struct GuiButton *gbtn);
short is_toggleable_menu(short mnu_idx);

void activate_room_build_mode(RoomKind rkind, TextStringId tooltip_id);
void choose_spell(PowerKind pwkind, TextStringId tooltip_id);
TbBool is_special_power(PowerKind pwkind);
void choose_special_spell(PowerKind pwkind, TextStringId tooltip_id);
void choose_workshop_item(int manufctr_idx, TextStringId tooltip_id);

int frontend_load_data(void);
void frontend_draw_scroll_tab(struct GuiButton *gbtn, long scroll_offset, long first_elem, long last_elem);
long frontend_scroll_tab_to_offset(struct GuiButton *gbtn, long scr_pos, long first_elem, long last_elem);
TbBool frontend_should_all_players_quit(void);
void frontend_init_options_menu(struct GuiMenu *gmnu);
void frontend_draw_text(struct GuiButton *gbtn);
void frontend_change_state(struct GuiButton *gbtn);
void frontend_draw_enter_text(struct GuiButton *gbtn);
void frontend_draw_small_menu_button(struct GuiButton *gbtn);
void frontend_toggle_computer_players(struct GuiButton *gbtn);
void frontend_draw_computer_players(struct GuiButton *gbtn);
void set_packet_start(struct GuiButton *gbtn);
void gui_area_scroll_window(struct GuiButton *gbtn);
void gui_go_to_event(struct GuiButton *gbtn);
void maintain_zoom_to_event(struct GuiButton *gbtn);
void gui_close_objective(struct GuiButton *gbtn);
void gui_scroll_text_up(struct GuiButton *gbtn);
void gui_scroll_text_down(struct GuiButton *gbtn);
void maintain_scroll_up(struct GuiButton *gbtn);
void maintain_scroll_down(struct GuiButton *gbtn);
void gui_scroll_text_down(struct GuiButton *gbtn);
void frontend_ldcampaign_change_state(struct GuiButton *gbtn);
void frontend_netservice_change_state(struct GuiButton *gbtn);
void frontend_start_new_game(struct GuiButton *gbtn);
void frontend_load_mappacks(struct GuiButton *gbtn);
void frontend_load_continue_game(struct GuiButton *gbtn);
short frontend_save_continue_game(short allow_lvnum_grow);
void frontend_continue_game_maintain(struct GuiButton *gbtn);
void frontend_main_menu_load_game_maintain(struct GuiButton *gbtn);
void frontend_mappacks_maintain(struct GuiButton *gbtn);
void frontend_main_menu_netservice_maintain(struct GuiButton *gbtn);
void frontend_main_menu_highscores_maintain(struct GuiButton *gbtn);
void frontend_load_data_from_cd(void);
void frontend_load_data_reset(void);
void init_load_menu(struct GuiMenu *gmnu);
void init_save_menu(struct GuiMenu *gmnu);
void init_video_menu(struct GuiMenu *gmnu);
void init_audio_menu(struct GuiMenu *gmnu);
void frontend_init_options_menu(struct GuiMenu *gmnu);
TbBool frontend_is_player_allied(long idx1, long idx2);
void frontend_set_alliance(long idx1, long idx2);
char update_menu_fade_level(struct GuiMenu *gmnu);
void draw_menu_buttons(struct GuiMenu *gmnu);
MenuNumber create_menu(struct GuiMenu *mnu);
void do_button_release_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
void draw_gui(void);
void init_gui(void);
void reinit_all_menus(void);

void gui_set_autopilot(struct GuiButton *gbtn);

FrontendMenuState frontend_set_state(FrontendMenuState nstate);
FrontendMenuState get_startup_menu_state(void);
FrontendMenuState get_menu_state_when_back_from_substate(FrontendMenuState substate);
void frontend_input(void);
void frontend_update(short *finish_menu);
short frontend_draw(void);
void create_frontend_error_box(long showTime, const char * text);
void try_restore_frontend_error_box(); // Restore error box if frontend state was switched

short menu_is_active(short idx);
TbBool a_menu_window_is_active(void);
void get_player_gui_clicks(void);
short game_is_busy_doing_gui(void);
void set_gui_visible(TbBool visible);
void toggle_gui(void);
void add_message(long plyr_idx, char *msg);
TbBool validate_versions(void);
void versions_different_error(void);
unsigned long toggle_status_menu(short visib);
TbBool toggle_first_person_menu(TbBool visible);
void toggle_gui_overlay_map(void);

void update_player_objectives(PlayerNumber plyr_idx);
void set_level_objective(const char *msg_text);
void display_objectives(PlayerNumber plyr_idx,long x,long y);

short toggle_main_cheat_menu(void);
short toggle_instance_cheat_menu(void);
TbBool open_creature_cheat_menu(void);
TbBool close_creature_cheat_menu(void);
TbBool toggle_creature_cheat_menu(void);
void initialise_tab_tags(MenuID menu_id);
void initialise_tab_tags_and_menu(MenuID menu_id);
void turn_off_roaming_menus(void);

void frontend_set_player_number(long plr_num);
TbBool frontend_start_new_campaign(const char *cmpgn_fname);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
