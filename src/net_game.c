/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_game.c
 *     Network game support for Dungeon Keeper.
 * @par Purpose:
 *     Functions to exchange packets through network.
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
#include "pre_inc.h"
#include "net_game.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_coroutine.h"
#include "bflib_network.h"
#include "bflib_network_exchange.h"
#include "net_resync.h"

#include "player_data.h"
#include "front_landview.h"
#include "player_utils.h"
#include "packets.h"
#include "frontend.h"
#include "front_network.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "net_input_lag.h"
#include "net_checksums.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbNetworkPlayerInfo net_player_info[NET_PLAYERS_COUNT];
struct TbNetworkSessionNameEntry *net_session[32];
long net_number_of_sessions;
long net_session_index_active;
struct TbNetworkPlayerName net_player[NET_PLAYERS_COUNT];
struct ConfigInfo net_config_info;
char net_service[16][NET_SERVICE_LEN];
char net_player_name[20];
/******************************************************************************/
short setup_network_service(int srvidx)
{
  struct ServiceInitData *init_data = NULL;
  SYNCMSG("Initializing 4-players type %d network",srvidx);
  memset(&net_player_info[0], 0, sizeof(struct TbNetworkPlayerInfo));
  if ( LbNetwork_Init(srvidx, NET_PLAYERS_COUNT, &net_player_info[0], init_data) )
  {
    if (srvidx > NS_ENET_UDP)
      process_network_error(-800);
    return 0;
  }
  net_service_index_selected = srvidx;
  frontend_set_state(FeSt_NET_SESSION);
  return 1;
}

unsigned long get_host_player_id(void) {
    return 0;
}

int setup_old_network_service(void)
{
    return setup_network_service(net_service_index_selected);
}

static CoroutineLoopState setup_exchange_player_number(CoroutineLoop *context)
{
  SYNCDBG(6,"Starting");
  clear_packets();
  struct PlayerInfo* player = get_my_player();
  struct Packet* pckt = get_packet_direct(my_player_number);
  set_packet_action(pckt, PckA_InitPlayerNum, player->is_active, settings.video_rotate_mode, 0, 0);
  if (LbNetwork_Exchange(NETMSG_SMALLDATA, pckt, game.packets, sizeof(struct Packet)))
      ERRORLOG("Network Exchange failed");
  int k = 0;
  for (int i = 0; i < NET_PLAYERS_COUNT; i++)
  {
      pckt = get_packet_direct(i);
      if (is_packet_empty(pckt))
      {
          MULTIPLAYER_LOG("setup_network_multiplayer_game: packet[%d] is EMPTY, skipping", i);
          continue;
      }
      if ((net_player_info[i].active) && (pckt->action == PckA_InitPlayerNum))
      {
          player = get_player(k);
          player->id_number = k;
          player->allocflags |= PlaF_Allocated;
          switch (pckt->actn_par2) {
              case 0: player->view_mode_restore = PVM_IsoWibbleView; break;
              case 1: player->view_mode_restore = PVM_IsoStraightView; break;
              case 2: player->view_mode_restore = PVM_FrontView; break;
              default: player->view_mode_restore = PVM_IsoWibbleView; break;
          }
          player->is_active = pckt->actn_par1;
          init_player(player, 0);
          snprintf(player->player_name, sizeof(struct TbNetworkPlayerName), "%s", net_player[i].name);
          k++;
      }
  }
  if (k != game.active_players_count)
  {
      return CLS_REPEAT; // Repeat
  }
  return CLS_CONTINUE; // Skip loop to next function
}

static short setup_select_player_number(void)
{
    short is_set = 0;
    int k = 0;
    SYNCDBG(6, "Starting");
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (net_player_info[i].active)
        {
            player->packet_num = i;
            if ((!is_set) && (my_player_number == i))
            {
                is_set = 1;
                my_player_number = k;
            }
            k++;
        }
    }
    return is_set;
}

void setup_count_players(void)
{
  if (game.game_kind == GKind_LocalGame)
  {
    game.active_players_count = 1;
  } else
  {
    game.active_players_count = 0;
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
      if (net_player_info[i].active)
        game.active_players_count++;
    }
  }
}

void init_players_network_game(CoroutineLoop *context)
{
  SYNCDBG(4,"Starting");
  setup_select_player_number();
  coroutine_add(context, &setup_exchange_player_number);
  coroutine_add(context, &perform_checksum_verification);
  coroutine_add(context, &setup_alliances);
}

/** Check whether a network player is active.
 *
 * @param plyr_idx
 * @return
 */
TbBool network_player_active(int plyr_idx)
{
    if ((plyr_idx < 0) || (plyr_idx >= NET_PLAYERS_COUNT))
        return false;
    return (net_player_info[plyr_idx].active != 0);
}

const char *network_player_name(int plyr_idx)
{
    if ((plyr_idx < 0) || (plyr_idx >= NET_PLAYERS_COUNT))
        return NULL;
    return net_player[plyr_idx].name;
}

long network_session_join(void)
{
    int32_t plyr_num;
    display_attempting_to_join_message();
    if ( LbNetwork_Join(net_session[net_session_index_active], net_player_name, &plyr_num, NULL) )
    {
      process_network_error(-802);
      return -1;
    }
    return plyr_num;
}

void sync_various_data()
{
   if ((game.system_flags & GSF_NetworkActive) == 0) {
      return;
   }

   struct {
      unsigned long action_random_seed;
      int input_lag_turns;
   } initial_sync_data;

   initial_sync_data.action_random_seed = game.action_random_seed;
   initial_sync_data.input_lag_turns = game.input_lag_turns;
   if (!LbNetwork_Resync(&initial_sync_data, sizeof(initial_sync_data))) {
      ERRORLOG("Initial sync failed");
      return;
   }
   game.action_random_seed = initial_sync_data.action_random_seed;
   game.ai_random_seed = game.action_random_seed * 9377 + 9391;
   game.player_random_seed = game.action_random_seed * 9473 + 9479;
   game.input_lag_turns = initial_sync_data.input_lag_turns;
   game.skip_initial_input_turns = calculate_skip_input();
   NETLOG("Initial network state synced: action_seed=%u, input_lag=%d", game.action_random_seed, game.input_lag_turns);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
