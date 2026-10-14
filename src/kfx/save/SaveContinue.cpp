/******************************************************************************/
/** @file SaveContinue.cpp
 *     The continue link (the last save or campaign played) and the per-campaign progress files.
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
#include "game_legacy.h"
#include "game_merge.h"
#include "kfx/platform/IPlatform.h"
#include "player_data.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#include <cstring>
#include <strings.h>

#include "kfx/save/SaveManager_Internal.h"
#include "kfx/save/core/schema/save_tables_decl.h"

namespace {

const char *const legacy_campaign_progress = "fx1contn.sav";
const char *const continue_filename = "fx1lastf.sav"; // (merely points to another save file)

#define INTRALEVEL_DATA_V0_SIZE      offsetof(struct IntralevelData, continue_level)
#define LEGACY_PROGRESS_FILE_SIZE (CAMPAIGN_FNAME_LEN + sizeof(LevelNumber) + INTRALEVEL_DATA_V0_SIZE)
#define CONTINUE_DATA_VER 0

IntralevelData progress_scratch;

IFileSystem &Fs() { return *GetPlatform()->GetFileSystem(); }

void get_progress_filename(const char *cmpgn_fname, char *out, size_t outlen)
{
    char name[CAMPAIGN_FNAME_LEN];
    get_campaign_sanitized_id(cmpgn_fname, name, sizeof(name));
    snprintf(out, outlen, "%s.cid", name);
}

bool is_kfxs_file(IFileSystem &filesystem, const char *fname)
{
    std::unique_ptr<IFileReader> reader = filesystem.OpenReader(fname);
    char magic[SAVE_MAGIC_SIZE];
    return reader && reader->ReadAt(0, magic, sizeof(magic)) && save_magic_matches(magic, sizeof(magic));
}

/** Next chunk header of a file in the raw chunk format, with pos moved past it. */
bool next_raw_chunk(const std::vector<uint8_t> &file, size_t &pos, FileChunkHeader &header)
{
    if (file.size() - pos < sizeof(header))
        return false;
    memcpy(&header, &file[pos], sizeof(header));
    pos += sizeof(header);
    return true;
}

/** Reads a progress file in the raw chunk format that came before the KFXS container. */
bool read_raw_progress(const std::vector<uint8_t> &file, const char *progress_fname, IntralevelData *ilvl)
{
    size_t pos = 0;
    FileChunkHeader header;
    while (next_raw_chunk(file, pos, header))
    {
        if (header.id == SGC_IntralevelData)
        {
            uint32_t len = 0;
            if ((header.ver == INTRALEVEL_DATA_VER) && (header.len == sizeof(IntralevelData)))
                len = sizeof(IntralevelData);
            else if ((header.ver == 0) && (header.len == INTRALEVEL_DATA_V0_SIZE))
                len = INTRALEVEL_DATA_V0_SIZE;
            if (len == 0)
            {
                SAVEWARN("progress file has an incompatible IntralevelData chunk (version %" PRIu32 ", length %" PRIu32 ")", header.ver, header.len);
                return false;
            }
            memset(ilvl, 0, sizeof(IntralevelData));
            if (file.size() - pos < len)
                return false;
            memcpy(ilvl, &file[pos], len);
            return true;
        }
        SAVEWARN("progress file \"%s\" has an unrecognised chunk (id %08" PRIx32 ")", progress_fname, header.id);
        if (file.size() - pos < header.len)
            break;
        pos += header.len;
    }
    return false;
}

bool read_progress_file(const char *progress_fname, IntralevelData *ilvl)
{
    IFileSystem &filesystem = Fs();
    const char *fname = prepare_file_path(FGrp_Save, progress_fname);
    if (!filesystem.Exists(fname))
        return false;
    if (is_kfxs_file(filesystem, fname))
    {
        SaveError err;
        return (savefile::ReadRecord(filesystem, fname, *ilvl, &err) == SVR_Ok);
    }
    // Raw chunk format (before the container): import once, the caller rewrites it through write_progress_file.
    std::vector<uint8_t> file;
    if (filesystem.ReadAll(fname, file) != FsStatus::Ok)
        return false;
    return read_raw_progress(file, progress_fname, ilvl);
}

bool write_progress_file(const char *progress_fname, const IntralevelData *ilvl)
{
    return savefile::WriteRecord(Fs(), prepare_file_path(FGrp_Save, progress_fname), *ilvl);
}

/** The oldest progress file, fx1contn.sav: a campaign name, a level and the progress data of version 0. */
bool read_legacy_progress_file(std::vector<uint8_t> &file)
{
    return (Fs().ReadAll(prepare_file_path(FGrp_Save, legacy_campaign_progress), file) == FsStatus::Ok)
        && (file.size() == LEGACY_PROGRESS_FILE_SIZE);
}

bool read_legacy_progress_fname(char *cmpgn_fname)
{
    std::vector<uint8_t> file;
    if (!read_legacy_progress_file(file))
        return false;
    memcpy(cmpgn_fname, file.data(), CAMPAIGN_FNAME_LEN);
    cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    return true;
}

bool legacy_progress_matches(const char *cmpgn_fname)
{
    char legacy_fname[CAMPAIGN_FNAME_LEN];
    return read_legacy_progress_fname(legacy_fname) && (strcasecmp(legacy_fname, cmpgn_fname) == 0);
}

bool read_legacy_progress(const char *cmpgn_fname, IntralevelData *ilvl)
{
    std::vector<uint8_t> file;
    if (!read_legacy_progress_file(file))
        return false;
    char legacy_fname[CAMPAIGN_FNAME_LEN];
    LevelNumber lvnum = 0;
    memcpy(legacy_fname, file.data(), CAMPAIGN_FNAME_LEN);
    memcpy(&lvnum, file.data() + CAMPAIGN_FNAME_LEN, sizeof(lvnum));
    legacy_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    if (strcasecmp(legacy_fname, cmpgn_fname) != 0)
        return false;
    memset(ilvl, 0, sizeof(IntralevelData));
    memcpy(ilvl, file.data() + CAMPAIGN_FNAME_LEN + sizeof(lvnum), INTRALEVEL_DATA_V0_SIZE);
    ilvl->continue_level = encode_continue_level(lvnum);
    return true;
}

void delete_legacy_progress()
{
    Fs().Delete(prepare_file_path(FGrp_Save, legacy_campaign_progress));
}

bool read_last_file_link(char *link, size_t linklen)
{
    IFileSystem &filesystem = Fs();
    const char *fname = prepare_file_path(FGrp_Save, continue_filename);
    if (!filesystem.Exists(fname))
        return false;
    ContinueData cdata;
    memset(&cdata, 0, sizeof(cdata));
    if (is_kfxs_file(filesystem, fname))
    {
        SaveError err;
        if (savefile::ReadRecord(filesystem, fname, cdata, &err) != SVR_Ok)
            return false;
    } else
    {
        // Raw chunk format (before the container): import once, the caller rewrites it through write_last_file_link.
        std::vector<uint8_t> file;
        if (filesystem.ReadAll(fname, file) != FsStatus::Ok)
            return false;
        size_t pos = 0;
        FileChunkHeader header;
        bool found = false;
        while (!found && next_raw_chunk(file, pos, header))
        {
            if ((header.id == SGC_Continue) && (header.ver == CONTINUE_DATA_VER) && (header.len == sizeof(ContinueData)))
            {
                if (file.size() - pos < sizeof(cdata))
                    return false;
                memcpy(&cdata, &file[pos], sizeof(cdata));
                found = true;
            } else
            {
                if (file.size() - pos < header.len)
                    break;
                pos += header.len;
            }
        }
        if (!found)
            return false;
    }
    cdata.link_fname[SAVE_FILENAME_MAX-1] = '\0';
    snprintf(link, linklen, "%s", cdata.link_fname);
    return (link[0] != '\0');
}

bool write_last_file_link(const char *link)
{
    ContinueData cdata;
    memset(&cdata, 0, sizeof(cdata));
    snprintf(cdata.link_fname, sizeof(cdata.link_fname), "%s", link);
    return savefile::WriteRecord(Fs(), prepare_file_path(FGrp_Save, continue_filename), cdata);
}

void delete_last_file_link()
{
    Fs().Delete(prepare_file_path(FGrp_Save, continue_filename));
}

} // namespace

/******************************************************************************/

bool SaveManager::LoadCampaignProgress(const GameCampaign *campgn, IntralevelData *ilvl, LevelNumber *continue_lvnum)
{
    char progress_fname[DISKPATH_SIZE];
    get_progress_filename(campgn->fname, progress_fname, sizeof(progress_fname));
    bool found = read_progress_file(progress_fname, ilvl);
    if (!found)
        found = read_legacy_progress(campgn->fname, ilvl);
    if (!found)
        return false;
    *continue_lvnum = resolve_campaign_progress(ilvl, campgn);
    return true;
}

void SaveManager::DeleteContinueLink()
{
    delete_last_file_link();
}

void SaveManager::UpdateLastFileAfterSave(int slot)
{
    int humans = 0;
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        const struct PlayerInfo *player = get_player(plyr_idx);
        if (flag_is_set(player->allocflags, PlaF_Allocated)
          && (!flag_is_set(player->allocflags, PlaF_CompCtrl) || flag_is_set(player->allocflags, PlaF_Placeholder)))
            humans++;
    }
    char link[SAVE_FILENAME_MAX];
    snprintf(link, sizeof(link), SAVED_GAME_FILENAME, slot);
    if (humans == 1)
    {
        write_last_file_link(link);
        return;
    }
    char current[SAVE_FILENAME_MAX];
    if (read_last_file_link(current, sizeof(current)) && (strcasecmp(current, link) == 0))
        delete_last_file_link();
}

enum ContinueTargets SaveManager::FindContinueTarget(int *slot_num, char *cmpgn_fname, size_t fnamelen)
{
    if (campaigns_list.items_num == 1)
    {
        const GameCampaign *campgn = &campaigns_list.items[0];
        if (!CampaignHasProgress(campgn))
            return CntT_None;
        snprintf(cmpgn_fname, fnamelen, "%s", campgn->fname);
        return CntT_CampaignProgress;
    }
    char link[SAVE_FILENAME_MAX];
    if (read_last_file_link(link, sizeof(link)))
    {
        const int slot = savegame::SlotFromFilename(link);
        if (slot >= 0)
        {
            // only a save this build can load is offered; the rest of the checks are the load's
            CatalogueEntry centry;
            int title_ok;
            if (savegame::CheckFile(Fs(), prepare_file_fmtpath(FGrp_Save, SAVED_GAME_FILENAME, slot), &centry, &title_ok,
                    nullptr, 0) == SvChk_Loadable)
            {
                *slot_num = slot;
                return CntT_SavedGame;
            }
        } else
        {
            for (unsigned long index = 0; index < campaigns_list.items_num; index++)
            {
                const GameCampaign *campgn = &campaigns_list.items[index];
                char progress_fname[DISKPATH_SIZE];
                get_progress_filename(campgn->fname, progress_fname, sizeof(progress_fname));
                if ((strcasecmp(link, progress_fname) == 0) && Fs().Exists(prepare_file_path(FGrp_Save, progress_fname)))
                {
                    snprintf(cmpgn_fname, fnamelen, "%s", campgn->fname);
                    return CntT_CampaignProgress;
                }
            }
        }
    }
    char legacy_fname[CAMPAIGN_FNAME_LEN];
    if (read_legacy_progress_fname(legacy_fname) && is_campaign_in_list(legacy_fname, &campaigns_list))
    {
        snprintf(cmpgn_fname, fnamelen, "%s", legacy_fname);
        return CntT_CampaignProgress;
    }
    return CntT_None;
}

bool SaveManager::ContinueAvailable()
{
    int slot_num = -1;
    char cmpgn_fname[DISKPATH_SIZE];
    return (FindContinueTarget(&slot_num, cmpgn_fname, sizeof(cmpgn_fname)) != CntT_None);
}

enum ContinueTargets SaveManager::LoadContinue()
{
    int slot_num = -1;
    char cmpgn_fname[DISKPATH_SIZE];
    switch (FindContinueTarget(&slot_num, cmpgn_fname, sizeof(cmpgn_fname)))
    {
    case CntT_SavedGame:
        LoadCatalogue();
        if ((slot_num >= slot_count_) || ((catalogue_[slot_num].flags & CEF_InUse) == 0))
            break;
        game.save_game_slot = slot_num;
        return CntT_SavedGame;
    case CntT_CampaignProgress:
        if (ResumeCampaign(cmpgn_fname))
            return CntT_CampaignProgress;
        break;
    default:
        break;
    }
    return CntT_None;
}

bool SaveManager::CampaignHasProgress(const GameCampaign *campgn)
{
    char progress_fname[DISKPATH_SIZE];
    get_progress_filename(campgn->fname, progress_fname, sizeof(progress_fname));
    if (Fs().Exists(prepare_file_path(FGrp_Save, progress_fname)))
        return true;
    return legacy_progress_matches(campgn->fname);
}

void SaveManager::UpdateCampaignPercents(CampaignsList *clist)
{
    for (unsigned long index = 0; index < clist->items_num; index++)
    {
        GameCampaign *campgn = &clist->items[index];
        LevelNumber continue_lvnum;
        if (LoadCampaignProgress(campgn, &progress_scratch, &continue_lvnum))
            campgn->progress_percent = campaign_progress_percent(campgn, &progress_scratch, continue_lvnum);
        else
            campgn->progress_percent = -1;
    }
}

bool SaveManager::ResumeCampaign(const char *cmpgn_fname)
{
    if (!change_campaign(CampgnT_Campaign, cmpgn_fname))
    {
        SAVEERR("can't load campaign \"%s\"", cmpgn_fname);
        return false;
    }
    LevelNumber lvnum;
    if (!LoadCampaignProgress(&campaign, &progress_scratch, &lvnum))
        return false;
    if (!is_singleplayer_like_level(lvnum))
    {
        SAVEWARN("continue level %d of \"%s\" is incorrect", (int)lvnum, cmpgn_fname);
        return false;
    }
    memcpy(&intralvl, &progress_scratch, sizeof(IntralevelData));
    set_continue_level_number(lvnum);
    snprintf(game.campaign_fname, sizeof(game.campaign_fname), "%s", campaign.fname);
    update_extra_levels_visibility();
    SAVELOG("continued level %d of %s", (int)lvnum, campaign.name);
    return true;
}

// writes campaign progress file; on a win also records the level and updates the continue file
bool SaveManager::SaveLevelProgress(LevelNumber lvnum, unsigned char victory_state)
{
    if ((victory_state == VicS_WonLevel) && is_campaign_progress_level(&campaign, lvnum))
        mark_level_completed(&intralvl, lvnum);
    intralvl.continue_level = encode_continue_level(get_continue_level_number());
    char progress_fname[DISKPATH_SIZE];
    get_progress_filename(campaign.fname, progress_fname, sizeof(progress_fname));
    if (!write_progress_file(progress_fname, &intralvl))
        return false;

    // quitting an undecided level keeps the link, so a saved game can still be continued
    if (victory_state != VicS_Undecided)
        write_last_file_link(progress_fname);

    // clean up legacy file if this is a replacement
    if (legacy_progress_matches(campaign.fname))
        delete_legacy_progress();
    return true;
}
