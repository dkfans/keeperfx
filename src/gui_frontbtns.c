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
#include "front_input.h"

// needed for timeGetTime() -- should be later removed
#if defined(_WIN32)
//instead of #include <windows.h>
#include <stdarg.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#endif

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
      if (gbtn->flags & LbBtnF_Active)
        if ( ((gmbtn_mouseover_idx == -1) || (gmbtn_mouseover_idx != gidx)) &&
             (gbtn->gbtype != LbBtnT_RadioBtn) && (gbtn != input_button) )
        {
          gbtn->flags &= ~LbBtnF_Unknown10;
          gbtn->gbactn_1 = 0;
          gbtn->gbactn_2 = 0;
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
        gmnu = &active_menus[(unsigned)gbtn->gmenu_idx];
        if (((gbtn->flags & LbBtnF_Active) != 0) && (gmnu->is_turned_on != 0) && (gbtn->id_num == gmbtn_idx))
        {
            if ((gbtn->click_event != NULL) || ((gbtn->flags & LbBtnF_Unknown02) != 0) || (gbtn->parent_menu != NULL) || (gbtn->gbtype == LbBtnT_RadioBtn)) {
                do_button_press_actions(gbtn, &gbtn->gbactn_1, gbtn->click_event);
            }
            if ((gbtn->click_event != NULL) || ((gbtn->flags & LbBtnF_Unknown02) != 0) || (gbtn->parent_menu != NULL) || (gbtn->gbtype == LbBtnT_RadioBtn)) {
                do_button_click_actions(gbtn, &gbtn->gbactn_1, gbtn->click_event);
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
    if ((gbtn->gbactn_1) && (left_button_released))
    {
        callback = gbtn->click_event;
        if ((callback != NULL) || ((gbtn->flags & LbBtnF_Unknown02) != 0) ||
            (gbtn->parent_menu != NULL) || (gbtn->gbtype == LbBtnT_RadioBtn))
        {
            left_button_released = 0;
            do_button_release_actions(gbtn, &gbtn->gbactn_1, callback);
        }
        return true;
    }
    if ((gbtn->gbactn_2) && (right_button_released))
    {
        callback = gbtn->rclick_event;
        if (callback != NULL)
        {
          right_button_released = 0;
          do_button_release_actions(gbtn, &gbtn->gbactn_2, callback);
        }
        return true;
    }
    return false;
}

TbBool gui_slider_button_inputs(int gbtn_idx)
{
    Gf_Btn_Callback callback;
    int mouse_x;
    int slide_start;
    int slide_end;
    struct GuiButton *gbtn;
    if (gbtn_idx < 0)
      return false;
    gbtn = &active_buttons[gbtn_idx];
    mouse_x = GetMouseX();
    gbtn->gbactn_1 = 1;
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, 2, 44);
    slide_start = gbtn->pos_x + 32*bs_units_per_px/16;
    slide_end = gbtn->pos_x + gbtn->width - 32*bs_units_per_px/16;
    if (mouse_x < slide_start)
    {
        gbtn->slide_val = 0;
    } else
    if (mouse_x >= slide_end)
    {
        gbtn->slide_val = 255;
    } else
    if (gbtn->width > 64*bs_units_per_px/16)
    {
        gbtn->slide_val = ((mouse_x-slide_start) << 8) / (gbtn->width-64*bs_units_per_px/16);
    } else
    {
        gbtn->slide_val = ((mouse_x-gbtn->pos_x) << 8) / (gbtn->width+1);
    }
    *gbtn->content = (gbtn->slide_val) * (((long)gbtn->field_2D)+1) >> 8;
    callback = gbtn->click_event;
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
    slider_pos_x = gbtn->scr_pos_x + 32*bs_units_per_px/16 + ((gbtn->slide_val)*(gbtn->width-64*bs_units_per_px/16) >> 8);

    int mouse_x;
    int mouse_y;
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
        if (gbtn->gbtype == LbBtnT_RadioBtn)
        {
            if (gmnu->number == gbtn->gmenu_idx)
                gbtn->gbactn_1 = 0;
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
        if ((rbstate != NULL) && (gbtn->gmenu_idx == gmnu->number))
        {
          if (gbtn->gbtype == LbBtnT_RadioBtn)
          {
              if (gbtn->gbactn_1)
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
        callback = gbtn->click_event;
        if ((callback != NULL) || ((gbtn->flags & LbBtnF_Unknown02) != 0) ||
           (gbtn->parent_menu != NULL) || (gbtn->gbtype == LbBtnT_RadioBtn))
        {
            if ((gbtn->flags & LbBtnF_Enabled) != 0)
            {
                SYNCDBG(18,"Left down action for type %d",(int)gbtn->gbtype);
                switch (gbtn->gbtype)
                {
                case LbBtnT_HoldableBtn:
                  if ((gbtn->gbactn_1 > 5) && (callback != NULL)) {
                      callback(gbtn);
                  } else {
                      gbtn->gbactn_1++;
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
        callback = gbtn->rclick_event;
        if ((callback != NULL) && ((gbtn->flags & LbBtnF_Enabled) != 0))
        {
            SYNCDBG(18,"Right down action for type %d",(int)gbtn->gbtype);
            switch (gbtn->gbtype)
            {
            case LbBtnT_HoldableBtn:
              if ((gbtn->gbactn_2 > 5) && (callback != NULL)) {
                  callback(gbtn);
              } else {
                  gbtn->gbactn_2++;
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
          if (gbtn->id_num == game.flash_button_index)
            game.flash_button_index = 0;
        }
        callback = gbtn->click_event;
        if ((callback != NULL) || ((gbtn->flags & LbBtnF_Unknown02) != 0) ||
           (gbtn->parent_menu != NULL) || (gbtn->gbtype == LbBtnT_RadioBtn))
        {
          left_button_clicked = 0;
          gui_last_left_button_pressed_id = gbtn->id_num;
          do_button_click_actions(gbtn, &gbtn->gbactn_1, callback);
        }
    } else
    if ( right_button_clicked )
    {
        SYNCDBG(8,"Right click for button %d",(int)gmbtn_idx);
        result = true;
        if (game.flash_button_index != 0)
        {
          if (gbtn->id_num == game.flash_button_index)
            game.flash_button_index = 0;
        }
        callback = gbtn->rclick_event;
        if ((callback != NULL))
        {
          right_button_clicked = 0;
          gui_last_right_button_pressed_id = gbtn->id_num;
          do_button_click_actions(gbtn, &gbtn->gbactn_2, callback);
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
        if ((gbtn->flags & LbBtnF_Active) == 0) {
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
        if ((gbtn->content) && (gbtn->gmenu_idx == gmnu->number))
        {
          if (gbtn->gbtype == LbBtnT_HorizSlider)
          {
              sldpos = *(long *)gbtn->content;
              if (sldpos < 0)
                sldpos = 0;
              else
              if (sldpos > gbtn->field_2D)
                sldpos = gbtn->field_2D;
              gbtn->slide_val = (sldpos << 8) / (gbtn->field_2D + 1);
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
      callback = gbtn->maintain_call;
      if ((callback != NULL) && (gbtn->gmenu_idx == gmnu->number))
        callback(gbtn);
    }
}

void kill_button(struct GuiButton *gbtn)
{
    if (gbtn != NULL) {
        gbtn->flags &= ~LbBtnF_Active;
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
        if ((gbtn->content) && (gmnu->number == gbtn->gmenu_idx))
        {
            if (gbtn->gbtype == LbBtnT_RadioBtn)
            {
                if ( *(unsigned char *)gbtn->content )
                  gbtn->gbactn_1 = 1;
                else
                  gbtn->gbactn_1 = 0;
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
      if (gmnu->visual_state == 1) {
          gmnu->fade_time = 1;
          gmnu->visual_state = 2;
      }
    } else
    {
        i = gmnu->menu_init->fade_time;
        if (i <= 0)
        {
            gmnu->visual_state = 2;
        } else {
            fade_h = ((int)MyScreenHeight - (int)gmnu->pos_y) / i;
            if (fade_h < 0)
                fade_h = 0;
        }
    }
    long px;
    long py;
    switch (gmnu->visual_state)
    {
    case 3:
        px = gmnu->pos_x;
        py = fade_h * (gmnu->menu_init->fade_time - gmnu->fade_time) + gmnu->pos_y;
        draw_round_slab64k(px, py, units_per_pixel, gmnu->width, gmnu->height);
        break;
    case 1:
        px = gmnu->pos_x;
        py = MyScreenHeight - fade_h * (gmnu->menu_init->fade_time - gmnu->fade_time);
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
    int fade_w;
    int fade_h;
    int i;
    fade_w = 0;
    fade_h = 0;
    if (game.time_delta < 12)
    {
      if (gmnu->visual_state == 1) {
          gmnu->fade_time = 1;
          gmnu->visual_state = 2;
      }
    } else
    {
        i = gmnu->menu_init->fade_time;
        if (i <= 0)
        {
            gmnu->visual_state = 2;
        } else {
            fade_w = (gmnu->width - 86*units_per_pixel/16) / i;
            if (fade_w < 0)
                fade_w = 0;
            fade_h = (gmnu->height - 64*units_per_pixel/16) / i;
            if (fade_h < 0)
                fade_h = 0;
        }
    }
    long px;
    long py;
    int width;
    int height;
    switch (gmnu->visual_state)
    {
    case 1:
        width = fade_w * (gmnu->menu_init->fade_time - gmnu->fade_time) + 86*units_per_pixel/16;
        height = fade_h * (gmnu->menu_init->fade_time - gmnu->fade_time) + 64*units_per_pixel/16;
        px = gmnu->pos_x + gmnu->width/2 - width/2;
        py = gmnu->pos_y + gmnu->height/2 - height/2;
        draw_ornate_slab_outline64k(px, py, units_per_pixel, width, height);
        break;
    case 3:
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
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        i = 0;
        if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2))
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
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        i = 0;
        if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2))
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
    if (gbtn->gbtype == LbBtnT_ToggleBtn) {
        ERRORLOG("Cycle button cannot use this draw function!");
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, i, 100);
    if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2))
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
    if (gbtn->gbtype == LbBtnT_ToggleBtn)
    {
        if (gbtn->content != NULL) {
            spr_idx += *(unsigned char *)gbtn->content;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->field_2D == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, spr_idx, 100);
    if ((gbtn->flags & LbBtnF_Enabled) == 0)
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
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
    if (gbtn->gbtype == LbBtnT_ToggleBtn)
    {
        if (gbtn->content != NULL) {
            spr_idx += *(unsigned char *)gbtn->content;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->field_2D == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, spr_idx, 138);
    if ((gbtn->flags & LbBtnF_Enabled) == 0)
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
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
    if (gbtn->gbtype == LbBtnT_ToggleBtn)
    {
        if (gbtn->content != NULL) {
            spr_idx += *(unsigned char *)gbtn->content;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->field_2D == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_height_units_per_px(gbtn, spr_idx, 128);
    if ((gbtn->flags & LbBtnF_Enabled) == 0)
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, ps_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
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
    if (gbtn->gbtype == LbBtnT_ToggleBtn)
    {
        unsigned char *ctptr;
        ctptr = (unsigned char *)gbtn->content;
        if (ctptr != NULL) {
            spr_idx += *ctptr;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->field_2D == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, spr_idx, 100);
    if ((gbtn->flags & LbBtnF_Enabled) == 0)
    {
        draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, spr_idx, 12);
    } else
    if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
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
    if (gbtn->gbtype == LbBtnT_ToggleBtn)
    {
        ERRORLOG("Cycle button cannot have a normal button draw function!");
    }
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_width_units_per_px(gbtn, spr_idx, 114);
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        if ( (gbtn->gbactn_1 != 0) || (gbtn->gbactn_2 != 0) )
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

    if (gbtn->gbtype == LbBtnT_EditBox)
      i = gbtn->btype_value & LbBFeF_IntValueMask;
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
    long x;
    long y;
    int h;
    SYNCDBG(9,"Drawing type %d, text \"%s\"",(int)btntype,text);
    febtn_idx = (unsigned int)gbtn->content;
    if ((gbtn->flags & LbBtnF_Enabled) == 0)
    {
        fntidx = 3;
        spridx = 14;
    } else
    {
        fntidx = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
        if ((febtn_idx > 0) && (frontend_mouse_over_button == febtn_idx)) {
            spridx = large_button_sprite_anims[((timeGetTime()-frontend_mouse_over_button_start_time)/100) & 7];
        } else {
            spridx = 14;
        }
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
    long pos_x;
    long pos_y;
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
    long pos_x;
    long pos_y;
    long height_lines;
    long draw_scrollbar;
    long spr_idx;
    long secspr_idx;
    long i;
    long delta;
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
    long spr_idx;
    long btn_id;
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
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
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
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
    set_menu_mode(gbtn->btype_value & LbBFeF_IntValueMask);
}

void gui_area_flash_cycle_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int spr_idx;
    spr_idx = gbtn->sprite_idx;
    int ps_units_per_px;
    ps_units_per_px = simple_gui_panel_sprite_width_units_per_px(gbtn, spr_idx, 113);
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2))
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
        if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2)) {
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
