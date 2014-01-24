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
#include "config_creature.h"
#include "config_magic.h"
#include "config_trapdoor.h"
#include "room_workshop.h"
#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "packets.h"
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
DLLIMPORT void _DK_draw_gold_total(unsigned char a1, long a2, long a3, long a4);
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
  _DK_gui_zoom_in(gbtn);
}

void gui_zoom_out(struct GuiButton *gbtn)
{
  _DK_gui_zoom_out(gbtn);
}

void gui_go_to_map(struct GuiButton *gbtn)
{
    zoom_to_map();
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
    set_flag_byte(&gbtn->flags, 0x08, (player->victory_state != VicS_LostLevel));
}

void gui_area_autopilot_button(struct GuiButton *gbtn)
{
  _DK_gui_area_autopilot_button(gbtn);
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
    if ((gbtn->flags & 0x08) != 0)
    {
        dungeon = get_players_num_dungeon(my_player_number);
        i = (unsigned long)gbtn->content;
        if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
        } else
        if ((i <= EVENT_BUTTONS_COUNT) && (dungeon->event_button_index[i] == dungeon->visible_event_idx))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
        } else
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29+1);
        }
    }
}

void gui_remove_area_for_rooms(struct GuiButton *gbtn)
{
  _DK_gui_remove_area_for_rooms(gbtn);
}

void gui_area_big_room_button(struct GuiButton *gbtn)
{
  _DK_gui_area_big_room_button(gbtn);
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

void gui_go_to_next_spell(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_spell(gbtn);
}

void gui_area_spell_button(struct GuiButton *gbtn)
{
  _DK_gui_area_spell_button(gbtn);
}

void gui_choose_special_spell(struct GuiButton *gbtn)
{
    //NOTE by Petter: factored out original gui_choose_special_spell code to choose_special_spell
    //TODO: equivalent to gui_choose_spell now... try merge
    choose_spell(((int) gbtn->content) % POWER_TYPES_COUNT, gbtn->tooltip_id);
}

void gui_area_big_spell_button(struct GuiButton *gbtn)
{
  _DK_gui_area_big_spell_button(gbtn);
}

/**
 * Choose a trap or a door.
 * @param manufctr_idx An index into manufacture data array, beware as this is different from models.
 * @param tooltip_id The tooltip string to display.
 */
void choose_workshop_item(int manufctr_idx, int tooltip_id)
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

void go_to_next_trap_of_type(unsigned long kind, PlayerNumber plyr_idx)
{
    static unsigned short trap[8];
    struct Thing *thing;
    int i;
    unsigned long k;
    if (kind >= 8) {
        ERRORLOG("Bad trap kind");
        return;
    }
    k = 0;
    i = trap[kind];
    trap[kind] = 0;
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
        if ((thing->owner == plyr_idx) && (thing->model == kind)) {
            trap[kind] = thing->index;
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
    i = trap[kind];
    if (i > 0) {
        struct Packet *pckt;
        pckt = get_packet(plyr_idx);
        set_packet_action(pckt, PckA_ZoomToTrap, i, 0, 0, 0);
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
    _DK_gui_over_trap_button(gbtn);
}

void gui_area_trap_button(struct GuiButton *gbtn)
{
    unsigned short flg_mem;
    int manufctr_idx;
    //_DK_gui_area_trap_button(gbtn);
    flg_mem = lbDisplay.DrawFlags;
    manufctr_idx = (long)gbtn->content;
    LbSpriteDraw(gbtn->scr_pos_x/(int)pixel_size, gbtn->scr_pos_y/(int)pixel_size, &gui_panel_sprites[24]);
    struct ManufactureData *manufctr;
    manufctr = get_manufacture_data(manufctr_idx);
    // Check if we should draw anything
    if (manufctr->tngclass == TCls_Trap)
    {
        if (!is_trap_buildable(my_player_number, manufctr->tngmodel) && !is_trap_placeable(my_player_number, manufctr->tngmodel)) {
            lbDisplay.DrawFlags = flg_mem;
            return;
        }
    } else
    if (manufctr->tngclass == TCls_Door)
    {
        if (!is_door_buildable(my_player_number, manufctr->tngmodel) && !is_door_placeable(my_player_number, manufctr->tngmodel)) {
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
        LbSpriteDraw(gbtn->scr_pos_x/(int)pixel_size, gbtn->scr_pos_y/(int)pixel_size, &gui_panel_sprites[25]);
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
            LbSpriteDraw(gbtn->scr_pos_x/(int)pixel_size, gbtn->scr_pos_y/(int)pixel_size, &gui_panel_sprites[27]);
        }
        amount = dungeon->trap_amount_placeable[manufctr->tngmodel];
        break;
    case TCls_Door:
        // If there are doors of that type placed on map
        if (find_door_of_type(manufctr->tngmodel, my_player_number)) {
            LbSpriteDraw(gbtn->scr_pos_x/(int)pixel_size, gbtn->scr_pos_y/(int)pixel_size, &gui_panel_sprites[27]);
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
        LbSpriteDrawRemap(gbtn->scr_pos_x/(int)pixel_size, gbtn->scr_pos_y/(int)pixel_size,
            &gui_panel_sprites[i], &pixmap.fade_tables[22*256]);
    } else
    {
        LbSpriteDraw(gbtn->scr_pos_x/(int)pixel_size, gbtn->scr_pos_y/(int)pixel_size,
            &gui_panel_sprites[i]);
    }
    lbDisplay.DrawFlags = flg_mem;
}

void gui_go_to_next_door(struct GuiButton *gbtn)
{
    _DK_gui_go_to_next_door(gbtn);
}

void gui_over_door_button(struct GuiButton *gbtn)
{
    _DK_gui_over_door_button(gbtn);
}

void gui_remove_area_for_traps(struct GuiButton *gbtn)
{
    _DK_gui_remove_area_for_traps(gbtn);
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

    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 26);
    if (manufctr_idx == 0) {
        lbDisplay.DrawFlags = flg_mem;
        return;
    }
    lbDisplay.DrawFlags &= ~0x0040;
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
    char *text;
    text = buf_sprintf("@%ld", (long)amount);
    if (amount <= 0) {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, gbtn->field_29 + 1);
    } else
    if ((((manufctr->tngclass == TCls_Trap) && (player->chosen_trap_kind == manufctr->tngmodel))
      || ((manufctr->tngclass == TCls_Door) && (player->chosen_door_kind == manufctr->tngmodel)))
      && ((game.play_gameturn & 1) == 0) )
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, gbtn->field_29, 11264);
    } else {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x - 4, gbtn->scr_pos_y - 32, gbtn->field_29);
    }
    lbDisplay.DrawFlags &= ~0x0040;
    LbTextDraw((gbtn->scr_pos_x + 44) / (int)pixel_size, (gbtn->scr_pos_y + 8 - 6) / (int)pixel_size, text);
    lbDisplay.DrawFlags = flg_mem;
}

void maintain_big_spell(struct GuiButton *gbtn)
{
    _DK_maintain_big_spell(gbtn);
}

void maintain_room(struct GuiButton *gbtn)
{
    _DK_maintain_room(gbtn);
}

void maintain_big_room(struct GuiButton *gbtn)
{
    _DK_maintain_big_room(gbtn);
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
      if (game.field_150356 != 0)
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
      if (dungeon->field_88C[0])
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

void gui_creature_query_background1(struct GuiMenu *gmnu)
{
  SYNCDBG(19,"Starting");
  _DK_gui_creature_query_background1(gmnu);
}

void gui_creature_query_background2(struct GuiMenu *gmnu)
{
  SYNCDBG(19,"Starting");
  _DK_gui_creature_query_background2(gmnu);
}

void pick_up_creature_doing_activity(struct GuiButton *gbtn)
{
    long i,job_idx,kind;
    unsigned char pick_flags;
    SYNCDBG(8,"Starting");
    //_DK_pick_up_creature_doing_activity(gbtn); return;
    i = gbtn->field_1B;
    // Get index from pointer
    job_idx = ((long *)gbtn->content - &activity_list[0]);
    if (i > 0)
        kind = breed_activities[(top_of_breed_list+i)%CREATURE_TYPES_COUNT];
    else
        kind = get_players_special_digger_model(my_player_number);
    pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        pick_flags |= TPF_OrderedPick;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_OrderedPick | TPF_ReverseOrder;
    pick_up_creature_of_breed_and_gui_job(kind, (job_idx & 0x03), my_player_number, pick_flags);
}

void gui_go_to_next_creature_activity(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_creature_activity(gbtn);
}

void gui_go_to_next_room(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_room(gbtn);
}

void gui_over_room_button(struct GuiButton *gbtn)
{
  _DK_gui_over_room_button(gbtn);
}

void gui_area_room_button(struct GuiButton *gbtn)
{
  _DK_gui_area_room_button(gbtn);
}

void pick_up_next_creature(struct GuiButton *gbtn)
{
    int kind;
    int i;
    unsigned pick_flags;

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
  _DK_gui_go_to_next_creature(gbtn);
}

void gui_area_anger_button(struct GuiButton *gbtn)
{
    long i,job_idx,kind;
    SYNCDBG(10,"Starting");
    i = gbtn->field_1B;
    // Get index from pointer
    job_idx = ((long *)gbtn->content - &activity_list[0]);
    if ( (i > 0) && (top_of_breed_list+i < CREATURE_TYPES_COUNT) )
        kind = breed_activities[top_of_breed_list+i];
    else
        kind = get_players_special_digger_model(my_player_number);
    // Now draw the button
    struct Dungeon *dungeon;
    int spridx;
    long cr_total;
    cr_total = 0;
    if ((kind > 0) && (kind < CREATURE_TYPES_COUNT) && (gbtn->flags & 0x08))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        spridx = gbtn->field_29;
        if (gbtn->content != NULL)
        {
          cr_total = *(long *)gbtn->content;
          if (cr_total > 0)
          {
            i = dungeon->field_4E4[kind][(job_idx & 0x03)];
            if (i > cr_total)
            {
              WARNDBG(7,"Creature %d stats inconsistency; total=%d, doing activity%d=%d",kind,cr_total,(job_idx & 0x03),i);
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
          draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y-2, spridx, 3072);
        } else
        {
          draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y-2, spridx);
        }
        if (gbtn->content != NULL)
        {
          sprintf(gui_textbuf, "%ld", cr_total);
          // We will use a special coding for our "string" - we want chars to represent
          // sprite index directly, without code pages and multibyte chars interpretation
          if ((cr_total > 0) && (dungeon->job_breeds_count[kind][(job_idx & 0x03)] ))
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

void gui_area_smiley_anger_button(struct GuiButton *gbtn)
{
  _DK_gui_area_smiley_anger_button(gbtn);
}

void gui_area_experience_button(struct GuiButton *gbtn)
{
  _DK_gui_area_experience_button(gbtn);
}

void gui_area_instance_button(struct GuiButton *gbtn)
{
  _DK_gui_area_instance_button(gbtn);
}

void maintain_instance(struct GuiButton *gbtn)
{
  _DK_maintain_instance(gbtn);
}

void gui_activity_background(struct GuiMenu *gmnu)
{
  SYNCDBG(9,"Starting");
  _DK_gui_activity_background(gmnu);
}

void maintain_activity_up(struct GuiButton *gbtn)
{
  _DK_maintain_activity_up(gbtn);
}

void maintain_activity_down(struct GuiButton *gbtn)
{
  _DK_maintain_activity_down(gbtn);
}

void maintain_activity_pic(struct GuiButton *gbtn)
{
  _DK_maintain_activity_pic(gbtn);
}

void maintain_activity_row(struct GuiButton *gbtn)
{
  _DK_maintain_activity_row(gbtn);
}

void gui_scroll_activity_up(struct GuiButton *gbtn)
{
  _DK_gui_scroll_activity_up(gbtn);
}

void gui_scroll_activity_down(struct GuiButton *gbtn)
{
  _DK_gui_scroll_activity_down(gbtn);
}

void gui_area_ally(struct GuiButton *gbtn)
{
    PlayerNumber plyr_idx;
    //_DK_gui_area_ally(gbtn);
    plyr_idx = (int)gbtn->content;
    int spr_idx;
    spr_idx = 498;
    if ((gbtn->flags & 0x08) == 0) {
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
        LbSpriteDrawRemap( gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
            &gui_panel_sprites[spr_idx], &pixmap.fade_tables[44*256]);
    } else
    {
        LbSpriteDraw(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
            &gui_panel_sprites[spr_idx]);
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
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 459);
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
      draw_gui_panel_sprite_left(gbtn->scr_pos_x-6, gbtn->scr_pos_y-12, gbtn->field_29);
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
  _DK_maintain_prison_bar(gbtn);
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
  _DK_pick_up_next_wanderer(gbtn);
}

void gui_go_to_next_wanderer(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_wanderer(gbtn);
}

void pick_up_next_worker(struct GuiButton *gbtn)
{
  _DK_pick_up_next_worker(gbtn);
}

void gui_go_to_next_worker(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_worker(gbtn);
}

void pick_up_next_fighter(struct GuiButton *gbtn)
{
  _DK_pick_up_next_fighter(gbtn);
}

void gui_go_to_next_fighter(struct GuiButton *gbtn)
{
  _DK_gui_go_to_next_fighter(gbtn);
}

void gui_area_payday_button(struct GuiButton *gbtn)
{
  _DK_gui_area_payday_button(gbtn);
}

void gui_area_research_bar(struct GuiButton *gbtn)
{
  _DK_gui_area_research_bar(gbtn);
}

void gui_area_workshop_bar(struct GuiButton *gbtn)
{
  _DK_gui_area_workshop_bar(gbtn);
}

void gui_area_player_creature_info(struct GuiButton *gbtn)
{
    //_DK_gui_area_player_creature_info(gbtn);
    PlayerNumber plyr_idx;
    struct PlayerInfo *player;
    plyr_idx = (int)gbtn->content;
    player = get_player(plyr_idx);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 459);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        if (((dungeon->num_active_creatrs < dungeon->max_creatures_attracted) && (!game.pool.is_empty))
            || ((game.play_gameturn & 1) != 0))
        {
            draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
        } else
        {
            draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29, 11264);
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
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, 459);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    if (player_exists(player) && !dungeon_invalid(dungeon))
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
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
    set_players_packet_action(player, PckA_SetPlyrState, 12, 0, 0, 0);
}

void draw_gold_total(unsigned char a1, long scr_x, long scr_y, long long value)
{
    struct TbSprite *spr;
    unsigned int flg_mem;
    int ndigits,val_width;
    long pos_x;
    long long i;
    //_DK_draw_gold_total(a1, a2, a3, value);
    flg_mem = lbDisplay.DrawFlags;
    ndigits = 0;
    val_width = 0;
    for (i = value; i > 0; i /= 10) {
        ndigits++;
    }
    spr = &button_sprite[71];
    val_width = (pixel_size * (int)spr->SWidth) * (ndigits - 1);
    if (ndigits > 0)
    {
        pos_x = val_width / 2 + scr_x;
        for (i = value; i > 0; i /= 10)
        {
            spr = &button_sprite[i % 10 + 71];
            LbSpriteDraw(pos_x / pixel_size, scr_y / pixel_size, spr);
            pos_x -= pixel_size * spr->SWidth;
        }
    } else
    {
        spr = &button_sprite[71];
        LbSpriteDraw(scr_x / pixel_size, scr_y / pixel_size, spr);
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
    lbDisplay.DrawColour = colours[15][15][15];
    lbDisplay.DrawFlags = 0;
    DrawBigSprite(0, 0, &status_panel, gui_panel_sprites);
    draw_gold_total(player->id_number, 60, 134, dungeon->total_money_owned);
    if (pixel_size < 3)
        mmzoom = (player->minimap_zoom) / (3-pixel_size);
    else
        mmzoom = player->minimap_zoom;
    pannel_map_draw(player->mouse_x, player->mouse_y, mmzoom);
    draw_overlay_things(mmzoom);
}

void gui_set_button_flashing(long btn_idx, long gameturns)
{
    game.flash_button_index = btn_idx;
    game.flash_button_gameturns = gameturns;
}
/******************************************************************************/
