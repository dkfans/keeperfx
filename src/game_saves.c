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

#include "config.h"
#include "front_simple.h"
#include "frontend.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
DLLIMPORT long _DK_save_game_save_catalogue(struct CatalogueEntry *game_catalg);
DLLIMPORT void _DK_load_game_save_catalogue(struct CatalogueEntry *game_catalg);
DLLIMPORT long _DK_load_game(long);
/******************************************************************************/
const long VersionMajor = 1;
const long VersionMinor = 12;

const char *continue_game_filename="continue.sav";
const char *saved_game_filename="game%04d.sav";
const char *save_catalogue_filename="save.cat";
const char *packet_filename="repl%04d.pck";

/******************************************************************************/

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
}

short save_catalogue_slot_disable(unsigned int slot_idx)
{
  char *fname;
  if (slot_idx >= SAVE_SLOTS_COUNT)
    return false;
  game.save_catalogue[slot_idx].used = false;
  save_game_save_catalogue(game.save_catalogue);
  return true;
}

short is_save_game_loadable(long num)
{
  TbFileHandle fh;
  char buf[12];
  char *fname;
  // Prepare filename and open the file
  fname = prepare_file_fmtpath(FGrp_Save,saved_game_filename,num);
  fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
  if (fh != -1)
  {
    // Let's try to read the file, just to be sure
    if (LbFileRead(fh, &buf, 10) == 10)
    {
      LbFileClose(fh);
      return true;
    }
    LbFileClose(fh);
  }
  return false;
}

short load_game(long num)
{
  static const char *func_name="load_game";
  //return _DK_load_game(num);
  char *fname;
  TbFileHandle handle;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  unsigned char buffer[10];
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  reset_eye_lenses();
  fname = prepare_file_fmtpath(FGrp_Save,saved_game_filename,num);
  if (!wait_for_cd_to_be_available())
    return 0;
  handle = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
  if (handle == -1)
  {
    LbWarnLog("Cannot open saved game file \"%s\".\n",fname);
    save_catalogue_slot_disable(num);
    return 0;
  }
  if (LbFileRead(handle, &buffer, 10) != 10)
  {
    LbFileClose(handle);
    save_catalogue_slot_disable(num);
    return 0;
  }
  LbFileClose(handle);
  if (!save_version_compatible(LbFileLength(fname),(struct Game *)buffer))
  {
    LbWarnLog("Saved game file \"%s\" has incompatible version; restarting level.\n",fname);
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    player->field_7 = 0;
    my_player_number = default_loc_player;
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    game.flagfield_14EA4A = 2;
    set_flag_byte(&game.numfield_A,0x01,false);
    player->field_2C = 1;
    game.level_file_number = buffer[0];
    game.level_number = buffer[0];
    startup_network_game();
    return 1;
  }
  if (LbFileLoadAt(fname, &game) != sizeof(struct Game))
  {
    LbWarnLog("Couldn't correctly load saved game \"%s\".\n",fname);
    return 0;
  }
  init_lookups();
  reinit_level_after_load();
  output_message(102, 0, 1);
  pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
  calculate_moon_phase(false,false);
  game.bonus_levels[MOON_BONUS_INDEX] = is_full_moon;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  set_flag_byte(&player->field_3,0x08,false);
  set_flag_byte(&player->field_3,0x04,false);
  player->field_4C1 = 0;
  player->field_4C5 = 0;
  player->field_7 = 0;
  PaletteSetPlayerPalette(player, _DK_palette);
  reinitialise_eye_lens(game.numfield_1B);
  if (player->field_29 != 0)
  {
    frontstats_initialise();
    dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
    dungeon->lvstats.player_score = 0;
    dungeon->lvstats.allow_save_score = 1;
  }
  game.field_1516FF = 0;
  return 1;
}

int count_valid_saved_games(void)
{
  struct CatalogueEntry *centry;
  int i;
  number_of_saved_games = 0;
  for (i=0; i < SAVE_SLOTS_COUNT; i++)
  {
    centry = &save_game_catalogue[i];
    if (centry->used)
      number_of_saved_games++;
  }
  return number_of_saved_games;
}

short initialise_load_game_slots(void)
{
  load_game_save_catalogue(save_game_catalogue);
  return (count_valid_saved_games() > 0);
}

short save_game_save_catalogue(struct CatalogueEntry *game_catalg)
{
  char *fname;
  //return _DK_save_game_save_catalogue(game_catalg);
  fname = prepare_file_path(FGrp_Save,save_catalogue_filename);
  return (LbFileSaveAt(fname, game_catalg, SAVE_SLOTS_COUNT*sizeof(struct CatalogueEntry)) != -1);
}

void load_game_save_catalogue(struct CatalogueEntry *game_catalg)
{
  char *fname;
  //_DK_load_game_save_catalogue(game_catalg); return;
  fname = prepare_file_path(FGrp_Save,save_catalogue_filename);
  if (LbFileLoadAt(fname, game_catalg) == -1)
    memset(game_catalg, 0, SAVE_SLOTS_COUNT*sizeof(struct CatalogueEntry));
}

void update_continue_game(void)
{
  struct PlayerInfo *player;
  int i,lv_num;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  // Only save continue if level was won, and not in packet mode
  if (((game.numfield_A & 0x01) != 0) || (game.packet_load_enable))
    return;
  i = player->field_29;
  clear_game_for_save();
  player->field_29 = i;
  if (player->field_29 == 1)
  {
    lv_num = next_singleplayer_level(game.level_number);
    if (lv_num <= 0)
      lv_num = game.level_number;
  } else
  {
    lv_num = game.level_number;
  }
  frontend_save_continue_game(lv_num, true);
}

void frontend_save_continue_game(long lv_num, short is_new_lvl)
{
  static const char *func_name="frontend_save_continue_game";
  char *fname;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Continue set to level %d (current is %d)\n",func_name,lv_num,game.level_number);
#endif
  //_DK_frontend_save_continue_game(lv_num,a2);
  if (is_new_lvl)
  {
    game.continue_level = lv_num;
  } else
  {
    error(func_name, 1620, "Why are we here when it's not a new level");
    game.continue_level = 0;
  }
  fname=prepare_file_path(FGrp_Save,continue_game_filename);
  LbFileSaveAt(fname, &game, sizeof(struct Game));
}

short read_continue_game_part(unsigned char *buf,long pos,long buf_len)
{
  static const char *func_name="read_continue_game_head";
  short result;
  char *fname;
  fname = prepare_file_path(FGrp_Save,continue_game_filename);
  if (LbFileLength(fname) != sizeof(struct Game))
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: No correct .SAV file; there's no continue\n",func_name);
  #endif
    return false;
  }
  TbFileHandle fh=LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
  if (fh == -1)
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Can't open .SAV file; there's no continue\n",func_name);
  #endif
    return false;
  }
  LbFileSeek(fh, pos, Lb_FILE_SEEK_BEGINNING);
  result = (LbFileRead(fh, buf, buf_len) == buf_len);
  LbFileClose(fh);
  return result;
}

short continue_game_available(void)
{
  static const char *func_name="continue_game_available";
  unsigned char buf[12];
  static short continue_needs_checking_file = 1;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  if ( continue_needs_checking_file )
  {
    if (read_continue_game_part(buf,0,10))
    {
      if (((struct Game *)buf)->continue_level > 0)
        game.level_number = ((struct Game *)buf)->continue_level;
    }
    continue_needs_checking_file = 0;
  }
  if (is_singleplayer_level(game.level_number))
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Returning that continue is available\n",func_name);
  #endif
    return true;
  } else
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: The level in .SAV isn't correct continue level\n",func_name);
  #endif
    return false;
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
