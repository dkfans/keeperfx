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
#include "player_utils.h"
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
#include "front_input.h"
#include "vidfade.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

unsigned long TimerTurns = 0;

/******************************************************************************/
void gui_open_event(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    EventIndex evidx;
    SYNCDBG(5,"Starting");
    unsigned int evbtn_idx = (unsigned long)gbtn->content;
    if (evbtn_idx <= EVENT_BUTTONS_COUNT) {
        evidx = dungeon->event_button_index[evbtn_idx];
    } else {
        evidx = 0;
    }
    if (evidx == dungeon->visible_event_idx)
    {
        gui_close_objective(gbtn);
    } else
    if (evidx != 0)
    {
        activate_event_box(evidx);
    }
}

void gui_kill_event(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    struct Dungeon* dungeon = get_players_dungeon(player);
    unsigned long i = (unsigned long)gbtn->content;
    set_players_packet_action(player, PckA_Unknown092, dungeon->event_button_index[i], 0, 0, 0);
}

void turn_on_event_info_panel_if_necessary(EventIndex evidx)
{
    struct Event* event = &game.event[evidx];
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

void activate_event_box(EventIndex evidx)
{
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_EventBoxActivate, evidx, 0,0,0);
}

void gui_previous_battle(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    BattleIndex battle_id = dungeon->visible_battles[0];
    if (battle_id != 0)
    {
        battle_id = find_previous_battle_of_mine_excluding_current_list(dungeon->owner, battle_id);
        if (battle_id > 0)
        {
            dungeon->visible_battles[0] = battle_id;
            battle_id = find_next_battle_of_mine_excluding_current_list(dungeon->owner, battle_id);
            dungeon->visible_battles[1] = battle_id;
            battle_id = find_next_battle_of_mine_excluding_current_list(dungeon->owner, battle_id);
            dungeon->visible_battles[2] = battle_id;
        }
    }
}

void gui_next_battle(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    BattleIndex battle_id = dungeon->visible_battles[2];
    if (battle_id != 0)
    {
        battle_id = find_next_battle_of_mine_excluding_current_list(dungeon->owner, battle_id);
        if (battle_id > 0)
        {
            dungeon->visible_battles[2] = battle_id;
            battle_id = find_previous_battle_of_mine_excluding_current_list(dungeon->owner, battle_id);
            dungeon->visible_battles[1] = battle_id;
            battle_id = find_previous_battle_of_mine_excluding_current_list(dungeon->owner, battle_id);
            dungeon->visible_battles[0] = battle_id;
        }
    }
}

void gui_get_creature_in_battle(struct GuiButton *gbtn)
{
    struct PlayerInfo* myplyr = get_my_player();
    if (battle_creature_over <= 0) {
        return;
    }
    PowerKind pwkind = 0;
    if (myplyr->work_state < PLAYER_STATES_COUNT)
        pwkind = player_state_to_power_kind[myplyr->work_state];
    struct Thing* thing = thing_get(battle_creature_over);
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
            struct Packet* pckt = get_packet(my_player_number);
            set_packet_action(pckt, PckA_UsePwrOnThing, pwkind, battle_creature_over, 0, 0);
        }
    } else
    {
        if (can_cast_spell(my_player_number, PwrK_HAND, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default)) {
            struct Packet* pckt = get_packet(my_player_number);
            set_packet_action(pckt, PckA_UsePwrHandPick, battle_creature_over, 0, 0, 0);
        }
    }
    battle_creature_over = 0;
}

void gui_go_to_person_in_battle(struct GuiButton *gbtn)
{
    struct Thing* thing = thing_get(battle_creature_over);
    if (thing_exists(thing))
    {
        struct Packet* pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_ZoomToPosition, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
    }
}

void gui_setup_friend_over(struct GuiButton *gbtn)
{
    int visbtl_id = gbtn->btype_value & LbBFeF_IntValueMask;
    if (battle_creature_over == 0)
    {
        struct Dungeon* dungeon = get_my_dungeon();
        struct Thing* thing = INVALID_THING;
        if (dungeon->visible_battles[visbtl_id] != 0)
        {
            int battlr_id = (gbtn->scr_pos_x - lbDisplay.MMouseX * pixel_size) / (gbtn->width / 7) + 6;
            if (battlr_id < MESSAGE_BATTLERS_COUNT-1) {
                thing = thing_get(friendly_battler_list[MESSAGE_BATTLERS_COUNT * visbtl_id + battlr_id]);
            }
        }
        if (!thing_is_invalid(thing) && thing_revealed(thing, dungeon->owner))
        {
            battle_creature_over = thing->index;
        }
    }
}

void draw_battle_head(struct Thing *thing, long scr_x, long scr_y, int units_per_px)
{
    if (thing_is_invalid(thing)) {
        return;
    }
    short spr_idx = get_creature_model_graphics(thing->model, CGI_HandSymbol);
    struct TbSprite* spr = &gui_panel_sprites[spr_idx];
    int ps_units_per_px = (50 * units_per_px + spr->SHeight / 2) / spr->SHeight;
    int curscr_x = scr_x - (spr->SWidth * ps_units_per_px / 16) / 2;
    int curscr_y = scr_y - (spr->SHeight * ps_units_per_px / 16) / 2;
    if ((thing->creature.health_bar_turns) && ((game.play_gameturn & 1) != 0)) {
        LbSpriteDrawResizedOneColour(curscr_x, curscr_y, ps_units_per_px, spr, player_flash_colours[thing->owner]);
    } else {
        LbSpriteDrawResized(curscr_x, curscr_y, ps_units_per_px, spr);
    }
    curscr_x = scr_x - 8*units_per_px/16;
    curscr_y = scr_y - 8*units_per_px/16 + (spr->SHeight*ps_units_per_px/16)/2;
    LbDrawBox(curscr_x, curscr_y, 16*units_per_px/16, 6*units_per_px/16, colours[0][0][0]);
    // Show health
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    HitPoints health = thing->health;
    if (health < 0)
        health = 0;
    HitPoints max_health = cctrl->max_health;
    if (max_health < 1)
        max_health = 1;
    LbDrawBox(curscr_x + 2*units_per_px/16, curscr_y + 2*units_per_px/16, ((12 * health)/max_health)*units_per_px/16, 2*units_per_px/16, player_room_colours[thing->owner]);
    // Draw experience level
    spr = &button_sprite[184];
    int bs_units_per_px = (17 * units_per_px + spr->SHeight / 2) / spr->SHeight;
    curscr_y = (scr_y - ((spr->SHeight*bs_units_per_px/16) >> 1));
    curscr_x = (scr_x - ((spr->SWidth*bs_units_per_px/16) >> 1));
    spr = &button_sprite[184 + cctrl->explevel];
    LbSpriteDrawResized(curscr_x, curscr_y, bs_units_per_px, spr);
}

void gui_area_friendly_battlers(struct GuiButton *gbtn)
{
    int visbtl_id = gbtn->btype_value & LbBFeF_IntValueMask;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    BattleIndex battle_id = dungeon->visible_battles[visbtl_id];
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle)) {
        return;
    }
    if (battle->fighters_num <= 0) {
        return;
    }
    int units_per_px = (gbtn->width * 16 + 160 / 2) / 160;
    int wdelta = gbtn->width / 7;
    int scr_pos_x = gbtn->scr_pos_x - wdelta + gbtn->width;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y,
        gbtn->width, gbtn->height, colours[0][0][0]);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    for (int battlr_id = 0; battlr_id < MESSAGE_BATTLERS_COUNT; battlr_id++)
    {
        int i = friendly_battler_list[MESSAGE_BATTLERS_COUNT * visbtl_id + battlr_id];
        struct Thing* thing = thing_get(i);
        if (thing_is_creature(thing))
        {
            draw_battle_head(thing, scr_pos_x + wdelta / 2, gbtn->scr_pos_y, units_per_px);
            if (thing->index == battle_creature_over)
            {
              if (game.play_gameturn & 2)
              {
                  TbPixel col = player_flash_colours[game.play_gameturn & 3];
                  lbDisplay.DrawFlags |= (Lb_SPRITE_OUTLINE|0x0004);
                  LbDrawBox(scr_pos_x, gbtn->scr_pos_y,
                    wdelta, gbtn->height, col);
                  lbDisplay.DrawFlags &= ~(Lb_SPRITE_OUTLINE|0x0004);
              }
            }
            scr_pos_x -= wdelta;
        }
    }
}

void gui_setup_enemy_over(struct GuiButton *gbtn)
{
    int visbtl_id = gbtn->btype_value & LbBFeF_IntValueMask;
    if (battle_creature_over == 0)
    {
        struct Dungeon* dungeon = get_my_dungeon();
        struct Thing* thing = INVALID_THING;
        if (dungeon->visible_battles[visbtl_id] != 0)
        {
            int battlr_id = (lbDisplay.MMouseX * pixel_size - gbtn->scr_pos_x) / (gbtn->width / 7);
            if (battlr_id < MESSAGE_BATTLERS_COUNT-1) {
                thing = thing_get(enemy_battler_list[MESSAGE_BATTLERS_COUNT * visbtl_id + battlr_id]);
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
    int visbtl_id = gbtn->btype_value & LbBFeF_IntValueMask;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    BattleIndex battle_id = dungeon->visible_battles[visbtl_id];
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle)) {
        return;
    }
    if (battle->fighters_num <= 0) {
        return;
    }
    int units_per_px = (gbtn->width * 16 + 160 / 2) / 160;
    int wdelta = gbtn->width / 7;
    int scr_pos_x = gbtn->scr_pos_x;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y,
        gbtn->width, gbtn->height, colours[0][0][0]);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    for (int battlr_id = 0; battlr_id < MESSAGE_BATTLERS_COUNT; battlr_id++)
    {
        int i = enemy_battler_list[MESSAGE_BATTLERS_COUNT * visbtl_id + battlr_id];
        struct Thing* thing = thing_get(i);
        if (thing_is_creature(thing))
        {
            draw_battle_head(thing, scr_pos_x + wdelta / 2, gbtn->scr_pos_y, units_per_px);
            if (thing->index == battle_creature_over)
            {
              if (game.play_gameturn & 2)
              {
                  TbPixel col = player_flash_colours[game.play_gameturn & 3];
                  lbDisplay.DrawFlags |= (Lb_SPRITE_OUTLINE|0x0004);
                  LbDrawBox(scr_pos_x, gbtn->scr_pos_y,
                    wdelta, gbtn->height, col);
                  lbDisplay.DrawFlags &= ~(Lb_SPRITE_OUTLINE|0x0004);
              }
            }
            scr_pos_x += wdelta;
        }
    }
}

short zoom_to_fight(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_my_player();
    if (active_battle_exists(plyr_idx))
    {
        struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
        set_players_packet_action(player, PckA_Unknown104, dungeon->visible_battles[0], 0, 0, 0);
        step_battles_forward(plyr_idx);
        return true;
    }
    return false;
}

void draw_bonus_timer(void)
{
    int nturns = game.bonus_time - game.play_gameturn;
    char* text;
    if (gameadd.timer_real)
    {
        unsigned long total_seconds = ((nturns) / game.num_fps) + 1;
        unsigned char seconds = total_seconds % 60;
        unsigned long total_minutes = total_seconds / 60;
        unsigned char minutes = total_minutes % 60;
        unsigned char hours = total_minutes / 60;
        text = (nturns >= 0) ? buf_sprintf("%02d:%02d:%02d", hours, minutes, seconds) : buf_sprintf("00:00:00");
    }
    else
    {
        if (nturns < 0)
        {
            nturns = 0;
        } 
        else if (nturns > 99999)
        {
            nturns = 99999;
        }
        text = buf_sprintf("%05d", nturns / 2);
    }
    LbTextSetFont(winfont);
    long width = 10 * (LbTextCharWidth('0') * units_per_pixel / 16);
    long height = LbTextLineHeight() * units_per_pixel / 16 + (LbTextLineHeight() * units_per_pixel / 16) / 2;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    long scr_x = MyScreenWidth - width - 16 * units_per_pixel / 16;
    long scr_y = 16 * units_per_pixel / 16;
    if (game.armageddon_cast_turn != 0)
    {
        struct GuiMenu *gmnu = get_active_menu(menu_id_to_number(GMnu_MAIN));
        scr_x = (gmnu->width + (width >> 1) - 16 * units_per_pixel / 16);
    }
    LbTextSetWindow(scr_x, scr_y, width, height);
    draw_slab64k(scr_x, scr_y, units_per_pixel, width, height);
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    LbTextDrawResized(0, 0, tx_units_per_px, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

/**
 * Returns if there is a bonus timer visible on the level.
 */
TbBool bonus_timer_enabled(void)
{
  return ((game.flags_gui & GGUI_CountdownTimer) != 0);
}

void draw_timer(void)
{
    char* text;
    if (TimerGame)
    {
        if (get_my_player()->victory_state != VicS_WonLevel)
        {
            TimerTurns = game.play_gameturn;
        }
        text = buf_sprintf("%08ld", TimerTurns);
    }
    else
    {
        if (!TimerFreeze)
        {
            update_time();
        }
        text = buf_sprintf("%02d:%02d:%02d", Timer.Hours, Timer.Minutes, Timer.Seconds);
    }
    LbTextSetFont(winfont); 
    long width = 10 * (LbTextCharWidth('0') * units_per_pixel >> 4);
    long height = LbTextLineHeight() * units_per_pixel / 16 + (LbTextLineHeight() * units_per_pixel / 16) / 2;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    long scr_x = MyScreenWidth - width - 16 * units_per_pixel / 16;
    long scr_y = 16 * units_per_pixel / 16;
    if ( (bonus_timer_enabled()) || (script_timer_enabled()) || (display_variable_enabled()) || (game.armageddon_cast_turn != 0) )
    {
        scr_y <<= 2;
    }
    LbTextSetWindow(scr_x, scr_y, width, height);
    draw_slab64k(scr_x, scr_y, units_per_pixel, width, height);
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    LbTextDrawResized(0, 0, tx_units_per_px, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

TbBool timer_enabled(void)
{
  return ((game_flags2 & GF2_Timer) != 0);
}

TbBool script_timer_enabled(void)
{
  return ((game.flags_gui & GGUI_ScriptTimer) != 0);
}

void draw_script_timer(PlayerNumber plyr_idx, unsigned char timer_id, unsigned long limit, TbBool real)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    int nturns = (limit > 0) ? limit - (game.play_gameturn - dungeon->turn_timers[timer_id].count) : game.play_gameturn - dungeon->turn_timers[timer_id].count;
    if (nturns < 0)
    {
        game.flags_gui &= ~GGUI_ScriptTimer;
        return;
    }
    char* text;
    if (real)
    {
        unsigned long total_seconds = ((nturns) / game.num_fps) + 1;
        unsigned char seconds = total_seconds % 60;
        unsigned long total_minutes = total_seconds / 60;
        unsigned char minutes = total_minutes % 60;
        unsigned char hours = total_minutes / 60;
        text = (nturns >= 0) ? buf_sprintf("%02d:%02d:%02d", hours, minutes, seconds) : buf_sprintf("00:00:00");
    }
    else
    {
        text = buf_sprintf("%08d", nturns);
    }
    LbTextSetFont(winfont);
    long width = 10 * (LbTextCharWidth('0') * units_per_pixel / 16);
    long height = LbTextLineHeight() * units_per_pixel / 16 + (LbTextLineHeight() * units_per_pixel / 16) / 2;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    long scr_x = MyScreenWidth - width - 16 * units_per_pixel / 16;
    long scr_y = 16 * units_per_pixel / 16;
    if (game.armageddon_cast_turn != 0)
    {
        struct GuiMenu *gmnu = get_active_menu(menu_id_to_number(GMnu_MAIN));
        scr_x = (gmnu->width + (width >> 1) - 16 * units_per_pixel / 16);
    }
    LbTextSetWindow(scr_x, scr_y, width, height);
    draw_slab64k(scr_x, scr_y, units_per_pixel, width, height);
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    LbTextDrawResized(0, 0, tx_units_per_px, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

TbBool display_variable_enabled(void)
{
  return ((game.flags_gui & GGUI_Variable) != 0);
}

void draw_script_variable(PlayerNumber plyr_idx, unsigned char valtype, unsigned char validx, long target, unsigned char targettype)
{
    long value = get_condition_value(plyr_idx, valtype, validx);
    if (target != 0)
    {
        if ( (targettype == 0) || (targettype == 2) )
        {
            value = target - value;
        }
        else if (targettype == 1)
        {
            value = ((~target)+1) + value; 
        }
    }
    if (targettype != 2)
    {
        if (value < 0)
        {
            value = 0;
        }
    }
    char* text = buf_sprintf("%ld", value);
    LbTextSetFont(winfont);
    long width = 10 * (LbTextCharWidth('0') * units_per_pixel / 16);
    long height = LbTextLineHeight() * units_per_pixel / 16 + (LbTextLineHeight() * units_per_pixel / 16) / 2;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    long scr_x = MyScreenWidth - width - 16 * units_per_pixel / 16;
    long scr_y = 16 * units_per_pixel / 16;
    if (game.armageddon_cast_turn != 0)
    {
        struct GuiMenu *gmnu = get_active_menu(menu_id_to_number(GMnu_MAIN));
        scr_x = (gmnu->width + (width >> 1) - 16 * units_per_pixel / 16);
        if ( (bonus_timer_enabled()) || (script_timer_enabled()) )
        {
            scr_x += ((width + (width >> 1)) - 16 * units_per_pixel / 16);
        }
    }
    else if ( (bonus_timer_enabled()) || (script_timer_enabled()) )
    {
        scr_x -= ((width + (width >> 1)) - 16 * units_per_pixel / 16);
    }
    LbTextSetWindow(scr_x, scr_y, width, height);
    draw_slab64k(scr_x, scr_y, units_per_pixel, width, height);
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    LbTextDrawResized(0, 0, tx_units_per_px, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}
/******************************************************************************/
