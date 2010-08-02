/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_edge.h
 *     Header file for ariadne_edge.c.
 * @par Purpose:
 *     Ariadne system edge support functions.
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
#ifndef DK_ARIADNE_EDGE_H
#define DK_ARIADNE_EDGE_H

#include "globals.h"

#define EDGE_POINTS_COUNT 200

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct EdgePoint { //sizeof = 8
    long field_0;
    long field_4;
};

/******************************************************************************/
DLLIMPORT long _DK_ix_EdgePoints;
#define ix_EdgePoints _DK_ix_EdgePoints

#pragma pack()
/******************************************************************************/
void edge_points_clean(void);
long edge_point_add(long pt_x, long pt_y);
struct EdgePoint *edge_point_get(long ept_id);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
