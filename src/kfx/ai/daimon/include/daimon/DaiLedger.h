/** @file DaiLedger.h
 *  Claims (02 section 4.7, 01 section 8.3): what a keeper's running gestures hold -- gold they are about to spend, the
 *  hand, a thing, an area -- so two jobs never plan on the same gold or pick up the same creature. Phase 0 uses Gold,
 *  Thing, Hand and Area; Group, RoomSpace and ImpCapacity come with the phases that need them. A claim ends with its
 *  job's gesture or the job, at its expiry, or in ReleaseAll(), the one exit every mode change, defeat, level end and
 *  hand-back takes (lesson L8). */
#ifndef DAIMON_DAILEDGER_H
#define DAIMON_DAILEDGER_H

#include <cstdint>

#include "globals.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiPool.h"
#include "daimon/DaiTypes.h"

/** Group, RoomSpace and ImpCapacity are never claimed (02's design; the managers plan room space and imps themselves),
 *  and the owners Human and Agent are never written (leases are not claims: DaiLeases, IDaiMode::Refuses): their numbers
 *  stay reserved, claims are saved (14 C4). */
enum class DaiClaimKind : uint8_t { Gold, Thing, Hand, Area, Group, RoomSpace, ImpCapacity };
enum class DaiClaimOwner : uint8_t { Job, Human, Agent };

struct DaiClaim {
    uint32_t id = 0;
    DaiClaimKind kind = DaiClaimKind::Gold;
    DaiClaimOwner owner = DaiClaimOwner::Job;
    uint32_t job_id = 0;        /**< 0 unless owner == Job */
    int64_t amount = 0;         /**< Gold */
    DaiThingRef thing;          /**< Thing */
    DaiRect area;               /**< Area */
    bool discretionary = false; /**< may be dropped first under a gold emergency (Phase 1) */
    DaiTurn expires = 0;       /**< 0: until released */
};

class DaiLedger {
public:
    /** 0 if the gold does not fit in what is unclaimed of `gold_now` (claims never exceed the keeper's gold). */
    uint32_t ClaimGold(uint32_t job, int64_t amount, int64_t gold_now, DaiTurn expires, bool discretionary = false);
    /** 0 if the thing is claimed already (exclusive per thing). */
    uint32_t ClaimThing(DaiClaimOwner o, uint32_t job, const DaiThingRef &t, DaiTurn expires);
    /** 0 if the hand is claimed already (exclusive). */
    uint32_t ClaimHand(uint32_t job, DaiTurn expires);
    /** 0 if it overlaps a claimed area. */
    uint32_t ClaimArea(DaiClaimOwner o, uint32_t job, const DaiRect &r, DaiTurn expires);

    int64_t GoldClaimed() const;
    /** gold_now less the active gold claims (never below 0). */
    int64_t GoldUnclaimed(int64_t gold_now) const;
    bool ThingFree(const DaiThingRef &t) const;
    bool HandFree() const;
    bool AreaFree(const DaiRect &r) const;

    void ReleaseJob(uint32_t job);
    void ReleaseAll();
    void Expire(DaiTurn now);
    int64_t Count() const { return m_claims.Count(); }
    template <typename F> void ForEach(F f) const { m_claims.ForEach(f); }
private:
    friend class DaiSave;
    uint32_t add(DaiClaimKind k, DaiClaimOwner o, uint32_t job, DaiTurn expires, DaiClaim **out);
    DaiPool<DaiClaim, DAI_MAX_CLAIMS> m_claims;
};

#endif
