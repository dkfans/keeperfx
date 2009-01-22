/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontend.c
 *     Frontend menu implementation for Dungeon Keeper.
 * @par Purpose:
 *     Functions to display and maintain the game menu.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2008 - 22 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontend.h"
#include "bflib_basics.h"
#include "globals.h"
#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_dernc.h"
#include "bflib_datetm.h"
#include "bflib_keybrd.h"
#include "bflib_sndlib.h"
#include "bflib_mouse.h"
#include "bflib_vidraw.h"
#include "keeperfx.h"
#include "gui_draw.h"
#include "kjm_input.h"
#include "vidmode.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT char _DK_update_menu_fade_level(struct GuiMenu *gmnu);
DLLIMPORT char _DK_create_button(struct GuiMenu *gmnu, struct GuiButtonInit *gbinit);
DLLIMPORT void _DK_gui_area_null(struct GuiButton *gbtn);
DLLIMPORT char _DK_create_menu(struct GuiMenu *mnu);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

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

/******************************************************************************/

void LbDataLoadSetModifyFilenameFunction(ModDL_Fname_Func nmodify_dl_filename_func)
{
  _DK_modify_data_load_filename_function=nmodify_dl_filename_func;
}

void turn_off_menu(short mnu_idx)
{
  _DK_turn_off_menu(mnu_idx);
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
  _DK_LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, height);
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

void do_button_release_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
  static const char *func_name="do_button_release_actions";
  //LbSyncLog("%s: Starting\n", func_name);
  //_DK_do_button_release_actions(gbtn, s, callback); return;
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
  case 3:
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
    gmnu = &active_menus[gbtn->gmenu_idx];
    if (gbtn->field_2F != NULL)
      create_menu(gbtn->field_2F);
    if ((gbtn->field_0 & 0x02) && (gbtn->gbtype != 5))
    {
      if (callback == NULL)
        do_sound_menu_click();
      gmnu->field_1 = 3;
    }
  }
  //LbSyncLog("%s: Finished\n", func_name);
}

unsigned long is_toggleable_menu(short mnu_idx)
{
  switch (mnu_idx)
  {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
      return 1;
  case 16:
  case 27:
  case 31:
  case 32:
  case 34:
  case 35:
  case 38:
      return 1;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 28:
  case 29:
  case 30:
  case 33:
  case 36:
  case 37:
      return 0;
  default:
      return 1;
  }
}

void add_to_menu_stack(unsigned char mnu_idx)
{
  static const char *func_name="add_to_menu_stack";
  short i;
  if (no_of_active_menus >= ACTIVE_MENUS_COUNT)
  {
    error(func_name, 1830, "No more room for menu stack");
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
      menu_stack[no_of_active_menus-1] = mnu_idx;
      //LbSyncLog("Menu %d moved to end of stack, at position %d.\n",mnu_idx,no_of_active_menus-1);
      return;
    }
  }
  // If not in stack, add at end
  menu_stack[no_of_active_menus] = mnu_idx;
  no_of_active_menus++;
  //LbSyncLog("Menu %d put on stack, at position %d.\n",mnu_idx,no_of_active_menus-1);
}

void update_radio_button_data(struct GuiMenu *gmnu)
{
  int i;
  struct GuiButton *gbtn;
  unsigned char *rbstate;
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
  int i;
  struct GuiButton *gbtn;
  long sldpos;
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
  int i;
  struct GuiButton *gbtn;
  Gf_Btn_Callback callback;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    callback = gbtn->field_17;
    if ((callback != NULL) && (gbtn->gmenu_idx == gmnu->field_14))
      callback(gbtn);
  }
}

char create_button(struct GuiMenu *gmnu, struct GuiButtonInit *gbinit)
{
  struct GuiButton *gbtn;

  char i;
  i=_DK_create_button(gmnu, gbinit);

  gbtn = &active_buttons[i];
  //LbSyncLog("Created button %d at (%d,%d) size (%d,%d)\n",i,
  //    gbtn->pos_x,gbtn->pos_y,gbtn->width,gbtn->height);

  return i;
}

long compute_menu_position_x(long desired_pos,int menu_width)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  long pos;
  switch (desired_pos)
  {
  case POS_MOUSMID: // Place menu centered over mouse
      pos = GetMouseX() - (menu_width >> 1);
      break;
  case POS_GAMECTR: // Player-based positioning
      pos = (player->field_448) + (player->field_444 >> 1) - (menu_width >> 1);
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
      pos = (desired_pos*units_per_pixel)>>4;
      if (pos+menu_width > lbDisplay.PhysicalScreenWidth)
        pos = lbDisplay.PhysicalScreenWidth-menu_width;
/* No idea what's this for - disabling
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
    if (pos < player->field_448)
      pos = player->field_448;
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
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  long pos;
  switch (desired_pos)
  {
  case POS_MOUSMID: // Place menu centered over mouse
      pos = GetMouseY() - (menu_height >> 1);
      break;
  case POS_GAMECTR: // Player-based positioning
      pos = (player->field_446 >> 1) - ((menu_height+20) >> 1);
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
      pos = (desired_pos*units_per_pixel)>>4;
      if (pos+menu_height > lbDisplay.PhysicalScreenHeight)
        pos = lbDisplay.PhysicalScreenHeight-menu_height;
      break;
  }
  // Clipping position Y
  if (pos+menu_height > MyScreenHeight)
    pos = MyScreenHeight-menu_height;
  if (pos < 0)
    pos = 0;
  return pos;
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

char create_menu(struct GuiMenu *gmnu)
{
  static const char *func_name="create_menu";
  //return _DK_create_menu(gmnu);
  int mnu_num;
  struct GuiMenu *amnu;
  struct PlayerInfo *player;
  Gf_Mnu_Callback callback;
  struct GuiButtonInit *btninit;
  int i;

  mnu_num = menu_id_to_number(gmnu->field_0);
  if (mnu_num != -1)
  {
    amnu = &active_menus[mnu_num];
    amnu->field_1 = 1;
    amnu->numfield_2 = gmnu->numfield_2;
    amnu->flgfield_1D = ((game.numfield_C & 0x20) != 0) || (!is_toggleable_menu(gmnu->field_0));
    return mnu_num;
  }
  add_to_menu_stack(gmnu->field_0);
  mnu_num = first_available_menu();
  if (mnu_num == -1)
  {
      error(func_name, 3066, "Too many menus open");
      return -1;
  }
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  amnu = &active_menus[mnu_num];
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

  for (i=0; i<3; i++)
  {
    if ((menu_ids[i] == gmnu->field_0) && (MyScreenHeight == 480))
    {
      amnu->pos_y += 80;
      break;
    }
  }
  amnu->numfield_2 = gmnu->numfield_2;
  if (amnu->numfield_2 < 1)
    error(func_name, 3019, "Oi! There is a fade time less than 1. Idiot.");
  amnu->ptrfield_4 = gmnu->ptrfield_4;
  amnu->width = gmnu->width;
  amnu->height = gmnu->height;
  amnu->ptrfield_10 = gmnu->ptrfield_10;
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
      error(func_name, 3050, "Cannot Allocate button");
      return -1;
    }
  }
  update_radio_button_data(amnu);
  init_slider_bars(amnu);
  init_menu_buttons(amnu);
  LbSyncLog("Created menu %d at (%d,%d) size (%d,%d)\n",mnu_num,
      amnu->pos_x,amnu->pos_y,amnu->width,amnu->height);
  return mnu_num;
}

void turn_on_menu(short idx)
{
  //_DK_turn_on_menu(idx); return;
  if ( create_menu(menu_list[idx]) )
  {
    if (menu_list[idx]->field_1F)
      game.field_1517F6 = idx;
  }
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

int frontend_set_state(long nstate)
{
    static const char *func_name="frontend_set_state";
    static char text[255];
    //_DK_frontend_set_state(nstate);return nstate;
  switch ( _DK_frontend_menu_state )
  {
    case 0:
      _DK_init_gui();
      sprintf(text, "%s/%s/front.pal", install_info.inst_path,"ldata");
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
      LbMouseChangeSpriteAndHotspot(0, 0, 0);
      break;
    case 1:
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_continue_game_option_available = continue_game_available();
      turn_on_menu(18);
      // change when all references to frontend_set_state() are rewritten
      //time_last_played_demo = LbTimerClock();
      _DK_time_last_played_demo=timeGetTime();
      _DK_last_mouse_x = GetMouseX();
      _DK_last_mouse_y = GetMouseY();
      _DK_fe_high_score_table_from_main_menu = 1;
      game.numfield_A &= 0xFEu;
      break;
    case 2:
      turn_on_menu(19);
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      break;
    case 3:
      if ( !_DK_frontmap_load() )
        nstate = 7;
      break;
    case 4:
      turn_on_menu(20);
      _DK_frontnet_service_setup();
      break;
    case 5:
      turn_on_menu(21);
      _DK_frontnet_session_setup();
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      game.numfield_A &= 0xFEu;
      break;
    case 6:
      turn_on_menu(22);
      _DK_frontnet_start_setup();
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
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
      turn_on_menu(23);
      _DK_frontnet_modem_setup();
      break;
    case 16:
      turn_on_menu(24);
      _DK_frontnet_serial_setup();
      break;
    case 17:
      turn_on_menu(25);
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_frontstats_set_timer(); // note: rewrite this in pack with frontstats_update
      break;
    case 18:
      turn_on_menu(26);
      if ( game.dungeon[my_player_number].allow_save_score )
      {
        game.dungeon[my_player_number].allow_save_score = false;
        add_score_to_high_score_table();
      }
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_lbInkey = 0;
      break;
    case 19:
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_fronttorture_load();
      break;
    case 24:
      LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_frontnetmap_load();
      break;
    case 26:
      _DK_defining_a_key = 0;
      _DK_define_key_scroll_offset = 0;
      turn_on_menu(36);
      break;
    case 27:
      turn_on_menu(39);
      break;
    default:
      error(func_name, 1609, "Unhandled FRONTEND new state");
      break;
  }
  _DK_frontend_menu_state = nstate;
  return _DK_frontend_menu_state;
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

void frontend_input(void)
{
    int mouse_x,mouse_y;
    switch ( _DK_frontend_menu_state )
    {
      case 1:
        mouse_x = GetMouseX();
        mouse_y = GetMouseY();
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

void kill_button(struct GuiButton *gbtn)
{
  gbtn->field_0 &= 0xFEu;
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

char update_menu_fade_level(struct GuiMenu *gmnu)
{
  return _DK_update_menu_fade_level(gmnu);
}

void draw_menu_buttons(struct GuiMenu *gmnu)
{
  //_DK_draw_menu_buttons(gmnu); return;
  int i;
  struct GuiButton *gbtn;
  Gf_Btn_Callback callback;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    callback = gbtn->field_13;
    if ((callback != NULL) && (gbtn->field_0 & 0x04) && (gbtn->field_0 & 0x01) && (gbtn->gmenu_idx == gmnu->field_14))
    {
      if ((gbtn->field_1 == 0) && (gbtn->field_2 == 0) || (gbtn->gbtype == 4) || (callback == gui_area_null))
        callback(gbtn);
    }
  }
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    callback = gbtn->field_13;
    if ((callback != NULL) && (gbtn->field_0 & 0x04) && (gbtn->field_0 & 0x01) && (gbtn->gmenu_idx == gmnu->field_14))
    {
      if (((gbtn->field_1) || (gbtn->field_2)) && (gbtn->gbtype != 4) && (callback != gui_area_null))
        callback(gbtn);
    }
  }
}

long menu_id_to_number(short menu_id)
{
  int idx;
  struct GuiMenu *gmnu;
  const short gmax=8;//sizeof(active_menus)/sizeof(struct GuiMenu);
  for(idx=0;idx<gmax;idx++)
  {
    gmnu=&active_menus[idx];
    if ((gmnu->field_1 != 0) && (gmnu->field_0 == menu_id))
      return idx;
  }
  return -1;
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

void update_fade_active_menus(void)
{
  struct GuiMenu *gmnu;
  int k;
  const short gmax=8;//sizeof(active_menus)/sizeof(struct GuiMenu);
  for (k=0; k < gmax; k++)
  {
    gmnu = &active_menus[k];
    if (update_menu_fade_level(gmnu) == -1)
    {
      kill_menu(gmnu);
      remove_from_menu_stack(gmnu->field_0);
    }
  }
}

void draw_active_menus_buttons(void)
{
  struct GuiMenu *gmnu;
  int k,i;
  Gf_Mnu_Callback callback;
  for (k=0; k < no_of_active_menus; k++)
  {
    i = menu_id_to_number(menu_stack[k]);
    if (i<0) continue;
    gmnu = &active_menus[i];
//LbSyncLog("DRAW menu %d, fields %d, %d\n",i,gmnu->field_1,gmnu->flgfield_1D);
    if ((gmnu->field_1) && (gmnu->flgfield_1D))
    {
        if ((gmnu->field_1 != 2) && (gmnu->numfield_2))
        {
          if (gmnu->ptrfield_15 != NULL)
            if (gmnu->ptrfield_15->numfield_2)
              lbDisplay.DrawFlags |= 0x04;
        }
        callback = gmnu->ptrfield_10;
        if (callback != NULL)
          callback(gmnu);
        if (gmnu->field_1 == 2)
          draw_menu_buttons(gmnu);
        lbDisplay.DrawFlags &= 0xFFFBu;
    }
  }
}

void draw_menu_highlight(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  struct GuiMenu *secmnu;
  int i,j;
  int x,y;
  short in_range;
  if (gmnu->flgfield_1D == 0)
    return;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    if ((!gbtn->field_13) || ((gbtn->field_0 & 0x04) == 0) || ((gbtn->field_0 & 0x01) == 0) || (!game.field_1516F3))
      continue;
    in_range = 0;
    switch (gbtn->id_num)
    {
    case 1:
      if ((game.field_1516F3 >= 68) && (game.field_1516F3 <= 71))
        in_range = 1;
      break;
    case 2:
      if ((game.field_1516F3 >= 6) && (game.field_1516F3 <= 20))
        in_range = 1;
      break;
    case 3:
      if ((game.field_1516F3 >= 21) && (game.field_1516F3 <= 36))
        in_range = 1;
      break;
    case 4:
      if ((game.field_1516F3 >= 53) && (game.field_1516F3 <= 61))
        in_range = 1;
      break;
    default:
      break;
    }
    if (in_range)
    {
      for (j=0; j<ACTIVE_MENUS_COUNT; j++)
      {
        secmnu = &active_menus[j];
        if (secmnu->field_1)
        {
          if ((gbtn->field_1B & 0xFF) == secmnu->field_0)
            break;
        }
      }
      if (j != -1)
        continue;
    } else
    {
      if ((gbtn->id_num == 0) || (gbtn->id_num != game.field_1516F3))
        continue;
    }
    x = ((gbtn->width >> 1) - pixel_size * button_sprite[176].SWidth / 2 + gbtn->pos_x);
    y = ((gbtn->height >> 1) - pixel_size * button_sprite[176].SHeight / 2 + gbtn->pos_y);
    j = 176+((game.seedchk_random_used >> 1) & 7);
    LbSpriteDraw(x/pixel_size, y/pixel_size, &button_sprite[j]);
  }
}

void draw_active_menus_highlights(void)
{
  struct GuiMenu *gmnu;
  int k;
  for (k=0; k<ACTIVE_MENUS_COUNT; k++)
  {
    gmnu = &active_menus[k];
    if ((gmnu->field_1) && (gmnu->field_0 == 1))
      draw_menu_highlight(gmnu);
  }
}


void draw_gui(void)
{
  //_DK_draw_gui(); return;
  unsigned int flg_mem;
  lbFontPtr = winfont;
  flg_mem = lbDisplay.DrawFlags;
  _DK_LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  update_fade_active_menus();
  draw_active_menus_buttons();
  if (game.field_1516F3 != 0)
  {
    draw_active_menus_highlights();
    if (game.field_1516F7 != -1)
    {
      game.field_1516F7--;
      if (game.field_1516F7 == 0)
        game.field_1516F3 = 0;
    }
  }
  lbDisplay.DrawFlags = flg_mem;
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
        draw_gui();
        break;
    case 3:
        _DK_frontmap_draw();
        break;
    case 6:
        draw_gui();
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
        draw_gui();
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

/******************************************************************************/

