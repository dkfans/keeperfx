#include "pre_inc.h"
#include "daimon/DaiDefence.h"

#include <algorithm>
#include <cstring>

#include "daimon/DaiCreatures.h"
#include "daimon/DaiFortify.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiStockPlanner.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
constexpr uint64_t KEY_ROOM = 1ULL << 32, KEY_IMPS = 2ULL << 32, KEY_INTERCEPT = 3ULL << 32, KEY_SPEND = 4ULL << 32,
                   KEY_WARD = 5ULL << 32, KEY_GUARD = 6ULL << 32, KEY_GUARD_OFF = 7ULL << 32, KEY_HOLD = 8ULL << 32,
                   KEY_LINING = 9ULL << 32, KEY_REINFORCE = 10ULL << 32;
/** Slabs of the perimeter further apart than this are of different gaps: a lining dig and a reinforce drop for each. */
constexpr int GAP_APART = 6;
/** P5.13: a creature spending a trap's shots is relieved below this health (%); a ward goes on an imp within WARD_NEAR
 *  slabs of a hot hazard, one each WARD_GAP turns; a seal's door within SEAL_REACH slabs of it; a vein within
 *  WORTH_REACH slabs past its reach is what makes it worth spending on. */
constexpr int64_t SPEND_RELIEVE_PCT = 40, WARD_NEAR = 2, SEAL_REACH = 8, WORTH_REACH = 2;
/** What lies past a hazard, added to the need to be rid of it (owner, 2026-10-31; DaiFix): its veins' gold in units
 *  of spend_gold_imps imps' price, up to VEIN_NEED_MAX (any gem slab: the most -- gems never run out); the strongest
 *  chokepoint within its reach, CHOKE_NEED_MAX for the top-scored one; TRAFFIC_NEED when the keeper's own creatures
 *  walk within its reach. */
constexpr DaiFix VEIN_NEED_MAX = 4 * DAI_FIX_ONE, CHOKE_NEED_MAX = 2 * DAI_FIX_ONE, TRAFFIC_NEED = DAI_FIX_ONE;
constexpr DaiTurn WARD_GAP = 200;
/** P5.13: rival imps seen this often on a slab of its floor (counted each defence interval, sightings within a slab
 *  merged) make it an imp trap's site; a site not seen for IMP_FORGET turns is forgotten. */
constexpr int64_t IMP_TRAP_SEEN = 3, MAX_IMP_SITES = 32;
constexpr DaiTurn IMP_FORGET = 3000;
constexpr uint64_t GUARDED_ROLES = DAI_ROLE_GOLD | DAI_ROLE_SPELLBOOKS | DAI_ROLE_LAIR | DAI_ROLE_FOOD_SPAWN | DAI_ROLE_MANUFACTURE
                                 | DAI_ROLE_TRAIN | DAI_ROLE_RESEARCH;
constexpr int64_t MAX_IMP_OBJECTIVES = 4;
constexpr DaiTurn SHUN_TURNS = 1000;
/** Locks are looked at this often. */
constexpr DaiTurn LOCK_EVERY = 10;
constexpr DaiFix PLACE_UTILITY = 5 * DAI_FIX_ONE, LOCK_UTILITY = 8 * DAI_FIX_ONE;
/** A guard post: its slabs, how near a chokepoint one counts as guarding it, the digging it may cost, how long an ask
 *  stands and the turns before the next. */
constexpr int64_t GUARD_SLABS = 2, GUARD_NEAR = 4, GUARD_BUDGET = 2000;
constexpr DaiTurn GUARD_ASK_TURNS = 3000;
/** The hold's guards kept at most (a saved list). */
constexpr int64_t DAI_MAX_GUARDS = 64;

DaiRect around(DaiSlabCoord x, DaiSlabCoord y, int64_t r)
{
    return DaiRect{ (DaiSlabCoord)(x - r), (DaiSlabCoord)(y - r), (DaiSlabCoord)(x + r), (DaiSlabCoord)(y + r) };
}

uint64_t slab_key(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y) { return (uint64_t)((int64_t)y * k.Width() + x); }

/** A slab the keeper's creatures walk on, as the reach map has it: floor, rooms, water, its own doors. */
bool walkable(const DaiSlabInfo &s, PlayerNumber p)
{
    if (!s.revealed) return false;
    switch (s.terrain) {
    case DaiTerrain::Floor: case DaiTerrain::Room: case DaiTerrain::Water: return true;
    case DaiTerrain::Door: return s.owner == p;
    default: return false;
    }
}

/** No walk (four sides) from `from` to `to` with the slab `cut` closed (off the map: none closed). */
bool cut_off(const DaiKnowledge &k, PlayerNumber p, DaiSlabXY from, DaiSlabXY cut, DaiSlabXY to)
{
    const int64_t w = k.Width(), h = k.Height();
    if ((from.x < 0) || (from.y < 0) || (from.x >= w) || (from.y >= h)) return true;
    std::vector<uint8_t> seen((size_t)(w * h), 0);
    std::vector<DaiSlabXY> queue{ from };
    seen[(size_t)(from.y * w + from.x)] = 1;
    static const int dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
    for (size_t i = 0; i < queue.size(); i++) {
        const DaiSlabXY a = queue[i];
        if ((a.x == to.x) && (a.y == to.y)) return false;
        for (int d = 0; d < 4; d++) {
            const DaiSlabCoord x = (DaiSlabCoord)(a.x + dx[d]), y = (DaiSlabCoord)(a.y + dy[d]);
            if ((x < 0) || (y < 0) || (x >= w) || (y >= h) || seen[(size_t)(y * w + x)] || ((x == cut.x) && (y == cut.y))) continue;
            seen[(size_t)(y * w + x)] = 1;
            if (walkable(k.SlabAt(x, y), p)) queue.push_back(DaiSlabXY{ x, y });
        }
    }
    return true;
}
}

void DaiDefence::Reset()
{
    m_next = 0;
    m_facts_seen = 0;
    m_shunned.clear();
    m_planned = m_wanted = 0;
    m_guard_next = 0;
    m_seals.clear();
    m_seal_door = 0;
    m_ward_next = 0;
    m_rival_imps.clear();
    m_guards.clear();
    m_hold_at = DaiSlabXY{ -1, -1 };
    m_hold_until = 0;
}

void DaiDefence::standing(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    std::vector<uint64_t> keep;
    for (const DaiRoomLook &r : k.Rooms()) {
        if ((r.roles & DAI_ROLE_HEART) != 0) {
            DaiObjective o;
            o.kind = DaiObjectiveKind::HoldHeart;
            o.area = r.box;
            o.stakes = c.personality.defence_heart_priority;
            w.PostObjective(o);
            continue;
        }
        if ((r.roles & GUARDED_ROLES) == 0) continue;
        int64_t threat = 0;
        for (DaiSlabCoord y = r.box.y0; y <= r.box.y1; y++)
            for (DaiSlabCoord x = r.box.x0; x <= r.box.x1; x++) threat = std::max(threat, c.threat.ThreatAt(x, y, c.now));
        if (threat < c.cfg.defence_area_threat) continue;
        DaiObjective o;
        o.kind = DaiObjectiveKind::HoldArea;
        o.key = KEY_ROOM | slab_key(k, r.ax, r.ay);
        o.area = r.box;
        o.stakes = c.personality.defence_rooms_priority;
        w.PostObjective(o);
        keep.push_back(o.key);
    }
    int64_t imp_objectives = 0;
    std::vector<DaiSlabXY> centres;
    for (const DaiImpLook &i : k.OwnImps()) {
        const DaiSlabCoord x = DaiSubtileSlab(i.x), y = DaiSubtileSlab(i.y);
        if (c.threat.ThreatAt(x, y, c.now) < c.cfg.defence_imps_threat) continue;
        bool near = false;
        for (const DaiSlabXY &s : centres) near = near || (DaiChess(s.x, s.y, x, y) <= 3);
        if (near || (imp_objectives >= MAX_IMP_OBJECTIVES)) continue;
        centres.push_back(DaiSlabXY{ x, y });
        DaiObjective o;
        o.kind = DaiObjectiveKind::ProtectImps;
        o.key = KEY_IMPS | slab_key(k, x, y);
        o.area = around(x, y, 2);
        o.stakes = c.personality.defence_imps_priority;
        w.PostObjective(o);
        keep.push_back(o.key);
        imp_objectives++;
    }
    // Rooms and imps no longer under threat: their objectives go.
    std::vector<uint32_t> drop;
    c.objectives.ForEach([&](const DaiObjective &o) {
        if ((o.owner != DaiDomain::Defence) || ((o.kind != DaiObjectiveKind::HoldArea) && (o.kind != DaiObjectiveKind::ProtectImps))) return;
        if (std::find(keep.begin(), keep.end(), o.key) == keep.end()) drop.push_back(o.id);
    });
    for (uint32_t id : drop) w.DropObjective(id);
}

void DaiDefence::invasions(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const uint32_t mask = (1u << KF_Intruder) | (1u << KF_TrapFired);
    DaiTurn newest = m_facts_seen;
    for (int64_t i = c.inbox.Next(mask, 0); i >= 0; i = c.inbox.Next(mask, i + 1)) {
        const DaiInboxEntry &e = c.inbox.Get(i);
        if (e.turn <= m_facts_seen) continue;
        newest = std::max(newest, e.turn);
        if (e.kind == KF_TrapFired) {
            // Only its own alarm: a trap that only warns.
            const int64_t model = e.args[1];
            const DaiStockLook &st = k.Stock();
            if ((e.args[2] != c.player) || (strcmp(st.Name(false, model), "ALARM") != 0)) continue;
        }
        const DaiSlabCoord x = DaiSubtileSlab(e.x), y = DaiSubtileSlab(e.y);
        DaiObjective o;
        o.kind = DaiObjectiveKind::Intercept;
        o.key = KEY_INTERCEPT | slab_key(k, x, y);
        o.area = around(x, y, 2);
        o.stakes = c.personality.defence_rooms_priority;
        o.until = c.now + (DaiTurn)c.cfg.defence_intercept_turns;
        w.PostObjective(o);
    }
    m_facts_seen = newest;
}

void DaiDefence::lining(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    // Each gap's first lining slab not yet the keeper's open ground: a dig request for it (claimed) -- the route there
    // from its land is the corridor out to the gap (level 61: a corridor from the heart to each gap, a bar across it).
    // Every gap at once (one slab each: the slabs of a gap lie together): level 61's second gap, lined after the first,
    // was a rival's floor at one end by then.
    const DaiKnowledge &k = c.know;
    std::vector<DaiSlabXY> asked;
    auto gap_asked = [&](const DaiSlabXY &s) {
        for (const DaiSlabXY &a : asked) if (std::max(std::abs((int)a.x - (int)s.x), std::abs((int)a.y - (int)s.y)) <= GAP_APART) return true;
        return false;
    };
    for (const DaiSlabXY &s : k.Lining()) {
        if (gap_asked(s)) continue;
        const DaiSlabInfo i = k.SlabAt(s.x, s.y);
        const bool done = (i.owner == c.player)
            && ((i.terrain == DaiTerrain::Floor) || (i.terrain == DaiTerrain::Room) || (i.terrain == DaiTerrain::Door));
        if (done || (i.terrain == DaiTerrain::Rock) || (i.terrain == DaiTerrain::Gem)) continue;
        if ((i.terrain == DaiTerrain::Floor) && (i.owner >= 0) && (i.owner != c.player)) continue;   // another's: not ours to dig
        DaiRequest q;
        q.kind = DaiRequestKind::Service;
        q.to = DaiDomain::Excavation;
        q.service = DaiService::Dig;
        q.key = (int64_t)(KEY_LINING + (uint64_t)s.y * (uint64_t)k.Width() + (uint64_t)s.x);
        q.at = s;
        q.expires = c.now + 2 * (DaiTurn)c.cfg.defence_interval;
        snprintf(q.why, sizeof(q.why), "LINING");
        w.PostRequest(q);
        asked.push_back(s);
    }
    // The gap's earth with the keeper's floor beside it: an imp dropped there reinforces it (the engine's imps
    // reinforce only with fewer than 32 tasks stacked, from the heart outward -- level 61's gaps, 23 slabs out, never
    // were). The first such slab of each gap, one imp.
    asked.clear();
    for (const DaiSlabXY &s : k.KeptSlabs()) {
        if (gap_asked(s)) continue;
        const DaiSlabInfo i = k.SlabAt(s.x, s.y);
        if ((i.terrain != DaiTerrain::Dirt) || !i.revealed) continue;
        bool foothold = false;
        const DaiSlabXY nb[4] = { { s.x, (DaiSlabCoord)(s.y - 1) }, { (DaiSlabCoord)(s.x + 1), s.y }, { s.x, (DaiSlabCoord)(s.y + 1) }, { (DaiSlabCoord)(s.x - 1), s.y } };
        for (const DaiSlabXY &b : nb) {
            const DaiSlabInfo f = k.SlabAt(b.x, b.y);
            foothold = foothold || ((f.owner == c.player) && ((f.terrain == DaiTerrain::Floor) || (f.terrain == DaiTerrain::Room)));
        }
        if (!foothold) continue;
        DaiRequest q;
        q.kind = DaiRequestKind::Service;
        q.to = DaiDomain::Excavation;
        q.service = DaiService::Reinforce;
        q.key = (int64_t)(KEY_REINFORCE + (uint64_t)s.y * (uint64_t)k.Width() + (uint64_t)s.x);
        q.at = s;
        q.amount = 1;
        q.expires = c.now + 2 * (DaiTurn)c.cfg.defence_interval;
        snprintf(q.why, sizeof(q.why), "PERIMETER");
        w.PostRequest(q);
        asked.push_back(s);
    }
}

void DaiDefence::fortify(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiStockLook &st = k.Stock();
    // The director's contest Defend goals (key 1 + the rival): fortify against that rival's routes first.
    uint32_t contested = 0;
    std::vector<const DaiGoal *> defend;
    c.goals.ForEach([&](const DaiGoal &g) {
        if (g.kind != DaiGoalKind::Defend) return;
        defend.push_back(&g);
        if ((g.p.key > DAI_DEFEND_HOME) && (g.p.key <= 32)) contested |= 1u << (g.p.key - 1);
    });
    std::vector<DaiFortImpSite> imp_sites;
    for (const ImpSeen &s : m_rival_imps) if (s.seen >= IMP_TRAP_SEEN) imp_sites.push_back(DaiFortImpSite{ s.at, s.seen });
    const std::vector<DaiFortPlacement> plan = DaiFortify::Plan(k, c.threat, c.player, c.cfg, c.personality, contested, imp_sites);
    const DaiFortWho who = DaiFortify::Who(k);
    m_planned = (int64_t)plan.size();
    {
        // Heart traps (P9): logged as the sites it finds change (rooms packed about the heart leave little floor).
        int64_t sites = 0;
        for (const DaiFortPlacement &f : plan) sites += (f.role == DaiFortRole::Heart) ? 1 : 0;
        if (sites != m_heart_sites_logged) {
            DaiLog::Info(c.player, "defence: %" PRId64 " heart trap sites planned (heart_traps %" PRId64 ")", sites, c.personality.defence_heart_traps);
            m_heart_sites_logged = sites;
        }
    }
    for (const DaiGoal *g : defend) {
        const DaiGoalStatus status = plan.empty() ? DaiGoalStatus::Met : DaiGoalStatus::Progress;
        if (g->status != status) w.NoteGoal(g->id, status, plan.empty() ? "" : "FORTIFYING");
    }
    // Secret doors (the personality's secret_doors) only where the enemy has not seen (SecretSite); a Midas door's hits
    // paid from the spare gold, shared by the doors planned.
    const bool secret_ok = (c.personality.defence_secret_doors != 0);
    int64_t ndoors = 0;
    for (const DaiFortPlacement &f : plan) ndoors += (f.role == DaiFortRole::Door) ? 1 : 0;
    const int64_t door_gold = std::max<int64_t>(c.forecast.spendable, 0) / std::max<int64_t>(ndoors, 1);
    // What the stock lacks: asked of the workshop as the Manufacture goal's floor, as Construction asks for its doors.
    std::vector<DaiStockWant> wants;
    auto want = [&](bool door, int64_t kind) {
        for (DaiStockWant &x : wants) if ((x.door == door) && (x.kind == kind)) { x.count++; return; }
        wants.push_back(DaiStockWant{ door, kind, 1 });
    };
    for (const DaiFortPlacement &f : plan) {
        const bool door = (f.role == DaiFortRole::Door);
        const bool secret = door && secret_ok && DaiFortify::SecretSite(k, f.at);
        const int64_t kind = door ? DaiFortify::DoorKind(st, false, secret, &c.know.Content(), who, door_gold)
                                  : DaiFortify::TrapKind(st, f.role, f.traffic, false, who, &c.know.Content());
        if (kind != 0) want(door, kind);
    }
    if (m_seal_door != 0) want(true, m_seal_door);      // a hazard's seal (hazards() runs first)
    const DaiStockPlan sp = DaiStockPlanner::Plan(k, wants);
    m_wanted = sp.short_items;
    const DaiRequest *asked = c.requests.Find(DaiDomain::Defence, DaiGoalKind::Manufacture, 0);
    if (sp.workers > 0) {
        if ((asked == nullptr) || (asked->count_min != sp.workers)) {
            DaiRequest r;
            r.goal = DaiGoalKind::Manufacture;
            r.count_min = sp.workers;
            snprintf(r.why, sizeof(r.why), "DEFENCE");
            w.PostRequest(r);
        }
    } else if (asked != nullptr) {
        w.WithdrawRequest(asked->id);
    }
    // A landing or a moat's post (P5.14): the best one asked of Construction as a one-slab guard post, one at a time.
    for (const DaiFortPlacement &f : plan) {
        if ((f.role != DaiFortRole::Landing) && (f.role != DaiFortRole::MoatPost)) continue;
        DaiRequest q;
        q.kind = DaiRequestKind::Service;
        q.to = DaiDomain::Construction;
        q.service = DaiService::PlaceRoomAt;
        q.key = (int64_t)DaiRoom::GuardPost;
        q.amount = 1;
        q.at = f.at;
        q.expires = c.now + GUARD_ASK_TURNS;
        snprintf(q.why, sizeof(q.why), "%s", (f.role == DaiFortRole::Landing) ? "LANDING" : "MOAT");
        w.PostRequest(q);
        break;
    }
    // One placement at a time, the best in stock.
    bool busy = false;
    c.jobs.ForEach([&](const DaiJob &j) {
        busy = busy || (((j.kind == DAI_JOB_PLACE_TRAP) || (j.kind == DAI_JOB_PLACE_DOOR)) && (j.state == DaiJobState::Ready));
    });
    if (busy) return;
    m_shunned.erase(std::remove_if(m_shunned.begin(), m_shunned.end(), [&](const Shunned &s) { return s.until <= c.now; }), m_shunned.end());
    for (const DaiFortPlacement &f : plan) {
        bool shunned = false;
        for (const Shunned &s : m_shunned) shunned = shunned || ((s.at.x == f.at.x) && (s.at.y == f.at.y));
        if (shunned || f.held) continue;
        const bool door = (f.role == DaiFortRole::Door);
        const bool secret = door && secret_ok && DaiFortify::SecretSite(k, f.at);
        const int64_t kind = door ? DaiFortify::DoorKind(st, true, secret, &c.know.Content(), who, door_gold)
                                  : DaiFortify::TrapKind(st, f.role, f.traffic, true, who, &c.know.Content());
        if (kind == 0) continue;
        const DaiJobKindId jk = door ? DAI_JOB_PLACE_DOOR : DAI_JOB_PLACE_TRAP;
        if (w.FindJob(jk, slab_key(k, f.at.x, f.at.y)) != nullptr) continue;
        DaiJob *j = w.AddJob(jk, slab_key(k, f.at.x, f.at.y), PLACE_UTILITY);
        if (j != nullptr) j->p = DaiDoorParams{ f.at, kind };
        return;
    }
}

void DaiDefence::guard_posts(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    if ((c.personality.defence_guard_posts == 0) || (c.now < m_guard_next) || !k.Build(DaiRoom::GuardPost).available) return;
    std::vector<DaiSlabXY> posts;
    for (const DaiRoomLook &r : k.Rooms()) if ((r.roles & DAI_ROLE_GUARD) != 0) posts.push_back(DaiSlabXY{ r.cx, r.cy });
    if ((int64_t)posts.size() >= c.cfg.defence_max_guard_posts) return;
    // The busiest own chokepoint no guard post stands by yet.
    const DaiChokepoint *best = nullptr;
    for (const DaiChokepoint &cp : c.threat.Chokepoints()) {   // busiest first
        if (!cp.own) continue;
        bool guarded = false;
        for (const DaiSlabXY &g : posts) guarded = guarded || (DaiChess(g.x, g.y, cp.x, cp.y) <= GUARD_NEAR);
        if (!guarded) { best = &cp; break; }
    }
    if (best != nullptr) ask_guard_post(c, w, *best);
}

/** A small guard post asked of Construction beside a chokepoint; the next ask GUARD_ASK_TURNS later. */
bool DaiDefence::ask_guard_post(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiChokepoint &cp)
{
    if ((c.now < m_guard_next) || !c.know.Build(DaiRoom::GuardPost).available) return false;
    const DaiChokepoint *best = &cp;
    DaiRequest q;
    q.kind = DaiRequestKind::Service;
    q.to = DaiDomain::Construction;
    q.service = DaiService::PlaceRoom;
    q.key = (int64_t)DaiRoom::GuardPost;
    q.amount = GUARD_SLABS;
    q.at = DaiSlabXY{ best->x, best->y };
    q.budget = GUARD_BUDGET;
    q.expires = c.now + GUARD_ASK_TURNS;
    snprintf(q.why, sizeof(q.why), "GUARD");
    if (w.PostRequest(q) == 0) return false;
    m_guard_next = c.now + GUARD_ASK_TURNS;
    return true;
}

/** A guard's turn over (or the hold ended): carried to its lair, from where Creatures puts it to work again. */
void DaiDefence::release(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiThingRef &ref)
{
    const DaiKnowledge &k = c.know;
    const DaiCreatureLook *cr = k.Creature(ref);
    if ((cr == nullptr) || (cr->battle != 0) || (cr->held_by >= 0)) return;
    // Only one still at its post: one that walked off to eat or sleep is about its own needs already.
    bool posted = false;
    for (const DaiRoomLook &r : k.Rooms()) posted = posted || (((r.roles & DAI_ROLE_GUARD) != 0) && (r.kind == cr->in_room));
    if (!posted) return;
    const DaiRoomLook *lair = nullptr;
    if (lair == nullptr) lair = k.FirstRoomWith(DAI_ROLE_LAIR);
    if ((lair == nullptr) || (w.FindJob(DAI_JOB_SEND_TO_LAIR, KEY_GUARD_OFF | ref.idx) != nullptr)) return;
    DaiJob *j = w.AddJob(DAI_JOB_SEND_TO_LAIR, KEY_GUARD_OFF | ref.idx, FixFromInt(1));
    if (j == nullptr) return;
    DaiHandParams p;
    p.things.push_back(ref);
    p.room = lair->index;
    j->p = p;
}

/** Holding the way in (plan 13 L3): with the force estimate against the keeper and an own chokepoint under threat
 *  (area_threat; the most threatened), it is held -- a HoldArea there, a guard post by it, hold_share % of the free
 *  fighters carried to the post, each for hold_turns. Otherwise its guards go back to work. */
void DaiDefence::hold(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    // The own chokepoint under the most threat (the busiest on a tie: they come busiest first).
    const DaiChokepoint *cp = nullptr;
    int64_t cp_threat = 0;
    for (const DaiChokepoint &q : c.threat.Chokepoints()) {
        if (!q.own) continue;
        const int64_t t = c.threat.ThreatAt(q.x, q.y, c.now);
        if ((cp == nullptr) || (t > cp_threat)) { cp = &q; cp_threat = t; }
    }
    bool known = false;
    const bool favourable = c.rivals.Favourable(k, c.personality, c.cfg, c.forecast, -1, &known);
    const bool wanted_now = (c.personality.defence_hold_share > 0) && known && !favourable && (cp != nullptr)
        && (cp_threat >= c.cfg.defence_area_threat) && k.Build(DaiRoom::GuardPost).available;
    // A hold lasts hold_turns from the last time it was wanted: the estimate wavers about the line as creatures come and
    // go, and each end sent the guards home (P9: started 1,051 times and ended 505 in 96 games). Held where it was while
    // the chokepoint is still one of its own.
    if (wanted_now) m_hold_until = c.now + (DaiTurn)c.cfg.defence_hold_turns;
    if (!wanted_now && (c.now < m_hold_until) && (m_hold_at.x >= 0)) {
        cp = nullptr;
        for (const DaiChokepoint &q : c.threat.Chokepoints())
            if (q.own && (q.x == m_hold_at.x) && (q.y == m_hold_at.y)) cp = &q;
    }
    const bool want = (cp != nullptr) && (wanted_now || (c.now < m_hold_until));
    // Guards whose turn is over, gone or not, leave the list; those still at the post are carried back.
    std::vector<Guard> stay;
    for (const Guard &g : m_guards) {
        if (k.Creature(g.ref) == nullptr) continue;
        if (!want || (c.now >= g.since + (DaiTurn)c.cfg.defence_hold_turns)) { release(c, w, g.ref); continue; }
        stay.push_back(g);
    }
    m_guards = stay;
    const DaiSlabXY before = m_hold_at;
    if (!want) {
        m_hold_at = DaiSlabXY{ -1, -1 };
        if (before.x >= 0) DaiLog::Info(c.player, "defence: hold at (%d,%d) ended", (int)before.x, (int)before.y);
        return;
    }
    m_hold_at = DaiSlabXY{ cp->x, cp->y };
    if ((before.x != m_hold_at.x) || (before.y != m_hold_at.y))
        DaiLog::Info(c.player, "defence: hold at (%d,%d), threat %" PRId64 ", the force estimate against it", (int)cp->x, (int)cp->y, cp_threat);
    DaiObjective o;
    o.kind = DaiObjectiveKind::HoldArea;
    o.key = KEY_HOLD;
    o.area = around(cp->x, cp->y, 2);
    o.stakes = c.personality.defence_rooms_priority;
    w.PostObjective(o);
    // Its guard post: one within GUARD_NEAR of the chokepoint, else asked for.
    const DaiRoomLook *post = nullptr;
    for (const DaiRoomLook &r : k.Rooms())
        if (((r.roles & DAI_ROLE_GUARD) != 0) && (DaiChess(r.cx, r.cy, cp->x, cp->y) <= GUARD_NEAR)) post = &r;
    if (post == nullptr) { ask_guard_post(c, w, *cp); return; }
    // hold_share % of the free fighters (at least one), no more than the post holds.
    int64_t free = 0;
    for (const DaiCreatureLook &x : k.Creatures())
        if (!k.Kind(x.model).digger && (x.battle == 0) && (x.held_by < 0)) free++;
    int64_t wanted = std::max<int64_t>(1, (free * c.personality.defence_hold_share + 99) / 100);
    if (post->capacity > 0) wanted = std::min<int64_t>(wanted, post->capacity);
    const int64_t need = wanted - (int64_t)m_guards.size();
    if ((need <= 0) || (w.FindJob(DAI_JOB_MOVE_TO_JOB, KEY_GUARD) != nullptr)) return;
    // The idle first, then those at work (not training: an attack waits on it), the highest level first.
    std::vector<const DaiCreatureLook *> pick;
    for (const DaiCreatureLook &x : k.Creatures()) {
        if (k.Kind(x.model).digger || (x.battle != 0) || (x.held_by >= 0) || (x.anger == DaiAnger::Livid)) continue;
        if ((x.activity != DaiActivity::Idle) && (x.activity != DaiActivity::Work)) continue;
        if (x.health_pct * 256 < k.Kind(x.model).heal_requirement * 100) continue;
        bool skip = false;
        for (const Guard &g : m_guards) skip = skip || (g.ref == x.ref);
        for (const DaiRoomLook &r : k.Rooms())
            skip = skip || ((r.kind == x.in_room) && ((r.roles & (DAI_ROLE_TRAIN | DAI_ROLE_GUARD)) != 0));
        if (!skip) pick.push_back(&x);
    }
    std::stable_sort(pick.begin(), pick.end(), [](const DaiCreatureLook *a, const DaiCreatureLook *b) {
        if ((a->activity == DaiActivity::Idle) != (b->activity == DaiActivity::Idle)) return a->activity == DaiActivity::Idle;
        return a->level > b->level;
    });
    if (pick.empty()) return;
    DaiJob *j = w.AddJob(DAI_JOB_MOVE_TO_JOB, KEY_GUARD, FixFromInt(2));
    if (j == nullptr) return;
    DaiHandParams p;
    p.room = post->index;
    for (const DaiCreatureLook *x : pick) {
        if (((int64_t)p.things.size() >= std::min<int64_t>(need, DAI_MOVE_PER_TRIP))) break;
        p.things.push_back(x->ref);
        m_guards.push_back(Guard{ x->ref, c.now });
    }
    j->p = p;
}

void DaiDefence::locks(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    bool busy = false;
    c.jobs.ForEach([&](const DaiJob &j) { busy = busy || ((j.kind == DAI_JOB_SET_DOOR_LOCK) && !DaiJobEnded(j.state)); });
    if (busy) return;
    for (const DaiDoorLook &d : k.Doors()) {
        const DaiSlabCoord x = DaiSubtileSlab(d.x), y = DaiSubtileSlab(d.y);
        bool enemy = false;
        for (const DaiThingLook &e : k.Enemies()) enemy = enemy || (DaiChess(DaiSubtileSlab(e.x), DaiSubtileSlab(e.y), x, y) <= c.cfg.defence_lock_radius);
        // Never a door its own creatures need (Q6); a hazard's seal always (P5.13).
        bool seal = false;
        for (const DaiSlabXY &s : m_seals) seal = seal || ((s.x == x) && (s.y == y));
        const bool lock = seal || (enemy && (c.threat.TrafficAt(x, y) == 0));
        if (d.locked == lock) continue;
        DaiJob *j = w.AddJob(DAI_JOB_SET_DOOR_LOCK, slab_key(k, x, y), LOCK_UTILITY);
        if (j != nullptr) j->p = DaiLockParams{ DaiSlabXY{ x, y }, lock };
        return;
    }
}

/** A trap's kind name (trapdoor.cfg) by its model; "" if unknown. */
static const char *trap_name(const DaiKnowledge &k, DaiThingModel model)
{
    const DaiStockLook &st = k.Stock();
    return st.Name(false, model);
}

/** What lies past a hazard (`hazard_importance`, DaiFix): 0 when nothing the keeper wants is there. */
static DaiFix hazard_importance(const DaiTurnCtx &c, const DaiHazard &h)
{
    const DaiKnowledge &k = c.know;
    const int64_t reach = c.cfg.defence_hazard_reach, r = reach + WORTH_REACH;
    std::vector<uint32_t> veins;
    int64_t gold = 0;
    bool gems = false, traffic = false;
    for (DaiSlabCoord y = (DaiSlabCoord)std::max<int64_t>(h.y - r, 0); y <= std::min<int64_t>(h.y + r, k.Height() - 1); y++)
        for (DaiSlabCoord x = (DaiSlabCoord)std::max<int64_t>(h.x - r, 0); x <= std::min<int64_t>(h.x + r, k.Width() - 1); x++) {
            traffic = traffic || ((DaiChess(x, y, h.x, h.y) <= reach) && (c.threat.TrafficAt(x, y) > 0));
            const DaiVein *v = k.VeinAt(x, y);
            if ((v == nullptr) || (std::find(veins.begin(), veins.end(), v->id) != veins.end())) continue;
            veins.push_back(v->id);
            gold += v->gold_left;
            gems = gems || ((v->gem_slabs > 0) && (v->status != DaiVeinStatus::Unreachable));   // endless while reachable
        }
    const int64_t unit = std::max<int64_t>(k.Power(DaiPower::CreateImp).price, 1) * c.cfg.defence_spend_gold_imps;
    DaiFix need = gems ? VEIN_NEED_MAX : std::min<DaiFix>(gold * DAI_FIX_ONE / unit, VEIN_NEED_MAX);
    const std::span<const DaiChokepoint> chokes = c.threat.Chokepoints();
    int64_t best = 0;
    for (const DaiChokepoint &cp : chokes) if (DaiChess(cp.x, cp.y, h.x, h.y) <= reach) best = std::max(best, cp.score);
    if ((best > 0) && (chokes.front().score > 0)) need += CHOKE_NEED_MAX * best / chokes.front().score;
    if (traffic) need += TRAFFIC_NEED;
    return need;
}

/** Hazards (07 P5.13): a hot one is sealed off with a locked door, and when it guards something the keeper wants -- a
 *  vein past it, a chokepoint or its own creatures' ways within its reach (`hazard_importance`) -- spent by a creature
 *  standing on its trap too, if one is cheap enough; its imps nearby are warded while it is hot. The seal first, always
 *  (P9): imps claim the ground by a trap and pick up the gold there unasked -- level 52's lightning traps by a vein took
 *  thirteen in a thousand turns after the marks by them came off, while a creature was on its way to spend them. */
void DaiDefence::hazards(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    std::vector<uint64_t> keep;
    std::vector<DaiSlabXY> seals;
    m_seal_door = 0;
    for (const DaiHazard &h : c.threat.Hazards()) {
        if (!c.threat.Hot(h)) continue;
        const char *trap = trap_name(k, h.model);
        ward(c, w, h, trap);
        if (seal(c, w, h)) {
            // Each seal once: two hot hazards sharing a door put it in twice, and the copies doubled every turn
            // (level 52: 51 GB in a planning turn, P9).
            for (const DaiSlabXY &s : m_seals) {
                if (DaiChess(s.x, s.y, h.x, h.y) > SEAL_REACH) continue;
                bool listed = false;
                for (const DaiSlabXY &t : seals) listed = listed || ((t.x == s.x) && (t.y == s.y));
                if (!listed) seals.push_back(s);
            }
        }
        const DaiFix importance = hazard_importance(c, h);
        if ((importance > 0) && (c.personality.defence_spend_traps != 0)) spend(c, w, h, trap, importance, keep);
    }
    m_seals = seals;
    std::vector<uint32_t> drop;
    c.objectives.ForEach([&](const DaiObjective &o) {
        if ((o.owner == DaiDomain::Defence) && (o.kind == DaiObjectiveKind::SpendTrap) && (std::find(keep.begin(), keep.end(), o.key) == keep.end()))
            drop.push_back(o.id);
    });
    for (uint32_t id : drop) w.DropObjective(id);
}

/** A hazard's rating on the creatures' force scale (owner, 2026-10-31): the force of the imps it took (the keeper's
 *  imps' mean, unhurt), scaled by the need to be rid of it -- what lies past it (1 + `importance`), times the gold: up
 *  to four times over when the keeper's gold is short of spend_gold_imps imps' price (it rises with every imp the
 *  keeper has), down to a quarter when well above it. */
static int64_t hazard_rating(const DaiTurnCtx &c, const DaiHazard &h, DaiFix importance)
{
    int64_t imp_force = 0, n = 0;
    for (const DaiImpLook &i : c.know.OwnImps()) { imp_force += i.force; n++; }
    imp_force = (n > 0) ? std::max<int64_t>(imp_force / n, 1) : 1;
    const int64_t price = std::max<int64_t>(c.know.Power(DaiPower::CreateImp).price, 1);
    const int64_t gold = std::max<int64_t>(c.know.Own().gold, 1);
    const int64_t scarce = std::clamp<int64_t>(price * c.cfg.defence_spend_gold_imps * DAI_FIX_ONE / gold, DAI_FIX_ONE / 4, 4 * DAI_FIX_ONE);
    return (int64_t)h.losses.size() * imp_force * scarce / DAI_FIX_ONE * (DAI_FIX_ONE + importance) / DAI_FIX_ONE;
}

/** Spend: one creature at a time sent to stand on the trap until the trap shows spent; relieved when hurt. The one that
 *  costs least to lose: nothing if immune to the trap (Q9: poison gas, boulder), else its force -- and only one whose
 *  force is no more than the trap's rating (`hazard_rating`); the sturdier of equals. Never one in a fight, fleeing, or
 *  the Order mode would refuse. False if none is sent or on its way (the hazard is sealed instead). */
bool DaiDefence::spend(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiHazard &h, const char *trap, DaiFix importance,
                       std::vector<uint64_t> &keep)
{
    const DaiKnowledge &k = c.know;
    const uint64_t key = KEY_SPEND | slab_key(k, h.x, h.y);
    const DaiObjective *was = nullptr;
    c.objectives.ForEach([&](const DaiObjective &o) {
        if ((o.owner == DaiDomain::Defence) && (o.kind == DaiObjectiveKind::SpendTrap) && (o.key == key)) was = &o;
    });
    if (was != nullptr) {
        for (const DaiCreatureLook &cr : k.Creatures())
            if ((cr.ref == was->target) && (cr.health_pct >= SPEND_RELIEVE_PCT) && (cr.activity != DaiActivity::Unconscious)) {
                keep.push_back(key);
                return true;
            }
    }
    const bool gas = (strcmp(trap, "POISON_GAS") == 0), boulder = (strcmp(trap, "BOULDER") == 0);
    const int64_t rating = hazard_rating(c, h, importance);
    const DaiCreatureLook *best = nullptr;
    int64_t best_cost = 0;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        const DaiKindLook &kind = k.Kind(cr.model);
        if (kind.digger || (cr.battle != 0) || cr.fleeing || !cr.orderable || (cr.activity == DaiActivity::Unconscious)) continue;
        if ((cr.health_pct < 90) || ((was != nullptr) && (cr.ref == was->target))) continue;
        const bool immune = (gas && kind.immune_gas) || (boulder && kind.immune_boulder);
        const int64_t cost = immune ? 0 : cr.force;
        if (cost > rating) continue;
        if ((best == nullptr) || (cost < best_cost) || ((cost == best_cost) && (cr.health > best->health))) {
            best = &cr;
            best_cost = cost;
        }
    }
    if (best == nullptr) return false;
    keep.push_back(key);
    DaiObjective o;
    o.kind = DaiObjectiveKind::SpendTrap;
    o.key = key;
    o.area = DaiRect{ h.x, h.y, h.x, h.y };
    o.target = best->ref;
    o.stakes = c.personality.defence_imps_priority;
    if (w.PostObjective(o) != 0)
        DaiLog::Info(c.player, "hazard at (%d,%d): creature %d (model %d, health %d, force %d) sent to spend its %s (rated %d, past it %d/%d)",
                     (int)h.x, (int)h.y, (int)best->ref.idx, (int)best->model, (int)best->health, (int)best_cost, trap, (int)rating,
                     (int)importance, (int)DAI_FIX_ONE);
    return true;
}

/** Seal: a door on the keeper's floor near the hazard (a door site off its creatures' ways) with no walk from its heart
 *  to the trap's slab round it -- what sets a pressure trap off -- placed if it has a door in stock (else the workshop
 *  is asked for one: `m_seal_door`), then kept locked while the hazard is hot. False if there is no seal and none can
 *  be placed (nor any needed: the trap is out of its creatures' reach already). */
bool DaiDefence::seal(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiHazard &h)
{
    const DaiKnowledge &k = c.know;
    const DaiSlabXY heart{ DaiSubtileSlab(k.Own().heart_x), DaiSubtileSlab(k.Own().heart_y) }, trap{ h.x, h.y };
    if (!k.Own().has_heart || cut_off(k, c.player, heart, DaiSlabXY{ -1, -1 }, trap)) return false;
    for (const DaiDoorLook &d : k.Doors()) {
        const DaiSlabXY at{ DaiSubtileSlab(d.x), DaiSubtileSlab(d.y) };
        if ((DaiChess(at.x, at.y, h.x, h.y) > SEAL_REACH) || (c.threat.TrafficAt(at.x, at.y) != 0) || !cut_off(k, c.player, heart, at, trap)) continue;
        bool known = false;
        for (const DaiSlabXY &s : m_seals) known = known || ((s.x == at.x) && (s.y == at.y));
        if (!known) m_seals.push_back(at);
        return true;
    }
    DaiSlabXY site{ -1, -1 };
    int64_t best = INT64_MAX;
    for (DaiSlabCoord y = (DaiSlabCoord)std::max<int64_t>(h.y - SEAL_REACH, 0); y <= std::min<int64_t>(h.y + SEAL_REACH, k.Height() - 1); y++)
        for (DaiSlabCoord x = (DaiSlabCoord)std::max<int64_t>(h.x - SEAL_REACH, 0); x <= std::min<int64_t>(h.x + SEAL_REACH, k.Width() - 1); x++) {
            if (!DaiFortify::DoorSite(k, c.player, x, y) || (c.threat.TrafficAt(x, y) != 0)) continue;
            const int64_t d = DaiChess(x, y, h.x, h.y);
            if ((d < best) && cut_off(k, c.player, heart, DaiSlabXY{ x, y }, trap)) { best = d; site = DaiSlabXY{ x, y }; }
        }
    if (site.x < 0) return false;
    const int64_t kind = DaiFortify::DoorKind(k.Stock(), true, false, &k.Content());
    if (kind == 0) {
        m_seal_door = DaiFortify::DoorKind(k.Stock(), false, false, &k.Content());
        return false;
    }
    if (w.FindJob(DAI_JOB_PLACE_DOOR, slab_key(k, site.x, site.y)) != nullptr) return true;
    DaiJob *j = w.AddJob(DAI_JOB_PLACE_DOOR, slab_key(k, site.x, site.y), PLACE_UTILITY);
    if (j == nullptr) return false;
    j->p = DaiDoorParams{ site, kind };
    DaiLog::Info(c.player, "hazard at (%d,%d): sealed off by a door at (%d,%d)", (int)h.x, (int)h.y, (int)site.x, (int)site.y);
    return true;
}

/** Ward: Protect on an imp by a hot lightning or Word of Power trap (less damage a shot), Speed on one by poison gas (less
 *  time in the cloud) -- while the spell costs under ward_share of an imp. */
void DaiDefence::ward(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiHazard &h, const char *trap)
{
    const DaiKnowledge &k = c.know;
    if (c.now < m_ward_next) return;
    const DaiPower power = (strcmp(trap, "POISON_GAS") == 0) ? DaiPower::Speed
        : ((strcmp(trap, "LIGHTNING") == 0) || (strcmp(trap, "WORD_OF_POWER") == 0)) ? DaiPower::Protect : DaiPower::Count;
    if (power == DaiPower::Count) return;
    const DaiPowerLook &pw = k.Power(power);
    const DaiPowerLook &imp = k.Power(DaiPower::CreateImp);
    if (!pw.available || (pw.price <= 0) || (pw.price * DAI_FIX_ONE > imp.price * c.cfg.defence_ward_share)) return;
    for (const DaiImpLook &i : k.OwnImps()) {
        if (DaiChess(DaiSubtileSlab(i.x), DaiSubtileSlab(i.y), h.x, h.y) > WARD_NEAR) continue;
        DaiJob *j = w.AddJob(DAI_JOB_TACTICS_CAST, KEY_WARD | slab_key(k, h.x, h.y), FixFromInt(3));
        if (j == nullptr) return;
        DaiCastParams p;
        p.power = power;
        p.target = i.ref;
        p.price = pw.price;
        j->p = p;
        m_ward_next = c.now + WARD_GAP;
        return;
    }
}

/** Rival imps on its floor, converting it: counted at their slab (or one within a slab already counted) each interval. */
void DaiDefence::rival_imps(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    for (const DaiThingLook &e : k.Enemies()) {
        if (!e.digger || !k.RivalKeeper(e.owner)) continue;
        const DaiSlabCoord x = DaiSubtileSlab(e.x), y = DaiSubtileSlab(e.y);
        const DaiSlabInfo s = k.SlabAt(x, y);
        if ((s.owner != c.player) || (s.terrain != DaiTerrain::Floor)) continue;
        ImpSeen *at = nullptr;
        for (ImpSeen &i : m_rival_imps) if (DaiChess(i.at.x, i.at.y, x, y) <= 1) at = &i;
        if (at == nullptr) {
            if ((int64_t)m_rival_imps.size() >= MAX_IMP_SITES) continue;
            m_rival_imps.push_back(ImpSeen{ DaiSlabXY{ x, y }, 0, 0 });
            at = &m_rival_imps.back();
        }
        if (at->last == c.now) continue;
        at->seen++;
        at->last = c.now;
    }
    m_rival_imps.erase(std::remove_if(m_rival_imps.begin(), m_rival_imps.end(), [&](const ImpSeen &i) {
        return i.last + IMP_FORGET < c.now;
    }), m_rival_imps.end());
}

void DaiDefence::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    invasions(c, w);
    if ((c.now % LOCK_EVERY) == 0) locks(c, w);
    if ((m_next != 0) && (c.now < m_next)) return;
    m_next = c.now + (DaiTurn)c.cfg.defence_interval;
    standing(c, w);
    rival_imps(c);
    hazards(c, w);
    lining(c, w);
    fortify(c, w);
    hold(c, w);
    guard_posts(c, w);
}

void DaiDefence::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    (void)w;
    if (((j.kind != DAI_JOB_PLACE_TRAP) && (j.kind != DAI_JOB_PLACE_DOOR)) || (j.state == DaiJobState::Done)) return;
    const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
    if (p != nullptr) m_shunned.push_back(Shunned{ p->at, c.now + SHUN_TURNS });
}

void DaiDefence::Save(DaiBlobWriter &w) const
{
    w.U64(m_next);
    w.U64(m_facts_seen);
    w.I64((int64_t)m_shunned.size());
    for (const Shunned &s : m_shunned) { w.I64(s.at.x); w.I64(s.at.y); w.U64(s.until); }
    w.I64(m_planned);
    w.I64(m_wanted);
    w.U64(m_guard_next);
    w.I64((int64_t)m_seals.size());
    for (const DaiSlabXY &s : m_seals) { w.I64(s.x); w.I64(s.y); }
    w.U64(m_ward_next);
    w.I64((int64_t)m_rival_imps.size());
    for (const ImpSeen &i : m_rival_imps) { w.I64(i.at.x); w.I64(i.at.y); w.I64(i.seen); w.U64(i.last); }
    w.I64(m_hold_at.x);
    w.I64(m_hold_at.y);
    w.U64(m_hold_until);
    w.I64((int64_t)m_guards.size());
    for (const Guard &g : m_guards) { w.U64(g.ref.idx); w.U64(g.ref.creation_turn); w.U64(g.since); }
}

bool DaiDefence::Load(DaiBlobReader &r)
{
    Reset();
    m_next = r.U64();
    m_facts_seen = r.U64();
    m_shunned.resize((size_t)r.Count(4096));
    for (Shunned &s : m_shunned) { s.at.x = (DaiSlabCoord)r.Range(0, 4096); s.at.y = (DaiSlabCoord)r.Range(0, 4096); s.until = r.U64(); }
    m_planned = r.I64();
    m_wanted = r.I64();
    m_guard_next = r.U64();
    m_seals.resize((size_t)r.Count(64));
    for (DaiSlabXY &s : m_seals) { s.x = (DaiSlabCoord)r.Range(0, 4096); s.y = (DaiSlabCoord)r.Range(0, 4096); }
    m_ward_next = r.U64();
    m_rival_imps.resize((size_t)r.Count(MAX_IMP_SITES));
    for (ImpSeen &i : m_rival_imps) {
        i.at.x = (DaiSlabCoord)r.Range(0, 4096); i.at.y = (DaiSlabCoord)r.Range(0, 4096); i.seen = r.Range(0, INT32_MAX); i.last = r.U64();
    }
    m_hold_at.x = (DaiSlabCoord)r.Range(-1, 4096);
    m_hold_at.y = (DaiSlabCoord)r.Range(-1, 4096);
    m_hold_until = r.U64();
    m_guards.resize((size_t)r.Count(DAI_MAX_GUARDS));
    for (Guard &g : m_guards) { g.ref.idx = (DaiThingIndex)r.U64(); g.ref.creation_turn = r.U64(); g.since = r.U64(); }
    if (!r.Ok()) Reset();
    return r.Ok();
}
