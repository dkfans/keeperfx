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
#include "game_legacy.h"
#include "kjm_input.h"
#include "front_input.h"
#include "player_utils.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
int user_defined_roomspace_width = DEFAULT_USER_ROOMSPACE_WIDTH;
int roomspace_detection_looseness = DEFAULT_USER_ROOMSPACE_DETECTION_LOOSENESS;
struct RoomSpace render_roomspace = { {{false}}, 1, true, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, false, true };
/******************************************************************************/
TbBool can_afford_roomspace(PlayerNumber plyr_idx, RoomKind rkind, int slab_count)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct RoomStats* rstat = room_stats_get_for_kind(rkind);
    return (slab_count * rstat->cost <= dungeon->total_money_owned);
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

struct RoomSpace check_slabs_in_roomspace(struct RoomSpace roomspace, PlayerNumber plyr_idx, RoomKind rkind, short rkind_cost)
{
    roomspace.slab_count = 0;
    roomspace.invalid_slabs_count = 0;
    for (int y = 0; y < roomspace.height; y++)
    {
        int current_y = roomspace.top + y;
        for (int x = 0; x < roomspace.width; x++)
        {
            int current_x = roomspace.left + x;
            if (roomspace.is_roomspace_a_box || roomspace.slab_grid[x][y] == true) // only check slabs in the roomspace
            {
                if (can_build_room_at_slab(plyr_idx, rkind, current_x, current_y))
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
    }
    roomspace.total_roomspace_cost = roomspace.slab_count * rkind_cost;
    if (roomspace.slab_count != (roomspace.width * roomspace.height))
    {
        roomspace.is_roomspace_a_box = false;
        roomspace.render_roomspace_as_box = false;
    }
    if ((roomspace.slab_count == 0) || (roomspace.slab_count > MAX_USER_ROOMSPACE_WIDTH * MAX_USER_ROOMSPACE_WIDTH))
    {
        roomspace = create_box_roomspace(roomspace, 1, 1, roomspace.centreX, roomspace.centreY);
    }
    return roomspace;
}

struct RoomSpace check_roomspace_for_sellable_slabs(struct RoomSpace roomspace, PlayerNumber plyr_idx)
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
            if ((subtile_is_sellable_room(plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0))) || (subtile_is_sellable_door_or_trap(plyr_idx, slab_subtile(current_x,0), slab_subtile(current_y,0))))
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
    struct RoomSpace default_room = { {{false}}, 0, true, 1, 1, cursor_x, cursor_y, cursor_x, cursor_y, cursor_x, cursor_y, 0, 0, current_plyr_idx, RoK_SELL, false, 0, 0, false, true };
    
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
        value =4;
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

void reset_dungeon_build_room_ui_variables()
{
    roomspace_detection_looseness = DEFAULT_USER_ROOMSPACE_DETECTION_LOOSENESS;
    user_defined_roomspace_width = DEFAULT_USER_ROOMSPACE_WIDTH;
}

void get_dungeon_sell_user_roomspace(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long keycode = 0;
    struct PlayerInfo* player = get_player(plyr_idx);
    int width = 1, height = 1;
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    struct RoomSpace current_roomspace;
    if (is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true))
    {
        current_roomspace = get_current_room_as_roomspace(player->id_number, slb_x, slb_y);
        if (!current_roomspace.is_roomspace_a_box)
        {
            current_roomspace.render_roomspace_as_box = false;
        }
    }
    else 
    {
        if (is_game_key_pressed(Gkey_SquareRoomSpace, &keycode, true)) // Define square room (mouse scroll-wheel changes size - default is 5x5)
        {
            if (is_game_key_pressed(Gkey_RoomSpaceIncSize, &keycode, true))
            {
                if (user_defined_roomspace_width != MAX_USER_ROOMSPACE_WIDTH)
                {
                    user_defined_roomspace_width++;
                }
            }
            if (is_game_key_pressed(Gkey_RoomSpaceDecSize, &keycode, true))
            {
                if (user_defined_roomspace_width != MIN_USER_ROOMSPACE_WIDTH)
                {
                    user_defined_roomspace_width--;
                }
            }
            width = height = user_defined_roomspace_width;
        }
        else
        {
            reset_dungeon_build_room_ui_variables();
            width = height = numpad_to_value(false);
            
        }
        current_roomspace = create_box_roomspace(render_roomspace, width, height, slb_x, slb_y);
        current_roomspace = check_roomspace_for_sellable_slabs(current_roomspace, plyr_idx);
    }
    player->boxsize = current_roomspace.slab_count;
    render_roomspace = current_roomspace;
}

void get_dungeon_build_user_roomspace(PlayerNumber plyr_idx, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y, int *mode, TbBool drag_check)
{
    long keycode = 0;
    struct PlayerInfo* player = get_player(plyr_idx);
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    int width = 1, height = 1; // 1x1 slabs
    struct DungeonAdd *dungeonadd = get_dungeonadd(player->id_number);
    dungeonadd->painter_build_mode = 0;
    if (rkind == RoK_BRIDGE)
    {
        reset_dungeon_build_room_ui_variables();
        if (drag_check) // Enable "paint mode" if Ctrl or Shift are held
        {
            dungeonadd->painter_build_mode = 1; // Enable GuiLayer_OneClickBridgeBuild layer
            (*mode) = drag_placement_mode;
        }
    }
    else if (is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true)) // Find "best" room
    {
        if (is_game_key_pressed(Gkey_RoomSpaceIncSize, &keycode, true))
        {
            if (roomspace_detection_looseness < tolerate_gold && roomspace_detection_looseness >=disable_tolerance_layers)
            {
                roomspace_detection_looseness = tolerate_gold;
            }
            else if (roomspace_detection_looseness != tolerate_rock)
            {
                roomspace_detection_looseness = tolerate_rock;
            }
        }
        if (is_game_key_pressed(Gkey_RoomSpaceDecSize, &keycode, true))
        {
            if (roomspace_detection_looseness == tolerate_rock)
            {
                roomspace_detection_looseness = tolerate_gold;
            }
            else if (roomspace_detection_looseness != disable_tolerance_layers)
            {
                roomspace_detection_looseness = disable_tolerance_layers;
            }
        }
        (*mode) = roomspace_detection_mode;
    }
    else if (is_game_key_pressed(Gkey_SquareRoomSpace, &keycode, true)) // Define square room (mouse scroll-wheel changes size - default is 5x5)
    {
        if (is_game_key_pressed(Gkey_RoomSpaceIncSize, &keycode, true))
        {
            if (user_defined_roomspace_width != MAX_USER_ROOMSPACE_WIDTH)
            {
                user_defined_roomspace_width++;
            }
        }
        if (is_game_key_pressed(Gkey_RoomSpaceDecSize, &keycode, true))
        {
            if (user_defined_roomspace_width != MIN_USER_ROOMSPACE_WIDTH)
            {
                user_defined_roomspace_width--;
            }
        }
        width = height = user_defined_roomspace_width;
    }
    else
    {
        reset_dungeon_build_room_ui_variables();
        width = height = numpad_to_value(false);
    }

    struct RoomSpace best_roomspace;
    best_roomspace.is_roomspace_a_box = true;
    best_roomspace.render_roomspace_as_box = true;
    struct RoomStats* rstat = room_stats_get_for_kind(rkind);
    best_roomspace.plyr_idx = plyr_idx;
    best_roomspace.rkind = rkind;
    if ((*mode) == roomspace_detection_mode) // room auto-detection mode
    {
        best_roomspace = get_biggest_roomspace(plyr_idx, rkind, slb_x, slb_y, rstat->cost, 0, 32, roomspace_detection_looseness);
        width = best_roomspace.width;
        height = best_roomspace.height;
        slb_x = best_roomspace.centreX;
        slb_y = best_roomspace.centreY;
        player->boxsize = best_roomspace.slab_count; // correct number of tiles always returned from get_biggest_roomspace
    }
    else if (width == 1 && height == 1)
    {
        player->boxsize = can_build_roomspace_of_dimensions(plyr_idx, rkind, slb_x, slb_y, width, height, true); //number of slabs to build, corrected for blocked tiles
        best_roomspace = create_box_roomspace(best_roomspace, width, height, slb_x, slb_y);
    }
    else
    {
        struct RoomSpace temp_best_room = create_box_roomspace(best_roomspace, width, height, slb_x, slb_y);
        temp_best_room = check_slabs_in_roomspace(temp_best_room, plyr_idx, rkind, rstat->cost);
        best_roomspace = temp_best_room;
        player->boxsize = best_roomspace.slab_count; // correct number of tiles returned from check_slabs_in_roomspace
            // Make sure the "outer box" bounding is drawn with square room mode
            best_roomspace.width = width;
            best_roomspace.height = height;
            best_roomspace.render_roomspace_as_box = true;
    }
    render_roomspace = best_roomspace; // make sure we can render the correct boundbox to the user
}

static void sell_at_point(struct RoomSpace *roomspace)
{
    struct SlabMap *slb = get_slabmap_block(roomspace->buildx, roomspace->buildy);
    if (slabmap_owner(slb) == roomspace->plyr_idx)
    {
        if (subtile_is_sellable_room(roomspace->plyr_idx,slab_subtile(roomspace->buildx,0), slab_subtile(roomspace->buildy,0)))// Trying to sell room
        {
            player_sell_room_at_subtile(roomspace->plyr_idx,slab_subtile(roomspace->buildx,0), slab_subtile(roomspace->buildy,0));
        }
        else if (player_sell_door_at_subtile(roomspace->plyr_idx, slab_subtile(roomspace->buildx,0), slab_subtile(roomspace->buildy,0))) // Trying to sell door
        {
            // Nothing to do here - door already sold
        }
        else if (player_sell_trap_at_subtile(roomspace->plyr_idx, slab_subtile_center(roomspace->buildx), slab_subtile_center(roomspace->buildy))) // Trying to sell trap
        {
            // Nothing to do here - trap already sold
        }
    }
}
static void find_next_point(struct RoomSpace *roomspace)
{
    // these store the coordinates of roomspace.slab_grid[][], rather than the in-game map coordinates
    int room_x = roomspace->buildx - roomspace->left;
    int room_y = roomspace->buildy - roomspace->top;
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
}

void keeper_sell_roomspace(struct RoomSpace *roomspace)
{
    struct DungeonAdd *dungeonadd = get_dungeonadd(roomspace->plyr_idx);
    if (dungeonadd->roomspace.is_active)
    {
        ERRORLOG("Building roomspace while it is still in progress plyr:%d", roomspace->plyr_idx);
        return;
    }
    roomspace->rkind = RoK_SELL;
    memcpy(&dungeonadd->roomspace, roomspace, sizeof(dungeonadd->roomspace));
    // Init
    dungeonadd->roomspace.is_active = true;
    dungeonadd->roomspace.buildx = roomspace->left;
    dungeonadd->roomspace.buildy = roomspace->top;
    if (!roomspace->is_roomspace_a_box)
    {
        // We want to find first point
        find_next_point(&dungeonadd->roomspace);
    }
}

void keeper_build_roomspace(struct RoomSpace *roomspace)
{
    struct DungeonAdd *dungeonadd = get_dungeonadd(roomspace->plyr_idx);
    if (dungeonadd->roomspace.is_active)
    {
        ERRORLOG("Building roomspace while it is still in progress plyr:%d", roomspace->plyr_idx);
        return;
    }
    memcpy(&dungeonadd->roomspace, roomspace, sizeof(dungeonadd->roomspace));
    // Init
    dungeonadd->roomspace.is_active = true;
    dungeonadd->roomspace.buildx = roomspace->left;
    dungeonadd->roomspace.buildy = roomspace->top;
    if (!roomspace->is_roomspace_a_box)
    {
        dungeonadd->roomspace.buildx--; // We want to find first point
        find_next_point(&dungeonadd->roomspace);
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
        keeper_build_room(slab_subtile(roomspace->buildx, 0), slab_subtile(roomspace->buildy, 0),
                          roomspace->plyr_idx, roomspace->rkind);
    }
    // find next point
    roomspace->buildx++;
    if (roomspace->buildx > roomspace->right)
    {
        roomspace->buildx = roomspace->left;
        roomspace->buildy++;
    }
    if (!roomspace->is_roomspace_a_box)
    {
        find_next_point(roomspace);
    }

    if ((roomspace->buildy > roomspace->bottom) || (roomspace->buildx > roomspace->right))
    {
        roomspace->is_active = false;
        return;
    }
}

void update_roomspaces()
{
    for (PlayerNumber plyr_idx = 0; plyr_idx < DUNGEONS_COUNT; plyr_idx++)
    {
        if (get_player(plyr_idx)->is_active)
        {
            keeper_update_roomspace(&get_dungeonadd(plyr_idx)->roomspace);
        }
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
