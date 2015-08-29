/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_input.h
 *     Header file for front_input.c.
 * @par Purpose:
 *     Front-end user keyboard and mouse input.
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

#ifndef DK_FRONTINPUT_H
#define DK_FRONTINPUT_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum GameKeys {
    GameKey_MoveUp = 0,
    GameKey_MoveDown,
    GameKey_MoveLeft,
    GameKey_MoveRight,
    GameKey_RotateMod,
    GameKey_SpeedMod, // 5
    GameKey_RotateCW,
    GameKey_RotateCCW,
    GameKey_ZoomIn,
    GameKey_ZoomOut,
    GameKey_ZoomRoom00, // 10
    GameKey_ZoomRoom01,
    GameKey_ZoomRoom02,
    GameKey_ZoomRoom03,
    GameKey_ZoomRoom04,
    GameKey_ZoomRoom05, // 15
    GameKey_ZoomRoom06,
    GameKey_ZoomRoom07,
    GameKey_ZoomRoom08,
    GameKey_ZoomRoom09,
    GameKey_ZoomRoom10, // 20
    GameKey_ZoomRoom11,
    GameKey_ZoomRoom12,
    GameKey_ZoomRoom13,
    GameKey_ZoomRoom14,
    GameKey_ZoomToFight, // 25
    GameKey_ZoomCrAnnoyed,
    GameKey_Possession,
    GameKey_QueryInfo,
    GameKey_DumpToOldPos,
    GameKey_TogglePause, // 30
    GameKey_SwitchToMap,
};

struct GuiMenu;
struct GuiButton;

#pragma pack()
/******************************************************************************/
DLLIMPORT long _DK_old_mx;
#define old_mx _DK_old_mx
DLLIMPORT long _DK_old_my;
#define old_my _DK_old_my
/******************************************************************************/
void input(int do_draw);
short get_inputs(int do_draw);
short get_screen_capture_inputs(void);
int is_game_key_pressed(long key_id, long *val, TbBool ignore_mods);
short game_is_busy_doing_gui_string_input(void);
short get_gui_inputs(short gameplay_on);
TbBool check_if_mouse_is_over_button(const struct GuiButton *gbtn);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
