#include "pre_inc.h"
#include "daimon/DaiArbiter.h"
#include "daimon/DaiDifficulty.h"

#include <algorithm>
#include <cinttypes>

#include "daimon/DaiLog.h"
#include "post_inc.h"

static_assert((int64_t)DaiDomain::Count == DAIMON_DOMAINS, "config_daimon.h's DAIMON_DOMAINS must be DaiDomain::Count");

namespace {
const IDaiJobKind *kind_of(std::span<const IDaiJobKind *const> kinds, DaiJobKindId k)
{
    return (k < kinds.size()) ? kinds[k] : nullptr;
}

bool runnable(const DaiTurnCtx &c, const DaiJob &j, std::span<const IDaiJobKind *const> kinds)
{
    if ((j.state != DaiJobState::Ready) || (j.retry_after > c.now) || (j.domain >= DaiDomain::Count)) return false;
    if (!c.mode.DomainActive(j.domain) || (kind_of(kinds, j.kind) == nullptr)) return false;
    if (c.mode.HoldsJob(j)) return false;   // a proposal the agent has not accepted (advise mode)
    if (j.needs.gold > c.ledger.GoldUnclaimed(c.know.Own().gold)) return false;
    if (j.needs.hand && !c.ledger.HandFree()) return false;
    if ((j.needs.thing.idx != 0) && !c.ledger.ThingFree(j.needs.thing)) return false;
    return true;
}

/** Takes the job's claims; false (none kept) if one cannot be had. */
bool take_claims(const DaiTurnCtx &c, DaiLedger &ledger, const DaiJob &j)
{
    // A claim outlives a lost result by at most the stall window (the stall rule releases it sooner).
    const DaiTurn expires = c.now + ((j.stall_window > 0) ? j.stall_window : 1);
    bool ok = true;
    if (j.needs.gold > 0) ok &= ledger.ClaimGold(j.id, j.needs.gold, c.know.Own().gold, expires) != 0;
    if (ok && j.needs.hand) ok &= ledger.ClaimHand(j.id, expires) != 0;
    if (ok && (j.needs.thing.idx != 0)) ok &= ledger.ClaimThing(DaiClaimOwner::Job, j.id, j.needs.thing, expires) != 0;
    if (!ok) ledger.ReleaseJob(j.id);
    return ok;
}
}

DaiFix DaiArbiter::Score(const DaiTurnCtx &c, const DaiJob &j)
{
    const int64_t age = (c.now > j.created) ? (int64_t)(c.now - j.created) : 0;
    return j.utility + (c.cfg.arbiter_age_bonus_per_kturn * age) / 1000;
}

void DaiArbiter::Reset()
{
    m_credit.fill(0);
    m_waited.fill(0);
    m_pace.Reset();
}

DaiJob *DaiArbiter::Step(const DaiTurnCtx &c, DaiJobPool &jobs, DaiLedger &ledger, IDaiActor &act, int64_t slot,
                         std::span<const IDaiJobKind *const> kinds)
{
    if (!act.Idle(slot)) return nullptr;
    // The pace cap (05 section 7.2): with the window full no attempt is made, so no domain's wait or credit moves --
    // except that Tactics may borrow up to [tactics] burst_turns gestures past it (06 section 10): fights come in
    // bursts, and the window holding them keeps the other domains waiting the longer afterwards.
    const int64_t paced = m_pace.Count(c.now), cap = c.difficulty.PaceCap(c.cfg);
    const bool borrowing = (paced >= cap);
    if (borrowing && (paced >= cap + c.cfg.tactics_burst_turns)) return nullptr;
    // The best runnable job of each domain.
    std::array<DaiJob *, (size_t)DaiDomain::Count> best{}, second{};
    auto better = [&](const DaiJob &x, const DaiJob *y) {
        if (y == nullptr) return true;
        const DaiFix sx = Score(c, x), sy = Score(c, *y);
        return (sx > sy) || ((sx == sy) && (x.id < y->id));
    };
    jobs.ForEach([&](DaiJob &j) {
        if (borrowing && (j.domain != DaiDomain::Tactics)) return;
        if (!runnable(c, j, kinds)) return;
        DaiJob *&b = best[(size_t)j.domain];
        DaiJob *&s2 = second[(size_t)j.domain];
        if (better(j, b)) { s2 = b; b = &j; }
        else if (better(j, s2)) s2 = &j;
    });
    // A borrowed gesture is Tactics' alone, and moves no other domain's credit or wait.
    if (borrowing && (best[(size_t)DaiDomain::Tactics] == nullptr)) return nullptr;
    // A Defend goal (the director's defend posture, 06 section 11) multiplies Defence's share by its weight.
    int64_t defend = 1;
    c.goals.ForEach([&](const DaiGoal &g) {
        if (g.kind == DaiGoalKind::Defend) defend = std::max<int64_t>(defend, FixToInt(g.weight));
    });
    int64_t total = 0;
    for (size_t d = 0; (d < best.size()) && !borrowing; d++) {
        if (best[d] == nullptr) {
            m_credit[d] = 0;    // a domain with nothing to do keeps no credit
            m_waited[d] = 0;
            continue;
        }
        const int64_t w = c.cfg.arbiter_weight[d] * (((DaiDomain)d == DaiDomain::Defence) ? defend : 1);
        m_credit[d] += w;
        total += w;
    }
    if ((total == 0) && !borrowing) return nullptr;
    // The quota: a domain may lose (quota - waited) more attempts, then must run. Sorted by that slack, the k-th domain
    // must run within its slack + 1 attempts; when that is tight for some k, the least slack runs now (earliest
    // deadline first), so no domain ever waits past its quota while there are no more domains with work than
    // quota_window + 1. Otherwise the most credit runs; ties: the lower domain.
    std::array<int64_t, (size_t)DaiDomain::Count> order{};
    int64_t m = 0;
    for (size_t d = 0; d < best.size(); d++) if (best[d] != nullptr) order[(size_t)m++] = (int64_t)d;
    std::sort(order.begin(), order.begin() + m, [&](int64_t a, int64_t b) {
        return (m_waited[(size_t)a] != m_waited[(size_t)b]) ? (m_waited[(size_t)a] > m_waited[(size_t)b]) : (a < b);
    });
    // Tactics is urgent (06 section 5.6): a runnable fight job goes first.
    int64_t pick = (best[(size_t)DaiDomain::Tactics] != nullptr) ? (int64_t)DaiDomain::Tactics : -1;
    for (int64_t k = 0; (k < m) && (pick < 0); k++) {
        const int64_t slack = c.cfg.arbiter_quota_window - m_waited[(size_t)order[(size_t)k]];
        if (slack < k + 1) { pick = order[0]; break; }
    }
    if (pick < 0) {
        for (size_t d = 0; d < best.size(); d++) {
            if ((best[d] != nullptr) && ((pick < 0) || (m_credit[d] > m_credit[(size_t)pick]))) pick = (int64_t)d;
        }
    }
    // The difficulty's noise (05 section 7.1): a share of the choices go to the domain's second-best job, drawn from
    // Daimon's random -- the same on replay; no draw at all without noise.
    DaiJob *chosen = best[(size_t)pick];
    if ((c.difficulty.noise > 0) && (second[(size_t)pick] != nullptr) && (c.rng.Below(DAI_FIX_ONE) < c.difficulty.noise))
        chosen = second[(size_t)pick];
    DaiJob &j = *chosen;
    const IDaiJobKind &kind = *kind_of(kinds, j.kind);
    if (!borrowing) {
        m_credit[(size_t)pick] -= total * kind.GestureCost();
        for (size_t d = 0; d < best.size(); d++) {
            if (best[d] != nullptr) m_waited[d] = ((int64_t)d == pick) ? 0 : m_waited[d] + 1;
        }
    }
    m_attempts++;

    if (!take_claims(c, ledger, j)) return nullptr;
    KeeperVerb verb;
    char why[32] = {};
    const DaiResult r = kind.NextVerb(c, j, &verb, why, sizeof(why));
    switch (r) {
    case DaiResult::Ok: {
        int64_t verb_id = 0;
        const char *err = c.mode.Refuses(c, verb);
        if (err == nullptr) err = act.Submit(slot, c.player, verb, &verb_id);
        if (err != nullptr) {
            ledger.ReleaseJob(j.id);
            jobs.SetState(j, DaiJobState::Failed, err, c.now);
            return &j;
        }
        j.verb_id = verb_id;
        j.gestures++;
        jobs.SetState(j, DaiJobState::Running, "", c.now);
        m_gestures++;
        m_pace.Note(c.now);
        DaiLog::Debug(c.player, "j%u %s: gesture v%" PRId64 " submitted (verb %d, rect (%d,%d)-(%d,%d), pos (%d,%d))", j.id, kind.GetName(), verb_id,
            (int)verb.kind, (int)verb.slab_x0, (int)verb.slab_y0, (int)verb.slab_x1, (int)verb.slab_y1, (int)verb.stl_x, (int)verb.stl_y);
        return &j;
    }
    case DaiResult::Done:
        ledger.ReleaseJob(j.id);
        jobs.SetState(j, DaiJobState::Done, "", c.now);
        return &j;
    case DaiResult::Wait:
        ledger.ReleaseJob(j.id);
        return &j;
    default:
        ledger.ReleaseJob(j.id);
        jobs.SetState(j, DaiJobState::Failed, (why[0] != '\0') ? why : "NO_VERB", c.now);
        return &j;
    }
}

void DaiArbiter::Save(DaiBlobWriter &w) const
{
    w.I64((int64_t)m_credit.size());
    for (size_t d = 0; d < m_credit.size(); d++) { w.I64(m_credit[d]); w.I64(m_waited[d]); }
    w.I64(m_attempts);
    w.I64(m_gestures);
    m_pace.Save(w);
}

bool DaiArbiter::Load(DaiBlobReader &r)
{
    Reset();
    // A save with fewer domains (an older minor) leaves the rest at 0; more is refused.
    const int64_t n = r.Count((int64_t)m_credit.size());
    for (int64_t d = 0; (d < n) && r.Ok(); d++) {
        m_credit[(size_t)d] = r.I64();
        m_waited[(size_t)d] = r.Range(0, INT64_MAX);
    }
    m_attempts = r.Range(0, INT64_MAX);
    m_gestures = r.Range(0, INT64_MAX);
    if (r.Ok()) m_pace.Load(r);
    if (!r.Ok()) Reset();
    return r.Ok();
}

int64_t DaiPaceWindow::Count(DaiTurn now)
{
    while (!m_turns.empty() && (m_turns.front() + DAI_PACE_WINDOW <= now)) m_turns.pop_front();
    return (int64_t)m_turns.size();
}

void DaiPaceWindow::Save(DaiBlobWriter &w) const
{
    w.I64((int64_t)m_turns.size());
    for (DaiTurn t : m_turns) w.U64(t);
}

bool DaiPaceWindow::Load(DaiBlobReader &r)
{
    m_turns.clear();
    const int64_t n = r.Count(DAI_PACE_WINDOW);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) m_turns.push_back((DaiTurn)r.U64());
    return r.Ok();
}
