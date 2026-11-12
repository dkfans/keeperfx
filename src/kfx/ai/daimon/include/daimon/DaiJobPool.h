/** @file DaiJobPool.h
 *  A keeper's jobs (02 section 4.6, 01 section 8.2): DAI_MAX_JOBS values in slot order. Every state change goes
 *  through SetState() (one DAI: line each); the stall rule is CheckStalls(): a Ready or Running job whose progress has
 *  not changed for its stall window is Blocked, its claims released, and its owning manager decides what next.
 *  Nothing else can hold a job open (lesson L2). */
#ifndef DAIMON_DAIJOBPOOL_H
#define DAIMON_DAIJOBPOOL_H

#include "daimon_base_types.h"
#include <cstdint>

#include "globals.h"
#include "daimon/DaiJob.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiPool.h"

class DaiLedger;

class DaiJobPool {
public:
    /** The keeper the jobs belong to (their log lines). */
    void SetPlayer(PlayerNumber p) { m_player = p; }
    /** A new Ready job; nullptr if the pool is full. */
    DaiJob *Add(DaiJobKindId kind, DaiDomain domain, uint64_t key, DaiFix utility, DaiTurn now, DaiTurn stall_window);
    /** The job of this kind and key that has not finished (Ready, Running or Blocked), to refresh rather than add. */
    DaiJob *Find(DaiJobKindId kind, uint64_t key);
    DaiJob *Get(uint32_t id) { return m_jobs.Find(id); }
    const DaiJob *Get(uint32_t id) const { return m_jobs.Find(id); }
    /** The job whose controller verb this is, or nullptr. */
    DaiJob *FindByVerb(int64_t verb_id);
    void SetState(DaiJob &j, DaiJobState s, const char *why, DaiTurn now);
    /** The kind's progress metric this turn: a change restarts the stall window. */
    void NoteProgress(DaiJob &j, int64_t progress, DaiTurn now);
    /** The stall rule. Blocked jobs' claims are released; the controller verbs they had in flight are written to
     *  `cancel` (at most `max`) for the caller to cancel. Returns how many verbs that is. */
    /** `held(ctx, job)`: a job the mode holds (an advised domain's proposal, P8.5) never stalls -- it has not been let run. */
    int64_t CheckStalls(DaiTurn now, DaiLedger &ledger, int64_t *cancel, int64_t max,
                        bool (*held)(const void *ctx, const DaiJob &j) = nullptr, const void *ctx = nullptr);
    /** Drops finished jobs their manager has seen. */
    void Reap();
    void Clear() { m_jobs.Clear(); }
    int64_t Count() const { return m_jobs.Count(); }
    template <typename F> void ForEach(F f) { m_jobs.ForEach(f); }
    template <typename F> void ForEach(F f) const { m_jobs.ForEach(f); }
private:
    friend class DaiSave;
    PlayerNumber m_player = -1;
    DaiPool<DaiJob, DAI_MAX_JOBS> m_jobs;
};

#endif
