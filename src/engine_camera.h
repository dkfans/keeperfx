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
struct Packet;
struct Thing;

// Camera constants; zoom max is zoomed in (everything large), zoom min is zoomed out (everything small)
#define CAMERA_ZOOM_MAX 12000
#define CAMERA_ZOOM_MIN 520 // Originally 4100, adjusted for view distance
#define FRONTVIEW_CAMERA_ZOOM_MAX 65536
#define FRONTVIEW_CAMERA_ZOOM_MIN 3000 // Originally 16384, adjusted for view distance
#define MINMAX_LENGTH 512 // Originally 64, adjusted for view distance
#define MINMAX_ALMOST_HALF ((MINMAX_LENGTH/2)-1)
#define CAMERA_TILT_DEFAULT -266
#define CAMERA_TILT_MIN -350
#define CAMERA_TILT_MAX -200

extern long zoom_distance_setting; // CFG setting
extern long frontview_zoom_distance_setting; // CFG setting

enum CameraIndexValues {
    CamIV_Isometric = 0,
    CamIV_FirstPerson,
    CamIV_Parchment,
    CamIV_FrontView,
    CamIV_EndList
};

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
    unsigned char view_mode;
    int rotation_angle_x;
    int rotation_angle_y;
    int rotation_angle_z;
    int horizontal_fov; // Horizontal Field of View in degrees
    int zoom;
    int inertia_rotation;
    TbBool in_active_movement_rotation;
    long inertia_x;
    TbBool in_active_movement_x;
    long inertia_y;
    TbBool in_active_movement_y;
};


/******************************************************************************/

extern struct EngineCoord object_origin;

#pragma pack()
/******************************************************************************/
extern long camera_zoom;
/******************************************************************************/
void angles_to_vector(short theta, short phi, long dist, struct ComponentVector *cvect);
long get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
long get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
MapCoordDelta get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
MapCoordDelta get_2d_distance_squared(const struct Coord3d *pos1, const struct Coord3d *pos2);
long get_angle_xy_to_vec(const struct CoordDelta3d *vec);
long get_angle_yz_to_vec(const struct CoordDelta3d *vec);
void project_point_to_wall_on_angle(const struct Coord3d *pos1, struct Coord3d *pos2, long angle_xy, long angle_z, long distance, long num_steps);

void view_zoom_camera_in(struct Camera *cam, long limit_max, long limit_min);
void set_camera_zoom(struct Camera *cam, long val);
void view_zoom_camera_out(struct Camera *cam, long limit_max, long limit_min);
long get_camera_zoom(struct Camera *cam);
unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl);
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min);

void view_set_camera_y_inertia(struct Camera *cam, long delta, long ilimit);
void view_set_camera_x_inertia(struct Camera *cam, long delta, long ilimit);
void view_set_camera_rotation_inertia(struct Camera *cam, long delta, long ilimit);
void view_set_camera_tilt(struct Camera *cam, unsigned char mode);
void view_process_camera_inertia(struct Camera *cam);

void update_all_players_cameras(void);
void init_player_cameras(struct PlayerInfo *player);
void update_first_person_position(struct Camera *cam, struct Thing *thing, int eye_height);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
