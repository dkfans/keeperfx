/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file desync_analysis.c
 *     Desync analysis history tracking for network game debugging.
 * @par Purpose:
 *     Stores and retrieves checksum history for multiplayer desync debugging.
 * @par Comment:
 *     Uses a circular buffer to store the last N turns of checksum data.
 * @author   KeeperFX Team
 * @date     03 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "desync_analysis.h"

#include "globals.h"
#include "bflib_basics.h"
#include "game_legacy.h"
#include "net_sync.h"
#include "player_data.h"
#include "thing_data.h"
#include "room_data.h"
#include "room_list.h"
#include "creature_control.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define DESYNC_HISTORY_TURNS 40

static struct {
    struct DesyncHistoryEntry entries[DESYNC_HISTORY_TURNS];
    int head;
} local_desync_history;

/******************************************************************************/
static void clear_desync_entry(struct DesyncHistoryEntry* entry) {
    entry->turn = 10000000;
    entry->valid = false;
    entry->things_sum = 0;
    entry->rooms_sum = 0;
    entry->action_random_seed = 0;
    entry->ai_random_seed = 0;
    entry->player_random_seed = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        entry->player_checksums[i] = 0;
    }
    entry->creatures_sum = 0;
    entry->traps_sum = 0;
    entry->shots_sum = 0;
    entry->objects_sum = 0;
    entry->effects_sum = 0;
    entry->dead_creatures_sum = 0;
    entry->effect_gens_sum = 0;
    entry->doors_sum = 0;
}

void clear_desync_analysis(void) {
    for (int i = 0; i < DESYNC_HISTORY_TURNS; ++i) {
        clear_desync_entry(&local_desync_history.entries[i]);
    }
    local_desync_history.head = 0;
}

void initialize_desync_analysis(void) {
    clear_desync_analysis();
}

static TbBigChecksum compute_things_list_checksum(struct StructureList *list) {
    TbBigChecksum sum = 0;
    unsigned long k = 0;
    int i = list->index;
    while (i != 0) {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Jump to invalid thing detected in list");
            break;
        }
        i = thing->next_of_class;
        sum += get_thing_checksum(thing);
        k++;
        if (k > THINGS_COUNT) {
            ERRORLOG("Infinite loop detected in thing list");
            break;
        }
    }
    return sum;
}

static TbBigChecksum compute_things_checksum(void) {
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

#define CHECKSUM_ADD(checksum, value) checksum = checksum * 31 + (ulong)(value)

static TbBigChecksum get_room_checksum(const struct Room* room) {
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, room->slabs_count);
    CHECKSUM_ADD(checksum, room->central_stl_x);
    CHECKSUM_ADD(checksum, room->central_stl_y);
    CHECKSUM_ADD(checksum, room->efficiency);
    CHECKSUM_ADD(checksum, room->used_capacity);
    CHECKSUM_ADD(checksum, room->index);
    return checksum;
}

static TbBigChecksum compute_rooms_checksum(void) {
    TbBigChecksum checksum = 0;
    for (struct Room* room = start_rooms; room < end_rooms; room++) {
        if (!room_exists(room)) {
            continue;
        }
        CHECKSUM_ADD(checksum, get_room_checksum(room));
    }
    return checksum;
}

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player) {
    if (((player->allocflags & PlaF_CompCtrl) == 0) && (player->acamera != NULL)) {
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

void store_turn_checksums(void) {
    int index = local_desync_history.head;
    struct DesyncHistoryEntry* entry = &local_desync_history.entries[index];

    entry->turn = game.play_gameturn;
    entry->valid = true;

    entry->things_sum = compute_things_checksum();
    entry->rooms_sum = compute_rooms_checksum();
    entry->action_random_seed = game.action_random_seed;
    entry->ai_random_seed = game.ai_random_seed;
    entry->player_random_seed = game.player_random_seed;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player)) {
            entry->player_checksums[i] = compute_player_checksum(player);
        } else {
            entry->player_checksums[i] = 0;
        }
    }

    entry->creatures_sum = compute_things_list_checksum(&game.thing_lists[TngList_Creatures]);
    entry->traps_sum = compute_things_list_checksum(&game.thing_lists[TngList_Traps]);
    entry->shots_sum = compute_things_list_checksum(&game.thing_lists[TngList_Shots]);
    entry->objects_sum = compute_things_list_checksum(&game.thing_lists[TngList_Objects]);
    entry->effects_sum = compute_things_list_checksum(&game.thing_lists[TngList_Effects]);
    entry->dead_creatures_sum = compute_things_list_checksum(&game.thing_lists[TngList_DeadCreatrs]);
    entry->effect_gens_sum = compute_things_list_checksum(&game.thing_lists[TngList_EffectGens]);
    entry->doors_sum = compute_things_list_checksum(&game.thing_lists[TngList_Doors]);

    local_desync_history.head = (index + 1) % DESYNC_HISTORY_TURNS;

    MULTIPLAYER_LOG("store_turn_checksums: turn=%lu things=%08lx rooms=%08lx",
            (unsigned long)entry->turn, (unsigned long)entry->things_sum, (unsigned long)entry->rooms_sum);
}

void pack_desync_history_for_resync(void) {
    memcpy(game.desync_diagnostics.host_history, local_desync_history.entries, sizeof(game.desync_diagnostics.host_history));
    game.desync_diagnostics.has_desync_diagnostics = true;
    ERRORLOG("Host packed desync history for resync");
}

static void log_checksum_comparison(const char* category_name, TbBigChecksum client_checksum, TbBigChecksum host_checksum) {
    const char* status = (client_checksum != host_checksum) ? "MISMATCH" : "match";
    ERRORLOG("    %s %s - Client: %08lx, Host: %08lx", category_name, status, client_checksum, host_checksum);
}

static void log_things_count_by_category(void) {
    int obj_count = 0;
    int creature_count = 0;
    int effectgen_count = 0;
    int total = 0;

    struct StructureList* list = &game.thing_lists[TngList_Objects];
    int i = list->index;
    while (i != 0) {
        obj_count++;
        total++;
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            break;
        }
        i = thing->next_of_class;
    }

    list = &game.thing_lists[TngList_Creatures];
    i = list->index;
    while (i != 0) {
        creature_count++;
        total++;
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            break;
        }
        i = thing->next_of_class;
    }

    list = &game.thing_lists[TngList_EffectGens];
    i = list->index;
    while (i != 0) {
        effectgen_count++;
        total++;
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            break;
        }
        i = thing->next_of_class;
    }

    ERRORLOG("Things count by category (pre-resync): OBJECT:%d CREATURE:%d EFFECTGEN:%d TOTAL:%d",
             obj_count, creature_count, effectgen_count, total);
}

static void log_analyze_things_mismatch_details(struct DesyncHistoryEntry* client_entry, struct DesyncHistoryEntry* host_entry) {
    ERRORLOG("  Breaking down THINGS MISMATCH by category:");
    log_checksum_comparison("Creatures", client_entry->creatures_sum, host_entry->creatures_sum);
    log_checksum_comparison("Traps", client_entry->traps_sum, host_entry->traps_sum);
    log_checksum_comparison("Shots", client_entry->shots_sum, host_entry->shots_sum);
    log_checksum_comparison("Objects", client_entry->objects_sum, host_entry->objects_sum);
    log_checksum_comparison("Effects", client_entry->effects_sum, host_entry->effects_sum);
    log_checksum_comparison("Dead Creatures", client_entry->dead_creatures_sum, host_entry->dead_creatures_sum);
    log_checksum_comparison("Effect Generators", client_entry->effect_gens_sum, host_entry->effect_gens_sum);
    log_checksum_comparison("Doors", client_entry->doors_sum, host_entry->doors_sum);
}

static void log_analyze_desync_diagnostics_from_host(struct DesyncHistoryEntry* client_entry, struct DesyncHistoryEntry* host_entry, GameTurn client_turn, GameTurn host_turn) {
    ERRORLOG("=== DESYNC ANALYSIS: Client (turn %lu) vs Host (turn %lu) ===",
             (unsigned long)client_turn, (unsigned long)host_turn);

    log_things_count_by_category();

    log_checksum_comparison("Things", client_entry->things_sum, host_entry->things_sum);

    if (client_entry->things_sum != host_entry->things_sum) {
        log_analyze_things_mismatch_details(client_entry, host_entry);
    }

    log_checksum_comparison("Objects", client_entry->objects_sum, host_entry->objects_sum);
    log_checksum_comparison("Effects", client_entry->effects_sum, host_entry->effects_sum);
    log_checksum_comparison("Dead Creatures", client_entry->dead_creatures_sum, host_entry->dead_creatures_sum);
    log_checksum_comparison("Effect Generators", client_entry->effect_gens_sum, host_entry->effect_gens_sum);
    log_checksum_comparison("Doors", client_entry->doors_sum, host_entry->doors_sum);
    log_checksum_comparison("Rooms", client_entry->rooms_sum, host_entry->rooms_sum);
    log_checksum_comparison("GAME_RANDOM seed", client_entry->action_random_seed, host_entry->action_random_seed);
    log_checksum_comparison("AI_RANDOM seed", client_entry->ai_random_seed, host_entry->ai_random_seed);
    log_checksum_comparison("PLAYER_RANDOM seed", client_entry->player_random_seed, host_entry->player_random_seed);

    ERRORLOG("=== END DESYNC ANALYSIS ===");
}

void compare_desync_history_from_host(void) {
    GameTurn client_current_turn = game.play_gameturn;
    GameTurn host_current_turn = 0;

    for (int i = 0; i < DESYNC_HISTORY_TURNS; i++) {
        if (game.desync_diagnostics.host_history[i].valid) {
            if (host_current_turn == 0 || game.desync_diagnostics.host_history[i].turn > host_current_turn) {
                host_current_turn = game.desync_diagnostics.host_history[i].turn;
            }
        }
    }

    GameTurn earliest_desync_turn = (client_current_turn < host_current_turn) ? client_current_turn : host_current_turn;

    struct DesyncHistoryEntry* client_entry = NULL;
    struct DesyncHistoryEntry* host_entry = NULL;

    for (int i = 0; i < DESYNC_HISTORY_TURNS; i++) {
        if (local_desync_history.entries[i].valid && local_desync_history.entries[i].turn == earliest_desync_turn) {
            client_entry = &local_desync_history.entries[i];
            break;
        }
    }

    for (int i = 0; i < DESYNC_HISTORY_TURNS; i++) {
        if (game.desync_diagnostics.host_history[i].valid && game.desync_diagnostics.host_history[i].turn == earliest_desync_turn) {
            host_entry = &game.desync_diagnostics.host_history[i];
            break;
        }
    }

    if (client_entry == NULL || host_entry == NULL) {
        ERRORLOG("No desync history found, so the cause of the desync was likely from a packet not being received, which means the input lag was set too low.");
        game.desync_diagnostics.has_desync_diagnostics = false;
        return;
    }

    log_analyze_desync_diagnostics_from_host(client_entry, host_entry, client_current_turn, host_current_turn);

    game.desync_diagnostics.has_desync_diagnostics = false;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
