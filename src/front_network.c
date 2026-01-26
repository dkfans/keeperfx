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
#include "bflib_datetm.h"
#include "bflib_fileio.h"

#include "kjm_input.h"
#include "gui_draw.h"
#include "front_simple.h"
#include "front_landview.h"
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

extern char autostart_multiplayer_campaign[80];
extern int autostart_multiplayer_level;

/******************************************************************************/
const char *keeper_netconf_file = "fxconfig.net";

const struct ConfigInfo default_net_config_info = {
    "",
    "Player",
};

int fe_network_active;
int net_service_index_selected;
char tmp_net_player_name[24];
static TbClockMSec frontnet_ping_stabilization_end_time = 0;
static int previous_player_count_for_ping_wait = -1;
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

void enum_players_callback(struct TbNetworkCallbackData *netcdat, void *a2)
{
    if (net_number_of_enum_players >= 4)
    {
        ERRORLOG("Too many players in enumeration");
        return;
    }
    snprintf(net_player[net_number_of_enum_players].name, sizeof(struct TbNetworkPlayerName), "%s", netcdat->plyr_name);
    net_number_of_enum_players++;
}

void enum_sessions_callback(struct TbNetworkCallbackData *netcdat, void *ptr)
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
static void enum_services_callback(struct TbNetworkCallbackData *netcdat, void *a2)
{
    if (net_number_of_services >= NET_SERVICES_COUNT)
    {
      ERRORLOG("Too many services in enumeration");
      return;
    }
    if (strcasecmp("TCP", netcdat->svc_name) == 0)
    {
        snprintf(net_service[net_number_of_services], NET_MESSAGE_LEN, "%s", "TCP/IP");//TODO TRANSLATION put this in GUI strings
        net_number_of_services++;
    }
    else if (strcasecmp("ENET/UDP", netcdat->svc_name) == 0)
    {
        snprintf(net_service[net_number_of_services], NET_MESSAGE_LEN, "%s", netcdat->svc_name);//TODO TRANSLATION put this in GUI strings
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
      memset(net_session, 0, sizeof(net_session));
      if ( LbNetwork_EnumerateSessions(enum_sessions_callback, 0) )
        ERRORLOG("LbNetwork_EnumerateSessions() failed");
      last_enum_sessions = LbTimerClock();

      if (net_number_of_sessions == 0)
      {
        net_session_index_active = -1;
        net_session_index_active_id = -1;
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
          if (net_session_index_active == -1)
            net_session_index_active_id = -1;
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
      memset(net_player, 0, sizeof(net_player));
      if ( LbNetwork_EnumeratePlayers(net_session[net_session_index_active], enum_players_callback, 0) )
      {
        net_session_index_active = -1;
        net_session_index_active_id = -1;
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
      memset(&lmsg[i], 0, sizeof(struct NetMessage));
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

TbBool frontnet_is_waiting_for_ping_stabilization(void)
{
    TbClockMSec now;
    if (net_number_of_enum_players != previous_player_count_for_ping_wait) {
        frontnet_ping_stabilization_end_time = LbTimerClock() + FRONTNET_PING_STABILIZATION_DELAY_MS;
        previous_player_count_for_ping_wait = net_number_of_enum_players;
    }
    if (net_number_of_enum_players < 2) {
        return true;
    }
    if (frontnet_ping_stabilization_end_time == 0) {
        return false;
    }
    now = LbTimerClock();
    if (now >= frontnet_ping_stabilization_end_time) {
        frontnet_ping_stabilization_end_time = 0;
        return false;
    }
    return true;
}

void frontnet_reset_ping_stabilization(void)
{
    frontnet_ping_stabilization_end_time = 0;
    previous_player_count_for_ping_wait = -1;
}

void handle_autostart_multiplayer_messaging(void)
{
    static TbBool send_pending = false;

    if (net_number_of_enum_players < 2) {
        send_pending = false;
        return;
    }
    if (!send_pending && my_player_number == get_host_player_id() &&
        (autostart_multiplayer_campaign[0] != '\0' || autostart_multiplayer_level > 0)) {
        send_pending = true;
    }
    if (send_pending && !frontnet_is_waiting_for_ping_stabilization()) {
        struct PlayerInfo *player = get_my_player();
        const char* camp = "keeporig";
        int level = 1;
        if (autostart_multiplayer_campaign[0]) {
            camp = autostart_multiplayer_campaign;
        }
        if (autostart_multiplayer_level > 0) {
            level = autostart_multiplayer_level;
        }
        snprintf(player->mp_message_text, PLAYER_MP_MESSAGE_LEN, "%s:%d", camp, level);
        lbInkey = KC_RETURN;
        send_pending = false;
    }
}

void frontnet_start_update(void)
{
    static TbClockMSec player_last_time = 0;
    SYNCDBG(18,"Starting");
    if (LbTimerClock() >= player_last_time+200)
    {
      net_number_of_enum_players = 0;
      memset(net_player, 0, sizeof(net_player));
      if ( LbNetwork_EnumeratePlayers(net_session[net_session_index_active], enum_players_callback, 0) )
      {
        ERRORLOG("LbNetwork_EnumeratePlayers() failed");
        return;
      }
      player_last_time = LbTimerClock();
    }

    handle_autostart_multiplayer_messaging();

    if ((net_number_of_messages <= 0) || (net_message_scroll_offset < 0))
    {
      net_message_scroll_offset = 0;
    }
    else if (net_message_scroll_offset > net_number_of_messages-1)
    {
      net_message_scroll_offset = net_number_of_messages-1;
    }
    process_frontend_packets();
    frontnet_rewite_net_messages();

    LbNetwork_UpdateInputLagIfHost();
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
    if (handle)
    {
      if (LbFileRead(handle, &net_config_info, sizeof(net_config_info)) == sizeof(net_config_info))
      {
        LbFileClose(handle);
        return;
      }
      LbFileClose(handle);
    }
    // If can't load, then use default config
    memcpy(&net_config_info, &default_net_config_info, sizeof(net_config_info));
    snprintf(net_config_info.net_player_name, sizeof(net_config_info.net_player_name), "%s", get_string(GUIStr_MnuNoName));
}

void net_write_config_file(void)
{
    // Try to load the config file
    char* fname = prepare_file_path(FGrp_Save, keeper_netconf_file);
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (handle)
    {
        LbFileWrite(handle, &net_config_info, sizeof(net_config_info));
        LbFileClose(handle);
    }
}

void frontnet_service_setup(void)
{
    net_number_of_services = 0;
    memset(net_service, 0, sizeof(net_service));
    // Create list of available services
    if (LbNetwork_EnumerateServices(enum_services_callback, NULL)) {
        ERRORLOG("LbNetwork_EnumerateServices() failed");
    }
    // Create skirmish option if it should be enabled
    if ((game.system_flags & GSF_AllowOnePlayer) != 0)
    {
        snprintf(net_service[net_number_of_services], NET_SERVICE_LEN, "%s", get_string(GUIStr_Net1Player));
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
    net_session_index_active_id = -1;
}

void frontnet_start_setup(void)
{
    frontnet_reset_ping_stabilization();
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
