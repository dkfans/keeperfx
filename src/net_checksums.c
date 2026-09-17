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
#include "bflib_dernc.h"
#include "config.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "net_game.h"
#include "packets.h"
#include "player_data.h"
#include "thing_data.h"
#include "room_list.h"
#include "slab_data.h"
#include "creature_control.h"
#include "thing_creature.h"
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
    CHECKSUM_ADD(checksum, thing->anim_sprite);
    CHECKSUM_ADD(checksum, thing->anim_speed);
    CHECKSUM_ADD(checksum, thing->anim_time);
    CHECKSUM_ADD(checksum, thing->current_frame);
    CHECKSUM_ADD(checksum, thing->max_frames);
    CHECKSUM_ADD(checksum, thing->active_state);
    CHECKSUM_ADD(checksum, thing->continue_state);
    CHECKSUM_ADD(checksum, thing->movement_flags);
    CHECKSUM_ADD(checksum, thing->move_angle_xy);
    CHECKSUM_ADD(checksum, thing->move_angle_z);
    CHECKSUM_ADD(checksum, thing->holding_player);
    CHECKSUM_ADD(checksum, thing->parent_idx);
    CHECKSUM_ADD(checksum, thing->fall_acceleration);
    CHECKSUM_ADD(checksum, thing->veloc_base.x.val);
    CHECKSUM_ADD(checksum, thing->veloc_base.y.val);
    CHECKSUM_ADD(checksum, thing->veloc_base.z.val);
    CHECKSUM_ADD(checksum, thing->veloc_push_once.x.val);
    CHECKSUM_ADD(checksum, thing->veloc_push_once.y.val);
    CHECKSUM_ADD(checksum, thing->veloc_push_once.z.val);
    CHECKSUM_ADD(checksum, thing->veloc_push_add.x.val);
    CHECKSUM_ADD(checksum, thing->veloc_push_add.y.val);
    CHECKSUM_ADD(checksum, thing->veloc_push_add.z.val);
    if (thing_is_creature_special_digger(thing)) {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        CHECKSUM_ADD(checksum, cctrl->moveto_pos.x.val);
        CHECKSUM_ADD(checksum, cctrl->moveto_pos.y.val);
        CHECKSUM_ADD(checksum, cctrl->moveto_pos.z.val);
        CHECKSUM_ADD(checksum, cctrl->dragtng_idx);
        CHECKSUM_ADD(checksum, cctrl->arming_thing_id);
        CHECKSUM_ADD(checksum, cctrl->pickup_object_id);
        CHECKSUM_ADD(checksum, cctrl->pickup_creature_id);
        CHECKSUM_ADD(checksum, cctrl->move_flags);
        CHECKSUM_ADD(checksum, cctrl->digger.stack_update_turn);
        CHECKSUM_ADD(checksum, cctrl->digger.working_stl);
        CHECKSUM_ADD(checksum, cctrl->digger.task_stl);
        CHECKSUM_ADD(checksum, cctrl->digger.consecutive_reinforcements);
        CHECKSUM_ADD(checksum, cctrl->digger.last_did_job);
        CHECKSUM_ADD(checksum, cctrl->digger.task_stack_pos);
        CHECKSUM_ADD(checksum, cctrl->digger.task_repeats);
    }
    return checksum;
}

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player) {
    struct Camera* camera = get_player_active_camera(player);
    if ((player->allocflags & PlaF_CompCtrl) != 0 || camera == NULL) {
        return 0;
    }
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, player->instance_remain_turns);
    CHECKSUM_ADD(checksum, player->instance_num);
    if (player->victory_state == VicS_Undecided) {
        CHECKSUM_ADD(checksum, camera->mappos.x.val);
        CHECKSUM_ADD(checksum, camera->mappos.y.val);
        CHECKSUM_ADD(checksum, camera->mappos.z.val);
    }
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
    checksums->dig_tasks = 0;
    for (int i = 0; i < DUNGEONS_COUNT; i++) {
        struct Dungeon* dungeon = get_dungeon(i);
        for (int t = 0; t < dungeon->task_count; t++) {
            struct MapTask* task = get_dungeon_task_list_entry(dungeon, t);
            CHECKSUM_ADD(checksums->dig_tasks, task->kind);
            CHECKSUM_ADD(checksums->dig_tasks, task->coords);
        }
    }
    checksums->action_seed = game.action_random_seed;
    checksums->ai_seed = game.ai_random_seed;
    checksums->player_seed = game.player_random_seed;
    checksums->game_turn = get_gameturn();
}

static struct ChecksumSnapshot* find_snapshot(GameTurn turn) {
    for (int i = 0; i < SNAPSHOT_BUFFER_SIZE; i++) {
        if (snapshot_buffer[i].valid && snapshot_buffer[i].turn == turn) {
            return &snapshot_buffer[i];
        }
    }
    return NULL;
}

short checksums_different(void)
{
    const NetUserId host_user_id = SERVER_ID;
    struct Packet* host_packet = get_packet(host_user_id);
    TbBigChecksum host_checksum = host_packet->checksum;
    TbBool mismatch = false;

    for (NetUserId i = 0; i < MAX_NET_USERS; i++) {
        if (i == host_user_id) {
            continue;
        }
        if (!user_present(i)) {
            continue;
        }
        struct PlayerInfo* player = get_player(get_net_user_player_number(i));
        if (!player_exists(player)) {
            continue;
        }
        struct Packet* packet = get_packet(i);
        // no need to validate checksm for users who are dropping
        // (and the host sometimes emits checksumless packets for such users)
        if ((packet->action == PckA_QuitToMainMenu) || (packet->action == PckA_ForceApplicationClose)) {
            continue;
        }
        if (is_packet_empty(packet)) {
            ERRORLOG("Missing checksum packet for user %d; host turn: %d", i, host_packet->turn);
            desync_turn = host_packet->turn;
            mismatch = true;
            continue;
        }
        if (packet->checksum != host_checksum) {
            ERRORLOG("Checksums %08x(Host) != %08x(Client) turn: %d vs %d", host_checksum, packet->checksum, host_packet->turn, packet->turn);
            desync_turn = host_packet->turn;
            mismatch = true;
        }
    }
    return mismatch;
}

TbBigChecksum calculate_file_checksum(const char *fname)
{
    int32_t file_size = (int32_t)LbFileLengthRnc(fname);
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, file_size);
    if (file_size <= 0) {
        return checksum;
    }
    unsigned char *file_buf = malloc(file_size);
    if (file_buf != NULL && LbFileLoadAt(fname, file_buf) == file_size) {
        CHECKSUM_ADD(checksum, rnc_crc(file_buf, file_size));
    }
    free(file_buf);
    return checksum;
}

void calculate_network_startup_map_checksums(TbBigChecksum checksums[NETWORK_STARTUP_MAP_FILE_COUNT])
{
    LevelNumber lvnum = get_loaded_level_number();
    short fgroup = get_level_fgroup(lvnum);
    for (int i = 0; i < NETWORK_STARTUP_MAP_FILE_COUNT; i++) {
        char* fname = prepare_file_fmtpath(fgroup, "map%05u.%s", lvnum, network_startup_compare_files[i]);
        checksums[i] = calculate_file_checksum(fname);
    }
}

void update_turn_checksums(void) {
    struct ChecksumSnapshot* snapshot = &snapshot_buffer[snapshot_head];
    struct LogDetailedSnapshot* snapshot_info = &snapshot->log_details;
    snapshot->turn = get_gameturn();
    snapshot->valid = true;
    compute_checksums(&snapshot->checksums);
    snapshot_info->thing_count = 0;
    snapshot_info->player_count = 0;
    snapshot_info->room_count = 0;
    memset(snapshot_info->dig_task_counts, 0, sizeof(snapshot_info->dig_task_counts));
    if (network_is_active()) {
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
            thing_snapshot->anim_sprite = thing->anim_sprite;
            thing_snapshot->anim_speed = thing->anim_speed;
            thing_snapshot->anim_time = thing->anim_time;
            thing_snapshot->current_frame = thing->current_frame;
            thing_snapshot->max_frames = thing->max_frames;
            thing_snapshot->active_state = thing->active_state;
            thing_snapshot->continue_state = thing->continue_state;
            thing_snapshot->movement_flags = thing->movement_flags;
            thing_snapshot->move_angle_xy = thing->move_angle_xy;
            thing_snapshot->move_angle_z = thing->move_angle_z;
            thing_snapshot->holding_player = thing->holding_player;
            thing_snapshot->parent_idx = thing->parent_idx;
            thing_snapshot->fall_acceleration = thing->fall_acceleration;
            thing_snapshot->veloc_base = thing->veloc_base;
            thing_snapshot->veloc_push_once = thing->veloc_push_once;
            thing_snapshot->veloc_push_add = thing->veloc_push_add;
            thing_snapshot->is_special_digger = thing_is_creature_special_digger(thing);
            if (thing_snapshot->is_special_digger) {
                struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
                thing_snapshot->digger_moveto_pos = cctrl->moveto_pos;
                thing_snapshot->digger_dragtng_idx = cctrl->dragtng_idx;
                thing_snapshot->digger_arming_thing_id = cctrl->arming_thing_id;
                thing_snapshot->digger_pickup_object_id = cctrl->pickup_object_id;
                thing_snapshot->digger_pickup_creature_id = cctrl->pickup_creature_id;
                thing_snapshot->digger_move_flags = cctrl->move_flags;
                thing_snapshot->digger_stack_update_turn = cctrl->digger.stack_update_turn;
                thing_snapshot->digger_working_stl = cctrl->digger.working_stl;
                thing_snapshot->digger_task_stl = cctrl->digger.task_stl;
                thing_snapshot->digger_task_idx = cctrl->digger.task_idx;
                thing_snapshot->digger_consecutive_reinforcements = cctrl->digger.consecutive_reinforcements;
                thing_snapshot->digger_last_did_job = cctrl->digger.last_did_job;
                thing_snapshot->digger_task_stack_pos = cctrl->digger.task_stack_pos;
                thing_snapshot->digger_task_repeats = cctrl->digger.task_repeats;
            }
            thing_snapshot->checksum = get_thing_checksum(thing);
        }
        for (int i = 0; i < PLAYERS_COUNT; i++) {
            struct PlayerInfo* player = get_player(i);
            struct Camera* camera = get_player_active_camera(player);
            if (!player_exists(player) || ((player->allocflags & PlaF_CompCtrl) != 0) || camera == NULL) {
                continue;
            }
            struct LogPlayerDesyncInfo* player_snapshot = &snapshot_info->players[snapshot_info->player_count++];
            player_snapshot->id = i;
            player_snapshot->instance_num = player->instance_num;
            player_snapshot->instance_remain_turns = player->instance_remain_turns;
            if (player->victory_state == VicS_Undecided) {
                player_snapshot->mappos = camera->mappos;
            } else {
                memset(&player_snapshot->mappos, 0, sizeof(player_snapshot->mappos));
            }
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
        for (int i = 0; i < DUNGEONS_COUNT; i++) {
            struct Dungeon* dungeon = get_dungeon(i);
            snapshot_info->dig_task_counts[i] = dungeon->task_count;
            memcpy(snapshot_info->dig_tasks[i], dungeon->task_list, dungeon->task_count * sizeof(struct MapTask));
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

    struct Packet* packet = get_local_packet();
    packet->checksum = 0;
    packet->checksum += things_sum;
    packet->checksum += checksums->rooms;
    packet->checksum += checksums->players;
    packet->checksum += checksums->dig_tasks;
    packet->checksum += checksums->action_seed;
    packet->checksum += checksums->player_seed;
    packet->checksum += checksums->ai_seed;

    MULTIPLAYER_LOG("update_turn_checksums: turn=%lu checksum=%08lx things=%08lx rooms=%08lx players=%08lx", (unsigned long)get_gameturn(), (unsigned long)packet->checksum, (unsigned long)things_sum, (unsigned long)checksums->rooms, (unsigned long)checksums->players);
}

void pack_desync_history_for_resync(void) {
    struct ChecksumSnapshot* snapshot = find_snapshot(desync_turn);
    if (snapshot == NULL) {
        compute_checksums(&game.host_checksums);
        game.log_snapshot.thing_count = 0;
        game.log_snapshot.player_count = 0;
        game.log_snapshot.room_count = 0;
        memset(game.log_snapshot.dig_task_counts, 0, sizeof(game.log_snapshot.dig_task_counts));
        return;
    }
    game.host_checksums = snapshot->checksums;
    game.log_snapshot = snapshot->log_details;
}

static TbBool log_checksum_mismatch(const char* name, TbBigChecksum client_sum, TbBigChecksum host_sum)
{
    if (client_sum == host_sum) {
        return false;
    }
    ERRORLOG("  %s MISMATCH - Host: %08lx, Client: %08lx", name, (unsigned long)host_sum, (unsigned long)client_sum);
    return true;
}

static void log_dig_task_differences(const struct LogDetailedSnapshot* client)
{
    const struct LogDetailedSnapshot* host = &game.log_snapshot;
    int shown = 0;
    for (int plyr_idx = 0; plyr_idx < DUNGEONS_COUNT && shown < 10; plyr_idx++) {
        unsigned short host_count = host->dig_task_counts[plyr_idx];
        unsigned short client_count = client->dig_task_counts[plyr_idx];
        if ((host_count == client_count) && (memcmp(host->dig_tasks[plyr_idx], client->dig_tasks[plyr_idx], host_count * sizeof(struct MapTask)) == 0)) {
            continue;
        }
        ERRORLOG("    Player[%d] dig tags: Host count=%u, Client count=%u", plyr_idx, (unsigned)host_count, (unsigned)client_count);
        shown++;
        int count = max(host_count, client_count);
        for (int i = 0; (i < count) && (shown < 10); i++) {
            struct MapTask host_task = host->dig_tasks[plyr_idx][i];
            struct MapTask client_task = client->dig_tasks[plyr_idx][i];
            if ((i < host_count) && (i < client_count) && (host_task.kind == client_task.kind) && (host_task.coords == client_task.coords)) {
                continue;
            }
            char host_text[32] = "missing";
            char client_text[32] = "missing";
            if (i < host_count) {
                snprintf(host_text, sizeof(host_text), "kind=%u stl=(%u,%u)", (unsigned)host_task.kind, (unsigned)stl_num_decode_x(host_task.coords), (unsigned)stl_num_decode_y(host_task.coords));
            }
            if (i < client_count) {
                snprintf(client_text, sizeof(client_text), "kind=%u stl=(%u,%u)", (unsigned)client_task.kind, (unsigned)stl_num_decode_x(client_task.coords), (unsigned)stl_num_decode_y(client_task.coords));
            }
            ERRORLOG("    Player[%d] dig tag[%d]: Host %s, Client %s", plyr_idx, i, host_text, client_text);
            shown++;
        }
    }
}

static void log_thing_differences(struct LogDetailedSnapshot* client, const char* name, TbBigChecksum client_sum, TbBigChecksum host_sum, ThingClass filter_class) {
    if (!log_checksum_mismatch(name, client_sum, host_sum)) {
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
                ERRORLOG("    [Host] Thing[%d] class_id=%d model=%d owner=%d mappos=(%ld,%ld,%ld) health=%ld creation_turn=%lu random_seed=%08x anim_sprite=%u anim_speed=%d anim_time=%ld current_frame=%u max_frames=%u active_state=%u continue_state=%u movement_flags=%04x move_angle_xy=%d move_angle_z=%d holding_player=%d parent_idx=%d fall_acceleration=%u veloc_base=(%ld,%ld,%ld) veloc_push_once=(%ld,%ld,%ld) veloc_push_add=(%ld,%ld,%ld)",
                    host_thing->index, host_thing->class_id, host_thing->model, host_thing->owner,
                    (long)host_thing->mappos.x.val, (long)host_thing->mappos.y.val, (long)host_thing->mappos.z.val,
                    (long)host_thing->health, (unsigned long)host_thing->creation_turn, host_thing->random_seed,
                    (unsigned)host_thing->anim_sprite, (int)host_thing->anim_speed, (long)host_thing->anim_time,
                    (unsigned)host_thing->current_frame, (unsigned)host_thing->max_frames,
                    (unsigned)host_thing->active_state, (unsigned)host_thing->continue_state,
                    (unsigned)host_thing->movement_flags,
                    (int)host_thing->move_angle_xy, (int)host_thing->move_angle_z,
                    (int)host_thing->holding_player, (int)host_thing->parent_idx, (unsigned)host_thing->fall_acceleration,
                    (long)host_thing->veloc_base.x.val, (long)host_thing->veloc_base.y.val, (long)host_thing->veloc_base.z.val,
                    (long)host_thing->veloc_push_once.x.val, (long)host_thing->veloc_push_once.y.val, (long)host_thing->veloc_push_once.z.val,
                    (long)host_thing->veloc_push_add.x.val, (long)host_thing->veloc_push_add.y.val, (long)host_thing->veloc_push_add.z.val);
            } else {
                ERRORLOG("    [Host] Thing[%d] missing", client_thing->index);
            }
            ERRORLOG("    [Client] Thing[%d] class_id=%d model=%d owner=%d mappos=(%ld,%ld,%ld) health=%ld creation_turn=%lu random_seed=%08x anim_sprite=%u anim_speed=%d anim_time=%ld current_frame=%u max_frames=%u active_state=%u continue_state=%u movement_flags=%04x move_angle_xy=%d move_angle_z=%d holding_player=%d parent_idx=%d fall_acceleration=%u veloc_base=(%ld,%ld,%ld) veloc_push_once=(%ld,%ld,%ld) veloc_push_add=(%ld,%ld,%ld)",
                client_thing->index, client_thing->class_id, client_thing->model, client_thing->owner,
                (long)client_thing->mappos.x.val, (long)client_thing->mappos.y.val, (long)client_thing->mappos.z.val,
                (long)client_thing->health, (unsigned long)client_thing->creation_turn, client_thing->random_seed,
                (unsigned)client_thing->anim_sprite, (int)client_thing->anim_speed, (long)client_thing->anim_time,
                (unsigned)client_thing->current_frame, (unsigned)client_thing->max_frames,
                (unsigned)client_thing->active_state, (unsigned)client_thing->continue_state,
                (unsigned)client_thing->movement_flags,
                (int)client_thing->move_angle_xy, (int)client_thing->move_angle_z,
                (int)client_thing->holding_player, (int)client_thing->parent_idx, (unsigned)client_thing->fall_acceleration,
                (long)client_thing->veloc_base.x.val, (long)client_thing->veloc_base.y.val, (long)client_thing->veloc_base.z.val,
                (long)client_thing->veloc_push_once.x.val, (long)client_thing->veloc_push_once.y.val, (long)client_thing->veloc_push_once.z.val,
                (long)client_thing->veloc_push_add.x.val, (long)client_thing->veloc_push_add.y.val, (long)client_thing->veloc_push_add.z.val);
            if (client_thing->is_special_digger) {
                if (host_thing != NULL) {
                    ERRORLOG("    [Host]   digger moveto=(%ld,%ld,%ld) dragtng=%d arming=%d pickup_obj=%d pickup_cr=%d move_flags=%u stack_update_turn=%ld working_stl=%u task_stl=%u task_idx=%u consecutive_reinforcements=%u last_did_job=%u task_stack_pos=%u task_repeats=%u",
                        (long)host_thing->digger_moveto_pos.x.val, (long)host_thing->digger_moveto_pos.y.val, (long)host_thing->digger_moveto_pos.z.val,
                        (int)host_thing->digger_dragtng_idx, (int)host_thing->digger_arming_thing_id,
                        (int)host_thing->digger_pickup_object_id, (int)host_thing->digger_pickup_creature_id,
                        (unsigned)host_thing->digger_move_flags, (long)host_thing->digger_stack_update_turn,
                        (unsigned)host_thing->digger_working_stl, (unsigned)host_thing->digger_task_stl,
                        (unsigned)host_thing->digger_task_idx, (unsigned)host_thing->digger_consecutive_reinforcements,
                        (unsigned)host_thing->digger_last_did_job,
                        (unsigned)host_thing->digger_task_stack_pos, (unsigned)host_thing->digger_task_repeats);
                }
                ERRORLOG("    [Client] digger moveto=(%ld,%ld,%ld) dragtng=%d arming=%d pickup_obj=%d pickup_cr=%d move_flags=%u stack_update_turn=%ld working_stl=%u task_stl=%u task_idx=%u consecutive_reinforcements=%u last_did_job=%u task_stack_pos=%u task_repeats=%u",
                    (long)client_thing->digger_moveto_pos.x.val, (long)client_thing->digger_moveto_pos.y.val, (long)client_thing->digger_moveto_pos.z.val,
                    (int)client_thing->digger_dragtng_idx, (int)client_thing->digger_arming_thing_id,
                    (int)client_thing->digger_pickup_object_id, (int)client_thing->digger_pickup_creature_id,
                    (unsigned)client_thing->digger_move_flags, (long)client_thing->digger_stack_update_turn,
                    (unsigned)client_thing->digger_working_stl, (unsigned)client_thing->digger_task_stl,
                    (unsigned)client_thing->digger_task_idx, (unsigned)client_thing->digger_consecutive_reinforcements,
                    (unsigned)client_thing->digger_last_did_job,
                    (unsigned)client_thing->digger_task_stack_pos, (unsigned)client_thing->digger_task_repeats);
            }
            shown++;
        }
    }
}

void compare_desync_history_from_host(void) {
    struct ChecksumSnapshot* snapshot = find_snapshot(desync_turn);
    if (snapshot == NULL) {
        ERRORLOG("=== DESYNC: No client snapshot for turn %u ===", desync_turn);
        return;
    }
    struct DesyncChecksums* host = &game.host_checksums;
    struct DesyncChecksums* client = &snapshot->checksums;
    struct LogDetailedSnapshot* host_snapshot = &game.log_snapshot;
    struct LogDetailedSnapshot* client_snapshot = &snapshot->log_details;

    ERRORLOG("=== DESYNC ANALYSIS: Host (turn %lu) vs Client (turn %lu) ===", (unsigned long)host->game_turn, (unsigned long)client->game_turn);
    if (log_checksum_mismatch("DigTags", client->dig_tasks, host->dig_tasks)) {
        log_dig_task_differences(client_snapshot);
    }
    log_checksum_mismatch("ACTION_SEED", client->action_seed, host->action_seed);
    log_checksum_mismatch("AI_SEED", client->ai_seed, host->ai_seed);
    log_checksum_mismatch("PLAYER_SEED", client->player_seed, host->player_seed);
    if (log_checksum_mismatch("Players", client->players, host->players)) {
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
    if (log_checksum_mismatch("Rooms", client->rooms, host->rooms)) {
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
                ERRORLOG("    Room[%d] slabs_count: Host=%d Client=%d, efficiency: Host=%d Client=%d, used_capacity: Host=%d Client=%d, central_stl: Host=(%d,%d) Client=(%d,%d)",
                    client_room->index, host_room->slabs_count, client_room->slabs_count, host_room->efficiency, client_room->efficiency,
                    (int)host_room->used_capacity, (int)client_room->used_capacity,
                    (int)host_room->central_stl_x, (int)host_room->central_stl_y,
                    (int)client_room->central_stl_x, (int)client_room->central_stl_y);
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
