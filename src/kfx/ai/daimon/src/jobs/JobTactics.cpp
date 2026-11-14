// Tactics' jobs (06 section 5.5), one gesture each: lift (the hand takes creatures out of harm's way, to a fight, or
// home -- pick_up_and_drop), cast (a fight rule's power on a creature or at a subtile, charged as a human holds the
// button; or a lasting power switched off), banner (Call to Arms placed or taken down), hold_audience, set_tendency and
// group (the keeper's one group formed and sent to move or attack, or released: move_creature / attack /
// release_creature, which the controller turns into the group packets for a Daimon slot). The
// controller checks each with the engine's own rules when it runs.
#include "pre_inc.h"
#include "JobKinds.h"

#include <cstring>

#include "post_inc.h"

namespace {
/** One of the keeper's creatures or imps, or an enemy it sees: still there. */
bool known(const DaiKnowledge &k, const DaiThingRef &r)
{
    if (k.Creature(r) != nullptr) return true;
    for (const DaiImpLook &i : k.OwnImps()) if (i.ref == r) return true;
    for (const DaiThingLook &e : k.Enemies()) if (e.ref == r) return true;
    const DaiCustodyLook &cu = k.Custody();
    for (int64_t i = 0; i < cu.prisoners; i++) if (cu.prisoner[i].ref == r) return true;    // Custody's heals
    return false;
}

bool orderable(const DaiKnowledge &k, const DaiThingRef &r)
{
    if (const DaiCreatureLook *c = k.Creature(r)) return c->orderable;
    return true;
}


class JobTacticsLift : public IDaiJobKind {
public:
    const char *GetName() const override { return "tactics_lift"; }
    DaiDomain GetDomain() const override { return DaiDomain::Tactics; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        DaiLiftParams *p = std::get_if<DaiLiftParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        if (j.state != DaiJobState::Ready) return j.state;
        std::vector<DaiThingRef> still;
        for (const DaiThingRef &r : p->things) if (known(c.know, r)) still.push_back(r);
        p->things = still;
        if (p->things.empty()) {
            snprintf(why, why_len, "GONE");
            return DaiJobState::Failed;
        }
        j.needs = DaiJobNeeds();
        j.needs.hand = true;
        j.needs.thing = p->things[0];
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        (void)c;
        const DaiLiftParams *p = std::get_if<DaiLiftParams>(&j.p);
        if ((p == nullptr) || p->things.empty()) {
            snprintf(why, why_len, "GONE");
            return DaiResult::Fail;
        }
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_PickUpAndDrop;
        verb->thing_count = std::min<int64_t>((int64_t)p->things.size(), EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; i < verb->thing_count; i++) verb->thing_ids[i] = p->things[(size_t)i].idx;
        if (p->room > 0) {
            verb->has_room = true;
            verb->room_id = p->room;
        } else {
            verb->has_pos = true;
            verb->stl_x = p->x;
            verb->stl_y = p->y;
        }
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return false; }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiLiftParams *p = std::get_if<DaiLiftParams>(&j.p);
        const DaiLiftParams v = (p != nullptr) ? *p : DaiLiftParams();
        w.I64((int64_t)v.things.size());
        for (const DaiThingRef &r : v.things) w.Ref(r);
        w.I64(v.x); w.I64(v.y); w.I64(v.room);
        w.I64(v.step); w.I64(v.slot);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiLiftParams p;
        const int64_t n = r.Count(EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; (i < n) && r.Ok(); i++) p.things.push_back(r.Ref());
        p.x = (DaiSubtlCoord)r.Range(0, 1 << 16); p.y = (DaiSubtlCoord)r.Range(0, 1 << 16);
        p.room = r.Range(0, INT32_MAX);
        p.step = (uint8_t)r.Range(0, 255);
        p.slot = (int8_t)r.Range(-1, 127);
        j.p = p;
        return r.Ok();
    }
};

class JobTacticsCast : public IDaiJobKind {
public:
    JobTacticsCast(const char *name, DaiDomain domain = DaiDomain::Tactics) : m_name(name), m_domain(domain) {}
    const char *GetName() const override { return m_name; }
    DaiDomain GetDomain() const override { return m_domain; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiCastParams *p = std::get_if<DaiCastParams>(&j.p);
        if ((p == nullptr) || ((p->power >= DaiPower::Count) && (c.know.Content().Find(DaiContentClass::Power, p->kind) == nullptr))) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        if (j.state != DaiJobState::Ready) return j.state;
        if ((p->target.idx != 0) && !known(c.know, p->target)) {
            snprintf(why, why_len, "GONE");
            return DaiJobState::Failed;
        }
        j.needs = DaiJobNeeds();
        j.needs.gold = p->off ? 0 : p->price;
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiCastParams *p = std::get_if<DaiCastParams>(&j.p);
        const DaiContentEntry *own = (p != nullptr) ? c.know.Content().Find(DaiContentClass::Power, p->kind) : nullptr;
        if ((p == nullptr) || ((p->power >= DaiPower::Count) && (own == nullptr))) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        memset(verb, 0, sizeof(*verb));
        snprintf(verb->name, sizeof(verb->name), "%s", (p->power < DaiPower::Count) ? DaiPowerCode(p->power) : own->name);
        if (p->off) {
            verb->kind = ESV_PowerOff;
            return DaiResult::Ok;
        }
        verb->kind = ESV_CastPower;
        verb->overcharge_turns = 4 * p->level;      // a level for every four turns the button is held
        if (p->x >= 0) {                            // an area power: at the target's subtile, as the assistant casts it
            verb->has_pos = true;
            verb->stl_x = p->x;
            verb->stl_y = p->y;
        } else if (p->target.idx != 0) {
            verb->has_thing = true;
            verb->thing_id = p->target.idx;
        } else {
            verb->overcharge_turns = 0;             // a whole-level power: no target, no charge
        }
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return false; }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiCastParams *p = std::get_if<DaiCastParams>(&j.p);
        const DaiCastParams v = (p != nullptr) ? *p : DaiCastParams();
        w.I64((int64_t)v.power);
        w.I64((int64_t)v.kind);
        w.I64(v.level);
        w.Ref(v.target);
        w.I64(v.x); w.I64(v.y);
        w.I64(v.slot);
        w.I64(v.price);
        w.Bool(v.off);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiCastParams p;
        p.power = (DaiPower)r.Range(0, (int64_t)DaiPower::Count);
        p.kind = (DaiPowerKind)r.Range(0, 1999);
        p.level = r.Range(0, 8);
        p.target = r.Ref();
        p.x = (DaiSubtlCoord)r.Range(-1, 1 << 16); p.y = (DaiSubtlCoord)r.Range(-1, 1 << 16);
        p.slot = r.Range(-1, 63);
        p.price = r.Range(0, INT64_MAX);
        p.off = r.Bool();
        j.p = p;
        return r.Ok();
    }
private:
    const char *m_name;
    DaiDomain m_domain;
};

class JobTacticsBanner : public IDaiJobKind {
public:
    const char *GetName() const override { return "tactics_banner"; }
    DaiDomain GetDomain() const override { return DaiDomain::Tactics; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        (void)c;
        if (std::get_if<DaiBannerParams>(&j.p) == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        (void)c;
        const DaiBannerParams *p = std::get_if<DaiBannerParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        memset(verb, 0, sizeof(*verb));
        snprintf(verb->name, sizeof(verb->name), "%s", DaiPowerCode(DaiPower::CallToArms));
        if (p->off) {
            verb->kind = ESV_PowerOff;
        } else {
            verb->kind = ESV_CastPower;
            verb->has_pos = true;
            verb->stl_x = p->x;
            verb->stl_y = p->y;
        }
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return false; }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiBannerParams *p = std::get_if<DaiBannerParams>(&j.p);
        const DaiBannerParams v = (p != nullptr) ? *p : DaiBannerParams();
        w.Bool(v.off); w.I64(v.x); w.I64(v.y);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiBannerParams p;
        p.off = r.Bool();
        p.x = (DaiSubtlCoord)r.Range(0, 1 << 16); p.y = (DaiSubtlCoord)r.Range(0, 1 << 16);
        j.p = p;
        return r.Ok();
    }
};

class JobTacticsGroup : public IDaiJobKind {
public:
    const char *GetName() const override { return "tactics_group"; }
    DaiDomain GetDomain() const override { return DaiDomain::Tactics; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        DaiGroupParams *p = std::get_if<DaiGroupParams>(&j.p);
        if ((p == nullptr) || (p->order >= DaiGroupOrder::Count)) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        if ((j.state != DaiJobState::Ready) || (p->order == DaiGroupOrder::Release)) return j.state;
        // Gone members leave, and so do those the Order mode would refuse now (hungry, owed pay): one refuses the whole order.
        std::vector<DaiThingRef> still;
        for (const DaiThingRef &r : p->members) if (known(c.know, r) && orderable(c.know, r)) still.push_back(r);
        p->members = still;
        if (p->members.empty()) {
            snprintf(why, why_len, "GONE");
            return DaiJobState::Failed;
        }
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiGroupParams *p = std::get_if<DaiGroupParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        memset(verb, 0, sizeof(*verb));
        verb->has_thing = true;
        if (p->order == DaiGroupOrder::Release) {
            if (c.know.Own().group_count <= 0) return DaiResult::Done;
            verb->kind = ESV_ReleaseCreature;
            verb->thing_id = c.know.Own().group_leader;
            return DaiResult::Ok;
        }
        if (p->members.empty()) {
            snprintf(why, why_len, "GONE");
            return DaiResult::Fail;
        }
        verb->kind = (p->order == DaiGroupOrder::Attack) ? ESV_Attack : ESV_MoveCreature;
        verb->thing_id = p->members[0].idx;
        verb->thing_count = std::min<int64_t>((int64_t)p->members.size() - 1, EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; i < verb->thing_count; i++) verb->thing_ids[i] = p->members[(size_t)i + 1].idx;
        verb->has_pos = true;
        verb->stl_x = p->x;
        verb->stl_y = p->y;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return false; }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiGroupParams *p = std::get_if<DaiGroupParams>(&j.p);
        const DaiGroupParams v = (p != nullptr) ? *p : DaiGroupParams();
        w.I64((int64_t)v.members.size());
        for (const DaiThingRef &r : v.members) w.Ref(r);
        w.I64((int64_t)v.order);
        w.I64(v.x); w.I64(v.y);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiGroupParams p;
        const int64_t n = r.Count(1 + EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; (i < n) && r.Ok(); i++) p.members.push_back(r.Ref());
        p.order = (DaiGroupOrder)r.Range(0, (int64_t)DaiGroupOrder::Count - 1);
        p.x = (DaiSubtlCoord)r.Range(0, 1 << 16); p.y = (DaiSubtlCoord)r.Range(0, 1 << 16);
        j.p = p;
        return r.Ok();
    }
};

/** A tendency set: one panel button, done when it reads so. Without params, the flee tendency to the personality's
 *  flee_tendency (Tactics); with them, the tendency named (Custody's imprison, 08 section 6). */
class JobSetTendency : public IDaiJobKind {
public:
    const char *GetName() const override { return "set_tendency"; }
    DaiDomain GetDomain() const override { return DaiDomain::Tactics; }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        (void)why; (void)why_len;
        if (Achieved(c, j)) return DaiResult::Done;
        const DaiTendencyParams t = wanted(c, j);
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_SetTendency;
        snprintf(verb->name, sizeof(verb->name), "%s", (t.which == DaiTendency::Imprison) ? "imprison" : "flee");
        verb->has_enabled = true;
        verb->enabled = t.on;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiTendencyParams t = wanted(c, j);
        return ((t.which == DaiTendency::Imprison) ? c.know.Own().imprison_tendency : c.know.Own().flee_tendency) == t.on;
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiTendencyParams *p = std::get_if<DaiTendencyParams>(&j.p);
        w.Bool(p != nullptr);
        if (p == nullptr) return;
        w.I64((int64_t)p->which);
        w.Bool(p->on);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        if (!r.Bool()) {
            j.p = std::monostate{};
            return r.Ok();
        }
        DaiTendencyParams p;
        p.which = (DaiTendency)r.Range(0, (int64_t)DaiTendency::Count - 1);
        p.on = r.Bool();
        j.p = p;
        return r.Ok();
    }
private:
    static DaiTendencyParams wanted(const DaiTurnCtx &c, const DaiJob &j)
    {
        if (const DaiTendencyParams *p = std::get_if<DaiTendencyParams>(&j.p)) return *p;
        DaiTendencyParams t;
        t.on = (c.personality.tactics_flee_tendency != 0);
        return t;
    }
};
}

const IDaiJobKind &JobTacticsLiftKind() { static const JobTacticsLift k; return k; }
const IDaiJobKind &JobTacticsCastKind() { static const JobTacticsCast k("tactics_cast"); return k; }
const IDaiJobKind &JobTacticsBannerKind() { static const JobTacticsBanner k; return k; }
const IDaiJobKind &JobHoldAudienceKind() { static const JobTacticsCast k("hold_audience"); return k; }
const IDaiJobKind &JobSetTendencyKind() { static const JobSetTendency k; return k; }
const IDaiJobKind &JobTacticsGroupKind() { static const JobTacticsGroup k; return k; }
const IDaiJobKind &JobMagicCastKind() { static const JobTacticsCast k("magic_cast", DaiDomain::Magic); return k; }
