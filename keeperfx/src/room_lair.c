/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_lair.c
 *     Lair room and creature lairs maintain functions.
 * @par Purpose:
 *     Functions to create and use lairs.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_lair.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_calculate_free_lair_space(struct Dungeon * dungeon);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long calculate_free_lair_space(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    return _DK_calculate_free_lair_space(dungeon);
}
/******************************************************************************/
