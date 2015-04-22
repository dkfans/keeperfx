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

/******************************************************************************/
void gui_open_event(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    unsigned int evbtn_idx;
    EventIndex evidx;
    SYNCDBG(5,"Starting");
    evbtn_idx = (unsigned long)gbtn->content;
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
    struct PlayerInfo *player;
    player = get_my_player();
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    unsigned long i;
    i = (unsigned long)gbtn->content;
    set_players_packet_action(player, PckA_Unknown092, dungeon->event_button_index[i], 0);
}

void turn_on_event_info_panel_if_necessary(EventIndex evidx)
{
    struct Event *event;
    event = &game.event[evidx];
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
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_EventBoxActivate, evidx, 0);
}

void gui_previous_battle(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    BattleIndex battle_id;
    battle_id = dungeon->visible_battles[0];
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
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    BattleIndex battle_id;
    battle_id = dungeon->visible_battles[2];
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
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    if (battle_creature_over <= 0) {
        return;
    }
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
            set_packet_action(pckt, PckA_UsePwrOnThing, pwkind, battle_creature_over);
        }
    } else
    {
        if (can_cast_spell(my_player_number, PwrK_HAND, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default)) {
            struct Packet *pckt;
            pckt = get_packet(my_player_number);
            set_packet_action(pckt, PckA_UsePwrHandPick, battle_creature_over, 0);
        }
    }
    battle_creature_over = 0;
}

void gui_go_to_person_in_battle(struct GuiButton *gbtn)
{
    struct Thing *thing;
    thing = thing_get(battle_creature_over);
    if (thing_exists(thing))
    {
        struct Packet *pckt;
        pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_Unknown087, thing->mappos.x.val, thing->mappos.y.val);
    }
}

void gui_setup_friend_over(struct GuiButton *gbtn)
{
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

void draw_battle_head(struct Thing *thing, long scr_x, long scr_y, int units_per_px)
{
    if (thing_is_invalid(thing)) {
        return;
    }
    unsigned long spr_idx;
    spr_idx = get_creature_model_graphics(thing->model, CGI_HandSymbol);
    struct TbSprite *spr;
    spr = &gui_panel_sprites[spr_idx];
    int ps_units_per_px;
    ps_units_per_px = (50 * units_per_px + spr->SHeight/2) / spr->SHeight;
    int curscr_x, curscr_y;
    curscr_x = scr_x - (spr->SWidth*ps_units_per_px/16)/2;
    curscr_y = scr_y - (spr->SHeight*ps_units_per_px/16)/2;
    if ((thing->creature.health_bar_turns) && ((game.play_gameturn & 1) != 0)) {
        LbSpriteDrawResizedOneColour(curscr_x, curscr_y, ps_units_per_px, spr, player_flash_colours[thing->owner]);
    } else {
        LbSpriteDrawResized(curscr_x, curscr_y, ps_units_per_px, spr);
    }
    curscr_x = scr_x - 8*units_per_px/16;
    curscr_y = scr_y - 8*units_per_px/16 + (spr->SHeight*ps_units_per_px/16)/2;
    LbDrawBox(curscr_x, curscr_y, 16*units_per_px/16, 6*units_per_px/16, colours[0][0][0]);
    // Show health
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    HitPoints health,max_health;
    health = thing->health;
    if (health < 0)
        health = 0;
    max_health = cctrl->max_health;
    if (max_health < 1)
        max_health = 1;
    LbDrawBox(curscr_x + 2*units_per_px/16, curscr_y + 2*units_per_px/16, ((12 * health)/max_health)*units_per_px/16, 2*units_per_px/16, player_room_colours[thing->owner]);
    // Draw experience level
    spr = &button_sprite[184];
    int bs_units_per_px;
    bs_units_per_px = (17 * units_per_px + spr->SHeight/2) / spr->SHeight;
    curscr_y = (scr_y - ((spr->SHeight*bs_units_per_px/16) >> 1));
    curscr_x = (scr_x - ((spr->SWidth*bs_units_per_px/16) >> 1));
    spr = &button_sprite[184 + cctrl->explevel];
    LbSpriteDrawResized(curscr_x, curscr_y, bs_units_per_px, spr);
}

void gui_area_friendly_battlers(struct GuiButton *gbtn)
{
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
    int units_per_px;
    units_per_px = (gbtn->width * 16 + 160/2) / 160;
    int scr_pos_x, wdelta;
    wdelta = gbtn->width / 7;
    scr_pos_x = gbtn->scr_pos_x - wdelta + gbtn->width;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y,
        gbtn->width, gbtn->height, colours[0][0][0]);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    int i,n;
    for (n=0; n < 7; n++)
    {
        struct Thing *thing;
        i = friendly_battler_list[n + MESSAGE_BATTLERS_COUNT*gbtn->field_1B];
        thing = thing_get(i);
        if (thing_is_creature(thing))
        {
            draw_battle_head(thing, scr_pos_x + wdelta / 2, gbtn->scr_pos_y, units_per_px);
            if (thing->index == battle_creature_over)
            {
              if (game.play_gameturn & 2)
              {
                  TbPixel col;
                  col = player_flash_colours[game.play_gameturn & 3];
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
    int units_per_px;
    units_per_px = (gbtn->width * 16 + 160/2) / 160;
    int scr_pos_x, wdelta;
    wdelta = gbtn->width / 7;
    scr_pos_x = gbtn->scr_pos_x;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y,
        gbtn->width, gbtn->height, colours[0][0][0]);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    int i,n;
    for (n=0; n < 7; n++)
    {
        struct Thing *thing;
        i = enemy_battler_list[n + MESSAGE_BATTLERS_COUNT*gbtn->field_1B];
        thing = thing_get(i);
        if (thing_is_creature(thing))
        {
            draw_battle_head(thing, scr_pos_x + wdelta / 2, gbtn->scr_pos_y, units_per_px);
            if (thing->index == battle_creature_over)
            {
              if (game.play_gameturn & 2)
              {
                  TbPixel col;
                  col = player_flash_colours[game.play_gameturn & 3];
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
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    if (active_battle_exists(plyr_idx))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        set_players_packet_action(player, PckA_Unknown104, dungeon->visible_battles[0], 0);
        step_battles_forward(plyr_idx);
        return true;
    }
    return false;
}

void draw_bonus_timer(void)
{
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
    width = 10 * (LbTextCharWidth('0')*units_per_pixel/16);
    height = LbTextLineHeight()*units_per_pixel/16 + (LbTextLineHeight()*units_per_pixel/16) / 2;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    long scr_x, scr_y;
    scr_x = MyScreenWidth - width - 16*units_per_pixel/16;
    scr_y = 16*units_per_pixel/16;
    LbTextSetWindow(scr_x, scr_y, width, height);
    draw_slab64k(scr_x, scr_y, units_per_pixel, width, height);
    int tx_units_per_px;
    tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
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
/******************************************************************************/
