/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_specials.h
 *     Header file for frontmenu_specials.c.
 * @par Purpose:
 *     GUI menus for in-game dungeon special boxes.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTMENU_SPECIALS_H
#define DK_FRONTMENU_SPECIALS_H

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
extern struct GuiMenu hold_audience_menu;
extern struct GuiMenu dungeon_special_menu;
extern struct GuiMenu resurrect_creature_menu;
extern struct GuiMenu transfer_creature_menu;
extern struct GuiMenu armageddon_menu;
/******************************************************************************/
void select_resurrect_creature(struct GuiButton *gbtn);
void maintain_resurrect_creature_select(struct GuiButton *gbtn);
void draw_resurrect_creature(struct GuiButton *gbtn);
void select_resurrect_creature_up(struct GuiButton *gbtn);
void select_resurrect_creature_down(struct GuiButton *gbtn);
void maintain_resurrect_creature_scroll(struct GuiButton *gbtn);
void select_transfer_creature(struct GuiButton *gbtn);
void draw_transfer_creature(struct GuiButton *gbtn);
void maintain_transfer_creature_select(struct GuiButton *gbtn);
void select_transfer_creature_up(struct GuiButton *gbtn);
void select_transfer_creature_down(struct GuiButton *gbtn);
void maintain_transfer_creature_scroll(struct GuiButton *gbtn);
void maintain_resurrect_creature_select(struct GuiButton *gbtn);
void maintain_resurrect_creature_scroll(struct GuiButton *gbtn);
void maintain_transfer_creature_select(struct GuiButton *gbtn);
void maintain_transfer_creature_scroll(struct GuiButton *gbtn);
void choose_hold_audience(struct GuiButton *gbtn);
void choose_armageddon(struct GuiButton *gbtn);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
