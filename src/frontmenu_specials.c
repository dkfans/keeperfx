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
#include "pre_inc.h"
#include "frontmenu_specials.h"
#include "globals.h"
#include "bflib_basics.h"
#include "custom_sprites.h"
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
#include "front_input.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

// Non-NULL no-op callback so that the controller snapping logic does not ignore the button
static void no_op(struct GuiButton* gbtn) {}

#define resurrect_creature_items_visible  6
struct GuiButtonInit resurrect_creature_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, gui_area_text,                     1, GUIStr_SpecResurrectCreature,0,{0},             0, NULL },
  { 0,  0, 0, 0, select_resurrect_creature,NULL,  NULL,               0, 999,  62, 999,  62,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, select_resurrect_creature,NULL,  NULL,               1, 999,  90, 999,  90,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, select_resurrect_creature,NULL,  NULL,               2, 999, 118, 999, 118,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, select_resurrect_creature,NULL,  NULL,               3, 999, 146, 999, 146,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, select_resurrect_creature,NULL,  NULL,               4, 999, 174, 999, 174,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_select },
  { 0,  0, 0, 0, select_resurrect_creature,NULL,  NULL,               5, 999, 202, 999, 202,250, 26, draw_resurrect_creature,           0, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_select },
  { 1,  0, 0, 0, select_resurrect_creature_up,NULL,NULL,              1, 305,  62, 305,  62, 22, 24, gui_area_new_normal_button,      278, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_scroll },
  { 1,  0, 0, 0, select_resurrect_creature_down,NULL,NULL,            2, 305, 204, 305, 204, 22, 24, gui_area_new_normal_button,      280, GUIStr_Empty,       0,       {0},               0, maintain_resurrect_creature_scroll },
  { 0,  0, 0, 1, no_op,              NULL,        NULL,               0, 999, 258, 999, 258,100, 32, gui_area_text,                     1, GUIStr_MnuCancel,   0,       {0},               0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},               0, NULL },
};

#define transfer_creature_items_visible  6
struct GuiButtonInit transfer_creature_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,200, 32, gui_area_text,                     1, GUIStr_SpecTransferCreature,0,{0},              0, NULL },
  { 0,  0, 0, 0, select_transfer_creature,NULL,   NULL,               0, 999,  62, 999,  62,250, 26, draw_transfer_creature,            0, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, select_transfer_creature,NULL,   NULL,               1, 999,  90, 999,  90,250, 26, draw_transfer_creature,            1, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, select_transfer_creature,NULL,   NULL,               2, 999, 118, 999, 118,250, 26, draw_transfer_creature,            2, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, select_transfer_creature,NULL,   NULL,               3, 999, 146, 999, 146,250, 26, draw_transfer_creature,            3, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, select_transfer_creature,NULL,   NULL,               4, 999, 174, 999, 174,250, 26, draw_transfer_creature,            4, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_select },
  { 0,  0, 0, 0, select_transfer_creature,NULL,   NULL,               5, 999, 202, 999, 202,250, 26, draw_transfer_creature,            5, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_select },
  { 1,  0, 0, 0, select_transfer_creature_up,NULL,NULL,               1, 305,  62, 305,  62, 22, 24, gui_area_new_normal_button,      278, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_scroll },
  { 1,  0, 0, 0, select_transfer_creature_down,NULL,NULL,             2, 305, 204, 305, 204, 22, 24, gui_area_new_normal_button,      280, GUIStr_Empty,       0,       {0},               0, maintain_transfer_creature_scroll },
  { 0,  0, 0, 1, no_op,              NULL,        NULL,               0, 999, 258, 999, 258,100, 32, gui_area_text,                     1, GUIStr_MnuCancel,   0,       {0},               0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},               0, NULL },
};

struct GuiButtonInit hold_audience_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, CpgStr_PowerKind1+4,0,       {0},               0, NULL },
  { 0,  0, 0, 1, no_op,              NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, gui_area_normal_button,           46, GUIStr_Empty,       0,       {0},               0, NULL },
  { 0,  0, 0, 1, choose_hold_audience,NULL,       NULL,               0, 116,  24, 118,  58, 46, 32, gui_area_normal_button,           48, GUIStr_Empty,       0,       {0},               0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},               0, NULL },
};

struct GuiButtonInit armageddon_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, CpgStr_PowerKind1+16,0,      {0},               0, NULL },
  { 0,  0, 0, 1, no_op,              NULL,        NULL,               0,  38,  24,  40,  58, 46, 32, gui_area_normal_button,           46, GUIStr_Empty,       0,       {0},               0, NULL },
  { 0,  0, 0, 1, choose_armageddon,  NULL,        NULL,               0, 116,  24, 118,  58, 46, 32, gui_area_normal_button,           48, GUIStr_Empty,       0,       {0},               0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},               0, NULL },
};

struct GuiButtonInit dungeon_special_buttons[] = {
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                0,       {0},               0, NULL },
};

struct GuiMenu hold_audience_menu =
 {     GMnu_HOLD_AUDIENCE, 0, 4, hold_audience_buttons,      POS_GAMECTR,POS_GAMECTR,200, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu dungeon_special_menu =
 {   GMnu_DUNGEON_SPECIAL, 0, 4, dungeon_special_buttons,            160, POS_SCRBTM,480, 86, gui_round_glass_background,   0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu resurrect_creature_menu =
 {GMnu_RESURRECT_CREATURE, 0, 4, resurrect_creature_buttons, POS_GAMECTR,POS_GAMECTR,350, 300, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu transfer_creature_menu =
 { GMnu_TRANSFER_CREATURE, 0, 4, transfer_creature_buttons,  POS_GAMECTR,POS_GAMECTR,350, 300, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu armageddon_menu =
 {        GMnu_ARMAGEDDON, 0, 4, armageddon_buttons,         POS_GAMECTR,POS_GAMECTR,200, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
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
        listitm_idx = resurrect_creature_scroll_offset + (gbtn->btype_value & LbBFeF_IntValueMask);
        if (listitm_idx < dungeon->dead_creatures_count) {
            return listitm_idx;
        }
    } else
    {
        listitm_idx = resurrect_creature_scroll_offset + (gbtn->btype_value & LbBFeF_IntValueMask);
        if (listitm_idx < DEAD_CREATURES_MAX_COUNT) {
            return abs(dungeon->dead_creature_idx + listitm_idx) % DEAD_CREATURES_MAX_COUNT;
        }
    }
    return -1;
}

void select_resurrect_creature(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    int i = selected_resurrect_creature(dungeon, gbtn);
    if (i != -1)
    {
        struct CreatureStorage* cstore = &dungeon->dead_creatures[i];
        struct Packet* pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_ResurrectCrtr, dungeon_special_selected, dungeon->owner | (cstore->model << 4) | (cstore->exp_level << 12), 0, 0);
        turn_off_menu(GMnu_RESURRECT_CREATURE);
    }
}

void draw_resurrect_creature(struct GuiButton *gbtn)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height, 0);
    LbTextSetFont(winfont);
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    struct Dungeon* dungeon = get_my_dungeon();
    int i = selected_resurrect_creature(dungeon, gbtn);
    int tx_units_per_px = scale_ui_value_lofi(16);
    if (i != -1)
    {
        struct CreatureStorage* cstore = &dungeon->dead_creatures[i];
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[cstore->model];
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        long spr_idx = get_creature_model_graphics(cstore->model, CGI_HandSymbol);
        const struct TbSprite* spr = get_panel_sprite(spr_idx);
        int x = gbtn->scr_pos_x - scale_ui_value_lofi(1);
        int y = gbtn->scr_pos_y - (19 * tx_units_per_px / 16);

        if (LbGraphicsScreenHeight() < 400)
        {
            y = gbtn->scr_pos_y - (19 * tx_units_per_px / 32);
        }
        LbSpriteDrawResized(x, y, tx_units_per_px, spr);
        int h = scale_ui_value_lofi(gbtn->height) / 16;
        int w = scale_ui_value_lofi(spr->SWidth + 2);
        LbTextDrawResizedFmt(w, h, tx_units_per_px, "%s", get_string(crconf->namestr_idx));
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        if ( (MyScreenHeight < 400) && (dbc_language == 1) )
        {
            LbTextDrawResizedFmt(0, h, tx_units_per_px, "%u", (cstore->exp_level+1));
        }
        else
        {
            LbTextDrawResizedFmt(0, h, tx_units_per_px, " %s %u", get_string(GUIStr_MnuLevel), (cstore->exp_level+1));
        }
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
    struct Dungeon* dungeon = get_my_dungeon();
    if (resurrect_creature_scroll_offset < dungeon->dead_creatures_count-resurrect_creature_items_visible+1) {
        resurrect_creature_scroll_offset++;
    }
}

int selected_transfer_creature(const struct Dungeon *dungeon, const struct GuiButton *gbtn)
{
    long listitm_idx = transfer_creature_scroll_offset + (gbtn->btype_value & LbBFeF_IntValueMask);
    if (listitm_idx < count_player_creatures_for_transfer(dungeon->owner)) {
        return listitm_idx;
    }
    return -1;
}

void select_transfer_creature(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    struct Thing* thing = INVALID_THING;
    int listitm_idx = selected_transfer_creature(dungeon, gbtn);
    if (listitm_idx != -1)
    {
        thing = get_player_list_nth_creature_with_property(dungeon->creatr_list_start, CMF_NoTransfer, listitm_idx);
    }
    if (thing_exists(thing))
    {
        struct Packet* pckt = get_packet(my_player_number);
        set_packet_action(pckt, PckA_TransferCreatr, dungeon_special_selected, thing->index, 0, 0);
        turn_off_menu(GMnu_TRANSFER_CREATURE);
    }
}

void draw_transfer_creature(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
      return;
    SYNCDBG(7,"Starting");
    unsigned long flgmem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    LbTextSetFont(winfont);
    LbDrawBox(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height, 0); // The 0 means black color
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    struct Dungeon* dungeon = get_my_dungeon();
    struct Thing* thing = INVALID_THING;
    int listitm_idx = selected_transfer_creature(dungeon, gbtn);
    int tx_units_per_px = scale_ui_value_lofi(16);
    if (listitm_idx != -1)
    {
        thing = get_player_list_nth_creature_with_property(dungeon->creatr_list_start, CMF_NoTransfer, listitm_idx);
    }
    if (!thing_is_invalid(thing))
    {
        const struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[thing->model];
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        long spr_idx = get_creature_model_graphics(thing->model, CGI_HandSymbol);
        const struct TbSprite* spr = get_panel_sprite(spr_idx);
        int x = gbtn->scr_pos_x - scale_ui_value_lofi(1);
        int y = gbtn->scr_pos_y - (19 * tx_units_per_px / 16);
        if (LbGraphicsScreenHeight() < 400)
        {
            y = gbtn->scr_pos_y - (19 * tx_units_per_px / 32);
        }
        LbSpriteDrawResized(x, y, tx_units_per_px, spr);
        int h = scale_ui_value_lofi(gbtn->height)/16;
        int w = scale_ui_value_lofi(spr->SWidth + 2);
        LbTextDrawResizedFmt(w, h, tx_units_per_px, "%s", get_string(crconf->namestr_idx));
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        if ( (MyScreenHeight < 400) && (dbc_language == 1) )
        {
            LbTextDrawResizedFmt(0, h, tx_units_per_px, "%u", (cctrl->exp_level+1));
        }
        else
        {
            LbTextDrawResizedFmt(0, h, tx_units_per_px, " %s %u", get_string(GUIStr_MnuLevel), (cctrl->exp_level+1));
        }
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
    struct Dungeon* dungeon = get_my_dungeon();
    if (transfer_creature_scroll_offset < count_player_creatures_for_transfer(dungeon->owner)-transfer_creature_items_visible+1) {
        transfer_creature_scroll_offset++;
    }
}

void maintain_resurrect_creature_select(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    long listitm_idx = resurrect_creature_scroll_offset + (gbtn->btype_value & LbBFeF_IntValueMask);
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (listitm_idx < dungeon->dead_creatures_count)) & LbBtnF_Enabled;
}

void maintain_resurrect_creature_scroll(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    int count = dungeon->dead_creatures_count;
    if (resurrect_creature_scroll_offset >= count-resurrect_creature_items_visible+1)
    {
        if (count+1 > resurrect_creature_items_visible) {
            resurrect_creature_scroll_offset = count-resurrect_creature_items_visible+1;
        } else {
            resurrect_creature_scroll_offset = 0;
        }
    }
    if ((gbtn->btype_value & LbBFeF_IntValueMask) == 1) {
        gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (resurrect_creature_scroll_offset > 0)) & LbBtnF_Enabled;
    } else {
        gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (resurrect_creature_scroll_offset < count-resurrect_creature_items_visible+1)) & LbBtnF_Enabled;
    }
    // Arrow keys are used for camera movement, so not here.
    if (wheel_scrolled_up)
    {
        if (resurrect_creature_scroll_offset > 0)
        {
            resurrect_creature_scroll_offset--;
        }
    }
    if (wheel_scrolled_down)
    {
        if (resurrect_creature_scroll_offset < dungeon->dead_creatures_count-resurrect_creature_items_visible+1)
        {
            resurrect_creature_scroll_offset++;
        }
    }
}

void maintain_transfer_creature_select(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    long listitm_idx = transfer_creature_scroll_offset + (gbtn->btype_value & LbBFeF_IntValueMask);
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (listitm_idx < dungeon->num_active_creatrs)) & LbBtnF_Enabled;
}

void maintain_transfer_creature_scroll(struct GuiButton *gbtn)
{
    struct Dungeon* dungeon = get_my_dungeon();
    int count = count_player_creatures_for_transfer(dungeon->owner);
    if (transfer_creature_scroll_offset > count-transfer_creature_items_visible+1)
    {
        if (count > transfer_creature_items_visible) {
            transfer_creature_scroll_offset = count-transfer_creature_items_visible+1;
        } else {
            transfer_creature_scroll_offset = 0;
        }
    }
    if ((gbtn->btype_value & LbBFeF_IntValueMask) == 1) {
        gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (transfer_creature_scroll_offset > 0)) & LbBtnF_Enabled;
    } else {
        gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (transfer_creature_scroll_offset < count-transfer_creature_items_visible+1)) & LbBtnF_Enabled;
    }
    // Arrow keys are used for camera movement, so not here.
    if (wheel_scrolled_up)
    {
        if (transfer_creature_scroll_offset > 0)
        {
            transfer_creature_scroll_offset--;
        }
    }
    if (wheel_scrolled_down)
    {
        if (transfer_creature_scroll_offset < count-transfer_creature_items_visible+1)
        {
            transfer_creature_scroll_offset++;
        }
    }
}

void choose_hold_audience(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_HoldAudience, 0, 0, 0, 0);
}

void choose_armageddon(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_UsePwrArmageddon, 0, 0, 0, 0);
}
/******************************************************************************/
