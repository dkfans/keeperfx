/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_rooms.h
 *     Header file for skirmish_ai_rooms.c
 * @par Purpose:
 *     Room/dungeon layout routines.
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

#ifndef SKIRMISH_AI_ROOMS_H
#define SKIRMISH_AI_ROOMS_H

#include "room_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SAI_Point
{
    signed char x;
    signed char y;
};

struct SAI_Rect
{
    signed char l; //left
    signed char t; //top
    signed char r; //right
    signed char b; //bottom
};

enum SAI_RoomState
{
    SAI_ROOM_UNUSED, //no other data is valid
    SAI_ROOM_USED, //checked out
    SAI_ROOM_UNDUG, //not (intentionally) dug - room kind determines if anything is planned for room
    SAI_ROOM_EMPTY, //is dug but nothing built, room kind determines if anything planned
    SAI_ROOM_ACTIVE, //room has been built and is a part of dungeon
    SAI_ROOM_INCOMPLETE, //room is in incompleted state (AI probably didn't build it)
    SAI_ROOM_BAD, //bad room layout, SELL ASAP and move to SAI_ROOM_UNUSED
    SAI_ROOM_BEING_BUILT, //room is being built
    SAI_ROOM_BEING_DUG, //room is being dug
    SAI_ROOM_BEING_SOLD, //room has been built (in parts or what not) but is being sold
};

struct SAI_Room
{
    int id;
    enum RoomKinds kind;
    enum SAI_RoomState state;
    SAI_Rect rect;
};

enum SAI_WallBreakOptions
{
    /**
     * We don't particularly care if we crash into enemy dungeon or not - it should
     * not stop us from pathing to where we want to be.
     */
    SAI_WB_DONT_CARE = 0,

    /**
     * We want to get into enemy player's dungeon.
     */
    SAI_WB_PREFER,

    /**
     * Avoid anything that could put us in contact with enemy (including digging to
     * lava or water - if we do we must dig all around said lava or water to be able
     * to fortify).
     */
    SAI_WB_AVOID
};

/**
 * Initializes the room layout data structures based on the current map state for
 * a given player.
 * @param plyr Index of player.
 */
void SAI_init_room_layout_for_player(int plyr);

/**
 * Sets parameters for how the room layout routines should behave in presence of
 * enemy keepers.
 * @param plyr Index of player.
 * @param path_safety_dist Minimum distance to enemy when digging paths. Should be
 * less than room_safety_dist for correct behavior.
 * @param room_safety_dist Minimum distance to enemy when digging rooms or approaching
 * neutral rooms.
 * @param wall_break How to approach the possibility of walling off.
 */
void SAI_set_room_layout_safety(int plyr, int path_safety_dist,
    int room_safety_dist, enum SAI_WallBreakOptions wall_break);

/**
 * Requests that player should have a room assigned for a certain purpose.
 * Will either return a dug (but empty) room, an undug room, or a neutral room which
 * the player is expected to dig to.
 * @param plyr Index of player.
 * @param kind Type of room.
 * @param min_size Interpreted or ignored depending on room kind.
 * @param max_size Interpreted or ignored depending on room kind.
 * @return If >= 0, the index of room. If it was not possibly to fulfill request,
 * returns -1.
 */
int SAI_request_room(int plyr, enum RoomKinds kind, int min_size, int max_size);

/**
 * Requests an access path (or possibly more depending on room kind) that can be dug
 * in order to access a room.
 * @param plyr Index of player.
 * @param id Index of room we want to dig to.
 * @param coords Output coordinate array. Last element will have negative coords.
 * @return Non-zero if successful and coordinate array contains at least one element.
 */
int SAI_request_path_to_room(int plyr, int id, struct SAI_Point ** coords);

/**
 * Requests an access path to a gold area.
 * @param plyr Index of player.
 * @param gold_area_idx Index of targeted gold area.
 * @param coords Output coordinate array. Last element wil have negative coords.
 * @return Non-zero if successful and coordinate array contains at least one element.
 */
int SAI_request_path_to_gold_area(int plyr, int gold_area_idx,
    struct SAI_Point ** coords);

/**
 * Reuqest an access path to a certain position.
 * @param plyr Index of player.
 * @param pos Targeted position.
 * @param coords Output coordinate array. Last element wil have negative coords.
 * @return Non-zero if successful and coordinate array contains at least one element.
 */
int SAI_request_path_to_position(int plyr, SAI_Point pos,
    struct SAI_Point ** coords);

/**
 * Gets an AI room. Read-only by purpose, state transition should be handled by
 * calling appropriate functions which can error check states.
 * @param plyr Index of player.
 * @param id Identifier of used room.
 * @return A pointer to the room if the id was valid, otherwise NULL.
 */
const struct SAI_Room * SAI_get_room(int plyr, int id);

/**
 * Sets the kind of a room. It is assert()ed if in a valid state.
 * @param plyr Index of player.
 * @param room_id Index of room.
 * @param kind Kind of room to set to.
 */
void SAI_set_room_kind(int plyr, int room_id, enum RoomKinds kind);


#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_ROOMS_H
