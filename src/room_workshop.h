/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_workshop.h
 *     Header file for room_workshop.c.
 * @par Purpose:
 *     Workshop room maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_WORKSHOP_H
#define DK_ROOM_WORKSHOP_H

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "thing_data.h"
#include "dungeon_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Maximal count of manufactured boxes of specific kind. */
#define MANUFACTURED_ITEMS_LIMIT 49

/******************************************************************************/
#pragma pack(1)

enum WorkshopCrateSource {
    WrkCrtS_None = 0,
    WrkCrtS_Offmap,
    WrkCrtS_Stored,
};

enum WorkshopCratesManageFlags {
    WrkCrtF_Default = 0,
    WrkCrtF_NoOffmap = 0x01,
    WrkCrtF_NoStored = 0x02,
};

#pragma pack()

/******************************************************************************/
#define manufacture_points_required(mfcr_type, mfcr_kind) manufacture_points_required_f(mfcr_type, mfcr_kind, __func__)
long manufacture_points_required_f(long mfcr_type, unsigned long mfcr_kind, const char *func_name);

#define add_workshop_item_to_amounts(plyr_idx, tngclass, tngmodel) add_workshop_item_to_amounts_f(plyr_idx, tngclass, tngmodel, __func__)
TbBool add_workshop_item_to_amounts_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, const char *func_name);
#define readd_workshop_item_to_amount_placeable(plyr_idx, tngclass, tngmodel) readd_workshop_item_to_amount_placeable_f(plyr_idx, tngclass, tngmodel, __func__)
TbBool readd_workshop_item_to_amount_placeable_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, const char *func_name);
#define remove_workshop_item_from_amount_stored(plyr_idx, tngclass, tngmodel, flags) remove_workshop_item_from_amount_stored_f(plyr_idx, tngclass, tngmodel, flags, __func__)
int remove_workshop_item_from_amount_stored_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, unsigned short flags, const char *func_name);
#define remove_workshop_item_from_amount_placeable(plyr_idx, tngclass, tngmodel) remove_workshop_item_from_amount_placeable_f(plyr_idx, tngclass, tngmodel, __func__)
TbBool remove_workshop_item_from_amount_placeable_f(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel, const char *func_name);
TbBool placing_offmap_workshop_item(PlayerNumber plyr_idx, ThingClass tngclass, ThingModel tngmodel);
TbBool check_workshop_item_limit_reached(PlayerNumber owner, ThingClass tngclass, ThingModel tngmodel);

TbBool add_workshop_object_to_workshop(struct Room *room,struct Thing *cratetng);
TbBool remove_workshop_object_from_workshop(struct Room *room,struct Thing *cratetng);
long calculate_manufacture_level(struct Dungeon* dungeon);
void set_manufacture_level(struct Dungeon *dungeon);
TbBool create_workshop_object_in_workshop_room(PlayerNumber plyr_idx, ThingClass tng_class, ThingModel tng_kind);
struct Thing *create_crate_in_workshop(struct Room *room, ThingModel cratngmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

TbBool remove_workshop_object_from_player(PlayerNumber owner, ThingModel objmodel);
long get_doable_manufacture_with_minimal_amount_available(const struct Dungeon *dungeon, int * mnfctr_class, int * mnfctr_kind);
TbBool get_next_manufacture(struct Dungeon *dungeon);
short process_player_manufacturing(PlayerNumber plyr_idx);
EventIndex update_workshop_object_pickup_event(struct Thing *creatng, struct Thing *picktng);

TbBool is_trap_buildable(PlayerNumber plyr_idx, long tngmodel);
TbBool is_door_buildable(PlayerNumber plyr_idx, long door_idx);

TbBool recreate_repositioned_crate_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
int check_crates_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void reposition_all_crates_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
void count_and_reposition_crates_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
void count_crates_in_room(struct Room *room);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
