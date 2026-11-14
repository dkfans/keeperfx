/** @file DaiPerimeter.h
 *  The keeper's perimeter (plan 13 section 32): the few slabs that, held, close its home off from the rest of the
 *  map -- level 61's keeper ringed in rock, two five-slab gaps of earth its only ways out (each opening on a gold
 *  bracket). The map as every keeper sees it from the start (all the rock, and the lava, abyss and impassable ground
 *  revealed; earth, gold, water and floor passable, they can be dug or walked) is cut between the keeper's heart and
 *  the slabs `radius` or more slabs of route from it: the slabs whose loss parts them (a minimum vertex cut, each slab
 *  priced by its open sides, so the cut lies in a gap between rock rather than on open ground at its mouth).
 *  The ground nearest the heart (half perimeter_min_area slabs) is never cut. Of the radii to DAI_PERIMETER_RADIUS_MAX,
 *  the smallest cut that encloses [defence] perimeter_min_area slabs, leaves at least as many outside as in (the rest
 *  of the map, not a corner of it) and is no more than perimeter_max_cut slabs; none on open ground (level 124).
 *
 *  Closed at first (the owner, 2026-11-09: no door needed at the start): every slab of every gap is kept -- the
 *  keeper's imps never dig them (DaiKnowledge::Kept, read by the dig search's costs and the room layout) -- and Defence
 *  has the lining (each gap's slabs just inside) dug and claimed, so the imps fortify the whole gap, which no enemy imp
 *  digs. Opened once (the keeper decides, SetOpen) when it is ready to attack or no gold is left inside: each gap's
 *  middle slab, its gate, is then its own way out -- dug through its own wall -- and Fortify puts a door on it. A gap
 *  slab dug open already (a rival in first) is a door's place at once, open or closed. A fortified wall stands only
 *  while the keeper's land is beside it (the engine's do_unprettying): the lining kept is what keeps the walls.
 *  Worked out when the keeper's heart is first known and again when the blocking ground it knows changes (lava
 *  revealed), at most every DAI_PERIMETER_INTERVAL turns -- a radius a planning turn; saved. */
#ifndef DAIMON_DAIPERIMETER_H
#define DAIMON_DAIPERIMETER_H

#include <cstdint>
#include <vector>

#include "globals.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiPersonality.h"
#include "daimon/DaiTypes.h"

struct DaimonGlobalConfig;
struct DaiForecastOut;
class DaiRivals;

constexpr int64_t DAI_PERIMETER_INTERVAL = 2000;
constexpr int64_t DAI_PERIMETER_RADIUS_MAX = 48, DAI_PERIMETER_RADIUS_STEP = 3;

class DaiPerimeter {
public:
    struct Gap {
        std::vector<DaiSlabXY> slabs;   /**< the cut's slabs in it, row by row */
        DaiSlabXY gate;                 /**< its middle slab: the keeper's own way out once open, a door's place */
        std::vector<DaiSlabXY> lining;  /**< the slabs just inside it, row by row: dug and claimed, the gap is fortified */
    };
    void Update(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now);
    void Compute(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now);
    bool Found() const { return !m_gaps.empty(); }
    /** Opened (for good): the gates no longer kept. */
    bool Open() const { return m_open; }
    void SetOpen() { m_open = true; }
    /** Closed until the keeper is ready to attack the best target with a door to put on a gate, or no gold is left
     *  inside (the map scanned once, so the vein table is whole): then the gates open, for good (plan 13 section 32).
     *  The reason it opened now, or nullptr. */
    const char *MaybeOpen(const DaiKnowledge &k, const DaiPersonality &per, const DaimonGlobalConfig &cfg,
                          const DaiForecastOut &forecast, const DaiRivals &rivals);
    /** An agent's perimeter directive (P8.4): open, or closed again (the gates get their doors back). */
    void SetOpenState(bool open) { m_open = open; }
    /** The gold left in the veins inside it (their middle slab inside); INT64_MAX / 4 with a gem face it touches. */
    int64_t GoldInside(const DaiKnowledge &k) const;
    const std::vector<Gap> &Gaps() const { return m_gaps; }
    int64_t CutSlabs() const;
    int64_t Enclosed() const { return m_enclosed; }
    int64_t Radius() const { return m_radius; }
    DaiTurn Turn() const { return m_turn; }
    /** The cut's slabs held now: rock, the keeper's fortified wall, or (a gate, open) its door. */
    int64_t Closed(const DaiKnowledge &k, PlayerNumber p) const;
    /** Into the knowledge (each planning turn): the kept slabs, the gates (open), the lining. */
    void Apply(DaiKnowledge &k) const;
    /** The minimum vertex cut between the ground nearest slab (hx, hy) -- the nearest `inner_area` slabs and those as
     *  near, never cut: else the cut is the four slabs about the heart -- and every passable slab `radius` or more
     *  slabs of route from it, if it is at most `max_cut` slabs (else false). Of the cuts as cheap, the one nearest the
     *  heart. The slabs on the heart's side in *enclosed, those it reaches beyond in *outside. */
    static bool MinCut(const DaiKnowledge &k, DaiSlabCoord hx, DaiSlabCoord hy, int64_t inner_area, int64_t radius,
                       int64_t max_cut, std::vector<DaiSlabXY> *cut, int64_t *enclosed, int64_t *outside = nullptr,
                       int64_t max_cost = INT64_MAX, int64_t *cost = nullptr);
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    static uint64_t signature(const DaiKnowledge &k);
    void straighten(const DaiKnowledge &k, DaiSlabCoord hx, DaiSlabCoord hy);
    /** The radius scan, a radius a planning turn (a whole map's minimum cuts in one turn cost ~90 ms on a 170-slab map):
     *  begun, stepped, its best published. */
    void start(const DaiKnowledge &k, const DaiPersonality &p);
    void step(const DaiKnowledge &k, const DaiPersonality &p);
    void finish(const DaiKnowledge &k, DaiTurn now);
    /** The slabs the heart reaches with the cut closed (a slab a byte; worked out again after a load). */
    const std::vector<uint8_t> &inside(const DaiKnowledge &k) const;

    std::vector<Gap> m_gaps;
    int64_t m_enclosed = 0, m_radius = 0;
    uint64_t m_signature = 0;
    DaiTurn m_turn = 0;            /**< 0: never */
    bool m_open = false;
    // The scan under way (saved: a keeper plays on after a load as if never saved).
    int64_t m_scan_r = 0;               /**< the next radius; 0 idle */
    uint64_t m_scan_sig = 0;
    std::vector<DaiSlabXY> m_scan_best;
    int64_t m_scan_cost = 0, m_scan_enclosed = 0, m_scan_radius = 0;
    mutable std::vector<uint8_t> m_inside;      /**< not saved: from the cut and the map's rock */
};

#endif
