/** @file DaiModeLieutenant.h
 *  An External seat's lieutenant (docs/plans/ai-plans/v2/10, Daimon Phase 8): the agent playing the seat hands domains
 *  to Daimon -- delegated: Daimon's managers for them plan and act; off: they do not run. The keeper stays the seat's
 *  (no PlaF_Daimon); Daimon writes its gestures into the seat's own packet slot through the controller's lieutenant
 *  lane, below the agent's, which cuts them (keeper_ctl.h). The domains are the assistant's (daimon_assist_types.h).
 *  Advised domains plan without acting (proposals, P8.5); the agent's directives (DaiDirectives) stand as goals; what the
 *  agent touched is leased to it (P8.3). */
#ifndef DAIMON_DAIMODELIEUTENANT_H
#define DAIMON_DAIMODELIEUTENANT_H

#include <cstdint>
#include <set>
#include <vector>

#include "daimon_assist_types.h"
#include "daimon/DaiDirectives.h"
#include "daimon/DaiGoals.h"
#include "daimon/IDaiMode.h"

class DaiModeLieutenant : public IDaiMode {
public:
    explicit DaiModeLieutenant(uint32_t delegated) : m_delegated(delegated & DAI_ASSIST_ALL) {}
    DaimonMode GetMode() const override { return DaimonMode::Lieutenant; }
    const char *GetName() const override { return "lieutenant"; }
    bool DomainActive(DaiDomain d) const override;
    /** An advised domain's jobs wait for the agent's accept (Tactics' never: fights do not wait). */
    bool HoldsJob(const DaiJob &j) const override;
    DaiFightCasting FightCasting(const DaiTurnCtx &c) const override;
    /** What the agent touched (its orders, triggers and hand rules: the seat's own packets) is the agent's for
     *  [lieutenant] agent_lease_turns, its slab decisions stand (10 section 7.2): the assistant's rules, the agent's codes. */
    const char *Refuses(const DaiTurnCtx &c, const KeeperVerb &v) const override;
    uint32_t Delegated() const { return m_delegated; }
    void SetDelegated(uint32_t d) { m_delegated = d & DAI_ASSIST_ALL; m_advise &= ~m_delegated; }
    /** Advise mode (P8.5): the domains whose managers plan but whose jobs are proposals until accepted. */
    uint32_t Advised() const { return m_advise; }
    void SetAdvised(uint32_t a) { m_advise = a & DAI_ASSIST_ALL & ~m_delegated; }
    /** The agent accepted the proposal (a held job's id): it runs. */
    void Accept(uint32_t job_id) { m_accepted.insert(job_id); }
    bool Accepted(uint32_t job_id) const { return m_accepted.count(job_id) != 0; }
    std::set<uint32_t> &AcceptedIds() { return m_accepted; }
    /** On takeover (the agent lost, 10 section 8): Daimon plays the keeper (true, the default) or the built-in AI does. */
    bool TakeoverDaimon() const { return m_takeover_daimon; }
    void SetTakeoverDaimon(bool d) { m_takeover_daimon = d; }
    const std::set<uint32_t> &AcceptedIds() const { return m_accepted; }
    /** A manager's domain as a delegation bit (Magic and Tactics: 0, they serve the others). */
    static uint32_t DomainBit(DaiDomain d);
    /** The agent's directives, by id (at most MAX_DIRECTIVES). */
    static constexpr size_t MAX_DIRECTIVES = DaiDirectives::MAX;
    std::vector<DaiDirective> &Directives() { return m_directives; }
    const std::vector<DaiDirective> &Directives() const { return m_directives; }
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;
private:
    uint32_t m_delegated;
    uint32_t m_advise = 0;
    bool m_takeover_daimon = true;
    std::set<uint32_t> m_accepted;
    std::vector<DaiDirective> m_directives;
};

#endif
