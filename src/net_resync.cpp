/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_resync.cpp
 *     Network resynchronization for Dungeon Keeper multiplayer.
 * @par Purpose:
 *     Network resynchronization routines for multiplayer games.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     31 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_resync.h"
#include "bflib_datetm.h"
#include "bflib_enet.h"
#include "net_exchange_gameplay.h"
#include "net_exchange_common.h"
#include "net_main.h"
#include <zlib.h>
#include "globals.h"
#include "frontend.h"
#include "frontmenu_ingame_map.h"
#include "player_data.h"
#include "net_game.h"
#include "net_lobby.h"
#include "game_legacy.h"
#include "lens_api.h"
#include "lua_base.h"
#include "net_input_lag.h"
#include "net_checksums.h"
#include "net_spectator.h"
#include "ariadne_update.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Boing {
  unsigned char active_panel_menu_index;
  unsigned char comp_player_aggressive;
  unsigned char comp_player_defensive;
  unsigned char comp_player_construct;
  unsigned char comp_player_creatrsonly;
  unsigned char creatures_tend_imprison;
  unsigned char creatures_tend_flee;
  unsigned short hand_over_subtile_x;
  unsigned short hand_over_subtile_y;
  unsigned long chosen_room_kind;
  unsigned long chosen_room_spridx;
  unsigned long chosen_room_tooltip;
  unsigned long chosen_spell_type;
  unsigned long chosen_spell_spridx;
  unsigned long chosen_spell_tooltip;
  unsigned long manufactr_element;
  unsigned long manufactr_spridx;
  unsigned long manufactr_tooltip;
  struct GuiMessage messages[GUI_MESSAGES_COUNT];
  unsigned char active_messages_count;
};

static struct Boing boing;

TbBool detailed_multiplayer_logging = false;

#define RESYNC_RECEIVE_TIMEOUT_MS 30000

struct ResyncHeader {
    unsigned char message_type;
    uint32_t compressed_length;
    uint32_t original_length;
    uint32_t data_checksum;
    uint32_t gameplay_generation;
};

// function to intentionally desync the game state for testing purposes
void intentional_desync()
{
    if (!network_is_active() || network_is_host()) {
        return;
    }
    struct Room* start_rooms = &game.rooms[1];
    struct Room* end_rooms = &game.rooms[ROOMS_COUNT];

    for (struct Room* room = start_rooms; room < end_rooms; room += 1) {
        if (room_exists(room)) {
            room->slabs_count += 1;
            break;
        }
    }
    int i = game.thing_lists[TngList_Creatures].index;
    if (i != 0) {
        struct Thing* thing = thing_get(i);
        if (!thing_is_invalid(thing)) {
            thing->health += 1;
        }
    }
    get_player(0)->instance_remain_turns += 1;
}

void animate_resync_progress_bar(int current_phase, int total_phases)
{
    if (get_gameturn() == 0) {
        return;
    }
    if ((game.operation_flags & GOF_Paused) != 0) {
        return;
    }
    const int32_t max_progress = 32 * units_per_pixel / 16;
    const int32_t progress_pixels = (int32_t)((double)max_progress * current_phase / total_phases);
    draw_out_of_sync_box(progress_pixels, max_progress, status_panel_width);
}

void store_localised_game_structure(void)
{
    boing.active_panel_menu_index = game.active_panel_mnu_idx;
    boing.comp_player_aggressive = game.comp_player_aggressive;
    boing.comp_player_defensive = game.comp_player_defensive;
    boing.comp_player_construct = game.comp_player_construct;
    boing.comp_player_creatrsonly = game.comp_player_creatrsonly;
    boing.creatures_tend_imprison = game.creatures_tend_imprison;
    boing.creatures_tend_flee = game.creatures_tend_flee;
    boing.hand_over_subtile_x = game.hand_over_subtile_x;
    boing.hand_over_subtile_y = game.hand_over_subtile_y;
    boing.chosen_room_kind = game.chosen_room_kind;
    boing.chosen_room_spridx = game.chosen_room_spridx;
    boing.chosen_room_tooltip = game.chosen_room_tooltip;
    boing.chosen_spell_type = game.chosen_spell_type;
    boing.chosen_spell_spridx = game.chosen_spell_spridx;
    boing.chosen_spell_tooltip = game.chosen_spell_tooltip;
    boing.manufactr_element = game.manufactr_element;
    boing.manufactr_spridx = game.manufactr_spridx;
    boing.manufactr_tooltip = game.manufactr_tooltip;
    memcpy(boing.messages, game.messages, sizeof(boing.messages));
    boing.active_messages_count = game.active_messages_count;
}

void recall_localised_game_structure(void)
{
    game.active_panel_mnu_idx = boing.active_panel_menu_index;
    game.comp_player_aggressive = boing.comp_player_aggressive;
    game.comp_player_defensive = boing.comp_player_defensive;
    game.comp_player_construct = boing.comp_player_construct;
    game.comp_player_creatrsonly = boing.comp_player_creatrsonly;
    game.creatures_tend_imprison = boing.creatures_tend_imprison;
    game.creatures_tend_flee = boing.creatures_tend_flee;
    game.hand_over_subtile_x = boing.hand_over_subtile_x;
    game.hand_over_subtile_y = boing.hand_over_subtile_y;
    game.chosen_room_kind = boing.chosen_room_kind;
    game.chosen_room_spridx = boing.chosen_room_spridx;
    game.chosen_room_tooltip = boing.chosen_room_tooltip;
    game.chosen_spell_type = boing.chosen_spell_type;
    game.chosen_spell_spridx = boing.chosen_spell_spridx;
    game.chosen_spell_tooltip = boing.chosen_spell_tooltip;
    game.manufactr_element = boing.manufactr_element;
    game.manufactr_spridx = boing.manufactr_spridx;
    game.manufactr_tooltip = boing.manufactr_tooltip;
    memcpy(game.messages, boing.messages, sizeof(game.messages));
    game.active_messages_count = boing.active_messages_count;
}

// resync message: ResyncHeader, then the zlib-compressed data
static char *encode_resync_message(const void * buffer, size_t total_length, size_t *message_size)
{
    TbClockMSec start_time = LbTimerClock();
    if (total_length > UINT32_MAX) {
        ERRORLOG("Resync data too large");
        return NULL;
    }

    uLongf compressed_size = compressBound(total_length);
    if (compressed_size > UINT32_MAX - sizeof(ResyncHeader)) {
        ERRORLOG("Compressed resync data too large");
        return NULL;
    }

    char * message_buffer = (char *) malloc(sizeof(ResyncHeader) + compressed_size);
    if (message_buffer == NULL) {
        ERRORLOG("Failed to allocate message buffer");
        return NULL;
    }

    int compress_result = compress2((Bytef *)(message_buffer + sizeof(ResyncHeader)), &compressed_size, (const Bytef *)buffer, total_length, Z_BEST_SPEED);
    if (compress_result != Z_OK) {
        ERRORLOG("Compression failed: zlib error %d", compress_result);
        free(message_buffer);
        return NULL;
    }

    uLong data_crc = crc32(0L, Z_NULL, 0);
    data_crc = crc32(data_crc, (const Bytef *)buffer, total_length);
    NETLOG("Compression successful: %u -> %u bytes in %u ms", (uint32_t)total_length, (uint32_t)compressed_size, (uint32_t)(LbTimerClock() - start_time));

    ResyncHeader header;
    memset(&header, 0, sizeof(header));
    header.message_type = NETMSG_RESYNC_DATA;
    header.compressed_length = (uint32_t)compressed_size;
    header.original_length = (uint32_t)total_length;
    header.data_checksum = (uint32_t)data_crc;
    header.gameplay_generation = netstate.gameplay_generation;
    memcpy(message_buffer, &header, sizeof(ResyncHeader));
    *message_size = sizeof(ResyncHeader) + compressed_size;
    return message_buffer;
}

static void send_resync_message(const char * message_buffer, size_t message_size)
{
    NETLOG("Host: Sending resync data to all clients");
    for (NetUserId user_index = 0; user_index < MAX_NET_CONNECTIONS; ++user_index) {
        if (user_index == SERVER_ID || netstate.users[user_index].progress != USER_LOGGEDIN || (user_index >= MAX_NET_USERS && netstate.users[user_index].spectator_state != NetSpectator_Streaming)) {
            continue;
        }
        netstate.sp->sendmsg_single(netstate.users[user_index].id, message_buffer, message_size);
    }
}

static TbBool send_resync_data(const void * buffer, size_t total_length)
{
    size_t message_size;
    char * message_buffer = encode_resync_message(buffer, total_length, &message_size);
    if (message_buffer == NULL) {
        return false;
    }
    send_resync_message(message_buffer, message_size);
    free(message_buffer);
    return true;
}

static TbBool decode_resync_message(const char * message_buffer, size_t message_size, size_t expected_length, char ** data_buffer, size_t * data_length)
{
    if (message_size < sizeof(ResyncHeader)) {
        ERRORLOG("Resync message too small: %u bytes", (uint32_t)message_size);
        return false;
    }
    ResyncHeader header;
    memcpy(&header, message_buffer, sizeof(ResyncHeader));
    if (header.message_type != NETMSG_RESYNC_DATA) {
        ERRORLOG("Resync message has wrong type: %d", header.message_type);
        return false;
    }
    if (header.compressed_length != message_size - sizeof(ResyncHeader)) {
        ERRORLOG("Resync message size mismatch: %u != %u",
            (uint32_t)(message_size - sizeof(ResyncHeader)), header.compressed_length);
        return false;
    }
    if (expected_length != 0 && header.original_length != expected_length) {
        ERRORLOG("Resync data with wrong size: %u != %u", header.original_length, (uint32_t)expected_length);
        return false;
    }

    size_t output_size = header.original_length;
    if (output_size > sizeof(game) + 16 * 1024 * 1024) {
        return false;
    }
    if (output_size == 0) {
        output_size = 1;
    }
    char * output_buffer = (char *) malloc(output_size);
    if (output_buffer == NULL) {
        ERRORLOG("Failed to allocate resync destination buffer");
        return false;
    }

    uLongf dest_len = output_size;
    int uncompress_result = uncompress((Bytef *)output_buffer, &dest_len,
        (const Bytef *)(message_buffer + sizeof(ResyncHeader)), header.compressed_length);
    if (uncompress_result != Z_OK || dest_len != header.original_length) {
        ERRORLOG("Decompression failed: zlib error %d, expected %u bytes, got %u bytes",
            uncompress_result, header.original_length, (uint32_t)dest_len);
        free(output_buffer);
        return false;
    }

    uLong verify_crc = crc32(0L, Z_NULL, 0);
    verify_crc = crc32(verify_crc, (const Bytef *)output_buffer, header.original_length);
    if ((uint32_t)verify_crc != header.data_checksum) {
        ERRORLOG("Resync data checksum mismatch");
        free(output_buffer);
        return false;
    }

    *data_buffer = output_buffer;
    *data_length = header.original_length;
    return true;
}

static TbBool receive_resync_message(char **message_out, size_t *message_size_out)
{
    netstate.resync_pending = false;
    NETLOG("Starting to receive resync data");

    TbClockMSec start_time = LbTimerClock();
    TbClockMSec last_progress_time = start_time;
    int32_t last_progress = 0;
    while (LbTimerClock() - start_time < RESYNC_RECEIVE_TIMEOUT_MS) {
        netstate.sp->update(OnNewUser);
        int32_t progress = GetResyncProgress() * 80 / 100;
        if (progress > last_progress && LbTimerClock() - last_progress_time >= 50) {
            animate_resync_progress_bar(progress, 100);
            last_progress = progress;
            last_progress_time = LbTimerClock();
        }
        size_t received_size = netstate.sp->msgready(SERVER_ID, 10);
        if (received_size == 0) {
            continue;
        }

        if (received_size > 16 * 1024 * 1024) {
            return false;
        }
        size_t expected_size = received_size;
        char *message_buffer = (char *)malloc(received_size);
        if (message_buffer == NULL) {
            ERRORLOG("Failed to allocate message buffer");
            return false;
        }

        received_size = netstate.sp->readmsg(SERVER_ID, message_buffer, received_size);
        if (received_size != expected_size) {
            free(message_buffer);
            return false;
        }
        if (received_size > 0 && received_size <= sizeof(netstate.msg_buffer) && message_buffer[0] == NETMSG_USERUPDATE) {
            memcpy(netstate.msg_buffer, message_buffer, received_size);
            netstate.msg_buffer_null = '\0';
            if (received_size < sizeof(netstate.msg_buffer)) {
                netstate.msg_buffer[received_size] = '\0';
            }
            process_user_update_message(SERVER_ID, netstate.msg_buffer + 1, netstate.msg_buffer + received_size);
            free(message_buffer);
            continue;
        }
        if (received_size < sizeof(ResyncHeader)) {
            if (received_size > 0 && message_buffer[0] == NETMSG_RESYNC_DATA) {
                ERRORLOG("Received incomplete resync data: %u bytes", (uint32_t)received_size);
            } else {
                ERRORLOG("Ignoring message too small for resync data: %u bytes", (uint32_t)received_size);
            }
            free(message_buffer);
            continue;
        }
        if (message_buffer[0] != NETMSG_RESYNC_DATA) {
            MULTIPLAYER_LOG("Received wrong message type: %d", message_buffer[0]);
            free(message_buffer);
            continue;
        }
        MULTIPLAYER_LOG("Client: Received resync message, %u bytes", (uint32_t)received_size);
        NETLOG("Resync transfer received: %u bytes in %u ms", (uint32_t)received_size, (uint32_t)(LbTimerClock() - start_time));
        animate_resync_progress_bar(80, 100);
        *message_out = message_buffer;
        *message_size_out = received_size;
        return true;
    }

    ERRORLOG("Client: Timeout waiting for resync data after %dms", RESYNC_RECEIVE_TIMEOUT_MS);
    return false;
}

static TbBool receive_resync_data(char ** data_buffer, size_t * data_length)
{
    char * message_buffer = NULL;
    size_t message_size = 0;
    if (!receive_resync_message(&message_buffer, &message_size)) {
        return false;
    }
    TbBool result = decode_resync_message(message_buffer, message_size, *data_length, data_buffer, data_length);
    free(message_buffer);
    if (result) {
        NETLOG("Client: Resync data received successfully");
    }
    return result;
}

TbBool LbNetwork_Resync(void * data_buffer, size_t buffer_length)
{
    MULTIPLAYER_LOG("Starting resync, my_id=%d, buffer_length=%u", netstate.my_id, (uint32_t)buffer_length);

    TbBool result;
    if (network_is_host()) {
        MULTIPLAYER_LOG("Resync: I am the server");
        result = send_resync_data(data_buffer, buffer_length);
    } else {
        MULTIPLAYER_LOG("Resync: I am a client, receiving");
        char * received_data = NULL;
        size_t received_length = buffer_length;
        result = receive_resync_data(&received_data, &received_length);
        if (result) {
            memcpy(data_buffer, received_data, buffer_length);
        }
        free(received_data);
    }

    if (!result) {
        ERRORLOG("Resync FAILED");
        return false;
    }

    MULTIPLAYER_LOG("Resync data transfer completed successfully");
    return true;
}

static char *build_resync_game_data(size_t *full_resync_len)
{
    const char * lua_data = "";
    size_t lua_data_len = 0;
    if (Lvl_script != NULL) {
        lua_data = lua_get_serialised_data(&lua_data_len);
        if (lua_data == NULL) {
            cleanup_serialized_data();
            return NULL;
        }
    }
    size_t lua_data_offset = sizeof(game) + sizeof(uint32_t);
    size_t navigation_size = transfer_navigation_state(NULL, NavigationState_Store);
    if (lua_data_len > UINT32_MAX - lua_data_offset - navigation_size) {
        ERRORLOG("Full resync data too large");
        cleanup_serialized_data();
        return NULL;
    }

    *full_resync_len = lua_data_offset + lua_data_len + navigation_size;
    char * full_resync_data = (char *) malloc(*full_resync_len);
    if (full_resync_data == NULL) {
        ERRORLOG("Failed to allocate full resync buffer");
        cleanup_serialized_data();
        return NULL;
    }

    uint32_t lua_data_len32 = (uint32_t)lua_data_len;
    memcpy(full_resync_data, &game, sizeof(game));
    memcpy(full_resync_data + sizeof(game), &lua_data_len32, sizeof(lua_data_len32));
    memcpy(full_resync_data + lua_data_offset, lua_data, lua_data_len);
    transfer_navigation_state(full_resync_data + lua_data_offset + lua_data_len, NavigationState_Store);
    cleanup_serialized_data();
    return full_resync_data;
}

static TbBool apply_resync_game_data(const char * full_resync_data, size_t full_resync_len)
{
    uint32_t lua_data_len = 0;
    size_t lua_data_offset = sizeof(game) + sizeof(lua_data_len);
    if (full_resync_len < lua_data_offset) {
        ERRORLOG("Full resync data too small: %u bytes", (uint32_t)full_resync_len);
        return false;
    }
    memcpy(&lua_data_len, full_resync_data + sizeof(game), sizeof(lua_data_len));
    if (lua_data_len > full_resync_len - lua_data_offset) {
        ERRORLOG("Received lua data with wrong size: %u != %u", lua_data_len, (uint32_t)(full_resync_len - lua_data_offset));
        return false;
    }
    size_t navigation_size = full_resync_len - lua_data_offset - lua_data_len;
    if (navigation_size != 0 && navigation_size != transfer_navigation_state(NULL, NavigationState_Store)) {
        ERRORLOG("Received navigation data with wrong size: %u", (uint32_t)navigation_size);
        return false;
    }
    if (Lvl_script == NULL && lua_data_len > 0) {
        ERRORLOG("Lua state is not initialized");
        return false;
    }
    if (Lvl_script != NULL && !lua_set_serialised_data(full_resync_data + lua_data_offset, lua_data_len)) {
        return false;
    }
    memcpy(&game, full_resync_data, sizeof(game));
    if (navigation_size != 0) {
        transfer_navigation_state((char *)full_resync_data + lua_data_offset + lua_data_len, NavigationState_Restore);
    } else {
        init_navigation();
    }
    return true;
}

static TbBool apply_resync_game_message(const char * message_buffer, size_t message_size)
{
    TbClockMSec start_time = LbTimerClock();
    char * full_resync_data = NULL;
    size_t full_resync_len = 0;
    if (!decode_resync_message(message_buffer, message_size, 0, &full_resync_data, &full_resync_len)) {
        return false;
    }
    TbBool result = apply_resync_game_data(full_resync_data, full_resync_len);
    free(full_resync_data);
    if (result) {
        ResyncHeader header;
        memcpy(&header, message_buffer, sizeof(header));
        netstate.gameplay_generation = header.gameplay_generation;
    }
    NETLOG("Resync decompression and state application took %u ms", (uint32_t)(LbTimerClock() - start_time));
    return result;
}

static char *create_resync_game_message(size_t *message_size)
{
    size_t full_resync_len = 0;
    char * full_resync_data = build_resync_game_data(&full_resync_len);
    if (full_resync_data == NULL) {
        return NULL;
    }
    char *message_buffer = encode_resync_message(full_resync_data, full_resync_len, message_size);
    free(full_resync_data);
    return message_buffer;
}

TbBool send_spectator_resync(uint64_t user_mask)
{
    size_t message_size = 0;
    char *message_buffer = create_resync_game_message(&message_size);
    if (message_buffer == NULL) {
        return false;
    }
    send_to_active_peers(1, NetSend_Reliable, message_buffer, message_size, user_mask);
    free(message_buffer);
    return true;
}

TbBool send_resync_game(void)
{
    network_spectator_flush_turns(1);
    pack_desync_history_for_resync();
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 100);
    NETLOG("Initiating re-synchronization of network game");
    take_game_timestamp();

    size_t message_size = 0;
    netstate.gameplay_generation += 1;
    char *message_buffer = create_resync_game_message(&message_size);
    if (message_buffer == NULL) {
        netstate.gameplay_generation -= 1;
        return false;
    }
    send_resync_message(message_buffer, message_size);
    replay_record_resync(message_buffer, message_size);
    free(message_buffer);
    animate_resync_progress_bar(90, 100);
    NETLOG("Host: Resync data queued");
    return true;
}

TbBool receive_resync_game(void)
{
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 100);
    NETLOG("Initiating re-synchronization of network game");
    char * message_buffer = NULL;
    size_t message_size = 0;
    if (!receive_resync_message(&message_buffer, &message_size)) {
        return false;
    }
    TbBool result = apply_resync_game_message(message_buffer, message_size);
    if (result) {
        replay_record_resync(message_buffer, message_size);
    }
    free(message_buffer);
    if (!result) {
        return false;
    }

    animate_resync_progress_bar(90, 100);

    compare_desync_history_from_host();

    return true;
}

static void finish_resync(const struct Packet *saved_packets)
{
    if (Lvl_script != NULL) {
        lua_set_random_seed(game.action_random_seed);
    }
    recall_localised_game_structure();
    rebuild_net_user_player_numbers();
    reinit_level_after_load();
    if (network_is_active() && net_join_role == NetRole_Spectator) {
        reinitialise_eye_lens(game.applied_lens_type);
    }
    memcpy(game.packets, saved_packets, sizeof(game.packets));

    if (!network_is_active() || !network_user_is_spectator(netstate.my_id)) {
        game.skip_initial_input_turns = calculate_skip_input();
    }
    initialize_packet_history();
    NETLOG("Input lag after resync: %d turns", game.input_lag_turns);

    clear_flag(local_system_flags, GSF_NetGameNoSync);
    clear_flag(local_system_flags, GSF_NetSeedNoSync);
    panel_map_update(0, 0, game.map_subtiles_x + 1, game.map_subtiles_y + 1);
}

void resync_game(void)
{
    TbClockMSec start_time = LbTimerClock();
    SYNCDBG(2,"Starting");
    draw_out_of_sync_box(0, 32*units_per_pixel/16, local_state.engine_window_x);
    reset_eye_lenses();
    store_localised_game_structure();
    struct Packet saved_packets[PACKETS_COUNT];
    memcpy(saved_packets, game.packets, sizeof(saved_packets));
    TbBool result;
    if (network_is_host()) {
        result = send_resync_game();
    } else {
        result = receive_resync_game();
    }
    if (!result) {
        recall_localised_game_structure();
        return;
    }
    finish_resync(saved_packets);
    wait_for_all_players();
    animate_resync_progress_bar(100, 100);
    NETLOG("Resync finished locally in %u ms", (uint32_t)(LbTimerClock() - start_time));
}

TbBool apply_recorded_resync(const char * message_buffer, size_t message_size)
{
    reset_eye_lenses();
    store_localised_game_structure();
    struct Packet saved_packets[PACKETS_COUNT];
    memcpy(saved_packets, game.packets, sizeof(saved_packets));
    if (!apply_resync_game_message(message_buffer, message_size)) {
        recall_localised_game_structure();
        return false;
    }
    finish_resync(saved_packets);
    if (network_is_active() && network_user_is_spectator(netstate.my_id)) {
        network_spectator_reset();
        network_spectator_resync_pending = true;
    }
    return true;
}

#ifdef __cplusplus
}
#endif
