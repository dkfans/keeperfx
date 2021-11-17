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

#define TRIANLGLES_COUNT 100000

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Point;

struct Triangle {
  short points[3];
  int tags[3]; // Id of each triangle neighbour of this one
  unsigned char tree_alt; // 255 is a special value here
  unsigned char field_D;
  unsigned short field_E;
};

/******************************************************************************/
extern struct Triangle Triangles[TRIANLGLES_COUNT];
extern long count_Triangles;
extern long ix_Triangles;

#pragma pack()
/******************************************************************************/
extern struct Triangle bad_triangle;
#define INVALID_TRIANGLE &bad_triangle
extern const long MOD3[];
/******************************************************************************/
long tri_new(void);
void tri_dispose(long tri_idx);

TbBool triangle_is_invalid(const struct Triangle *tri);
struct Triangle *get_triangle(long tri_id);
long triangle_find_first_used(void);

long get_triangle_region_id(long tri_id);
TbBool set_triangle_region_id(long tri_id, long reg_id);
long get_triangle_edgelen(long tri_id);
TbBool set_triangle_edgelen(long tri_id, long edgelen);
long get_triangle_tree_alt(long tri_id);
struct Point *get_triangle_point(long tri_id, long pt_cor);
TbBool triangle_tip_equals(long tri_id, long pt_cor, long pt_x, long pt_y);
long link_find(long ntri, long val);
TbBool outer_locked(long ntri, long ncor);

long point_loop(long pt_tri, long pt_cor);
long reduce_point(long *pt_tri, long *pt_cor);
void edgelen_set(long tri_id);
long edge_rotateAC(long tri1_id, long cor1_id);

void triangulation_init_triangles(long pt_id1, long pt_id2, long pt_id3, long pt_id4);
char triangle_divide_areas_s8differ(long ntri, long ncorA, long ncorB, long pt_x, long pt_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
