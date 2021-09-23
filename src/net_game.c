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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
DLLIMPORT struct _GUID _DK_net_guid;
#define net_guid _DK_net_guid
DLLIMPORT int _DK_noOfEnumeratedDPlayServices;
#define noOfEnumeratedDPlayServices _DK_noOfEnumeratedDPlayServices
DLLIMPORT struct _GUID _DK_clientGuidTable[];
#define clientGuidTable _DK_clientGuidTable
/******************************************************************************/
long number_of_comports;
long number_of_speeds;
long net_comport_scroll_offset;
long net_speed_scroll_offset;
char tmp_net_irq[8];
char net_current_message[64];
long net_current_message_index;
/******************************************************************************/
short setup_network_service(int srvidx)
{
  struct ServiceInitData *init_data;
  long maxplayrs;

  switch (srvidx)
  {
  case 0:
      maxplayrs = 2;
      init_data = &net_serial_data;
      set_flag_byte(&game.flags_font,FFlg_unk10,true);
      SYNCMSG("Initializing %d-players serial network",maxplayrs);
      break;
  case 1:
      maxplayrs = 2;
      init_data = &net_modem_data;
      set_flag_byte(&game.flags_font,FFlg_unk10,true);
      SYNCMSG("Initializing %d-players modem network",maxplayrs);
      break;
  case 2:
      maxplayrs = 4;
      init_data = NULL;
      set_flag_byte(&game.flags_font,FFlg_unk10,false);
      SYNCMSG("Initializing %d-players IPX network",maxplayrs);
      break;
  default:
      maxplayrs = 4;
      init_data = NULL;
      set_flag_byte(&game.flags_font,FFlg_unk10,false);
      SYNCMSG("Initializing %d-players type %d network",maxplayrs,srvidx);
      break;
  }
  LbMemorySet(&net_player_info[0], 0, sizeof(struct TbNetworkPlayerInfo));
  if ( LbNetwork_Init(srvidx, maxplayrs, &net_screen_packet,
      sizeof(struct ScreenPacket), &net_player_info[0], init_data) )
  {
    if (srvidx != 0)
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

static CoroutineLoopState setup_exchange_player_number(CoroutineLoop *context)
{
  SYNCDBG(6,"Starting");
  clear_packets();
  struct PlayerInfo* player = get_my_player();
  struct Packet* pckt = get_packet_direct(my_player_number);
  set_packet_action(pckt, PckA_InitPlayerNum, player->is_active, settings.video_rotate_mode, 0, 0);
  if (LbNetwork_Exchange(pckt))
      ERRORLOG("Network Exchange failed");
  int k = 0;
  for (int i = 0; i < NET_PLAYERS_COUNT; i++)
  {
      pckt = get_packet_direct(i);
      if ((net_player_info[i].active) && (pckt->action == PckA_InitPlayerNum))
      {
          player = get_player(k);
          player->id_number = k;
          player->allocflags |= PlaF_Allocated;
          if (pckt->actn_par2 < 1)
            player->view_mode_restore = PVM_IsometricView;
          else
            player->view_mode_restore = PVM_FrontView;
          player->is_active = pckt->actn_par1;
          init_player(player, 0);
          strncpy(player->field_15,net_player[i].name,sizeof(struct TbNetworkPlayerName));
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
  if (LbNetwork_ChangeExchangeBuffer(game.packets, sizeof(struct Packet)))
      ERRORLOG("Unable to reinitialize ExchangeBuffer");
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

void set_network_player_name(int plyr_idx, const char *name)
{
    if ((plyr_idx < 0) || (plyr_idx >= NET_PLAYERS_COUNT)) {
        ERRORLOG("Outranged network player %d",plyr_idx);
        return;
    }
    strncpy(net_player[plyr_idx].name, name, sizeof(net_player[0].name));
}

long network_session_join(void)
{
    unsigned long plyr_num;
    void *conn_options;
    switch (net_service_index_selected)
    {
    case 1:
      modem_dev.field_0 = 0;
      modem_dev.field_4 = 0;
      strcpy(modem_dev.field_58, net_config_info.str_join);
      modem_dev.field_AC = modem_initialise_callback;
      modem_dev.field_B0 = modem_connect_callback;
      conn_options = &modem_dev;
      break;
    default:
      display_attempting_to_join_message();
      conn_options = NULL;
      break;
    }
    if ( LbNetwork_Join(net_session[net_session_index_active], net_player_name, &plyr_num, conn_options) )
    {
      if (net_service_index_selected == 1)
        process_network_error(modem_dev.field_A8);
      else
        process_network_error(-802);
      return -1;
    }
    return plyr_num;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
