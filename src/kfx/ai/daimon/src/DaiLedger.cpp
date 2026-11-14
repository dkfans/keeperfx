#include "pre_inc.h"
#include "daimon/DaiLedger.h"
#include "post_inc.h"

namespace {
bool overlaps(const DaiRect &a, const DaiRect &b)
{
    return (a.x0 <= b.x1) && (b.x0 <= a.x1) && (a.y0 <= b.y1) && (b.y0 <= a.y1);
}
}

uint32_t DaiLedger::add(DaiClaimKind k, DaiClaimOwner o, uint32_t job, DaiTurn expires, DaiClaim **out)
{
    DaiClaim *c = m_claims.Add();
    if (c == nullptr) return 0;
    c->kind = k;
    c->owner = o;
    c->job_id = (o == DaiClaimOwner::Job) ? job : 0;
    c->expires = expires;
    *out = c;
    return c->id;
}

uint32_t DaiLedger::ClaimGold(uint32_t job, int64_t amount, int64_t gold_now, DaiTurn expires, bool discretionary)
{
    if ((amount < 0) || (amount > GoldUnclaimed(gold_now))) return 0;
    DaiClaim *c = nullptr;
    const uint32_t id = add(DaiClaimKind::Gold, DaiClaimOwner::Job, job, expires, &c);
    if (id != 0) {
        c->amount = amount;
        c->discretionary = discretionary;
    }
    return id;
}

uint32_t DaiLedger::ClaimThing(DaiClaimOwner o, uint32_t job, const DaiThingRef &t, DaiTurn expires)
{
    if ((t.idx == 0) || !ThingFree(t)) return 0;
    DaiClaim *c = nullptr;
    const uint32_t id = add(DaiClaimKind::Thing, o, job, expires, &c);
    if (id != 0) c->thing = t;
    return id;
}

uint32_t DaiLedger::ClaimHand(uint32_t job, DaiTurn expires)
{
    if (!HandFree()) return 0;
    DaiClaim *c = nullptr;
    return add(DaiClaimKind::Hand, DaiClaimOwner::Job, job, expires, &c);
}

uint32_t DaiLedger::ClaimArea(DaiClaimOwner o, uint32_t job, const DaiRect &r, DaiTurn expires)
{
    if (!AreaFree(r)) return 0;
    DaiClaim *c = nullptr;
    const uint32_t id = add(DaiClaimKind::Area, o, job, expires, &c);
    if (id != 0) c->area = r;
    return id;
}

int64_t DaiLedger::GoldClaimed() const
{
    int64_t sum = 0;
    m_claims.ForEach([&](const DaiClaim &c) { if (c.kind == DaiClaimKind::Gold) sum += c.amount; });
    return sum;
}

int64_t DaiLedger::GoldUnclaimed(int64_t gold_now) const
{
    const int64_t left = gold_now - GoldClaimed();
    return (left > 0) ? left : 0;
}

bool DaiLedger::ThingFree(const DaiThingRef &t) const
{
    bool free = true;
    m_claims.ForEach([&](const DaiClaim &c) { free &= !((c.kind == DaiClaimKind::Thing) && (c.thing == t)); });
    return free;
}

bool DaiLedger::HandFree() const
{
    bool free = true;
    m_claims.ForEach([&](const DaiClaim &c) { free &= (c.kind != DaiClaimKind::Hand); });
    return free;
}

bool DaiLedger::AreaFree(const DaiRect &r) const
{
    bool free = true;
    m_claims.ForEach([&](const DaiClaim &c) { free &= !((c.kind == DaiClaimKind::Area) && overlaps(c.area, r)); });
    return free;
}

void DaiLedger::ReleaseJob(uint32_t job)
{
    if (job == 0) return;
    uint32_t ids[DAI_MAX_CLAIMS];
    int64_t n = 0;
    m_claims.ForEach([&](const DaiClaim &c) { if (c.job_id == job) ids[n++] = c.id; });
    for (int64_t i = 0; i < n; i++) m_claims.Remove(ids[i]);
}

void DaiLedger::ReleaseAll()
{
    m_claims.Clear();
}

void DaiLedger::Expire(DaiTurn now)
{
    uint32_t ids[DAI_MAX_CLAIMS];
    int64_t n = 0;
    m_claims.ForEach([&](const DaiClaim &c) { if ((c.expires != 0) && (c.expires <= now)) ids[n++] = c.id; });
    for (int64_t i = 0; i < n; i++) m_claims.Remove(ids[i]);
}
