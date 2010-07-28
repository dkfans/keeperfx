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
#include "kjm_input.h"
#include "frontend.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

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
  SYNCDBG(7,"Starting");
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
