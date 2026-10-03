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
#include "kfx/renderer/RendererManager.h"
#include "front_network.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_enet.h"

#include "net_exchange_common.h"
#include "net_lobby.h"
#include "bflib_netsession.h"
#include "bflib_guibtns.h"
#include "bflib_keybrd.h"
#include "bflib_sound.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
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
#include "config.h"
#include "config_sounds.h"
#include "config_strings.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "net_matchmaking.h"
#include "net_lan.h"
#include "config_campaigns.h"
#include "post_inc.h"

#define SESSION_LIST_MOUSE_IDLE_TIMEOUT_MS 1000

#ifdef __cplusplus
extern "C" {
#endif

extern char autostart_multiplayer_campaign[80];
extern int autostart_multiplayer_level;
extern int autostart_multiplayer_users_expected;

/******************************************************************************/
const char *keeper_netconf_file = "fxconfig.net";

const struct ConfigInfo default_net_config_info = {
    "",
    "Player",
    "",
};

int fe_network_active;
int net_service_index_selected;
struct TbNetworkSessionNameEntry *net_session[SESSION_ENTRIES_COUNT];
long net_number_of_sessions;
long net_session_index_active;
struct TbNetworkPlayerName net_player[MAX_NET_USERS];
struct ConfigInfo net_config_info;
char net_service[16][NET_SERVICE_LEN];
char net_player_name[20];
char tmp_net_player_name[24];
static TbBool attempting_to_join_cancelled = false;
static int32_t previous_active_players = 0;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
static TbBool check_frontend_version_mismatch(void);

static TbBool try_starting_level_from_chat(const char *message, NetUserId user_id)
{
    const char *separator_pos = strchr(message, ':');
    if (separator_pos == NULL) {
        separator_pos = strchr(message, ' ');
    }
    if ((separator_pos == NULL) || (separator_pos == message)) {
        return false;
    }
    int32_t campaign_len = separator_pos - message;
    if (campaign_len >= 64) {
        return false;
    }
    const char *level_str = separator_pos + 1;
    while (*level_str == ' ') {
        level_str++;
    }
    LevelNumber level_num = -1;
    if (level_str[0] != '_') {
        if (!isdigit(level_str[0]) || (user_id != SERVER_ID)) {
            return false;
        }
        level_num = atoi(level_str);
        if (level_num <= 0) {
            return false;
        }
    }
    char campaign_filename[80];
    snprintf(campaign_filename, sizeof(campaign_filename), "%.*s", campaign_len, message);
    return frontnet_start_level(campaign_filename, level_num);
}

TbBool frontnet_start_level(const char *campaign_fname, LevelNumber lvnum)
{
    if (campaign_fname == NULL || campaign_fname[0] == '\0') {
        return false;
    }
    if (lvnum > 0 && check_frontend_version_mismatch()) {
        return false;
    }
    char campaign_file[DISKPATH_SIZE];
    uint8_t pack = prepare_campaign_file_name(campaign_fname, campaign_file, sizeof(campaign_file));
    if ((pack == CampgnT_Default) && ((lvnum <= 0) || is_campaign_in_list(campaign_file, &mp_mappacks_list))) {
        pack = CampgnT_MultiplayerMappack;
    }
    if (!change_campaign(pack, campaign_file)
     || (strcasecmp(campaign.fname, campaign_file) != 0)) {
        ERRORLOG("Unable to load campaign '%s' for level %d", campaign_fname, (int)lvnum);
        return false;
    }
    if (lvnum <= 0) {
        return true;
    }
    if (get_level_info(lvnum) == NULL) {
        ERRORLOG("Campaign '%s' does not contain level %d", campaign_fname, (int)lvnum);
        return false;
    }
    set_selected_level_number(lvnum);
    LbNetwork_EnableNewPlayers(0);
    fe_network_active = 1;
    frontend_set_state(FeSt_START_MPLEVEL);
    return true;
}

void process_frontend_chat_message(NetUserId user, const char *message)
{
    if (message[0] == '\0') {
        return;
    }
    char text[PLAYER_MP_MESSAGE_LEN];
    snprintf(text, sizeof(text), "%s", message);
    if (!try_starting_level_from_chat(text, user)) {
        add_message(user, text);
        play_non_3d_sample(snd_chat_message[user == netstate.my_id]);
    }
}

TbBool frontnet_service_selected(enum FrontendNetService service)
{
    return (net_service_index_selected == service);
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
  create_frontend_error_box(text);
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
    if (RendererBeginFrame())
    {
        long ornate_width = 200 * units_per_px / 16;
        long ornate_height = 100 * units_per_px / 16;
        long x = box_width + (MyScreenWidth - box_width - ornate_width) / 2;
        long y = (MyScreenHeight - ornate_height) / 2;
        draw_ornate_slab64k(x, y, units_per_px, ornate_width, ornate_height);
        LbTextSetFont(winfont);
        RendererSetDrawFlags(Lb_TEXT_HALIGN_CENTER);
        LbTextSetWindow(x, y, ornate_width, ornate_height);
        int tx_units_per_px = (22 * units_per_px) / LbTextLineHeight();
        long text_h = LbTextLineHeight() * tx_units_per_px / 16;
        long text_x = x + 100 * units_per_px / 16 - max_width;
        long text_y = y + 58 * units_per_px / 16;
        LbTextDrawResized(0, 50*units_per_px/16 - text_h, tx_units_per_px, get_string(GUIStr_NetResyncing));
        LbDrawBox(text_x, text_y, 2*max_width, 16*units_per_px/16, 0);
        LbDrawBox(text_x, text_y, 2*min_width, 16*units_per_px/16, 133);
        RendererEndFrame();
        RendererPresentFrame();
    }
}

void setup_alliances(void)
{
    for (int i = 0; i < MAX_NET_USERS; i++) {
        if (!player_exists(get_player(i))) {
            continue;
        }
        for (int k = i + 1; k < MAX_NET_USERS; k++) {
            if (!player_exists(get_player(k))) {
                continue;
            }
            if (frontend_is_player_allied(i, k)) {
                set_ally_with_player(i, k, true);
                set_ally_with_player(k, i, true);
            }
        }
    }
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

static void enum_players_callback(struct TbNetworkCallbackData *netcdat, void *a2)
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
        strcpy(&netcdat->svc_name[8],get_string(GUIStr_NetLan));
        net_number_of_sessions++;
    }
}

static int compare_lobbies(const void *left, const void *right)
{
    const struct TbNetworkSessionNameEntry *a = *(const struct TbNetworkSessionNameEntry *const *)left;
    const struct TbNetworkSessionNameEntry *b = *(const struct TbNetworkSessionNameEntry *const *)right;
    int incompatible = net_session_incompatible(a) - net_session_incompatible(b);
    if (incompatible) {
        return incompatible;
    }
    int phase = (a->phase == NetPhase_InGame) - (b->phase == NetPhase_InGame);
    if (phase) {
        return phase;
    }
    int a_full = a->roster_known && a->max_players && a->player_count >= a->max_players;
    int b_full = b->roster_known && b->max_players && b->player_count >= b->max_players;
    if (a_full != b_full) {
        return a_full - b_full;
    }
    if (!a_full && a->player_count != b->player_count) {
        return b->player_count - a->player_count;
    }
    if (a->created_at != b->created_at) {
        return (a->created_at < b->created_at) - (a->created_at > b->created_at);
    }
    int name = strcmp(a->text, b->text);
    if (name) {
        return name;
    }
    return strcmp(a->join_address, b->join_address);
}

static TbBool frontnet_session_list_in_use(void)
{
    static int32_t last_mouse_x = -1;
    static int32_t last_mouse_y = -1;
    static uint32_t last_mouse_move;
    int32_t mouse_x = GetMouseX();
    int32_t mouse_y = GetMouseY();
    uint32_t now = LbTimerClock();
    if (mouse_x != last_mouse_x || mouse_y != last_mouse_y || left_button_held) {
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        last_mouse_move = now;
    }
    return net_number_of_sessions > 0 && frontend_mouse_over_button >= 45 && frontend_mouse_over_button < 45 + frontend_sessions_menu_items_visible && now - last_mouse_move < SESSION_LIST_MOUSE_IDLE_TIMEOUT_MS;
}

void frontnet_session_update(void)
{
    if (frontnet_service_selected(FrontendNetSvc_LAN)) {
        lan_service();
    }
    if (frontnet_service_selected(FrontendNetSvc_Online)) {
        matchmaking_service();
    }
    if (frontnet_session_list_in_use()) {
        return;
    }
    char selected[SESSION_LOBBY_ID_MAX_LEN] = "";
    char selected_name[SESSION_NAME_MAX_LEN] = "";
    if (net_session_index_active >= 0 && net_session_index_active < net_number_of_sessions) {
        snprintf(selected, sizeof(selected), "%s", net_session[net_session_index_active]->join_address);
        snprintf(selected_name, sizeof(selected_name), "%s", net_session[net_session_index_active]->text);
    }
    net_number_of_sessions = 0;
    memset(net_session, 0, sizeof(net_session));
    LbNetwork_EnumerateSessions(enum_sessions_callback, NULL);
    if (frontnet_service_selected(FrontendNetSvc_LAN)) {
        lan_refresh_sessions();
        for (int i = 0; i < lan_session_count && net_number_of_sessions < SESSION_ENTRIES_COUNT; i++) {
            net_session[net_number_of_sessions++] = &lan_sessions[i];
        }
    }
    if (frontnet_service_selected(FrontendNetSvc_Online)) {
        matchmaking_refresh_sessions();
        for (int i = 0; i < matchmaking_session_count && net_number_of_sessions < SESSION_ENTRIES_COUNT; i++) {
            net_session[net_number_of_sessions++] = &matchmaking_sessions[i];
        }
    }
    qsort(net_session, net_number_of_sessions, sizeof(net_session[0]), compare_lobbies);
    net_session_index_active = -1;
    net_session_index_active_id = -1;
    for (int i = 0; i < net_number_of_sessions; i++) {
        if (selected_name[0] && strcmp(selected_name, net_session[i]->text) == 0 && (!selected[0] || strcmp(selected, net_session[i]->join_address) == 0)) {
            net_session_index_active = i;
            net_session_index_active_id = net_session[i]->id;
            break;
        }
    }
    net_session_scroll_offset = max(0, min(net_session_scroll_offset, net_number_of_sessions - frontend_sessions_menu_items_visible));
}

static TbBool check_frontend_version_mismatch(void)
{
  int32_t active_players = 0;
  const struct NetUser *host_user = &netstate.users[SERVER_ID];
  NetUserId remote_id = -1;
  TbBool start_requested = false;

  for (NetUserId i = 0; i < MAX_NET_USERS; i++) {
    if (!network_user_active(i)) {
      continue;
    }
    active_players++;
    unsigned char action = screen_packet_action(&net_screen_packet[i]);
    if (action == NetAct_OpenLandView || action == NetAct_HostStartLevel) {
      start_requested = true;
    }
    if (i != SERVER_ID && !net_versions_match(&host_user->version, &netstate.users[i].version) && (remote_id == -1 || i == my_player_number)) {
      remote_id = i;
    }
  }
  TbBool player_joined = active_players > previous_active_players;
  if (active_players < previous_active_players && snd_lobby_player_leave_count > 0) {
    play_non_3d_sample(snd_lobby_player_leave + SOUND_RANDOM(snd_lobby_player_leave_count));
  }
  previous_active_players = active_players;
  if (remote_id == -1 || (!player_joined && !start_requested)) {
    return remote_id != -1;
  }
  const struct NetUser *remote_user = &netstate.users[remote_id];
  char text[MESSAGE_TEXT_LEN];
  snprintf(text, sizeof(text), "%s\n%s: %d.%d.%d.%d\n%s: %d.%d.%d.%d",
      get_string(GUIStr_VersionMismatch),
      network_user_name(SERVER_ID), (int)host_user->version.major, (int)host_user->version.minor, (int)host_user->version.release, (int)host_user->version.build,
      network_user_name(remote_id), (int)remote_user->version.major, (int)remote_user->version.minor, (int)remote_user->version.release, (int)remote_user->version.build);
  create_frontend_error_box(text);
  return true;
}

static void process_frontend_packets(void)
{
  if (!frontnet_matchmaking_update()) {
    return;
  }
  NetUserId i;
  for (i = 0; i < MAX_NET_USERS; i++) {
    net_screen_packet[i].networkstatus_flags &= ~NetStat_PlayerConnected;
  }
  struct ScreenPacket* nspckt = &net_screen_packet[my_player_number];
  nspckt->networkstatus_flags |= NetStat_PlayerConnected;
  nspckt->frontend_alliances = frontend_alliances;
  nspckt->networkstatus_flags &= ~NetStat_ComputerPlayersMask;
  nspckt->networkstatus_flags |= (fe_computer_players << NetStat_ComputerPlayersShift) & NetStat_ComputerPlayersMask;
  if (LbNetwork_ExchangeFrontend(nspckt, &net_screen_packet, sizeof(struct ScreenPacket))) {
      ERRORLOG("LbNetwork_Exchange failed");
      net_service_index_selected = -1;
  }
  if (netstate.my_id != SERVER_ID && netstate.users[SERVER_ID].progress == USER_UNUSED) {
    LbNetwork_Stop();
    if (!setup_network_service(net_service_index_selected)) {
      frontend_set_state(FeSt_MAIN_MENU);
    }
    return;
  }
#if DEBUG_NETWORK_PACKETS
  write_debug_screenpackets();
#endif
  TbBool version_mismatch_found = check_frontend_version_mismatch();
  for (i = 0; i < MAX_NET_USERS; i++) {
    nspckt = &net_screen_packet[i];
    if ((nspckt->networkstatus_flags & NetStat_PlayerConnected) != 0) {
      if (frontend_alliances == -1) {
        if (nspckt->frontend_alliances != -1) {
          frontend_alliances = nspckt->frontend_alliances;
        }
      }
        switch (screen_packet_action(nspckt)) {
        case NetAct_HostStartLevel:
            if (version_mismatch_found) {
                break;
            }
            fe_network_active = 1;
            if (game_flags2 & GF2_Connect) {
                frontend_set_state(FeSt_START_MPLEVEL);
            } else {
                frontend_set_state(FeSt_NETLAND_VIEW);
            }
            break;
        case NetAct_OpenLandView:
            if (version_mismatch_found) {
                break;
            }
            fe_network_active = 1;
            frontend_set_state(FeSt_NETLAND_VIEW);
            break;
        case NetAct_SetAlliance:
            frontend_set_alliance(nspckt->action_par1, nspckt->action_par2);
            break;
        case NetAct_SetComputerPlayers:
            fe_computer_players = nspckt->action_par1;
            break;
        default:
            break;
        }
      if (fe_computer_players == 2) {
        int32_t k = (nspckt->networkstatus_flags & NetStat_ComputerPlayersMask) >> NetStat_ComputerPlayersShift;
        if (k != 2) {
          fe_computer_players = k;
        }
      }
    }
    screen_packet_set_action(nspckt, NetAct_None);
  }
  if (frontend_alliances == -1) {
    frontend_alliances = 0;
  }
  for (i = 0; i < MAX_NET_USERS; i++) {
    if (!network_user_active(i)) {
      const int32_t alliances_to_clear = alliance_grid[i][0] | alliance_grid[i][1] | alliance_grid[i][2] | alliance_grid[i][3];
      frontend_alliances = frontend_alliances & ~alliances_to_clear;
    }
  }
}

TbBool frontnet_matchmaking_update(void)
{
    if (network_is_host() && frontnet_service_selected(FrontendNetSvc_Online) && enet_matchmaking_host_update() < 0) {
        frontnet_return_to_session_menu(NULL);
        create_frontend_error_box(get_string(GUIStr_NetLobbyConnectionLost));
        return false;
    }
    return true;
}

void frontnet_send_campaign_change_message(const char* campaign_fname)
{
    char base_name[64];
    if ((campaign_fname == NULL) || (campaign_fname[0] == '\0')) {
        return;
    }
    strncpy(base_name, campaign_fname, sizeof(base_name)-1);
    base_name[sizeof(base_name)-1] = '\0';
    char* dot = strrchr(base_name, '.');
    if (dot != NULL) {
        *dot = '\0';
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "%s:_", base_name);
    send_network_chat_message(netstate.my_id, msg);
}

void handle_autostart_multiplayer_messaging(void)
{
    static int previous_enum_players = 0;
    TbBool player_joined = (net_number_of_enum_players > previous_enum_players);
    previous_enum_players = net_number_of_enum_players;

    if (net_number_of_enum_players < autostart_multiplayer_users_expected) {
        return;
    }

    if (player_joined && network_is_host()) {
        frontnet_send_campaign_change_message(campaign.fname);
    }
    if (!network_is_host() || get_selected_level_number() > SINGLEPLAYER_NOTSTARTED) {
        return;
    }
    if (autostart_multiplayer_campaign[0] == '\0' && autostart_multiplayer_level <= 0) {
        return;
    }
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


}

void display_attempting_to_join_message(int remaining_s)
{
    char msg[128];
    if (remaining_s >= 0)
        snprintf(msg, sizeof(msg), "%s (%ds)", get_string(GUIStr_NetAttemptingToJoin), remaining_s);
    else
        snprintf(msg, sizeof(msg), "%s", get_string(GUIStr_NetAttemptingToJoin));
    frontend_draw();
    if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE)) {
        clear_key_pressed(KC_ESCAPE);
        attempting_to_join_cancelled = true;
    }
    if (RendererBeginFrame()) {
        draw_text_box(msg);
        RendererEndFrame();
    }
    RendererPresentFrame();
}

void reset_attempting_to_join_cancel(void)
{
    attempting_to_join_cancelled = false;
}

TbBool attempting_to_join_cancel_requested(void)
{
    return attempting_to_join_cancelled;
}

void net_load_config_file(void)
{
    // Try to load the config file
    char* fname = prepare_file_path(FGrp_Save, keeper_netconf_file);
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (handle) {
        memset(&net_config_info, 0, sizeof(net_config_info));
        int32_t read_size = LbFileRead(handle, &net_config_info, sizeof(net_config_info));
        LbFileClose(handle);
        if (read_size == sizeof(net_config_info) || read_size == sizeof(net_config_info) - sizeof(net_config_info.net_lobby_name)) {
            net_config_info.net_player_name[sizeof(net_config_info.net_player_name) - 1] = '\0';
            net_config_info.net_lobby_name[sizeof(net_config_info.net_lobby_name) - 1] = '\0';
            return;
        }
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
    snprintf(net_service[net_number_of_services++], NET_SERVICE_LEN, "%s", get_string(GUIStr_NetOnline));
    snprintf(net_service[net_number_of_services++], NET_SERVICE_LEN, "%s", get_string(GUIStr_NetLan));
    // Create skirmish option if it should be enabled
    if ((local_system_flags & GSF_AllowOnePlayer) != 0)
    {
        snprintf(net_service[net_number_of_services], NET_SERVICE_LEN, "%s", get_string(GUIStr_NetServiceSkirmish));
        net_number_of_services++;
    }
    net_load_config_file();
}

void frontnet_session_setup(void)
{
    net_number_of_sessions = 0;
    if (net_player_name[0] == '\0') {
        snprintf(net_player_name, sizeof(net_player_name), "%s", net_config_info.net_player_name);
        strcpy(tmp_net_player_name, net_config_info.net_player_name);
    }
    set_default_mp_mappack();
    net_session_index_active = -1;
    fe_computer_players = 2;
    lbInkey = 0;
    net_session_index_active_id = -1;
    if (frontnet_service_selected(FrontendNetSvc_Online)) {
        matchmaking_connect_async();
    }
}

void frontnet_start_setup(void)
{
    set_selected_level_number(SINGLEPLAYER_NOTSTARTED);
    previous_active_players = 0;
    memset(net_screen_packet, 0, sizeof(net_screen_packet));
    frontend_alliances = -1;
    net_number_of_messages = 0;
    net_player_scroll_offset = 0;
    net_message_scroll_offset = 0;
    //net_old_number_of_players = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        player->mp_message_text[0] = '\0';
    }
}

/******************************************************************************/
