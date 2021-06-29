/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_doors.c
 *     Door things support functions.
 * @par Purpose:
 *     Functions to create and operate on door things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_doors.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "thing_objects.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "config_terrain.h"
#include "creature_senses.h"
#include "ariadne.h"
#include "ariadne_wallhug.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "sounds.h"
#include "gui_topmsg.h"
#include "game_legacy.h"
#include "frontmenu_ingame_map.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char const build_door_angle[] = {-1, -1, -1, -1, -1, 0, -1, 0, -1, -1, 1, 1, -1, 0, 1, -1 };
/* Obsolete - use DoorConfigStats instead
const short door_names[] = {
    201, 590, 591, 592, 593, 0,
};
*/
/******************************************************************************/
DLLIMPORT struct Thing *_DK_create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4);
DLLIMPORT char _DK_find_door_angle(unsigned char stl_x, unsigned char stl_y, unsigned char plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
char find_door_angle(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    return _DK_find_door_angle(stl_x, stl_y, plyr_idx);
}

char get_door_orientation(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    if (!slab_is_door(slb_x, slb_y))
    {
        return -1;
    }
    else if ( ((slab_is_wall(slb_x-1, slb_y))) && ((slab_is_wall(slb_x+1, slb_y))) && ((!slab_is_wall(slb_x, slb_y-1))) && ((!slab_is_wall(slb_x, slb_y+1))) )
    {
        return 0;    
    }
    else if ( ((slab_is_wall(slb_x-1, slb_y))) && ((slab_is_wall(slb_x+1, slb_y))) && ((slab_is_wall(slb_x, slb_y-1))) && ((!slab_is_wall(slb_x, slb_y+1))) )
    {
        return 0;    
    }
    else if ( ((slab_is_wall(slb_x-1, slb_y))) && ((slab_is_wall(slb_x+1, slb_y))) && ((!slab_is_wall(slb_x, slb_y-1))) && ((slab_is_wall(slb_x, slb_y+1))) )
    {
        return 0;    
    }
    else if ( ((slab_is_wall(slb_x, slb_y-1))) && ((slab_is_wall(slb_x, slb_y+1))) && ((!slab_is_wall(slb_x-1, slb_y))) && ((!slab_is_wall(slb_x+1, slb_y))) )
    {
        return 1;    
    }
    else if ( ((slab_is_wall(slb_x, slb_y-1))) && ((slab_is_wall(slb_x, slb_y+1))) && ((slab_is_wall(slb_x-1, slb_y))) && ((!slab_is_wall(slb_x+1, slb_y))) )
    {
        return 1;    
    }
    else if ( ((slab_is_wall(slb_x, slb_y-1))) && ((slab_is_wall(slb_x, slb_y+1))) && ((!slab_is_wall(slb_x-1, slb_y))) && ((slab_is_wall(slb_x+1, slb_y))) )
    {
        return 1;    
    }
    else if ( ((slab_is_wall(slb_x, slb_y-1))) && ((slab_is_wall(slb_x, slb_y+1))) && ((slab_is_wall(slb_x-1, slb_y))) && ((slab_is_wall(slb_x+1, slb_y))) )
    {
        return -1;    
    }
    else
    {
        return -1;
    }
}

struct Thing *create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4)
{
  return _DK_create_door(pos, a1, a2, a3, a4);
}

TbBool remove_key_on_door(struct Thing *thing)
{
    struct Thing* keytng = find_base_thing_on_mapwho(TCls_Object, 44, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(keytng))
        return false;
    delete_thing_structure(keytng, 0);
    return true;
}

TbBool add_key_on_door(struct Thing *thing)
{
    struct Thing* keytng = create_object(&thing->mappos, 44, thing->owner, 0);
    if (thing_is_invalid(keytng))
      return false;
    keytng->mappos.x.stl.pos = COORD_PER_STL/2;
    keytng->mappos.y.stl.pos = COORD_PER_STL/2;
    keytng->mappos.z.stl.num = 4;
    return true;
}

void unlock_door(struct Thing *thing)
{
    thing->trap_door_active_state = 0;
    game.field_14EA4B = 1;
    update_navigation_triangulation(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1,
      thing->mappos.x.stl.num+1, thing->mappos.y.stl.num+1);
    pannel_map_update(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1, STL_PER_SLB, STL_PER_SLB);
    if (!remove_key_on_door(thing)) {
        WARNMSG("Cannot remove keyhole when unlocking door.");
    }
}

void lock_door(struct Thing *doortng)
{
    struct DoorStats* dostat = &door_stats[doortng->model][doortng->door.orientation];
    long stl_x = doortng->mappos.x.stl.num;
    long stl_y = doortng->mappos.y.stl.num;
    doortng->active_state = DorSt_Closed;
    doortng->door.word_16d = 0;
    doortng->door.is_locked = 1;
    game.field_14EA4B = 1;
    place_animating_slab_type_on_map(dostat->slbkind, 0, stl_x, stl_y, doortng->owner);
    update_navigation_triangulation(stl_x-1,  stl_y-1, stl_x+1,stl_y+1);
    pannel_map_update(stl_x-1, stl_y-1, STL_PER_SLB, STL_PER_SLB);
    if (!add_key_on_door(doortng)) {
        WARNMSG("Cannot create a keyhole when locking a door.");
    }
}

long destroy_door(struct Thing *doortng)
{
    SYNCDBG(18,"Starting for %s index %d owned by player %d",thing_model_name(doortng),(int)doortng->index,(int)doortng->owner);
    struct Coord3d pos;
    pos.x.val = doortng->mappos.x.val;
    pos.y.val = doortng->mappos.y.val;
    pos.z.val = doortng->mappos.z.val;
    MapSubtlCoord stl_x = pos.x.stl.num;
    MapSubtlCoord stl_y = pos.y.stl.num;
    PlayerNumber plyr_idx = doortng->owner;
    remove_key_on_door(doortng);
    ceiling_partially_recompute_heights(stl_x - 1, stl_y - 1, stl_x + 2, stl_y + 2);
    create_dirt_rubble_for_dug_block(stl_x, stl_y, 4, plyr_idx);
    if (doortng->belongs_to)
    {
        create_dirt_rubble_for_dug_block(stl_x, stl_y + 1, 4, plyr_idx);
        create_dirt_rubble_for_dug_block(stl_x, stl_y - 1, 4, plyr_idx);
    } else
    {
        create_dirt_rubble_for_dug_block(stl_x + 1, stl_y, 4, plyr_idx);
        create_dirt_rubble_for_dug_block(stl_x - 1, stl_y, 4, plyr_idx);
    }
    struct Thing* efftng = create_effect(&pos, TngEff_DamageBlood, plyr_idx);
    if (!thing_is_invalid(efftng)) {
        thing_play_sample(efftng, 72 + UNSYNC_RANDOM(4), NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
    }
    if (plyr_idx != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
        if (!dungeon_invalid(dungeon)) {
            dungeon->total_doors--;
            dungeon->doors_destroyed++;
        }
    }
    delete_thing_structure(doortng, 0);
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    place_slab_type_on_map(SlbT_CLAIMED, stl_x, stl_y, slabmap_owner(slb), 0);
    do_slab_efficiency_alteration(slb_x, slb_y);
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (!player_exists(player))
            continue;
        struct Thing* thing = thing_get(player->controlled_thing_idx);
        long dist = get_2d_box_distance(&pos, &thing->mappos);
        long sight_stl = slab_subtile(get_explore_sight_distance_in_slabs(thing), 0);
        if (dist <= subtile_coord(sight_stl,0)) {
            check_map_explored(thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        }
    }
    return 1;
}

TbBool subtile_has_door_thing_on(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* doortng = get_door_for_position(stl_x, stl_y);
    return !thing_is_invalid(doortng);
}

TbBool subtile_has_door_thing_on_for_trap_placement(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* doortng = get_door_for_position_for_trap_placement(stl_x, stl_y);
    return !thing_is_invalid(doortng);
}

TbBool slab_row_has_door_thing_on(MapSlabCoord slb_x, MapSubtlCoord stl_y)
{
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    if (subtile_has_door_thing_on_for_trap_placement(stl_x, stl_y))
    {
        return true;
    }
    if (subtile_has_door_thing_on_for_trap_placement(stl_x-1, stl_y))
    {
        return true;
    }
    if (subtile_has_door_thing_on_for_trap_placement(stl_x+1, stl_y))
    {
        return true;
    }
    return false;
}

TbBool slab_column_has_door_thing_on(MapSubtlCoord stl_x, MapSlabCoord slb_y)
{
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    if (subtile_has_door_thing_on_for_trap_placement(stl_x, stl_y))
    {
        return true;
    }
    if (subtile_has_door_thing_on_for_trap_placement(stl_x, stl_y-1))
    {
        return true;
    }
    if (subtile_has_door_thing_on_for_trap_placement(stl_x, stl_y+1))
    {
        return true;
    }
    return false;
}

TbBool subtile_has_locked_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* doortng = get_door_for_position(stl_x, stl_y);
    return (!thing_is_invalid(doortng) && doortng->door.is_locked);
}

TbBool thing_is_deployed_door(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Door)
        return false;
    return true;
}

TbBool door_can_stand(struct Thing *thing)
{
    unsigned int wall_flags = 0;
    for (int i = 0; i < 4; i++)
    {
        wall_flags *= 2;
        long slb_x = subtile_slab_fast(thing->mappos.x.stl.num) + (int)small_around[i].delta_x;
        long slb_y = subtile_slab_fast(thing->mappos.y.stl.num) + (int)small_around[i].delta_y;
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        struct SlabAttr* slbattr = get_slab_attrs(slb);
        if ((slbattr->category == SlbAtCtg_FortifiedWall) || (slb->kind == SlbT_ROCK) || (slbattr->category == SlbAtCtg_FriableDirt) || (slb->kind == SlbT_GOLD) || (slb->kind == SlbT_GEMS))
            wall_flags |= 0x01;
    }
    // The array needs to have 2^4 = 16 values
    return (build_door_angle[wall_flags] != -1);
}

TbBool check_door_should_open(struct Thing *thing)
{
    // If doors are locked, never should open
    if (thing->door.is_locked != 0)
    {
        return false;
    }
    struct Thing* openertng = get_creature_in_range_and_owned_by_or_allied_with(thing->mappos.x.val, thing->mappos.y.val, 5, thing->owner);
    if (thing_is_invalid(openertng))
    {
        return false;
    }
    return true;
}

long process_door_open(struct Thing *thing)
{
    // If doors are locked, delay to closing = 0
    if (thing->door.is_locked)
        thing->door.byte_15d = 0;
    if ( check_door_should_open(thing) )
    {
        thing->door.byte_15d = 10;
        return 0;
    }
    if (thing->door.byte_15d > 0)
    {
        thing->door.byte_15d--;
        return 0;
    }
    thing->active_state = DorSt_Closing;
    thing_play_sample(thing, 92, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return 1;
}

long process_door_closed(struct Thing *thing)
{
    if ( !check_door_should_open(thing) )
      return 0;
    thing->active_state = DorSt_Opening;
    thing_play_sample(thing, 91, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return 1;
}

long process_door_opening(struct Thing *thing)
{
    struct DoorStats* dostat = &door_stats[thing->model][thing->door.orientation];
    int old_frame = (thing->door.word_16d / 256);
    short delta_h = dostat->field_6;
    int slbparam = dostat->slbkind;
    if (thing->door.word_16d+delta_h < 768)
    {
        thing->door.word_16d += delta_h;
    } else
    {
        thing->active_state = DorSt_Open;
        thing->door.byte_15d = 10;
        thing->door.word_16d = 768;
    }
    int new_frame = (thing->door.word_16d / 256);
    if (new_frame != old_frame)
      place_animating_slab_type_on_map(slbparam, new_frame, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner);
    return 1;
}

long process_door_closing(struct Thing *thing)
{
    int old_frame = (thing->door.word_16d / 256);
    struct DoorStats* dostat = &door_stats[thing->model][thing->door.orientation];
    int delta_h = dostat->field_6;
    int slbparam = dostat->slbkind;
    if ( check_door_should_open(thing) )
    {
        thing->active_state = DorSt_Opening;
        thing_play_sample(thing, 91, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    if (thing->door.word_16d > delta_h)
    {
        thing->door.word_16d -= delta_h;
    } else
    {
        thing->active_state = DorSt_Closed;
        thing->door.word_16d = 0;
    }
    int new_frame = (thing->door.word_16d / 256);
    if (new_frame != old_frame)
      place_animating_slab_type_on_map(slbparam, new_frame, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner);
    return 1;
}

TngUpdateRet process_door(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    if ( !door_can_stand(thing) || (thing->health <= 0) )
    {
        thing->health = -1;
        destroy_door(thing);
        return TUFRet_Deleted;
    }
    if ((thing->door.orientation > 1) || (thing->door.orientation < 0))
    {
        ERRORLOG("Invalid %s (index %d) orientation %d",thing_model_name(thing),(int)thing->index,(int)thing->door.orientation);
        thing->door.orientation &= 1;
    }
    SYNCDBG(18,"State %d",(int)thing->active_state);
    switch (thing->active_state)
    {
    case DorSt_Open:
        process_door_open(thing);
        break;
    case DorSt_Closed:
        process_door_closed(thing);
        break;
    case DorSt_Opening:
        process_door_opening(thing);
        break;
    case DorSt_Closing:
        process_door_closing(thing);
        break;
    default:
        ERRORLOG("Invalid %s state %d",thing_model_name(thing),(int)thing->active_state);
        thing->active_state = DorSt_Closing;
        break;
    }
    return TUFRet_Modified;
}

long count_player_deployed_doors_of_model(PlayerNumber owner, int model)
{
    long n = 0;
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Door);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner == owner) && ((thing->model == model) || (model == -1)))
            n++;
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}
/**
 * Returns whether the player has any door deployed which matches given properties.
 * @param owner The owning player to be checked.
 * @param model Door model selection, or -1 for any.
 * @param locked Door locked state selection, or -1 for any.
 * @return
 */
TbBool player_has_deployed_door_of_model(PlayerNumber owner, int model, short locked)
{
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Door);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner == owner) &&
            ((thing->model == model) || (model == -1)) &&
            ((thing->door.is_locked == locked) || (locked == -1)))
            return true;
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return false;
}

long count_player_deployed_traps_of_model(PlayerNumber owner, int model)
{
    long n = 0;
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner == owner) && ((thing->model == model) || (model == -1)) && (thing->trap.num_shots > 0))
            n++;
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}

TbBool player_has_deployed_trap_of_model(PlayerNumber owner, int model)
{
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner == owner) && ((thing->model == model) || (model == -1)))
            return true;
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return false;
}

// Update all placed doors to new stats
void update_all_door_stats()
{
    const struct StructureList* slist = get_list_for_thing_class(TCls_Door);
    for(int i = slist->index; i > 0;)
    {
        struct Thing* thing = thing_get(i);
        i = thing->next_of_class
        TRACE_THING(thing);
        struct DoorStats* dostat = &door_stats[thing->model][thing->door.orientation];
        thing->health = dostat->health;
    }
}
/******************************************************************************/
