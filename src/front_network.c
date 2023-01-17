/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_network.c
 *     Front-end menus for network games.
 * @par Purpose:
 *     Functions to maintain network-related frontend screens.
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
#include "front_network.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_network.h"
#include "bflib_netsession.h"
#include "bflib_guibtns.h"
#include "bflib_keybrd.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
#include "bflib_memory.h"
#include "bflib_datetm.h"
#include "bflib_fileio.h"

#include "kjm_input.h"
#include "gui_draw.h"
#include "front_simple.h"
#include "front_landview.h"
#include "frontmenu_net.h"
#include "frontend.h"
#include "player_data.h"
#include "net_game.h"
#include "packets.h"
#include "config.h"
#include "config_strings.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char *keeper_netconf_file = "fxconfig.net";
#define DEDUP_MAX_TICK 31

const struct ConfigInfo default_net_config_info = {
    "",
    "Player",
};

int fe_network_active;
int net_service_index_selected;
char tmp_net_player_name[24];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void process_network_error(long errcode)
{
  const char *text;
  switch (errcode)
  {
  case 4:
      text = get_string(GUIStr_NetLineEngaged);
      break;
  case 5:
      text = get_string(GUIStr_NetUnknownError);
      break;
  case 6:
      text = get_string(GUIStr_NetNoCarrier);
      break;
  case 7:
      text = get_string(GUIStr_NetNoDialTone);
      break;
  case -1:
      text = get_string(GUIStr_NetNoResponse);
      break;
  case -2:
      text = get_string(GUIStr_NetNoServer);
      break;
  case -800:
      text = get_string(GUIStr_NetUnableToInit);
      break;
  case -801:
      text = get_string(GUIStr_NetUnableToCrGame);
      break;
  case -802:
      text = get_string(GUIStr_NetUnableToJoin);
      break;
  default:
      ERRORLOG("Unknown modem error code %ld",errcode);
      return;
  }
  //display_centered_message(3000, text);
  create_frontend_error_box(3000, text);
}

void draw_out_of_sync_box(long a1, long a2, long box_width)
{
    long min_width = 2 * a1;
    long max_width = 2 * a2;
    if (min_width > max_width)
    {
        min_width = max_width;
    }
    if (min_width < 0)
    {
        min_width = 0;
    }
    int units_per_px = units_per_pixel;
    if (LbScreenLock() == Lb_SUCCESS)
    {
        long ornate_width = 200 * units_per_px / 16;
        long ornate_height = 100 * units_per_px / 16;
        long x = box_width + (MyScreenWidth - box_width - ornate_width) / 2;
        long y = (MyScreenHeight - ornate_height) / 2;
        draw_ornate_slab64k(x, y, units_per_px, ornate_width, ornate_height);
        LbTextSetFont(winfont);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
        LbTextSetWindow(x, y, ornate_width, ornate_height);
        int tx_units_per_px = (22 * units_per_px) / LbTextLineHeight();
        long text_h = LbTextLineHeight() * tx_units_per_px / 16;
        long text_x = x + 100 * units_per_px / 16 - max_width;
        long text_y = y + 58 * units_per_px / 16;
        LbTextDrawResized(0, 50*units_per_px/16 - text_h, tx_units_per_px, get_string(GUIStr_NetResyncing));
        LbDrawBox(text_x, text_y, 2*max_width, 16*units_per_px/16, 0);
        LbDrawBox(text_x, text_y, 2*min_width, 16*units_per_px/16, 133);
        LbScreenUnlock();
        LbScreenSwap();
    }
}

CoroutineLoopState setup_alliances(CoroutineLoop *loop)
{
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (!is_my_player_number(i) && player_exists(player))
        {
            if (frontend_is_player_allied(my_player_number, i))
            {
                set_ally_with_player(my_player_number, i, true);
                set_ally_with_player(i, my_player_number, true);
            }
        }
    }
    return CLS_CONTINUE; // Exit the loop
}

void frontnet_service_update(void)
{
    if (net_number_of_services < 1)
    {
        net_service_scroll_offset = 0;
    } else
    if (net_service_scroll_offset < 0)
    {
        net_service_scroll_offset = 0;
    } else
    if (net_service_scroll_offset > net_number_of_services - 1)
    {
        net_service_scroll_offset = net_number_of_services - 1;
    }
}

void enum_players_callback(struct TbNetworkCallbackData *netcdat, TbBool is_new)
{
    if (net_number_of_enum_players >= 4)
    {
        ERRORLOG("Too many players in enumeration");
        return;
    }
    // TODO: keep order
    snprintf(net_player_info[net_number_of_enum_players].name, sizeof(net_player_info[0].name), "%s", netcdat->plyr_name);
    net_player_info[net_number_of_enum_players].active = 1;
    if (is_new)
    {
        net_player_info[net_number_of_enum_players].last_packet_tick = 0;
    }
    net_number_of_enum_players++;
}

void enum_sessions_callback(struct TbNetworkCallbackData *netcdat, TbBool is_new)
{
    if (net_number_of_sessions >= 32)
    {
        ERRORLOG("Too many sessions in enumeration");
        return;
    }
    if (net_service_index_selected >= 0)
    {
        net_session[net_number_of_sessions] = (struct TbNetworkSessionNameEntry *)netcdat;
        net_number_of_sessions++;
    } else
    if (net_number_of_sessions == 0)
    {
        net_session[net_number_of_sessions] = (struct TbNetworkSessionNameEntry *)netcdat;
        strcpy(&netcdat->svc_name[8],get_string(GUIStr_NetModem));
        net_number_of_sessions++;
    }
}

// TODO: remove all this weird stuff
static void enum_services_callback(struct TbNetworkCallbackData *netcdat, TbBool is_new)
{
    if (net_number_of_services >= NET_SERVICES_COUNT)
    {
      ERRORLOG("Too many services in enumeration");
      return;
    }
    if (strcasecmp("TCP", netcdat->svc_name) == 0)
    {
        LbStringCopy(net_service[net_number_of_services], "TCP/IP", NET_MESSAGE_LEN);//TODO TRANSLATION put this in GUI strings
        net_number_of_services++;
    }
    else if (strcasecmp("ENET/UDP", netcdat->svc_name) == 0)
    {
        LbStringCopy(net_service[net_number_of_services], netcdat->svc_name, NET_MESSAGE_LEN);//TODO TRANSLATION put this in GUI strings
        net_number_of_services++;
    } else
    {
        ERRORLOG("Unrecognized Network Service");
    }
}

void frontnet_session_update(void)
{
    static long last_enum_players = 0;
    static long last_enum_sessions = 0;

    if (LbTimerClock() >= last_enum_sessions)
    {
      net_number_of_sessions = 0;
      LbMemorySet(net_session, 0, sizeof(net_session));
      if ( LbNetwork_EnumerateSessions(enum_sessions_callback, 0) )
        ERRORLOG("LbNetwork_EnumerateSessions() failed");
      last_enum_sessions = LbTimerClock();

      if (net_number_of_sessions == 0)
      {
        net_session_index_active = -1;
      } else
      if (net_session_index_active != -1)
      {
          if ((net_session_index_active >= net_number_of_sessions)
            || (!net_session[net_session_index_active]->joinable))
          {
            net_session_index_active = -1;
            for (long i = 0; i < net_number_of_sessions; i++)
            {
              if (net_session[i]->joinable)
              {
                net_session_index_active = i;
                break;
              }
            }
          }
      }
    }

    if ((net_number_of_sessions == 0) || (net_session_scroll_offset < 0))
    {
      net_session_scroll_offset = 0;
    } else
    if (net_session_scroll_offset > net_number_of_sessions-1)
    {
      net_session_scroll_offset = net_number_of_sessions-1;
    }

    if (net_session_index_active == -1)
    {
      net_number_of_enum_players = 0;
    } else
    if (LbTimerClock() >= last_enum_players)
    {
      net_number_of_enum_players = 0;
      for (int i = 0; i < NET_PLAYERS_COUNT; i++)
      {
          net_player_info[i].name[0] = 0;
          net_player_info[i].active = 0;
      }
      if ( LbNetwork_EnumeratePlayers(net_session[net_session_index_active], enum_players_callback, 0) )
      {
        net_session_index_active = -1;
        return;
      }
      last_enum_players = LbTimerClock();
    }

    if (net_number_of_enum_players == 0)
    {
      net_player_scroll_offset = 0;
    } else
    if (net_player_scroll_offset < 0)
    {
      net_player_scroll_offset = 0;
    } else
    if (net_player_scroll_offset > net_number_of_enum_players-1)
    {
      net_player_scroll_offset = net_number_of_enum_players-1;
    }
}

void frontnet_rewite_net_messages(void)
{
    struct NetMessage lmsg[NET_MESSAGES_COUNT];
    long k = 0;
    long i = net_number_of_messages;
    for (i=0; i < NET_MESSAGES_COUNT; i++)
      LbMemorySet(&lmsg[i], 0, sizeof(struct NetMessage));
    for (i=0; i < net_number_of_messages; i++)
    {
        struct NetMessage* nmsg = &net_message[i];
        if (network_player_active(nmsg->plyr_idx))
        {
            memcpy(&lmsg[k], nmsg, sizeof(struct NetMessage));
            k++;
      }
    }
    net_number_of_messages = k;
    for (i=0; i < NET_MESSAGES_COUNT; i++)
      memcpy(&net_message[i], &lmsg[i], sizeof(struct NetMessage));
}

void frontnet_start_update(CoroutineLoop *context)
{
    static TbClockMSec player_last_time = 0;
    SYNCDBG(18,"Starting");
    if (LbTimerClock() >= player_last_time+200)
    {
        net_number_of_enum_players = 0;
        for (int i = 0; i < NET_PLAYERS_COUNT; i++)
        {
            net_player_info[i].name[0] = 0;
            net_player_info[i].active = 0;
        }

        if (net_session_index_active < 0)
        {
            net_session_index_active = 0; // I am the host here
        }

        // Pulling Connected Client names from Backend
        if ( LbNetwork_EnumeratePlayers(net_session[net_session_index_active], enum_players_callback, 0) )
        {
            ERRORLOG("LbNetwork_EnumeratePlayers() failed");
            return;
        }
        player_last_time = LbTimerClock();
    }
    if ((net_number_of_messages <= 0) || (net_message_scroll_offset < 0))
    {
      net_message_scroll_offset = 0;
    }
    else if (net_message_scroll_offset > net_number_of_messages-1)
    {
      net_message_scroll_offset = net_number_of_messages-1;
    }
    process_frontend_packets(context);
    frontnet_rewite_net_messages();
}

void display_attempting_to_join_message(void)
{
  if (LbScreenLock() == Lb_SUCCESS)
  {
    draw_text_box(get_string(GUIStr_NetAttemptingToJoin));
    LbScreenUnlock();
  }
  LbScreenSwap();
}

void net_load_config_file(void)
{
    // Try to load the config file
    char* fname = prepare_file_path(FGrp_Save, keeper_netconf_file);
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (handle != -1)
    {
      if (LbFileRead(handle, &net_config_info, sizeof(net_config_info)) == sizeof(net_config_info))
      {
        LbFileClose(handle);
        return;
      }
      LbFileClose(handle);
    }
    // If can't load, then use default config
    LbMemoryCopy(&net_config_info, &default_net_config_info, sizeof(net_config_info));
    LbStringCopy(net_config_info.net_player_name, get_string(GUIStr_MnuNoName), 20);
}

void net_write_config_file(void)
{
    // Try to load the config file
    char* fname = prepare_file_path(FGrp_Save, keeper_netconf_file);
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (handle != -1)
    {
        LbFileWrite(handle, &net_config_info, sizeof(net_config_info));
        LbFileClose(handle);
    }
}

void frontnet_service_setup(void)
{
    net_number_of_services = 0;
    LbMemorySet(net_service, 0, sizeof(net_service));
    // Create list of available services
    if (LbNetwork_EnumerateServices(enum_services_callback, NULL)) {
        ERRORLOG("LbNetwork_EnumerateServices() failed");
    }
    // Create skirmish option if it should be enabled
    if ((game.system_flags & GSF_AllowOnePlayer) != 0)
    {
        LbStringCopy(net_service[net_number_of_services], get_string(GUIStr_Net1Player), NET_SERVICE_LEN);
        net_number_of_services++;
    }
    net_load_config_file();
}

void frontnet_session_setup(void)
{
    if (net_player_name[0] == '\0')
    {
        snprintf(net_player_name, sizeof(net_player_name), "%s", net_config_info.net_player_name);
        strcpy(tmp_net_player_name, net_config_info.net_player_name);
    }
    net_session_index_active = -1;
    fe_computer_players = 2;
    lbInkey = 0;
}

void frontnet_start_setup(void)
{
    frontend_alliances = -1;
    net_number_of_messages = 0;
    net_player_scroll_offset = 0;
    net_message_scroll_offset = 0;
    //net_old_number_of_players = 0;
    players_currently_in_session = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        player->mp_message_text[0] = '\0';
    }
}

/******************************************************************************/
static TbBool frontnet_quit_all()
{
    int i = frontnet_number_of_players_in_session();
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
                return true;
            }
            frontend_set_state(FeSt_MAIN_MENU);
        }
        else if (frontend_menu_state == FeSt_NET_START)
        {
            if (LbNetwork_Stop())
            {
                ERRORLOG("LbNetwork_Stop() failed");
                return true;
            }
            if (setup_network_service(net_service_index_selected))
            {
                frontend_set_state(FeSt_NET_SESSION);
            }
            else
            {
                frontend_set_state(FeSt_MAIN_MENU);
            }
        }
    }
    return false;
}


static TbBool process_frontend_packets_cb(void *context, unsigned long turn, int net_player_idx, unsigned char kind, void *packet_data, short size)
{
    // TODO: Guard net_player_idx against overflow
    if (kind == PckA_LandviewFrameSrv)
    {
        assert(size <= sizeof(net_screen_packet));
        memcpy(&net_screen_packet, packet_data, size);
    }
    else
    {
        ERRORLOG("Unexpected packet kind: %d", kind);
    }
    return false;
}

static TbBool process_frontend_packets_server_cb(void *context, unsigned long turn, int net_player_idx, unsigned char kind, void *packet_data, short size)
{
    // TODO: Guard net_player_idx against overflow
    if (kind == PckA_LandviewFrameCli)
    {
        memcpy(&net_screen_packet[net_player_idx], packet_data, size);
    }
    else
    {
        ERRORLOG("Unexpected packet kind: %d", kind);
    }
    return false;
}

void process_frontend_packets(CoroutineLoop *context)
{
    static TbClockMSec next_time = 0;
    TbClockMSec now_time = LbTimerClock();
    TbBool need_packet = false;
    if (now_time > next_time)
    {
        next_time = now_time + 300; // Send events sometimes
        need_packet = true;
    }

    for (int i = 0; i < NET_PLAYERS_COUNT; i++) // TODO weird
    {
        net_screen_packet[i].flags &= ~0x01;
    }
    struct ScreenPacket* nspckt = &net_screen_packet[my_player_number];
    set_flag_value(nspckt->flags, 0x01, true);
    nspckt->frontend_alliances = frontend_alliances;
    nspckt->flags ^= ((nspckt->flags ^ (fe_computer_players << 1)) & 0x06);
    nspckt->field_6 = VersionMajor;
    nspckt->field_8 = VersionMinor;
    if (LbNetwork_IsServer())
    {
        if (LbNetwork_Exchange(net_screen_packet, &process_frontend_packets_server_cb))
        {
            ERRORLOG("LbNetwork_Exchange failed");
            net_service_index_selected = -1; // going to quit
        }
        if (nspckt->event != 0)
        {
            need_packet = true;
        }
        if (need_packet)
        {
            nspckt->tick = (net_player_info[my_player_number].last_packet_tick + 1) & DEDUP_MAX_TICK; // Deduplication
        }
        for (int i=0; i < NET_PLAYERS_COUNT; i++)
        {
            if (net_screen_packet[i].event != 0)
            {
                need_packet = true; // Forward packets for other players
                break;
            }
        }
        if (need_packet)
        {
            void *outgoing = LbNetwork_AddPacket(PckA_LandviewFrameSrv, 0,
                                                 sizeof(struct ScreenPacket) * NET_PLAYERS_COUNT);
            memcpy(outgoing, net_screen_packet, sizeof(struct ScreenPacket) * NET_PLAYERS_COUNT);
        }
    }
    else
    {
        if (nspckt->event != 0)
        {
            need_packet = true;
        }
        if (need_packet)
        {
            void *outgoing = LbNetwork_AddPacket(PckA_LandviewFrameCli, 0,
                                                 sizeof(struct ScreenPacket));

            nspckt->tick = (net_player_info[my_player_number].last_packet_tick + 1) & DEDUP_MAX_TICK; // Deduplication

            memcpy(outgoing, nspckt, sizeof(struct ScreenPacket));
        }
        if (LbNetwork_Exchange(net_screen_packet, &process_frontend_packets_cb))
        {
            ERRORLOG("LbNetwork_Exchange failed");
            net_service_index_selected = -1; // going to quit
        }
    }
    if (frontend_should_all_players_quit())
    {
        if (frontnet_quit_all())
        {
            return;
        }
    }
#if DEBUG_NETWORK_PACKETS
    write_debug_screenpackets();
#endif
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        nspckt = &net_screen_packet[i];
        struct PlayerInfo* player = get_player(i);
        if (nspckt->event != 0)
        {
            uint8_t diff = (nspckt->tick - net_player_info[i].last_packet_tick) & DEDUP_MAX_TICK; // Deduplication
            if ((diff == 0) || (diff >= (DEDUP_MAX_TICK / 2))) // Some packets could be reordered or lost but not that much
            {
                nspckt->event = 0;
                continue;
            }
            NETLOG("event:%d for:%d tick:%d old:%d diff:%d", nspckt->event, i, nspckt->tick, net_player_info[i].last_packet_tick,
                   diff);
            net_player_info[i].last_packet_tick = nspckt->tick;

            long k;
            switch (nspckt->event)
            {
                case 2:
                    add_message(i, (char*)&nspckt->param1);
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
                    if (nspckt->key == KC_BACK)
                    {
                        if (k > 0)
                        {
                            k--;
                            player->mp_message_text[k] = '\0';
                        }
                    }
                    else if (nspckt->key == KC_RETURN)
                    {
                        if (k > 0)
                        {
                            add_message(i, player->mp_message_text);
                            k = 0;
                            player->mp_message_text[k] = '\0';
                        }
                    }
                    else
                    {
                        c = key_to_ascii(nspckt->key, nspckt->shift);
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
                if (nspckt->frontend_alliances != -1)
                    frontend_alliances = nspckt->frontend_alliances;
            }
            if (fe_computer_players == 2)
            {
                k = ((nspckt->flags & 0x06) >> 1);
                if (k != 2)
                    fe_computer_players = k;
            }
            player->game_version = nspckt->field_8 + (nspckt->field_6 << 8);
            nspckt->event = 0;
        }
    }
    if (frontend_alliances == -1)
        frontend_alliances = 0;
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        nspckt = &net_screen_packet[i];
        if ((nspckt->flags & 0x01) == 0)
        {
            if (frontend_is_player_allied(my_player_number, i))
                frontend_set_alliance(my_player_number, i);
        }
    }
}
