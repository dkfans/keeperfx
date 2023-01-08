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
  FeSt_UNUSED1,
  FeSt_UNUSED2,
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
extern char info_tag;
extern char room_tag;
extern char spell_tag;
extern char trap_tag;
extern char creature_tag;
extern char input_string[8][16];
extern char gui_error_text[256];
extern long net_service_scroll_offset;
extern long net_number_of_services;
extern long net_number_of_players;
extern long net_number_of_enum_players;
extern long net_map_slap_frame;
extern long net_level_hilighted;
extern struct NetMessage net_message[NET_MESSAGES_COUNT];
extern long net_number_of_messages;
extern long net_message_scroll_offset;
extern long net_session_index_active_id;
extern long net_session_scroll_offset;
extern long net_player_scroll_offset;
extern struct GuiButton active_buttons[ACTIVE_BUTTONS_COUNT];
extern long frontend_mouse_over_button_start_time;
extern short old_menu_mouse_x;
extern short old_menu_mouse_y;
extern unsigned char menu_ids[3];
extern unsigned char new_objective;
extern int frontend_menu_state;
extern int load_game_scroll_offset;
extern unsigned char video_gamma_correction;

// *** SPRITES ***
extern struct TbSprite *font_sprites;
extern struct TbSprite *end_font_sprites;
extern unsigned char * font_data;
extern struct TbSprite *frontend_font[FRONTEND_FONTS_COUNT];
extern struct TbSprite *frontend_end_font[FRONTEND_FONTS_COUNT];
extern unsigned char * frontend_font_data[FRONTEND_FONTS_COUNT];
extern unsigned char * frontend_end_font_data[FRONTEND_FONTS_COUNT];
extern struct TbSprite *button_sprite;
extern struct TbSprite *end_button_sprites;
extern unsigned char * button_sprite_data;
extern unsigned long end_button_sprite_data;
extern struct TbSprite *winfont;
extern struct TbSprite *end_winfonts;
extern unsigned char * winfont_data;
extern unsigned char * end_winfont_data;
extern struct TbSprite *edit_icon_sprites;
extern struct TbSprite *end_edit_icon_sprites;
extern unsigned char * edit_icon_data;
extern struct TbSprite *port_sprite;
extern struct TbSprite *end_port_sprites;
extern unsigned char * port_sprite_data;
extern unsigned long playing_bad_descriptive_speech;
extern unsigned long playing_good_descriptive_speech;
extern long scrolling_index;
extern long scrolling_offset;
extern long packet_left_button_double_clicked[6];
extern long packet_left_button_click_space_count[6];
extern char frontend_alliances;
extern char busy_doing_gui;
extern long gui_last_left_button_pressed_id;
extern long gui_last_right_button_pressed_id;
extern int fe_computer_players;
extern long old_mouse_over_button;
extern long frontend_mouse_over_button;

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
extern char *mdlf_default(struct TbLoadFiles *);
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
TbBool close_main_cheat_menu(void);
short toggle_instance_cheat_menu(void);
TbBool close_instance_cheat_menu(void);
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
