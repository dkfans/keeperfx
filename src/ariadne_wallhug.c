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
static TbBool check_forward_for_prospective_hugs(struct Thing *creatng, struct Coord3d *pos_a, long angle, long side, long a3, long speed, PlayerBitFlag crt_owner_flags);
static int small_around_index_in_direction(long srcpos_x, long srcpos_y, long dstpos_x, long dstpos_y);
static long get_angle_of_wall_hug(struct Thing *creatng, long slab_flags, long a3, PlayerBitFlag crt_owner_flags);
static void set_hugging_pos_using_blocked_flags(struct Coord3d *dstpos, struct Thing *creatng, unsigned short block_flags, int nav_radius);
static TbBool navigation_push_towards_target(struct Navigation *navi, struct Thing *creatng, const struct Coord3d *pos, MoveSpeed speed, MoveSpeed nav_radius, PlayerBitFlag crt_owner_flags);
static TbBool find_approach_position_to_subtile(const struct Coord3d *srcpos, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MoveSpeed spacing, struct Coord3d *aproachpos);
static long creature_cannot_move_directly_to_with_collide(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlag crt_owner_flags);
static unsigned short get_hugging_blocked_flags(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlag crt_owner_flags);

const uint8_t byte_5111FA[] = { 1,0,4,2,0,0,2,0,4,1,0,0,0,0 };
const uint8_t byte_51120A[] = { 2,0,2,1,0,6,1,0,2,2,0,0,0,0 };
const uint8_t byte_51121A[22] = { 2,0,0,1,0,2,1,0,0,2,0,6,1,0,4,2,0,2,2,0,4,1 };

/******************************************************************************/
/**
 * Computes index in small_around[] array which contains coordinates directing towards given destination.
 * @param srcpos_x Source position X; either map coordinates or subtiles, but have to match type of other coords.
 * @param srcpos_y Source position Y; either map coordinates or subtiles, but have to match type of other coords.
 * @param dstpos_x Destination position X; either map coordinates or subtiles, but have to match type of other coords.
 * @param dstpos_y Destination position Y; either map coordinates or subtiles, but have to match type of other coords.
 * @return Index for small_around[] array.
 */
static int small_around_index_in_direction(long srcpos_x, long srcpos_y, long dstpos_x, long dstpos_y)
{
    long i = ((LbArcTanAngle(dstpos_x - srcpos_x, dstpos_y - srcpos_y) & LbFPMath_AngleMask) + LbFPMath_PI / 4);
    return (i >> 9) & 3;
}

static TbBool can_step_on_unsafe_terrain_at_position(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    // We can step on lava if it doesn't hurt us or we can fly
    if (slb->kind == SlbT_LAVA) {
        return (crstat->hurt_by_lava <= 0) || ((creatng->movement_flags & TMvF_Flying) != 0);
    }
    return false;
}

TbBool terrain_toxic_for_creature_at_position(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    // If the position is over lava, and we can't continuously fly, then it's toxic
    if ((crstat->hurt_by_lava > 0) && map_pos_is_lava(stl_x,stl_y)) {
        // Check not only if a creature is now flying, but also whether it's natural ability
        if (((creatng->movement_flags & TMvF_Flying) == 0) || (!crstat->flying))
            return true;
    }
    return false;
}

static TbBool hug_can_move_on(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb))
        return false;
    struct SlabAttr* slbattr = get_slab_attrs(slb);
    if ((slbattr->block_flags & SlbAtFlg_IsDoor) != 0)
    {
        struct Thing* doortng = get_door_for_position(stl_x, stl_y);
        if (!thing_is_invalid(doortng) && (doortng->owner == creatng->owner) && !doortng->door.is_locked)
        {
            return true;
        }
    }
    else
    {
        if (slbattr->is_safe_land || can_step_on_unsafe_terrain_at_position(creatng, stl_x, stl_y))
        {
            return true;
        }
    }
    return false;
}

static TbBool wallhug_angle_with_collide_valid(struct Thing *thing, long slab_flags, long speed, long angle, PlayerBitFlag crt_owner_flags)
{
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(speed, angle);
    pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(speed, angle);
    pos.z.val = get_thing_height_at(thing, &pos);
    return (creature_cannot_move_directly_to_with_collide(thing, &pos, slab_flags, crt_owner_flags) != 4);
}

static long get_angle_of_wall_hug(struct Thing *creatng, long slab_flags, long speed, PlayerBitFlag crt_owner_flags)
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
        whangle = (LbFPMath_PI/2) * ((quadr - 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * (quadr & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 2) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        break;
    case 2:
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * (quadr & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr - 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 2) & 3);
        if (wallhug_angle_with_collide_valid(creatng, slab_flags, speed, whangle, crt_owner_flags))
          return whangle;
        break;
    }
    return -1;
}

static int hug_round_sub(struct Thing *creatng, MapSubtlCoord *pos1_stl_x, MapSubtlCoord *pos1_stl_y,
                            const MapSubtlCoord pos2_stl_x, const MapSubtlCoord pos2_stl_y, char *v58,
                            MapSubtlDelta *delta, struct Coord3d *pos1, long *hug_val, unsigned short *i,
                            int *round_idx_plus1_1, const int arr_offset_1, const int arr_offset_2)
{
    if (*v58 == 2)
    {
        return -1;
    }

    unsigned short quadrant = (((LbArcTanAngle(pos2_stl_x - *pos1_stl_x, pos2_stl_y - *pos1_stl_y) & LbFPMath_AngleMask) + 256) >> 9) & 3;

    int v20 = max(abs(*pos1_stl_x - pos2_stl_x), abs(*pos1_stl_y - pos2_stl_y));
    if ((int)abs(v20) <= *delta && hug_can_move_on(
                                       creatng,
                                       3 * small_around[quadrant].delta_x + *pos1_stl_x,
                                       3 * small_around[quadrant].delta_y + *pos1_stl_y))
    {
        *pos1_stl_x += 3 * small_around[quadrant].delta_x;
        *pos1_stl_y += 3 * small_around[quadrant].delta_y;

        *delta = max(abs(*pos1_stl_x - pos2_stl_x), abs(*pos1_stl_y - pos2_stl_y));

        *v58 = 1;
    }
    else
    {
        if (*v58 == 1)
        {
            pos1->x.stl.num = *pos1_stl_x;
            pos1->y.stl.num = *pos1_stl_y;
            *hug_val -= *i;
            return 0;
        }
        int v24 = (*round_idx_plus1_1 + arr_offset_1) & 3;
        for (unsigned short j = 0;; ++j)
        {
            int v41 = v24;
            if (j >= 4u)
              break;
            int v25 = v24;
            if (hug_can_move_on(
                    creatng,
                    3 * small_around[v25].delta_x + *pos1_stl_x,
                    *pos1_stl_y + 3 * small_around[v25].delta_y))
            {
              *round_idx_plus1_1 = v41;
              *pos1_stl_x += 3 * small_around[v25].delta_x;
              *pos1_stl_y += 3 * small_around[v25].delta_y;
              break;
            }
            v24 = v41 + arr_offset_2;
            v24 = (v41 + arr_offset_2) & 3;
        }
    }
    if (*pos1_stl_x == pos2_stl_x && *pos1_stl_y == pos2_stl_y)
    {
        *hug_val -= *i;
        return 1;
    }
    return -1;
}

static int hug_round(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, unsigned short round_idx, long *hug_val)
{
    MapSubtlCoord pos2_stl_x = pos2->x.stl.num;
    MapSubtlCoord pos2_stl_y = pos2->y.stl.num;
    MapSubtlCoord pos1_stl_x = pos1->x.stl.num;
    MapSubtlCoord pos1_stl_y = pos1->y.stl.num;

    MapSubtlCoord pos1_stl_x_2 = pos1->x.stl.num;
    MapSubtlCoord pos1_stl_y_2 = pos1->y.stl.num;

    int round_idx_plus1 = (round_idx + 1) & 3;
    int round_idx_minus1 = (round_idx - 1) & 3;

    MapSubtlDelta max_delta_1 = max(abs(pos1_stl_x - pos2_stl_x), abs(pos1_stl_y - pos2_stl_y)) - 1;
    MapSubtlDelta max_delta_2 = max(abs(pos1_stl_x_2 - pos2_stl_x), abs(pos1_stl_y_2 - pos2_stl_y)) - 1;

    char v58 = 0;
    char v57 = 0;
    unsigned short i;
    for (i = *hug_val; i && (v58 != 2 || v57 != 2); --i)
    {
      char return_val;
      return_val = hug_round_sub(creatng, &pos1_stl_x, &pos1_stl_y, pos2_stl_x, pos2_stl_y, &v58, &max_delta_1, pos1, hug_val, &i, &round_idx_plus1, 3, 1);
      if (return_val != -1)
        return return_val;
      return_val = hug_round_sub(creatng, &pos1_stl_x_2, &pos1_stl_y_2, pos2_stl_x, pos2_stl_y, &v57, &max_delta_2, pos1, hug_val, &i, &round_idx_minus1, 1, 3);
      if (return_val != -1)
        return return_val;
    }
    if (!i)
      return -1;

    MapSubtlDelta dist_1 = abs(pos1_stl_y - pos2_stl_y) + abs(pos1_stl_x - pos2_stl_x);
    MapSubtlDelta dist_2 = abs(pos1_stl_y_2 - pos2_stl_y) + abs(pos1_stl_x_2 - pos2_stl_x);
    if (dist_2 >= dist_1)
    {
      pos1->x.stl.num = pos1_stl_x;
      pos1->y.stl.num = pos1_stl_y;
    }
    else
    {
      pos1->x.stl.num = pos1_stl_x_2;
      pos1->y.stl.num = pos1_stl_y_2;
    }
    *hug_val -= i;
    return 0;
}

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
    struct Coord3d pos3;
    pos3.x.val = pos->x.val;
    pos3.y.val = pos->y.val;
    pos3.z.val = pos->z.val;
    pos3.x.stl.num = stl_x;
    pos3.y.stl.num = stl_y;
    struct Coord3d next_pos;
    next_pos.x.val = curr_pos.x.val;
    next_pos.y.val = curr_pos.y.val;
    next_pos.z.val = curr_pos.z.val;
    for (int i = 0; i < max_val; i++)
    {
        if ((curr_pos.x.stl.num == stl_x) && (curr_pos.y.stl.num == stl_y)) {
            return i + 1;
        }
        int round_idx = small_around_index_in_direction(curr_pos.x.stl.num, curr_pos.y.stl.num, stl_x, stl_y);
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
            int hug_ret = hug_round(thing, &next_pos, &pos3, round_idx, &hug_val);
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

unsigned short get_hugging_blocked_flags(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlag crt_owner_flags)
{
    struct Coord3d tmpos;
    unsigned short blkflags = 0;
    {
        tmpos.x.val = pos->x.val;
        tmpos.y.val = creatng->mappos.y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, slab_flags, crt_owner_flags) == 4) {
            blkflags |= 0x01;
        }
    }
    {
        tmpos.x.val = creatng->mappos.x.val;
        tmpos.y.val = pos->y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, slab_flags, crt_owner_flags) == 4) {
            blkflags |= 0x02;
        }
    }
    if (blkflags == 0)
    {
        tmpos.x.val = pos->x.val;
        tmpos.y.val = pos->y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, slab_flags, crt_owner_flags) == 4) {
            blkflags |= 0x04;
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
            tmpos.x.stl.pos = 255;
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
            tmpos.y.stl.pos = 255;
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
            tmpos.x.stl.pos = 255;
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
            tmpos.y.stl.pos = 255;
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
 * @param slab_flags Contains slab attribute flags (SlabAttrFlags) passed to this function; this variable is used to decide if the current slab should be checked for collision, or not.
 * @param crt_owner_flags Contains player bitflags (type PlayerBitFlag) passed to this function; this variable is used to optionally check the slab's owner (if crt_owner_flags is 0, the ownership check is nullified).
*/
static long get_map_index_of_first_block_thing_colliding_with_at(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlag crt_owner_flags)
{

    MapCoordDelta nav_radius = thing_nav_sizexy(creatng) / 2;

    MapSubtlCoord start_stl_x = (pos->x.val - nav_radius) / COORD_PER_STL;
    if (start_stl_x <= 0)
        start_stl_x = 0;
    MapSubtlCoord end_stl_x = (pos->x.val + nav_radius) / COORD_PER_STL + 1;
    if (end_stl_x >= gameadd.map_subtiles_x)
        end_stl_x = gameadd.map_subtiles_x;
        

    MapSubtlCoord start_stl_y = (pos->y.val - nav_radius) / COORD_PER_STL;
    if (start_stl_y <= 0)
        start_stl_y = 0;
    MapSubtlCoord end_stl_y = (pos->y.val + nav_radius) / COORD_PER_STL + 1;
    if (end_stl_y >= gameadd.map_subtiles_y)
        end_stl_y = gameadd.map_subtiles_y;

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

            if (((mapblk->flags & slab_flags) == 0 && slb->kind != SlbT_ROCK)
             || ((slab_flags & mapblk->flags & SlbAtFlg_Filled) != 0 && CHECK_SLAB_OWNER))
            {
                continue; // Don't check the current slab for collision, because {assumption} it is a low slab (which the current creature is allowed to walk across).
            }
            if ((mapblk->flags & SlbAtFlg_IsDoor) == 0)
            {
                return get_subtile_number(current_stl_x,current_stl_y);
            }
            struct Thing *doortng = get_door_for_position(current_stl_x, current_stl_y);
            if (thing_is_invalid(doortng) || !door_will_open_for_thing(doortng, creatng))
            {
                return get_subtile_number(current_stl_x,current_stl_y);
            }

        }
    }
    return -1;        
}

static long creature_cannot_move_directly_to_with_collide_sub(struct Thing *creatng, struct Coord3d pos, long slab_flags, PlayerBitFlag crt_owner_flags)
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

long creature_cannot_move_directly_to_with_collide(struct Thing *creatng, struct Coord3d *pos, long slab_flags, PlayerBitFlag crt_owner_flags)
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

static TbBool thing_can_continue_direct_line_to(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, long a4, long a5, PlayerBitFlag crt_owner_flags)
{
    long angle = get_angle_xy_to(pos1, pos2);
    struct Coord3d posa;
    posa.x.val = pos1->x.val;
    posa.y.val = pos1->y.val;
    posa.z.val = pos1->z.val;
    posa.x.val += distance_with_angle_to_coord_x(a5, angle);
    posa.y.val += distance_with_angle_to_coord_y(a5, angle);
    posa.z.val = get_thing_height_at(creatng, &posa);
    int coord = pos1->x.val;
    if (coord < posa.x.val) {
        coord += a5;
    } else
    if (coord > posa.x.val) {
        coord -= a5;
    }
    struct Coord3d posb;
    posb.x.val = coord;
    posb.y.val = pos1->y.val;
    posb.z.val = get_thing_height_at(creatng, &posb);
    coord = pos1->y.val;
    if (coord < posa.y.val) {
        coord += a5;
    } else
    if (coord > posa.y.val) {
        coord -= a5;
    }
    struct Coord3d posc;
    posc.y.val = coord;
    posc.x.val = pos1->x.val;
    posc.z.val = get_thing_height_at(creatng, &posc);
    return creature_cannot_move_directly_to_with_collide(creatng, &posb, a4, crt_owner_flags) != 4
        && creature_cannot_move_directly_to_with_collide(creatng, &posc, a4, crt_owner_flags) != 4
        && creature_cannot_move_directly_to_with_collide(creatng, &posa, a4, crt_owner_flags) != 4;
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
    PlayerBitFlag crt_owner_flags)
{

    struct Coord3d pos;
    struct Coord3d v47;
    struct Coord3d pos_52;
    struct Navigation temp_navi;

    int v46 = INT_MAX;

    pos_52 = creatng->mappos;

    short move_angle_xy = creatng->move_angle_xy;
    memcpy(&temp_navi, navi, sizeof(struct Navigation));
    creatng->move_angle_xy = arg_move_angle_xy;
    navi->side = side;
    navi->dist_to_final_pos = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
    int v49 = 0;
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
    int v16 = hugging_blocked_flags;
    *arg_pos = pos;
    if (v16 == 4)
    {
        if (!arg_move_angle_xy || arg_move_angle_xy == 1024)
        {
            creatng->mappos.x.val = arg_pos->x.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
        else if (arg_move_angle_xy == 512 || arg_move_angle_xy == 1536)
        {
            creatng->mappos.y.val = arg_pos->y.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
    }
    else
    {
        creatng->mappos = pos;
    }
    int v51 = 0;

    short v17;
    short v18;
    short v19;
    short v20;
    long v21;
    long v25;
    char v27;
    short v33;
    long v38;
    long _2d_distance_squared;
    int v40;
    char v43;
    int angle_of_wall_hug;

    navi->angle = arg_move_angle_xy;
    while (1)
    {
        v43 = 0;
        if (get_2d_distance_squared(&creatng->mappos, &navi->pos_final) < navi->dist_to_final_pos && thing_can_continue_direct_line_to(creatng, &creatng->mappos, &navi->pos_final, slab_flags, max_speed, crt_owner_flags))
        {
            _2d_distance_squared = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
            goto LABEL_69;
        }
        if (v51)
        {
            angle_of_wall_hug = get_angle_of_wall_hug(creatng, slab_flags, speed, crt_owner_flags);
            goto LABEL_38;
        }
        v17 = creatng->move_angle_xy;
        v18 = v17;
        if (navi->side != 1)
        {
            v20 = v17 - 512;
            goto LABEL_36;
        }
        v19 = v17 + 512;
        creatng->move_angle_xy = v19;
        if ((unsigned short)v19 >= 0x800u)
        {
            v20 = v19 - 2048;
        LABEL_36:
            creatng->move_angle_xy = v20;
        }
        v21 = get_angle_of_wall_hug(creatng, slab_flags, speed, crt_owner_flags);
        creatng->move_angle_xy = v18;
        angle_of_wall_hug = v21;
    LABEL_38:
        if (!v51 || navi->angle != angle_of_wall_hug)
        {
            v47.x.val = move_coord_with_angle_x(creatng->mappos.x.val, speed, navi->angle);
            v47.y.val = move_coord_with_angle_y(creatng->mappos.x.val, speed, navi->angle);
            v47.z.val = get_thing_height_at(creatng, &v47);
            if (creature_cannot_move_directly_to_with_collide(creatng, &v47, slab_flags, crt_owner_flags) == 4)
            {
                v25 = get_hugging_blocked_flags(creatng, &v47, slab_flags, IGNORE_SLAB_OWNER_CHECK);
                hugging_blocked_flags = v25;
                v27 = v25;
                pos.x.val = creatng->mappos.x.val;
                pos.y.val = creatng->mappos.y.val;
                if ((v27 & 1) != 0)
                {
                    if (v47.x.val >= creatng->mappos.x.val)
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
                    if (v47.y.val >= creatng->mappos.y.val)
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
                    if (v47.x.val >= creatng->mappos.x.val)
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
                    if (v47.y.val >= (unsigned int)creatng->mappos.y.val)
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
                v47 = pos;
                if (creatng->mappos.x.val != pos.x.val && creatng->mappos.y.val != pos.y.val)
                {
                    creatng->mappos = pos;
                    v43 = 1;
                    navi->distance_to_next_pos = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
                }
            }
        }
        if (!v43)
        {
            v33 = angle_of_wall_hug;
            navi->angle = angle_of_wall_hug;
            creatng->move_angle_xy = v33;

            v47.x.val = move_coord_with_angle_x(creatng->mappos.x.val, speed, navi->angle);
            v47.y.val = move_coord_with_angle_y(creatng->mappos.y.val, speed, navi->angle);
            v47.z.val = get_thing_height_at(creatng, &v47);
            check_forward_for_prospective_hugs(
                creatng,
                &v47,
                (unsigned short)creatng->move_angle_xy,
                navi->side,
                slab_flags,
                speed,
                crt_owner_flags);
            creatng->mappos = v47;
        }
        v49 += speed;
        v38 = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
        if (v38 < v46)
        {
            v46 = v38;
            if (v38 < 0x10000)
                break;
        }
        if (++v51 >= 100)
            goto LABEL_70;
    }
    _2d_distance_squared = v46;
LABEL_69:
    v46 = -_2d_distance_squared;
LABEL_70:
    if (v46 >= 0)
        v40 = v49 * v49 + v46;
    else
        v40 = v46 - v49 * v49;
    v46 = v40;
    creatng->mappos = pos_52;
    creatng->move_angle_xy = move_angle_xy;

    memcpy(navi, &temp_navi, sizeof(struct Navigation));
    return v46;
}

static int get_starting_angle_and_side_of_hug_sub1(
    struct Thing *creatng,
    struct Coord3d *pos,
    long slab_flags,
    PlayerBitFlag crt_owner_flags)
{
    
    struct Coord3d pos_2;

    int hugging_blocked_flags = get_hugging_blocked_flags(creatng, pos, slab_flags, crt_owner_flags);
    MapCoordDelta nav_radius = thing_nav_sizexy(creatng) / 2;
    pos_2.x.val = creatng->mappos.x.val;
    pos_2.y.val = creatng->mappos.y.val;
    if ((hugging_blocked_flags & 1) != 0)
    {
        if (pos->x.val >= creatng->mappos.x.val)
        {
            pos_2.x.stl.num = (nav_radius + creatng->mappos.x.val) >> 8;
            pos_2.x.stl.pos = -1;
            pos_2.x.val -= nav_radius;
        }
        else
        {
            pos_2.x.stl.num = (creatng->mappos.x.val - nav_radius) >> 8;
            pos_2.x.stl.pos = 1;
            pos_2.x.val += nav_radius;
        }
        pos_2.z.val = get_thing_height_at(creatng, &pos_2);
    }
    if ((hugging_blocked_flags & 2) != 0)
    {
        if (pos->y.val >= creatng->mappos.y.val)
        {
            pos_2.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
            pos_2.y.stl.pos = -1;
            pos_2.y.val -= nav_radius;
        }
        else
        {
            pos_2.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
            pos_2.y.stl.pos = 1;
            pos_2.y.val += nav_radius;
        }
        pos_2.z.val = get_thing_height_at(creatng, &pos_2);
    }
    if ((hugging_blocked_flags & 4) != 0)
    {
        if (pos->x.val >= creatng->mappos.x.val)
        {
            pos_2.x.stl.num = (nav_radius + creatng->mappos.x.val) >> 8;
            pos_2.x.stl.pos = -1;
            pos_2.x.val -= nav_radius;
        }
        else
        {
            pos_2.x.stl.num = (creatng->mappos.x.val - nav_radius) >> 8;
            pos_2.x.stl.pos = 1;
            pos_2.x.val += nav_radius;
        }
        if (pos->y.val >= creatng->mappos.y.val)
        {
            pos_2.y.stl.num = (nav_radius + creatng->mappos.y.val) >> 8;
            pos_2.y.stl.pos = -1;
            pos_2.y.val -= nav_radius;
        }
        else
        {
            pos_2.y.stl.num = (creatng->mappos.y.val - nav_radius) >> 8;
            pos_2.y.stl.pos = 1;
            pos_2.y.val += nav_radius;
        }
        pos_2.z.val = get_thing_height_at(creatng, &pos_2);
    }
    *pos = pos_2;
    return hugging_blocked_flags;
}

static signed char get_starting_angle_and_side_of_hug(
    struct Thing *creatng,
    struct Coord3d *pos,
    long *angle,
    unsigned char *side,
    long slab_flags,
    PlayerBitFlag crt_owner_flags)
{
    int v9;
    int v10;
    char hugging_blocked_flags;
    int v12;
    int v13;
    int v14;
    char v15;
    int v16;
    int v17;
    int v18;
    int v19;
    int v20;
    int v21;
    int v23;
    int32_t angle_of_wall_hug;
    int16_t v25;
    int16_t v26;
    int16_t v27;
    int32_t _2d_distance_squared;
    int v29;
    int v31;
    int8_t result;
    char v33;
    uint8_t v34;
    char v35;
    int16_t move_angle_xy;
    uint16_t angle_37;
    int v38;
    uint16_t angle_39;
    int16_t v40;
    int v41;
    int v42;
    struct Coord3d pos_43;
    int angle_44;
    char v44_2;
    int v45;
    struct Coord3d pos_46;
    struct Navigation v49;


    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    struct Navigation *navi = &cctrl->navi;
    const short max_speed = cctrl->max_speed;

    pos_43.x.stl.pos = creatng->mappos.y.val - (uint16_t)pos->y.val <= 0;
    v38 = (uint16_t)creatng->mappos.x.val - (uint16_t)pos->x.val <= 0;
    v9 = creatng->mappos.y.val - navi->pos_final.y.val;
    v49.navstate = v9 <= 0;
    v10 = (uint16_t)creatng->mappos.x.val - navi->pos_final.x.val;
    pos_46.x.stl.pos = v10 <= 0;
    v44_2 = (int)abs(v10) < (int)abs(v9);
    hugging_blocked_flags = get_hugging_blocked_flags(creatng, pos, slab_flags, crt_owner_flags);
    if ((hugging_blocked_flags & 1) != 0)
    {
        v12 = 2 * v38;
        v13 = v12 + (uint8_t)v49.navstate;
        angle_39 = blocked_x_hug_start[0][v13].angle;
        v34 = byte_5111FA[3 * v13];
        v14 = v12 + (v49.navstate == 0);
        angle_37 = blocked_x_hug_start[0][v14].angle;
        v15 = byte_5111FA[3 * v14];
    }
    else if ((hugging_blocked_flags & 2) != 0)
    {
        v16 = 2 * (uint8_t)pos_43.x.stl.pos;
        v17 = v16 + (unsigned char)pos_46.x.stl.pos;
        angle_39 = blocked_y_hug_start[0][v17].angle;
        v34 = byte_51120A[3 * v17];
        v18 = v16 + (pos_46.x.stl.pos == 0);
        angle_37 = blocked_y_hug_start[0][v18].angle;
        v15 = byte_51120A[3 * v18];
    }
    else
    {
        if ((hugging_blocked_flags & 4) == 0)
        {
            ERRORLOG("Illegal block direction for lookahead");
            return 0;
        }
        v19 = 2 * (v38 + 2 * (uint8_t)pos_43.x.stl.pos);
        v20 = v19 + v44_2;
        angle_39 = blocked_xy_hug_start[0][0][v20].angle;
        v34 = byte_51121A[3 * v20];
        v21 = v19 + (v44_2 == 0);
        angle_37 = blocked_xy_hug_start[0][0][v21].angle;
        v15 = byte_51121A[3 * v21];
    }
    v41 = 0x7FFFFFFF;
    v35 = v15;
    pos_46.x.val = creatng->mappos.x.val;
    pos_46.y.val = creatng->mappos.y.val;
    pos_46.z.val = creatng->mappos.z.val;
    move_angle_xy = creatng->move_angle_xy;
    memcpy(&v49, navi, sizeof(struct Navigation));
    creatng->move_angle_xy = angle_39;
    navi->side = v34;
    navi->dist_to_final_pos = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
    v45 = 0;
    if (get_starting_angle_and_side_of_hug_sub1(creatng, pos, slab_flags, crt_owner_flags) == 4)
    {
        if (angle_39 == ANGLE_NORTH || angle_39 == ANGLE_SOUTH)
        {
            creatng->mappos.x.val = pos->x.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
        else if (angle_39 == ANGLE_WEST || angle_39 == ANGLE_EAST)
        {
            creatng->mappos.y.val = pos->y.val;
            creatng->mappos.z.val = get_thing_height_at(creatng, &creatng->mappos);
        }
    }
    else
    {
        creatng->mappos = *pos;
    }
    v23 = 0;
    angle_44 = angle_39;
    navi->angle = angle_39;
    while (1)
    {
        v33 = 0;
        if (get_2d_distance_squared(&creatng->mappos, &navi->pos_final) < navi->dist_to_final_pos)
        {
            if (thing_can_continue_direct_line_to(creatng, &creatng->mappos, &navi->pos_final, slab_flags, max_speed, crt_owner_flags))
                break;
        }
        if (v23)
        {
            angle_of_wall_hug = get_angle_of_wall_hug(creatng, slab_flags, 255, crt_owner_flags);
            goto LABEL_26;
        }
        v25 = creatng->move_angle_xy;
        v40 = v25;
        if (navi->side != 1)
        {
            v27 = v25 - 512;
            goto LABEL_24;
        }
        v26 = v25 + 512;
        creatng->move_angle_xy = v26;
        if ((unsigned short)v26 >= 0x800u)
        {
            v27 = v26 - 2048;
        LABEL_24:
            creatng->move_angle_xy = v27;
        }
        angle_of_wall_hug = get_angle_of_wall_hug(creatng, slab_flags, 255, crt_owner_flags);
        creatng->move_angle_xy = v40;
    LABEL_26:
        if (!v23 || navi->angle != angle_of_wall_hug)
        {
            pos_43.x.val = move_coord_with_angle_x(creatng->mappos.x.val, COORD_PER_STL, navi->angle);
            pos_43.y.val = move_coord_with_angle_y(creatng->mappos.y.val, COORD_PER_STL, navi->angle);
            pos_43.z.val = get_thing_height_at(creatng, &pos_43);
            if (creature_cannot_move_directly_to_with_collide(creatng, &pos_43, slab_flags, crt_owner_flags) == 4)
            {
                get_starting_angle_and_side_of_hug_sub1(creatng, &pos_43, slab_flags, IGNORE_SLAB_OWNER_CHECK);
                if (creatng->mappos.x.val != pos_43.x.val || creatng->mappos.y.val != pos_43.y.val)
                {
                    creatng->mappos = pos_43;
                    v33 = 1;
                    navi->distance_to_next_pos = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
                }
            }
        }
        if (!v33)
        {
            navi->angle = angle_of_wall_hug;
            creatng->move_angle_xy = angle_of_wall_hug;
            pos_43.x.val = move_coord_with_angle_x(creatng->mappos.x.val, COORD_PER_STL, navi->angle);
            pos_43.y.val = move_coord_with_angle_y(creatng->mappos.y.val, COORD_PER_STL, navi->angle);
            pos_43.z.val = get_thing_height_at(creatng, &pos_43);
            check_forward_for_prospective_hugs(
                creatng,
                &pos_43,
                (unsigned short)creatng->move_angle_xy,
                navi->side,
                slab_flags,
                255,
                crt_owner_flags);
            creatng->mappos = pos_43;
        }
        v45 += 255;
        _2d_distance_squared = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
        if (_2d_distance_squared < v41)
        {
            v41 = _2d_distance_squared;
            if (_2d_distance_squared < 0x10000)
                goto LABEL_39;
        }
        if (++v23 >= 100)
            goto LABEL_40;
    }
    _2d_distance_squared = get_2d_distance_squared(&creatng->mappos, &navi->pos_final);
LABEL_39:
    v41 = -_2d_distance_squared;
LABEL_40:
    if (v41 >= 0)
        v29 = v45 * v45 + v41;
    else
        v29 = v41 - v45 * v45;
    v42 = v29;
    creatng->mappos.x.val = pos_46.x.val;
    creatng->mappos.y.val = pos_46.y.val;
    creatng->mappos.z.val = pos_46.z.val;
    creatng->move_angle_xy = move_angle_xy;
    memcpy(navi, &v49, sizeof(struct Navigation));
    v31 = get_starting_angle_and_side_of_hug_sub2(creatng, navi, pos, slab_flags, angle_37, v35, max_speed, 255, crt_owner_flags);
    if (v42 >= 0)
    {
        if (v31 >= 0)
        {
            if (v31 <= v42)
            {
                *angle = angle_37;
                result = 1;
                *side = v35;
            }
            else
            {
                *angle = angle_44;
                *side = v34;
                return 1;
            }
        }
        else
        {
            *angle = angle_37;
            result = 1;
            *side = v35;
        }
    }
    else if (v31 >= 0)
    {
        *angle = angle_44;
        *side = v34;
        return 1;
    }
    else if (v31 <= v42)
    {
        *angle = angle_44;
        *side = v34;
        return 1;
    }
    else
    {
        *angle = angle_37;
        result = 1;
        *side = v35;
    }
    return result;
}

static TbBool check_forward_for_prospective_hugs(struct Thing *creatng, struct Coord3d *pos_a, long angle, long side, long slab_flags, long speed, PlayerBitFlag crt_owner_flags)
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
        quadrant_angle = (((unsigned char)angle_to_quadrant(angle) - 1) & 3) << 9;

        next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
        next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
        next_pos.z.val = get_thing_height_at(creatng, &next_pos);
        if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, slab_flags, crt_owner_flags) == 4)
        {
            stored_creature_pos = creatng->mappos;
            creatng->mappos.x.val = pos.x.val;
            creatng->mappos.y.val = pos.y.val;
            creatng->mappos.z.val = pos.z.val;
            quadrant_angle = (((unsigned char)angle_to_quadrant(angle) - 1) & 3) << 9;
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
    quadrant_angle = (((unsigned char)angle_to_quadrant(angle) + 1) & 3) << 9;
    next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
    next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
    next_pos.z.val = get_thing_height_at(creatng, &next_pos);
    if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, slab_flags, crt_owner_flags) != 4)
        return false;
    stored_creature_pos = creatng->mappos;
    creatng->mappos = pos;
    quadrant_angle = (((unsigned char)angle_to_quadrant(angle) + 1) & 3) << 9;
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
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
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
            MapCoordDelta dist = get_2d_box_distance(srcpos, &tmpos);
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

static SubtlCodedCoords get_map_index_of_first_block_thing_colliding_with_travelling_to(struct Thing *creatng, struct Coord3d *startpos, struct Coord3d *endpos, long slab_flags, PlayerBitFlag crt_owner_flags)
{
    SubtlCodedCoords stl_num;
    struct Coord3d pos;
    SubtlCodedCoords return_stl_num = 0;

    struct Coord3d creature_pos = *startpos;

    MapCoordDelta delta_x = creature_pos.x.val - endpos->x.val;
    MapCoordDelta delta_y = creature_pos.y.val - endpos->y.val;

    MapCoord v27_x = creatng->mappos.x.val;
    MapCoord v27_y = creatng->mappos.y.val;
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
            pos.x.val = (int)(delta_x * abs(pos.y.val - v27_x)) / delta_y + v27_y;

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
            
            pos.y.val = (int)(delta_y * abs(pos.x.val - v27_x) / delta_x + v27_y);
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
    pos.y.val = (int)(delta_y * abs(pos.x.val - v27_y)) / delta_x + v27_x;
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
    pos.x.val = (int)(delta_x * abs(pos.y.val - v27_x) / delta_y + v27_y);
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

static TbBool navigation_push_towards_target(struct Navigation *navi, struct Thing *creatng, const struct Coord3d *pos, MoveSpeed speed, MoveSpeed nav_radius, PlayerBitFlag crt_owner_flags)
{
    navi->navstate = 2;
    navi->pos_next.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
    navi->pos_next.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
    navi->pos_next.z.val = get_thing_height_at(creatng, &navi->pos_next);
    struct Coord3d pos1;
    pos1.x.val = navi->pos_next.x.val;
    pos1.y.val = navi->pos_next.y.val;
    pos1.z.val = navi->pos_next.z.val;
    check_forward_for_prospective_hugs(creatng, &pos1, navi->angle, navi->side, SlbAtFlg_Filled|SlbAtFlg_Valuable, speed, crt_owner_flags);
    if (get_2d_box_distance(&pos1, &creatng->mappos) > 16)
    {
        navi->pos_next.x.val = pos1.x.val;
        navi->pos_next.y.val = pos1.y.val;
        navi->pos_next.z.val = pos1.z.val;
    }
    navi->distance_to_next_pos = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
    int cannot_move = creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags);

    if (cannot_move == 4)
    {
        navi->pos_next.x.val = creatng->mappos.x.val;
        navi->pos_next.y.val = creatng->mappos.y.val;
        navi->pos_next.z.val = creatng->mappos.z.val;
        navi->distance_to_next_pos = 0;
    }
    navi->dist_to_final_pos = get_2d_box_distance(&creatng->mappos, pos);
    if (cannot_move == 1)
    {
        SubtlCodedCoords stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Digable, IGNORE_SLAB_OWNER_CHECK);
        navi->first_colliding_block = stl_num;
        MapSubtlCoord stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(stl_num)));
        MapSubtlCoord stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(stl_num)));
        find_approach_position_to_subtile(&creatng->mappos, stl_x, stl_y, nav_radius + 385, &navi->pos_next);
        navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
        navi->navstate = 3;
    }
    return true;
}

long get_next_position_and_angle_required_to_tunnel_creature_to(struct Thing *creatng, struct Coord3d *pos, PlayerBitFlag crt_owner_flags)
{
    struct Navigation *navi;
    int speed;
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        navi = &cctrl->navi;
        speed = cctrl->max_speed;
        cctrl->flgfield_2 = 0;
        cctrl->combat_flags = 0;
    }
    crt_owner_flags |= CREATURE_OWNER_FLAG;
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
    case 1:
        dist_to_next = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next >= navi->distance_to_next_pos) {
            navi->field_4 = 0;
        }
        if (navi->field_4 == 0)
        {
            navi->angle = get_angle_xy_to(&creatng->mappos, pos);
            navi->pos_next.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
            navi->pos_next.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
            navi->pos_next.z.val = get_thing_height_at(creatng, &navi->pos_next);
            if (get_2d_box_distance(&creatng->mappos, pos) < get_2d_box_distance(&creatng->mappos, &navi->pos_next))
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
                unsigned short ownflag;
                ownflag = 0;
                if (!slabmap_block_invalid(slb)) {
                    ownflag = 1 << slabmap_owner(slb);
                }
                navi->field_19[0] = ownflag;

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
                    navi->field_4 = 1;
                } else
                {
                    navi->navstate = 1;
                    navi->pos_final.x.val = pos->x.val;
                    navi->pos_final.y.val = pos->y.val;
                    navi->pos_final.z.val = pos->z.val;
                    navi->field_3 = 0;
                    navi->field_2 = 0;
                    navi->field_4 = 0;
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
                navi->navstate = 3;
                return 1;
            }
        }
        if (navi->field_4 > 0)
        {
            navi->field_4++;
            if (navi->field_4 > 32) {
                ERRORLOG("I've been pushing for a very long time now...");
            }
            if (get_2d_box_distance(&creatng->mappos, &navi->pos_next) <= 16)
            {
                navi->field_4 = 0;
                navigation_push_towards_target(navi, creatng, pos, speed, thing_nav_sizexy(creatng)/2, crt_owner_flags);
            }
        }
        return 1;
    case 2:
        dist_to_next = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            if ((dist_to_next > navi->distance_to_next_pos) || creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags))
            {
                navi->navstate = 1;
                navi->pos_final.x.val = pos->x.val;
                navi->pos_final.y.val = pos->y.val;
                navi->pos_final.z.val = pos->z.val;
                navi->field_3 = 0;
                navi->field_2 = 0;
                navi->field_4 = 0;
                return 1;
            }
            return 1;
        }
        if ((get_2d_box_distance(&creatng->mappos, pos) < navi->dist_to_final_pos)
          && thing_can_continue_direct_line_to(creatng, &creatng->mappos, pos, SlbAtFlg_Filled|SlbAtFlg_Valuable, 1, crt_owner_flags))

        {
            navi->navstate = 1;
            navi->pos_final.x.val = pos->x.val;
            navi->pos_final.y.val = pos->y.val;
            navi->pos_final.z.val = pos->z.val;
            navi->field_3 = 0;
            navi->field_2 = 0;
            navi->field_4 = 0;
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
              if (get_2d_box_distance(&tmpos, &creatng->mappos) > 16)
              {
                  navi->pos_next.x.val = tmpos.x.val;
                  navi->pos_next.y.val = tmpos.y.val;
                  navi->pos_next.z.val = tmpos.z.val;
                  navi->distance_to_next_pos = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
                  return 1;
              }
          }
        }
        if (((angle + LbFPMath_PI/2) & LbFPMath_AngleMask) == navi->angle)
        {
            if (navi->field_3 == 1)
            {
                navi->field_2++;
            } else
            {
                navi->field_3 = 1;
                navi->field_2 = 1;
            }
        } else
        if (((angle - LbFPMath_PI/2) & LbFPMath_AngleMask) == navi->angle)
        {
          if (navi->field_3 == 2)
          {
              navi->field_2++;
          } else
          {
              navi->field_3 = 2;
              navi->field_2 = 1;
          }
        } else
        {
          navi->field_2 = 0;
          navi->field_3 = 0;
        }
        if (navi->field_2 >= 4)
        {
            navi->navstate = 1;
            navi->pos_final.x.val = pos->x.val;
            navi->pos_final.y.val = pos->y.val;
            navi->pos_final.z.val = pos->z.val;
            navi->field_3 = 0;
            navi->field_2 = 0;
            navi->field_4 = 0;
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

        if (get_2d_box_distance(&tmpos, &creatng->mappos) > 16)
        {
            navi->pos_next.x.val = tmpos.x.val;
            navi->pos_next.y.val = tmpos.y.val;
            navi->pos_next.z.val = tmpos.z.val;
        }
        navi->distance_to_next_pos = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
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
          navi->navstate = 1;
          navi->pos_final.x.val = pos->x.val;
          navi->pos_final.y.val = pos->y.val;
          navi->pos_final.z.val = pos->z.val;
          navi->field_3 = 0;
          navi->field_2 = 0;
          navi->field_4 = 0;
          navi->pos_next.x.val = creatng->mappos.x.val;
          navi->pos_next.y.val = creatng->mappos.y.val;
          navi->pos_next.z.val = creatng->mappos.z.val;
          return 1;
        }
        if (cannot_move != 1)
        {
            navi->distance_to_next_pos = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
            return 1;
        }
        stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Digable, IGNORE_SLAB_OWNER_CHECK);
        navi->first_colliding_block = stl_num;
        nav_radius = thing_nav_sizexy(creatng) / 2;
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(stl_num)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(stl_num)));
        find_approach_position_to_subtile(&creatng->mappos, stl_x, stl_y, nav_radius + 385, &navi->pos_next);
        navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
        navi->field_2 = 0;
        navi->field_3 = 0;
        navi->distance_to_next_pos = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        navi->navstate = 4;
        return 1;
    case 4:
        dist_to_next = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            if (get_2d_box_distance(&creatng->mappos, &navi->pos_next) > navi->distance_to_next_pos
             || creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, SlbAtFlg_Filled|SlbAtFlg_Valuable, crt_owner_flags))
            {
                navi->navstate = 1;
                navi->pos_final.x.val = pos->x.val;
                navi->pos_final.y.val = pos->y.val;
                navi->pos_final.z.val = pos->z.val;
                navi->field_3 = 0;
                navi->field_2 = 0;
                navi->field_4 = 0;
            }
            navi->navstate = 4;
            return 1;
        }
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        tmpos.x.val = subtile_coord_center(stl_x);
        tmpos.y.val = subtile_coord_center(stl_y);
        navi->angle = get_angle_xy_to(&creatng->mappos, &tmpos);
        navi->field_2 = 0;
        navi->field_3 = 0;
        navi->distance_to_next_pos = 0;
        if (get_angle_difference(creatng->move_angle_xy, navi->angle) != 0) {
            navi->navstate = 4;
            return 1;
        }
        navi->navstate = 6;
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->field_17 = stl_num;
        return 2;
    case 3:
        dist_to_next = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
            navi->navstate = 3;
            return 1;
        }
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        tmpos.x.val = subtile_coord_center(stl_x);
        tmpos.y.val = subtile_coord_center(stl_y);
        navi->angle = get_angle_xy_to(&creatng->mappos, &tmpos);
        if (get_angle_difference(creatng->move_angle_xy, navi->angle) != 0) {
            navi->navstate = 3;
            return 1;
        }
        navi->navstate = 5;
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->field_17 = stl_num;
        return 2;
    case 6:
    {
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->field_17 = stl_num;
        mapblk = get_map_block_at_pos(navi->first_colliding_block);
        if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
          return 2;
        }
        nav_radius = thing_nav_sizexy(creatng) / 2;
        long i;
        if (navi->side == 1)
        {
            i = (creatng->move_angle_xy + LbFPMath_PI/4) / (LbFPMath_PI/2) - 1;
        }
        else
        {
            i = (creatng->move_angle_xy + LbFPMath_PI/4) / (LbFPMath_PI/2) + 1;
        }
        navi->pos_next.x.val += (384 - nav_radius) * small_around[i&3].delta_x;
        navi->pos_next.y.val += (384 - nav_radius) * small_around[i&3].delta_y;
        i = (creatng->move_angle_xy + LbFPMath_PI/4) / (LbFPMath_PI/2);
        navi->pos_next.x.val += (128) * small_around[i&3].delta_x;
        i = (creatng->move_angle_xy) / (LbFPMath_PI/2);
        navi->pos_next.y.val += (128) * small_around[i&3].delta_y;
        navi->navstate = 7;
        return 1;
    }
    case 5:
        stl_x = slab_subtile_center(subtile_slab(stl_num_decode_x(navi->first_colliding_block)));
        stl_y = slab_subtile_center(subtile_slab(stl_num_decode_y(navi->first_colliding_block)));
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->first_colliding_block = stl_num;
        navi->field_17 = stl_num;
        mapblk = get_map_block_at_pos(navi->first_colliding_block);
        if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            return 2;
        }
        navi->navstate = 1;
        return 1;
    case 7:
        if (get_2d_box_distance(&creatng->mappos, &navi->pos_next) > 16)
        {
            return 1;
        }
        if (navi->side == 1)
            angle = creatng->move_angle_xy + LbFPMath_PI/2;
        else
            angle = creatng->move_angle_xy - LbFPMath_PI/2;
        navi->angle = angle & LbFPMath_AngleMask;
        navi->navstate = 2;
        return 1;
    default:
        break;
    }
    return 1;
}

TbBool slab_good_for_computer_dig_path(const struct SlabMap *slb)
{
    const struct SlabAttr* slbattr = get_slab_attrs(slb);
    if ( ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) || (slb->kind == SlbT_LAVA) )
        return true;
    return false;
}

static TbBool is_valid_hug_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    const struct SlabAttr* slbattr = get_slab_attrs(slb);
    if ((slbattr->is_diggable) && !slab_kind_is_indestructible(slb->kind))
    {
        struct Map* mapblk = get_map_block_at(stl_x, stl_y);
        if (((mapblk->flags & SlbAtFlg_Filled) == 0) || (slabmap_owner(slb) == plyr_idx)) {
            SYNCDBG(17,"Subtile (%d,%d) rejected based on attrs",(int)stl_x,(int)stl_y);
            return false;
        }
    }
    if (!slab_good_for_computer_dig_path(slb)) {
        SYNCDBG(17,"Subtile (%d,%d) rejected as not good for dig",(int)stl_x,(int)stl_y);
        return false;
    }
    return true;
}

long dig_to_position(PlayerNumber plyr_idx, MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, int direction_around, TbBool revside)
{
    long round_change;
    SYNCDBG(14,"Starting for subtile (%d,%d)",(int)basestl_x,(int)basestl_y);
    if (revside) {
      round_change = 1;
    } else {
      round_change = 3;
    }
    long round_idx = (direction_around + SMALL_AROUND_LENGTH - round_change) % SMALL_AROUND_LENGTH;
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

static inline void get_hug_side_next_step(MapSubtlCoord dst_stl_x, MapSubtlCoord dst_stl_y, int dirctn, PlayerNumber plyr_idx,
    char *state, MapSubtlCoord *ostl_x, MapSubtlCoord *ostl_y, short *round, int *maxdist)
{
    MapSubtlCoord curr_stl_x = *ostl_x;
    MapSubtlCoord curr_stl_y = *ostl_y;
    unsigned short round_idx = small_around_index_in_direction(curr_stl_x, curr_stl_y, dst_stl_x, dst_stl_y);
    int dist = max(abs(curr_stl_x - dst_stl_x), abs(curr_stl_y - dst_stl_y));
    int dx = small_around[round_idx].delta_x;
    int dy = small_around[round_idx].delta_y;
    // If we can follow direction straight to the target, and we will get closer to it, then do it
    if ((dist <= *maxdist) && !is_valid_hug_subtile(curr_stl_x + STL_PER_SLB*dx, curr_stl_y + STL_PER_SLB*dy, plyr_idx))
    {
        curr_stl_x += STL_PER_SLB*dx;
        curr_stl_y += STL_PER_SLB*dy;
        *state = WaHSS_Val1;
        *maxdist = max(abs(curr_stl_x - dst_stl_x), abs(curr_stl_y - dst_stl_y));
    } else
    // If met second wall, finish
    if (*state == WaHSS_Val1)
    {
        *state = WaHSS_Val2;
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

short get_hug_side_options(MapSubtlCoord src_stl_x, MapSubtlCoord src_stl_y, MapSubtlCoord dst_stl_x, MapSubtlCoord dst_stl_y,
    unsigned short direction, PlayerNumber plyr_idx, MapSubtlCoord *ostla_x, MapSubtlCoord *ostla_y, MapSubtlCoord *ostlb_x, MapSubtlCoord *ostlb_y)
{
    SYNCDBG(4,"Starting");

    int dist = max(abs(src_stl_x - dst_stl_x), abs(src_stl_y - dst_stl_y));

    char state_a = WaHSS_Val0;
    MapSubtlCoord stl_a_x = src_stl_x;
    MapSubtlCoord stl_a_y = src_stl_y;
    short round_a = (direction + SMALL_AROUND_LENGTH + 1) % SMALL_AROUND_LENGTH;
    int maxdist_a = dist - 1;
    char state_b = WaHSS_Val0;
    MapSubtlCoord stl_b_x = src_stl_x;
    MapSubtlCoord stl_b_y = src_stl_y;
    short round_b = (direction + SMALL_AROUND_LENGTH - 1) % SMALL_AROUND_LENGTH;
    int maxdist_b = dist - 1;

    // Try moving in both directions
    for (int i = 150; i > 0; i--)
    {
        if ((state_a == WaHSS_Val2) && (state_b == WaHSS_Val2)) {
            break;
        }
        if (state_a != WaHSS_Val2)
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
        if (state_b != WaHSS_Val2)
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
