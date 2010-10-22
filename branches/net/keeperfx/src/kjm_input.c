/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file kjm_input.c
 *     Keyboard-Joypad-Mouse input routines.
 * @par Purpose:
 *     Allows reading state of input devices.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "kjm_input.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
DLLIMPORT  int _DK_set_game_key(long key_id, unsigned char key, int shift_state, int ctrl_state);
DLLIMPORT void _DK_update_mouse(void);
DLLIMPORT long _DK_GetMouseY(void);
/******************************************************************************/

/******************************************************************************/
/**
 * Returns X position of mouse cursor on screen.
 */
long GetMouseX(void)
{
  long result;
  result = lbDisplay.MMouseX * (long)pixel_size;
  return result;
}

/**
 * Returns Y position of mouse cursor on screen.
 */
long GetMouseY(void)
{
  //return _DK_GetMouseY();
  long result;
  result = lbDisplay.MMouseY * (long)pixel_size;
/*  if ((lbDisplay.ScreenMode == 13) && (!MinimalResolutionSetup))
  {
      result -= 40;
      if (result < 0)
        result = 0;
  }*/
  return result;
}

short is_mouse_pressed_leftbutton(void)
{
  return lbDisplay.LeftButton;
}

short is_mouse_pressed_rightbutton(void)
{
  return lbDisplay.RightButton;
}

short is_mouse_pressed_lrbutton(void)
{
  return (lbDisplay.LeftButton || lbDisplay.RightButton);
}

void clear_mouse_pressed_lrbutton(void)
{
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
}

void update_left_button_released(void)
{
  left_button_released = 0;
  left_button_double_clicked = 0;
  if ( lbDisplay.LeftButton )
  {
    left_button_held = 1;
    left_button_held_x = GetMouseX();
    left_button_held_y = GetMouseY();
  }
  if (left_button_held)
  {
    if (!lbDisplay.MLeftButton)
    {
      left_button_released = 1;
      left_button_held = 0;
      left_button_released_x = GetMouseX();
      left_button_released_y = GetMouseY();
      if ( left_button_click_space_count < 5 )
      {
        left_button_double_clicked = 1;
        left_button_double_clicked_x = left_button_released_x;
        left_button_double_clicked_y = left_button_released_y;
      }
      left_button_click_space_count = 0;
    }
  } else
  {
    if (left_button_click_space_count < LONG_MAX)
      left_button_click_space_count++;
  }
}

void update_right_button_released(void)
{
  right_button_released = 0;
  right_button_double_clicked = 0;
  if (lbDisplay.RightButton)
  {
    right_button_held = 1;
    right_button_held_x = GetMouseX();
    right_button_held_y = GetMouseY();
  }
  if ( right_button_held )
  {
    if ( !lbDisplay.MRightButton )
    {
      right_button_released = 1;
      right_button_held = 0;
      right_button_released_x = GetMouseX();
      right_button_released_y = GetMouseY();
      if (right_button_click_space_count < 5)
      {
        right_button_double_clicked = 1;
        right_button_double_clicked_x = right_button_released_x;
        right_button_double_clicked_y = right_button_released_y;
      }
      right_button_click_space_count = 0;
    }
  } else
  {
    if (right_button_click_space_count < LONG_MAX)
      right_button_click_space_count++;
  }
}

void update_left_button_clicked(void)
{
  left_button_clicked = lbDisplay.LeftButton;
  left_button_clicked_x = lbDisplay.MouseX * (long)pixel_size;
  left_button_clicked_y = lbDisplay.MouseY * (long)pixel_size;
}

void update_right_button_clicked(void)
{
  right_button_clicked = lbDisplay.RightButton;
  right_button_clicked_x = lbDisplay.MouseX * (long)pixel_size;
  right_button_clicked_y = lbDisplay.MouseY * (long)pixel_size;
}

void update_mouse(void)
{
  update_left_button_released();
  update_right_button_released();
  update_left_button_clicked();
  update_right_button_clicked();
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
}

/**
 * Checks if a specific key is pressed.
 */
short is_key_pressed(long key, long kmodif)
{
  if ((kmodif == KM_DONTCARE) || (kmodif == key_modifiers))
    return lbKeyOn[key];
  return 0;
}

/**
 * Converts keyboard key code into ASCII character.
 */
unsigned short key_to_ascii(long key, long kmodif)
{
  if ((key<0) || (key>=128))
    return 0;
  if (kmodif & KM_SHIFT)
    return lbInkeyToAsciiShift[key];
  return lbInkeyToAscii[key];
}

/**
 * Clears the marking that a specific key is pressed.
 */
void clear_key_pressed(long key)
{
  if ((key<0) || (key>=sizeof(lbKeyOn)))
    return;
  lbKeyOn[key] = 0;
  if (key == lbInkey)
    lbInkey = KC_UNASSIGNED;
}

/**
 * Set key modifiers based on the pressed key codes.
 */
void update_key_modifiers(void)
{
  unsigned short key_mods=0;
  if ( lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT] )
    key_mods |= KM_SHIFT;
  if ( lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL] )
    key_mods |= KM_CONTROL;
  if ( lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT] )
    key_mods |= KM_ALT;
  key_modifiers = key_mods;
}

void define_key_input(void)
{
  TbBool shift_state;
  TbBool ctrl_state;
  if (lbInkey == 1)
  {
    defining_a_key = 0;
    lbInkey = 0;
  } else
  if (lbInkey != 0)
  {
    ctrl_state = 0;
    if ( lbKeyOn[KC_LCONTROL] || (lbKeyOn[KC_RCONTROL]) )
      ctrl_state = 1;
    shift_state = 0;
    if ( lbKeyOn[KC_LSHIFT] || (lbKeyOn[KC_RSHIFT]) )
      shift_state = 1;
    if ( _DK_set_game_key(defining_a_key_id, lbInkey, shift_state, ctrl_state) )
      defining_a_key = 0;
    lbInkey = 0;
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
