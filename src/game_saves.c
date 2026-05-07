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
#include "config_creature.h"
#include "config_compp.h"
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
#include "keeperfx.hpp"
#include "api.h"
#include "lvl_filesdk1.h"
#include "lua_base.h"
#include "lua_triggers.h"
#include "moonphase.h"
#include "post_inc.h"

#include <sys/stat.h>
#include <time.h>
#include <zlib.h>

#define FULL_PATH_BUFFER_SIZE 2048

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
TbBool load_catalogue_entry(TbFileHandle fh,struct FileChunkHeader *hdr,struct CatalogueEntry *centry);
static void build_save_dir_for(const char *cmpgn_fname, char *out, size_t out_size);
/******************************************************************************/
const short VersionMajor    = VER_MAJOR;
const short VersionMinor    = VER_MINOR;
short const VersionRelease  = VER_RELEASE;
short const VersionBuild    = VER_BUILD;

const char *continue_game_filename="fx1contn.sav";
const char *saved_game_filename="fx1g%04d.sav";
const char *packet_filename="fx1rp%04d.pck";

struct CatalogueEntry save_game_catalogue[TOTAL_SAVE_SLOTS_COUNT];

int number_of_saved_games;

struct GlobalSaveEntry global_save_entries[MAX_GLOBAL_SAVES];
int global_save_count;
int global_load_scroll_offset;
TbBool global_load_is_all_campaigns;

static const char* get_safe_campaign_name(void);

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
        hdr.ver = 0;
        hdr.len = sizeof(struct CatalogueEntry);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, centry, sizeof(struct CatalogueEntry)) == sizeof(struct CatalogueEntry))
            chunks_done |= SGF_InfoBlock;
    }
    { // Game data chunk — zero wasteful fields, compress with zlib
        // Back up fields we'll zero (they're rebuilt on load)
        struct Configs *conf_backup = (struct Configs *)malloc(sizeof(struct Configs));
        void *nav_backup = malloc(sizeof(game.navigation_map));
        if (!conf_backup || !nav_backup) {
            ERRORLOG("Failed to allocate backup buffers for save compression");
            free(conf_backup);
            free(nav_backup);
            return false;
        }
        memcpy(conf_backup, &game.conf, sizeof(struct Configs));
        memcpy(nav_backup, game.navigation_map, sizeof(game.navigation_map));

        // Zero wasteful fields (~5.5 MB of data that's rebuilt on load)
        memset(&game.conf, 0, sizeof(struct Configs));
        memset(game.navigation_map, 0, sizeof(game.navigation_map));
        memset(&game.log_snapshot, 0, sizeof(game.log_snapshot));
        memset(&game.host_checksums, 0, sizeof(game.host_checksums));

        // Compress the game struct
        uLongf compressed_bound = compressBound(sizeof(struct Game));
        unsigned char *compressed = (unsigned char *)malloc(compressed_bound);
        if (!compressed) {
            ERRORLOG("Failed to allocate compression buffer");
            memcpy(&game.conf, conf_backup, sizeof(struct Configs));
            memcpy(game.navigation_map, nav_backup, sizeof(game.navigation_map));
            free(conf_backup);
            free(nav_backup);
            return false;
        }

        uLongf compressed_size = compressed_bound;
        int zret = compress2(compressed, &compressed_size,
                             (const Bytef *)&game, sizeof(struct Game), Z_DEFAULT_COMPRESSION);

        // Restore zeroed fields immediately
        memcpy(&game.conf, conf_backup, sizeof(struct Configs));
        memcpy(game.navigation_map, nav_backup, sizeof(game.navigation_map));
        free(conf_backup);
        free(nav_backup);

        if (zret != Z_OK) {
            ERRORLOG("zlib compress failed: %d", zret);
            free(compressed);
            return false;
        }

        SYNCLOG("Save compressed: %lu -> %lu bytes (%.1f%%)",
                (unsigned long)sizeof(struct Game), (unsigned long)compressed_size,
                100.0f * compressed_size / sizeof(struct Game));

        // Write header: ver=1 means compressed, len=decompressed size
        hdr.id = SGC_GameOrig;
        hdr.ver = 1;
        hdr.len = sizeof(struct Game);
        unsigned long comp_len = (unsigned long)compressed_size;
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, &comp_len, sizeof(unsigned long)) == sizeof(unsigned long))
        if (LbFileWrite(fhandle, compressed, compressed_size) == compressed_size)
            chunks_done |= SGF_GameOrig;

        free(compressed);
    }
    { // IntralevelData data chunk
        hdr.id = SGC_IntralevelData;
        hdr.ver = 0;
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
        hdr.ver = 0;
        hdr.len = sizeof(struct PacketSaveHead);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, &game.packet_save_head, sizeof(struct PacketSaveHead)) == sizeof(struct PacketSaveHead))
            chunks_done |= SGF_PacketHeader;
    }
    { // Info chunk
        hdr.id = SGC_InfoBlock;
        hdr.ver = 0;
        hdr.len = sizeof(struct CatalogueEntry);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, centry, sizeof(struct CatalogueEntry)) == sizeof(struct CatalogueEntry))
            chunks_done |= SGF_InfoBlock;
    }
    // If it's not start of a level, save progress data too
    if (game.play_gameturn != 0)
    {
        { // Game data chunk — compressed
            // Zero wasteful fields
            struct Configs *conf_backup = (struct Configs *)malloc(sizeof(struct Configs));
            void *nav_backup = malloc(sizeof(game.navigation_map));
            if (!conf_backup || !nav_backup) {
                WARNLOG("Failed to allocate backup buffers for packet save compression");
                free(conf_backup);
                free(nav_backup);
            } else {
                memcpy(conf_backup, &game.conf, sizeof(struct Configs));
                memcpy(nav_backup, game.navigation_map, sizeof(game.navigation_map));
                memset(&game.conf, 0, sizeof(struct Configs));
                memset(game.navigation_map, 0, sizeof(game.navigation_map));
                memset(&game.log_snapshot, 0, sizeof(game.log_snapshot));
                memset(&game.host_checksums, 0, sizeof(game.host_checksums));

                uLongf compressed_bound = compressBound(sizeof(struct Game));
                unsigned char *compressed = (unsigned char *)malloc(compressed_bound);
                if (compressed) {
                    uLongf compressed_size = compressed_bound;
                    if (compress2(compressed, &compressed_size,
                                  (const Bytef *)&game, sizeof(struct Game), Z_DEFAULT_COMPRESSION) == Z_OK) {
                        hdr.id = SGC_GameOrig;
                        hdr.ver = 1;
                        hdr.len = sizeof(struct Game);
                        unsigned long comp_len = (unsigned long)compressed_size;
                        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
                        if (LbFileWrite(fhandle, &comp_len, sizeof(unsigned long)) == sizeof(unsigned long))
                        if (LbFileWrite(fhandle, compressed, compressed_size) == compressed_size)
                            chunks_done |= SGF_GameOrig;
                    }
                    free(compressed);
                }
                memcpy(&game.conf, conf_backup, sizeof(struct Configs));
                memcpy(game.navigation_map, nav_backup, sizeof(game.navigation_map));
                free(conf_backup);
                free(nav_backup);
            }
        }
    }
    { // Packet file data start indicator
        hdr.id = SGC_PacketData;
        hdr.ver = 0;
        hdr.len = 0;
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
            chunks_done |= SGF_PacketData;
    }
    if ((chunks_done != SGF_PacketStart) && (chunks_done != SGF_PacketContinue))
        return false;
    return true;
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
            if (load_catalogue_entry(fhandle, &hdr, centry))
            {
                chunks_done |= SGF_InfoBlock;
                if (!change_campaign(centry->campaign_fname)) {
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
                check_and_auto_fix_stats();
                init_creature_scores();
                snprintf(high_score_entry, PLAYER_NAME_LENGTH, "%s", centry->player_name);
            }
            break;
        case SGC_GameOrig:
            if (hdr.len != sizeof(struct Game))
            {
                if (hdr.ver == 1)
                {
                    // Compressed: skip 4-byte comp_len + compressed data
                    unsigned long skip_comp_len = 0;
                    if (LbFileRead(fhandle, &skip_comp_len, sizeof(unsigned long)) == sizeof(unsigned long))
                    {
                        if (LbFileSeek(fhandle, skip_comp_len, Lb_FILE_SEEK_CURRENT) < 0)
                            LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
                    }
                }
                else
                {
                    if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                        LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
                }
                WARNLOG("Incompatible GameOrig chunk (expected %lu, got %lu)",
                        (unsigned long)sizeof(struct Game), (unsigned long)hdr.len);
                break;
            }
            if (hdr.ver == 1)
            {
                // Compressed format: read 4-byte compressed_size, then compressed data
                unsigned long comp_len = 0;
                if (LbFileRead(fhandle, &comp_len, sizeof(unsigned long)) != sizeof(unsigned long))
                {
                    WARNLOG("Could not read compressed size");
                    break;
                }
                unsigned char *comp_data = (unsigned char *)malloc(comp_len);
                if (!comp_data)
                {
                    WARNLOG("Failed to allocate decompression buffer (%lu bytes)", comp_len);
                    break;
                }
                if (LbFileRead(fhandle, comp_data, comp_len) != (long)comp_len)
                {
                    WARNLOG("Could not read compressed GameOrig data");
                    free(comp_data);
                    break;
                }
                uLongf decomp_size = sizeof(struct Game);
                int zret = uncompress((Bytef *)&game, &decomp_size, comp_data, comp_len);
                free(comp_data);
                if (zret != Z_OK || decomp_size != sizeof(struct Game))
                {
                    WARNLOG("Decompression failed: zlib=%d, got %lu expected %lu",
                            zret, (unsigned long)decomp_size, (unsigned long)sizeof(struct Game));
                    break;
                }
                SYNCLOG("Save decompressed: %lu -> %lu bytes", comp_len, (unsigned long)decomp_size);
                chunks_done |= SGF_GameOrig;
            }
            else
            {
                // Legacy uncompressed format (ver=0)
                if (LbFileRead(fhandle, &game, sizeof(struct Game)) == sizeof(struct Game)) {
                    chunks_done |= SGF_GameOrig;
                } else {
                    WARNLOG("Could not read GameOrig chunk");
                }
            }
            break;
        case SGC_PacketHeader:
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
            if (hdr.len != sizeof(struct IntralevelData))
            {
                if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                    LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
                WARNLOG("Incompatible IntralevelData chunk");
                break;
            }
            if (LbFileRead(fhandle, &intralvl, sizeof(struct IntralevelData)) == sizeof(struct IntralevelData)) {
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
            WARNLOG("Unrecognized chunk, ID = %08lx", hdr.id);
            if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
            break;
        }
    }
    if ((chunks_done & SGF_SavedGame) == SGF_SavedGame)
    {
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
    if ((slot_num < 0) || (slot_num >= TOTAL_SAVE_SLOTS_COUNT))
    {
        ERRORLOG("Outranged slot index %d",(int)slot_num);
        return false;
    }
    char* fname = prepare_campaign_save_fmtpath(saved_game_filename, slot_num);
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
    api_event("GAME_SAVED");
    return true;
}

TbBool is_save_game_loadable(long slot_num)
{
    // Prepare filename and open the file
    char* fname = prepare_campaign_save_fmtpath(saved_game_filename, slot_num);
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
    if ((slot_num < 0) || (slot_num >= TOTAL_SAVE_SLOTS_COUNT))
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
        char* fname = prepare_campaign_save_fmtpath(saved_game_filename, slot_num);
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
    snprintf(game.campaign_fname, sizeof(game.campaign_fname), "%s", campaign.fname);
    reinit_level_after_load();
    output_message(SMsg_GameLoaded, 0);
    panel_map_update(0, 0, game.map_subtiles_x+1, game.map_subtiles_y+1);
    calculate_moon_phase(false,false);
    update_extra_levels_visibility();
    struct PlayerInfo* player = get_my_player();
    clear_flag(player->additional_flags, PlaAF_LightningPaletteIsActive);
    clear_flag(player->additional_flags, PlaAF_FreezePaletteIsActive);
    player->palette_fade_step_pain = 0;
    player->palette_fade_step_possession = 0;
    player->lens_palette = 0;
    // Reinitialize lens first (restores lens_palette pointer from config)
    reinitialise_eye_lens(game.applied_lens_type);
    // Apply the appropriate palette (lens palette if active, otherwise engine default)
    PaletteSetPlayerPalette(player, player->lens_palette ? player->lens_palette : engine_palette);
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
  for (int i = 0; i < TOTAL_SAVE_SLOTS_COUNT; i++)
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
    snprintf(centry->campaign_fname, DISKPATH_SIZE, "%s", campaign.fname);
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
    if ((slot_num < 0) || (slot_num >= TOTAL_SAVE_SLOTS_COUNT))
    {
        ERRORLOG("Outranged slot index %d",(int)slot_num);
        return false;
    }
    struct CatalogueEntry* centry = &save_game_catalogue[slot_num];
    return fill_game_catalogue_entry(centry,textname);
}

TbBool game_catalogue_slot_disable(struct CatalogueEntry *game_catalg,unsigned int slot_idx)
{
  if (slot_idx >= TOTAL_SAVE_SLOTS_COUNT)
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
    long saves_found = 0;
    for (long slot_num = 0; slot_num < TOTAL_SAVE_SLOTS_COUNT; slot_num++)
    {
        struct CatalogueEntry* centry = &save_game_catalogue[slot_num];
        memset(centry, 0, sizeof(struct CatalogueEntry));
        char* fname = prepare_campaign_save_fmtpath(saved_game_filename, slot_num);
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

static TbBool migrate_file(const char *src, const char *dst)
{
    create_directory_for_file(dst);
    if (rename(src, dst) == 0) {
        SYNCLOG("Migrated save: %s -> %s", src, dst);
        return true;
    } else {
        WARNLOG("Failed to migrate save: %s -> %s (errno %d: %s)", src, dst, errno, strerror(errno));
        return false;
    }
}

/**
 * @brief One-time migration of saves from flat save/ to per-campaign subdirectories.
 * Scans for old-format slot saves (fx1g*.sav) in the flat save/ directory,
 * reads their InfoBlock to determine campaign_fname, and moves them to save/{campaign}/.
 * Also migrates old continue saves (fx1_*_contn_.sav) and high score files.
 * Writes a .migrated sentinel file when complete to prevent re-running.
 */
void migrate_saves_to_campaign_dirs(void)
{
    char *sentinel = prepare_file_path(FGrp_Save, ".migrated");
    if (LbFileExists(sentinel)) {
        return;
    }
    SYNCLOG("Migrating saves to per-campaign directories...");

    TbBool migrated = false;

    // Migrate slot saves (fx1g0000.sav .. fx1g0007.sav)
    for (int slot = 0; slot < TOTAL_SAVE_SLOTS_COUNT; slot++)
    {
        char *old_path = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot);
        if (!LbFileExists(old_path))
            continue;

        // Read the InfoBlock to find campaign_fname
        TbFileHandle fh = LbFileOpen(old_path, Lb_FILE_MODE_READ_ONLY);
        if (!fh)
            continue;
        struct FileChunkHeader hdr;
        struct CatalogueEntry centry;
        memset(&centry, 0, sizeof(centry));
        TbBool got_info = false;
        if (LbFileRead(fh, &hdr, sizeof(hdr)) == sizeof(hdr)) {
            got_info = load_catalogue_entry(fh, &hdr, &centry);
        }
        LbFileClose(fh);

        char safe_dir[CAMPAIGN_FNAME_LEN];
        if (got_info && centry.campaign_fname[0] != '\0') {
            snprintf(safe_dir, sizeof(safe_dir), "%s", centry.campaign_fname);
            // Strip .cfg extension if present
            char *dot = strrchr(safe_dir, '.');
            if (dot != NULL && strcasecmp(dot, ".cfg") == 0)
                *dot = '\0';
        } else {
            snprintf(safe_dir, sizeof(safe_dir), "unknown");
        }

        // Build destination path: save/{campaign}/fx1g{slot}.sav
        char dest_fname[DISKPATH_SIZE];
        char slot_name[32];
        snprintf(slot_name, sizeof(slot_name), saved_game_filename, slot);
        path_join(dest_fname, sizeof(dest_fname), safe_dir, slot_name);
        char *new_path = prepare_file_path(FGrp_Save, dest_fname);

        // Copy new_path to a local buffer since prepare_file_path uses a static buffer
        char new_path_buf[FULL_PATH_BUFFER_SIZE];
        snprintf(new_path_buf, sizeof(new_path_buf), "%s", new_path);

        // Re-resolve old_path since prepare_file_path overwrote the static buffer
        old_path = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot);

        if (!LbFileExists(new_path_buf)) {
            migrated |= migrate_file(old_path, new_path_buf);
        }
    }

    // Migrate old continue saves: fx1_*_contn_.sav and fx1contn.sav
    {
        char *search_path = prepare_file_path(FGrp_Save, "fx1*contn*.sav");
        struct TbFileEntry fe;
        struct TbFileFind *ff = LbFileFindFirst(search_path, &fe);
        if (ff) {
            do {
                // Try to extract campaign name from filename pattern "fx1_{name}_contn_.sav"
                const char *fn = fe.Filename;
                char safe_dir[CAMPAIGN_FNAME_LEN];
                snprintf(safe_dir, sizeof(safe_dir), "unknown");

                if (strncmp(fn, "fx1_", 4) == 0) {
                    const char *end = strstr(fn + 4, "_contn_");
                    if (end && end > fn + 4) {
                        int len = (int)(end - (fn + 4));
                        if (len >= (int)sizeof(safe_dir)) len = sizeof(safe_dir) - 1;
                        memcpy(safe_dir, fn + 4, len);
                        safe_dir[len] = '\0';
                    }
                }

                char *old_path = prepare_file_path(FGrp_Save, fn);
                char old_buf[FULL_PATH_BUFFER_SIZE];
                snprintf(old_buf, sizeof(old_buf), "%s", old_path);

                char dest_fname[DISKPATH_SIZE];
                path_join(dest_fname, sizeof(dest_fname), safe_dir, continue_game_filename);
                char *new_path = prepare_file_path(FGrp_Save, dest_fname);
                char new_buf[FULL_PATH_BUFFER_SIZE];
                snprintf(new_buf, sizeof(new_buf), "%s", new_path);

                if (!LbFileExists(new_buf)) {
                    migrated |= migrate_file(old_buf, new_buf);
                }
            } while (LbFileFindNext(ff, &fe) >= 0);
            LbFileFindEnd(ff);
        }
    }

    // Migrate high score files (scr_*.dat)
    {
        char *search_path = prepare_file_path(FGrp_Save, "scr_*.dat");
        struct TbFileEntry fe;
        struct TbFileFind *ff = LbFileFindFirst(search_path, &fe);
        if (ff) {
            do {
                // High score filenames like "scr_dkpr.dat" - we need to find which campaign owns this file.
                // We scan all known campaigns to match hiscore_fname.
                const char *fn = fe.Filename;
                char safe_dir[CAMPAIGN_FNAME_LEN];
                snprintf(safe_dir, sizeof(safe_dir), "unknown");

                for (long i = 0; i < (long)campaigns_list.items_num; i++) {
                    struct GameCampaign *cmpgn = &campaigns_list.items[i];
                    if (strcmp(cmpgn->hiscore_fname, fn) == 0) {
                        snprintf(safe_dir, sizeof(safe_dir), "%s", cmpgn->fname);
                        char *dot = strrchr(safe_dir, '.');
                        if (dot != NULL && strcasecmp(dot, ".cfg") == 0)
                            *dot = '\0';
                        break;
                    }
                }

                char *old_path = prepare_file_path(FGrp_Save, fn);
                char old_buf[FULL_PATH_BUFFER_SIZE];
                snprintf(old_buf, sizeof(old_buf), "%s", old_path);

                char dest_fname[DISKPATH_SIZE];
                path_join(dest_fname, sizeof(dest_fname), safe_dir, fn);
                char *new_path = prepare_file_path(FGrp_Save, dest_fname);
                char new_buf[FULL_PATH_BUFFER_SIZE];
                snprintf(new_buf, sizeof(new_buf), "%s", new_path);

                if (!LbFileExists(new_buf)) {
                    migrated |= migrate_file(old_buf, new_buf);
                }
            } while (LbFileFindNext(ff, &fe) >= 0);
            LbFileFindEnd(ff);
        }
    }

    SYNCLOG("Save migration complete: %s files moved.", migrated ? "some" : "no");

    // Write sentinel file
    TbFileHandle sfh = LbFileOpen(sentinel, Lb_FILE_MODE_NEW);
    if (sfh) {
        const char marker[] = "migrated";
        LbFileWrite(sfh, marker, sizeof(marker));
        LbFileClose(sfh);
    }
}

/**
 * @brief Migrate mappack saves from save/<pack>/ to save/freeplay/<pack>/.
 * Uses a separate sentinel (.freeplay_migrated) to run once.
 */
void migrate_freeplay_saves(void)
{
    char *sentinel = prepare_file_path(FGrp_Save, ".freeplay_migrated");
    if (LbFileExists(sentinel))
        return;

    TbBool migrated = false;
    for (unsigned long ci = 0; ci < mappacks_list.items_num; ci++)
    {
        struct GameCampaign *cmpgn = &mappacks_list.items[ci];
        if (cmpgn->fname[0] == '\0')
            continue;

        // Build old dir name (without freeplay/ prefix)
        char old_dir[CAMPAIGN_FNAME_LEN];
        snprintf(old_dir, sizeof(old_dir), "%s", cmpgn->fname);
        char *dot = strrchr(old_dir, '.');
        if (dot != NULL && strcasecmp(dot, ".cfg") == 0)
            *dot = '\0';

        // Build new dir name (with freeplay/ prefix)
        char new_dir[DISKPATH_SIZE];
        snprintf(new_dir, sizeof(new_dir), "freeplay/%s", old_dir);

        // Migrate slot saves
        for (int slot = 0; slot < TOTAL_SAVE_SLOTS_COUNT; slot++)
        {
            char slot_name[32];
            snprintf(slot_name, sizeof(slot_name), saved_game_filename, slot);

            char old_subpath[DISKPATH_SIZE];
            path_join(old_subpath, sizeof(old_subpath), old_dir, slot_name);
            char *old_full = prepare_file_path(FGrp_Save, old_subpath);
            char old_buf[FULL_PATH_BUFFER_SIZE];
            snprintf(old_buf, sizeof(old_buf), "%s", old_full);

            if (!LbFileExists(old_buf))
                continue;

            char new_subpath[DISKPATH_SIZE];
            path_join(new_subpath, sizeof(new_subpath), new_dir, slot_name);
            char *new_full = prepare_file_path(FGrp_Save, new_subpath);
            char new_buf[FULL_PATH_BUFFER_SIZE];
            snprintf(new_buf, sizeof(new_buf), "%s", new_full);

            if (!LbFileExists(new_buf))
                migrated |= migrate_file(old_buf, new_buf);
        }

        // Migrate continue save
        {
            char old_subpath[DISKPATH_SIZE];
            path_join(old_subpath, sizeof(old_subpath), old_dir, continue_game_filename);
            char *old_full = prepare_file_path(FGrp_Save, old_subpath);
            char old_buf[FULL_PATH_BUFFER_SIZE];
            snprintf(old_buf, sizeof(old_buf), "%s", old_full);

            if (LbFileExists(old_buf))
            {
                char new_subpath[DISKPATH_SIZE];
                path_join(new_subpath, sizeof(new_subpath), new_dir, continue_game_filename);
                char *new_full = prepare_file_path(FGrp_Save, new_subpath);
                char new_buf[FULL_PATH_BUFFER_SIZE];
                snprintf(new_buf, sizeof(new_buf), "%s", new_full);

                if (!LbFileExists(new_buf))
                    migrated |= migrate_file(old_buf, new_buf);
            }
        }

        // Migrate progress.json
        {
            char old_subpath[DISKPATH_SIZE];
            path_join(old_subpath, sizeof(old_subpath), old_dir, "progress.json");
            char *old_full = prepare_file_path(FGrp_Save, old_subpath);
            char old_buf[FULL_PATH_BUFFER_SIZE];
            snprintf(old_buf, sizeof(old_buf), "%s", old_full);

            if (LbFileExists(old_buf))
            {
                char new_subpath[DISKPATH_SIZE];
                path_join(new_subpath, sizeof(new_subpath), new_dir, "progress.json");
                char *new_full = prepare_file_path(FGrp_Save, new_subpath);
                char new_buf[FULL_PATH_BUFFER_SIZE];
                snprintf(new_buf, sizeof(new_buf), "%s", new_full);

                if (!LbFileExists(new_buf))
                    migrated |= migrate_file(old_buf, new_buf);
            }
        }

        // Migrate high score file
        if (cmpgn->hiscore_fname[0] != '\0')
        {
            char old_subpath[DISKPATH_SIZE];
            path_join(old_subpath, sizeof(old_subpath), old_dir, cmpgn->hiscore_fname);
            char *old_full = prepare_file_path(FGrp_Save, old_subpath);
            char old_buf[FULL_PATH_BUFFER_SIZE];
            snprintf(old_buf, sizeof(old_buf), "%s", old_full);

            if (LbFileExists(old_buf))
            {
                char new_subpath[DISKPATH_SIZE];
                path_join(new_subpath, sizeof(new_subpath), new_dir, cmpgn->hiscore_fname);
                char *new_full = prepare_file_path(FGrp_Save, new_subpath);
                char new_buf[FULL_PATH_BUFFER_SIZE];
                snprintf(new_buf, sizeof(new_buf), "%s", new_full);

                if (!LbFileExists(new_buf))
                    migrated |= migrate_file(old_buf, new_buf);
            }
        }
    }

    if (migrated)
        SYNCLOG("Freeplay save migration complete.");

    TbFileHandle sfh = LbFileOpen(sentinel, Lb_FILE_MODE_NEW);
    if (sfh) {
        const char marker[] = "freeplay_migrated";
        LbFileWrite(sfh, marker, sizeof(marker));
        LbFileClose(sfh);
    }
}

static int compare_global_saves_by_date(const void *a, const void *b)
{
    const struct GlobalSaveEntry *ea = (const struct GlobalSaveEntry *)a;
    const struct GlobalSaveEntry *eb = (const struct GlobalSaveEntry *)b;
    if (eb->modified_time > ea->modified_time) return 1;
    if (eb->modified_time < ea->modified_time) return -1;
    return 0;
}

/**
 * @brief Scan all campaign subdirectories for save files.
 * Populates global_save_entries[] with all saves across all campaigns,
 * sorted by modification time (most recent first).
 */
void scan_all_campaign_saves(void)
{
    global_save_count = 0;
    global_load_scroll_offset = 0;
    memset(global_save_entries, 0, sizeof(global_save_entries));

    // Scan both campaigns and mappacks
    struct CampaignsList *lists[] = { &campaigns_list, &mappacks_list };
    for (int li = 0; li < (int)(sizeof(lists)/sizeof(lists[0])); li++)
    {
        struct CampaignsList *clist = lists[li];
        for (unsigned long ci = 0; ci < clist->items_num; ci++)
        {
            struct GameCampaign *cmpgn = &clist->items[ci];
            char safe_dir[DISKPATH_SIZE];
            build_save_dir_for(cmpgn->fname, safe_dir, sizeof(safe_dir));

            for (int slot = 0; slot < TOTAL_SAVE_SLOTS_COUNT; slot++)
            {
                if (global_save_count >= MAX_GLOBAL_SAVES)
                    break;

                char slot_fname[32];
                snprintf(slot_fname, sizeof(slot_fname), saved_game_filename, slot);
                char subpath[DISKPATH_SIZE];
                path_join(subpath, sizeof(subpath), safe_dir, slot_fname);
                char *fullpath = prepare_file_path(FGrp_Save, subpath);

                if (!LbFileExists(fullpath))
                    continue;

                TbFileHandle fh = LbFileOpen(fullpath, Lb_FILE_MODE_READ_ONLY);
                if (!fh)
                    continue;
                struct FileChunkHeader hdr;
                struct CatalogueEntry centry;
                memset(&centry, 0, sizeof(centry));
                TbBool got_info = false;
                if (LbFileRead(fh, &hdr, sizeof(hdr)) == sizeof(hdr)) {
                    got_info = load_catalogue_entry(fh, &hdr, &centry);
                }
                LbFileClose(fh);

                if (!got_info)
                    continue;

                struct GlobalSaveEntry *entry = &global_save_entries[global_save_count];
                snprintf(entry->campaign_name, sizeof(entry->campaign_name), "%s", cmpgn->name);
                snprintf(entry->campaign_fname, sizeof(entry->campaign_fname), "%s", cmpgn->fname);
                snprintf(entry->save_textname, sizeof(entry->save_textname), "%s", centry.textname);
                snprintf(entry->save_dir, sizeof(entry->save_dir), "%s", safe_dir);
                entry->slot_num = slot;
                entry->in_use = true;

                // Get file modification time
                path_join(subpath, sizeof(subpath), safe_dir, slot_fname);
                fullpath = prepare_file_path(FGrp_Save, subpath);
                struct stat st;
                if (stat(fullpath, &st) == 0) {
                    entry->modified_time = (long)st.st_mtime;
                } else {
                    entry->modified_time = 0;
                }

                global_save_count++;
            }
            if (global_save_count >= MAX_GLOBAL_SAVES)
                break;
        }
    }

    // Sort by modification time (most recent first)
    if (global_save_count > 1) {
        qsort(global_save_entries, global_save_count, sizeof(struct GlobalSaveEntry),
              compare_global_saves_by_date);
    }

    SYNCLOG("Found %d saves across all campaigns", global_save_count);
}

/**
 * @brief Scan only the current campaign's save directory for save files.
 * Populates global_save_entries[] with saves for the active campaign only.
 */
void scan_current_campaign_saves(void)
{
    global_save_count = 0;
    global_load_scroll_offset = 0;
    memset(global_save_entries, 0, sizeof(global_save_entries));

    const char *safe_dir = get_safe_campaign_name();

    for (int slot = 0; slot < TOTAL_SAVE_SLOTS_COUNT; slot++)
    {
        if (global_save_count >= MAX_GLOBAL_SAVES)
            break;

        char slot_fname[32];
        snprintf(slot_fname, sizeof(slot_fname), saved_game_filename, slot);
        char subpath[DISKPATH_SIZE];
        path_join(subpath, sizeof(subpath), safe_dir, slot_fname);
        char *fullpath = prepare_file_path(FGrp_Save, subpath);

        if (!LbFileExists(fullpath))
            continue;

        TbFileHandle fh = LbFileOpen(fullpath, Lb_FILE_MODE_READ_ONLY);
        if (!fh)
            continue;
        struct FileChunkHeader hdr;
        struct CatalogueEntry centry;
        memset(&centry, 0, sizeof(centry));
        TbBool got_info = false;
        if (LbFileRead(fh, &hdr, sizeof(hdr)) == sizeof(hdr)) {
            got_info = load_catalogue_entry(fh, &hdr, &centry);
        }
        LbFileClose(fh);

        if (!got_info)
            continue;

        struct GlobalSaveEntry *entry = &global_save_entries[global_save_count];
        snprintf(entry->campaign_name, sizeof(entry->campaign_name), "%s", campaign.name);
        snprintf(entry->campaign_fname, sizeof(entry->campaign_fname), "%s", campaign.fname);
        snprintf(entry->save_textname, sizeof(entry->save_textname), "%s", centry.textname);
        snprintf(entry->save_dir, sizeof(entry->save_dir), "%s", safe_dir);
        entry->slot_num = slot;
        entry->in_use = true;

        path_join(subpath, sizeof(subpath), safe_dir, slot_fname);
        fullpath = prepare_file_path(FGrp_Save, subpath);
        struct stat st;
        if (stat(fullpath, &st) == 0) {
            entry->modified_time = (long)st.st_mtime;
        } else {
            entry->modified_time = 0;
        }

        global_save_count++;
    }

    if (global_save_count > 1) {
        qsort(global_save_entries, global_save_count, sizeof(struct GlobalSaveEntry),
              compare_global_saves_by_date);
    }

    SYNCLOG("Found %d saves for campaign %s", global_save_count, campaign.fname);
}

/**
 * @brief Find the campaign with the most recent continue file and switch to it.
 * Scans all campaign directories for fx1contn.sav, picks the newest, and calls change_campaign().
 * This ensures continue_game_available() checks the right directory at startup.
 * @return true if a continue file was found and campaign was loaded
 */
TbBool find_and_set_continue_campaign(void)
{
    const char *continue_fname = continue_game_filename;
    long best_mtime = 0;
    char best_campaign_fname[CAMPAIGN_FNAME_LEN] = {0};

    const struct CampaignsList *lists[] = { &campaigns_list, &mappacks_list };
    for (int li = 0; li < (int)(sizeof(lists)/sizeof(lists[0])); li++)
    {
        const struct CampaignsList *list = lists[li];
        for (int ci = 0; ci < list->items_num; ci++)
        {
            const struct GameCampaign *cmpgn = &list->items[ci];
            if (cmpgn->fname[0] == '\0')
                continue;

            char safe_dir[DISKPATH_SIZE];
            build_save_dir_for(cmpgn->fname, safe_dir, sizeof(safe_dir));

            char subpath[DISKPATH_SIZE];
            path_join(subpath, sizeof(subpath), safe_dir, continue_fname);
            char *fullpath = prepare_file_path(FGrp_Save, subpath);

            struct stat st;
            if (stat(fullpath, &st) == 0)
            {
                if ((long)st.st_mtime > best_mtime)
                {
                    best_mtime = (long)st.st_mtime;
                    snprintf(best_campaign_fname, sizeof(best_campaign_fname), "%s", cmpgn->fname);
                }
            }
        }
    }

    if (best_campaign_fname[0] != '\0')
    {
        SYNCLOG("Most recent continue file found for campaign %s", best_campaign_fname);
        return change_campaign(best_campaign_fname);
    }

    SYNCLOG("No continue files found in any campaign directory");
    return false;
}

TbBool initialise_load_game_slots(void)
{
    load_game_save_catalogue();
    return (count_valid_saved_games() > 0);
}

/**
 * @brief Build the save subdirectory name for a given campaign filename.
 * Strips the .cfg extension and prefixes with "freeplay/" for mappacks.
 *
 * @param cmpgn_fname  Campaign/mappack config filename (e.g. "keeporig.cfg")
 * @param out          Output buffer
 * @param out_size     Size of output buffer
 */
static void build_save_dir_for(const char *cmpgn_fname, char *out, size_t out_size)
{
    char base[CAMPAIGN_FNAME_LEN];
    snprintf(base, sizeof(base), "%s", cmpgn_fname);
    char *dot = strrchr(base, '.');
    if (dot != NULL && strcasecmp(dot, ".cfg") == 0)
        *dot = '\0';

    if (is_campaign_in_list(cmpgn_fname, &mappacks_list))
        snprintf(out, out_size, "freeplay/%s", base);
    else
        snprintf(out, out_size, "%s", base);
}

static const char* get_safe_campaign_name(void) {
    static char safe_name[DISKPATH_SIZE];
    if (campaign.fname[0] != '\0') {
        build_save_dir_for(campaign.fname, safe_name, sizeof(safe_name));
    } else {
        strncpy(safe_name, "default", sizeof(safe_name) - 1);
        safe_name[sizeof(safe_name) - 1] = '\0';
        WARNLOG("Campaign fname is empty, using default instead");
    }
    return safe_name;
}

/**
 * @brief Build a full path to a file within the current campaign's save directory.
 * Creates the campaign subdirectory (save/{campaign}/) if it does not yet exist.
 *
 * @param fname  The filename (e.g. "fx1g0000.sav")
 * @return char* Full path in a static buffer (same semantics as prepare_file_path)
 */
char *prepare_campaign_save_path(const char *fname)
{
    static char subpath[DISKPATH_SIZE];
    path_join(subpath, sizeof(subpath), get_safe_campaign_name(), fname);
    char *fullpath = prepare_file_path(FGrp_Save, subpath);
    create_directory_for_file(fullpath);
    return fullpath;
}

/**
 * @brief Build a full path with printf-style filename within the campaign's save directory.
 *
 * @param fmt  Format string for the filename (e.g. "fx1g%04d.sav")
 * @param ...  Format arguments
 * @return char* Full path in a static buffer
 */
char *prepare_campaign_save_fmtpath(const char *fmt, ...)
{
    char fname[256];
    va_list val;
    va_start(val, fmt);
    vsnprintf(fname, sizeof(fname), fmt, val);
    va_end(val);
    return prepare_campaign_save_path(fname);
}

short save_continue_game(LevelNumber lvnum)
{
    // Update continue level number
    if (is_singleplayer_like_level(lvnum))
      set_continue_level_number(lvnum);
    SYNCDBG(6,"Continue set to level %d (loaded is %d)",(int)get_continue_level_number(),(int)get_loaded_level_number());

    char* fname = prepare_campaign_save_path(continue_game_filename);


    long fsize = LbFileSaveAt(fname, &game, sizeof(struct Game) + sizeof(struct IntralevelData));
    // Appending IntralevelData
    TbFileHandle fh = LbFileOpen(fname,Lb_FILE_MODE_OLD);
    LbFileSeek(fh, sizeof(struct Game), Lb_FILE_SEEK_BEGINNING);
    LbFileWrite(fh, &intralvl, sizeof(struct IntralevelData));
    LbFileClose(fh);
    return (fsize == sizeof(struct Game) + sizeof(struct IntralevelData));
}

short read_continue_game_part(unsigned char *buf,long pos,long buf_len)
{
    char* fname = prepare_campaign_save_path(continue_game_filename);
    
    if (LbFileLength(fname) != sizeof(struct Game) + sizeof(struct IntralevelData))
    {
        SYNCDBG(7, "No correct .SAV file; there's no continue");
        return false;
  }
  TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
  if (!fh)
  {
    SYNCDBG(7,"Can't open .SAV file; there's no continue");
    return false;
  }
  LbFileSeek(fh, pos, Lb_FILE_SEEK_BEGINNING);
  short result = (LbFileRead(fh, buf, buf_len) == buf_len);
  LbFileClose(fh);
  return result;
}

/**
 * Indicates whether continue game option is available.
 * @return
 */
TbBool continue_game_available(void)
{
    LevelNumber lvnum;
    SYNCDBG(6,"Starting");
    char cmpgn_fname[CAMPAIGN_FNAME_LEN];
    long offset = offsetof(struct Game, campaign_fname);
    if (!read_continue_game_part((unsigned char*)cmpgn_fname, offset, CAMPAIGN_FNAME_LEN)) {
        WARNLOG("Can't read continue game file head");
        return false;
    }
    cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    offset = offsetof(struct Game, continue_level_number);
    if (!read_continue_game_part((unsigned char*)&lvnum, offset, sizeof(lvnum))) {
        WARNLOG("Can't read continue game file head");
        return false;
    }
    if (!change_campaign(cmpgn_fname))
    {
        ERRORLOG("Unable to load campaign");
        return false;
    }
    if (is_singleplayer_like_level(lvnum))
    {
        set_continue_level_number(lvnum);
    }
    lvnum = get_continue_level_number();
    if (is_singleplayer_like_level(lvnum))
    {
        SYNCDBG(7,"Continue to level %d is available",(int)lvnum);
        return true;
    } else
    {
        SYNCDBG(7,"Level %d from continue file is not single player",(int)lvnum);
        return false;
    }
}

short load_continue_game(void)
{
    LevelNumber lvnum;
    char cmpgn_fname[CAMPAIGN_FNAME_LEN];
    long offset = offsetof(struct Game, campaign_fname);
    if (!read_continue_game_part((unsigned char*)cmpgn_fname, offset, CAMPAIGN_FNAME_LEN)) {
        WARNLOG("Can't read continue game file head");
        return false;
    }
    cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    if (!change_campaign(cmpgn_fname))
    {
        ERRORLOG("Unable to load campaign");
        return false;
    }
    offset = offsetof(struct Game, continue_level_number);
    if (!read_continue_game_part((unsigned char*)&lvnum, offset, sizeof(lvnum))) {
        WARNLOG("Can't read continue game file head");
        return false;
    }
    if (!is_singleplayer_like_level(lvnum))
    {
      WARNLOG("Level number in continue file is incorrect");
      return false;
    }
    set_continue_level_number(lvnum);
    // Restoring intralevel data
    read_continue_game_part((unsigned char *)&intralvl, sizeof(struct Game),
        sizeof(struct IntralevelData));
    snprintf(game.campaign_fname, sizeof(game.campaign_fname), "%s", campaign.fname);
    update_extra_levels_visibility();
    JUSTMSG("Continued level %d from %s", lvnum, campaign.name);
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
