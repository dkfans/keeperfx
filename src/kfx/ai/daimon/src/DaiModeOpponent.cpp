#include "pre_inc.h"
#include "daimon/DaiModeOpponent.h"

#include "daimon_assist_types.h"
#include "post_inc.h"

void DaiModeOpponent::Save(DaiBlobWriter &w) const
{
    w.Bool(m_return.valid);
    if (!m_return.valid) return;
    w.U64(m_return.delegated);
    w.U64(m_return.advised);
    w.Str(m_return.personality.c_str());
    w.Str(m_return.difficulty.c_str());
    DaiDirectives::Save(m_return.directives, w);
}

bool DaiModeOpponent::Load(DaiBlobReader &r)
{
    m_return = DaiSeatReturn();
    m_return.valid = r.Bool();
    if (!m_return.valid) return r.Ok();
    m_return.delegated = (uint32_t)r.Range(0, DAI_ASSIST_ALL);
    m_return.advised = (uint32_t)r.Range(0, DAI_ASSIST_ALL);
    m_return.personality = r.Str();
    m_return.difficulty = r.Str();
    return DaiDirectives::Load(&m_return.directives, r) && r.Ok();
}
