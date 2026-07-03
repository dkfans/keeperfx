/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_render.h
 *     Header file for bflib_render.c and bflib_render_*.c.
 * @par Purpose:
 *     Rendering the 3D view functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_REND_H
#define BFLIB_REND_H

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum VecModes {
    VM_FlatColor = 0,
    VM_Unusedparam1,
    VM_TriangularGouraud,
    VM_TriangularTexture,
    VM_QuadFlatColor,
    VM_QuadTextured,
    VM_TriangularTextured,
    VM_SolidColor,
    VM_Unusedparam8,
    VM_Unusedparam9,
    VM_SpriteTranslucent,
    VM_Unusedparam11,
    VM_Unusedparam12,
    VM_Unusedparam13,
    VM_Unusedparam14,
    VM_Unusedparam15,
    VM_Unusedparam16,
    VM_Unusedparam17,
    VM_Unusedparam18,
    VM_Unusedparam19,
    VM_Unusedparam20,
    VM_Unusedparam21,
    VM_Unusedparam22,
    VM_Unusedparam23,
    VM_Unusedparam24,
    VM_Unusedparam25,
    VM_Unusedparam26,
    VM_Unusedparam27,
};


// These are used "per screen row"
struct PolyPoint {
    long X; // Horizontal coordinate within screen buffer
    long Y; // Vertical coordinate within screen buffer
    long U; // Texture UV mapping, U coordinate
    long V; // Texture UV mapping, V coordinate
    long S; // Shininess / brightness of the point
};

struct GtBlock { // sizeof = 48
  unsigned char *texturedata;
  unsigned long width;
  unsigned long height;
  unsigned long lightness0;
  unsigned long lightness1;
  unsigned long lightness3;
  unsigned long lightness2;
  unsigned long texturestride;
  unsigned long scalingfactor;
  unsigned long colorformat;
  unsigned long renderflags;
  unsigned long textureoffset;
};

/******************************************************************************/

#pragma pack()
/******************************************************************************/
extern TbPixel vec_colour;
extern unsigned char vec_mode;
extern unsigned char *render_fade_tables;
extern unsigned char *render_ghost;
extern unsigned char *render_alpha;
extern struct PolyPoint *polyscans;
// Rename pending for these entries
extern unsigned char *LOC_poly_screen;
extern unsigned char *LOC_vec_map;
extern unsigned char *LOC_vec_screen;
extern long LOC_vec_screen_width;
extern long LOC_vec_window_width;
extern long LOC_vec_window_height;
/******************************************************************************/
void draw_gpoly(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c);
/******************************************************************************/
void gtblock_set_clipping_window(unsigned char *screen_addr, long clip_width, long clip_height, long screen_width);
/******************************************************************************/
void trig(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c);
/******************************************************************************/
void setup_bflib_render();
void reset_bflib_render();
void finish_bflib_render();

#ifdef __cplusplus
}
#endif
#endif
