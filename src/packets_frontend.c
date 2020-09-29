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

static TbBool process_frontend_packet_cb(void *context, unsigned long turn, int plyr_idx, unsigned char kind, void *packet_data, short size)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct ScreenPacket* nspckt = (struct ScreenPacket*)packet_data;
    if ((nspckt->flags_4 & SPF_PlayerActive) != 0)
    {
        long k;
        switch (nspckt->flags_4 >> 3)
        {
        case 2:
            add_message(plyr_idx, (char*)&nspckt->param1);
            break;
        case 3:
            if (!validate_versions())
            {
                versions_different_error();
                break;
            }
            fe_network_active = 1;
            frontend_set_state(FeSt_NETLAND_VIEW);
            break;
        case 4:
            frontend_set_alliance(nspckt->param1, nspckt->param2);
            break;
        case 7:
            fe_computer_players = nspckt->param1;
            break;
        case 8:
        {
            k = strlen(player->mp_message_text);
            unsigned short c;
            if (nspckt->param1 == KC_BACK)
            {
                if (k > 0)
                {
                    k--;
                    player->mp_message_text[k] = '\0';
                }
            }
            else if (nspckt->param1 == KC_RETURN)
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
                c = key_to_ascii(nspckt->param1, nspckt->param2);
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
        if (nspckt->field_5 != -1)
          frontend_alliances = nspckt->field_5;
      }
      if (fe_computer_players == 2)
      {
        k = ((nspckt->flags_4 & 0x06) >> 1);
        if (k != 2)
          fe_computer_players = k;
      }
      player->field_4E7 = nspckt->mouse_y + (nspckt->mouse_x << 8);
    }
    nspckt->flags_4 &= 0x07;
    return true;
}

void process_frontend_packets(void)
{
  long i;
  static int failed_net_times = 0;
  for (i=0; i < NET_PLAYERS_COUNT; i++) //TODO: remove it
  {
    net_screen_packet_NEW[i].flags_4 &= ~SPF_PlayerActive;
  }
  struct ScreenPacket* nspckt = LbNetwork_AddPacket(PckA_Frontmenu, 0, sizeof(struct ScreenPacket));
  nspckt->flags_4 |= 0x01;
  nspckt->field_5 = frontend_alliances;
  nspckt->flags_4 ^= ((nspckt->flags_4 ^ (fe_computer_players << 1)) & 0x06);
  nspckt->mouse_x = VersionMajor;
  nspckt->mouse_y = VersionMinor;
  if (LbNetwork_Exchange(NULL, &process_frontend_packet_cb) != NR_OK)
  {
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
  }
  else
  {
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
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    nspckt = &net_screen_packet_NEW[i];
    if ((nspckt->flags_4 & SPF_PlayerActive) == 0)
    {
      if (frontend_is_player_allied(my_player_number, i))
        frontend_set_alliance(my_player_number, i);
    }
  }
}
