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

#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_math.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_key_to_string[256];
#define key_to_string _DK_key_to_string
/******************************************************************************/
DLLIMPORT  long _DK_set_game_key(long key_id, unsigned char key, long shift_state, long ctrl_state);
DLLIMPORT void _DK_update_mouse(void);
DLLIMPORT long _DK_GetMouseY(void);
DLLIMPORT char _DK_mouse_is_over_small_map(int, int);
/******************************************************************************/
/** Initialization array, used to create array which stores index of text name of keyboard keys. */
struct KeyToStringInit key_to_string_init[] = {
  {KC_A,  -65},
  {KC_B,  -66},
  {KC_C,  -67},
  {KC_D,  -68},
  {KC_E,  -69},
  {KC_F,  -70},
  {KC_G,  -71},
  {KC_H,  -72},
  {KC_I,  -73},
  {KC_J,  -74},
  {KC_K,  -75},
  {KC_L,  -76},
  {KC_M,  -77},
  {KC_N,  -78},
  {KC_O,  -79},
  {KC_P,  -80},
  {KC_Q,  -81},
  {KC_R,  -82},
  {KC_S,  -83},
  {KC_T,  -84},
  {KC_U,  -85},
  {KC_V,  -86},
  {KC_W,  -87},
  {KC_X,  -88},
  {KC_Y,  -89},
  {KC_Z,  -90},
  {KC_F1,  515},
  {KC_F2,  516},
  {KC_F3,  517},
  {KC_F4,  518},
  {KC_F5,  519},
  {KC_F6,  520},
  {KC_F7,  521},
  {KC_F8,  522},
  {KC_F9,  523},
  {KC_F10, 524},
  {KC_F11, 525},
  {KC_F12, 526},
  {KC_CAPITAL, 490},
  {KC_LSHIFT,  483},
  {KC_RSHIFT,  484},
  {KC_LCONTROL, 481},
  {KC_RCONTROL, 482},
  {KC_RETURN,  488},
  {KC_BACK,    491},
  {KC_INSERT,  492},
  {KC_DELETE,  493},
  {KC_HOME,    494},
  {KC_END,     495},
  {KC_PGUP,    496},
  {KC_PGDOWN,  497},
  {KC_NUMLOCK, 498},
  {KC_DIVIDE,  499},
  {KC_MULTIPLY, 500},
  {KC_NUMPADENTER, 503},
  {KC_DECIMAL, 504},
  {KC_NUMPAD0, 514},
  {KC_NUMPAD1, 505},
  {KC_NUMPAD2, 506},
  {KC_NUMPAD3, 507},
  {KC_NUMPAD4, 508},
  {KC_NUMPAD5, 509},
  {KC_NUMPAD6, 510},
  {KC_NUMPAD7, 511},
  {KC_NUMPAD8, 512},
  {KC_NUMPAD9, 513},
  {KC_UP,     527},
  {KC_DOWN,   528},
  {KC_LEFT,   529},
  {KC_RIGHT,  530},
  {  0,     0},
};
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
 * @param key Code of the key to check.
 * @param kmodif Key modifier flags required.
 */
short is_key_pressed(TbKeyCode key, TbKeyMods kmodif)
{
  if ((kmodif == KMod_DONTCARE) || (kmodif == key_modifiers))
    return lbKeyOn[key];
  return 0;
}

/**
 * Converts keyboard key code into ASCII character.
 * @param key Code of the key being pressed.
 * @param kmodif Key modifier flags.
 * @note Key modifier can't be KMod_DONTCARE in this function.
 */
unsigned short key_to_ascii(TbKeyCode key, TbKeyMods kmodif)
{
  if ((key<0) || (key>=128))
    return 0;
  if (kmodif & KMod_SHIFT)
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
    key_mods |= KMod_SHIFT;
  if ( lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL] )
    key_mods |= KMod_CONTROL;
  if ( lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT] )
    key_mods |= KMod_ALT;
  key_modifiers = key_mods;
}

long set_game_key(long key_id, unsigned char key, long shift_state, long ctrl_state)
{
    return _DK_set_game_key(key_id, key, shift_state, ctrl_state);
}

void define_key_input(void)
{
  TbBool shift_state;
  TbBool ctrl_state;
  if (lbInkey == KC_ESCAPE)
  {
    defining_a_key = 0;
    lbInkey = KC_UNASSIGNED;
  } else
  if (lbInkey != KC_UNASSIGNED)
  {
    ctrl_state = 0;
    if ( lbKeyOn[KC_LCONTROL] || (lbKeyOn[KC_RCONTROL]) )
      ctrl_state = 1;
    shift_state = 0;
    if ( lbKeyOn[KC_LSHIFT] || (lbKeyOn[KC_RSHIFT]) )
      shift_state = 1;
    if ( set_game_key(defining_a_key_id, lbInkey, shift_state, ctrl_state) )
      defining_a_key = 0;
    lbInkey = KC_UNASSIGNED;
  }
}

/**
 * Fills the array of keyboard key names.
 */
void init_key_to_strings(void)
{
    struct KeyToStringInit *ktsi;
    long k;
    LbMemorySet(key_to_string, 0, sizeof(key_to_string));
    for (ktsi = &key_to_string_init[0]; ktsi->chr != 0; ktsi++)
    {
      k = ktsi->chr;
      key_to_string[k] = ktsi->str_idx;
    }
}

/**
 * Returns if the mouse is over "small map" - the circular minimap area on top left.
 * @param x Small map circle start X coordinate.
 * @param y Small map circle start Y coordinate.
 * @return
 */
TbBool mouse_is_over_small_map(long x, long y)
{
    long cmx,cmy;
    long px,py;
    cmx = GetMouseX();
    cmy = GetMouseY();
    px = (cmx-(x+SMALL_MAP_RADIUS));
    py = (cmy-(y+SMALL_MAP_RADIUS));
    return (LbSqrL(px*px + py*py) < SMALL_MAP_RADIUS);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
