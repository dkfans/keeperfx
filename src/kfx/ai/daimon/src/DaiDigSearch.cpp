#include "pre_inc.h"
#include "daimon/DaiDigSearch.h"

#include <algorithm>

#include "post_inc.h"

namespace {
/** The open list's order: least f, then least g, then least slab index, on top (a max-heap of the reverse). */
struct Later {
    template <typename N> bool operator()(const N &a, const N &b) const
    {
        if (a.f != b.f) return a.f > b.f;
        if (a.g != b.g) return a.g > b.g;
        return a.at > b.at;
    }
};

bool in_rect(const DaiRect &r, DaiSlabCoord x, DaiSlabCoord y)
{
    return (x >= r.x0) && (x <= r.x1) && (y >= r.y0) && (y <= r.y1);
}

bool next_to_abyss(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y)
{
    return (k.SlabAt(x, y - 1).terrain == DaiTerrain::Abyss) || (k.SlabAt(x + 1, y).terrain == DaiTerrain::Abyss)
        || (k.SlabAt(x, y + 1).terrain == DaiTerrain::Abyss) || (k.SlabAt(x - 1, y).terrain == DaiTerrain::Abyss);
}

int32_t clamp_cost(int64_t v) { return (int32_t)std::clamp<int64_t>(v, 1, DAI_DIG_NEVER - 1); }
}

DaiDigCosts DaiDigCosts::OwnRoute(const DaimonGlobalConfig &cfg, const DaiPersonality &p, const DaiKnowledge &k)
{
    DaiDigCosts c;
    c.model = DaiDigModel::OwnRoute;
    c.floor = clamp_cost(cfg.dig_floor);
    c.earth = clamp_cost(cfg.dig_earth);
    c.unknown = clamp_cost(p.excavation_unknown_cost);
    c.gold_route = clamp_cost(cfg.dig_gold_route);
    c.dense_route = clamp_cost(cfg.dig_dense_route);
    c.own_wall = clamp_cost(cfg.dig_own_wall);
    c.enemy_floor = clamp_cost(cfg.dig_enemy_floor);
    c.abyss_edge = (int32_t)std::clamp<int64_t>(cfg.dig_abyss_edge, 0, 1000000);
    c.heuristic_weight = std::max<DaiFix>(cfg.dig_heuristic_weight, DAI_FIX_ONE);
    const DaiBuildLook &bridge = k.Build(DaiRoom::Bridge);
    if (bridge.available) {
        c.bridge = clamp_cost(cfg.dig_bridge_base + FixToInt(FixMul(FixFromInt(bridge.price), p.excavation_gold_to_turns)));
        c.bridge_lava = k.BridgeRoles().lava;
        c.bridge_abyss = k.BridgeRoles().abyss;
    }
    return c;
}

DaiDigCosts DaiDigCosts::EnemyView(const DaimonGlobalConfig &cfg, PlayerNumber viewer)
{
    DaiDigCosts c;
    c.model = DaiDigModel::EnemyView;
    c.floor = clamp_cost(cfg.dig_floor);
    c.earth = clamp_cost(cfg.dig_earth);
    c.unknown = clamp_cost(cfg.dig_earth);      // a rival knows its way; unknown to us is most likely earth
    c.gold_route = clamp_cost(cfg.dig_gold_route);
    c.dense_route = clamp_cost(cfg.dig_dense_route);
    c.own_wall = clamp_cost(cfg.dig_own_wall);  // the viewer's own walls: its imps dig them
    c.enemy_floor = c.floor;
    c.abyss_edge = 0;
    c.heuristic_weight = std::max<DaiFix>(cfg.dig_heuristic_weight, DAI_FIX_ONE);
    c.viewer = viewer;
    c.destroy_walls = (cfg.threat_destroy_walls > 0) ? clamp_cost(cfg.threat_destroy_walls) : DAI_DIG_NEVER;
    c.own_door_per_hp = cfg.threat_own_door_per_hp;
    return c;
}

DaiDigCosts DaiDigCosts::Walk(const DaimonGlobalConfig &cfg)
{
    DaiDigCosts c;
    c.model = DaiDigModel::Walk;
    c.floor = clamp_cost(cfg.dig_floor);
    c.earth = c.unknown = c.gold_route = c.dense_route = c.own_wall = c.enemy_floor = DAI_DIG_NEVER;
    c.abyss_edge = 0;
    c.heuristic_weight = std::max<DaiFix>(cfg.dig_heuristic_weight, DAI_FIX_ONE);
    return c;
}

DaiDigCosts DaiDigCosts::ClaimRoute(const DaimonGlobalConfig &cfg, const DaiPersonality &p, const DaiKnowledge &k)
{
    DaiDigCosts c = OwnRoute(cfg, p, k);
    c.model = DaiDigModel::ClaimRoute;
    c.bridge_water = true;
    if (!k.Build(DaiRoom::Bridge).available || !k.BridgeRoles().water) c.bridge = DAI_DIG_NEVER;
    return c;
}

/** A door of the keeper's this update's look does not have (it was just placed): priced as one of this health. */
constexpr int64_t DOOR_HP_UNKNOWN = 400;

DaiDigCosts DaiDigCosts::Attack(const DaimonGlobalConfig &cfg, const DaiPersonality &p, const DaiKnowledge &k)
{
    DaiDigCosts c = OwnRoute(cfg, p, k);
    c.model = DaiDigModel::Attack;
    // The way to the staging point becomes the keeper's land (its creatures are dropped there): water on it is bridged,
    // as a claim route's, and with no bridge that can cross it there is no way (level 52: a target across the lake, the
    // breach's claim-route digs refused at once, three times).
    c.bridge_water = true;
    if (!k.Build(DaiRoom::Bridge).available || !k.BridgeRoles().water) c.bridge = DAI_DIG_NEVER;
    c.creep = clamp_cost(cfg.dig_creep_cost);
    c.door_per_hp = cfg.dig_door_cost_per_hp;
    const DaiPowerLook &dw = k.Power(DaiPower::DestroyWalls);
    if (!dw.available) c.destroy_walls = DAI_DIG_NEVER;
    else if (cfg.dig_destroy_walls_cost > 0) c.destroy_walls = clamp_cost(cfg.dig_destroy_walls_cost);
    else c.destroy_walls = clamp_cost(std::max<int64_t>(1, FixToInt(FixMul(FixFromInt(dw.price), p.excavation_gold_to_turns))));
    return c;
}

/** Attack: another keeper's door, by its health as last seen. */
static int32_t enemy_door_cost(const DaiKnowledge &k, const DaiDigCosts &c, DaiSlabCoord x, DaiSlabCoord y)
{
    const DaiEnemyDefence *d = k.EnemyDefenceAt(x, y);
    const int64_t hp = ((d != nullptr) && d->look.door && (d->look.health > 0)) ? d->look.health : DOOR_HP_UNKNOWN;
    return clamp_cost(std::max<int64_t>(c.floor, FixToInt(FixMul(FixFromInt(hp), c.door_per_hp))));
}

/** EnemyView: the slab as a rival's creatures and imps would cross it. */
static int32_t enemy_cost(const DaiKnowledge &k, const DaiDigCosts &c, const DaiSlabInfo &s, bool own, DaiSlabCoord x, DaiSlabCoord y)
{
    switch (s.terrain) {
    case DaiTerrain::Unknown: return c.unknown;
    case DaiTerrain::Dirt: return c.earth;
    case DaiTerrain::Gold: return c.gold_route;
    case DaiTerrain::DenseGold: return c.dense_route;
    case DaiTerrain::Water: case DaiTerrain::Floor: case DaiTerrain::Room: return c.floor;
    case DaiTerrain::Door: {
        if (!own) return c.floor;               // its own door, or another's it has to get past anyway
        const DaiDoorLook *d = k.DoorAt(x, y);
        const int64_t hp = ((d != nullptr) && (d->health > 0)) ? d->health : DOOR_HP_UNKNOWN;
        return clamp_cost(std::max<int64_t>(c.floor, FixToInt(FixMul(FixFromInt(hp), c.own_door_per_hp))));
    }
    // A fortified wall: its owner's imps dig it, nobody else's do -- only a Destroy Walls cast breaks it.
    case DaiTerrain::Wall: return ((c.viewer >= 0) && (s.owner == c.viewer)) ? c.own_wall : c.destroy_walls;
    default: return DAI_DIG_NEVER;              // rock, gems, lava, abyss: no way through
    }
}

/** Walk: the keeper's own creatures between its rooms. */
static int32_t walk_cost(const DaiDigCosts &c, const DaiSlabInfo &s, bool own)
{
    switch (s.terrain) {
    case DaiTerrain::Water: return c.floor;
    case DaiTerrain::Floor: case DaiTerrain::Room: case DaiTerrain::Door: return (own || (s.owner < 0)) ? c.floor : DAI_DIG_NEVER;
    default: return DAI_DIG_NEVER;
    }
}

int32_t DaiDigSearch::EnterCost(const DaiKnowledge &k, PlayerNumber p, const DaiDigCosts &c, DaiSlabCoord x, DaiSlabCoord y,
                                bool goal)
{
    if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height())) return DAI_DIG_NEVER;
    const DaiSlabInfo s = k.SlabAt(x, y);
    const bool own = (s.owner == p), nobody = (s.owner < 0);
    if (c.model == DaiDigModel::EnemyView) return enemy_cost(k, c, s, own, x, y);
    // The keeper's perimeter (plan 13 section 32): its imps never dig a kept slab -- its way out is the gate.
    if ((c.model != DaiDigModel::Walk) && k.Kept(x, y)) {
        switch (s.terrain) {
        case DaiTerrain::Unknown: case DaiTerrain::Dirt: case DaiTerrain::Gold: case DaiTerrain::DenseGold: case DaiTerrain::Wall:
            return DAI_DIG_NEVER;
        default: break;
        }
    }
    if (c.model == DaiDigModel::Walk) return walk_cost(c, s, own);
    int64_t cost = DAI_DIG_NEVER;
    bool ground = true;         // stood on or dug into: the abyss edge applies
    const bool attack = (c.model == DaiDigModel::Attack);
    switch (s.terrain) {
    case DaiTerrain::Unknown: cost = c.unknown; break;
    case DaiTerrain::Dirt: cost = c.earth; break;
    case DaiTerrain::Gold: cost = goal ? c.earth : c.gold_route; break;
    case DaiTerrain::DenseGold: cost = goal ? c.earth : c.dense_route; break;
    case DaiTerrain::Gem: cost = goal ? c.earth : DAI_DIG_NEVER; break;    // mined in place, never dug through
    case DaiTerrain::Water:
        if (c.bridge_water) { cost = c.bridge; ground = false; }
        else cost = c.floor;                            // waded (the navigation lets walkers through)
        break;
    case DaiTerrain::Floor: cost = (own || nobody) ? c.floor : attack ? c.creep : c.enemy_floor; break;
    case DaiTerrain::Room: cost = (own || nobody) ? c.floor : attack ? c.creep : DAI_DIG_NEVER; break;
    case DaiTerrain::Door: cost = own ? c.floor : attack ? enemy_door_cost(k, c, x, y) : DAI_DIG_NEVER; break;
    case DaiTerrain::Wall: cost = own ? c.own_wall : attack ? c.destroy_walls : DAI_DIG_NEVER; break;
    case DaiTerrain::Lava: cost = c.bridge_lava ? c.bridge : DAI_DIG_NEVER; ground = false; break;
    case DaiTerrain::Abyss: cost = c.bridge_abyss ? c.bridge : DAI_DIG_NEVER; ground = false; break;
    default: break;             // rock, impassable
    }
    if (cost >= DAI_DIG_NEVER) return DAI_DIG_NEVER;
    if (ground && (c.abyss_edge > 0) && next_to_abyss(k, x, y)) cost += c.abyss_edge;
    return clamp_cost(cost);
}

void DaiDigSearch::Start(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> sources, std::span<const DaiSlabXY> goals,
                         const DaiDigCosts &costs, std::span<const DaiRect> blocked)
{
    m_player = p;
    m_costs = costs;
    m_from = From::Given;
    m_sources_in.assign(sources.begin(), sources.end());
    m_goals_in.assign(goals.begin(), goals.end());
    m_blocked.assign(blocked.begin(), blocked.end());
    m_expanded = 0;
    m_restarts = 0;
    restart(k);
    m_restarts = 0;
}

void DaiDigSearch::StartFromReach(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> goals, const DaiDigCosts &costs,
                                  std::span<const DaiRect> blocked)
{
    Start(k, p, {}, goals, costs, blocked);
    m_from = From::Reach;
    restart(k);
    m_restarts = 0;
}

void DaiDigSearch::StartFromOwnLand(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> goals, const DaiDigCosts &costs,
                                    std::span<const DaiRect> blocked)
{
    Start(k, p, {}, goals, costs, blocked);
    m_from = From::OwnLand;
    restart(k);
    m_restarts = 0;
}

void DaiDigSearch::StartFlood(const DaiKnowledge &k, PlayerNumber p, const DaiDigCosts &costs, std::span<const DaiRect> blocked)
{
    Start(k, p, {}, {}, costs, blocked);
    m_from = From::OwnLandFlood;
    restart(k);
    m_restarts = 0;
}

int32_t DaiDigSearch::CostAt(DaiSlabCoord x, DaiSlabCoord y) const
{
    if ((m_from != From::OwnLandFlood) || (m_state != DaiResult::Done) || (x < 0) || (y < 0) || (x >= m_w) || (y >= m_h))
        return DAI_DIG_NEVER;
    return m_g[(size_t)y * m_w + x];
}

void DaiDigSearch::restart(const DaiKnowledge &k)
{
    m_restarts++;
    m_w = k.Width();
    m_h = k.Height();
    const size_t n = (size_t)m_w * (size_t)m_h;
    m_g.assign(n, DAI_DIG_NEVER);
    m_parent.assign(n, -1);
    m_closed.assign(n, 0);
    m_goal.assign(n, 0);
    m_open.clear();
    m_sources.clear();
    m_seeded = 0;
    m_path.clear();
    m_cost = 0;
    m_seen_seq = k.ChangeSeq();
    auto on_map = [&](const DaiSlabXY &s) { return (s.x >= 0) && (s.y >= 0) && (s.x < m_w) && (s.y < m_h); };
    // The goals and the box round them (the heuristic's target: no further than the nearest goal).
    bool any_goal = false;
    for (const DaiSlabXY &g : m_goals_in) {
        if (!on_map(g)) continue;
        m_goal[(size_t)g.y * m_w + g.x] = 1;
        if (!any_goal) m_goal_box = DaiRect{ g.x, g.y, g.x, g.y };
        m_goal_box.x0 = std::min(m_goal_box.x0, g.x); m_goal_box.y0 = std::min(m_goal_box.y0, g.y);
        m_goal_box.x1 = std::max(m_goal_box.x1, g.x); m_goal_box.y1 = std::max(m_goal_box.y1, g.y);
        any_goal = true;
    }
    if (m_from == From::Reach) {
        for (DaiSlabCoord y = 0; y < m_h; y++)
            for (DaiSlabCoord x = 0; x < m_w; x++)
                if (k.Reach().Reached(x, y)) m_sources.push_back((int32_t)(y * m_w + x));
    } else if ((m_from == From::OwnLand) || (m_from == From::OwnLandFlood)) {
        for (DaiSlabCoord y = 0; y < m_h; y++) {
            for (DaiSlabCoord x = 0; x < m_w; x++) {
                const DaiSlabInfo s = k.SlabAt(x, y);
                const bool land = (s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room) || (s.terrain == DaiTerrain::Door);
                if (land && (s.owner == m_player)) m_sources.push_back((int32_t)(y * m_w + x));
            }
        }
    } else {
        std::vector<uint8_t> seen(n, 0);
        for (const DaiSlabXY &s : m_sources_in) {
            if (!on_map(s)) continue;
            const int32_t at = (int32_t)(s.y * m_w + s.x);
            if (seen[(size_t)at]) continue;
            seen[(size_t)at] = 1;
            m_sources.push_back(at);
        }
    }
    // The cheapest step any slab can cost: Manhattan distance times it never overestimates.
    m_min_step = std::min({ m_costs.floor, m_costs.earth, m_costs.unknown, m_costs.gold_route, m_costs.dense_route,
                            m_costs.own_wall, m_costs.enemy_floor, m_costs.bridge });
    const bool flood = (m_from == From::OwnLandFlood);
    if (flood) m_goal_box = DaiRect{ -1, -1, -1, -1 };
    m_state = ((any_goal || flood) && !m_sources.empty()) ? DaiResult::Wait : DaiResult::Fail;
}

bool DaiDigSearch::touched(int32_t at) const
{
    return (at >= 0) && ((size_t)at < m_g.size()) && (m_g[(size_t)at] != DAI_DIG_NEVER);
}

int64_t DaiDigSearch::heuristic(int32_t at) const
{
    if (m_from == From::OwnLandFlood) return 0;     // no goal: Dijkstra
    const DaiSlabCoord x = at % m_w, y = at / m_w;
    const int64_t dx = (x < m_goal_box.x0) ? m_goal_box.x0 - x : (x > m_goal_box.x1) ? x - m_goal_box.x1 : 0;
    const int64_t dy = (y < m_goal_box.y0) ? m_goal_box.y0 - y : (y > m_goal_box.y1) ? y - m_goal_box.y1 : 0;
    return FixToInt(FixMul(FixFromInt((dx + dy) * m_min_step), m_costs.heuristic_weight));
}

void DaiDigSearch::push(int32_t at, int32_t g, int32_t parent)
{
    m_g[(size_t)at] = g;
    m_parent[(size_t)at] = parent;
    m_open.push_back(Node{ g + heuristic(at), g, at });
    std::push_heap(m_open.begin(), m_open.end(), Later());
}

DaiResult DaiDigSearch::Step(const DaiKnowledge &k, DaiBudget &b)
{
    if (m_state != DaiResult::Wait) return m_state;
    // Ground it has touched changed since the last step (or the log lost track): start over.
    bool stale = (k.Width() != m_w) || (k.Height() != m_h);
    if (!stale) {
        const bool followed = k.ChangesSince(m_seen_seq, [&](DaiSlabCoord x, DaiSlabCoord y) {
            const int32_t at = (int32_t)(y * m_w + x);
            stale = stale || touched(at) || ((y > 0) && touched(at - m_w)) || ((x + 1 < m_w) && touched(at + 1))
                || ((y + 1 < m_h) && touched(at + m_w)) || ((x > 0) && touched(at - 1));
        });
        stale = stale || !followed;
    }
    m_seen_seq = k.ChangeSeq();
    if (stale) {
        restart(k);
        if (m_state != DaiResult::Wait) return m_state;
    }
    auto blocked = [&](DaiSlabCoord x, DaiSlabCoord y) {
        for (const DaiRect &r : m_blocked) if (in_rect(r, x, y)) return true;
        return false;
    };
    while ((m_seeded < m_sources.size()) && (b.search_nodes > 0)) {
        const int32_t at = m_sources[m_seeded++];
        b.search_nodes--;
        m_expanded++;
        if ((m_g[(size_t)at] != 0) && !blocked(at % m_w, at / m_w)) push(at, 0, -1);
    }
    if (m_seeded < m_sources.size()) return DaiResult::Wait;
    while (b.search_nodes > 0) {
        if (m_open.empty()) {
            m_state = (m_from == From::OwnLandFlood) ? DaiResult::Done : DaiResult::Fail;
            return m_state;
        }
        std::pop_heap(m_open.begin(), m_open.end(), Later());
        const Node nd = m_open.back();
        m_open.pop_back();
        if (m_closed[(size_t)nd.at] || (nd.g != m_g[(size_t)nd.at])) continue;   // a stale copy
        m_closed[(size_t)nd.at] = 1;
        b.search_nodes--;
        m_expanded++;
        if (m_goal[(size_t)nd.at]) {
            m_cost = nd.g;
            for (int32_t at = nd.at; at >= 0; at = m_parent[(size_t)at]) m_path.push_back(DaiSlabXY{ at % m_w, at / m_w });
            std::reverse(m_path.begin(), m_path.end());
            m_state = DaiResult::Done;
            return m_state;
        }
        const DaiSlabCoord x = nd.at % m_w, y = nd.at / m_w;
        const int32_t next[4] = { (y > 0) ? nd.at - (int32_t)m_w : -1, (x + 1 < m_w) ? nd.at + 1 : -1,
                                  (y + 1 < m_h) ? nd.at + (int32_t)m_w : -1, (x > 0) ? nd.at - 1 : -1 };
        for (int32_t nb : next) {
            if ((nb < 0) || m_closed[(size_t)nb]) continue;
            const DaiSlabCoord nx = nb % m_w, ny = nb / m_w;
            if (blocked(nx, ny)) continue;
            // Onto a guard post from water the model wades: a climb of two, no step (P5.14). Bridged water is floor.
            if (!m_costs.bridge_water && k.RaisedStep(x, y, nx, ny)) continue;
            const int32_t step = EnterCost(k, m_player, m_costs, nx, ny, m_goal[(size_t)nb] != 0);
            if (step >= DAI_DIG_NEVER) continue;
            const int64_t g = (int64_t)nd.g + step;
            if (g >= m_g[(size_t)nb]) continue;
            push(nb, (int32_t)std::min<int64_t>(g, DAI_DIG_NEVER - 1), nd.at);
        }
    }
    return DaiResult::Wait;
}

int64_t DaiDigSearch::Path(DaiSlabXY *out, int64_t max) const
{
    const int64_t n = std::min<int64_t>((int64_t)m_path.size(), max);
    for (int64_t i = 0; i < n; i++) out[i] = m_path[(size_t)i];
    return (int64_t)m_path.size();
}

namespace {
void save_costs(DaiBlobWriter &w, const DaiDigCosts &c)
{
    w.I64((int64_t)c.model);
    for (int32_t v : { c.floor, c.earth, c.unknown, c.gold_route, c.dense_route, c.own_wall, c.enemy_floor, c.bridge, c.abyss_edge })
        w.I64(v);
    w.Bool(c.bridge_lava);
    w.Bool(c.bridge_abyss);
    w.Bool(c.bridge_water);
    w.I64(c.heuristic_weight);
    if (c.model == DaiDigModel::EnemyView) { w.I64(c.viewer); w.I64(c.destroy_walls); w.I64(c.own_door_per_hp); }
    if (c.model == DaiDigModel::Attack) { w.I64(c.creep); w.I64(c.door_per_hp); w.I64(c.destroy_walls); }
}

void load_costs(DaiBlobReader &r, DaiDigCosts *c)
{
    c->model = (DaiDigModel)r.Range(0, (int64_t)DaiDigModel::Walk);
    for (int32_t *v : { &c->floor, &c->earth, &c->unknown, &c->gold_route, &c->dense_route, &c->own_wall, &c->enemy_floor,
                        &c->bridge, &c->abyss_edge })
        *v = (int32_t)r.Range(0, DAI_DIG_NEVER);
    c->bridge_lava = r.Bool();
    c->bridge_abyss = r.Bool();
    c->bridge_water = r.Bool();
    c->heuristic_weight = r.Range(DAI_FIX_ONE, 1000 * DAI_FIX_ONE);
    if (c->model == DaiDigModel::EnemyView) {
        c->viewer = (PlayerNumber)r.Range(-1, 63);
        c->destroy_walls = (int32_t)r.Range(1, DAI_DIG_NEVER);
        c->own_door_per_hp = r.Range(0, 1000 * DAI_FIX_ONE);
    }
    if (c->model == DaiDigModel::Attack) {
        c->creep = (int32_t)r.Range(1, DAI_DIG_NEVER);
        c->door_per_hp = r.Range(0, 1000 * DAI_FIX_ONE);
        c->destroy_walls = (int32_t)r.Range(1, DAI_DIG_NEVER);
    }
}

void save_slabs(DaiBlobWriter &w, const std::vector<DaiSlabXY> &v)
{
    w.I64((int64_t)v.size());
    for (const DaiSlabXY &s : v) { w.I64(s.x); w.I64(s.y); }
}

void load_slabs(DaiBlobReader &r, std::vector<DaiSlabXY> *v, int64_t max)
{
    const int64_t n = r.Count(max);
    v->clear();
    for (int64_t i = 0; (i < n) && r.Ok(); i++) v->push_back(DaiSlabXY{ (DaiSlabCoord)r.Range(-1, 256), (DaiSlabCoord)r.Range(-1, 256) });
}
}

void DaiDigSearch::Save(DaiBlobWriter &w) const
{
    w.I64(m_player);
    w.I64(m_w);
    w.I64(m_h);
    save_costs(w, m_costs);
    w.I64((int64_t)m_from);
    save_slabs(w, m_sources_in);
    save_slabs(w, m_goals_in);
    w.I64((int64_t)m_blocked.size());
    for (const DaiRect &b : m_blocked) { w.I64(b.x0); w.I64(b.y0); w.I64(b.x1); w.I64(b.y1); }
    w.I64((int64_t)m_sources.size());
    for (int32_t s : m_sources) w.U32((uint32_t)s);
    // The touched slabs only: a search rarely touches more than a part of the map.
    int64_t touched = 0;
    for (int32_t g : m_g) if (g != DAI_DIG_NEVER) touched++;
    w.I64(touched);
    for (size_t at = 0; at < m_g.size(); at++) {
        if (m_g[at] == DAI_DIG_NEVER) continue;
        w.U32((uint32_t)at);
        w.U32((uint32_t)m_g[at]);
        w.U32((uint32_t)(m_parent[at] + 1));
        w.Bool(m_closed[at] != 0);
    }
    w.I64((int64_t)m_open.size());
    for (const Node &nd : m_open) { w.I64(nd.f); w.U32((uint32_t)nd.g); w.U32((uint32_t)nd.at); }
    w.I64((int64_t)m_seeded);
    w.I64(m_min_step);
    w.U64(m_seen_seq);
    save_slabs(w, m_path);
    w.I64(m_cost);
    w.I64((int64_t)m_state);
    w.I64(m_expanded);
    w.I64(m_restarts);
}

bool DaiDigSearch::Load(DaiBlobReader &r)
{
    m_player = (PlayerNumber)r.Range(-1, 255);
    m_w = (DaiSlabCoord)r.Range(0, 256);
    m_h = (DaiSlabCoord)r.Range(0, 256);
    const int64_t n = (int64_t)m_w * m_h;
    load_costs(r, &m_costs);
    m_from = (From)r.Range(0, (int64_t)From::OwnLandFlood);
    load_slabs(r, &m_sources_in, n);
    load_slabs(r, &m_goals_in, n);
    const int64_t nb = r.Count(1024);
    m_blocked.clear();
    for (int64_t i = 0; (i < nb) && r.Ok(); i++) {
        DaiRect b;
        b.x0 = (DaiSlabCoord)r.I64(); b.y0 = (DaiSlabCoord)r.I64(); b.x1 = (DaiSlabCoord)r.I64(); b.y1 = (DaiSlabCoord)r.I64();
        m_blocked.push_back(b);
    }
    const int64_t ns = r.Count(n);
    m_sources.clear();
    for (int64_t i = 0; (i < ns) && r.Ok(); i++) {
        const uint32_t s = r.U32();
        if (s >= (uint64_t)n) r.Fail("search source");
        m_sources.push_back((int32_t)s);
    }
    // The goals and their box as restart() makes them.
    m_goal.assign((size_t)n, 0);
    bool any = false;
    for (const DaiSlabXY &g : m_goals_in) {
        if ((g.x < 0) || (g.y < 0) || (g.x >= m_w) || (g.y >= m_h)) continue;
        m_goal[(size_t)g.y * m_w + g.x] = 1;
        if (!any) m_goal_box = DaiRect{ g.x, g.y, g.x, g.y };
        m_goal_box.x0 = std::min(m_goal_box.x0, g.x); m_goal_box.y0 = std::min(m_goal_box.y0, g.y);
        m_goal_box.x1 = std::max(m_goal_box.x1, g.x); m_goal_box.y1 = std::max(m_goal_box.y1, g.y);
        any = true;
    }
    m_g.assign((size_t)n, DAI_DIG_NEVER);
    m_parent.assign((size_t)n, -1);
    m_closed.assign((size_t)n, 0);
    const int64_t touched = r.Count(n);
    for (int64_t i = 0; (i < touched) && r.Ok(); i++) {
        const uint32_t at = r.U32(), g = r.U32(), parent = r.U32();
        const bool closed = r.Bool();
        if ((at >= (uint64_t)n) || (g >= (uint32_t)DAI_DIG_NEVER) || (parent > (uint64_t)n)) { r.Fail("search slab"); break; }
        m_g[at] = (int32_t)g;
        m_parent[at] = (int32_t)parent - 1;
        m_closed[at] = closed ? 1 : 0;
    }
    const int64_t no = r.Count(4 * n + 4);
    m_open.clear();
    for (int64_t i = 0; (i < no) && r.Ok(); i++) {
        Node nd;
        nd.f = r.Range(0, INT64_MAX);
        nd.g = (int32_t)r.U32();
        nd.at = (int32_t)r.U32();
        if ((nd.at < 0) || (nd.at >= n) || (nd.g < 0)) { r.Fail("search node"); break; }
        m_open.push_back(nd);
    }
    if (r.Ok() && !std::is_heap(m_open.begin(), m_open.end(), Later())) r.Fail("search heap");
    m_seeded = (size_t)r.Range(0, (int64_t)m_sources.size());
    m_min_step = r.Range(1, DAI_DIG_NEVER);
    m_seen_seq = r.U64();
    load_slabs(r, &m_path, n);
    m_cost = r.Range(0, INT64_MAX);
    m_state = (DaiResult)r.Range(0, (int64_t)DaiResult::BadInput);
    m_expanded = r.Range(0, INT64_MAX);
    m_restarts = r.Range(0, INT64_MAX);
    if (!r.Ok()) {
        *this = DaiDigSearch();
        return false;
    }
    return true;
}
