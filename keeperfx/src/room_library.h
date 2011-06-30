/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_library.h
 *     Header file for room_library.c.
 * @par Purpose:
 *     Library room maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 05 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_LIBRARY_H
#define DK_ROOM_LIBRARY_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Room;
struct Thing;

#pragma pack()
/******************************************************************************/
TbBool remove_spell_from_library(struct Room *room, struct Thing *spelltng, long new_owner);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
