/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_side_score.h
 *     A room's efficiency from what surrounds its slabs, as pure functions.
 * @par Purpose:
 *     The rule calculate_room_efficiency() applies, on neighbours already
 *     classified: the engine classifies the real map (room_neighbour_kind(),
 *     slab_data.c), Daimon's layout planner a predicted one, and both score
 *     them here, so a predicted room is scored exactly as a built one.
 * @par Comment:
 *     docs/plans/ai-plans/v2/04-phase2-build-and-keep.md section 2.2.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_SIDE_SCORE_H
#define DK_ROOM_SIDE_SCORE_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** What is next to a room slab, as far as its room's efficiency goes. */
enum RoomNeighbour {
    RoNb_OffMap = 0,    /**< off the map: scores nothing, and the slab may still be inside */
    RoNb_SameRoom,      /**< the same kind of room, of the same owner */
    RoNb_Synergy,       /**< the room kind's synergy slab (the owner's, or of a kind nobody owns) */
    RoNb_Natural,       /**< rock, gold, dense gold, earth, gems */
    RoNb_OwnWall,       /**< a fortified wall of the room's owner */
    RoNb_OtherWall,     /**< someone else's fortified wall */
    RoNb_OwnDoor,       /**< a door of the room's owner */
    RoNb_Open,          /**< anything else: open floor, liquid, another room, someone else's door */
};

/** What one side scores: 2 for the room, its synergy slab, an own wall or door; 1 for natural ground or someone
 *  else's wall; 0 otherwise. `continues_room` (may be NULL) is whether that side keeps the slab inside the room. */
int64_t room_side_score_of(enum RoomNeighbour side, TbBool *continues_room);
/** One room slab's score: its four sides, and 2 more when it is inside (every side continues the room and every
 *  corner is the room or off the map; a corner of the synergy slab does not count). */
int64_t room_slab_score_from(const enum RoomNeighbour sides[4], const enum RoomNeighbour corners[4]);
/** The score a room of `nslabs` slabs reaches when fully walled in. */
int64_t room_widespread_factor(int64_t nslabs);
/** Efficiency (0..ROOM_EFFICIENCY_MAX) of a room of `nslabs` slabs whose slabs scored `score` in all. */
int64_t room_efficiency_from_score(int64_t nslabs, int64_t score);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
