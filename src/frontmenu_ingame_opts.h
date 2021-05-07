/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_opts.h
 *     Header file for frontmenu_ingame_opts.c.
 * @par Purpose:
 *     In-game options GUI, available under "escape" while in game.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 20 Apr 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTMENU_INGAMEOPTS_H
#define DK_FRONTMENU_INGAMEOPTS_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct GuiMenu;
struct GuiButton;

#pragma pack()
/******************************************************************************/
extern struct GuiMenu options_menu;
extern struct GuiMenu instance_menu;
extern struct GuiMenu quit_menu;
extern struct GuiMenu error_box;
extern struct GuiMenu autopilot_menu;
extern struct GuiMenu message_box;

extern struct GuiMenu video_menu;
extern struct GuiMenu sound_menu;

struct MsgBoxInfo {
    char title[24];
    char line1[24];
    char line2[24];
    char line3[24];
    char line4[24];
    char line5[24];
};
extern struct MsgBoxInfo MsgBox;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
