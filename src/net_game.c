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
#include "net_matchmaking.h"
#include "net_game.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_coroutine.h"
#include "bflib_datetm.h"
#include "bflib_enet.h"
#include "net_exchange_common.h"
#include "net_lobby.h"
#include "net_main.h"
#include "net_resync.h"

#include "player_data.h"
#include "front_landview.h"
#include "player_utils.h"
#include "light_data.h"
#include "packets.h"
#include "frontend.h"
#include "front_network.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "custom_sprites.h"
#include "dungeon_data.h"
#include "game_legacy.h"
#include "gui_msgs.h"
#include "map_events.h"
#include "net_exchange_gameplay.h"
#include "net_input_lag.h"
#include "net_checksums.h"
#include "net_spectator.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbNetworkUserInfo net_user_info[MAX_NET_USERS];
extern int32_t multiplayer_speed_adjustment_ns;
/******************************************************************************/

#pragma pack(1)
struct StartupSyncPacket {
    uint8_t startup_sync_packet_valid;
    TbBigChecksum map_checksums[NETWORK_STARTUP_MAP_FILE_COUNT];
    TbBigChecksum required_sprite_zip_checksums[REQUIRED_SPRITE_ZIP_COUNT];
    UserPreferences user_prefs;
    uint8_t initial_input_lag_turns;
    uint32_t initial_action_seed;
    // TODO: also record alliance matrix.
};
#pragma pack()

short setup_network_service(enum FrontendNetService service)
{
  struct ServiceInitData *init_data = NULL;
  SYNCMSG("Initializing 4-players type %d network", service);
  memset(net_user_info, 0, sizeof(net_user_info));
  network_lobby_ping = 0;
  if (service != FrontendNetSvc_Online && service != FrontendNetSvc_LAN) {
    process_network_error(-800);
    return 0;
  }
  if ( LbNetwork_Init(NS_ENET_UDP, MAX_NET_USERS, &net_user_info[0], init_data) )
  {
    process_network_error(-800);
    return 0;
  }
  net_service_index_selected = service;
  if (service == FrontendNetSvc_LAN) {
    frontend_button_info[11].capstr_idx = GUIStr_MnuLanLobby;
    frontend_button_info[12].capstr_idx = GUIStr_MnuLanLobbies;
  } else {
    frontend_button_info[11].capstr_idx = GUIStr_MnuOnlineLobby;
    frontend_button_info[12].capstr_idx = GUIStr_MnuOnlineLobbies;
  }
  frontend_set_state(FeSt_NET_SESSION);
  return 1;
}

int setup_old_network_service(void)
{
    return setup_network_service(net_service_index_selected);
}

TbBool network_is_host(void)
{
    return netstate.my_id == SERVER_ID;
}

// user->player map received with the spectator bootstrap, applied once the level is set up
static PlayerNumber spectator_user_player_number[MAX_NET_USERS];

PlayerNumber get_net_user_player_number(NetUserId user)
{
    if ((user < 0) || (user >= MAX_NET_USERS)) {
        return PLAYER_NONE;
    }
    return game.user_states[user].player_id;
}

// user exists to the game layer (even if their connection has dropped,
// the game needs to hear about it through the proper channel first to avoid
// nondeterminism.)
TbBool user_present(NetUserId user)
{
    return get_net_user_player_number(user) >= 0;
}

void set_net_user_player_number(NetUserId user, PlayerNumber plyr_idx)
{
    if ((user < 0) || (user >= MAX_NET_USERS)) {
        return;
    }
    game.user_states[user].player_id = plyr_idx;
}

static const unsigned char user_preference_flags[UPref_Count] = {
    [UPref_FrontView]             = 0,
    [UPref_Wibble]                = UPF_ApplyOnLoad,
    [UPref_Cheats]                = UPF_ApplyOnLoad | UPF_ApplyOnTakeover,
    [UPref_SkipHeartZoom]         = UPF_ApplyOnLoad,
    [UPref_StartingHighlightMode] = 0,
    [UPref_StartingIsometricTilt] = UPF_ApplyOnLoad,
    [UPref_MaxZoomIso]            = UPF_ApplyOnLoad,
    [UPref_MaxZoomFrontview]      = UPF_ApplyOnLoad,
    [UPref_Censorship]            = UPF_ApplyOnLoad | UPF_ApplyOnTakeover,
    [UPref_WallHeight]            = UPF_ApplyOnLoad,
    [UPref_MapFade]               = UPF_ApplyOnLoad | UPF_ApplyOnTakeover,
    [UPref_StartingIsometricZoom] = 0,
    [UPref_StartingFrontviewZoom] = 0,
    [UPref_StartingTendencies]    = 0,
    [UPref_MinimapZoom]           = 0,
};

void build_local_user_preferences(UserPreferences prefs)
{
    memset(prefs, 0, sizeof(UserPreferences));
    TbBool front_view = false;
    TbBool wibble = true;
    rotate_mode_to_dungeon_view(settings.video_rotate_mode, &front_view, &wibble);
    prefs[UPref_FrontView] = front_view;
    prefs[UPref_Wibble] = wibble;
    prefs[UPref_Cheats] = start_params.easter_egg;
    prefs[UPref_SkipHeartZoom] = get_skip_heart_zoom_feature();
    prefs[UPref_StartingHighlightMode] = get_starting_highlight_mode();
    prefs[UPref_StartingIsometricTilt] = settings.isometric_tilt;
    prefs[UPref_MaxZoomIso] = zoom_distance_setting;
    prefs[UPref_MaxZoomFrontview] = frontview_zoom_distance_setting;
    prefs[UPref_Censorship] = local_censorship_enabled();
    prefs[UPref_WallHeight] = settings.video_cluedo_mode;
    prefs[UPref_MapFade] = min(get_parchment_map_fade_turns(), PARCHMENT_MAP_FADE_MAX_TURNS);
    prefs[UPref_StartingIsometricZoom] = settings.isometric_view_zoom_level;
    prefs[UPref_StartingFrontviewZoom] = settings.frontview_zoom_level;
    if (IMPRISON_BUTTON_DEFAULT)
        prefs[UPref_StartingTendencies] |= CrTend_Imprison;
    if (FLEE_BUTTON_DEFAULT)
        prefs[UPref_StartingTendencies] |= CrTend_Flee;
    prefs[UPref_MinimapZoom] = settings.minimap_zoom;
}

static void apply_user_starting_preferences(struct UserState *ustate, unsigned char flags)
{
    const uint32_t *prefs = ustate->prefs;
    if (flags != UPF_ApplyOnTakeover)
        ustate->dungeon_camera.pitch = clamp((int32_t)prefs[UPref_StartingIsometricTilt], CAMERA_TILT_MIN, CAMERA_TILT_MAX);
    if (flags != UPF_NewGame)
        return;
    ustate->dungeon_camera.zoom[false] = (prefs[UPref_StartingIsometricZoom] != 0) ? (int32_t)prefs[UPref_StartingIsometricZoom] : CAMERA_ZOOM_MAX;
    ustate->dungeon_camera.zoom[true] = (prefs[UPref_StartingFrontviewZoom] != 0) ? (int32_t)prefs[UPref_StartingFrontviewZoom] : FRONTVIEW_CAMERA_ZOOM_MAX;
}

void apply_user_preferences(NetUserId user, const UserPreferences prefs, unsigned char flags)
{
    struct UserState *ustate = get_user_state(user);
    if (user_state_invalid(ustate))
        return;
    for (int pref = 0; pref < UPref_Count; pref++)
    {
        if ((flags == UPF_NewGame) || ((user_preference_flags[pref] & flags) != 0))
            ustate->prefs[pref] = prefs[pref];
    }
    apply_user_starting_preferences(ustate, flags);
}

void apply_local_user_preferences(NetUserId user, unsigned char flags)
{
    UserPreferences prefs;
    build_local_user_preferences(prefs);
    apply_user_preferences(user, prefs, flags);
}

void apply_user_start_tendencies(struct PlayerInfo *player)
{
    const struct UserState *ustate = get_player_user_state(player);
    if (user_state_invalid(ustate))
        return;
    TbBool imprison = (ustate->prefs[UPref_StartingTendencies] & CrTend_Imprison) != 0;
    TbBool flee = (ustate->prefs[UPref_StartingTendencies] & CrTend_Flee) != 0;
    set_creature_tendencies(player, CrTend_Imprison, imprison);
    set_creature_tendencies(player, CrTend_Flee, flee);
    if (player->id_number == my_player_number) {
        game.creatures_tend_imprison = imprison;
        game.creatures_tend_flee = flee;
    }
}

TbBool user_cheats_allowed(NetUserId user)
{
    if (get_user_state(SERVER_ID)->prefs[UPref_Cheats] == 0)
        return false;
    if (!user_present(user))
        return true;
    return get_user_state(user)->prefs[UPref_Cheats] != 0;
}

TbBool game_censorship_enabled(void)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        if (user_present(user) && (get_user_state(user)->prefs[UPref_Censorship] != 0))
            return true;
    }
    return false;
}

static void setup_players_from_startup_packets(const struct StartupSyncPacket startup_sync_packets[MAX_NET_USERS])
{
    for (NetUserId i = 0; i < MAX_NET_USERS; i++) {
        const struct StartupSyncPacket *sync = &startup_sync_packets[i];
        if (!net_user_info[i].network_user_active) {
            continue;
        }
        int k = get_net_user_player_number(i);
        if (k < 0) {
            continue;
        }
        struct PlayerInfo *player = get_player(k);
        struct UserState* ustate = get_user_state(i);
        player->id_number = k;
        player->allocflags |= PlaF_Allocated;
        init_user_state(i, k);
        apply_user_preferences(i, sync->user_prefs, UPF_NewGame);
        init_player(player, 0);
        apply_user_start_tendencies(player);
        snprintf(player->player_name, sizeof(struct TbNetworkPlayerName), "%s", network_user_name(i));
    }
}

static TbBool verify_map_checksums(const struct StartupSyncPacket startup_sync_packets[MAX_NET_USERS])
{
    const TbBigChecksum *host = startup_sync_packets[SERVER_ID].map_checksums;
    for (int i = 0; i < MAX_NET_USERS; i++) {
        const TbBigChecksum *client = startup_sync_packets[i].map_checksums;
        if (!net_user_info[i].network_user_active) {
            continue;
        }
        int diff_count = 0;
        for (int j = 0; j < NETWORK_STARTUP_MAP_FILE_COUNT; j++) {
            if (client[j] == host[j]) {
                continue;
            }
            if (diff_count == 0) {
                ERRORLOG("Level checksums differ for player %d", i);
            }
            ERRORLOG("Level file map%05u.%s differs for player %d", get_loaded_level_number(), network_startup_compare_files[j], i);
            diff_count++;
        }
        if (diff_count != 0) {
            return false;
        }
    }
    NETLOG("Map checksums are verified");
    return true;
}

static TbBool verify_startup_sprite_zip_checksums(const struct StartupSyncPacket startup_sync_packets[MAX_NET_USERS])
{
    NetUserId host_user_id = SERVER_ID;
    const struct StartupSyncPacket *host_sync = &startup_sync_packets[host_user_id];
    TbBool verified = true;
    for (NetUserId i = 0; i < MAX_NET_USERS; i++) {
        if (!net_user_info[i].network_user_active || i == host_user_id) {
            continue;
        }
        const struct StartupSyncPacket *client_sync = &startup_sync_packets[i];
        for (int zip_idx = 0; zip_idx < REQUIRED_SPRITE_ZIP_COUNT; zip_idx++) {
            if (client_sync->required_sprite_zip_checksums[zip_idx] == host_sync->required_sprite_zip_checksums[zip_idx]) {
                continue;
            }
            WARNLOG("Custom sprite zip differs between %s and %s: %s", network_user_name(host_user_id), network_user_name(i), required_sprite_zips[zip_idx]);
            message_add_fmt(MsgType_Blank, 0, "/fxdata/%.30s differs for %.12s", required_sprite_zips[zip_idx], network_user_name(i));
            verified = false;
        }
    }
    return verified;
}

static struct StartupSyncPacket s_local_startup_sync;
static struct StartupSyncPacket s_startup_sync_packets[MAX_NET_USERS];

// the preferences a user sent in the startup sync, for network games
TbBool get_startup_user_preferences(NetUserId user, UserPreferences prefs)
{
    if (!network_is_active() || (user < 0) || (user >= MAX_NET_USERS) || !s_startup_sync_packets[user].startup_sync_packet_valid)
        return false;
    memcpy(prefs, s_startup_sync_packets[user].user_prefs, sizeof(UserPreferences));
    return true;
}



static uint8_t calculate_initial_input_lag(void)
{
    int32_t player_count = 0;
    for (int32_t i = 0; i < MAX_NET_USERS; i++) {
        if (net_user_info[i].network_user_active) {
            player_count++;
        }
    }
    uint64_t ping = network_lobby_ping;
    if (player_count == 2) {
        ping /= 2;
    }
    uint64_t input_lag_turns = 0;
    if (turns_per_second > 0) {
        input_lag_turns = (ping * turns_per_second + 999) / 1000;
    }
    uint64_t uncapped_input_lag_turns = input_lag_turns;
    if (input_lag_turns > 0) {
        input_lag_turns -= 1;
    }
    if (input_lag_turns > MAXIMUM_INPUT_LAG_TURNS) {
        input_lag_turns = MAXIMUM_INPUT_LAG_TURNS;
    }
    JUSTLOG("Initial input lag: (%llu ms * %d turns/s + 999) / 1000 = %llu turns, adjusted to %llu", (unsigned long long)ping, turns_per_second, (unsigned long long)uncapped_input_lag_turns, (unsigned long long)input_lag_turns);
    return input_lag_turns;
}

static void build_local_startup_sync(void)
{
    memset(&s_local_startup_sync, 0, sizeof(s_local_startup_sync));
    s_local_startup_sync.startup_sync_packet_valid = 1;
    calculate_network_startup_map_checksums(s_local_startup_sync.map_checksums);
    memcpy(s_local_startup_sync.required_sprite_zip_checksums, required_sprite_zip_checksums, sizeof(s_local_startup_sync.required_sprite_zip_checksums));
    build_local_user_preferences(s_local_startup_sync.user_prefs);
    s_local_startup_sync.initial_input_lag_turns = calculate_initial_input_lag();
    s_local_startup_sync.initial_action_seed = (uint32_t)initial_replay_seed;
}

static TbBool net_startup_sync_exchange_and_apply(void)
{
    memset(s_startup_sync_packets, 0, sizeof(s_startup_sync_packets));
    if (exchange_frame_block(NETMSG_STARTUP_SYNC, &s_local_startup_sync, s_startup_sync_packets, sizeof(struct StartupSyncPacket)) != Lb_OK) {
        ERRORLOG("Startup sync exchange failed");
        return false;
    }

    for (int i = 0; i < MAX_NET_USERS; i++) {
        if (net_user_info[i].network_user_active && !s_startup_sync_packets[i].startup_sync_packet_valid) {
            ERRORLOG("Startup sync exchange missed one or more peers");
            return false;
        }
    }
    if (!verify_map_checksums(s_startup_sync_packets)) {
        create_frontend_error_box(get_string(GUIStr_NetUnsyncedMap));
        return false;
    }

    if (!verify_startup_sprite_zip_checksums(s_startup_sync_packets)) {
        create_frontend_error_box(get_string(GUIStr_NetVerifyFxdataSame));
        return false;
    }
    const struct StartupSyncPacket *host_sync = &s_startup_sync_packets[SERVER_ID];
    game.input_lag_turns = host_sync->initial_input_lag_turns;
    input_lag_reset();
    game.skip_initial_input_turns = calculate_skip_input();
    NETLOG("Startup input lag: %d", game.input_lag_turns);
    if (host_sync->initial_action_seed != (uint32_t)initial_replay_seed)
    {
        ERRORLOG("Initial action seed %u differs from host's %u", (unsigned)initial_replay_seed, (unsigned)host_sync->initial_action_seed);
        initial_replay_seed = host_sync->initial_action_seed;
    }
    setup_players_from_startup_packets(s_startup_sync_packets);
    return true;
}

static void setup_network_player_numbers(void)
{
    TbBool is_set = false;
    int k = 0;
    SYNCDBG(6, "Starting");
    for (NetUserId i = 0; i < MAX_NET_USERS; i++)
    {
        game.user_states[i].player_id = PLAYER_NONE;
        if (net_user_info[i].network_user_active)
        {
            game.user_states[i].player_id = k;
            if ((!is_set) && (my_player_number == i))
            {
                is_set = true;
                my_player_number = k;
            }
            k++;
        }
    }
    if (!is_set) {
        ERRORLOG("Local player number %d not found among active network players", my_player_number);
    }
}

void setup_count_players(void)
{
  if (game.game_kind == GKind_LocalGame)
  {
    game.human_players_count = 1;
  } else
  {
    game.human_players_count = 0;
    for (int i = 0; i < MAX_NET_USERS; i++)
    {
      if (net_user_info[i].network_user_active)
        game.human_players_count++;
    }
  }
}

TbBool init_players_network_game(void)
{
    if (net_join_role == NetRole_Spectator) {
        TbBigChecksum checksums[NETWORK_STARTUP_MAP_FILE_COUNT];
        calculate_network_startup_map_checksums(checksums);
        if (memcmp(checksums, s_startup_sync_packets[SERVER_ID].map_checksums, sizeof(checksums)) != 0 || memcmp(required_sprite_zip_checksums, s_startup_sync_packets[SERVER_ID].required_sprite_zip_checksums, sizeof(required_sprite_zip_checksums)) != 0) {
            create_frontend_error_box(get_string(GUIStr_NetUnsyncedMap));
            LbNetwork_Stop();
            return false;
        }
        for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
            set_net_user_player_number(user, spectator_user_player_number[user]);
        }
        setup_players_from_startup_packets(s_startup_sync_packets);
        return true;
    }
    SYNCDBG(4,"Starting");
    TbBool initialized = true;
    setup_network_player_numbers();
    for (int zip_idx = 0; zip_idx < REQUIRED_SPRITE_ZIP_COUNT; zip_idx++) {
        if (required_sprite_zip_checksums[zip_idx] != 0) {
            continue;
        }
        WARNLOG("Required custom sprite zip missing: %s", required_sprite_zips[zip_idx]);
        message_add_fmt(MsgType_Blank, 0, "/fxdata/%.30s missing", required_sprite_zips[zip_idx]);
        create_frontend_error_box(get_string(GUIStr_NetVerifyFxdataSame));
        initialized = false;
        break;
    }
    if (initialized) {
        build_local_startup_sync();
        initialized = net_startup_sync_exchange_and_apply();
    }
    if (!initialized) {
        LbNetwork_Stop();
    }
    return initialized;
}

void network_game_started(void)
{
    net_lobby_set_phase(NetPhase_InGame);
    if (netstate.my_id == SERVER_ID && frontnet_service_selected(FrontendNetSvc_Online)) {
        LevelNumber map_number = get_level_number();
        struct LevelInformation *level_info = get_level_info(map_number);
        const char *map_name = "";
        if (level_info) {
            map_name = level_info->name;
            if (level_info->name_stridx > 0) {
                map_name = get_string(level_info->name_stridx);
            }
        }
        matchmaking_start_game((int)map_number, map_name);
    }
}

/** Check whether a network user is active.
 *
 * @param user
 * @return
 */
TbBool network_user_active(NetUserId user)
{
    if ((user < 0) || (user >= MAX_NET_USERS))
        return false;
    return (net_user_info[user].network_user_active != 0);
}

const char *network_user_name(NetUserId user)
{
    if ((user < 0) || (user >= MAX_NET_USERS))
        return NULL;
    return net_user_info[user].name;
}

TbBool network_human_contenders_remain(void)
{
    for (PlayerNumber player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
        struct PlayerInfo *player = get_player(player_idx);
        if (is_active_keeper(player) && ((player->allocflags & PlaF_CompCtrl) == 0) && !player_cannot_win(player_idx)) {
            return true;
        }
    }
    return false;
}

static TbBool network_has_remote_users_remaining(void)
{
    for (NetUserId user_id = 0; user_id < MAX_NET_USERS; user_id += 1) {
        if (user_id == netstate.my_id) {
            continue;
        }
        if ((user_id < (NetUserId)netstate.max_users) && (netstate.users[user_id].progress != USER_UNUSED)) {
            return true;
        }
        if (user_present(user_id)) {
            const struct PlayerInfo *player = get_player(get_net_user_player_number(user_id));
            if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
                return true;
            }
        }
    }
    return false;
}

static TbBool replay_has_remote_humans(void)
{
    const NetUserId local_user = replay.head.recording_user;
    for (NetUserId user_id = 0; user_id < MAX_NET_USERS; user_id++) {
        const PlayerNumber plyr_idx = get_net_user_player_number(user_id);
        if ((user_id == local_user) || (plyr_idx < 0)) {
            continue;
        }
        const struct PlayerInfo *player = get_player(plyr_idx);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
            return true;
        }
    }
    return false;
}

static void replace_network_player_with_ai(struct PlayerInfo *player)
{
    player->allocflags |= PlaF_CompCtrl | PlaF_Placeholder;
    toggle_computer_player(player->id_number);
    message_add(MsgType_Player, player->id_number, get_string(GUIStr_NetAiTookOver));
    JUSTLOG("p:%d computer took over", player->id_number);
}

// used when ending a netplay game or recording.
// local single-player must have the local user in slot 0.
void remap_user_to_solo(struct PlayerInfo *myplyr)
{
    NetUserId old_user = get_player_primary_user(myplyr);
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (user == old_user) {
            continue;
        }
        struct UserState *ustate = get_user_state(user);
        if (ustate->cursor_light_idx != 0) {
            light_delete_light(ustate->cursor_light_idx);
        }
        memset(ustate, 0, sizeof(*ustate));
        ustate->player_id = PLAYER_NONE;
    }
    struct UserState *old_state = get_user_state(old_user);
    if ((old_user != SOLO_HUMAN_ID) && !user_state_invalid(old_state)) {
        *get_user_state(SOLO_HUMAN_ID) = *old_state;
        memset(old_state, 0, sizeof(*old_state));
        old_state->player_id = PLAYER_NONE;
    }
    set_net_user_player_number(SOLO_HUMAN_ID, myplyr->id_number);
    if (myplyr->roomspace.is_active && (myplyr->roomspace.user == old_user)) {
        myplyr->roomspace.user = SOLO_HUMAN_ID;
    }
}

static void stop_network_game_state(void)
{
    network_spectator_clear_roles();
    memset(net_user_info, 0, sizeof(net_user_info));
    clear_flag(local_system_flags, GSF_NetworkActive);
    remap_user_to_solo(get_my_player());
    clear_flag(local_system_flags, GSF_NetGameNoSync);
    clear_flag(local_system_flags, GSF_NetSeedNoSync);
    fe_network_active = 0;
    game.game_kind = GKind_LocalGame;
    game.input_lag_turns = 0;
    game.skip_initial_input_turns = 0;
    input_lag_reset();
    multiplayer_speed_adjustment_ns = 0;
    setup_count_players();
}

static void stop_network_game_and_quit_to_main_menu(void)
{
    LbNetwork_Stop();
    stop_network_game_state();
    quit_game = 1;
}

static void stop_network_game_and_continue_locally(void)
{
    struct PlayerInfo *survivor;
    if (network_is_active()) {
        LbNetwork_Stop();
        stop_network_game_state();
        survivor = get_my_player();
    } else {
        const PlayerNumber plyr_idx = get_net_user_player_number(replay.head.recording_user);
        survivor = (plyr_idx >= 0) ? get_player(plyr_idx) : get_my_player();
        remap_user_to_solo(survivor);
        game.game_kind = GKind_LocalGame;
        setup_count_players();
    }
    survivor->display_objective_turn = get_gameturn() + 1;
}

static TbBool host_already_won_level(void)
{
    GameTurn newest_turn = get_gameturn();
    for (GameTurnDelta offset = 0; offset <= game.input_lag_turns; offset += 1) {
        if ((GameTurn)offset > newest_turn) {
            break;
        }
        const struct Packet *host_packet = get_history_packet(SERVER_ID, newest_turn - offset);
        if (host_packet != NULL && host_packet->action == PckA_FinishGame && host_packet->actn_par1 == VicS_WonLevel) {
            return true;
        }
    }
    return false;
}

static TbBool placeholder_has_loadbearing_ally(const struct PlayerInfo *placeholder)
{
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++) {
        struct PlayerInfo *other = get_player(plyr_idx);
        if ((other == placeholder) || !is_active_keeper(other)
            || flag_is_set(other->allocflags, PlaF_CompCtrl) || player_defeat_settled(plyr_idx)) {
            continue;
        }
        if (players_are_mutual_allies(placeholder->id_number, plyr_idx)) {
            return true;
        }
    }
    return false;
}

// placeholders with no human still propping them up are marked as defeated;
// placeholders whose outcome is settled are deallocated
void resolve_placeholders(void)
{
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++) {
        struct PlayerInfo *player = get_player(plyr_idx);
        if (!player_exists(player) || !player_is_placeholder(player)) {
            continue;
        }
        if ((player->victory_state == VicS_Undecided) && !placeholder_has_loadbearing_ally(player)) {
            JUSTLOG("p:%d defeated, no human allies remain", (int)plyr_idx);
            event_kill_all_players_events(plyr_idx);
            set_player_as_lost_level(player);
        }
        if ((player->victory_state == VicS_WonLevel) || player_defeat_settled(plyr_idx)) {
            player->allocflags &= ~PlaF_Allocated;
        }
    }
}

static void abandon_network_player(struct PlayerInfo *player, TbBool announce)
{
    if ((player->allocflags & PlaF_CompCtrl) == 0) {
        if (network_is_active()) {
            // re-negotiate input latency
            network_lobby_ping = GetPlayersPing();
            input_lag_reset_request(calculate_initial_input_lag());
        }
        if (announce && player->player_name[0] != '\0') {
            message_add_fmt(MsgType_Blank, 0, get_string(GUIStr_NetPlayerDisconnected), player->player_name);
        }
        JUSTLOG("p:%d player %s departed", player->id_number, player->player_name);
        if (player->victory_state == VicS_Undecided) {
            replace_network_player_with_ai(player);
        }
    }
    resolve_placeholders();
    if (player->victory_state != VicS_Undecided) {
        player->allocflags &= ~PlaF_Allocated;
    }
}

// Take a user out of the game: their player is handed to the AI, or written
// off if their fate was already decided. Nothing here touches the transport.
static void remove_user_from_game(NetUserId user, TbBool announce)
{
    struct UserState *ustate = get_user_state(user);
    if (user_state_invalid(ustate) || (ustate->player_id == PLAYER_NONE)) {
        return;
    }
    struct PlayerInfo *player = get_player(ustate->player_id);
    JUSTLOG("u:%d user left the game (player %d)", (int)user, (int)ustate->player_id);
    if (ustate->cursor_light_idx != 0) {
        light_delete_light(ustate->cursor_light_idx);
    }
    memset(ustate, 0, sizeof(*ustate));
    ustate->player_id = PLAYER_NONE;
    // another user may still be driving this keeper
    if (!player_exists(player) || (get_player_primary_user(player) >= 0)) {
        return;
    }
    abandon_network_player(player, announce);
}

static void leave_network_if_alone(void)
{
    if (network_is_host() && net_config_info.spectators_enabled) {
        return;
    }
    if (!network_has_remote_users_remaining()) {
        stop_network_game_and_continue_locally();
    }
}

void process_user_leave_game_packet(NetUserId user)
{
    if (network_is_active() && network_user_is_spectator(netstate.my_id) && user == SERVER_ID) {
        stop_network_game_and_quit_to_main_menu();
        return;
    }
    if (user == get_local_user()) {
        if (network_is_active()) {
            stop_network_game_and_quit_to_main_menu();
        } else {
            quit_game = 1;
        }
        get_my_player()->allocflags &= ~PlaF_Allocated;
        return;
    }
    if (game.game_kind != GKind_MultiGame) {
        // the user's keeper simply vanishes
        struct PlayerInfo *player = get_player(get_net_user_player_number(user));
        if (player_exists(player)) {
            player->allocflags &= ~PlaF_Allocated;
        }
        return;
    }

    // note: packet processed in sync with simulation, it's okay to do this.
    // replays take this path too, so the departure is visible there.
    if (network_is_active()) {
        OnDroppedUser(user, NETDROP_MANUAL);
    }
    remove_user_from_game(user, user != SERVER_ID);
    if (network_is_active()) {
        leave_network_if_alone();
    } else if (replay.load_enable && !replay_has_remote_humans()) {
        stop_network_game_and_continue_locally();
    }
}

// (host-only) host sends packets for dropped users, indicating
// the user has dropped.
void host_spoof_dropped_user_packets(void)
{
    if (!network_is_active() || !network_is_host()) {
        return;
    }
    GameTurn first_turn = get_gameturn();
    if (first_turn > (GameTurn)game.input_lag_turns) {
        first_turn -= game.input_lag_turns;
    }
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if ((user == netstate.my_id) || network_user_active(user) || !user_present(user)) {
            continue;
        }
        struct Packet spoofed;
        memset(&spoofed, 0, sizeof(spoofed));
        set_packet_action(&spoofed, PckA_ForceApplicationClose, 0, 0, 0, 0);
        // only the turns user_has_required_turn_packets expects
        for (GameTurn turn = first_turn; turn < first_turn + 2; turn++) {
            if (get_history_packet(user, turn) != NULL) {
                continue;
            }
            spoofed.turn = turn;
            store_packet_history(user, &spoofed);
        }
    }
}
 
void process_disconnected_network_players(void)
{
    if (!network_is_active() || network_is_host() || (netstate.users[SERVER_ID].progress != USER_UNUSED)) {
        return;
    }
    struct UserState *ustate = get_local_user_state();
    if (host_already_won_level()) {
        ustate->additional_flags &= ~UsrAF_UnlockedLordTorture;
        quit_game = 1;
        return;
    }
    message_add(MsgType_Blank, 0, get_string(GUIStr_NetHostConnectionLost));
    if (netstate.my_id >= MAX_NET_USERS) {
        stop_network_game_and_quit_to_main_menu();
        return;
    }
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (user != netstate.my_id) {
            remove_user_from_game(user, false);
        }
    }
    replay_record_network_stopped();
    stop_network_game_and_continue_locally();
}

void apply_recorded_network_stop(void)
{
    message_add(MsgType_Blank, 0, get_string(GUIStr_NetHostConnectionLost));
    const NetUserId local_user = replay.head.recording_user;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (user != local_user) {
            remove_user_from_game(user, false);
        }
    }
    stop_network_game_and_continue_locally();
}

long network_session_join(void)
{
    int32_t plyr_num;
    net_join_rejection = NetJoin_Accepted;
    reset_attempting_to_join_cancel();
    display_attempting_to_join_message(-1);
    if (attempting_to_join_cancel_requested())
        return -1;
    snprintf(join_lobby_id, sizeof(join_lobby_id), "%s", net_session[net_session_index_active]->join_address);
    if (LbNetwork_Join(net_session[net_session_index_active], net_player_name, &plyr_num, NULL) == 0)
        return plyr_num;
    join_lobby_id[0] = '\0';
    if (!attempting_to_join_cancel_requested()) {
        if (frontnet_service_selected(FrontendNetSvc_Online)) {
            net_session_index_active = -1;
            net_session_index_active_id = -1;
            matchmaking_request_list();
        }
        const char *error = net_join_error_text(net_join_rejection);
        if (error) {
            create_frontend_error_box(error);
        } else {
            process_network_error(-802);
        }
    }
    return -1;
}

void sync_initial_network_seed(void)
{
   if (net_join_role == NetRole_Spectator) {
       game.action_random_seed = s_startup_sync_packets[SERVER_ID].initial_action_seed;
       game.ai_random_seed = game.action_random_seed * 9377 + 9391;
       game.player_random_seed = game.action_random_seed * 9473 + 9479;
       initial_replay_seed = game.action_random_seed;
       return;
   }
   if (!network_is_active()) {
      return;
   }
   struct { uint32_t action_seed; int32_t timestamp_tz; int64_t timestamp; } initial = { game.action_random_seed, game.timestamp_tz, game.timestamp };
   if (!LbNetwork_Resync(&initial, sizeof(initial))) {
      ERRORLOG("Initial sync failed");
      return;
   }
   game.action_random_seed = initial.action_seed;
   game.timestamp = initial.timestamp;
   game.timestamp_tz = initial.timestamp_tz;
   game.ai_random_seed = game.action_random_seed * 9377 + 9391;
   game.player_random_seed = game.action_random_seed * 9473 + 9479;
   initial_replay_seed = game.action_random_seed;
   NETLOG("Initial network seed synced: action_seed=%u", game.action_random_seed);
}
void network_spectator_send_bootstrap(NetUserId user_id)
{
    char *write_pos = begin_net_message(NETMSG_SPECTATOR_BOOTSTRAP);
    int32_t level = get_loaded_level_number();
    memcpy(write_pos, &level, sizeof(level));
    write_pos += sizeof(level);
    snprintf(write_pos, DISKPATH_SIZE, "%s", campaign.fname);
    write_pos += strlen(write_pos) + 1;
    PlayerNumber user_player_number[MAX_NET_USERS];
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        user_player_number[user] = get_net_user_player_number(user);
    }
    memcpy(write_pos, user_player_number, sizeof(user_player_number));
    write_pos += sizeof(user_player_number);
    memcpy(write_pos, s_startup_sync_packets, sizeof(s_startup_sync_packets));
    write_pos += sizeof(s_startup_sync_packets);
    send_message_buffer(user_id, write_pos);
}

TbError process_network_spectator_bootstrap(NetUserId source, const char *buffer, size_t size)
{
    if (source != SERVER_ID || net_join_role != NetRole_Spectator || size < sizeof(int32_t) + 1 + sizeof(spectator_user_player_number) + sizeof(s_startup_sync_packets)) {
        return Lb_FAIL;
    }
    int32_t level;
    memcpy(&level, buffer, sizeof(level));
    buffer += sizeof(level);
    size -= sizeof(level);
    size_t name_length = strnlen(buffer, min(size, (size_t)DISKPATH_SIZE));
    if (name_length == 0 || name_length >= DISKPATH_SIZE || name_length + 1 + sizeof(spectator_user_player_number) + sizeof(s_startup_sync_packets) != size || strchr(buffer, '/') || strchr(buffer, '\\') || strstr(buffer, "..")) {
        return Lb_FAIL;
    }
    char campaign_file[DISKPATH_SIZE];
    uint8_t pack = prepare_campaign_file_name(buffer, campaign_file, sizeof(campaign_file));
    if (pack == CampgnT_Default && is_campaign_in_list(campaign_file, &mp_mappacks_list)) {
        pack = CampgnT_MultiplayerMappack;
    }
    if (!change_campaign(pack, campaign_file) || strcasecmp(campaign.fname, campaign_file) != 0 || level <= 0 || get_level_info(level) == NULL) {
        return Lb_FAIL;
    }
    buffer += name_length + 1;
    memcpy(spectator_user_player_number, buffer, sizeof(spectator_user_player_number));
    buffer += sizeof(spectator_user_player_number);
    memcpy(s_startup_sync_packets, buffer, sizeof(s_startup_sync_packets));
    for (NetUserId id = 0; id < MAX_NET_USERS; id++) {
        if (spectator_user_player_number[id] < PLAYER_NONE || spectator_user_player_number[id] >= PLAYERS_COUNT) {
            return Lb_FAIL;
        }
    }
    set_selected_level_number(level);
    netstate.phase = NetPhase_InGame;
    return Lb_OK;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
