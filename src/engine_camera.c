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
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_planar.h"

#include "engine_lenses.h"
#include "engine_render.h"
#include "thing_physics.h"
#include "vidmode.h"
#include "map_blocks.h"
#include "dungeon_data.h"
#include "config_settings.h"
#include "player_instances.h"
#include "frontmenu_ingame_map.h"
#include "local_camera.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
long zoom_distance_setting;
long frontview_zoom_distance_setting;
long camera_zoom;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

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
    return LbArcTanAngle(vec->x.val, vec->y.val) & ANGLE_MASK;
}

long get_angle_yz_to_vec(const struct CoordDelta3d *vec)
{
    long dist = LbDiagonalLength(abs(vec->x.val), abs(vec->y.val));
    return LbArcTanAngle(vec->z.val, dist) & ANGLE_MASK;
}

long get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    return LbArcTanAngle((long)pos2->x.val - (long)pos1->x.val, (long)pos2->y.val - (long)pos1->y.val) & ANGLE_MASK;
}

long get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist = get_2d_distance(pos1, pos2);
    return LbArcTanAngle(pos2->z.val - pos1->z.val, dist) & ANGLE_MASK;
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

void set_view_position(MapCoord *pos_x, MapCoord *pos_y, MapCoord x, MapCoord y)
{
    *pos_x = clamp(x, 0, game.map_subtiles_x * COORD_PER_STL - 1);
    *pos_y = clamp(y, 0, game.map_subtiles_y * COORD_PER_STL - 1);
}

void view_set_camera_position(struct Camera *cam, MapCoord x, MapCoord y)
{
    set_view_position(&cam->mappos.x.val, &cam->mappos.y.val, x, y);
}

int32_t zoom_in_step(int32_t old_zoom, int32_t limit_max, int32_t limit_min)
{
    int32_t new_zoom = (100 * old_zoom) / 85;
    if (new_zoom == old_zoom)
        new_zoom++;
    return clamp(new_zoom, limit_min, limit_max);
}

int32_t zoom_out_step(int32_t old_zoom, int32_t limit_max, int32_t limit_min)
{
    int32_t new_zoom = (85 * old_zoom) / 100;
    if (new_zoom == old_zoom)
        new_zoom--;
    return clamp(new_zoom, limit_min, limit_max);
}

/** One zoom-in step for a view mode; the limits apply to iso, front view has its own. */
int32_t zoom_in_for_view(int32_t old_zoom, unsigned char view_mode, int32_t limit_max, int32_t limit_min)
{
    switch (view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        return zoom_in_step(old_zoom, limit_max, limit_min);
    case PVM_FrontView:
        return zoom_in_step(old_zoom, FRONTVIEW_CAMERA_ZOOM_MAX, FRONTVIEW_CAMERA_ZOOM_MIN); //Originally 16384, adjusted for view distance
    default:
        return old_zoom;
    }
}

int32_t zoom_out_for_view(int32_t old_zoom, unsigned char view_mode, int32_t limit_max, int32_t limit_min)
{
    switch (view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        return zoom_out_step(old_zoom, limit_max, limit_min);
    case PVM_FrontView:
        return zoom_out_step(old_zoom, FRONTVIEW_CAMERA_ZOOM_MAX, max(FRONTVIEW_CAMERA_ZOOM_MIN, frontview_zoom_distance_setting));
    default:
        return old_zoom;
    }
}

void shift_view_position_for_zoom(MapCoord *pos_x, MapCoord *pos_y, int32_t old_zoom, int32_t new_zoom, MapCoord x, MapCoord y)
{
    if ((x | y) < 0 || new_zoom == 0)
        return;

    const int64_t dx = x - *pos_x;
    const int64_t dy = y - *pos_y;
    const MapCoord new_x = *pos_x + dx * (new_zoom - old_zoom) / new_zoom;
    const MapCoord new_y = *pos_y + dy * (new_zoom - old_zoom) / new_zoom;
    set_view_position(pos_x, pos_y, new_x, new_y);
}

static void view_move_camera_on_zoom(struct Camera *cam, int32_t a, int32_t b, MapCoord x, MapCoord y)
{
    shift_view_position_for_zoom(&cam->mappos.x.val, &cam->mappos.y.val, a, b, x, y);
}

void view_zoom_camera_in_to(struct Camera *cam, int32_t limit_max, int32_t limit_min, MapCoord x, MapCoord y)
{
    const int32_t old_zoom = cam->zoom;
    cam->zoom = zoom_in_for_view(old_zoom, cam->view_mode, limit_max, limit_min);
    view_move_camera_on_zoom(cam, old_zoom, cam->zoom, x, y);
}

void view_zoom_camera_out_from(struct Camera *cam, int32_t limit_max, int32_t limit_min, MapCoord x, MapCoord y)
{
    const int32_t old_zoom = cam->zoom;
    cam->zoom = zoom_out_for_view(old_zoom, cam->view_mode, limit_max, limit_min);
    view_move_camera_on_zoom(cam, old_zoom, cam->zoom, x, y);
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

void view_set_camera_y_velocity(struct Camera *cam, long delta, long ilimit)
{
    long abslimit = abs(ilimit);
    cam->velocity_y += delta;
    if (cam->velocity_y < -abslimit) {
        cam->velocity_y = -abslimit;
    } else
    if (cam->velocity_y > abslimit) {
        cam->velocity_y = abslimit;
    }
    cam->in_active_movement_y = true;
}

void view_set_camera_x_velocity(struct Camera *cam, long delta, long ilimit)
{
    long abslimit = abs(ilimit);
    cam->velocity_x += delta;
    if (cam->velocity_x < -abslimit) {
        cam->velocity_x = -abslimit;
    } else
    if (cam->velocity_x > abslimit) {
        cam->velocity_x = abslimit;
    }
    cam->in_active_movement_x = true;
}

void view_set_camera_rotation_velocity(struct Camera *cam, int32_t delta, int32_t ilimit)
{
    const int32_t limit_val = abs(ilimit);
    const int32_t new_val = delta + cam->velocity_rotation;
    cam->velocity_rotation = clamp(new_val, -limit_val, +limit_val);
    cam->in_active_movement_rotation = true;
}

void view_set_camera_rotation_velocity_around(struct Camera *cam, int32_t delta, int32_t ilimit, MapCoord x, MapCoord y)
{
    view_set_camera_rotation_velocity(cam, delta, ilimit);
    if ((x | y) < 0)
        return;

    cam->rotation_pivot.x.val = x;
    cam->rotation_pivot.y.val = y;
    cam->use_rotation_pivot = true;
}

/** 0=resets, 1=up, 2=down. */
int32_t tilt_step(int32_t tilt, unsigned char mode)
{
    switch (mode)
    {
    case 0:
        return CAMERA_TILT_DEFAULT;
    case 1:
        return (tilt < CAMERA_TILT_MAX) ? tilt + 1 : tilt;
    case 2:
        return (tilt > CAMERA_TILT_MIN) ? tilt - 1 : tilt;
    default:
        return tilt;
    }
}

void view_set_camera_tilt(struct Camera *cam, unsigned char mode)
{
    cam->rotation_angle_y = tilt_step(cam->rotation_angle_y, mode);
}

void init_player_cameras(struct PlayerInfo *player)
{
    struct UserState* ustate = get_player_user_state(player);
    if (user_state_invalid(ustate))
        return;
    struct Thing* heartng = get_player_soul_container(player->id_number);
    ustate->dungeon_camera.x = heartng->mappos.x.val;
    ustate->dungeon_camera.y = heartng->mappos.y.val;
    ustate->dungeon_camera.yaw[false] = DEGREES_45;
    ustate->dungeon_camera.yaw[true] = 0;
    init_local_cameras(player);
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

void update_first_person_position(struct Camera *cam, struct Thing *thing, int eye_height)
{
    if ( thing_is_creature(thing) )
    {
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        if (cctrl->move_speed && thing_touching_floor(thing))
            cctrl->head_bob = 16 * get_walking_bob_direction(thing);
        else
            cctrl->head_bob = 0;

        int pos_x = move_coord_with_angle_x(thing->mappos.x.val,-90,thing->move_angle_xy);
        int pos_y = move_coord_with_angle_y(thing->mappos.y.val,-90,thing->move_angle_xy);

        view_set_camera_position(cam, pos_x, pos_y);

        if ( (thing->movement_flags & TMvF_Flying) != 0 )
        {
            cam->mappos.z.val = thing->mappos.z.val + eye_height;
            cam->rotation_angle_z = cctrl->roll;
        }
        else
        {
            cam->mappos.z.val = cam->mappos.z.val + ((int64_t)thing->mappos.z.val + cctrl->head_bob - cam->mappos.z.val + eye_height) / 2;
            cam->rotation_angle_z = 0;
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
            cam->rotation_angle_z = 0;
            thing->move_angle_z = 0;
            if ( thing->food.possession_startup_timer )
            {
                if ( thing->food.possession_startup_timer <= 3 )
                    thing->move_angle_z = -116 * thing->food.possession_startup_timer + DEGREES_360;
                else
                    thing->move_angle_z = 116 * thing->food.possession_startup_timer + 1352;
            }
        }
        else
        {
            cam->rotation_angle_z = 0;
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

// Positive distance moves rightwards
static void view_move_camera_x(struct Camera *cam, int32_t distance)
{
    const MapCoord pos_x = move_coord_with_angle_x(cam->mappos.x.val, distance, cam->rotation_angle_x + DEGREES_90);
    const MapCoord pos_y = move_coord_with_angle_y(cam->mappos.y.val, distance, cam->rotation_angle_x + DEGREES_90);
    view_set_camera_position(cam, pos_x, pos_y);
}

// Positive distance moves downwards
static void view_move_camera_y(struct Camera *cam, int32_t distance)
{
    const MapCoord pos_x = move_coord_with_angle_x(cam->mappos.x.val, distance, cam->rotation_angle_x + DEGREES_180);
    const MapCoord pos_y = move_coord_with_angle_y(cam->mappos.y.val, distance, cam->rotation_angle_x + DEGREES_180);
    view_set_camera_position(cam, pos_x, pos_y);
}

void view_process_camera_velocity(struct Camera *cam)
{
    if (cam->velocity_x)
        view_move_camera_x(cam, cam->velocity_x);

    if (cam->velocity_y)
        view_move_camera_y(cam, cam->velocity_y);

    if (cam->velocity_rotation)
    {
        cam->rotation_angle_x = (cam->velocity_rotation + cam->rotation_angle_x) & ANGLE_MASK;
        if (cam->use_rotation_pivot)
        {
            const MapCoordDelta x0 = cam->mappos.x.val - cam->rotation_pivot.x.val;
            const MapCoordDelta y0 = cam->mappos.y.val - cam->rotation_pivot.y.val;
            const int64_t sin = LbSinL(cam->velocity_rotation);
            const int64_t cos = LbCosL(cam->velocity_rotation);
            const MapCoordDelta x1 = (x0 * cos - y0 * sin) >> 16;
            const MapCoordDelta y1 = (x0 * sin + y0 * cos) >> 16;
            const MapCoord new_x = (MapCoord)cam->rotation_pivot.x.val + x1;
            const MapCoord new_y = (MapCoord)cam->rotation_pivot.y.val + y1;
            view_set_camera_position(cam, new_x, new_y);
        }
    }

    if (! cam->in_active_movement_x)
        cam->velocity_x /= 2;

    if (! cam->in_active_movement_y)
        cam->velocity_y /= 2;

    if (! cam->in_active_movement_rotation)
        cam->velocity_rotation /= 2;

    if (cam->velocity_rotation == 0)
        cam->use_rotation_pivot = false;

    cam->in_active_movement_x = false;
    cam->in_active_movement_y = false;
    cam->in_active_movement_rotation = false;
}

void view_set_camera_move_to_position(MapCoord from_x, MapCoord from_y, MapCoord x, MapCoord y, MapCoordDelta *move_x, MapCoordDelta *move_y)
{
    MapCoord positions[] = {from_x, from_y};
    MapCoord targets[] = {x, y};
    MapCoordDelta *movement[] = {move_x, move_y};
    for (int i = 0; i < 2; i++) {
        *movement[i] = max(abs(targets[i] - positions[i]) / 8, 256);
        if (targets[i] < positions[i]) {
            *movement[i] = -*movement[i];
        }
    }
}

TbBool view_move_camera_to_position(MapCoord *pos_x, MapCoord *pos_y, MapCoord x, MapCoord y, MapCoordDelta move_x, MapCoordDelta move_y)
{
    MapCoord *positions[] = {pos_x, pos_y};
    MapCoord targets[] = {x, y};
    MapCoordDelta movement[] = {move_x, move_y};
    for (int i = 0; i < 2; i++) {
        if (abs(*positions[i] - targets[i]) >= abs(movement[i])) {
            *positions[i] += movement[i];
        } else {
            *positions[i] = targets[i];
        }
    }
    return (*positions[0] == targets[0]) && (*positions[1] == targets[1]);
}

void update_player_camera(struct PlayerInfo *player)
{
    struct Dungeon *dungeon = get_players_dungeon(player);

    if (((get_player_view_type(player) == PVT_CreatureContrl) || (get_player_view_type(player) == PVT_CreaturePasngr))
     && (player->controlled_thing_idx <= 0) && (player->instance_num != PI_HeartZoom))
    {
        ERRORLOG("Cannot go first person without controlling creature");
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
/******************************************************************************/
