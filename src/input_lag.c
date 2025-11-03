#include "pre_inc.h"
#include "input_lag.h"

#include "globals.h"
#include "packets.h"
#include "player_data.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct Packet local_input_lag_packets[MAXIMUM_INPUT_LAG_TURNS + 1];

void store_local_packet_in_input_lag_queue(PlayerNumber my_packet_num) {
    if (game.input_lag_turns + 1 <= 0) {
        return;
    }
    int slot = game.play_gameturn % (game.input_lag_turns + 1);
    local_input_lag_packets[slot] = game.packets[my_packet_num];
    MULTIPLAYER_LOG("store_local_packet_in_input_lag_queue: STORING local packet[%d] turn=%lu checksum=%08lx into queue slot %d",
            my_packet_num, (unsigned long)game.packets[my_packet_num].turn,
            (unsigned long)game.packets[my_packet_num].checksum, slot);
}

struct Packet* get_local_input_lag_packet_for_turn(GameTurn target_turn) {
    for (int i = 0; i < game.input_lag_turns + 1; i++) {
        struct Packet* packet = &local_input_lag_packets[i];
        if (!is_packet_empty(packet) && packet->turn == target_turn) {
            MULTIPLAYER_LOG("get_local_input_lag_packet_for_turn: found packet for turn=%lu in slot %d", (unsigned long)target_turn, i);
            return packet;
        }
    }
    MULTIPLAYER_LOG("get_local_input_lag_packet_for_turn: no packet found for turn=%lu", (unsigned long)target_turn);
    return NULL;
}

TbBool input_lag_should_skip_processing(void) {
    if (game.skip_initial_input_turns > 0) {
        game.skip_initial_input_turns--;
        MULTIPLAYER_LOG("process_packets: Input lag skip turns remaining: %d, skipping packet processing", game.skip_initial_input_turns);
        return true;
    }
    return false;
}

void clear_input_lag_queue(void) {
    memset(local_input_lag_packets, 0, sizeof(local_input_lag_packets));
}

#ifdef __cplusplus
}
#endif
