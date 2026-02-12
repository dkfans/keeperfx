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
#include "pre_inc.h"
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
#include "config_effects.h"
#include "power_hand.h"
#include "game_legacy.h"
#include "gui_soundmsgs.h"
#include "player_instances.h"
#include "creature_states.h"
#include "keeperfx.hpp"
#include "post_inc.h"

/******************************************************************************/
TbBool add_workshop_object_to_workshop(struct Room *room,struct Thing *cratetng)
{
    if (!room_role_matches(room->kind, RoRoF_CratesStorage)) {
        SYNCDBG(4,"Crate %s owned by player %d cannot be placed in a %s owned by player %d, wrong room",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return add_item_to_room_capacity(room, true);
}

struct Thing *create_crate_in_workshop(struct Room *room, ThingModel cratngmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (!room_role_matches(room->kind, RoRoF_CratesStorage)) {
        SYNCDBG(4,"Crate %s cannot be created in a %s owned by player %d, wrong room",object_code_name(cratngmodel),room_code_name(room->kind),(int)room->owner);
        return INVALID_THING;
    }
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    struct Thing* cratetng = create_object(&pos, cratngmodel, room->owner, -1);
    if (thing_is_invalid(cratetng))
    {
        ERRORLOG("Failed to create %s in %s index %d", thing_model_name(cratetng), room_code_name(room->kind), (int)room->index);
        return INVALID_THING;
    }
    // Neutral thing do not need any more processing
    if (is_neutral_thing(cratetng) || !player_exists(get_player(room->owner))) {
        return cratetng;
    }
    if (!add_workshop_object_to_workshop(room, cratetng)) {
        ERRORLOG("Could not fit %s in %s index %d", thing_model_name(cratetng),room_code_name(room->kind),(int)room->index);
        //remove_item_from_room_capacity(room); -- no need, it was not added
        destroy_object(cratetng);
        return INVALID_THING;
    }
    ThingClass tngclass = crate_thing_to_workshop_item_class(cratetng);
    ThingModel tngmodel = crate_thing_to_workshop_item_model(cratetng);
    add_workshop_item_to_amounts(cratetng->owner, tngclass, tngmodel);
    return cratetng;
}

TbBool create_workshop_object_in_workshop_room(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel)
{
    struct Thing *cratetng;
    SYNCDBG(7,"Making player %d new %s",(int)plyr_idx,thing_class_code_name(tngclass));
    struct Coord3d pos;
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
    struct Room* room = find_random_room_of_role_for_thing_with_spare_room_item_capacity(cratetng, plyr_idx, RoRoF_CratesStorage, 0);
    if (room_is_invalid(room))
    {
        ERRORLOG("No %s room found which would accept %s crate",room_role_code_name(RoRoF_CratesStorage),thing_class_code_name(tngclass));
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
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    switch (tngclass)
    {
    case TCls_Trap:
        if ((dungeon->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Built) == 0) {
            event_create_event(cratetng->mappos.x.val, cratetng->mappos.y.val, EvKind_NewTrap, plyr_idx, tngmodel);
        }
        break;
    case TCls_Door:
        if ((dungeon->mnfct_info.door_build_flags[tngmodel] & MnfBldF_Built) == 0) {
          event_create_event(cratetng->mappos.x.val, cratetng->mappos.y.val, EvKind_NewDoor, plyr_idx, tngmodel);
        }
        break;
    default:
        break;
    }
    create_effect(&pos, TngEff_RoomSparkeLarge, cratetng->owner);
    thing_play_sample(cratetng, 89, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
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
    if (!room_role_matches(room->kind, RoRoF_CratesStorage) || (cratetng->owner != room->owner)) {
        SYNCDBG(4,"Crate %s owned by player %d found in a %s owned by player %d, instead of proper storage room",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return remove_item_from_room_capacity(room);
}

long calculate_manufacture_level(struct Dungeon* dungeon)
{
    int mnfctr_slabs = get_room_of_role_slabs_count(dungeon->owner, RoRoF_CratesManufctr);
    int level = 0;
    while (mnfctr_slabs > (level + 3) * (level + 3))
    {
        level++;
    }
    return level;
}

void set_manufacture_level(struct Dungeon *dungeon)
{
    dungeon->manufacture_level = calculate_manufacture_level(dungeon);
    SYNCDBG(19, "Dungeon %d manufacture level set to %d",dungeon->owner, dungeon->manufacture_level);
}

struct Thing *get_workshop_box_thing(PlayerNumber owner, ThingModel objmodel)
{
    int k = 0;
    int i = game.thing_lists[TngList_Objects].index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ( ((thing->alloc_flags & TAlF_Exists) != 0) && (thing->model == objmodel) && (thing->owner == owner) )
        {
            struct Room* room = get_room_thing_is_on(thing);
            if (!thing_is_picked_up(thing) && room_role_matches(room->kind, RoRoF_CratesStorage) && (room->owner == owner))
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
    struct Dungeon* dungeon = get_players_num_dungeon_f(plyr_idx, func_name);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't add item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return false;
    }
    switch (tngclass)
    {
    case TCls_Trap:
        SYNCDBG(8,"%s: Adding Trap %s",func_name,trap_code_name(tngmodel));
        dungeon->mnfct_info.trap_amount_stored[tngmodel]++;
        dungeon->mnfct_info.trap_amount_placeable[tngmodel]++;
        dungeon->mnfct_info.trap_build_flags[tngmodel] |= MnfBldF_Built;
        // In case the placeable amount lost it, do a fix
        if (dungeon->mnfct_info.trap_amount_placeable[tngmodel]
            > dungeon->mnfct_info.trap_amount_stored[tngmodel]+dungeon->mnfct_info.trap_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s traps amount for player %d was too large; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.trap_amount_placeable[tngmodel] = dungeon->mnfct_info.trap_amount_stored[tngmodel]
                + dungeon->mnfct_info.trap_amount_offmap[tngmodel];
        }
        if (dungeon->mnfct_info.trap_amount_placeable[tngmodel] < dungeon->mnfct_info.trap_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s traps amount for player %d was too small; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.trap_amount_placeable[tngmodel] = dungeon->mnfct_info.trap_amount_offmap[tngmodel];
        }
        break;
    case TCls_Door:
        SYNCDBG(8,"%s: Adding Door %s",func_name,door_code_name(tngmodel));
        dungeon->mnfct_info.door_amount_stored[tngmodel]++;
        dungeon->mnfct_info.door_amount_placeable[tngmodel]++;
        dungeon->mnfct_info.door_build_flags[tngmodel] |= MnfBldF_Built;
        // In case the placeable amount lost it, do a fix
        if (dungeon->mnfct_info.door_amount_placeable[tngmodel]
                > dungeon->mnfct_info.door_amount_stored[tngmodel] + dungeon->mnfct_info.door_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s doors amount for player %d was too large; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.door_amount_placeable[tngmodel] = dungeon->mnfct_info.door_amount_stored[tngmodel]
                    + dungeon->mnfct_info.door_amount_offmap[tngmodel];
        }
        if (dungeon->mnfct_info.door_amount_placeable[tngmodel]
                < dungeon->mnfct_info.door_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s doors amount for player %d was too small; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.door_amount_placeable[tngmodel] = dungeon->mnfct_info.door_amount_offmap[tngmodel];
        }
        break;
    default:
        ERRORLOG("%s: Can't add item; illegal item class %d (%s)",func_name,(int)tngclass, thing_class_code_name(tngclass));
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
    struct Dungeon* dungeon = get_players_num_dungeon_f(plyr_idx, func_name);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't add item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return false;
    }
    switch (tngclass)
    {
    case TCls_Trap:
        SYNCDBG(8,"%s: Adding Trap %s",func_name,trap_code_name(tngmodel));
        dungeon->mnfct_info.trap_amount_placeable[tngmodel]++;
        if (dungeon->mnfct_info.trap_amount_placeable[tngmodel] > dungeon->mnfct_info.trap_amount_stored[tngmodel]+dungeon->mnfct_info.trap_amount_offmap[tngmodel]) {
            SYNCLOG("%s: Placeable %s traps amount for player %d was too large; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.trap_amount_placeable[tngmodel] = dungeon->mnfct_info.trap_amount_stored[tngmodel]+dungeon->mnfct_info.trap_amount_offmap[tngmodel];
        }
        if (dungeon->mnfct_info.trap_amount_placeable[tngmodel] < dungeon->mnfct_info.trap_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s traps amount for player %d was too small; fixed",func_name,trap_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.trap_amount_placeable[tngmodel] = dungeon->mnfct_info.trap_amount_offmap[tngmodel];
        }
        break;
    case TCls_Door:
        SYNCDBG(8,"%s: Adding Door %s",func_name,door_code_name(tngmodel));
        dungeon->mnfct_info.door_amount_placeable[tngmodel]++;
        // In case the placeable amount lost it, do a fix
        if (dungeon->mnfct_info.door_amount_placeable[tngmodel] > dungeon->mnfct_info.door_amount_stored[tngmodel]+dungeon->mnfct_info.door_amount_offmap[tngmodel]) {
            SYNCLOG("%s: Placeable %s doors amount for player %d was too large; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.door_amount_placeable[tngmodel] = dungeon->mnfct_info.door_amount_stored[tngmodel]+dungeon->mnfct_info.door_amount_offmap[tngmodel];
        }
        if (dungeon->mnfct_info.door_amount_placeable[tngmodel] < dungeon->mnfct_info.door_amount_offmap[tngmodel]) {
            WARNLOG("%s: Placeable %s doors amount for player %d was too small; fixed",func_name,door_code_name(tngmodel),(int)plyr_idx);
            dungeon->mnfct_info.door_amount_placeable[tngmodel] = dungeon->mnfct_info.door_amount_offmap[tngmodel];
        }
        break;
    default:
        ERRORLOG("%s: Can't add item; illegal item class %d (%s)",func_name,(int)tngclass, thing_class_code_name(tngclass));
        return false;
    }
    return true;
}

/**
 * Removes item from the amount of crates stored in workshops.
 * @param owner
 * @param tngclass
 * @param tngmodel
 * @return Gives WrkCrtS_None if no crate was found, WrkCrtS_Offmap if offmap crate was used, WrkCrtS_Stored if crate from workshop was used.
 * @note was named remove_workshop_item()
 */
int remove_workshop_item_from_amount_stored_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, unsigned short flags, const char *func_name)
{
    SYNCDBG(18,"%s: Starting",func_name);
    struct Dungeon* dungeon = get_players_num_dungeon_f(plyr_idx, func_name);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't remove item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return WrkCrtS_None;
    }
    long amount = 0;
    switch (tngclass)
    {
    case TCls_Trap:
        if ((flags & WrkCrtF_NoStored) == 0) {
            amount = dungeon->mnfct_info.trap_amount_stored[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing stored trap %s",func_name,trap_code_name(tngmodel));
            dungeon->mnfct_info.trap_amount_stored[tngmodel] = amount - 1;
            return WrkCrtS_Stored;
        }
        if ((flags & WrkCrtF_NoOffmap) == 0) {
            amount = dungeon->mnfct_info.trap_amount_offmap[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing offmap trap %s",func_name,trap_code_name(tngmodel));
            dungeon->mnfct_info.trap_amount_offmap[tngmodel] = amount - 1;
            return WrkCrtS_Offmap;
        }
        ERRORLOG("%s: Trap %s not available",func_name,trap_code_name(tngmodel));
        break;
    case TCls_Door:
        if ((flags & WrkCrtF_NoStored) == 0) {
            amount = dungeon->mnfct_info.door_amount_stored[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing stored door %s",func_name,door_code_name(tngmodel));
            dungeon->mnfct_info.door_amount_stored[tngmodel] = amount - 1;
            return WrkCrtS_Stored;
        }
        if ((flags & WrkCrtF_NoOffmap) == 0) {
            amount = dungeon->mnfct_info.door_amount_offmap[tngmodel];
        }
        if (amount > 0) {
            SYNCDBG(8,"%s: Removing offmap door %s",func_name,door_code_name(tngmodel));
            dungeon->mnfct_info.door_amount_offmap[tngmodel] = amount - 1;
            return WrkCrtS_Offmap;
        }
        ERRORLOG("%s: Door %s not available",func_name,door_code_name(tngmodel));
        break;
    default:
        ERRORLOG("%s: Can't remove item; illegal item class %d (%s)",func_name,(int)tngclass, thing_class_code_name(tngclass));
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
    struct Dungeon* dungeon = get_players_num_dungeon_f(plyr_idx, func_name);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("%s: Can't remove item; player %d has no dungeon.",func_name,(int)plyr_idx);
        return false;
    }
    long amount;
    switch (tngclass)
    {
    case TCls_Trap:
        amount = dungeon->mnfct_info.trap_amount_placeable[tngmodel];
        if (amount <= 0) {
            ERRORLOG("%s: Trap %s not available",func_name,trap_code_name(tngmodel));
            break;
        }
        SYNCDBG(8,"%s: Removing Trap %s",func_name,trap_code_name(tngmodel));
        dungeon->mnfct_info.trap_amount_placeable[tngmodel] = amount - 1;
        dungeon->mnfct_info.trap_build_flags[tngmodel] |= MnfBldF_Used;
        dungeon->lvstats.traps_used++;
        return true;
    case TCls_Door:
        amount = dungeon->mnfct_info.door_amount_placeable[tngmodel];
        if (amount <= 0) {
            ERRORLOG("%s: Door %s not available",func_name,door_code_name(tngmodel));
            break;
        }
        SYNCDBG(8,"%s: Removing Door %s",func_name,door_code_name(tngmodel));
        dungeon->mnfct_info.door_amount_placeable[tngmodel] = amount - 1;
        dungeon->mnfct_info.door_build_flags[tngmodel] |= MnfBldF_Used;
        dungeon->lvstats.doors_used++;
        return true;
    default:
        ERRORLOG("%s: Can't remove item; illegal item class %d (%s)",func_name,(int)tngclass, thing_class_code_name(tngclass));
        break;
    }
    return false;
}

TbBool placing_offmap_workshop_item(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        // Player with no dungeon has only on-map items
        // But this shouldn't really happen
        return true;
    }
    switch (tngclass)
    {
    case TCls_Trap:
        if (dungeon->mnfct_info.trap_amount_stored[tngmodel] > 0) {
            return false;
        }
        if (dungeon->mnfct_info.trap_amount_offmap[tngmodel] > 0) {
            return true;
        }
        break;
    case TCls_Door:
        if (dungeon->mnfct_info.door_amount_stored[tngmodel] > 0) {
            return false;
        }
        if (dungeon->mnfct_info.door_amount_offmap[tngmodel] > 0) {
            return true;
        }
        break;
    }
    return false;
}

TbBool check_workshop_item_limit_reached(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return true;
    switch (tngclass)
    {
    case TCls_Trap:
        return (dungeon->mnfct_info.trap_amount_stored[tngmodel] >= MANUFACTURED_ITEMS_LIMIT);
    case TCls_Door:
        return (dungeon->mnfct_info.door_amount_stored[tngmodel] >= MANUFACTURED_ITEMS_LIMIT);
    }
    return true;
}

TbBool remove_workshop_object_from_player(PlayerNumber owner, ThingModel objmodel)
{
    struct Thing* cratetng = get_workshop_box_thing(owner, objmodel);
    if (thing_is_invalid(cratetng)) {
        WARNLOG("Crate %s could not be found",object_code_name(objmodel));
        return false;
    }
    struct Room* room = get_room_thing_is_on(cratetng);
    if (room_exists(room)) {
        remove_workshop_object_from_workshop(room,cratetng);
    } else {
        WARNLOG("Crate thing index %d isn't placed existing room; removing anyway",(int)cratetng->index);
    }
    create_effect(&cratetng->mappos, imp_spangle_effects[get_player_color_idx(cratetng->owner)], cratetng->owner);
    destroy_object(cratetng);
    return true;
}

/**
 * Finds a doable manufacture which has minimal amount of items ready to use.
 * Returns that minimal amount of items, and manufacture kind related to it.
 * @param dungeon The dungeon which is to be checked.
 * @param mnfctr_class Class of the manufacture with minimal items available.
 * @param mnfctr_kind Kind of the manufacture with minimal items available.
 * @return Gives minimal amount of items available, or INT32_MAX if no doable manufacture was found.
 */
long get_doable_manufacture_with_minimal_amount_available(const struct Dungeon *dungeon, int * mnfctr_class, int * mnfctr_kind)
{
    struct DoorConfigStats *doorst;
    struct TrapConfigStats *trapst;
    int tngmodel;
    long amount;
    int chosen_class = TCls_Empty;
    int chosen_kind = 0;
    int chosen_amount = INT_MAX;
    int chosen_level = INT_MAX;

    // Try getting door kind for manufacture
    for (tngmodel = 1; tngmodel < game.conf.trapdoor_conf.door_types_count; tngmodel++)
    {
        doorst = get_door_model_stats(tngmodel);
        if (((dungeon->mnfct_info.door_build_flags[tngmodel] & MnfBldF_Manufacturable) != 0) && (dungeon->manufacture_level >= doorst->manufct_level))
        {
            amount = dungeon->mnfct_info.door_amount_stored[tngmodel];
            if ( (chosen_amount > amount) ||
                ((chosen_amount == amount) && (chosen_level > doorst->manufct_level)) )
            {
                chosen_class = TCls_Door;
                chosen_amount = dungeon->mnfct_info.door_amount_stored[tngmodel];
                chosen_kind = tngmodel;
                chosen_level = doorst->manufct_level;
            }
        }
    }
    // Try getting trap kind for manufacture
    for (tngmodel = 1; tngmodel < game.conf.trapdoor_conf.trap_types_count; tngmodel++)
    {
        trapst = get_trap_model_stats(tngmodel);
        if (((dungeon->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Manufacturable) != 0) && (dungeon->manufacture_level >= trapst->manufct_level))
        {
            amount = dungeon->mnfct_info.trap_amount_stored[tngmodel];
            if ( (chosen_amount > amount) ||
                ((chosen_amount == amount) && (chosen_level > trapst->manufct_level)) )
            {
                chosen_class = TCls_Trap;
                chosen_amount = dungeon->mnfct_info.trap_amount_stored[tngmodel];
                chosen_kind = tngmodel;
                chosen_level = trapst->manufct_level;
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
    set_manufacture_level(dungeon);
    int chosen_class = TCls_Empty;
    int chosen_kind = 0;
    int chosen_amount = get_doable_manufacture_with_minimal_amount_available(dungeon, &chosen_class, &chosen_kind);
    if (chosen_amount >= MANUFACTURED_ITEMS_LIMIT)
    {
        if (chosen_amount == INT_MAX) {
            WARNDBG(7,"Player %d has %s but no doable manufacture",(int)dungeon->owner,room_role_code_name(RoRoF_CratesStorage));
        } else {
            WARNDBG(6,"Player %d reached manufacture limit for all items",(int)dungeon->owner);
        }
        return false;
    }
    if (chosen_class != TCls_Empty)
    {
        SYNCDBG(8,"Player %d manufacturing class %d kind %d (%s)",(int)dungeon->owner,(int)chosen_class,(int)chosen_kind, thing_class_and_model_name(chosen_class, chosen_kind));
        dungeon->manufacture_class = chosen_class;
        dungeon->manufacture_kind = chosen_kind;
        return true;
    }
    WARNDBG(6,"Player %d has nothing to manufacture",(int)dungeon->owner);
    return false;
}

long manufacture_points_required_f(long mfcr_type, unsigned long mfcr_kind, const char *func_name)
{
    const struct DoorConfigStats *doorst;
    const struct TrapConfigStats *trapst;
    switch (mfcr_type)
    {
    case TCls_Trap:
        trapst = get_trap_model_stats(mfcr_kind%game.conf.trapdoor_conf.trap_types_count);
        return trapst->manufct_required;
    case TCls_Door:
        doorst = get_door_model_stats(mfcr_kind%game.conf.trapdoor_conf.door_types_count);
        return doorst->manufct_required;
    default:
        ERRORMSG("%s: Invalid type of manufacture: %d",func_name,(int)mfcr_type);
        return 0;
    }
}

short process_player_manufacturing(PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting for player %d",(int)plyr_idx);

    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (!player_has_room_of_role(plyr_idx, RoRoF_CratesManufctr))
    {
        return true;
    }
    if (dungeon->manufacture_class == TCls_Empty)
    {
        if (get_next_manufacture(dungeon))
        {
            return true;
        }
        return false;
    }
    int k = manufacture_points_required(dungeon->manufacture_class, dungeon->manufacture_kind);
    // If we don't have enough manufacture points, don't do anything
    if (dungeon->manufacture_progress < (k << 8))
        return true;
    // Try to do the manufacturing
    struct Room* room = find_room_of_role_with_spare_room_item_capacity(plyr_idx, RoRoF_CratesStorage);
    if (room_is_invalid(room))
    {
        dungeon->manufacture_class = TCls_Empty;
        return false;
    }
    if (check_workshop_item_limit_reached(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind))
    {
        ERRORLOG("Bad choice for manufacturing - limit reached for %s",thing_class_and_model_name(dungeon->manufacture_class, dungeon->manufacture_kind));
        get_next_manufacture(dungeon);
        return false;
    }
    if (create_workshop_object_in_workshop_room(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind) == 0)
    {
        ERRORLOG("Could not create manufactured %s",thing_class_and_model_name(dungeon->manufacture_class, dungeon->manufacture_kind));
        return false;
    }
    add_workshop_item_to_amounts(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind);

    switch (dungeon->manufacture_class)
    {
    case TCls_Trap:
        dungeon->lvstats.manufactured_traps++;
        // If that's local player - make a message
        if (is_my_player_number(plyr_idx))
            output_message(SMsg_ManufacturedTrap, 0);
        break;
    case TCls_Door:
        dungeon->lvstats.manufactured_doors++;
        // If that's local player - make a message
        if (is_my_player_number(plyr_idx))
            output_message(SMsg_ManufacturedDoor, 0);
        break;
    default:
        ERRORLOG("Invalid type of new manufacture: %d (%s)",(int)dungeon->manufacture_class, thing_class_code_name(dungeon->manufacture_class));
        return false;
    }

    dungeon->manufacture_progress -= (k << 8);
    dungeon->turn_last_manufacture = game.play_gameturn;
    dungeon->lvstats.manufactured_items++;
    if (get_next_manufacture(dungeon))
    {
        return true;
    }
    dungeon->manufacture_class = TCls_Empty;
    return false;
}

EventIndex update_workshop_object_pickup_event(struct Thing *creatng, struct Thing *picktng)
{
    EventIndex evidx;
    struct PlayerInfo* player;
    ThingClass tngclass = crate_thing_to_workshop_item_class(picktng);
    if (tngclass == TCls_Trap)
    {
        evidx = event_create_event_or_update_nearby_existing_event(
            picktng->mappos.x.val, picktng->mappos.y.val,
            EvKind_TrapCrateFound, creatng->owner, picktng->index);
            if ( (is_my_player_number(picktng->owner)) && (!is_my_player_number(creatng->owner)) )
            {
                output_message(SMsg_TrapStolen, 0);
            }
            else if ( (is_my_player_number(creatng->owner)) && (!is_my_player_number(picktng->owner)) )
            {
                if (picktng->owner != game.neutral_player_num)
                {
                    player = get_my_player();
                    if (creatng->index != player->influenced_thing_idx)
                    {
                        output_message(SMsg_TrapTaken, 0);
                    }
                }
            }
    } else if (tngclass == TCls_Door)
    {
       evidx = event_create_event_or_update_nearby_existing_event(
            picktng->mappos.x.val, picktng->mappos.y.val,
            EvKind_DoorCrateFound, creatng->owner, picktng->index);
            if ( (is_my_player_number(picktng->owner)) && (!is_my_player_number(creatng->owner)) )
            {
                output_message(SMsg_DoorStolen, 0);
            }
            else if ( (is_my_player_number(creatng->owner)) && (!is_my_player_number(picktng->owner)) )
            {
                if (picktng->owner != game.neutral_player_num)
                {
                    player = get_my_player();
                    if (creatng->index != player->influenced_thing_idx)
                    {
                        output_message(SMsg_DoorTaken, 0);
                    }
                }
            }
    } else
    {
        WARNLOG("Strange pickup (%s) - no event",thing_class_and_model_name(picktng->class_id, picktng->model));
        evidx = 0;
    }
    return evidx;
}

TbBool recreate_repositioned_crate_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    if ((rrepos->used < 0) || (room->used_capacity >= room->total_capacity)) {
        return false;
    }
    for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] != 0)
        {
            struct Thing* objtng = create_crate_in_workshop(room, rrepos->models[ri], stl_x, stl_y);
            if (!thing_is_invalid(objtng))
            {
                rrepos->used--;
                rrepos->models[ri] = 0;
                return true;
            }
        }
    }
    return false;
}

int check_crates_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return -2; // do nothing
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    if ((roomst->storage_height >= 0) && (get_map_floor_filled_subtiles(mapblk) != roomst->storage_height)) {
        return -1; // re-create all
    }
    int matching_things_at_subtile = 0;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if (thing_is_workshop_crate(thing) && !thing_is_dragged_or_pulled(thing) && (thing->owner == room->owner))
        {
            // If exceeded capacity of the library
            if (room->used_capacity >= room->total_capacity)
            {
                WARNLOG("The %s capacity %d exceeded; space used is %d",room_code_name(room->kind),(int)room->total_capacity,(int)room->used_capacity);
                return -1; // re-create all (this could save the object if there are duplicates)
            } else
            // If the thing is in wall, remove it but store to re-create later
            if (thing_in_wall_at(thing, &thing->mappos))
            {
                if (position_over_floor_level(thing, &thing->mappos)) //If it's inside the floors, simply move it up and count it.
                {
                    matching_things_at_subtile++;
                }
                else
                {
                    return -1; // If it's inside the wall or cannot be moved up, recreate all items.
                }
            } else
            {
                matching_things_at_subtile++;
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return matching_things_at_subtile; // Increase used capacity
}

void reposition_all_crates_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if (thing_is_workshop_crate(thing) && !thing_is_dragged_or_pulled(thing) && (thing->owner == room->owner))
        {
            ThingModel objkind = thing->model;
            ThingClass tngclass = crate_thing_to_workshop_item_class(thing);
            ThingModel tngmodel = crate_thing_to_workshop_item_model(thing);
            if (!store_reposition_entry(rrepos, objkind)) {
                WARNLOG("Too many things to reposition in %s index %d",room_code_name(room->kind),(int)room->index);
            }
            if (!is_neutral_thing(thing) && player_exists(get_player(thing->owner)))
            {
                if (remove_workshop_item_from_amount_stored(thing->owner, tngclass, tngmodel, WrkCrtF_NoOffmap) > WrkCrtS_None) {
                    remove_workshop_item_from_amount_placeable(thing->owner, tngclass, tngmodel);
                }
            }
            delete_thing_structure(thing, 0);
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
}

void count_and_reposition_crates_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    int matching_things_at_subtile = check_crates_on_subtile_for_reposition_in_room(room, stl_x, stl_y);
    if (matching_things_at_subtile > 0) {
        // This subtile contains matching things
        SYNCDBG(19,"Got %d matching things at (%d,%d)",(int)matching_things_at_subtile,(int)stl_x,(int)stl_y);
        room->used_capacity += matching_things_at_subtile;
    } else
    {
        switch (matching_things_at_subtile)
        {
        case -2:
            // No matching things, but also cannot recreate anything on this subtile
            break;
        case -1:
            // All matching things are to be removed from the subtile and stored for re-creation
            reposition_all_crates_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        case 0:
            // There are no matching things there, something can be re-created
            recreate_repositioned_crate_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        default:
            WARNLOG("Invalid value returned by reposition check");
            break;
        }
    }
}

/**
 * Updates count of crates (used capacity) in a workshop.
 * Also repositions crates which are in solid columns.
 * @param room The room to be recomputed and repositioned.
 */
void count_crates_in_room(struct Room *room)
{
    SYNCDBG(17,"Starting for %s",room_code_name(room->kind));
    struct RoomReposition rrepos;
    init_reposition_struct(&rrepos);
    // Making two loops guarantees that no rrepos things will be lost
    for (long n = 0; n < 2; n++)
    {
        // The correct count should be taken from last sweep
        room->used_capacity = 0;
        room->capacity_used_for_storage = 0;
        unsigned long k = 0;
        unsigned long i = room->slabs_list;
        while (i > 0)
        {
            MapSubtlCoord slb_x = slb_num_decode_x(i);
            MapSubtlCoord slb_y = slb_num_decode_y(i);
            // Per-slab code
            for (long dy = 0; dy < STL_PER_SLB; dy++)
            {
                for (long dx = 0; dx < STL_PER_SLB; dx++)
                {
                    count_and_reposition_crates_in_room_on_subtile(room, STL_PER_SLB*slb_x+dx, STL_PER_SLB*slb_y+dy, &rrepos);
                }
            }
            // Per-slab code ends
            i = get_next_slab_number_in_room(i);
            k++;
            if (k > room->slabs_count)
            {
                ERRORLOG("Infinite loop detected when sweeping room slabs");
                break;
            }
        }
    }
    if (rrepos.used > 0) {
        ERRORLOG("The %s index %d capacity %d wasn't enough; %d items belonging to player %d dropped",
          room_code_name(room->kind),(int)room->index,(int)room->total_capacity,(int)rrepos.used,(int)room->owner);
    }
    room->capacity_used_for_storage = room->used_capacity;
}
/******************************************************************************/
