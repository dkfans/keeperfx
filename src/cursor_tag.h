/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file cursor_tag.h
 *     Header file for cursor_tag.c.
 * @par Purpose:
 *     Cursor box functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     10 March 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "globals.h"
#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
struct PlayerInfo;
struct Packet;
struct RoomSpace;
struct MapVolumeBox;
TbBool should_draw_cursor_box(NetUserId user);
struct MapVolumeBox *get_user_cursor_box(NetUserId user);
const struct MapVolumeBox *get_visible_co_user_cursor_box(NetUserId user);
void clear_co_user_cursor_box(NetUserId user);
unsigned char get_user_cursor_box_colour(NetUserId user, unsigned char colour);
unsigned char tag_cursor_blocks_dig(struct PlayerInfo *player, NetUserId user, const struct Packet *pckt, struct RoomSpace *render_roomspace, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool full_slab);
void tag_cursor_blocks_thing_in_hand(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_sell_area(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool full_slab);
TbBool tag_cursor_blocks_place_door(PlayerNumber plyr_idx, NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_place_room(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool full_slab);
void tag_cursor_blocks_place_terrain(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_place_thing(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_order_creature(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing* creatng);
TbBool tag_cursor_blocks_steal_slab(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_place_trap(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y, ThingModel trapmodel);
TbBool tag_cursor_blocks_for_work_state(NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
