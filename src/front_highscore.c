/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_highscore.c
 *     High Score screen displaying routines.
 * @par Purpose:
 *     Functions to show and maintain the high scores screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Jan 2012 - 23 Jun 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_highscore.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_sprite.h"
#include "bflib_guibtns.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
#include "kjm_input.h"
#include "config_campaigns.h"
#include "config_strings.h"
#include "frontend.h"
#include "gui_draw.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "game_merge.h"

/******************************************************************************/
void draw_high_score_entry(int idx, long pos_x, long pos_y, int col1_width, int col2_width, int col3_width, int col4_width, int units_per_px)
{
    if ((idx >= campaign.hiscore_count) || (campaign.hiscore_table == NULL))
      return;
    struct HighScore* hscore = &campaign.hiscore_table[idx];
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
    int i = pos_x + col1_width;
    LbTextNumberDraw(i, pos_y, units_per_px, idx+1, Fnt_RightJustify);
    i += col2_width;
    LbTextNumberDraw(i, pos_y, units_per_px, hscore->score, Fnt_RightJustify);
    i += col3_width;
    LbTextNumberDraw(i, pos_y, units_per_px, hscore->lvnum, Fnt_RightJustify);
    i += col4_width;
    if (idx == high_score_entry_input_active)
    {
        char str[64];
        memcpy(str, high_score_entry, sizeof(str));
        str[sizeof(str)-1] = '\0';
        LbTextStringDraw(i, pos_y, units_per_px, str, Fnt_LeftJustify);
        str[high_score_entry_index] = '\0';
        i += LbTextStringWidth(str) * units_per_px / 16;
        // Blinking cursor
        if ((LbTimerClock() & 0x0100) != 0)
        {
            LbTextStringDraw(i, pos_y, units_per_px, "_", Fnt_LeftJustify);
        }
    } else
    {
        LbTextStringDraw(i, pos_y, units_per_px, hscore->name, Fnt_LeftJustify);
    }
}

void frontend_draw_high_score_table(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    long i;
    // Detect scaling factor is quite complicated for this item
    int fs_units_per_px;
    {
        int orig_size = 0;
        spr = &frontend_sprite[33];
        for (i=0; i < 6; i++)
        {
            orig_size += spr->SWidth;
            spr++;
        }
        fs_units_per_px = (gbtn->width * 16 + orig_size/2) / orig_size;
    }
    // Draw the high scores area - top
    long pos_x = gbtn->scr_pos_x;
    long pos_y = gbtn->scr_pos_y;
    spr = &frontend_sprite[25];
    struct TbSprite* swpspr = spr;
    for (i=6; i > 0; i--)
    {
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, swpspr);
        pos_x += swpspr->SWidth * fs_units_per_px / 16;
        swpspr++;
    }
    pos_y += spr->SHeight * fs_units_per_px / 16;
    // Draw the high scores area - filling
    long k = 12;
    while (k > 0)
    {
        if (k < 3)
          i = 33;
        else
          i = 40;
        spr = &frontend_sprite[i];
        pos_x = gbtn->scr_pos_x;
        swpspr = spr;
        for (i=6; i > 0; i--)
        {
          LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, swpspr);
          pos_x += swpspr->SWidth * fs_units_per_px / 16;
          swpspr++;
        }
        pos_y += spr->SHeight * fs_units_per_px / 16;
        if (k < 3)
          k--;
        else
          k -= 3;
    }
    // Draw the high scores area - bottom
    pos_x = gbtn->scr_pos_x;
    spr = &frontend_sprite[47];
    swpspr = spr;
    for (i=6; i > 0; i--)
    {
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, swpspr);
        pos_x += swpspr->SWidth * fs_units_per_px / 16;
        swpspr++;
    }
    LbTextSetFont(frontend_font[1]);
    lbDisplay.DrawFlags = 0;
    spr = &frontend_sprite[33];
    pos_x = gbtn->scr_pos_x + spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[25];
    pos_y = gbtn->scr_pos_y + (spr->SHeight + 3) * fs_units_per_px / 16;
    // The GUI item height should be 11 lines of text
    int tx_units_per_px = gbtn->height * 16 / (11 * (LbTextLineHeight()));
    long col1_width = LbTextStringWidth("99") * tx_units_per_px / 16;
    long col2_width = LbTextStringWidth(" 99999") * tx_units_per_px / 16;
    long col3_width = LbTextStringWidth(" 9999") * tx_units_per_px / 16;
    long col4_width = LbTextCharWidth('-') * tx_units_per_px / 16;
    for (k=0; k < VISIBLE_HIGH_SCORES_COUNT-1; k++)
    {
        draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width, tx_units_per_px);
        pos_y += LbTextLineHeight() * tx_units_per_px / 16;
    }
    if (high_score_entry_input_active > k)
      draw_high_score_entry(high_score_entry_input_active, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width, tx_units_per_px);
    else
      draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width, tx_units_per_px);
}

void frontend_quit_high_score_table(struct GuiButton *gbtn)
{
    FrontendMenuState nstate = get_menu_state_when_back_from_substate(FeSt_HIGH_SCORES);
    frontend_set_state(nstate);
}

/**
 * Does high score table new entry input.
 * @return True if the entry input is active, false otherwise.
 */
TbBool frontend_high_score_table_input(void)
{
    long i;
    if (high_score_entry_input_active >= campaign.hiscore_count)
        high_score_entry_input_active  = -1;
    if (high_score_entry_input_active < 0)
        return false;
    if (lbInkey == KC_BACK)
    {
        // Delete previous character
        if (high_score_entry_index > 0)
        {
            i = high_score_entry_index-1;
            while (high_score_entry[i] != '\0') {
                high_score_entry[i] = high_score_entry[i+1];
                i++;
            }
            high_score_entry_index -= 1;
        }
        clear_key_pressed(KC_BACK);
        return true;
    }
    if (lbInkey == KC_DELETE)
    {
        // Delete next character
        i = high_score_entry_index;
        while (high_score_entry[i] != '\0') {
            high_score_entry[i] = high_score_entry[i+1];
            i++;
        }
        clear_key_pressed(KC_DELETE);
        return true;
    }
    if (lbInkey == KC_LEFT)
    {
        // Move cursor left
        if (high_score_entry_index > 0) {
            high_score_entry_index--;
        }
        clear_key_pressed(KC_LEFT);
        return true;
    }
    if (lbInkey == KC_RIGHT)
    {
        // Move cursor right
        i = high_score_entry_index;
        if (high_score_entry[i] != '\0') {
            high_score_entry_index++;
        }
        clear_key_pressed(KC_RIGHT);
        return true;
    }
    if ((lbInkey == KC_HOME) || (lbInkey == KC_PGUP))
    {
        // Move cursor to beginning.
        high_score_entry_index = 0;
        clear_key_pressed(lbInkey);
    }
    if ((lbInkey == KC_END) || (lbInkey == KC_PGDOWN))
    {
        // Move cursor to end.
        while (high_score_entry[high_score_entry_index] != '\0')
        {
            high_score_entry_index++;
        }
        clear_key_pressed(lbInkey);
    }
    if ((lbInkey == KC_RETURN) || (lbInkey == KC_NUMPADENTER) || (lbInkey == KC_ESCAPE))
    {
        struct HighScore* hscore = &campaign.hiscore_table[high_score_entry_input_active];
        if (lbInkey == KC_ESCAPE) {
            strncpy(hscore->name, get_string(GUIStr_TeamLeader), HISCORE_NAME_LENGTH);
        } else {
            strncpy(hscore->name, high_score_entry, HISCORE_NAME_LENGTH);
        }
        high_score_entry_input_active = -1;
        save_high_score_table();
        clear_key_pressed(lbInkey);
        return true;
    }
    if (high_score_entry_index < HISCORE_NAME_LENGTH)
    {
        char chr = key_to_ascii(lbInkey, key_modifiers);
        if (chr != 0)
        {
            int entry_len = strlen(high_score_entry);
            LbTextSetFont(frontend_font[1]);
            i = LbTextCharWidth(chr);
            if ((entry_len < (HISCORE_NAME_LENGTH - 1)) &&
                ((i > 0) && (i + LbTextStringWidth(high_score_entry) < 308)))
            {
                i = entry_len;
                high_score_entry[i+1] = '\0';
                while (i > high_score_entry_index) {
                    high_score_entry[i] = high_score_entry[i-1];
                    i--;
                }
                high_score_entry[i] = chr;
                high_score_entry_index = i + 1;
                clear_key_pressed(lbInkey);
                return true;
            }
        }
    }
    // No input, but return true to make sure other input functions are skipped
    return true;
}

void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn)
{
    if (high_score_entry_input_active == -1)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void add_score_to_high_score_table(void)
{
    struct PlayerInfo* player = get_my_player();
    struct Dungeon* dungeon = get_players_dungeon(player);
    int idx = add_high_score_entry(dungeon->lvstats.player_score, get_loaded_level_number(), "");
    if (idx >= 0)
    {
        // Preparing input in the new entry
        // Note that we're not clearing previous name - this way it may be easily kept unchanged
        high_score_entry_input_active = idx;
        high_score_entry_index = strlen(high_score_entry);
    } else
    {
        high_score_entry_input_active = -1;
        high_score_entry_index = 0;
    }
}

void frontstats_save_high_score(void)
{
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    if (dungeon->lvstats.allow_save_score)
    {
        dungeon->lvstats.allow_save_score = false;
        add_score_to_high_score_table();
    }
    lbInkey = 0;
}

/******************************************************************************/
