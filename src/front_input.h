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

#define CHEAT_GAME_KEYS 6 // number of game keys that are considered cheat keys and are only enabled when Easter Eggs are enabled

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
    Gkey_ZoomRoomTreasure, // 10
    Gkey_ZoomRoomLibrary,
    Gkey_ZoomRoomLair,
    Gkey_ZoomRoomPrison,
    Gkey_ZoomRoomTorture,
    Gkey_ZoomRoomTraining, // 15
    Gkey_ZoomRoomHeart,
    Gkey_ZoomRoomWorkshop,
    Gkey_ZoomRoomScavenger,
    Gkey_ZoomRoomTemple,
    Gkey_ZoomRoomGraveyard, // 20
    Gkey_ZoomRoomBarracks,
    Gkey_ZoomRoomHatchery,
    Gkey_ZoomRoomGuardPost,
    Gkey_ZoomRoomBridge,
    Gkey_ZoomRoomPortal, // 25
    Gkey_ZoomToFight,
    Gkey_ZoomCrAnnoyed,
    Gkey_CrtrContrlMod,
    Gkey_CrtrQueryMod,
    Gkey_DumpToOldPos, // 30
    Gkey_TogglePause,
    Gkey_SwitchToMap,
    Gkey_ToggleMessage,
    Gkey_SnapCamera,
    Gkey_BestRoomSpace, // 35
    Gkey_SquareRoomSpace,
    Gkey_RoomSpaceIncSize,
    Gkey_RoomSpaceDecSize,
    Gkey_SellTrapOnSubtile,
    Gkey_TiltUp, // 40
    Gkey_TiltDown,
    Gkey_TiltReset,
    Gkey_Ascend,
    Gkey_Descend,
    GKey_ScreenRecord,
    GKey_ScreenShot,
    GKey_FrameSkipIncrease,
    GKey_FrameSkipDecrease,
    GKey_ZoomMinimapIn,
    GKey_ZoomMinimapOut,
    GKey_NextInstance,
    GKey_PrevInstance,
    Gkey_ToggleGui,
    Gkey_ToggleTooltips,
    Gkey_ExitGame,
    Gkey_DisablePacketMode,
    Gkey_SwitchScreenRes,
    Gkey_ToggleConsole,
    Gkey_FinishLevel,
    Gkey_ToggleHeroHealthFlowers,
    Gkey_TeleportFight,
    Gkey_TeleportLastWorkroom,
    Gkey_TeleportCallToArms,
    Gkey_TeleportDefault,
    GKey_CheatMenu1,
    GKey_CheatMenu2,
    GKey_CheatMenu3,
    Gkey_LVShowAllEnsigns,
    Gkey_LVNextLevel,
    Gkey_LVPrevLevel,
    GAME_KEYS_COUNT
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
extern long old_mx;
extern long old_my;
extern int synthetic_left;
extern int synthetic_right;
/******************************************************************************/
void input(void);
short get_inputs(void);
short get_screen_capture_inputs(void);
int is_game_key_pressed(long key_id, int32_t *val, TbBool ignore_mods);
short game_is_busy_doing_gui_string_input(void);
short get_gui_inputs(short gameplay_on);
extern unsigned short const zoom_key_room_order[];
TbBool check_if_mouse_is_over_button(const struct GuiButton *gbtn);
long get_current_gui_layer();
TbBool check_current_gui_layer(long layer_id);
void process_cheat_mode_selection_inputs();
TbBool process_cheat_heart_health_inputs(HitPoints *value, HitPoints max_health);
void disable_packet_mode();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
