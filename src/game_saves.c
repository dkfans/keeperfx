/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_saves.c
 *     Saved games maintain functions.
 * @par Purpose:
 *     For opening, writing, listing saved games.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 Jan 2009 - 25 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_saves.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
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
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
TbBool load_catalogue_entry(TbFileHandle fh,struct FileChunkHeader *hdr,struct CatalogueEntry *centry);
/******************************************************************************/
const short VersionMajor    = VER_MAJOR;
const short VersionMinor    = VER_MINOR;
short const VersionRelease  = VER_RELEASE;
short const VersionBuild    = VER_BUILD;

const char *legacy_campaign_progress="fx1contn.sav";
const char *continue_filename="fx1lastf.sav"; // (merely points to another save file)
const char *saved_game_filename="fx1g%04d.sav";
const char *packet_filename="fx1rp%04d.pck";

/* Dynamically-grown savegame catalogue (see game_saves.h): holds one CatalogueEntry per
 * reachable save slot. It is sized on load to (highest existing slot + 2), min
 * SAVE_SLOTS_MIN, and grown on demand when writing to a higher slot, so the slot count
 * is limited only by disk space. */
struct CatalogueEntry *save_game_catalogue = NULL;
long save_game_catalogue_count = 0;      // logical length (slots the menus range over)
static long save_game_catalogue_capacity = 0;  // number of entries actually allocated

int number_of_saved_games;

/** Grow the catalogue so that index slot is valid; any newly added entries are zeroed
 *  (not in use). Returns false only on allocation failure. */
static TbBool ensure_catalogue_slot(long slot)
{
    if ((slot < 0) || (slot >= SAVE_SLOTS_LIMIT))
        return false;
    if (slot >= save_game_catalogue_capacity)
    {
        long newcap = slot + 1;
        struct CatalogueEntry* p = (struct CatalogueEntry*)realloc(save_game_catalogue,
            newcap * sizeof(struct CatalogueEntry));
        if (p == NULL)
        {
            ERRORLOG("Cannot grow save catalogue to %ld entries", newcap);
            return false;
        }
        memset(&p[save_game_catalogue_capacity], 0,
            (newcap - save_game_catalogue_capacity) * sizeof(struct CatalogueEntry));
        save_game_catalogue = p;
        save_game_catalogue_capacity = newcap;
    }
    if (slot >= save_game_catalogue_count)
        save_game_catalogue_count = slot + 1;
    return true;
}

/** Parse the slot index from a savegame filename "fx1gNNNN.sav" (case-insensitive).
 *  Returns the index, or -1 if the name does not match the expected pattern. */
static int save_slot_index_from_filename(const char *fname)
{
    if (strncasecmp(fname, "fx1g", 4) != 0)
        return -1;
    const char* p = fname + 4;
    if ((*p < '0') || (*p > '9'))
        return -1;
    long idx = atol(p);
    while ((*p >= '0') && (*p <= '9'))
        p++;
    if (strcasecmp(p, ".sav") != 0)
        return -1;
    if ((idx < 0) || (idx >= SAVE_SLOTS_LIMIT))
        return -1;
    return (int)idx;
}

#define INTRALEVEL_DATA_V0_SIZE      offsetof(struct IntralevelData, continue_level)
#define LEGACY_PROGRESS_FILE_SIZE (CAMPAIGN_FNAME_LEN + sizeof(LevelNumber) + INTRALEVEL_DATA_V0_SIZE)
#define CONTINUE_DATA_VER 0

static struct IntralevelData progress_scratch;

static void update_last_file_after_save(int32_t slot_num);

/******************************************************************************/
TbBool is_primitive_save_version(long filesize)
{
    if (filesize < (char *)&game.loaded_level_number - (char *)&game)
        return false;
    if (filesize <= 1382437) // sizeof(struct Game) - but it's better to use constant here
        return true;
    return false;
}

TbBool save_game_chunks(TbFileHandle fhandle, struct CatalogueEntry *centry)
{
    struct FileChunkHeader hdr;
    long chunks_done = 0;
    // Currently there is some game data outside of structs - make sure it is updated
    light_export_system_state(&game.lightst);
    { // Info chunk
        hdr.id = SGC_InfoBlock;
        hdr.ver = CATALOGUE_ENTRY_VER;
        hdr.len = sizeof(struct CatalogueEntry);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, centry, sizeof(struct CatalogueEntry)) == sizeof(struct CatalogueEntry))
            chunks_done |= SGF_InfoBlock;
    }
    { // Game data chunk
        hdr.id = SGC_GameOrig;
        hdr.ver = 0;
        hdr.len = sizeof(struct Game);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, &game, sizeof(struct Game)) == sizeof(struct Game))
            chunks_done |= SGF_GameOrig;
    }
    { // IntralevelData data chunk
        hdr.id = SGC_IntralevelData;
        hdr.ver = INTRALEVEL_DATA_VER;
        hdr.len = sizeof(struct IntralevelData);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, &intralvl, sizeof(struct IntralevelData)) == sizeof(struct IntralevelData))
            chunks_done |= SGF_IntralevelData;
    }

    // Adding Lua serialized data chunk
    {
        size_t lua_data_len;
        const char* lua_data = lua_get_serialised_data(&lua_data_len);

        hdr.id = SGC_LuaData;
        hdr.ver = 0;
        hdr.len = lua_data_len;
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, lua_data, lua_data_len) == lua_data_len)
            chunks_done |= SGF_LuaData;
        cleanup_serialized_data();
    }

    if (chunks_done != SGF_SavedGame)
        return false;
    return true;
}

TbBool save_packet_chunks(TbFileHandle fhandle,struct CatalogueEntry *centry)
{
    struct FileChunkHeader hdr;
    long chunks_done = 0;
    { // Packet file header
        hdr.id = SGC_PacketHeader;
        hdr.ver = PACKET_SAVE_HEAD_VER;
        hdr.len = sizeof(struct PacketSaveHead);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, &game.packet_save_head, sizeof(struct PacketSaveHead)) == sizeof(struct PacketSaveHead))
            chunks_done |= SGF_PacketHeader;
    }
    { // Info chunk
        hdr.id = SGC_InfoBlock;
        hdr.ver = CATALOGUE_ENTRY_VER;
        hdr.len = sizeof(struct CatalogueEntry);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, centry, sizeof(struct CatalogueEntry)) == sizeof(struct CatalogueEntry))
            chunks_done |= SGF_InfoBlock;
    }
    // If it's not start of a level, save progress data too
    if (get_gameturn() != 0)
    {
        { // Game data chunk
            hdr.id = SGC_GameOrig;
            hdr.ver = 0;
            hdr.len = sizeof(struct Game);
            if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
            if (LbFileWrite(fhandle, &game, sizeof(struct Game)) == sizeof(struct Game))
                chunks_done |= SGF_GameOrig;
        }
    }
    { // Packet file data start indicator
        hdr.id = SGC_PacketData;
        hdr.ver = PACKET_VER;
        hdr.len = 0; // unbounded
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
            chunks_done |= SGF_PacketData;
    }
    if ((chunks_done != SGF_PacketStart) && (chunks_done != SGF_PacketContinue))
        return false;
    return true;
}

static TbBool chunk_version_ok(TbFileHandle fhandle, const struct FileChunkHeader *hdr, unsigned expected)
{
    if (hdr->ver == expected)
        return true;
    WARNLOG("Chunk %04x is version %u, expected %u; skipping it",
        (unsigned)hdr->id, (unsigned)hdr->ver, (unsigned)expected);
    if (LbFileSeek(fhandle, hdr->len, Lb_FILE_SEEK_CURRENT) < 0)
        LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
    return false;
}

static TbBool write_chunk(TbFileHandle fhandle, uint32_t id, uint32_t ver, const void *data, uint32_t len)
{
    struct FileChunkHeader hdr;
    hdr.id = id;
    hdr.ver = ver;
    hdr.len = len;
    return (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        && (LbFileWrite(fhandle, data, len) == len);
}

static void skip_chunk(TbFileHandle fhandle, const struct FileChunkHeader *hdr)
{
    if (LbFileSeek(fhandle, hdr->len, Lb_FILE_SEEK_CURRENT) < 0)
        LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
}

static TbBool read_intralevel_chunk(TbFileHandle fhandle, const struct FileChunkHeader *hdr, struct IntralevelData *ilvl)
{
    uint32_t len = 0;
    if ((hdr->ver == INTRALEVEL_DATA_VER) && (hdr->len == sizeof(struct IntralevelData)))
        len = sizeof(struct IntralevelData);
    else if ((hdr->ver == 0) && (hdr->len == INTRALEVEL_DATA_V0_SIZE))
        len = INTRALEVEL_DATA_V0_SIZE;
    if (len == 0)
    {
        WARNLOG("Incompatible IntralevelData chunk, version %" PRIu32 ", length %" PRIu32, hdr->ver, hdr->len);
        skip_chunk(fhandle, hdr);
        return false;
    }
    memset(ilvl, 0, sizeof(struct IntralevelData));
    return (LbFileRead(fhandle, ilvl, len) == (int)len);
}

static TbBool is_campaign_progress_level(const struct GameCampaign *campgn, LevelNumber lvnum)
{
    return (campaign_singleplayer_level_index(campgn, lvnum) >= 0) || (campaign_bonus_level_index(campgn, lvnum) >= 0)
        || (campaign_extra_level_index(campgn, lvnum) >= 0);
}

static TbBool is_level_completed(const struct IntralevelData *ilvl, LevelNumber lvnum)
{
    if (lvnum < 1)
        return false;
    for (int i = 0; i < LEVELS_COMPLETED_COUNT; i++)
    {
        if (ilvl->levels_completed[i] == (uint32_t)lvnum + 1)
            return true;
    }
    return false;
}

static void mark_level_completed(struct IntralevelData *ilvl, LevelNumber lvnum)
{
    if ((lvnum < 1) || is_level_completed(ilvl, lvnum))
        return;
    for (int i = 0; i < LEVELS_COMPLETED_COUNT; i++)
    {
        if (ilvl->levels_completed[i] == 0)
        {
            ilvl->levels_completed[i] = (uint32_t)lvnum + 1;
            return;
        }
    }
}

static void fill_missing_levels_completed(struct IntralevelData *ilvl, const struct GameCampaign *campgn, LevelNumber ref_lvnum)
{
    for (int i = 0; i < LEVELS_COMPLETED_COUNT; i++)
    {
        if (ilvl->levels_completed[i] != 0)
            return;
    }
    int limit;
    if (ref_lvnum == SINGLEPLAYER_FINISHED)
    {
        limit = CAMPAIGN_LEVELS_COUNT;
    } else
    {
        limit = campaign_singleplayer_level_index(campgn, ref_lvnum);
        if (limit < 0)
            limit = campaign_bonus_level_index(campgn, ref_lvnum);
    }
    for (int i = 0; i < limit; i++)
        mark_level_completed(ilvl, campgn->single_levels[i]);
}

static uint32_t encode_continue_level(LevelNumber lvnum)
{
    if (lvnum == SINGLEPLAYER_FINISHED)
        return UINT32_MAX;
    if (lvnum < 0)
        return 0;
    return (uint32_t)lvnum + 1;
}

static LevelNumber stored_continue_level(const struct IntralevelData *ilvl)
{
    if (ilvl->continue_level == 0)
        return LEVELNUMBER_ERROR;
    if (ilvl->continue_level == UINT32_MAX)
        return SINGLEPLAYER_FINISHED;
    return (LevelNumber)(ilvl->continue_level - 1);
}

static LevelNumber resolve_continue_level(const struct IntralevelData *ilvl, const struct GameCampaign *campgn)
{
    LevelNumber lvnum = stored_continue_level(ilvl);
    if ((lvnum == SINGLEPLAYER_FINISHED) || (lvnum == SINGLEPLAYER_NOTSTARTED) || (campaign_singleplayer_level_index(campgn, lvnum) >= 0))
        return lvnum;
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if ((campgn->single_levels[i] > 0) && !is_level_completed(ilvl, campgn->single_levels[i]))
            return campgn->single_levels[i];
    }
    return SINGLEPLAYER_FINISHED;
}

static LevelNumber resolve_campaign_progress(struct IntralevelData *ilvl, const struct GameCampaign *campgn)
{
    fill_missing_levels_completed(ilvl, campgn, stored_continue_level(ilvl));
    LevelNumber lvnum = resolve_continue_level(ilvl, campgn);
    ilvl->continue_level = encode_continue_level(lvnum);
    return lvnum;
}

static short campaign_progress_percent(const struct GameCampaign *campgn, const struct IntralevelData *ilvl, LevelNumber continue_lvnum)
{
    int total = 0;
    int done = 0;
    int secret = 0;
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if (campgn->single_levels[i] <= 0)
            continue;
        total++;
        if (is_level_completed(ilvl, campgn->single_levels[i]))
            done++;
    }
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        LevelNumber lvnum = campgn->bonus_levels[i];
        if ((campaign_bonus_level_index(campgn, lvnum) == i) && (campaign_singleplayer_level_index(campgn, lvnum) < 0)
          && is_level_completed(ilvl, lvnum))
            secret++;
    }
    for (int i = 0; i < EXTRA_LEVELS_COUNT; i++)
    {
        LevelNumber lvnum = campgn->extra_levels[i];
        if ((campaign_extra_level_index(campgn, lvnum) == i) && (campaign_singleplayer_level_index(campgn, lvnum) < 0)
          && (campaign_bonus_level_index(campgn, lvnum) < 0) && is_level_completed(ilvl, lvnum))
            secret++;
    }
    TbBool finished = (continue_lvnum == SINGLEPLAYER_FINISHED) || (done == total);
    int percent;
    if (finished)
        percent = 100;
    else if ((done == 0) || (total <= 0))
        percent = 0;
    else
        percent = (99 * done + total - 1) / total;
    percent += secret;
    if (!finished && (percent > 99))
        percent = 99;
    return percent;
}

int load_game_chunks(TbFileHandle fhandle, struct CatalogueEntry *centry)
{
    long chunks_done = 0;
    while (!LbFileEof(fhandle))
    {
        struct FileChunkHeader hdr;
        if (LbFileRead(fhandle, &hdr, sizeof(struct FileChunkHeader)) != sizeof(struct FileChunkHeader))
            break;
        switch (hdr.id)
        {
        case SGC_InfoBlock:
            if (!chunk_version_ok(fhandle, &hdr, CATALOGUE_ENTRY_VER))
                break;
            if (load_catalogue_entry(fhandle, &hdr, centry))
            {
                chunks_done |= SGF_InfoBlock;
                if (!change_campaign(CampgnT_Default, centry->campaign_fname)) {
                    ERRORLOG("Unable to load campaign");
                    return GLoad_Failed;
                }
                free_level_strings_data();
                struct GameCampaign *campgn = &campaign;
                load_map_string_data(campgn, centry->level_num, get_level_fgroup(centry->level_num));
                // Load configs which may have per-campaign part, and even be modified within a level
                recheck_all_mod_exist();
                init_custom_sprites(centry->level_num);
                load_stats_files();
                snprintf(high_score_entry, PLAYER_NAME_LENGTH, "%s", centry->player_name);
            }
            break;
        case SGC_GameOrig:
            if (hdr.len != sizeof(struct Game))
            {
                if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                    LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
                WARNLOG("Incompatible GameOrig chunk");
                break;
            }
            if (LbFileRead(fhandle, &game, sizeof(struct Game)) == sizeof(struct Game)) {
                chunks_done |= SGF_GameOrig;
            } else {
                WARNLOG("Could not read GameOrig chunk");
            }
            break;
        case SGC_PacketHeader:
            if (!chunk_version_ok(fhandle, &hdr, PACKET_SAVE_HEAD_VER))
                break;
            if (hdr.len != sizeof(struct PacketSaveHead))
            {
                if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                    LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
                WARNLOG("Incompatible PacketHeader chunk");
                break;
            }
            if (LbFileRead(fhandle, &game.packet_save_head, sizeof(struct PacketSaveHead))
                == sizeof(struct PacketSaveHead)) {
                chunks_done |= SGF_PacketHeader;
            } else {
                WARNLOG("Could not read GameOrig chunk");
            }
            break;
        case SGC_PacketData:
            if (!chunk_version_ok(fhandle, &hdr, PACKET_VER))
                break;
            if (hdr.len != 0)
            {
                if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                    LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
                WARNLOG("Incompatible PacketData chunk");
                break;
            }
            chunks_done |= SGF_PacketData;
            if ((chunks_done & SGF_PacketContinue) == SGF_PacketContinue)
                return GLoad_PacketContinue;
            if ((chunks_done & SGF_PacketStart) == SGF_PacketStart)
                return GLoad_PacketStart;
            return GLoad_Failed;
        case SGC_IntralevelData:
            if (read_intralevel_chunk(fhandle, &hdr, &intralvl)) {
                chunks_done |= SGF_IntralevelData;
            } else {
                WARNLOG("Could not read IntralevelData chunk");
            }
            break;
        case SGC_LuaData:
            {
                char* lua_data = (char*)malloc(hdr.len);
                if (lua_data == NULL) {
                    WARNLOG("Could not allocate memory for LuaData chunk");
                    break;
                }
                if (LbFileRead(fhandle, lua_data, hdr.len) == hdr.len) {
                    //has to be loaded here as level num only filled while gamestruct loaded, and need it for setting serialised_data
                    open_lua_script(get_loaded_level_number());

                    lua_set_serialised_data(lua_data, hdr.len);
                    chunks_done |= SGF_LuaData;
                } else {
                    WARNLOG("Could not read LuaData chunk");
                    free(lua_data);
                }
            }
            break;
        default:
            WARNLOG("Unrecognized chunk, ID = %08x", (unsigned)hdr.id);
            if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
            break;
        }
    }
    if ((chunks_done & SGF_SavedGame) == SGF_SavedGame)
    {
        fill_missing_levels_completed(&intralvl, &campaign, get_loaded_level_number());
        // Update interface items
        update_trap_tab_to_config();
        update_room_tab_to_config();
        return GLoad_SavedGame;
    }
    return GLoad_Failed;
}

/**
 * Saves the game state file (savegame).
 * @note fill_game_catalogue_entry() should be called before to fill level information.
 *
 * @param slot_num
 * @return
 */
TbBool save_game(long slot_num)
{
    if (!ensure_catalogue_slot(slot_num))
    {
        ERRORLOG("Outranged slot index %d",(int)slot_num);
        return false;
    }
    char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (!handle)
    {
        WARNMSG("Cannot open file to save, \"%s\".",fname);
        return false;
    }
    if (!save_game_chunks(handle,&save_game_catalogue[slot_num]))
    {
        LbFileClose(handle);
        WARNMSG("Cannot write to save file, \"%s\".",fname);
        return false;
    }
    LbFileClose(handle);
    update_last_file_after_save(slot_num);
    api_event("GAME_SAVED");
    return true;
}

TbBool is_save_game_loadable(long slot_num)
{
    // Prepare filename and open the file
    char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
    TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (fh)
    {
        // Let's try to read the file, just to be sure
        struct FileChunkHeader hdr;
        if (LbFileRead(fh, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        {
            LbFileClose(fh);
            return true;
        }
        LbFileClose(fh);
    }
    return false;
}

TbBool load_game(long slot_num)
{
    if (!ensure_catalogue_slot(slot_num))
    {
        ERRORLOG("Outranged slot index %d",(int)slot_num);
        return false;
    }
    TbFileHandle fh;
//  unsigned char buf[14];
//  char cmpgn_fname[CAMPAIGN_FNAME_LEN];
    SYNCDBG(6,"Starting");
    reset_eye_lenses();
    {
        // Use fname only here - it is overwritten by next use of prepare_file_fmtpath()
        char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
        fh = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
        if (!fh)
        {
          WARNMSG("Cannot open saved game file \"%s\".",fname);
          save_catalogue_slot_disable(slot_num);
          return false;
        }
    }
    long file_len = LbFileLengthHandle(fh);
    if (is_primitive_save_version(file_len))
    {
        {
          LbFileClose(fh);
          save_catalogue_slot_disable(slot_num);
          return false;
        }
    }
    struct CatalogueEntry* centry = &save_game_catalogue[slot_num];

        // Check if the game version is compatible
    if ((centry->game_ver_major != VER_MAJOR) || (centry->game_ver_minor != VER_MINOR) ||
        (centry->game_ver_release != VER_RELEASE) || (centry->game_ver_build != VER_BUILD))
    {
        WARNLOG("loading savegame made in different version %d.%d.%d.%d current %d.%d.%d.%d",
            (int)centry->game_ver_major, (int)centry->game_ver_minor,
            (int)centry->game_ver_release, (int)centry->game_ver_build,
            VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD);
    }

    LbFileSeek(fh, 0, Lb_FILE_SEEK_BEGINNING);
    // Here is the actual loading
    if (load_game_chunks(fh,centry) != GLoad_SavedGame)
    {
        LbFileClose(fh);
        if (game.loaded_level_number == 0)
        {
            game.loaded_level_number = centry->level_num;
        }
        WARNMSG("Couldn't correctly load saved game in slot %d.",(int)slot_num);
        return false;
    }
    my_player_number = game.local_plyr_idx;
    LbFileClose(fh);
    // Re-apply creature sound overrides: SGC_GameOrig restored game.conf with
    // session-specific negative bank indices from the save; fix them to match
    // the current session's custom bank layout.
    sound_manager_reapply_creature_sounds();
    snprintf(game.campaign_fname, sizeof(game.campaign_fname), "%s", campaign.fname);
    reinit_level_after_load();
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
    local_state.minimap_zoom = settings.minimap_zoom;
    local_state.roomspace_size = DEFAULT_USER_ROOMSPACE_WIDTH;
    // Reinitialize lens first (restores lens_palette pointer from config)
    reinitialise_eye_lens(game.applied_lens_type);
    // Apply the appropriate palette (lens palette if active, otherwise engine default)
    PaletteSetUserPalette(player->user_id, local_state.lens_palette ? local_state.lens_palette : engine_palette);
    init_local_cameras(player);
    // Update the lights system state
    light_import_system_state(&game.lightst);
    // Victory state
    if (player->victory_state != VicS_Undecided)
    {
      frontstats_initialise();
      struct Dungeon* dungeon = get_players_dungeon(player);
      dungeon->lvstats.player_score = 0;
      dungeon->lvstats.allow_save_score = 1;
    }
    game.loaded_swipe_idx = -1;
    JUSTMSG("Loaded level %d from %s", game.continue_level_number, campaign.name);

    api_event("GAME_LOADED");

    return true;
}

int count_valid_saved_games(void)
{
  number_of_saved_games = 0;
  for (int i = 0; i < save_game_catalogue_count; i++)
  {
      struct CatalogueEntry* centry = &save_game_catalogue[i];
      if ((centry->flags & CEF_InUse) != 0)
          number_of_saved_games++;
  }
  return number_of_saved_games;
}

TbBool fill_game_catalogue_entry(struct CatalogueEntry *centry,const char *textname)
{
    centry->level_num = get_loaded_level_number();
    snprintf(centry->textname, SAVE_TEXTNAME_LEN, "%s", textname);
    snprintf(centry->campaign_name, LINEMSG_SIZE, "%s", campaign.name);
    const char *cmpgn_pfx = "";
    for (int i = 0; i < CampgnT_COUNT; i++) {
        if ((cmpgn_fgroup[i] == campaign.fgroup) && (cmpgn_prefix[i] != NULL)) {
            cmpgn_pfx = cmpgn_prefix[i];
            break;
        }
    }
    snprintf(centry->campaign_fname, DISKPATH_SIZE, "%s%s", cmpgn_pfx, campaign.fname);
    snprintf(centry->player_name, PLAYER_NAME_LENGTH, "%s", high_score_entry);
    set_flag(centry->flags, CEF_InUse);
    centry->game_ver_major = VER_MAJOR;
    centry->game_ver_minor = VER_MINOR;
    centry->game_ver_release = VER_RELEASE;
    centry->game_ver_build = VER_BUILD;
    return true;
}

TbBool fill_game_catalogue_slot(long slot_num,const char *textname)
{
    if (!ensure_catalogue_slot(slot_num))
    {
        ERRORLOG("Outranged slot index %d",(int)slot_num);
        return false;
    }
    struct CatalogueEntry* centry = &save_game_catalogue[slot_num];
    return fill_game_catalogue_entry(centry,textname);
}

TbBool game_catalogue_slot_disable(struct CatalogueEntry *game_catalg,unsigned int slot_idx)
{
  if (slot_idx >= (unsigned int)save_game_catalogue_count)
    return false;
  clear_flag(game_catalg[slot_idx].flags, CEF_InUse);
  return true;
}

TbBool save_catalogue_slot_disable(unsigned int slot_idx)
{
  return game_catalogue_slot_disable(save_game_catalogue,slot_idx);
}

TbBool load_catalogue_entry(TbFileHandle fh,struct FileChunkHeader *hdr,struct CatalogueEntry *centry)
{
    clear_flag(centry->flags, CEF_InUse);
    if ((hdr->id == SGC_InfoBlock) && (hdr->len == sizeof(struct CatalogueEntry)))
    {
        if (LbFileRead(fh, centry, sizeof(struct CatalogueEntry))
          == sizeof(struct CatalogueEntry))
        {
            set_flag(centry->flags, CEF_InUse);
        }
    }
    centry->textname[SAVE_TEXTNAME_LEN-1] = '\0';
    centry->campaign_name[LINEMSG_SIZE-1] = '\0';
    centry->campaign_fname[DISKPATH_SIZE-1] = '\0';
    centry->player_name[PLAYER_NAME_LENGTH-1] = '\0';
    return ((centry->flags & CEF_InUse) != 0);
}


TbBool load_game_save_catalogue(void)
{
    // Scan the save directory to find the highest existing slot index, so the catalogue
    // can be sized to exactly the saves that exist plus one free slot to save into.
    long highest = -1;
    struct TbFileEntry fe;
    char* spec = prepare_file_path(FGrp_Save, "fx1g*.sav");
    struct TbFileFind* ff = LbFileFindFirst(spec, &fe);
    if (ff != NULL)
    {
        do {
            int idx = save_slot_index_from_filename(fe.Filename);
            if (idx > highest)
                highest = idx;
        } while (LbFileFindNext(ff, &fe) >= 0);
        LbFileFindEnd(ff);
    }
    long needed = highest + 2;               // used slots + one free slot to save into
    if (needed < SAVE_SLOTS_MIN)
        needed = SAVE_SLOTS_MIN;
    if (needed > SAVE_SLOTS_LIMIT)
        needed = SAVE_SLOTS_LIMIT;
    if (!ensure_catalogue_slot(needed - 1))
        return false;
    save_game_catalogue_count = needed;      // logical length the menus range over

    // (Re)load metadata for every slot in range; missing files leave a zeroed (free) entry.
    long saves_found = 0;
    for (long slot_num = 0; slot_num < save_game_catalogue_count; slot_num++)
    {
        struct CatalogueEntry* centry = &save_game_catalogue[slot_num];
        memset(centry, 0, sizeof(struct CatalogueEntry));
        char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
        TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
        if (!fh)
            continue;
        struct FileChunkHeader hdr;
        if (LbFileRead(fh, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        {
            if (load_catalogue_entry(fh,&hdr,centry))
                saves_found++;
        }
        LbFileClose(fh);
    }
    return (saves_found > 0);
}

TbBool initialise_load_game_slots(void)
{
    load_game_save_catalogue();
    return (count_valid_saved_games() > 0);
}

static void get_progress_filename(const char *cmpgn_fname, char *out, size_t outlen)
{
    char name[CAMPAIGN_FNAME_LEN];
    get_campaign_sanitized_id(cmpgn_fname, name, sizeof(name));
    snprintf(out, outlen, "%s.cid", name);
}


static TbBool read_progress_file(const char *progress_fname, struct IntralevelData *ilvl)
{
    char* fname = prepare_file_path(FGrp_Save, progress_fname);
    TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (!fh)
        return false;
    TbBool result = false;
    while (!LbFileEof(fh))
    {
        struct FileChunkHeader hdr;
        if (LbFileRead(fh, &hdr, sizeof(struct FileChunkHeader)) != sizeof(struct FileChunkHeader))
            break;
        if (hdr.id == SGC_IntralevelData)
        {
            result = read_intralevel_chunk(fh, &hdr, ilvl);
            break;
        }
        WARNLOG("Unrecognized chunk in \"%s\", ID = %08" PRIx32, progress_fname, hdr.id);
        skip_chunk(fh, &hdr);
    }
    LbFileClose(fh);
    return result;
}

static TbBool write_progress_file(const char *progress_fname, const struct IntralevelData *ilvl)
{
    char* fname = prepare_file_path(FGrp_Save, progress_fname);
    TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (!fh)
    {
        WARNMSG("Cannot open progress file \"%s\".", fname);
        return false;
    }
    TbBool result = write_chunk(fh, SGC_IntralevelData, INTRALEVEL_DATA_VER, ilvl, sizeof(struct IntralevelData));
    LbFileClose(fh);
    return result;
}

static TbFileHandle open_legacy_progress(void)
{
    char* fname = prepare_file_path(FGrp_Save, legacy_campaign_progress);
    if (LbFileLength(fname) != (long)LEGACY_PROGRESS_FILE_SIZE)
        return NULL;
    return LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
}

static TbBool read_legacy_progress_fname(char *cmpgn_fname)
{
    TbFileHandle fh = open_legacy_progress();
    if (!fh)
        return false;
    TbBool result = (LbFileRead(fh, cmpgn_fname, CAMPAIGN_FNAME_LEN) == CAMPAIGN_FNAME_LEN);
    LbFileClose(fh);
    cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    return result;
}

static TbBool legacy_progress_matches(const char *cmpgn_fname)
{
    char legacy_fname[CAMPAIGN_FNAME_LEN];
    return read_legacy_progress_fname(legacy_fname) && (strcasecmp(legacy_fname, cmpgn_fname) == 0);
}

static TbBool read_legacy_progress(const char *cmpgn_fname, struct IntralevelData *ilvl)
{
    TbFileHandle fh = open_legacy_progress();
    if (!fh)
        return false;
    char legacy_fname[CAMPAIGN_FNAME_LEN];
    LevelNumber lvnum = 0;
    memset(ilvl, 0, sizeof(struct IntralevelData));
    TbBool result = (LbFileRead(fh, legacy_fname, CAMPAIGN_FNAME_LEN) == CAMPAIGN_FNAME_LEN)
        && (LbFileRead(fh, &lvnum, sizeof(lvnum)) == sizeof(lvnum))
        && (LbFileRead(fh, ilvl, INTRALEVEL_DATA_V0_SIZE) == (int)INTRALEVEL_DATA_V0_SIZE);
    LbFileClose(fh);
    legacy_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    if (!result || (strcasecmp(legacy_fname, cmpgn_fname) != 0))
        return false;
    ilvl->continue_level = encode_continue_level(lvnum);
    return true;
}

static void delete_legacy_progress(void)
{
    LbFileDelete(prepare_file_path(FGrp_Save, legacy_campaign_progress));
}

static TbBool load_campaign_progress(const struct GameCampaign *campgn, struct IntralevelData *ilvl, LevelNumber *continue_lvnum)
{
    char progress_fname[DISKPATH_SIZE];
    get_progress_filename(campgn->fname, progress_fname, sizeof(progress_fname));
    TbBool found = read_progress_file(progress_fname, ilvl);
    if (!found)
        found = read_legacy_progress(campgn->fname, ilvl);
    if (!found)
        return false;
    *continue_lvnum = resolve_campaign_progress(ilvl, campgn);
    return true;
}

static TbBool read_last_file_link(char *link, size_t linklen)
{
    char* fname = prepare_file_path(FGrp_Save, continue_filename);
    TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (!fh)
        return false;
    TbBool result = false;
    while (!LbFileEof(fh))
    {
        struct FileChunkHeader hdr;
        if (LbFileRead(fh, &hdr, sizeof(struct FileChunkHeader)) != sizeof(struct FileChunkHeader))
            break;
        if ((hdr.id == SGC_Continue) && (hdr.ver == CONTINUE_DATA_VER) && (hdr.len == sizeof(struct ContinueData)))
        {
            struct ContinueData cdata;
            if (LbFileRead(fh, &cdata, sizeof(cdata)) == sizeof(cdata))
            {
                cdata.link_fname[SAVE_FILENAME_MAX-1] = '\0';
                snprintf(link, linklen, "%s", cdata.link_fname);
                result = (link[0] != '\0');
            }
            break;
        }
        skip_chunk(fh, &hdr);
    }
    LbFileClose(fh);
    return result;
}

static TbBool write_last_file_link(const char *link)
{
    struct ContinueData cdata;
    memset(&cdata, 0, sizeof(cdata));
    snprintf(cdata.link_fname, sizeof(cdata.link_fname), "%s", link);
    char* fname = prepare_file_path(FGrp_Save, continue_filename);
    TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (!fh)
    {
        WARNMSG("Cannot open continue file \"%s\".", fname);
        return false;
    }
    TbBool result = write_chunk(fh, SGC_Continue, CONTINUE_DATA_VER, &cdata, sizeof(cdata));
    LbFileClose(fh);
    return result;
}

static void delete_last_file_link(void)
{
    LbFileDelete(prepare_file_path(FGrp_Save, continue_filename));
}

static void update_last_file_after_save(int32_t slot_num)
{
    int humans = 0;
    int non_computer = 0;
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if (flag_is_set(player->allocflags, PlaF_OriginallyHuman))
            humans++;
        if (flag_is_set(player->allocflags, PlaF_Allocated) && !flag_is_set(player->allocflags, PlaF_CompCtrl))
            non_computer++;
    }
    // saves from before PlaF_OriginallyHuman existed have no flagged player
    if (humans == 0)
        humans = non_computer;
    char link[SAVE_FILENAME_MAX];
    snprintf(link, sizeof(link), saved_game_filename, (int)slot_num);
    if (humans == 1)
    {
        write_last_file_link(link);
        return;
    }
    char current[SAVE_FILENAME_MAX];
    if (read_last_file_link(current, sizeof(current)) && (strcasecmp(current, link) == 0))
        delete_last_file_link();
}

static TbBool read_saved_game_catalogue_entry(int32_t slot_num, struct CatalogueEntry *centry)
{
    memset(centry, 0, sizeof(struct CatalogueEntry));
    char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
    TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (!fh)
        return false;
    struct FileChunkHeader hdr;
    TbBool loaded = (LbFileRead(fh, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        && load_catalogue_entry(fh, &hdr, centry);
    LbFileClose(fh);
    return loaded;
}

static enum ContinueTargets find_continue_target(int32_t *slot_num, char *cmpgn_fname, size_t fnamelen)
{
    if (campaigns_list.items_num == 1)
    {
        struct GameCampaign* campgn = &campaigns_list.items[0];
        if (!campaign_progress_exists(campgn))
            return CntT_None;
        snprintf(cmpgn_fname, fnamelen, "%s", campgn->fname);
        return CntT_CampaignProgress;
    }
    char link[SAVE_FILENAME_MAX];
    if (read_last_file_link(link, sizeof(link)))
    {
        int slot = save_slot_index_from_filename(link);
        if (slot >= 0)
        {
            struct CatalogueEntry centry;
            if (read_saved_game_catalogue_entry(slot, &centry))
            {
                *slot_num = slot;
                return CntT_SavedGame;
            }
        } else
        {
            for (unsigned long i = 0; i < campaigns_list.items_num; i++)
            {
                struct GameCampaign* campgn = &campaigns_list.items[i];
                char progress_fname[DISKPATH_SIZE];
                get_progress_filename(campgn->fname, progress_fname, sizeof(progress_fname));
                if ((strcasecmp(link, progress_fname) == 0) && LbFileExists(prepare_file_path(FGrp_Save, progress_fname)))
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

/**
 * Indicates whether continue game option is available.
 */
TbBool continue_game_available(void)
{
    int32_t slot_num = -1;
    char cmpgn_fname[DISKPATH_SIZE];
    return (find_continue_target(&slot_num, cmpgn_fname, sizeof(cmpgn_fname)) != CntT_None);
}

enum ContinueTargets load_continue_game(void)
{
    int32_t slot_num = -1;
    char cmpgn_fname[DISKPATH_SIZE];
    switch (find_continue_target(&slot_num, cmpgn_fname, sizeof(cmpgn_fname)))
    {
    case CntT_SavedGame:
        load_game_save_catalogue();
        if ((slot_num >= save_game_catalogue_count) || ((save_game_catalogue[slot_num].flags & CEF_InUse) == 0))
            break;
        game.save_game_slot = slot_num;
        return CntT_SavedGame;
    case CntT_CampaignProgress:
        if (resume_campaign_progress(cmpgn_fname))
            return CntT_CampaignProgress;
        break;
    default:
        break;
    }
    return CntT_None;
}

TbBool campaign_progress_exists(const struct GameCampaign *campgn)
{
    char progress_fname[DISKPATH_SIZE];
    get_progress_filename(campgn->fname, progress_fname, sizeof(progress_fname));
    if (LbFileExists(prepare_file_path(FGrp_Save, progress_fname)))
        return true;
    return legacy_progress_matches(campgn->fname);
}

TbBool any_campaign_progress_exists(void)
{
    for (unsigned long i = 0; i < campaigns_list.items_num; i++)
    {
        if (campaign_progress_exists(&campaigns_list.items[i]))
            return true;
    }
    return false;
}

void update_campaigns_progress_percent(struct CampaignsList *clist)
{
    for (unsigned long i = 0; i < clist->items_num; i++)
    {
        struct GameCampaign* campgn = &clist->items[i];
        LevelNumber continue_lvnum;
        if (load_campaign_progress(campgn, &progress_scratch, &continue_lvnum))
            campgn->progress_percent = campaign_progress_percent(campgn, &progress_scratch, continue_lvnum);
        else
            campgn->progress_percent = -1;
    }
}

TbBool resume_campaign_progress(const char *cmpgn_fname)
{
    if (!change_campaign(CampgnT_Campaign, cmpgn_fname))
    {
        ERRORLOG("Unable to load campaign \"%s\"", cmpgn_fname);
        return false;
    }
    LevelNumber lvnum;
    if (!load_campaign_progress(&campaign, &progress_scratch, &lvnum))
        return false;
    if (!is_singleplayer_like_level(lvnum))
    {
        WARNLOG("Continue level %d of \"%s\" is incorrect", (int)lvnum, cmpgn_fname);
        return false;
    }
    memcpy(&intralvl, &progress_scratch, sizeof(struct IntralevelData));
    set_continue_level_number(lvnum);
    snprintf(game.campaign_fname, sizeof(game.campaign_fname), "%s", campaign.fname);
    update_extra_levels_visibility();
    JUSTMSG("Continued level %d from %s", (int)lvnum, campaign.name);
    return true;
}

// writes campaign progress file; on a win also records the level and updates the continue file
TbBool save_level_progress(LevelNumber lvnum, TbBool won)
{
    if (won && is_campaign_progress_level(&campaign, lvnum))
        mark_level_completed(&intralvl, lvnum);
    intralvl.continue_level = encode_continue_level(get_continue_level_number());
    char progress_fname[DISKPATH_SIZE];
    get_progress_filename(campaign.fname, progress_fname, sizeof(progress_fname));
    if (!write_progress_file(progress_fname, &intralvl))
        return false;

    // continue
    if (won)
        write_last_file_link(progress_fname);

    // clean up legacy file if this is a replacement
    if (legacy_progress_matches(campaign.fname))
        delete_legacy_progress();
    return true;
}

// check if saved game is for the given campaign
static TbBool saved_game_belongs_to(int32_t slot_num, const struct GameCampaign *campgn)
{
    struct CatalogueEntry centry;
    if (!read_saved_game_catalogue_entry(slot_num, &centry))
        return false;
    char centry_fname[DISKPATH_SIZE];
    prepare_campaign_file_name(centry.campaign_fname, centry_fname, sizeof(centry_fname));
    if (strcasecmp(centry_fname, campgn->fname) != 0)
        return false;
    return is_campaign_progress_level(campgn, centry.level_num);
}

TbBool erase_progress(const struct GameCampaign *campgn)
{
    char progress_fname[DISKPATH_SIZE];
    get_progress_filename(campgn->fname, progress_fname, sizeof(progress_fname));
    LbFileDelete(prepare_file_path(FGrp_Save, progress_fname));
    if (legacy_progress_matches(campgn->fname))
        delete_legacy_progress();
    char link[SAVE_FILENAME_MAX];
    if (read_last_file_link(link, sizeof(link)))
    {
        int slot = save_slot_index_from_filename(link);
        if ((strcasecmp(link, progress_fname) == 0) || ((slot >= 0) && saved_game_belongs_to(slot, campgn)))
            delete_last_file_link();
    }
    JUSTMSG("Erased progress of %s", campgn->fname);
    return true;
}

TbBool add_transfered_creature(PlayerNumber plyr_idx, ThingModel model, CrtrExpLevel exp_level, char *name)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
    {
        ERRORDBG(11, "Can't transfer creature; player %d has no dungeon.", (int)plyr_idx);
        return false;
    }

    short i = dungeon->creatures_transferred; //makes sure it fits 255 units

    intralvl.transferred_creatures[plyr_idx][i].model = model;
    intralvl.transferred_creatures[plyr_idx][i].exp_level = exp_level;
    strcpy(intralvl.transferred_creatures[plyr_idx][i].creature_name, name);
    return true;
}

void clear_transfered_creatures(void)
{
    for (int p = 0; p < PLAYERS_COUNT; p++)
    {
        for (int i = 0; i < TRANSFER_CREATURE_STORAGE_COUNT; i++)
        {
            intralvl.transferred_creatures[p][i].model = 0;
            intralvl.transferred_creatures[p][i].exp_level = 0;
        }
    }
}

LevelNumber move_campaign_to_next_level(void)
{
    LevelNumber curr_lvnum = get_continue_level_number();
    LevelNumber lvnum = next_singleplayer_level(curr_lvnum, false);
    SYNCDBG(15,"Campaign move %d to %d",curr_lvnum,lvnum);
    {
        struct PlayerInfo* player = get_my_player();
        player->display_flags &= ~PlaF6_PlyrHasQuit;
    }
    if (lvnum != LEVELNUMBER_ERROR)
    {
        curr_lvnum = set_continue_level_number(lvnum);
        SYNCDBG(8,"Continue level moved to %d.",curr_lvnum);
        return curr_lvnum;
    } else
    {
        curr_lvnum = set_continue_level_number(SINGLEPLAYER_NOTSTARTED);
        SYNCDBG(8,"Continue level moved to NOTSTARTED.");
        return curr_lvnum;
    }
}

LevelNumber move_campaign_to_prev_level(void)
{
    LevelNumber curr_lvnum = get_continue_level_number();
    LevelNumber lvnum = prev_singleplayer_level(curr_lvnum);
    SYNCDBG(15,"Campaign move %d to %d",curr_lvnum,lvnum);
    if (lvnum != LEVELNUMBER_ERROR)
    {
        curr_lvnum = set_continue_level_number(lvnum);
        SYNCDBG(8,"Continue level moved to %d.",curr_lvnum);
        return curr_lvnum;
    } else
    {
        curr_lvnum = set_continue_level_number(SINGLEPLAYER_FINISHED);
        SYNCDBG(8,"Continue level moved to FINISHED.");
        return curr_lvnum;
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
