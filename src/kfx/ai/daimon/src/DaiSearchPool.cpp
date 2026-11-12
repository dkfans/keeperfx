#include "pre_inc.h"
#include "daimon/DaiSearchPool.h"
#include "post_inc.h"

DaiDigSearch *DaiSearchPool::Get(uint32_t job, int64_t limit, bool *fresh)
{
    *fresh = false;
    if (job == 0) return nullptr;
    if (DaiDigSearch *s = Find(job)) return s;
    if (Held() >= limit) return nullptr;
    for (Slot &s : m_slots) {
        if (s.job != 0) continue;
        s.job = job;
        *fresh = true;
        return &s.search;
    }
    return nullptr;
}

DaiDigSearch *DaiSearchPool::Find(uint32_t job)
{
    for (Slot &s : m_slots) if ((job != 0) && (s.job == job)) return &s.search;
    return nullptr;
}

void DaiSearchPool::Release(uint32_t job)
{
    for (Slot &s : m_slots) if (s.job == job) s.job = 0;
}

void DaiSearchPool::Save(DaiBlobWriter &w) const
{
    for (const Slot &s : m_slots) {
        w.U32(s.job);
        if (s.job != 0) s.search.Save(w);
    }
}

bool DaiSearchPool::Load(DaiBlobReader &r)
{
    for (Slot &s : m_slots) {
        s.job = r.U32();
        if ((s.job != 0) && !s.search.Load(r)) r.Fail("search");
    }
    if (!r.Ok()) ReleaseAll();
    return r.Ok();
}

int64_t DaiSearchPool::Held() const
{
    int64_t n = 0;
    for (const Slot &s : m_slots) if (s.job != 0) n++;
    return n;
}
