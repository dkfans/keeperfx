/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_senses.c
 *     Functions to check vision, hearing and other senses of creatures.
 * @par Purpose:
 *     Creature senses checks and handling.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     27 Nov 2011 - 22 Jan 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_senses.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "map_blocks.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
DLLIMPORT long _DK_line_of_sight_3d_ignoring_specific_door(const struct Coord3d *frpos, const struct Coord3d *topos, const struct Thing *doortng);
DLLIMPORT long _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(const struct Coord3d *frpos, const struct Coord3d *topos, const struct Thing *doortng);
DLLIMPORT long _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(const struct Coord3d *frpos, const struct Coord3d *topos, long plyr_idx);
DLLIMPORT unsigned char _DK_line_of_sight_3d(const struct Coord3d *pos1, const struct Coord3d *pos2);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool sibling_line_of_sight_ignoring_door(const struct Coord3d *prevpos,
    const struct Coord3d *nextpos, const struct Thing *doortng)
{
    // If both dimensions changed, allow the pass
    if ((nextpos->x.stl.num != prevpos->x.stl.num) &&
        (nextpos->y.stl.num != prevpos->y.stl.num)) {
        return true;
    }
    struct Coord3d pos1;
    struct Coord3d pos2;
    int subdelta_x, subdelta_y;
    subdelta_x = (nextpos->x.stl.num - prevpos->x.stl.num);
    subdelta_y = (nextpos->y.stl.num - prevpos->y.stl.num);
    switch (subdelta_x + 2 * subdelta_y)
    {
    case -3:
        pos2.x.val = prevpos->x.val;
        pos2.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val;
        pos1.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val - 256;
        pos2.y.val = prevpos->y.val - 256;
        if (!point_in_map_is_solid_ignoring_door(&pos1, doortng) &&
            !point_in_map_is_solid_ignoring_door(&pos2, doortng)) {
            return false;
        }
        break;
    case -1:
        pos2.y.val = prevpos->y.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val - 256;
        pos2.x.val = prevpos->x.val + 256;
        if (!point_in_map_is_solid_ignoring_door(&pos1, doortng) &&
            !point_in_map_is_solid_ignoring_door(&pos2, doortng)) {
            return false;
        }
        break;
    case 1:
        pos2.x.val = prevpos->x.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val - 256;
        pos2.y.val = prevpos->y.val + 256;
        if (!point_in_map_is_solid_ignoring_door(&pos1, doortng) &&
            !point_in_map_is_solid_ignoring_door(&pos2, doortng)) {
            return false;
        }
        break;
    case 3:
        pos2.y.val = prevpos->y.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val + 256;
        pos2.x.val = prevpos->x.val + 256;
        if (!point_in_map_is_solid_ignoring_door(&pos1, doortng) &&
            !point_in_map_is_solid_ignoring_door(&pos2, doortng)) {
            return false;
        }
        break;
    }
    return true;
}


TbBool line_of_sight_3d_ignoring_specific_door(const struct Coord3d *frpos,
    const struct Coord3d *topos, const struct Thing *doortng)
{
    MapSubtlCoord dx,dy,dz;
    dx = (MapSubtlCoord)topos->x.val - (MapSubtlCoord)frpos->x.val;
    dy = (MapSubtlCoord)topos->y.val - (MapSubtlCoord)frpos->y.val;
    dz = (MapSubtlCoord)topos->z.val - (MapSubtlCoord)frpos->z.val;
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        return true;
    }
    //return _DK_line_of_sight_3d_ignoring_specific_door(frpos, topos, doortng);
    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = 256;
    } else {
        dx = -dx;
        increase_x = -256;
    }
    if (dy >= 0) {
        increase_y = 256;
    } else {
        dy = -dy;
        increase_y = -256;
    }
    if (dz >= 0) {
        increase_z = 256;
    } else {
        dz = -dz;
        increase_z = -256;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = dx * increase_x / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;
    while (distance > 0)
    {
        if (point_in_map_is_solid_ignoring_door(&nextpos, doortng)) {
            return false;
        }
        if (!sibling_line_of_sight_ignoring_door(&prevpos, &nextpos, doortng)) {
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    return true;
}

TbBool sibling_line_of_sight_3d_including_lava_check_ignoring_door(const struct Coord3d *prevpos,
    const struct Coord3d *nextpos, const struct Thing *doortng)
{
    // If both dimensions changed, allow the pass
    if ((nextpos->x.stl.num != prevpos->x.stl.num) &&
        (nextpos->y.stl.num != prevpos->y.stl.num)) {
        return true;
    }
    struct Coord3d pos1;
    struct Coord3d pos2;
    int subdelta_x, subdelta_y;
    subdelta_x = (nextpos->x.stl.num - prevpos->x.stl.num);
    subdelta_y = (nextpos->y.stl.num - prevpos->y.stl.num);
    switch (subdelta_x + 2 * subdelta_y)
    {
    case -3:
        pos2.x.val = prevpos->x.val;
        pos2.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val;
        pos1.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val - 256;
        pos2.y.val = prevpos->y.val - 256;
        if (get_point_in_map_solid_flags_ignoring_door(&pos1, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&pos2, doortng) & 0x01) {
            return false;
        }
        break;
    case -1:
        pos2.y.val = prevpos->y.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val - 256;
        pos2.x.val = prevpos->x.val + 256;
        if (get_point_in_map_solid_flags_ignoring_door(&pos1, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&pos2, doortng) & 0x01) {
            return false;
        }
        break;
    case 1:
        pos2.x.val = prevpos->x.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val - 256;
        pos2.y.val = prevpos->y.val + 256;
        if (get_point_in_map_solid_flags_ignoring_door(&pos1, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&pos2, doortng) & 0x01) {
            return false;
        }
        break;
    case 3:
        pos2.y.val = prevpos->y.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val + 256;
        pos2.x.val = prevpos->x.val + 256;
        if (get_point_in_map_solid_flags_ignoring_door(&pos1, doortng) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_door(&pos2, doortng) & 0x01) {
            return false;
        }
        break;
    }
    return true;
}

TbBool jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(const struct Coord3d *frpos,
    const struct Coord3d *topos, const struct Thing *doortng)
{
    MapSubtlCoord dx,dy,dz;
    dx = (MapSubtlCoord)topos->x.val - (MapSubtlCoord)frpos->x.val;
    dy = (MapSubtlCoord)topos->y.val - (MapSubtlCoord)frpos->y.val;
    dz = (MapSubtlCoord)topos->z.val - (MapSubtlCoord)frpos->z.val;
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        return true;
    }
    //return _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(frpos, topos, doortng);
    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = 256;
    } else {
        dx = -dx;
        increase_x = -256;
    }
    if (dy >= 0) {
        increase_y = 256;
    } else {
        dy = -dy;
        increase_y = -256;
    }
    if (dz >= 0) {
        increase_z = 256;
    } else {
        dz = -dz;
        increase_z = -256;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = dx * increase_x / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;
    while (distance > 0)
    {
        if (get_point_in_map_solid_flags_ignoring_door(&nextpos, doortng) & 0x01) {
            return false;
        }
        if (!sibling_line_of_sight_3d_including_lava_check_ignoring_door(&prevpos, &nextpos, doortng)) {
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    return true;
}

TbBool sibling_line_of_sight_3d_including_lava_check_ignoring_own_door(const struct Coord3d *prevpos,
    const struct Coord3d *nextpos, PlayerNumber plyr_idx)
{
    // Check for door at central subtile
    if (subtile_is_door(slab_subtile_center(subtile_slab(nextpos->x.stl.num)), slab_subtile_center(subtile_slab(nextpos->y.stl.num))))
        return false;
    // If both subtiles didn't changed, allow the pass
    if ((nextpos->x.stl.num == prevpos->x.stl.num)
     || (nextpos->y.stl.num == prevpos->y.stl.num)) {
        return true;
    }
    struct Coord3d pos1;
    struct Coord3d pos2;
    int subdelta_x, subdelta_y;
    subdelta_x = (nextpos->x.stl.num - prevpos->x.stl.num);
    subdelta_y = (nextpos->y.stl.num - prevpos->y.stl.num);
    switch (subdelta_x + 2 * subdelta_y)
    {
    case -3:
        pos2.x.val = prevpos->x.val;
        pos2.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val;
        pos1.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val - subtile_coord(1,0);
        pos2.y.val = prevpos->y.val - subtile_coord(1,0);
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos1, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos2, plyr_idx) & 0x01) {
            return false;
        }
        break;

    case -1:
        pos2.y.val = prevpos->y.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val - subtile_coord(1,0);
        pos2.x.val = prevpos->x.val + subtile_coord(1,0);
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos1, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos2, plyr_idx) & 0x01) {
            return false;
        }
        break;

    case 1:
        pos2.x.val = prevpos->x.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val - subtile_coord(1,0);
        pos2.y.val = prevpos->y.val + subtile_coord(1,0);
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos1, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos2, plyr_idx) & 0x01) {
            return false;
        }
        break;

    case 3:
        pos2.y.val = prevpos->y.val;
        pos2.z.val = prevpos->z.val;
        pos1.x.val = prevpos->x.val;
        pos1.z.val = prevpos->z.val;
        pos1.y.val = prevpos->y.val + 256;
        pos2.x.val = prevpos->x.val + 256;
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos1, plyr_idx) & 0x01) {
            return false;
        }
        if (get_point_in_map_solid_flags_ignoring_own_door(&pos2, plyr_idx) & 0x01) {
            return false;
        }
        break;
    }
    return true;
}

TbBool jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(const struct Coord3d *frpos,
    const struct Coord3d *topos, PlayerNumber plyr_idx)
{
    MapSubtlCoord dx,dy,dz;
    dx = (MapSubtlCoord)topos->x.val - (MapSubtlCoord)frpos->x.val;
    dy = (MapSubtlCoord)topos->y.val - (MapSubtlCoord)frpos->y.val;
    dz = (MapSubtlCoord)topos->z.val - (MapSubtlCoord)frpos->z.val;
    // Allow the travel to the same subtile
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        return true;
    }
    //return _DK_jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(frpos, topos, plyr_idx);
    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = 256;
    } else {
        dx = -dx;
        increase_x = -256;
    }
    if (dy >= 0) {
        increase_y = 256;
    } else {
        dy = -dy;
        increase_y = -256;
    }
    if (dz >= 0) {
        increase_z = 256;
    } else {
        dz = -dz;
        increase_z = -256;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = increase_x * dx / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;
    while (distance > 0)
    {
        if (get_point_in_map_solid_flags_ignoring_own_door(&nextpos, plyr_idx) & 0x01) {
            SYNCDBG(17, "Player %d cannot see through (%d,%d) due to solid flags",
                (int)plyr_idx,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num);
            return false;
        }
        if (!sibling_line_of_sight_3d_including_lava_check_ignoring_own_door(&prevpos, &nextpos, plyr_idx)) {
            SYNCDBG(17, "Player %d cannot see through (%d,%d) due to 3D line of sight",
                (int)plyr_idx,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num);
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    return true;
}

TbBool jonty_creature_can_see_thing_including_lava_check(const struct Thing *creatng, const struct Thing *thing)
{
    struct CreatureStats *crstat;
    const struct Coord3d *srcpos;
    struct Coord3d pos1;
    struct Coord3d pos2;
    //return _DK_jonty_creature_can_see_thing_including_lava_check(creatng, thing);
    crstat = creature_stats_get_from_thing(creatng);
    srcpos = &creatng->mappos;
    pos1.x.val = srcpos->x.val;
    pos1.y.val = srcpos->y.val;
    pos1.z.val = srcpos->z.val;
    pos2.x.val = thing->mappos.x.val;
    pos2.y.val = thing->mappos.y.val;
    pos2.z.val = thing->mappos.z.val;
    pos1.z.val += crstat->eye_height;
    if (thing->class_id == TCls_Door)
    {
        // If we're immune to lava, or we're already on it - don't care, travel over it
        if (lava_at_position(srcpos) || creature_can_travel_over_lava(creatng))
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks w/o lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (line_of_sight_3d_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (line_of_sight_3d_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            return false;
        } else
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks with lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_specific_door(&pos1, &pos2, thing))
                return true;
            return false;
        }
    } else
    {
        // If we're immune to lava, or we're already on it - don't care, travel over it
        if (lava_at_position(srcpos) || creature_can_travel_over_lava(creatng))
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks w/o lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (line_of_sight_3d(&pos1, &pos2))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (line_of_sight_3d(&pos1, &pos2))
                return true;
            return false;
        } else
        {
            SYNCDBG(17, "The %s index %d owned by player %d checks with lava %s index %d",
                thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(thing),(int)thing->index);
            // Check bottom of the thing
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(&pos1, &pos2, creatng->owner))
                return true;
            // Check top of the thing
            pos2.z.val += thing->field_58;
            if (jonty_line_of_sight_3d_including_lava_check_ignoring_own_door(&pos1, &pos2, creatng->owner))
                return true;
            return false;
        }
    }
}

TbBool line_of_sight_3d(const struct Coord3d *frpos, const struct Coord3d *topos)
{
    MapSubtlCoord dx,dy,dz;
    dx = (MapSubtlCoord)topos->x.val - (MapSubtlCoord)frpos->x.val;
    dy = (MapSubtlCoord)topos->y.val - (MapSubtlCoord)frpos->y.val;
    dz = (MapSubtlCoord)topos->z.val - (MapSubtlCoord)frpos->z.val;
    if ((topos->x.stl.num == frpos->x.stl.num) &&
        (topos->y.stl.num == frpos->y.stl.num)) {
        return true;
    }
    //return _DK_line_of_sight_3d(frpos, topos);

    MapCoord increase_x, increase_y, increase_z;
    MapSubtlCoord distance;
    if (dx >= 0) {
        increase_x = 256;
    } else {
        dx = -dx;
        increase_x = -256;
    }
    if (dy >= 0) {
        increase_y = 256;
    } else {
        dy = -dy;
        increase_y = -256;
    }
    if (dz >= 0) {
        increase_z = 256;
    } else {
        dz = -dz;
        increase_z = -256;
    }
    { // Compute amount of steps for the loop
        int maxdim1, maxdim2;
        if (dy == dx)
        {
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        else
        if (dy > dx)
        {
            increase_x = dx * increase_x / dy;
            increase_z = increase_z * dz / dy;
            maxdim1 = frpos->y.stl.num;
            maxdim2 = topos->y.stl.num;
        } else
        {
            increase_y = increase_y * dy / dx;
            increase_z = increase_z * dz / dx;
            maxdim1 = frpos->x.stl.num;
            maxdim2 = topos->x.stl.num;
        }
        distance = abs(maxdim2 - maxdim1);
    }
    // Go through the distance with given increases
    struct Coord3d prevpos;
    struct Coord3d nextpos;
    prevpos.x.val = frpos->x.val;
    prevpos.y.val = frpos->y.val;
    prevpos.z.val = frpos->z.val;
    nextpos.x.val = prevpos.x.val + increase_x;
    nextpos.y.val = prevpos.y.val + increase_y;
    nextpos.z.val = prevpos.z.val + increase_z;
    while (distance > 0)
    {
        if (point_in_map_is_solid(&nextpos)) {
            return false;
        }
        if (!sibling_line_of_sight(&prevpos, &nextpos)) {
            return false;
        }
        // Go to next sibling subtile
        prevpos.x.val = nextpos.x.val;
        prevpos.y.val = nextpos.y.val;
        prevpos.z.val = nextpos.z.val;
        nextpos.x.val += increase_x;
        nextpos.y.val += increase_y;
        nextpos.z.val += increase_z;
        distance--;
    }
    return true;
}

/******************************************************************************/
