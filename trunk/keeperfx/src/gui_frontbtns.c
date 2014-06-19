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

// needed for timeGetTime() -- should be later removed
#if defined(WIN32)
//instead of #include <windows.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <mmsystem.h>
#endif

#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_gui_pretty_background(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_round_glass_background(struct GuiMenu *gmnu);

DLLIMPORT void _DK_gui_area_new_normal_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_draw_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_new_null_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_new_no_anim_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_no_anim_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_normal_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_over_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_button(struct GuiButton *gbtn, long a2, const char *text, long a4);
DLLIMPORT void _DK_frontend_draw_large_menu_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_vlarge_menu_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_reset_scroll_window(struct GuiMenu *gmnu);
DLLIMPORT void _DK_gui_area_null(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_menu_mode(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_area_flash_cycle_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_scroll_box_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_scroll_box(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontnet_draw_slider_button(struct GuiButton *gbtn);
/******************************************************************************/
void gui_clear_buttons_not_over_mouse(int gmbtn_mouseover_idx)
{
    struct GuiButton *gbtn;
    int gidx;
    for (gidx=0;gidx<ACTIVE_BUTTONS_COUNT;gidx++)
    {
      gbtn = &active_buttons[gidx];
      if (gbtn->flags & LbBtnF_Unknown01)
        if ( ((gmbtn_mouseover_idx == -1) || (gmbtn_mouseover_idx != gidx)) &&
             (gbtn->gbtype != Lb_RADIOBTN) && (gbtn != input_button) )
        {
          gbtn->flags &= ~LbBtnF_Unknown10;
          gbtn->gbactn_1 = 0;
          gbtn->gbactn_2 = 0;
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
            (gbtn->parent_menu != 0) || (gbtn->gbtype == Lb_RADIOBTN))
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
    int slide_start,slide_end;
    struct GuiButton *gbtn;
    if (gbtn_idx < 0)
      return false;
    gbtn = &active_buttons[gbtn_idx];
    mouse_x = GetMouseX();
    gbtn->gbactn_1 = 1;
    slide_start = gbtn->pos_x+32;
    slide_end = gbtn->pos_x+gbtn->width-32;
    if (mouse_x < slide_start)
    {
      gbtn->slide_val = 0;
    } else
    if (mouse_x >= slide_end)
    {
      gbtn->slide_val = 255;
    } else
    if (gbtn->width > 64)
    {
      gbtn->slide_val = ((mouse_x-slide_start) << 8) / (gbtn->width-64);
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
           (gbtn->parent_menu != 0) || (gbtn->gbtype == Lb_RADIOBTN))
        {
            if ((gbtn->flags & LbBtnF_Unknown08) != 0)
            {
                SYNCDBG(18,"Left down action for type %d",(int)gbtn->gbtype);
                switch (gbtn->gbtype)
                {
                case Lb_UNKNBTN1:
                  if ((gbtn->gbactn_1 > 5) && (callback != NULL)) {
                      callback(gbtn);
                  } else {
                      gbtn->gbactn_1++;
                  }
                  break;
                case Lb_UNKNBTN6:
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
        if ((callback != NULL) && ((gbtn->flags & LbBtnF_Unknown08) != 0))
        {
            SYNCDBG(18,"Right down action for type %d",(int)gbtn->gbtype);
            switch (gbtn->gbtype)
            {
            case Lb_UNKNBTN1:
              if ((gbtn->gbactn_2 > 5) && (callback != NULL)) {
                  callback(gbtn);
              } else {
                  gbtn->gbactn_2++;
              }
              break;
            case Lb_UNKNBTN6:
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
           (gbtn->parent_menu != 0) || (gbtn->gbtype == Lb_RADIOBTN))
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
        if ((gbtn->flags & LbBtnF_Unknown01) == 0) {
            return i;
        }
    }
    return -1;
}

void kill_button(struct GuiButton *gbtn)
{
  if (gbtn != NULL)
    set_flag_byte(&gbtn->flags, LbBtnF_Unknown01, false);
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
            if (gbtn->gbtype == Lb_RADIOBTN)
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
  frontend_copy_background_at(gmnu->pos_x,gmnu->pos_y,gmnu->width,gmnu->height);
}

void frontend_copy_background(void)
{
    frontend_copy_background_at(0,0,POS_AUTO,POS_AUTO);
}

void gui_round_glass_background(struct GuiMenu *gmnu)
{
    SYNCDBG(19,"Starting");
    //_DK_gui_round_glass_background(gmnu);
    int fade_h;
    int i;
    fade_h = 0;
    if (game.time_delta < 12)
    {
      if (gmnu->visible == 1) {
          gmnu->fade_time = 1;
          gmnu->visible = 2;
      }
    } else
    {
        i = gmnu->menu_init->fade_time;
        if (i <= 0)
        {
            gmnu->visible = 2;
        } else {
            fade_h = ((int)MyScreenHeight - (int)gmnu->pos_y) / i;
            if (fade_h < 0)
                fade_h = 0;
        }
    }
    long px,py;
    switch (gmnu->visible)
    {
    case 3:
        px = gmnu->pos_x;
        py = fade_h * (gmnu->menu_init->fade_time - gmnu->fade_time) + gmnu->pos_y;
        draw_round_slab64k(px, py, gmnu->width, gmnu->height);
        break;
    case 1:
        px = gmnu->pos_x;
        py = MyScreenHeight - fade_h * (gmnu->menu_init->fade_time - gmnu->fade_time);
        draw_round_slab64k(px, py, gmnu->width, gmnu->height);
        break;
    default:
        px = gmnu->pos_x;
        py = gmnu->pos_y;
        draw_round_slab64k(px, py, gmnu->width, gmnu->height);
        break;
    }
}

void gui_pretty_background(struct GuiMenu *gmnu)
{
  SYNCDBG(9,"Starting");
  //_DK_gui_pretty_background(gmnu);
  int fade_w,fade_h;
  int i;
  fade_w = 0;
  fade_h = 0;
  if (game.time_delta < 12)
  {
    if (gmnu->visible == 1) {
        gmnu->fade_time = 1;
        gmnu->visible = 2;
    }
  } else
  {
      i = gmnu->menu_init->fade_time;
      if (i <= 0)
      {
          gmnu->visible = 2;
      } else {
          fade_w = (gmnu->width - 86) / i;
          if (fade_w < 0)
              fade_w = 0;
          fade_h = (gmnu->height - 64) / i;
          if (fade_h < 0)
              fade_h = 0;
      }
  }
  long px,py;
  int width, height;
  switch (gmnu->visible)
  {
  case 1:
      width = fade_w * (gmnu->menu_init->fade_time - gmnu->fade_time) + 86;
      height = fade_h * (gmnu->menu_init->fade_time - gmnu->fade_time) + 64;
      px = gmnu->pos_x + gmnu->width/2 - width/2;
      py = gmnu->pos_y + gmnu->height/2 - height/2;
      draw_ornate_slab_outline64k(px, py, width, height);
      break;
  case 3:
      width = gmnu->width;
      height = gmnu->height;
      px = gmnu->pos_x + gmnu->width/2 - width/2;
      py = gmnu->pos_y + gmnu->height/2 - (gmnu->height - fade_h)/2;
      draw_ornate_slab_outline64k(px, py, width, height);
      break;
  default:
      draw_ornate_slab64k(gmnu->pos_x, gmnu->pos_y, gmnu->width, gmnu->height);
      break;
  }
}

void gui_area_new_normal_button(struct GuiButton *gbtn)
{
  SYNCDBG(10,"Starting");
  int i;
  //_DK_gui_area_new_normal_button(gbtn);
  if ((gbtn->flags & LbBtnF_Unknown08) != 0)
  {
      i = 0;
      if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2))
          i = 1;
      draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29+i);
  } else
  {
      draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29+1, 12);
  }
  SYNCDBG(12,"Finished");
}

void gui_draw_tab(struct GuiButton *gbtn)
{
    int i;
    i = gbtn->field_29;
    if (gbtn->gbtype == Lb_CYCLEBTN) {
        ERRORLOG("Cycle button cannot use this draw function!");
    }
    if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2))
        i++;
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, i);
}

void gui_area_new_null_button(struct GuiButton *gbtn)
{
    //_DK_gui_area_new_null_button(gbtn);
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
}

void gui_area_new_no_anim_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    //_DK_gui_area_new_no_anim_button(gbtn); return;
    int i;
    i = gbtn->field_29;
    if (gbtn->gbtype == Lb_CYCLEBTN)
    {
        if (gbtn->content != NULL) {
            i += *(unsigned char *)gbtn->content;
        } else {
            ERRORLOG("Cycle button must have a non-null UBYTE Data pointer!");
        }
        if (gbtn->field_2D == 0) {
            ERRORLOG("Cycle button must have a non-zero MaxVal!");
        }
    }
    if ((gbtn->flags & LbBtnF_Unknown08) == 0)
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, i, 12);
    } else
    if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, i, 44);
    } else
    {
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, i);
    }
    SYNCDBG(12,"Finished");
}

void gui_area_no_anim_button(struct GuiButton *gbtn)
{
    int spr_idx;
    //_DK_gui_area_no_anim_button(gbtn); return;
    spr_idx = gbtn->field_29;
    if (gbtn->gbtype == Lb_CYCLEBTN)
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
    if ((gbtn->flags & LbBtnF_Unknown08) != 0)
    {
        if ( (gbtn->gbactn_1 != 0) || (gbtn->gbactn_2 != 0) ) {
            draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, spr_idx, 44);
        } else {
            draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, spr_idx);
        }
    } else
    {
        draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, spr_idx, 12);
    }
}

void gui_area_normal_button(struct GuiButton *gbtn)
{
    int spr_idx;
    //_DK_gui_area_normal_button(gbtn);
    spr_idx = gbtn->field_29;
    if (gbtn->gbtype == Lb_CYCLEBTN)
    {
        ERRORLOG("Cycle button cannot have a normal button draw function!");
    }
    if ((gbtn->flags & LbBtnF_Unknown08) != 0)
    {
        if ( (gbtn->gbactn_1 != 0) || (gbtn->gbactn_2 != 0) )
            spr_idx++;
        draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, spr_idx);
    } else
    {
        draw_button_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, spr_idx, 12);
    }
}

void frontend_over_button(struct GuiButton *gbtn)
{
    //_DK_frontend_over_button(gbtn);
    int i;

    if (gbtn->gbtype == Lb_EDITBTN)
      i = gbtn->field_1B;
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
    if ((gbtn->flags & LbBtnF_Unknown08) == 0)
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
    x = gbtn->scr_pos_x;
    y = gbtn->scr_pos_y;
    switch (btntype)
    {
     case 1:
        LbSpriteDraw(x, y, &frontend_sprite[spridx]);
        x += frontend_sprite[spridx].SWidth;
        LbSpriteDraw(x, y, &frontend_sprite[spridx+1]);
        x += frontend_sprite[spridx+1].SWidth;
        break;
    case 2:
        LbSpriteDraw(x, y, &frontend_sprite[spridx]);
        x += frontend_sprite[spridx].SWidth;
        LbSpriteDraw(x, y, &frontend_sprite[spridx+1]);
        x += frontend_sprite[spridx+1].SWidth;
        LbSpriteDraw(x, y, &frontend_sprite[spridx+1]);
        x += frontend_sprite[spridx+1].SWidth;
        break;
    default:
        LbSpriteDraw(x, y, &frontend_sprite[spridx]);
        x += frontend_sprite[spridx].SWidth;
        break;
    }
    LbSpriteDraw(x, y, &frontend_sprite[spridx+2]);
    if (text != NULL)
    {
        lbDisplay.DrawFlags = drw_flags;
        LbTextSetFont(frontend_font[fntidx]);
        h = LbTextHeight(text);
        x = gbtn->scr_pos_x + ((40) >> 1);
        y = gbtn->scr_pos_y + ((frontend_sprite[spridx].SHeight-h) >> 1);
        LbTextSetWindow(x, y, gbtn->width-40, h);
        LbTextDraw(0, 0, text);
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
    //_DK_frontnet_draw_scroll_box_tab(gbtn);
    pos_x = gbtn->scr_pos_x;
    pos_y = gbtn->scr_pos_y;
    spr = &frontend_sprite[74];
    LbSpriteDraw(pos_x, pos_y, spr);
    pos_x += spr->SWidth;
    spr = &frontend_sprite[75];
    LbSpriteDraw(pos_x, pos_y, spr);
    pos_x += spr->SWidth;
    spr = &frontend_sprite[75];
    LbSpriteDraw(pos_x, pos_y, spr);
    pos_x += spr->SWidth;
    spr = &frontend_sprite[76];
    LbSpriteDraw(pos_x, pos_y, spr);
}

void frontend_draw_scroll_box(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    long pos_x,pos_y;
    long height_lines,draw_endingspr;
    long spr_idx,secspr_idx;
    long i,delta;
    //_DK_frontnet_draw_scroll_box(gbtn);
    pos_y = gbtn->scr_pos_y;
    switch ( (long)gbtn->content )
    {
      case 24:
        height_lines = 2;
        draw_endingspr = 1;
        break;
      case 25:
        height_lines = 3;
        draw_endingspr = 1;
        break;
      case 26:
        height_lines = 7;
        draw_endingspr = 1;
        break;
      case 89:
        height_lines = 3;
        draw_endingspr = 0;
        break;
      case 90:
        height_lines = 4;
        draw_endingspr = 0;
        break;
      case 91:
        height_lines = 4;
        draw_endingspr = 1;
        break;
      case 94:
        height_lines = 10;
        draw_endingspr = 1;
        break;
      default:
        height_lines = 0;
        draw_endingspr = 0;
        break;
    }
    // Draw top border
    spr = &frontend_sprite[25];
    pos_x = gbtn->scr_pos_x;
    for (i=0; i < 6; i++)
    {
        LbSpriteDraw(pos_x, pos_y, spr);
        pos_x += spr->SWidth;
        spr++;
    }
    if ( draw_endingspr )
    {
        spr = &frontend_sprite[31];
        LbSpriteDraw(pos_x, pos_y - 1, spr);
    }
    // Draw inside
    spr = &frontend_sprite[25];
    pos_y += spr->SHeight;
    for (; height_lines > 0; height_lines -= delta )
    {
      if ( height_lines < 3 )
          spr_idx = 33;
      else
          spr_idx = 40;
      spr = &frontend_sprite[spr_idx];
      pos_x = gbtn->scr_pos_x;
      for (i=0; i < 6; i++)
      {
          LbSpriteDraw(pos_x, pos_y, spr);
          pos_x += spr->SWidth;
          spr++;
      }
      if ( draw_endingspr )
      {
        if ( height_lines < 3 )
            secspr_idx = 39;
        else
            secspr_idx = 46;
        spr = &frontend_sprite[secspr_idx];
        LbSpriteDraw(pos_x, pos_y, spr);
      }
      spr = &frontend_sprite[spr_idx];
      pos_y += spr->SHeight;
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
        LbSpriteDraw(pos_x, pos_y, spr);
        pos_x += spr->SWidth;
        spr++;
    }
    if ( draw_endingspr )
    {
        spr = &frontend_sprite[53];
        LbSpriteDraw(pos_x, pos_y, spr);
    }
}

void frontend_draw_slider_button(struct GuiButton *gbtn)
{
    long spr_idx,btn_id;
    //_DK_frontnet_draw_slider_button(gbtn);
    if ((gbtn->flags & LbBtnF_Unknown08) != 0)
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
        LbSpriteDraw(gbtn->scr_pos_x, gbtn->scr_pos_y, &frontend_sprite[spr_idx]);
    }
}

void gui_area_null(struct GuiButton *gbtn)
{
  if ((gbtn->flags & LbBtnF_Unknown08) != 0)
  {
      draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
  } else
  {
      draw_button_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
  }
}

void reset_scroll_window(struct GuiMenu *gmnu)
{
    //_DK_reset_scroll_window(gmnu);
    game.evntbox_scroll_window.start_y = 0;
    game.evntbox_scroll_window.action = 0;
    game.evntbox_scroll_window.text_height = 0;
    game.evntbox_scroll_window.window_height = 0;
}

void gui_set_menu_mode(struct GuiButton *gbtn)
{
    set_menu_mode(gbtn->field_1B);
}

void gui_area_flash_cycle_button(struct GuiButton *gbtn)
{
    SYNCDBG(10,"Starting");
    int i;
    //_DK_gui_area_flash_cycle_button(gbtn); return;
    i = gbtn->field_29;
    if ((gbtn->flags & LbBtnF_Unknown08) != 0)
    {
        if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2))
        {
            // If function is active, the button should blink
            unsigned char *ctptr;
            ctptr = (unsigned char *)gbtn->content;
            if ((ctptr != NULL) && (*ctptr > 0))
            {
                if (game.play_gameturn & 1) {
                    i += 2;
                }
            }
        }
        if ((!gbtn->gbactn_1) && (!gbtn->gbactn_2)) {
            i++;
        }
        draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, i);
    } else
    {
        draw_gui_panel_sprite_rmleft(gbtn->scr_pos_x, gbtn->scr_pos_y, i, 12);
    }
    SYNCDBG(12,"Finished");
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
