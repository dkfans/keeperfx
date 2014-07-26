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
#include "config_creature.h"
#include "config_magic.h"
#include "config_trapdoor.h"
#include "room_workshop.h"
#include "gui_frontbtns.h"
#include "gui_parchment.h"
#include "gui_draw.h"
#include "packets.h"
#include "magic.h"
#include "player_instances.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_opts.h"
#include "frontmenu_ingame_map.h"
#include "frontend.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "vidfade.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_draw_gold_total(unsigned char plyr_idx, long a2, long a3, long a4);
DLLIMPORT void _DK_gui_zoom_in(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_zoom_out(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_map(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_turn_on_autopilot(struct GuiButton *gbtn);
DLLIMPORT void _DK_menu_tab_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_autopilot_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_turn_on_autopilot(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_event_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_remove_area_for_rooms(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_big_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_spell_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_special_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_big_spell_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_choose_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_over_trap_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_trap_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_door(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_door(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_over_door_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_remove_area_for_traps(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_big_trap_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_trap(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_creature_query_background1(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_creature_query_background2(struct GuiMenu *gmnu);
DLLIMPORT void _DK_maintain_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_big_spell(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_creature_doing_activity(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_creature_activity(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_room(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_over_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_room_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_creature(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_anger_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_smiley_anger_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_experience_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_instance_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_instance(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_wanderer(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_wanderer(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_worker(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_worker(struct GuiButton *gbtn);
DLLIMPORT void _DK_pick_up_next_fighter(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_go_to_next_fighter(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_scroll_activity_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_pic(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_activity_row(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_activity_background(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_area_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_stat_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_event_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_toggle_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_ally(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_prison_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_maintain_room_and_creature_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_payday_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_research_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_workshop_bar(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_player_creature_info(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_player_room_info(struct GuiButton *gbtn);
DLLIMPORT void _DK_spell_lost_first_person(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_tend_to(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_query(struct GuiButton *gbtn);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void gui_zoom_in(struct GuiButton *gbtn)
{
    //_DK_gui_zoom_in(gbtn);
    struct PlayerInfo *player;
    player = get_my_player();
    if (player->minimap_zoom > 0x80) {
        set_players_packet_action(player, PckA_SetMinimapConf, player->minimap_zoom >> 1, 0, 0, 0);
    }
}

void gui_zoom_out(struct GuiButton *gbtn)
{
    //_DK_gui_zoom_out(gbtn);
    struct PlayerInfo *player;
    player = get_my_player();
    if (player->minimap_zoom < 0x800) {
        set_players_packet_action(player, PckA_SetMinimapConf, player->minimap_zoom << 1, 0, 0, 0);
    }
}

void gui_go_to_map(struct GuiButton *gbtn)
{
    zoom_to_patchment_map();
}

void gui_turn_on_autopilot(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    player = get_my_player();
    if (player->victory_state != VicS_LostLevel)
    {
      set_players_packet_action(player, PckA_ToggleComputer, 0, 0, 0, 0);
    }
}

void menu_tab_maintain(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    player = get_my_player();
    set_flag_byte(&gbtn->flags, LbBtnF_Unknown08, (player->victory_state != VicS_LostLevel));
}

void gui_area_autopilot_button(struct GuiButton *gbtn)
{
    //_DK_gui_area_autopilot_button(gbtn);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    int spr_idx;
    spr_idx = gbtn->field_29;
    if (gbtn->gbtype == Lb_CYCLEBTN) {
        ERRORLOG("Cycle button cannot have a normal button draw function!");
    }
    int units_per_px;
    units_per_px = simple_button_sprite_height_units_per_px(gbtn, spr_idx, 100);
    if ((gbtn->flags & 0x08) != 0)
    {
        if ((dungeon->computer_enabled & 0x01) != 0)
        {
          if (game.play_gameturn & 1)
            spr_idx += 2;
        }
        if ((gbtn->gbactn_1 == 0) && (gbtn->gbactn_2 == 0))
          spr_idx += 1;
        draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, units_per_px, spr_idx);
    }
    else
    {
        draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, units_per_px, spr_idx, 12);
    }
}

void maintain_turn_on_autopilot(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    unsigned long cplr_model;
    player = get_my_player();
    struct Computer2 *comp;
    comp = get_computer_player(player->id_number);
    cplr_model = comp->model;
    if ((cplr_model >= 0) && (cplr_model < 10)) {
        gbtn->tooltip_id = computer_types[cplr_model];
    } else {
        ERRORLOG("Illegal computer player");
    }
}

void gui_choose_room(struct GuiButton *gbtn)
{
    // prepare to enter room build mode
    activate_room_build_mode((long)gbtn->content, gbtn->tooltip_id);
}

void gui_area_event_button(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    unsigned long i;
    if ((gbtn->flags & LbBtnF_Unknown08) != 0)
    {
        int ps_units_per_px;
        ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 258, 100);
        dungeon = get_players_num_dungeon(my_player_number);
        i = (unsigned long)gbtn->content;
        if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->field_29);
        } else
        if ((i <= EVENT_BUTTONS_COUNT) && (dungeon->event_button_index[i] == dungeon->visible_event_idx))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->field_29);
        } else
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->field_29+1);
        }
    }
}

void gui_remove_area_for_rooms(struct GuiButton *gbtn)
{
    //_DK_gui_remove_area_for_rooms(gbtn);
    game.chosen_room_kind = 0;
    game.chosen_room_look = 0;
    game.chosen_room_tooltip = 0;
    struct Packet *pckt;
    pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_SetPlyrState, PSt_Sell, 0, 0, 0);
}

long find_room_type_capacity_total_percentage(PlayerNumber plyr_idx, RoomKind rkind)
{
    int used_cap, total_cap;
    struct Dungeon *dungeon;
    long i;
    unsigned long k;
    used_cap = 0;
    total_cap = 0;
    dungeon = get_dungeon(plyr_idx);
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        struct Room *room;
        room = room_get(i);
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

long count_rooms_of_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    long i;
    unsigned long k;
    dungeon = get_dungeon(plyr_idx);
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        struct Room *room;
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return k;
}
void gui_area_big_room_button(struct GuiButton *gbtn)
{
    struct PlayerInfo * player;
    RoomKind rkind;
    rkind = (int)gbtn->content;
    player = get_my_player();
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);

    unsigned short flg_mem;
    flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 26, 100);

    if (rkind == 0) {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    int i;
    i = find_room_type_capacity_total_percentage(player->id_number, rkind);
    if ((rkind == RoK_ENTRANCE) || (rkind == RoK_DUNGHEART) || (i < 0))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 23);
        int fill_part;
        fill_part = 42 - (2 * 21 * i / 256);
        LbDrawBox(
          (gbtn->scr_pos_x - fill_part + 114) / pixel_size,
          (gbtn->scr_pos_y + 12) / pixel_size,
          fill_part / pixel_size,
          6 / pixel_size,
          colours[0][0][0]);
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;

    struct RoomStats *rstat;
    rstat = room_stats_get_for_kind(rkind);
    //game.chosen_room_kind
    sprintf(gui_textbuf, "%ld", (long)rstat->cost);
    if (rstat->cost <= dungeon->total_money_owned)
    {
        if ((player->work_state == 2) && (player->chosen_room_kind == game.chosen_room_kind)
          && ((game.play_gameturn & 1) == 0))
        {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29, 44);
        } else {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29);
        }
        // We will use a special coding for our "string" - we want chars to represent
        // sprite index directly, without code pages and multibyte chars interpretation
        for (i=0; gui_textbuf[i] != '\0'; i++)
            gui_textbuf[i] -= 120;
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29 + 1);
    }
    LbTextUseByteCoding(false);
    draw_string64k(gbtn->scr_pos_x + 44, gbtn->scr_pos_y + 8 - 6, gui_textbuf);
    LbTextUseByteCoding(true);

    long amount;
    amount = count_rooms_of_type(player->id_number, rkind);
    // Note that "@" is "x" in that font
    sprintf(gui_textbuf, "@%ld", amount);
    draw_string64k(gbtn->scr_pos_x + 40, gbtn->scr_pos_y - 14 - 6, gui_textbuf);
    lbDisplay.DrawFlags = flg_mem;
}

/**
 * Sets a new chosen spell.
 * Fills packet with the previous spell disable action.
 */
void gui_choose_spell(struct GuiButton *gbtn)
{
    //NOTE by Petter: factored out original gui_choose_spell code to choose_spell
    choose_spell((int) gbtn->content, gbtn->tooltip_id);
}

void go_to_next_spell_of_type(PowerKind pwkind, PlayerNumber plyr_idx)
{
    struct Packet *pckt;
    pckt = get_packet(plyr_idx);
    set_packet_action(pckt, PckA_ZoomToSpell, pwkind, 0, 0, 0);
}

void gui_go_to_next_spell(struct GuiButton *gbtn)
{
    PowerKind pwkind;
    //_DK_gui_go_to_next_spell(gbtn);
    pwkind = (int)gbtn->content;
    struct PlayerInfo * player;
    player = get_my_player();
    go_to_next_spell_of_type(pwkind, player->id_number);
    set_chosen_power(pwkind, gbtn->tooltip_id);
}

void gui_area_spell_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem;
    flg_mem = lbDisplay.DrawFlags;
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, 24, 128);
    PowerKind pwkind;
    pwkind = (long)gbtn->content;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 24);
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int spr_idx;
    if ((dungeon->magic_resrchable[pwkind]) || (dungeon->magic_level[pwkind] > 0))
    {
        if ((gbtn->flags & 0x08) != 0)
        {
            int i;
            i = spell_data[pwkind].field_4;
            if (((i == 6) && (dungeon->cta_start_turn != 0))
             || ((i == 8) && (dungeon->sight_casted_thing_idx != 0))
             || ((pwkind == PwrK_OBEY) && (dungeon->must_obey_turn != 0))) {
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 27);
            }
            spr_idx = gbtn->field_29;
            if (game.keeper_power_stats[pwkind].cost[0] > dungeon->total_money_owned)
                spr_idx++;
            TbBool drawn;
            drawn = false;
            if ((gbtn->gbactn_1 == 0) && (gbtn->gbactn_2 == 0))
            {
                if ((((i != 6) || (dungeon->cta_start_turn == 0))
                  && ((i != 8) || (dungeon->sight_casted_thing_idx == 0)))
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
                spr_idx = gbtn->field_29 + 1;
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
    choose_spell(((int) gbtn->content) % POWER_TYPES_COUNT, gbtn->tooltip_id);
}

void gui_area_big_spell_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem;
    flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 26, 100);

    PowerKind pwkind;
    pwkind = (long)gbtn->content;
    struct SpellData *pwrdata;
    pwrdata = get_power_data(pwkind);
    if (power_data_is_invalid(pwrdata))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    struct PlayerInfo *player;
    player = get_my_player();
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);

    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    int pwage;
    pwage = find_spell_age_percentage(player->id_number, pwkind);
    if ((pwrdata->flag_8 != 0) && (pwage >= 0))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 23);
        int fill_bar;
        fill_bar = 42 - (2 * 21 * pwage / 256);
        LbDrawBox((gbtn->scr_pos_x - fill_bar + 114) / pixel_size, (gbtn->scr_pos_y + 12) / pixel_size,
          fill_bar / pixel_size, 6 / pixel_size, colours[0][0][0]);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, 26);
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;

    GoldAmount price;
    price = compute_power_price(dungeon->owner, pwkind, 0);
    char *text;
    text = buf_sprintf("%ld", (long)price);
    if (dungeon->total_money_owned >= price)
    {
        if ((player->work_state == pwrdata->field_4) && ((game.play_gameturn & 1) != 0)) {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29, 44);
        } else {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29);
        }
        char *c;
        for (c=text; *c != 0; c++) {
            *c = (*c) - 120;
        }
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29 + 1);
    }
    draw_string64k(gbtn->scr_pos_x + 44, gbtn->scr_pos_y + 8 - 6, text);
    lbDisplay.DrawFlags = flg_mem;
}

/**
 * Choose a trap or a door.
 * @param manufctr_idx An index into manufacture data array, beware as this is different from models.
 * @param tooltip_id The tooltip string to display.
 */
void choose_workshop_item(int manufctr_idx, TextStringId tooltip_id)
{
    struct PlayerInfo * player;
    struct ManufactureData *manufctr;

    player = get_my_player();
    manufctr = get_manufacture_data(manufctr_idx);
    set_players_packet_action(player, PckA_SetPlyrState, manufctr->work_state,
        manufctr->tngmodel, 0, 0);

    game.manufactr_element = manufctr_idx;
    game.numfield_15181D = manufctr->field_8;
    game.manufactr_tooltip = tooltip_id;
}

void gui_choose_trap(struct GuiButton *gbtn)
{
    //_DK_gui_choose_trap(gbtn);

    //Note by Petter: factored out gui_choose_trap to choose_workshop_item (better name as well)
    choose_workshop_item((int) gbtn->content, gbtn->tooltip_id);
}

void go_to_next_trap_of_type(ThingModel tngmodel, PlayerNumber plyr_idx)
{
    static unsigned short seltrap[8];
    struct Thing *thing;
    int i;
    unsigned long k;
    if (tngmodel >= 8) {
        ERRORLOG("Bad trap kind");
        return;
    }
    k = 0;
    i = seltrap[tngmodel];
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
        struct Packet *pckt;
        pckt = get_packet(plyr_idx);
        set_packet_action(pckt, PckA_ZoomToTrap, i, 0, 0, 0);
    }
}

void go_to_next_door_of_type(ThingModel tngmodel, PlayerNumber plyr_idx)
{
    static unsigned short seldoor[8];
    struct Thing *thing;
    int i;
    unsigned long k;
    if (tngmodel >= 8) {
        ERRORLOG("Bad door kind");
        return;
    }
    k = 0;
    i = seldoor[tngmodel];
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
        struct Packet *pckt;
        pckt = get_packet(plyr_idx);
        set_packet_action(pckt, PckA_ZoomToDoor, i, 0, 0, 0);
    }
}

void gui_go_to_next_trap(struct GuiButton *gbtn)
{
    struct PlayerInfo * player;
    int manufctr_idx;
    manufctr_idx = (int)gbtn->content;
    player = get_my_player();
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    //_DK_gui_go_to_next_trap(gbtn);
    go_to_next_trap_of_type(manufctr->tngmodel, player->id_number);
    game.manufactr_element = manufctr_idx;
    game.numfield_15181D = manufctr->field_8;
    game.manufactr_tooltip = gbtn->tooltip_id;
}

void gui_over_trap_button(struct GuiButton *gbtn)
{
    int manufctr_idx;
    //_DK_gui_over_trap_button(gbtn);
    manufctr_idx = (long)gbtn->content;
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    gui_trap_type_highlighted = manufctr->tngmodel;
}

void gui_area_trap_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem;
    int manufctr_idx;
    //_DK_gui_area_trap_button(gbtn);
    flg_mem = lbDisplay.DrawFlags;
    manufctr_idx = (long)gbtn->content;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 24);
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
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
    if ((gbtn->flags & LbBtnF_Unknown08) == 0)
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 25);
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    // Check how many traps/doors do we have to place
    unsigned int amount;
    switch (manufctr->tngclass)
    {
    case TCls_Trap:
        // If there are traps of that type placed on map
        if (find_trap_of_type(manufctr->tngmodel, my_player_number)) {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 27);
        }
        amount = dungeon->trap_amount_placeable[manufctr->tngmodel];
        break;
    case TCls_Door:
        // If there are doors of that type placed on map
        if (find_door_of_type(manufctr->tngmodel, my_player_number)) {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 27);
        }
        amount = dungeon->door_amount_placeable[manufctr->tngmodel];
        break;
    default:
        amount = 0;
        break;
    }
    int i;
    i = gbtn->field_29 + (amount < 1);
    if (gbtn->gbactn_1 || gbtn->gbactn_2)
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, i, 22);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, i);
    }
    lbDisplay.DrawFlags = flg_mem;
}

void gui_go_to_next_door(struct GuiButton *gbtn)
{
    struct PlayerInfo * player;
    int manufctr_idx;
    manufctr_idx = (int)gbtn->content;
    player = get_my_player();
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    //_DK_gui_go_to_next_door(gbtn);
    go_to_next_door_of_type(manufctr->tngmodel, player->id_number);
    game.manufactr_element = manufctr_idx;
    game.numfield_15181D = manufctr->field_8;
    game.manufactr_tooltip = gbtn->tooltip_id;
}

void gui_over_door_button(struct GuiButton *gbtn)
{
    int manufctr_idx;
    //_DK_gui_over_door_button(gbtn);
    manufctr_idx = (long)gbtn->content;
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    gui_door_type_highlighted = manufctr->tngmodel;
}

void gui_remove_area_for_traps(struct GuiButton *gbtn)
{
    //_DK_gui_remove_area_for_traps(gbtn);
    struct PlayerInfo *player;
    player = get_my_player();
    game.manufactr_element = 0;
    game.numfield_15181D = 0;
    game.manufactr_tooltip = 0;
    set_players_packet_action(player, PckA_SetPlyrState, PSt_Sell, 0, 0, 0);
}

void gui_area_big_trap_button(struct GuiButton *gbtn)
{
    //_DK_gui_area_big_trap_button(gbtn); return;
    struct PlayerInfo * player;
    int manufctr_idx;
    manufctr_idx = (int)gbtn->content;
    player = get_my_player();
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    unsigned short flg_mem;
    flg_mem = lbDisplay.DrawFlags;

    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, 26, 100);

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
        amount = dungeon->trap_amount_placeable[manufctr->tngmodel];
        break;
    case TCls_Door:
        amount = dungeon->door_amount_placeable[manufctr->tngmodel];
        break;
    default:
        amount = 0;
        break;
    }
    // Note that "@" is "x" in that font
    sprintf(gui_textbuf, "@%ld", (long)amount);
    if (amount <= 0) {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29 + 1);
    } else
    if ((((manufctr->tngclass == TCls_Trap) && (player->chosen_trap_kind == manufctr->tngmodel))
      || ((manufctr->tngclass == TCls_Door) && (player->chosen_door_kind == manufctr->tngmodel)))
      && ((game.play_gameturn & 1) == 0) )
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29, 44);
    } else {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, ps_units_per_px, gbtn->field_29);
    }
    draw_string64k(gbtn->scr_pos_x + 44, gbtn->scr_pos_y + 8 - 6, gui_textbuf);
    lbDisplay.DrawFlags = flg_mem;
}

void maintain_big_spell(struct GuiButton *gbtn)
{
    //_DK_maintain_big_spell(gbtn);
    long spl_idx;
    spl_idx = game.chosen_spell_type;
    if ((spl_idx < 0) || (spl_idx >= KEEPER_SPELLS_COUNT)) {
        return;
    }
    gbtn->content = (unsigned long *)spl_idx;
    gbtn->field_29 = game.chosen_spell_look;
    gbtn->tooltip_id = game.chosen_spell_tooltip;
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    if (dungeon->magic_level[spl_idx] > 0) {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else {
        gbtn->field_1B |= 0x8000;
        gbtn->flags &= ~LbBtnF_Unknown08;
    }
}

void maintain_room(struct GuiButton *gbtn)
{
    //_DK_maintain_room(gbtn);
    RoomKind rkind;
    rkind = (long)gbtn->content;
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    if ((rkind < 1) || (rkind >= ROOM_TYPES_COUNT)) {
        return;
    }
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon");
        return;
    }
    if (dungeon->room_buildable[rkind]) {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else {
        gbtn->field_1B |= 0x8000;
        gbtn->flags &= ~LbBtnF_Unknown08;
    }
}

void maintain_big_room(struct GuiButton *gbtn)
{
    //_DK_maintain_big_room(gbtn);
    long rkind;
    rkind = game.chosen_room_kind;
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    if ((rkind < 1) || (rkind >= ROOM_TYPES_COUNT)) {
        return;
    }
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Invalid dungeon");
        return;
    }
    gbtn->content = (unsigned long *)rkind;
    gbtn->field_29 = game.chosen_room_look;
    gbtn->tooltip_id = game.chosen_room_tooltip;
    if (dungeon->room_buildable[rkind]) {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else {
        gbtn->field_1B |= 0x8000;
        gbtn->flags &= ~LbBtnF_Unknown08;
    }
}

void maintain_spell(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  long i;
  player = get_my_player();
  dungeon = get_players_dungeon(player);
  i = (unsigned long)(gbtn->content) & 0xff;
  if (!is_power_available(player->id_number,i))
  {
    gbtn->field_1B |= 0x8000u;
    gbtn->flags &= ~LbBtnF_Unknown08;
  } else
  if (i == 19)
  {
      if (game.armageddon_cast_turn != 0)
      {
        gbtn->field_1B |= 0x8000u;
        gbtn->flags &= ~LbBtnF_Unknown08;
      } else
      {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
      }
  } else
  if (i == 9)
  {
      if (dungeon->hold_audience_field_88C)
      {
        gbtn->field_1B |= 0x8000u;
        gbtn->flags &= ~LbBtnF_Unknown08;
      } else
      {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
      }
  } else
  {
    gbtn->field_1B = 0;
    gbtn->flags |= LbBtnF_Unknown08;
  }
}

void maintain_trap(struct GuiButton *gbtn)
{
    int manufctr_idx;
    manufctr_idx = (unsigned int)gbtn->content;
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    //_DK_maintain_trap(gbtn);
    if (is_trap_placeable(my_player_number, manufctr->tngmodel) || is_trap_built(my_player_number, manufctr->tngmodel))
    {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else
    {
        gbtn->field_1B |= 0x8000u;
        gbtn->flags &= ~LbBtnF_Unknown08;
    }
}

void maintain_door(struct GuiButton *gbtn)
{
    int manufctr_idx;
    manufctr_idx = (unsigned int)gbtn->content;
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    if (is_door_placeable(my_player_number, manufctr->tngmodel) || is_door_built(my_player_number, manufctr->tngmodel))
    {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else
    {
        gbtn->field_1B |= 0x8000u;
        gbtn->flags &= ~LbBtnF_Unknown08;
    }
}

void maintain_big_trap(struct GuiButton *gbtn)
{
    int manufctr_idx;
    //_DK_maintain_big_trap(gbtn);
    manufctr_idx = game.manufactr_element%MANUFCTR_TYPES_COUNT;
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    gbtn->content = (unsigned long *)manufctr_idx;
    gbtn->field_29 = game.numfield_15181D;
    gbtn->tooltip_id = game.manufactr_tooltip;
    if ( ((manufctr->tngclass == TCls_Trap) && is_trap_placeable(my_player_number, manufctr->tngmodel))
      || ((manufctr->tngclass == TCls_Door) && is_door_placeable(my_player_number, manufctr->tngmodel)) )
    {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else
    {
        gbtn->field_1B |= 0x8000u;
        gbtn->flags &= ~LbBtnF_Unknown08;
    }
}

void draw_centred_string64k(const char *text, short x, short y, short w)
{
    unsigned long flg_mem;
    flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    LbTextSetJustifyWindow((x - (w / 2)) / pixel_size, y / (int)pixel_size, w / (int)pixel_size);
    LbTextSetClipWindow( (x - (w / 2)) / pixel_size, y / (int)pixel_size, w / (int)pixel_size, 16 / pixel_size);
    lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    LbTextDraw(0 / pixel_size, -6 / pixel_size, text);
    LbTextSetJustifyWindow(0 / pixel_size, 0 / pixel_size, 640 / pixel_size);
    LbTextSetClipWindow(0 / pixel_size, 0 / pixel_size, 640 / pixel_size, 480 / pixel_size);
    LbTextSetWindow(0 / pixel_size, 0 / pixel_size, MyScreenWidth / pixel_size, MyScreenHeight / pixel_size);
    lbDisplay.DrawFlags = flg_mem;
}

void draw_name_box(long x, long y, struct Thing *thing)
{
    draw_gui_panel_sprite_left(x, y, 16, 458);
    if (thing_is_creature(thing) && (thing->ccontrol_idx > 0))
    {
        // Draw health bar
        struct PlayerInfo *player;
        player = get_my_player();
        struct Thing *ctrltng;
        ctrltng = thing_get(player->controlled_thing_idx);
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(ctrltng);
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(ctrltng);
        long maxhealth, curhealth;
        maxhealth = compute_creature_max_health(crstat->health,cctrl->explevel);
        curhealth = ctrltng->health;
        if (curhealth <= 0) {
            curhealth = 0;
        } else
        if (curhealth > maxhealth) {
            curhealth = maxhealth;
        }
        //draw_percentage_bar();
        if (maxhealth > 0)
        {
            long i, bar_fill;
            i = 63 * curhealth / maxhealth;
            bar_fill = 126 - 2 * i;
            if (bar_fill < 0) {
                bar_fill = 0;
            } else
            if (bar_fill > 126) {
                bar_fill = 126;
            }
            LbDrawBox((x - bar_fill + 128) / pixel_size, (y + 4) / pixel_size, bar_fill / pixel_size, 14 / pixel_size, colours[0][0][0]);
        }
        // Draw creature name
        const char *text;
        text = creature_own_name(thing);
        draw_centred_string64k(text, x + 63, y + 2, 120);
    }
}

void gui_creature_query_background1(struct GuiMenu *gmnu)
{
    SYNCDBG(19,"Starting");
    //_DK_gui_creature_query_background1(gmnu);
    struct PlayerInfo *player;
    player = get_my_player();
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    draw_name_box(gmnu->pos_x + 4, gmnu->pos_y + 262, ctrltng);
    if (thing_is_creature(ctrltng) && (ctrltng->ccontrol_idx > 0))
    {
        long spr_idx;
        spr_idx = get_creature_model_graphics(ctrltng->model, CGI_ButtonPortrait);
        LbSpriteDraw((gmnu->pos_x + 16) / pixel_size, (gmnu->pos_y + 200) / pixel_size, &button_sprite[spr_idx]);
    }
    draw_gui_panel_sprite_left(gmnu->pos_x + 4, gmnu->pos_y + 188, 16, 464);
}

void gui_creature_query_background2(struct GuiMenu *gmnu)
{
    SYNCDBG(19,"Starting");
    //_DK_gui_creature_query_background2(gmnu);
    struct PlayerInfo *player;
    player = get_my_player();
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    draw_name_box(gmnu->pos_x + 4, gmnu->pos_y + 200, ctrltng);
    if (thing_is_creature(ctrltng) && (ctrltng->ccontrol_idx > 0))
    {
        long spr_idx;
        spr_idx = get_creature_model_graphics(ctrltng->model, CGI_GUIPanelSymbol);
        draw_gui_panel_sprite_left(gmnu->pos_x + 4, gmnu->pos_y + 196, 16, spr_idx);
    }
}

void pick_up_creature_doing_activity(struct GuiButton *gbtn)
{
    long i;
    unsigned char pick_flags;
    SYNCDBG(8,"Starting");
    //_DK_pick_up_creature_doing_activity(gbtn); return;
    i = gbtn->field_1B;
    ThingModel crmodel;
    if (i > 0)
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    else
        crmodel = get_players_special_digger_model(my_player_number);
    // Get index from pointer
    long job_idx;
    job_idx = ((long *)gbtn->content - &activity_list[0]);
    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        pick_flags |= TPF_OrderedPick;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_OrderedPick | TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(crmodel, (job_idx & 0x03), my_player_number, pick_flags);
}

void gui_go_to_next_creature_activity(struct GuiButton *gbtn)
{
    //_DK_gui_go_to_next_creature_activity(gbtn);
    ThingModel crmodel;
    int i;
    i = gbtn->field_1B;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    // Get index from pointer
    int job_idx;
    job_idx = ((long *)gbtn->content - &activity_list[0]);
    go_to_next_creature_of_model_and_gui_job(crmodel, (job_idx & 0x3));
}

RoomIndex find_my_next_room_of_type(RoomKind rkind)
{
    static RoomIndex next_room[ROOM_TYPES_COUNT];
    if (next_room[rkind] > 0)
    {
        struct Room *room;
        room = room_get(next_room[rkind]);
        if (room_exists(room) && (room->owner == my_player_number) && (room->kind == rkind))
          next_room[rkind] = room->next_of_owner;
        else
          next_room[rkind] = 0;
    }
    if (next_room[rkind] <= 0)
    {
        struct Dungeon *dungeon;
        dungeon = get_my_dungeon();
        next_room[rkind] = dungeon->room_kind[rkind];
    }
    return next_room[rkind];
}

void go_to_my_next_room_of_type_and_select(unsigned long rkind)
{
    RoomIndex room_idx;
    room_idx = find_my_next_room_of_type(rkind);
    struct PlayerInfo *player;
    player = get_my_player();
    if (room_idx > 0) {
        set_players_packet_action(player, PckA_ZoomToRoom, room_idx, 0, 0, 0);
    }
}

void gui_go_to_next_room(struct GuiButton *gbtn)
{
    //_DK_gui_go_to_next_room(gbtn);
    unsigned long rkind;
    rkind = (long)gbtn->content;
    go_to_my_next_room_of_type_and_select(rkind);
    game.chosen_room_kind = rkind;
    game.chosen_room_look = room_info[rkind].field_0;
    game.chosen_room_tooltip = gbtn->tooltip_id;
}

void gui_over_room_button(struct GuiButton *gbtn)
{
    //_DK_gui_over_room_button(gbtn);
    gui_room_type_highlighted = (long)gbtn->content;
}

void gui_area_room_button(struct GuiButton *gbtn)
{
    //_DK_gui_area_room_button(gbtn);
    unsigned short flg_mem;
    flg_mem = lbDisplay.DrawFlags;
    RoomKind rkind;
    rkind = (long)gbtn->content;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 24);
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int spr_idx;
    if (dungeon->room_resrchable[rkind] || dungeon->room_buildable[rkind])
    {
        if ((gbtn->flags & 0x08) != 0)
        {
            if (dungeon->room_kind[rkind] > 0)
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 27);
            spr_idx = (dungeon->total_money_owned < game.room_stats[rkind].cost) + gbtn->field_29;
            if ((gbtn->gbactn_1 == 0) && (gbtn->gbactn_2 == 0)) {
                draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, spr_idx);
            } else {
                draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, spr_idx, 44);
            }
        } else
        {
            // Draw a question mark over the button, to indicate it can be researched
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 25);
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void pick_up_next_creature(struct GuiButton *gbtn)
{
    int kind;
    int i;
    unsigned short pick_flags;

    //_DK_pick_up_next_creature(gbtn);

    i = gbtn->field_1B;
    if (i > 0) {
        kind = breed_activities[(i + top_of_breed_list) % CREATURE_TYPES_COUNT];
    }
    else {
        kind = get_players_special_digger_model(my_player_number);
    }

    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        pick_flags |= TPF_OrderedPick;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(kind, -1, my_player_number, pick_flags);
}

void gui_go_to_next_creature(struct GuiButton *gbtn)
{
    long i;
    SYNCDBG(8,"Starting");
    //_DK_gui_go_to_next_creature(gbtn);
    i = gbtn->field_1B;
    ThingModel crmodel;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    go_to_next_creature_of_model_and_gui_job(crmodel, -1);
}

void gui_area_anger_button(struct GuiButton *gbtn)
{
    long i,job_idx,crmodel;
    SYNCDBG(10,"Starting");
    i = gbtn->field_1B;
    // Get index from pointer
    job_idx = ((long *)gbtn->content - &activity_list[0]);
    if ( (i > 0) && (top_of_breed_list+i < CREATURE_TYPES_COUNT) )
        crmodel = breed_activities[top_of_breed_list+i];
    else
        crmodel = get_players_special_digger_model(my_player_number);
    // Now draw the button
    struct Dungeon *dungeon;
    int spridx;
    long cr_total;
    cr_total = 0;
    if ((crmodel > 0) && (crmodel < CREATURE_TYPES_COUNT) && (gbtn->flags & LbBtnF_Unknown08))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        spridx = gbtn->field_29;
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
          draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y-2, 16, spridx, 12);
        } else
        {
          draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y-2, 16, spridx);
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
          draw_button_string(gbtn, gui_textbuf);
          LbTextUseByteCoding(true);
        }
    }
    SYNCDBG(12,"Finished");
}

#define BAR_FULL_WIDTH 32
void gui_area_progress_bar_short(struct GuiButton *gbtn, int progress, int total)
{
    int bar_fill;
    bar_fill = BAR_FULL_WIDTH;
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
    LbDrawBox((gbtn->scr_pos_x + 22 - bar_fill + BAR_FULL_WIDTH) / pixel_size, (gbtn->scr_pos_y + 8) / pixel_size, bar_fill / pixel_size, 8 / pixel_size, colours[0][0][0]);
}
#undef BAR_FULL_WIDTH

#define BAR_FULL_WIDTH 48
void gui_area_progress_bar_med2(struct GuiButton *gbtn, int progress, int total)
{
    int bar_fill;
    bar_fill = BAR_FULL_WIDTH;
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
    LbDrawBox((gbtn->scr_pos_x +  4 - bar_fill + BAR_FULL_WIDTH) / pixel_size, (gbtn->scr_pos_y + 4) / pixel_size, bar_fill / pixel_size, 16 / pixel_size, colours[0][0][0]);
}
#undef BAR_FULL_WIDTH

long anger_get_creature_highest_anger_type_and_byte_percentage(struct Thing *creatng, long *out_angr_typ, long *out_angr_prct)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    int angr_typ;
    long angr_lvl, angr_lmt;
    angr_lmt = crstat->annoy_level;
    angr_typ = 0;
    angr_lvl = 0;
    int i;
    for (i=1; i < 5; i++)
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
    //_DK_gui_area_smiley_anger_button(gbtn);
    struct PlayerInfo *player;
    player = get_my_player();
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, gbtn->field_29);
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (thing_is_creature(ctrltng))
    {
        long angr_typ, angr_prct;
        anger_get_creature_highest_anger_type_and_byte_percentage(ctrltng, &angr_typ, &angr_prct);
        int angr_pos;
        angr_pos = 5 * angr_prct / 256;
        if (angr_pos < 0) {
            angr_pos = 0;
        } else
        if (angr_pos > 4) {
            angr_pos = 4;
        }
        int spr_idx;
        int shift_x;
        spr_idx = angr_pos + 468;
        shift_x = (48 * angr_prct - 16) / 256;
        if (shift_x < 0) {
            shift_x = 0;
        } else
        if ( shift_x > 36 ) {
            shift_x = 36;
        }
        draw_gui_panel_sprite_left(gbtn->scr_pos_x + shift_x - 12, gbtn->scr_pos_y - 22, 16, spr_idx);
    }
}

void gui_area_experience_button(struct GuiButton *gbtn)
{
    //_DK_gui_area_experience_button(gbtn); return;
    struct PlayerInfo *player;
    player = get_my_player();
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (thing_is_creature(ctrltng))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, gbtn->field_29);
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(ctrltng);
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(ctrltng);
        long points_progress, points_required;
        points_progress = cctrl->exp_points;
        points_required = (crstat->to_level[cctrl->explevel] << 8);
        gui_area_progress_bar_med2(gbtn, points_progress, points_required);
        char * text;
        text = buf_sprintf("%d", (int)(cctrl->explevel + 1));
        draw_button_string(gbtn, text);
    } else
    if (thing_is_dead_creature(ctrltng))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, gbtn->field_29);
        //TODO maybe show some info about dead creature too?
    }
}

void gui_area_instance_button(struct GuiButton *gbtn)
{
    //_DK_gui_area_instance_button(gbtn); return;
    struct PlayerInfo *player;
    player = get_my_player();
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (!thing_is_creature(ctrltng))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 463);
        LbDrawBox((gbtn->scr_pos_x + 22) / pixel_size, (gbtn->scr_pos_y + 8) / pixel_size, 32 / pixel_size, 8 / pixel_size, colours[0][0][0]);
        return;
    }
    int curbtn_avail_pos;
    curbtn_avail_pos = (long)gbtn->content;
    if (!first_person_instance_top_half_selected)
        curbtn_avail_pos += 4;
    int curbtn_inst_id;
    curbtn_inst_id = creature_instance_get_available_id_for_pos(ctrltng, curbtn_avail_pos);
    if (!creature_instance_is_available(ctrltng, curbtn_inst_id))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 463);
        LbDrawBox((gbtn->scr_pos_x + 22) / pixel_size, (gbtn->scr_pos_y + 8) / pixel_size, 32 / pixel_size, 8 / pixel_size, colours[0][0][0]);
        return;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(ctrltng);
    int spr_idx;
    if (cctrl->field_1E8 == curbtn_inst_id) {
      spr_idx = 462;
    } else {
      spr_idx = 463;
    }
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, spr_idx);
    struct InstanceInfo *inst_inf;
    inst_inf = creature_instance_info_get(curbtn_inst_id);
    if (!creature_instance_has_reset(ctrltng, curbtn_inst_id))
    {
        long turns_progress, turns_required;
        if ((ctrltng->alloc_flags & TAlF_IsControlled) != 0) {
            turns_required = inst_inf->fp_reset_time;
        } else {
            turns_required = inst_inf->reset_time;
        }
        turns_progress = (long)game.play_gameturn - (long)cctrl->instance_use_turn[curbtn_inst_id];
        gui_area_progress_bar_short(gbtn, turns_progress, turns_required);
    } else
    {
        gui_area_progress_bar_short(gbtn, 32, 32);
    }
    const char * text;
    text = buf_sprintf("%d", (curbtn_avail_pos + 1) % 10);
    LbTextDraw((gbtn->scr_pos_x + 52) / pixel_size, (gbtn->scr_pos_y + 9) / pixel_size, text);
    spr_idx = gbtn->field_29;
    long spkind;
    spkind = inst_inf->func_params[0];
    if (!creature_instance_has_reset(ctrltng, curbtn_inst_id) || ((spkind != 0) && thing_affected_by_spell(ctrltng, spkind)))
      spr_idx++;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 8, 16, spr_idx);
}

void maintain_instance(struct GuiButton *gbtn)
{
    //_DK_maintain_instance(gbtn); return;
    struct PlayerInfo *player;
    player = get_my_player();
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    TRACE_THING(ctrltng);
    if (!thing_is_creature(ctrltng))
    {
        gbtn->field_1B |= 0x8000;
        gbtn->flags &= ~0x0008;
        return;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(ctrltng);
    // Switch to correct menu page based on selected instance position
    int chosen_avail_pos;
    chosen_avail_pos = creature_instance_get_available_pos_for_id(ctrltng, cctrl->field_1E8);
    int curbtn_avail_pos;
    if ((chosen_avail_pos < 6) && (first_person_instance_top_half_selected || chosen_avail_pos < 4))
    {
        first_person_instance_top_half_selected = 1;
        curbtn_avail_pos = (long)gbtn->content;
    } else
    {
        first_person_instance_top_half_selected = 0;
        curbtn_avail_pos = ((long)gbtn->content) + 4;
    }
    // Now handle instance for this button
    int curbtn_inst_id;
    curbtn_inst_id = creature_instance_get_available_id_for_pos(ctrltng, curbtn_avail_pos);
    int i;
    i = instance_button_init[curbtn_inst_id].numfield_0;
    if ( i )
    {
        gbtn->field_29 = i;
        gbtn->tooltip_id = instance_button_init[curbtn_inst_id].numfield_4;
    }
    if (creature_instance_is_available(ctrltng, curbtn_inst_id))
    {
        gbtn->field_1B = 0;
        gbtn->flags |= 0x0008;
        return;
    }
    gbtn->field_1B |= 0x8000;
    gbtn->flags &= ~0x0008;
}

void gui_activity_background(struct GuiMenu *gmnu)
{
    SYNCDBG(9,"Starting");
    //_DK_gui_activity_background(gmnu);return;
    unsigned short flg_mem;
    flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    if (no_of_breeds_owned <= 6) {
        top_of_breed_list = 0;
    }
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int visible_count;
    visible_count = no_of_breeds_owned;
    if (no_of_breeds_owned <= 1)
      visible_count = 1;
    if (visible_count >= 6)
        visible_count = 6;
    int i;
    for (i=0; i < visible_count; i++)
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
        int n;
        for (n=0; n < 15; n++)
        {
            int job_idx;
            job_idx = state_type_to_gui_state[n];
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
    LbDrawBox((gmnu->pos_x + 2) / pixel_size, (gmnu->pos_y + 218) / pixel_size, 134 / pixel_size, 24 / pixel_size, colours[0][0][0]);
    lbDisplay.DrawFlags = flg_mem;
}

void maintain_activity_up(struct GuiButton *gbtn)
{
    //_DK_maintain_activity_up(gbtn);
    if (no_of_breeds_owned <= 6)
    {
        gbtn->flags &= ~0x04;
        gbtn->flags &= ~0x08;
    } else
    {
        gbtn->flags |= 0x04;
        gbtn->flags ^= (gbtn->flags ^ 8 * (top_of_breed_list > 0)) & 8;
    }
}

void maintain_activity_down(struct GuiButton *gbtn)
{
    //_DK_maintain_activity_down(gbtn);
    if (no_of_breeds_owned <= 6)
    {
        gbtn->flags &= ~0x04;
        gbtn->flags &= ~0x08;
    }
    else
    {
        gbtn->flags |= 0x04;
        gbtn->flags ^= (gbtn->flags ^ 8 * (no_of_breeds_owned - 6 > top_of_breed_list)) & 8;
    }
}

void maintain_activity_pic(struct GuiButton *gbtn)
{
    //_DK_maintain_activity_pic(gbtn); return;
    ThingModel crmodel;
    int i;
    i = gbtn->field_1B;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int amount;
    /*if (crmodel == get_players_special_digger_model(my_player_number))
      amount = dungeon->num_active_diggers;
    else*/
    amount = dungeon->owned_creatures_of_model[crmodel];
    gbtn->flags ^= (gbtn->flags ^ 8 * (amount > 0)) & 8;
    gbtn->flags ^= (gbtn->flags ^ 4 * (no_of_breeds_owned > i)) & 4;
    gbtn->field_29 = get_creature_model_graphics(crmodel, CGI_GUIPanelSymbol);
}

void maintain_activity_row(struct GuiButton *gbtn)
{
    //_DK_maintain_activity_row(gbtn); return;
    ThingModel crmodel;
    int i;
    i = gbtn->field_1B;
    if (i > 0) {
        crmodel = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    } else {
        crmodel = get_players_special_digger_model(my_player_number);
    }
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int amount;
    /*if (crmodel == get_players_special_digger_model(my_player_number))
      amount = dungeon->num_active_diggers;
    else*/
    amount = dungeon->owned_creatures_of_model[crmodel];
    gbtn->flags ^= (gbtn->flags ^ 8 * (amount > 0)) & 8;
    gbtn->flags ^= (gbtn->flags ^ 4 * (no_of_breeds_owned > i)) & 4;
}

void gui_scroll_activity_up(struct GuiButton *gbtn)
{
    //_DK_gui_scroll_activity_up(gbtn);
    if (top_of_breed_list > 0)
      top_of_breed_list--;
}

void gui_scroll_activity_down(struct GuiButton *gbtn)
{
    //_DK_gui_scroll_activity_down(gbtn);
    if (top_of_breed_list + 6 < no_of_breeds_owned)
        top_of_breed_list++;
}

void gui_area_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx;
    //_DK_gui_area_ally(gbtn);
    plyr_idx = (int)gbtn->content;
    int spr_idx;
    spr_idx = 498;
    if ((gbtn->flags & LbBtnF_Unknown08) == 0) {
        return;
    }
    if (game.play_gameturn & 1)
    {
        struct PlayerInfo *player;
        player = get_my_player();
        if (player_allied_with(player, plyr_idx)) {
            spr_idx = 488 + (plyr_idx & 0x0f);
        }
    } else
    {
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        if (player_allied_with(player, my_player_number)) {
            spr_idx = 488 + (plyr_idx & 0x0f);
        }
    }
    if ( gbtn->gbactn_1 || gbtn->gbactn_2 )
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, spr_idx, 44);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, spr_idx);
    }
}

void gui_area_stat_button(struct GuiButton *gbtn)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    struct PlayerInfo *player;
    struct Thing *thing;
    char *text;
    long i;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 459);
    player = get_my_player();
    thing = thing_get(player->controlled_thing_idx);
    if (!thing_exists(thing))
      return;
    if (thing->class_id == TCls_Creature)
    {
      crstat = creature_stats_get_from_thing(thing);
      cctrl = creature_control_get_from_thing(thing);
      switch ((long)gbtn->content)
      {
      case 0: // kills
          i = cctrl->kills_num;
          text = buf_sprintf("%ld", i);
          break;
      case 1: // strength
          i = compute_creature_max_strength(crstat->strength,cctrl->explevel);
          text = buf_sprintf("%ld", i);
          break;
      case 2: // gold held
          i = thing->creature.gold_carried;
          text = buf_sprintf("%ld", i);
          break;
      case 3: // payday wage
          i = calculate_correct_creature_pay(thing);
          text = buf_sprintf("%ld", i);
          break;
      case 4: // armour
          i = compute_creature_max_armour(crstat->armour,cctrl->explevel);
          text = buf_sprintf("%ld", i);
          break;
      case 5: // defence
          i = compute_creature_max_defense(crstat->defense,cctrl->explevel);
          text = buf_sprintf("%ld", i);
          break;
      case 6: // time in dungeon
          i = (game.play_gameturn-thing->creation_turn) / 2000 + cctrl->joining_age;
          if (i >= 99)
            i = 99;
          text = buf_sprintf("%ld", i);
          break;
      case 7: // dexterity
          i = compute_creature_max_dexterity(crstat->dexterity,cctrl->explevel);
          text = buf_sprintf("%ld", i);
          break;
      case 8: // luck
          i = compute_creature_max_luck(crstat->luck,cctrl->explevel);
          text = buf_sprintf("%ld", i);
          break;
      case 9: // blood type
          i = cctrl->blood_type;
          text = buf_sprintf("%s", blood_types[i%BLOOD_TYPES_COUNT]);
          break;
      default:
          return;
      }
      draw_gui_panel_sprite_left(gbtn->scr_pos_x-6, gbtn->scr_pos_y-12, 16, gbtn->field_29);
      draw_button_string(gbtn, text);
    }
}

void maintain_event_button(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    unsigned short evnt_idx;
    unsigned long evbtn_idx;
    evbtn_idx = (unsigned long)gbtn->content;
    if (evbtn_idx <= EVENT_BUTTONS_COUNT) {
        evnt_idx = dungeon->event_button_index[evbtn_idx];
    } else {
        evnt_idx = 0;
    }

    if ((dungeon->visible_event_idx != 0) && (evnt_idx == dungeon->visible_event_idx))
    {
        turn_on_event_info_panel_if_necessary(dungeon->visible_event_idx);
    }

    if (evnt_idx == 0)
    {
      gbtn->field_1B |= 0x4000;
      gbtn->field_29 = 0;
      gbtn->flags &= ~LbBtnF_Unknown08;
      gbtn->gbactn_1 = 0;
      gbtn->gbactn_2 = 0;
      gbtn->tooltip_id = 201;
      return;
    }
    struct Event *event;
    event = &game.event[evnt_idx];
    if ((event->kind == EvKind_Objective) && (new_objective))
    {
        activate_event_box(evnt_idx);
    }
    gbtn->field_29 = event_button_info[event->kind].field_0;
    if (((event->kind == EvKind_FriendlyFight) || (event->kind == EvKind_EnemyFight))
        && ((event->mappos_x != 0) || (event->mappos_y != 0)) && ((game.play_gameturn & 0x01) != 0))
    {
        gbtn->field_29 += 2;
    } else
    if ((event->kind == EvKind_Information) && (event->target < 0)
       && ((game.play_gameturn & 0x01) != 0))
    {
      gbtn->field_29 += 2;
    }
    gbtn->tooltip_id = event_button_info[event->kind].tooltip_stridx;
    gbtn->flags |= LbBtnF_Unknown08;
    gbtn->field_1B = 0;
}

void gui_toggle_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx;
    //_DK_gui_toggle_ally(gbtn);
    plyr_idx = (int)gbtn->content;
    if ((gbtn->flags & LbBtnF_Unknown08) != 0) {
        struct Packet *pckt;
        pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_PlyrToggleAlly, plyr_idx, 0, 0, 0);
    }
}

void maintain_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx;
    struct PlayerInfo *player;
    //_DK_maintain_ally(gbtn);
    plyr_idx = (int)gbtn->content;
    player = get_player(plyr_idx);
    if (!is_my_player_number(plyr_idx) && ((player->field_0 & 0x01) != 0))
    {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else
    {
        gbtn->field_1B |= 0x8000;
        gbtn->flags &= ~LbBtnF_Unknown08;
    }
}

void maintain_prison_bar(struct GuiButton *gbtn)
{
    //_DK_maintain_prison_bar(gbtn);
    if (player_has_room(my_player_number, RoK_PRISON))
    {
        gbtn->field_29 = 350;
        gbtn->flags |= LbBtnF_Unknown08;
    } else
    {
        gbtn->field_29 = 354;
        gbtn->flags &= ~LbBtnF_Unknown08;
        /*if (gbtn->gbactn_1) - this does nothing, but was in original function
        {
            menu_id_to_number(7);
        }*/
    }
}

void maintain_room_and_creature_button(struct GuiButton *gbtn)
{
    //_DK_maintain_room_and_creature_button(gbtn);
    PlayerNumber plyr_idx;
    struct PlayerInfo *player;
    //_DK_maintain_ally(gbtn);
    plyr_idx = (int)gbtn->content;
    player = get_player(plyr_idx);
    if (player_exists(player))
    {
        gbtn->field_1B = 0;
        gbtn->flags |= LbBtnF_Unknown08;
    } else
    {
        gbtn->field_1B |= 0x4000;
        gbtn->flags &= ~LbBtnF_Unknown08;
        gbtn->tooltip_id = 201;
    }
}

void pick_up_next_wanderer(struct GuiButton *gbtn)
{
    unsigned short pick_flags;
    //_DK_pick_up_next_wanderer(gbtn);
    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        pick_flags |= TPF_OrderedPick;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(-1, 0, my_player_number, pick_flags);
}

void gui_go_to_next_wanderer(struct GuiButton *gbtn)
{
    //_DK_gui_go_to_next_wanderer(gbtn);
    go_to_next_creature_of_model_and_gui_job(-1, 0);
}

void pick_up_next_worker(struct GuiButton *gbtn)
{
    unsigned short pick_flags;
    //_DK_pick_up_next_worker(gbtn);
    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        pick_flags |= TPF_OrderedPick;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(-1, 1, my_player_number, pick_flags);
}

void gui_go_to_next_worker(struct GuiButton *gbtn)
{
    //_DK_gui_go_to_next_worker(gbtn);
    go_to_next_creature_of_model_and_gui_job(-1, 1);
}

void pick_up_next_fighter(struct GuiButton *gbtn)
{
    unsigned short pick_flags;
    //_DK_pick_up_next_fighter(gbtn);
    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        pick_flags |= TPF_OrderedPick;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(-1, 2, my_player_number, pick_flags);
}

void gui_go_to_next_fighter(struct GuiButton *gbtn)
{
    //_DK_gui_go_to_next_fighter(gbtn);
    go_to_next_creature_of_model_and_gui_job(-1, 2);
}

void gui_area_payday_button(struct GuiButton *gbtn)
{
    //_DK_gui_area_payday_button(gbtn);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, gbtn->field_29);
    int turns_passed;
    turns_passed = game.field_15033A;
    if (game.field_15033A > game.pay_day_gap)
      turns_passed = game.pay_day_gap;
    if (game.pay_day_gap > 0)
    {
        int bar_fill;
        bar_fill = 96 - 2 * (48 * turns_passed / game.pay_day_gap);
        if (bar_fill < 0) {
            bar_fill = 0;
        } else
        if (bar_fill > 96) {
            bar_fill = 96;
        }
        LbDrawBox((gbtn->scr_pos_x + 30 - bar_fill + 94) / pixel_size, (gbtn->scr_pos_y + 12) / pixel_size,
            bar_fill / pixel_size, 8 / pixel_size, colours[0][0][0]);
    }
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    char *text;
    text = buf_sprintf("%d", (int)dungeon->creatures_total_pay);
    draw_centred_string64k(text, gbtn->scr_pos_x + (gbtn->width >> 1), gbtn->scr_pos_y + 8, gbtn->width);
}

void gui_area_research_bar(struct GuiButton *gbtn)
{
    //_DK_gui_area_research_bar(gbtn);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    int resrch_required, resrch_progress;
    struct ResearchVal *rsrchval;
    rsrchval = get_players_current_research_val(my_player_number);
    if (rsrchval != NULL)
    {
        resrch_required = rsrchval->req_amount;
        resrch_progress = (dungeon->research_progress >> 8);
    } else
    {
        resrch_required = 0;
        resrch_progress = 0;
    }
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 462);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x - 8, gbtn->scr_pos_y - 10, 16, gbtn->field_29);
    gui_area_progress_bar_short(gbtn, resrch_progress, resrch_required);
}

void gui_area_workshop_bar(struct GuiButton *gbtn)
{
    //_DK_gui_area_workshop_bar(gbtn);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(my_player_number);
    int manufct_required, manufct_progress;
    if (dungeon->manufacture_class != TCls_Empty)
    {
        manufct_required = manufacture_points_required(dungeon->manufacture_class, dungeon->manufacture_kind);
        manufct_progress = (dungeon->manufacture_progress >> 8);
    } else
    {
        manufct_required = 0;
        manufct_progress = 0;
    }
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 462);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x - 8, gbtn->scr_pos_y - 10, 16, gbtn->field_29);
    gui_area_progress_bar_short(gbtn, manufct_progress, manufct_required);
}

void gui_area_player_creature_info(struct GuiButton *gbtn)
{
    //_DK_gui_area_player_creature_info(gbtn);
    PlayerNumber plyr_idx;
    struct PlayerInfo *player;
    plyr_idx = (int)gbtn->content;
    player = get_player(plyr_idx);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 459);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        if (((dungeon->num_active_creatrs < dungeon->max_creatures_attracted) && (!game.pool.is_empty))
            || ((game.play_gameturn & 1) != 0))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, gbtn->field_29);
        } else
        {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, gbtn->field_29, 44);
        }
        char *text;
        long i;
        i = dungeon->num_active_creatrs;
        text = buf_sprintf("%ld", i);
        draw_button_string(gbtn, text);
    }
}

void gui_area_player_room_info(struct GuiButton *gbtn)
{
    //_DK_gui_area_player_room_info(gbtn);
    PlayerNumber plyr_idx;
    struct PlayerInfo *player;
    //_DK_maintain_ally(gbtn);
    plyr_idx = (int)gbtn->content;
    player = get_player(plyr_idx);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, 459);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 16, gbtn->field_29);
        char *text;
        long i;
        i = dungeon->total_rooms;
        text = buf_sprintf("%ld", i);
        draw_button_string(gbtn, text);
    }
}

void spell_lost_first_person(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    SYNCDBG(19,"Starting");
    player=get_my_player();
    set_players_packet_action(player, PckA_GoSpectator, 0, 0, 0, 0);
}

void gui_set_tend_to(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  set_players_packet_action(player, PckA_ToggleTendency, gbtn->field_1B, 0, 0, 0);
}

void gui_set_query(struct GuiButton *gbtn)
{
  //_DK_gui_set_query(gbtn);
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_SetPlyrState, PSt_Unknown12, 0, 0, 0);
}

void draw_gold_total(PlayerNumber plyr_idx, long scr_x, long scr_y, long units_per_px, long long value)
{
    struct TbSprite *spr;
    unsigned int flg_mem;
    int ndigits,val_width;
    long pos_x;
    long long i;
    flg_mem = lbDisplay.DrawFlags;
    ndigits = 0;
    val_width = 0;
    for (i = value; i > 0; i /= 10) {
        ndigits++;
    }
    spr = &button_sprite[71];
    val_width = (spr->SWidth * units_per_px / 16) * ndigits;
    if (ndigits > 0)
    {
        pos_x = scr_x + val_width / 2;
        for (i = value; i > 0; i /= 10)
        {
            // Make space for the character first, as we're drawing right char towards left
            pos_x -= spr->SWidth * units_per_px / 16;
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
    struct Dungeon *dungeon;
    struct PlayerInfo *player;
    long mmzoom;
    player = get_my_player();
    dungeon = get_players_dungeon(player);
    // Get the menu scale
    struct GuiMenu *gmnu;
    int fs_units_per_px;
    int mm_units_per_px;
    int bs_units_per_px;
    {
        int mnu_num;
        mnu_num = menu_id_to_number(GMnu_MAIN);
        gmnu = get_active_menu(mnu_num);
        fs_units_per_px = (gmnu->height * 16 + 8) / LbTiledSpriteHeight(&status_panel, gui_panel_sprites);
        mm_units_per_px = (gmnu->width * 16 + 8) / 136;
        bs_units_per_px = gmnu->width * 4 / 35;
    }
    lbDisplay.DrawColour = colours[15][15][15];
    lbDisplay.DrawFlags = 0;
    LbTiledSpriteDraw(0, 0, fs_units_per_px, &status_panel, gui_panel_sprites);
    // Draws gold amount; note that button_sprite[] is used instead of full font
    draw_gold_total(player->id_number, gmnu->pos_x + gmnu->width/2, gmnu->pos_y + gmnu->height*67/200, bs_units_per_px, dungeon->total_money_owned);
    if (16/mm_units_per_px < 3)
        mmzoom = (player->minimap_zoom) / (3-16/mm_units_per_px);
    else
        mmzoom = player->minimap_zoom;
    pannel_map_draw_slabs(player->minimap_pos_x, player->minimap_pos_y, mm_units_per_px, mmzoom);
    pannel_map_draw_overlay_things(mm_units_per_px, mmzoom);
}

void gui_set_button_flashing(long btn_idx, long gameturns)
{
    game.flash_button_index = btn_idx;
    game.flash_button_gameturns = gameturns;
}
/******************************************************************************/
