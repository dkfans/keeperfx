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
#include "pre_inc.h"
#include "front_highscore.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_keybrd.h"
#include "bflib_inputctrl.h"
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
#include "sprites.h"
#include "gui_frontbtns.h"
#include "custom_sprites.h"
#include "highscores.h"
#include "post_inc.h"

/******************************************************************************/
static unsigned long high_score_entry_index;

char high_score_entry[HISCORE_NAME_LENGTH];
int fe_high_score_table_from_main_menu;
long high_score_entry_input_active = -1;
int highscore_scroll_offset = 0;
unsigned long scores_count;
/******************************************************************************/

static void finalize_high_score_entry(TbBool restore_default_name)
{
    if ((high_score_entry_input_active < 0) || (high_score_entry_input_active >= campaign.hiscore_count))
    {
        return;
    }
    struct HighScore* hscore = &campaign.hiscore_table[high_score_entry_input_active];
    if (restore_default_name)
    {
        snprintf(hscore->name, HISCORE_NAME_LENGTH, "%s", get_string(GUIStr_Keeper));
    }
    else
    {
        snprintf(hscore->name, HISCORE_NAME_LENGTH, "%s", high_score_entry);
    }
    highscore_scroll_offset = high_score_entry_input_active - (VISIBLE_HIGH_SCORES_COUNT-1);
    high_score_entry_input_active = -1;
    save_high_score_table();
}

void draw_high_score_entry(int idx, long pos_x, long pos_y, int col1_width, int col2_width, int col3_width, int col4_width, int units_per_px)
{
    if ((idx >= scores_count) || (campaign.hiscore_table == NULL))
    {
        return;
    }
    struct HighScore* hscore = &campaign.hiscore_table[idx];
    // TODO: These were originally right-aligned, but there's a glitch that causes longer numbers to be aligned weirdly at some resolutions in dbc mode.
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    int i = pos_x + col1_width;
    LbTextNumberDraw(i, pos_y, units_per_px, idx+1, Fnt_CenterPos);
    i += col2_width;
    LbTextNumberDraw(i, pos_y, units_per_px, hscore->score, Fnt_LeftJustify);
    i += col3_width;
    LbTextNumberDraw(i, pos_y, units_per_px, hscore->lvnum, Fnt_LeftJustify);
    i += col4_width;
    if (idx == high_score_entry_input_active)
    {
        char str[HISCORE_NAME_LENGTH];
        snprintf(str, sizeof(str), "%s", high_score_entry);
        // Blinking cursor
        if ((LbTimerClock() & 0x0100) != 0)
        {
            size_t len = strlen(str);
            if (len < (HISCORE_NAME_LENGTH - 1))
            {
                str[len] = '_';
                str[len+1] = '\0';
            }
        }
        LbTextStringDraw(i, pos_y, units_per_px, str, Fnt_LeftJustify);
    } else
    {
        LbTextStringDraw(i, pos_y, units_per_px, hscore->name, Fnt_LeftJustify);
    }
}

void frontend_draw_high_score_table(struct GuiButton *gbtn)
{
    scores_count = count_high_scores();
    gui_draw_scroll_box(gbtn, 12, true);
    int fs_units_per_px;
    const struct TbSprite *spr;
    {
        int orig_size = 0;
        spr = get_frontend_sprite(GFS_hugearea_thn_cor_ml);
        for (int i=0; i < 6; i++)
        {
            orig_size += spr->SWidth;
            spr++;
        }
        fs_units_per_px = (gbtn->width * 16 + orig_size/2) / orig_size;
    }
    LbTextSetFont(frontend_font[1]);
    lbDisplay.DrawFlags = 0;
    spr = get_frontend_sprite(GFS_hugearea_thn_cor_ml);
    int pos_x = gbtn->scr_pos_x + spr->SWidth * fs_units_per_px / 16;
    spr = get_frontend_sprite(GFS_hugearea_thn_cor_tl);
    int pos_y = gbtn->scr_pos_y + (spr->SHeight + 3) * fs_units_per_px / 16;
    int tx_units_per_px = scale_value_menu(16);
    // The GUI item height should be 11 lines of text
    long col1_width = LbTextStringWidthM("99", tx_units_per_px);
    long col2_width = LbTextStringWidthM("  999", tx_units_per_px);
    long col3_width = LbTextStringWidthM("   9999", tx_units_per_px);
    long col4_width = LbTextStringWidthM(" 99999", tx_units_per_px);
    int k;
    if (high_score_entry_input_active >= 0)
    {
        if (high_score_entry_input_active <= VISIBLE_HIGH_SCORES_COUNT)
        {
            highscore_scroll_offset = 0;
        }
        else
        {
            highscore_scroll_offset = high_score_entry_input_active - (VISIBLE_HIGH_SCORES_COUNT-1);
        }
    }
    for (k=highscore_scroll_offset; k < (highscore_scroll_offset+VISIBLE_HIGH_SCORES_COUNT)-1; k++)
    {
        draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width, tx_units_per_px);
        pos_y += LbTextLineHeight() * tx_units_per_px / 16;
        if (dbc_language > 0)
        {
            pos_y += scale_value_menu(4);
        }
    }
    if (high_score_entry_input_active > k)
    {
        draw_high_score_entry(high_score_entry_input_active, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width, tx_units_per_px);
    }
    else
    {
        if (pos_y < (gbtn->scr_pos_y + gbtn->height))
        {
            draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width, tx_units_per_px);
        }
    }
}

void frontend_quit_high_score_table(struct GuiButton *gbtn)
{
    finalize_high_score_entry(false);
    FrontendMenuState nstate = get_menu_state_when_back_from_substate(FeSt_HIGH_SCORES);
    frontend_set_state(nstate);
}

/**
 * Does high score table new entry input.
 * @return True if the entry input is active, false otherwise.
 */
TbBool frontend_high_score_table_input(void)
{
    unsigned long i;
    if (high_score_entry_input_active >= campaign.hiscore_count)
        high_score_entry_input_active = -1;
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
            high_score_entry_index--;
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
        finalize_high_score_entry(lbInkey == KC_ESCAPE);
        clear_key_pressed(lbInkey);
        return true;
    }
    char chr = key_to_ascii(lbInkey, key_modifiers);
    if (chr != 0)
    {
        LbTextSetFont(frontend_font[1]);
        int tx_units_per_px;
        if (dbc_language > 0)
        {
            tx_units_per_px = scale_value_menu(24);
        }
        else
        {
            tx_units_per_px = scale_value_menu(16);
        }
        i = LbTextCharWidthM(chr, tx_units_per_px);
        size_t entry_len = strlen(high_score_entry);
        if ((entry_len < (HISCORE_NAME_LENGTH - 1)) &&
            ((i > 0) && (i + LbTextStringWidth(high_score_entry) < 260)))
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
    // No input, but return true to make sure other input functions are skipped
    return true;
}

void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn)
{
    if ((high_score_entry_input_active == -1) || (last_used_input_device == ID_Controller))
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
        if (last_used_input_device == ID_Controller)
        {
            snprintf(high_score_entry, HISCORE_NAME_LENGTH, "%s", get_string(GUIStr_Keeper));
        }
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

void highscore_scroll_up(struct GuiButton *gbtn)
{
    if (highscore_scroll_offset > 0)
      highscore_scroll_offset--;
}

void highscore_scroll_down(struct GuiButton *gbtn)
{
  if (highscore_scroll_offset < scores_count-VISIBLE_HIGH_SCORES_COUNT)
    highscore_scroll_offset++;
}

void highscore_scroll(struct GuiButton *gbtn)
{
    highscore_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), VISIBLE_HIGH_SCORES_COUNT-1, scores_count);
}

void frontend_highscore_scroll_up_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (scores_count > VISIBLE_HIGH_SCORES_COUNT)
        gbtn->flags |= LbBtnF_Visible;
    else
        gbtn->flags &= ~LbBtnF_Visible;
    if (highscore_scroll_offset != 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontend_highscore_scroll_down_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (scores_count > VISIBLE_HIGH_SCORES_COUNT)
        gbtn->flags |= LbBtnF_Visible;
    else
        gbtn->flags &= ~LbBtnF_Visible;
    if (highscore_scroll_offset < scores_count-VISIBLE_HIGH_SCORES_COUNT)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontend_highscore_scroll_tab_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (scores_count > VISIBLE_HIGH_SCORES_COUNT)
        gbtn->flags |= LbBtnF_Visible;
    else
        gbtn->flags &= ~LbBtnF_Visible;
}

void frontend_draw_highscores_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, highscore_scroll_offset, VISIBLE_HIGH_SCORES_COUNT-1, scores_count);
}

void frontend_high_scores_update()
{
    if (scores_count == 0)
    {
        highscore_scroll_offset = 0;
    }
    else if (highscore_scroll_offset < 0)
    {
        highscore_scroll_offset = 0;
    } 
    else if (highscore_scroll_offset > scores_count-VISIBLE_HIGH_SCORES_COUNT+1)
    {
        if (highscore_scroll_offset != high_score_entry_input_active)
        {
            highscore_scroll_offset = scores_count-VISIBLE_HIGH_SCORES_COUNT+1;
        }
    }
    if (scores_count > VISIBLE_HIGH_SCORES_COUNT)
    {
        if (wheel_scrolled_down || (is_key_pressed(KC_DOWN,KMod_NONE)))
        {
            if (highscore_scroll_offset < scores_count-VISIBLE_HIGH_SCORES_COUNT)
            {
                highscore_scroll_offset++;
            }
        }
        else if (wheel_scrolled_up || (is_key_pressed(KC_UP,KMod_NONE)))
        {
            if (highscore_scroll_offset > 0)
            {
                highscore_scroll_offset--;
            }
        }
    }
}

void frontend_draw_highscores_scroll_box_tab(struct GuiButton *gbtn)
{
    int fs_units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, GFS_hugearea_thc_tx1_tc, 100);
    const struct TbSprite *spr = get_frontend_sprite(GFS_hugearea_thc_tx1_tc);
    int pos_x = gbtn->scr_pos_x;
    // Since this tab is attachable from top, it is important to keep bottom position without variation
    int pos_y = gbtn->scr_pos_y + gbtn->height - spr->SHeight * fs_units_per_px / 16;
    spr = get_frontend_sprite(GFS_hugearea_thc_cor_tl);
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = get_frontend_sprite(GFS_hugearea_thc_tx1_tc);
    for (int i = 3; i > 0; i--)
    {
        LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
        pos_x += spr->SWidth * fs_units_per_px / 16;
    }
    spr = get_frontend_sprite(GFS_hugearea_thc_cor_tr);
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
}

void frontend_draw_high_scores_mappack(struct GuiButton *gbtn)
{
    const char *text;
    if (campaign.display_name[0] != '\0')
        text = campaign.display_name;
    else
        text = frontend_button_caption_text(gbtn);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    LbTextSetFont(frontend_font[2]);
    int tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    LbTextDrawResized((dbc_language > 0) ? -30 : 0, 0, tx_units_per_px, text);
}

unsigned long count_high_scores()
{
    unsigned long i;
    for (i = 0; i < campaign.hiscore_count; i++)
    {
        struct HighScore* hscore = &campaign.hiscore_table[i];
        if ( (hscore->name[0] == '\0') && (hscore->score == 0) && (hscore->lvnum == 0) )
        {
            break;
        }
    }
    return i;
}

/******************************************************************************/
