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
#include "bflib_network.h"
#include "bflib_network_exchange.h"
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
struct TbNetworkPlayerInfo net_player_info[NET_PLAYERS_COUNT];
struct TbNetworkSessionNameEntry *net_session[32];
long net_number_of_sessions;
long net_session_index_active;
struct TbNetworkPlayerName net_player[NET_PLAYERS_COUNT];
struct ConfigInfo net_config_info;
char net_service[16][NET_SERVICE_LEN];
char net_player_name[20];
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
  memset(&net_player_info[0], 0, sizeof(struct TbNetworkPlayerInfo));
  if (service != FrontendNetSvc_Online && service != FrontendNetSvc_LAN) {
    process_network_error(-800);
    return 0;
  }
  if ( LbNetwork_Init(NS_ENET_UDP, NET_PLAYERS_COUNT, &net_player_info[0], init_data) )
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


static void setup_players_from_startup_packets(const struct StartupSyncPacket startup_sync_packets[NET_PLAYERS_COUNT])
{
    int k = 0;
    for (int i = 0; i < NET_PLAYERS_COUNT; i++) {
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

static void sync_startup_input_lag(const struct StartupSyncPacket startup_sync_packets[NET_PLAYERS_COUNT])
{
    game.input_lag_turns = startup_sync_packets[get_host_player_id()].input_lag_turns;
    game.skip_initial_input_turns = calculate_skip_input();
    NETLOG("Startup input lag synced: input_lag=%d", game.input_lag_turns);
}

static void sync_startup_zoom_distance(const struct StartupSyncPacket startup_sync_packets[NET_PLAYERS_COUNT])
{
    zoom_distance_setting = startup_sync_packets[get_host_player_id()].zoom_distance_setting;
    frontview_zoom_distance_setting = startup_sync_packets[get_host_player_id()].frontview_zoom_distance_setting;
}

static TbBool verify_map_checksums(const struct StartupSyncPacket startup_sync_packets[NET_PLAYERS_COUNT])
{
    const TbBigChecksum host_checksum = startup_sync_packets[get_host_player_id()].map_checksum;
    for (int i = 0; i < NET_PLAYERS_COUNT; i++) {
        if (net_player_info[i].network_user_active && startup_sync_packets[i].map_checksum != host_checksum) {
            ERRORLOG("Level checksums %08x(Host) != %08x(Client) for player %d", host_checksum, startup_sync_packets[i].map_checksum, i);
            return false;
        }
    }
    NETLOG("Map checksums are verified");
    return true;
}

static void verify_startup_sprite_zip_checksums(const struct StartupSyncPacket startup_sync_packets[NET_PLAYERS_COUNT])
{
    for (int i = 0; i < NET_PLAYERS_COUNT; i++) {
        if (net_player_info[i].network_user_active && startup_sync_packets[i].sprite_zip_checksum != sprite_zip_combined_checksum) {
            message_add_fmt(MsgType_Player, 0, "Verify /fxdata/ is the same across all PCs.");
            message_add_fmt(MsgType_Player, 0, "WARNING: Custom sprite mismatch with %s!", network_player_name(i));
        }
    }
}

static struct StartupSyncPacket s_local_startup_sync;
static struct StartupSyncPacket s_startup_sync_packets[NET_PLAYERS_COUNT];

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

static TbBool all_human_players_sent_startup_sync(void)
{
    for (int i = 0; i < NET_PLAYERS_COUNT; i++) {
        if (net_player_info[i].network_user_active && !s_startup_sync_packets[i].startup_sync_packet_valid) {
            return false;
        }
    }
    return true;
}

static CoroutineLoopState net_startup_wait_for_players_and_exchange(CoroutineLoop *context)
{
    (void)context;
    memset(s_startup_sync_packets, 0, sizeof(s_startup_sync_packets));
    LbNetwork_Exchange(NETMSG_STARTUP_SYNC, &s_local_startup_sync, s_startup_sync_packets, sizeof(struct StartupSyncPacket));
    if (!all_human_players_sent_startup_sync()) {
        return CLS_REPEAT;
    }
    return CLS_CONTINUE;
}

static CoroutineLoopState net_startup_sync_apply(CoroutineLoop *context)
{
    if (!verify_map_checksums(s_startup_sync_packets)) {
        create_frontend_error_box(5000, get_string(GUIStr_NetUnsyncedMap));
        coroutine_clear(context, true);
        return CLS_ABORT;
    }
    verify_startup_sprite_zip_checksums(s_startup_sync_packets);
    sync_startup_input_lag(s_startup_sync_packets);
    sync_startup_zoom_distance(s_startup_sync_packets);
    setup_players_from_startup_packets(s_startup_sync_packets);
    return CLS_CONTINUE;
}

static void setup_network_player_numbers(void)
{
    TbBool is_set = false;
    int k = 0;
    SYNCDBG(6, "Starting");
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
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
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
      if (net_player_info[i].network_user_active)
        game.active_players_count++;
    }
  }
}

void init_players_network_game(CoroutineLoop *context)
{
    SYNCDBG(4,"Starting");
    setup_network_player_numbers();
    build_local_startup_sync();
    coroutine_add(context, &net_startup_wait_for_players_and_exchange);
    coroutine_add(context, &net_startup_sync_apply);
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
    return (net_player_info[plyr_idx].network_user_active != 0);
}

const char *network_player_name(int plyr_idx)
{
    if ((plyr_idx < 0) || (plyr_idx >= NET_PLAYERS_COUNT))
        return NULL;
    return net_player_info[plyr_idx].name;
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
