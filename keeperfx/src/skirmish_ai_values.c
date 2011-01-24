/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_values.c
 *     Skirmish AI "magic values" that determine how the AI plays.
 * @par Purpose:
 *     The values are either encoded as constants or are initialized from the
 *     configuration data loaded by other parts of game.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "skirmish_ai_values.h"

static const int room_tiles_for_usable_room[ROOM_TYPES_COUNT + 1] = //unsure about last one
{

    0, 0, 16, 25, 16, 12, 25, 0, 25, 16, 9, 16, 9, 25, 25, 1, 1, 0
};

int SAI_tiles_for_next_room_of_kind(int plyr, enum RoomKinds kind)
{
    //hardcoded for now - get better approximation later
    return room_tiles_for_usable_room[kind];
}
