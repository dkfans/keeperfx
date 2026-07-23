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
  int32_t str_idx;
};

struct TbSpriteSheet;

/******************************************************************************/
extern uint32_t key_modifiers;
extern int defining_a_key;
extern int32_t defining_a_key_id;

extern int32_t left_button_held_x;
extern int32_t left_button_held_y;
extern int32_t left_button_double_clicked_y;
extern int32_t left_button_double_clicked_x;
extern int32_t right_button_double_clicked_y;
extern int32_t right_button_double_clicked_x;
extern char right_button_clicked;
extern char left_button_clicked;
extern int32_t right_button_released_x;
extern int32_t right_button_released_y;
extern char right_button_double_clicked;
extern int32_t left_button_released_y;
extern int32_t left_button_released_x;
extern char left_button_double_clicked;
extern char right_button_released;
extern char right_button_held;
extern int32_t right_button_click_space_count;
extern int32_t right_button_held_y;
extern int32_t left_button_clicked_y;
extern int32_t left_button_clicked_x;
extern int32_t left_button_click_space_count;
extern int32_t right_button_held_x;
extern char left_button_released;
extern int32_t right_button_clicked_y;
extern int32_t right_button_clicked_x;
extern char left_button_held;

extern int32_t key_to_string[256];

#pragma pack()
/******************************************************************************/
extern TbBool defined_keys_that_have_been_swapped[];
extern TbBool wheel_scrolled_up;
extern TbBool wheel_scrolled_down;

TbBool poll_inputs(void);

int32_t GetMouseX(void);
int32_t GetMouseY(void);
short is_mouse_pressed_lrbutton(void);
void clear_mouse_pressed_lrbutton(void);
void update_mouse(void);
void update_wheel_scrolled(void);

short is_key_pressed(TbKeyCode key, TbKeyMods kmodif);
void clear_key_pressed(int32_t key);
void update_key_modifiers(void);
void define_key_input(void);
void init_key_to_strings(void);
TbBool add_input_text_to_message(char *message, int max_message_length, struct TbSpriteSheet *font, int max_width);

TbBool mouse_is_over_panel_map(ScreenCoord x, ScreenCoord y);
TbBool mouse_is_over_side_panel_bottom();

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
