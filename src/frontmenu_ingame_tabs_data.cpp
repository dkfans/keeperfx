/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_tabs_data.cpp
 *     Main in-game GUI, visible during gameplay.
 * @par Purpose:
 *     Structures to show and maintain tabbed menu appearing ingame.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 11 Feb 2013
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

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"

#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "frontmenu_saves.h"
#include "config_settings.h"
#include "config_strings.h"
#include "frontmenu_options.h"
#include "game_legacy.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_opts.h"

#ifdef __cplusplus
extern "C" {
#endif
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
void maintain_room_button(struct GuiButton *gbtn);
void maintain_creature_button(struct GuiButton* gbtn);
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
  {LbBtnT_NormalBtn,BID_MAP_ZOOM_IN, 0, 0,          gui_zoom_in,           NULL,  NULL,               0, 112,   4, 114,   4, 26, 66, gui_area_new_vertical_button,    237, GUIStr_PaneZoomInDesc,      0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_MAP_ZOOM_OU, 0, 0,         gui_zoom_out,           NULL,  NULL,               0, 110,  70, 114,  70, 26, 66, gui_area_new_vertical_button,    239, GUIStr_PaneZoomOutDesc,     0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_MAP_ZOOM_FS, 0, 0,        gui_go_to_map,           NULL,  NULL,               0,   0,   0,   0,   0, 30, 31, gui_area_new_vertical_button,    304, GUIStr_PaneLargeMapDesc,    0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0,gui_turn_on_autopilot,           NULL,  NULL,               0,   0,  70,   0,  70, 16, 67, gui_area_autopilot_button,       492, GUIStr_Empty,               0,       {0},            0, maintain_turn_on_autopilot },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0,                 NULL,           NULL,  NULL,               0,  68,   0,  68,   0, 68, 16, gui_area_new_normal_button,      499, GUIStr_MnuOptionsDesc,&options_menu, {0},        0, NULL },
  { LbBtnT_RadioBtn,   BID_INFO_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               7,   0, 154,   0, 154, 28, 34, gui_draw_tab,                      7, GUIStr_InformationPanelDesc,0,{(long)&info_tag},     0, menu_tab_maintain },
  { LbBtnT_RadioBtn,   BID_ROOM_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               2,  28, 154,  28, 154, 28, 34, gui_draw_tab,                      9, GUIStr_RoomPanelDesc,       0,{(long)&room_tag},     0, menu_tab_maintain },
  { LbBtnT_RadioBtn,  BID_SPELL_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               3,  56, 154,  56, 154, 28, 34, gui_draw_tab,                     11, GUIStr_ResearchPanelDesc,   0,{(long)&spell_tag},    0, menu_tab_maintain },
  { LbBtnT_RadioBtn,  BID_MNFCT_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               4,  84, 154,  84, 154, 28, 34, gui_draw_tab,                     13, GUIStr_WorkshopPanelDesc,   0,{(long)&trap_tag},     0, menu_tab_maintain },
  { LbBtnT_RadioBtn, BID_CREATR_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               5, 112, 154, 112, 154, 28, 34, gui_draw_tab,                     15, GUIStr_CreaturePanelDesc,   0,{(long)&creature_tag}, 0, menu_tab_maintain },
  {LbBtnT_NormalBtn,   BID_MSG_EV01, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 360, 138, 360, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {0},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV02, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 330, 138, 330, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {1},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV03, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 300, 138, 300, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {2},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV04, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 270, 138, 270, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {3},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV05, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 240, 138, 240, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {4},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV06, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 210, 138, 210, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {5},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV07, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 180, 138, 180, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {6},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV08, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 150, 138, 150, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {7},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV09, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 120, 138, 120, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {8},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV10, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  90, 138,  90, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,       {9},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV11, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  60, 138,  60, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,      {10},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV12, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  30, 138,  30, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,      {11},            0, maintain_event_button },
  {LbBtnT_NormalBtn,   BID_MSG_EV13, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,   0, 138,   0, 24, 30, gui_area_event_button,             0, GUIStr_Empty,               0,      {12},            0, maintain_event_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0,                 NULL,           NULL,  NULL,               0,  22, 122,  22, 122, 94, 40, NULL,                              0, GUIStr_TeamMoneyAvailable,  0,       {0},            0, NULL },
  {              -1,    BID_DEFAULT, 0, 0,                 NULL,           NULL,  NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                        0,       {0},            0, NULL },
};

struct GuiButtonInit room_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_ROOM_TD01, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,238,  6, 242, 32, 36, gui_area_room_button,             57, CpgStr_RoomDesc1+0,  0,       {2},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD03, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,238, 38, 242, 32, 36, gui_area_room_button,             79, CpgStr_RoomDesc1+10, 0,      {14},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD02, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,238, 70, 242, 32, 36, gui_area_room_button,             59, CpgStr_RoomDesc1+9,  0,      {13},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD05, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,238,102, 242, 32, 36, gui_area_room_button,             67, CpgStr_RoomDesc1+3,  0,       {6},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD04, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,276,  6, 280, 32, 36, gui_area_room_button,             61, CpgStr_RoomDesc1+1,  0,       {3},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD13, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,276, 38, 280, 32, 36, gui_area_room_button,             81, CpgStr_RoomDesc1+11, 0,      {15},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD14, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,276, 70, 280, 32, 36, gui_area_room_button,             83, CpgStr_RoomDesc1+12, 0,      {16},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD08, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,276,102, 280, 32, 36, gui_area_room_button,             75, CpgStr_RoomDesc1+6,  0,       {8},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD06, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,314,  6, 318, 32, 36, gui_area_room_button,             65, CpgStr_RoomDesc1+2,  0,       {4},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD12, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,314, 38, 318, 32, 36, gui_area_room_button,             63, CpgStr_RoomDesc1+4,  0,       {5},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD11, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,314, 70, 318, 32, 36, gui_area_room_button,             69, CpgStr_RoomDesc1+8,  0,      {12},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD07, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,314,102, 318, 32, 36, gui_area_room_button,             73, CpgStr_RoomDesc1+13, 0,      {10},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD10, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,352,  6, 356, 32, 36, gui_area_room_button,             71, CpgStr_RoomDesc1+7,  0,      {11},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD09, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,352, 38, 356, 32, 36, gui_area_room_button,             77, CpgStr_RoomDesc1+14, 0,       {9},               0, maintain_room },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,        0,       {0},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD16, 0, 0, gui_remove_area_for_rooms,NULL,NULL,                 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button,     107, GUIStr_SellRoomDesc, 0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_room_button,          0, GUIStr_Empty,        0,       {0},               0, maintain_big_room },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                 0,       {0},               0, NULL },
};

struct GuiButtonInit spell_menu_buttons[] = {
  {LbBtnT_NormalBtn, BID_POWER_TD16, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 238,   6, 242, 32, 36, gui_area_spell_button,           114, CpgStr_PowerDesc1+0,  0,      {18},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD01, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 238,  38, 242, 32, 36, gui_area_spell_button,           118, CpgStr_PowerDesc1+1,  0,       {2},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD02, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 238,  70, 242, 32, 36, gui_area_spell_button,           108, CpgStr_PowerDesc1+2,  0,       {5},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD07, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 238, 102, 242, 32, 36, gui_area_spell_button,           122, CpgStr_PowerDesc1+7,  0,      {11},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD15, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 276,   6, 280, 32, 36, gui_area_spell_button,           452, CpgStr_PowerDesc1+6,  0,       {3},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD03, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 276,  38, 280, 32, 36, gui_area_spell_button,           116, CpgStr_PowerDesc1+3,  0,       {6},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD09, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 276,  70, 280, 32, 36, gui_area_spell_button,           128, CpgStr_PowerDesc1+9,  0,      {13},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD14, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  98, 276, 102, 280, 32, 36, gui_area_spell_button,           112, CpgStr_PowerDesc1+4,  0,       {9},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD04, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 314,   6, 318, 32, 36, gui_area_spell_button,           120, CpgStr_PowerDesc1+5,  0,       {7},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD06, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 314,  38, 318, 32, 36, gui_area_spell_button,           110, CpgStr_PowerDesc1+14, 0,       {8},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD05, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 314,  70, 318, 32, 36, gui_area_spell_button,           124, CpgStr_PowerDesc1+10, 0,      {10},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD08, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 314, 102, 318, 32, 36, gui_area_spell_button,           126, CpgStr_PowerDesc1+8,  0,      {12},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD10, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 352,   6, 356, 32, 36, gui_area_spell_button,           314, CpgStr_PowerDesc1+11, 0,      {15},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD11, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 352,  38, 356, 32, 36, gui_area_spell_button,           319, CpgStr_PowerDesc1+12, 0,      {14},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD13, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_spell_button,           321, CpgStr_PowerDesc1+16, 0,      {19},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD12, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 352, 102, 356, 32, 36, gui_area_spell_button,           317, CpgStr_PowerDesc1+13, 0,      {16},               0, maintain_spell },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_spell_button,         0, GUIStr_Empty,         0,       {0},               0, maintain_big_spell },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                  0,       {0},               0, NULL },
};

struct GuiButtonInit spell_lost_menu_buttons[] = {
  {LbBtnT_NormalBtn, BID_POWER_TD16, 0, 0, spell_lost_first_person,NULL,    NULL,               0,   2, 238,   8, 250, 32, 36, gui_area_new_null_button,        114, CpgStr_PowerDesc1+0,  0,      {18},               0, maintain_spell },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 238,  40, 250, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 238,  72, 250, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 238, 104, 250, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   2, 276,   8, 288, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 276,  40, 288, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  72, 288, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 104, 288, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   2, 314,   8, 326, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 314,  40, 326, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 314,  72, 326, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 314, 104, 326, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   8, 364, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  40, 364, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  72, 364, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 352, 104, 364, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_spell_button,         0, GUIStr_Empty,         0,       {0},               0, NULL },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                  0,       {0},               0, NULL },
};

struct GuiButtonInit trap_menu_buttons[] = {
  {LbBtnT_NormalBtn, BID_MNFCT_TD02, 0, 0, NULL,                      NULL, NULL, 0,   2, 238,   6, 242, 32, 36, NULL,                            154, CpgStr_AlarmTrapDesc,     0,       {2},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD03, 0, 0, NULL,                      NULL, NULL, 0,  34, 238,  38, 242, 32, 36, NULL,                            156, CpgStr_PoisonGasTrapDesc, 0,       {3},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD04, 0, 0, NULL,                      NULL, NULL, 0,  66, 238,  70, 242, 32, 36, NULL,                            158, CpgStr_LightningTrapDesc, 0,       {4},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD15, 0, 0, NULL,                      NULL, NULL, 0,  98, 238, 102, 242, 32, 36, NULL,                            162, CpgStr_LavaTrapDesc,      0,       {6},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD01, 0, 0, NULL,                      NULL, NULL, 0,   2, 276,   6, 280, 32, 36, NULL,                            152, CpgStr_TrapBoulderDesc,   0,       {1},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD05, 0, 0, NULL,                      NULL, NULL, 0,  34, 276,  38, 280, 32, 36, NULL,                            160, CpgStr_WordOfPowerTrapDesc,0,      {5},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD11, 0, 0, NULL,                      NULL, NULL, 0,  66, 276,  70, 280, 32, 36, NULL,                             24, GUIStr_Empty,             0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD12, 0, 0, NULL,                      NULL, NULL, 0,  98, 276, 102, 280, 32, 36, NULL,                             24, GUIStr_Empty,             0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD06, 0, 0, NULL,                      NULL, NULL, 0,   2, 314,   6, 318, 32, 36, NULL,                            166, CpgStr_WoodenDoorDesc,    0,       {7},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD07, 0, 0, NULL,                      NULL, NULL, 0,  34, 314,  38, 318, 32, 36, NULL,                            168, CpgStr_BracedDoorDesc,    0,       {8},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD08, 0, 0, NULL,                      NULL, NULL, 0,  66, 314,  70, 318, 32, 36, NULL,                            170, CpgStr_IronDoorDesc,      0,       {9},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD09, 0, 0, NULL,                      NULL, NULL, 0,  98, 314, 102, 318, 32, 36, NULL,                            172, CpgStr_MagicDoorDesc,     0,      {10},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD13, 0, 0, NULL,                      NULL, NULL, 0,   2, 352,   6, 356, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,             0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD14, 0, 0, NULL,                      NULL, NULL, 0,  34, 352,  38, 356, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,             0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD16, 0, 0, NULL,                      NULL, NULL, 0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button,         24, GUIStr_Empty,             0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD10, 0, 0, gui_remove_area_for_traps, NULL, NULL, 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button,     107, GUIStr_SellItemDesc,      0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0,   8, 210,   8, 194,126, 44, gui_area_big_trap_button,          0, GUIStr_Empty,             0,       {0},               0, maintain_big_trap },
  {              -1,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                      0,       {0},               0, NULL },
};

struct GuiButtonInit creature_menu_buttons[] = {
  {LbBtnT_NormalBtn,BID_CRTR_NXWNDR, 0, 0, pick_up_next_wanderer,gui_go_to_next_wanderer,NULL,  0,  26, 192,  26, 192, 38, 24, gui_area_new_normal_button,      284, GUIStr_CreatureIdleDesc,       0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_CRTR_NXWRKR, 0, 0, pick_up_next_worker,gui_go_to_next_worker,NULL,      0,  62, 192,  62, 192, 38, 24, gui_area_new_normal_button,      282, GUIStr_CreatureWorkingDesc,    0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_CRTR_NXFIGT, 0, 0, pick_up_next_fighter,gui_go_to_next_fighter,NULL,    0,  98, 192,  98, 192, 38, 24, gui_area_new_normal_button,      286, GUIStr_CreatureFightingDesc,   0,       {0},            0, NULL },
  {LbBtnT_HoldableBtn,  BID_DEFAULT, 0, 0, gui_scroll_activity_up,gui_scroll_activity_up,NULL,  0,   4, 192,   4, 192, 22, 24, gui_area_new_normal_button,      278, GUIStr_Empty,                  0,       {0},            0, maintain_activity_up },
  {LbBtnT_HoldableBtn,  BID_DEFAULT, 0, 0, gui_scroll_activity_down,gui_scroll_activity_down,NULL,0, 4, 364,   4, 364, 22, 24, gui_area_new_normal_button,      280, GUIStr_Empty,                  0,       {0},            0, maintain_activity_down },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  0,   0, 196,   0, 218, 22, 22, gui_area_creatrmodel_button,       0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,26,220,26,220,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrIdleDesc,     0,{(long)&activity_list[0+0]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,62,220,62,220,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrWorkingDesc,  0,{(long)&activity_list[0+1]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,98,220,98,220,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrFightingDesc, 0,{(long)&activity_list[0+2]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  1,   0, 220,   0, 242, 22, 22, gui_area_creatrmodel_button,       0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,26,244,26,244,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrIdleDesc,     0,{(long)&activity_list[4+0]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,62,244,62,244,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrWorkingDesc,  0,{(long)&activity_list[4+1]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,98,244,98,244,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrFightingDesc, 0,{(long)&activity_list[4+2]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  2,   0, 244,   0, 266, 22, 22, gui_area_creatrmodel_button,       0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,26,268,26,268,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrIdleDesc,     0,{(long)&activity_list[8]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,62,268,62,268,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrWorkingDesc,  0,{(long)&activity_list[9]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,98,268,98,268,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrFightingDesc, 0,{(long)&activity_list[10]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  3,   0, 268,   0, 290, 22, 22, gui_area_creatrmodel_button,       0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,26,292,26,292,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrIdleDesc,     0,{(long)&activity_list[12]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,62,292,62,292,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrWorkingDesc,  0,{(long)&activity_list[13]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,98,292,98,292,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrFightingDesc, 0,{(long)&activity_list[14]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  4,   0, 292,   0, 314, 22, 22, gui_area_creatrmodel_button,       0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,26,316,26,316,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrIdleDesc,     0,{(long)&activity_list[16]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,62,316,62,316,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrWorkingDesc,  0,{(long)&activity_list[17]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,98,316,98,316,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrFightingDesc, 0,{(long)&activity_list[18]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  5,   0, 314,   0, 338, 22, 22, gui_area_creatrmodel_button,       0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,26,340,26,340,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrIdleDesc,     0,{(long)&activity_list[20]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,62,340,62,340,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrWorkingDesc,  0,{(long)&activity_list[21]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,98,340,98,340,32,20,gui_area_anger_button,   288, GUIStr_PickCreatrFightingDesc, 0,{(long)&activity_list[22]},0,maintain_activity_row },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                           0,       {0},            0, NULL },
};

struct GuiButtonInit query_menu_buttons[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, gui_set_query,      NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      475, GUIStr_GoToQueryMode,        0,       {0},            0, NULL },
  {LbBtnT_ToggleBtn, BID_QRY_IMPRSN, 0, 0, gui_set_tend_to,    NULL,        NULL,               1,  36, 190,  36, 190, 32, 26, gui_area_flash_cycle_button,     350, GUIStr_CreatureImprisonDesc, 0,{(long)&game.creatures_tend_imprison}, 1, maintain_prison_bar },
  {LbBtnT_ToggleBtn,   BID_QRY_FLEE, 0, 0, gui_set_tend_to,    NULL,        NULL,               2,  74, 190,  74, 190, 32, 26, gui_area_flash_cycle_button,     346, GUIStr_CreatureFleeDesc,     0,{(long)&game.creatures_tend_flee}, 1, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 216,   4, 222,132, 24, gui_area_payday_button,          341, GUIStr_PayTimeDesc,          0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   2, 246,   2, 246, 60, 24, gui_area_research_bar,            61, GUIStr_ResearchTimeDesc,     0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  74, 246,  74, 246, 60, 24, gui_area_workshop_bar,            75, GUIStr_WorkshopTimeDesc,     0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  74, 274,  74, 274, 60, 24, gui_area_player_creature_info,   323, GUIStr_NumberOfCreaturesDesc,0,       {0},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  74, 298,  74, 298, 60, 24, gui_area_player_creature_info,   325, GUIStr_NumberOfCreaturesDesc,0,       {1},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  74, 322,  74, 322, 60, 24, gui_area_player_creature_info,   327, GUIStr_NumberOfCreaturesDesc,0,       {2},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  74, 346,  74, 346, 60, 24, gui_area_player_creature_info,   329, GUIStr_NumberOfCreaturesDesc,0,       {3},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 274,   4, 274, 60, 24, gui_area_player_room_info,       324, GUIStr_NumberOfRoomsDesc,    0,       {0},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 298,   4, 298, 60, 24, gui_area_player_room_info,       326, GUIStr_NumberOfRoomsDesc,    0,       {1},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 322,   4, 322, 60, 24, gui_area_player_room_info,       328, GUIStr_NumberOfRoomsDesc,    0,       {2},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_player_room_info,       330, GUIStr_NumberOfRoomsDesc,    0,       {3},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 274,  62, 274, 14, 22, gui_area_ally,                     0, GUIStr_AllyWithPlayer,       0,       {0},            0, maintain_ally },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 298,  62, 298, 14, 22, gui_area_ally,                     0, GUIStr_AllyWithPlayer,       0,       {1},            0, maintain_ally },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 322,  62, 322, 14, 22, gui_area_ally,                     0, GUIStr_AllyWithPlayer,       0,       {2},            0, maintain_ally },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 346,  62, 346, 14, 22, gui_area_ally,                     0, GUIStr_AllyWithPlayer,       0,       {3},            0, maintain_ally },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                         0,       {0},            0, NULL },
};

struct GuiButtonInit event_menu_buttons[] = {
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                         0,       {0},            0, NULL },
};

struct GuiButtonInit creature_query_buttons1[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, GUIStr_MoreInformation,&creature_query_menu2,{0},    0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button,    466, GUIStr_CreatureAngerDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button,      467, GUIStr_ExperienceDesc,   0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, GUIStr_NameAndHealthDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {0},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {1},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {2},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {3},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {4},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {5},               0, maintain_instance },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},               0, NULL },
};

struct GuiButtonInit creature_query_buttons2[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, GUIStr_MoreInformation,&creature_query_menu3,{0},    0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button,    466, GUIStr_CreatureAngerDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button,      467, GUIStr_ExperienceDesc,   0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, GUIStr_NameAndHealthDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {4},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {5},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {6},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {7},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {8},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {9},               0, maintain_instance },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},               0, NULL },
};

struct GuiButtonInit creature_query_buttons3[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, GUIStr_MoreInformation,&creature_query_menu4,          {0}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 226,   4, 226, 60, 24, gui_area_stat_button,            414, GUIStr_CreatureHealthDesc,     0,         {CrLStat_Health}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 226,  72, 226, 60, 24, gui_area_stat_button,            332, GUIStr_CreatureStrengthDesc,   0,       {CrLStat_Strength}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 256,   4, 256, 60, 24, gui_area_stat_button,            335, GUIStr_CreatureArmourDesc,     0,         {CrLStat_Armour}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 256,  72, 256, 60, 24, gui_area_stat_button,            338, GUIStr_CreatureDefenceDesc,    0,        {CrLStat_Defence}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 286,   4, 286, 60, 24, gui_area_stat_button,            339, GUIStr_CreatureLuckDesc,       0,           {CrLStat_Luck}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 286,  72, 286, 60, 24, gui_area_stat_button,            337, GUIStr_CreatureDexterityDesc,  0,      {CrLStat_Dexterity}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 316,   4, 316, 60, 24, gui_area_stat_button,            334, GUIStr_CreatureWageDesc,       0,       {CrLStat_GoldWage}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 316,  72, 316, 60, 24, gui_area_stat_button,            333, GUIStr_CreatureGoldHeldDesc,   0,       {CrLStat_GoldHeld}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_stat_button,            336, GUIStr_CreatureTimeInDungeonDesc,0,      {CrLStat_AgeTime}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_stat_button,            331, GUIStr_CreatureKillsDesc,      0,          {CrLStat_Kills}, 0, NULL },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                           0,                      {0}, 0, NULL },
};

struct GuiButtonInit creature_query_buttons4[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, GUIStr_MoreInformation,&creature_query_menu1,           {0}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 226,   4, 226, 60, 24, gui_area_stat_button,            423, GUIStr_CreatureSpeedDesc,       0,          {CrLStat_Speed}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 226,  72, 226, 60, 24, gui_area_stat_button,            452, GUIStr_CreatureLoyaltyDesc,     0,        {CrLStat_Loyalty}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 256,   4, 256, 60, 24, gui_area_stat_button,            61, GUIStr_CreatureResrchSkillDesc,  0,  {CrLStat_ResearchSkill}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 256,  72, 256, 60, 24, gui_area_stat_button,            75, GUIStr_CreatureManfctrSkillDesc, 0,{CrLStat_ManufactureSkill},0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 286,   4, 286, 60, 24, gui_area_stat_button,            67, GUIStr_CreatureTraingSkillDesc,  0,  {CrLStat_TrainingSkill}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 286,  72, 286, 60, 24, gui_area_stat_button,            77, GUIStr_CreatureScavngSkillDesc,  0,  {CrLStat_ScavengeSkill}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 316,   4, 316, 60, 24, gui_area_stat_button,            515, GUIStr_CreatureTraingCostDesc,  0,   {CrLStat_TrainingCost}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 316,  72, 316, 60, 24, gui_area_stat_button,            516, GUIStr_CreatureScavngCostDesc,  0,   {CrLStat_ScavengeCost}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_stat_button,            152, GUIStr_CreatureWeightDesc,      0,         {CrLStat_Weight}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_stat_button,            340, GUIStr_CreatureBloodTypeDesc,   0,      {CrLStat_BloodType}, 0, NULL },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                            0,                      {0}, 0, NULL },
};

struct GuiMenu main_menu =
 {           GMnu_MAIN, 0, 1, main_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu room_menu =
 {           GMnu_ROOM, 0, 1, room_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu spell_menu =
 {          GMnu_SPELL, 0, 1, spell_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu spell_lost_menu =
 {     GMnu_SPELL_LOST, 0, 1, spell_lost_menu_buttons,                     0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu trap_menu =
 {           GMnu_TRAP, 0, 1, trap_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu creature_menu =
 {       GMnu_CREATURE, 0, 1, creature_menu_buttons,                       0,   0, 140, 400, gui_activity_background,     0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu query_menu =
 {          GMnu_QUERY, 0, 1, query_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu event_menu =
 {          GMnu_EVENT, 0, 1, event_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu creature_query_menu1 =
 {GMnu_CREATURE_QUERY1, 0, 1, creature_query_buttons1,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu2 =
 {GMnu_CREATURE_QUERY2, 0, 1, creature_query_buttons2,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu3 =
 {GMnu_CREATURE_QUERY3, 0, 1, creature_query_buttons3,             0,          0, 140, 400, gui_creature_query_background2,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu4 =
 {GMnu_CREATURE_QUERY4, 0, 1, creature_query_buttons4,             0,          0, 140, 400, gui_creature_query_background2,0,NULL,   NULL,                    0, 0, 1,};

struct TiledSprite status_panel = {
    2, 4, {
        { 1, 2,},
        { 3, 4,},
        { 5, 6,},
        {21,22,},
    },
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
