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
#include "pre_inc.h"
#include "kjm_input.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_math.h"

#include "config_settings.h"
#include "config_strings.h"
#include "frontend.h"
#include "front_input.h"
#include "frontmenu_ingame_map.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbBool wheel_scrolled_up;
TbBool wheel_scrolled_down;

unsigned long key_modifiers;
int defining_a_key;
long defining_a_key_id;

long left_button_held_x;
long left_button_held_y;
long left_button_double_clicked_y;
long left_button_double_clicked_x;
long right_button_double_clicked_y;
long right_button_double_clicked_x;
char right_button_clicked;
char left_button_clicked;
long right_button_released_x;
long right_button_released_y;
char right_button_double_clicked;
long left_button_released_y;
long left_button_released_x;
char left_button_double_clicked;
char right_button_released;
char right_button_held;
long right_button_click_space_count;
long right_button_held_y;
long left_button_clicked_y;
long left_button_clicked_x;
long left_button_click_space_count;
long right_button_held_x;
char left_button_released;
long right_button_clicked_y;
long right_button_clicked_x;
char left_button_held;

long key_to_string[256];

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
  {KC_F1,  GUIStr_KeyF1},
  {KC_F2,  GUIStr_KeyF2},
  {KC_F3,  GUIStr_KeyF3},
  {KC_F4,  GUIStr_KeyF4},
  {KC_F5,  GUIStr_KeyF5},
  {KC_F6,  GUIStr_KeyF6},
  {KC_F7,  GUIStr_KeyF7},
  {KC_F8,  GUIStr_KeyF8},
  {KC_F9,  GUIStr_KeyF9},
  {KC_F10, GUIStr_KeyF10},
  {KC_F11, GUIStr_KeyF11},
  {KC_F12, GUIStr_KeyF12},
  {KC_CAPITAL, GUIStr_KeyCapsLock},
  {KC_LSHIFT,  GUIStr_KeyLeftShift},
  {KC_RSHIFT,  GUIStr_KeyRightShift},
  {KC_LCONTROL, GUIStr_KeyLeftControl},
  {KC_RCONTROL, GUIStr_KeyRightControl},
  {KC_RETURN,  GUIStr_KeyReturn},
  {KC_BACK,    GUIStr_KeyBackspace},
  {KC_INSERT,  GUIStr_KeyInsert},
  {KC_DELETE,  GUIStr_KeyDelete},
  {KC_HOME,    GUIStr_KeyHome},
  {KC_END,     GUIStr_KeyEnd},
  {KC_PGUP,    GUIStr_KeyPageUp},
  {KC_PGDOWN,  GUIStr_KeyPageDown},
  {KC_NUMLOCK, GUIStr_KeyNumLock},
  {KC_DIVIDE,  GUIStr_KeyNumSlash},
  {KC_MULTIPLY, GUIStr_KeyNumMul},
  {KC_NUMPADENTER, GUIStr_KeyNumEnter},
  {KC_DECIMAL, GUIStr_KeyNumDelete},
  {KC_NUMPAD0, GUIStr_KeyNum0},
  {KC_NUMPAD1, GUIStr_KeyNum1},
  {KC_NUMPAD2, GUIStr_KeyNum2},
  {KC_NUMPAD3, GUIStr_KeyNum3},
  {KC_NUMPAD4, GUIStr_KeyNum4},
  {KC_NUMPAD5, GUIStr_KeyNum5},
  {KC_NUMPAD6, GUIStr_KeyNum6},
  {KC_NUMPAD7, GUIStr_KeyNum7},
  {KC_NUMPAD8, GUIStr_KeyNum8},
  {KC_NUMPAD9, GUIStr_KeyNum9},
  {KC_UP,     GUIStr_KeyUp},
  {KC_DOWN,   GUIStr_KeyDown},
  {KC_LEFT,   GUIStr_KeyLeft},
  {KC_RIGHT,  GUIStr_KeyRight},
  {KC_LALT,   GUIStr_KeyLeftAlt},
  {KC_RALT,   GUIStr_KeyRightAlt},
// [mouse buttons as keybinds - quick fix]
  {KC_MOUSE3,          GUIStr_MouseButton},
  {KC_MOUSEWHEEL_UP,   GUIStr_MouseScrollWheelUp},
  {KC_MOUSEWHEEL_DOWN, GUIStr_MouseScrollWheelDown},
  {KC_MOUSE4,          GUIStr_MouseButton},
  {KC_MOUSE5,          GUIStr_MouseButton},
  {KC_MOUSE6,          GUIStr_MouseButton},
  {KC_MOUSE7,          GUIStr_MouseButton},
  {KC_MOUSE8,          GUIStr_MouseButton},
  {KC_MOUSE9,          GUIStr_MouseButton},
  {  0,     0},
};

// An array of the defined keys, when an indexed key is true in this array,
// it should be highlighted in font color #3 in the list, to show that it was swapped
TbBool defined_keys_that_have_been_swapped[GAME_KEYS_COUNT] = { false };
/******************************************************************************/
/**
 * Returns X position of mouse cursor on screen.
 */
long GetMouseX(void)
{
    long result = lbDisplay.MMouseX * (long)pixel_size;
    return result;
}

/**
 * Returns Y position of mouse cursor on screen.
 */
long GetMouseY(void)
{
    long result = lbDisplay.MMouseY * (long)pixel_size;
    return result;
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
    if (left_button_click_space_count < INT32_MAX)
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
    if (right_button_click_space_count < INT32_MAX)
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

void update_wheel_scrolled(void)
{
    wheel_scrolled_up = (lbDisplayEx.WhellMoveUp > 0);
    wheel_scrolled_down = (lbDisplayEx.WhellMoveDown > 0);
}

/**
 * Translates mouse input from lbDisplay struct into simple variables.
 */
void update_mouse(void)
{
  update_left_button_released();
  update_right_button_released();
  update_left_button_clicked();
  update_right_button_clicked();
  update_wheel_scrolled();
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
  lbDisplayEx.WhellMoveUp = 0;
  lbDisplayEx.WhellMoveDown = 0;
  // [mouse buttons as keybinds - quick fix]
  lbKeyOn[KC_MOUSE3] = lbDisplay.MiddleButton;
  lbKeyOn[KC_MOUSEWHEEL_UP] = wheel_scrolled_up;
  lbKeyOn[KC_MOUSEWHEEL_DOWN] = wheel_scrolled_down;
  lbInkey = lbDisplay.MiddleButton ? KC_MOUSE3 : wheel_scrolled_down ? KC_MOUSEWHEEL_DOWN : wheel_scrolled_up ? KC_MOUSEWHEEL_UP : lbInkey;

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
  if (key >= 128)
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
    if (key >= sizeof(lbKeyOn))
    {
        return;
    }
    if ((key >= 0xF0) && (key <= 0xFC)) // This is a mouse button
    {
        if (key == KC_MOUSE3)
        {
            lbDisplay.MiddleButton = 0;
        }
    }
    lbKeyOn[key] = 0;
    if (key == lbInkey)
    {
        lbInkey = KC_UNASSIGNED;
    }
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

void swap_assigned_keys(long current_key_id, struct GameKey* current_kbk, long new_key_id, unsigned char new_key, unsigned int new_mods)
{
    struct GameKey* kbk_swap = current_kbk;
    current_kbk = &settings.kbkeys[new_key_id];
    kbk_swap->code = current_kbk->code;
    kbk_swap->mods = current_kbk->mods;
    current_kbk->code = new_key;
    current_kbk->mods = new_mods;
    defined_keys_that_have_been_swapped[current_key_id] = true;
    if (defined_keys_that_have_been_swapped[new_key_id])
    {
        defined_keys_that_have_been_swapped[new_key_id] = false;
    }
}

void assign_key(long key_id, unsigned char key, unsigned int mods)
{
    struct GameKey* kbk = &settings.kbkeys[key_id];
    kbk->code = key;
    kbk->mods = mods;
    if (defined_keys_that_have_been_swapped[key_id])
    {
        defined_keys_that_have_been_swapped[key_id] = false;
    }
}

int mod_key_to_normal_key(unsigned int mods)
{
    int ncode;
    if (mods & KMod_SHIFT)
    {
        ncode = KC_LSHIFT;
    }
    else if (mods & KMod_CONTROL)
    {
        ncode = KC_LCONTROL;
    }
    else if (mods & KMod_ALT)
    {
        ncode = KC_LALT;
    }
    else
    {
        ERRORLOG("Reached a place we should not be able to");
        ncode = KC_UNASSIGNED;
    }
    return ncode;
}
void check_and_assign_mod_keys_group(long key_id, unsigned int mods, long reference_key_ids[], long reference_key_count)
{
    int ncode = mod_key_to_normal_key(mods);
    // Do not allow the key if it is used as other mod key by any in reference_key_ids[]
    struct GameKey *kbk;
    for (long i = 0; i < reference_key_count; i++)
    {
        kbk = &settings.kbkeys[reference_key_ids[i]];
        if ((reference_key_ids[i] != key_id) && (kbk->code == ncode))
        {
            swap_assigned_keys(reference_key_ids[i], kbk, key_id, ncode, 0);
            return;
        }
    }
    assign_key(key_id, ncode, 0);
}

void check_and_assign_mod_keys(long key_id, unsigned int mods, long reference_key_id)
{
    // This only works for a pair of adjacent "linked" keys (i.e Speed/Rotate and Query/Possess)
    int ncode = mod_key_to_normal_key(mods);
    // Do not allow the key if it is used as other mod key
    long other_key_id = ((unsigned int)(key_id - reference_key_id) < 1) + reference_key_id;
    struct GameKey* kbk = &settings.kbkeys[other_key_id];
    if (kbk->code != ncode)
    {
        assign_key(key_id, ncode, 0);
    }
    else
    {
        swap_assigned_keys(other_key_id, kbk, key_id, ncode, 0);
    }
}

void check_and_assign_normal_keys(long key_id, unsigned char key, unsigned int mods, unsigned int set_mod)
{
    struct GameKey  *kbk;
    for (long i = 0; i < GAME_KEYS_COUNT; i++)
    {
        kbk = &settings.kbkeys[i];
        if ((i != key_id) && (kbk->code == key) && (kbk->mods == mods))
        {
            swap_assigned_keys(i, kbk, key_id, key, (set_mod ? mods & (KMod_SHIFT|KMod_CONTROL|KMod_ALT) : 0));
            return;
        }
    }
    assign_key(key_id, key, (set_mod ? mods & (KMod_SHIFT|KMod_CONTROL|KMod_ALT) : 0));
}

long set_game_key(long key_id, unsigned char key, unsigned int mods)
{
    if (!key_to_string[key])
    {
      return 0;
    }
    // One-Click Build & Sell Trap on Subtile - allow lone modifiers and normal keys
    if (key_id == Gkey_SellTrapOnSubtile || key_id == Gkey_SquareRoomSpace || key_id == Gkey_BestRoomSpace)
    {
        if ((mods & KMod_SHIFT) || (mods & KMod_CONTROL) || (mods & KMod_ALT))
        {
            long reference_key_ids[3] = {Gkey_SellTrapOnSubtile, Gkey_SquareRoomSpace, Gkey_BestRoomSpace};
            check_and_assign_mod_keys_group(key_id, mods, reference_key_ids, 3);
            return 1;
        }
        else
        {
            check_and_assign_normal_keys(key_id, key, mods, 0);
            return 1;
        }
    }
    // Rotate & speed - allow lone modifiers and normal keys
    if (key_id == Gkey_RotateMod || key_id == Gkey_SpeedMod)
    {
        if ((mods & KMod_SHIFT) || (mods & KMod_CONTROL) || (mods & KMod_ALT))
        {
            check_and_assign_mod_keys(key_id, mods, Gkey_RotateMod);
            return 1;
        }
        else
        {
            check_and_assign_normal_keys(key_id, key, mods, 0);
            return 1;
        }
    }
    // Possess & query - allow lone modifiers and normal keys
    if (key_id == Gkey_CrtrContrlMod || key_id == Gkey_CrtrQueryMod)
    {
        if ((mods & KMod_SHIFT) || (mods & KMod_CONTROL) || (mods & KMod_ALT))
        {
            check_and_assign_mod_keys(key_id, mods, Gkey_CrtrContrlMod);
            return 1;
        }
        else
        {
            check_and_assign_normal_keys(key_id, key, mods, 0);
            return 1;
        }
    }
    // Single control keys - just ignore these keystrokes
    if ( key == KC_LSHIFT || key == KC_RSHIFT || key == KC_LCONTROL || key == KC_RCONTROL  || key == KC_LALT || key == KC_RALT )
    {
        return 0;
    }
    // The normal keys - allow a key alone, or with one modifier
    {
        if (((mods & KMod_SHIFT) && (mods & KMod_CONTROL))
         || ((mods & KMod_SHIFT) && (mods & KMod_ALT))
         || ((mods & KMod_CONTROL) && (mods & KMod_ALT)))
        {
            return 0;
        }
        check_and_assign_normal_keys(key_id, key, mods, 1);
        return 1;
    }
}

void define_key_input(void)
{
  if (lbInkey == KC_ESCAPE)
  {
      lbKeyOn[KC_ESCAPE] = 0;
      lbInkey = KC_UNASSIGNED;
      defining_a_key = 0;
  } else
  if (right_button_clicked)
  {
      right_button_clicked = 0;
      defining_a_key = 0;
  } else
  if (lbInkey != KC_UNASSIGNED)
  {
      update_key_modifiers();
      if ( set_game_key(defining_a_key_id, lbInkey, key_modifiers) )
        defining_a_key = 0;
      lbInkey = KC_UNASSIGNED;
  }
}

/**
 * Fills the array of keyboard key names.
 */
void init_key_to_strings(void)
{
    memset(key_to_string, 0, sizeof(key_to_string));
    for (struct KeyToStringInit* ktsi = &key_to_string_init[0]; ktsi->chr != 0; ktsi++)
    {
        long k = ktsi->chr;
        key_to_string[k] = ktsi->str_idx;
    }
}

/**
 * Returns if the mouse is over "pannel map" - the circular minimap area on top left.
 * @param x Pannel map circle start X coordinate.
 * @param y Pannel map circle start Y coordinate.
 * @return
 */
TbBool mouse_is_over_panel_map(ScreenCoord x, ScreenCoord y)
{
    long cmx = GetMouseX();
    long cmy = GetMouseY();
    int units_per_px = (16 * status_panel_width + 140 / 2) / 140;
    long px = (cmx - (x + PANEL_MAP_RADIUS * units_per_px / 16));
    long py = (cmy - (y + PANEL_MAP_RADIUS * units_per_px / 16));
    return (LbSqrL(px*px + py*py) < PANEL_MAP_RADIUS*units_per_px/16);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
