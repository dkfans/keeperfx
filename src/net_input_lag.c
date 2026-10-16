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
#define INPUT_LAG_UNDROPPABLE_CONTROLS (PCtr_LBtnClick | PCtr_RBtnClick | PCtr_LBtnRelease | PCtr_RBtnRelease | PCtr_ViewTiltReset)

static GameTurn input_lag_sent_turns[PACKET_HISTORY_SIZE];
static TbClockMSec input_lag_sent_times[PACKET_HISTORY_SIZE];
static int32_t input_lag_packet_delays[INPUT_LAG_DECREASE_WINDOW_TURNS];
static int32_t input_lag_delay_position;
static int32_t input_lag_delay_count;
static int32_t input_lag_recent_delay;
static int32_t input_lag_increase_level;
static int32_t input_lag_decrease_level;
static int32_t local_input_lag_request;
static int32_t input_lag_target;
static GameTurn input_lag_processed_turn;
static TbBool input_lag_sampling;

static void input_lag_reset_samples(void)
{
    memset(input_lag_sent_turns, 0, sizeof(input_lag_sent_turns));
    memset(input_lag_packet_delays, 0, sizeof(input_lag_packet_delays));
    input_lag_delay_position = 0;
    input_lag_delay_count = 0;
    input_lag_recent_delay = 0;
    input_lag_increase_level = 0;
    input_lag_decrease_level = 0;
}

void input_lag_reset_request(int32_t input_lag_turns)
{
    input_lag_reset_samples();
    local_input_lag_request = input_lag_turns;
}

void input_lag_reset(void)
{
    input_lag_reset_samples();
    local_input_lag_request = game.input_lag_turns;
    input_lag_target = game.input_lag_turns;
    input_lag_processed_turn = 0;
    input_lag_sampling = false;
}

void input_lag_get_stats(struct InputLagStats *stats)
{
    stats->target = input_lag_target;
    stats->packet_delay = input_lag_recent_delay;
    stats->increase_level = input_lag_increase_level;
    stats->decrease_level = input_lag_decrease_level;
    stats->sample_count = input_lag_delay_count;
}

static TbBool input_lag_turn_is_droppable(GameTurn turn)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user += 1) {
        if (!user_present(user)) {
            continue;
        }
        const struct Packet *packet = get_history_packet(user, turn);
        if (packet != NULL && (packet->action != PckA_None || (packet->control_flags & INPUT_LAG_UNDROPPABLE_CONTROLS) != 0)) {
            return false;
        }
    }
    return true;
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
    GameTurn turn = get_gameturn() - game.input_lag_turns;
    if ((GameTurnDelta)(turn - input_lag_processed_turn) <= 0) {
        MULTIPLAYER_LOG("Input lag increase: skipping input turn, turn=%lu already processed", (unsigned long)turn);
        return true;
    }
    if (input_lag_target < game.input_lag_turns && input_lag_turn_is_droppable(turn)) {
        JUSTLOG("Input lag decreased from %d to %d; discarded_turn=%lu target=%d", game.input_lag_turns, game.input_lag_turns - 1, (unsigned long)turn, input_lag_target);
        game.input_lag_turns -= 1;
        turn += 1;
    }
    input_lag_processed_turn = turn;
    return false;
}

const int heartZoomTime = 35; //30 isn't enough, it causes palette issues if it desyncs during the heart zoom
unsigned short calculate_skip_input(void) {
    if (get_gameturn() <= heartZoomTime) {
        return game.input_lag_turns + heartZoomTime;
    }
    return game.input_lag_turns + (turns_per_second * 0.25);
}

static int32_t input_lag_window_delay(int32_t window_turns)
{
    int32_t largest_delays[INPUT_LAG_IGNORED_SPIKES + 1] = {0};
    if (window_turns > input_lag_delay_count) {
        window_turns = input_lag_delay_count;
    }
    for (int32_t age = 0; age < window_turns; age += 1) {
        int32_t delay = input_lag_packet_delays[(input_lag_delay_position + INPUT_LAG_DECREASE_WINDOW_TURNS - age) % INPUT_LAG_DECREASE_WINDOW_TURNS];
        for (int32_t rank = 0; rank <= INPUT_LAG_IGNORED_SPIKES; rank += 1) {
            if (delay > largest_delays[rank]) {
                int32_t smaller_delay = largest_delays[rank];
                largest_delays[rank] = delay;
                delay = smaller_delay;
            }
        }
    }
    return largest_delays[INPUT_LAG_IGNORED_SPIKES];
}

static int32_t input_lag_turns_for_delay(int32_t delay, int32_t tolerated_wait_ms)
{
    int32_t wait_time = delay - tolerated_wait_ms;
    if (wait_time <= 0 || turns_per_second <= 0) {
        return 0;
    }
    return min((wait_time * turns_per_second + 999) / 1000, MAXIMUM_INPUT_LAG_TURNS);
}

static void input_lag_update_request(void)
{
    input_lag_recent_delay = input_lag_window_delay(INPUT_LAG_INCREASE_WINDOW_TURNS);
    input_lag_increase_level = input_lag_turns_for_delay(input_lag_recent_delay, INPUT_LAG_TOLERATED_WAIT_MS);
    int32_t settled_delay = input_lag_window_delay(INPUT_LAG_DECREASE_WINDOW_TURNS);
    input_lag_decrease_level = max(input_lag_turns_for_delay(settled_delay, INPUT_LAG_TOLERATED_WAIT_MS - INPUT_LAG_DECREASE_MARGIN_MS), input_lag_increase_level);
    if (input_lag_increase_level > local_input_lag_request) {
        MULTIPLAYER_LOG("Input lag request increased from %d to %d: packet delay=%dms", local_input_lag_request, input_lag_increase_level, input_lag_recent_delay);
        local_input_lag_request = input_lag_increase_level;
    } else if (input_lag_delay_count >= INPUT_LAG_DECREASE_WINDOW_TURNS && input_lag_decrease_level < local_input_lag_request) {
        MULTIPLAYER_LOG("Input lag request decreased from %d to %d: packet delay=%dms", local_input_lag_request, input_lag_decrease_level, settled_delay);
        local_input_lag_request = input_lag_decrease_level;
    }
    input_lag_delay_position = (input_lag_delay_position + 1) % INPUT_LAG_DECREASE_WINDOW_TURNS;
    input_lag_packet_delays[input_lag_delay_position] = 0;
    if (input_lag_delay_count < INPUT_LAG_DECREASE_WINDOW_TURNS) {
        input_lag_delay_count += 1;
    }
}

void input_lag_update(struct Packet *packet)
{
    packet->input_lag_turns = 0;
    if (!network_is_active()) { return; }
    GameTurn turn = get_gameturn();
    input_lag_sent_turns[turn % PACKET_HISTORY_SIZE] = turn;
    input_lag_sent_times[turn % PACKET_HISTORY_SIZE] = LbTimerClock();
    input_lag_sampling = (game.operation_flags & GOF_Paused) == 0 && game.skip_initial_input_turns == 0 && turn >= INPUT_LAG_SAMPLE_START_TURN;
    if (input_lag_sampling) {
        input_lag_update_request();
    }
    packet->input_lag_turns = local_input_lag_request;
    if (!network_is_host() || !netstate.sp) { return; }
    for (NetUserId id = 0; id < netstate.max_users; id += 1) {
        if (id == netstate.my_id || netstate.users[id].progress != USER_LOGGEDIN) { continue; }
        const struct Packet *peer_packet = get_latest_history_packet(id);
        if (peer_packet != NULL && (uint8_t)peer_packet->input_lag_turns <= MAXIMUM_INPUT_LAG_TURNS && peer_packet->input_lag_turns > packet->input_lag_turns) {
            packet->input_lag_turns = peer_packet->input_lag_turns;
        }
    }
    if (GetRemoteUserCount() <= 0) {
        input_lag_reset_samples();
        local_input_lag_request = 0;
        packet->input_lag_turns = 0;
    }
}

void input_lag_note_packet_arrival(GameTurn turn)
{
    if (!input_lag_sampling || input_lag_delay_count == 0 || input_lag_sent_turns[turn % PACKET_HISTORY_SIZE] != turn) {
        return;
    }
    int32_t delay = LbTimerClock() - input_lag_sent_times[turn % PACKET_HISTORY_SIZE];
    if (delay > input_lag_packet_delays[input_lag_delay_position]) {
        input_lag_packet_delays[input_lag_delay_position] = delay;
    }
}

void input_lag_observe_host_packet(const struct Packet *packet)
{
    int32_t target = packet->input_lag_turns;
    if (is_packet_empty(packet) || target == input_lag_target) {
        return;
    }
    if ((uint32_t)target > MAXIMUM_INPUT_LAG_TURNS) {
        WARNLOG("Ignoring invalid input lag target %d", target);
        return;
    }
    input_lag_target = target;
    if (target > game.input_lag_turns) {
        JUSTLOG("Input lag increased from %d to %d", game.input_lag_turns, target);
        game.input_lag_turns = target;
    }
    MULTIPLAYER_LOG("Input lag target synchronized: current=%d target=%d", game.input_lag_turns, input_lag_target);
}

TbBool input_lag_needs_lookahead(void)
{
    return network_is_active() && (game.operation_flags & GOF_Paused) == 0 && game.skip_initial_input_turns == 0 && input_lag_target < game.input_lag_turns && input_lag_turn_is_droppable(get_gameturn() - game.input_lag_turns);
}

#ifdef __cplusplus
}
#endif
