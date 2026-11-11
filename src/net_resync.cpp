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
#include <algorithm>
#include <vector>
#include "globals.h"
#include "frontend.h"
#include "frontmenu_ingame_map.h"
#include "player_data.h"
#include "net_game.h"
#include "net_lobby.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "lens_api.h"
#include "light_data.h"
#include "lua_base.h"
#include "net_input_lag.h"
#include "net_checksum_turn.h"
#include "net_spectator.h"
#include "ariadne_update.h"
#include "keeperfx.hpp"
#include "kjm_input.h"
#include "kfx/save/core/save_resync.h"
#include "kfx/save/core/save_config_overlay.h"
#include "replay.h"
#include <SDL3/SDL.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Boing {
  unsigned char active_panel_menu_index;
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
static TbBool resync_progress_visible;

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
    get_user_state(0)->instance_remain_turns += 1;
}

void animate_resync_progress_bar(int current_phase, int total_phases)
{
    if (!resync_progress_visible || get_gameturn() == 0) {
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
char *encode_resync_message(const void * buffer, size_t total_length, size_t *message_size)
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
    while (LbTimerClock() - start_time < RESYNC_RECEIVE_TIMEOUT_MS && netstate.users[SERVER_ID].progress != USER_UNUSED) {
        netstate.sp->update(OnNewUser);
        if (!poll_inputs()) {
            exit_keeper = 1;
            return false;
        }
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
        if (received_size > 0 && message_buffer[0] == NETMSG_RESYNC_HASHES) {
            // the host asks while this client already waits for the resync (a player taking a seat)
            process_resync_hashes_message(SERVER_ID, message_buffer + 1, received_size - 1);
            free(message_buffer);
            continue;
        }
        if (received_size < sizeof(ResyncHeader)) {
            if (received_size > 0 && (message_buffer[0] == NETMSG_RESYNC_DATA || message_buffer[0] == NETMSG_RESYNC_DELTA)) {
                ERRORLOG("Received incomplete resync data: %u bytes", (uint32_t)received_size);
            } else {
                ERRORLOG("Ignoring message too small for resync data: %u bytes", (uint32_t)received_size);
            }
            free(message_buffer);
            continue;
        }
        if (message_buffer[0] != NETMSG_RESYNC_DATA && message_buffer[0] != NETMSG_RESYNC_DELTA) {
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

    ERRORLOG("Client: Gave up waiting for resync data after %dms", (int32_t)(LbTimerClock() - start_time));
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

static TbBool encode_resync_config(struct SaveBuffer *conf_buf)
{
    if (conf_baseline == NULL) {
        return save_buf_u32(conf_buf, 0) == 0;
    }
    struct SaveError save_err;
    if (save_config_overlay_write(&game.conf, conf_baseline, conf_buf, &save_err) != SVR_Ok) {
        ERRORLOG("Resync config encode failed: %s", save_err.message);
        return false;
    }
    return true;
}

static TbBool apply_resync_config(const char *conf_data, uint32_t conf_len)
{
    if (conf_baseline != NULL) {
        memcpy(&game.conf, conf_baseline, sizeof(game.conf));
    }
    struct SaveError save_err;
    if (save_config_overlay_apply((const uint8_t *)conf_data, conf_len, &game.conf, &save_err) != SVR_Ok) {
        ERRORLOG("Resync config apply failed: %s", save_err.message);
        return false;
    }
    return true;
}

// The full resync data: [u32 len][game] [u32 len][intralevel] [u32 len][config] [u32 len][lua] [u32 len][custom intralevel] [navigation]
enum ResyncSection {
    RSec_Game,
    RSec_Intralevel,
    RSec_Config,
    RSec_Lua,
    RSec_CustomIntralevel,
    RSec_COUNT
};

char *build_resync_game_data(size_t *full_resync_len)
{
    // Light system state lives outside Game; the same as for a savegame
    light_export_system_state(&game.lightst);

    // Field-table encode, not a raw memcpy: lets host and client resync even when
    // their struct Game layouts differ (mixed Windows/Linux, x86/x64 lobbies).
    SaveBufferOwner game_buf;
    SaveBufferOwner ilvl_buf;
    struct SaveError save_err;
    if ((save_resync_encode_game(&game_buf, &game, &save_err) != SVR_Ok)
     || (save_resync_encode_intralevel(&ilvl_buf, &intralvl, &save_err) != SVR_Ok)) {
        ERRORLOG("Resync game encode failed: %s", save_err.message);
        return NULL;
    }
    SaveBufferOwner conf_buf;
    if (!encode_resync_config(&conf_buf)) {
        return NULL;
    }

    char *lua_data = NULL;
    size_t lua_data_len = 0;
    if (Lvl_script != NULL) {
        lua_data = lua_get_serialised_data(&lua_data_len);
        if (lua_data == NULL) {
            return NULL;
        }
    }
    const void *section_data[RSec_COUNT];
    size_t section_len[RSec_COUNT];
    section_data[RSec_Game] = game_buf.data;
    section_len[RSec_Game] = game_buf.len;
    section_data[RSec_Intralevel] = ilvl_buf.data;
    section_len[RSec_Intralevel] = ilvl_buf.len;
    section_data[RSec_Config] = conf_buf.data;
    section_len[RSec_Config] = conf_buf.len;
    section_data[RSec_Lua] = lua_data;
    section_len[RSec_Lua] = lua_data_len;
    section_data[RSec_CustomIntralevel] = custom_intralevel_data;
    section_len[RSec_CustomIntralevel] = custom_intralevel_data_len;
    size_t navigation_size = transfer_navigation_state(NULL, NavigationState_Store);
    size_t total_len = navigation_size;
    for (int i = 0; i < RSec_COUNT; i++) {
        if (section_len[i] > UINT32_MAX - sizeof(uint32_t) - total_len) {
            ERRORLOG("Full resync data too large");
            free(lua_data);
            return NULL;
        }
        total_len += sizeof(uint32_t) + section_len[i];
    }

    char * full_resync_data = (char *) malloc(total_len);
    if (full_resync_data == NULL) {
        ERRORLOG("Failed to allocate full resync buffer");
        free(lua_data);
        return NULL;
    }
    char *pos = full_resync_data;
    for (int i = 0; i < RSec_COUNT; i++) {
        uint32_t len32 = (uint32_t)section_len[i];
        memcpy(pos, &len32, sizeof(len32));
        pos += sizeof(len32);
        if (len32 > 0) {
            memcpy(pos, section_data[i], len32);
        }
        pos += len32;
    }
    transfer_navigation_state(pos, NavigationState_Store);
    free(lua_data);
    *full_resync_len = total_len;
    return full_resync_data;
}

static TbBool apply_resync_game_data(const char * full_resync_data, size_t full_resync_len)
{
    const char *section_data[RSec_COUNT];
    uint32_t section_len[RSec_COUNT];
    size_t offset = 0;
    for (int i = 0; i < RSec_COUNT; i++) {
        if (full_resync_len - offset < sizeof(uint32_t)) {
            ERRORLOG("Full resync data too small: %u bytes", (uint32_t)full_resync_len);
            return false;
        }
        memcpy(&section_len[i], full_resync_data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        if (section_len[i] > full_resync_len - offset) {
            ERRORLOG("Received resync section %d with wrong size: %u > %u", i, section_len[i], (uint32_t)(full_resync_len - offset));
            return false;
        }
        section_data[i] = full_resync_data + offset;
        offset += section_len[i];
    }
    size_t navigation_size = full_resync_len - offset;
    if (navigation_size != 0 && navigation_size != transfer_navigation_state(NULL, NavigationState_Store)) {
        ERRORLOG("Received navigation data with wrong size: %u", (uint32_t)navigation_size);
        return false;
    }
    if (Lvl_script == NULL && section_len[RSec_Lua] > 0) {
        ERRORLOG("Lua state is not initialized");
        return false;
    }
    if (Lvl_script != NULL && !lua_set_serialised_data(section_data[RSec_Lua], section_len[RSec_Lua])) {
        return false;
    }
    if (!set_custom_intralevel_data(section_data[RSec_CustomIntralevel], section_len[RSec_CustomIntralevel])) {
        ERRORLOG("Failed to allocate custom intralevel data: %u bytes", section_len[RSec_CustomIntralevel]);
        return false;
    }
    // Decoded in place: fields the table excludes from sync (local UI/per-player
    // state -- see save_resync_decode_game) are left exactly as they already are
    // in game, not zeroed.
    struct SaveError save_err;
    if (save_resync_decode_intralevel((const uint8_t *)section_data[RSec_Intralevel], section_len[RSec_Intralevel], &intralvl, &save_err) != SVR_Ok) {
        ERRORLOG("Resync intralevel decode failed: %s", save_err.message);
        return false;
    }
    if (save_resync_decode_game((const uint8_t *)section_data[RSec_Game], section_len[RSec_Game], &game, &save_err) != SVR_Ok) {
        ERRORLOG("Resync game decode failed: %s", save_err.message);
        return false;
    }
    if (!apply_resync_config(section_data[RSec_Config], section_len[RSec_Config])) {
        return false;
    }
    if (navigation_size != 0) {
        transfer_navigation_state((char *)full_resync_data + offset, NavigationState_Restore);
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

/******************************************************************************/
// Delta resync.
//
// When the host resyncs, it first asks the clients for a hash of each chunk of their own resync
// data (NETMSG_RESYNC_HASHES; the host's request carries only the generation). A client builds its
// data when the request comes, keeps it until the resync, and always answers: with its hashes, or
// with none if it cannot make them, which means "send me the full state". The host builds and
// splits its own data meanwhile and waits for the answers (at most DELTA_HASH_WAIT_MAX_MS). A
// client that sent hashes gets references to the chunks of the host's data it has and the bytes it
// lacks (NETMSG_RESYNC_DELTA); the others, spectators and replays get the full state. If the rebuilt
// data does not fit, the client asks once for the full state (NETMSG_RESYNC_FULL_REQUEST).
//
// The host builds its data at the end of process_packets, after it sent its packet for that turn;
// a client stops when it lacks the host's next packet, so its data is from up to 1 + input lag
// turns later. Those turns change some chunks too, but most still match.
//
// The resync data has no fixed layout (a union's stored bytes depend on the member in use), so
// one changed thing can shift everything after it. Chunks therefore end where the content says
// (a rolling hash), not at fixed offsets: a shift only changes the chunks around it. A 23 MB state
// gives about 750 chunks (the hashes take ~6 KB).
//
// A chunk is known by a 64-bit hash that also covers its length: with under a thousand chunks on
// each side, a chunk the client lacks is taken for one it has with a chance below 10^6 / 2^64.
// The rebuilt data is checked against a hash of the host's data with another seed, which does not
// follow from the chunk hashes (as a CRC of the whole would, CRC being linear), so even then the
// client asks for the full state rather than use a wrong one.

#define DELTA_CHUNK_BITS 13 // chunks of 2 to 32 KB, 8 KB plus a run of equal bytes on average
#define DELTA_MAX_CHUNKS ((NET_MSG_BUFFER_SIZE - 64) / sizeof(uint64_t))
#define DELTA_CHUNK_SEED 0
#define DELTA_STATE_SEED 0x6B66785F72657379ULL
#define DELTA_COPY_FLAG 0x80000000u
// A client answers once it has built and hashed its data (~100 ms); the limit covers a slow
// computer or link, and a client that does not answer.
#define DELTA_HASH_WAIT_MAX_MS 2000

#pragma pack(1)
struct ResyncHashesHeader {
    uint32_t generation;    // of the state the hashes are of
    uint32_t chunk_count;   // 0: no delta, send the full state
};

struct ResyncDeltaHeader {
    unsigned char message_type;
    uint32_t compressed_length;
    uint32_t payload_length;
    uint32_t full_length;
    uint64_t full_hash;     // delta_hash of the host's data, DELTA_STATE_SEED
    uint32_t gameplay_generation;   // after the resync: one more than that of the client's kept data
};
#pragma pack()

struct DeltaChunks {
    uint32_t *ends;     // end offset of each chunk
    uint64_t *hashes;
    uint32_t count;
};

// client: its resync data from when the host asked, and its chunks
static char *delta_base = NULL;
static uint32_t delta_base_generation = 0;
static struct DeltaChunks delta_base_chunks;
// host: the clients asked on this resync, those that answered, and the hashes they sent (or NULL)
static uint32_t delta_asked_users = 0;
static uint32_t delta_answered_users = 0;
static uint64_t *user_hashes[MAX_NET_USERS];
static uint32_t user_hash_count[MAX_NET_USERS];
// host: the last full resync message, for a client whose delta failed
static char *last_full_message = NULL;
static size_t last_full_message_size = 0;
static uint32_t last_full_generation = 0;
static TbBool last_full_sent[MAX_NET_USERS];

static inline uint64_t delta_rotl(uint64_t x, int r)
{
    return (x << r) | (x >> (64 - r));
}

static inline uint64_t delta_read64(const unsigned char *p)
{
    uint64_t v;
    memcpy(&v, p, sizeof(v)); // little-endian on every platform the game runs on
    return v;
}

#define DELTA_P1 0x9E3779B185EBCA87ULL
#define DELTA_P2 0xC2B2AE3D27D4EB4FULL
#define DELTA_P3 0x165667B19E3779F9ULL
#define DELTA_P4 0x85EBCA77C2B2AE63ULL
#define DELTA_P5 0x27D4EB2F165667C5ULL

static inline uint64_t delta_round(uint64_t acc, uint64_t input)
{
    return delta_rotl(acc + input * DELTA_P2, 31) * DELTA_P1;
}

/** XXH64: a fast 64-bit hash; the length is part of it. */
static uint64_t delta_hash(const void *data, size_t len, uint64_t seed)
{
    const unsigned char *p = (const unsigned char *)data;
    const unsigned char *end = p + len;
    uint64_t h;
    if (len >= 32) {
        uint64_t v[4] = { seed + DELTA_P1 + DELTA_P2, seed + DELTA_P2, seed, seed - DELTA_P1 };
        do {
            for (int i = 0; i < 4; i++) {
                v[i] = delta_round(v[i], delta_read64(p + 8 * i));
            }
            p += 32;
        } while (end - p >= 32);
        h = delta_rotl(v[0], 1) + delta_rotl(v[1], 7) + delta_rotl(v[2], 12) + delta_rotl(v[3], 18);
        for (int i = 0; i < 4; i++) {
            h = (h ^ delta_round(0, v[i])) * DELTA_P1 + DELTA_P4;
        }
    } else {
        h = seed + DELTA_P5;
    }
    h += len;
    for (; end - p >= 8; p += 8) {
        h = delta_rotl(h ^ delta_round(0, delta_read64(p)), 27) * DELTA_P1 + DELTA_P4;
    }
    if (end - p >= 4) {
        uint32_t w;
        memcpy(&w, p, sizeof(w));
        h = delta_rotl(h ^ (w * DELTA_P1), 23) * DELTA_P2 + DELTA_P3;
        p += 4;
    }
    for (; p < end; p++) {
        h = delta_rotl(h ^ (*p * DELTA_P5), 11) * DELTA_P1;
    }
    h ^= h >> 33;
    h *= DELTA_P2;
    h ^= h >> 29;
    h *= DELTA_P3;
    h ^= h >> 32;
    return h;
}

static void free_delta_chunks(struct DeltaChunks *chunks)
{
    free(chunks->ends);
    free(chunks->hashes);
    memset(chunks, 0, sizeof(*chunks));
}

/** Splits data into chunks whose ends depend only on the content near them. */
static TbBool split_delta_chunks(const char *data, size_t len, struct DeltaChunks *chunks)
{
    static uint64_t gear[256];
    if (gear[0] == 0) {
        // splitmix64: any fixed table works, it only has to be the same in every game
        uint64_t state = 0x9E3779B97F4A7C15ULL;
        for (int i = 0; i < 256; i++) {
            state += 0x9E3779B97F4A7C15ULL;
            uint64_t z = state;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            gear[i] = z ^ (z >> 31);
        }
    }
    memset(chunks, 0, sizeof(*chunks));
    chunks->ends = (uint32_t *)malloc(DELTA_MAX_CHUNKS * sizeof(uint32_t));
    chunks->hashes = (uint64_t *)malloc(DELTA_MAX_CHUNKS * sizeof(uint64_t));
    const unsigned char *bytes = (const unsigned char *)data;
    size_t start = 0;
    while (start < len) {
        if (chunks->ends == NULL || chunks->hashes == NULL || chunks->count >= DELTA_MAX_CHUNKS) {
            free_delta_chunks(chunks);
            return false;
        }
        const size_t end = min(start + ((size_t)1 << (DELTA_CHUNK_BITS + 2)), len);
        size_t pos = min(start + ((size_t)1 << (DELTA_CHUNK_BITS - 2)), end);
        uint64_t hash = 0;
        while (pos < end) {
            hash = (hash << 1) + gear[bytes[pos++]];
            if ((hash >> (64 - DELTA_CHUNK_BITS)) == 0) {
                break;
            }
        }
        chunks->ends[chunks->count] = (uint32_t)pos;
        chunks->hashes[chunks->count] = delta_hash(bytes + start, pos - start, DELTA_CHUNK_SEED);
        chunks->count++;
        start = pos;
    }
    return true;
}

static void forget_delta_base(void)
{
    free(delta_base);
    delta_base = NULL;
    free_delta_chunks(&delta_base_chunks);
}

static void forget_last_full_message(void)
{
    free(last_full_message);
    last_full_message = NULL;
    last_full_message_size = 0;
    memset(last_full_sent, 0, sizeof(last_full_sent));
}

void net_resync_forget_delta_state(void)
{
    forget_delta_base();
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        free(user_hashes[user]);
        user_hashes[user] = NULL;
    }
    delta_asked_users = 0;
    delta_answered_users = 0;
}

/** Client: the host asked for chunk hashes; keeps its resync data and answers, with no hashes if it cannot make them. */
static void send_delta_hashes(uint32_t generation)
{
    TbClockMSec start_time = LbTimerClock();
    forget_delta_base();
    size_t len = 0;
    char *data = build_resync_game_data(&len);
    struct ResyncHashesHeader header = { generation, 0 };
    if (data != NULL && split_delta_chunks(data, len, &delta_base_chunks)) {
        delta_base = data;
        delta_base_generation = generation;
        header.chunk_count = delta_base_chunks.count;
    } else {
        WARNLOG("Resync: could not make chunk hashes of %u bytes, asking for the full state", (unsigned)len);
        free(data);
    }
    char *write_pos = begin_net_message(NETMSG_RESYNC_HASHES);
    memcpy(write_pos, &header, sizeof(header));
    write_pos += sizeof(header);
    memcpy(write_pos, delta_base_chunks.hashes, header.chunk_count * sizeof(uint64_t));
    write_pos += header.chunk_count * sizeof(uint64_t);
    send_message_buffer(SERVER_ID, write_pos);
    NETLOG("Resync: sent the host %u chunk hashes (%u bytes) of turn %lu in %u ms", (unsigned)header.chunk_count,
        (unsigned)(write_pos - netstate.msg_buffer), (unsigned long)get_gameturn(), (unsigned)(LbTimerClock() - start_time));
}

/** NETMSG_RESYNC_HASHES: on a client, the host asking for them; on the host, a client's answer. */
TbError process_resync_hashes_message(NetUserId source, const char *buffer, size_t buffer_size)
{
    if (!network_is_host()) {
        uint32_t generation;
        if (source == SERVER_ID && buffer_size == sizeof(generation) && !network_user_is_spectator(netstate.my_id)) {
            memcpy(&generation, buffer, sizeof(generation));
            send_delta_hashes(generation);
        }
        return Lb_OK;
    }
    struct ResyncHashesHeader header;
    if (source <= SERVER_ID || source >= MAX_NET_USERS || (delta_asked_users & (1u << source)) == 0 || buffer_size < sizeof(header)) {
        return Lb_OK;
    }
    memcpy(&header, buffer, sizeof(header));
    if (header.generation != netstate.gameplay_generation) {
        return Lb_OK;
    }
    delta_answered_users |= 1u << source;
    free(user_hashes[source]);
    user_hashes[source] = NULL;
    if (header.chunk_count == 0 || header.chunk_count > DELTA_MAX_CHUNKS || buffer_size != sizeof(header) + (size_t)header.chunk_count * sizeof(uint64_t)) {
        MULTIPLAYER_LOG("Host: user %d sent no usable resync chunk hashes", (int)source);
        return Lb_OK;
    }
    user_hashes[source] = (uint64_t *)malloc(header.chunk_count * sizeof(uint64_t));
    if (user_hashes[source] != NULL) {
        memcpy(user_hashes[source], buffer + sizeof(header), header.chunk_count * sizeof(uint64_t));
        user_hash_count[source] = header.chunk_count;
    }
    return Lb_OK;
}

/** Host: a client could not use its delta (NETMSG_RESYNC_FULL_REQUEST): send it the full state, once. */
TbError process_resync_full_request_message(NetUserId source, const char *buffer, size_t buffer_size)
{
    if (!network_is_host() || source <= SERVER_ID || source >= MAX_NET_USERS || buffer_size != sizeof(uint32_t)) {
        return Lb_OK;
    }
    uint32_t generation;
    memcpy(&generation, buffer, sizeof(generation));
    if (generation != last_full_generation || last_full_message == NULL || last_full_sent[source]) {
        return Lb_OK;
    }
    last_full_sent[source] = true;
    NETLOG("Host: user %d could not apply the resync delta, sending the full state", (int)source);
    netstate.sp->sendmsg_single(source, last_full_message, last_full_message_size);
    return Lb_OK;
}

static TbBool user_in_game(NetUserId user)
{
    return user > SERVER_ID && user < MAX_NET_USERS && netstate.users[user].progress == USER_LOGGEDIN;
}

/** Host: asks the clients for their chunk hashes, then gives them a moment to answer. */
static void ask_for_delta_hashes(void)
{
    delta_asked_users = 0;
    delta_answered_users = 0;
    for (NetUserId user = SERVER_ID + 1; user < MAX_NET_USERS; user++) {
        if (user_in_game(user)) {
            char *write_pos = begin_net_message(NETMSG_RESYNC_HASHES);
            memcpy(write_pos, &netstate.gameplay_generation, sizeof(netstate.gameplay_generation));
            send_message_buffer(user, write_pos + sizeof(netstate.gameplay_generation));
            delta_asked_users |= 1u << user;
        }
    }
}

static void wait_for_delta_hashes(void)
{
    TbClockMSec start = LbTimerClock();
    for (;;) {
        TbBool missing = false;
        for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
            if ((delta_asked_users & ~delta_answered_users & (1u << user)) != 0 && user_in_game(user)) {
                missing = true;
            }
        }
        if (!missing || LbTimerClock() - start >= DELTA_HASH_WAIT_MAX_MS || quit_game || exit_keeper) {
            break;
        }
        netstate.sp->update(OnNewUser);
        for (NetUserId peer_id = 0; peer_id < MAX_NET_USERS; peer_id++) {
            while (can_send_to_peer(peer_id) && netstate.sp->msgready(peer_id, 0)) {
                process_network_message(peer_id, game.packets, sizeof(struct Packet), NETMSG_GAMEPLAY_UNSEQUENCED, NULL);
            }
        }
        if (!poll_inputs()) {
            exit_keeper = 1;
        }
        SDL_Delay(1);
    }
    NETLOG("Host: waited %u ms for resync chunk hashes", (unsigned)(LbTimerClock() - start));
}

static void append_delta_u32(char *payload, size_t *payload_len, uint32_t value)
{
    memcpy(payload + *payload_len, &value, sizeof(value));
    *payload_len += sizeof(value);
}

/** Host: the host's data as references to the client's chunks and the bytes it lacks, or NULL if a delta would not pay. */
static char *encode_resync_delta(const char *data, size_t len, uint64_t full_hash, const struct DeltaChunks *host_chunks,
    const uint64_t *client_hashes, uint32_t client_count, size_t *message_size)
{
    // the client's chunk for each hash; the lowest index if several have it
    std::vector<std::pair<uint64_t, uint32_t>> index(client_count);
    for (uint32_t i = 0; i < client_count; i++) {
        index[i] = std::make_pair(client_hashes[i], i);
    }
    std::sort(index.begin(), index.end());
    // Each chunk adds at most one tag; the bytes of all chunks add up to len.
    char *payload = (char *)malloc(len + ((size_t)host_chunks->count + 1) * sizeof(uint32_t));
    if (payload == NULL) {
        return NULL;
    }
    size_t payload_len = 0;
    uint32_t sent_chunks = 0;
    size_t literal_start = 0;
    size_t start = 0;
    for (uint32_t i = 0; i <= host_chunks->count; i++) {
        auto found = index.end();
        if (i < host_chunks->count) {
            found = std::lower_bound(index.begin(), index.end(), std::make_pair(host_chunks->hashes[i], (uint32_t)0));
            if (found != index.end() && found->first != host_chunks->hashes[i]) {
                found = index.end();
            }
        }
        if ((found != index.end() || i == host_chunks->count) && start > literal_start) {
            append_delta_u32(payload, &payload_len, (uint32_t)(start - literal_start));
            memcpy(payload + payload_len, data + literal_start, start - literal_start);
            payload_len += start - literal_start;
        }
        if (i == host_chunks->count) {
            break;
        }
        if (found != index.end()) {
            append_delta_u32(payload, &payload_len, DELTA_COPY_FLAG | found->second);
            literal_start = host_chunks->ends[i];
        } else {
            sent_chunks++;
        }
        start = host_chunks->ends[i];
    }
    // Not worth it if the client lacks more than half of the data.
    uLongf compressed_size = compressBound(payload_len);
    char *message = payload_len > len / 2 ? NULL : (char *)malloc(sizeof(struct ResyncDeltaHeader) + compressed_size);
    if (message == NULL || compress2((Bytef *)(message + sizeof(struct ResyncDeltaHeader)), &compressed_size, (const Bytef *)payload, payload_len, Z_BEST_SPEED) != Z_OK) {
        NETLOG("Host: no resync delta (%u of %u chunks differ), sending the full state", (unsigned)sent_chunks, (unsigned)host_chunks->count);
        free(payload);
        free(message);
        return NULL;
    }
    free(payload);
    struct ResyncDeltaHeader header;
    header.message_type = NETMSG_RESYNC_DELTA;
    header.compressed_length = (uint32_t)compressed_size;
    header.payload_length = (uint32_t)payload_len;
    header.full_length = (uint32_t)len;
    header.full_hash = full_hash;
    header.gameplay_generation = netstate.gameplay_generation;
    memcpy(message, &header, sizeof(header));
    *message_size = sizeof(header) + compressed_size;
    NETLOG("Host: resync delta of %u of %u chunks, %u bytes", (unsigned)sent_chunks, (unsigned)host_chunks->count, (unsigned)*message_size);
    return message;
}

/** Client: rebuilds the host's resync data from a delta and the kept copy of its own. */
static TbBool decode_resync_delta(const char *message, size_t message_size, char **data_out, size_t *len_out, uint32_t *generation)
{
    struct ResyncDeltaHeader header;
    if (message_size < sizeof(header)) {
        return false;
    }
    memcpy(&header, message, sizeof(header));
    if (delta_base == NULL || header.gameplay_generation != delta_base_generation + 1 || header.compressed_length != message_size - sizeof(header)
        || header.full_length > sizeof(game) + 16 * 1024 * 1024
        || header.payload_length > header.full_length + (size_t)(DELTA_MAX_CHUNKS + 1) * sizeof(uint32_t)) {
        WARNLOG("Resync delta does not fit the kept state");
        return false;
    }
    char *payload = (char *)malloc(max(header.payload_length, 1U));
    char *full = (char *)malloc(max(header.full_length, 1U));
    uLongf payload_len = header.payload_length;
    TbBool ok = payload != NULL && full != NULL
        && uncompress((Bytef *)payload, &payload_len, (const Bytef *)(message + sizeof(header)), header.compressed_length) == Z_OK
        && payload_len == header.payload_length;
    size_t pos = 0;
    size_t written = 0;
    while (ok && pos < payload_len) {
        uint32_t tag;
        ok = payload_len - pos >= sizeof(tag);
        if (!ok) {
            break;
        }
        memcpy(&tag, payload + pos, sizeof(tag));
        pos += sizeof(tag);
        const char *source = payload + pos;
        size_t length = tag;
        if ((tag & DELTA_COPY_FLAG) != 0) {
            const uint32_t chunk = tag & ~DELTA_COPY_FLAG;
            ok = chunk < delta_base_chunks.count;
            const size_t chunk_start = (!ok || chunk == 0) ? 0 : delta_base_chunks.ends[chunk - 1];
            source = delta_base + chunk_start;
            length = ok ? delta_base_chunks.ends[chunk] - chunk_start : 0;
        } else {
            ok = payload_len - pos >= length;
            pos += length;
        }
        ok = ok && header.full_length - written >= length;
        if (ok) {
            memcpy(full + written, source, length);
            written += length;
        }
    }
    free(payload);
    if (ok && (written != header.full_length || delta_hash(full, header.full_length, DELTA_STATE_SEED) != header.full_hash)) {
        WARNLOG("Resync delta rebuilt a state with the wrong length or hash");
        ok = false;
    }
    if (!ok) {
        free(full);
        return false;
    }
    *data_out = full;
    *len_out = header.full_length;
    *generation = header.gameplay_generation;
    return true;
}

static void request_full_resync(uint32_t generation)
{
    char *write_pos = begin_net_message(NETMSG_RESYNC_FULL_REQUEST);
    memcpy(write_pos, &generation, sizeof(generation));
    send_message_buffer(SERVER_ID, write_pos + sizeof(generation));
}

TbBool send_resync_game(void)
{
    netstate.resync_pending = false;
    pack_desync_history_for_resync();
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 100);
    NETLOG("Initiating re-synchronization of network game");
    take_game_timestamp();

    // The clients build and hash their data while the host builds and splits its own.
    ask_for_delta_hashes();
    size_t full_len = 0;
    char *full_data = build_resync_game_data(&full_len);
    if (full_data == NULL) {
        return false;
    }
    struct DeltaChunks host_chunks;
    memset(&host_chunks, 0, sizeof(host_chunks));
    uint64_t full_hash = 0;
    if (delta_asked_users != 0) {
        full_hash = delta_hash(full_data, full_len, DELTA_STATE_SEED);
        split_delta_chunks(full_data, full_len, &host_chunks);
        // before the generation changes: the answers carry the current one
        wait_for_delta_hashes();
    }
    netstate.gameplay_generation += 1;
    // Deltas first: the full state is compressed while these clients rebuild theirs.
    uint32_t delta_sent = 0;
    for (NetUserId user = SERVER_ID + 1; user < MAX_NET_USERS; user++) {
        size_t delta_size = 0;
        char *delta = NULL;
        if (user_hashes[user] != NULL && host_chunks.count != 0 && user_in_game(user)) {
            delta = encode_resync_delta(full_data, full_len, full_hash, &host_chunks, user_hashes[user], user_hash_count[user], &delta_size);
        }
        if (delta != NULL) {
            netstate.sp->sendmsg_single(netstate.users[user].id, delta, delta_size);
            free(delta);
            delta_sent |= 1u << user;
        }
    }
    free_delta_chunks(&host_chunks);
    size_t message_size = 0;
    char *message_buffer = encode_resync_message(full_data, full_len, &message_size);
    free(full_data);
    if (message_buffer == NULL) {
        if (delta_sent == 0) {
            netstate.gameplay_generation -= 1;
            return false;
        }
        // Only the clients that got a delta can resync; the others will time out and stay out of sync.
        return true;
    }
    NETLOG("Host: Sending resync data to all clients");
    for (NetUserId user_index = SERVER_ID + 1; user_index < MAX_NET_CONNECTIONS; ++user_index) {
        if (netstate.users[user_index].progress != USER_LOGGEDIN || (user_index < MAX_NET_USERS && (delta_sent & (1u << user_index)) != 0)
         || (user_index >= MAX_NET_USERS && netstate.users[user_index].spectator_state != NetSpectator_Streaming)) {
            continue;
        }
        netstate.sp->sendmsg_single(netstate.users[user_index].id, message_buffer, message_size);
    }
    replay_record_resync(message_buffer, message_size);
    // Kept until the clients are ready again, for one whose delta fails.
    forget_last_full_message();
    last_full_message = message_buffer;
    last_full_message_size = message_size;
    last_full_generation = netstate.gameplay_generation;
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
    TbBool result;
    if (message_buffer[0] == NETMSG_RESYNC_DELTA) {
        TbClockMSec start_time = LbTimerClock();
        char *full_data = NULL;
        size_t full_len = 0;
        uint32_t generation = 0;
        struct ResyncDeltaHeader header;
        memset(&header, 0, sizeof(header));
        memcpy(&header, message_buffer, min(sizeof(header), message_size));
        result = decode_resync_delta(message_buffer, message_size, &full_data, &full_len, &generation)
            && apply_resync_game_data(full_data, full_len);
        free(message_buffer);
        message_buffer = NULL;
        if (result) {
            netstate.gameplay_generation = generation;
            NETLOG("Resync delta (%u bytes) applied in %u ms", (unsigned)message_size, (unsigned)(LbTimerClock() - start_time));
            if (replay.save_enable) {
                // Replays and the spectators they serve keep full states.
                message_buffer = encode_resync_message(full_data, full_len, &message_size);
            }
        } else {
            // Ask the host for the full state instead; only once, so this cannot loop.
            NETLOG("Resync delta could not be applied, asking the host for the full state");
            request_full_resync(header.gameplay_generation);
            free(full_data);
            full_data = NULL;
            if (!receive_resync_message(&message_buffer, &message_size)) {
                return false;
            }
            result = message_buffer[0] == NETMSG_RESYNC_DATA && apply_resync_game_message(message_buffer, message_size);
        }
        free(full_data);
    } else {
        result = apply_resync_game_message(message_buffer, message_size);
    }
    if (result && message_buffer != NULL) {
        replay_record_resync(message_buffer, message_size);
    }
    free(message_buffer);
    if (!result) {
        return false;
    }

    animate_resync_progress_bar(90, 100);

    if ((local_system_flags & (GSF_NetGameNoSync | GSF_NetSeedNoSync)) != 0) {
        compare_desync_history_from_host();
    }

    return true;
}

static void finish_resync(const struct Packet *saved_packets)
{
    net_resync_forget_delta_state();
    if (Lvl_script != NULL) {
        lua_set_random_seed(game.action_random_seed);
    }
    recall_localised_game_structure();
    reinit_level_after_load();
    light_import_system_state(&game.lightst);
    light_rebuild_after_load();
    network_spectator_finish_seat();
    memcpy(game.packets, saved_packets, sizeof(game.packets));

    if (!network_is_active() || !network_user_is_spectator(netstate.my_id)) {
        game.skip_initial_input_turns = calculate_skip_input();
    }
    initialize_packet_history();
    NETLOG("Input lag after resync: %d turns", game.input_lag_turns);

    clear_flag(local_system_flags, GSF_NetGameNoSync);
    clear_flag(local_system_flags, GSF_NetSeedNoSync);
}

void resync_game(void)
{
    TbClockMSec start_time = LbTimerClock();
    SYNCDBG(2,"Starting");
    resync_progress_visible = (local_system_flags & (GSF_NetGameNoSync | GSF_NetSeedNoSync)) != 0;
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
        net_resync_forget_delta_state();
        recall_localised_game_structure();
        return;
    }
    finish_resync(saved_packets);
    wait_for_all_players();
    forget_last_full_message();
    animate_resync_progress_bar(100, 100);
    update_gameplay_delta_time();
    local_state.process_turn_time = 1.0;
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
