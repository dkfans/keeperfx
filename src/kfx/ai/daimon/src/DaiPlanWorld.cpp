#include "pre_inc.h"
#include "daimon/DaiPlanWorld.h"

#include <algorithm>
#include <tuple>

#include "post_inc.h"

bool DaiQuestion::operator<(const DaiQuestion &o) const
{
    return std::make_tuple(kind, power, x, y, target.idx, target.creation_turn)
        < std::make_tuple(o.kind, o.power, o.x, o.y, o.target.idx, o.target.creation_turn);
}

void DaiPlanWorld::Gather(IDaiWorldReader &world, PlayerNumber p, std::span<const DaiQuestion> asked)
{
    for (size_t d = 0; d < m_kind.size(); d++) m_kind[d] = world.PowerOf((DaiPower)d);
    const int64_t n = std::max<int64_t>(world.PowerKindCount(), 1);
    m_available.assign((size_t)n, false);
    m_own_too.assign((size_t)n, false);
    m_price.assign((size_t)(n * DAI_POWER_LEVELS), 0);
    m_range.assign((size_t)(n * DAI_POWER_LEVELS), 0);
    for (int64_t k = 1; k < n; k++) {
        m_available[(size_t)k] = world.PowerAvailable(p, (DaiPowerKind)k);
        for (int64_t l = 0; l < DAI_POWER_LEVELS; l++) {
            bool own = false;
            m_price[(size_t)(k * DAI_POWER_LEVELS + l)] = world.PowerPrice(p, (DaiPowerKind)k, l);
            m_range[(size_t)(k * DAI_POWER_LEVELS + l)] = world.PowerAreaRange((DaiPowerKind)k, l, &own);
            if (l == 0) m_own_too[(size_t)k] = own;
        }
    }
    m_answers.clear();
    for (const DaiQuestion &q : asked) {
        const bool yes = (q.kind == DaiQuestion::Kind::Castable) ? world.PowerCastable(p, q.power, q.x, q.y, nullptr)
                                                                 : world.PowerTakes(p, q.power, q.target);
        m_answers[q] = yes;
    }
}

std::vector<DaiQuestion> DaiPlanWorld::TakeAsked()
{
    std::vector<DaiQuestion> out;
    out.swap(m_asked);
    m_asked_set.clear();
    return out;
}

DaiAnswer DaiPlanWorld::ask(const DaiQuestion &q) const
{
    if (m_asked_set.insert(q).second) m_asked.push_back(q);
    const auto it = m_answers.find(q);
    if (it == m_answers.end()) return DaiAnswer::Unknown;
    return it->second ? DaiAnswer::Yes : DaiAnswer::No;
}

DaiPowerKind DaiPlanWorld::PowerOf(DaiPower d) const
{
    return (d < DaiPower::Count) ? m_kind[(size_t)d] : 0;
}

bool DaiPlanWorld::PowerAvailable(PlayerNumber p, DaiPowerKind k) const
{
    (void)p;
    return known(k) && m_available[(size_t)k];
}

int64_t DaiPlanWorld::PowerPrice(PlayerNumber p, DaiPowerKind k, int64_t level) const
{
    (void)p;
    if (!known(k)) return 0;
    level = std::clamp<int64_t>(level, 0, DAI_POWER_LEVELS - 1);
    return m_price[(size_t)(k * DAI_POWER_LEVELS + level)];
}

int64_t DaiPlanWorld::PowerAreaRange(DaiPowerKind k, int64_t level, bool *own_too) const
{
    *own_too = known(k) && m_own_too[(size_t)k];
    if (!known(k)) return 0;
    level = std::clamp<int64_t>(level, 0, DAI_POWER_LEVELS - 1);
    return m_range[(size_t)(k * DAI_POWER_LEVELS + level)];
}

DaiAnswer DaiPlanWorld::PowerCastable(PlayerNumber p, DaiPowerKind k, DaiSubtlCoord x, DaiSubtlCoord y) const
{
    (void)p;
    DaiQuestion q;
    q.kind = DaiQuestion::Kind::Castable;
    q.power = k;
    q.x = x;
    q.y = y;
    return ask(q);
}

DaiAnswer DaiPlanWorld::PowerTakes(PlayerNumber p, DaiPowerKind k, const DaiThingRef &target) const
{
    (void)p;
    DaiQuestion q;
    q.kind = DaiQuestion::Kind::Takes;
    q.power = k;
    q.target = target;
    return ask(q);
}
