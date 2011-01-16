/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_tabs.c
 *     Main in-game GUI, visible during gameplay.
 * @par Purpose:
 *     Functions to show and maintain tabbed menu appearing ingame.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 03 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontmenu_ingame_tabs.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_keybrd.h"
#include "bflib_guibtns.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "thing_doors.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_magic.hpp"
#include "room_workshop.h"
#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "packets.h"
#include "frontmenu_ingame_evnt.h"
#include "frontend.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_gui_zoom_in(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_zoom_out(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_map(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_turn_on_autopilot(struct GuiButton *gbtn);
DLLIMPORT void _DK_menu_tab_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_autopilot_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_turn_on_autopilot(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_event_button(struct GuiButton *gbtn);
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
DLLIMPORT void _DK_gui_creature_query_background1(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_creature_query_background2(struct GuiMenu *gmnu);
DLLIMPORT void _DK_maintain_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_creature_doing_activity(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_creature_activity(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_over_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_anger_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_smiley_anger_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_experience_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_instance_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_instance(struct GuiButton *gbtn);
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
DLLIMPORT void _DK_maintain_activity_row(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_activity_background(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_area_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_stat_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_event_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_toggle_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_prison_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_room_and_creature_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_payday_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_research_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_workshop_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_player_creature_info(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_player_room_info(struct GuiButton *gbtn);
DLLIMPORT void _DK_spell_lost_first_person(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_tend_to(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_query(struct GuiButton *gbtn);
/******************************************************************************/
void gui_zoom_in(struct GuiButton *gbtn);
void gui_zoom_out(struct GuiButton *gbtn);
void gui_go_to_map(struct GuiButton *gbtn);
void gui_turn_on_autopilot(struct GuiButton *gbtn);
void menu_tab_maintain(struct GuiButton *gbtn);
void gui_area_autopilot_button(struct GuiButton *gbtn);
void maintain_turn_on_autopilot(struct GuiButton *gbtn);
void gui_choose_room(struct GuiButton *gbtn);
void gui_area_event_button(struct GuiButton *gbtn);
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
void gui_creature_query_background1(struct GuiMenu *gmnu);
void gui_creature_query_background2(struct GuiMenu *gmnu);
void maintain_room(struct GuiButton *gbtn);
void maintain_big_room(struct GuiButton *gbtn);
void maintain_spell(struct GuiButton *gbtn);
void maintain_big_spell(struct GuiButton *gbtn);
void maintain_trap(struct GuiButton *gbtn);
void maintain_door(struct GuiButton *gbtn);
void maintain_big_trap(struct GuiButton *gbtn);
void pick_up_creature_doing_activity(struct GuiButton *gbtn);
void gui_go_to_next_creature_activity(struct GuiButton *gbtn);
void gui_go_to_next_room(struct GuiButton *gbtn);
void gui_over_room_button(struct GuiButton *gbtn);
void gui_area_room_button(struct GuiButton *gbtn);
void pick_up_next_creature(struct GuiButton *gbtn);
void gui_go_to_next_creature(struct GuiButton *gbtn);
void gui_area_anger_button(struct GuiButton *gbtn);
void gui_area_smiley_anger_button(struct GuiButton *gbtn);
void gui_area_experience_button(struct GuiButton *gbtn);
void gui_area_instance_button(struct GuiButton *gbtn);
void maintain_instance(struct GuiButton *gbtn);
void maintain_activity_up(struct GuiButton *gbtn);
void maintain_activity_down(struct GuiButton *gbtn);
void maintain_activity_pic(struct GuiButton *gbtn);
void maintain_activity_row(struct GuiButton *gbtn);
void gui_scroll_activity_up(struct GuiButton *gbtn);
void gui_scroll_activity_up(struct GuiButton *gbtn);
void gui_scroll_activity_down(struct GuiButton *gbtn);
void gui_scroll_activity_down(struct GuiButton *gbtn);
void maintain_activity_up(struct GuiButton *gbtn);
void maintain_activity_down(struct GuiButton *gbtn);
void maintain_activity_pic(struct GuiButton *gbtn);
void maintain_activity_row(struct GuiButton *gbtn);
void gui_activity_background(struct GuiMenu *gmnu);
void gui_area_ally(struct GuiButton *gbtn);
void gui_area_stat_button(struct GuiButton *gbtn);
void maintain_event_button(struct GuiButton *gbtn);
void gui_toggle_ally(struct GuiButton *gbtn);
void maintain_ally(struct GuiButton *gbtn);
void maintain_prison_bar(struct GuiButton *gbtn);
void maintain_room_and_creature_button(struct GuiButton *gbtn);
void pick_up_next_wanderer(struct GuiButton *gbtn);
void gui_go_to_next_wanderer(struct GuiButton *gbtn);
void pick_up_next_worker(struct GuiButton *gbtn);
void gui_go_to_next_worker(struct GuiButton *gbtn);
void pick_up_next_fighter(struct GuiButton *gbtn);
void gui_go_to_next_fighter(struct GuiButton *gbtn);
void gui_area_payday_button(struct GuiButton *gbtn);
void gui_area_research_bar(struct GuiButton *gbtn);
void gui_area_workshop_bar(struct GuiButton *gbtn);
void gui_area_player_creature_info(struct GuiButton *gbtn);
void gui_area_player_room_info(struct GuiButton *gbtn);
void spell_lost_first_person(struct GuiButton *gbtn);
void gui_set_tend_to(struct GuiButton *gbtn);
void gui_set_query(struct GuiButton *gbtn);
/******************************************************************************/
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
  { 3, BID_CREATR_TAB, 0, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               5, 112, 154, 112, 154, 28, 34, gui_draw_tab,                     15, 451,  0,{(long)&creature_tag}, 0, 0, menu_tab_maintain },
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

struct GuiButtonInit event_menu_buttons[] = {
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
struct GuiMenu query_menu =
 { 7, 0, 1, query_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu event_menu =
 { 6, 0, 1, event_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu creature_query_menu1 =
 { 31, 0, 1, creature_query_buttons1,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu2 =
 { 35, 0, 1, creature_query_buttons2,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu3 =
 { 32, 0, 1, creature_query_buttons3,             0,          0, 140, 400, gui_creature_query_background2,0,NULL,   NULL,                    0, 0, 1,};
/******************************************************************************/
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

void gui_turn_on_autopilot(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    player = get_my_player();
    if (player->victory_state != VicS_LostLevel)
    {
      set_players_packet_action(player, 107, 0, 0, 0, 0);
    }
}

void menu_tab_maintain(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  set_flag_byte(&gbtn->field_0, 0x08, (player->victory_state != VicS_LostLevel));
}

void gui_area_autopilot_button(struct GuiButton *gbtn)
{
  _DK_gui_area_autopilot_button(gbtn);
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

void gui_choose_room(struct GuiButton *gbtn)
{
    // prepare to enter room build mode
    activate_room_build_mode((long)gbtn->field_33, gbtn->tooltip_id);
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

void gui_remove_area_for_rooms(struct GuiButton *gbtn)
{
  _DK_gui_remove_area_for_rooms(gbtn);
}

void gui_area_big_room_button(struct GuiButton *gbtn)
{
  _DK_gui_area_big_room_button(gbtn);
}

/**
 * Sets a new chosen spell.
 * Fills packet with the spell disable action.
 */
void gui_choose_spell(struct GuiButton *gbtn)
{
    //NOTE by Petter: factored out original gui_choose_spell code to choose_spell
    choose_spell((int) gbtn->field_33, gbtn->tooltip_id);
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
    //NOTE by Petter: factored out original gui_choose_special_spell code to choose_special_spell
    //TODO: equivalent to gui_choose_spell now... try merge
    choose_spell(((int) gbtn->field_33) % POWER_TYPES_COUNT, gbtn->tooltip_id);
}

void gui_area_big_spell_button(struct GuiButton *gbtn)
{
  _DK_gui_area_big_spell_button(gbtn);
}

/**
 * Choose a trap or a door.
 * @param kind An index into trap_data array, beware as this is different from models.
 * @param tooltip_id The tooltip string to display.
 */
void choose_workshop_item(int kind, int tooltip_id)
{
    PlayerInfo * player;

    kind = kind % MANUFCTR_TYPES_COUNT;

    player = get_my_player();
    set_players_packet_action(player, 36, trap_data[kind].field_0,
        trap_data[kind].field_4, 0, 0);

    game.numfield_151819 = kind;
    game.numfield_15181D = trap_data[kind].field_8;
    game.numfield_151821 = tooltip_id;
}

void gui_choose_trap(struct GuiButton *gbtn)
{
    //_DK_gui_choose_trap(gbtn);

    //Note by Petter: factored out gui_choose_trap to choose_workshop_item (better name as well)
    choose_workshop_item((int) gbtn->field_33, gbtn->tooltip_id);
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

void maintain_big_spell(struct GuiButton *gbtn)
{
  _DK_maintain_big_spell(gbtn);
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

void pick_up_next_creature(struct GuiButton *gbtn)
{
    int kind;
    int i;
    unsigned pick_flags;

    //_DK_pick_up_next_creature(gbtn);

    i = gbtn->field_1B;
    if (i > 0) {
        kind = breed_activities[(i + top_of_breed_list) % CREATURE_TYPES_COUNT];
    }
    else {
        kind = 23;
    }

    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(kind, -1, my_player_number, pick_flags);
}

void gui_go_to_next_creature(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_creature(gbtn);
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

void maintain_instance(struct GuiButton *gbtn)
{
  _DK_maintain_instance(gbtn);
}

void gui_activity_background(struct GuiMenu *gmnu)
{
  SYNCDBG(9,"Starting");
  _DK_gui_activity_background(gmnu);
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

void gui_scroll_activity_up(struct GuiButton *gbtn)
{
  _DK_gui_scroll_activity_up(gbtn);
}

void gui_scroll_activity_down(struct GuiButton *gbtn)
{
  _DK_gui_scroll_activity_down(gbtn);
}

void gui_area_ally(struct GuiButton *gbtn)
{
  _DK_gui_area_ally(gbtn);
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
        if (dungeon->tortured_creatures[thing->model] > 0)
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

void gui_toggle_ally(struct GuiButton *gbtn)
{
  _DK_gui_toggle_ally(gbtn);
}

void maintain_ally(struct GuiButton *gbtn)
{
  _DK_maintain_ally(gbtn);
}

void maintain_prison_bar(struct GuiButton *gbtn)
{
  _DK_maintain_prison_bar(gbtn);
}

void maintain_room_and_creature_button(struct GuiButton *gbtn)
{
  _DK_maintain_room_and_creature_button(gbtn);
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

void spell_lost_first_person(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player=get_my_player();
  set_players_packet_action(player, 110, 0, 0, 0, 0);
}

void gui_set_tend_to(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  set_players_packet_action(player, PckA_ToggleTendency, gbtn->field_1B, 0, 0, 0);
}

void gui_set_query(struct GuiButton *gbtn)
{
  //_DK_gui_set_query(gbtn);
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_SetPlyrState, 12, 0, 0, 0);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
