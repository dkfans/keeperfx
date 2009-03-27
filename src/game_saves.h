/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_saves.h
 *     Header file for game_saves.c.
 * @par Purpose:
 *     Saved games maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 Jan 2009 - 25 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_GAMESAVE_H
#define DK_GAMESAVE_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define SAVE_SLOTS_COUNT       8

struct CatalogueEntry {
    char used;
    char  numfield_1;
    char textname[15];
};

/******************************************************************************/
DLLIMPORT extern struct CatalogueEntry _DK_save_game_catalogue[SAVE_SLOTS_COUNT];
#define save_game_catalogue _DK_save_game_catalogue
/******************************************************************************/
short load_game(long num);
short save_game_save_catalogue(struct CatalogueEntry *game_catalg);
void load_game_save_catalogue(struct CatalogueEntry *game_catalg);
short initialise_load_game_slots(void);
int count_valid_saved_games(void);
short save_version_compatible(long filesize,struct Game *header);
short is_save_game_loadable(long num);
/******************************************************************************/
short continue_game_available(void);
void update_continue_game(void);
short read_continue_game_part(unsigned char *buf,long pos,long buf_len);
void frontend_save_continue_game(long lv_num, short is_new_lvl);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
