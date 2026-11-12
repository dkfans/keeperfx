// What the dig, bridge and area kinds share (03 section 7.3): how a slab on a planned path stands now, the straight
// run of slabs one gesture handles, and the verbs.
#ifndef DAIMON_JOBS_JOBPATH_H
#define DAIMON_JOBS_JOBPATH_H

#include <algorithm>
#include <span>

#include "globals.h"
#include "keeper_ctl_types.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiTurnCtx.h"
#include "daimon/DaiTypes.h"

/** What a path slab needs now: nothing (walked over), digging, a bridge, or it cannot be passed (re-plan). */
enum class JobStep : uint8_t { Walk, Dig, Bridge, Blocked };

/** Earth, gold, dense gold or unknown ground: the imps dig it (a mark takes it). */
bool JobDiggable(const DaiSlabInfo &s);
/** ...or the keeper's own fortified wall, which a mark of its takes too (a room's site and entrance may cover it). */
bool JobDiggableBy(const DaiSlabInfo &s, PlayerNumber p);
/** The keeper's own solid land: its claimed floor, rooms (bridges among them) and doors -- where claiming spreads from
 *  and a bridge may start. */
bool JobOwnLand(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y);
/** A bridge of the keeper's on the slab. */
bool JobBridged(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y);
/** What slab (x, y) needs, on a route of this kind (claim: water must be bridged). */
JobStep JobStepAt(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, bool claim);
/** The straight run of path slabs from `from` (included) while each next slab goes on in the first step's direction
 *  and `take(i)` says so, at most `max` long; its rectangle. Returns the run's length (0 if `take(from)` is false). */
template <typename F> int64_t JobRun(std::span<const DaiSlabXY> path, int64_t from, int64_t max, F take, DaiRect *rect)
{
    if ((from < 0) || (from >= (int64_t)path.size()) || (max <= 0) || !take(from)) return 0;
    int64_t n = 1;
    const DaiSlabCoord dx = (from + 1 < (int64_t)path.size()) ? path[(size_t)from + 1].x - path[(size_t)from].x : 0;
    const DaiSlabCoord dy = (from + 1 < (int64_t)path.size()) ? path[(size_t)from + 1].y - path[(size_t)from].y : 0;
    const bool step = (dx == 0) != (dy == 0) && (dx >= -1) && (dx <= 1) && (dy >= -1) && (dy <= 1);   // one slab N, E, S or W
    while (step && (n < max) && (from + n < (int64_t)path.size())) {
        const DaiSlabXY &a = path[(size_t)(from + n - 1)], &b = path[(size_t)(from + n)];
        if ((b.x - a.x != dx) || (b.y - a.y != dy) || !take(from + n)) break;
        n++;
    }
    const DaiSlabXY &s = path[(size_t)from], &e = path[(size_t)(from + n - 1)];
    *rect = DaiRect{ std::min(s.x, e.x), std::min(s.y, e.y), std::max(s.x, e.x), std::max(s.y, e.y) };
    return n;
}
void JobMarkDigVerb(KeeperVerb *v, const DaiRect &r);
/** The same marks asked for by this many gestures running: the game is not taking them; the job waits JOB_REMARK_WAIT. */
constexpr int32_t JOB_REMARK_LIMIT = 3;
constexpr DaiTurn JOB_REMARK_WAIT = 200;
inline bool JobSameRect(const DaiRect &a, const DaiRect &b)
{
    return (a.x0 == b.x0) && (a.y0 == b.y0) && (a.x1 == b.x1) && (a.y1 == b.y1);
}
/** A job whose next gesture would mark `r` (Tick): counts the gestures that went since the last call -- each asked
 *  for what was pending then -- and makes the job wait when the same marks were asked for JOB_REMARK_LIMIT times
 *  running and are wanted again. Waiting on the arbiter is not asking: only gestures count. */
inline bool JobMarkRepeated(DaiMarkRepeat &m, const DaiRect &r, DaiJob &j, DaiTurn now)
{
    if (j.gestures != m.seen_gestures) {
        m.repeats = JobSameRect(m.pending, m.last) ? m.repeats + 1 : 1;
        m.last = m.pending;
        m.seen_gestures = j.gestures;
    }
    m.pending = r;
    if ((m.repeats < JOB_REMARK_LIMIT) || !JobSameRect(r, m.last)) return false;
    m.repeats = 0;
    j.retry_after = now + JOB_REMARK_WAIT;
    return true;
}
inline void JobSaveMarks(DaiBlobWriter &w, const DaiMarkRepeat &m)
{
    for (const DaiRect *r : { &m.pending, &m.last }) { w.I64(r->x0); w.I64(r->y0); w.I64(r->x1); w.I64(r->y1); }
    w.I64(m.repeats);
    w.I64(m.seen_gestures);
}
inline void JobLoadMarks(DaiBlobReader &r, DaiMarkRepeat *m)
{
    for (DaiRect *q : { &m->pending, &m->last }) {
        q->x0 = (DaiSlabCoord)r.Range(0, 255); q->y0 = (DaiSlabCoord)r.Range(0, 255);
        q->x1 = (DaiSlabCoord)r.Range(0, 255); q->y1 = (DaiSlabCoord)r.Range(0, 255);
    }
    m->repeats = (int32_t)r.Range(0, JOB_REMARK_LIMIT);
    m->seen_gestures = r.Range(0, INT64_MAX);
}
/** Excavation's share of the keeper's marks is used up ([excavation] marks_share of DAI_MAX_MARKS, 03 section 8.2):
 *  a marking gesture waits. The marks are counted from the world, not from Daimon's bookkeeping. */
inline bool JobMarksFull(const DaiTurnCtx &c)
{
    return c.know.OwnMarkCount() >= FixToInt(FixMul(FixFromInt(DAI_MAX_MARKS), c.cfg.excavation_marks_share));
}
void JobBuildRoomVerb(KeeperVerb *v, DaiRoom room, const DaiRect &r);
/** Slabs in a rectangle. */
/** A fetch (P5.12, dig_route; room builds since the refactor-renderer merge): a slab of the job's marked and left undug
 *  (or dug and left unclaimed) this long since the job's last progress and its last fetch gets an imp dropped on the
 *  keeper's land beside it -- a dropped imp takes the work where it lands; a fetch stands this long; the land it is
 *  dropped on is at most this far from the slab. */
constexpr DaiTurn JOB_DROP_AFTER = 300, JOB_DROP_TURNS = 60;
constexpr int64_t JOB_DROP_REACH = 8;
/** The keeper's land to drop an imp on for slab `a`: `before` (a route's slab before it) if given and the keeper's,
 *  else the nearest within JOB_DROP_REACH. */
bool JobDropSite(const DaiKnowledge &k, PlayerNumber p, const DaiSlabXY *before, const DaiSlabXY &a, DaiSlabXY *out);
/** The imp a fetch drops: one carrying no gold and not at work, else one carrying none, else any; the nearest to `at`. */
const DaiImpLook *JobDropImp(const DaiKnowledge &k, const DaiSlabXY &at);
inline int64_t JobRectSlabs(const DaiRect &r) { return (int64_t)(r.x1 - r.x0 + 1) * (int64_t)(r.y1 - r.y0 + 1); }
/** Bridge gesture: the run of unbridged liquid path slabs from `from`, if the slab before it is the keeper's land
 *  (a bridge grows only from own land). Its length, 0 if it cannot start now. */
int64_t JobBridgeRun(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> path, int64_t from, bool claim,
                     bool first_from_any_side, DaiRect *rect);

/** At most this many bridge slabs a gesture (the engine builds one a turn; a long line holds the gold for long). */
constexpr int64_t JOB_BRIDGE_RUN_MAX = 8;

#endif
