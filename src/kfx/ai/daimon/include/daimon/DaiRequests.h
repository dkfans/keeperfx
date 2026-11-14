/** @file DaiRequests.h
 *  Requests between managers (02 section 4.5, section 12 C1): the one channel by which a manager asks for something it
 *  does not own. Phase 1 has goal-weight requests: a manager asks the director to weigh a goal more or less (Economy:
 *  Mine and KeepGold when the next pay day is short), and the director folds them into the goals it sets, capped.
 *  A request may also put a floor under the goal's count (Phase 2: Economy asks for the treasury's size, Creatures for
 *  lair and attraction-room slabs; 04 section 4). Service requests are one manager asking another for
 *  work (Phase 2: Construction sells a room's slabs when Economy asks). A service request is keyed by (from, to,
 *  service, key). A request is keyed by (from, goal, key): posting again refreshes it. */
#ifndef DAIMON_DAIREQUESTS_H
#define DAIMON_DAIREQUESTS_H

#include <cstdint>

#include "globals.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiGoals.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiPool.h"
#include "daimon/DaiTypes.h"

enum class DaiRequestKind : uint8_t { GoalWeight, Service, Count };
/** Work one manager asks of another (02 section 4.5): Construction sells a room's slabs (Economy's emergency rule,
 *  04 section 8), or places a room near a spot (Economy's forward treasury by a far vein, 04 section 8). */
/** What one manager may ask another to do (02 C1): Construction sells or places a room (near a spot, or exactly on a
 *  slab of its own floor: PlaceRoomAt, P5.14's landings); Excavation digs a route to a slab (Phase 5: probe tunnels,
 *  breach digs) or brings imps to creep onto another keeper's slab; DaiMagic casts Sight of Evil or Destroy Walls at a
 *  slab (Phase 5). */
enum class DaiService : uint8_t { None, SellRoom, PlaceRoom, Dig, SightPeek, DestroyWalls, Creep, PlaceRoomAt, Reinforce, Count };

struct DaiRequest {
    uint32_t id = 0;
    DaiDomain from = DaiDomain::Count;
    DaiRequestKind kind = DaiRequestKind::GoalWeight;
    DaiGoalKind goal = DaiGoalKind::Count;
    int64_t key = 0;                /**< the goal's key (DaiGoalParams::key); a service's own key: one request stands per
                                         asker, server, service and key, so an asker with several at once (Offence's
                                         casts and digs at different slabs) gives each its slab index */
    DaiFix weight_delta = 0;        /**< added to the goal's weight while the request stands */
    int64_t count_min = 0;          /**< a floor on the goal's count while it stands (a Build goal: its slabs; 04 section 4) */
    DaiDomain to = DaiDomain::Count;    /**< Service: the manager asked */
    DaiService service = DaiService::None;
    int64_t amount = 0;             /**< Service: how much (SellRoom, PlaceRoom: slabs of the DaiRoom `key`; SightPeek,
                                         DestroyWalls: the power's level; Creep: imps) */
    DaiSlabXY at{ -1, -1 };         /**< PlaceRoom: the spot it should be near (PlaceRoomAt: the slab, one); Dig: the slab to dig (dug, not stood by); SightPeek,
                                         DestroyWalls: the slab to cast at; Creep: the slab to convert */
    int64_t budget = 0;             /**< PlaceRoom: turns of digging it may cost at most (more and it is not worth it) */
    bool served = false;            /**< Service: the manager asked has acted on it */
    bool refused = false;           /**< Service: ...and could not do it (the asker may ask for something else) */
    DaiTurn posted = 0;
    DaiTurn expires = 0;           /**< 0: until withdrawn */
    char why[32] = {};
};

/** The most a goal's weight may reach with requests folded in (and the least is 0). */
constexpr DaiFix DAI_GOAL_WEIGHT_MAX = 8 * DAI_FIX_ONE;

class DaiRequests {
public:
    /** Inserts the request, or refreshes the poster's request for the same goal and key; its id, 0 if full or the
     *  request names no goal. */
    uint32_t Post(DaiDomain from, const DaiRequest &r, DaiTurn now);
    /** Drops the poster's own request (another domain's is left alone). */
    void Withdraw(DaiDomain from, uint32_t id);
    /** The poster's request for this goal and key, or nullptr. */
    const DaiRequest *Find(DaiDomain from, DaiGoalKind goal, int64_t key) const;
    void Expire(DaiTurn now);
    /** Handed back, or the level ended. */
    void Clear() { m_requests.Clear(); }
    /** The goal-weight requests' summed delta for a goal. */
    DaiFix WeightDelta(DaiGoalKind goal, int64_t key) const;
    /** A goal's weight with the requests folded in, within [0, DAI_GOAL_WEIGHT_MAX]. */
    DaiFix Folded(DaiGoalKind goal, int64_t key, DaiFix base) const;
    /** The largest count any request asks of a goal (0: none asks). */
    int64_t CountFloor(DaiGoalKind goal, int64_t key) const;
    /** The server marks a service request addressed to it as acted on (another domain's mark is refused). */
    void MarkServed(DaiDomain server, uint32_t id);
    /** The server could not do it (marked served too). */
    void MarkRefused(DaiDomain server, uint32_t id);
    /** f(request) for each service request addressed to `server`, in slot order. */
    template <typename F> void ForEachService(DaiDomain server, F f) const
    {
        m_requests.ForEach([&](const DaiRequest &r) { if ((r.kind == DaiRequestKind::Service) && (r.to == server)) f(r); });
    }
    int64_t Count() const { return m_requests.Count(); }
    template <typename F> void ForEach(F f) const { m_requests.ForEach(f); }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    DaiPool<DaiRequest, DAI_MAX_REQUESTS> m_requests;
};

const char *DaiRequestKindName(DaiRequestKind k);
const char *DaiServiceName(DaiService s);

#endif
