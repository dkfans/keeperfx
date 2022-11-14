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
#include "thing_physics.h"
#include "thing_navigate.h"
#include "engine_camera.h"
#include "config_terrain.h"
#include "creature_control.h"
#include "creature_states.h"
#include "config_creature.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT signed char _DK_get_starting_angle_and_side_of_hug(struct Thing *creatng, struct Coord3d *pos, long *a3, unsigned char *a4, long a5, unsigned char direction);
DLLIMPORT long _DK_get_map_index_of_first_block_thing_colliding_with_travelling_to(struct Thing *creatng, struct Coord3d *startpos, struct Coord3d *endpos, long a4, unsigned char a5);
DLLIMPORT long _DK_get_map_index_of_first_block_thing_colliding_with_at(struct Thing *creatng, struct Coord3d *pos, long a3, unsigned char a4);
/******************************************************************************/
struct Around const my_around_eight[] = {
  { 0,-1}, { 1,-1},
  { 1, 0}, { 1, 1},
  { 0, 1}, {-1, 1},
  {-1, 0}, {-1,-1},
};

struct Around const my_around_nine[] = {
  {-1,-1}, { 0,-1}, { 1,-1},
  {-1, 0}, { 0, 0}, { 1, 0},
  {-1, 1}, { 0, 1}, { 1, 1},
};

short const around_map[] = {-257, -256, -255, -1, 0, 1, 255, 256, 257};

/**
 * Should contain values encoded with get_subtile_number(). */
const unsigned short small_around_pos[] = {
  0xFF00, 0x0001, 0x0100, 0xFFFF,
};


struct Around const start_at_around[] = {
    { 0,  0}, {-1, -1}, {-1,  0},
    {-1,  1}, { 0, -1}, { 0,  1},
    { 1, -1}, { 1,  0}, { 1,  1},
};

/******************************************************************************/
/**
 * Computes index in small_around[] array which contains coordinates directing towards given destination.
 * @param srcpos_x Source position X; either map coordinates or subtiles, but have to match type of other coords.
 * @param srcpos_y Source position Y; either map coordinates or subtiles, but have to match type of other coords.
 * @param dstpos_x Destination position X; either map coordinates or subtiles, but have to match type of other coords.
 * @param dstpos_y Destination position Y; either map coordinates or subtiles, but have to match type of other coords.
 * @return Index for small_around[] array.
 */
int small_around_index_in_direction(long srcpos_x, long srcpos_y, long dstpos_x, long dstpos_y)
{
    long i = ((LbArcTanAngle(dstpos_x - srcpos_x, dstpos_y - srcpos_y) & LbFPMath_AngleMask) + LbFPMath_PI / 4);
    return (i >> 9) & 3;
}

TbBool can_step_on_unsafe_terrain_at_position(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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

TbBool hug_can_move_on(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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

TbBool wallhug_angle_with_collide_valid(struct Thing *thing, long a2, long move_delta, long angle, unsigned char a4)
{
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(move_delta, angle);
    pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(move_delta, angle);
    pos.z.val = get_thing_height_at(thing, &pos);
    return (creature_cannot_move_directly_to_with_collide(thing, &pos, a2, a4) != 4);
}

long get_angle_of_wall_hug(struct Thing *creatng, long a2, long a3, unsigned char a4)
{
    struct Navigation *navi;
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        navi = &cctrl->navi;
    }
    long quadr;
    long whangle;
    switch (navi->field_1[0])
    {
    case 1:
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr - 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * (quadr & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 2) & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        break;
    case 2:
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * (quadr & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr - 1) & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        quadr = angle_to_quadrant(creatng->move_angle_xy);
        whangle = (LbFPMath_PI/2) * ((quadr + 2) & 3);
        if (wallhug_angle_with_collide_valid(creatng, a2, a3, whangle, a4))
          return whangle;
        break;
    }
    return -1;
}

static short hug_round_new(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, unsigned short round_idx, long *hug_val)
{

    signed int delta_x_1;
    unsigned __int16 v7;
    int delta_y;
    signed int delta_x;
    int around_idx3;
    struct Thing *doortng2;
    int v15;
    signed int v16;
    signed int v17;
    unsigned __int16 around_idx2;
    int around_idx1;
    struct Thing *doortng1;
    int kind;
    int v24;
    unsigned __int16 v25;
    int v26;
    signed int v27;
    int around_idx5;
    MapSubtlCoord stl_y_4;
    MapSubtlCoord stl_x_4;
    struct Thing *doortng4;
    int slb_kind;
    int v33;
    signed int v34;
    signed int v35;                    
    unsigned __int16 v36;              
    int around_idx4;
    struct Thing *doortng3;
    int v41;
    int v42;                           
    __int16 result;                    
    unsigned __int8 v44;                
    char bool2;                        
    unsigned __int16 biggest_delta_minus1;  
    char round_idx_plus1_2;            
    unsigned __int16 round_idx_minus1; 
    char round_idx_minus1_2;
    unsigned __int16 v53;
    unsigned __int16 v54;
    unsigned __int16 i;

    MapSubtlCoord pos2_stl_x = pos2->x.stl.pos;
    MapSubtlCoord pos2_stl_y = pos2->y.stl.pos;
    MapSubtlCoord pos1_stl_x = pos1->x.stl.pos;
    MapSubtlCoord pos1_stl_y = pos1->y.stl.pos;
    round_idx_plus1_2 = (round_idx + 1) & 3;
    MapSubtlCoord pos2_stl_y_2 = pos2->y.stl.pos;
    MapSubtlCoord pos2_stl_x_2 = pos2->x.stl.pos;
    signed int biggest_delta = abs(pos1->y.stl.pos - pos2_stl_y_2);
    delta_x_1 = abs(pos1->x.stl.pos - pos2_stl_x_2);
    if (biggest_delta <= delta_x_1)
        biggest_delta = delta_x_1;
    TbBool bool1 = 0;
    biggest_delta_minus1 = biggest_delta - 1;
    bool2 = 0;
    MapSubtlCoord pos1_stl_x_2 = pos1->x.stl.pos;
    MapSubtlCoord pos1_stl_y_2 = pos1->y.stl.pos;
    round_idx_minus1_2 = (round_idx - 1) & 3;

    for (i = *hug_val; i; --i)
    {
        v7 = (((LbArcTanAngle(pos2_stl_x_2 - pos1_stl_x, pos2_stl_y_2 - pos1_stl_y) & LbFPMath_AngleMask) + 256) >> 9) & 3;
        delta_y = abs(pos1_stl_y - pos2_stl_y_2);
        delta_x = abs(pos1_stl_x - pos2_stl_x_2);
        if (delta_y <= delta_x)
            delta_y = delta_x;
        if (delta_y > biggest_delta_minus1)
        {
        LABEL_21:
            if (bool1 == 1)
            {
                pos1->y.stl.pos = pos1_stl_y;
                pos1->x.stl.pos = pos1_stl_x;
                result = 0;
                *hug_val -= i;
                return result;
            }
            v53 = 0;
            around_idx2 = (round_idx_plus1_2 - 1) & 3;
            while (2)
            {
                around_idx1 = around_idx2;
                MapSubtlCoord stl_y_1 = 3 * small_around[around_idx1].delta_y + pos1_stl_y;
                MapSubtlCoord stl_x_1 = 3 * small_around[around_idx1].delta_x + pos1_stl_x;
                doortng1 = get_door_for_position(stl_x_1, stl_y_1);

                struct SlabMap *slb_1 = get_slabmap_for_subtile(stl_x_1, stl_y_1);

                kind = slb_1->kind;
                struct SlabAttr *slbattr_1 = get_slab_attrs(slb_1);

                if ( (slbattr_1->block_flags & 0x40) != 0 )
                {
                    if (!doortng1 || doortng1->owner != creatng->owner || doortng1->door.is_locked)
                    {
                        v24 = 0;
                    LABEL_33:
                        if (v24)
                        {
                            round_idx_plus1_2 = around_idx2;
                            pos1_stl_x += 3 * small_around[around_idx2].delta_x;
                            pos1_stl_y += 3 * small_around[around_idx2].delta_y;
                            goto LABEL_37;
                        }
                        around_idx2 = (around_idx2 + 1) & 3;
                        if (++v53 >= 4u)
                            goto LABEL_37;
                        continue;
                    }
                }
                else if (!slbattr_1->is_safe_land && (kind != SlbT_LAVA || creature_stats_get_from_thing(creatng)->hurt_by_lava))
                {
                    v24 = 0;
                    goto LABEL_33;
                }
                break;
            }
            v24 = 1;
            goto LABEL_33;
        }
        around_idx3 = v7;
        MapSubtlCoord stl_y_2 = 3 * small_around[around_idx3].delta_y + pos1_stl_y;
        MapSubtlCoord stl_x_2 = 3 * small_around[around_idx3].delta_x + pos1_stl_x;
        doortng2 = get_door_for_position(stl_x_2, stl_y_2);



        struct SlabMap *slb_2 = get_slabmap_for_subtile(stl_y_2, stl_x_2);
        kind = slb_2->kind;
        struct SlabAttr *slbattr_2 = get_slab_attrs(slb_2);

        if ((slbattr_2->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            if (!doortng2 || doortng2->owner != creatng->owner || doortng2->door.is_locked)
            {
                v15 = 0;
                goto LABEL_17;
            }
        }
        else if (!slbattr_2->is_safe_land && (kind != SlbT_LAVA || creature_stats_get_from_thing(creatng)->hurt_by_lava))
        {
            v15 = 0;
            goto LABEL_17;
        }
        v15 = 1;
    LABEL_17:
        if (!v15)
            goto LABEL_21;
        pos1_stl_x += 3 * small_around[around_idx3].delta_x;
        pos1_stl_y += 3 * small_around[around_idx3].delta_y;
        v16 = abs(pos1_stl_y - pos2_stl_y_2);
        v17 = abs(pos1_stl_x - pos2_stl_x_2);
        if (v16 <= v17)
            v16 = v17;
        bool1 = 1;
        biggest_delta_minus1 = v16;
    LABEL_37:
        if (pos2_stl_x == pos1_stl_x && pos1_stl_y == pos2_stl_y)
        {
            result = 1;
            *hug_val -= i;
            return result;
        }
        v25 = (((LbArcTanAngle(pos2_stl_x_2 - pos1_stl_x_2, pos2_stl_y_2 - pos1_stl_y_2) & 0x7FFu) + 256) >> 9) & 3;
        v26 = abs(pos1_stl_y_2 - pos2_stl_y_2);
        v27 = abs(pos1_stl_x_2 - pos2_stl_x_2);
        if (v26 <= v27)
            v26 = v27;
        if (v26 > round_idx_minus1)
        {
        LABEL_56:
            if (bool2 == 1)
            {
                pos1->y.stl.pos = pos1_stl_y_2;
                pos1->x.stl.pos = pos1_stl_x_2;
                result = 0;
                *hug_val -= i;
                return result;
            }
            v54 = 0;
            v36 = (round_idx_minus1_2 + 1) & 3; 
            while (2)
            {
                around_idx4 = v36;
                MapSubtlCoord stl_x_3 = 3 * small_around[around_idx4].delta_x + pos1_stl_x_2;
                MapSubtlCoord stl_y_3 = 3 * small_around[around_idx4].delta_y + pos1_stl_y_2;
                doortng3 = get_door_for_position(stl_x_3, stl_y_3);

                struct SlabMap *slb_3 = get_slabmap_for_subtile(stl_x_3, stl_y_3);
                v41 = slb_3->kind;
                struct SlabAttr *slbattr_3 = get_slab_attrs(slb_3);
                
                if ((slbattr_3->block_flags & SlbAtFlg_IsDoor) != 0)
                {
                    if (!doortng3 || doortng3->owner != creatng->owner || doortng3->door.is_locked)
                    {
                        v42 = 0;
                    LABEL_68:
                        if (v42)
                        {
                            round_idx_minus1_2 = v36;
                            pos1_stl_x_2 += 3 * small_around[v36].delta_x;
                            pos1_stl_y_2 += 3 * small_around[v36].delta_y;
                            goto LABEL_72;
                        }
                        v36 = (v36 - 1) & 3;
                        if (++v54 >= 4u)
                            goto LABEL_72;
                        continue;
                    }
                }
                else if (!slbattr_3->is_safe_land && (v41 != SlbT_LAVA || creature_stats_get_from_thing(creatng)->hurt_by_lava))
                {
                    v42 = 0;
                    goto LABEL_68;
                }
                break;
            }
            v42 = 1;
            goto LABEL_68;
        }
        around_idx5 = v25;
        stl_y_4 = 3 * small_around[around_idx5].delta_y + pos1_stl_y_2;
        stl_x_4 = 3 * small_around[around_idx5].delta_x + pos1_stl_x_2;
        doortng4 = get_door_for_position(stl_x_4, stl_y_4);

        struct SlabMap *slb_4 = get_slabmap_for_subtile(stl_y_4, stl_x_4);
        slb_kind = slb_4->kind;
        struct SlabAttr *slbattr_4 = get_slab_attrs(slb_4);

        if ((slbattr_4->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            if (!doortng4 || creatng->owner != doortng4->owner || doortng4->door.is_locked)
            {
                v33 = 0;
                goto LABEL_52;
            }
        }
        else if (!slbattr_4->is_safe_land && (slb_kind != SlbT_LAVA || creature_stats_get_from_thing(creatng)->hurt_by_lava))
        {
            v33 = 0;
            goto LABEL_52;
        }
        v33 = 1;
    LABEL_52:
        if (!v33)
            goto LABEL_56;
        pos1_stl_x_2 += 3 * small_around[around_idx5].delta_x;
        pos1_stl_y_2 += 3 * small_around[around_idx5].delta_y;
        v34 = abs(pos1_stl_y_2 - pos2_stl_y_2);
        v35 = abs(pos1_stl_x_2 - pos2_stl_x_2);
        if (v34 <= v35)
            v34 = v35;
        bool2 = 1;
        round_idx_minus1 = v34;
    LABEL_72:
        if (pos2_stl_x == pos1_stl_x_2 && pos1_stl_y_2 == pos2_stl_y)
        {
            result = 1;
            *hug_val -= i;
            return result;
        }
    }
    if (!i)
        return -1;
    if ((unsigned __int16)(abs(pos1_stl_x_2 - pos2_stl_x_2) + abs(pos1_stl_y_2 - pos2_stl_y_2)) >= (unsigned __int16)(abs(pos1_stl_x - pos2_stl_x_2) + abs(pos1_stl_y - pos2_stl_y_2)))
    {
        v44 = pos1_stl_y;
        pos1->x.stl.pos = pos1_stl_x;
    }
    else
    {
        v44 = pos1_stl_y_2;
        pos1->x.stl.pos = pos1_stl_x_2;
    }
    pos1->y.stl.pos = v44;
    result = 0;
    *hug_val -= i;
    return result;
}

DLLIMPORT short _DK_hug_round(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, unsigned short a4, long *a5);

static short hug_round(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, unsigned short round_idx, long *hug_val)
{

    struct Coord3d old_pos1;
    struct Coord3d old_pos2;
    long old_hug_val = *hug_val;

    old_pos1.x = pos1->x;
    old_pos1.y = pos1->y;
    old_pos1.z = pos1->z;
    old_pos2.x = pos2->x;
    old_pos2.y = pos2->y;
    old_pos2.z = pos2->z;

    short old_return = _DK_hug_round(creatng, &old_pos1, &old_pos2, round_idx, &old_hug_val);

    short return_val = hug_round_new(creatng, pos1, pos2, round_idx, hug_val);


    if (old_return != return_val) JUSTLOG("return not same return %d,%d",old_return,return_val);
    if (old_return == return_val) JUSTLOG("ok %d,%d",old_return,return_val);
    if (old_pos1.x.val != pos1->x.val) JUSTLOG("pos1  %d,%d  %d,%d",old_pos1.x.val,old_pos1.y.val, pos1->x.val,pos1->y.val);
    if (old_pos2.x.val != pos2->x.val) JUSTLOG("pos1  %d,%d  %d,%d",old_pos2.x.val,old_pos2.y.val, pos2->x.val,pos2->y.val);
    if (old_hug_val == *hug_val) JUSTLOG("hug_val  %d,%d",old_hug_val,*hug_val);

    return return_val;
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

signed char get_starting_angle_and_side_of_hug(struct Thing *creatng, struct Coord3d *pos, long *a3, unsigned char *a4, long a5, unsigned char a6)
{
    return _DK_get_starting_angle_and_side_of_hug(creatng, pos, a3, a4, a5, a6);
}

unsigned short get_hugging_blocked_flags(struct Thing *creatng, struct Coord3d *pos, long a3, unsigned char a4)
{
    struct Coord3d tmpos;
    unsigned short blkflags = 0;
    {
        tmpos.x.val = pos->x.val;
        tmpos.y.val = creatng->mappos.y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, a3, a4) == 4) {
            blkflags |= 0x01;
        }
    }
    {
        tmpos.x.val = creatng->mappos.x.val;
        tmpos.y.val = pos->y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, a3, a4) == 4) {
            blkflags |= 0x02;
        }
    }
    if (blkflags == 0)
    {
        tmpos.x.val = pos->x.val;
        tmpos.y.val = pos->y.val;
        tmpos.z.val = creatng->mappos.z.val;
        if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, a3, a4) == 4) {
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

long get_map_index_of_first_block_thing_colliding_with_at(struct Thing *creatng, struct Coord3d *pos, long a3, unsigned char a4)
{
    return _DK_get_map_index_of_first_block_thing_colliding_with_at(creatng, pos, a3, a4);
}

long creature_cannot_move_directly_to_with_collide_sub(struct Thing *creatng, struct Coord3d pos, long a3, unsigned char a4)
{
    if (thing_in_wall_at(creatng, &pos))
    {
        pos.z.val = subtile_coord(map_subtiles_z,COORD_PER_STL-1);
        MapCoord height = get_thing_height_at(creatng, &pos);
        if ((height >= subtile_coord(map_subtiles_z,COORD_PER_STL-1)) || (height - creatng->mappos.z.val > COORD_PER_STL))
        {
            if (get_map_index_of_first_block_thing_colliding_with_at(creatng, &pos, a3, a4) >= 0) {
                return 4;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

long creature_cannot_move_directly_to_with_collide(struct Thing *creatng, struct Coord3d *pos, long a3, unsigned char a4)
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
        cannot_mv = creature_cannot_move_directly_to_with_collide_sub(creatng, *pos, a3, a4);
        return cannot_mv;
    }

    if (cross_x_boundary_first(&prev_pos, pos))
    {
        if (pos->x.val <= prev_pos.x.val)
            clpcor = (prev_pos.x.val & 0xFF00) - 1;
        else
            clpcor = (prev_pos.x.val + COORD_PER_STL) & 0xFF00;
        next_pos.x.val = clpcor;
        next_pos.y.val = dt_y * abs(clpcor - orig_pos.x.val) / dt_x + orig_pos.y.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, prev_pos, a3, a4))
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
            clpcor = (prev_pos.y.val & 0xFF00) - 1;
        else
            clpcor = (prev_pos.y.val + COORD_PER_STL) & 0xFF00;
        next_pos.y.val = clpcor;
        next_pos.x.val = dt_x * abs(clpcor - orig_pos.y.val) / dt_y + orig_pos.x.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, a3, a4))
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
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, a3, a4))
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
            clpcor = (prev_pos.y.val & 0xFF00) - 1;
        else
            clpcor = (prev_pos.y.val + COORD_PER_STL) & 0xFF00;
        next_pos.y.val = clpcor;
        next_pos.x.val = dt_x * abs(clpcor - orig_pos.y.val) / dt_y + orig_pos.x.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, a3, a4))
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
            clpcor = (prev_pos.x.val & 0xFF00) - 1;
        else
            clpcor = (prev_pos.x.val + COORD_PER_STL) & 0xFF00;
        next_pos.x.val = clpcor;
        next_pos.y.val = dt_y * abs(clpcor - orig_pos.x.val) / dt_x + orig_pos.y.val;
        next_pos.z.val = prev_pos.z.val;
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, next_pos, a3, a4))
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
        switch (creature_cannot_move_directly_to_with_collide_sub(creatng, *pos, a3, a4))
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
    switch (creature_cannot_move_directly_to_with_collide_sub(creatng, *pos, a3, a4))
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

TbBool thing_can_continue_direct_line_to(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, long a4, long a5, unsigned char a6)
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
    return creature_cannot_move_directly_to_with_collide(creatng, &posb, a4, a6) != 4
        && creature_cannot_move_directly_to_with_collide(creatng, &posc, a4, a6) != 4
        && creature_cannot_move_directly_to_with_collide(creatng, &posa, a4, a6) != 4;
}

static TbBool check_forward_for_prospective_hugs(struct Thing *creatng, struct Coord3d *pos_a, long angle, long navi_field_1, long a3, long speed, unsigned char a4)
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
                pos.y.stl.pos = (nav_radius + creatng->mappos.y.val - 256) / COORD_PER_STL;
                pos.y.stl.num = -1;
                pos.y.val -= nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        case ANGLE_EAST:
            if ((int)((nav_radius + pos_a->x.val) & 0xFFFFFF00) > (int)((nav_radius + creatng->mappos.x.val) & 0xFFFFFF00))
            {
                pos.y.val = pos_a->y.val;
                pos.x.stl.pos = (creatng->mappos.x.val - nav_radius + 256) / COORD_PER_STL;
                pos.x.stl.num = 0;
                pos.x.val += nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        case ANGLE_SOUTH:
            if ((int)((nav_radius + pos_a->y.val) & 0xFFFFFF00) > (int)((nav_radius + creatng->mappos.y.val) & 0xFFFFFF00))
            {
                pos.x.val = pos_a->x.val;
                pos.y.stl.pos = (creatng->mappos.y.val - nav_radius + 256) / COORD_PER_STL;
                pos.y.stl.num = 0;
                pos.y.val += nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        case ANGLE_WEST:
            if ((int)((pos_a->x.val - nav_radius) & 0xFFFFFF00) < (int)((creatng->mappos.x.val - nav_radius) & 0xFFFFFF00))
            {
                pos.y.val = pos_a->y.val;
                pos.x.stl.pos = (uint16_t)(nav_radius + creatng->mappos.x.val - 256) / COORD_PER_STL;
                pos.x.stl.num = -1;
                pos.x.val -= nav_radius;
                pos.z.val = get_thing_height_at(creatng, &pos);
                break;
            }
            return false;
        default:
            return false;
    }
    if ( navi->field_1[0] == 1 )
    {
        quadrant_angle = (((unsigned char)angle_to_quadrant(angle) - 1) & 3) << 9;

        next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
        next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
        next_pos.z.val = get_thing_height_at(creatng, &next_pos);
        if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, a3, a4) == 4)
        {
            stored_creature_pos = creatng->mappos;
            creatng->mappos.x.val = pos.x.val;
            creatng->mappos.y.val = pos.y.val;
            creatng->mappos.z.val = pos.z.val;
            quadrant_angle = (((unsigned char)angle_to_quadrant(angle) - 1) & 3) << 9;
            next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
            next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
            next_pos.z.val = get_thing_height_at(creatng, &next_pos);
            if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, a3, a4) != 4)
            {
                *pos_a = pos;
                creatng->mappos = stored_creature_pos;
                return true;
            }
            creatng->mappos = stored_creature_pos;
        }
    }
    if ( navi->field_1[0] != 2 )
        return false;
    quadrant_angle = (((unsigned char)angle_to_quadrant(angle) + 1) & 3) << 9;
    next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
    next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
    next_pos.z.val = get_thing_height_at(creatng, &next_pos);
    if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, a3, a4) != 4)
        return false;
    stored_creature_pos = creatng->mappos;
    creatng->mappos = pos;
    quadrant_angle = (((unsigned char)angle_to_quadrant(angle) + 1) & 3) << 9;
    next_pos.x.val = move_coord_with_angle_x(creatng->mappos.x.val,speed,quadrant_angle);
    next_pos.y.val = move_coord_with_angle_y(creatng->mappos.y.val,speed,quadrant_angle);
    next_pos.z.val = get_thing_height_at(creatng, &next_pos);


    if (creature_cannot_move_directly_to_with_collide(creatng, &next_pos, a3, a4) == 4)
    {
        creatng->mappos = stored_creature_pos;
        return false;
    }
    *pos_a = pos;
    creatng->mappos = stored_creature_pos;
    return true;
}

TbBool find_approach_position_to_subtile(const struct Coord3d *srcpos, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MoveSpeed spacing, struct Coord3d *aproachpos)
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

long get_map_index_of_first_block_thing_colliding_with_travelling_to(struct Thing *creatng, struct Coord3d *startpos, struct Coord3d *endpos, long a4, unsigned char a5)
{
    return _DK_get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, startpos, endpos, a4, a5);
}

TbBool navigation_push_towards_target(struct Navigation *navi, struct Thing *creatng, const struct Coord3d *pos, MoveSpeed speed, MoveSpeed nav_radius, unsigned char a3)
{
    navi->navstate = 2;
    navi->pos_next.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
    navi->pos_next.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
    navi->pos_next.z.val = get_thing_height_at(creatng, &navi->pos_next);
    struct Coord3d pos1;
    pos1.x.val = navi->pos_next.x.val;
    pos1.y.val = navi->pos_next.y.val;
    pos1.z.val = navi->pos_next.z.val;
    check_forward_for_prospective_hugs(creatng, &pos1, navi->angle, navi->field_1[0], 33, speed, a3);
    if (get_2d_box_distance(&pos1, &creatng->mappos) > 16)
    {
        navi->pos_next.x.val = pos1.x.val;
        navi->pos_next.y.val = pos1.y.val;
        navi->pos_next.z.val = pos1.z.val;
    }
    navi->field_9 = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
    int cannot_move = creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, 33, a3);
    if (cannot_move == 4)
    {
        navi->pos_next.x.val = creatng->mappos.x.val;
        navi->pos_next.y.val = creatng->mappos.y.val;
        navi->pos_next.z.val = creatng->mappos.z.val;
        navi->field_9 = 0;
    }
    navi->field_5 = get_2d_box_distance(&creatng->mappos, pos);
    if (cannot_move == 1)
    {
        SubtlCodedCoords stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, 40, 0);
        navi->field_15 = stl_num;
        MapSubtlCoord stl_x = slab_subtile_center(subtile_slab_fast(stl_num_decode_x(stl_num)));
        MapSubtlCoord stl_y = slab_subtile_center(subtile_slab_fast(stl_num_decode_y(stl_num)));
        find_approach_position_to_subtile(&creatng->mappos, stl_x, stl_y, nav_radius + 385, &navi->pos_next);
        navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
        navi->navstate = 3;
    }
    return true;
}

long get_next_position_and_angle_required_to_tunnel_creature_to(struct Thing *creatng, struct Coord3d *pos, unsigned char a3)
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
    a3 |= (1 << creatng->owner);
    //return _DK_get_next_position_and_angle_required_to_tunnel_creature_to(creatng, pos, a3);
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
        if (dist_to_next >= navi->field_9) {
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

            cannot_move = creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, 33, a3);
            if (cannot_move == 4)
            {
                struct SlabMap *slb;
                stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, 40, 0);
                slb = get_slabmap_for_subtile(stl_num_decode_x(stl_num), stl_num_decode_y(stl_num));
                unsigned short ownflag;
                ownflag = 0;
                if (!slabmap_block_invalid(slb)) {
                    ownflag = 1 << slabmap_owner(slb);
                }
                navi->field_19[0] = ownflag;

                if (get_starting_angle_and_side_of_hug(creatng, &navi->pos_next, &navi->angle, navi->field_1, 33, a3))
                {
                    block_flags = get_hugging_blocked_flags(creatng, &navi->pos_next, 33, a3);
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
                    navi->field_1[2] = 0;
                    navi->field_1[1] = 0;
                    navi->field_4 = 0;
                }
            }
            if (cannot_move == 1)
            {
                stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, 40, 0);
                navi->field_15 = stl_num;
                nav_radius = thing_nav_sizexy(creatng) / 2;
                stl_x = slab_subtile_center(subtile_slab_fast(stl_num_decode_x(stl_num)));
                stl_y = slab_subtile_center(subtile_slab_fast(stl_num_decode_y(stl_num)));
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
                navigation_push_towards_target(navi, creatng, pos, speed, thing_nav_sizexy(creatng)/2, a3);
            }
        }
        return 1;
    case 2:
        dist_to_next = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            if ((dist_to_next > navi->field_9) || creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, 33, a3))
            {
                navi->navstate = 1;
                navi->pos_final.x.val = pos->x.val;
                navi->pos_final.y.val = pos->y.val;
                navi->pos_final.z.val = pos->z.val;
                navi->field_1[2] = 0;
                navi->field_1[1] = 0;
                navi->field_4 = 0;
                return 1;
            }
            return 1;
        }
        if ((get_2d_box_distance(&creatng->mappos, pos) < navi->field_5)
          && thing_can_continue_direct_line_to(creatng, &creatng->mappos, pos, 33, 1, a3))
        {
            navi->navstate = 1;
            navi->pos_final.x.val = pos->x.val;
            navi->pos_final.y.val = pos->y.val;
            navi->pos_final.z.val = pos->z.val;
            navi->field_1[2] = 0;
            navi->field_1[1] = 0;
            navi->field_4 = 0;
            return 1;
        }
        if (creatng->move_angle_xy != navi->angle) {
            return 1;
        }
        angle = get_angle_of_wall_hug(creatng, 33, speed, a3);
        if (angle != navi->angle)
        {
          tmpos.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
          tmpos.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
          tmpos.z.val = get_thing_height_at(creatng, &tmpos);
          if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, 33, a3) == 4)
          {
              block_flags = get_hugging_blocked_flags(creatng, &tmpos, 33, a3);
              set_hugging_pos_using_blocked_flags(&tmpos, creatng, block_flags, thing_nav_sizexy(creatng)/2);
              if (get_2d_box_distance(&tmpos, &creatng->mappos) > 16)
              {
                  navi->pos_next.x.val = tmpos.x.val;
                  navi->pos_next.y.val = tmpos.y.val;
                  navi->pos_next.z.val = tmpos.z.val;
                  navi->field_9 = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
                  return 1;
              }
          }
        }
        if (((angle + LbFPMath_PI/2) & LbFPMath_AngleMask) == navi->angle)
        {
            if (navi->field_1[2] == 1)
            {
                navi->field_1[1]++;
            } else
            {
                navi->field_1[2] = 1;
                navi->field_1[1] = 1;
            }
        } else
        if (((angle - LbFPMath_PI/2) & LbFPMath_AngleMask) == navi->angle)
        {
          if (navi->field_1[2] == 2)
          {
              navi->field_1[1]++;
          } else
          {
              navi->field_1[2] = 2;
              navi->field_1[1] = 1;
          }
        } else
        {
          navi->field_1[1] = 0;
          navi->field_1[2] = 0;
        }
        if (navi->field_1[1] >= 4)
        {
            navi->navstate = 1;
            navi->pos_final.x.val = pos->x.val;
            navi->pos_final.y.val = pos->y.val;
            navi->pos_final.z.val = pos->z.val;
            navi->field_1[2] = 0;
            navi->field_1[1] = 0;
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
        check_forward_for_prospective_hugs(creatng, &tmpos, navi->angle, navi->field_1[0], 33, speed, a3);
        if (get_2d_box_distance(&tmpos, &creatng->mappos) > 16)
        {
            navi->pos_next.x.val = tmpos.x.val;
            navi->pos_next.y.val = tmpos.y.val;
            navi->pos_next.z.val = tmpos.z.val;
        }
        navi->field_9 = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        cannot_move = creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, 33, a3);
        if (cannot_move == 4)
        {
          ERRORLOG("I've been given a shite position");
          tmpos.x.val = creatng->mappos.x.val + distance_with_angle_to_coord_x(speed, navi->angle);
          tmpos.y.val = creatng->mappos.y.val + distance_with_angle_to_coord_y(speed, navi->angle);
          tmpos.z.val = get_thing_height_at(creatng, &tmpos);
          if (creature_cannot_move_directly_to_with_collide(creatng, &tmpos, 33, a3) == 4) {
              ERRORLOG("It's even more shit than I first thought");
          }
          navi->navstate = 1;
          navi->pos_final.x.val = pos->x.val;
          navi->pos_final.y.val = pos->y.val;
          navi->pos_final.z.val = pos->z.val;
          navi->field_1[2] = 0;
          navi->field_1[1] = 0;
          navi->field_4 = 0;
          navi->pos_next.x.val = creatng->mappos.x.val;
          navi->pos_next.y.val = creatng->mappos.y.val;
          navi->pos_next.z.val = creatng->mappos.z.val;
          return 1;
        }
        if (cannot_move != 1)
        {
            navi->field_9 = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
            return 1;
        }
        stl_num = get_map_index_of_first_block_thing_colliding_with_travelling_to(creatng, &creatng->mappos, &navi->pos_next, 40, 0);
        navi->field_15 = stl_num;
        nav_radius = thing_nav_sizexy(creatng) / 2;
        stl_x = slab_subtile_center(subtile_slab_fast(stl_num_decode_x(stl_num)));
        stl_y = slab_subtile_center(subtile_slab_fast(stl_num_decode_y(stl_num)));
        find_approach_position_to_subtile(&creatng->mappos, stl_x, stl_y, nav_radius + 385, &navi->pos_next);
        navi->angle = get_angle_xy_to(&creatng->mappos, &navi->pos_next);
        navi->field_1[1] = 0;
        navi->field_1[2] = 0;
        navi->field_9 = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        navi->navstate = 4;
        return 1;
    case 4:
        dist_to_next = get_2d_box_distance(&creatng->mappos, &navi->pos_next);
        if (dist_to_next > 16)
        {
            if (get_2d_box_distance(&creatng->mappos, &navi->pos_next) > navi->field_9
             || creature_cannot_move_directly_to_with_collide(creatng, &navi->pos_next, 33, a3))
            {
                navi->navstate = 1;
                navi->pos_final.x.val = pos->x.val;
                navi->pos_final.y.val = pos->y.val;
                navi->pos_final.z.val = pos->z.val;
                navi->field_1[2] = 0;
                navi->field_1[1] = 0;
                navi->field_4 = 0;
            }
            navi->navstate = 4;
            return 1;
        }
        stl_x = slab_subtile_center(subtile_slab_fast(stl_num_decode_x(navi->field_15)));
        stl_y = slab_subtile_center(subtile_slab_fast(stl_num_decode_y(navi->field_15)));
        tmpos.x.val = subtile_coord_center(stl_x);
        tmpos.y.val = subtile_coord_center(stl_y);
        navi->angle = get_angle_xy_to(&creatng->mappos, &tmpos);
        navi->field_1[1] = 0;
        navi->field_1[2] = 0;
        navi->field_9 = 0;
        if (get_angle_difference(creatng->move_angle_xy, navi->angle) != 0) {
            navi->navstate = 4;
            return 1;
        }
        navi->navstate = 6;
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->field_15 = stl_num;
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
        stl_x = slab_subtile_center(subtile_slab_fast(stl_num_decode_x(navi->field_15)));
        stl_y = slab_subtile_center(subtile_slab_fast(stl_num_decode_y(navi->field_15)));
        tmpos.x.val = subtile_coord_center(stl_x);
        tmpos.y.val = subtile_coord_center(stl_y);
        navi->angle = get_angle_xy_to(&creatng->mappos, &tmpos);
        if (get_angle_difference(creatng->move_angle_xy, navi->angle) != 0) {
            navi->navstate = 3;
            return 1;
        }
        navi->navstate = 5;
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->field_15 = stl_num;
        navi->field_17 = stl_num;
        return 2;
    case 6:
    {
        stl_x = slab_subtile_center(subtile_slab_fast(stl_num_decode_x(navi->field_15)));
        stl_y = slab_subtile_center(subtile_slab_fast(stl_num_decode_y(navi->field_15)));
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->field_15 = stl_num;
        navi->field_17 = stl_num;
        mapblk = get_map_block_at_pos(navi->field_15);
        if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
          return 2;
        }
        nav_radius = thing_nav_sizexy(creatng) / 2;
        long i;
        if (navi->field_1[0] == 1)
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
        stl_x = slab_subtile_center(subtile_slab_fast(stl_num_decode_x(navi->field_15)));
        stl_y = slab_subtile_center(subtile_slab_fast(stl_num_decode_y(navi->field_15)));
        stl_num = get_subtile_number(stl_x,stl_y);
        navi->field_15 = stl_num;
        navi->field_17 = stl_num;
        mapblk = get_map_block_at_pos(navi->field_15);
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
        if (navi->field_1[0] == 1)
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

TbBool is_valid_hug_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
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
    if ((dist <= *maxdist) && is_valid_hug_subtile(curr_stl_x + STL_PER_SLB*dx, curr_stl_y + STL_PER_SLB*dy, plyr_idx))
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
