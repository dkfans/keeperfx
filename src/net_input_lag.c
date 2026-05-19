#include "pre_inc.h"
#include "net_input_lag.h"

#include "globals.h"
#include "packets.h"
#include "player_data.h"
#include "net_game.h"
#include "game_legacy.h"
#include "bflib_enet.h"
#include "net_main.h"
#include "bflib_math.h"
#include "bflib_datetm.h"
#include "frontend.h"
#include "front_landview.h"
#include "front_network.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

enum InputLagMode {
    INPUT_LAG_MODE_ONE_VS_ONE = 0,
    INPUT_LAG_MODE_RELAY,
};

TbBool input_lag_skips_initial_processing(void) {
    if ((game.operation_flags & GOF_Paused) != 0 && game.game_kind != GKind_LocalGame) {return true;}

    if (game.skip_initial_input_turns > 0) {
        game.skip_initial_input_turns--;
        MULTIPLAYER_LOG("process_packets: Input lag skip turns remaining: %d, skipping packet processing", game.skip_initial_input_turns);
        return true;
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

void LbNetwork_UpdateInputLagIfHost(void) {
    static TbClockMSec last_update_ms = 0;
    static TbClockMSec player_count_change_time = 0;
    static int total_ping = 0;
    static int sample_count = 0;
    static int previous_remote_player_count = -1;
    if ((game.system_flags & GSF_NetworkActive) == 0) { return; }
    if (frontend_menu_state == FeSt_START_MPLEVEL) { return; }
    if (my_player_number != get_host_player_id()) { return; }
    if (!netstate.sp) { return; }
    TbClockMSec now = LbTimerClock();
    netstate.sp->update(OnNewUser);
    int remote_player_count = 0;
    NetUserId id;
    for (id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id) { continue; }
        if (netstate.users[id].progress == USER_LOGGEDIN) {
            remote_player_count += 1;
        }
    }
    if (remote_player_count == 0) {
        player_count_change_time = 0;
        sample_count = 0;
        total_ping = 0;
        previous_remote_player_count = 0;
        return;
    }
    enum InputLagMode mode = INPUT_LAG_MODE_RELAY;
    if (remote_player_count == 1) {
        mode = INPUT_LAG_MODE_ONE_VS_ONE;
    }
    if (previous_remote_player_count != remote_player_count) {
        player_count_change_time = now;
        sample_count = 0;
        total_ping = 0;
        previous_remote_player_count = remote_player_count;
    }
    if (player_count_change_time == 0) {
        player_count_change_time = now;
    }
    if (now - player_count_change_time < WAIT_FOR_STABLE_PLAYER) {
        sample_count = 0;
        total_ping = 0;
        return;
    }
    if (last_update_ms != 0 && now - last_update_ms < AVERAGE_PING_UPDATE_RATE) { return; }
    last_update_ms = now;
    int max_ping = 0;
    for (id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id) { continue; }
        if (!(netstate.users[id].progress == USER_LOGGEDIN)) { continue; }
        unsigned long ping = GetPing(id);
        if (ping <= 0) {
            MULTIPLAYER_LOG("Player %d (%s) has no RTT data yet", id, netstate.users[id].name);
            continue;
        }
        MULTIPLAYER_LOG("Player %d (%s) Ping: %lums", id, netstate.users[id].name, ping);
        if ((int)ping > max_ping) {
            max_ping = (int)ping;
        }
    }
    if (max_ping == 0) {
        return;
    }
    total_ping += max_ping;
    sample_count++;
    float average_ping = (float)total_ping / sample_count;
    float turn_time = 1000.0f / turns_per_second;
    float half_turn_time = (turn_time*0.5);
    float adjusted_ping = 0;
    switch (mode) {
        case INPUT_LAG_MODE_ONE_VS_ONE: adjusted_ping = (average_ping - half_turn_time) * 0.5f; break;
        case INPUT_LAG_MODE_RELAY:      adjusted_ping = (average_ping); break;
    }
    int input_lag = CEILING(adjusted_ping / turn_time);
    if (average_ping < 25) {input_lag = 0;} // LAN
    if (input_lag < 0) {input_lag = 0;} // Input lag cannot be below 0
    game.input_lag_turns = min(input_lag, MAXIMUM_INPUT_LAG_TURNS);
    MULTIPLAYER_LOG("Average Ping: %dms (samples: %d), Adjusted Ping: %.2fms, Setting Input Lag: %d", (int)average_ping, sample_count, adjusted_ping, game.input_lag_turns);
}

#ifdef __cplusplus
}
#endif
