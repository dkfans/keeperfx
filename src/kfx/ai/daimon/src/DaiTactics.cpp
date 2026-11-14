#include "pre_inc.h"
#include "daimon/DaiTactics.h"

#include <algorithm>
#include <cinttypes>

#include "keeper_reflexes.h"
#include "daimon/DaiAssistWorld.h"
#include "daimon/DaiDropSpot.h"
#include "daimon/DaiFightRules.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
constexpr uint64_t KEY_BATTLE = 1ULL << 32, KEY_CONTACT = 2ULL << 32;
/** Slabs within which imps count as near a fight, and contacts are one engagement. */
constexpr int64_t NEAR = 3;
constexpr int64_t MAX_ENGAGEMENTS = 64;
/** The weight of an implicit objective (P4.9 makes them the personality's defence priorities). */
constexpr DaiFix IMPLICIT_HOLD_AREA = 2 * DAI_FIX_ONE, IMPLICIT_PROTECT_IMPS = 3 * DAI_FIX_ONE / 2, AWAY = DAI_FIX_ONE / 2;
/** A healthy fighter: free to be sent, at this health % or more. */

/** Knocked out or dying: inactive, and the hand cannot take it (a human's try is refused too). */
bool down(const DaiCreatureLook &o) { return (o.activity == DaiActivity::Unconscious) || (o.health < 0); }

/** Slabs from (x, y) to the rectangle (0 inside). */
int64_t to_rect(const DaiRect &r, DaiSlabCoord x, DaiSlabCoord y)
{
    const int64_t dx = (x < r.x0) ? r.x0 - x : (x > r.x1) ? x - r.x1 : 0;
    const int64_t dy = (y < r.y0) ? r.y0 - y : (y > r.y1) ? y - r.y1 : 0;
    return std::max(dx, dy);
}

DaiFix capped_risk(int64_t num, int64_t den)
{
    // 1 plus num / den, at most 3.
    const DaiFix extra = (den > 0) ? (DaiFix)std::min<int64_t>(2 * DAI_FIX_ONE, num * DAI_FIX_ONE / den) : 0;
    return DAI_FIX_ONE + extra;
}

bool standing(DaiObjectiveKind k)
{
    return (k == DaiObjectiveKind::HoldHeart) || (k == DaiObjectiveKind::HoldArea) || (k == DaiObjectiveKind::ProtectImps);
}

/** The menu's steps (06 section 5.5), then the offensive ones' hand trips (07 section 4.3). */
enum Step : uint8_t { STEP_EVACUATE = 1, STEP_HEART, STEP_REINFORCE, STEP_BANNER, STEP_CAST, STEP_WOUNDED, STEP_RECALL, STEP_GROUP,
                      STEP_WAVE, STEP_WITHDRAW };
/** Hand rule slots of Tactics' reflexes. */
constexpr int SLOT_EVACUATE = 0, SLOT_WOUNDED = 1, SLOT_REINFORCE = 2;
/** Turns between two Hold Audiences (it lasts about this long). */
constexpr DaiTurn AUDIENCE_GAP = 500;
/** Turns before a failed set_tendency is tried again. */
constexpr DaiTurn TENDENCY_RETRY = 1000;
/** Turns before the group is formed again for the same use, once the game has broken it up (arrival, pay day,
 *  hunger, an attack on a member, the hold up) or an order failed. */
constexpr DaiTurn GROUP_REORDER = 300;
/** Turns a group formed for a fight is left to it before one is formed for another fight: battles begin and end and
 *  their keys change (the arena re-formed it every 19 turns without this). A fight may always take a group from a
 *  non-combat objective. */
constexpr DaiTurn GROUP_GAP = 150;
/** The group's use (m_group_use) for a non-combat objective: this | its id. */
constexpr uint64_t GROUP_OBJECTIVE = 3ULL << 32;
/** The banner moves when its fight has moved this many slabs. */
constexpr int64_t BANNER_MOVE = 2;
/** The banner's use for the offensive objective (gather, advance) when no fight of its own has it. */
constexpr uint64_t KEY_OFFENCE = 4ULL << 32;
/** The offensive hand trips' job keys. */
constexpr uint64_t KEY_WAVE = 5ULL << 32, KEY_WITHDRAW = 6ULL << 32;
/** A summon's cast job (Phase 7): its key, by the power's kind. */
constexpr uint64_t KEY_SUMMON = 7ULL << 32;
/** Turns between "no summon" lines (a diagnostic, not saved). */
constexpr DaiTurn SUMMON_LOG_TURNS = 2000;
/** The rival's heart is in reach (heart focus) within this many slabs of the target or of the keeper's creatures
 *  away from home. */
constexpr int64_t HEART_FOCUS = 6;
/** A fighter this many slabs or fewer from the staging point (or away from home) is forward; the rest are at home. */
constexpr int64_t WAVE_FAR = 2 * NEAR;
/** A wave lands within this many subtiles of the staging point. */
constexpr int64_t WAVE_DROP_RADIUS = 6;
/** A withdrawal lifts the keeper's creatures away from home within this many slabs of its area. */
constexpr int64_t WITHDRAW_REACH = 16;
constexpr DaiTurn KTURN = 1000;

DaiSubtlCoord centre(DaiSlabCoord s) { return (DaiSubtlCoord)(s * 3 + 1); }

DaiSlabXY middle(const DaiRect &r) { return DaiSlabXY{ (DaiSlabCoord)((r.x0 + r.x1) / 2), (DaiSlabCoord)((r.y0 + r.y1) / 2) }; }

bool offensive(DaiObjectiveKind k) { return (k == DaiObjectiveKind::Assault) || (k == DaiObjectiveKind::Raid); }

/** Served by the offensive steps (gather, advance, waves, strike): an assault, or a raid that harasses. */
bool stepped(const DaiObjective &o)
{
    return (o.kind == DaiObjectiveKind::Assault) || ((o.kind == DaiObjectiveKind::Raid) && (o.sub == DaiObjectiveSub::Harass));
}

/** A use of the group alone (it moves there, or attacks what the objective names): an Intercept, Escort, Scout, or a
 *  raid that steals a portal or breaks a creature out of a prison (escorted) or hunts an imp. */
bool group_use(const DaiObjective &o)
{
    switch (o.kind) {
    case DaiObjectiveKind::Intercept: case DaiObjectiveKind::Escort: case DaiObjectiveKind::Scout: case DaiObjectiveKind::SpendTrap:
        return true;
    case DaiObjectiveKind::Raid: return (o.sub == DaiObjectiveSub::PortalSteal) || (o.sub == DaiObjectiveSub::ImpHunt) || (o.sub == DaiObjectiveSub::Jailbreak);
    default: return false;
    }
}

/** A healthy fighter in no fight: free to be sent. */
bool free_fighter(const DaiKnowledge &k, const DaiCreatureLook &o)
{
    return (o.battle == 0) && !o.fleeing && !down(o) && !k.Kind(o.model).digger && (o.health_pct >= DAI_HEALTHY_PCT);
}

/** The reflexes' world over the knowledge: the reach map joins, Daimon's drop spots. */
TbBool reach_connected(void *ctx, DaiSubtlCoord ax, DaiSubtlCoord ay, DaiSubtlCoord bx, DaiSubtlCoord by)
{
    const DaiKnowledge &k = *static_cast<const DaiKnowledge *>(ctx);
    return k.Reach().Reached(DaiSubtileSlab(ax), DaiSubtileSlab(ay)) && k.Reach().Reached(DaiSubtileSlab(bx), DaiSubtileSlab(by));
}

TbBool reach_can_reach(void *ctx, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y)
{
    (void)creature;
    return static_cast<const DaiKnowledge *>(ctx)->Reach().Reached(DaiSubtileSlab(x), DaiSubtileSlab(y));
}

const char *knowledge_drop(void *ctx, PlayerNumber plyr, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y, int64_t radius,
    DaiSubtlCoord reach_x, DaiSubtlCoord reach_y, DaiSubtlCoord *out_x, DaiSubtlCoord *out_y)
{
    (void)creature;
    return DaiFindDropSpot(*static_cast<const DaiKnowledge *>(ctx), plyr, x, y, radius, reach_x, reach_y, out_x, out_y);
}

/** One of the keeper's creatures or imps by thing index: its ref (idx 0 if it has none). */
DaiThingRef own_ref(const DaiKnowledge &k, DaiThingIndex idx)
{
    if (const DaiCreatureLook *c = k.CreatureByIdx(idx)) return c->ref;
    for (const DaiImpLook &i : k.OwnImps()) if (i.ref.idx == idx) return i.ref;
    return DaiThingRef{};
}

/** The keeper's first lair (the game's room index), or 0. */
int64_t lair_room(const DaiKnowledge &k)
{
    for (const DaiRoomLook &r : k.Rooms()) if ((r.roles & DAI_ROLE_LAIR) != 0) return r.index;
    return 0;
}

struct ExecCtx {
    DaiTactics *self;
    DaiManagerWrites *w;
    const DaiTurnCtx *c;
    std::array<int64_t, KEEPER_ASSIST_RULES> *fire;
};
}

DaiFix DaiTactics::StepUtility(int step)
{
    // Earlier steps first (06 section 5.5): evacuating imps before anything, recall last.
    return FixFromInt(20 - step);
}

void DaiTactics::Reset()
{
    m_next = m_fight_logged = 0;
    m_engagements.clear();
    m_banner_key = m_group_key = 0;
    m_group_objective = 0;
    m_free_force = m_home_reserve = 0;
    m_tally.clear();
    m_assist = KeeperAssistState{};
    m_reflex_fire.fill(0);
    m_banner_at = DaiSlabXY{ -1, -1 };
    m_audience_turn = 0;
    m_tendency_after = 0;
    m_group_use = 0;
    m_group_turn = 0;
    m_group_members.clear();
    m_group_at = DaiSlabXY{ -1, -1 };
    m_off_id = 0;
    m_off_since = 0;
    m_off_advanced = false;
    m_off_summoned = false;
    m_waves.clear();
    m_off_point = m_off_heart = DaiSlabXY{ -1, -1 };
}

void DaiTactics::find_engagements(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    m_engagements.clear();
    auto imps_near = [&](DaiSlabCoord x, DaiSlabCoord y) {
        int64_t n = 0;
        for (const DaiImpLook &i : k.OwnImps()) n += (DaiChess(DaiSubtileSlab(i.x), DaiSubtileSlab(i.y), x, y) <= NEAR) ? 1 : 0;
        return n;
    };
    for (const DaiBattleLook &b : k.Battles()) {
        if ((int64_t)m_engagements.size() >= MAX_ENGAGEMENTS) break;
        DaiEngagement e;
        e.key = KEY_BATTLE | (uint64_t)(uint32_t)b.index;
        e.battle = true;
        e.x = DaiSubtileSlab(b.x);
        e.y = DaiSubtileSlab(b.y);
        e.own_force = b.own_force;
        e.enemy_force = b.enemy_force;
        e.own_count = b.own_count;
        e.enemy_count = b.enemy_count;
        e.own_ground = (k.SlabAt(e.x, e.y).owner == c.player);
        e.imps_near = imps_near(e.x, e.y);
        for (const DaiThingLook &en : k.Enemies()) if ((en.battle == b.index) && !en.digger) e.enemy_levels += std::max<int64_t>(en.level, 1);
        m_engagements.push_back(e);
    }
    // Contacts: enemies not in one of these battles, on the keeper's ground, near an objective, or on a predicted route.
    const DaiSlabCoord w = k.Width(), h = k.Height();
    std::vector<uint8_t> route((size_t)w * h, 0);
    for (const DaiThreatRoute &rt : c.threat.Routes()) {
        for (const DaiSlabXY &s : rt.path) if ((s.x >= 0) && (s.y >= 0) && (s.x < w) && (s.y < h)) route[(size_t)s.y * w + s.x] = 1;
    }
    for (const DaiThingLook &en : k.Enemies()) {
        if (en.battle != 0) {
            bool ours = false;
            for (const DaiBattleLook &b : k.Battles()) ours = ours || (b.index == en.battle);
            if (ours) continue;
        }
        const DaiSlabCoord ex = DaiSubtileSlab(en.x), ey = DaiSubtileSlab(en.y);
        bool contact = (k.SlabAt(ex, ey).owner == c.player)
                    || ((ex >= 0) && (ey >= 0) && (ex < w) && (ey < h) && route[(size_t)ey * w + ex]);
        c.objectives.ForEach([&](const DaiObjective &o) {
            if (!contact && (to_rect(o.area, ex, ey) <= c.cfg.tactics_contact_radius)) contact = true;
        });
        if (!contact) continue;
        DaiEngagement *into = nullptr;
        for (DaiEngagement &e : m_engagements) {
            if (!e.battle && (DaiChess(e.x, e.y, ex, ey) <= NEAR)) { into = &e; break; }
        }
        if (into == nullptr) {
            if ((int64_t)m_engagements.size() >= MAX_ENGAGEMENTS) continue;
            DaiEngagement e;
            e.key = KEY_CONTACT | (uint64_t)((int64_t)ey * w + ex);
            e.x = ex;
            e.y = ey;
            e.own_ground = (k.SlabAt(ex, ey).owner == c.player);
            e.imps_near = imps_near(ex, ey);
            m_engagements.push_back(e);
            into = &m_engagements.back();
        }
        into->enemy_force += en.force;
        into->enemy_count++;
        into->enemy_levels += en.digger ? 0 : std::max<int64_t>(en.level, 1);
    }
}

void DaiTactics::match(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    const DaiOwnLook &own = k.Own();
    for (DaiEngagement &e : m_engagements) {
        const DaiObjective *best = nullptr;
        c.objectives.ForEach([&](const DaiObjective &o) {
            bool touches = (to_rect(o.area, e.x, e.y) <= c.cfg.tactics_match_margin);
            if (!touches && (o.target.idx != 0)) {
                for (const DaiThingLook &en : k.Enemies()) {
                    if (!(en.ref == o.target)) continue;
                    touches = (e.battle && (en.battle == (int64_t)(e.key & 0xFFFFFFFFu)))
                           || (DaiChess(DaiSubtileSlab(en.x), DaiSubtileSlab(en.y), e.x, e.y) <= NEAR);
                }
            }
            if (touches && ((best == nullptr) || (o.stakes > best->stakes))) best = &o;
        });
        DaiFix base;
        if (best != nullptr) {
            e.objective = best->id;
            e.kind = best->kind;
            base = best->stakes;
        } else if (e.own_ground) {
            e.objective = 0;
            e.kind = (e.imps_near > 0) ? DaiObjectiveKind::ProtectImps : DaiObjectiveKind::HoldArea;
            base = (e.imps_near > 0) ? IMPLICIT_PROTECT_IMPS : IMPLICIT_HOLD_AREA;
        } else {
            e.objective = 0;
            e.kind = DaiObjectiveKind::HoldArea;   // one of its creatures fighting away from home
            base = AWAY;
        }
        DaiFix risk = DAI_FIX_ONE;
        switch (e.kind) {
        case DaiObjectiveKind::HoldHeart: {
            const int64_t pct = (own.has_heart && (own.heart_max_health > 0)) ? own.heart_health * 100 / own.heart_max_health : 100;
            risk = capped_risk(2 * (100 - pct), 100);
            break;
        }
        case DaiObjectiveKind::HoldArea: {
            int64_t rooms = 0;
            for (DaiSlabCoord y = e.y - NEAR; y <= e.y + NEAR; y++)
                for (DaiSlabCoord x = e.x - NEAR; x <= e.x + NEAR; x++) {
                    const DaiSlabInfo s = k.SlabAt(x, y);
                    rooms += ((s.terrain == DaiTerrain::Room) && (s.owner == c.player)) ? 1 : 0;
                }
            risk = capped_risk(rooms, 16);
            break;
        }
        case DaiObjectiveKind::ProtectImps: risk = capped_risk(e.imps_near, 4); break;
        default: risk = capped_risk(e.enemy_force, 2 * std::max<int64_t>(1, e.own_force)); break;
        }
        e.stakes = FixMul(base, risk);
    }
}

void DaiTactics::allocate(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    std::vector<size_t> order(m_engagements.size());
    for (size_t i = 0; i < order.size(); i++) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        const DaiEngagement &x = m_engagements[a], &y = m_engagements[b];
        return (x.stakes != y.stakes) ? (x.stakes > y.stakes) : (x.key < y.key);
    });
    // Free fighters: healthy creatures (not imps) in no fight; while an Assault or Raid runs, a share stays home.
    m_free_force = 0;
    for (const DaiCreatureLook &o : k.Creatures()) {
        if ((o.battle != 0) || o.fleeing || down(o) || k.Kind(o.model).digger) continue;
        if (o.health_pct >= DAI_HEALTHY_PCT) m_free_force += o.force;
    }
    m_home_reserve = 0;
    c.objectives.ForEach([&](const DaiObjective &o) {
        if ((o.kind == DaiObjectiveKind::Assault) || (o.kind == DaiObjectiveKind::Raid))
            m_home_reserve = std::max<int64_t>(m_home_reserve, FixToInt(FixMul(FixFromInt(m_free_force), o.home_guard_share)));
    });
    offence(c);
    int64_t left = m_free_force;
    int64_t gold = std::max<int64_t>(0, c.forecast.spendable);
    for (size_t i : order) {
        DaiEngagement &e = m_engagements[i];
        e.deficit = c.personality.tactics_superiority_to_defend * e.enemy_force / 100 - e.own_force;
        e.fighters = 0;
        if (e.deficit > 0) {
            const int64_t may = e.own_ground ? left : std::max<int64_t>(0, left - m_home_reserve);
            e.fighters = std::min(e.deficit, may);
            left -= e.fighters;
        }
        const DaiObjective *o = (e.objective != 0) ? c.objectives.Find(e.objective) : nullptr;
        const int64_t want = (o != nullptr) ? o->gold_per_kturn : c.personality.tactics_cast_max_spend;
        e.gold = std::min(want, gold);
        gold -= e.gold;
        e.banner = e.group = false;
    }
    // One banner: free on the keeper's ground, elsewhere only for an objective with gold for its upkeep. It moves only
    // when another engagement's stakes beat the current one's by the switch margin (moving it calls everyone again).
    const DaiFix margin = c.personality.tactics_cta_switch_margin;
    // The personality's cta_use: 0 never, 1 own ground only, 2 also where an objective pays.
    const int64_t cta_use = c.personality.tactics_cta_use;
    auto banner_ok = [&](const DaiEngagement &e) {
        if (cta_use == 0) return false;
        const DaiObjective *o = (e.objective != 0) ? c.objectives.Find(e.objective) : nullptr;
        return e.own_ground || ((cta_use >= 2) && (o != nullptr) && (o->gold_per_kturn > 0));
    };
    DaiEngagement *best = nullptr, *current = nullptr;
    for (size_t i : order) {
        DaiEngagement &e = m_engagements[i];
        if (!banner_ok(e)) continue;
        if (best == nullptr) best = &e;
        if (e.key == m_banner_key) current = &e;
    }
    DaiEngagement *banner = best;
    if ((current != nullptr) && (best != nullptr) && (best != current) && (best->stakes <= FixMul(current->stakes, margin))) banner = current;
    // The offensive objective's banner (gather, advance) while no fight of its own has it: free at the staging point,
    // beyond it paid by the objective's budget; taken from a fight, and given back, by the same margin.
    bool off_banner = false;
    if ((m_off_id != 0) && (m_off_point.x >= 0)) {
        const DaiObjective *o = c.objectives.Find(m_off_id);
        bool fought = false;
        for (const DaiEngagement &e : m_engagements) fought = fought || (e.objective == m_off_id);
        const bool home = (k.SlabAt(m_off_point.x, m_off_point.y).owner == c.player);
        if ((o != nullptr) && !fought && (cta_use > 0) && (home || ((cta_use >= 2) && (o->gold_per_kturn > 0)))) {
            off_banner = (banner == nullptr) || ((m_banner_key == KEY_OFFENCE) ? (banner->stakes <= FixMul(o->stakes, margin))
                                                                                 : (o->stakes > FixMul(banner->stakes, margin)));
        }
    }
    if (off_banner) banner = nullptr;
    m_banner_key = off_banner ? KEY_OFFENCE : (banner != nullptr) ? banner->key : 0;
    if (banner != nullptr) banner->banner = true;
    // One group: the best engagement, or a non-combat objective (Intercept, Escort, Scout) with none, by stakes and with
    // the same margin to move.
    DaiFix group_stakes = -1, held_stakes = -1;
    uint64_t gkey = 0;
    uint32_t gobj = 0;
    for (size_t i : order) {
        const DaiEngagement &e = m_engagements[i];
        if (e.stakes > group_stakes) { group_stakes = e.stakes; gkey = e.key; gobj = 0; }
        if ((m_group_key != 0) && (e.key == m_group_key)) held_stakes = e.stakes;
    }
    c.objectives.ForEach([&](const DaiObjective &o) {
        // The offensive objective once advanced: the strike (its own fights' groups attack what they see, except at
        // the heart).
        const bool strike = (o.id == m_off_id) && m_off_advanced;
        if (!strike && !group_use(o)) return;
        bool served = false;
        for (const DaiEngagement &e : m_engagements) served = served || (e.objective == o.id);
        if (served && !(strike && (m_off_heart.x >= 0))) return;
        if (o.stakes > group_stakes) { group_stakes = o.stakes; gkey = 0; gobj = o.id; }
        if ((m_group_key == 0) && (m_group_objective == o.id)) held_stakes = o.stakes;
    });
    if ((held_stakes >= 0) && (group_stakes <= FixMul(held_stakes, margin))) {
        gkey = m_group_key;
        gobj = m_group_objective;
    }
    m_group_key = gkey;
    m_group_objective = gobj;
    for (DaiEngagement &e : m_engagements) e.group = (gkey != 0) && (e.key == gkey);
}

void DaiTactics::report(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiOwnLook &own = c.know.Own();
    std::vector<Tally> next;
    c.objectives.ForEach([&](const DaiObjective &o) {
        Tally t;
        t.id = o.id;
        int64_t own_force = 0, enemy_force = 0, enemy_levels = 0;
        for (const DaiEngagement &e : m_engagements) {
            if (e.objective != o.id) continue;
            t.engaged = true;
            own_force += e.own_force;
            enemy_force += e.enemy_force;
            enemy_levels += e.enemy_levels;
            t.own_count += e.own_count;
            t.enemy_count += e.enemy_count;
        }
        const Tally *prev = nullptr;
        for (const Tally &p : m_tally) if (p.id == o.id) prev = &p;
        DaiObjectiveReport r = o.report;
        r.own_force = own_force;
        r.enemy_force = enemy_force;
        r.enemy_levels = enemy_levels;
        if (t.engaged) {
            t.ratio = enemy_force * DAI_FIX_ONE / std::max<int64_t>(1, own_force);
            if (own_force == 0) r.state = DaiObjectiveState::Engaged;
            else if (t.ratio < 7 * DAI_FIX_ONE / 10) r.state = DaiObjectiveState::Winning;
            else if (t.ratio <= 13 * DAI_FIX_ONE / 10) r.state = DaiObjectiveState::Holding;
            else r.state = DaiObjectiveState::Losing;
            if ((prev != nullptr) && prev->engaged) {
                r.losses += std::max<int64_t>(0, prev->own_count - t.own_count);
                r.kills += std::max<int64_t>(0, prev->enemy_count - t.enemy_count);
                r.ratio_trend = t.ratio - prev->ratio;
            } else {
                r.ratio_trend = 0;
            }
        } else if ((prev != nullptr) && prev->engaged) {
            r.state = standing(o.kind) ? DaiObjectiveState::Idle : DaiObjectiveState::Done;
            r.ratio_trend = 0;
        } else if ((r.state != DaiObjectiveState::Done) && (r.state != DaiObjectiveState::Lost)) {
            r.state = DaiObjectiveState::Idle;
        }
        if (o.kind == DaiObjectiveKind::Withdraw) r.state = away(c, o).empty() ? DaiObjectiveState::Done : DaiObjectiveState::Engaged;
        if (o.kind == DaiObjectiveKind::HoldHeart) {
            if (!own.has_heart) r.state = DaiObjectiveState::Lost;
            r.target_health = (own.has_heart && (own.heart_max_health > 0)) ? own.heart_health * 100 / own.heart_max_health : 0;
        }
        r.updated = c.now;
        w.WriteReport(o.id, r);
        next.push_back(t);
    });
    m_tally = std::move(next);
}

void DaiTactics::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if ((m_next != 0) && (c.now < m_next)) return;
    find_engagements(c);
    match(c);
    allocate(c);
    report(c, w);
    act(c, w);
    tendency(c, w);
    log_fights(c);
    m_next = c.now + (DaiTurn)(m_engagements.empty() ? c.cfg.tactics_idle_tick : c.cfg.tactics_fight_tick);
}

/** Each battle as Tactics weighs it, every FIGHT_LOG_TURNS turns while there are any (for logs only: nothing reads it,
 *  nor is it saved). */
void DaiTactics::log_fights(const DaiTurnCtx &c)
{
    constexpr DaiTurn FIGHT_LOG_TURNS = 200;
    if ((m_fight_logged != 0) && (c.now < m_fight_logged + FIGHT_LOG_TURNS)) return;
    bool any = false;
    for (const DaiEngagement &e : m_engagements) {
        if (!e.battle) continue;
        any = true;
        DaiLog::Info(c.player, "fight %" PRIx64 " at (%d,%d)%s: own %" PRId64 " force %" PRId64 ", enemy %" PRId64
            " force %" PRId64 " levels %" PRId64 "; deficit %" PRId64 ", sent %" PRId64 ", gold %" PRId64 "%s%s, imps near %" PRId64,
            e.key, (int)e.x, (int)e.y, e.own_ground ? " home" : " away", e.own_count, e.own_force, e.enemy_count, e.enemy_force,
            e.enemy_levels, e.deficit, e.fighters, e.gold, e.banner ? ", banner" : "", e.group ? ", group" : "", e.imps_near);
    }
    if (any) {
        DaiLog::Info(c.player, "fights: free force %" PRId64 ", home reserve %" PRId64, m_free_force, m_home_reserve);
        m_fight_logged = c.now;
    }
}

/** keeper_reflexes' lift as one of Tactics' hand jobs. */
static const char *exec_lift(void *vctx, const KeeperReflexLift *l)
{
    ExecCtx &x = *static_cast<ExecCtx *>(vctx);
    const DaiKnowledge &k = x.c->know;
    const uint8_t step = (l->action == KAA_EvacuateDiggers) ? STEP_EVACUATE : (l->action == KAA_Reinforce) ? STEP_REINFORCE : STEP_WOUNDED;
    DaiJob *j = x.w->AddJob(DAI_JOB_TACTICS_LIFT, (uint64_t)l->slot, DaiTactics::StepUtility(step));
    if (j == nullptr) return "JOBS_FULL";
    DaiLiftParams p;
    for (int i = 0; i < l->count; i++) {
        const DaiThingRef r = own_ref(k, l->things[i]);
        if (r.idx != 0) p.things.push_back(r);
    }
    p.x = (DaiSubtlCoord)l->x;
    p.y = (DaiSubtlCoord)l->y;
    p.room = l->room_id;
    p.step = step;
    p.slot = (int8_t)l->slot;
    j->p = p;
    (*x.fire)[(size_t)l->slot] = (int64_t)x.c->now;
    return nullptr;
}

bool DaiTactics::reflexes(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    DaiAssistWorld aw(c.world, k, c.player);
    KeeperAssistCandidate *acands = m_cands.data();
    const int nac = aw.Candidates(true, true, acands);
    auto facts_of = [&](DaiThingIndex idx) {
        for (int i = 0; i < nac; i++) if (acands[i].thing == idx) return acands[i];
        KeeperAssistCandidate none{};
        none.thing = idx;
        none.facts[KAF_HealthPct] = 100;
        return none;
    };
    std::vector<KeeperReflexCandidate> cands;
    auto add = [&](DaiThingIndex idx, uint8_t role, DaiSubtlCoord x, DaiSubtlCoord y) -> KeeperReflexCandidate & {
        KeeperReflexCandidate rc{};
        rc.thing = idx;
        rc.role = role;
        rc.x = x;
        rc.y = y;
        rc.pos_x = (int64_t)x * 256 + 128;
        rc.pos_y = (int64_t)y * 256 + 128;
        cands.push_back(rc);
        return cands.back();
    };
    // In the order the seat's game lists give them: its fighters, its imps, the enemies it sees, its fighters in fights,
    // then each battle's members (06 section 8: the same decisions, the knowledge's candidates; no boulders, which the
    // knowledge does not keep).
    for (const DaiCreatureLook &o : k.Creatures()) {
        if (down(o) || k.Kind(o.model).digger) continue;
        KeeperReflexCandidate &rc = add(o.ref.idx, KRR_Fighter, o.x, o.y);
        rc.force = o.force;
        rc.health_pct = o.health_pct;
        rc.battle = (BattleIndex)o.battle;
        rc.ac = facts_of(o.ref.idx);
    }
    for (const DaiImpLook &i : k.OwnImps()) {
        // One the hand cannot take (knocked out, dragging a body) is no candidate: seat_reflexes' liftable(), and a
        // single refused member fails the whole hand-load.
        if (!i.pickable) continue;
        KeeperReflexCandidate &rc = add(i.ref.idx, KRR_Digger, i.x, i.y);
        rc.digger = true;
        rc.ac = facts_of(i.ref.idx);
        rc.ac.facts[KAF_IsDigger] = 1;
    }
    for (const DaiThingLook &e : k.Enemies()) if (!e.digger) add(e.ref.idx, KRR_Enemy, e.x, e.y);
    for (int i = 0; i < nac; i++) {
        if ((acands[i].sets & KEEPER_ASSIST_SET(KAT_AllyInFight)) == 0) continue;
        const DaiCreatureLook *o = k.CreatureByIdx(acands[i].thing);
        if ((o == nullptr) || k.Kind(o->model).digger) continue;
        KeeperReflexCandidate &rc = add(o->ref.idx, KRR_Wounded, o->x, o->y);
        rc.ac = acands[i];
    }
    for (const DaiBattleLook &b : k.Battles()) {
        for (const DaiCreatureLook &o : k.Creatures()) {
            if ((o.battle != b.index) || down(o)) continue;
            KeeperReflexCandidate &rc = add(o.ref.idx, KRR_BattleOwn, o.x, o.y);
            rc.force = o.force;
            rc.battle = (BattleIndex)b.index;
            rc.ac = facts_of(o.ref.idx);
        }
        for (const DaiThingLook &e : k.Enemies()) {
            if (e.battle != b.index) continue;
            KeeperReflexCandidate &rc = add(e.ref.idx, KRR_BattleEnemy, e.x, e.y);
            rc.force = e.force;
            rc.battle = (BattleIndex)b.index;
        }
    }
    // The hand rules: imps lifted to the keeper's own ground furthest from the fighting (where its imps can stand; not
    // the heart's room as such -- a raid makes for the heart, and imps there are where it ends), the wounded to a lair,
    // help sent. Ties go to the first slab in scan order.
    KeeperAssistRule rules[KEEPER_ASSIST_RULES]{};
    DaiSlabCoord sx = -1, sy = -1;
    int64_t best = -1;
    for (DaiSlabCoord y = 0; y < k.Height(); y++) {
        for (DaiSlabCoord x = 0; x < k.Width(); x++) {
            const DaiSlabInfo si = k.SlabAt(x, y);
            if ((si.owner != c.player) || (si.terrain != DaiTerrain::Floor) || !k.Reach().Reached(x, y)) continue;
            int64_t near = INT64_MAX;
            for (const DaiEngagement &e : m_engagements) near = std::min(near, DaiChess(x, y, e.x, e.y));
            for (const DaiThingLook &en : k.Enemies()) near = std::min(near, DaiChess(x, y, DaiSubtileSlab(en.x), DaiSubtileSlab(en.y)));
            if (near > best) { best = near; sx = x; sy = y; }
        }
    }
    if (sx >= 0) {
        KeeperAssistRule &ev = rules[SLOT_EVACUATE];
        keeper_assist_default_rule(&ev, KAA_EvacuateDiggers, 0, KAT_AllyAny);
        ev.dest_x = (uint16_t)centre(sx);
        ev.dest_y = (uint16_t)centre(sy);
        ev.count = KEEPER_ASSIST_HAND_MAX;
        ev.radius = KEEPER_ASSIST_DEFAULT_HAND_RADIUS / 2;
        ev.cooldown = KEEPER_ASSIST_DEFAULT_HAND_COOLDOWN;
    }
    KeeperAssistRule &wo = rules[SLOT_WOUNDED];
    keeper_assist_default_rule(&wo, KAA_LiftWounded, 0, KAT_AllyInFight);
    wo.cond[wo.cond_count++] = KeeperAssistCond{ (uint8_t)KAF_HealthPct, (uint8_t)KAO_Lt, (int32_t)c.personality.tactics_retreat_health };
    wo.count = KEEPER_ASSIST_HAND_MAX;
    wo.cooldown = KEEPER_ASSIST_DEFAULT_HAND_COOLDOWN;
    KeeperAssistRule &re = rules[SLOT_REINFORCE];
    keeper_assist_default_rule(&re, KAA_Reinforce, 0, KAT_AllyInFight);
    re.count = KEEPER_ASSIST_HAND_MAX;
    re.radius = KEEPER_ASSIST_DEFAULT_HAND_RADIUS;
    re.min_match = KEEPER_ASSIST_DEFAULT_MIN_MATCH;
    re.cooldown = 2 * KEEPER_ASSIST_DEFAULT_HAND_COOLDOWN;
    // A seat's lieutenant running the agent's rules (P8.7): the agent's hand rules, in their own slots, instead.
    if (c.mode.FightCasting(c) == DaiFightCasting::Human) {
        const DaiHumanFightRules &h = c.know.HumanFightRules();
        bool any = false;
        for (const KeeperAssistRule &r : h.rules) any = any || (r.used && keeper_assist_action_is_hand(r.action));
        if (any) {
            for (int s = 0; s < KEEPER_ASSIST_RULES; s++)
                rules[s] = (h.rules[s].used && keeper_assist_action_is_hand(h.rules[s].action)) ? h.rules[s] : KeeperAssistRule{};
        }
    }
    int64_t facts[KAF_Count];
    aw.DungeonFacts(facts);
    // A heal of its own rules could come: lift_wounded waits for it.
    bool heal_ready = false;
    for (int s = 0; s < KEEPER_ASSIST_RULES; s++) {
        const KeeperAssistRule &r = m_assist.live[s];
        if (!r.used || (r.power != c.world.PowerOf(DaiPower::Heal))) continue;
        const DaiPowerLook &heal = k.Power(DaiPower::Heal);
        heal_ready = heal_ready || (heal.available && (facts[KAF_Gold] - heal.price >= m_assist.globals.reserve));
    }
    KeeperReflexInput in{};
    in.plyr = c.player;
    in.cands = cands.data();
    in.n = (int64_t)cands.size();
    in.rules = rules;
    in.dungeon_facts = facts;
    in.last_fire_turn = m_reflex_fire.data();
    in.now = (int64_t)c.now;
    in.hand_max = KEEPER_ASSIST_HAND_MAX;
    in.lair_room = lair_room(k);
    in.heal_ready = heal_ready;
    KeeperCtlWorld world{};
    world.ctx = const_cast<DaiKnowledge *>(&k);
    world.connected = reach_connected;
    world.can_reach = reach_can_reach;
    world.find_drop_spot = knowledge_drop;
    ExecCtx x{ this, &w, &c, &m_reflex_fire };
    const char *refusal[KEEPER_ASSIST_RULES]{};
    return keeper_reflexes_decide(&in, &world, exec_lift, &x, refusal);
}

bool DaiTactics::recall(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    for (const DaiEngagement &e : m_engagements) {
        if (!e.battle || (e.own_force <= 0)) continue;
        if (e.enemy_force * 100 <= c.personality.tactics_hopeless_ratio * e.own_force) continue;
        if (e.fighters >= e.deficit) continue;      // help is on its way that turns it
        DaiLiftParams p;
        for (const DaiCreatureLook &o : k.Creatures()) {
            if ((o.battle == (int64_t)(e.key & 0xFFFFFFFFu)) && !down(o) && ((int64_t)p.things.size() < KEEPER_ASSIST_HAND_MAX))
                p.things.push_back(o.ref);
        }
        if (p.things.empty()) continue;
        p.room = lair_room(k);
        if (p.room == 0) {
            for (const DaiRoomLook &r : k.Rooms()) if ((r.roles & DAI_ROLE_HEART) != 0) { p.x = centre(r.ax); p.y = centre(r.ay); }
        }
        p.step = STEP_RECALL;
        DaiJob *j = w.AddJob(DAI_JOB_TACTICS_LIFT, (uint64_t)e.key, StepUtility(STEP_RECALL));
        if (j == nullptr) return false;
        j->p = p;
        if (e.objective != 0) {
            const DaiObjectiveReport *rep = c.objectives.Report(e.objective);
            DaiObjectiveReport r = (rep != nullptr) ? *rep : DaiObjectiveReport();
            r.state = DaiObjectiveState::Lost;
            r.updated = c.now;
            w.WriteReport(e.objective, r);
        }
        return true;
    }
    return false;
}

void DaiTactics::casts(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    DaiFightRules rules;
    switch (c.mode.FightCasting(c)) {
    case DaiFightCasting::None: return;          // the human's runner casts (an assistant's, 09 section 5.4)
    case DaiFightCasting::Human: rules.UseHuman(c.know.HumanFightRules()); break;
    case DaiFightCasting::Personality:
        rules.UsePersonality(c.personality, c.world, c.player, c.know.Own().armageddon, c.forecast.spare, &c.know.Content());
        break;
    }
    // The offensive bank while an assault's (or raid's) fight outweighs every fight at home.
    DaiFix off = -1, home = -1;
    for (const DaiEngagement &e : m_engagements) {
        if (offensive(e.kind)) off = std::max(off, e.stakes);
        else if (e.own_ground) home = std::max(home, e.stakes);
    }
    rules.SetBank(((off >= 0) && (off >= home)) ? DaiFightBank::Offensive : DaiFightBank::Defensive);
    keeper_assist_use_rules(&m_assist, &rules.View());
    DaiAssistWorld aw(c.world, k, c.player);
    KeeperAssistCandidate *cands = m_cands.data();
    const int n = aw.Candidates(true, false, cands);
    int64_t facts[KAF_Count];
    aw.DungeonFacts(facts);
    const KeeperAssistDecision d = keeper_assist_decide(&m_assist, aw.World(), facts, cands, n, c.now, true);
    if (d.slot < 0) return;
    const KeeperAssistRule &r = m_assist.live[d.slot];
    DaiPower power = DaiPower::Count;
    for (size_t p = 0; (p < (size_t)DaiPower::Count) && (power == DaiPower::Count); p++)
        if (c.world.PowerOf((DaiPower)p) == (DaiPowerKind)r.power) power = (DaiPower)p;
    // Not one of Daimon's named powers: a map pack's own, cast by its game kind (Phase 7).
    const bool own_kind = (power == DaiPower::Count);
    if (own_kind && (k.Content().Find(DaiContentClass::Power, (int64_t)r.power) == nullptr)) return;
    DaiCastParams p;
    p.power = power;
    if (own_kind) p.kind = (DaiPowerKind)r.power;
    p.level = r.level;
    p.slot = d.slot;
    p.price = d.price;
    if (d.cand < 0) {
        p.off = true;
    } else {
        const DaiAssistWorld::Known *t = aw.Find(cands[d.cand].thing);
        if (t == nullptr) return;
        p.target = t->ref;
        bool own_too = false;
        if ((power == DaiPower::Lightning) || (power == DaiPower::CaveIn)
            || (own_kind && (c.world.PowerAreaRange(p.kind, r.level, &own_too) > 0))) { p.x = t->x; p.y = t->y; }
    }
    DaiJob *j = w.AddJob(DAI_JOB_TACTICS_CAST, (uint64_t)d.slot, StepUtility(STEP_CAST));
    if (j != nullptr) j->p = p;
}

bool DaiTactics::summon(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    // The best summon it can pay for from its spare gold: the strongest creature for the price.
    int64_t price = 0;
    const char *none = "none in the level";
    const DaiContentEntry *best = DaiRivals::BestSummon(k, c.forecast.spare, &price, &none);
    // The home fight most short of force, help on its way counted.
    const DaiEngagement *fight = nullptr;
    for (const DaiEngagement &e : m_engagements) {
        if (!e.own_ground || (e.deficit <= e.fighters)) continue;
        if ((fight == nullptr) || (e.deficit - e.fighters > fight->deficit - fight->fighters)) fight = &e;
    }
    if (fight == nullptr) return false;
    if (best == nullptr) {
        // A summon the level has but none cast: why, now and then.
        if (!k.Content().OfRole(DaiContentClass::Power, DaiContentRole::Summon).empty() && (c.now >= m_summon_logged + SUMMON_LOG_TURNS)) {
            DaiLog::Info(c.player, "tactics: no summon for the fight at (%d,%d), short %d: %s (spare %" PRId64 ")", (int)fight->x,
                         (int)fight->y, (int)(fight->deficit - fight->fighters), none, c.forecast.spare);
            m_summon_logged = c.now;
        }
        return false;
    }
    // Cast on the keeper's own ground: the fight's slab, or the nearest of its own within NEAR.
    DaiSlabXY at{ -1, -1 };
    int64_t near = INT64_MAX;
    for (DaiSlabCoord y = (DaiSlabCoord)std::max<int64_t>(fight->y - NEAR, 0); y <= std::min<int64_t>(fight->y + NEAR, k.Height() - 1); y++)
        for (DaiSlabCoord x = (DaiSlabCoord)std::max<int64_t>(fight->x - NEAR, 0); x <= std::min<int64_t>(fight->x + NEAR, k.Width() - 1); x++) {
            const DaiSlabInfo s = k.SlabAt(x, y);
            if ((s.owner != c.player) || (s.terrain != DaiTerrain::Floor)) continue;
            const int64_t d = DaiChess(x, y, fight->x, fight->y);
            if (d < near) { near = d; at = DaiSlabXY{ x, y }; }
        }
    if (at.x < 0) return false;
    DaiCastParams p;
    p.kind = (DaiPowerKind)best->kind;
    p.x = centre(at.x);
    p.y = centre(at.y);
    p.price = price;
    DaiJob *j = w.AddJob(DAI_JOB_TACTICS_CAST, KEY_SUMMON, StepUtility(STEP_CAST));
    if (j == nullptr) return false;
    j->p = p;
    DaiLog::Info(c.player, "tactics: summon %s (%s level %d) at (%d,%d) for the fight at (%d,%d), short %d, %d gold",
                 best->name, k.Kind((DaiThingModel)best->creature).code, (int)best->creature_level, (int)at.x, (int)at.y,
                 (int)fight->x, (int)fight->y, (int)(fight->deficit - fight->fighters), (int)price);
    return true;
}

bool DaiTactics::summon_at_staging(const DaiTurnCtx &c, DaiManagerWrites &w, DaiSlabXY stg)
{
    const DaiKnowledge &k = c.know;
    const DaiSlabInfo s = k.SlabAt(stg.x, stg.y);
    if ((s.owner != c.player) || ((s.terrain != DaiTerrain::Floor) && (s.terrain != DaiTerrain::Room))) return false;
    if (w.FindJob(DAI_JOB_TACTICS_CAST, KEY_SUMMON) != nullptr) return false;   // one summon at a time
    int64_t price = 0;
    const DaiContentEntry *best = DaiRivals::BestSummon(k, c.forecast.spare, &price);
    if (best == nullptr) return true;           // none to cast (or no gold now): the force goes without
    DaiCastParams p;
    p.kind = (DaiPowerKind)best->kind;
    p.x = centre(stg.x);
    p.y = centre(stg.y);
    p.price = price;
    DaiJob *j = w.AddJob(DAI_JOB_TACTICS_CAST, KEY_SUMMON, StepUtility(STEP_CAST));
    if (j == nullptr) return false;
    j->p = p;
    DaiLog::Info(c.player, "assault %u: summon %s (%s level %d) at the staging point (%d,%d), %d gold", m_off_id, best->name,
                 k.Kind((DaiThingModel)best->creature).code, (int)best->creature_level, (int)stg.x, (int)stg.y, (int)price);
    return true;
}

void DaiTactics::act(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiOwnLook &own = k.Own();
    // A pay day starts a new pay period for the casts' budgets (as process_keeper_assists).
    if (own.payday_progress < m_assist.last_payday_progress) {
        m_assist.spent_this_payday = 0;
        for (KeeperAssistRuleRun &run : m_assist.run) run.spent_this_payday = 0;
    }
    m_assist.last_payday_progress = own.payday_progress;
    bool lift = false, cast = false, banner = false, audience = false;
    c.jobs.ForEach([&](const DaiJob &j) {
        if ((j.domain != DaiDomain::Tactics) || DaiJobEnded(j.state)) return;
        lift = lift || (j.kind == DAI_JOB_TACTICS_LIFT);
        cast = cast || (j.kind == DAI_JOB_TACTICS_CAST);
        banner = banner || (j.kind == DAI_JOB_TACTICS_BANNER);
        audience = audience || (j.kind == DAI_JOB_HOLD_AUDIENCE);
    });
    // 1, 3, 6: the hand's reflexes; one trip at a time. Then a withdrawal's trips home, then the waves.
    if (!lift && !m_engagements.empty()) lift = reflexes(c, w);
    if (!lift) lift = withdraw(c, w);
    if (!lift) lift = waves(c, w);
    // 2: the heart under attack.
    DaiSlabXY heart{ -1, -1 };
    for (const DaiRoomLook &r : k.Rooms()) if ((r.roles & DAI_ROLE_HEART) != 0) heart = DaiSlabXY{ r.ax, r.ay };
    bool heart_fight = false;
    for (const DaiEngagement &e : m_engagements)
        heart_fight = heart_fight || (e.kind == DaiObjectiveKind::HoldHeart) || ((heart.x >= 0) && (DaiChess(e.x, e.y, heart.x, heart.y) <= NEAR));
    const int64_t heart_pct = (own.has_heart && (own.heart_max_health > 0)) ? own.heart_health * 100 / own.heart_max_health : 100;
    DaiSlabXY banner_to{ -1, -1 };
    if (heart_fight && (heart_pct < c.personality.tactics_hold_audience_below)) {
        // Every creature to the heart (Q3); and, as when Hold Audience cannot be cast, the banner there.
        if (!audience && k.Power(DaiPower::HoldAudience).available && ((m_audience_turn == 0) || (c.now >= m_audience_turn + AUDIENCE_GAP))) {
            DaiCastParams p;
            p.power = DaiPower::HoldAudience;
            DaiJob *j = w.AddJob(DAI_JOB_HOLD_AUDIENCE, 0, StepUtility(STEP_HEART));
            if (j != nullptr) { j->p = p; audience = true; }
        }
        if (c.personality.tactics_cta_use > 0) banner_to = heart;
    }
    // 4: the banner where it was given (a fight, or the offensive objective's point), down when nobody has it.
    if (banner_to.x < 0) {
        if (m_banner_key == KEY_OFFENCE) banner_to = m_off_point;
        for (const DaiEngagement &e : m_engagements) if (e.banner) banner_to = DaiSlabXY{ e.x, e.y };
    }
    if (!banner) {
        const DaiPowerLook &cta = k.Power(DaiPower::CallToArms);
        if ((banner_to.x >= 0) && cta.available) {
            const bool placed = own.cta_active && (m_banner_at.x >= 0) && (DaiChess(m_banner_at.x, m_banner_at.y, banner_to.x, banner_to.y) <= BANNER_MOVE);
            if (!placed && (own.gold >= cta.price)) {
                DaiBannerParams p;
                p.x = centre(banner_to.x);
                p.y = centre(banner_to.y);
                DaiJob *j = w.AddJob(DAI_JOB_TACTICS_BANNER, 0, StepUtility(STEP_BANNER));
                if (j != nullptr) { j->p = p; m_banner_at = banner_to; }
            }
        } else if ((banner_to.x < 0) && own.cta_active && (m_banner_at.x >= 0)) {
            DaiBannerParams p;
            p.off = true;
            DaiJob *j = w.AddJob(DAI_JOB_TACTICS_BANNER, 1, StepUtility(STEP_BANNER));
            if (j != nullptr) { j->p = p; m_banner_at = DaiSlabXY{ -1, -1 }; }
        }
    }
    // 5: casts; a pack's summon first where a fight at home is short; then the one an assault's force was counted with
    // (DaiRivals::Readiness), once at its staging point while the force gathers -- its own ground, the creature
    // gathering with the rest.
    if (!cast && !m_engagements.empty()) cast = summon(c, w);
    if (!cast && (m_off_id != 0) && !m_off_advanced && !m_off_summoned) {
        const DaiObjective *o = c.objectives.Find(m_off_id);
        if ((o != nullptr) && (o->staging.x >= 0)) m_off_summoned = summon_at_staging(c, w, o->staging);
    }
    if (!cast && !m_engagements.empty()) casts(c, w);
    // 7: recall a hopeless fight.
    if (!lift && !m_engagements.empty()) recall(c, w);
    // 8: the group.
    group(c, w);
}

/** The keeper's one group (06 section 5a) for the use the allocation gave it: its free fighters nearest the place, up
 *  to the personality's group_size (the rules' BarrackMaxPartySize and the verb's followers at most; one without a
 *  barracks), sent to attack an enemy the keeper sees there (an engagement) or to move there and hold (an Intercept,
 *  Escort or Scout). Formed once for a use; again only GROUP_REORDER turns after the game broke it up; from one fight
 *  to another only GROUP_GAP turns after the last. Released when nothing has a use for it. */
void DaiTactics::group(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiOwnLook &own = k.Own();
    bool open = false;
    std::vector<DaiThingRef> lifting;
    c.jobs.ForEach([&](const DaiJob &j) {
        if ((j.domain != DaiDomain::Tactics) || DaiJobEnded(j.state)) return;
        open = open || (j.kind == DAI_JOB_TACTICS_GROUP);
        if (const DaiLiftParams *p = std::get_if<DaiLiftParams>(&j.p)) lifting.insert(lifting.end(), p->things.begin(), p->things.end());
    });
    if (open) return;
    const uint64_t use = (m_group_key != 0) ? m_group_key : (m_group_objective != 0) ? (GROUP_OBJECTIVE | m_group_objective) : 0;
    if (use == 0) {
        if ((own.group_count > 0) && (m_group_use != 0)) {
            DaiGroupParams p;
            p.order = DaiGroupOrder::Release;
            DaiJob *j = w.AddJob(DAI_JOB_TACTICS_GROUP, 0, StepUtility(STEP_GROUP));
            if (j != nullptr) j->p = p;
        }
        m_group_use = 0;
        m_group_members.clear();
        return;
    }
    // The offensive strike: where, and whether that has moved since the group standing was sent (the heart came in
    // reach): then the same members are ordered again, which keeps the group.
    const bool strike_use = (m_group_key == 0) && (m_group_objective != 0) && (m_group_objective == m_off_id);
    DaiGroupParams p;
    DaiSlabXY at{ -1, -1 };
    DaiThingRef only;
    if (strike_use) {
        const DaiObjective *o = c.objectives.Find(m_group_objective);
        if (o == nullptr) return;
        strike(c, *o, p, at);
    }
    if (strike_use && (use == m_group_use) && (own.group_count > 0) && ((at.x != m_group_at.x) || (at.y != m_group_at.y))) {
        for (const DaiThingRef &r : m_group_members) if (own_ref(k, r.idx) == r) p.members.push_back(r);
        if (p.members.empty()) return;
        DaiJob *j = w.AddJob(DAI_JOB_TACTICS_GROUP, use, StepUtility(STEP_GROUP));
        if (j == nullptr) return;
        j->p = p;
        DaiLog::Info(c.player, "group of %zu struck again: %s (%d,%d)", p.members.size(), (p.order == DaiGroupOrder::Attack) ? "attack" : "move",
            (int)p.x, (int)p.y);
        m_group_at = at;
        m_group_turn = c.now;
        return;
    }
    if ((use == m_group_use) && ((own.group_count > 0) || (c.now < m_group_turn + GROUP_REORDER))) return;
    const bool fight_to_fight = (m_group_key != 0) && (m_group_use != 0) && ((m_group_use & ~0xFFFFFFFFull) != GROUP_OBJECTIVE);
    if ((use != m_group_use) && fight_to_fight && (c.now < m_group_turn + GROUP_GAP)) return;
    // Where, and what to do there.
    if (m_group_key != 0) {
        const DaiEngagement *e = nullptr;
        for (const DaiEngagement &x : m_engagements) if (x.key == m_group_key) e = &x;
        if (e == nullptr) return;
        at = DaiSlabXY{ e->x, e->y };
        // The nearest enemy it sees in the same battle, wherever it is (a battle's place is its own fighters' mean
        // place: the enemy may be slabs off, closing in); else the nearest it sees by the fight's place.
        const DaiThingLook *target = nullptr;
        int64_t best = INT64_MAX;
        const int64_t reach = c.cfg.tactics_match_margin + 2;
        for (const DaiThingLook &t : k.Enemies()) {
            const int64_t d = DaiChess(DaiSubtileSlab(t.x), DaiSubtileSlab(t.y), e->x, e->y);
            const bool same = e->battle && (t.battle == (int64_t)(e->key & 0xFFFFFFFFu));
            if (!same && (d > reach)) continue;
            const int64_t score = same ? d : (1 << 20) + d;
            if (score < best) { best = score; target = &t; }
        }
        if (target != nullptr) {
            p.order = DaiGroupOrder::Attack;
            p.x = target->x;
            p.y = target->y;
        } else {
            p.order = DaiGroupOrder::Move;
            p.x = centre(e->x);
            p.y = centre(e->y);
        }
    } else if (!strike_use) {               // (a strike's place is strike()'s)
        const DaiObjective *o = c.objectives.Find(m_group_objective);
        if (o == nullptr) return;
        at = middle(o->area);
        // A scout goes as far towards it as the keeper's creatures can walk: the reached slab nearest it (a target
        // with no way in is what recon scouts for; the walk reveals the ground on the way).
        if (o->kind == DaiObjectiveKind::Scout) {
            int64_t best = INT64_MAX;
            const DaiSlabXY goal = at;
            for (DaiSlabCoord y = 0; y < k.Height(); y++) {
                for (DaiSlabCoord x = 0; x < k.Width(); x++) {
                    if (!k.Reach().Reached(x, y)) continue;
                    const int64_t d = DaiChess(x, y, goal.x, goal.y) * 1024 + std::abs(x - goal.x) + std::abs(y - goal.y);
                    if (d < best) { best = d; at = DaiSlabXY{ x, y }; }
                }
            }
        }
        p.order = DaiGroupOrder::Move;
        p.x = centre(at.x);
        p.y = centre(at.y);
        // A hunt attacks its imp where the keeper sees it.
        if ((o->kind == DaiObjectiveKind::Raid) && (o->sub == DaiObjectiveSub::ImpHunt)) {
            for (const DaiThingLook &t : k.Enemies()) {
                if (!(t.ref == o->target)) continue;
                p.order = DaiGroupOrder::Attack;
                p.x = t.x;
                p.y = t.y;
                at = DaiSlabXY{ DaiSubtileSlab(t.x), DaiSubtileSlab(t.y) };
            }
        }
        // A trap spent (P5.13): the one creature Defence chose, onto the trap's own slab (it fires at what stands there).
        if (o->kind == DaiObjectiveKind::SpendTrap) only = o->target;
    }
    // Who: the free fighters nearest it.
    bool barracks = false;
    barracks = barracks || k.HasRoomRole(DAI_ROLE_GROUP);
    int64_t size = std::min<int64_t>({ c.personality.tactics_group_size, std::max<int64_t>(own.group_max, 1), 1 + EXTSEAT_MAX_HAND_SET });
    if (!barracks || (only.idx != 0)) size = 1;
    std::vector<const DaiCreatureLook *> free;
    for (const DaiCreatureLook &o : k.Creatures()) {
        if ((o.battle != 0) || o.fleeing || down(o) || k.Kind(o.model).digger) continue;
        if (!o.orderable || (std::find(lifting.begin(), lifting.end(), o.ref) != lifting.end())) continue;
        if ((only.idx != 0) ? !(o.ref == only) : (o.health_pct < DAI_HEALTHY_PCT)) continue;
        free.push_back(&o);
    }
    if (free.empty()) return;
    std::stable_sort(free.begin(), free.end(), [&](const DaiCreatureLook *a, const DaiCreatureLook *b) {
        const int64_t da = DaiChess(DaiSubtileSlab(a->x), DaiSubtileSlab(a->y), at.x, at.y), db = DaiChess(DaiSubtileSlab(b->x), DaiSubtileSlab(b->y), at.x, at.y);
        return (da != db) ? (da < db) : (a->ref.idx < b->ref.idx);
    });
    for (size_t i = 0; (i < free.size()) && ((int64_t)i < size); i++) p.members.push_back(free[i]->ref);
    DaiJob *j = w.AddJob(DAI_JOB_TACTICS_GROUP, use, StepUtility(STEP_GROUP));
    if (j == nullptr) return;
    j->p = p;
    DaiLog::Info(c.player, "group of %zu for %s %" PRIx64 ": %s (%d,%d)", p.members.size(), (m_group_key != 0) ? "engagement" : "objective",
        (m_group_key != 0) ? m_group_key : (uint64_t)m_group_objective, (p.order == DaiGroupOrder::Attack) ? "attack" : "move", (int)p.x, (int)p.y);
    m_group_use = use;
    m_group_turn = c.now;
    m_group_members = p.members;
    m_group_at = strike_use ? at : DaiSlabXY{ -1, -1 };
}

/** The offensive objective of the highest stakes with a staging point: gathering until enough of what may be sent
 *  stands at the staging point (or gather_turns), then advanced; heart focus when the rival's heart is seen in reach. */
void DaiTactics::offence(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    const DaiObjective *best = nullptr;
    c.objectives.ForEach([&](const DaiObjective &o) {
        if (!stepped(o) || (o.staging.x < 0)) return;
        if ((best == nullptr) || (o.stakes > best->stakes)) best = &o;
    });
    m_waves.erase(std::remove_if(m_waves.begin(), m_waves.end(), [&](DaiTurn t) { return t + KTURN <= c.now; }), m_waves.end());
    m_off_point = m_off_heart = DaiSlabXY{ -1, -1 };
    if (best == nullptr) {
        m_off_id = 0;
        m_off_since = 0;
        m_off_advanced = false;
        m_off_summoned = false;
        return;
    }
    if (best->id != m_off_id) {
        m_off_id = best->id;
        m_off_since = c.now;
        m_off_advanced = false;
        m_off_summoned = false;
        m_group_at = DaiSlabXY{ -1, -1 };
    }
    const DaiSlabXY stg = best->staging;
    if (k.RivalKeeper(best->rival) && k.Sighting(best->rival).heart_seen) {
        const DaiRivalSighting &s = k.Sighting(best->rival);
        const DaiSlabXY h{ DaiSubtileSlab(s.heart_x), DaiSubtileSlab(s.heart_y) };
        bool reach = (to_rect(best->area, h.x, h.y) <= HEART_FOCUS);
        for (const DaiCreatureLook &o : k.Creatures()) {
            const DaiSlabCoord x = DaiSubtileSlab(o.x), y = DaiSubtileSlab(o.y);
            reach = reach || ((k.SlabAt(x, y).owner != c.player) && (DaiChess(x, y, h.x, h.y) <= HEART_FOCUS));
        }
        if (reach) m_off_heart = h;
    }
    if (!m_off_advanced) {
        int64_t there = 0;
        for (const DaiCreatureLook &o : k.Creatures())
            if (free_fighter(k, o) && (DaiChess(DaiSubtileSlab(o.x), DaiSubtileSlab(o.y), stg.x, stg.y) <= NEAR)) there += o.force;
        const int64_t may = std::max<int64_t>(0, m_free_force - m_home_reserve);
        const bool gathered = (may > 0) && (there * 100 >= c.cfg.offence_gather_share * may);
        if (gathered || (c.now >= m_off_since + (DaiTurn)c.cfg.offence_gather_turns)) {
            m_off_advanced = true;
            DaiLog::Info(c.player, "assault %u: advance (%" PRId64 " of %" PRId64 " at the staging point%s)", m_off_id, there, may,
                gathered ? "" : ", gather_turns up");
        }
    }
    m_off_point = !m_off_advanced ? stg : (m_off_heart.x >= 0) ? m_off_heart : middle(best->area);
}

void DaiTactics::strike(const DaiTurnCtx &c, const DaiObjective &o, DaiGroupParams &p, DaiSlabXY &at) const
{
    const DaiKnowledge &k = c.know;
    const DaiSlabXY target = middle(o.area);
    p.order = DaiGroupOrder::Attack;
    if (m_off_heart.x >= 0) {
        const DaiRivalSighting &s = k.Sighting(o.rival);
        p.x = s.heart_x;
        p.y = s.heart_y;
        at = m_off_heart;
        return;
    }
    // The rival's door nearest the staging point and no farther from it than the target: the way in.
    const int64_t span = DaiChess(o.staging.x, o.staging.y, target.x, target.y);
    const DaiEnemyDefenceLook *door = nullptr;
    int64_t best = INT64_MAX;
    for (const DaiEnemyDefence &d : k.EnemyDefences()) {
        if (!d.look.door || (d.look.owner != o.rival)) continue;
        const int64_t dist = DaiChess(DaiSubtileSlab(d.look.x), DaiSubtileSlab(d.look.y), o.staging.x, o.staging.y);
        if ((dist > span) || (dist >= best)) continue;
        best = dist;
        door = &d.look;
    }
    if (door != nullptr) {
        p.x = door->x;
        p.y = door->y;
        at = DaiSlabXY{ DaiSubtileSlab(door->x), DaiSubtileSlab(door->y) };
        return;
    }
    const DaiSlabInfo s = k.SlabAt(target.x, target.y);
    if ((s.terrain != DaiTerrain::Room) || (s.owner != o.rival)) p.order = DaiGroupOrder::Move;
    p.x = centre(target.x);
    p.y = centre(target.y);
    at = target;
}

/** Waves (07 section 4.3): the free fighters still at home, the strongest first, a hand-load dropped at the staging
 *  point; what is forward already and the home guard kept out of it. */
bool DaiTactics::waves(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if ((m_off_id == 0) || ((int64_t)m_waves.size() >= c.cfg.offence_waves_per_kturn)) return false;
    const DaiObjective *o = c.objectives.Find(m_off_id);
    if (o == nullptr) return false;
    const DaiKnowledge &k = c.know;
    const DaiSlabXY stg = o->staging;
    if (k.SlabAt(stg.x, stg.y).owner != c.player) return false;
    int64_t forward = 0;
    std::vector<const DaiCreatureLook *> home;
    for (const DaiCreatureLook &x : k.Creatures()) {
        if (!free_fighter(k, x)) continue;
        const DaiSlabCoord sx = DaiSubtileSlab(x.x), sy = DaiSubtileSlab(x.y);
        if ((DaiChess(sx, sy, stg.x, stg.y) <= WAVE_FAR) || (k.SlabAt(sx, sy).owner != c.player)) forward += x.force;
        else home.push_back(&x);
    }
    int64_t may = m_free_force - m_home_reserve - forward;
    if ((may <= 0) || home.empty()) return false;
    std::stable_sort(home.begin(), home.end(), [](const DaiCreatureLook *a, const DaiCreatureLook *b) {
        return (a->force != b->force) ? (a->force > b->force) : (a->ref.idx < b->ref.idx);
    });
    DaiLiftParams p;
    for (const DaiCreatureLook *x : home) {
        if ((int64_t)p.things.size() >= KEEPER_ASSIST_HAND_MAX) break;
        if (x->force > may) continue;
        p.things.push_back(x->ref);
        may -= x->force;
    }
    if (p.things.empty()) return false;
    DaiSubtlCoord dx = 0, dy = 0;
    if (DaiFindDropSpot(k, c.player, centre(stg.x), centre(stg.y), WAVE_DROP_RADIUS, -1, -1, &dx, &dy) != nullptr) return false;
    p.x = dx;
    p.y = dy;
    p.step = STEP_WAVE;
    DaiJob *j = w.AddJob(DAI_JOB_TACTICS_LIFT, KEY_WAVE, StepUtility(STEP_WAVE));
    if (j == nullptr) return false;
    j->p = p;
    m_waves.push_back(c.now);
    DaiLog::Info(c.player, "assault %u: a wave of %zu to (%d,%d)", m_off_id, p.things.size(), (int)dx, (int)dy);
    return true;
}

std::vector<const DaiCreatureLook *> DaiTactics::away(const DaiTurnCtx &c, const DaiObjective &o) const
{
    const DaiKnowledge &k = c.know;
    std::vector<const DaiCreatureLook *> out;
    for (const DaiCreatureLook &x : k.Creatures()) {
        if (down(x) || k.Kind(x.model).digger) continue;
        const DaiSlabCoord sx = DaiSubtileSlab(x.x), sy = DaiSubtileSlab(x.y);
        if ((k.SlabAt(sx, sy).owner == c.player) || (to_rect(o.area, sx, sy) > WITHDRAW_REACH)) continue;
        out.push_back(&x);
    }
    std::stable_sort(out.begin(), out.end(), [&](const DaiCreatureLook *a, const DaiCreatureLook *b) {
        const int64_t da = to_rect(o.area, DaiSubtileSlab(a->x), DaiSubtileSlab(a->y)), db = to_rect(o.area, DaiSubtileSlab(b->x), DaiSubtileSlab(b->y));
        return (da != db) ? (da < db) : (a->ref.idx < b->ref.idx);
    });
    return out;
}

/** A withdrawal (07 section 4.3): the keeper's creatures away from home near it lifted to a lair (the heart without
 *  one), a hand-load a trip. */
bool DaiTactics::withdraw(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiObjective *wd = nullptr;
    c.objectives.ForEach([&](const DaiObjective &o) { if ((o.kind == DaiObjectiveKind::Withdraw) && (wd == nullptr)) wd = &o; });
    if (wd == nullptr) return false;
    const std::vector<const DaiCreatureLook *> out = away(c, *wd);
    if (out.empty()) return false;
    DaiLiftParams p;
    for (const DaiCreatureLook *x : out) if ((int64_t)p.things.size() < KEEPER_ASSIST_HAND_MAX) p.things.push_back(x->ref);
    p.room = lair_room(k);
    if (p.room == 0) {
        bool heart = false;
        for (const DaiRoomLook &r : k.Rooms()) if ((r.roles & DAI_ROLE_HEART) != 0) { p.x = centre(r.ax); p.y = centre(r.ay); heart = true; }
        if (!heart) return false;
    }
    p.step = STEP_WITHDRAW;
    DaiJob *j = w.AddJob(DAI_JOB_TACTICS_LIFT, KEY_WITHDRAW, StepUtility(STEP_WITHDRAW));
    if (j == nullptr) return false;
    j->p = p;
    DaiLog::Info(c.player, "withdraw %u: %zu lifted home", wd->id, p.things.size());
    return true;
}

void DaiTactics::tendency(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if ((c.know.Own().flee_tendency == (c.personality.tactics_flee_tendency != 0)) || (c.now < m_tendency_after)) return;
    if (w.FindJob(DAI_JOB_SET_TENDENCY, 0) == nullptr) w.AddJob(DAI_JOB_SET_TENDENCY, 0, StepUtility(STEP_RECALL));
}

void DaiTactics::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    (void)w;
    if ((j.kind == DAI_JOB_SET_TENDENCY) && (j.state != DaiJobState::Done)) m_tendency_after = c.now + TENDENCY_RETRY;
    if ((j.kind == DAI_JOB_TACTICS_GROUP) && (j.state != DaiJobState::Done)) m_group_turn = c.now;   // not again for a while
    if (j.state != DaiJobState::Done) return;
    if (j.kind == DAI_JOB_TACTICS_CAST) {
        const DaiCastParams *p = std::get_if<DaiCastParams>(&j.p);
        if ((p != nullptr) && !p->off && (p->slot >= 0)) keeper_assist_note_cast(&m_assist, (int)p->slot, p->target.idx, p->price, c.now);
    } else if (j.kind == DAI_JOB_HOLD_AUDIENCE) {
        m_audience_turn = c.now;
    }
}

void DaiTactics::Save(DaiBlobWriter &w) const
{
    w.U64(m_next);
    w.I64((int64_t)m_engagements.size());
    for (const DaiEngagement &e : m_engagements) {
        w.U64(e.key);
        w.Bool(e.battle);
        w.I64(e.x); w.I64(e.y);
        w.I64(e.own_force); w.I64(e.enemy_force); w.I64(e.own_count); w.I64(e.enemy_count); w.I64(e.enemy_levels);
        w.Bool(e.own_ground);
        w.I64(e.imps_near);
        w.U32(e.objective);
        w.I64((int64_t)e.kind);
        w.I64(e.stakes);
        w.I64(e.deficit); w.I64(e.fighters); w.I64(e.gold);
        w.Bool(e.banner); w.Bool(e.group);
    }
    w.U64(m_banner_key);
    w.U64(m_group_key);
    w.U32(m_group_objective);
    w.I64(m_free_force);
    w.I64(m_home_reserve);
    w.I64((int64_t)m_tally.size());
    for (const Tally &t : m_tally) { w.U32(t.id); w.I64(t.own_count); w.I64(t.enemy_count); w.I64(t.ratio); w.Bool(t.engaged); }
    // The casting state as the simulation's assistant keeps its own (a packed C struct): its bytes, with their size.
    w.I64((int64_t)sizeof(m_assist));
    w.Bytes(reinterpret_cast<const uint8_t *>(&m_assist), sizeof(m_assist));
    for (int64_t f : m_reflex_fire) w.I64(f);
    w.I64(m_banner_at.x);
    w.I64(m_banner_at.y);
    w.U64(m_audience_turn);
    w.U64(m_tendency_after);
    w.U64(m_group_use);
    w.U64(m_group_turn);
    w.I64((int64_t)m_group_members.size());
    for (const DaiThingRef &r : m_group_members) { w.Ref(r); }
    w.I64(m_group_at.x);
    w.I64(m_group_at.y);
    w.U32(m_off_id);
    w.U64(m_off_since);
    w.Bool(m_off_advanced);
    w.I64((int64_t)m_waves.size());
    for (DaiTurn t : m_waves) w.U64(t);
    w.Bool(m_off_summoned);
}

bool DaiTactics::Load(DaiBlobReader &r)
{
    Reset();
    m_next = r.U64();
    m_engagements.resize((size_t)r.Count(MAX_ENGAGEMENTS));
    for (DaiEngagement &e : m_engagements) {
        e.key = r.U64();
        e.battle = r.Bool();
        e.x = (DaiSlabCoord)r.Range(-1, 4096); e.y = (DaiSlabCoord)r.Range(-1, 4096);
        e.own_force = r.I64(); e.enemy_force = r.I64(); e.own_count = r.I64(); e.enemy_count = r.I64(); e.enemy_levels = r.I64();
        e.own_ground = r.Bool();
        e.imps_near = r.Range(0, 1 << 20);
        e.objective = r.U32();
        e.kind = (DaiObjectiveKind)r.Range(0, (int64_t)DaiObjectiveKind::Count - 1);
        e.stakes = r.I64();
        e.deficit = r.I64(); e.fighters = r.I64(); e.gold = r.I64();
        e.banner = r.Bool(); e.group = r.Bool();
    }
    m_banner_key = r.U64();
    m_group_key = r.U64();
    m_group_objective = r.U32();
    m_free_force = r.I64();
    m_home_reserve = r.I64();
    m_tally.resize((size_t)r.Count(DAI_MAX_OBJECTIVES));
    for (Tally &t : m_tally) { t.id = r.U32(); t.own_count = r.I64(); t.enemy_count = r.I64(); t.ratio = r.I64(); t.engaged = r.Bool(); }
    if (r.I64() != (int64_t)sizeof(m_assist)) r.Fail("the casting state's size");
    else r.Bytes(reinterpret_cast<uint8_t *>(&m_assist), sizeof(m_assist));
    for (int64_t &f : m_reflex_fire) f = r.I64();
    m_banner_at.x = (DaiSlabCoord)r.Range(-1, 4096);
    m_banner_at.y = (DaiSlabCoord)r.Range(-1, 4096);
    m_audience_turn = r.U64();
    m_tendency_after = r.U64();
    m_group_use = r.U64();
    m_group_turn = r.U64();
    const int64_t members = r.Count(1 + EXTSEAT_MAX_HAND_SET);
    for (int64_t i = 0; (i < members) && r.Ok(); i++) {
        DaiThingRef t;
        t = r.Ref();
        m_group_members.push_back(t);
    }
    m_group_at.x = (DaiSlabCoord)r.Range(-1, 4096);
    m_group_at.y = (DaiSlabCoord)r.Range(-1, 4096);
    m_off_id = r.U32();
    m_off_since = r.U64();
    m_off_advanced = r.Bool();
    const int64_t waves = r.Count(100);
    for (int64_t i = 0; (i < waves) && r.Ok(); i++) m_waves.push_back((DaiTurn)r.U64());
    m_off_summoned = r.Bool();
    if (!r.Ok()) Reset();
    return r.Ok();
}
