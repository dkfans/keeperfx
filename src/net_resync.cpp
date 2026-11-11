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
// Every player sees a desync on the same game turn: they compare the checksums in the same
// packets. A client then keeps the resync data of its own state at that point, where the host
// builds its own (the end of process_packets), and sends the host a hash of each chunk of it.
// When the host resyncs, such a client gets only the chunks of the host's data that it lacks,
// and references to the ones it has; the result is checked against the CRC of the host's data.
// The state is ~23 MB, ~320 KB compressed; a delta is usually a small fraction of that. A client
// that cannot use a delta answers NETMSG_RESYNC_FULL_REQUEST and the host sends it the full state,
// once. Players in one game always run the same build, so every client and host takes part;
// spectators and replays keep using full states.
//
// The resync data has no fixed layout (a union's stored bytes depend on the member in use), so
// one changed thing can shift everything after it. Chunks therefore end where the content says
// (a rolling hash of the last 64 bytes), not at fixed offsets: a shift only changes the chunks
// around it.

#define DELTA_TARGET_CHUNKS 4096
#define DELTA_MIN_CHUNK_BITS 12
#define DELTA_MAX_CHUNK_BITS 20
#define DELTA_MAX_CHUNKS ((NET_MSG_BUFFER_SIZE - 64) / sizeof(uint32_t))
#define DELTA_COPY_FLAG 0x80000000u
// Clients send their hashes on the turn they see the desync, so the host normally stops waiting as
// soon as they arrive; the limit covers building them (~100 ms) and sending them on a slow link.
#define DELTA_HASH_WAIT_MIN_MS 1000
#define DELTA_HASH_WAIT_MAX_MS 2000

#pragma pack(1)
struct ResyncHashesHeader {
    uint32_t generation;
    uint32_t base_id;
    uint32_t total_length;
    uint32_t chunk_bits;
    uint32_t chunk_count;
};

struct ResyncDeltaHeader {
    unsigned char message_type;
    uint32_t compressed_length;
    uint32_t payload_length;
    uint32_t full_length;
    uint32_t full_checksum;
    uint32_t gameplay_generation;
    uint32_t base_id;
};
#pragma pack()

struct DeltaChunks {
    uint32_t *ends;     // end offset of each chunk
    uint32_t *hashes;
    uint32_t count;
};

struct DeltaHashes {
    uint32_t *hashes;
    struct ResyncHashesHeader header;
};

struct DeltaHashIndex {
    uint32_t hash;
    uint32_t chunk;
};

// client: its resync data from when it saw the desync, and its chunks
static char *delta_base = NULL;
static size_t delta_base_len = 0;
static uint32_t delta_base_id = 0;
static struct DeltaChunks delta_base_chunks;
// host: what each client sent
static struct DeltaHashes user_hashes[MAX_NET_USERS];
// host: the last full resync message, for a client whose delta failed
static char *last_full_message = NULL;
static size_t last_full_message_size = 0;
static uint32_t last_full_generation = 0;
static TbBool last_full_sent[MAX_NET_USERS];

static uint64_t delta_gear[256];

static void init_delta_gear(void)
{
    if (delta_gear[0] != 0) {
        return;
    }
    // splitmix64: any fixed table works, it only has to be the same in every game
    uint64_t state = 0x9E3779B97F4A7C15ULL;
    for (int i = 0; i < 256; i++) {
        state += 0x9E3779B97F4A7C15ULL;
        uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        delta_gear[i] = z ^ (z >> 31);
    }
}

static uint32_t delta_chunk_bits(size_t total)
{
    uint32_t bits = DELTA_MIN_CHUNK_BITS;
    while (bits < DELTA_MAX_CHUNK_BITS && ((size_t)1 << bits) * DELTA_TARGET_CHUNKS < total) {
        bits++;
    }
    return bits;
}

static void free_delta_chunks(struct DeltaChunks *chunks)
{
    free(chunks->ends);
    free(chunks->hashes);
    memset(chunks, 0, sizeof(*chunks));
}

/** Splits data into chunks of 2^bits bytes on average whose ends depend only on the content near them. */
static TbBool split_delta_chunks(const char *data, size_t len, uint32_t bits, struct DeltaChunks *chunks)
{
    init_delta_gear();
    memset(chunks, 0, sizeof(*chunks));
    chunks->ends = (uint32_t *)malloc(DELTA_MAX_CHUNKS * sizeof(uint32_t));
    chunks->hashes = (uint32_t *)malloc(DELTA_MAX_CHUNKS * sizeof(uint32_t));
    if (chunks->ends == NULL || chunks->hashes == NULL) {
        free_delta_chunks(chunks);
        return false;
    }
    const size_t min_size = (size_t)1 << (bits - 2);
    const size_t max_size = (size_t)1 << (bits + 2);
    const unsigned shift = 64 - bits;
    const unsigned char *bytes = (const unsigned char *)data;
    size_t start = 0;
    while (start < len) {
        if (chunks->count >= DELTA_MAX_CHUNKS) {
            free_delta_chunks(chunks);
            return false;
        }
        const size_t end = min(start + max_size, len);
        size_t pos = start + min_size;
        if (pos >= end) {
            pos = end;
        } else {
            uint64_t hash = 0;
            for (; pos < end; pos++) {
                hash = (hash << 1) + delta_gear[bytes[pos]];
                if ((hash >> shift) == 0) {
                    pos++;
                    break;
                }
            }
        }
        chunks->ends[chunks->count] = (uint32_t)pos;
        chunks->hashes[chunks->count] = (uint32_t)crc32(0L, bytes + start, (uInt)(pos - start));
        chunks->count++;
        start = pos;
    }
    return true;
}

static void forget_user_hashes(NetUserId user)
{
    free(user_hashes[user].hashes);
    memset(&user_hashes[user], 0, sizeof(user_hashes[user]));
}

void net_resync_forget_delta_state(void)
{
    free(delta_base);
    delta_base = NULL;
    delta_base_len = 0;
    free_delta_chunks(&delta_base_chunks);
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        forget_user_hashes(user);
    }
}

/** Client, at the end of process_packets on a turn where the game is out of sync. */
void net_resync_note_client_desync(void)
{
    if (!network_is_active() || network_is_host() || network_user_is_spectator(netstate.my_id) || delta_base != NULL) {
        return;
    }
    TbClockMSec start_time = LbTimerClock();
    size_t len = 0;
    char *data = build_resync_game_data(&len);
    if (data == NULL) {
        return;
    }
    const uint32_t bits = delta_chunk_bits(len);
    struct DeltaChunks chunks;
    char *write_pos = begin_net_message(NETMSG_RESYNC_HASHES);
    if (!split_delta_chunks(data, len, bits, &chunks)
        || (size_t)(write_pos - netstate.msg_buffer) + sizeof(struct ResyncHashesHeader) + chunks.count * sizeof(uint32_t) > sizeof(netstate.msg_buffer)) {
        free_delta_chunks(&chunks);
        free(data);
        return;
    }
    delta_base = data;
    delta_base_len = len;
    delta_base_chunks = chunks;
    delta_base_id += 1;
    struct ResyncHashesHeader header = { netstate.gameplay_generation, delta_base_id, (uint32_t)len, bits, chunks.count };
    memcpy(write_pos, &header, sizeof(header));
    write_pos += sizeof(header);
    memcpy(write_pos, chunks.hashes, chunks.count * sizeof(uint32_t));
    write_pos += chunks.count * sizeof(uint32_t);
    send_message_buffer(SERVER_ID, write_pos);
    NETLOG("Desync: kept own state (%u bytes) and sent the host %u chunk hashes in %u ms",
        (unsigned)len, (unsigned)chunks.count, (unsigned)(LbTimerClock() - start_time));
}

/** Host: a client's chunk hashes (NETMSG_RESYNC_HASHES). */
TbError process_resync_hashes_message(NetUserId source, const char *buffer, size_t buffer_size)
{
    if (!network_is_host() || source <= SERVER_ID || source >= MAX_NET_USERS || buffer_size < sizeof(struct ResyncHashesHeader)) {
        return Lb_OK;
    }
    struct ResyncHashesHeader header;
    memcpy(&header, buffer, sizeof(header));
    if (header.chunk_bits < DELTA_MIN_CHUNK_BITS || header.chunk_bits > DELTA_MAX_CHUNK_BITS
        || header.chunk_count == 0 || header.chunk_count > DELTA_MAX_CHUNKS
        || buffer_size != sizeof(header) + (size_t)header.chunk_count * sizeof(uint32_t)) {
        WARNLOG("Invalid resync chunk hashes from user %d", (int)source);
        return Lb_OK;
    }
    forget_user_hashes(source);
    user_hashes[source].hashes = (uint32_t *)malloc(header.chunk_count * sizeof(uint32_t));
    if (user_hashes[source].hashes == NULL) {
        return Lb_OK;
    }
    memcpy(user_hashes[source].hashes, buffer + sizeof(header), header.chunk_count * sizeof(uint32_t));
    user_hashes[source].header = header;
    MULTIPLAYER_LOG("Host: user %d sent %u resync chunk hashes", (int)source, (unsigned)header.chunk_count);
    return Lb_OK;
}

static void forget_last_full_message(void)
{
    free(last_full_message);
    last_full_message = NULL;
    last_full_message_size = 0;
    memset(last_full_sent, 0, sizeof(last_full_sent));
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

static TbBool user_wants_delta(NetUserId user)
{
    return user > SERVER_ID && user < MAX_NET_USERS && netstate.users[user].progress == USER_LOGGEDIN;
}

/** Host: gives the clients a moment to send their chunk hashes. */
static void wait_for_delta_hashes(uint32_t generation)
{
    TbClockMSec wait = DELTA_HASH_WAIT_MIN_MS;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        if (user_wants_delta(user)) {
            wait = max(wait, (TbClockMSec)(2 * GetPing(user) + 500));
        }
    }
    wait = min(wait, (TbClockMSec)DELTA_HASH_WAIT_MAX_MS);
    TbClockMSec start = LbTimerClock();
    for (;;) {
        TbBool missing = false;
        for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
            if (user_wants_delta(user) && (user_hashes[user].hashes == NULL || user_hashes[user].header.generation != generation)) {
                missing = true;
            }
        }
        if (!missing || LbTimerClock() - start >= wait || quit_game || exit_keeper) {
            break;
        }
        netstate.sp->update(OnNewUser);
        for (NetUserId peer_id = 0; peer_id < MAX_NET_USERS; peer_id++) {
            if (!can_send_to_peer(peer_id)) {
                continue;
            }
            while (netstate.sp->msgready(peer_id, 0)) {
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

static int compare_delta_hash_index(const void *a, const void *b)
{
    const struct DeltaHashIndex *x = (const struct DeltaHashIndex *)a;
    const struct DeltaHashIndex *y = (const struct DeltaHashIndex *)b;
    if (x->hash != y->hash) {
        return x->hash < y->hash ? -1 : 1;
    }
    return x->chunk < y->chunk ? -1 : (x->chunk > y->chunk ? 1 : 0);
}

static void append_delta_u32(char *payload, size_t *payload_len, uint32_t value)
{
    memcpy(payload + *payload_len, &value, sizeof(value));
    *payload_len += sizeof(value);
}

/** Host: the host's data as references to the client's chunks and the bytes it lacks, or NULL if a delta would not pay. */
static char *encode_resync_delta(const char *data, size_t len, uint32_t full_checksum, const struct DeltaChunks *host_chunks,
    const struct DeltaHashes *client, size_t full_message_size, size_t *message_size)
{
    const uint32_t client_count = client->header.chunk_count;
    struct DeltaHashIndex *index = (struct DeltaHashIndex *)malloc(client_count * sizeof(struct DeltaHashIndex));
    // Each chunk adds at most one tag; the bytes of all chunks add up to len.
    char *payload = (char *)malloc(len + (size_t)host_chunks->count * sizeof(uint32_t) + sizeof(uint32_t));
    if (index == NULL || payload == NULL) {
        free(index);
        free(payload);
        return NULL;
    }
    for (uint32_t i = 0; i < client_count; i++) {
        index[i].hash = client->hashes[i];
        index[i].chunk = i;
    }
    qsort(index, client_count, sizeof(*index), compare_delta_hash_index);
    size_t payload_len = 0;
    uint32_t sent_chunks = 0;
    size_t literal_start = 0;
    size_t literal_len = 0;
    size_t start = 0;
    for (uint32_t i = 0; i <= host_chunks->count; i++) {
        const struct DeltaHashIndex *found = NULL;
        size_t end = len;
        if (i < host_chunks->count) {
            end = host_chunks->ends[i];
            struct DeltaHashIndex key = { host_chunks->hashes[i], 0 };
            // the lowest client chunk with this hash
            size_t lo = 0;
            size_t hi = client_count;
            while (lo < hi) {
                size_t mid = (lo + hi) / 2;
                if (index[mid].hash < key.hash) {
                    lo = mid + 1;
                } else {
                    hi = mid;
                }
            }
            if (lo < client_count && index[lo].hash == key.hash) {
                found = &index[lo];
            }
        }
        if ((found != NULL || i == host_chunks->count) && literal_len > 0) {
            append_delta_u32(payload, &payload_len, (uint32_t)literal_len);
            memcpy(payload + payload_len, data + literal_start, literal_len);
            payload_len += literal_len;
            literal_len = 0;
        }
        if (i == host_chunks->count) {
            break;
        }
        if (found != NULL) {
            append_delta_u32(payload, &payload_len, DELTA_COPY_FLAG | found->chunk);
        } else {
            if (literal_len == 0) {
                literal_start = start;
            }
            literal_len += end - start;
            sent_chunks++;
        }
        start = end;
    }
    free(index);
    uLongf compressed_size = compressBound(payload_len);
    char *message = (char *)malloc(sizeof(struct ResyncDeltaHeader) + compressed_size);
    if (message == NULL) {
        free(payload);
        return NULL;
    }
    int result = compress2((Bytef *)(message + sizeof(struct ResyncDeltaHeader)), &compressed_size, (const Bytef *)payload, payload_len, Z_BEST_SPEED);
    free(payload);
    // Not worth it if it isn't much smaller than the full state.
    if (result != Z_OK || sizeof(struct ResyncDeltaHeader) + compressed_size > full_message_size / 2) {
        NETLOG("Host: resync delta of %u of %u chunks would not pay, sending the full state", (unsigned)sent_chunks, (unsigned)host_chunks->count);
        free(message);
        return NULL;
    }
    struct ResyncDeltaHeader header;
    header.message_type = NETMSG_RESYNC_DELTA;
    header.compressed_length = (uint32_t)compressed_size;
    header.payload_length = (uint32_t)payload_len;
    header.full_length = (uint32_t)len;
    header.full_checksum = full_checksum;
    header.gameplay_generation = netstate.gameplay_generation;
    header.base_id = client->header.base_id;
    memcpy(message, &header, sizeof(header));
    *message_size = sizeof(header) + compressed_size;
    NETLOG("Host: resync delta of %u of %u chunks, %u bytes (full state %u bytes)",
        (unsigned)sent_chunks, (unsigned)host_chunks->count, (unsigned)*message_size, (unsigned)full_message_size);
    return message;
}

/** Client: rebuilds the host's resync data from a delta and the kept copy of its own. */
static TbBool decode_resync_delta(const char *message, size_t message_size, char **data_out, size_t *len_out, uint32_t *generation)
{
    if (message_size < sizeof(struct ResyncDeltaHeader)) {
        return false;
    }
    struct ResyncDeltaHeader header;
    memcpy(&header, message, sizeof(header));
    if (delta_base == NULL || header.base_id != delta_base_id || header.compressed_length != message_size - sizeof(header)
        || header.full_length > sizeof(game) + 16 * 1024 * 1024
        || header.payload_length > header.full_length + (size_t)(DELTA_MAX_CHUNKS + 1) * sizeof(uint32_t)) {
        WARNLOG("Resync delta does not fit the kept state (base %u, kept %u)", (unsigned)header.base_id, (unsigned)delta_base_id);
        return false;
    }
    char *payload = (char *)malloc(max(header.payload_length, 1U));
    char *full = (char *)malloc(max(header.full_length, 1U));
    uLongf payload_len = header.payload_length;
    if (payload == NULL || full == NULL
        || uncompress((Bytef *)payload, &payload_len, (const Bytef *)(message + sizeof(header)), header.compressed_length) != Z_OK
        || payload_len != header.payload_length) {
        free(payload);
        free(full);
        return false;
    }
    TbBool ok = true;
    size_t pos = 0;
    size_t written = 0;
    while (ok && pos < payload_len) {
        uint32_t tag;
        if (payload_len - pos < sizeof(tag)) {
            ok = false;
            break;
        }
        memcpy(&tag, payload + pos, sizeof(tag));
        pos += sizeof(tag);
        const char *source;
        size_t length;
        if ((tag & DELTA_COPY_FLAG) != 0) {
            uint32_t chunk = tag & ~DELTA_COPY_FLAG;
            if (chunk >= delta_base_chunks.count) {
                ok = false;
                break;
            }
            size_t chunk_start = chunk == 0 ? 0 : delta_base_chunks.ends[chunk - 1];
            source = delta_base + chunk_start;
            length = delta_base_chunks.ends[chunk] - chunk_start;
        } else {
            if (payload_len - pos < tag) {
                ok = false;
                break;
            }
            source = payload + pos;
            length = tag;
            pos += tag;
        }
        if (header.full_length - written < length) {
            ok = false;
            break;
        }
        memcpy(full + written, source, length);
        written += length;
    }
    free(payload);
    if (ok && written != header.full_length) {
        ok = false;
    }
    if (ok && (uint32_t)crc32(crc32(0L, Z_NULL, 0), (const Bytef *)full, header.full_length) != header.full_checksum) {
        WARNLOG("Resync delta rebuilt a state with the wrong checksum");
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
    write_pos += sizeof(generation);
    send_message_buffer(SERVER_ID, write_pos);
}

TbBool send_resync_game(void)
{
    netstate.resync_pending = false;
    pack_desync_history_for_resync();
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 100);
    NETLOG("Initiating re-synchronization of network game");
    take_game_timestamp();

    const uint32_t previous_generation = netstate.gameplay_generation;
    netstate.gameplay_generation += 1;
    size_t full_len = 0;
    char *full_data = build_resync_game_data(&full_len);
    size_t message_size = 0;
    char *message_buffer = NULL;
    if (full_data != NULL) {
        message_buffer = encode_resync_message(full_data, full_len, &message_size);
    }
    if (message_buffer == NULL) {
        free(full_data);
        netstate.gameplay_generation -= 1;
        return false;
    }
    wait_for_delta_hashes(previous_generation);
    const uint32_t full_checksum = (uint32_t)crc32(crc32(0L, Z_NULL, 0), (const Bytef *)full_data, (uInt)full_len);
    NETLOG("Host: Sending resync data to all clients");
    struct DeltaChunks host_chunks;
    memset(&host_chunks, 0, sizeof(host_chunks));
    uint32_t host_chunk_bits = 0;
    for (NetUserId user_index = 0; user_index < MAX_NET_CONNECTIONS; ++user_index) {
        if (user_index == SERVER_ID || netstate.users[user_index].progress != USER_LOGGEDIN || (user_index >= MAX_NET_USERS && netstate.users[user_index].spectator_state != NetSpectator_Streaming)) {
            continue;
        }
        char *delta = NULL;
        size_t delta_size = 0;
        const struct DeltaHashes *client = &user_hashes[user_index];
        if (user_wants_delta(user_index) && client->hashes != NULL && client->header.generation == previous_generation) {
            // The host's chunks of the size this client used, the same for all clients in practice.
            if (host_chunk_bits != client->header.chunk_bits) {
                free_delta_chunks(&host_chunks);
                host_chunk_bits = 0;
                if (split_delta_chunks(full_data, full_len, client->header.chunk_bits, &host_chunks)) {
                    host_chunk_bits = client->header.chunk_bits;
                }
            }
            if (host_chunk_bits != 0) {
                delta = encode_resync_delta(full_data, full_len, full_checksum, &host_chunks, client, message_size, &delta_size);
            }
        }
        if (delta != NULL) {
            netstate.sp->sendmsg_single(netstate.users[user_index].id, delta, delta_size);
            free(delta);
        } else {
            netstate.sp->sendmsg_single(netstate.users[user_index].id, message_buffer, message_size);
        }
    }
    free_delta_chunks(&host_chunks);
    free(full_data);
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
