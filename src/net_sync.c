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
struct DetailedThingChecksums {
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
    TbBigChecksum creatures_sum;
    TbBigChecksum things_sum;
    TbBigChecksum rooms_sum;
    GameTurn gameturn;
    TbBigChecksum action_random_seed;
    TbBigChecksum ai_random_seed;
    TbBigChecksum player_random_seed;
    TbBigChecksum player_checksums[PLAYERS_COUNT];
    struct DetailedThingChecksums things_detailed;
    TbBool is_stored;
} pre_resync_checksums = {0};

// Host populates game state with diagnostic checksums that clients receive during resync
// This allows clients to compare their pre-resync state with host's pre-resync state
static void populate_desync_diagnostics(void)
{
    game.desync_diagnostics.desync_turn = pre_resync_checksums.gameturn;
    game.desync_diagnostics.host_creatures_sum = pre_resync_checksums.creatures_sum;
    game.desync_diagnostics.host_things_sum = pre_resync_checksums.things_sum;
    game.desync_diagnostics.host_rooms_sum = pre_resync_checksums.rooms_sum;

    game.desync_diagnostics.host_traps_sum = pre_resync_checksums.things_detailed.traps;
    game.desync_diagnostics.host_shots_sum = pre_resync_checksums.things_detailed.shots;
    game.desync_diagnostics.host_objects_sum = pre_resync_checksums.things_detailed.objects;
    game.desync_diagnostics.host_effects_sum = pre_resync_checksums.things_detailed.effects;
    game.desync_diagnostics.host_dead_creatures_sum = pre_resync_checksums.things_detailed.dead_creatures;
    game.desync_diagnostics.host_effect_gens_sum = pre_resync_checksums.things_detailed.effect_gens;
    game.desync_diagnostics.host_doors_sum = pre_resync_checksums.things_detailed.doors;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        game.desync_diagnostics.host_player_checksums[i] = pre_resync_checksums.player_checksums[i];
    }
    game.desync_diagnostics.host_action_random_seed = pre_resync_checksums.action_random_seed;
    game.desync_diagnostics.host_ai_random_seed = pre_resync_checksums.ai_random_seed;
    game.desync_diagnostics.host_player_random_seed = pre_resync_checksums.player_random_seed;
    game.desync_diagnostics.has_desync_diagnostics = true;
}
/******************************************************************************/
long get_resync_sender(void)
{
    for (int i = 0; i < NET_PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
            return i;
        }
    }
    return -1;
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

  if (pre_resync_checksums.is_stored) {
    populate_desync_diagnostics();
    ERRORLOG("Host including desync diagnostics in resync: turn %ld", pre_resync_checksums.gameturn);
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
    if (result && game.desync_diagnostics.has_desync_diagnostics && pre_resync_checksums.is_stored) {
        analyze_desync_diagnostics_from_host();
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

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player)
{
    TbBigChecksum sum = 0;
    if (((player->allocflags & PlaF_CompCtrl) == 0) && (player->acamera != NULL))
    {
        struct Coord3d* mappos = &(player->acamera->mappos);
        sum += (TbBigChecksum)player->instance_remain_rurns + (TbBigChecksum)player->instance_num;
        sum += (TbBigChecksum)mappos->x.val + (TbBigChecksum)mappos->z.val + (TbBigChecksum)mappos->y.val;
    }
    return sum;
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
    sum += game.action_random_seed;
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
 * Computes checksum for all creatures in the game.
 * @return The creatures checksum value for multiplayer sync verification.
 */
static TbBigChecksum compute_creatures_checksum(void)
{
    return compute_things_list_checksum(&game.thing_lists[TngList_Creatures]);
}

/**
 * Computes detailed checksums for all thing categories.
 * @param checksums Pointer to structure to store the individual checksums.
 */
static void compute_detailed_things_checksums(struct DetailedThingChecksums *checksums)
{
    checksums->traps = compute_things_list_checksum(&game.thing_lists[TngList_Traps]);
    checksums->shots = compute_things_list_checksum(&game.thing_lists[TngList_Shots]);
    checksums->objects = compute_things_list_checksum(&game.thing_lists[TngList_Objects]);
    checksums->effects = compute_things_list_checksum(&game.thing_lists[TngList_Effects]);
    checksums->dead_creatures = compute_things_list_checksum(&game.thing_lists[TngList_DeadCreatrs]);
    checksums->effect_gens = compute_things_list_checksum(&game.thing_lists[TngList_EffectGens]);
    checksums->doors = compute_things_list_checksum(&game.thing_lists[TngList_Doors]);
}

/**
 * Computes checksum for all non-creature things (traps, shots, objects, effects, etc).
 * @return The combined checksum value for multiplayer sync verification.
 */
static TbBigChecksum compute_things_checksum(void)
{
    TbBigChecksum sum = 0;
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
 * Computes checksum for all rooms based on core room properties.
 * @return The rooms checksum value for multiplayer sync verification.
 */
static TbBigChecksum compute_rooms_checksum(void)
{
    TbBigChecksum sum = 0;
    for (struct Room* room = start_rooms; room < end_rooms; room++)
    {
        if (!room_exists(room)) {
            continue;
        }
        sum += room->slabs_count + room->central_stl_x + room->central_stl_y + room->efficiency + room->used_capacity;
    }
    return sum;
}

/**
 * Centralized function to compute all game state checksums for multiplayer sync verification.
 * Used only in multiplayer games to detect desynchronization between networked players.
 */
void compute_multiplayer_checksum_sync(void)
{
    TbBigChecksum creatures_sum = compute_creatures_checksum();
    player_packet_checksum_add(my_player_number, creatures_sum, "creatures");

    TbBigChecksum things_sum = compute_things_checksum();
    player_packet_checksum_add(my_player_number, things_sum, "things");

    TbBigChecksum rooms_sum = compute_rooms_checksum();
    player_packet_checksum_add(my_player_number, rooms_sum, "rooms");

    TbBigChecksum players_sum = compute_players_checksum();
    player_packet_checksum_add(my_player_number, players_sum, "players");
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
    if (!thing_exists(thing))
        return 0;
    TbBigChecksum csum = (ulong)thing->class_id + ((ulong)thing->model << 4) + (ulong)thing->owner;
    if (thing->class_id == TCls_Creature)
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        csum += (ulong)cctrl->inst_turn + (ulong)cctrl->instance_id
            + (ulong)thing->max_frames + (ulong)thing->current_frame;
    }
    else if ((thing->class_id == TCls_EffectElem) || (thing->class_id == TCls_AmbientSnd))
    {
        // No syncing on Effect Elements or Sounds
        return 0;
    }
    else if (thing->class_id == TCls_Effect)
    {
        const struct EffectConfigStats* effcst = get_effect_model_stats(thing->model);
        if (effcst->area_affect_type != AAffT_None)
        {
            csum += (ulong)thing->mappos.z.val +
                (ulong)thing->mappos.x.val +
                (ulong)thing->mappos.y.val +
                (ulong)thing->health;
        } else {
            // No syncing on Effects that do not affect the area around them
            return 0;
        }
    } else {
        csum += (ulong)thing->mappos.z.val +
            (ulong)thing->mappos.x.val +
            (ulong)thing->mappos.y.val +
            (ulong)thing->health;
    }
    return csum * thing->index;
}

// Store current game state checksums before resync occurs
// Host uses these to send diagnostic data to clients, clients use for comparison analysis
void store_checksums_for_desync_analysis(void)
{
    pre_resync_checksums.creatures_sum = compute_creatures_checksum();
    compute_detailed_things_checksums(&pre_resync_checksums.things_detailed);
    pre_resync_checksums.things_sum = compute_things_checksum();
    pre_resync_checksums.rooms_sum = compute_rooms_checksum();

    // Store random seeds for individual comparison
    pre_resync_checksums.action_random_seed = game.action_random_seed;
    pre_resync_checksums.ai_random_seed = game.ai_random_seed;
    pre_resync_checksums.player_random_seed = game.player_random_seed;

    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player)) {
            pre_resync_checksums.player_checksums[i] = compute_player_checksum(player);
        } else {
            pre_resync_checksums.player_checksums[i] = 0;
        }
    }

    pre_resync_checksums.gameturn = game.play_gameturn;
    pre_resync_checksums.is_stored = true;

    ERRORLOG("Stored pre-resync checksums at turn %ld: creatures=%08lx things=%08lx rooms=%08lx",
            pre_resync_checksums.gameturn, pre_resync_checksums.creatures_sum,
            pre_resync_checksums.things_sum, pre_resync_checksums.rooms_sum);
}


// Count entities in a thing list to help client identify source of desync
// Used when logging detailed breakdown of thing category mismatches
static int count_things_in_list(struct StructureList *list)
{
    int count = 0;
    int i = list->index;

    while (i != 0 && count < THINGS_COUNT) {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            break;
        }
        count++;
        i = thing->next_of_class;
    }
    return count;
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
static void analyze_things_mismatch_details(void)
{
    ERRORLOG("  Breaking down THINGS MISMATCH by category:");

    // Use detailed checksums computed before resync to ensure consistent comparison
    struct DetailedThingChecksums client_checksums = pre_resync_checksums.things_detailed;

    // Compare each thing category between client and host pre-resync states
    log_checksum_comparison("Traps", client_checksums.traps, game.desync_diagnostics.host_traps_sum);
    log_checksum_comparison("Shots", client_checksums.shots, game.desync_diagnostics.host_shots_sum);
    log_checksum_comparison("Objects", client_checksums.objects, game.desync_diagnostics.host_objects_sum);
    log_checksum_comparison("Effects", client_checksums.effects, game.desync_diagnostics.host_effects_sum);
    log_checksum_comparison("Dead Creatures", client_checksums.dead_creatures, game.desync_diagnostics.host_dead_creatures_sum);
    log_checksum_comparison("Effect Generators", client_checksums.effect_gens, game.desync_diagnostics.host_effect_gens_sum);
    log_checksum_comparison("Doors", client_checksums.doors, game.desync_diagnostics.host_doors_sum);

    // Show current thing counts to help identify which lists have extra/missing entities
    int trap_count = count_things_in_list(&game.thing_lists[TngList_Traps]);
    int shot_count = count_things_in_list(&game.thing_lists[TngList_Shots]);
    int object_count = count_things_in_list(&game.thing_lists[TngList_Objects]);
    int effect_count = count_things_in_list(&game.thing_lists[TngList_Effects]);
    int dead_creature_count = count_things_in_list(&game.thing_lists[TngList_DeadCreatrs]);
    int effect_gen_count = count_things_in_list(&game.thing_lists[TngList_EffectGens]);
    int door_count = count_things_in_list(&game.thing_lists[TngList_Doors]);

    ERRORLOG("  Thing counts - Traps: %d, Shots: %d, Objects: %d, Effects: %d, Dead: %d, EffGens: %d, Doors: %d",
             trap_count, shot_count, object_count, effect_count, dead_creature_count, effect_gen_count, door_count);
}


// Client compares local pre-resync checksums with host diagnostic data
// This analysis helps identify which game systems diverged between host and client
void analyze_desync_diagnostics_from_host(void)
{
    ERRORLOG("=== DESYNC ANALYSIS: Client (turn %ld) vs Host (turn %ld) ===", pre_resync_checksums.gameturn, game.desync_diagnostics.desync_turn);

    // Compare client pre-resync state with host pre-resync state from diagnostic data
    log_checksum_comparison("Creatures", pre_resync_checksums.creatures_sum, game.desync_diagnostics.host_creatures_sum);

    log_checksum_comparison("Things", pre_resync_checksums.things_sum, game.desync_diagnostics.host_things_sum);
    if (pre_resync_checksums.things_sum != game.desync_diagnostics.host_things_sum) {
        analyze_things_mismatch_details();
    }

    log_checksum_comparison("Rooms", pre_resync_checksums.rooms_sum, game.desync_diagnostics.host_rooms_sum);

    // Check each random seed individually
    log_checksum_comparison("GAME_RANDOM seed", pre_resync_checksums.action_random_seed, game.desync_diagnostics.host_action_random_seed);
    log_checksum_comparison("AI_RANDOM seed", pre_resync_checksums.ai_random_seed, game.desync_diagnostics.host_ai_random_seed);
    log_checksum_comparison("PLAYER_RANDOM seed", pre_resync_checksums.player_random_seed, game.desync_diagnostics.host_player_random_seed);

    // Check each player individually
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        struct PlayerInfo* player = get_player(i);
        TbBigChecksum client_checksum = pre_resync_checksums.player_checksums[i];
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
