#include "pre_inc.h"
#include "daimon/DaiAssistWorld.h"
#include "daimon/DaiGeom.h"

#include <cstring>

#include "post_inc.h"

namespace {
/** Subtiles from a Disease victim an ally's creature must be for the cast: a few slabs of spreading. */
constexpr int64_t DISEASE_ALLY_GAP = 3 * DAI_STL_PER_SLB;

const DaiAssistWorld &of(void *ctx) { return *static_cast<const DaiAssistWorld *>(ctx); }

TbBool can_target(void *ctx, const KeeperAssistRule *r, const KeeperAssistCandidate *c)
{
    const DaiAssistWorld &w = of(ctx);
    const DaiAssistWorld::Known *k = w.Find(c->thing);
    if (k == nullptr) return false;
    // Disease spreads to the creatures beside its victim (process_disease), never the caster's own, but an ally's: none
    // of an ally's within DISEASE_ALLY_GAP subtiles (P9).
    if ((DaiPowerKind)r->power == w.Reader().PowerOf(DaiPower::Disease)) {
        for (const DaiThingLook &o : w.Knowledge().Enemies()) {
            if (o.heart || (o.owner < 0) || (o.owner >= DAI_MAX_PLAYERS) || !w.Knowledge().Alliance(o.owner).ours) continue;
            const int64_t dx = (o.x > k->x) ? o.x - k->x : k->x - o.x, dy = (o.y > k->y) ? o.y - k->y : k->y - o.y;
            if (((dx > dy) ? dx : dy) <= DISEASE_ALLY_GAP) return false;
        }
    }
    // Not known yet (the next gather answers it): not a target this turn.
    return w.Reader().PowerTakes(w.Player(), (DaiPowerKind)r->power, k->ref) == DaiAnswer::Yes;
}

/** The keeper's own creatures (and imps) the power would also hit: allies' creatures are not in the knowledge, so a
 *  power that spares the caster's own (Lightning) counts none. */
int64_t allies_near(void *ctx, const KeeperAssistRule *r, const KeeperAssistCandidate *c)
{
    const DaiAssistWorld &w = of(ctx);
    const DaiAssistWorld::Known *k = w.Find(c->thing);
    if (k == nullptr) return 0;
    bool own_too = false;
    const int64_t range = w.Reader().PowerAreaRange((DaiPowerKind)r->power, r->level, &own_too);
    if ((range <= 0) || !own_too) return 0;
    auto within = [&](DaiSubtlCoord x, DaiSubtlCoord y) {
        const int64_t dx = (x > k->x) ? x - k->x : k->x - x, dy = (y > k->y) ? y - k->y : k->y - y;
        return ((dx > dy) ? dx : dy) * DAI_COORD_PER_STL <= range;
    };
    int64_t n = 0;
    for (const DaiCreatureLook &o : w.Knowledge().Creatures()) n += within(o.x, o.y) ? 1 : 0;
    for (const DaiImpLook &o : w.Knowledge().OwnImps()) n += within(o.x, o.y) ? 1 : 0;
    return n;
}

TbBool power_available(void *ctx, DaiPowerKind power)
{
    const DaiAssistWorld &w = of(ctx);
    return w.Reader().PowerAvailable(w.Player(), power);
}

int64_t price(void *ctx, DaiPowerKind power, int64_t level)
{
    const DaiAssistWorld &w = of(ctx);
    return w.Reader().PowerPrice(w.Player(), power, level);
}

bool on_own_ground(const DaiKnowledge &k, PlayerNumber p, DaiSubtlCoord x, DaiSubtlCoord y)
{
    return k.SlabAt(DaiSubtileSlab(x), DaiSubtileSlab(y)).owner == p;
}
}

DaiAssistWorld::DaiAssistWorld(const IDaiPlanWorld &world, const DaiKnowledge &k, PlayerNumber p)
    : m_reader(world), m_k(k), m_player(p)
{
    m_world.ctx = this;
    m_world.can_target = can_target;
    m_world.allies_near = allies_near;
    m_world.power_available = power_available;
    m_world.price = price;
}

const DaiAssistWorld::Known *DaiAssistWorld::Find(DaiThingIndex thing) const
{
    for (int i = 0; i < m_nknown; i++) {
        if (m_known[(size_t)i].thing == thing) return &m_known[(size_t)i];
    }
    return nullptr;
}

int DaiAssistWorld::Candidates(bool want_fights, bool want_any, KeeperAssistCandidate *out)
{
    int n = 0;
    m_nknown = 0;
    auto add = [&](const DaiThingRef &ref, DaiSubtlCoord x, DaiSubtlCoord y) -> KeeperAssistCandidate * {
        if (n >= KEEPER_ASSIST_MAX_CANDIDATES) return nullptr;
        KeeperAssistCandidate *c = &out[n++];
        std::memset(c, 0, sizeof(*c));
        c->thing = ref.idx;
        m_known[(size_t)m_nknown++] = Known{ ref.idx, ref, x, y };
        return c;
    };
    auto own_facts = [&](const DaiCreatureLook &o, KeeperAssistCandidate *c) {
        c->score = o.force;
        c->facts[KAF_HealthPct] = o.health_pct;
        c->facts[KAF_Health] = o.health;
        c->facts[KAF_Level] = o.level;
        c->facts[KAF_Force] = o.force;
        c->facts[KAF_IsDigger] = m_k.Kind(o.model).digger ? 1 : 0;
        c->facts[KAF_OnOwnGround] = on_own_ground(m_k, m_player, o.x, o.y) ? 1 : 0;
        c->facts[KAF_Fleeing] = o.fleeing ? 1 : 0;
        c->facts[KAF_Kind] = o.model;
    };
    auto targetable = [](const DaiCreatureLook &o) { return o.activity != DaiActivity::Unconscious; };
    if (want_fights) {
        for (const DaiBattleLook &b : m_k.Battles()) {
            const int first = n;
            for (const DaiCreatureLook &o : m_k.Creatures()) {
                if ((o.battle != b.index) || !targetable(o)) continue;
                KeeperAssistCandidate *c = add(o.ref, o.x, o.y);
                if (c == nullptr) break;
                own_facts(o, c);
                c->sets = KEEPER_ASSIST_SET(KAT_AllyInFight) | KEEPER_ASSIST_SET(KAT_AllyAny);
            }
            for (const DaiThingLook &e : m_k.Enemies()) {
                if (e.battle != b.index) continue;
                KeeperAssistCandidate *c = add(e.ref, e.x, e.y);
                if (c == nullptr) break;
                c->score = e.force;
                c->facts[KAF_HealthPct] = e.health_pct;
                c->facts[KAF_Level] = e.level;
                c->facts[KAF_Force] = e.force;
                c->facts[KAF_IsDigger] = e.digger ? 1 : 0;
                c->facts[KAF_OnOwnGround] = on_own_ground(m_k, m_player, e.x, e.y) ? 1 : 0;
                c->facts[KAF_Kind] = e.model;
                c->sets = KEEPER_ASSIST_SET(KAT_EnemyInFight);
            }
            for (int i = first; i < n; i++) {
                out[i].facts[KAF_BattleAllyForce] = b.own_force;
                out[i].facts[KAF_BattleEnemyForce] = b.enemy_force;
                out[i].facts[KAF_ForceRatio] = b.enemy_force * 100 / ((b.own_force > 0) ? b.own_force : 1);
                out[i].facts[KAF_BattleSize] = b.own_count + b.enemy_count;
            }
        }
    }
    if (want_any) {
        for (const DaiCreatureLook &o : m_k.Creatures()) {
            if (!targetable(o) || (Find(o.ref.idx) != nullptr)) continue;
            KeeperAssistCandidate *c = add(o.ref, o.x, o.y);
            if (c == nullptr) break;
            own_facts(o, c);
            c->sets = KEEPER_ASSIST_SET(KAT_AllyAny);
        }
    }
    return n;
}

void DaiAssistWorld::DungeonFacts(int64_t *f) const
{
    std::memset(f, 0, sizeof(int64_t) * KAF_Count);
    const DaiOwnLook &own = m_k.Own();
    f[KAF_Gold] = own.gold;
    f[KAF_GoldAfterWages] = own.gold - own.wage_bill;
    f[KAF_PaydayTurns] = own.turns_to_payday;
    f[KAF_Fights] = (int64_t)m_k.Battles().size();
    int64_t fighters = 0;
    for (const DaiCreatureLook &o : m_k.Creatures()) fighters += m_k.Kind(o.model).digger ? 0 : 1;
    f[KAF_Fighters] = fighters;
    f[KAF_CtaActive] = own.cta_active ? 1 : 0;
    f[KAF_CtaPaying] = own.cta_paying ? 1 : 0;
    f[KAF_ObeyActive] = own.obey_active ? 1 : 0;
    f[KAF_HeartHealthPct] = (own.has_heart && (own.heart_max_health > 0)) ? own.heart_health * 100 / own.heart_max_health : 100;
}
