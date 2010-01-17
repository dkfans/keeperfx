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
#define SAVE_TEXTNAME_LEN     15

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Game;

struct CatalogueEntry {
    char used;
    char  numfield_1;
    char textname[SAVE_TEXTNAME_LEN];
};

enum SaveGameChunks {
     SGC_GameOrig,
     SGC_GameAdd,
};

struct FileChunkHeader {
    unsigned short id;
    unsigned long len;
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT extern struct CatalogueEntry _DK_save_game_catalogue[SAVE_SLOTS_COUNT];
#define save_game_catalogue _DK_save_game_catalogue
/******************************************************************************/
extern long const VersionMajor;
extern long const VersionMinor;
/******************************************************************************/
short load_game(long slot_idx);
short save_game(long slot_idx);
short initialise_load_game_slots(void);
int count_valid_saved_games(void);
short save_version_compatible(long filesize,struct Game *header);
short is_save_game_loadable(long slot_num);
/******************************************************************************/
short save_catalogue_slot_disable(unsigned int slot_idx);
short save_game_save_catalogue(void);
short load_game_save_catalogue(void);
/******************************************************************************/
TbBool set_transfered_creature(long plyr_idx, long model, long explevel);
void clear_transfered_creature(void);
/******************************************************************************/
short continue_game_available(void);
short load_continue_game(void);
short save_continue_game(long lv_num);
short read_continue_game_part(unsigned char *buf,long pos,long buf_len);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
