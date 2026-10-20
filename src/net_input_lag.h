#ifndef INPUT_LAG_H
#define INPUT_LAG_H

#include "globals.h"
#include "bflib_basics.h"

#define MAXIMUM_INPUT_LAG_TURNS 12
// Each turn records the worst packet delay; the largest few delays in a window are ignored so isolated hiccups don't change the input lag.
#define INPUT_LAG_IGNORED_SPIKES 2
#define INPUT_LAG_INCREASE_WINDOW_TURNS 20
#define INPUT_LAG_DECREASE_WINDOW_TURNS 300
// Waiting this long for a packet each turn is tolerated, in exchange for lower input lag.
#define INPUT_LAG_TOLERATED_WAIT_MS 35
// Extra headroom required before a decrease, so the input lag can't oscillate between two values.
#define INPUT_LAG_DECREASE_MARGIN_MS 10

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

struct InputLagStats {
    int32_t target;
    int32_t packet_delay;
    int32_t increase_level;
    int32_t decrease_level;
    int32_t sample_count;
};

TbBool input_lag_skips_processing(void);
unsigned short calculate_skip_input(void);
void input_lag_update(struct Packet *packet);
void input_lag_reset(void);
void input_lag_reset_request(int32_t input_lag_turns);
void input_lag_get_stats(struct InputLagStats *stats);
void input_lag_note_packet_arrival(GameTurn turn);
void input_lag_observe_host_packet(const struct Packet *packet);
TbBool input_lag_needs_lookahead(void);
GameTurn input_lag_first_pending_turn(void);

#ifdef __cplusplus
}
#endif

#endif
