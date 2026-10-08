#ifndef DK_OBSERVER_H
#define DK_OBSERVER_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayerInfo;
struct UserState;
struct Packet;

extern struct PlayerInfo local_observer_player;
extern struct UserState local_observer_user_state;
extern TbBool local_observer_rendering;

TbBool observer_is_active(void);
void observer_reset(void);
void observer_init(PlayerNumber camera_player_number);
void observer_set_packet_cursor(struct Packet *pckt);
void observer_store_packets(const struct Packet *packets);
void observer_update_view(void);
void observer_set_view_mode(unsigned char mode);
const struct Packet *observer_get_view_packet(void);
void observer_update_cursor(void);
TbBool gameplay_cursor_is_over_gui(void);
TbBool get_gameplay_cursor_position(int32_t *screen_x, int32_t *screen_y);
void observer_cycle_player(int step);

#ifdef __cplusplus
}
#endif

#endif
