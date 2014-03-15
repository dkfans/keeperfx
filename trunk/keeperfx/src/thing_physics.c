/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_physics.c
 *     Implementation of physics functions used for things.
 * @par Purpose:
 *     Functions to move things, with acceleration, speed and bouncing/sliding
 *     on walls.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 02 Mar 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_physics.h"

#include "globals.h"
#include "bflib_basics.h"

#include "thing_data.h"
#include "thing_stats.h"
#include "thing_creature.h"
#include "thing_list.h"
#include "creature_control.h"
#include "map_data.h"
#include "map_columns.h"
#include "map_utils.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_slide_thing_against_wall_at(struct Thing *thing, struct Coord3d *pos, long a3);
DLLIMPORT void _DK_bounce_thing_off_wall_at(struct Thing *thing, struct Coord3d *pos, long a3);
DLLIMPORT long _DK_creature_cannot_move_directly_to(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_thing_in_wall_at(const struct Thing *thing, const struct Coord3d *pos);
DLLIMPORT long _DK_get_thing_height_at(const struct Thing *thing, const struct Coord3d *pos);
DLLIMPORT long _DK_get_thing_height_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long a3);
DLLIMPORT long _DK_thing_in_wall_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long a3);
/******************************************************************************/


/******************************************************************************/
TbBool thing_touching_floor(const struct Thing *thing)
{
    return (thing->field_60 == thing->mappos.z.val);
}

TbBool thing_touching_flight_altitude(const struct Thing *thing)
{
    int i;
    if (thing->acceleration.z.val > 0) {
        return false;
    }
    i = get_floor_height_under_thing_at(thing, &thing->mappos);
    return (thing->mappos.z.val >= i + 9*NORMAL_FLYING_ALTITUDE/10)
        && (thing->mappos.z.val <= i + 11*NORMAL_FLYING_ALTITUDE/10);
}

void slide_thing_against_wall_at(struct Thing *thing, struct Coord3d *pos, long a3)
{
    _DK_slide_thing_against_wall_at(thing, pos, a3); return;
}

void bounce_thing_off_wall_at(struct Thing *thing, struct Coord3d *pos, long a3)
{
    _DK_bounce_thing_off_wall_at(thing, pos, a3); return;
}

void remove_relevant_forces_from_thing_after_slide(struct Thing *thing, struct Coord3d *pos, long a3)
{
    switch ( a3 )
    {
    case 1:
        thing->pos_2C.x.val = 0;
        break;
    case 2:
        thing->pos_2C.y.val = 0;
        break;
    case 3:
        thing->pos_2C.x.val = 0;
        thing->pos_2C.y.val = 0;
        break;
    case 4:
        thing->pos_2C.z.val = 0;
        break;
    case 5:
        thing->pos_2C.x.val = 0;
        thing->pos_2C.z.val = 0;
        break;
    case 6:
        thing->pos_2C.y.val = 0;
        thing->pos_2C.z.val = 0;
        break;
    case 7:
        thing->pos_2C.x.val = 0;
        thing->pos_2C.y.val = 0;
        thing->pos_2C.z.val = 0;
        break;
    }
}

TbBool positions_equivalent(const struct Coord3d *pos_a, const struct Coord3d *pos_b)
{
    if (pos_a->x.val != pos_b->x.val)
        return false;
    if (pos_a->y.val != pos_b->y.val)
        return false;
    if (pos_a->z.val != pos_b->z.val)
        return false;
    return true;
}

void creature_set_speed(struct Thing *thing, long speed)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (speed < -MAX_VELOCITY)
    {
        cctrl->move_speed = -MAX_VELOCITY;
    } else
    if (speed > MAX_VELOCITY)
    {
        cctrl->move_speed = MAX_VELOCITY;
    } else
    {
        cctrl->move_speed = speed;
    }
    cctrl->flgfield_1 |= CCFlg_Unknown40;
}

TbBool cross_x_boundary_first(struct Coord3d *pos1, struct Coord3d *pos2)
{
  int delta_x, delta_y;
  int mul_x, mul_y;
  delta_x = pos2->x.val - (int)pos1->x.val;
  delta_y = pos2->y.val - (int)pos1->y.val;
  if (delta_x < 0) {
      mul_x = pos1->x.stl.pos;
  } else {
      mul_x = 255 - (int)pos1->x.stl.pos;
  }
  if ( delta_y < 0 ) {
      mul_y = pos1->y.stl.pos;
  } else {
      mul_y = 255 - (int)pos1->y.stl.pos;
  }
  return abs(delta_x * mul_y) > abs(mul_x * delta_y);
}

TbBool cross_y_boundary_first(struct Coord3d *pos1, struct Coord3d *pos2)
{
  int delta_x, delta_y;
  int mul_x, mul_y;
  delta_x = pos2->x.val - (int)pos1->x.val;
  delta_y = pos2->y.val - (int)pos1->y.val;
  if (delta_x < 0) {
      mul_x = pos1->x.stl.pos;
  } else {
      mul_x = 255 - (int)pos1->x.stl.pos;
  }
  if ( delta_y < 0 ) {
      mul_y = pos1->y.stl.pos;
  } else {
      mul_y = 255 - (int)pos1->y.stl.pos;
  }
  return abs(delta_y * mul_x) > abs(mul_y * delta_x);
}

TbBool position_over_floor_level(const struct Thing *thing, const struct Coord3d *pos)
{
    struct Coord3d modpos;
    modpos.x.val = pos->x.val;
    modpos.y.val = pos->y.val;
    modpos.z.val = pos->z.val;
    if (thing_in_wall_at(thing, &modpos))
    {
        long curr_height, norm_height;
        curr_height = thing->mappos.z.val;
        norm_height = get_floor_height_under_thing_at(thing, &modpos);
        if (norm_height < curr_height)
        {
            return true;
        }
        modpos.z.val = -1;
        norm_height = get_thing_height_at(thing, &modpos);
        if ((norm_height == -1) || (norm_height - curr_height > 256))
        {
            return true;
        }
    }
    return false;
}

long creature_cannot_move_directly_to(struct Thing *thing, struct Coord3d *pos)
{
    //return _DK_creature_cannot_move_directly_to(thing, pos);
    struct Coord3d realpos;
    realpos.x.val = thing->mappos.x.val;
    realpos.y.val = thing->mappos.y.val;
    realpos.z.val = thing->mappos.z.val;
    int delta_x, delta_y;
    delta_x = pos->x.val - (long)realpos.x.val;
    delta_y = pos->y.val - (long)realpos.y.val;
    struct Coord3d newpos;
    newpos.x.val = thing->mappos.x.val;
    newpos.y.val = thing->mappos.y.val;
    newpos.z.val = thing->mappos.z.val;

    if ((pos->x.stl.num != realpos.x.stl.num) && (pos->y.stl.num != realpos.y.stl.num))
    {
        struct Coord3d modpos;

        if (cross_x_boundary_first(&realpos, pos))
        {
            int i;

            if (pos->x.val <= realpos.x.val)
              i = (realpos.x.val & 0xFF00) - 1;
            else
              i = (realpos.x.val + 256) & 0xFF00;
            modpos.x.val = i;
            modpos.y.val = delta_y * (i - newpos.x.val) / delta_x + newpos.y.val;
            modpos.z.val = realpos.z.val;
            if (position_over_floor_level(thing, &modpos)) {
                return 1;
            }
            thing->mappos.x.val = modpos.x.val;
            thing->mappos.y.val = modpos.y.val;
            thing->mappos.z.val = modpos.z.val;
            thing->mappos.z.val = get_thing_height_at(thing, &modpos);

            realpos.x.val = thing->mappos.x.val;
            realpos.y.val = thing->mappos.y.val;
            realpos.z.val = thing->mappos.z.val;

            if (pos->y.val <= realpos.y.val)
              i = (realpos.y.val & 0xFF00) - 1;
            else
              i = (realpos.y.val + 256) & 0xFF00;
            modpos.y.val = i;
            modpos.x.val = delta_x * (i - newpos.y.val) / delta_y + newpos.x.val;
            modpos.z.val = realpos.z.val;
            if (position_over_floor_level(thing, &modpos)) {
                thing->mappos.x.val = newpos.x.val;
                thing->mappos.y.val = newpos.y.val;
                thing->mappos.z.val = newpos.z.val;
                return 1;
            }
            thing->mappos.x.val = modpos.x.val;
            thing->mappos.y.val = modpos.y.val;
            thing->mappos.z.val = modpos.z.val;
            thing->mappos.z.val = get_thing_height_at(thing, &modpos);

            realpos.x.val = thing->mappos.x.val;
            realpos.y.val = thing->mappos.y.val;
            realpos.z.val = thing->mappos.z.val;

            modpos.x.val = pos->x.val;
            modpos.y.val = pos->y.val;
            modpos.z.val = realpos.z.val;
            if (position_over_floor_level(thing, &modpos)) {
                thing->mappos.x.val = newpos.x.val;
                thing->mappos.y.val = newpos.y.val;
                thing->mappos.z.val = newpos.z.val;
                return 1;
            }
            thing->mappos.x.val = newpos.x.val;
            thing->mappos.y.val = newpos.y.val;
            thing->mappos.z.val = newpos.z.val;
            return 0;
        }

        if (cross_y_boundary_first(&realpos, pos))
        {
            int i;

            if (pos->y.val <= realpos.y.val)
              i = (realpos.y.val & 0xFF00) - 1;
            else
              i = (realpos.y.val + 256) & 0xFF00;
            modpos.y.val = i;
            modpos.x.val = delta_x * (i - newpos.y.val) / delta_y + newpos.x.val;
            modpos.z.val = realpos.z.val;
            if (position_over_floor_level(thing, &modpos)) {
                return 1;
            }
            thing->mappos.x.val = modpos.x.val;
            thing->mappos.y.val = modpos.y.val;
            thing->mappos.z.val = modpos.z.val;
            thing->mappos.z.val = get_thing_height_at(thing, &modpos);

            realpos.x.val = thing->mappos.x.val;
            realpos.y.val = thing->mappos.y.val;
            realpos.z.val = thing->mappos.z.val;

            if (pos->x.val <= realpos.x.val)
              i = (realpos.x.val & 0xFF00) - 1;
            else
              i = (realpos.x.val + 256) & 0xFF00;
            modpos.x.val = i;
            modpos.y.val = delta_y * (modpos.x.val - newpos.x.val) / delta_x + newpos.y.val;
            modpos.z.val = realpos.z.val;
            if (position_over_floor_level(thing, &modpos)) {
                thing->mappos.x.val = newpos.x.val;
                thing->mappos.y.val = newpos.y.val;
                thing->mappos.z.val = newpos.z.val;
                return 1;
            }
            thing->mappos.x.val = modpos.x.val;
            thing->mappos.y.val = modpos.y.val;
            thing->mappos.z.val = modpos.z.val;
            thing->mappos.z.val = get_thing_height_at(thing, &modpos);

            realpos.x.val = thing->mappos.x.val;
            realpos.y.val = thing->mappos.y.val;
            realpos.z.val = thing->mappos.z.val;

            modpos.x.val = pos->x.val;
            modpos.y.val = pos->y.val;
            modpos.z.val = realpos.z.val;
            if (position_over_floor_level(thing, &modpos)) {
                thing->mappos.x.val = newpos.x.val;
                thing->mappos.y.val = newpos.y.val;
                thing->mappos.z.val = newpos.z.val;
                return 1;
            }
            thing->mappos.x.val = newpos.x.val;
            thing->mappos.y.val = newpos.y.val;
            thing->mappos.z.val = newpos.z.val;
            return 0;
        }

        if (position_over_floor_level(thing, pos)) {
            thing->mappos.x.val = newpos.x.val;
            thing->mappos.y.val = newpos.y.val;
            thing->mappos.z.val = newpos.z.val;
            return 1;
        }
        thing->mappos.x.val = newpos.x.val;
        thing->mappos.y.val = newpos.y.val;
        thing->mappos.z.val = newpos.z.val;
        return 0;
    }

    if (position_over_floor_level(thing, pos)) {
        return 1;
    }
    return 0;
}

/** Retrieves planned next position for given thing, without collision detection.
 *  Just adds thing velocity to current position and does some clipping. Nothing fancy.
 * @param pos The position to be set.
 * @param thing Source thing which position and velocity is used.
 * @return Gives true if values were in map coords range, false if they were
 *  outside map area and had to be corrected.
 */
TbBool get_thing_next_position(struct Coord3d *pos, const struct Thing *thing)
{
    // Don't clip the Z coord - clipping would make impossible to hit base ground (ie. water drip over water)
    return set_coords_add_velocity(pos, &thing->mappos, &thing->velocity, MapCoord_ClipX|MapCoord_ClipY);
}

long get_thing_height_at(const struct Thing *thing, const struct Coord3d *pos)
{
  SYNCDBG(18,"Starting");
  return _DK_get_thing_height_at(thing, pos);
}

long get_thing_height_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long radius)
{
    return _DK_get_thing_height_at_with_radius(thing, pos, radius);
}

TbBool map_is_solid_at_height(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapCoord height_beg, MapCoord height_end)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & MapFlg_IsTall) != 0)
    {
        return true;
    }
    if (get_map_floor_height(mapblk) > height_beg)
    {
        return true;
    }
    if (get_map_ceiling_height(mapblk) < height_end)
    {
        return true;
    }
    return false;
}

long thing_in_wall_at(const struct Thing *thing, const struct Coord3d *pos)
{
    int radius;
    long i;
    //return _DK_thing_in_wall_at(thing, pos);
    if (thing_is_creature(thing)) {
        i = thing_nav_sizexy(thing);
    } else {
        i = thing->sizexy;
    }
    radius = i/2;
    // Base on the radius, determine bounds of the object
    MapSubtlCoord stl_x_beg, stl_x_end;
    MapSubtlCoord stl_y_beg, stl_y_end;
    MapCoord height_beg, height_end;
    height_beg = pos->z.val;
    height_end = height_beg + thing->field_58;
    stl_x_beg = coord_subtile(pos->x.val - radius);
    stl_x_end = coord_subtile(pos->x.val + radius);
    stl_y_end = coord_subtile(pos->y.val + radius);
    stl_y_beg = coord_subtile(pos->y.val - radius);
    MapSubtlCoord stl_x, stl_y;
    for (stl_y = stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (stl_x = stl_x_beg; stl_x <= stl_x_end; stl_x++)
        {
            if (map_is_solid_at_height(stl_x, stl_y, height_beg, height_end)) {
                return 1;
            }
        }
    }
    return 0;
}

long thing_in_wall_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long radius)
{
    return _DK_thing_in_wall_at_with_radius(thing, pos, radius);
}

long get_floor_height_under_thing_at(const struct Thing *thing, const struct Coord3d *pos)
{
    int radius;
    long i;
    //return _DK_get_floor_height_under_thing_at(thing, pos);
    if (thing_is_creature(thing)) {
        i = thing_nav_sizexy(thing);
    } else {
        i = thing->sizexy;
    }
    radius = i/2;
    // Get range of coords under thing
    MapCoord pos_x_beg, pos_x_end;
    MapCoord pos_y_beg, pos_y_end;
    pos_x_beg = (pos->x.val - radius);
    if (pos_x_beg < 0)
        pos_x_beg = 0;
    pos_x_end = pos->x.val + radius;
    pos_y_beg = (pos->y.val - radius);
    if (pos_y_beg < 0)
        pos_y_beg = 0;
    if (pos_x_end >= 65535)
        pos_x_end = 65535;
    pos_y_end = pos->y.val + radius;
    if (pos_y_end >= 65535)
        pos_y_end = 65535;
    // Find correct floor and ceiling plane for the area
    MapSubtlCoord floor_height, ceiling_height;
    get_min_floor_and_ceiling_heights_for_rect(coord_subtile(pos_x_beg), coord_subtile(pos_y_beg),
        coord_subtile(pos_x_end), coord_subtile(pos_y_end), &floor_height, &ceiling_height);
    return floor_height << 8;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
