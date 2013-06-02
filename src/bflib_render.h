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

#define POLY_SCANS_COUNT 576

enum VecModes {
    VM_Unknown0 = 0,
    VM_Unknown1,
    VM_Unknown2,
    VM_Unknown3,
    VM_Unknown4,
    VM_Unknown5,
    VM_Unknown6,
    VM_Unknown7,
    VM_Unknown8,
    VM_Unknown9,
    VM_Unknown10,
    VM_Unknown11,
    VM_Unknown12,
    VM_Unknown13,
    VM_Unknown14,
    VM_Unknown15,
    VM_Unknown16,
    VM_Unknown17,
    VM_Unknown18,
    VM_Unknown19,
    VM_Unknown20,
    VM_Unknown21,
    VM_Unknown22,
    VM_Unknown23,
    VM_Unknown24,
    VM_Unknown25,
    VM_Unknown26,
    VM_Unknown27,
};


struct PolyPoint { // sizeof = 20
  long field_0;
  long field_4;
  long field_8;
  long field_C;
  long field_10;
};

struct GtBlock { // sizeof = 48
  unsigned char *field_0;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C;
  unsigned long field_20;
  unsigned long field_24;
  unsigned long field_28;
  unsigned long field_2C;
};

/******************************************************************************/
DLLIMPORT unsigned char _DK_vec_mode;
//#define vec_mode _DK_vec_mode
DLLIMPORT unsigned char _DK_vec_colour;
//#define vec_colour _DK_vec_colour

#pragma pack()
/******************************************************************************/
extern TbPixel vec_colour;
extern unsigned char vec_mode;
extern unsigned char *render_fade_tables;
extern unsigned char *render_ghost;
extern unsigned char *render_alpha;
extern struct PolyPoint polyscans[2*POLY_SCANS_COUNT];
// Rename pending for these entries
extern unsigned char *LOC_poly_screen;
extern unsigned char *LOC_vec_map;
extern unsigned char *LOC_vec_screen;
extern unsigned long LOC_vec_screen_width;
extern unsigned long LOC_vec_window_width;
extern unsigned long LOC_vec_window_height;
/******************************************************************************/
void draw_triangle(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c);
void draw_quad(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c, struct PolyPoint *point_d);
/******************************************************************************/
void gpoly_enable_pentium_pro(TbBool state);
void draw_gpoly(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c);
/******************************************************************************/
void gtblock_set_clipping_window(unsigned char *screen_addr, long clip_width, long clip_height, long screen_width);
void gtblock_draw(struct GtBlock *gtb);
/******************************************************************************/
void trig(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
