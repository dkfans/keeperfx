/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_sync.c
 *     Network game synchronization for Dungeon Keeper.
 * @par Purpose:
 *     Functions to keep network games synchronized.
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
#include "net_sync.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_network.h"

#include "config.h"
#include "config_effects.h"
#include "front_network.h"
#include "player_data.h"
#include "game_merge.h"
#include "net_game.h"
#include "lens_api.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "frontend.h"
#include "thing_effects.h"
#include "thing_data.h"
#include "room_data.h"
#include "room_list.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
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
/******************************************************************************/
/** Structure used for storing 'localised parameters' when resyncing net game. */
struct Boing boing;

/**
 * Structure to hold detailed thing category checksums for analysis.
 */
struct LogDetailedThingChecksums {
    TbBigChecksum creatures;
    TbBigChecksum traps;
    TbBigChecksum shots;
    TbBigChecksum objects;
    TbBigChecksum effects;
    TbBigChecksum dead_creatures;
    TbBigChecksum effect_gens;
    TbBigChecksum doors;
};

// Pre-resync checksum storage for desync analysis between host and clients
// Host stores these before sending resync data, clients store before receiving
static struct {
    TbBigChecksum log_things_sum;
    TbBigChecksum log_rooms_sum;
    GameTurn log_gameturn;
    TbBigChecksum log_action_random_seed;
    TbBigChecksum log_ai_random_seed;
    TbBigChecksum log_player_random_seed;
    TbBigChecksum log_player_checksums[PLAYERS_COUNT];
    struct LogDetailedThingChecksums log_things_detailed;
    struct LogThingDesyncInfo log_individual_thing_info[THINGS_COUNT];
    struct LogRoomDesyncInfo log_individual_room_info[ROOMS_COUNT + 1];
    TbBool log_is_stored;
} log_pre_resync_checksums = {0};


// Host populates game state with diagnostic checksums that clients receive during resync
// This allows clients to compare their pre-resync state with host's pre-resync state
static void populate_desync_diagnostics(void)
{
    game.desync_diagnostics.desync_turn = log_pre_resync_checksums.log_gameturn;
    game.desync_diagnostics.host_things_sum = log_pre_resync_checksums.log_things_sum;
    game.desync_diagnostics.host_rooms_sum = log_pre_resync_checksums.log_rooms_sum;

    game.desync_diagnostics.host_creatures_sum = log_pre_resync_checksums.log_things_detailed.creatures;
    game.desync_diagnostics.host_traps_sum = log_pre_resync_checksums.log_things_detailed.traps;
    game.desync_diagnostics.host_shots_sum = log_pre_resync_checksums.log_things_detailed.shots;
    game.desync_diagnostics.host_objects_sum = log_pre_resync_checksums.log_things_detailed.objects;
    game.desync_diagnostics.host_effects_sum = log_pre_resync_checksums.log_things_detailed.effects;
    game.desync_diagnostics.host_dead_creatures_sum = log_pre_resync_checksums.log_things_detailed.dead_creatures;
    game.desync_diagnostics.host_effect_gens_sum = log_pre_resync_checksums.log_things_detailed.effect_gens;
    game.desync_diagnostics.host_doors_sum = log_pre_resync_checksums.log_things_detailed.doors;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        game.desync_diagnostics.host_player_checksums[i] = log_pre_resync_checksums.log_player_checksums[i];
    }
    game.desync_diagnostics.host_action_random_seed = log_pre_resync_checksums.log_action_random_seed;
    game.desync_diagnostics.host_ai_random_seed = log_pre_resync_checksums.log_ai_random_seed;
    game.desync_diagnostics.host_player_random_seed = log_pre_resync_checksums.log_player_random_seed;

    // Copy stored pre-resync individual Thing detailed info for detailed analysis
    memcpy(game.desync_diagnostics.host_thing_info, log_pre_resync_checksums.log_individual_thing_info, sizeof(game.desync_diagnostics.host_thing_info));

    // Copy stored pre-resync individual Room detailed info for detailed analysis
    memcpy(game.desync_diagnostics.host_room_info, log_pre_resync_checksums.log_individual_room_info, sizeof(game.desync_diagnostics.host_room_info));

    game.desync_diagnostics.has_desync_diagnostics = true;
}
/******************************************************************************/
long get_resync_sender(void)
{
    return get_host_player_id();
}

TbBool send_resync_game(void)
{
  //TODO NET see if it is necessary to dump to file... probably superfluous
  char* fname = prepare_file_path(FGrp_Save, "resync.dat");
  TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_NEW);
  if (!fh) {
    ERRORLOG("Can't open resync file.");
    return false;
  }

  if (log_pre_resync_checksums.log_is_stored) {
    populate_desync_diagnostics();
    ERRORLOG("Host including desync diagnostics in resync: turn %ld", log_pre_resync_checksums.log_gameturn);
  } else {
    game.desync_diagnostics.has_desync_diagnostics = false;
  }

  LbFileWrite(fh, &game, sizeof(game));
  LbFileClose(fh);

  NETLOG("Initiating re-synchronization of network game");
  return LbNetwork_Resync(&game, sizeof(game));
}

TbBool receive_resync_game(void)
{
    NETLOG("Initiating re-synchronization of network game");
    TbBool result = LbNetwork_Resync(&game, sizeof(game));

    // After receiving the resync, check if host included diagnostic data
    if (result && game.desync_diagnostics.has_desync_diagnostics && log_pre_resync_checksums.log_is_stored) {
        log_analyze_desync_diagnostics_from_host();
    }

    return result;
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
}

void resync_game(void)
{
    SYNCDBG(2,"Starting");
    struct PlayerInfo* player = get_my_player();
    draw_out_of_sync_box(0, 32*units_per_pixel/16, player->engine_window_x);
    reset_eye_lenses();
    store_localised_game_structure();
    int i = get_resync_sender();
    if (is_my_player_number(i))
    {
        send_resync_game();
    } else
    {
        receive_resync_game();
    }
    recall_localised_game_structure();
    reinit_level_after_load();
    clear_flag(game.system_flags, GSF_NetGameNoSync);
    clear_flag(game.system_flags, GSF_NetSeedNoSync);
}

/**
 * Exchanges verification packets between all players, making sure level data is identical.
 * @return Returns true if all players return same checksum.
 */
CoroutineLoopState perform_checksum_verification(CoroutineLoop *con)
{
    short result = true;
    unsigned long checksum_mem = 0;
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing)) {
            checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
        }
    }
    clear_packets();
    struct Packet* pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_LevelExactCheck, 0, 0, 0, 0);
    pckt->chksum = checksum_mem + game.action_random_seed;
    if (LbNetwork_Exchange(pckt, game.packets, sizeof(struct Packet)))
    {
        ERRORLOG("Network exchange failed on level checksum verification");
        result = false;
    }
    if (get_packet(0)->action != get_packet(1)->action)
    {
        // Wait for message from other side
        return CLS_REPEAT;
    }
    if ( checksums_different() )
    {
        ERRORLOG("Level checksums different for network players");
        result = false;
    }
    if (!result)
    {
        coroutine_clear(con, true);

        create_frontend_error_box(5000, get_string(GUIStr_NetUnsyncedMap));
        return CLS_ABORT;
    }
    NETLOG("Checksums are verified");
    return CLS_CONTINUE;
}

// Add value to checksum using simple hash algorithm
#define CHECKSUM_ADD(checksum, value) checksum = checksum * 31 + (ulong)(value)

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player)
{
    if (((player->allocflags & PlaF_CompCtrl) == 0) && (player->acamera != NULL))
    {
        struct Coord3d* mappos = &(player->acamera->mappos);
        TbBigChecksum checksum = 0;
        CHECKSUM_ADD(checksum, player->instance_remain_turns);
        CHECKSUM_ADD(checksum, player->instance_num);
        CHECKSUM_ADD(checksum, mappos->x.val);
        CHECKSUM_ADD(checksum, mappos->y.val);
        CHECKSUM_ADD(checksum, mappos->z.val);
        return checksum;
    }
    return 0;
}

/**
 * Computes checksum of current state of all existing players.
 * @return The checksum value.
 */
TbBigChecksum compute_players_checksum(void)
{
    TbBigChecksum sum = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player))
        {
            sum += compute_player_checksum(player);
        }
    }
    return sum;
}

/**
 * Computes checksum for things in a specific list without updating them.
 * Used for multiplayer synchronization verification to detect desync issues.
 * @param list The thing list to compute checksum for. Can be NULL.
 * @return The checksum value, or 0 if list is NULL or empty.
 */
static TbBigChecksum compute_things_list_checksum(struct StructureList *list)
{
    TbBigChecksum sum = 0;
    unsigned long k = 0;
    int i = list->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected in list");
            break;
        }
        i = thing->next_of_class;
        sum += get_thing_checksum(thing);
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected in thing list");
            break;
        }
    }
    return sum;
}


/**
 * Computes detailed checksums for all thing categories.
 * @param checksums Pointer to structure to store the individual checksums.
 */
static void compute_detailed_things_checksums(struct LogDetailedThingChecksums *checksums)
{
    checksums->creatures = compute_things_list_checksum(&game.thing_lists[TngList_Creatures]);
    checksums->traps = compute_things_list_checksum(&game.thing_lists[TngList_Traps]);
    checksums->shots = compute_things_list_checksum(&game.thing_lists[TngList_Shots]);
    checksums->objects = compute_things_list_checksum(&game.thing_lists[TngList_Objects]);
    checksums->effects = compute_things_list_checksum(&game.thing_lists[TngList_Effects]);
    checksums->dead_creatures = compute_things_list_checksum(&game.thing_lists[TngList_DeadCreatrs]);
    checksums->effect_gens = compute_things_list_checksum(&game.thing_lists[TngList_EffectGens]);
    checksums->doors = compute_things_list_checksum(&game.thing_lists[TngList_Doors]);
}

/**
 * Computes checksum for all things (creatures, traps, shots, objects, effects, etc).
 * @return The combined checksum value for multiplayer sync verification.
 */
static TbBigChecksum compute_things_checksum(void)
{
    TbBigChecksum sum = 0;
    sum += compute_things_list_checksum(&game.thing_lists[TngList_Creatures]);
    sum += compute_things_list_checksum(&game.thing_lists[TngList_Traps]);
    sum += compute_things_list_checksum(&game.thing_lists[TngList_Shots]);
    sum += compute_things_list_checksum(&game.thing_lists[TngList_Objects]);
    sum += compute_things_list_checksum(&game.thing_lists[TngList_Effects]);
    sum += compute_things_list_checksum(&game.thing_lists[TngList_DeadCreatrs]);
    sum += compute_things_list_checksum(&game.thing_lists[TngList_EffectGens]);
    sum += compute_things_list_checksum(&game.thing_lists[TngList_Doors]);
    return sum;
}

/**
 * Computes checksum for a single room based on core room properties.
 * @param room The room to compute checksum for.
 * @return The room checksum value.
 */
static TbBigChecksum get_room_checksum(const struct Room* room)
{
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, room->slabs_count);
    CHECKSUM_ADD(checksum, room->central_stl_x);
    CHECKSUM_ADD(checksum, room->central_stl_y);
    CHECKSUM_ADD(checksum, room->efficiency);
    CHECKSUM_ADD(checksum, room->used_capacity);
    CHECKSUM_ADD(checksum, room->index);
    return checksum;
}

/**
 * Computes checksum for all rooms based on core room properties.
 * @return The rooms checksum value for multiplayer sync verification.
 */
static TbBigChecksum compute_rooms_checksum(void)
{
    TbBigChecksum checksum = 0;
    for (struct Room* room = start_rooms; room < end_rooms; room++)
    {
        if (!room_exists(room)) {
            continue;
        }
        CHECKSUM_ADD(checksum, get_room_checksum(room));
    }
    return checksum;
}

/**
 * Analyzes room mismatch by comparing client pre-resync rooms with host pre-resync rooms.
 * Called when overall room checksum mismatch is detected.
 */
static void log_analyze_room_mismatch_details(void)
{
    ERRORLOG("  Breaking down ROOMS MISMATCH by individual room:");

    int mismatched_count = 0;
    int total_rooms_checked = 0;

    for (int i = 0; i < ROOMS_COUNT; i++) {
        struct LogRoomDesyncInfo* host_info = &game.desync_diagnostics.host_room_info[i];
        struct LogRoomDesyncInfo* client_info = &log_pre_resync_checksums.log_individual_room_info[i];
        TbBigChecksum host_checksum = host_info->checksum;
        TbBigChecksum client_checksum = client_info->checksum;

        if (host_checksum != 0 && client_checksum != 0) {
            total_rooms_checked++;
            if (client_checksum != host_checksum) {
                ERRORLOG("    Room INDEX %d MISMATCH - Client: %08lx vs Host: %08lx", client_info->index, client_checksum, host_checksum);
                ERRORLOG("      CLIENT Room[%d]: Kind:%d Owner:%d Pos:(%d,%d) Slabs:%lu Eff:%ld UsedCap:%ld",
                         client_info->index, client_info->kind, client_info->owner,
                         (int)client_info->central_stl_x, (int)client_info->central_stl_y,
                         client_info->slabs_count, client_info->efficiency, client_info->used_capacity);
                ERRORLOG("      HOST Room[%d]: Kind:%d Owner:%d Pos:(%d,%d) Slabs:%lu Eff:%ld UsedCap:%ld",
                         host_info->index, host_info->kind, host_info->owner,
                         (int)host_info->central_stl_x, (int)host_info->central_stl_y,
                         host_info->slabs_count, host_info->efficiency, host_info->used_capacity);
                mismatched_count++;
            }
        } else if (host_checksum != 0 && client_checksum == 0) {
            ERRORLOG("    Room INDEX %d MISSING on client - Host had checksum: %08lx", host_info->index, host_checksum);
            ERRORLOG("      HOST Room[%d]: Kind:%d Owner:%d Pos:(%d,%d) Slabs:%lu Eff:%ld UsedCap:%ld",
                     host_info->index, host_info->kind, host_info->owner,
                     (int)host_info->central_stl_x, (int)host_info->central_stl_y,
                     host_info->slabs_count, host_info->efficiency, host_info->used_capacity);
            mismatched_count++;
        } else if (host_checksum == 0 && client_checksum != 0) {
            ERRORLOG("    Room INDEX %d EXTRA on client (host has none) - Client checksum: %08lx", client_info->index, client_checksum);
            ERRORLOG("      CLIENT Room[%d]: Kind:%d Owner:%d Pos:(%d,%d) Slabs:%lu Eff:%ld UsedCap:%ld",
                     client_info->index, client_info->kind, client_info->owner,
                     (int)client_info->central_stl_x, (int)client_info->central_stl_y,
                     client_info->slabs_count, client_info->efficiency, client_info->used_capacity);
            mismatched_count++;
        }
    }

    ERRORLOG("  Room analysis: %d mismatches found out of %d total rooms", mismatched_count, total_rooms_checked);
}

/**
 * Counts and logs Things from pre-resync data by category (for diagnostic purposes).
 */
static void log_things_count_by_category(void)
{
    int class_counts[THING_CLASSES_COUNT] = {0};
    int total_things = 0;

    // Use pre-resync stored data instead of current game state
    for (int i = 1; i < THINGS_COUNT; i++) {
        struct LogThingDesyncInfo* client_info = &log_pre_resync_checksums.log_individual_thing_info[i];
        if (client_info->checksum != 0) {
            total_things++;
            if (client_info->class_id < THING_CLASSES_COUNT) {
                class_counts[client_info->class_id]++;
            }
        }
    }

    char log_buffer[512] = "Things count by category (pre-resync): ";
    for (int class_id = 0; class_id < THING_CLASSES_COUNT; class_id++) {
        if (class_counts[class_id] > 0) {
            char temp[64];
            snprintf(temp, sizeof(temp), "%s:%d ", thing_class_code_name(class_id), class_counts[class_id]);
            strcat(log_buffer, temp);
        }
    }
    char temp[32];
    snprintf(temp, sizeof(temp), "TOTAL:%d", total_things);
    strcat(log_buffer, temp);

    ERRORLOG("%s", log_buffer);
}


/**
 * Centralized function to compute all game state checksums for multiplayer sync verification.
 * Used only in multiplayer games to detect desynchronization between networked players.
 */
void compute_multiplayer_checksum_sync(void)
{
    TbBigChecksum things_sum = compute_things_checksum();
    player_packet_checksum_add(my_player_number, things_sum, "things");

    TbBigChecksum rooms_sum = compute_rooms_checksum();
    player_packet_checksum_add(my_player_number, rooms_sum, "rooms");

    TbBigChecksum players_sum = compute_players_checksum();
    player_packet_checksum_add(my_player_number, players_sum, "players");

    // Seeds
    player_packet_checksum_add(my_player_number, game.action_random_seed, "action_random");
    player_packet_checksum_add(my_player_number, game.player_random_seed, "player_random");
    player_packet_checksum_add(my_player_number, game.ai_random_seed, "ai_random");
}

/**
 * Adds given value to checksum at current game turn stored in packet file.
 *
 * @param plyr_idx The player whose checksum is computed.
 * @param sum Checksum increase.
 * @param area_name Name of the area from which the checksum increase comes, for logging purposes.
 */
void player_packet_checksum_add(PlayerNumber plyr_idx, TbBigChecksum sum, const char *area_name)
{
    struct Packet* pckt = get_packet(plyr_idx);
    pckt->chksum += sum;
    SYNCDBG(9,"Checksum increase from %s is %06lX",area_name,(unsigned long)sum);
}

/**
 * Checks if all active players packets have same checksums.
 * @return Returns false if all checksums are same; true if there's mismatch.
 */
short checksums_different(void)
{
    TbChecksum checksum = 0;
    TbBool is_set = false;
    int plyr = -1;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
            struct Packet* pckt = get_packet_direct(player->packet_num);
            if (!is_set) {
                checksum = pckt->chksum;
                is_set = true;
                plyr = i;
            } else if (checksum != pckt->chksum) {
                ERRORLOG("Checksums %08x(%d) != %08x(%d) turn: %ld", checksum, plyr, pckt->chksum, i, game.play_gameturn);
                return true;
            }
        }
    }
    return false;
}

TbBigChecksum get_thing_checksum(const struct Thing* thing)
{
    SYNCDBG(18, "Starting");
    if (!thing_exists(thing)) {
        return 0;
    }
    if (is_non_synchronized_thing_class(thing->class_id)) {
        return 0;
    }

    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, thing->index);
    CHECKSUM_ADD(checksum, thing->class_id);
    CHECKSUM_ADD(checksum, thing->model);
    CHECKSUM_ADD(checksum, thing->owner);
    CHECKSUM_ADD(checksum, thing->creation_turn);
    CHECKSUM_ADD(checksum, thing->mappos.x.val);
    CHECKSUM_ADD(checksum, thing->mappos.y.val);
    CHECKSUM_ADD(checksum, thing->mappos.z.val);
    CHECKSUM_ADD(checksum, thing->health);
    CHECKSUM_ADD(checksum, thing->random_seed);
    CHECKSUM_ADD(checksum, thing->max_frames);
    CHECKSUM_ADD(checksum, thing->current_frame);

    if (thing->class_id == TCls_Creature) {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        CHECKSUM_ADD(checksum, cctrl->inst_turn);
        CHECKSUM_ADD(checksum, cctrl->instance_id);
    }
    return checksum;
}

// Store current game state checksums before resync occurs
// Host uses these to send diagnostic data to clients, clients use for comparison analysis
void store_checksums_for_desync_analysis(void)
{
    compute_detailed_things_checksums(&log_pre_resync_checksums.log_things_detailed);
    log_pre_resync_checksums.log_things_sum = compute_things_checksum();
    log_pre_resync_checksums.log_rooms_sum = compute_rooms_checksum();

    // Store random seeds for individual comparison
    log_pre_resync_checksums.log_action_random_seed = game.action_random_seed;
    log_pre_resync_checksums.log_ai_random_seed = game.ai_random_seed;
    log_pre_resync_checksums.log_player_random_seed = game.player_random_seed;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player)) {
            log_pre_resync_checksums.log_player_checksums[i] = compute_player_checksum(player);
        } else {
            log_pre_resync_checksums.log_player_checksums[i] = 0;
        }
    }

    // Store individual Thing detailed info for detailed desync analysis
    memset(log_pre_resync_checksums.log_individual_thing_info, 0, sizeof(log_pre_resync_checksums.log_individual_thing_info));
    for (int i = 1; i < THINGS_COUNT; i++) {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing) && !is_non_synchronized_thing_class(thing->class_id)) {
            struct LogThingDesyncInfo* info = &log_pre_resync_checksums.log_individual_thing_info[i];
            info->class_id = thing->class_id;
            info->model = thing->model;
            info->owner = thing->owner;
            info->random_seed = thing->random_seed;
            info->pos_x = thing->mappos.x.val;
            info->pos_y = thing->mappos.y.val;
            info->pos_z = thing->mappos.z.val;
            info->creation_turn = thing->creation_turn;
            info->index = thing->index;
            info->health = thing->health;
            info->max_frames = thing->max_frames;
            info->current_frame = thing->current_frame;
            info->checksum = get_thing_checksum(thing);
        }
    }

    // Store individual Room detailed info for detailed desync analysis
    memset(log_pre_resync_checksums.log_individual_room_info, 0, sizeof(log_pre_resync_checksums.log_individual_room_info));
    for (struct Room* room = start_rooms; room < end_rooms; room++) {
        if (room_exists(room)) {
            struct LogRoomDesyncInfo* info = &log_pre_resync_checksums.log_individual_room_info[room->index];
            info->kind = room->kind;
            info->owner = room->owner;
            info->central_stl_x = room->central_stl_x;
            info->central_stl_y = room->central_stl_y;
            info->slabs_count = room->slabs_count;
            info->efficiency = room->efficiency;
            info->used_capacity = room->used_capacity;
            info->index = room->index;
            info->checksum = get_room_checksum(room);
        }
    }

    log_pre_resync_checksums.log_gameturn = game.play_gameturn;
    log_pre_resync_checksums.log_is_stored = true;

    ERRORLOG("Stored pre-resync checksums at turn %ld: things=%08lx rooms=%08lx",
            log_pre_resync_checksums.log_gameturn, log_pre_resync_checksums.log_things_sum, log_pre_resync_checksums.log_rooms_sum);
}



// Log checksum comparison between client local state and host diagnostic data
// Shows whether specific category matches or differs to pinpoint desync source
static void log_checksum_comparison(const char* category_name, TbBigChecksum client_checksum, TbBigChecksum host_checksum)
{
    const char* status = (client_checksum != host_checksum) ? "MISMATCH" : "match";
    ERRORLOG("    %s %s - Client: %08lx, Host: %08lx", category_name, status, client_checksum, host_checksum);
}

// Client breaks down things checksum mismatch by category to identify desync source
// Compares client's pre-resync thing lists with host's diagnostic data
static void log_analyze_things_mismatch_details(void)
{
    ERRORLOG("  Breaking down THINGS MISMATCH by category:");

    // Use detailed checksums computed before resync to ensure consistent comparison
    struct LogDetailedThingChecksums client_checksums = log_pre_resync_checksums.log_things_detailed;

    // Compare each thing category between client and host pre-resync states
    log_checksum_comparison("Creatures", client_checksums.creatures, game.desync_diagnostics.host_creatures_sum);
    log_checksum_comparison("Traps", client_checksums.traps, game.desync_diagnostics.host_traps_sum);
    log_checksum_comparison("Shots", client_checksums.shots, game.desync_diagnostics.host_shots_sum);
    log_checksum_comparison("Objects", client_checksums.objects, game.desync_diagnostics.host_objects_sum);
    log_checksum_comparison("Effects", client_checksums.effects, game.desync_diagnostics.host_effects_sum);
    log_checksum_comparison("Dead Creatures", client_checksums.dead_creatures, game.desync_diagnostics.host_dead_creatures_sum);
    log_checksum_comparison("Effect Generators", client_checksums.effect_gens, game.desync_diagnostics.host_effect_gens_sum);
    log_checksum_comparison("Doors", client_checksums.doors, game.desync_diagnostics.host_doors_sum);

}

// Compare individual Thing checksums between client pre-resync state and host's pre-resync state
// Logs specific Things that have different checksums to pinpoint desync sources
static void log_analyze_individual_thing_differences(void)
{
    int mismatched_count = 0;

    ERRORLOG("  Analyzing individual Thing checksums:");

    for (int i = 1; i < THINGS_COUNT; i++) {
        struct LogThingDesyncInfo* host_info = &game.desync_diagnostics.host_thing_info[i];
        struct LogThingDesyncInfo* client_info = &log_pre_resync_checksums.log_individual_thing_info[i];
        TbBigChecksum host_checksum = host_info->checksum;
        TbBigChecksum client_checksum = client_info->checksum;

        // Skip things that were non-synchronized based on stored pre-resync data
        if (host_checksum != 0 && is_non_synchronized_thing_class(host_info->class_id)) {
            continue;
        }
        if (client_checksum != 0 && is_non_synchronized_thing_class(client_info->class_id)) {
            continue;
        }

        if (host_checksum != 0 && client_checksum != 0) {
            if (client_checksum != host_checksum) {
                ERRORLOG("    Thing INDEX %d MISMATCH - Client: %08lx vs Host: %08lx", i, client_checksum, host_checksum);
                ERRORLOG("      CLIENT Thing[%d]: %s/%s owner:%d pos_val:(%ld,%ld,%ld) pos_stl:(%d,%d,%d) health:%ld seed:%08lx creation_turn:%ld frames:%u/%u",
                         client_info->index, thing_class_code_name(client_info->class_id),
                         thing_class_and_model_name(client_info->class_id, client_info->model),
                         client_info->owner,
                         client_info->pos_x, client_info->pos_y, client_info->pos_z,
                         (int)(client_info->pos_x >> 8), (int)(client_info->pos_y >> 8), (int)(client_info->pos_z >> 8),
                         client_info->health, client_info->random_seed, client_info->creation_turn,
                         client_info->current_frame, client_info->max_frames);
                ERRORLOG("      HOST Thing[%d]: %s/%s owner:%d pos_val:(%ld,%ld,%ld) pos_stl:(%d,%d,%d) health:%ld seed:%08lx creation_turn:%ld frames:%u/%u",
                         host_info->index, thing_class_code_name(host_info->class_id),
                         thing_class_and_model_name(host_info->class_id, host_info->model),
                         host_info->owner,
                         host_info->pos_x, host_info->pos_y, host_info->pos_z,
                         (int)(host_info->pos_x >> 8), (int)(host_info->pos_y >> 8), (int)(host_info->pos_z >> 8),
                         host_info->health, host_info->random_seed, host_info->creation_turn,
                         host_info->current_frame, host_info->max_frames);
                mismatched_count++;
            }
        } else if (host_checksum != 0 && client_checksum == 0) {
            ERRORLOG("    Thing INDEX %d MISSING on client - Host had checksum: %08lx", i, host_checksum);
            ERRORLOG("      HOST Thing[%d]: %s/%s owner:%d pos_val:(%ld,%ld,%ld) pos_stl:(%d,%d,%d) health:%ld seed:%08lx creation_turn:%ld frames:%u/%u",
                     host_info->index, thing_class_code_name(host_info->class_id),
                     thing_class_and_model_name(host_info->class_id, host_info->model),
                     host_info->owner,
                     host_info->pos_x, host_info->pos_y, host_info->pos_z,
                     (int)(host_info->pos_x >> 8), (int)(host_info->pos_y >> 8), (int)(host_info->pos_z >> 8),
                     host_info->health, host_info->random_seed, host_info->creation_turn,
                     host_info->current_frame, host_info->max_frames);
            mismatched_count++;
        } else if (host_checksum == 0 && client_checksum != 0) {
            ERRORLOG("    Thing INDEX %d EXTRA on client (host has none) - Client checksum: %08lx", i, client_checksum);
            ERRORLOG("      CLIENT Thing[%d]: %s/%s owner:%d pos_val:(%ld,%ld,%ld) pos_stl:(%d,%d,%d) health:%ld seed:%08lx creation_turn:%ld frames:%u/%u",
                     client_info->index, thing_class_code_name(client_info->class_id),
                     thing_class_and_model_name(client_info->class_id, client_info->model),
                     client_info->owner,
                     client_info->pos_x, client_info->pos_y, client_info->pos_z,
                     (int)(client_info->pos_x >> 8), (int)(client_info->pos_y >> 8), (int)(client_info->pos_z >> 8),
                     client_info->health, client_info->random_seed, client_info->creation_turn,
                     client_info->current_frame, client_info->max_frames);
            mismatched_count++;
        }
    }

    ERRORLOG("  Individual Thing analysis: %d mismatches found", mismatched_count);
}


// Client compares local pre-resync checksums with host diagnostic data
// This analysis helps identify which game systems diverged between host and client
void log_analyze_desync_diagnostics_from_host(void)
{
    ERRORLOG("=== DESYNC ANALYSIS: Client (turn %ld) vs Host (turn %ld) ===", log_pre_resync_checksums.log_gameturn, game.desync_diagnostics.desync_turn);

    // Log Things count by category
    log_things_count_by_category();

    // Compare client pre-resync state with host pre-resync state from diagnostic data
    log_checksum_comparison("Things", log_pre_resync_checksums.log_things_sum, game.desync_diagnostics.host_things_sum);
    if (log_pre_resync_checksums.log_things_sum != game.desync_diagnostics.host_things_sum) {
        log_analyze_things_mismatch_details();
        log_analyze_individual_thing_differences();
    }

    log_checksum_comparison("Rooms", log_pre_resync_checksums.log_rooms_sum, game.desync_diagnostics.host_rooms_sum);
    if (log_pre_resync_checksums.log_rooms_sum != game.desync_diagnostics.host_rooms_sum) {
        log_analyze_room_mismatch_details();
    }

    // Check each random seed individually
    log_checksum_comparison("GAME_RANDOM seed", log_pre_resync_checksums.log_action_random_seed, game.desync_diagnostics.host_action_random_seed);
    log_checksum_comparison("AI_RANDOM seed", log_pre_resync_checksums.log_ai_random_seed, game.desync_diagnostics.host_ai_random_seed);
    log_checksum_comparison("PLAYER_RANDOM seed", log_pre_resync_checksums.log_player_random_seed, game.desync_diagnostics.host_player_random_seed);

    // Check each player individually
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        TbBigChecksum client_checksum = log_pre_resync_checksums.log_player_checksums[i];
        TbBigChecksum host_checksum = game.desync_diagnostics.host_player_checksums[i];

        if (player_exists(player)) {
            if (client_checksum != host_checksum) {
                ERRORLOG("Player %d MISMATCH - Client: %08lx, Host: %08lx, exists=1, computer=%d",
                         i, client_checksum, host_checksum, (player->allocflags & PlaF_CompCtrl) ? 1 : 0);
            }
        } else if (host_checksum != 0) {
            ERRORLOG("Player %d MISMATCH - Client: does not exist (%08lx), Host: %08lx",
                     i, client_checksum, host_checksum);
        }
    }

    ERRORLOG("=== END DESYNC ANALYSIS ===");

    // Clear the diagnostic data to avoid confusion on subsequent resyncs
    game.desync_diagnostics.has_desync_diagnostics = false;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
