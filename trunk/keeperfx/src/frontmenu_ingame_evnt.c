/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_evnt.c
 *     In-game events GUI, visible during gameplay at bottom.
 * @par Purpose:
 *     Functions to show and maintain message menu appearing ingame.
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
#include "frontmenu_ingame_evnt.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "gui_draw.h"
#include "gui_frontbtns.h"
#include "gui_frontmenu.h"
#include "packets.h"
#include "frontend.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_gui_open_event(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_kill_event(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_previous_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_next_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_get_creature_in_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_person_in_battle(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_setup_friend_over(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_friendly_battlers(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_setup_enemy_over(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_enemy_battlers(struct GuiButton *gbtn);
/******************************************************************************/
void gui_previous_battle(struct GuiButton *gbtn);
void gui_next_battle(struct GuiButton *gbtn);
void gui_get_creature_in_battle(struct GuiButton *gbtn);
void gui_go_to_person_in_battle(struct GuiButton *gbtn);
void gui_setup_friend_over(struct GuiButton *gbtn);
void gui_area_friendly_battlers(struct GuiButton *gbtn);
void gui_setup_enemy_over(struct GuiButton *gbtn);
void gui_area_enemy_battlers(struct GuiButton *gbtn);
/******************************************************************************/

struct GuiButtonInit text_info_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,   4, 999,   4,400, 78, gui_area_scroll_window,            0, 201,  0,{(long)&game.evntbox_scroll_window},0,0, NULL },
  { 1, 63, 0, 0, 0, gui_go_to_event,    NULL,        NULL,               0,   4,   4,   4,   4, 30, 24, gui_area_new_normal_button,      276, 466,  0,       {0},             0,0, maintain_zoom_to_event },
  { 0, 64, 0, 0, 1, gui_close_objective,gui_close_objective,NULL,        0,   4,  56,   4,  56, 30, 24, gui_area_new_normal_button,      274, 465,  0,       {0},             0,0, NULL },
  { 1, 66, 0, 0, 0, gui_scroll_text_up, NULL,        NULL,               0, 446,   4, 446,   4, 30, 24, gui_area_new_normal_button,      486, 201,  0,{(long)&game.evntbox_scroll_window},0,0, maintain_scroll_up },
  { 1, 65, 0, 0, 0, gui_scroll_text_down,NULL,       NULL,               0, 446,  56, 446,  56, 30, 24, gui_area_new_normal_button,      272, 201,  0,{(long)&game.evntbox_scroll_window},0,0, maintain_scroll_down },
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


struct GuiMenu text_info_menu =
// { 16, 0, 4, text_info_buttons,                        160, 316, 480,  86, gui_round_glass_background,  0, NULL,    reset_scroll_window,     0, 0, 0,};
 { 16, 0, 4, text_info_buttons,                  160, POS_SCRBTM,480,  86, gui_round_glass_background,  0, NULL,    reset_scroll_window,     0, 0, 0,};
struct GuiMenu battle_menu =
// { 34, 0, 4, battle_buttons,                    160,        300, 480, 102, gui_round_glass_background,  0, NULL,    NULL,                    0, 0, 0,};
 { 34, 0, 4, battle_buttons,                    160, POS_SCRBTM, 480, 102, gui_round_glass_background,  0, NULL,    NULL,                    0, 0, 0,};
/******************************************************************************/

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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
