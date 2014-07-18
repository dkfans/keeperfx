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
#include "bflib_sprfnt.h"

#include "player_data.h"
#include "player_states.h"
#include "dungeon_data.h"
#include "creature_battle.h"
#include "creature_graphics.h"
#include "config_creature.h"
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
DLLIMPORT void _DK_draw_battle_head(struct Thing *thing, long scr_x, long scr_y);
DLLIMPORT void _DK_draw_bonus_timer(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void gui_open_event(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    unsigned int evbtn_idx;
    unsigned int evnt_idx;
    SYNCDBG(5,"Starting");
    evbtn_idx = (unsigned long)gbtn->content;
    if (evbtn_idx <= EVENT_BUTTONS_COUNT) {
        evnt_idx = dungeon->event_button_index[evbtn_idx];
    } else {
        evnt_idx = 0;
    }
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
    //_DK_gui_kill_event(gbtn);
    struct PlayerInfo *player;
    player = get_my_player();
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    unsigned long i;
    i = (unsigned long)gbtn->content;
    set_players_packet_action(player, PckA_Unknown092, dungeon->event_button_index[i], 0, 0, 0);
}

void turn_on_event_info_panel_if_necessary(unsigned short evnt_idx)
{
    struct Event *event;
    event = &game.event[evnt_idx];
    if ((event->kind == EvKind_FriendlyFight) || (event->kind == EvKind_EnemyFight))
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
    set_players_packet_action(player, PckA_EventBoxActivate, evnt_idx, 0,0,0);
}

void gui_previous_battle(struct GuiButton *gbtn)
{
    //_DK_gui_previous_battle(gbtn); return;
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    BattleIndex battle_idx;
    battle_idx = dungeon->visible_battles[0];
    if (battle_idx != 0)
    {
        battle_idx = find_previous_battle_of_mine_excluding_current_list(dungeon->owner, battle_idx);
        if (battle_idx > 0)
        {
            dungeon->visible_battles[0] = battle_idx;
            battle_idx = find_next_battle_of_mine_excluding_current_list(dungeon->owner, battle_idx);
            dungeon->visible_battles[1] = battle_idx;
            battle_idx = find_next_battle_of_mine_excluding_current_list(dungeon->owner, battle_idx);
            dungeon->visible_battles[2] = battle_idx;
        }
    }
}

void gui_next_battle(struct GuiButton *gbtn)
{
    //_DK_gui_next_battle(gbtn); return;
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    BattleIndex battle_idx;
    battle_idx = dungeon->visible_battles[2];
    if (battle_idx != 0)
    {
        battle_idx = find_next_battle_of_mine_excluding_current_list(dungeon->owner, battle_idx);
        if (battle_idx > 0)
        {
            dungeon->visible_battles[2] = battle_idx;
            battle_idx = find_previous_battle_of_mine_excluding_current_list(dungeon->owner, battle_idx);
            dungeon->visible_battles[1] = battle_idx;
            battle_idx = find_previous_battle_of_mine_excluding_current_list(dungeon->owner, battle_idx);
            dungeon->visible_battles[0] = battle_idx;
        }
    }
}

void gui_get_creature_in_battle(struct GuiButton *gbtn)
{
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    if (battle_creature_over <= 0) {
        return;
    }
    //_DK_gui_get_creature_in_battle(gbtn); return;
    PowerKind pwkind;
    pwkind = 0;
    if (myplyr->work_state < PLAYER_STATES_COUNT)
        pwkind = player_state_to_power_kind[myplyr->work_state];
    struct Thing *thing;
    thing = thing_get(battle_creature_over);
    if (!thing_exists(thing)) {
        WARNLOG("Nonexisting thing %d in battle",(int)battle_creature_over);
        battle_creature_over = 0;
        return;
    }
    TRACE_THING(thing);
    // If a spell is selected, try to cast it
    if (pwkind > 0)
    {
        if (can_cast_spell(my_player_number, pwkind, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default)) {
            struct Packet *pckt;
            pckt = get_packet(my_player_number);
            set_packet_action(pckt, PckA_PwrUseOnThing, pwkind, battle_creature_over, 0, 0);
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
    //_DK_gui_go_to_person_in_battle(gbtn);
    struct Thing *thing;
    thing = thing_get(battle_creature_over);
    if (thing_exists(thing))
    {
        struct Packet *pckt;
        pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_Unknown087, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
    }
}

void gui_setup_friend_over(struct GuiButton *gbtn)
{
    //_DK_gui_setup_friend_over(gbtn);
    int visbtl_id;
    visbtl_id = gbtn->field_1B;
    if (battle_creature_over == 0)
    {
        struct Dungeon *dungeon;
        dungeon = get_my_dungeon();
        struct Thing *thing;
        thing = INVALID_THING;
        if (dungeon->visible_battles[visbtl_id] != 0)
        {
            int battlr_id;
            battlr_id = (gbtn->scr_pos_x - lbDisplay.MMouseX * pixel_size) / (gbtn->width / 7) + 6;
            if (battlr_id < MESSAGE_BATTLERS_COUNT-1) {
                thing = thing_get(friendly_battler_list[battlr_id + MESSAGE_BATTLERS_COUNT * visbtl_id]);
            }
        }
        if (!thing_is_invalid(thing) && thing_revealed(thing, dungeon->owner))
        {
            battle_creature_over = thing->index;
        }
    }
}

void draw_battle_head(struct Thing *thing, long scr_x, long scr_y)
{
    //_DK_draw_battle_head(thing, a2, a3); return;
    if (thing_is_invalid(thing)) {
        return;
    }
    unsigned long spr_idx;
    spr_idx = get_creature_model_graphics(thing->model, CGI_GUIPanelSymbol);
    struct TbSprite *spr;
    spr = &gui_panel_sprites[spr_idx];
    int curscr_x, curscr_y;
    curscr_x = scr_x - ((pixel_size * spr->SWidth) >> 1);
    curscr_y = scr_y - ((pixel_size * spr->SHeight) >> 1);
    if ((thing->creature.health_bar_turns) && ((game.play_gameturn & 1) != 0)) {
        LbSpriteDrawOneColour(curscr_x / pixel_size, curscr_y / pixel_size, spr, player_flash_colours[thing->owner]);
    } else {
        LbSpriteDraw(curscr_x / pixel_size, curscr_y / pixel_size, spr);
    }
    curscr_x = scr_x - 8;
    curscr_y = scr_y + ((pixel_size * spr->SHeight) >> 1) - 8;
    LbDrawBox(curscr_x / pixel_size, curscr_y / pixel_size, 16 / pixel_size, 6 / pixel_size, colours[0][0][0]);
    // Show health
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    long health,max_health;
    health = thing->health;
    max_health = compute_creature_max_health(crstat->health,cctrl->explevel);
    if (health <= 0)
        health = 0;
    LbDrawBox((curscr_x + 2) / pixel_size, (curscr_y + 2) / pixel_size, ((12 * health)/max_health) / pixel_size, 2 / pixel_size, player_room_colours[thing->owner]);
    // Draw experience level
    spr = &button_sprite[184];
    curscr_y = (scr_y - ((pixel_size * spr->SHeight) >> 1));
    curscr_x = (scr_x - ((pixel_size * spr->SWidth) >> 1));
    LbSpriteDraw(curscr_x / pixel_size, curscr_y / pixel_size, &button_sprite[184 + cctrl->explevel]);
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
                  lbDisplay.DrawFlags |= (Lb_SPRITE_UNKNOWN0010|0x0004);
                  LbDrawBox(scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
                    wdelta/pixel_size, gbtn->height/pixel_size, col);
                  lbDisplay.DrawFlags &= ~(Lb_SPRITE_UNKNOWN0010|0x0004);
              }
            }
            scr_pos_x -= wdelta;
        }
    }
}

void gui_setup_enemy_over(struct GuiButton *gbtn)
{
    //_DK_gui_setup_enemy_over(gbtn); return;
    int visbtl_id;
    visbtl_id = gbtn->field_1B;
    if (battle_creature_over == 0)
    {
        struct Dungeon *dungeon;
        dungeon = get_my_dungeon();
        struct Thing *thing;
        thing = INVALID_THING;
        if (dungeon->visible_battles[visbtl_id] != 0)
        {
            int battlr_id;
            battlr_id = (lbDisplay.MMouseX * pixel_size - gbtn->scr_pos_x) / (gbtn->width / 7);
            if (battlr_id < MESSAGE_BATTLERS_COUNT-1) {
                thing = thing_get(enemy_battler_list[battlr_id + MESSAGE_BATTLERS_COUNT * visbtl_id]);
            }
        }
        if (!thing_is_invalid(thing) && thing_revealed(thing, dungeon->owner))
        {
            battle_creature_over = thing->index;
        }
    }
}

void gui_area_enemy_battlers(struct GuiButton *gbtn)
{
    //_DK_gui_area_enemy_battlers(gbtn); return;
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
    scr_pos_x = gbtn->scr_pos_x;
    lbDisplay.DrawFlags |= 0x0004;
    LbDrawBox(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
        gbtn->width/pixel_size, gbtn->height/pixel_size, colours[0][0][0]);
    lbDisplay.DrawFlags &= ~0x0004;
    int i,n;
    for (n=0; n < 7; n++)
    {
        struct Thing *thing;
        i = enemy_battler_list[n + MESSAGE_BATTLERS_COUNT*gbtn->field_1B];
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
                  lbDisplay.DrawFlags |= (Lb_SPRITE_UNKNOWN0010|0x0004);
                  LbDrawBox(scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
                    wdelta/pixel_size, gbtn->height/pixel_size, col);
                  lbDisplay.DrawFlags &= ~(Lb_SPRITE_UNKNOWN0010|0x0004);
              }
            }
            scr_pos_x += wdelta;
        }
    }
}

short zoom_to_fight(unsigned char a1)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    if (active_battle_exists(a1))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        set_players_packet_action(player, PckA_Unknown104, dungeon->visible_battles[0], 0, 0, 0);
        step_battles_forward(a1);
        return true;
    }
    return false;
}

void draw_bonus_timer(void)
{
    //_DK_draw_bonus_timer(); return;
    int nturns;
    nturns = game.bonus_time - game.play_gameturn;
    if (nturns < 0) {
        nturns = 0;
    } else
    if (nturns > 99999) {
        nturns = 99999;
    }
    LbTextSetFont(winfont);
    char * text;
    text = buf_sprintf("%05d", nturns/2);
    long width, height;
    width = 10 * pixel_size * LbTextCharWidth('0');
    height = pixel_size * LbTextStringHeight(text) + pixel_size * LbTextStringHeight(text) / 2;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    long scr_x, scr_y;
    scr_x = MyScreenWidth - width - 16;
    scr_y = 16;
    LbTextSetWindow(scr_x/pixel_size, scr_y/pixel_size, width/pixel_size, height/pixel_size);
    draw_slab64k(scr_x, scr_y, width, height);
    LbTextDraw(0/pixel_size, 0/pixel_size, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

/**
 * Returns if there is a bonus timer visible on the level.
 */
TbBool bonus_timer_enabled(void)
{
  return ((game.flags_gui & GGUI_CountdownTimer) != 0);
}
/******************************************************************************/
