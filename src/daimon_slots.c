/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_slots.c
 *     The packet slots Daimon's keepers are played from.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "daimon_slots.h"

#include <string.h>

#include "daimon/Daimon.h"
#include "dungeon_data.h"
#include "engine_camera.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "net_game.h"
#include "packets.h"
#include "player_computer.h"
#include "player_data.h"
#include "player_utils.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint16_t pending_slots_mask;
static int64_t prev_models[PLAYERS_COUNT];
static GameTurn last_written_turn = (GameTurn)-1;

NetUserId daimon_slot_of(PlayerNumber plyr_idx)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) {
        return -1;
    }
    const struct PlayerInfo *player = get_player(plyr_idx);
    if (!player_exists(player) || !flag_is_set(player->allocflags, PlaF_Daimon)) {
        return -1;
    }
    for (NetUserId slot = MAX_NET_USERS; slot < PACKETS_COUNT; slot++) {
        if (get_net_user_player_number(slot) == plyr_idx) {
            return slot;
        }
    }
    return -1;
}

static NetUserId free_slot(void)
{
    for (NetUserId slot = MAX_NET_USERS; slot < PACKETS_COUNT; slot++) {
        if (get_net_user_player_number(slot) == PLAYER_NONE) {
            return slot;
        }
    }
    return -1;
}

NetUserId daimon_assign_slot(PlayerNumber plyr_idx, int64_t *prev_model)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) {
        return -1;
    }
    struct PlayerInfo *player = get_player(plyr_idx);
    if (!player_exists(player) || !flag_is_set(player->allocflags, PlaF_CompCtrl)
        || flag_is_set(player->allocflags, PlaF_Daimon | PlaF_Placeholder)) {
        return -1;
    }
    const NetUserId slot = free_slot();
    if (slot < 0) {
        WARNLOG("No free packet slot for player %d; it stays the computer's", (int)plyr_idx);
        return -1;
    }
    const struct Computer2 *comp = get_computer_player(plyr_idx);
    prev_models[plyr_idx] = (int64_t)comp->model;
    if (prev_model != NULL) {
        *prev_model = prev_models[plyr_idx];
    }
    // As seat_network_user() takes a keeper from the computer, for a slot instead of a network user.
    player->allocflags &= ~PlaF_CompCtrl;
    player->allocflags |= PlaF_Daimon;
    if ((get_players_dungeon(player)->computer_enabled & 0x01) != 0) {
        toggle_computer_player(plyr_idx);
    }
    char name[32];
    snprintf(name, sizeof(name), "Daimon %d", (int)(slot - MAX_NET_USERS + 1));
    init_user_state(slot, plyr_idx, name);
    init_user_controls(get_user_state(slot));
    reset_user_view_type(slot, PVT_DungeonTop);
    init_user_cameras(slot);
    memset(get_packet(slot), 0, sizeof(struct Packet));
    SYNCLOG("Daimon: slot %d plays player %d", (int)slot, (int)plyr_idx);
    return slot;
}

NetUserId daimon_adopt_slot(PlayerNumber plyr_idx)
{
    return daimon_assign_slot(plyr_idx, NULL);
}

int64_t daimon_slot_prev_model(PlayerNumber plyr_idx)
{
    return ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT)) ? prev_models[plyr_idx] : 0;
}

static void free_slot_of(PlayerNumber plyr_idx)
{
    const NetUserId slot = daimon_slot_of(plyr_idx);
    if (slot < 0) {
        return;
    }
    set_net_user_player_number(slot, PLAYER_NONE);
    memset(get_packet(slot), 0, sizeof(struct Packet));
    clear_flag(get_player(plyr_idx)->allocflags, PlaF_Daimon);
}

TbBool daimon_release_slot(PlayerNumber plyr_idx, int64_t computer_model)
{
    if (daimon_slot_of(plyr_idx) < 0) {
        return false;
    }
    free_slot_of(plyr_idx);
    if (!script_support_setup_player_as_computer_keeper(plyr_idx, computer_model)) {
        WARNLOG("Player %d could not be handed back to the computer", (int)plyr_idx);
    }
    SYNCLOG("Daimon: player %d handed back to the computer (model %d)", (int)plyr_idx, (int)computer_model);
    return true;
}

void daimon_forget_slot(PlayerNumber plyr_idx)
{
    free_slot_of(plyr_idx);
}

void daimon_pending_slots_clear(void)
{
    pending_slots_mask = 0;
}

void daimon_pending_slots_add(PlayerNumber plyr_idx)
{
    if ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT)) {
        pending_slots_mask |= (uint16_t)(1u << plyr_idx);
    }
}

uint16_t daimon_pending_slots_mask(void)
{
    return pending_slots_mask;
}

void daimon_pending_slots_set_mask(uint16_t mask)
{
    pending_slots_mask = mask;
}

int32_t daimon_claim_pending_slots(void)
{
    int32_t claimed = 0;
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++) {
        if ((pending_slots_mask & (1u << plyr_idx)) == 0) {
            continue;
        }
        if (daimon_assign_slot(plyr_idx, NULL) >= 0) {
            claimed++;
        }
    }
    pending_slots_mask = 0;
    return claimed;
}

void daimon_write_slot_packets(void)
{
    // A frame that waits for the exchange runs again: write each turn once.
    if (last_written_turn == get_gameturn()) {
        return;
    }
    last_written_turn = get_gameturn();
    for (NetUserId slot = MAX_NET_USERS; slot < PACKETS_COUNT; slot++) {
        if (get_net_user_player_number(slot) != PLAYER_NONE) {
            memset(get_packet(slot), 0, sizeof(struct Packet));
        }
    }
    DaimonTickAuthors(get_gameturn());
}

#ifdef __cplusplus
}
#endif
