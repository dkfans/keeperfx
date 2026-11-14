#include "pre_inc.h"
#include "daimon/DaiDirector.h"
#include "daimon/DaiGeom.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include "daimon/DaiLog.h"
#include "daimon/DaiRivals.h"
#include "post_inc.h"

namespace {
DaiGoal goal(DaiGoalKind kind, DaiFix weight, int64_t count = 0, int64_t key = 0)
{
    DaiGoal g;
    g.kind = kind;
    g.weight = weight;
    g.source = DaiGoalSource::Director;
    g.p.count = count;
    g.p.key = key;
    return g;
}

constexpr DaiFix W_KEEP_IMPS = DAI_FIX_ONE, W_KEEP_GOLD = DAI_FIX_ONE, W_MINE = 819;                    // 1.0, 1.0, 0.8
constexpr DaiFix W_FIRST_ENTRANCE = 922, W_MORE_ENTRANCES = 307, W_NEUTRAL_ROOM = 205;                  // 0.9, 0.3, 0.2
// A Build goal's weight by its rank in the personality's programme: 1.0 for the first, 0.1 less each rank after, at
// least 0.2; 0.5 for a room only a manager asked for.
constexpr DaiFix W_BUILD_STEP = 102, W_BUILD_LAST = 205, W_BUILD_ASKED = 512;
constexpr DaiFix W_TRAIN = DAI_FIX_ONE;
// Defend goals (06 section 11): the keeper's own ground in defend (Defence's share of the gestures times its weight,
// DaiArbiter), a contested rival's routes in contest.
constexpr DaiFix W_DEFEND_HOME = 4 * DAI_FIX_ONE, W_DEFEND_RIVAL = DAI_FIX_ONE;
constexpr DaiFix W_ATTACK = DAI_FIX_ONE;

// Each posture's factor on a goal kind's base weight (DaiFix, 05 section 6.2's effects): opening reaches for the portal
// and trains nobody; build_up builds; expand reaches out (ExpandTo also times the personality's expand_appetite) and
// spends; contest keeps near home and trains; recover keeps gold, reaches for nothing new and trains nobody; defend
// (Phase 4) holds back the building, the reaching out and the training while it fights; raid and assault (Phase 5)
// train and keep gold (the banner's upkeep) and reach out and build less.
constexpr int kFactorKinds = 6;
//                                                              KeepImps KeepGold Mine  ExpandTo Build  Train
constexpr DaiFix kPostureFactor[(size_t)DaiPosture::Count][kFactorKinds] = {
    /* opening  */ { 1024, 1024, 1024, 1280, 1024,    0 },
    /* build_up */ { 1024, 1024, 1024,  768, 1024,  256 },
    /* expand   */ { 1024,  768, 1024, 1536,  768,  512 },
    /* contest  */ { 1024, 1024,  768,  512,  768, 1024 },
    /* recover  */ { 1024, 1536, 1024,  256, 1024,    0 },
    /* defend   */ { 1024, 1024,  768,  256,  512,  512 },
    /* raid     */ { 1024, 1024, 1024,  768,  768, 1024 },
    /* assault  */ { 1024, 1280, 1024,  512,  512, 1024 },
};
int factor_column(DaiGoalKind k)
{
    switch (k) {
    case DaiGoalKind::KeepImps: return 0;
    case DaiGoalKind::KeepGold: return 1;
    case DaiGoalKind::Mine: return 2;
    case DaiGoalKind::ExpandTo: return 3;
    case DaiGoalKind::Build: return 4;
    case DaiGoalKind::Train: return 5;
    default: return -1;
    }
}


/** The rooms recover keeps building (05 section 6.2: no new rooms beyond the core). */
bool core_room(DaiRoom r) { return (r == DaiRoom::Treasury) || (r == DaiRoom::Lair) || (r == DaiRoom::Hatchery); }

/** Rooms only a manager's request or attraction puts on the programme (custody's, the scavenger's). */
bool extra_room(DaiRoom r)
{
    return (r == DaiRoom::Prison) || (r == DaiRoom::Torture) || (r == DaiRoom::Temple) || (r == DaiRoom::Graveyard) || (r == DaiRoom::Scavenger);
}

/** Room in the economy for spending beyond the programme -- a new extra room, a kind more to attract, a workshop for
 *  the most demanding item: imps_min imps, the next pay day covered, the runway comfortable (P9, level 52 with all the
 *  research done: a torture chamber, graveyard, temple and a 29-slab workshop by turn 10,000 took the gold the imps a
 *  trap killed were not bought back with, and eight steady keepers of twelve starved). */
bool room_for_extras(const DaiTurnCtx &c)
{
    // And the income lasting extras_horizon turns (plan 13: on a poor map the gold trains instead).
    const int64_t horizon = c.forecast.horizon_turns;
    return (c.know.Own().imps >= c.personality.workforce_imps_min) && !c.forecast.short_next
        && (c.forecast.runway_paydays >= c.personality.economy_comfortable_paydays)
        && ((horizon == DAI_NEVER) || (horizon >= c.personality.economy_extras_horizon));
}

/** Spare gold (plan 13): beyond the bank, train_spare_paydays pay days' wages (at least 1,000 a pay day), creatures to
 *  spend it on, the next pay day covered -- gold that should be training (level 124: 35,000-50,000 banked while its
 *  creatures trained slowly). */
bool spare_rich(const DaiTurnCtx &c)
{
    return (c.know.CreatureSummary().creatures > 0) && !c.forecast.short_next
        && (c.forecast.SparePaydays() >= c.personality.creatures_train_spare_paydays);
}

/** The slabs a work room of `role` grows to with spare gold when its rooms are at grow_full % of their capacity (plan
 *  13: worth it only when the room is the limit; else the gold trains and casts): a quarter more workers' room. */
int64_t grown(const DaiTurnCtx &c, uint64_t role, int64_t slabs)
{
    if (!spare_rich(c)) return slabs;
    int64_t capacity = 0, used = 0;
    for (const DaiRoomLook &r : c.know.Rooms()) {
        if ((r.roles & role) == 0) continue;
        capacity += r.capacity;
        used += r.used;
    }
    if ((capacity <= 0) || (used * 100 < capacity * c.personality.economy_grow_full)) return slabs;
    return std::max(slabs, DaiDirector::SlabsFor(c.know, role, used + std::max<int64_t>(used / 4, 1)));
}

int64_t slabs_of(const DaiKnowledge &k, DaiRoom r)
{
    const RoomKind kind = k.RoomKindOf(r);
    int64_t n = 0;
    for (const DaiRoomLook &l : k.Rooms()) if ((kind != 0) && (l.kind == kind)) n += l.slabs;
    return n;
}

/** A rival seen lately: within contest_forget turns. */
bool recent(const DaiTurnCtx &c, PlayerNumber r)
{
    const DaiRivalSighting &s = c.know.Sighting(r);
    return c.know.RivalKeeper(r) && (s.last_seen != 0) && (c.now <= s.last_seen + (DaiTurn)c.cfg.director_contest_forget);
}
}

const char *DaiPostureName(DaiPosture p)
{
    switch (p) {
    case DaiPosture::Opening: return "opening";
    case DaiPosture::BuildUp: return "build_up";
    case DaiPosture::Expand: return "expand";
    case DaiPosture::Contest: return "contest";
    case DaiPosture::Recover: return "recover";
    case DaiPosture::Defend: return "defend";
    case DaiPosture::Raid: return "raid";
    case DaiPosture::Assault: return "assault";
    default: return "?";
    }
}

DaiFix DaiDirector::factor(DaiGoalKind kind) const
{
    const int col = factor_column(kind);
    return (col < 0) ? DAI_FIX_ONE : kPostureFactor[(size_t)m_posture][col];
}

bool DaiDirector::programme_done(const DaiKnowledge &k) const
{
    for (size_t i = 0; i < m_target.size(); i++) {
        const DaiRoom r = (DaiRoom)i;
        if ((m_target[i] <= 0) || !k.Build(r).available) continue;
        if (slabs_of(k, r) * 8 < m_target[i] * 7) return false;     // within an eighth of its size
    }
    return true;
}

bool DaiDirector::recover_trigger(const DaiTurnCtx &c) const
{
    const DaiKnowledge &k = c.know;
    const DaiTurn window = (DaiTurn)c.cfg.director_recover_window;
    const RoomKind bridge = k.RoomKindOf(DaiRoom::Bridge);
    bool lost = false;
    k.ForEachRoomRecord([&](const DaiRoomRecord &r) {
        lost = lost || ((r.index == 0) && (r.lost != 0) && (r.kind != bridge) && (c.now < r.lost + window));
    });
    const int64_t creatures = k.CreatureSummary().creatures;
    const bool army = (m_army_peak >= 4) && ((m_army_peak - creatures) * DAI_FIX_ONE >= m_army_peak * c.personality.director_recover_threshold);
    return lost || c.forecast.short_next || army;
}

void DaiDirector::score(const DaiTurnCtx &c, const DaiGoals &goals, std::array<std::string, (size_t)DaiPosture::Count> *why)
{
    const DaiKnowledge &k = c.know;
    const DaiPersonality &p = c.personality;
    char buf[96];
    m_score.fill(0);
    for (std::string &s : *why) s.clear();
    // opening: until the core rooms the programme wants stand and a portal is owned (or none is left to reach).
    bool portal = false;
    portal = portal || k.HasRoomRole(DAI_ROLE_PORTAL);
    bool reachable = false;
    goals.ForEach([&](const DaiGoal &g) {
        reachable = reachable || ((g.kind == DaiGoalKind::ExpandTo) && (g.p.count == DAI_EXPAND_ENTRANCE) && (g.status != DaiGoalStatus::Blocked));
    });
    bool core = true;
    for (DaiRoom r : { DaiRoom::Treasury, DaiRoom::Lair, DaiRoom::Hatchery })
        if ((m_target[(size_t)r] > 0) && k.Build(r).available && (slabs_of(k, r) == 0)) core = false;
    if (!m_opened && (!core || (!portal && reachable))) {
        m_score[(size_t)DaiPosture::Opening] = 1000;
        (*why)[(size_t)DaiPosture::Opening] = !core ? "core rooms to build" : "a portal to reach";
    }
    // build_up: while the programme's rooms are below their size, for at most build_up_max_turns of it.
    const bool done = programme_done(k);
    const bool timed_out = (m_posture == DaiPosture::BuildUp) && (c.now >= m_posture_since + (DaiTurn)c.cfg.director_build_up_max_turns);
    m_score[(size_t)DaiPosture::BuildUp] = (!done && !timed_out) ? 700 : 300;
    (*why)[(size_t)DaiPosture::BuildUp] = !done ? (timed_out ? "rooms below size, timed out" : "rooms below size") : "nothing better";
    // expand: the rooms at size (or build_up timed out) and the runway comfortable.
    if ((done || timed_out) && (c.forecast.runway_paydays >= p.economy_comfortable_paydays)) {
        m_score[(size_t)DaiPosture::Expand] = std::min<int64_t>(600 * p.director_expand_appetite / DAI_FIX_ONE, 950);
        snprintf(buf, sizeof(buf), "rooms at size, runway %" PRId64 " pay days", c.forecast.runway_paydays);
        (*why)[(size_t)DaiPosture::Expand] = buf;
    }
    // contest: a rival keeper seen lately holds ground near the keeper's rooms, or is near its strength.
    int64_t own_force = k.Own().imps;
    for (const DaiCreatureLook &cr : k.Creatures()) own_force += cr.level;
    const int64_t radius = c.cfg.director_contest_radius;
    for (PlayerNumber r = 0; (r < DAI_MAX_PLAYERS) && (m_score[(size_t)DaiPosture::Contest] == 0); r++) {
        if (!recent(c, r)) continue;
        const int64_t force = k.KnownForce(r, c.now);
        if ((force > 0) && (force * DAI_FIX_ONE >= own_force * p.director_contest_threshold)) {
            snprintf(buf, sizeof(buf), "rival p%d strength %" PRId64 " against %" PRId64, (int)r, force, own_force);
            m_score[(size_t)DaiPosture::Contest] = 800;
            (*why)[(size_t)DaiPosture::Contest] = buf;
            break;
        }
        int64_t nearest = INT64_MAX;
        for (const DaiRoomLook &room : k.Rooms()) {
            const DaiSlabCoord x0 = (DaiSlabCoord)std::max<int64_t>(room.box.x0 - radius, 0);
            const DaiSlabCoord y0 = (DaiSlabCoord)std::max<int64_t>(room.box.y0 - radius, 0);
            const DaiSlabCoord x1 = (DaiSlabCoord)std::min<int64_t>(room.box.x1 + radius, k.Width() - 1);
            const DaiSlabCoord y1 = (DaiSlabCoord)std::min<int64_t>(room.box.y1 + radius, k.Height() - 1);
            for (DaiSlabCoord y = y0; y <= y1; y++)
                for (DaiSlabCoord x = x0; x <= x1; x++)
                    if (k.SlabAt(x, y).owner == r) {
                        const int64_t dx = std::max<int64_t>({ (int64_t)room.box.x0 - x, 0, (int64_t)x - room.box.x1 });
                        const int64_t dy = std::max<int64_t>({ (int64_t)room.box.y0 - y, 0, (int64_t)y - room.box.y1 });
                        nearest = std::min(nearest, std::max(dx, dy));
                    }
        }
        if (nearest <= radius) {
            snprintf(buf, sizeof(buf), "rival p%d ground %" PRId64 " slabs from a room", (int)r, nearest);
            m_score[(size_t)DaiPosture::Contest] = 800;
            (*why)[(size_t)DaiPosture::Contest] = buf;
        }
    }
    // raid and assault: from expand or contest (or each other), a best target, not resting from the last operation --
    // from any posture but opening when the keeper is ready and the force is overwhelming or the income near its end
    // (urgency, plan 13 section 30: on
    // level 124 at 60,000 turns 52 keepers with twice the rival's army and more, ready, went defend -> recover -> defend
    // to the end, a pay day short for good on a mined-out map, and never attacked).
    const bool attacking = (m_posture == DaiPosture::Raid) || (m_posture == DaiPosture::Assault);
    const DaiRivalTarget *best = c.rivals.Best();
    const bool may_attack = (p.offence_never_attack == 0) && (c.now >= m_attack_rest_until) && (best != nullptr);
    const DaiReadiness ready = may_attack ? DaiRivals::Readiness(k, p, c.cfg, c.forecast, *best) : DaiReadiness{};
    const bool pressing = may_attack && ready.Ready() && (ready.overwhelming || (ready.urgency > 0));
    const bool from = attacking || (m_posture == DaiPosture::Expand) || (m_posture == DaiPosture::Contest)
                      || (pressing && (m_posture != DaiPosture::Opening));
    if (from && may_attack) {
        snprintf(buf, sizeof(buf), "p%d's %s, score %" PRId64 "/1024%s", (int)best->rival, DaiTargetKindName(best->kind), (int64_t)best->score,
            ready.Ready() ? ", ready" : "");
        // An operation under way holds assault, ready or not: its own fighting and losses take readiness away (07 P5.12:
        // a breach that opened the way lost its Attack goal as the keeper's creatures went to work, NO_GOAL).
        bool under_way = false;
        if ((m_posture == DaiPosture::Assault) && (m_attack_rival >= 0)) {
            const DaiGoal *g = goals.Find(DaiGoalKind::Attack, (int64_t)m_attack_rival + 1);
            under_way = (g != nullptr) && (g->status == DaiGoalStatus::Progress);
        }
        if (ready.Ready() || under_way) {
            m_score[(size_t)DaiPosture::Assault] = 950;
            (*why)[(size_t)DaiPosture::Assault] = buf;
        }
        if (p.offence_raid_appetite > 0) {
            m_score[(size_t)DaiPosture::Raid] = 900;
            (*why)[(size_t)DaiPosture::Raid] = buf;
        }
    }
    // recover: a room lost, the next pay day short, or recover_threshold of the creatures gone -- not over an attack the
    // force presses (the economy's emergency selling does not wait on the posture; the rival's gold is what is left).
    if (recover_trigger(c) && !(pressing && from)) {
        m_score[(size_t)DaiPosture::Recover] = 1000;
        (*why)[(size_t)DaiPosture::Recover] = c.forecast.short_next ? "next pay day short" : "a room or creatures lost";
    }
    // finish (P9): from an Armageddon's teleport (whoever cast it) the whole map is revealed, and a keeper loses only
    // with no creatures left -- its survivors walk home from the last fight. Until armageddon_finish_turns after it is
    // over: assault the best target's keeper once the creatures' levels are armageddon_superiority % of its, whatever the
    // posture, the readiness, the rest or the losses (defend still first while an enemy is on its ground).
    m_finishing = false;
    const DaiOwnLook &own = k.Own();
    if (own.armageddon && (c.now >= own.armageddon_teleport))
        m_finish_until = std::max<DaiTurn>(m_finish_until, own.armageddon_over + (DaiTurn)p.magic_armageddon_finish_turns);
    if ((c.now < m_finish_until) && (p.offence_never_attack == 0) && (best != nullptr)) {
        int64_t mine = 0;
        for (const DaiCreatureLook &cr : k.Creatures()) if (!k.Kind(cr.model).digger) mine += cr.level;
        const int64_t theirs = k.KnownForce(best->rival, c.now);
        if (mine * 100 >= p.magic_armageddon_superiority * theirs) {
            m_finishing = true;
            m_score[(size_t)DaiPosture::Assault] = 1000;
            m_score[(size_t)DaiPosture::Recover] = 0;
            snprintf(buf, sizeof(buf), "finish p%d: %" PRId64 " levels against %" PRId64 " after an Armageddon", (int)best->rival, mine, theirs);
            (*why)[(size_t)DaiPosture::Assault] = buf;
        }
    }
    // defend: an enemy on the keeper's ground within defend_cooldown turns -- the fight before everything else, the
    // repairs and the opening included (opening takes up again after it).
    if (defending(c)) {
        for (int64_t &s : m_score) s = std::min<int64_t>(s, 900);
        m_score[(size_t)DaiPosture::Defend] = 1000;
        snprintf(buf, sizeof(buf), "%s, turn %" PRIu64, m_threat_why, (uint64_t)m_threat_turn);
        (*why)[(size_t)DaiPosture::Defend] = buf;
    }
}

void DaiDirector::watch_threats(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    // An enemy force on its ground near home: its creatures' levels (imps apart: they dig and claim; Tactics deals with them and
    // with a lone scout) at [defence] threat_force or more (07 P5.12: on level 60 Computer2's flies and imps crossing the
    // border kept the keeper in defend for most of the game, and so out of every attack).
    const char *what = nullptr;
    char seen[64];
    int64_t force = 0, count = 0;
    PlayerNumber who = -1;
    // ...and near home: within contest_radius of its heart or of one of its room slabs (bridges apart). P5.12: a breach's
    // claimed way runs to the rival's door, and the rival's creatures on it turned the assault into defend as it began.
    const int64_t radius = c.cfg.director_contest_radius;
    for (const DaiThingLook &e : k.Enemies()) {
        const DaiSlabCoord ex = DaiSubtileSlab(e.x), ey = DaiSubtileSlab(e.y);
        if (e.digger || (k.SlabAt(ex, ey).owner != c.player) || !k.NearHome(c.player, ex, ey, radius)) continue;
        force += std::max<int64_t>(e.level, 1);
        count++;
        if (who < 0) who = e.owner;
    }
    const bool threat = (force >= c.cfg.defence_threat_force);
    if (threat) {
        snprintf(seen, sizeof(seen), "%" PRId64 " creatures of p%d (levels %" PRId64 ") on its ground", count, (int)who, force);
        what = seen;
    }
    const DaiOwnLook &own = k.Own();
    if ((what == nullptr) && own.has_heart && (m_heart_seen >= 0) && (own.heart_health < m_heart_seen)) what = "its heart hurt";
    m_heart_seen = own.has_heart ? own.heart_health : -1;
    const uint32_t mask = (1u << KF_Intruder) | (1u << KF_WallCue) | (1u << KF_TrapFired);
    DaiTurn newest = m_facts_seen;
    for (int64_t i = c.inbox.Next(mask, 0); i >= 0; i = c.inbox.Next(mask, i + 1)) {
        const DaiInboxEntry &e = c.inbox.Get(i);
        if (e.turn <= m_facts_seen) continue;
        newest = std::max(newest, e.turn);
        if (e.kind == KF_TrapFired) {
            // Only its own alarm: the other traps fire at whatever walks by, alarms are what warn.
            const DaiStockLook &st = k.Stock();
            const int64_t model = e.args[1];
            if ((e.args[2] != c.player) || (strcmp(st.Name(false, model), "ALARM") != 0)) continue;
            if (what == nullptr) what = "its alarm";
        } else if ((e.kind == KF_WallCue) && (what == nullptr)) {
            what = "a breach";
        }
        // An intruder fact is the same presence the scan above weighs: it counts only at threat_force.
    }
    m_facts_seen = newest;
    if (what == nullptr) return;
    m_threat_turn = c.now;
    snprintf(m_threat_why, sizeof(m_threat_why), "%s", what);
}

bool DaiDirector::defending(const DaiTurnCtx &c) const
{
    return (m_threat_turn != 0) && (c.now < m_threat_turn + (DaiTurn)c.cfg.defence_defend_cooldown);
}

/** defend: one Defend goal for the keeper's ground; contest: one against each rival it contests (a rival seen lately
 *  that is strong or near), so Defence fortifies against that rival's routes first. Others are dropped. */
void DaiDirector::defend_goals(const DaiTurnCtx &c, DaiGoals &goals)
{
    std::array<bool, DAI_MAX_PLAYERS + 1> want{};
    want[DAI_DEFEND_HOME] = (m_posture == DaiPosture::Defend);
    if (m_posture == DaiPosture::Contest)
        for (PlayerNumber r = 0; r < DAI_MAX_PLAYERS; r++) want[(size_t)r + 1] = recent(c, r);
    for (size_t key = 0; key < want.size(); key++) {
        if (want[key]) {
            const DaiFix base = (key == DAI_DEFEND_HOME) ? W_DEFEND_HOME : W_DEFEND_RIVAL;
            goals.Set(goal(DaiGoalKind::Defend, c.requests.Folded(DaiGoalKind::Defend, (int64_t)key, base), 0, (int64_t)key), c.now);
        } else if (const DaiGoal *g = goals.Find(DaiGoalKind::Defend, (int64_t)key)) {
            goals.DropOwn(g->id);
        }
    }
}

/** raid and assault: the Attack goal on the best target's rival (kept on the rival it names while that one is a keeper
 *  and the goal is in progress), raids only in raid; none in any other posture. */
void DaiDirector::attack_goal(const DaiTurnCtx &c, DaiGoals &goals)
{
    const bool raid = (m_posture == DaiPosture::Raid), assault = (m_posture == DaiPosture::Assault);
    PlayerNumber rival = -1;
    if (raid || assault) {
        const DaiGoal *held = (m_attack_rival >= 0) ? goals.Find(DaiGoalKind::Attack, (int64_t)m_attack_rival + 1) : nullptr;
        const bool keep = (held != nullptr) && (held->status == DaiGoalStatus::Progress) && c.know.RivalKeeper(m_attack_rival);
        const DaiRivalTarget *best = c.rivals.Best();
        rival = keep ? m_attack_rival : (best != nullptr) ? best->rival : m_attack_rival;
        if ((rival >= 0) && !c.know.RivalKeeper(rival)) rival = -1;
    }
    std::vector<uint32_t> drop;
    // Its own only: an Attack goal given as a directive (a human's or an agent's order, Ftests') stays.
    goals.ForEach([&](const DaiGoal &g) {
        if ((g.kind == DaiGoalKind::Attack) && (g.source == DaiGoalSource::Director) && (g.p.key != (int64_t)rival + 1)) drop.push_back(g.id);
    });
    for (uint32_t id : drop) goals.DropOwn(id);
    m_attack_rival = rival;
    if (rival < 0) return;
    const DaiGoal *given = goals.Find(DaiGoalKind::Attack, (int64_t)rival + 1);
    if ((given != nullptr) && (given->source != DaiGoalSource::Director)) return;      // a directive on that rival already
    goals.Set(goal(DaiGoalKind::Attack, c.requests.Folded(DaiGoalKind::Attack, (int64_t)rival + 1, W_ATTACK), raid ? DAI_ATTACK_RAIDS : m_finishing ? DAI_ATTACK_FINISH : 0,
        (int64_t)rival + 1), c.now);
}

void DaiDirector::posture(const DaiTurnCtx &c, const DaiGoals &goals)
{
    const int64_t creatures = c.know.CreatureSummary().creatures;
    if ((creatures >= m_army_peak) || (c.now >= m_army_peak_turn + (DaiTurn)c.cfg.director_recover_window)) {
        m_army_peak = creatures;
        m_army_peak_turn = c.now;
    }
    watch_threats(c);
    // An operation ended: rest from raids and assaults, and look again at once.
    bool ended = false;
    if ((m_posture == DaiPosture::Raid) || (m_posture == DaiPosture::Assault)) {
        goals.ForEach([&](const DaiGoal &g) {
            ended = ended || ((g.kind == DaiGoalKind::Attack) && ((g.status == DaiGoalStatus::Met) || (g.status == DaiGoalStatus::Blocked))
                              && (g.status_turn >= m_posture_since));
        });
        if (ended && (c.now >= m_attack_rest_until)) m_attack_rest_until = c.now + (DaiTurn)c.cfg.director_attack_rest;
    }
    const bool trigger = ((m_posture != DaiPosture::Recover) && recover_trigger(c)) || ((m_posture != DaiPosture::Defend) && defending(c)) || ended;
    if (!trigger && (c.now < m_next_posture)) return;
    m_next_posture = c.now + (DaiTurn)c.cfg.director_interval;
    std::array<std::string, (size_t)DaiPosture::Count> why;
    score(c, goals, &why);
    DaiPosture best = m_posture;
    for (size_t i = 0; i < m_score.size(); i++) if (m_score[i] > m_score[(size_t)best]) best = (DaiPosture)i;
    if (best == m_posture) return;
    // Hysteresis: a clear win, held long enough -- but losses and enemies matter now, opening ends when it is done,
    // and defend when its cooldown has run out.
    const bool held = c.now >= m_posture_since + (DaiTurn)c.personality.director_dwell_turns;
    // Raid to assault is the same attack, ready now: no margin or dwell (07 P5.12: assault's 950 never cleared raid's
    // 900 by switch_margin, so a keeper raiding when it became ready raided on).
    const bool escalate = (m_posture == DaiPosture::Raid) && (best == DaiPosture::Assault);
    const bool finish = m_finishing && (best == DaiPosture::Assault);
    const bool urgent = (best == DaiPosture::Recover) || (best == DaiPosture::Defend) || escalate || finish;
    // A raid or assault with nothing to do any more (resting, no target) is left at once.
    const bool spent = ((m_posture == DaiPosture::Raid) || (m_posture == DaiPosture::Assault)) && (m_score[(size_t)m_posture] == 0);
    if (!held && !urgent && !spent && (m_posture != DaiPosture::Opening) && (m_posture != DaiPosture::Defend)) return;
    if (!escalate && !finish && (m_score[(size_t)best] < m_score[(size_t)m_posture] + c.cfg.director_switch_margin)) return;
    DaiLog::Info(c.player, "posture %s -> %s (%s; score %" PRId64 " vs %" PRId64 ")", DaiPostureName(m_posture),
        DaiPostureName(best), why[(size_t)best].c_str(), m_score[(size_t)best], m_score[(size_t)m_posture]);
    snprintf(m_why, sizeof(m_why), "%s", why[(size_t)best].c_str());
    if ((m_posture == DaiPosture::Opening) && (best != DaiPosture::Defend)) m_opened = true;
    m_posture = best;
    m_posture_since = c.now;
    m_next_scan = c.now;            // the ExpandTo weights at once
    m_next_programme = c.now;       // and the Build goals
}

void DaiDirector::Update(const DaiTurnCtx &c, DaiGoals &goals)
{
    if (c.now >= m_next_scan) {
        m_next_scan = c.now + (DaiTurn)c.cfg.excavation_rescore_interval;
        expansion(c, goals);
    }
    programme(c, goals);
    // The posture, from the programme's targets just worked out; a change reweighs ExpandTo and Build next turn.
    posture(c, goals);
    DaiGoal imps = goal(DaiGoalKind::KeepImps, c.requests.Folded(DaiGoalKind::KeepImps, 0, FixMul(W_KEEP_IMPS, factor(DaiGoalKind::KeepImps))),
        c.personality.workforce_imps_min);
    imps.source = DaiGoalSource::Maintenance;
    goals.Set(imps, c.now);
    // KeepGold: two pay days of runway; Mine: what Economy's requests make of it -- in contest, near home only (its
    // count: the slabs from the heart a vein may be, 0 anywhere).
    goals.Set(goal(DaiGoalKind::KeepGold, c.requests.Folded(DaiGoalKind::KeepGold, 0, FixMul(W_KEEP_GOLD, factor(DaiGoalKind::KeepGold))), 2),
        c.now);
    const int64_t mine_radius = (m_posture == DaiPosture::Contest) ? 3 * c.cfg.director_contest_radius : 0;
    goals.Set(goal(DaiGoalKind::Mine, c.requests.Folded(DaiGoalKind::Mine, 0, FixMul(W_MINE, factor(DaiGoalKind::Mine))), mine_radius), c.now);
    // Train: Creatures trains while it stands (and the runway allows), to the personality's level -- in expand, contest
    // or raid, with a target the keeper is ready for but for its creatures' level, twice as hard and to
    // attack_level_target (07 P5.12: the level never came, so the attack never did).
    DaiFix train = FixMul(W_TRAIN, factor(DaiGoalKind::Train));
    int64_t train_to = c.personality.creatures_train_target_level;
    // With gold for train_rich_runway pay days in hand, on to train_rich_level: P9, screen i -- its creatures stopped at
    // level 4 and researched with some 65,000 gold unspent, its army behind the rival's until 45,000 turns. In hand: the
    // spare gold (plan 13), not the runway -- the training's own cost shortens the runway, and the level fell back to 4
    // with 36,000 spare (level 124).
    if (c.forecast.SparePaydays() >= c.personality.creatures_train_rich_runway)
        train_to = std::max<int64_t>(train_to, c.personality.creatures_train_rich_level);
    const DaiRivalTarget *best = c.rivals.Best();
    const bool may_push = (m_posture == DaiPosture::Expand) || (m_posture == DaiPosture::Contest) || (m_posture == DaiPosture::Raid);
    if (may_push && (best != nullptr)) {
        const DaiReadiness ready = DaiRivals::Readiness(c.know, c.personality, c.cfg, c.forecast, *best);
        if (!ready.level_ok) {
            train = 2 * W_TRAIN;
            train_to = std::max<int64_t>(train_to, c.personality.offence_attack_level_target);
        }
    }
    // Spare gold (plan 13): training first in any posture but defend, pushed (creatures taken from their work); a recover
    // for losses, not a short pay day, trains as build_up does.
    if (spare_rich(c) && (m_posture != DaiPosture::Defend)) {
        train = std::max<DaiFix>(train, 2 * W_TRAIN);
    } else if ((m_posture == DaiPosture::Recover) && !c.forecast.short_next) {
        train = std::max<DaiFix>(train, FixMul(W_TRAIN, kPostureFactor[(size_t)DaiPosture::BuildUp][5]));
    }
    if (train > 0) {
        goals.Set(goal(DaiGoalKind::Train, c.requests.Folded(DaiGoalKind::Train, 0, train), train_to), c.now);
    } else if (const DaiGoal *g = goals.Find(DaiGoalKind::Train)) {
        goals.DropOwn(g->id);
    }
    // recover: nobody dismissed.
    if (m_posture == DaiPosture::Recover) {
        goals.Set(goal(DaiGoalKind::Forbid, DAI_FIX_ONE, 0, DAI_FORBID_DISMISS), c.now);
    } else if (const DaiGoal *g = goals.Find(DaiGoalKind::Forbid, DAI_FORBID_DISMISS)) {
        goals.DropOwn(g->id);
    }
    defend_goals(c, goals);
    attack_goal(c, goals);
    // Manufacture: workshop workers, while a manager asks for them (Construction's stock planner, 04 section 9).
    const int64_t workers = c.requests.CountFloor(DaiGoalKind::Manufacture, 0);
    if (workers > 0) {
        goals.Set(goal(DaiGoalKind::Manufacture, c.requests.Folded(DaiGoalKind::Manufacture, 0, W_BUILD_ASKED), workers), c.now);
    } else if (const DaiGoal *m = goals.Find(DaiGoalKind::Manufacture)) {
        goals.DropOwn(m->id);
    }
}

int64_t DaiDirector::SlabsFor(const DaiKnowledge &k, uint64_t role, int64_t units)
{
    if (units <= 0) return 0;
    int64_t capacity = 0, slab_eff = 0;
    for (const DaiRoomLook &r : k.Rooms()) {
        if ((r.roles & role) == 0) continue;
        capacity += r.capacity;
        slab_eff += r.slabs * r.efficiency;
    }
    // Capacity a slab at full efficiency, x256; then at the planned efficiency -- or the efficiency the keeper's rooms
    // of the role have, when it is lower (rooms not walled in yet: sized for the planned one, the lair fell short).
    const int64_t full = ((capacity > 0) && (slab_eff > 0)) ? std::max<int64_t>(capacity * 256 * 256 / slab_eff, 1) : 256;
    int64_t slabs = 0;
    slabs += k.SlabsWithRole(role);
    const int64_t have_eff = (slabs > 0) ? slab_eff / slabs : DAI_PLAN_EFFICIENCY;
    const int64_t planned = std::max<int64_t>(full * std::min<int64_t>(DAI_PLAN_EFFICIENCY, std::max<int64_t>(have_eff, 1)) / 256, 1);
    return (units * 256 + planned - 1) / planned;
}

int64_t DaiDirector::WorkshopSlabsFor(const DaiStockLook &st)
{
    // Each item needs a manufacture level, and the level comes from the workshop's slabs: more than (level + 2)^2.
    int64_t level = 0;
    st.ForEachKind(false, [&](int64_t s, int64_t) { if (st.traps_buildable[s]) level = std::max(level, st.trap_levels[s]); });
    st.ForEachKind(true, [&](int64_t s, int64_t) { if (st.doors_buildable[s]) level = std::max(level, st.door_levels[s]); });
    return (level > 0) ? (level + 2) * (level + 2) + 1 : 0;
}

int64_t DaiDirector::FoodCapacityNeeded(const DaiKnowledge &k)
{
    // Demand, in millionths of a chicken a turn; supply from capacity C is C / (G + C) a turn, so C = G D / (1 - D).
    constexpr int64_t MILLION = 1000000;
    int64_t demand = 0;
    for (const DaiCreatureLook &c : k.Creatures()) {
        const DaiKindLook &kind = k.Kind(c.model);
        if (kind.hunger_rate > 0) demand += kind.hunger_fill * MILLION / kind.hunger_rate;
    }
    const int64_t speed = k.Own().food_generation_speed;
    if ((demand <= 0) || (speed <= 0)) return 0;
    if (demand >= MILLION) return speed;            // more than a chicken a turn: as big as it can usefully be
    return (speed * demand + (MILLION - demand) - 1) / (MILLION - demand);
}

/** A kind the keeper may attract comes to this room (one of its entrance rooms). */
bool DaiDirector::Draws(const DaiKnowledge &k, DaiRoom r)
{
    const RoomKind room = k.RoomKindOf(r);
    if (room == 0) return false;
    for (DaiThingModel m = 1; m <= (DaiThingModel)k.KindCount(); m++) {
        const DaiKindLook &kind = k.Kind(m);
        if (!kind.known || kind.digger || kind.spectator || !kind.allowed || (kind.in_pool == 0)) continue;
        for (int e = 0; e < 3; e++) if ((kind.entrance_rooms[e] == room) && (kind.entrance_slabs[e] > 0)) return true;
    }
    return false;
}

/** What the keeper wants now (04 section 4's table): every room it has a rank for, when its condition holds, at its
 *  starting size or what its creatures need. */
void DaiDirector::targets(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    const DaiPersonality &p = c.personality;
    const DaiCreatureSummary &cs = k.CreatureSummary();
    bool portal = false;
    portal = portal || k.HasRoomRole(DAI_ROLE_PORTAL);
    bool manufacture = false;
    const DaiStockLook &st = k.Stock();
    for (int64_t i = 0; i < DaiStockLook::MAX_SLOTS; i++) manufacture = manufacture || st.traps_buildable[i] || st.doors_buildable[i];
    // The lair: every creature's lair size, and room for lair_spare more of the average size.
    int64_t lair_units = 0;
    for (const DaiCreatureLook &cr : k.Creatures()) lair_units += k.Kind(cr.model).lair_size;
    const int64_t avg_lair = (cs.creatures > 0) ? std::max<int64_t>((lair_units + cs.creatures - 1) / cs.creatures, 1) : 1;
    lair_units += c.cfg.programme_lair_spare * avg_lair;
    const bool extras = room_for_extras(c);
    for (size_t i = 0; i < m_target.size(); i++) {
        const DaiRoom r = (DaiRoom)i;
        const int64_t rank = p.ProgrammeRank(r);
        int64_t slabs = p.ProgrammeSlabs(r);
        bool wanted = (rank > 0);
        switch (r) {
        case DaiRoom::Treasury: wanted = wanted && k.Own().has_heart; break;
        case DaiRoom::Lair:
            wanted = wanted && ((cs.creatures > 0) || portal);
            slabs = std::max(slabs, SlabsFor(k, DAI_ROLE_LAIR, lair_units));
            break;
        case DaiRoom::Hatchery:
            wanted = wanted && (cs.creatures > 0);
            slabs = std::max(slabs, SlabsFor(k, DAI_ROLE_FOOD, FoodCapacityNeeded(k)));
            break;
        // Rooms that need workers: none before the first creature (a library nobody reads in is gold spent early). With
        // the research done the library still draws the kinds that come to one, more the bigger it is (warlocks).
        case DaiRoom::Library:
            wanted = wanted && (cs.creatures > 0) && ((k.Research().count > 0) || Draws(k, r));
            slabs = grown(c, DAI_ROLE_RESEARCH, slabs);
            break;
        case DaiRoom::Training:
            wanted = wanted && (cs.creatures > 0) && (c.forecast.runway_paydays >= c.cfg.programme_train_runway);
            // With spare gold, room for half the creatures to train at once (plan 13).
            if (spare_rich(c)) slabs = std::max(slabs, SlabsFor(k, DAI_ROLE_TRAIN, (cs.creatures + 1) / 2));
            slabs = grown(c, DAI_ROLE_TRAIN, slabs);
            break;
        case DaiRoom::Workshop:
            wanted = wanted && (cs.creatures > 0) && manufacture;
            if (extras) slabs = std::max(slabs, WorkshopSlabsFor(st));
            slabs = grown(c, DAI_ROLE_MANUFACTURE, slabs);
            break;
        // A group of more than one needs a barracks, of any size (06 section 5a.2): its programme size, once there are
        // creatures to group.
        case DaiRoom::Barracks: wanted = (wanted || (p.tactics_group_size > 1)) && (cs.creatures > 1); break;
        default: break;
        }
        m_target[i] = wanted ? slabs : 0;
    }
    if (portal) attraction(c, (m_posture == DaiPosture::Expand) && extras);
}

/** Rooms a creature needs before it comes (the engine's creature_will_generate_for_dungeon: every one of its entrance
 *  rooms, of at least its slabs, counted over all the keeper's rooms of that kind). When no kind the keeper may attract
 *  could come to the rooms it has and wants, it wants the rooms of the kind that needs the fewest more slabs, all of
 *  them rooms it can build -- else a map whose creatures each want a library, workshop or hatchery first never gets
 *  the creature that would make it want one (P2.11, map 55). With `more` (expand: attraction rooms, 05 section 6.2)
 *  it does so whether or not a kind can come already: one more kind each time the programme is worked out. */
int64_t DaiDirector::KindWorth(const DaiKnowledge &k, const DaiPersonality &p, DaiThingModel m, int64_t level)
{
    const DaiKindLook &kind = k.Kind(m);
    // What it becomes: its fight at the level it is trained to (train_target_level) or its own if higher -- a level-1
    // dragon's or warlock's fight is little, theirs at 4 among the best (P9: judged at 1, they were dismissed first) --
    // and its work, its best skill at research or making times [creatures] work_worth.
    const size_t lv = (size_t)std::clamp<int64_t>(std::max(level, p.creatures_train_target_level) - 1, 0, DaiKindLook::LEVELS - 1);
    // At least 1: kinds of no known worth weigh the same, the fewest slabs missing first (as before P9).
    const int64_t fight = std::max<int64_t>((kind.output[lv] > 0) ? kind.output[lv] : kind.score[lv], 1);
    const int64_t work = std::max(kind.research_value, kind.manufacture_value) * p.creatures_work_worth;
    return std::max<int64_t>(FixToInt(FixMul(FixFromInt(fight + work), p.Favour(kind.code))), 1);
}

bool DaiDirector::KindCanCome(const DaiKnowledge &k, DaiThingModel m)
{
    const DaiKindLook &kind = k.Kind(m);
    if (!kind.known || kind.digger || kind.spectator || !kind.allowed || (kind.in_pool == 0)) return false;
    if (kind.forced) return true;
    for (int e = 0; e < 3; e++) {
        if ((kind.entrance_rooms[e] == 0) || (kind.entrance_slabs[e] <= 0)) continue;
        int64_t slabs = 0;
        for (const DaiRoomLook &r : k.Rooms()) if (r.kind == kind.entrance_rooms[e]) slabs += r.slabs;
        if (slabs < kind.entrance_slabs[e]) return false;
    }
    return true;
}

/** The rooms that draw creatures (04 section 7.1): of the kinds that cannot come yet, the one worth most to the keeper
 *  for the slabs it still needs -- its worth (KindWorth: its fight, the personality's favour) over those slabs and
 *  ATTRACT_SLABS more, all of them rooms it can build -- gets its rooms in the programme. While some kind can come
 *  already, only with `more` (expand: attraction rooms, 05 section 6.2) or when the kind is worth the personality's
 *  attract_upgrade more than the draw's mean (P9: picking the fewest slabs first, level 124 got beetles and spiders
 *  while its rival drew bile demons, dragons and mistresses). Else a map whose creatures each want a library, workshop
 *  or hatchery first never gets the creature that would make it want one (P2.11, map 55). */
void DaiDirector::attraction(const DaiTurnCtx &c, bool more)
{
    const DaiKnowledge &k = c.know;
    constexpr int64_t ATTRACT_SLABS = 20;   // a kind's slabs wanted are weighed with this many more
    std::array<int64_t, (size_t)DaiRoom::Count> have{};
    for (size_t i = 0; i < have.size(); i++) {
        const RoomKind kind = k.RoomKindOf((DaiRoom)i);
        for (const DaiRoomLook &r : k.Rooms()) if ((kind != 0) && (r.kind == kind)) have[i] += r.slabs;
        have[i] = std::max(have[i], m_target[i]);
    }
    int64_t best = -1, best_value = 0, best_worth = 0;
    // The draw: the portal picks among every kind that can come, weighted by its entrance score (the engine adds a
    // share for spare room capacity, left out here), so a new kind is weighed against the draw's mean worth -- not the
    // best of them (level 124: with dragons able to come, bile demons were never wanted, and the beetles and flies
    // came).
    int64_t draw_worth = 0, draw_weight = 0;
    bool can_come = false;
    auto comes = [&](const DaiKindLook &kind, int64_t worth) {
        can_come = true;
        const int64_t wgt = std::max<int64_t>(kind.entrance_score, 1);
        draw_worth += worth * wgt;
        draw_weight += wgt;
    };
    for (DaiThingModel m = 1; m <= (DaiThingModel)k.KindCount(); m++) {
        const DaiKindLook &kind = k.Kind(m);
        if (!kind.known || kind.digger || kind.spectator || !kind.allowed || (kind.in_pool == 0)) continue;   // 0: known none left (Q8)
        const int64_t worth = KindWorth(k, c.personality, m);
        if (kind.forced) {
            comes(kind, worth);
            continue;
        }
        int64_t missing = 0;
        bool buildable = true;
        for (int e = 0; e < 3; e++) {
            if ((kind.entrance_rooms[e] == 0) || (kind.entrance_slabs[e] <= 0)) continue;
            size_t r = have.size();
            for (size_t i = 0; i < have.size(); i++) if (k.RoomKindOf((DaiRoom)i) == kind.entrance_rooms[e]) r = i;
            if (r == have.size()) { missing += kind.entrance_slabs[e]; buildable = false; continue; }
            missing += std::max<int64_t>(kind.entrance_slabs[e] - have[r], 0);
            buildable = buildable && k.Build((DaiRoom)r).available;
        }
        if (missing == 0) {                     // one can come already
            comes(kind, worth);
            continue;
        }
        const int64_t value = worth * 1000 / (missing + ATTRACT_SLABS);
        if (buildable && (value > best_value)) { best = m; best_value = value; best_worth = worth; }
    }
    if (best < 0) return;
    const int64_t can_come_worth = (draw_weight > 0) ? draw_worth / draw_weight : 0;
    const bool upgrade = FixToInt(FixMul(FixFromInt(can_come_worth), c.personality.creatures_attract_upgrade)) < best_worth;
    const bool act = !can_come || more || upgrade;
    {
        const DaiKindLook &b = k.Kind((DaiThingModel)best);
        int64_t missing = 0;
        for (int e = 0; e < 3; e++)
            for (size_t i = 0; i < have.size(); i++)
                if ((b.entrance_rooms[e] != 0) && (k.RoomKindOf((DaiRoom)i) == b.entrance_rooms[e]))
                    missing += std::max<int64_t>(b.entrance_slabs[e] - have[i], 0);
        DaiLog::Info(c.player, "attraction: best to draw %s (%" PRId64 " slabs missing, worth %" PRId64 "), the draw worth %"
            PRId64 "%s%s: %s", b.code, missing, best_worth, can_come_worth, more ? ", expanding" : "", upgrade ? ", an upgrade" : "",
            act ? "its rooms wanted" : "not now");
    }
    if (!act) return;
    const DaiKindLook &kind = k.Kind((DaiThingModel)best);
    for (int e = 0; e < 3; e++)
        for (size_t i = 0; i < have.size(); i++)
            if ((kind.entrance_rooms[e] != 0) && (k.RoomKindOf((DaiRoom)i) == kind.entrance_rooms[e]))
                m_target[i] = std::max(m_target[i], kind.entrance_slabs[e]);
}

/** The Build goals: worked out every [programme] interval or when the creatures or rooms change, each update with the
 *  managers' requests folded in. A room the keeper cannot build yet has a goal only while the research is on it,
 *  at half weight (a site can be got ready). */
void DaiDirector::programme(const DaiTurnCtx &c, DaiGoals &goals)
{
    const DaiKnowledge &k = c.know;
    const int64_t creatures = k.CreatureSummary().creatures, rooms = (int64_t)k.Rooms().size();
    if ((c.now >= m_next_programme) || (creatures != m_seen_creatures) || (rooms != m_seen_rooms)) {
        targets(c);
        m_next_programme = c.now + (DaiTurn)c.cfg.programme_interval;
        m_seen_creatures = creatures;
        m_seen_rooms = rooms;
    }
    const DaiResearchLook &research = k.Research();
    const bool extras = room_for_extras(c);
    for (size_t i = 0; i < m_target.size(); i++) {
        const DaiRoom r = (DaiRoom)i;
        int64_t floor = c.requests.CountFloor(DaiGoalKind::Build, (int64_t)i);
        // A manager's new extra room waits for room in the economy; one already built keeps its goal.
        if (extra_room(r) && !extras && (slabs_of(k, r) == 0)) floor = 0;
        int64_t target = std::max(m_target[i], floor);
        // Recover: no new rooms beyond the core -- unless the gold is spare (a recover for losses rebuilds, plan 13).
        if ((m_posture == DaiPosture::Recover) && !core_room(r) && !spare_rich(c)) target = 0;
        const bool available = k.Build(r).available;
        const RoomKind kind = k.RoomKindOf(r);
        const bool coming = !available && (kind != 0) && (research.count > 0)
            && (research.items[0].kind == DaiResearchKind::Room) && (research.items[0].which == kind);
        if ((target <= 0) || (!available && !coming)) {
            if (const DaiGoal *g = goals.Find(DaiGoalKind::Build, (int64_t)i)) goals.DropOwn(g->id);
            continue;
        }
        const int64_t rank = c.personality.ProgrammeRank(r);
        DaiFix base = (rank > 0) ? std::max<DaiFix>(W_BUILD_LAST, DAI_FIX_ONE - W_BUILD_STEP * (rank - 1)) : W_BUILD_ASKED;
        if (coming) base /= 2;
        base = FixMul(base, factor(DaiGoalKind::Build));
        goals.Set(goal(DaiGoalKind::Build, c.requests.Folded(DaiGoalKind::Build, (int64_t)i, base), target, (int64_t)i), c.now);
    }
}

void DaiDirector::Save(DaiBlobWriter &w) const
{
    w.U64(m_next_scan);
    w.U64(m_next_programme);
    w.I64(m_seen_creatures);
    w.I64(m_seen_rooms);
    w.I64((int64_t)m_target.size());
    for (int64_t t : m_target) w.I64(t);
    // Phase 3 (DIRC v3): the posture.
    w.I64((int64_t)m_posture);
    w.U64(m_posture_since);
    w.U64(m_next_posture);
    for (int64_t s : m_score) w.I64(s);
    w.I64(m_army_peak);
    w.U64(m_army_peak_turn);
    w.Str(m_why);
    // Phase 4 (DIRC v4): the defend posture's trigger.
    w.U64(m_threat_turn);
    w.U64(m_facts_seen);
    w.I64(m_heart_seen);
    w.Str(m_threat_why);
    w.Bool(m_opened);
    // Phase 5 (DIRC v5): raid and assault.
    w.U64(m_attack_rest_until);
    w.I64(m_attack_rival);
    // P9 (DIRC v6): the finish after an Armageddon.
    w.U64(m_finish_until);
}

bool DaiDirector::Load(DaiBlobReader &r)
{
    m_next_scan = (DaiTurn)r.U64();
    m_next_programme = (DaiTurn)r.U64();
    m_seen_creatures = r.Range(-1, INT32_MAX);
    m_seen_rooms = r.Range(-1, INT32_MAX);
    m_target.fill(0);
    const int64_t n = r.Count((int64_t)m_target.size());
    for (int64_t i = 0; (i < n) && r.Ok(); i++) m_target[(size_t)i] = r.Range(0, 1000000);
    m_posture = (DaiPosture)r.Range(0, (int64_t)DaiPosture::Count - 1);
    m_posture_since = (DaiTurn)r.U64();
    m_next_posture = (DaiTurn)r.U64();
    for (int64_t &s : m_score) s = r.Range(0, 1000);
    m_army_peak = r.Range(0, INT32_MAX);
    m_army_peak_turn = (DaiTurn)r.U64();
    snprintf(m_why, sizeof(m_why), "%s", r.Str().c_str());
    m_threat_turn = (DaiTurn)r.U64();
    m_facts_seen = (DaiTurn)r.U64();
    m_heart_seen = r.Range(-1, INT64_MAX);
    snprintf(m_threat_why, sizeof(m_threat_why), "%s", r.Str().c_str());
    m_opened = r.Bool();
    m_attack_rest_until = (DaiTurn)r.U64();
    m_attack_rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
    m_finish_until = (DaiTurn)r.U64();
    return r.Ok();
}

/** ExpandTo for the nearest neutral entrances (0.9 until the keeper owns one, then 0.3) and rooms (0.2); goals for
 *  targets no longer neutral or no longer among the nearest are dropped. */
void DaiDirector::expansion(const DaiTurnCtx &c, DaiGoals &goals)
{
    struct Target { int64_t key, dist, what; };
    std::vector<Target> entrances, rooms;
    const DaiKnowledge &k = c.know;
    const RoomKind entrance = k.RoomKindOf(DaiRoom::Entrance);
    const DaiSlabCoord hx = DaiSubtileSlab(k.Own().heart_x), hy = DaiSubtileSlab(k.Own().heart_y);
    std::vector<uint8_t> seen((size_t)k.Width() * (size_t)k.Height(), 0);
    std::vector<DaiSlabXY> cluster((size_t)k.Width() * (size_t)k.Height());
    for (DaiSlabCoord y = 0; y < k.Height(); y++) {
        for (DaiSlabCoord x = 0; x < k.Width(); x++) {
            const size_t at = (size_t)y * k.Width() + x;
            const DaiSlabInfo s = k.SlabAt(x, y);
            if (seen[at] || (s.terrain != DaiTerrain::Room) || (s.owner >= 0)) continue;
            const int64_t n = k.RoomCluster(x, y, cluster.data(), (int64_t)cluster.size());
            for (int64_t i = 0; i < n; i++) seen[(size_t)cluster[(size_t)i].y * k.Width() + cluster[(size_t)i].x] = 1;
            const Target t{ (int64_t)at, std::abs(x - hx) + std::abs(y - hy), ((entrance != 0) && (s.room == entrance)) ? DAI_EXPAND_ENTRANCE : DAI_EXPAND_ROOM };
            // contest: away from the rival -- nothing nearer a rival's heart seen lately than the keeper's own.
            bool rival_side = false;
            for (PlayerNumber r = 0; (r < DAI_MAX_PLAYERS) && (m_posture == DaiPosture::Contest); r++) {
                const DaiRivalSighting &rs = k.Sighting(r);
                if (recent(c, r) && rs.heart_seen
                    && (std::abs(x - DaiSubtileSlab(rs.heart_x)) + std::abs(y - DaiSubtileSlab(rs.heart_y)) < t.dist)) rival_side = true;
            }
            if (rival_side) continue;
            (t.what == DAI_EXPAND_ENTRANCE ? entrances : rooms).push_back(t);
        }
    }
    auto nearest = [](std::vector<Target> &v, int64_t n) {
        std::stable_sort(v.begin(), v.end(), [](const Target &a, const Target &b) { return a.dist < b.dist; });
        if ((int64_t)v.size() > n) v.resize((size_t)n);
    };
    nearest(entrances, DAI_EXPAND_ENTRANCES);
    nearest(rooms, DAI_EXPAND_ROOMS);
    int64_t owned_entrances = 0;
    for (const DaiRoomLook &r : k.Rooms()) if ((entrance != 0) && (r.kind == entrance)) owned_entrances++;
    // Drop the ExpandTo goals not wanted any more.
    std::vector<uint32_t> drop;
    goals.ForEach([&](const DaiGoal &g) {
        if ((g.kind != DaiGoalKind::ExpandTo) || (g.source == DaiGoalSource::Script) || (g.source == DaiGoalSource::Directive)) return;
        const auto &list = (g.p.count == DAI_EXPAND_ENTRANCE) ? entrances : rooms;
        if (std::none_of(list.begin(), list.end(), [&](const Target &t) { return t.key == g.p.key; })) drop.push_back(g.id);
    });
    for (uint32_t id : drop) goals.DropOwn(id);
    const DaiFix f = FixMul(factor(DaiGoalKind::ExpandTo), c.personality.director_expand_appetite);
    const DaiFix w_entrance = FixMul((owned_entrances == 0) ? W_FIRST_ENTRANCE : W_MORE_ENTRANCES, f);
    for (const Target &t : entrances)
        goals.Set(goal(DaiGoalKind::ExpandTo, c.requests.Folded(DaiGoalKind::ExpandTo, t.key, w_entrance), t.what, t.key), c.now);
    for (const Target &t : rooms)
        goals.Set(goal(DaiGoalKind::ExpandTo, c.requests.Folded(DaiGoalKind::ExpandTo, t.key, FixMul(W_NEUTRAL_ROOM, f)), t.what, t.key), c.now);
}
