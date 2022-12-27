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
extern unsigned long key_modifiers;
extern int defining_a_key;
extern long defining_a_key_id;

extern long left_button_held_x;
extern long left_button_held_y;
extern long left_button_double_clicked_y;
extern long left_button_double_clicked_x;
extern long right_button_double_clicked_y;
extern long right_button_double_clicked_x;
extern char right_button_clicked;
extern char left_button_clicked;
extern long right_button_released_x;
extern long right_button_released_y;
extern char right_button_double_clicked;
extern long left_button_released_y;
extern long left_button_released_x;
extern char left_button_double_clicked;
extern char right_button_released;
extern char right_button_held;
extern long right_button_click_space_count;
extern long right_button_held_y;
extern long left_button_clicked_y;
extern long left_button_clicked_x;
extern long left_button_click_space_count;
extern long right_button_held_x;
extern char left_button_released;
extern long right_button_clicked_y;
extern long right_button_clicked_x;
extern char left_button_held;

//doesn't actually need to still be in dll, just need 1 remaining DLLIMPORT so DLL is still loaded
//extern long key_to_string[256];
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
