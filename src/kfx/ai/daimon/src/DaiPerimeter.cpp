#include "pre_inc.h"
#include "daimon/DaiPerimeter.h"
#include "daimon/DaiGeom.h"

#include <algorithm>

#include "daimon/DaiFortify.h"
#include "daimon/DaiRivals.h"
#include "post_inc.h"

namespace {
/** What blocks a way for good, as every keeper sees it: rock (all shown from the start), gems (never dug through),
 *  and lava, abyss and impassable ground once revealed. */
bool passable(DaiTerrain t)
{
    switch (t) {
    case DaiTerrain::Rock: case DaiTerrain::Gem: case DaiTerrain::Lava: case DaiTerrain::Abyss: case DaiTerrain::Impassable:
        return false;
    default:
        return true;
    }
}

constexpr int32_t INF = INT32_MAX / 4;

/** A flow network on the slabs, each split in two (in, out) joined by an edge of capacity 1: a cut of those edges
 *  is a cut of slabs. */
struct Net {
    std::vector<int32_t> head, next, to, cap;
    explicit Net(size_t nodes) : head(nodes, -1) {}
    void add(int32_t a, int32_t b, int32_t c)
    {
        to.push_back(b); cap.push_back(c); next.push_back(head[(size_t)a]); head[(size_t)a] = (int32_t)to.size() - 1;
        to.push_back(a); cap.push_back(0); next.push_back(head[(size_t)b]); head[(size_t)b] = (int32_t)to.size() - 1;
    }
};
}

bool DaiPerimeter::MinCut(const DaiKnowledge &k, DaiSlabCoord hx, DaiSlabCoord hy, int64_t inner_area, int64_t radius,
                          int64_t max_cut, std::vector<DaiSlabXY> *cut, int64_t *enclosed, int64_t *outside, int64_t max_cost,
                          int64_t *cost)
{
    cut->clear();
    *enclosed = 0;
    if (outside != nullptr) *outside = 0;
    const DaiSlabCoord w = k.Width(), h = k.Height();
    if ((hx < 0) || (hy < 0) || (hx >= w) || (hy >= h)) return false;
    const int32_t n = (int32_t)w * (int32_t)h;
    std::vector<uint8_t> open((size_t)n, 0);
    for (int32_t i = 0; i < n; i++) open[(size_t)i] = passable(k.SlabAt(i % w, i / w).terrain) ? 1 : 0;
    const int32_t heart = (int32_t)hy * w + hx;
    open[(size_t)heart] = 1;
    // The route from the heart, in slabs: the sink is every slab `radius` or more away.
    std::vector<int32_t> dist((size_t)n, -1), queue;
    queue.reserve((size_t)n);
    dist[(size_t)heart] = 0;
    queue.push_back(heart);
    auto neighbours = [&](int32_t at, int32_t out[4]) {
        const DaiSlabCoord x = at % w, y = at / w;
        out[0] = (y > 0) ? at - (int32_t)w : -1;
        out[1] = (x + 1 < w) ? at + 1 : -1;
        out[2] = (y + 1 < h) ? at + (int32_t)w : -1;
        out[3] = (x > 0) ? at - 1 : -1;
    };
    bool far = false;
    for (size_t q = 0; q < queue.size(); q++) {
        int32_t nb[4];
        neighbours(queue[q], nb);
        for (int32_t b : nb) {
            if ((b < 0) || !open[(size_t)b] || (dist[(size_t)b] >= 0)) continue;
            dist[(size_t)b] = dist[(size_t)queue[q]] + 1;
            far = far || (dist[(size_t)b] >= radius);
            queue.push_back(b);
        }
    }
    if (!far) return false;                 // nothing that far: the heart's ground closed already
    // The inner ground: out to the route length at which inner_area slabs are reached (breadth first: in order).
    const int32_t inner = dist[(size_t)queue[(size_t)std::clamp<int64_t>(inner_area, 1, (int64_t)queue.size()) - 1]];
    if (radius <= inner + 1) return false;
    // The network, on the slabs nearer than `radius` only (the rest is the sink): in(v) = 2 id(v), out(v) = 2 id(v) + 1.
    std::vector<int32_t> id((size_t)n, -1);
    int32_t m = 0;
    for (int32_t v : queue) if (dist[(size_t)v] < radius) id[(size_t)v] = m++;
    Net net((size_t)2 * m + 1);
    const int32_t sink = 2 * m;
    for (int32_t v : queue) {
        if (id[(size_t)v] < 0) continue;
        const int32_t iv = id[(size_t)v];
        // A slab's price: its open sides. Of cuts as small, the one between rock -- a gap's own slabs, not the open
        // ground just inside or outside its mouth -- where walls and a door hold.
        int32_t nb[4];
        neighbours(v, nb);
        int32_t sides = 0;
        for (int32_t b : nb) sides += ((b >= 0) && (dist[(size_t)b] >= 0)) ? 1 : 0;
        net.add(2 * iv, 2 * iv + 1, (dist[(size_t)v] <= inner) ? INF : std::max<int32_t>(sides, 1));
        bool to_sink = false;
        for (int32_t b : nb) {
            if ((b < 0) || (dist[(size_t)b] < 0)) continue;
            if (id[(size_t)b] >= 0) net.add(2 * iv + 1, 2 * id[(size_t)b], INF);
            else to_sink = true;
        }
        if (to_sink) net.add(2 * iv + 1, sink, INF);
    }
    // Augmenting paths, breadth first, each by its narrowest edge; dearer than max_cost (or than max_cut slabs could
    // be, four sides each): no use.
    const int64_t limit = std::min<int64_t>(max_cost, 4 * max_cut);
    const int32_t source = 2 * id[(size_t)heart] + 1;
    std::vector<int32_t> via((size_t)2 * m + 1);
    std::vector<int32_t> bfs;
    bfs.reserve((size_t)2 * m + 1);
    int64_t flow = 0;
    for (;;) {
        std::fill(via.begin(), via.end(), -2);
        via[(size_t)source] = -1;
        bfs.clear();
        bfs.push_back(source);
        for (size_t q = 0; (q < bfs.size()) && (via[(size_t)sink] == -2); q++) {
            const int32_t a = bfs[q];
            for (int32_t e = net.head[(size_t)a]; e >= 0; e = net.next[(size_t)e]) {
                const int32_t b = net.to[(size_t)e];
                if ((net.cap[(size_t)e] <= 0) || (via[(size_t)b] != -2)) continue;
                via[(size_t)b] = e;
                bfs.push_back(b);
            }
        }
        if (via[(size_t)sink] == -2) break;
        int32_t push = INF;
        for (int32_t b = sink; b != source; b = net.to[(size_t)(via[(size_t)b] ^ 1)]) push = std::min(push, net.cap[(size_t)via[(size_t)b]]);
        for (int32_t b = sink; b != source; b = net.to[(size_t)(via[(size_t)b] ^ 1)]) {
            net.cap[(size_t)via[(size_t)b]] -= push;
            net.cap[(size_t)(via[(size_t)b] ^ 1)] += push;
        }
        flow += push;
        if (flow > limit) return false;
    }
    if (cost != nullptr) *cost = flow;
    // The heart's side: what the last search reached; a slab whose in is on it and whose out is not is cut. Of the cuts
    // as cheap, the nearest the heart: with the slabs priced by their open sides, a gap's inner slabs (level 61: the
    // ring's inner column, not the open ground at the gap's mouth outside, which costs as much).
    for (int32_t v : queue) {
        const int32_t iv = id[(size_t)v];
        if (iv < 0) continue;
        const bool in = (via[(size_t)(2 * iv)] != -2) || (dist[(size_t)v] <= inner), out = (via[(size_t)(2 * iv + 1)] != -2);
        if (in) (*enclosed)++;
        if (in && !out) cut->push_back(DaiSlabXY{ (DaiSlabCoord)(v % w), (DaiSlabCoord)(v / w) });
    }
    if (outside != nullptr) *outside = (int64_t)queue.size() - *enclosed;
    std::sort(cut->begin(), cut->end(), [](const DaiSlabXY &a, const DaiSlabXY &b) { return (a.y != b.y) ? (a.y < b.y) : (a.x < b.x); });
    return (int64_t)cut->size() <= max_cut;
}

uint64_t DaiPerimeter::signature(const DaiKnowledge &k)
{
    uint64_t s = 1469598103934665603ull;
    const DaiSlabCoord w = k.Width(), h = k.Height();
    for (DaiSlabCoord y = 0; y < h; y++)
        for (DaiSlabCoord x = 0; x < w; x++)
            if (!passable(k.SlabAt(x, y).terrain)) s = (s ^ (uint64_t)((int64_t)y * w + x)) * 1099511628211ull;
    return s;
}

void DaiPerimeter::Update(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now)
{
    // Not before the map has been read once: unread ground is Unknown, passable -- no gap would be found, and the
    // next look would come only DAI_PERIMETER_INTERVAL turns later (level 61: the early digs opened a gap meanwhile).
    if (!k.Own().has_heart || (k.ScanCycles() < 1)) return;
    if (m_scan_r > 0) {                 // a radius a planning turn
        step(k, p);
        if (m_scan_r > DAI_PERIMETER_RADIUS_MAX) finish(k, now);
        return;
    }
    if (m_turn != 0) {
        if (now < m_turn + (DaiTurn)DAI_PERIMETER_INTERVAL) return;
        if (signature(k) == m_signature) {
            m_turn = now;               // looked at: nothing that blocks has changed
            return;
        }
    }
    start(k, p);
    step(k, p);
    if (m_scan_r > DAI_PERIMETER_RADIUS_MAX) finish(k, now);
}

void DaiPerimeter::Compute(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now)
{
    start(k, p);
    while (m_scan_r <= DAI_PERIMETER_RADIUS_MAX) step(k, p);
    finish(k, now);
}

void DaiPerimeter::start(const DaiKnowledge &k, const DaiPersonality &p)
{
    m_scan_sig = signature(k);
    m_scan_best.clear();
    m_scan_cost = m_scan_enclosed = m_scan_radius = 0;
    // Off, or no heart: nothing to scan (finish publishes none).
    m_scan_r = (k.Own().has_heart && (p.defence_perimeter_max_cut > 0)) ? DAI_PERIMETER_RADIUS_STEP : DAI_PERIMETER_RADIUS_MAX + 1;
}

void DaiPerimeter::step(const DaiKnowledge &k, const DaiPersonality &p)
{
    if ((m_scan_r <= 0) || (m_scan_r > DAI_PERIMETER_RADIUS_MAX)) return;
    const int64_t r = m_scan_r;
    m_scan_r += DAI_PERIMETER_RADIUS_STEP;
    const DaiSlabCoord hx = DaiSubtileSlab(k.Own().heart_x), hy = DaiSubtileSlab(k.Own().heart_y);
    std::vector<DaiSlabXY> cut;
    int64_t enclosed = 0, outside = 0, cost = 0;
    const int64_t limit = m_scan_best.empty() ? INT64_MAX : m_scan_cost;      // as cheap as the best, or cheaper
    if (!MinCut(k, hx, hy, p.defence_perimeter_min_area / 2, r, p.defence_perimeter_max_cut, &cut, &enclosed, &outside, limit, &cost)
        || cut.empty())
        return;
    // Enough inside, and the rest of the map outside -- not a corner of it (an open map's far corners cut off).
    if ((enclosed < p.defence_perimeter_min_area) || (outside < enclosed)) return;
    // The fewest slabs; as few, the more ground inside.
    if (m_scan_best.empty() || (cut.size() < m_scan_best.size()) || ((cut.size() == m_scan_best.size()) && (enclosed > m_scan_enclosed))) {
        m_scan_best = cut;
        m_scan_cost = cost;
        m_scan_enclosed = enclosed;
        m_scan_radius = r;
    }
}

void DaiPerimeter::finish(const DaiKnowledge &k, DaiTurn now)
{
    m_scan_r = 0;
    m_turn = std::max<DaiTurn>(now, 1);
    m_signature = m_scan_sig;
    m_gaps.clear();
    m_inside.clear();
    m_enclosed = 0;
    m_radius = 0;
    const std::vector<DaiSlabXY> best = m_scan_best;
    m_scan_best.clear();
    if (best.empty() || !k.Own().has_heart) return;
    const DaiSlabCoord hx = DaiSubtileSlab(k.Own().heart_x), hy = DaiSubtileSlab(k.Own().heart_y);
    m_radius = m_scan_radius;
    m_enclosed = m_scan_enclosed;

    // The gaps: the cut's slabs touching (eight ways), each with its middle slab its gate.
    std::vector<uint8_t> taken(best.size(), 0);
    for (size_t i = 0; i < best.size(); i++) {
        if (taken[i]) continue;
        Gap g;
        std::vector<size_t> stack{ i };
        taken[i] = 1;
        while (!stack.empty()) {
            const size_t a = stack.back();
            stack.pop_back();
            g.slabs.push_back(best[a]);
            for (size_t b = 0; b < best.size(); b++) {
                if (taken[b] || (std::abs((int)best[a].x - (int)best[b].x) > 1) || (std::abs((int)best[a].y - (int)best[b].y) > 1)) continue;
                taken[b] = 1;
                stack.push_back(b);
            }
        }
        m_gaps.push_back(g);
    }
    straighten(k, hx, hy);
    auto row_major = [](const DaiSlabXY &a, const DaiSlabXY &b) { return (a.y != b.y) ? (a.y < b.y) : (a.x < b.x); };
    for (Gap &g : m_gaps) {
        std::sort(g.slabs.begin(), g.slabs.end(), row_major);
        g.gate = g.slabs[g.slabs.size() / 2];
    }
    // The lining: each gap slab's sides inside (the heart reaches them with the cut closed), open ground or diggable.
    m_inside.clear();
    const std::vector<uint8_t> &in = inside(k);
    const DaiSlabCoord w = k.Width(), h = k.Height();
    for (Gap &g : m_gaps) {
        for (const DaiSlabXY &s : g.slabs) {
            const DaiSlabXY nb[4] = { { s.x, (DaiSlabCoord)(s.y - 1) }, { (DaiSlabCoord)(s.x + 1), s.y }, { s.x, (DaiSlabCoord)(s.y + 1) }, { (DaiSlabCoord)(s.x - 1), s.y } };
            for (const DaiSlabXY &b : nb) {
                if ((b.x < 0) || (b.y < 0) || (b.x >= w) || (b.y >= h) || !in[(size_t)b.y * w + b.x]) continue;
                bool listed = false;
                for (const DaiSlabXY &l : g.lining) listed = listed || ((l.x == b.x) && (l.y == b.y));
                if (!listed) g.lining.push_back(b);
            }
        }
        std::sort(g.lining.begin(), g.lining.end(), row_major);
    }
}

const std::vector<uint8_t> &DaiPerimeter::inside(const DaiKnowledge &k) const
{
    const DaiSlabCoord w = k.Width(), h = k.Height();
    if (!m_inside.empty() || m_gaps.empty() || !k.Own().has_heart) return m_inside;
    const size_t n = (size_t)w * (size_t)h;
    std::vector<uint8_t> cut(n, 0);
    for (const Gap &g : m_gaps) for (const DaiSlabXY &s : g.slabs) cut[(size_t)s.y * w + s.x] = 1;
    m_inside.assign(n, 0);
    const DaiSlabCoord hx = DaiSubtileSlab(k.Own().heart_x), hy = DaiSubtileSlab(k.Own().heart_y);
    if ((hx < 0) || (hy < 0) || (hx >= w) || (hy >= h)) return m_inside;
    std::vector<int32_t> q{ (int32_t)((int64_t)hy * w + hx) };
    m_inside[(size_t)q[0]] = 1;
    for (size_t i = 0; i < q.size(); i++) {
        const DaiSlabCoord x = q[i] % w, y = q[i] / w;
        const DaiSlabXY nb[4] = { { x, (DaiSlabCoord)(y - 1) }, { (DaiSlabCoord)(x + 1), y }, { x, (DaiSlabCoord)(y + 1) }, { (DaiSlabCoord)(x - 1), y } };
        for (const DaiSlabXY &b : nb) {
            if ((b.x < 0) || (b.y < 0) || (b.x >= w) || (b.y >= h)) continue;
            const size_t at = (size_t)b.y * w + b.x;
            if (m_inside[at] || cut[at] || !passable(k.SlabAt(b.x, b.y).terrain)) continue;
            m_inside[at] = 1;
            q.push_back((int32_t)at);
        }
    }
    return m_inside;
}

int64_t DaiPerimeter::GoldInside(const DaiKnowledge &k) const
{
    const std::vector<uint8_t> &in = inside(k);
    if (in.empty()) return 0;
    const DaiSlabCoord w = k.Width(), h = k.Height();
    auto inside_at = [&](DaiSlabCoord x, DaiSlabCoord y) { return (x >= 0) && (y >= 0) && (x < w) && (y < h) && in[(size_t)y * w + x]; };
    int64_t gold = 0;
    bool gems = false;
    k.ForEachVein([&](const DaiVein &v) {
        if (inside_at(v.cx, v.cy)) gold += v.gold_left;
        // A gem face the inside touches (level 61: two on the ring's lower-left diagonal): gold without end.
        if (v.gem_slabs > 0)
            for (DaiSlabCoord y = v.box.y0; (y <= v.box.y1) && !gems; y++)
                for (DaiSlabCoord x = v.box.x0; (x <= v.box.x1) && !gems; x++)
                    if (k.SlabAt(x, y).terrain == DaiTerrain::Gem)
                        gems = inside_at(x, (DaiSlabCoord)(y - 1)) || inside_at((DaiSlabCoord)(x + 1), y) || inside_at(x, (DaiSlabCoord)(y + 1))
                            || inside_at((DaiSlabCoord)(x - 1), y);
    });
    return gems ? INT64_MAX / 4 : gold;
}

void DaiPerimeter::straighten(const DaiKnowledge &k, DaiSlabCoord hx, DaiSlabCoord hy)
{
    // Cuts as cheap may bulge a slab in or out of a gap's line (its middle slab is as open in either place), and a
    // door wants closed slabs on two opposite sides. Each gap not straight: the straight line of as many slabs, rock at
    // both its ends, near it, that closes the heart off as well (the slabs `radius` away out of reach) takes its place.
    const DaiSlabCoord w = k.Width(), h = k.Height();
    const int32_t n = (int32_t)w * (int32_t)h;
    auto open_at = [&](DaiSlabCoord x, DaiSlabCoord y) { return (x >= 0) && (y >= 0) && (x < w) && (y < h) && passable(k.SlabAt(x, y).terrain); };
    auto routes = [&](const std::vector<uint8_t> &blocked, std::vector<int32_t> *d) {
        d->assign((size_t)n, -1);
        std::vector<int32_t> q{ (int32_t)((int64_t)hy * w + hx) };
        (*d)[(size_t)q[0]] = 0;
        for (size_t i = 0; i < q.size(); i++) {
            const DaiSlabCoord x = q[i] % w, y = q[i] / w;
            const DaiSlabXY nb[4] = { { x, (DaiSlabCoord)(y - 1) }, { (DaiSlabCoord)(x + 1), y }, { x, (DaiSlabCoord)(y + 1) }, { (DaiSlabCoord)(x - 1), y } };
            for (const DaiSlabXY &s : nb) {
                if (!open_at(s.x, s.y)) continue;
                const int32_t at = (int32_t)s.y * w + s.x;
                if (blocked[(size_t)at] || ((*d)[(size_t)at] >= 0)) continue;
                (*d)[(size_t)at] = (*d)[(size_t)q[i]] + 1;
                q.push_back(at);
            }
        }
    };
    std::vector<int32_t> free_dist, d;
    routes(std::vector<uint8_t>((size_t)n, 0), &free_dist);
    auto closes = [&](const std::vector<uint8_t> &blocked) {
        routes(blocked, &d);
        for (int32_t i = 0; i < n; i++) if ((d[(size_t)i] >= 0) && (free_dist[(size_t)i] >= m_radius)) return false;
        return true;
    };
    for (size_t gi = 0; gi < m_gaps.size(); gi++) {
        Gap &g = m_gaps[gi];
        bool same_x = true, same_y = true;
        DaiSlabCoord x0 = g.slabs[0].x, x1 = x0, y0 = g.slabs[0].y, y1 = y0;
        for (const DaiSlabXY &s : g.slabs) {
            same_x = same_x && (s.x == g.slabs[0].x);
            same_y = same_y && (s.y == g.slabs[0].y);
            x0 = std::min(x0, s.x); x1 = std::max(x1, s.x); y0 = std::min(y0, s.y); y1 = std::max(y1, s.y);
        }
        if (same_x || same_y) continue;
        const int64_t len = (int64_t)g.slabs.size();
        std::vector<uint8_t> others((size_t)n, 0);
        for (size_t oi = 0; oi < m_gaps.size(); oi++)
            if (oi != gi) for (const DaiSlabXY &s : m_gaps[oi].slabs) others[(size_t)s.y * w + s.x] = 1;
        const int64_t cx2 = (int64_t)x0 + x1, cy2 = (int64_t)y0 + y1;      // twice its middle
        std::vector<DaiSlabXY> best;
        int64_t best_off = INT64_MAX;
        for (int vertical = 1; vertical >= 0; vertical--) {
            for (DaiSlabCoord a = (DaiSlabCoord)((vertical ? x0 : y0) - 1); a <= (vertical ? x1 : y1) + 1; a++) {
                for (DaiSlabCoord b = (DaiSlabCoord)((vertical ? y0 : x0) - 1); b + len - 1 <= (vertical ? y1 : x1) + 1; b++) {
                    auto at = [&](int64_t i) { return vertical ? DaiSlabXY{ a, (DaiSlabCoord)(b + i) } : DaiSlabXY{ (DaiSlabCoord)(b + i), a }; };
                    const DaiSlabXY before = at(-1), after = at(len);
                    if (open_at(before.x, before.y) || open_at(after.x, after.y)) continue;
                    bool ok = true;
                    for (int64_t i = 0; (i < len) && ok; i++) ok = open_at(at(i).x, at(i).y);
                    if (!ok) continue;
                    const int64_t off = std::abs(2 * (int64_t)(vertical ? a : b) + (vertical ? 0 : len - 1) - cx2)
                                      + std::abs(2 * (int64_t)(vertical ? b : a) + (vertical ? len - 1 : 0) - cy2);
                    if (off >= best_off) continue;
                    std::vector<uint8_t> blocked = others;
                    std::vector<DaiSlabXY> line;
                    for (int64_t i = 0; i < len; i++) { line.push_back(at(i)); blocked[(size_t)at(i).y * w + at(i).x] = 1; }
                    if (!closes(blocked)) continue;
                    best = line;
                    best_off = off;
                }
            }
        }
        if (!best.empty()) g.slabs = best;
    }
}

int64_t DaiPerimeter::CutSlabs() const
{
    int64_t n = 0;
    for (const Gap &g : m_gaps) n += (int64_t)g.slabs.size();
    return n;
}

namespace {
/** Dug open: no wall can be had there any more (the engine fortifies earth only) -- a door's place. */
bool dug(DaiTerrain t)
{
    return (t == DaiTerrain::Floor) || (t == DaiTerrain::Room) || (t == DaiTerrain::Door) || (t == DaiTerrain::Water);
}
}

int64_t DaiPerimeter::Closed(const DaiKnowledge &k, PlayerNumber p) const
{
    int64_t n = 0;
    for (const Gap &g : m_gaps)
        for (const DaiSlabXY &s : g.slabs) {
            const DaiSlabInfo i = k.SlabAt(s.x, s.y);
            const bool gate = m_open && (s.x == g.gate.x) && (s.y == g.gate.y);
            if (gate || dug(i.terrain)) n += ((i.terrain == DaiTerrain::Door) && (i.owner == p)) ? 1 : 0;
            else n += ((i.terrain == DaiTerrain::Rock) || (i.terrain == DaiTerrain::Gem) || ((i.terrain == DaiTerrain::Wall) && (i.owner == p))) ? 1 : 0;
        }
    return n;
}

void DaiPerimeter::Apply(DaiKnowledge &k) const
{
    // A gap slab dug open already (level 61: a rival came in by the gap's end before the keeper's lining reached it) is
    // a door's place even while closed: the gap is held by its walls and a door there (the owner: a fortified wall
    // stands while the keeper's land is beside it -- a claimed lining lost to the intruder reverts it to earth).
    std::vector<DaiSlabXY> kept, gates, lining;
    for (const Gap &g : m_gaps) {
        for (const DaiSlabXY &s : g.slabs) {
            const bool gate = (m_open && (s.x == g.gate.x) && (s.y == g.gate.y)) || dug(k.SlabAt(s.x, s.y).terrain);
            if (gate) gates.push_back(s);
            else kept.push_back(s);
        }
        lining.insert(lining.end(), g.lining.begin(), g.lining.end());
    }
    k.SetPerimeter(kept, gates, lining);
}

void DaiPerimeter::Save(DaiBlobWriter &w) const
{
    w.U64(m_turn);
    w.U64(m_signature);
    w.I64(m_enclosed);
    w.I64(m_radius);
    w.Bool(m_open);
    w.I64(m_scan_r);
    w.U64(m_scan_sig);
    w.I64(m_scan_cost);
    w.I64(m_scan_enclosed);
    w.I64(m_scan_radius);
    w.I64((int64_t)m_scan_best.size());
    for (const DaiSlabXY &c : m_scan_best) { w.I64(c.x); w.I64(c.y); }
    w.I64((int64_t)m_gaps.size());
    for (const Gap &g : m_gaps) {
        w.I64(g.gate.x);
        w.I64(g.gate.y);
        w.I64((int64_t)g.slabs.size());
        for (const DaiSlabXY &s : g.slabs) { w.I64(s.x); w.I64(s.y); }
        w.I64((int64_t)g.lining.size());
        for (const DaiSlabXY &s : g.lining) { w.I64(s.x); w.I64(s.y); }
    }
}

bool DaiPerimeter::Load(DaiBlobReader &r)
{
    *this = DaiPerimeter();
    m_turn = (DaiTurn)r.U64();
    m_signature = r.U64();
    m_enclosed = r.Range(0, INT64_MAX / 4);
    m_radius = r.Range(0, 4096);
    m_open = r.Bool();
    m_scan_r = r.Range(0, DAI_PERIMETER_RADIUS_MAX + DAI_PERIMETER_RADIUS_STEP);
    m_scan_sig = r.U64();
    m_scan_cost = r.Range(0, INT64_MAX / 4);
    m_scan_enclosed = r.Range(0, INT64_MAX / 4);
    m_scan_radius = r.Range(0, 4096);
    const int64_t sb = r.Count(4096);
    for (int64_t i = 0; (i < sb) && r.Ok(); i++) {
        DaiSlabXY c;
        c.x = (DaiSlabCoord)r.Range(0, 4096);
        c.y = (DaiSlabCoord)r.Range(0, 4096);
        m_scan_best.push_back(c);
    }
    const int64_t n = r.Count(256);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        Gap g;
        g.gate.x = (DaiSlabCoord)r.Range(0, 4096);
        g.gate.y = (DaiSlabCoord)r.Range(0, 4096);
        const int64_t m = r.Count(4096);
        for (int64_t j = 0; (j < m) && r.Ok(); j++) {
            DaiSlabXY s;
            s.x = (DaiSlabCoord)r.Range(0, 4096);
            s.y = (DaiSlabCoord)r.Range(0, 4096);
            g.slabs.push_back(s);
        }
        const int64_t l = r.Count(4096);
        for (int64_t j = 0; (j < l) && r.Ok(); j++) {
            DaiSlabXY s;
            s.x = (DaiSlabCoord)r.Range(0, 4096);
            s.y = (DaiSlabCoord)r.Range(0, 4096);
            g.lining.push_back(s);
        }
        m_gaps.push_back(g);
    }
    if (!r.Ok()) *this = DaiPerimeter();
    return r.Ok();
}

const char *DaiPerimeter::MaybeOpen(const DaiKnowledge &k, const DaiPersonality &per, const DaimonGlobalConfig &cfg,
                                    const DaiForecastOut &forecast, const DaiRivals &rivals)
{
    if (!Found() || Open() || (k.ScanCycles() < 1)) return nullptr;
    const char *why = nullptr;
    // Ready to attack, with a door to put on a gate (level 61: opened at 6,000 turns for a raid on a portal with no
    // door to hand, the gate a hole, the gap overrun).
    if (const DaiRivalTarget *best = rivals.Best())
        if (DaiRivals::Readiness(k, per, cfg, forecast, *best).Ready()
            && (DaiFortify::DoorKind(k.Stock(), true, per.defence_secret_doors != 0, &k.Content()) != 0))
            why = "ready to attack";
    if ((why == nullptr) && (GoldInside(k) <= 0)) why = "no gold left inside";
    if (why != nullptr) SetOpen();
    return why;
}
