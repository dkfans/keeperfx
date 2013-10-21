/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_values.h
 *     Header file for skirmish_ai_values.c
 * @par Purpose:
 *     Skirmish AI "magic values" that determine how the AI plays.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef SKIRMISH_AI_VALUES_H
#define SKIRMISH_AI_VALUES_H

#include "room_data.h"

#ifdef __cplusplus
extern "C" {
#endif

int SAI_tiles_for_next_room_of_kind(int plyr, enum RoomKinds kind);

#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_VALUES_H
