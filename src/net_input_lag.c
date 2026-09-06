#include "pre_inc.h"
#include "net_input_lag.h"

#include "globals.h"
#include "packets.h"
#include "player_data.h"
#include "net_game.h"
#include "net_exchange_gameplay.h"
#include "game_legacy.h"
#include "net_main.h"
#include "bflib_datetm.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
#define INPUT_LAG_SAMPLE_START_TURN 50

static int32_t input_lag_decrease_sample_time;
static int32_t input_lag_decrease_wait_time;
static uint32_t input_lag_decrease_last_update;
static int32_t input_lag_time_spent_waiting;
static unsigned char input_lag_increase_wait_history[INPUT_LAG_INCREASE_SAMPLE_MS];
static uint32_t input_lag_increase_last_update;
static int32_t local_input_lag_request;
static int32_t input_lag_target;
static int32_t input_lag_increase_turns;
static GameTurn input_lag_next_increase_turn;
static int32_t input_lag_adjustment_time = INPUT_LAG_ADJUSTMENT_TIME_1V1_MS;

static void input_lag_update_increase_sample(uint32_t current_time)
{
    uint32_t elapsed = current_time - input_lag_increase_last_update;
    if (elapsed >= INPUT_LAG_INCREASE_SAMPLE_MS) {
        memset(input_lag_increase_wait_history, 0, sizeof(input_lag_increase_wait_history));
        input_lag_time_spent_waiting = 0;
    } else {
        for (uint32_t offset = 1; offset <= elapsed; offset += 1) {
            uint32_t index = (input_lag_increase_last_update + offset) % INPUT_LAG_INCREASE_SAMPLE_MS;
            input_lag_time_spent_waiting -= input_lag_increase_wait_history[index];
            input_lag_increase_wait_history[index] = 0;
        }
    }
    input_lag_increase_last_update = current_time;
}

static void input_lag_reset_samples(void)
{
    input_lag_decrease_sample_time = 0;
    input_lag_decrease_wait_time = 0;
    input_lag_decrease_last_update = LbTimerClock();
    input_lag_time_spent_waiting = 0;
    memset(input_lag_increase_wait_history, 0, sizeof(input_lag_increase_wait_history));
    input_lag_increase_last_update = input_lag_decrease_last_update;
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

void input_lag_get_stats(int32_t *increase_wait_time, int32_t *increase_turn_time, int32_t *decrease_wait_time, int32_t *decrease_sample_time)
{
    *increase_wait_time = input_lag_time_spent_waiting;
    *increase_turn_time = INPUT_LAG_INCREASE_WAIT_MS;
    *decrease_wait_time = input_lag_decrease_wait_time;
    *decrease_sample_time = input_lag_decrease_sample_time;
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
    input_lag_adjustment_time = INPUT_LAG_ADJUSTMENT_TIME_1V1_MS;
    if (remote_player_count > 1) {
        input_lag_adjustment_time = INPUT_LAG_ADJUSTMENT_TIME_HOST_RELAY_MS;
    }
    if ((game.operation_flags & GOF_Paused) == 0 && input_lag_increase_turns == 0 && input_lag_target > game.input_lag_turns) {
        JUSTLOG("Input lag increased from %d to %d", game.input_lag_turns, input_lag_target);
        game.input_lag_turns = input_lag_target;
    }
    uint32_t current_time = LbTimerClock();
    input_lag_update_increase_sample(current_time);
    if ((game.operation_flags & GOF_Paused) == 0 && game.skip_initial_input_turns == 0 && get_gameturn() >= INPUT_LAG_SAMPLE_START_TURN) {
        uint32_t sample_time = current_time - input_lag_decrease_last_update;
        if (sample_time > input_lag_adjustment_time) {
            sample_time = input_lag_adjustment_time;
        }
        input_lag_decrease_sample_time += sample_time;
        if (input_lag_decrease_sample_time >= input_lag_adjustment_time) {
            if (input_lag_decrease_wait_time * 100 <= INPUT_LAG_DECREASE_WAIT_PERCENT * input_lag_decrease_sample_time && local_input_lag_request > 0) {
                local_input_lag_request -= 1;
                input_lag_next_increase_turn = 0;
                MULTIPLAYER_LOG("Input lag request decreased after %dms spent waiting in %dms: request=%d", input_lag_decrease_wait_time, input_lag_decrease_sample_time, local_input_lag_request);
                input_lag_reset_samples();
            } else {
                input_lag_decrease_sample_time = 0;
                input_lag_decrease_wait_time = 0;
            }
        }
    }
    input_lag_decrease_last_update = current_time;
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

void input_lag_note_packet_wait(int32_t wait_time)
{
    if (!network_is_active() || (game.operation_flags & GOF_Paused) != 0 || get_gameturn() < INPUT_LAG_SAMPLE_START_TURN) { return; }
    uint32_t current_time = LbTimerClock();
    input_lag_update_increase_sample(current_time);
    input_lag_decrease_wait_time += wait_time;
    if (wait_time > INPUT_LAG_INCREASE_SAMPLE_MS) {
        wait_time = INPUT_LAG_INCREASE_SAMPLE_MS;
    }
    for (int32_t offset = 0; offset < wait_time; offset += 1) {
        uint32_t index = (current_time % INPUT_LAG_INCREASE_SAMPLE_MS + INPUT_LAG_INCREASE_SAMPLE_MS - offset) % INPUT_LAG_INCREASE_SAMPLE_MS;
        if (input_lag_increase_wait_history[index] == 0) {
            input_lag_increase_wait_history[index] = 1;
            input_lag_time_spent_waiting += 1;
        }
    }
    if (input_lag_time_spent_waiting >= INPUT_LAG_INCREASE_WAIT_MS && local_input_lag_request < MAXIMUM_INPUT_LAG_TURNS && get_gameturn() >= input_lag_next_increase_turn) {
        local_input_lag_request += 1;
        input_lag_next_increase_turn = get_gameturn() + (int64_t)input_lag_adjustment_time * turns_per_second / 1000;
        MULTIPLAYER_LOG("Input lag request increased after %dms spent waiting: request=%d", input_lag_time_spent_waiting, local_input_lag_request);
        input_lag_reset_samples();
    }
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
