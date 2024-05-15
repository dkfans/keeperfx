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
#include "bflib_memory.h"
#include "bflib_network.h"

#include "player_data.h"
#include "front_landview.h"
#include "player_utils.h"
#include "packets.h"
#include "frontend.h"
#include "front_network.h"
#include "net_sync.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "post_inc.h"
#include "bflib_datetm.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct TbNetworkPlayerInfo net_player_info[NET_PLAYERS_COUNT];
struct TbNetworkSessionNameEntry *net_session[32];
long net_number_of_sessions;
long net_session_index_active;
struct ConfigInfo net_config_info;
char net_service[16][NET_SERVICE_LEN];
char net_player_name[20];
/******************************************************************************/
short setup_network_service(int srvidx)
{
  struct ServiceInitData *init_data = NULL;
  clear_flag(game.flags_font, FFlg_unk10);
  SYNCMSG("Initializing 4-players type %d network",srvidx);
  LbMemorySet(&net_player_info[0], 0, sizeof(struct TbNetworkPlayerInfo));
  if ( LbNetwork_Init(srvidx, NET_PLAYERS_COUNT, &net_player_info[0], init_data) )
  {
    if (srvidx > NS_ENET_UDP)
      process_network_error(-800);
    return 0;
  }
  net_service_index_selected = srvidx;
  if ((game.flags_font & FFlg_unk10) != 0)
    LbNetwork_ChangeExchangeTimeout(10);
  frontend_set_state(FeSt_NET_SESSION);
  return 1;
}

int setup_old_network_service(void)
{
    return setup_network_service(net_service_index_selected);
}

static TbBool setup_exchange_player_number_cb(void *context, unsigned long turn, int net_player_idx, unsigned char kind, void *packet_data, short size)
{
    if (kind == PckA_InitPlayerNum)
    {
        struct Packet *pckt = ((struct Packet*) context) + net_player_idx;
        memcpy(pckt, packet_data, size);
    }
    else
    {
        NETLOG("Unexpected packet %d from %d", kind, net_player_idx);
    }
    return false;
}

static CoroutineLoopState setup_exchange_player_number(CoroutineLoop *context)
{
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_my_player();
    TbClockMSec now = LbTimerClock();
    if (now > coroutine_vars(context)[0])
    {
        coroutine_vars(context)[0] = now + 500; // two times per second
        clear_packets(); // Only once

        struct Packet* pckt = LbNetwork_AddPacket(PckA_InitPlayerNum, 0, sizeof(struct Packet));
        set_packet_action(pckt, PckA_InitPlayerNum, player->is_active, settings.video_rotate_mode, 0, 0);
    }
    if (LbNetwork_Exchange(game.packets, &setup_exchange_player_number_cb))
    {
        ERRORLOG("Network Exchange failed");
        return CLS_ERROR;
    }
    int k = 0;
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        struct Packet* pckt = get_packet_direct(i);
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
            snprintf(player->player_name, sizeof(player->player_name) - 1, "%s", net_player_info[i].name);
            k++;
        }
    }
    if (k != game.active_players_count)
    {
        return CLS_REPEAT; // Repeat
    }
    NETLOG("Done");
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

int network_num_clients()
{
    // TODO: separate them for observers etc
    return game.active_players_count;
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
    return net_player_info[plyr_idx].name;
}

void set_network_player_name(int plyr_idx, const char *name)
{
    if ((plyr_idx < 0) || (plyr_idx >= NET_PLAYERS_COUNT)) {
        ERRORLOG("Outranged network player %d",plyr_idx);
        return;
    }
    snprintf(net_player_info[plyr_idx].name, sizeof(net_player_info[0].name), "%s", name);
}

long network_session_join(void)
{
    long plyr_num;
    display_attempting_to_join_message();
    if ( LbNetwork_Join(net_session[net_session_index_active], net_player_name, &plyr_num, NULL) )
    {
      process_network_error(-802);
      return -1;
    }
    return plyr_num;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
