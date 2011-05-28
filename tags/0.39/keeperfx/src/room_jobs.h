/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_jobs.h
 *     Header file for room_jobs.c.
 * @par Purpose:
 *     List of things in room maintain functions.
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
#ifndef DK_ROOM_JOBS_H
#define DK_ROOM_JOBS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Thing;
struct Room;
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

TbBool add_creature_to_work_room(struct Thing *crtng, struct Room *room);
TbBool add_creature_to_torture_room(struct Thing *crtng, struct Room *room);
TbBool remove_creature_from_specific_room(struct Thing *crtng, struct Room *room);
TbBool remove_creature_from_work_room(struct Thing *thing);

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
