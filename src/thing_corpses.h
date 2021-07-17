/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_corpses.h
 *     Header file for thing_corpses.c.
 * @par Purpose:
 *     Dead creature things support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 02 Mar 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TNGCORPSES_H
#define DK_TNGCORPSES_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Dungeon;

enum DeadCreatureStates {
    DCrSt_Unused = 0,
    DCrSt_DramaticDying,
    DCrSt_RigorMortis,
};

#pragma pack()
/******************************************************************************/
TbBool corpse_is_rottable(const struct Thing *thing);
TbBool corpse_laid_to_rest(const struct Thing* thing);
TbBool corpse_ready_for_collection(const struct Thing* thing);
TbBool dead_creature_is_room_inventory(const struct Thing *thing, RoomKind rkind);
TbBool update_dead_creatures_list(struct Dungeon *dungeon, const struct Thing *thing);
TbBool update_dead_creatures_list_for_owner(const struct Thing *thing);
TbBool add_item_to_dead_creature_list(struct Dungeon *dungeon, ThingModel crmodel, long crlevel);
TbBool remove_item_from_dead_creature_list(struct Dungeon *dungeon, ThingModel crmodel, long crlevel);
TngUpdateRet update_dead_creature(struct Thing *thing);
struct Thing *create_dead_creature(const struct Coord3d *pos, ThingModel model, unsigned short a1, unsigned short owner, long explevel);
struct Thing *destroy_creature_and_create_corpse(struct Thing *thing, long crpscondition);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
