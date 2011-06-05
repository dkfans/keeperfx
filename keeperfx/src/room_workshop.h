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

#ifdef __cplusplus
extern "C" {
#endif

/** Count of possible manufacture types */
#define MANUFCTR_TYPES_COUNT  11
/** Maximal count of manufactured boxes of specific kind. */
#define MANUFACTURED_ITEMS_LIMIT 199

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Room;

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
TbBool add_workshop_object_to_workshop(struct Room *room);
TbBool remove_workshop_object_from_workshop(struct Room *room);
TbBool add_workshop_item(long plyr_idx, long wrkitm_class, long wrkitm_kind);
TbBool check_workshop_item_limit_reached(long plyr_idx, long wrkitm_class, long wrkitm_kind);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
