/******************************************************************************/
/** @file SaveManager.h
 *     Everything the engine does with save files: savegame slots and their catalogue, the continue
 *     link and campaign progress files, and the network config and high score files.
 *     The SaveManager_* functions are the C entry points; C++ code uses GetSaveManager().
 */
/******************************************************************************/
#ifndef KFX_SAVEMANAGER_H
#define KFX_SAVEMANAGER_H

#include <stdint.h>

#include "bflib_basics.h"
#include "kfx/save/SaveTypes.h"
#include "kfx/save/core/save_codec.h"
#include "kfx/save/core/save_schema.h"

struct ConfigInfo;
struct HighScore;
struct IntralevelData;
struct GameCampaign;
struct CampaignsList;
struct PacketSaveHead;
struct Game;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Saves the running game in a slot. fill the slot's entry first (SaveManager_FillSlot). */
int SaveManager_Save(int slot);
/** Loads a slot. A refused load changes nothing; see SaveLoadOutcome. *why says why it was refused. */
enum SaveLoadOutcome SaveManager_Load(int slot, enum SaveCheckResult *why);
/** Cheap check of a slot's file: header, chunk headers and title. detail may be NULL. */
enum SaveCheckResult SaveManager_Check(int slot, char *detail, size_t detail_len);
/** Full check: also reads and decodes the whole save into scratch memory. Slow, changes nothing. */
enum SaveCheckResult SaveManager_Verify(int slot, char *detail, size_t detail_len);
TextStringId SaveManager_CheckMessage(enum SaveCheckResult result);
/** Renames a file the game can't read to "<name>.bak", so a fresh one doesn't destroy the player's data. */
int SaveManager_KeepUnreadableFile(const char *fname);

/** The catalogue: one entry per slot the menus range over, a used entry flagged CEF_InUse. */
int SaveManager_LoadCatalogue(void);
/** Loads the catalogue and counts the used entries; true when there are any. */
int SaveManager_InitialiseSlots(void);
/** Used entries, as counted by SaveManager_InitialiseSlots. */
int SaveManager_SavedGamesCount(void);
int SaveManager_SlotCount(void);
/** NULL when slot is out of range. Not valid across a reload of the catalogue. */
struct CatalogueEntry *SaveManager_Entry(int slot);
/** Fills a slot's entry from the running game, and a free entry of any kind. */
int SaveManager_FillSlot(int slot, const char *textname);
int SaveManager_FillEntry(struct CatalogueEntry *centry, const char *textname);
int SaveManager_DisableSlot(int slot);

/** The continue link (the last save or campaign played) and per-campaign progress. */
int SaveManager_ContinueAvailable(void);
enum ContinueTargets SaveManager_LoadContinue(void);
int SaveManager_SaveLevelProgress(LevelNumber lvnum, unsigned char victory_state);
int SaveManager_ResumeCampaign(const char *campaign_fname);
int SaveManager_CampaignHasProgress(const struct GameCampaign *campgn);
void SaveManager_UpdateCampaignPercents(struct CampaignsList *clist);
void SaveManager_DeleteContinueLink(void);

/** Network config file (fxconfig.net). On any read failure *cfg is left untouched. */
enum SaveResult SaveManager_ReadNetConfig(const char *fname, struct ConfigInfo *config, struct SaveError *err);
int SaveManager_WriteNetConfig(const char *fname, const struct ConfigInfo *config);

/** A campaign's high score table. The table comes back malloc'd, to be freed with free(); it is
 *  NULL when there are no entries or on any error. */
enum SaveResult SaveManager_ReadHighScores(const char *fname, struct HighScore **table, uint32_t *count,
    struct SaveError *err);
int SaveManager_WriteHighScores(const char *fname, const struct HighScore *table, uint32_t count);

/** The KFXS header of a replay (.pck) file, written to and read from an open file handle. The header is
 *  followed by the streamed turn data, which stays the replay code's own. g may be NULL to leave out the
 *  mid-level Game snapshot. */
enum SaveResult SaveManager_WriteReplayHeader(TbFileHandle file, const struct CatalogueEntry *centry,
    const struct PacketSaveHead *phead, const struct Game *game_state, struct SaveError *err);
/** Reads the header from fh, at the start of the file, into *centry and *phead. A Game snapshot, if the
 *  file has one, is decoded into *g and *g_present set; otherwise *g is untouched. On success fh is at
 *  the first byte of the turn stream, and *out_schema and *out_dec are set up and must be kept for the
 *  per-turn decodes, then freed with save_schema_free/save_decoder_free. Nothing needs freeing on failure. */
enum SaveResult SaveManager_ReadReplayHeader(TbFileHandle file, struct CatalogueEntry *centry,
    struct PacketSaveHead *phead, struct Game *game_state, TbBool *g_present, struct SaveFileSchema *out_schema,
    struct SaveDecoder *out_dec, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}

#include <vector>

class SaveManager {
public:
    bool Save(int slot);
    enum SaveLoadOutcome Load(int slot, SaveCheckResult *why);
    enum SaveCheckResult Check(int slot, char *detail, size_t detail_len);
    enum SaveCheckResult Verify(int slot, char *detail, size_t detail_len);
    static TextStringId CheckMessage(SaveCheckResult result);
    static bool KeepUnreadableFile(const char *fname);

    bool LoadCatalogue();
    bool InitialiseSlots();
    int SavedGamesCount() const { return saved_games_; }
    int SlotCount() const { return (int)slot_count_; }
    CatalogueEntry *Entry(int slot);
    bool FillSlot(int slot, const char *textname);
    static bool FillEntry(CatalogueEntry *centry, const char *textname);
    bool DisableSlot(unsigned int slot);

    bool ContinueAvailable();
    enum ContinueTargets LoadContinue();
    bool SaveLevelProgress(LevelNumber lvnum, unsigned char victory_state);
    bool ResumeCampaign(const char *campaign_fname);
    bool CampaignHasProgress(const GameCampaign *campgn);
    void UpdateCampaignPercents(CampaignsList *clist);
    void DeleteContinueLink();

    enum SaveResult ReadNetConfig(const char *fname, ConfigInfo *config, SaveError *err);
    bool WriteNetConfig(const char *fname, const ConfigInfo *config);
    enum SaveResult ReadHighScores(const char *fname, HighScore **table, uint32_t *count, SaveError *err);
    bool WriteHighScores(const char *fname, const HighScore *table, uint32_t count);

    static enum SaveResult WriteReplayHeader(TbFileHandle file, const CatalogueEntry *centry,
        const PacketSaveHead *phead, const Game *game_state, SaveError *err);
    static enum SaveResult ReadReplayHeader(TbFileHandle file, CatalogueEntry *centry, PacketSaveHead *phead,
        Game *game_state, bool *g_present, SaveFileSchema *out_schema, SaveDecoder *out_dec, SaveError *err);

private:
    bool EnsureSlot(long slot);
    int CountSavedGames();
    void UpdateLastFileAfterSave(int slot);
    enum ContinueTargets FindContinueTarget(int *slot, char *campaign_fname, size_t fname_len);
    bool LoadCampaignProgress(const GameCampaign *campgn, IntralevelData *ilvl, LevelNumber *continue_lvnum);

    std::vector<CatalogueEntry> catalogue_;
    long slot_count_ = 0;      // logical length: the slots the menus range over
    int saved_games_ = 0;
};

SaveManager &GetSaveManager();
#endif
#endif
