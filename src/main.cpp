
#include <windows.h>
#include <winbase.h>
#include <math.h>
#include "keeperfx.h"

#include "bflib_dernc.h"
#include "bflib_pom.h"
#include "bflib_memory.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_sprite.h"
#include "bflib_fileio.h"
#include "bflib_sndlib.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"

#define CMDLN_MAXLEN 259
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];
unsigned char palette_buf[768];

char window_class_name[128]="Bullfrog Shell";

#define key_modifiers _DK_key_modifiers
#define my_player_number _DK_my_player_number
#define lbFontPtr _DK_lbFontPtr
#define winfont _DK_winfont
#define input_button _DK_input_button
#define game _DK_game
#define input_string _DK_input_string
#define error_box_message _DK_error_box_message
#define pointer_x _DK_pointer_x
#define pointer_y _DK_pointer_y
#define top_pointed_at_x _DK_top_pointed_at_x
#define top_pointed_at_y _DK_top_pointed_at_y
#define block_pointed_at_x _DK_block_pointed_at_x
#define block_pointed_at_y _DK_block_pointed_at_y
#define top_pointed_at_frac_x _DK_top_pointed_at_frac_x
#define top_pointed_at_frac_y _DK_top_pointed_at_frac_y
#define pointed_at_frac_x _DK_pointed_at_frac_x
#define pointed_at_frac_y _DK_pointed_at_frac_y
#define activity_list _DK_activity_list
#define load_game_scroll_offset _DK_load_game_scroll_offset
#define number_of_saved_games _DK_number_of_saved_games
#define load_game_scroll_offset _DK_load_game_scroll_offset
#define save_game_catalogue _DK_save_game_catalogue
#define datafiles_path _DK_datafiles_path
#define exit_keeper _DK_exit_keeper
#define quit_game _DK_quit_game
#define net_service_scroll_offset _DK_net_service_scroll_offset
#define net_number_of_services _DK_net_number_of_services
#define frontend_mouse_over_button _DK_frontend_mouse_over_button
#define frontend_font _DK_frontend_font
#define net_service _DK_net_service

struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE", &_DK_palette, NULL, PALETTE_SIZE, 0, 0},
    {"*SCRATCH", &_DK_scratch, NULL, 0x10000, 1, 0},
    {"", NULL, NULL, 0, 0, 0}, };

unsigned char *nocd_raw;
unsigned char *nocd_pal;

struct TbLoadFiles nocd_load_files[] = {
    {"data/nocd.raw", &nocd_raw, NULL, 0, 0, 0},
    {"data/nocd.pal", &nocd_pal, NULL, 0, 0, 0},
    {"", NULL, NULL, 0, 0, 0}, };

long gf_change_player_state(struct GuiBox *gbox, struct GuiBoxOption *goptn, char, long *tag)
{}

struct GuiBoxOption gui_main_cheat_list[] = { //gui_main_option_list in beta
    {"Null mode",              1, 0, gf_change_player_state, 0, 0, 0,  0, 0, 0, 0},
    {"Place tunneller mode",   1, 0, gf_change_player_state, 0, 0, 0,  3, 0, 0, 0},
    {"Place creature mode",    1, 0, gf_change_player_state, 0, 0, 0, 14, 0, 0, 0},
    {"Place hero mode",        1, 0, gf_change_player_state, 0, 0, 0,  4, 0, 0, 0},
    {"Destroy walls mode",     1, 0, gf_change_player_state, 0, 0, 0, 25, 0, 0, 0},
    {"Disease mode",           1, 0, gf_change_player_state, 0, 0, 0, 26, 0, 0, 0},
    {"Peter mode",	           1, 0, gf_change_player_state, 0, 0, 0, 27, 0, 0, 0},
    {"",                       2, 0,                   NULL, 0, 0, 0,  0, 0, 0, 0},
    {"Passenger control mode", 1, 0, gf_change_player_state, 0, 0, 0, 10, 0, 0, 0},
    {"Direct control mode",    1, 0, gf_change_player_state, 0, 0, 0, 11, 0, 0, 0},
    {"Order creature mode",    1, 0, gf_change_player_state, 0, 0, 0, 13, 0, 0, 0},
    {"",                       2, 0,                   NULL, 0,	0, 0,  0, 0, 0, 0},
    {"!",                      0, 0,                   NULL, 0, 0, 0,  0, 0, 0, 0},
};

struct GuiButtonInit main_menu_buttons[] = {
  { 0, 38, 0, 0, 0, _DK_gui_zoom_in,    NULL,        NULL,               0, 110,   4, 114,   4, 26, 64, _DK_gui_area_new_normal_button,  237, 321,  0,       0,            0, 0, NULL },
  { 0, 39, 0, 0, 0, _DK_gui_zoom_out,   NULL,        NULL,               0, 110,  70, 114,  70, 26, 64, _DK_gui_area_new_normal_button,  239, 322,  0,       0,            0, 0, NULL },
  { 0, 37, 0, 0, 0, _DK_gui_go_to_map,  NULL,        NULL,               0,   0,   0,   0,   0, 30, 30, _DK_gui_area_new_normal_button,  304, 323,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_gui_turn_on_autopilot,NULL,  NULL,               0,   0,  70,   0,  70, 16, 68, _DK_gui_area_autopilot_button,   492, 201,  0,       0,            0, 0, _DK_maintain_turn_on_autopilot },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  68,   0,  68,   0, 68, 16, _DK_gui_area_new_normal_button,  499, 722,&options_menu, 0,        0, 0, NULL },
  { 3,  1, 0, 0, 0, _DK_gui_set_menu_mode,NULL,      NULL,               7,   0, 154,   0, 154, 28, 34, _DK_gui_draw_tab,                  7, 447,  0,(long)&_DK_info_tag, 0, 0, _DK_menu_tab_maintain },
  { 3,  2, 0, 0, 0, _DK_gui_set_menu_mode,NULL,      NULL,               2,  28, 154,  28, 154, 28, 34, _DK_gui_draw_tab,                  9, 448,  0,(long)&_DK_room_tag, 0, 0, _DK_menu_tab_maintain },
  { 3,  3, 0, 0, 0, _DK_gui_set_menu_mode,NULL,      NULL,               3,  56, 154,  56, 154, 28, 34, _DK_gui_draw_tab,                 11, 449,  0,(long)&_DK_spell_tag,0, 0, _DK_menu_tab_maintain },
  { 3,  4, 0, 0, 0, _DK_gui_set_menu_mode,NULL,      NULL,               4,  84, 154,  84, 154, 28, 34, _DK_gui_draw_tab,                 13, 450,  0,(long)&_DK_trap_tag, 0, 0, _DK_menu_tab_maintain },
  { 3,  5, 0, 0, 0, _DK_gui_set_menu_mode,NULL,      NULL,               5, 112, 154, 112, 154, 28, 34, _DK_gui_draw_tab,                 15, 451,  0,(long)&_DK_creature_tag,0,0,_DK_menu_tab_maintain },
  { 0, 40, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 360, 138, 360, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       0,            0, 0, _DK_maintain_event_button },
  { 0, 41, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 330, 138, 330, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       1,            0, 0, _DK_maintain_event_button },
  { 0, 42, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 300, 138, 300, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       2,            0, 0, _DK_maintain_event_button },
  { 0, 43, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 270, 138, 270, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       3,            0, 0, _DK_maintain_event_button },
  { 0, 44, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 240, 138, 240, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       4,            0, 0, _DK_maintain_event_button },
  { 0, 45, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 210, 138, 210, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       5,            0, 0, _DK_maintain_event_button },
  { 0, 46, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 180, 138, 180, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       6,            0, 0, _DK_maintain_event_button },
  { 0, 47, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 150, 138, 150, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       7,            0, 0, _DK_maintain_event_button },
  { 0, 48, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138, 120, 138, 120, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       8,            0, 0, _DK_maintain_event_button },
  { 0, 49, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138,  90, 138,  90, 24, 30, _DK_gui_area_event_button,         0, 201,  0,       9,            0, 0, _DK_maintain_event_button },
  { 0, 50, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138,  60, 138,  60, 24, 30, _DK_gui_area_event_button,         0, 201,  0,      10,            0, 0, _DK_maintain_event_button },
  { 0, 51, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138,  30, 138,  30, 24, 30, _DK_gui_area_event_button,         0, 201,  0,      11,            0, 0, _DK_maintain_event_button },
  { 0, 52, 0, 0, 0, _DK_gui_open_event, _DK_gui_kill_event,NULL,         0, 138,   0, 138,   0, 24, 30, _DK_gui_area_event_button,         0, 201,  0,      12,            0, 0, _DK_maintain_event_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  22, 122,  22, 122, 94, 40, NULL,                              0, 441,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit room_menu_buttons[] = {
  { 0,  6, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0, 2,238,  6,242,32,36,_DK_gui_area_room_button, 57, 615,  0,       2,            0, 0, _DK_maintain_room },
  { 0,  8, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,34,238, 38,242,32,36,_DK_gui_area_room_button, 79, 625,  0,      14,            0, 0, _DK_maintain_room },
  { 0,  7, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,66,238, 70,242,32,36,_DK_gui_area_room_button, 59, 624,  0,      13,            0, 0, _DK_maintain_room },
  { 0, 10, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,98,238,102,242,32,36,_DK_gui_area_room_button, 67, 618,  0,       6,            0, 0, _DK_maintain_room },
  { 0,  9, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0, 2,276,  6,280,32,36,_DK_gui_area_room_button, 61, 616,  0,       3,            0, 0, _DK_maintain_room },
  { 0, 18, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,34,276, 38,280,32,36,_DK_gui_area_room_button, 81, 626,  0,      15,            0, 0, _DK_maintain_room },
  { 0, 19, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,66,276, 70,280,32,36,_DK_gui_area_room_button, 83, 627,  0,      16,            0, 0, _DK_maintain_room },
  { 0, 13, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,98,276,102,280,32,36,_DK_gui_area_room_button, 75, 621,  0,       8,            0, 0, _DK_maintain_room },
  { 0, 11, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0, 2,314,  6,318,32,36,_DK_gui_area_room_button, 65, 617,  0,       4,            0, 0, _DK_maintain_room },
  { 0, 17, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,34,314, 38,318,32,36,_DK_gui_area_room_button, 63, 619,  0,       5,            0, 0, _DK_maintain_room },
  { 0, 16, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,66,314, 70,318,32,36,_DK_gui_area_room_button, 69, 623,  0,      12,            0, 0, _DK_maintain_room },
  { 0, 12, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,98,314,102,318,32,36,_DK_gui_area_room_button, 73, 628,  0,      10,            0, 0, _DK_maintain_room },
  { 0, 15, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0, 2,352,  6,356,32,36,_DK_gui_area_room_button, 71, 622,  0,      11,            0, 0, _DK_maintain_room },
  { 0, 14, 0, 0, 0, _DK_gui_choose_room,_DK_gui_go_to_next_room,_DK_gui_over_room_button,0,34,352, 38,356,32,36,_DK_gui_area_room_button, 77, 629,  0,       9,            0, 0, _DK_maintain_room },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356,  32, 36, _DK_gui_area_new_null_button,    24, 201,  0,       0,            0, 0, _DK_maintain_room },
  { 0, 20, 0, 0, 0, _DK_gui_remove_area_for_rooms,NULL,NULL,             0,  98, 352, 102, 356,  32, 36, _DK_gui_area_new_no_anim_button,107, 462,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,  46, 44, _DK_gui_area_big_room_button,     0, 201,  0,       0,            0, 0, _DK_maintain_big_room },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit spell_menu_buttons[] = {
  { 0, 36, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,   2, 238,   6, 242, 32, 36, _DK_gui_area_spell_button,       114, 647,  0,      18,            0, 0, _DK_maintain_spell },
  { 0, 21, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  34, 238,  38, 242, 32, 36, _DK_gui_area_spell_button,       118, 648,  0,       2,            0, 0, _DK_maintain_spell },
  { 0, 22, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  66, 238,  70, 242, 32, 36, _DK_gui_area_spell_button,       108, 649,  0,       5,            0, 0, _DK_maintain_spell },
  { 0, 27, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  98, 238, 102, 242, 32, 36, _DK_gui_area_spell_button,       122, 654,  0,      11,            0, 0, _DK_maintain_spell },
  { 0, 35, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,   2, 276,   6, 280, 32, 36, _DK_gui_area_spell_button,       452, 653,  0,       3,            0, 0, _DK_maintain_spell },
  { 0, 23, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  34, 276,  38, 280, 32, 36, _DK_gui_area_spell_button,       116, 650,  0,       6,            0, 0, _DK_maintain_spell },
  { 0, 29, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  66, 276,  70, 280, 32, 36, _DK_gui_area_spell_button,       128, 656,  0,      13,            0, 0, _DK_maintain_spell },
  { 0, 34, 0, 0, 0, _DK_gui_choose_special_spell,NULL,NULL,              0,  98, 276, 102, 280, 32, 36, _DK_gui_area_spell_button,       112, 651,  0,       9,            0, 0, _DK_maintain_spell },
  { 0, 24, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,   2, 314,   6, 318, 32, 36, _DK_gui_area_spell_button,       120, 652,  0,       7,            0, 0, _DK_maintain_spell },
  { 0, 26, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  34, 314,  38, 318, 32, 36, _DK_gui_area_spell_button,       110, 661,  0,       8,            0, 0, _DK_maintain_spell },
  { 0, 25, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  66, 314,  70, 318, 32, 36, _DK_gui_area_spell_button,       124, 657,  0,      10,            0, 0, _DK_maintain_spell },
  { 0, 28, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  98, 314, 102, 318, 32, 36, _DK_gui_area_spell_button,       126, 655,  0,      12,            0, 0, _DK_maintain_spell },
  { 0, 30, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,   2, 352,   6, 356, 32, 36, _DK_gui_area_spell_button,       314, 658,  0,      15,            0, 0, _DK_maintain_spell },
  { 0, 31, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  34, 352,  38, 356, 32, 36, _DK_gui_area_spell_button,       319, 659,  0,      14,            0, 0, _DK_maintain_spell },
  { 0, 33, 0, 0, 0, _DK_gui_choose_special_spell,NULL,NULL,              0,  66, 352,  70, 356, 32, 36, _DK_gui_area_spell_button,       321, 663,  0,      19,            0, 0, _DK_maintain_spell },
  { 0, 32, 0, 0, 0, _DK_gui_choose_spell,_DK_gui_go_to_next_spell,NULL,  0,  98, 352, 102, 356, 32, 36, _DK_gui_area_spell_button,       317, 660,  0,      16,            0, 0, _DK_maintain_spell },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194, 46, 44, _DK_gui_area_big_spell_button,     0, 201,  0,       0,            0, 0, _DK_maintain_big_spell },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit spell_lost_menu_buttons[] = {
  { 0, 36, 0, 0, 0, _DK_spell_lost_first_person,NULL,NULL,               0,   2, 238,   8, 250, 24, 24, _DK_gui_area_new_null_button,     114,647,  0,      18,            0, 0, _DK_maintain_spell },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 238,  40, 250, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 238,  72, 250, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 238, 104, 250, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 276,   8, 288, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 276,  40, 288, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  72, 288, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 104, 288, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 314,   8, 326, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 314,  40, 326, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 314,  72, 326, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 314, 104, 326, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   8, 364, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  40, 364, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  72, 364, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 352, 104, 364, 24, 24, _DK_gui_area_new_null_button,      24,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194, 46, 44, _DK_gui_area_big_spell_button,     0, 201,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit trap_menu_buttons[] = {
  { 0, 54, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_trap,_DK_gui_over_trap_button,0, 2,238, 6,242,32,36,_DK_gui_area_trap_button, 154, 585,  0,       2,            0, 0, _DK_maintain_trap },
  { 0, 55, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_trap,_DK_gui_over_trap_button,0,34,238,38,242,32,36,_DK_gui_area_trap_button, 156, 586,  0,       3,            0, 0, _DK_maintain_trap },
  { 0, 56, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_trap,_DK_gui_over_trap_button,0,66,238,70,242,32,36,_DK_gui_area_trap_button, 158, 587,  0,       4,            0, 0, _DK_maintain_trap },
  { 0, 67, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_trap,_DK_gui_over_trap_button,0,98,238,102,242,32,36,_DK_gui_area_trap_button,162, 589,  0,       6,            0, 0, _DK_maintain_trap },
  { 0, 53, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_trap,_DK_gui_over_trap_button,0, 2,276, 6,280,32,36,_DK_gui_area_trap_button, 152, 584,  0,       1,            0, 0, _DK_maintain_trap },
  { 0, 57, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_trap,_DK_gui_over_trap_button,0,34,276,38,280,32,36,_DK_gui_area_trap_button, 160, 588,  0,       5,            0, 0, _DK_maintain_trap },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  70, 280, 32, 36, _DK_gui_area_trap_button,         24, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 102, 280, 32, 36, _DK_gui_area_trap_button,         24, 201,  0,       0,            0, 0, NULL },
  { 0, 58, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_door,_DK_gui_over_door_button,0, 2,314, 6,318,32,36,_DK_gui_area_trap_button, 166, 594,  0,       7,            0, 0, _DK_maintain_door },
  { 0, 59, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_door,_DK_gui_over_door_button,0,34,314,38,318,32,36,_DK_gui_area_trap_button, 168, 595,  0,       8,            0, 0, _DK_maintain_door },
  { 0, 60, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_door,_DK_gui_over_door_button,0,66,314,70,318,32,36,_DK_gui_area_trap_button, 170, 596,  0,       9,            0, 0, _DK_maintain_door },
  { 0, 61, 0, 0, 0, _DK_gui_choose_trap,_DK_gui_go_to_next_door,_DK_gui_over_door_button,0,98,314,102,318,32,36,_DK_gui_area_trap_button,172, 597,  0,      10,            0, 0, _DK_maintain_door },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   6, 356, 32, 36, _DK_gui_area_new_null_button,     24, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  38, 356, 32, 36, _DK_gui_area_new_null_button,     24, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, _DK_gui_area_new_null_button,     24, 201,  0,       0,            0, 0, NULL },
  { 0, 62, 0, 0, 0, _DK_gui_remove_area_for_traps,NULL,NULL,             0,  98, 352, 102, 356, 32, 36, _DK_gui_area_new_no_anim_button, 107, 463,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194, 46, 44, _DK_gui_area_big_trap_button,      0, 201,  0,       0,            0, 0, _DK_maintain_big_trap },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit creature_menu_buttons[] = {
  { 0, 72, 0, 0, 0, _DK_pick_up_next_wanderer,_DK_gui_go_to_next_wanderer,NULL,0,26,192,26,192, 38, 24, _DK_gui_area_new_normal_button,  284, 302,  0,       0,            0, 0, NULL },
  { 0, 73, 0, 0, 0, _DK_pick_up_next_worker,_DK_gui_go_to_next_worker,NULL,0,62, 192,  62, 192, 38, 24, _DK_gui_area_new_normal_button,  282, 303,  0,       0,            0, 0, NULL },
  { 0, 74, 0, 0, 0, _DK_pick_up_next_fighter,_DK_gui_go_to_next_fighter,NULL,0,98,192, 98, 192, 38, 24, _DK_gui_area_new_normal_button,  286, 304,  0,       0,            0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_gui_scroll_activity_up,_DK_gui_scroll_activity_up,NULL,0,4,192, 4, 192, 22, 24, _DK_gui_area_new_normal_button,  278, 201,  0,       0,            0, 0, _DK_maintain_activity_up },
  { 1,  0, 0, 0, 0, _DK_gui_scroll_activity_down,_DK_gui_scroll_activity_down,NULL,0,4,364,4,364,22,24, _DK_gui_area_new_normal_button,  280, 201,  0,       0,            0, 0, _DK_maintain_activity_down },
  { 0,  0, 0, 0, 0, _DK_pick_up_next_creature,_DK_gui_go_to_next_creature,NULL,0,0,196, 0, 218, 22, 22, _DK_gui_area_new_no_anim_button,   0, 733,  0,       0,            0, 0, _DK_maintain_activity_pic },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,0,26,220,26,220,32,20,_DK_gui_area_anger_button,288,734,0,(long)&activity_list[0], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,0,62,220,62,220,32,20,_DK_gui_area_anger_button,288,735,0,(long)&activity_list[1], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,0,98,220,98,220,32,20,_DK_gui_area_anger_button,288,736,0,(long)&activity_list[2], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_next_creature,_DK_gui_go_to_next_creature,NULL,1,0,220, 0, 242, 22, 22, _DK_gui_area_new_no_anim_button,   0, 733,  0,       0,            0, 0, _DK_maintain_activity_pic },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,1,26,244,26,244,32,20,_DK_gui_area_anger_button,288,734,0,(long)&activity_list[4], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,1,62,244,62,244,32,20,_DK_gui_area_anger_button,288,735,0,(long)&activity_list[5], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,1,98,244,98,244,32,20,_DK_gui_area_anger_button,288,736,0,(long)&activity_list[6], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_next_creature,_DK_gui_go_to_next_creature,NULL,2,0,244, 0, 266, 22, 22, _DK_gui_area_new_no_anim_button,   0, 733,  0,       0,            0, 0, _DK_maintain_activity_pic },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,2,26,268,26,268,32,20,_DK_gui_area_anger_button,288,734,0,(long)&activity_list[8], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,2,62,268,62,268,32,20,_DK_gui_area_anger_button,288,735,0,(long)&activity_list[9], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,2,98,268,98,268,32,20,_DK_gui_area_anger_button,288,736,0,(long)&activity_list[10], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_next_creature,_DK_gui_go_to_next_creature,NULL,3,0,268, 0, 290, 22, 22, _DK_gui_area_new_no_anim_button,   0, 733,  0,       0,            0, 0, _DK_maintain_activity_pic },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,3,26,292,26,292,32,20,_DK_gui_area_anger_button,288,734,0,(long)&activity_list[12], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,3,62,292,62,292,32,20,_DK_gui_area_anger_button,288,735,0,(long)&activity_list[13], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,3,98,292,98,292,32,20,_DK_gui_area_anger_button,288,736,0,(long)&activity_list[14], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_next_creature,_DK_gui_go_to_next_creature,NULL,4,0,292, 0, 314, 22, 22, _DK_gui_area_new_no_anim_button,   0, 733,  0,       0,            0, 0, _DK_maintain_activity_pic },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,4,26,316,26,316,32,20,_DK_gui_area_anger_button,288,734,0,(long)&activity_list[16], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,4,62,316,62,316,32,20,_DK_gui_area_anger_button,288,735,0,(long)&activity_list[17], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,4,98,316,98,316,32,20,_DK_gui_area_anger_button,288,736,0,(long)&activity_list[18], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_next_creature,_DK_gui_go_to_next_creature,NULL,5,0,314, 0, 338, 22, 22, _DK_gui_area_new_no_anim_button,   0, 733,  0,       0,            0, 0, _DK_maintain_activity_pic },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,5,26,340,26,340,32,20,_DK_gui_area_anger_button,288,734,0,(long)&activity_list[20], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,5,62,340,62,340,32,20,_DK_gui_area_anger_button,288,735,0,(long)&activity_list[21], 0, 0, _DK_maintain_activity_row },
  { 0,  0, 0, 0, 0, _DK_pick_up_creature_doing_activity,_DK_gui_go_to_next_creature_activity,NULL,5,98,340,98,340,32,20,_DK_gui_area_anger_button,288,736,0,(long)&activity_list[22], 0, 0, _DK_maintain_activity_row },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit event_menu_buttons[] = {
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit options_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 716,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  12,  36,  12,  36, 46, 64, _DK_gui_area_no_anim_button,      23, 725, &load_menu, 0,          0, 0, _DK_maintain_loadsave },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  60,  36,  60,  36, 46, 64, _DK_gui_area_no_anim_button,      22, 726, &save_menu, 0,          0, 0, _DK_maintain_loadsave },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 108,  36, 108,  36, 46, 64, _DK_gui_area_no_anim_button,      25, 723, &video_menu,0,          0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 156,  36, 156,  36, 46, 64, _DK_gui_area_no_anim_button,      24, 724, &sound_menu,0,          0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 204,  36, 204,  36, 46, 64, _DK_gui_area_new_no_anim_button, 501, 728, &autopilot_menu,0,      0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 252,  36, 252,  36, 46, 64, _DK_gui_area_no_anim_button,      26, 727, &quit_menu,0,           0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit query_menu_buttons[] = {
  { 0,  0, 0, 0, 0, _DK_gui_set_query,  NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, _DK_gui_area_new_normal_button,  475, 432,  0,       0,            0, 0, NULL },
  { 2, 69, 0, 0, 0, _DK_gui_set_tend_to,NULL,        NULL,               1,  36, 190,  36, 190, 32, 26, _DK_gui_area_flash_cycle_button, 350, 307,  0,(long)&game.field_1517FB, 1, 0, _DK_maintain_prison_bar },
  { 2, 70, 0, 0, 0, _DK_gui_set_tend_to,NULL,        NULL,               2,  74, 190,  74, 190, 32, 26, _DK_gui_area_flash_cycle_button, 346, 306,  0,(long)&game.field_1517FC, 1, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 216,   4, 222,130, 24, _DK_gui_area_payday_button,      341, 454,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   2, 246,   2, 246, 60, 24, _DK_gui_area_research_bar,        61, 452,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 246,  74, 246, 60, 24, _DK_gui_area_workshop_bar,        75, 453,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 274,  74, 274, 60, 24, _DK_gui_area_player_creature_info,323,456,  0,       0,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 298,  74, 298, 60, 24, _DK_gui_area_player_creature_info,325,456,  0,       1,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 322,  74, 322, 60, 24, _DK_gui_area_player_creature_info,327,456,  0,       2,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  74, 346,  74, 346, 60, 24, _DK_gui_area_player_creature_info,329,456,  0,       3,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 274,   4, 274, 60, 24, _DK_gui_area_player_room_info,   324, 455,  0,       0,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 298,   4, 298, 60, 24, _DK_gui_area_player_room_info,   326, 455,  0,       1,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 322,   4, 322, 60, 24, _DK_gui_area_player_room_info,   328, 455,  0,       2,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, _DK_gui_area_player_room_info,   330, 455,  0,       3,            0, 0, _DK_maintain_room_and_creature_button },
  { 0,  0, 0, 0, 0, _DK_gui_toggle_ally,NULL,        NULL,               0,  62, 274,  62, 274, 14, 22, _DK_gui_area_ally,                 0, 469,  0,       0,            0, 0, _DK_maintain_ally },
  { 0,  0, 0, 0, 0, _DK_gui_toggle_ally,NULL,        NULL,               0,  62, 298,  62, 298, 14, 22, _DK_gui_area_ally,                 0, 469,  0,       1,            0, 0, _DK_maintain_ally },
  { 0,  0, 0, 0, 0, _DK_gui_toggle_ally,NULL,        NULL,               0,  62, 322,  62, 322, 14, 22, _DK_gui_area_ally,                 0, 469,  0,       2,            0, 0, _DK_maintain_ally },
  { 0,  0, 0, 0, 0, _DK_gui_toggle_ally,NULL,        NULL,               0,  62, 346,  62, 346, 14, 22, _DK_gui_area_ally,                 0, 469,  0,       3,            0, 0, _DK_maintain_ally },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit quit_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,210, 32, _DK_gui_area_text,                 1, 309,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  70,  24,  72,  58, 46, 32, _DK_gui_area_normal_button,       46, 311,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, _DK_gui_quit_game,  NULL,        NULL,               0, 136,  24, 138,  58, 46, 32, _DK_gui_area_normal_button,       48, 310,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit load_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 719,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               0, 999,  58, 999,  58,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[0], 0, 0, _DK_gui_load_game_maintain },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               1, 999,  90, 999,  90,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[1], 0, 0, _DK_gui_load_game_maintain },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               2, 999, 122, 999, 122,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[2], 0, 0, _DK_gui_load_game_maintain },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               3, 999, 154, 999, 154,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[3], 0, 0, _DK_gui_load_game_maintain },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               4, 999, 186, 999, 186,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[4], 0, 0, _DK_gui_load_game_maintain },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               5, 999, 218, 999, 218,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[5], 0, 0, _DK_gui_load_game_maintain },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               6, 999, 250, 999, 250,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[6], 0, 0, _DK_gui_load_game_maintain },
  { 0,  0, 0, 0, 1, _DK_gui_load_game,  NULL,        NULL,               7, 999, 282, 999, 282,300, 32, _DK_draw_load_button,              1, 201,  0,(long)&input_string[7], 0, 0, _DK_gui_load_game_maintain },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit save_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 720,  0,       0,            0, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               0, 999,  58, 999,  58,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[0],15, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               1, 999,  90, 999,  90,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[1],15, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               2, 999, 122, 999, 122,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[2],15, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               3, 999, 154, 999, 154,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[3],15, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               4, 999, 186, 999, 186,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[4],15, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               5, 999, 218, 999, 218,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[5],15, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               6, 999, 250, 999, 250,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[6],15, 0, NULL },
  { 5, -2,-1,-1, 1, _DK_gui_save_game,  NULL,        NULL,               7, 999, 282, 999, 282,300, 32, _DK_gui_area_text,                 1, 201,  0,(long)&input_string[7],15, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit video_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 717,  0,       0,            0, 0, NULL },
  { 2,  0, 0, 0, 0, _DK_gui_video_shadows,NULL,      NULL,               0,   8,  38,  10,  38, 46, 64, _DK_gui_area_no_anim_button,      27, 313,  0,(long)&_DK_video_shadows, 4, 0, NULL },
  { 2,  0, 0, 0, 0, _DK_gui_video_view_distance_level,NULL,NULL,         0,  56,  38,  58,  38, 46, 64, _DK_gui_area_no_anim_button,      36, 316,  0,(long)&_DK_video_view_distance_level, 3, 0, NULL },
  { 2,  0, 0, 0, 0, _DK_gui_video_rotate_mode,NULL,  NULL,               0, 104,  38, 106,  38, 46, 64, _DK_gui_area_no_anim_button,      32, 314,  0,(long)&_DK_settings.field_3, 1, 0, NULL },
  { 2,  0, 0, 0, 0, _DK_gui_video_cluedo_mode,NULL,  NULL,               0,  32,  90,  32,  90, 46, 64, _DK_gui_area_no_anim_button,      42, 315,  0,(long)&_DK_video_cluedo_mode,1, 0, _DK_gui_video_cluedo_maintain },
  { 0,  0, 0, 0, 0, _DK_gui_video_gamma_correction,NULL,NULL,            0,  80,  90,  80,  90, 46, 64, _DK_gui_area_no_anim_button,      44, 317,  0,(long)&_DK_video_gamma_correction, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit sound_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 718,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8,  28,  10,  28, 46, 64, _DK_gui_area_no_anim_button,      41, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8,  80,  10,  80, 46, 64, _DK_gui_area_no_anim_button,      40, 201,  0,       0,            0, 0, NULL },
  { 4,  0, 0, 0, 0, _DK_gui_set_sound_volume,NULL,   NULL,               0,  66,  58,  66,  58,190, 32, _DK_gui_area_slider,               0, 340,  0,(long)&_DK_sound_level, 127, 0, NULL },
  { 4,  0, 0, 0, 0, _DK_gui_set_music_volume,NULL,   NULL,               0,  66, 110,  66, 110,190, 32, _DK_gui_area_slider,               0, 341,  0,(long)&_DK_music_level, 127, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit error_box_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 670,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,   0, 999,   0,155,155, _DK_gui_area_text,                 0, 201,  0,(long)&error_box_message,0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 999, 100, 999, 132, 46, 34, _DK_gui_area_normal_button,       48, 201,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit instance_menu_buttons[] = {
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit text_info_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,   4, 999,   4,400, 78, _DK_gui_area_scroll_window,        0, 201,  0,(long)&game.text_info,0,0, NULL },
  { 1, 63, 0, 0, 0, _DK_gui_go_to_event,NULL,        NULL,               0,   4,   4,   4,   4, 30, 24, _DK_gui_area_new_normal_button,  276, 466,  0,       0,             0,0, _DK_maintain_zoom_to_event },
  { 0, 64, 0, 0, 1, _DK_gui_close_objective,_DK_gui_close_objective,NULL,0,   4,  56,   4,  56, 30, 24, _DK_gui_area_new_normal_button,  274, 465,  0,       0,             0,0, NULL },
  { 1, 66, 0, 0, 0, _DK_gui_scroll_text_up,NULL,     NULL,               0, 446,   4, 446,   4, 30, 24, _DK_gui_area_new_normal_button,  486, 201,  0,(long)&game.text_info,0,0, _DK_maintain_scroll_up },
  { 1, 65, 0, 0, 0, _DK_gui_scroll_text_down,NULL,   NULL,               0, 446,  56, 446,  56, 30, 24, _DK_gui_area_new_normal_button,  272, 201,  0,(long)&game.text_info,0,0, _DK_maintain_scroll_down },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit pause_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999, 999, 999, 999,140,100, _DK_gui_area_text,                 0, 320,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit battle_buttons[] = {
  { 0,  0, 0, 0, 1, _DK_gui_close_objective,NULL,    NULL,               0,   4,  72,   4,  72, 30, 24, _DK_gui_area_new_normal_button,  274, 465,  0,       0,            0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_gui_previous_battle,NULL,    NULL,               0, 446,   4, 446,   4, 30, 24, _DK_gui_area_new_normal_button,  486, 464,  0,       0,            0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_gui_next_battle,NULL,        NULL,               0, 446,  72, 446,  72, 30, 24, _DK_gui_area_new_normal_button,  272, 464,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_gui_get_creature_in_battle,_DK_gui_go_to_person_in_battle,_DK_gui_setup_friend_over,0, 42,12, 42,12,160,24,_DK_gui_area_friendly_battlers,0,201,0,0,0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_gui_get_creature_in_battle,_DK_gui_go_to_person_in_battle,_DK_gui_setup_enemy_over, 0,260,12,260,12,160,24,_DK_gui_area_enemy_battlers,   0,201,0,0,0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_gui_get_creature_in_battle,_DK_gui_go_to_person_in_battle,_DK_gui_setup_friend_over,1, 42,42, 42,42,160,24,_DK_gui_area_friendly_battlers,0,201,0,0,0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_gui_get_creature_in_battle,_DK_gui_go_to_person_in_battle,_DK_gui_setup_enemy_over, 1,260,42,260,42,160,24,_DK_gui_area_enemy_battlers,   0,201,0,0,0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_gui_get_creature_in_battle,_DK_gui_go_to_person_in_battle,_DK_gui_setup_friend_over,2, 42,72, 42,72,160,24,_DK_gui_area_friendly_battlers,0,201,0,0,0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_gui_get_creature_in_battle,_DK_gui_go_to_person_in_battle,_DK_gui_setup_enemy_over, 2,260,72,260,72,160,24,_DK_gui_area_enemy_battlers,   0,201,0,0,0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 214,  34, 214,  34, 32, 32, _DK_gui_area_null,               175, 201,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit resurrect_creature_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, _DK_gui_area_text,                 1, 428,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_select_resurrect_creature,   NULL,NULL,          0, 999,  62, 999,  62,250, 26, _DK_draw_resurrect_creature,       0, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_resurrect_creature,   NULL,NULL,          1, 999,  90, 999,  90,250, 26, _DK_draw_resurrect_creature,       0, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_resurrect_creature,   NULL,NULL,          2, 999, 118, 999, 118,250, 26, _DK_draw_resurrect_creature,       0, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_resurrect_creature,   NULL,NULL,          3, 999, 146, 999, 146,250, 26, _DK_draw_resurrect_creature,       0, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_resurrect_creature,   NULL,NULL,          4, 999, 174, 999, 174,250, 26, _DK_draw_resurrect_creature,       0, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_resurrect_creature,   NULL,NULL,          5, 999, 202, 999, 202,250, 26, _DK_draw_resurrect_creature,       0, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_select },
  { 1,  0, 0, 0, 0, _DK_select_resurrect_creature_up,NULL,NULL,          1, 305,  62, 305,  62, 22, 24, _DK_gui_area_new_normal_button,  278, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_scroll },
  { 1,  0, 0, 0, 0, _DK_select_resurrect_creature_down,NULL,NULL,        2, 305, 204, 305, 204, 22, 24, _DK_gui_area_new_normal_button,  280, 201,  0,       0,            0, 0, _DK_maintain_resurrect_creature_scroll },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 999, 258, 999, 258,100, 32, _DK_gui_area_text,                 1, 403,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit transfer_creature_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, _DK_gui_area_text,                 1, 429,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_select_transfer_creature,NULL,NULL,              0, 999,  62, 999,  62,250, 26, _DK_draw_transfer_creature,        0, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_transfer_creature,NULL,NULL,              1, 999,  90, 999,  90,250, 26, _DK_draw_transfer_creature,        1, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_transfer_creature,NULL,NULL,              2, 999, 118, 999, 118,250, 26, _DK_draw_transfer_creature,        2, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_transfer_creature,NULL,NULL,              3, 999, 146, 999, 146,250, 26, _DK_draw_transfer_creature,        3, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_transfer_creature,NULL,NULL,              4, 999, 174, 999, 174,250, 26, _DK_draw_transfer_creature,        4, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_select },
  { 0,  0, 0, 0, 0, _DK_select_transfer_creature,NULL,NULL,              5, 999, 202, 999, 202,250, 26, _DK_draw_transfer_creature,        5, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_select },
  { 1,  0, 0, 0, 0, _DK_select_transfer_creature_up,NULL,NULL,           1, 305,  62, 305,  62, 22, 24, _DK_gui_area_new_normal_button,  278, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_scroll },
  { 1,  0, 0, 0, 0, _DK_select_transfer_creature_down,NULL,NULL,         2, 305, 204, 305, 204, 22, 24, _DK_gui_area_new_normal_button,  280, 201,  0,       0,            0, 0, _DK_maintain_transfer_creature_scroll },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0, 999, 258, 999, 258,100, 32, _DK_gui_area_text,                 1, 403,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit hold_audience_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 634,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, _DK_gui_area_normal_button,       46, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, _DK_choose_hold_audience,NULL,   NULL,               0, 116,  24, 118,  58, 46, 32, _DK_gui_area_normal_button,       48, 201,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit armageddon_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                 1, 646,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, _DK_gui_area_normal_button,       46, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 1, _DK_choose_armageddon,NULL,      NULL,               0, 116,  24, 118,  58, 46, 32, _DK_gui_area_normal_button,       48, 201,  0,       0,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit dungeon_special_buttons[] = {
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_main_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,       1,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_start_new_game,NULL,_DK_frontend_over_button,3,999,104,999,104,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,       2,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_load_continue_game,NULL,_DK_frontend_over_button,0,999,154,999,154,371,46,_DK_frontend_draw_large_menu_button,0,201,0,      8,            0, 0, _DK_frontend_continue_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL, _DK_frontend_over_button, 2,999,204,999,204,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,       3,            0, 0, _DK_frontend_main_menu_load_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL, _DK_frontend_over_button, 4,999,254,999,254,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,       4,            0, 0, _DK_frontend_main_menu_netservice_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL, _DK_frontend_over_button,27,999,304,999,304,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,      97,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL, _DK_frontend_over_button,18,999,354,999,354,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,     104,            0, 0, _DK_frontend_main_menu_highscores_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL, _DK_frontend_over_button, 9,999,404,999,404,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,       5,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_load_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,       7,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, _DK_frontnet_draw_scroll_box_tab,  0, 201,  0,      28,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,180, _DK_frontnet_draw_scroll_box,      0, 201,  0,      26,            0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontend_load_game_up,NULL,_DK_frontend_over_button,0,532,149,532, 149, 26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,      17,            0, 0, _DK_frontend_load_game_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontend_load_game_down,NULL,_DK_frontend_over_button,0,532,317,532,317,26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,      18,            0, 0, _DK_frontend_load_game_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 163, 536, 163, 10,154, _DK_frontend_draw_games_scroll_tab,0, 201,  0,      40,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 125, 102, 125,220, 26, _DK_frontend_draw_text,            0, 201,  0,      30,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_load_game,NULL,_DK_frontend_over_button,0,  95, 157,  95, 157,424, 26, _DK_frontend_draw_load_game_button,0, 201,  0,      45,            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_load_game,NULL,_DK_frontend_over_button,0,  95, 185,  95, 185,424, 26, _DK_frontend_draw_load_game_button,0, 201,  0,      46,            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_load_game,NULL,_DK_frontend_over_button,0,  95, 213,  95, 213,424, 26, _DK_frontend_draw_load_game_button,0, 201,  0,      47,            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_load_game,NULL,_DK_frontend_over_button,0,  95, 241,  95, 241,424, 26, _DK_frontend_draw_load_game_button,0, 201,  0,      48,            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_load_game,NULL,_DK_frontend_over_button,0,  95, 269,  95, 269,424, 26, _DK_frontend_draw_load_game_button,0, 201,  0,      49,            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_load_game,NULL,_DK_frontend_over_button,0,  95, 297,  95, 297,424, 26, _DK_frontend_draw_load_game_button,0, 201,  0,      50,            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL,_DK_frontend_over_button,1,999,404,999, 404,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,       6,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_net_service_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,      10,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, _DK_frontnet_draw_scroll_box_tab,  0, 201,  0,      12,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,180, _DK_frontnet_draw_scroll_box,      0, 201,  0,      26,            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontnet_service_up,NULL,_DK_frontend_over_button,   0, 532, 149, 532, 149, 26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,      17,            0, 0, frontnet_service_up_maintain },
  { 1,  0, 0, 0, 0, frontnet_service_down,NULL,_DK_frontend_over_button, 0, 532, 317, 532, 317, 26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,      18,            0, 0, frontnet_service_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 163, 536, 163, 10,154, _DK_frontnet_draw_services_scroll_tab,0,201,0,      40,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 125, 102, 125,220, 26, _DK_frontend_draw_text,            0, 201,  0,      33,            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,_DK_frontend_over_button,0, 95, 158,  95, 158,424, 26, frontnet_draw_service_button,      0, 201,  0,      45,            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,_DK_frontend_over_button,0, 95, 184,  95, 184,424, 26, frontnet_draw_service_button,      0, 201,  0,      46,            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,_DK_frontend_over_button,0, 95, 210,  95, 210,424, 26, frontnet_draw_service_button,      0, 201,  0,      47,            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,_DK_frontend_over_button,0, 95, 236,  95, 236,424, 26, frontnet_draw_service_button,      0, 201,  0,      48,            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,_DK_frontend_over_button,0, 95, 262,  95, 262,424, 26, frontnet_draw_service_button,      0, 201,  0,      49,            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, frontnet_service_select,NULL,_DK_frontend_over_button,0, 95, 288,  95, 288,424, 26, frontnet_draw_service_button,      0, 201,  0,      50,            0, 0, frontnet_service_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL,_DK_frontend_over_button,1,999,404,999, 404,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,       6,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_net_session_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 999,  30, 999,  30, 371, 46, _DK_frontend_draw_large_menu_button,0,201, 0,      12,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82,  79,  82,  79, 165, 29, _DK_frontnet_draw_text_bar,       0, 201,  0,      27,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95,  81,  91,  81, 165, 25, _DK_frontend_draw_text,           0, 201,  0,      19,            0, 0, NULL },
  { 5, -1,-1,-1, 0, _DK_frontnet_session_set_player_name,NULL,_DK_frontend_over_button,19,200,81,95,81,432,25,_DK_frontend_draw_enter_text,0, 201,  0,(long)_DK_tmp_net_player_name, 20, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 112,  82, 112, 220, 26, _DK_frontnet_draw_scroll_box_tab, 0, 201,  0,      28,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 138,  82, 138, 450,180, _DK_frontnet_draw_scroll_box,     0, 201,  0,      25,            0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontnet_session_up,NULL,_DK_frontend_over_button,  0, 532, 137, 532, 137,  26, 14, _DK_frontnet_draw_slider_button,0,201,  0,      17,            0, 0, _DK_frontnet_session_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontnet_session_down,NULL,_DK_frontend_over_button,0, 532, 217, 532, 217,  26, 14, _DK_frontnet_draw_slider_button,0,201,  0,      18,            0, 0, _DK_frontnet_session_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 536, 151, 536, 151,  10, 66, _DK_frontnet_draw_sessions_scroll_tab,0,201,0,     40,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 102, 113, 102, 113, 220, 26, _DK_frontend_draw_text,           0, 201,  0,      29,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 230,  82, 230, 450, 23, _DK_frontnet_draw_session_selected,0,201,  0,      35,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_session_select,NULL,_DK_frontend_over_button,0,95, 141,  95, 141, 424, 26, _DK_frontnet_draw_session_button,   0,201,       0,           45, 0, 0, _DK_frontnet_session_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_session_select,NULL,_DK_frontend_over_button,0,95, 167,  95, 167, 424, 26, _DK_frontnet_draw_session_button,   0,201,       0,           46, 0, 0, _DK_frontnet_session_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_session_select,NULL,_DK_frontend_over_button,0,95, 193,  95, 193, 424, 26, _DK_frontnet_draw_session_button,   0,201,       0,           47, 0, 0, _DK_frontnet_session_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 261,  82, 261, 220, 26, _DK_frontnet_draw_scroll_box_tab, 0, 201,  0,      28,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  82, 287,  82, 287, 450, 74, _DK_frontnet_draw_scroll_box,     0, 201,  0,      24,            0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontnet_players_up,NULL,_DK_frontend_over_button,0, 532, 286, 532, 286,  26, 14, _DK_frontnet_draw_slider_button, 0, 201,  0,      36,            0, 0, _DK_frontnet_players_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontnet_players_down,NULL,_DK_frontend_over_button,0,532,344, 532, 344,  26, 14, _DK_frontnet_draw_slider_button, 0, 201,  0,      37,            0, 0, _DK_frontnet_players_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0, 536, 300, 536, 300,  10, 44, _DK_frontnet_draw_players_scroll_tab,0,201,0,      40,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 262,  95, 262, 220, 22, _DK_frontend_draw_text,           0, 201,  0,      31,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,   NULL,                    0,  95, 291,  82, 291, 450, 52, _DK_frontnet_draw_net_session_players, 0,201,0,    21,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_session_join,NULL,_DK_frontend_over_button,0,  72, 360,  72, 360, 247, 46, _DK_frontend_draw_small_menu_button,0,201,0,    13,            0, 0, _DK_frontnet_join_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_session_create,NULL,_DK_frontend_over_button,0,321,360, 321, 360, 247, 46, _DK_frontend_draw_small_menu_button,0,201,0,    14,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_return_to_main_menu,NULL,_DK_frontend_over_button,0,999,404,999,404,371,46,_DK_frontend_draw_large_menu_button,0,201,0,     6,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_net_start_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 999,  30, 999,  30, 371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,  12, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82,  78,  82,  78, 220, 26, _DK_frontnet_draw_scroll_box_tab,  0, 201,  0,  28, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 421,  81, 421,  81, 100, 27, _DK_frontnet_draw_alliance_box_tab,0, 201,  0,  28, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 104,  82, 104, 450, 70, _DK_frontnet_draw_scroll_box,      0, 201,  0,  90, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 102,  79, 102,  79, 220, 26, _DK_frontend_draw_text,            0, 201,  0,  31, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  95, 105,  82, 105, 432,104, _DK_frontnet_draw_net_start_players,0,201,  0,  21, 0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,0,431,107,431, 116, 432, 88, _DK_frontnet_draw_alliance_grid,   0, 201,  0,  74, 0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,0,431,108,431, 108,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  74, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,1,453,108,453, 108,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  74, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,2,475,108,475, 108,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  74, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,3,497,108,497, 108,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  74, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,0,431,134,431, 134,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  75, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,1,453,134,453, 134,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  75, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,2,475,134,475, 134,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  75, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,3,497,134,497, 134,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  75, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,0,431,160,431, 160,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  76, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,1,453,160,453, 160,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  76, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,2,475,160,475, 160,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  76, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,3,497,160,497, 160,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  76, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,0,431,186,431, 183,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  77, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,1,453,186,453, 186,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  77, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,2,475,186,475, 186,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  77, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, _DK_frontnet_select_alliance,0,_DK_frontend_over_button,3,497,186,497, 186,  22, 26, _DK_frontnet_draw_alliance_button, 0, 201,  0,  77, 0, 0, _DK_frontnet_maintain_alliance },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 284, 217, 284, 217,   0,  0, _DK_frontnet_draw_bottom_scroll_box_tab,0,201,0,28,0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_toggle_computer_players,0,_DK_frontend_over_button,0,297,214,297,214,220,26,_DK_frontend_draw_computer_players,0,201,0,103, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 246,  82, 246, 220, 26, _DK_frontnet_draw_scroll_box_tab,  0, 201,  0,  28, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 272,  82, 272, 450,111, _DK_frontnet_draw_scroll_box,      0, 201,  0,  91, 0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontnet_messages_up,0, _DK_frontend_over_button,0, 532, 271, 532, 271,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  38, 0, 0, _DK_frontnet_messages_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontnet_messages_down,0,_DK_frontend_over_button,0,532, 373, 532, 373,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  39, 0, 0, _DK_frontnet_messages_down_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 102, 247, 102, 247, 220, 26, _DK_frontend_draw_text,            0, 201,  0,  34, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 536, 285, 536, 285,  10, 88, _DK_frontnet_draw_messages_scroll_tab,0,201,0,  40, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 386,  82, 386, 459, 23, _DK_frontnet_draw_current_message, 0, 201,  0,  43, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  89, 273,  89, 273, 438,104, _DK_frontnet_draw_messages,        0, 201,  0,  44, 0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_set_packet_start,0,     _DK_frontend_over_button,0,  49, 412,  49, 412, 247, 46, _DK_frontend_draw_small_menu_button,0,201,  0,  15, 0, 0, _DK_frontnet_start_game_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_return_to_session_menu,0,_DK_frontend_over_button,1,345,412,345,412,247,46,_DK_frontend_draw_small_menu_button,0,201,0, 16, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,                0,     NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,   0,  0,   0, 0, 0, NULL },
};

struct GuiButtonInit frontend_net_modem_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 999,  30, 999,  30, 371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,  53, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  41, 102,  41, 102, 212, 26, _DK_frontnet_draw_small_scroll_box_tab,0,201,0, 28, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  41, 128,  41, 128, 268, 70, _DK_frontnet_draw_small_scroll_box,0, 201,  0,  24, 0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontnet_comport_up,0,  _DK_frontend_over_button,0, 275, 128, 275, 128,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  17, 0, 0, _DK_frontnet_comport_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontnet_comport_down,0,_DK_frontend_over_button,0, 275, 186, 275, 186,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  18, 0, 0, _DK_frontnet_comport_down_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 279, 142, 279, 142,  22, 44, _DK_frontnet_draw_comport_scroll_tab,0,201, 0,  40, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  61, 103,  61, 103, 172, 25, _DK_frontend_draw_text,            0, 201,  0,  55, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  41, 198,  41, 198, 268, 23, _DK_frontnet_draw_comport_selected,0, 201,  0,  57, 0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_comport_select,0,_DK_frontend_over_button,0,54, 136,  54, 136, 190, 26, _DK_frontnet_draw_comport_button,  0, 201,  0,  45, 0, 0, _DK_frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_comport_select,0,_DK_frontend_over_button,0,54, 164,  54, 164, 190, 26, _DK_frontnet_draw_comport_button,  0, 201,  0,  46, 0, 0, _DK_frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 331, 102, 331, 102, 212, 26, _DK_frontnet_draw_small_scroll_box_tab,0,201,0, 28, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 331, 128, 331, 128, 268, 70, _DK_frontnet_draw_small_scroll_box,0, 201,  0,  24, 0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontnet_speed_up,0,    _DK_frontend_over_button,0, 565, 128, 565, 128,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  36, 0, 0, _DK_frontnet_speed_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontnet_speed_down,0,  _DK_frontend_over_button,0, 565, 186, 565, 186,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  37, 0, 0, _DK_frontnet_speed_down_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 569, 142, 569, 142,  22, 44, _DK_frontnet_draw_speed_scroll_tab,0, 201,  0,  40, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 351, 103, 351, 103, 172, 25, _DK_frontend_draw_text,            0, 201,  0,  56, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 331, 198, 331, 198, 450, 23, _DK_frontnet_draw_speed_selected,  0, 201,  0,  58, 0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_speed_select,0,_DK_frontend_over_button,0, 344, 136, 344, 136, 190, 14, _DK_frontnet_draw_speed_button,    0, 201,  0,  47, 0, 0, _DK_frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_speed_select,0,_DK_frontend_over_button,0, 344, 164, 344, 164, 190, 14, _DK_frontnet_draw_speed_button,    0, 201,  0,  48, 0, 0, _DK_frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 254,  82, 254, 165, 28, _DK_frontnet_draw_text_cont_bar,   0, 201,  0,  27, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  95, 255,  91, 255, 165, 25, _DK_frontend_draw_text,            0, 201,  0,  71, 0, 0, NULL },
  { 5, -3,-1,-1, 0, _DK_frontnet_net_set_phone_number,0,_DK_frontend_over_button,71,280,255,95,255,432,25,_DK_frontend_draw_enter_text,     0, 201,  0, (long)_DK_tmp_net_phone_number, 20, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 282,  82, 282, 165, 28, _DK_frontnet_draw_text_cont_bar,   0, 201,  0,  27, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  95, 283,  91, 283, 165, 25, _DK_frontend_draw_text,            0, 201,  0,  66, 0, 0, NULL },
  { 5, -1,-1,-1, 0, _DK_frontnet_net_set_modem_init,0,_DK_frontend_over_button,66,280,283,95,283,432,25, _DK_frontend_draw_enter_text,      0, 201,  0, (long)_DK_tmp_net_modem_init, -20, -1, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 310,  82, 310, 165, 28, _DK_frontnet_draw_text_cont_bar,   0, 201,  0,  27, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  95, 311,  91, 311, 165, 25, _DK_frontend_draw_text,            0, 201,  0,  67, 0, 0, NULL },
  { 5, -1,-1,-1, 0, _DK_frontnet_net_set_modem_hangup,0,_DK_frontend_over_button,67,280,311,95,311,432,25,_DK_frontend_draw_enter_text,     0, 201,  0, (long)_DK_tmp_net_modem_hangup, -20, -1, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 338,  82, 338, 165, 28, _DK_frontnet_draw_text_cont_bar,   0, 201,  0,  27, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  95, 339,  91, 339, 165, 25, _DK_frontend_draw_text,            0, 201,  0,  68, 0, 0, NULL },
  { 5, -1,-1,-1, 0, _DK_frontnet_net_set_modem_dial,0,_DK_frontend_over_button,68,280,339,95,339,432,25, _DK_frontend_draw_enter_text,      0, 201,  0, (long)_DK_tmp_net_modem_dial, -20, -1, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  82, 366,  82, 366, 165, 28, _DK_frontnet_draw_text_cont_bar,   0, 201,  0,  27, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  95, 367,  91, 367, 165, 25, _DK_frontend_draw_text,            0, 201,  0,  69, 0, 0, NULL },
  { 5, -1,-1,-1, 0, _DK_frontnet_net_set_modem_answer,0,_DK_frontend_over_button,69,280,367,95,367,432,25,_DK_frontend_draw_enter_text,     0, 201,  0, (long)_DK_tmp_net_modem_answer, -20, -1, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_net_modem_start,0,_DK_frontend_over_button,0,49,412,  49, 412, 247, 46, _DK_frontend_draw_small_menu_button,0,201,  0,  72, 0, 0, _DK_frontnet_net_modem_start_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,0,_DK_frontend_over_button,1, 345, 412, 345, 412, 247, 46, _DK_frontend_draw_small_menu_button,0,201,  0,  16, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,                0,     NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,   0,  0,   0, 0, 0, NULL },
};

struct GuiButtonInit frontend_net_serial_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 999,  30, 999,  30, 371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,  54, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  41, 178,  41, 178, 212, 26, _DK_frontnet_draw_small_scroll_box_tab,0,201,0, 28, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  41, 204,  41, 204, 268, 70, _DK_frontnet_draw_small_scroll_box,0, 201,  0,  24, 0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontnet_comport_up,0,  _DK_frontend_over_button,0, 275, 204, 275, 204,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  17, 0, 0, _DK_frontnet_comport_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontnet_comport_down,0,_DK_frontend_over_button,0, 275, 262, 275, 262,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  18, 0, 0, _DK_frontnet_comport_down_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 279, 218, 279, 218,  22, 44, _DK_frontnet_draw_comport_scroll_tab,0,201, 0,  40, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  61, 179,  61, 179, 172, 25, _DK_frontend_draw_text,            0, 201,  0,  55, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0,  41, 274,  41, 274, 268, 23, _DK_frontnet_draw_comport_selected,0, 201,  0,  57, 0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_comport_select,0,_DK_frontend_over_button,0,54, 212,  54, 212, 190, 26, _DK_frontnet_draw_comport_button,  0, 201,  0,  45, 0, 0, _DK_frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_comport_select,0,_DK_frontend_over_button,0,54, 240,  54, 240, 190, 26, _DK_frontnet_draw_comport_button,  0, 201,  0,  46, 0, 0, _DK_frontnet_comport_select_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 331, 178, 331, 178, 212, 26, _DK_frontnet_draw_small_scroll_box_tab,0,201,0, 28, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 331, 204, 331, 204, 268, 70, _DK_frontnet_draw_small_scroll_box,0, 201,  0,  24, 0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontnet_speed_up,0,    _DK_frontend_over_button,0, 565, 204, 565, 204,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  36, 0, 0, _DK_frontnet_speed_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontnet_speed_down,0,  _DK_frontend_over_button,0, 565, 262, 565, 262,  26, 14, _DK_frontnet_draw_slider_button,   0, 201,  0,  37, 0, 0, _DK_frontnet_speed_down_maintain },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 569, 218, 569, 218,  22, 44, _DK_frontnet_draw_speed_scroll_tab,0, 201,  0,  40, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 351, 179, 351, 179, 172, 25, _DK_frontend_draw_text,            0, 201,  0,  56, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,                0,     NULL,                    0, 331, 274, 331, 274, 450, 23, _DK_frontnet_draw_speed_selected,  0, 201,  0,  58, 0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontnet_speed_select,0,_DK_frontend_over_button,0, 344, 212, 344, 212, 190, 26, _DK_frontnet_draw_speed_button,    0, 201,  0,  47, 0, 0, _DK_frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_speed_select,0,_DK_frontend_over_button,0, 344, 240, 344, 240, 190, 26, _DK_frontnet_draw_speed_button,    0, 201,  0,  48, 0, 0, _DK_frontnet_speed_select_maintain },
  { 0,  0, 0, 0, 0, _DK_frontnet_net_serial_start,0,_DK_frontend_over_button,0,49,412, 49, 412, 247, 46, _DK_frontend_draw_small_menu_button,0,201,  0,  73, 0, 0, _DK_frontnet_net_serial_start_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,0,_DK_frontend_over_button,1, 345, 412, 345, 412, 247, 46, _DK_frontend_draw_small_menu_button,0,201,  0,  16, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,                0,     NULL,                    0,   0,   0,   0,   0,   0,  0, NULL,                              0,   0,  0,   0, 0, 0, NULL },
};

struct GuiButtonInit frontend_statistics_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,      84,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  90, 999,  90,450,158, _DK_frontstats_draw_main_stats,    0, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999, 260, 999, 260,450,136, _DK_frontstats_draw_scrolling_stats,0,201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontstats_leave,NULL,_DK_frontend_over_button, 18, 999, 404, 999, 404,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,      83,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_high_score_score_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,495, 46, _DK_frontend_draw_vlarge_menu_button,0,201, 0,      85,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  97, 999,  97,450,286, _DK_frontend_draw_high_score_table,0, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_quit_high_score_table,NULL,_DK_frontend_over_button,3,999,404,999,404,371,46,_DK_frontend_draw_large_menu_button,0,201,0,  83,            0, 0, _DK_frontend_maintain_high_score_ok_button },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons1[] = {
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, _DK_gui_area_new_normal_button,  473, 433,&creature_query_menu2,0, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, _DK_gui_area_smiley_anger_button,466, 291,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, _DK_gui_area_experience_button,  467, 223,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, 222,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       0,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       1,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       2,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       3,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       4,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       5,            0, 0, _DK_maintain_instance },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons2[] = {
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, _DK_gui_area_new_normal_button,  473, 433,&creature_query_menu3,0, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, _DK_gui_area_smiley_anger_button,466, 291,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, _DK_gui_area_experience_button,  467, 223,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, 222,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       4,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       5,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       6,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       7,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       8,            0, 0, _DK_maintain_instance },
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, _DK_gui_area_instance_button,     45, 201,  0,       9,            0, 0, _DK_maintain_instance },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons3[] = {
  { 0,  0, 0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, _DK_gui_area_new_normal_button,  473, 433,&creature_query_menu1,0, 0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 226,   4, 226, 60, 24, _DK_gui_area_stat_button,        331, 292,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 226,  72, 226, 60, 24, _DK_gui_area_stat_button,        332, 293,  0,       1,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 256,   4, 256, 60, 24, _DK_gui_area_stat_button,        333, 295,  0,       2,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 256,  72, 256, 60, 24, _DK_gui_area_stat_button,        334, 294,  0,       3,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 286,   4, 286, 60, 24, _DK_gui_area_stat_button,        335, 296,  0,       4,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 286,  72, 286, 60, 24, _DK_gui_area_stat_button,        336, 297,  0,       5,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 316,   4, 316, 60, 24, _DK_gui_area_stat_button,        337, 298,  0,       6,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 316,  72, 316, 60, 24, _DK_gui_area_stat_button,        338, 299,  0,       7,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, _DK_gui_area_stat_button,        339, 300,  0,       8,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, _DK_gui_area_stat_button,        340, 301,  0,       9,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_define_keys_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,      92,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 117,  82, 117,450,246, _DK_frontnet_draw_scroll_box,      0, 201,  0,      94,            0, 0, NULL },
  { 1,  0, 0, 0, 0, _DK_frontend_define_key_up,NULL, _DK_frontend_over_button,0,532, 116,532,116,26,14, _DK_frontnet_draw_slider_button,   0, 201,  0,      17,            0, 0, _DK_frontend_define_key_up_maintain },
  { 1,  0, 0, 0, 0, _DK_frontend_define_key_down,NULL,_DK_frontend_over_button,0,532,350,532,350,26,14, _DK_frontnet_draw_slider_button,   0, 201,  0,      18,            0, 0, _DK_frontend_define_key_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 130, 536, 130, 10,220, _DK_frontend_draw_define_key_scroll_tab,0,201,0,    40,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 130,  95, 130,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -1,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 152,  95, 152,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -2,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 174,  95, 174,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -3,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 196,  95, 196,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -4,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 218,  95, 218,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -5,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 240,  95, 240,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -6,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 262,  95, 262,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -7,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 284,  95, 284,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -8,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 306,  95, 306,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,      -9,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_define_key,NULL, _DK_frontend_over_button,0,95, 328,  95, 328,424, 22, _DK_frontend_draw_define_key,      0, 201,  0,     -10,            0, 0, _DK_frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL,_DK_frontend_over_button,27,999,404,999,404,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,      98,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit autopilot_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, _DK_gui_area_text,                1, 845,  0,       0,            0, 0, NULL },
  { 3,  0, 0, 0, 0, _DK_gui_set_autopilot,NULL,      NULL,               0,  12,  36,  12,  36, 46, 64, _DK_gui_area_new_normal_button, 503, 729,  0,(long)&game.field_1517F7, 0, 0, NULL },
  { 3,  0, 0, 0, 0, _DK_gui_set_autopilot,NULL,      NULL,               0,  60,  36,  60,  36, 46, 64, _DK_gui_area_new_normal_button, 505, 730,  0,(long)&game.field_1517F8, 0, 0, NULL },
  { 3,  0, 0, 0, 0, _DK_gui_set_autopilot,NULL,      NULL,               0, 108,  36, 108,  36, 46, 64, _DK_gui_area_new_normal_button, 507, 731,  0,(long)&game.field_1517F9, 0, 0, NULL },
  { 3,  0, 0, 0, 0, _DK_gui_set_autopilot,NULL,      NULL,               0, 156,  36, 156,  36, 46, 64, _DK_gui_area_new_normal_button, 509, 732,  0,(long)&game.field_1517FA, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                             0,   0,  0,       0,            0, 0, NULL },
};

struct GuiButtonInit frontend_option_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, _DK_frontend_draw_large_menu_button,0,201,  0,      96,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 107,  95, 107,220, 26, _DK_frontnet_draw_scroll_box_tab,  0, 201,  0,      28,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 133,  95, 133,  2, 88, _DK_frontnet_draw_scroll_box,      0, 201,  0,      89,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 108, 115, 108,220, 26, _DK_frontend_draw_text,            0, 201,  0,      99,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 142, 146, 142, 48, 32, _DK_frontend_draw_icon,           90, 201,  0,       0,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 182, 146, 182, 48, 32, _DK_frontend_draw_icon,           89, 201,  0,       0,            0, 0, NULL },
  { 4, 75, 0, 0, 0, _DK_gui_set_sound_volume,NULL,   NULL,               0, 194, 147, 194, 147,300, 22, _DK_frontend_draw_slider,          0, 201,  0,(long)&_DK_sound_level, 127, 0, NULL },
  { 4,  0, 0, 0, 0, _DK_gui_set_music_volume,NULL,   NULL,               0, 194, 187, 194, 187,300, 22, _DK_frontend_draw_slider,          0, 201,  0,(long)&_DK_music_level, 127, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 231,  95, 231,220, 26, _DK_frontnet_draw_scroll_box_tab,  0, 201,  0,      28,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 257,  95, 257,  0, 88, _DK_frontnet_draw_scroll_box,      0, 201,  0,      89,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 232, 115, 232,220, 26, _DK_frontend_draw_text,            0, 201,  0,     100,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 271, 102, 271,190, 22, _DK_frontend_draw_text,            0, 201,  0,     101,            0, 0, NULL },
  { 4,  0, 0, 0, 0, _DK_frontend_set_mouse_sensitivity,NULL,NULL,        0, 304, 271, 304, 271,190, 22, _DK_frontend_draw_small_slider,    0, 201,  0,(long)&_DK_fe_mouse_sensitivity, 7, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_invert_mouse,NULL, _DK_frontend_over_button,0,102,303,102,303,380, 22, _DK_frontend_draw_text,            0, 201,  0,     102,            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 320, 303,   0,   0,100, 22, _DK_frontend_draw_invert_mouse,    0, 201,  0,     102,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL, _DK_frontend_over_button,26,999,357,999,357,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,      95,            0, 0, NULL },
  { 0,  0, 0, 0, 0, _DK_frontend_change_state,NULL, _DK_frontend_over_button, 1,999,404,999,404,371,46, _DK_frontend_draw_large_menu_button,0,201,  0,       6,            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       0,            0, 0, NULL },
};

struct GuiMenu main_menu =
 { 1, 0, 1, main_menu_buttons,                  0,   0, 140, 400, NULL,                        0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu room_menu =
 { 2, 0, 1, room_menu_buttons,                  0,   0, 140, 400, NULL,                        0, 0, 0, 0, NULL,                    0, 0, 1 };
struct GuiMenu spell_menu =
 { 3, 0, 1, spell_menu_buttons,                 0,   0, 140, 400, NULL,                        0, 0, 0, 0, NULL,                    0, 0, 1 };
struct GuiMenu spell_lost_menu =
 { 38, 0, 1, spell_lost_menu_buttons,           0,   0, 140, 400, NULL,                        0, 0, 0, 0, NULL,                    0, 0, 1 };
struct GuiMenu trap_menu =
 { 4, 0, 1, trap_menu_buttons,                  0,   0, 140, 400, NULL,                        0, 0, 0, 0, NULL,                    0, 0, 1 };
struct GuiMenu creature_menu =
 { 5, 0, 1, creature_menu_buttons,              0,   0, 140, 400, gui_activity_background,     0, 0, 0, 0, NULL,                    0, 0, 1 };
struct GuiMenu event_menu =
 { 6, 0, 1, event_menu_buttons,                 0,   0, 140, 400, NULL,                        0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu options_menu =
 { 8, 0, 1, options_menu_buttons,             999, 999, 308, 120, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu instance_menu =
 { 9, 0, 1, instance_menu_buttons,            999, 999, 318, 120, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu query_menu =
 { 7, 0, 1, query_menu_buttons,                 0,   0, 140, 400, NULL,                        0, 0, 0, 0, NULL,                    0, 0, 1 };
struct GuiMenu quit_menu =
 { 10, 0, 1, quit_menu_buttons,               999, 999, 264, 116, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu load_menu =
 { 11, 0, 4, load_menu_buttons,               999, 999, 436, 350, gui_pretty_background,       0, 0, 0, 0, init_load_menu,          0, 1, 0 };
struct GuiMenu save_menu =
 { 12, 0, 4, save_menu_buttons,               999, 999, 436, 350, gui_pretty_background,       0, 0, 0, 0, init_save_menu,          0, 1, 0 };
struct GuiMenu video_menu =
 { 13, 0, 4, video_menu_buttons,              999, 999, 160, 170, gui_pretty_background,       0, 0, 0, 0, init_video_menu,         0, 1, 0 };
struct GuiMenu sound_menu =
 { 14, 0, 4, sound_menu_buttons,              999, 999, 280, 170, gui_pretty_background,       0, 0, 0, 0, init_audio_menu,         0, 1, 0 };
struct GuiMenu error_box =
 { 15, 0, 1, error_box_buttons,               999, 999, 280, 180, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu text_info_menu =
 { 16, 0, 4, text_info_buttons,               160, 316, 480,  86, gui_round_glass_background,  0, 0, 0, 0, reset_scroll_window,     0, 0, 0 };
struct GuiMenu hold_audience_menu =
 { 17, 0, 4, hold_audience_buttons,           999, 999, 200, 116, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu dungeon_special_menu =
 { 27, 0, 4, dungeon_special_buttons,         160, 316, 480,  86, gui_round_glass_background,  0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu resurrect_creature_menu =
 { 28, 0, 4, resurrect_creature_buttons,      999, 999, 350, 300, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu transfer_creature_menu =
 { 29, 0, 4, transfer_creature_buttons,       999, 999, 350, 300, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu armageddon_menu =
 { 30, 0, 4, armageddon_buttons,              999, 999, 200, 116, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu frontend_main_menu =
 { 18, 0, 1, frontend_main_menu_buttons,        0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_load_menu =
 { 19, 0, 1, frontend_load_menu_buttons,        0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_net_service_menu =
 { 20, 0, 1, frontend_net_service_buttons,      0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_net_session_menu =
 { 21, 0, 1, frontend_net_session_buttons,      0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_net_start_menu =
 { 22, 0, 1, frontend_net_start_buttons,        0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_net_modem_menu =
 { 23, 0, 1, frontend_net_modem_buttons,        0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_net_serial_menu =
 { 24, 0, 1, frontend_net_serial_buttons,       0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_statistics_menu =
 { 25, 0, 1, frontend_statistics_buttons,       0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_high_score_table_menu =
 { 26, 0, 1, frontend_high_score_score_buttons, 0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu creature_query_menu1 =
 { 31, 0, 1, creature_query_buttons1,           0,   0, 140, 400, gui_creature_query_background1,0,0,0, 0, NULL,                    0, 0, 1 };
struct GuiMenu creature_query_menu2 =
 { 35, 0, 1, creature_query_buttons2,           0,   0, 140, 400, gui_creature_query_background1,0,0,0, 0, NULL,                    0, 0, 1 };
struct GuiMenu creature_query_menu3 =
 { 32, 0, 1, creature_query_buttons3,           0,   0, 140, 400, gui_creature_query_background2,0,0,0, 0, NULL,                    0, 0, 1 };
struct GuiMenu battle_menu =
 { 34, 0, 4, battle_buttons,                  160, 300, 480, 102, gui_round_glass_background,  0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu frontend_define_keys_menu =
 { 36, 0, 1, frontend_define_keys_buttons,      0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, NULL,                    0, 0, 0 };
struct GuiMenu autopilot_menu =
 { 37, 0, 4, autopilot_menu_buttons,          999, 999, 224, 120, gui_pretty_background,       0, 0, 0, 0, NULL,                    0, 1, 0 };
struct GuiMenu frontend_option_menu =
 { 39, 0, 1, frontend_option_buttons,           0,   0, 640, 480, frontend_copy_background,    0, 0, 0, 0, frontend_init_options_menu,0,0,0 };

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
};

struct FrontEndButtonData frontend_button_info[] = {
    {0,   0, 0},
    {87,  1, 0},
    {104, 1, 1},
    {89,  1, 1},
    {91,  1, 1},
    {103, 1, 1},
    {92,  1, 1},
    {89,  1, 0},
    {90,  1, 1},
    {93,  1, 1},
    {94,  1, 0},
    {95,  1, 0},
    {146, 1, 0},
    {144, 1, 1},
    {143, 1, 1},
    {145, 1, 1},
    {147, 1, 1},
    {201, 0, 1},
    {201, 0, 1},
    {140, 1, 1},
    {201, 0, 1},
    {201, 0, 1},
    {150, 1, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {139, 1, 2},
    {152, 1, 2},
    {149, 1, 2},
    {151, 1, 2},
    {141, 1, 2},
    {142, 1, 2},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {153, 1, 0},
    {154, 1, 0},
    {97,  1, 2},
    {96,  1, 2},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {99,  1, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {155, 1, 1},
    {156, 1, 1},
    {21,  2, 1},
    {158, 1, 1},
    {201, 0, 1},
    {98,  1, 1},
    {22,  2, 1},
    {22,  2, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {162, 1, 1},
    {163, 1, 1},
    {100, 1, 0},
    {175, 1, 0},
    {201, 1, 0},
    {202, 1, 2},
    {159, 1, 1},
    {201, 0, 1},
    {201, 0, 1},
    {201, 0, 1},
    {212, 1, 0},
    {201, 0, 1},
    {201, 0, 1},
    {212, 1, 1},
    {204, 2, 0},
    {204, 2, 1},
    {72,  3, 1},
    {206, 2, 1},
    {82,  3, 1},
    {81,  3, 1},
    {75,  3, 1},
    {77,  3, 1},
    {175, 1, 1},
};

int frontend_set_state(long nstate);

void LbDataLoadSetModifyFilenameFunction(ModDL_Fname_Func nmodify_dl_filename_func)
{
  _DK_modify_data_load_filename_function=nmodify_dl_filename_func;
}

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 1 byte long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_byte(unsigned char *flags,short mask,short value)
{
  if (value) value=mask;
  *flags = (*flags ^ (unsigned char)value) & mask ^ *flags;
}

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 4 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_dword(unsigned long *flags,unsigned long mask,unsigned long value)
{
  if (value) value=mask;
  *flags = (*flags ^ (unsigned long)value) & mask ^ *flags;
}

TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data)
{
  return _DK_LbNetwork_Init(srvcp,guid,maxplayrs,exchng_buf,exchng_size,locplayr,init_data);
}

void gui_activity_background(struct GuiMenu *gmnu)
{
  _DK_gui_activity_background(gmnu);
}

void gui_pretty_background(struct GuiMenu *gmnu)
{
  _DK_gui_pretty_background(gmnu);
}

void frontend_copy_background(struct GuiMenu *gmnu)
{
  _DK_frontend_copy_background(gmnu);
}

void gui_round_glass_background(struct GuiMenu *gmnu)
{
  _DK_gui_round_glass_background(gmnu);
}

void gui_creature_query_background1(struct GuiMenu *gmnu)
{
  _DK_gui_creature_query_background1(gmnu);
}

void gui_creature_query_background2(struct GuiMenu *gmnu)
{
  _DK_gui_creature_query_background2(gmnu);
}

void reset_scroll_window(struct GuiMenu *gmnu)
{
  _DK_reset_scroll_window(gmnu);
}

void init_load_menu(struct GuiMenu *gmnu)
{
  _DK_init_load_menu(gmnu);
}

void init_save_menu(struct GuiMenu *gmnu)
{
  _DK_init_save_menu(gmnu);
}

void init_video_menu(struct GuiMenu *gmnu)
{
  _DK_init_video_menu(gmnu);
}

void init_audio_menu(struct GuiMenu *gmnu)
{
  _DK_init_audio_menu(gmnu);
}

void frontend_init_options_menu(struct GuiMenu *gmnu)
{
  _DK_frontend_init_options_menu(gmnu);
}

void frontnet_service_up_maintain(struct GuiButton *gbtn)
{
  if (net_service_scroll_offset != 0)
    gbtn->field_0 |= 0x08;
  else
    gbtn->field_0 ^= (gbtn->field_0 & 0x08);
}

void frontnet_service_down_maintain(struct GuiButton *gbtn)
{
  if (net_number_of_services-1 > net_service_scroll_offset)
    gbtn->field_0 |= 0x08;
  else
    gbtn->field_0 ^= (gbtn->field_0 & 0x08);
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
  if (net_service_scroll_offset+(long)gbtn->field_33-45 < net_number_of_services)
    gbtn->field_0 |= 0x08;
  else
    gbtn->field_0 ^= (gbtn->field_0 & 0x08);
}

void frontnet_draw_service_button(struct GuiButton *gbtn)
{
//  _DK_frontnet_draw_service_button(gbtn);
  int srvidx;
  // Find and verify selected network service
  srvidx = (long)(gbtn->field_33) + net_service_scroll_offset - 45;
  if ( srvidx >= net_number_of_services )
    return;
  // Select font to draw
  int fntidx;
  fntidx = frontend_button_info[(long)gbtn->field_33].field_2;
  if (((long)gbtn->field_33 != 0) && (frontend_mouse_over_button == (long)gbtn->field_33))
      fntidx = 2;
  lbFontPtr = frontend_font[fntidx];
  // Set drawing windsow
  int height = 0;
  lbDisplay.DrawFlags = 0x20;
  if ( lbFontPtr!=NULL )
      height = lbFontPtr[1].SHeight;
  _DK_LbTextSetWindow(gbtn->field_1D, gbtn->field_1F, gbtn->width, height);
  //Draw the text
  _DK_LbTextDraw(0, 0, net_service[srvidx]);
}

void frontnet_service_select(struct GuiButton *gbtn)
{
//  _DK_frontnet_service_select(gbtn);
  int srvidx;
  srvidx = (long)(gbtn->field_33) + net_service_scroll_offset - 45;
  if ( (game.one_player) && (srvidx+1>=_DK_net_number_of_services) )
  {
    _DK_fe_network_active = 0;
    frontend_set_state(24);
  } else
  if (srvidx <= 0)
  {
    frontend_set_state(16);
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
  if (game_index<number_of_saved_games)
    gbtn->field_0 |= 0x08;
  else
    gbtn->field_0 ^= (gbtn->field_0 & 0x08);
}


short inline calculate_moon_phase(short add_to_log)
{
  //Moon phase calculation
  _DK_phase_of_moon=PhaseOfMoon::Calculate();
  if ((_DK_phase_of_moon > -0.05) && (_DK_phase_of_moon < 0.05))
  {
    if (add_to_log)
      LbSyncLog("Full moon %.4f\n", _DK_phase_of_moon);
    game.is_full_moon = 1;
  } else
  {
    if (add_to_log)
      LbSyncLog("Moon phase %.4f\n", _DK_phase_of_moon);
    game.is_full_moon = 0;
  }
  return game.is_full_moon;
}

short show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,long tmdelay)
{
  static const char *func_name="show_rawimage_screen";
      if (height>lbDisplay.PhysicalScreenHeight)
           height=lbDisplay.PhysicalScreenHeight;
      LbPaletteSet(pal);
      long end_time;
      end_time = LbTimerClock() + tmdelay;
      long tmdelta;
      tmdelta = tmdelay/100;
      if (tmdelta>100) tmdelta=100;
      if (tmdelta<5) tmdelta=5;
      while ( LbTimerClock() < end_time )
      {
          LbWindowsControl();
          if ( LbScreenLock() == 1 )
          {
              unsigned char *raw_line;
              unsigned char *scrn_line;
              raw_line = raw;
              scrn_line = lbDisplay.WScreen;
              int i;
              for ( i = 0; i < height; i++ )
              {
                  memcpy(scrn_line, raw_line, width);
                  raw_line += width;
                  scrn_line += lbDisplay.GraphicsScreenWidth;
              }
              LbScreenUnlock();
          }
          LbScreenSwap();
          if ( lbKeyOn[KC_SPACE] || lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_RETURN] ||
               lbDisplay.LeftButton || lbDisplay.RightButton )
          {
              lbKeyOn[KC_SPACE] = 0;
              lbKeyOn[KC_ESCAPE] = 0;
              lbKeyOn[KC_RETURN] = 0;
              lbDisplay.LeftButton = 0;
              lbDisplay.RightButton = 0;
              break;
          }
          LbSleepFor(tmdelta);
      }
}

short LbPaletteStopOpenFade(void)
{
    _DK_lbfade_open=0;
    return 1;
}

void ProperFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
    if ( _DK_lbUseSdk )
    {
        long last_loop_time;
        last_loop_time = LbTimerClock();
        while ( LbPaletteFade(pal, n, flg) < n )
        {
          if ( !lbKeyOn[KC_SPACE] && !lbKeyOn[KC_ESCAPE] && !lbKeyOn[KC_RETURN] &&
               !lbDisplay.LeftButton && !lbDisplay.RightButton )
          {
            LbSleepUntil(last_loop_time+50);
          }
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

void ProperForcedFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
    if (flg==Lb_PALETTE_FADE_OPEN)
    {
        LbPaletteFade(pal, n, flg);
        return;
    }
    if ( _DK_lbUseSdk )
    {
        long last_loop_time;
        last_loop_time = LbTimerClock();
        while ( LbPaletteFade(pal, n, flg) < n )
        {
          LbSleepUntil(last_loop_time+50);
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

void inline fade_in(void)
{
  ProperFadePalette(_DK_frontend_palette, 8, Lb_PALETTE_FADE_OPEN);
}

void inline fade_out(void)
{
  ProperFadePalette(0, 8, Lb_PALETTE_FADE_CLOSED);
  LbScreenClear(0);
}

short thing_is_special(struct Thing *thing)
{
  return (thing->field_1F==1) && (_DK_object_to_special[thing->field_1A]);
}

short copy_lowres_image_to_screen(const unsigned char *buf,const int img_width,const int img_height)
{
  if (LbScreenLock()!=1)
    return false;
  int w,h;
  unsigned char *dst;
  const unsigned char *src;
  if ((lbDisplay.ScreenMode!=Lb_SCREEN_MODE_640_400_8) &&
      (lbDisplay.ScreenMode!=Lb_SCREEN_MODE_640_480_8))
  {
    dst = lbDisplay.WScreen;
    src = buf;
    for (h=img_height;h>0;h--)
    {
          memcpy(dst, src, img_width);
          src += img_width;
          dst += lbDisplay.GraphicsScreenWidth;
    }
  } else
  {
        if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
        {
          dst = lbDisplay.WScreen + 40*lbDisplay.GraphicsScreenWidth;
        } else
        {
          dst = lbDisplay.WScreen;
        }
        src = buf;
        for (h=img_height;h>0;h--)
        {
          for (w=img_width;w>0;w--)
          {
            dst[0] = *src;
            dst[1] = *src;
            dst += 2;
            src++;
          }
          memcpy(dst+lbDisplay.GraphicsScreenWidth-640, dst-640, 640);
          dst += (lbDisplay.GraphicsScreenWidth<<1)-640;
        }
  }
  LbScreenUnlock();
  LbScreenSwap();
  return true;
}


/*
 * Displays the 320x200 loading screen.
 * Will work properly even on 640x4?0 resolutions.
 * @return Returns true on success.
 */
short display_lores_loading_screen(void)
{
  static const char *func_name="display_lores_loading_screen";
  const int img_width = 320;
  const int img_height = 200;
  char fname[256];
  unsigned char *buf=NULL;

  memset(palette_buf, 0, 768);
  LbPaletteSet(palette_buf);

  check_cd_in_drive();
  short dproceed=true;
  if (dproceed)
  {
      buf=(unsigned char *)malloc(img_width*img_height);
      if (buf==NULL)
      {
          error(func_name, 1056, "Cannot allocate bitmap buffer");
          dproceed = false;
      }
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory, "data", "loading.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory,"data","loading.pal");
      check_cd_in_drive();
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed=copy_lowres_image_to_screen(buf,img_width,img_height);
  }
  free(buf);
  return dproceed;
}

/*
 * Displays the 640x480 loading screen.
 * Will work properly only on high resolutions.
 * @return Returns true on success.
 */
short display_hires_loading_screen(void)
{
  static const char *func_name="display_hires_loading_screen";
  const int img_width = 640;
  const int img_height = 480;
  char fname[256];
  unsigned char *buf=NULL;

  memset(palette_buf, 0, 768);
  LbPaletteSet(palette_buf);

  check_cd_in_drive();
  short dproceed=true;
  if (dproceed)
  {
      buf=(unsigned char *)malloc(img_width*img_height);
      if (buf==NULL)
      {
          error(func_name, 1056, "Cannot allocate bitmap buffer");
          dproceed = false;
      }
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory, "fxdata", "loading_640.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory,"fxdata","loading_640.pal");
      check_cd_in_drive();
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed = (LbScreenLock()==1);
  }
  if ( dproceed )
  {
      int w,h;
      unsigned char *dst;
      unsigned char *src;
      int cp_height;
      if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
      {
          src = buf;
          cp_height=480;
      } else
      {
          src = buf + 40*img_width;
          cp_height=400;
      }
      dst = lbDisplay.WScreen;
      for (h=cp_height;h>0;h--)
      {
          memcpy(dst, src, img_width);
          src += img_width;
          dst += lbDisplay.GraphicsScreenWidth;
      }
      LbScreenUnlock();
      LbScreenSwap();
  }
  free(buf);
  return dproceed;
}

short display_loading_screen(void)
{
    short done=false;
    if ((lbDisplay.ScreenMode==Lb_SCREEN_MODE_640_400_8) ||
        (lbDisplay.ScreenMode==Lb_SCREEN_MODE_640_480_8))
    {
        done=display_hires_loading_screen();
    }
    if (!done)
    {
        done=display_lores_loading_screen();
    }
    LbPaletteStopOpenFade();
    ProperForcedFadePalette(palette_buf, 64, Lb_PALETTE_FADE_CLOSED);
    return done;
}

/*
 * Plays a smacker animation file and sets frontend state to nstate.
 * @param nstate Frontend state; -1 means no change, -2 means don't even
 *    change screen mode.
 * @return Returns false if fatal error occured and probram execution should end.
 */
short play_smacker_file(char *filename, int nstate)
{
  static const char *func_name="play_smacker_file";
  unsigned int movie_flags = 0;
  if ( _DK_SoundDisabled )
    movie_flags |= 0x01;
  short result=1;

  if ((result)&&(nstate>-2))
  {
    if ( _DK_setup_screen_mode_minimal(Lb_SCREEN_MODE_320_200_8) )
    {
      LbMouseChangeSprite(0);
      LbScreenClear(0);
      LbScreenSwap();
    } else
      result=0;
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if ( play_smk_(filename, 0, movie_flags | 0x100)==0 )
    {
      error(func_name, 2357, "Smacker play error");
      result=0;
    }
  }
  if (nstate>-2)
  {
    if ( !_DK_setup_screen_mode_minimal(Lb_SCREEN_MODE_640_480_8) )
    {
      _DK_FatalError = 1;
      exit_keeper = 1;
      return 0;
    }
  } else
  {
    memset(_DK_frontend_palette, 0, PALETTE_SIZE);
  }
  LbScreenClear(0);
  LbScreenSwap();
  LbPaletteSet(_DK_frontend_palette);
  if ( nstate >= 0 )
    frontend_set_state(nstate);
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
  lbDisplay.MiddleButton = 0;
  if (nstate>-2)
    _DK_LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
  return result;
}

short sound_emitter_in_use(long emidx)
{
  return (emidx!=0) && (_DK_emitter[emidx].flags & 1);
}

char game_is_busy_doing_gui_string_input(void)
{
  return _DK_game_is_busy_doing_gui_string_input();
}

short get_global_inputs()
{
  if ( game_is_busy_doing_gui_string_input() && (_DK_input_button==NULL) )
    return false;
  struct PlayerInfo *player=&(game.players[my_player_number]);
  struct Packet *pckt;
  long keycode;
  if ( (player->field_0 & 0x04) != 0 )
  {
    if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_RETURN]) )
    {
      pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt, 14, 0, 0, 0, 0);
      lbKeyOn[KC_RETURN] = 0;
      lbInkey = 0;
      return true;
    }
    lbFontPtr = winfont;
    int msg_width = pixel_size * _DK_LbTextStringWidth(player->strfield_463);
    if ( (lbInkey == 14) || (msg_width < 450) )
    {
      pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt,121,lbInkey,0,0,0);
      lbKeyOn[lbInkey] = 0;
      lbInkey = 0;
    }
    return true;
  }
  if ( player->field_452 == 1 )
  {
      if ( ((game.numfield_A & 0x01) != 0) && (key_modifiers==KM_NONE) && (lbKeyOn[KC_RETURN]) )
      {
        lbKeyOn[KC_RETURN] = 0;
        pckt = &game.packets[player->field_B];
        _DK_set_packet_action(pckt, 13, 0, 0, 0, 0);
        return true;
      }
  }
  int idx;
  for (idx=KC_F1;idx<=KC_F8;idx++)
  {
      if ( (key_modifiers==KM_ALT) && (lbKeyOn[idx]) )
      {
        lbKeyOn[idx] = 0;
        pckt = &game.packets[player->field_B];
        _DK_set_packet_action(pckt, 108, idx-KC_F1, 0, 0, 0);
        return true;
      }
  }
  if ( (player->field_4B0 != 14) && (player->field_4B0 != 15) && (input_button==0) )
  {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        lbKeyOn[keycode] = 0;
        pckt = &game.packets[player->field_B];
        _DK_set_packet_action(pckt, 22, 0, 0, 0, 0);
        return true;
      }
  }
  if ((game.numfield_C & 0x01) != 0)
      return true;
  if ( (key_modifiers==KM_CONTROL) && (lbKeyOn[KC_ADD]))
  {
        lbKeyOn[KC_ADD] = 0;
        game.timingvar1 += 2;
        if (game.timingvar1 < 0)
          game.timingvar1 = 0;
        else
        if ( game.timingvar1 > 64 )
          game.timingvar1 = 64;
  }
  if ( (key_modifiers==KM_CONTROL) && (lbKeyOn[KC_SUBTRACT]))
  {
      lbKeyOn[KC_SUBTRACT] = 0;
      game.timingvar1 -= 2;
        if (game.timingvar1 < 0)
        game.timingvar1 = 0;
      else
      if ( game.timingvar1 > 64 )
        game.timingvar1 = 64;
  }
  if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_SUBTRACT]))
  {
      lbKeyOn[KC_SUBTRACT] = 0;
      if ( player->field_450 < 0x800u )
      {
        pckt = &game.packets[player->field_B];
        _DK_set_packet_action(pckt, 28, 2 * player->field_450, 0, 0, 0);
      }
  }
  if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_ADD]))
  {
      lbKeyOn[KC_ADD] = 0;
      if ( player->field_450 > 0x80u )
      {
        pckt = &game.packets[player->field_B];
        _DK_set_packet_action(pckt, 28, player->field_450 >> 1, 0, 0, 0);
      }
  }
  if ( (key_modifiers==KM_ALT) && (lbKeyOn[KC_R]) )
  {
      lbKeyOn[KC_R] = 0;
      pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt, 21, 0, 0, 0, 0);
      return true;
  }
  if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_SPACE]))
  {
      if ( player->field_29 )
      {
        pckt = &game.packets[player->field_B];
        _DK_set_packet_action(pckt, 5, 0, 0, 0, 0);
        lbKeyOn[KC_SPACE] = 0;
        return true;
      }
  }
  if ( lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT] )
  {
      if ( lbKeyOn[KC_M] )
      {
        lbKeyOn[KC_M] = 0;
        if ( game.numfield_A & 0x08 )
        {
          game.numfield_A &= 0xF7u;
          anim_stop();
        } else
        if ( anim_record() )
        {
          game.numfield_A |= 0x08;
        }
      }
      if ( lbKeyOn[KC_C] )
      {
        lbKeyOn[KC_C] = 0;
        game.numfield_A |= 0x10;
      }
  }
  if ( _DK_is_game_key_pressed(29, &keycode, 0) )
  {
      lbKeyOn[keycode] = 0;
      pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt, 111, 0, 0, 0, 0);
  }
  return false;
}

void play_non_3d_sample(long sample_idx)
{
  static const char *func_name="play_non_3d_sample";
  if ( _DK_SoundDisabled )
    return;
  if ( GetCurrentSoundMasterVolume() <= 0 )
    return;
  if (_DK_Non3DEmitter!=0)
    if ( sound_emitter_in_use(_DK_Non3DEmitter) == 0 )
    {
      error(func_name, 263, "Non 3d Emitter has been deleted!");
      _DK_Non3DEmitter = 0;
    }
  if (_DK_Non3DEmitter!=0)
  {
    _DK_S3DAddSampleToEmitterPri(_DK_Non3DEmitter, sample_idx, 0, 100, 256, 0, 3, 8, 2147483646);
  } else
  {
    _DK_Non3DEmitter = _DK_S3DCreateSoundEmitterPri(0, 0, 0, sample_idx, 0, 100, 256, 0, 8, 2147483646);
  }
}

short setup_network_service(int srvidx)
{
//  return _DK_setup_network_service(srvidx);
  struct SerialInitData *init_data;
  long maxplayrs;

  switch (srvidx)
  {
  case 0:
      maxplayrs = 2;
      init_data = &_DK_net_serial_data;
      game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players serial network.\n",maxplayrs);
      break;
  case 1:
      maxplayrs = 2;
      init_data = &_DK_net_modem_data;
      game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players modem network.\n",maxplayrs);
      break;
  case 2:
      maxplayrs = 4;
      init_data = NULL;
      game.flags_font &= 0xEFu;
      LbSyncLog("Initializing %d-players IPX network.\n",maxplayrs);
      break;
  default:
      maxplayrs = 4;
      init_data = NULL;
      game.flags_font &= 0xEFu;
      LbSyncLog("Initializing %d-players type %d network.\n",maxplayrs,srvidx);
      break;
  }
  memset(&_DK_net_player_info, 0, sizeof(struct TbNetworkPlayerInfo));
  if ( LbNetwork_Init(srvidx, _DK_net_guid, maxplayrs, &_DK_net_screen_packet, 0xCu, &_DK_net_player_info, init_data) )
  {
    if (srvidx != 0)
      _DK_process_network_error(-800);
    return 0;
  }
  _DK_net_service_index_selected = srvidx;
  if ( game.flags_font & 0x10 )
    ;//rndseed_nullsub();
  frontend_set_state(5);
  return 1;
}

void frontnet_service_update(void)
{
  _DK_frontnet_service_update();
}

void frontnet_session_update(void)
{
  _DK_frontnet_session_update();
}

void frontnet_modem_update(void)
{
  _DK_frontnet_modem_update();
}

void frontnet_serial_update(void)
{
  _DK_frontnet_serial_update();
}

void frontnet_start_update(void)
{
  _DK_frontnet_start_update();
}

/*
 * Displays 'legal' screens, intro and initializes basic game data.
 * If true is returned, then all files needed for startup were loaded,
 * and there should be the loading screen visible.
 * @return Returns true on success, false on error which makes the 
 *   gameplay impossible (usually files loading failure).
 * @note The current screen resolution at end of this function may vary.
 */
short setup_game(void)
{
  static const char *func_name="setup_game";
  // CPU and memory status
  struct CPU_INFO cpu_info;
  struct _MEMORYSTATUS msbuffer;
  char filename[DISKPATH_SIZE];

  // Do only a very basic setup
  
  _DK_get_cpu_info(&cpu_info);
  msbuffer.dwLength = 32;
  GlobalMemoryStatus(&msbuffer);
  if ( msbuffer.dwTotalPhys <= (8*1024*1024) )
      _DK_mem_size = 8;
  else
  if ( msbuffer.dwTotalPhys <= (16*1024*1024) )
      _DK_mem_size = 16;
  else
  if ( msbuffer.dwTotalPhys <= (24*1024*1024) )
      _DK_mem_size = 24;
  else
      _DK_mem_size = 32;
  LbSyncLog("PhysicalMemory %d\n", _DK_mem_size);

  // Configuration file
  if ( !_DK_load_configuration() )
  {
      error(func_name, 1912, "Configuration load error.");
      return 0;
  }

  //Moon phase calculation
  calculate_moon_phase(true);

  LbIKeyboardOpen();

  if ( _DK_LbDataLoadAll(legal_load_files) != 0 )
  {
      error(func_name, 1911, "Error on allocation/loading of legal_load_files.");
      return 0;
  }

  short result=1;
  short need_clean;

  // This should be closed in int initial_setup(void)

  // View the legal screen

  memset(_DK_palette, 0, PALETTE_SIZE);
  if ( LbScreenSetup(Lb_SCREEN_MODE_640_480_8, LEGAL_WIDTH, LEGAL_HEIGHT, _DK_palette, 1, 0) != 1 )
  {
      error(func_name, 1912, "Screen mode setup error.");
      return 0;
  }

  unsigned char *legal_raw;

  if ( result )
  {
      LbScreenClear(0);
      LbScreenSwap();
      legal_raw = LbMemoryAlloc(LEGAL_WIDTH*LEGAL_HEIGHT);
      if (legal_raw == NULL)
      {
          error(func_name, 1456, "Unable to open allocate screen buffer");
          result=0;
      }
  }

  if ( result )
  {
      int loaded_size=LbFileLoadAt("data/legal.raw", legal_raw);
      if ( loaded_size != LEGAL_WIDTH*LEGAL_HEIGHT )
      {
          error(func_name, 1457, "Unable to load legal image data");
          result=0;
      }
  }
  if ( result )
  {
      int loaded_size=LbFileLoadAt("data/legal.pal", _DK_palette);
      if ( loaded_size != PALETTE_SIZE )
      {
          error(func_name, 1458, "Unable to load legal image palette");
          result=0;
      }
  }

  if ( result )
      show_rawimage_screen(legal_raw,_DK_palette,LEGAL_WIDTH,LEGAL_HEIGHT,3000);
  else
      LbSyncLog("%s - Legal image skipped\n", func_name);

  // Now do more setup

  _DK_init_sound();

  // View second legal screen

  result = (legal_raw!=NULL);

  if ( result )
  {
      int loaded_size=LbFileLoadAt("fxdata/startup_fx.raw", legal_raw);
      if ( loaded_size != LEGAL_WIDTH*LEGAL_HEIGHT )
      {
          error(func_name, 1477, "Unable to open startup_fx Screen");
          result=0;
      }
  }
  if ( result )
  {
      int loaded_size=LbFileLoadAt("fxdata/startup_fx.pal", _DK_palette);
      if ( loaded_size != PALETTE_SIZE )
      {
          error(func_name, 1484, "Unable to open startup_fx Palette");
          result=0;
      }
  }

  if ( result )
      show_rawimage_screen(legal_raw,_DK_palette,LEGAL_WIDTH,LEGAL_HEIGHT,4000);
  else
      LbSyncLog("%s - startup_fx image skipped\n", func_name);

  {
      LbMemoryFree(legal_raw);
      memset(_DK_palette, 0, PALETTE_SIZE);
      LbScreenClear(0);
      LbScreenSwap();
  }

  // View Bullfrog company logo animation when new moon
  if ((_DK_phase_of_moon < -0.95) || (_DK_phase_of_moon > 0.95))
  if ( !game.no_intro )
  {
      sprintf(filename, "fxdata/%s","bullfrog.smk");
      result=play_smacker_file(filename, -2);
      if ( !result )
        error(func_name, 1483, "Unable to play new moon movie");
  }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!game.no_intro) )
  {
    int mode_ok = LbScreenSetup(Lb_SCREEN_MODE_320_200_8, 320, 200, _DK_palette, 2, 0);
    if (mode_ok != 1)
    {
      error(func_name, 1500, "Can't enter 320x200 screen mode");
      result=0;
    }
  }

  if ( result )
  {
      LbScreenClear(0);
      LbScreenSwap();
      result=check_cd_in_drive();
  }
  if ( result && (!game.no_intro) )
  {
      sprintf(filename, "%s/ldata/%s",_DK_datafiles_path,"intromix.smk");
      result=play_smacker_file(filename, -2);
  }
  if ( !result )
      LbSyncLog("%s - some problem prevented playing intro\n", func_name);
  // Intro problems shouldn't force the game to quit,
  // so we're re-setting the result flag
  result = 1;

  if ( result )
  {
        display_loading_screen();
  }
  _DK_LbDataFreeAll(legal_load_files);

  char *text_end;
  if ( result )
  {
      long filelen;
      filelen = LbFileLengthRnc("data/text.dat");
      _DK_strings_data = (char *)LbMemoryAlloc(filelen + 256);
      if ( _DK_strings_data == NULL )
      {
        exit(1);
      }
      text_end = _DK_strings_data+filelen+255;
      int loaded_size=LbFileLoadAt("data/text.dat", _DK_strings_data);
      if (loaded_size<2*STRINGS_MAX)
      {
          error(func_name, 1501, "Strings data too small");
          result = 0;
      }
  }
  if ( result )
  {
      char **text_arr = _DK_strings;
      int text_idx;
      char *text_ptr;
      text_idx = STRINGS_MAX;
      text_ptr = _DK_strings_data;
      while (text_idx>=0)
      {
        if (text_ptr>=text_end)
        {
            result = 0;
            break;
        }
        *text_arr = text_ptr;
        text_arr++;
        char chr_prev;
        do {
            chr_prev = *text_ptr;
            text_ptr++;
        } while ((chr_prev!='\0')&&(text_ptr<text_end));
        text_idx--;
      }
  }

  if ( result )
  {
      _DK_IsRunningMark();
      if ( !_DK_initial_setup() )
      {
        error(func_name, 1502, "Initial setup failed");
        result = 0;
      }
  }
  if ( result )
  {
      _DK_load_settings();
      if ( !_DK_setup_game_sound_heap() )
      {
        error(func_name, 1503, "Sound heap setup failed");
        result = 0;
      }
  }

  if ( result )
  {
        _DK_init_creature_scores();
        if ( cpu_info.field_0 )
        {
            if ( ((cpu_info.field_0>>8) & 0x0Fu) >= 0x06 )
              _DK_set_cpu_mode(1);
        }
        _DK_set_gamma(_DK_settings.gamma_correction, 0);
        SetRedbookVolume(_DK_settings.redbook_volume);
        SetSoundMasterVolume(_DK_settings.sound_volume);
        SetMusicMasterVolume(_DK_settings.sound_volume);
        _DK_setup_3d();
        _DK_setup_stuff();
        _DK_init_lookups();
        result = 1;
  }
  return result;
}

void update_left_button_released(void)
{
  _DK_left_button_released = 0;
  _DK_left_button_double_clicked = 0;
  if ( lbDisplay.LeftButton )
  {
    _DK_left_button_held = 1;
    _DK_left_button_held_x = lbDisplay.MMouseX * pixel_size;
    _DK_left_button_held_y = lbDisplay.MMouseY * pixel_size;
  }
  if (_DK_left_button_held)
  {
    if (!lbDisplay.MLeftButton)
    {
      _DK_left_button_released = 1;
      _DK_left_button_held = 0;
      _DK_left_button_released_x = lbDisplay.MMouseX * pixel_size;
      _DK_left_button_released_y = lbDisplay.MMouseY * pixel_size;
      if ( _DK_left_button_click_space_count < 5 )
      {
        _DK_left_button_double_clicked = 1;
        _DK_left_button_double_clicked_x = _DK_left_button_released_x;
        _DK_left_button_double_clicked_y = _DK_left_button_released_y;
      }
      _DK_left_button_click_space_count = 0;
    }
  } else
  {
    if ( _DK_left_button_click_space_count < 2147483647 )
      _DK_left_button_click_space_count++;
  }
}

void update_right_button_released(void)
{
  _DK_right_button_released = 0;
  _DK_right_button_double_clicked = 0;
  if (lbDisplay.RightButton)
  {
    _DK_right_button_held = 1;
    _DK_right_button_held_x = lbDisplay.MMouseX * pixel_size;
    _DK_right_button_held_y = lbDisplay.MMouseY * pixel_size;
  }
  if ( _DK_right_button_held )
  {
    if ( !lbDisplay.MRightButton )
    {
      _DK_right_button_released = 1;
      _DK_right_button_held = 0;
      _DK_right_button_released_x = lbDisplay.MMouseX * pixel_size;
      _DK_right_button_released_y = lbDisplay.MMouseY * pixel_size;
      if ( _DK_right_button_click_space_count < 5 )
      {
        _DK_right_button_double_clicked = 1;
        _DK_right_button_double_clicked_x = _DK_right_button_released_x;
        _DK_right_button_double_clicked_y = _DK_right_button_released_y;
      }
      _DK_right_button_click_space_count = 0;
    }
  } else
  {
    if ( _DK_right_button_click_space_count < 2147483647 )
      _DK_right_button_click_space_count++;
  }
}

void update_left_button_clicked(void)
{
  _DK_left_button_clicked = lbDisplay.LeftButton;
  _DK_left_button_clicked_x = lbDisplay.MouseX * pixel_size;
  _DK_left_button_clicked_y = lbDisplay.MouseY * pixel_size;
}

void update_right_button_clicked(void)
{
  _DK_right_button_clicked = lbDisplay.RightButton;
  _DK_right_button_clicked_x = lbDisplay.MouseX * pixel_size;
  _DK_right_button_clicked_y = lbDisplay.MouseY * pixel_size;
}

void update_mouse(void)
{
  update_left_button_released();
  update_right_button_released();
  update_left_button_clicked();
  update_right_button_clicked();
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
}

void input_eastegg(void)
{
  static const char *func_name="input_eastegg";
  //_DK_input_eastegg();return;
  // Maintain the FECKOFF cheat
  if ( lbKeyOn[KC_LSHIFT] )
  {
    switch (game.eastegg01_cntr)
    {
    case 0:
      if ( lbKeyOn[KC_F] )
        game.eastegg01_cntr++;
      break;
    case 1:
      if ( lbKeyOn[KC_E] )
        game.eastegg01_cntr++;
      break;
    case 2:
      if ( lbKeyOn[KC_C] )
        game.eastegg01_cntr++;
      break;
    case 3:
      if ( lbKeyOn[KC_K] )
        game.eastegg01_cntr++;
      break;
    case 4:
      if ( lbKeyOn[KC_O] )
        game.eastegg01_cntr++;
      break;
    case 5:
    case 6:
      if ( lbKeyOn[KC_F] )
        game.eastegg01_cntr++;
      break;
    }
  } else
  {
    game.eastegg01_cntr = 0;
  }
  // Maintain the JLW cheat
  if ( game.flags_font & 0x20 )
  {
    if ( lbKeyOn[KC_LSHIFT] && lbKeyOn[KC_RSHIFT] )
    {
      switch (game.eastegg02_cntr)
      {
      case 0:
        if ( lbKeyOn[KC_J] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      case 1:
        if ( lbKeyOn[KC_L] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      case 2:
        if ( lbKeyOn[KC_W] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      }
    } else
    {
      game.eastegg02_cntr = 0;
    }
  }
  // Maintain the SKEKSIS cheat
  if ( lbKeyOn[KC_LSHIFT] )
  {
    switch (_DK_eastegg03_cntr)
    {
    case 0:
    case 4:
      if ( _DK_lbInkey==KC_S )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 1:
    case 3:
      if ( _DK_lbInkey==KC_K )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 2:
      if ( _DK_lbInkey==KC_E )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 5:
      if ( _DK_lbInkey==KC_I )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 6:
      if ( _DK_lbInkey==KC_S )
      {
        _DK_eastegg03_cntr++;
        _DK_lbInkey=0;
        //'Your pants are definitely too tight'
        _DK_output_message(94, 0, 1);
      }
      break;
    }
  } else
  {
    _DK_eastegg03_cntr = 0;
  }
  if (_DK_lbInkey!=0)
  {
    _DK_eastegg03_cntr = 0;
  }
}

/*
 * Checks if the given screen point is over a gui menu.
 * @param x,y Screen coordinates to check.
 * @return Returns index of the menu, or -1 if there's no menu on this point.
 */
int point_is_over_gui_menu(long x, long y)
{
  int idx;
  int gidx = -1;
  const short gmax=8;//sizeof(_DK_active_menus)/sizeof(struct GuiMenu);
  for(idx=0;idx<gmax;idx++)
  {
    struct GuiMenu *gmnu;
    gmnu=&_DK_active_menus[idx];
    if (gmnu->field_1 != 2)
        continue;
    if ( gmnu->flgfield_1D == 0 )
        continue;
    short gx = gmnu->pos_x;
    if ((x >= gx) && (x < gx+gmnu->width))
    {
        short gy = gmnu->pos_y;
        if ((y >= gy) && (y < gy+gmnu->height))
            gidx=idx;
    }
  }
  return gidx;
}

int first_monopoly_menu(void)
{
  int idx;
  const short gmax=8;//sizeof(_DK_active_menus)/sizeof(struct GuiMenu);
  struct GuiMenu *gmnu;
  for (idx=0;idx<gmax;idx++)
  {
    gmnu=&_DK_active_menus[idx];
    if ((gmnu->field_1!=0) && (gmnu->flgfield_1E!=0))
        return idx;
  }
  return -1;
}

/*
 * Checks if mouse pointer is currently over a specific button.
 * @return Returns true it mouse is over the button.
 */
short check_if_mouse_is_over_button(struct GuiButton *gbtn)
{
  int mouse_x = lbDisplay.MMouseX*pixel_size;
  int mouse_y = _DK_GetMouseY();
  int x = gbtn->pos_x;
  int y = gbtn->pos_y;
  if ( (mouse_x >= x) && (mouse_x < x + gbtn->width)
    && (mouse_y >= y) && (mouse_y < y + gbtn->height)
    && (gbtn->field_0 & 4) )
    return true;
  return false;
}

inline void reset_scrolling_tooltip(void)
{
    _DK_tooltip_scroll_offset = 0;
    _DK_tooltip_scroll_timer = 25;
}

inline void do_sound_menu_click(void)
{
  play_non_3d_sample(61);
}

short setup_trap_tooltips(struct Coord3d *pos)
{
    struct Thing *thing=_DK_get_trap_for_position(_DK_map_to_slab[pos->x.stl.num],_DK_map_to_slab[pos->y.stl.num]);
    if (thing==NULL) return false;
    struct PlayerInfo *player=&(game.players[my_player_number]);
    if ((thing->field_18==0) && (player->field_2B != thing->field_6))
      return false;
    if ( thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = thing;
    }
    if ((_DK_help_tip_time > 20) || (player->field_453 == 12))
    {
      _DK_tool_tip_box.field_0 = 1;
      int stridx=_DK_trap_data[thing->field_1A].field_C;
      sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY()+86;
      _DK_tool_tip_box.field_809 = 4;
    } else
    {
      _DK_help_tip_time++;
    }
    return true;
}

short setup_object_tooltips(struct Coord3d *pos)
{
  static char text[2048];
  struct Thing *thing = NULL;
  struct PlayerInfo *player=&(game.players[my_player_number]);
  thing = game.things_lookup[player->field_35];
  if ((thing!=NULL)&&(!thing_is_special(thing)))
      thing = NULL;
  if (thing==NULL)
    thing = _DK_get_special_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = thing;
    }
    int stridx=_DK_specials_text[_DK_object_to_special[thing->field_1A]];
    sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
    _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
    _DK_tool_tip_box.field_0 = 1;
    _DK_tool_tip_box.field_809 = 5;
    _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    return true;
  }
  thing = _DK_get_spellbook_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = (void *)thing;
    }
    int stridx = _DK_spell_data[_DK_object_to_magic[thing->field_1A]].field_D;
    if (stridx>0)
    {
      sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
      _DK_tool_tip_box.field_0 = 1;
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
      _DK_tool_tip_box.field_809 = 5;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    }
    return 1;
  }
  thing = _DK_get_crate_at_position(pos->x.stl.num, pos->y.stl.num);
  if ( thing )
  {
    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = (void *)thing;
    }
    _DK_tool_tip_box.field_0 = 1;
    int objidx = thing->field_1A;
    int stridx;
    if ( _DK_workshop_object_class[objidx] == 8 )
      stridx = _DK_trap_data[_DK_object_to_door_or_trap[objidx]].field_C;
    else
      stridx = _DK_door_names[_DK_object_to_door_or_trap[objidx]];
    sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
    _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
    _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    _DK_tool_tip_box.field_809 = 5;
    return true;
  }
  if (!_DK_settings.tooltips_on)
    return false;
  thing = _DK_get_nearest_object_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    int objidx = thing->field_1A;
    int crtridx;
    if ( objidx == 49 )
    {
      sprintf(text,"%s",_DK_strings[545]);
    } else
    if (crtridx = _DK_objects[objidx].field_13)
    {
      int stridx=_DK_creature_data[crtridx].field_3;
      sprintf(text, "%s %s", _DK_strings[stridx], _DK_strings[609]);
    } else
    {
      return 0;
    }

    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = (void *)thing;
    }
    if ( (_DK_help_tip_time>20) || (player->field_453==12))
    {
      _DK_tool_tip_box.field_0 = 1;
      strcpy(_DK_tool_tip_box.text, text);
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
      _DK_tool_tip_box.field_809 = 5;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    } else
    {
      _DK_help_tip_time++;
    }
    return true;
  }
  return false;
}

short setup_land_tooltips(struct Coord3d *pos)
{
  if (!_DK_settings.tooltips_on)
    return false;
  int slab_idx = _DK_map_to_slab[pos->x.stl.num] + 85*_DK_map_to_slab[pos->y.stl.num];
  int attridx = game.slabmap[slab_idx].field_0;
  int stridx = _DK_slab_attrs[attridx].field_0;
  if (stridx==201)
    return false;
  if ((void *)attridx != _DK_tool_tip_box.target)
  {
    _DK_help_tip_time = 0;
    _DK_tool_tip_box.target = (void *)attridx;
  }
  struct PlayerInfo *player=&(game.players[my_player_number]);
  if ((_DK_help_tip_time>20) || (player->field_453==12))
  {
    _DK_tool_tip_box.field_0 = 1;
    sprintf(_DK_tool_tip_box.text, "%s", _DK_strings[stridx]);
    _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
    _DK_tool_tip_box.field_809 = 2;
    _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
  } else
  {
    _DK_help_tip_time++;
  }
  return true;
}

short setup_room_tooltips(struct Coord3d *pos)
{
  if (!_DK_settings.tooltips_on)
    return false;
  int slab_idx = _DK_map_to_slab[pos->x.stl.num] + 85*_DK_map_to_slab[pos->y.stl.num];
  struct Room *room;
  room = &game.rooms[game.slabmap[slab_idx].field_3];
  if (room==NULL)
    return false;
  int stridx=_DK_room_data[room->field_A].field_13;
  if ( stridx == 201 )
    return false;
  if ( room != _DK_tool_tip_box.target )
  {
    _DK_help_tip_time = 0;
    _DK_tool_tip_box.target = room;
  }
  struct PlayerInfo *player=&(game.players[my_player_number]);
  int widener=0;
  if ( (_DK_help_tip_time>20) || (player->field_453==12) )
  {
      if ( room->field_A >= 2 )
      {
        if ( (room->field_A<=14) || (room->field_A==16) )
          widener = 0;
      }
      sprintf(_DK_tool_tip_box.text, "%s", _DK_strings[stridx]);
      _DK_tool_tip_box.field_0 = 1;
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86 + 20*widener;
      _DK_tool_tip_box.field_809 = 1;
  } else
  {
    _DK_help_tip_time++;
  }
  return true;
}

void toggle_gui_overlay_map(void)
{
  unsigned char mask;
  if ((game.numfield_C & 0x20) == 0)
    mask=0x20;
  else
    mask=0;
  game.numfield_C = (game.numfield_C & 0xDFu) | mask;
}

short engine_point_to_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  struct PlayerInfo *player=&(game.players[my_player_number]);
  *map_x = 0;
  *map_y = 0;
  if ( (_DK_pointer_x >= 0) && (_DK_pointer_y >= 0)
    && (_DK_pointer_x < (player->field_444/pixel_size))
    && (_DK_pointer_y < (player->field_446/pixel_size)) )
  {
    int i;
    i = player->field_453;
    if ( ((i == 1) && (player->field_454)) || (i == 2) ||
      (i == 18) || (i == 16) || (i == 8) || (i == 23) )
    {
          *map_x = ( top_pointed_at_x << 8) + top_pointed_at_frac_x;
          *map_y = ( top_pointed_at_y << 8) + top_pointed_at_frac_y;
    } else
    {
          *map_x = (block_pointed_at_x << 8) + pointed_at_frac_x;
          *map_y = (block_pointed_at_y << 8) + pointed_at_frac_y;
    }
    // Clipping coordinates
    if ( *map_y < 0 )
      *map_y = 0;
    else
    if ( *map_y > 0xFEFFu )
      *map_y = 0xFEFFu;
    if ( *map_x < 0 )
      *map_x = 0;
    else
    if ( *map_x > 0xFEFFu )
      *map_x = 0xFEFFu;
    return true;
  }
  return false;
}

short point_to_overhead_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  struct PlayerInfo *player=&(game.players[my_player_number]);
  *map_x = 0;
  *map_y = 0;
  if ((screen_x >= 150) && (screen_x < 490)
    && (screen_y >= 56) && (screen_y < 396))
  {
    *map_x = 3*256 * (screen_x-150) / 4 + 384;
    *map_y = 3*256 * (screen_y-56) / 4 + 384;
    return ((*map_x >= 0) && (*map_x < 65536) && (*map_y >= 0) && (*map_y < 65536));
  }
  return false;
}

short screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
//return _DK_screen_to_map(camera,screen_x,screen_y,mappos);
  short result;
  long x,y;
  switch ( camera->field_6 )
  {
    case 1:
    case 2:
    case 5:
      // 3D view mode
      result=engine_point_to_map(camera,screen_x,screen_y,&x,&y);
      break;
    case 3: //map mode
      result=point_to_overhead_map(camera,screen_x,screen_y,&x,&y);
      break;
    default:
      result = false;
      break;
  }
  if ( result )
  {
    mappos->x.val = x;
    mappos->y.val = y;
  }
  if ( mappos->x.val > 0xFEFFu )
    mappos->x.val = 0xFEFFu;
  if ( mappos->y.val > 0xFEFFu )
    mappos->y.val = 0xFEFFu;
  return result;
}

short is_key_pressed(long key, long kmodif)
{
  if ( (kmodif==-1) || (kmodif==_DK_key_modifiers) )
    return lbKeyOn[key];
  return 0;
}

short check_cd_in_drive(void)
{
    static const char *func_name="check_cd_in_drive";
//  _DK_check_cd_in_drive(); return;
  static char src_fname[256];
  const int img_width = 320;
  const int img_height = 200;
  short was_locked = LbScreenIsLocked();
  sprintf(src_fname, "%s/%s/%s", datafiles_path, "ldata","dkwind00.dat");
  if ( LbFileExists(src_fname) )
    return true;
  if ( was_locked )
    LbScreenUnlock();
  if ( _DK_LbDataLoadAll(nocd_load_files) )
  {
      error(func_name, 78, "Unable to test for CD in drive");
      return false;
  }
  LbSyncLog("CD not found in drive, waiting\n");
  LbScreenClear(0);
  LbPaletteSet(nocd_pal);
  unsigned long counter;
  counter=0;
  while ( !exit_keeper )
  {
      if ( LbFileExists(src_fname) )
        break;
      copy_lowres_image_to_screen(nocd_raw,img_width,img_height);
      while ( (!_DK_LbIsActive()) && (!exit_keeper) && (!quit_game) )
      {
        if (!LbWindowsControl())
          exit_keeper = 1;
      }
      if (counter>300)
      {
          error(func_name, 79, "Wait time too long, giving up");
          exit_keeper = 1;
      }
      if ( lbKeyOn[KC_Q] )
      {
          error(func_name, 77, "User requested quit, giving up");
          lbKeyOn[KC_Q] = 0;
          exit_keeper = 1;
      }
      LbSleepFor(1000);
      counter++;
  }
  LbSyncLog("Finished waiting for CD after %lu seconds\n",counter);
  _DK_LbDataFreeAll(nocd_load_files);
  if ( was_locked )
    LbScreenLock();
  return (!exit_keeper);
}

short get_gui_inputs(short gameplay_on)
{
  //return _DK_get_gui_inputs(gameplay_on);
  static short doing_tooltip;
  static char over_slider_button=-1;
  doing_tooltip = 0;
  _DK_update_breed_activities();
  _DK_battle_creature_over = 0;
  _DK_gui_room_type_highlighted = -1;
  _DK_gui_door_type_highlighted = -1;
  _DK_gui_trap_type_highlighted = -1;
  _DK_gui_creature_type_highlighted = -1;
  if ( gameplay_on )
    _DK_maintain_my_battle_list();
  if ( !lbDisplay.MLeftButton )
  {
    const short gmax=86;//sizeof(_DK_active_buttons)/sizeof(struct GuiButton);
    _DK_drag_menu_x = -999;
    _DK_drag_menu_y = -999;
    int idx;
    for (idx=0;idx<gmax;idx++)
    {
      struct GuiButton *gbtn = &_DK_active_buttons[idx];
      if ((gbtn->field_0 & 1) && (gbtn->gbtype == 6))
          gbtn->field_1 = 0;
    }
  }

  int gidx;
  gidx = point_is_over_gui_menu(lbDisplay.MMouseX * pixel_size, _DK_GetMouseY());
  if ( gidx == -1 )
    _DK_busy_doing_gui = 0;
  else
    _DK_busy_doing_gui = 1;
  int fmmenu_idx = first_monopoly_menu();

  struct PlayerInfo *player=&(game.players[my_player_number]);
  int gmbtn_idx = -1;
  struct GuiButton *gbtn;
  // Sweep through buttons
  for (gidx=0;gidx<86;gidx++)
  {
    gbtn = &_DK_active_buttons[gidx];
    if ((gbtn->field_0 & 1)==0)
      continue;
    if (!_DK_active_menus[gbtn->gmenu_idx].flgfield_1D)
      continue;
    Gf_Btn_Callback callback;
    callback = gbtn->field_17;
    if ( callback != NULL )
      callback(gbtn);
    if ( ((gbtn->field_1C & 0x40)!=0) || _DK_mouse_is_over_small_map(player->mouse_x,player->mouse_y) )
      continue;

    if ( check_if_mouse_is_over_button(gbtn) && (_DK_input_button==NULL)
      || (gbtn->gbtype==6) && (gbtn->field_1!=0) )
    {
      if ( (fmmenu_idx==-1) || (gbtn->gmenu_idx==fmmenu_idx) )
      {
        gmbtn_idx = gidx;
        gbtn->field_0 |= 0x10;
        _DK_busy_doing_gui = 1;
        callback = gbtn->field_F;
        if ( callback != NULL )
          callback(gbtn);
        if ( gbtn->gbtype == 6 )
          break;
        if ( gbtn->gbtype != Lb_SLIDER )
          over_slider_button = -1;
      } else
      {
        gbtn->field_0 &= 0xEFu;
      }
    } else
    {
      gbtn->field_0 &= 0xEFu;
    }
    if ( gbtn->gbtype == Lb_SLIDER )
    {
      int mouse_x = lbDisplay.MMouseX*pixel_size;
      int btnsize;
      btnsize = gbtn->field_1D + ((gbtn->slide_val)*(gbtn->width-64) >> 8);
      if ((mouse_x>(btnsize+22)) && (mouse_x<=(btnsize+44)))
      {
        int mouse_y = _DK_GetMouseY();
        if ((mouse_y>gbtn->pos_y) && (mouse_y<=(gbtn->pos_y+gbtn->height)))
        {
          if ( _DK_left_button_clicked )
          {
            _DK_left_button_clicked = 0;
            over_slider_button = gidx;
            do_sound_menu_click();
          }
        }
      }
    }
  }  // end for

  short result = 0;
  if (_DK_input_button!=NULL)
  {
    _DK_busy_doing_gui = 1;
    if (_DK_get_button_area_input(_DK_input_button,_DK_input_button->id_num) != 0)
        result = 1;
  }
  if ((over_slider_button!=-1) && (_DK_left_button_released))
  {
      _DK_left_button_released = 0;
      if (gmbtn_idx!=-1)
        _DK_active_buttons[gmbtn_idx].field_1 = 0;
      over_slider_button = -1;
      do_sound_menu_click();
  }

  if (gmbtn_idx!=-1)
  {
    gbtn = &_DK_active_buttons[gmbtn_idx];
    if ((_DK_active_menus[gbtn->gmenu_idx].field_1 == 2) && ((gbtn->field_1C & 0x80)==0))
    {
      if (_DK_tool_tip_box.gbutton == gbtn)
      {
        if ((_DK_tool_tip_time>10) || (player->field_453==12))
        {
          _DK_busy_doing_gui = 1;
          if ( gbtn->field_13 != _DK_gui_area_text )
            _DK_setup_gui_tooltip(gbtn);
        } else
        {
          _DK_tool_tip_time++;
          doing_tooltip = 1;
          _DK_busy_doing_gui = 1;
        }
      } else
      {
        _DK_tool_tip_time = 0;
        _DK_tool_tip_box.gbutton = gbtn;
        _DK_tool_tip_box.pos_x = lbDisplay.MMouseX*pixel_size;
        _DK_tool_tip_box.pos_y = _DK_GetMouseY()+86;
        _DK_tool_tip_box.field_809 = 0;
      }
    } else
    {
      _DK_tool_tip_time = 0;
      _DK_tool_tip_box.gbutton = NULL;
    }
  } else
  {
    _DK_tool_tip_time = 0;
    _DK_tool_tip_box.gbutton = NULL;
  }

  if ( over_slider_button != -1 )
  {
    gbtn = &_DK_active_buttons[over_slider_button];
    int mouse_x = lbDisplay.MMouseX*pixel_size;
    gbtn->field_1 = 1;
    int slide_start,slide_end;
    slide_start = gbtn->pos_x+32;
    slide_end = gbtn->pos_x+gbtn->width-32;
    if (mouse_x > slide_start)
    {
      if (mouse_x < slide_end)
      {
        if ( gbtn->width>64 )
            gbtn->slide_val = ((mouse_x-slide_start) << 8) / (gbtn->width-64);
        else
            gbtn->slide_val = ((mouse_x-gbtn->pos_x) << 8) / (gbtn->width+1);
      } else
      {
        gbtn->slide_val = 255;
      }
    } else
    {
      gbtn->slide_val = 0;
    }
    *gbtn->field_33 = (gbtn->slide_val) * (gbtn->field_2D+1) >> 8;
    Gf_Btn_Callback callback;
    callback = gbtn->click_event;
    if ( callback != NULL )
      callback(gbtn);
    return 1;
  }

  if ( gmbtn_idx != -1 )
  {
    gbtn = &_DK_active_buttons[gmbtn_idx];
    Gf_Btn_Callback callback;
    if ( lbDisplay.MLeftButton )
    {
      result = 1;
      callback = gbtn->click_event;
      if ((callback!=NULL) || (((gbtn->field_0 & 2)!=0) || (gbtn->field_2F!=0) || (gbtn->gbtype==3)))
        if ((gbtn->field_0 & 8)!=0)
        {
          switch (gbtn->gbtype)
          {
          case 1:
            if ( (gbtn->field_1>5) && (callback!=NULL) )
              callback(gbtn);
            else
              gbtn->field_1++;
            break;
          case 6:
            if (callback!=NULL)
              callback(gbtn);
            break;
          }
        }
    } else
    if ( lbDisplay.MRightButton )
    {
      result = 1;
      callback = gbtn->rclick_event;
      if ((callback != NULL) && ((gbtn->field_0 & 8)!=0))
      {
        switch (gbtn->gbtype)
        {
        case 1:
          if ( (gbtn->field_2>5) && (callback!=NULL) )
            callback(gbtn);
          else
            gbtn->field_2++;
          break;
        case 6:
          if (callback!=NULL)
            callback(gbtn);
          break;
        }
      }
    }
    if ( _DK_left_button_clicked )
    {
      result = 1;
      if (game.field_1516F3 != 0)
      {
        if (gbtn->id_num == game.field_1516F3)
          game.field_1516F3 = 0;
      }
      callback = gbtn->click_event;
      if ((callback!=NULL) || (gbtn->field_0 & 2) || (gbtn->field_2F) || (gbtn->gbtype==3) )
      {
        _DK_left_button_clicked = 0;
        _DK_gui_last_left_button_pressed_id = gbtn->id_num;
        do_button_click_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ( _DK_right_button_clicked )
    {
      result = 1;
      if (game.field_1516F3 != 0)
      {
        if (gbtn->id_num == game.field_1516F3)
          game.field_1516F3 = 0;
      }
      callback = gbtn->rclick_event;
      if ((callback!=NULL))
      {
        _DK_right_button_clicked = 0;
        _DK_gui_last_right_button_pressed_id = gbtn->id_num;
        do_button_click_actions(gbtn, &gbtn->field_2, callback);
      }
    }
  }

  for (gidx=0;gidx<86;gidx++)
  {
    gbtn = &_DK_active_buttons[gidx];
    if (gbtn->field_0 & 1)
      if ( ((gmbtn_idx==-1) || (gmbtn_idx!=gidx)) && (gbtn->gbtype!=3) && (gbtn!=_DK_input_button) )
      {
        gbtn->field_0 &= 0xEFu;
        gbtn->field_1 = 0;
        gbtn->field_2 = 0;
      }
  }
  if ( gmbtn_idx != -1 )
  {
    Gf_Btn_Callback callback;
    gbtn = &_DK_active_buttons[gmbtn_idx];
    if ((gbtn->field_1) && (_DK_left_button_released))
    {
      callback = gbtn->click_event;
      result = 1;
      if ((callback!=NULL) || (gbtn->field_0 & 2) || (gbtn->field_2F!=0) || (gbtn->gbtype==3))
      {
        _DK_left_button_released = 0;
        do_button_release_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ((gbtn->field_2) && (_DK_right_button_released))
    {
      callback = gbtn->rclick_event;
      result = 1;
      if ( callback!=NULL )
      {
        _DK_right_button_released = 0;
        do_button_release_actions(gbtn, &gbtn->field_2, callback);
      }
    }
  }
  if ((gameplay_on) && (_DK_tool_tip_time==0) && (!_DK_busy_doing_gui))
  {
        int mouse_x = lbDisplay.MMouseX*pixel_size;
        int mouse_y = _DK_GetMouseY();
        struct Coord3d mappos;
        if ( screen_to_map(player->camera,mouse_x,mouse_y,&mappos) )
        {
          int bblock_x=mappos.x.stl.num;
          int bblock_y=mappos.y.stl.num;
          unsigned short bitval;
          // Get the top four bits
          bitval = (game.map[bblock_x+bblock_y*256].field_3) >> 12;
          if ((1 << player->field_2B) & (bitval))
          {
            if (player->field_37 != 1)
            {
              short shown;
              shown=setup_trap_tooltips(&mappos);
              if (!shown)
                shown=setup_object_tooltips(&mappos);
              if (!shown)
                shown=setup_land_tooltips(&mappos);
              if (!shown)
                shown=setup_room_tooltips(&mappos);
              if (!shown)
              {
                _DK_help_tip_time = 0;
                _DK_tool_tip_box.target = NULL;
              }
            }
          }
        }
  }
  if ( _DK_tool_tip_box.field_0 == 0 )
    reset_scrolling_tooltip();
  return result;
}

short get_creature_passenger_action_inputs(void)
{
  if ( ((game.numfield_C & 0x01)==0) || (game.numfield_C & 0x80) )
      get_gui_inputs(1);
  struct PlayerInfo *player=&(game.players[my_player_number]);
  if ( !player->field_2F )
    return false;
  if (_DK_right_button_released)
  {
    struct Packet *pckt = &game.packets[player->field_B];
    _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  struct Thing *thing;
  thing = game.things_lookup[player->field_2F];
  if ((player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0) )
  {
    struct Packet *pckt = &game.packets[player->field_B];
    _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  if ( is_key_pressed(KC_TAB,0) )
  {
    lbKeyOn[KC_TAB] = 0;
    toggle_gui_overlay_map();
  }
  return false;
}

long menu_id_to_number(short menu_id)
{
  int idx;
  const short gmax=8;//sizeof(_DK_active_menus)/sizeof(struct GuiMenu);
  for(idx=0;idx<gmax;idx++)
  {
    struct GuiMenu *gmnu;
    gmnu=&_DK_active_menus[idx];
    if ((gmnu->field_1 != 0) && (gmnu->field_0 == menu_id))
      return idx;
  }
  return -1;
}

void toggle_tooltips(void)
{
  _DK_settings.tooltips_on = !_DK_settings.tooltips_on;
  if (_DK_settings.tooltips_on)
    do_sound_menu_click();
  _DK_save_settings();
}

void set_menu_visible_on(long menu_id)
{
  const short gmax=86;//sizeof(_DK_active_buttons)/sizeof(struct GuiButton);
  long menu_num;
  menu_num = menu_id_to_number(menu_id);
  if ( menu_num == -1 )
    return;
  _DK_active_menus[menu_num].flgfield_1D = 1;
  int idx;
  for (idx=0;idx<gmax;idx++)
  {
    struct GuiButton *gbtn = &_DK_active_buttons[idx];
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
  if ( menu_num == -1 )
    return;
  _DK_active_menus[menu_num].flgfield_1D = 0;
}

void zoom_from_map(void)
{
  struct PlayerInfo *player=&(game.players[my_player_number]);
  if ( (game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320) )
  {
      _DK_toggle_status_menu((game.numfield_C & 0x40) != 0);
      struct Packet *pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt, 120,1,0,0,0);
  } else
  {
      struct Packet *pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt, 80,6,0,0,0);
  }
}

short get_map_action_inputs()
{
  struct PlayerInfo *player=&(game.players[my_player_number]);
  long keycode;
  long mouse_x = lbDisplay.MMouseX * pixel_size;
  long mouse_y = _DK_GetMouseY();
  int mappos_x = 3 * (mouse_x - 150) / 4 + 1;
  int mappos_y = 3 * (mouse_y - 56) / 4 + 1;
  if ((mappos_x >= 0) && (mappos_x < 255) && (mappos_y >= 0) && (mappos_y < 255) )
  {
    if ( _DK_left_button_clicked )
    {
      _DK_left_button_clicked = 0;
    }
    if ( _DK_left_button_released )
    {
      struct Packet *pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt, 81,mappos_x,mappos_y,0,0);
      _DK_left_button_released = 0;
      return true;
    }
  }

  if ( _DK_right_button_clicked )
    _DK_right_button_clicked = 0;
  if ( _DK_right_button_released )
  {
    _DK_right_button_released = 0;
    zoom_from_map();
    return true;
  } else
  {
    struct Packet *pckt = &game.packets[player->field_B];
    if ( pckt->field_5 )
      return true;
    if ( lbKeyOn[KC_F8] )
    {
      lbKeyOn[KC_F8] = 0;
      toggle_tooltips();
    }
    if ( lbKeyOn[KC_NUMPADENTER] )
    {
      lbKeyOn[KC_NUMPADENTER] = 0;
      // Toggle cheat menu
/*
      if ( (_DK_gui_box==NULL) || (_DK_gui_box_is_not_valid(_DK_gui_box)) )
      {
        gui_create_box(?,?,_DK_gui_main_option_list);
      } else
      {
        gui_delete_box(_DK_gui_box);
      }
*/
    }
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      _DK_turn_off_all_window_menus();
      zoom_from_map();
      return true;
    }
    return false;
  }
}

short toggle_first_person_menu(short visible)
{
  static char creature_query1_on = 0;
  static char creature_query2_on = 0;
  if (visible)
  {
    if ( creature_query1_on )
        set_menu_visible_on(31);
    else
    if ( creature_query2_on )
      set_menu_visible_on(35);
    return 1;
  } else
  {
    long gmnu_idx;
    // Menu no 31
    gmnu_idx=menu_id_to_number(31);
    if (gmnu_idx != -1)
      creature_query1_on = _DK_active_menus[gmnu_idx].flgfield_1D;
    else
      creature_query1_on = _DK_default_menu.flgfield_1D;
    set_menu_visible_off(31);
    // Menu no 35
    gmnu_idx=menu_id_to_number(35);
    if (gmnu_idx != -1)
      creature_query2_on = _DK_active_menus[gmnu_idx].flgfield_1D;
    else
      creature_query2_on = _DK_default_menu.flgfield_1D;
    set_menu_visible_off(31);
    return 1;
  }
}

void set_gui_visible(unsigned long visible)
{
  if (visible)
    game.numfield_C |= 0x20;
  else
    game.numfield_C &= 0xDF;
  struct PlayerInfo *player=&(game.players[my_player_number]);
  unsigned char is_visbl = ((game.numfield_C & 0x20)!=0);
  if (player->field_452 == 2)
    toggle_first_person_menu(is_visbl);
  else
    _DK_toggle_status_menu(is_visbl);
  if ( (game.numfield_D & 0x20) && (game.numfield_C & 0x20) )
    _DK_setup_engine_window(140, 0, MyScreenWidth, MyScreenHeight);
  else
    _DK_setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
}

void toggle_gui(void)
{
  int visible=((game.numfield_C & 0x20)==0);
  set_gui_visible(visible);
}

long creature_instance_is_available(struct Thing *thing, long inum)
{
  struct CreatureControl *crctrl;
  crctrl = game.creature_control_lookup[thing->field_64];
  return crctrl->instances[inum];
}

short get_creature_control_action_inputs(void)
{
  if ( ((game.numfield_C & 0x01)==0) || (game.numfield_C & 0x80) )
    get_gui_inputs(1);
  struct PlayerInfo *player=&(game.players[my_player_number]);
  long keycode;
  if ( lbKeyOn[KC_NUMPADENTER] )
  {
      lbKeyOn[KC_NUMPADENTER] = 0;
      // Toggle cheat menu
/*
      if ( (_DK_gui_box==NULL) || (_DK_gui_box_is_not_valid(_DK_gui_box)) )
      {
        gui_box=gui_create_box(?,?,_DK_gui_instance_option_list); / 200,20 or 20,200
        player->unknownbyte  |= 0x08;
        game.unknownbyte |= 0x08;
      } else
      {
        gui_delete_box(_DK_gui_box);
        player->unknownbyte &= 0xF7;
        game.unknownbyte &= 0xF7;
      }
*/
      return 1;
  }
  if ( lbKeyOn[KC_F12] )
  {
      lbKeyOn[KC_F12] = 0;
      // Cheat sub-menus
/*
      if ( (_DK_gui_cheat_box==NULL) || (_DK_gui_box_is_not_valid(_DK_gui_cheat_box)) )
      {
        ...
      } else
      {
        gui_delete_box(_DK_gui_cheat_box);
        player->unknownbyte &= 0xF7;
      }
*/
  }

  if ( player->field_2F )
  {
    short make_packet = _DK_right_button_released;
    if (!make_packet)
    {
      struct Thing *thing;
      thing = game.things_lookup[player->field_2F];
      if ( (player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0)
         || (thing->field_7 == 67) )
        make_packet = true;
    }
    if (make_packet)
    {
      struct Packet *pckt = &game.packets[player->field_B];
      _DK_right_button_released = 0;
      _DK_set_packet_action(pckt, 33, player->field_2F,0,0,0);
    }
  }
  if ( is_key_pressed(KC_TAB,0) )
  {
    lbKeyOn[KC_TAB] = 0;
    toggle_gui();
  }
  int numkey;
  numkey = -1;
  {
    for (keycode=KC_1;keycode<=KC_0;keycode++)
    {
      if ( is_key_pressed(keycode,0) )
      {
        lbKeyOn[keycode] = 0;
        numkey = keycode-2;
        break;
      }
    }
  }
  if ( numkey != -1 )
  {
    int idx;
    int num_avail = 0;
    for (idx=0;idx<10;idx++)
    {
      struct Thing *thing;
      thing = game.things_lookup[player->field_2F];
      int instnce = game.creature_stats[thing->field_1A].field_80[idx];
      if ( creature_instance_is_available(thing,instnce) )
      {
        if ( numkey == num_avail )
        {
          struct Packet *pckt = &game.packets[player->field_B];
          _DK_set_packet_action(pckt, 39, instnce,0,0,0);
          break;
        }
        num_avail++;
      }
    }
  }
  return false;
}

void turn_off_menu(short mnu_idx)
{
  _DK_turn_off_menu(mnu_idx);
}

void get_level_lost_inputs(void)
{
//  _DK_get_level_lost_inputs();
  struct PlayerInfo *player=&(game.players[my_player_number]);
  long keycode;
  if ( player->field_0 & 4 )
  {
    if (is_key_pressed(KC_RETURN,0))
    {
      struct Packet *pckt=&game.packets[player->field_B];
      _DK_set_packet_action(pckt, 14, 0,0,0,0);
      _DK_lbInkey = 0;
      lbKeyOn[KC_RETURN] = 0;
    } else
    {
      lbFontPtr = _DK_winfont;
      if ( (_DK_lbInkey == KC_BACK) || (pixel_size*_DK_LbTextStringWidth(player->strfield_463) < 450) )
      {
        struct Packet *pckt=&game.packets[player->field_B];
        _DK_set_packet_action(pckt, 121, _DK_lbInkey,0,0,0);
        lbKeyOn[_DK_lbInkey] = 0;
        _DK_lbInkey = 0;
      }
    }
    return;
  }
  if ((game.numfield_A & 0x01) != 0)
  {
    if (is_key_pressed(KC_RETURN,0))
    {
      struct Packet *pckt=&game.packets[player->field_B];
      _DK_set_packet_action(pckt, 13, 0,0,0,0);
      lbKeyOn[KC_RETURN] = 0;
      return;
    }
  }
  if (is_key_pressed(KC_SPACE,0))
  {
    struct Packet *pckt=&game.packets[player->field_B];
    _DK_set_packet_action(pckt, 5, 0,0,0,0);
    lbKeyOn[KC_SPACE] = 0;
  }


  if ( player->field_452 == 4 )
  {
    int screen_x = lbDisplay.MMouseX * pixel_size - 150;
    int screen_y = _DK_GetMouseY() - 56;
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      if (((game.numfield_A & 0x01) == 0) && (lbDisplay.PhysicalScreenWidth <= 320))
      {
        struct Packet *pckt=&game.packets[player->field_B];
        _DK_set_packet_action(pckt, 80, 6,0,0,0);
      }
      else
      {
        _DK_toggle_status_menu((game.numfield_C & 0x40) != 0);
        struct Packet *pckt=&game.packets[player->field_B];
        _DK_set_packet_action(pckt, 120, 1,0,0,0);
      }
    } else
    if ( _DK_right_button_released )
    {
        _DK_right_button_released = 0;
        if ( (game.numfield_A & 0x01) || lbDisplay.PhysicalScreenWidth > 320 )
        {
          _DK_toggle_status_menu((game.numfield_C & 0x40) != 0);
          struct Packet *pckt=&game.packets[player->field_B];
          _DK_set_packet_action(pckt, 120, 1,0,0,0);
        }
        else
        {
          struct Packet *pckt=&game.packets[player->field_B];
          _DK_set_packet_action(pckt, 80, 6,0,0,0);
        }
    } else
    if ( _DK_left_button_released )
    {
        int actn_x = 3*screen_x/4 + 1;
        int actn_y = 3*screen_y/4 + 1;
        if  ((actn_x >= 0) && (actn_x < 255) && (actn_y >= 0) && (actn_y < 255))
        {
          struct Packet *pckt=&game.packets[player->field_B];
          _DK_set_packet_action(pckt, 81, actn_x,actn_y,0,0);
          _DK_left_button_released = 0;
        }
    }
  } else
  if ( player->field_452 == 1 )
  {
    if ( lbKeyOn[KC_TAB] )
    {
        if ((player->field_37 == 2) || (player->field_37 == 5))
        {
          lbKeyOn[KC_TAB] = 0;
          toggle_gui();
        }
    } else
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      if (player->field_37 != 7)
      {
        _DK_turn_off_all_window_menus();
        game.numfield_C = (game.numfield_C ^ (unsigned __int8)(2 * game.numfield_C)) & 0x40 ^ game.numfield_C;
        struct Packet *pckt=&game.packets[player->field_B];
        if ((game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320))
        {
              if (_DK_toggle_status_menu(0))
                game.numfield_C |= 0x40;
              else
                game.numfield_C &= 0xBF;
              _DK_set_packet_action(pckt, 119, 4,0,0,0);
        } else
        {
              _DK_set_packet_action(pckt, 80, 5,0,0,0);
        }
        _DK_turn_off_roaming_menus();
      }
    }
  }
  if ( lbKeyOn[KC_ESCAPE] )
  {
    lbKeyOn[KC_ESCAPE] = 0;
    if ( _DK_a_menu_window_is_active() )
      _DK_turn_off_all_window_menus();
    else
      _DK_turn_on_menu(8);
  }
  struct Packet *pckt=&game.packets[player->field_B];
  struct Thing *thing;
  short inp_done=false;
  switch (player->field_452)
  {
    case 1:
      inp_done=_DK_menu_is_active(38);
      if ( !inp_done )
      {
        if ((game.numfield_C & 0x20) != 0)
        {
          _DK_initialise_tab_tags_and_menu(3);
          _DK_turn_off_all_panel_menus();
          _DK_turn_on_menu(38);
        }
      }
      inp_done=get_gui_inputs(1);
      if ( !inp_done )
      {
        if (player->field_453 == 15)
        {
          _DK_set_player_instance(player, 10, 0);
        } else
        {
          inp_done=_DK_get_small_map_inputs(player->mouse_x, player->mouse_y, player->field_450 / (3-pixel_size));
          if ( !inp_done )
            _DK_get_bookmark_inputs();
          _DK_get_dungeon_control_nonaction_inputs();
        }
      }
      break;
    case 2:
      thing = game.things_lookup[player->field_2F];
      if (thing->field_1F == 5)
      {
        struct CreatureControl *crctrl;
        crctrl = game.creature_control_lookup[thing->field_64];
        if ((crctrl->field_2 & 0x02) == 0)
          _DK_set_packet_action(pckt, 33, player->field_2F,0,0,0);
      } else
      {
        _DK_set_packet_action(pckt, 33, player->field_2F,0,0,0);
      }
      break;
    case 3:
      _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
      break;
    case 4:
      if ( _DK_menu_is_active(38) )
      {
        if ((game.numfield_C & 0x20) != 0)
          turn_off_menu(38);
      }
      break;
    default:
      return;
  }
}

short get_inputs(void)
{
  //return _DK_get_inputs();
  long keycode;
  if ( game.flags_cd & 0x01 )
  {
    _DK_load_packets_for_turn(game.gameturn);
    game.gameturn++;
    if ( lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_SPACE] || lbKeyOn[KC_RETURN]
       || (lbKeyOn[KC_LALT] && lbKeyOn[KC_X]) || _DK_left_button_clicked )
    {
      lbKeyOn[KC_ESCAPE] = 0;
      lbKeyOn[KC_SPACE] = 0;
      lbKeyOn[KC_RETURN] = 0;
      lbKeyOn[KC_X] = 0;
      _DK_left_button_clicked = 0;
      quit_game = 1;
    }
    return false;
  }
  if ( game.field_149E81 )
  {
    _DK_load_packets_for_turn(game.gameturn);
    game.gameturn++;
    if ( lbKeyOn[KC_LALT] && lbKeyOn[KC_X] )
    {
      lbKeyOn[KC_X] = 0;
      if ( game.numfield_A & 0x01 )
        _DK_LbNetwork_Stop();
      quit_game = 1;
      exit_keeper = 1;
    }
    return false;
  }
  struct PlayerInfo *player=&(game.players[my_player_number]);
  if ( player->field_0 & 0x80 )
  {
    struct Packet *pckt = &game.packets[player->field_B];
    pckt->field_A = 127;
    pckt->field_C = 127;
    if ((_DK_input_button==NULL) && (game.numfield_C & 0x01))
    {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        lbKeyOn[keycode] = 0;
        _DK_set_packet_action(pckt, 22, 0,0,0,0);
      }
    }
    return false;
  }
  if (player->field_29 == 2)
  {
    if (player->field_2C != 1)
    {
      get_level_lost_inputs();
      return true;
    }
    struct Thing *thing;
    thing = game.things_lookup[player->field_2F];
    if ( (thing <= game.things_lookup[0]) || (thing->field_1F != 5) )
    {
      get_level_lost_inputs();
      return true;
    }
    struct CreatureControl *crctrl;
    crctrl = game.creature_control_lookup[thing->field_64];
    if ((crctrl->field_2 & 2) == 0)
    {
      get_level_lost_inputs();
      return true;
    }
  }
  short inp_handled = false;
  if ( !(game.numfield_C & 0x01) || (game.numfield_C & 0x80) )
    inp_handled = get_gui_inputs(1);
  if ( !inp_handled )
    inp_handled = get_global_inputs();
  if (_DK_input_button!=NULL)
    return false;
  struct Packet *pckt;
  switch ( player->field_452 )
  {
  case 1:
      if (!inp_handled)
        inp_handled=_DK_get_dungeon_control_action_inputs();
      _DK_get_dungeon_control_nonaction_inputs();
      _DK_get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 2:
      if (!inp_handled)
        inp_handled = get_creature_control_action_inputs();
      _DK_get_creature_control_nonaction_inputs();
      _DK_get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 3:
      if ( inp_handled )
      {
        _DK_get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      if ( get_creature_passenger_action_inputs() )
      {
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      {
        _DK_get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return false;
      }
  case 4:
      if (!inp_handled)
        inp_handled = get_map_action_inputs();
      _DK_get_map_nonaction_inputs();
      _DK_get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 5:
      if (player->field_37==6)
        return false;
      if ( !(game.numfield_A & 0x01) )
        game.numfield_C &= 0xFE;
      if ( _DK_toggle_status_menu(0) )
        player->field_1 |= 0x01;
      else
        player->field_1 &= 0xFE;
      pckt = &game.packets[player->field_B];
      _DK_set_packet_action(pckt, 80, 4,0,0,0);
      return false;
  case 6:
      if (player->field_37 != 7)
      {
        pckt = &game.packets[player->field_B];
        _DK_set_packet_action(pckt, 80, 1,0,0,0);
      }
      return false;
  default:
      return false;
  }
}

void input(void)
{
  static const char *func_name="input";
  //_DK_input();return;
  // Set key modifiers based on the pressed key codes
  unsigned short key_mods=0;
  if ( lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT] )
    key_mods |= KM_SHIFT;
  if ( lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL] )
    key_mods |= KM_CONTROL;
  if ( lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT] )
    key_mods |= KM_ALT;
  _DK_key_modifiers = key_mods;
  if ( _DK_input_button )
  {
    if ( _DK_lbInkey )
      lbKeyOn[_DK_lbInkey] = 0;
  }

  struct PlayerInfo *player=&(game.players[my_player_number]);
  int idx=player->field_B;
  struct Packet *pckt=&game.packets[idx];
  if (_DK_is_game_key_pressed(27, 0, 0) != 0)
    pckt->field_10 |= 0x20;
  else
    pckt->field_10 &= 0xDFu;
  if (_DK_is_game_key_pressed(28, 0, 0) != 0)
    pckt->field_10 |= 0x40;
  else
    pckt->field_10 &= 0xBFu;

  get_inputs();
}

void keeper_gameplay_loop(void)
{
    static const char *func_name="keeper_gameplay_loop";
    static char text[255];
    static unsigned long last_loop_time=0;
    static unsigned long prev_time1=0;
    static unsigned long cntr_time1=0;
    static unsigned long prev_time2=0;
    static unsigned long cntr_time2=0;
    struct PlayerInfo *player=&(game.players[my_player_number]);
    _DK_PaletteSetPlayerPalette(player, _DK_palette);
    if ( game.numfield_C & 0x02 )
      _DK_initialise_eye_lenses();
    LbSyncLog("Entering the gameplay loop for level %d\n",(int)game.level_number);

    //the main gameplay loop starts
    while ( (!quit_game)&&(!exit_keeper) )
    {
      if ( game.flags_font & 0x10 )
      {
        if ( game.seedchk_random_used == 4 )
          ;//rndseed_nullsub();
      }

      // Some timing (which I don't understand; but it affects graphics)
      short do_draw;
      do_draw = true;
      if ( !(game.numfield_C & 0x01) )
      {
        if ( (!game.numfield_149F34) && (!game.numfield_149F38) )
        {
          unsigned long curr_time;
          curr_time = clock();
          cntr_time2++;
          if ( curr_time-prev_time2 >= 1000 )
          {
              double time_fdelta = 1000.0*((double)(cntr_time2))/((double)(curr_time-prev_time2));
              prev_time2 = curr_time;
              game.time_delta = (unsigned long)(time_fdelta*256.0);
              cntr_time2 = 0;
          }
          if ( (game.timingvar1!=0) && (game.seedchk_random_used % game.timingvar1) )
          {
            do_draw = false;
          }
        } else
        if ( ((game.seedchk_random_used & 0x3F)==0) ||
           ((game.numfield_149F38) && ((game.seedchk_random_used & 7)==0)) )
        {
            unsigned long curr_time;
            curr_time = clock();
            if ( (curr_time-prev_time1) < 5000 )
            {
              cntr_time1 += 64;
            } else
            {
              double time_fdelta = 1000.0*((double)(cntr_time1+64))/(curr_time-prev_time1);
              prev_time1 = curr_time;
              game.time_delta = (unsigned long)(time_fdelta*256.0);
              cntr_time1 = 0;
            }
        } else
        {
            do_draw = false;
        }
      }
      if (!do_draw)
          do_draw = !_DK_LbIsActive();

      update_mouse();
      input_eastegg();
      input();
      _DK_update();

      if ( do_draw )
      {
        LbScreenClear(0);
        if ( LbScreenLock() == 1 )
        {
          _DK_setup_engine_window(player->field_448, player->field_44A,
                player->field_444, player->field_446);
          _DK_redraw_display();
          LbScreenUnlock();
        }
      }
      do {
        if ( !LbWindowsControl() )
        {
          if ( !(game.numfield_A & 0x01) )
          {
            exit_keeper = 1;
            break;
          }
          sprintf(text, "alex");
          LbSyncLog("%s - %s\n",func_name,text);
        }
        if ( (game.numfield_A & 0x01) || _DK_LbIsActive() )
          break;
      } while ( (!exit_keeper) && (!quit_game) );

      if ( game.numfield_A & 0x10 )
      {
        game.numfield_A &= 0xEFu;
        _DK_cumulative_screen_shot();
      }

      // Direct information/error messages
      if ( LbScreenLock() == 1 )
      {
        if ( game.numfield_A & 0x08 )
        {
          if (anim_record_frame(lbDisplay.WScreen, _DK_palette))
            _DK_LbTextDraw(600/pixel_size, 4/pixel_size, "REC");
        }
        // Display in-game message for debug purposes
        //_DK_LbTextDraw(200/pixel_size, 8/pixel_size, text);text[0]='\0';
        if ( game.numfield_A & 0x02 )
        {
          sprintf(text, "OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
          error(func_name, 413, text);
          if ( lbDisplay.WScreen != NULL )
            _DK_LbTextDraw(300/pixel_size, 200/pixel_size, "OUT OF SYNC");
        }
        if ( game.numfield_A & 0x04 )
        {
          sprintf(text, "SEED OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
          error(func_name, 427, text);
          if ( lbDisplay.WScreen != NULL)
            _DK_LbTextDraw(300/pixel_size, 220/pixel_size, "SEED OUT OF SYNC");
        }
        LbScreenUnlock();
      }

      // Music and sound control
      if ( !_DK_SoundDisabled )
        if ((!game.numfield_149F34) && (!game.numfield_149F38))
        {
            MonitorStreamedSoundTrack();
            _DK_process_sound_heap();
        }

      // Move the graphics window to center of screen buffer and swap screen
      if ( do_draw )
      {
        // For resolution 640x480, move the graphics data 40 lines lower
        if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
          if ( LbScreenLock() == 1 )
          {
            int i;
            int scrmove_x=0;
            int scrmove_y=40;
            int scanline_len=640;
            for (i=400;i>=0;i--)
              memcpy(lbDisplay.WScreen+scanline_len*(i+scrmove_y)+scrmove_x, lbDisplay.WScreen+scanline_len*i, scanline_len-scrmove_x);
            memset(lbDisplay.WScreen, 0, scanline_len*scrmove_y);
            LbScreenUnlock();
          }
        LbScreenSwap();
      }

      // Make delay if the machine is too fast
      if ( (!game.field_149E81) || (!game.numfield_149F34) )
      {
        if ( game.numfield_D & 0x10 )
        {
          unsigned long sleep_end = last_loop_time + 1000;
          LbSleepUntil(sleep_end);
          last_loop_time = LbTimerClock();
        } else
        if ( game.timingvar1 == 0 )
        {
          unsigned long sleep_end = last_loop_time + 1000/game.num_fps;
          LbSleepUntil(sleep_end);
          last_loop_time = LbTimerClock();
        }
      }

      if ( game.numfield_149F42 == game.seedchk_random_used )
        exit_keeper = 1;
    } // end while
    LbSyncLog("Gameplay loop finished after %u turns\n",game.seedchk_random_used);
}

void initialise_load_game_slots(void)
{
  _DK_load_game_save_catalogue(save_game_catalogue);
  number_of_saved_games = 0;
  int entry_idx;
  for (entry_idx=0;entry_idx<8;entry_idx++)
  {
    if ( save_game_catalogue[entry_idx].used )
      number_of_saved_games++;
  }
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

void define_key_input(void)
{
    if ( _DK_lbInkey == 1 )
    {
          _DK_defining_a_key = 0;
          _DK_lbInkey = 0;
    } else
    if ( _DK_lbInkey )
    {
            short ctrl_state = 0;
            if ( lbKeyOn[KC_LCONTROL] || (lbKeyOn[KC_RCONTROL]) )
              ctrl_state = 1;
            short shift_state = 0;
            if ( lbKeyOn[KC_LSHIFT] || (lbKeyOn[KC_RSHIFT]) )
              shift_state = 1;
            if ( _DK_set_game_key(_DK_defining_a_key_id, _DK_lbInkey, shift_state, ctrl_state) )
              _DK_defining_a_key = 0;
            _DK_lbInkey = 0;
    }
}

void frontend_load_high_score_table(void)
{
    static const char *hiscores_fname="data\\hiscores.dat";
    if ( LbFileLoadAt(hiscores_fname, _DK_high_score_table) != sizeof(_DK_high_score_table) )
    {
        int i;
        int npoints = 1000;
        int nlevel = 10;
        for (i=0;i<10;i++)
        {
            sprintf(_DK_high_score_table[i].name, "Bullfrog");
            _DK_high_score_table[i].score=npoints;
            _DK_high_score_table[i].level=nlevel;
            npoints -= 100;
            nlevel -= 1;
        }
        LbFileSaveAt(hiscores_fname, _DK_high_score_table, sizeof(_DK_high_score_table));
    }
}

void add_score_to_high_score_table(void)
{
    struct Dungeon *dungeon=&(game.dungeon[my_player_number]);
    // Determining position of the new entry
    int idx;
    long new_score=dungeon->player_score;
    for (idx=0;idx<10;idx++)
    {
        if (_DK_high_score_table[idx].score <= new_score )
          break;
    }
    // If the new score is poor, return
    if (idx>9) return;
    // Moving entries down
    int k;
    for (k=8;k>=idx;k--)
    {
        memcpy(&_DK_high_score_table[k+1],&_DK_high_score_table[k],sizeof(struct HighScore));
    }
    // Preparing the new entry
    _DK_high_score_entry_input_active = idx;
    _DK_high_score_entry[0] = '\0';
    _DK_high_score_entry_index = 0;
    _DK_high_score_table[idx].score = new_score;
    _DK_high_score_table[idx].level = game.level_number - 1;
}

void frontend_load_data_from_cd(void)
{
    LbDataLoadSetModifyFilenameFunction(_DK_mdlf_for_cd);
}

void frontstory_load(void)
{
    static const char *func_name="frontstory_load";
    check_cd_in_drive();
    frontend_load_data_from_cd();
    if ( _DK_LbDataLoadAll(_DK_frontstory_load_files) )
    {
        error(func_name, 2790, "Unable to Load FRONT STORY FILES");
    } else
    {
        LbDataLoadSetModifyFilenameFunction(_DK_mdlf_default);
        _DK_LbSpriteSetupAll(_DK_frontstory_setup_sprites);
        LbPaletteSet(_DK_frontend_palette);
        srand(LbTimerClock());
        _DK_frontstory_text_no = rand() % 26 + 803;
    }
}
void inline frontstory_unload(void)
{
    _DK_LbDataFreeAll(_DK_frontstory_load_files);
}

short continue_game_available()
{
      static char buf[255];
      static short continue_needs_checking_file = 1;
      sprintf(buf, "%s\\%s\\%s",_DK_keeper_runtime_directory,"save","continue.sav");
      check_cd_in_drive();
      if ( LbFileLength(buf) != sizeof(struct Game) )
          return false;
      if ( continue_needs_checking_file )
      {
          TbFileHandle fh=LbFileOpen(buf,Lb_FILE_MODE_READ_ONLY);
          if ( fh != -1 )
          {
            LbFileRead(fh, buf, 10);
            LbFileClose(fh);
            if ( buf[0] )
              game.level_number = (unsigned char)buf[0];
          }
          continue_needs_checking_file = 0;
      }
      if ( game.level_number > 20 )
          return false;
      else
          return true;
}

int frontend_set_state(long nstate)
{
    static const char *func_name="frontend_set_state";
    static char text[255];
    //_DK_frontend_set_state(nstate);return nstate;
  switch ( _DK_frontend_menu_state )
  {
    case 0:
      _DK_init_gui();
      sprintf(text, "%s\\%s\\front.pal", _DK_datafiles_path,"ldata");
      check_cd_in_drive();
      if ( LbFileLoadAt(text, &_DK_frontend_palette) != 768 )
        error(func_name, 1323, "Unable to load FRONTEND PALETTE");
      check_cd_in_drive();
      frontend_load_high_score_table();
      _DK_LbMouseSetPosition(lbDisplay.PhysicalScreenWidth>>1, lbDisplay.PhysicalScreenHeight>>1);
      update_mouse();
      break;
    case 1: // main menu state
      turn_off_menu(18);
      break;
    case 2:
      turn_off_menu(19);
      break;
    case 3:
      _DK_frontmap_unload();
      _DK_frontend_load_data();
      break;
    case 4:
      turn_off_menu(20);
      break;
    case 5: // Network play mode
      turn_off_menu(21);
      break;
    case 6:
      turn_off_menu(22);
      break;
    case 12:
    case 29:
      frontstory_unload();
      break;
    case 13:
      if ( !(game.flags_cd & 0x10) )
        StopRedbookTrack();
      break;
    case 15:
      turn_off_menu(23);
      _DK_frontnet_modem_reset();
      break;
    case 16:
      turn_off_menu(24);
      _DK_frontnet_serial_reset();
      break;
    case 17:
      StopStreamedSample();
      turn_off_menu(25);
      break;
    case 18:
      turn_off_menu(26);
      break;
    case 19:
      _DK_fronttorture_unload();
      _DK_frontend_load_data();
      break;
    case 24:
      _DK_frontnetmap_unload();
      _DK_frontend_load_data();
      break;
    case 26:
      turn_off_menu(36);
      _DK_save_settings();
      break;
    case 27:
      turn_off_menu(39);
      if ( !(game.flags_cd & 0x10) )
        StopRedbookTrack();
      break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 14: //demo state (intro/credits)
    case 21:
    case 25:
      break;
    default:
      error(func_name, 1444, "Unhandled FRONTEND previous state");
      break;
  }
  if ( _DK_frontend_menu_state )
    fade_out();
  _DK_fade_palette_in = 1;
  LbSyncLog("Frontend state change from %u into %u\n",_DK_frontend_menu_state,nstate);
  switch ( nstate )
  {
    case 0:
      _DK_LbMouseChangeSpriteAndHotspot(0, 0, 0);
      break;
    case 1:
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_continue_game_option_available = continue_game_available();
      _DK_turn_on_menu(18);
      // change when all references to frontend_set_state() are rewritten
      //time_last_played_demo = LbTimerClock();
      _DK_time_last_played_demo=timeGetTime();
      _DK_last_mouse_x = lbDisplay.MMouseX * pixel_size;
      _DK_last_mouse_y = _DK_GetMouseY();
      _DK_fe_high_score_table_from_main_menu = 1;
      game.numfield_A &= 0xFEu;
      break;
    case 2:
      _DK_turn_on_menu(19);
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      break;
    case 3:
      if ( !_DK_frontmap_load() )
        nstate = 7;
      break;
    case 4:
      _DK_turn_on_menu(20);
      _DK_frontnet_service_setup();
      break;
    case 5:
      _DK_turn_on_menu(21);
      _DK_frontnet_session_setup();
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      game.numfield_A &= 0xFEu;
      break;
    case 6:
      _DK_turn_on_menu(22);
      _DK_frontnet_start_setup();
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      game.numfield_A |= 0x01;
      break;
    case 7:
    case 9:
    case 10:
    case 11:
    case 14:
    case 21:
    case 25:
      _DK_fade_palette_in = 0;
      break;
    case 8:
      if ( game.flags_font & 0x10 )
        ;//rndseed_nullsub();
      _DK_fade_palette_in = 0;
      break;
    case 12:
    case 29:
      frontstory_load();
      break;
    case 13:
      _DK_credits_offset = lbDisplay.PhysicalScreenHeight;
      _DK_credits_end = 0;
      _DK_LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
      lbDisplay.DrawFlags = 256;
      break;
    case 15:
      _DK_turn_on_menu(23);
      _DK_frontnet_modem_setup();
      break;
    case 16:
      _DK_turn_on_menu(24);
      _DK_frontnet_serial_setup();
      break;
    case 17:
      _DK_turn_on_menu(25);
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_frontstats_set_timer(); // note: rewrite this in pack with frontstats_update
      break;
    case 18:
      _DK_turn_on_menu(26);
      if ( game.dungeon[my_player_number].allow_save_score )
      {
        game.dungeon[my_player_number].allow_save_score = false;
        add_score_to_high_score_table();
      }
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_lbInkey = 0;
      break;
    case 19:
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_fronttorture_load();
      break;
    case 24:
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_frontnetmap_load();
      break;
    case 26:
      _DK_defining_a_key = 0;
      _DK_define_key_scroll_offset = 0;
      _DK_turn_on_menu(36);
      break;
    case 27:
      _DK_turn_on_menu(39);
      break;
    default:
      error(func_name, 1609, "Unhandled FRONTEND new state");
      break;
  }
  _DK_frontend_menu_state = nstate;
  return _DK_frontend_menu_state;
}


short end_input(void)
{
    if ( lbKeyOn[KC_SPACE] )
    {
        lbKeyOn[KC_SPACE] = 0;
        frontend_set_state(1);
    } else
    if ( lbKeyOn[KC_RETURN] )
    {
        lbKeyOn[KC_RETURN] = 0;
        frontend_set_state(1);
    } else
    if ( lbKeyOn[KC_ESCAPE] )
    {
        lbKeyOn[KC_ESCAPE] = 0;
        frontend_set_state(1);
    } else
    if ( _DK_left_button_clicked )
    {
        _DK_left_button_clicked = 0;
        frontend_set_state(1);
    } else
        return false;
    return true;
}

void frontcredits_input(void)
{
    _DK_credits_scroll_speed = 1;
    int speed;
    if ( lbKeyOn[KC_DOWN] )
    {
        speed = _DK_frontend_font[1][32].SHeight;
        _DK_credits_scroll_speed = speed;
    } else
    if ((lbKeyOn[KC_UP]) && (_DK_credits_offset<=0))
    {
        speed = -_DK_frontend_font[1][32].SHeight;
        if ( speed <= _DK_credits_offset )
          speed = _DK_credits_offset;
        _DK_credits_scroll_speed = speed;
    }
}

void frontend_input(void)
{
    int mouse_x,mouse_y;
    switch ( _DK_frontend_menu_state )
    {
      case 1:
        mouse_x = lbDisplay.MMouseX*pixel_size;
        mouse_y = _DK_GetMouseY();
        if ((mouse_x != _DK_last_mouse_x) || (mouse_y != _DK_last_mouse_y))
        {
          _DK_last_mouse_x = mouse_x;
          _DK_last_mouse_y = mouse_y;
          //time_last_played_demo = LbTimerClock();
          _DK_time_last_played_demo = timeGetTime();
        }
        get_gui_inputs(0);
        break;
      case 3:
        _DK_frontmap_input();
        break;
      case 6:
        get_gui_inputs(0);
        _DK_frontnet_start_input();
        break;
      case 12:
      case 29:
        if ( lbKeyOn[KC_SPACE] )
        {
          lbKeyOn[KC_SPACE] = 0;
          frontend_set_state(1);
        } else
        if ( lbKeyOn[KC_RETURN] )
        {
            lbKeyOn[KC_RETURN] = 0;
            frontend_set_state(1);
        } else
        if ( lbKeyOn[KC_ESCAPE] )
        {
            lbKeyOn[KC_ESCAPE] = 0;
            frontend_set_state(1);
        } else
        if ( _DK_left_button_clicked )
        {
            _DK_left_button_clicked = 0;
            frontend_set_state(1);
        }
        break;
      case 13:
        if (!end_input())
        {
          if ( _DK_credits_end )
            frontend_set_state(1);
        }
        frontcredits_input();
        break;
      case 18:
        get_gui_inputs(0);
         _DK_frontend_high_score_table_input();
        break;
      case 19:
        _DK_fronttorture_input();
        break;
      case 24:
        _DK_frontnetmap_input();
        break;
      case 26:
        if ( !_DK_defining_a_key )
          get_gui_inputs(0);
        else
          define_key_input();
        break;
      default:
        get_gui_inputs(0);
        break;
    } // end switch
}

void intro(void)
{
    char text[255];
    sprintf(text, "%s/ldata/%s", _DK_datafiles_path, "intromix.smk");
    LbSyncLog("Playing video \"%s\"\n",text);
    play_smacker_file(text, 1);
}

void outro(void)
{
    char text[255];
    sprintf(text, "%s/ldata/%s", _DK_datafiles_path, "outromix.smk");
    LbSyncLog("Playing video \"%s\"\n",text);
    play_smacker_file(text, 17);
}

void frontend_copy_background(void)
{
    unsigned char *wscrn = lbDisplay.WScreen;
    unsigned char *sscrn = _DK_frontend_background;
    int qwidth = lbDisplay.PhysicalScreenWidth >> 2;
    int i;
    for ( i=0; i<lbDisplay.PhysicalScreenHeight; i++ )
    {
        memcpy(wscrn, sscrn, 4*qwidth);
        memcpy(wscrn+4*qwidth, sscrn+4*qwidth, lbDisplay.PhysicalScreenWidth & 0x03);
        sscrn += 640;
        wscrn += lbDisplay.GraphicsScreenWidth;
    }
}

int __cdecl frontstory_draw()
{
  frontend_copy_background();
  _DK_LbTextSetWindow(70, 70, 500, 340);
  lbFontPtr = _DK_frontstory_font;
  lbDisplay.DrawFlags = 256;
  _DK_LbTextDraw(0,0,_DK_strings[_DK_frontstory_text_no]);
}

void draw_defining_a_key_box(void)
{
    _DK_draw_text_box(_DK_strings[470]);
}

struct TbBirthday {
    unsigned char day;
    unsigned char month;
    const char *name;
    };

const struct TbBirthday team_birthdays[] = {
    {13,1,"Mark Healey"},
    {21,3,"Jonty Barnes"},
    {3,5,"Simon Carter"},
    {5,5,"Peter Molyneux"},
    {13,11,"Alex Peters"},
    {1,12,"Dene Carter"},
    {25,5,"Tomasz Lis"},
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
  _DK_LbTextSetWindow(70, 70, 500, 340);
  lbFontPtr = _DK_frontstory_font;
  lbDisplay.DrawFlags = 256;
  const char *name=get_team_birthday();
  if ( name != NULL )
  {
      unsigned short line_pos = 0;
      if ( lbFontPtr != NULL )
          line_pos = lbFontPtr[1].SHeight;
      _DK_LbTextDraw(0, 170-line_pos, _DK_strings[885]);
      _DK_LbTextDraw(0, 170, name);
  } else
  {
      frontend_set_state(11);
  }
}

short frontend_draw(void)
{
    short result=1;
    switch (_DK_frontend_menu_state)
    {
    case 11:
        intro();
        return 0;
    case 14:
        _DK_demo();
        return 0;
    case 21:
        outro();
        return 0;
    }

    if ( LbScreenLock() != 1 )
        return result;

    switch ( _DK_frontend_menu_state )
    {
    case 1:
    case 2:
    case 4:
    case 5:
    case 15:
    case 16:
    case 17:
    case 18:
    case 20:
    case 27:
        _DK_draw_gui();
        break;
    case 3:
        _DK_frontmap_draw();
        break;
    case 6:
        _DK_draw_gui();
        break;
    case 12:
        frontstory_draw();
        break;
    case 13:
        _DK_frontcredits_draw();
        break;
    case 19:
        _DK_fronttorture_draw();
        break;
    case 24:
        _DK_frontnetmap_draw();
        break;
    case 26:
        _DK_draw_gui();
        if ( _DK_defining_a_key )
            draw_defining_a_key_box();
        break;
    case 29:
        frontbirthday_draw();
        break;
    default:
        break;
    }
    // In-Menu information, for debugging messages
    //char text[255];
    //sprintf(text, "time %7d, mode %d",LbTimerClock(),_DK_frontend_menu_state);
    //lbDisplay.DrawFlags=0;_DK_LbTextSetWindow(0,0,640,200);lbFontPtr = _DK_frontend_font[0];
    //_DK_LbTextDraw(200/pixel_size, 8/pixel_size, text);text[0]='\0';
    LbScreenUnlock();
    return result;
}

void frontend_update(short *finish_menu)
{
    switch ( _DK_frontend_menu_state )
    {
      case 1:
        frontend_button_info[8].field_2 = (_DK_continue_game_option_available?1:3);
        //this uses original timing function for compatibility with frontend_set_state()
        //if ( abs(LbTimerClock()-time_last_played_demo) > 30000 )
        if ( abs(timeGetTime()-_DK_time_last_played_demo) > 30000 )
          frontend_set_state(14);
        break;
      case 2:
        load_game_update();
        break;
      case 3:
        *finish_menu = _DK_frontmap_update();
        break;
      case 4:
        frontnet_service_update();
        break;
      case 5:
        frontnet_session_update();
        break;
      case 6:
        frontnet_start_update();
        break;
      case 7:
      case 8:
      case 10:
      case 25:
        *finish_menu = 1;
        break;
      case 9:
        *finish_menu = 1;
        exit_keeper = 1;
        break;
      case 13:
        if ( !(game.flags_cd & 0x10) )
          PlayRedbookTrack(7);
        break;
      case 15:
        frontnet_modem_update();
        break;
      case 16:
        frontnet_serial_update();
        break;
      case 17:
        _DK_frontstats_update(); // rewrite with frontstats_set_timer
        break;
      case 19:
        _DK_fronttorture_update();
        break;
      case 24:
        *finish_menu = _DK_frontnetmap_update();
        break;
      case 27:
        if ( !(game.flags_cd & 0x10) )
          PlayRedbookTrack(3);
        break;
      default:
        break;
    }
}

void faststartup_saved_packet_game(void)
{
    int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
    if (_DK_setup_screen_mode(scrmode))
        return;
    if ( _DK_settings.field_B != 13 )
    {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
    }
    if ( !_DK_setup_screen_mode(1) )
    {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
    }
    _DK_settings.field_B = 1;
    _DK_save_settings();
    _DK_startup_saved_packet_game();
    game.players[my_player_number].field_6 &= 0xFDu;
}

void faststartup_network_game(void)
{
    int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
    if ( !_DK_setup_screen_mode(scrmode) )
    {
      if ( _DK_settings.field_B != 13 )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
      }
      if ( !_DK_setup_screen_mode(1) )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
      }
      _DK_settings.field_B = 1;
      _DK_save_settings();
    }
    my_player_number = 0;
    game.flagfield_14EA4A = 2;
    game.players[my_player_number].field_2C = 1;
    game.numfield_14A83D = game.numfield_16;
    _DK_startup_network_game();
    game.players[my_player_number].field_6 &= 0xFDu;
}

short is_bonus_level(long levidx)
{
    if ((levidx>=100)&&(levidx<106))
        return true;
    return false;
}

int setup_old_network_service()
{
    return setup_network_service(_DK_net_service_index_selected);
}

int get_startup_menu_state(void)
{
  static const char *func_name="get_startup_menu_state";
  if ( game.flags_cd & 0x40 )
  {
    if (game.is_full_moon)
    {
        LbSyncLog("%s: Full moon state selected\n",func_name);
        return 12;
    } else
    if ( get_team_birthday() != NULL )
    {
        LbSyncLog("%s: Birthday state selected\n",func_name);
        return 29;
    } else
    {
        LbSyncLog("%s: Standard startup state selected\n",func_name);
        return 1;
    }
  } else
  {
    LbSyncLog("%s: Player-based state selected\n",func_name);
    struct PlayerInfo *player=&(game.players[my_player_number]);
    if ( !(game.numfield_A & 0x01) )
    {
      if ( (player->field_6 & 0x02) || (!player->field_29) )
      {
        return 3;
      } else
      if ( game.flags_cd & 1 )
      {
        game.flags_cd &= 0xFEu;
        return 1;
      } else
      if ( player->field_29 == 1 )
      {
          if ( game.level_number <= 20 )
          {
            if ( player->field_3 & 0x10 )
            {
                player->field_3 &= 0xEF;
                return 19;
            } else
            if ( is_bonus_level(game.numfield_14A83D) )
            {
                return 3;
            } else
            {
                return 17;
            }
          } else
          if ( is_bonus_level(game.numfield_14A83D) )
          {
              return 3;
          } else
          {
              return 21;
          }
      } else
      if ( player->field_29 == 3 )
      {
          return 17;
      } else
      if ( (game.numfield_14A83D < 50) || (game.numfield_14A83D > 79) )
      {
          return 3;
      } else
      {
          return 1;
      }
    } else
    {
      if ( !(player->field_3 & 0x10) )
      {
        if ( !(player->field_6 & 2) )
        {
          return 17;
        } else
        if ( setup_old_network_service() )
        {
          return 5;
        } else
        {
          return 1;
        }
      } else
      {
        player->field_3 &= 0xEF;
        return 19;
      }
    }
  }
  error(func_name, 978, "Unresolved menu state");
  return 1;
}
void wait_at_frontend(void)
{
  static const char *func_name="wait_at_frontend";
  //_DK_wait_at_frontend(); return;
  LbSyncLog("Falling into frontend menu.\n");

  //Moon phase calculation
  calculate_moon_phase(false);
  if ( game.flags_cd & 0x01 )
    game.field_149E81 = 0;
  game.numfield_15 = -1;

  initialise_load_game_slots();

  if ( (game.field_149E81) && (!game.numfield_149F47) )
  {
    faststartup_saved_packet_game();
    return;
  }

  if ( game.numfield_C & 0x02 )
  {
    faststartup_network_game();
    return;
  }

  if ( !_DK_setup_screen_mode_minimal(13) )
  {
    _DK_FatalError = 1;
    exit_keeper = 1;
    return;
  }
  LbScreenClear(0);
  LbScreenSwap();
  if ( !_DK_frontend_load_data() )
  {
    error(func_name, 738, "Unable to load frontend data");
    exit_keeper = 1;
    return;
  }
  memset(_DK_scratch, 0, 0x300u);
  LbPaletteSet(_DK_scratch);
  frontend_set_state(get_startup_menu_state());

  short finish_menu = 0;
  game.flags_cd &= 0xBFu;

  // Begin the frontend loop
  long last_loop_time = LbTimerClock();
  do
  {
    if ( (!LbWindowsControl()) && ((game.numfield_A & 0x01)==0) )
    {
      exit_keeper = 1;
      LbSyncLog("%s: Windows Control exit condition invoked\n",func_name);
      break;
    }
//LbSyncLog("update_mouse\n");
    update_mouse();
    _DK_old_mouse_over_button = frontend_mouse_over_button;
    frontend_mouse_over_button = 0;

//LbSyncLog("frontend_input\n");
    frontend_input();
    if ( exit_keeper )
    {
      LbSyncLog("%s: Frontend Input exit condition invoked\n",func_name);
      break; // end while
    }

//LbSyncLog("frontend_update\n");
    frontend_update(&finish_menu);
    if ( exit_keeper )
    {
      LbSyncLog("%s: Frontend Update exit condition invoked\n",func_name);
      break; // end while
    }

    if ( !finish_menu && _DK_LbIsActive() )
    {
//LbSyncLog("frontend_draw\n");
      frontend_draw();
      LbScreenSwap();
    }

    if ( !_DK_SoundDisabled )
    {
      _DK_process_3d_sounds();
      _DK_process_sound_heap();
      MonitorStreamedSoundTrack();
    }

    if ( _DK_fade_palette_in )
    {
//LbSyncLog("fade_in\n");
      fade_in();
      _DK_fade_palette_in = 0;
    } else
    {
      LbSleepUntil(last_loop_time + 30);
    }
    last_loop_time = LbTimerClock();
  } while (!finish_menu);

  LbPaletteFade(0, 8, Lb_PALETTE_FADE_CLOSED);
  LbScreenClear(0);
  LbScreenSwap();
  short prev_state = _DK_frontend_menu_state;
  frontend_set_state(0);
  if ( exit_keeper )
  {
    game.players[my_player_number].field_6 &= 0xFDu;
    return;
  }
  int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
  if ( !_DK_setup_screen_mode(scrmode) )
  {
    if ((_DK_settings.field_B==13) && _DK_setup_screen_mode(1))
    {
      _DK_settings.field_B = 1;
      _DK_save_settings();
    } else
    {
      _DK_FatalError = 1;
      exit_keeper = 1;
      return;
    }
  }

  display_loading_screen();
  short flgmem;
  switch ( prev_state )
  {
      case 7:
        my_player_number = 0;
        game.flagfield_14EA4A = 2;
        game.numfield_A &= 0xFEu;
        game.players[my_player_number].field_2C = 1;
        game.numfield_14A83D = game.numfield_16;
        _DK_startup_network_game();
        break;
      case 8:
        game.numfield_14A83D = game.numfield_16;
        game.numfield_A |= 0x01;
        game.flagfield_14EA4A = 5;
        game.players[my_player_number].field_2C = 1;
        _DK_startup_network_game();
        break;
      case 10:
        flgmem = game.numfield_15;
        game.numfield_A &= 0xFEu;
        if ( game.numfield_15 == -2 )
        {
          error(func_name, 1012, "Why are we here");
          game.numfield_15 = flgmem;
        } else
        {
          LbScreenClear(0);
          LbScreenSwap();
          _DK_load_game(game.numfield_15);
          game.numfield_15 = flgmem;
        }
        break;
      case 25:
        game.flags_cd |= 1;
        _DK_startup_saved_packet_game();
        break;
  }
  game.players[my_player_number].field_6 &= 0xFDu;
}

void game_loop(void)
{
  //_DK_game_loop();
  unsigned long random_seed;
  unsigned long playtime;
  playtime = 0;
  random_seed = 0;
  LbSyncLog("Entering gameplay loop.\n");
  while ( !exit_keeper )
  {
    update_mouse();
    wait_at_frontend();
    if ( exit_keeper )
      break;
    struct PlayerInfo *player=&(game.players[my_player_number]);
    if ( game.flagfield_14EA4A == 2 )
    {
      if ( game.numfield_15 == -1 )
      {
        _DK_set_player_instance(player, 11, 0);
      } else
      {
        game.numfield_15 = -1;
        game.numfield_C &= 0xFE;
      }
    }
    unsigned long starttime;
    unsigned long endtime;
    starttime = LbTimerClock();
    game.dungeon[my_player_number].time1 = timeGetTime();//starttime;
    game.dungeon[my_player_number].time2 = timeGetTime();//starttime;
    LbScreenClear(0);
    LbScreenSwap();
    keeper_gameplay_loop();
    _DK_LbMouseChangeSpriteAndHotspot(0, 0, 0);
    LbScreenClear(0);
    LbScreenSwap();
    StopRedbookTrack();
    StopMusic();
    _DK_frontstats_initialise();
    _DK_delete_all_structures();
    _DK_clear_mapwho();
    endtime = LbTimerClock();
    quit_game = 0;
    if ( game.numfield_C & 0x02 )
        exit_keeper=true;
    playtime += endtime-starttime;
    LbSyncLog("Play time is %d seconds\n",playtime>>10);
    random_seed += game.seedchk_random_used;
    _DK_reset_eye_lenses();
    if ( game.packet_fopened )
    {
      _DK_LbFileClose(game.packet_fhandle);
      game.packet_fopened = 0;
      game.packet_fhandle = 0;
    }
  } // end while
  // Stop the movie recording if it's on
  if ( game.numfield_A & 0x08 )
  {
    game.numfield_A &= 0xF7u;
    anim_stop();
  }
}

short reset_game(void)
{
  _DK_IsRunningUnmark();
  _DK_LbMouseSuspend();
  LbIKeyboardClose();
  _DK_LbScreenReset();
  _DK_LbDataFreeAll(_DK_game_load_files);
  _DK_LbMemoryFree(_DK_strings_data);
  _DK_strings_data = NULL;
  FreeAudio();
  return _DK_LbMemoryReset();
}

short process_command_line(unsigned short argc, char *argv[])
{
  static const char *func_name="process_command_line";
  char fullpath[CMDLN_MAXLEN+1];
  strncpy(fullpath, argv[0], CMDLN_MAXLEN);

  sprintf( _DK_keeper_runtime_directory, fullpath);
  char *endpos=strrchr( _DK_keeper_runtime_directory, '\\');
  if (endpos!=NULL)
      *endpos='\0';

  _DK_SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain
  _DK_LbErrorLogSetup(0, 0, 1);

  game.numfield_149F42 = -1;
  game.numfield_149F46 = 0;
  game.packet_checksum = 1;
  game.numfield_1503A2 = -1;
  game.flags_font &= 0xFEu;
  game.numfield_149F47 = 0;
  game.numfield_16 = 0;
  game.num_fps = 20;
  game.flags_cd = (game.flags_cd & 0xFE) | 0x40;

  short bad_param = 0;
  long level_num = -1;

  unsigned short narg;
  narg = 1;
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par==NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      strncpy(parstr, par+1, CMDLN_MAXLEN);
      if ( narg+1 < argc )
        strncpy(pr2str,  argv[narg+1], CMDLN_MAXLEN);
      else
        pr2str[0]='\0';

      if ( stricmp(parstr, "nointro") == 0 )
      {
        game.no_intro = 1;
      } else
      if ( stricmp(parstr, "nocd") == 0 )
      {
          game.flags_cd |= 0x10;
      } else
      if ( stricmp(parstr, "1player") == 0 )
      {
          game.one_player = 1;
      } else
      if ( (stricmp(parstr, "s") == 0) || (stricmp(parstr, "nosound") == 0) )
      {
          _DK_SoundDisabled = 1;
      } else
      if ( stricmp(parstr, "fps") == 0 )
      {
          narg++;
          game.num_fps = atoi(pr2str);
      } else
      if ( stricmp(parstr, "usersfont") == 0 )
      {
          game.flags_font |= 0x40;
      } else
      if ( stricmp(parstr,"level") == 0 )
      {
        game.numfield_C |= 0x02;
        level_num = atoi(pr2str);
        narg++;
      } else
      if ( stricmp(parstr,"q") == 0 )
      {
         game.numfield_C |= 0x02;
      } else
      if ( stricmp(parstr,"columnconvert") == 0 )
      {
         game.numfield_C |= 0x08;
      } else
      if ( stricmp(parstr,"lightconvert") == 0 )
      {
         game.numfield_C |= 0x10;
      } else
      if ( stricmp(parstr, "alex") == 0 )
      {
          game.flags_font |= 0x20;
      } else
      {
        LbSyncLog("Warning - Unrecognized command line parameter '%s'.\n",parstr);
        bad_param=narg;
      }
      narg++;
  }

  if ( level_num == -1 )
    level_num = 1;
  game.numfield_16 = level_num;
  game.level_number = level_num;
  if ( (game.numfield_C & 0x02) == 0 )
    game.level_number = 1;
  my_player_number = 0;
  return (bad_param==0);
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
  static const char *func_name="LbBullfrogMain";
  short retval=0;
  LbErrorLogSetup("/", log_file_name, 5);
  strcpy(window_class_name, PROGRAM_NAME);
  LbSetIcon(110);
  srand(LbTimerClock());

  retval=process_command_line(argc,argv);
  if ( retval < 1 )
  {
      static const char *msg_text="Command line parameters analysis failed.\n";
      error_dialog_fatal(func_name, 1, msg_text);
      LbErrorLogClose();
      return 0;
  }

  retval=setup_game();
  if ( retval )
    game_loop();
  reset_game();
/*
          MessageBox(NULL, "Finished OK.",
             "Dungeon Keeper Fan eXpansion", MB_OK | MB_ICONINFORMATION);
*/
  if ( !retval )
  {
      static const char *msg_text="Setting up game failed.\n";
      error_dialog_fatal(func_name, 2, msg_text);
  } else
  {
      LbSyncLog("%s finished properly.\n",func_name);
  }
  LbErrorLogClose();
  return 0;
}

void get_cmdln_args(unsigned short &argc, char *argv[])
{
  char *ptr;
  const char *cmndln_orig;
  cmndln_orig = GetCommandLineA();
  strncpy(cmndline, cmndln_orig, CMDLN_MAXLEN);
  ptr = cmndline;
  bf_argc = 0;
  while ( *ptr != '\0' )
  {
      if ((*ptr == '\t') || (*ptr == ' '))
      {
          ptr++;
          continue;
      }
      if (*ptr == '\"')
      {
          ptr++;
          bf_argv[bf_argc] = ptr;
          bf_argc++;
          while (*ptr != '\0')
          {
            if (*ptr == '\"')
            {
              *ptr++ = '\0';
              break;
            }
            ptr++;
          }
      } else
      {
          bf_argv[bf_argc] = ptr;
          bf_argc++;
          while (*ptr != '\0')
          {
            if ((*ptr == '\t') || (*ptr == ' '))
            {
              *ptr++ = '\0';
              break;
            }
            ptr++;
          }
      }
  }
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
//  return _DK_WinMain(hThisInstance,hPrevInstance,lpszArgument,nFunsterStil);
  static const char *func_name="WinMain";
  _DK_hInstance = hThisInstance;
  _DK_lpDDC = NULL;

  get_cmdln_args(bf_argc, bf_argv);

/*  {
  struct PlayerInfo *player=&(game.players[0]);
      static char msg_text[255];
      sprintf(msg_text,"Position of the first Player is %06x, first Camera is %06x bytes.\n",((int)player) - ((int)&_DK_game),((int)&(player->camera)) - ((int)player));
      error_dialog(func_name, 1, msg_text);
      return 0;
  }*/

//TODO: delete when won't be needed anymore
  memcpy(_DK_menu_list,menu_list,sizeof(menu_list));


  if (sizeof(struct Game)!=SIZEOF_Game)
  {
      static char msg_text[255];
      sprintf(msg_text,"Bad compilation - struct Game has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Game)-SIZEOF_Game);
      error_dialog(func_name, 1, msg_text);
      return 0;
  }

  LbBullfrogMain(bf_argc, bf_argv);

//  LbFileSaveAt("!tmp_file", &_DK_game, sizeof(struct Game));

  if ( _DK_lpDDC != NULL )
  {
    TDDrawBaseVTable *vtable=_DK_lpDDC->vtable;
    //Note: __thiscall is for functions with no arguments same as __fastcall,
    // so we're using __fastcall and passing the "this" pointer manually
    vtable->reset_screen(_DK_lpDDC);
    if ( _DK_lpDDC != NULL )
    {
      vtable=_DK_lpDDC->vtable;
      // Destructor requires a flag passed in AX
      asm volatile ("  movl $1,%eax\n");
      vtable->dt(_DK_lpDDC);
    }
  }
    return 0;
}
