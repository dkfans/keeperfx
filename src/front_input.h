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
    Gkey_MoveUp = 0,
    Gkey_MoveDown,
    Gkey_MoveLeft,
    Gkey_MoveRight,
    Gkey_RotateMod,
    Gkey_SpeedMod, // 5
    Gkey_RotateCW,
    Gkey_RotateCCW,
    Gkey_ZoomIn,
    Gkey_ZoomOut,
    Gkey_ZoomRoom00, // 10
    Gkey_ZoomRoom01,
    Gkey_ZoomRoom02,
    Gkey_ZoomRoom03,
    Gkey_ZoomRoom04,
    Gkey_ZoomRoom05, // 15
    Gkey_ZoomRoom06,
    Gkey_ZoomRoom07,
    Gkey_ZoomRoom08,
    Gkey_ZoomRoom09,
    Gkey_ZoomRoom10, // 20
    Gkey_ZoomRoom11,
    Gkey_ZoomRoom12,
    Gkey_ZoomRoom13,
    Gkey_ZoomRoom14,
    Gkey_ZoomToFight, // 25
    Gkey_ZoomCrAnnoyed,
    Gkey_CrtrContrlMod,
    Gkey_CrtrQueryMod,
    Gkey_DumpToOldPos,
    Gkey_TogglePause, // 30
    Gkey_SwitchToMap,
    Gkey_ToggleMessage,
    Gkey_SnapCamera,
    Gkey_BestRoomSpace,
    Gkey_SquareRoomSpace, // 35
    Gkey_RoomSpaceIncSize,
    Gkey_RoomSpaceDecSize,
    Gkey_SellTrapOnSubtile,
};

enum TbButtonFrontendFlags {
    LbBFeF_IntValueMask = 0x3fff,
    LbBFeF_NoMouseOver  = 0x4000,
    LbBFeF_NoTooltip    = 0x8000,
};

// Rudimentary GUI Layer support
// Add layers to this enum to distinguish between input layers
// This allows conflicting use of the same input to be resolved sensibly
// e.g. `GuiLayer_OneClick` is supposed to signify that the user is in "one-click mode"
enum GuiLayers {
    GuiLayer_Default  = 0,
    GuiLayer_OneClick,
    GuiLayer_OneClickBridgeBuild,
};

struct GuiLayer {
    long current_gui_layer;
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
void input(void);
short get_inputs(void);
short get_screen_capture_inputs(void);
int is_game_key_pressed(long key_id, long *val, TbBool ignore_mods);
short game_is_busy_doing_gui_string_input(void);
short get_gui_inputs(short gameplay_on);
extern unsigned short const zoom_key_room_order[];
TbBool check_if_mouse_is_over_button(const struct GuiButton *gbtn);
long get_current_gui_layer();
TbBool check_current_gui_layer(long layer_id);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
