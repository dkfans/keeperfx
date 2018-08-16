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
#define CAMPAIGN_SAVE_SLOTS_COUNT 8
#define TOTAL_SAVE_SLOTS_COUNT    8
#define SAVE_TEXTNAME_LEN        15
#define PLAYER_NAME_LENGTH       64

enum SaveGameChunks {
     SGC_InfoBlock      = 0x4F464E49, //"INFO"
     SGC_GameOrig       = 0x53444C4F, //"OLDS"
     SGC_GameAdd        = 0x44444147, //"GADD"
     SGC_PacketHeader   = 0x52444850, //"PHDR"
     SGC_PacketData     = 0x544B4350, //"PCKT"
     SGC_IntralevelData = 0x4C564C49, //"ILVL"
};

enum SaveGameChunkFlags {
     SGF_InfoBlock      = 0x0001,
     SGF_GameOrig       = 0x0002,
     SGF_GameAdd        = 0x0004,
     SGF_PacketHeader   = 0x0100,
     SGF_PacketData     = 0x0200,
     SGF_IntralevelData = 0x0400,
};
#define SGF_SavedGame      (SGF_InfoBlock|SGF_GameOrig|SGF_GameAdd|SGF_IntralevelData)
#define SGF_PacketStart    (SGF_PacketHeader|SGF_PacketData|SGF_InfoBlock)
#define SGF_PacketContinue (SGF_PacketHeader|SGF_PacketData|SGF_InfoBlock|SGF_GameOrig|SGF_GameAdd)

enum GameLoadStatus {
    GLoad_Failed = 0,
    GLoad_SavedGame,
    GLoad_ContinueGame,
    GLoad_PacketStart,
    GLoad_PacketContinue,
};
/******************************************************************************/
#pragma pack(1)

struct Game;

enum CatalogueEntryFlags {
    CEF_InUse       = 0x0001,
};

struct CatalogueEntry {
    unsigned short flags;
    unsigned long version;
    char textname[SAVE_TEXTNAME_LEN];
    LevelNumber level_num;
    char campaign_name[LINEMSG_SIZE];
    char campaign_fname[DISKPATH_SIZE];
    char player_name[PLAYER_NAME_LENGTH];
};

struct FileChunkHeader {
    unsigned long len;
    unsigned long id;
    unsigned long ver;
};

/******************************************************************************/
//DLLIMPORT extern struct CatalogueEntry _DK_save_game_catalogue[SAVE_SLOTS_COUNT];
//#define save_game_catalogue _DK_save_game_catalogue
DLLIMPORT extern int _DK_number_of_saved_games;
#define number_of_saved_games _DK_number_of_saved_games

#pragma pack()
/******************************************************************************/
extern long const VersionMajor;
extern long const VersionMinor;
extern struct CatalogueEntry save_game_catalogue[];
/******************************************************************************/
int load_game_chunks(TbFileHandle fhandle,struct CatalogueEntry *centry);
TbBool fill_game_catalogue_entry(struct CatalogueEntry *centry,const char *textname);
TbBool save_game_chunks(TbFileHandle fhandle,struct CatalogueEntry *centry);
TbBool save_packet_chunks(TbFileHandle fhandle,struct CatalogueEntry *centry);
/******************************************************************************/
TbBool load_game(long slot_idx);
TbBool save_game(long slot_idx);
TbBool initialise_load_game_slots(void);
int count_valid_saved_games(void);
TbBool is_save_game_loadable(long slot_num);
/******************************************************************************/
TbBool save_catalogue_slot_disable(unsigned int slot_idx);
TbBool save_game_save_catalogue(void);
TbBool load_game_save_catalogue(void);
TbBool fill_game_catalogue_slot(long slot_num,const char *textname);
/******************************************************************************/
TbBool set_transfered_creature(PlayerNumber plyr_idx, ThingModel model, long explevel);
void clear_transfered_creature(void);
/******************************************************************************/
LevelNumber move_campaign_to_next_level(void);
LevelNumber move_campaign_to_prev_level(void);
/******************************************************************************/
TbBool continue_game_available(void);
short load_continue_game(void);
short save_continue_game(LevelNumber lv_num);
short read_continue_game_part(unsigned char *buf,long pos,long buf_len);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
