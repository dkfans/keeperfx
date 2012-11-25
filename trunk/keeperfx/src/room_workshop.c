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
#include "thing_stats.h"
#include "thing_effects.h"
#include "config_terrain.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_remove_workshop_item(long a1, long a2, long a3);
DLLIMPORT long _DK_remove_workshop_object_from_player(long a1, long a2);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool add_workshop_object_to_workshop(struct Room *room,struct Thing *cratetng)
{
    if (room->kind != RoK_WORKSHOP) {
        SYNCDBG(4,"Crate %s owned by player %d can't be placed in a %s owned by player %d, expected proper workshop",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return add_item_to_room_capacity(room);
}

TbBool remove_workshop_object_from_workshop(struct Room *room,struct Thing *cratetng)
{
    if ( (room->kind != RoK_WORKSHOP) || (cratetng->owner != room->owner) ) {
        SYNCDBG(4,"Crate %s owned by player %d found in a %s owned by player %d, instead of proper workshop",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return remove_item_from_room_capacity(room);
}

TbBool add_workshop_item(long plyr_idx, long wrkitm_class, long wrkitm_kind)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Can't add item; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    switch (wrkitm_class)
    {
    case TCls_Trap:
        dungeon->trap_amount[wrkitm_kind]++;
        dungeon->trap_placeable[wrkitm_kind] = true;
        break;
    case TCls_Door:
        dungeon->door_amount[wrkitm_kind]++;
        dungeon->door_placeable[wrkitm_kind] = true;
        break;
    default:
        ERRORLOG("Can't add item; illegal item class %d",(int)wrkitm_class);
        return false;
    }
    return true;
}

TbBool check_workshop_item_limit_reached(long plyr_idx, long wrkitm_class, long wrkitm_kind)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return true;
    switch (wrkitm_class)
    {
    case TCls_Trap:
        return (dungeon->trap_amount[wrkitm_kind] >= MANUFACTURED_ITEMS_LIMIT);
    case TCls_Door:
        return (dungeon->door_amount[wrkitm_kind] >= MANUFACTURED_ITEMS_LIMIT);
    }
    return true;
}

struct Thing *get_workshop_box_thing(long owner, long model)
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
        if ( ((thing->alloc_flags & TAlF_Exists) != 0) && (thing->model == model) && (thing->owner == owner) )
        {
            if ( ((thing->alloc_flags & TAlF_IsInLimbo) == 0) && ((thing->field_1 & TF1_InCtrldLimbo) == 0) )
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

long remove_workshop_item(long owner, long tngclass, long tngmodel)
{
    SYNCDBG(8,"Starting");
    return _DK_remove_workshop_item(owner, tngclass, tngmodel);
}

long remove_workshop_object_from_player(PlayerNumber owner, long model)
{
    struct Thing *thing;
    struct Room *room;
    //return _DK_remove_workshop_object_from_player(a1, a2);
    thing = get_workshop_box_thing(owner, model);
    if (thing_is_invalid(thing))
        return 0;
    room = get_room_thing_is_on(thing);
    if ( room_exists(room) ) {
        remove_workshop_object_from_workshop(room,thing);
    } else {
        WARNLOG("Crate thing index %d isn't placed existing room; removing anyway",(int)thing->index);
    }
    create_effect(&thing->mappos, imp_spangle_effects[thing->owner], thing->owner);
    delete_thing_structure(thing, 0);
    return 1;
}
/******************************************************************************/
