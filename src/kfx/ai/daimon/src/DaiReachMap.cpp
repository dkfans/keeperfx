#include "pre_inc.h"
#include "daimon/DaiReachMap.h"
#include "post_inc.h"

void DaiReachMap::Init(DaiSlabCoord w, DaiSlabCoord h)
{
    m_w = (w > 0) ? w : 0;
    m_h = (h > 0) ? h : 0;
    m_published.assign((size_t)m_w * (size_t)m_h, 0);
    m_work.clear();
    m_queue.clear();
    m_head = 0;
    m_running = false;
    m_stale = true;
    m_seeded = false;
    m_seed_at = 0;
    m_cycles = 0;
}

bool DaiReachMap::Reached(DaiSlabCoord x, DaiSlabCoord y) const
{
    if ((x < 0) || (y < 0) || (x >= m_w) || (y >= m_h) || m_published.empty()) return false;
    return m_published[(size_t)y * (size_t)m_w + (size_t)x] != 0;
}

bool DaiReachMap::NextToReached(DaiSlabCoord x, DaiSlabCoord y) const
{
    return Reached(x, y - 1) || Reached(x + 1, y) || Reached(x, y + 1) || Reached(x - 1, y);
}

void DaiReachMap::Save(DaiBlobWriter &w) const
{
    w.I64(m_w);
    w.I64(m_h);
    w.Bytes(m_published.data(), m_published.size());
    w.Bool(m_running);
    w.Bool(m_stale);
    w.Bool(m_seeded);
    w.I64(m_seed_at);
    w.I64(m_cycles);
    if (m_running) {
        w.Bytes(m_work.data(), m_work.size());
        w.I64((int64_t)m_queue.size());
        for (int32_t q : m_queue) w.U32((uint32_t)q);
        w.I64((int64_t)m_head);
    }
}

bool DaiReachMap::Load(DaiBlobReader &r)
{
    const DaiSlabCoord w = (DaiSlabCoord)r.Range(0, 256), h = (DaiSlabCoord)r.Range(0, 256);
    if (!r.Ok()) return false;
    Init(w, h);
    const int64_t n = (int64_t)w * h;
    r.Bytes(m_published.data(), m_published.size());
    for (uint8_t &b : m_published) if (b > 1) r.Fail("reach byte");
    m_running = r.Bool();
    m_stale = r.Bool();
    m_seeded = r.Bool();
    m_seed_at = r.Range(0, n);
    m_cycles = r.Range(0, INT64_MAX);
    if (m_running && r.Ok()) {
        m_work.assign((size_t)n, 0);
        r.Bytes(m_work.data(), m_work.size());
        for (uint8_t &b : m_work) if (b > 1) r.Fail("reach byte");
        const int64_t q = r.Count(n);
        m_queue.resize((size_t)q);
        for (int32_t &v : m_queue) {
            const uint32_t at = r.U32();
            if (at >= (uint64_t)n) r.Fail("reach queue");
            v = (int32_t)at;
        }
        m_head = (size_t)r.Range(0, q);
    }
    if (!r.Ok()) Init(w, h);
    return r.Ok();
}
