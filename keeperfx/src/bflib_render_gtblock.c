/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_render_gtblock.c
 *     Rendering function gtblock_draw() for drawing 3D view elements.
 * @par Purpose:
 *     Function for rendering 3D elements.
 * @par Comment:
 *     Go away from here, you bad optimizer! Do not compile this with optimizations.
 * @author   Tomasz Lis
 * @date     05 Oct 2010 - 11 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_render.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"

/******************************************************************************/
DLLIMPORT unsigned char *_DK_gtblock_screen_addr;
#define gtblock_screen_addr _DK_gtblock_screen_addr
DLLIMPORT long _DK_gtblock_clip_width;
#define gtblock_clip_width _DK_gtblock_clip_width
DLLIMPORT long _DK_gtblock_clip_height;
#define gtblock_clip_height _DK_gtblock_clip_height
DLLIMPORT long _DK_gtblock_screen_width;
#define gtblock_screen_width _DK_gtblock_screen_width
/******************************************************************************/
unsigned char *LOC_gtblock_screen_addr;
long LOC_gtblock_clip_width;
long LOC_gtblock_clip_height;
long LOC_gtblock_screen_width;
/******************************************************************************/
void gtblock_set_clipping_window(unsigned char *screen_addr, long clip_width, long clip_height, long screen_width)
{
    gtblock_screen_addr = screen_addr;
    gtblock_clip_width = clip_width;
    gtblock_clip_height = clip_height;
    gtblock_screen_width = screen_width;
}
/******************************************************************************/
void gtblock_draw(struct GtBlock *gtb)
{
    //!!!!!!!!!!!
    LOC_gtblock_screen_addr = gtblock_screen_addr;
    LOC_gtblock_clip_width = gtblock_clip_width;
    LOC_gtblock_clip_height = gtblock_clip_height;
    LOC_gtblock_screen_width = gtblock_screen_width;

    asm volatile (" \
locret1:\n \
    addl    $0x1A0,%%esp\n \
    popa    \n \
"
         : // no outputs
         : "a" (gtb)
         : "memory", "cc");
}
/******************************************************************************/
