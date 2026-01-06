/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_tabs.c
 *     Main in-game GUI, visible during gameplay.
 * @par Purpose:
 *     Functions to show and maintain tabbed menu appearing ingame.
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
#include "pre_inc.h"
#include "frontmenu_ingame_tabs.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_keybrd.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "thing_doors.h"
#include "creature_control.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "creature_states_rsrch.h"
#include "creature_instances.h"
#include "config_strings.h"
#include "config_creature.h"
#include "config_magic.h"
#include "config_trapdoor.h"
#include "config_terrain.h"
#include "config_spritecolors.h"
#include "engine_render.h"
#include "room_workshop.h"
#include "room_list.h"
#include "gui_frontbtns.h"
#include "gui_parchment.h"
#include "gui_draw.h"
#include "packets.h"
#include "magic_powers.h"
#include "player_computer.h"
#include "player_instances.h"
#include "config_players.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_opts.h"
#include "frontmenu_ingame_map.h"
#include "frontend.h"
#include "front_input.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "vidfade.h"
#include "kjm_input.h"
#include "custom_sprites.h"
#include "sprites.h"
#include "post_inc.h"
#include "room_workshop.h"

struct Around const draw_square[] = {
{ 0, 0},
{ 1, 0},{ 1, 1},{ 0, 1},{-1, 1},{-1, 0},{-1,-1},{ 0,-1},
{ 1,-1},{ 2,-1},{ 2, 0},{ 2, 1},{ 2, 2},{ 1, 2},{ 0, 2},
{-1, 2},{-2, 2},{-2, 1},{-2, 0},{-2,-1},{-2,-2},{-1,-2},
{ 0,-2},{ 1,-2},{ 2,-2},{ 3,-2},{ 3,-1},{ 3, 0},{ 3, 1},
{ 3, 2},{ 3, 3},{ 2, 3},{ 1, 3},{ 0, 3},{-1, 3},{-2, 3}
};

const short pixels_needed[] = {
    1,
    1,
    AROUND_2x2_PIXEL,
    AROUND_3x3_PIXEL,
    AROUND_4x4_PIXEL,
    AROUND_5x5_PIXEL,
    AROUND_6x6_PIXEL,
};

int32_t activity_list[24];
char gui_room_type_highlighted;
char gui_door_type_highlighted;
char gui_trap_type_highlighted;
char gui_creature_type_highlighted;
unsigned long first_person_instance_top_half_selected;

static unsigned char info_page;
/******************************************************************************/
/******************************************************************************/
static PlayerNumber info_panel_pos_to_player_number(int idx)
{
    if(idx == 0)
        return my_player_number;

    unsigned char current_players_count = 0;

    idx += info_page * 3;
    for (size_t i = 0; i < PLAYERS_COUNT; i++)
    {
        if(i == my_player_number || player_is_roaming(i))
            continue;

        struct PlayerInfo* player = get_player(i);
        if(player_exists(player))
            current_players_count++;

        if(idx == current_players_count)
          return i;
    }
    return -1;
}

short scale_pixel(long basic_zoom)
{
    short pixels_per_map_dot = 5;
    if (basic_zoom >= ONE_PIXEL)
    {
        pixels_per_map_dot = 1;
    }
    else if (basic_zoom >= TWO_PIXELS)
    {
        pixels_per_map_dot = 2;
    }
    else if (basic_zoom >= THREE_PIXELS)
    {
        pixels_per_map_dot = 3;
    }
    else if (basic_zoom >= FOUR_PIXELS)
    {
        pixels_per_map_dot = 4;
    } // 128 = 5

    short draw_pixels = scale_fixed_DK_value(pixels_per_map_dot) * 2 / 5;
    if (draw_pixels > 6)
    {
        draw_pixels = 6; // We just support 6 pixels for now
    }
    return draw_pixels;
}

short get_pixels_scaled_and_zoomed(long basic_zoom)
{
    short draw_pixels = scale_pixel(basic_zoom);
    return pixels_needed[draw_pixels];
}

void gui_zoom_in(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    if (player->minimap_zoom > 128) {
        set_players_packet_action(player, PckA_SetMinimapConf, player->minimap_zoom >> 1, 0, 0, 0);
    }
}

void gui_zoom_out(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    if (player->minimap_zoom < 2048) {
        set_players_packet_action(player, PckA_SetMinimapConf, player->minimap_zoom << 1, 0, 0, 0);
    }
}

void gui_go_to_map(struct GuiButton *gbtn)
{
    zoom_to_parchment_map();
}

void gui_turn_on_autopilot(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    if (player->victory_state != VicS_LostLevel)
    {
      set_players_packet_action(player, PckA_ToggleComputer, 0, 0, 0, 0);
    }
}

void menu_tab_maintain(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    if (player->victory_state != VicS_LostLevel)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

/**
 * Returns tab designation ID if the button with given designation ID is within a tab.
 * @param btn_designt_id
 */
short button_designation_to_tab_designation(short btn_designt_id)
{
    if ((btn_designt_id >= BID_QRY_IMPRSN) && (btn_designt_id <= BID_QRY_BTN3))
        return BID_INFO_TAB;
    if ( ((btn_designt_id >= BID_ROOM_TD01) && (btn_designt_id <= BID_ROOM_TD16)) || ((btn_designt_id >= BID_ROOM_TD17) && (btn_designt_id <= BID_ROOM_TD32)) )
        return BID_ROOM_TAB;
    if ( ((btn_designt_id >= BID_POWER_TD01) && (btn_designt_id <= BID_POWER_TD16)) || ((btn_designt_id >= BID_POWER_TD17) && (btn_designt_id <= BID_POWER_TD32)) )
        return BID_SPELL_TAB;
    if ( ((btn_designt_id >= BID_MNFCT_TD01) && (btn_designt_id <= BID_MNFCT_TD16)) || ((btn_designt_id >= BID_MNFCT_TD17) && (btn_designt_id <= BID_MNFCT_TD32)) )
        return BID_MNFCT_TAB;
    if ((btn_designt_id >= BID_CRTR_NXWNDR) && (btn_designt_id <= BID_CRTR_NXFIGT))
        return BID_CREATR_TAB;
    return BID_DEFAULT;
}

/**
 * Converts button group and item index into designation ID.
 * To be used for referencing interface items within scripts.
 *
 * @param btn_group Group definition, from IngameButtonGroupIDs.
 * @param btn_item Item definition within group, may be room index, manufacture index, power index or just button within group index.
 */
short get_button_designation(short btn_group, short btn_item)
{
    int i;
    int n;
    struct GuiButtonInit * ibtn;
    switch (btn_group)
    {
    case GID_MINIMAP_AREA:
        return BID_MAP_ZOOM_FS+btn_item-1;
    case GID_TABS_AREA:
        return BID_INFO_TAB+btn_item-1;
    case GID_INFO_PANE:
        return BID_QRY_IMPRSN+btn_item-1;
    case GID_ROOM_PANE:
        switch (btn_item)
        {
        case TERRAIN_ITEMS_MAX+1:
            return BID_ROOM_TD16;
        }
        for (i=0; i < 4*4; i++)
        {
            ibtn = &room_menu.buttons[i];
            if (ibtn->content.lval == btn_item)
                return ibtn->id_num;
        }
        for (i=0; i < 4*4; i++)
        {
            ibtn = &room_menu2.buttons[i];
            if (ibtn->content.lval == btn_item)
                return ibtn->id_num;
        }
        break;
    case GID_POWER_PANE:
        for (i=0; i < 4*4; i++)
        {
            ibtn = &spell_menu.buttons[i];
            if (ibtn->content.lval == btn_item)
                return ibtn->id_num;
        }
        for (i=0; i < 4*4; i++)
        {
            ibtn = &spell_menu2.buttons[i];
            if (ibtn->content.lval == btn_item)
                return ibtn->id_num;
        }
        break;
    case GID_TRAP_PANE:
        switch (btn_item)
        {
        case TRAPDOOR_TYPES_MAX+1:
            return BID_MNFCT_TD10;
        }
        n = get_manufacture_data_index_for_thing(TCls_Trap, btn_item);
        for (i=0; i < 4*4; i++)
        {
            ibtn = &trap_menu.buttons[i];
            if (ibtn->content.lval == n)
                return ibtn->id_num;
        }
        for (i=0; i < 4*4; i++)
        {
            ibtn = &trap_menu2.buttons[i];
            if (ibtn->content.lval == n)
                return ibtn->id_num;
        }
        break;
    case GID_DOOR_PANE:
        switch (btn_item)
        {
        case TRAPDOOR_TYPES_MAX+1:
            return BID_MNFCT_TD10;
        }
        n = get_manufacture_data_index_for_thing(TCls_Door, btn_item);
        for (i=0; i < 4*4; i++)
        {
            ibtn = &trap_menu.buttons[i];
            if (ibtn->content.lval == n)
                return ibtn->id_num;
        }
        for (i=0; i < 4*4; i++)
        {
            ibtn = &trap_menu2.buttons[i];
            if (ibtn->content.lval == n)
                return ibtn->id_num;
        }
        break;
    case GID_CREATR_PANE:
        return BID_CRTR_NXWNDR+btn_item-1;
    case GID_MESSAGE_AREA:
        return BID_MSG_EV01+btn_item-1;
    }
    return BID_DEFAULT;
}

void gui_area_autopilot_button(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    int spr_idx = gbtn->sprite_idx;
    if (gbtn->gbtype == LbBtnT_ToggleBtn) {
        ERRORLOG("Cycle button cannot have a normal button draw function!");
    }
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, spr_idx, 100);
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        if ((dungeon->computer_enabled & 0x01) != 0)
        {
          if ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate)
            spr_idx += 2;
        }
        if ((gbtn->button_state_left_pressed == 0) && (gbtn->button_state_right_pressed == 0))
          spr_idx += 1;
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
    }
    else
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    }
}

void maintain_turn_on_autopilot(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    struct Computer2* comp = get_computer_player(player->id_number);
    unsigned long cplr_model = comp->model;
    if (cplr_model < comp_player_conf.computers_count) {
        struct ComputerType* cpt = get_computer_type_template(cplr_model);
        gbtn->tooltip_stridx = cpt->tooltip_stridx;
    } else {
        ERRORLOG("Illegal computer player model %d",(int)cplr_model);
    }
}

void gui_choose_room(struct GuiButton *gbtn)
{
    // prepare to enter room build mode
    activate_room_build_mode(gbtn->content.lval, gbtn->tooltip_stridx);
}

void gui_area_event_button(struct GuiButton *gbtn)
{
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_message_rpanel_msg_questn_act, 100);
        struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
        unsigned long i = gbtn->content.lval;
        if ((gbtn->button_state_left_pressed) || (gbtn->button_state_right_pressed))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
        } else
        if ((i <= EVENT_BUTTONS_COUNT) && (dungeon->event_button_index[i] == dungeon->visible_event_idx))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
        } else
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx+1);
        }
    }
}

#define BAR_FULL_WIDTH 32
void gui_area_progress_bar_short(struct GuiButton *gbtn, int units_per_px, int progress, int total)
{
    int bar_fill = BAR_FULL_WIDTH;
    if (progress < 0) {
        progress = 0;
    } else
    if (progress > total) {
        progress = total;
    }
    if (total > 0)
    {
        bar_fill = 2 * ((BAR_FULL_WIDTH/2) - ((BAR_FULL_WIDTH/2) * progress / total));
        if (bar_fill < 0) {
            bar_fill = 0;
        } else
        if (bar_fill > BAR_FULL_WIDTH) {
            bar_fill = BAR_FULL_WIDTH;
        }
    }
    int bar_fill_scaled = (bar_fill * units_per_px + units_per_px / 2) / 16;
    int bar_whole_scaled = (BAR_FULL_WIDTH * units_per_px + units_per_px / 2) / 16;
    LbDrawBox(gbtn->scr_pos_x + (22*units_per_px + 16/2)/16 + bar_whole_scaled - bar_fill_scaled,
              gbtn->scr_pos_y + (8*units_per_px + 16/2)/16,
              bar_fill_scaled, (8*units_per_px + units_per_px/2)/16, colours[0][0][0]);
}
#undef BAR_FULL_WIDTH

#define BAR_FULL_WIDTH 42
void gui_area_progress_bar_med1(struct GuiButton *gbtn, int units_per_px, int progress, int total)
{
    int bar_fill = BAR_FULL_WIDTH;
    if (progress < 0) {
        progress = 0;
    } else
    if (progress > total) {
        progress = total;
    }
    if (total > 0)
    {
        bar_fill = 2 * ((BAR_FULL_WIDTH/2) - ((BAR_FULL_WIDTH/2) * progress / total));
        if (bar_fill < 0) {
            bar_fill = 0;
        } else
        if (bar_fill > BAR_FULL_WIDTH) {
            bar_fill = BAR_FULL_WIDTH;
        }
    }
    int bar_fill_scaled = (bar_fill * units_per_px + units_per_px / 2) / 16;
    int bar_whole_scaled = (BAR_FULL_WIDTH * units_per_px + units_per_px / 2) / 16;
    LbDrawBox(gbtn->scr_pos_x + (72*units_per_px + 16/2)/16 + bar_whole_scaled - bar_fill_scaled,
              gbtn->scr_pos_y + (12*units_per_px + 16/2)/16,
              bar_fill_scaled, (6*units_per_px + units_per_px/2)/16, colours[0][0][0]);
}
#undef BAR_FULL_WIDTH

#define BAR_FULL_WIDTH 48
void gui_area_progress_bar_med2(struct GuiButton *gbtn, int units_per_px, int progress, int total)
{
    int bar_fill = BAR_FULL_WIDTH;
    if (progress < 0) {
        progress = 0;
    } else
    if (progress > total) {
        progress = total;
    }
    if (total > 0)
    {
        bar_fill = 2 * ((BAR_FULL_WIDTH/2) - ((BAR_FULL_WIDTH/2) * progress / total));
        if (bar_fill < 0) {
            bar_fill = 0;
        } else
        if (bar_fill > BAR_FULL_WIDTH) {
            bar_fill = BAR_FULL_WIDTH;
        }
    }
    int bar_fill_scaled = (bar_fill * units_per_px + units_per_px / 2) / 16;
    int bar_whole_scaled = (BAR_FULL_WIDTH * units_per_px + units_per_px / 2) / 16;
    LbDrawBox(gbtn->scr_pos_x + (4*units_per_px + 16/2)/16 + bar_whole_scaled - bar_fill_scaled,
              gbtn->scr_pos_y + (4*units_per_px + 16/2)/16,
              bar_fill_scaled, (16*units_per_px + units_per_px/2)/16, colours[0][0][0]);
}
#undef BAR_FULL_WIDTH

#define BAR_FULL_WIDTH 96
void gui_area_progress_bar_wide(struct GuiButton *gbtn, int units_per_px, int progress, int total)
{
    int bar_fill = BAR_FULL_WIDTH;
    if (progress < 0) {
        progress = 0;
    } else
    if (progress > total) {
        progress = total;
    }
    if (total > 0)
    {
        bar_fill = 2 * ((BAR_FULL_WIDTH/2) - ((BAR_FULL_WIDTH/2) * progress / total));
        if (bar_fill < 0) {
            bar_fill = 0;
        } else
        if (bar_fill > BAR_FULL_WIDTH) {
            bar_fill = BAR_FULL_WIDTH;
        }
    }
    int bar_fill_scaled = (bar_fill * units_per_px + units_per_px / 2) / 16;
    int bar_whole_scaled = (BAR_FULL_WIDTH * units_per_px + units_per_px / 2) / 16;
    LbDrawBox(gbtn->scr_pos_x + (28*units_per_px + 16/2)/16 + bar_whole_scaled - bar_fill_scaled,
              gbtn->scr_pos_y + (12*units_per_px + 16/2)/16,
              bar_fill_scaled, (8*units_per_px + units_per_px/2)/16, colours[0][0][0]);
}
#undef BAR_FULL_WIDTH

void gui_remove_area_for_rooms(struct GuiButton *gbtn)
{
    game.chosen_room_kind = 0;
    game.chosen_room_spridx = 0;
    game.chosen_room_tooltip = 0;
    struct Packet* pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_SetPlyrState, PSt_Sell, 0, 0, 0);
}

long find_room_type_capacity_total_percentage(PlayerNumber plyr_idx, RoomKind rkind)
{
    int used_cap = 0;
    int total_cap = 0;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long i = dungeon->room_list_start[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        used_cap += room->used_capacity;
        total_cap += room->total_capacity;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    if (total_cap < 1)
        return -1;
    return (used_cap << 8) / total_cap;
}

void gui_area_big_room_button(struct GuiButton *gbtn)
{
    RoomKind rkind = gbtn->content.lval;
    struct PlayerInfo* player = get_my_player();

    struct Dungeon* dungeon = get_players_dungeon(player);

    unsigned short flg_mem = lbDisplay.DrawFlags;
    int units_per_px = (gbtn->width * 16 + 126 / 2) / 126;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, GPS_rpanel_frame_wide_empty, 100);

    if (rkind == RoK_NONE) {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_wide_empty);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    int i = find_room_type_capacity_total_percentage(player->id_number, rkind);
    if ((rkind == RoK_ENTRANCE) || (rkind == RoK_DUNGHEART) || (i < 0))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_wide_empty);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_wide_wbar);
        gui_area_progress_bar_med1(gbtn, units_per_px, i, 256);
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;

    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    unsigned char boxsize = player->boxsize;
    if (boxsize == 0)
    {
        boxsize = 1;
    }
    if ((player->work_state == PSt_BuildRoom) && (boxsize > 1))
    {
        snprintf(gui_textbuf, sizeof(gui_textbuf), "%ld", (long)roomst->cost * boxsize);
    }
    else
    {
        snprintf(gui_textbuf, sizeof(gui_textbuf), "%ld", (long)roomst->cost);
    }
    if (player->render_roomspace.total_roomspace_cost <= dungeon->total_money_owned)
    {
        if ((player->work_state == PSt_BuildRoom) && (player->chosen_room_kind == game.chosen_room_kind)
          && ((game.play_gameturn % (2 * gui_blink_rate)) < gui_blink_rate))
        {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx, 44);
        } else {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx);
        }
        // We will use a special coding for our "string" - we want chars to represent
        // sprite index directly, without code pages and multibyte chars interpretation
        for (i=0; gui_textbuf[i] != '\0'; i++)
            gui_textbuf[i] -= 120;
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx + 1);
    }
    LbTextUseByteCoding(false);
    int tx_units_per_px = (24 * units_per_pixel_ui) / LbTextLineHeight();
    draw_string64k(gbtn->scr_pos_x + 44*units_per_px/16, gbtn->scr_pos_y + (8 - 6)*units_per_px/16, tx_units_per_px, gui_textbuf);

    long amount = count_player_rooms_of_type(player->id_number, rkind);
    // Note that "@" is "x" in that font
    snprintf(gui_textbuf, sizeof(gui_textbuf), "@%ld", amount);
    draw_string64k(gbtn->scr_pos_x + 40*units_per_px/16, gbtn->scr_pos_y - (14 + 6)*units_per_px/16, tx_units_per_px, gui_textbuf);
    LbTextUseByteCoding(true);
    lbDisplay.DrawFlags = flg_mem;
}

/**
 * Sets a new chosen spell.
 * Fills packet with the previous spell disable action.
 */
void gui_choose_spell(struct GuiButton *gbtn)
{
    //NOTE by Petter: factored out original gui_choose_spell code to choose_spell
    choose_spell(gbtn->content.lval, gbtn->tooltip_stridx);
}

void go_to_next_spell_of_type(PowerKind pwkind, PlayerNumber plyr_idx)
{
    struct Packet* pckt = get_packet(plyr_idx);
    set_packet_action(pckt, PckA_ZoomToSpell, pwkind, 0, 0, 0);
}

void gui_go_to_next_spell(struct GuiButton *gbtn)
{
    PowerKind pwkind = gbtn->content.lval;
    struct PlayerInfo* player = get_my_player();
    go_to_next_spell_of_type(pwkind, player->id_number);
    set_chosen_power(pwkind, gbtn->tooltip_stridx);
}

void gui_area_spell_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_frame_portrt_empty, 128);

    PowerKind pwkind = gbtn->content.lval;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_empty);
    struct Dungeon* dungeon = get_my_dungeon();
    if ((dungeon->magic_resrchable[pwkind]) || (dungeon->magic_level[pwkind] > 0))
    {
        int spr_idx;
        if ((gbtn->flags & LbBtnF_Enabled) != 0)
        {
            const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
            int i = powerst->work_state;
            if (((i == PSt_CallToArms) && player_uses_power_call_to_arms(my_player_number))
             || ((i == PSt_SightOfEvil) && player_uses_power_sight(my_player_number))
             || ((pwkind == PwrK_OBEY) && player_uses_power_obey(my_player_number))) {
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_light);
            }
            GoldAmount price = compute_power_price(dungeon->owner, pwkind, 0);
            spr_idx = gbtn->sprite_idx;
            if (dungeon->total_money_owned < price)
                spr_idx++;
            TbBool drawn = false;
            if ((gbtn->button_state_left_pressed == 0) && (gbtn->button_state_right_pressed == 0))
            {
                if ((((i != PSt_CallToArms) || !player_uses_power_call_to_arms(my_player_number))
                  && ((i != PSt_SightOfEvil) || !player_uses_power_sight(my_player_number)))
                 || ((game.play_gameturn % (2 * gui_blink_rate)) < gui_blink_rate))
                {
                    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
                    drawn = true;
                }
            }
            if (!drawn)
            {
                draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
            }
        } else
        {
            if ((pwkind == PwrK_HOLDAUDNC) && (dungeon->magic_level[pwkind] > 0)) {
                spr_idx = gbtn->sprite_idx + 1;
            } else {
                // Draw a question mark over the button, to indicate it can be researched
                spr_idx = GPS_rpanel_frame_portrt_qmark;
            }
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void gui_choose_special_spell(struct GuiButton *gbtn)
{
    //NOTE by Petter: factored out original gui_choose_special_spell code to choose_special_spell
    //TODO: equivalent to gui_choose_spell now... try merge
    choose_spell((gbtn->content.lval) % POWER_TYPES_MAX, gbtn->tooltip_stridx);
}

void gui_area_big_spell_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int units_per_px = (gbtn->width * 16 + 126 / 2) / 126;

    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, GPS_rpanel_frame_wide_empty, 100);
    PowerKind pwkind = gbtn->content.lval;
    struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
    if (power_model_stats_invalid(powerst))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_wide_empty);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    struct PlayerInfo* player = get_my_player();
    struct Dungeon* dungeon = get_players_dungeon(player);

    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    int pwage = find_spell_age_percentage(player->id_number, pwkind);
    if (((powerst->config_flags & PwCF_HasProgress) != 0) && (pwage >= 0))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_wide_wbar);
        int fill_bar = 42 - (2 * 21 * pwage / 256);
        LbDrawBox(
            gbtn->scr_pos_x + (114 - fill_bar)*units_per_px/16,
            gbtn->scr_pos_y + 12*units_per_px/16,
          fill_bar*units_per_px/16, 6*units_per_px/16, colours[0][0][0]);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_wide_empty);
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;

    GoldAmount price = compute_power_price(dungeon->owner, pwkind, 0);
    char text[16];
    snprintf(text, sizeof(text), "%ld", (long)price);
    if (dungeon->total_money_owned >= price)
    {
        if ((player->work_state == powerst->work_state) && ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate)) {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx, 44);
        } else {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx);
        }
        for (char* c = text; *c != 0; c++)
        {
            *c -= 120;
        }
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx + 1);
    }
    LbTextUseByteCoding(false);
    int tx_units_per_px = (24 * units_per_pixel_ui) / LbTextLineHeight();
    draw_string64k(gbtn->scr_pos_x + 44*units_per_px/16, gbtn->scr_pos_y + (8 - 6)*units_per_px/16, tx_units_per_px, text);
    LbTextUseByteCoding(true);
    lbDisplay.DrawFlags = flg_mem;
}

/**
 * Choose a trap or a door.
 * @param manufctr_idx An index into manufacture data array, beware as this is different from models.
 * @param tooltip_id The tooltip string to display.
 */
void choose_workshop_item(int manufctr_idx, TextStringId tooltip_id)
{
    struct PlayerInfo* player = get_my_player();
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    if (   ((manufctr->tngclass == TCls_Trap) && is_trap_placeable(my_player_number, manufctr->tngmodel))
        || ((manufctr->tngclass == TCls_Door) && is_door_placeable(my_player_number, manufctr->tngmodel))  )
    {
        set_players_packet_action(player, PckA_SetPlyrState, manufctr->work_state,
            manufctr->tngmodel, 0, 0);
    }
    else
    {
        set_players_packet_action(player, PckA_SetPlyrState, PSt_CtrlDungeon, 0, 0, 0);
    }
    game.manufactr_element = manufctr_idx;
    game.manufactr_spridx = manufctr->bigsym_sprite_idx;
    game.manufactr_tooltip = tooltip_id;
}

void gui_choose_workshop_item(struct GuiButton *gbtn)
{
    choose_workshop_item(gbtn->content.lval, gbtn->tooltip_stridx);
}

void go_to_next_trap_of_type(ThingModel tngmodel, PlayerNumber plyr_idx)
{
    struct Thing *thing;
    if (tngmodel >= TRAPDOOR_TYPES_MAX) {
        ERRORLOG("Bad trap kind");
        return;
    }
    unsigned long k = 0;
    static ThingModel seltrap[TRAPDOOR_TYPES_MAX];
    int i = seltrap[tngmodel];
    SYNCDBG(9,"Starting, prev index %d",i);
    {
        if (i != 0) {
            thing = thing_get(i);
        } else {
            thing = INVALID_THING;
        }
        if (!thing_exists(thing) || (thing->class_id != TCls_Trap)) {
            i = get_thing_class_list_head(TCls_Trap);
        } else {
            i = thing->next_of_class;
        }
    }
    seltrap[tngmodel] = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner == plyr_idx) && (thing->model == tngmodel)) {
            seltrap[tngmodel] = thing->index;
            break;
        }
        if (i == 0) {
            i = get_thing_class_list_head(TCls_Trap);
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    i = seltrap[tngmodel];
    if (i > 0) {
        struct Packet* pckt = get_packet(plyr_idx);
        set_packet_action(pckt, PckA_ZoomToTrap, i, 0, 0, 0);
    }
}

void go_to_next_door_of_type(ThingModel tngmodel, PlayerNumber plyr_idx)
{
    struct Thing *thing;
    if (tngmodel >= TRAPDOOR_TYPES_MAX) {
        ERRORLOG("Bad door kind");
        return;
    }
    unsigned long k = 0;
    static ThingModel seldoor[TRAPDOOR_TYPES_MAX];
    int i = seldoor[tngmodel];
    {
        if (i != 0) {
            thing = thing_get(i);
        } else {
            thing = INVALID_THING;
        }
        if (!thing_exists(thing) || (thing->class_id != TCls_Door)) {
            i = get_thing_class_list_head(TCls_Door);
        } else {
            i = thing->next_of_class;
        }
    }
    seldoor[tngmodel] = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner == plyr_idx) && (thing->model == tngmodel)) {
            seldoor[tngmodel] = thing->index;
            break;
        }
        if (i == 0) {
            i = get_thing_class_list_head(TCls_Door);
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    i = seldoor[tngmodel];
    if (i > 0) {
        struct Packet* pckt = get_packet(plyr_idx);
        set_packet_action(pckt, PckA_ZoomToDoor, i, 0, 0, 0);
    }
}

void gui_go_to_next_trap(struct GuiButton *gbtn)
{
    int manufctr_idx = gbtn->content.lval;
    struct PlayerInfo* player = get_my_player();
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    go_to_next_trap_of_type(manufctr->tngmodel, player->id_number);
    game.manufactr_element = manufctr_idx;
    game.manufactr_spridx = manufctr->bigsym_sprite_idx;
    game.manufactr_tooltip = manufctr->tooltip_stridx;
}

void gui_over_trap_button(struct GuiButton *gbtn)
{
    int manufctr_idx = gbtn->content.lval;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    gui_trap_type_highlighted = manufctr->tngmodel;
}

void gui_area_trap_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_frame_portrt_empty, 128);

    int manufctr_idx = gbtn->content.lval;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_empty);
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    // Check if we should draw anything
    if (manufctr->tngclass == TCls_Trap)
    {
        if (!is_trap_buildable(my_player_number, manufctr->tngmodel)
          && !is_trap_placeable(my_player_number, manufctr->tngmodel)
          && !is_trap_built(my_player_number, manufctr->tngmodel)) {
            lbDisplay.DrawFlags = flg_mem;
            return;
        }
    } else
    if (manufctr->tngclass == TCls_Door)
    {
        if (!is_door_buildable(my_player_number, manufctr->tngmodel)
          && !is_door_placeable(my_player_number, manufctr->tngmodel)
          && !is_door_built(my_player_number, manufctr->tngmodel)) {
            lbDisplay.DrawFlags = flg_mem;
            return;
        }
    } else
    {
        SYNCDBG(15,"Invalid manufacture index %d",(int)manufctr_idx);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    // We should draw; maybe just disabled button
    if ((manufctr->tngclass == TCls_Door &&
     is_door_buildable(my_player_number, manufctr->tngmodel) &&
     !is_door_placeable(my_player_number, manufctr->tngmodel) &&
     !is_door_built(my_player_number, manufctr->tngmodel)) ||
    (manufctr->tngclass == TCls_Trap &&
     is_trap_buildable(my_player_number, manufctr->tngmodel) &&
     !is_trap_placeable(my_player_number, manufctr->tngmodel) &&
     !is_trap_built(my_player_number, manufctr->tngmodel)))
    {
        if (gbtn->button_state_left_pressed || gbtn->button_state_right_pressed)
        {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_portrt_qmark, 22);
        } else if (game.manufactr_element == manufctr_idx)
        {
            //Draw here when the trap is selected
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_portrt_qmark, 44);
        } else
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_qmark);
        }
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    // Check how many traps/doors do we have to place
    unsigned int amount;
    switch (manufctr->tngclass)
    {
    case TCls_Trap:
        // If there are traps of that type placed on map
        if (player_has_deployed_trap_of_model(my_player_number, manufctr->tngmodel)) {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_light);
        }
        amount = dungeon->mnfct_info.trap_amount_placeable[manufctr->tngmodel];
        break;
    case TCls_Door:
        // If there are doors of that type placed on map
        if (player_has_deployed_door_of_model(my_player_number, manufctr->tngmodel, -1)) {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_light);
        }
        amount = dungeon->mnfct_info.door_amount_placeable[manufctr->tngmodel];
        break;
    default:
        amount = 0;
        break;
    }
    int i = gbtn->sprite_idx + (amount < 1);
    if (gbtn->button_state_left_pressed || gbtn->button_state_right_pressed)
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, i, 22);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, i);
    }
    lbDisplay.DrawFlags = flg_mem;
}

void gui_go_to_next_door(struct GuiButton *gbtn)
{
    int manufctr_idx = gbtn->content.lval;
    struct PlayerInfo* player = get_my_player();
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    go_to_next_door_of_type(manufctr->tngmodel, player->id_number);
    game.manufactr_element = manufctr_idx;
    game.manufactr_spridx = manufctr->bigsym_sprite_idx;
    game.manufactr_tooltip = manufctr->tooltip_stridx;
}

void gui_over_creature_button(struct GuiButton* gbtn)
{
    SYNCDBG(8, "Starting");
    long i = gbtn->btype_value & LbBFeF_IntValueMask;
    ThingModel crmodel;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list + i) % game.conf.crtr_conf.model_count];
    }
    else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    gui_creature_type_highlighted = crmodel;
}

void gui_over_door_button(struct GuiButton *gbtn)
{
    int manufctr_idx = gbtn->content.lval;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);

    gui_door_type_highlighted = manufctr->tngmodel;
}

void gui_remove_area_for_traps(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    game.manufactr_element = 0;
    game.manufactr_spridx = 0;
    game.manufactr_tooltip = 0;
    set_players_packet_action(player, PckA_SetPlyrState, PSt_Sell, 0, 0, 0);
}

void gui_area_trap_build_info_button(struct GuiButton* gbtn)
{
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, gbtn->sprite_idx, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
}

void gui_area_big_trap_button(struct GuiButton *gbtn)
{
    int manufctr_idx = gbtn->content.lval;
    struct PlayerInfo* player = get_my_player();

    struct Dungeon* dungeon = get_players_dungeon(player);
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    unsigned short flg_mem = lbDisplay.DrawFlags;
    int units_per_px = (gbtn->width * 16 + 126 / 2) / 126;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, GPS_rpanel_frame_wide_empty, 100);

    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_wide_empty);
    if (manufctr_idx == 0) {
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    if (((manufctr->tngclass == TCls_Door) &&
     (!is_door_buildable(my_player_number, manufctr->tngmodel) ||
     is_door_placeable(my_player_number, manufctr->tngmodel) ||
     is_door_built(my_player_number, manufctr->tngmodel))) ||
    ((manufctr->tngclass == TCls_Trap) &&
     (!is_trap_buildable(my_player_number, manufctr->tngmodel) ||
     is_trap_placeable(my_player_number, manufctr->tngmodel) ||
     is_trap_built(my_player_number, manufctr->tngmodel))))
     {
        lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
        unsigned int amount;
        switch (manufctr->tngclass)
        {
        case TCls_Trap:
            amount = dungeon->mnfct_info.trap_amount_placeable[manufctr->tngmodel];
            break;
        case TCls_Door:
            amount = dungeon->mnfct_info.door_amount_placeable[manufctr->tngmodel];
            break;
        default:
            amount = 0;
            break;
        }
        if (dbc_enabled && dbc_initialized)
        {
            snprintf(gui_textbuf, sizeof(gui_textbuf), "x%ld", (long)amount);
        }
        else
        {
            // Note that "@" is "×" in that font
            snprintf(gui_textbuf, sizeof(gui_textbuf), "@%ld", (long)amount);
        }
        if (amount <= 0) {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx + 1);
        } else
        if ((((manufctr->tngclass == TCls_Trap) && (player->chosen_trap_kind == manufctr->tngmodel) && (player->work_state == PSt_PlaceTrap))
        || ((manufctr->tngclass == TCls_Door) && (player->chosen_door_kind == manufctr->tngmodel) && (player->work_state == PSt_PlaceDoor)))
        && ((game.play_gameturn % (2 * gui_blink_rate)) < gui_blink_rate) )
        {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx, 44);
        } else {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx);
        }
        int tx_units_per_px = (24 * units_per_pixel_ui) / LbTextLineHeight();
        draw_string64k(gbtn->scr_pos_x + 44*units_per_px/16, gbtn->scr_pos_y + (8 - 6)*units_per_px/16, tx_units_per_px, gui_textbuf);
        lbDisplay.DrawFlags = flg_mem;
    } else
    {
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
}

void maintain_big_spell(struct GuiButton *gbtn)
{
    long spl_idx = game.chosen_spell_type;
    if ((spl_idx < 0) || (spl_idx >= game.conf.magic_conf.power_types_count)) {
        return;
    }
    gbtn->content.lval = spl_idx;
    gbtn->sprite_idx = game.chosen_spell_spridx;
    gbtn->tooltip_stridx = game.chosen_spell_tooltip;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    if (dungeon->magic_level[spl_idx] > 0) {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_room(struct GuiButton *gbtn)
{
    RoomKind rkind = gbtn->content.lval;
    struct Dungeon* dungeon = get_dungeon(my_player_number);
    if ((rkind < 1) || (rkind >= game.conf.slab_conf.room_types_count)) {
        return;
    }
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(8,"Cannot do; player %d has no dungeon",(int)my_player_number);
        return;
    }
    if (dungeon->room_buildable[rkind] & 1) {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_big_room(struct GuiButton *gbtn)
{
    long rkind = game.chosen_room_kind;
    struct Dungeon* dungeon = get_dungeon(my_player_number);
    if ((rkind < 1) || (rkind >= game.conf.slab_conf.room_types_count)) {
        return;
    }
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(8,"Cannot do; player %d has no dungeon",(int)my_player_number);
        return;
    }
    gbtn->content.lval = rkind;
    gbtn->sprite_idx = game.chosen_room_spridx;
    gbtn->tooltip_stridx = game.chosen_room_tooltip;
    if (dungeon->room_buildable[rkind] & 1) {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_spell(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    long i = gbtn->content.lval;
    if (!is_power_available(player->id_number, i))
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
  } else
  if (i == PwrK_ARMAGEDDON)
  {
      if (game.armageddon_cast_turn != 0)
      {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
      } else
      {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
      }
  } else
  if (i == PwrK_HOLDAUDNC)
  {
      if (player_uses_power_hold_audience(my_player_number))
      {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
      } else
      {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
      }
  } else
  {
    gbtn->btype_value &= LbBFeF_IntValueMask;
    gbtn->flags |= LbBtnF_Enabled;
  }
}

void maintain_trap(struct GuiButton *gbtn)
{
    int manufctr_idx = gbtn->content.lval;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    if (is_trap_placeable(my_player_number, manufctr->tngmodel) || is_trap_built(my_player_number, manufctr->tngmodel))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else if (is_trap_buildable(my_player_number, manufctr->tngmodel))
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_door(struct GuiButton *gbtn)
{
    int manufctr_idx = gbtn->content.lval;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    if (is_door_placeable(my_player_number, manufctr->tngmodel) || is_door_built(my_player_number, manufctr->tngmodel))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else if (is_door_buildable(my_player_number, manufctr->tngmodel))
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_big_trap(struct GuiButton *gbtn)
{
    int manufctr_idx = game.manufactr_element % game.conf.trapdoor_conf.manufacture_types_count;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    gbtn->content.lval = manufctr_idx;
    gbtn->sprite_idx = game.manufactr_spridx;
    gbtn->tooltip_stridx = game.manufactr_tooltip;
    if ( ((manufctr->tngclass == TCls_Trap) && is_trap_placeable(my_player_number, manufctr->tngmodel))
      || ((manufctr->tngclass == TCls_Door) && is_door_placeable(my_player_number, manufctr->tngmodel)) )
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else if ((manufctr->tngclass == TCls_Door &&
     is_door_buildable(my_player_number, manufctr->tngmodel) &&
     !is_door_placeable(my_player_number, manufctr->tngmodel) &&
     !is_door_built(my_player_number, manufctr->tngmodel)) ||
    (manufctr->tngclass == TCls_Trap &&
     is_trap_buildable(my_player_number, manufctr->tngmodel) &&
     !is_trap_placeable(my_player_number, manufctr->tngmodel) &&
     !is_trap_built(my_player_number, manufctr->tngmodel)))
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_buildable_info(struct GuiButton* gbtn)
{
    int manufctr_idx = game.manufactr_element % game.conf.trapdoor_conf.manufacture_types_count;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    gbtn->content.lval = manufctr_idx;
    if (((manufctr->tngclass == TCls_Trap) && (is_trap_placeable(my_player_number, manufctr->tngmodel) || is_trap_built(my_player_number, manufctr->tngmodel)))
        || ((manufctr->tngclass == TCls_Door) && (is_door_placeable(my_player_number, manufctr->tngmodel) || is_door_built(my_player_number, manufctr->tngmodel))))
    {
        gbtn->flags |= LbBtnF_Enabled;
    } else if ((manufctr->tngclass == TCls_Door &&
     is_door_buildable(my_player_number, manufctr->tngmodel) &&
     !is_door_placeable(my_player_number, manufctr->tngmodel) &&
     !is_door_built(my_player_number, manufctr->tngmodel)) ||
    (manufctr->tngclass == TCls_Trap &&
     is_trap_buildable(my_player_number, manufctr->tngmodel) &&
     !is_trap_placeable(my_player_number, manufctr->tngmodel) &&
     !is_trap_built(my_player_number, manufctr->tngmodel)))
    {
        gbtn->flags |= LbBtnF_Enabled;
        gbtn->tooltip_stridx = GUIStr_Empty;
        gbtn->sprite_idx = 0;
    } else
    {
        gbtn->flags &= ~LbBtnF_Enabled;
        gbtn->tooltip_stridx = GUIStr_Empty;
        gbtn->sprite_idx = 0;
        return;
    }
    struct PlayerInfo* player = get_my_player();
    struct Dungeon* dungeon = get_players_dungeon(player);
    //We cannot use the actual manufacture level because that does not update enough.
    int manufacture_level = calculate_manufacture_level(dungeon);

    // Overlay a hammer symbol on the top-right to denote manufacturability
    if (manufctr_idx > 0 ||
    (manufctr->tngclass == TCls_Door &&
    is_door_buildable(my_player_number, manufctr->tngmodel) &&
    !is_door_placeable(my_player_number, manufctr->tngmodel) &&
    !is_door_built(my_player_number, manufctr->tngmodel)) ||
    (manufctr->tngclass == TCls_Trap &&
    is_trap_buildable(my_player_number, manufctr->tngmodel) &&
    !is_trap_placeable(my_player_number, manufctr->tngmodel) &&
    !is_trap_built(my_player_number, manufctr->tngmodel)))
    {
        // Required manufacture level for this item
        TbBool is_buildable = false;
        int required_level = 0;
        switch (manufctr->tngclass)
        {
        case TCls_Trap:
            is_buildable = is_trap_buildable(my_player_number, manufctr->tngmodel);
            struct TrapConfigStats* trapst = get_trap_model_stats(manufctr->tngmodel);
            required_level = trapst->manufct_level;
            break;
        case TCls_Door:
            is_buildable = is_door_buildable(my_player_number, manufctr->tngmodel);
            struct DoorConfigStats* doorst = get_door_model_stats(manufctr->tngmodel);
            required_level = doorst->manufct_level;
            break;
        }

        if (is_buildable)
        {
            if (manufacture_level >= required_level)
            {
                gbtn->sprite_idx = GPS_rpanel_manufacture_std; // If manufacturable and high enough level: lit hammer
                gbtn->tooltip_stridx = GUIStr_TrapAvailable;
            }
            else
            {
                gbtn->tooltip_stridx = GUIStr_TrapWorkshopNeeded;
                gbtn->sprite_idx = GPS_rpanel_manufacture_dis; // If manufacturable and level too low: greyed hammer
            }
        }
        else
        {
            gbtn->tooltip_stridx = GUIStr_TrapUnavailable;
            gbtn->sprite_idx = GPS_rpanel_manufacture_cant; // If owned but not manufacturable: no entry hammer
        }
    }
}

void draw_centred_string64k(const char *text, short x, short y, short base_w, short dst_w)
{
    unsigned long flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    LbTextSetJustifyWindow((x - (dst_w / 2)), y, dst_w);
    LbTextSetClipWindow( (x - (dst_w / 2)), y, dst_w, 16*dst_w/base_w);
    lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    int tx_units_per_px;
    int text_x;
    int text_y = -6*dst_w/base_w;
    if ( (MyScreenHeight < 400) && (dbc_language > 0) )
    {
        tx_units_per_px = scale_ui_value(32);
        text_x = 12;
    }
    else
    {
        tx_units_per_px = (22 * units_per_pixel_ui) / LbTextLineHeight();
        if ( (dbc_language > 0) && (MyScreenWidth > 640) )
        {
            tx_units_per_px = scale_value_by_horizontal_resolution(12 + (MyScreenWidth / 640));
            text_y += 12;
        }
        else
        {
            tx_units_per_px = (22 * units_per_pixel_ui) / LbTextLineHeight();
        }
        text_x = 0;
    }
    LbTextDrawResized(text_x, text_y, tx_units_per_px, text);
    LbTextSetJustifyWindow(0, 0, LbGraphicsScreenWidth());
    LbTextSetClipWindow(0, 0, LbGraphicsScreenWidth(), LbGraphicsScreenHeight());
    LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    lbDisplay.DrawFlags = flg_mem;
}

void draw_name_box(long x, long y, int width, struct Thing *thing)
{
    int ps_units_per_px;
    {
        const struct TbSprite* spr = get_panel_sprite(GPS_rpanel_bar_long_full);
        ps_units_per_px = (width*95/100) * 16 / spr->SWidth;
    }
    draw_gui_panel_sprite_left(x, y, ps_units_per_px, GPS_rpanel_bar_long_full);
    if (thing_is_creature(thing) && (thing->ccontrol_idx > 0))
    {
        // Draw health bar
        struct PlayerInfo* player = get_my_player();
        struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
        struct CreatureControl* cctrl = creature_control_get_from_thing(ctrltng);
        HitPoints maxhealth = cctrl->max_health;
        HitPoints curhealth = ctrltng->health;
        if (curhealth <= 0) {
            curhealth = 0;
        } else
        if (curhealth > maxhealth) {
            curhealth = maxhealth;
        }
        if (maxhealth > 0)
        {
            long i = 63 * curhealth / maxhealth;
            long bar_fill = 126 - 2 * i;
            if (bar_fill < 0) {
                bar_fill = 0;
            } else
            if (bar_fill > 126) {
                bar_fill = 126;
            }
            LbDrawBox(x + ((128-bar_fill)*width + 70)/140, y + (4*width + 70)/140, (bar_fill*width + 70)/140, (14*width + 70)/140, colours[0][0][0]);
        }
        // Draw creature name
        const char* text = creature_own_name(thing);
        draw_centred_string64k(text, x + 63*width/140, y + 2*width/140, 120, 120*width/140);
    }
}

void gui_creature_query_background1(struct GuiMenu *gmnu)
{
    SYNCDBG(19,"Starting");
    int units_per_px = (gmnu->width * 16 + 140 / 2) / 140;
    struct PlayerInfo* player = get_my_player();
    struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
    draw_name_box(gmnu->pos_x + 4*units_per_px/16, gmnu->pos_y + 262*units_per_px/16, gmnu->width, ctrltng);
    int portrt_x = gmnu->pos_x + (4 * units_per_px + 8) / 16;
    int portrt_y = gmnu->pos_y + (188 * units_per_px + 8) / 16;
    if (thing_is_creature(ctrltng) && (ctrltng->ccontrol_idx > 0))
    {
        long spr_idx = get_creature_model_graphics(ctrltng->model, CGI_QuerySymbol);
        if (spr_idx > 0)
        {
            const struct TbSprite* spr = get_button_sprite_for_player(spr_idx, ctrltng->owner);
            int bs_units_per_px = (gmnu->width * 35 / 100) * 16 / spr->SWidth;
            LbSpriteDrawResized(portrt_x + 12 * units_per_px / 16, portrt_y + 12 * units_per_px / 16, bs_units_per_px, spr);
        }
    }
    {
        const struct TbSprite* spr = get_panel_sprite(GPS_rpanel_frame_double_hex_med);
        int ps_units_per_px = (gmnu->width * 52 / 100) * 16 / spr->SWidth;
        draw_gui_panel_sprite_left(portrt_x, portrt_y, ps_units_per_px, GPS_rpanel_frame_double_hex_med);
    }
}

void gui_creature_query_background2(struct GuiMenu *gmnu)
{
    SYNCDBG(19,"Starting");
    int units_per_px = (gmnu->width * 16 + 140 / 2) / 140;
    struct PlayerInfo* player = get_my_player();
    struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
    int nambox_x = gmnu->pos_x + 4 * units_per_px / 16;
    int nambox_y = gmnu->pos_y + 200 * units_per_px / 16;
    draw_name_box(nambox_x, nambox_y, gmnu->width, ctrltng);
    if (thing_is_creature(ctrltng) && (ctrltng->ccontrol_idx > 0))
    {
        long spr_idx = get_creature_model_graphics(ctrltng->model, CGI_HandSymbol);
        if (spr_idx > 0)
        {
            const struct TbSprite* spr = get_panel_sprite(spr_idx);
            int ps_units_per_px = (gmnu->width * 22 / 100) * 16 / spr->SWidth;
            draw_gui_panel_sprite_left_player(nambox_x, nambox_y - 22*units_per_px/16, ps_units_per_px, spr_idx,ctrltng->owner);
        }
    }
}

unsigned short get_creature_pick_flags(TbBool pick_up)
{
    unsigned short pick_flags = pick_up ? TPF_PickableCheck : 0;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
    {
        pick_flags |= TPF_OrderedPick;
    }
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
    {
        pick_flags |= TPF_OrderedPick | TPF_ReverseOrder;
    }
    return pick_flags;
}

void pick_up_creature_doing_activity(struct GuiButton *gbtn)
{
    SYNCDBG(8,"Starting");
    long i = gbtn->btype_value & LbBFeF_IntValueMask;
    ThingModel crmodel;
    if (i > 0)
        crmodel = breed_activities[(top_of_breed_list+i)%game.conf.crtr_conf.model_count];
    else
        crmodel = get_players_special_digger_model(my_player_number);
    // Get index from pointer
    long job_idx = (gbtn->content.lptr - &activity_list[0]);
    unsigned char pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(crmodel, (job_idx & 0x03), my_player_number, pick_flags);
}

void gui_go_to_next_creature_activity(struct GuiButton *gbtn)
{
    ThingModel crmodel;
    int i = gbtn->btype_value & LbBFeF_IntValueMask;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%game.conf.crtr_conf.model_count];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    // Get index from pointer
    int job_idx = (gbtn->content.lptr - &activity_list[0]);
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(crmodel, (job_idx & 0x3), pick_flags);
}

RoomIndex find_my_next_room_of_type(RoomKind rkind)
{
    return find_next_room_of_type(my_player_number, rkind);
}

RoomIndex find_next_room_of_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    static RoomIndex next_room[TERRAIN_ITEMS_MAX];
    if (next_room[rkind] > 0)
    {
        struct Room* room = room_get(next_room[rkind]);
        if (room_exists(room) && (room->owner == plyr_idx) && (room->kind == rkind))
          next_room[rkind] = room->next_of_owner;
        else
          next_room[rkind] = 0;
    }
    if (next_room[rkind] <= 0)
    {
        struct Dungeon* dungeon = get_dungeon(plyr_idx);
        next_room[rkind] = dungeon->room_list_start[rkind];
    }
    return next_room[rkind];
}

void go_to_my_next_room_of_type_and_select(RoomKind rkind)
{
    RoomIndex room_idx = find_my_next_room_of_type(rkind);
    struct PlayerInfo* player = get_my_player();
    if (room_idx > 0) {
        set_players_packet_action(player, PckA_ZoomToRoom, room_idx, 0, 0, 0);
    }
}

void go_to_my_next_room_of_type(RoomKind rkind)
{
    //_DK_go_to_my_next_room_of_type(rkind); return;
    RoomIndex room_idx = find_my_next_room_of_type(rkind);
    struct PlayerInfo* player = get_my_player();
    if (room_idx > 0) {
        struct Room* room = room_get(room_idx);
        set_players_packet_action(player, PckA_ZoomToPosition, subtile_coord_center(room->central_stl_x), subtile_coord_center(room->central_stl_y), 0, 0);
    }
}

void gui_go_to_next_room(struct GuiButton *gbtn)
{
    unsigned long rkind = gbtn->content.lval;
    go_to_my_next_room_of_type_and_select(rkind);
    game.chosen_room_kind = rkind;
    struct RoomConfigStats* roomst = &game.conf.slab_conf.room_cfgstats[rkind];
    game.chosen_room_spridx = roomst->bigsym_sprite_idx;
    game.chosen_room_tooltip = gbtn->tooltip_stridx;
}

void gui_over_room_button(struct GuiButton *gbtn)
{
    //todo support more then 17 rooms
    if (gbtn->content.lval >= 17)
        gui_room_type_highlighted = 0;
    else
        gui_room_type_highlighted = gbtn->content.lval;
}

void gui_area_room_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_frame_portrt_empty, 128);

    RoomKind rkind = gbtn->content.lval;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_empty);
    struct Dungeon* dungeon = get_my_dungeon();
    if ((dungeon->room_buildable[rkind] & 1) // One can build it now
         || (dungeon->room_resrchable[rkind] == 1) // One can research it at any time
         || (dungeon->room_resrchable[rkind] == 2) // One can research it and get instantly then found
         || ((dungeon->room_resrchable[rkind] == 4) && (dungeon->room_buildable[rkind] & 2)) // Player able to research
         )
    {
        if ((gbtn->flags & LbBtnF_Enabled) != 0)
        {
            if (dungeon->room_list_start[rkind] > 0)
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_light);
            int spr_idx = (dungeon->total_money_owned < get_room_kind_stats(rkind)->cost) + gbtn->sprite_idx;
            if ((gbtn->button_state_left_pressed == 0) && (gbtn->button_state_right_pressed == 0)) {
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
            } else {
                draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
            }
        } else
        {
            // Draw a question mark over the button, to indicate it can be researched
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_portrt_qmark);
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void pick_up_next_creature(struct GuiButton *gbtn)
{
    int kind;

    int i = gbtn->btype_value & LbBFeF_IntValueMask;
    if (i > 0) {
        kind = breed_activities[(i + top_of_breed_list) % game.conf.crtr_conf.model_count];
    }
    else {
        kind = get_players_special_digger_model(my_player_number);
    }

    unsigned short pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(kind, CrGUIJob_Any, my_player_number, pick_flags);
}

void gui_go_to_next_creature(struct GuiButton *gbtn)
{
    SYNCDBG(8,"Starting");
    long i = gbtn->btype_value & LbBFeF_IntValueMask;
    ThingModel crmodel;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%game.conf.crtr_conf.model_count];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    unsigned char pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(crmodel, CrGUIJob_Any, pick_flags);
}

void gui_area_anger_button(struct GuiButton *gbtn)
{
    long crmodel;
    SYNCDBG(10,"Starting");
    long i = gbtn->btype_value & LbBFeF_IntValueMask;
    // Get index from pointer
    long job_idx = (gbtn->content.lptr - &activity_list[0]);
    if ( (i > 0) && (top_of_breed_list+i < game.conf.crtr_conf.model_count) )
        crmodel = breed_activities[top_of_breed_list+i];
    else
        crmodel = get_players_special_digger_model(my_player_number);
    // Get scale factor
    int units_per_px = (gbtn->width * 16 + 32 / 2) / 32;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, GPS_rpanel_tab_crtr_annoy_lv00, 113);
    // Now draw the button
    long cr_total = 0;
    if ((crmodel > 0) && (crmodel < game.conf.crtr_conf.model_count) && (gbtn->flags & LbBtnF_Enabled))
    {
        struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
        int spridx = gbtn->sprite_idx;
        if (gbtn->content.lptr != NULL)
        {
          cr_total = *gbtn->content.lptr;
          if (cr_total > 0)
          {
            i = dungeon->guijob_angry_creatrs_count[crmodel][(job_idx & 0x03)];
            if (i > cr_total)
            {
              WARNDBG(7,"Creature %ld stats inconsistency; total=%ld, doing activity%ld=%ld",crmodel,cr_total,(job_idx & 0x03),i);
              i = cr_total;
            }
            if (i < 0)
            {
              i = 0;
            }
            spridx += 14 * i / cr_total;
          }
        }
        if ((gbtn->button_state_left_pressed) || (gbtn->button_state_right_pressed))
        {
          draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y-2*units_per_px/16, ps_units_per_px, spridx, 12);
        } else
        {
          draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y-2*units_per_px/16, ps_units_per_px, spridx);
        }
        if (gbtn->content.lptr != NULL)
        {
          snprintf(gui_textbuf, sizeof(gui_textbuf), "%ld", cr_total);
          // We will use a special coding for our "string" - we want chars to represent
          // sprite index directly, without code pages and multibyte chars interpretation
          if ((cr_total > 0) && (dungeon->guijob_all_creatrs_count[crmodel][(job_idx & 0x03)] ))
          {
              for (i=0; gui_textbuf[i] != '\0'; i++)
                  gui_textbuf[i] -= 120;
          }
          LbTextUseByteCoding(false);
          draw_button_string(gbtn, 32, gui_textbuf);
          LbTextUseByteCoding(true);
        }
    }
    SYNCDBG(12,"Finished");
}

long anger_get_creature_highest_anger_type_and_byte_percentage(struct Thing *creatng, int32_t *out_angr_typ, int32_t *out_angr_prct)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    long angr_lmt = crconf->annoy_level;
    int angr_typ = 0;
    long angr_lvl = 0;
    for (int i = 1; i < 5; i++)
    {
        if (angr_lvl < cctrl->annoyance_level[i])
        {
            angr_lvl = cctrl->annoyance_level[i];
            angr_typ = i;
        }
    }
    if (angr_lmt <= 0)
    {
        *out_angr_prct = 0;
        *out_angr_typ = 0;
        return 0;
    }
    if (angr_lvl < 0) {
        angr_lvl = 0;
    } else
    if (angr_lvl > 2 * angr_lmt) {
        angr_lvl = 2 * angr_lmt;
    }
    // Return value scaled 0..256
    *out_angr_prct = (angr_lvl << 8) / (2 * angr_lmt);
    *out_angr_typ = angr_typ;
    return 1;
}

void gui_area_smiley_anger_button(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    // Get scale factor
    int units_per_px = (gbtn->width * 16 + 56 / 2) / 56;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, gbtn->sprite_idx, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
    struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (thing_is_creature(ctrltng))
    {
        int32_t angr_typ;
        int32_t angr_prct;
        anger_get_creature_highest_anger_type_and_byte_percentage(ctrltng, &angr_typ, &angr_prct);
        int angr_pos = 5 * angr_prct / 256;
        if (angr_pos < 0) {
            angr_pos = 0;
        } else
        if (angr_pos > 4) {
            angr_pos = 4;
        }
        int spr_idx = angr_pos + GPS_symbols_creatr_mood_vhappy_std;
        int shift_x = (48 * angr_prct - 16) / 256;
        if (shift_x < 0) {
            shift_x = 0;
        } else
        if ( shift_x > 36 ) {
            shift_x = 36;
        }
        draw_gui_panel_sprite_left_player(gbtn->scr_pos_x + (shift_x - 12) * units_per_px / 16, gbtn->scr_pos_y - 22 * units_per_px / 16, ps_units_per_px, spr_idx,ctrltng->owner);
    }
}

void gui_area_experience_button(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    int units_per_px = (gbtn->width * 16 + 56 / 2) / 56;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, gbtn->sprite_idx, 100);
    struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (thing_is_creature(ctrltng))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(ctrltng);
        struct CreatureControl* cctrl = creature_control_get_from_thing(ctrltng);
        long points_progress = cctrl->exp_points;
        long points_required = (crconf->to_level[cctrl->exp_level] << 8);
        gui_area_progress_bar_med2(gbtn, units_per_px, points_progress, points_required);
        char text[16];
        snprintf(text, sizeof(text), "%d", (int)(cctrl->exp_level + 1));
        draw_button_string(gbtn, 56, text);
    } else
    if (thing_is_dead_creature(ctrltng))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
        //TODO maybe show some info about dead creature too?
    }
}

void gui_area_instance_button(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    int units_per_px = (gbtn->width * 16 + 60 / 2) / 60;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, GPS_rpanel_bar_with_pic_full_blue_down, 100);
    struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (!thing_is_creature(ctrltng))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_bar_with_pic_full_blue_down);
        gui_area_progress_bar_short(gbtn, units_per_px, 0, 32);
        return;
    }
    int curbtn_avail_pos = gbtn->content.lval;
    int curbtn_inst_id = creature_instance_get_available_id_for_pos(ctrltng, curbtn_avail_pos);
    if (!creature_instance_is_available(ctrltng, curbtn_inst_id))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_bar_with_pic_full_blue_down);
        gui_area_progress_bar_short(gbtn, units_per_px, 0, 32);
        return;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(ctrltng);
    int spr_idx;
    if (cctrl->active_instance_id == curbtn_inst_id) {
      spr_idx = GPS_rpanel_bar_with_pic_full_blue_up;
    } else {
      spr_idx = GPS_rpanel_bar_with_pic_full_blue_down;
    }
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
    struct InstanceInfo* inst_inf = creature_instance_info_get(curbtn_inst_id);
    if (cctrl->instance_id == curbtn_inst_id)
    {
        gui_area_progress_bar_short(gbtn, units_per_px, 0, 32);
    } else
    if (!creature_instance_has_reset(ctrltng, curbtn_inst_id))
    {
        long turns_progress;
        long turns_required;
        if ((ctrltng->alloc_flags & TAlF_IsControlled) != 0) {
            turns_required = inst_inf->fp_reset_time;
        } else {
            turns_required = inst_inf->reset_time;
        }
        turns_progress = (long)game.play_gameturn - (long)cctrl->instance_use_turn[curbtn_inst_id] + cctrl->inst_action_turns - cctrl->inst_total_turns;
        gui_area_progress_bar_short(gbtn, units_per_px, turns_progress, turns_required);
    } else
    {
        gui_area_progress_bar_short(gbtn, units_per_px, 32, 32);
    }

    // Calculating text size.
    int tx_units_per_px = ( (MyScreenHeight < 400) && (dbc_language > 0) ) ? scale_ui_value(32) : (gbtn->height * 11 / 12) * 16 / LbTextLineHeight();
    char text[16];
    snprintf(text, sizeof(text), "%d", (curbtn_avail_pos + 1) % 10);
    LbTextDrawResized(gbtn->scr_pos_x + 52*units_per_px/16, gbtn->scr_pos_y + 9*units_per_px/16, tx_units_per_px, text);
    spr_idx = gbtn->sprite_idx;
    // Show disabled icon if instance is on cooldown or creature is frozen.
    if ((!creature_instance_has_reset(ctrltng, curbtn_inst_id)) || (creature_under_spell_effect(ctrltng, CSAfF_Freeze) && (!inst_inf->instant)))
    {
        spr_idx++;
    }
    if (MyScreenHeight < 400)
    {
        const struct TbSprite* spr = get_panel_sprite(GPS_plyrsym_symbol_player_red_std_b);
        ps_units_per_px = (22 * units_per_pixel) / spr->SHeight;
    }
    draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 8*units_per_px/16, ps_units_per_px, spr_idx);
}

/** Callback function of maintaining creature skill button. */
void maintain_instance(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (!thing_is_creature(ctrltng))
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
        gbtn->sprite_idx = 0;
        gbtn->tooltip_stridx = 0;
        return;
    }
    int curbtn_avail_pos = gbtn->content.lval;
    int curbtn_inst_id = creature_instance_get_available_id_for_pos(ctrltng, curbtn_avail_pos);
    struct InstanceInfo* inst_inf = creature_instance_info_get(curbtn_inst_id);
    gbtn->sprite_idx = inst_inf->symbol_spridx;
    gbtn->tooltip_stridx = inst_inf->tooltip_stridx;
    if (creature_instance_is_available(ctrltng, curbtn_inst_id))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
        return;
    }
    gbtn->btype_value |= LbBFeF_NoTooltip;
    gbtn->flags &= ~LbBtnF_Enabled;
}

void gui_activity_background(struct GuiMenu *gmnu)
{
    SYNCDBG(9,"Starting");
    unsigned short flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    if (no_of_breeds_owned <= 6) {
        top_of_breed_list = 0;
    }
    struct Dungeon* dungeon = get_my_dungeon();
    int visible_count = no_of_breeds_owned;
    if (no_of_breeds_owned <= 1)
      visible_count = 1;
    if (visible_count >= 6)
        visible_count = 6;
    for (int i = 0; i < visible_count; i++)
    {
        ThingModel crmodel;
        if ( (i > 0) && (top_of_breed_list+i < game.conf.crtr_conf.model_count) )
            crmodel = breed_activities[top_of_breed_list+i];
        else
            crmodel = get_players_special_digger_model(my_player_number);
        // Clear activity list
        activity_list[4*i+0] = 0;
        activity_list[4*i+1] = 0;
        activity_list[4*i+2] = 0;
        for (int n = 0; n < STATE_TYPES_COUNT; n++)
        {
            int gui_state_idx = state_type_to_gui_state[n];
            switch (gui_state_idx)
            {
            case CrGUIJob_Wandering:
                activity_list[4*i+0] += dungeon->crmodel_state_type_count[crmodel][n];
                break;
            case CrGUIJob_Working:
                activity_list[4*i+1] += dungeon->crmodel_state_type_count[crmodel][n];
                break;
            case CrGUIJob_Fighting:
                activity_list[4*i+2] += dungeon->crmodel_state_type_count[crmodel][n];
                break;
            default:
                ERRORLOG("Outranged GUI state value %d",(int)gui_state_idx);
                break;
            }
        }
    }
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbDrawBox(gmnu->pos_x + scale_ui_value(2),gmnu->pos_y + scale_ui_value(218),scale_ui_value(134),scale_ui_value(24),colours[0][0][0]);

    lbDisplay.DrawFlags = flg_mem;
}

void maintain_activity_up(struct GuiButton *gbtn)
{
    if (no_of_breeds_owned <= 6)
    {
        gbtn->flags &= ~LbBtnF_Visible;
        gbtn->flags &= ~LbBtnF_Enabled;
    } else
    {
        gbtn->flags |= LbBtnF_Visible;
        gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (top_of_breed_list > 0)) & LbBtnF_Enabled;
    }
    if (wheel_scrolled_up && (is_game_key_pressed(Gkey_SpeedMod, NULL, true)))
    {
        if (top_of_breed_list > 0)
        {
            top_of_breed_list--;
        }
    }
}

void maintain_activity_down(struct GuiButton *gbtn)
{
    if (no_of_breeds_owned <= 6)
    {
        gbtn->flags &= ~LbBtnF_Visible;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
    else
    {
        gbtn->flags |= LbBtnF_Visible;
        gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (no_of_breeds_owned - 6 > top_of_breed_list)) & LbBtnF_Enabled;
    }
    if (wheel_scrolled_down && (is_game_key_pressed(Gkey_SpeedMod, NULL, true)))
    {
        if (top_of_breed_list + 6 < no_of_breeds_owned)
        {
            top_of_breed_list++;
        }
    }
}

void maintain_activity_pic(struct GuiButton *gbtn)
{
    ThingModel crmodel;
    int i = gbtn->btype_value & LbBFeF_IntValueMask;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%game.conf.crtr_conf.model_count];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    struct Dungeon* dungeon = get_my_dungeon();
    /*if (crmodel == get_players_special_digger_model(my_player_number))
      amount = dungeon->num_active_diggers;
    else*/
    int amount = dungeon->owned_creatures_of_model[crmodel];
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (amount > 0)) & LbBtnF_Enabled;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Visible * (no_of_breeds_owned > i)) & LbBtnF_Visible;
    gbtn->sprite_idx = get_creature_model_graphics(crmodel, CGI_HandSymbol);
}

void maintain_activity_row(struct GuiButton *gbtn)
{
    ThingModel crmodel;
    int i = gbtn->btype_value & LbBFeF_IntValueMask;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%game.conf.crtr_conf.model_count];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    struct Dungeon* dungeon = get_my_dungeon();
    /*if (crmodel == get_players_special_digger_model(my_player_number))
      amount = dungeon->num_active_diggers;
    else*/
    int amount = dungeon->owned_creatures_of_model[crmodel];
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (amount > 0)) & LbBtnF_Enabled;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Visible * (no_of_breeds_owned > i)) & LbBtnF_Visible;
}

void gui_scroll_activity_up(struct GuiButton *gbtn)
{
    if (top_of_breed_list > 0)
      top_of_breed_list--;
}

void gui_scroll_activity_down(struct GuiButton *gbtn)
{
    if (top_of_breed_list + 6 < no_of_breeds_owned)
        top_of_breed_list++;
}

void gui_area_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = info_panel_pos_to_player_number(gbtn->content.lval);
    if(plyr_idx == -1)
        return;

    int spr_idx = GPS_plyrsym_symbol_player_any_dis;
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        return;
    }
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_plyrsym_symbol_player_any_dis, 100);
    if ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate)
    {
        struct PlayerInfo* player = get_my_player();
        if (player_allied_with(player, plyr_idx)) {
            spr_idx = get_player_colored_icon_idx(GPS_plyrsym_symbol_player_red_std_b,plyr_idx);
        }
    } else
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if (player_allied_with(player, my_player_number)) {
            spr_idx = get_player_colored_icon_idx(GPS_plyrsym_symbol_player_red_std_b,plyr_idx);
        }
    }
    if ( gbtn->button_state_left_pressed || gbtn->button_state_right_pressed )
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
    } else
    {
        draw_gui_panel_sprite_left_player(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, plyr_idx);
    }
}

void gui_area_stat_button(struct GuiButton *gbtn)
{
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_frame_rect_wide_up, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_rect_wide_up);
    struct PlayerInfo* player = get_my_player();
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    if (!thing_exists(thing))
        return;
    if (thing->class_id == TCls_Creature)
    {
        const char* text = creature_statistic_text(thing, gbtn->content.lval);
        int x = gbtn->scr_pos_x - 6*ps_units_per_px/16;
        int y = gbtn->scr_pos_y - 12*ps_units_per_px/16;
        if (MyScreenHeight < 400)
        {
            y += (gbtn->height / 2);
        }
        draw_gui_panel_sprite_left(x, y, ps_units_per_px, gbtn->sprite_idx);
        draw_button_string(gbtn, 60, text);
    }
}

void maintain_event_button(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    EventIndex evidx;
    unsigned long evbtn_idx = gbtn->content.lval;
    int32_t keycode;
    if (evbtn_idx <= EVENT_BUTTONS_COUNT)
    {
        evidx = dungeon->event_button_index[evbtn_idx];
    }
    else
    {
        evidx = 0;
    }
    struct Event* event = &game.event[evidx];
    if ((dungeon->visible_event_idx != 0) && (evidx == dungeon->visible_event_idx))
    {
        turn_on_event_info_panel_if_necessary(dungeon->visible_event_idx);
        //TODO: that should be not here, Keys should be processed at one place
        if (is_game_key_pressed(Gkey_ToggleMessage, &keycode, false)
            && ((get_player(my_player_number)->allocflags & PlaF_NewMPMessage) == 0))
        {
            gui_kill_event(gbtn);
            clear_key_pressed(keycode);
        }
    }
    else
    {
        if (dungeon->visible_event_idx == 0)
        {
            if (is_game_key_pressed(Gkey_ToggleMessage, &keycode, false)
                && ((get_player(my_player_number)->allocflags & PlaF_NewMPMessage) == 0))
            {
                for (int i = EVENT_BUTTONS_COUNT; i >= 0; i--)
                {
                    long k = dungeon->event_button_index[i];
                    if (k != 0)
                    {
                        activate_event_box(k);
                        break;
                    }
                }
                clear_key_pressed(keycode);
            }
        }
    }

    if (evidx == 0)
    {
      gbtn->btype_value |= LbBFeF_NoMouseOver;
      gbtn->sprite_idx = 0;
      gbtn->flags &= ~LbBtnF_Enabled;
      gbtn->button_state_left_pressed = 0;
      gbtn->button_state_right_pressed = 0;
      gbtn->tooltip_stridx = GUIStr_Empty;
      return;
    }
    if ((event->kind == EvKind_Objective) && (new_objective))
    {
        activate_event_box(evidx);
    }
    gbtn->sprite_idx = event_button_info[event->kind].bttn_sprite;
    if (((event->kind == EvKind_FriendlyFight) || (event->kind == EvKind_EnemyFight))
        && ((event->mappos_x != 0) || (event->mappos_y != 0)) && ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate))
    {
        // Fight icon flashes when there are fights to show
        gbtn->sprite_idx += 2;
        if(is_game_key_pressed(Gkey_ZoomToFight, &keycode, true) && (is_game_key_pressed(Gkey_SpeedMod, NULL, true)))
        {
            if (evidx == dungeon->visible_event_idx)
            {
            clear_key_pressed(keycode);
            gui_close_objective(gbtn);
            }
            else
            {
            clear_key_pressed(keycode);
            activate_event_box(evidx);
            }
        }
    } else
    if (((event->kind == EvKind_Information) || (event->kind == EvKind_QuickInformation))
      && (event->target < 0) && ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate))
    {
        // Unread information flashes
        gbtn->sprite_idx += 2;
    } else
    if ((event->kind == EvKind_HeartAttacked)
        && ((event->mappos_x != 0) || (event->mappos_y != 0)) && ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate))
    {
        // Heart alert icon flashes when heart is being attacked
        gbtn->sprite_idx += 2;
    }
    gbtn->tooltip_stridx = event_button_info[event->kind].tooltip_stridx;
    gbtn->flags |= LbBtnF_Enabled;
    gbtn->btype_value &= LbBFeF_IntValueMask;
}

void gui_toggle_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = info_panel_pos_to_player_number(gbtn->content.lval);
    if(plyr_idx == -1)
        return;
    if ((gbtn->flags & LbBtnF_Enabled) != 0) {
        struct Packet* pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_PlyrToggleAlly, plyr_idx, 0, 0, 0);
    }
}

static unsigned char count_current_players_count()
{
    unsigned char current_players_count = 0;
    for (size_t i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if(player_exists(player) && player_is_keeper(i))
            current_players_count++;
    }
    return current_players_count;
}

void maintain_player_page2(struct GuiButton *gbtn)
{
    unsigned char current_players_count = count_current_players_count();
    if(current_players_count > 4)
    {
        set_flag(gbtn->flags, (LbBtnF_Visible | LbBtnF_Enabled));
    }
    else
    {
        clear_flag(gbtn->flags, (LbBtnF_Visible | LbBtnF_Enabled));
    }
}

void maintain_query_button(struct GuiButton *gbtn)
{
    unsigned char current_players_count = count_current_players_count();
    if(current_players_count > 4)
    {
        gbtn->pos_x = scale_ui_value(74);
        gbtn->scr_pos_x = scale_ui_value(74);
    }
    else
    {
        gbtn->pos_x = scale_ui_value(44);
        gbtn->scr_pos_x = scale_ui_value(44);
    }

}

void maintain_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = info_panel_pos_to_player_number(gbtn->content.lval);
    if(plyr_idx == -1)
        return;

    struct PlayerInfo* player = get_player(plyr_idx);
    if (!is_my_player_number(plyr_idx) && ((player->allocflags & PlaF_Allocated) != 0))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_prison_bar(struct GuiButton *gbtn)
{
    if (player_has_room_of_role(my_player_number, RoRoF_Prison))
    {
        gbtn->sprite_idx = GPS_rpanel_tendency_prisne_act;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->sprite_idx = GPS_rpanel_tendency_prisnu_dis;
        gbtn->flags &= ~LbBtnF_Enabled;
        /*if (gbtn->button_state_left_pressed) - this does nothing, but was in original function
        {
            menu_id_to_number(7);
        }*/
    }
}

void maintain_room_button(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = gbtn->content.lval;
    struct PlayerInfo* player = get_player(plyr_idx);
    gbtn->sprite_idx = get_player_colored_icon_idx(GPS_plyrsym_symbol_room_red_std_a,plyr_idx);

    if (player_exists(player))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoMouseOver;
        gbtn->flags &= ~LbBtnF_Enabled;
        gbtn->tooltip_stridx = GUIStr_Empty;
    }
}
void maintain_creature_button(struct GuiButton* gbtn)
{
    PlayerNumber plyr_idx = gbtn->content.lval;
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_exists(player))
    {
        if (player_has_heart(plyr_idx))
        {
            gbtn->sprite_idx = get_player_colored_icon_idx(GPS_plyrsym_symbol_player_red_std_a,plyr_idx);
        }
        else
        {
            gbtn->sprite_idx = get_player_colored_icon_idx(GPS_plyrsym_symbol_player_red_dead,plyr_idx);
        }
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    }
    else
    {
        gbtn->btype_value |= LbBFeF_NoMouseOver;
        gbtn->flags &= ~LbBtnF_Enabled;
        gbtn->tooltip_stridx = GUIStr_Empty;
    }
}

void maintain_compsetting_button(struct GuiButton* gbtn)
{
    struct ComputerType* cpt = get_computer_type_template(comp_player_conf.computer_assist_types[gbtn->btype_value]);
    gbtn->tooltip_stridx = cpt->tooltip_stridx;
    gbtn->sprite_idx = cpt->sprite_idx;
}

void pick_up_next_wanderer(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(CREATURE_ANY, CrGUIJob_Wandering, my_player_number, pick_flags);
}

void gui_go_to_next_wanderer(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(CREATURE_ANY, CrGUIJob_Wandering, pick_flags);
}

void pick_up_next_worker(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(CREATURE_ANY, CrGUIJob_Working, my_player_number, pick_flags);
}

void gui_go_to_next_worker(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(CREATURE_ANY, CrGUIJob_Working, pick_flags);
}

void pick_up_next_fighter(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(CREATURE_ANY, CrGUIJob_Fighting, my_player_number, pick_flags);
}

void gui_go_to_next_fighter(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(CREATURE_ANY, CrGUIJob_Fighting, pick_flags);
}

void gui_area_payday_button(struct GuiButton *gbtn)
{
    int units_per_px = (gbtn->width * 16 + 132 / 2) / 132;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, gbtn->sprite_idx, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
    gui_area_progress_bar_wide(gbtn, units_per_px, game.pay_day_progress[my_player_number], game.conf.rules[my_player_number].game.pay_day_gap);
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    char text[16];
    snprintf(text, sizeof(text), "%d", (int)dungeon->creatures_total_pay);
    draw_centred_string64k(text, gbtn->scr_pos_x + (gbtn->width >> 1), gbtn->scr_pos_y + scale_value_by_vertical_resolution(8), 130, gbtn->width);
}

void gui_area_research_bar(struct GuiButton *gbtn)
{
    int units_per_px = (gbtn->width * 16 + 60 / 2) / 60;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    int resrch_required;
    int resrch_progress;
    struct ResearchVal* rsrchval = get_players_current_research_val(my_player_number);
    if (rsrchval != NULL)
    {
        resrch_required = rsrchval->req_amount;
        resrch_progress = (dungeon->research_progress >> 8);
    } else
    {
        resrch_required = 0;
        resrch_progress = 0;
    }
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_bar_with_pic_full_blue_up, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_bar_with_pic_full_blue_up);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x - 8*units_per_px/16, gbtn->scr_pos_y - 10*units_per_px/16, ps_units_per_px, gbtn->sprite_idx);
    gui_area_progress_bar_short(gbtn, units_per_px, resrch_progress, resrch_required);
}

void gui_area_workshop_bar(struct GuiButton *gbtn)
{
    int units_per_px = (gbtn->width * 16 + 60 / 2) / 60;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    int manufct_required;
    int manufct_progress;
    if (dungeon->manufacture_class != TCls_Empty)
    {
        manufct_required = manufacture_points_required(dungeon->manufacture_class, dungeon->manufacture_kind);
        manufct_progress = (dungeon->manufacture_progress >> 8);
    } else
    {
        manufct_required = 0;
        manufct_progress = 0;
    }
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_bar_with_pic_full_blue_up, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_bar_with_pic_full_blue_up);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x - 8*units_per_px/16, gbtn->scr_pos_y - 10*units_per_px/16, ps_units_per_px, gbtn->sprite_idx);
    gui_area_progress_bar_short(gbtn, units_per_px, manufct_progress, manufct_required);
}

void gui_area_player_creature_info(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = info_panel_pos_to_player_number(gbtn->content.lval);
    if(plyr_idx == -1)
        return;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_frame_rect_wide_up, 100);
    struct PlayerInfo* player = get_player(plyr_idx);
    draw_gui_panel_sprite_left_player(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_rect_wide_up, plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        unsigned long spr_idx = get_player_colored_icon_idx(player_has_heart(plyr_idx) ? GPS_plyrsym_symbol_player_red_std_a : GPS_plyrsym_symbol_player_red_dead, plyr_idx);
        if (((dungeon->num_active_creatrs < dungeon->max_creatures_attracted) && (!game.pool.is_empty))
            || ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate))
        {
            draw_gui_panel_sprite_left_player(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, plyr_idx);
        } else
        {
            draw_gui_panel_sprite_rmleft_player(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44, plyr_idx);
        }
        char text[32];
        if (game.conf.rules[plyr_idx].game.display_portal_limit == true)
        {
            snprintf(text, sizeof(text), " %u/%d", dungeon->num_active_creatrs, dungeon->max_creatures_attracted);
        } else {
            snprintf(text, sizeof(text), "%u", dungeon->num_active_creatrs);
        }
        draw_button_string(gbtn, 60, text);
    }
}

void gui_area_player_room_info(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = info_panel_pos_to_player_number(gbtn->content.lval);
    if(plyr_idx == -1)
        return;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, GPS_rpanel_frame_rect_wide_up, 100);
    struct PlayerInfo* player = get_player(plyr_idx);
    draw_gui_panel_sprite_left_player(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_rpanel_frame_rect_wide_up, plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);

    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        draw_gui_panel_sprite_left_player(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, GPS_plyrsym_symbol_room_red_std_a, plyr_idx);
        long i = dungeon->total_rooms;
        char text[16];
        snprintf(text, sizeof(text), "%ld", i);
        draw_button_string(gbtn, 60, text);
    }
}

void spell_lost_first_person(struct GuiButton *gbtn)
{
    SYNCDBG(19,"Starting");
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_GoSpectator, 0, 0, 0, 0);
}

void gui_set_tend_to(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_ToggleTendency, gbtn->btype_value & LbBFeF_IntValueMask, 0, 0, 0);
}

void gui_set_query(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_SetPlyrState, PSt_CreatrQuery, 0, 0, 0);
}

void gui_switch_players_visible(struct GuiButton *gbtn)
{
    if(info_page == 0)
    {
        info_page = 1;
        return;
    }
    else if (info_page == 1)
    {
        unsigned char current_players_count = count_current_players_count();
        if(current_players_count > 7)
        {
            info_page = 2;
            return;
        }
    }
    info_page = 0;
    return;
}

void draw_gold_total(PlayerNumber plyr_idx, int32_t scr_x, int32_t scr_y, int32_t units_per_px, long long value)
{
    long long i;
    unsigned int flg_mem = lbDisplay.DrawFlags;
    int ndigits = 0;
    int val_width = 0;
    for (i = value; i > 0; i /= 10) {
        ndigits++;
    }
    const struct TbSprite* spr = get_button_sprite(GBS_fontchars_number_dig0);
    val_width = scale_value_for_resolution_with_upp(spr->SWidth, units_per_px) * ndigits;
    if (ndigits > 0)
    {
        long pos_x = scr_x + val_width / 2;
        for (i = value; i > 0; i /= 10)
        {
            // Make space for the character first, as we're drawing right char towards left
            pos_x -= scale_value_for_resolution_with_upp(spr->SWidth, units_per_px);
            spr = get_button_sprite(i % 10 + GBS_fontchars_number_dig0);
            LbSpriteDrawResized(pos_x, scr_y, units_per_px, spr);
        }
    } else
    {
        // Just draw zero
        spr = get_button_sprite(GBS_fontchars_number_dig0);
        LbSpriteDrawResized(scr_x, scr_y, units_per_px, spr);
    }
    lbDisplay.DrawFlags = flg_mem;
}

void draw_whole_status_panel(void)
{
    long mmzoom;
    struct PlayerInfo* player = get_my_player();
    struct Dungeon* dungeon = get_players_dungeon(player);
    // Get the menu scale
    struct GuiMenu *gmnu;
    int fs_units_per_px;
    int mm_units_per_px;
    {
        int mnu_num = menu_id_to_number(GMnu_MAIN);
        gmnu = get_active_menu(mnu_num);
        mm_units_per_px = (gmnu->width * 16 + 140/2) / 140;
        if (mm_units_per_px < 1)
            mm_units_per_px = 1;
        fs_units_per_px = (gmnu->height * 16 + 8) / LbTiledSpriteHeight(&status_panel);
    }
    lbDisplay.DrawColour = colours[15][15][15];
    lbDisplay.DrawFlags = 0;
    LbTiledSpriteDraw(0, 0, fs_units_per_px, &status_panel);
    // Draws gold amount; note that button_sprite[] is used instead of full font
    draw_gold_total(player->id_number, gmnu->pos_x + gmnu->width/2, gmnu->pos_y + gmnu->height*67/200, fs_units_per_px, dungeon->total_money_owned);
    if (16/mm_units_per_px < 3)
        mmzoom = (player->minimap_zoom) / scale_value_for_resolution_with_upp(2,mm_units_per_px);
    else
        mmzoom = player->minimap_zoom;
    panel_map_draw_slabs(player->minimap_pos_x, player->minimap_pos_y, mm_units_per_px, mmzoom);
    long basic_zoom = player->minimap_zoom;
    panel_map_draw_overlay_things(mm_units_per_px, mmzoom, basic_zoom);
    reset_all_minimap_interpolation = false; // Done resetting
    unsigned char placefill_threshold = (LbScreenHeight() >= 400) ? 80 : 40;
    if (LbScreenHeight() - gmnu->height >= placefill_threshold)
    {
        draw_placefiller(0, gmnu->pos_y + gmnu->height, fs_units_per_px);
    }
}

void gui_set_button_flashing(long btn_idx, long gameturns)
{
    game.flash_button_index = btn_idx;
    game.flash_button_time = gameturns;
}

void update_room_tab_to_config(void)
{
    SYNCDBG(8, "Starting");
    int i;
    struct GuiButtonInit* ibtn;
    // Clear 4x4 area of buttons, but skip "sell" button at end
    for (i=0; i < 15; i++)
    {
        ibtn = &room_menu.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = RoK_NONE;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
        ibtn = &room_menu2.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = RoK_NONE;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
    }
    for (i=0; i < game.conf.slab_conf.room_types_count; i++)
    {
        struct RoomConfigStats* roomst = &game.conf.slab_conf.room_cfgstats[i];
        if (roomst->panel_tab_idx < 1)
            continue;
        if (roomst->panel_tab_idx <= 16)
        {
            ibtn = &room_menu.buttons[roomst->panel_tab_idx - 1];
        }
        else
        {
            ibtn = &room_menu2.buttons[roomst->panel_tab_idx - 17];
        }
        ibtn->sprite_idx = roomst->medsym_sprite_idx;
        ibtn->tooltip_stridx = roomst->tooltip_stridx;
        ibtn->content.lval = i;
        ibtn->click_event = gui_choose_room;
        ibtn->rclick_event = gui_go_to_next_room;
        ibtn->ptover_event = gui_over_room_button;
        ibtn->draw_call = gui_area_room_button;
    }
    // Update active menu
    struct PlayerInfo *player = get_my_player();
    if (player->view_type == PVT_DungeonTop)
    {
        if (menu_is_active(GMnu_ROOM))
        {
            turn_off_menu(GMnu_ROOM);
            turn_on_menu(GMnu_ROOM);
        }
        else if (menu_is_active(GMnu_ROOM2))
        {
            turn_off_menu(GMnu_ROOM2);
            turn_on_menu(GMnu_ROOM2);
        }
    }
}

void update_trap_tab_to_config(void)
{
    SYNCDBG(8, "Starting");
    int i;
    struct GuiButtonInit* ibtn;
    // Clear 4x4 area of buttons, but skip "sell" button at end
    for (i=0; i < 15; i++)
    {
        ibtn = &trap_menu.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = 0;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
        ibtn->maintain_call = NULL;

        ibtn = &trap_menu2.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = 0;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
        ibtn->maintain_call = NULL;
    }
    for (i=0; i < game.conf.trapdoor_conf.manufacture_types_count; i++)
    {
        struct ManufactureData* manufctr = get_manufacture_data(i);
        if (manufctr->panel_tab_idx < 1)
            continue;
        if (manufctr->panel_tab_idx <= 16)
        {
            ibtn = &trap_menu.buttons[manufctr->panel_tab_idx - 1];
        }
        else
        {
            ibtn = &trap_menu2.buttons[manufctr->panel_tab_idx - 17];
        }
        ibtn->sprite_idx = manufctr->medsym_sprite_idx;
        ibtn->tooltip_stridx = manufctr->tooltip_stridx;
        ibtn->content.lval = i;
        switch (manufctr->tngclass)
        {
            case TCls_Trap:
                ibtn->click_event = gui_choose_workshop_item;
                ibtn->rclick_event = gui_go_to_next_trap;
                ibtn->ptover_event = gui_over_trap_button;
                ibtn->draw_call = gui_area_trap_button;
                ibtn->maintain_call = maintain_trap;
                break;
            case TCls_Door:
                ibtn->click_event = gui_choose_workshop_item;
                ibtn->rclick_event = gui_go_to_next_door;
                ibtn->ptover_event = gui_over_door_button;
                ibtn->draw_call = gui_area_trap_button;
                ibtn->maintain_call = maintain_door;
                break;
            default:
                break;
        }
    }
    // Update active menu
    struct PlayerInfo *player = get_my_player();
    if (player->view_type == PVT_DungeonTop)
    {
        if ( menu_is_active(GMnu_TRAP) )
        {
            turn_off_menu(GMnu_TRAP);
            turn_on_menu(GMnu_TRAP);
        }
        else if ( menu_is_active(GMnu_TRAP2) )
        {
            turn_off_menu(GMnu_TRAP2);
            turn_on_menu(GMnu_TRAP2);
        }
    }
}

void update_powers_tab_to_config(void)
{
    SYNCDBG(8, "Starting");
    int i;
    struct GuiButtonInit* ibtn;
    // Clear 4x4 area of buttons, no "sell" button at end
    for (i=0; i < 16; i++)
    {
        ibtn = &spell_menu.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = PwrK_None;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
        ibtn->maintain_call = NULL;
        ibtn = &spell_menu2.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = PwrK_None;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
        ibtn->maintain_call = NULL;
    }
    for (PowerKind pwkind = PwrK_None; pwkind < game.conf.magic_conf.power_types_count; pwkind++)
    {
        struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
        if (powerst->panel_tab_idx < 1)
            continue;
        if (powerst->panel_tab_idx <= 16)
        {
            ibtn = &spell_menu.buttons[powerst->panel_tab_idx - 1];
        }
        else
        {
            ibtn = &spell_menu2.buttons[powerst->panel_tab_idx - 17];
        }
        ibtn->sprite_idx = powerst->medsym_sprite_idx;
        ibtn->tooltip_stridx = powerst->tooltip_stridx;
        ibtn->content.lval = pwkind;
        if (is_special_power(pwkind)) {
            ibtn->click_event = gui_choose_special_spell;
            ibtn->rclick_event = NULL;
            ibtn->ptover_event = NULL;
        } else {
            ibtn->click_event = gui_choose_spell;
            ibtn->rclick_event = gui_go_to_next_spell;
            ibtn->ptover_event = NULL;
        }
        ibtn->draw_call = gui_area_spell_button;
        ibtn->maintain_call = maintain_spell;
    }
}

void draw_placefiller(long scr_x, long scr_y, long units_per_px)
{
    const struct TbSprite* spr = get_panel_sprite(GPS_rpanel_rpanel_extra);
    LbSpriteDrawResized(scr_x, scr_y, units_per_px, spr);
}

void gui_query_next_creature_of_owner_and_model(struct GuiButton *gbtn)
{
    struct PlayerInfo *player = get_my_player();
    struct Thing *creatng = thing_get(player->influenced_thing_idx);
    ThingIndex next_creature = get_index_of_next_creature_of_owner_and_model(creatng, creatng->owner, creatng->model, player);
    if (next_creature != player->influenced_thing_idx)
    {
        struct Packet* pckt = get_packet(player->id_number);
        set_packet_action(pckt, PckA_PlyrQueryCreature, next_creature, 0, 1, 0);
        play_non_3d_sample(62);
    }
}

void gui_query_next_creature_of_owner(struct GuiButton *gbtn)
{
    struct PlayerInfo *player = get_my_player();
    struct Thing *creatng = thing_get(player->influenced_thing_idx);
    ThingIndex next_creature = get_index_of_next_creature_of_owner_and_model(creatng, creatng->owner, 0, player);
    if (next_creature != player->influenced_thing_idx)
    {
        struct Packet* pckt = get_packet(player->id_number);
        set_packet_action(pckt, PckA_PlyrQueryCreature, next_creature, 0, 1, 0);
        play_non_3d_sample(62);
    }
}

void maintain_spell_next_page_button(struct GuiButton *gbtn)
{
    for (int i=0; i < 16; i++)
    {
        struct GuiButtonInit* ibtn = &spell_menu2.buttons[i];
        if (is_power_obtainable(my_player_number, ibtn->content.lval))
        {
            gbtn->flags |= (LbBtnF_Visible|LbBtnF_Enabled);
            return;
        }
    }
    gbtn->flags &= ~(LbBtnF_Visible|LbBtnF_Enabled);
}

void maintain_room_next_page_button(struct GuiButton *gbtn)
{
    for (int i=0; i < 16; i++)
    {
        struct GuiButtonInit* ibtn = &room_menu2.buttons[i];
        if (ibtn->content.lval != RoK_NONE)
        {
            if (is_room_obtainable(my_player_number, ibtn->content.lval))
            {
                gbtn->flags |= (LbBtnF_Visible|LbBtnF_Enabled);
                return;
            }
        }
    }
    gbtn->flags &= ~(LbBtnF_Visible|LbBtnF_Enabled);
}

void maintain_trap_next_page_button(struct GuiButton *gbtn)
{
    for (int i=0; i < 16; i++)
    {
        struct GuiButtonInit* ibtn = &trap_menu2.buttons[i];
        struct ManufactureData* manufctr = get_manufacture_data(ibtn->content.lval);
        TbBool result;
        switch (manufctr->tngclass)
        {
            case TCls_Trap:
                result = ( (is_trap_buildable(my_player_number, manufctr->tngmodel)) || (is_trap_placeable(my_player_number, manufctr->tngmodel)) || (is_trap_built(my_player_number, manufctr->tngmodel)) );
                break;
            case TCls_Door:
                result = ( (is_door_buildable(my_player_number, manufctr->tngmodel)) || (is_door_placeable(my_player_number, manufctr->tngmodel)) || (is_door_built(my_player_number, manufctr->tngmodel)) );
                break;
            default:
                result = false;
                break;
        }
        if (result)
        {
            gbtn->flags |= (LbBtnF_Visible|LbBtnF_Enabled);
            return;
        }
    }
    gbtn->flags &= ~(LbBtnF_Visible|LbBtnF_Enabled);
}
/******************************************************************************/
