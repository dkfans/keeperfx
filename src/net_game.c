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
#include "net_exchange_common.h"
#include "net_lobby.h"
#include "net_main.h"
#include "net_resync.h"

#include "player_data.h"
#include "front_landview.h"
#include "player_utils.h"
#include "packets.h"
#include "frontend.h"
#include "front_network.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "config_strings.h"
#include "custom_sprites.h"
#include "dungeon_data.h"
#include "game_legacy.h"
#include "gui_msgs.h"
#include "net_input_lag.h"
#include "net_checksums.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbNetworkPlayerInfo net_player_info[MAX_NET_USERS];
extern int32_t multiplayer_speed_adjustment_ns;
/******************************************************************************/

#pragma pack(1)
struct StartupSyncPacket {
    uint8_t startup_sync_packet_valid;
    int32_t video_rotate_mode;
    int32_t input_lag_turns;
    TbBigChecksum map_checksum;
    uint32_t sprite_zip_checksum;
    uint16_t initial_tendencies;
    uint32_t isometric_view_zoom_level;
    uint32_t frontview_zoom_level;
    uint32_t zoom_distance_setting;
    uint32_t frontview_zoom_distance_setting;
};
#pragma pack()

short setup_network_service(enum FrontendNetService service)
{
  struct ServiceInitData *init_data = NULL;
  SYNCMSG("Initializing 4-players type %d network", service);
  memset(net_player_info, 0, sizeof(net_player_info));
  if (service != FrontendNetSvc_Online && service != FrontendNetSvc_LAN) {
    process_network_error(-800);
    return 0;
  }
  if ( LbNetwork_Init(NS_ENET_UDP, MAX_NET_USERS, &net_player_info[0], init_data) )
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

unsigned long get_host_player_id(void) {
    return 0;
}

int setup_old_network_service(void)
{
    return setup_network_service(net_service_index_selected);
}


static void setup_players_from_startup_packets(const struct StartupSyncPacket startup_sync_packets[MAX_NET_USERS])
{
    int k = 0;
    for (int i = 0; i < MAX_NET_USERS; i++) {
        const struct StartupSyncPacket *sync = &startup_sync_packets[i];
        if (!net_player_info[i].network_user_active) {
            continue;
        }
        struct PlayerInfo *player = get_player(k);
        player->id_number = k;
        player->packet_num = i;
        player->allocflags |= PlaF_Allocated;
        switch (sync->video_rotate_mode) {
            case 0: player->view_mode_restore = PVM_IsoWibbleView; break;
            case 1: player->view_mode_restore = PVM_IsoStraightView; break;
            case 2: player->view_mode_restore = PVM_FrontView; break;
            default: player->view_mode_restore = PVM_IsoWibbleView; break;
        }
        player->is_active = 1;
        init_player(player, 0);
        player->isometric_view_zoom_level = sync->isometric_view_zoom_level;
        player->frontview_zoom_level = sync->frontview_zoom_level;
        TbBool imprison = (sync->initial_tendencies & CrTend_Imprison) != 0;
        TbBool flee = (sync->initial_tendencies & CrTend_Flee) != 0;
        set_creature_tendencies(player, CrTend_Imprison, imprison);
        set_creature_tendencies(player, CrTend_Flee, flee);
        if (player->id_number == my_player_number) {
            game.creatures_tend_imprison = imprison;
            game.creatures_tend_flee = flee;
        }
        snprintf(player->player_name, sizeof(struct TbNetworkPlayerName), "%s", net_player[i].name);
        k++;
    }
}

static TbBool verify_map_checksums(const struct StartupSyncPacket startup_sync_packets[MAX_NET_USERS])
{
    const TbBigChecksum host_checksum = startup_sync_packets[get_host_player_id()].map_checksum;
    for (int i = 0; i < MAX_NET_USERS; i++) {
        if (net_player_info[i].network_user_active && startup_sync_packets[i].map_checksum != host_checksum) {
            ERRORLOG("Level checksums %08x(Host) != %08x(Client) for player %d", host_checksum, startup_sync_packets[i].map_checksum, i);
            return false;
        }
    }
    NETLOG("Map checksums are verified");
    return true;
}

static void verify_startup_sprite_zip_checksums(const struct StartupSyncPacket startup_sync_packets[MAX_NET_USERS])
{
    for (int i = 0; i < MAX_NET_USERS; i++) {
        if (net_player_info[i].network_user_active && startup_sync_packets[i].sprite_zip_checksum != sprite_zip_combined_checksum) {
            message_add(MsgType_Player, 0, get_string(GUIStr_NetVerifyFxdataSame));
            message_add_fmt(MsgType_Player, 0, get_string(GUIStr_NetCustomSpriteMismatch), network_player_name(i));
        }
    }
}

static struct StartupSyncPacket s_local_startup_sync;
static struct StartupSyncPacket s_startup_sync_packets[MAX_NET_USERS];

static void build_local_startup_sync(void)
{
    memset(&s_local_startup_sync, 0, sizeof(s_local_startup_sync));
    s_local_startup_sync.startup_sync_packet_valid = 1;
    s_local_startup_sync.video_rotate_mode = settings.video_rotate_mode;
    s_local_startup_sync.input_lag_turns = game.input_lag_turns;
    s_local_startup_sync.map_checksum = calculate_network_startup_map_checksum();
    s_local_startup_sync.sprite_zip_checksum = sprite_zip_combined_checksum;
    uint16_t initial_tendencies = 0;
    if (IMPRISON_BUTTON_DEFAULT) {initial_tendencies |= CrTend_Imprison;}
    if (FLEE_BUTTON_DEFAULT) {initial_tendencies |= CrTend_Flee;}
    s_local_startup_sync.initial_tendencies = initial_tendencies;
    s_local_startup_sync.isometric_view_zoom_level = settings.isometric_view_zoom_level;
    s_local_startup_sync.frontview_zoom_level = settings.frontview_zoom_level;
    s_local_startup_sync.zoom_distance_setting = zoom_distance_setting;
    s_local_startup_sync.frontview_zoom_distance_setting = frontview_zoom_distance_setting;
}

static TbBool net_startup_sync_exchange_and_apply(void)
{
    memset(s_startup_sync_packets, 0, sizeof(s_startup_sync_packets));
    if (exchange_frame_block(NETMSG_STARTUP_SYNC, &s_local_startup_sync, s_startup_sync_packets, sizeof(struct StartupSyncPacket)) != Lb_OK) {
        ERRORLOG("Startup sync exchange failed");
        return false;
    }

    for (int i = 0; i < MAX_NET_USERS; i++) {
        if (net_player_info[i].network_user_active && !s_startup_sync_packets[i].startup_sync_packet_valid) {
            ERRORLOG("Startup sync exchange missed one or more peers");
            return false;
        }
    }
    if (!verify_map_checksums(s_startup_sync_packets)) {
        create_frontend_error_box(5000, get_string(GUIStr_NetUnsyncedMap));
        return false;
    }

    verify_startup_sprite_zip_checksums(s_startup_sync_packets);
    const struct StartupSyncPacket *host_sync = &s_startup_sync_packets[get_host_player_id()];
    game.input_lag_turns = host_sync->input_lag_turns;
    game.skip_initial_input_turns = calculate_skip_input();
    NETLOG("Startup input lag synced: input_lag=%d", game.input_lag_turns);
    zoom_distance_setting = host_sync->zoom_distance_setting;
    frontview_zoom_distance_setting = host_sync->frontview_zoom_distance_setting;
    setup_players_from_startup_packets(s_startup_sync_packets);
    return true;
}

static void setup_network_player_numbers(void)
{
    TbBool is_set = false;
    int k = 0;
    SYNCDBG(6, "Starting");
    for (int i = 0; i < MAX_NET_USERS; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (net_player_info[i].network_user_active)
        {
            player->packet_num = i;
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
    game.active_players_count = 1;
  } else
  {
    game.active_players_count = 0;
    for (int i = 0; i < MAX_NET_USERS; i++)
    {
      if (net_player_info[i].network_user_active)
        game.active_players_count++;
    }
  }
}

TbBool init_players_network_game(void)
{
    SYNCDBG(4,"Starting");
    setup_network_player_numbers();
    build_local_startup_sync();
    return net_startup_sync_exchange_and_apply();
}

/** Check whether a network player is active.
 *
 * @param plyr_idx
 * @return
 */
TbBool network_player_active(int plyr_idx)
{
    if ((plyr_idx < 0) || (plyr_idx >= MAX_NET_USERS))
        return false;
    return (net_player_info[plyr_idx].network_user_active != 0);
}

const char *network_player_name(int plyr_idx)
{
    if ((plyr_idx < 0) || (plyr_idx >= MAX_NET_USERS))
        return NULL;
    return net_player_info[plyr_idx].name;
}

static void resolve_network_quit_outcome(struct PlayerInfo *player)
{
    if (player->victory_state != VicS_Undecided) {
        return;
    }
    if (player_cannot_win(player->id_number)) {
        set_player_as_lost_level(player);
        return;
    }
    set_player_as_won_level(player);
}

static TbBool network_has_connected_human_opponents(void)
{
    struct PlayerInfo *myplyr = get_my_player();
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo *player = get_player(i);
        if (player_exists(player) && !is_my_player(player) && player->is_active == 1 && (player->allocflags & PlaF_CompCtrl) == 0
         && !player_cannot_win(player->id_number) && network_player_active(player->packet_num)
         && players_are_enemies(myplyr->id_number, player->id_number)) {
            return true;
        }
    }
    return false;
}

static TbBool replace_network_player_with_ai(struct PlayerInfo *player, const char *departure_message_format)
{
    if (is_my_player(player) || (player->allocflags & PlaF_CompCtrl) != 0 || player->victory_state != VicS_Undecided) {
        return false;
    }
    player->allocflags |= PlaF_CompCtrl;
    toggle_computer_player(player->id_number);
    message_add(MsgType_Player, player->id_number, get_string(GUIStr_NetAiTookOver));
    JUSTLOG("p:%d computer took over", player->id_number);
    if (departure_message_format != NULL && player->player_name[0] != '\0') {
        message_add_fmt(MsgType_Blank, 0, departure_message_format, player->player_name);
        JUSTLOG("p:%d player %s departed", player->id_number, player->player_name);
    }
    return true;
}

static void stop_network_game_state(void)
{
    memset(net_player_info, 0, sizeof(net_player_info));
    clear_flag(game.system_flags, GSF_NetworkActive);
    clear_flag(game.system_flags, GSF_NetGameNoSync);
    clear_flag(game.system_flags, GSF_NetSeedNoSync);
    fe_network_active = 0;
    game.game_kind = GKind_LocalGame;
    game.input_lag_turns = 0;
    game.skip_initial_input_turns = 0;
    multiplayer_speed_adjustment_ns = 0;
    setup_count_players();
}

static void stop_network_game_and_quit(short complete_quit)
{
    LbNetwork_Stop();
    stop_network_game_state();
    quit_game = 1;
    if (complete_quit) {
        exit_keeper = 1;
    }
}

static void stop_network_game_and_continue_locally(void)
{
    LbNetwork_Stop();
    stop_network_game_state();
    get_my_player()->display_objective_turn = get_gameturn() + 1;
}

void process_quit_packet(struct PlayerInfo *player, short complete_quit)
{
    struct PlayerInfo *myplyr = get_my_player();
    if ((game.system_flags & GSF_NetworkActive) == 0) {
        player->allocflags &= ~PlaF_Allocated;
        if (player == myplyr) {
            quit_game = 1;
            if (complete_quit) {
                exit_keeper = 1;
            }
        }
        return;
    }

    int32_t plyr_count = 0;
    short winning_quit = winning_player_quitting(player, &plyr_count);
    TbBool replaced_with_ai = replace_network_player_with_ai(player, get_string(GUIStr_NetPlayerLeftGame));
    if (player != myplyr) {
        OnDroppedUser(player->packet_num, NETDROP_MANUAL);
    }
    TbBool has_human_opponents = network_has_connected_human_opponents();
    if (has_human_opponents && replaced_with_ai) {
        return;
    }

    if (winning_quit) {
        for (int i = 0; i < PLAYERS_COUNT; i++) {
            struct PlayerInfo *swplyr = get_player(i);
            if (player_exists(swplyr) && (swplyr->is_active == 1)) {
                resolve_network_quit_outcome(swplyr);
            }
        }
    }

    if (player == myplyr || player->id_number == get_host_player_id() || !has_human_opponents) {
        if (player != myplyr) {
            resolve_network_quit_outcome(myplyr);
        }
        if (winning_quit && (plyr_count > 1)) {
            myplyr->additional_flags |= PlaAF_UnlockedLordTorture;
        }
        if ((player != myplyr) && (myplyr->victory_state != VicS_Undecided)) {
            stop_network_game_and_continue_locally();
        } else {
            stop_network_game_and_quit(complete_quit);
        }
    }

    if (!replaced_with_ai) {
        player->allocflags &= ~PlaF_Allocated;
    }
}

void process_disconnected_network_players(void)
{
    if ((game.system_flags & GSF_NetworkActive) == 0) {
        return;
    }
    TbBool host_disconnected = (netstate.my_id != SERVER_ID) && (netstate.users[SERVER_ID].progress == USER_UNUSED);
    const char *departure_message_format = get_string(GUIStr_NetPlayerDisconnected);
    if (host_disconnected) {
        departure_message_format = NULL;
    }
    for (int player_index = 0; player_index < MAX_NET_USERS; player_index++) {
        struct PlayerInfo *player = get_player(player_index);
        if (!player_exists(player) || is_my_player(player) || (!host_disconnected && network_player_active(player->packet_num))) {
            continue;
        }
        if (!replace_network_player_with_ai(player, departure_message_format) && player->victory_state != VicS_Undecided) {
            player->allocflags &= ~PlaF_Allocated;
        }
    }

    if (!host_disconnected && network_has_connected_human_opponents()) {
        return;
    }
    struct PlayerInfo *myplyr = get_my_player();
    resolve_network_quit_outcome(myplyr);
    const char *message = get_string(GUIStr_NetOpponentDisconnected);
    if (host_disconnected) {
        message = get_string(GUIStr_NetHostConnectionLost);
    }
    message_add(MsgType_Blank, 0, message);
    if (myplyr->victory_state != VicS_Undecided) {
        stop_network_game_and_continue_locally();
    } else {
        stop_network_game_and_quit(false);
    }
}

long network_session_join(void)
{
    int32_t plyr_num;
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
        process_network_error(-802);
    }
    return -1;
}

void sync_initial_network_seed(void)
{
   if ((game.system_flags & GSF_NetworkActive) == 0) {
      return;
   }
   if (!LbNetwork_Resync(&game.action_random_seed, sizeof(game.action_random_seed))) {
      ERRORLOG("Initial sync failed");
      return;
   }
   game.ai_random_seed = game.action_random_seed * 9377 + 9391;
   game.player_random_seed = game.action_random_seed * 9473 + 9479;
   NETLOG("Initial network seed synced: action_seed=%u", game.action_random_seed);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
