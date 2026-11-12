#include "pre_inc.h"
#include "daimon/DaiObjectives.h"
#include "post_inc.h"

uint32_t DaiObjectives::Post(DaiDomain owner, const DaiObjective &ob, DaiTurn now)
{
    DaiObjective *o = nullptr;
    m_objectives.ForEach([&](DaiObjective &x) {
        if ((o == nullptr) && (x.owner == owner) && (x.kind == ob.kind) && (x.key == ob.key)) o = &x;
    });
    if (o == nullptr) {
        o = m_objectives.Add();
        if (o == nullptr) return 0;
        o->posted = now;
        o->report = DaiObjectiveReport();
    }
    const uint32_t id = o->id;
    const DaiTurn posted = o->posted;
    const DaiObjectiveReport report = o->report;
    *o = ob;
    o->id = id;
    o->owner = owner;
    o->posted = posted;
    o->report = report;
    return id;
}

void DaiObjectives::Drop(DaiDomain owner, uint32_t id)
{
    const DaiObjective *o = m_objectives.Find(id);
    if ((o != nullptr) && (o->owner == owner)) m_objectives.Remove(id);
}

const DaiObjective *DaiObjectives::Find(DaiDomain owner, DaiObjectiveKind kind, uint64_t key) const
{
    const DaiObjective *o = nullptr;
    m_objectives.ForEach([&](const DaiObjective &x) {
        if ((o == nullptr) && (x.owner == owner) && (x.kind == kind) && (x.key == key)) o = &x;
    });
    return o;
}

const DaiObjectiveReport *DaiObjectives::Report(uint32_t id) const
{
    const DaiObjective *o = m_objectives.Find(id);
    return (o != nullptr) ? &o->report : nullptr;
}

bool DaiObjectives::WriteReport(DaiDomain writer, uint32_t id, const DaiObjectiveReport &r)
{
    if (writer != DaiDomain::Tactics) return false;
    DaiObjective *o = m_objectives.Find(id);
    if (o == nullptr) return false;
    o->report = r;
    return true;
}

void DaiObjectives::Expire(DaiTurn now)
{
    uint32_t gone[DAI_MAX_OBJECTIVES];
    int64_t n = 0;
    m_objectives.ForEach([&](const DaiObjective &o) { if ((o.until != 0) && (now > o.until)) gone[n++] = o.id; });
    for (int64_t i = 0; i < n; i++) m_objectives.Remove(gone[i]);
}

void DaiObjectives::Save(DaiBlobWriter &w) const
{
    w.U32(m_objectives.NextId());
    for (int64_t i = 0; i < m_objectives.Capacity(); i++) {
        const DaiObjective &o = m_objectives.SlotAt(i);
        w.U32(o.id);
        if (o.id == 0) continue;
        w.I64((int64_t)o.owner);
        w.I64((int64_t)o.kind);
        w.I64((int64_t)o.sub);
        w.U64(o.key);
        w.I64(o.area.x0); w.I64(o.area.y0); w.I64(o.area.x1); w.I64(o.area.y1);
        w.Ref(o.target);
        w.I64(o.rival);
        w.I64(o.staging.x); w.I64(o.staging.y);
        w.I64(o.stakes);
        w.I64(o.gold_per_kturn);
        w.I64(o.home_guard_share);
        w.U64(o.posted);
        w.U64(o.until);
        const DaiObjectiveReport &r = o.report;
        w.I64((int64_t)r.state);
        w.I64(r.own_force); w.I64(r.enemy_force); w.I64(r.enemy_levels); w.I64(r.losses); w.I64(r.kills);
        w.I64(r.ratio_trend); w.I64(r.gold_spent); w.I64(r.target_health);
        w.U64(r.updated);
    }
}

bool DaiObjectives::Load(DaiBlobReader &r)
{
    m_objectives = DaiPool<DaiObjective, DAI_MAX_OBJECTIVES>();
    const uint32_t next = r.U32();
    if (next == 0) r.Fail("objective id");
    m_objectives.SetNextId(next);
    for (int64_t i = 0; (i < m_objectives.Capacity()) && r.Ok(); i++) {
        DaiObjective o;
        o.id = r.U32();
        if (o.id == 0) continue;
        o.owner = (DaiDomain)r.Range(0, (int64_t)DaiDomain::Count - 1);
        o.kind = (DaiObjectiveKind)r.Range(0, (int64_t)DaiObjectiveKind::Count - 1);
        o.sub = (DaiObjectiveSub)r.Range(0, (int64_t)DaiObjectiveSub::Count - 1);
        o.key = r.U64();
        o.area.x0 = (DaiSlabCoord)r.Range(-1, 4096); o.area.y0 = (DaiSlabCoord)r.Range(-1, 4096);
        o.area.x1 = (DaiSlabCoord)r.Range(-1, 4096); o.area.y1 = (DaiSlabCoord)r.Range(-1, 4096);
        o.target.idx = (DaiThingIndex)r.Range(0, 1 << 20);
        o.target.creation_turn = (DaiTurn)r.U64();
        o.rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
        o.staging.x = (DaiSlabCoord)r.Range(-1, 4096); o.staging.y = (DaiSlabCoord)r.Range(-1, 4096);
        o.stakes = r.Range(0, 1000 * DAI_FIX_ONE);
        o.gold_per_kturn = r.Range(0, INT64_MAX);
        o.home_guard_share = r.Range(0, DAI_FIX_ONE);
        o.posted = (DaiTurn)r.U64();
        o.until = (DaiTurn)r.U64();
        DaiObjectiveReport &rp = o.report;
        rp.state = (DaiObjectiveState)r.Range(0, (int64_t)DaiObjectiveState::Count - 1);
        rp.own_force = r.I64(); rp.enemy_force = r.I64(); rp.enemy_levels = r.I64(); rp.losses = r.I64(); rp.kills = r.I64();
        rp.ratio_trend = r.I64(); rp.gold_spent = r.I64(); rp.target_health = r.Range(-1, 100);
        rp.updated = (DaiTurn)r.U64();
        if (r.Ok() && !m_objectives.RestoreAt(i, o)) r.Fail("objective slot or id");
    }
    if (!r.Ok()) m_objectives = DaiPool<DaiObjective, DAI_MAX_OBJECTIVES>();
    return r.Ok();
}

const char *DaiObjectiveKindName(DaiObjectiveKind k)
{
    constexpr const char *names[] = { "hold_heart", "hold_area", "protect_imps", "intercept", "escort", "assault", "raid",
                                      "scout", "withdraw" };
    return (k < DaiObjectiveKind::Count) ? names[(size_t)k] : "?";
}

const char *DaiObjectiveStateName(DaiObjectiveState s)
{
    constexpr const char *names[] = { "idle", "engaged", "winning", "holding", "losing", "lost", "done" };
    return (s < DaiObjectiveState::Count) ? names[(size_t)s] : "?";
}

const char *DaiObjectiveSubName(DaiObjectiveSub s)
{
    constexpr const char *names[] = { "none", "portal_steal", "imp_hunt", "harass", "jailbreak" };
    return (s < DaiObjectiveSub::Count) ? names[(size_t)s] : "?";
}
