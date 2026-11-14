/******************************************************************************/
/** @file keeper_ctl.c
 *     The keeper controller: verbs into the packets a human's input would produce, per packet slot.
 * @par Purpose:
 *     See keeper_ctl.h. Moved out of external_seat.c (docs/plans/ai-plans/v2/01-phase0-foundations.md P0.2) so the
 *     External seat and Daimon drive keepers through the same validation and packet steps.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "keeper_ctl.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_magic.h"
#include "config_players.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_control.h"
#include "creature_jobs.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "dungeon_data.h"
#include "game_legacy.h"
#include "magic_powers.h"
#include "net_game.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "creature_states_tresr.h"
#include "power_hand.h"
#include "map_columns.h"
#include "room_data.h"
#include "roomspace.h"
#include "thing_objects.h"
#include "slab_data.h"
#include "thing_data.h"
#include "thing_list.h"
#include "config_keeperfx.h"
#include "thing_doors.h"
#include "power_specials.h"
#include "power_process.h"
#include "thing_traps.h"
#include "thing_creature.h"
#include "map_data.h"
#include "thing_doors.h"
#include "thing_navigate.h"
#include "ariadne.h"
#include "thing_physics.h"
#include "thing_stats.h"
#include "daimon_engine.h"
#include "daimon_slots.h"
#include "keeper_assist_state.h"
#include "keeper_exclusion.h"
#include "keeper_group.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct UserState *plan_ustate(void);

/** Slaps are ignored for this many turns after a creature was dropped (magic_use_power_slap_thing). */
#define SLAP_AFTER_DROP_TURNS 10

struct OrderedCreature {
    DaiThingIndex idx;
    DaiTurn creation_turn;
    DaiTurn ordered_turn;
    DaiTurn hold_until;
    TbBool releasing; /**< release_creature is running: hold_until is when the safety net frees it if the clicks did not */
    TbBool attack;    /**< an attack order: on arrival at the goal it is set fighting there instead of held */
    int64_t verb_id;  /**< the order that holds it, while its steps are being written (ExtSeatQueue.cur_id) */
    DaiSubtlCoord goal_x, goal_y;
};

/** release_creature's clicks take about ten turns; a creature still held this long after they started is freed directly. */
#define EXTSEAT_RELEASE_GRACE_TURNS 20
/** attack: how near an enemy heart (from its centre, subtiles; the heart's room reaches 7) and enemy creatures must be to
 *  the spot for the order to have something to fight there. */

/** A verb waiting in a lane: as submitted, or cut part-way for an urgent verb (suspended), to be planned again. */
struct QueuedVerb {
    struct ExtSeatVerb verb;
    int64_t id;
    int64_t submitted;
    int64_t started;    /**< -1 until it first ran */
    int64_t suspended;  /**< times it was cut */
};

struct ExtSeatQueue {
    struct ExtSeatStep steps[EXTSEAT_MAX_STEPS];
    int64_t count;
    int64_t next;
    TbBool button_down; /**< a written step pressed the button and none has released it yet */
    int64_t cur_id;     /**< id of the verb whose steps are in `steps` (0 = none, e.g. raw steps) */
    struct ExtSeatVerb cur_verb; /**< that verb, while cur_id != 0 */
    int64_t cur_submitted, cur_started; /**< when that verb was accepted, and when it started */
    int64_t cur_lane;       /**< the lane it came from */
    int64_t cur_suspended;  /**< times it was cut before (it resumed from its lane) */
    const char *cur_error; /**< why that verb failed part-way (a step's precondition), recorded as its result; NULL = none */
    int64_t step_wait;     /**< turns the next step has been held back waiting for its precondition */
    int64_t last_chat_turn; /**< send_message: when the seat last spoke (0 = never) */
    uint64_t hand_dropped; /**< pick_up_and_drop: which of the running verb's things (by index in its list) were dropped */
    char cur_error_buf[28];
    int64_t assist_written;  /**< 1 + the turn the seat last wrote a combat assistant packet (PckA_Assist*); 0 = never */
    struct QueuedVerb lane[EXTSEAT_LANES][EXTSEAT_MAX_QUEUED_VERBS]; /**< oldest first */
    int64_t lane_count[EXTSEAT_LANES];
    int64_t next_id;
    struct ExtSeatResult results[EXTSEAT_RESULT_RING];
    int64_t results_total;
    struct OrderedCreature ordered[EXTSEAT_MAX_ORDERED];
    int64_t ordered_count;
    struct ExtSeatAutoRelease auto_rel[EXTSEAT_AUTO_RELEASE_RING];
    int64_t auto_total;
};

static struct ExtSeatQueue s_queue[PACKETS_COUNT];
/** Each slot's answers to the planning's map questions (NULL = s_default_world). */
static const struct KeeperCtlWorld *s_world[PACKETS_COUNT];
/** Slots whose move, attack and release orders are the group packets, not Order creature mode (kctl_set_group_orders). */
static TbBool s_group_orders[PACKETS_COUNT];
/* The lieutenant lane's own planning (P8.2): a seat's agent plans with the slot's world and orders; its Daimon
 * lieutenant with its knowledge (principle 8: never the game's navigation) and group orders. */
static const struct KeeperCtlWorld *s_lt_world[PACKETS_COUNT];
/* The turn (+1) a tick last wrote a step into the slot's packet, and the lane of that step's verb (P8.3: the touch log
 * tells the lieutenant's gestures from the agent's). */
static int64_t s_wrote_turn[PACKETS_COUNT];
static int64_t s_wrote_lane[PACKETS_COUNT];
static TbBool s_lt_group_orders[PACKETS_COUNT];
/** The gestures as PckA_Dai* packets (kctl_set_gesture_packets). */
static TbBool s_gesture_packets = true;
/** Slots that write into a human's own packet (the human's Daimon assistant, kctl_set_self_contained): only verbs
 *  whose steps neither switch the tool nor move the cursor nor click. */
static TbBool s_self_contained[PACKETS_COUNT];
/* The slot being planned for is self-contained (plan_verb_for): its hand picks and casts on a thing go as the
 * PckA_DaiHandPick / PckA_DaiCastOn gestures, refused on what the keeper's human is doing whoever sends them. */
static TbBool s_plan_self_contained = false;

void kctl_set_self_contained(int64_t slot, TbBool on)
{
    if ((slot >= 0) && (slot < PACKETS_COUNT)) s_self_contained[slot] = on;
}

/** A plan for a self-contained slot: nothing of the human's hand -- no tool switch, cursor or click. */
static const char *self_contained_check(int64_t slot, const struct ExtSeatStep *steps, int64_t n)
{
    if ((slot < 0) || (slot >= PACKETS_COUNT) || !s_self_contained[slot]) return NULL;
    for (int64_t i = 0; i < n; i++) {
        if ((steps[i].action == PckA_SetPlyrState) || steps[i].has_pos || (steps[i].control_flags != 0) || (steps[i].context != 0))
            return "NOT_SELF_CONTAINED";
    }
    return NULL;
}

void kctl_set_gesture_packets(TbBool on) { s_gesture_packets = on; }
TbBool kctl_gesture_packets(void) { return s_gesture_packets; }


/* The solo human's own slot only for a self-contained slot (the assistant, 09 section 13.4): its gestures write
 * nothing of the human's hand. */
static TbBool slot_ok(int64_t slot)
{
    if ((slot < 0) || (slot >= PACKETS_COUNT)) return false;
    return (slot > SOLO_HUMAN_ID) || s_self_contained[slot];
}

static void promote_verbs(int64_t slot, PlayerNumber plyr);

static TbBool steps_done(int64_t slot)
{
    return slot_ok(slot) && (s_queue[slot].next >= s_queue[slot].count);
}

static int64_t queued_total(const struct ExtSeatQueue *q)
{
    return q->lane_count[EXTSEAT_LANE_NOW] + q->lane_count[EXTSEAT_LANE_ROUTINE] + q->lane_count[EXTSEAT_LANE_LIEUTENANT];
}

/** The agent's own verbs (the urgent and routine lanes) waiting or running: what makes the slot busy for the agent. */
static TbBool agent_busy(const struct ExtSeatQueue *q)
{
    if ((q->lane_count[EXTSEAT_LANE_NOW] + q->lane_count[EXTSEAT_LANE_ROUTINE]) > 0) return true;
    return (q->next < q->count) && !((q->cur_id != 0) && (q->cur_lane == EXTSEAT_LANE_LIEUTENANT));
}

TbBool kctl_lane_busy(int64_t slot, int64_t lane)
{
    if (!slot_ok(slot) || (lane < 0) || (lane >= EXTSEAT_LANES)) return false;
    const struct ExtSeatQueue *q = &s_queue[slot];
    if (q->lane_count[lane] > 0) return true;
    return (q->next < q->count) && (q->cur_id != 0) && (q->cur_lane == lane);
}

int64_t kctl_cancel_lane_and_running(int64_t slot, int64_t lane)
{
    if (!slot_ok(slot) || (lane < 0) || (lane >= EXTSEAT_LANES)) return 0;
    struct ExtSeatQueue *q = &s_queue[slot];
    int64_t n = kctl_cancel_lane(slot, lane);
    if ((q->cur_id != 0) && (q->cur_lane == lane) && (q->next < q->count) && kctl_cancel_id(slot, q->cur_id)) n++;
    return n;
}

TbBool kctl_idle(int64_t slot)
{
    return steps_done(slot) && (queued_total(&s_queue[slot]) == 0);
}

int64_t kctl_queued_verbs(int64_t slot)
{
    return slot_ok(slot) ? queued_total(&s_queue[slot]) : 0;
}

static void record_result_ex(int64_t slot, int64_t id, const char *error, int64_t submitted, int64_t started,
    int64_t lane, int64_t suspended)
{
    struct ExtSeatQueue *q = &s_queue[slot];
    struct ExtSeatResult *r = &q->results[q->results_total % EXTSEAT_RESULT_RING];
    memset(r, 0, sizeof(*r));
    r->id = id;
    r->rejected = (error != NULL);
    if (error != NULL) {
        snprintf(r->error, sizeof(r->error), "%s", error);
    }
    r->turn = (int64_t)get_gameturn();
    r->submitted = submitted;
    r->started = started;
    r->lane = lane;
    r->suspended = suspended;
    q->results_total++;
}

static void record_queued(int64_t slot, int64_t lane, const struct QueuedVerb *e, const char *error)
{
    record_result_ex(slot, e->id, error, e->submitted, e->started, lane, e->suspended);
}

static void record_running(int64_t slot, const char *error)
{
    const struct ExtSeatQueue *q = &s_queue[slot];
    record_result_ex(slot, q->cur_id, error, q->cur_submitted, q->cur_started, q->cur_lane, q->cur_suspended);
}

static TbBool lane_push(struct ExtSeatQueue *q, int64_t lane, const struct QueuedVerb *e, TbBool front)
{
    if (q->lane_count[lane] >= EXTSEAT_MAX_QUEUED_VERBS) return false;
    struct QueuedVerb *l = q->lane[lane];
    if (front) {
        memmove(&l[1], &l[0], sizeof(l[0]) * (size_t)q->lane_count[lane]);
        l[0] = *e;
    } else {
        l[q->lane_count[lane]] = *e;
    }
    q->lane_count[lane]++;
    return true;
}

static void lane_remove(struct ExtSeatQueue *q, int64_t lane, int64_t i)
{
    struct QueuedVerb *l = q->lane[lane];
    memmove(&l[i], &l[i + 1], sizeof(l[0]) * (size_t)(q->lane_count[lane] - i - 1));
    q->lane_count[lane]--;
}

/** Every verb (14 S2): its name in the agent API (NULL: not an agent's -- the combat assistant's own and Daimon's
 *  delegation), whether a newer verb may cut it, and whether it is planned against the combat assistant's state. */
struct KctlVerbSpec {
    const char *name;
    TbBool cuttable;    /**< its plan, made again from the live state, does exactly what is left: a room build skips the
                         *   slabs already built, a dig starts on an unmarked corner (or taps the unmarked slabs), an
                         *   unmark or a sell takes what is still there, a rule change stages against the rules live now
                         *   (a half-staged set is never live). The hand verbs, a cast, an order to a creature and the short
                         *   one-click verbs run to their end. */
    TbBool assist;      /**< planned against the combat assistant's state (assist_verb) */
};
static const struct KctlVerbSpec verb_specs[ESV_COUNT] = {
    [ESV_None]            = { NULL, false, false },
    [ESV_PlaceTrap]       = { "place_trap", false, false },
    [ESV_PlaceDoor]       = { "place_door", false, false },
    [ESV_Slap]            = { "slap", false, false },
    [ESV_PickUp]          = { "pick_up", false, false },
    [ESV_Drop]            = { "drop", false, false },
    [ESV_CastPower]       = { "cast_power", false, false },
    [ESV_BuildRoom]       = { "build_room", true, false },
    [ESV_MarkDig]         = { "mark_dig", true, false },
    [ESV_Sell]            = { "sell", true, false },
    [ESV_Cancel]          = { "cancel", false, false },
    [ESV_MoveCreature]    = { "move_creature", false, false },
    [ESV_SetTendency]     = { "set_tendency", false, false },
    [ESV_ReleaseCreature] = { "release_creature", false, false },
    [ESV_SetAlliance]     = { "set_alliance", false, false },
    [ESV_PickUpAndDrop]   = { "pick_up_and_drop", false, false },
    [ESV_UnmarkDig]       = { "unmark_dig", true, false },
    [ESV_SetDoorLock]     = { "set_door_lock", false, false },
    [ESV_PowerOff]        = { "power_off", false, false },
    [ESV_UseSpecial]      = { "use_special", false, false },
    [ESV_SendMessage]     = { "send_message", false, false },
    [ESV_SetAssist]       = { NULL, true, true },
    [ESV_AssistEnable]    = { NULL, false, true },
    [ESV_Attack]          = { "attack", false, false },
    [ESV_AssistUseBank]   = { NULL, false, true },
    [ESV_GiveGold]        = { "give_gold", false, false },
    [ESV_AssistDelegate]  = { NULL, false, false },
};

TbBool kctl_verb_cuttable(enum ExtSeatVerbKind kind)
{
    return ((kind > ESV_None) && (kind < ESV_COUNT)) ? verb_specs[kind].cuttable : false;
}

enum ExtSeatVerbKind kctl_verb_from_name(const char *name)
{
    if (name == NULL) return ESV_None;
    for (int k = ESV_None + 1; k < ESV_COUNT; k++)
        if ((verb_specs[k].name != NULL) && (strcasecmp(verb_specs[k].name, name) == 0)) return (enum ExtSeatVerbKind)k;
    return ESV_None;
}

static const char *const lane_names[EXTSEAT_LANES] = { "now", "routine", "lieutenant" };

const char *kctl_lane_name(int64_t lane)
{
    return ((lane >= 0) && (lane < EXTSEAT_LANES)) ? lane_names[lane] : NULL;
}

int64_t kctl_lane_from_name(const char *name)
{
    if (name == NULL) return -1;
    for (int64_t l = 0; l < EXTSEAT_LANES; l++) if (strcasecmp(lane_names[l], name) == 0) return l;
    return -1;
}

const char *kctl_verb_name(enum ExtSeatVerbKind kind)
{
    return ((kind > ESV_None) && (kind < ESV_COUNT)) ? verb_specs[kind].name : NULL;
}

static TbBool is_assist_action(unsigned char action)
{
    (void)action;
    return false; // the combat assistant is not in this build
}

/** A verb planned against the combat assistant's state (only what differs from it is sent) must not be planned before
 *  the packets the seat just wrote have been processed -- the turn after they are written. Planned the same turn a
 *  clear's commit was written, a new set took the rules about to go for live and left them out, so they were gone. */
static TbBool assist_verb(enum ExtSeatVerbKind kind)
{
    return ((kind > ESV_None) && (kind < ESV_COUNT)) ? verb_specs[kind].assist : false;
}

/** Whether the combat assistant's state is about to change for this seat: an assistant verb runs or waits, or the
 *  packets one just wrote are not processed yet. A change compared with the live set now (to plan it, or to answer
 *  "unchanged") would be compared with what is about to go. */
static TbBool assist_settling(const struct ExtSeatQueue *q)
{
    const int64_t now = (int64_t)get_gameturn();
    if ((q->assist_written > 0) && (now + 1 >= q->assist_written) && (now <= q->assist_written)) return true;
    if ((q->cur_id != 0) && (q->next < q->count) && assist_verb(q->cur_verb.kind)) return true;
    for (int lane = 0; lane < EXTSEAT_LANES; lane++) {
        for (int64_t i = 0; i < q->lane_count[lane]; i++) {
            if (assist_verb(q->lane[lane][i].verb.kind)) return true;
        }
    }
    return false;
}

TbBool kctl_assist_settling(int64_t slot)
{
    return slot_ok(slot) && assist_settling(&s_queue[slot]);
}

/** A cut verb that resumes and finds nothing left to do has done its job. */
static TbBool resume_nothing_left(const char *err)
{
    return (strcmp(err, "CANNOT_BUILD_HERE") == 0) || (strcmp(err, "NOTHING_MARKED") == 0)
        || (strcmp(err, "NOTHING_TO_SELL") == 0) || (strcmp(err, "UNCHANGED") == 0);
}

/** The steps that end a gesture cleanly where it stands: leave the tool (a state switch also drops a room drag still
 *  being laid), and release the button if it is down. */
static void end_gesture_here(struct ExtSeatQueue *q)
{
    q->count = q->next = 0;
    q->step_wait = 0;
    if (q->button_down) {
        struct ExtSeatStep end[2];
        memset(end, 0, sizeof(end));
        end[0].action = PckA_SetPlyrState;
        end[0].par1 = PSt_CtrlDungeon;
        end[1].has_pos = true;
        end[1].control_flags = PCtr_LBtnRelease;
        memcpy(q->steps, end, sizeof(end));
        q->count = 2;
    }
}

/** An urgent verb waits and a routine one that can be cut is running: cut it where it stands, and put it back at the
 *  front of its lane to be planned again once the urgent ones are done. */
static void preempt_for_urgent(int64_t slot)
{
    struct ExtSeatQueue *q = &s_queue[slot];
    if ((q->cur_id == 0) || (q->next >= q->count) || !kctl_verb_cuttable(q->cur_verb.kind)) return;
    // An urgent verb cuts a routine one; any agent verb cuts the lieutenant's (10 section 7.1).
    const TbBool cut_routine = (q->cur_lane == EXTSEAT_LANE_ROUTINE) && (q->lane_count[EXTSEAT_LANE_NOW] > 0);
    const TbBool cut_lieutenant = (q->cur_lane == EXTSEAT_LANE_LIEUTENANT)
        && ((q->lane_count[EXTSEAT_LANE_NOW] + q->lane_count[EXTSEAT_LANE_ROUTINE]) > 0);
    if (!cut_routine && !cut_lieutenant) return;
    struct QueuedVerb e;
    memset(&e, 0, sizeof(e));
    e.verb = q->cur_verb;
    e.id = q->cur_id;
    e.submitted = q->cur_submitted;
    e.started = q->cur_started;
    e.suspended = q->cur_suspended + 1;
    if (!lane_push(q, q->cur_lane, &e, true)) return; // no room to put it back: let it finish
    JUSTMSG("Keeper controller: slot %d's verb #%" PRId64 " cut at step %" PRId64 " of %" PRId64 " for %s",
            (int)slot, q->cur_id, q->next, q->count, cut_routine ? "an urgent one" : "the agent's");
    q->cur_id = 0;
    q->cur_error = NULL;
    q->hand_dropped = 0;
    end_gesture_here(q);
}

void kctl_queue_info(int64_t slot, struct ExtSeatQueueInfo *out)
{
    memset(out, 0, sizeof(*out));
    if (!slot_ok(slot)) return;
    const struct ExtSeatQueue *q = &s_queue[slot];
    if ((q->cur_id != 0) && (q->next < q->count)) {
        out->running_id = q->cur_id;
        out->running_lane = q->cur_lane;
        out->running_steps_left = q->count - q->next;
    }
    for (int64_t l = 0; l < EXTSEAT_LANES; l++) {
        out->lane_count[l] = q->lane_count[l];
        for (int64_t i = 0; i < q->lane_count[l]; i++) {
            out->lane_ids[l][i] = q->lane[l][i].id;
            out->lane_suspended[l][i] = (q->lane[l][i].suspended > 0);
        }
    }
}

int64_t kctl_cancel_lane(int64_t slot, int64_t lane)
{
    if (!slot_ok(slot) || (lane < 0) || (lane >= EXTSEAT_LANES)) return 0;
    struct ExtSeatQueue *q = &s_queue[slot];
    const int64_t n = q->lane_count[lane];
    for (int64_t i = 0; i < n; i++) record_queued(slot, lane, &q->lane[lane][i], "CANCELLED");
    q->lane_count[lane] = 0;
    return n;
}

TbBool kctl_cancel_id(int64_t slot, int64_t id)
{
    if (!slot_ok(slot) || (id <= 0)) return false;
    struct ExtSeatQueue *q = &s_queue[slot];
    for (int64_t l = 0; l < EXTSEAT_LANES; l++) {
        for (int64_t i = 0; i < q->lane_count[l]; i++) {
            if (q->lane[l][i].id != id) continue;
            record_queued(slot, l, &q->lane[l][i], "CANCELLED");
            lane_remove(q, l, i);
            return true;
        }
    }
    if ((q->cur_id == id) && (q->next < q->count)) {
        record_running(slot, "CANCELLED");
        q->cur_id = 0;
        q->cur_error = NULL;
        q->hand_dropped = 0;
        end_gesture_here(q);
        return true;
    }
    return false;
}

// ---- ordered creatures: the safety net ---------------------------------------------------------------------------------

// The order rules are the simulation's (keeper_group.h): a group's order ends by the same ones, on every machine.
static int64_t turns_to_payday(PlayerNumber plyr_idx) { return keeper_order_turns_to_payday(plyr_idx); }
static int64_t default_hold_turns(PlayerNumber plyr_idx) { return keeper_order_default_hold_turns(plyr_idx); }
static int64_t max_hold_turns(PlayerNumber plyr_idx) { return keeper_order_max_hold_turns(plyr_idx); }
static TbBool creature_near_hunger_limit(const struct Thing *thing) { return keeper_order_near_hunger_limit(thing); }
static TbBool creature_owed_pay(const struct Thing *thing) { return keeper_order_owed_pay(thing); }
static TbBool in_manual_control(const struct Thing *thing) { return keeper_order_in_manual_control(thing); }

static void record_auto_release(int64_t slot, DaiThingIndex idx, const char *reason)
{
    struct ExtSeatQueue *q = &s_queue[slot];
    struct ExtSeatAutoRelease *r = &q->auto_rel[q->auto_total % EXTSEAT_AUTO_RELEASE_RING];
    memset(r, 0, sizeof(*r));
    r->thing_id = idx;
    snprintf(r->reason, sizeof(r->reason), "%s", reason);
    r->turn = (int64_t)get_gameturn();
    q->auto_total++;
}

static void hand_back(int64_t slot, struct OrderedCreature *o, const char *reason)
{
    struct Thing *thing = thing_get(o->idx);
    if (!thing_is_invalid(thing) && thing_is_creature(thing) && (thing->creation_turn == o->creation_turn) && in_manual_control(thing)) {
        set_start_state(thing);
        record_auto_release(slot, o->idx, reason);
        WARNLOG("Ordered creature %" PRId64 " handed back to its own behaviour (%s)", (int64_t)o->idx, reason);
    }
}

static void drop_ordered(struct ExtSeatQueue *q, int64_t i)
{
    q->ordered[i] = q->ordered[--q->ordered_count];
}

int64_t kctl_auto_releases(int64_t slot, struct ExtSeatAutoRelease *out, int64_t max)
{
    if (!slot_ok(slot) || (out == NULL)) {
        return 0;
    }
    const struct ExtSeatQueue *q = &s_queue[slot];
    const int64_t have = (q->auto_total < EXTSEAT_AUTO_RELEASE_RING) ? q->auto_total : EXTSEAT_AUTO_RELEASE_RING;
    int64_t n = 0;
    for (int64_t i = q->auto_total - have; (i < q->auto_total) && (n < max); i++) {
        out[n++] = q->auto_rel[i % EXTSEAT_AUTO_RELEASE_RING];
    }
    return n;
}


/** Each player's message from a seat's send_message verb, until its PckA_PlyrMsgEnd packet takes it. */
static char s_pending_chat[PLAYERS_COUNT][PLAYER_MP_MESSAGE_LEN];

void kctl_take_pending_chat(PlayerNumber plyr_idx, char *out, size_t out_size)
{
    if ((out == NULL) || (out_size == 0)) return;
    out[0] = '\0';
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) return;
    snprintf(out, out_size, "%s", s_pending_chat[plyr_idx]);
    s_pending_chat[plyr_idx][0] = '\0';
}

int64_t kctl_pending_dig_rects(int64_t slot, int64_t *out4, int64_t max)
{
    if (!slot_ok(slot) || (out4 == NULL)) {
        return 0;
    }
    const struct ExtSeatQueue *q = &s_queue[slot];
    int64_t n = 0;
    if ((q->cur_id != 0) && (q->next < q->count) && (q->cur_verb.kind == ESV_MarkDig) && q->cur_verb.has_rect && (n < max)) {
        const struct ExtSeatVerb *v = &q->cur_verb;
        out4[4 * n] = v->slab_x0; out4[4 * n + 1] = v->slab_y0; out4[4 * n + 2] = v->slab_x1; out4[4 * n + 3] = v->slab_y1;
        n++;
    }
    for (int64_t l = 0; l < EXTSEAT_LANES; l++) {
        for (int64_t i = 0; (i < q->lane_count[l]) && (n < max); i++) {
            const struct ExtSeatVerb *v = &q->lane[l][i].verb;
            if ((v->kind != ESV_MarkDig) || !v->has_rect) continue;
            out4[4 * n] = v->slab_x0; out4[4 * n + 1] = v->slab_y0; out4[4 * n + 2] = v->slab_x1; out4[4 * n + 3] = v->slab_y1;
            n++;
        }
    }
    return n;
}

int64_t kctl_ordered_count(int64_t slot)
{
    return slot_ok(slot) ? s_queue[slot].ordered_count : 0;
}

TbBool kctl_creature_ordered(int64_t slot, DaiThingIndex idx)
{
    if (!slot_ok(slot)) return false;
    const struct ExtSeatQueue *q = &s_queue[slot];
    for (int64_t i = 0; i < q->ordered_count; i++) {
        if (q->ordered[i].idx == idx) return true;
    }
    return false;
}

void kctl_release_ordered_now(int64_t slot, const char *reason)
{
    if (!slot_ok(slot)) {
        return;
    }
    struct ExtSeatQueue *q = &s_queue[slot];
    for (int64_t i = 0; i < q->ordered_count; i++) {
        hand_back(slot, &q->ordered[i], reason);
    }
    q->ordered_count = 0;
}

/** An attack order's creature has arrived: fighting what is there (keeper_order_start_attack). */
static void start_attack(int64_t slot, struct OrderedCreature *o, struct Thing *thing)
{
    keeper_order_start_attack(thing, (DaiSubtlCoord)o->goal_x, (DaiSubtlCoord)o->goal_y);
    record_auto_release(slot, o->idx, "attacking");
}

static TbBool stopped_at_enemy_door(const struct Thing *thing, DaiSubtlCoord goal_x, DaiSubtlCoord goal_y)
{
    return keeper_order_stopped_at_enemy_door(thing, goal_x, goal_y);
}

/** One turn of the safety net for one seat. */
static void sweep_ordered(int64_t slot)
{
    struct ExtSeatQueue *q = &s_queue[slot];
    const DaiTurn now = get_gameturn();
    for (int64_t i = 0; i < q->ordered_count;) {
        struct OrderedCreature *o = &q->ordered[i];
        struct Thing *thing = thing_get(o->idx);
        if (thing_is_invalid(thing) || !thing_is_creature(thing) || (thing->creation_turn != o->creation_turn)) {
            drop_ordered(q, i); // gone (dead, or the slot was reused)
            continue;
        }
        if (!in_manual_control(thing)) {
            // The order's clicks take a few turns to land; after that, not being in manual control means someone (the
            // agent, or the engine) already handed the creature back. A release was held when it started, so free now
            // means it worked.
            if (o->releasing || (now > o->ordered_turn + 30)) {
                drop_ordered(q, i);
                continue;
            }
            i++;
            continue;
        }
        if (((q->cur_id == o->verb_id) && (q->next < q->count)) || (keeper_possessed_thing(thing->owner) == thing->index)) {
            // The order that holds it is still clicking (Order creature mode): its selection click holds the creature where
            // it stands and the next click sends it off, so nothing here may act on it yet -- a creature that started
            // beside its goal would otherwise count as arrived, be set fighting, and then be held again by that click.
            i++;
            continue;
        }
        if (o->attack && (thing->active_state == CrSt_ManualControl)
            && (((llabs((int64_t)thing->mappos.x.stl.num - o->goal_x) <= 1) && (llabs((int64_t)thing->mappos.y.stl.num - o->goal_y) <= 1))
                || stopped_at_enemy_door(thing, o->goal_x, o->goal_y))) {
            start_attack(slot, o, thing); // arrived (walking, it is in a move state that continues into manual control)
            drop_ordered(q, i);
            continue;
        }
        const char *why = NULL;
        // The release's second click missed (the creature was knocked onto the next subtile between the clicks, so the
        // click became a move order): free it here, as the agent asked.
        if (o->releasing) why = (now >= o->hold_until) ? "released" : NULL;
        // A held creature does not fight back on its own (CrSt_ManualControl), so an enemy engaging it would otherwise
        // kill it where it stands; the attacker is already in its opponent lists, so handing it back lets it fight.
        else if (creature_is_being_attacked_by_enemy_player(thing)) why = "attacked";
        else if (creature_near_hunger_limit(thing)) why = "hungry";
        else if (creature_owed_pay(thing)) why = "owed_pay";
        else if (turns_to_payday(thing->owner) <= EXTSEAT_ORDER_PAYDAY_MARGIN_TURNS) why = "payday";
        else if (now >= o->hold_until) why = "max_hold";
        if (why != NULL) {
            hand_back(slot, o, why);
            drop_ordered(q, i);
            continue;
        }
        i++;
    }
}

static void note_order_started(int64_t slot, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb)
{
    struct ExtSeatQueue *q = &s_queue[slot];
    if ((q->cur_lane == EXTSEAT_LANE_LIEUTENANT) ? s_lt_group_orders[slot] : s_group_orders[slot]) {
        return; // the simulation ends a group's order (process_keeper_groups), the same on every machine
    }
    if (!verb->has_thing) {
        return;
    }
    struct Thing *thing = thing_get((DaiThingIndex)verb->thing_id);
    if (thing_is_invalid(thing) || !thing_is_creature(thing)) {
        return;
    }
    for (int64_t i = 0; i < q->ordered_count; i++) {
        if (q->ordered[i].idx == thing->index) {
            drop_ordered(q, i);
            break;
        }
    }
    if ((verb->kind != ESV_MoveCreature) && (verb->kind != ESV_ReleaseCreature) && (verb->kind != ESV_Attack)) {
        return;
    }
    if (q->ordered_count >= EXTSEAT_MAX_ORDERED) {
        drop_ordered(q, 0);
    }
    struct OrderedCreature *o = &q->ordered[q->ordered_count++];
    memset(o, 0, sizeof(*o));
    o->idx = thing->index;
    o->creation_turn = thing->creation_turn;
    o->ordered_turn = get_gameturn();
    o->verb_id = q->cur_id;
    if (verb->kind == ESV_ReleaseCreature) {
        // Tracked until it is free, not dropped as the release starts: a release whose clicks miss would otherwise leave
        // the creature held with nothing left to hand it back (the live bug: Bile Demon 388 stayed in Manual Control).
        o->releasing = true;
        o->hold_until = get_gameturn() + EXTSEAT_RELEASE_GRACE_TURNS;
        return;
    }
    if (verb->kind == ESV_Attack) {
        o->attack = true;
        o->goal_x = (DaiSubtlCoord)verb->stl_x;
        o->goal_y = (DaiSubtlCoord)verb->stl_y;
    }
    const int64_t hold = (verb->hold_turns > 0) ? verb->hold_turns : default_hold_turns(plyr_idx);
    o->hold_until = get_gameturn() + hold;
}

int64_t kctl_results(int64_t slot, struct ExtSeatResult *out, int64_t max)
{
    if (!slot_ok(slot) || (out == NULL)) {
        return 0;
    }
    const struct ExtSeatQueue *q = &s_queue[slot];
    const int64_t have = (q->results_total < EXTSEAT_RESULT_RING) ? q->results_total : EXTSEAT_RESULT_RING;
    int64_t n = 0;
    for (int64_t i = q->results_total - have; (i < q->results_total) && (n < max); i++) {
        out[n++] = q->results[i % EXTSEAT_RESULT_RING];
    }
    return n;
}

TbBool kctl_enqueue(int64_t slot, const struct ExtSeatStep *steps, int64_t count)
{
    if (!slot_ok(slot) || (count < 1) || (count > EXTSEAT_MAX_STEPS) || !kctl_idle(slot)) {
        return false;
    }
    memcpy(s_queue[slot].steps, steps, sizeof(struct ExtSeatStep) * count);
    s_queue[slot].count = count;
    s_queue[slot].next = 0;
    return true;
}

void kctl_reset(int64_t slot)
{
    if ((slot >= 0) && (slot < PACKETS_COUNT)) {
        memset(&s_queue[slot], 0, sizeof(s_queue[slot]));
        s_world[slot] = NULL;
        s_group_orders[slot] = false;
        s_self_contained[slot] = false;
        s_lt_world[slot] = NULL;
        s_lt_group_orders[slot] = false;
        s_wrote_turn[slot] = 0;
    }
}

/* The saved queue (kctl_serialise): little-endian, version 1. */
#define KCTL_SAVE_VERSION 1

struct KctlOut {
    uint8_t *out;
    size_t cap;
    size_t len;
};

static void ko_bytes(struct KctlOut *o, const void *p, size_t n)
{
    if (n == 0) return;
    if ((o->out != NULL) && (o->len + n <= o->cap))
        memcpy(o->out + o->len, p, n);
    o->len += n;
}

static void ko_u(struct KctlOut *o, uint64_t v, int n)
{
    uint8_t b[8];
    for (int i = 0; i < n; i++) b[i] = (uint8_t)(v >> (8 * i));
    ko_bytes(o, b, (size_t)n);
}

static void ko_i64(struct KctlOut *o, int64_t v) { ko_u(o, (uint64_t)v, 8); }
static void ko_u8(struct KctlOut *o, uint8_t v) { ko_u(o, v, 1); }

static void ko_str(struct KctlOut *o, const char *s, size_t max)
{
    const size_t n = (s != NULL) ? strnlen(s, max) : 0;
    ko_u8(o, (uint8_t)n);
    ko_bytes(o, s, n);
}

struct KctlIn {
    const uint8_t *in;
    size_t len;
    size_t pos;
    TbBool ok;
};

static uint64_t ki_u(struct KctlIn *r, int n)
{
    if (!r->ok || ((size_t)n > r->len - r->pos)) {
        r->ok = false;
        return 0;
    }
    uint64_t v = 0;
    for (int i = 0; i < n; i++) v |= (uint64_t)r->in[r->pos + (size_t)i] << (8 * i);
    r->pos += (size_t)n;
    return v;
}

static int64_t ki_i64(struct KctlIn *r) { return (int64_t)ki_u(r, 8); }

static int64_t ki_range(struct KctlIn *r, int64_t lo, int64_t hi)
{
    const int64_t v = ki_i64(r);
    if ((v < lo) || (v > hi)) r->ok = false;
    return r->ok ? v : lo;
}

static TbBool ki_bool(struct KctlIn *r)
{
    const uint64_t v = ki_u(r, 1);
    if (v > 1) r->ok = false;
    return v == 1;
}

/** A string into `buf` (size bytes, terminated); longer than size - 1 fails. */
static void ki_str(struct KctlIn *r, char *buf, size_t size)
{
    const size_t n = (size_t)ki_u(r, 1);
    if (!r->ok || (n >= size) || (n > r->len - r->pos)) {
        r->ok = false;
        buf[0] = '\0';
        return;
    }
    memcpy(buf, r->in + r->pos, n);
    buf[n] = '\0';
    r->pos += n;
}

/** The parts of a verb the running gesture's steps still use (not the combat assistant's rule set). */
static void ko_verb(struct KctlOut *o, const struct ExtSeatVerb *v)
{
    ko_i64(o, v->kind);
    ko_str(o, v->name, sizeof(v->name) - 1);
    ko_u8(o, v->has_pos); ko_i64(o, v->stl_x); ko_i64(o, v->stl_y);
    ko_u8(o, v->has_thing); ko_i64(o, v->thing_id);
    ko_u8(o, v->has_rect); ko_i64(o, v->slab_x0); ko_i64(o, v->slab_y0); ko_i64(o, v->slab_x1); ko_i64(o, v->slab_y1);
    ko_i64(o, v->overcharge_turns); ko_u8(o, v->has_enabled); ko_u8(o, v->enabled); ko_i64(o, v->hold_turns);
    ko_i64(o, v->expires_turn); ko_u8(o, v->has_target_player); ko_i64(o, v->target_player);
    ko_i64(o, v->thing_count);
    for (int64_t i = 0; i < v->thing_count; i++) ko_i64(o, v->thing_ids[i]);
    ko_u8(o, v->has_room); ko_i64(o, v->room_id); ko_u8(o, v->has_direction); ko_i64(o, v->direction_angle);
    ko_i64(o, v->level); ko_u8(o, v->has_target_thing); ko_i64(o, v->target_thing);
    ko_str(o, v->message, sizeof(v->message) - 1);
    ko_u8(o, v->release); ko_u8(o, v->sacrifice); ko_u8(o, v->imprison); ko_u8(o, v->torture); ko_u8(o, v->dismiss);
    ko_u8(o, v->lose_spells); ko_i64(o, v->assist_bank); ko_u8(o, v->priority);
}

static void ki_verb(struct KctlIn *r, struct ExtSeatVerb *v)
{
    memset(v, 0, sizeof(*v));
    v->kind = (enum ExtSeatVerbKind)ki_range(r, ESV_None, ESV_COUNT - 1);
    if (v->kind == ESV_SetAssist) r->ok = false; // never written
    ki_str(r, v->name, sizeof(v->name));
    v->has_pos = ki_bool(r); v->stl_x = ki_i64(r); v->stl_y = ki_i64(r);
    v->has_thing = ki_bool(r); v->thing_id = ki_i64(r);
    v->has_rect = ki_bool(r); v->slab_x0 = ki_i64(r); v->slab_y0 = ki_i64(r); v->slab_x1 = ki_i64(r); v->slab_y1 = ki_i64(r);
    v->overcharge_turns = ki_range(r, 0, EXTSEAT_MAX_OVERCHARGE_TURNS); v->has_enabled = ki_bool(r); v->enabled = ki_bool(r);
    v->hold_turns = ki_i64(r);
    v->expires_turn = ki_i64(r); v->has_target_player = ki_bool(r); v->target_player = ki_i64(r);
    v->thing_count = ki_range(r, 0, EXTSEAT_MAX_HAND_SET);
    for (int64_t i = 0; i < v->thing_count; i++) v->thing_ids[i] = ki_i64(r);
    v->has_room = ki_bool(r); v->room_id = ki_i64(r); v->has_direction = ki_bool(r); v->direction_angle = ki_i64(r);
    v->level = ki_i64(r); v->has_target_thing = ki_bool(r); v->target_thing = ki_i64(r);
    ki_str(r, v->message, sizeof(v->message));
    v->release = ki_bool(r); v->sacrifice = ki_bool(r); v->imprison = ki_bool(r); v->torture = ki_bool(r);
    v->dismiss = ki_bool(r); v->lose_spells = ki_bool(r); v->assist_bank = ki_range(r, 0, KEEPER_ASSIST_BANKS);
    v->priority = ki_bool(r);
}

size_t kctl_verb_serialise(const KeeperVerb *v, uint8_t *out, size_t cap)
{
    if ((v == NULL) || (v->kind == ESV_SetAssist)) return 0;
    struct KctlOut o = { out, cap, 0 };
    ko_u(&o, KCTL_SAVE_VERSION, 1);
    ko_verb(&o, v);
    ko_i64(&o, v->cancel_id);
    ko_i64(&o, v->cancel_lane);
    return o.len;
}

TbBool kctl_verb_deserialise(KeeperVerb *v, const uint8_t *in, size_t len)
{
    if ((v == NULL) || (in == NULL)) return false;
    struct KctlIn r = { in, len, 0, true };
    if (ki_u(&r, 1) != KCTL_SAVE_VERSION) r.ok = false;
    ki_verb(&r, v);
    v->cancel_id = ki_range(&r, 0, INT64_MAX);
    v->cancel_lane = ki_range(&r, 0, EXTSEAT_LANES);
    return r.ok && (r.pos == r.len);
}

size_t kctl_serialise(int64_t slot, uint8_t *out, size_t cap)
{
    if (!slot_ok(slot)) return 0;
    const struct ExtSeatQueue *q = &s_queue[slot];
    if ((queued_total(q) != 0) || (q->ordered_count != 0) || ((q->cur_id != 0) && (q->cur_verb.kind == ESV_SetAssist)))
        return 0;
    struct KctlOut o = { out, cap, 0 };
    ko_u8(&o, KCTL_SAVE_VERSION);
    ko_i64(&o, q->next_id);
    ko_i64(&o, q->results_total);
    for (int64_t i = 0; i < EXTSEAT_RESULT_RING; i++) {
        const struct ExtSeatResult *res = &q->results[i];
        ko_i64(&o, res->id); ko_u8(&o, res->rejected); ko_str(&o, res->error, sizeof(res->error) - 1);
        ko_i64(&o, res->turn); ko_i64(&o, res->submitted); ko_i64(&o, res->started); ko_i64(&o, res->lane);
        ko_i64(&o, res->suspended);
    }
    ko_i64(&o, q->last_chat_turn); ko_i64(&o, q->assist_written); ko_i64(&o, q->step_wait);
    ko_u8(&o, q->button_down); ko_u(&o, q->hand_dropped, 8);
    ko_i64(&o, q->cur_id); ko_i64(&o, q->cur_submitted); ko_i64(&o, q->cur_started); ko_i64(&o, q->cur_lane);
    ko_i64(&o, q->cur_suspended);
    ko_str(&o, q->cur_error, sizeof(q->cur_error_buf) - 1);
    ko_verb(&o, &q->cur_verb);
    ko_i64(&o, q->count - q->next);
    for (int64_t i = q->next; i < q->count; i++) {
        const struct ExtSeatStep *st = &q->steps[i];
        ko_u8(&o, st->action); ko_i64(&o, st->par1); ko_i64(&o, st->par2); ko_i64(&o, st->par3); ko_i64(&o, st->par4);
        ko_u8(&o, st->has_pos); ko_i64(&o, st->pos_x); ko_i64(&o, st->pos_y); ko_u(&o, st->control_flags, 8);
        ko_u8(&o, st->context); ko_i64(&o, st->follow_thing); ko_u8(&o, st->hand_set_drop); ko_u8(&o, st->send_chat);
        ko_u8(&o, st->wait_build_free); ko_u8(&o, st->wait_hand_free);
    }
    return o.len;
}

TbBool kctl_deserialise(int64_t slot, const uint8_t *in, size_t len)
{
    if (!slot_ok(slot) || (in == NULL)) return false;
    struct ExtSeatQueue *q = (struct ExtSeatQueue *)calloc(1, sizeof(struct ExtSeatQueue));
    if (q == NULL) return false;
    struct KctlIn r = { in, len, 0, true };
    if (ki_u(&r, 1) != KCTL_SAVE_VERSION) r.ok = false;
    q->next_id = ki_range(&r, 0, INT64_MAX);
    q->results_total = ki_range(&r, 0, INT64_MAX);
    for (int64_t i = 0; i < EXTSEAT_RESULT_RING; i++) {
        struct ExtSeatResult *res = &q->results[i];
        res->id = ki_range(&r, 0, INT64_MAX); res->rejected = ki_bool(&r); ki_str(&r, res->error, sizeof(res->error));
        res->turn = ki_i64(&r); res->submitted = ki_i64(&r); res->started = ki_i64(&r);
        res->lane = ki_range(&r, 0, EXTSEAT_LANES - 1); res->suspended = ki_range(&r, 0, INT64_MAX);
    }
    q->last_chat_turn = ki_i64(&r); q->assist_written = ki_i64(&r);
    q->step_wait = ki_range(&r, 0, (EXTSEAT_BUILD_WAIT_TURNS > EXTSEAT_HAND_WAIT_TURNS) ? EXTSEAT_BUILD_WAIT_TURNS : EXTSEAT_HAND_WAIT_TURNS);
    q->button_down = ki_bool(&r); q->hand_dropped = ki_u(&r, 8);
    q->cur_id = ki_range(&r, 0, INT64_MAX); q->cur_submitted = ki_i64(&r); q->cur_started = ki_i64(&r);
    q->cur_lane = ki_range(&r, 0, EXTSEAT_LANES - 1); q->cur_suspended = ki_range(&r, 0, INT64_MAX);
    ki_str(&r, q->cur_error_buf, sizeof(q->cur_error_buf));
    q->cur_error = (q->cur_error_buf[0] != '\0') ? q->cur_error_buf : NULL;
    ki_verb(&r, &q->cur_verb);
    q->count = ki_range(&r, 0, EXTSEAT_MAX_STEPS);
    for (int64_t i = 0; (i < q->count) && r.ok; i++) {
        struct ExtSeatStep *st = &q->steps[i];
        st->action = (unsigned char)ki_u(&r, 1); st->par1 = ki_i64(&r); st->par2 = ki_i64(&r); st->par3 = ki_i64(&r);
        st->par4 = ki_i64(&r); st->has_pos = ki_bool(&r); st->pos_x = ki_i64(&r); st->pos_y = ki_i64(&r);
        st->control_flags = ki_u(&r, 8); st->context = (unsigned char)ki_u(&r, 1);
        st->follow_thing = (DaiThingIndex)ki_range(&r, 0, THINGS_COUNT - 1); st->hand_set_drop = ki_bool(&r);
        st->send_chat = ki_bool(&r); st->wait_build_free = ki_bool(&r); st->wait_hand_free = ki_bool(&r);
    }
    q->next = 0;
    if (r.ok && (r.pos != r.len)) r.ok = false; // trailing bytes: not this format
    const TbBool ok = r.ok;
    if (ok) {
        // The slot's world is not part of the queue: whoever drives the slot set it already.
        memcpy(&s_queue[slot], q, sizeof(*q));
        if (s_queue[slot].cur_error != NULL) s_queue[slot].cur_error = s_queue[slot].cur_error_buf;
    } else {
        memset(&s_queue[slot], 0, sizeof(s_queue[slot]));
    }
    free(q);
    return ok;
}

int64_t kctl_queued_steps(int64_t slot)
{
    return slot_ok(slot) ? (s_queue[slot].count - s_queue[slot].next) : 0;
}

TbBool kctl_mid_gesture(int64_t slot)
{
    return slot_ok(slot) && s_queue[slot].button_down;
}

int64_t kctl_cancel(int64_t slot)
{
    if (!slot_ok(slot)) {
        return 0;
    }
    struct ExtSeatQueue *q = &s_queue[slot];
    int64_t dropped = (q->next < q->count) ? (q->count - q->next) : 0;
    for (int64_t lane = 0; lane < EXTSEAT_LANES; lane++) dropped += kctl_cancel_lane(slot, lane);   // every lane (14 R9)
    if ((q->cur_id != 0) && (q->next < q->count)) {
        record_running(slot, "CANCELLED");
    }
    q->cur_id = 0;
    q->cur_error = NULL;
    // Leaving the tool cancels a drag in progress (a state switch clears the roomspace and its drag), and the
    // release clears the engine's "button down" bookkeeping so the next gesture starts clean.
    end_gesture_here(q);
    return dropped;
}

/** The ordered-creature safety net, once per turn for this slot (before any reflex that may submit an urgent verb). */
void kctl_sweep(int64_t slot)
{
    if (!slot_ok(slot)) return;
    sweep_ordered(slot);
}

void kctl_tick(int64_t slot, PlayerNumber plyr, struct Packet *out)
{
    if (!slot_ok(slot) || (out == NULL)) return;
    struct ExtSeatQueue *q = &s_queue[slot];
    preempt_for_urgent(slot);
    promote_verbs(slot, plyr);
    if (q->next >= q->count) {
        return;
    }
    const struct ExtSeatStep *s = &q->steps[q->next];
    if (s->wait_build_free) {
        const struct UserState *player = get_user_state(slot);
        if (player->roomspace.is_active) {
            if (q->step_wait++ < EXTSEAT_BUILD_WAIT_TURNS) {
                return;
            }
            q->next = q->count;
            q->step_wait = 0;
            q->cur_error = "BUILD_BUSY";
            return;
        }
    }
    if (s->wait_hand_free) {
        const struct UserState *player = get_user_state(slot);
        // A hand instance still playing: the engine ignores a new hand action (a grab, a drop, a slap) until it ends.
        if ((player->instance_num == PI_Grab) || (player->instance_num == PI_Drop)
         || (player->instance_num == PI_Whip) || (player->instance_num == PI_WhipEnd)) {
            if (q->step_wait++ < EXTSEAT_HAND_WAIT_TURNS) {
                return;
            }
            q->next = q->count;
            q->step_wait = 0;
            q->cur_error = "HAND_BUSY";
            return;
        }
    }
    if (s->hand_set_drop && (q->cur_verb.kind == ESV_GiveGold)) {
        // The hand started empty and grabbed gold: the pile on top of it is ours (a hoard grab makes a new one).
        const struct UserState *seat = get_user_state(slot); // the slot's own hand
        const struct Thing *top = (seat->num_things_in_hand > 0) ? thing_get(seat->things_in_hand[0]) : NULL;
        if ((top == NULL) || thing_is_invalid(top) || !object_is_gold_pile(top)) {
            if ((top == NULL) && (q->step_wait++ < 3)) {
                return; // the grab may still be landing
            }
            q->next = q->count;
            q->step_wait = 0;
            q->cur_error = (top == NULL) ? "PICK_UP_FAILED" : "HAND_CHANGED";
            return;
        }
    } else if (s->hand_set_drop) {
        const struct UserState *seat = get_user_state(slot);
        const struct ExtSeatVerb *v = &q->cur_verb;
        // One thing (thing_id) or several (thing_ids).
        const int64_t nset = (v->thing_count > 0) ? v->thing_count : 1;
        const int64_t *set = (v->thing_count > 0) ? v->thing_ids : &v->thing_id;
        const DaiThingIndex top = (seat->num_things_in_hand > 0) ? seat->things_in_hand[0] : 0;
        int64_t mine = -1, dropped = 0, waiting = 0;
        for (int64_t i = 0; i < nset; i++) {
            if ((q->hand_dropped & (1ULL << i)) != 0) { dropped++; continue; }
            if (set[i] == top) mine = i;
            const struct Thing *t = thing_get((DaiThingIndex)set[i]);
            if (!thing_is_invalid(t) && !thing_is_picked_up_by_player(t, plyr)) waiting++;
        }
        if (mine < 0) {
            // Nothing of ours on top of the hand (empty, or what it held before). A pick may still be landing (the
            // grab is a player instance), so give it a moment; then end the verb -- never drop something else.
            if ((waiting > 0) && (dropped == 0) && (q->step_wait++ < 3)) {
                return;
            }
            q->next = q->count;
            q->step_wait = 0;
            if (dropped == 0) {
                q->cur_error = ((top == 0) || (waiting > 0)) ? "PICK_UP_FAILED" : "HAND_CHANGED";
            } else if (dropped < nset) {
                snprintf(q->cur_error_buf, sizeof(q->cur_error_buf), "PARTIAL: %" PRId64 " of %" PRId64, dropped, nset);
                q->cur_error = q->cur_error_buf;
            }
            return;
        }
        q->hand_dropped |= (1ULL << mine);
    }
    q->step_wait = 0;
    q->next++;
    s_wrote_turn[slot] = (int64_t)get_gameturn() + 1;   // +1: 0 means never
    s_wrote_lane[slot] = q->cur_lane;
    if (s->send_chat) {
        const PlayerNumber speaker = plyr;
        if ((speaker >= 0) && (speaker < PLAYERS_COUNT))
            snprintf(s_pending_chat[speaker], PLAYER_MP_MESSAGE_LEN, "%s", q->cur_verb.message);
        q->last_chat_turn = (int64_t)get_gameturn();
    }
    struct Packet *p = out;
    if (s->action != PckA_None) {
        int64_t a1 = s->par1, a2 = s->par2;
        if ((s->action == PckA_UsePwrHandDrop) && (s->follow_thing != 0)) {
            const struct Thing *ft = thing_get(s->follow_thing);
            if (!thing_is_invalid(ft)) { a1 = ft->mappos.x.stl.num; a2 = ft->mappos.y.stl.num; } // where it stands now
        } else if ((s->action == PckA_DaiDropAt) && (s->follow_thing != 0)) {
            const struct Thing *ft = thing_get(s->follow_thing);
            if (!thing_is_invalid(ft)) a1 = packet_pack_xy(ft->mappos.x.stl.num, ft->mappos.y.stl.num);
        }
        set_packet_action(p, s->action, a1, a2, s->par3, s->par4);
        if (is_assist_action(s->action)) {
            q->assist_written = (int64_t)get_gameturn() + 1;
        }
    }
    if (s->has_pos) {
        int64_t px = s->pos_x, py = s->pos_y;
        if (s->follow_thing != 0) {
            // Order-creature clicks pick "the creature nearest the cursor": aim at where it is now, not where it was
            // when the verb was planned (it may have walked since).
            const struct Thing *ft = thing_get(s->follow_thing);
            if (!thing_is_invalid(ft)) {
                // The exact position, not the subtile centre: the engine takes the creature nearest the cursor, and a
                // neighbour standing in the same subtile would otherwise win.
                px = ft->mappos.x.val;
                py = ft->mappos.y.val;
            }
        }
        set_players_packet_position(p, px, py, s->context);
    }
    p->control_flags |= s->control_flags;
    if ((s->control_flags & (PCtr_LBtnClick | PCtr_LBtnHeld)) != 0) {
        q->button_down = true;
    }
    if ((s->control_flags & PCtr_LBtnRelease) != 0) {
        q->button_down = false;
    }
}

// ---------------------------------------------------------------------------------------------
// Verbs
// ---------------------------------------------------------------------------------------------

static TbBool stl_on_map(int64_t x, int64_t y)
{
    return (x >= 0) && (y >= 0) && (x < game.map_subtiles_x) && (y < game.map_subtiles_y);
}

#define PRESS_FLAGS (PCtr_LBtnClick | PCtr_LBtnHeld)

static TbBool slab_on_map(int64_t x, int64_t y)
{
    return (x >= 0) && (y >= 0) && (x < game.map_tiles_x) && (y < game.map_tiles_y);
}

static unsigned char hand_pick_action(void)
{
    return s_plan_self_contained ? PckA_DaiHandPick : PckA_UsePwrHandPick;
}

static void step_action(struct ExtSeatStep *s, unsigned char action, int64_t p1, int64_t p2)
{
    memset(s, 0, sizeof(*s));
    s->action = action;
    s->par1 = p1;
    s->par2 = p2;
}

static void step_click_ctx(struct ExtSeatStep *s, int64_t stl_x, int64_t stl_y, uint64_t button, unsigned char context)
{
    memset(s, 0, sizeof(*s));
    s->has_pos = true;
    s->pos_x = subtile_coord_center(stl_x);
    s->pos_y = subtile_coord_center(stl_y);
    s->control_flags = button;
    s->context = context;
}

static void step_click(struct ExtSeatStep *s, int64_t stl_x, int64_t stl_y, uint64_t button)
{
    step_click_ctx(s, stl_x, stl_y, button, 0);
}

static void step_slab(struct ExtSeatStep *s, int64_t slb_x, int64_t slb_y, uint64_t button, unsigned char context)
{
    step_click_ctx(s, DaiSlabSubtileCenter(slb_x), DaiSlabSubtileCenter(slb_y), button, context);
}

static void step_idle(struct ExtSeatStep *s)
{
    memset(s, 0, sizeof(*s));
}

static const struct Thing *creature_thing(int64_t id)
{
    const struct Thing *thing = thing_get((DaiThingIndex)id);
    if (thing_is_invalid(thing) || !thing_is_creature(thing)) {
        return NULL;
    }
    return thing;
}


#define PRESS (PCtr_LBtnClick | PCtr_LBtnHeld) /* the frame the button goes down: the input driver reports both */
#define HOLD  (PCtr_LBtnHeld)
#define LET_GO (PCtr_LBtnRelease)

static int64_t kctl_plan_dig_sweep_from(struct ExtSeatStep *out, int64_t max, int64_t x0, int64_t y0, int64_t x1, int64_t y1, int corner);

int64_t kctl_plan_dig_sweep(struct ExtSeatStep *out, int64_t max, int64_t x0, int64_t y0, int64_t x1, int64_t y1)
{
    return kctl_plan_dig_sweep_from(out, max, x0, y0, x1, y1, 0);
}

/** kctl_plan_dig_sweep starting from a chosen corner (0 top-left, 1 bottom-right, 2 top-right, 3 bottom-left): a
 *  pickaxe drag tags or untags depending on its first slab, so mark_dig starts on one not already marked. */
static int64_t kctl_plan_dig_sweep_from(struct ExtSeatStep *out, int64_t max, int64_t x0, int64_t y0, int64_t x1, int64_t y1, int corner)
{
    const int64_t lx = (x0 < x1) ? x0 : x1, rx = (x0 < x1) ? x1 : x0;
    const int64_t ty = (y0 < y1) ? y0 : y1, by = (y0 < y1) ? y1 : y0;
    const int64_t rows = by - ty + 1;
    int64_t n = 0;
    if (rows > EXTSEAT_MAX_DIG_ROWS || (4 + 1 + 2 * rows + 1) > max) {
        return 0;
    }
    const TbBool rev_x = (corner == 1) || (corner == 2), rev_y = (corner == 1) || (corner == 3);
    const int64_t sx = rev_x ? rx : lx, ex = rev_x ? lx : rx;
    const int64_t sy = rev_y ? by : ty, dy = rev_y ? -1 : 1;
    // The brush is a 1x1 box; the pickaxe cursor context makes the held button tag what the cursor crosses,
    // and the engine interpolates a fast move between turns, so each row is one sweep and each row change one step.
    // That is the pickaxe's box mode, which is the player's own setting (a local player starts in DEFAULT_TAG_MODE,
    // and in DRAG mode a drag tags only the box from press to release), so set it rather than inherit it.
    step_action(&out[n++], PckA_SetPlyrState, PSt_CtrlDungeon, 0);
    step_action(&out[n++], PckA_SetRoomspaceMan, 1, 0);
    step_action(&out[n++], PckA_SetRoomspaceHighlight, box_placement_mode, 1);
    step_idle(&out[n++]);
    step_slab(&out[n++], sx, sy, PRESS, CSt_PickAxe);
    int64_t x = sx;
    int64_t y = sy;
    for (int64_t r = 0; r < rows; r++) {
        y = sy + dy * r;
        if (r > 0) {
            step_slab(&out[n++], x, y, HOLD, CSt_PickAxe);
        }
        const int64_t end = (r % 2 == 0) ? ex : sx;
        step_slab(&out[n++], end, y, HOLD, CSt_PickAxe);
        x = end;
    }
    step_slab(&out[n++], x, y, LET_GO, CSt_PickAxe);
    return n;
}

/** Press-and-release once over each of `count` slabs in the given player state and cursor context. */
static int64_t plan_slab_taps(struct ExtSeatStep *out, int64_t max, const int64_t *slab_xy, int64_t count, int64_t state, unsigned char context)
{
    int64_t n = 0;
    if (count < 1 || (4 + 2 * count) > max) {
        return 0;
    }
    step_action(&out[n++], PckA_SetPlyrState, state, 0);
    step_action(&out[n++], PckA_SetRoomspaceMan, 1, 0);
    // One slab per tap: the pickaxe's box mode, not the player's DRAG or auto-detect tag mode (see the dig sweep).
    step_action(&out[n++], PckA_SetRoomspaceHighlight, box_placement_mode, 1);
    step_idle(&out[n++]);
    for (int64_t i = 0; i < count; i++) {
        step_slab(&out[n++], slab_xy[2 * i], slab_xy[2 * i + 1], PRESS, context);
        step_slab(&out[n++], slab_xy[2 * i], slab_xy[2 * i + 1], LET_GO, context);
    }
    return n;
}

int64_t kctl_plan_slab_clicks(struct ExtSeatStep *out, int64_t max, const int64_t *slab_xy, int64_t count)
{
    int64_t n = 0;
    if (count < 1 || (3 + 2 * count) > max) {
        return 0;
    }
    step_action(&out[n++], PckA_SetPlyrState, PSt_Sell, 0);
    step_action(&out[n++], PckA_SetRoomspaceMan, 1, 0);
    step_idle(&out[n++]);
    for (int64_t i = 0; i < count; i++) {
        step_slab(&out[n++], slab_xy[2 * i], slab_xy[2 * i + 1], PRESS, 0);
        step_slab(&out[n++], slab_xy[2 * i], slab_xy[2 * i + 1], LET_GO, 0);
    }
    return n;
}

/** Validates `verb` against the live state and writes its steps into `steps` (EXTSEAT_MAX_STEPS long). NULL on success. */
/** Whether a bridge-kind slab could be built at (x, y) once `built` (the rectangle's slabs already built in this plan,
 *  indexed from lx/ty with width w) count as the player's land -- the engine's own rule, can_build_room_at_slab, needs
 *  a slab of the player's safe land beside a bridge slab, and a bridge slab just built is such land. */
static TbBool bridge_slab_buildable(PlayerNumber plyr_idx, RoomKind rkind, DaiSlabCoord x, DaiSlabCoord y,
    const unsigned char *built, int64_t lx, int64_t ty, int64_t w, int64_t h)
{
    if (can_build_room_at_slab(plyr_idx, rkind, x, y)) return true;
    const struct SlabMap *slb = get_slabmap_block(x, y);
    if (!room_can_build_on_bridge_slab(rkind, slb->kind) || (slb->room_index > 0)) return false;
    if (!subtile_revealed(DaiSlabSubtileCenter(x), DaiSlabSubtileCenter(y), plyr_idx)) return false;
    if (slab_has_trap_on(x, y) || slab_has_door_thing_on(x, y)) return false;
    static const int dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
    for (int d = 0; d < 4; d++) {
        const int64_t nx = x + dx[d] - lx, ny = y + dy[d] - ty;
        if ((nx >= 0) && (nx < w) && (ny >= 0) && (ny < h) && built[ny * w + nx]) return true;
    }
    return false;
}

int64_t kctl_room_build_check(PlayerNumber plyr_idx, RoomKind rkind, int64_t x0, int64_t y0, int64_t x1, int64_t y1,
    int *corner_out, int64_t *bad_xy, int64_t max_bad, int64_t *bad_count)
{
    const int64_t lx = (x0 < x1) ? x0 : x1, rx = (x0 < x1) ? x1 : x0, ty = (y0 < y1) ? y0 : y1, by = (y0 < y1) ? y1 : y0;
    const int64_t w = rx - lx + 1, h = by - ty + 1;
    unsigned char built[EXTSEAT_MAX_BUILD_SLABS], best_built[EXTSEAT_MAX_BUILD_SLABS];
    if ((w * h > EXTSEAT_MAX_BUILD_SLABS) || (w <= 0) || (h <= 0)) { if (bad_count) *bad_count = 0; return 0; }
    int64_t best = -1;
    int best_corner = 0;
    if (!room_role_matches(rkind, RoRoF_PassWater | RoRoF_PassLava | RoRoF_PassAbyss)) {
        // Any order: each slab stands on its own.
        best = 0;
        for (int64_t y = 0; y < h; y++)
            for (int64_t x = 0; x < w; x++) {
                best_built[y * w + x] = can_build_room_at_slab(plyr_idx, rkind, (DaiSlabCoord)(lx + x), (DaiSlabCoord)(ty + y)) ? 1 : 0;
                best += best_built[y * w + x];
            }
    } else {
        // Bridges: the engine builds one slab per turn, in the drag's order (rows from the start corner's row, and
        // along each row from its column), and each needs land beside it by its turn -- so simulate every start corner
        // (0 top-left, 1 bottom-right, 2 top-right, 3 bottom-left: roomspace.c's drag directions) and keep the best.
        for (int c = 0; c < 4; c++) {
            memset(built, 0, (size_t)(w * h));
            const TbBool rev_x = (c == 1) || (c == 2), rev_y = (c == 1) || (c == 3);
            int64_t count = 0;
            for (int64_t iy = 0; iy < h; iy++) {
                const int64_t y = rev_y ? (h - 1 - iy) : iy;
                for (int64_t ix = 0; ix < w; ix++) {
                    const int64_t x = rev_x ? (w - 1 - ix) : ix;
                    if (bridge_slab_buildable(plyr_idx, rkind, (DaiSlabCoord)(lx + x), (DaiSlabCoord)(ty + y), built, lx, ty, w, h)) {
                        built[y * w + x] = 1;
                        count++;
                    }
                }
            }
            if (count > best) { best = count; best_corner = c; memcpy(best_built, built, (size_t)(w * h)); }
        }
    }
    int64_t nbad = 0;
    for (int64_t y = 0; y < h; y++)
        for (int64_t x = 0; x < w; x++) {
            if (best_built[y * w + x]) continue;
            if ((bad_xy != NULL) && (nbad < max_bad)) { bad_xy[2 * nbad] = lx + x; bad_xy[2 * nbad + 1] = ty + y; }
            nbad++;
        }
    if (bad_count) *bad_count = nbad;
    if (corner_out) *corner_out = best_corner;
    return best;
}

/** Why the hand may not take a creature (can_thing_be_picked_up_by_player said no), its rules in that function's order:
 *  "CANNOT_PICK_UP:<rule>" (P9: Daimon's hand lifts at level 124 were refused 1,417 times in 48 games, all alike). */
static const char *pickup_refusal(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (!thing_is_creature(thing)) return "CANNOT_PICK_UP";
    if (flag_is_set(thing->state_flags, TF1_FallingIntoAbyss)) return "CANNOT_PICK_UP:FALLING";
    if (flag_is_set(get_creature_model_flags(thing), CMF_CannotPickUp)) return "CANNOT_PICK_UP:KIND";
    if (thing_pickup_is_blocked_by_hand_rule(thing, plyr_idx)) return "CANNOT_PICK_UP:HAND_RULE";
    if (creature_is_leaving_and_cannot_be_stopped(thing)) return "CANNOT_PICK_UP:LEAVING";
    if (!is_power_available(plyr_idx, PwrK_HAND)) return "CANNOT_PICK_UP:NO_HAND";
    if (!can_cast_power_at_xy(plyr_idx, PwrK_HAND, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0)) return "CANNOT_PICK_UP:NOT_HERE";
    if (thing_is_picked_up(thing)) return "CANNOT_PICK_UP:HELD";
    const struct PowerConfigStats *powerst = get_power_model_stats(PwrK_HAND);
    if (((powerst->can_cast_flags & PwCast_NeedsDelay) != 0) && (get_gameturn() <= get_player(plyr_idx)->power_of_cooldown_turn))
        return "CANNOT_PICK_UP:COOLDOWN";
    if (creature_is_dying(thing)) return "CANNOT_PICK_UP:DYING";
    if (creature_is_being_unconscious(thing)) return "CANNOT_PICK_UP:UNCONSCIOUS";
    if (armageddon_blocks_creature_pickup(thing, plyr_idx)) return "CANNOT_PICK_UP:ARMAGEDDON";
    if (creature_is_dragging_something(thing)) return "CANNOT_PICK_UP:DRAGGING";
    if (creature_is_being_sacrificed(thing) || creature_is_being_summoned(thing)) return "CANNOT_PICK_UP:SACRIFICE";
    const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (!creature_control_invalid(cctrl) && flag_is_set(cctrl->stateblock_flags, CCSpl_Teleport)) return "CANNOT_PICK_UP:TELEPORT";
    if (creature_under_spell_effect(thing, CSAfF_Timebomb)) return "CANNOT_PICK_UP:TIMEBOMB";
    return "CANNOT_PICK_UP:OTHER";
}

/** A thing the hand may take for this seat: its own creature, a prisoner in its custody, or an object the hand picks
 *  up (food, gold, ...). NULL with `err` set otherwise. */
static struct Thing *hand_thing(PlayerNumber plyr_idx, int64_t id, const char **err)
{
    struct Thing *thing = thing_get((DaiThingIndex)id);
    if (thing_is_invalid(thing) || !(thing_is_creature(thing) || thing_is_object(thing))) { *err = "NO_SUCH_THING"; return NULL; }
    if (thing_is_creature(thing) && (thing->owner != plyr_idx) && !creature_is_kept_in_custody_by_player(thing, plyr_idx)) {
        *err = "NOT_YOURS";
        return NULL;
    }
    if (!can_thing_be_picked_up_by_player(thing, plyr_idx)) {
        *err = pickup_refusal(thing, plyr_idx);
        if (thing_is_creature(thing)) {
            const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
            SYNCLOG("External seat %d: %s for %s %d: health %d of %d, state %s (continue %s), battle %d", (int)plyr_idx, *err,
                thing_model_name(thing), (int)thing->index, (int)thing->health,
                creature_control_invalid(cctrl) ? 0 : (int)cctrl->max_health, creature_state_code_name(thing->active_state),
                creature_state_code_name(thing->continue_state), creature_control_invalid(cctrl) ? 0 : (int)cctrl->battle_id);
        }
        return NULL;
    }
    return thing;
}

/** Whether dropping `thing` at the subtile is what the order says it means. The hand does more than move a creature:
 *  anything dropped on a temple pool is sacrificed (unless the order says `sacrifice`, only on the pool) -- an object
 *  too: a chicken kills all the dropper's chickens, gold is lost (or shared out to allies); a creature
 *  that lands on the seat's own portal walks out of the dungeon for good (unless it says `dismiss`, only on the portal);
 *  one of the seat's own creatures that lands in its prison is jailed there (unless it says `imprison`, only in the
 *  prison), and one that lands in its torture chamber is put on the rack (unless it says `torture`, only in the
 *  chamber); and a prisoner dropped anywhere but the seat's own prison or torture chamber goes free (unless it says
 *  `release`). Dismissal, jailing and the rack are the job the engine gives the creature on landing
 *  (predict_hand_drop_job, the creature_being_dropped rule: with the stock jobs, anywhere on the portal, in the prison
 *  or in the chamber, not for imps). A torture job the creature keeps its own will in (no JoKF_NoSelfControl: the stock
 *  KINKY_TORTURE a dark mistress enjoys) is what it would choose anyway, and is not refused. */
static const char *check_drop_intent(PlayerNumber plyr_idx, struct Thing *thing, DaiSubtlCoord x, DaiSubtlCoord y,
    const struct ExtSeatVerb *verb)
{
    const TbBool pool = subtile_has_sacrificial_on_top(x, y);
    if (pool && !verb->sacrifice) return "WOULD_SACRIFICE";
    if (!pool && verb->sacrifice) return "NOT_A_POOL";
    if (!thing_is_creature(thing)) return NULL;
    const struct Room *room = subtile_room_get(x, y);
    const TbBool own_room = !room_is_invalid(room) && (room->owner == plyr_idx);
    if (verb->dismiss && !(own_room && room_role_matches(room->kind, RoRoF_CrPoolLeave))) return "NOT_A_PORTAL";
    if (verb->imprison && !(own_room && room_role_matches(room->kind, RoRoF_Prison))) return "NOT_A_PRISON";
    if (verb->torture && !(own_room && room_role_matches(room->kind, RoRoF_Torture))) return "NOT_A_TORTURE_CHAMBER";
    const CreatureJob job = pool ? Job_NULL : predict_hand_drop_job(thing, plyr_idx, x, y);
    const RoomRole role = (job != Job_NULL) ? get_room_role_for_job(job) : RoRoF_None;
    const TbBool dismissed = (role & RoRoF_CrPoolLeave) != 0;
    if (dismissed && !verb->dismiss) return "WOULD_DISMISS";
    if ((thing->owner == plyr_idx) && ((role & RoRoF_Prison) != 0) && !verb->imprison) return "WOULD_IMPRISON";
    if ((thing->owner == plyr_idx) && ((role & RoRoF_Torture) != 0) && ((get_flags_for_job(job) & JoKF_NoSelfControl) != 0)
     && !verb->torture) return "WOULD_TORTURE";
    if ((thing->owner != plyr_idx) && !pool && !dismissed && !verb->release) {
        const TbBool holds = own_room && (room_role_matches(room->kind, RoRoF_Prison) || room_role_matches(room->kind, RoRoF_Torture));
        if (!holds) return "WOULD_RELEASE_PRISONER";
    }
    return NULL;
}

TbBool kctl_subtiles_connected(int64_t ax, int64_t ay, int64_t bx, int64_t by)
{
    if (!stl_on_map(ax, ay) || !stl_on_map(bx, by)) return false;
    struct Coord3d a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.x.val = subtile_coord_center(ax); a.y.val = subtile_coord_center(ay);
    b.x.val = subtile_coord_center(bx); b.y.val = subtile_coord_center(by);
    return navigation_points_connected(&a, &b);
}

const char *kctl_find_drop_spot(PlayerNumber plyr_idx, int64_t thing_id, int64_t x, int64_t y, int64_t radius,
    int64_t *out_x, int64_t *out_y)
{
    return kctl_find_drop_spot_reaching(plyr_idx, thing_id, x, y, radius, -1, -1, out_x, out_y);
}

const char *kctl_find_drop_spot_reaching(PlayerNumber plyr_idx, int64_t thing_id, int64_t x, int64_t y, int64_t radius,
    int64_t reach_x, int64_t reach_y, int64_t *out_x, int64_t *out_y)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) return "NOT_A_VALID_SEAT";
    struct Thing *thing = NULL;
    if (thing_id > 0) {
        thing = thing_get((DaiThingIndex)thing_id);
        if (thing_is_invalid(thing) || !thing_is_creature(thing) || (thing->owner != plyr_idx)) return "NO_CREATURE";
    } else {
        // Any of the seat's creatures stands in for "a creature of mine": the drop rules depend on the spot, not on it.
        const struct Dungeon *dungeon = get_players_num_dungeon(plyr_idx);
        const int64_t idx = (dungeon->creatr_list_start > 0) ? dungeon->creatr_list_start : dungeon->digger_list_start;
        thing = (idx > 0) ? thing_get((DaiThingIndex)idx) : NULL;
        if (thing_is_invalid(thing) || !thing_is_creature(thing)) return "NO_CREATURE";
    }
    if (radius < 0) radius = 0;
    if (radius > 45) radius = 45;
    struct ExtSeatVerb plain;
    memset(&plain, 0, sizeof(plain));
    // Ring by ring, nearest first; within a ring, the first in scan order.
    for (int64_t r = 0; r <= radius; r++) {
        for (int64_t dy = -r; dy <= r; dy++) {
            for (int64_t dx = -r; dx <= r; dx++) {
                if ((dx > -r) && (dx < r) && (dy > -r) && (dy < r)) continue;
                const int64_t cx = x + dx, cy = y + dy;
                if (!stl_on_map(cx, cy)) continue;
                if (!can_place_thing_here(thing, cx, cy, plyr_idx)) continue;
                if (check_drop_intent(plyr_idx, thing, (DaiSubtlCoord)cx, (DaiSubtlCoord)cy, &plain) != NULL) continue;
                if ((reach_x >= 0) && !kctl_subtiles_connected(cx, cy, reach_x, reach_y)) continue;
                *out_x = cx;
                *out_y = cy;
                return NULL;
            }
        }
    }
    return "NO_DROP_SPOT";
}

/** can_place_thing_here refused: say which of its tests failed, in the log (CANNOT_DROP_HERE is all the agent gets). */
static void log_drop_refusal(PlayerNumber plyr_idx, const struct Thing *thing, DaiSubtlCoord x, DaiSubtlCoord y)
{
    const struct Map *mapblk = get_map_block_at(x, y);
    const struct SlabMap *slb = get_slabmap_for_subtile(x, y);
    struct Coord3d pos = thing->mappos;
    pos.x.val = subtile_coord_center(x);
    pos.y.val = subtile_coord_center(y);
    pos.z.val = get_thing_height_at(thing, &pos);
    SYNCLOG("External seat %d: CANNOT_DROP_HERE for %s %d at (%d,%d): revealed %d, blocking %d, door %d, slab %s owner %d,"
        " in wall %d (z %d from %d, size %dx%d, floor %d, ceiling %d)", (int)plyr_idx, thing_model_name(thing), (int)thing->index,
        (int)x, (int)y, (int)map_block_revealed(mapblk, plyr_idx), (int)((mapblk->flags & SlbAtFlg_Blocking) != 0),
        (int)((mapblk->flags & SlbAtFlg_IsDoor) != 0), slab_code_name(slb->kind), (int)slabmap_owner(slb),
        (int)thing_in_wall_at(thing, &pos), (int)pos.z.val, (int)thing->mappos.z.val, (int)thing->clipbox_size_xy,
        (int)thing->clipbox_size_z, (int)get_map_floor_height(mapblk), (int)get_map_ceiling_height(mapblk));
}

/** A prisoner dropped into a prison or torture chamber with no room left for it is not held: on arrival the engine's
 *  add_creature_to_work_room refuses it ("room too small") and it walks free. Same test, summed over the prisoners
 *  being dropped (Job_CAPTIVITY in a prison, Job_PAINFUL_TORTURE in a torture chamber). */
static const char *check_room_admission(PlayerNumber plyr_idx, DaiSubtlCoord x, DaiSubtlCoord y, struct Thing *const *things, int64_t count)
{
    const struct Room *room = subtile_room_get(x, y);
    if (room_is_invalid(room) || (room->owner != plyr_idx)) return NULL;
    CreatureJob job;
    if (room_role_matches(room->kind, RoRoF_Prison)) job = Job_CAPTIVITY;
    else if (room_role_matches(room->kind, RoRoF_Torture)) job = Job_PAINFUL_TORTURE;
    else return NULL;
    int64_t need = 0;
    for (int64_t i = 0; i < count; i++) {
        if (thing_is_creature(things[i]) && (things[i]->owner != plyr_idx)) need += get_required_room_capacity_for_job(job, things[i]->model);
    }
    if ((need > 0) && (room->used_capacity + need > room->total_capacity)) return "ROOM_FULL";
    return NULL;
}

static TbBool room_drop_spot_ok(PlayerNumber plyr_idx, struct Thing *thing, DaiSubtlCoord x, DaiSubtlCoord y, TbBool sacrifice)
{
    return ((subtile_has_sacrificial_on_top(x, y) != 0) == (sacrifice != 0)) && can_place_thing_here(thing, x, y, plyr_idx);
}

/** to_room: a subtile inside the seat's room `room_id` where the hand would put `thing` -- the pool when the order
 *  sacrifices, else anywhere but the pool -- room centre first. */
static const char *resolve_room_drop(PlayerNumber plyr_idx, int64_t room_id, struct Thing *thing, TbBool sacrifice,
    int64_t *out_x, int64_t *out_y)
{
    struct Room *room = room_get((RoomIndex)room_id);
    if ((room_id <= 0) || room_is_invalid(room) || !room_exists(room)) return "NO_ROOM";
    if (room->owner != plyr_idx) return "NOT_YOUR_ROOM";
    // The room centre first, then every subtile of every slab of the room.
    if (room_drop_spot_ok(plyr_idx, thing, room->central_stl_x, room->central_stl_y, sacrifice)) {
        *out_x = room->central_stl_x; *out_y = room->central_stl_y;
        return NULL;
    }
    SlabCodedCoords sn = room->slabs_list;
    for (int64_t guard = 0; (sn != 0) && (guard < 4096); guard++) {
        const DaiSlabCoord sx = slb_num_decode_x(sn), sy = slb_num_decode_y(sn);
        for (int k = 0; k < 9; k++) {
            const DaiSubtlCoord x = slab_subtile(sx, k % 3), y = slab_subtile(sy, k / 3);
            if (room_drop_spot_ok(plyr_idx, thing, x, y, sacrifice)) {
                *out_x = x; *out_y = y;
                return NULL;
            }
        }
        sn = get_next_slab_number_in_room(sn);
    }
    return sacrifice ? "NO_POOL" : "CANNOT_DROP_HERE";
}

/** sell: the spellbooks on the seat's library slabs being sold are moved to another of its libraries with shelf space
 *  (position_books_in_room_with_capacity), and any that do not fit are destroyed, taking their spells away from the
 *  seat (remove_power_from_player) -- an agent that sells "the finished library" loses every spell it researched.
 *  Refused (WOULD_LOSE_SPELLS) when the books on the sold slabs outnumber the free space in the seat's libraries that
 *  keep all their slabs; the room being cut down is not counted, since how much it still holds afterwards depends on
 *  its new size and efficiency. */
static const char *check_sell_keeps_spells(PlayerNumber plyr_idx, int64_t lx, int64_t ty, int64_t rx, int64_t by)
{
    int64_t books = 0;
    int64_t idx = game.thing_lists[TngList_Objects].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *thing = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(thing)) break;
        idx = thing->next_of_class;
        if (!thing_is_spellbook(thing) || (thing->owner != plyr_idx)) continue;
        const int64_t sx = DaiSubtileSlab(thing->mappos.x.stl.num), sy = DaiSubtileSlab(thing->mappos.y.stl.num);
        if ((sx < lx) || (sx > rx) || (sy < ty) || (sy > by)) continue;
        const struct Room *room = subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (!room_is_invalid(room) && (room->owner == plyr_idx) && room_role_matches(room->kind, RoRoF_PowersStorage)) books++;
    }
    if (books == 0) return NULL;
    const struct Dungeon *dungeon = get_players_num_dungeon(plyr_idx);
    int64_t spare = 0;
    for (int64_t rkind = 1; rkind < TERRAIN_ITEMS_MAX; rkind++) {
        if (!room_role_matches((RoomKind)rkind, RoRoF_PowersStorage)) continue;
        int64_t ridx = dungeon->room_list_start[rkind];
        for (int64_t guard = 0; (ridx > 0) && (guard < ROOMS_COUNT); guard++) {
            const struct Room *room = room_get((RoomIndex)ridx);
            if (room_is_invalid(room)) break;
            ridx = room->next_of_owner;
            TbBool cut = false;
            SlabCodedCoords sn = room->slabs_list;
            for (int64_t g2 = 0; (sn != 0) && (g2 < 4096) && !cut; g2++) {
                const int64_t sx = slb_num_decode_x(sn), sy = slb_num_decode_y(sn);
                cut = (sx >= lx) && (sx <= rx) && (sy >= ty) && (sy <= by);
                sn = get_next_slab_number_in_room(sn);
            }
            if (!cut && (room->total_capacity > room->used_capacity)) spare += room->total_capacity - room->used_capacity;
        }
    }
    return (books > spare) ? "WOULD_LOSE_SPELLS" : NULL;
}

/** send_message: at most one per EXTSEAT_CHAT_MIN_TURNS -- short messages, not a stream. */
static const char *check_chat_rate(int64_t slot, const struct ExtSeatVerb *verb)
{
    const struct ExtSeatQueue *q = &s_queue[slot];
    if ((verb->kind == ESV_SendMessage) && (q->last_chat_turn > 0)
     && ((int64_t)get_gameturn() - q->last_chat_turn < EXTSEAT_CHAT_MIN_TURNS)) return "TOO_SOON";
    return NULL;
}

static const char *attack_target_at(PlayerNumber plyr_idx, DaiSubtlCoord x, DaiSubtlCoord y)
{
    return keeper_order_attack_target_at(plyr_idx, x, y);
}

static TbBool default_connected(void *ctx, DaiSubtlCoord ax, DaiSubtlCoord ay, DaiSubtlCoord bx, DaiSubtlCoord by)
{
    return kctl_subtiles_connected(ax, ay, bx, by);
}

static TbBool default_can_reach(void *ctx, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y)
{
    struct Thing *thing = thing_get(creature);
    if (thing_is_invalid(thing)) return false;
    struct Coord3d dest;
    memset(&dest, 0, sizeof(dest));
    dest.x.stl.num = x;
    dest.y.stl.num = y;
    dest.z.val = get_floor_height(x, y);
    return creature_can_navigate_to(thing, &dest, NavRtF_Default);
}

static const char *default_find_drop_spot(void *ctx, PlayerNumber plyr, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y,
    int64_t radius, DaiSubtlCoord reach_x, DaiSubtlCoord reach_y, DaiSubtlCoord *out_x, DaiSubtlCoord *out_y)
{
    (void)ctx;
    int64_t ox = 0, oy = 0;
    const char *err = kctl_find_drop_spot_reaching(plyr, creature, x, y, radius, reach_x, reach_y, &ox, &oy);
    if (err == NULL) {
        *out_x = (DaiSubtlCoord)ox;
        *out_y = (DaiSubtlCoord)oy;
    }
    return err;
}

static const struct KeeperCtlWorld s_default_world = { NULL, default_connected, default_can_reach, NULL, default_find_drop_spot };

const struct KeeperCtlWorld *kctl_default_world(void)
{
    return &s_default_world;
}

void kctl_set_group_orders(int64_t slot, TbBool on)
{
    if ((slot >= 0) && (slot < PACKETS_COUNT)) {
        s_group_orders[slot] = on;
    }
}

void kctl_set_world(int64_t slot, const struct KeeperCtlWorld *world)
{
    if ((slot >= 0) && (slot < PACKETS_COUNT)) {
        s_world[slot] = world;
    }
}

static const struct KeeperCtlWorld *slot_world(int64_t slot)
{
    return ((slot >= 0) && (slot < PACKETS_COUNT) && (s_world[slot] != NULL)) ? s_world[slot] : &s_default_world;
}

TbBool kctl_lane_wrote_this_turn(int64_t slot, int64_t lane)
{
    if ((slot < 0) || (slot >= PACKETS_COUNT)) return false;
    return (s_wrote_turn[slot] == (int64_t)get_gameturn() + 1) && (s_wrote_lane[slot] == lane);
}

void kctl_set_lieutenant_planning(int64_t slot, const struct KeeperCtlWorld *world, TbBool group_orders)
{
    if ((slot >= 0) && (slot < PACKETS_COUNT)) {
        s_lt_world[slot] = world;
        s_lt_group_orders[slot] = group_orders;
    }
}

static TbBool world_can_reach(const struct KeeperCtlWorld *w, const struct Thing *thing, DaiSubtlCoord x, DaiSubtlCoord y)
{
    if (w->can_reach != NULL) return w->can_reach(w->ctx, thing->index, x, y);
    return s_default_world.can_reach(NULL, thing->index, x, y);
}

/** Validates `verb` against the live state and writes its steps into `steps` (EXTSEAT_MAX_STEPS long). NULL on success.
 *  The map questions it cannot answer without writing navigation state go to `w` (the slot's world). */
/** A move, attack or release order as the group packets (keeper_group.h), for a slot that may not use Order creature
 *  mode (a Daimon keeper's, in any game): the creature leads the keeper's one group, the verb's thing_ids follow it; a
 *  group of other creatures is released first. Returns the steps written; *err if the group could not be made. */
static int64_t plan_group_order(PlayerNumber plyr_idx, const struct ExtSeatVerb *verb, const struct Thing *leader,
    int64_t goal_x, int64_t goal_y, struct ExtSeatStep *steps, const char **err)
{
    int64_t n = 0;
    *err = NULL;
    const struct KeeperGroupState *g = keeper_group_get(plyr_idx);
    if (verb->kind == ESV_ReleaseCreature) {
        step_action(&steps[n++], PckA_ReleaseGroup, 0, 0);
        return n;
    }
    DaiThingIndex follow[EXTSEAT_MAX_HAND_SET];
    int64_t nf = 0;
    for (int64_t i = 0; (i < verb->thing_count) && (i < EXTSEAT_MAX_HAND_SET); i++) {
        const struct Thing *t = creature_thing(verb->thing_ids[i]);
        if ((t == NULL) || (t->index == leader->index)) continue;
        if (t->owner != plyr_idx) { *err = "NOT_YOURS"; return 0; }
        if (!keeper_order_orderable(t)) { *err = "NOT_ORDERABLE"; return 0; }
        follow[nf++] = t->index;
    }
    int64_t cap = game.conf.rules[plyr_idx].rooms.barrack_max_party_size;
    if (cap > KEEPER_GROUP_MAX) cap = KEEPER_GROUP_MAX;
    if (1 + nf > cap) { *err = "GROUP_TOO_BIG"; return 0; }
    if ((nf > 0) && !player_has_room_of_role(plyr_idx, RoRoF_CrMakeGroup)) { *err = "NO_BARRACKS"; return 0; }
    // The group as it is already, led by this creature with these followers: ordered again, not made again.
    TbBool same = (g->count == 1 + nf) && (g->member[0] == leader->index);
    for (int64_t i = 0; same && (i < nf); i++) same = keeper_group_has(plyr_idx, follow[i]);
    if (!same) {
        if (g->count > 0) step_action(&steps[n++], PckA_ReleaseGroup, 0, 0);
        step_action(&steps[n++], PckA_FormGroup, leader->index, 0);
        for (int64_t i = 0; i < nf; i += 4) {
            step_action(&steps[n], PckA_GroupAdd, follow[i], (i + 1 < nf) ? follow[i + 1] : 0);
            steps[n].par3 = (i + 2 < nf) ? follow[i + 2] : 0;
            steps[n].par4 = (i + 3 < nf) ? follow[i + 3] : 0;
            n++;
        }
    }
    const int64_t order = (verb->kind == ESV_Attack) ? KGO_Attack : KGO_Move;
    step_action(&steps[n++], PckA_OrderGroup, order | (verb->hold_turns << 8), goal_x | (goal_y << 16));
    return n;
}

/** ESV_PlaceTrap, ESV_PlaceDoor: the steps (plan_verb's case, 14 S3). */
static const char *plan_place_trap(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        if (!verb->has_pos) return "MISSING_POSITION";
        if (!stl_on_map(verb->stl_x, verb->stl_y)) return "POSITION_OFF_MAP";
        const TbBool trap = (verb->kind == ESV_PlaceTrap);
        const int64_t model = trap ? trap_model_id(verb->name) : door_model_id(verb->name);
        if (model < 1) return "UNKNOWN_KIND";
        if (!(trap ? is_trap_placeable(plyr_idx, model) : is_door_placeable(plyr_idx, model))) return "NOT_AVAILABLE";
        // The engine's own spot tests (the click would otherwise land and do nothing): a trap on the seat's claimed
        // floor (or a bridge, for the kinds allowed there) not already trapped; a door on its claimed floor between two
        // walls, with nothing across the doorway.
        if (trap ? !can_place_trap_on(plyr_idx, (DaiSubtlCoord)verb->stl_x, (DaiSubtlCoord)verb->stl_y, (DaiThingModel)model)
                 : !door_placement_allowed(plyr_idx, (DaiSubtlCoord)verb->stl_x, (DaiSubtlCoord)verb->stl_y)) return "CANNOT_PLACE_HERE";
        if (s_gesture_packets) {
            step_action(&steps[n++], trap ? PckA_DaiPlaceTrap : PckA_DaiPlaceDoor, packet_pack_xy(verb->stl_x, verb->stl_y), model);
            break;
        }
        step_action(&steps[n++], PckA_SetPlyrState, trap ? PSt_PlaceTrap : PSt_PlaceDoor, model);
        step_click(&steps[n++], verb->stl_x, verb->stl_y, PCtr_LBtnClick);
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_Slap: the steps (plan_verb's case, 14 S3). */
static const char *plan_slap(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        if (!verb->has_thing) return "MISSING_THING";
        struct Thing *thing = thing_get((DaiThingIndex)verb->thing_id);
        if (thing_is_invalid(thing) || !(thing_is_creature(thing) || (thing->class_id == TCls_Trap))) return "NO_SUCH_THING";
        if (!thing_slappable(thing, plyr_idx)) return "NOT_SLAPPABLE";
        if (!dungeon_invalid(dungeon)
         && ((int64_t)(get_gameturn() - dungeon->last_creature_dropped_gameturn) <= SLAP_AFTER_DROP_TURNS)) return "SLAP_NOT_READY";
        // A whip still playing is waited out by the step (wait_hand_free), not refused: several slaps can be batched.
        if (s_plan_self_contained) {
            // The gesture: no camera, no aim (an aimed trap's slap needs its direction in the camera's bytes).
            if (verb->has_direction) return "NOT_SELF_CONTAINED";
            step_action(&steps[n], PckA_DaiCastOn, PwrK_SLAP, thing->index);
            steps[n++].wait_hand_free = true;
            break;
        }
        step_action(&steps[n], PckA_UsePwrOnThing, PwrK_SLAP, thing->index);
        steps[n].wait_hand_free = true; // the engine ignores a slap while the last one's whip is still playing
        if ((thing->class_id == TCls_Trap)) {
            // A slapped trap that shoots at a target (a boulder) goes the way the order says: the packet carries that angle
            // (packet_action_has_explicit_angle), whatever the camera faces.
            const struct TrapConfigStats *trapst = get_trap_model_stats(thing->model);
            const TbBool aimed = (trapst->activation_type == DaiTrapAct_HeadforTarget90) || (trapst->activation_type == TrpAcT_CreatureShot);
            if (aimed) return "AIMED_SLAP_UNSUPPORTED"; // a slap goes the way the slot's camera faces
        }
        n++;
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_PickUp: the steps (plan_verb's case, 14 S3). */
static const char *plan_pick_up(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        if (!verb->has_thing) return "MISSING_THING";
        const char *err = NULL;
        const struct Thing *thing = hand_thing(plyr_idx, verb->thing_id, &err);
        if (thing == NULL) return err;
        step_action(&steps[n], hand_pick_action(), thing->index, 0);
        steps[n++].wait_hand_free = true;
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_PickUpAndDrop: the steps (plan_verb's case, 14 S3). */
static const char *plan_pick_up_and_drop(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // pick_up then drop in one verb -- one thing or several (thing_ids, up to the hand's MaxThingsInHand), to one
        // spot (pos, or to_room) -- so an agent need not see the hand full before it may ask for the drop. Everything is
        // checked now with the engine's own tests: the hand must have room for them (what it holds already stays under
        // them: the drops release the top of the hand, and every drop checks the top is one of this verb's things),
        // each thing must be one the hand may take, and the
        // spot one the hand would drop it on, meaning what the order says (no prisoner freed, nothing sacrificed, by
        // accident).
        int64_t ids[EXTSEAT_MAX_HAND_SET];
        int64_t count = 0;
        if (verb->thing_count > 0) {
            if (verb->thing_count > EXTSEAT_MAX_HAND_SET) return "TOO_MANY";
            for (int64_t i = 0; i < verb->thing_count; i++) ids[count++] = verb->thing_ids[i];
        } else if (verb->has_thing) {
            ids[count++] = verb->thing_id;
        } else {
            return "MISSING_THING";
        }
        const int64_t hand_cap = (int64_t)game.conf.rules[plyr_idx].gameplay.max_things_in_hand;
        if (count > hand_cap) return "TOO_MANY";
        // The things the hand may take now. One it may not (knocked out, dying, dragging a body, ... or gone since the
        // order) is skipped, as a human's grab at it would be: the others are carried and the verb ends PARTIAL (the
        // drops below are one per thing of the order). A wrong order -- not the seat's, twice, treasury gold -- is
        // refused whole; so is one with nothing the hand may take (that thing's refusal).
        struct Thing *things[EXTSEAT_MAX_HAND_SET];
        int64_t take = 0;
        const char *skipped = NULL;
        for (int64_t i = 0; i < count; i++) {
            for (int64_t j = 0; j < i; j++) if (ids[j] == ids[i]) return "DUPLICATE_THING";
            const char *err = NULL;
            struct Thing *t = hand_thing(plyr_idx, ids[i], &err);
            if (t == NULL) {
                if ((strncmp(err, "CANNOT_PICK_UP", 14) != 0) && (strcmp(err, "NO_SUCH_THING") != 0)) return err;
                if (skipped == NULL) skipped = err;
                continue;
            }
            // A hand grab of treasury gold puts a new pile in the hand, not the hoard: give_gold carries it.
            if (object_is_gold_hoard(t)) return "TREASURY_GOLD";
            things[take++] = t;
        }
        if (take == 0) return skipped;
        // What the hand already holds stays under ours: the picks land on top, and the drops take the top first.
        if ((int64_t)plan_ustate()->num_things_in_hand + take > hand_cap) return "HAND_FULL";
        int64_t x, y;
        if (verb->has_room) {
            // Room capacity is checked below for both spot forms (check_room_admission: prisoners only, since a full
            // prison or torture chamber turns them away free); free torture devices are in the view for the agent to judge.
            const char *err = resolve_room_drop(plyr_idx, verb->room_id, things[0], verb->sacrifice, &x, &y);
            if (err != NULL) return err;
        } else if (verb->has_pos) {
            if (!stl_on_map(verb->stl_x, verb->stl_y)) return "POSITION_OFF_MAP";
            x = verb->stl_x; y = verb->stl_y;
        } else {
            return "MISSING_POSITION";
        }
        {
            const char *err = check_room_admission(plyr_idx, (DaiSubtlCoord)x, (DaiSubtlCoord)y, things, take);
            if (err != NULL) return err;
        }
        for (int64_t i = 0; i < take; i++) {
            if (!can_place_thing_here(things[i], x, y, plyr_idx)) {
                log_drop_refusal(plyr_idx, things[i], (DaiSubtlCoord)x, (DaiSubtlCoord)y);
                return "CANNOT_DROP_HERE";
            }
            const char *err = check_drop_intent(plyr_idx, things[i], (DaiSubtlCoord)x, (DaiSubtlCoord)y, verb);
            if (err != NULL) return err;
        }
        if (take + count > EXTSEAT_MAX_STEPS) return "TOO_MANY";
        for (int64_t i = 0; i < take; i++) {
            step_action(&steps[n], hand_pick_action(), things[i]->index, 0);
            steps[n++].wait_hand_free = true;
        }
        // A drop for each thing of the order, a skipped one's too: the one after the carried things finds none of
        // them on top of the hand and ends the verb PARTIAL.
        for (int64_t i = 0; i < count; i++) {
            step_action(&steps[n], PckA_UsePwrHandDrop, x, y);
            steps[n].wait_hand_free = true;
            steps[n++].hand_set_drop = true;
        }
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_GiveGold: the steps (plan_verb's case, 14 S3). */
static const char *plan_give_gold(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // Pay one of our creatures by hand: grab gold (a treasury hoard, or a loose pile) and drop it on the creature.
        if (!verb->has_thing) return "MISSING_THING";
        struct Thing *who = thing_get((DaiThingIndex)verb->thing_id);
        if (thing_is_invalid(who) || !thing_is_creature(who)) return "NO_SUCH_THING";
        if (who->owner != plyr_idx) return "NOT_YOURS";
        if (thing_is_picked_up(who) || !creature_able_to_get_salary(who)) return "CANNOT_PAY";
        if (!can_drop_thing_here(who->mappos.x.stl.num, who->mappos.y.stl.num, plyr_idx, 0)) return "CANNOT_DROP_HERE";
        if ((int64_t)plan_ustate()->num_things_in_hand
            >= (int64_t)game.conf.rules[plyr_idx].gameplay.max_things_in_hand) return "HAND_FULL";
        struct Thing *gold = NULL;
        if (verb->has_target_thing) {
            const char *err = NULL;
            gold = hand_thing(plyr_idx, verb->target_thing, &err);
            if (gold == NULL) return err;
            if (!object_is_gold_hoard(gold) && !object_is_gold_pile(gold)) return "NOT_GOLD";
            if (object_is_gold_hoard(gold)) {
                const struct Room *room = get_room_thing_is_on(gold);
                if (room_is_invalid(room) || (room->owner != plyr_idx)) return "NOT_YOURS";
            }
        } else {
            // The seat's largest hoard: the one the hand would take most from.
            int64_t best = 0;
            int64_t idx = game.thing_lists[TngList_Objects].index;
            for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
                struct Thing *t = thing_get((DaiThingIndex)idx);
                if (thing_is_invalid(t)) break;
                idx = t->next_of_class;
                if (!object_is_gold_hoard(t) || (t->valuable.gold_stored <= best)) continue;
                const struct Room *room = get_room_thing_is_on(t);
                if (room_is_invalid(room) || (room->owner != plyr_idx) || !can_thing_be_picked_up_by_player(t, plyr_idx)) continue;
                gold = t;
                best = t->valuable.gold_stored;
            }
            if (gold == NULL) return "NO_GOLD";
        }
        step_action(&steps[n], hand_pick_action(), gold->index, 0);
        steps[n++].wait_hand_free = true;
        if (s_gesture_packets) {
            // The drop names the creature it pays; it follows the creature if it walks meanwhile.
            step_action(&steps[n], PckA_DaiDropAt, packet_pack_xy(who->mappos.x.stl.num, who->mappos.y.stl.num), 0);
            steps[n].par3 = (int64_t)who->index;
            steps[n].follow_thing = who->index;
            steps[n].wait_hand_free = true;
            steps[n++].hand_set_drop = true;
            break;
        }
        // The cursor over the creature first (the drop pays the creature under the hand), then the drop on it; both follow
        // it if it walks meanwhile.
        step_idle(&steps[n]);   // a cursor step and nothing else: no action, button or wait left over on the stack
        steps[n].has_pos = true;
        steps[n].follow_thing = who->index;
        steps[n].pos_x = subtile_coord_center(who->mappos.x.stl.num);
        steps[n].pos_y = subtile_coord_center(who->mappos.y.stl.num);
        n++;
        step_action(&steps[n], PckA_UsePwrHandDrop, who->mappos.x.stl.num, who->mappos.y.stl.num);
        steps[n].has_pos = true;
        steps[n].follow_thing = who->index;
        steps[n].pos_x = subtile_coord_center(who->mappos.x.stl.num);
        steps[n].pos_y = subtile_coord_center(who->mappos.y.stl.num);
        steps[n].wait_hand_free = true;
        steps[n++].hand_set_drop = true;
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_Drop: the steps (plan_verb's case, 14 S3). */
static const char *plan_drop(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        if (plan_ustate()->num_things_in_hand == 0) return "HAND_EMPTY";
        struct Thing *held = get_first_held_thing(plan_ustate());
        int64_t x, y;
        if (verb->has_room) {
            const char *err = resolve_room_drop(plyr_idx, verb->room_id, held, verb->sacrifice, &x, &y);
            if (err != NULL) return err;
        } else if (verb->has_pos) {
            if (!stl_on_map(verb->stl_x, verb->stl_y)) return "POSITION_OFF_MAP";
            x = verb->stl_x; y = verb->stl_y;
        } else {
            return "MISSING_POSITION";
        }
        if (!thing_is_invalid(held)) {
            const char *err = check_drop_intent(plyr_idx, held, (DaiSubtlCoord)x, (DaiSubtlCoord)y, verb);
            if (err == NULL) err = check_room_admission(plyr_idx, (DaiSubtlCoord)x, (DaiSubtlCoord)y, &held, 1);
            if (err != NULL) return err;
        }
        step_action(&steps[n], PckA_UsePwrHandDrop, x, y);
        steps[n++].wait_hand_free = true;
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_CastPower: the steps (plan_verb's case, 14 S3). */
static const char *plan_cast_power(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        const int64_t pwkind = power_model_id(verb->name);
        if (pwkind < 1) return "UNKNOWN_KIND";
        if (!is_power_available(plyr_idx, (DaiPowerKind)pwkind)) return "NOT_AVAILABLE";
        if ((pwkind == PwrK_OBEY) || (pwkind == PwrK_ARMAGEDDON) || (pwkind == PwrK_HOLDAUDNC)) {
            // Whole-level powers: no target. (The engine takes them as a generic level power, exactly as the panel does.)
            if (verb->has_thing || verb->has_pos) return "UNSUPPORTED_POWER";
            step_action(&steps[n++], PckA_GenericLevelPower, 0, pwkind);
        } else if (verb->has_thing) {
            const struct Thing *thing = creature_thing(verb->thing_id);
            if (thing == NULL) return "NO_SUCH_THING";
            if (verb->overcharge_turns < 0 || verb->overcharge_turns > EXTSEAT_MAX_OVERCHARGE_TURNS) return "BAD_OVERCHARGE";
            if (s_plan_self_contained) {
                // One gesture, at the level the held turns would build (one a four).
                step_action(&steps[n], PckA_DaiCastOn, pwkind, thing->index);
                steps[n++].par3 = verb->overcharge_turns / 4;
            } else if (verb->overcharge_turns == 0) {
                step_action(&steps[n++], PckA_UsePwrOnThing, pwkind, thing->index);
            } else {
                // As a human charges a creature power: choose it, hold the button while the level builds (one level per
                // four held turns), and release on the creature, which sends the cast with the charge reached.
                const DaiSubtlCoord sx = thing->mappos.x.stl.num, sy = thing->mappos.y.stl.num;
                step_action(&steps[n++], PckA_SetPlyrState, PST_CastPowerOnTarget, pwkind);
                step_idle(&steps[n++]);
                step_click(&steps[n++], sx, sy, PRESS_FLAGS);
                for (int64_t i = 0; i < verb->overcharge_turns; i++) {
                    step_click(&steps[n++], sx, sy, PCtr_LBtnHeld);
                }
                step_click(&steps[n], sx, sy, PCtr_LBtnRelease);
                steps[n].action = PckA_UsePwrOnThing;
                steps[n].par1 = pwkind;
                steps[n].par2 = thing->index;
                n++;
            }
        } else if (verb->has_pos) {
            if (!stl_on_map(verb->stl_x, verb->stl_y)) return "POSITION_OFF_MAP";
            const uint64_t ws = get_power_model_stats((DaiPowerKind)pwkind)->work_state;
            if ((ws != PSt_CastPowerOnSubtile) && (ws != PSt_CallToArms) && (ws != PSt_SightOfEvil) && (ws != PSt_CreateDigger)) {
                return "UNSUPPORTED_POWER";
            }
            if (verb->overcharge_turns < 0 || verb->overcharge_turns > EXTSEAT_MAX_OVERCHARGE_TURNS) return "BAD_OVERCHARGE";
            // The engine's own rule for where this power may be cast (revealed ground, whose land, walls...): a click
            // elsewhere does nothing and costs nothing, so the order would read as done. The usual cause is a slab
            // coordinate given for the subtile position.
            if (!can_cast_power_at_xy(plyr_idx, (DaiPowerKind)pwkind, (DaiSubtlCoord)verb->stl_x, (DaiSubtlCoord)verb->stl_y, 0)) return "CANNOT_CAST_HERE";
            if (s_gesture_packets) {
                // One self-contained packet, at the level the held turns would build (one a four).
                step_action(&steps[n], PckA_DaiCastAt, packet_pack_xy(verb->stl_x, verb->stl_y), pwkind);
                steps[n++].par3 = verb->overcharge_turns / 4;
                break;
            }
            step_action(&steps[n++], PckA_SetPlyrState, ws, pwkind);
            if (verb->overcharge_turns == 0) {
                step_click(&steps[n++], verb->stl_x, verb->stl_y, PCtr_LBtnRelease);
            } else {
                // Holding builds the charge (one step per held turn; the level is turns / 4); the release casts at it.
                step_idle(&steps[n++]);
                step_click(&steps[n++], verb->stl_x, verb->stl_y, PRESS_FLAGS);
                for (int64_t i = 0; i < verb->overcharge_turns; i++) {
                    step_click(&steps[n++], verb->stl_x, verb->stl_y, PCtr_LBtnHeld);
                }
                step_click(&steps[n++], verb->stl_x, verb->stl_y, PCtr_LBtnRelease);
            }
        } else {
            return "MISSING_POSITION";
        }
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_MoveCreature, ESV_Attack, ESV_ReleaseCreature: the steps (plan_verb's case, 14 S3). */
static const char *plan_move_creature(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // Order creature mode (PSt_OrderCreatr): one click selects a creature, the next sends it to a subtile, a click on its
        // own subtile hands it back to its normal AI; leaving the mode clears the selection but not the order. Possession
        // needs a live view, this does not (ftest ai_gesture_order_creature records the engine's behaviour).
        if (!verb->has_thing) return "MISSING_THING";
        struct Thing *thing = (struct Thing *)creature_thing(verb->thing_id);
        if (thing == NULL) return "NO_SUCH_THING";
        if (thing->owner != plyr_idx) return "NOT_YOURS";
        if (thing_is_picked_up(thing)) return "NOT_ORDERABLE";
        const char *err_group = NULL;
        // Releasing a creature that is not held would hold it: the first click of Order creature mode selects the
        // creature and puts it in manual control (the safety net may already have handed it back: SEAT auto_released).
        if ((verb->kind == ESV_ReleaseCreature) && (group_orders ? !keeper_group_has(plyr_idx, thing->index) : !in_manual_control(thing))) {
            return "NOT_HELD";
        }
        const DaiSubtlCoord cx = thing->mappos.x.stl.num, cy = thing->mappos.y.stl.num;
        int64_t goal_x = cx, goal_y = cy;
        if (verb->kind != ESV_ReleaseCreature) {
            // A creature held in manual control does not work, eat or collect its pay. Refuse orders that would cost it
            // that, and bound how long an order may hold it (the safety net releases it when the time is up).
            if ((verb->hold_turns < 0) || (verb->hold_turns > max_hold_turns(plyr_idx))) return "BAD_HOLD";
            if (creature_near_hunger_limit(thing)) return "HUNGRY";
            if (creature_owed_pay(thing)) return "OWED_PAY";
            if (turns_to_payday(plyr_idx) <= EXTSEAT_ORDER_PAYDAY_MARGIN_TURNS) return "PAYDAY_TOO_CLOSE";
            if (!verb->has_pos) return "MISSING_POSITION";
            if (!stl_on_map(verb->stl_x, verb->stl_y)) return "POSITION_OFF_MAP";
            if ((verb->stl_x == cx) && (verb->stl_y == cy)) return "ALREADY_THERE";
            if ((verb->kind == ESV_Attack) && (attack_target_at(plyr_idx, (DaiSubtlCoord)verb->stl_x, (DaiSubtlCoord)verb->stl_y) == NULL)) {
                return "NOTHING_TO_ATTACK";
            }
            if (!world_can_reach(w, thing, (DaiSubtlCoord)verb->stl_x, (DaiSubtlCoord)verb->stl_y)) return "CANNOT_REACH";
            goal_x = verb->stl_x;
            goal_y = verb->stl_y;
            // A click on an enemy door's own slab sends nothing walking: go to the nearest slab beside it instead (the
            // arrival check in sweep_ordered counts a creature stopped beside the door as there).
            if ((verb->kind == ESV_Attack) && !thing_is_invalid(get_door_for_position((DaiSubtlCoord)goal_x, (DaiSubtlCoord)goal_y))) {
                const DaiSlabCoord sx = DaiSubtileSlab(goal_x), sy = DaiSubtileSlab(goal_y);
                int64_t best = -1;
                for (int k = 0; k < 4; k++) {
                    static const int8_t ddx[4] = { 0, 1, 0, -1 }, ddy[4] = { -1, 0, 1, 0 };
                    if (!slab_on_map(sx + ddx[k], sy + ddy[k])) continue;
                    const DaiSubtlCoord side_x = DaiSlabSubtileCenter(sx + ddx[k]), side_y = DaiSlabSubtileCenter(sy + ddy[k]);
                    if (!world_can_reach(w, thing, side_x, side_y)) continue;
                    const int64_t d = llabs((int64_t)side_x - cx) + llabs((int64_t)side_y - cy);
                    if ((best < 0) || (d < best)) { best = d; goal_x = side_x; goal_y = side_y; }
                }
            }
        }
        if (group_orders) {
            n = plan_group_order(plyr_idx, verb, thing, goal_x, goal_y, steps, &err_group);
            if (err_group != NULL) return err_group;
            break;
        }
        step_action(&steps[n++], PckA_SetPlyrState, PSt_CtrlDungeon, 0); // a clean start: no creature selected
        step_idle(&steps[n++]);
        step_action(&steps[n++], PckA_SetPlyrState, PSt_OrderCreatr, 0);
        step_idle(&steps[n++]);
        step_click(&steps[n], cx, cy, PCtr_LBtnRelease);                 // select the creature (aimed at where it is then)
        steps[n++].follow_thing = thing->index;
        step_idle(&steps[n++]);
        step_click(&steps[n], goal_x, goal_y, PCtr_LBtnRelease);         // send it there / (its own subtile) hand it back
        if (verb->kind == ESV_ReleaseCreature) steps[n].follow_thing = thing->index;
        n++;
        step_idle(&steps[n++]);
        step_action(&steps[n++], PckA_SetPlyrState, PSt_CtrlDungeon, 0); // leaving the mode clears the selection
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_SetTendency: the steps (plan_verb's case, 14 S3). */
static const char *plan_set_tendency(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        if (!verb->has_enabled) return "MISSING_ENABLED";
        int64_t tend;
        if (strcasecmp(verb->name, "imprison") == 0) tend = CrTend_Imprison;
        else if (strcasecmp(verb->name, "flee") == 0) tend = CrTend_Flee;
        else return "UNKNOWN_KIND";
        if (((dungeon->creature_tendencies & tend) != 0) == (verb->enabled != 0)) return "ALREADY_SET";
        // As the panel button: imprison can only be switched on with a prison to take captives to (the packet itself
        // does not check, so without this a seat could do what a human cannot).
        if ((tend == CrTend_Imprison) && verb->enabled && !player_has_room_of_role(plyr_idx, RoRoF_Prison)) return "NO_PRISON";
        step_action(&steps[n++], PckA_ToggleTendency, tend, 0);
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_SetAlliance: the steps (plan_verb's case, 14 S3). */
static const char *plan_set_alliance(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // A one-way declaration, exactly what the human alliance button does (toggle_ally_with_player only ever
        // touches the caller's own allied_players bit): the engine's own rules (players_are_mutual_allies) require
        // both sides to declare before it actually changes combat/vision, and own.alliance in the view shows both.
        if (!verb->has_target_player) return "MISSING_TARGET_PLAYER";
        const PlayerNumber other = (PlayerNumber)verb->target_player;
        if ((other < 0) || (other >= PLAYERS_COUNT) || (other == plyr_idx)) return "INVALID_PLAYER";
        if (!player_exists(get_player(other))) return "INVALID_PLAYER";
        if (!verb->has_enabled) return "MISSING_ENABLED";
        if (is_player_ally_locked(plyr_idx, other)) return "ALLIANCE_LOCKED";
        if (player_allied_with(player, other) == (verb->enabled != 0)) return "ALREADY_SET";
        step_action(&steps[n++], PckA_PlyrToggleAlly, other, 0);
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_BuildRoom: the steps (plan_verb's case, 14 S3). */
static const char *plan_build_room(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        if (!verb->has_rect) return "MISSING_RECT";
        const int64_t rx0 = verb->slab_x0, ry0 = verb->slab_y0, rx1 = verb->slab_x1, ry1 = verb->slab_y1;
        if (!slab_on_map(rx0, ry0) || !slab_on_map(rx1, ry1)) return "POSITION_OFF_MAP";
        const int64_t area = ((rx1 > rx0 ? rx1 - rx0 : rx0 - rx1) + 1) * ((ry1 > ry0 ? ry1 - ry0 : ry0 - ry1) + 1);
        if (area > EXTSEAT_MAX_BUILD_SLABS) return "AREA_TOO_LARGE";
        const int64_t rkind = get_rid(room_desc, verb->name);
        if (rkind < 1) return "UNKNOWN_KIND";
        if (!is_room_available(plyr_idx, (RoomKind)rkind)) return "NOT_AVAILABLE";
        // The engine's own per-slab rule, in the order it builds: a room nowhere in the rectangle is refused (the drag
        // would do nothing), and a bridge is dragged from the corner whose order lets each slab reach land in time.
        // A bridge drag is a path, not a box (roomspace.c's detect_bridge_shape: an L between the ends, turning the way the
        // previous drag did), so the seat builds bridges as straight lines only -- deterministic, and wider crossings
        // are several lines.
        if (room_role_matches((RoomKind)rkind, RoRoF_PassWater | RoRoF_PassLava | RoRoF_PassAbyss) && (rx0 != rx1) && (ry0 != ry1)) {
            return "BRIDGE_NOT_A_LINE";
        }
        int corner = 0;
        if (kctl_room_build_check(plyr_idx, (RoomKind)rkind, rx0, ry0, rx1, ry1, &corner, NULL, 0, NULL) == 0) return "CANNOT_BUILD_HERE";
        int64_t px = rx0, py = ry0, qx = rx1, qy = ry1;
        if (room_role_matches((RoomKind)rkind, RoRoF_PassWater | RoRoF_PassLava | RoRoF_PassAbyss)) {
            const int64_t lx = (rx0 < rx1) ? rx0 : rx1, hx = (rx0 < rx1) ? rx1 : rx0, ty = (ry0 < ry1) ? ry0 : ry1, by = (ry0 < ry1) ? ry1 : ry0;
            const TbBool rev_x = (corner == 1) || (corner == 2), rev_y = (corner == 1) || (corner == 3);
            px = rev_x ? hx : lx; qx = rev_x ? lx : hx;
            py = rev_y ? by : ty; qy = rev_y ? ty : by;
        }
        // The engine's drag is at most MAX_USER_ROOMSPACE_WIDTH slabs a side (create_box_roomspace_from_drag clips the
        // far corner, silently), so a larger rectangle is several drags of at most that, made one after another from
        // the start corner -- each waits for the engine to finish building the last (wait_build_free), so a bridge's
        // next piece starts from the end of the one before. Per drag: select the room, switch to drag mode, let a turn
        // pass, then press at one corner, hold at the other, and release there; the engine then builds the slabs, one
        // per turn, with no further packets.
        const int64_t side = MAX_USER_ROOMSPACE_WIDTH;
        const int64_t sx = (qx >= px) ? 1 : -1, sy = (qy >= py) ? 1 : -1;
        const int64_t wx = ((qx - px) * sx) + 1, wy = ((qy - py) * sy) + 1;
        const int64_t drags = ((wx + side - 1) / side) * ((wy + side - 1) / side);
        if (drags * (s_gesture_packets ? 1 : 7) > EXTSEAT_MAX_STEPS) return "AREA_TOO_LARGE";
        for (int64_t ty0 = 0; ty0 < wy; ty0 += side) {
            for (int64_t tx0 = 0; tx0 < wx; tx0 += side) {
                const int64_t ax = px + sx * tx0, ay = py + sy * ty0;
                const int64_t bx = px + sx * (((tx0 + side < wx) ? tx0 + side : wx) - 1);
                const int64_t by = py + sy * (((ty0 + side < wy) ? ty0 + side : wy) - 1);
                if (s_gesture_packets) {
                    step_action(&steps[n], PckA_DaiBuildRect, packet_pack_xy(ax, ay), packet_pack_xy(bx, by));
                    steps[n].par3 = rkind;
                    steps[n++].wait_build_free = true;
                    continue;
                }
                step_action(&steps[n], PckA_SetPlyrState, PSt_BuildRoom, rkind);
                steps[n++].wait_build_free = true;
                step_action(&steps[n++], PckA_SetRoomspaceDrag, 0, 0);
                step_idle(&steps[n++]);
                step_click_ctx(&steps[n++], DaiSlabSubtileCenter(ax), DaiSlabSubtileCenter(ay), PRESS_FLAGS, 0);
                step_click_ctx(&steps[n++], DaiSlabSubtileCenter(bx), DaiSlabSubtileCenter(by), PCtr_LBtnHeld, 0);
                step_click_ctx(&steps[n++], DaiSlabSubtileCenter(bx), DaiSlabSubtileCenter(by), PCtr_LBtnHeld, 0);
                step_click_ctx(&steps[n++], DaiSlabSubtileCenter(bx), DaiSlabSubtileCenter(by), PCtr_LBtnRelease, 0);
            }
        }
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_MarkDig: the steps (plan_verb's case, 14 S3). */
static const char *plan_mark_dig(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        if (!verb->has_rect) return "MISSING_RECT";
        if (!slab_on_map(verb->slab_x0, verb->slab_y0) || !slab_on_map(verb->slab_x1, verb->slab_y1)) return "POSITION_OFF_MAP";
        const int64_t dw = ((verb->slab_x1 > verb->slab_x0) ? verb->slab_x1 - verb->slab_x0 : verb->slab_x0 - verb->slab_x1) + 1;
        const int64_t dh = ((verb->slab_y1 > verb->slab_y0) ? verb->slab_y1 - verb->slab_y0 : verb->slab_y0 - verb->slab_y1) + 1;
        if (dw * dh > EXTSEAT_MAX_DIG_SLABS) return "AREA_TOO_LARGE";
        if (s_gesture_packets) {
            // The rectangle tagged as one packet: no first-slab rule, nothing untagged.
            step_action(&steps[n], PckA_DaiDigRect, packet_pack_xy(verb->slab_x0, verb->slab_y0), packet_pack_xy(verb->slab_x1, verb->slab_y1));
            steps[n++].par3 = 0;
            break;
        }
        // A pickaxe drag tags or untags according to its first slab (a marked one untags the whole sweep), so start on a
        // corner that is not marked; if all four are, tap the unmarked slabs one by one.
        const int64_t lx = (verb->slab_x0 < verb->slab_x1) ? verb->slab_x0 : verb->slab_x1, rx = (verb->slab_x0 < verb->slab_x1) ? verb->slab_x1 : verb->slab_x0;
        const int64_t ty = (verb->slab_y0 < verb->slab_y1) ? verb->slab_y0 : verb->slab_y1, by = (verb->slab_y0 < verb->slab_y1) ? verb->slab_y1 : verb->slab_y0;
        const int64_t cx[4] = { lx, rx, rx, lx }, cy[4] = { ty, by, ty, by };
        int corner = -1;
        for (int c = 0; (c < 4) && (corner < 0); c++) {
            if (find_from_task_list_by_slab(plyr_idx, (DaiSlabCoord)cx[c], (DaiSlabCoord)cy[c]) == -1) corner = c;
        }
        if (corner >= 0) {
            n = kctl_plan_dig_sweep_from(steps, EXTSEAT_MAX_STEPS, verb->slab_x0, verb->slab_y0, verb->slab_x1, verb->slab_y1, corner);
        } else {
            int64_t xy[2 * EXTSEAT_MAX_DIG_SLABS], cnt = 0;
            for (int64_t y = ty; y <= by; y++)
                for (int64_t x = lx; x <= rx; x++)
                    if (find_from_task_list_by_slab(plyr_idx, (DaiSlabCoord)x, (DaiSlabCoord)y) == -1) { xy[2 * cnt] = x; xy[2 * cnt + 1] = y; cnt++; }
            if (cnt == 0) {
                // Everything is marked already: nothing to do (and a drag here would take the marks off).
                step_idle(&steps[n++]);
                break;
            }
            n = plan_slab_taps(steps, EXTSEAT_MAX_STEPS, xy, cnt, PSt_CtrlDungeon, CSt_PickAxe);
        }
        if (n == 0) return "AREA_TOO_LARGE";
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_UnmarkDig: the steps (plan_verb's case, 14 S3). */
static const char *plan_unmark_dig(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // A pickaxe tap on a marked slab takes its mark off (the tap's first slab is marked, so it untags).
        if (!verb->has_rect) return "MISSING_RECT";
        if (!slab_on_map(verb->slab_x0, verb->slab_y0) || !slab_on_map(verb->slab_x1, verb->slab_y1)) return "POSITION_OFF_MAP";
        const int64_t lx = (verb->slab_x0 < verb->slab_x1) ? verb->slab_x0 : verb->slab_x1, rx = (verb->slab_x0 < verb->slab_x1) ? verb->slab_x1 : verb->slab_x0;
        const int64_t ty = (verb->slab_y0 < verb->slab_y1) ? verb->slab_y0 : verb->slab_y1, by = (verb->slab_y0 < verb->slab_y1) ? verb->slab_y1 : verb->slab_y0;
        if ((rx - lx + 1) * (by - ty + 1) > EXTSEAT_MAX_DIG_SLABS) return "AREA_TOO_LARGE";
        int64_t xy[2 * EXTSEAT_MAX_DIG_SLABS], cnt = 0;
        for (int64_t y = ty; y <= by; y++)
            for (int64_t x = lx; x <= rx; x++)
                if (find_from_task_list_by_slab(plyr_idx, (DaiSlabCoord)x, (DaiSlabCoord)y) != -1) { xy[2 * cnt] = x; xy[2 * cnt + 1] = y; cnt++; }
        if (cnt == 0) return "NOTHING_MARKED";
        if (s_gesture_packets) {
            step_action(&steps[n], PckA_DaiDigRect, packet_pack_xy(lx, ty), packet_pack_xy(rx, by));
            steps[n++].par3 = 1;
            break;
        }
        n = plan_slab_taps(steps, EXTSEAT_MAX_STEPS, xy, cnt, PSt_CtrlDungeon, CSt_PickAxe);
        if (n == 0) return "AREA_TOO_LARGE";
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_SetDoorLock: the steps (plan_verb's case, 14 S3). */
static const char *plan_set_door_lock(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // A click on one of your own doors toggles its lock, as a human's click does (the door-key cursor).
        if (!verb->has_pos) return "MISSING_POSITION";
        if (!verb->has_enabled) return "MISSING_ENABLED";
        if (!stl_on_map(verb->stl_x, verb->stl_y)) return "POSITION_OFF_MAP";
        struct Thing *door = get_door_for_position((DaiSubtlCoord)verb->stl_x, (DaiSubtlCoord)verb->stl_y);
        if (thing_is_invalid(door)) return "NO_DOOR";
        if (door->owner != plyr_idx) return "NOT_YOURS";
        if ((door->door.is_locked != 0) == (verb->enabled != 0)) return "ALREADY_SET";
        step_action(&steps[n++], PckA_SetPlyrState, PSt_CtrlDungeon, 0);
        step_idle(&steps[n++]);
        step_click_ctx(&steps[n++], door->mappos.x.stl.num, door->mappos.y.stl.num, PRESS, CSt_DoorKey);
        step_click_ctx(&steps[n++], door->mappos.x.stl.num, door->mappos.y.stl.num, LET_GO, CSt_DoorKey);
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_PowerOff: the steps (plan_verb's case, 14 S3). */
static const char *plan_power_off(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        const int64_t pwkind = power_model_id(verb->name);
        if (pwkind < 1) return "UNKNOWN_KIND";
        if (pwkind == PwrK_CALL2ARMS) {
            if (!player_uses_power_call_to_arms(plyr_idx)) return "NOT_ACTIVE";
            step_action(&steps[n++], PckA_PwrCTADis, 0, 0);
        } else if (pwkind == PwrK_SIGHT) {
            if (!player_uses_power_sight(plyr_idx)) return "NOT_ACTIVE";
            step_action(&steps[n++], PckA_PwrSOEDis, 0, 0);
        } else if (pwkind == PwrK_OBEY) {
            // Must Obey switches off by being cast again, as the panel button does.
            if (!player_uses_power_obey(plyr_idx)) return "NOT_ACTIVE";
            step_action(&steps[n++], PckA_UsePwrObey, 0, 0);
        } else {
            return "UNSUPPORTED_POWER";
        }
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_UseSpecial: the steps (plan_verb's case, 14 S3). */
static const char *plan_use_special(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // A special box the seat's hand could reach (what a human needs to pick it up and use it). Resurrect and
        // transfer open a chooser for a human; here the choice comes with the order.
        if (!verb->has_thing) return "MISSING_THING";
        struct Thing *box = thing_get((DaiThingIndex)verb->thing_id);
        if (thing_is_invalid(box) || !thing_is_special_box(box)) return "NO_SUCH_THING";
        if (!can_thing_be_picked_up_by_player(box, plyr_idx)) return "CANNOT_REACH";
        const SpecialKind spk = box_thing_to_special(box);
        if (spk == SpcKind_Resurrect) {
            const int64_t model = creature_model_id(verb->name);
            if (model < 1) return "MISSING_CHOICE";
            if ((verb->level < 1) || (verb->level > CREATURE_MAX_LEVEL)) return "BAD_LEVEL";
            TbBool found = false;
            for (int64_t i = 0; (i < dungeon->dead_creatures_count) && (i < DEAD_CREATURES_MAX_COUNT); i++) {
                if ((dungeon->dead_creatures[i].model == model) && (dungeon->dead_creatures[i].exp_level == verb->level - 1)) found = true;
            }
            if (!found) return "NO_SUCH_DEAD";
            step_action(&steps[n++], PckA_ResurrectCrtr, box->index, (plyr_idx & 0x0F) | ((model & 0xFF) << 4) | (((verb->level - 1) & 0x0F) << 12));
        } else if (spk == SpcKind_TrnsfrCrtr) {
            if (!verb->has_target_thing) return "MISSING_CHOICE";
            const struct Thing *cr = creature_thing(verb->target_thing);
            if (cr == NULL) return "NO_SUCH_THING";
            if (cr->owner != plyr_idx) return "NOT_YOURS";
            step_action(&steps[n++], PckA_TransferCreatr, box->index, cr->index);
        } else {
            step_action(&steps[n++], PckA_UseSpecialBox, box->index, 0);
        }
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_SendMessage: the steps (plan_verb's case, 14 S3). */
static const char *plan_send_message(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        // Short messages only, never a game command (a message starting with the command character would run one).
        const size_t len = strlen(verb->message);
        if ((len < 1) || (len > EXTSEAT_CHAT_MAX_LEN)) return "BAD_MESSAGE";
        if (verb->message[0] == cmd_char) return "BAD_MESSAGE";
        for (size_t i = 0; i < len; i++) if (((unsigned char)verb->message[i] < 32) || ((unsigned char)verb->message[i] > 126)) return "BAD_MESSAGE";
        step_action(&steps[n], PckA_PlyrMsgEnd, 0, 0);
        steps[n++].send_chat = true;
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_Sell: the steps (plan_verb's case, 14 S3). */
static const char *plan_sell(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    int64_t n = 0;
    do {
        int64_t x0, y0, x1, y1;
        if (verb->has_rect) {
            x0 = verb->slab_x0; y0 = verb->slab_y0; x1 = verb->slab_x1; y1 = verb->slab_y1;
        } else if (verb->has_pos) {
            x0 = x1 = DaiSubtileSlab(verb->stl_x); y0 = y1 = DaiSubtileSlab(verb->stl_y);
        } else {
            return "MISSING_RECT";
        }
        if (!slab_on_map(x0, y0) || !slab_on_map(x1, y1)) return "POSITION_OFF_MAP";
        const int64_t lx = (x0 < x1) ? x0 : x1, rx = (x0 < x1) ? x1 : x0, ty = (y0 < y1) ? y0 : y1, by = (y0 < y1) ? y1 : y0;
        if ((rx - lx + 1) * (by - ty + 1) > EXTSEAT_MAX_SELL_SLABS) return "AREA_TOO_LARGE";
        // A sell drag skips items in this engine (see ftest_ai_gesture_drag_verbs), so each slab of the area that is
        // the seat's own gets its own click; anything else cannot be sold and is left out.
        int64_t xy[2 * EXTSEAT_MAX_SELL_SLABS];
        int64_t count = 0;
        for (int64_t y = ty; y <= by; y++) {
            for (int64_t x = lx; x <= rx; x++) {
                if (slabmap_owner(get_slabmap_block((DaiSlabCoord)x, (DaiSlabCoord)y)) == plyr_idx) {
                    xy[2 * count] = x;
                    xy[2 * count + 1] = y;
                    count++;
                }
            }
        }
        if (count == 0) return "NOTHING_TO_SELL";
        if (!verb->lose_spells) {
            const char *err = check_sell_keeps_spells(plyr_idx, lx, ty, rx, by);
            if (err != NULL) return err;
        }
        if (s_gesture_packets) {
            // As sell drags of at most MAX_USER_ROOMSPACE_WIDTH a side, each after the engine sold the last.
            const int64_t side = MAX_USER_ROOMSPACE_WIDTH;
            for (int64_t y = ty; y <= by; y += side)
                for (int64_t x = lx; x <= rx; x += side) {
                    if (n >= EXTSEAT_MAX_STEPS) return "AREA_TOO_LARGE";
                    const int64_t ex = (x + side - 1 < rx) ? x + side - 1 : rx, ey = (y + side - 1 < by) ? y + side - 1 : by;
                    step_action(&steps[n], PckA_DaiSellRect, packet_pack_xy(x, y), packet_pack_xy(ex, ey));
                    steps[n++].wait_build_free = true;
                }
            break;
        }
        n = kctl_plan_slab_clicks(steps, EXTSEAT_MAX_STEPS, xy, count);
        if (n == 0) return "AREA_TOO_LARGE";
        break;
    } while (0);
    *out_n = n;
    return NULL;
}

/** ESV_SetAssist: the steps (plan_verb's case, 14 S3). */
static const char *plan_set_assist(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    (void)w; (void)group_orders; (void)plyr_idx; (void)verb; (void)steps; (void)player; (void)dungeon;
    *out_n = 0;
    return "NOT_AVAILABLE"; // the combat assistant is not in this build
}

/** ESV_AssistUseBank: the steps (plan_verb's case, 14 S3). */
static const char *plan_assist_use_bank(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    (void)w; (void)group_orders; (void)plyr_idx; (void)verb; (void)steps; (void)player; (void)dungeon;
    *out_n = 0;
    return "NOT_AVAILABLE"; // the combat assistant is not in this build
}

/** ESV_AssistEnable: the steps (plan_verb's case, 14 S3). */
static const char *plan_assist_enable(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    (void)w; (void)group_orders; (void)plyr_idx; (void)verb; (void)steps; (void)player; (void)dungeon;
    *out_n = 0;
    return "NOT_AVAILABLE"; // the combat assistant is not in this build
}

/** ESV_AssistDelegate: the steps (plan_verb's case, 14 S3). */
static const char *plan_assist_delegate(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n, struct PlayerInfo *player, struct Dungeon *dungeon)
{
    (void)w; (void)group_orders; (void)plyr_idx; (void)verb; (void)steps; (void)player; (void)dungeon;
    *out_n = 0;
    return "NOT_AVAILABLE"; // the combat assistant is not in this build
}

/* The slot plan_verb_for() plans for: its seat is the hand and the tools the plan reads (a co-seat's, co-op). */
static int64_t s_plan_slot = -1;
/** The user state of the slot a verb is planned for: its hand. */
static struct UserState *plan_ustate(void)
{
    return (s_plan_slot >= 0) ? get_user_state(s_plan_slot) : INVALID_USER_STATE;
}

static const char *plan_verb(const struct KeeperCtlWorld *w, TbBool group_orders, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n)
{
    struct PlayerInfo *player = (s_plan_slot >= 0) ? get_packet_seat(s_plan_slot) : INVALID_PLAYER;
    if (player_invalid(player) || (player->id_number != plyr_idx))
        player = get_player(plyr_idx);
    struct Dungeon *dungeon = get_players_dungeon(player);
    switch (verb->kind) {
    case ESV_PlaceTrap:
    case ESV_PlaceDoor:
        return plan_place_trap(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_Slap:
        return plan_slap(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_PickUp:
        return plan_pick_up(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_PickUpAndDrop:
        return plan_pick_up_and_drop(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_GiveGold:
        return plan_give_gold(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_Drop:
        return plan_drop(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_CastPower:
        return plan_cast_power(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_MoveCreature:
    case ESV_Attack:
    case ESV_ReleaseCreature:
        return plan_move_creature(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_SetTendency:
        return plan_set_tendency(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_SetAlliance:
        return plan_set_alliance(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_BuildRoom:
        return plan_build_room(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_MarkDig:
        return plan_mark_dig(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_UnmarkDig:
        return plan_unmark_dig(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_SetDoorLock:
        return plan_set_door_lock(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_PowerOff:
        return plan_power_off(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_UseSpecial:
        return plan_use_special(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_SendMessage:
        return plan_send_message(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_Sell:
        return plan_sell(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_SetAssist:
        return plan_set_assist(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_AssistUseBank:
        return plan_assist_use_bank(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_AssistEnable:
        return plan_assist_enable(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    case ESV_AssistDelegate:
        return plan_assist_delegate(w, group_orders, plyr_idx, verb, steps, out_n, player, dungeon);
    default:
        return "UNKNOWN_VERB";
    }
}

/** The running verb's last step is written: its result is recorded and the slot holds no verb. */
static void retire_finished(int64_t slot)
{
    struct ExtSeatQueue *q = &s_queue[slot];
    if ((q->next >= q->count) && (q->cur_id != 0)) {
        record_running(slot, q->cur_error);
        q->cur_id = 0;
        q->cur_error = NULL;
        q->step_wait = 0;
        q->hand_dropped = 0;
    }
}

/* plan_verb for a slot: its self-contained rules (s_plan_self_contained, self_contained_check) applied. */
static const char *plan_verb_for(int64_t slot, int64_t lane, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb,
    struct ExtSeatStep *steps, int64_t *out_n)
{
    s_plan_self_contained = s_self_contained[slot];
    s_plan_slot = slot;
    const TbBool lt = (lane == EXTSEAT_LANE_LIEUTENANT) && (s_lt_world[slot] != NULL);
    const char *err = plan_verb(lt ? s_lt_world[slot] : slot_world(slot), lt ? s_lt_group_orders[slot] : s_group_orders[slot],
        plyr_idx, verb, steps, out_n);
    s_plan_self_contained = false;
    s_plan_slot = -1;
    if (err == NULL) err = self_contained_check(slot, steps, *out_n);
    return err;
}

/** Starts the next waiting verb if the running gesture is done. Called each turn before a step is written. */
static void promote_verbs(int64_t slot, PlayerNumber plyr)
{
    struct ExtSeatQueue *q = &s_queue[slot];
    retire_finished(slot);
    while ((q->next >= q->count) && (queued_total(q) > 0)) {
        // The urgent lane first, the lieutenant's last.
        const int64_t lane = (q->lane_count[EXTSEAT_LANE_NOW] > 0) ? EXTSEAT_LANE_NOW
                           : (q->lane_count[EXTSEAT_LANE_ROUTINE] > 0) ? EXTSEAT_LANE_ROUTINE : EXTSEAT_LANE_LIEUTENANT;
        const int64_t now_turn = (int64_t)get_gameturn();
        if (assist_verb(q->lane[lane][0].verb.kind) && (q->assist_written > 0) && (now_turn + 1 >= q->assist_written)
            && (now_turn <= q->assist_written)) {
            break; // the assistant packets just written are not processed yet: plan it next turn
        }
        const struct QueuedVerb e = q->lane[lane][0];
        lane_remove(q, lane, 0);
        const struct ExtSeatVerb verb = e.verb;
        const int64_t id = e.id;
        // A verb cut part-way has started: it never expires.
        if ((e.suspended == 0) && (verb.expires_turn > 0) && ((int64_t)get_gameturn() > verb.expires_turn)) {
            record_queued(slot, lane, &e, "EXPIRED");
            continue;
        }
        struct ExtSeatStep steps[EXTSEAT_MAX_STEPS];
        int64_t n = 0;
        const char *err = plan_verb_for(slot, lane, plyr, &verb, steps, &n);
        if (err != NULL) {
            // An assistant change that finds the set already as it wants it (the change ahead made it so) is done.
            const TbBool done = ((e.suspended > 0) && resume_nothing_left(err)) || (assist_verb(verb.kind) && (strcmp(err, "UNCHANGED") == 0));
            record_queued(slot, lane, &e, done ? NULL : err);
            continue;
        }
        memcpy(q->steps, steps, sizeof(struct ExtSeatStep) * n);
        q->count = n;
        q->next = 0;
        q->cur_id = id;
        q->cur_verb = verb;
        q->cur_submitted = e.submitted;
        q->cur_started = (e.started >= 0) ? e.started : (int64_t)get_gameturn();
        q->cur_lane = lane;
        q->cur_suspended = e.suspended;
        q->cur_error = NULL;
        q->step_wait = 0;
        q->hand_dropped = 0;
        note_order_started(slot, plyr, &verb);
    }
}

const char *kctl_submit(int64_t slot, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb, TbBool queue, struct ExtSeatSubmitInfo *info)
{
    return kctl_submit_lane(slot, plyr_idx, verb, queue, verb->priority ? EXTSEAT_LANE_NOW : EXTSEAT_LANE_ROUTINE, info);
}

const char *kctl_submit_lane(int64_t slot, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb, TbBool queue, int64_t lane,
    struct ExtSeatSubmitInfo *info)
{
    if ((lane < 0) || (lane >= EXTSEAT_LANES)) {
        return "BAD_LANE";
    }
    if (!slot_ok(slot)) {
        return "NOT_A_VALID_SLOT";
    }
    struct ExtSeatQueue *q = &s_queue[slot];
    if (verb->kind == ESV_Cancel) {
        // One verb (cancel_id), one lane's waiting verbs (cancel_lane), or everything.
        int64_t dropped = 0;
        if (verb->cancel_id > 0) {
            if (!kctl_cancel_id(slot, verb->cancel_id)) return "NO_SUCH_ORDER";
            dropped = 1;
        } else if ((verb->cancel_lane > 0) && (verb->cancel_lane <= EXTSEAT_LANES)) {
            dropped = kctl_cancel_lane(slot, verb->cancel_lane - 1);
        } else {
            dropped = kctl_cancel(slot);
        }
        if (info != NULL) {
            info->steps = dropped;
            info->id = 0;
            info->queued_behind = 0;
        }
        return NULL;
    }
    if ((verb->expires_turn > 0) && ((int64_t)get_gameturn() > verb->expires_turn)) {
        return "STALE_VIEW";
    }
    // An assistant change while the assistant is about to change waits in its lane, to be planned (and found
    // unchanged or not) against the set it will meet -- never against the one that is going.
    const TbBool settle = assist_verb(verb->kind) && assist_settling(q);
    const TbBool busy = !kctl_idle(slot) || settle;
    // An urgent verb always queues: it is meant to go ahead of whatever runs. An agent's verb is refused only for the
    // agent's own work: the lieutenant's is cut for it, or waited out (10 section 7.1).
    const TbBool own_busy = (lane == EXTSEAT_LANE_LIEUTENANT) ? busy : (agent_busy(q) || settle);
    if (own_busy && !settle && !queue && (lane == EXTSEAT_LANE_ROUTINE)) {
        return "ACTION_ALREADY_QUEUED";
    }
    if (busy && (q->lane_count[lane] >= EXTSEAT_MAX_QUEUED_VERBS)) {
        return "QUEUE_FULL";
    }
    struct ExtSeatStep steps[EXTSEAT_MAX_STEPS];
    int64_t n = 0;
    const char *err = plan_verb_for(slot, lane, plyr_idx, verb, steps, &n);
    if (settle && (err != NULL) && (strcmp(err, "UNCHANGED") == 0)) err = NULL; // planned again when it starts
    if (err == NULL) err = check_chat_rate(slot, verb); // a bad message says so whenever it is sent
    if (err != NULL) {
        return err;
    }
    const int64_t id = ++q->next_id;
    if (verb->kind == ESV_SendMessage) q->last_chat_turn = (int64_t)get_gameturn(); // counts from acceptance: a batch cannot spam
    int64_t behind = 0;
    if (busy) {
        struct QueuedVerb e;
        memset(&e, 0, sizeof(e));
        e.verb = *verb;
        e.id = id;
        e.submitted = (int64_t)get_gameturn();
        e.started = -1;
        // Ahead of it: the urgent verbs already waiting, and the running verb unless it will be cut for this one (or
        // has only clean-up steps left); a routine verb also waits for every routine verb before it.
        const TbBool running = (q->next < q->count);
        const TbBool cuttable = running && (q->cur_id != 0) && kctl_verb_cuttable(q->cur_verb.kind);
        const TbBool cut = cuttable && (((lane == EXTSEAT_LANE_NOW) && (q->cur_lane == EXTSEAT_LANE_ROUTINE))
            || ((lane != EXTSEAT_LANE_LIEUTENANT) && (q->cur_lane == EXTSEAT_LANE_LIEUTENANT)));
        behind = q->lane_count[EXTSEAT_LANE_NOW] + ((running && !cut) ? 1 : 0)
            + ((lane != EXTSEAT_LANE_NOW) ? q->lane_count[EXTSEAT_LANE_ROUTINE] : 0)
            + ((lane == EXTSEAT_LANE_LIEUTENANT) ? q->lane_count[EXTSEAT_LANE_LIEUTENANT] : 0);
        lane_push(q, lane, &e, false);
    } else {
        // Idle once its last step is written, the slot may still hold that verb until the next tick records it:
        // record it now, or the new verb would take its place and its result would never come.
        retire_finished(slot);
        if (!kctl_enqueue(slot, steps, n)) {
            return "ACTION_ALREADY_QUEUED";
        }
        q->cur_id = id;
        q->cur_verb = *verb;
        q->cur_submitted = q->cur_started = (int64_t)get_gameturn();
        q->cur_lane = lane;
        q->cur_suspended = 0;
        q->cur_error = NULL;
        q->step_wait = 0;
        q->hand_dropped = 0;
        note_order_started(slot, plyr_idx, verb);
    }
    if (info != NULL) {
        info->steps = n;
        info->id = id;
        info->queued_behind = behind;
    }
    return NULL;
}

const char *kctl_check(int64_t slot, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb, int64_t *out_steps)
{
    return kctl_check_lane(slot, plyr_idx, verb, verb->priority ? EXTSEAT_LANE_NOW : EXTSEAT_LANE_ROUTINE, out_steps);
}

const char *kctl_check_lane(int64_t slot, PlayerNumber plyr_idx, const struct ExtSeatVerb *verb, int64_t lane, int64_t *out_steps)
{
    if (!slot_ok(slot)) {
        return "NOT_A_VALID_SLOT";
    }
    if (verb->kind == ESV_Cancel) {
        if (out_steps != NULL) {
            *out_steps = 0;
        }
        return NULL; // a dry-run cancel is trivially "fine": the real one always succeeds, never mutates anything to check
    }
    if ((verb->expires_turn > 0) && ((int64_t)get_gameturn() > verb->expires_turn)) {
        return "STALE_VIEW";
    }
    struct ExtSeatStep steps[EXTSEAT_MAX_STEPS];
    int64_t n = 0;
    const char *err = plan_verb_for(slot, lane, plyr_idx, verb, steps, &n);
    if (err == NULL) err = check_chat_rate(slot, verb); // a bad message says so whenever it is sent
    if (err != NULL) {
        return err;
    }
    if (out_steps != NULL) {
        *out_steps = n;
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif
