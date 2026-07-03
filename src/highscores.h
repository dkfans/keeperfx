/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file highscores.h
 *     Header file for highscores.c.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void load_or_create_high_score_table(void);
TbBool save_high_score_table(void);
int add_high_score_entry(unsigned long score, LevelNumber lvnum, const char *name);
unsigned long get_level_highest_score(LevelNumber lvnum);

#ifdef __cplusplus
}
#endif
#endif
