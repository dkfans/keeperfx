/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_tringls.h
 *     Header file for ariadne_tringls.c.
 * @par Purpose:
 *     Triangles array for Ariadne system support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 22 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_TRINGLS_H
#define DK_ARIADNE_TRINGLS_H

#include "bflib_basics.h"
#include "globals.h"

#define TRIANLGLES_COUNT 9000

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Point;

struct Triangle { // sizeof = 16
  short points[3];
  short tags[3];
  unsigned char tree_alt; // 255 is a special value here
  unsigned char field_D;
  unsigned short field_E;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT struct Triangle _DK_Triangles[TRIANLGLES_COUNT];
#define Triangles _DK_Triangles
DLLIMPORT long _DK_count_Triangles;
#define count_Triangles _DK_count_Triangles
DLLIMPORT long _DK_ix_Triangles;
#define ix_Triangles _DK_ix_Triangles
/******************************************************************************/
extern struct Triangle bad_triangle;
#define INVALID_TRIANGLE &bad_triangle;
extern const long MOD3[];
/******************************************************************************/
long tri_new(void);
void tri_dispose(long tri_idx);

long get_triangle_region_id(long tri_id);
TbBool set_triangle_region_id(long tri_id, long reg_id);
long get_triangle_edgelen(long tri_id);
TbBool set_triangle_edgelen(long tri_id, long edgelen);
long get_triangle_tree_alt(long tri_id);
struct Point *get_triangle_point(long tri_id, long pt_cor);
TbBool triangle_tip_equals(long tri_id, long pt_cor, long pt_x, long pt_y);
struct Triangle *get_triangle(long tri_id);
long link_find(long ntri, long val);
TbBool outer_locked(long ntri, long ncor);
long edge_rotateAC(long a1, long a2);

long point_loop(long pt_tri, long pt_cor);
long reduce_point(long *pt_tri, long *pt_cor);

void triangulation_init(void);
void triangulation_initxy(long a1, long a2, long a3, long a4);
char triangle_divide_areas_s8differ(long ntri, long ncorA, long ncorB, long pt_x, long pt_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
