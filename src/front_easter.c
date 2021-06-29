/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_easter.c
 *     Easter Eggs displaying routines.
 * @par Purpose:
 *     Functions to show and maintain Easter Eggs and related screens.
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
#include "front_easter.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_keybrd.h"
#include "bflib_math.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"
#include "bflib_datetm.h"
#include "bflib_sound.h"
#include "kjm_input.h"
#include "gui_frontbtns.h"
#include "gui_soundmsgs.h"
#include "config_strings.h"
#include "frontend.h"
#include "front_credits.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbBirthday {
    unsigned char day;
    unsigned char month;
    const char *name;
    };

const struct TbBirthday team_birthdays[] = {
    {13, 1,"Mark Healey"},
    {21, 3,"Jonty Barnes"},
    { 3, 5,"Simon Carter"},
    { 5, 5,"Peter Molyneux"},
    {13,11,"Alex Peters"},
    { 1,12,"Dene Carter"},
    {25, 5,"Tomasz Lis"},
    {29,11,"Michael Chateauneuf"},
    {0,0,NULL},
    };

struct KeycodeString eastegg_feckoff_codes = {
    {KC_F,KC_E,KC_C,KC_K,KC_O,KC_F,KC_F,KC_UNASSIGNED}, 7,
};
struct KeycodeString eastegg_jlw_codes = {
    {KC_J,KC_L,KC_W,KC_UNASSIGNED}, 3,
};
struct KeycodeString eastegg_skeksis_codes = {
    {KC_S,KC_K,KC_E,KC_K,KC_S,KC_I,KC_S,KC_UNASSIGNED}, 7,
};
/******************************************************************************/
DLLIMPORT extern unsigned char _DK_eastegg_skeksis_cntr;
#define eastegg_skeksis_cntr _DK_eastegg_skeksis_cntr
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
const char *get_team_birthday(void)
{
  struct TbDate curr_date;
  LbDate(&curr_date);
  for (int i = 0; team_birthdays[i].day != 0; i++)
  {
      if ((team_birthdays[i].day==curr_date.Day) &&
          (team_birthdays[i].month==curr_date.Month))
      {
          return team_birthdays[i].name;
      }
  }
  return NULL;
}

void frontbirthday_draw(void)
{
    frontend_copy_background();
    LbTextSetWindow(70, 70, 500, 340);
    LbTextSetFont(frontstory_font);
    lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
    const char *name=get_team_birthday();
    if ( name != NULL )
    {
        unsigned short line_pos = LbTextLineHeight();
        LbTextDraw(0, 170-line_pos, get_string(GUIStr_HappyBirthday));
        LbTextDraw(0, 170, name);
    } else
    {
        frontend_set_state(FeSt_INTRO);
    }
}

unsigned short input_eastegg_keycodes(unsigned char *counter,short allow,struct KeycodeString const *codes)
{
    if (!allow)
    {
      (*counter) = 0;
      return 0;
    }
    unsigned short result = 0;
    if ((*counter) < codes->length)
    {
        TbKeyCode currkey = codes->keys[(*counter)];
        if (lbKeyOn[currkey])
        {
            (*counter)++;
            result = 1;
            if ((*counter) > 2)
            {
                clear_key_pressed(currkey);
                result = 2;
            }
      }
    }
    if ((*counter) == codes->length)
    {
      if (result > 0)
        result = 3;
      else
        result = 4;
    }
    return result;
}

void input_eastegg(void)
{
    // Maintain the FECKOFF cheat
    short allow = (lbKeyOn[KC_LSHIFT] != 0);
    unsigned short state = input_eastegg_keycodes(&game.eastegg01_cntr, allow, &eastegg_feckoff_codes);
    if ((state == 2) || (state == 3))
      play_non_3d_sample(60);
    // Maintain the JLW cheat
    if ((game.flags_font & FFlg_AlexCheat) != 0)
    {
      allow = (lbKeyOn[KC_LSHIFT]) && (lbKeyOn[KC_RSHIFT]);
      state = input_eastegg_keycodes(&game.eastegg02_cntr,allow,&eastegg_jlw_codes);
      if ((state == 1) || (state == 2)  || (state == 3))
        play_non_3d_sample(159);
    }
    // Maintain the SKEKSIS cheat
    allow = (lbKeyOn[KC_LSHIFT] != 0);
    state = input_eastegg_keycodes(&eastegg_skeksis_cntr,allow,&eastegg_skeksis_codes);
    if (state == 3)
      output_message(SMsg_PantsTooTight, 0, true);
}

/**
 * Displays easter egg messages on screen.
 */
void draw_eastegg(void)
{
  char *text;
  static long px[2] = {0, 0};
  static long py[2] = {0, 0};
  static long vx[2] = {4, 4};
  static long vy[2] = {6, 6};
  long i;
  long k;
  SYNCDBG(5,"Starting");
  int ee_units_per_px = calculate_relative_upp(22, units_per_pixel_best, LbTextLineHeight());
  int width = 640, height = 400, skeksis_x_offset = 120, skeksis_y_offset = 200;
  if (is_ar_wider_than_original(MyScreenWidth, MyScreenHeight))
  {
    width = height * (MyScreenWidth * 10 / MyScreenHeight) / 10;
  }
  else
  {
    height = (width * 10) / (MyScreenWidth * 10 / MyScreenHeight);
  }
  
  skeksis_y_offset = height / 2;
  skeksis_x_offset = (width / 2) - 200;
  LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  if (eastegg_skeksis_cntr >= eastegg_skeksis_codes.length)
  {
      eastegg_skeksis_cntr++;
      LbTextSetFont(winfont);
      text=buf_sprintf("Dene says a big 'Hello' to Goth Buns, Tarts and Barbies");
      lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
      unsigned char pos;
      for (i = 0; i < 30; i += 2)
      {
        pos = game.play_gameturn - i;
        lbDisplay.DrawColour = pos;
        LbTextDrawResized(scale_fixed_DK_value((LbCosL(16*(long)pos) / 512 + skeksis_x_offset) / pixel_size),
          scale_fixed_DK_value((LbSinL(32*(long)pos) / 512 + skeksis_y_offset) / pixel_size), ee_units_per_px, text);
      }
      set_flag_word(&lbDisplay.DrawFlags,Lb_TEXT_ONE_COLOR,false);
      pos=game.play_gameturn;
      LbTextDrawResized(scale_fixed_DK_value((LbCosL(16*(long)pos) / 512 + skeksis_x_offset) / pixel_size),
          scale_fixed_DK_value((LbSinL(32*(long)pos) / 512 + skeksis_y_offset) / pixel_size), ee_units_per_px, text);
      if (eastegg_skeksis_cntr >= 255)
        eastegg_skeksis_cntr = 0;
  }

  if (game.eastegg01_cntr >= eastegg_feckoff_codes.length)
  {
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    LbTextSetFont(winfont);
    i = 0;
    text = buf_sprintf("Simon says Hi to everyone he knows...");
    px[i] += vx[i];
    if (px[i] < 0)
    {
      px[i] = 0;
      vx[i] = -vx[i];
    }
    py[i] += vy[i];
    if (py[i] < 0)
    {
      py[i] = 0;
      vy[i] = -vy[i];
    }
    k = pixel_size*LbTextStringWidth(text);
    if (px[i]+k  >= width)
    {
      vx[i] = -vx[i];
      px[i] = width-k-1;
    }
    k = pixel_size*LbTextStringHeight(text);
    if (py[i]+k >= height)
    {
      vy[i] = -vy[i];
      py[i] = height-k-1;
    }
    if (LbScreenIsLocked())
    {
      LbTextDrawResized(scale_fixed_DK_value(px[i]/pixel_size), scale_fixed_DK_value(py[i]/pixel_size), ee_units_per_px, text);
    }
    play_non_3d_sample_no_overlap(90);
  }
  if ((game.flags_font & FFlg_AlexCheat) == 0)
    return;

  if (game.eastegg02_cntr >= eastegg_jlw_codes.length)
  {
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    LbTextSetFont(winfont);
    i = 1;
    text = buf_sprintf("Alex, hopefully lying on a beach with Jo, says Hi");
    px[i] += vx[i];
    if (px[i] < 0)
    {
      px[i] = 0;
      vx[i] = -vx[i];
    }
    py[i] += vy[i];
    if (py[i] < 0)
    {
      py[i] = 0;
      vy[i] = -vy[i];
    }
    k = pixel_size * LbTextStringWidth(text);
    if (px[i]+k >= width)
    {
      vx[i] = -vx[i];
      px[i] = width-k-1;
    }
    k = pixel_size * LbTextStringHeight(text);
    if (py[i]+k >= height)
    {
      vy[i] = -vy[i];
      py[i] = height-k-1;
    }
    if (LbScreenIsLocked())
    {
        LbTextDrawResized(scale_fixed_DK_value(px[i]/pixel_size), scale_fixed_DK_value(py[i]/pixel_size), ee_units_per_px, text);
    }
    play_non_3d_sample_no_overlap(90);
  }
}

/******************************************************************************/
