/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace.c
 *     Functions to facilitate the use of a "room space" (an area of many slabs)
 *     instead of a single slab when placing and selling rooms.
 
 * @par Purpose:
 *     Establishes a "room space" as a 2D array of booleans, where a value of 1
 *     represents the slabs that are in the "room space", and a value of 0 
 *     represents the slabs that are not in the "room space".
 * @par Comment:
 *     None.
 * @author   KeeperFx Team
 * @date     10 Jun 2020 - 07 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "front_input.h"
#include "player_utils.h"
#include "map_blocks.h"
#include "gui_soundmsgs.h"
#include "config_settings.h"
#include "slab_data.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
TbBool can_afford_roomspace(PlayerNumber plyr_idx, RoomKind rkind, int slab_count)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    return (slab_count * roomst->cost <= dungeon->total_money_owned);
}

int calc_distance_from_roomspace_centre(int total_distance, TbBool offset)
{
    return ((total_distance - 1 + offset) / 2);
}

int can_build_roomspace_of_dimensions_loose(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y, int width, int height, int *invalid_blocks, int roomspace_discovery_looseness)
{
    MapCoord buildx;
    MapCoord buildy;
    int count = 0;
    (*invalid_blocks) = 0;
    int leftExtent = slb_x - calc_distance_from_roomspace_centre(width,0);
    int rightExtent = slb_x + calc_distance_from_roomspace_centre(width,(width % 2 == 0));
    int topExtent = slb_y - calc_distance_from_roomspace_centre(height,0);
    int bottomExtent = slb_y + calc_distance_from_roomspace_centre(height,(height % 2 == 0));
    
    for (buildy = topExtent; buildy <= bottomExtent; buildy++)
    {
        for (buildx = leftExtent; buildx <= rightExtent; buildx++)
        {
            int room_check = check_room_at_slab_loose(plyr_idx, rkind, buildx, buildy, roomspace_discovery_looseness);
            if (room_check > 0)
            {
                count++;
            }
            if (room_check > 1)
            {
                (*invalid_blocks)++;
            }
        }
    }
    return count;
}

struct RoomSpace create_box_roomspace_from_drag(struct RoomSpace roomspace, MapSlabCoord start_x, MapSlabCoord start_y, MapSlabCoord end_x, MapSlabCoord end_y)
{
    if (abs(end_x - start_x) >= MAX_USER_ROOMSPACE_WIDTH)
    {
        end_x = ((end_x >= start_x) ? (start_x + MAX_USER_ROOMSPACE_WIDTH - 1) : (start_x - MAX_USER_ROOMSPACE_WIDTH + 1));
    }
    if (abs(end_y - start_y) >= MAX_USER_ROOMSPACE_WIDTH)
    {
        end_y = ((end_y >= start_y) ? (start_y + MAX_USER_ROOMSPACE_WIDTH - 1) : (start_y - MAX_USER_ROOMSPACE_WIDTH + 1));
    }
    TbBool blank_slab_grid[MAX_ROOMSPACE_WIDTH][MAX_ROOMSPACE_WIDTH] = {{false}};
    memcpy(&roomspace.slab_grid, &blank_slab_grid, sizeof(blank_slab_grid));
    roomspace.left   = ((start_x <= end_x) ? start_x : end_x);
    roomspace.right  = ((end_x >= start_x) ? end_x : start_x);
    roomspace.top    = ((start_y <= end_y) ? start_y : end_y);
    roomspace.bottom = ((end_y >= start_y) ? end_y : start_y);
    roomspace.width = roomspace.right - roomspace.left + 1;
    roomspace.height = roomspace.bottom - roomspace.top + 1;
    roomspace.slab_count = roomspace.width * roomspace.height;
    roomspace.centreX = roomspace.left + calc_distance_from_roomspace_centre(roomspace.width, 0);
    roomspace.centreY = roomspace.top + calc_distance_from_roomspace_centre(roomspace.height, 0);
    roomspace.is_roomspace_a_single_subtile = false;
    roomspace.is_roomspace_a_box = true;
    roomspace.render_roomspace_as_box = true;
    roomspace.tag_for_dig = false;
    roomspace.highlight_mode = false;
    roomspace.untag_mode = false;
    roomspace.one_click_mode_exclusive = false;
    roomspace.drag_mode = true;
    roomspace.drag_start_x = start_x;
    roomspace.drag_start_y = start_y;
    roomspace.drag_end_x = end_x;
    roomspace.drag_end_y = end_y;
    return roomspace;
}

struct RoomSpace create_box_roomspace(struct RoomSpace roomspace, int width, int height, int centre_x, int centre_y)
{
    TbBool blank_slab_grid[MAX_ROOMSPACE_WIDTH][MAX_ROOMSPACE_WIDTH] = {{false}};
    memcpy(&roomspace.slab_grid, &blank_slab_grid, sizeof(blank_slab_grid));
    roomspace.left   = centre_x - calc_distance_from_roomspace_centre(width,0);
    roomspace.right  = centre_x + calc_distance_from_roomspace_centre(width,(width % 2 == 0));
    roomspace.top    = centre_y - calc_distance_from_roomspace_centre(height,0);
    roomspace.bottom = centre_y + calc_distance_from_roomspace_centre(height,(height % 2 == 0));
    roomspace.width = width;
    roomspace.height = height;
    roomspace.slab_count = roomspace.width * roomspace.height;
    roomspace.centreX = centre_x;
    roomspace.centreY = centre_y;
    roomspace.is_roomspace_a_single_subtile = false;
    roomspace.is_roomspace_a_box = true;
    roomspace.render_roomspace_as_box = true;
    roomspace.tag_for_dig = false;
    roomspace.highlight_mode = false;
    roomspace.untag_mode = false;
    roomspace.one_click_mode_exclusive = false;
    roomspace.drag_mode = false;
    return roomspace;
}

int can_build_roomspace_of_dimensions(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y, int width, int height, TbBool full_check)
{
    MapCoord buildx;
    MapCoord buildy;
    int count = 0;
    int leftExtent = slb_x - calc_distance_from_roomspace_centre(width,0);
    int rightExtent = slb_x + calc_distance_from_roomspace_centre(width,(width % 2 == 0));
    int topExtent = slb_y - calc_distance_from_roomspace_centre(height,0);
    int bottomExtent = slb_y + calc_distance_from_roomspace_centre(height,(height % 2 == 0));
    
    for (buildy = topExtent; buildy <= bottomExtent; buildy++)
    {
        for (buildx = leftExtent; buildx <= rightExtent; buildx++)
        {
            if (full_check)
            {
                if (can_build_room_at_slab(plyr_idx, rkind, buildx, buildy))
                {
                    count++;
                }
            }
            else
            {
                if (can_build_room_at_slab_fast(plyr_idx, rkind, buildx, buildy))
                {
                    count++;
                }
            }
        }
    }
    if (full_check)
    {
        if (!can_afford_roomspace(plyr_idx, rkind, count))
        {
            return 0;
        }
    }
    return count;
}

int can_build_fancy_roomspace(PlayerNumber plyr_idx, RoomKind rkind, struct RoomSpace roomspace)
{
    if (!can_afford_roomspace(plyr_idx, rkind, roomspace.slab_count))
    {
        return 0;
    }
    return roomspace.slab_count;
}

struct RoomSpace check_slabs_in_roomspace(struct RoomSpace roomspace, short rkind_cost)
{
    roomspace.slab_count = 0;
    roomspace.invalid_slabs_count = 0;
    update_slab_grid(&roomspace, roomspace.drag_direction, false);
    roomspace.total_roomspace_cost = roomspace.slab_count * rkind_cost;
    if (roomspace.slab_count != (roomspace.width * roomspace.height))
    {
        roomspace.is_roomspace_a_box = false;
        roomspace.render_roomspace_as_box = false;
    }
    struct PlayerInfo* player = get_player(roomspace.plyr_idx);
    if (player->roomspace_mode != drag_placement_mode) // don't alter the roomspace in drag mode
    {
        if ((roomspace.slab_count == 0) || (roomspace.slab_count > MAX_USER_ROOMSPACE_WIDTH * MAX_USER_ROOMSPACE_WIDTH))
        {
            roomspace = create_box_roomspace(roomspace, 1, 1, roomspace.centreX, roomspace.centreY);
        }
    }
    return roomspace;
}

struct RoomSpace check_roomspace_for_diggable_slabs(struct RoomSpace roomspace, PlayerNumber plyr_idx)
{
    roomspace.slab_count = 0;
    roomspace.invalid_slabs_count = 0;
    roomspace.render_roomspace_as_box = true;
    roomspace.is_roomspace_a_box = true;
    for (int y = 0; y < roomspace.height; y++)
    {
        int current_y = roomspace.top + y;
        for (int x = 0; x < roomspace.width; x++)
        {
            int current_x = roomspace.left + x;
            if ( (subtile_is_diggable_for_player(plyr_idx, slab_subtile(current_x, 0), slab_subtile(current_y, 0), false))
                && ( ((find_from_task_list(plyr_idx, get_subtile_number(stl_slab_center_subtile(slab_subtile(current_x, 0)),stl_slab_center_subtile(slab_subtile(current_y, 0)))) != -1) && roomspace.untag_mode) 
                  || ((find_from_task_list(plyr_idx, get_subtile_number(stl_slab_center_subtile(slab_subtile(current_x, 0)),stl_slab_center_subtile(slab_subtile(current_y, 0)))) == -1) && !roomspace.untag_mode) ) )
            {
                roomspace.slab_grid[x][y] = true;
                roomspace.slab_count++;
            }
            else
            {
                roomspace.slab_grid[x][y] = false;
                roomspace.invalid_slabs_count++;
            }
        }
    }
    roomspace.total_roomspace_cost = 0;
    if (roomspace.slab_count != (roomspace.width * roomspace.height))
    {
        if (roomspace.slab_count != 0) // this ensures we show an empty "red" bounding box
        {
            roomspace.is_roomspace_a_box = false;
        }
    }
    return roomspace;
}

struct RoomSpace check_roomspace_for_sellable_slabs(struct RoomSpace roomspace, PlayerNumber plyr_idx)
{
    roomspace.slab_count = 0;
    roomspace.invalid_slabs_count = 0;
    roomspace.render_roomspace_as_box = true;
    roomspace.is_roomspace_a_box = true;
    update_slab_grid(&roomspace, roomspace.drag_direction, true);
    roomspace.total_roomspace_cost = 0;
    if (roomspace.slab_count != (roomspace.width * roomspace.height))
    {
        if (roomspace.slab_count != 0) // this ensures we show an empty "red" bounding box
        {
            roomspace.is_roomspace_a_box = false;
        }
    }
    return roomspace;
}

void create_roomspace_from_current_room(struct RoomSpace *roomspace, int search_width, int room_index)
{
    // get an array to write to
    struct RoomSpace current_roomspace = *roomspace;
    current_roomspace.slab_count = 0;
    current_roomspace.is_roomspace_a_box = true;
    current_roomspace.render_roomspace_as_box = true;
    //current slab
    int centre_x = current_roomspace.centreX; // current position; x
    int centre_y = current_roomspace.centreY; // current position; y
    // Get current room
    struct Room* current_room = slab_room_get(centre_x, centre_y);
    //store extents for room in x and y
    int left_extent = centre_x;
    int top_extent = centre_y;
    int right_extent = centre_x;
    int bottom_extent = centre_y;
    // Loop through list of slabs in the room to find extents
    unsigned long k = 0;
    long i = current_room->slabs_list;
    while (i != 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        // Per room tile code
        if (slb_x < left_extent)
        {
            left_extent = slb_x;
        }
        if (slb_y < top_extent)
        {
            top_extent = slb_y;
        }
        if (slb_x > right_extent)
        {
            right_extent = slb_x;
        }
        if (slb_y > bottom_extent)
        {
            bottom_extent = slb_y;
        }
        // Per room tile code ends
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > current_room->slabs_count)
        {
            // have gone through every slab in room, so exit loop
            break;
        }
    }
    // Set width and height of roomspace (making sure it is between 1 and MAX_ROOMSPACE_WIDTH)
    int current_width  = min(MAX_ROOMSPACE_WIDTH - 1, max(1, right_extent - left_extent + 1));
    int current_height = min(MAX_ROOMSPACE_WIDTH - 1, max(1, bottom_extent - top_extent + 1));

    // Loop through all of the slabs within the extents, and then test those slabs to see if they are part of the room.
    for (int y = 0; y <= current_height; y++)
    {
        for (int x = 0; x <= current_width; x++)
        {
            struct SlabMap* slb = get_slabmap_block(left_extent + x, top_extent + y);
            if (slb->room_index == room_index)
            {
                current_roomspace.slab_grid[x][y] = true;
                current_roomspace.slab_count++;
            }
            else
            {
                current_roomspace.slab_grid[x][y] = false;
                current_roomspace.invalid_slabs_count++;
            }
        }
    }
    // Set extents of new roomspace
    current_roomspace.width = current_width;
    current_roomspace.height = current_height;
    centre_x = left_extent + ((current_width - 1 - (current_width % 2 == 0)) / 2);
    centre_y = top_extent + ((current_height - 1 - (current_height % 2 == 0)) / 2);
    current_roomspace.left = centre_x - calc_distance_from_roomspace_centre(current_width,0);
    current_roomspace.right = centre_x + calc_distance_from_roomspace_centre(current_width,(current_width % 2 == 0));
    current_roomspace.top = centre_y - calc_distance_from_roomspace_centre(current_height,0);
    current_roomspace.bottom = centre_y + calc_distance_from_roomspace_centre(current_height,(current_height % 2 == 0));
    current_roomspace.centreX = current_roomspace.left + ((current_roomspace.width - 1 - (current_roomspace.width % 2 == 0)) / 2);
    current_roomspace.centreY = current_roomspace.top + ((current_roomspace.height - 1 - (current_roomspace.height % 2 == 0)) / 2);
    if (current_roomspace.width * current_roomspace.height > current_roomspace.slab_count)
    {
        current_roomspace.is_roomspace_a_box = false;
        current_roomspace.render_roomspace_as_box = false;
    }
    *roomspace = current_roomspace;
}

struct RoomSpace get_current_room_as_roomspace(PlayerNumber current_plyr_idx, MapSlabCoord cursor_x, MapSlabCoord cursor_y)
{
    struct SlabMap *slb = get_slabmap_block(cursor_x, cursor_y);
    // Set default "room" - i.e. 1x1 slabs, centred on the cursor
    struct RoomSpace default_room = { {{false}}, 0, true, 1, 1, cursor_x, cursor_y, cursor_x, cursor_y, cursor_x, cursor_y, 0, 0, current_plyr_idx, RoK_SELL, false, 0, 0, false, true, false, false, false, false, 0, 0, 0, 0, false, top_left_to_bottom_right };
    
    if (slabmap_owner(slb) == current_plyr_idx)
    {
        if (subtile_is_sellable_room(current_plyr_idx,slab_subtile(cursor_x,0), slab_subtile(cursor_y,0)))
        {
            // return a RoomSpace of the "current room"
            struct RoomSpace current_room = default_room;
            int room_index = slb->room_index;
            create_roomspace_from_current_room(&current_room, MAX_USER_ROOMSPACE_WIDTH, room_index);
            
            if (current_room.slab_count > 0)
            {
                return current_room;
            }
        }
    }
    default_room = create_box_roomspace(default_room, 1, 1, cursor_x, cursor_y);
    default_room.slab_count = 0;
    return default_room; // return empty 1x1 roomspace
}

int can_build_roomspace(PlayerNumber plyr_idx, RoomKind rkind, struct RoomSpace roomspace)
{
    int canbuild = 0;
    if (roomspace.is_roomspace_a_box)
    {
        canbuild = can_build_roomspace_of_dimensions(plyr_idx, rkind, roomspace.centreX, roomspace.centreY, roomspace.width, roomspace.height, true);
    }
    else
    {
        canbuild = can_build_fancy_roomspace(plyr_idx, rkind, roomspace);
    }
    return canbuild;
}

int numpad_to_value(TbBool allow_zero)
{
    int value = 0;
    if (!allow_zero)
    {
        value = 1;
    }
    if (is_key_pressed(KC_NUMPAD0, KMod_DONTCARE) && allow_zero)
    {
        value = 0;
    }
    else if (is_key_pressed(KC_NUMPAD1, KMod_DONTCARE))
    {
        value = 1;
    }
    else if (is_key_pressed(KC_NUMPAD2, KMod_DONTCARE))
    {
        value = 2;
    }
    else if (is_key_pressed(KC_NUMPAD3, KMod_DONTCARE))
    {
        value = 3;
    }
    else if (is_key_pressed(KC_NUMPAD4, KMod_DONTCARE))
    {
        value = 4;
    }
    else if (is_key_pressed(KC_NUMPAD5, KMod_DONTCARE))
    {
        value = 5;
    }
    else if (is_key_pressed(KC_NUMPAD6, KMod_DONTCARE))
    {
        value = 6;
    }
    else if (is_key_pressed(KC_NUMPAD7, KMod_DONTCARE))
    {
        value = 7;
    }
    else if (is_key_pressed(KC_NUMPAD8, KMod_DONTCARE))
    {
        value = 8;
    }
    else if (is_key_pressed(KC_NUMPAD9, KMod_DONTCARE))
    {
        value = 9;
    }
    return value;
}

void reset_dungeon_build_room_ui_variables(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    player->roomspace_detection_looseness = DEFAULT_USER_ROOMSPACE_DETECTION_LOOSENESS;
    player->user_defined_roomspace_width = DEFAULT_USER_ROOMSPACE_WIDTH;
}

void get_dungeon_highlight_user_roomspace(struct RoomSpace *roomspace, PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    struct RoomSpace current_roomspace;
    TbBool highlight_mode = false;
    TbBool untag_mode = false;
    TbBool one_click_mode_exclusive = false;
    MapSlabCoord drag_start_x = slb_x;
    MapSlabCoord drag_start_y = slb_y;
    struct Packet* pckt = get_packet_direct(player->packet_num);
    if (player->ignore_next_PCtr_LBtnRelease)
    {
        // because player cancelled a tag/untag with RMB, we need to default back to vanilla 1x1 box
        player->render_roomspace.drag_mode = false;
        player->one_click_lock_cursor = false;
        reset_dungeon_build_room_ui_variables(plyr_idx);
        current_roomspace = create_box_roomspace(player->render_roomspace, player->roomspace_width, player->roomspace_height, slb_x, slb_y);
        current_roomspace.highlight_mode = false;
        current_roomspace.untag_mode = false;
        current_roomspace.one_click_mode_exclusive = false;
        current_roomspace = check_roomspace_for_diggable_slabs(current_roomspace, plyr_idx);
        player->boxsize = current_roomspace.slab_count;
        *roomspace = current_roomspace;
        return;
    }
    if (!player->render_roomspace.drag_mode) // reset drag start slab
    {
        player->render_roomspace.drag_start_x = slb_x;
        player->render_roomspace.drag_start_y = slb_y;
    }
    if ((pckt->control_flags & PCtr_LBtnHeld) == PCtr_LBtnHeld) // highlight "paint mode" enabled
    {
        player->one_click_lock_cursor = true;
        untag_mode = player->render_roomspace.untag_mode; // get tag/untag mode from the slab that was clicked (before the user started holding mouse button)
    }
    else // user is hovering the mouse cursor
    {
        if (find_from_task_list(plyr_idx, get_subtile_number(stl_slab_center_subtile(stl_x),stl_slab_center_subtile(stl_y))) != -1)
        {
            untag_mode = true;
        }
    }
    if ((player->swap_to_untag_mode == -1) && ((pckt->control_flags & PCtr_RBtnHeld) == PCtr_RBtnHeld) && (player->roomspace_highlight_mode == 2) && (!subtile_is_diggable_for_player(plyr_idx, stl_x, stl_y, false)) && ((pckt->control_flags & PCtr_LBtnAnyAction) == 0))
    {
        // Allow RMB + CTRL to work as expected over lowslabs (for tagging and untagging)
        // we reset swap_to_untag_mode whenever LMB is not pressed (i.e. we are still in preview mode)
        player->swap_to_untag_mode = 0;
    }
    if (player->swap_to_untag_mode == 0) // if swap_to_untag_mode ==  no / enabled
    {
        //if (untag_or_tag_started_on_undiggable_highslab OR lowslab)
        if (!subtile_is_diggable_for_player(plyr_idx, stl_x, stl_y, false))
        {
            player->swap_to_untag_mode = 1; // maybe
        }
    }
    if (player->roomspace_highlight_mode == 1)
    {
        if (((pckt->control_flags & PCtr_HeldAnyButton) != 0) || ((pckt->control_flags & PCtr_LBtnRelease) != 0))
        {
            player->one_click_lock_cursor = true; // Allow click and drag over low slabs (if clicked on high slab)
            untag_mode = player->render_roomspace.untag_mode; // get tag/untag mode from the slab that was clicked (before the user started holding mouse button)
            one_click_mode_exclusive = true; // Block camera zoom/rotate if Ctrl is held with LMB/RMB
            drag_start_x = player->render_roomspace.drag_start_x; // if we are dragging, get the starting coords from the slab the player clicked on
            drag_start_y = player->render_roomspace.drag_start_y;
        }
        if (((pckt->control_flags & PCtr_RBtnHeld) != 0) && ((pckt->control_flags & PCtr_LBtnClick) != 0))
        {
            player->ignore_next_PCtr_RBtnRelease = true;
        }
        if (((pckt->control_flags & PCtr_LBtnHeld) != 0) && ((pckt->control_flags & PCtr_RBtnClick) != 0))
        {
            player->ignore_next_PCtr_LBtnRelease = true;
            player->ignore_next_PCtr_RBtnRelease = true;
            drag_start_x = slb_x;
            drag_start_y = slb_y;
        }
        highlight_mode = true;
        current_roomspace = create_box_roomspace_from_drag(player->render_roomspace, drag_start_x, drag_start_y, slb_x, slb_y);
    }
    else if (player->roomspace_highlight_mode == 2) // Define square room (mouse scroll-wheel changes size - default is 5x5)
    {
        if ((pckt->control_flags & PCtr_HeldAnyButton) != 0) // Block camera zoom/rotate if Ctrl is held with LMB/RMB
        {
            player->one_click_lock_cursor = true;
            one_click_mode_exclusive = true;
        }
        player->roomspace_width = player->roomspace_height = player->user_defined_roomspace_width;
        highlight_mode = true;
        current_roomspace = create_box_roomspace(player->render_roomspace, player->roomspace_width, player->roomspace_height, slb_x, slb_y);
    }
    else
    {
        current_roomspace = create_box_roomspace(player->render_roomspace, player->roomspace_width, player->roomspace_height, slb_x, slb_y);
    }
    current_roomspace.highlight_mode = highlight_mode;
    current_roomspace.untag_mode = untag_mode;
    current_roomspace.one_click_mode_exclusive = one_click_mode_exclusive;
    current_roomspace = check_roomspace_for_diggable_slabs(current_roomspace, plyr_idx);
    if (player->swap_to_untag_mode == 1) // if swap_to_untag_mode == maybe
    {
        // highlight roomspace was started on undiggable highslab, and we are therefore in "tag mode"...
        if (current_roomspace.slab_count == 0)
        {
            // if highlight roomspace is empty
            // then check for slabs for untagging instead, and if some are found, change to untag mode
            struct RoomSpace untag_roomspace = current_roomspace;
            untag_roomspace.untag_mode = true;
            untag_roomspace = check_roomspace_for_diggable_slabs(untag_roomspace, plyr_idx);
            if ((untag_roomspace.slab_count > 0) && ((pckt->control_flags & PCtr_LBtnAnyAction) == 0)) //only switch modes when no buttons are held
            {
                current_roomspace = untag_roomspace;
                player->swap_to_untag_mode = 2;
            }
        }
        else if (current_roomspace.slab_count > 0)
        {
            // player has started a "room" in tag mode, so...
            player->swap_to_untag_mode = -1; // disable
        }
    }
    player->boxsize = current_roomspace.slab_count;
    if (current_roomspace.slab_count > 0)
    {
        current_roomspace.tag_for_dig = true;
    }
    if ((player->one_click_lock_cursor) && ((pckt->control_flags & PCtr_LBtnHeld) != 0) && (!current_roomspace.drag_mode))
    {
        current_roomspace.is_roomspace_a_box = true; // force full box cursor in "paint mode" - this stops the accurate boundbox appearing for a frame, before the slabs are tagged/untagged (which appears as flickering to the user)
    }
    if (player->swap_to_untag_mode == 2) // if swap_to_untag_mode == yes
    {
        // change to untag mode, as requested, and disable swap_to_untag_mode
        set_tag_untag_mode(plyr_idx, stl_x, stl_y);
        player->swap_to_untag_mode = -1; // disable
    }
    *roomspace = current_roomspace;
}

void get_dungeon_sell_user_roomspace(struct RoomSpace *roomspace, PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct RoomSpace current_roomspace;
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    current_roomspace.plyr_idx = plyr_idx;
    MapSlabCoord drag_start_x = slb_x;
    MapSlabCoord drag_start_y = slb_y;
    struct Packet* pckt = get_packet_direct(player->packet_num);
    player->one_click_lock_cursor = false;
    player->one_click_mode_exclusive = false;
    if (player->ignore_next_PCtr_LBtnRelease)
    {
        // because player cancelled with RMB, we need to default back to vanilla 1x1 box
        player->render_roomspace.drag_mode = false;
        reset_dungeon_build_room_ui_variables(plyr_idx);
        current_roomspace = create_box_roomspace(player->render_roomspace, player->roomspace_width, player->roomspace_height, slb_x, slb_y);
        current_roomspace.highlight_mode = false;
        current_roomspace.untag_mode = false;
        current_roomspace.one_click_mode_exclusive = false;
        current_roomspace = check_roomspace_for_diggable_slabs(current_roomspace, plyr_idx);
        player->boxsize = current_roomspace.slab_count;
        *roomspace = current_roomspace;
        player->ignore_next_PCtr_LBtnRelease = false;
        return;
    }
    if (!player->render_roomspace.drag_mode) // reset drag start slab
    {
        player->render_roomspace.drag_start_x = slb_x;
        player->render_roomspace.drag_start_y = slb_y;
    }
    if (player->roomspace_mode == roomspace_detection_mode)
    {
        current_roomspace = get_current_room_as_roomspace(plyr_idx, slb_x, slb_y);
        if (!current_roomspace.is_roomspace_a_box)
        {
            current_roomspace.render_roomspace_as_box = false;
        }
    }
    else if (player->roomspace_mode == box_placement_mode)
    {
        current_roomspace.is_roomspace_a_box = true;
        current_roomspace.render_roomspace_as_box = true;
        current_roomspace = create_box_roomspace(current_roomspace, player->roomspace_width, player->roomspace_height, slb_x, slb_y);
        current_roomspace.drag_direction = top_left_to_bottom_right;
        current_roomspace = check_roomspace_for_sellable_slabs(current_roomspace, plyr_idx);
    }
    else if (player->roomspace_mode == single_subtile_mode)
    {
        current_roomspace = player->render_roomspace;
    }
    else if (player->roomspace_mode == drag_placement_mode)
    {
        if (((pckt->control_flags & PCtr_HeldAnyButton) != 0) || ((pckt->control_flags & PCtr_LBtnRelease) != 0))
        {
            player->one_click_lock_cursor = true; // Allow click and drag over low slabs (if clicked on high slab)
            drag_start_x = player->render_roomspace.drag_start_x; // if we are dragging, get the starting coords from the slab the player clicked on
            drag_start_y = player->render_roomspace.drag_start_y;
        }
        if (((pckt->control_flags & PCtr_RBtnHeld) != 0) && ((pckt->control_flags & PCtr_LBtnClick) != 0))
        {
            player->ignore_next_PCtr_RBtnRelease = true;
        }
        if (((pckt->control_flags & PCtr_LBtnHeld) != 0) && ((pckt->control_flags & PCtr_RBtnClick) != 0))
        {
            player->ignore_next_PCtr_LBtnRelease = true;
            player->ignore_next_PCtr_RBtnRelease = true;
            drag_start_x = slb_x;
            drag_start_y = slb_y;
        }
        current_roomspace.is_roomspace_a_box = true;
        current_roomspace.render_roomspace_as_box = true;
        current_roomspace = create_box_roomspace_from_drag(current_roomspace, drag_start_x, drag_start_y, slb_x, slb_y);
        if (roomspace->drag_start_y > roomspace->drag_end_y)
        {
            if (roomspace->drag_start_x > roomspace->drag_end_x)
            {
                current_roomspace.drag_direction = bottom_right_to_top_left;
            }
            else
            {
                current_roomspace.drag_direction = bottom_left_to_top_right;
            }
        }
        else
        {
            if (roomspace->drag_start_x > roomspace->drag_end_x)
            {
                current_roomspace.drag_direction = top_right_to_bottom_left;
            }
            else
            {
                current_roomspace.drag_direction = top_left_to_bottom_right;
            }
        }
        current_roomspace = check_roomspace_for_sellable_slabs(current_roomspace, plyr_idx);
        player->roomspace_width = current_roomspace.width;
        player->roomspace_height = current_roomspace.height;
    }
    player->boxsize = current_roomspace.slab_count;
    current_roomspace.one_click_mode_exclusive = player->one_click_mode_exclusive;
    *roomspace = current_roomspace;
}

void get_dungeon_build_user_roomspace(struct RoomSpace *roomspace, PlayerNumber plyr_idx, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char mode)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    struct RoomSpace best_roomspace;
    best_roomspace.is_roomspace_a_box = true;
    best_roomspace.render_roomspace_as_box = true;
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    best_roomspace.plyr_idx = plyr_idx;
    best_roomspace.rkind = rkind;
    MapSlabCoord drag_start_x = slb_x;
    MapSlabCoord drag_start_y = slb_y;
    struct Packet* pckt = get_packet_direct(player->packet_num);
    struct RoomSpace temp_best_room;
    player->one_click_lock_cursor = false;
    if (player->ignore_next_PCtr_LBtnRelease)
    {
        // because player cancelled a tag/untag with RMB, we need to default back to vanilla 1x1 box
        player->render_roomspace.drag_mode = false;
        reset_dungeon_build_room_ui_variables(plyr_idx);
        best_roomspace = create_box_roomspace(player->render_roomspace, player->roomspace_width, player->roomspace_height, slb_x, slb_y);
        best_roomspace.highlight_mode = false;
        best_roomspace.untag_mode = false;
        best_roomspace.one_click_mode_exclusive = false;
        best_roomspace = check_roomspace_for_diggable_slabs(best_roomspace, plyr_idx);
        player->boxsize = best_roomspace.slab_count;
        *roomspace = best_roomspace;
        player->ignore_next_PCtr_LBtnRelease = false;
        return;
    }
    if (!player->render_roomspace.drag_mode) // reset drag start slab
    {
        player->render_roomspace.drag_start_x = slb_x;
        player->render_roomspace.drag_start_y = slb_y;
    }
    if ((pckt->control_flags & PCtr_LBtnHeld) == PCtr_LBtnHeld) // highlight "paint mode" enabled
    {
        player->one_click_lock_cursor = true;
    }
    if (mode == roomspace_detection_mode) // room auto-detection mode
    {
        best_roomspace = get_biggest_roomspace(plyr_idx, rkind, slb_x, slb_y, roomst->cost, 0, 32, player->roomspace_detection_looseness);
        slb_x = best_roomspace.centreX;
        slb_y = best_roomspace.centreY;
        player->boxsize = best_roomspace.slab_count; // correct number of tiles always returned from get_biggest_roomspace
    }
    else if ( (mode == drag_placement_mode) && (player->roomspace_drag_paint_mode == false) )
    {
        if (((pckt->control_flags & PCtr_HeldAnyButton) != 0) || ((pckt->control_flags & PCtr_LBtnRelease) != 0))
        {
            player->one_click_lock_cursor = true; // Allow click and drag over low slabs (if clicked on high slab)
            player->one_click_mode_exclusive = true; // Block camera zoom/rotate if Ctrl is held with LMB/RMB
            drag_start_x = player->render_roomspace.drag_start_x; // if we are dragging, get the starting coords from the slab the player clicked on
            drag_start_y = player->render_roomspace.drag_start_y;
        }
        if (((pckt->control_flags & PCtr_RBtnHeld) != 0) && ((pckt->control_flags & PCtr_LBtnClick) != 0))
        {
            player->ignore_next_PCtr_RBtnRelease = true;
        }
        if (((pckt->control_flags & PCtr_LBtnHeld) != 0) && ((pckt->control_flags & PCtr_RBtnClick) != 0))
        {
            player->ignore_next_PCtr_LBtnRelease = true;
            player->ignore_next_PCtr_RBtnRelease = true;
            drag_start_x = slb_x;
            drag_start_y = slb_y;
        }
        TbBool can_drag; 
        if (room_role_matches(rkind,RoRoF_PassWater|RoRoF_PassLava))
        {
            can_drag = ((can_build_room_at_slab(plyr_idx, rkind, drag_start_x, drag_start_y)) || 
                        (room_role_matches(rkind,RoRoF_PassWater) && (players_land_by_slab_kind(plyr_idx, drag_start_x, drag_start_y,SlbT_WATER))) ||
                        (room_role_matches(rkind,RoRoF_PassLava)  && (players_land_by_slab_kind(plyr_idx, drag_start_x, drag_start_y,SlbT_LAVA))) );
            player->one_click_mode_exclusive = false;
        }
        else
        {
            can_drag = true;
        }
        if (can_drag)
        {
            temp_best_room = create_box_roomspace_from_drag(best_roomspace, drag_start_x, drag_start_y, slb_x, slb_y);
        }
        else
        {
            temp_best_room = create_box_roomspace(best_roomspace, 1, 1, slb_x, slb_y);
        }
        if (!player->roomspace.is_active)
        {
            detect_roomspace_direction(&temp_best_room);
        }
        if (room_role_matches(rkind,RoRoF_PassWater|RoRoF_PassLava))
        {
            detect_bridge_shape(plyr_idx);
        }
        temp_best_room = check_slabs_in_roomspace(temp_best_room, roomst->cost);
        best_roomspace = temp_best_room;
        player->boxsize = best_roomspace.slab_count;
        player->roomspace_width = best_roomspace.width;
        player->roomspace_height = best_roomspace.height;
        best_roomspace.render_roomspace_as_box = true;
    }
    else
    {
        temp_best_room = create_box_roomspace(best_roomspace, player->roomspace_width, player->roomspace_height, slb_x, slb_y);
        temp_best_room.drag_direction = top_left_to_bottom_right;
        temp_best_room = check_slabs_in_roomspace(temp_best_room, roomst->cost);
        best_roomspace = temp_best_room;
        player->boxsize = best_roomspace.slab_count; // correct number of tiles returned from check_slabs_in_roomspace
            // Make sure the "outer box" bounding is drawn with square room mode
            best_roomspace.width = player->roomspace_width;
            best_roomspace.height = player->roomspace_height;
            best_roomspace.render_roomspace_as_box = true;
    }
    if ((player->one_click_lock_cursor) && ((pckt->control_flags & PCtr_LBtnHeld) != 0) && (!best_roomspace.drag_mode) && (mode != roomspace_detection_mode))
    {
        best_roomspace.is_roomspace_a_box = true;
    }
    best_roomspace.one_click_mode_exclusive = player->one_click_mode_exclusive;
    *roomspace = best_roomspace; // make sure we can render the correct boundbox to the user
}

static void sell_at_point(struct RoomSpace *roomspace)
{
    struct SlabMap *slb = get_slabmap_block(roomspace->buildx, roomspace->buildy);
    if (slabmap_owner(slb) == roomspace->plyr_idx)
    {
        if (player_sell_trap_at_subtile(roomspace->plyr_idx, slab_subtile_center(roomspace->buildx), slab_subtile_center(roomspace->buildy)) ||
            player_sell_door_at_subtile(roomspace->plyr_idx, slab_subtile(roomspace->buildx, 0), slab_subtile(roomspace->buildy, 0)) ) // Trying to sell trap
        {
            // Nothing to do here - trap already sold
        } else
        if (subtile_is_sellable_room(roomspace->plyr_idx,slab_subtile(roomspace->buildx,0), slab_subtile(roomspace->buildy,0)))// Trying to sell room
        {
            player_sell_room_at_subtile(roomspace->plyr_idx,slab_subtile(roomspace->buildx,0), slab_subtile(roomspace->buildy,0));
        }
    }
}

static void find_next_point(struct RoomSpace *roomspace, unsigned char mode)
{
    // these store the coordinates of roomspace.slab_grid[][], rather than the in-game map coordinates
    int room_x = roomspace->buildx - roomspace->left;
    int room_y = roomspace->buildy - roomspace->top;
    switch(mode)
    {
        case 0: // top-left to bottom-right
        {
            while ((roomspace->buildy <= roomspace->bottom) && (roomspace->buildx <= roomspace->right))
            {
                if (roomspace->slab_grid[room_x][room_y]) // the slab is part of the room
                {
                    break;
                }
                room_x++;
                roomspace->buildx++;
                if (roomspace->buildx > roomspace->right)
                {
                    room_x = 0;
                    roomspace->buildx = roomspace->left;
                    room_y++;
                    roomspace->buildy++;
                }
            }
            break;
        }
        case 1: // bottom-right to top-left
        {
            while ((roomspace->buildy >= roomspace->top) && (roomspace->buildx >= roomspace->left))
            {
                if (roomspace->slab_grid[room_x][room_y]) // the slab is part of the room
                {
                    break;
                }
                room_x--;
                roomspace->buildx--;
                if (roomspace->buildx < roomspace->left)
                {
                    room_x = roomspace->width - 1;
                    roomspace->buildx = roomspace->right;
                    room_y--;
                    roomspace->buildy--;
                }
            }
            break;
        }
        case 2: // top-right to bottom-left
        {
            while ((roomspace->buildy <= roomspace->bottom) && (roomspace->buildx >= roomspace->left))
            {
                if (roomspace->slab_grid[room_x][room_y]) // the slab is part of the room
                {
                    break;
                }
                room_x--;
                roomspace->buildx--;
                if (roomspace->buildx < roomspace->left)
                {
                    room_x = roomspace->width - 1;
                    roomspace->buildx = roomspace->right;
                    room_y++;
                    roomspace->buildy++;
                }
            }
            break;
        }
        case 3: // bottom-left to top-right
        {
            while ((roomspace->buildy >= roomspace->top) && (roomspace->buildx <= roomspace->right))
            {
                if (roomspace->slab_grid[room_x][room_y]) // the slab is part of the room
                {
                    break;
                }
                room_x++;
                roomspace->buildx++;
                if (roomspace->buildx > roomspace->right)
                {
                    room_x = 0;
                    roomspace->buildx = roomspace->left;
                    room_y--;
                    roomspace->buildy--;
                }
            }
            break;
        }
    }
}

void keeper_highlight_roomspace(PlayerNumber plyr_idx, struct RoomSpace *roomspace, int task_allowance_reduction)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    TbBool tag_for_digging = ((player->allocflags & PlaF_ChosenSlabHasActiveTask) == 0);
    int task_allowance = MAPTASKS_COUNT - task_allowance_reduction;
    for (int y = 0; y < roomspace->height; y++)
    {
        int current_y = roomspace->top + y;
        for (int x = 0; x < roomspace->width; x++)
        {
            int current_x = roomspace->left + x;
            
            // Tag a line of slabs inbetween previous mouse slab position and current mouse slab position
            int draw_path_x = player->previous_cursor_subtile_x / STL_PER_SLB;
            int draw_path_y = player->previous_cursor_subtile_y / STL_PER_SLB;
            while (true)
            {
                MapSubtlCoord stl_cx = stl_slab_center_subtile(draw_path_x * STL_PER_SLB);
                MapSubtlCoord stl_cy = stl_slab_center_subtile(draw_path_y * STL_PER_SLB);
                if (!tag_for_digging) // if the chosen slab is tagged for digging...
                {
                    untag_blocks_for_digging_in_rectangle_around(stl_cx, stl_cy, plyr_idx); // untag the slab for digging
                }
                else if (dungeon->task_count < task_allowance)
                {
                    tag_blocks_for_digging_in_rectangle_around(stl_cx, stl_cy, plyr_idx); // tag the slab for digging (add_task_list_entry is run by this which will increase dungeon->task_count by 1)
                }
                else if (is_my_player(player))
                {
                    output_message(SMsg_WorkerJobsLimit, 500, true); // show an error message if the task limit (MAPTASKS_COUNT) has been reached
                    return;
                }
                
                if (draw_path_x != current_x || draw_path_y != current_y) {
                    // Choose the axis that has more ground to cover.
                    if (abs(draw_path_x-current_x) > abs(draw_path_y-current_y)) {
                        if (draw_path_x < current_x) {
                            draw_path_x += 1;
                        } else {
                            draw_path_x -= 1;
                        }
                    } else {
                        if (draw_path_y < current_y) {
                            draw_path_y += 1;
                        } else {
                            draw_path_y -= 1;
                        }
                    }
                } else {
                    // Exit the While loop because the path has been drawn to the current_x & current_y
                    break;
                }
            }
        }
    }
}

void keeper_sell_roomspace(PlayerNumber plyr_idx, struct RoomSpace *roomspace)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    if (player->roomspace.is_active)
    {
        ERRORLOG("Selling roomspace while it is still in progress plyr:%d", roomspace->plyr_idx);
        return;
    }
    roomspace->rkind = RoK_SELL;
    memcpy(&player->roomspace, roomspace, sizeof(player->roomspace));
    // Init
    player->roomspace.is_active = true;
    if (!player->roomspace.drag_mode)
    {
        player->roomspace.buildx = roomspace->left;
        player->roomspace.buildy = roomspace->top;
    }
    else
    {
        player->roomspace.buildx = roomspace->drag_start_x;
        player->roomspace.buildy = roomspace->drag_start_y;
    }
    if (!roomspace->is_roomspace_a_box)
    {
        // We want to find first point
        find_next_point(&player->roomspace, roomspace->drag_direction);
    }
}

void keeper_build_roomspace(PlayerNumber plyr_idx, struct RoomSpace *roomspace)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    if (player->roomspace.is_active)
    {
        ERRORLOG("Building roomspace while it is still in progress plyr:%d", roomspace->plyr_idx);
        return;
    }
    memcpy(&player->roomspace, roomspace, sizeof(player->roomspace));
    // Init
    player->roomspace.is_active = true;
    if (!player->roomspace.drag_mode)
    {
        player->roomspace.buildx = roomspace->left;
        player->roomspace.buildy = roomspace->top;
    }
    else
    {
        player->roomspace.buildx = roomspace->drag_start_x;
        player->roomspace.buildy = roomspace->drag_start_y;
    }
    if (!roomspace->is_roomspace_a_box)
    {
        if (!player->roomspace.drag_mode)
        {
            player->roomspace.buildx--; // We want to find first point
        }
        find_next_point(&player->roomspace, roomspace->drag_direction);
    }
}

static void keeper_update_roomspace(struct RoomSpace *roomspace)
{
    if (!roomspace->is_active)
        return;
    // build a room
    if (roomspace->rkind == RoK_SELL)
        sell_at_point(roomspace);
    else
    {
        if (!is_room_available(roomspace->plyr_idx, roomspace->rkind))
        {
            roomspace->is_active = false;
            return;
        }
        keeper_build_room(slab_subtile(roomspace->buildx, 0), slab_subtile(roomspace->buildy, 0), roomspace->plyr_idx, roomspace->rkind);
    }
    // find next point
    if (roomspace->drag_mode)
    {
        switch (roomspace->drag_direction)
        {
            case top_left_to_bottom_right:
            {
                do
                {
                    roomspace->buildx++;
                    if (roomspace->buildx > roomspace->right)
                    {
                        roomspace->buildx = roomspace->left;
                        roomspace->buildy++;
                    }
                    if (!roomspace->is_roomspace_a_box)
                    {
                        find_next_point(roomspace, roomspace->drag_direction);
                    }
                    if ((roomspace->buildy > roomspace->bottom) || (roomspace->buildx > roomspace->right))
                    {
                        roomspace->is_active = false;
                        return;
                    }
                }
                while ( (roomspace->rkind != RoK_SELL) && (!can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, roomspace->buildx, roomspace->buildy)) );
                break;
            }
            case bottom_right_to_top_left:
            {
                do
                {
                    roomspace->buildx--;
                    if (roomspace->buildx < roomspace->left)
                    {
                        roomspace->buildx = roomspace->right;
                        roomspace->buildy--;
                    }
                    if (!roomspace->is_roomspace_a_box)
                    {
                        find_next_point(roomspace, roomspace->drag_direction);
                    }
                    if ((roomspace->buildy < roomspace->top) || (roomspace->buildx < roomspace->left))
                    {
                        roomspace->is_active = false;
                        return;
                    }
                }
                while ( (roomspace->rkind != RoK_SELL) && (!can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, roomspace->buildx, roomspace->buildy)) );
                break;
            }
            case top_right_to_bottom_left:
            {
                do
                {
                    roomspace->buildx--;
                    if (roomspace->buildx < roomspace->left)
                    {
                        roomspace->buildx = roomspace->right;
                        roomspace->buildy++;
                    }
                    if (!roomspace->is_roomspace_a_box)
                    {
                        find_next_point(roomspace, roomspace->drag_direction);
                    }
                    if ((roomspace->buildy > roomspace->bottom) || (roomspace->buildx < roomspace->left))
                    {
                        roomspace->is_active = false;
                        return;
                    }
                }
                while ( (roomspace->rkind != RoK_SELL) && (!can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, roomspace->buildx, roomspace->buildy)) );
                break;
            }
            case bottom_left_to_top_right:
            {
                do
                {
                    roomspace->buildx++;
                    if (roomspace->buildx > roomspace->right)
                    {
                        roomspace->buildx = roomspace->left;
                        roomspace->buildy--;
                    }
                    if (!roomspace->is_roomspace_a_box)
                    {
                        find_next_point(roomspace, roomspace->drag_direction);
                    }
                    if ((roomspace->buildy < roomspace->top) || (roomspace->buildx > roomspace->right))
                    {
                        roomspace->is_active = false;
                        return;
                    }
                }
                while ( (roomspace->rkind != RoK_SELL) && (!can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, roomspace->buildx, roomspace->buildy)) );
                break;
            }
        }
    }
    else
    {
        do
        {
            roomspace->buildx++;
            if (roomspace->buildx > roomspace->right)
            {
                roomspace->buildx = roomspace->left;
                roomspace->buildy++;
            }
            if (!roomspace->is_roomspace_a_box)
            {
                find_next_point(roomspace, top_left_to_bottom_right);
            }
            if ((roomspace->buildy > roomspace->bottom) || (roomspace->buildx > roomspace->right))
            {
                roomspace->is_active = false;
                return;
            }
        }
        while ( (roomspace->rkind != RoK_SELL) && (!can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, roomspace->buildx, roomspace->buildy)) );
    }
}

void update_roomspaces()
{
    for (PlayerNumber plyr_idx = 0; plyr_idx < DUNGEONS_COUNT; plyr_idx++)
    {
        if (get_player(plyr_idx)->is_active)
        {
            keeper_update_roomspace(&get_player(plyr_idx)->roomspace);
        }
    }
}

void process_build_roomspace_inputs(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    long keycode = 0;
    struct Packet* pckt = get_packet(plyr_idx);
    if (room_role_matches(player->chosen_room_kind,RoRoF_PassLava|RoRoF_PassWater))
    {
        TbBool drag_check = ( ( (is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true)) || (is_game_key_pressed(Gkey_SquareRoomSpace, &keycode, true)) ) && (left_button_held));
        if (drag_check) // Enable "paint mode" if Ctrl or Shift are held
        {
            set_packet_action(pckt, PckA_SetRoomspaceDragPaint, 0, 0, 0, 0);
        }
        else
        {
            set_packet_action(pckt, PckA_SetRoomspaceDrag, 0, 0, 0, 0);
        }
    }
    else if (is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true)) // Find "best" room
    {
        unsigned char looseness = player->roomspace_detection_looseness;
        if (is_game_key_pressed(Gkey_RoomSpaceIncSize, &keycode, true))
        {
            if (looseness < tolerate_gold)
            {
                looseness = tolerate_gold;
            }
            else if (looseness != tolerate_rock)
            {
                looseness = tolerate_rock;
            }
        }
        else if (is_game_key_pressed(Gkey_RoomSpaceDecSize, &keycode, true))
        {
            if (looseness == tolerate_rock)
            {
                looseness = tolerate_gold;
            }
            else if (looseness != disable_tolerance_layers)
            {
                looseness = disable_tolerance_layers;
            }
        }
        set_packet_action(pckt, PckA_SetRoomspaceAuto, looseness, 0, 0, 0);
    }
    else if (is_game_key_pressed(Gkey_SquareRoomSpace, &keycode, true)) // Define square room (mouse scroll-wheel changes size - default is 5x5)
    {
        int width = (player->roomspace_no_default) ? player->user_defined_roomspace_width : DEFAULT_USER_ROOMSPACE_WIDTH;
        if (is_game_key_pressed(Gkey_RoomSpaceIncSize, &keycode, true))
        {
            if (width != MAX_USER_ROOMSPACE_WIDTH)
            {
                width++;
                set_packet_action(pckt, PckA_SetRoomspaceMan, width, 0, 0, 0);
            }
        }
        else if (is_game_key_pressed(Gkey_RoomSpaceDecSize, &keycode, true))
        {
            if (width != MIN_USER_ROOMSPACE_WIDTH)
            {
                width--;
                set_packet_action(pckt, PckA_SetRoomspaceMan, width, 0, 0, 0);
            }
        }
        if (player->roomspace_no_default == false)
        {
            set_packet_action(pckt, PckA_SetRoomspaceMan, width, 0, 0, 0);
        }
    }
    else
        {
            int size = numpad_to_value(false);
            if (size > 1)
            {
                set_packet_action(pckt, PckA_SetRoomspaceDefault, size, 0, 0, 0);
            }
            else
            {
                set_packet_action(pckt, PckA_SetRoomspaceDrag, 0, 0, 0, 0);
            }
        }
}

void process_sell_roomspace_inputs(PlayerNumber plyr_idx)
{
    struct Packet* pckt = get_packet(plyr_idx);
    long keycode = 0;
    struct PlayerInfo* player = get_player(plyr_idx);
    if (is_game_key_pressed(Gkey_SellTrapOnSubtile, &keycode, true))
    {
        set_packet_action(pckt, PckA_SetRoomspaceSubtile, 0, 0, 0, 0);
    }
    else if (is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true))
    {
        set_packet_action(pckt, PckA_SetRoomspaceWholeRoom, 0, 0, 0, 0);
    }
    else if (is_game_key_pressed(Gkey_SquareRoomSpace, &keycode, true)) // Define square room (mouse scroll-wheel changes size - default is 5x5)
    {
        int width = (player->roomspace_no_default) ? player->user_defined_roomspace_width : DEFAULT_USER_ROOMSPACE_WIDTH;
        if (is_game_key_pressed(Gkey_RoomSpaceIncSize, &keycode, true))
        {
            if (width != MAX_USER_ROOMSPACE_WIDTH)
            {
                width++;
                set_packet_action(pckt, PckA_SetRoomspaceMan, width, 0, 0, 0);
            }
        }
        else if (is_game_key_pressed(Gkey_RoomSpaceDecSize, &keycode, true))
        {
            if (width != MIN_USER_ROOMSPACE_WIDTH)
            {
                width--;
                set_packet_action(pckt, PckA_SetRoomspaceMan, width, 0, 0, 0);
            }
        }
        if (player->roomspace_no_default == false)
        {
            set_packet_action(pckt, PckA_SetRoomspaceMan, width, 0, 0, 0);
        }
    }
    else
    {
        int size = numpad_to_value(false);
        if (size > 1)
        {
            set_packet_action(pckt, PckA_SetRoomspaceDefault, size, 0, 0, 0);
        }
        else
        {
            set_packet_action(pckt, PckA_SetRoomspaceDrag, 0, 0, 0, 0);
        }
    }
}

void process_highlight_roomspace_inputs(PlayerNumber plyr_idx)
{
    long keycode = 0;
    unsigned short par1, par2;
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true))
    {
        par2 = 1;
    }
    else
    {
        par2 = 0;
    }
    if ( (is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true)) ) // Use "modern" click and drag method
    {
        par1 = 1;
        par2 = 0;
    }
    else if ( (is_game_key_pressed(Gkey_SquareRoomSpace, &keycode, true))  ) // Use "modern" click and drag method
    {
        par1 = 2;
        par2 = (player->roomspace_no_default) ? player->user_defined_roomspace_width : DEFAULT_USER_ROOMSPACE_WIDTH;
        if (is_game_key_pressed(Gkey_RoomSpaceIncSize, &keycode, true))
        {
            if (par2 != MAX_USER_ROOMSPACE_WIDTH)
            {
                par2++;
            }
        }
        if (is_game_key_pressed(Gkey_RoomSpaceDecSize, &keycode, true))
        {
            if (par2 != MIN_USER_ROOMSPACE_WIDTH)
            {
                par2--;
            }
        }
    }
    else if (is_game_key_pressed(Gkey_SellTrapOnSubtile, &keycode, true) )
    {
        if (player->primary_cursor_state == CSt_PowerHand)
        {
            player = get_player(plyr_idx);
            if (player->roomspace_mode != single_subtile_mode)
            {
                struct Packet* pckt = get_packet(my_player_number);
                set_packet_action(pckt, PckA_SetRoomspaceSubtile, 0, 0, 0, 0);
            }
        }
        return;
    }
    else
    {
        par1 = 0;
        par2 = numpad_to_value(false);
    }
    set_players_packet_action(player, PckA_SetRoomspaceHighlight, par1, par2, 0, 0);
}

void update_slab_grid(struct RoomSpace* roomspace, unsigned char mode, TbBool sell)
{
    int x, y, current_x, current_y;
    TbBool can;
    switch (mode)
    {
        case top_left_to_bottom_right:
        {
            for (y = 0; y < roomspace->height; y++)
            {
                current_y = roomspace->top + y;
                for (x = 0; x < roomspace->width; x++)
                {
                    TbBool * row = roomspace->slab_grid[x];
                    current_x = roomspace->left + x;
                    if (roomspace->is_roomspace_a_box || roomspace->slab_grid[x][y] == true) // only check slabs in the roomspace
                    {
                        can = (sell) ? ((subtile_is_sellable_room(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0))) || (subtile_is_sellable_door_or_trap(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0)))) : (roomspace_can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, current_x, current_y));
                        if (can)
                        {
                            row[y] = true;
                            roomspace->slab_count++;
                        }
                        else
                        {
                            row[y] = false;
                            roomspace->invalid_slabs_count++;
                        }
                    }
                }
            }
            break;
        }
        case bottom_right_to_top_left:
        {
            for (y = 0; y < roomspace->height; y++)
            {
                current_y = roomspace->bottom - y;
                for (x = 0; x < roomspace->width; x++)
                {
                    TbBool * row = roomspace->slab_grid[(roomspace->width - 1) - x];
                    current_x = roomspace->right - x;
                    if (roomspace->is_roomspace_a_box || roomspace->slab_grid[(roomspace->width - 1) - x][(roomspace->height - 1) - y] == true) // only check slabs in the roomspace
                    {
                        can = (sell) ? ((subtile_is_sellable_room(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0))) || (subtile_is_sellable_door_or_trap(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0)))) : (roomspace_can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, current_x, current_y));
                        if (can)
                        {
                            row[(roomspace->height - 1) - y] = true;
                            roomspace->slab_count++;
                        }
                        else
                        {
                            row[(roomspace->height - 1) - y] = false;
                            roomspace->invalid_slabs_count++;
                        }
                    }
                }
            }
            break;
        }
        case top_right_to_bottom_left:
        {
            for (y = 0; y < roomspace->height; y++)
            {
                current_y = roomspace->top + y;
                for (x = 0; x < roomspace->width; x++)
                {
                    TbBool * row = roomspace->slab_grid[(roomspace->width - 1) - x];
                    current_x = roomspace->right - x;
                    if (roomspace->is_roomspace_a_box || roomspace->slab_grid[(roomspace->width - 1) - x][y] == true) // only check slabs in the roomspace
                    {
                        can = (sell) ? ((subtile_is_sellable_room(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0))) || (subtile_is_sellable_door_or_trap(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0)))) : (roomspace_can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, current_x, current_y));
                        if (can)
                        {
                            row[y] = true;
                            roomspace->slab_count++;
                        }
                        else
                        {
                            row[y] = false;
                            roomspace->invalid_slabs_count++;
                        }
                    }
                }
            }
            break;
        }
        case bottom_left_to_top_right:
        {
            for (y = 0; y < roomspace->height; y++)
            {
                current_y = roomspace->bottom - y;
                for (x = 0; x < roomspace->width; x++)
                {
                    TbBool * row = roomspace->slab_grid[x];
                    current_x = roomspace->left + x;
                    if (roomspace->is_roomspace_a_box || roomspace->slab_grid[x][(roomspace->height - 1) - y] == true) // only check slabs in the roomspace
                    {
                        can = (sell) ? ((subtile_is_sellable_room(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0))) || (subtile_is_sellable_door_or_trap(roomspace->plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0)))) : (roomspace_can_build_room_at_slab(roomspace->plyr_idx, roomspace->rkind, current_x, current_y));
                        if (can)
                        {
                            row[(roomspace->height - 1) - y] = true;
                            roomspace->slab_count++;
                        }
                        else
                        {
                            row[(roomspace->height - 1) - y] = false;
                            roomspace->invalid_slabs_count++;
                        }
                    }
                }
            }
            break;
        }
    }
}

TbBool roomspace_can_build_room_at_slab(PlayerNumber plyr_idx, RoomKind rkind, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (room_role_matches(player->chosen_room_kind,RoRoF_PassLava|RoRoF_PassWater))
    {
        if (!subtile_revealed(slab_subtile_center(slb_x), slab_subtile_center(slb_y), plyr_idx))
        {
            return false;
        }
        if (!slab_is_liquid(slb_x, slb_y))
        {
            return false;
        }
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        if(slb->kind == SlbT_WATER && !room_role_matches( rkind,RoRoF_PassWater))
        {
            return false;
        }
        if(slb->kind == SlbT_LAVA && !room_role_matches( rkind,RoRoF_PassLava))
        {
            return false;
        }
        if (player->roomspace_horizontal_first)
        {
            if (roomspace_liquid_path_is_blocked(plyr_idx, player->render_roomspace.drag_start_x, slb_x, player->render_roomspace.drag_start_y, 0))
            {
                return false;
            }
            if (roomspace_liquid_path_is_blocked(plyr_idx, player->render_roomspace.drag_start_y, slb_y, player->render_roomspace.drag_end_x, 1))
            {
                return false;
            }
        }
        else
        {
            if (roomspace_liquid_path_is_blocked(plyr_idx, player->render_roomspace.drag_start_y, slb_y, player->render_roomspace.drag_start_x, 1))
            {
                return false;
            }
            if (roomspace_liquid_path_is_blocked(plyr_idx, player->render_roomspace.drag_start_x, slb_x, player->render_roomspace.drag_end_y, 0))
            {
                return false;
            }
        }
        switch (player->roomspace_l_shape)
        {
            case 0:
            {
                if (slb_y != player->render_roomspace.drag_start_y)
                {
                    return (slb_x == player->render_roomspace.drag_end_x);
                }
                break;
            }
            case 1:
            {
                if (slb_x != player->render_roomspace.drag_start_x)
                {
                    return (slb_y == player->render_roomspace.drag_end_y);
                }
                break;
            }
        }
        return true;
    }
    else
    {
        return (can_build_room_at_slab(plyr_idx, rkind, slb_x, slb_y));
    }
}

void detect_roomspace_direction(struct RoomSpace *roomspace)
{
    if (roomspace->drag_start_y > roomspace->drag_end_y)
    {
        if (roomspace->drag_start_x > roomspace->drag_end_x)
        {
            roomspace->drag_direction = bottom_right_to_top_left;
        }
        else
        {
            roomspace->drag_direction = bottom_left_to_top_right;
        }
    }
    else
    {
        if (roomspace->drag_start_x > roomspace->drag_end_x)
        {
            roomspace->drag_direction = top_right_to_bottom_left;
        }
        else
        {
            roomspace->drag_direction = top_left_to_bottom_right;
        }
    }
}

void detect_bridge_shape(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    if (player->render_roomspace.drag_end_x != player->render_roomspace.drag_start_x)
    {
        if (player->render_roomspace.drag_start_y == player->render_roomspace.drag_end_y)
        {
            player->roomspace_horizontal_first = true;
        } 
    }
    else if (player->render_roomspace.drag_end_y != player->render_roomspace.drag_start_y)
    {
        player->roomspace_horizontal_first = false;
    }
    if (player->roomspace_horizontal_first)
    {
        if (player->render_roomspace.drag_end_y != player->render_roomspace.drag_start_y)
        {
            player->roomspace_l_shape = 0;
        }
    }
    else
    {
        player->roomspace_l_shape = 1;
    }
}

TbBool roomspace_liquid_path_is_blocked(PlayerNumber plyr_idx, MapSlabCoord start, MapSlabCoord end, MapSlabCoord other_axis, TbBool vertical)
{
    MapSlabCoord begin = start;
    MapSlabCoord finish = end;
    if (begin <= finish)
    {
        if (vertical)
        {
            while (begin < finish)
            {
                if (roomspace_slab_blocks_bridge(plyr_idx, other_axis, begin))
                {
                    return true;
                }
                begin++;
            }
        }
        else
        {
            while (begin < finish)
            {
                if (roomspace_slab_blocks_bridge(plyr_idx, begin, other_axis))
                {
                    return true;
                }
                begin++;
            }
        }
    }
    else
    {
        if (vertical)
        {
            while (finish < begin)
            {
                if (roomspace_slab_blocks_bridge(plyr_idx, other_axis, begin))
                {
                    return true;
                }
                begin--;
            }

        }
        else
        {
            while (finish < begin)
            {
                if (roomspace_slab_blocks_bridge(plyr_idx, begin, other_axis))
                {
                    return true;
                }
                begin--;
            }
        }
    }
    return false;
}

TbBool roomspace_slab_blocks_bridge(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    if (slab_is_wall(slb_x, slb_y))
    {
        return true;
    }
    if (!slab_is_liquid(slb_x, slb_y))
    {
        struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
        if ((slabmap_owner(slb)) != plyr_idx)
        {
            return true;
        }
    }
    return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
