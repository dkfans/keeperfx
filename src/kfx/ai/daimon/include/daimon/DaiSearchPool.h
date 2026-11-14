/** @file DaiSearchPool.h
 *  A keeper's dig searches in progress (03 section 7.2), each held by the job that runs it: at most
 *  [budgets] max_searches at once (others wait for a free one), their memory kept between searches. Not saved: a job
 *  starts its search again after a load. */
#ifndef DAIMON_DAISEARCHPOOL_H
#define DAIMON_DAISEARCHPOOL_H

#include <array>
#include <cstdint>

#include "daimon/DaiDigSearch.h"
#include "daimon/DaiLimits.h"

/** Search keys at or above this belong to managers (their own probes), below it to jobs (their ids). */
constexpr uint32_t DAI_SEARCH_KEY_MANAGER = 0xFFFF0000u;

class DaiSearchPool {
public:
    /** The job's search, or a free one for it while fewer than `limit` are held; nullptr if none. `fresh` is set
     *  when the search is new to the job (it must be started). */
    DaiDigSearch *Get(uint32_t job, int64_t limit, bool *fresh);
    /** The job's search, or nullptr. */
    DaiDigSearch *Find(uint32_t job);
    void Release(uint32_t job);
    void ReleaseAll() { for (Slot &s : m_slots) s.job = 0; }
    /** Releases every job's search that `keep(job)` says is no longer searching (managers release their own). */
    template <typename F> void Sweep(F keep)
    {
        for (Slot &s : m_slots) if ((s.job != 0) && (s.job < DAI_SEARCH_KEY_MANAGER) && !keep(s.job)) s.job = 0;
    }
    int64_t Held() const;
    /** The held searches in a saved game (each with its key), so they go on after a load where they were. */
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    struct Slot {
        uint32_t job = 0;
        DaiDigSearch search;
    };
    std::array<Slot, DAI_MAX_SEARCHES> m_slots;
};

#endif
