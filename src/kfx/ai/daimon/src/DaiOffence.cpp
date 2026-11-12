#include "pre_inc.h"
#include "daimon/DaiOffence.h"

#include <algorithm>
#include <cinttypes>
#include <cstdlib>
#include <cstring>

#include "daimon/DaiLog.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** The objectives' keys (one of each kind at a time): the operation's, the raid's. */
constexpr uint64_t KEY_OP = 1, KEY_RAID = 2;
/** The target's surroundings, in slabs either way: the objectives' area. */
constexpr int64_t TARGET_RADIUS = 2;

bool open_ground(const DaiSlabInfo &s)
{
    return (s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room) || (s.terrain == DaiTerrain::Door)
        || (s.terrain == DaiTerrain::Water);
}

/** The keeper's imps dig it: earth, gold, unknown ground, its own wall. */
bool diggable(const DaiSlabInfo &s, PlayerNumber p)
{
    switch (s.terrain) {
    case DaiTerrain::Unknown: case DaiTerrain::Dirt: case DaiTerrain::Gold: case DaiTerrain::DenseGold: return true;
    case DaiTerrain::Wall: return s.owner == p;
    default: return false;
    }
}

void save_target(DaiBlobWriter &w, const DaiRivalTarget &t)
{
    w.I64(t.rival); w.I64((int64_t)t.kind); w.I64(t.x); w.I64(t.y); w.I64(t.room); w.I64(t.room_index);
    w.I64(t.prize); w.I64(t.defence); w.I64(t.route_cost); w.U64(t.route_turn); w.I64(t.score);
}

DaiRivalTarget load_target(DaiBlobReader &r)
{
    DaiRivalTarget t;
    t.rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
    t.kind = (DaiTargetKind)r.Range(0, (int64_t)DaiTargetKind::Count - 1);
    t.x = (DaiSlabCoord)r.Range(0, 4096); t.y = (DaiSlabCoord)r.Range(0, 4096);
    t.room = (RoomKind)r.Range(0, 255); t.room_index = r.Range(0, INT32_MAX);
    t.prize = r.Range(0, INT64_MAX / 4); t.defence = r.Range(0, INT64_MAX / 4);
    t.route_cost = r.Range(-1, DAI_DIG_NEVER); t.route_turn = (DaiTurn)r.U64(); t.score = r.Range(0, INT64_MAX);
    return t;
}

bool same_target(const DaiRivalTarget &a, const DaiRivalTarget &b)
{
    return (a.rival == b.rival) && (a.kind == b.kind) && (a.room_index == b.room_index);
}

bool routed(const DaiRivalTarget &t) { return (t.route_cost > 0) && (t.route_cost < DAI_DIG_NEVER); }

bool own_land(const DaiSlabInfo &s, PlayerNumber p)
{
    return (s.owner == p) && ((s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room));
}

/** The keeper's own floor or room slab nearest (x, y) within r slabs (chess; of those as near, the fewest steps away,
 *  then the first in scan order). */
bool own_near(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, int64_t r, DaiSlabXY *out)
{
    int64_t best = INT64_MAX;
    for (DaiSlabCoord sy = (DaiSlabCoord)(y - r); sy <= y + r; sy++) {
        for (DaiSlabCoord sx = (DaiSlabCoord)(x - r); sx <= x + r; sx++) {
            const int64_t d = DaiChess(sx, sy, x, y) * 1024 + std::abs(sx - x) + std::abs(sy - y);
            if ((d < best) && own_land(k.SlabAt(sx, sy), p)) { best = d; *out = DaiSlabXY{ sx, sy }; }
        }
    }
    return best != INT64_MAX;
}

DaiRect around(DaiSlabXY at, int64_t r)
{
    return DaiRect{ (DaiSlabCoord)(at.x - r), (DaiSlabCoord)(at.y - r), (DaiSlabCoord)(at.x + r), (DaiSlabCoord)(at.y + r) };
}
}

const char *DaiOpPhaseName(DaiOpPhase p)
{
    switch (p) {
    case DaiOpPhase::None: return "none";
    case DaiOpPhase::Recon: return "recon";
    case DaiOpPhase::Stage: return "stage";
    case DaiOpPhase::Breach: return "breach";
    case DaiOpPhase::Assault: return "assault";
    case DaiOpPhase::Exploit: return "exploit";
    case DaiOpPhase::Withdraw: return "withdraw";
    default: return "?";
    }
}

void DaiOffence::Reset()
{
    *this = DaiOffence();
}

DaiTurn DaiOffence::CoolUntil(const DaiRivalTarget &t) const
{
    DaiTurn until = 0;
    for (const Cooled &c : m_cooled)
        if ((c.rival == t.rival) && (c.kind == t.kind) && (c.room_index == t.room_index)) until = std::max(until, c.until);
    return until;
}

DaiRect DaiOffence::target_area() const
{
    return DaiRect{ (DaiSlabCoord)(m_target.x - TARGET_RADIUS), (DaiSlabCoord)(m_target.y - TARGET_RADIUS),
                    (DaiSlabCoord)(m_target.x + TARGET_RADIUS), (DaiSlabCoord)(m_target.y + TARGET_RADIUS) };
}

bool DaiOffence::home_fight(const DaiTurnCtx &c) const
{
    // An engaged Defence objective near home whose enemy is a force ([defence] threat_force; P5.12: an enemy imp by the
    // keeper's imps engaged a ProtectImps objective and kept every operation from starting, and a fight by its imps on
    // the far frontier called an assault back). Near home is the director's Defend rule (DaiKnowledge::NearHome).
    bool fight = false;
    c.objectives.ForEach([&](const DaiObjective &o) {
        if (o.owner != DaiDomain::Defence) return;
        const DaiSlabCoord ox = (DaiSlabCoord)((o.area.x0 + o.area.x1) / 2), oy = (DaiSlabCoord)((o.area.y0 + o.area.y1) / 2);
        if (!c.know.NearHome(c.player, ox, oy, c.cfg.director_contest_radius)) return;
        const DaiObjectiveState s = o.report.state;
        const bool engaged = (s == DaiObjectiveState::Engaged) || (s == DaiObjectiveState::Winning) || (s == DaiObjectiveState::Holding)
            || (s == DaiObjectiveState::Losing);
        fight = fight || (engaged && (o.report.enemy_levels >= c.cfg.defence_threat_force));
    });
    return fight;
}

bool DaiOffence::refresh_target(const DaiTurnCtx &c)
{
    for (const DaiRivalTarget &t : c.rivals.Targets()) {
        if (!same_target(t, m_target)) continue;
        m_target = t;
        return true;
    }
    return false;
}

void DaiOffence::go(DaiOpPhase p, const DaiTurnCtx &c, const char *why)
{
    DaiLog::Info(c.player, "offence: %s -> %s against p%d's %s at (%d,%d)%s%s", DaiOpPhaseName(m_phase), DaiOpPhaseName(p),
        (int)m_target.rival, DaiTargetKindName(m_target.kind), (int)m_target.x, (int)m_target.y, (why[0] != '\0') ? ": " : "", why);
    m_phase = p;
    m_phase_since = c.now;
}

uint32_t DaiOffence::ask(const DaiTurnCtx &c, DaiManagerWrites &w, DaiDomain to, DaiService s, DaiSlabXY at, int64_t amount)
{
    DaiRequest q;
    q.kind = DaiRequestKind::Service;
    q.to = to;
    q.service = s;
    q.key = (int64_t)at.y * c.know.Width() + at.x;      // one standing request a slab
    q.at = at;
    q.amount = amount;
    const uint32_t id = w.PostRequest(q);
    if ((id != 0) && (std::find(m_requests.begin(), m_requests.end(), id) == m_requests.end())) m_requests.push_back(id);
    return id;
}

void DaiOffence::end(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why, bool won, bool cool)
{
    for (uint32_t id : m_requests) w.WithdrawRequest(id);
    m_requests.clear();
    for (uint32_t *o : { &m_scout, &m_assault, &m_withdraw }) {
        if (*o != 0) w.DropObjective(*o);
        *o = 0;
    }
    if (cool) {
        Cooled cd;
        cd.rival = m_target.rival;
        cd.kind = m_target.kind;
        cd.room_index = m_target.room_index;
        cd.until = c.now + (DaiTurn)c.cfg.offence_retry_after;
        m_cooled.push_back(cd);
    }
    if (won) m_won++;
    else m_given_up++;
    // A fight at home comes and goes as the enemy wanders: none begun again for a while.
    if (strcmp(why, "HOME_FIGHT") == 0) m_quiet_until = c.now + (DaiTurn)c.cfg.offence_withdraw_turns;
    if (m_goal != 0) w.NoteGoal(m_goal, won ? DaiGoalStatus::Met : DaiGoalStatus::Blocked, why);
    snprintf(m_last_end, sizeof(m_last_end), "%s", why);
    go(DaiOpPhase::None, c, why);
    m_search_on = false;
    m_refusals = 0;
    m_breach_left = -1;
    m_breach_moved = 0;
    m_bad_since = 0;
    m_staging = DaiSlabXY{ -1, -1 };
}

void DaiOffence::begin_withdraw(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why)
{
    if (m_assault != 0) w.DropObjective(m_assault);
    m_assault = 0;
    DaiObjective o;
    o.kind = DaiObjectiveKind::Withdraw;
    o.key = KEY_OP;
    o.area = target_area();
    o.rival = m_target.rival;
    o.staging = m_staging;
    o.until = c.now + (DaiTurn)c.cfg.offence_withdraw_turns;
    m_withdraw = w.PostObjective(o);
    snprintf(m_last_end, sizeof(m_last_end), "%s", why);
    go(DaiOpPhase::Withdraw, c, why);
}

/** An operation against the best target of the goal's rival: straight to stage when it has a way in, else recon. */
void DaiOffence::start(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiGoal &g)
{
    const PlayerNumber rival = (g.p.key > 0) ? (PlayerNumber)(g.p.key - 1) : (PlayerNumber)-1;
    const DaiRivalTarget *pick = nullptr, *unrouted = nullptr;
    for (const DaiRivalTarget &t : c.rivals.Targets()) {
        if (((rival >= 0) && (t.rival != rival)) || !c.know.RivalKeeper(t.rival) || c.know.Alliance(t.rival).ours || (CoolUntil(t) > c.now)) continue;
        if (routed(t) && (t.score > 0)) { pick = &t; break; }
        if ((unrouted == nullptr) && !routed(t)) unrouted = &t;
    }
    if ((pick == nullptr) && (unrouted == nullptr)) {
        w.NoteGoal(g.id, DaiGoalStatus::Blocked, "NO_TARGET");
        return;
    }
    m_goal = g.id;
    m_target = (pick != nullptr) ? *pick : *unrouted;
    m_last_end[0] = '\0';
    go((pick != nullptr) ? DaiOpPhase::Stage : DaiOpPhase::Recon, c, "");
    w.NoteGoal(g.id, DaiGoalStatus::Progress, DaiOpPhaseName(m_phase));
}

void DaiOffence::recon(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if (!refresh_target(c)) { end(c, w, "TARGET_GONE", false, false); return; }
    if (routed(m_target)) {
        if (m_scout != 0) w.DropObjective(m_scout);
        m_scout = 0;
        go(DaiOpPhase::Stage, c, "a way in");
        return;
    }
    if (home_fight(c)) { end(c, w, "HOME_FIGHT", false, false); return; }
    if (c.now >= m_phase_since + (DaiTurn)c.cfg.offence_recon_timeout) { end(c, w, "RECON_TIMEOUT", false, true); return; }
    if (m_scout == 0) {
        DaiObjective o;
        o.kind = DaiObjectiveKind::Scout;
        o.key = KEY_OP;
        o.area = target_area();
        o.rival = m_target.rival;
        m_scout = w.PostObjective(o);
    }
    const DaiSlabXY at{ m_target.x, m_target.y };
    if (c.know.Power(DaiPower::Sight).available && !c.know.Sight().active) ask(c, w, DaiDomain::Magic, DaiService::SightPeek, at, c.cfg.offence_sight_level);
}

void DaiOffence::stage(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if (!refresh_target(c)) { end(c, w, "TARGET_GONE", false, false); return; }
    if (home_fight(c)) { end(c, w, "HOME_FIGHT", false, false); return; }
    if (c.now >= m_phase_since + (DaiTurn)c.cfg.offence_stage_timeout) { end(c, w, "NOT_READY", false, true); return; }
    const DaiReadiness r = DaiRivals::Readiness(c.know, c.personality, c.cfg, c.forecast, m_target);
    // The finish after an Armageddon (P9): the director weighed the rival seen whole; the runway and the levels do not
    // matter at the end.
    bool finish = false;
    c.goals.ForEach([&](const DaiGoal &g) { finish = finish || ((g.id == m_goal) && (g.p.count == DAI_ATTACK_FINISH)); });
    if (r.Ready() || (finish && r.force_ok)) go(DaiOpPhase::Breach, c, r.Ready() ? "ready" : "finish");
}

void DaiOffence::breach(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if (home_fight(c)) { end(c, w, "HOME_FIGHT", false, false); return; }
    // Timed out only when the way has not got shorter for breach_timeout turns (level 52: a way through a gold seam,
    // dug a slab every few hundred turns, given up half dug).
    const DaiTurn moved = std::max(m_phase_since, m_breach_moved);
    if (c.now >= moved + (DaiTurn)c.cfg.offence_breach_timeout) { end(c, w, "BREACH_TIMEOUT", false, true); return; }
    // Refused requests: each counts; too many and the way is not to be had.
    std::vector<uint32_t> keep;
    for (uint32_t id : m_requests) {
        bool found = false, refused = false;
        c.requests.ForEach([&](const DaiRequest &q) { if (q.id == id) { found = true; refused = q.refused; } });
        if (found && refused) { m_refusals++; w.WithdrawRequest(id); continue; }
        if (found) keep.push_back(id);
    }
    m_requests = keep;
    if (m_refusals >= c.cfg.offence_max_refusals) { end(c, w, "REFUSED", false, true); return; }
    const DaiKnowledge &k = c.know;
    if (!m_search_on) {
        const DaiSlabXY goal{ m_target.x, m_target.y };
        m_search.StartFromOwnLand(k, c.player, std::span<const DaiSlabXY>(&goal, 1), DaiDigCosts::Attack(c.cfg, c.personality, k));
        m_search_on = true;
    }
    DaiBudget b;
    b.search_nodes = c.cfg.offence_search_nodes;
    const DaiResult res = m_search.Step(k, b);
    if (res == DaiResult::Wait) return;
    m_search_on = false;                    // planned again at the next look, from the knowledge as it is then
    if (res != DaiResult::Done) { end(c, w, "NO_WAY", false, true); return; }
    const std::span<const DaiSlabXY> path = m_search.Path();
    // `e`: the first slab of another keeper's on the way (its ground, rooms or wall); the staging point is the one before.
    size_t e = path.size();
    for (size_t i = 1; i < path.size(); i++) {
        const PlayerNumber o = k.SlabAt(path[i].x, path[i].y).owner;
        if ((o >= 0) && (o != c.player)) { e = i; break; }
    }
    int64_t left = 0;
    for (size_t i = 1; i < std::min(e + 1, path.size()); i++) if (!open_ground(k.SlabAt(path[i].x, path[i].y))) left++;
    if ((m_breach_left >= 0) && (left < m_breach_left)) m_breach_moved = c.now;
    if ((m_breach_left < 0) || (left < m_breach_left)) m_breach_left = left;
    // The first slab to dig never seen, the slab before it reached: a scout walks there, and a creature entering a slab
    // shows what its sight reaches (level 52: the way ran through water the keeper had never seen, far from any of its
    // land to drop an imp on -- the dig of "unknown ground" that was water stood marked until the breach timed out).
    bool scouting = false;
    for (size_t i = 1; i < std::min(e + 1, path.size()); i++) {
        const DaiSlabInfo s = k.SlabAt(path[i].x, path[i].y);
        if (open_ground(s)) continue;
        if (!s.revealed && (i < e) && k.Reach().Reached(path[i - 1].x, path[i - 1].y)) {
            scouting = true;
            const DaiObjective *was = (m_scout != 0) ? c.objectives.Find(m_scout) : nullptr;
            const DaiRect at = around(path[i - 1], 0);
            if ((was == nullptr) || (was->area.x0 != at.x0) || (was->area.y0 != at.y0)) {
                if (m_scout != 0) w.DropObjective(m_scout);
                DaiObjective o;
                o.kind = DaiObjectiveKind::Scout;
                o.key = KEY_OP;
                o.area = at;
                o.rival = m_target.rival;
                m_scout = w.PostObjective(o);
            }
        }
        if (!scouting && (m_scout != 0)) { w.DropObjective(m_scout); m_scout = 0; }
        if (diggable(s, c.player) && (i < e)) {
            size_t run = i;
            while ((run + 1 < e) && diggable(k.SlabAt(path[run + 1].x, path[run + 1].y), c.player)) run++;
            ask(c, w, DaiDomain::Excavation, DaiService::Dig, path[run], 0);
            return;
        }
        // Lava or abyss the attack's route bridges (the bridge available and crossing it): a dig request to its far edge
        // -- Excavation's dig_route bridges it on the way (plan 13 section 31: on level 124 every breach that ended
        // BLOCKED stood at lava, 142 of 142).
        if (((s.terrain == DaiTerrain::Lava) || (s.terrain == DaiTerrain::Abyss)) && (i < e)) {
            auto liquid = [&](size_t at) {
                const DaiTerrain t = k.SlabAt(path[at].x, path[at].y).terrain;
                return (t == DaiTerrain::Lava) || (t == DaiTerrain::Abyss);
            };
            size_t run = i;
            while ((run + 1 < e) && liquid(run + 1)) run++;
            ask(c, w, DaiDomain::Excavation, DaiService::Dig, path[run], 0);
            return;
        }
        if ((s.terrain == DaiTerrain::Wall) && (s.owner != c.player)) {
            ask(c, w, DaiDomain::Magic, DaiService::DestroyWalls, path[i], 1);   // level 1 digs the slab out
            return;
        }
        // What stops it (plan 13 section 31: on level 124 most breaches ended here, the slab not logged).
        static const char *const terrain_names[] = { "unknown", "rock", "earth", "gold", "dense gold", "gems", "floor", "room",
                                                     "water", "lava", "abyss", "wall", "door", "impassable" };
        const size_t tn = (size_t)s.terrain;
        DaiLog::Info(c.player, "offence: breach blocked at (%d,%d), %s of p%d, slab %d of %d before p%d's ground", (int)path[i].x,
            (int)path[i].y, (tn < sizeof(terrain_names) / sizeof(terrain_names[0])) ? terrain_names[tn] : "?", (int)s.owner, (int)i,
            (int)e, (int)m_target.rival);
        end(c, w, "BLOCKED", false, true);
        return;
    }
    // The way is open: the staging point is the slab before the rival's ground; drops need it the keeper's own.
    m_staging = (e < path.size()) ? path[e - 1] : path[path.size() >= 2 ? path.size() - 2 : 0];
    if (k.SlabAt(m_staging.x, m_staging.y).owner != c.player) return;      // the imps claim the dug end first
    for (uint32_t id : m_requests) w.WithdrawRequest(id);
    m_requests.clear();
    if (m_scout != 0) w.DropObjective(m_scout);
    m_scout = 0;
    go(DaiOpPhase::Assault, c, "the way is open");
}

void DaiOffence::assault(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    // Won: a heart target's keeper is out of the game; a room's or portal's slab is not the rival's any more.
    const bool won = (m_target.kind == DaiTargetKind::Heart) ? !k.RivalKeeper(m_target.rival)
                                                             : (k.SlabAt(m_target.x, m_target.y).revealed && (k.SlabAt(m_target.x, m_target.y).owner != m_target.rival));
    if (won) {
        if (m_assault != 0) w.DropObjective(m_assault);
        m_assault = 0;
        for (uint32_t id : m_requests) w.WithdrawRequest(id);
        m_requests.clear();
        if (m_target.kind == DaiTargetKind::Heart) { go(DaiOpPhase::Exploit, c, "won"); return; }
        end(c, w, "WON", true, false);
        return;
    }
    if (m_assault == 0) {
        DaiObjective o;
        o.kind = DaiObjectiveKind::Assault;
        o.key = KEY_OP;
        o.area = target_area();
        o.rival = m_target.rival;
        o.staging = m_staging;
        o.stakes = std::max<DaiFix>(DAI_FIX_ONE, FixFromInt(m_target.prize) / 250);
        o.gold_per_kturn = c.personality.offence_cta_attack_budget;
        o.home_guard_share = c.personality.offence_home_guard_share;
        o.until = c.now + (DaiTurn)c.cfg.offence_assault_timeout;
        m_assault = w.PostObjective(o);
        m_bad_since = 0;
        return;
    }
    if (home_fight(c)) { begin_withdraw(c, w, "HOME_FIGHT"); return; }
    const DaiObjective *o = c.objectives.Find(m_assault);
    if (o == nullptr) { begin_withdraw(c, w, "TIMED_OUT"); return; }
    const DaiObjectiveReport &r = o->report;
    if (r.state == DaiObjectiveState::Lost) { begin_withdraw(c, w, "LOST"); return; }
    const bool bad = (r.own_force > 0 || r.enemy_force > 0)
        && (r.enemy_force * 100 > c.personality.offence_withdraw_ratio * std::max<int64_t>(r.own_force, 1));
    if (!bad) { m_bad_since = 0; return; }
    if (m_bad_since == 0) m_bad_since = c.now;
    if (c.now >= m_bad_since + (DaiTurn)c.personality.offence_withdraw_patience) begin_withdraw(c, w, "OUTMATCHED");
}

/** After a heart falls: the loser's rooms by imp creep, the most valuable first, one at a time (07 section 8). */
void DaiOffence::exploit(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const PlayerNumber loser = m_target.rival;
    // The room under way: on until it is the keeper's (the creep job's) or its request is refused.
    if (m_exploit_at.x >= 0) {
        bool refused = false;
        c.requests.ForEach([&](const DaiRequest &q) {
            refused = refused || ((q.service == DaiService::Creep) && (q.at.x == m_exploit_at.x) && (q.at.y == m_exploit_at.y) && q.refused);
        });
        if ((k.SlabAt(m_exploit_at.x, m_exploit_at.y).owner != loser) || refused) {
            for (uint32_t id : m_requests) w.WithdrawRequest(id);
            m_requests.clear();
            m_exploit_at = DaiSlabXY{ -1, -1 };
        } else {
            return;
        }
    }
    const RoomKind portal = k.RoomKindOf(DaiRoom::Entrance), treasury = k.RoomKindOf(DaiRoom::Treasury), library = k.RoomKindOf(DaiRoom::Library);
    auto rank = [&](RoomKind kind) { return (kind == portal) ? 0 : (kind == treasury) ? 1 : (kind == library) ? 2 : 3; };
    const DaiRivalRoom *pick = nullptr;
    DaiSlabXY at{ -1, -1 };
    int64_t left = 0;
    for (const DaiRivalRoom &rm : k.RivalRooms()) {
        if (rm.look.owner != loser) continue;
        left++;
        if ((pick != nullptr) && (rank(rm.look.kind) >= rank(pick->look.kind))) continue;
        // A slab of it the keeper's land touches.
        bool found = false;
        for (DaiSlabCoord y = rm.look.box.y0; (y <= rm.look.box.y1) && !found; y++) {
            for (DaiSlabCoord x = rm.look.box.x0; (x <= rm.look.box.x1) && !found; x++) {
                const DaiSlabInfo s = k.SlabAt(x, y);
                if ((s.terrain != DaiTerrain::Room) || (s.owner != loser)) continue;
                const int *const dx = DAI_N4_DX, *const dy = DAI_N4_DY;
                for (int d = 0; (d < 4) && !found; d++) {
                    const DaiSlabInfo n = k.SlabAt((DaiSlabCoord)(x + dx[d]), (DaiSlabCoord)(y + dy[d]));
                    if ((n.owner == c.player) && ((n.terrain == DaiTerrain::Floor) || (n.terrain == DaiTerrain::Room))) { found = true; at = DaiSlabXY{ x, y }; }
                }
            }
        }
        if (found) pick = &rm;
    }
    if ((left == 0) || (c.now >= m_phase_since + (DaiTurn)c.cfg.offence_exploit_turns)) {
        end(c, w, "WON", true, false);
        return;
    }
    if (pick == nullptr) return;            // the keeper's imps claim the loser's path meanwhile
    m_exploit_at = at;
    ask(c, w, DaiDomain::Excavation, DaiService::Creep, at, c.cfg.offence_creep_imps);
    DaiLog::Info(c.player, "offence: exploit, creep onto p%d's room (kind %d) at (%d,%d); %" PRId64 " of its rooms left",
        (int)loser, (int)pick->look.kind, (int)at.x, (int)at.y, left);
}

void DaiOffence::withdraw(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if (c.now >= m_phase_since + (DaiTurn)c.cfg.offence_withdraw_turns) {
        char why[32];
        snprintf(why, sizeof(why), "%s", m_last_end);
        end(c, w, (why[0] != '\0') ? why : "WITHDRAWN", false, true);
    }
}

void DaiOffence::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    // A breach's search steps every turn while it runs: it starts over when ground it has touched changes, and the
    // keeper's imps change ground all the time (at one step a look it never ends).
    const bool searching = (m_phase == DaiOpPhase::Breach) && m_search_on;
    const bool look = (c.now >= m_next);
    if (!look && !searching) return;
    if (look) m_next = c.now + (DaiTurn)c.cfg.offence_op_interval;
    m_cooled.erase(std::remove_if(m_cooled.begin(), m_cooled.end(), [&](const Cooled &cd) { return cd.until <= c.now; }), m_cooled.end());
    // The director's Attack goal, whatever its key (the rival it names); the one this operation serves first.
    const DaiGoal *g = nullptr;
    c.goals.ForEach([&](const DaiGoal &x) {
        if ((x.kind == DaiGoalKind::Attack) && ((g == nullptr) || (x.id == m_goal))) g = &x;
    });
    if (look) {
        diplomacy(c, w);
        raids(c, w, g);
    }
    if (m_phase == DaiOpPhase::None) {
        if ((g == nullptr) || home_fight(c) || (c.now < m_quiet_until)) return;    // none begun while the keeper fights at home
        if (g->p.count == DAI_ATTACK_RAIDS) return;        // the raid posture: raids only
        start(c, w, *g);
        if (m_phase == DaiOpPhase::None) return;
    } else if ((g == nullptr) || (g->id != m_goal)) {
        // The director wants it no longer: an assault withdraws, an exploit ends won, anything before them ends.
        if (m_phase == DaiOpPhase::Exploit) { m_goal = 0; end(c, w, "WON", true, false); return; }
        if (m_phase == DaiOpPhase::Assault) begin_withdraw(c, w, "NO_GOAL");
        else if (m_phase != DaiOpPhase::Withdraw) { m_goal = 0; end(c, w, "NO_GOAL", false, false); return; }
    }
    const DaiOpPhase before = m_phase;
    switch (m_phase) {
    case DaiOpPhase::Recon: recon(c, w); break;
    case DaiOpPhase::Stage: stage(c, w); break;
    case DaiOpPhase::Breach: breach(c, w); break;
    case DaiOpPhase::Assault: assault(c, w); break;
    case DaiOpPhase::Exploit: exploit(c, w); break;
    case DaiOpPhase::Withdraw: withdraw(c, w); break;
    default: break;
    }
    if ((m_phase != before) && (m_phase != DaiOpPhase::None) && (m_goal != 0)) w.NoteGoal(m_goal, DaiGoalStatus::Progress, DaiOpPhaseName(m_phase));
}

void DaiOffence::raids(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiGoal *g)
{
    if (m_raid_sub != DaiObjectiveSub::None) { follow_raid(c, w, g); return; }
    if ((g == nullptr) || (c.personality.offence_raid_appetite <= 0) || (c.now < m_raid_next) || home_fight(c)) return;
    if ((m_phase == DaiOpPhase::Assault) || (m_phase == DaiOpPhase::Withdraw)) return;     // its fighters are out
    const PlayerNumber rival = (g->p.key > 0) ? (PlayerNumber)(g->p.key - 1) : (PlayerNumber)-1;
    if (!start_raid(c, w, rival)) m_raid_next = c.now + (DaiTurn)std::max<int64_t>(c.cfg.offence_raid_interval / 10, 1);
}

/** The first raid with a target: a portal beside the keeper's land, an imp near it, a room near it. With the force
 *  estimate favourable (plan 13 L2), the imp hunt and the harass look out to raid_reach when nothing lies within
 *  raid_border: the rival behind a barrier is otherwise left to mine in peace. */
bool DaiOffence::start_raid(const DaiTurnCtx &c, DaiManagerWrites &w, PlayerNumber rival)
{
    if (try_raid(c, w, rival, c.cfg.offence_raid_border, true)) return true;
    if ((c.cfg.offence_raid_reach > c.cfg.offence_raid_border)
        && c.rivals.Favourable(c.know, c.personality, c.cfg, c.forecast, rival, nullptr)
        && try_raid(c, w, rival, c.cfg.offence_raid_reach, false)) return true;
    m_raid_at = DaiSlabXY{ -1, -1 };
    return false;
}

/** A raid on a target within `border` slabs of the keeper's land; the portal steal only at the border (`steal`). */
bool DaiOffence::try_raid(const DaiTurnCtx &c, DaiManagerWrites &w, PlayerNumber rival, int64_t border, bool steal)
{
    const DaiKnowledge &k = c.know;
    auto wanted = [&](PlayerNumber r) { return ((rival < 0) || (r == rival)) && k.RivalKeeper(r) && !k.Alliance(r).ours; };
    DaiObjective o;
    o.kind = DaiObjectiveKind::Raid;
    o.key = KEY_RAID;
    o.stakes = 2 * DAI_FIX_ONE;
    o.home_guard_share = c.personality.offence_home_guard_share;
    o.until = c.now + (DaiTurn)c.cfg.offence_raid_timeout;
    // PortalSteal: a slab of the portal with the keeper's land beside it.
    for (const DaiRivalTarget &t : c.rivals.Targets()) {
        if (!steal) break;
        if ((t.kind != DaiTargetKind::Portal) || !wanted(t.rival) || (CoolUntil(t) > c.now)) continue;
        for (DaiSlabCoord y = (DaiSlabCoord)(t.y - 2); y <= t.y + 2; y++) {
            for (DaiSlabCoord x = (DaiSlabCoord)(t.x - 2); x <= t.x + 2; x++) {
                const DaiSlabInfo s = k.SlabAt(x, y);
                if ((s.terrain != DaiTerrain::Room) || (s.owner != t.rival) || (s.room != t.room)) continue;
                const int *const dx = DAI_N4_DX, *const dy = DAI_N4_DY;
                for (int d = 0; d < 4; d++) {
                    const DaiSlabXY fh{ (DaiSlabCoord)(x + dx[d]), (DaiSlabCoord)(y + dy[d]) };
                    if (!own_land(k.SlabAt(fh.x, fh.y), c.player)) continue;
                    o.sub = DaiObjectiveSub::PortalSteal;
                    o.area = around(DaiSlabXY{ x, y }, 1);
                    o.rival = t.rival;
                    o.staging = fh;
                    m_raid = w.PostObjective(o);
                    DaiRequest q;
                    q.kind = DaiRequestKind::Service;
                    q.to = DaiDomain::Excavation;
                    q.service = DaiService::Creep;
                    q.key = (int64_t)y * k.Width() + x;
                    q.at = DaiSlabXY{ x, y };
                    q.amount = c.cfg.offence_creep_imps;
                    m_raid_request = w.PostRequest(q);
                    m_raid_at = DaiSlabXY{ x, y };
                    m_raid_rival = t.rival;
                    break;
                }
                if (m_raid != 0) break;
            }
            if (m_raid != 0) break;
        }
        if (m_raid != 0) break;
    }
    // ImpHunt: the rival's imp nearest the keeper's land, within raid_border of it.
    if (m_raid == 0) {
        const DaiThingLook *imp = nullptr;
        int64_t best = border + 1;
        for (const DaiThingLook &e : k.Enemies()) {
            if (!e.digger || !wanted(e.owner)) continue;
            DaiSlabXY near;
            const DaiSlabCoord ex = DaiSubtileSlab(e.x), ey = DaiSubtileSlab(e.y);
            if (!own_near(k, c.player, ex, ey, border, &near)) continue;
            const int64_t d = DaiChess(near.x, near.y, ex, ey);
            if (d < best) { best = d; imp = &e; }
        }
        if (imp != nullptr) {
            m_raid_at = DaiSlabXY{ DaiSubtileSlab(imp->x), DaiSubtileSlab(imp->y) };
            o.sub = DaiObjectiveSub::ImpHunt;
            o.area = around(m_raid_at, 2);
            o.rival = imp->owner;
            o.target = imp->ref;
            m_raid = w.PostObjective(o);
            m_raid_imp = imp->ref;
            m_raid_rival = imp->owner;
        }
    }
    // Harass: a room of the rival's within raid_border of the keeper's land, the banner gathered on the nearest own floor;
    // while one of its creatures is being lured, a scavenger room first (its scavengers fight instead, 08 section 7a.3).
    bool lured = false;
    for (const DaiCreatureLook &cr : k.Creatures()) lured = lured || cr.scavenged;
    const RoomKind scav = k.RoomKindOf(DaiRoom::Scavenger);
    for (int pass = (lured && (scav != 0)) ? 0 : 1; (pass < 2) && (m_raid == 0); pass++) {
        for (const DaiRivalTarget &t : c.rivals.Targets()) {
            if ((t.kind != DaiTargetKind::Room) || !wanted(t.rival) || (CoolUntil(t) > c.now)) continue;
            if ((pass == 0) && (t.room != scav)) continue;
            DaiSlabXY stg;
            if (!own_near(k, c.player, t.x, t.y, border, &stg)) continue;
            m_raid_at = DaiSlabXY{ t.x, t.y };
            o.sub = DaiObjectiveSub::Harass;
            o.area = around(m_raid_at, 2);
            o.rival = t.rival;
            o.staging = stg;
            o.gold_per_kturn = c.personality.offence_cta_attack_budget / 2;
            m_raid = w.PostObjective(o);
            m_raid_rival = t.rival;
            break;
        }
    }
    if (m_raid == 0) return false;
    m_raid_sub = o.sub;
    m_raid_since = c.now;
    m_raid_end[0] = '\0';
    DaiLog::Info(c.player, "raid: %s on p%d at (%d,%d)%s", DaiObjectiveSubName(o.sub), (int)m_raid_rival, (int)m_raid_at.x, (int)m_raid_at.y,
        steal ? "" : " (across, favourable)");
    return true;
}

void DaiOffence::follow_raid(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiGoal *g)
{
    const DaiKnowledge &k = c.know;
    if (m_raid_withdraw != 0) {
        if (c.now >= m_raid_since + (DaiTurn)c.cfg.offence_withdraw_turns) {
            char why[32];
            snprintf(why, sizeof(why), "%s", (m_raid_end[0] != '\0') ? m_raid_end : "WITHDRAWN");
            end_raid(c, w, why);
        }
        return;
    }
    auto withdraw = [&](const char *why) {
        if (m_raid != 0) w.DropObjective(m_raid);
        m_raid = 0;
        DaiObjective o;
        o.kind = DaiObjectiveKind::Withdraw;
        o.key = KEY_RAID;
        o.area = around(m_raid_at, 2);
        o.rival = m_raid_rival;
        o.until = c.now + (DaiTurn)c.cfg.offence_withdraw_turns;
        m_raid_withdraw = w.PostObjective(o);
        m_raid_since = c.now;
        snprintf(m_raid_end, sizeof(m_raid_end), "%s", why);
        DaiLog::Info(c.player, "raid: %s withdrawn (%s)", DaiObjectiveSubName(m_raid_sub), why);
    };
    const bool harass = (m_raid_sub == DaiObjectiveSub::Harass);
    if (g == nullptr) { if (harass) withdraw("NO_GOAL"); else end_raid(c, w, "NO_GOAL"); return; }
    if (home_fight(c)) { if (harass) withdraw("HOME_FIGHT"); else end_raid(c, w, "HOME_FIGHT"); return; }
    if ((c.objectives.Find(m_raid) == nullptr) || (c.now >= m_raid_since + (DaiTurn)c.cfg.offence_raid_timeout)) {
        if (harass) withdraw("TIMED_OUT"); else end_raid(c, w, "TIMED_OUT");
        return;
    }
    switch (m_raid_sub) {
    case DaiObjectiveSub::PortalSteal: {
        if (k.SlabAt(m_raid_at.x, m_raid_at.y).owner == c.player) { end_raid(c, w, "WON"); return; }
        bool refused = false;
        c.requests.ForEach([&](const DaiRequest &q) { refused = refused || ((q.id == m_raid_request) && q.refused); });
        if (refused) end_raid(c, w, "REFUSED");
        break;
    }
    case DaiObjectiveSub::ImpHunt: {
        const DaiThingLook *imp = nullptr;
        for (const DaiThingLook &e : k.Enemies()) if (e.ref == m_raid_imp) imp = &e;
        if (imp == nullptr) { end_raid(c, w, "DONE"); return; }
        // The objective follows the imp.
        const DaiSlabXY at{ DaiSubtileSlab(imp->x), DaiSubtileSlab(imp->y) };
        if ((at.x != m_raid_at.x) || (at.y != m_raid_at.y)) {
            const DaiObjective *o = c.objectives.Find(m_raid);
            DaiObjective moved = *o;
            moved.area = around(at, 2);
            w.PostObjective(moved);
            m_raid_at = at;
        }
        break;
    }
    case DaiObjectiveSub::Harass:
        if (c.now >= m_raid_since + (DaiTurn)c.cfg.offence_harass_turns) withdraw("HARASSED");
        break;
    default:
        break;
    }
}

void DaiOffence::end_raid(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why)
{
    for (uint32_t *o : { &m_raid, &m_raid_withdraw }) {
        if (*o != 0) w.DropObjective(*o);
        *o = 0;
    }
    if (m_raid_request != 0) w.WithdrawRequest(m_raid_request);
    m_raid_request = 0;
    DaiLog::Info(c.player, "raid: %s on p%d ended: %s", DaiObjectiveSubName(m_raid_sub), (int)m_raid_rival, why);
    snprintf(m_raid_end, sizeof(m_raid_end), "%s", why);
    m_raid_sub = DaiObjectiveSub::None;
    m_raid_at = DaiSlabXY{ -1, -1 };
    m_raid_rival = -1;
    m_raid_imp = DaiThingRef{};
    m_raids++;
    const int64_t appetite = c.personality.offence_raid_appetite;
    m_raid_next = c.now + ((appetite > 0) ? (DaiTurn)(c.cfg.offence_raid_interval * DAI_FIX_ONE / appetite) : (DaiTurn)INT32_MAX);
}

DaiFix DaiOffence::ally_value(const DaiTurnCtx &c, PlayerNumber r) const
{
    const DaiKnowledge &k = c.know;
    const int64_t me = std::max<int64_t>(DaiRivals::OwnLevels(k), 1);
    const int64_t them = k.KnownForce(r, c.now);
    const int64_t stronger = std::max(me, them);
    DaiFix common = 0;
    for (PlayerNumber t = 0; t < DAI_MAX_PLAYERS; t++) {
        const DaiAllianceLook &a = k.Alliance(t);
        if ((t == r) || (t == c.player) || !a.keeper || (a.ours && a.theirs)) continue;
        const int64_t f = k.KnownForce(t, c.now);
        if (f > stronger) common = std::max<DaiFix>(common, DAI_FIX_ONE + (DaiFix)std::min<int64_t>(DAI_FIX_ONE, (f - stronger) * DAI_FIX_ONE / stronger));
    }
    const DaiFix relief = (them > me) ? (DaiFix)std::min<int64_t>(DAI_FIX_ONE / 2, (them - me) * DAI_FIX_ONE / (2 * them)) : 0;
    const DaiFix threat = (DaiFix)std::min<int64_t>(2 * DAI_FIX_ONE, c.rivals.Hate(r, c.now, c.cfg) * DAI_FIX_ONE / HATE_ALLY);
    const DaiRivalTarget *best = c.rivals.Best();
    const DaiFix lost = ((best != nullptr) && (best->rival == r)) ? DAI_FIX_ONE / 2 : 0;
    return common + relief - threat - lost;
}

void DaiOffence::declare(const DaiTurnCtx &c, DaiManagerWrites &w, PlayerNumber r, bool on, const char *why)
{
    DaiJob *j = w.AddJob(DAI_JOB_SET_ALLIANCE, (uint64_t)r, FixFromInt(2));
    if (j == nullptr) return;
    DaiDiplomacyParams p;
    p.other = r;
    p.enabled = on;
    j->p = p;
    if (DaiJob *m = w.AddJob(DAI_JOB_SEND_MESSAGE, (uint64_t)r, FixFromInt(1))) {
        DaiDiplomacyParams q;
        q.other = r;
        snprintf(q.message, sizeof(q.message), on ? "Alliance offered to player %d" : "Alliance with player %d ended", (int)r);
        m->p = q;
    }
    if (on) m_ties[(size_t)r].allied = c.now;
    DaiLog::Info(c.player, "diplomacy: alliance with p%d %s (%s; value %" PRId64 "/1024)", (int)r, on ? "declared" : "withdrawn", why,
        (int64_t)m_ties[(size_t)r].value);
}

void DaiOffence::diplomacy(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if (c.now < m_dip_next) return;
    m_dip_next = c.now + (DaiTurn)c.cfg.diplomacy_interval;
    const DaiKnowledge &k = c.know;
    const DaiPersonality &per = c.personality;
    const DaiFix min = per.offence_ally_min_value;
    const DaiFix accept = (DaiFix)((int64_t)min * (DAI_FIX_ONE - per.offence_loyalty / 2) / DAI_FIX_ONE);
    const DaiFix keep = (DaiFix)((int64_t)min * (DAI_FIX_ONE - per.offence_loyalty) / DAI_FIX_ONE);
    const int64_t me = DaiRivals::OwnLevels(k);
    bool acted = false;
    for (PlayerNumber r = 0; r < DAI_MAX_PLAYERS; r++) {
        Tie &t = m_ties[(size_t)r];
        const DaiAllianceLook &a = k.Alliance(r);
        if ((r == c.player) || !a.keeper) { t = Tie(); continue; }
        t.value = ally_value(c, r);
        const int64_t hate = c.rivals.Hate(r, c.now, c.cfg);
        if (hate > t.hate) t.war = c.now;               // it took the keeper's ground, or came onto it
        t.hate = hate;
        if (((m_phase != DaiOpPhase::None) && (m_target.rival == r)) || (m_raid_rival == r)) t.war = c.now;     // the keeper went against it
        t.above_since = (t.value >= min) ? ((t.above_since != 0) ? t.above_since : c.now) : 0;
        const bool was_mutual = t.mutual;
        t.mutual = a.ours && a.theirs;
        const bool peace = (t.war == 0) || (c.now >= t.war + (DaiTurn)c.cfg.diplomacy_peace_after_war);
        if (a.locked || acted || (w.FindJob(DAI_JOB_SET_ALLIANCE, (uint64_t)r) != nullptr)) continue;
        if (!a.ours) {
            if (a.theirs && peace && (t.value >= accept)) { declare(c, w, r, true, "ACCEPTED"); acted = true; }
            else if (peace && (t.above_since != 0) && (c.now >= t.above_since + (DaiTurn)c.cfg.diplomacy_ally_dwell)) { declare(c, w, r, true, "PROPOSED"); acted = true; }
            continue;
        }
        const char *why = nullptr;
        if (was_mutual && !a.theirs) why = "BETRAYED";
        else if (t.war > t.allied) why = "ATTACKED";
        else if (t.value < keep) why = "NOT_WORTH";
        else if (t.mutual && (per.offence_treachery > 0) && (k.KnownForce(r, c.now) * 2 < me) && (c.rng.Below(DAI_FIX_ONE) < per.offence_treachery)) why = "TREACHERY";
        if (why == nullptr) continue;
        declare(c, w, r, false, why);
        t.war = c.now;
        t.above_since = 0;
        acted = true;
    }
}

void DaiOffence::Save(DaiBlobWriter &w) const
{
    w.I64((int64_t)m_phase);
    w.U64(m_phase_since);
    w.U64(m_next);
    w.U32(m_goal);
    save_target(w, m_target);
    w.I64(m_staging.x); w.I64(m_staging.y);
    w.U32(m_scout); w.U32(m_assault); w.U32(m_withdraw);
    w.I64((int64_t)m_requests.size());
    for (uint32_t id : m_requests) w.U32(id);
    w.I64(m_refusals);
    w.U64(m_bad_since);
    w.I64((int64_t)m_cooled.size());
    for (const Cooled &cd : m_cooled) { w.I64(cd.rival); w.I64((int64_t)cd.kind); w.I64(cd.room_index); w.U64(cd.until); }
    w.I64(m_won);
    w.I64(m_given_up);
    w.Str(m_last_end);
    w.Bool(m_search_on);
    if (m_search_on) m_search.Save(w);
    w.I64((int64_t)m_raid_sub);
    w.U32(m_raid); w.U32(m_raid_withdraw); w.U32(m_raid_request);
    w.U64(m_raid_since); w.U64(m_raid_next);
    w.I64(m_raid_at.x); w.I64(m_raid_at.y);
    w.I64(m_raid_rival);
    w.Ref(m_raid_imp);
    w.I64(m_raids);
    w.Str(m_raid_end);
    w.U64(m_quiet_until);
    w.I64(m_exploit_at.x); w.I64(m_exploit_at.y);
    for (const Tie &t : m_ties) { w.I64(t.value); w.U64(t.above_since); w.U64(t.war); w.U64(t.allied); w.I64(t.hate); w.Bool(t.mutual); }
    w.U64(m_dip_next);
    w.I64(m_breach_left);
    w.U64(m_breach_moved);
}

bool DaiOffence::Load(DaiBlobReader &r)
{
    Reset();
    m_phase = (DaiOpPhase)r.Range(0, (int64_t)DaiOpPhase::Count - 1);
    m_phase_since = (DaiTurn)r.U64();
    m_next = (DaiTurn)r.U64();
    m_goal = r.U32();
    m_target = load_target(r);
    m_staging.x = (DaiSlabCoord)r.Range(-1, 4096); m_staging.y = (DaiSlabCoord)r.Range(-1, 4096);
    m_scout = r.U32(); m_assault = r.U32(); m_withdraw = r.U32();
    const int64_t n = r.Count(256);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) m_requests.push_back(r.U32());
    m_refusals = r.Range(0, INT32_MAX);
    m_bad_since = (DaiTurn)r.U64();
    const int64_t nc = r.Count(256);
    for (int64_t i = 0; (i < nc) && r.Ok(); i++) {
        Cooled cd;
        cd.rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
        cd.kind = (DaiTargetKind)r.Range(0, (int64_t)DaiTargetKind::Count - 1);
        cd.room_index = r.Range(0, INT32_MAX);
        cd.until = (DaiTurn)r.U64();
        m_cooled.push_back(cd);
    }
    m_won = r.Range(0, INT64_MAX);
    m_given_up = r.Range(0, INT64_MAX);
    snprintf(m_last_end, sizeof(m_last_end), "%s", r.Str().c_str());
    m_search_on = r.Bool();
    if (m_search_on && r.Ok() && !m_search.Load(r)) r.Fail("offence search");
    m_raid_sub = (DaiObjectiveSub)r.Range(0, (int64_t)DaiObjectiveSub::Count - 1);
    m_raid = r.U32(); m_raid_withdraw = r.U32(); m_raid_request = r.U32();
    m_raid_since = (DaiTurn)r.U64(); m_raid_next = (DaiTurn)r.U64();
    m_raid_at.x = (DaiSlabCoord)r.Range(-1, 4096); m_raid_at.y = (DaiSlabCoord)r.Range(-1, 4096);
    m_raid_rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
    m_raid_imp = r.Ref();
    m_raids = r.Range(0, INT64_MAX);
    snprintf(m_raid_end, sizeof(m_raid_end), "%s", r.Str().c_str());
    m_quiet_until = (DaiTurn)r.U64();
    m_exploit_at.x = (DaiSlabCoord)r.Range(-1, 4096); m_exploit_at.y = (DaiSlabCoord)r.Range(-1, 4096);
    for (Tie &t : m_ties) {
        t.value = r.Range(-16 * DAI_FIX_ONE, 16 * DAI_FIX_ONE);
        t.above_since = (DaiTurn)r.U64(); t.war = (DaiTurn)r.U64(); t.allied = (DaiTurn)r.U64();
        t.hate = r.Range(0, INT64_MAX);
        t.mutual = r.Bool();
    }
    m_dip_next = (DaiTurn)r.U64();
    m_breach_left = r.Range(-1, INT32_MAX);
    m_breach_moved = (DaiTurn)r.U64();
    if (!r.Ok()) Reset();
    return r.Ok();
}
