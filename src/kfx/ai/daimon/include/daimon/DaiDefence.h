/** @file DaiDefence.h
 *  Defence (06 sections 4, 5.2, 6): the keeper's defence strategy. It never acts in a fight -- it posts objectives for
 *  Tactics and fortifies:
 *   - standing objectives, every [defence] interval: HoldHeart always; HoldArea for a room (treasury, library, lair,
 *     hatchery, workshop, training room) whose box the threat map holds at least area_threat on; ProtectImps where its
 *     imps work on at least imps_threat; those no longer threatened are dropped. Weights: the personality's
 *     heart_priority, rooms_priority, imps_priority;
 *   - invasions, every turn: an intruder seen (KF_Intruder) or an alarm trap going off (KF_TrapFired of an ALARM) is an
 *     Intercept there for intercept_turns. A breach (KF_WallCue) already has the threat model search its routes again;
 *   - fortification (DaiFortify's plan): what the stock lacks asked of the workshop (its Manufacture floor, as
 *     Construction asks for doors); a place_trap / place_door job for the best placement in stock, one at a time, a
 *     slab whose placement failed left alone for a while; destroyed traps and doors come back into the plan;
 *   - locks (Q6): a door not on the own-traffic map locked while an enemy is within lock_radius, else unlocked.
 *   - guard posts (the personality's guard_posts): a small guard post asked of Construction (a PlaceRoom service) beside
 *     the busiest own chokepoint none guards yet, up to [defence] max_guard_posts;
 *   - hold (plan 13 L3): with the force estimate against it (DaiRivals::Favourable) and an own chokepoint under
 *     threat (area_threat; the most threatened), a HoldArea there, a guard post asked beside it, and the
 *     personality's hold_share % of the free fighters carried to it by hand, each sent back (to its lair, then to
 *     work) after [defence] hold_turns -- another takes its place while the hold lasts; all go back when it ends;
 *   - contest (the director's Defend goals, key 1 + the rival): that rival's routes fortified first, the caps raised;
 *     the goals' status is the plan's (Met when nothing is left to place).
 *  Outposts across water (06 section 4.3) are not built. */
#ifndef DAIMON_DAIDEFENCE_H
#define DAIMON_DAIDEFENCE_H

#include "daimon_base_types.h"
#include <cstdint>
#include <vector>

#include "daimon/DaiFortify.h"
#include "daimon/DaiThreatModel.h"
#include "daimon/IDaiManager.h"

class DaiDefence : public IDaiManager {
public:
    const char *GetName() const override { return "defence"; }
    DaiDomain GetDomain() const override { return DaiDomain::Defence; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;
    /** Placements the last plan wanted, and how many traps and doors it asked the workshop for. */
    int64_t Planned() const { return m_planned; }
    int64_t Wanted() const { return m_wanted; }
private:
    struct Shunned { DaiSlabXY at; DaiTurn until = 0; };
    void standing(const DaiTurnCtx &c, DaiManagerWrites &w);
    void invasions(const DaiTurnCtx &c, DaiManagerWrites &w);
    void fortify(const DaiTurnCtx &c, DaiManagerWrites &w);
    /** The perimeter's lining (plan 13 section 32): its slabs dug and claimed, one at a time, so the imps fortify the
     *  gaps beside them -- from the start, before rivals digging for the gold reach the gaps. */
    void lining(const DaiTurnCtx &c, DaiManagerWrites &w);
    int64_t m_heart_sites_logged = -1;      /**< for the log only (not saved) */
    void locks(const DaiTurnCtx &c, DaiManagerWrites &w);
    void guard_posts(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool ask_guard_post(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiChokepoint &cp);
    void hold(const DaiTurnCtx &c, DaiManagerWrites &w);
    void release(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiThingRef &ref);
    void hazards(const DaiTurnCtx &c, DaiManagerWrites &w);
    void rival_imps(const DaiTurnCtx &c);
    bool spend(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiHazard &h, const char *trap, DaiFix importance, std::vector<uint64_t> &keep);
    bool seal(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiHazard &h);
    void ward(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiHazard &h, const char *trap);

    DaiTurn m_next = 0;
    DaiTurn m_facts_seen = 0;          /**< the last fact turn turned into an Intercept */
    std::vector<Shunned> m_shunned;     /**< slabs whose placement failed, left alone until `until` */
    int64_t m_planned = 0, m_wanted = 0;
    DaiTurn m_guard_next = 0;          /**< no guard post asked for before this */
    std::vector<DaiSlabXY> m_seals;     /**< P5.13: doors that seal a hot hazard off, kept locked while it is hot */
public:
    /** The seal doors kept locked (tests). */
    const std::vector<DaiSlabXY> &Seals() const { return m_seals; }
private:
    int64_t m_seal_door = 0;            /**< a door kind wanted for a seal (none in stock), this defence interval */
    DaiTurn m_ward_next = 0;           /**< P5.13: no ward cast before this */
    struct ImpSeen { DaiSlabXY at; int64_t seen = 0; DaiTurn last = 0; };
    std::vector<ImpSeen> m_rival_imps;  /**< P5.13: own floor where rival imps were seen, how often, when last */
    struct Guard { DaiThingRef ref; DaiTurn since = 0; };
    std::vector<Guard> m_guards;        /**< plan 13 L3: the creatures carried to the hold's guard post, and when */
    DaiSlabXY m_hold_at{ -1, -1 };      /**< the chokepoint held; (-1,-1) no hold */
    DaiTurn m_hold_until = 0;          /**< held until this turn at least: hold_turns after it was last wanted */
public:
    /** The chokepoint held (x -1: none) and its guards (tests, the log). */
    DaiSlabXY HoldAt() const { return m_hold_at; }
    int64_t Guards() const { return (int64_t)m_guards.size(); }
};

#endif
