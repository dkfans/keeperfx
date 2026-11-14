/** @file DaiArbiter.h
 *  The next gesture (02 section 4.10, 01 section 8.4): at most one a turn, and none while the slot's controller is
 *  busy. Among the domains with a runnable job (Ready, its retry time passed, its claims satisfiable, its domain
 *  allowed by the mode) the one to run is chosen by smooth weighted round robin -- each gains its weight in credit,
 *  the chosen one pays the credit of all of them per gesture of cost -- except that when the domains' waits come
 *  close to quota_window, the one that has waited longest runs (earliest deadline first), so none waits longer. Within the domain, the highest utility plus age bonus wins. An attempt counts
 *  whether or not the gesture is accepted, so a job that keeps failing cannot starve the others. */
#ifndef DAIMON_DAIARBITER_H
#define DAIMON_DAIARBITER_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>
#include <deque>
#include <span>

#include "daimon/DaiBlob.h"
#include "daimon/DaiJobPool.h"
#include "daimon/DaiLedger.h"
#include "daimon/DaiTurnCtx.h"
#include "daimon/IDaiActor.h"
#include "daimon/IDaiJobKind.h"

/** The turns of the world gestures made in the last DAI_PACE_WINDOW turns (05 section 7.2): the difficulty's pace cap
 *  is how many it may hold. */
constexpr DaiTurn DAI_PACE_WINDOW = 1000;
class DaiPaceWindow {
public:
    /** Gestures made in (now - DAI_PACE_WINDOW, now]; the older ones are forgotten. */
    int64_t Count(DaiTurn now);
    void Note(DaiTurn now) { m_turns.push_back(now); }
    void Reset() { m_turns.clear(); }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    std::deque<DaiTurn> m_turns;
};

class DaiArbiter {
public:
    /** The job a gesture was attempted for this turn (Running if it was submitted, else Failed / Done / still Ready),
     *  or nullptr. `kinds` is indexed by DaiJobKindId. */
    DaiJob *Step(const DaiTurnCtx &c, DaiJobPool &jobs, DaiLedger &ledger, IDaiActor &act, int64_t slot,
                 std::span<const IDaiJobKind *const> kinds);
    void Reset();
    int64_t Credit(DaiDomain d) const { return m_credit[(size_t)d]; }
    /** Attempts since the domain last ran while it had runnable work. */
    int64_t Waited(DaiDomain d) const { return m_waited[(size_t)d]; }
    int64_t Attempts() const { return m_attempts; }
    int64_t Gestures() const { return m_gestures; }
    /** Gestures in the pace window now (05 section 7.2). */
    int64_t PaceCount(DaiTurn now) { return m_pace.Count(now); }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
    /** Utility plus the age bonus: what decides within a domain. */
    static DaiFix Score(const DaiTurnCtx &c, const DaiJob &j);
private:
    std::array<int64_t, (size_t)DaiDomain::Count> m_credit{};
    std::array<int64_t, (size_t)DaiDomain::Count> m_waited{};
    int64_t m_attempts = 0, m_gestures = 0;
    DaiPaceWindow m_pace;
};

#endif
