// use_special (P9): a dungeon special box the keeper's hand can reach, used as a human uses it -- picked up, and for
// Resurrect with the dead creature chosen. Done when the box is gone from what the hand can reach.
#include "pre_inc.h"
#include "JobKinds.h"

#include <climits>
#include <cstring>

#include "post_inc.h"

namespace {
bool box_there(const DaiTurnCtx &c, const DaiThingRef &box)
{
    for (const DaiSpecialLook &b : c.know.Specials()) if (b.ref == box) return true;
    return false;
}

class JobUseSpecial : public IDaiJobKind {
public:
    const char *GetName() const override { return "use_special"; }
    DaiDomain GetDomain() const override { return DaiDomain::Magic; }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiSpecialParams *p = std::get_if<DaiSpecialParams>(&j.p);
        if ((p == nullptr) || (p->box.idx == 0)) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        if (Achieved(c, j)) return DaiResult::Done;
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_UseSpecial;
        verb->has_thing = true;
        verb->thing_id = p->box.idx;
        if (p->kind == DaiSpecial::Resurrect) {
            snprintf(verb->name, sizeof(verb->name), "%s", p->code);
            verb->level = p->level;
        }
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiSpecialParams *p = std::get_if<DaiSpecialParams>(&j.p);
        return (p != nullptr) && !box_there(c, p->box);
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Running;  // the box goes at the next update
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiSpecialParams *p = std::get_if<DaiSpecialParams>(&j.p);
        const DaiSpecialParams v = (p != nullptr) ? *p : DaiSpecialParams();
        w.Ref(v.box);
        w.I64((int64_t)v.kind);
        w.I64(v.model);
        w.Str(v.code);
        w.I64(v.level);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiSpecialParams p;
        p.box = r.Ref();
        p.kind = (DaiSpecial)r.Range(0, (int64_t)DaiSpecial::Count - 1);
        p.model = (DaiThingModel)r.Range(0, DAI_MAX_KINDS - 1);
        snprintf(p.code, sizeof(p.code), "%s", r.Str().c_str());
        p.level = r.Range(0, 10);
        j.p = p;
        return r.Ok();
    }
};

const JobUseSpecial s_use_special;
}

const IDaiJobKind &JobUseSpecialKind()
{
    return s_use_special;
}
