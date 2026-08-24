#include "pre_inc.h"
#include "net_input_lag.h"

#include "globals.h"
#include "packets.h"
#include "player_data.h"
#include "net_game.h"
#include "net_exchange_gameplay.h"
#include "game_legacy.h"
#include "net_main.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
#define INPUT_LAG_SAMPLE_START_TURN 100

static int32_t input_lag_increase_sample_count;
static int32_t input_lag_decrease_sample_count;
static int32_t input_lag_decrease_missed_turn_count;
static uint8_t input_lag_missed_turn_history;
static int32_t local_input_lag_request;
static int32_t input_lag_target;
static int32_t input_lag_increase_turns;
static GameTurn input_lag_next_increase_turn;

static void input_lag_reset_samples(void)
{
    input_lag_increase_sample_count = 0;
    input_lag_decrease_sample_count = 0;
    input_lag_decrease_missed_turn_count = 0;
    input_lag_missed_turn_history = 0;
}

void input_lag_reset_request(int32_t input_lag_turns)
{
    input_lag_reset_samples();
    local_input_lag_request = input_lag_turns;
    input_lag_next_increase_turn = 0;
}

void input_lag_reset(void)
{
    input_lag_reset_samples();
    local_input_lag_request = game.input_lag_turns;
    input_lag_target = game.input_lag_turns;
    input_lag_increase_turns = 0;
    input_lag_next_increase_turn = 0;
}

void input_lag_get_stats(int32_t *increase_missed_turns, int32_t *increase_sampled_turns, int32_t *decrease_missed_turns, int32_t *decrease_sampled_turns)
{
    *increase_missed_turns = __builtin_popcount(input_lag_missed_turn_history);
    *increase_sampled_turns = input_lag_increase_sample_count;
    *decrease_missed_turns = input_lag_decrease_missed_turn_count;
    *decrease_sampled_turns = input_lag_decrease_sample_count;
}

TbBool input_lag_skips_processing(void)
{
    if (!network_is_active()) {
        return false;
    }
    if ((game.operation_flags & GOF_Paused) != 0) {
        return true;
    }

    if (game.skip_initial_input_turns > 0) {
        game.skip_initial_input_turns--;
        MULTIPLAYER_LOG("process_packets: Input lag skip turns remaining: %d, skipping packet processing", game.skip_initial_input_turns);
        return true;
    }
    if (input_lag_increase_turns > 0) {
        input_lag_increase_turns -= 1;
        MULTIPLAYER_LOG("Input lag increase: skipping input turn, remaining=%d", input_lag_increase_turns);
        return true;
    }
    if (input_lag_target < game.input_lag_turns) {
        JUSTLOG("Input lag decreased from %d to %d; discarded_turn=%lu target=%d", game.input_lag_turns, game.input_lag_turns - 1, (unsigned long)(get_gameturn() - game.input_lag_turns), input_lag_target);
        game.input_lag_turns -= 1;
    }
    return false;
}

const int heartZoomTime = 35; //30 isn't enough, it causes palette issues if it desyncs during the heart zoom
unsigned short calculate_skip_input(void) {
    if (get_gameturn() <= heartZoomTime) {
        return game.input_lag_turns + heartZoomTime;
    }
    return game.input_lag_turns + (turns_per_second * 0.25);
}

void input_lag_update(struct Packet *packet)
{
    packet->input_lag_turns = 0;
    if (!network_is_active()) { return; }
    int32_t remote_player_count = GetRemoteUserCount();
    if ((game.operation_flags & GOF_Paused) == 0 && input_lag_increase_turns == 0 && input_lag_target > game.input_lag_turns) {
        JUSTLOG("Input lag increased from %d to %d", game.input_lag_turns, input_lag_target);
        game.input_lag_turns = input_lag_target;
    }
    if ((game.operation_flags & GOF_Paused) == 0 && game.skip_initial_input_turns == 0 && get_gameturn() >= INPUT_LAG_SAMPLE_START_TURN) {
        if (input_lag_increase_sample_count > 0) {
            input_lag_decrease_sample_count += 1;
            if ((input_lag_missed_turn_history & 1) != 0) {
                input_lag_decrease_missed_turn_count += 1;
            }
        }
        int32_t missed_turns = __builtin_popcount(input_lag_missed_turn_history);
        if (input_lag_increase_sample_count == INPUT_LAG_INCREASE_TURNS && missed_turns >= INPUT_LAG_INCREASE_MISSES && local_input_lag_request < MAXIMUM_INPUT_LAG_TURNS && get_gameturn() >= input_lag_next_increase_turn) {
            local_input_lag_request += 1;
            input_lag_next_increase_turn = get_gameturn() + INPUT_LAG_INCREASE_COOLDOWN_TURNS;
            MULTIPLAYER_LOG("Input lag request increased after %d waits in %d turns: request=%d", missed_turns, input_lag_increase_sample_count, local_input_lag_request);
            input_lag_reset_samples();
        } else if (input_lag_decrease_sample_count >= INPUT_LAG_DECREASE_SAMPLE_TURNS) {
            if (input_lag_decrease_missed_turn_count * 100 <= INPUT_LAG_DECREASE_MISS_PERCENT * input_lag_decrease_sample_count && local_input_lag_request > 0) {
                local_input_lag_request -= 1;
                input_lag_next_increase_turn = 0;
                MULTIPLAYER_LOG("Input lag request decreased after %d waits in %d turns: request=%d", input_lag_decrease_missed_turn_count, input_lag_decrease_sample_count, local_input_lag_request);
                input_lag_reset_samples();
            } else {
                input_lag_decrease_sample_count = 0;
                input_lag_decrease_missed_turn_count = 0;
            }
        }
        input_lag_missed_turn_history = (input_lag_missed_turn_history << 1) & ((1 << INPUT_LAG_INCREASE_TURNS) - 1);
        if (input_lag_increase_sample_count < INPUT_LAG_INCREASE_TURNS) {
            input_lag_increase_sample_count += 1;
        }
    }
    packet->input_lag_turns = local_input_lag_request;
    if (my_player_number != get_host_player_id() || !netstate.sp) { return; }
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id || netstate.users[id].progress != USER_LOGGEDIN) { continue; }
        const struct Packet *peer_packet = get_latest_history_packet((PlayerNumber)id);
        if (peer_packet != NULL && (uint8_t)peer_packet->input_lag_turns <= MAXIMUM_INPUT_LAG_TURNS && peer_packet->input_lag_turns > packet->input_lag_turns) {
            packet->input_lag_turns = peer_packet->input_lag_turns;
        }
    }
    if (remote_player_count <= 0) {
        input_lag_reset_samples();
        local_input_lag_request = 0;
        packet->input_lag_turns = 0;
    }
}

void input_lag_note_packet_wait(void)
{
    if (!network_is_active() || (game.operation_flags & GOF_Paused) != 0 || get_gameturn() < INPUT_LAG_SAMPLE_START_TURN || input_lag_increase_sample_count <= 0) { return; }
    input_lag_missed_turn_history |= 1;
}

void input_lag_observe_host_packet(const struct Packet *packet)
{
    int32_t target = packet->input_lag_turns;
    if ((uint32_t)target > MAXIMUM_INPUT_LAG_TURNS) {
        WARNLOG("Ignoring invalid input lag target %d", target);
        return;
    }
    if (target == input_lag_target) {
        return;
    }
    input_lag_target = target;
    if (target > game.input_lag_turns) {
        input_lag_increase_turns = target - game.input_lag_turns;
    }
    MULTIPLAYER_LOG("Input lag target synchronized: current=%d target=%d", game.input_lag_turns, input_lag_target);
}

TbBool input_lag_needs_lookahead(void)
{
    return network_is_active() && (game.operation_flags & GOF_Paused) == 0 && game.skip_initial_input_turns == 0 && input_lag_increase_turns == 0 && input_lag_target < game.input_lag_turns;
}

#ifdef __cplusplus
}
#endif
