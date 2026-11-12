/** @file IDaiJobKind.h
 *  A kind of job (02 section 4.6): the behaviour behind DaiJob's data -- its gesture, its progress metric, what its
 *  results mean. Kinds are stateless (no members), so one table serves every keeper and, in Phase 10, every worker
 *  thread. One kind per file under src/jobs/. Save and Load join with P0.7. */
#ifndef DAIMON_IDAIJOBKIND_H
#define DAIMON_IDAIJOBKIND_H

#include <cstdint>
#include <span>

#include "globals.h"
#include "keeper_ctl_types.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiJob.h"
#include "daimon/DaiTurnCtx.h"
#include "daimon/DaiTypes.h"
#include "daimon/IDaiActor.h"

class IDaiJobKind {
public:
    virtual ~IDaiJobKind() = default;
    /** "create_imp", "dig_route": logs, the save, the console. */
    virtual const char *GetName() const = 0;
    virtual DaiDomain GetDomain() const = 0;
    /** The least stall window a job of this kind gets (0: [jobs] stall_window as it is). */
    virtual DaiTurn MinStallWindow() const { return 0; }
    /** Arbiter credits one gesture of this kind costs (02 section 4.10). */
    virtual int64_t GestureCost() const { return 1; }
    /** Once a turn, before Achieved and Progress, for a job that is Ready, Running or Blocked: the kind's thinking
     *  that changes the job -- a search stepped on (c.searches, c.budget), a path kept, the next gesture's needs
     *  (j.needs, claimed before NextVerb) or when to try again (j.retry_after). The state it returns is the job's
     *  (Failed with the reason in why ends it); by default nothing changes. */
    virtual DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const
    {
        (void)c; (void)why; (void)why_len;
        return j.state;
    }
    /** Slabs the job waits on (a mark to be dug, a bank to be claimed): the knowledge reads them again at the next
     *  update, outside its scan budget, so the job sees them change the turn after, not when the scan comes round.
     *  At most `max`; how many were written. */
    virtual int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const
    {
        (void)c; (void)j; (void)out; (void)max;
        return 0;
    }
    /** The next gesture: Ok with *verb filled, Done (nothing left to do), Wait (not now), or Fail / BadInput with
     *  the reason in why (the job fails). */
    virtual DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const = 0;
    /** The job's monotone progress metric (imps owned, slabs dug): no change for the stall window blocks the job. */
    virtual int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const = 0;
    /** Whether what the job is for has happened (the job is then Done, whatever its gesture is doing). */
    virtual bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const = 0;
    /** The state a gesture's result leaves the job in. By default a gesture that ran finishes the job; a kind whose
     *  effect shows later (Achieved) stays Running, one with more gestures goes back to Ready. */
    virtual DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Done;
    }
    /** The kind's parameters (DaiJob::p) in a saved game; Load sets them, false on anything out of range. */
    /** Its parameters' format in the save (inside JOBS): a job whose saved version differs is dropped, with its claims;
     *  the others are read. Bump it when SaveParams' layout changes (14 F1). */
    virtual uint16_t ParamsVersion() const { return 1; }
    virtual void SaveParams(const DaiJob &j, DaiBlobWriter &w) const { (void)j; (void)w; }
    virtual bool LoadParams(DaiJob &j, DaiBlobReader &r) const { (void)r; j.p = std::monostate{}; return true; }
};

/** The game's job kinds, indexed by DaiJobKindId (entry 0 is nullptr). */
std::span<const IDaiJobKind *const> DaiJobKinds();
/** A kind of the game's table by id, nullptr for none or out of range; its name, `none` then (14 K10). */
inline const IDaiJobKind *DaiJobKindOf(DaiJobKindId id)
{
    const auto kinds = DaiJobKinds();
    return (id < kinds.size()) ? kinds[id] : nullptr;
}
inline const char *DaiJobKindName(DaiJobKindId id, const char *none = "?")
{
    const IDaiJobKind *k = DaiJobKindOf(id);
    return (k != nullptr) ? k->GetName() : none;
}

#endif
