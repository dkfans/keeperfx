#include "pre_inc.h"
#include "daimon/DaiInbox.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

static_assert(KF_Count == DAIMON_INBOX_KINDS, "config_daimon.h's DAIMON_INBOX_KINDS must be keeper_facts_types.h's KF_Count");

namespace {
uint32_t fact_key(const KeeperFact &f)
{
    // Kind, sub-kind and slab: a fight that goes on in one place is one entry, a second one elsewhere another.
    uint32_t h = 2166136261u;
    const int64_t parts[4] = { (int64_t)f.kind, f.sub, (int64_t)(f.x / DAI_STL_PER_SLB), (int64_t)(f.y / DAI_STL_PER_SLB) };
    for (int64_t v : parts) { h ^= (uint32_t)v; h *= 16777619u; h ^= (uint32_t)(v >> 32); h *= 16777619u; }
    return (h == 0) ? 1 : h;
}
}

void DaiInbox::Feed(const KeeperFact *facts, int64_t n, DaiTurn now, const DaimonGlobalConfig &cfg)
{
    for (int64_t k = 0; k < n; k++) {
        const KeeperFact &f = facts[k];
        if ((f.kind <= KF_None) || (f.kind >= KF_Count)) continue;
        m_fed++;
        const uint32_t key = fact_key(f);
        const DaiTurn expires = now + (DaiTurn)cfg.inbox_expiry[f.kind];
        DaiInboxEntry *e = nullptr;
        for (int64_t i = 0; i < m_count; i++) {
            DaiInboxEntry &c = m_ring[(size_t)slot(i)];
            if ((c.key == key) && (c.kind == f.kind) && (c.expires > now)) { e = &c; break; }
        }
        if (e != nullptr) {
            m_refreshed++;
        } else {
            if (m_count == DAI_MAX_INBOX) {
                // Full: the oldest entry nobody needs any more, else the oldest of all.
                int64_t victim = 0;
                for (int64_t i = 0; i < m_count; i++) {
                    const DaiInboxEntry &c = m_ring[(size_t)slot(i)];
                    if (c.handled || (c.expires <= now)) { victim = i; break; }
                }
                // Close the gap: the entries after the victim move up one, so the ring stays in arrival order.
                for (int64_t i = victim; i > 0; i--) m_ring[(size_t)slot(i)] = m_ring[(size_t)slot(i - 1)];
                drop_first();
            }
            e = &m_ring[(size_t)slot(m_count)];
            m_count++;
            e->kind = f.kind;
            e->key = key;
        }
        e->sub = f.sub;
        e->x = f.x;
        e->y = f.y;
        for (int a = 0; a < 4; a++) e->args[a] = f.args[a];
        e->turn = (f.turn != 0) ? f.turn : now;
        e->expires = expires;
        e->handled = false;
    }
}

int64_t DaiInbox::Next(uint32_t kinds_mask, int64_t from) const
{
    for (int64_t i = (from < 0) ? 0 : from; i < m_count; i++) {
        const DaiInboxEntry &e = m_ring[(size_t)slot(i)];
        if (!e.handled && ((kinds_mask & (1u << e.kind)) != 0)) return i;
    }
    return -1;
}

void DaiInbox::Handle(int64_t i)
{
    if ((i >= 0) && (i < m_count)) m_ring[(size_t)slot(i)].handled = true;
}

void DaiInbox::drop_first()
{
    m_ring[(size_t)m_first] = DaiInboxEntry{};
    m_first = (m_first + 1) % DAI_MAX_INBOX;
    m_count--;
}

void DaiInbox::Expire(DaiTurn now)
{
    // Keep the live entries in order, packed from the front.
    int64_t kept = 0;
    for (int64_t i = 0; i < m_count; i++) {
        const DaiInboxEntry e = m_ring[(size_t)slot(i)];
        if (e.expires <= now) continue;
        m_ring[(size_t)slot(kept++)] = e;
    }
    for (int64_t i = kept; i < m_count; i++) m_ring[(size_t)slot(i)] = DaiInboxEntry{};
    m_count = kept;
}

void DaiInbox::Clear()
{
    m_ring.fill(DaiInboxEntry{});
    m_first = 0;
    m_count = 0;
}

void DaiInbox::Save(DaiBlobWriter &w) const
{
    w.I64(m_count);
    for (int64_t i = 0; i < m_count; i++) {
        const DaiInboxEntry &e = Get(i);
        w.I64(e.kind);
        w.I64(e.sub);
        w.I64(e.x);
        w.I64(e.y);
        for (int64_t a : e.args) w.I64(a);
        w.U64(e.turn);
        w.U64(e.expires);
        w.U32(e.key);
        w.Bool(e.handled);
    }
    w.I64(m_fed);
    w.I64(m_refreshed);
}

bool DaiInbox::Load(DaiBlobReader &r)
{
    Clear();
    const int64_t n = r.Count(DAI_MAX_INBOX);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        DaiInboxEntry &e = m_ring[(size_t)i];
        e.kind = (KeeperFactKind)r.Range(KF_None + 1, KF_Count - 1);
        e.sub = r.I64();
        e.x = (DaiSubtlCoord)r.I64();
        e.y = (DaiSubtlCoord)r.I64();
        for (int64_t &a : e.args) a = r.I64();
        e.turn = (DaiTurn)r.U64();
        e.expires = (DaiTurn)r.U64();
        e.key = r.U32();
        e.handled = r.Bool();
    }
    m_count = n;
    m_fed = r.Range(0, INT64_MAX);
    m_refreshed = r.Range(0, INT64_MAX);
    if (!r.Ok()) Clear();
    return r.Ok();
}
