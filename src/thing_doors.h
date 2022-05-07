/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_doors.h
 *     Header file for thing_doors.c.
 * @par Purpose:
 *     XXXX functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_DOORS_H
#define DK_THING_DOORS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define DOOR_TYPES_COUNT        5
/******************************************************************************/
#pragma pack(1)

enum DoorStates {
    DorSt_Unused = 0,
    DorSt_Open,
    DorSt_Closed,
    DorSt_Opening,
    DorSt_Closing,
    DorSt_Unknown05,
};

struct Thing;

struct DoorStats { // sizeof = 8
    unsigned short slbkind;
    long health;
    unsigned short open_speed;
};

/******************************************************************************/

#pragma pack()
/******************************************************************************/
TbBool subtile_has_door_thing_on(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_has_door_thing_on_for_trap_placement(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_has_locked_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool slab_row_has_door_thing_on(MapSlabCoord slb_x, MapSubtlCoord stl_y);
TbBool slab_column_has_door_thing_on(MapSubtlCoord stl_x, MapSlabCoord slb_y);

struct Thing *create_door(struct Coord3d *pos, ThingModel tngmodel, unsigned char orient, PlayerNumber plyr_idx, TbBool is_locked);
TbBool thing_is_deployed_door(const struct Thing *thing);
void lock_door(struct Thing *thing);
void unlock_door(struct Thing *thing);
long destroy_door(struct Thing *thing);
TngUpdateRet process_door(struct Thing *thing);

char find_door_angle(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
char get_door_orientation(MapSlabCoord slb_x, MapSlabCoord slb_y);

TbBool player_has_deployed_door_of_model(PlayerNumber owner, int model, short locked);
long count_player_deployed_doors_of_model(PlayerNumber owner, int model);
TbBool player_has_deployed_trap_of_model(PlayerNumber owner, int model);
long count_player_deployed_traps_of_model(PlayerNumber owner, int model);

void update_all_door_stats();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
