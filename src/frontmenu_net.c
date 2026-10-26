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
#include "kfx/renderer/RendererManager.h"
#include "frontmenu_net.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_datetm.h"
#include "bflib_inputctrl.h"
#include "bflib_guibtns.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"

#include "front_network.h"
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
#include "vidfade.h"
#include "keeperfx.hpp"
#include "custom_sprites.h"
#include "bflib_enet.h"
#include "net_exchange_common.h"
#include "net_lobby.h"
#include "packets.h"
#include "post_inc.h"

static int32_t lobby_chat_text_width;

const char *net_join_error_text(enum NetJoinRejection reason)
{
    switch (reason) {
    case NetJoin_InGame:
        return get_string(GUIStr_NetGameStarted);
    case NetJoin_Locked:
        return get_string(GUIStr_NetJoiningLocked);
    case NetJoin_Full:
        return get_string(GUIStr_NetLobbyFull);
    case NetJoin_Version:
    case NetJoin_SpectatorsUnsupported:
        return get_string(GUIStr_NetDifferentVersion);
    case NetJoin_SpectatorState:
        return get_string(GUIStr_NetSpectatorGameNotRunning);
    case NetJoin_SpectatorsDisabled:
        return get_string(GUIStr_NetSpectatorsDisabled);
    case NetJoin_SpectatorsFull:
        return get_string(GUIStr_NetSpectatorsFull);
    case NetJoin_SpectatorDesynced:
        return get_string(GUIStr_NetSpectatorDesynced);
    case NetJoin_MapMismatch:
        return get_string(GUIStr_NetUnsyncedMap);
    case NetJoin_FxdataMismatch:
        return get_string(GUIStr_NetVerifyFxdataSame);
    default:
        return NULL;
    }
}

/******************************************************************************/
void frontnet_session_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_session_scroll_offset != 0)) & LbBtnF_Enabled;
}

void frontnet_session_down_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_number_of_sessions - frontend_sessions_menu_items_visible > net_session_scroll_offset)) & LbBtnF_Enabled;
}

void frontnet_session_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_session_scroll_offset + gbtn->content.lval - 45 < net_number_of_sessions)) & LbBtnF_Enabled;
}

void frontnet_players_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_player_scroll_offset != 0)) & LbBtnF_Enabled;
}

void frontnet_players_down_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_number_of_enum_players - 1 > net_player_scroll_offset)) & LbBtnF_Enabled;
}

void frontnet_messages_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_message_scroll_offset != 0)) & LbBtnF_Enabled;
}

void frontnet_messages_down_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (net_number_of_messages - 1 > net_message_scroll_offset)) & LbBtnF_Enabled;
}

void frontnet_start_game_maintain(struct GuiButton *gbtn)
{
    TbBool enabled = network_is_host() && net_number_of_enum_players > 1 && !net_player_colours_conflict();
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * enabled) & LbBtnF_Enabled;
}

TbBool frontnet_start_input(void)
{
    if (game_is_busy_doing_gui_string_input()) {
        return false;
    }
    if (!LbIsTextInputActive()) {
        LbStartTextInput();
    }
    if (lbInkey == KC_RETURN) {
        if (local_state.mp_message_text[0] != '\0') {
            send_network_chat_message(netstate.my_id, local_state.mp_message_text);
        }
        process_frontend_chat_message(netstate.my_id, local_state.mp_message_text);
        memset(local_state.mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
    } else if (lbInkey == KC_ESCAPE) {
        local_state.mp_message_text[0] = '\0';
    } else if (is_key_pressed(KC_BACK,KMod_DONTCARE)){
        int chpos = strlen(local_state.mp_message_text);
        // Skip UTF-8 continuation bytes so the whole last character is removed
        while ((chpos > 0) && ((local_state.mp_message_text[chpos-1] & 0xc0) == 0x80))
            chpos--;
        if (chpos > 0)
            local_state.mp_message_text[chpos-1] = '\0';
        clear_key_pressed(KC_BACK);
    } else {
        add_input_text_to_message(local_state.mp_message_text, PLAYER_MP_MESSAGE_LEN, frontend_font[1], lobby_chat_text_width);
    }
    lbInkey = KC_UNASSIGNED;
    return true;
}

void frontnet_draw_services_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_service_scroll_offset, frontend_services_menu_items_visible-2, net_number_of_services);
}

void frontnet_session_set_player_name(struct GuiButton *gbtn)
{
    strcpy(net_player_name, tmp_net_player_name);
    strcpy(net_config_info.net_player_name, tmp_net_player_name);
    net_write_config_file();
}

static void draw_tiled_bar(struct GuiButton *gbtn, int left_index, int first_middle_index, int middle_count, int right_index)
{
    int scale = simple_frontend_sprite_height_units_per_px(gbtn, first_middle_index, 100);
    const struct TbSprite *left = get_frontend_sprite(left_index);
    const struct TbSprite *right = get_frontend_sprite(right_index);
    int left_width = left->SWidth * scale / 16;
    int right_width = right->SWidth * scale / 16;
    int middle_width = gbtn->width - left_width - right_width;
    LbSpriteDrawResized(gbtn->scr_pos_x, gbtn->scr_pos_y, scale, left);
    LbSpriteDrawResized(gbtn->scr_pos_x + gbtn->width - right_width, gbtn->scr_pos_y, scale, right);
    struct GraphicsWindow window;
    LbScreenStoreGraphicsWindow(&window);
    LbScreenSetGraphicsWindow(gbtn->scr_pos_x + left_width, gbtn->scr_pos_y, middle_width, gbtn->height);
    int x = 0;
    for (int tile = 0; x < middle_width; tile++) {
        const struct TbSprite *middle = get_frontend_sprite(first_middle_index + tile % middle_count);
        LbSpriteDrawResized(x, 0, scale, middle);
        x += max(1, middle->SWidth * scale / 16);
    }
    LbScreenLoadGraphicsWindow(&window);
}

void frontnet_draw_text_bar(struct GuiButton *gbtn)
{
    draw_tiled_bar(gbtn, GFS_largearea_nx1_cor_l, GFS_largearea_nx1_tx5_c, 1, GFS_largearea_nx1_cor_r);
}

void frontnet_session_up(struct GuiButton *gbtn)
{
    if (net_session_scroll_offset > 0)
      net_session_scroll_offset--;
}

void frontnet_session_down(struct GuiButton *gbtn)
{
    if (net_session_scroll_offset < net_number_of_sessions - frontend_sessions_menu_items_visible) {
        net_session_scroll_offset++;
    }
}

void frontnet_session_scroll(struct GuiButton *gbtn)
{
    net_session_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_sessions_menu_items_visible-1, net_number_of_sessions);
}

void frontnet_draw_sessions_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_session_scroll_offset, frontend_sessions_menu_items_visible-1, net_number_of_sessions);
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
    RendererSetDrawFlags(0);
    LbTextSetFont(frontend_font[i]);
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / (2*LbTextLineHeight());
    const struct TbSprite *spr;
    spr = get_frontend_sprite(GFS_bullfrog_red_med);
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / (2*(spr->SHeight*13/8));
    int height;
    height = LbTextLineHeight() * tx_units_per_px / 16;
    long netplyr_idx;
    int shift_y;
    netplyr_idx = net_player_scroll_offset;
    for (shift_y=0; shift_y < gbtn->height; shift_y += height, netplyr_idx++)
    {
        const char *text;
        text = net_player[netplyr_idx].name;
        if (netplyr_idx >= net_number_of_enum_players)
            break;
        spr = get_frontend_sprite(GFS_bullfrog_red_med+netplyr_idx);
        i = height - spr->SHeight * fs_units_per_px / 16;
        LbSpriteDrawResized(gbtn->scr_pos_x, gbtn->scr_pos_y + shift_y + abs(i)/2, fs_units_per_px, spr);
        LbTextSetWindow(gbtn->scr_pos_x, shift_y + gbtn->scr_pos_y, gbtn->width - spr->SWidth * fs_units_per_px / 16, height);
        LbTextDrawResized(spr->SWidth * fs_units_per_px / 16, 0, tx_units_per_px, text);
    }
}

void frontnet_session_join(struct GuiButton *gbtn)
{
    if (gbtn != NULL) {
        int index = gbtn->content.lval + net_session_scroll_offset - 45;
        if (index < 0 || index >= net_number_of_sessions || net_session[index] == NULL) {
            return;
        }
        net_session_index_active = index;
        net_session_index_active_id = net_session[index]->id;
    }
    if (net_session_index_active < 0 || net_session_index_active >= net_number_of_sessions || net_session[net_session_index_active] == NULL) {
        return;
    }
    struct TbNetworkSessionNameEntry *session = net_session[net_session_index_active];
    enum NetConnectionRole requested_role = net_join_role;
    net_join_role = NetRole_Player;
    enum NetJoinRejection reason = net_session_join_rejection(session);
    if (session->phase == NetPhase_InGame) {
        net_join_role = NetRole_Spectator;
        reason = net_session_spectator_rejection(session);
    } else if (gbtn == NULL && requested_role == NetRole_Spectator && session->phase == NetPhase_Unknown) {
        net_join_role = NetRole_Spectator;
        reason = NetJoin_Accepted;
    }
    const char *error = net_join_error_text(reason);
    if (error) {
        create_frontend_error_box(error);
        return;
    }
    int32_t plyr_num = network_session_join();
    if (plyr_num < 0) {
        return;
    }
    if (net_join_role == NetRole_Spectator) {
        frontend_set_player_number(0);
        fe_computer_players = 0;
        frontend_set_state(FeSt_START_MPLEVEL);
        return;
    }
    frontend_set_player_number(plyr_num);
    frontend_set_state(FeSt_NET_START);
}

void frontnet_return_to_main_menu(struct GuiButton *gbtn)
{
  if ( LbNetwork_Stop() )
  {
    ERRORLOG("LbNetwork_Stop() failed");
    return;
  }
  frontend_set_state(FeSt_MAIN_MENU);
}

static void build_frontend_dimmed_remap(unsigned char *remap, int red_sixteenths, int green_sixteenths, int blue_sixteenths)
{
    for (int i = 0; i < PALETTE_COLORS; i++) {
        int intensity = max(frontend_palette[3*i], max(frontend_palette[3*i+1], frontend_palette[3*i+2]));
        remap[i] = LbPaletteFindColour(frontend_palette, intensity * red_sixteenths / 16, intensity * green_sixteenths / 16, intensity * blue_sixteenths / 16);
    }
}

void frontnet_draw_player_icon(NetUserId user, int pos_x, int pos_y, int units_per_px)
{
    static unsigned char shadow_remap[PALETTE_COLORS];
    static TbBool shadow_remap_initialized;
    if (!shadow_remap_initialized) {
        build_frontend_dimmed_remap(shadow_remap, 3, 3, 3);
        shadow_remap_initialized = true;
    }
    if (net_player_colour[user] < 0) {
        LbSpriteDrawResizedRemap(pos_x, pos_y, units_per_px, get_frontend_sprite(GFS_bullfrog_blue_med), shadow_remap);
        return;
    }
    if (net_player_colour[user] == NET_PLAYER_COLOUR_RANDOM) {
        LbSpriteDrawResized(pos_x, pos_y, units_per_px, get_frontend_sprite(GFS_random_colour));
        return;
    }
    LbSpriteDrawResized(pos_x, pos_y, units_per_px, get_frontend_sprite(GFS_bullfrog_red_med + net_player_colour[user]));
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

void frontnet_draw_tab_bar(struct GuiButton *gbtn)
{
    draw_tiled_bar(gbtn, GFS_hugearea_thc_cor_tl, GFS_hugearea_thc_tx1_tc, 1, GFS_hugearea_thc_cor_tr);
}

void frontnet_messages_scroll(struct GuiButton *gbtn)
{
    net_message_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), 0, net_number_of_messages);
}

void frontnet_draw_messages_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, net_message_scroll_offset, 0, net_number_of_messages);
}

void frontnet_draw_scroll_selection_box(struct GuiButton *gbtn, long font_idx, const char *text)
{
    unsigned char height;
    const struct TbSprite *spr = get_frontend_sprite(GFS_largearea_xts_cor_l);
    int fs_units_per_px = gbtn->height * 16 / spr->SHeight;
    draw_tiled_bar(gbtn, GFS_largearea_xts_cor_l, GFS_largearea_xts_tx1_c, GFS_largearea_xts_cor_r - GFS_largearea_xts_tx1_c, GFS_largearea_xts_cor_r);

    if (text != NULL)
    {
        LbTextSetFont(frontend_font[font_idx]);
        RendererSetDrawFlags(0);
        int tx_units_per_px;
        tx_units_per_px = (gbtn->height*13/14) * 16 / LbTextLineHeight();
        height = LbTextLineHeight() * tx_units_per_px / 16;
        LbTextSetWindow(gbtn->scr_pos_x + 13*fs_units_per_px/16, gbtn->scr_pos_y, gbtn->width - 26*fs_units_per_px/16, height);
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
    if (player_invalid(player)) {
        return;
    }

    // Prepare text buffer and font
    snprintf(text, sizeof(text), "%s%s", local_state.mp_message_text, print_with_cursor?"_":"");
    font_idx = frontend_button_caption_font(gbtn, 0);
    // And draw the message
    frontnet_draw_scroll_selection_box(gbtn, font_idx, text);
}

void frontnet_draw_messages(struct GuiButton *gbtn)
{
    static unsigned char player_left_remap[PALETTE_COLORS];
    static int player_left_remap_initialized;
    if (!player_left_remap_initialized) {
        build_frontend_dimmed_remap(player_left_remap, 10, 6, 4);
        player_left_remap_initialized = 1;
    }
    int font_idx;
    font_idx = frontend_button_caption_font(gbtn, 0);
    LbTextSetFont(frontend_font[font_idx]);
    RendererSetDrawFlags(0);
    // While setting scale, aim for the visible number of message lines
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / (frontend_messages_items_visible * LbTextLineHeight());
    const struct TbSprite *spr;
    spr = get_frontend_sprite(GFS_bullfrog_red_med);
    int fs_units_per_px;
    fs_units_per_px = gbtn->height * 16 / (frontend_messages_items_visible * (spr->SHeight*13/8));
    int font_height;
    font_height = LbTextLineHeight() * tx_units_per_px / 16;
    int icon_width = spr->SWidth * fs_units_per_px / 16;
    char sender_prefix[NET_MESSAGE_LEN];
    snprintf(sender_prefix, sizeof(sender_prefix), "%s: ", net_user_info[netstate.my_id].name);
    lobby_chat_text_width = (gbtn->width - icon_width) * 16 / tx_units_per_px - LbTextStringWidth(sender_prefix);
    int y;
    y = 0;
    int netmsg_id;
    for (netmsg_id=net_message_scroll_offset; netmsg_id < net_number_of_messages; netmsg_id++)
    {
        if (y + font_height/2 > gbtn->height)
            break;
        struct NetMessage *nmsg;
        nmsg = &net_message[netmsg_id];
        TbBool sender_active = nmsg->connection_id == net_user_info[nmsg->plyr_idx].connection_id && net_player_colour[nmsg->plyr_idx] >= 0;
        int icon_y = font_height - spr->SHeight * fs_units_per_px / 16;
        if (sender_active) {
            frontnet_draw_player_icon(nmsg->plyr_idx, gbtn->scr_pos_x, y + gbtn->scr_pos_y + (icon_y >> 1), fs_units_per_px);
        } else {
            lbSpriteReMapPtr = player_left_remap;
            RendererSetDrawFlags(Lb_TEXT_REMAP);
            LbSpriteDrawResizedRemap(gbtn->scr_pos_x, y + gbtn->scr_pos_y + (icon_y >> 1), fs_units_per_px, spr, player_left_remap);
        }

        LbTextSetWindow(gbtn->scr_pos_x, y + gbtn->scr_pos_y, gbtn->width, min(font_height, gbtn->height-y));
        LbTextDrawResized(icon_width, 0, tx_units_per_px, nmsg->text);
        RendererSetDrawFlags(0);

        y += font_height;
    }
}

void frontnet_return_to_session_menu(struct GuiButton *gbtn)
{
    if (LbNetwork_Stop()) {
        ERRORLOG("LbNetwork_Stop() failed");
    }
    FrontendMenuState nstate;
    nstate = get_menu_state_when_back_from_substate(FeSt_NET_START);
    if (nstate == FeSt_NET_SESSION)
    {
        // If the parent state is network session state, try to stay in net service
        if (!setup_old_network_service()) {
            nstate = get_menu_state_when_back_from_substate(nstate);
        }
    }
    frontend_set_state(nstate);
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
    if (net_service_scroll_offset < net_number_of_services-frontend_services_menu_items_visible+1)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontnet_service_scroll(struct GuiButton *gbtn)
{
    net_service_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_services_menu_items_visible-2, net_number_of_services);
}

void frontnet_service_up(struct GuiButton *gbtn)
{
    if (net_service_scroll_offset > 0)
      net_service_scroll_offset--;
}

void frontnet_service_down(struct GuiButton *gbtn)
{
    if (net_service_scroll_offset < net_number_of_services-frontend_services_menu_items_visible+1)
        net_service_scroll_offset++;
}

void frontnet_service_maintain(struct GuiButton *gbtn)
{
    int srvidx;
    srvidx = gbtn->content.lval + net_service_scroll_offset - 45;
    if (srvidx < net_number_of_services)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontnet_draw_service_button(struct GuiButton *gbtn)
{
  int srvidx;
  // Find and verify selected network service
  srvidx = gbtn->content.lval + net_service_scroll_offset - 45;
  if (srvidx >= net_number_of_services)
    return;
  // Select font to draw
  int font_idx;
  font_idx = frontend_button_caption_font(gbtn,frontend_mouse_over_button);
  LbTextSetFont(frontend_font[font_idx]);
  RendererSetDrawFlags(Lb_TEXT_HALIGN_LEFT);
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
  srvidx = gbtn->content.lval + net_service_scroll_offset - 45;
  if ( ((local_system_flags & GSF_AllowOnePlayer) != 0)
     && (srvidx+1 >= net_number_of_services) )
  {
      frontend_set_player_number(default_loc_player);
      fe_network_active = 0;
      net_service_index_selected = FrontendNetSvc_Skirmish;
      frontend_set_state(FeSt_MP_MAPPACK_SELECT);
  } else
  if (srvidx < 0)
  {
      frontend_set_state(FeSt_NET_SERVICE);
  } else
  {
      setup_network_service(srvidx);
  }
}

/******************************************************************************/
