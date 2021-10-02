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
#include "front_network.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_coroutine.h"

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
#include "frontend.h"
#include "hist_actions.h"
#include "player_data.h"
#include "net_game.h"
#include "net_sync.h"
#include "packets.h"
#include "config.h"
#include "config_strings.h"
#include "game_merge.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char *keeper_netconf_file = "fxconfig.net";

const struct ConfigInfo default_net_config_info = {
    -1, {4, 3, 4, 3, 4, 3, 4, 3, }, -1,
    "ATZ",
    "ATDT",
    "ATH",
    "ATS0=1",
    "",
    "",
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long modem_initialise_callback(void)
{
    if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
    {
      clear_key_pressed(KC_ESCAPE);
      return -7;
    }
    if (LbScreenLock() == Lb_SUCCESS)
    {
      draw_text_box(get_string(GUIStr_NetInitingModem));
      LbScreenUnlock();
    }
    LbScreenSwap();
    return 0;
}

long modem_connect_callback(void)
{
  if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
  {
    clear_key_pressed(KC_ESCAPE);
    return -7;
  }
  if (LbScreenLock() == Lb_SUCCESS)
  {
    draw_text_box(get_string(GUIStr_NetConnectnModem));
    LbScreenUnlock();
  }
  LbScreenSwap();
  return 0;
}

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

void gui_draw_tick_time()
{
    if ((game.flags_gui & GGUI_ShowTickTime) == 0)
    {
        return;
    }
    int val = tick_time;
    TbClockMSec now = LbTimerClock();

    static int max_time_val = 0;
    static int max_time_t = 0;
    if (now > max_time_t)
    {
        max_time_val = 0;
    }
    if (val > max_time_val)
    {
        max_time_val = val;
        max_time_t = now + 5000;
    }
    
    if (winfont != NULL)
    {
        LbTextSetFont(winfont);
    }
    char* text = buf_sprintf("now %02d", val);
    long width = 10 * (LbTextCharWidth('0') * units_per_pixel / 16);
    long height = LbTextLineHeight() * units_per_pixel / 16 + (LbTextLineHeight() * units_per_pixel / 16) / 2;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    long scr_x = MyScreenWidth - width - 16 * units_per_pixel / 16;
    long scr_y = 3 * 16 * units_per_pixel / 16;
    LbTextSetWindow(scr_x, scr_y, width, height);
    if (winfont != NULL)
    {
        draw_slab64k(scr_x, scr_y, units_per_pixel, width, height * 2);
    }
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    LbTextDrawResized(0, 0, tx_units_per_px, text);
    LbTextSetWindow(scr_x, scr_y + height, width, height);
    
    text = buf_sprintf("max %02d", max_time_val);
    LbTextDrawResized(0, 0, tx_units_per_px, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

static void draw_event_log()
{
    const int lines = 9;
    const int width = 500;
    const int font_size = 16;
    const int margin_y = 5;
    char left_buf[64], right_buf[32];
    int x = MyScreenWidth - width;
    int y = 100;
    int units_per_px = units_per_pixel;
    if ((game_flags2 & GF2_ShowEventLog) == 0)
    {
        return;
    }
    LbTextSetFont(winfont);
    int tx_units_per_px = (font_size * units_per_px) / LbTextLineHeight();
    long text_h = LbTextLineHeight() * tx_units_per_px / 16;
    int height = text_h * lines + 2 * margin_y * units_per_px/16;
  
    LbDrawBox(x, y, width, height, 0);

    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    LbTextSetWindow(x+4, y, width-8, height);

    sprintf(left_buf, "%6ld", game.play_gameturn);
    LbTextDrawResized(0, margin_y*units_per_px/16 + text_h * 0,
            tx_units_per_px, left_buf);
    for (int i = 0; i < lines-1; i++)
    {
        hist_get_string(i, left_buf, right_buf);
        if (left_buf[0] == 0) 
            break;
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        LbTextDrawResized(0, margin_y*units_per_px/16 + text_h * (i+1),
            tx_units_per_px, left_buf);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        LbTextDrawResized(0, margin_y*units_per_px/16 + text_h * (i+1),
            tx_units_per_px, right_buf);
    }
}

static int get_val_def(int x)
{
    return (game.play_gameturn - x) % 50;
}

static TbBool comment_fn_def(int i, char *left, char *right)
{
    switch (i)
    {
        case 0:
            strcpy(left, "turn");
            sprintf(right, "%6ld", game.play_gameturn);
            return true;
        case 1:
            strcpy(left, "mod");
            strcpy(right, "");
            return true;
        default:
            return false;
    }
}

static PlotFn get_val = &get_val_def;
static PlotCommentFn get_comment = &comment_fn_def;

void front_set_plot_fn(PlotFn data_fn, PlotCommentFn comment_fn)
{
    get_val = data_fn;
    get_comment = comment_fn;
}

static void draw_plot()
{
    const int width = 128 + 8;
    const int height = 100 + 8;
    const int font_size = 16;
    const int margin_y = 5;
    char left_buf[64], right_buf[32];
    int x = MyScreenWidth - width - 8;
    int y = 100;
    int units_per_px = units_per_pixel;
    if ((game_flags2 & GF2_ShowPlot) == 0)
    {
        return;
    }
    LbTextSetFont(winfont);
    int tx_units_per_px = (font_size * units_per_px) / LbTextLineHeight();
    long text_h = LbTextLineHeight() * tx_units_per_px / 16;

    LbDrawBox(x, y, width, height, 0);

    LbTextSetWindow(x+4, y, width-8, height);

    int min_val = 101;
    int max_val = 0;

    // green 164
    LbDrawHVLine(x + 4, y + height - 4 - 25, x + width -  4, y + height - 4 - 25, 166);
    int val;
    for (int i = 0; i < 64; i++)
    {
        TbPixel color = 200; // orange
        val = get_val(64 - 1 - i);
        if (val < min_val)
        {
            min_val = val;
        }
        if (val > max_val)
        {
            max_val = val;
        }
        LbDrawHVLine(x + 4 + 2 * i, y + height - 4 - val, x + 4 + 2 * i, y + height - 4, color);
        LbDrawHVLine(x + 4 + 2 * i + 1, y + height - 4 - val, x + 4 + 2 * i + 1, y + height - 4, color);
    }

    int text_row = 0;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    strcpy(left_buf, "min");
    sprintf(right_buf, "%3d", min_val);
    LbTextDrawResized(0, margin_y*units_per_px/16 + text_h * text_row ,
                      tx_units_per_px, left_buf);
    LbTextDrawResized(width/2, margin_y*units_per_px/16 + text_h * text_row,
                      tx_units_per_px, right_buf);
    text_row++;

    strcpy(left_buf, "max");
    sprintf(right_buf, "%3d", max_val);
    LbTextDrawResized(0, margin_y*units_per_px/16 + text_h * text_row ,
                      tx_units_per_px, left_buf);
    LbTextDrawResized(width/2, margin_y*units_per_px/16 + text_h * text_row,
                      tx_units_per_px, right_buf);
    text_row++;

    strcpy(left_buf, "last");
    sprintf(right_buf, "%3d", val);
    LbTextDrawResized(0, margin_y*units_per_px/16 + text_h * text_row ,
                      tx_units_per_px, left_buf);
    LbTextDrawResized(width/2, margin_y*units_per_px/16 + text_h * text_row,
                      tx_units_per_px, right_buf);

    text_row = 0;
    while ((get_comment != NULL) && get_comment(text_row, left_buf, right_buf))
    {
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        LbTextDrawResized(0, margin_y*units_per_px/16 + text_h * (text_row + 3),
                          tx_units_per_px, left_buf);
        LbTextDrawResized(width/2, margin_y*units_per_px/16 + text_h * (text_row + 3),
                          tx_units_per_px, right_buf);
        text_row++;
    }
}

void draw_out_of_sync_box(long a1, long a2, long box_width)
{
    static char old_data[32] = {0};

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
    {
        long ornate_width = 200 * units_per_px / 16;
        long ornate_height = 110 * units_per_px / 16;
        long x = box_width + (MyScreenWidth - box_width - ornate_width) / 2;
        long y = (MyScreenHeight - ornate_height) / 2;
        draw_ornate_slab64k(x, y, units_per_px, ornate_width, ornate_height);
        LbTextSetFont(winfont);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
        LbTextSetWindow(x, y, ornate_width, ornate_height);
        int tx_units_per_px = (22 * units_per_px) / LbTextLineHeight();
        long text_h = LbTextLineHeight() * tx_units_per_px / 16;
        long text_x = x + 100 * units_per_px / 16 - max_width;
        long text_y = 48 * units_per_px / 16;
        LbTextDrawResized(0, 40*units_per_px/16 - text_h, tx_units_per_px, get_string(GUIStr_NetResyncing));
        LbDrawBox(text_x, text_y, 2*min_width, 16*units_per_px/16, 133);

        int a, b;
        LbNetwork_GetResyncProgress(&a, &b);
        if (b == 0) b = 1;

        LbDrawBox(text_x, y + text_y, 2 * max_width * a / b, 16*units_per_px/16,
              player_flash_colours[0]);
              
        text_y += text_h + 16 * units_per_px / 16;
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;

        const char *desync_info = get_desync_info();
        LbTextDrawResized(10, text_y - text_h, tx_units_per_px, desync_info);

        if (strcmp(old_data, desync_info) != 0)
        {
            JUSTLOG("desync turn:%lu ui_turn:%lu %s", (unsigned long)game.play_gameturn, ui_turn, desync_info);
            strcpy(old_data, desync_info);
        }
    }
}

void gui_draw_network_state()
{
    if (((game.system_flags & GSF_NetGameNoSync) != 0)
      || ((game.system_flags & GSF_NetSeedNoSync) != 0))
    {
      draw_out_of_sync_box(0, 32*units_per_pixel/16, get_my_player()->engine_window_x);
    }
    gui_draw_tick_time();
    draw_event_log();
    draw_plot();
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

void __stdcall enum_players_callback(struct TbNetworkCallbackData *netcdat, void *a2)
{
    if (net_number_of_enum_players >= 4)
    {
        ERRORLOG("Too many players in enumeration");
        return;
    }
    strncpy(net_player[net_number_of_enum_players].name, netcdat->plyr_name, sizeof(struct TbNetworkPlayerName));
    net_number_of_enum_players++;
}

void __stdcall enum_sessions_callback(struct TbNetworkCallbackData *netcdat, void *ptr)
{
    if (net_number_of_sessions >= 32)
    {
        ERRORLOG("Too many sessions in enumeration");
        return;
    }
    if (net_service_index_selected == 0)
    {
        net_session[net_number_of_sessions] = (struct TbNetworkSessionNameEntry *)netcdat;
        net_number_of_sessions++;
    } else
    if (net_service_index_selected != 1)
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

void __stdcall enum_services_callback(struct TbNetworkCallbackData *netcdat, void *a2)
{
    if (net_number_of_services >= NET_SERVICES_COUNT)
    {
      ERRORLOG("Too many services in enumeration");
      return;
    }
    if (strcasecmp("SERIAL", netcdat->svc_name) == 0)
    {
      LbStringCopy(net_service[net_number_of_services], get_string(GUIStr_NetSerial), NET_MESSAGE_LEN);
      net_number_of_services++;
    } else
    if (strcasecmp("MODEM", netcdat->svc_name) == 0)
    {
        LbStringCopy(net_service[net_number_of_services], get_string(GUIStr_NetModem), NET_MESSAGE_LEN);
        net_number_of_services++;
    } else
    if (strcasecmp("IPX", netcdat->svc_name) == 0)
    {
        LbStringCopy(net_service[net_number_of_services], get_string(GUIStr_NetIpx), NET_MESSAGE_LEN);
        net_number_of_services++;
    } else
    if (strcasecmp("UDP", netcdat->svc_name) == 0)
    {
        LbStringCopy(net_service[net_number_of_services], "UDP", NET_MESSAGE_LEN);//TODO TRANSLATION put this in GUI strings
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
      LbMemorySet(net_player, 0, sizeof(net_player));
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

void frontnet_modem_update(void)
{
    if (net_comport_scroll_offset < 0)
    {
        net_comport_scroll_offset = 0;
    } else
    if (net_comport_scroll_offset > number_of_comports - 1)
    {
        net_comport_scroll_offset = number_of_comports - 1;
    }
    if (net_speed_scroll_offset < 0)
    {
        net_speed_scroll_offset = 0;
    } else
    if (net_speed_scroll_offset > number_of_speeds - 1)
    {
        net_speed_scroll_offset = number_of_speeds - 1;
    }
    if (number_of_speeds - 1 < net_speed_index_active) {
        net_speed_index_active = -1;
    }
    if (number_of_comports - 1 < net_comport_index_active) {
        net_comport_index_active = -1;
    }
}

void frontnet_serial_update(void)
{
    if (net_comport_scroll_offset < 0)
    {
        net_comport_scroll_offset = 0;
    } else
    if (net_comport_scroll_offset > number_of_comports - 1)
    {
        net_comport_scroll_offset = number_of_comports - 1;
    }
    if (net_speed_scroll_offset < 0)
    {
        net_speed_scroll_offset = 0;
    } else
    if (net_speed_scroll_offset > number_of_speeds - 1)
    {
        net_speed_scroll_offset = number_of_speeds - 1;
    }
    if (number_of_speeds - 1 < net_speed_index_active) {
        net_speed_index_active = -1;
    }
    if (number_of_comports - 1 < net_comport_index_active) {
        net_comport_index_active = -1;
    }
}

void frontnet_start_update(void)
{
    static TbClockMSec player_last_time = 0;
    SYNCDBG(18,"Starting");
    if (LbTimerClock() >= player_last_time+200)
    {
      net_number_of_enum_players = 0;
      LbMemorySet(net_player, 0, sizeof(net_player));
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
    } else
    if (net_message_scroll_offset > net_number_of_messages-1)
    {
      net_message_scroll_offset = net_number_of_messages-1;
    }
    process_frontend_packets();
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
    LbStringCopy(net_config_info.str_u2, get_string(GUIStr_MnuNoName), 20);
}

void net_write_config_file(void)
{
    // Try to load the config file
    char* fname = prepare_file_path(FGrp_Save, keeper_netconf_file);
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (handle != -1)
    {
        strupr(net_config_info.str_atz);
        strupr(net_config_info.str_ath);
        strupr(net_config_info.str_atdt);
        strupr(net_config_info.str_ats);
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
      strncpy(net_player_name, net_config_info.str_u2, sizeof(net_player_name));
      strcpy(tmp_net_player_name, net_config_info.str_u2);
    }
    net_session_index_active = -1;
    fe_computer_players = 2;
    lbInkey = 0;
    net_session_index_active_id = -1;
}

void frontnet_start_setup(void)
{
    frontend_alliances = -1;
    net_current_message_index = 0;
    net_current_message[0] = 0;
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

void frontnet_modem_setup(void)
{
    net_load_config_file();
    number_of_comports = 8;
    net_comport_index_active = net_config_info.numfield_0;
    net_speed_index_active = net_config_info.numfield_9;
    if (net_comport_index_active == -1)
      strcpy(tmp_net_irq, "4");
    else
      sprintf(tmp_net_irq, "%d", net_config_info.numfield_1[net_comport_index_active]);
    number_of_speeds = 8;
    strcpy(tmp_net_modem_init, net_config_info.str_atz);
    strcpy(tmp_net_modem_dial, net_config_info.str_atdt);
    strcpy(tmp_net_modem_hangup, net_config_info.str_ath);
    strcpy(tmp_net_modem_answer, net_config_info.str_ats);
    strcpy(tmp_net_phone_number, net_config_info.str_join);
}

void frontnet_serial_setup(void)
{
    net_load_config_file();
    number_of_comports = 8;
    net_comport_index_active = net_config_info.numfield_0;
    net_speed_index_active = net_config_info.numfield_9;
    if (net_comport_index_active == -1)
      strcpy(tmp_net_irq, "4");
    else
      sprintf(tmp_net_irq, "%d", net_config_info.numfield_1[net_comport_index_active]);
    number_of_speeds = 7;
}
/******************************************************************************/
