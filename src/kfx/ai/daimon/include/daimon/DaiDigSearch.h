/** @file DaiDigSearch.h
 *  The dig search (02 section 4.13, 03 section 7): the cheapest way for the keeper's imps to get from where they can
 *  stand to a set of goal slabs, in turns of imp work -- walking, digging, bridging. A* on the slab grid, 4
 *  neighbours in the fixed order N, E, S, W, integer costs, the open list ordered by (f, g, slab index): the same
 *  answer on every machine and however the work is sliced. Multi-source: every source starts at cost 0, so the search
 *  also finds where to start digging. Incremental: Step() expands at most the turn's `search_nodes` and returns Wait.
 *  It reads the knowledge as it changes between steps, and starts over when a slab it has touched changes. Not saved:
 *  a job starts its search again after a load. */
#ifndef DAIMON_DAIDIGSEARCH_H
#define DAIMON_DAIDIGSEARCH_H

#include <cstdint>
#include <span>
#include <vector>

#include "globals.h"
#include "config_daimon.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiPersonality.h"
#include "daimon/DaiTypes.h"

/** Whose eyes the costs are through. Phase 1: OwnRoute, where the keeper's imps can go (mining: water is waded), and
 *  ClaimRoute, ground that must become the keeper's own (an entrance, a neutral room, a building site): imps claim only
 *  ground next to the keeper's solid land and water never is, so water must be bridged. Attack (Phase 5) and EnemyView
 *  (Phase 4) give enemy defences a price. */
enum class DaiDigModel : uint8_t { OwnRoute, ClaimRoute, Attack, EnemyView, Walk };

constexpr int32_t DAI_DIG_NEVER = INT32_MAX;     /**< a slab the search never enters */

struct DaiDigCosts {
    DaiDigModel model = DaiDigModel::OwnRoute;
    int32_t floor = 1, earth = 20, unknown = 30, gold_route = 200, dense_route = 500, own_wall = 200,
            enemy_floor = 80, bridge = DAI_DIG_NEVER, abyss_edge = 5;
    bool bridge_lava = false, bridge_abyss = false;     /**< the keeper can build a bridge over it now */
    bool bridge_water = false;                          /**< ClaimRoute: water is bridged (and never entered without) */
    /** EnemyView (06 section 3.2): whose eyes (`viewer`: the rival keeper the route starts from; -1 heroes or anyone);
     *  a fortified wall not the viewer's -- only a Destroy Walls cast breaks one, imps never dig it -- at
     *  `destroy_walls` (DAI_DIG_NEVER: nobody casts it), and the keeper's doors at `own_door_per_hp` turns a point of
     *  their health (DaiFix). */
    PlayerNumber viewer = -1;
    int32_t destroy_walls = DAI_DIG_NEVER;
    DaiFix own_door_per_hp = DAI_FIX_ONE / 4;
    /** Attack (07 section 4.2): another keeper's claimed floor and rooms at `creep` (imps convert them, contested), its
     *  doors at `door_per_hp` turns a point of their health as last seen (the assault bashes them), its revealed fortified
     *  walls at `destroy_walls` (a Destroy Walls cast: [dig_costs] destroy_walls_cost, 0 its price through gold_to_turns;
     *  DAI_DIG_NEVER when the keeper does not have the power). Walls it has not seen are unknown ground. */
    int32_t creep = DAI_DIG_NEVER;
    DaiFix door_per_hp = 0;
    DaiFix heuristic_weight = DAI_FIX_ONE;
    /** OwnRoute for this keeper now: [dig_costs], the personality's excavation table, and the bridge as the keeper
     *  may build it (available, its roles, its price at gold_to_turns). */
    static DaiDigCosts OwnRoute(const DaimonGlobalConfig &cfg, const DaiPersonality &p, const DaiKnowledge &k);
    /** ClaimRoute: OwnRoute with water bridged (if the bridge may cross it and is available, else never entered). */
    static DaiDigCosts ClaimRoute(const DaimonGlobalConfig &cfg, const DaiPersonality &p, const DaiKnowledge &k);
    /** Attack: OwnRoute (the keeper's imps dig the way; its creatures wade) with another keeper's ground, doors and walls
     *  priced as above. */
    static DaiDigCosts Attack(const DaimonGlobalConfig &cfg, const DaiPersonality &p, const DaiKnowledge &k);
    /** EnemyView: how `viewer` (a rival keeper; -1 heroes or anyone) reaches the keeper -- its creatures walk any open
     *  ground (the keeper's included) and wade water, its imps dig earth and gold and its own fortified walls; anyone
     *  else's fortified walls stop it (unless [threat] destroy_walls prices a Destroy Walls cast), the keeper's doors
     *  slow it by their health; lava and abyss (with no bridge of its own) and rock stop it. */
    static DaiDigCosts EnemyView(const DaimonGlobalConfig &cfg, PlayerNumber viewer);
    /** Walk: where the keeper's own creatures walk between its rooms -- open ground only, nothing dug (the own-traffic
     *  map, 06 section 3.3). */
    static DaiDigCosts Walk(const DaimonGlobalConfig &cfg);
};

class DaiDigSearch {
public:
    /** A new search from `sources` to any of `goals` (both de-duplicated; off-map slabs dropped), never entering
     *  `blocked` (forbidden areas, other jobs' area claims). Nothing is expanded until Step(). */
    void Start(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> sources, std::span<const DaiSlabXY> goals,
               const DaiDigCosts &costs, std::span<const DaiRect> blocked = {});
    /** The same, from every slab of the keeper's last complete reach map (taken again on each restart): where its
     *  imps can stand. For OwnRoute. */
    void StartFromReach(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> goals, const DaiDigCosts &costs,
                        std::span<const DaiRect> blocked = {});
    /** The same, from the keeper's own solid land (its claimed floor, rooms, bridges and doors; taken again on each
     *  restart): where claiming spreads from. For ClaimRoute -- ground the imps can walk to across water is not land
     *  the keeper can claim from. */
    void StartFromOwnLand(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> goals, const DaiDigCosts &costs,
                          std::span<const DaiRect> blocked = {});
    /** A flood from the keeper's own solid land with no goal: Step() ends Done when every slab it can reach has its
     *  cost, which CostAt() then gives (the layout planner's price of connecting a site, 04 section 5.1). */
    void StartFlood(const DaiKnowledge &k, PlayerNumber p, const DaiDigCosts &costs, std::span<const DaiRect> blocked = {});
    /** A finished flood's cost of reaching (x, y) from own land; DAI_DIG_NEVER if it cannot (or the flood is not done). */
    int32_t CostAt(DaiSlabCoord x, DaiSlabCoord y) const;
    /** Seeds and expands at most `b.search_nodes` nodes (decremented). Wait: budget spent, call again; Done: a path;
     *  Fail: no goal can be reached (or no search was started). */
    DaiResult Step(const DaiKnowledge &k, DaiBudget &b);
    /** The path found, from its source slab to its goal slab, both included; its length. */
    int64_t Path(DaiSlabXY *out, int64_t max) const;
    std::span<const DaiSlabXY> Path() const { return std::span<const DaiSlabXY>(m_path); }
    /** The path's cost (the sum of the costs of entering each slab after the source). */
    int64_t Cost() const { return m_cost; }
    DaiResult State() const { return m_state; }
    /** Nodes expanded since the search started (restarts included), and the restarts. */
    int64_t Expanded() const { return m_expanded; }
    int64_t Restarts() const { return m_restarts; }
    /** What entering slab (x, y) costs the keeper under these costs (DAI_DIG_NEVER: never entered). A goal slab that
     *  is gold or gems costs what earth does: it is mined, not dug through. */
    static int32_t EnterCost(const DaiKnowledge &k, PlayerNumber p, const DaiDigCosts &c, DaiSlabCoord x, DaiSlabCoord y,
                             bool goal);
    /** A search in progress in a saved game, so a keeper plays on after a load as if never saved: its inputs, the
     *  slabs it has touched (sparse), its open list and where it had read the knowledge's change log to. */
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    struct Node { int64_t f; int32_t g; int32_t at; };
    void restart(const DaiKnowledge &k);
    bool touched(int32_t at) const;
    int64_t heuristic(int32_t at) const;
    void push(int32_t at, int32_t g, int32_t parent);

    PlayerNumber m_player = -1;
    DaiSlabCoord m_w = 0, m_h = 0;
    DaiDigCosts m_costs;
    enum class From : uint8_t { Given, Reach, OwnLand, OwnLandFlood };
    From m_from = From::Given;
    std::vector<DaiSlabXY> m_sources_in, m_goals_in;
    std::vector<DaiRect> m_blocked;
    std::vector<int32_t> m_sources;     /**< slab indexes, in the order seeded */
    std::vector<uint8_t> m_goal;        /**< 1: a goal slab */
    DaiRect m_goal_box;
    std::vector<int32_t> m_g;           /**< best known cost; DAI_DIG_NEVER: not reached */
    std::vector<int32_t> m_parent;      /**< -1: a source */
    std::vector<uint8_t> m_closed;
    std::vector<Node> m_open;           /**< a binary heap, least (f, g, at) on top */
    size_t m_seeded = 0;
    int64_t m_min_step = 1;
    uint64_t m_seen_seq = 0;            /**< the knowledge's change log, read up to here */
    std::vector<DaiSlabXY> m_path;
    int64_t m_cost = 0;
    DaiResult m_state = DaiResult::Fail;
    int64_t m_expanded = 0, m_restarts = 0;
};

#endif
