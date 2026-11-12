/** @file DaiExcavation.h
 *  Excavation (03 section 8.2): which veins to mine and which neutral entrances and rooms to reach, and the dig
 *  routes, mining and bridges for them. Every rescore_interval turns it scores the veins -- value over the turns to
 *  reach (a straight-line estimate through what it would dig), mine and haul them -- and starts the best (a vein for
 *  every 4 imps, up to max_veins at once; none while Mine weighs nothing or while the keeper has no treasury to bank
 *  the gold in): a mine_vein job where imps can stand by it, else it searches
 *  for a route itself (one probe at a time, in the keeper's search pool) and only a route found becomes a dig_route
 *  job, its path given; a vein with no route is Unreachable until unreachable_retry turns pass. An ExpandTo goal gets
 *  a claim route (03 section 7.5), one at a time; once it is open the imps must claim the room within claim_window.
 *  Phase 5 (07 section 4.2): another manager's Dig request (a probe tunnel, a breach dig) is a dig_route job to its
 *  slab; the request is refused when no route is found or the job fails. */
#ifndef DAIMON_DAIEXCAVATION_H
#define DAIMON_DAIEXCAVATION_H

#include "daimon_base_types.h"
#include <array>
#include <vector>

#include "daimon/IDaiManager.h"

class DaiExcavation : public IDaiManager {
public:
    const char *GetName() const override { return "excavation"; }
    DaiDomain GetDomain() const override { return DaiDomain::Excavation; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;

    /** A vein's score (x1024 gold a turn of work): its value over route, mining and haul turns -- its gold and its gems
     *  (their gold over the horizon, hit by hit) scored apart, the better taken. `route_turns` is the estimate to reach
     *  it, `haul_to` the treasury (or heart) its gold goes to. Pure, for tests. */
    static int64_t VeinScore(const DaiTurnCtx &c, const DaiVein &v, int64_t route_turns, DaiSlabXY entry, DaiSlabXY haul_to,
                             int64_t imps_available);
    /** Job keys: a vein's route and its mining are keyed by the vein id; an ExpandTo target's by this tag and its key. */
    static constexpr uint64_t EXPAND_KEY = 1ull << 40;
    /** A Dig request's route, keyed by the request's id under this tag. */
    static constexpr uint64_t SERVICE_KEY = 1ull << 41;
private:
    void rescore(const DaiTurnCtx &c, DaiManagerWrites &w);
    void expand(const DaiTurnCtx &c, DaiManagerWrites &w);
    void probe(const DaiTurnCtx &c, DaiManagerWrites &w);
    void hazards(const DaiTurnCtx &c, DaiManagerWrites &w);
    /** Its own marks on the perimeter's kept slabs taken off (plan 13 section 32): made before the perimeter was known,
     *  the imps would dig the gap open all the same. */
    void kept_marks(const DaiTurnCtx &c, DaiManagerWrites &w);
    void services(const DaiTurnCtx &c, DaiManagerWrites &w);
    /** The search key of Excavation's route probe in the keeper's search pool. */
    static constexpr uint32_t PROBE_KEY = DAI_SEARCH_KEY_MANAGER + 1;
    /** An ExpandTo target whose route is open, waiting for the imps to claim it. */
    struct ClaimWatch { int64_t key = -1; DaiTurn since = 0; };
    DaiTurn m_next_rescore = 0;
    uint32_t m_probe = 0;               /**< the vein whose route is being searched (0: none); its search is not saved */
    std::array<ClaimWatch, 8> m_watch{};
};

#endif
