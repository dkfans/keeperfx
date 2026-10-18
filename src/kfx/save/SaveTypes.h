/******************************************************************************/
/** @file SaveTypes.h
 *     The data types of saved games: the title record, the continue link, the results of checking and
 *     loading a save. Plain data, shared by the save manager, the field tables and the tests.
 */
/******************************************************************************/
#ifndef KFX_SAVETYPES_H
#define KFX_SAVETYPES_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/* Savegames are individual files fx1gNNNN.sav. The catalogue grows to hold exactly the saves that exist
 * plus one free slot to save into, so the only real limit is disk space. SAVE_SLOTS_LIMIT is only a
 * sanity ceiling matching the fx1g%04d filename range (0-9999).
 */
#define SAVE_SLOTS_LIMIT         10000
#define SAVE_SLOTS_MIN           8
#define SAVE_TEXTNAME_LEN        30
#define PLAYER_NAME_LENGTH       64
#define SAVE_FILENAME_MAX        64

#define MAKE_CHUNK_ID(char1, char2, char3, char4) \
    ((uint32_t)(unsigned char)(char1)         | ((uint32_t)(unsigned char)(char2) << 8) \
   | ((uint32_t)(unsigned char)(char3) << 16) | ((uint32_t)(unsigned char)(char4) << 24))

/** Chunk ids of the files written before the KFXS container. Only the importers of those files use them. */
enum SaveGameChunks {
    /** The title record (CatalogueEntry) of a savegame. */
    SGC_InfoBlock         = MAKE_CHUNK_ID('I', 'N', 'F', 'O'),
    /** The progress data (IntralevelData) of a campaign. */
    SGC_IntralevelData    = MAKE_CHUNK_ID('I', 'L', 'V', 'L'),
    /** The continue link (ContinueData). */
    SGC_Continue          = MAKE_CHUNK_ID('C', 'O', 'N', 'T'),
};

/** What the Continue button of the main menu resumes. */
enum ContinueTargets {
    /** Nothing to continue: no saved game and no campaign with progress. */
    CntT_None = 0,
    /** The last saved game. */
    CntT_SavedGame,
    /** The campaign that was played last, at the level its progress file says. */
    CntT_CampaignProgress,
};

/** Result of checking a savegame file without loading it. */
enum SaveCheckResult {
    /** This build can load the file. */
    SvChk_Loadable = 0,
    /** There is no file, or it can't be opened. */
    SvChk_Missing,
    /** The file is truncated, or a required chunk is missing. */
    SvChk_Damaged,
    /** The file was made by another build, or before the save format changed. */
    SvChk_Incompatible,
};

/** How a load ended. Refused means nothing was changed, so a running game carries on. */
enum SaveLoadOutcome {
    /** The save is loaded. */
    SvLoad_Done = 0,
    /** The save was checked and read in full first, and can't be loaded; the reason is in *why. */
    SvLoad_Refused,
    /** The save read fine but failed while being applied: the running game is half replaced. */
    SvLoad_Broken,
};
/******************************************************************************/
#pragma pack(1)

/** Flags of a catalogue entry (CatalogueEntry::flags). */
enum CatalogueEntryFlags {
    /** The slot holds a save. A free slot has no flags. */
    CEF_InUse       = 0x0001,
    /** Runtime only, never stored: the catalogue sets it for a save that can't be loaded. */
    CEF_Unloadable  = 0x0002,
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
    uint32_t reserved;

    // point to a save game file or a campaign progress file
    char link_fname[SAVE_FILENAME_MAX];
};

struct FileChunkHeader {
    uint32_t len;
    uint32_t id;
    uint32_t ver;
};

#pragma pack()
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
