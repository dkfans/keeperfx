/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_msgs.c
 *     Game GUI Messages functions.
 * @par Purpose:
 *     Functions to display and maintain GUI Messages.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     14 May 2010 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "gui_msgs.h"
#include <stdarg.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"
#include "config_spritecolors.h"
#include "creature_graphics.h"
#include "creature_instances.h"
#include "gui_draw.h"
#include "frontend.h"
#include "game_legacy.h"
#include "frontmenu_ingame_evnt.h"
#include "sprites.h"

#include "keeperfx.hpp"
#include "post_inc.h"

/******************************************************************************/

void message_draw(void)
{
    SYNCDBG(7,"Starting");
    LbTextSetFont(winfont);
    int ps_units_per_px;
    {
        //just used for height, color irrelevant here
        struct TbSprite* spr = &gui_panel_sprites[GPS_plyrsym_symbol_player_red_std_b];
        ps_units_per_px = (22 * units_per_pixel) / spr->SHeight;
    }
    TbBool low_res = (MyScreenHeight < 400);
    int tx_units_per_px = ( (low_res) && (dbc_language > 0) ) ? ps_units_per_px : (22 * units_per_pixel) / LbTextLineHeight();
    int h = LbTextLineHeight();
    long y = 28 * units_per_pixel / 16;
    if (game.armageddon_cast_turn != 0)
    {
        if ( (bonus_timer_enabled()) || (script_timer_enabled()) || display_variable_enabled() )
        {
            y += (h*units_per_pixel/16) << (unsigned char)low_res;
        }
    }
    for (int i = 0; i < game.active_messages_count; i++)
    {
        if ( (gameadd.messages[i].target_idx == my_player_number) || (gameadd.messages[i].target_idx == -1) )
        {
            long x = 148 * units_per_pixel / 16;
            LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
            clear_flag(lbDisplay.DrawFlags, Lb_TEXT_ONE_COLOR);
            LbTextDrawResized(x+32*units_per_pixel/16, y, tx_units_per_px, gameadd.messages[i].text);
            unsigned long spr_idx = 0;
            TbBool IsCreature = false;
            TbBool IsCreatureSpell = false;
            TbBool IsRoom = false;
            TbBool IsKeeperSpell = false;
            TbBool IsQuery = false;
            TbBool NotPlayer = ((char)gameadd.messages[i].plyr_idx < 0);
            PlayerNumber plyr_idx = gameadd.messages[i].plyr_idx;
            if (NotPlayer)
            {
                IsCreature = ( ((char)gameadd.messages[i].plyr_idx >= -31) && ((char)gameadd.messages[i].plyr_idx <= -1) );
                IsCreatureSpell = ((char)gameadd.messages[i].plyr_idx >= -78) && ((char)gameadd.messages[i].plyr_idx <= -32);
                IsRoom = ((char)gameadd.messages[i].plyr_idx >= -94) && ((char)gameadd.messages[i].plyr_idx <= -79);
                IsKeeperSpell = ((char)gameadd.messages[i].plyr_idx >= -113) && ((char)gameadd.messages[i].plyr_idx <= -95);
                IsQuery = ((char)gameadd.messages[i].plyr_idx >= -123) && ((char)gameadd.messages[i].plyr_idx <= -114);
                if (IsCreature)
                {
                    spr_idx = get_creature_model_graphics(((~gameadd.messages[i].plyr_idx) + 1), CGI_HandSymbol);
                    x -= (7 * units_per_pixel / 16);
                    y -= (20 * units_per_pixel / 16);
                }
                else if (IsCreatureSpell)
                {
                    struct InstanceInfo* inst_inf = creature_instance_info_get(~(char)(((char)gameadd.messages[i].plyr_idx) + 31) + 1);
                    spr_idx = inst_inf->symbol_spridx;
                    x -= (10 * units_per_pixel / 16);
                    y -= (10 * units_per_pixel / 16);
                }
                else if (IsRoom)
                {
                    const struct RoomConfigStats* roomst = get_room_kind_stats(~(char)(((char)gameadd.messages[i].plyr_idx) + 78) + 1);
                    spr_idx = roomst->medsym_sprite_idx;
                    x -= (10 * units_per_pixel / 16);
                    y -= (10 * units_per_pixel / 16);
                }
                else if (IsKeeperSpell)
                {
                    struct PowerConfigStats* powerst = get_power_model_stats(~(char)(((char)gameadd.messages[i].plyr_idx) + 94) + 1);
                    spr_idx = powerst->medsym_sprite_idx;
                    x -= (10 * units_per_pixel / 16);
                    y -= (10 * units_per_pixel / 16);
                }
                else if (IsQuery)
                {
                    spr_idx = (~(char)(((char)gameadd.messages[i].plyr_idx) + 113) + 1) + GPS_plyrsym_symbol_room_yellow_std_a;
                    x -= (10 * units_per_pixel / 16);
                    y -= (10 * units_per_pixel / 16);
                }
            }
            else
            {
                if (gameadd.messages[i].plyr_idx == game.hero_player_num)
                {
                    spr_idx = GPS_plyrsym_symbol_player_red_std_b;
                }
                else if (gameadd.messages[i].plyr_idx == game.neutral_player_num)
                {
                    spr_idx = ((game.play_gameturn >> 1) & 3) + GPS_plyrsym_symbol_player_red_std_b;
                    plyr_idx = 0;
                }
                else
                {
                    spr_idx = (player_has_heart(gameadd.messages[i].plyr_idx)) ? GPS_plyrsym_symbol_player_red_std_b : GPS_plyrsym_symbol_player_red_dead;
                }
            }
            if (gameadd.messages[i].plyr_idx != 127)
            {
                if (plyr_idx >= 0)
                {
                    draw_gui_panel_sprite_left_player(x, y, ps_units_per_px, spr_idx, plyr_idx);
                }
                else
                {
                    struct TbSprite* spr = &gui_panel_sprites[spr_idx];
                    LbSpriteDrawResized(x, y, ps_units_per_px, spr);
                }
            }
            y += (h*units_per_pixel/16) << (unsigned char)low_res;
            if (NotPlayer)
            {
                if (IsCreature)
                {
                    y += (20 * units_per_pixel / 16) << (unsigned char)low_res;
                }
                else if ( (IsCreatureSpell) || (IsRoom) || (IsKeeperSpell) || (IsQuery) )
                {
                    y += (10 * units_per_pixel / 16) << (unsigned char)low_res;
                }
            }
        }
    }
}

void message_update(void)
{
    SYNCDBG(6,"Starting");
    int i = game.active_messages_count - 1;
    // Set end turn for all messages
    while (i >= 0)
    {
        struct GuiMessage* gmsg = &gameadd.messages[i];
        if (gmsg->creation_turn < game.play_gameturn)
        {
            game.active_messages_count--;
            gameadd.messages[game.active_messages_count].text[0] = 0;
        }
        i--;
    }
}

void zero_messages(void)
{
    game.active_messages_count = 0;
    for (int i = 0; i < 3; i++)
    {
      memset(&gameadd.messages[i], 0, sizeof(struct GuiMessage));
    }
}

void clear_messages_from_player(char plyr_idx)
{
    for (int i = 0; i < game.active_messages_count; i++)
    {
        if ((char)gameadd.messages[i].plyr_idx == plyr_idx)
        {
            delete_message(i);
        }
    }
}

void delete_message(unsigned char msg_idx)
{
    memset(&gameadd.messages[msg_idx], 0, sizeof(struct GuiMessage));
    if (msg_idx < game.active_messages_count - 1)
    {
        for (int i = msg_idx; i < game.active_messages_count; i++)
        {
            gameadd.messages[i] = gameadd.messages[i+1];
        }
        memset(&gameadd.messages[game.active_messages_count - 1], 0, sizeof(struct GuiMessage));
    }
    game.active_messages_count--;
}

void message_add(PlayerNumber plyr_idx, const char *text)
{
    SYNCDBG(2,"Player %d: %s",(int)plyr_idx,text);
    for (int i = GUI_MESSAGES_COUNT - 1; i > 0; i--)
    {
        memcpy(&gameadd.messages[i], &gameadd.messages[i-1], sizeof(struct GuiMessage));
    }
    snprintf(gameadd.messages[0].text, sizeof(gameadd.messages[0].text), "%s", text);
    gameadd.messages[0].plyr_idx = plyr_idx;
    gameadd.messages[0].creation_turn = game.play_gameturn + GUI_MESSAGES_DELAY;
    gameadd.messages[0].target_idx = -1;
    if (game.active_messages_count < GUI_MESSAGES_COUNT) {
        game.active_messages_count++;
    }
}

void message_add_va(PlayerNumber plyr_idx, const char *fmt_str, va_list arg)
{
    static char full_msg_text[2048];
    vsnprintf(full_msg_text, sizeof(full_msg_text), fmt_str, arg);
    message_add(plyr_idx, full_msg_text);
}

void message_add_fmt(PlayerNumber plyr_idx, const char *fmt_str, ...)
{
    va_list val;
    va_start(val, fmt_str);
    message_add_va(plyr_idx, fmt_str, val);
    va_end(val);
}

void targeted_message_add(PlayerNumber plyr_idx, PlayerNumber target_idx, unsigned long timeout, const char *fmt_str, ...)
{
    va_list val;
    va_start(val, fmt_str);
    static char full_msg_text[2048];
    vsnprintf(full_msg_text, sizeof(full_msg_text), fmt_str, val);
    SYNCDBG(2,"Player %d: %s",(int)plyr_idx,full_msg_text);
    for (int i = GUI_MESSAGES_COUNT - 1; i > 0; i--)
    {
        memcpy(&gameadd.messages[i], &gameadd.messages[i-1], sizeof(struct GuiMessage));
    }
    snprintf(gameadd.messages[0].text, sizeof(gameadd.messages[0].text), "%s", full_msg_text);
    gameadd.messages[0].plyr_idx = plyr_idx;
    gameadd.messages[0].creation_turn = game.play_gameturn + timeout;
    gameadd.messages[0].target_idx = target_idx;
    if (game.active_messages_count < GUI_MESSAGES_COUNT) {
        game.active_messages_count++;
    }
    va_end(val);
}

void show_game_time_taken(unsigned long fps, unsigned long turns)
{
    struct GameTime gt = get_game_time(turns, fps);
    struct PlayerInfo* player = get_my_player();
    targeted_message_add(player->id_number, player->id_number, GUI_MESSAGES_DELAY, "%s: %02ld:%02ld:%02ld", get_string(746), gt.Hours, gt.Minutes, gt.Seconds);
}

void show_real_time_taken(void)
{
    update_time();
    struct PlayerInfo* player = get_my_player();
    targeted_message_add(player->id_number, player->id_number, GUI_MESSAGES_DELAY, "%s: %02ld:%02ld:%02ld:%03ld", get_string(746), Timer.Hours, Timer.Minutes, Timer.Seconds, Timer.MSeconds);
}
/******************************************************************************/
