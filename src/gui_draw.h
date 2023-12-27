/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_draw.h
 *     Header file for gui_draw.c.
 * @par Purpose:
 *     GUI elements drawing functions.
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

#ifndef DK_GUIDRAW_H
#define DK_GUIDRAW_H

#include "bflib_basics.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "globals.h"
#include "vidmode.h"

// Sprites
// Maybe "Count + 1"? there is no sprite#517
#define GUI_SLAB_DIMENSION 64
// Positioning constants for menus
#define POS_AUTO -9999
#define POS_MOUSMID -999
#define POS_MOUSPRV -998
#define POS_SCRCTR  -997
#define POS_SCRBTM  -996
#define POS_GAMECTR  999
#define ROUNDSLAB64K_LIGHT 0
#define ROUNDSLAB64K_DARK 1
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct GuiButton;
/******************************************************************************/
extern struct TbSprite gui_panel_sprites[];
extern int num_icons_total;

extern unsigned char * gui_panel_sprite_data;
extern unsigned char * end_gui_panel_sprite_data;

extern unsigned char *gui_slab;

extern unsigned char *frontend_background;

extern struct TbSprite *frontend_sprite;
extern struct TbSprite *frontend_end_sprite;
extern unsigned char * frontend_sprite_data;
extern unsigned char * frontend_end_sprite_data;

#pragma pack()
/******************************************************************************/
extern char gui_textbuf[TEXT_BUFFER_LENGTH];
/******************************************************************************/
int get_bitmap_max_scale(int img_w,int img_h,int rect_w,int rect_h);
int simple_button_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction);
int simple_button_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction);
int simple_frontend_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction);
int simple_frontend_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction);
int simple_gui_panel_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction);
int simple_gui_panel_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction);

void draw_bar64k(long pos_x, long pos_y, int units_per_px, long width);
void draw_lit_bar64k(long pos_x, long pos_y, int units_per_px, long width);
void draw_slab64k(long pos_x, long pos_y, int units_per_px, long width, long height);
void draw_ornate_slab64k(long pos_x, long pos_y, int units_per_px, long width, long height);
void draw_ornate_slab_outline64k(long pos_x, long pos_y, int units_per_px, long width, long height);
void draw_round_slab64k(long pos_x, long pos_y, int units_per_px, long width, long height, long style_type);
void draw_string64k(long x, long y, int units_per_px, const char * text);

void draw_button_string(struct GuiButton *gbtn, int base_width, const char *text);
TbBool draw_text_box(const char *text);
void draw_scroll_box(struct GuiButton *gbtn, int units_per_px, int num_rows);
int scroll_box_get_units_per_px(struct GuiButton *gbtn);

#define draw_gui_panel_sprite_left(x, y, units_per_px, spridx) draw_gui_panel_sprite_left_player(x, y, units_per_px, spridx, my_player_number) 
void draw_gui_panel_sprite_left_player(long x, long y, int units_per_px, long spridx, PlayerNumber plyr_idx);
void draw_gui_panel_sprite_rmleft(long x, long y, int units_per_px, long spridx, unsigned long remap);
void draw_gui_panel_sprite_ocleft(long x, long y, int units_per_px, long spridx, TbPixel color);
void draw_gui_panel_sprite_centered(long x, long y, int units_per_px, long spridx);
void draw_gui_panel_sprite_occentered(long x, long y, int units_per_px, long spridx, TbPixel color);
void draw_button_sprite_left(long x, long y, int units_per_px, long spridx);
void draw_button_sprite_rmleft(long x, long y, int units_per_px, long spridx, unsigned long remap);

void draw_frontend_sprite_left(long x, long y, int units_per_px, long spridx);

void draw_frontmenu_background(int rect_x,int rect_y,int rect_w,int rect_h);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
