/** @file DaiEconomy.h
 *  Economy (03 section 8.1): acts on the forecast. When the next pay day is short, asks the director for Mine at the
 *  most and more KeepGold (goal-weight requests), and when the gold covers comfortable_paydays pay days withdraws them
 *  and asks for a little less Mine; counts the pay days missed anyway; asks for the treasury's size (a floor on its
 *  Build goal, 04 section 4). Phase 1 built the treasury itself; from Phase 2 Construction does (02 section 12 C8).
 *  Phase 2 (04 section 8): when the next pay day is short even so (or the imps are below [defence] hazard_imps_floor
 *  with no gold for one), asks Construction to sell slabs of a room not wanted (or bigger than wanted), never the
 *  treasury, lairs, hatcheries, portal or bridges; and asks for a treasury
 *  by a vein whose gold has far to go to the nearest one, or by the gold a dig route digs through (P9). */
#ifndef DAIMON_DAIECONOMY_H
#define DAIMON_DAIECONOMY_H

#include "daimon_base_types.h"
#include <vector>

#include "daimon/IDaiManager.h"

class DaiEconomy : public IDaiManager {
public:
    const char *GetName() const override { return "economy"; }
    DaiDomain GetDomain() const override { return DaiDomain::Economy; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;
    int64_t PaydaysMissed() const { return m_missed; }
    /** Library slabs the keeper's libraries could lose and still hold every spellbook (04 section 8). */
    static int64_t library_spare(const DaiKnowledge &k);
private:
    void payday(const DaiTurnCtx &c);
    void pressure(const DaiTurnCtx &c, DaiManagerWrites &w);
    void treasury_size(const DaiTurnCtx &c, DaiManagerWrites &w);
    void emergency(const DaiTurnCtx &c, DaiManagerWrites &w);
    void forward_treasury(const DaiTurnCtx &c, DaiManagerWrites &w);
    void fortify(const DaiTurnCtx &c, DaiManagerWrites &w);
    int64_t m_last_ttp = -1, m_gold_before = 0, m_wages_before = 0;
    int64_t m_missed = 0;
    bool m_pressing = false;            /**< the short-pay-day requests stand */
    std::vector<uint32_t> m_forward_veins;      /**< the veins a forward treasury was asked for (once each) */
    std::vector<uint32_t> m_forward_routes;     /**< the dig routes one was asked for, for the gold they dig through (once each) */
    DaiTurn m_fortify_next = 0;        /**< when it may next drop an imp to fortify a room (P9) */
    uint32_t m_sell_refused = 0;        /**< rooms (1 << DaiRoom) Construction could not sell from, this emergency */
};

#endif
