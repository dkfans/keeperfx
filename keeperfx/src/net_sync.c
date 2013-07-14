/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_sync.c
 *     Network game synchronization for Dungeon Keeper.
 * @par Purpose:
 *     Functions to keep network games synchronized.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "net_sync.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_network.h"

#include "config.h"
#include "front_network.h"
#include "player_data.h"
#include "game_merge.h"
#include "net_game.h"
#include "lens_api.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Boing {
  unsigned char field_0;
  unsigned char comp_player_aggressive;
  unsigned char comp_player_defensive;
  unsigned char comp_player_construct;
  unsigned char comp_player_creatrsonly;
  unsigned char field_5;
  unsigned char field_6;
  unsigned short hand_over_subtile_x;
  unsigned short hand_over_subtile_y;
  unsigned long field_B;
  unsigned long field_F;
  unsigned long field_13;
  unsigned long chosen_spell_type;
  unsigned long chosen_spell_look;
  unsigned long chosen_spell_tooltip;
  unsigned long manufactr_element;
  unsigned long field_27;
  unsigned long manufactr_tooltip;
};
/******************************************************************************/
DLLIMPORT void _DK_resync_game(void);
/******************************************************************************/
/** Structure used for storing 'localised parameters' when resyncing net game. */
struct Boing boing;
/******************************************************************************/
long get_resync_sender(void)
{
  struct PlayerInfo *player;
  int i;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player) && ((player->field_0 & 0x40) == 0))
      return i;
  }
  return -1;
}

TbBool send_resync_game(void)
{
  TbFileHandle fh;
  char *fname;

  //TODO NET see if it is necessary to dump to file... probably superfluous
  fname = prepare_file_path(FGrp_Save,"resync.dat");
  fh = LbFileOpen(fname, Lb_FILE_MODE_NEW);
  if (fh == -1)
  {
    ERRORLOG("Can't open resync file.");
    return false;
  }

  LbFileWrite(fh, &game, sizeof(game));
  LbFileClose(fh);

  NETLOG("Initiating re-synchronization of network game");
  return LbNetwork_Resync(&game, sizeof(game));
}

TbBool receive_resync_game(void)
{
    NETLOG("Initiating re-synchronization of network game");
    return LbNetwork_Resync(&game, sizeof(game));
}

void store_localised_game_structure(void)
{
    boing.field_0 = game.field_1517F6;
    boing.comp_player_aggressive = game.comp_player_aggressive;
    boing.comp_player_defensive = game.comp_player_defensive;
    boing.comp_player_construct = game.comp_player_construct;
    boing.comp_player_creatrsonly = game.comp_player_creatrsonly;
    boing.field_5 = game.creatures_tend_1;
    boing.field_6 = game.creatures_tend_2;
    boing.hand_over_subtile_x = game.hand_over_subtile_x;
    boing.hand_over_subtile_y = game.hand_over_subtile_y;
    boing.field_B = game.field_151801;
    boing.field_F = game.field_151805;
    boing.field_13 = game.field_151809;
    boing.chosen_spell_type = game.chosen_spell_type;
    boing.chosen_spell_look = game.chosen_spell_look;
    boing.chosen_spell_tooltip = game.chosen_spell_tooltip;
    boing.manufactr_element = game.manufactr_element;
    boing.field_27 = game.numfield_15181D;
    boing.manufactr_tooltip = game.manufactr_tooltip;
}

void recall_localised_game_structure(void)
{
    game.field_1517F6 = boing.field_0;
    game.comp_player_aggressive = boing.comp_player_aggressive;
    game.comp_player_defensive = boing.comp_player_defensive;
    game.comp_player_construct = boing.comp_player_construct;
    game.comp_player_creatrsonly = boing.comp_player_creatrsonly;
    game.creatures_tend_1 = boing.field_5;
    game.creatures_tend_2 = boing.field_6;
    game.hand_over_subtile_x = boing.hand_over_subtile_x;
    game.hand_over_subtile_y = boing.hand_over_subtile_y;
    game.field_151801 = boing.field_B;
    game.field_151805 = boing.field_F;
    game.field_151809 = boing.field_13;
    game.chosen_spell_type = boing.chosen_spell_type;
    game.chosen_spell_look = boing.chosen_spell_look;
    game.chosen_spell_tooltip = boing.chosen_spell_tooltip;
    game.manufactr_element = boing.manufactr_element;
    game.numfield_15181D = boing.field_27;
    game.manufactr_tooltip = boing.manufactr_tooltip;
}

void resync_game(void)
{
    struct PlayerInfo *player;
    int i;
    SYNCDBG(2,"Starting");
    //return _DK_resync_game();
    player = get_my_player();
    draw_out_of_sync_box(0, 32, player->engine_window_x);
    reset_eye_lenses();
    store_localised_game_structure();
    i = get_resync_sender();
    if (is_my_player_number(i))
    {
        send_resync_game();
    } else
    {
        receive_resync_game();
    }
    recall_localised_game_structure();
    reinit_level_after_load();
    set_flag_byte(&game.system_flags,GSF_NetGameNoSync,false);
    set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,false);
}

/**
 * Exchanges verification packets between all players.
 * @return Returns true if all players return same checksum.
 */
short perform_checksum_verification(void)
{
    struct Packet *pckt;
    struct Thing *thing;
    unsigned long checksum_mem;
    short result;
    int i;
    result = true;
    checksum_mem = 0;
    for (i=1; i<THINGS_COUNT; i++)
    {
        thing = thing_get(i);
        if (thing_exists(thing)) {
            checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
        }
    }
    clear_packets();
    pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_Unknown012, 0, 0, 0, 0);
    pckt->chksum = checksum_mem + game.action_rand_seed;
    if (LbNetwork_Exchange(pckt))
    {
        ERRORLOG("Network exchange failed on level checksum verification");
        result = false;
    }
    if ( checksums_different() )
    {
        ERRORLOG("Level checksums different for network players");
        result = false;
    }
    return result;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
