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
#include "kfx_memory.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
// Increase quickly to prevent stutter, but decrease slowly to avoid oscillation.
#define INPUT_LAG_INCREASE_SAMPLE_WINDOW_TURNS 30
#define INPUT_LAG_DECREASE_MAX_SAMPLE_WINDOW_TURNS 6000

static int32_t input_lag_increase_sample_count;
static int32_t input_lag_decrease_sample_count;
static int32_t input_lag_history_position;
static int32_t input_lag_increase_missing_turn_count;
static int32_t input_lag_decrease_missing_turn_count;
static TbBool *input_lag_missing_turn_history;
static TbClockMSec input_lag_last_decrease_check;
static TbClockMSec input_lag_last_increase_check;
static int32_t local_input_lag_request;
static int32_t input_lag_target;
static int32_t input_lag_increase_turns;

static void input_lag_reset_history(void)
{
    input_lag_missing_turn_history = KfxRealloc(input_lag_missing_turn_history, INPUT_LAG_DECREASE_MAX_SAMPLE_WINDOW_TURNS * sizeof(*input_lag_missing_turn_history));
    input_lag_increase_sample_count = 0;
    input_lag_decrease_sample_count = 0;
    input_lag_history_position = -1;
    input_lag_increase_missing_turn_count = 0;
    input_lag_decrease_missing_turn_count = 0;
    memset(input_lag_missing_turn_history, 0, INPUT_LAG_DECREASE_MAX_SAMPLE_WINDOW_TURNS * sizeof(*input_lag_missing_turn_history));
    input_lag_last_decrease_check = input_lag_last_increase_check = LbTimerClock();
}

void input_lag_reset_request(int32_t input_lag_turns)
{
    input_lag_reset_history();
    local_input_lag_request = input_lag_turns;
}

void input_lag_reset(void)
{
    input_lag_reset_history();
    local_input_lag_request = game.input_lag_turns;
    input_lag_target = game.input_lag_turns;
    input_lag_increase_turns = 0;
}

void input_lag_get_stats(int32_t *increase_miss_percent, int32_t *decrease_miss_percent, TbClockMSec *increase_countdown, TbClockMSec *decrease_countdown)
{
    TbClockMSec now = LbTimerClock();
    TbClockMSec increase_window = INPUT_LAG_INCREASE_SAMPLE_WINDOW_TURNS * 1000 / turns_per_second;
    TbClockMSec decrease_window = INPUT_LAG_DECREASE_MAX_SAMPLE_WINDOW_TURNS * 1000 / turns_per_second / max(GetRemoteUserCount(), 1);
    TbClockMSec elapsed = now - input_lag_last_increase_check;
    *increase_miss_percent = 0;
    if (input_lag_increase_sample_count > 0) {
        *increase_miss_percent = (input_lag_increase_missing_turn_count * 100 + input_lag_increase_sample_count - 1) / input_lag_increase_sample_count;
    }
    *decrease_miss_percent = 0;
    if (input_lag_decrease_sample_count > 0) {
        *decrease_miss_percent = input_lag_decrease_missing_turn_count * 100 / input_lag_decrease_sample_count;
    }
    *increase_countdown = 0;
    if (elapsed < increase_window) {
        *increase_countdown = increase_window - elapsed;
    }
    elapsed = now - input_lag_last_decrease_check;
    *decrease_countdown = 0;
    if (elapsed < decrease_window) {
        *decrease_countdown = decrease_window - elapsed;
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
    TbClockMSec now = LbTimerClock();
    if ((game.operation_flags & GOF_Paused) != 0 || game.skip_initial_input_turns > 0) {
        input_lag_last_decrease_check = input_lag_last_increase_check = now;
    } else {
        int32_t decrease_sample_window_turns = INPUT_LAG_DECREASE_MAX_SAMPLE_WINDOW_TURNS / max(remote_player_count, 1);
        TbClockMSec decrease_window = decrease_sample_window_turns * 1000 / turns_per_second;
        if (now - input_lag_last_decrease_check >= decrease_window) {
            input_lag_last_decrease_check = now;
            if (input_lag_decrease_sample_count > 0 && input_lag_decrease_missing_turn_count * 100 < MISS_PERCENT_DEC_THRESHOLD * input_lag_decrease_sample_count && local_input_lag_request > 0) {
                local_input_lag_request -= 1;
                input_lag_last_increase_check = now;
                MULTIPLAYER_LOG("Input lag request decreased after %d waits in %d turns: request=%d next_check=%dms", input_lag_decrease_missing_turn_count, input_lag_decrease_sample_count, local_input_lag_request, decrease_window);
            }
        }
        TbClockMSec increase_window = INPUT_LAG_INCREASE_SAMPLE_WINDOW_TURNS * 1000 / turns_per_second;
        if (now - input_lag_last_increase_check >= increase_window) {
            input_lag_last_increase_check = now;
            if (input_lag_increase_missing_turn_count * 100 > MISS_PERCENT_INC_THRESHOLD * input_lag_increase_sample_count && local_input_lag_request < MAXIMUM_INPUT_LAG_TURNS) {
                local_input_lag_request += 1;
                MULTIPLAYER_LOG("Input lag request increased after %d waits in %d turns: request=%d next_check=%dms", input_lag_increase_missing_turn_count, input_lag_increase_sample_count, local_input_lag_request, increase_window);
            }
        }
        input_lag_history_position = (input_lag_history_position + 1) % decrease_sample_window_turns;
        input_lag_decrease_missing_turn_count -= input_lag_missing_turn_history[input_lag_history_position];
        int32_t increase_history_position = (input_lag_history_position + decrease_sample_window_turns - INPUT_LAG_INCREASE_SAMPLE_WINDOW_TURNS) % decrease_sample_window_turns;
        input_lag_increase_missing_turn_count -= input_lag_missing_turn_history[increase_history_position];
        input_lag_missing_turn_history[input_lag_history_position] = false;
        if (input_lag_increase_sample_count < INPUT_LAG_INCREASE_SAMPLE_WINDOW_TURNS) {
            input_lag_increase_sample_count += 1;
        }
        if (input_lag_decrease_sample_count < decrease_sample_window_turns) {
            input_lag_decrease_sample_count += 1;
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
        input_lag_reset_history();
        local_input_lag_request = 0;
        packet->input_lag_turns = 0;
    }
}

void input_lag_note_packet_wait(void)
{
    if (!network_is_active() || (game.operation_flags & GOF_Paused) != 0 || input_lag_history_position < 0) { return; }
    if (!input_lag_missing_turn_history[input_lag_history_position]) {
        input_lag_missing_turn_history[input_lag_history_position] = true;
        input_lag_increase_missing_turn_count += 1;
        input_lag_decrease_missing_turn_count += 1;
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
