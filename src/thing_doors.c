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
#include "pre_inc.h"
#include "thing_doors.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "bflib_planar.h"
#include "cursor_tag.h"
#include "thing_objects.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "config_terrain.h"
#include "creature_senses.h"
#include "ariadne.h"
#include "ariadne_wallhug.h"
#include "map_blocks.h"
#include "map_ceiling.h"
#include "map_utils.h"
#include "sounds.h"
#include "gui_topmsg.h"
#include "game_legacy.h"
#include "frontmenu_ingame_map.h"
#include "player_instances.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

char const build_door_angle[] = {-1, -1, -1, -1, -1, 0, -1, 0, -1, -1, 1, 1, -1, 0, 1, -1 };

/******************************************************************************/

static void check_if_enemy_can_see_placement_of_hidden_door(struct Thing *doortng);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
char find_door_angle(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSlabCoord door_slb_x = subtile_slab(stl_x);
    MapSlabCoord door_slb_y = subtile_slab(stl_y);
    struct SlabMap* door_slb = get_slabmap_block(door_slb_x, door_slb_y);
    if ( door_slb->kind != SlbT_CLAIMED || slabmap_owner(door_slb) != plyr_idx )
    {
        return -1;
    }
    return determine_door_angle(door_slb_x, door_slb_y);
}

char get_door_orientation(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    if (!slab_is_door(slb_x, slb_y))
    {
        return -1;
    }
    return determine_door_angle(slb_x, slb_y);
}

char determine_door_angle(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    unsigned int wall_flags = 0;
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    for ( int i = 0; i < SMALL_AROUND_LENGTH; ++i )
    {
        wall_flags <<= 1;
        MapSubtlCoord astl_x = stl_x + (small_around[i].delta_x * 2);
        MapSubtlCoord astl_y = stl_y + (small_around[i].delta_y * 2);
        if (subtile_is_wall(astl_x,astl_y))
        {
            wall_flags |= 0x01;
        }
    }
    return build_door_angle[wall_flags];
}

struct Thing *create_door(struct Coord3d *pos, ThingModel tngmodel, unsigned char orient, PlayerNumber plyr_idx, TbBool is_locked)
{
    if (!i_can_allocate_free_thing_structure(TCls_Door))
    {
        ERRORDBG(3,"Cannot create door model %d (%s) for player %d. There are too many things allocated.",(int)tngmodel, door_code_name(tngmodel), (int)plyr_idx);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct Thing* doortng = allocate_free_thing_structure(TCls_Door);
    if (doortng->index == 0) {
        ERRORDBG(3,"Should be able to allocate door %d (%s) for player %d, but failed.",(int)tngmodel, door_code_name(tngmodel), (int)plyr_idx);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }

    struct DoorConfigStats* doorst = get_door_model_stats(tngmodel);

    doortng->class_id = TCls_Door;
    doortng->model = tngmodel;
    doortng->mappos.x.val = pos->x.val;
    doortng->mappos.y.val = pos->y.val;
    doortng->mappos.z.val = 384;
    doortng->next_on_mapblk = 0;
    doortng->parent_idx = doortng->index;
    doortng->owner = plyr_idx;
    doortng->rendering_flags |= TRF_Invisible;
    doortng->door.orientation = orient;
    doortng->active_state = DorSt_Closed;
    doortng->creation_turn = game.play_gameturn;
    doortng->health = doorst->health;
    doortng->door.is_locked = is_locked;
    if (doorst->model_flags & DoMF_Thick)
    {
        doortng->clipbox_size_xy = 3*COORD_PER_STL;
    }
    add_thing_to_its_class_list(doortng);
    place_thing_in_mapwho(doortng);
    check_if_enemy_can_see_placement_of_hidden_door(doortng);
    place_animating_slab_type_on_map(doorst->slbkind[orient], 0,  doortng->mappos.x.stl.num, doortng->mappos.y.stl.num, plyr_idx);
    ceiling_partially_recompute_heights(pos->x.stl.num - 1, pos->y.stl.num - 1, pos->x.stl.num + 2, pos->y.stl.num + 2);
    if (nav_map_initialised) // Can't update triangulation before map start
    {
        update_navigation_triangulation(pos->x.stl.num - 1, pos->y.stl.num - 1, pos->x.stl.num + 2, pos->y.stl.num + 2);
    }
    ++game.dungeon[plyr_idx].total_doors;
    return doortng; 
}

/**
 * Hides all secret door keys
 */
void init_keys()
{
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    for (int i = slist->index; i > 0;)
    {
        struct Thing* keytng = thing_get(i);
        i = keytng->next_of_class;
        if (keytng->model != ObjMdl_SpinningKey)
        {
            continue;
        }
        TRACE_THING(keytng);
        struct Thing* doortng = find_base_thing_on_mapwho(TCls_Door, 0, keytng->mappos.x.stl.num, keytng->mappos.y.stl.num);
        if (thing_is_invalid(doortng))
        {
            WARNLOG("Key (%d) has no door on position (%d,%d)", keytng->index, keytng->mappos.x.stl.num, keytng->mappos.y.stl.num);
            continue;
        }
        struct DoorConfigStats* doorst = get_door_model_stats(doortng->model);
        if (flag_is_set(doorst->model_flags, DoMF_Secret))
        {
            if (is_my_player_number(doortng->owner)) //On map init doors are never revealed
            {
                set_flag(keytng->rendering_flags,TRF_Transpar_4);
            }
            else
            {
                set_flag(keytng->rendering_flags,TRF_Invisible);
            }
        }
    }
}

TbBool remove_key_on_door(struct Thing *thing)
{
    struct Thing* keytng = find_base_thing_on_mapwho(TCls_Object, ObjMdl_SpinningKey, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(keytng))
        return false;
    delete_thing_structure(keytng, 0);
    return true;
}

TbBool add_key_on_door(struct Thing *thing)
{
    struct Thing* keytng = create_object(&thing->mappos, ObjMdl_SpinningKey, thing->owner, 0);
    if (thing_is_invalid(keytng))
      return false;
    struct DoorConfigStats* doorst = get_door_model_stats(thing->model);
    if (flag_is_set(doorst->model_flags, DoMF_Secret))
    {
        if (is_my_player_number(thing->owner) || !door_is_hidden_to_player(thing,my_player_number))
        {
            clear_flag(keytng->rendering_flags,TRF_Transpar_Flags);
            set_flag(keytng->rendering_flags,TRF_Transpar_4);
        }
        else
        {
            set_flag(keytng->rendering_flags, TRF_Invisible);
        }
    }
    keytng->mappos.x.stl.pos = COORD_PER_STL/2;
    keytng->mappos.y.stl.pos = COORD_PER_STL/2;
    keytng->mappos.z.stl.num = 5;
    return true;
}

void unlock_door(struct Thing *thing)
{
    thing->door.is_locked = false;
    game.map_changed_for_nagivation = 1;
    update_navigation_triangulation(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1,
      thing->mappos.x.stl.num+1, thing->mappos.y.stl.num+1);
    panel_map_update(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1, STL_PER_SLB, STL_PER_SLB);
    if (!remove_key_on_door(thing)) {
        WARNMSG("Cannot remove keyhole when unlocking door.");
    }
}

void lock_door(struct Thing *doortng)
{
    struct DoorConfigStats* doorst = get_door_model_stats(doortng->model);
    long stl_x = doortng->mappos.x.stl.num;
    long stl_y = doortng->mappos.y.stl.num;
    doortng->active_state = DorSt_Closed;
    doortng->door.closing_counter = 0;
    doortng->door.is_locked = 1;
    game.map_changed_for_nagivation = 1;
    place_animating_slab_type_on_map(doorst->slbkind[doortng->door.orientation], 0, stl_x, stl_y, doortng->owner);
    update_navigation_triangulation(stl_x-1,  stl_y-1, stl_x+1,stl_y+1);
    panel_map_update(stl_x-1, stl_y-1, STL_PER_SLB, STL_PER_SLB);
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
    if (doortng->door.orientation)
    {
        create_dirt_rubble_for_dug_block(stl_x, stl_y + 1, 4, plyr_idx);
        create_dirt_rubble_for_dug_block(stl_x, stl_y - 1, 4, plyr_idx);
    } else
    {
        create_dirt_rubble_for_dug_block(stl_x + 1, stl_y, 4, plyr_idx);
        create_dirt_rubble_for_dug_block(stl_x - 1, stl_y, 4, plyr_idx);
    }
    struct Thing* efftng = create_effect(&pos, TngEff_Dummy, plyr_idx);
    if (!thing_is_invalid(efftng)) {
        thing_play_sample(efftng, 72 + SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
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
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    place_slab_type_on_map(SlbT_CLAIMED, stl_x, stl_y, slabmap_owner(slb), 0);
    do_slab_efficiency_alteration(slb_x, slb_y);
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (!player_exists(player))
            continue;
        struct Thing* thing = thing_get(player->controlled_thing_idx);
        if (thing == INVALID_THING)
            continue;
        MapCoordDelta dist = get_chessboard_distance(&pos, &thing->mappos);
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
    if (!thing_exists(thing))
        return false;
    return (thing->class_id == TCls_Door);
}

TbBool thing_is_sellable_door(const struct Thing* thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Door)
        return false;
    struct DoorConfigStats* doorst = get_door_model_stats(thing->model);
    return (doorst->unsellable == 0);
}

TbBool slab_has_sellable_door(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Thing* doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    return thing_is_sellable_door(doortng);
}

TbBool door_can_stand(struct Thing *thing)
{
    unsigned int wall_flags = 0;
    for (int i = 0; i < SMALL_AROUND_LENGTH; i++)
    {
        wall_flags *= 2;
        long slb_x = subtile_slab(thing->mappos.x.stl.num) + (int)small_around[i].delta_x;
        long slb_y = subtile_slab(thing->mappos.y.stl.num) + (int)small_around[i].delta_y;
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        struct SlabConfigStats* slabst = get_slab_stats(slb);
        if ((slabst->category == SlbAtCtg_FortifiedWall) || (slb->kind == SlbT_ROCK) || (slabst->category == SlbAtCtg_FriableDirt) || (slb->kind == SlbT_GOLD) || (slb->kind == SlbT_DENSEGOLD) || (slb->kind == SlbT_GEMS))
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

TbBool door_will_open_for_thing(const struct Thing *doortng, const struct Thing *creatng)
{
  if ( !doortng->door.is_locked && thing_is_creature(creatng) )
  {
    if ( players_are_mutual_allies(doortng->owner,creatng->owner) )
    {
      return true;
    }
  }
  return false;
}

static void check_if_enemy_can_see_placement_of_hidden_door(struct Thing *doortng)
{
    struct DoorConfigStats* doorst = get_door_model_stats(doortng->model);
    if(!(doorst->model_flags & DoMF_Secret))
    {
        return;
    }
    MapSubtlCoord z = doortng->mappos.z.stl.num;
    doortng->mappos.z.stl.num = 2;

    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* creatng = thing_get(i);
        if (thing_is_invalid(creatng))
          break;

        if(creature_can_see_thing(creatng,doortng)) 
        {
            reveal_secret_door_to_player(doortng,creatng->owner);
        }

        i = creatng->next_of_class;
    }
    doortng->mappos.z.stl.num = z;
}

TbBool door_is_hidden_to_player(struct Thing *doortng,PlayerNumber plyr_idx)
{
    struct DoorConfigStats* doorst = get_door_model_stats(doortng->model);
    if((plyr_idx != doortng->owner) && flag_is_set(doorst->model_flags,DoMF_Secret))
    {
        return !flag_is_set(doortng->door.revealed,to_flag(plyr_idx));
    }
    return false;
}

void reveal_secret_door_to_player(struct Thing *doortng,PlayerNumber plyr_idx)
{
    if(!door_is_hidden_to_player(doortng,plyr_idx))
    {
        return;
    }
    struct Thing* keytng = find_base_thing_on_mapwho(TCls_Object, ObjMdl_SpinningKey, doortng->mappos.x.stl.num, doortng->mappos.y.stl.num);
    if (is_my_player_number(plyr_idx)) //reveal key too
    {
        clear_flag(keytng->rendering_flags, TRF_Invisible);
        set_flag(keytng->rendering_flags, TRF_Transpar_4);
    }
    if (!players_are_mutual_allies(plyr_idx, doortng->owner))
    {
        event_create_event(doortng->mappos.x.val, doortng->mappos.y.val, EvKind_SecretDoorDiscovered, plyr_idx, 0);
        event_create_event(doortng->mappos.x.val, doortng->mappos.y.val, EvKind_SecretDoorSpotted, doortng->owner, 0);
    }
    set_flag(doortng->door.revealed,to_flag(plyr_idx));
    MapSubtlCoord stl_x = doortng->mappos.x.stl.num;
    MapSubtlCoord stl_y = doortng->mappos.y.stl.num;
    update_navigation_triangulation(stl_x-1,  stl_y-1, stl_x+1,stl_y+1);
    panel_map_update(stl_x-1, stl_y-1, STL_PER_SLB, STL_PER_SLB);

}

long process_door_open(struct Thing *thing)
{
    // If doors are locked, delay to closing = 0
    if (thing->door.is_locked)
        thing->door.opening_counter = 0;
    if ( check_door_should_open(thing) )
    {
        thing->door.opening_counter = 10;
        return 0;
    }
    if (thing->door.opening_counter > 0)
    {
        thing->door.opening_counter--;
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
    struct DoorConfigStats* doorst = get_door_model_stats(thing->model);
    int old_frame = (thing->door.closing_counter / 256);
    short delta_h = doorst->open_speed;
    int slbparam = doorst->slbkind[thing->door.orientation];
    if (thing->door.closing_counter + delta_h < 768)
    {
        thing->door.closing_counter += delta_h;
    } else
    {
        thing->active_state = DorSt_Open;
        thing->door.opening_counter = 10;
        thing->door.closing_counter = 768;
    }
    int new_frame = (thing->door.closing_counter / 256);
    if (new_frame != old_frame)
      place_animating_slab_type_on_map(slbparam, new_frame, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner);
    return 1;
}

long process_door_closing(struct Thing *thing)
{
    int old_frame = (thing->door.closing_counter / 256);
    struct DoorConfigStats* doorst = get_door_model_stats(thing->model);
    int delta_h = doorst->open_speed;
    int slbparam = doorst->slbkind[thing->door.orientation];
    if ( check_door_should_open(thing) )
    {
        thing->active_state = DorSt_Opening;
        thing_play_sample(thing, 91, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    if (thing->door.closing_counter > delta_h)
    {
        thing->door.closing_counter -= delta_h;
    } else
    {
        thing->active_state = DorSt_Closed;
        thing->door.closing_counter = 0;
    }
    int new_frame = (thing->door.closing_counter / 256);
    if (new_frame != old_frame)
      place_animating_slab_type_on_map(slbparam, new_frame, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner);
    return 1;
}

TngUpdateRet process_door(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);

    struct DoorConfigStats* doorst = get_door_model_stats(thing->model);

    if (doorst->updatefn_idx < 0)
    {
        if (luafunc_thing_update_func(doorst->updatefn_idx, thing) <= 0) {
            return TUFRet_Deleted;
        }
    }

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

/**
 * Goes through thing list to count the traps of the given model.
 * @param owner The owning player to be checked.
 * @param model Trap model to count, or -1 for any.
 * @return the number of things of class trap with matching model and available shots.
 */
long count_player_deployed_traps_of_model(PlayerNumber owner, ThingModel model)
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

/**
 * Goes through thing list to find a trap matching the given model.
 * @param owner The owning player to be checked.
 * @param model Trap model to find, or -1 for any.
 * @return true when it finds any trap, false when not.
 */
TbBool player_has_deployed_trap_of_model(PlayerNumber owner, ThingModel model)
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

/**
 * Checks door crates in workshop and offmap doors available for placement.
 * @param plyr_idx The owning player to be checked.
 * @param model Door model to count, or -1 for all.
 * @return Amount of doors that the player may place.
 */
long count_player_available_doors_of_model(PlayerNumber plyr_idx, ThingModel model)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long count = 0;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Tried to count doors for Player %d which has no dungeon", (int)plyr_idx);
        return 0;
    }
    for (int i = 0; i < game.conf.trapdoor_conf.door_types_count; i++)
    {
        if ((i == model) || (model == -1))
        {
            count += dungeon->mnfct_info.door_amount_stored[i];
            count += dungeon->mnfct_info.door_amount_offmap[i];
        }
    }
    return count;
}

/**
 * Checks trap crates in workshop and offmap trapss available for placement.
 * @param plyr_idx The owning player to be checked.
 * @param model Trap model to count, or -1 for all.
 * @return Amount of traps that the player may place.
 */
long count_player_available_traps_of_model(PlayerNumber plyr_idx, ThingModel model)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long count = 0;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Tried to count traps for Player %d which has no dungeon", (int)plyr_idx);
        return 0;
    }
    for (int i = 0; i < game.conf.trapdoor_conf.trap_types_count; i++)
    {
        if ((i == model) || (model == -1))
        {
            count += dungeon->mnfct_info.trap_amount_stored[i];
            count += dungeon->mnfct_info.trap_amount_offmap[i];
        }
    }
    return count;
}

// Update all placed doors to new stats
void update_all_door_stats()
{
    const struct StructureList* slist = get_list_for_thing_class(TCls_Door);
    for(int i = slist->index; i > 0;)
    {
        struct Thing* thing = thing_get(i);
        i = thing->next_of_class;
        TRACE_THING(thing);
        struct DoorConfigStats* doorst = get_door_model_stats(thing->model);
        thing->health = doorst->health;
    }
}

void script_place_door(PlayerNumber plyridx, ThingModel doorkind, MapSlabCoord slb_x, MapSlabCoord slb_y, TbBool locked, TbBool free)
{
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    TbBool success;

    if (tag_cursor_blocks_place_door(plyridx, stl_x, stl_y))
    {
        if (!free)
        {
            if (!is_door_placeable(plyridx, doorkind))
            {
                return;
            }
        }
        success = player_place_door_without_check_at(stl_x, stl_y, plyridx, doorkind, free);
        if (success)
        {
            delete_room_slabbed_objects(get_slab_number(slb_x, slb_y));
            remove_dead_creatures_from_slab(slb_x, slb_y);
            if (locked)
            {
                struct Thing* doortng = get_door_for_position(stl_x, stl_y);
                if (!thing_is_invalid(doortng))
                {
                    lock_door(doortng);
                }
            }
        }
    }
}
/******************************************************************************/
