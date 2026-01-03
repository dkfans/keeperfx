/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_checksums.c
 *     Network checksum computation and desync analysis for multiplayer games.
 * @par Purpose:
 *     Computes checksums for multiplayer sync and stores history for debugging.
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
#include "net_checksums.h"
#include "bflib_network_exchange.h"
#include "game_legacy.h"
#include "net_game.h"
#include "packets.h"
#include "player_data.h"
#include "thing_data.h"
#include "room_list.h"
#include "creature_control.h"
#include "frontend.h"
#include "thing_list.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define CHECKSUM_ADD(checksum, value) checksum = ((checksum << 5) | (checksum >> 27)) ^ (ulong)(value)
#define SNAPSHOT_BUFFER_SIZE 15

struct ChecksumSnapshot {
    GameTurn turn;
    TbBool valid;
    struct DesyncChecksums checksums;
    struct LogDetailedSnapshot log_details;
};

static struct ChecksumSnapshot snapshot_buffer[SNAPSHOT_BUFFER_SIZE];
static int snapshot_head = 0;
static GameTurn desync_turn = 0;

TbBigChecksum get_thing_checksum(const struct Thing* thing) {
    if (!thing_exists(thing) || is_non_synchronized_thing_class(thing->class_id)) {
        return 0;
    }
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, thing->index);
    CHECKSUM_ADD(checksum, thing->class_id);
    CHECKSUM_ADD(checksum, thing->model);
    CHECKSUM_ADD(checksum, thing->owner);
    CHECKSUM_ADD(checksum, thing->creation_turn);
    CHECKSUM_ADD(checksum, thing->random_seed);
    CHECKSUM_ADD(checksum, thing->mappos.x.val);
    CHECKSUM_ADD(checksum, thing->mappos.y.val);
    CHECKSUM_ADD(checksum, thing->mappos.z.val);
    CHECKSUM_ADD(checksum, thing->health);
    CHECKSUM_ADD(checksum, thing->current_frame);
    CHECKSUM_ADD(checksum, thing->max_frames);

    if (thing->class_id == TCls_Creature) {
        struct CreatureControl* creature_control = creature_control_get_from_thing(thing);
        CHECKSUM_ADD(checksum, creature_control->inst_turn);
        CHECKSUM_ADD(checksum, creature_control->instance_id);
    }
    return checksum;
}

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player) {
    if ((player->allocflags & PlaF_CompCtrl) != 0 || player->acamera == NULL) {
        return 0;
    }
    struct Coord3d* mappos = &(player->acamera->mappos);
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, player->instance_remain_turns);
    CHECKSUM_ADD(checksum, player->instance_num);
    CHECKSUM_ADD(checksum, mappos->x.val);
    CHECKSUM_ADD(checksum, mappos->y.val);
    CHECKSUM_ADD(checksum, mappos->z.val);
    return checksum;
}

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

static void compute_checksums(struct DesyncChecksums* checksums) {
    checksums->creatures = compute_things_list_checksum(&game.thing_lists[TngList_Creatures]);
    checksums->traps = compute_things_list_checksum(&game.thing_lists[TngList_Traps]);
    checksums->shots = compute_things_list_checksum(&game.thing_lists[TngList_Shots]);
    checksums->objects = compute_things_list_checksum(&game.thing_lists[TngList_Objects]);
    checksums->effects = compute_things_list_checksum(&game.thing_lists[TngList_Effects]);
    checksums->dead_creatures = compute_things_list_checksum(&game.thing_lists[TngList_DeadCreatrs]);
    checksums->effect_gens = compute_things_list_checksum(&game.thing_lists[TngList_EffectGens]);
    checksums->doors = compute_things_list_checksum(&game.thing_lists[TngList_Doors]);
    checksums->rooms = 0;
    for (struct Room* room = start_rooms; room < end_rooms; room++) {
        if (room_exists(room)) {
            CHECKSUM_ADD(checksums->rooms, get_room_checksum(room));
        }
    }
    checksums->players = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player)) {
            checksums->players += compute_player_checksum(player);
        }
    }
    checksums->action_seed = game.action_random_seed;
    checksums->ai_seed = game.ai_random_seed;
    checksums->player_seed = game.player_random_seed;
    checksums->game_turn = game.play_gameturn;
}

static struct ChecksumSnapshot* find_snapshot(GameTurn turn) {
    for (int i = 0; i < SNAPSHOT_BUFFER_SIZE; i++) {
        if (snapshot_buffer[i].valid && snapshot_buffer[i].turn == turn) {
            return &snapshot_buffer[i];
        }
    }
    return NULL;
}

short checksums_different(void) {
    int host_player_id = get_host_player_id();
    struct Packet* host_packet = get_packet(host_player_id);
    TbBigChecksum host_checksum = host_packet->checksum;
    TbBool mismatch = false;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (i == host_player_id || !player_exists(player) || ((player->allocflags & PlaF_CompCtrl) != 0)) {
            continue;
        }
        struct Packet* packet = get_packet_direct(player->packet_num);
        if (is_packet_empty(packet)) {
            MULTIPLAYER_LOG("checksums_different: packet[%d] is EMPTY", i);
            mismatch = true;
            continue;
        }
        if (packet->checksum != host_checksum) {
            ERRORLOG("Checksums %08lx(Host) != %08lx(Client) turn: %d vs %d", host_checksum, packet->checksum, host_packet->turn, packet->turn);
            desync_turn = host_packet->turn;
            mismatch = true;
        }
    }
    return mismatch;
}

void update_turn_checksums(void) {
    struct ChecksumSnapshot* snapshot = &snapshot_buffer[snapshot_head];
    struct LogDetailedSnapshot* snapshot_info = &snapshot->log_details;
    snapshot->turn = game.play_gameturn;
    snapshot->valid = true;
    compute_checksums(&snapshot->checksums);
    snapshot_info->thing_count = 0;
    snapshot_info->player_count = 0;
    snapshot_info->room_count = 0;
    if ((game.system_flags & GSF_NetworkActive) != 0) {
        for (int i = 1; i < SYNCED_THINGS_COUNT; i++) {
            struct Thing* thing = thing_get(i);
            if (!thing_exists(thing) || is_non_synchronized_thing_class(thing->class_id)) {
                continue;
            }
            struct LogThingDesyncInfo* thing_snapshot = &snapshot_info->things[snapshot_info->thing_count++];
            thing_snapshot->index = thing->index;
            thing_snapshot->class_id = thing->class_id;
            thing_snapshot->model = thing->model;
            thing_snapshot->owner = thing->owner;
            thing_snapshot->mappos = thing->mappos;
            thing_snapshot->health = thing->health;
            thing_snapshot->creation_turn = thing->creation_turn;
            thing_snapshot->random_seed = thing->random_seed;
            thing_snapshot->checksum = get_thing_checksum(thing);
        }
        for (int i = 0; i < PLAYERS_COUNT; i++) {
            struct PlayerInfo* player = get_player(i);
            if (!player_exists(player) || ((player->allocflags & PlaF_CompCtrl) != 0) || player->acamera == NULL) {
                continue;
            }
            struct LogPlayerDesyncInfo* player_snapshot = &snapshot_info->players[snapshot_info->player_count++];
            player_snapshot->id = i;
            player_snapshot->instance_num = player->instance_num;
            player_snapshot->instance_remain_turns = player->instance_remain_turns;
            player_snapshot->mappos = player->acamera->mappos;
            player_snapshot->checksum = compute_player_checksum(player);
        }
        for (struct Room* room = start_rooms; room < end_rooms; room++) {
            if (!room_exists(room)) {
                continue;
            }
            struct LogRoomDesyncInfo* room_snapshot = &snapshot_info->rooms[snapshot_info->room_count++];
            room_snapshot->index = room->index;
            room_snapshot->slabs_count = room->slabs_count;
            room_snapshot->central_stl_x = room->central_stl_x;
            room_snapshot->central_stl_y = room->central_stl_y;
            room_snapshot->efficiency = room->efficiency;
            room_snapshot->used_capacity = room->used_capacity;
            room_snapshot->checksum = get_room_checksum(room);
        }
    }
    snapshot_head = (snapshot_head + 1) % SNAPSHOT_BUFFER_SIZE;

    struct DesyncChecksums* checksums = &snapshot->checksums;
    TbBigChecksum things_sum = 0;
    things_sum += checksums->creatures;
    things_sum += checksums->traps;
    things_sum += checksums->shots;
    things_sum += checksums->objects;
    things_sum += checksums->effects;
    things_sum += checksums->dead_creatures;
    things_sum += checksums->effect_gens;
    things_sum += checksums->doors;

    struct Packet* packet = get_packet(my_player_number);
    packet->checksum = 0;
    packet->checksum += things_sum;
    packet->checksum += checksums->rooms;
    packet->checksum += checksums->players;
    packet->checksum += checksums->action_seed;
    packet->checksum += checksums->player_seed;
    packet->checksum += checksums->ai_seed;

    MULTIPLAYER_LOG("update_turn_checksums: turn=%lu checksum=%08lx things=%08lx rooms=%08lx players=%08lx", (unsigned long)game.play_gameturn, (unsigned long)packet->checksum, (unsigned long)things_sum, (unsigned long)checksums->rooms, (unsigned long)checksums->players);
}

void pack_desync_history_for_resync(void) {
    struct ChecksumSnapshot* snapshot = find_snapshot(desync_turn);
    if (snapshot == NULL) {
        compute_checksums(&game.host_checksums);
        game.log_snapshot.thing_count = 0;
        game.log_snapshot.player_count = 0;
        game.log_snapshot.room_count = 0;
        return;
    }
    game.host_checksums = snapshot->checksums;
    game.log_snapshot = snapshot->log_details;
}

static void log_thing_differences(struct LogDetailedSnapshot* client, const char* name, TbBigChecksum client_sum, TbBigChecksum host_sum, ThingClass filter_class) {
    ERRORLOG("  %s %s - Host: %08lx, Client: %08lx", name, (client_sum == host_sum) ? "match" : "MISMATCH", (unsigned long)host_sum, (unsigned long)client_sum);
    if (client_sum == host_sum) {
        return;
    }
    struct LogDetailedSnapshot* host = &game.log_snapshot;
    int shown = 0;
    for (int s = 0; s < client->thing_count && shown < 10; s++) {
        struct LogThingDesyncInfo* client_thing = &client->things[s];
        if (client_thing->class_id != filter_class) {
            continue;
        }
        struct LogThingDesyncInfo* host_thing = NULL;
        for (int h = 0; h < host->thing_count; h++) {
            if (host->things[h].index == client_thing->index) {
                host_thing = &host->things[h];
                break;
            }
        }
        if (host_thing == NULL || client_thing->checksum != host_thing->checksum) {
            if (host_thing != NULL) {
                ERRORLOG("    [Host] Thing[%d] class_id=%d model=%d owner=%d mappos=(%ld,%ld,%ld) health=%ld creation_turn=%lu random_seed=%08lx", host_thing->index, host_thing->class_id, host_thing->model, host_thing->owner, (long)host_thing->mappos.x.val, (long)host_thing->mappos.y.val, (long)host_thing->mappos.z.val, (long)host_thing->health, (unsigned long)host_thing->creation_turn, host_thing->random_seed);
            } else {
                ERRORLOG("    [Host] Thing[%d] missing", client_thing->index);
            }
            ERRORLOG("    [Client] Thing[%d] class_id=%d model=%d owner=%d mappos=(%ld,%ld,%ld) health=%ld creation_turn=%lu random_seed=%08lx", client_thing->index, client_thing->class_id, client_thing->model, client_thing->owner, (long)client_thing->mappos.x.val, (long)client_thing->mappos.y.val, (long)client_thing->mappos.z.val, (long)client_thing->health, (unsigned long)client_thing->creation_turn, client_thing->random_seed);
            shown++;
        }
    }
}

void compare_desync_history_from_host(void) {
    struct ChecksumSnapshot* snapshot = find_snapshot(desync_turn);
    if (snapshot == NULL) {
        ERRORLOG("=== DESYNC: No client snapshot for turn %lu ===", (unsigned long)desync_turn);
        return;
    }
    struct DesyncChecksums* host = &game.host_checksums;
    struct DesyncChecksums* client = &snapshot->checksums;
    struct LogDetailedSnapshot* host_snapshot = &game.log_snapshot;
    struct LogDetailedSnapshot* client_snapshot = &snapshot->log_details;

    ERRORLOG("=== DESYNC ANALYSIS: Host (turn %lu) vs Client (turn %lu) ===", (unsigned long)host->game_turn, (unsigned long)client->game_turn);
    ERRORLOG("  ACTION_SEED %s - Host: %08lx, Client: %08lx", (client->action_seed == host->action_seed) ? "match" : "MISMATCH", (unsigned long)host->action_seed, (unsigned long)client->action_seed);
    ERRORLOG("  AI_SEED %s - Host: %08lx, Client: %08lx", (client->ai_seed == host->ai_seed) ? "match" : "MISMATCH", (unsigned long)host->ai_seed, (unsigned long)client->ai_seed);
    ERRORLOG("  PLAYER_SEED %s - Host: %08lx, Client: %08lx", (client->player_seed == host->player_seed) ? "match" : "MISMATCH", (unsigned long)host->player_seed, (unsigned long)client->player_seed);
    ERRORLOG("  Players %s - Host: %08lx, Client: %08lx", (client->players == host->players) ? "match" : "MISMATCH", (unsigned long)host->players, (unsigned long)client->players);
    if (client->players != host->players) {
        for (int i = 0; i < client_snapshot->player_count; i++) {
            struct LogPlayerDesyncInfo* client_player = &client_snapshot->players[i];
            struct LogPlayerDesyncInfo* host_player = NULL;
            for (int j = 0; j < host_snapshot->player_count; j++) {
                if (host_snapshot->players[j].id == client_player->id) {
                    host_player = &host_snapshot->players[j];
                    break;
                }
            }
            if (host_player == NULL) {
                ERRORLOG("    Player[%d] missing from host", client_player->id);
            } else if (client_player->checksum != host_player->checksum) {
                ERRORLOG("    Player[%d] instance_num: Host=%u Client=%u, instance_remain_turns: Host=%lu Client=%lu, mappos: Host=(%ld,%ld,%ld) Client=(%ld,%ld,%ld)", client_player->id,
                    (unsigned)host_player->instance_num, (unsigned)client_player->instance_num,
                    (unsigned long)host_player->instance_remain_turns, (unsigned long)client_player->instance_remain_turns,
                    (long)host_player->mappos.x.val, (long)host_player->mappos.y.val, (long)host_player->mappos.z.val,
                    (long)client_player->mappos.x.val, (long)client_player->mappos.y.val, (long)client_player->mappos.z.val);
            }
        }
    }
    ERRORLOG("  Rooms %s - Host: %08lx, Client: %08lx", (client->rooms == host->rooms) ? "match" : "MISMATCH", (unsigned long)host->rooms, (unsigned long)client->rooms);
    if (client->rooms != host->rooms) {
        int shown = 0;
        for (int i = 0; i < client_snapshot->room_count && shown < 10; i++) {
            struct LogRoomDesyncInfo* client_room = &client_snapshot->rooms[i];
            struct LogRoomDesyncInfo* host_room = NULL;
            for (int j = 0; j < host_snapshot->room_count; j++) {
                if (host_snapshot->rooms[j].index == client_room->index) {
                    host_room = &host_snapshot->rooms[j];
                    break;
                }
            }
            if (host_room == NULL) {
                ERRORLOG("    Room[%d] missing from host", client_room->index);
                shown++;
            } else if (client_room->checksum != host_room->checksum) {
                ERRORLOG("    Room[%d] slabs_count: Host=%d Client=%d, efficiency: Host=%d Client=%d",
                    client_room->index, host_room->slabs_count, client_room->slabs_count, host_room->efficiency, client_room->efficiency);
                shown++;
            }
        }
    }
    log_thing_differences(client_snapshot, "Creatures", client->creatures, host->creatures, TCls_Creature);
    log_thing_differences(client_snapshot, "Traps", client->traps, host->traps, TCls_Trap);
    log_thing_differences(client_snapshot, "Shots", client->shots, host->shots, TCls_Shot);
    log_thing_differences(client_snapshot, "Objects", client->objects, host->objects, TCls_Object);
    log_thing_differences(client_snapshot, "Effects", client->effects, host->effects, TCls_Effect);
    log_thing_differences(client_snapshot, "DeadCreatures", client->dead_creatures, host->dead_creatures, TCls_DeadCreature);
    log_thing_differences(client_snapshot, "EffectGens", client->effect_gens, host->effect_gens, TCls_EffectGen);
    log_thing_differences(client_snapshot, "Doors", client->doors, host->doors, TCls_Door);
    ERRORLOG("=== END ===");
}

CoroutineLoopState perform_checksum_verification(CoroutineLoop *con) {
    short result = true;
    unsigned long checksum_mem = 0;
    for (int i = 1; i < THINGS_COUNT; i++) {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing)) {
            checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
        }
    }
    clear_packets();
    struct Packet* pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_LevelExactCheck, 0, 0, 0, 0);
    pckt->checksum = checksum_mem + game.action_random_seed;
    if (LbNetwork_Exchange(NETMSG_SMALLDATA, pckt, game.packets, sizeof(struct Packet))) {
        ERRORLOG("Network exchange failed on level checksum verification");
        result = false;
    }
    if (get_packet(0)->action != get_packet(1)->action) {
        MULTIPLAYER_LOG("perform_checksum_verification: actions don't match, waiting");
        return CLS_REPEAT;
    }
    if ( checksums_different() ) {
        ERRORLOG("Level checksums different for network players");
        result = false;
    }
    if (!result) {
        coroutine_clear(con, true);

        create_frontend_error_box(5000, get_string(GUIStr_NetUnsyncedMap));
        return CLS_ABORT;
    }
    NETLOG("Checksums are verified");

    return CLS_CONTINUE;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
