/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace_detection.h
 *     Header file for roomspace_detection.c.
 * @par Purpose:
 *     Function for finding the best "room space" based on the player's 
 *     current cursor position.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Ed Kearney
 * @date     28 Aug 2020 - 07 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOMSPACE_DETECT_H
#define DK_ROOMSPACE_DETECT_H
// This is included at the end of "roomspace.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct RoomQuery {
    short rkind_cost;
    int total_player_money;
    int mode;
    int maxRoomRadius;
    int maxRoomWidth;
    int minRoomWidth;
    int minRoomHeight;
    int subRoomCheckCount;
    int bestRoomsCount;
    struct RoomSpace best_room;
    struct RoomSpace best_corridor;
    MapSlabCoord cursor_x;
    MapSlabCoord cursor_y;
    MapSlabCoord centre_x;
    MapSlabCoord centre_y;
    PlayerNumber plyr_idx;
    RoomKind rkind;
    float minimumRatio;
    float minimumComparisonRatio;
    TbBool isCorridor;
    TbBool isCompoundRoom;
    int leniency;
    int moneyLeft;
    int InvalidBlocksIgnored;
    TbBool findCorridors;
    TbBool foundRoom;
    int roomspace_discovery_looseness;
};
/******************************************************************************/
struct RoomSpace get_biggest_roomspace(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord cursor_x, MapSlabCoord cursor_y, short rkind_cost, 
    int total_player_money, int mode, int roomspace_discovery_looseness);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
