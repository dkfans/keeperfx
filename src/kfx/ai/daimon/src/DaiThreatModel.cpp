#include "pre_inc.h"
#include "daimon/DaiThreatModel.h"

#include <algorithm>

#include "daimon/DaiLimits.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** Sources a route search starts from at most (the ones nearest the heart), queries of each kind at most. */
constexpr size_t MAX_SOURCES = 64;
constexpr size_t MAX_GATE_QUERIES = 8;
constexpr size_t MAX_TRAFFIC_QUERIES = 16;
constexpr int64_t MAX_REACH = 6;
/** Hazards kept at most, and the losses each keeps. */
constexpr size_t MAX_HAZARDS = 32;
constexpr size_t DAI_HAZARD_LOSSES = 16;
constexpr uint64_t TARGET_ROLES = DAI_ROLE_HEART | DAI_ROLE_GOLD | DAI_ROLE_SPELLBOOKS;
constexpr uint64_t TRAFFIC_ROLES = DAI_ROLE_LAIR | DAI_ROLE_FOOD_SPAWN | DAI_ROLE_GOLD | DAI_ROLE_RESEARCH | DAI_ROLE_MANUFACTURE
                                 | DAI_ROLE_TRAIN;

/** Ground a creature walks on (or wades): the way an enemy comes, and the sides a corridor does not have. */
bool open_ground(DaiTerrain t)
{
    return (t == DaiTerrain::Floor) || (t == DaiTerrain::Room) || (t == DaiTerrain::Door) || (t == DaiTerrain::Water);
}

/** `v` at `dt` turns later: halved every `hl` turns, and in between linearly down toward the next half. */
int64_t decayed(int64_t v, DaiTurn dt, int64_t hl)
{
    if ((v == 0) || (dt == 0)) return v;
    const DaiTurn halves = dt / (DaiTurn)hl;
    if (halves >= 62) return 0;
    v >>= halves;
    const int64_t rem = (int64_t)(dt % (DaiTurn)hl);
    return v - v * rem / (2 * hl);
}

/** The `max` slabs of `in` nearest (hx, hy), ties by slab order: the same pick on every machine. */
std::vector<DaiSlabXY> nearest(std::vector<DaiSlabXY> in, DaiSlabCoord hx, DaiSlabCoord hy, size_t max)
{
    std::stable_sort(in.begin(), in.end(), [&](const DaiSlabXY &a, const DaiSlabXY &b) {
        return DaiChess(a.x, a.y, hx, hy) < DaiChess(b.x, b.y, hx, hy);
    });
    if (in.size() > max) in.resize(max);
    return in;
}

void save_slabs(DaiBlobWriter &w, const std::vector<DaiSlabXY> &v)
{
    w.I64((int64_t)v.size());
    for (const DaiSlabXY &s : v) { w.I64(s.x); w.I64(s.y); }
}

void load_slabs(DaiBlobReader &r, std::vector<DaiSlabXY> &v, int64_t max)
{
    v.resize((size_t)r.Count(max));
    for (DaiSlabXY &s : v) { s.x = (DaiSlabCoord)r.Range(0, 4096); s.y = (DaiSlabCoord)r.Range(0, 4096); }
}

void save_routes(DaiBlobWriter &w, const std::vector<DaiThreatRoute> &v)
{
    w.I64((int64_t)v.size());
    for (const DaiThreatRoute &rt : v) {
        w.I64((int64_t)rt.source);
        w.I64(rt.rival);
        w.I64(rt.weight);
        w.I64(rt.cost);
        save_slabs(w, rt.path);
    }
}

void load_routes(DaiBlobReader &r, std::vector<DaiThreatRoute> &v)
{
    v.resize((size_t)r.Count(1024));
    for (DaiThreatRoute &rt : v) {
        rt.source = (DaiThreatSource)r.Range(0, (int64_t)DaiThreatSource::Count - 1);
        rt.rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
        rt.weight = r.I64();
        rt.cost = r.I64();
        load_slabs(r, rt.path, 1 << 20);
    }
}

void save_counts(DaiBlobWriter &w, const std::vector<uint8_t> &v)
{
    int64_t n = 0;
    for (uint8_t c : v) n += (c != 0);
    w.I64(n);
    for (size_t i = 0; i < v.size(); i++) if (v[i] != 0) { w.U32((uint32_t)i); w.I64(v[i]); }
}

void load_counts(DaiBlobReader &r, std::vector<uint8_t> &v)
{
    const int64_t n = r.Count((int64_t)v.size());
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        const uint32_t at = r.U32();
        const int64_t c = r.Range(0, 255);
        if (at < v.size()) v[at] = (uint8_t)c;
        else r.Fail("a traffic slab off the map");
    }
}
}

void DaiThreatModel::init(DaiSlabCoord w, DaiSlabCoord h)
{
    m_w = w;
    m_h = h;
    const size_t n = (size_t)w * (size_t)h;
    m_value.assign(n, 0);
    m_stamp.assign(n, 0);
    m_traffic.assign(n, 0);
    m_building_traffic.assign(n, 0);
    m_routes.clear();
    m_chokepoints.clear();
    m_queue.clear();
    m_qi = 0;
    m_search_on = false;
    m_building.clear();
    m_hazards.clear();
    m_imps.clear();
}

int64_t DaiThreatModel::ThreatAt(DaiSlabCoord x, DaiSlabCoord y, DaiTurn now) const
{
    if ((x < 0) || (y < 0) || (x >= m_w) || (y >= m_h)) return 0;
    const size_t at = (size_t)y * m_w + x;
    return decayed(m_value[at], (now > m_stamp[at]) ? now - m_stamp[at] : 0, m_halflife);
}

int64_t DaiThreatModel::TrafficAt(DaiSlabCoord x, DaiSlabCoord y) const
{
    if ((x < 0) || (y < 0) || (x >= m_w) || (y >= m_h)) return 0;
    return m_traffic[(size_t)y * m_w + x];
}

void DaiThreatModel::add_threat(DaiSlabCoord x, DaiSlabCoord y, int64_t force, DaiTurn now)
{
    const size_t at = (size_t)y * m_w + x;
    m_value[at] = decayed(m_value[at], (now > m_stamp[at]) ? now - m_stamp[at] : 0, m_halflife) + force;
    m_stamp[at] = now;
}

void DaiThreatModel::add_enemies(const DaiKnowledge &k, const DaimonGlobalConfig &cfg, DaiTurn now)
{
    for (const DaiThingLook &e : k.Enemies()) {
        if (e.force <= 0) continue;
        // A fast creature's reach: where it could be by the next look.
        const int64_t reach = std::min<int64_t>(MAX_REACH, 1 + k.Kind(e.model).speed / std::max<int64_t>(1, cfg.threat_speed_per_slab));
        const DaiSlabCoord cx = DaiSubtileSlab(e.x), cy = DaiSubtileSlab(e.y);
        for (DaiSlabCoord y = std::max<DaiSlabCoord>(0, cy - reach); y <= std::min<DaiSlabCoord>(m_h - 1, cy + reach); y++)
            for (DaiSlabCoord x = std::max<DaiSlabCoord>(0, cx - reach); x <= std::min<DaiSlabCoord>(m_w - 1, cx + reach); x++)
                add_threat(x, y, e.force, now);
    }
}

/** What a reroute answers to: the rivals whose ground is seen, the hero gates seen, and how much rival ground touches
 *  the keeper's own (a breach makes it grow). The keeper's own digging, which moves the frontier all the time, does not
 *  change it. */
uint64_t DaiThreatModel::signature(const DaiKnowledge &k, PlayerNumber p) const
{
    uint64_t h = 1469598103934665603ULL;
    auto mix = [&](uint64_t v) { h = (h ^ v) * 1099511628211ULL; };
    uint64_t rivals = 0;
    int64_t contact = 0;
    for (DaiSlabCoord y = 0; y < m_h; y++) {
        for (DaiSlabCoord x = 0; x < m_w; x++) {
            const DaiSlabInfo s = k.SlabAt(x, y);
            if ((s.owner < 0) || (s.owner == p) || !k.RivalKeeper(s.owner)) continue;
            rivals |= 1ULL << s.owner;
            if (!open_ground(s.terrain)) continue;
            const int8_t (&d)[4][2] = DAI_N4_PAIRS;
            for (const auto &o : d) {
                const DaiSlabCoord nx = x + o[0], ny = y + o[1];
                if ((nx >= 0) && (ny >= 0) && (nx < m_w) && (ny < m_h) && (k.SlabAt(nx, ny).owner == p)) { contact++; break; }
            }
        }
    }
    mix(rivals);
    mix((uint64_t)contact);
    for (const DaiSlabXY &g : k.HeroGates()) { mix((uint64_t)g.x); mix((uint64_t)g.y); }
    return h;
}

void DaiThreatModel::plan(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now)
{
    m_queue.clear();
    m_qi = 0;
    m_search_on = false;
    m_building.clear();
    std::fill(m_building_traffic.begin(), m_building_traffic.end(), 0);
    std::vector<DaiSlabXY> targets, traffic_rooms;
    DaiSlabCoord hx = 0, hy = 0;
    bool heart = false;
    for (const DaiRoomLook &r : k.Rooms()) {
        if ((r.roles & TARGET_ROLES) != 0) targets.push_back(DaiSlabXY{ r.ax, r.ay });
        if (((r.roles & DAI_ROLE_HEART) != 0) && !heart) { heart = true; hx = r.ax; hy = r.ay; }
        if ((r.roles & TRAFFIC_ROLES) != 0) traffic_rooms.push_back(DaiSlabXY{ r.ax, r.ay });
    }
    if (!heart || targets.empty()) {
        finish(k, p, cfg, now);     // nothing to defend: an empty set
        return;
    }
    // Where enemies would start: each rival's open ground, the open ground nobody keeps that touches ours.
    std::vector<std::vector<DaiSlabXY>> rival(DAI_MAX_PLAYERS);
    std::vector<DaiSlabXY> frontier;
    const int8_t (&d)[4][2] = DAI_N4_PAIRS;
    for (DaiSlabCoord y = 0; y < m_h; y++) {
        for (DaiSlabCoord x = 0; x < m_w; x++) {
            const DaiSlabInfo s = k.SlabAt(x, y);
            if (!open_ground(s.terrain) || (s.owner == p)) continue;
            if ((s.owner >= 0) && k.RivalKeeper(s.owner)) { rival[(size_t)s.owner].push_back(DaiSlabXY{ x, y }); continue; }
            for (const auto &o : d) {
                const DaiSlabCoord nx = x + o[0], ny = y + o[1];
                if ((nx >= 0) && (ny >= 0) && (nx < m_w) && (ny < m_h) && (k.SlabAt(nx, ny).owner == p)) {
                    frontier.push_back(DaiSlabXY{ x, y });
                    break;
                }
            }
        }
    }
    for (PlayerNumber r = 0; r < DAI_MAX_PLAYERS; r++) {
        if (rival[(size_t)r].empty()) continue;
        Query q;
        q.source = DaiThreatSource::Rival;
        q.rival = r;
        q.weight = std::max<int64_t>(1, k.KnownForce(r, now));
        q.sources = nearest(std::move(rival[(size_t)r]), hx, hy, MAX_SOURCES);
        q.goals = targets;
        m_queue.push_back(std::move(q));
    }
    size_t gates = 0;
    for (const DaiSlabXY &g : k.HeroGates()) {
        if (gates++ >= MAX_GATE_QUERIES) break;
        Query q;
        q.source = DaiThreatSource::HeroGate;
        q.weight = cfg.threat_hero_weight;
        q.sources = { g };
        q.goals = targets;
        m_queue.push_back(std::move(q));
    }
    if (!frontier.empty()) {
        Query q;
        q.source = DaiThreatSource::Frontier;
        q.weight = cfg.threat_frontier_weight;
        q.sources = nearest(std::move(frontier), hx, hy, MAX_SOURCES);
        q.goals = targets;
        m_queue.push_back(std::move(q));
    }
    size_t walks = 0;
    for (const DaiSlabXY &room : traffic_rooms) {
        if (walks++ >= MAX_TRAFFIC_QUERIES) break;
        Query q;
        q.source = DaiThreatSource::Traffic;
        q.sources = { DaiSlabXY{ hx, hy } };
        q.goals = { room };
        m_queue.push_back(std::move(q));
    }
    if (m_queue.empty()) finish(k, p, cfg, now);
}

void DaiThreatModel::start_next(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg)
{
    const Query &q = m_queue[m_qi];
    // A rival's route through its own eyes (its walls are its own to dig); heroes' and the frontier's through no keeper's.
    const DaiDigCosts costs = (q.source == DaiThreatSource::Traffic) ? DaiDigCosts::Walk(cfg) : DaiDigCosts::EnemyView(cfg, q.rival);
    m_search.Start(k, p, q.sources, q.goals, costs);
    m_search_on = true;
}

void DaiThreatModel::finish(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now)
{
    // Chokepoints: route slabs (not its ends) on open ground nobody else keeps, whose two sides across the way are not
    // open ground -- a corridor one slab wide -- scored by the weight of the routes through them.
    std::vector<int64_t> score((size_t)m_w * m_h, 0);
    for (const DaiThreatRoute &rt : m_building) {
        for (size_t i = 1; i + 1 < rt.path.size(); i++) {
            const DaiSlabXY a = rt.path[i - 1], s = rt.path[i], b = rt.path[i + 1];
            const DaiSlabInfo si = k.SlabAt(s.x, s.y);
            if (!open_ground(si.terrain) || ((si.owner != p) && (si.owner >= 0))) continue;
            // The way through it: straight (a and b in line with it), else the step in.
            int dx = (int)(b.x - a.x), dy = (int)(b.y - a.y);
            if ((dx != 0) && (dy != 0)) { dx = (int)(s.x - a.x); dy = (int)(s.y - a.y); }
            dx = (dx > 0) - (dx < 0);
            dy = (dy > 0) - (dy < 0);
            const DaiSlabCoord lx = s.x - dy, ly = s.y + dx, rx = s.x + dy, ry = s.y - dx;
            auto closed = [&](DaiSlabCoord x, DaiSlabCoord y) {
                return (x < 0) || (y < 0) || (x >= m_w) || (y >= m_h) || !open_ground(k.SlabAt(x, y).terrain);
            };
            if (closed(lx, ly) && closed(rx, ry)) score[(size_t)s.y * m_w + s.x] += rt.weight;
        }
    }
    m_chokepoints.clear();
    for (size_t at = 0; at < score.size(); at++) {
        if (score[at] <= 0) continue;
        DaiChokepoint c;
        c.x = (DaiSlabCoord)(at % m_w);
        c.y = (DaiSlabCoord)(at / m_w);
        c.score = score[at];
        c.own = (k.SlabAt(c.x, c.y).owner == p);
        m_chokepoints.push_back(c);
    }
    std::stable_sort(m_chokepoints.begin(), m_chokepoints.end(), [](const DaiChokepoint &a, const DaiChokepoint &b) { return a.score > b.score; });
    if ((int64_t)m_chokepoints.size() > cfg.threat_max_chokepoints) m_chokepoints.resize((size_t)cfg.threat_max_chokepoints);
    m_routes = std::move(m_building);
    m_building.clear();
    m_traffic = m_building_traffic;
    m_routes_turn = now;
    m_generation++;
    m_queue.clear();
    m_qi = 0;
    m_search_on = false;
}

bool DaiThreatModel::AnyHot() const
{
    for (const DaiHazard &h : m_hazards) if (Hot(h)) return true;
    return false;
}

const DaiHazard *DaiThreatModel::HotNear(DaiSlabCoord x, DaiSlabCoord y, int64_t reach) const
{
    const DaiHazard *best = nullptr;
    int64_t bd = reach + 1;
    for (const DaiHazard &h : m_hazards) {
        const int64_t d = DaiChess(h.x, h.y, x, y);
        if (Hot(h) && (d < bd)) { best = &h; bd = d; }
    }
    return best;
}

void DaiThreatModel::hazards(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now)
{
    m_hazard_imps = cfg.defence_hazard_imps;
    std::vector<ImpSeen> imps;
    for (const DaiImpLook &i : k.OwnImps()) imps.push_back(ImpSeen{ i.ref, DaiSubtileSlab(i.x), DaiSubtileSlab(i.y) });
    // A full list may leave imps out: none counts as lost then.
    if ((int64_t)imps.size() >= DAI_MAX_OWN_IMPS) m_imps.clear();
    // Each imp gone since the last update, lost where it was last seen: to the nearest trap within reach that no keeper
    // owns (a keeper's traps stand on its own land -- the offence's ground), armed.
    for (const ImpSeen &was : m_imps) {
        bool still = false;
        for (const ImpSeen &i : imps) still = still || (i.ref == was.ref);
        if (still) continue;
        const DaiEnemyDefenceLook *best = nullptr;
        int64_t bd = cfg.defence_hazard_reach + 1;
        for (const DaiEnemyDefence &d : k.EnemyDefences()) {
            const DaiEnemyDefenceLook &l = d.look;
            if (l.door || l.spent || (l.owner == p) || k.Alliance(l.owner).keeper) continue;
            const int64_t dist = DaiChess(DaiSubtileSlab(l.x), DaiSubtileSlab(l.y), was.x, was.y);
            if (dist < bd) { best = &l; bd = dist; }
        }
        if (best == nullptr) continue;
        DaiHazard *h = nullptr;
        for (DaiHazard &x : m_hazards) if (x.trap == best->ref) h = &x;
        if (h == nullptr) {
            if (m_hazards.size() >= MAX_HAZARDS) continue;
            DaiHazard n;
            n.trap = best->ref;
            n.model = best->model;
            n.owner = best->owner;
            n.x = DaiSubtileSlab(best->x);
            n.y = DaiSubtileSlab(best->y);
            m_hazards.push_back(n);
            h = &m_hazards.back();
        }
        const bool was_hot = Hot(*h);
        h->losses.push_back(now);
        if (h->losses.size() > DAI_HAZARD_LOSSES) h->losses.erase(h->losses.begin());
        if (!was_hot && Hot(*h))
            DaiLog::Info(p, "hazard: p%d's trap %d at (%d,%d), %d imps lost by it", (int)h->owner, (int)h->model, (int)h->x, (int)h->y,
                         (int)h->losses.size());
    }
    m_imps = std::move(imps);
    // Losses older than the window go; a hazard whose trap is spent or gone, and one with no losses left, goes.
    for (DaiHazard &h : m_hazards) {
        h.losses.erase(std::remove_if(h.losses.begin(), h.losses.end(), [&](DaiTurn t) {
            return t + (DaiTurn)cfg.defence_hazard_window < now;
        }), h.losses.end());
    }
    m_hazards.erase(std::remove_if(m_hazards.begin(), m_hazards.end(), [&](const DaiHazard &h) {
        if (h.losses.empty()) return true;
        for (const DaiEnemyDefence &d : k.EnemyDefences()) if (d.look.ref == h.trap) return d.look.spent;
        DaiLog::Info(p, "hazard: the trap at (%d,%d) is gone", (int)h.x, (int)h.y);
        return true;
    }), m_hazards.end());
}

void DaiThreatModel::Update(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now)
{
    if ((k.Width() != m_w) || (k.Height() != m_h)) init(k.Width(), k.Height());
    if ((m_w <= 0) || (m_h <= 0)) return;
    hazards(k, p, cfg, now);
    m_halflife = std::max<int64_t>(1, cfg.threat_halflife);
    if ((m_last_add == 0) || (now >= m_last_add + (DaiTurn)cfg.threat_interval)) {
        add_enemies(k, cfg, now);
        m_last_add = (now > 0) ? now : 1;
    }
    if (!Searching() && ((m_last_check == 0) || (now >= m_last_check + (DaiTurn)cfg.threat_check_interval))) {
        m_last_check = (now > 0) ? now : 1;
        const uint64_t sig = signature(k, p);
        const bool due = (m_generation == 0) || (now >= m_routes_turn + (DaiTurn)cfg.threat_route_interval);
        if (due || (sig != m_signature) || m_reroute) {
            m_signature = sig;
            m_reroute = false;
            plan(k, p, cfg, now);
        }
    }
    DaiBudget b;
    b.search_nodes = cfg.threat_search_nodes;
    while (Searching() && (b.search_nodes > 0)) {
        if (!m_search_on) start_next(k, p, cfg);
        const DaiResult res = m_search.Step(k, b);
        if (res == DaiResult::Wait) break;
        const Query &q = m_queue[m_qi];
        if (res == DaiResult::Done) {
            if (q.source == DaiThreatSource::Traffic) {
                for (const DaiSlabXY &s : m_search.Path()) {
                    uint8_t &c = m_building_traffic[(size_t)s.y * m_w + s.x];
                    if (c < 255) c++;
                }
            } else {
                DaiThreatRoute rt;
                rt.source = q.source;
                rt.rival = q.rival;
                rt.weight = q.weight;
                rt.cost = m_search.Cost();
                rt.path.assign(m_search.Path().begin(), m_search.Path().end());
                m_building.push_back(std::move(rt));
            }
        }
        m_qi++;
        m_search_on = false;
        if (!Searching()) finish(k, p, cfg, now);
    }
}

void DaiThreatModel::Save(DaiBlobWriter &w) const
{
    w.I64(m_w);
    w.I64(m_h);
    w.I64(m_halflife);
    int64_t n = 0;
    for (int64_t v : m_value) n += (v != 0);
    w.I64(n);
    for (size_t i = 0; i < m_value.size(); i++) if (m_value[i] != 0) { w.U32((uint32_t)i); w.I64(m_value[i]); w.U64(m_stamp[i]); }
    w.U64(m_last_add);
    w.U64(m_last_check);
    w.U64(m_signature);
    w.Bool(m_reroute);
    save_routes(w, m_routes);
    w.I64((int64_t)m_chokepoints.size());
    for (const DaiChokepoint &c : m_chokepoints) { w.I64(c.x); w.I64(c.y); w.I64(c.score); w.Bool(c.own); }
    save_counts(w, m_traffic);
    w.U64(m_routes_turn);
    w.I64(m_generation);
    w.I64((int64_t)m_queue.size());
    for (const Query &q : m_queue) {
        w.I64((int64_t)q.source);
        w.I64(q.rival);
        w.I64(q.weight);
        save_slabs(w, q.sources);
        save_slabs(w, q.goals);
    }
    w.I64((int64_t)m_qi);
    w.Bool(m_search_on);
    if (m_search_on) m_search.Save(w);
    save_routes(w, m_building);
    save_counts(w, m_building_traffic);
    // THRT v2 (P5.13): the hazards and the imps as last seen.
    w.I64(m_hazard_imps);
    w.I64((int64_t)m_hazards.size());
    for (const DaiHazard &h : m_hazards) {
        w.Ref(h.trap); w.I64(h.model); w.I64(h.owner); w.I64(h.x); w.I64(h.y);
        w.I64((int64_t)h.losses.size());
        for (DaiTurn t : h.losses) w.U64(t);
    }
    w.I64((int64_t)m_imps.size());
    for (const ImpSeen &i : m_imps) { w.Ref(i.ref); w.I64(i.x); w.I64(i.y); }
}

bool DaiThreatModel::Load(DaiBlobReader &r)
{
    const DaiSlabCoord w = (DaiSlabCoord)r.Range(0, 4096), h = (DaiSlabCoord)r.Range(0, 4096);
    init(w, h);
    m_halflife = r.Range(1, 1000000);
    const int64_t n = r.Count((int64_t)m_value.size());
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        const uint32_t at = r.U32();
        const int64_t v = r.I64();
        const DaiTurn t = r.U64();
        if (at >= m_value.size()) { r.Fail("a threat slab off the map"); break; }
        m_value[at] = v;
        m_stamp[at] = t;
    }
    m_last_add = r.U64();
    m_last_check = r.U64();
    m_signature = r.U64();
    m_reroute = r.Bool();
    load_routes(r, m_routes);
    m_chokepoints.resize((size_t)r.Count(64));
    for (DaiChokepoint &c : m_chokepoints) {
        c.x = (DaiSlabCoord)r.Range(0, 4096);
        c.y = (DaiSlabCoord)r.Range(0, 4096);
        c.score = r.I64();
        c.own = r.Bool();
    }
    load_counts(r, m_traffic);
    m_routes_turn = r.U64();
    m_generation = r.I64();
    m_queue.resize((size_t)r.Count(256));
    for (Query &q : m_queue) {
        q.source = (DaiThreatSource)r.Range(0, (int64_t)DaiThreatSource::Count - 1);
        q.rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
        q.weight = r.I64();
        load_slabs(r, q.sources, 4096);
        load_slabs(r, q.goals, 4096);
    }
    m_qi = (size_t)r.Range(0, (int64_t)m_queue.size());
    m_search_on = r.Bool() && (m_qi < m_queue.size());
    if (m_search_on && !m_search.Load(r)) return false;
    load_routes(r, m_building);
    load_counts(r, m_building_traffic);
    m_hazard_imps = r.Range(1, 100);
    m_hazards.resize((size_t)r.Count((int64_t)MAX_HAZARDS));
    for (DaiHazard &hz : m_hazards) {
        hz.trap = r.Ref();
        hz.model = (DaiThingModel)r.Range(0, 255); hz.owner = (PlayerNumber)r.Range(-1, 255);
        hz.x = (DaiSlabCoord)r.Range(0, 4096); hz.y = (DaiSlabCoord)r.Range(0, 4096);
        hz.losses.resize((size_t)r.Count((int64_t)DAI_HAZARD_LOSSES));
        for (DaiTurn &t : hz.losses) t = (DaiTurn)r.U64();
    }
    m_imps.resize((size_t)r.Count(DAI_MAX_OWN_IMPS));
    for (ImpSeen &i : m_imps) {
        i.ref = r.Ref();
        i.x = (DaiSlabCoord)r.Range(0, 4096); i.y = (DaiSlabCoord)r.Range(0, 4096);
    }
    return r.Ok();
}
