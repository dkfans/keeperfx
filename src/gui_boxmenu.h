/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_boxmenu.h
 *     Header file for gui_boxmenu.c.
 * @par Purpose:
 *     Displaying service menu on screen.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_BOXMENU_H
#define DK_GUI_BOXMENU_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct GuiBox;
struct GuiBoxOption;

#pragma pack()
/******************************************************************************/
void gui_draw_all_boxes(void);
short gui_box_is_not_valid(struct GuiBox *gbox);
struct GuiBox *gui_create_box(long x, long y, struct GuiBoxOption *optn_list);
void gui_delete_box(struct GuiBox *gbox);
void gui_draw_box(struct GuiBox *gbox);
short gui_move_box(struct GuiBox *gbox, long x, long y, unsigned short fdflags);
struct GuiBox *gui_get_highest_priority_box(void);
struct GuiBox *gui_get_lowest_priority_box(void);
struct GuiBox *gui_get_next_highest_priority_box(struct GuiBox *gbox);
struct GuiBox *gui_get_next_lowest_priority_box(struct GuiBox *gbox);
void gui_remove_box_from_list(struct GuiBox *gbox);
void gui_insert_box_at_list_top(struct GuiBox *gbox);
struct GuiBox *gui_get_box_point_over(long x, long y);
struct GuiBoxOption *gui_get_box_option_point_over(struct GuiBox *gbox, long x, long y);
short gui_process_inputs(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
