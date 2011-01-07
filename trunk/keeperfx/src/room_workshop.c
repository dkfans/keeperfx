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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
TbBool add_workshop_object_to_workshop(struct Room *room)
{
    if (room->used_capacity >= room->total_capacity)
    {
        return false;
    }
    room->used_capacity++;
    room->long_17++;
    return true;
}

TbBool remove_workshop_object_from_workshop(struct Room *room)
{
    if ( (room->used_capacity <= 0) || (room->long_17 <= 0) )
    {
        ERRORLOG("Invalid workshop content");
        return false;
    }
    room->used_capacity--;
    room->long_17--;
    return true;
}

TbBool add_workshop_item(long plyr_idx, long wrkitm_class, long wrkitm_kind)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    switch (wrkitm_class)
    {
    case 8:
        dungeon->trap_amount[wrkitm_kind]++;
        dungeon->trap_placeable[wrkitm_kind] = 1;
        break;
    case 9:
        dungeon->door_amount[wrkitm_kind]++;
        dungeon->door_placeable[wrkitm_kind] = 1;
        break;
    default:
        ERRORLOG("Illegal item class");
        return false;
    }
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
