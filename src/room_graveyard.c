/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_graveyard.c
 *     Graveyard room maintain functions.
 * @par Purpose:
 *     Functions to create and use graveyards.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Feb 2012 - 01 Jul 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_graveyard.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_corpses.h"
#include "thing_data.h"
#include "thing_stats.h"
#include "config_terrain.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
/**
 * Adds a corpse to the graveyard room capacity.
 * @param corpse The dead creature thing to be added.
 * @param room The graveyard room.
 * @return Gives true on success.
 */
TbBool add_body_to_graveyard(struct Thing *deadtng, struct Room *room)
{
    if (room->total_capacity <= room->used_capacity)
    {
        ERRORLOG("The %s has no space for another corpse",room_code_name(room->kind));
        return false;
    }
    if (corpse_laid_to_rest(deadtng))
    {
        ERRORLOG("The %s is already decomposing in %s",thing_model_name(deadtng),room_code_name(RoK_GRAVEYARD));
        return false;
    }
    room->used_capacity++;
    deadtng->corpse.laid_to_rest = 1;
    deadtng->health = game.graveyard_convert_time;
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
