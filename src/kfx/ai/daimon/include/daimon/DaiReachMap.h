/** @file DaiReachMap.h
 *  Where the keeper's imps can stand (03 section 5), without the navigation mesh (whose queries write caches, 02
 *  principle 8): the slabs connected by 4-neighbour walkable slabs to the keeper's own walkable ground -- the seat's
 *  reach check's rule (api_seat_unreachable_dig_slabs), which it agrees with on every fixture. Walkable as known:
 *  revealed, not blocking, not lava, not abyss, not another keeper's door; and a step that climbs two (onto a guard
 *  post from water, DaiKnowledge::RaisedStep) is none.
 *
 *  A flood over the whole map, spread over turns by a visit budget and double-buffered: decisions read the last
 *  complete map while the next is being built; a change of walkability or ownership starts another once this one is
 *  published. */
#ifndef DAIMON_DAIREACHMAP_H
#define DAIMON_DAIREACHMAP_H

#include "daimon_base_types.h"
#include <cstdint>
#include <vector>

#include "globals.h"
#include "daimon/DaiBlob.h"

class DaiReachMap {
public:
    /** What a slab is to the flood. */
    enum Walk : uint8_t { NotWalkable = 0, Walkable = 1, OwnWalkable = 2 };

    void Init(DaiSlabCoord w, DaiSlabCoord h);
    /** A slab's walkability or owner changed: the published map is out of date. */
    void NoteChange() { m_stale = true; }
    /** Goes on with the flood (starting one if the map is stale), visiting at most `budget` slabs (decremented).
     *  `walk(index)` says what slab `index` (y * w + x) is, `barred(from, to)` whether the step between two walkable
     *  neighbours is none. Returns true when a new map was published. */
    template <typename WalkFn, typename BarFn> bool Step(WalkFn walk, BarFn barred, int64_t &budget);

    bool Reached(DaiSlabCoord x, DaiSlabCoord y) const;
    /** A slab next to a reached one (4 sides): where an imp can stand to dig it. */
    bool NextToReached(DaiSlabCoord x, DaiSlabCoord y) const;
    /** Floods completed (0: no map yet). */
    int64_t Cycles() const { return m_cycles; }
    bool Running() const { return m_running; }
    DaiSlabCoord Width() const { return m_w; }
    DaiSlabCoord Height() const { return m_h; }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    DaiSlabCoord m_w = 0, m_h = 0;
    std::vector<uint8_t> m_published;   /**< 1: reached */
    std::vector<uint8_t> m_work;        /**< the flood in progress */
    std::vector<int32_t> m_queue;
    size_t m_head = 0;
    bool m_running = false, m_stale = true, m_seeded = false;
    int64_t m_seed_at = 0;              /**< seeding goes through the map within the budget too */
    int64_t m_cycles = 0;
};

template <typename WalkFn, typename BarFn>
bool DaiReachMap::Step(WalkFn walk, BarFn barred, int64_t &budget)
{
    const int64_t n = (int64_t)m_w * m_h;
    if (n <= 0) return false;
    if (!m_running) {
        if (!m_stale) return false;
        m_running = true;
        m_stale = false;
        m_seeded = false;
        m_seed_at = 0;
        m_work.assign((size_t)n, 0);
        m_queue.clear();
        m_head = 0;
    }
    // Seeds: the keeper's own walkable ground.
    while (!m_seeded && (budget > 0)) {
        for (; (m_seed_at < n) && (budget > 0); m_seed_at++, budget--) {
            if (walk(m_seed_at) == OwnWalkable) {
                m_work[(size_t)m_seed_at] = 1;
                m_queue.push_back((int32_t)m_seed_at);
            }
        }
        if (m_seed_at >= n) m_seeded = true;
    }
    while (m_seeded && (m_head < m_queue.size()) && (budget > 0)) {
        const int32_t at = m_queue[m_head++];
        const DaiSlabCoord x = at % m_w, y = at / m_w;
        // N, E, S, W: a fixed order, the same flood on every machine
        const int32_t w = (int32_t)m_w;
        const int32_t next[4] = { (y > 0) ? at - w : -1, (x + 1 < m_w) ? at + 1 : -1, (y + 1 < m_h) ? at + w : -1,
                                  (x > 0) ? at - 1 : -1 };
        for (int32_t nb : next) {
            budget--;
            if ((nb < 0) || m_work[(size_t)nb] || (walk(nb) == NotWalkable) || barred(at, nb)) continue;
            m_work[(size_t)nb] = 1;
            m_queue.push_back(nb);
        }
    }
    if (!m_seeded || (m_head < m_queue.size())) return false;
    m_published.swap(m_work);
    m_running = false;
    m_queue.clear();
    m_head = 0;
    m_cycles++;
    return true;
}

#endif
