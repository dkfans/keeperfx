#include "pre_inc.h"
#include "daimon/DaiWorkforce.h"
#include "jobs/JobKinds.h"

#include <algorithm>
#include <cinttypes>

#include "daimon/DaiLog.h"

#include "post_inc.h"

namespace {
bool own_land(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y)
{
    const DaiSlabInfo s = k.SlabAt(x, y);
    return (s.owner == p) && ((s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room) || (s.terrain == DaiTerrain::Door));
}
}

int64_t DaiWorkforce::ImpTarget(const DaiTurnCtx &c, int64_t open_marks, int64_t claim_frontier, int64_t mining_faces, int64_t owned_slabs)
{
    const DaiPersonality &p = c.personality;
    const int64_t work = open_marks + claim_frontier + mining_faces * 2;
    const int64_t by_work = (work + p.workforce_work_per_imp - 1) / p.workforce_work_per_imp;
    const int64_t by_land = (owned_slabs + p.workforce_slabs_per_imp - 1) / p.workforce_slabs_per_imp;
    return std::clamp<int64_t>(std::max(by_work, by_land), p.workforce_imps_min, std::max(p.workforce_imps_max, p.workforce_imps_min));
}

/** The work and the land, from one pass over the knowledge. */
void DaiWorkforce::count(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    int64_t frontier = 0, faces = 0, owned = 0;
    for (DaiSlabCoord y = 0; y < k.Height(); y++) {
        for (DaiSlabCoord x = 0; x < k.Width(); x++) {
            const DaiSlabInfo s = k.SlabAt(x, y);
            if (own_land(k, c.player, x, y)) {
                owned++;
                continue;
            }
            if ((s.terrain == DaiTerrain::Floor) && (s.owner < 0)
                && (own_land(k, c.player, x, y - 1) || own_land(k, c.player, x + 1, y) || own_land(k, c.player, x, y + 1)
                    || own_land(k, c.player, x - 1, y)))
                frontier++;
            if ((k.VeinAt(x, y) != nullptr) && k.Reach().NextToReached(x, y)) faces++;
        }
    }
    m_target = ImpTarget(c, k.OwnMarkCount(), frontier, faces, owned);
}

void DaiWorkforce::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if ((c.now >= m_next_count) || (m_target == 0)) {
        m_next_count = c.now + (DaiTurn)c.cfg.workforce_interval;
        count(c);
    }
    slap(c, w);
    const DaiOwnLook &own = c.know.Own();
    const DaiPower power = DiggerPower(c);
    const DaiPowerLook &imp = c.know.Power(power);
    const DaiForecastOut &f = c.forecast;
    const DaiGoal *goal = c.goals.Find(DaiGoalKind::KeepImps);
    DaiJob *j = w.FindJob(DAI_JOB_CREATE_IMP, 0);
    if (own.imps >= m_target) {
        if ((j != nullptr) && (j->state == DaiJobState::Ready)) w.Cancel(*j, "ENOUGH_IMPS");
        if (goal != nullptr) w.NoteGoal(goal->id, DaiGoalStatus::Met, "");
        return;
    }
    // Below imps_min an imp only has to leave the next wages; above it, it may take only imp_gold_share of what may
    // be spent (the price rises with every imp, so the share stops the buying with gold left for the rest), and
    // never while the next pay day is short (03 section 8.1). While a hazard is hot (P5.13) the imps it kills are
    // not replaced at any price: the share rule even below imps_min, and one imp each hazard_rebuy_turns at most.
    // Below hazard_imps_floor neither holds and the wages are not kept back: without imps nothing pays them.
    const bool hazard = c.threat.AnyHot();
    const bool below_floor = own.imps < c.cfg.defence_hazard_imps_floor;
    const bool below_min = own.imps < c.personality.workforce_imps_min;
    const bool affordable = below_floor ? (imp.price <= f.gold_now - f.committed)
                          : (below_min && !hazard) ? (imp.price <= f.gold_now - f.committed - f.wages_next)
                          : (!f.short_next && (imp.price * DAI_FIX_ONE <= f.spendable * c.personality.workforce_imp_gold_share));
    const bool braked = hazard && !below_floor && (m_last_imp != 0) && (c.now < m_last_imp + (DaiTurn)c.cfg.defence_hazard_rebuy_turns);
    const DaiFix utility = FixFromInt(std::min<int64_t>(m_target - own.imps, 8)) + (below_min ? FixFromInt(4) : 0) + (below_floor ? FixFromInt(4) : 0);
    if (j != nullptr) {
        if (goal != nullptr) w.NoteGoal(goal->id, DaiGoalStatus::Progress, "");
        if (j->state == DaiJobState::Ready) {
            if (!affordable) {
                w.Cancel(*j, f.short_next ? "SHORT_NEXT" : "NOT_ENOUGH_GOLD");
                return;
            }
            // The price rises with every imp: the claim the arbiter takes is today's.
            j->utility = utility;
            j->needs.gold = imp.price;
        }
        return;
    }
    const char *why = (c.now < m_retry_after) ? "RETRY_LATER" : !own.has_heart ? "NO_HEART" : !imp.available ? "NOT_AVAILABLE"
        : braked ? "HAZARD" : !affordable ? (f.short_next && !below_min ? "SHORT_NEXT" : "NOT_ENOUGH_GOLD") : nullptr;
    if (why != nullptr) {
        if (goal != nullptr) w.NoteGoal(goal->id, DaiGoalStatus::Blocked, why);
        return;
    }
    if (goal != nullptr) w.NoteGoal(goal->id, DaiGoalStatus::Progress, "");
    j = w.AddJob(DAI_JOB_CREATE_IMP, 0, utility);
    if (j == nullptr) return;
    j->needs.gold = imp.price;
    j->p = DaiCreateImpParams{ own.imps, power };
}

int64_t DaiWorkforce::TunnellerPay(const DaiKnowledge &k)
{
    for (DaiThingModel m = 1; m <= k.KindCount(); m++) if (k.Kind(m).known && k.Kind(m).tunneller) return k.Kind(m).pay;
    return 0;
}

DaiPower DaiWorkforce::DiggerPower(const DaiTurnCtx &c)
{
    const DaiPowerLook &imp = c.know.Power(DaiPower::CreateImp), &tun = c.know.Power(DaiPower::Tunneller);
    if ((c.personality.workforce_tunnellers == 0) || !tun.available) return DaiPower::CreateImp;
    if (!imp.available) return DaiPower::Tunneller;
    // A tunneller is paid as a creature: its price and tunneller_paydays of its pay against the imp's price.
    const int64_t cost = tun.price + TunnellerPay(c.know) * c.personality.workforce_tunneller_paydays;
    return (cost < imp.price) ? DaiPower::Tunneller : DaiPower::CreateImp;
}

/** Keeps slap_share of the working imps hurried, one slap at most every slap_interval turns (0 = off; 0.25 by default, Q2). */
void DaiWorkforce::slap(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if ((c.personality.workforce_slap_share <= 0) || (c.now < m_next_slap)) return;
    if (DaiJob *s = w.FindJob(DAI_JOB_SLAP_IMPS, 0)) if (!DaiJobEnded(s->state)) return;
    int64_t working = 0, hurried = 0;
    for (const DaiImpLook &i : c.know.OwnImps()) {
        if (!i.working) continue;
        working++;
        if (i.sped_up) hurried++;
    }
    if ((working == 0) || (hurried * DAI_FIX_ONE >= c.personality.workforce_slap_share * working)) return;
    m_next_slap = c.now + (DaiTurn)c.personality.workforce_slap_interval;
    if (DaiJob *j = w.AddJob(DAI_JOB_SLAP_IMPS, 0, FixFromInt(1))) JobSlapAim(c.know, *j);
}

void DaiWorkforce::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    if (j.kind != DAI_JOB_CREATE_IMP) return;
    if (j.state == DaiJobState::Done) m_last_imp = c.now;
    if (j.state == DaiJobState::Blocked) {
        // It stalled (no gold any more, the cast never landed): drop it, and try a fresh one after the window.
        m_retry_after = j.retry_after;
        w.Cancel(j, j.why);
    } else if (j.state == DaiJobState::Failed) {
        // Refused (the power went, nowhere to cast): not again every turn.
        m_retry_after = c.now + c.cfg.jobs_stall_window / 4;
    }
}

void DaiWorkforce::Reset()
{
    m_retry_after = 0;
    m_next_count = 0;
    m_next_slap = 0;
    m_target = 0;
    m_last_imp = 0;
}

void DaiWorkforce::Save(DaiBlobWriter &w) const
{
    w.U64(m_retry_after);
    w.U64(m_next_count);
    w.U64(m_next_slap);
    w.I64(m_target);
    w.U64(m_last_imp);
}

bool DaiWorkforce::Load(DaiBlobReader &r)
{
    m_retry_after = (DaiTurn)r.U64();
    m_next_count = (DaiTurn)r.U64();
    m_next_slap = (DaiTurn)r.U64();
    m_target = r.Range(0, 1000);
    m_last_imp = (DaiTurn)r.U64();
    return r.Ok();
}
