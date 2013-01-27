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


#pragma pack()
/******************************************************************************/
DLLIMPORT int _DK_parchment_loaded;
#define parchment_loaded _DK_parchment_loaded
DLLIMPORT unsigned char *_DK_hires_parchment;
#define hires_parchment _DK_hires_parchment
/******************************************************************************/
void draw_map_parchment(void);
void parchment_copy_background_at(int rect_x,int rect_y,int rect_w,int rect_h);

void load_parchment_file(void);
void reload_parchment_file(TbBool hires);

void redraw_parchment_view(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
