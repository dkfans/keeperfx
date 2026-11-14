/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_side_score.c
 *     A room's efficiency from what surrounds its slabs, as pure functions.
 * @par Purpose:
 *     The scoring half of calculate_room_efficiency(), shared with Daimon's
 *     layout planner (room_side_score.h).
 * @par Comment:
 *     Moved out of slab_data.c and room_data.c unchanged.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "room_side_score.h"

#include "room_data.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static unsigned char const slabs_to_centre_pieces[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  2,  2,  2,  3,  4,  4,
  4,  5,  6,  6,  6,  7,  8,  9,  9,
  9, 10, 11, 12, 12, 12, 13, 14, 15,
 16, 16, 16, 17, 18, 19, 20, 20, 20,
 21, 22, 23, 24, 25,
};

int64_t room_side_score_of(enum RoomNeighbour side, TbBool *continues_room)
{
    const TbBool continues = (side == RoNb_OffMap) || (side == RoNb_SameRoom) || (side == RoNb_Synergy);
    if (continues_room != NULL) *continues_room = continues;
    switch (side)
    {
    case RoNb_SameRoom:
    case RoNb_Synergy:
    case RoNb_OwnWall:
    case RoNb_OwnDoor:
        return 2;
    case RoNb_Natural:
    case RoNb_OtherWall:
        return 1;
    default:
        return 0;
    }
}

int64_t room_slab_score_from(const enum RoomNeighbour sides[4], const enum RoomNeighbour corners[4])
{
    TbBool is_room_inside = true;
    int64_t score = 0;
    for (int n = 0; n < 4; n++)
    {
        TbBool continues_room;
        score += room_side_score_of(sides[n], &continues_room);
        if (!continues_room)
            is_room_inside = false;
    }
    for (int n = 0; (n < 4) && is_room_inside; n++)
    {
        if ((corners[n] != RoNb_OffMap) && (corners[n] != RoNb_SameRoom))
            is_room_inside = false;
    }
    if (is_room_inside)
        score += 2;
    return score;
}

int64_t room_widespread_factor(int64_t nslabs)
{
    int64_t i = nslabs;
    if (i >= (int64_t)(sizeof(slabs_to_centre_pieces)/sizeof(slabs_to_centre_pieces[0])))
        i = sizeof(slabs_to_centre_pieces)/sizeof(slabs_to_centre_pieces[0]) - 1;
    int64_t npieces = slabs_to_centre_pieces[i];
    return 2 * (npieces + 4 * nslabs);
}

int64_t room_efficiency_from_score(int64_t nslabs, int64_t score)
{
    if (nslabs <= 0)
        return 0;
    int64_t expected_base = (nslabs == 1) ? 0 : 4 * (nslabs - 1);
    int64_t widespread = room_widespread_factor(nslabs);
    int64_t effic;
    if (score <= expected_base) {
        effic = 0;
    } else
    if (widespread <= expected_base) {
        effic = ROOM_EFFICIENCY_MAX;
    } else
    {
        effic = ((score - expected_base) << 8) / (widespread - expected_base);
    }
    if (effic > ROOM_EFFICIENCY_MAX)
        effic = ROOM_EFFICIENCY_MAX;
    return effic;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
