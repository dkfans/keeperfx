#ifndef INPUT_LAG_H
#define INPUT_LAG_H

#include "globals.h"
#include "bflib_basics.h"

#define MAXIMUM_INPUT_LAG_TURNS 12
#define MISS_PERCENT_INC_THRESHOLD 65
#define MISS_PERCENT_DEC_THRESHOLD 1

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

TbBool input_lag_skips_processing(void);
unsigned short calculate_skip_input(void);
void input_lag_update(struct Packet *packet);
void input_lag_reset(void);
void input_lag_reset_request(int32_t input_lag_turns);
void input_lag_get_stats(int32_t *increase_miss_percent, int32_t *decrease_miss_percent, TbClockMSec *increase_countdown, TbClockMSec *decrease_countdown);
void input_lag_note_packet_wait(void);
void input_lag_observe_host_packet(const struct Packet *packet);
TbBool input_lag_needs_lookahead(void);

#ifdef __cplusplus
}
#endif

#endif
