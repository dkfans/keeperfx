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
#include "engine_redraw.h"

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
#include "kfx/save/SaveManager.h"
#include <stddef.h>
#include <SDL3/SDL.h>
#include "post_inc.h"

#include <string.h>
#define SESSION_LIST_MOUSE_IDLE_TIMEOUT_MS 1000
#define LOBBY_SCREEN_PACKET_RESEND_INTERVAL_MS 100

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
    MAX_NET_USERS,
    1,
    1,
    0,
    0,
    0,
};

int fe_network_active;
int net_service_index_selected;
struct TbNetworkSessionNameEntry *net_session[SESSION_ENTRIES_COUNT];
long net_number_of_sessions;
long net_session_index_active;
struct TbNetworkPlayerName net_player[MAX_NET_USERS];
struct ConfigInfo net_config_info;
struct ConfigInfo net_lobby_settings;
char net_service[16][NET_SERVICE_LEN];
char net_player_name[20];
char tmp_net_player_name[24];
int8_t net_player_colour[MAX_NET_USERS];
int8_t net_player_team[MAX_NET_USERS];
int32_t net_lobby_max_players;
static TbBool attempting_to_join_cancelled = false;
static int32_t previous_active_players = 0;
static struct ScreenPacket last_sent_screen_packet;
static TbClockMSec last_sent_screen_packet_time;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
#define MINIMUM_RANDOM_TEAMS 2

struct NetPlayerSetting {
    int8_t *values;
    int8_t values_count;
    int8_t requested;
};

static struct NetPlayerSetting net_player_settings[NetPlayerSetting_Count] = {
    [NetPlayerSetting_Colour] = {net_player_colour, NET_PLAYER_COLOUR_RANDOM + 1, -1},
    [NetPlayerSetting_Team] = {net_player_team, NET_TEAM_RANDOM + 1, -1},
};

static TbBool check_frontend_version_mismatch(void);

static NetUserId net_user_with_value(const int8_t values[MAX_NET_USERS], int8_t value)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (values[user] == value) {
            return user;
        }
    }
    return -1;
}

static int8_t first_unused_value(const int8_t values[MAX_NET_USERS])
{
    for (int8_t value = 0; value < MAX_NET_USERS; value++) {
        if (net_user_with_value(values, value) < 0) {
            return value;
        }
    }
    return MAX_NET_USERS;
}

TbBool net_player_colours_conflict(void)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        PlayerNumber colour = net_player_colour[user];
        if (colour >= 0 && colour < NET_PLAYER_COLOUR_RANDOM && net_user_with_value(net_player_colour, colour) != user) {
            return true;
        }
    }
    return false;
}

void frontnet_cycle_player_setting(enum NetPlayerSettingKind kind, NetUserId user, TbBool backwards)
{
    struct NetPlayerSetting *setting = &net_player_settings[kind];
    int8_t step = 1;
    if (backwards) {
        step = setting->values_count - 1;
    }
    setting->values[user] = (setting->values[user] + step) % setting->values_count;
    if (network_is_host()) {
        return;
    }
    setting->requested = setting->values[user];
    struct ScreenPacket *nspck = &net_screen_packet[my_player_number];
    screen_packet_set_action(nspck, NetAct_SetPlayerSetting);
    nspck->action_par1 = setting->requested;
    nspck->action_par2 = kind;
}

static void shuffle_values(int8_t values[MAX_NET_USERS], int32_t count)
{
    for (int32_t i = count - 1; i > 0; i--) {
        int32_t k = SDL_rand(i + 1);
        int8_t swapped = values[i];
        values[i] = values[k];
        values[k] = swapped;
    }
}

static char alliances_from_teams(const int8_t teams[MAX_NET_USERS])
{
    char alliances = 0;
    for (NetUserId user1 = 0; user1 < MAX_NET_USERS; user1++) {
        for (NetUserId user2 = user1 + 1; user2 < MAX_NET_USERS; user2++) {
            if (teams[user1] >= 0 && teams[user1] < NET_TEAM_RANDOM && teams[user1] == teams[user2]) {
                alliances |= alliance_grid[user1][user2];
            }
        }
    }
    return alliances;
}

static void resolve_random_teams(int8_t teams[MAX_NET_USERS])
{
    int8_t team_sizes[NET_TEAM_COUNT];
    int8_t random_slots[MAX_NET_USERS];
    int8_t candidate_teams[MAX_NET_USERS];
    int32_t random_count = 0;
    int32_t candidates_count = 0;
    memset(team_sizes, 0, sizeof(team_sizes));
    for (PlayerNumber slot = 0; slot < MAX_NET_USERS; slot++) {
        if (teams[slot] == NET_TEAM_RANDOM) {
            random_slots[random_count] = slot;
            random_count++;
        } else if (teams[slot] >= 0) {
            if (team_sizes[teams[slot]] == 0) {
                candidate_teams[candidates_count] = teams[slot];
                candidates_count++;
            }
            team_sizes[teams[slot]]++;
        }
    }
    shuffle_values(random_slots, random_count);
    shuffle_values(candidate_teams, candidates_count);
    for (int32_t i = 0; i < random_count; i++) {
        if (candidates_count < MINIMUM_RANDOM_TEAMS) {
            candidate_teams[candidates_count] = first_unused_value(teams);
            candidates_count++;
        }
        int8_t smallest = candidate_teams[0];
        for (int32_t k = 1; k < candidates_count; k++) {
            if (team_sizes[candidate_teams[k]] < team_sizes[smallest]) {
                smallest = candidate_teams[k];
            }
        }
        teams[random_slots[i]] = smallest;
        team_sizes[smallest]++;
    }
}

void net_resolve_player_colours(PlayerNumber colours_count, int8_t player_colours[MAX_NET_USERS], char *alliances)
{
    PlayerNumber free_colours[MAX_NET_USERS];
    PlayerNumber free_count = 0;
    for (PlayerNumber colour = 0; colour < colours_count; colour++) {
        if (net_user_with_value(net_player_colour, colour) < 0) {
            free_colours[free_count] = colour;
            free_count++;
        }
    }
    shuffle_values(free_colours, free_count);
    PlayerNumber next_free = 0;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        player_colours[user] = net_player_colour[user];
        if (player_colours[user] < 0 || (player_colours[user] < colours_count && net_user_with_value(net_player_colour, player_colours[user]) == user)) {
            continue;
        }
        player_colours[user] = -1;
        if (next_free < free_count) {
            player_colours[user] = free_colours[next_free];
            next_free++;
        }
    }
    for (NetUserId user = 0; user < MAX_NET_USERS && next_free < free_count; user++) {
        if (net_lobby_computer_player(user)) {
            player_colours[user] = free_colours[next_free];
            next_free++;
        }
    }
    int8_t slot_teams[MAX_NET_USERS];
    memset(slot_teams, -1, sizeof(slot_teams));
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (player_colours[user] >= 0) {
            slot_teams[player_colours[user]] = net_player_team[user];
        }
    }
    resolve_random_teams(slot_teams);
    *alliances = alliances_from_teams(slot_teams);
}

TbBool net_lobby_computer_player(NetUserId user)
{
    return fe_computer_players == 1 && user < net_lobby_max_players && net_player_colour[user] < 0;
}

static void clear_teamless_users(void)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (net_player_colour[user] < 0 && !net_lobby_computer_player(user)) {
            net_player_team[user] = -1;
        }
    }
}

static void host_update_players(void)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (!network_user_active(user)) {
            net_player_colour[user] = -1;
        }
    }
    clear_teamless_users();
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (network_user_active(user) && net_player_colour[user] < 0) {
            net_player_colour[user] = first_unused_value(net_player_colour);
        }
        if ((net_player_colour[user] >= 0 || net_lobby_computer_player(user)) && net_player_team[user] < 0) {
            net_player_team[user] = first_unused_value(net_player_team);
        }
    }
}

static TbBool try_starting_level_from_chat(const char *message, NetUserId user_id)
{
    if (user_id != SERVER_ID) {
        return false;
    }
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
        if (!isdigit(level_str[0])) {
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
    copy_chat_message(text, message);
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
    keeper_screen_redraw();
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
        TbBool same_address = selected[0] && strcmp(selected, net_session[i]->join_address) == 0;
        TbBool same_name = !selected[0] && selected_name[0] && strcmp(selected_name, net_session[i]->text) == 0;
        if (same_address || same_name) {
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

static void process_frontend_packet(NetUserId user)
{
  struct ScreenPacket* nspckt = &net_screen_packet[user];
  if ((frontend_alliances == -1) && (nspckt->frontend_alliances != -1)) {
    frontend_alliances = nspckt->frontend_alliances;
  }
  switch (screen_packet_action(nspckt)) {
  case NetAct_HostStartLevel:
      if (user != SERVER_ID || check_frontend_version_mismatch()) {
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
      if (user != SERVER_ID || check_frontend_version_mismatch()) {
          break;
      }
      fe_network_active = 1;
      frontend_set_state(FeSt_NETLAND_VIEW);
      break;
  case NetAct_SetComputerPlayers:
      if (user == SERVER_ID) {
          fe_computer_players = nspckt->action_par1;
      }
      break;
  case NetAct_SetPlayerSetting:
      if (network_is_host() && nspckt->action_par2 < NetPlayerSetting_Count) {
          struct NetPlayerSetting *setting = &net_player_settings[nspckt->action_par2];
          if (nspckt->action_par1 >= 0 && nspckt->action_par1 < setting->values_count) {
              setting->values[user] = nspckt->action_par1;
          }
      }
      break;
  default:
      break;
  }
  if (user == SERVER_ID && fe_computer_players == 2) {
    int32_t k = (nspckt->networkstatus_flags & NetStat_ComputerPlayersMask) >> NetStat_ComputerPlayersShift;
    if (k != 2) {
      fe_computer_players = k;
    }
  }
  screen_packet_set_action(nspckt, NetAct_None);
}

static void process_frontend_packets(void)
{
  if (!frontnet_matchmaking_update()) {
    return;
  }
  struct ScreenPacket* nspckt = &net_screen_packet[my_player_number];
  nspckt->networkstatus_flags |= NetStat_PlayerConnected;
  nspckt->frontend_alliances = frontend_alliances;
  nspckt->networkstatus_flags &= ~NetStat_ComputerPlayersMask;
  nspckt->networkstatus_flags |= (fe_computer_players << NetStat_ComputerPlayersShift) & NetStat_ComputerPlayersMask;
  if (network_is_host()) {
    for (int i = 0; i < NetPlayerSetting_Count; i++) {
      memcpy(nspckt->player_settings[i], net_player_settings[i].values, MAX_NET_USERS);
    }
    nspckt->max_players = netstate.max_users;
    net_lobby_max_players = netstate.max_users;
  }
  TbClockMSec now = LbTimerClock();
  TbBool send_frame = (screen_packet_action(nspckt) != NetAct_None)
    || (memcmp(nspckt, &last_sent_screen_packet, sizeof(*nspckt)) != 0)
    || (now - last_sent_screen_packet_time >= LOBBY_SCREEN_PACKET_RESEND_INTERVAL_MS);
  if (send_frame) {
    last_sent_screen_packet = *nspckt;
    screen_packet_set_action(&last_sent_screen_packet, NetAct_None);
    last_sent_screen_packet_time = now;
  }
  if (LbNetwork_ExchangeLobby(nspckt, &net_screen_packet, sizeof(struct ScreenPacket), send_frame, process_frontend_packet)) {
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
  process_frontend_packet(my_player_number);
  check_frontend_version_mismatch();
  if (network_is_host()) {
    host_update_players();
  } else {
    const struct ScreenPacket *host_packet = &net_screen_packet[SERVER_ID];
    for (int i = 0; i < NetPlayerSetting_Count; i++) {
      struct NetPlayerSetting *setting = &net_player_settings[i];
      for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        setting->values[user] = host_packet->player_settings[i][user];
        if (setting->values[user] >= setting->values_count) {
          setting->values[user] = -1;
        }
      }
      if (setting->values[my_player_number] == setting->requested) {
        setting->requested = -1;
      } else if (setting->requested >= 0) {
        setting->values[my_player_number] = setting->requested;
      }
    }
    if (host_packet->max_players >= MIN_NET_USERS && host_packet->max_players <= MAX_NET_USERS) {
      net_lobby_max_players = host_packet->max_players;
    }
    clear_teamless_users();
  }
  if ((frontend_alliances != -1) || network_is_host() || is_connected_screen_packet(&net_screen_packet[SERVER_ID])) {
    frontend_alliances = alliances_from_teams(net_player_team);
  }
  for (NetUserId i = 0; i < MAX_NET_USERS; i++) {
    if (!network_user_active(i)) {
      net_screen_packet[i].networkstatus_flags &= ~NetStat_PlayerConnected;
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
    const char* camp = "keeporig";
    int level = 1;
    if (autostart_multiplayer_campaign[0]) {
        camp = autostart_multiplayer_campaign;
    }
    if (autostart_multiplayer_level > 0) {
        level = autostart_multiplayer_level;
    }
    snprintf(local_state.mp_message_text, PLAYER_MP_MESSAGE_LEN, "%s:%d", camp, level);
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

/** Makes a config read from a file safe to use: strings terminated, the player limit in range, flags 0 or 1. */
static void net_config_sanitise(void)
{
    net_config_info.net_player_name[sizeof(net_config_info.net_player_name) - 1] = '\0';
    net_config_info.net_lobby_name[sizeof(net_config_info.net_lobby_name) - 1] = '\0';
    if (net_config_info.max_players < MIN_NET_USERS || net_config_info.max_players > MAX_NET_USERS)
        net_config_info.max_players = MAX_NET_USERS;
    net_config_info.spectators_enabled = net_config_info.spectators_enabled != 0;
    net_config_info.spectator_chat = net_config_info.spectator_chat != 0;
    net_config_info.computer_players = net_config_info.computer_players != 0;
    net_config_info.cheats_enabled = net_config_info.cheats_enabled != 0;
    net_config_info.computer_assistant = net_config_info.computer_assistant != 0;
}

void net_load_config_file(void)
{
    // Try to load the config file
    char* fname = prepare_file_path(FGrp_Save, keeper_netconf_file);
    struct SaveError err;
    if (SaveManager_ReadNetConfig(fname, &net_config_info, &err) == SVR_Ok)
    {
        net_config_sanitise();
        return;
    }
    // If can't load, then use default config
    memcpy(&net_config_info, &default_net_config_info, sizeof(net_config_info));
    snprintf(net_config_info.net_player_name, sizeof(net_config_info.net_player_name), "%s", get_string(GUIStr_MnuNoName));
}

void net_write_config_file(void)
{
    char* fname = prepare_file_path(FGrp_Save, keeper_netconf_file);
    SaveManager_WriteNetConfig(fname, &net_config_info);
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
    for (int i = 0; i < NetPlayerSetting_Count; i++) {
        net_player_settings[i].requested = -1;
        memset(net_player_settings[i].values, -1, MAX_NET_USERS);
    }
    memset(net_screen_packet, 0, sizeof(net_screen_packet));
    last_sent_screen_packet_time = 0;
    net_lobby_max_players = MAX_NET_USERS;
    if (network_is_host()) {
        net_lobby_max_players = netstate.max_users;
    } else if (net_session_index_active >= 0 && net_session_index_active < net_number_of_sessions && net_session[net_session_index_active]->max_players >= MIN_NET_USERS) {
        net_lobby_max_players = min(net_session[net_session_index_active]->max_players, MAX_NET_USERS);
    }
    frontend_alliances = -1;
    net_number_of_messages = 0;
    net_player_scroll_offset = 0;
    net_lobby_mappack_scroll_offset = 0;
    net_lobby_mappack_list_open = false;
    net_message_scroll_offset = 0;
    //net_old_number_of_players = 0;
    local_state.mp_message_text[0] = '\0';
}

/******************************************************************************/
