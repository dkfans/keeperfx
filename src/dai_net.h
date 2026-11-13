/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dai_net.h
 *     Daimon and the host: the lobby's choice of Daimon opponents, a keeper's difficulty cheats and a dropped
 *     player's adoption, sent once in the host's own packet (dai_net.c). Network play itself (the host authoring the
 *     Daimon slots for the clients, replicating Daimon's state) is not in this build yet: those do nothing here.
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

/** The lobby's choice: the AI keepers are Daimon's, with this personality and difficulty ("" for the default). */
void dai_net_set_opponents(TbBool on, const char *personality);
TbBool dai_net_opponents(void);
const char *dai_net_opponents_personality(void);
void dai_net_set_opponents_difficulty(const char *difficulty);
const char *dai_net_opponents_difficulty(void);
/** Queues every keeper but the humans' (a bit per player) for Daimon at the level start, if the lobby chose it. */
void dai_net_queue_opponents(uint16_t human_slots);
void dai_net_set_assistants_allowed(TbBool on);
TbBool dai_net_assistants_allowed(void);
uint16_t dai_net_lobby_bits(void);
void dai_net_read_lobby_bits(uint16_t bits);

/** A dropped player's keeper adopted by Daimon (network games). */
void dai_net_set_adoption(TbBool on);
TbBool dai_net_adoption(void);
void dai_net_request_adoption(PlayerNumber plyr_idx);
void dai_net_cancel_adoption(PlayerNumber plyr_idx);
/** A Daimon keeper's difficulty cheats, applied once on every machine (PckA_DaimonCheats in the host's packet). */
void dai_net_request_cheats(PlayerNumber plyr_idx, uint64_t flags, int64_t gold, int64_t bonus_pct, int64_t discount_pct);
/** The host's own packet, after its input: the first waiting request, if the packet has no action of its own. */
void dai_net_set_pending_host_action(struct Packet *pckt);

static inline TbBool dai_net_slot_expected(NetUserId slot) { (void)slot; return false; }
static inline void dai_net_host_author_slots(void) {}
static inline void dai_net_send_state(DaiTurn turn, const uint8_t *blob, size_t len) { (void)turn; (void)blob; (void)len; }
static inline TbBool dai_net_snapshot(int64_t i, const uint8_t **data, size_t *len, DaiTurn *turn)
    { (void)i; (void)data; (void)len; (void)turn; return false; }
static inline void dai_net_reset(void) {}

#ifdef __cplusplus
}
#endif
#endif
