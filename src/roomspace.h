/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace.h
 *     Header file for roomspace.c.
 * @par Purpose:
 *     Functions to facilitate the use of a "room space" (an area of many slabs)
 *     instead of a single slab when placing and selling rooms.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFx Team
 * @date     10 Jun 2020 - 07 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOMSPACE_H
#define DK_ROOMSPACE_H

// This is included at the end of "slab_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MAX_ROOMSPACE_WIDTH                        20
#define MAX_USER_ROOMSPACE_WIDTH                    9
#define MIN_USER_ROOMSPACE_WIDTH                    1
#define DEFAULT_USER_ROOMSPACE_WIDTH                5
#define DEFAULT_USER_ROOMSPACE_DETECTION_LOOSENESS  2
#define MAX_USER_ROOMSPACE_DETECTION_LOOSENESS      9

enum roomspace_placement_modes {
    box_placement_mode = 0, // fixed width/height (default)
    drag_placement_mode = 1, // fixed 1x1 roomspace, can hold left click to paint
    roomspace_detection_mode = 2, // find biggest/best roomspace under cursor
};

// 
enum roomspace_tolerance_layers {
    disable_tolerance_layers = 0,
    tolerate_only_self_claimed_path = 1,
    tolerate_same_room_type = 2,
    tolerate_other_room_types = 3,
    tolerate_gems = 4,
    tolerate_gold = 5,
    tolerate_liquid = 6,
    tolerate_rock = 7,
    tolerate_unclaimed_path = 8,
    tolerate_other_claimed_path = 9,
};

// RoomSpace describes a space or "roomspace" - i.e. a collection of slabs that are a valid
// location from the currently selected room type (when placing rooms).
// The 2D array of booleans, slab_grid[][] describes each of the slabs within
// the roomspace's extents. A value of 1 indicates a slab that is part of the roomspace,
// a value of 0 indicates a slab that is not part of the roomspace.

struct RoomSpace {
    TbBool slab_grid[MAX_ROOMSPACE_WIDTH][MAX_ROOMSPACE_WIDTH];
    int slab_count;
    TbBool is_roomspace_a_box;
    int width;
    int height;
    MapSlabCoord left;
    MapSlabCoord top;
    MapSlabCoord right;
    MapSlabCoord bottom;
    MapSlabCoord centreX;
    MapSlabCoord centreY;
    int total_roomspace_cost;
    int invalid_slabs_count;
    PlayerNumber plyr_idx;
    RoomKind rkind;
	  TbBool is_roomspace_a_single_subtile;

	  MapSlabCoord buildx, buildy;
	  TbBool is_active;
    TbBool render_roomspace_as_box;
    TbBool tag_for_dig;
    TbBool highlight_mode;
    TbBool untag_mode;
    TbBool one_click_mode_exclusive;
    MapSlabCoord drag_start_x;
    MapSlabCoord drag_start_y;
    MapSlabCoord drag_end_x;
    MapSlabCoord drag_end_y;
    TbBool drag_mode;
};
/******************************************************************************/
extern int user_defined_roomspace_width;
extern int roomspace_detection_looseness;
extern struct RoomSpace render_roomspace;
/******************************************************************************/
int calc_distance_from_roomspace_centre(int total_distance, TbBool offset);

struct RoomSpace create_box_roomspace(struct RoomSpace roomspace, int width,
int height, int centre_x, int centre_y);

int can_build_roomspace_of_dimensions(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y, int width, int height,
    TbBool full_check);

int can_build_fancy_roomspace(PlayerNumber plyr_idx, RoomKind rkind,
    struct RoomSpace roomspace);

struct RoomSpace check_slabs_in_roomspace(struct RoomSpace roomspace,
    PlayerNumber plyr_idx, RoomKind rkind, short rkind_cost);

int can_build_roomspace_of_dimensions_loose(PlayerNumber plyr_idx,
    RoomKind rkind, MapSlabCoord slb_x, MapSlabCoord slb_y, int width,
    int height, int *invalid_blocks, int roomspace_discovery_looseness);

int can_build_roomspace(PlayerNumber plyr_idx, RoomKind rkind,
    struct RoomSpace roomspace);
    
struct RoomSpace get_current_room_as_roomspace(PlayerNumber current_plyr_idx, 
                                               MapSlabCoord cursor_x, 
                                               MapSlabCoord cursor_y);

void get_dungeon_highlight_user_roomspace(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

void get_dungeon_sell_user_roomspace(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

void get_dungeon_build_user_roomspace(PlayerNumber plyr_idx, RoomKind rkind,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, int *mode, TbBool drag_check);

void keeper_highlight_roomspace(PlayerNumber plyr_idx, struct RoomSpace *roomspace, int task_allowance_reduction);
void keeper_sell_roomspace(struct RoomSpace *roomspace);
void keeper_build_roomspace(struct RoomSpace *roomspace);

void update_roomspaces();
/******************************************************************************/
#include "roomspace_detection.h"
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
