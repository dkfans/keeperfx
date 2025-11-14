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
#include "bflib_datetm.h"

#include "config.h"
#include "config_effects.h"
#include "front_network.h"
#include "player_data.h"
#include "game_merge.h"
#include "net_game.h"
#include "bflib_network_resync.h"
#include "lens_api.h"
#include "game_legacy.h"
#include "input_lag.h"
#include "received_packets.h"
#include "redundant_packets.h"
#include "desync_analysis.h"
#include "keeperfx.hpp"
#include "frontend.h"
#include "thing_effects.h"
#include "thing_data.h"
#include "creature_control.h"
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

TbBool detailed_multiplayer_logging = false;

#define CONSECUTIVE_RESYNC_DECAY_SECONDS 30
#define CONSECUTIVE_RESYNC_THRESHOLD_FOR_LAG_INCREASE 3

/******************************************************************************/
static GameTurn last_resync_turn = 0;
static int consecutive_resync_count = 0;

void decrement_consecutive_resync_count(void)
{
    if (consecutive_resync_count == 0) {
        return;
    }
    if (last_resync_turn == 0) {
        return;
    }
    GameTurn decay_window = CONSECUTIVE_RESYNC_DECAY_SECONDS * game_num_fps;
    GameTurn turns_since_last = game.play_gameturn - last_resync_turn;
    if (turns_since_last >= decay_window) {
        consecutive_resync_count--;
        MULTIPLAYER_LOG("Consecutive resync count decayed to %d", consecutive_resync_count);
    }
}

static void resync_potentially_increases_input_lag(void)
{
    consecutive_resync_count += 1;
    last_resync_turn = game.play_gameturn;
    MULTIPLAYER_LOG("Consecutive resync count: %d", consecutive_resync_count);

    if (consecutive_resync_count >= CONSECUTIVE_RESYNC_THRESHOLD_FOR_LAG_INCREASE) {
        consecutive_resync_count = 0;
        game.input_lag_turns += 1;
        NETLOG("Input lag increased: %d -> %d", game.input_lag_turns - 1, game.input_lag_turns);
    }
}



TbBool send_resync_game(void)
{
  pack_desync_history_for_resync();

  resync_potentially_increases_input_lag();
  clear_flag(game.operation_flags, GOF_Paused);
  animate_resync_progress_bar(0, 6);
  NETLOG("Initiating re-synchronization of network game");
  TbBool result = LbNetwork_Resync(&game, sizeof(game));
  if (!result) {
    return false;
  }
  animate_resync_progress_bar(2, 6);
  LbNetwork_TimesyncBarrier();
  animate_resync_progress_bar(6, 6);
  NETLOG("Host: Resync complete");
  return true;
}

TbBool receive_resync_game(void)
{
    clear_flag(game.operation_flags, GOF_Paused);
    animate_resync_progress_bar(0, 6);
    NETLOG("Initiating re-synchronization of network game");
    TbBool result = LbNetwork_Resync(&game, sizeof(game));
    if (!result) {
        return false;
    }
    animate_resync_progress_bar(2, 6);
    LbNetwork_TimesyncBarrier();
    animate_resync_progress_bar(6, 6);
    NETLOG("Client: Resync complete");
    if (game.desync_diagnostics.has_desync_diagnostics) {
        compare_desync_history_from_host();
    }

    return true;
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
    if (my_player_number == get_host_player_id()) {
        send_resync_game();
    } else {
        receive_resync_game();
    }
    recall_localised_game_structure();
    reinit_level_after_load();

    game.skip_initial_input_turns = calculate_skip_input();
    clear_packet_tracking();
    clear_redundant_packets();
    clear_input_lag_queue();
    clear_desync_analysis();
    NETLOG("Input lag after resync: %d turns", game.input_lag_turns);

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
    pckt->checksum = checksum_mem + game.action_random_seed;
    if (LbNetwork_Exchange(NETMSG_SMALLDATA, pckt, game.packets, sizeof(struct Packet)))
    {
        ERRORLOG("Network exchange failed on level checksum verification");
        result = false;
    }
    if (get_packet(0)->action != get_packet(1)->action)
    {
        MULTIPLAYER_LOG("perform_checksum_verification: actions don't match, waiting");
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
            ERRORLOG("Infinite loop detected in thing list on class %s",thing_class_code_name(thing->class_id));
            break;
        }
    }
    return sum;
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
 * Centralized function to compute all game state checksums for multiplayer sync verification.
 * Used only in multiplayer games to detect desynchronization between networked players.
 */
void compute_multiplayer_checksum(void)
{
    struct Packet* pckt = get_packet(my_player_number);
    pckt->checksum = 0;

    pckt->checksum += compute_things_checksum();
    pckt->checksum += compute_rooms_checksum();
    pckt->checksum += compute_players_checksum();
    pckt->checksum += game.action_random_seed;
    pckt->checksum += game.player_random_seed;
    pckt->checksum += game.ai_random_seed;

    MULTIPLAYER_LOG("compute_multiplayer_checksum: turn=%lu checksum=%08lx",
            (unsigned long)game.play_gameturn, (unsigned long)pckt->checksum);
}

void set_local_packet_turn(void)
{
    struct Packet* pckt = get_packet(my_player_number);
    pckt->turn = game.play_gameturn;
    MULTIPLAYER_LOG("set_local_packet_turn: turn=%lu checksum=%08lx",
            (unsigned long)game.play_gameturn, (unsigned long)pckt->checksum);
}

/**
 * Checks if all active players packets have same checksums.
 * @return Returns false if all checksums are same; true if there's mismatch.
 */
short checksums_different(void)
{
    TbBigChecksum checksum = 0;
    TbBool is_set = false;
    TbBool mismatch = false;
    int plyr = -1;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
            struct Packet* pckt = get_packet_direct(player->packet_num);
            if (is_packet_empty(pckt)) {
                MULTIPLAYER_LOG("checksums_different: packet[%d] is EMPTY", i);
                mismatch = true;
                continue;
            }
            if (!is_set) {
                checksum = pckt->checksum;
                is_set = true;
                plyr = i;
            } else if (checksum != pckt->checksum) {
                ERRORLOG("Checksums %08lx(%d) != %08lx(%d) turn: %ld", checksum, plyr, pckt->checksum, i, game.play_gameturn);
                mismatch = true;
            }
        }
    }
    return mismatch;
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
    CHECKSUM_ADD(checksum, thing->random_seed);
    CHECKSUM_ADD(checksum, thing->mappos.x.val);
    CHECKSUM_ADD(checksum, thing->mappos.y.val);
    CHECKSUM_ADD(checksum, thing->mappos.z.val);
    CHECKSUM_ADD(checksum, thing->health);
    CHECKSUM_ADD(checksum, thing->current_frame);
    CHECKSUM_ADD(checksum, thing->max_frames);

    if (thing->class_id == TCls_Creature) {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        CHECKSUM_ADD(checksum, cctrl->inst_turn);
        CHECKSUM_ADD(checksum, cctrl->instance_id);
    }
    return checksum;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
