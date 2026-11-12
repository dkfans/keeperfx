/** @file DaiThreatModel.h
 *  Where the keeper's enemies are and how they would come (06 section 3; owned by Defence, readable by Tactics, the
 *  director and the planners, 02 section 4.13c):
 *   - the threat map: per slab, the decayed influence of the enemy force the keeper has seen (each enemy adds its force
 *     to the slabs within its reach every [threat] interval; it halves every halflife turns -- decayed lazily, so a
 *     slab nobody looks at costs nothing);
 *   - the predicted routes: from each source (a rival keeper's ground seen, a hero gate seen, the frontier where open
 *     ground not the keeper's meets its own) to its heart, treasury and library, by the dig search's EnemyView costs
 *     (water waded, earth dug, fortified walls stopping all but their owner, the keeper's doors slowing); searched again every
 *     route_interval turns, or at once when a source is new or a rival's ground came to touch the keeper's (a breach);
 *   - the chokepoints: slabs on those routes where the way is one slab wide, scored by the weight of the routes
 *     through them (a rival's known force, or the frontier's and a hero gate's weight);
 *   - the own-traffic map: the slabs the keeper's own creatures walk between its heart and its lairs, hatcheries,
 *     treasuries and work rooms (traps that hurt them stay off it, doors on it stay unlocked, 06 section 3.3);
 *   - the hazards (07 P5.13): a neutral's or the heroes' trap, armed, by which (within [defence] hazard_reach slabs)
 *     the keeper's imps were lost -- each imp gone from its list since the last update, at the slab it was last on.
 *     Hot while hazard_imps were lost by it within hazard_window turns; gone when the trap is spent or gone.
 *  Reads the knowledge only. Saved whole, the route searches in progress included, so a keeper plays on after a load
 *  as if never saved. */
#ifndef DAIMON_DAITHREATMODEL_H
#define DAIMON_DAITHREATMODEL_H

#include <cstdint>
#include <span>
#include <vector>

#include "config_daimon.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiDigSearch.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiTypes.h"

enum class DaiThreatSource : uint8_t { Rival, HeroGate, Frontier, Traffic, Count };

struct DaiThreatRoute {
    DaiThreatSource source = DaiThreatSource::Frontier;
    PlayerNumber rival = -1;        /**< Rival: whose ground it starts on */
    int64_t weight = 0;
    int64_t cost = 0;               /**< the search's cost, turns of walking and digging */
    std::vector<DaiSlabXY> path;    /**< from its source slab to the target slab */
};

struct DaiChokepoint {
    DaiSlabCoord x = 0, y = 0;
    int64_t score = 0;              /**< the weight of the routes through it */
    bool own = false;               /**< the keeper's own ground (a trap or door can go there); else nobody's */
};

struct DaiHazard {
    DaiThingRef trap;
    DaiThingModel model = 0;
    PlayerNumber owner = -1;
    DaiSlabCoord x = 0, y = 0;
    std::vector<DaiTurn> losses;   /**< when imps were lost by it, within hazard_window (the newest DAI_HAZARD_LOSSES) */
};

class DaiThreatModel {
public:
    /** One turn: the threat map's adds when due, the sources looked at when due, and the route searches stepped
     *  (at most [threat] search_nodes nodes). */
    void Update(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now);
    /** The decayed threat on slab (x, y) at `now` (0 off the map). */
    int64_t ThreatAt(DaiSlabCoord x, DaiSlabCoord y, DaiTurn now) const;
    /** The last complete set of routes (empty until the first is done), and when it was done. */
    std::span<const DaiThreatRoute> Routes() const { return std::span<const DaiThreatRoute>(m_routes); }
    std::span<const DaiChokepoint> Chokepoints() const { return std::span<const DaiChokepoint>(m_chokepoints); }
    DaiTurn RoutesTurn() const { return m_routes_turn; }
    /** How many complete sets have been made (a reader can tell a new one from the last it saw). */
    int64_t Generation() const { return m_generation; }
    /** Own creatures' routes through slab (x, y) in the last complete set (0: none; saturates at 255). */
    int64_t TrafficAt(DaiSlabCoord x, DaiSlabCoord y) const;
    /** Route searches are under way. */
    bool Searching() const { return m_qi < m_queue.size(); }
    /** The hazards (P5.13), hot or not; a hazard is hot while hazard_imps imps were lost by it within hazard_window. */
    std::span<const DaiHazard> Hazards() const { return std::span<const DaiHazard>(m_hazards); }
    bool Hot(const DaiHazard &h) const { return (int64_t)h.losses.size() >= m_hazard_imps; }
    bool AnyHot() const;
    /** The nearest hot hazard within `reach` slabs of (x, y) (chess distance); nullptr if none. */
    const DaiHazard *HotNear(DaiSlabCoord x, DaiSlabCoord y, int64_t reach) const;
    /** The next check searches the routes again whatever it finds (a breach Defence was told of, a test). */
    void RequestReroute() { m_reroute = true; }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    struct Query {
        DaiThreatSource source = DaiThreatSource::Frontier;
        PlayerNumber rival = -1;
        int64_t weight = 0;
        std::vector<DaiSlabXY> sources, goals;
    };
    void init(DaiSlabCoord w, DaiSlabCoord h);
    void add_threat(DaiSlabCoord x, DaiSlabCoord y, int64_t force, DaiTurn now);
    void add_enemies(const DaiKnowledge &k, const DaimonGlobalConfig &cfg, DaiTurn now);
    uint64_t signature(const DaiKnowledge &k, PlayerNumber p) const;
    void plan(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now);
    void start_next(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg);
    void finish(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now);
    void hazards(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, DaiTurn now);

    DaiSlabCoord m_w = 0, m_h = 0;
    int64_t m_halflife = 1500;
    std::vector<int64_t> m_value;       /**< the threat as it was at m_stamp */
    std::vector<DaiTurn> m_stamp;
    DaiTurn m_last_add = 0, m_last_check = 0;
    uint64_t m_signature = 0;
    bool m_reroute = false;
    // The published set.
    std::vector<DaiThreatRoute> m_routes;
    std::vector<DaiChokepoint> m_chokepoints;
    std::vector<uint8_t> m_traffic;
    DaiTurn m_routes_turn = 0;
    int64_t m_generation = 0;
    // The set in the making.
    std::vector<Query> m_queue;
    size_t m_qi = 0;
    bool m_search_on = false;
    DaiDigSearch m_search;
    std::vector<DaiThreatRoute> m_building;
    std::vector<uint8_t> m_building_traffic;
    // The hazards and the imps as last seen (P5.13).
    struct ImpSeen { DaiThingRef ref; DaiSlabCoord x = 0, y = 0; };
    std::vector<DaiHazard> m_hazards;
    std::vector<ImpSeen> m_imps;
    int64_t m_hazard_imps = 2;
};

#endif
