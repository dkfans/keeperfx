/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_wallhug.c
 *     Simple wallhug pathfinding functions.
 * @par Purpose:
 *     Functions implementing wallhug pathfinding algorithm.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 10 Jan 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "ariadne_wallhug.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"

#include "ariadne.h"
#include "slab_data.h"
#include "map_data.h"
#include "map_utils.h"
#include "thing_data.h"
#include "thing_doors.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "engine_camera.h"
#include "config_terrain.h"
#include "creature_control.h"
#include "creature_states.h"
#include "config_creature.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool check_forward_for_prospective_hugs(struct Thing *creatng, struct Coord3d *pos_a, long angle, long side, long slab_flags, long speed, PlayerBitFlags crt_owner_flags);
static long get_angle_of_wall_hug(struct Thing *creatng, long slab_flags, long speed, PlayerBitFlags crt_owner_flags);
static void set_hugging_pos_using_blocked_flags(struct Coord3d *dstpos, struct Thing *creatng, unsigned short block_flags, int nav_radius);
static TbBool navigation_push_towards_target(struct Navigation *navi, struct Thing *creatng, const struct Coord3d *pos, MoveSpeed speed, MoveSpeed nav_radius, PlayerBitFlags crt_owner_flags);
static TbBool find_approach_position_to_subtile(const struct Coord3d *srcpos, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MoveSpeed spacing, struct Coord3d *aproachpos);
static long creature_cannot_move_directly_to_with_collide(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlags crt_owner_flags);
static unsigned short get_hugging_blocked_flags(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlags crt_owner_flags);

const uint8_t wallhug_x_blocked_priorities[] = { 1,0,4,2,0,0,2,0,4,1,0,0,0,0 };
const uint8_t wallhug_y_blocked_priorities[] = { 2,0,2,1,0,6,1,0,2,2,0,0,0,0 };
const uint8_t wallhug_xy_blocked_priorities[22] = { 2,0,0,1,0,2,1,0,0,2,0,6,1,0,4,2,0,2,2,0,4,1 };

/******************************************************************************/
static TbBool wallhug_angle_with_collide_valid(struct Thing *thing, long slab_flags, long speed, long angle, PlayerBitFlags crt_owner_flags)
{
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(speed, angle);
    pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(speed, angle);
    pos.z.val = get_thing_height_at(thing, &pos);
    return (creature_cannot_move_directly_to_with_collide(thing, &pos, slab_flags, crt_owner_flags) != 4);
}

static long get_angle_of_wall_hug(struct Thing *creatng, long slab_flags, long speed, PlayerBitFlags crt_owner_flags)
{
    struct Navigation *navi;
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        navi = &cctrl->navi;
    }
    long quadr;
    long whangle;
    switch (navi->side)
    {
    case 1:
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * ((quadr - 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * (quadr & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * ((quadr + 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * ((quadr + 2) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        break;
    case 2:
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * ((quadr + 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * (quadr & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * ((quadr - 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = DEGREES_90 * ((quadr + 2) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        break;
    }
    return -1;
}

static int hug_round_sub(struct Thing *creatng, MapSubtlCoord *current_position_x, MapSubtlCoord *current_position_y,
                            const MapSubtlCoord target_position_x, const MapSubtlCoord target_position_y, char *hug_state_flag,
                            MapSubtlDelta *delta, struct Coord3d *pos1, long *hug_val, unsigned short *i,
                            int *next_round_index_1, const int arr_offset_1, const int arr_offset_2)
{
    if (*hug_state_flag == 2)
    {
        return -1;
    }

    SmallAroundIndex quadrant = (((LbArcTanAngle(target_position_x - *current_position_x, target_position_y - *current_position_y) & ANGLE_MASK) + DEGREES_45) >> 9) & 3;

    int distance_to_target = chessboard_distance(*current_position_x, *current_position_y, target_position_x, target_position_y);
    if ((int)abs(distance_to_target) <= *delta && hug_can_move_on(
                                       creatng,
                                       3 * small_around[quadrant].delta_x + *current_position_x,
                                       3 * small_around[quadrant].delta_y + *current_position_y))
    {
        *current_position_x += 3 * small_around[quadrant].delta_x;
        *current_position_y += 3 * small_around[quadrant].delta_y;

        *delta = chessboard_distance(*current_position_x, *current_position_y, target_position_x, target_position_y);

        *hug_state_flag = 1;
    }
    else
    {
        if (*hug_state_flag == 1)
        {
            pos1->x.stl.num = *current_position_x;
            pos1->y.stl.num = *current_position_y;
            *hug_val -= *i;
            return 0;
        }
        SmallAroundIndex search_direction_index = (*next_round_index_1 + arr_offset_1) & 3;
        for (unsigned short j = 0;; ++j)
        {
            SmallAroundIndex current_direction = search_direction_index;
            if (j >= 4u)
              break;
            SmallAroundIndex small_around_index = search_direction_index;
            if (hug_can_move_on(
                    creatng,
                    3 * small_around[small_around_index].delta_x + *current_position_x,
                    *current_position_y + 3 * small_around[small_around_index].delta_y))
            {
              *next_round_index_1 = current_direction;
              *current_position_x += 3 * small_around[small_around_index].delta_x;
              *current_position_y += 3 * small_around[small_around_index].delta_y;
              break;
            }
            search_direction_index = current_direction + arr_offset_2;
            search_direction_index = (current_direction + arr_offset_2) & 3;
        }
    }
    if (*current_position_x == target_position_x && *current_position_y == target_position_y)
    {
        *hug_val -= *i;
        return 1;
    }
    return -1;
}

static int hug_round(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, unsigned short round_idx, long *hug_val)
{
    MapSubtlCoord target_position_x = pos2->x.stl.num;
    MapSubtlCoord target_position_y = pos2->y.stl.num;
    MapSubtlCoord current_position_x = pos1->x.stl.num;
    MapSubtlCoord current_position_y = pos1->y.stl.num;

    MapSubtlCoord backup_position_x = pos1->x.stl.num;
    MapSubtlCoord backup_position_y = pos1->y.stl.num;

    int next_round_index = (round_idx + 1) & 3;
    int previous_round_index = (round_idx - 1) & 3;

    MapSubtlDelta max_delta_1 = chessboard_distance(current_position_x, current_position_y, target_position_x, target_position_y) - 1;
    MapSubtlDelta max_delta_2 = chessboard_distance(backup_position_x, backup_position_y, target_position_x, target_position_y) - 1;

    char hug_completion_status_primary = 0;
    char hug_completion_status_secondary = 0;
    unsigned short i;
    for (i = *hug_val; i && (hug_completion_status_primary != 2 || hug_completion_status_secondary != 2); --i)
    {
      char return_val;
      return_val = hug_round_sub(creatng, &current_position_x, &current_position_y, target_position_x, target_position_y, &hug_completion_status_primary, &max_delta_1, pos1, hug_val, &i, &next_round_index, 3, 1);
      if (return_val != -1)
        return return_val;
      return_val = hug_round_sub(creatng, &backup_position_x, &backup_position_y, target_position_x, target_position_y, &hug_completion_status_secondary, &max_delta_2, pos1, hug_val, &i, &previous_round_index, 1, 3);
      if (return_val != -1)
        return return_val;
    }
    if (!i)
      return -1;

    MapSubtlDelta distance_from_current = grid_distance(current_position_x, current_position_y, target_position_x, target_position_y);
    MapSubtlDelta distance_from_alternate = grid_distance(backup_position_x, backup_position_y, target_position_x, target_position_y);
    if (distance_from_alternate >= distance_from_current)
    {
      pos1->x.stl.num = current_position_x;
      pos1->y.stl.num = current_position_y;
    }
    else
    {
      pos1->x.stl.num = backup_position_x;
      pos1->y.stl.num = backup_position_y;
    }
    *hug_val -= i;
    return 0;
}

// Used only at `creature_states_combt
long slab_wall_hug_route(struct Thing *thing, struct Coord3d *pos, long max_val)
{
    struct Coord3d curr_pos;
    curr_pos.x.val = thing->mappos.x.val;
    curr_pos.y.val = thing->mappos.y.val;
    curr_pos.z.val = thing->mappos.z.val;
    curr_pos.x.stl.num = stl_slab_center_subtile(curr_pos.x.stl.num);
    curr_pos.y.stl.num = stl_slab_center_subtile(curr_pos.y.stl.num);
    MapSubtlCoord stl_x = stl_slab_center_subtile(pos->x.stl.num);
    MapSubtlCoord stl_y = stl_slab_center_subtile(pos->y.stl.num);
    struct Coord3d target_centered_position;
    target_centered_position.x.val = pos->x.val;
    target_centered_position.y.val = pos->y.val;
    target_centered_position.z.val = pos->z.val;
    target_centered_position.x.stl.num = stl_x;
    target_centered_position.y.stl.num = stl_y;
    struct Coord3d next_pos;
    next_pos.x.val = curr_pos.x.val;
    next_pos.y.val = curr_pos.y.val;
    next_pos.z.val = curr_pos.z.val;
    for (int i = 0; i < max_val; i++)
    {
        if ((curr_pos.x.stl.num == stl_x) && (curr_pos.y.stl.num == stl_y)) {
            return i + 1;
        }
        SmallAroundIndex round_idx = small_around_index_in_direction(curr_pos.x.stl.num, curr_pos.y.stl.num, stl_x, stl_y);
        if (hug_can_move_on(thing, curr_pos.x.stl.num, curr_pos.y.stl.num))
        {
            next_pos.x.val = curr_pos.x.val;
            next_pos.y.val = curr_pos.y.val;
            next_pos.z.val = curr_pos.z.val;
            curr_pos.x.stl.num += STL_PER_SLB * (int)small_around[round_idx].delta_x;
            curr_pos.y.stl.num += STL_PER_SLB * (int)small_around[round_idx].delta_y;
        } else
        {
            long hug_val = max_val - i;
            int hug_ret = hug_round(thing, &next_pos, &target_centered_position, round_idx, &hug_val);
            if (hug_ret == -1) {
                return -1;
            }
            i += hug_val;
            if (hug_ret == 1) {
                return i + 1;
            }
            curr_pos.x.val = next_pos.x.val;
            curr_pos.y.val = next_pos.y.val;
            curr_pos.z.val = next_pos.z.val;
        }
    }
    return 0;
}

unsigned short get_hugging_blocked_flags(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlags crt_owner_flags)
{
    struct Coord3d tmpos;
    unsigned short blkflags = 0;
    {
        tmpos.x.val = pos->x.val;
        tmpos.y.val = creatng->mappos.y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, slab_flags, crt_owner_flags) == 4) {
            blkflags |= SlbBloF_WalledX;
        }
    }
    {
        tmpos.x.val = creatng->mappos.x.val;
        tmpos.y.val = pos->y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, slab_flags, crt_owner_flags) == 4) {
            blkflags |= SlbBloF_WalledY;
        }
    }
    if (blkflags == 0)
    {
        tmpos.x.val = pos->x.val;
        tmpos.y.val = pos->y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, slab_flags, crt_owner_flags) == 4) {
            blkflags |= SlbBloF_WalledZ;
        }
    }
    return blkflags;
}

void set_hugging_pos_using_blocked_flags(struct Coord3d *dstpos, struct Thing *creatng, unsigned short block_flags, int nav_radius)
{
    struct Coord3d tmpos;
    int coord;
    tmpos.x.val = creatng->mappos.x.val;
    tmpos.y.val = creatng->mappos.y.val;
    if (block_flags & 1)
    {
        coord = creatng->mappos.x.val;
        if (dstpos->x.val >= coord)
        {
            tmpos.x.val = coord + nav_radius;
            tmpos.x.stl.pos = COORD_PER_STL-1;
            tmpos.x.val -= nav_radius;
        } else
        {
            tmpos.x.val = coord - nav_radius;
            tmpos.x.stl.pos = 1;
            tmpos.x.val += nav_radius;
        }
    }
    if (block_flags & 2)
    {
        coord = creatng->mappos.y.val;
        if (dstpos->y.val >= coord)
        {
            tmpos.y.val = coord + nav_radius;
            tmpos.y.stl.pos = COORD_PER_STL-1;
            tmpos.y.val -= nav_radius;
        } else
        {
            tmpos.y.val = coord - nav_radius;
            tmpos.y.stl.pos = 1;
            tmpos.y.val += nav_radius;
        }
    }
    if (block_flags & 4)
    {
        coord = creatng->mappos.x.val;
        if (dstpos->x.val >= coord)
        {
            tmpos.x.val = coord + nav_radius;
            tmpos.x.stl.pos = COORD_PER_STL-1;
            tmpos.x.val -= nav_radius;
        } else
        {
            tmpos.x.val = coord - nav_radius;
            tmpos.x.stl.pos = 1;
            tmpos.x.val += nav_radius;
        }
        coord = creatng->mappos.y.val;
        if (dstpos->y.val >= coord)
        {
            tmpos.y.val = coord + nav_radius;
            tmpos.y.stl.pos = COORD_PER_STL-1;
            tmpos.y.val -= nav_radius;
        } else
        {
            tmpos.y.val = coord - nav_radius;
            tmpos.y.stl.pos = 1;
            tmpos.y.val += nav_radius;
        }
    }
    tmpos.z.val = get_thing_height_at(creatng, &tmpos);
    dstpos->x.val = tmpos.x.val;
    dstpos->y.val = tmpos.y.val;
    dstpos->z.val = tmpos.z.val;
}

/**
 * Returns the first subtile that the creature will collide with.
 *
 * @param slab_flags Contains slab attribute flags (SlabAttrFlags) passed to this function; flagging the attributes that we want the creature to collide with.
 * @param crt_owner_flags Contains player bitflags (type PlayerBitFlags) passed to this function; this variable is used to check a dungeon wall's owner (if crt_owner_flags is 0, the ownership check is nullified).
*/
static long get_map_index_of_first_block_thing_colliding_with_at(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlags crt_owner_flags)
{

    MapCoordDelta nav_radius = thing_nav_sizexy(creatng) / 2;

    MapSubtlCoord start_stl_x = (pos->x.val - nav_radius) / COORD_PER_STL;
    if (start_stl_x <= 0)
        start_stl_x = 0;
    MapSubtlCoord end_stl_x = (pos->x.val + nav_radius) / COORD_PER_STL + 1;
    if (end_stl_x >= game.map_subtiles_x)
        end_stl_x = game.map_subtiles_x;


    MapSubtlCoord start_stl_y = (pos->y.val - nav_radius) / COORD_PER_STL;
    if (start_stl_y <= 0)
        start_stl_y = 0;
    MapSubtlCoord end_stl_y = (pos->y.val + nav_radius) / COORD_PER_STL + 1;
    if (end_stl_y >= game.map_subtiles_y)
        end_stl_y = game.map_subtiles_y;

    if (start_stl_y >= end_stl_y)
    {
        return -1;
    }
    for(MapSubtlCoord current_stl_y = start_stl_y; current_stl_y < end_stl_y; current_stl_y++)
    {
        for(MapSubtlCoord current_stl_x = start_stl_x; current_stl_x < end_stl_x; current_stl_x++)
        {

            struct Map* mapblk = get_map_block_at(current_stl_x,current_stl_y);
            struct SlabMap* slb = get_slabmap_block(subtile_slab(current_stl_x), subtile_slab(current_stl_y));

            // If the current subtile has none of the attribute flags passed to this function (as slab_flags) and is not ROCK
            // OR the current subtile is a dungeon wall that we should dig through.
            if (((mapblk->flags & slab_flags) == 0 && slb->kind != SlbT_ROCK)
             || ((slab_flags & mapblk->flags & SlbAtFlg_Filled) != 0 && CHECK_SLAB_OWNER))
            {
                // Note: "room pillars" get through the above check.
                // If the subtile is a "room pillar"
                if ((mapblk->flags & SlbAtFlg_IsRoom) && (mapblk->flags & SlbAtFlg_Blocking))
                {
                    return get_subtile_number(current_stl_x,current_stl_y); // then the creature collided with a "room pillar".
                }
                // else there is nothing for the creature to collide with on the subtile (creature can path through subtile).
                continue;  // Continue the loop and check the next subtile.
            }
            // else there is a potential collision.
            // If the subtile is not flagged as a door
            if ((mapblk->flags & SlbAtFlg_IsDoor) == 0)
            {
                return get_subtile_number(current_stl_x,current_stl_y); // then the creature collided with ROCK, or a subtile with any attribute flag that was passed to this function (as slab_flags), or a dungeon wall we aren't allowed to dig.
            }
            // else the subtile is flagged as a door.
            struct Thing *doortng = get_door_for_position(current_stl_x, current_stl_y);
            // If there is no valid door in the subtile, or the door is impassable for the creature
            if (thing_is_invalid(doortng) || !door_will_open_for_thing(doortng, creatng))
            {
                return get_subtile_number(current_stl_x,current_stl_y); // then the creature collided with an invalid door, or a door the creature cannot pass.
            }
            // Else there is nothing for the creature to collide with on the subtile (the subtile has a valid door that the creature can pass).
            // Continue the loop and check the next subtile.
        }
    }
    return -1;
}

static long creature_cannot_move_directly_to_with_collide_sub(struct Thing *creatng, struct Coord3d pos, long slab_flags, PlayerBitFlags crt_owner_flags)
{
    if (thing_in_wall_at(creatng, &pos))
    {
        pos.z.val = subtile_coord(map_subtiles_z,COORD_PER_STL-1);
        MapCoord height = get_thing_height_at(creatng, &pos);
        if ((height >= subtile_coord(map_subtiles_z,COORD_PER_STL-1)) || (height - creatng->mappos.z.val > COORD_PER_STL))
        {
            if (get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, slab_flags, crt_owner_flags) >= 0) {
                return 4;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

long creature_cannot_move_directly_to_with_collide(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlags crt_owner_flags)
{
    MapCoord clpcor;

    struct Coord3d next_pos;
    struct Coord3d prev_pos = creatng->mappos;
    MapCoordDelta dt_x = (prev_pos.x.val - pos->x.val);
    MapCoordDelta dt_y = (prev_pos.y.val - pos->y.val);
    int cannot_mv = 0;
    struct Coord3d orig_pos = creatng->mappos;
    if ((pos->x.stl.num == prev_pos.x.stl.num) || (pos->y.stl.num == prev_pos.y.stl.num))
    {
        // Only one coordinate changed enough to switch subtile - easy path
        cannot_mv = creature_cannot_move_directly_to_with_collide_sub(creatng, *pos, slab_flags, crt_owner_flags);
        return cannot_mv;
    }

    if (cross_x_boundary_first(&prev_pos, pos))
    {
        if (pos->x.val <= prev_pos.x.val)
            clpcor = (prev_pos.x.val & 0xFFFFFF00) - 1;
        else
            clpcor = (prev_pos.x.val + COORD_PER_STL) & 0xFFFFFF00;
        next_pos.x.val = clpcor;
        next_pos.y.val = dt_y * abs(clpcor - orig_pos.x.val) / dt_x + orig_pos.y.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, prev_pos, slab_flags, crt_owner_flags))
        {
        case 0:
            creatng->mappos = next_pos;
            creatng->mappos.z.val = get_thing_height_at(creatng, &next_pos);
            break;
        case 1:
            creatng->mappos = orig_pos;
            cannot_mv = 1;
            break;
        case 4:
            // mappos unchanged - no need to restore
            return 4;
        }

        prev_pos = creatng->mappos;
        if (pos->y.val <= prev_pos.y.val)
            clpcor = (prev_pos.y.val & 0xFFFFFF00) - 1;
        else
            clpcor = (prev_pos.y.val + COORD_PER_STL) & 0xFFFFFF00;
        next_pos.y.val = clpcor;
        next_pos.x.val = dt_x * abs(clpcor - orig_pos.y.val) / dt_y + orig_pos.x.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, slab_flags, crt_owner_flags))
        {
        case 0:
            creatng->mappos = next_pos;
            creatng->mappos.z.val = get_thing_height_at(creatng, &next_pos);
            break;
        case 1:
            creatng->mappos = next_pos;
            creatng->mappos.z.val = 0;
            cannot_mv = 1;
            break;
        case 4:
            creatng->mappos = orig_pos;
            return 4;
        }

        prev_pos = creatng->mappos;
        next_pos.x.val = pos->x.val;
        next_pos.y.val = pos->y.val;
        next_pos.z.val = creatng->mappos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, slab_flags, crt_owner_flags))
        {
        case 0:
            creatng->mappos = orig_pos; // restore mappos
            break;
        case 1:
            creatng->mappos = orig_pos; // restore mappos
            cannot_mv = 1;
            break;
        case 4:
            creatng->mappos = orig_pos;
            return 4;
        }
        return cannot_mv;
    }

    if (cross_y_boundary_first(&prev_pos, pos))
    {
        if (pos->y.val <= prev_pos.y.val)
            clpcor = (prev_pos.y.val & 0xFFFFFF00) - 1;
        else
            clpcor = (prev_pos.y.val + COORD_PER_STL) & 0xFFFFFF00;
        next_pos.y.val = clpcor;
        next_pos.x.val = dt_x * abs(clpcor - orig_pos.y.val) / dt_y + orig_pos.x.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, slab_flags, crt_owner_flags))
        {
        case 0:
            creatng->mappos = next_pos;
            creatng->mappos.z.val = get_thing_height_at(creatng, &next_pos);
            break;
        case 1:
            creatng->mappos = next_pos;
            creatng->mappos.z.val = 0;
            cannot_mv = 1;
            break;
        case 4:
            // mappos unchanged - no need to restore
            return 4;
        }
        prev_pos = creatng->mappos;
        if (pos->x.val <= prev_pos.x.val)
            clpcor = (prev_pos.x.val & 0xFFFFFF00) - 1;
        else
            clpcor = (prev_pos.x.val + COORD_PER_STL) & 0xFFFFFF00;
        next_pos.x.val = clpcor;
        next_pos.y.val = dt_y * abs(clpcor - orig_pos.x.val) / dt_x + orig_pos.y.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, slab_flags, crt_owner_flags))
        {
        case 0:
            creatng->mappos = next_pos;
            creatng->mappos.z.val = get_thing_height_at(creatng, &next_pos);
            break;
        case 1:
            creatng->mappos = next_pos;
            creatng->mappos.z.val = 0;
            cannot_mv = 1;
            break;
        case 4:
            creatng->mappos = orig_pos;
            return 4;
        }
        prev_pos = creatng->mappos;
        next_pos.x.val = pos->x.val;
        next_pos.y.val = pos->y.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, *pos, slab_flags, crt_owner_flags))
        {
        default:
            creatng->mappos = orig_pos; // restore mappos
            break;
        case 1:
            creatng->mappos = orig_pos; // restore mappos
            cannot_mv = 1;
            break;
        case 4:
            creatng->mappos = orig_pos;
            return 4;
        }
        return cannot_mv;
    }

    WARNDBG(3,"While moving %s index %d - crossing two boundaries, but neither is first",thing_model_name(creatng),(int)creatng->index);
    switch (creature_cannot_move_directly_to_with_collide_sub(creatng, *pos, slab_flags, crt_owner_flags))
    {
    default:
        creatng->mappos = orig_pos; // restore mappos
        break;
    case 1:
        creatng->mappos = orig_pos; // restore mappos
        cannot_mv = 1;
        break;
    case 4:
        creatng->mappos = orig_pos;
        return 4;
    }
    return cannot_mv;
}

static TbBool thing_can_continue_direct_line_to(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, long slab_flags, long speed, PlayerBitFlags crt_owner_flags)
{
    long angle = get_angle_xy_to(pos1, pos2);
    struct Coord3d direct_line_position;
    direct_line_position.x.val = pos1->x.val;
    direct_line_position.y.val = pos1->y.val;
    direct_line_position.z.val = pos1->z.val;
    direct_line_position.x.val += distance_with_angle_to_coord_x(speed, angle);
    direct_line_position.y.val += distance_with_angle_to_coord_y(speed, angle);
    direct_line_position.z.val = get_thing_height_at(creatng, &direct_line_position);
    int coord = pos1->x.val;
    if (coord < direct_line_position.x.val) {
        coord += speed;
    } else
    if (coord > direct_line_position.x.val) {
        coord -= speed;
    }
    struct Coord3d horizontal_step_position;
    horizontal_step_position.x.val = coord;
    horizontal_step_position.y.val = pos1->y.val;
    horizontal_step_position.z.val = get_thing_height_at(creatng, &horizontal_step_position);
    coord = pos1->y.val;
    if (coord < direct_line_position.y.val) {
        coord += speed;
    } else
    if (coord > direct_line_position.y.val) {
        coord -= speed;
    }
    struct Coord3d vertical_step_position;
    vertical_step_position.y.val = coord;
    vertical_step_position.x.val = pos1->x.val;
    vertical_step_position.z.val = get_thing_height_at(creatng, &vertical_step_position);
    return creature_cannot_move_directly_to_with_collide(creatng, &horizontal_step_position, slab_flags, crt_owner_flags) != 4
        && creature_cannot_move_directly_to_with_collide(creatng, &vertical_step_position, slab_flags, crt_owner_flags) != 4
        && creature_cannot_move_directly_to_with_collide(creatng, &direct_line_position, slab_flags, crt_owner_flags) != 4;
}



static int get_starting_angle_and_side_of_hug_sub2(
    struct Thing *creatng,
    struct Navigation *navi,
    struct Coord3d *arg_pos,
    long slab_flags,
    int arg_move_angle_xy,
    char side,
    int max_speed,
    int speed,
    PlayerBitFlags crt_owner_flags)
{

    struct Coord3d pos;
    struct Coord3d next_position;
    struct Coord3d saved_creature_position;
    struct Navigation temp_navi;

    int minimum_distance_found = INT_MAX;

    saved_creature_position = creatng->mappos;

    short move_angle_xy = creatng->move_angle_xy;
    memcpy(&temp_navi, navi, sizeof(struct Navigation));
    creatng->move_angle_xy = arg_move_angle_xy;
    navi->side = side;
    navi->dist_to_final_pos = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
    int total_distance_moved = 0;
    int hugging_blocked_flags = get_hugging_blocked_flags(creatng, arg_pos, slab_flags, crt_owner_flags);
    MapCoordDelta nav_radius = thing_nav_sizexy(creatng) / 2;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    if ((hugging_blocked_flags & 1) != 0)
    {
        if (arg_pos->x.val >= creatng->mappos.x.val)
        {
            pos.x.stl.num = creatng->mappos.x.stl.num;
            pos.x.stl.pos = -1;
            pos.x.val -= nav_radius;
        }
        else
        {
            pos.x.stl.num = (creatng->mappos.x.val - nav_radius) >> 8;
            pos.x.stl.pos = 1;
            pos.x.val += nav_radius;
        }
        pos.z.val = get_thing_height_at(creatng, &pos);
    }
    if ((hugging_blocked_flags & 2) != 0)
    {
        if (arg_pos->y.val >= creatng->mappos.y.val)
        {
            pos.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
            pos.y.stl.pos = -1;
            pos.y.val -= nav_radius;
        }
        else
        {
            pos.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
            pos.y.stl.pos = 1;
            pos.y.val += nav_radius;
        }
        pos.z.val = get_thing_height_at(creatng, &pos);
    }
    if ((hugging_blocked_flags & 4) != 0)
    {
        if (arg_pos->x.val >= creatng->mappos.x.val)
        {
            pos.x.stl.num = creatng->mappos.x.stl.num;
            pos.x.stl.pos = -1;
            pos.x.val -= nav_radius;
        }
        else
        {
            pos.x.stl.num = creatng->mappos.x.stl.num;
            pos.x.stl.pos = 1;
            pos.x.val += nav_radius;
        }
        if (arg_pos->y.val >= creatng->mappos.y.val)
        {
            pos.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
            pos.y.stl.pos = -1;
            pos.y.val -= nav_radius;
        }
        else
        {
            pos.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
            pos.y.stl.pos = 1;
            pos.y.val += nav_radius;
        }
        pos.z.val = get_thing_height_at(creatng, &pos);
    }
    int blocked_movement_flags = hugging_blocked_flags;
    *arg_pos = pos;
    if (blocked_movement_flags == 4)
    {
        if (!arg_move_angle_xy || arg_move_angle_xy == ANGLE_SOUTH)
        {
            creatng->mappos.x.val = arg_pos->x.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
        else if (arg_move_angle_xy == ANGLE_EAST || arg_move_angle_xy == ANGLE_WEST)
        {
            creatng->mappos.y.val = arg_pos->y.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
    }
    else
    {
        creatng->mappos = pos;
    }
    int wall_hug_iteration_count = 0;

    short current_move_angle;
    short saved_move_angle;
    short adjusted_move_angle_plus;
    short adjusted_move_angle_minus;
    long wall_hug_angle;
    long temporary_blocked_flags;
    char blocking_flags;
    short position_changed;
    long current_distance_to_final_position;
    long _2d_distance_squared;
    int calculated_movement_cost;
    char position_adjustment_applied;
    int angle_of_wall_hug;

    navi->angle = arg_move_angle_xy;
    while (1)
    {
        position_adjustment_applied = 0;
        if (get_2d_distance_squared(&creatng->mappos, &navi->pos_final) < navi->dist_to_final_pos && thing_can_continue_direct_line_to(creatng, &creatng->mappos, &navi->pos_final, slab_flags, max_speed, crt_owner_flags))
        {
            _2d_distance_squared = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
            goto calculate_final_distance;
        }
        if (wall_hug_iteration_count)
        {
            angle_of_wall_hug = get_angle_of_wall_hug(creatng, slab_flags, speed, crt_owner_flags);
            goto apply_wall_hug_angle;
        }
        current_move_angle = creatng->move_angle_xy;
        saved_move_angle = current_move_angle;
        if (navi->side != 1)
        {
            adjusted_move_angle_minus = current_move_angle - DEGREES_90;
            goto normalize_angle_range;
        }
        adjusted_move_angle_plus = current_move_angle + DEGREES_90;
        creatng->move_angle_xy = adjusted_move_angle_plus;
        if ((unsigned short)adjusted_move_angle_plus >= 0x800u)
        {
            adjusted_move_angle_minus = adjusted_move_angle_plus - DEGREES_360;
        normalize_angle_range:
            creatng->move_angle_xy = adjusted_move_angle_minus;
        }
        wall_hug_angle = get_angle_of_wall_hug(creatng, slab_flags, speed, crt_owner_flags);
        creatng->move_angle_xy = saved_move_angle;
        angle_of_wall_hug = wall_hug_angle;
    apply_wall_hug_angle:
        if (!wall_hug_iteration_count || navi->angle != angle_of_wall_hug)
        {
            next_position.x.val = move_coord_with_angle_x(creatng->mappos.x.val, speed, navi->angle);
            next_position.y.val = move_coord_with_angle_y(creatng->mappos.x.val, speed, navi->angle);
            next_position.z.val = get_thing_height_at(creatng, &next_position);
            if (creature_cannot_move_directly_to_with_collide(creatng, &next_position, slab_flags, crt_owner_flags) == 4)
            {
                temporary_blocked_flags = get_hugging_blocked_flags(creatng, &next_position, slab_flags, IGNORE_SLAB_OWNER_CHECK);
                hugging_blocked_flags = temporary_blocked_flags;
                blocking_flags = temporary_blocked_flags;
                pos.x.val = creatng->mappos.x.val;
                pos.y.val = creatng->mappos.y.val;
                if ((blocking_flags & 1) != 0)
                {
                    if (next_position.x.val >= creatng->mappos.x.val)
                    {
                        pos.x.stl.num = (nav_radius + creatng->mappos.x.val) >> 8;
                        pos.x.stl.pos = -1;
                        pos.x.val -= nav_radius;
                    }
                    else
                    {
                        pos.x.stl.num = (creatng->mappos.x.val - nav_radius) >> 8;
                        pos.x.stl.pos = 1;
                        pos.x.val += nav_radius;
                    }
                    pos.z.val = get_thing_height_at(creatng, &pos);
                }
                if ((hugging_blocked_flags & 2) != 0)
                {
                    if (next_position.y.val >= creatng->mappos.y.val)
                    {
                        pos.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
                        pos.y.stl.pos = -1;
                        pos.y.val -= nav_radius;
                    }
                    else
                    {
                        pos.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
                        pos.y.stl.pos = 1;
                        pos.y.val += nav_radius;
                    }
                    pos.z.val = get_thing_height_at(creatng, &pos);
                }
                if ((hugging_blocked_flags & 4) != 0)
                {
                    if (next_position.x.val >= creatng->mappos.x.val)
                    {
                        pos.x.stl.num = (nav_radius + creatng->mappos.x.val) >> 8;
                        pos.x.stl.pos = -1;
                        pos.x.val -= nav_radius;
                    }
                    else
                    {
                        pos.x.stl.num = (creatng->mappos.x.val - nav_radius) >> 8;
                        pos.x.stl.pos = 1;
                        pos.x.val += nav_radius;
                    }
                    if (next_position.y.val >= (unsigned int)creatng->mappos.y.val)
                    {
                        pos.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
                        pos.y.stl.pos = -1;
                        pos.y.val -= nav_radius;
                    }
                    else
                    {
                        pos.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
                        pos.y.stl.pos = 1;
                        pos.y.val += nav_radius;
                    }
                    pos.z.val = get_thing_height_at(creatng, &pos);
                }
                next_position = pos;
                if (creatng->mappos.x.val != pos.x.val && creatng->mappos.y.val != pos.y.val)
                {
                    creatng->mappos = pos;
                    position_adjustment_applied = 1;
                    navi->distance_to_next_pos = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
                }
            }
        }
        if (!position_adjustment_applied)
        {
            position_changed = angle_of_wall_hug;
            navi->angle = angle_of_wall_hug;
            creatng->move_angle_xy = position_changed;

            next_position.x.val = move_coord_with_angle_x(creatng->mappos.x.val, speed, navi->angle);
            next_position.y.val = move_coord_with_angle_y(creatng->mappos.y.val, speed, navi->angle);
            next_position.z.val = get_thing_height_at(creatng, &next_position);
            check_forward_for_prospective_hugs(
                creatng,
                &next_position,
                (unsigned short)creatng->move_angle_xy,
                navi->side,
                slab_flags,
                speed,
                crt_owner_flags);
            creatng->mappos = next_position;
        }
        total_distance_moved += speed;
        current_distance_to_final_position = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
        if (current_distance_to_final_position < minimum_distance_found)
        {
            minimum_distance_found = current_distance_to_final_position;
            if (current_distance_to_final_position < 0x10000)
                break;
        }
        if (++wall_hug_iteration_count >= 100)
            goto finalize_movement_cost;
    }
    _2d_distance_squared = minimum_distance_found;
calculate_final_distance:
    minimum_distance_found = -_2d_distance_squared;
finalize_movement_cost:
    if (minimum_distance_found >= 0)
        calculated_movement_cost = total_distance_moved * total_distance_moved + minimum_distance_found;
    else
        calculated_movement_cost = minimum_distance_found - total_distance_moved * total_distance_moved;
    minimum_distance_found = calculated_movement_cost;
    creatng->mappos = saved_creature_position;
    creatng->move_angle_xy = move_angle_xy;

    memcpy(navi, &temp_navi, sizeof(struct Navigation));
    return minimum_distance_found;
}

static int get_starting_angle_and_side_of_hug_sub1(
    struct Thing *creatng,
    struct Coord3d *pos,
    long slab_flags,
    PlayerBitFlags crt_owner_flags)
{

    struct Coord3d adjusted_navigation_position;

    int hugging_blocked_flags = get_hugging_blocked_flags(creatng, pos, slab_flags, crt_owner_flags);
    MapCoordDelta nav_radius = thing_nav_sizexy(creatng) / 2;
    adjusted_navigation_position.x.val = creatng->mappos.x.val;
    adjusted_navigation_position.y.val = creatng->mappos.y.val;
    if ((hugging_blocked_flags & 1) != 0)
    {
        if (pos->x.val >= creatng->mappos.x.val)
        {
            adjusted_navigation_position.x.stl.num = (nav_radius + creatng->mappos.x.val) >> 8;
            adjusted_navigation_position.x.stl.pos = -1;
            adjusted_navigation_position.x.val -= nav_radius;
        }
        else
        {
            adjusted_navigation_position.x.stl.num = (creatng->mappos.x.val - nav_radius) >> 8;
            adjusted_navigation_position.x.stl.pos = 1;
            adjusted_navigation_position.x.val += nav_radius;
        }
        adjusted_navigation_position.z.val = get_thing_height_at(creatng, &adjusted_navigation_position);
    }
    if ((hugging_blocked_flags & 2) != 0)
    {
        if (pos->y.val >= creatng->mappos.y.val)
        {
            adjusted_navigation_position.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
            adjusted_navigation_position.y.stl.pos = -1;
            adjusted_navigation_position.y.val -= nav_radius;
        }
        else
        {
            adjusted_navigation_position.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
            adjusted_navigation_position.y.stl.pos = 1;
            adjusted_navigation_position.y.val += nav_radius;
        }
        adjusted_navigation_position.z.val = get_thing_height_at(creatng, &adjusted_navigation_position);
    }
    if ((hugging_blocked_flags & 4) != 0)
    {
        if (pos->x.val >= creatng->mappos.x.val)
        {
            adjusted_navigation_position.x.stl.num = (nav_radius + creatng->mappos.x.val) >> 8;
            adjusted_navigation_position.x.stl.pos = -1;
            adjusted_navigation_position.x.val -= nav_radius;
        }
        else
        {
            adjusted_navigation_position.x.stl.num = (creatng->mappos.x.val - nav_radius) >> 8;
            adjusted_navigation_position.x.stl.pos = 1;
            adjusted_navigation_position.x.val += nav_radius;
        }
        if (pos->y.val >= creatng->mappos.y.val)
        {
            adjusted_navigation_position.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
            adjusted_navigation_position.y.stl.pos = -1;
            adjusted_navigation_position.y.val -= nav_radius;
        }
        else
        {
            adjusted_navigation_position.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
            adjusted_navigation_position.y.stl.pos = 1;
            adjusted_navigation_position.y.val += nav_radius;
        }
        adjusted_navigation_position.z.val = get_thing_height_at(creatng, &adjusted_navigation_position);
    }
    *pos = adjusted_navigation_position;
    return hugging_blocked_flags;
}

static signed char get_starting_angle_and_side_of_hug(
    struct Thing *creatng,
    struct Coord3d *pos,
    long *angle,
    unsigned char *side,
    long slab_flags,
    PlayerBitFlags crt_owner_flags)
{
    int y_distance_to_final;
    int x_distance_to_final;
    char hugging_blocked_flags;
    int movement_direction_index;
    int primary_priority_index;
    int secondary_priority_index;
    char secondary_side_priority;
    int base_movement_index;
    int current_move_angle;
    int saved_move_angle;
    int adjusted_move_angle_plus;
    int adjusted_move_angle_minus;
    int wall_hug_angle;
    int pathfinding_iteration_count;
    int32_t angle_of_wall_hug;
    int16_t saved_creature_angle;
    int16_t adjusted_positive_angle;
    int16_t adjusted_angle;
    int32_t _2d_distance_squared;
    int calculated_cost_with_distance;
    int alternative_pathfinding_cost;
    int8_t result;
    char position_changed;
    uint8_t primary_side_priority;
    char selected_side_priority;
    int16_t move_angle_xy;
    uint16_t navigation_angle;
    int x_direction_flag;
    uint16_t calculated_angle;
    int16_t stored_move_angle;
    int best_distance_squared;
    int primary_pathfinding_cost;
    struct Coord3d target_movement_position;
    int movement_angle_difference;
    char v44_2;
    int accumulated_movement_distance;
    struct Coord3d next_movement_position;
    struct Navigation backup_navigation_state;


    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    struct Navigation *navi = &cctrl->navi;
    const short max_speed = cctrl->max_speed;

    target_movement_position.x.stl.pos = creatng->mappos.y.val - (uint16_t)pos->y.val <= 0;
    x_direction_flag = (uint16_t)creatng->mappos.x.val - (uint16_t)pos->x.val <= 0;
    y_distance_to_final = creatng->mappos.y.val - navi->pos_final.y.val;
    backup_navigation_state.navstate = y_distance_to_final <= NavS_NavigationDisabled;
    x_distance_to_final = (uint16_t)creatng->mappos.x.val - navi->pos_final.x.val;
    next_movement_position.x.stl.pos = x_distance_to_final <= 0;
    v44_2 = (int)abs(x_distance_to_final) < (int)abs(y_distance_to_final);
    hugging_blocked_flags = get_hugging_blocked_flags(creatng, pos, slab_flags, crt_owner_flags);
    if ((hugging_blocked_flags & 1) != 0)
    {
        movement_direction_index = 2 * x_direction_flag;
        primary_priority_index = movement_direction_index + (uint8_t)backup_navigation_state.navstate;
        calculated_angle = blocked_x_hug_start[0][primary_priority_index].wh_angle;
        primary_side_priority = wallhug_x_blocked_priorities[3 * primary_priority_index];
        secondary_priority_index = movement_direction_index + (backup_navigation_state.navstate == NavS_NavigationDisabled);
        navigation_angle = blocked_x_hug_start[0][secondary_priority_index].wh_angle;
        secondary_side_priority = wallhug_x_blocked_priorities[3 * secondary_priority_index];
    }
    else if ((hugging_blocked_flags & 2) != 0)
    {
        base_movement_index = 2 * (uint8_t)target_movement_position.x.stl.pos;
        current_move_angle = base_movement_index + (unsigned char)next_movement_position.x.stl.pos;
        calculated_angle = blocked_y_hug_start[0][current_move_angle].wh_angle;
        primary_side_priority = wallhug_y_blocked_priorities[3 * current_move_angle];
        saved_move_angle = base_movement_index + (next_movement_position.x.stl.pos == 0);
        navigation_angle = blocked_y_hug_start[0][saved_move_angle].wh_angle;
        secondary_side_priority = wallhug_y_blocked_priorities[3 * saved_move_angle];
    }
    else
    {
        if ((hugging_blocked_flags & 4) == 0)
        {
            ERRORLOG("Illegal block direction for lookahead");
            return 0;
        }
        adjusted_move_angle_plus = 2 * (x_direction_flag + 2 * (uint8_t)target_movement_position.x.stl.pos);
        adjusted_move_angle_minus = adjusted_move_angle_plus + v44_2;
        calculated_angle = blocked_xy_hug_start[0][0][adjusted_move_angle_minus].wh_angle;
        primary_side_priority = wallhug_xy_blocked_priorities[3 * adjusted_move_angle_minus];
        wall_hug_angle = adjusted_move_angle_plus + (v44_2 == 0);
        navigation_angle = blocked_xy_hug_start[0][0][wall_hug_angle].wh_angle;
        secondary_side_priority = wallhug_xy_blocked_priorities[3 * wall_hug_angle];
    }
    best_distance_squared = 0x7FFFFFFF;
    selected_side_priority = secondary_side_priority;
    next_movement_position.x.val = creatng->mappos.x.val;
    next_movement_position.y.val = creatng->mappos.y.val;
    next_movement_position.z.val = creatng->mappos.z.val;
    move_angle_xy = creatng->move_angle_xy;
    memcpy(&backup_navigation_state, navi, sizeof(struct Navigation));
    creatng->move_angle_xy = calculated_angle;
    navi->side = primary_side_priority;
    navi->dist_to_final_pos = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
    accumulated_movement_distance = 0;
    if (get_starting_angle_and_side_of_hug_sub1(creatng, pos, slab_flags, crt_owner_flags) == 4)
    {
        if (calculated_angle == ANGLE_NORTH || calculated_angle == ANGLE_SOUTH)
        {
            creatng->mappos.x.val = pos->x.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
        else if (calculated_angle == ANGLE_WEST || calculated_angle == ANGLE_EAST)
        {
            creatng->mappos.y.val = pos->y.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
    }
    else
    {
        creatng->mappos = *pos;
    }
    pathfinding_iteration_count = 0;
    movement_angle_difference = calculated_angle;
    navi->angle = calculated_angle;
    while (1)
    {
        position_changed = 0;
        if (get_2d_distance_squared(&creatng->mappos, &navi->pos_final) < navi->dist_to_final_pos)
        {
            if (thing_can_continue_direct_line_to(creatng, &creatng->mappos, &navi->pos_final, slab_flags, max_speed, crt_owner_flags))
                break;
        }
        if (pathfinding_iteration_count)
        {
            angle_of_wall_hug = get_angle_of_wall_hug(creatng, slab_flags, 255, crt_owner_flags);
            goto apply_pathfinding_angle;
        }
        saved_creature_angle = creatng->move_angle_xy;
        stored_move_angle = saved_creature_angle;
        if (navi->side != 1)
        {
            adjusted_angle = saved_creature_angle - DEGREES_90;
            goto normalize_pathfinding_angle;
        }
        adjusted_positive_angle = saved_creature_angle + DEGREES_90;
        creatng->move_angle_xy = adjusted_positive_angle;
        if ((unsigned short)adjusted_positive_angle >= 0x800u)
        {
            adjusted_angle = adjusted_positive_angle - DEGREES_360;
        normalize_pathfinding_angle:
            creatng->move_angle_xy = adjusted_angle;
        }
        angle_of_wall_hug = get_angle_of_wall_hug(creatng, slab_flags, 255, crt_owner_flags);
        creatng->move_angle_xy = stored_move_angle;
    apply_pathfinding_angle:
        if (!pathfinding_iteration_count || navi->angle != angle_of_wall_hug)
        {
            target_movement_position.x.val = move_coord_with_angle_x(creatng->mappos.x.val, COORD_PER_STL, navi->angle);
            target_movement_position.y.val = move_coord_with_angle_y(creatng->mappos.y.val, COORD_PER_STL, navi->angle);
            target_movement_position.z.val = get_thing_height_at(creatng, &target_movement_position);
            if (creature_cannot_move_directly_to_with_collide(creatng, &target_movement_position, slab_flags, crt_owner_flags) == 4)
            {
                get_starting_angle_and_side_of_hug_sub1(creatng, &target_movement_position, slab_flags, IGNORE_SLAB_OWNER_CHECK);
                if (creatng->mappos.x.val != target_movement_position.x.val || creatng->mappos.y.val != target_movement_position.y.val)
                {
                    creatng->mappos = target_movement_position;
                    position_changed = 1;
                    navi->distance_to_next_pos = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
                }
            }
        }
        if (!position_changed)
        {
            navi->angle = angle_of_wall_hug;
            creatng->move_angle_xy = angle_of_wall_hug;
            target_movement_position.x.val = move_coord_with_angle_x(creatng->mappos.x.val, COORD_PER_STL, navi->angle);
            target_movement_position.y.val = move_coord_with_angle_y(creatng->mappos.y.val, COORD_PER_STL, navi->angle);
            target_movement_position.z.val = get_thing_height_at(creatng, &target_movement_position);
            check_forward_for_prospective_hugs(
                creatng,
                &target_movement_position,
                (unsigned short)creatng->move_angle_xy,
                navi->side,
                slab_flags,
                255,
                crt_owner_flags);
            creatng->mappos = target_movement_position;
        }
        accumulated_movement_distance += 255;
        _2d_distance_squared = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
        if (_2d_distance_squared < best_distance_squared)
        {
            best_distance_squared = _2d_distance_squared;
            if (_2d_distance_squared < 0x10000)
                goto calculate_best_distance;
        }
        if (++pathfinding_iteration_count >= 100)
            goto finalize_pathfinding_cost;
    }
    _2d_distance_squared = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
calculate_best_distance:
    best_distance_squared = -_2d_distance_squared;
finalize_pathfinding_cost:
    if (best_distance_squared >= 0)
        calculated_cost_with_distance = accumulated_movement_distance * accumulated_movement_distance + best_distance_squared;
    else
        calculated_cost_with_distance = best_distance_squared - accumulated_movement_distance * accumulated_movement_distance;
    primary_pathfinding_cost = calculated_cost_with_distance;
    creatng->mappos.x.val = next_movement_position.x.val;
    creatng->mappos.y.val = next_movement_position.y.val;
    creatng->mappos.z.val = next_movement_position.z.val;
    creatng->move_angle_xy = move_angle_xy;
    memcpy(navi, &backup_navigation_state, sizeof(struct Navigation));
    alternative_pathfinding_cost = get_starting_angle_and_side_of_hug_sub2(creatng, navi, pos, slab_flags, navigation_angle, selected_side_priority, max_speed, 255, crt_owner_flags);
    if (primary_pathfinding_cost >= 0)
    {
        if (alternative_pathfinding_cost >= 0)
        {
            if (alternative_pathfinding_cost <= primary_pathfinding_cost)
            {
                *angle = navigation_angle;
                result = 1;
                *side = selected_side_priority;
            }
            else
            {
                *angle = movement_angle_difference;
                *side = primary_side_priority;
                return 1;
            }
        }
        else
        {
            *angle = navigation_angle;
            result = 1;
            *side = selected_side_priority;
        }
    }
    else if (alternative_pathfinding_cost >= 0)
    {
        *angle = movement_angle_difference;
        *side = primary_side_priority;
        return 1;
    }
    else if (alternative_pathfinding_cost <= primary_pathfinding_cost)
    {
        *angle = movement_angle_difference;
        *side = primary_side_priority;
        return 1;
    }
    else
    {
        *angle = navigation_angle;
        result = 1;
        *side = selected_side_priority;
    }
    return result;
}

static TbBool check_forward_for_prospective_hugs(struct Thing *creatng, struct Coord3d *pos_a, long angle, long side, long slab_flags, long speed, PlayerBitFlags crt_owner_flags)
{
    int quadrant_angle;
    struct Coord3d pos;
    struct Coord3d next_pos;
    struct Coord3d stored_creature_pos;

    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    struct Navigation *navi = &cctrl->navi;
    MapCoordDelta nav_radius = thing_nav_sizexy(creatng) / 2;
    switch (angle)
    {
        case ANGLE_NORTH:
            if ((int)((pos_a->y.val - nav_radius) & 0xFFFFFF00) < (int)((creatng->mappos.y.val - nav_radius) & 0xFFFFFF00))
            {
                pos.x.val = pos_a->x.val;
                pos.y.val = nav_radius + creatng->mappos.y.val - COORD_PER_STL;
                pos.y.stl.pos = (COORD_PER_STL-1);
                pos.y.val -= nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        case ANGLE_EAST:
            if ((int)((nav_radius + pos_a->x.val) & 0xFFFFFF00) > (int)((nav_radius + creatng->mappos.x.val) & 0xFFFFFF00))
            {
                pos.y.val = pos_a->y.val;
                pos.x.val = creatng->mappos.x.val - nav_radius + COORD_PER_STL;
                pos.x.stl.pos = 0;
                pos.x.val += nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        case ANGLE_SOUTH:
            if ((int)((nav_radius + pos_a->y.val) & 0xFFFFFF00) > (int)((nav_radius + creatng->mappos.y.val) & 0xFFFFFF00))
            {
                pos.x.val = pos_a->x.val;
                pos.y.val = creatng->mappos.y.val - nav_radius + COORD_PER_STL;
                pos.y.stl.pos = 0;
                pos.y.val += nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        case ANGLE_WEST:
            if ((int)((pos_a->x.val - nav_radius) & 0xFFFFFF00) < (int)((creatng->mappos.x.val - nav_radius) & 0xFFFFFF00))
            {
                pos.y.val = pos_a->y.val;
                pos.x.val = nav_radius + creatng->mappos.x.val - COORD_PER_STL;
                pos.x.stl.pos = (COORD_PER_STL-1);
                pos.x.val -= nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        default:
            return false;
    }
    if ( navi->side == 1 )
    {
        quadrant_angle = (((unsigned char)angle_to_quadrant(angle) - 1) & 3) * DEGREES_90;

        next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
        next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
        next_pos.z.val = get_thing_height_at(creatng, &next_pos);
        if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, slab_flags, crt_owner_flags) == 4)
        {
            stored_creature_pos = creatng->mappos;
            creatng->mappos.x.val = pos.x.val;
            creatng->mappos.y.val = pos.y.val;
            creatng->mappos.z.val = pos.z.val;
            quadrant_angle = (((unsigned char)angle_to_quadrant(angle) - 1) & 3) * DEGREES_90;
            next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
            next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
            next_pos.z.val = get_thing_height_at(creatng, &next_pos);
            if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, slab_flags, crt_owner_flags) != 4)
            {
                *pos_a = pos;
                creatng->mappos = stored_creature_pos;
                return true;
            }
            creatng->mappos = stored_creature_pos;
        }
    }
    if ( navi->side != 2 )
        return false;
    quadrant_angle = (((unsigned char)angle_to_quadrant(angle) + 1) & 3) * DEGREES_90;
    next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
    next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
    next_pos.z.val = get_thing_height_at(creatng, &next_pos);
    if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, slab_flags, crt_owner_flags) != 4)
        return false;
    stored_creature_pos = creatng->mappos;
    creatng->mappos = pos;
    quadrant_angle = (((unsigned char)angle_to_quadrant(angle) + 1) & 3) * DEGREES_90;
    next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
    next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
    next_pos.z.val = get_thing_height_at(creatng, &next_pos);


    if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, slab_flags, crt_owner_flags) == 4)
    {
        creatng->mappos = stored_creature_pos;
        return false;
    }
    *pos_a = pos;
    creatng->mappos = stored_creature_pos;
    return true;
}

static TbBool find_approach_position_to_subtile(const struct Coord3d *srcpos, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MoveSpeed spacing, struct Coord3d *aproachpos)
{
    struct Coord3d targetpos;
    targetpos.x.val = subtile_coord_center(stl_x);
    targetpos.y.val = subtile_coord_center(stl_y);
    targetpos.z.val = 0;
    long min_dist = LONG_MAX;
    for (SmallAroundIndex n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long dx = spacing * (long)small_around[n].delta_x;
        long dy = spacing * (long)small_around[n].delta_y;
        struct Coord3d tmpos;
        tmpos.x.val = targetpos.x.val + dx;
        tmpos.y.val = targetpos.y.val + dy;
        tmpos.z.val = 0;
        struct Map* mapblk = get_map_block_at(tmpos.x.stl.num, tmpos.y.stl.num);
        if ((!map_block_invalid(mapblk)) && ((mapblk->flags & SlbAtFlg_Blocking) == 0))
        {
            MapCoordDelta dist = get_chessboard_distance(srcpos, &tmpos);
            if (min_dist > dist)
            {
                min_dist = dist;
                aproachpos->x.val = tmpos.x.val;
                aproachpos->y.val = tmpos.y.val;
                aproachpos->z.val = tmpos.z.val;
            }
        }
    }
    return (min_dist < LONG_MAX);
}

static SubtlCodedCoords get_map_index_of_first_block_thing_colliding_with_travelling_to(struct Thing *creatng, struct Coord3d *startpos, struct Coord3d *endpos, long slab_flags, PlayerBitFlags crt_owner_flags)
{
    SubtlCodedCoords stl_num;
    struct Coord3d pos;
    SubtlCodedCoords return_stl_num = 0;

    struct Coord3d creature_pos = *startpos;

    MapCoordDelta delta_x = creature_pos.x.val - endpos->x.val;
    MapCoordDelta delta_y = creature_pos.y.val - endpos->y.val;

    MapCoord reference_x = creatng->mappos.x.val;
    MapCoord reference_y = creatng->mappos.y.val;
    struct Coord3d orig_creat_pos = creatng->mappos;

    if (endpos->x.stl.num == creature_pos.x.stl.num || endpos->y.stl.num == creature_pos.y.stl.num)
    {
        stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, endpos, slab_flags, crt_owner_flags);
        if (stl_num >= 0)
        {
            return_stl_num = stl_num;
        }
        creatng->mappos = orig_creat_pos;
        return return_stl_num;
    }
    if (!cross_x_boundary_first(&creature_pos, endpos))
    {
        if (cross_y_boundary_first(&creature_pos, endpos))
        {
            pos = creature_pos;
            if (endpos->y.val <= creature_pos.y.val)
            {
                pos.y.stl.num = creature_pos.y.stl.num - 1;
                pos.y.stl.pos = COORD_PER_STL - 1;
            }
            else
            {
                pos.y.stl.num = creature_pos.y.stl.num + 1;
                pos.y.stl.pos = 0;
            }
            pos.x.val = (int)(delta_x * abs(pos.y.val - reference_x)) / delta_y + reference_y;

            pos.z.val = creature_pos.z.val;
            stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, slab_flags, crt_owner_flags);
            if (stl_num >= 0)
            {
                creatng->mappos = orig_creat_pos;
                return stl_num;
            }
            creature_pos = creatng->mappos;
            if (endpos->x.val <= creature_pos.x.val)
            {
                pos.x.stl.num = creature_pos.x.stl.num - 1;
                pos.x.stl.pos = COORD_PER_STL - 1;
            }
            else
            {
                pos.x.stl.num = creature_pos.x.stl.num + 1;
                pos.x.stl.pos = 0;
            }

            pos.y.val = (int)(delta_y * abs(pos.x.val - reference_x)) / delta_x + reference_y;
            pos.z.val = creature_pos.z.val;
            stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, slab_flags, crt_owner_flags);
            if (stl_num >= 0)
            {
                creatng->mappos = orig_creat_pos;
                return stl_num;
            }
            creature_pos = creatng->mappos;
            pos.x.val = endpos->x.val;
            pos.y = endpos->y;
            pos.z = creatng->mappos.z;
            stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, slab_flags, crt_owner_flags);
            if (stl_num >= 0)
            {
                return_stl_num = stl_num;
            }
            creatng->mappos = orig_creat_pos;
            return return_stl_num;
        }
        stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, endpos, slab_flags, crt_owner_flags);
        if (stl_num >= 0)
        {
            return_stl_num = stl_num;
        }
        creatng->mappos = orig_creat_pos;
        return stl_num;
    }
    if (endpos->x.val <= creature_pos.x.val)
    {
        pos.x.stl.num = creature_pos.x.stl.num - 1;
        pos.x.stl.pos = COORD_PER_STL - 1;
    }
    else
    {
        pos.x.stl.num = creature_pos.x.stl.num + 1;
        pos.x.stl.pos = 0;
    }
    pos.y.val = (int)(delta_y * abs(pos.x.val - reference_y)) / delta_x + reference_x;
    pos.z.val = creature_pos.z.val;
    stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, slab_flags, crt_owner_flags);
    if (stl_num >= 0)
    {
        creatng->mappos = orig_creat_pos;
        return stl_num;
    }
    creature_pos = creatng->mappos;
    if (endpos->y.val <= creature_pos.y.val)
    {
        pos.y.stl.num = creature_pos.y.stl.num - 1;
        pos.y.stl.pos = COORD_PER_STL - 1;
    }
    else
    {
        pos.y.stl.num = creature_pos.y.stl.num + 1;
        pos.y.stl.pos = 0;
    }
    pos.x.val = (int)(delta_x * abs(pos.y.val - reference_x)) / delta_y + reference_y;
    pos.z.val = creature_pos.z.val;
    stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, slab_flags, crt_owner_flags);
    if (stl_num >= 0)
    {
        creatng->mappos = orig_creat_pos;
        return stl_num;
    }

    creature_pos = creatng->mappos;
    pos = *endpos;
    pos.z.val = creatng->mappos.z.val;

    stl_num = get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, slab_flags, crt_owner_flags);
    if (stl_num >= 0)
    {
        return_stl_num = stl_num;
    }
    creatng->mappos = orig_creat_pos;
    return return_stl_num;
}

static TbBool navigation_push_towards_target(struct Navigation *navi, struct Thing *creatng, const struct Coord3d *pos, MoveSpeed speed, MoveSpeed nav_radius, PlayerBitFlags crt_owner_flags)
{
    navi->navstate = NavS_InitialWallhugSetup;
    navi->pos_next.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
    navi->pos_next.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
    navi->pos_next.z.val = get_thing_height_at(creatng, &navi->pos_next);
    struct Coord3d pos1;
    pos1.x.val = navi->pos_next.x.val;
    pos1.y.val = navi->pos_next.y.val;
    pos1.z.val = navi->pos_next.z.val;
    check_forward_for_prospective_hugs(creatng, &pos1, navi->angle, navi->side, SlbAtFlg_Filled|SlbAtFlg_Valuable, speed, crt_owner_flags);
    if (get_chessboard_distance(&pos1, &creatng->mappos) > 16)
    {
        navi->pos_next.x.val = pos1.x.val;
        navi->pos_next.y.val = pos1.y.val;
        navi->pos_next.z.val = pos1.z.val;
    }
    navi->distance_to_next_pos = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
    int cannot_move = creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags);

    if (cannot_move == 4)
    {
        navi->pos_next.x.val = creatng->mappos.x.val;
        navi->pos_next.y.val = creatng->mappos.y.val;
        navi->pos_next.z.val = creatng->mappos.z.val;
        navi->distance_to_next_pos = 0;
    }
    navi->dist_to_final_pos = get_chessboard_distance(&creatng->mappos, pos);
    if (cannot_move == 1)
    {
        SubtlCodedCoords stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Digable, IGNORE_SLAB_OWNER_CHECK);
        navi->first_colliding_block = stl_num;
        MapSubtlCoord stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(stl_num)));
        MapSubtlCoord stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(stl_num)));
        find_approach_position_to_subtile(&creatng->mappos, stl_x, stl_y, nav_radius + 385, &navi->pos_next);
        navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
        navi->navstate = NavS_WallhugDirectionCheck;
    }
    return true;
}

long get_next_position_and_angle_required_to_tunnel_creature_to(struct Thing *creatng, struct Coord3d *pos, PlayerBitFlags crt_owner_flags)
{
    struct Navigation *navi;
    int speed;
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        navi = &cctrl->navi;
        speed = cctrl->max_speed;
        cctrl->creature_state_flags = 0;
        cctrl->combat_flags = 0;
    }
    set_flag(crt_owner_flags, to_flag(creatng->owner));
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    SubtlCodedCoords stl_num;
    MapCoordDelta dist_to_next;
    struct Coord3d tmpos;
    int nav_radius;
    long angle;
    int block_flags;
    int cannot_move;
    struct Map *mapblk;
    switch (navi->navstate)
    {
    case NavS_WallhugInProgress:
        dist_to_next = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next >= navi->distance_to_next_pos) {
            navi->push_counter = 0;
        }
        if (navi->push_counter == 0)
        {
            navi->angle = get_angle_xy_to(&creatng->mappos, pos);
            navi->pos_next.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
            navi->pos_next.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
            navi->pos_next.z.val = get_thing_height_at(creatng, &navi->pos_next);
            if (get_chessboard_distance(&creatng->mappos, pos) < get_chessboard_distance(&creatng->mappos, &navi->pos_next))
            {
                navi->pos_next.x.val = pos->x.val;
                navi->pos_next.y.val = pos->y.val;
                navi->pos_next.z.val = pos->z.val;
            }

            cannot_move = creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags);
            if (cannot_move == 4)
            {
                struct SlabMap *slb;
                stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Digable, IGNORE_SLAB_OWNER_CHECK);
                slb = get_slabmap_for_subtile(stl_num_decode_x(stl_num), stl_num_decode_y(stl_num));
                PlayerBitFlags ownflag;
                ownflag = 0;
                if (!slabmap_block_invalid(slb)) {
                    ownflag = to_flag(slabmap_owner(slb));
                }
                navi->owner_flags[0] = ownflag;

                if (get_starting_angle_and_side_of_hug(creatng, &navi->pos_next, &navi->angle, &navi->side, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags))
                {
                    block_flags = get_hugging_blocked_flags(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags);
                    set_hugging_pos_using_blocked_flags(&navi->pos_next, creatng, block_flags, thing_nav_sizexy(creatng)/2);
                    if (block_flags == 4)
                    {
                        if ((navi->angle == ANGLE_NORTH) || (navi->angle == ANGLE_SOUTH))
                        {
                            navi->pos_next.y.val = creatng->mappos.y.val;
                            navi->pos_next.z.val = get_thing_height_at(creatng, &creatng->mappos);
                        } else
                        if ((navi->angle == ANGLE_EAST) || (navi->angle == ANGLE_WEST)) {
                            navi->pos_next.x.val = creatng->mappos.x.val;
                            navi->pos_next.z.val = get_thing_height_at(creatng, &creatng->mappos);
                        }
                    }
                    navi->push_counter = 1;
                } else
                {
                    navi->navstate = NavS_WallhugInProgress;
                    navi->pos_final.x.val = pos->x.val;
                    navi->pos_final.y.val = pos->y.val;
                    navi->pos_final.z.val = pos->z.val;
                    navi->wallhug_state = 0;
                    navi->wallhug_retry_counter = 0;
                    navi->push_counter = 0;
                }
            }
            if (cannot_move == 1)
            {
                stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Digable, IGNORE_SLAB_OWNER_CHECK);
                navi->first_colliding_block = stl_num;
                nav_radius = thing_nav_sizexy(creatng) / 2;
                stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(stl_num)));
                stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(stl_num)));
                find_approach_position_to_subtile(&creatng->mappos, stl_x, stl_y, nav_radius + 385, &navi->pos_next);
                navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
                navi->navstate = NavS_WallhugDirectionCheck;
                return 1;
            }
        }
        if (navi->push_counter > 0)
        {
            navi->push_counter++;
            if (navi->push_counter > 32) {
                ERRORLOG("I've been pushing for a very long time now...");
            }
            if (get_chessboard_distance(&creatng->mappos, &navi->pos_next) <= 16)
            {
                navi->push_counter = 0;
                navigation_push_towards_target(navi, creatng, pos, speed, thing_nav_sizexy(creatng)/2, crt_owner_flags);
            }
        }
        return 1;
    case NavS_InitialWallhugSetup:
        dist_to_next = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            if ((dist_to_next > navi->distance_to_next_pos) || creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags))
            {
                navi->navstate = NavS_WallhugInProgress;
                navi->pos_final.x.val = pos->x.val;
                navi->pos_final.y.val = pos->y.val;
                navi->pos_final.z.val = pos->z.val;
                navi->wallhug_state = 0;
                navi->wallhug_retry_counter = 0;
                navi->push_counter = 0;
                return 1;
            }
            return 1;
        }
        if ((get_chessboard_distance(&creatng->mappos, pos) < navi->dist_to_final_pos)
          && thing_can_continue_direct_line_to(creatng, &creatng->mappos, pos, SlbAtFlg_Filled|SlbAtFlg_Valuable, 1, crt_owner_flags))

        {
            navi->navstate = NavS_WallhugInProgress;
            navi->pos_final.x.val = pos->x.val;
            navi->pos_final.y.val = pos->y.val;
            navi->pos_final.z.val = pos->z.val;
            navi->wallhug_state = 0;
            navi->wallhug_retry_counter = 0;
            navi->push_counter = 0;
            return 1;
        }
        if (creatng->move_angle_xy != navi->angle) {
            return 1;
        }
        angle = get_angle_of_wall_hug(creatng, SlbAtFlg_Filled|SlbAtFlg_Valuable, speed, crt_owner_flags);
        if (angle != navi->angle)
        {
          tmpos.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
          tmpos.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
          tmpos.z.val = get_thing_height_at(creatng, &tmpos);
          if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags) == 4)
          {
              block_flags = get_hugging_blocked_flags(creatng, &tmpos, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags);
              set_hugging_pos_using_blocked_flags(&tmpos, creatng, block_flags, thing_nav_sizexy(creatng)/2);
              if (get_chessboard_distance(&tmpos, &creatng->mappos) > 16)
              {
                  navi->pos_next.x.val = tmpos.x.val;
                  navi->pos_next.y.val = tmpos.y.val;
                  navi->pos_next.z.val = tmpos.z.val;
                  navi->distance_to_next_pos = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
                  return 1;
              }
          }
        }
        if (((angle + DEGREES_90) & ANGLE_MASK) == navi->angle)
        {
            if (navi->wallhug_state == 1)
            {
                navi->wallhug_retry_counter++;
            } else
            {
                navi->wallhug_state = 1;
                navi->wallhug_retry_counter = 1;
            }
        } else
        if (((angle - DEGREES_90) & ANGLE_MASK) == navi->angle)
        {
          if (navi->wallhug_state == 2)
          {
              navi->wallhug_retry_counter++;
          } else
          {
              navi->wallhug_state = 2;
              navi->wallhug_retry_counter = 1;
          }
        } else
        {
          navi->wallhug_retry_counter = 0;
          navi->wallhug_state = 0;
        }
        if (navi->wallhug_retry_counter >= 4)
        {
            navi->navstate = NavS_WallhugInProgress;
            navi->pos_final.x.val = pos->x.val;
            navi->pos_final.y.val = pos->y.val;
            navi->pos_final.z.val = pos->z.val;
            navi->wallhug_state = 0;
            navi->wallhug_retry_counter = 0;
            navi->push_counter = 0;
            return 1;
        }
        navi->angle = angle;
        navi->pos_next.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
        navi->pos_next.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
        navi->pos_next.z.val = get_thing_height_at(creatng, &navi->pos_next);
        tmpos.x.val = navi->pos_next.x.val;
        tmpos.y.val = navi->pos_next.y.val;
        tmpos.z.val = navi->pos_next.z.val;
        check_forward_for_prospective_hugs(creatng, &tmpos, navi->angle, navi->side, SlbAtFlg_Filled|SlbAtFlg_Valuable, speed, crt_owner_flags);

        if (get_chessboard_distance(&tmpos, &creatng->mappos) > 16)
        {
            navi->pos_next.x.val = tmpos.x.val;
            navi->pos_next.y.val = tmpos.y.val;
            navi->pos_next.z.val = tmpos.z.val;
        }
        navi->distance_to_next_pos = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
        cannot_move = creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags);
        if (cannot_move == 4)
        {
          ERRORLOG("I've been given a shite position");
          tmpos.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
          tmpos.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
          tmpos.z.val = get_thing_height_at(creatng, &tmpos);
          if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags) == 4) {
              ERRORLOG("It's even more shit than I first thought");
          }
          navi->navstate = NavS_WallhugInProgress;
          navi->pos_final.x.val = pos->x.val;
          navi->pos_final.y.val = pos->y.val;
          navi->pos_final.z.val = pos->z.val;
          navi->wallhug_state = 0;
          navi->wallhug_retry_counter = 0;
          navi->push_counter = 0;
          navi->pos_next.x.val = creatng->mappos.x.val;
          navi->pos_next.y.val = creatng->mappos.y.val;
          navi->pos_next.z.val = creatng->mappos.z.val;
          return 1;
        }
        if (cannot_move != 1)
        {
            navi->distance_to_next_pos = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
            return 1;
        }
        stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Digable, IGNORE_SLAB_OWNER_CHECK);
        navi->first_colliding_block = stl_num;
        nav_radius = thing_nav_sizexy(creatng) / 2;
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(stl_num)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(stl_num)));
        find_approach_position_to_subtile(&creatng->mappos, stl_x, stl_y, nav_radius + 385, &navi->pos_next);
        navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
        navi->wallhug_retry_counter = 0;
        navi->wallhug_state = 0;
        navi->distance_to_next_pos = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
        navi->navstate = NavS_WallhugPositionAdjust;
        return 1;
    case NavS_WallhugPositionAdjust:
        dist_to_next = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            if (get_chessboard_distance(&creatng->mappos, &navi->pos_next) > navi->distance_to_next_pos
             || creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags))
            {
                navi->navstate = NavS_WallhugInProgress;
                navi->pos_final.x.val = pos->x.val;
                navi->pos_final.y.val = pos->y.val;
                navi->pos_final.z.val = pos->z.val;
                navi->wallhug_state = 0;
                navi->wallhug_retry_counter = 0;
                navi->push_counter = 0;
            }
            navi->navstate = NavS_WallhugPositionAdjust;
            return 1;
        }
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        tmpos.x.val = subtile_coord_center(stl_x);
        tmpos.y.val = subtile_coord_center(stl_y);
        navi->angle = get_angle_xy_to(&creatng->mappos, &tmpos);
        navi->wallhug_retry_counter = 0;
        navi->wallhug_state = 0;
        navi->distance_to_next_pos = 0;
        if (get_angle_difference(creatng->move_angle_xy, navi->angle) != 0) {
            navi->navstate = NavS_WallhugPositionAdjust;
            return 1;
        }
        navi->navstate = NavS_WallhugGapDetected;
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->second_colliding_block = stl_num;
        return 2;
    case NavS_WallhugDirectionCheck:
        dist_to_next = get_chessboard_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
            navi->navstate = NavS_WallhugDirectionCheck;
            return 1;
        }
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        tmpos.x.val = subtile_coord_center(stl_x);
        tmpos.y.val = subtile_coord_center(stl_y);
        navi->angle = get_angle_xy_to(&creatng->mappos, &tmpos);
        if (get_angle_difference(creatng->move_angle_xy, navi->angle) != 0) {
            navi->navstate = NavS_WallhugDirectionCheck;
            return 1;
        }
        navi->navstate = NavS_WallhugAngleCorrection;
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->second_colliding_block = stl_num;
        return 2;
    case NavS_WallhugGapDetected:
    {
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->second_colliding_block = stl_num;
        mapblk = get_map_block_at_pos(navi->first_colliding_block);
        if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
          return 2;
        }
        nav_radius = thing_nav_sizexy(creatng) / 2;
        long i;
        if (navi->side == 1)
        {
            i = (creatng->move_angle_xy + DEGREES_45) / DEGREES_90 - 1;
        }
        else
        {
            i = (creatng->move_angle_xy + DEGREES_45) / DEGREES_90 + 1;
        }
        navi->pos_next.x.val += (384 - nav_radius) * small_around[i&3].delta_x;
        navi->pos_next.y.val += (384 - nav_radius) * small_around[i&3].delta_y;
        i = (creatng->move_angle_xy + DEGREES_45) / DEGREES_90;
        navi->pos_next.x.val += (128) * small_around[i&3].delta_x;
        i = (creatng->move_angle_xy) / DEGREES_90;
        navi->pos_next.y.val += (128) * small_around[i&3].delta_y;
        navi->navstate = NavS_WallhugRestartSetup;
        return 1;
    }
    case NavS_WallhugAngleCorrection:
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->second_colliding_block = stl_num;
        mapblk = get_map_block_at_pos(navi->first_colliding_block);
        if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            return 2;
        }
        navi->navstate = NavS_WallhugInProgress;
        return 1;
    case NavS_WallhugRestartSetup:
        if (get_chessboard_distance(&creatng->mappos, &navi->pos_next) > 16)
        {
            return 1;
        }
        if (navi->side == 1)
            angle = creatng->move_angle_xy + DEGREES_90;
        else
            angle = creatng->move_angle_xy - DEGREES_90;
        navi->angle = angle & ANGLE_MASK;
        navi->navstate = NavS_InitialWallhugSetup;
        return 1;
    default:
        break;
    }
    return 1;
}

SubtlCodedCoords dig_to_position(PlayerNumber plyr_idx, MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, SmallAroundIndex direction_around, TbBool revside)
{
    long round_change;
    SYNCDBG(14,"Starting for subtile (%d,%d)",(int)basestl_x,(int)basestl_y);
    if (revside) {
      round_change = 1;
    } else {
      round_change = 3;
    }
    SmallAroundIndex round_idx = (direction_around + SMALL_AROUND_LENGTH - round_change) % SMALL_AROUND_LENGTH;
    for (long i = 0; i < SMALL_AROUND_LENGTH; i++)
    {
        MapSubtlCoord stl_x = basestl_x + STL_PER_SLB * (int)small_around[round_idx].delta_x;
        MapSubtlCoord stl_y = basestl_y + STL_PER_SLB * (int)small_around[round_idx].delta_y;
        if (!is_valid_hug_subtile(stl_x, stl_y, plyr_idx))
        {
            SYNCDBG(7,"Subtile (%d,%d) accepted",(int)stl_x,(int)stl_y);
            SubtlCodedCoords stl_num = get_subtile_number(stl_x, stl_y);
            return stl_num;
        }
        round_idx = (round_idx + round_change) % SMALL_AROUND_LENGTH;
    }
    return -1;
}

// used only by AI (get_hug_side, tool_dig_to_pos2)
static inline void get_hug_side_next_step(MapSubtlCoord dst_stl_x, MapSubtlCoord dst_stl_y, int dirctn, PlayerNumber plyr_idx,
    char *state, MapSubtlCoord *ostl_x, MapSubtlCoord *ostl_y, SmallAroundIndex *round, int *maxdist)
{
    MapSubtlCoord curr_stl_x = *ostl_x;
    MapSubtlCoord curr_stl_y = *ostl_y;
    SmallAroundIndex round_idx = small_around_index_in_direction(curr_stl_x, curr_stl_y, dst_stl_x, dst_stl_y);
    int dist = chessboard_distance(curr_stl_x, curr_stl_y, dst_stl_x, dst_stl_y);
    int dx = small_around[round_idx].delta_x;
    int dy = small_around[round_idx].delta_y;
    // If we can follow direction straight to the target, and we will get closer to it, then do it
    if ((dist <= *maxdist) && !is_valid_hug_subtile(curr_stl_x + STL_PER_SLB*dx, curr_stl_y + STL_PER_SLB*dy, plyr_idx))
    {
        curr_stl_x += STL_PER_SLB*dx;
        curr_stl_y += STL_PER_SLB*dy;
        *state = WaHSS_HitWall;
        *maxdist = chessboard_distance(curr_stl_x, curr_stl_y, dst_stl_x, dst_stl_y);
    } else
    // If met second wall, finish
    if (*state == WaHSS_HitWall)
    {
        *state = WaHSS_Completed;
    } else
    { // Here we need to use wallhug to slide until we will be able to move towards destination again
        // Try directions starting at the one towards the wall, in case wall has ended
        round_idx = (*round + SMALL_AROUND_LENGTH + dirctn) % SMALL_AROUND_LENGTH;
        int n;
        for (n = 0; n < SMALL_AROUND_LENGTH; n++)
        {
            dx = small_around[round_idx].delta_x;
            dy = small_around[round_idx].delta_y;
            if (!is_valid_hug_subtile(curr_stl_x + STL_PER_SLB*dx, curr_stl_y + STL_PER_SLB*dy, plyr_idx))
            {
                break;
            }
            // If direction not for wallhug, try next
            round_idx = (round_idx + SMALL_AROUND_LENGTH - dirctn) % SMALL_AROUND_LENGTH;
        }
        if ((n < SMALL_AROUND_LENGTH) || (dirctn > 0)) {
            dx = small_around[round_idx].delta_x;
            dy = small_around[round_idx].delta_y;
            *round = round_idx;
            curr_stl_x += STL_PER_SLB*dx;
            curr_stl_y += STL_PER_SLB*dy;
        }
    }

    *ostl_x = curr_stl_x;
    *ostl_y = curr_stl_y;
}

// used only by AI (get_hug_side, tool_dig_to_pos2)
short get_hug_side_options(MapSubtlCoord src_stl_x, MapSubtlCoord src_stl_y, MapSubtlCoord dst_stl_x, MapSubtlCoord dst_stl_y,
    SmallAroundIndex direction, PlayerNumber plyr_idx, MapSubtlCoord *ostla_x, MapSubtlCoord *ostla_y, MapSubtlCoord *ostlb_x, MapSubtlCoord *ostlb_y)
{
    SYNCDBG(4,"Starting");

    int dist = chessboard_distance(src_stl_x, src_stl_y, dst_stl_x, dst_stl_y);

    char state_a = WaHSS_Initial;
    MapSubtlCoord stl_a_x = src_stl_x;
    MapSubtlCoord stl_a_y = src_stl_y;
    SmallAroundIndex round_a = (direction + SMALL_AROUND_LENGTH + 1) % SMALL_AROUND_LENGTH;
    int maxdist_a = dist - 1;
    char state_b = WaHSS_Initial;
    MapSubtlCoord stl_b_x = src_stl_x;
    MapSubtlCoord stl_b_y = src_stl_y;
    SmallAroundIndex round_b = (direction + SMALL_AROUND_LENGTH - 1) % SMALL_AROUND_LENGTH;
    int maxdist_b = dist - 1;

    // Try moving in both directions
    for (int i = 150; i > 0; i--)
    {
        if ((state_a == WaHSS_Completed) && (state_b == WaHSS_Completed)) {
            break;
        }
        if (state_a != WaHSS_Completed)
        {
            get_hug_side_next_step(dst_stl_x, dst_stl_y, -1, plyr_idx, &state_a, &stl_a_x, &stl_a_y, &round_a, &maxdist_a);
            if ((stl_a_x == dst_stl_x) && (stl_a_y == dst_stl_y)) {
                *ostla_x = stl_a_x;
                *ostla_y = stl_a_y;
                *ostlb_x = stl_b_x;
                *ostlb_y = stl_b_y;
                return 1;
            }
        }
        if (state_b != WaHSS_Completed)
        {
            get_hug_side_next_step(dst_stl_x, dst_stl_y, 1, plyr_idx, &state_b, &stl_b_x, &stl_b_y, &round_b, &maxdist_b);
            if ((stl_b_x == dst_stl_x) && (stl_b_y == dst_stl_y)) {
                *ostla_x = stl_a_x;
                *ostla_y = stl_a_y;
                *ostlb_x = stl_b_x;
                *ostlb_y = stl_b_y;
                return 0;
            }
        }
    }
    *ostla_x = stl_a_x;
    *ostla_y = stl_a_y;
    *ostlb_x = stl_b_x;
    *ostlb_y = stl_b_y;
    return 2;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
