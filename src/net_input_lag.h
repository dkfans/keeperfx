#ifndef INPUT_LAG_H
#define INPUT_LAG_H

#include "globals.h"
#include "bflib_basics.h"

#define MAXIMUM_INPUT_LAG_TURNS 12
// The minimum amount of time needed to achieve a stable reading
#define INPUT_LAG_INCREASE_SAMPLE_MS 150
#define INPUT_LAG_INCREASE_WAIT_MS 75
// Host relay and 1v1 times are treated differently because the value in 4-player takes much longer to shift (requires all players to have the same value for a decrease).
#define INPUT_LAG_ADJUSTMENT_TIME_1V1_MS 60000
#define INPUT_LAG_ADJUSTMENT_TIME_HOST_RELAY_MS 15000
// Percentage of misses allowed before a decrease, to take noise into consideration
#define INPUT_LAG_DECREASE_WAIT_PERCENT 1

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

TbBool input_lag_skips_processing(void);
unsigned short calculate_skip_input(void);
void input_lag_update(struct Packet *packet);
void input_lag_reset(void);
void input_lag_reset_request(int32_t input_lag_turns);
void input_lag_get_stats(int32_t *increase_wait_time, int32_t *increase_turn_time, int32_t *decrease_wait_time, int32_t *decrease_sample_time);
void input_lag_note_packet_wait(int32_t wait_time);
void input_lag_observe_host_packet(const struct Packet *packet);
TbBool input_lag_needs_lookahead(void);

#ifdef __cplusplus
}
#endif

#endif
