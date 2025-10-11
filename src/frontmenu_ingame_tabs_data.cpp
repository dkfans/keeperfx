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
#include "pre_inc.h"
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
#include "sprites.h"
#include "player_instances.h"
#include "post_inc.h"

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
void gui_choose_workshop_item(struct GuiButton *gbtn);
void gui_go_to_next_trap(struct GuiButton *gbtn);
void gui_over_trap_button(struct GuiButton *gbtn);
void maintain_trap(struct GuiButton *gbtn);
void gui_area_trap_button(struct GuiButton *gbtn);
void gui_go_to_next_door(struct GuiButton *gbtn);
void maintain_door(struct GuiButton *gbtn);
void gui_over_door_button(struct GuiButton *gbtn);
void gui_remove_area_for_traps(struct GuiButton *gbtn);
void gui_area_big_trap_button(struct GuiButton *gbtn);
void gui_area_trap_build_info_button(struct GuiButton* gbtn);
void maintain_big_trap(struct GuiButton *gbtn);
void gui_creature_query_background1(struct GuiMenu *gmnu);
void gui_creature_query_background2(struct GuiMenu *gmnu);
void maintain_room(struct GuiButton *gbtn);
void maintain_big_room(struct GuiButton *gbtn);
void maintain_spell(struct GuiButton *gbtn);
void maintain_big_spell(struct GuiButton *gbtn);
void maintain_trap(struct GuiButton *gbtn);
void maintain_door(struct GuiButton *gbtn);
void maintain_buildable_info(struct GuiButton *gbtn);
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
void maintain_query_button(struct GuiButton *gbtn);
void maintain_player_page2(struct GuiButton *gbtn);
/******************************************************************************/
struct GuiButtonInit main_menu_buttons[] = {
  {LbBtnT_NormalBtn,    BID_OPTIONS, 0, 0,                 NULL,           NULL,  NULL,               0,  68,   0,  68,   0, 68, 16, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_quit_act, GUIStr_MnuOptionsDesc,&options_menu, {0},        0, NULL },
  {LbBtnT_NormalBtn,BID_MAP_ZOOM_IN, 0, 0,          gui_zoom_in,           NULL,  NULL,               0, 112,   4, 114,   4, 26, 66, gui_area_new_vertical_button, GPS_rpanel_rpanel_mapbt2a, GUIStr_PaneZoomInDesc,      0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_MAP_ZOOM_OU, 0, 0,         gui_zoom_out,           NULL,  NULL,               0, 110,  70, 114,  70, 26, 66, gui_area_new_vertical_button, GPS_rpanel_rpanel_mapbt3a, GUIStr_PaneZoomOutDesc,     0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_MAP_ZOOM_FS, 0, 0,        gui_go_to_map,           NULL,  NULL,               0,   0,   0,   0,   0, 30, 31, gui_area_new_vertical_button, GPS_rpanel_rpanel_btn_bigmap_act, GUIStr_PaneLargeMapDesc,    0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0,gui_turn_on_autopilot,           NULL,  NULL,               0,   0,  70,   0,  70, 16, 67, gui_area_autopilot_button, GPS_rpanel_rpanel_btn_cassisti_act, GUIStr_Empty,               0,       {0},            0, maintain_turn_on_autopilot },
  { LbBtnT_RadioBtn,   BID_INFO_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,      GMnu_QUERY,   0, 154,   0, 154, 28, 34, gui_draw_tab, GPS_rpanel_rpanel_tab_infoa, GUIStr_InformationPanelDesc,0,{.ptr = &info_tag},     0, menu_tab_maintain },
  { LbBtnT_RadioBtn,   BID_ROOM_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,       GMnu_ROOM,  28, 154,  28, 154, 28, 34, gui_draw_tab, GPS_rpanel_rpanel_tab_rooma, GUIStr_RoomPanelDesc,       0,{.ptr = &room_tag},     0, menu_tab_maintain },
  { LbBtnT_RadioBtn,  BID_SPELL_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,      GMnu_SPELL,  56, 154,  56, 154, 28, 34, gui_draw_tab, GPS_rpanel_rpanel_tab_spela, GUIStr_ResearchPanelDesc,   0,{.ptr = &spell_tag},    0, menu_tab_maintain },
  { LbBtnT_RadioBtn,  BID_MNFCT_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,       GMnu_TRAP,  84, 154,  84, 154, 28, 34, gui_draw_tab, GPS_rpanel_rpanel_tab_wrksha, GUIStr_WorkshopPanelDesc,   0,{.ptr = &trap_tag},     0, menu_tab_maintain },
  { LbBtnT_RadioBtn, BID_CREATR_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,   GMnu_CREATURE, 112, 154, 112, 154, 28, 34, gui_draw_tab, GPS_rpanel_rpanel_tab_crtra, GUIStr_CreaturePanelDesc,   0,{.ptr = &creature_tag}, 0, menu_tab_maintain },
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
  {LbBtnT_NormalBtn,  BID_ROOM_TD01, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,238,  6, 242, 32, 36, gui_area_room_button, GPS_room_treasury_std_s, CpgStr_RoomDesc1+0,  0,       {2},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD03, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,238, 38, 242, 32, 36, gui_area_room_button, GPS_room_lair_std_s, CpgStr_RoomDesc1+10, 0,      {14},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD02, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,238, 70, 242, 32, 36, gui_area_room_button, GPS_room_hatchery_std_s, CpgStr_RoomDesc1+9,  0,      {13},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD05, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,238,102, 242, 32, 36, gui_area_room_button, GPS_room_training_std_s, CpgStr_RoomDesc1+3,  0,       {6},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD04, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,276,  6, 280, 32, 36, gui_area_room_button, GPS_room_research_std_s, CpgStr_RoomDesc1+1,  0,       {3},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD13, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,276, 38, 280, 32, 36, gui_area_room_button, GPS_room_bridge_std_s, CpgStr_RoomDesc1+11, 0,      {15},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD14, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,276, 70, 280, 32, 36, gui_area_room_button, GPS_room_grdpost_std_s, CpgStr_RoomDesc1+12, 0,      {16},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD08, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,276,102, 280, 32, 36, gui_area_room_button, GPS_room_workshop_std_s, CpgStr_RoomDesc1+6,  0,       {8},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD06, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,314,  6, 318, 32, 36, gui_area_room_button, GPS_room_prison_std_s, CpgStr_RoomDesc1+2,  0,       {4},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD12, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,314, 38, 318, 32, 36, gui_area_room_button, GPS_room_torture_std_s, CpgStr_RoomDesc1+4,  0,       {5},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD11, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,314, 70, 318, 32, 36, gui_area_room_button, GPS_room_armory_std_s, CpgStr_RoomDesc1+8,  0,      {12},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD07, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,314,102, 318, 32, 36, gui_area_room_button, GPS_room_temple_std_s, CpgStr_RoomDesc1+13, 0,      {10},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD10, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,352,  6, 356, 32, 36, gui_area_room_button, GPS_room_graveyard_std_s, CpgStr_RoomDesc1+7,  0,      {11},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD09, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,352, 38, 356, 32, 36, gui_area_room_button, GPS_room_scavenge_std_s, CpgStr_RoomDesc1+14, 0,       {9},               0, maintain_room },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,        0,       {0},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD16, 0, 0, gui_remove_area_for_rooms,NULL,NULL,                 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button, GPS_rpanel_frame_portrt_sell, GUIStr_SellRoomDesc, 0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_room_button,          0, GUIStr_Empty,        0,       {0},               0, maintain_big_room },
  {LbBtnT_NormalBtn,  BID_ROOM_NXPG, 0, 1, NULL,               NULL,        NULL,               0,  78, 188,  78, 188, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_Empty,&room_menu2,{0},    0, maintain_room_next_page_button },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                 0,       {0},               0, NULL },
};

struct GuiButtonInit room_menu2_buttons[] = {
  {LbBtnT_NormalBtn,  BID_ROOM_TD17, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,238,  6, 242, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,       {2},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD18, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,238, 38, 242, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty, 0,      {14},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD19, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,238, 70, 242, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,      {13},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD20, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,238,102, 242, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,       {6},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD21, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,276,  6, 280, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,       {3},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD22, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,276, 38, 280, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty, 0,      {15},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD23, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,276, 70, 280, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty, 0,      {16},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD24, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,276,102, 280, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,       {8},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD25, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,314,  6, 318, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,       {4},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD26, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,314, 38, 318, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,       {5},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD27, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,314, 70, 318, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,      {12},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD28, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,314,102, 318, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty, 0,      {10},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD29, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,352,  6, 356, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,  0,      {11},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD30, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,352, 38, 356, 32, 36, gui_area_room_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty, 0,       {9},               0, maintain_room },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,        0,       {0},               0, maintain_room },
  {LbBtnT_NormalBtn,  BID_ROOM_TD31, 0, 0, gui_remove_area_for_rooms,NULL,NULL,                 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button, GPS_rpanel_frame_portrt_sell, GUIStr_SellRoomDesc, 0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_room_button,          0, GUIStr_Empty,        0,       {0},               0, maintain_big_room },
  {LbBtnT_NormalBtn,  BID_ROOM_NXPG, 0, 1, NULL,               NULL,        NULL,               0,  78, 188,  78, 188, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_Empty,&room_menu,{0},    0, maintain_room_next_page_button },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                  0,       {0},               0, NULL },
};

struct GuiButtonInit spell_menu_buttons[] = {
  {LbBtnT_NormalBtn, BID_POWER_TD16, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 238,   6, 242, 32, 36, gui_area_spell_button, GPS_keepower_possess_std_s, CpgStr_PowerDesc1+0,  0,      {18},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD01, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 238,  38, 242, 32, 36, gui_area_spell_button, GPS_keepower_imp_std_s, CpgStr_PowerDesc1+1,  0,       {2},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD02, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 238,  70, 242, 32, 36, gui_area_spell_button, GPS_keepower_sight_std_s, CpgStr_PowerDesc1+2,  0,       {5},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD07, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 238, 102, 242, 32, 36, gui_area_spell_button, GPS_keepower_speed_std_s, CpgStr_PowerDesc1+7,  0,      {11},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD15, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 276,   6, 280, 32, 36, gui_area_spell_button, GPS_crspell_whip_std_s, CpgStr_PowerDesc1+6,  0,       {3},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD03, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 276,  38, 280, 32, 36, gui_area_spell_button, GPS_keepower_cta_std_s, CpgStr_PowerDesc1+3,  0,       {6},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD09, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 276,  70, 280, 32, 36, gui_area_spell_button, GPS_keepower_conceal_std_s, CpgStr_PowerDesc1+9,  0,      {13},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD14, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  98, 276, 102, 280, 32, 36, gui_area_spell_button, GPS_keepower_holdaud_std_s, CpgStr_PowerDesc1+4,  0,       {9},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD04, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 314,   6, 318, 32, 36, gui_area_spell_button, GPS_keepower_cavein_std_s, CpgStr_PowerDesc1+5,  0,       {7},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD06, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 314,  38, 318, 32, 36, gui_area_spell_button, GPS_keepower_heal_std_s, CpgStr_PowerDesc1+14, 0,       {8},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD05, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 314,  70, 318, 32, 36, gui_area_spell_button, GPS_keepower_lightng_std_s, CpgStr_PowerDesc1+10, 0,      {10},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD08, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 314, 102, 318, 32, 36, gui_area_spell_button, GPS_keepower_armor_std_s, CpgStr_PowerDesc1+8,  0,      {12},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD10, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 352,   6, 356, 32, 36, gui_area_spell_button, GPS_keepower_chicken_std_s, CpgStr_PowerDesc1+11, 0,      {15},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD11, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 352,  38, 356, 32, 36, gui_area_spell_button, GPS_keepower_disease_std_s, CpgStr_PowerDesc1+12, 0,      {14},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD13, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_spell_button, GPS_keepower_armagedn_std_s, CpgStr_PowerDesc1+16, 0,      {19},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD12, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 352, 102, 356, 32, 36, gui_area_spell_button, GPS_keepower_dstwall_std_s, CpgStr_PowerDesc1+13, 0,      {16},               0, maintain_spell },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_spell_button,         0, GUIStr_Empty,         0,       {0},               0, maintain_big_spell },
  {LbBtnT_NormalBtn, BID_POWER_NXPG, 0, 1, NULL,               NULL,        NULL,               0,  78, 188,  78, 188, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_Empty,&spell_menu2,{0},    0, maintain_spell_next_page_button },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                  0,       {0},               0, NULL },
};

struct GuiButtonInit spell_menu2_buttons[] = {
  {LbBtnT_NormalBtn, BID_POWER_TD17, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 238,   6, 242, 32, 36, gui_area_spell_button, GPS_keepower_timebomb_std_s, CpgStr_PowerDesc1+15,  0,      {18},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD18, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 238,  38, 242, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,       {2},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD19, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 238,  70, 242, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,       {5},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD20, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 238, 102, 242, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,      {11},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD21, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 276,   6, 280, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,       {3},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD22, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 276,  38, 280, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,       {6},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD23, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 276,  70, 280, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,      {13},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD24, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,               0,  98, 276, 102, 280, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,       {9},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD25, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 314,   6, 318, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,       {7},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD26, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 314,  38, 318, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty, 0,       {8},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD27, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 314,  70, 318, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty, 0,      {10},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD28, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 314, 102, 318, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty,  0,      {12},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD29, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 352,   6, 356, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty, 0,      {15},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD30, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 352,  38, 356, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty, 0,      {14},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD31, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty, 0,      {19},               0, maintain_spell },
  {LbBtnT_NormalBtn, BID_POWER_TD32, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 352, 102, 356, 32, 36, gui_area_spell_button, GPS_rpanel_frame_portrt_empty, CpgStr_Empty, 0,      {16},               0, maintain_spell },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_spell_button,         0, GUIStr_Empty,         0,       {0},               0, maintain_big_spell },
  {LbBtnT_NormalBtn, BID_POWER_NXPG, 0, 1, NULL,               NULL,        NULL,               0,  78, 188,  78, 188, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_Empty,&spell_menu,{0},    0, maintain_spell_next_page_button },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                  0,       {0},               0, NULL },
};

struct GuiButtonInit spell_lost_menu_buttons[] = {
  {LbBtnT_NormalBtn, BID_POWER_TD16, 0, 0, spell_lost_first_person,NULL,    NULL,               0,   2, 238,   8, 250, 32, 36, gui_area_new_null_button, GPS_keepower_possess_std_s, CpgStr_PowerDesc1+0,  0,      {18},               0, maintain_spell },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 238,  40, 250, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 238,  72, 250, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 238, 104, 250, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   2, 276,   8, 288, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 276,  40, 288, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  72, 288, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 104, 288, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   2, 314,   8, 326, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 314,  40, 326, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 314,  72, 326, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 314, 104, 326, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   8, 364, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  40, 364, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  72, 364, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,  98, 352, 104, 364, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_spell_button,         0, GUIStr_Empty,         0,       {0},               0, NULL },
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                  0,       {0},               0, NULL },
};

struct GuiButtonInit trap_menu_buttons[] = {
  {LbBtnT_NormalBtn, BID_MNFCT_TD02, 0, 0, NULL,                      NULL, NULL, 0,   2, 238,   6, 242, 32, 36, NULL, GPS_trapdoor_trap_alarm_std_s, CpgStr_AlarmTrapDesc,                     0,       {2},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD03, 0, 0, NULL,                      NULL, NULL, 0,  34, 238,  38, 242, 32, 36, NULL, GPS_trapdoor_trap_gas_std_s, CpgStr_PoisonGasTrapDesc,                   0,       {3},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD04, 0, 0, NULL,                      NULL, NULL, 0,  66, 238,  70, 242, 32, 36, NULL, GPS_trapdoor_trap_lightning_std_s, CpgStr_LightningTrapDesc,             0,       {4},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD15, 0, 0, NULL,                      NULL, NULL, 0,  98, 238, 102, 242, 32, 36, NULL, GPS_trapdoor_trap_lava_std_s, CpgStr_LavaTrapDesc,                       0,       {6},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD01, 0, 0, NULL,                      NULL, NULL, 0,   2, 276,   6, 280, 32, 36, NULL, GPS_trapdoor_trap_boulder_std_s, CpgStr_TrapBoulderDesc,                 0,       {1},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD05, 0, 0, NULL,                      NULL, NULL, 0,  34, 276,  38, 280, 32, 36, NULL, GPS_trapdoor_trap_wop_std_s, CpgStr_WordOfPowerTrapDesc,                 0,       {5},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD11, 0, 0, NULL,                      NULL, NULL, 0,  66, 276,  70, 280, 32, 36, NULL, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,                             0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD12, 0, 0, NULL,                      NULL, NULL, 0,  98, 276, 102, 280, 32, 36, NULL, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,                             0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD06, 0, 0, NULL,                      NULL, NULL, 0,   2, 314,   6, 318, 32, 36, NULL, GPS_trapdoor_door_wood_std_s, CpgStr_WoodenDoorDesc,                     0,       {7},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD07, 0, 0, NULL,                      NULL, NULL, 0,  34, 314,  38, 318, 32, 36, NULL, GPS_trapdoor_door_braced_std_s, CpgStr_BracedDoorDesc,                   0,       {8},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD08, 0, 0, NULL,                      NULL, NULL, 0,  66, 314,  70, 318, 32, 36, NULL, GPS_trapdoor_door_iron_std_s, CpgStr_IronDoorDesc,                       0,       {9},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD09, 0, 0, NULL,                      NULL, NULL, 0,  98, 314, 102, 318, 32, 36, NULL, GPS_trapdoor_door_magic_std_s, CpgStr_MagicDoorDesc,                     0,      {10},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD13, 0, 0, NULL,                      NULL, NULL, 0,   2, 352,   6, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD14, 0, 0, NULL,                      NULL, NULL, 0,  34, 352,  38, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD16, 0, 0, NULL,                      NULL, NULL, 0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,         0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD10, 0, 0, gui_remove_area_for_traps, NULL, NULL, 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button, GPS_rpanel_frame_portrt_sell, GUIStr_SellItemDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0,   8, 210,   8, 194,126, 44, gui_area_big_trap_button,          0, GUIStr_Empty,                            0,       {0},               0, maintain_big_trap },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0, 110, 215, 110, 216, 16, 20, gui_area_trap_build_info_button,   0, GUIStr_Empty,                            0,       {0},               0, maintain_buildable_info },
  {LbBtnT_NormalBtn, BID_MNFCT_NXPG, 0, 1, NULL,                      NULL, NULL, 0,  78, 188,  78, 188, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_Empty,&trap_menu2, {0},               0, maintain_trap_next_page_button },
  {              -1,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                                     0,       {0},               0, NULL },
};

struct GuiButtonInit trap_menu2_buttons[] = {
  {LbBtnT_NormalBtn, BID_MNFCT_TD17, 0, 0, NULL,                      NULL, NULL, 0,   2, 238,   6, 242, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {2},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD18, 0, 0, NULL,                      NULL, NULL, 0,  34, 238,  38, 242, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {3},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD19, 0, 0, NULL,                      NULL, NULL, 0,  66, 238,  70, 242, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {4},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD20, 0, 0, NULL,                      NULL, NULL, 0,  98, 238, 102, 242, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {6},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD21, 0, 0, NULL,                      NULL, NULL, 0,   2, 276,   6, 280, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {1},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD22, 0, 0, NULL,                      NULL, NULL, 0,  34, 276,  38, 280, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {5},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD23, 0, 0, NULL,                      NULL, NULL, 0,  66, 276,  70, 280, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD24, 0, 0, NULL,                      NULL, NULL, 0,  98, 276, 102, 280, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD25, 0, 0, NULL,                      NULL, NULL, 0,   2, 314,   6, 318, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {7},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD26, 0, 0, NULL,                      NULL, NULL, 0,  34, 314,  38, 318, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {8},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD27, 0, 0, NULL,                      NULL, NULL, 0,  66, 314,  70, 318, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {9},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD28, 0, 0, NULL,                      NULL, NULL, 0,  98, 314, 102, 318, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,      {10},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD29, 0, 0, NULL,                      NULL, NULL, 0,   2, 352,   6, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD30, 0, 0, NULL,                      NULL, NULL, 0,  34, 352,  38, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD31, 0, 0, NULL,                      NULL, NULL, 0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button, GPS_rpanel_frame_portrt_empty, GUIStr_Empty,               0,       {0},               0, NULL },
  {LbBtnT_NormalBtn, BID_MNFCT_TD32, 0, 0, gui_remove_area_for_traps, NULL, NULL, 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button, GPS_rpanel_frame_portrt_sell, GUIStr_SellItemDesc,      0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0,   8, 210,   8, 194,126, 44, gui_area_big_trap_button,          0, GUIStr_Empty,                                  0,       {0},               0,    maintain_big_trap },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0, 110, 215, 110, 216, 16, 20, gui_area_trap_build_info_button,   0, GUIStr_Empty,                                  0,       {0},               0,    maintain_buildable_info },
  {LbBtnT_NormalBtn, BID_MNFCT_NXPG, 0, 1, NULL,                      NULL, NULL, 0,  78, 188,  78, 188, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_Empty, &trap_menu,       {0},               0, maintain_trap_next_page_button },
  {              -1,    BID_DEFAULT, 0, 0, NULL,                      NULL, NULL, 0,   0,   0,   0,   0,  0,  0,                    NULL,           0,                              0,                0,       {0},               0, NULL },
};

struct GuiButtonInit creature_menu_buttons[] = {
  {LbBtnT_NormalBtn,BID_CRTR_NXWNDR, 0, 0, pick_up_next_wanderer,gui_go_to_next_wanderer    ,NULL,  0,  26, 192,  26, 192, 38, 24, gui_area_new_normal_button, GPS_rpanel_tab_crtr_wandr_act, GUIStr_CreatureIdleDesc,       0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_CRTR_NXWRKR, 0, 0, pick_up_next_worker,gui_go_to_next_worker        ,NULL,  0,  62, 192,  62, 192, 38, 24, gui_area_new_normal_button, GPS_rpanel_tab_crtr_work_act, GUIStr_CreatureWorkingDesc,    0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,BID_CRTR_NXFIGT, 0, 0, pick_up_next_fighter,gui_go_to_next_fighter      ,NULL,  0,  98, 192,  98, 192, 38, 24, gui_area_new_normal_button, GPS_rpanel_tab_crtr_fight_act, GUIStr_CreatureFightingDesc,   0,       {0},            0, NULL },
  {LbBtnT_HoldableBtn,  BID_DEFAULT, 0, 0, gui_scroll_activity_up,gui_scroll_activity_up    ,NULL,  0,   4, 192,   4, 192, 22, 24, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_up_act, GUIStr_Empty,                  0,       {0},            0, maintain_activity_up },
  {LbBtnT_HoldableBtn,  BID_DEFAULT, 0, 0, gui_scroll_activity_down,gui_scroll_activity_down,NULL,  0,   4, 364,   4, 364, 22, 24, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_down_act, GUIStr_Empty,                  0,       {0},            0, maintain_activity_down },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,gui_over_creature_button,  0,   0, 196,   0, 218, 22, 22, gui_area_creatrmodel_button,            0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,26,220,26,220,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrIdleDesc,     0,{.lptr = &activity_list[0+0]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,62,220,62,220,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrWorkingDesc,  0,{.lptr = &activity_list[0+1]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,98,220,98,220,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrFightingDesc, 0,{.lptr = &activity_list[0+2]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,gui_over_creature_button,  1,   0, 220,   0, 242, 22, 22, gui_area_creatrmodel_button,            0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,26,244,26,244,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrIdleDesc,     0,{.lptr = &activity_list[4+0]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,62,244,62,244,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrWorkingDesc,  0,{.lptr = &activity_list[4+1]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,98,244,98,244,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrFightingDesc, 0,{.lptr = &activity_list[4+2]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,gui_over_creature_button,  2,   0, 244,   0, 266, 22, 22, gui_area_creatrmodel_button,            0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,26,268,26,268,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrIdleDesc,     0,{.lptr = &activity_list[8]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,62,268,62,268,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrWorkingDesc,  0,{.lptr = &activity_list[9]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,98,268,98,268,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrFightingDesc, 0,{.lptr = &activity_list[10]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,gui_over_creature_button,  3,   0, 268,   0, 290, 22, 22, gui_area_creatrmodel_button,            0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,26,292,26,292,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrIdleDesc,     0,{.lptr = &activity_list[12]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,62,292,62,292,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrWorkingDesc,  0,{.lptr = &activity_list[13]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,98,292,98,292,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrFightingDesc, 0,{.lptr = &activity_list[14]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,gui_over_creature_button,  4,   0, 292,   0, 314, 22, 22, gui_area_creatrmodel_button,            0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,26,316,26,316,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrIdleDesc,     0,{.lptr = &activity_list[16]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,62,316,62,316,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrWorkingDesc,  0,{.lptr = &activity_list[17]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,98,316,98,316,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrFightingDesc, 0,{.lptr = &activity_list[18]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_next_creature,gui_go_to_next_creature,gui_over_creature_button,  5,   0, 314,   0, 338, 22, 22, gui_area_creatrmodel_button,            0, GUIStr_PickCreatrMostExpDesc,  0,       {0},            0, maintain_activity_pic },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,26,340,26,340,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrIdleDesc,     0,{.lptr = &activity_list[20]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,62,340,62,340,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrWorkingDesc,  0,{.lptr = &activity_list[21]},0,maintain_activity_row },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,98,340,98,340,32,20,gui_area_anger_button, GPS_rpanel_tab_crtr_annoy_lv00, GUIStr_PickCreatrFightingDesc, 0,{.lptr = &activity_list[22]},0,maintain_activity_row },
  {              -1,    BID_DEFAULT, 0, 0,                            NULL,                            NULL,NULL,0, 0,  0, 0,  0, 0, 0,                 NULL,                              0,                             0, 0,       {0},            0, NULL },
};

struct GuiButtonInit query_menu_buttons[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, gui_set_query,                NULL, NULL, 0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,    GPS_rpanel_rpanel_btn_crinfo_act,      GUIStr_GoToQueryMode,        0,       {0},            0, maintain_query_button },
  {LbBtnT_NormalBtn,    BID_QUERY_2,      0, 0, gui_switch_players_visible,   NULL, NULL, 0,  14, 374,  14, 374, 52, 20, gui_area_new_normal_button,    GPS_rpanel_rpanel_btn_nxpage_act,      GUIStr_MoreInformation,      0,       {0},            0, maintain_player_page2 },
  {LbBtnT_ToggleBtn,    BID_QRY_IMPRSN,   0, 0, gui_set_tend_to, NULL, NULL, 1,  36, 190,  36, 190, 32, 26, gui_area_flash_cycle_button,   GPS_rpanel_tendency_prisne_act,        GUIStr_CreatureImprisonDesc, 0,{.ptr = &game.creatures_tend_imprison}, 1, maintain_prison_bar },
  {LbBtnT_ToggleBtn,    BID_QRY_FLEE,     0, 0, gui_set_tend_to, NULL, NULL, 2,  74, 190,  74, 190, 32, 26, gui_area_flash_cycle_button,   GPS_rpanel_tendency_fleee_act,         GUIStr_CreatureFleeDesc,     0,{.ptr = &game.creatures_tend_flee}, 1, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, NULL,            NULL, NULL, 0,   4, 216,   4, 222,132, 24, gui_area_payday_button,        GPS_rpanel_rpanel_payday_counter,      GUIStr_PayTimeDesc,          0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, NULL,            NULL, NULL, 0,   2, 246,   2, 246, 60, 24, gui_area_research_bar,         GPS_room_research_std_s,               GUIStr_ResearchTimeDesc,     0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, NULL,            NULL, NULL, 0,  74, 246,  74, 246, 60, 24, gui_area_workshop_bar,         GPS_room_workshop_std_s,               GUIStr_WorkshopTimeDesc,     0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,  74, 274,  74, 274, 60, 24, gui_area_player_creature_info, GPS_plyrsym_symbol_player_red_std_a,   GUIStr_NumberOfCreaturesDesc,0,       {0},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,  74, 298,  74, 298, 60, 24, gui_area_player_creature_info, GPS_plyrsym_symbol_player_blue_std_a,  GUIStr_NumberOfCreaturesDesc,0,       {1},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,  74, 322,  74, 322, 60, 24, gui_area_player_creature_info, GPS_plyrsym_symbol_player_green_std_a, GUIStr_NumberOfCreaturesDesc,0,       {2},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,  74, 346,  74, 346, 60, 24, gui_area_player_creature_info, GPS_plyrsym_symbol_player_yellow_std_a,GUIStr_NumberOfCreaturesDesc,0,       {3},            0, maintain_creature_button },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,   4, 274,   4, 274, 60, 24, gui_area_player_room_info,     GPS_plyrsym_symbol_room_red_std_a,     GUIStr_NumberOfRoomsDesc,    0,       {0},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,   4, 298,   4, 298, 60, 24, gui_area_player_room_info,     GPS_plyrsym_symbol_room_blue_std_a,    GUIStr_NumberOfRoomsDesc,    0,       {1},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,   4, 322,   4, 322, 60, 24, gui_area_player_room_info,     GPS_plyrsym_symbol_room_green_std_a,   GUIStr_NumberOfRoomsDesc,    0,       {2},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DUNGEON_INFO, 0, 0, NULL,            NULL, NULL, 0,   4, 346,   4, 346, 60, 24, gui_area_player_room_info,     GPS_plyrsym_symbol_room_yellow_std_a,  GUIStr_NumberOfRoomsDesc,    0,       {3},            0, maintain_room_button },
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, gui_toggle_ally, NULL, NULL, 0,  62, 274,  62, 274, 14, 22, gui_area_ally,                 0,                                     GUIStr_AllyWithPlayer,       0,       {0},            0, maintain_ally },
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, gui_toggle_ally, NULL, NULL, 0,  62, 298,  62, 298, 14, 22, gui_area_ally,                 0,                                     GUIStr_AllyWithPlayer,       0,       {1},            0, maintain_ally },
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, gui_toggle_ally, NULL, NULL, 0,  62, 322,  62, 322, 14, 22, gui_area_ally,                 0,                                     GUIStr_AllyWithPlayer,       0,       {2},            0, maintain_ally },
  {LbBtnT_NormalBtn,    BID_DEFAULT,      0, 0, gui_toggle_ally, NULL, NULL, 0,  62, 346,  62, 346, 14, 22, gui_area_ally,                 0,                                     GUIStr_AllyWithPlayer,       0,       {3},            0, maintain_ally },
  {              -1,    BID_DEFAULT,      0, 0, NULL,            NULL, NULL, 0,   0,   0,   0,   0,  0,  0, NULL,                          0,                                     0,                           0,       {0},            0, NULL },
};    

struct GuiButtonInit event_menu_buttons[] = {
  {              -1,    BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                         0,       {0},            0, NULL },
};

struct GuiButtonInit creature_query_buttons1[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,                                NULL,                                               NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_MoreInformation,&creature_query_menu2,{0},    0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button, GPS_rpanel_bar_rounded_empty, GUIStr_CreatureAngerDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button, GPS_rpanel_bar_rounded_full, GUIStr_ExperienceDesc,   0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, gui_query_next_creature_of_owner,    gui_query_next_creature_of_owner_and_model,         NULL,               0,   4, 266,   4, 266,126, 14, NULL,                              0, GUIStr_NameAndHealthDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {0},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {1},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {2},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {3},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {4},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,                                NULL,                                               NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {5},               0, maintain_instance },
  {              -1,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},               0, NULL },
};

struct GuiButtonInit creature_query_buttons2[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,                                NULL,                                               NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_MoreInformation,&creature_query_menu3,{0},    0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button, GPS_rpanel_bar_rounded_empty, GUIStr_CreatureAngerDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button, GPS_rpanel_bar_rounded_full, GUIStr_ExperienceDesc,   0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, gui_query_next_creature_of_owner,    gui_query_next_creature_of_owner_and_model,         NULL,               0,   4, 266,   4, 266,126, 14, NULL,                              0, GUIStr_NameAndHealthDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {4},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {5},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {6},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {7},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {8},               0, maintain_instance },
  {LbBtnT_NormalBtn,    BID_DEFAULT, 0, 1, NULL,                                NULL,                                               NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,          0, GUIStr_Empty,            0,       {9},               0, maintain_instance },
  {              -1,    BID_DEFAULT, 0, 0, NULL,                                NULL,                                               NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},               0, NULL },
};

struct GuiButtonInit creature_query_buttons3[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT,    0, 1, NULL,                                NULL,                                               NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_MoreInformation,&creature_query_menu4,          {0}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT,    0, 0, gui_query_next_creature_of_owner,    gui_query_next_creature_of_owner_and_model,         NULL,               0,   4, 204,   4, 204,126, 14, NULL,                              0, GUIStr_NameAndHealthDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 226,   4, 226, 60, 24, gui_area_stat_button, GPS_crspell_heal_std_s, GUIStr_CreatureHealthDesc,     0,         {CrLStat_Health}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 226,  72, 226, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_strength_std, GUIStr_CreatureStrengthDesc,   0,       {CrLStat_Strength}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 256,   4, 256, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_armor_std, GUIStr_CreatureArmourDesc,     0,         {CrLStat_Armour}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 256,  72, 256, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_defense_std, GUIStr_CreatureDefenceDesc,    0,        {CrLStat_Defence}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 286,   4, 286, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_luck_std, GUIStr_CreatureLuckDesc,       0,           {CrLStat_Luck}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 286,  72, 286, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_dexterity_std, GUIStr_CreatureDexterityDesc,  0,      {CrLStat_Dexterity}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 316,   4, 316, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_wage_std, GUIStr_CreatureWageDesc,       0,       {CrLStat_GoldWage}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 316,  72, 316, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_gold_std, GUIStr_CreatureGoldHeldDesc,   0,       {CrLStat_GoldHeld}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_age_std, GUIStr_CreatureTimeInDungeonDesc,0,      {CrLStat_AgeTime}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_kills_std, GUIStr_CreatureKillsDesc,      0,          {CrLStat_Kills}, 0, NULL },
  {              -1,    BID_DEFAULT,    0, 0, NULL,                                NULL,                                               NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                           0,                      {0}, 0, NULL },
};

struct GuiButtonInit creature_query_buttons4[] = {
  {LbBtnT_NormalBtn,    BID_DEFAULT,    0, 1, NULL,                                NULL,                                               NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button, GPS_rpanel_rpanel_btn_nxpage_act, GUIStr_MoreInformation,&creature_query_menu1,           {0}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_DEFAULT,    0, 0, gui_query_next_creature_of_owner,    gui_query_next_creature_of_owner_and_model,         NULL,               0,   4, 204,   4, 204,126, 14, NULL,                              0, GUIStr_NameAndHealthDesc,0,       {0},               0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 226,   4, 226, 60, 24, gui_area_stat_button, GPS_crspell_speedup_dis_s, GUIStr_CreatureSpeedDesc,       0,          {CrLStat_Speed}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 226,  72, 226, 60, 24, gui_area_stat_button, GPS_crspell_whip_std_s, GUIStr_CreatureLoyaltyDesc,     0,        {CrLStat_Loyalty}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 256,   4, 256, 60, 24, gui_area_stat_button, GPS_room_research_std_s, GUIStr_CreatureResrchSkillDesc,  0,  {CrLStat_ResearchSkill}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 256,  72, 256, 60, 24, gui_area_stat_button, GPS_room_workshop_std_s, GUIStr_CreatureManfctrSkillDesc, 0,{CrLStat_ManufactureSkill},0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 286,   4, 286, 60, 24, gui_area_stat_button, GPS_room_training_std_s, GUIStr_CreatureTraingSkillDesc,  0,  {CrLStat_TrainingSkill}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 286,  72, 286, 60, 24, gui_area_stat_button, GPS_room_scavenge_std_s, GUIStr_CreatureScavngSkillDesc,  0,  {CrLStat_ScavengeSkill}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 316,   4, 316, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_traingcst_std, GUIStr_CreatureTraingCostDesc,  0,   {CrLStat_TrainingCost}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 316,  72, 316, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_scavngcst_std, GUIStr_CreatureScavngCostDesc,  0,   {CrLStat_ScavengeCost}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_stat_button, GPS_trapdoor_trap_boulder_std_s, GUIStr_CreatureWeightDesc,      0,         {CrLStat_Weight}, 0, NULL },
  {LbBtnT_NormalBtn,    BID_QUERY_INFO, 0, 0, NULL,                                NULL,                                               NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_stat_button, GPS_symbols_creatr_stat_blood_std, GUIStr_CreatureBloodTypeDesc,   0,      {CrLStat_BloodType}, 0, NULL },
  {              -1,    BID_DEFAULT,    0, 0, NULL,                                NULL,                                               NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                            0,                      {0}, 0, NULL },
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
struct GuiMenu spell_menu2 =
 {          GMnu_SPELL2, 0, 1, spell_menu2_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
 struct GuiMenu room_menu2 =
 {          GMnu_ROOM2, 0, 1, room_menu2_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
 struct GuiMenu trap_menu2 =
 {          GMnu_TRAP2, 0, 1, trap_menu2_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};

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
