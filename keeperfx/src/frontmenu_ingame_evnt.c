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
#include "bflib_vidraw.h"

#include "player_data.h"
#include "player_states.h"
#include "dungeon_data.h"
#include "creature_battle.h"
#include "magic.h"
#include "gui_draw.h"
#include "gui_frontbtns.h"
#include "gui_frontmenu.h"
#include "packets.h"
#include "frontend.h"
#include "vidfade.h"
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
DLLIMPORT void _DK_draw_battle_head(struct Thing *thing, long a2, long a3);
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
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    if (battle_creature_over <= 0) {
        return;
    }
    //_DK_gui_get_creature_in_battle(gbtn);
    PowerKind pwmodel;
    pwmodel = 0;
    if (myplyr->work_state < PLAYER_STATES_COUNT)
        pwmodel = player_state_to_spell[myplyr->work_state];
    struct Thing *thing;
    thing = thing_get(battle_creature_over);
    if (!thing_exists(thing)) {
        WARNLOG("Nonexisting thing in battle");
        battle_creature_over = 0;
        return;
    }
    TRACE_THING(thing);
    // If a spell is selected, try to cast it
    if (pwmodel > 0)
    {
        if (can_cast_spell(my_player_number, pwmodel, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default)) {
            struct Packet *pckt;
            pckt = get_packet(my_player_number);
            set_packet_action(pckt, PckA_PwrUseOnThing, pwmodel, battle_creature_over, 0, 0);
        }
    } else
    {
        if (can_cast_spell(my_player_number, PwrK_HAND, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default)) {
            struct Packet *pckt;
            pckt = get_packet(my_player_number);
            set_packet_action(pckt, PckA_PickUpThing, battle_creature_over, 0, 0, 0);
        }
    }
    battle_creature_over = 0;
}

void gui_go_to_person_in_battle(struct GuiButton *gbtn)
{
  _DK_gui_go_to_person_in_battle(gbtn);
}

void gui_setup_friend_over(struct GuiButton *gbtn)
{
  _DK_gui_setup_friend_over(gbtn);
}

void draw_battle_head(struct Thing *thing, long a2, long a3)
{
    _DK_draw_battle_head(thing, a2, a3); return;
}

void gui_area_friendly_battlers(struct GuiButton *gbtn)
{
    //_DK_gui_area_friendly_battlers(gbtn);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    BattleIndex battle_id;
    battle_id = dungeon->visible_battles[gbtn->field_1B];
    struct CreatureBattle *battle;
    battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle)) {
        return;
    }
    if (battle->fighters_num <= 0) {
        return;
    }
    int scr_pos_x, wdelta;
    wdelta = gbtn->width / 7;
    scr_pos_x = gbtn->scr_pos_x - wdelta + gbtn->width;
    lbDisplay.DrawFlags |= 0x0004;
    LbDrawBox(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
        gbtn->width/pixel_size, gbtn->height/pixel_size, colours[0][0][0]);
    lbDisplay.DrawFlags &= ~0x0004;
    int i,n;
    for (n=0; n < 7; n++)
    {
        struct Thing *thing;
        i = friendly_battler_list[n + MESSAGE_BATTLERS_COUNT*gbtn->field_1B];
        thing = thing_get(i);
        if (thing_is_creature(thing))
        {
            draw_battle_head(thing, scr_pos_x + wdelta / 2, gbtn->scr_pos_y);
            if (thing->index == battle_creature_over)
            {
              if (game.play_gameturn & 2)
              {
                  TbPixel col;
                  col = player_flash_colours[game.play_gameturn & 3];
                  lbDisplay.DrawFlags |= (0x0010|0x0004);
                  LbDrawBox(scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
                    wdelta/pixel_size, gbtn->height/pixel_size, col);
                  lbDisplay.DrawFlags &= ~(0x0010|0x0004);
              }
            }
            scr_pos_x -= wdelta;
        }
    }
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
