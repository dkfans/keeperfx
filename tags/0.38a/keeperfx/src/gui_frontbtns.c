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


#include "keeperfx.hpp"

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
void gui_clear_buttons_not_over_mouse(int gmbtn_idx)
{
  struct GuiButton *gbtn;
  int gidx;
  for (gidx=0;gidx<ACTIVE_BUTTONS_COUNT;gidx++)
  {
    gbtn = &active_buttons[gidx];
    if (gbtn->field_0 & 0x01)
      if ( ((gmbtn_idx == -1) || (gmbtn_idx != gidx)) &&
           (gbtn->gbtype != Lb_RADIOBTN) && (gbtn != input_button) )
      {
        set_flag_byte(&gbtn->field_0,0x10,false);
        gbtn->field_1 = 0;
        gbtn->field_2 = 0;
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
  if ((gbtn->field_1) && (left_button_released))
  {
    callback = gbtn->click_event;
    if ((callback != NULL) || ((gbtn->field_0 & 0x02) != 0) ||
        (gbtn->field_2F != 0) || (gbtn->gbtype == Lb_RADIOBTN))
    {
      left_button_released = 0;
      do_button_release_actions(gbtn, &gbtn->field_1, callback);
    }
    return true;
  }
  if ((gbtn->field_2) && (right_button_released))
  {
    callback = gbtn->rclick_event;
    if (callback != NULL)
    {
      right_button_released = 0;
      do_button_release_actions(gbtn, &gbtn->field_2, callback);
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
  gbtn->field_1 = 1;
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
  *gbtn->field_33 = (gbtn->slide_val) * (((long)gbtn->field_2D)+1) >> 8;
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
      result = true;
      callback = gbtn->click_event;
      if ((callback != NULL) || (((gbtn->field_0 & 2)!=0) ||
         (gbtn->field_2F != 0) || (gbtn->gbtype == Lb_RADIOBTN)))
        if ((gbtn->field_0 & 0x08) != 0)
        {
          switch (gbtn->gbtype)
          {
          case 1:
            if ((gbtn->field_1 > 5) && (callback != NULL))
              callback(gbtn);
            else
              gbtn->field_1++;
            break;
          case 6:
            if (callback != NULL)
              callback(gbtn);
            break;
          }
        }
  } else
  if (lbDisplay.MRightButton)
  {
      result = true;
      callback = gbtn->rclick_event;
      if ((callback != NULL) && ((gbtn->field_0 & 8)!=0))
      {
        switch (gbtn->gbtype)
        {
        case 1:
          if ( (gbtn->field_2>5) && (callback!=NULL) )
            callback(gbtn);
          else
            gbtn->field_2++;
          break;
        case 6:
          if (callback!=NULL)
            callback(gbtn);
          break;
        }
      }
  }
  if ( left_button_clicked )
  {
      result = true;
      if (game.flash_button_index != 0)
      {
        if (gbtn->id_num == game.flash_button_index)
          game.flash_button_index = 0;
      }
      callback = gbtn->click_event;
      if ((callback != NULL) || (gbtn->field_0 & 0x02) ||
         (gbtn->field_2F) || (gbtn->gbtype == Lb_RADIOBTN))
      {
        left_button_clicked = 0;
        gui_last_left_button_pressed_id = gbtn->id_num;
        do_button_click_actions(gbtn, &gbtn->field_1, callback);
      }
  } else
  if ( right_button_clicked )
  {
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
        do_button_click_actions(gbtn, &gbtn->field_2, callback);
      }
  }
  return result;
}

void kill_button(struct GuiButton *gbtn)
{
  if (gbtn != NULL)
    set_flag_byte(&gbtn->field_0, 0x01, false);
}

void kill_button_area_input(void)
{
  if (input_button != NULL)
    strcpy((char *)input_button->field_33, backup_input_field);
  input_button = NULL;
}

void setup_radio_buttons(struct GuiMenu *gmnu)
{
  struct GuiButton *gbtn;
  int i;
  for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
  {
    gbtn = &active_buttons[i];
    if ((gbtn->field_33) && (gmnu->field_14 == gbtn->gmenu_idx))
    {
      if (gbtn->gbtype == Lb_RADIOBTN)
      {
        if ( *(unsigned char *)gbtn->field_33 )
          gbtn->field_1 = 1;
        else
          gbtn->field_1 = 0;
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
  _DK_gui_round_glass_background(gmnu);
}

void gui_pretty_background(struct GuiMenu *gmnu)
{
  SYNCDBG(9,"Starting");
  _DK_gui_pretty_background(gmnu);
}

void gui_area_new_normal_button(struct GuiButton *gbtn)
{
  SYNCDBG(10,"Starting");
  _DK_gui_area_new_normal_button(gbtn);
  SYNCDBG(12,"Finished");
}

void gui_draw_tab(struct GuiButton *gbtn)
{
  if (gbtn->gbtype == Lb_CYCLEBTN)
    ERRORLOG("Cycle button cannot use this draw function!");
  if ((gbtn->field_1) || (gbtn->field_2))
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29);
  else
    draw_gui_panel_sprite_left(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->field_29+1);
}

void gui_area_new_null_button(struct GuiButton *gbtn)
{
  _DK_gui_area_new_null_button(gbtn);
}

void gui_area_new_no_anim_button(struct GuiButton *gbtn)
{
  SYNCDBG(10,"Starting");
  _DK_gui_area_new_no_anim_button(gbtn);
  SYNCDBG(12,"Finished");
}

void gui_area_no_anim_button(struct GuiButton *gbtn)
{
  _DK_gui_area_no_anim_button(gbtn);
}

void gui_area_normal_button(struct GuiButton *gbtn)
{
  _DK_gui_area_normal_button(gbtn);
}

void frontend_draw_button(struct GuiButton *gbtn, unsigned short btntype, const char *text, unsigned int drw_flags)
{
  static const long large_button_sprite_anims[] =
      { 2, 5, 8, 11, 14, 11, 8, 5, };
  unsigned int fbinfo_idx;
  unsigned int spridx;
  int fntidx;
  long x,y;
  int h;
  SYNCDBG(9,"Drawing type %d, text \"%s\"",(int)btntype,text);
  fbinfo_idx = (unsigned int)gbtn->field_33;
  if ((gbtn->field_0 & 0x08) == 0)
  {
    fntidx = 3;
    spridx = 14;
  } else
  if ((fbinfo_idx>0) && (frontend_mouse_over_button == fbinfo_idx))
  {
    fntidx = 2;
    spridx = large_button_sprite_anims[((timeGetTime()-frontend_mouse_over_button_start_time)/100) & 7];
  } else
  {
    fntidx = frontend_button_info[fbinfo_idx%FRONTEND_BUTTON_INFO_COUNT].font_index;
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
  unsigned long btninfo_idx;
  char *text;
  int idx;
  btninfo_idx = (unsigned long)gbtn->field_33;
  if (btninfo_idx < FRONTEND_BUTTON_INFO_COUNT)
    idx = frontend_button_info[btninfo_idx].capstr_idx;
  else
    idx = -1;
  if ((idx >= 0) && (idx < STRINGS_MAX))
    text = gui_strings[idx];
  else
    text = NULL;
  frontend_draw_button(gbtn, 1, text, 0x0100);
}

void frontend_draw_vlarge_menu_button(struct GuiButton *gbtn)
{
  unsigned int btninfo_idx;
  const char *text;
  int idx;
  //_DK_frontend_draw_vlarge_menu_button(gbtn);
  btninfo_idx = (unsigned long)gbtn->field_33;
  if (btninfo_idx < FRONTEND_BUTTON_INFO_COUNT)
    idx = frontend_button_info[btninfo_idx].capstr_idx;
  else
    idx = -1;
  if ((idx >= 0) && (idx < STRINGS_MAX))
    text = gui_strings[idx];
  else
    text = NULL;
  frontend_draw_button(gbtn, 2, text, 0x0100);
}

void frontend_draw_scroll_box_tab(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_scroll_box_tab(gbtn);
}

void frontend_draw_scroll_box(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_scroll_box(gbtn);
}

void frontend_draw_slider_button(struct GuiButton *gbtn)
{
  _DK_frontnet_draw_slider_button(gbtn);
}

void gui_area_null(struct GuiButton *gbtn)
{
  if ((gbtn->field_0 & 0x08) != 0)
  {
    LbSpriteDraw(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
      &button_sprite[gbtn->field_29]);
  } else
  {
    LbSpriteDraw(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
      &button_sprite[gbtn->field_29]);
  }
}

void reset_scroll_window(struct GuiMenu *gmnu)
{
  _DK_reset_scroll_window(gmnu);
}

void gui_set_menu_mode(struct GuiButton *gbtn)
{
  set_menu_mode(gbtn->field_1B);
}

void gui_area_flash_cycle_button(struct GuiButton *gbtn)
{
  _DK_gui_area_flash_cycle_button(gbtn);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
