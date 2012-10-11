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
#include "frontend.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "game_merge.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_frontend_draw_high_score_table(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_quit_high_score_table(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_maintain_high_score_ok_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_high_score_table_input(void);
/******************************************************************************/
void draw_high_score_entry(int idx, long pos_x, long pos_y, int col1_width, int col2_width, int col3_width, int col4_width)
{
    struct HighScore *hscore;
    int i;
    if ((idx >= campaign.hiscore_count) || (campaign.hiscore_table == NULL))
      return;
    hscore = &campaign.hiscore_table[idx];
    lbDisplay.DrawFlags = 0x80;
    i = pos_x + col1_width;
    LbTextNumberDraw(i, pos_y, idx+1, Fnt_RightJustify);
    i += col2_width;
    LbTextNumberDraw(i, pos_y, hscore->score, Fnt_RightJustify);
    i += col3_width;
    LbTextNumberDraw(i, pos_y, hscore->lvnum, Fnt_RightJustify);
    i += col4_width;
    if (idx == high_score_entry_input_active)
    {
      i += LbTextStringDraw(i, pos_y, high_score_entry, Fnt_LeftJustify);
      // Blinking cursor
      if ((LbTimerClock() & 0x0100) != 0)
      {
        LbTextStringDraw(i, pos_y, "_", Fnt_LeftJustify);
      }
    } else
    {
      LbTextStringDraw(i, pos_y, hscore->name, Fnt_LeftJustify);
    }
}

void frontend_draw_high_score_table(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    struct TbSprite *swpspr;
    long pos_x,pos_y;
    long col1_width,col2_width,col3_width,col4_width;
    long i,k;
//    _DK_frontend_draw_high_score_table(gbtn); return;
    // Draw the high scores area - top
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    spr = &frontend_sprite[25];
    swpspr = spr;
    for (i=6; i > 0; i--)
    {
      LbSpriteDraw(pos_x, pos_y, swpspr);
      pos_x += swpspr->SWidth;
      swpspr++;
    }
    pos_y += spr->SHeight;
    // Draw the high scores area - filling
    k = 12;
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
          LbSpriteDraw(pos_x, pos_y, swpspr);
          pos_x += swpspr->SWidth;
          swpspr++;
        }
        pos_y += spr->SHeight;
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
        LbSpriteDraw(pos_x, pos_y, swpspr);
        pos_x += swpspr->SWidth;
        swpspr++;
    }
    LbTextSetFont(frontend_font[1]);
    lbDisplay.DrawFlags = 0;
    spr = &frontend_sprite[33];
    pos_x = gbtn->scr_pos_x + spr->SWidth;
    spr = &frontend_sprite[25];
    pos_y = spr->SHeight + gbtn->scr_pos_y + 3;
    col1_width = LbTextStringWidth("99");
    col2_width = LbTextStringWidth(" 99999");
    col3_width = LbTextStringWidth(" 999");
    col4_width = LbTextCharWidth('-');
    for (k=0; k < VISIBLE_HIGH_SCORES_COUNT-1; k++)
    {
      draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width);
      pos_y += LbTextLineHeight();
    }
    if (high_score_entry_input_active > k)
      draw_high_score_entry(high_score_entry_input_active, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width);
    else
      draw_high_score_entry(k, pos_x, pos_y, col1_width, col2_width, col3_width, col4_width);
}

void frontend_quit_high_score_table(struct GuiButton *gbtn)
{
    LevelNumber lvnum;
    lvnum = get_loaded_level_number();
    if (fe_high_score_table_from_main_menu)
    {
      frontend_set_state(FeSt_MAIN_MENU);
    } else
    if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum) || is_extra_level(lvnum))
    {
      frontend_set_state(FeSt_LAND_VIEW);
    } else
    if (is_multiplayer_level(lvnum))
    {
      frontend_set_state(FeSt_MAIN_MENU);
    } else
    if (is_freeplay_level(lvnum))
    {
      frontend_set_state(FeSt_LEVEL_SELECT);
    } else
    {
      frontend_set_state(FeSt_MAIN_MENU);
    }
}

TbBool frontend_high_score_table_input(void)
{
  struct HighScore *hscore;
  char chr;
  long i;
  if (high_score_entry_input_active >= campaign.hiscore_count)
    return false;
  if (lbInkey == KC_BACK)
  {
    if (high_score_entry_index > 0)
    {
      i = high_score_entry_index-1;
      high_score_entry[i] = '\0';
      high_score_entry_index = i;
      lbInkey = KC_UNASSIGNED;
      return true;
    }
  }
  if (lbInkey == KC_RETURN)
  {
    hscore = &campaign.hiscore_table[high_score_entry_input_active];
    strncpy(hscore->name, high_score_entry, HISCORE_NAME_LENGTH);
    high_score_entry_input_active = -1;
    save_high_score_table();
    lbInkey = KC_UNASSIGNED;
    return true;
  }
  if (high_score_entry_index < HISCORE_NAME_LENGTH)
  {
    chr = key_to_ascii(lbInkey, key_modifiers);
    if (chr != 0)
    {
      LbTextSetFont(frontend_font[1]);
      i = LbTextCharWidth(chr);
      if ((i > 0) && (i+LbTextStringWidth(high_score_entry) < 308))
      {
        high_score_entry[high_score_entry_index] = chr;
        i = high_score_entry_index+1;
        high_score_entry[i] = 0;
        high_score_entry_index = i;
        lbInkey = KC_UNASSIGNED;
        return true;
      }
    }
  }
  return false;
}

void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->flags, 0x08, (high_score_entry_input_active == -1));
}

void add_score_to_high_score_table(void)
{
  struct Dungeon *dungeon;
  struct PlayerInfo *player;
  int idx;
  player = get_my_player();
  dungeon = get_players_dungeon(player);
  idx = add_high_score_entry(dungeon->lvstats.player_score, get_loaded_level_number(), "");
  if (idx >= 0)
  {
    // Preparing input in the new entry
    // Note that we're not clearing previous name - this way it may be easily kept unchanged
    high_score_entry_input_active = idx;
    high_score_entry_index = 0;
  } else
  {
    high_score_entry_input_active = -1;
    high_score_entry_index = 0;
  }
}

void frontstats_save_high_score(void)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(my_player_number);
  if (dungeon->lvstats.allow_save_score)
  {
    dungeon->lvstats.allow_save_score = false;
    add_score_to_high_score_table();
  }
  lbInkey = 0;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
