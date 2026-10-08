/******************************************************************************/
/** @file SaveManager.cpp
 *     The save manager: savegame slots (save, load, check) and the C entry points of everything it does.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_saves.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_campaigns.h"
#include "config_settings.h"
#include "dungeon_stats.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_compp.h"
#include "sound_manager.h"
#include "custom_sprites.h"
#include "front_simple.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "front_landview.h"
#include "front_highscore.h"
#include "front_lvlstats.h"
#include "front_network.h"
#include "lens_api.h"
#include "local_camera.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "frontmenu_ingame_map.h"
#include "gui_boxmenu.h"
#include "net_exchange_gameplay.h"
#include "packets.h"
#include "player_data.h"
#include "roomspace.h"
#include "keeperfx.hpp"
#include "api.h"
#include "lvl_filesdk1.h"
#include "lua_base.h"
#include "lua_triggers.h"
#include "moonphase.h"
#include "ariadne_update.h"
#include "replay.h"
#include "user_prefs.h"
#include "light_data.h"
#include "map_ceiling.h"
#include "map_columns.h"
#include "map_data.h"
#include "thing_creature.h"
#include "kfx/platform/IPlatform.h"
#include "post_inc.h"

#include <cstdlib>
#include <cstring>

#include <memory>

#include "kfx/save/SaveManager_Internal.h"
#include "kfx/save/core/save_bytes.h"
#include "kfx/save/core/save_config_overlay.h"
#include "kfx/save/core/save_names.h"
#include "kfx/save/core/schema/save_tables_decl.h"

namespace {

IFileSystem &Fs() { return *GetPlatform()->GetFileSystem(); }

void set_detail(char *detail, size_t detail_len, const char *text)
{
    if ((detail != nullptr) && (detail_len > 0))
        snprintf(detail, detail_len, "%s", text);
}

} // namespace

/******************************************************************************/

/**
 * Saves the game state file (savegame).
 * @note FillSlot() should be called before to fill level information.
 */
bool SaveManager::Save(int slot)
{
    if (!EnsureSlot(slot))
    {
        SAVEERR("can't save: slot index %d is out of range", slot);
        return false;
    }
    const char *fname = prepare_file_fmtpath(FGrp_Save, SAVED_GAME_FILENAME, slot);
    // Some game data lives outside the saved structs; make sure it's current first.
    light_export_system_state(&game.lightst);
    if (!savegame::Write(Fs(), fname, &catalogue_[slot]))
    {
        SAVEWARN("can't write save file \"%s\"", fname);
        return false;
    }
    UpdateLastFileAfterSave(slot);
    api_event("GAME_SAVED");
    return true;
}

enum SaveCheckResult SaveManager::Check(int slot, char *detail, size_t detail_len)
{
    if ((slot < 0) || (slot >= SAVE_SLOTS_LIMIT))
        return SvChk_Missing;
    char reason[256];
    int title_ok;
    enum SaveCheckResult result = savegame::CheckFile(Fs(), prepare_file_fmtpath(FGrp_Save, SAVED_GAME_FILENAME, slot),
        nullptr, &title_ok, reason, sizeof(reason));
    if (result != SvChk_Loadable)
        SAVEWARN("slot %d can't be loaded: %s", slot, reason);
    set_detail(detail, detail_len, reason);
    return result;
}

enum SaveCheckResult SaveManager::Verify(int slot, char *detail, size_t detail_len)
{
    enum SaveCheckResult result = Check(slot, detail, detail_len);
    if (result != SvChk_Loadable)
        return result;
    SaveError err;
    enum SaveResult file_result = savegame::Verify(Fs(), prepare_file_fmtpath(FGrp_Save, SAVED_GAME_FILENAME, slot), &err);
    if (file_result != SVR_Ok)
        set_detail(detail, detail_len, err.message);
    return savegame::Classify(file_result);
}

TextStringId SaveManager::CheckMessage(SaveCheckResult result)
{
    switch (result)
    {
    case SvChk_Incompatible:
        return GUIStr_SaveIncompatible;
    case SvChk_Damaged:
        return GUIStr_SaveDamaged;
    default:
        return GUIStr_SaveLoadFailed;
    }
}

bool SaveManager::KeepUnreadableFile(const char *fname)
{
    IFileSystem &filesystem = Fs();
    if (!filesystem.Exists(fname))
        return false;
    char bak_fname[4096];
    snprintf(bak_fname, sizeof(bak_fname), "%s.bak", fname);
    if (!filesystem.Rename(fname, bak_fname))
    {
        SAVEWARN("can't keep unreadable file \"%s\" as \"%s\"", fname, bak_fname);
        return false;
    }
    SAVELOG("kept unreadable file \"%s\" as \"%s\"", fname, bak_fname);
    return true;
}

/******************************************************************************/

/** A save read into scratch memory. Nothing in the running game changes until all of it has been read. */
struct LoadedSave {
    std::unique_ptr<Game, savebytes::FreeDeleter> game;
    std::unique_ptr<IntralevelData, savebytes::FreeDeleter> ilvl;
    CatalogueEntry centry = {};
    savegame::ReadResult read_result = {};

    ~LoadedSave() { Release(); }
    void Release()
    {
        savegame::FreeReadResult(&read_result);
        game.reset();
        ilvl.reset();
    }
};

/** Reads and decodes a KFXS save (already known to be loadable by Check) into scratch memory. */
static enum SaveCheckResult read_game_kfxs(const char *fname, struct LoadedSave *loaded_save)
{
    struct SaveError err;
    loaded_save->game.reset(static_cast<Game *>(malloc(sizeof(Game))));
    loaded_save->ilvl.reset(static_cast<IntralevelData *>(malloc(sizeof(IntralevelData))));
    if (!loaded_save->game || !loaded_save->ilvl)
    {
        SAVEWARN("out of memory reading \"%s\"", fname);
        loaded_save->Release();
        return SvChk_Missing;
    }
    enum SaveResult result = savegame::Read(Fs(), fname, &loaded_save->centry, loaded_save->game.get(), loaded_save->ilvl.get(), &loaded_save->read_result, &err);
    if (result != SVR_Ok)
    {
        SAVEWARN("can't read \"%s\": %s", fname, err.message);
        loaded_save->Release();
        return savegame::Classify(result);
    }
    return SvChk_Loadable;
}

/** Rebuilds the parts of struct Game that the save doesn't carry; they depend on the loaded map and config. */
static void rebuild_derived_game_state(void)
{
    set_map_size(game.map_tiles_x, game.map_tiles_y);
    ceiling_set_info(12, 4, 1);
    init_top_texture_to_cube_table();
    init_creature_scores();
}

/**
 * Puts a save that has been read in full into the running game. NAMS/CONF only mean anything once config
 * files are loaded for the save's own campaign and level, so that comes first. From here on the running
 * game is changed.
 */
static TbBool apply_game_kfxs(const char *fname, struct LoadedSave *loaded_save, struct CatalogueEntry *centry)
{
    const savegame::ReadResult *read_result = &loaded_save->read_result;
    struct SaveError err;
    *centry = loaded_save->centry;
    // Config loading reads the level numbers, and writes into parts of game (slab sets, research, ...)
    // that the save replaces afterwards, so the save goes in only after it.
    game.loaded_level_number = loaded_save->game->loaded_level_number;
    game.selected_level_number = loaded_save->game->selected_level_number;
    if (!change_campaign(CampgnT_Default, centry->campaign_fname))
    {
        SAVEERR("can't load campaign \"%s\" for \"%s\"", centry->campaign_fname, fname);
        return false;
    }
    free_level_strings_data();
    load_map_string_data(&campaign, centry->level_num, get_level_fgroup(centry->level_num));
    // Load configs which may have per-campaign part, and can even be modified within a level
    recheck_all_mod_exist();
    init_custom_sprites(centry->level_num);
    load_stats_files();
    snprintf(high_score_entry, PLAYER_NAME_LENGTH, "%s", centry->player_name);
    check_and_auto_fix_stats();
    // Same point as at a level start: the baseline the next save's config overlay is diffed against is the
    // config as loaded from files, without this save's own changes.
    take_conf_baseline();
    // The save replaces the game; the config just loaded and the process's own fields (timing, seeds, ...) stay.
    save_copy_runtime(&save_desc_Game, loaded_save->game.get(), &game);
    memcpy(&game, loaded_save->game.get(), sizeof(game));
    memcpy(&intralvl, loaded_save->ilvl.get(), sizeof(intralvl));
    // Apply the save's config changes on top of the fresh config.
    if (save_config_overlay_apply(read_result->conf_data, read_result->conf_len, &game.conf, &err) != SVR_Ok)
    {
        SAVEWARN("can't apply the config overlay of \"%s\": %s", fname, err.message);
        return false;
    }
    // Refuses if a used index no longer names the same config entry in this build.
    if (save_names_check(read_result->nams_data, read_result->nams_len, &game, &err) != SVR_Ok)
    {
        SAVEWARN("\"%s\" refused: %s", fname, err.message);
        return false;
    }
    if (read_result->lua_len > 0)
    {
        // has to be loaded here as level num only filled while gamestruct loaded, and need it for setting serialised_data
        open_lua_script(get_loaded_level_number());
        lua_set_serialised_data(read_result->lua_data, read_result->lua_len);
    }
    rebuild_derived_game_state();
    update_trap_tab_to_config();
    update_room_tab_to_config();
    return true;
}

enum SaveLoadOutcome SaveManager::Load(int slot_num, enum SaveCheckResult *why)
{
    *why = SvChk_Loadable;
    if (!EnsureSlot(slot_num))
    {
        SAVEERR("can't load: slot index %d is out of range", (int)slot_num);
        *why = SvChk_Missing;
        return SvLoad_Refused;
    }
    SYNCDBG(6,"Starting");
    // Nothing may change before the save has been read in full: a refused save leaves the
    // frontend or the running game as it was.
    *why = Check(slot_num, nullptr, 0);
    if (*why != SvChk_Loadable)
        return SvLoad_Refused;
    // prepare_file_fmtpath()'s buffer is overwritten by its next call, and
    // apply_game_kfxs() below triggers several (change_campaign, load_stats_files...),
    // so take a stable copy now.
    char fname[DISKPATH_SIZE];
    snprintf(fname, sizeof(fname), "%s", prepare_file_fmtpath(FGrp_Save, SAVED_GAME_FILENAME, slot_num));
    if (!Fs().Exists(fname))
    {
        SAVEWARN("can't open saved game file \"%s\"", fname);
        DisableSlot(slot_num);
        *why = SvChk_Missing;
        return SvLoad_Refused;
    }
    struct LoadedSave loaded_save;
    *why = read_game_kfxs(fname, &loaded_save);
    if (*why != SvChk_Loadable)
        return SvLoad_Refused;
    reset_eye_lenses();
    struct CatalogueEntry* centry = &catalogue_[slot_num];

        // Check if the game version is compatible
    if ((centry->game_ver_major != VER_MAJOR) || (centry->game_ver_minor != VER_MINOR) ||
        (centry->game_ver_release != VER_RELEASE) || (centry->game_ver_build != VER_BUILD))
    {
        SAVEWARN("\"%s\" was made by version %d.%d.%d.%d, this is %d.%d.%d.%d",
            fname, (int)centry->game_ver_major, (int)centry->game_ver_minor,
            (int)centry->game_ver_release, (int)centry->game_ver_build,
            VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD);
    }

    // Here is the actual loading
    TbBool applied = apply_game_kfxs(fname, &loaded_save, centry);
    loaded_save.Release();
    if (!applied)
    {
        if (game.loaded_level_number == 0)
        {
            game.loaded_level_number = centry->level_num;
        }
        SAVEWARN("slot %d didn't load correctly", (int)slot_num);
        return SvLoad_Broken;
    }
    my_player_number = game.local_plyr_idx;
    // Re-apply creature sound overrides: the config overlay (or a fresh config load)
    // may have restored session-specific negative bank indices; fix them to match
    // the current session's custom bank layout.
    sound_manager_reapply_creature_sounds();
    snprintf(game.campaign_fname, sizeof(game.campaign_fname), "%s", campaign.fname);
    init_navigation();
    reinit_level_after_load();
    take_game_timestamp();
    reinit_packets_after_load();
    initialize_packet_history();
    clear_packets();
    process_pause_packet(0, 0);
    clear_flag(game.operation_flags, GOF_Paused);
    clear_flag(game.operation_flags, GOF_WorldInfluence);
    close_main_cheat_menu();
    close_creature_cheat_menu();
    close_instance_cheat_menu();
    close_secondary_cheat_menu();
    output_message(SMsg_GameLoaded, 0);
    panel_map_update(0, 0, game.map_subtiles_x+1, game.map_subtiles_y+1);
    calculate_moon_phase(false,false);
    update_extra_levels_visibility();
    struct PlayerInfo* player = get_my_player();
    struct UserState* ustate = get_user_state(get_local_user());
    clear_flag(ustate->additional_flags, UsrAF_LightningPaletteIsActive);
    clear_flag(ustate->additional_flags, UsrAF_FreezePaletteIsActive);
    local_state.view_type = PVT_None;
    local_state.palette_fade_step_pain = 0;
    local_state.palette_fade_step_possession = 0;
    local_state.lens_palette = 0;
    local_state.minimap_pos_x = 11;
    local_state.minimap_pos_y = 11;
    local_state.roomspace_size = DEFAULT_USER_ROOMSPACE_WIDTH;
    // Reinitialize lens first (restores lens_palette pointer from config)
    reinitialise_eye_lens(game.applied_lens_type);
    // Apply the appropriate palette (lens palette if active, otherwise engine default)
    PaletteSetUserPalette(player->user_id, local_state.lens_palette ? local_state.lens_palette : engine_palette);
    apply_local_user_preferences(get_local_user(), UPF_ApplyOnLoad);
    init_local_cameras(player);
    // Update the lights system state
    light_import_system_state(&game.lightst);
    light_rebuild_after_load();
    // Victory state
    if (player->victory_state != VicS_Undecided)
    {
      frontstats_initialise();
      struct Dungeon* dungeon = get_players_dungeon(player);
      dungeon->lvstats.player_score = 0;
      dungeon->lvstats.allow_save_score = 1;
    }
    game.loaded_swipe_idx = -1;
    SAVELOG("loaded level %d of %s", game.continue_level_number, campaign.name);

    api_event("GAME_LOADED");

    return SvLoad_Done;
}

/******************************************************************************/

enum SaveResult SaveManager::ReadNetConfig(const char *fname, ConfigInfo *config, SaveError *err)
{
    return savefile::ReadRecord(Fs(), fname, *config, err);
}

bool SaveManager::WriteNetConfig(const char *fname, const ConfigInfo *config)
{
    return savefile::WriteRecord(Fs(), fname, *config);
}

enum SaveResult SaveManager::ReadHighScores(const char *fname, HighScore **table, uint32_t *count, SaveError *err)
{
    std::vector<HighScore> scores;
    enum SaveResult result = savefile::ReadRecords(Fs(), fname, scores, err);
    *count = (uint32_t)scores.size();
    *table = nullptr;
    if (!scores.empty())
    {
        *table = static_cast<HighScore *>(malloc(scores.size() * sizeof(HighScore)));
        if (*table == nullptr)
        {
            *count = 0;
            return save_fail(err, SVR_NoMemory, "out of memory");
        }
        memcpy(*table, scores.data(), scores.size() * sizeof(HighScore));
    }
    return result;
}

bool SaveManager::WriteHighScores(const char *fname, const HighScore *table, uint32_t count)
{
    return savefile::WriteRecords(Fs(), fname, table, count);
}

/******************************************************************************/

SaveManager &GetSaveManager()
{
    static SaveManager manager;
    return manager;
}

extern "C" {

int SaveManager_Save(int slot) { return GetSaveManager().Save(slot); }
enum SaveLoadOutcome SaveManager_Load(int slot, enum SaveCheckResult *why) { return GetSaveManager().Load(slot, why); }
enum SaveCheckResult SaveManager_Check(int slot, char *detail, size_t detail_len) { return GetSaveManager().Check(slot, detail, detail_len); }
enum SaveCheckResult SaveManager_Verify(int slot, char *detail, size_t detail_len) { return GetSaveManager().Verify(slot, detail, detail_len); }
TextStringId SaveManager_CheckMessage(enum SaveCheckResult result) { return SaveManager::CheckMessage(result); }
int SaveManager_KeepUnreadableFile(const char *fname) { return SaveManager::KeepUnreadableFile(fname); }

int SaveManager_LoadCatalogue(void) { return GetSaveManager().LoadCatalogue(); }
int SaveManager_InitialiseSlots(void) { return GetSaveManager().InitialiseSlots(); }
int SaveManager_SavedGamesCount(void) { return GetSaveManager().SavedGamesCount(); }
int SaveManager_SlotCount(void) { return GetSaveManager().SlotCount(); }
struct CatalogueEntry *SaveManager_Entry(int slot) { return GetSaveManager().Entry(slot); }
int SaveManager_FillSlot(int slot, const char *textname) { return GetSaveManager().FillSlot(slot, textname); }
int SaveManager_FillEntry(struct CatalogueEntry *centry, const char *textname) { return SaveManager::FillEntry(centry, textname); }
int SaveManager_DisableSlot(int slot) { return GetSaveManager().DisableSlot(slot); }

int SaveManager_ContinueAvailable(void) { return GetSaveManager().ContinueAvailable(); }
enum ContinueTargets SaveManager_LoadContinue(void) { return GetSaveManager().LoadContinue(); }
int SaveManager_SaveLevelProgress(LevelNumber lvnum, unsigned char victory_state) { return GetSaveManager().SaveLevelProgress(lvnum, victory_state); }
int SaveManager_ResumeCampaign(const char *campaign_fname) { return GetSaveManager().ResumeCampaign(campaign_fname); }
int SaveManager_CampaignHasProgress(const struct GameCampaign *campgn) { return GetSaveManager().CampaignHasProgress(campgn); }
void SaveManager_UpdateCampaignPercents(struct CampaignsList *clist) { GetSaveManager().UpdateCampaignPercents(clist); }
void SaveManager_DeleteContinueLink(void) { GetSaveManager().DeleteContinueLink(); }

enum SaveResult SaveManager_ReadNetConfig(const char *fname, struct ConfigInfo *config, struct SaveError *err)
{
    return GetSaveManager().ReadNetConfig(fname, config, err);
}

int SaveManager_WriteNetConfig(const char *fname, const struct ConfigInfo *config)
{
    return GetSaveManager().WriteNetConfig(fname, config);
}

enum SaveResult SaveManager_ReadHighScores(const char *fname, struct HighScore **table, uint32_t *count,
    struct SaveError *err)
{
    return GetSaveManager().ReadHighScores(fname, table, count, err);
}

int SaveManager_WriteHighScores(const char *fname, const struct HighScore *table, uint32_t count)
{
    return GetSaveManager().WriteHighScores(fname, table, count);
}

} // extern "C"
