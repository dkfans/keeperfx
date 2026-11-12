/** @file DaiWorkforce.h
 *  Workforce (03 section 8.3): the keeper's imps. Every [workforce] interval turns it works out how many it needs --
 *  the work (open marks, neutral ground next to its land waiting to be claimed, mining faces twice) an imp for
 *  work_per_imp, or its land an imp for slabs_per_imp, whichever is more, within imps_min..imps_max -- and keeps one
 *  create_imp job while it has fewer: below imps_min whenever the gold covers the imp and the next wages, above it
 *  only when the imp costs at most imp_gold_share of what the forecast says may be spent (each imp costs more than
 *  the last) and the next pay day is not short. The digger is an imp, or a tunneller when that comes cheaper
 *  (DiggerPower, P9). With slap_share
 *  above 0 it keeps that share of the working imps hurried, a slap at most every slap_interval turns. */
#ifndef DAIMON_DAIWORKFORCE_H
#define DAIMON_DAIWORKFORCE_H

#include "daimon_base_types.h"
#include "daimon/IDaiManager.h"

class DaiWorkforce : public IDaiManager {
public:
    const char *GetName() const override { return "workforce"; }
    DaiDomain GetDomain() const override { return DaiDomain::Workforce; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override;
    DaiTurn RetryAfter() const { return m_retry_after; }
    int64_t Target() const { return m_target; }
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;
    /** The imp count wanted (03 section 8.3), from the work and the land. Pure, for tests. */
    static int64_t ImpTarget(const DaiTurnCtx &c, int64_t open_marks, int64_t claim_frontier, int64_t mining_faces, int64_t owned_slabs);
    /** The digger to buy (P9): Create Imp, or the tunneller summon when the personality allows it and a tunneller --
     *  its price and tunneller_paydays of its pay -- comes cheaper than the next imp, or there is no Create Imp. */
    static DaiPower DiggerPower(const DaiTurnCtx &c);
    /** A tunneller kind's pay (the first known; 0 none). */
    static int64_t TunnellerPay(const DaiKnowledge &k);
private:
    void count(const DaiTurnCtx &c);
    void slap(const DaiTurnCtx &c, DaiManagerWrites &w);
    DaiTurn m_retry_after = 0;
    DaiTurn m_next_count = 0, m_next_slap = 0;
    DaiTurn m_last_imp = 0;        /**< when its last imp was made (P5.13: the hazard brake's rebuy gap) */
    int64_t m_target = 0;
};

#endif
