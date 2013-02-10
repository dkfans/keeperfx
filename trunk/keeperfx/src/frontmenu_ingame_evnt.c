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
#include "game_legacy.h"

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
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void gui_open_event(struct GuiButton *gbtn)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(my_player_number);
  unsigned int idx;
  unsigned int evnt_idx;
  SYNCDBG(5,"Starting");
  idx = (unsigned long)gbtn->content;
  if (idx < 121) //size of the field_13A7 array (I can't be completely sure of it)
    evnt_idx = dungeon->field_13A7[idx];
  else
    evnt_idx = 0;
  if (evnt_idx == dungeon->visible_event_idx)
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
  set_players_packet_action(player, PckA_Unknown115, evnt_idx, 0,0,0);
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

short zoom_to_fight(unsigned char a1)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    if (active_battle_exists(a1))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        set_players_packet_action(player, 104, dungeon->visible_battles[0], 0, 0, 0);
        step_battles_forward(a1);
        return true;
    }
    return false;
}
/******************************************************************************/
