/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_facts.h
 *     The keeper-facts tracker: what changed for a keeper since the last look -- events, losses, intruders, traps
 *     firing, custody, sacrifices, Sight of Evil, the pay day, the objective, victory -- read from the game as a keeper
 *     in that seat would notice it (keeper_visibility.h). Shared by the External seat (api_seat_decision.c turns facts
 *     into its decision reasons) and Daimon (its inbox); docs/plans/ai-plans/v2/01-phase0-foundations.md P0.4.
 *
 *     Caller-owned, one per keeper per consumer: two consumers never share or double a baseline. Read-only on the
 *     simulation. Session state: after a load or a rewind, keeper_facts_rebaseline() and the next update raises nothing.
 */
/******************************************************************************/
#ifndef DK_KEEPER_FACTS_H
#define DK_KEEPER_FACTS_H

#include "globals.h"
#include "daimon_base_types.h"
#include "keeper_facts_types.h"
#include "keeper_assist_rules.h"
#include "map_events.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEEPER_FACTS_MODEL_BITS 64
#define KEEPER_FACTS_CUSTODY 64
/** Creatures going into the keeper's temple pool at once, at most (more: the rest not followed). */
#define KEEPER_FACTS_SINKING 16
#define KEEPER_FACTS_OPEN 64
#define KEEPER_FACTS_INTRUDERS_LISTED 16
#define KEEPER_FACTS_INTRUDERS_REMEMBERED 64
/** An intruder not seen on the keeper's ground for this many turns is news again when it comes back. */
#define KEEPER_FACTS_INTRUDER_FORGET_TURNS 1200
#define KEEPER_FACTS_TRAPS 256
/** Most facts one update reports; more are dropped (and counted). */
#define KEEPER_FACTS_MAX 512

/** A trap's state between two looks: what tells that it fired. */
struct KeeperTrapLook {
    int64_t shots;
    int64_t rearm_turn;
    uint8_t revealed;
};

struct KeeperFactsPrisoner {
    DaiThingIndex id;
    DaiTurn creation_turn;
    TbBool hungry;
    TbBool hurt;
    TbBool on_rack;
    int64_t torture_points; /**< 0 when not on the rack */
};

struct KeeperFactsIntruderSeen {
    DaiThingIndex id;
    DaiTurn born;
    int64_t last_seen;
};

/** An enemy creature on the keeper's ground at the last scan. */
struct KeeperFactsIntruder {
    DaiThingIndex id;
    DaiThingModel model;
    PlayerNumber owner;
    int level;
    DaiSubtlCoord x, y;
};

/** A creature going into the keeper's pool at the last look: gone at the next, it was sacrificed. */
struct KeeperFactsSinking {
    DaiThingIndex id;
    DaiTurn born;
    DaiThingModel model;
};

struct KeeperFactsTrap {
    DaiThingIndex id;
    DaiTurn born;
    DaiThingModel model;
    DaiSubtlCoord x, y;
    struct KeeperTrapLook look;
};

struct KeeperFactsTracker {
    PlayerNumber plyr;
    TbBool baselined;
    int64_t last_quarter;
    TbBool event_seen[EVENTS_COUNT];
    unsigned char event_kind[EVENTS_COUNT];
    TbBool kind_owned[KEEPER_FACTS_MODEL_BITS];
    unsigned char last_victory;
    char objective[MESSAGE_TEXT_LEN];
    struct KeeperFactsPrisoner custody[KEEPER_FACTS_CUSTODY];
    int custody_count;
    int64_t bodies_toward_vampire;
    unsigned char offered[CREATURE_TYPES_MAX];
    struct KeeperFactsSinking sinking[KEEPER_FACTS_SINKING];
    int sinking_count;
    int64_t open_xy[2 * KEEPER_FACTS_OPEN];
    int64_t open_count;
    uint8_t assist_refusal[KEEPER_ASSIST_RULES];
    uint64_t assist_log_seen;
    /** Sight of Evil: the cast being watched, how far it has opened, for how many turns it has not grown. */
    DaiThingIndex sight_eye;
    DaiTurn sight_born;
    int64_t sight_open, sight_still, sight_turn;
    TbBool sight_told;
    /** The map as last seen, one entry per slab (map_tiles_x * map_tiles_y, heap): owned, marked to dig, room kind. */
    uint16_t *slabs;
    unsigned char *marked;
    int64_t slabs_w, slabs_h;
    int64_t scan_turn;
    struct KeeperFactsIntruderSeen remembered[KEEPER_FACTS_INTRUDERS_REMEMBERED];
    int remembered_count;
    struct KeeperFactsIntruder intruders[KEEPER_FACTS_INTRUDERS_LISTED];
    int intruder_count;
    int64_t intruder_total;
    struct KeeperFactsTrap traps[KEEPER_FACTS_TRAPS];
    int trap_count;
    struct KeeperFactsTrap traps_next[KEEPER_FACTS_TRAPS]; /**< scratch for a scan */
    TbBool traps_matched[KEEPER_FACTS_TRAPS];
    int64_t dropped;        /**< facts that did not fit an update's output */
};

/** Sets the tracker up for `plyr_idx`; the first update takes the baseline and reports nothing. */
void keeper_facts_init(struct KeeperFactsTracker *t, PlayerNumber plyr_idx);
/** Frees what init and the updates allocated. */
void keeper_facts_free(struct KeeperFactsTracker *t);
/** The next update takes a new baseline and reports nothing (after a load or a rewind). */
void keeper_facts_rebaseline(struct KeeperFactsTracker *t);
/** Looks at the game: writes what changed since the last update into out (at most max) and returns how many. May be
 *  called more than once a turn (the scans of the map, intruders and traps run once per game turn). */
int64_t keeper_facts_update(struct KeeperFactsTracker *t, DaiTurn now, struct KeeperFact *out, int64_t max);
/** The level's objective text as last seen ("" if none). */
const char *keeper_facts_objective(const struct KeeperFactsTracker *t);
/** Whether a trap went off between two looks at it: its next-fire turn moved on, a limited charge was spent, or a hidden
 *  trap revealed itself. */
TbBool keeper_facts_trap_fired(const struct KeeperTrapLook *was, const struct KeeperTrapLook *now);

#ifdef __cplusplus
}
#endif

#endif
