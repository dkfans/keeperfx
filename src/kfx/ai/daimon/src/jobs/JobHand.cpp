// Creatures' hand jobs (04 section 7): one trip of the hand each, through pick_up_and_drop (or give_gold), which the
// controller checks with the engine's own tests -- the hand has room, each thing may be taken, the drop is allowed and
// means what it says. move_to_job and train_creature drop creatures in a work room (the hand drop gives them its job,
// and it sticks), send_to_lair a wounded one in its lair, dismiss_creature one on the portal for good; feed_creature
// takes a chicken from a hatchery to a hungry creature; pay_creature gives one its pay by hand from the treasury;
// jail_creature puts a livid one in its prison or on its rack (discipline, 08 section 8: the intents say so).
#include "pre_inc.h"
#include "JobKinds.h"

#include <cstring>

#include "post_inc.h"

namespace {
enum class Hand : uint8_t { Move, Train, Lair, Dismiss, Feed, Pay, Jail };

const DaiCreatureLook *creature(const DaiKnowledge &k, const DaiThingRef &r)
{
    if (const DaiCreatureLook *c = k.Creature(r)) return c;
    return nullptr;
}

/** An imp carried to train, or back to work from training (P9). */
const DaiImpLook *imp_of(const DaiKnowledge &k, const DaiThingRef &r)
{
    for (const DaiImpLook &i : k.OwnImps()) if (i.ref == r) return &i;
    return nullptr;
}
bool imp_there(const DaiKnowledge &k, const DaiThingRef &r) { return imp_of(k, r) != nullptr; }

bool food_there(const DaiKnowledge &k, const DaiThingRef &r)
{
    for (const DaiFoodLook &f : k.Food()) if (f.ref == r) return true;
    return false;
}

class JobHand : public IDaiJobKind {
public:
    JobHand(const char *name, Hand hand) : m_name(name), m_hand(hand) {}
    const char *GetName() const override { return m_name; }
    DaiDomain GetDomain() const override { return DaiDomain::Creatures; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        DaiHandParams *p = std::get_if<DaiHandParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        if (j.state != DaiJobState::Ready) return j.state;
        // Creatures that went (died, left) are left out; with none left there is nothing to carry.
        std::vector<DaiThingRef> still;
        for (const DaiThingRef &r : p->things) {
            const bool there = (m_hand == Hand::Feed) ? food_there(c.know, r)
                : ((creature(c.know, r) != nullptr) || (((m_hand == Hand::Train) || (m_hand == Hand::Move)) && imp_there(c.know, r)));
            if (there) still.push_back(r);
        }
        p->things = still;
        // An imp the hand cannot take this turn (digging into a wall, landing from a drop): wait for it, not a refusal.
        for (const DaiThingRef &r : p->things) {
            const DaiImpLook *imp = imp_of(c.know, r);
            if ((imp != nullptr) && !imp->pickable) {
                j.retry_after = c.now + 8;
                return j.state;
            }
        }
        const bool target_gone = ((m_hand == Hand::Feed) || (m_hand == Hand::Pay)) && (creature(c.know, p->target) == nullptr);
        if ((m_hand != Hand::Pay) && p->things.empty()) {
            snprintf(why, why_len, "GONE");
            return DaiJobState::Failed;
        }
        if (target_gone) {
            snprintf(why, why_len, "GONE");
            return DaiJobState::Failed;
        }
        j.needs = DaiJobNeeds();
        j.needs.hand = true;
        if (!p->things.empty()) j.needs.thing = p->things[0];
        if (m_hand == Hand::Pay) j.needs.gold = c.know.Kind(creature(c.know, p->target)->model).pay;
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiHandParams *p = std::get_if<DaiHandParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        memset(verb, 0, sizeof(*verb));
        if (m_hand == Hand::Pay) {
            verb->kind = ESV_GiveGold;
            verb->has_thing = true;
            verb->thing_id = p->target.idx;
            return DaiResult::Ok;
        }
        if (p->things.empty()) {
            snprintf(why, why_len, "GONE");
            return DaiResult::Fail;
        }
        verb->kind = ESV_PickUpAndDrop;
        verb->thing_count = std::min<int64_t>((int64_t)p->things.size(), EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; i < verb->thing_count; i++) verb->thing_ids[i] = p->things[(size_t)i].idx;
        if (m_hand == Hand::Feed) {
            const DaiCreatureLook *t = creature(c.know, p->target);
            if (t == nullptr) {
                snprintf(why, why_len, "GONE");
                return DaiResult::Fail;
            }
            verb->has_pos = true;
            verb->stl_x = t->x;
            verb->stl_y = t->y;
        } else {
            verb->has_room = true;
            verb->room_id = p->room;
            verb->dismiss = (m_hand == Hand::Dismiss);
            verb->imprison = (m_hand == Hand::Jail) && !p->torture;
            verb->torture = (m_hand == Hand::Jail) && p->torture;
        }
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return false; }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Done;     // one trip
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiHandParams *p = std::get_if<DaiHandParams>(&j.p);
        const DaiHandParams v = (p != nullptr) ? *p : DaiHandParams();
        w.I64((int64_t)v.things.size());
        for (const DaiThingRef &r : v.things) { w.Ref(r); }
        w.Ref(v.target);
        w.I64(v.room);
        w.I64((int64_t)v.work);
        w.Bool(v.torture);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiHandParams p;
        const int64_t n = r.Count(EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; (i < n) && r.Ok(); i++) {
            DaiThingRef t;
            t = r.Ref();
            p.things.push_back(t);
        }
        p.target = r.Ref();
        p.room = r.Range(0, INT32_MAX);
        p.work = (DaiWork)r.Range(0, (int64_t)DaiWork::Count);
        p.torture = r.Bool();
        j.p = p;
        return r.Ok();
    }
private:
    const char *m_name;
    Hand m_hand;
};

const JobHand s_move("move_to_job", Hand::Move), s_train("train_creature", Hand::Train), s_lair("send_to_lair", Hand::Lair),
    s_dismiss("dismiss_creature", Hand::Dismiss), s_feed("feed_creature", Hand::Feed), s_pay("pay_creature", Hand::Pay),
    s_jail("jail_creature", Hand::Jail);
}

const IDaiJobKind &JobMoveToJobKind() { return s_move; }
const IDaiJobKind &JobTrainCreatureKind() { return s_train; }
const IDaiJobKind &JobSendToLairKind() { return s_lair; }
const IDaiJobKind &JobDismissCreatureKind() { return s_dismiss; }
const IDaiJobKind &JobJailCreatureKind() { return s_jail; }
const IDaiJobKind &JobFeedCreatureKind() { return s_feed; }
const IDaiJobKind &JobPayCreatureKind() { return s_pay; }
