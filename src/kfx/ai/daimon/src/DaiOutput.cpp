#include "pre_inc.h"
#include "daimon/DaiOutput.h"

#include "post_inc.h"

const char *DaiOutput::Submit(int64_t slot, PlayerNumber p, const KeeperVerb &v, int64_t *verb_id)
{
    (void)slot; (void)p;
    DaiOutputOp op;
    op.kind = DaiOpKind::Submit;
    op.verb_id = ++*m_next_id;
    op.verb = v;
    m_out.ops.push_back(op);
    m_submitted = true;
    *verb_id = op.verb_id;
    return nullptr;
}

const char *DaiOutput::Check(int64_t slot, PlayerNumber p, const KeeperVerb &v) const
{
    (void)slot; (void)p; (void)v;
    return nullptr;
}

int64_t DaiOutput::Results(int64_t slot, DaiActResult *out, int64_t max, int64_t since) const
{
    (void)slot; (void)out; (void)max; (void)since;
    return 0;
}

void DaiOutput::Cancel(int64_t slot, int64_t verb_id)
{
    (void)slot;
    DaiOutputOp op;
    op.kind = DaiOpKind::Cancel;
    op.verb_id = verb_id;
    m_out.ops.push_back(op);
}

void DaiOutput::CancelAll(int64_t slot)
{
    (void)slot;
    DaiOutputOp op;
    op.kind = DaiOpKind::CancelAll;
    m_out.ops.push_back(op);
}

bool DaiOutput::SaveQueue(int64_t slot, std::vector<uint8_t> *out) const
{
    (void)slot; (void)out;
    return false;
}

bool DaiOutput::LoadQueue(int64_t slot, const uint8_t *data, size_t len)
{
    (void)slot; (void)data; (void)len;
    return false;
}
