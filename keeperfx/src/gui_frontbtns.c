/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_frontbtns.c
 *     gui_frontbtns support functions.
 * @par Purpose:
 *     Functions to gui_frontbtns.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#if defined(WIN32)
// needed for timeGetTime() -- should be later removed
#include <windows.h>
#endif

#include "gui_frontbtns.h"
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_guibtns.h"
#include "bflib_vidraw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_datetm.h"
#include "kjm_input.h"
#include "gui_draw.h"
#include "gui_frontmenu.h"
#include "frontend.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void gui_clear_buttons_not_over_mouse(int gmbtn_mouseover_idx)
{
    struct GuiButton *gbtn;
    int gidx;
    for (gidx=0;gidx<ACTIVE_BUTTONS_COUNT;gidx++)
    {
      gbtn = &active_buttons[gidx];
      if (gbtn->flags & LbBtnFlag_Created)
        if ( ((gmbtn_mouseover_idx == -1) || (gmbtn_mouseover_idx != gidx)) &&
             (gbtn->button_type != LbBtnT_RadioBtn) && (gbtn != input_button) )
        {
          gbtn->flags &= ~LbBtnFlag_MouseOver;
          gbtn->leftclick_flag = 0;
          gbtn->rightclick_flag = 0;
        }
    }
}

void fake_button_click(int gmbtn_idx)
{
    int i;
    for (i=0; i < ACTIVE_BUTTONS_COUNT; i++)
    {
        struct GuiButton *gbtn;
        gbtn = &active_buttons[i];
        struct GuiMenu *gmnu;
        gmnu = &active_menus[(unsigned)gbtn->menu_idx];
        if (((gbtn->flags & LbBtnFlag_Created) != 0) && (gmnu->is_turned_on != 0) && (gbtn->designation_id == gmbtn_idx))
        {
            if ((gbtn->callback_click != NULL) || (gbtn->flags & LbBtnFlag_CloseCurrentMenu) || (gbtn->parent_menu != NULL) || (gbtn->button_type == LbBtnT_RadioBtn)) {
                do_button_press_actions(gbtn, &gbtn->leftclick_flag, gbtn->callback_click);
            }
            if ((gbtn->callback_click != NULL) || (gbtn->flags & LbBtnFlag_CloseCurrentMenu) || (gbtn->parent_menu != NULL) || (gbtn->button_type == LbBtnT_RadioBtn)) {
                do_button_click_actions(gbtn, &gbtn->leftclick_flag, gbtn->callback_click);
            }
        }
    }
}

TbBool gui_button_release_inputs(int gmbtn_idx)
{
    struct GuiButton *gbtn;
    SYNCDBG(17,"Starting");
    if (gmbtn_idx < 0)
      return false;
    Gf_Btn_Callback callback;
    gbtn = &active_buttons[gmbtn_idx%ACTIVE_BUTTONS_COUNT];
    if ((gbtn->leftclick_flag) && (left_button_released))
    {
        callback = gbtn->callback_click;
        if ((callback != NULL) || ((gbtn->flags & LbBtnFlag_CloseCurrentMenu) != 0) ||
            (gbtn->parent_menu != NULL) || (gbtn->button_type == LbBtnT_RadioBtn))
        {
            left_button_released = 0;
            do_button_release_actions(gbtn, &gbtn->leftclick_flag, callback);
        }
        return true;
    }
    if ((gbtn->rightclick_flag) && (right_button_released))
    {
        callback = gbtn->callback_rightclick;
        if (callback != NULL)
        {
          right_button_released = 0;
          do_button_release_actions(gbtn, &gbtn->rightclick_flag, callback);
        }
        return true;
    }
    return false;
}

TbBool gui_slider_button_inputs(int gbtn_idx)
{
    Gf_Btn_Callback callback;
    int mouse_x;
    int slide_start,slide_end;
    struct GuiButton *gbtn;
    if (gbtn_idx < 0)
      return false;
    gbtn = &active_buttons[gbtn_idx];
    mouse_x = GetMouseX();
    gbtn->leftclick_flag = 1;
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, 2, 44);
    slide_start = gbtn->pos_x + 32*bs_units_per_px/16;
    slide_end = gbtn->pos_x + gbtn->width - 32*bs_units_per_px/16;
    if (mouse_x < slide_start)
    {
        gbtn->slider_value = 0;
    } else
    if (mouse_x >= slide_end)
    {
        gbtn->slider_value = 255;
    } else
    if (gbtn->width > 64*bs_units_per_px/16)
    {
        gbtn->slider_value = ((mouse_x - slide_start) * SLIDER_MAXVALUE) / (gbtn->width - 64 * bs_units_per_px / 16);
    } else
    {
        gbtn->slider_value = ((mouse_x - gbtn->pos_x) * SLIDER_MAXVALUE) / (gbtn->width + 1);
    }
    *gbtn->content = (gbtn->slider_value) * (((long)gbtn->max_value) + 1) / SLIDER_MAXVALUE;
    callback = gbtn->callback_click;
    if (callback != NULL)
      callback(gbtn);
    return true;
}

TbBool gui_slider_button_mouse_over_slider_tracker(int gbtn_idx)
{
    struct GuiButton *gbtn;
    if (gbtn_idx < 0)
      return false;
    gbtn = &active_buttons[gbtn_idx];
    int bs_units_per_px;
    bs_units_per_px = gbtn->height * 16 / 22;
    int slider_pos_x;
    slider_pos_x = gbtn->scr_pos_x + 32*bs_units_per_px/16 + ((gbtn->slider_value)*(gbtn->width-64*bs_units_per_px/16) >> 8);

    int mouse_x, mouse_y;
    mouse_x = GetMouseX();
    if ((mouse_x >= (slider_pos_x-11*bs_units_per_px/16)) && (mouse_x <= (slider_pos_x+11*bs_units_per_px/16)))
    {
        mouse_y = GetMouseY();
        if ((mouse_y >= gbtn->pos_y) && (mouse_y <= (gbtn->pos_y+gbtn->height))) {
            return true;
        }
    }
    return false;
}

void clear_radio_buttons(struct GuiMenu *gmnu)
{
    struct GuiButton *gbtn;
    int i;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        if (gbtn->button_type == LbBtnT_RadioBtn)
        {
            if (gmnu->index == gbtn->menu_idx)
                gbtn->leftclick_flag = 0;
        }
    }
}

void update_radio_button_data(struct GuiMenu *gmnu)
{
    struct GuiButton *gbtn;
    unsigned char *rbstate;
    int i;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        rbstate = (unsigned char *)gbtn->content;
        if ((rbstate != NULL) && (gbtn->menu_idx == gmnu->index))
        {
          if (gbtn->button_type == LbBtnT_RadioBtn)
          {
              if (gbtn->leftclick_flag)
                *rbstate = 1;
              else
                *rbstate = 0;
          }
        }
    }
}

TbBool gui_button_click_inputs(int gmbtn_idx)
{
    TbBool result;
    struct GuiButton *gbtn;
    if (gmbtn_idx < 0)
      return false;
    result = false;
    gbtn = &active_buttons[gmbtn_idx];
    Gf_Btn_Callback callback;
    if (lbDisplay.MLeftButton)
    {
        SYNCDBG(8,"Left down for button %d",(int)gmbtn_idx);
        result = true;
        callback = gbtn->callback_click;
        if ((callback != NULL) || ((gbtn->flags & LbBtnFlag_CloseCurrentMenu)) ||
           (gbtn->parent_menu != NULL) || (gbtn->button_type == LbBtnT_RadioBtn))
        {
            if (gbtn->flags & LbBtnFlag_Enabled)
            {
                SYNCDBG(18,"Left down action for type %d",(int)gbtn->button_type);
                switch (gbtn->button_type)
                {
                case LbBtnT_HoldableBtn:
                  if ((gbtn->leftclick_flag > 5) && (callback != NULL)) {
                      callback(gbtn);
                  } else {
                      gbtn->leftclick_flag++;
                  }
                  break;
                case LbBtnT_Unknown6:
                  if (callback != NULL) {
                      callback(gbtn);
                  }
                  break;
                }
            }
        }
    } else
    if (lbDisplay.MRightButton)
    {
        SYNCDBG(8,"Right down for button %d",(int)gmbtn_idx);
        result = true;
        callback = gbtn->callback_rightclick;
        if ((callback != NULL) && ((gbtn->flags & LbBtnFlag_Enabled) != 0))
        {
            SYNCDBG(18,"Right down action for type %d",(int)gbtn->button_type);
            switch (gbtn->button_type)
            {
            case LbBtnT_HoldableBtn:
                if ((gbtn->rightclick_flag > 5) && (callback != NULL))
                {
                    callback(gbtn);
                }
                else
                {
                    gbtn->rightclick_flag++;
                }
                break;
            case LbBtnT_Unknown6:
                if (callback != NULL) {
                    callback(gbtn);
                }
                break;
            }
        }
    }
    if ( left_button_clicked )
    {
        SYNCDBG(8,"Left click for button %d",(int)gmbtn_idx);
        result = true;
        if (game.flash_button_index != 0)
        {
          if (gbtn->designation_id == game.flash_button_index)
            game.flash_button_index = 0;
        }
        callback = gbtn->callback_click;
        if ((callback != NULL) || ((gbtn->flags & LbBtnFlag_CloseCurrentMenu) != 0) ||
           (gbtn->parent_menu != NULL) || (gbtn->button_type == LbBtnT_RadioBtn))
        {
          left_button_clicked = 0;
          gui_last_left_button_pressed_id = gbtn->designation_id;
          do_button_click_actions(gbtn, &gbtn->leftclick_flag, callback);
        }
    } else
    if ( right_button_clicked )
    {
        SYNCDBG(8,"Right click for button %d",(int)gmbtn_idx);
        result = true;
        if (game.flash_button_index != 0)
        {
          if (gbtn->designation_id == game.flash_button_index)
            game.flash_button_index = 0;
        }
        callback = gbtn->callback_rightclick;
        if ((callback != NULL))
        {
          right_button_clicked = 0;
          gui_last_right_button_pressed_id = gbtn->designation_id;
          do_button_click_actions(gbtn, &gbtn->rightclick_flag, callback);
        }
    }
    return result;
}

/**
 * Returns index of an unused button slot.
 * @return
 */
int guibutton_get_unused_slot(void)
{
    struct GuiButton *gbtn;
    int i;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        if ((gbtn->flags & LbBtnFlag_Created) == 0)
        {
            return i;
        }
    }
    return -1;
}

void init_slider_bars(struct GuiMenu *gmnu)
{
    struct GuiButton *gbtn;
    long sldpos;
    int i;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        if ((gbtn->content) && (gbtn->menu_idx == gmnu->index))
        {
          if (gbtn->button_type == LbBtnT_HorizSlider)
          {
              sldpos = *(long *)gbtn->content;
              if (sldpos < 0)
                sldpos = 0;
              else
                  if (sldpos > gbtn->max_value)
                  sldpos = gbtn->max_value;
              gbtn->slider_value = (sldpos * SLIDER_MAXVALUE) / (gbtn->max_value + 1);
          }
        }
    }
}

void init_menu_buttons(struct GuiMenu *gmnu)
{
    struct GuiButton *gbtn;
    Gf_Btn_Callback callback;
    int i;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
      gbtn = &active_buttons[i];
      callback = gbtn->callback_maintain;
      if ((callback != NULL) && (gbtn->menu_idx == gmnu->index))
        callback(gbtn);
    }
}

void kill_button(struct GuiButton *gbtn)
{
    if (gbtn != NULL) {
        gbtn->flags &= ~LbBtnFlag_Created;
    }
}

void kill_button_area_input(void)
{
  if (input_button != NULL)
    strcpy((char *)input_button->content, backup_input_field);
  input_button = NULL;
}

void setup_radio_buttons(struct GuiMenu *gmnu)
{
    struct GuiButton *gbtn;
    int i;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        if ((gbtn->content) && (gmnu->index == gbtn->menu_idx))
        {
            if (gbtn->button_type == LbBtnT_RadioBtn)
            {
                if ( *(unsigned char *)gbtn->content )
                  gbtn->leftclick_flag = 1;
                else
                  gbtn->leftclick_flag = 0;
            }
        }
    }
}

void frontend_copy_mnu_background(struct GuiMenu *gmnu)
{
    SYNCDBG(9,"Starting");
    draw_frontmenu_background(gmnu->pos_x, gmnu->pos_y, gmnu->width, gmnu->height);
}

void frontend_copy_background(void)
{
    draw_frontmenu_background(0,0,POS_AUTO,POS_AUTO);
}

void gui_round_glass_background(struct GuiMenu *gmnu)
{
    SYNCDBG(19,"Starting");
    int fade_h;
    int i;
    fade_h = 0;
    if (game.time_delta < 12)
    {
        if (gmnu->visibility == Visibility_Shown)
        {
            gmnu->fade_time = 1;
            gmnu->visibility = Visibility_Fading;
        }
    }
    else
    {
        i = gmnu->menu_template->fade_time;
        if (i <= 0)
        {
            gmnu->visibility = Visibility_Fading;
        } else {
            fade_h = ((int)MyScreenHeight - (int)gmnu->pos_y) / i;
            if (fade_h < 0)
                fade_h = 0;
        }
    }
    long px,py;
    switch (gmnu->visibility)
    {
    case Visibility_Hidden:
        px = gmnu->pos_x;
        py = fade_h * (gmnu->menu_template->fade_time - gmnu->fade_time) + gmnu->pos_y;
        draw_round_slab64k(px, py, units_per_pixel, gmnu->width, gmnu->height);
        break;
    case Visibility_Shown:
        px = gmnu->pos_x;
        py = MyScreenHeight - fade_h * (gmnu->menu_template->fade_time - gmnu->fade_time);
        draw_round_slab64k(px, py, units_per_pixel, gmnu->width, gmnu->height);
        break;
    default:
        px = gmnu->pos_x;
        py = gmnu->pos_y;
        draw_round_slab64k(px, py, units_per_pixel, gmnu->width, gmnu->height);
        break;
    }
}

void gui_pretty_background(struct GuiMenu *gmnu)
{
    SYNCDBG(9,"Starting");
    int fade_w,fade_h;
    int i;
    fade_w = 0;
    fade_h = 0;
    if (game.time_delta < 12)
    {
        if (gmnu->visibility == Visibility_Shown)
      {
          gmnu->fade_time = 1;
          gmnu->visibility = Visibility_Fading;
      }
    } else
    {
        i = gmnu->menu_template->fade_time;
        if (i <= 0)
        {
            gmnu->visibility = Visibility_Fading;
        } else {
            fade_w = (gmnu->width - 86*units_per_pixel/16) / i;
            if (fade_w < 0)
                fade_w = 0;
            fade_h = (gmnu->height - 64*units_per_pixel/16) / i;
            if (fade_h < 0)
                fade_h = 0;
        }
    }
    long px,py;
    int width, height;
    switch (gmnu->visibility)
    {
    case Visibility_Shown:
        width = fade_w * (gmnu->menu_template->fade_time - gmnu->fade_time) + 86*units_per_pixel/16;
        height = fade_h * (gmnu->menu_template->fade_time - gmnu->fade_time) + 64*units_per_pixel/16;
        px = gmnu->pos_x + gmnu->width/2 - width/2;
        py = gmnu->pos_y + gmnu->height/2 - height/2;
        draw_ornate_slab_outline64k(px, py, units_per_pixel, width, height);
        break;
    case Visibility_Hidden:
        width = gmnu->width;
        height = gmnu->height;
        px = gmnu->pos_x + gmnu->width/2 - width/2;
        py = gmnu->pos_y + gmnu->height/2 - (gmnu->height - fade_h)/2;
        draw_ornate_slab_outline64k(px, py, units_per_pixel, width, height);
        break;
    default:
        draw_ornate_slab64k(gmnu->pos_x, gmnu->pos_y, units_per_pixel, gmnu->width, gmnu->height);
        break;
    }
}

void gui_area_new_normal_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int i;
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, gbtn->sprite_idx+1, 100);
    if ((gbtn->flags & LbBtnFlag_Enabled) != 0)
    {
        i = 0;
        if ((!gbtn->leftclick_flag) && (!gbtn->rightclick_flag))
            i = 1;
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx+i);
    } else
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx+1, 12);
    }
    SYNCDBG(12,"Finished");
}

void gui_area_new_vertical_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int i;
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, gbtn->sprite_idx+1, 100);
    if ((gbtn->flags & LbBtnFlag_Enabled) != 0)
    {
        i = 0;
        if ((!gbtn->leftclick_flag) && (!gbtn->rightclick_flag))
            i = 1;
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx+i);
    } else
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx+1, 12);
    }
    SYNCDBG(12,"Finished");
}

void gui_draw_tab(struct GuiButton *gbtn)
{
    int i;
    i = gbtn->sprite_idx;
    if (gbtn->button_type == LbBtnT_ToggleBtn) {
        ERRORLOG("Cycle button cannot use this draw function!");
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, i, 100);
    if ((!gbtn->leftclick_flag) && (!gbtn->rightclick_flag))
        i++;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, i);
}

void gui_area_new_null_button(struct GuiButton *gbtn)
{
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, gbtn->sprite_idx, 128);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, gbtn->sprite_idx);
}

void gui_area_compsetting_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int spr_idx;
    spr_idx = gbtn->sprite_idx;
    if (gbtn->button_type == LbBtnT_ToggleBtn)
    {
        if (gbtn->content != NULL) {
            spr_idx += *(unsigned char *)gbtn->content;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->max_value == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, spr_idx, 100);
    if (!(gbtn->flags & LbBtnFlag_Enabled))
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->leftclick_flag) || (gbtn->rightclick_flag))
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
    }
    SYNCDBG(12,"Finished");
}

void gui_area_creatrmodel_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int spr_idx;
    spr_idx = gbtn->sprite_idx;
    if (gbtn->button_type == LbBtnT_ToggleBtn)
    {
        if (gbtn->content != NULL) {
            spr_idx += *(unsigned char *)gbtn->content;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->max_value == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, spr_idx, 138);
    if (!(gbtn->flags & LbBtnFlag_Enabled))
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->leftclick_flag) || (gbtn->rightclick_flag))
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
    }
    SYNCDBG(12,"Finished");
}

void gui_area_new_no_anim_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int spr_idx;
    spr_idx = gbtn->sprite_idx;
    if (gbtn->button_type == LbBtnT_ToggleBtn)
    {
        if (gbtn->content != NULL) {
            spr_idx += *(unsigned char *)gbtn->content;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->max_value == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, spr_idx, 128);
    if (!(gbtn->flags & LbBtnFlag_Enabled))
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->leftclick_flag) || (gbtn->rightclick_flag))
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 44);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
    }
    SYNCDBG(12,"Finished");
}

void gui_area_no_anim_button(struct GuiButton *gbtn)
{
    int spr_idx;
    spr_idx = gbtn->sprite_idx;
    if (gbtn->button_type == LbBtnT_ToggleBtn)
    {
        unsigned char *ctptr;
        ctptr = (unsigned char *)gbtn->content;
        if (ctptr != NULL) {
            spr_idx += *ctptr;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->max_value == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, spr_idx, 100);
    if (!(gbtn->flags & LbBtnFlag_Enabled))
    {
        draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->leftclick_flag) || (gbtn->rightclick_flag))
    {
        draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, spr_idx, 44);
    } else
    {
        draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, spr_idx);
    }
}

void gui_area_normal_button(struct GuiButton *gbtn)
{
    int spr_idx;
    spr_idx = gbtn->sprite_idx;
    if (gbtn->button_type == LbBtnT_ToggleBtn)
    {
        ERRORLOG("Cycle button cannot have a normal button draw function!");
    }
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_width_units_per_px(gbtn, spr_idx, 114);
    if ((gbtn->flags & LbBtnFlag_Enabled) != 0)
    {
        if ( (gbtn->leftclick_flag != 0) || (gbtn->rightclick_flag != 0) )
            spr_idx++;
        draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, spr_idx);
    } else
    {
        draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, spr_idx, 12);
    }
}

void frontend_over_button(struct GuiButton *gbtn)
{
    int i;

    if (gbtn->button_type == LbBtnT_EditBox)
      i = gbtn->btype_value;
    else
      i = (long)gbtn->content;
    if (old_mouse_over_button != i)
      frontend_mouse_over_button_start_time = timeGetTime();
    frontend_mouse_over_button = i;
}

void frontend_draw_button(struct GuiButton *gbtn, unsigned short btntype, const char *text, unsigned int drw_flags)
{
    static const long large_button_sprite_anims[] =
        { 2, 5, 8, 11, 14, 11, 8, 5, };
    unsigned int febtn_idx;
    unsigned int spridx;
    int fntidx;
    long x,y;
    int h;
    SYNCDBG(9,"Drawing type %d, text \"%s\"",(int)btntype,text);
    febtn_idx = (unsigned int)gbtn->content;
    if (!(gbtn->flags & LbBtnFlag_Enabled))
    {
        fntidx = 3;
        spridx = 14;
    } else
    if ((febtn_idx > 0) && (frontend_mouse_over_button == febtn_idx))
    {
        fntidx = 2;
        spridx = large_button_sprite_anims[((timeGetTime()-frontend_mouse_over_button_start_time)/100) & 7];
    } else
    {
        fntidx = frontend_button_caption_font(gbtn, 0);
        spridx = 14;
    }
    struct TbSprite *spr;
    // Detect scaling factor
    int units_per_px;
    units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, 14, 100);
    x = gbtn->scr_pos_x;
    y = gbtn->scr_pos_y;
    switch (btntype)
    {
     case 1:
         spr = &frontend_sprite[spridx];
         LbSpriteDrawResized(x, y, units_per_px, spr);
         x += spr->SWidth * units_per_px / 16;
         spr = &frontend_sprite[spridx+1];
         LbSpriteDrawResized(x, y, units_per_px, spr);
         x += spr->SWidth * units_per_px / 16;
         break;
    case 2:
        spr = &frontend_sprite[spridx];
        LbSpriteDrawResized(x, y, units_per_px, spr);
        x += spr->SWidth * units_per_px / 16;
        spr = &frontend_sprite[spridx+1];
        LbSpriteDrawResized(x, y, units_per_px, spr);
        x += spr->SWidth * units_per_px / 16;
        LbSpriteDrawResized(x, y, units_per_px, spr);
        x += spr->SWidth * units_per_px / 16;
        break;
    default:
        spr = &frontend_sprite[spridx];
        LbSpriteDrawResized(x, y, units_per_px, spr);
        x += spr->SWidth * units_per_px / 16;
        break;
    }
    spr = &frontend_sprite[spridx+2];
    LbSpriteDrawResized(x, y, units_per_px, spr);
    if (text != NULL)
    {
        lbDisplay.DrawFlags = drw_flags;
        LbTextSetFont(frontend_font[fntidx]);
        spr = &frontend_sprite[spridx];
        h = LbTextHeight(text) * units_per_px / 16;
        x = gbtn->scr_pos_x + ((40*units_per_px/16) >> 1);
        y = gbtn->scr_pos_y + ((spr->SHeight*units_per_px/16 - h) >> 1);
        LbTextSetWindow(x, y, gbtn->width-40*units_per_px/16, h);
        LbTextDrawResized(0, 0, units_per_px, text);
    }
}

void frontend_draw_large_menu_button(struct GuiButton *gbtn)
{
    const char *text;
    text = frontend_button_caption_text(gbtn);
    frontend_draw_button(gbtn, 1, text, Lb_TEXT_HALIGN_CENTER);
}

void frontend_draw_vlarge_menu_button(struct GuiButton *gbtn)
{
    const char *text;
    text = frontend_button_caption_text(gbtn);
    frontend_draw_button(gbtn, 2, text, Lb_TEXT_HALIGN_CENTER);
}

void frontend_draw_scroll_box_tab(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    long pos_x,pos_y;
    int fs_units_per_px;
    fs_units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, 75, 100);
    spr = &frontend_sprite[75];
    pos_x = gbtn->scr_pos_x;
    // Since this tab is attachable from top, it is important to keep bottom position without variation
    pos_y = gbtn->scr_pos_y + gbtn->height - spr->SHeight * fs_units_per_px / 16;
    spr = &frontend_sprite[74];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[75];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[75];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
    pos_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[76];
    LbSpriteDrawResized(pos_x, pos_y, fs_units_per_px, spr);
}

void frontend_draw_scroll_box(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    long pos_x,pos_y;
    long height_lines,draw_scrollbar;
    long spr_idx,secspr_idx;
    long i,delta;
    pos_y = gbtn->scr_pos_y;
    switch ( (long)gbtn->content )
    {
      case 24:
        height_lines = 2;
        draw_scrollbar = 1;
        break;
      case 25:
        height_lines = 3;
        draw_scrollbar = 1;
        break;
      case 26:
        height_lines = 7;
        draw_scrollbar = 1;
        break;
      case 89:
        height_lines = 3;
        draw_scrollbar = 0;
        break;
      case 90:
        height_lines = 4;
        draw_scrollbar = 0;
        break;
      case 91:
        height_lines = 4;
        draw_scrollbar = 1;
        break;
      case 94:
        height_lines = 10;
        draw_scrollbar = 1;
        break;
      default:
        height_lines = 0;
        draw_scrollbar = 0;
        break;
    }
    // Detect scaling factor is quite complicated for this item
    int units_per_px;
    {
        int orig_size;
        orig_size = 0;
        spr = &frontend_sprite[33];
        for (i=0; i < 6; i++)
        {
            orig_size += spr->SWidth;
            spr++;
        }
        units_per_px = (gbtn->width * 16 + orig_size/2) / orig_size;
    }
    // Draw top border
    spr = &frontend_sprite[25];
    pos_x = gbtn->scr_pos_x;
    for (i=0; i < 6; i++)
    {
        LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
        spr++;
    }
    if ( draw_scrollbar )
    {
        pos_x = gbtn->scr_pos_x + gbtn->width;
        draw_frontend_sprite_left(pos_x, pos_y - units_per_px/16, units_per_px, 31);
    }
    // Draw inside
    spr = &frontend_sprite[25];
    pos_y += spr->SHeight * units_per_px / 16;
    for (; height_lines > 0; height_lines -= delta )
    {
      if (height_lines < 3)
          spr_idx = 33;
      else
          spr_idx = 40;
      spr = &frontend_sprite[spr_idx];
      pos_x = gbtn->scr_pos_x;
      for (i=0; i < 6; i++)
      {
          LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
          pos_x += spr->SWidth * units_per_px / 16;
          spr++;
      }
      if ( draw_scrollbar )
      {
        if ( height_lines < 3 )
            secspr_idx = 39;
        else
            secspr_idx = 46;
        pos_x = gbtn->scr_pos_x + gbtn->width;
        draw_frontend_sprite_left(pos_x, pos_y, units_per_px, secspr_idx);
      }
      spr = &frontend_sprite[spr_idx];
      pos_y += spr->SHeight * units_per_px / 16;
      if (height_lines < 3)
          delta = 1;
      else
          delta = 3;
    }
    // Draw bottom border
    spr = &frontend_sprite[47];
    pos_x = gbtn->scr_pos_x;
    for (i=0; i < 6; i++)
    {
        LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
        spr++;
    }
    if ( draw_scrollbar )
    {
        pos_x = gbtn->scr_pos_x + gbtn->width;
        draw_frontend_sprite_left(pos_x, pos_y, units_per_px, 53);
    }
}

void frontend_draw_slider_button(struct GuiButton *gbtn)
{
    long spr_idx,btn_id;
    if ((gbtn->flags & LbBtnFlag_Enabled) != 0)
    {
        btn_id = (long)gbtn->content;
        if ( (btn_id != 0) && (frontend_mouse_over_button == btn_id) )
        {
            if ( (btn_id == 17) || (btn_id == 36) || (btn_id == 38) ) {
                spr_idx = 32;
            } else {
                spr_idx = 54;
            }
        } else
        {
            if ( (btn_id == 17) || (btn_id == 36) || (btn_id == 38) ) {
                spr_idx = 31;
            } else {
                spr_idx = 53;
            }
        }
    } else
    {
      spr_idx = 0;
    }
    if (spr_idx > 0)
    {
        // Detect scaling factor
        int units_per_px;
        units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, spr_idx, 100);
        draw_frontend_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, units_per_px, spr_idx);
    }
}

void gui_area_null(struct GuiButton *gbtn)
{
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, gbtn->sprite_idx, 100);
    if ((gbtn->flags & LbBtnFlag_Enabled) != 0)
    {
        draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->sprite_idx);
    } else
    {
        draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->sprite_idx);
    }
}

void reset_scroll_window(struct GuiMenu *gmnu)
{
    game.evntbox_scroll_window.start_y = 0;
    game.evntbox_scroll_window.action = 0;
    game.evntbox_scroll_window.text_height = 0;
    game.evntbox_scroll_window.window_height = 0;
}

void gui_set_menu_mode(struct GuiButton *gbtn)
{
    set_menu_mode(gbtn->btype_value);
}

void gui_area_flash_cycle_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int spr_idx;
    spr_idx = gbtn->sprite_idx;
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, spr_idx, 113);
    if ((gbtn->flags & LbBtnFlag_Enabled) != 0)
    {
        if ((!gbtn->leftclick_flag) && (!gbtn->rightclick_flag))
        {
            // If function is active, the button should blink
            unsigned char *ctptr;
            ctptr = (unsigned char *)gbtn->content;
            if ((ctptr != NULL) && (*ctptr > 0))
            {
                if (game.play_gameturn & 1) {
                    spr_idx += 2;
                }
            }
        }
        if ((!gbtn->leftclick_flag) && (!gbtn->rightclick_flag)) {
            spr_idx++;
        }
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx);
    } else
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    }
    SYNCDBG(12,"Finished");
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
