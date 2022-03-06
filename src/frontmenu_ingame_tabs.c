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
#include "frontmenu_ingame_tabs.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_keybrd.h"
#include "bflib_guibtns.h"
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
#include "room_workshop.h"
#include "room_list.h"
#include "gui_frontbtns.h"
#include "gui_parchment.h"
#include "gui_draw.h"
#include "packets.h"
#include "magic.h"
#include "player_computer.h"
#include "player_instances.h"
#include "player_states.h"
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

/******************************************************************************/
/******************************************************************************/
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
    if ((btn_designt_id >= BID_ROOM_TD01) && (btn_designt_id <= BID_ROOM_TD16))
        return BID_ROOM_TAB;
    if ((btn_designt_id >= BID_POWER_TD01) && (btn_designt_id <= BID_POWER_TD16))
        return BID_SPELL_TAB;
    if ((btn_designt_id >= BID_MNFCT_TD01) && (btn_designt_id <= BID_MNFCT_TD16))
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
            struct GuiButtonInit * ibtn;
            ibtn = &room_menu.buttons[i];
            if (ibtn->content.lval == btn_item)
                return ibtn->id_num;
        }
        break;
    case GID_POWER_PANE:
        for (i=0; i < 4*4; i++)
        {
            struct GuiButtonInit * ibtn;
            ibtn = &spell_menu.buttons[i];
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
            struct GuiButtonInit * ibtn;
            ibtn = &trap_menu.buttons[i];
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
            struct GuiButtonInit * ibtn;
            ibtn = &trap_menu.buttons[i];
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
          if (game.play_gameturn & 1)
            spr_idx += 2;
        }
        if ((gbtn->gbactn_1 == 0) && (gbtn->gbactn_2 == 0))
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
    //TODO COMPUTER_PLAYER change limit to comp_player_conf.computers_count when the array is inside computer player config
    if (cplr_model < COMPUTER_MODELS_COUNT) {
        gbtn->tooltip_stridx = computer_types_tooltip_stridx[cplr_model];
    } else {
        ERRORLOG("Illegal computer player model %d",(int)cplr_model);
    }
}

void gui_choose_room(struct GuiButton *gbtn)
{
    // prepare to enter room build mode
    activate_room_build_mode((long)gbtn->content, gbtn->tooltip_stridx);
}

void gui_area_event_button(struct GuiButton *gbtn)
{
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 258, 100);
        struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
        unsigned long i = (unsigned long)gbtn->content;
        if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
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
    long i = dungeon->room_kind[rkind];
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
    RoomKind rkind = (int)gbtn->content;
    struct PlayerInfo* player = get_my_player();

    struct Dungeon* dungeon = get_players_dungeon(player);

    unsigned short flg_mem = lbDisplay.DrawFlags;
    int units_per_px = (gbtn->width * 16 + 126 / 2) / 126;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 26, 100);

    if (rkind == 0) {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    int i = find_room_type_capacity_total_percentage(player->id_number, rkind);
    if ((rkind == RoK_ENTRANCE) || (rkind == RoK_DUNGHEART) || (i < 0))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 23);
        gui_area_progress_bar_med1(gbtn, units_per_px, i, 256);
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;

    struct RoomStats* rstat = room_stats_get_for_kind(rkind);
    if ((player->work_state == PSt_BuildRoom) && (player->boxsize > 1))
    {
        sprintf(gui_textbuf, "%ld", (long)rstat->cost * player->boxsize);
    }
    else
    {
        sprintf(gui_textbuf, "%ld", (long)rstat->cost);
    }
    if (rstat->cost * player->boxsize <= dungeon->total_money_owned)
    {
        if ((player->work_state == PSt_BuildRoom) && (player->chosen_room_kind == game.chosen_room_kind)
          && ((game.play_gameturn & 1) == 0))
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
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    draw_string64k(gbtn->scr_pos_x + 44*units_per_px/16, gbtn->scr_pos_y + (8 - 6)*units_per_px/16, tx_units_per_px, gui_textbuf);

    long amount = count_player_rooms_of_type(player->id_number, rkind);
    // Note that "@" is "x" in that font
    sprintf(gui_textbuf, "@%ld", amount);
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
    choose_spell((int) gbtn->content, gbtn->tooltip_stridx);
}

void go_to_next_spell_of_type(PowerKind pwkind, PlayerNumber plyr_idx)
{
    struct Packet* pckt = get_packet(plyr_idx);
    set_packet_action(pckt, PckA_ZoomToSpell, pwkind, 0, 0, 0);
}

void gui_go_to_next_spell(struct GuiButton *gbtn)
{
    PowerKind pwkind = (int)gbtn->content;
    struct PlayerInfo* player = get_my_player();
    go_to_next_spell_of_type(pwkind, player->id_number);
    set_chosen_power(pwkind, gbtn->tooltip_stridx);
}

void gui_area_spell_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 24, 128);

    PowerKind pwkind = (long)gbtn->content;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 24);
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
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 27);
            }
            GoldAmount price = compute_power_price(dungeon->owner, pwkind, 0);
            spr_idx = gbtn->sprite_idx;
            if (dungeon->total_money_owned < price)
                spr_idx++;
            TbBool drawn = false;
            if ((gbtn->gbactn_1 == 0) && (gbtn->gbactn_2 == 0))
            {
                if ((((i != PSt_CallToArms) || !player_uses_power_call_to_arms(my_player_number))
                  && ((i != PSt_SightOfEvil) || !player_uses_power_sight(my_player_number)))
                 || ((game.play_gameturn & 1) == 0))
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
                spr_idx = 25;
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
    choose_spell(((int) gbtn->content) % POWER_TYPES_COUNT, gbtn->tooltip_stridx);
}

void gui_area_big_spell_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int units_per_px = (gbtn->width * 16 + 126 / 2) / 126;

    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 26, 100);
    PowerKind pwkind = (long)gbtn->content;
    struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
    if (power_model_stats_invalid(powerst))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
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
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 23);
        int fill_bar = 42 - (2 * 21 * pwage / 256);
        LbDrawBox(
            gbtn->scr_pos_x + (114 - fill_bar)*units_per_px/16,
            gbtn->scr_pos_y + 12*units_per_px/16,
          fill_bar*units_per_px/16, 6*units_per_px/16, colours[0][0][0]);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;

    GoldAmount price = compute_power_price(dungeon->owner, pwkind, 0);
    char* text = buf_sprintf("%ld", (long)price);
    if (dungeon->total_money_owned >= price)
    {
        if ((player->work_state == powerst->work_state) && ((game.play_gameturn & 1) != 0)) {
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
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
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
    set_players_packet_action(player, PckA_SetPlyrState, manufctr->work_state,
        manufctr->tngmodel, 0, 0);

    game.manufactr_element = manufctr_idx;
    game.manufactr_spridx = manufctr->bigsym_sprite_idx;
    game.manufactr_tooltip = tooltip_id;
}

void gui_choose_trap(struct GuiButton *gbtn)
{
    //Note by Petter: factored out gui_choose_trap to choose_workshop_item (better name as well)
    choose_workshop_item((int) gbtn->content, gbtn->tooltip_stridx);
}

void go_to_next_trap_of_type(ThingModel tngmodel, PlayerNumber plyr_idx)
{
    struct Thing *thing;
    if (tngmodel >= 8) {
        ERRORLOG("Bad trap kind");
        return;
    }
    unsigned long k = 0;
    static unsigned short seltrap[8];
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
    if (tngmodel >= 8) {
        ERRORLOG("Bad door kind");
        return;
    }
    unsigned long k = 0;
    static unsigned short seldoor[8];
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
    int manufctr_idx = (int)gbtn->content;
    struct PlayerInfo* player = get_my_player();
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    go_to_next_trap_of_type(manufctr->tngmodel, player->id_number);
    game.manufactr_element = manufctr_idx;
    game.manufactr_spridx = manufctr->bigsym_sprite_idx;
    game.manufactr_tooltip = manufctr->tooltip_stridx;
}

void gui_over_trap_button(struct GuiButton *gbtn)
{
    int manufctr_idx = (long)gbtn->content;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    gui_trap_type_highlighted = manufctr->tngmodel;
}

void gui_area_trap_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 24, 128);

    int manufctr_idx = (long)gbtn->content;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 24);
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
    if ((gbtn->flags & LbBtnF_Enabled) == 0)
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 25);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    // Check how many traps/doors do we have to place
    unsigned int amount;
    switch (manufctr->tngclass)
    {
    case TCls_Trap:
        // If there are traps of that type placed on map
        if (player_has_deployed_trap_of_model(my_player_number, manufctr->tngmodel)) {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 27);
        }
        amount = dungeonadd->mnfct_info.trap_amount_placeable[manufctr->tngmodel];
        break;
    case TCls_Door:
        // If there are doors of that type placed on map
        if (player_has_deployed_door_of_model(my_player_number, manufctr->tngmodel, -1)) {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 27);
        }
        amount = dungeonadd->mnfct_info.door_amount_placeable[manufctr->tngmodel];
        break;
    default:
        amount = 0;
        break;
    }
    int i = gbtn->sprite_idx + (amount < 1);
    if (gbtn->gbactn_1 || gbtn->gbactn_2)
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
    int manufctr_idx = (int)gbtn->content;
    struct PlayerInfo* player = get_my_player();
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    go_to_next_door_of_type(manufctr->tngmodel, player->id_number);
    game.manufactr_element = manufctr_idx;
    game.manufactr_spridx = manufctr->bigsym_sprite_idx;
    game.manufactr_tooltip = manufctr->tooltip_stridx;
}

void gui_over_door_button(struct GuiButton *gbtn)
{
    int manufctr_idx = (long)gbtn->content;
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

void gui_area_big_trap_button(struct GuiButton *gbtn)
{
    int manufctr_idx = (int)gbtn->content;
    struct PlayerInfo* player = get_my_player();

    struct Dungeon* dungeon = get_players_dungeon(player);
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    unsigned short flg_mem = lbDisplay.DrawFlags;
    int units_per_px = (gbtn->width * 16 + 126 / 2) / 126;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 26, 100);

    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
    if (manufctr_idx == 0) {
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    unsigned int amount;
    switch (manufctr->tngclass)
    {
    case TCls_Trap:
        amount = dungeonadd->mnfct_info.trap_amount_placeable[manufctr->tngmodel];
        break;
    case TCls_Door:
        amount = dungeonadd->mnfct_info.door_amount_placeable[manufctr->tngmodel];
        break;
    default:
        amount = 0;
        break;
    }
    // Note that "@" is "x" in that font
    sprintf(gui_textbuf, "@%ld", (long)amount);
    if (amount <= 0) {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx + 1);
    } else
    if ((((manufctr->tngclass == TCls_Trap) && (player->chosen_trap_kind == manufctr->tngmodel) && (player->work_state == PSt_PlaceTrap))
      || ((manufctr->tngclass == TCls_Door) && (player->chosen_door_kind == manufctr->tngmodel) && (player->work_state == PSt_PlaceDoor)))
      && ((game.play_gameturn & 1) == 0) )
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx, 44);
    } else {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4*units_per_px/16, gbtn->scr_pos_y - 32*units_per_px/16, ps_units_per_px, gbtn->sprite_idx);
    }
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    draw_string64k(gbtn->scr_pos_x + 44*units_per_px/16, gbtn->scr_pos_y + (8 - 6)*units_per_px/16, tx_units_per_px, gui_textbuf);
    lbDisplay.DrawFlags = flg_mem;
}

void maintain_big_spell(struct GuiButton *gbtn)
{
    long spl_idx = game.chosen_spell_type;
    if ((spl_idx < 0) || (spl_idx >= KEEPER_POWERS_COUNT)) {
        return;
    }
    gbtn->content = (unsigned long *)spl_idx;
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
    RoomKind rkind = (long)gbtn->content;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    if ((rkind < 1) || (rkind >= ROOM_TYPES_COUNT)) {
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
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    if ((rkind < 1) || (rkind >= ROOM_TYPES_COUNT)) {
        return;
    }
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(8,"Cannot do; player %d has no dungeon",(int)my_player_number);
        return;
    }
    gbtn->content = (unsigned long *)rkind;
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
    long i = (unsigned long)(gbtn->content) & 0xff;
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
    int manufctr_idx = (unsigned int)gbtn->content;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    if (is_trap_placeable(my_player_number, manufctr->tngmodel) || is_trap_built(my_player_number, manufctr->tngmodel))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_door(struct GuiButton *gbtn)
{
    int manufctr_idx = (unsigned int)gbtn->content;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    if (is_door_placeable(my_player_number, manufctr->tngmodel) || is_door_built(my_player_number, manufctr->tngmodel))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void maintain_big_trap(struct GuiButton *gbtn)
{
    int manufctr_idx = game.manufactr_element % gameadd.trapdoor_conf.manufacture_types_count;
    struct ManufactureData* manufctr = get_manufacture_data(manufctr_idx);
    gbtn->content = (unsigned long *)manufctr_idx;
    gbtn->sprite_idx = game.manufactr_spridx;
    gbtn->tooltip_stridx = game.manufactr_tooltip;
    if ( ((manufctr->tngclass == TCls_Trap) && is_trap_placeable(my_player_number, manufctr->tngmodel))
      || ((manufctr->tngclass == TCls_Door) && is_door_placeable(my_player_number, manufctr->tngmodel)) )
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    }
}

void draw_centred_string64k(const char *text, short x, short y, short base_w, short dst_w)
{
    unsigned long flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    LbTextSetJustifyWindow((x - (dst_w / 2)), y, dst_w);
    LbTextSetClipWindow( (x - (dst_w / 2)), y, dst_w, 16*dst_w/base_w);
    lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    LbTextDrawResized(0, -6*dst_w/base_w, (22 * units_per_pixel) / LbTextLineHeight(), text);
    LbTextSetJustifyWindow(0, 0, LbGraphicsScreenWidth());
    LbTextSetClipWindow(0, 0, LbGraphicsScreenWidth(), LbGraphicsScreenHeight());
    LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    lbDisplay.DrawFlags = flg_mem;
}

void draw_name_box(long x, long y, int width, struct Thing *thing)
{
    int ps_units_per_px;
    {
        struct TbSprite* spr = &gui_panel_sprites[458];
        ps_units_per_px = (width*95/100) * 16 / spr->SWidth;
    }
    draw_gui_panel_sprite_left(x, y, ps_units_per_px, 458);
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
        const struct TbSprite* spr = get_button_sprite(spr_idx);
        int bs_units_per_px = (gmnu->width * 35 / 100) * 16 / spr->SWidth;
        LbSpriteDrawResized(portrt_x + 12*units_per_px/16, portrt_y + 12*units_per_px/16, bs_units_per_px, get_button_sprite(spr_idx));
    }
    {
        struct TbSprite* spr = &gui_panel_sprites[464];
        int ps_units_per_px = (gmnu->width * 52 / 100) * 16 / spr->SWidth;
        draw_gui_panel_sprite_left(portrt_x, portrt_y, ps_units_per_px, 464);
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
        struct TbSprite* spr = &gui_panel_sprites[spr_idx];
        int ps_units_per_px = (gmnu->width * 22 / 100) * 16 / spr->SWidth;
        draw_gui_panel_sprite_left(nambox_x, nambox_y - 22*units_per_px/16, ps_units_per_px, spr_idx);
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
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    else
        crmodel = get_players_special_digger_model(my_player_number);
    // Get index from pointer
    long job_idx = ((long*)gbtn->content - &activity_list[0]);
    unsigned char pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(crmodel, (job_idx & 0x03), my_player_number, pick_flags);
}

void gui_go_to_next_creature_activity(struct GuiButton *gbtn)
{
    ThingModel crmodel;
    int i = gbtn->btype_value & LbBFeF_IntValueMask;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    // Get index from pointer
    int job_idx = ((long*)gbtn->content - &activity_list[0]);
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(crmodel, (job_idx & 0x3), pick_flags);
}

RoomIndex find_my_next_room_of_type(RoomKind rkind)
{
    return find_next_room_of_type(my_player_number, rkind);
}

RoomIndex find_next_room_of_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    static RoomIndex next_room[ROOM_TYPES_COUNT];
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
        next_room[rkind] = dungeon->room_kind[rkind];
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
    unsigned long rkind = (long)gbtn->content;
    go_to_my_next_room_of_type_and_select(rkind);
    game.chosen_room_kind = rkind;
    struct RoomConfigStats* roomst = &slab_conf.room_cfgstats[rkind];
    game.chosen_room_spridx = roomst->bigsym_sprite_idx;
    game.chosen_room_tooltip = gbtn->tooltip_stridx;
}

void gui_over_room_button(struct GuiButton *gbtn)
{
    gui_room_type_highlighted = (long)gbtn->content;
}

void gui_area_room_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 24, 128);

    RoomKind rkind = (long)gbtn->content;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 24);
    struct Dungeon* dungeon = get_my_dungeon();
    if ((dungeon->room_buildable[rkind] & 1) // One can build it now
         || (dungeon->room_resrchable[rkind] == 1) // One can research it at any time
         || (dungeon->room_resrchable[rkind] == 2) // One can research it and get instantly then found
         || ((dungeon->room_resrchable[rkind] == 4) && (dungeon->room_buildable[rkind] & 2)) // Player able to research
         )
    {
        if ((gbtn->flags & LbBtnF_Enabled) != 0)
        {
            if (dungeon->room_kind[rkind] > 0)
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 27);
            int spr_idx = (dungeon->total_money_owned < game.room_stats[rkind].cost) + gbtn->sprite_idx;
            if ((gbtn->gbactn_1 == 0) && (gbtn->gbactn_2 == 0)) {
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
            } else {
                draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
            }
        } else
        {
            // Draw a question mark over the button, to indicate it can be researched
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 25);
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void pick_up_next_creature(struct GuiButton *gbtn)
{
    int kind;

    int i = gbtn->btype_value & LbBFeF_IntValueMask;
    if (i > 0) {
        kind = breed_activities[(i + top_of_breed_list) % CREATURE_TYPES_COUNT];
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
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
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
    long job_idx = ((long*)gbtn->content - &activity_list[0]);
    if ( (i > 0) && (top_of_breed_list+i < CREATURE_TYPES_COUNT) )
        crmodel = breed_activities[top_of_breed_list+i];
    else
        crmodel = get_players_special_digger_model(my_player_number);
    // Get scale factor
    int units_per_px = (gbtn->width * 16 + 32 / 2) / 32;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 288, 113);
    // Now draw the button
    long cr_total = 0;
    if ((crmodel > 0) && (crmodel < CREATURE_TYPES_COUNT) && (gbtn->flags & LbBtnF_Enabled))
    {
        struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
        int spridx = gbtn->sprite_idx;
        if (gbtn->content != NULL)
        {
          cr_total = *(long *)gbtn->content;
          if (cr_total > 0)
          {
            i = dungeon->guijob_angry_creatrs_count[crmodel][(job_idx & 0x03)];
            if (i > cr_total)
            {
              WARNDBG(7,"Creature %d stats inconsistency; total=%d, doing activity%d=%d",crmodel,cr_total,(job_idx & 0x03),i);
              i = cr_total;
            }
            if (i < 0)
            {
              i = 0;
            }
            spridx += 14 * i / cr_total;
          }
        }
        if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
        {
          draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y-2*units_per_px/16, ps_units_per_px, spridx, 12);
        } else
        {
          draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y-2*units_per_px/16, ps_units_per_px, spridx);
        }
        if (gbtn->content != NULL)
        {
          sprintf(gui_textbuf, "%ld", cr_total);
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

long anger_get_creature_highest_anger_type_and_byte_percentage(struct Thing *creatng, long *out_angr_typ, long *out_angr_prct)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    long angr_lmt = crstat->annoy_level;
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
        long angr_typ;
        long angr_prct;
        anger_get_creature_highest_anger_type_and_byte_percentage(ctrltng, &angr_typ, &angr_prct);
        int angr_pos = 5 * angr_prct / 256;
        if (angr_pos < 0) {
            angr_pos = 0;
        } else
        if (angr_pos > 4) {
            angr_pos = 4;
        }
        int spr_idx = angr_pos + 468;
        int shift_x = (48 * angr_prct - 16) / 256;
        if (shift_x < 0) {
            shift_x = 0;
        } else
        if ( shift_x > 36 ) {
            shift_x = 36;
        }
        draw_gui_panel_sprite_left(gbtn->scr_pos_x + (shift_x - 12) * units_per_px / 16, gbtn->scr_pos_y - 22 * units_per_px / 16, ps_units_per_px, spr_idx);
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
        struct CreatureStats* crstat = creature_stats_get_from_thing(ctrltng);
        struct CreatureControl* cctrl = creature_control_get_from_thing(ctrltng);
        long points_progress = cctrl->exp_points;
        long points_required = (crstat->to_level[cctrl->explevel] << 8);
        gui_area_progress_bar_med2(gbtn, units_per_px, points_progress, points_required);
        char* text = buf_sprintf("%d", (int)(cctrl->explevel + 1));
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
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 463, 100);
    struct Thing* ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (!thing_is_creature(ctrltng))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 463);
        gui_area_progress_bar_short(gbtn, units_per_px, 0, 32);
        return;
    }
    int curbtn_avail_pos = (long)gbtn->content;
    int curbtn_inst_id = creature_instance_get_available_id_for_pos(ctrltng, curbtn_avail_pos);
    if (!creature_instance_is_available(ctrltng, curbtn_inst_id))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 463);
        gui_area_progress_bar_short(gbtn, units_per_px, 0, 32);
        return;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(ctrltng);
    int spr_idx;
    if (cctrl->active_instance_id == curbtn_inst_id) {
      spr_idx = 462;
    } else {
      spr_idx = 463;
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
    int tx_units_per_px = (gbtn->height * 11 / 12) * 16 / LbTextLineHeight();
    const char* text = buf_sprintf("%d", (curbtn_avail_pos + 1) % 10);
    LbTextDrawResized(gbtn->scr_pos_x + 52*units_per_px/16, gbtn->scr_pos_y + 9*units_per_px/16, tx_units_per_px, text);
    spr_idx = gbtn->sprite_idx;

    long spkind = inst_inf->func_params[0];
    if (!creature_instance_has_reset(ctrltng, curbtn_inst_id) || ((spkind != 0) && thing_affected_by_spell(ctrltng, spkind)))
      spr_idx++;
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
    int curbtn_avail_pos = (long)gbtn->content;
    int curbtn_inst_id = creature_instance_get_available_id_for_pos(ctrltng, curbtn_avail_pos);
    gbtn->sprite_idx = instance_button_init[curbtn_inst_id].symbol_spridx;
    gbtn->tooltip_stridx = instance_button_init[curbtn_inst_id].tooltip_stridx;
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
        if ( (i > 0) && (top_of_breed_list+i < CREATURE_TYPES_COUNT) )
            crmodel = breed_activities[top_of_breed_list+i];
        else
            crmodel = get_players_special_digger_model(my_player_number);
        // Clear activity list
        activity_list[4*i+0] = 0;
        activity_list[4*i+1] = 0;
        activity_list[4*i+2] = 0;
        for (int n = 0; n < 15; n++)
        {
            int job_idx = state_type_to_gui_state[n];
            switch (job_idx)
            {
            case 0:
                activity_list[4*i+0] += dungeon->field_64[crmodel][n];
                break;
            case 1:
                activity_list[4*i+1] += dungeon->field_64[crmodel][n];
                break;
            case 2:
                activity_list[4*i+2] += dungeon->field_64[crmodel][n];
                break;
            default:
                ERRORLOG("Outranged GUI state value %d",(int)job_idx);
                break;
            }
        }
    }
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    int units_per_px = gmnu->width * 16 / 140;
    LbDrawBox(gmnu->pos_x + 2*units_per_px/16, gmnu->pos_y + 218*units_per_px/16, 134*units_per_px/16, 24*units_per_px/16, colours[0][0][0]);
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
    if (wheel_scrolled_up & lbKeyOn[KC_LSHIFT])
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
    if (wheel_scrolled_down & lbKeyOn[KC_LSHIFT])
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
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
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
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
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
    PlayerNumber plyr_idx = (int)gbtn->content;
    int spr_idx = 498;
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        return;
    }
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 498, 100);
    if (game.play_gameturn & 1)
    {
        struct PlayerInfo* player = get_my_player();
        if (player_allied_with(player, plyr_idx)) {
            spr_idx = 488 + (plyr_idx & 0x0f);
        }
    } else
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if (player_allied_with(player, my_player_number)) {
            spr_idx = 488 + (plyr_idx & 0x0f);
        }
    }
    if ( gbtn->gbactn_1 || gbtn->gbactn_2 )
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
    }
}

void gui_area_stat_button(struct GuiButton *gbtn)
{
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 459, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 459);
    struct PlayerInfo* player = get_my_player();
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    if (!thing_exists(thing))
        return;
    if (thing->class_id == TCls_Creature)
    {
        const char* text = creature_statistic_text(thing, (long)gbtn->content);
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 6*ps_units_per_px/16, gbtn->scr_pos_y - 12*ps_units_per_px/16, ps_units_per_px, gbtn->sprite_idx);
        draw_button_string(gbtn, 60, text);
    }
}

void maintain_event_button(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    EventIndex evidx;
    unsigned long evbtn_idx = (unsigned long)gbtn->content;
    long keycode;
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
                int i = EVENT_BUTTONS_COUNT;
                for (i=EVENT_BUTTONS_COUNT; i > 0; i--)
                {
                    struct Event* evloop = &game.event[i];
                    if((evloop->kind > 0) && (evloop->owner == my_player_number))
                    {
                        activate_event_box(i);
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
      gbtn->gbactn_1 = 0;
      gbtn->gbactn_2 = 0;
      gbtn->tooltip_stridx = GUIStr_Empty;
      return;
    }
    if ((event->kind == EvKind_Objective) && (new_objective))
    {
        activate_event_box(evidx);
    }
    gbtn->sprite_idx = event_button_info[event->kind].bttn_sprite;
    if (((event->kind == EvKind_FriendlyFight) || (event->kind == EvKind_EnemyFight))
        && ((event->mappos_x != 0) || (event->mappos_y != 0)) && ((game.play_gameturn & 0x01) != 0))
    {
        // Fight icon flashes when there are fights to show
        gbtn->sprite_idx += 2;
        if(is_game_key_pressed(Gkey_ZoomToFight, &keycode, true) && lbKeyOn[KC_LSHIFT])
        {
            if ((evidx == dungeon->visible_event_idx)) 
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
      && (event->target < 0) && ((game.play_gameturn & 0x01) != 0))
    {
        // Unread information flashes
        gbtn->sprite_idx += 2;
    }
    gbtn->tooltip_stridx = event_button_info[event->kind].tooltip_stridx;
    gbtn->flags |= LbBtnF_Enabled;
    gbtn->btype_value &= LbBFeF_IntValueMask;
}

void gui_toggle_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = (int)gbtn->content;
    if ((gbtn->flags & LbBtnF_Enabled) != 0) {
        struct Packet* pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_PlyrToggleAlly, plyr_idx, 0, 0, 0);
    }
}

void maintain_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = (int)gbtn->content;
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
        gbtn->sprite_idx = 350;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->sprite_idx = 354;
        gbtn->flags &= ~LbBtnF_Enabled;
        /*if (gbtn->gbactn_1) - this does nothing, but was in original function
        {
            menu_id_to_number(7);
        }*/
    }
}

void maintain_room_button(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = (int)gbtn->content;
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_exists(player))
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value |= LbBFeF_NoMouseOver;
        gbtn->flags &= ~LbBtnF_Enabled;
        gbtn->tooltip_stridx = 201;
    }
}
void maintain_creature_button(struct GuiButton* gbtn)
{
    PlayerNumber plyr_idx = (int)gbtn->content;
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_exists(player))
    {
        if (player_has_heart(plyr_idx))
        {
            gbtn->sprite_idx = 323 + (plyr_idx * 2);
        }
        else
        {
            gbtn->sprite_idx = 535 + (plyr_idx);
        }
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    }
    else
    {
        gbtn->btype_value |= LbBFeF_NoMouseOver;
        gbtn->flags &= ~LbBtnF_Enabled;
        gbtn->tooltip_stridx = 201;
    }
}

void pick_up_next_wanderer(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(-1, CrGUIJob_Wandering, my_player_number, pick_flags);
}

void gui_go_to_next_wanderer(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(-1, CrGUIJob_Wandering, pick_flags);
}

void pick_up_next_worker(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(-1, CrGUIJob_Working, my_player_number, pick_flags);
}

void gui_go_to_next_worker(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(-1, CrGUIJob_Working, pick_flags);
}

void pick_up_next_fighter(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(1);
    pick_up_creature_of_model_and_gui_job(-1, CrGUIJob_Fighting, my_player_number, pick_flags);
}

void gui_go_to_next_fighter(struct GuiButton *gbtn)
{
    unsigned short pick_flags = get_creature_pick_flags(0);
    go_to_next_creature_of_model_and_gui_job(-1, CrGUIJob_Fighting, pick_flags);
}

void gui_area_payday_button(struct GuiButton *gbtn)
{
    int units_per_px = (gbtn->width * 16 + 132 / 2) / 132;
    int ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, gbtn->sprite_idx, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
    gui_area_progress_bar_wide(gbtn, units_per_px, game.pay_day_progress, game.pay_day_gap);
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    char* text = buf_sprintf("%d", (int)dungeon->creatures_total_pay);
    draw_centred_string64k(text, gbtn->scr_pos_x + (gbtn->width >> 1), gbtn->scr_pos_y + 8*units_per_px/16, 130, gbtn->width);
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
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 462, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 462);
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
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 462, 100);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 462);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x - 8, gbtn->scr_pos_y - 10, ps_units_per_px, gbtn->sprite_idx);
    gui_area_progress_bar_short(gbtn, units_per_px, manufct_progress, manufct_required);
}

void gui_area_player_creature_info(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = (int)gbtn->content;
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 459, 100);
    struct PlayerInfo* player = get_player(plyr_idx);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 459);
    struct Dungeon* dungeon = get_players_dungeon(player);
    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        if (((dungeon->num_active_creatrs < dungeon->max_creatures_attracted) && (!game.pool.is_empty))
            || ((game.play_gameturn & 1) != 0))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
        } else
        {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx, 44);
        }
        long i = dungeon->num_active_creatrs;
        char* text = buf_sprintf("%ld", i);
        draw_button_string(gbtn, 60, text);
    }
}

void gui_area_player_room_info(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx = (int)gbtn->content;
    int ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 459, 100);
    struct PlayerInfo* player = get_player(plyr_idx);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 459);
    struct Dungeon* dungeon = get_players_dungeon(player);
    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
        long i = dungeon->total_rooms;
        char* text = buf_sprintf("%ld", i);
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

void draw_gold_total(PlayerNumber plyr_idx, long scr_x, long scr_y, long units_per_px, long long value)
{
    long long i;
    unsigned int flg_mem = lbDisplay.DrawFlags;
    int ndigits = 0;
    int val_width = 0;
    for (i = value; i > 0; i /= 10) {
        ndigits++;
    }
    struct TbSprite* spr = &button_sprite[71];
    val_width = scale_value_for_resolution_with_upp(spr->SWidth, units_per_px) * ndigits;
    if (ndigits > 0)
    {
        long pos_x = scr_x + val_width / 2;
        for (i = value; i > 0; i /= 10)
        {
            // Make space for the character first, as we're drawing right char towards left
            pos_x -= scale_value_for_resolution_with_upp(spr->SWidth, units_per_px);
            spr = &button_sprite[i % 10 + 71];
            LbSpriteDrawResized(pos_x, scr_y, units_per_px, spr);
        }
    } else
    {
        // Just draw zero
        spr = &button_sprite[71];
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
    int bs_units_per_px;
    {
        int mnu_num = menu_id_to_number(GMnu_MAIN);
        gmnu = get_active_menu(mnu_num);
        mm_units_per_px = (gmnu->width * 16 + 140/2) / 140;
        if (mm_units_per_px < 1)
            mm_units_per_px = 1;
        fs_units_per_px = (gmnu->height * 16 + 8) / LbTiledSpriteHeight(&status_panel, gui_panel_sprites);
        bs_units_per_px = gmnu->width * 4 / 35;
    }
    lbDisplay.DrawColour = colours[15][15][15];
    lbDisplay.DrawFlags = 0;
    LbTiledSpriteDraw(0, 0, fs_units_per_px, &status_panel, gui_panel_sprites);
    // Draws gold amount; note that button_sprite[] is used instead of full font
    draw_gold_total(player->id_number, gmnu->pos_x + gmnu->width/2, gmnu->pos_y + gmnu->height*67/200, fs_units_per_px, dungeon->total_money_owned);
    if (16/mm_units_per_px < 3)
        mmzoom = (player->minimap_zoom) / scale_value_for_resolution_with_upp(2,mm_units_per_px);
    else
        mmzoom = player->minimap_zoom;
    pannel_map_draw_slabs(player->minimap_pos_x, player->minimap_pos_y, mm_units_per_px, mmzoom);
    long basic_zoom = player->minimap_zoom;
    pannel_map_draw_overlay_things(mm_units_per_px, mmzoom, basic_zoom);
}

void gui_set_button_flashing(long btn_idx, long gameturns)
{
    game.flash_button_index = btn_idx;
    game.flash_button_gameturns = gameturns;
}

void update_room_tab_to_config(void)
{
    int i;
    // Clear 4x4 area of buttons, but skip "sell" button at end
    for (i=0; i < 4*4-1; i++)
    {
        struct GuiButtonInit* ibtn = &room_menu.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = 0;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
    }
    for (i=0; i < slab_conf.room_types_count; i++)
    {
        struct RoomConfigStats* roomst = &slab_conf.room_cfgstats[i];
        if (roomst->panel_tab_idx < 1)
            continue;
        struct GuiButtonInit* ibtn = &room_menu.buttons[roomst->panel_tab_idx - 1];
        ibtn->sprite_idx = roomst->medsym_sprite_idx;
        ibtn->tooltip_stridx = roomst->tooltip_stridx;
        ibtn->content.lval = i;
        ibtn->click_event = gui_choose_room;
        ibtn->rclick_event = gui_go_to_next_room;
        ibtn->ptover_event = gui_over_room_button;
        ibtn->draw_call = gui_area_room_button;
    }
}

void update_trap_tab_to_config(void)
{
    int i;
    // Clear 4x4 area of buttons, but skip "sell" button at end
    for (i=0; i < 4*4-1; i++)
    {
        struct GuiButtonInit* ibtn = &trap_menu.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = 0;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
        ibtn->maintain_call = NULL;
    }
    for (i=0; i < gameadd.trapdoor_conf.manufacture_types_count; i++)
    {
        struct ManufactureData* manufctr = get_manufacture_data(i);
        if (manufctr->panel_tab_idx < 1)
            continue;
        struct GuiButtonInit* ibtn = &trap_menu.buttons[manufctr->panel_tab_idx - 1];
        ibtn->sprite_idx = manufctr->medsym_sprite_idx;
        ibtn->tooltip_stridx = manufctr->tooltip_stridx;
        ibtn->content.lval = i;
        switch (manufctr->tngclass)
        {
        case TCls_Trap:
            ibtn->click_event = gui_choose_trap;
            ibtn->rclick_event = gui_go_to_next_trap;
            ibtn->ptover_event = gui_over_trap_button;
            ibtn->draw_call = gui_area_trap_button;
            ibtn->maintain_call = maintain_trap;
            break;
        case TCls_Door:
            ibtn->click_event = gui_choose_trap;
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
    if ( menu_is_active(GMnu_TRAP) )
    {
        turn_off_menu(GMnu_TRAP);
        turn_on_menu(GMnu_TRAP);
    }
}

void update_powers_tab_to_config(void)
{
    int i;
    // Clear 4x4 area of buttons, no "sell" button at end
    for (i=0; i < 4*4; i++)
    {
        struct GuiButtonInit* ibtn = &spell_menu.buttons[i];
        ibtn->sprite_idx = 24;
        ibtn->tooltip_stridx = GUIStr_Empty;
        ibtn->content.lval = 0;
        ibtn->click_event = NULL;
        ibtn->rclick_event = NULL;
        ibtn->ptover_event = NULL;
        ibtn->draw_call = gui_area_new_null_button;
        ibtn->maintain_call = NULL;
    }
    for (i=0; i < magic_conf.power_types_count; i++)
    {
        struct PowerConfigStats* powerst = get_power_model_stats(i);
        if (powerst->panel_tab_idx < 1)
            continue;
        struct GuiButtonInit* ibtn = &spell_menu.buttons[powerst->panel_tab_idx - 1];
        ibtn->sprite_idx = powerst->medsym_sprite_idx;
        ibtn->tooltip_stridx = powerst->tooltip_stridx;
        ibtn->content.lval = i;
        if (is_special_power(i)) {
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
/******************************************************************************/
