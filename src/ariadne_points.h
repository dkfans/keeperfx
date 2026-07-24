/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_points.h
 *     Header file for ariadne_points.c.
 * @par Purpose:
 *     ariadne_points functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 22 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_POINTS_H
#define DK_ARIADNE_POINTS_H

#include "bflib_basics.h"
#include "globals.h"

#define POINTS_COUNT 30000

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

typedef int32_t AridPointId;

struct Point { // sizeof = 4
  short x;
  short y;
};

/******************************************************************************/
extern struct Point ari_Points[];

#pragma pack()
/******************************************************************************/
#define INVALID_POINT (&ari_Points[0])
/******************************************************************************/
void point_dispose(AridPointId pt_id);
TbBool point_set(AridPointId pt_id, int32_t x, int32_t y);
struct Point *point_get(AridPointId pt_id);
TbBool point_is_invalid(const struct Point *pt);
TbBool point_equals(AridPointId pt_idx, int32_t pt_x, int32_t pt_y);
AridPointId point_set_new_or_reuse(int32_t pt_x, int32_t pt_y);
void triangulation_initxy_points(int32_t startx, int32_t starty, int32_t endx, int32_t endy);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
