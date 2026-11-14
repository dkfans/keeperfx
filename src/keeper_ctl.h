/**
 * @file keeper_ctl.h
 * @brief The keeper controller: turns a verb (keeper_ctl_types.h) into the packets a human's input would produce, one
 *        packet per simulated turn, for any packet author -- an External seat (external_seat.h) or a Daimon keeper
 *        (docs/plans/ai-plans/v2/01-phase0-foundations.md section 4).
 *
 * State is per packet slot (0..PACKETS_COUNT-1; slot 0, the local human's, only when self-contained -- Daimon's
 * assistant): the running gesture's steps, three lanes of waiting verbs (now, routine, and the lieutenant's below them),
 * recent results, and the creatures a move_creature or attack order holds (the safety net that hands them back). A
 * slot's owner calls kctl_sweep() and then kctl_tick() once per simulated turn; kctl_tick() writes at most one step into
 * the packet it is given. An External seat's state is session state; a Daimon slot's is saved with Daimon
 * (kctl_serialise).
 */
#ifndef DK_KEEPER_CTL_H
#define DK_KEEPER_CTL_H

#include "globals.h"
#include "daimon_base_types.h"
#include "packets.h"
#include "keeper_ctl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Validates `verb` for `plyr_idx` against the live state and queues it on `slot` (see extseat_submit_verb_ex for
 *  queue, lanes and the error codes). NULL on success; "NOT_A_VALID_SLOT" for a slot out of range. */
const char *kctl_submit(int64_t slot, PlayerNumber plyr_idx, const KeeperVerb *verb, TbBool queue, struct ExtSeatSubmitInfo *info);
/** kctl_submit into a lane of its own choosing: EXTSEAT_LANE_LIEUTENANT for a seat's Daimon lieutenant (P8.2). */
const char *kctl_submit_lane(int64_t slot, PlayerNumber plyr_idx, const KeeperVerb *verb, TbBool queue, int64_t lane,
    struct ExtSeatSubmitInfo *info);
/** kctl_check for a lane (the lieutenant's plans with its own world). */
const char *kctl_check_lane(int64_t slot, PlayerNumber plyr_idx, const KeeperVerb *verb, int64_t lane, int64_t *out_steps);
/** The lieutenant lane's planning on a seat's slot: its world (nullptr: the slot's) and its group orders. Reset clears it. */
void kctl_set_lieutenant_planning(int64_t slot, const struct KeeperCtlWorld *world, TbBool group_orders);
/** This turn's tick wrote a step of one of `lane`'s verbs into the slot's packet. */
TbBool kctl_lane_wrote_this_turn(int64_t slot, int64_t lane);
/** A lane's verbs waiting, or its verb running. */
TbBool kctl_lane_busy(int64_t slot, int64_t lane);
/** One lane's waiting verbs and its running verb cancelled (a lieutenant stopping leaves the agent's alone). */
int64_t kctl_cancel_lane_and_running(int64_t slot, int64_t lane);
/** kctl_submit as a dry run: validates and counts steps, queues nothing. */
const char *kctl_check(int64_t slot, PlayerNumber plyr_idx, const KeeperVerb *verb, int64_t *out_steps);
/** The ordered-creature safety net for `slot`, once per turn before kctl_tick. */
void kctl_sweep(int64_t slot);
/** Once per simulated turn: cut a routine verb for an urgent one, start the next waiting verb, and write the running
 *  gesture's next step into `out` (unless a step waits for the hand or a room drag). */
void kctl_tick(int64_t slot, PlayerNumber plyr, struct Packet *out);
/** Forgets everything the slot holds, its world included. */
void kctl_reset(int64_t slot);
/** The slot's queue for a Daimon saved game (docs/plans/ai-plans/v2/01 section 9.1): the gesture in progress (its
 *  remaining steps and its verb), the recent results and the id counter, field by field. Writes at most `cap` bytes to
 *  `out` (NULL: none) and returns the size the whole image needs; 0 when the slot holds what this format does not
 *  keep -- verbs waiting in a lane, creatures held by an order, a set_assist verb -- which a load then drops. */
size_t kctl_serialise(int64_t slot, uint8_t *out, size_t cap);
/** Puts a kctl_serialise image back into the slot (its world is kept). Every count, length and enum is checked as
 *  untrusted input; false, and the slot left empty, on anything malformed. */
TbBool kctl_deserialise(int64_t slot, const uint8_t *in, size_t len);
/** One verb in the saved queue's own format, with its cancel fields: a gesture planned and not yet submitted, in a Daimon
 *  saved game (docs/plans/ai-plans/v2/12 section 13.5). Writes at most `cap` bytes to `out` (NULL: none) and returns the
 *  size it needs; 0 for a set_assist verb (its rule set is never written). */
size_t kctl_verb_serialise(const KeeperVerb *v, uint8_t *out, size_t cap);
/** Reads a kctl_verb_serialise image, checked as untrusted input; false on anything malformed or left over. */
TbBool kctl_verb_deserialise(KeeperVerb *v, const uint8_t *in, size_t len);
/** Who answers the slot's planning questions (keeper_ctl_types.h); NULL = the default world. The struct must outlive
 *  its use; kctl_reset puts the default back. */
void kctl_set_world(int64_t slot, const struct KeeperCtlWorld *world);
/** Whether the slot's move_creature, attack and release_creature orders are the group packets (keeper_group.h) rather
 *  than Order creature mode, a cheat state: Daimon's slots, in any game. kctl_reset clears it. */
void kctl_set_group_orders(int64_t slot, TbBool on);
/** Whether the controller plays placing, building, digging, selling and paying as the self-contained gesture packets
 *  (PckA_Dai*, docs/plans/ai-plans/v2/09 section 5.2: no tool switch, cursor or drag; the default) or as the clicks a
 *  human's mouse makes (for the parity tests). Every slot alike. */
void kctl_set_gesture_packets(TbBool on);
/** The slot writes into a human's own packet (their Daimon assistant, docs/plans/ai-plans/v2/09 section 5.2): a verb
 *  whose steps would switch the tool, move the cursor or click is refused, NOT_SELF_CONTAINED. */
void kctl_set_self_contained(int64_t slot, TbBool on);
TbBool kctl_gesture_packets(void);
/** The game's own answers, as the External seat uses them. */
const struct KeeperCtlWorld *kctl_default_world(void);

TbBool  kctl_idle(int64_t slot);
int64_t kctl_queued_verbs(int64_t slot);
int64_t kctl_queued_steps(int64_t slot);
TbBool  kctl_mid_gesture(int64_t slot);
TbBool  kctl_enqueue(int64_t slot, const KeeperStep *steps, int64_t count);
int64_t kctl_cancel(int64_t slot);
TbBool  kctl_cancel_id(int64_t slot, int64_t id);
int64_t kctl_cancel_lane(int64_t slot, int64_t lane);
void    kctl_queue_info(int64_t slot, struct ExtSeatQueueInfo *out);
int64_t kctl_results(int64_t slot, KeeperCtlResult *out, int64_t max);
TbBool  kctl_assist_settling(int64_t slot);
TbBool  kctl_verb_cuttable(enum ExtSeatVerbKind kind);
/** A verb by its agent-API name (any case), ESV_None for none; and its name (NULL: not an agent's verb). */
enum ExtSeatVerbKind kctl_verb_from_name(const char *name);
const char *kctl_verb_name(enum ExtSeatVerbKind kind);
/** A lane's name in the agent API ("now", "routine", "lieutenant"; NULL out of range), and the lane of a name (any
 *  case; -1 for none). */
const char *kctl_lane_name(int64_t lane);
int64_t kctl_lane_from_name(const char *name);
int64_t kctl_pending_dig_rects(int64_t slot, int64_t *out4, int64_t max);
int64_t kctl_ordered_count(int64_t slot);
TbBool  kctl_creature_ordered(int64_t slot, DaiThingIndex idx);
void    kctl_release_ordered_now(int64_t slot, const char *reason);
int64_t kctl_auto_releases(int64_t slot, struct ExtSeatAutoRelease *out, int64_t max);
/** The message a send_message verb left for the player's PckA_PlyrMsgEnd packet, copied into out and taken. */
void    kctl_take_pending_chat(PlayerNumber plyr_idx, char *out, size_t out_size);

/* Planning helpers (pure on the game state; the extseat_* names forward to them). */
int64_t kctl_plan_dig_sweep(KeeperStep *out, int64_t max, int64_t x0, int64_t y0, int64_t x1, int64_t y1);
int64_t kctl_plan_slab_clicks(KeeperStep *out, int64_t max, const int64_t *slab_xy, int64_t count);
int64_t kctl_room_build_check(PlayerNumber plyr_idx, RoomKind rkind, int64_t x0, int64_t y0, int64_t x1, int64_t y1,
    int *corner_out, int64_t *bad_xy, int64_t max_bad, int64_t *bad_count);
TbBool  kctl_subtiles_connected(int64_t ax, int64_t ay, int64_t bx, int64_t by);
const char *kctl_find_drop_spot(PlayerNumber plyr_idx, int64_t thing_id, int64_t x, int64_t y, int64_t radius,
    int64_t *out_x, int64_t *out_y);
const char *kctl_find_drop_spot_reaching(PlayerNumber plyr_idx, int64_t thing_id, int64_t x, int64_t y, int64_t radius,
    int64_t reach_x, int64_t reach_y, int64_t *out_x, int64_t *out_y);

#ifdef __cplusplus
}
#endif

#endif
