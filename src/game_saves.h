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
/* Savegames are individual files fx1gNNNN.sav. The in-memory catalogue
 * (save_game_catalogue) grows dynamically to hold exactly the saves that exist plus
 * one free slot to save into, so the only real limit is disk space, there is no fixed
 * slot count. SAVE_SLOTS_LIMIT is only a sanity ceiling matching the fx1g%04d filename
 * range (0-9999).
 */
#define SAVE_SLOTS_LIMIT         10000
#define SAVE_SLOTS_MIN           8
#define SAVE_TEXTNAME_LEN        30
#define PLAYER_NAME_LENGTH       64
#define SAVE_FILENAME_MAX        64

#define MAKE_CHUNK_ID(char1, char2, char3, char4) \
    ((uint32_t)(unsigned char)(char1)         | ((uint32_t)(unsigned char)(char2) << 8) \
   | ((uint32_t)(unsigned char)(char3) << 16) | ((uint32_t)(unsigned char)(char4) << 24))

enum SaveGameChunks {
     SGC_InfoBlock        = MAKE_CHUNK_ID('I', 'N', 'F', 'O'),
     SGC_GameOrig         = MAKE_CHUNK_ID('O', 'L', 'D', 'S'),
     SGC_PacketHeader     = MAKE_CHUNK_ID('P', 'H', 'D', 'R'),
     SGC_PacketData       = MAKE_CHUNK_ID('P', 'C', 'K', 'T'),
     SGC_IntralevelData   = MAKE_CHUNK_ID('I', 'L', 'V', 'L'),
     SGC_LuaData          = MAKE_CHUNK_ID('L', 'U', 'A', ' '),
     SGC_Continue         = MAKE_CHUNK_ID('C', 'O', 'N', 'T'),
};

enum SaveGameChunkFlags {
     SGF_InfoBlock      = 0x0001,
     SGF_GameOrig       = 0x0002,
     SGF_PacketHeader   = 0x0100,
     SGF_PacketData     = 0x0200,
     SGF_IntralevelData = 0x0400,
     SGF_LuaData        = 0x0800,
};
#define SGF_SavedGame      (SGF_InfoBlock|SGF_GameOrig|SGF_IntralevelData|SGF_LuaData)
#define SGF_PacketStart    (SGF_PacketHeader|SGF_PacketData|SGF_InfoBlock)
#define SGF_PacketContinue (SGF_PacketHeader|SGF_PacketData|SGF_InfoBlock|SGF_GameOrig)

enum ContinueTargets {
    CntT_None = 0,
    CntT_SavedGame,
    CntT_CampaignProgress,
};

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

// file header for game saves. Also used in packet recordings.
// (Remember to bump the version number if the layout of this struct changes!)
#define CATALOGUE_ENTRY_VER 0
struct CatalogueEntry {
    unsigned short flags;
    char textname[SAVE_TEXTNAME_LEN];
    LevelNumber level_num;
    char campaign_name[LINEMSG_SIZE];
    char campaign_fname[DISKPATH_SIZE];
    char player_name[PLAYER_NAME_LENGTH];
    unsigned short game_ver_major;
    unsigned short game_ver_minor;
    unsigned short game_ver_release;
    unsigned short game_ver_build;
};

struct ContinueData {
    // point to a save game file or a campaign progress file
    char link_fname[SAVE_FILENAME_MAX];
};

struct FileChunkHeader {
    uint32_t len;
    uint32_t id;
    uint32_t ver;
};

/******************************************************************************/
extern int number_of_saved_games;

#pragma pack()
/******************************************************************************/
extern const short VersionMajor;
extern const short VersionMinor;
extern short const VersionRelease;
extern short const VersionBuild;
extern struct CatalogueEntry *save_game_catalogue;
extern long save_game_catalogue_count;
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
TbBool load_game_save_catalogue(void);
TbBool fill_game_catalogue_slot(long slot_num,const char *textname);
/******************************************************************************/
TbBool add_transfered_creature(PlayerNumber plyr_idx, ThingModel model, CrtrExpLevel exp_level, char *name);
void clear_transfered_creatures(void);
/******************************************************************************/
LevelNumber move_campaign_to_next_level(void);
LevelNumber move_campaign_to_prev_level(void);
/******************************************************************************/
TbBool continue_game_available(void);
enum ContinueTargets load_continue_game(void);
TbBool save_level_progress(LevelNumber lvnum, TbBool won);
/******************************************************************************/
struct GameCampaign;
struct CampaignsList;
TbBool campaign_progress_exists(const struct GameCampaign *campgn);
TbBool any_campaign_progress_exists(void);
void update_campaigns_progress_percent(struct CampaignsList *clist);
TbBool resume_campaign_progress(const char *cmpgn_fname);
TbBool erase_progress(const struct GameCampaign *campgn);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
