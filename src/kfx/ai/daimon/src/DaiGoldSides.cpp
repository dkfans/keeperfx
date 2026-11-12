#include "pre_inc.h"
#include "daimon/DaiGoldSides.h"
#include "daimon/DaiGeom.h"

#include <algorithm>

#include "post_inc.h"

namespace {
bool passable(DaiTerrain t)
{
    switch (t) {
    case DaiTerrain::Rock: case DaiTerrain::Gem: case DaiTerrain::Lava: case DaiTerrain::Abyss: case DaiTerrain::Impassable:
        return false;
    default:
        return true;                // unknown ground (rock never is: the game shows all of it), earth, gold, water, floor
    }
}

bool vein_terrain(DaiTerrain t)
{
    return (t == DaiTerrain::Gold) || (t == DaiTerrain::DenseGold) || (t == DaiTerrain::Gem);
}
}

void DaiGoldSides::Routes(const DaiKnowledge &k, DaiSlabCoord hx, DaiSlabCoord hy, std::vector<int32_t> *out)
{
    const DaiSlabCoord w = k.Width(), h = k.Height();
    out->assign((size_t)w * (size_t)h, DAI_SIDES_NEVER);
    if ((hx < 0) || (hy < 0) || (hx >= w) || (hy >= h)) return;
    // Breadth first, neighbours N, E, S, W: every step a slab.
    std::vector<int32_t> queue;
    queue.reserve(out->size());
    const int32_t start = (int32_t)(hy * w + hx);
    (*out)[(size_t)start] = 0;
    queue.push_back(start);
    for (size_t head = 0; head < queue.size(); head++) {
        const int32_t at = queue[head];
        const DaiSlabCoord x = at % w, y = at / w;
        const int32_t next[4] = { (y > 0) ? at - (int32_t)w : -1, (x + 1 < w) ? at + 1 : -1, (y + 1 < h) ? at + (int32_t)w : -1,
                                  (x > 0) ? at - 1 : -1 };
        for (int32_t nb : next) {
            if ((nb < 0) || ((*out)[(size_t)nb] != DAI_SIDES_NEVER)) continue;
            if (!passable(k.SlabAt(nb % w, nb / w).terrain)) continue;
            (*out)[(size_t)nb] = (*out)[(size_t)at] + 1;
            queue.push_back(nb);
        }
    }
}

uint32_t DaiGoldSides::hearts_seen(const DaiKnowledge &k) const
{
    uint32_t bits = 0;
    for (PlayerNumber r = 0; r < (PlayerNumber)DAI_MAX_PLAYERS; r++)
        if (k.RivalKeeper(r) && k.Sighting(r).heart_seen) bits |= 1u << r;
    return bits;
}

void DaiGoldSides::Update(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now)
{
    if ((m_turn != 0) && (now < m_turn + (DaiTurn)DAI_SIDES_INTERVAL) && (hearts_seen(k) == m_hearts)) return;
    Compute(k, p, now);
}

void DaiGoldSides::Compute(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now)
{
    m_turn = std::max<DaiTurn>(now, 1);
    m_hearts = hearts_seen(k);
    m_veins.clear();
    const DaiOwnLook &own = k.Own();
    m_split = own.has_heart && (m_hearts != 0);
    if (!m_split) return;
    // Each keeper's routes: the keeper's own first.
    std::vector<std::vector<int32_t>> routes;
    routes.emplace_back();
    Routes(k, DaiSubtileSlab(own.heart_x), DaiSubtileSlab(own.heart_y), &routes.back());
    for (PlayerNumber r = 0; r < (PlayerNumber)DAI_MAX_PLAYERS; r++) {
        if ((m_hearts & (1u << r)) == 0) continue;
        const DaiRivalSighting &s = k.Sighting(r);
        routes.emplace_back();
        Routes(k, DaiSubtileSlab(s.heart_x), DaiSubtileSlab(s.heart_y), &routes.back());
    }
    const DaiSlabCoord w = k.Width(), h = k.Height();
    std::vector<int32_t> cost(routes.size());
    k.ForEachVein([&](const DaiVein &v) {
        // A keeper's route to the vein: to its nearest gold slab, or beside its nearest gem (never entered).
        for (size_t i = 0; i < routes.size(); i++) {
            const std::vector<int32_t> &g = routes[i];
            int32_t best = DAI_SIDES_NEVER;
            for (DaiSlabCoord y = std::max<DaiSlabCoord>(v.box.y0, 0); y <= std::min<DaiSlabCoord>(v.box.y1, h - 1); y++)
                for (DaiSlabCoord x = std::max<DaiSlabCoord>(v.box.x0, 0); x <= std::min<DaiSlabCoord>(v.box.x1, w - 1); x++) {
                    const DaiTerrain t = k.SlabAt(x, y).terrain;
                    if (!vein_terrain(t)) continue;
                    if (t != DaiTerrain::Gem) {
                        best = std::min(best, g[(size_t)y * w + x]);
                        continue;
                    }
                    const DaiSlabXY nb[4] = { { x, y - 1 }, { x + 1, y }, { x, y + 1 }, { x - 1, y } };
                    for (const DaiSlabXY &n : nb) {
                        if ((n.x < 0) || (n.y < 0) || (n.x >= w) || (n.y >= h)) continue;
                        const int32_t c = g[(size_t)n.y * w + n.x];
                        if (c != DAI_SIDES_NEVER) best = std::min(best, c + 1);
                    }
                }
            cost[i] = best;
        }
        Vein out;
        out.id = v.id;
        out.own_route = cost[0];
        out.best_route = *std::min_element(cost.begin(), cost.end());
        if (out.best_route != DAI_SIDES_NEVER) {
            // Every keeper within gold_contest_share % of the shortest route shares it.
            bool mine = false;
            for (size_t i = 0; i < cost.size(); i++) {
                if (cost[i] == DAI_SIDES_NEVER) continue;
                if (((int64_t)cost[i] - out.best_route) * 100 > p.economy_gold_contest_share * (int64_t)cost[i]) continue;
                out.sharers++;
                mine = mine || (i == 0);
            }
            out.share = mine ? DAI_FIX_ONE / out.sharers : 0;
        }
        m_veins.push_back(out);
    });
    std::sort(m_veins.begin(), m_veins.end(), [](const Vein &a, const Vein &b) { return a.id < b.id; });
}

int64_t DaiGoldSides::HeartsSeen() const
{
    int64_t n = 0;
    for (uint32_t b = m_hearts; b != 0; b &= b - 1) n++;
    return n;
}

const DaiGoldSides::Vein *DaiGoldSides::Find(uint32_t id) const
{
    const auto it = std::lower_bound(m_veins.begin(), m_veins.end(), id, [](const Vein &v, uint32_t i) { return v.id < i; });
    return ((it != m_veins.end()) && (it->id == id)) ? &*it : nullptr;
}

void DaiGoldSides::Save(DaiBlobWriter &w) const
{
    w.U64(m_turn);
    w.U32(m_hearts);
    w.Bool(m_split);
    w.I64((int64_t)m_veins.size());
    for (const Vein &v : m_veins) {
        w.U32(v.id);
        w.I64(v.share);
        w.I64(v.sharers);
        w.I64(v.own_route);
        w.I64(v.best_route);
    }
}

bool DaiGoldSides::Load(DaiBlobReader &r)
{
    *this = DaiGoldSides();
    m_turn = (DaiTurn)r.U64();
    m_hearts = r.U32();
    m_split = r.Bool();
    const int64_t n = r.Count(1 << 16);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        Vein v;
        v.id = r.U32();
        v.share = (DaiFix)r.Range(0, DAI_FIX_ONE);
        v.sharers = (int32_t)r.Range(0, DAI_MAX_PLAYERS + 1);
        v.own_route = (int32_t)r.Range(0, DAI_SIDES_NEVER);
        v.best_route = (int32_t)r.Range(0, DAI_SIDES_NEVER);
        m_veins.push_back(v);
    }
    std::sort(m_veins.begin(), m_veins.end(), [](const Vein &a, const Vein &b) { return a.id < b.id; });
    if (!r.Ok()) *this = DaiGoldSides();
    return r.Ok();
}
