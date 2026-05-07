#ifndef INPUT_LAG_H
#define INPUT_LAG_H

#include "bflib_basics.h"
#include "globals.h"

#define MAXIMUM_INPUT_LAG_TURNS    40

struct Packet;

#ifdef __cplusplus
extern "C" {
#endif

void store_local_packet_in_input_lag_queue(PlayerNumber my_packet_num);
struct Packet* get_local_input_lag_packet_for_turn(GameTurn target_turn);
TbBool input_lag_skips_initial_processing(void);
void clear_input_lag_queue(void);
unsigned short calculate_skip_input(void);

#ifdef __cplusplus
}
#endif

#endif
