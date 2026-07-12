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
#include "net_exchange_gameplay.h"
#include "net_main.h"
#include <zlib.h>
#include "globals.h"
#include "frontend.h"
#include "player_data.h"
#include "net_game.h"
#include "game_legacy.h"
#include "lens_api.h"
#include "lua_base.h"
#include "net_input_lag.h"
#include "net_checksums.h"
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
};

static struct Boing boing;

TbBool detailed_multiplayer_logging = false;

#define RESYNC_RECEIVE_TIMEOUT_MS 30000

struct ResyncHeader {
    unsigned char message_type;
    uint32_t compressed_length;
    uint32_t original_length;
    uint32_t data_checksum;
};


// function to intentionally desync the game state for testing purposes
void intentional_desync() {
    if (!is_my_player_number(0)) {
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

void animate_resync_progress_bar(int current_phase, int total_phases) {
    if (get_gameturn() == 0) {
        return;
    }
    if ((game.operation_flags & GOF_Paused) != 0) {
        return;
    }
    const long max_progress = (long)(32 * units_per_pixel / 16);
    const long progress_pixels = (long)((double)max_progress * current_phase / total_phases);
    draw_out_of_sync_box(progress_pixels, max_progress, status_panel_width);
}

void store_localised_game_structure(void) {
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
}

void recall_localised_game_structure(void) {
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
}

static TbBool send_resync_data(const void * buffer, size_t total_length)
{
    if (total_length > UINT32_MAX) {
        ERRORLOG("Resync data too large");
        return false;
    }

    uLongf compressed_size = compressBound(total_length);
    if (compressed_size > UINT32_MAX - sizeof(ResyncHeader)) {
        ERRORLOG("Compressed resync data too large");
        return false;
    }

    size_t message_size = sizeof(ResyncHeader) + compressed_size;
    char * message_buffer = (char *) malloc(message_size);
    if (message_buffer == NULL) {
        ERRORLOG("Failed to allocate message buffer");
        return false;
    }

    int compress_result = compress((Bytef *)(message_buffer + sizeof(ResyncHeader)), &compressed_size, (const Bytef *)buffer, total_length);
    if (compress_result != Z_OK) {
        ERRORLOG("Compression failed: zlib error %d", compress_result);
        free(message_buffer);
        return false;
    }

    uLong data_crc = crc32(0L, Z_NULL, 0);
    data_crc = crc32(data_crc, (const Bytef *)buffer, total_length);
    NETLOG("Compression successful: %u -> %u bytes", (uint32_t)total_length, (uint32_t)compressed_size);

    ResyncHeader header;
    memset(&header, 0, sizeof(header));
    header.message_type = NETMSG_RESYNC_DATA;
    header.compressed_length = (uint32_t)compressed_size;
    header.original_length = (uint32_t)total_length;
    header.data_checksum = (uint32_t)data_crc;

    message_size = sizeof(ResyncHeader) + compressed_size;
    memcpy(message_buffer, &header, sizeof(ResyncHeader));

    NETLOG("Host: Sending resync data to all clients");
    for (NetUserId user_index = 0; user_index < MAX_NET_USERS; ++user_index) {
        if (netstate.users[user_index].progress != USER_LOGGEDIN) {
            continue;
        }
        netstate.sp->sendmsg_single(netstate.users[user_index].id, message_buffer, message_size);
    }

    free(message_buffer);
    return true;
}

static TbBool receive_resync_data(char ** data_buffer, size_t * data_length)
{
    NETLOG("Starting to receive resync data");

    TbClockMSec start_time = LbTimerClock();
    while (LbTimerClock() - start_time < RESYNC_RECEIVE_TIMEOUT_MS) {
        netstate.sp->update(OnNewUser);
        size_t received_size = netstate.sp->msgready(SERVER_ID, 0);
        if (received_size == 0) {
            continue;
        }

        char * message_buffer = (char *) malloc(received_size);
        if (message_buffer == NULL) {
            ERRORLOG("Failed to allocate message buffer");
            return false;
        }

        received_size = netstate.sp->readmsg(SERVER_ID, message_buffer, received_size);
        if (received_size < sizeof(ResyncHeader)) {
            ERRORLOG("Received message too small: %u bytes", (uint32_t)received_size);
            free(message_buffer);
            continue;
        }

        ResyncHeader header;
        memcpy(&header, message_buffer, sizeof(ResyncHeader));

        if (header.message_type != NETMSG_RESYNC_DATA) {
            MULTIPLAYER_LOG("Received wrong message type: %d", header.message_type);
            free(message_buffer);
            continue;
        }

        if (header.compressed_length != received_size - sizeof(ResyncHeader)) {
            ERRORLOG("Received message size mismatch: %u != %u",
                (uint32_t)(received_size - sizeof(ResyncHeader)), header.compressed_length);
            free(message_buffer);
            continue;
        }
        if (*data_length != 0 && header.original_length != *data_length) {
            ERRORLOG("Received data with wrong size: %u != %u", header.original_length, (uint32_t)*data_length);
            free(message_buffer);
            continue;
        }

        size_t output_size = header.original_length;
        if (output_size == 0) {
            output_size = 1;
        }
        char * output_buffer = (char *) malloc(output_size);
        if (output_buffer == NULL) {
            ERRORLOG("Failed to allocate resync destination buffer");
            free(message_buffer);
            return false;
        }

        uLongf dest_len = output_size;

        MULTIPLAYER_LOG("Client: Received resync message, decompressing %u bytes", header.compressed_length);

        int uncompress_result = uncompress((Bytef *)output_buffer, &dest_len,
            (const Bytef *)(message_buffer + sizeof(ResyncHeader)), header.compressed_length);
        if (uncompress_result != Z_OK || dest_len != header.original_length) {
            ERRORLOG("Decompression failed: zlib error %d, expected %u bytes, got %u bytes",
                uncompress_result, header.original_length, (uint32_t)dest_len);
            free(output_buffer);
            free(message_buffer);
            return false;
        }

        uLong verify_crc = crc32(0L, Z_NULL, 0);
        verify_crc = crc32(verify_crc, (const Bytef *)output_buffer, header.original_length);
        if ((uint32_t)verify_crc != header.data_checksum) {
            ERRORLOG("Resync data checksum mismatch");
            free(output_buffer);
            free(message_buffer);
            return false;
        }

        *data_buffer = output_buffer;
        *data_length = header.original_length;
        free(message_buffer);
        NETLOG("Client: Resync data received successfully");
        return true;
    }

    ERRORLOG("Client: Timeout waiting for resync data after %dms", RESYNC_RECEIVE_TIMEOUT_MS);
    return false;
}

TbBool LbNetwork_Resync(void * data_buffer, size_t buffer_length)
{
    MULTIPLAYER_LOG("Starting resync, my_id=%d, buffer_length=%u", netstate.my_id, (uint32_t)buffer_length);

    TbBool result;
    if (my_player_number == get_host_player_id()) {
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

TbBool send_resync_game(void)
{
    pack_desync_history_for_resync();
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 6);
    NETLOG("Initiating re-synchronization of network game");

    const char * lua_data = "";
    size_t lua_data_len = 0;
    if (Lvl_script != NULL) {
        lua_data = lua_get_serialised_data(&lua_data_len);
        if (lua_data == NULL) {
            cleanup_serialized_data();
            return false;
        }
    }
    size_t lua_data_offset = sizeof(game) + sizeof(uint32_t);
    if (lua_data_len > UINT32_MAX - lua_data_offset) {
        ERRORLOG("Full resync data too large");
        cleanup_serialized_data();
        return false;
    }

    size_t full_resync_len = lua_data_offset + lua_data_len;
    char * full_resync_data = (char *) malloc(full_resync_len);
    if (full_resync_data == NULL) {
        ERRORLOG("Failed to allocate full resync buffer");
        cleanup_serialized_data();
        return false;
    }

    uint32_t lua_data_len32 = (uint32_t)lua_data_len;
    memcpy(full_resync_data, &game, sizeof(game));
    memcpy(full_resync_data + sizeof(game), &lua_data_len32, sizeof(lua_data_len32));
    memcpy(full_resync_data + lua_data_offset, lua_data, lua_data_len);
    TbBool result = send_resync_data(full_resync_data, full_resync_len);
    free(full_resync_data);
    cleanup_serialized_data();
    if (!result) {
        return false;
    }
    animate_resync_progress_bar(2, 6);
    animate_resync_progress_bar(6, 6);
    NETLOG("Host: Resync complete");
    return true;
}

TbBool receive_resync_game(void)
{
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 6);
    NETLOG("Initiating re-synchronization of network game");
    char * full_resync_data = NULL;
    size_t full_resync_len = 0;
    uint32_t lua_data_len = 0;
    size_t lua_data_offset = sizeof(game) + sizeof(lua_data_len);

    if (!receive_resync_data(&full_resync_data, &full_resync_len)) {
        return false;
    }

    if (full_resync_len < lua_data_offset) {
        ERRORLOG("Full resync data too small: %u bytes", (uint32_t)full_resync_len);
        free(full_resync_data);
        return false;
    }

    memcpy(&lua_data_len, full_resync_data + sizeof(game), sizeof(lua_data_len));
    if (lua_data_len != full_resync_len - lua_data_offset) {
        ERRORLOG("Received lua data with wrong size: %u != %u", lua_data_len, (uint32_t)(full_resync_len - lua_data_offset));
        free(full_resync_data);
        return false;
    }

    if (Lvl_script == NULL && lua_data_len > 0) {
        ERRORLOG("Lua state is not initialized");
        free(full_resync_data);
        return false;
    }

    if (Lvl_script != NULL && !lua_set_serialised_data(full_resync_data + lua_data_offset, lua_data_len)) {
        free(full_resync_data);
        return false;
    }

    memcpy(&game, full_resync_data, sizeof(game));
    free(full_resync_data);

    animate_resync_progress_bar(2, 6);
    animate_resync_progress_bar(6, 6);
    NETLOG("Client: Resync complete");

    compare_desync_history_from_host();

    return true;
}


void resync_game(void)
{
    SYNCDBG(2,"Starting");
    struct PlayerInfo* player = get_my_player();
    draw_out_of_sync_box(0, 32*units_per_pixel/16, player->engine_window_x);
    reset_eye_lenses();
    store_localised_game_structure();
    TbBool result;
    if (my_player_number == get_host_player_id()) {
        result = send_resync_game();
    } else {
        result = receive_resync_game();
    }
    if (!result) {
        recall_localised_game_structure();
        return;
    }
    if (Lvl_script != NULL) {
        lua_set_random_seed(game.action_random_seed);
    }
    recall_localised_game_structure();
    reinit_level_after_load();

    game.skip_initial_input_turns = calculate_skip_input();
    initialize_packet_history();
    NETLOG("Input lag after resync: %d turns", game.input_lag_turns);

    clear_flag(game.system_flags, GSF_NetGameNoSync);
    clear_flag(game.system_flags, GSF_NetSeedNoSync);
}

#ifdef __cplusplus
}
#endif
