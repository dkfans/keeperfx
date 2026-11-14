#include "pre_inc.h"
#include "daimon/DaiCreatures.h"
#include "daimon/DaiGeom.h"
#include "daimon/DaiCustody.h"
#include "daimon/DaiDirector.h"
#include "daimon/DaiLog.h"

#include <cinttypes>
#include "daimon/DaiDifficulty.h"

#include <algorithm>

#include "post_inc.h"

namespace {
constexpr uint64_t KEY_HAND = 1;        /**< every hand job shares one key: one at a time */

bool wounded(const DaiTurnCtx &c, const DaiCreatureLook &cr)
{
    return cr.health_pct * 256 < c.know.Kind(cr.model).heal_requirement * 100;
}

int64_t slabs_apart(DaiSubtlCoord ax, DaiSubtlCoord ay, DaiSlabCoord bx, DaiSlabCoord by)
{
    return std::abs(DaiSubtileSlab(ax) - bx) + std::abs(DaiSubtileSlab(ay) - by);
}

bool hand_busy(const DaiTurnCtx &c)
{
    bool busy = false;
    c.jobs.ForEach([&](const DaiJob &j) { busy = busy || ((j.domain == DaiDomain::Creatures) && !DaiJobEnded(j.state)); });
    return busy;
}

int64_t skill(const DaiKindLook &k, DaiWork w)
{
    return (w == DaiWork::Research) ? k.research_value : (w == DaiWork::Manufacture) ? k.manufacture_value : k.training_value;
}
}

const DaiCreatures::Note *DaiCreatures::NoteOf(const DaiThingRef &r) const
{
    for (const Note &n : m_notes) if (n.ref == r) return &n;
    return nullptr;
}

DaiCreatures::Note *DaiCreatures::note(const DaiThingRef &r)
{
    for (Note &n : m_notes) if (n.ref == r) return &n;
    return nullptr;
}

int64_t DaiCreatures::WorkValue(const DaiTurnCtx &c, const DaiCreatureLook &cr, DaiWork w, const DaiRoomLook &room)
{
    const DaiKindLook &k = c.know.Kind(cr.model);
    if (!k.known || k.digger || k.refuses[(size_t)w] || (room.capacity <= room.used)) return 0;
    // Its skill there times the room's efficiency (of 256), less 64 a slab of the walk. Whole skill points once lost
    // most of it: at 62% efficiency a skill of 1 was worth nothing and one of 2 lost to four slabs of walk (P9).
    int64_t v = skill(k, w) * std::max<int64_t>(room.efficiency, 1);
    if (w == DaiWork::Research) {
        // While a room is still to be researched, times research_rooms_weight: training must not stall the rooms early
        // (P9).
        const DaiResearchLook &rs = c.know.Research();
        bool rooms = false;
        for (int64_t i = 0; (i < rs.count) && (i < DaiResearchLook::MAX); i++) rooms = rooms || (rs.items[i].kind == DaiResearchKind::Room);
        if (rooms) v = FixToInt(FixMul(FixFromInt(v), c.personality.creatures_research_rooms_weight));
    }
    if (w == DaiWork::Train) {
        // Times train_weight, and more by the share of its fight its next two levels' abilities add (DaiAbilities),
        // times train_ability_weight.
        const DaiFix gain = k.ability_gain[std::clamp<int64_t>(cr.level - 1, 0, DaiKindLook::LEVELS - 1)];
        const DaiFix weight = FixMul(c.personality.creatures_train_weight, DAI_FIX_ONE + FixMul(gain, c.personality.creatures_train_ability_weight));
        v = FixToInt(FixMul(FixFromInt(v), weight));
    }
    if (k.stressful[(size_t)w]) v /= 2;
    return v - 64 * slabs_apart(cr.x, cr.y, room.cx, room.cy);
}

void DaiCreatures::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    track(c);
    arrivals(c, w);
    if (hand_busy(c)) return;
    if (trouble(c, w)) return;
    if (make_room(c, w)) return;
    if (care(c, w)) return;
    if (scavenge(c, w)) return;
    if (work(c, w)) return;
    train_imps(c, w);
}

/** At its creature limit, its weakest free creature (worth at its level, KindWorth: trained ones count for more) on the
 *  portal, when a kind that can come is worth dismiss_upgrade more (P9: level 124's 25 places held by flies, beetles
 *  and spiders while the rival drew bile demons and dragons). Each one makes room for the next arrival only. */
bool DaiCreatures::make_room(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiPersonality &p = c.personality;
    const int64_t max = k.Research().max_creatures;
    if ((p.creatures_dismiss_weak == 0) || (max <= 0) || ((int64_t)k.Creatures().size() < max)) return false;
    if ((m_room_made != 0) && (c.now < m_room_made + (DaiTurn)p.creatures_dismiss_interval)) return false;
    if (c.goals.Find(DaiGoalKind::Forbid, DAI_FORBID_DISMISS) != nullptr) return false;     // recover
    const DaiRoomLook *portal = nullptr;
    if (portal == nullptr) portal = k.FirstRoomWith(DAI_ROLE_PORTAL);
    if (portal == nullptr) return false;
    int64_t best_come = 0;
    for (DaiThingModel m = 1; m <= (DaiThingModel)k.KindCount(); m++)
        if (DaiDirector::KindCanCome(k, m)) best_come = std::max(best_come, DaiDirector::KindWorth(k, p, m));
    const DaiCreatureLook *weakest = nullptr;
    int64_t weakest_worth = 0;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if ((cr.held_by >= 0) || (cr.battle != 0)) continue;
        const Note *n = NoteOf(cr.ref);
        if ((n != nullptr) && (c.now < n->hand_retry)) continue;
        const int64_t worth = DaiDirector::KindWorth(k, p, cr.model, cr.level);
        if ((weakest == nullptr) || (worth < weakest_worth)) { weakest = &cr; weakest_worth = worth; }
    }
    if ((weakest == nullptr) || (FixToInt(FixMul(FixFromInt(weakest_worth), p.creatures_dismiss_upgrade)) >= best_come)) return false;
    DaiJob *j = w.AddJob(DAI_JOB_DISMISS_CREATURE, KEY_HAND, FixFromInt(2));
    if (j == nullptr) return true;
    DaiHandParams hp;
    hp.things.push_back(weakest->ref);
    hp.room = portal->index;
    j->p = hp;
    m_room_made = c.now;
    DaiLog::Info(c.player, "making room: creature %d (kind %d L%" PRId64 ", worth %" PRId64 ") dismissed for a kind worth %" PRId64,
        (int)weakest->ref.idx, (int)weakest->model, weakest->level, weakest_worth, best_come);
    return true;
}

namespace {
constexpr int64_t IMP_TRIPS = 2;            // imp training trips a 1,000 turns at most
constexpr DaiTurn IMP_STAY = 1500;         // turns a trainee is left before it may be taken out below its level
}

/** A gem face next to the keeper's own ground: gold without end, the one reason to train imps past speed (P9). */
bool DaiCreatures::gem_held(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    for (DaiSlabCoord y = 0; y < k.Height(); y++) {
        for (DaiSlabCoord x = 0; x < k.Width(); x++) {
            if (k.SlabAt(x, y).terrain != DaiTerrain::Gem) continue;
            for (const DaiSlabXY &d : { DaiSlabXY{ 0, -1 }, DaiSlabXY{ 1, 0 }, DaiSlabXY{ 0, 1 }, DaiSlabXY{ -1, 0 } }) {
                const DaiSlabCoord nx = x + d.x, ny = y + d.y;
                if ((nx < 0) || (ny < 0) || (nx >= k.Width()) || (ny >= k.Height())) continue;
                const DaiSlabInfo n = k.SlabAt(nx, ny);
                if ((n.owner == c.player) && ((n.terrain == DaiTerrain::Floor) || (n.terrain == DaiTerrain::Room))) return true;
            }
        }
    }
    return false;
}

/** Imps to the training room for speed (imp_train_level, 3: the main win), and past it towards teleport (10) only with
 *  gold plentiful (imp_train_rich_runway), a gem face held and little else for the imps (KeepImps met, no more dig
 *  marks than imps). The imps above imps_min, train_imps at a time, the closest to the level first; IMP_TRIPS trips a
 *  1,000 turns. An imp trains until taken out (the engine never stops it): at its level, or once training is not
 *  wanted -- started at twice the training runway, kept down to once it, a trainee left IMP_STAY turns (P9: the out
 *  rule flipping at one threshold took imps in and out 30-57 times a game). */
bool DaiCreatures::train_imps(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiPersonality &p = c.personality;
    while (!m_imp_trips.empty() && (c.now >= m_imp_trips.front().at + 2 * IMP_STAY)) m_imp_trips.erase(m_imp_trips.begin());
    if ((p.workforce_train_imps == 0) || ((int64_t)m_trips.size() >= c.cfg.creatures_moves_per_kturn)) return false;
    const int64_t runway = c.forecast.runway_paydays, R = c.cfg.programme_train_runway;
    const bool goal = c.goals.Find(DaiGoalKind::Train) != nullptr;
    // The runway, or spare gold in hand (plan 13).
    const int64_t spare = c.forecast.SparePaydays();
    const bool keep = goal && ((runway >= R) || (spare >= R)), start = goal && ((runway >= 2 * R) || (spare >= 2 * R));
    int64_t imps = 0, training = 0;
    for (const DaiImpLook &i : k.OwnImps()) {
        if (i.tunneller) continue;
        imps++;
        training += i.training ? 1 : 0;
    }
    // Only with the imps the work wants (KeepImps met), no more dig marks than imps and no room being placed or grown:
    // an imp sent to train early held the lair back (P9, dai_build_basic). Past speed: kept while the gem is held and half the rich runway is there;
    // started only with all of it.
    const DaiGoal *imps_goal = c.goals.Find(DaiGoalKind::KeepImps);
    bool building = false;                  // a room placed or grown: its imps' work (the lair, P9)
    c.jobs.ForEach([&](const DaiJob &j) {
        building = building || (((j.kind == DAI_JOB_PLACE_ROOM) || (j.kind == DAI_JOB_GROW_ROOM)) && !DaiJobEnded(j.state));
    });
    const bool quiet = ((imps_goal == nullptr) || (k.Own().imps >= imps_goal->p.count)) && (k.OwnMarkCount() <= imps) && !building;
    bool rich_keep = false, rich_start = false;
    // Rich: the gold in hand, spare gold in pay days' wages (plan 13), not the runway the training itself shortens.
    if ((p.workforce_imp_train_rich_level > p.workforce_imp_train_level) && (spare * 2 >= p.workforce_imp_train_rich_runway)) {
        rich_keep = gem_held(c);
        rich_start = rich_keep && quiet && (spare >= p.workforce_imp_train_rich_runway);
    }
    const int64_t keep_to = rich_keep ? p.workforce_imp_train_rich_level : p.workforce_imp_train_level;
    const int64_t send_to = rich_start ? p.workforce_imp_train_rich_level : p.workforce_imp_train_level;
    const bool too_many = (training > p.workforce_train_imps) || (imps - training < p.workforce_imps_min);
    const DaiImpLook *done = nullptr, *next = nullptr;
    for (const DaiImpLook &i : k.OwnImps()) {
        if (i.tunneller || !i.pickable) continue;
        if (i.training) {
            bool fresh = false;
            for (const ImpTrip &t : m_imp_trips) fresh = fresh || ((t.ref == i.ref) && (c.now < t.at + IMP_STAY));
            const bool out = (i.level >= keep_to) || !keep || (too_many && !fresh);
            if (out && (done == nullptr)) done = &i;
            continue;
        }
        if (i.sacrificing || (i.gold > 0) || (i.level >= send_to)) continue;
        if ((next == nullptr) || (i.level > next->level)) next = &i;
    }
    const DaiRoomLook *heart = nullptr, *gym = nullptr;
    for (const DaiRoomLook &r : k.Rooms()) {
        if ((r.roles & DAI_ROLE_HEART) != 0) heart = &r;
        if (((r.roles & DaiWorkRole(DaiWork::Train)) != 0) && (r.capacity > r.used) && ((gym == nullptr) || (r.efficiency > gym->efficiency))) gym = &r;
    }
    DaiHandParams hp;
    DaiJobKindId kind = DAI_JOB_TRAIN_CREATURE;
    if ((done != nullptr) && (heart != nullptr)) {
        // Back to work, dropped on the heart (nothing there to do, so its own tasks).
        hp.things.push_back(done->ref);
        hp.room = heart->index;
        hp.work = DaiWork::Count;
        kind = DAI_JOB_MOVE_TO_JOB;
    } else {
        int64_t recent = 0;
        for (const ImpTrip &t : m_imp_trips) recent += (c.now < t.at + 1000) ? 1 : 0;
        if (!start || !quiet || (next == nullptr) || (gym == nullptr) || (training >= p.workforce_train_imps)
            || (imps - training <= p.workforce_imps_min) || (recent >= IMP_TRIPS)) return false;
        hp.things.push_back(next->ref);
        hp.room = gym->index;
        hp.work = DaiWork::Train;
    }
    DaiJob *j = w.AddJob(kind, KEY_HAND, FixFromInt(1));
    if (j == nullptr) return false;
    j->p = hp;
    m_trips.push_back(c.now);
    if (kind == DAI_JOB_TRAIN_CREATURE) m_imp_trips.push_back(ImpTrip{ hp.things[0], c.now });
    DaiLog::Info(c.player, "imp %d (level %" PRId64 ") %s", (int)hp.things[0].idx, (done != nullptr) ? done->level : next->level,
        (kind == DAI_JOB_TRAIN_CREATURE) ? "to train" : "back to work");
    return true;
}

/** Notes for the creatures there now; livid since when. */
void DaiCreatures::track(const DaiTurnCtx &c)
{
    std::vector<Note> kept;
    for (const DaiCreatureLook &cr : c.know.Creatures()) {
        Note n;
        if (const Note *old = note(cr.ref)) n = *old;
        n.ref = cr.ref;
        if (cr.anger != DaiAnger::Livid) n.livid_since = 0;
        else if (n.livid_since == 0) n.livid_since = c.now;
        if ((cr.anger == DaiAnger::Calm) && !wounded(c, cr)) n.need_since = 0;
        else if (n.need_since == 0) n.need_since = c.now;
        kept.push_back(n);
    }
    m_notes = kept;
    while (!m_trips.empty() && (m_trips.front() + 1000 <= c.now)) m_trips.erase(m_trips.begin());
}

/** Can a livid creature's cause be put right while it is held (08 section 8)? Unpaid: the pay day within the hold and
 *  the gold for its pay there; hungry: chickens in a hatchery; no lair: a lair goal open (one growing). */
bool cause_fixable(const DaiTurnCtx &c, const DaiCreatureLook &cr)
{
    const DaiKnowledge &k = c.know;
    switch (cr.anger_reason) {
    case DaiAngerReason::NotPaid: return (c.forecast.turns_to_payday <= c.personality.custody_discipline_hold) && (k.Own().gold >= k.Kind(cr.model).pay);
    case DaiAngerReason::Hungry: return !k.Food().empty();
    case DaiAngerReason::NoLair: return c.goals.Find(DaiGoalKind::Build, (int64_t)DaiRoom::Lair) != nullptr;
    default: return false;
    }
}

const DaiRoomLook *room_free(const DaiKnowledge &k, uint64_t roles)
{
    for (const DaiRoomLook &r : k.Rooms()) if (((r.roles & roles) != 0) && (r.used < r.capacity)) return &r;
    return nullptr;
}

/** A room with these roles and a free place, else any with them; nullptr if none. */
const DaiRoomLook *room_free_any(const DaiKnowledge &k, uint64_t roles)
{
    if (const DaiRoomLook *r = room_free(k, roles)) return r;
    for (const DaiRoomLook &r : k.Rooms()) if ((r.roles & roles) != 0) return &r;
    return nullptr;
}

/** A livid creature past its grace, of a kind the personality dismisses, goes on the portal -- or, with its discipline,
 *  to prison or a rack; a jailed one goes home once its time is up. */
bool DaiCreatures::trouble(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    // Released: a creature jailed for discipline_hold turns, back to its lair (a lair of its; else the first with room).
    for (const DaiCreatureLook &cr : k.Creatures()) {
        Note *n = note(cr.ref);
        if ((n == nullptr) || (n->jailed == 0)) continue;
        if (cr.held_by != c.player) { n->jailed = 0; continue; }       // let out, or taken
        if (c.now < n->jailed + (DaiTurn)c.personality.custody_discipline_hold) continue;
        const DaiRoomLook *lair = room_free(k, DAI_ROLE_LAIR);
        if (lair == nullptr) continue;
        DaiJob *j = w.AddJob(DAI_JOB_SEND_TO_LAIR, KEY_HAND, FixFromInt(3));
        if (j == nullptr) return true;
        DaiHandParams p;
        p.things.push_back(cr.ref);
        p.room = lair->index;
        j->p = p;
        n->jailed = 0;
        n->livid_since = c.now;     // its anger was frozen while held: a fresh grace to cool
        return true;
    }
    const int64_t mode = c.personality.creatures_dismiss_livid;
    if ((mode == 0) || (c.goals.Find(DaiGoalKind::Forbid, DAI_FORBID_DISMISS) != nullptr)) return false;   // recover
    const DaiRoomLook *portal = nullptr;
    if (portal == nullptr) portal = k.FirstRoomWith(DAI_ROLE_PORTAL);
    const int64_t discipline = c.personality.custody_discipline;    // 1 prison, 2 torture, 3 both
    for (const DaiCreatureLook &cr : k.Creatures()) {
        const Note *n = NoteOf(cr.ref);
        if ((n == nullptr) || (n->livid_since == 0) || (c.now < n->livid_since + (DaiTurn)c.cfg.creatures_livid_grace)) continue;
        if ((mode == 1) && !k.Kind(cr.model).anger_harmful) continue;
        if (cr.held_by >= 0) continue;                                  // jailed already (anger frozen)
        if (c.now < n->hand_retry) continue;                            // a trip for it refused lately
        const DaiRoomLook *prison = ((discipline & 1) != 0) && cause_fixable(c, cr) ? room_free(k, DAI_ROLE_PRISON) : nullptr;
        const DaiRoomLook *rack = (((discipline & 2) != 0) && (prison == nullptr) && (cr.model == DaiCustody::MostPaidKind(k))
                                   && (c.personality.custody_pay_discount_torture != 0) && (k.Custody().devices_free > 0))
            ? room_free(k, DAI_ROLE_TORTURE) : nullptr;
        DaiLog::Debug(c.player, "livid creature %d (reason %d): discipline %" PRId64 ", fixable %d, prison %d, rack %d, portal %d", (int)cr.ref.idx,
            (int)cr.anger_reason, discipline, (int)cause_fixable(c, cr), (int)(prison != nullptr), (int)(rack != nullptr), (int)(portal != nullptr));
        if ((prison != nullptr) || (rack != nullptr)) {
            DaiJob *j = w.AddJob(DAI_JOB_JAIL_CREATURE, KEY_HAND, FixFromInt(3));
            if (j == nullptr) return true;
            DaiHandParams p;
            p.things.push_back(cr.ref);
            p.room = (prison != nullptr) ? prison->index : rack->index;
            p.torture = (prison == nullptr);
            j->p = p;
            if (Note *nn = note(cr.ref)) nn->jailed = c.now;
            return true;
        }
        if (portal == nullptr) continue;
        DaiJob *j = w.AddJob(DAI_JOB_DISMISS_CREATURE, KEY_HAND, FixFromInt(3));
        if (j == nullptr) return true;
        DaiHandParams p;
        p.things.push_back(cr.ref);
        p.room = portal->index;
        j->p = p;
        return true;
    }
    return false;
}

/** Pay the angriest unpaid creature by hand when its pay day is far; feed a hungry one; carry a wounded one home --
 *  each only once its need is the difficulty's care_delay old (05 section 7.1: easy sees to needs late). */
bool DaiCreatures::care(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    auto late = [&](const DaiCreatureLook &cr) {
        if (c.difficulty.care_delay <= 0) return false;
        const Note *n = NoteOf(cr.ref);
        return (n == nullptr) || (n->need_since == 0) || (c.now < n->need_since + (DaiTurn)c.difficulty.care_delay);
    };
    // Pay: the livid first, then the angry, in the knowledge's order.
    const DaiCreatureLook *pay = nullptr;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if ((cr.anger == DaiAnger::Calm) || (cr.anger_reason != DaiAngerReason::NotPaid) || late(cr)) continue;
        const Note *n = NoteOf(cr.ref);
        if ((n != nullptr) && (n->paid != 0) && (c.now < n->paid + DAI_PAY_AGAIN_TURNS)) continue;
        if (k.Kind(cr.model).pay > k.Own().gold) continue;
        if ((pay == nullptr) || ((cr.anger == DaiAnger::Livid) && (pay->anger != DaiAnger::Livid))) pay = &cr;
    }
    if ((pay != nullptr) && (c.forecast.turns_to_payday > DAI_PAY_LEAD_TURNS)) {
        DaiJob *j = w.AddJob(DAI_JOB_PAY_CREATURE, KEY_HAND, FixFromInt(2));
        if (j == nullptr) return true;
        DaiHandParams p;
        p.target = pay->ref;
        j->p = p;
        if (Note *n = note(pay->ref)) n->paid = c.now;
        return true;
    }
    // Feed: a creature angry with hunger gets the chicken nearest it.
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if ((cr.anger == DaiAnger::Calm) || (cr.anger_reason != DaiAngerReason::Hungry) || k.Food().empty() || late(cr)) continue;
        const DaiFoodLook *best = nullptr;
        int64_t best_d = 0;
        for (const DaiFoodLook &f : k.Food()) {
            const int64_t d = std::abs(f.x - cr.x) + std::abs(f.y - cr.y);
            if ((best == nullptr) || (d < best_d)) { best = &f; best_d = d; }
        }
        DaiJob *j = w.AddJob(DAI_JOB_FEED_CREATURE, KEY_HAND, FixFromInt(2));
        if (j == nullptr) return true;
        DaiHandParams p;
        p.things.push_back(best->ref);
        p.target = cr.ref;
        j->p = p;
        return true;
    }
    // Home: a wounded creature standing about far from its nearest lair with room is carried there.
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if (!wounded(c, cr) || ((cr.activity != DaiActivity::Idle) && (cr.activity != DaiActivity::Move)) || late(cr)) continue;
        const DaiRoomLook *lair = nullptr;
        int64_t lair_d = 0;
        for (const DaiRoomLook &r : k.Rooms()) {
            if (((r.roles & DAI_ROLE_LAIR) == 0) || (r.used >= r.capacity)) continue;
            const int64_t d = slabs_apart(cr.x, cr.y, r.cx, r.cy);
            if ((lair == nullptr) || (d < lair_d)) { lair = &r; lair_d = d; }
        }
        if ((lair == nullptr) || (lair_d <= DAI_LAIR_FAR_SLABS)) continue;
        DaiJob *j = w.AddJob(DAI_JOB_SEND_TO_LAIR, KEY_HAND, FixFromInt(2));
        if (j == nullptr) return true;
        DaiHandParams p;
        p.things.push_back(cr.ref);
        p.room = lair->index;
        j->p = p;
        return true;
    }
    // Prayer (08 section 7.1): a creature angry for no reason the keeper can mend, diseased or a chicken, to a temple --
    // praying calms "other" anger and cures both. Not one held, fighting, praying already, or carried lately.
    const DaiRoomLook *temple = nullptr;
    if (temple == nullptr) temple = k.FirstRoomWith(DAI_ROLE_SACRIFICE);
    if (temple == nullptr) return false;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        const bool needs = ((cr.anger != DaiAnger::Calm) && (cr.anger_reason == DaiAngerReason::Other)) || cr.diseased || cr.chickened
                        || cr.scavenged;    // a rival's scavenger room luring it: praying stops the lure (08 section 7a.3)
        if (!needs || (cr.held_by >= 0) || (cr.battle != 0) || (cr.in_room == temple->kind) || late(cr)) continue;
        const Note *n = NoteOf(cr.ref);
        if ((n != nullptr) && (n->moved != 0) && (c.now < n->moved + 1000)) continue;
        DaiJob *j = w.AddJob(DAI_JOB_MOVE_TO_JOB, KEY_HAND, FixFromInt(2));
        if (j == nullptr) return true;
        DaiHandParams p;
        p.things.push_back(cr.ref);
        p.room = temple->index;
        j->p = p;
        if (Note *nn = note(cr.ref)) nn->moved = c.now;
        return true;
    }
    return false;
}

/** Idle creatures to the work worth most to them, a trip of up to DAI_MOVE_PER_TRIP to one room. */
bool DaiCreatures::work(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    if ((int64_t)m_trips.size() >= c.cfg.creatures_moves_per_kturn) return false;
    // What work is wanted now.
    bool wanted[(size_t)DaiWork::Count] = {};
    wanted[(size_t)DaiWork::Research] = (k.Research().count > 0);
    const DaiGoal *mf = c.goals.Find(DaiGoalKind::Manufacture);
    int64_t makers = 0;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if (cr.activity != DaiActivity::Work) continue;
        for (const DaiRoomLook &r : k.Rooms()) if ((r.kind == cr.in_room) && ((r.roles & DAI_ROLE_MANUFACTURE) != 0)) { makers++; break; }
    }
    // The Manufacture goal's workers (doors the keeper lacks); else one, while a workshop stands and something can be
    // made -- the director built it to make things (P2.10: a workshop stood 12,000 turns with nobody in it).
    bool makeable = false;
    const DaiStockLook &st = k.Stock();
    st.ForEachKind(true, [&](int64_t s, int64_t) { makeable = makeable || st.doors_buildable[s]; });
    st.ForEachKind(false, [&](int64_t s, int64_t) { makeable = makeable || st.traps_buildable[s]; });
    bool workshop = false;
    workshop = workshop || k.HasRoomRole(DAI_ROLE_MANUFACTURE);
    const int64_t makers_wanted = std::max<int64_t>((mf != nullptr) ? mf->p.count : 0, (workshop && makeable) ? 1 : 0);
    wanted[(size_t)DaiWork::Manufacture] = (makers < makers_wanted);
    // Training while the director's Train goal stands (not in opening or recover) and the runway allows it, to the
    // goal's level (the director raises it to attack_level_target, and the weight above 1, when an attack waits on it).
    const DaiGoal *train_goal = c.goals.Find(DaiGoalKind::Train);
    // The runway, or spare gold in hand (plan 13: the training's own cost shortens the runway with gold banked).
    const bool spare = c.forecast.SparePaydays() >= c.personality.creatures_train_spare_paydays;
    wanted[(size_t)DaiWork::Train] = (train_goal != nullptr) && ((c.forecast.runway_paydays >= c.cfg.programme_train_runway) || spare);
    const int64_t train_to = std::max<int64_t>(c.personality.creatures_train_target_level, (train_goal != nullptr) ? train_goal->p.count : 0);
    // Taking creatures from their work to train needs twice the runway (P5.12: a warlord on a thin economy trained
    // itself into short pay days and recover).
    const bool train_push = wanted[(size_t)DaiWork::Train] && (train_goal->weight > DAI_FIX_ONE)
        && ((c.forecast.runway_paydays >= 2 * c.cfg.programme_train_runway) || spare);
    // Each idle creature's best work room.
    struct Pick { const DaiCreatureLook *cr; DaiWork work; const DaiRoomLook *room; int64_t value; };
    std::vector<Pick> picks;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if ((cr.activity != DaiActivity::Idle) || (cr.anger == DaiAnger::Livid) || wounded(c, cr)) continue;
        const Note *n = NoteOf(cr.ref);
        if ((n != nullptr) && (n->moved != 0) && (c.now < n->moved + 1000)) continue;
        Pick best{ &cr, DaiWork::Count, nullptr, 0 };
        for (size_t wi = 0; wi < (size_t)DaiWork::Count; wi++) {
            const DaiWork wk = (DaiWork)wi;
            if (!wanted[wi]) continue;
            if ((wk == DaiWork::Train) && (cr.level >= train_to)) continue;
            for (const DaiRoomLook &r : k.Rooms()) {
                if ((r.roles & DaiWorkRole(wk)) == 0) continue;
                const int64_t v = WorkValue(c, cr, wk, r);
                if (v > best.value) best = Pick{ &cr, wk, &r, v };
            }
        }
        if (best.room != nullptr) picks.push_back(best);
    }
    // Workshop workers wanted and no idle creature to send: the best maker from other work (it would never be idle:
    // P2.10, a workshop built at turn 14,775 had nobody in it by 30,000 -- everyone researched or trained).
    const bool to_make = std::any_of(picks.begin(), picks.end(), [](const Pick &pk) { return pk.work == DaiWork::Manufacture; });
    if (wanted[(size_t)DaiWork::Manufacture] && !to_make) {
        // However far the walk: a value below nothing still names the best maker (0 is a creature that cannot make
        // there at all). P2.10: a workshop built late and far had every creature's value under nothing. The best for
        // its making against its fight (DaiAbilities): only the Manufacture goal's makers are taken, and a troll makes
        // where a bile demon, its health and gas worth training, would not (P9).
        bool found = false;
        int64_t best_fit = 0;
        Pick best{ nullptr, DaiWork::Manufacture, nullptr, 0 };
        for (const DaiCreatureLook &cr : k.Creatures()) {
            if (((cr.activity != DaiActivity::Work) && (cr.activity != DaiActivity::Idle)) || (cr.anger == DaiAnger::Livid) || wounded(c, cr))
                continue;
            bool making = false;
            for (const DaiRoomLook &r : k.Rooms())
                making = making || ((r.kind == cr.in_room) && ((r.roles & (DAI_ROLE_MANUFACTURE | DAI_ROLE_GUARD)) != 0));   // a guard: Defence's
            const Note *n = NoteOf(cr.ref);
            if (making || ((n != nullptr) && (n->moved != 0) && (c.now < n->moved + 1000))) continue;
            for (const DaiRoomLook &r : k.Rooms()) {
                if ((r.roles & DAI_ROLE_MANUFACTURE) == 0) continue;
                const int64_t v = WorkValue(c, cr, DaiWork::Manufacture, r);
                if (v == 0) continue;
                const DaiKindLook &kl = k.Kind(cr.model);
                const int64_t fight = std::max<int64_t>(kl.output[std::clamp<int64_t>(cr.level - 1, 0, DaiKindLook::LEVELS - 1)], 1);
                const int64_t fit = skill(kl, DaiWork::Manufacture) * 100000 / fight;
                if (!found || (fit > best_fit) || ((fit == best_fit) && (v > best.value))) {
                    best = Pick{ &cr, DaiWork::Manufacture, &r, v };
                    best_fit = fit;
                    found = true;
                }
            }
        }
        if (found) picks.push_back(best);
    }
    // Training pushed for an attack and nobody idle to send: the lowest-level creatures at other work below the goal's
    // level, a trip's worth to the best training room for the lowest (07 P5.12: creatures research or manufacture and
    // are never idle, so nobody trained; one a trip trained too few to reach the level in a game).
    const bool to_train = std::any_of(picks.begin(), picks.end(), [](const Pick &pk) { return pk.work == DaiWork::Train; });
    if (train_push && !to_train) {
        // Not from research while a room is still to research (plan 13: spare gold pushes training, but the rooms first).
        const DaiResearchLook &rs = k.Research();
        bool rooms_left = false;
        for (int64_t i = 0; (i < rs.count) && (i < DaiResearchLook::MAX); i++) rooms_left = rooms_left || (rs.items[i].kind == DaiResearchKind::Room);
        std::vector<Pick> pulls;
        for (const DaiCreatureLook &cr : k.Creatures()) {
            if ((cr.activity != DaiActivity::Work) || (cr.anger == DaiAnger::Livid) || wounded(c, cr) || (cr.level >= train_to)) continue;
            bool training = false, researching = false;
            for (const DaiRoomLook &r : k.Rooms()) {
                training = training || ((r.kind == cr.in_room) && ((r.roles & DaiWorkRole(DaiWork::Train)) != 0));
                researching = researching || ((r.kind == cr.in_room) && ((r.roles & DAI_ROLE_RESEARCH) != 0));
            }
            if (researching && rooms_left) continue;
            bool guarding = false;     // Defence's hold keeps it there for its turn (plan 13 L3)
            guarding = guarding || k.InRoomWith(cr, DAI_ROLE_GUARD);
            if (guarding) continue;
            const Note *n = NoteOf(cr.ref);
            if (training || ((n != nullptr) && (n->moved != 0) && (c.now < n->moved + 1000))) continue;
            // However far: the hand carries them, so any room it can train in with space left, the best valued.
            const DaiKindLook &kind = k.Kind(cr.model);
            if (!kind.known || kind.digger || kind.refuses[(size_t)DaiWork::Train]) continue;
            Pick best{ &cr, DaiWork::Train, nullptr, 0 };
            for (const DaiRoomLook &r : k.Rooms()) {
                if (((r.roles & DaiWorkRole(DaiWork::Train)) == 0) || (r.capacity <= r.used)) continue;
                const int64_t v = WorkValue(c, cr, DaiWork::Train, r);
                if ((best.room == nullptr) || (v > best.value)) best = Pick{ &cr, DaiWork::Train, &r, v };
            }
            if (best.room != nullptr) pulls.push_back(best);
        }
        std::stable_sort(pulls.begin(), pulls.end(), [](const Pick &a, const Pick &b) {
            return (a.cr->level != b.cr->level) ? (a.cr->level < b.cr->level) : (a.value > b.value);
        });
        // Its value raised to the top: the trip below goes to the first pick's room, and the pulls with it.
        int64_t top = 0;
        for (const Pick &pk : picks) top = std::max(top, pk.value);
        for (Pick &pk : pulls) {
            if (pk.room != pulls.front().room) continue;
            pk.value = top + 1;
            picks.push_back(pk);
        }
    }
    if (picks.empty()) return false;
    // By value -- but an empty workshop's worker first: research and training outrank most kinds' making, and with the
    // trips spent on them the workshop never got its one (P2.10).
    const bool workshop_empty = wanted[(size_t)DaiWork::Manufacture] && (makers == 0);
    std::stable_sort(picks.begin(), picks.end(), [&](const Pick &a, const Pick &b) {
        if (workshop_empty && ((a.work == DaiWork::Manufacture) != (b.work == DaiWork::Manufacture))) return a.work == DaiWork::Manufacture;
        return a.value > b.value;
    });
    const Pick &top = picks.front();
    DaiHandParams p;
    p.room = top.room->index;
    p.work = top.work;
    for (const Pick &pk : picks) {
        if (((int64_t)p.things.size() >= DAI_MOVE_PER_TRIP) || (pk.room != top.room)) continue;
        p.things.push_back(pk.cr->ref);
    }
    DaiJob *j = w.AddJob((top.work == DaiWork::Train) ? DAI_JOB_TRAIN_CREATURE : DAI_JOB_MOVE_TO_JOB, KEY_HAND, FixFromInt(1));
    if (j == nullptr) return true;
    j->p = p;
    m_trips.push_back(c.now);
    for (const DaiThingRef &r : p.things) if (Note *n = note(r)) n->moved = c.now;
    return true;
}

/** KeepGold weighed up while the gold would not pay the average creature likely to come next (04 section 7.1). */
void DaiCreatures::arrivals(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    int64_t pay = 0, kinds = 0;
    for (DaiThingModel m = 1; m <= k.KindCount(); m++) {
        const DaiKindLook &kd = k.Kind(m);
        if (!kd.known || !kd.allowed || (kd.in_pool == 0) || kd.digger || kd.spectator) continue;   // 0: known none left (Q8)
        pay += kd.pay;
        kinds++;
    }
    const bool short_of = (kinds > 0) && (k.Own().gold < pay / kinds);
    const DaiRequest *asked = c.requests.Find(DaiDomain::Creatures, DaiGoalKind::KeepGold, 0);
    if (short_of && (asked == nullptr)) {
        DaiRequest r;
        r.goal = DaiGoalKind::KeepGold;
        r.weight_delta = DAI_FIX_ONE / 4;
        snprintf(r.why, sizeof(r.why), "NEXT_ARRIVAL");
        w.PostRequest(r);
    } else if (!short_of && (asked != nullptr)) {
        w.WithdrawRequest(asked->id);
    }
}

void DaiCreatures::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    // A discipline trip refused (the creature would not be picked up: a livid one in a fight, 344 tries in one game while
    // the hand was wanted for the fight): not the same creature again every turn.
    const DaiHandParams *hp = std::get_if<DaiHandParams>(&j.p);
    if (((j.kind == DAI_JOB_DISMISS_CREATURE) || (j.kind == DAI_JOB_JAIL_CREATURE)) && (j.state == DaiJobState::Failed) && (hp != nullptr)
        && !hp->things.empty())
        if (Note *n = note(hp->things[0])) n->hand_retry = c.now + (DaiTurn)(c.cfg.jobs_stall_window / 4);
    if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
}

void DaiCreatures::Reset()
{
    m_notes.clear();
    m_trips.clear();
    m_imp_trips.clear();
    m_room_made = 0;
    m_scav_kind = 0;
    m_scav_n = 0;
    m_scav_pool = false;
    m_scav_next = 0;
    m_pool_since.fill(0);
    m_pool_dry.fill(0);
    m_pool_count.fill(0);
}

/** Each kind it has weighed for scavenging; the best worth anything chosen (08 section 7a). */
void DaiCreatures::weigh_scavenging(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    const int64_t appetite = c.personality.creatures_scavenge_appetite;
    m_scav_kind = 0;
    m_scav_n = 0;
    m_scav_pool = false;
    if ((appetite <= 0) || (c.cfg.creatures_scavengers_max <= 0)) return;
    const int64_t fpg = DaiCustody::ForcePerGold(k);
    const DaiRoomLook *room = room_free_any(k, DAI_ROLE_SCAVENGE);
    const int64_t cap = (room != nullptr) ? std::max<int64_t>(1, room->capacity) : c.cfg.creatures_scavengers_max;
    // What the portal may yet send (an alluring kind's scavenging speeds it, P9): the level-1 worth of the kinds the pool
    // still holds and the keeper may get, while it has room for more creatures.
    int64_t pool_worth = 0, pool_kinds = 0;
    if ((k.Research().max_creatures <= 0) || ((int64_t)k.Creatures().size() < k.Research().max_creatures)) {
        for (DaiThingModel m = 1; m < DAI_MAX_KINDS; m++) {
            const DaiKindLook &kd = k.Kind(m);
            if (!kd.known || kd.digger || kd.spectator || !kd.allowed || (kd.in_pool == 0)) continue;
            pool_worth += kd.score[0];
            pool_kinds++;
        }
    }
    int64_t best = 0;
    for (DaiThingModel m = 1; m < DAI_MAX_KINDS; m++) {
        const DaiKindLook &kd = k.Kind(m);
        if (!kd.known || kd.digger || kd.spectator) continue;
        // Its creatures that could go: not held, not fighting; their work, the best first.
        std::vector<int64_t> works;
        int64_t own_required = 0;
        for (const DaiCreatureLook &cr : k.Creatures()) {
            if ((cr.model != m) || (cr.held_by >= 0) || (cr.battle != 0) || (cr.scavenge_work <= 0)) continue;
            works.push_back(cr.scavenge_work);
            own_required = cr.scavenge_required;
        }
        if (works.empty()) continue;
        std::sort(works.rbegin(), works.rend());
        const int64_t n = std::min<int64_t>({ (int64_t)works.size(), c.cfg.creatures_scavengers_max, cap });
        int64_t work = 0;
        for (int64_t i = 0; i < n; i++) work += works[(size_t)i];
        // The target: the weakest rival's creature of the kind in sight, else the pool (allowed, not known dry).
        int64_t required = 0, gain = 0;
        bool pool = false;
        for (const DaiThingLook &e : k.Enemies()) {
            if ((e.model != m) || (e.scavenge_required <= 0) || !k.RivalKeeper(e.owner)) continue;
            if ((required == 0) || (e.scavenge_required < required)) {
                required = e.scavenge_required;
                gain = kd.score[std::max<int64_t>(0, std::min<int64_t>(e.level - 1, DaiKindLook::LEVELS - 1))];
            }
        }
        if ((required == 0) && kd.allowed && (kd.in_pool != 0) && (c.now >= m_pool_dry[(size_t)m]) && (own_required > 0)) {
            required = own_required;
            gain = kd.score[0];
            pool = true;
        }
        // An alluring kind (a vampire) with nothing of its kind to lure: the portal sped for any kind, allure_value of the
        // pool's level-1 worth each lure's worth of work (portal_scavenge_boost, P9).
        if ((required == 0) && kd.alluring && (own_required > 0) && (pool_kinds > 0)) {
            required = own_required;
            gain = FixToInt(FixMul(FixFromInt(pool_worth / pool_kinds), c.cfg.abilities_allure_value));
        }
        if ((required <= 0) || (work <= 0)) continue;
        const int64_t turns = (required * 256) / work;
        const int64_t gold = n * kd.scavenger_cost * turns / 64;
        const int64_t worth = FixToInt(FixMul(FixFromInt(gain), 2 * appetite)) - FixToInt(FixMul(FixFromInt(gold), fpg));
        if (worth > best) {
            best = worth;
            m_scav_kind = m;
            m_scav_n = n;
            m_scav_pool = pool;
        }
    }
    if (m_scav_kind != 0)
        DaiLog::Debug(c.player, "scavenging: kind %d, %" PRId64 " scavengers, %s, worth %" PRId64, (int)m_scav_kind, m_scav_n,
            m_scav_pool ? "the pool" : "a rival's", best);
}

/** The chosen kind's scavengers into the scavenger room, one trip a turn; out when not wanted there; a creature being
 *  scavenged to pray; the room asked for. */
bool DaiCreatures::scavenge(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    if (c.now >= m_scav_next) {
        weigh_scavenging(c);
        m_scav_next = c.now + (DaiTurn)c.cfg.creatures_scavenge_interval;
    }
    // The pool draw: a creature of the kind come is a draw; tried pool_try_turns with nothing, the kind is dry.
    if ((m_scav_kind != 0) && m_scav_pool) {
        const size_t m = (size_t)m_scav_kind;
        int64_t have = 0;
        for (const DaiCreatureLook &cr : k.Creatures()) have += (cr.model == m_scav_kind) ? 1 : 0;
        if ((m_pool_since[m] == 0) || (have > m_pool_count[m])) {
            m_pool_since[m] = c.now;
            m_pool_count[m] = have;
        } else if (c.now >= m_pool_since[m] + (DaiTurn)c.cfg.creatures_pool_try_turns) {
            m_pool_dry[m] = c.now + (DaiTurn)c.cfg.creatures_pool_dry_turns;
            m_pool_since[m] = 0;
            DaiLog::Info(c.player, "scavenging: the pool has no more of kind %d to give (dry until %" PRIu64 ")", (int)m_scav_kind, (uint64_t)m_pool_dry[m]);
            m_scav_kind = 0;
        }
    }
    const DaiRoomLook *room = nullptr;
    if (room == nullptr) room = k.FirstRoomWith(DAI_ROLE_SCAVENGE);
    // The room: asked for while a kind is worth it and there is none.
    const DaiRequest *asked = c.requests.Find(DaiDomain::Creatures, DaiGoalKind::Build, (int64_t)DaiRoom::Scavenger);
    const bool want_room = (m_scav_kind != 0) && (room == nullptr) && k.Build(DaiRoom::Scavenger).available;
    if (want_room && (asked == nullptr)) {
        DaiRequest q;
        q.goal = DaiGoalKind::Build;
        q.key = (int64_t)DaiRoom::Scavenger;
        q.count_min = std::max<int64_t>(c.personality.ProgrammeSlabs(DaiRoom::Scavenger), 9);
        q.weight_delta = DAI_FIX_ONE / 4;
        snprintf(q.why, sizeof(q.why), "SCAVENGE");
        w.PostRequest(q);
    } else if (!want_room && (asked != nullptr)) {
        w.WithdrawRequest(asked->id);
    }
    if (room == nullptr) return false;
    const bool short_of_gold = c.forecast.runway_paydays < c.personality.economy_comfortable_paydays;
    int64_t in = 0;
    const DaiCreatureLook *out = nullptr, *go = nullptr;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        const bool there = (cr.in_room == room->kind) && (cr.activity == DaiActivity::Work);
        const bool wanted = (cr.model == m_scav_kind) && !short_of_gold;
        if (there && wanted) in++;
        if (there && !wanted && (out == nullptr)) out = &cr;
        const Note *n = NoteOf(cr.ref);
        const bool moved_lately = (n != nullptr) && (n->moved != 0) && (c.now < n->moved + 1000);
        if (!there && wanted && (go == nullptr) && (cr.held_by < 0) && (cr.battle == 0) && (cr.anger != DaiAnger::Livid) && !moved_lately
            && ((cr.activity == DaiActivity::Idle) || (cr.activity == DaiActivity::Work)))
            go = &cr;
    }
    const DaiCreatureLook *who = (out != nullptr) ? out : ((in < m_scav_n) ? go : nullptr);
    if (who == nullptr) return false;
    const DaiRoomLook *to = room;
    if (who == out) {
        to = room_free_any(k, DAI_ROLE_LAIR);
        if (to == nullptr) return false;
    }
    DaiJob *j = w.AddJob((who == out) ? DAI_JOB_SEND_TO_LAIR : DAI_JOB_MOVE_TO_JOB, KEY_HAND, FixFromInt(1));
    if (j == nullptr) return true;
    DaiHandParams p;
    p.things.push_back(who->ref);
    p.room = to->index;
    j->p = p;
    if (Note *n = note(who->ref)) n->moved = c.now;
    return true;
}

void DaiCreatures::Save(DaiBlobWriter &w) const
{
    w.I64((int64_t)m_notes.size());
    for (const Note &n : m_notes) {
        w.Ref(n.ref);
        w.U64(n.livid_since);
        w.U64(n.paid);
        w.U64(n.moved);
        w.U64(n.need_since);
        w.U64(n.jailed);
        w.U64(n.hand_retry);
    }
    w.I64((int64_t)m_trips.size());
    for (DaiTurn t : m_trips) w.U64(t);
    w.I64(m_scav_kind);
    w.I64(m_scav_n);
    w.Bool(m_scav_pool);
    w.U64(m_scav_next);
    for (size_t m = 0; m < m_pool_since.size(); m++) { w.U64(m_pool_since[m]); w.U64(m_pool_dry[m]); w.I64(m_pool_count[m]); }
    w.I64((int64_t)m_imp_trips.size());
    for (const ImpTrip &t : m_imp_trips) { w.Ref(t.ref); w.U64(t.at); }
    w.U64(m_room_made);
}

bool DaiCreatures::Load(DaiBlobReader &r)
{
    Reset();
    const int64_t n = r.Count(DAI_MAX_OWN_CREATURES);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        Note nt;
        nt.ref = r.Ref();
        nt.livid_since = (DaiTurn)r.U64();
        nt.paid = (DaiTurn)r.U64();
        nt.moved = (DaiTurn)r.U64();
        nt.need_since = (DaiTurn)r.U64();
        nt.jailed = (DaiTurn)r.U64();
        nt.hand_retry = (DaiTurn)r.U64();
        m_notes.push_back(nt);
    }
    const int64_t t = r.Count(100);
    for (int64_t i = 0; (i < t) && r.Ok(); i++) m_trips.push_back((DaiTurn)r.U64());
    m_scav_kind = (DaiThingModel)r.Range(0, DAI_MAX_KINDS - 1);
    m_scav_n = r.Range(0, 1000);
    m_scav_pool = r.Bool();
    m_scav_next = (DaiTurn)r.U64();
    for (size_t m = 0; (m < m_pool_since.size()) && r.Ok(); m++) {
        m_pool_since[m] = (DaiTurn)r.U64();
        m_pool_dry[m] = (DaiTurn)r.U64();
        m_pool_count[m] = r.Range(0, INT64_MAX);
    }
    const int64_t it = r.Count(1000);
    for (int64_t i = 0; (i < it) && r.Ok(); i++) {
        ImpTrip trip;
        trip.ref = r.Ref();
        trip.at = (DaiTurn)r.U64();
        m_imp_trips.push_back(trip);
    }
    m_room_made = (DaiTurn)r.U64();
    if (!r.Ok()) Reset();
    return r.Ok();
}
