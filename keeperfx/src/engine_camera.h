/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_camera.h
 *     Header file for engine_camera.c.
 * @par Purpose:
 *     Camera move, maintain and support functions.
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

#ifndef DK_ENGNCAM_H
#define DK_ENGNCAM_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct EngineCoord;
struct M33;
struct EngineCol;

struct MinMax { // sizeof = 8
    long min;
    long max;
};

struct ComponentVector {
    short x;
    short y;
    short z;
};

struct Camera {
    struct Coord3d mappos;
    unsigned char field_6;
    int orient_a;
    int orient_b;
    int orient_c;
    int field_13;
    int field_17;
    int field_1B;
    unsigned char field_1F[9];
    short field_28;
};

#ifdef __cplusplus
#pragma pack()
#endif

/******************************************************************************/

/******************************************************************************/
DLLIMPORT extern struct M33 _DK_camera_matrix;
#define camera_matrix _DK_camera_matrix
DLLIMPORT extern struct EngineCoord _DK_object_origin;
#define object_origin _DK_object_origin
DLLIMPORT extern struct MinMax _DK_minmaxs[];
#define minmaxs _DK_minmaxs
/******************************************************************************/
long get_3d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
long get_2d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
void angles_to_vector(short theta, short phi, long dist, struct ComponentVector *cvect);
long get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
long get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
long get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
void project_point_to_wall_on_angle(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4, long a5, long a6);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
