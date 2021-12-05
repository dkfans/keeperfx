/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace_detection.c
 *     Function for finding the best "room space" based on the player's 
 *     current cursor position.
 * @par Purpose:
 *     Establishes an algorithm to search the area surrounding the cursor for 
 *     square/rectangular areas that could contain the chosen room type.
 *     A composite of the several boxes is then returned as a "room space".
 * @par Comment:
 *     None.
 * @author   Ed Kearney
 * @date     28 Aug 2020 - 07 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "slab_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
//variable init
struct RoomQuery new_room_query;
/******************************************************************************/
//functions
void test_box_roomspaces_from_biggest_to_smallest(struct RoomQuery *room_query)
{
    TbBool findCorridors = room_query->findCorridors;
    struct RoomSpace current_biggest_room = room_query->best_room;
    MapSlabCoord centre_x = room_query->centre_x, centre_y = room_query->centre_y;
    MapSlabCoord cursor_x = room_query->cursor_x, cursor_y = room_query->cursor_y;
    int max_width = room_query->maxRoomWidth;
    int min_width, min_height;
    if (findCorridors)
    {
        max_width += 1; //(+ 1 to check for corridors)// Check for 10x10, so as to detect corridors
        int distance_from_cursor_x = max(cursor_x, centre_x) - min(cursor_x, centre_x);
        int distance_from_cursor_y = max(cursor_y, centre_y) - min(cursor_y, centre_y);
        distance_from_cursor_x = min(distance_from_cursor_x, max_width);
        distance_from_cursor_y = min(distance_from_cursor_y, max_width);
        min_width = max(room_query->minRoomWidth, ((distance_from_cursor_x * 2)));
        min_height = max(room_query->minRoomHeight, ((distance_from_cursor_y * 2)));
    }
    else // we have found a corridor, and want to find the nearest square room to the cursor that fits in the found corridor
    {
        min_width = min_height = max_width;
    }
    float minimumRatio = room_query->minimumRatio;
    float minimumComparisonRatio = room_query->minimumComparisonRatio;
    struct RoomSpace best_corridor = room_query->best_corridor;
    int roomarea = 0;

    //don't check for rooms when they can't be found
    if ((room_query->mode & 2) == 2)
    {
        if ((check_room_at_slab_loose(room_query->plyr_idx, room_query->rkind, centre_x, centre_y, room_query->roomspace_discovery_looseness)  + room_query->leniency) <= 0)
        {
            return;
        }
    }
    else if ((can_build_room_at_slab_fast(room_query->plyr_idx, room_query->rkind, centre_x, centre_y) + room_query->leniency) <= 0)
    {
        return;
    }

    // for a tile, with a given X (centre_x) and Y (centre_y) coordinate :- loop through the room sizes, from biggest width/height to smallest width/height
    for (int w = max_width; w >= min_width; w--)
    {
        if ((w * max_width) < current_biggest_room.slab_count) // || (findCorridors && ((w * max_width) < best_corridor.slab_count)))
        {   // sanity check, to stop pointless iterations of the loop
            break; 
        }

        for (int h = max_width; h >= min_height; h--)
        {
            if ((w * h) < current_biggest_room.slab_count) // || (findCorridors && ((w * h) < best_corridor.slab_count)))
            {   // sanity check, to stop pointless iterations of the loop
                break;
            }
            int slabs = w * h;
            // check aspect ratio of the new room, and if the room w/h is = 10
            if ((((min(w,h) * 1.0) / (max(w,h) * 1.0)) < minimumComparisonRatio) || (max(w,h) >= 10))
            {   // this is a corridor
                if (!findCorridors || (slabs <= best_corridor.slab_count))
                {   // skip small corridors (this is trigger most of the time, as we either have a large corridor (1st one found) or we are not finding any corridors.
                    continue;
                }
            }
            // get the extents of the current room
            int leftExtent   = centre_x - calc_distance_from_roomspace_centre(w,0);
            int rightExtent  = centre_x + calc_distance_from_roomspace_centre(w,(w % 2 == 0));
            int topExtent    = centre_y - calc_distance_from_roomspace_centre(h,0);
            int bottomExtent = centre_y + calc_distance_from_roomspace_centre(h,(h % 2 == 0));
            // check if cursor is not in the current room
            if (!(cursor_x >= leftExtent && cursor_x <= rightExtent && cursor_y >= topExtent && cursor_y <= bottomExtent))
            {
                continue; // not a valid room
            }
            // check to see if the room collides with any walls (etc)
            int invalid_slabs = 0;
            if ((room_query->mode & 2) == 2)
            {
                roomarea = can_build_roomspace_of_dimensions_loose(room_query->plyr_idx, room_query->rkind, centre_x, centre_y, w, h, &invalid_slabs, room_query->roomspace_discovery_looseness);
            }
            else
            {
                roomarea = can_build_roomspace_of_dimensions(room_query->plyr_idx, room_query->rkind, centre_x, centre_y, w, h, false);
            }
            if (roomarea >= (slabs - room_query->leniency))
            {
                // check aspect ratio of the new room, and if the room w/h is = 10
                if ((((min(w,h) * 1.0) / (max(w,h) * 1.0)) < minimumComparisonRatio) || ((max(w,h) >= 10)))
                { 
                    // this is a corridor
                    if (roomarea > best_corridor.slab_count)
                    {
                        best_corridor.slab_count = roomarea;
                        best_corridor.centreX = centre_x;
                        best_corridor.centreY = centre_y;
                        best_corridor.width = w;
                        best_corridor.height = h;
                        best_corridor.left = leftExtent;
                        best_corridor.right = rightExtent;
                        best_corridor.top = topExtent;
                        best_corridor.bottom = bottomExtent;
                        best_corridor.is_roomspace_a_box = true;
                    }
                    break;
                }
                //else if (((roomarea * room_query->rkind_cost) <= room_query->moneyLeft) && (((min(w,h) * 1.0) / (max(w,h) * 1.0)) >= minimumRatio))
                else if (((min(w,h) * 1.0) / (max(w,h) * 1.0)) >= minimumRatio)
                { 
                    TbBool isCorridorHorizontal = (best_corridor.width >= best_corridor.height);
                    TbBool isInCorridor = isCorridorHorizontal ? (topExtent >= best_corridor.top && bottomExtent <= best_corridor.bottom) : (leftExtent >= best_corridor.left && rightExtent <= best_corridor.right);
                    if (!isInCorridor || !findCorridors)
                    {
                        // this is a room
                        if (roomarea > current_biggest_room.slab_count)
                        {
                            current_biggest_room.slab_count = roomarea;
                            current_biggest_room.invalid_slabs_count = invalid_slabs;
                            current_biggest_room.centreX = centre_x;
                            current_biggest_room.centreY = centre_y;
                            current_biggest_room.width = w;
                            current_biggest_room.height = h;
                            current_biggest_room.left = leftExtent;
                            current_biggest_room.right = rightExtent;
                            current_biggest_room.top = topExtent;
                            current_biggest_room.bottom = bottomExtent;
                            current_biggest_room.is_roomspace_a_box = true;
                            room_query->foundRoom = true;
                        }
                        break;
                    }
                }
            }
        }
    } // end loop of room sizes
    
    room_query->best_corridor = best_corridor;
    room_query->best_room = current_biggest_room;
    if (findCorridors && best_corridor.width > 1 && best_corridor.height > 1)
    {
        TbBool isCorridorAreaSmaller = (best_corridor.slab_count < current_biggest_room.slab_count);
        TbBool isCorridorHorizontal = (best_corridor.width >= best_corridor.height);
        TbBool isCorridorHeightSmaller = (min(best_corridor.width, best_corridor.height) < (isCorridorHorizontal ? current_biggest_room.height : current_biggest_room.width));
        room_query->isCorridor = (isCorridorHeightSmaller || isCorridorAreaSmaller) ? false : true; // small 2xA corridors might be found as part of a composite room check, these are ignored along with any other corridors (and any potential subrooms that might be in that corridor)
        if (room_query->isCorridor)
        {
            room_query->foundRoom = false;
        }
    }
}

void find_roomspace_within_radius(struct RoomQuery *room_query)
{
    // Loop through all of the tiles in a search area, and then test for rooms centred on each of these tiles.

    int direction = 0; // current direction; 0=RIGHT, 1=DOWN, 2=LEFT, 3=UP
    int tile_counter = 0; // the number of tiles that have been processed
    int chain_size = 1; // a spiral is constructed out of chains, increasing in size around the centre
    int searchWidth = room_query->maxRoomWidth + (room_query->maxRoomWidth % 2 == 0);
    int max_count = searchWidth * searchWidth; // total number of tiles to iterate over
    int chain_position = 0; // position along the current chain
    int chain_iterations = 0; // every 2 iterations, the chain size is increased

    // starting point (centre of area - start at current cursor position)
    int x = room_query->cursor_x; // current position; x
    int y = room_query->cursor_y; // current position; y

    do
    {
        tile_counter++;
        room_query->centre_x = x;
        room_query->centre_y = y;
        test_box_roomspaces_from_biggest_to_smallest(room_query);
        if (chain_position < chain_size) // move along the current chain
        {
            chain_position++;
        }
        else // we need to rotate and start the next chain
        {
            chain_position = 1; // only use 0 to place the first centre tile
            chain_iterations++;
            if (chain_iterations % 2 == 0) // 2 chains of movement of the same length, then increase chain length by 1
            {
                chain_size++;
            }
            direction = (direction + 1) % 4; // switch direction (Clockwise rotation)
        }
        switch (direction) // increase x/y based on direction of the chain
        {
            case 0: y = y + 1; break; // moving Right
            case 1: x = x + 1; break; // moving Down
            case 2: y = y - 1; break; // moving Left
            case 3: x = x - 1; break; // moving Up
        }
    } while (tile_counter < max_count); // loop through every tile in the search area
}

void add_to_composite_roomspace(struct RoomQuery *room_query, struct RoomQuery *meta_room)
{
    // find the extents of the meta room
    int minX = meta_room->best_room.left;
    int maxX = meta_room->best_room.right;
    int minY = meta_room->best_room.top;
    int maxY = meta_room->best_room.bottom;
    if (((maxX - minX) < MAX_ROOMSPACE_WIDTH) && (room_query->best_room.left < minX))
    {
        minX = room_query->best_room.left;
    }
    if (((maxX - minX) < MAX_ROOMSPACE_WIDTH) && (room_query->best_room.right > maxX))
    {
        maxX = room_query->best_room.right;
    }
    if (((maxY - minY) < MAX_ROOMSPACE_WIDTH) && (room_query->best_room.top < minY))
    {
        minY = room_query->best_room.top;
    }
    if (((maxY - minY) < MAX_ROOMSPACE_WIDTH) && (room_query->best_room.bottom > maxY))
    {
        maxY = room_query->best_room.bottom;
    }
    int metaRoomWidth = (maxX - minX + 1);
    int metaRoomHeight = (maxY - minY + 1);
    // idiot check for empty room
    if ((metaRoomWidth * metaRoomHeight) <= 1) 
    {
        return;
    }
    // else, populate best_room
    struct RoomSpace best_room = meta_room->best_room;
    best_room.width = metaRoomWidth;
    best_room.height = metaRoomHeight;
    best_room.centreX = minX + ((best_room.width - 1 - (best_room.width % 2 == 0)) / 2);
    best_room.centreY = minY + ((best_room.height - 1 - (best_room.height % 2 == 0)) / 2);
    best_room.left = minX;
    best_room.right = maxX;
    best_room.top = minY;
    best_room.bottom = maxY;
    best_room.slab_count = 0;
    best_room.is_roomspace_a_box = true;
    // loop through all of the tiles within the extents of the meta room, and check if it is found in the current sub room
    for (int y = 0; y < best_room.height; y++)
    {
        int current_y = minY + y;
        for (int x = 0; x < best_room.width; x++)
        {
            int current_x = minX + x;
            best_room.slab_grid[x][y] = false; // set to false by default
            TbBool isSlabInMetaRoom = (current_x >= meta_room->best_room.left && current_x <= meta_room->best_room.right && current_y >= meta_room->best_room.top && current_y <= meta_room->best_room.bottom);
            TbBool isSlabInNewRoom = (current_x >= room_query->best_room.left && current_x <= room_query->best_room.right && current_y >= room_query->best_room.top && current_y <= room_query->best_room.bottom);
            if (isSlabInMetaRoom)
            {
                best_room.slab_grid[x][y] = meta_room->best_room.slab_grid[current_x-meta_room->best_room.left][current_y-meta_room->best_room.top];
            }
            if (isSlabInNewRoom && (best_room.slab_grid[x][y] == false))
            {
                best_room.slab_grid[x][y] = true;
            }
             best_room.slab_count += best_room.slab_grid[x][y];
        }
    }
    if (best_room.slab_count != (best_room.width * best_room.height))
    {
        best_room.is_roomspace_a_box = false;
    }
    meta_room->best_room = best_room;
}

void find_composite_roomspace(struct RoomQuery *room_query)
{
    //struct RoomQuery bestRooms[room_query.subRoomCheckCount];
    new_room_query = (*room_query);
    struct RoomQuery meta_room = new_room_query;
    int bestRoomsCount = 0;
    int mode = room_query->mode;
    int subRoomCheckCount = room_query->subRoomCheckCount;

    // Find the biggest room
    // loop through the room_query.subRoomCheckCount sub rooms, that are used to construct the meta room (mode 32 only, otherwise it only loops once)
    do
    {
        find_roomspace_within_radius(&new_room_query);
        //room_query->maxRoomRadius = room_query->maxRoomRadius - 1;
        //room_query->maxRoomWidth = (room_query->maxRoomRadius * 2) - 1;
        //new_room_query.findCorridors = false;
        //new_room_query.bestRoomsCount = bestRoomsCount;
        if (((mode & 32) == 32) && (bestRoomsCount < subRoomCheckCount))
        {
            // Adjust leniency counter
            if ((new_room_query.leniency > 0) && new_room_query.best_room.slab_count > 1) // make sure we found a subroom, and then adjust leniency allowance as needed
            {
                int usedLeniency = new_room_query.best_room.invalid_slabs_count;
                if ((usedLeniency > 0) && !new_room_query.isCorridor)
                {
                    new_room_query.leniency = room_query->leniency - usedLeniency;
                    new_room_query.InvalidBlocksIgnored += usedLeniency;
                }
            }
            if (bestRoomsCount == 0) // first time through only
            {
                meta_room = new_room_query; // establish the first room
                if (!meta_room.foundRoom && !meta_room.isCorridor)
                {
                    return; // if the first room found is a single tile, exit now
                }
                meta_room.best_room = room_query->best_room;
            }
            // add new subroom to meta room
            if (new_room_query.foundRoom)
            {
                if (!(meta_room.best_room.slab_count == new_room_query.best_room.slab_count && meta_room.best_room.width == new_room_query.best_room.width && meta_room.best_room.height == new_room_query.best_room.height))
                {
                    add_to_composite_roomspace(&new_room_query, &meta_room);
                    meta_room.isCorridor = false;
                }
            }
            bestRoomsCount++;
            if (bestRoomsCount >= subRoomCheckCount)
            {
                new_room_query = meta_room;
            }
            else
            {   // set up settings for next pass for subrooms
                //int correct_index = ((bestRoomsCount > 3) ? bestRoomsCount - 3 : 0); // we want to compare to the previously stored widest/tallest room
                if (bestRoomsCount % 3 == 1) // check for wider rooms
                {
                    new_room_query.minRoomWidth = min(room_query->maxRoomWidth,max(meta_room.best_room.width + 1, room_query->minRoomWidth));
                    new_room_query.minRoomHeight = room_query->minRoomWidth;
                    //new_room_query.minimumRatio = (1.0 / 4.0);
                    new_room_query.minimumRatio = room_query->minimumRatio;
                }
                else if (bestRoomsCount % 3 == 2)  // check for taller rooms
                {
                    new_room_query.minRoomWidth = room_query->minRoomWidth;
                    new_room_query.minRoomHeight = min(room_query->maxRoomWidth,max(meta_room.best_room.height + 1, room_query->minRoomWidth));
                    //new_room_query.minimumRatio = (1.0 / 4.0);
                    new_room_query.minimumRatio = room_query->minimumRatio;
                }
                else //check for square room (only worth running once)
                {
                    new_room_query.minRoomWidth = room_query->minRoomWidth;
                    new_room_query.minRoomHeight = room_query->minRoomWidth;
                    new_room_query.minimumRatio = 1.0;
                }
                // finish meta room checks
                new_room_query.best_room.slab_count = 1;
                new_room_query.best_room.invalid_slabs_count = 0;
                
            }
        }
    } while(((mode & 32) == 32) && (bestRoomsCount < subRoomCheckCount)); // loop again, if in mode 32, for the extra room checks
    // if the "best room" is a corridor, then grab the best AxA room in the corridor.
    if (new_room_query.isCorridor) //  if the "best room" is a corridor, then grab the best AxA room in the corridor.
    {
        new_room_query.moneyLeft = room_query->moneyLeft;
        new_room_query.maxRoomWidth = min(room_query->maxRoomWidth, min(new_room_query.best_corridor.width, new_room_query.best_corridor.height)); // don't use a width/height > room_query->maxRoomWidth anything bigger will not be a valid room (i.e it will be another corridor, we want a room in the corridor)
        new_room_query.minRoomHeight = new_room_query.minRoomWidth = new_room_query.maxRoomWidth;
        new_room_query.minimumRatio = 1.0;
        new_room_query.isCorridor = false;
        new_room_query.findCorridors = false;
        new_room_query.best_room.slab_count = 1;
        new_room_query.best_corridor.slab_count = 1;
        //new_room_query.best_corridor = room_query->best_corridor;
        /*if ((new_room_query.best_corridor.width * new_room_query.best_corridor.height) >= ((room_query->maxRoomWidth - 1) * (room_query->maxRoomWidth - 1))) // 10x10 room = open plan
        {   // so return a 5x5 room?? 9x9???
            new_room_query.maxRoomWidth = room_query->maxRoomWidth;
            new_room_query.minRoomWidth = room_query->minRoomWidth;
            new_room_query.maxRoomRadius = 0;
        }
        else
        {*/
           // grab the closest AxA room to the cursor, that is within the detected corridor
            
        //}
        
        find_roomspace_within_radius(&new_room_query);
    }
    (*room_query) = new_room_query;
}

struct RoomSpace get_biggest_roomspace(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord cursor_x, MapSlabCoord cursor_y, short rkind_cost, int total_player_money, int mode, int roomspace_discovery_looseness)
{
    int maxRoomWidth = 9; // 9x9 Room
    int minRoomWidth = 2; // Don't look for rooms smaller than 2x2
    float minimumRatio = (1.0 / 3.0);
    float minimumComparisonRatio = minimumRatio;
    int subRoomCheckCount = 6; // the number of sub-rooms to combine in to a final meta-room
    int bestRoomsCount = 0;
    // Set default "room" - i.e. 1x1 slabs, centred on the cursor
    struct RoomSpace best_room = { {{false}}, 1, true, 1, 1, cursor_x, cursor_y, cursor_x, cursor_y, cursor_x, cursor_y, rkind_cost, 0, plyr_idx, rkind, false, 0, 0, false, true, false, false, false, false, 0, 0, 0, 0, false };
    //int leniency = (((mode & 16) == 16)) ? tolerance : 0; // mode=16 :- (setting to 1 would allow e.g. 1 dirt block in the room)
    int leniency = 0;
    struct RoomSpace best_corridor = best_room;
    if (roomspace_discovery_looseness > 0)
    {
        mode |= 2;
    }
    //don't check for rooms when they can't be found
    if ((mode & 2) == 2)
    {
        int room_check = check_room_at_slab_loose(plyr_idx, rkind, cursor_x, cursor_y, roomspace_discovery_looseness);
        if (room_check == 0 || room_check == 6) //reject invalid and liquid slabs
        {
            return best_room;
        }
    }
    else if ((can_build_room_at_slab_fast(plyr_idx, rkind, cursor_x, cursor_y) + leniency) <= 0)
    {
        return best_room;
    }
    struct RoomQuery room_query = { rkind_cost, total_player_money, mode, 0, maxRoomWidth, minRoomWidth, minRoomWidth, subRoomCheckCount, bestRoomsCount, best_room, best_corridor, cursor_x, cursor_y, cursor_x, cursor_y, plyr_idx, rkind, minimumRatio, minimumComparisonRatio, false, false, leniency, total_player_money, 0, true, false, roomspace_discovery_looseness};
    find_composite_roomspace(&room_query);
    room_query.best_room = check_slabs_in_roomspace(room_query.best_room, plyr_idx, rkind, rkind_cost);
    if (room_query.best_room.slab_count > 0)
    {
        return room_query.best_room;
    }
    return best_room;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
