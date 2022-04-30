/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_map.h
 *     Header file for frontmenu_ingame_map.c.
 * @par Purpose:
 *     Map on in-game GUI panel drawing and support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 23 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_FRONTMENU_INGAMEMAP_H
#define DK_FRONTMENU_INGAMEMAP_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PANNEL_MAP_RADIUS       58
/******************************************************************************/
DLLIMPORT long _DK_clicked_on_small_map;
#define clicked_on_small_map _DK_clicked_on_small_map
DLLIMPORT unsigned char _DK_grabbed_small_map;
#define grabbed_small_map _DK_grabbed_small_map
DLLIMPORT long _DK_PannelMapY;
#define PannelMapY _DK_PannelMapY
DLLIMPORT long _DK_PannelMapX;
#define PannelMapX _DK_PannelMapX
DLLIMPORT long _DK_MapShapeStart[116];
//#define MapShapeStart _DK_MapShapeStart
DLLIMPORT long _DK_MapShapeEnd[116];
//#define MapShapeEnd _DK_MapShapeEnd
DLLIMPORT long _DK_NoBackColours;
#define NoBackColours _DK_NoBackColours
DLLIMPORT long _DK_PrevPixelSize;
#define PrevPixelSize _DK_PrevPixelSize
DLLIMPORT unsigned char _DK_MapBackColours[256];
#define MapBackColours _DK_MapBackColours
DLLIMPORT unsigned char _DK_MapBackground[116*116];//pannel map size in pixels
//#define MapBackground _DK_MapBackground
DLLIMPORT unsigned char _DK_PannelColours[4096];
#define PannelColours _DK_PannelColours
DLLIMPORT long _DK_PrevRoomHighlight;
#define PrevRoomHighlight _DK_PrevRoomHighlight
DLLIMPORT long _DK_PrevDoorHighlight;
#define PrevDoorHighlight _DK_PrevDoorHighlight
DLLIMPORT unsigned char _DK_PannelMap[256*256];//map subtiles x*y
#define PannelMap _DK_PannelMap
/******************************************************************************/
extern long MapDiagonalLength;
/******************************************************************************/
void pannel_map_update(long x, long y, long w, long h);
void pannel_map_draw_slabs(long x, long y, long units_per_px, long zoom);
void pannel_map_draw_overlay_things(long units_per_px, long zoom, long basic_zoom);

void do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
short do_left_map_drag(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_left_map_click(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_right_map_click(long start_x, long start_y, long curr_x, long curr_y, long zoom);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
