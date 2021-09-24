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
#include "frontend.h"

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
  unsigned char creatures_tend_imprison;
  unsigned char creatures_tend_flee;
  unsigned short hand_over_subtile_x;
  unsigned short hand_over_subtile_y;
  unsigned long chosen_room_kind;
  unsigned long chosen_room_spridx;
  unsigned long chosen_room_tooltip;
  unsigned long chosen_spell_type;
  unsigned long chosen_spell_spridx;
  unsigned long chosen_spell_tooltip;
  unsigned long manufactr_element;
  unsigned long manufactr_spridx;
  unsigned long manufactr_tooltip;
};
/******************************************************************************/
/** Structure used for storing 'localised parameters' when resyncing net game. */
struct Boing boing;
/******************************************************************************/
long get_resync_sender(void)
{
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
            return i;
  }
  return -1;
}

TbBool send_resync_game(void)
{
  //TODO NET see if it is necessary to dump to file... probably superfluous
  char* fname = prepare_file_path(FGrp_Save, "resync.dat");
  TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_NEW);
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
    boing.field_0 = game.active_panel_mnu_idx;
    boing.comp_player_aggressive = game.comp_player_aggressive;
    boing.comp_player_defensive = game.comp_player_defensive;
    boing.comp_player_construct = game.comp_player_construct;
    boing.comp_player_creatrsonly = game.comp_player_creatrsonly;
    boing.creatures_tend_imprison = game.creatures_tend_imprison;
    boing.creatures_tend_flee = game.creatures_tend_flee;
    boing.hand_over_subtile_x = game.hand_over_subtile_x;
    boing.hand_over_subtile_y = game.hand_over_subtile_y;
    boing.chosen_room_kind = game.chosen_room_kind;
    boing.chosen_room_spridx = game.chosen_room_spridx;
    boing.chosen_room_tooltip = game.chosen_room_tooltip;
    boing.chosen_spell_type = game.chosen_spell_type;
    boing.chosen_spell_spridx = game.chosen_spell_spridx;
    boing.chosen_spell_tooltip = game.chosen_spell_tooltip;
    boing.manufactr_element = game.manufactr_element;
    boing.manufactr_spridx = game.manufactr_spridx;
    boing.manufactr_tooltip = game.manufactr_tooltip;
}

void recall_localised_game_structure(void)
{
    game.active_panel_mnu_idx = boing.field_0;
    game.comp_player_aggressive = boing.comp_player_aggressive;
    game.comp_player_defensive = boing.comp_player_defensive;
    game.comp_player_construct = boing.comp_player_construct;
    game.comp_player_creatrsonly = boing.comp_player_creatrsonly;
    game.creatures_tend_imprison = boing.creatures_tend_imprison;
    game.creatures_tend_flee = boing.creatures_tend_flee;
    game.hand_over_subtile_x = boing.hand_over_subtile_x;
    game.hand_over_subtile_y = boing.hand_over_subtile_y;
    game.chosen_room_kind = boing.chosen_room_kind;
    game.chosen_room_spridx = boing.chosen_room_spridx;
    game.chosen_room_tooltip = boing.chosen_room_tooltip;
    game.chosen_spell_type = boing.chosen_spell_type;
    game.chosen_spell_spridx = boing.chosen_spell_spridx;
    game.chosen_spell_tooltip = boing.chosen_spell_tooltip;
    game.manufactr_element = boing.manufactr_element;
    game.manufactr_spridx = boing.manufactr_spridx;
    game.manufactr_tooltip = boing.manufactr_tooltip;
}

void resync_game(void)
{
    SYNCDBG(2,"Starting");
    struct PlayerInfo* player = get_my_player();
    draw_out_of_sync_box(0, 32*units_per_pixel/16, player->engine_window_x);
    reset_eye_lenses();
    store_localised_game_structure();
    int i = get_resync_sender();
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
 * Exchanges verification packets between all players, making sure level data is identical.
 * @return Returns true if all players return same checksum.
 */
CoroutineLoopState perform_checksum_verification(CoroutineLoop *con)
{
    short result = true;
    unsigned long checksum_mem = 0;
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing)) {
            checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
        }
    }
    clear_packets();
    struct Packet* pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_LevelExactCheck, 0, 0, 0, 0);
    pckt->chksum = checksum_mem + game.action_rand_seed;
    if (LbNetwork_Exchange(pckt))
    {
        ERRORLOG("Network exchange failed on level checksum verification");
        result = false;
    }
    if (get_packet(0)->action != get_packet(1)->action)
    {
        // Wait for message from other side
        return CLS_REPEAT;
    }
    if ( checksums_different() )
    {
        ERRORLOG("Level checksums different for network players");
        result = false;
    }
    if (!result)
    {
        coroutine_clear(con, true);

        create_frontend_error_box(5000, get_string(GUIStr_NetUnsyncedMap));
        return CLS_ABORT;
    }
    NETLOG("Checksums are verified");
    return CLS_CONTINUE;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
