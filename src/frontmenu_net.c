/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_net.c
 *     GUI menus for network support.
 * @par Purpose:
 *     Functions to show and maintain network screens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "frontmenu_net.h"
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_netsession.h"

#include <SDL_net.h>
#include <json-dom.h>

#include "bflib_datetm.h"
#include "bflib_guibtns.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "value_util.h"

#include "custom_sprites.h"
#include "front_network.h"
#include "front_masterserver.h"
#include "frontmenu_ingame_tabs.h"
#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "front_landview.h"
#include "front_input.h"
#include "net_game.h"
#include "kjm_input.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "sprites.h"
#include "keeperfx.hpp"
#include "post_inc.h"

char fe_masterserver_token[32]; //Token

/******************************************************************************/
long frontnet_number_of_players_in_session(void)
{
    long i;
    long nplyr;
    nplyr = 0;
    for (i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        if (network_player_active(i))
            nplyr++;
    }
    return nplyr;
}

void frontnet_session_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_session_scroll_offset != 0)) & LbBtnF_Enabled;

    if (wheel_scrolled_up || (is_key_pressed(KC_UP, KMod_NONE)))
    {
        if (net_session_scroll_offset > 0)
            net_session_scroll_offset--;
    }
}

void frontnet_session_down_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^=
            (gbtn->flags ^ LbBtnF_Enabled * (net_number_of_sessions - 1 > net_session_scroll_offset)) & LbBtnF_Enabled;

    if (wheel_scrolled_down || (is_key_pressed(KC_DOWN, KMod_NONE)))
    {
        if (net_session_scroll_offset < net_number_of_sessions - 1)
            net_session_scroll_offset++;
    }
}

void frontnet_session_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^
                    LbBtnF_Enabled * (net_session_scroll_offset + (long) gbtn->content - 45 < net_number_of_sessions)) &
                   LbBtnF_Enabled;
}

void frontnet_players_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_player_scroll_offset != 0)) & LbBtnF_Enabled;
}

void frontnet_players_down_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_number_of_enum_players - 1 > net_player_scroll_offset)) &
                   LbBtnF_Enabled;
}

void frontnet_refresh_maintain(struct GuiButton *gbtn)
{
    if (is_key_pressed(KC_F5, KMod_NONE))
    {
        frontend_mouse_over_button = (long) gbtn->content;
        masterserver_fetch_sessions();
    }
}

void frontnet_join_game_maintain(struct GuiButton *gbtn)
{
    if ((net_session_index_active != -1) && (net_session[net_session_index_active] != NULL) &&
        (net_session[net_session_index_active]->joinable))
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontnet_maintain_alliance(struct GuiButton *gbtn)
{
    long plyr_idx1;
    long plyr_idx2;
    plyr_idx1 = gbtn->btype_value & LbBFeF_IntValueMask;
    plyr_idx2 = (long) gbtn->content - 74;
    if (plyr_idx2 >= net_number_of_enum_players || net_number_of_enum_players <= plyr_idx1 || plyr_idx2 == plyr_idx1)
        gbtn->flags &= ~LbBtnF_Enabled;
    else
        gbtn->flags |= LbBtnF_Enabled;
}

void frontnet_messages_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_message_scroll_offset != 0)) & LbBtnF_Enabled;
}

void frontnet_messages_down_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^=
            (gbtn->flags ^ LbBtnF_Enabled * (net_number_of_messages - 1 > net_message_scroll_offset)) & LbBtnF_Enabled;
}

void frontnet_start_game_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_number_of_enum_players > 1)) & LbBtnF_Enabled;
}

TbBool frontnet_start_input(void)
{
    if (lbInkey != KC_UNASSIGNED)
    {
        unsigned short asckey;
        asckey = key_to_ascii(lbInkey, KMod_NONE);
        if ((lbInkey == KC_BACK) || (lbInkey == KC_RETURN) || (frontend_font_char_width(1, asckey) > 0))
        {
            struct ScreenPacket *nspck;
            nspck = &net_screen_packet[my_player_number];
            if ((nspck->field_4 & 0xF8) == 0)
            {
                nspck->field_4 = (nspck->field_4 & 7) | 0x40;
                nspck->param1 = lbInkey;
                nspck->param2 = key_modifiers;
                if (key_modifiers)
                {
                    lbInkey = KC_UNASSIGNED;
                    return true;
                }
            }
        }
        lbInkey = KC_UNASSIGNED;
    }
    return false;
}

void frontnet_draw_services_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_service_scroll_offset, frontend_services_menu_items_visible - 2,
                             net_number_of_services);
}

void frontnet_session_set_player_name(struct GuiButton *gbtn)
{
    strcpy(net_player_name, tmp_net_player_name);
    strcpy(net_config_info.net_player_name, tmp_net_player_name);
    net_write_config_file();
}

void frontnet_draw_text_bar(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    int i;
    long pos_x;
    long pos_y;
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    int fs_units_per_px;
    fs_units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, GFS_largearea_nx1_tx5_c, 100);
    spr = &frontend_sprite[GFS_largearea_nx1_cor_l];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_largearea_nx1_tx5_c];
    for (i = 0; i < 4; i++)
    {
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
    }
    spr = &frontend_sprite[GFS_largearea_nx1_cor_r];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
}

void frontnet_session_up(struct GuiButton *gbtn)
{
    if (net_session_scroll_offset > 0)
        net_session_scroll_offset--;
}

void frontnet_session_down(struct GuiButton *gbtn)
{
    if (net_session_scroll_offset < net_number_of_sessions - 1)
        net_session_scroll_offset++;
}

void frontnet_draw_sessions_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_session_scroll_offset, 0, net_number_of_sessions);
}

void frontnet_players_up(struct GuiButton *gbtn)
{
    if (net_player_scroll_offset > 0)
        net_player_scroll_offset--;
}

void frontnet_players_down(struct GuiButton *gbtn)
{
    if (net_player_scroll_offset < net_number_of_enum_players - 1)
        net_player_scroll_offset++;
}

void frontnet_draw_players_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_player_scroll_offset, 0, net_number_of_enum_players);
}

void frontnet_draw_net_session_players(struct GuiButton *gbtn)
{
    int i;
    i = frontend_button_caption_font(gbtn, 0);
    lbDisplay.DrawFlags = 0;
    LbTextSetFont(frontend_font[i]);
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / (2 * LbTextLineHeight());
    const struct TbSprite *spr;
    spr = &frontend_sprite[GFS_bullfrog_red_med];
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / (2 * (spr->SHeight * 13 / 8));
    int height;
    height = LbTextLineHeight() * tx_units_per_px / 16;
    long netplyr_idx;
    int shift_y;
    netplyr_idx = net_player_scroll_offset;
    for (shift_y = 0; shift_y < gbtn->height; shift_y += height, netplyr_idx++)
    {
        const char *text;
        text = net_player[netplyr_idx].name;
        if (netplyr_idx >= net_number_of_enum_players)
            break;
        spr = &frontend_sprite[GFS_bullfrog_red_med + netplyr_idx];
        i = height - spr->SHeight * fs_units_per_px / 16;
        LbSpriteDrawResized(gbtn->scr_pos_x, gbtn->scr_pos_y + shift_y + abs(i) / 2, fs_units_per_px, spr);
        LbTextSetWindow(gbtn->scr_pos_x, shift_y + gbtn->scr_pos_y, gbtn->width - spr->SWidth * fs_units_per_px / 16,
                        height);
        LbTextDrawResized(spr->SWidth * fs_units_per_px / 16, 0, tx_units_per_px, text);
    }
}

void frontnet_masterserver_refresh(struct GuiButton *gbtn)
{
    masterserver_fetch_sessions();
}

void frontend_masterserver_draw_refresh(struct GuiButton *gbtn)
{
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();

    // TODO: cache it somewhere
    const struct TbSprite *spr;
    if ((long) gbtn->content == frontend_mouse_over_button)
    {
        spr = get_frontend_sprite(GFS_refresh_ylw);
    }
    else
    {
        spr = get_frontend_sprite(GFS_refresh_red);
    }
    if (spr)
    {
        LbSpriteDrawResized(gbtn->scr_pos_x + (gbtn->width * 3 / 4), gbtn->scr_pos_y + (gbtn->height / 3),
                            tx_units_per_px, spr);
    }
}

void frontnet_session_add(struct GuiButton *gbtn)
{
    turn_on_menu(GMnu_FEADD_SESSION);
    //TODO NET When clicked, it should display a modal text field (for IP address) and OK/Cancel buttons.
    set_menu_visible_on(GMnu_FEADD_SESSION);
}

void frontnet_session_join(struct GuiButton *gbtn)
{
    long plyr_num;
    if (net_session_index_active == -1)
    {
        display_message(TR("No session selected"));
        return;
    }
    plyr_num = network_session_join();
    if (plyr_num < 0)
        return;
    frontend_set_player_number(plyr_num);
    frontend_set_state(FeSt_NET_START);
}

void frontnet_return_to_main_menu(struct GuiButton *gbtn)
{
    if (LbNetwork_Stop())
    {
        ERRORLOG("LbNetwork_Stop() failed");
        return;
    }
    frontend_set_state(FeSt_MAIN_MENU);
}

void frontnet_add_session_back(struct GuiButton *gbtn)
{
    //TODO NET Finish session add menu
    turn_off_menu(GMnu_FEADD_SESSION);
}

void frontnet_add_session_done(struct GuiButton *gbtn)
{
    //TODO NET Finish session add menu
    turn_off_menu(GMnu_FEADD_SESSION);
}

void frontnet_draw_alliance_box_tab(struct GuiButton *gbtn)
{
    int units_per_px;
    units_per_px = (gbtn->width * 16 + 100 / 2) / 100;

    const struct TbSprite *spr;
    int pos_x;
    int pos_y;

    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    spr = &frontend_sprite[GFS_hugearea_thc_cor_tl];
    int fs_units_per_px;
    fs_units_per_px = spr->SHeight * units_per_px / 26;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_hugearea_thc_tx2_tc];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_hugearea_thc_cor_tr];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);

    pos_y += 5;
    pos_x = gbtn->scr_pos_x;
    spr = &frontend_sprite[GFS_hugearea_thc_cor_tl];
    pos_x += spr->SWidth * fs_units_per_px / 16 - 1;
    if (net_number_of_enum_players > 0)
    {
        spr = &frontend_sprite[GFS_bullfrog_red_med];
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
    }
    if (net_number_of_enum_players > 1)
    {
        spr = &frontend_sprite[GFS_bullfrog_blue_med];
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
    }
    if (net_number_of_enum_players > 2)
    {
        spr = &frontend_sprite[GFS_bullfrog_green_med];
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
    }
    if (net_number_of_enum_players > 3)
    {
        spr = &frontend_sprite[GFS_bullfrog_yellow_med];
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    }
}

short frontnet_map_latency_to_icon(int latency_time)
{
    if (latency_time < 120)
    {
        return GFS_status_green;
    }
    else if (latency_time < 250)
    {
        return GFS_status_orange;
    }
    else
    {
        return  GFS_status_red;
    }
}

void frontnet_draw_net_start_players(struct GuiButton *gbtn)
{
    int i;
    i = frontend_button_caption_font(gbtn, 0);
    lbDisplay.DrawFlags = 0;
    LbTextSetFont(frontend_font[i]);
    int height;
    height = 0;
    long netplyr_idx;
    int shift_y;
    netplyr_idx = net_player_scroll_offset;
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / (4 * LbTextLineHeight());
    const struct TbSprite *spr;
    spr = &frontend_sprite[GFS_bullfrog_red_med];
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / (4 * (spr->SHeight * 13 / 8));
    height = LbTextLineHeight() * tx_units_per_px / 16;
    for (shift_y = 0; shift_y < gbtn->height; shift_y += height, netplyr_idx++)
    {
        const char *text;
        text = net_player[netplyr_idx].name;
        if (netplyr_idx >= net_number_of_enum_players)
            break;

        long subplyr_idx;
        for (subplyr_idx = 0; subplyr_idx < net_number_of_enum_players; subplyr_idx++)
        {
            if (subplyr_idx >= NET_PLAYERS_COUNT)
                break;
            if (net_player_info[subplyr_idx].active)
            {
                if (subplyr_idx == netplyr_idx)
                    break;
            }
        }
        // Icon
        spr = &frontend_sprite[GFS_bullfrog_red_med + netplyr_idx];
        i = height - spr->SHeight * fs_units_per_px / 16;
        LbSpriteDrawResized(gbtn->scr_pos_x, gbtn->scr_pos_y + shift_y + abs(i) / 2, fs_units_per_px, spr);
        int pos_x = gbtn->scr_pos_x + spr->SWidth * fs_units_per_px / 16;

        // Latency
        short latencystatus = frontnet_map_latency_to_icon(net_player_info[netplyr_idx].latency);
        LbSpriteDrawResized(gbtn->scr_pos_x - 8, gbtn->scr_pos_y + (gbtn->height / 5), tx_units_per_px,
                            get_frontend_sprite(latencystatus));

        pos_x += spr ->SWidth * fs_units_per_px / 16;
        LbTextSetWindow(pos_x, gbtn->scr_pos_y + shift_y,
                        gbtn->width - spr->SWidth * fs_units_per_px / 16, height);
        LbTextDrawResized(0, 0, tx_units_per_px, text);
    }
}

void frontnet_select_alliance(struct GuiButton *gbtn)
{
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    int plyr1_idx;
    int plyr2_idx;
    plyr1_idx = (long) gbtn->content - 74;
    plyr2_idx = gbtn->btype_value & LbBFeF_IntValueMask;
    if (plyr1_idx == myplyr->id_number || plyr2_idx == myplyr->id_number)
    {
        struct ScreenPacket *nspck;
        nspck = &net_screen_packet[my_player_number];
        if ((nspck->field_4 & 0xF8) == 0)
        {
            nspck->field_4 = (nspck->field_4 & 7) | 0x20;
            nspck->param1 = plyr1_idx;
            nspck->param2 = plyr2_idx;
        }
    }
}

void frontnet_draw_alliance_grid(struct GuiButton *gbtn)
{
    int pos_x;
    int pos_y;
    pos_y = gbtn->scr_pos_y;
    struct TbSprite *spr;
    int netplyr_idx;
    int units_per_px;
    units_per_px = gbtn->height * 16 / (22 * 4);

    pos_x = gbtn->scr_pos_x;
    spr = &frontend_sprite[GFS_slidrect_indicator_std0];
    for (netplyr_idx = 0; netplyr_idx < NET_PLAYERS_COUNT; netplyr_idx++)
    {
        LbSpriteDrawResized(pos_x / pixel_size, pos_y / pixel_size, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
    }
    pos_y += spr->SHeight * units_per_px / 16;

    pos_x = gbtn->scr_pos_x;
    spr = &frontend_sprite[GFS_slidrect_indicator_std1];
    for (netplyr_idx = 0; netplyr_idx < NET_PLAYERS_COUNT; netplyr_idx++)
    {
        LbSpriteDrawResized(pos_x / pixel_size, pos_y / pixel_size, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
    }
    pos_y += spr->SHeight * units_per_px / 16;

    pos_x = gbtn->scr_pos_x;
    spr = &frontend_sprite[GFS_slidrect_indicator_std2];
    for (netplyr_idx = 0; netplyr_idx < NET_PLAYERS_COUNT; netplyr_idx++)
    {
        LbSpriteDrawResized(pos_x / pixel_size, pos_y / pixel_size, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
    }
    pos_y += spr->SHeight * units_per_px / 16;

    pos_x = gbtn->scr_pos_x;
    spr = &frontend_sprite[GFS_slidrect_indicator_std2];
    for (netplyr_idx = 0; netplyr_idx < NET_PLAYERS_COUNT; netplyr_idx++)
    {
        LbSpriteDrawResized(pos_x / pixel_size, pos_y / pixel_size, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
    }
    pos_y += spr->SHeight * units_per_px / 16;
}

void frontnet_draw_alliance_button(struct GuiButton *gbtn)
{
    int plyr1_idx;
    int plyr2_idx;
    struct TbSprite *spr;
    plyr2_idx = gbtn->btype_value & LbBFeF_IntValueMask;
    plyr1_idx = (long) gbtn->content - 74;
    if ((plyr1_idx == plyr2_idx) || (frontend_alliances & alliance_grid[plyr1_idx][plyr2_idx]))
        spr = &frontend_sprite[GFS_scrollbar_indicator_std];
    else
        spr = &frontend_sprite[GFS_slidrect_indicator_std1];
    int units_per_px;
    units_per_px = gbtn->height * 16 / spr->SHeight;
    LbSpriteDrawResized(gbtn->scr_pos_x, gbtn->scr_pos_y, units_per_px, spr);
}

void frontnet_messages_up(struct GuiButton *gbtn)
{
    if (net_message_scroll_offset > 0)
        net_message_scroll_offset--;
}

void frontnet_messages_down(struct GuiButton *gbtn)
{
    if (net_message_scroll_offset < net_number_of_messages - 1)
        net_message_scroll_offset++;
}

void frontnet_draw_bottom_scroll_box_tab(struct GuiButton *gbtn)
{
    long pos_x;
    long pos_y;
    struct TbSprite *spr;
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    lbDisplay.DrawFlags = Lb_SPRITE_FLIP_VERTIC;
    spr = &frontend_sprite[GFS_hugearea_thc_cor_tl];
    int fs_units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, GFS_hugearea_thc_tx1_tc, 100);
    int tail = gbtn->width - (15 + 9) * fs_units_per_px / 16; //Left & Right borders;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_hugearea_thc_tx1_tc];
    for (; tail > 0; tail -= spr->SWidth * fs_units_per_px / 16)
    {
        //LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        long dwidth = (spr->SWidth * fs_units_per_px + 8) / 16;
        LbSpriteSetScalingData(pos_x, pos_y, spr->SWidth, spr->SHeight,
                               dwidth, (spr->SHeight * fs_units_per_px + 8) / 16);
        if (tail < spr->SWidth)
        {
            LbSpriteSetScalingWidthClipped(pos_x, spr->SWidth, dwidth, pos_x + tail);
            pos_x += tail;
        }
        else
        {
            pos_x += spr->SWidth * fs_units_per_px / 16;
        }
        LbSpriteDrawUsingScalingData(0, 0, spr);

    }
    spr = &frontend_sprite[GFS_hugearea_thc_cor_tr];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    lbDisplay.DrawFlags = 0;
}

void frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_message_scroll_offset, 0, net_number_of_messages);
}

void frontnet_draw_scroll_selection_box(struct GuiButton *gbtn, long font_idx, const char *text)
{
    struct TbSprite *spr;
    int pos_x;
    int i;
    unsigned char height;
    spr = &frontend_sprite[GFS_largearea_xts_cor_l];
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / spr->SHeight;
    pos_x = gbtn->scr_pos_x;
    for (i = 6; i > 0; i--)
    {
        LbSpriteDrawResized(pos_x, gbtn->scr_pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
        spr++;
    }

    if (text != NULL)
    {
        LbTextSetFont(frontend_font[font_idx]);
        lbDisplay.DrawFlags = 0;
        int tx_units_per_px;
        tx_units_per_px = (gbtn->height * 13 / 14) * 16 / LbTextLineHeight();
        height = LbTextLineHeight() * tx_units_per_px / 16;
        LbTextSetWindow(gbtn->scr_pos_x + 13 * fs_units_per_px / 16, gbtn->scr_pos_y,
                        gbtn->width - 26 * fs_units_per_px / 16, height);
        LbTextDrawResized(0, 0, tx_units_per_px, text);
    }
}

void frontnet_draw_current_message(struct GuiButton *gbtn)
{
    static TbClockMSec last_time = 0;
    static TbBool print_with_cursor = 1;

    struct PlayerInfo *player;
    int font_idx;
    char text[2048];
    // Blink cursor - switch state every 100ms
    if (LbTimerClock() >= last_time + 100)
    {
        print_with_cursor = !print_with_cursor;
        last_time = LbTimerClock();
    }

    // Get player
    player = get_my_player();
    if (player_invalid(player))
    {
        return;
    }

    // Prepare text buffer and font
    snprintf(text, sizeof(text), "%s%s", player->mp_message_text, print_with_cursor ? "_" : "");
    font_idx = frontend_button_caption_font(gbtn, 0);
    // And draw the message
    frontnet_draw_scroll_selection_box(gbtn, font_idx, text);
}

void frontnet_draw_messages(struct GuiButton *gbtn)
{
    int font_idx;
    font_idx = frontend_button_caption_font(gbtn, 0);
    LbTextSetFont(frontend_font[font_idx]);
    lbDisplay.DrawFlags = 0;
    // While setting scale, aim for 4 lines of text
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / (4 * LbTextLineHeight());
    struct TbSprite *spr;
    spr = &frontend_sprite[GFS_bullfrog_red_med];
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / (4 * (spr->SHeight * 13 / 8));
    int font_height;
    font_height = LbTextLineHeight() * tx_units_per_px / 16;
    int y;
    y = 0;
    int netmsg_id;
    for (netmsg_id = net_message_scroll_offset; netmsg_id < net_number_of_messages; netmsg_id++)
    {
        if (y + font_height / 2 > gbtn->height)
            break;
        struct NetMessage *nmsg;
        nmsg = &net_message[netmsg_id];
        int num_active;
        num_active = 0;
        int i;
        for (i = nmsg->plyr_idx; i > 0; i--)
        {
            if (net_player_info[i].active)
                num_active++;
        }

        spr = &frontend_sprite[GFS_bullfrog_red_med + num_active];

        i = font_height - spr->SHeight * fs_units_per_px / 16;
        LbSpriteDrawResized(gbtn->scr_pos_x, y + gbtn->scr_pos_y + (i >> 1), fs_units_per_px, spr);

        LbTextSetWindow(gbtn->scr_pos_x, y + gbtn->scr_pos_y, gbtn->width, min(font_height, gbtn->height - y));
        LbTextDrawResized(spr->SWidth * fs_units_per_px / 16, 0, tx_units_per_px, nmsg->text);

        y += font_height;
    }
}

void masterserver_remove_session()
{
    char out_buf[256];
    VALUE ret_buf;
    VALUE *ret = &ret_buf;
    if (my_player_number != SERVER_ID)
        return;
    if (fe_public && *fe_masterserver_token)
    {
        sprintf(out_buf, "{\"method\":\"remove_lobby\",\"token\":\"%s\"}\n", fe_masterserver_token);
        fe_masterserver_token[0] = 0;
        send_json_to_masterserver(out_buf, ret);
        value_fini(ret);
    }
}

void frontnet_return_to_session_menu(struct GuiButton *gbtn)
{
    masterserver_remove_session();
    if (LbNetwork_Stop())
    {
        ERRORLOG("LbNetwork_Stop() failed");
    }
    FrontendMenuState nstate;
    nstate = get_menu_state_when_back_from_substate(FeSt_NET_START);
    if (nstate == FeSt_NET_SESSION)
    {
        // If the parent state is network session state, try to stay in net service
        if (!setup_old_network_service())
        {
            nstate = get_menu_state_when_back_from_substate(nstate);
        }
    }
    frontend_set_state(nstate);
}

void frontnet_draw_small_scroll_box_tab(struct GuiButton *gbtn)
{
    long pos_x;
    long pos_y;
    struct TbSprite *spr;
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    spr = &frontend_sprite[GFS_hugearea_thc_tx1_tc];
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / spr->SHeight;
    spr = &frontend_sprite[GFS_hugearea_thc_cor_tl];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_hugearea_thc_tx1_tc];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_hugearea_thc_tx2_tc];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_hugearea_thc_cor_tr];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
}

void frontnet_draw_small_scroll_selection_box(struct GuiButton *gbtn, long font_idx, const char *text)
{
    struct TbSprite *spr;
    int pos_x;
    int pos_y;
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    spr = &frontend_sprite[GFS_largearea_xts_cor_l];
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / spr->SHeight;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_largearea_xts_tx1_c];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[GFS_largearea_xts_cor_r];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    if (text != NULL)
    {
        LbTextSetFont(frontend_font[font_idx]);
        lbDisplay.DrawFlags = 0;
        int tx_units_per_px;
        tx_units_per_px = (gbtn->height * 13 / 14) * 16 / LbTextLineHeight();
        int height;
        height = LbTextLineHeight() * tx_units_per_px / 16;
        LbTextSetWindow(gbtn->scr_pos_x + 13 * fs_units_per_px / 16, gbtn->scr_pos_y,
                        gbtn->width - 26 * fs_units_per_px / 16, height);
        LbTextDrawResized(0, 0, tx_units_per_px, text);
    }
}

int small_scroll_box_get_units_per_px(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    int width;
    width = 0;
    spr = &frontend_sprite[GFS_hugearea_thc_cor_ml];
    width += spr->SWidth;
    spr++;
    width += spr->SWidth;
    spr++;
    width += spr->SWidth;
    spr += 3;
    width += spr->SWidth;
    spr++;
    width += spr->SWidth;
    return (gbtn->width * 16 + 8) / width;
}

void frontnet_draw_small_scroll_box(struct GuiButton *gbtn)
{
    long pos_x;
    long pos_y;
    struct TbSprite *spr;
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    int fs_units_per_px;
    fs_units_per_px = small_scroll_box_get_units_per_px(gbtn);
    int btn_type;
    int len;
    btn_type = (long) gbtn->content;
    if (btn_type == 24)
    {
        len = 2;
    }
    else if (btn_type == 25)
    {
        len = 3;
    }
    else if (btn_type == 26)
    {
        len = 7;
    }
    else
    {
        ERRORLOG("Unknown button type %d", (int) btn_type);
        return;
    }
    spr = &frontend_sprite[GFS_hugearea_thn_cor_tl];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr++;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr++;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr += 3;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr++;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);

    int dlen;
    dlen = 3;
    spr = &frontend_sprite[GFS_hugearea_thn_cor_tl];
    pos_y += spr->SHeight * fs_units_per_px / 16;
    for (; len > 0; len -= dlen)
    {
        pos_x = gbtn->scr_pos_x;
        int spr_idx;
        if (len < 3)
            spr_idx = GFS_hugearea_thn_cor_ml;
        else
            spr_idx = GFS_hugearea_thc_cor_ml;
        spr = &frontend_sprite[spr_idx];
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
        spr++;
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
        spr++;
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
        spr += 3;
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
        if (len < 3)
            spr_idx = GFS_scrollbar_vert_ct_short;
        else
            spr_idx = GFS_scrollbar_vert_ct_long;
        spr = &frontend_sprite[spr_idx];
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_y += spr->SHeight * fs_units_per_px / 16;
        if (len < 3)
            dlen = 1;
        else
            dlen = 3;
    }

    pos_x = gbtn->scr_pos_x;
    spr = &frontend_sprite[GFS_hugearea_thn_cor_bl];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr++;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr++;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr += 3;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr++;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
}

void frontnet_draw_text_cont_bar(struct GuiButton *gbtn)
{
    int units_per_px;
    units_per_px = (gbtn->width * 16 + 165 / 2) / 165;

    int pos_x;
    int pos_y;
    pos_y = gbtn->scr_pos_y;
    struct TbSprite *spr;
    int netplyr_idx;
    pos_x = gbtn->scr_pos_x;

    spr = &frontend_sprite[GFS_largearea_nx2_cor_l];
    int fs_units_per_px;
    fs_units_per_px = spr->SHeight * units_per_px / 28;
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;

    spr = &frontend_sprite[GFS_largearea_nx2_tx5_c];
    for (netplyr_idx = 0; netplyr_idx < NET_PLAYERS_COUNT; netplyr_idx++)
    {
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
    }

    spr = &frontend_sprite[GFS_largearea_nx2_cor_r];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
}

void frontnet_service_up_maintain(struct GuiButton *gbtn)
{
    if (net_service_scroll_offset > 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontnet_service_down_maintain(struct GuiButton *gbtn)
{
    if (net_service_scroll_offset < net_number_of_services - frontend_services_menu_items_visible + 1)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontnet_service_up(struct GuiButton *gbtn)
{
    if (net_service_scroll_offset > 0)
        net_service_scroll_offset--;
}

void frontnet_service_down(struct GuiButton *gbtn)
{
    if (net_service_scroll_offset < net_number_of_services - frontend_services_menu_items_visible + 1)
        net_service_scroll_offset++;
}

void frontnet_service_maintain(struct GuiButton *gbtn)
{
    int srvidx;
    srvidx = (long) gbtn->content + net_service_scroll_offset - 45;
    if (srvidx < net_number_of_services)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontnet_draw_service_button(struct GuiButton *gbtn)
{
    int srvidx;
    // Find and verify selected network service
    srvidx = (long) gbtn->content + net_service_scroll_offset - 45;
    if (srvidx >= net_number_of_services)
        return;
    // Select font to draw
    int font_idx;
    font_idx = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
    LbTextSetFont(frontend_font[font_idx]);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    // Set drawing window and draw the text
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();
    int height;
    height = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, height);
    LbTextDrawResized(0, 0, tx_units_per_px, net_service[srvidx]);
}

void frontnet_service_select(struct GuiButton *gbtn)
{
    int srvidx;
    srvidx = (long) (gbtn->content) + net_service_scroll_offset - 45;
    if (((game.system_flags & GSF_AllowOnePlayer) != 0)
        && (srvidx + 1 >= net_number_of_services))
    {
        fe_network_active = 0;
        frontend_set_state(FeSt_NETLAND_VIEW);
    }
    else if (srvidx < 0)
    {
        frontend_set_state(FeSt_NET_SERVICE);
    }
    else
    {
        setup_network_service(srvidx);
    }
}

void frontnet_init_session_menu()
{
    int idx = guibutton_get_idx_by_id(BID_MASTERSERVER_REFRESH);
    if (net_service_index_selected != NS_ENET_UDP)
    {
        guibutton_get(idx)->flags &= ~LbBtnF_Visible;
    }
}

void frontnet_init_session_start_menu()
{
    TbBool show = (net_service_index_selected == NS_ENET_UDP);
    int idx = guibutton_get_idx_by_id(BID_MASTERSERVER_PUBLIC);
    if (!show)
    {
        guibutton_get(idx)->flags &= ~LbBtnF_Visible;
        guibutton_get(idx - 1)->flags &= ~LbBtnF_Visible;
    }
}

#define MAX_RECV_BUF 1024

TbBool send_json_to_masterserver(char *buf, VALUE *out)
{
    char recv_buf[MAX_RECV_BUF];
    TbBool bool_ret = false;
    VALUE tmp_obj;
    VALUE *ret = &tmp_obj, *val;
    // Check for masterserver
    VALUE *masterserver = value_dict_get(&config_dict, "MASTERSERVER_HOST");
    if (masterserver == NULL)
    {
        // No Masterserver;
        return true;
    }

    // Find an address
    const char *str_src = value_string(masterserver);
    if (str_src == NULL)
    {
        // Invalid masterserver url
        return true;
    }
    strcpy(recv_buf, str_src);
    if (*recv_buf == 0)
    {
        // Empty Masterserver;
        return true;
    }
    char *p = strchr(recv_buf, ':');
    char *end;
    Uint16 port;
    if (p)
    {
        *p = 0;
        p++;
        port = strtol(p, &end, 10);
        if (end == p)
        {
            port = 5566;
        }
    }
    else
    {
        port = 5566;
    }
    IPaddress addr;
    SDLNet_ResolveHost(&addr, recv_buf, port);

    // Connect
    TCPsocket sock = SDLNet_TCP_Open(&addr);
    if (sock == NULL)
    {
        ERRORLOG("Unable to connect to Masterserver at '%s'", value_string(masterserver));
        return false;
    }
    // Parse greeting
    int len = SDLNet_TCP_Recv(sock, recv_buf, MAX_RECV_BUF);
    if ((len <= 0) || json_dom_parse(recv_buf, len, NULL, 0, ret, NULL))
    {
        goto unable;
    }
    VALUE_GET_KEY("keeperfx");
    if (!value_bool(val))
    {
        ERRORLOG("Not a masterserver?");
        goto end;
    }
    VALUE_GET_KEY("v")
    if (value_int32(val) != 1)
    {
        ERRORLOG("Unsupported ver");
        goto end;
    }
    VALUE_GET_KEY("success");
    if (!value_bool(val))
    {
        ERRORLOG("Got an error from masterserver");
        goto end;
    }
    value_fini(ret);
    // Send a buf
    len = (int) strlen(buf);
    if (SDLNet_TCP_Send(sock, buf, len) >= len)
    {
        len = SDLNet_TCP_Recv(sock, recv_buf, MAX_RECV_BUF);
        if ((len <= 0) || json_dom_parse(recv_buf, len, NULL, 0, out, NULL))
        {
            goto unable;
        }
    }
    bool_ret = true;
    goto end;
    unable:
    WARNLOG("Unable to parse answer from masterserver");
    bool_ret = false;
    end:
    value_fini(ret);
    SDLNet_TCP_Close(sock);
    return bool_ret;
}

int masterserver_create_lobby(VALUE *ret)
{
    char out_buf[256];
    VALUE *val;

    sprintf(out_buf, "{\"method\":\"create_lobby\",\"player_name\":\"%s\",\"game_version\":\""PRODUCT_VERSION"\"}\n",
            net_player_name);
    send_json_to_masterserver(out_buf, ret);
    VALUE_GET_KEY("v");
    if (value_int32(val) != 1)
    {
        ERRORLOG("Unsupported ver");
        return 0;
    }
    VALUE_GET_KEY("token");
    if (value_string_length(val) >= sizeof(fe_masterserver_token))
    {
        ERRORLOG("Token is too big");
        return 0;
    }
    const char *token = value_string(val);
    strcpy(fe_masterserver_token, token);
    return 1;
    unable: // Used by macro
    return 0;
}

void masterserver_toggle_public(struct GuiButton *gbtn)
{
    VALUE ret_obj = {0};
    VALUE *ret = &ret_obj;
    if (my_player_number != SERVER_ID)
    {
        // TODO: update fe_public from server or masterserver
        return;
    }
    masterserver_remove_session();
    fe_public = !fe_public;
    if (fe_public)
    {
        if (masterserver_create_lobby(ret))
        {
            goto end;
        }
        ERRORLOG("Unable to parse response from masterserver");
        end:
        value_fini(ret);
    }
}

void masterserver_session_started()
{
    char out_buf[512];
    char players[256] = "", *P;
    VALUE ret_buf;
    VALUE *ret = &ret_buf;
    if (fe_public && *fe_masterserver_token)
    {
        P = players;
        for (int i = 0; i < net_number_of_enum_players; i++)
        {
            if (P != players)
            {
                *P = ',';
                P++;
            }
            if (net_player_info[i].active)
            {
                P += sprintf(P, "{\"name\":\"%s\",\"color\":\"%s\"}", net_player[i].name,
                             cmpgn_human_player_options[i].name);
            }
        }
        sprintf(out_buf, "{\"method\":\"update_lobby\",\"token\":\"%s\",\"players\":[%s],\"status\":\"playing\"}\n",
                fe_masterserver_token, players);
        fe_masterserver_token[0] = 0;
        send_json_to_masterserver(out_buf, ret);
        value_fini(ret);
    }
}

void masterserver_send_update()
{
    char out_buf[512];
    char players[256] = "", *P;
    VALUE ret_buf;
    VALUE *ret = &ret_buf;
    if (fe_public && *fe_masterserver_token)
    {
        P = players;
        for (int i = 0; i < net_number_of_enum_players; i++)
        {
            if (P != players)
            {
                *P = ',';
                P++;
            }
            if (net_player_info[i].active)
            {
                P += sprintf(P, "{\"name\":\"%s\",\"color\":\"%s\"}", net_player[i].name,
                             cmpgn_human_player_options[i].name);
            }
        }
        sprintf(out_buf, "{\"method\":\"update_lobby\",\"token\":\"%s\",\"players\":[%s]}\n",
                fe_masterserver_token, players);
        send_json_to_masterserver(out_buf, ret);
        VALUE *err = value_dict_get(ret, "error");
        if (err && (0 == strcmp(value_string(err), "LOBBY_NOT_FOUND")))
        {
            // reconnect
            value_fini(ret);
            WARNLOG("LOBBY_NOT_FOUND");
            if (!masterserver_create_lobby(ret))
            {
                ERRORLOG("Unable to recreate lobby");
            }
        }
        else if (err)
        {
            ERRORLOG("Error from masterserver: %s", value_string(err));
        }
        value_fini(ret);
    }
}
/******************************************************************************/
