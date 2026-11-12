/** @file DaiModeAssistant.h
 *  The human's assistant (docs/plans/ai-plans/v2/09 section 5, Daimon P7.12): Daimon plays the domains the human chose
 *  for their own keeper, never PlaF_Daimon (the keeper stays the human's), writing self-contained gestures into the
 *  human's packet only on turns it leaves free. It never touches what the human is doing now (the live exclusion:
 *  their possessed creature and its group, their hand, the thing under their cursor, nor casts or drops by the
 *  possessed creature), what they touched lately (a creature's lease) or a slab decision they made (dig, room, trap,
 *  door): Refuses, asked by the arbiter for every gesture. Presets: builder, steward, defender, raider. */
#ifndef DAIMON_DAIMODEASSISTANT_H
#define DAIMON_DAIMODEASSISTANT_H

#include <cstdint>

#include "daimon_assist_types.h"
#include "daimon/IDaiMode.h"

class DaiModeAssistant : public IDaiMode {
public:
    explicit DaiModeAssistant(uint32_t domains) : m_domains(domains) {}
    DaimonMode GetMode() const override { return DaimonMode::Assistant; }
    const char *GetName() const override { return "assistant"; }
    bool DomainActive(DaiDomain d) const override;
    /** A domain mask's managers (the lieutenant's delegation uses the same, 10 section 3). */
    static bool DomainActiveFor(uint32_t domains, DaiDomain d);
    /** A manager's domain as its delegation bit (Magic and Tactics: 0, they serve the others). */
    static uint32_t DomainBit(DaiDomain d);
    const char *Refuses(const DaiTurnCtx &c, const KeeperVerb &v) const override;
    /** The refusals shared with the lieutenant (P8.3): the live exclusion, leases of `lease` turns, the standing slab
     *  decisions -- the agent's codes (LEASED_BY_AGENT, AGENT_*) when `agent`. */
    static const char *RefusesFor(const DaiTurnCtx &c, const KeeperVerb &v, int64_t lease, bool agent);
    DaiFightCasting FightCasting(const DaiTurnCtx &c) const override;
    uint32_t Domains() const { return m_domains; }
    /** The coexistence with the human's combat assistant (09 section 5.4): with defence chosen and their assistant on,
     *  Daimon runs its rules -- once delegated (PckA_AssistDelegate), none until then, so nothing is cast twice; with
     *  defence and their assistant off, the personality's; without defence, none (their runner casts as before). */
    static DaiFightCasting CastingFor(uint32_t domains, bool human_on, bool delegated);
    /** Whether the keeper's combat-assistant rules should be delegated to Daimon (the flag Daimon keeps in step). */
    static bool WantsDelegate(uint32_t domains, bool human_on) { return ((domains & DAI_ASSIST_DEFENCE) != 0) && human_on; }
    void SetDomains(uint32_t d) { m_domains = d; }
    void Save(DaiBlobWriter &w) const override { w.U64(m_domains); }
    bool Load(DaiBlobReader &r) override { m_domains = (uint32_t)r.Range(0, DAI_ASSIST_ALL); return r.Ok(); }
    /** A preset's domains and personality (09 section 5.1; 02 section 12 C24; daimon_assist_types.h's table): builder,
     *  steward, defender, raider. False for another name. */
    static bool Preset(const char *name, uint32_t *domains, const char **personality);
private:
    uint32_t m_domains;
};

#endif
