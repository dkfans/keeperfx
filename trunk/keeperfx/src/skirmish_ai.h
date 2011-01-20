/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai.h
 *     Header file for skirmish_ai.c
 * @par Purpose:
 *     Experimental computer player intended to play multiplayer maps better.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef SKIRMISH_AI_H
#define SKIRMISH_AI_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Resets Skirmish AI for a player. Should be called both as init and cleanup.
 * @param plyr Index of player.
 */
void SAI_reset_for_player(int plyr);

/**
 * Runs Skirmish AI for a player.
 * @param plyr Index of player.
 */
void SAI_run_for_player(int plyr);



#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_H
