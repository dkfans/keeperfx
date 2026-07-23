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
  NavColour tree_alt; // NAV_COL_UNSET is a special value here
  unsigned char navigation_flags;
  unsigned short region_and_edgelen;
};

#define NAV_COL_UNSET USHRT_MAX

/******************************************************************************/
extern struct Triangle Triangles[TRIANLGLES_COUNT];
extern int32_t count_Triangles;
extern int32_t ix_Triangles;

#pragma pack()
/******************************************************************************/
extern struct Triangle bad_triangle;
#define INVALID_TRIANGLE &bad_triangle
extern const int32_t MOD3[];
/******************************************************************************/
int32_t tri_new(void);
void tri_dispose(int32_t tri_idx);

TbBool triangle_is_invalid(const struct Triangle *tri);
struct Triangle *get_triangle(int32_t tri_id);
int32_t triangle_find_first_used(void);

int32_t get_triangle_region_id(int32_t tri_id);
TbBool set_triangle_region_id(int32_t tri_id, int32_t reg_id);
int32_t get_triangle_edgelen(int32_t tri_id);
TbBool set_triangle_edgelen(int32_t tri_id, int32_t edgelen);
NavColour get_triangle_tree_alt(int32_t tri_id);
struct Point *get_triangle_point(int32_t tri_id, int32_t pt_cor);
TbBool triangle_tip_equals(int32_t tri_id, int32_t pt_cor, int32_t pt_x, int32_t pt_y);
int32_t link_find(int32_t ntri, int32_t val);
TbBool outer_locked(int32_t ntri, int32_t ncor);

int32_t point_loop(int32_t pt_tri, int32_t pt_cor);
int32_t reduce_point(int32_t *pt_tri, int32_t *pt_cor);
void edgelen_set(int32_t tri_id);
int32_t edge_rotateAC(int32_t tri1_id, int32_t cor1_id);

void triangulation_init_triangles(int32_t pt_id1, int32_t pt_id2, int32_t pt_id3, int32_t pt_id4);
char triangle_divide_areas_s8differ(int32_t ntri, int32_t ncorA, int32_t ncorB, int32_t pt_x, int32_t pt_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
