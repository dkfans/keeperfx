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
#pragma pack(1)

struct EngineCoord;
struct M33;
struct EngineCol;
struct PlayerInfo;

// Camera constants; max zoom is when everything is large
#define CAMERA_ZOOM_MIN     4100
#define CAMERA_ZOOM_MAX    12000
#define MINMAX_LENGTH         64

#define CAMERA_VIEW_EMPTY       2
#define CAMERA_VIEW_CREATURE    1
#define CAMERA_VIEW_ISOMETRIC   3
#define CAMERA_VIEW_PARCHMENT   5

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
    // Location of camera on map
    struct Coord3d mappos;

    // View type of camera, was field_6
    unsigned char viewType;

    int orient_a;
    int orient_b;
    int orient_c;

    int field_13;
    int zoom;

    // Delta of camera horizontal angle, was field_1B.
    int rotationDelta;
    // Whether apply interia effect when rotating camera, was field_1F (Meant different thing then).
    unsigned char rotationInteriaOn;

    // Delta of camera horizontal position, was field_20.
    long horizontalPosDelta;
    // Whether apply interia effect when moving camera horizontally, was field_24 (Meant different thing then).
    unsigned char horizontalInteriaOn;

    // Delta of camera vertical position, was field_25.
    long verticalPosDelta;
    // Whether apply interia effect when moving camera vertically, was field_29 (Meant different thing then).
    unsigned char verticalInteriaOn;
};

#pragma pack()
/******************************************************************************/

extern long camera_zoom;
/******************************************************************************/
DLLIMPORT extern struct M33 _DK_camera_matrix;
#define camera_matrix _DK_camera_matrix
DLLIMPORT extern struct EngineCoord _DK_object_origin;
#define object_origin _DK_object_origin
DLLIMPORT extern struct MinMax _DK_minmaxs[MINMAX_LENGTH];
#define minmaxs _DK_minmaxs
/******************************************************************************/
MapCoordDelta get_3d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
MapCoordDelta get_2d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
MapCoordDelta get_2d_box_distance_xy(long pos1_x, long pos1_y, long pos2_x, long pos2_y);
void angles_to_vector(short theta, short phi, long dist, struct ComponentVector *cvect);
long get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
long get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
MapCoordDelta get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
MapCoordDelta get_2d_distance_squared(const struct Coord3d *pos1, const struct Coord3d *pos2);
long get_angle_xy_to_vec(const struct CoordDelta3d *vec);
long get_angle_yz_to_vec(const struct CoordDelta3d *vec);
void project_point_to_wall_on_angle(const struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4, long a5, long a6);

void view_zoom_camera_in(struct Camera *cam, long limit_max, long limit_min);
void set_camera_zoom(struct Camera *cam, long val);
void view_zoom_camera_out(struct Camera *cam, long limit_max, long limit_min);
long get_camera_zoom(struct Camera *cam);
unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl);
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min);

void view_set_camera_y_inertia(struct Camera *cam, long a2, long a3, int interiaOn);
void view_set_camera_x_inertia(struct Camera *cam, long a2, long a3, int interiaOn);
void view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3, int interiaOn);

void init_player_cameras(struct PlayerInfo *player);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
