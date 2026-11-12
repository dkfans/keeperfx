/** @file DaiProf.h
 *  A keeper's profile counters (02 section 8): turn time and the work its turns did, over the current window (since
 *  the last summary line) and in total; and over the whole game, each stage of the turn (12 section 13: what would
 *  stay on the main thread and what a worker would take) with a histogram for its percentiles. Times are wall clock,
 *  for the log and tests only -- no decision reads them. */
#ifndef DAIMON_DAIPROF_H
#define DAIMON_DAIPROF_H

#include <array>
#include <chrono>
#include <cstdint>

#include "daimon/DaiTypes.h"

struct DaiProfCounts {
    int64_t turns = 0;
    int64_t ns_total = 0, ns_max = 0;
    int64_t slabs_scanned = 0;
    int64_t facts = 0;
    int64_t gestures = 0;
};

/** The stages of a turn, in order (12 section 13.5): the main thread's apply and gather, then the planning turn's,
 *  one per manager after Director. */
enum class DaiProfStage : uint8_t {
    Apply, Facts, Read, Derive, Copy, Feed, Threat, Rivals, Results, Director,
    Manager, Arbiter = Manager + (uint8_t)DaiDomain::Count, Watch, Tail, Count
};
const char *DaiProfStageName(DaiProfStage s);

/** One stage over the game: its time per turn, in buckets of an eighth of an octave from DAI_PROF_MIN_NS. */
constexpr int64_t DAI_PROF_MIN_NS = 64;
constexpr int64_t DAI_PROF_BUCKETS = 8 * 26;
struct DaiProfStageCounts {
    int64_t ns_total = 0, ns_max = 0;
    std::array<int32_t, DAI_PROF_BUCKETS> hist{};
    /** The time below which `per_mille` of the turns fell (a bucket's upper bound). */
    int64_t Percentile(int64_t turns, int64_t per_mille) const;
};

/** One turn's stage times, kept by the thread running the stages (the gather's travel with its snapshot to the
 *  planning turn, which adds its own). */
struct DaiStageClock {
    std::chrono::steady_clock::time_point mark = std::chrono::steady_clock::now();
    std::array<int64_t, (size_t)DaiProfStage::Count> ns{};
    /** The clock starts again without a stage (a worker picking the turn up: its wait is not the keeper's). */
    void Start() { mark = std::chrono::steady_clock::now(); }
    /** The stage that ran since the last mark (or Start). */
    void Mark(DaiProfStage s)
    {
        const auto now = std::chrono::steady_clock::now();
        ns[(size_t)s] += (int64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(now - mark).count();
        mark = now;
    }
};

class DaiProf {
public:
    /** One turn done, its stages timed by `clock` (what ran since its last mark is Tail): its time -- the stages' sum,
     *  whichever threads ran them -- and work go into the window and the total. */
    void End(DaiStageClock &clock, int64_t slabs, int64_t facts, int64_t gestures);
    const DaiProfCounts &Window() const { return m_window; }
    const DaiProfCounts &Total() const { return m_total; }
    const DaiProfStageCounts &Stage(DaiProfStage s) const { return m_stages[(size_t)s]; }
    /** The whole turn's own histogram, as a stage's. */
    const DaiProfStageCounts &Turn() const { return m_turn_counts; }
    void NewWindow() { m_window = DaiProfCounts{}; }
private:
    DaiProfCounts m_window, m_total;
    std::array<DaiProfStageCounts, (size_t)DaiProfStage::Count> m_stages{};
    DaiProfStageCounts m_turn_counts;
};

#endif
