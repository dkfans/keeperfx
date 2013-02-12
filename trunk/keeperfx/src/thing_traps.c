/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_traps.c
 *     Traps support functions.
 * @par Purpose:
 *     Functions to support trap things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_traps.h"

#include "globals.h"
#include "bflib_basics.h"
#include "thing_data.h"
#include "map_utils.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct Thing *_DK_create_trap(struct Coord3d *pos, unsigned short a1, unsigned short a2);
DLLIMPORT struct Thing *_DK_get_trap_for_position(long pos_x, long pos_y);
DLLIMPORT struct Thing *_DK_get_trap_for_slab_position(long slb_x, long slb_y);
DLLIMPORT long _DK_update_trap(struct Thing *thing);
DLLIMPORT void _DK_update_trap_trigger(struct Thing *thing);
DLLIMPORT void _DK_external_activate_trap_shot_at_angle(struct Thing *thing, long a2);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_trap(unsigned char a1, long a2, long a3);

/******************************************************************************/
TbBool destroy_trap(struct Thing *thing)
{
    delete_thing_structure(thing, 0);
    return true;
}

TbBool trap_is_active(const struct Thing *thing)
{
    return ((thing->byte_13 > 0) && (thing->long_14 <= game.play_gameturn));
}

TbBool trap_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (thing->owner == plyr_idx)
    {
        return (thing->model == 1) && trap_is_active(thing);
    }
    return false;
}

struct Thing *get_trap_for_position(long pos_x, long pos_y)
{
    return _DK_get_trap_for_position(pos_x, pos_y);
}

struct Thing *get_trap_for_slab_position(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapCoord pos_x,pos_y;
    //return _DK_get_trap_for_slab_position(slb_x, slb_y);
    pos_x = subtile_coord_center(slab_subtile_center(slb_x));
    pos_y = subtile_coord_center(slab_subtile_center(slb_y));
    return get_trap_around_of_model_and_owned_by(pos_x, pos_y, -1, -1);
}

TbBool thing_is_deployed_trap(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Trap)
        return false;
    return true;
}

void update_trap_trigger(struct Thing *thing)
{
    _DK_update_trap_trigger(thing); return;
}

TngUpdateRet update_trap(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    //return _DK_update_trap(thing);
    if (thing->health < 0)
    {
        delete_thing_structure(thing, 0);
        return TUFRet_Deleted;
    }
    if (trap_is_active(thing))
    {
        update_trap_trigger(thing);
    }
    if ( map_pos_is_lava(thing->mappos.x.stl.num, thing->mappos.y.stl.num)
      && ((thing->field_1 & TF1_IsDragged1) == 0) && ((thing->alloc_flags & TAlF_IsDragged) == 0) )
    {
        delete_thing_structure(thing, 0);
        return TUFRet_Deleted;
    }
    return TUFRet_Modified;
}

struct Thing *create_trap(struct Coord3d *pos, unsigned short a1, unsigned short a2)
{
    SYNCDBG(7,"Starting");
    return _DK_create_trap(pos, a1, a2);
}

void init_traps(void)
{
    struct Thing *thing;
    int i, k;
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per thing code
        if (thing->byte_13 == 0)
        {
            thing->byte_13 = game.traps_config[thing->model].shots;
            thing->field_4F ^= (thing->field_4F ^ (trap_stats[thing->model].field_12 << 4)) & 0x30;
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

/**
 * Finds index into trap data array for a given trap/door class and model.
 * @param wrkshop_class
 * @param wrkshop_index
 * @return -1 if not found, otherwise index where 0 <= index < MANUFCTR_TYPES_COUNT
 */
int get_trap_data_index(int wrkshop_class, int wrkshop_index)
{
    //TODO: verify that this actually works (I don't know about wrkshop_class relation to field_0)
    //either way this function is needed, so if erroneous simply correct and leave to be
    int i;
    for (i=0; i < MANUFCTR_TYPES_COUNT; i++)
    {
        if ((trap_data[i].field_0 == wrkshop_class) && (wrkshop_index == trap_data[i].field_4)) {
            return i;
        }
    }
    return -1;
}

/**
 * Removes traps on the subtile and all sibling subtiles.
 *
 * @param stl_x Central subtile X coordinate.
 * @param stl_y Central subtile Y coordinate.
 * @param sell_value Value to be added to treasury if selling traps; if not selling but just removing, should be null.
 * @return Amount of traps removed.
 */
long remove_traps_around_subtile(long stl_x, long stl_y, long *sell_value)
{
    long i,k;
    long total;
    total = 0;
    for (k=0; k < AROUND_TILES_COUNT; k++)
    {
        struct Thing *thing;
        thing = get_trap_for_position(stl_x+around[k].delta_x, stl_y+around[k].delta_y);
        if (!thing_is_invalid(thing))
        {
            if (sell_value != NULL) {
                i = game.traps_config[thing->model].selling_value;
                if (thing->trap.num_shots == 0) {
                    remove_workshop_object_from_player(thing->owner, trap_to_object[thing->model%TRAP_TYPES_COUNT]);
                }
                (*sell_value) += i;
            }
            destroy_trap(thing);
            total++;
        }
    }
    return total;
}

void external_activate_trap_shot_at_angle(struct Thing *thing, long a2)
{
    _DK_external_activate_trap_shot_at_angle(thing, a2);
}

unsigned char tag_cursor_blocks_place_trap(unsigned char a1, long a2, long a3)
{
    SYNCDBG(7,"Starting");
    return _DK_tag_cursor_blocks_place_trap(a1, a2, a3);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
