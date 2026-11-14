// Offence's diplomacy jobs (07 section 5a): set_alliance -- the keeper's declaration towards another player set on or
// off, the alliance button's one-way toggle (mutual needs both; the controller refuses on an ALLIANCE_LOCKED level),
// done when the panel reads so; send_message -- a short message to every player, so a human notices a proposal.
#include "pre_inc.h"
#include "JobKinds.h"

#include <cstring>

#include "post_inc.h"

namespace {
void save_params(const DaiJob &j, DaiBlobWriter &w)
{
    const DaiDiplomacyParams *p = std::get_if<DaiDiplomacyParams>(&j.p);
    const DaiDiplomacyParams v = (p != nullptr) ? *p : DaiDiplomacyParams();
    w.I64(v.other);
    w.Bool(v.enabled);
    w.Str(v.message);
}

bool load_params(DaiJob &j, DaiBlobReader &r)
{
    DaiDiplomacyParams p;
    p.other = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
    p.enabled = r.Bool();
    snprintf(p.message, sizeof(p.message), "%s", r.Str().c_str());
    j.p = p;
    return r.Ok();
}

class JobSetAlliance : public IDaiJobKind {
public:
    const char *GetName() const override { return "set_alliance"; }
    DaiDomain GetDomain() const override { return DaiDomain::Offence; }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiDiplomacyParams *p = std::get_if<DaiDiplomacyParams>(&j.p);
        if ((p == nullptr) || (p->other < 0)) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        if (Achieved(c, j)) return DaiResult::Done;
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_SetAlliance;
        verb->has_target_player = true;
        verb->target_player = p->other;
        verb->has_enabled = true;
        verb->enabled = p->enabled;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiDiplomacyParams *p = std::get_if<DaiDiplomacyParams>(&j.p);
        return (p != nullptr) && (c.know.Alliance(p->other).ours == p->enabled);
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Running;  // the panel reads so at the next update
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override { save_params(j, w); }
    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override { return load_params(j, r); }
};

class JobSendMessage : public IDaiJobKind {
public:
    const char *GetName() const override { return "send_message"; }
    DaiDomain GetDomain() const override { return DaiDomain::Offence; }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        (void)c;
        const DaiDiplomacyParams *p = std::get_if<DaiDiplomacyParams>(&j.p);
        if ((p == nullptr) || (p->message[0] == '\0')) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_SendMessage;
        snprintf(verb->message, sizeof(verb->message), "%s", p->message);
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return false; }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override { save_params(j, w); }
    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override { return load_params(j, r); }
};

const JobSetAlliance s_set_alliance;
const JobSendMessage s_send_message;
}

const IDaiJobKind &JobSetAllianceKind() { return s_set_alliance; }
const IDaiJobKind &JobSendMessageKind() { return s_send_message; }
