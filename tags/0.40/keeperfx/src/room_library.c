/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_library.c
 *     Library room maintain functions.
 * @par Purpose:
 *     Functions to create and use libraries.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 05 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_library.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_objects.h"
#include "thing_stats.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "magic.h"
#include "gui_soundmsgs.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
TbBool remove_spell_from_library(struct Room *room, struct Thing *spelltng, long new_owner)
{
    if ( (room->kind != RoK_LIBRARY) || (spelltng->owner != room->owner) ) {
        SYNCDBG(4,"Spell %s owned by player %d found in a %s owned by player %d, instead of proper library",thing_model_name(spelltng),(int)spelltng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    if (!remove_item_from_room_capacity(room))
        return false;
    remove_spell_from_player(object_to_magic[spelltng->model], room->owner);
    if (is_my_player_number(room->owner))
    {
        output_message(SMsg_SpellbookStolen, 0, true);
    } else
    if (is_my_player_number(new_owner))
    {
        output_message(SMsg_SpellbookTaken, 0, true);
    }
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
