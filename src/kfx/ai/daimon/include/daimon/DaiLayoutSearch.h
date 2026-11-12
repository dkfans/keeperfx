/** @file DaiLayoutSearch.h
 *  The layout planner (02 section 4.13a, 04 section 5): where a room goes, or which way it grows. A site is a
 *  rectangle of slabs that will become the keeper's claimed floor and then the room, with one entrance slab on its
 *  rim (the ring round it) through which it is reached; the rest of the rim keeps it apart from every other room.
 *  Sites are weighed around an anchor in a fixed spiral order, a budget of candidates a turn; the best wins, ties to
 *  the first weighed. The room's efficiency is predicted with the engine's own rule (room_side_score.h) on the map as
 *  it will be: earth beside the room will be the keeper's wall once its imps fortify it.
 *
 *  Before weighing sites the planner floods the map from the keeper's own solid land with the claim-route costs
 *  (DaiDigSearch, 03 section 7.5): what connecting each slab costs, bridging water included. A site whose entrance
 *  the flood cannot reach is not feasible. Integer arithmetic, fixed order: the same answer on every machine and
 *  however the work is sliced. Saved with whatever holds it, so a keeper plays on after a load as if never saved. */
#ifndef DAIMON_DAILAYOUTSEARCH_H
#define DAIMON_DAILAYOUTSEARCH_H

#include <cstdint>
#include <span>
#include <vector>

#include "globals.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiDigSearch.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiPersonality.h"
#include "daimon/DaiTypes.h"

/** A place for a room, or the strip a room grows by. */
struct DaiSite {
    bool found = false;
    DaiRect r;                              /**< the site (a growth: the strip added) */
    DaiSlabCoord ent_x = -1, ent_y = -1;    /**< its entrance, on the rim (a growth has none: the room is its way in) */
    int64_t score = 0;
    int32_t eff_pred = 0;                   /**< 0..256: the room it makes (a growth: the room once grown) */
    int32_t dig_turns = 0;                  /**< digging the site and connecting its entrance to the keeper's land */
};

/** What is wanted: a new room of `kind` of at least `slabs_want` slabs near the anchor, or (grow_record != 0) that room
 *  grown by at least `slabs_want` slabs. */
struct DaiLayoutQuery {
    RoomKind kind = 0;
    int64_t slabs_want = 9;
    DaiSlabCoord ax = 0, ay = 0;            /**< where it should be near: the heart, an anchor room, a vein's entry */
    RoomKind anchor_kind = 0;               /**< rooms of the keeper's it should be beside (0: none) */
    bool door = false;                      /**< a door will close its entrance */
    uint32_t grow_record = 0;               /**< grow this room (its DaiRoomRecord) instead */
    bool exact = false;                     /**< that rectangle and no other (a script's DAIMON_BUILD, Phase 7) */
    DaiRect exact_rect;
};

/** The score's weights (DaiFix), from the personality's layout table (04 section 5.2). */
struct DaiLayoutWeights {
    DaiFix eff = 4 * DAI_FIX_ONE, adj = DAI_FIX_ONE, dist = 4 * DAI_FIX_ONE, dig = 102, exp = 0, grow = 2 * DAI_FIX_ONE,
           waste = 20 * DAI_FIX_ONE;
    static DaiLayoutWeights From(const DaiPersonality &p);
};

/** The score's terms of a site, before the weights (for tests and logs). */
struct DaiSiteTerms {
    int64_t eff_pct = 0, adj = 0, dist = 0, dig = 0, exp = 0, grow = 0, waste = 0;
};

class DaiLayoutSearch {
public:
    /** A new search. `radius`: how far from the anchor a site's middle may be; `rim`: its width (1 or 2). Nothing
     *  is weighed until Step(). */
    void Start(const DaiKnowledge &k, PlayerNumber p, const DaiLayoutQuery &q, const DaiLayoutWeights &w,
               const DaiDigCosts &claim_costs, int64_t radius, int64_t rim, std::span<const DaiRect> blocked = {});
    /** The flood within `b.search_nodes`, then at most `b.layout_candidates` sites (both decremented). Wait: call
     *  again; Done: Best() is the site; Fail: no feasible site (or no search). */
    DaiResult Step(const DaiKnowledge &k, DaiBudget &b);
    const DaiSite &Best() const { return m_best; }
    DaiResult State() const { return m_state; }
    /** Candidates weighed so far, and of them feasible. */
    int64_t Weighed() const { return m_next; }
    int64_t Feasible() const { return m_feasible; }
    /** Whether the best site is still feasible on the knowledge as it is now (a result kept a while, 04 section 5.3). */
    bool StillFeasible(const DaiKnowledge &k) const;
    /** The efficiency (0..256) the engine will give a room on rectangle `r` of the keeper `p`'s, predicted from the
     *  knowledge: earth round it fortified by then, the entrance (-1: none) open or closed by a door. */
    static int32_t PredictEfficiency(const DaiKnowledge &k, PlayerNumber p, const DaiRect &r, DaiSlabCoord ent_x,
                                     DaiSlabCoord ent_y, bool door);
    /** The terms of a site as Step() weighs it (DaiSite::found false: not feasible). */
    DaiSite Weigh(const DaiKnowledge &k, const DaiRect &r, DaiSiteTerms *terms = nullptr) const;
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    struct Shape { int32_t w, h; };
    void prepare();
    bool candidate(int64_t n, DaiRect *r) const;
    int64_t candidates() const;
    bool inside(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y, const DaiRect &r) const;
    int32_t predict(const DaiKnowledge &k, const DaiRect &r, DaiSlabCoord ent_x, DaiSlabCoord ent_y) const;

    PlayerNumber m_player = -1;
    DaiLayoutQuery m_q;
    DaiLayoutWeights m_w;
    int64_t m_radius = 12, m_rim = 1;
    std::vector<DaiRect> m_blocked;
    DaiDigCosts m_costs;                    /**< the claim route's: a site slab's digging, the flood's */
    DaiDigSearch m_flood;
    // Derived from the above at Start and Load (not saved).
    std::vector<DaiSlabXY> m_offsets;       /**< the spiral of site middles round the anchor */
    std::vector<Shape> m_shapes;
    DaiRect m_room_box;                     /**< a growth: the room's box (-1s: none) */
    int64_t m_next = 0;                     /**< the next candidate to weigh */
    int64_t m_feasible = 0;
    DaiSite m_best;
    DaiResult m_state = DaiResult::Fail;
};

#endif
