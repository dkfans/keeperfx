/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_redraw.h
 *     Header file for engine_redraw.c.
 * @par Purpose:
 *     Functions to redraw the engine screen.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     06 Nov 2010 - 03 Jul 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_ENGNRDRAW_H
#define DK_ENGNRDRAW_H

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;

#pragma pack()
/******************************************************************************/
/** Screen columns on the left the engine window must leave free for the sidebar. */
int32_t engine_window_reserved_left(void);
void setup_engine_window(long x1, long y1, long x2, long y2);
void store_engine_window(TbGraphicsWindow *ewnd,int divider);
void load_engine_window(TbGraphicsWindow *ewnd);

void set_user_dungeon_view(NetUserId user, TbBool front_view, TbBool wibble);
void update_local_view(NetUserId user, TbBool keep_local_camera);

void draw_overlay_compass(long a1, long a2);

void clear_custom_buff_icons(void);
TbBool add_custom_buff_icon(long spridx);

TbBool keeper_screen_redraw(void);

int get_place_terrain_pointer_graphics(SlabKind skind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
