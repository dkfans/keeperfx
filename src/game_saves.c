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
#include "game_saves.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_bufrw.h"

#include "config.h"
#include "config_campaigns.h"
#include "config_creature.h"
#include "config_compp.h"
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
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
TbBool load_catalogue_entry(TbFileHandle fh,struct FileChunkHeader *hdr,struct CatalogueEntry *centry);
/******************************************************************************/
long const VersionMajor = 1;
long const VersionMinor = 12;

const char *continue_game_filename="fx1contn.sav";
const char *saved_game_filename="fx1g%04d.sav";
const char *packet_filename="fx1rp%04d.pck";

struct CatalogueEntry save_game_catalogue[TOTAL_SAVE_SLOTS_COUNT];
/******************************************************************************/
TbBool is_primitive_save_version(long filesize)
{
    if (filesize < (char *)&game.loaded_level_number - (char *)&game)
        return false;
    if (filesize <= 1382437) // sizeof(struct Game) - but it's better to use constant here
        return true;
    return false;
}
/*
short save_version_compatible(long filesize,struct Game *header)
{
  // Native version
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor == VersionMinor))
    return true;
  // Compatibility list
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor <= 8) &&
    (VersionMinor == 10))
    return true;
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor <= 10) &&
    (VersionMinor == 12))
    return true;
  return false;
}*/

TbBool save_game_chunks(TbFileHandle fhandle,struct CatalogueEntry *centry)
{
    struct FileChunkHeader hdr;
    long chunks_done = 0;
    // Currently there is some game data oustide of structs - make sure it is updated
    light_export_system_state(&gameadd.lightst);
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
    { // GameAdd data chunk
        hdr.id = SGC_GameAdd;
        hdr.ver = 0;
        hdr.len = sizeof(struct GameAdd);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, &gameadd, sizeof(struct GameAdd)) == sizeof(struct GameAdd))
            chunks_done |= SGF_GameAdd;
    }
    { // IntralevelData data chunk
        hdr.id = SGC_IntralevelData;
        hdr.ver = 0;
        hdr.len = sizeof(struct IntralevelData);
        if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        if (LbFileWrite(fhandle, &intralvl, sizeof(struct IntralevelData)) == sizeof(struct IntralevelData))
            chunks_done |= SGF_IntralevelData;
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
        { // GameAdd data chunk
            hdr.id = SGC_GameAdd;
            hdr.ver = 0;
            hdr.len = sizeof(struct GameAdd);
            if (LbFileWrite(fhandle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
            if (LbFileWrite(fhandle, &gameadd, sizeof(struct GameAdd)) == sizeof(struct GameAdd))
                chunks_done |= SGF_GameAdd;
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

int load_game_chunks(TbFileHandle fhandle,struct CatalogueEntry *centry)
{
    long chunks_done = 0;
    while (!LbFileEof(fhandle))
    {
        struct FileChunkHeader hdr;
        if (LbFileRead(fhandle, &hdr, sizeof(struct FileChunkHeader)) != sizeof(struct FileChunkHeader))
            break;
        switch(hdr.id)
        {
        case SGC_InfoBlock:
            if (load_catalogue_entry(fhandle,&hdr,centry))
            {
                chunks_done |= SGF_InfoBlock;
                if (!change_campaign(centry->campaign_fname)) {
                    ERRORLOG("Unable to load campaign");
                    return GLoad_Failed;
                }
                // Load configs which may have per-campaign part, and even be modified within a level
                load_computer_player_config(CnfLd_Standard);
                load_stats_files();
                check_and_auto_fix_stats();
                init_creature_scores();
                strncpy(high_score_entry,centry->player_name,PLAYER_NAME_LENGTH);
            }
            break;
        case SGC_GameAdd:
            if (hdr.len != sizeof(struct GameAdd))
            {
                if (LbFileSeek(fhandle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                    LbFileSeek(fhandle, 0, Lb_FILE_SEEK_END);
                WARNLOG("Incompatible GameAdd chunk");
                break;
            }
            if (LbFileRead(fhandle, &gameadd, sizeof(struct GameAdd)) == sizeof(struct GameAdd)) {
            //accept invalid saves -- if (LbFileRead(fhandle, &gameadd, hdr.len) == hdr.len) {
                chunks_done |= SGF_GameAdd;
            } else {
                WARNLOG("Could not read GameAdd chunk");
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
        default:
            WARNLOG("Unrecognized chunk, ID = %08lx",hdr.id);
            break;
        }
    }
    if ((chunks_done & SGF_SavedGame) == SGF_SavedGame)
    {
        // Update interface items
        update_trap_tab_to_config();
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
    if (!save_game_save_catalogue())
        return false;
/*  game.version_major = VersionMajor;
    game.version_minor = VersionMinor;
    game.load_restart_level = get_loaded_level_number();*/
    char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (handle == -1)
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
    return true;
}

TbBool is_save_game_loadable(long slot_num)
{
    // Prepare filename and open the file
    char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
    TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (fh != -1)
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
    TbFileHandle fh;
//  unsigned char buf[14];
//  char cmpgn_fname[CAMPAIGN_FNAME_LEN];
    SYNCDBG(6,"Starting");
    reset_eye_lenses();
    {
        // Use fname only here - it is overwritten by next use of prepare_file_fmtpath()
        char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
        if (!wait_for_cd_to_be_available())
          return false;
        fh = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
        if (fh == -1)
        {
          WARNMSG("Cannot open saved game file \"%s\".",fname);
          save_catalogue_slot_disable(slot_num);
          return false;
        }
    }
    long file_len = LbFileLengthHandle(fh);
    if (is_primitive_save_version(file_len))
    {
        //if (LbFileRead(handle, buf, sizeof(buf)) != sizeof(buf))
        {
          LbFileClose(fh);
          save_catalogue_slot_disable(slot_num);
          return false;
        }
        /*LbFileSeek(fh, (char *)&game.campaign_fname[0] - (char *)&game, Lb_FILE_SEEK_BEGINNING);
        LbFileRead(fh, cmpgn_fname, CAMPAIGN_FNAME_LEN);
        cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
        if (!change_campaign(cmpgn_fname))
        {
          ERRORLOG("Unable to load campaign associated with saved game");
        }
        LbFileClose(fh);
        WARNMSG("Saved game file \"%s\" has incompatible version; restarting level.",fname);
        player = get_my_player();
        player->lens_palette = 0;
        my_player_number = default_loc_player;
        player = get_my_player();
        game.flagfield_14EA4A = 2;
        set_flag_byte(&game.system_flags,GSF_NetworkActive,false);
        player->is_active = 1;
        set_selected_level_number(((struct Game *)buf)->load_restart_level);
        set_continue_level_number(((struct Game *)buf)->continue_level_number);
        startup_network_game();
        return true;*/
    }
    struct CatalogueEntry* centry = &save_game_catalogue[slot_num];
    LbFileSeek(fh, 0, Lb_FILE_SEEK_BEGINNING);
    // Here is the actual loading
    if (load_game_chunks(fh,centry) != GLoad_SavedGame)
    {
        LbFileClose(fh);
        WARNMSG("Couldn't correctly load saved game in slot %d.",(int)slot_num);
        init_lookups();
        return false;
    }
    my_player_number = game.local_plyr_idx;
    LbFileClose(fh);
    LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
    reinit_level_after_load();
    output_message(SMsg_GameLoaded, 0, true);
    pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
    calculate_moon_phase(false,false);
    update_extra_levels_visibility();
    struct PlayerInfo* player = get_my_player();
    set_flag_byte(&player->additional_flags,PlaAF_LightningPaletteIsActive,false);
    set_flag_byte(&player->additional_flags,PlaAF_FreezePaletteIsActive,false);
    player->palette_fade_step_pain = 0;
    player->palette_fade_step_possession = 0;
    player->lens_palette = 0;
    PaletteSetPlayerPalette(player, engine_palette);
    reinitialise_eye_lens(game.numfield_1B);
    // Update the lights system state
    light_import_system_state(&gameadd.lightst);
    // Victory state
    if (player->victory_state != VicS_Undecided)
    {
      frontstats_initialise();
      struct Dungeon* dungeon = get_players_dungeon(player);
      dungeon->lvstats.player_score = 0;
      dungeon->lvstats.allow_save_score = 1;
    }
    game.loaded_swipe_idx = -1;
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
    centry->version = (VersionMajor << 16) + VersionMinor;
    centry->level_num = get_loaded_level_number();
    strncpy(centry->textname,textname,SAVE_TEXTNAME_LEN);
    strncpy(centry->campaign_name,campaign.name,LINEMSG_SIZE);
    strncpy(centry->campaign_fname,campaign.fname,DISKPATH_SIZE);
    strncpy(centry->player_name,high_score_entry,PLAYER_NAME_LENGTH);
    centry->textname[SAVE_TEXTNAME_LEN-1] = '\0';
    centry->campaign_name[LINEMSG_SIZE-1] = '\0';
    centry->campaign_fname[DISKPATH_SIZE-1] = '\0';
    centry->player_name[PLAYER_NAME_LENGTH-1] = '\0';
    set_flag_word(&centry->flags, CEF_InUse, true);
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

TbBool game_save_catalogue(struct CatalogueEntry *game_catalg,int nentries)
{
  //Saved games descriptions are no longer stored in catalogue file - so writing is disabled.
    return true;
}

TbBool game_catalogue_slot_disable(struct CatalogueEntry *game_catalg,unsigned int slot_idx)
{
  if (slot_idx >= TOTAL_SAVE_SLOTS_COUNT)
    return false;
  set_flag_word(&game_catalg[slot_idx].flags, CEF_InUse, false);
  game_save_catalogue(game_catalg,TOTAL_SAVE_SLOTS_COUNT);
  return true;
}

TbBool save_catalogue_slot_disable(unsigned int slot_idx)
{
  return game_catalogue_slot_disable(save_game_catalogue,slot_idx);
}

TbBool save_game_save_catalogue(void)
{
  return game_save_catalogue(save_game_catalogue,TOTAL_SAVE_SLOTS_COUNT);
}

TbBool load_catalogue_entry(TbFileHandle fh,struct FileChunkHeader *hdr,struct CatalogueEntry *centry)
{
    set_flag_word(&centry->flags, CEF_InUse, false);
    if ((hdr->id == SGC_InfoBlock) && (hdr->len == sizeof(struct CatalogueEntry)))
    {
        if (LbFileRead(fh, centry, sizeof(struct CatalogueEntry))
          == sizeof(struct CatalogueEntry))
        {
            set_flag_word(&centry->flags, CEF_InUse, true);
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
    //return load_game_catalogue(save_game_catalogue);
    long saves_found = 0;
    for (long slot_num = 0; slot_num < TOTAL_SAVE_SLOTS_COUNT; slot_num++)
    {
        struct CatalogueEntry* centry = &save_game_catalogue[slot_num];
        LbMemorySet(centry, 0, sizeof(struct CatalogueEntry));
        char* fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
        TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
        if (fh == -1)
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
    TbFileHandle fh = LbFileOpen(fname,Lb_FILE_MODE_NEW);
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
  if (fh == -1)
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
    long lvnum;
    SYNCDBG(6,"Starting");
    {
        unsigned char buf[14];
        if (!read_continue_game_part(buf, 0, sizeof(buf)))
        {
            return false;
        }
        long i = (char*)&game.campaign_fname[0] - (char*)&game;
        char cmpgn_fname[CAMPAIGN_FNAME_LEN];
        read_continue_game_part((unsigned char*)cmpgn_fname, i, CAMPAIGN_FNAME_LEN);
        cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
        lvnum = ((struct Game *)buf)->continue_level_number;
        if (!change_campaign(cmpgn_fname))
        {
          ERRORLOG("Unable to load campaign");
          return false;
        }
        if (is_singleplayer_like_level(lvnum))
        {
            set_continue_level_number(lvnum);
        }
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
    unsigned char buf[14];

    if (!read_continue_game_part(buf,0,14))
    {
        WARNLOG("Can't read continue game file head");
        return false;
    }
    long i = (char*)&game.campaign_fname[0] - (char*)&game;
    char cmpgn_fname[CAMPAIGN_FNAME_LEN];
    read_continue_game_part((unsigned char*)cmpgn_fname, i, CAMPAIGN_FNAME_LEN);
    cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    if (!change_campaign(cmpgn_fname))
    {
        ERRORLOG("Unable to load campaign");
        return false;
    }
    long lvnum = ((struct Game*)buf)->continue_level_number;
    if (!is_singleplayer_like_level(lvnum))
    {
      WARNLOG("Level number in continue file is incorrect");
      return false;
    }
    set_continue_level_number(lvnum);
    // Restoring intralevel data
    read_continue_game_part((unsigned char *)&intralvl, sizeof(struct Game),
        sizeof(struct IntralevelData));
    LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
    update_extra_levels_visibility();
    return true;
}

TbBool set_transfered_creature(PlayerNumber plyr_idx, ThingModel model, long explevel)
{
    if (is_my_player_number(plyr_idx))
    {
        intralvl.transferred_creature.model = model;
        intralvl.transferred_creature.explevel = explevel;
        return true;
    }
    return false;
}

void clear_transfered_creature(void)
{
    intralvl.transferred_creature.model = 0;
    intralvl.transferred_creature.explevel = 0;
}

LevelNumber move_campaign_to_next_level(void)
{
    LevelNumber curr_lvnum = get_continue_level_number();
    LevelNumber lvnum = next_singleplayer_level(curr_lvnum);
    SYNCDBG(15,"Campaign move %ld to %ld",(long)curr_lvnum,(long)lvnum);
    {
        struct PlayerInfo* player = get_my_player();
        player->flgfield_6 &= ~PlaF6_PlyrHasQuit;
    }
    if (lvnum != LEVELNUMBER_ERROR)
    {
        curr_lvnum = set_continue_level_number(lvnum);
        SYNCDBG(8,"Continue level moved to %ld.",curr_lvnum);
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
    long curr_lvnum = get_continue_level_number();
    long lvnum = prev_singleplayer_level(curr_lvnum);
    SYNCDBG(15,"Campaign move %ld to %ld",(long)curr_lvnum,(long)lvnum);
    if (lvnum != LEVELNUMBER_ERROR)
    {
        curr_lvnum = set_continue_level_number(lvnum);
        SYNCDBG(8,"Continue level moved to %ld.",(long)curr_lvnum);
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
