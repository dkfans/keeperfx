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
// Used for TOLERATE_PACKET_MISS_PERCENT
#define INPUT_LAG_SAMPLE_WINDOW_TURNS 100
// Input lag increases when the packet miss percentage exceeds this value within the sample window. [Higher = more choppy FPS but lower input lag, lower = smoother FPS but higher input lag]
#define TOLERATE_PACKET_MISS_PERCENT 70
// Starts out low so we can more quickly determine the correct input_lag_turns at the start of the level, but doubles and takes longer to change as the game progresses.
#define INPUT_LAG_INCREASE_INITIAL_INTERVAL_MS 600
#define INPUT_LAG_DECREASE_INITIAL_INTERVAL_MS 500
// When increasing input_lag_turns, cap the interval so that we can always get out of a laggy spot. Decreasing has no cap so that it will do less decreases as the game progresses.
#define INPUT_LAG_INCREASE_MAX_INTERVAL_MS 5000

static int32_t input_lag_sample_count;
static int32_t input_lag_history_position;
static int32_t input_lag_missing_turn_count;
static TbBool input_lag_missing_turn_history[INPUT_LAG_SAMPLE_WINDOW_TURNS];
static TbClockMSec input_lag_reduction_check_interval;
static TbClockMSec input_lag_increase_check_interval;
static TbClockMSec input_lag_last_reduction_check;
static TbClockMSec input_lag_last_increase_check;
static int32_t local_input_lag_request;
static int32_t input_lag_target;
static int32_t input_lag_increase_turns;

static void input_lag_reset_history(TbClockMSec now)
{
    input_lag_sample_count = 0;
    input_lag_history_position = -1;
    input_lag_missing_turn_count = 0;
    memset(input_lag_missing_turn_history, 0, sizeof(input_lag_missing_turn_history));
    input_lag_reduction_check_interval = INPUT_LAG_DECREASE_INITIAL_INTERVAL_MS;
    input_lag_increase_check_interval = INPUT_LAG_INCREASE_INITIAL_INTERVAL_MS;
    input_lag_last_reduction_check = input_lag_last_increase_check = now;
}

void input_lag_reset(void)
{
    input_lag_reset_history(LbTimerClock());
    local_input_lag_request = game.input_lag_turns;
    input_lag_target = game.input_lag_turns;
    input_lag_increase_turns = 0;
}

void input_lag_get_stats(int32_t *packet_misses, TbClockMSec *increase_countdown, TbClockMSec *decrease_countdown)
{
    TbClockMSec now = LbTimerClock();
    TbClockMSec elapsed = now - input_lag_last_increase_check;
    *packet_misses = input_lag_missing_turn_count;
    *increase_countdown = 0;
    if (elapsed < input_lag_increase_check_interval) {
        *increase_countdown = input_lag_increase_check_interval - elapsed;
    }
    elapsed = now - input_lag_last_reduction_check;
    *decrease_countdown = 0;
    if (elapsed < input_lag_reduction_check_interval) {
        *decrease_countdown = input_lag_reduction_check_interval - elapsed;
    }
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
        MULTIPLAYER_LOG("Input lag decrease forced: discarded_turn=%lu current=%d target=%d", (unsigned long)(get_gameturn() - game.input_lag_turns), game.input_lag_turns - 1, input_lag_target);
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
    if ((game.operation_flags & GOF_Paused) == 0 && input_lag_increase_turns == 0 && input_lag_target > game.input_lag_turns) {
        game.input_lag_turns = input_lag_target;
        MULTIPLAYER_LOG("Input lag increase committed: current=%d", game.input_lag_turns);
    }
    TbClockMSec now = LbTimerClock();
    if ((game.operation_flags & GOF_Paused) != 0 || game.skip_initial_input_turns > 0) {
        input_lag_last_reduction_check = input_lag_last_increase_check = now;
    } else {
        if (now - input_lag_last_reduction_check >= input_lag_reduction_check_interval) {
            input_lag_last_reduction_check = now;
            if (input_lag_reduction_check_interval <= INT32_MAX / 2) {
                input_lag_reduction_check_interval *= 2;
            }
            if (input_lag_sample_count > 0 && input_lag_missing_turn_count == 0 && local_input_lag_request > 0) {
                local_input_lag_request -= 1;
                input_lag_last_increase_check = now;
                MULTIPLAYER_LOG("Input lag request decreased after %d clean turns: request=%d next_check=%dms", input_lag_sample_count, local_input_lag_request, input_lag_reduction_check_interval);
            }
        }
        if (now - input_lag_last_increase_check >= input_lag_increase_check_interval) {
            input_lag_last_increase_check = now;
            input_lag_increase_check_interval *= 2;
            if (input_lag_increase_check_interval > INPUT_LAG_INCREASE_MAX_INTERVAL_MS) {
                input_lag_increase_check_interval = INPUT_LAG_INCREASE_MAX_INTERVAL_MS;
            }
            if (input_lag_missing_turn_count * 100 > TOLERATE_PACKET_MISS_PERCENT * input_lag_sample_count && local_input_lag_request < MAXIMUM_INPUT_LAG_TURNS) {
                local_input_lag_request += 1;
                MULTIPLAYER_LOG("Input lag request increased after %d waits in %d turns: request=%d next_check=%dms", input_lag_missing_turn_count, input_lag_sample_count, local_input_lag_request, input_lag_increase_check_interval);
            }
        }
        input_lag_history_position = (input_lag_history_position + 1) % INPUT_LAG_SAMPLE_WINDOW_TURNS;
        input_lag_missing_turn_count -= input_lag_missing_turn_history[input_lag_history_position];
        input_lag_missing_turn_history[input_lag_history_position] = false;
        if (input_lag_sample_count < INPUT_LAG_SAMPLE_WINDOW_TURNS) {
            input_lag_sample_count += 1;
        }
    }
    packet->input_lag_turns = local_input_lag_request;
    if (my_player_number != get_host_player_id() || !netstate.sp) { return; }
    int32_t remote_player_count = 0;
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id || netstate.users[id].progress != USER_LOGGEDIN) { continue; }
        remote_player_count += 1;
        const struct Packet *peer_packet = get_latest_history_packet((PlayerNumber)id);
        if (peer_packet != NULL && (uint8_t)peer_packet->input_lag_turns <= MAXIMUM_INPUT_LAG_TURNS && peer_packet->input_lag_turns > packet->input_lag_turns) {
            packet->input_lag_turns = peer_packet->input_lag_turns;
        }
    }
    if (remote_player_count <= 0) {
        input_lag_reset_history(now);
        local_input_lag_request = 0;
        packet->input_lag_turns = 0;
    }
}

void input_lag_note_packet_wait(void)
{
    if (!network_is_active() || (game.operation_flags & GOF_Paused) != 0 || input_lag_history_position < 0) { return; }
    if (!input_lag_missing_turn_history[input_lag_history_position]) {
        input_lag_missing_turn_history[input_lag_history_position] = true;
        input_lag_missing_turn_count += 1;
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
