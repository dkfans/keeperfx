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

const char *continue_game_filename="fx1contn.sav";
const char *saved_game_filename="fx1g%04d.sav";
const char *packet_filename="fx1rp%04d.pck";

struct CatalogueEntry save_game_catalogue[TOTAL_SAVE_SLOTS_COUNT];

int number_of_saved_games;
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
        init_lookups();
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
    PaletteSetPlayerPalette(player, engine_palette);
    reinitialise_eye_lens(game.applied_lens_type);
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

short save_continue_game(LevelNumber lvnum)
{
    // Update continue level number
    if (is_singleplayer_like_level(lvnum))
      set_continue_level_number(lvnum);
    SYNCDBG(6,"Continue set to level %d (loaded is %d)",(int)get_continue_level_number(),(int)get_loaded_level_number());
    char* fname = prepare_file_path(FGrp_Save, continue_game_filename);
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
    char* fname = prepare_file_path(FGrp_Save, continue_game_filename);
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
