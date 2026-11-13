/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_slots.h
 *     The packet slots Daimon's keepers are played from.
 * @par Purpose:
 *     A keeper Daimon plays writes its packets into a slot of its own (MAX_NET_USERS..PACKETS_COUNT-1), with that
 *     slot's user state, as a human's client writes theirs.
 */
/******************************************************************************/
#ifndef DK_DAIMON_SLOTS_H
#define DK_DAIMON_SLOTS_H

#include "globals.h"
#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

/** Writes a Daimon keeper's packet for this turn into its slot's packet (cleared before the call). */
typedef void (*DaimonSlotWriter)(NetUserId slot, PlayerNumber plyr_idx, struct Packet *pckt);

/** The slot a keeper is played from, or -1. */
NetUserId daimon_slot_of(PlayerNumber plyr_idx);
/** Moves a computer keeper to a free slot, Computer2 parked; its previous model in *prev_model. -1 if it can't. */
NetUserId daimon_assign_slot(PlayerNumber plyr_idx, int32_t *prev_model);
/** Hands a Daimon keeper back to Computer2 with the given model and frees its slot. */
TbBool daimon_release_slot(PlayerNumber plyr_idx, int32_t computer_model);
/** Frees a keeper's slot when something else takes the keeper over (a script, a human), leaving its flags to it. */
void daimon_forget_slot(PlayerNumber plyr_idx);

/** The keepers to give slots at the level start, claimed by daimon_claim_pending_slots(). */
void daimon_pending_slots_clear(void);
void daimon_pending_slots_add(PlayerNumber plyr_idx);
uint16_t daimon_pending_slots_mask(void);
void daimon_pending_slots_set_mask(uint16_t mask);
int32_t daimon_claim_pending_slots(void);

/** Sets the function that writes the keepers' packets (Daimon's), NULL for none. */
void daimon_set_slot_writer(DaimonSlotWriter writer);
/** Once a turn, after input and before the packet exchange: each Daimon keeper's slot packet written afresh. */
void daimon_write_slot_packets(void);

#ifdef __cplusplus
}
#endif
#endif
