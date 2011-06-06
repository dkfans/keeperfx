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
#include "config_terrain.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
