/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_workshop.c
 *     Workshop room maintain functions.
 * @par Purpose:
 *     Functions to create and use workshops.
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
#include "room_workshop.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_physics.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "config_terrain.h"
#include "power_hand.h"
#include "game_legacy.h"
#include "gui_soundmsgs.h"
#include "player_instances.h"
#include "creature_states.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_remove_workshop_item(long a1, long a2, long a3);
DLLIMPORT long _DK_remove_workshop_object_from_player(long a1, long a2);
DLLIMPORT long _DK_get_next_manufacture(struct Dungeon *dungeon);
DLLIMPORT long _DK_process_player_manufacturing(int plr_idx);
/******************************************************************************/
/**
 * Stores manufacturable items.
 * Was originally named trap_data.
 */
struct ManufactureData manufacture_data[] = {
    {PSt_None,      TCls_Empty,0,   0,   0},
    {PSt_PlaceTrap, TCls_Trap, 1, 130, 152},
    {PSt_PlaceTrap, TCls_Trap, 2, 132, 154},
    {PSt_PlaceTrap, TCls_Trap, 3, 134, 156},
    {PSt_PlaceTrap, TCls_Trap, 4, 136, 158},
    {PSt_PlaceTrap, TCls_Trap, 5, 138, 160},
    {PSt_PlaceTrap, TCls_Trap, 6, 140, 162},
    {PSt_PlaceDoor, TCls_Door, 1, 144, 166},
    {PSt_PlaceDoor, TCls_Door, 2, 146, 168},
    {PSt_PlaceDoor, TCls_Door, 3, 148, 170},
    {PSt_PlaceDoor, TCls_Door, 4, 150, 172},
};
/******************************************************************************/


#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Returns manufacture data for a given manufacture index.
 * @param manufctr_idx Manufacture array index.
 * @return Dummy entry pinter if not found, manufacture data pointer otherwise.
 */
struct ManufactureData *get_manufacture_data(int manufctr_idx)
{
    if ((manufctr_idx < 0) || (manufctr_idx >= MANUFCTR_TYPES_COUNT)) {
        return &manufacture_data[0];
    }
    return &manufacture_data[manufctr_idx];
}

/**
 * Finds index into manufactures data array for a given trap/door class and model.
 * @param tngclass Manufacturable thing class.
 * @param tngmodel Manufacturable thing model.
 * @return 0 if not found, otherwise index where 1 <= index < MANUFCTR_TYPES_COUNT
 */
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel)
{
    int i;
    for (i=1; i < MANUFCTR_TYPES_COUNT; i++)
    {
        struct ManufactureData *manufctr;
        manufctr = &manufacture_data[i];
        if ((manufctr->tngclass == tngclass) && (manufctr->tngmodel == tngmodel)) {
            return i;
        }
    }
    return 0;
}

/**
 * Returns manufacture data for a given trap/door class and model.
 * @param tngclass Manufacturable thing class.
 * @param tngmodel Manufactureble thing model.
 * @return Dummy entry pinter if not found, manufacture data pointer otherwise.
 */
struct ManufactureData *get_manufacture_data_for_thing(ThingClass tngclass, ThingModel tngmodel)
{
    int i;
    for (i=1; i < MANUFCTR_TYPES_COUNT; i++)
    {
        struct ManufactureData *manufctr;
        manufctr = &manufacture_data[i];
        if ((manufctr->tngclass == tngclass) && (manufctr->tngmodel == tngmodel)) {
            return manufctr;
        }
    }
    return &manufacture_data[0];
}

TbBool add_workshop_object_to_workshop(struct Room *room,struct Thing *cratetng)
{
    if (room->kind != RoK_WORKSHOP) {
        SYNCDBG(4,"Crate %s owned by player %d can't be placed in a %s owned by player %d, expected proper workshop",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return add_item_to_room_capacity(room, true);
}

struct Thing *create_crate_in_workshop(struct Room *room, ThingModel cratngmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Coord3d pos;
    struct Thing *cratetng;
    if (room->kind != RoK_WORKSHOP) {
        SYNCDBG(4,"Cannot add crate to %s owned by player %d",room_code_name(room->kind),(int)room->owner);
        return INVALID_THING;
    }
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    cratetng = create_object(&pos, cratngmodel, room->owner, -1);
    if (thing_is_invalid(cratetng))
    {
        return INVALID_THING;
    }
    // Neutral thing do not need any more processing
    if (is_neutral_thing(cratetng))
    {
        return cratetng;
    }
    if (!add_workshop_object_to_workshop(room, cratetng)) {
        ERRORLOG("Could not fit %s in %s index %d",
            thing_model_name(cratetng),room_code_name(room->kind),(int)room->index);
        remove_item_from_room_capacity(room);
        destroy_object(cratetng);
        return INVALID_THING;
    }
    ThingClass tngclass;
    ThingModel tngmodel;
    tngclass = crate_thing_to_workshop_item_class(cratetng);
    tngmodel = crate_thing_to_workshop_item_model(cratetng);
    add_workshop_item_to_amounts(cratetng->owner, tngclass, tngmodel);
    return cratetng;
}

TbBool create_workshop_object_in_workshop_room(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel)
{
    struct Coord3d pos;
    struct Thing *cratetng;
    struct Room *room;
    struct Dungeon *dungeon;
    SYNCDBG(7,"Making player %d new %s",(int)plyr_idx,thing_class_code_name(tngclass));
    //return _DK_create_workshop_object_in_workshop_room(plyr_idx, tng_class, tng_kind);
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    switch (tngclass)
    {
    case TCls_Trap:
        cratetng = create_object(&pos, trap_crate_object_model(tngmodel), plyr_idx, -1);
        break;
    case TCls_Door:
        cratetng = create_object(&pos, door_crate_object_model(tngmodel), plyr_idx, -1);
        break;
    default:
        cratetng = INVALID_THING;
        ERRORLOG("No known workshop crate can represent %s model %d",thing_class_code_name(tngclass),(int)tngmodel);
        break;
    }
    if (thing_is_invalid(cratetng))
    {
        ERRORLOG("Could not create workshop crate thing for %s",thing_class_code_name(tngclass));
        return false;
    }
    room = find_random_room_for_thing_with_spare_room_item_capacity(cratetng, plyr_idx, RoK_WORKSHOP, 0);
    if (room_is_invalid(room))
    {
        ERRORLOG("No %s room found which would accept %s crate",room_code_name(RoK_WORKSHOP),thing_class_code_name(tngclass));
        destroy_object(cratetng);
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room_avoiding_object(cratetng, room, &pos))
    {
        ERRORLOG("Could not find a place in %s index %d for the new %s crate",
            room_code_name(room->kind),(int)room->index,thing_class_code_name(tngclass));
        destroy_object(cratetng);
        return false;
    }
    pos.z.val = get_thing_height_at(cratetng, &pos);
    move_thing_in_map(cratetng, &pos);
    if (!add_workshop_object_to_workshop(room, cratetng)) {
        ERRORLOG("Could not fit %s crate in %s index %d",
            thing_class_code_name(tngclass),room_code_name(room->kind),(int)room->index);
        destroy_object(cratetng);
        return false;
    }
    dungeon = get_players_num_dungeon(plyr_idx);
    switch (tngclass)
    {
    case TCls_Trap:
        if ((dungeon->trap_build_flags[tngmodel] & MnfBldF_Built) == 0) {
            event_create_event(cratetng->mappos.x.val, cratetng->mappos.y.val, EvKind_NewTrap, plyr_idx, tngmodel);
        }
        break;
    case TCls_Door:
        if ((dungeon->door_build_flags[tngmodel] & MnfBldF_Built) == 0) {
          event_create_event(cratetng->mappos.x.val, cratetng->mappos.y.val, EvKind_NewDoor, plyr_idx, tngmodel);
        }
        break;
    default:
        break;
    }
    create_effect(&pos, TngEff_Unknown56, cratetng->owner);
    thing_play_sample(cratetng, 89, 100, 0, 3, 0, 2, 256);
    return true;
}

/**
 * Removes workshop item from its capacity.
 * @param room The workshop room.
 * @param cratetng The thing to be removed.
 * @return True if the thing was removed, false if a problem prevented the removal.
 * @see remove_workshop_object_from_player() is a higher level function to be used for such removal
 */
TbBool remove_workshop_object_from_workshop(struct Room *room, struct Thing *cratetng)
{
    if ( (room->kind != RoK_WORKSHOP) || (cratetng->owner != room->owner) ) {
        SYNCDBG(4,"Crate %s owned by player %d found in a %s owned by player %d, instead of proper workshop",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return remove_item_from_room_capacity(room);
}

TbBool set_manufacture_level(struct Dungeon *dungeon)
{
    int wrkshp_slabs;
    wrkshp_slabs = count_slabs_of_room_type(dungeon->owner, RoK_WORKSHOP);
    if (wrkshp_slabs <= 9)
    {
        dungeon->manufacture_level = 0;
    } else
    if (wrkshp_slabs <= 16)
    {
        dungeon->manufacture_level = 1;
    } else
    if (wrkshp_slabs <= 25)
    {
        if (wrkshp_slabs == 20) // why there's special code for 20 slabs!?
            dungeon->manufacture_level = 4;
        else
            dungeon->manufacture_level = 2;
    } else
    if (wrkshp_slabs <= 36)
    {
        dungeon->manufacture_level = 3;
    } else
    {
        dungeon->manufacture_level = 4;
    }
    return true;
}

struct Thing *get_workshop_box_thing(PlayerNumber owner, ThingModel objmodel)
{
    struct Thing *thing;
    int i,k;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ( ((thing->alloc_flags & TAlF_Exists) != 0) && (thing->model == objmodel) && (thing->owner == owner) )
        {
            struct Room *room;
            room = get_room_thing_is_on(thing);
            if (!thing_is_picked_up(thing) && (room->kind == RoK_WORKSHOP) && (room->owner == owner))
                return thing;
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

/**
 * Adds item to the amount of crates in workshops, but also to the amount available to be placed.
 * @param owner
 * @param tngclass
 * @param tngmodel
 * @return
 * @note was named add_workshop_item()
 */
TbBool add_workshop_item_to_amounts_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, const char *func_name)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't add item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return false;
    }
    switch (tngclass)
    {
    case TCls_Trap:
        SYNCDBG(8,"%s: Adding Trap %s",func_name,trap_code_name(tngmodel));
        dungeon->trap_amount_stored[tngmodel]++;
        dungeon->trap_amount_placeable[tngmodel]++;
        dungeon->trap_build_flags[tngmodel] |= MnfBldF_Built;
        // In case the placeable amount lost it, do a fix
        if (dungeon->trap_amount_placeable[tngmodel] > dungeon->trap_amount_stored[tngmodel]+dungeon->trap_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s traps amount for player %d was too large; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->trap_amount_placeable[tngmodel] = dungeon->trap_amount_stored[tngmodel]+dungeon->trap_amount_offmap[tngmodel];
        }
        if (dungeon->trap_amount_placeable[tngmodel] < dungeon->trap_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s traps amount for player %d was too small; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->trap_amount_placeable[tngmodel] = dungeon->trap_amount_offmap[tngmodel];
        }
        break;
    case TCls_Door:
        SYNCDBG(8,"%s: Adding Door %s",func_name,door_code_name(tngmodel));
        dungeon->door_amount_stored[tngmodel]++;
        dungeon->door_amount_placeable[tngmodel]++;
        dungeon->door_build_flags[tngmodel] |= MnfBldF_Built;
        // In case the placeable amount lost it, do a fix
        if (dungeon->door_amount_placeable[tngmodel] > dungeon->door_amount_stored[tngmodel]+dungeon->door_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s doors amount for player %d was too large; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->door_amount_placeable[tngmodel] = dungeon->door_amount_stored[tngmodel]+dungeon->door_amount_offmap[tngmodel];
        }
        if (dungeon->door_amount_placeable[tngmodel] < dungeon->door_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s doors amount for player %d was too small; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->door_amount_placeable[tngmodel] = dungeon->door_amount_offmap[tngmodel];
        }
        break;
    default:
        ERRORLOG("%s: Can't add item; illegal item class %d",func_name,(int)tngclass);
        return false;
    }
    return true;
}

/**
 * Re-adds item to the amount available to be placed on map, if an empty trap gets destroyed.
 * @param owner
 * @param tngclass
 * @param tngmodel
 * @return
 * @note was named add_workshop_item()
 */
TbBool readd_workshop_item_to_amount_placeable_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, const char *func_name)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't add item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return false;
    }
    switch (tngclass)
    {
    case TCls_Trap:
        SYNCDBG(8,"%s: Adding Trap %s",func_name,trap_code_name(tngmodel));
        dungeon->trap_amount_placeable[tngmodel]++;
        if (dungeon->trap_amount_placeable[tngmodel] > dungeon->trap_amount_stored[tngmodel]+dungeon->trap_amount_offmap[tngmodel]) {
            SYNCLOG("%s: Placeable %s traps amount for player %d was too large; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->trap_amount_placeable[tngmodel] = dungeon->trap_amount_stored[tngmodel]+dungeon->trap_amount_offmap[tngmodel];
        }
        if (dungeon->trap_amount_placeable[tngmodel] < dungeon->trap_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s traps amount for player %d was too small; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->trap_amount_placeable[tngmodel] = dungeon->trap_amount_offmap[tngmodel];
        }
        break;
    case TCls_Door:
        SYNCDBG(8,"%s: Adding Door %s",func_name,door_code_name(tngmodel));
        dungeon->door_amount_placeable[tngmodel]++;
        // In case the placeable amount lost it, do a fix
        if (dungeon->door_amount_placeable[tngmodel] > dungeon->door_amount_stored[tngmodel]+dungeon->door_amount_offmap[tngmodel]) {
            SYNCLOG("%s: Placeable %s doors amount for player %d was too large; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->door_amount_placeable[tngmodel] = dungeon->door_amount_stored[tngmodel]+dungeon->door_amount_offmap[tngmodel];
        }
        if (dungeon->door_amount_placeable[tngmodel] < dungeon->door_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s doors amount for player %d was too small; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->door_amount_placeable[tngmodel] = dungeon->door_amount_offmap[tngmodel];
        }
        break;
    default:
        ERRORLOG("%s: Can't add item; illegal item class %d",func_name,(int)tngclass);
        return false;
    }
    return true;
}

/**
 * Removes item from the amount of crates stored in workshops.
 * @param owner
 * @param tngclass
 * @param tngmodel
 * @return Gives 0 if no crate was found, 1 if offmap crate was used, 2 if crate from workshop was used.
 * @note was named remove_workshop_item()
 */
int remove_workshop_item_from_amount_stored_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, unsigned short flags, const char *func_name)
{
    SYNCDBG(18,"%s: Starting",func_name);
    //return _DK_remove_workshop_item(plyr_idx, tngclass, tngmodel);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't remove item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return WrkCrtS_None;
    }
    long amount;
    amount = 0;
    switch (tngclass)
    {
    case TCls_Trap:
        if ((flags & WrkCrtF_NoStored) == 0) {
            amount = dungeon->trap_amount_stored[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing stored trap %s",func_name,trap_code_name(tngmodel));
            dungeon->trap_amount_stored[tngmodel] = amount - 1;
            return WrkCrtS_Stored;
        }
        if ((flags & WrkCrtF_NoOffmap) == 0) {
            amount = dungeon->trap_amount_offmap[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing offmap trap %s",func_name,trap_code_name(tngmodel));
            dungeon->trap_amount_offmap[tngmodel] = amount - 1;
            return WrkCrtS_Offmap;
        }
        ERRORLOG("%s: Trap %s not available",func_name,trap_code_name(tngmodel));
        break;
    case TCls_Door:
        if ((flags & WrkCrtF_NoStored) == 0) {
            amount = dungeon->door_amount_stored[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing stored door %s",func_name,door_code_name(tngmodel));
            dungeon->door_amount_stored[tngmodel] = amount - 1;
            return WrkCrtS_Stored;
        }
        if ((flags & WrkCrtF_NoOffmap) == 0) {
            amount = dungeon->door_amount_offmap[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing offmap door %s",func_name,door_code_name(tngmodel));
            dungeon->door_amount_offmap[tngmodel] = amount - 1;
            return WrkCrtS_Offmap;
        }
        ERRORLOG("%s: Door %s not available",func_name,door_code_name(tngmodel));
        break;
    default:
        ERRORLOG("%s: Can't remove item; illegal item class %d",func_name,(int)tngclass);
        break;
    }
    return WrkCrtS_None;
}

/**
 * Removes item from the amount available to be placed on map.
 * @param owner
 * @param tngclass
 * @param tngmodel
 * @return
 */
TbBool remove_workshop_item_from_amount_placeable_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, const char *func_name)
{
    SYNCDBG(18,"%s: Starting",func_name);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't remove item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return false;
    }
    long amount;
    switch (tngclass)
    {
    case TCls_Trap:
        amount = dungeon->trap_amount_placeable[tngmodel];
        if (amount <= 0) {
            ERRORLOG("%s: Trap %s not available",func_name,trap_code_name(tngmodel));
            break;
        }
        SYNCDBG(8,"%s: Removing Trap %s",func_name,trap_code_name(tngmodel));
        dungeon->trap_amount_placeable[tngmodel] = amount - 1;
        dungeon->trap_build_flags[tngmodel] |= MnfBldF_Used;
        dungeon->lvstats.traps_used++;
        return true;
    case TCls_Door:
        amount = dungeon->door_amount_placeable[tngmodel];
        if (amount <= 0) {
            ERRORLOG("%s: Door %s not available",func_name,door_code_name(tngmodel));
            break;
        }
        SYNCDBG(8,"%s: Removing Door %s",func_name,door_code_name(tngmodel));
        dungeon->door_amount_placeable[tngmodel] = amount - 1;
        dungeon->door_build_flags[tngmodel] |= MnfBldF_Used;
        dungeon->lvstats.doors_used++;
        return true;
    default:
        ERRORLOG("%s: Can't remove item; illegal item class %d",func_name,(int)tngclass);
        break;
    }
    return false;
}

TbBool placing_offmap_workshop_item(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel)
{
    SYNCDBG(18,"Starting");
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        // Player with no dungeon has only off-map items
        // But this shouldn't really happen
        return true;
    }
    switch (tngclass)
    {
    case TCls_Trap:
        if (dungeon->trap_amount_stored[tngmodel] > 0) {
            return false;
        }
        if (dungeon->trap_amount_offmap[tngmodel] > 0) {
            return true;
        }
        break;
    case TCls_Door:
        if (dungeon->door_amount_stored[tngmodel] > 0) {
            return false;
        }
        if (dungeon->door_amount_offmap[tngmodel] > 0) {
            return true;
        }
        break;
    }
    return false;
}

TbBool check_workshop_item_limit_reached(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return true;
    switch (tngclass)
    {
    case TCls_Trap:
        return (dungeon->trap_amount_stored[tngmodel] >= MANUFACTURED_ITEMS_LIMIT);
    case TCls_Door:
        return (dungeon->door_amount_stored[tngmodel] >= MANUFACTURED_ITEMS_LIMIT);
    }
    return true;
}

TbBool remove_workshop_object_from_player(PlayerNumber owner, ThingModel objmodel)
{
    struct Thing *cratetng;
    struct Room *room;
    //return _DK_remove_workshop_object_from_player(a1, a2);
    cratetng = get_workshop_box_thing(owner, objmodel);
    if (thing_is_invalid(cratetng)) {
        WARNLOG("Crate %s could not be found",object_code_name(objmodel));
        return false;
    }
    room = get_room_thing_is_on(cratetng);
    if (room_exists(room)) {
        remove_workshop_object_from_workshop(room,cratetng);
    } else {
        WARNLOG("Crate thing index %d isn't placed existing room; removing anyway",(int)cratetng->index);
    }
    create_effect(&cratetng->mappos, imp_spangle_effects[cratetng->owner], cratetng->owner);
    destroy_object(cratetng);
    return true;
}

/**
 * Finds a doable manufacture which has minimal amount of items ready to use.
 * Returns that minimal amount of items, and manufacture kind related to it.
 * @param dungeon The dungeon which is to be checked.
 * @param mnfctr_class Class of the manufacture with minimal items available.
 * @param mnfctr_kind Kind of the manufacture with minimal items available.
 * @return Gives minimal amount of items available, or LONG_MAX if no doable manufacture was found.
 */
long get_doable_manufacture_with_minimal_amount_available(const struct Dungeon *dungeon, int * mnfctr_class, int * mnfctr_kind)
{
    int chosen_class,chosen_kind,chosen_amount,chosen_level;
    struct ManfctrConfig *mconf;
    int tngmodel;
    long amount;
    chosen_class = TCls_Empty;
    chosen_kind = 0;
    chosen_amount = LONG_MAX;
    chosen_level = LONG_MAX;
    // Try getting door kind for manufacture
    for (tngmodel = 1; tngmodel < DOOR_TYPES_COUNT; tngmodel++)
    {
        mconf = &game.doors_config[tngmodel];
        if (((dungeon->door_build_flags[tngmodel] & MnfBldF_Manufacturable) != 0) && (dungeon->manufacture_level >= mconf->manufct_level) )
        {
            amount = dungeon->door_amount_stored[tngmodel];
            if ( (chosen_amount > amount) ||
                ((chosen_amount == amount) && (chosen_level > mconf->manufct_level)) )
            {
                chosen_class = TCls_Door;
                chosen_amount = dungeon->door_amount_stored[tngmodel];
                chosen_kind = tngmodel;
                chosen_level = mconf->manufct_level;
            }
        }
    }
    // Try getting trap kind for manufacture
    for (tngmodel = 1; tngmodel < TRAP_TYPES_COUNT; tngmodel++)
    {
        mconf = &game.traps_config[tngmodel];
        if (((dungeon->trap_build_flags[tngmodel] & MnfBldF_Manufacturable) != 0) && (dungeon->manufacture_level >= mconf->manufct_level))
        {
            amount = dungeon->trap_amount_stored[tngmodel];
            if ( (chosen_amount > amount) ||
                ((chosen_amount == amount) && (chosen_level > mconf->manufct_level)) )
            {
                chosen_class = TCls_Trap;
                chosen_amount = dungeon->trap_amount_stored[tngmodel];
                chosen_kind = tngmodel;
                chosen_level = mconf->manufct_level;
            }
        }
    }
    if (chosen_class != TCls_Empty)
    {
        if (mnfctr_class != NULL)
            *mnfctr_class = chosen_class;
        if (mnfctr_kind != NULL)
            *mnfctr_kind = chosen_kind;
    }
    return chosen_amount;
}

TbBool get_next_manufacture(struct Dungeon *dungeon)
{
    int chosen_class,chosen_kind,chosen_amount;
    //return _DK_get_next_manufacture(dungeon);
    set_manufacture_level(dungeon);
    chosen_class = TCls_Empty;
    chosen_kind = 0;
    chosen_amount = get_doable_manufacture_with_minimal_amount_available(dungeon, &chosen_class, &chosen_kind);
    if (chosen_amount >= MANUFACTURED_ITEMS_LIMIT)
    {
        WARNDBG(6,"Player %d reached manufacture limit for all items",(int)dungeon->owner);
        return false;
    }
    if (chosen_class != TCls_Empty)
    {
        SYNCDBG(8,"Player %d manufacturing class %d kind %d",(int)dungeon->owner,(int)chosen_class,(int)chosen_kind);
        dungeon->manufacture_class = chosen_class;
        dungeon->manufacture_kind = chosen_kind;
        return true;
    }
    WARNDBG(6,"Player %d has nothing to manufacture",(int)dungeon->owner);
    return false;
}

long manufacture_points_required_f(long mfcr_type, unsigned long mfcr_kind, const char *func_name)
{
  switch (mfcr_type)
  {
  case TCls_Trap:
      return game.traps_config[mfcr_kind%TRAP_TYPES_COUNT].manufct_required;
  case TCls_Door:
      return game.doors_config[mfcr_kind%DOOR_TYPES_COUNT].manufct_required;
  default:
      ERRORMSG("%s: Invalid type of manufacture",func_name);
      return 0;
  }
}

short process_player_manufacturing(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    struct Room *room;
    int k;
    SYNCDBG(17,"Starting for player %d",(int)plyr_idx);
    //return _DK_process_player_manufacturing(plr_idx);

    dungeon = get_players_num_dungeon(plyr_idx);
    if (!player_has_room(plyr_idx, RoK_WORKSHOP))
    {
        return true;
    }
    if (dungeon->manufacture_class == TCls_Empty)
    {
        get_next_manufacture(dungeon);
        return true;
    }
    k = manufacture_points_required_f(dungeon->manufacture_class, dungeon->manufacture_kind, __func__);
    // If we don't have enough manufacture points, don't do anything
    if (dungeon->manufacture_progress < (k << 8))
        return true;
    // Try to do the manufacturing
    room = find_room_with_spare_room_item_capacity(plyr_idx, RoK_WORKSHOP);
    if (room_is_invalid(room))
    {
        dungeon->manufacture_class = TCls_Empty;
        return false;
    }
    if (check_workshop_item_limit_reached(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind))
    {
        ERRORLOG("Bad choice for manufacturing - limit reached for %s kind %d",thing_class_code_name(dungeon->manufacture_class),(int)dungeon->manufacture_kind);
        get_next_manufacture(dungeon);
        return false;
    }
    if (create_workshop_object_in_workshop_room(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind) == 0)
    {
        ERRORLOG("Could not create manufactured %s kind %d",thing_class_code_name(dungeon->manufacture_class),(int)dungeon->manufacture_kind);
        return false;
    }
    add_workshop_item_to_amounts(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind);

    switch (dungeon->manufacture_class)
    {
    case TCls_Trap:
        dungeon->lvstats.manufactured_traps++;
        // If that's local player - make a message
        if (is_my_player_number(plyr_idx))
            output_message(SMsg_ManufacturedTrap, 0, true);
        break;
    case TCls_Door:
        dungeon->lvstats.manufactured_doors++;
        // If that's local player - make a message
        if (is_my_player_number(plyr_idx))
            output_message(SMsg_ManufacturedDoor, 0, true);
        break;
    default:
        ERRORLOG("Invalid type of new manufacture, %d",(int)dungeon->manufacture_class);
        return false;
    }

    dungeon->manufacture_progress -= (k << 8);
    dungeon->field_118B = game.play_gameturn;
    dungeon->lvstats.manufactured_items++;
    get_next_manufacture(dungeon);
    return true;
}
/******************************************************************************/
