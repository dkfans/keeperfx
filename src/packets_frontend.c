/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_frontend.c
 *     Packet processing routines.
 * @par Purpose:
 *     Functions for creating and executing packets.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     29 Sep 2020 -
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packets.h"

#include "bflib_keybrd.h"
#include "front_landview.h"
#include "front_network.h"
#include "frontend.h"
#include "frontmenu_net.h"
#include "game_saves.h"
#include "game_merge.h"
#include "kjm_input.h"
#include "net_game.h"
#include "player_data.h"

/**
 * Checks if all the network players are using compatible version of DK.
 */
static TbBool validate_versions(void)
{
/*
    struct PlayerInfo *player;
    long i;
    long ver;
    ver = -1;
    for (i=0; i < NET_PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      struct ScreenPacket *nscpk = &net_screen_packet_NEW[i];
      if ((nscpk->flags_4 & SPF_PlayerActive) != 0)
      {
        if (ver == -1)
          ver = player->field_4E7;
        if (player->field_4E7 != ver)
          return false;
      }
    }
    */
    return true;
}

static TbBool process_frontend_packet_cb(void *context, unsigned long turn, int plyr_idx, unsigned char kind, void *packet_data, short size)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct ScreenPacket* nspck = (struct ScreenPacket*)packet_data;

#if NETDBG_LEVEL > 6
    static unsigned char flags[6] = {255,255,255,255,255,255};
    if (nspck->flags_4 != flags[plyr_idx])
    {
        flags[plyr_idx] = nspck->flags_4;
        NETLOG("ui_turn: %04ld player:%d flag_a:%d flag_b:%0x",
            ui_turn, plyr_idx, nspck->flags_4 >> 3, nspck->flags_4 & SPF_FlagsMask);
    }
#endif

    if ((nspck->flags_4 & SPF_PlayerActive) != 0)
    {
        long k;
        switch (nspck->flags_4 >> 3)
        {
        case 2:
            add_message(plyr_idx, (char*)&nspck->param1);
            break;
        case SPF_StartGame >> 3:
            if (!validate_versions())
            {
                versions_different_error();
                break;
            }
            fe_network_active = 1;
            frontend_set_state(FeSt_NETLAND_VIEW);
            break;
        case 4:
            frontend_set_alliance(nspck->param1, nspck->param2);
            break;
        case 7:
            fe_computer_players = nspck->param1;
            break;
        case SPF_ChatLetter >> 3:
        {
            k = strlen(player->mp_message_text);
            unsigned short c;
            if (nspck->param1 == KC_BACK)
            {
                if (k > 0)
                {
                    k--;
                    player->mp_message_text[k] = '\0';
                }
            }
            else if (nspck->param1 == KC_RETURN)
            {
                if (k > 0)
                {
                    add_message(plyr_idx, player->mp_message_text);
                    k = 0;
                    player->mp_message_text[k] = '\0';
                }
            }
            else
            {
                c = key_to_ascii(nspck->param1, nspck->param2);
                if ((c != 0) && (frontend_font_char_width(1, c) > 1) && (k < 62))
                {
                    player->mp_message_text[k] = c;
                    k++;
                    player->mp_message_text[k] = '\0';
                }
            }
            if (frontend_font_string_width(1, player->mp_message_text) >= 420)
            {
                if (k > 0)
                {
                    k--;
                    player->mp_message_text[k] = '\0';
                }
            }
            break;
        }
      default:
        break;
      }
      if (frontend_alliances == -1)
      {
        if (nspck->field_5 != -1)
          frontend_alliances = nspck->field_5;
      }
      if (fe_computer_players == 2)
      {
        k = ((nspck->flags_4 & 0x06) >> 1); // SPF_ComputerPlayer == 4
        if (k != 2)
          fe_computer_players = k;
      }
      player->field_4E7 = nspck->mouse_y + (nspck->mouse_x << 8);
    }
    //TODO: useless because on client this packet will be deleted
    //nspck->flags_4 &= SPF_FlagsMask; 
    return true;
}

void process_frontend_packets(void)
{
  long i;
  static int failed_net_times = 0;
  struct ScreenPacket* nspck = LbNetwork_AddPacket(PckA_Frontmenu, 0, sizeof(struct ScreenPacket));
  nspck[0] = net_screen_packet_NEW[my_player_number];
  /* send version once 
  nspck->flags_4 |= SPF_PlayerActive;
  nspck->field_5 = frontend_alliances;
  nspck->flags_4 ^= ((nspck->flags_4 ^ (fe_computer_players << 1)) & 0x06);
  nspck->mouse_x = VersionMajor;
  nspck->mouse_y = VersionMinor;
  */
  if (LbNetwork_Exchange(NULL, &process_frontend_packet_cb) != NR_OK)
  {
    LbNetwork_EmptyQueue();
    ERRORLOG("LbNetwork_Exchange failed %d", failed_net_times);
    failed_net_times++;
    if (failed_net_times < net_max_failed_login_turns)
    {
      return; // nothing to process
    }
    else
    {
      if (LbNetwork_Stop())
      {
        ERRORLOG("LbNetwork_Stop() failed");
        return;
      }
      if (setup_network_service(net_service_index_selected))
      {
        frontend_set_state(FeSt_NET_SESSION);
      } else
      {
        frontend_set_state(FeSt_MAIN_MENU);
      }
      process_network_error(-1);
      return;
    }
  } // if exchange != ok
  else
  {
    LbNetwork_EmptyQueue();
    failed_net_times = 0;
  }

  if (frontend_should_all_players_quit())
  {
    i = frontnet_number_of_players_in_session();
    if (players_currently_in_session < i)
    {
      players_currently_in_session = i;
    }
    if (players_currently_in_session > i)
    {
      if (frontend_menu_state == FeSt_NET_SESSION)
      {
          if (LbNetwork_Stop())
          {
            ERRORLOG("LbNetwork_Stop() failed");
            return;
          }
          frontend_set_state(FeSt_MAIN_MENU);
      } else
      if (frontend_menu_state == FeSt_NET_START)
      {
          if (LbNetwork_Stop())
          {
            ERRORLOG("LbNetwork_Stop() failed");
            return;
          }
          if (setup_network_service(net_service_index_selected))
          {
            frontend_set_state(FeSt_NET_SESSION);
          } else
          {
            frontend_set_state(FeSt_MAIN_MENU);
          }
      }
    }
  }
  if (frontend_alliances == -1)
    frontend_alliances = 0;
  // TODO: this should not work because we dont move packets back to New
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    nspck = &net_screen_packet_NEW[i];
    if ((nspck->flags_4 & SPF_PlayerActive) == 0)
    {
      if (frontend_is_player_allied(my_player_number, i))
        frontend_set_alliance(my_player_number, i);
    }
  }
}
