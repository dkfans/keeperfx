/** @file DaiConstruction.h
 *  Construction (04 section 6): builds the room programme. Each turn it takes the most wanted Build goal whose room
 *  has fewer slabs than the goal asks (or is full past [layout] grow_at), asks the layout planner where it goes --
 *  beside the room the personality anchors it to, or the heart; grown by a strip when it already stands; where a lost
 *  one was when it is rebuilt -- links the site's entrance to the keeper's land with a claim route when it is not beside
 *  it (digging through its own wall, bridging water: 03 section 7.5), then places the room (place_room, grow_room,
 *  rebuild_room). One room is built at a time ([construction] room_build_concurrency). It puts a door on each entrance
 *  of the rooms it built when the keeper has one, asks for workshop workers for the doors it lacks (DaiStockPlanner,
 *  the Manufacture goal), sells a room's slabs or places a room near a spot when another manager asks it to (service
 *  requests: Economy's emergency sale, its forward treasury by a far vein). From Phase 2
 *  it builds the treasury too (02 section 12 C8), at the size Economy asks for. */
#ifndef DAIMON_DAICONSTRUCTION_H
#define DAIMON_DAICONSTRUCTION_H

#include "daimon_base_types.h"
#include <array>
#include <vector>

#include "daimon/DaiLayoutSearch.h"
#include "daimon/IDaiManager.h"

class DaiConstruction : public IDaiManager {
public:
    const char *GetName() const override { return "construction"; }
    DaiDomain GetDomain() const override { return DaiDomain::Construction; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;

    /** How a room is built: a new one, a strip added to one, or one where a lost one was. */
    enum class Mode : uint8_t { Place, Grow, Rebuild };
    /** An entrance of a room Construction built, and its door. */
    struct Entrance {
        DaiRoom room = DaiRoom::Count;
        DaiRect area;
        DaiSlabXY at;
        bool door = false;
        DaiTurn retry_after = 0;
    };
    bool Searching() const { return m_searching; }
    DaiRoom SearchRoom() const { return m_search_room; }
    const std::vector<Entrance> &Entrances() const { return m_entrances; }
private:
    void serve(const DaiTurnCtx &c, DaiManagerWrites &w);
    void place_at(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiRequest &r);
    void doors(const DaiTurnCtx &c, DaiManagerWrites &w);
    void rooms(const DaiTurnCtx &c, DaiManagerWrites &w);
    void start_search(const DaiTurnCtx &c, DaiRoom r, int64_t need, int64_t have, const DaiSlabXY *at = nullptr,
                      const DaiRect *exact = nullptr);
    void step_search(const DaiTurnCtx &c, DaiManagerWrites &w);
    void start_site(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool sell_strip(const DaiTurnCtx &c, DaiRoom r, int64_t amount, DaiRect *out);

    DaiLayoutSearch m_search;
    bool m_searching = false;
    DaiRoom m_search_room = DaiRoom::Count;
    Mode m_mode = Mode::Place;
    uint32_t m_record = 0;
    // The site found, waiting for its jobs (a link first, when its entrance is not beside the keeper's land).
    bool m_has_site = false;
    DaiSite m_site;
    DaiTurn m_site_turn = 0;
    DaiRoom m_site_room = DaiRoom::Count;
    Mode m_site_mode = Mode::Place;
    uint32_t m_site_record = 0;
    int64_t m_budget = 0;               /**< a requested room's digging may cost at most this (0: no limit) */
    std::array<DaiTurn, (size_t)DaiRoom::Count> m_retry{};         /**< no new search for the room before this */
    std::array<DaiTurn, (size_t)DaiRoom::Count> m_grow_failed{};   /**< growing it last failed then: a new room instead */
    std::vector<Entrance> m_entrances;
};

#endif
