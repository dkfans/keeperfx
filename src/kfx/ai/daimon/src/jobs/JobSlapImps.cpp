// slap_imps (03 section 8.3, Q2: off until measured): one slap of a working imp that is not still running from the
// last one -- a hand gesture, so it competes for the turn. Workforce spaces the jobs (slap_interval).
#include "pre_inc.h"
#include "JobKinds.h"

#include <cstring>

#include "post_inc.h"

bool JobSlapAim(const DaiKnowledge &know, DaiJob &j)
{
    for (const DaiImpLook &i : know.OwnImps()) {
        if (!i.working || i.sped_up) continue;
        j.p = DaiSlapParams{ i.ref };
        j.needs = DaiJobNeeds();
        j.needs.hand = true;
        j.needs.thing = i.ref;
        return true;
    }
    return false;
}

namespace {

class JobSlapImps : public IDaiJobKind {
public:
    const char *GetName() const override { return "slap_imps"; }
    DaiDomain GetDomain() const override { return DaiDomain::Workforce; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        (void)why; (void)why_len;
        if (j.state != DaiJobState::Ready) return j.state;
        if (!JobSlapAim(c.know, j)) return DaiJobState::Done;    // nobody to hurry
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        (void)c;
        const DaiSlapParams *p = std::get_if<DaiSlapParams>(&j.p);
        if ((p == nullptr) || (p->imp.idx == 0)) {
            snprintf(why, why_len, "NO_IMP");
            return DaiResult::Fail;
        }
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_Slap;
        verb->has_thing = true;
        verb->thing_id = p->imp.idx;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        (void)c; (void)j;
        return 0;
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        (void)c; (void)j;
        return false;                   // done when its slap ran (OnResult)
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiSlapParams *p = std::get_if<DaiSlapParams>(&j.p);
        w.I64((p != nullptr) ? p->imp.idx : 0);
        w.U64((p != nullptr) ? p->imp.creation_turn : 0);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiSlapParams p;
        p.imp = r.Ref();
        j.p = p;
        return r.Ok();
    }
};

const JobSlapImps s_slap_imps;
}

const IDaiJobKind &JobSlapImpsKind()
{
    return s_slap_imps;
}
