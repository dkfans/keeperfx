/** @file DaiModeOpponent.h
 *  Daimon plays the keeper: every domain runs; the keeper carries PlaF_Daimon and counts as a computer player. A seat
 *  Daimon took over (10 section 8, P8.8) keeps the lieutenant to put back when the agent claims the seat again, saved
 *  with the mode (14 F2). */
#ifndef DAIMON_DAIMODEOPPONENT_H
#define DAIMON_DAIMODEOPPONENT_H

#include <cstdint>
#include <string>
#include <vector>

#include "daimon/DaiDirectives.h"
#include "daimon/IDaiMode.h"

/** The lieutenant a takeover keeper hands back: its settings and the agent's directives. */
struct DaiSeatReturn {
    bool valid = false;
    uint32_t delegated = 0, advised = 0;
    std::string personality, difficulty;        /**< difficulty as a spec (DaiDifficulty::Spec) */
    std::vector<DaiDirective> directives;
};

class DaiModeOpponent : public IDaiMode {
public:
    DaimonMode GetMode() const override { return DaimonMode::Opponent; }
    const char *GetName() const override { return "opponent"; }
    bool DomainActive(DaiDomain d) const override { return d != DaiDomain::Count; }
    const DaiSeatReturn &SeatReturn() const { return m_return; }
    void SetSeatReturn(const DaiSeatReturn &r) { m_return = r; }
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;
private:
    DaiSeatReturn m_return;
};

#endif
