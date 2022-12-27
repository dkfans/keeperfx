/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file kjm_input.h
 *     Header file for kjm_input.c.
 * @par Purpose:
 *     Keyboard-Joypad-Mouse input routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_KJMINPUT_H
#define DK_KJMINPUT_H

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_keybrd.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct KeyToStringInit { // sizeof = 5
  unsigned char chr;
  long str_idx;
};

/******************************************************************************/
DLLIMPORT extern unsigned long _DK_key_modifiers;
#define key_modifiers _DK_key_modifiers
DLLIMPORT extern int _DK_defining_a_key;
#define defining_a_key _DK_defining_a_key
DLLIMPORT extern long _DK_defining_a_key_id;
#define defining_a_key_id _DK_defining_a_key_id
DLLIMPORT extern long _DK_left_button_held_x;
#define left_button_held_x _DK_left_button_held_x
DLLIMPORT extern long _DK_left_button_held_y;
#define left_button_held_y _DK_left_button_held_y
DLLIMPORT extern long _DK_left_button_double_clicked_y;
#define left_button_double_clicked_y _DK_left_button_double_clicked_y
DLLIMPORT extern long _DK_left_button_double_clicked_x;
#define left_button_double_clicked_x _DK_left_button_double_clicked_x
DLLIMPORT extern long _DK_right_button_double_clicked_y;
#define right_button_double_clicked_y _DK_right_button_double_clicked_y
DLLIMPORT extern long _DK_right_button_double_clicked_x;
#define right_button_double_clicked_x _DK_right_button_double_clicked_x
DLLIMPORT extern char _DK_right_button_clicked;
#define right_button_clicked _DK_right_button_clicked
DLLIMPORT extern char _DK_left_button_clicked;
#define left_button_clicked _DK_left_button_clicked
DLLIMPORT extern long _DK_right_button_released_x;
#define right_button_released_x _DK_right_button_released_x
DLLIMPORT extern long _DK_right_button_released_y;
#define right_button_released_y _DK_right_button_released_y
DLLIMPORT extern char _DK_right_button_double_clicked;
#define right_button_double_clicked _DK_right_button_double_clicked
DLLIMPORT extern long _DK_left_button_released_y;
#define left_button_released_y _DK_left_button_released_y
DLLIMPORT extern long _DK_left_button_released_x;
#define left_button_released_x _DK_left_button_released_x
DLLIMPORT extern char _DK_left_button_double_clicked;
#define left_button_double_clicked _DK_left_button_double_clicked
DLLIMPORT extern char _DK_right_button_released;
#define right_button_released _DK_right_button_released
DLLIMPORT extern char _DK_right_button_held;
#define right_button_held _DK_right_button_held
DLLIMPORT extern long _DK_right_button_click_space_count;
#define right_button_click_space_count _DK_right_button_click_space_count
DLLIMPORT extern long _DK_right_button_held_y;
#define right_button_held_y _DK_right_button_held_y
DLLIMPORT extern long _DK_left_button_clicked_y;
#define left_button_clicked_y _DK_left_button_clicked_y
DLLIMPORT extern long _DK_left_button_clicked_x;
#define left_button_clicked_x _DK_left_button_clicked_x
DLLIMPORT extern long _DK_left_button_click_space_count;
#define left_button_click_space_count _DK_left_button_click_space_count
DLLIMPORT extern long _DK_right_button_held_x;
#define right_button_held_x _DK_right_button_held_x
DLLIMPORT extern char _DK_left_button_released;
#define left_button_released _DK_left_button_released
DLLIMPORT extern long _DK_right_button_clicked_y;
#define right_button_clicked_y _DK_right_button_clicked_y
DLLIMPORT extern long _DK_right_button_clicked_x;
#define right_button_clicked_x _DK_right_button_clicked_x
DLLIMPORT extern char _DK_left_button_held;
#define left_button_held _DK_left_button_held
DLLIMPORT long _DK_key_to_string[256];
#define key_to_string _DK_key_to_string

#pragma pack()
/******************************************************************************/
extern TbBool defined_keys_that_have_been_swapped[];
extern TbBool wheel_scrolled_up;
extern TbBool wheel_scrolled_down;


long GetMouseX(void);
long GetMouseY(void);
short is_mouse_pressed_leftbutton(void);
short is_mouse_pressed_rightbutton(void);
short is_mouse_pressed_lrbutton(void);
void clear_mouse_pressed_lrbutton(void);
void update_mouse(void);
void update_wheel_scrolled(void);

short is_key_pressed(TbKeyCode key, TbKeyMods kmodif);
unsigned short key_to_ascii(TbKeyCode key, TbKeyMods kmodif);
void clear_key_pressed(long key);
void update_key_modifiers(void);
void define_key_input(void);
void init_key_to_strings(void);

TbBool mouse_is_over_pannel_map(ScreenCoord x, ScreenCoord y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
