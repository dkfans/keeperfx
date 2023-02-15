/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_camera.c
 *     Camera move, maintain and support functions.
 * @par Purpose:
 *     Defines and maintains cameras.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "engine_camera.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_planar.h"

#include "engine_lenses.h"
#include "engine_render.h"
#include "vidmode.h"
#include "map_blocks.h"
#include "dungeon_data.h"
#include "config_settings.h"
#include "player_instances.h"
#include "frontmenu_ingame_map.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
int zoom_speed_near = 15; // Default value [1-100]. Set by CFG.
int zoom_speed_far = 15; // Default value [1-100]. Set by CFG.
long zoom_distance_setting;
long frontview_zoom_distance_setting;
long camera_zoom;

long previous_cam_mappos_x;
long previous_cam_mappos_y;
long previous_cam_mappos_z;
long interpolated_cam_mappos_x;
long interpolated_cam_mappos_y;
long interpolated_cam_mappos_z;
long previous_cam_orient_a;
long previous_cam_orient_b;
long previous_cam_orient_c;
long interpolated_cam_orient_a;
long interpolated_cam_orient_b;
long interpolated_cam_orient_c;
long previous_camera_zoom;
long interpolated_camera_zoom;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

static void calculate_zoomed_percentage(struct Camera *cam) {
	// This function is performed once per turn, unlike calculate_hud_scale which is performed each rendering frame
    switch (cam->view_mode) {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            cam->zoomed_percent = normalize_within_range(cam->zoom, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);
            break;
        case PVM_FrontView:
            cam->zoomed_percent = normalize_within_range(cam->zoom, FRONTVIEW_CAMERA_ZOOM_MIN, FRONTVIEW_CAMERA_ZOOM_MAX);
            break;
        default:
            cam->zoomed_percent = 0;
            return;
    }
}

// Instantly move camera when going from parchment view to main view
void reset_interpolation_for_parchment_view(struct PlayerInfo* player)
{
    struct Camera *cam = player->acamera;
    interpolated_cam_orient_a = cam->orient_a;
    interpolated_cam_orient_c = cam->orient_c;
    previous_cam_orient_a = cam->orient_a;
    previous_cam_orient_c = cam->orient_c;
    interpolated_cam_mappos_x = cam->mappos.x.val;
    interpolated_cam_mappos_y = cam->mappos.y.val;
    interpolated_cam_mappos_z = cam->mappos.z.val;
    previous_cam_mappos_x = cam->mappos.x.val;
    previous_cam_mappos_y = cam->mappos.y.val;
    previous_cam_mappos_z = cam->mappos.z.val;
    reset_all_minimap_interpolation = true; // Stops minimap from smoothly moving to the new position upon exiting Parchment View
}

void reset_interpolation_of_camera(struct PlayerInfo* player)
{
    struct Camera *cam = player->acamera;
    interpolated_camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    previous_camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    interpolated_cam_orient_a = cam->orient_a;
    interpolated_cam_orient_b = cam->orient_b;
    interpolated_cam_orient_c = cam->orient_c;
    previous_cam_orient_a = cam->orient_a;
    previous_cam_orient_b = cam->orient_b;
    previous_cam_orient_c = cam->orient_c;
    interpolated_cam_mappos_x = cam->mappos.x.val;
    interpolated_cam_mappos_y = cam->mappos.y.val;
    interpolated_cam_mappos_z = cam->mappos.z.val;
    previous_cam_mappos_x = cam->mappos.x.val;
    previous_cam_mappos_y = cam->mappos.y.val;
    previous_cam_mappos_z = cam->mappos.z.val;
    reset_all_minimap_interpolation = true;
}

void set_previous_camera_values(struct PlayerInfo* player) {
    

    // Used for interpolation mainly
    struct Camera *cam = player->acamera;
    previous_cam_mappos_x = cam->mappos.x.val;
    previous_cam_mappos_y = cam->mappos.y.val;
    previous_cam_mappos_z = cam->mappos.z.val;
    previous_cam_orient_a = cam->orient_a;
    previous_cam_orient_b = cam->orient_b;
    previous_cam_orient_c = cam->orient_c;
    previous_camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    if (game.frame_skip > 0)
    {
        reset_interpolation_of_camera(player); // Stop camera from being laggy while frameskipping
    }
    
    cam->previous_zoomed_percent = cam->zoomed_percent;
}

MapCoordDelta get_3d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist_y = abs(pos2->y.val - (long)pos1->y.val);
    long dist_x = abs(pos2->x.val - (long)pos1->x.val);
    if (dist_y <= dist_x)
        dist_y = dist_x;
    long dist_z = abs(pos2->z.val - (long)pos1->z.val);
    if (dist_y <= dist_z)
        dist_y = dist_z;
    return dist_y;
}

MapCoordDelta get_2d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist_y = abs((long)pos1->y.val - (long)pos2->y.val);
    long dist_x = abs((long)pos1->x.val - (long)pos2->x.val);
    if (dist_y <= dist_x)
        return dist_x;
    return dist_y;
}

MapCoordDelta get_2d_box_distance_xy(long pos1_x, long pos1_y, long pos2_x, long pos2_y)
{
    long dist_x = abs((long)pos1_x - (long)pos2_x);
    long dist_y = abs((long)pos1_y - (long)pos2_y);
    if (dist_y <= dist_x)
      return dist_x;
    return dist_y;
}

void angles_to_vector(short angle_xy, short angle_yz, long dist, struct ComponentVector *cvect)
{
    long long cos_yz = LbCosL(angle_yz) >> 2;
    long long sin_yz = LbSinL(angle_yz) >> 2;
    long long cos_xy = LbCosL(angle_xy) >> 2;
    long long sin_xy = LbSinL(angle_xy) >> 2;
    long long lldist = dist;
    long long mag = (lldist << 14) - cos_yz;
    long long factor = sin_xy * mag;
    cvect->x = (factor >> 14) >> 14;
    factor = cos_xy * mag;
    cvect->y = -(factor >> 14) >> 14;
    factor = lldist * sin_yz;
    cvect->z = (factor >> 14);
}

long get_angle_xy_to_vec(const struct CoordDelta3d *vec)
{
    return LbArcTanAngle(vec->x.val, vec->y.val) & LbFPMath_AngleMask;
}

long get_angle_yz_to_vec(const struct CoordDelta3d *vec)
{
    long dist = LbDiagonalLength(abs(vec->x.val), abs(vec->y.val));
    return LbArcTanAngle(vec->z.val, dist) & LbFPMath_AngleMask;
}

long get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    return LbArcTanAngle((long)pos2->x.val - (long)pos1->x.val, (long)pos2->y.val - (long)pos1->y.val) & LbFPMath_AngleMask;
}

long get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist = get_2d_distance(pos1, pos2);
    return LbArcTanAngle(pos2->z.val - pos1->z.val, dist) & LbFPMath_AngleMask;
}

// TODO these are actually Coord2d and Coord3d just inherits from it
MapCoordDelta get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist_x = (long)pos1->x.val - (long)pos2->x.val;
    long dist_y = (long)pos1->y.val - (long)pos2->y.val;
    return LbDiagonalLength(abs(dist_x), abs(dist_y));
}

MapCoordDelta get_2d_distance_squared(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist_x = (long)pos1->x.val - (long)pos2->x.val;
    long dist_y = (long)pos1->y.val - (long)pos2->y.val;
    return dist_x * dist_x + dist_y * dist_y;
}

void project_point_to_wall_on_angle(const struct Coord3d *pos1, struct Coord3d *pos2, long angle_xy, long angle_z, long distance, long num_steps)
{
    long dx = distance_with_angle_to_coord_x(distance, angle_xy);
    long dy = distance_with_angle_to_coord_y(distance, angle_xy);
    long dz = distance_with_angle_to_coord_z(distance, angle_z);
    struct Coord3d pos;
    pos.x.val = pos1->x.val;
    pos.y.val = pos1->y.val;
    pos.z.val = pos1->z.val;
    // Do num_steps until a solid wall is reached
    for (long n = num_steps; n > 0; n--)
    {
        if (point_in_map_is_solid(&pos))
            break;
        pos.x.val += dx;
        pos.y.val += dy;
        pos.z.val += dz;
    }
    pos2->x.val = pos.x.val;
    pos2->y.val = pos.y.val;
    pos2->z.val = pos.z.val;
}

int calculate_zoom_rate(struct Camera *cam)
{
    // If playing multiplayer then return original 85 value, because otherwise desyncs can occur if two players have different cfg values
    if (game.game_kind != GKind_LocalGame)
    {
        return 85;
    }

    int easing = (1.0-pow(cam->zoomed_percent, 0.25)) * 100;
    int calc_rate = integer_lerp(zoom_speed_near, zoom_speed_far, easing);
    calc_rate = 100 - clamp(calc_rate, 1, 99);
    return calc_rate;
}

void view_zoom_camera_in(struct Camera *cam, long limit_max, long limit_min)
{
    long new_zoom;
    long old_zoom = get_camera_zoom(cam);

    int zoom_rate = calculate_zoom_rate(cam);

    switch (cam->view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        new_zoom = (100 * old_zoom) / zoom_rate;
        if (new_zoom == old_zoom)
            new_zoom++;
        if (new_zoom < limit_min) {
            new_zoom = limit_min;
        } else
        if (new_zoom > limit_max) {
            new_zoom = limit_max;
        }
        break;
    case PVM_ParchmentView:
        new_zoom = (5 * old_zoom) / 4;
        if (new_zoom == old_zoom)
            new_zoom++;
        if (new_zoom < 16) {
            new_zoom = 16;
        } else
        if (new_zoom > 1024) {
            new_zoom = 1024;
        }
        break;
    case PVM_FrontView:
        new_zoom = (100 * old_zoom) / zoom_rate;
        if (new_zoom == old_zoom)
            new_zoom++;
        if (new_zoom < FRONTVIEW_CAMERA_ZOOM_MIN) { //Originally 16384, adjusted for view distance
            new_zoom = FRONTVIEW_CAMERA_ZOOM_MIN;
        } else
        if (new_zoom > FRONTVIEW_CAMERA_ZOOM_MAX) {
            new_zoom = FRONTVIEW_CAMERA_ZOOM_MAX;
        }
        break;
    default:
        new_zoom = old_zoom;
    }
    set_camera_zoom(cam, new_zoom);
}

void set_camera_zoom(struct Camera *cam, long new_zoom)
{
    if (cam == NULL)
      return;
    switch (cam->view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_FrontView:
    case PVM_IsoStraightView:
        cam->zoom = new_zoom;
        break;
    case PVM_ParchmentView:
        cam->mappos.z.val = new_zoom;
        break;
    }
    calculate_zoomed_percentage(cam);
}

void view_zoom_camera_out(struct Camera *cam, long limit_max, long limit_min)
{
    long new_zoom;
    long old_zoom = get_camera_zoom(cam);
    
    int zoom_rate = calculate_zoom_rate(cam);

    switch (cam->view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        new_zoom = (zoom_rate * old_zoom) / 100;
        if (new_zoom == old_zoom)
            new_zoom--;
        if (new_zoom < limit_min) {
            new_zoom = limit_min;
        } else
        if (new_zoom > limit_max) {
            new_zoom = limit_max;
        }
        break;
    case PVM_ParchmentView:
        new_zoom = (4 * old_zoom) / 5;
        if (new_zoom == old_zoom)
            new_zoom--;
        if (new_zoom < 16) {
            new_zoom = 16;
        } else
        if (new_zoom > 1024) {
            new_zoom = 1024;
        }
        break;
    case PVM_FrontView:
        new_zoom = (zoom_rate * old_zoom) / 100;
        if (new_zoom == old_zoom)
            new_zoom--;
        if (new_zoom < max(FRONTVIEW_CAMERA_ZOOM_MIN, frontview_zoom_distance_setting)) {
            new_zoom = max(FRONTVIEW_CAMERA_ZOOM_MIN, frontview_zoom_distance_setting);
        } else
        if (new_zoom > FRONTVIEW_CAMERA_ZOOM_MAX) {
            new_zoom = FRONTVIEW_CAMERA_ZOOM_MAX;
        }
        break;
    default:
        new_zoom = old_zoom;
    }
    set_camera_zoom(cam, new_zoom);
}

/**
 * Conducts clipping to zoom level of given camera, based on current screen mode.
 */
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min)
{
    SYNCDBG(7,"Starting");
    long zoom_val = get_camera_zoom(cam);
    if (zoom_val < zoom_min)
    {
      zoom_val = zoom_min;
    } else
    if (zoom_val > zoom_max)
    {
      zoom_val = zoom_max;
    }
    set_camera_zoom(cam, zoom_val);
}

long get_camera_zoom(struct Camera *cam)
{
    if (cam == NULL)
      return 0;
    switch (cam->view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_FrontView:
    case PVM_IsoStraightView:
        return cam->zoom;
    case PVM_ParchmentView:
        return cam->mappos.z.val;
    default:
        return 0;
    }
}


/** Scales camera zoom for current screen resolution.
 *
 * @param zoom_lvl Unscaled zoom level.
 * @return Zoom level scaled with use of current units_per_pixel value.
 */
unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl)
{
    return scale_fixed_DK_value(zoom_lvl);
}

void view_set_camera_y_inertia(struct Camera *cam, long delta, long ilimit)
{
    long abslimit = abs(ilimit);
    cam->inertia_y += delta;
    if (cam->inertia_y < -abslimit) {
        cam->inertia_y = -abslimit;
    } else
    if (cam->inertia_y > abslimit) {
        cam->inertia_y = abslimit;
    }
    cam->in_active_movement_y = true;
}

void view_set_camera_x_inertia(struct Camera *cam, long delta, long ilimit)
{
    long abslimit = abs(ilimit);
    cam->inertia_x += delta;
    if (cam->inertia_x < -abslimit) {
        cam->inertia_x = -abslimit;
    } else
    if (cam->inertia_x > abslimit) {
        cam->inertia_x = abslimit;
    }
    cam->in_active_movement_x = true;
}

void view_set_camera_rotation_inertia(struct Camera *cam, long delta, long ilimit)
{
    int limit_val = abs(ilimit);
    int new_val = delta + cam->inertia_rotation;
    cam->inertia_rotation = new_val;
    if (new_val < -limit_val)
    {
        cam->inertia_rotation = -limit_val;
        cam->in_active_movement_rotation = true;
    } else
    if (new_val > limit_val)
    {
        cam->inertia_rotation = limit_val;
        cam->in_active_movement_rotation = true;
    } else
    {
        cam->in_active_movement_rotation = true;
    }
}

void init_player_cameras(struct PlayerInfo *player)
{
    struct Thing* heartng = get_player_soul_container(player->id_number);
    struct Camera* cam = &player->cameras[CamIV_FirstPerson];
    cam->mappos.x.val = 0;
    cam->mappos.y.val = 0;
    cam->mappos.z.val = 256;
    cam->orient_b = 0;
    cam->orient_c = 0;
    cam->horizontal_fov = first_person_horizontal_fov;
    cam->orient_a = LbFPMath_PI/2;
    cam->view_mode = PVM_CreatureView;

    cam = &player->cameras[CamIV_Isometric];
    cam->mappos.x.val = heartng->mappos.x.val;
    cam->mappos.y.val = heartng->mappos.y.val;
    cam->mappos.z.val = 0;
    cam->orient_c = 0;
    cam->horizontal_fov = 94;
    cam->orient_b = -266;
    cam->orient_a = LbFPMath_PI/4;
    if (settings.video_rotate_mode == 1) {
        cam->view_mode = PVM_IsoStraightView;
    } else {
        cam->view_mode = PVM_IsoWibbleView;
    }
    cam->zoom = settings.isometric_view_zoom_level;

    cam = &player->cameras[CamIV_Parchment];
    cam->mappos.x.val = 0;
    cam->mappos.y.val = 0;
    cam->mappos.z.val = 32;
    cam->horizontal_fov = 94;
    cam->view_mode = PVM_ParchmentView;

    cam = &player->cameras[CamIV_FrontView];
    cam->mappos.x.val = heartng->mappos.x.val;
    cam->mappos.y.val = heartng->mappos.y.val;
    cam->mappos.z.val = 32;
    cam->horizontal_fov = 94;
    cam->view_mode = PVM_FrontView;
    cam->zoom = settings.frontview_zoom_level;

    reset_interpolation_of_camera(player);
    
    // setup previous_zoomed_percent
    calculate_zoomed_percentage(cam);
    cam->previous_zoomed_percent = cam->zoomed_percent;
}

static int get_walking_bob_direction(struct Thing *thing)
{
    const int anim_time = thing->anim_time;
    if ( anim_time >= 256 && anim_time < 640 )
    {
        return ( thing->anim_speed < 0 ) ? -1 : 1;
    }
    else if ( anim_time >= 1024 && anim_time < 1408 )
    {
        return ( thing->anim_speed < 0 ) ? -1 : 1;
    }
    else
    {
        return ( thing->anim_speed < 0 ) ? 1 : -1;
    }
}

void update_player_camera_fp(struct Camera *cam, struct Thing *thing)
{
    int eye_height = get_creature_eye_height(thing);

    if ( thing_is_creature(thing) )
    {
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        // apply square wave as head bob motion, could be improved by using sine wave instead
        if ( cctrl->move_speed && thing->floor_height >= thing->mappos.z.val )
            cctrl->head_bob = 16 * get_walking_bob_direction(thing);
        else
            cctrl->head_bob = 0;

        int pos_x = move_coord_with_angle_x(thing->mappos.x.val,-90,thing->move_angle_xy);
        int pos_y = move_coord_with_angle_y(thing->mappos.y.val,-90,thing->move_angle_xy);

        if ( pos_x >= 0 )
        {
            if ( pos_x > gameadd.map_subtiles_x * COORD_PER_STL )
                pos_x = gameadd.map_subtiles_x * COORD_PER_STL - 1;
        }
        else
        {
            pos_x = 0;
        }
        if ( pos_y >= 0 )
        {
            if ( pos_y > gameadd.map_subtiles_y * COORD_PER_STL )
                pos_y = gameadd.map_subtiles_y * COORD_PER_STL - 1;
        }
        else
        {
            pos_y = 0;
        }

        cam->mappos.x.val = pos_x;
        cam->mappos.y.val = pos_y;


        if ( (thing->movement_flags & TMvF_Flying) != 0 )
        {
            cam->mappos.z.val = thing->mappos.z.val + eye_height;
            cam->orient_a = thing->move_angle_xy;
            cam->orient_b = thing->move_angle_z;
            cam->orient_c = cctrl->field_CC;
        }
        else
        {
            cam->mappos.z.val = cam->mappos.z.val + (thing->mappos.z.val + cctrl->head_bob - cam->mappos.z.val + eye_height) / 2;
            cam->orient_a = thing->move_angle_xy;
            cam->orient_b = thing->move_angle_z;
            cam->orient_c = 0;
            if ( eye_height + thing->mappos.z.val <= cam->mappos.z.val )
            {
                if ( eye_height + thing->mappos.z.val + cctrl->head_bob > cam->mappos.z.val )
                    cam->mappos.z.val = eye_height + thing->mappos.z.val + cctrl->head_bob;
            }
            else
            {
                if ( eye_height + thing->mappos.z.val + cctrl->head_bob < cam->mappos.z.val )
                    cam->mappos.z.val = eye_height + thing->mappos.z.val + cctrl->head_bob;
            }
        }

        struct Map* mapblk1 = get_map_block_at(thing->mappos.x.stl.num,     thing->mappos.y.stl.num);
        struct Map* mapblk2 = get_map_block_at(thing->mappos.x.stl.num + 1, thing->mappos.y.stl.num);
        struct Map* mapblk3 = get_map_block_at(thing->mappos.x.stl.num,     thing->mappos.y.stl.num + 1);
        struct Map* mapblk4 = get_map_block_at(thing->mappos.x.stl.num + 1, thing->mappos.y.stl.num + 1);


        const int ceiling = ((get_mapblk_filled_subtiles(mapblk1) * COORD_PER_STL) +
                          (get_mapblk_filled_subtiles(mapblk2) * COORD_PER_STL) +
                          (get_mapblk_filled_subtiles(mapblk3) * COORD_PER_STL) +
                          (get_mapblk_filled_subtiles(mapblk4) * COORD_PER_STL) )/4;

        if ( cam->mappos.z.val > ceiling - 64 )
            cam->mappos.z.val = ceiling - 64;

    }
    else
    {
        cam->mappos.x.val = thing->mappos.x.val;
        cam->mappos.y.val = thing->mappos.y.val;
        if ( thing_is_mature_food(thing) )
        {
            cam->mappos.z.val = thing->mappos.z.val + 240;
            cam->orient_a = thing->move_angle_xy;
            cam->orient_c = 0;
            cam->orient_b = thing->move_angle_z;
            thing->move_angle_z = 0;
            if ( thing->food.byte_16 )
            {
                if ( thing->food.byte_16 <= 3 )
                    thing->move_angle_z = -116 * thing->food.byte_16 + 2048;
                else
                    thing->move_angle_z = 116 * thing->food.byte_16 + 1352;
            }
        }
        else
        {
            cam->orient_a = thing->move_angle_xy;
            cam->orient_b = thing->move_angle_z;
            cam->orient_c = 0;
            if ( thing->mappos.z.val + 32 <= cam->mappos.z.val )
            {
                cam->mappos.z.val = cam->mappos.z.val + (thing->mappos.z.val - cam->mappos.z.val + 64) / 2;
                if ( thing->mappos.z.val + 64 > cam->mappos.z.val )
                    cam->mappos.z.val = thing->mappos.z.val + 64;
            }
            else
            {
                cam->mappos.z.val = cam->mappos.z.val + (thing->mappos.z.val - cam->mappos.z.val + 64) / 2;
                if ( thing->mappos.z.val + 64 < cam->mappos.z.val )
                    cam->mappos.z.val = thing->mappos.z.val + 64;
            }
        }
    }
}

void view_move_camera_left(struct Camera *cam, long distance)
{

    int pos_x;
    int pos_y;
    int parchment_pos_x;

    if (
        cam->view_mode == PVM_IsoWibbleView ||
        cam->view_mode == PVM_FrontView ||
        cam->view_mode == PVM_IsoStraightView
    ) {

        pos_x = move_coord_with_angle_x(cam->mappos.x.val,distance,cam->orient_a - LbFPMath_PI/2);
        pos_y = move_coord_with_angle_y(cam->mappos.y.val,distance,cam->orient_a - LbFPMath_PI/2);

        if ( pos_x < 0 )
            pos_x = 0;
        if ( pos_x > gameadd.map_subtiles_x * COORD_PER_STL )
            pos_x = gameadd.map_subtiles_x * COORD_PER_STL - 1;

        if ( pos_y < 0 )
            pos_y = 0;
        if ( pos_y > gameadd.map_subtiles_y * COORD_PER_STL )
            pos_y = gameadd.map_subtiles_y * COORD_PER_STL - 1;

        cam->mappos.x.val = pos_x;
        cam->mappos.y.val = pos_y;
        return;
    }

    else if ( cam->view_mode == PVM_ParchmentView )
    {
        parchment_pos_x = cam->mappos.x.val - distance;

        if ( parchment_pos_x < 0 )
            parchment_pos_x = 0;
        if ( parchment_pos_x > gameadd.map_subtiles_x * COORD_PER_STL )
            parchment_pos_x = gameadd.map_subtiles_x * COORD_PER_STL - 1;

        cam->mappos.x.stl.pos = parchment_pos_x;

    }

}

void view_move_camera_right(struct Camera *cam, long distance)
{
    int pos_x;
    int pos_y;
    int parchment_pos_x;

    if (
        cam->view_mode == PVM_IsoWibbleView ||
        cam->view_mode == PVM_FrontView ||
        cam->view_mode == PVM_IsoStraightView
    ) {

        pos_x = move_coord_with_angle_x(cam->mappos.x.val,distance,cam->orient_a + LbFPMath_PI/2);
        pos_y = move_coord_with_angle_y(cam->mappos.y.val,distance,cam->orient_a + LbFPMath_PI/2);

        if ( pos_x < 0 )
            pos_x = 0;
        if ( pos_x > gameadd.map_subtiles_x * COORD_PER_STL )
            pos_x = gameadd.map_subtiles_x * COORD_PER_STL - 1;

        if ( pos_y < 0 )
            pos_y = 0;
        if ( pos_y > gameadd.map_subtiles_y * COORD_PER_STL )
            pos_y = gameadd.map_subtiles_y * COORD_PER_STL - 1;

        cam->mappos.x.val = pos_x;
        cam->mappos.y.val = pos_y;
        return;
    }

    else if ( cam->view_mode == PVM_ParchmentView )
    {
        parchment_pos_x = cam->mappos.x.val + distance;

        if ( parchment_pos_x < 0 )
            parchment_pos_x = 0;
        if ( parchment_pos_x > gameadd.map_subtiles_x * COORD_PER_STL )
            parchment_pos_x = gameadd.map_subtiles_x * COORD_PER_STL - 1;

        cam->mappos.x.stl.pos = parchment_pos_x;

    }

}

void view_move_camera_up(struct Camera *cam, long distance)
{
    int pos_x;
    int pos_y;
    int parchment_pos_y;

    if (
        cam->view_mode == PVM_IsoWibbleView ||
        cam->view_mode == PVM_FrontView ||
        cam->view_mode == PVM_IsoStraightView
    ) {

        pos_x = move_coord_with_angle_x(cam->mappos.x.val,distance,cam->orient_a);
        pos_y = move_coord_with_angle_y(cam->mappos.y.val,distance,cam->orient_a);

        if ( pos_x < 0 )
            pos_x = 0;
        if ( pos_x > gameadd.map_subtiles_x * COORD_PER_STL )
            pos_x = gameadd.map_subtiles_x * COORD_PER_STL - 1;

        if ( pos_y < 0 )
            pos_y = 0;
        if ( pos_y > gameadd.map_subtiles_y * COORD_PER_STL )
            pos_y = gameadd.map_subtiles_y * COORD_PER_STL - 1;

        cam->mappos.x.val = pos_x;
        cam->mappos.y.val = pos_y;
        return;
    }
    else if ( cam->view_mode == PVM_ParchmentView )
    {
        parchment_pos_y = cam->mappos.y.val - distance;

        if ( parchment_pos_y < 0 )
            parchment_pos_y = 0;
        if ( parchment_pos_y > gameadd.map_subtiles_y * COORD_PER_STL )
            parchment_pos_y = gameadd.map_subtiles_y * COORD_PER_STL - 1;

        cam->mappos.y.stl.pos = parchment_pos_y;

    }
}

void view_move_camera_down(struct Camera *cam, long distance)
{
    int pos_x;
    int pos_y;
    int parchment_pos_y;

    if (
        cam->view_mode == PVM_IsoWibbleView ||
        cam->view_mode == PVM_FrontView ||
        cam->view_mode == PVM_IsoStraightView
    ) {

        pos_x = move_coord_with_angle_x(cam->mappos.x.val,distance,cam->orient_a + LbFPMath_PI);
        pos_y = move_coord_with_angle_y(cam->mappos.y.val,distance,cam->orient_a + LbFPMath_PI);

        if ( pos_x < 0 )
            pos_x = 0;
        if ( pos_x > gameadd.map_subtiles_x * COORD_PER_STL )
            pos_x = gameadd.map_subtiles_x * COORD_PER_STL - 1;

        if ( pos_y < 0 )
            pos_y = 0;
        if ( pos_y > gameadd.map_subtiles_y * COORD_PER_STL )
            pos_y = gameadd.map_subtiles_y * COORD_PER_STL - 1;

        cam->mappos.x.val = pos_x;
        cam->mappos.y.val = pos_y;
        return;
    }

    else if ( cam->view_mode == PVM_ParchmentView )
    {
        parchment_pos_y = cam->mappos.y.val - distance;

        if ( parchment_pos_y < 0 )
            parchment_pos_y = 0;
        if ( parchment_pos_y > gameadd.map_subtiles_y * COORD_PER_STL )
            parchment_pos_y = gameadd.map_subtiles_y * COORD_PER_STL - 1;

        cam->mappos.y.stl.pos = parchment_pos_y;

    }

}

void view_process_camera_inertia(struct Camera *cam)
{
    int i;
    i = cam->inertia_x;
    if (i > 0) {
        view_move_camera_right(cam, abs(i));
    } else
    if (i < 0) {
        view_move_camera_left(cam, abs(i));
    }
    if ( cam->in_active_movement_x ) {
        cam->in_active_movement_x = false;
    } else {
        cam->inertia_x /= 2;
    }
    i = cam->inertia_y;
    if (i > 0) {
        view_move_camera_down(cam, abs(i));
    } else
    if (i < 0) {
        view_move_camera_up(cam, abs(i));
    }
    if (cam->in_active_movement_y) {
        cam->in_active_movement_y = false;
    } else {
        cam->inertia_y /= 2;
    }
    if (cam->inertia_rotation) {
        cam->orient_a = (cam->inertia_rotation + cam->orient_a) & LbFPMath_AngleMask;
    }
    if (cam->in_active_movement_rotation) {
        cam->in_active_movement_rotation = false;
    } else {
        cam->inertia_rotation /= 2;
    }
}

void update_player_camera(struct PlayerInfo *player)
{
    struct Dungeon *dungeon = get_players_dungeon(player);
    struct Camera *cam = player->acamera;

    view_process_camera_inertia(cam);
    switch (cam->view_mode)
    {
    case PVM_CreatureView:
        if (player->controlled_thing_idx > 0) {
            struct Thing *ctrltng;
            ctrltng = thing_get(player->controlled_thing_idx);
            update_player_camera_fp(cam, ctrltng);
        } else
        if (player->instance_num != PI_HeartZoom) {
            ERRORLOG("Cannot go first person without controlling creature");
        }
        break;
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        // correct according to dissassembly
        player->cameras[CamIV_FrontView].mappos.x.val = cam->mappos.x.val;
        player->cameras[CamIV_FrontView].mappos.y.val = cam->mappos.y.val;
        break;
    case PVM_FrontView:
        // correct according to dissassembly
        player->cameras[CamIV_Isometric].mappos.x.val = cam->mappos.x.val;
        player->cameras[CamIV_Isometric].mappos.y.val = cam->mappos.y.val;
        break;
    }
    if (dungeon->camera_deviate_quake) {
        dungeon->camera_deviate_quake--;
    }
    if (dungeon->camera_deviate_jump > 0) {
        dungeon->camera_deviate_jump -= 32;
    }
}

void update_all_players_cameras(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
    {
          update_player_camera(player);
    }
  }
}

void zoom_moves_cam_towards_mouse(struct PlayerInfo *player, int pointer_x, int pointer_y)
{
    if (!is_my_player(player)) {return;}

    // This function runs after the zooming function is called.
    struct Camera* cam = player->acamera;

    int viewport_w = player->engine_window_width;
    int viewport_h = player->engine_window_height;

    int viewport_center_x = (viewport_w / 2);
    int viewport_center_y = (viewport_h / 2);

    float previous_pos_x = (pointer_x - viewport_center_x) / cam->previous_zoomed_percent;
    float previous_pos_y = (pointer_y - viewport_center_y) / cam->previous_zoomed_percent;
    
    float new_pos_x = (pointer_x - viewport_center_x) / cam->zoomed_percent;
    float new_pos_y = (pointer_y - viewport_center_y) / cam->zoomed_percent;

    int distance_x = (previous_pos_x - new_pos_x) * 2.0;
    int distance_y = (previous_pos_y - new_pos_y) * 2.0;
    
    if (distance_x < 0) {
        view_move_camera_left(cam, abs(distance_x));
    } else {
        if (distance_x > 0) {
            view_move_camera_right(cam, abs(distance_x));
        }
    }
    if (distance_y < 0) {
        view_move_camera_up(cam, abs(distance_y));
    } else {
        if (distance_y > 0) {
            view_move_camera_down(cam, abs(distance_y));
        }
    }
}
/******************************************************************************/
