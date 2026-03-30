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
#include "thing_creature.h"
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

static void log_thing_snapshot(const char *side, const struct LogThingDesyncInfo *thing_snapshot)
{
    ERRORLOG("    [%s] Thing[%d] class_id=%d model=%d owner=%d mappos=(%ld,%ld,%ld) health=%ld creation_turn=%lu random_seed=%08x current_frame=%u max_frames=%u max_health=%ld exp_level=%u inst_turn=%u instance_id=%u active_state=%u continue_state=%u tasks_check_turn=%lu moveto_pos=(%ld,%ld,%ld) dragtng_idx=%d arming_thing_id=%d pickup_object_id=%d pickup_creature_id=%d move_flags=%u players_prev_creature_idx=%d players_next_creature_idx=%d last_work_room_id=%d work_room_id=%d target_room_id=%d turns_at_job=%ld blocking_door_id=%d active_instance_id=%u active_state_bkp=%u continue_state_bkp=%u damage_wall_coords=%u job_assigned=%u healing_sleep_check_turn=%lu job_assigned_check_turn=%lu stopped_for_hand_turns=%u idle_start_gameturn=%lu current_slab_kind=%u current_slab_owner=%d task_slab_kind=%u task_slab_owner=%d working_slab_kind=%u working_slab_owner=%d digger_stack_update_turn=%lu digger_working_stl=%u digger_task_stl=%u digger_task_idx=%u digger_consecutive_reinforcements=%u digger_last_did_job=%u digger_task_stack_pos=%u digger_task_repeats=%u",
        side, thing_snapshot->index, thing_snapshot->class_id, thing_snapshot->model, thing_snapshot->owner,
        (long)thing_snapshot->mappos.x.val, (long)thing_snapshot->mappos.y.val, (long)thing_snapshot->mappos.z.val,
        (long)thing_snapshot->health, (unsigned long)thing_snapshot->creation_turn, thing_snapshot->random_seed,
        (unsigned)thing_snapshot->current_frame, (unsigned)thing_snapshot->max_frames, (long)thing_snapshot->max_health, (unsigned)thing_snapshot->exp_level,
        (unsigned)thing_snapshot->inst_turn, (unsigned)thing_snapshot->instance_id,
        (unsigned)thing_snapshot->active_state, (unsigned)thing_snapshot->continue_state, (unsigned long)thing_snapshot->tasks_check_turn,
        (long)thing_snapshot->moveto_pos.x.val, (long)thing_snapshot->moveto_pos.y.val, (long)thing_snapshot->moveto_pos.z.val,
        thing_snapshot->dragtng_idx, thing_snapshot->arming_thing_id, thing_snapshot->pickup_object_id, thing_snapshot->pickup_creature_id,
        (unsigned)thing_snapshot->move_flags, thing_snapshot->players_prev_creature_idx, thing_snapshot->players_next_creature_idx,
        thing_snapshot->last_work_room_id, thing_snapshot->work_room_id,
        thing_snapshot->target_room_id, (long)thing_snapshot->turns_at_job, thing_snapshot->blocking_door_id,
        (unsigned)thing_snapshot->active_instance_id, (unsigned)thing_snapshot->active_state_bkp,
        (unsigned)thing_snapshot->continue_state_bkp, (unsigned)thing_snapshot->damage_wall_coords, (unsigned)thing_snapshot->job_assigned,
        (unsigned long)thing_snapshot->healing_sleep_check_turn, (unsigned long)thing_snapshot->job_assigned_check_turn, (unsigned)thing_snapshot->stopped_for_hand_turns,
        (unsigned long)thing_snapshot->idle_start_gameturn, (unsigned)thing_snapshot->current_slab_kind, thing_snapshot->current_slab_owner,
        (unsigned)thing_snapshot->task_slab_kind, thing_snapshot->task_slab_owner, (unsigned)thing_snapshot->working_slab_kind,
        thing_snapshot->working_slab_owner, (unsigned long)thing_snapshot->digger_stack_update_turn,
        (unsigned)thing_snapshot->digger_working_stl, (unsigned)thing_snapshot->digger_task_stl, (unsigned)thing_snapshot->digger_task_idx,
        (unsigned)thing_snapshot->digger_consecutive_reinforcements, (unsigned)thing_snapshot->digger_last_did_job,
        (unsigned)thing_snapshot->digger_task_stack_pos, (unsigned)thing_snapshot->digger_task_repeats);
}

static void fill_slab_snapshot(MapSlabCoord slab_x, MapSlabCoord slab_y, SlabKind *kind, PlayerNumber *owner)
{
    *kind = 0;
    *owner = 0;
    if ((slab_x < 0) || (slab_x >= game.map_tiles_x) || (slab_y < 0) || (slab_y >= game.map_tiles_y)) {
        return;
    }
    struct SlabMap *slab = get_slabmap_block(slab_x, slab_y);
    *kind = slab->kind;
    *owner = slab->owner;
}

static void fill_slab_snapshot_from_subtile(SubtlCodedCoords subtile, SlabKind *kind, PlayerNumber *owner)
{
    if (subtile == 0) {
        *kind = 0;
        *owner = 0;
        return;
    }
    fill_slab_snapshot(subtile_slab(stl_num_decode_x(subtile)), subtile_slab(stl_num_decode_y(subtile)), kind, owner);
}

static TbBigChecksum get_map_task_checksum(const struct MapTask *task, long index)
{
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, index);
    CHECKSUM_ADD(checksum, task->kind);
    CHECKSUM_ADD(checksum, task->coords);
    return checksum;
}

static TbBigChecksum get_digger_stack_checksum(const struct DiggerStack *digger_task, long index)
{
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, index);
    CHECKSUM_ADD(checksum, digger_task->stl_num);
    CHECKSUM_ADD(checksum, digger_task->task_type);
    return checksum;
}

static TbBigChecksum compute_dungeon_task_list_checksum(const struct Dungeon *dungeon)
{
    TbBigChecksum checksum = 0;
    long task_limit = dungeon->highest_task_number;
    if (task_limit > MAPTASKS_COUNT) {
        task_limit = MAPTASKS_COUNT;
    }
    for (long i = 0; i < task_limit; i++) {
        CHECKSUM_ADD(checksum, get_map_task_checksum(&dungeon->task_list[i], i));
    }
    return checksum;
}

static TbBigChecksum compute_dungeon_digger_stack_checksum(const struct Dungeon *dungeon)
{
    TbBigChecksum checksum = 0;
    uint32_t digger_task_limit = dungeon->digger_stack_length;
    if (digger_task_limit > DIGGER_TASK_MAX_COUNT) {
        digger_task_limit = DIGGER_TASK_MAX_COUNT;
    }
    for (uint32_t i = 0; i < digger_task_limit; i++) {
        CHECKSUM_ADD(checksum, get_digger_stack_checksum(&dungeon->digger_stack[i], i));
    }
    return checksum;
}

static TbBigChecksum compute_imp_scheduler_checksum(const struct Dungeon *dungeon)
{
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, dungeon->owner);
    CHECKSUM_ADD(checksum, dungeon->digger_list_start);
    CHECKSUM_ADD(checksum, dungeon->task_count);
    CHECKSUM_ADD(checksum, dungeon->highest_task_number);
    CHECKSUM_ADD(checksum, dungeon->digger_stack_update_turn);
    CHECKSUM_ADD(checksum, dungeon->digger_stack_length);
    CHECKSUM_ADD(checksum, compute_dungeon_task_list_checksum(dungeon));
    CHECKSUM_ADD(checksum, compute_dungeon_digger_stack_checksum(dungeon));
    return checksum;
}

static void log_imp_scheduler_snapshot(const char *side, const struct LogImpSchedulerDesyncInfo *scheduler_snapshot)
{
    ERRORLOG("    [%s] ImpScheduler[%d] digger_list_start=%d task_count=%d highest_task_number=%d digger_stack_update_turn=%lu digger_stack_length=%lu task_list_checksum=%08lx digger_stack_checksum=%08lx checksum=%08lx",
        side, scheduler_snapshot->id, scheduler_snapshot->digger_list_start, scheduler_snapshot->task_count,
        scheduler_snapshot->highest_task_number, (unsigned long)scheduler_snapshot->digger_stack_update_turn,
        (unsigned long)scheduler_snapshot->digger_stack_length, (unsigned long)scheduler_snapshot->task_list_checksum,
        (unsigned long)scheduler_snapshot->digger_stack_checksum, (unsigned long)scheduler_snapshot->checksum);
}

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
        CHECKSUM_ADD(checksum, creature_control->max_health);
        CHECKSUM_ADD(checksum, creature_control->exp_level);
        CHECKSUM_ADD(checksum, creature_control->inst_turn);
        CHECKSUM_ADD(checksum, creature_control->instance_id);
        CHECKSUM_ADD(checksum, thing->active_state);
        CHECKSUM_ADD(checksum, thing->continue_state);
        CHECKSUM_ADD(checksum, creature_control->tasks_check_turn);
        CHECKSUM_ADD(checksum, creature_control->players_prev_creature_idx);
        CHECKSUM_ADD(checksum, creature_control->players_next_creature_idx);
        CHECKSUM_ADD(checksum, creature_control->last_work_room_id);
        CHECKSUM_ADD(checksum, creature_control->work_room_id);
        CHECKSUM_ADD(checksum, creature_control->target_room_id);
        CHECKSUM_ADD(checksum, creature_control->turns_at_job);
        CHECKSUM_ADD(checksum, creature_control->blocking_door_id);
        CHECKSUM_ADD(checksum, creature_control->active_instance_id);
        CHECKSUM_ADD(checksum, creature_control->active_state_bkp);
        CHECKSUM_ADD(checksum, creature_control->continue_state_bkp);
        CHECKSUM_ADD(checksum, creature_control->damage_wall_coords);
        CHECKSUM_ADD(checksum, creature_control->job_assigned);
        CHECKSUM_ADD(checksum, creature_control->healing_sleep_check_turn);
        CHECKSUM_ADD(checksum, creature_control->job_assigned_check_turn);
        CHECKSUM_ADD(checksum, creature_control->stopped_for_hand_turns);
        CHECKSUM_ADD(checksum, creature_control->idle.start_gameturn);
        if (thing_is_creature_digger(thing)) {
            SlabKind current_slab_kind;
            PlayerNumber current_slab_owner;
            SlabKind task_slab_kind;
            PlayerNumber task_slab_owner;
            SlabKind working_slab_kind;
            PlayerNumber working_slab_owner;
            fill_slab_snapshot(subtile_slab(thing->mappos.x.stl.num), subtile_slab(thing->mappos.y.stl.num), &current_slab_kind, &current_slab_owner);
            fill_slab_snapshot_from_subtile(creature_control->digger.task_stl, &task_slab_kind, &task_slab_owner);
            fill_slab_snapshot_from_subtile(creature_control->digger.working_stl, &working_slab_kind, &working_slab_owner);
            CHECKSUM_ADD(checksum, current_slab_kind);
            CHECKSUM_ADD(checksum, current_slab_owner);
            CHECKSUM_ADD(checksum, task_slab_kind);
            CHECKSUM_ADD(checksum, task_slab_owner);
            CHECKSUM_ADD(checksum, working_slab_kind);
            CHECKSUM_ADD(checksum, working_slab_owner);
            CHECKSUM_ADD(checksum, creature_control->moveto_pos.x.val);
            CHECKSUM_ADD(checksum, creature_control->moveto_pos.y.val);
            CHECKSUM_ADD(checksum, creature_control->moveto_pos.z.val);
            CHECKSUM_ADD(checksum, creature_control->dragtng_idx);
            CHECKSUM_ADD(checksum, creature_control->arming_thing_id);
            CHECKSUM_ADD(checksum, creature_control->pickup_object_id);
            CHECKSUM_ADD(checksum, creature_control->pickup_creature_id);
            CHECKSUM_ADD(checksum, creature_control->move_flags);
            CHECKSUM_ADD(checksum, creature_control->digger.stack_update_turn);
            CHECKSUM_ADD(checksum, creature_control->digger.working_stl);
            CHECKSUM_ADD(checksum, creature_control->digger.task_stl);
            CHECKSUM_ADD(checksum, creature_control->digger.task_idx);
            CHECKSUM_ADD(checksum, creature_control->digger.consecutive_reinforcements);
            CHECKSUM_ADD(checksum, creature_control->digger.last_did_job);
            CHECKSUM_ADD(checksum, creature_control->digger.task_stack_pos);
            CHECKSUM_ADD(checksum, creature_control->digger.task_repeats);
        }
    }
    return checksum;
}

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player) {
    struct Camera* camera = get_player_active_camera(player);
    if ((player->allocflags & PlaF_CompCtrl) != 0 || camera == NULL) {
        return 0;
    }
    struct Coord3d* mappos = &(camera->mappos);
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
    checksums->imp_schedulers = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        struct Dungeon* dungeon = get_dungeon(i);
        if (player_exists(player) && !dungeon_invalid(dungeon)) {
            checksums->imp_schedulers += compute_imp_scheduler_checksum(dungeon);
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
            ERRORLOG("Checksums %08x(Host) != %08x(Client) turn: %d vs %d", host_checksum, packet->checksum, host_packet->turn, packet->turn);
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
    snapshot_info->imp_scheduler_count = 0;
    if ((game.system_flags & GSF_NetworkActive) != 0) {
        for (int i = 1; i < SYNCED_THINGS_COUNT; i++) {
            struct Thing* thing = thing_get(i);
            if (!thing_exists(thing) || is_non_synchronized_thing_class(thing->class_id)) {
                continue;
            }
            struct LogThingDesyncInfo* thing_snapshot = &snapshot_info->things[snapshot_info->thing_count++];
            memset(thing_snapshot, 0, sizeof(*thing_snapshot));
            thing_snapshot->index = thing->index;
            thing_snapshot->class_id = thing->class_id;
            thing_snapshot->model = thing->model;
            thing_snapshot->owner = thing->owner;
            thing_snapshot->mappos = thing->mappos;
            thing_snapshot->health = thing->health;
            thing_snapshot->creation_turn = thing->creation_turn;
            thing_snapshot->random_seed = thing->random_seed;
            thing_snapshot->current_frame = thing->current_frame;
            thing_snapshot->max_frames = thing->max_frames;
            if (thing->class_id == TCls_Creature) {
                struct CreatureControl* creature_control = creature_control_get_from_thing(thing);
                thing_snapshot->max_health = creature_control->max_health;
                thing_snapshot->exp_level = creature_control->exp_level;
                thing_snapshot->inst_turn = creature_control->inst_turn;
                thing_snapshot->instance_id = creature_control->instance_id;
                thing_snapshot->active_state = thing->active_state;
                thing_snapshot->continue_state = thing->continue_state;
                thing_snapshot->tasks_check_turn = creature_control->tasks_check_turn;
                thing_snapshot->moveto_pos = creature_control->moveto_pos;
                thing_snapshot->dragtng_idx = creature_control->dragtng_idx;
                thing_snapshot->arming_thing_id = creature_control->arming_thing_id;
                thing_snapshot->pickup_object_id = creature_control->pickup_object_id;
                thing_snapshot->pickup_creature_id = creature_control->pickup_creature_id;
                thing_snapshot->move_flags = creature_control->move_flags;
                thing_snapshot->players_prev_creature_idx = creature_control->players_prev_creature_idx;
                thing_snapshot->players_next_creature_idx = creature_control->players_next_creature_idx;
                thing_snapshot->last_work_room_id = creature_control->last_work_room_id;
                thing_snapshot->work_room_id = creature_control->work_room_id;
                thing_snapshot->target_room_id = creature_control->target_room_id;
                thing_snapshot->turns_at_job = creature_control->turns_at_job;
                thing_snapshot->blocking_door_id = creature_control->blocking_door_id;
                thing_snapshot->active_instance_id = creature_control->active_instance_id;
                thing_snapshot->active_state_bkp = creature_control->active_state_bkp;
                thing_snapshot->continue_state_bkp = creature_control->continue_state_bkp;
                thing_snapshot->damage_wall_coords = creature_control->damage_wall_coords;
                thing_snapshot->job_assigned = creature_control->job_assigned;
                thing_snapshot->healing_sleep_check_turn = creature_control->healing_sleep_check_turn;
                thing_snapshot->job_assigned_check_turn = creature_control->job_assigned_check_turn;
                thing_snapshot->stopped_for_hand_turns = creature_control->stopped_for_hand_turns;
                thing_snapshot->idle_start_gameturn = creature_control->idle.start_gameturn;
                if (thing_is_creature_digger(thing)) {
                    fill_slab_snapshot(subtile_slab(thing->mappos.x.stl.num), subtile_slab(thing->mappos.y.stl.num), &thing_snapshot->current_slab_kind, &thing_snapshot->current_slab_owner);
                    fill_slab_snapshot_from_subtile(creature_control->digger.task_stl, &thing_snapshot->task_slab_kind, &thing_snapshot->task_slab_owner);
                    fill_slab_snapshot_from_subtile(creature_control->digger.working_stl, &thing_snapshot->working_slab_kind, &thing_snapshot->working_slab_owner);
                    thing_snapshot->digger_stack_update_turn = creature_control->digger.stack_update_turn;
                    thing_snapshot->digger_working_stl = creature_control->digger.working_stl;
                    thing_snapshot->digger_task_stl = creature_control->digger.task_stl;
                    thing_snapshot->digger_task_idx = creature_control->digger.task_idx;
                    thing_snapshot->digger_consecutive_reinforcements = creature_control->digger.consecutive_reinforcements;
                    thing_snapshot->digger_last_did_job = creature_control->digger.last_did_job;
                    thing_snapshot->digger_task_stack_pos = creature_control->digger.task_stack_pos;
                    thing_snapshot->digger_task_repeats = creature_control->digger.task_repeats;
                }
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
            player_snapshot->mappos = camera->mappos;
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
        for (int i = 0; i < PLAYERS_COUNT; i++) {
            struct PlayerInfo* player = get_player(i);
            struct Dungeon* dungeon = get_dungeon(i);
            if (!player_exists(player) || dungeon_invalid(dungeon)) {
                continue;
            }
            struct LogImpSchedulerDesyncInfo* scheduler_snapshot = &snapshot_info->imp_schedulers[snapshot_info->imp_scheduler_count++];
            memset(scheduler_snapshot, 0, sizeof(*scheduler_snapshot));
            scheduler_snapshot->id = i;
            scheduler_snapshot->digger_list_start = dungeon->digger_list_start;
            scheduler_snapshot->task_count = dungeon->task_count;
            scheduler_snapshot->highest_task_number = dungeon->highest_task_number;
            scheduler_snapshot->digger_stack_update_turn = dungeon->digger_stack_update_turn;
            scheduler_snapshot->digger_stack_length = dungeon->digger_stack_length;
            scheduler_snapshot->task_list_checksum = compute_dungeon_task_list_checksum(dungeon);
            scheduler_snapshot->digger_stack_checksum = compute_dungeon_digger_stack_checksum(dungeon);
            scheduler_snapshot->checksum = compute_imp_scheduler_checksum(dungeon);
            for (int task_index = 0; task_index < MAPTASKS_COUNT; task_index++) {
                struct LogMapTaskDesyncInfo* task_snapshot = &scheduler_snapshot->tasks[task_index];
                task_snapshot->kind = dungeon->task_list[task_index].kind;
                task_snapshot->coords = dungeon->task_list[task_index].coords;
                task_snapshot->checksum = get_map_task_checksum(&dungeon->task_list[task_index], task_index);
            }
            for (int digger_task_index = 0; digger_task_index < DIGGER_TASK_MAX_COUNT; digger_task_index++) {
                struct LogDiggerStackDesyncInfo* digger_task_snapshot = &scheduler_snapshot->digger_tasks[digger_task_index];
                digger_task_snapshot->stl_num = dungeon->digger_stack[digger_task_index].stl_num;
                digger_task_snapshot->task_type = dungeon->digger_stack[digger_task_index].task_type;
                digger_task_snapshot->checksum = get_digger_stack_checksum(&dungeon->digger_stack[digger_task_index], digger_task_index);
            }
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
    packet->checksum += checksums->imp_schedulers;
    packet->checksum += checksums->action_seed;
    packet->checksum += checksums->player_seed;
    packet->checksum += checksums->ai_seed;

    MULTIPLAYER_LOG("update_turn_checksums: turn=%lu checksum=%08lx things=%08lx rooms=%08lx players=%08lx imp_schedulers=%08lx", (unsigned long)game.play_gameturn, (unsigned long)packet->checksum, (unsigned long)things_sum, (unsigned long)checksums->rooms, (unsigned long)checksums->players, (unsigned long)checksums->imp_schedulers);
}

void pack_desync_history_for_resync(void) {
    struct ChecksumSnapshot* snapshot = find_snapshot(desync_turn);
    if (snapshot == NULL) {
        compute_checksums(&game.host_checksums);
        game.log_snapshot.thing_count = 0;
        game.log_snapshot.player_count = 0;
        game.log_snapshot.room_count = 0;
        game.log_snapshot.imp_scheduler_count = 0;
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
                log_thing_snapshot("Host", host_thing);
            } else {
                ERRORLOG("    [Host] Thing[%d] missing", client_thing->index);
            }
            log_thing_snapshot("Client", client_thing);
            shown++;
        }
    }
}

static void log_imp_scheduler_differences(struct LogDetailedSnapshot* client, struct LogDetailedSnapshot* host, TbBigChecksum client_sum, TbBigChecksum host_sum)
{
    ERRORLOG("  ImpSchedulers %s - Host: %08lx, Client: %08lx", (client_sum == host_sum) ? "match" : "MISMATCH", (unsigned long)host_sum, (unsigned long)client_sum);
    if (client_sum == host_sum) {
        return;
    }
    for (int i = 0; i < client->imp_scheduler_count; i++) {
        struct LogImpSchedulerDesyncInfo* client_scheduler = &client->imp_schedulers[i];
        struct LogImpSchedulerDesyncInfo* host_scheduler = NULL;
        for (int j = 0; j < host->imp_scheduler_count; j++) {
            if (host->imp_schedulers[j].id == client_scheduler->id) {
                host_scheduler = &host->imp_schedulers[j];
                break;
            }
        }
        if (host_scheduler == NULL) {
            ERRORLOG("    ImpScheduler[%d] missing from host", client_scheduler->id);
            continue;
        }
        if (client_scheduler->checksum == host_scheduler->checksum) {
            continue;
        }
        log_imp_scheduler_snapshot("Host", host_scheduler);
        log_imp_scheduler_snapshot("Client", client_scheduler);
        if (client_scheduler->task_list_checksum != host_scheduler->task_list_checksum) {
            int shown = 0;
            int task_limit = client_scheduler->highest_task_number;
            if (host_scheduler->highest_task_number > task_limit) {
                task_limit = host_scheduler->highest_task_number;
            }
            if (task_limit > MAPTASKS_COUNT) {
                task_limit = MAPTASKS_COUNT;
            }
            for (int task_index = 0; task_index < task_limit && shown < 10; task_index++) {
                struct LogMapTaskDesyncInfo* client_task = &client_scheduler->tasks[task_index];
                struct LogMapTaskDesyncInfo* host_task = &host_scheduler->tasks[task_index];
                if (client_task->checksum == host_task->checksum) {
                    continue;
                }
                ERRORLOG("      Task[%d] kind: Host=%u Client=%u, coords: Host=%u Client=%u",
                    task_index, (unsigned)host_task->kind, (unsigned)client_task->kind,
                    (unsigned)host_task->coords, (unsigned)client_task->coords);
                shown++;
            }
        }
        if (client_scheduler->digger_stack_checksum != host_scheduler->digger_stack_checksum) {
            int shown = 0;
            uint32_t digger_task_limit = client_scheduler->digger_stack_length;
            if (host_scheduler->digger_stack_length > digger_task_limit) {
                digger_task_limit = host_scheduler->digger_stack_length;
            }
            if (digger_task_limit > DIGGER_TASK_MAX_COUNT) {
                digger_task_limit = DIGGER_TASK_MAX_COUNT;
            }
            for (uint32_t digger_task_index = 0; digger_task_index < digger_task_limit && shown < 10; digger_task_index++) {
                struct LogDiggerStackDesyncInfo* client_digger_task = &client_scheduler->digger_tasks[digger_task_index];
                struct LogDiggerStackDesyncInfo* host_digger_task = &host_scheduler->digger_tasks[digger_task_index];
                if (client_digger_task->checksum == host_digger_task->checksum) {
                    continue;
                }
                ERRORLOG("      DiggerTask[%lu] task_type: Host=%u Client=%u, stl_num: Host=%u Client=%u",
                    (unsigned long)digger_task_index, (unsigned)host_digger_task->task_type,
                    (unsigned)client_digger_task->task_type, (unsigned)host_digger_task->stl_num,
                    (unsigned)client_digger_task->stl_num);
                shown++;
            }
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
    log_imp_scheduler_differences(client_snapshot, host_snapshot, client->imp_schedulers, host->imp_schedulers);
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
