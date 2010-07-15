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
#include "front_simple.h"
#include "frontend.h"
#include "front_landview.h"
#include "lens_api.h"
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
DLLIMPORT long _DK_save_game_save_catalogue(struct CatalogueEntry *game_catalg);
DLLIMPORT void _DK_load_game_save_catalogue(struct CatalogueEntry *game_catalg);
DLLIMPORT long _DK_load_game(long);
/******************************************************************************/
TbBool load_catalogue_entry(TbFileHandle fh,struct FileChunkHeader *hdr,long slot_num);
/******************************************************************************/
long const VersionMajor = 1;
long const VersionMinor = 12;

const char *continue_game_filename="fx1contn.sav";
const char *saved_game_filename="fx1g%04d.sav";
const char *save_catalogue_filename="fx1save.cat";
const char *packet_filename="fx1rp%04d.pck";

struct CatalogueEntry save_game_catalogue[SAVE_SLOTS_COUNT];
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

/**
 * Saves the game state file (savegame).
 * @note fill_game_catalogue_entry() should be called before to fill level information.
 *
 * @param slot_num
 * @return
 */
TbBool save_game(long slot_num)
{
  char *fname;
  TbFileHandle handle;
  struct FileChunkHeader hdr;
  long chunks_done;
  if (!save_game_save_catalogue())
    return false;
/*  game.version_major = VersionMajor;
  game.version_minor = VersionMinor;
  game.load_restart_level = get_loaded_level_number();*/
  fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
  handle = LbFileOpen(fname,Lb_FILE_MODE_NEW);
  if (handle == -1)
  {
      WARNMSG("Cannot open file to save, \"%s\".",fname);
      return false;
  }
  chunks_done = 0;
  { // Info chunk
      hdr.id = SGC_InfoBlock;
      hdr.ver = 0;
      hdr.len = sizeof(struct CatalogueEntry);
      if (LbFileWrite(handle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
      if (LbFileWrite(handle, &save_game_catalogue[slot_num], sizeof(struct CatalogueEntry)) == sizeof(struct CatalogueEntry))
          chunks_done |= 0x0001;
  }
  { // Game data chunk
      hdr.id = SGC_GameOrig;
      hdr.ver = 0;
      hdr.len = sizeof(struct Game);
      if (LbFileWrite(handle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
      if (LbFileWrite(handle, &game, sizeof(struct Game)) == sizeof(struct Game))
          chunks_done |= 0x0002;
  }
  { // GameAdd data chunk
      hdr.id = SGC_GameAdd;
      hdr.ver = 0;
      hdr.len = sizeof(struct GameAdd);
      if (LbFileWrite(handle, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
      if (LbFileWrite(handle, &gameadd, sizeof(struct GameAdd)) == sizeof(struct GameAdd))
          chunks_done |= 0x0004;
  }
  LbFileClose(handle);
  if (chunks_done != 0x0007)
  {
      WARNMSG("Cannot write to save file, \"%s\".",fname);
      return false;
  }
  return true;
}

TbBool is_save_game_loadable(long slot_num)
{
    char *fname;
    TbFileHandle fh;
    struct FileChunkHeader hdr;
    // Prepare filename and open the file
    fname = prepare_file_fmtpath(FGrp_Save,saved_game_filename,slot_num);
    fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (fh != -1)
    {
      // Let's try to read the file, just to be sure
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
  //return _DK_load_game(slot_num);
  char *fname;
  TbFileHandle handle;
  long file_len;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  long chunks_done;
  struct FileChunkHeader hdr;
  struct CatalogueEntry *centry;
//  unsigned char buf[14];
//  char cmpgn_fname[CAMPAIGN_FNAME_LEN];
  SYNCDBG(6,"Starting");
  reset_eye_lenses();
  fname = prepare_file_fmtpath(FGrp_Save,saved_game_filename,slot_num);
  if (!wait_for_cd_to_be_available())
    return false;
  handle = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
  if (handle == -1)
  {
    WARNMSG("Cannot open saved game file \"%s\".",fname);
    save_catalogue_slot_disable(slot_num);
    return false;
  }
  file_len = LbFileLengthHandle(handle);
/*  if (is_primitive_save_version(file_len))
  {
      if (LbFileRead(handle, buf, sizeof(buf)) != sizeof(buf))
      {
        LbFileClose(handle);
        save_catalogue_slot_disable(slot_num);
        return false;
      }
      LbFileSeek(handle, (char *)&game.campaign_fname[0] - (char *)&game, Lb_FILE_SEEK_BEGINNING);
      LbFileRead(handle, cmpgn_fname, CAMPAIGN_FNAME_LEN);
      cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
      if (!change_campaign(cmpgn_fname))
      {
        ERRORLOG("Unable to load campaign associated with saved game");
      }
      LbFileClose(handle);
      WARNMSG("Saved game file \"%s\" has incompatible version; restarting level.",fname);
      player = get_my_player();
      player->field_7 = 0;
      my_player_number = default_loc_player;
      player = get_my_player();
      game.flagfield_14EA4A = 2;
      set_flag_byte(&game.system_flags,GSF_NetworkActive,false);
      player->field_2C = 1;
      set_selected_level_number(((struct Game *)buf)->load_restart_level);
      set_continue_level_number(((struct Game *)buf)->continue_level_number);
      startup_network_game();
      return true;
  }*/
  LbFileSeek(handle, 0, Lb_FILE_SEEK_BEGINNING);
  chunks_done = 0;
  while (!LbFileEof(handle))
  {
      if (LbFileRead(handle, &hdr, sizeof(struct FileChunkHeader)) != sizeof(struct FileChunkHeader))
          break;
      switch(hdr.id)
      {
      case SGC_InfoBlock:
          if (load_catalogue_entry(handle,&hdr,slot_num))
          {
              chunks_done |= 0x0001;
              centry = &save_game_catalogue[slot_num];
              if (!change_campaign(centry->campaign_fname))
              {
                ERRORLOG("Unable to load campaign");
                return false;
              }
              strncpy(high_score_entry,centry->player_name,PLAYER_NAME_LENGTH);
          }
          break;
      case SGC_GameAdd:
          if (hdr.len != sizeof(struct GameAdd))
          {
              if (LbFileSeek(handle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                  LbFileSeek(handle, 0, Lb_FILE_SEEK_END);
              break;
          }
          if (LbFileRead(handle, &gameadd, sizeof(struct GameAdd)) == sizeof(struct GameAdd))
              chunks_done |= 0x0004;
          break;
      case SGC_GameOrig:
          if (hdr.len != sizeof(struct Game))
          {
              if (LbFileSeek(handle, hdr.len, Lb_FILE_SEEK_CURRENT) < 0)
                  LbFileSeek(handle, 0, Lb_FILE_SEEK_END);
              break;
          }
          if (LbFileRead(handle, &game, sizeof(struct Game)) == sizeof(struct Game))
              chunks_done |= 0x0002;
          break;
      default:
          WARNLOG("Unrecognized chunk, ID = %08lx",hdr.id);
          break;
      }
  }
  LbFileClose(handle);
  if (chunks_done != 0x0007)
  {
      WARNMSG("Couldn't correctly load saved game \"%s\".",fname);
      return false;
  }
  LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
  init_lookups();
  reinit_level_after_load();
  output_message(SMsg_GameLoaded, 0, 1);
  pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
  calculate_moon_phase(false,false);
  update_extra_levels_visibility();
  player = get_my_player();
  set_flag_byte(&player->field_3,0x08,false);
  set_flag_byte(&player->field_3,0x04,false);
  player->field_4C1 = 0;
  player->field_4C5 = 0;
  player->field_7 = 0;
  PaletteSetPlayerPalette(player, _DK_palette);
  reinitialise_eye_lens(game.numfield_1B);
  if (player->victory_state != VicS_Undecided)
  {
    frontstats_initialise();
    dungeon = get_players_dungeon(player);
    dungeon->lvstats.player_score = 0;
    dungeon->lvstats.allow_save_score = 1;
  }
  game.field_1516FF = 0;
  return true;
}

int count_valid_saved_games(void)
{
  struct CatalogueEntry *centry;
  int i;
  number_of_saved_games = 0;
  for (i=0; i < SAVE_SLOTS_COUNT; i++)
  {
    centry = &save_game_catalogue[i];
    if ((centry->flags & CEF_InUse) != 0)
      number_of_saved_games++;
  }
  return number_of_saved_games;
}

TbBool fill_game_catalogue_entry(long slot_num,const char *textname)
{
    struct CatalogueEntry *centry;
    centry = &save_game_catalogue[slot_num];
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

TbBool game_save_catalogue(struct CatalogueEntry *game_catalg,int nentries)
{
/*  Saved games descriptions are no longer stored in catalogue file - so writing is disabled.
    char *fname;
    fname = prepare_file_path(FGrp_Save,save_catalogue_filename);
    return (LbFileSaveAt(fname, game_catalg, nentries*sizeof(struct CatalogueEntry)) != -1);
*/
    return true;
}

TbBool game_catalogue_slot_disable(struct CatalogueEntry *game_catalg,unsigned int slot_idx)
{
  if (slot_idx >= SAVE_SLOTS_COUNT)
    return false;
  set_flag_word(&game_catalg[slot_idx].flags, CEF_InUse, false);
  game_save_catalogue(game_catalg,SAVE_SLOTS_COUNT);
  return true;
}

TbBool save_catalogue_slot_disable(unsigned int slot_idx)
{
  return game_catalogue_slot_disable(save_game_catalogue,slot_idx);
}

TbBool save_game_save_catalogue(void)
{
  return game_save_catalogue(save_game_catalogue,SAVE_SLOTS_COUNT);
}
/*
short load_game_catalogue(struct CatalogueEntry *game_catalg)
{
  char *fname;
  fname = prepare_file_path(FGrp_Save,save_catalogue_filename);
  if (LbFileLoadAt(fname, game_catalg) == -1)
  {
    memset(game_catalg, 0, SAVE_SLOTS_COUNT*sizeof(struct CatalogueEntry));
    return false;
  }
  return true;
}
*/
TbBool load_catalogue_entry(TbFileHandle fh,struct FileChunkHeader *hdr,long slot_num)
{
    struct CatalogueEntry *centry;
    centry = &save_game_catalogue[slot_num];
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
    long slot_num;
    char *fname;
    TbFileHandle fh;
    struct FileChunkHeader hdr;
    struct CatalogueEntry *centry;
    long saves_found;
    //return load_game_catalogue(save_game_catalogue);
    saves_found = 0;
    for (slot_num=0; slot_num < SAVE_SLOTS_COUNT; slot_num++)
    {
        centry = &save_game_catalogue[slot_num];
        memset(centry, 0, sizeof(struct CatalogueEntry));
        fname = prepare_file_fmtpath(FGrp_Save, saved_game_filename, slot_num);
        fh = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
        if (fh == -1)
            continue;
        if (LbFileRead(fh, &hdr, sizeof(struct FileChunkHeader)) == sizeof(struct FileChunkHeader))
        {
            if (load_catalogue_entry(fh,&hdr,slot_num))
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
  LevelNumber bkp_lvnum;
  char *fname;
  long fsize;
  // Update continue level number
  bkp_lvnum = get_continue_level_number();
  if (is_singleplayer_like_level(lvnum))
    set_continue_level_number(lvnum);
  SYNCDBG(6,"Continue set to level %d (loaded is %d)",(int)get_continue_level_number(),(int)get_loaded_level_number());
  fname = prepare_file_path(FGrp_Save,continue_game_filename);
  fsize = LbFileSaveAt(fname, &game, sizeof(struct Game));
  // Reset original continue level number
  if (is_singleplayer_like_level(lvnum))
    set_continue_level_number(bkp_lvnum);
  return (fsize == sizeof(struct Game));
}

short read_continue_game_part(unsigned char *buf,long pos,long buf_len)
{
  TbFileHandle fh;
  short result;
  char *fname;
  fname = prepare_file_path(FGrp_Save,continue_game_filename);
  if (LbFileLength(fname) != sizeof(struct Game))
  {
    SYNCDBG(7,"No correct .SAV file; there's no continue");
    return false;
  }
  fh = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
  if (fh == -1)
  {
    SYNCDBG(7,"Can't open .SAV file; there's no continue");
    return false;
  }
  LbFileSeek(fh, pos, Lb_FILE_SEEK_BEGINNING);
  result = (LbFileRead(fh, buf, buf_len) == buf_len);
  LbFileClose(fh);
  return result;
}

short continue_game_available(void)
{
  unsigned char buf[14];
  char cmpgn_fname[CAMPAIGN_FNAME_LEN];
  long lvnum;
  long i;
  SYNCDBG(6,"Starting");
  {
    if (!read_continue_game_part(buf,0,sizeof(buf)))
    {
      return false;
    }
    i = (char *)&game.campaign_fname[0] - (char *)&game;
    read_continue_game_part((unsigned char *)cmpgn_fname,i,CAMPAIGN_FNAME_LEN);
    cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
    lvnum = ((struct Game *)buf)->continue_level_number;
    if (!change_campaign(cmpgn_fname))
    {
      ERRORLOG("Unable to load campaign");
      return false;
    }
    if (is_singleplayer_like_level(lvnum))
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
  unsigned char buf[14];
  unsigned char bonus[12];
  char cmpgn_fname[CAMPAIGN_FNAME_LEN];
  long lvnum;
  long i;

  if (!read_continue_game_part(buf,0,14))
  {
    WARNLOG("Can't read continue game file head");
    return false;
  }
  i = (char *)&game.campaign_fname[0] - (char *)&game;
  read_continue_game_part((unsigned char *)cmpgn_fname,i,CAMPAIGN_FNAME_LEN);
  cmpgn_fname[CAMPAIGN_FNAME_LEN-1] = '\0';
  if (!change_campaign(cmpgn_fname))
  {
    ERRORLOG("Unable to load campaign");
    return false;
  }
  lvnum = ((struct Game *)buf)->continue_level_number;
  if (!is_singleplayer_like_level(lvnum))
  {
    WARNLOG("Level number in continue file is incorrect");
    return false;
  }
  set_continue_level_number(lvnum);
  LbMemorySet(bonus,0,12);
  i = (char *)&game.bonuses_found[0] - (char *)&game;
  read_continue_game_part(bonus,i,10);
/*  game.load_restart_level = ((struct Game *)buf)->load_restart_level;
  game.version_major = ((struct Game *)buf)->version_major;
  game.version_minor = ((struct Game *)buf)->version_minor;*/
  for (i=0; i < BONUS_LEVEL_STORAGE_COUNT; i++)
    game.bonuses_found[i] = bonus[i];
  LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
  update_extra_levels_visibility();
  i = (char *)&game.transfered_creature - (char *)&game.bonuses_found[0];
  game.transfered_creature.model = bonus[i];
  game.transfered_creature.explevel = bonus[i+1];
  return true;
}

TbBool set_transfered_creature(long plyr_idx, long model, long explevel)
{
  if (is_my_player_number(plyr_idx))
  {
    game.transfered_creature.model = model;
    game.transfered_creature.explevel = explevel;
    return true;
  }
  return false;
}

void clear_transfered_creature(void)
{
  game.transfered_creature.model = 0;
  game.transfered_creature.explevel = 0;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
