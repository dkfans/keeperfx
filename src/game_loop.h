/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_legacy.h
 *     Header file for game_loop.c.
 * @author   Loobinex
 * @date     14 Jul 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_GAMELOOP_H
#define DK_GAMELOOP_H

#include "bflib_basics.h"
#include "keeperfx.hpp"
#include "game_legacy.h"
#include "game_merge.h"
#include "globals.h"
#include "thing_effects.h"


#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void process_dungeon_destroy(struct Thing* heartng);
void update_manufacturing(void);
void update_research(void);
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
