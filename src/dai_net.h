/******************************************************************************/
/** @file dai_net.h
 *     Daimon in a network game (docs/plans/ai-plans/v2/01-phase0-foundations.md section 10).
 * @par Purpose:
 *     The host writes every Daimon keeper's packet slot (past the network users) and sends it to the clients with its
 *     own frame; clients wait for those slots as for a peer. And the host keeps the clients supplied with a recent
 *     copy of Daimon's state (daimon.bin, compressed and split into NETMSG_DAIMON_STATE parts), which a client that
 *     loses the host takes its Daimon keepers over with.
 */
/******************************************************************************/
#ifndef DK_DAI_NET_H
#define DK_DAI_NET_H

#include "globals.h"
#include "daimon_base_types.h"
#include "bflib_basics.h"
#include "net_main.h"

struct Packet;

#ifdef __cplusplus
extern "C" {
#endif

/** The largest Daimon state a client keeps (uncompressed), and the bytes of one NETMSG_DAIMON_STATE part. */
#define DAI_NET_MAX_STATE (1 << 20)
#define DAI_NET_PART_BYTES 4000
#define DAI_NET_MAX_PARTS ((DAI_NET_MAX_STATE + DAI_NET_PART_BYTES - 1) / DAI_NET_PART_BYTES)
/** Complete snapshots a client keeps, newest first: a bad newest one falls back to the one before. */
#define DAI_NET_SNAPSHOTS 2

/** In a network game, whether `slot` is a Daimon keeper's, written by the host (clients wait for it). */
TbBool dai_net_slot_expected(NetUserId slot);
/** Host, once per turn in exchange_packets(): each Daimon keeper's controller writes this turn's step into its slot's
 *  packet, which is stored in the slot's history (with the turn and the host's checksum, as the host's own) and sent. */
void dai_net_host_author_slots(void);

/** Host: whether the human keepers of the next network game are Daimon's to adopt if their human leaves (the start-up
 *  sync marks them PlaF_DaimonAdopt on every machine). The network lobby's AI-keepers setting (Phase 2, 04 section
 *  10.2) will set it; off until then, so a network game plays as before unless a host (a test) turns it on. */
void dai_net_set_adoption(TbBool on);
TbBool dai_net_adoption(void);
/** Host: the network lobby's "AI keepers" setting (04 section 10.2): Daimon plays every computer keeper of the next
 *  game (queued for the level start with the start-up sync), with this personality ("" the default); off, Computer2
 *  does. Off until a host's lobby turns it on. */
void dai_net_set_opponents(TbBool on, const char *personality);
TbBool dai_net_opponents(void);
const char *dai_net_opponents_personality(void);
/** Host: the lobby's one difficulty for those keepers (05 section 8: "easy", "normal", "hard" or daimon.toml's own; ""
 *  normal). Only the host's Daimon plays them, so the clients need no copy. */
void dai_net_set_opponents_difficulty(const char *difficulty);
const char *dai_net_opponents_difficulty(void);
/** The lobby: the host's two settings as the high byte of its frame's stored_data2 (bit 0 Daimon opponents, bit 1
 *  Daimon adopts), and a client's copy of them. */
/** The host's "Daimon assistants" lobby setting (co-op fairness): whether a player may start one in a network game. */
void dai_net_set_assistants_allowed(TbBool on);
TbBool dai_net_assistants_allowed(void);
uint16_t dai_net_lobby_bits(void);
void dai_net_read_lobby_bits(uint16_t bits);
/** Host, building the start-up sync: with Daimon opponents and computer keepers on, every keeper slot but the humans'
 *  (`human_slots`, 1 << player) is queued for Daimon (daimon_pending_slots_add); a slot without a keeper is skipped at
 *  the claim. */
void dai_net_queue_opponents(uint16_t human_slots);
/** Host: the keeper of a human who left (marked PlaF_DaimonAdopt), now the built-in AI's, is to be adopted. */
void dai_net_request_adoption(PlayerNumber plyr_idx);
/** Host: a queued adoption of the keeper is dropped (its human took it back: docs/plans/mp-rejoin.md). */
void dai_net_cancel_adoption(PlayerNumber plyr_idx);
/** The authoring machine's Daimon: a keeper it took with rich start or stronger creatures (DAI_CHEAT_*'s simulation
 *  flags: 2 rich start, 4 stronger creatures) gets them once, at the take: the host's PckA_DaimonCheats, with the
 *  host's amounts (gold; creatures' health, strength and armour up, pay down, in percent). */
void dai_net_request_cheats(PlayerNumber plyr_idx, uint64_t flags, int64_t gold, int64_t bonus_pct, int64_t discount_pct);
/** Host, in exchange_packets(): an adoption or a keeper's cheats waiting goes into the host's own packet, when it has
 *  no action of its own this turn (as the timestamp does), so every machine applies it the same turn and a replay
 *  records it -- also once the game has gone local (the client that left was the last). */
void dai_net_set_pending_host_action(struct Packet *pckt);

/** Host: sends Daimon's state taken at `turn` to every client (compressed, in parts, reliable). */
void dai_net_send_state(DaiTurn turn, const uint8_t *blob, size_t len);
/** NETMSG_DAIMON_STATE from `source` (after the type byte): only the host's, only on a client. */
TbError dai_net_process_state_message(NetUserId source, const char *buf, size_t len);
/** One part as it came off the wire, without the source checks (tests): false if it is malformed. A part of a newer
 *  turn replaces an incomplete older one; an older turn's and a duplicate are dropped; the last part decompresses the
 *  whole into the newest snapshot. */
TbBool dai_net_state_part(const uint8_t *payload, size_t len);
/** The parts `blob` is sent as: `part(ctx, payload, len)` for each, in order. False if it cannot be sent. */
TbBool dai_net_state_parts(DaiTurn turn, const uint8_t *blob, size_t len, void (*part)(void *ctx, const uint8_t *payload, size_t len), void *ctx);
/** Complete snapshot `i` (0: the newest): its bytes and the turn it was taken at; false if there is none. */
TbBool dai_net_snapshot(int64_t i, const uint8_t **data, size_t *len, DaiTurn *turn);
/** Forgets every snapshot and part (a new game, a load). */
void dai_net_reset(void);

#ifdef __cplusplus
}
#endif

#endif
