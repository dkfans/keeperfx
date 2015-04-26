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
int number_of_freeplay_levels = 0;
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
        gbtn->flags |= LbBtnFlag_Unknown08;
    else
        gbtn->flags &=  ~LbBtnFlag_Unknown08;
}

void frontend_level_select_down_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_level_scroll_offset < number_of_freeplay_levels-frontend_select_level_items_visible+1)
        gbtn->flags |= LbBtnFlag_Unknown08;
    else
        gbtn->flags &=  ~LbBtnFlag_Unknown08;
}

void frontend_level_select_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    long i;
    i = (long)gbtn->content - 45;
    if (select_level_scroll_offset+i < number_of_freeplay_levels)
        gbtn->flags |= LbBtnFlag_Unknown08;
    else
        gbtn->flags &=  ~LbBtnFlag_Unknown08;
}

void frontend_draw_level_select_button(struct GuiButton *gbtn)
{
    struct LevelInformation *lvinfo;
    long btn_idx;
    long lvnum;
    long i;
    btn_idx = (long)gbtn->content;
    i = btn_idx + select_level_scroll_offset - 45;
    lvnum = 0;
    if ((i >= 0) && (i < campaign.freeplay_levels_count))
      lvnum = campaign.freeplay_levels[i];
    lvinfo = get_level_info(lvnum);
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
    int tx_units_per_px;
    // This text is a bit condensed - button size is smaller than text height
    tx_units_per_px = (gbtn->height*13/11) * 16 / LbTextLineHeight();
    i = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, i);
    LbTextDrawResized(0, 0, tx_units_per_px, lvinfo->name);
}

void frontend_draw_levels_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, select_level_scroll_offset, frontend_select_level_items_visible-2, number_of_freeplay_levels);
}

void frontend_level_select(struct GuiButton *gbtn)
{
    // Find the level number
    long i;
    long lvnum;
    i = (long)gbtn->content + select_level_scroll_offset - 45;
    lvnum = 0;
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
    // Load the default campaign (free play levels should be played with default campaign settings)
    if (!change_campaign("")) {
        number_of_freeplay_levels = 0;
        return;
    }
    number_of_freeplay_levels = campaign.freeplay_levels_count;
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
        gbtn->flags |= LbBtnFlag_Unknown08;
    else
        gbtn->flags &=  ~LbBtnFlag_Unknown08;
}

void frontend_campaign_select_down_maintain(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    if (select_campaign_scroll_offset < campaigns_list.items_num-frontend_select_campaign_items_visible+1)
        gbtn->flags |= LbBtnFlag_Unknown08;
    else
        gbtn->flags &=  ~LbBtnFlag_Unknown08;
}

void frontend_campaign_select_maintain(struct GuiButton *gbtn)
{
  long btn_idx;
  long i;
  if (gbtn == NULL)
    return;
  btn_idx = (long)gbtn->content;
  i = select_campaign_scroll_offset + btn_idx-45;
  if (i < campaigns_list.items_num)
      gbtn->flags |= LbBtnFlag_Unknown08;
  else
      gbtn->flags &=  ~LbBtnFlag_Unknown08;
}

void frontend_draw_campaign_select_button(struct GuiButton *gbtn)
{
    struct GameCampaign *campgn;
    long btn_idx;
    long i;
    if (gbtn == NULL)
      return;
    btn_idx = (long)gbtn->content;
    i = select_campaign_scroll_offset + btn_idx-45;
    campgn = NULL;
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
    int tx_units_per_px;
    // This text is a bit condensed - button size is smaller than text height
    tx_units_per_px = (gbtn->height*13/11) * 16 / LbTextLineHeight();
    i = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, i);
    LbTextDrawResized(0, 0, tx_units_per_px, campgn->name);
}

void frontend_campaign_select(struct GuiButton *gbtn)
{
    long i;
    long btn_idx;
    struct GameCampaign *campgn;
    if (gbtn == NULL)
        return;
    btn_idx = (long)gbtn->content;
    i = select_campaign_scroll_offset + btn_idx-45;
    campgn = NULL;
    if ((i >= 0) && (i < campaigns_list.items_num))
        campgn = &campaigns_list.items[i];
    if (campgn == NULL)
        return;
    if (!frontend_start_new_campaign(campgn->fname))
    {
        ERRORLOG("Unable to start new campaign");
        return;
    }
    frontend_set_state(FeSt_LAND_VIEW);
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
}

void frontend_draw_campaign_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, select_campaign_scroll_offset, frontend_select_campaign_items_visible-2, campaigns_list.items_num);
}
/******************************************************************************/
