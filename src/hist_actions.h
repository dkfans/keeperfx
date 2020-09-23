/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file hist_actions.h
 *     Accounting of game events for improved multiplayer
 * @par Purpose:
 *     Allows to sync present with events in past in multiplayer to compensate lag.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   TheSim
 * @date     17 Aug 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_HIST_ACTIONS_H
#define DK_HIST_ACTIONS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

enum HistActionType
{
    HAT_None = 0,
    HAT_Tag,
    HAT_Untag,
    HAT_TaskTaken, // Imp started a task

    HAT_Power,
    HAT_Special,
    HAT_PlaceRoom,
    HAT_PlaceDoor,
    HAT_PlaceTrap,
};

struct HistActionRecord
{
    enum HistActionType type;
    long              gameturn;
    union
    {
        PowerKind     power;
        SpecialKind   special;
    };
    SubtlCodedCoords  stl_num;

    int               task_id;
    int               creature_idx;
};
/*
    This function record actions to history buffer
*/
void hist_map_action(enum HistActionType type, PlayerNumber plyr_idx, MapSubtlCoord cx, MapSubtlCoord cy);
void hist_take_task(PlayerNumber plyr_idx, int task_idx, int creature_idx);

void hist_get_string(int order, char *str_left, char *str_right);
#ifdef __cplusplus
}
#endif

#endif