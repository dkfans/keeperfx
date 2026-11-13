/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dai_net.h
 *     Daimon in network games: not in this build yet. Every machine runs a local game's Daimon only; these do
 *     nothing, so Daimon's keepers are not offered in a network game.
 */
/******************************************************************************/
#ifndef DK_DAI_NET_H
#define DK_DAI_NET_H

#include "globals.h"
#include "bflib_basics.h"
#include "net_main.h"
#include "daimon_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

#define DAI_NET_SNAPSHOTS 2

static inline TbBool dai_net_slot_expected(NetUserId slot) { (void)slot; return false; }
static inline void dai_net_host_author_slots(void) {}
static inline void dai_net_set_adoption(TbBool on) { (void)on; }
static inline TbBool dai_net_adoption(void) { return false; }
static inline void dai_net_set_opponents(TbBool on, const char *personality) { (void)on; (void)personality; }
static inline TbBool dai_net_opponents(void) { return false; }
static inline const char *dai_net_opponents_personality(void) { return ""; }
static inline void dai_net_set_opponents_difficulty(const char *difficulty) { (void)difficulty; }
static inline const char *dai_net_opponents_difficulty(void) { return ""; }
static inline void dai_net_queue_opponents(uint16_t human_slots) { (void)human_slots; }
static inline void dai_net_request_adoption(PlayerNumber plyr_idx) { (void)plyr_idx; }
static inline void dai_net_cancel_adoption(PlayerNumber plyr_idx) { (void)plyr_idx; }
static inline void dai_net_request_cheats(PlayerNumber plyr_idx, uint64_t flags, int64_t gold, int64_t bonus_pct, int64_t discount_pct)
    { (void)plyr_idx; (void)flags; (void)gold; (void)bonus_pct; (void)discount_pct; }
static inline void dai_net_set_pending_host_action(struct Packet *pckt) { (void)pckt; }
static inline void dai_net_send_state(DaiTurn turn, const uint8_t *blob, size_t len) { (void)turn; (void)blob; (void)len; }
static inline TbBool dai_net_snapshot(int64_t i, const uint8_t **data, size_t *len, DaiTurn *turn)
    { (void)i; (void)data; (void)len; (void)turn; return false; }
static inline void dai_net_reset(void) {}

#ifdef __cplusplus
}
#endif
#endif
