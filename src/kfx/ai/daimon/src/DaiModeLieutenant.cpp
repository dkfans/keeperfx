#include "pre_inc.h"
#include "daimon/DaiModeLieutenant.h"

#include "daimon/DaiJob.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiModeAssistant.h"
#include "daimon/DaiTurnCtx.h"
#include "post_inc.h"

bool DaiModeLieutenant::DomainActive(DaiDomain d) const
{
    return DaiModeAssistant::DomainActiveFor(m_delegated | m_advise, d);
}

uint32_t DaiModeLieutenant::DomainBit(DaiDomain d)
{
    return DaiModeAssistant::DomainBit(d);
}

bool DaiModeLieutenant::HoldsJob(const DaiJob &j) const
{
    return ((DomainBit(j.domain) & m_advise) != 0) && !Accepted(j.id);
}

const char *DaiModeLieutenant::Refuses(const DaiTurnCtx &c, const KeeperVerb &v) const
{
    return DaiModeAssistant::RefusesFor(c, v, c.cfg.lieutenant_agent_lease_turns, true);
}

DaiFightCasting DaiModeLieutenant::FightCasting(const DaiTurnCtx &c) const
{
    // The seat's combat-assistant rules are the agent's: while they are on, nothing is cast twice -- Tactics runs them
    // only once they are handed over (P8.7); with them off, the personality's rules for a delegated defence.
    const DaiHumanFightRules &h = c.know.HumanFightRules();
    return DaiModeAssistant::CastingFor(m_delegated, h.enabled, h.delegated);
}

void DaiModeLieutenant::Save(DaiBlobWriter &w) const
{
    w.U64(m_delegated);
    w.U64(m_advise);
    w.Bool(m_takeover_daimon);
    w.I64((int64_t)m_accepted.size());
    for (uint32_t id : m_accepted) w.U32(id);
    DaiDirectives::Save(m_directives, w);
}

bool DaiModeLieutenant::Load(DaiBlobReader &r)
{
    m_delegated = (uint32_t)r.Range(0, DAI_ASSIST_ALL);
    SetAdvised((uint32_t)r.Range(0, DAI_ASSIST_ALL));
    m_takeover_daimon = r.Bool();
    m_accepted.clear();
    const int64_t na = r.Count(DAI_MAX_JOBS);
    for (int64_t i = 0; (i < na) && r.Ok(); i++) m_accepted.insert(r.U32());
    return DaiDirectives::Load(&m_directives, r) && r.Ok();
}
