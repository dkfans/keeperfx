#ifndef INPUT_LAG_H
#define INPUT_LAG_H

#include "globals.h"
#include "bflib_basics.h"

#define MAXIMUM_INPUT_LAG_TURNS 12
#define INPUT_LAG_INCREASE_MISSES 3
#define INPUT_LAG_INCREASE_TURNS 4
#define INPUT_LAG_INCREASE_COOLDOWN_TURNS 600
#define INPUT_LAG_DECREASE_SAMPLE_TURNS 600
#define INPUT_LAG_DECREASE_MISS_PERCENT 1

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

TbBool input_lag_skips_processing(void);
unsigned short calculate_skip_input(void);
void input_lag_update(struct Packet *packet);
void input_lag_reset(void);
void input_lag_reset_request(int32_t input_lag_turns);
void input_lag_get_stats(int32_t *increase_missed_turns, int32_t *increase_sampled_turns, int32_t *decrease_missed_turns, int32_t *decrease_sampled_turns);
void input_lag_note_packet_wait(void);
void input_lag_observe_host_packet(const struct Packet *packet);
TbBool input_lag_needs_lookahead(void);

#ifdef __cplusplus
}
#endif

#endif
