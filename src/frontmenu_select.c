/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_select.c
 *     GUI menus for level and campaign select screens.
 * @par Purpose:
 *     Structures to show and maintain menus used for level and campaign list screens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     07 Dec 2012 - 11 Aug 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontmenu_select.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "config_strings.h"
#include "game_saves.h"
#include "gui_draw.h"
#include "gui_frontbtns.h"
#include "gui_soundmsgs.h"
#include "player_data.h"
#include "packets.h"
#include "frontend.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "keeperfx.hpp"

/******************************************************************************/
int select_level_scroll_offset = 0;
int select_campaign_scroll_offset = 0;
int select_mappack_scroll_offset = 0;
int number_of_freeplay_levels = 0;
int frontend_select_level_items_visible = 0;
int frontend_select_campaign_items_visible = 0;
int frontend_select_mappack_items_visible = 0;
/******************************************************************************/
void frontend_level_select_up(struct GuiButton *gbtn)
{
    if (select_level_scroll_offset > 0)
      select_level_scroll_offset--;
}

void frontend_level_select_down(struct GuiButton *gbtn)
{
  if (select_level_scroll_offset < number_of_freeplay_levels-frontend_select_level_items_visible+1)
    select_level_scroll_offset++;
}

void frontend_level_select_scroll(struct GuiButton *gbtn)
{
    select_level_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_select_level_items_visible-2, number_of_freeplay_levels);
}

void frontend_level_select_up_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_level_scroll_offset != 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_level_select_down_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_level_scroll_offset < number_of_freeplay_levels-frontend_select_level_items_visible+1)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_level_select_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    long i = (long)gbtn->content - 45;
    if (select_level_scroll_offset+i < number_of_freeplay_levels)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_draw_level_select_button(struct GuiButton *gbtn)
{
    long btn_idx = (long)gbtn->content;
    long i = btn_idx + select_level_scroll_offset - 45;
    long lvnum = 0;
    if ((i >= 0) && (i < campaign.freeplay_levels_count))
      lvnum = campaign.freeplay_levels[i];
    struct LevelInformation* lvinfo = get_level_info(lvnum);
    if (lvinfo == NULL)
      return;
    if ((btn_idx > 0) && (frontend_mouse_over_button == btn_idx))
      i = 2;
    else
    if (get_level_highest_score(lvnum))
      i = 3;
    else
      i = 1;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    LbTextSetFont(frontend_font[i]);
    // This text is a bit condensed - button size is smaller than text height
    int tx_units_per_px = (gbtn->height * 13 / 11) * 16 / LbTextLineHeight();
    i = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, i);
    if (lvinfo->name_stridx > 0)
    {
        LbTextDrawResized(0, 0, tx_units_per_px, get_string(lvinfo->name_stridx));
    }
    else
    {
        LbTextDrawResized(0, 0, tx_units_per_px, lvinfo->name);
    }
}

void frontend_draw_levels_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, select_level_scroll_offset, frontend_select_level_items_visible-2, number_of_freeplay_levels);
}

void frontend_level_select(struct GuiButton *gbtn)
{
    // Find the level number
    long i = (long)gbtn->content + select_level_scroll_offset - 45;
    long lvnum = 0;
    if (i < campaign.freeplay_levels_count)
      lvnum = campaign.freeplay_levels[i];
    if (lvnum <= 0)
        return;
    game.selected_level_number = lvnum;
    game.flags_font |= FFlg_unk80;
    frontend_set_state(FeSt_START_KPRLEVEL);
}

void frontend_level_list_unload(void)
{
  // Nothing needs to be really unloaded; just menu cleanup here
  number_of_freeplay_levels = 0;
}

void frontend_level_list_load(void)
{
    number_of_freeplay_levels = campaign.freeplay_levels_count;
    frontend_select_level_items_visible = (campaign.freeplay_levels_count < frontend_select_level_items_max_visible)?campaign.freeplay_levels_count+1:frontend_select_level_items_max_visible;
}

void frontend_level_select_update(void)
{
  if (number_of_freeplay_levels <= 0)
  {
    select_level_scroll_offset = 0;
  } else
  if (select_level_scroll_offset < 0)
  {
    select_level_scroll_offset = 0;
  } else
  if (select_level_scroll_offset > number_of_freeplay_levels-frontend_select_level_items_visible+1)
  {
    select_level_scroll_offset = number_of_freeplay_levels-frontend_select_level_items_visible+1;
  }
  if (wheel_scrolled_down || (is_key_pressed(KC_DOWN,KMod_NONE)))
  {
    if (select_level_scroll_offset < number_of_freeplay_levels-frontend_select_level_items_visible+1)
    {
        select_level_scroll_offset++;
    }
  }
  if (wheel_scrolled_up || (is_key_pressed(KC_UP,KMod_NONE)))
  {
    if (select_level_scroll_offset > 0)
    {
        select_level_scroll_offset--;
    }
  }
}

void frontend_draw_level_select_mappack(struct GuiButton *gbtn)
{
    const char *text;
    if (campaign.name != NULL)
        text = campaign.name;
    else
        text = frontend_button_caption_text(gbtn);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    LbTextSetFont(frontend_font[2]);
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    LbTextDrawResized(0, 0, tx_units_per_px, text);
}

void frontend_campaign_select_up(struct GuiButton *gbtn)
{
  if (select_campaign_scroll_offset > 0)
      select_campaign_scroll_offset--;
}

void frontend_campaign_select_down(struct GuiButton *gbtn)
{
  if (select_campaign_scroll_offset < campaigns_list.items_num-frontend_select_campaign_items_visible+1)
      select_campaign_scroll_offset++;
}

void frontend_campaign_select_scroll(struct GuiButton *gbtn)
{
    select_campaign_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_select_campaign_items_visible-2, campaigns_list.items_num);
}

void frontend_campaign_select_up_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_campaign_scroll_offset != 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_campaign_select_down_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_campaign_scroll_offset < campaigns_list.items_num-frontend_select_campaign_items_visible+1)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_campaign_select_maintain(struct GuiButton *gbtn)
{
  if (gbtn == NULL)
    return;
  long btn_idx = (long)gbtn->content;
  long i = select_campaign_scroll_offset + btn_idx - 45;
  if (i < campaigns_list.items_num)
      gbtn->flags |= LbBtnF_Enabled;
  else
      gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_draw_campaign_select_button(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
      return;
    long btn_idx = (long)gbtn->content;
    long i = select_campaign_scroll_offset + btn_idx - 45;
    struct GameCampaign* campgn = NULL;
    if ((i >= 0) && (i < campaigns_list.items_num))
      campgn = &campaigns_list.items[i];
    if (campgn == NULL)
      return;
    if ((btn_idx > 0) && (frontend_mouse_over_button == btn_idx))
      i = 2;
    else
/*    if (campaign has been passed)
      i = 3;
    else*/
      i = 1;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    LbTextSetFont(frontend_font[i]);
    // This text is a bit condensed - button size is smaller than text height
    int tx_units_per_px = (gbtn->height * 13 / 11) * 16 / LbTextLineHeight();
    i = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, i);
    LbTextDrawResized(0, 0, tx_units_per_px, campgn->name);
}

void frontend_campaign_select(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    long btn_idx = (long)gbtn->content;
    long i = select_campaign_scroll_offset + btn_idx - 45;
    struct GameCampaign* campgn = NULL;
    if ((i >= 0) && (i < campaigns_list.items_num))
        campgn = &campaigns_list.items[i];
    if (campgn == NULL)
        return;
    if (!frontend_start_new_campaign(campgn->fname))
    {
        ERRORLOG("Unable to start new campaign");
        return;
    }
    frontend_set_state(FeSt_CAMPAIGN_INTRO);
}

void frontend_campaign_select_update(void)
{
    if (campaigns_list.items_num <= 0)
    {
        select_campaign_scroll_offset = 0;
    } else
    if (select_campaign_scroll_offset < 0)
    {
        select_campaign_scroll_offset = 0;
    } else
    if (select_campaign_scroll_offset > campaigns_list.items_num-frontend_select_campaign_items_visible+1)
    {
        select_campaign_scroll_offset = campaigns_list.items_num-frontend_select_campaign_items_visible+1;
    }
    if (wheel_scrolled_down || (is_key_pressed(KC_DOWN,KMod_NONE)))
    {
        if (select_campaign_scroll_offset < campaigns_list.items_num-frontend_select_campaign_items_visible+1)
        {
            select_campaign_scroll_offset++;
        }
    }
    if (wheel_scrolled_up || (is_key_pressed(KC_UP,KMod_NONE)))
    {
        if (select_campaign_scroll_offset > 0)
        {
            select_campaign_scroll_offset--;
        }
    }
}

void frontend_draw_campaign_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, select_campaign_scroll_offset, frontend_select_campaign_items_visible-2, campaigns_list.items_num);
}

void frontend_mappack_list_load(void)
{
    select_level_scroll_offset = 0; // Reset the scroll of the level select screen here, as it should only be reset when user returns to map pack select screen (Not from exiting level etc).
    frontend_select_mappack_items_visible = (mappacks_list.items_num < frontend_select_mappack_items_max_visible)?mappacks_list.items_num+1:frontend_select_mappack_items_max_visible;
}

void frontend_mappack_select_up(struct GuiButton *gbtn)
{
  if (select_mappack_scroll_offset > 0)
      select_mappack_scroll_offset--;
}

void frontend_mappack_select_down(struct GuiButton *gbtn)
{
  if (select_mappack_scroll_offset < mappacks_list.items_num-frontend_select_mappack_items_visible+1)
      select_mappack_scroll_offset++;
}

void frontend_mappack_select_scroll(struct GuiButton *gbtn)
{
    select_mappack_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_select_mappack_items_visible-2, mappacks_list.items_num);
}

void frontend_mappack_select_up_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_mappack_scroll_offset != 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_mappack_select_down_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_mappack_scroll_offset < mappacks_list.items_num-frontend_select_mappack_items_visible+1)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_mappack_select_maintain(struct GuiButton *gbtn)
{
  if (gbtn == NULL)
    return;
  long btn_idx = (long)gbtn->content;
  long i = select_mappack_scroll_offset + btn_idx - 45;
  if (i < mappacks_list.items_num)
      gbtn->flags |= LbBtnF_Enabled;
  else
      gbtn->flags &=  ~LbBtnF_Enabled;
}

void frontend_mappack_select(struct GuiButton *gbtn)
{
    long i;
    long btn_idx;
    struct GameCampaign *campgn;
    if (gbtn == NULL)
        return;
    btn_idx = (long)gbtn->content;
    i = select_mappack_scroll_offset + btn_idx-45;
    campgn = NULL;
    if ((i >= 0) && (i < mappacks_list.items_num))
        campgn = &mappacks_list.items[i];
    if (campgn == NULL)
        return;
    if (!change_campaign(campgn->fname))
        return;
    frontend_set_state(FeSt_LEVEL_SELECT);
}

void frontend_draw_mappack_select_button(struct GuiButton *gbtn)
{
    struct GameCampaign *campgn;
    long btn_idx;
    long i;
    if (gbtn == NULL)
      return;
    btn_idx = (long)gbtn->content;
    i = select_mappack_scroll_offset + btn_idx-45;
    campgn = NULL;
    if ((i >= 0) && (i < mappacks_list.items_num))
      campgn = &mappacks_list.items[i];
    if (campgn == NULL)
      return;
    if ((btn_idx > 0) && (frontend_mouse_over_button == btn_idx))
      i = 2;
    else
      i = 1;
  
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    LbTextSetFont(frontend_font[i]);
    int tx_units_per_px;
    // This text is a bit condensed - button size is smaller than text height
    tx_units_per_px = (gbtn->height*13/11) * 16 / LbTextLineHeight();
    i = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, i);
    LbTextDrawResized(0, 0, tx_units_per_px, campgn->name);
}

void frontend_mappack_select_update(void)
{
    if (mappacks_list.items_num <= 0)
    {
        select_mappack_scroll_offset = 0;
    } else
    if (select_mappack_scroll_offset < 0)
    {
        select_mappack_scroll_offset = 0;
    } else
    if (select_mappack_scroll_offset > mappacks_list.items_num-frontend_select_mappack_items_visible+1)
    {
        select_mappack_scroll_offset = mappacks_list.items_num-frontend_select_mappack_items_visible+1;
    }
    if (wheel_scrolled_down || (is_key_pressed(KC_DOWN,KMod_NONE)))
    {
        if (select_mappack_scroll_offset < mappacks_list.items_num-frontend_select_mappack_items_visible+1)
        {
            select_mappack_scroll_offset++;
        }
    }
    if (wheel_scrolled_up || (is_key_pressed(KC_UP,KMod_NONE)))
    {
        if (select_mappack_scroll_offset > 0)
        {
            select_mappack_scroll_offset--;
        }
    }
}

void frontend_draw_mappack_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, select_mappack_scroll_offset, frontend_select_mappack_items_visible-2, mappacks_list.items_num);
}

void frontend_campaign_list_load(void)
{
    frontend_select_campaign_items_visible = (campaigns_list.items_num < frontend_select_campaign_items_max_visible)?campaigns_list.items_num+1:frontend_select_campaign_items_max_visible;
}
void frontend_draw_variable_mappack_exit_button(struct GuiButton *gbtn)
{
    long str_idx = 111; //Return to Free-Play
    unsigned short mnu_idx = 34; //map pack selection screen
    if (mappacks_list.items_num == 1)
    {
        str_idx = 6; // Return to Main menu
        mnu_idx = 1; //main menu
    }
    gbtn->btype_value = mnu_idx;
    gbtn->content = (unsigned long *)str_idx;
    const char *text;
    text = frontend_button_caption_text(gbtn);
    frontend_draw_button(gbtn, 1, text, Lb_TEXT_HALIGN_CENTER);
}
/******************************************************************************/
