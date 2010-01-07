/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_tooltips.h
 *     Header file for gui_tooltips.c.
 * @par Purpose:
 *     Tooltips support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     26 Feb 2009 - 14 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUITLTIPS_H
#define DK_GUITLTIPS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct GuiButton;

enum TooltipFlags {
  TTip_Visible   = 0x01,
  TTip_NeedReset = 0x02,
};

/******************************************************************************/
DLLIMPORT extern long _DK_tooltip_scroll_offset;
#define tooltip_scroll_offset _DK_tooltip_scroll_offset
DLLIMPORT extern long _DK_tooltip_scroll_timer;
#define tooltip_scroll_timer _DK_tooltip_scroll_timer
DLLIMPORT extern struct ToolTipBox _DK_tool_tip_box;
#define tool_tip_box _DK_tool_tip_box

#pragma pack()
/******************************************************************************/
void toggle_tooltips(void);
void draw_tooltip(void);
TbBool input_gameplay_tooltips(TbBool gameplay_on);
short setup_scrolling_tooltips(struct Coord3d *mappos);
void setup_gui_tooltip(struct GuiButton *gbtn);
TbBool gui_button_tooltip_update(int gbtn_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
