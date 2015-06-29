/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_specials.c
 *     GUI menus for in-game dungeon special boxes.
 * @par Purpose:
 *     Functions to show and maintain menus appearing when specials are used.
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
#include "frontmenu_specials.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "dungeon_data.h"
#include "creature_control.h"
#include "creature_states.h"
#include "config_creature.h"
#include "config_strings.h"
#include "power_specials.h"
#include "gui_draw.h"
#include "gui_frontbtns.h"
#include "frontend.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define resurrect_creature_items_visible  6
struct GuiButtonTemplate resurrect_creature_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, gui_area_text,                     1, GUIStr_SpecResurrectCreature,0,{0},          0, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_resurrect_creature,NULL,  NULL,               0, 999,  62, 999,  62,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_resurrect_creature,NULL,  NULL,               1, 999,  90, 999,  90,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_resurrect_creature,NULL,  NULL,               2, 999, 118, 999, 118,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_resurrect_creature,NULL,  NULL,               3, 999, 146, 999, 146,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_resurrect_creature,NULL,  NULL,               4, 999, 174, 999, 174,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_resurrect_creature,NULL,  NULL,               5, 999, 202, 999, 202,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_select },
  {LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, select_resurrect_creature_up,NULL,NULL,              1, 305,  62, 305,  62, 22, 24, gui_area_new_normal_button,      278, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_scroll },
  {LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, select_resurrect_creature_down,NULL,NULL,            2, 305, 204, 305, 204, 22, 24, gui_area_new_normal_button,      280, GUIStr_Empty,       0,       {0},            0, 0, maintain_resurrect_creature_scroll },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 999, 258, 999, 258,100, 32, gui_area_text,                     1, GUIStr_MnuCancel,   0,       {0},            0, 0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},            0, 0, NULL },
};

#define transfer_creature_items_visible  6
struct GuiButtonTemplate transfer_creature_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, gui_area_text,                     1, GUIStr_SpecTransferCreature,0,{0},           0, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_transfer_creature,NULL,   NULL,               0, 999,  62, 999,  62,250, 26, draw_transfer_creature,            0, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_transfer_creature,NULL,   NULL,               1, 999,  90, 999,  90,250, 26, draw_transfer_creature,            1, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_transfer_creature,NULL,   NULL,               2, 999, 118, 999, 118,250, 26, draw_transfer_creature,            2, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_transfer_creature,NULL,   NULL,               3, 999, 146, 999, 146,250, 26, draw_transfer_creature,            3, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_transfer_creature,NULL,   NULL,               4, 999, 174, 999, 174,250, 26, draw_transfer_creature,            4, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_select },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, select_transfer_creature,NULL,   NULL,               5, 999, 202, 999, 202,250, 26, draw_transfer_creature,            5, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_select },
  {LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, select_transfer_creature_up,NULL,NULL,               1, 305,  62, 305,  62, 22, 24, gui_area_new_normal_button,      278, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_scroll },
  {LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, select_transfer_creature_down,NULL,NULL,             2, 305, 204, 305, 204, 22, 24, gui_area_new_normal_button,      280, GUIStr_Empty,       0,       {0},            0, 0, maintain_transfer_creature_scroll },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 999, 258, 999, 258,100, 32, gui_area_text,                     1, GUIStr_MnuCancel,   0,       {0},            0, 0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},            0, 0, NULL },
};

struct GuiButtonTemplate hold_audience_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, CpgStr_PowerKind1+4,0,       {0},            0, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, gui_area_normal_button,           46, GUIStr_Empty,       0,       {0},            0, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, choose_hold_audience,NULL,       NULL,               0, 116,  24, 118,  58, 46, 32, gui_area_normal_button,           48, GUIStr_Empty,       0,       {0},            0, 0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},            0, 0, NULL },
};

struct GuiButtonTemplate armageddon_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, CpgStr_PowerKind1+16,0,      {0},            0, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, gui_area_normal_button,           46, GUIStr_Empty,       0,       {0},            0, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, choose_armageddon,  NULL,        NULL,               0, 116,  24, 118,  58, 46, 32, gui_area_normal_button,           48, GUIStr_Empty,       0,       {0},            0, 0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},            0, 0, NULL },
};

struct GuiButtonTemplate dungeon_special_buttons[] = {
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},            0, 0, NULL },
};

struct GuiMenu hold_audience_menu =
 { 17, 0, 4, hold_audience_buttons,      POS_GAMECTR,POS_GAMECTR,200, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu dungeon_special_menu =
 { 27, 0, 4, dungeon_special_buttons,            160, POS_SCRBTM,480, 86, gui_round_glass_background,   0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu resurrect_creature_menu =
 { 28, 0, 4, resurrect_creature_buttons, POS_GAMECTR,POS_GAMECTR,350, 300, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu transfer_creature_menu =
 { 29, 0, 4, transfer_creature_buttons,  POS_GAMECTR,POS_GAMECTR,350, 300, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu armageddon_menu =
 { 30, 0, 4, armageddon_buttons,         POS_GAMECTR,POS_GAMECTR,200, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
int selected_resurrect_creature(const struct Dungeon *dungeon, const struct GuiButton *gbtn)
{
    long listitm_idx;
    if (dungeon->dead_creatures_count < DEAD_CREATURES_MAX_COUNT)
    {
        listitm_idx = resurrect_creature_scroll_offset + gbtn->btype_value;
        if (listitm_idx < dungeon->dead_creatures_count) {
            return listitm_idx;
        }
    } else
    {
        listitm_idx = resurrect_creature_scroll_offset + gbtn->btype_value;
        if (listitm_idx < DEAD_CREATURES_MAX_COUNT) {
            return abs(dungeon->dead_creature_idx + listitm_idx) % DEAD_CREATURES_MAX_COUNT;
        }
    }
    return -1;
}

void select_resurrect_creature(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int i;
    i = selected_resurrect_creature(dungeon, gbtn);
    if (i != -1)
    {
        struct CreatureStorage *cstore;
        cstore = &dungeon->dead_creatures[i];
        struct Packet *pckt;
        pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_ResurrectCrtr, dungeon_special_selected, dungeon->owner | (cstore->model << 4) | (cstore->explevel << 12));
        turn_off_menu(GMnu_RESURRECT_CREATURE);
    }
}

void draw_resurrect_creature(struct GuiButton *gbtn)
{
    unsigned short flg_mem;
    flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height, 0);
    LbTextSetFont(winfont);
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int i;
    i = selected_resurrect_creature(dungeon, gbtn);
    int tx_units_per_px;
    tx_units_per_px = ((gbtn->height*22/26) * 16) / LbTextLineHeight();
    if (i != -1)
    {
        struct CreatureStorage *cstore;
        cstore = &dungeon->dead_creatures[i];
        struct CreatureData *crdata;
        crdata = creature_data_get(cstore->model);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        LbTextDrawResizedFmt(0, 0, tx_units_per_px, " %s", get_string(crdata->namestr_idx));
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        LbTextDrawResizedFmt(0, 0, tx_units_per_px, "%s %d ", get_string(GUIStr_MnuLevel), (int)(cstore->explevel + 1));
    }
    lbDisplay.DrawFlags = flg_mem;
}

void select_resurrect_creature_up(struct GuiButton *gbtn)
{
    if (resurrect_creature_scroll_offset > 0) {
        resurrect_creature_scroll_offset--;
    }
}

void select_resurrect_creature_down(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    if (resurrect_creature_scroll_offset < dungeon->dead_creatures_count-resurrect_creature_items_visible+1) {
        resurrect_creature_scroll_offset++;
    }
}

int selected_transfer_creature(const struct Dungeon *dungeon, const struct GuiButton *gbtn)
{
    long listitm_idx;
    listitm_idx = transfer_creature_scroll_offset + gbtn->btype_value;
    if (listitm_idx < dungeon->num_active_creatrs) {
        return listitm_idx;
    }
    return -1;
}

void select_transfer_creature(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    struct Thing *thing;
    thing = INVALID_THING;
    int listitm_idx;
    listitm_idx = selected_transfer_creature(dungeon, gbtn);
    if (listitm_idx != -1)
    {
        thing = get_player_list_nth_creature_of_model(dungeon->creatr_list_start, 0, listitm_idx);
    }
    if (thing_exists(thing))
    {
        struct Packet *pckt;
        pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_TransferCreatr, dungeon_special_selected, thing->index);
        turn_off_menu(GMnu_TRANSFER_CREATURE);
    }
}

void draw_transfer_creature(struct GuiButton *gbtn)
{
    unsigned long flgmem;
    if (gbtn == NULL)
      return;
    SYNCDBG(7,"Starting");
    flgmem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    LbTextSetFont(winfont);
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y,
        gbtn->width, gbtn->height, 0); // The 0 means black color
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y,
        gbtn->width, gbtn->height);
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    struct Thing *thing;
    thing = INVALID_THING;
    int listitm_idx;
    listitm_idx = selected_transfer_creature(dungeon, gbtn);
    int tx_units_per_px;
    tx_units_per_px = ((gbtn->height*22/26) * 16) / LbTextLineHeight();
    if (listitm_idx != -1)
    {
        thing = get_player_list_nth_creature_of_model(dungeon->creatr_list_start, 0, listitm_idx);
    }
    if (!thing_is_invalid(thing))
    {
        const struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        const struct CreatureData *crdata;
        crdata = creature_data_get_from_thing(thing);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        LbTextDrawResizedFmt(0, 0, tx_units_per_px, " %s", get_string(crdata->namestr_idx));
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        LbTextDrawResizedFmt(0, 0, tx_units_per_px, "%s %d ", get_string(GUIStr_MnuLevel), (int)(cctrl->explevel+1));
    }
    lbDisplay.DrawFlags = flgmem;
}

void select_transfer_creature_up(struct GuiButton *gbtn)
{
    if (transfer_creature_scroll_offset > 0) {
        transfer_creature_scroll_offset--;
    }
}

void select_transfer_creature_down(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    if (transfer_creature_scroll_offset < dungeon->num_active_creatrs-transfer_creature_items_visible+1) {
        transfer_creature_scroll_offset++;
    }
}

void maintain_resurrect_creature_select(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    long listitm_idx;
    listitm_idx = resurrect_creature_scroll_offset + gbtn->btype_value;
    set_flag_byte(&gbtn->flags, LbBtnFlag_Enabled, (listitm_idx < dungeon->dead_creatures_count));
}

void maintain_resurrect_creature_scroll(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int count;
    count = dungeon->dead_creatures_count;
    if (resurrect_creature_scroll_offset >= count-resurrect_creature_items_visible+1)
    {
        if (count+1 > resurrect_creature_items_visible) {
            resurrect_creature_scroll_offset = count-resurrect_creature_items_visible+1;
        } else {
            resurrect_creature_scroll_offset = 0;
        }
    }
    if (gbtn->btype_value == 1)
    {
        set_flag_byte(&gbtn->flags, LbBtnFlag_Enabled, (resurrect_creature_scroll_offset > 0));
    }
    else
    {
        set_flag_byte(&gbtn->flags, LbBtnFlag_Enabled, (resurrect_creature_scroll_offset < count-resurrect_creature_items_visible+1));
    }
}

void maintain_transfer_creature_select(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    long listitm_idx;
    listitm_idx = transfer_creature_scroll_offset + gbtn->btype_value;

    set_flag_byte(&gbtn->flags, LbBtnFlag_Enabled,  (listitm_idx < dungeon->num_active_creatrs));
}

void maintain_transfer_creature_scroll(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int count;
    count = dungeon->num_active_creatrs;
    if (transfer_creature_scroll_offset > count-transfer_creature_items_visible+1)
    {
        if (count > transfer_creature_items_visible) {
            transfer_creature_scroll_offset = count-transfer_creature_items_visible+1;
        } else {
            transfer_creature_scroll_offset = 0;
        }
    }
    if (gbtn->btype_value == 1)
    {
        set_flag_byte(&gbtn->flags, LbBtnFlag_Enabled, (transfer_creature_scroll_offset > 0));
    }
    else
    {
        set_flag_byte(&gbtn->flags, LbBtnFlag_Enabled, (transfer_creature_scroll_offset < count-transfer_creature_items_visible+1));
    }
}

void choose_hold_audience(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_HoldAudience, 0, 0);
}

void choose_armageddon(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_UsePwrArmageddon, 0, 0);
}
/******************************************************************************/
