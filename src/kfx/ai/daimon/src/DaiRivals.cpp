#include "pre_inc.h"
#include "daimon/DaiRivals.h"

#include <algorithm>
#include <functional>
#include <vector>

#include "keeper_facts_types.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** Prizes: a heart (the rival removed: and its estimated force, x10, with it), a portal, a room's slab (a treasury's
 *  or library's twice). */
constexpr int64_t PRIZE_HEART = 1000, PRIZE_FORCE = 10, PRIZE_PORTAL = 400, PRIZE_ROOM_SLAB = 25;
/** Hate: a slab or room slab it took from the keeper; one of its creatures seen on the keeper's ground at a look. */
constexpr int64_t HATE_PER_SLAB = 10, HATE_PER_INTRUDER = 5;
/** Below this the rival has earned no hate (only_attack_attackers). */
constexpr int64_t HATE_MIN = 1;
/** Defence: a door's health per level of defence; a trap's, a guard post's. */
constexpr int64_t DOOR_HP_PER_LEVEL = 1000, TRAP_LEVELS = 1, GUARD_POST_LEVELS = 2;

int64_t decayed(int64_t v, DaiTurn dt, int64_t hl)
{
    if ((v == 0) || (dt == 0) || (hl <= 0)) return v;
    const DaiTurn halves = dt / (DaiTurn)hl;
    if (halves >= 62) return 0;
    v >>= halves;
    const int64_t rem = (int64_t)(dt % (DaiTurn)hl);
    return v - v * rem / (2 * hl);
}

bool same_target(const DaiRivalTarget &a, const DaiRivalTarget &b)
{
    return (a.rival == b.rival) && (a.kind == b.kind) && (a.room_index == b.room_index);
}

void save_target(DaiBlobWriter &w, const DaiRivalTarget &t)
{
    w.I64(t.rival); w.I64((int64_t)t.kind); w.I64(t.x); w.I64(t.y); w.I64(t.room); w.I64(t.room_index);
    w.I64(t.prize); w.I64(t.defence); w.I64(t.route_cost); w.U64(t.route_turn); w.I64(t.score);
}

DaiRivalTarget load_target(DaiBlobReader &r)
{
    DaiRivalTarget t;
    t.rival = (PlayerNumber)r.Range(0, DAI_MAX_PLAYERS - 1);
    t.kind = (DaiTargetKind)r.Range(0, (int64_t)DaiTargetKind::Count - 1);
    t.x = (DaiSlabCoord)r.Range(0, 4096); t.y = (DaiSlabCoord)r.Range(0, 4096);
    t.room = (RoomKind)r.Range(0, 255); t.room_index = r.Range(0, INT32_MAX);
    t.prize = r.Range(0, INT64_MAX / 4); t.defence = r.Range(1, INT64_MAX / 4);
    t.route_cost = r.Range(-1, DAI_DIG_NEVER); t.route_turn = (DaiTurn)r.U64(); t.score = r.Range(0, INT64_MAX);
    return t;
}
}

const char *DaiTargetKindName(DaiTargetKind k)
{
    switch (k) {
    case DaiTargetKind::Heart: return "heart";
    case DaiTargetKind::Room: return "room";
    case DaiTargetKind::Portal: return "portal";
    default: return "?";
    }
}

const DaiRivalEstimate &DaiRivals::Rival(PlayerNumber r) const
{
    static const DaiRivalEstimate none;
    return ((r >= 0) && (r < DAI_MAX_PLAYERS)) ? m_rivals[(size_t)r] : none;
}

int64_t DaiRivals::Hate(PlayerNumber r, DaiTurn now, const DaimonGlobalConfig &cfg) const
{
    const DaiRivalEstimate &e = Rival(r);
    return decayed(e.hate, (now > e.hate_turn) ? now - e.hate_turn : 0, cfg.offence_hate_halflife);
}

void DaiRivals::add_hate(PlayerNumber r, int64_t v, DaiTurn now, const DaimonGlobalConfig &cfg)
{
    if ((r < 0) || (r >= DAI_MAX_PLAYERS) || (v <= 0)) return;
    DaiRivalEstimate &e = m_rivals[(size_t)r];
    e.hate = Hate(r, now, cfg) + v;
    e.hate_turn = now;
}

const DaiRivalTarget *DaiRivals::Best() const
{
    for (const DaiRivalTarget &t : m_targets)
        if ((t.score > 0) && (t.score >= m_threshold) && (t.route_cost > 0) && (t.route_cost < DAI_DIG_NEVER)) return &t;
    return nullptr;
}

int64_t DaiRivals::OwnLevels(const DaiKnowledge &k)
{
    int64_t f = 0;
    for (const DaiCreatureLook &c : k.Creatures()) if (!k.Kind(c.model).digger) f += c.level;
    return f;
}

int64_t DaiRivals::FreeLevels(const DaiKnowledge &k)
{
    int64_t f = 0;
    for (const DaiCreatureLook &c : k.Creatures()) {
        if (k.Kind(c.model).digger || (c.battle != 0) || c.fleeing || (c.activity == DaiActivity::Unconscious)) continue;
        if (c.health_pct >= DAI_HEALTHY_PCT) f += c.level;
    }
    return f;
}

const DaiContentEntry *DaiRivals::BestSummon(const DaiKnowledge &k, int64_t spare, int64_t *price, const char **why)
{
    const DaiContentEntry *best = nullptr;
    int64_t best_worth = 0;
    const char *none = "none in the level";
    *price = 0;
    for (const DaiContentEntry *e : k.Content().OfRole(DaiContentClass::Power, DaiContentRole::Summon)) {
        if (e->base) continue;
        const DaiPowerLook look = k.OwnPower(e->kind);
        if (!look.available) { none = "not available"; continue; }
        if ((look.price <= 0) || (look.price > spare)) { none = "short of spare gold"; continue; }
        const int64_t lv = std::clamp<int64_t>(e->creature_level, 1, DaiKindLook::LEVELS);
        const int64_t worth = k.Kind((DaiThingModel)e->creature).output[lv - 1] * 1000 / look.price;
        if ((best == nullptr) || (worth > best_worth)) { best = e; best_worth = worth; *price = look.price; }
    }
    if ((best == nullptr) && (why != nullptr)) *why = none;
    return best;
}

DaiReadiness DaiRivals::Readiness(const DaiKnowledge &k, const DaiPersonality &per, const DaimonGlobalConfig &cfg,
                                  const DaiForecastOut &forecast, const DaiRivalTarget &t)
{
    DaiReadiness r;
    r.free_force = FreeLevels(k);
    // With the income ending soon and no gem (plan 13): less superiority -- the rival's treasury is the next income.
    int64_t superiority = per.offence_superiority_to_attack;
    if (forecast.HorizonWithin(per.economy_extras_horizon))
        superiority = std::max<int64_t>(std::min<int64_t>(superiority, 100), superiority - per.offence_horizon_attack_bonus);
    r.needed = superiority * std::max<int64_t>(t.defence, 1) / 100;
    // The share kept home rounded up: it is what must stay.
    r.sendable = r.free_force - (r.free_force * per.offence_home_guard_share + DAI_FIX_ONE - 1) / DAI_FIX_ONE;
    // A summon it can pay for joins the force at the staging point (cast there as it gathers).
    int64_t summon_price = 0;
    if (const DaiContentEntry *s = BestSummon(k, forecast.spare, &summon_price))
        r.summon_force = std::clamp<int64_t>(s->creature_level, 1, DaiKindLook::LEVELS);
    r.force_ok = (r.sendable + r.summon_force >= r.needed);
    // The mean level of those that would go: the strongest, the home guard's share (rounded up) left home (07 P5.12:
    // the mean of every creature, the newest arrivals and the guard included, never reached the target in play).
    std::vector<int64_t> lv;
    for (const DaiCreatureLook &c : k.Creatures()) if (!k.Kind(c.model).digger) lv.push_back(c.level);
    std::sort(lv.begin(), lv.end(), std::greater<int64_t>());
    const int64_t all = (int64_t)lv.size();
    const int64_t home = (all * per.offence_home_guard_share + DAI_FIX_ONE - 1) / DAI_FIX_ONE;
    const int64_t n = std::max<int64_t>(std::min<int64_t>(all, 1), all - home);
    int64_t levels = 0;
    for (int64_t i = 0; i < n; i++) levels += lv[(size_t)i];
    r.mean_level = (n > 0) ? levels * 100 / n : 0;
    // The urgency relaxes the asks (plan 13): the level toward 1, the runway toward none, the overwhelming share toward
    // 100 % -- the force just enough once the side's gold is gone.
    r.urgency = Urgency(k, per, forecast);
    const DaiFix slack = DAI_FIX_ONE - r.urgency;
    const int64_t target = std::max<int64_t>(per.offence_attack_level_target, 1);
    r.level_wanted = 100 + (target - 1) * 100 * slack / DAI_FIX_ONE;
    r.runway_wanted = std::max<int64_t>(cfg.offence_attack_runway, 0) * slack / DAI_FIX_ONE;
    r.level_ok = (n > 0) && (r.mean_level >= r.level_wanted);
    r.runway_ok = (forecast.runway_paydays >= r.runway_wanted);
    if (per.offence_overwhelm_share > 0) {
        const int64_t share = 100 + std::max<int64_t>(per.offence_overwhelm_share - 100, 0) * slack / DAI_FIX_ONE;
        r.overwhelming = r.force_ok && ((r.sendable + r.summon_force) * 100 >= share * r.needed);
    }
    return r;
}

DaiFix DaiRivals::Urgency(const DaiKnowledge &k, const DaiPersonality &per, const DaiForecastOut &forecast)
{
    if ((per.offence_urgency_turns <= 0) || !forecast.gold_known || forecast.gem_held) return 0;
    // None in the grace: the game's first urgency_grace_paydays pay days (the rules' gap when pay day does not come,
    // 10,000), the early mining's pace no measure of the gold's end.
    const DaiOwnLook &own = k.Own();
    int64_t interval = DaiForecast::TurnsToPayday(0, own.payday_gap, own.payday_speed);
    if (interval == DAI_NEVER) interval = 10000;
    if (forecast.now < per.offence_urgency_grace_paydays * std::max<int64_t>(interval, 1)) return 0;
    if (forecast.side_gold <= 0) return DAI_FIX_ONE;
    if (forecast.horizon_turns == DAI_NEVER) return 0;               // gold left, none mined yet: no end to see
    const int64_t window = per.offence_urgency_turns;
    if (forecast.horizon_turns >= window) return 0;
    return (DaiFix)((window - forecast.horizon_turns) * DAI_FIX_ONE / window);
}

bool DaiRivals::Favourable(const DaiKnowledge &k, const DaiPersonality &per, const DaimonGlobalConfig &cfg,
                           const DaiForecastOut &forecast, PlayerNumber r, bool *known) const
{
    const DaiRivalTarget *weakest = nullptr;
    for (const DaiRivalTarget &t : m_targets) {
        if ((r >= 0) && (t.rival != r)) continue;
        if ((t.route_cost <= 0) || (t.route_cost >= DAI_DIG_NEVER)) continue;
        if ((weakest == nullptr) || (t.defence < weakest->defence)) weakest = &t;
    }
    if (known != nullptr) *known = (weakest != nullptr);
    return (weakest != nullptr) && Readiness(k, per, cfg, forecast, *weakest).force_ok;
}

void DaiRivals::Update(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, const DaiPersonality &per,
                       const KeeperFact *facts, int64_t nfacts, DaiTurn now)
{
    // Ground and rooms another keeper took: hate at once (args[1]: who holds it now, args[2]: slabs this scan).
    for (int64_t i = 0; i < nfacts; i++) {
        if (facts[i].kind != KF_Loss) continue;
        const int64_t holder = facts[i].args[1];
        if ((holder >= 0) && (holder < DAI_MAX_PLAYERS) && (holder != p))
            add_hate((PlayerNumber)holder, HATE_PER_SLAB * std::max<int64_t>(facts[i].args[2], 1), now, cfg);
    }
    if (now >= m_next_look) {
        m_next_look = now + (DaiTurn)cfg.offence_interval;
        look(k, p, cfg, per, now);
    }
    routes(k, p, cfg, per, now);
}

/** The estimates and targets, as the knowledge has them now; a target kept from the last look keeps its route. */
void DaiRivals::look(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, const DaiPersonality &per, DaiTurn now)
{
    m_last_look = now;
    const RoomKind lair = k.RoomKindOf(DaiRoom::Lair), training = k.RoomKindOf(DaiRoom::Training);
    const RoomKind portal = k.RoomKindOf(DaiRoom::Entrance), guard = k.RoomKindOf(DaiRoom::GuardPost);
    const RoomKind treasury = k.RoomKindOf(DaiRoom::Treasury), library = k.RoomKindOf(DaiRoom::Library);
    // Its creatures on the keeper's ground: hate.
    std::array<int64_t, DAI_MAX_PLAYERS> intruders{};
    for (const DaiThingLook &e : k.Enemies()) {
        if ((e.owner < 0) || (e.owner >= DAI_MAX_PLAYERS)) continue;
        if (k.SlabAt(DaiSubtileSlab(e.x), DaiSubtileSlab(e.y)).owner == p) intruders[(size_t)e.owner]++;
    }
    std::vector<DaiRivalTarget> next;
    // (an int64_t counter: i686 gcc's -Warray-bounds can't see an int8_t one stays non-negative)
    for (int64_t r = 0; r < DAI_MAX_PLAYERS; r++) {
        DaiRivalEstimate &e = m_rivals[(size_t)r];
        e.rival = (r != p) && k.RivalKeeper((PlayerNumber)r);
        if (intruders[(size_t)r] > 0) add_hate(r, HATE_PER_INTRUDER * intruders[(size_t)r], now, cfg);
        if (!e.rival) continue;
        const DaiRivalSighting &s = k.Sighting(r);
        e.heart_known = s.heart_seen;
        e.heart_x = s.heart_x;
        e.heart_y = s.heart_y;
        e.force = k.KnownForce(r, now);
        e.growth = 0;
        for (const DaiRivalRoom &rm : k.RivalRooms())
            if ((rm.look.owner == r) && ((rm.look.kind == lair) || (rm.look.kind == training))) e.growth += rm.look.slabs_seen / 2;
        // The defence seen within defence_radius of a slab, in levels.
        auto defence_at = [&](DaiSlabCoord x, DaiSlabCoord y) {
            const int64_t rad = cfg.offence_defence_radius;
            int64_t d = 0;
            for (const DaiThingLook &t : k.Enemies())
                if ((t.owner == r) && !t.digger && (DaiChess(DaiSubtileSlab(t.x), DaiSubtileSlab(t.y), x, y) <= rad)) d += std::max<int64_t>(t.level, 1);
            for (const DaiEnemyDefence &df : k.EnemyDefences()) {
                if ((df.look.owner != r) || (DaiChess(DaiSubtileSlab(df.look.x), DaiSubtileSlab(df.look.y), x, y) > rad)) continue;
                d += df.look.door ? std::max<int64_t>(1, (df.look.health + DOOR_HP_PER_LEVEL - 1) / DOOR_HP_PER_LEVEL) : TRAP_LEVELS;
            }
            for (const DaiRivalRoom &rm : k.RivalRooms())
                if ((rm.look.owner == r) && (guard != 0) && (rm.look.kind == guard) && (DaiChess(rm.look.x, rm.look.y, x, y) <= rad))
                    d += GUARD_POST_LEVELS;
            return d;
        };
        // Its creatures come to defend whatever is attacked (P9: a room's or portal's defence was only what stood there, so
        // an empty room needed one level, and the whole army met the attack -- level 124, across its lava lake).
        const int64_t response = (e.force + e.growth) * per.offence_response_share / 100;
        if (e.heart_known) {
            DaiRivalTarget t;
            t.rival = r;
            t.kind = DaiTargetKind::Heart;
            t.x = DaiSubtileSlab(e.heart_x);
            t.y = DaiSubtileSlab(e.heart_y);
            t.prize = PRIZE_HEART + PRIZE_FORCE * (e.force + e.growth);
            t.defence = std::max<int64_t>(1, defence_at(t.x, t.y) + response);
            next.push_back(t);
        }
        // Its portals, and its least defended other room.
        const DaiRivalTarget *weakest = nullptr;
        DaiRivalTarget room_best;
        for (const DaiRivalRoom &rm : k.RivalRooms()) {
            if (rm.look.owner != r) continue;
            DaiRivalTarget t;
            t.rival = r;
            t.x = rm.look.x;
            t.y = rm.look.y;
            t.room = rm.look.kind;
            t.room_index = rm.look.index;
            t.defence = std::max<int64_t>(1, defence_at(t.x, t.y) + response);
            if ((portal != 0) && (rm.look.kind == portal)) {
                t.kind = DaiTargetKind::Portal;
                t.prize = PRIZE_PORTAL;
                next.push_back(t);
                continue;
            }
            t.kind = DaiTargetKind::Room;
            const bool rich = ((treasury != 0) && (rm.look.kind == treasury)) || ((library != 0) && (rm.look.kind == library));
            t.prize = PRIZE_ROOM_SLAB * rm.look.slabs_seen * (rich ? 2 : 1);
            if ((weakest == nullptr) || (t.defence < room_best.defence) || ((t.defence == room_best.defence) && (t.prize > room_best.prize))) {
                room_best = t;
                weakest = &room_best;
            }
        }
        if (weakest != nullptr) next.push_back(room_best);
    }
    // Routes already searched carry over to the same target.
    for (DaiRivalTarget &t : next) {
        for (const DaiRivalTarget &old : m_targets) {
            if (!same_target(t, old) || (old.x != t.x) || (old.y != t.y)) continue;
            t.route_cost = old.route_cost;
            t.route_turn = old.route_turn;
        }
    }
    m_targets = std::move(next);
    score(k, cfg, per, now);
}

void DaiRivals::score(const DaiKnowledge &k, const DaimonGlobalConfig &cfg, const DaiPersonality &per, DaiTurn now)
{
    const int64_t own = OwnLevels(k);
    m_threshold = per.offence_attack_threshold;
    for (DaiRivalTarget &t : m_targets) {
        t.score = 0;
        if (per.offence_never_attack != 0) continue;
        if ((per.offence_only_attack_attackers != 0) && (Hate(t.rival, now, cfg) < HATE_MIN)) continue;
        if ((t.route_cost <= 0) || (t.route_cost >= DAI_DIG_NEVER) || (own <= 0)) continue;
        const int64_t aim = ((t.rival >= 0) && (t.rival < DAI_MAX_PLAYERS)) ? m_aims[(size_t)t.rival] : -1;
        const int64_t first = (((aim >= 0) ? aim : per.offence_first_target) == (int64_t)t.kind) ? 2 : 1;
        const int64_t num = t.prize * first * own * 100;
        const int64_t den = std::max<int64_t>(t.defence, 1) * std::max<int64_t>(per.offence_superiority_to_attack, 1) * t.route_cost;
        t.score = (DaiFix)(num / den * DAI_FIX_ONE + (num % den) * DAI_FIX_ONE / den);
    }
    std::stable_sort(m_targets.begin(), m_targets.end(), [](const DaiRivalTarget &a, const DaiRivalTarget &b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.rival != b.rival) return a.rival < b.rival;
        return (a.kind != b.kind) ? (a.kind < b.kind) : (a.room_index < b.room_index);
    });
}

/** One route search at a time: a target never searched first, then the one searched longest ago when it is older than
 *  route_interval. The search steps [offence] search_nodes nodes a turn; its target, if still there, takes the cost. */
void DaiRivals::routes(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, const DaiPersonality &per, DaiTurn now)
{
    if (!m_search_on) {
        const DaiRivalTarget *pick = nullptr;
        for (const DaiRivalTarget &t : m_targets) {
            const bool due = (t.route_cost < 0) || (now >= t.route_turn + (DaiTurn)cfg.offence_route_interval);
            if (!due) continue;
            if ((pick == nullptr) || ((t.route_cost < 0) && (pick->route_cost >= 0)) || (((t.route_cost < 0) == (pick->route_cost < 0)) && (t.route_turn < pick->route_turn)))
                pick = &t;
        }
        if (pick == nullptr) return;
        m_searching = *pick;
        const DaiSlabXY goal{ pick->x, pick->y };
        m_search.StartFromOwnLand(k, p, std::span<const DaiSlabXY>(&goal, 1), DaiDigCosts::Attack(cfg, per, k));
        m_search_on = true;
    }
    DaiBudget b;
    b.search_nodes = cfg.offence_search_nodes;
    const DaiResult r = m_search.Step(k, b);
    if (r == DaiResult::Wait) return;
    m_search_on = false;
    const int64_t cost = (r == DaiResult::Done) ? std::max<int64_t>(m_search.Cost(), 1) : DAI_DIG_NEVER;
    for (DaiRivalTarget &t : m_targets) {
        if (!same_target(t, m_searching)) continue;
        t.route_cost = cost;
        t.route_turn = (now > 0) ? now : 1;
    }
    score(k, cfg, per, now);
}

void DaiRivals::Save(DaiBlobWriter &w) const
{
    for (const DaiRivalEstimate &e : m_rivals) {
        w.Bool(e.rival); w.Bool(e.heart_known); w.I64(e.heart_x); w.I64(e.heart_y);
        w.I64(e.force); w.I64(e.growth); w.I64(e.hate); w.U64(e.hate_turn);
    }
    w.I64((int64_t)m_targets.size());
    for (const DaiRivalTarget &t : m_targets) save_target(w, t);
    w.I64(m_threshold);
    w.U64(m_last_look);
    w.U64(m_next_look);
    w.Bool(m_search_on);
    if (m_search_on) {
        save_target(w, m_searching);
        m_search.Save(w);
    }
}

bool DaiRivals::Load(DaiBlobReader &r)
{
    for (DaiRivalEstimate &e : m_rivals) {
        e.rival = r.Bool(); e.heart_known = r.Bool();
        e.heart_x = (DaiSubtlCoord)r.Range(0, 1 << 16); e.heart_y = (DaiSubtlCoord)r.Range(0, 1 << 16);
        e.force = r.Range(0, INT64_MAX / 4); e.growth = r.Range(0, INT64_MAX / 4);
        e.hate = r.Range(0, INT64_MAX / 4); e.hate_turn = (DaiTurn)r.U64();
    }
    m_targets.clear();
    const int64_t n = r.Count(4096);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) m_targets.push_back(load_target(r));
    m_threshold = r.Range(0, 100 * DAI_FIX_ONE);
    m_last_look = (DaiTurn)r.U64();
    m_next_look = (DaiTurn)r.U64();
    m_search_on = r.Bool();
    if (m_search_on && r.Ok()) {
        m_searching = load_target(r);
        if (!m_search.Load(r)) r.Fail("rivals' route search");
    }
    if (!r.Ok()) {
        *this = DaiRivals();
        return false;
    }
    return true;
}
