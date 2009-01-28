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
DLLIMPORT  int __cdecl _DK_set_game_key(long key_id, unsigned char key, int shift_state, int ctrl_state);
/******************************************************************************/

/******************************************************************************/
/*
 * Returns X position of mouse cursor on screen.
 */
long GetMouseX(void)
{
  return lbDisplay.MMouseX * pixel_size;
}

/*
 * Returns Y position of mouse cursor on screen.
 */
long GetMouseY(void)
{
  //return _DK_GetMouseY();
  long result;
  result = lbDisplay.MMouseY * pixel_size;
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

/*
 * Checks if a specific key is pressed.
 */
short is_key_pressed(long key, long kmodif)
{
  if ( (kmodif==-1) || (kmodif==key_modifiers) )
    return lbKeyOn[key];
  return 0;
}

/*
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

/*
 * Clears the marking that a specific key is pressed.
 */
void clear_key_pressed(long key)
{
  if ((key<0) || (key>=sizeof(lbKeyOn)))
    return;
  lbKeyOn[key] = 0;
  lbInkey = 0;
}

/*
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
  short shift_state;
  short ctrl_state;
  if (lbInkey == 1)
  {
    _DK_defining_a_key = 0;
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
    if ( _DK_set_game_key(_DK_defining_a_key_id, lbInkey, shift_state, ctrl_state) )
      _DK_defining_a_key = 0;
    lbInkey = 0;
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
