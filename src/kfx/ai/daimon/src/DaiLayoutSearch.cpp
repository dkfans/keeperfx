/** @file DaiLayoutSearch.cpp
 *  The layout planner (DaiLayoutSearch.h, 04 section 5). */
#include "pre_inc.h"
#include "daimon/DaiLayoutSearch.h"

#include <algorithm>

#include "room_side_score.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** The largest room drag the game builds in one go (MAX_USER_ROOMSPACE_WIDTH): a site is no wider. */
constexpr int32_t SITE_MAX_SIDE = 9;
/** Deepest strip a room grows by at once. */
constexpr int32_t GROW_MAX_DEPTH = 3;
/** How far from a site its enemies' slabs are counted (Phase 2's stand-in for Phase 4's threat model). */
constexpr DaiSlabCoord EXPOSURE_RANGE = 8;

bool on_map(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y)
{
    return (x >= 0) && (y >= 0) && (x < k.Width()) && (y < k.Height());
}

bool in_rect(const DaiRect &r, DaiSlabCoord x, DaiSlabCoord y)
{
    return (x >= r.x0) && (x <= r.x1) && (y >= r.y0) && (y <= r.y1);
}

bool valuable(DaiTerrain t)
{
    return (t == DaiTerrain::Gold) || (t == DaiTerrain::DenseGold) || (t == DaiTerrain::Gem);
}

/** What a room slab's neighbour will be once the room stands (04 section 5.2): earth beside the keeper's room is
 *  fortified by its imps, unknown ground counts as natural ground, the entrance is open or a door. */
template <typename Inside>
RoomNeighbour classify(const DaiKnowledge &k, PlayerNumber p, Inside inside, DaiSlabCoord x, DaiSlabCoord y,
                       DaiSlabCoord ent_x, DaiSlabCoord ent_y, bool door)
{
    if (!on_map(k, x, y)) return RoNb_OffMap;
    if (inside(x, y)) return RoNb_SameRoom;
    if ((x == ent_x) && (y == ent_y)) return door ? RoNb_OwnDoor : RoNb_Open;
    const DaiSlabInfo s = k.SlabAt(x, y);
    switch (s.terrain) {
    case DaiTerrain::Dirt:
        return RoNb_OwnWall;
    case DaiTerrain::Unknown: case DaiTerrain::Rock: case DaiTerrain::Gold: case DaiTerrain::DenseGold:
    case DaiTerrain::Gem: case DaiTerrain::Impassable:
        return RoNb_Natural;
    case DaiTerrain::Wall:
        return (s.owner == p) ? RoNb_OwnWall : RoNb_OtherWall;
    case DaiTerrain::Door:
        return (s.owner == p) ? RoNb_OwnDoor : RoNb_Open;
    default:
        return RoNb_Open;
    }
}

/** The engine's efficiency rule (room_side_score.h) on the slabs `inside` says are the room, within `box`. */
template <typename Inside>
int32_t predict_in(const DaiKnowledge &k, PlayerNumber p, Inside inside, const DaiRect &box, DaiSlabCoord ent_x,
                   DaiSlabCoord ent_y, bool door)
{
    const int *const dx4 = DAI_N4_DX, *const dy4 = DAI_N4_DY;
    static const int dxc[4] = { -1, 1, 1, -1 }, dyc[4] = { -1, -1, 1, 1 };
    int64_t n = 0, score = 0;
    for (DaiSlabCoord y = box.y0; y <= box.y1; y++) {
        for (DaiSlabCoord x = box.x0; x <= box.x1; x++) {
            if (!inside(x, y)) continue;
            RoomNeighbour sides[4], corners[4];
            for (int d = 0; d < 4; d++) {
                sides[d] = classify(k, p, inside, (DaiSlabCoord)(x + dx4[d]), (DaiSlabCoord)(y + dy4[d]), ent_x, ent_y, door);
                corners[d] = classify(k, p, inside, (DaiSlabCoord)(x + dxc[d]), (DaiSlabCoord)(y + dyc[d]), ent_x, ent_y, door);
            }
            score += room_slab_score_from(sides, corners);
            n++;
        }
    }
    return (int32_t)room_efficiency_from_score(n, score);
}
}

DaiLayoutWeights DaiLayoutWeights::From(const DaiPersonality &p)
{
    DaiLayoutWeights w;
    w.eff = p.layout_w_eff;
    w.adj = p.layout_w_adj;
    w.dist = p.layout_w_dist;
    w.dig = p.layout_w_dig;
    w.exp = p.layout_w_exp;
    w.grow = p.layout_w_grow;
    w.waste = p.layout_w_waste;
    return w;
}

int32_t DaiLayoutSearch::PredictEfficiency(const DaiKnowledge &k, PlayerNumber p, const DaiRect &r, DaiSlabCoord ent_x,
                                           DaiSlabCoord ent_y, bool door)
{
    return predict_in(k, p, [&](DaiSlabCoord x, DaiSlabCoord y) { return in_rect(r, x, y); }, r, ent_x, ent_y, door);
}

void DaiLayoutSearch::Start(const DaiKnowledge &k, PlayerNumber p, const DaiLayoutQuery &q, const DaiLayoutWeights &w,
                            const DaiDigCosts &claim_costs, int64_t radius, int64_t rim, std::span<const DaiRect> blocked)
{
    m_player = p;
    m_q = q;
    m_w = w;
    m_radius = std::clamp<int64_t>(radius, 0, 64);
    m_rim = std::clamp<int64_t>(rim, 1, 2);
    m_blocked.assign(blocked.begin(), blocked.end());
    m_costs = claim_costs;
    m_next = 0;
    m_feasible = 0;
    m_best = DaiSite();
    m_room_box = DaiRect{ -1, -1, -1, -1 };
    m_flood = DaiDigSearch();
    m_state = DaiResult::Wait;
    if (q.grow_record != 0) {
        const DaiRoomRecord *rec = k.RoomRecord(q.grow_record);
        if ((rec == nullptr) || (rec->index == 0)) m_state = DaiResult::Fail;
        else m_room_box = rec->box;
    } else {
        m_flood.StartFlood(k, p, claim_costs, blocked);
    }
    prepare();
}

void DaiLayoutSearch::prepare()
{
    // The site middles: the anchor, then ring after ring round it, each ring from its top-left corner clockwise.
    m_offsets.clear();
    m_offsets.push_back(DaiSlabXY{ 0, 0 });
    for (DaiSlabCoord r = 1; r <= (DaiSlabCoord)m_radius; r++) {
        for (DaiSlabCoord x = -r; x <= r; x++) m_offsets.push_back(DaiSlabXY{ x, (DaiSlabCoord)-r });
        for (DaiSlabCoord y = (DaiSlabCoord)(-r + 1); y <= r; y++) m_offsets.push_back(DaiSlabXY{ r, y });
        for (DaiSlabCoord x = (DaiSlabCoord)(r - 1); x >= -r; x--) m_offsets.push_back(DaiSlabXY{ x, r });
        for (DaiSlabCoord y = (DaiSlabCoord)(r - 1); y > -r; y--) m_offsets.push_back(DaiSlabXY{ (DaiSlabCoord)-r, y });
    }
    // The shapes: near-square rectangles of at least the slabs wanted, wasting less than a row, no longer than twice
    // their width; smallest first, then squarest.
    m_shapes.clear();
    const int64_t want = std::clamp<int64_t>(m_q.slabs_want, 1, (int64_t)SITE_MAX_SIDE * SITE_MAX_SIDE);
    for (int32_t w = 1; w <= SITE_MAX_SIDE; w++) {
        for (int32_t h = 1; h <= SITE_MAX_SIDE; h++) {
            const int64_t area = (int64_t)w * h;
            if ((area < want) || (area > want + std::max(w, h) - 1) || (std::max(w, h) > 2 * std::min(w, h))) continue;
            m_shapes.push_back(Shape{ w, h });
        }
    }
    if (m_shapes.empty()) m_shapes.push_back(Shape{ SITE_MAX_SIDE, SITE_MAX_SIDE });
    std::stable_sort(m_shapes.begin(), m_shapes.end(), [](const Shape &a, const Shape &b) {
        const int64_t aa = (int64_t)a.w * a.h, ba = (int64_t)b.w * b.h;
        if (aa != ba) return aa < ba;
        return std::abs(a.w - a.h) < std::abs(b.w - b.h);
    });
}

int64_t DaiLayoutSearch::candidates() const
{
    if (m_q.exact) return 1;
    return (m_q.grow_record != 0) ? 4 * GROW_MAX_DEPTH : (int64_t)(m_offsets.size() * m_shapes.size());
}

bool DaiLayoutSearch::candidate(int64_t n, DaiRect *r) const
{
    if (m_q.exact) {
        *r = m_q.exact_rect;
        return n == 0;
    }
    if (m_q.grow_record != 0) {
        // Strips along each side of the room's box, shallow ones first: N, E, S, W.
        const DaiSlabCoord d = (DaiSlabCoord)(n / 4 + 1);
        const DaiRect &b = m_room_box;
        switch (n % 4) {
        case 0: *r = DaiRect{ b.x0, (DaiSlabCoord)(b.y0 - d), b.x1, (DaiSlabCoord)(b.y0 - 1) }; break;
        case 1: *r = DaiRect{ (DaiSlabCoord)(b.x1 + 1), b.y0, (DaiSlabCoord)(b.x1 + d), b.y1 }; break;
        case 2: *r = DaiRect{ b.x0, (DaiSlabCoord)(b.y1 + 1), b.x1, (DaiSlabCoord)(b.y1 + d) }; break;
        default: *r = DaiRect{ (DaiSlabCoord)(b.x0 - d), b.y0, (DaiSlabCoord)(b.x0 - 1), b.y1 }; break;
        }
        return true;
    }
    const DaiSlabXY o = m_offsets[(size_t)(n / (int64_t)m_shapes.size())];
    const Shape s = m_shapes[(size_t)(n % (int64_t)m_shapes.size())];
    const DaiSlabCoord cx = (DaiSlabCoord)(m_q.ax + o.x), cy = (DaiSlabCoord)(m_q.ay + o.y);
    r->x0 = (DaiSlabCoord)(cx - (s.w - 1) / 2);
    r->y0 = (DaiSlabCoord)(cy - (s.h - 1) / 2);
    r->x1 = (DaiSlabCoord)(r->x0 + s.w - 1);
    r->y1 = (DaiSlabCoord)(r->y0 + s.h - 1);
    return true;
}

bool DaiLayoutSearch::inside(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y, const DaiRect &r) const
{
    if (in_rect(r, x, y)) return true;
    if ((m_q.grow_record == 0) || !in_rect(m_room_box, x, y)) return false;
    const DaiSlabInfo s = k.SlabAt(x, y);
    return (s.terrain == DaiTerrain::Room) && (s.owner == m_player) && (s.room == m_q.kind);
}

int32_t DaiLayoutSearch::predict(const DaiKnowledge &k, const DaiRect &r, DaiSlabCoord ent_x, DaiSlabCoord ent_y) const
{
    DaiRect box = r;
    if (m_q.grow_record != 0) {
        box.x0 = std::min(box.x0, m_room_box.x0); box.y0 = std::min(box.y0, m_room_box.y0);
        box.x1 = std::max(box.x1, m_room_box.x1); box.y1 = std::max(box.y1, m_room_box.y1);
    }
    return predict_in(k, m_player, [&](DaiSlabCoord x, DaiSlabCoord y) { return inside(k, x, y, r); }, box, ent_x, ent_y, m_q.door);
}

DaiSite DaiLayoutSearch::Weigh(const DaiKnowledge &k, const DaiRect &r, DaiSiteTerms *terms) const
{
    DaiSite site;
    DaiSiteTerms t;
    const bool grow = (m_q.grow_record != 0);
    if (!on_map(k, r.x0, r.y0) || !on_map(k, r.x1, r.y1)) return site;
    for (const DaiRect &b : m_blocked)
        if ((r.x0 <= b.x1) && (r.x1 >= b.x0) && (r.y0 <= b.y1) && (r.y1 >= b.y0)) return site;
    // Every slab becomes the keeper's floor: its own or unclaimed floor, earth, its own fortified wall (dug again), or
    // ground not yet seen.
    int64_t dig = 0;
    for (DaiSlabCoord y = r.y0; y <= r.y1; y++) {
        for (DaiSlabCoord x = r.x0; x <= r.x1; x++) {
            if (k.Kept(x, y)) return site;          // the perimeter's (plan 13 section 32): never a room's
            const DaiSlabInfo s = k.SlabAt(x, y);
            if ((s.terrain == DaiTerrain::Floor) && (s.owner == m_player)) continue;
            const DaiDigCosts &c = m_costs;
            if ((s.terrain == DaiTerrain::Floor) && (s.owner < 0)) dig += c.floor;
            else if (s.terrain == DaiTerrain::Dirt) dig += c.earth;
            else if ((s.terrain == DaiTerrain::Wall) && (s.owner == m_player) && (c.own_wall < DAI_DIG_NEVER)) dig += c.own_wall;
            else if (s.terrain == DaiTerrain::Unknown) dig += c.unknown;
            else return site;
        }
    }
    // The rim: no other room in it (a growing room's own slabs apart); valuables in it are walled in.
    const DaiRect rim{ (DaiSlabCoord)(r.x0 - m_rim), (DaiSlabCoord)(r.y0 - m_rim), (DaiSlabCoord)(r.x1 + m_rim), (DaiSlabCoord)(r.y1 + m_rim) };
    for (DaiSlabCoord y = rim.y0; y <= rim.y1; y++) {
        for (DaiSlabCoord x = rim.x0; x <= rim.x1; x++) {
            if (in_rect(r, x, y) || !on_map(k, x, y)) continue;
            const DaiSlabInfo s = k.SlabAt(x, y);
            if ((s.terrain == DaiTerrain::Room) && !inside(k, x, y, r)) return site;
            if (valuable(s.terrain)) t.waste++;
        }
    }
    // The entrance: the slab beside a side (not a corner) the flood reaches cheapest; first found on a tie.
    DaiSlabCoord ex = -1, ey = -1;
    int64_t connect = 0;
    if (!grow) {
        int32_t best = DAI_DIG_NEVER;
        auto consider = [&](DaiSlabCoord x, DaiSlabCoord y) {
            if (!on_map(k, x, y) || k.Kept(x, y)) return;
            const DaiSlabInfo s = k.SlabAt(x, y);
            const bool open = (s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Dirt) || (s.terrain == DaiTerrain::Unknown)
                || ((s.terrain == DaiTerrain::Wall) && (s.owner == m_player) && (m_costs.own_wall < DAI_DIG_NEVER));
            if (!open || ((s.terrain == DaiTerrain::Floor) && (s.owner >= 0) && (s.owner != m_player))) return;
            const int32_t c = m_flood.CostAt(x, y);
            if (c < best) { best = c; ex = x; ey = y; }
        };
        for (DaiSlabCoord x = r.x0; x <= r.x1; x++) consider(x, (DaiSlabCoord)(r.y0 - 1));
        for (DaiSlabCoord y = r.y0; y <= r.y1; y++) consider((DaiSlabCoord)(r.x1 + 1), y);
        for (DaiSlabCoord x = r.x0; x <= r.x1; x++) consider(x, (DaiSlabCoord)(r.y1 + 1));
        for (DaiSlabCoord y = r.y0; y <= r.y1; y++) consider((DaiSlabCoord)(r.x0 - 1), y);
        if (best >= DAI_DIG_NEVER) return site;
        connect = best;
    }
    // A grown room keeps a way in: the keeper's floor or door beside it, not all covered by the strip.
    if (grow) {
        DaiRect box = r;
        box.x0 = std::min(box.x0, m_room_box.x0); box.y0 = std::min(box.y0, m_room_box.y0);
        box.x1 = std::max(box.x1, m_room_box.x1); box.y1 = std::max(box.y1, m_room_box.y1);
        bool way_in = false;
        for (DaiSlabCoord y = (DaiSlabCoord)(box.y0 - 1); (y <= box.y1 + 1) && !way_in; y++) {
            for (DaiSlabCoord x = (DaiSlabCoord)(box.x0 - 1); (x <= box.x1 + 1) && !way_in; x++) {
                if (!on_map(k, x, y) || inside(k, x, y, r)) continue;
                const bool beside = inside(k, (DaiSlabCoord)(x + 1), y, r) || inside(k, (DaiSlabCoord)(x - 1), y, r)
                    || inside(k, x, (DaiSlabCoord)(y + 1), r) || inside(k, x, (DaiSlabCoord)(y - 1), r);
                const DaiSlabInfo s = k.SlabAt(x, y);
                way_in = beside && (s.owner == m_player) && ((s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Door));
            }
        }
        if (!way_in) return site;
    }
    // The terms.
    const int32_t eff = predict(k, r, ex, ey);
    t.eff_pct = (int64_t)eff * 100 / 256;
    t.dig = dig + connect;
    if (!grow) {
        const DaiSlabCoord cx = (DaiSlabCoord)((r.x0 + r.x1) / 2), cy = (DaiSlabCoord)((r.y0 + r.y1) / 2);
        t.dist = std::abs(cx - m_q.ax) + std::abs(cy - m_q.ay);
    }
    // Beyond the rim: an anchor room beside it, and earth to grow into (the two best sides).
    const DaiSlabCoord out = (DaiSlabCoord)(m_rim + 1);
    int64_t side_earth[4] = { 0, 0, 0, 0 };
    auto beyond = [&](DaiSlabCoord x, DaiSlabCoord y, int side) {
        if (!on_map(k, x, y)) return;
        const DaiSlabInfo s = k.SlabAt(x, y);
        if ((m_q.anchor_kind != 0) && (s.terrain == DaiTerrain::Room) && (s.owner == m_player) && (s.room == m_q.anchor_kind)) t.adj = 100;
        if ((s.terrain == DaiTerrain::Dirt) || (s.terrain == DaiTerrain::Unknown)) side_earth[side]++;
    };
    for (DaiSlabCoord x = r.x0; x <= r.x1; x++) { beyond(x, (DaiSlabCoord)(r.y0 - out), 0); beyond(x, (DaiSlabCoord)(r.y1 + out), 2); }
    for (DaiSlabCoord y = r.y0; y <= r.y1; y++) { beyond((DaiSlabCoord)(r.x1 + out), y, 1); beyond((DaiSlabCoord)(r.x0 - out), y, 3); }
    std::sort(side_earth, side_earth + 4);
    t.grow = side_earth[3] + side_earth[2];
    if (m_w.exp > 0) {
        for (DaiSlabCoord y = (DaiSlabCoord)(r.y0 - EXPOSURE_RANGE); y <= r.y1 + EXPOSURE_RANGE; y++) {
            for (DaiSlabCoord x = (DaiSlabCoord)(r.x0 - EXPOSURE_RANGE); x <= r.x1 + EXPOSURE_RANGE; x++) {
                if (!on_map(k, x, y)) continue;
                const PlayerNumber o = k.SlabAt(x, y).owner;
                if ((o >= 0) && (o != m_player)) t.exp++;
            }
        }
    }
    const int64_t score = t.eff_pct * m_w.eff + t.adj * m_w.adj - t.dist * m_w.dist - t.dig * m_w.dig - t.exp * m_w.exp
        + t.grow * m_w.grow - t.waste * m_w.waste;
    site.found = true;
    site.r = r;
    site.ent_x = ex;
    site.ent_y = ey;
    site.score = score / DAI_FIX_ONE;
    site.eff_pred = eff;
    site.dig_turns = (int32_t)std::min<int64_t>(t.dig, INT32_MAX);
    if (terms != nullptr) *terms = t;
    return site;
}

DaiResult DaiLayoutSearch::Step(const DaiKnowledge &k, DaiBudget &b)
{
    if (m_state != DaiResult::Wait) return m_state;
    if (m_q.grow_record == 0) {
        const DaiResult f = m_flood.Step(k, b);
        if (f == DaiResult::Wait) return DaiResult::Wait;
        if (f != DaiResult::Done) {
            m_state = DaiResult::Fail;
            return m_state;
        }
    }
    const int64_t total = candidates();
    while ((b.layout_candidates > 0) && (m_next < total)) {
        DaiRect r;
        const bool ok = candidate(m_next++, &r);
        b.layout_candidates--;
        if (!ok) continue;
        // A strip counts if it adds the slabs wanted, or is the deepest on its side.
        const int64_t area = (int64_t)(r.x1 - r.x0 + 1) * (r.y1 - r.y0 + 1);
        if ((m_q.grow_record != 0) && (area < m_q.slabs_want) && ((m_next - 1) / 4 + 1 < GROW_MAX_DEPTH)) continue;
        const DaiSite s = Weigh(k, r);
        if (!s.found) continue;
        m_feasible++;
        if (!m_best.found || (s.score > m_best.score)) m_best = s;
    }
    if (m_next >= total) m_state = m_best.found ? DaiResult::Done : DaiResult::Fail;
    return m_state;
}

bool DaiLayoutSearch::StillFeasible(const DaiKnowledge &k) const
{
    return m_best.found && Weigh(k, m_best.r).found;
}

void DaiLayoutSearch::Save(DaiBlobWriter &w) const
{
    w.I64(m_player);
    w.I64(m_q.kind); w.I64(m_q.slabs_want); w.I64(m_q.ax); w.I64(m_q.ay); w.I64(m_q.anchor_kind); w.Bool(m_q.door);
    w.U32(m_q.grow_record);
    w.I64(m_w.eff); w.I64(m_w.adj); w.I64(m_w.dist); w.I64(m_w.dig); w.I64(m_w.exp); w.I64(m_w.grow); w.I64(m_w.waste);
    w.I64(m_costs.floor); w.I64(m_costs.earth); w.I64(m_costs.unknown); w.I64(m_costs.own_wall);
    w.I64(m_radius);
    w.I64(m_rim);
    w.I64((int64_t)m_blocked.size());
    for (const DaiRect &b : m_blocked) { w.I64(b.x0); w.I64(b.y0); w.I64(b.x1); w.I64(b.y1); }
    m_flood.Save(w);
    w.I64(m_room_box.x0); w.I64(m_room_box.y0); w.I64(m_room_box.x1); w.I64(m_room_box.y1);
    w.I64(m_next);
    w.I64(m_feasible);
    w.Bool(m_best.found);
    w.I64(m_best.r.x0); w.I64(m_best.r.y0); w.I64(m_best.r.x1); w.I64(m_best.r.y1);
    w.I64(m_best.ent_x); w.I64(m_best.ent_y); w.I64(m_best.score); w.I64(m_best.eff_pred); w.I64(m_best.dig_turns);
    w.I64((int64_t)m_state);
}

bool DaiLayoutSearch::Load(DaiBlobReader &r)
{
    auto coord = [&r]() { return (DaiSlabCoord)r.Range(-300, 300); };
    m_player = (PlayerNumber)r.Range(-1, 255);
    m_q.kind = (RoomKind)r.Range(0, 255);
    m_q.slabs_want = r.Range(1, 100000);
    m_q.ax = coord(); m_q.ay = coord();
    m_q.anchor_kind = (RoomKind)r.Range(0, 255);
    m_q.door = r.Bool();
    m_q.grow_record = r.U32();
    m_w.eff = r.I64(); m_w.adj = r.I64(); m_w.dist = r.I64(); m_w.dig = r.I64(); m_w.exp = r.I64(); m_w.grow = r.I64();
    m_w.waste = r.I64();
    m_costs = DaiDigCosts();
    m_costs.floor = (int32_t)r.Range(1, 10000); m_costs.earth = (int32_t)r.Range(1, 10000); m_costs.unknown = (int32_t)r.Range(1, 10000);
    m_costs.own_wall = (int32_t)r.Range(1, DAI_DIG_NEVER);
    m_radius = r.Range(0, 64);
    m_rim = r.Range(1, 2);
    const int64_t nb = r.Count(1024);
    m_blocked.clear();
    for (int64_t i = 0; (i < nb) && r.Ok(); i++) {
        DaiRect b;
        b.x0 = coord(); b.y0 = coord(); b.x1 = coord(); b.y1 = coord();
        m_blocked.push_back(b);
    }
    if (r.Ok() && !m_flood.Load(r)) r.Fail("layout flood");
    m_room_box.x0 = coord(); m_room_box.y0 = coord(); m_room_box.x1 = coord(); m_room_box.y1 = coord();
    prepare();
    m_next = r.Range(0, candidates());
    m_feasible = r.Range(0, m_next);
    m_best = DaiSite();
    m_best.found = r.Bool();
    m_best.r.x0 = coord(); m_best.r.y0 = coord(); m_best.r.x1 = coord(); m_best.r.y1 = coord();
    m_best.ent_x = coord(); m_best.ent_y = coord();
    m_best.score = r.I64();
    m_best.eff_pred = (int32_t)r.Range(0, 256);
    m_best.dig_turns = (int32_t)r.Range(0, INT32_MAX);
    m_state = (DaiResult)r.Range(0, (int64_t)DaiResult::BadInput);
    if (!r.Ok()) {
        *this = DaiLayoutSearch();
        return false;
    }
    return true;
}
