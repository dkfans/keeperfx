/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_parchment.h
 *     Header file for gui_parchment.c.
 * @par Purpose:
 *     The map parchment screen support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     23 May 2010 - 10 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_PARCHMENT_H
#define DK_GUI_PARCHMENT_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct TbRect;
struct Camera;

/******************************************************************************/
DLLIMPORT int _DK_parchment_loaded;
#define parchment_loaded _DK_parchment_loaded
DLLIMPORT unsigned char *_DK_hires_parchment;
#define hires_parchment _DK_hires_parchment

#pragma pack()
/******************************************************************************/
void draw_map_parchment(void);
TbBool parchment_copy_background_at(const struct TbRect *bkgnd_area, int m);

void load_parchment_file(void);
void reload_parchment_file(TbBool hires);

void redraw_parchment_view(void);
void redraw_minimal_overhead_view(void);

long get_parchment_map_area_rect(struct TbRect *map_area);
TbBool point_to_overhead_map(const struct Camera *camera, const long screen_x, const long screen_y, long *map_x, long *map_y);

void zoom_from_patchment_map(void);
void zoom_to_parchment_map(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
