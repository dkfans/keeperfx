/** @file DaiDirector.h
 *  The director (02 section 4.9): runs before the managers and sets the keeper's goals, each weight with the managers'
 *  goal-weight requests folded in (DaiRequests::Folded). Phase 1's fixed goals (03 section 8.4): KeepImps, KeepGold,
 *  Mine, and ExpandTo for the neutral entrances and rooms the keeper knows (looked for every rescore_interval turns,
 *  the nearest few to its heart). Phase 2's room programme (04 section 4): a Build goal for each room the keeper
 *  wants (key: the DaiRoom; count: its target slabs), ranked by the personality, sized from the keeper's creatures
 *  and the managers' requests. Phase 3's postures (05 section 6.2): opening, build_up, expand, contest, recover, each
 *  scored from the keeper's state; the one held scales every goal's base weight (kPostureFactor) and adds its own
 *  goals (Train, the dismissal Forbid), with hysteresis -- a switch needs switch_margin more score and dwell_turns
 *  held, except that recover interrupts at once and opening ends as soon as it is done. Phase 4's defend (06 section 11):
 *  entered at once on an enemy on the keeper's ground, an intruder or breach fact, its own alarm going off or its heart
 *  hurt; left once none has been seen for [defence] defend_cooldown turns. It outranks every other posture (the fight
 *  first; an opening it interrupts takes up again after it); a Defend goal (key DAI_DEFEND_HOME) gives Defence its
 *  share of the gestures (the arbiter) while construction slows. contest posts a Defend goal against each contested rival (key 1 + the rival). A personality whose group_size is
 *  above 1 gets a barracks in its programme (06 section 5a.2). Phase 5's raid and assault (07 section 5): taken from
 *  expand or contest (or each other) when DaiRivals has a best target (a way in, at attack_threshold or above), unless
 *  never_attack; assault once the keeper is ready for it (DaiRivals::Readiness), raid while it is not (with any
 *  raid_appetite). Both post the Attack goal on the target's rival (key 1 + it; raid's count DAI_ATTACK_RAIDS: raids
 *  only, no operation), kept on that rival while it stands and the goal is in progress; dropped in every other
 *  posture. When the operation ends (the goal Met or Blocked) the keeper rests from both for [director] attack_rest
 *  turns, and leaves them at once. defend and recover outrank both. */
#ifndef DAIMON_DAIDIRECTOR_H
#define DAIMON_DAIDIRECTOR_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>
#include <string>

#include "daimon/DaiBlob.h"
#include "daimon/DaiGoals.h"
#include "daimon/DaiTurnCtx.h"

/** ExpandTo's kind of target (DaiGoalParams::count): a neutral entrance, or another neutral room. */
constexpr int64_t DAI_EXPAND_ENTRANCE = 0, DAI_EXPAND_ROOM = 1;
/** An ExpandTo goal's count: a level script's DAIMON_DIG -- a route dug and claimed to its one slab (Phase 7). */
constexpr int64_t DAI_EXPAND_POINT = 2;
/** Entrances and rooms the director keeps goals for at once. */
constexpr int64_t DAI_EXPAND_ENTRANCES = 6, DAI_EXPAND_ROOMS = 3;

/** What the keeper is doing now (05 section 6.2); Phase 4 adds defend, Phase 5 raid and assault (07 section 5). */
enum class DaiPosture : uint8_t { Opening, BuildUp, Expand, Contest, Recover, Defend, Raid, Assault, Count };
const char *DaiPostureName(DaiPosture p);

/** The efficiency a planned room's size assumes (x256): what the layout planner's sites reach once walled. */
constexpr int64_t DAI_PLAN_EFFICIENCY = 218;

class DaiDirector {
public:
    void Update(const DaiTurnCtx &c, DaiGoals &goals);
    /** The programme's own target slabs for a room as last worked out (0: not wanted; requests apart). */
    int64_t Target(DaiRoom r) const { return m_target[(size_t)r]; }
    /** Slabs of a room with role `role` that hold `units` (lair-size units, chickens...) at the planned efficiency,
     *  judged from what the keeper's rooms of that role hold per slab (one unit a slab at full efficiency if none). */
    static int64_t SlabsFor(const DaiKnowledge &k, uint64_t role, int64_t units);
    /** The workshop slabs the most demanding item the keeper can make needs: more than (level + 2)^2 (the engine's
     *  calculate_manufacture_level); 0 when every one is level 0. */
    static int64_t WorkshopSlabsFor(const DaiStockLook &st);
    /** Chickens of hatchery capacity that grow as fast as the keeper's creatures eat: each eats its kind's hunger_fill
     *  every hunger_rate turns, and a hatchery of capacity C grows one every food_generation_speed / C + 1 turns. */
    static int64_t FoodCapacityNeeded(const DaiKnowledge &k);
    /** A kind the keeper may attract comes to room `r` (one of its entrance rooms). */
    static bool Draws(const DaiKnowledge &k, DaiRoom r);
    /** A kind's worth to the keeper at a level (1-based): its fight (DaiKindLook::output, else its score) at that level
     *  or the one it is trained to, its work (best of research and making, times work_worth), times the personality's
     *  favour for it (P9). */
    static int64_t KindWorth(const DaiKnowledge &k, const DaiPersonality &p, DaiThingModel m, int64_t level = 1);
    /** The portal may send the kind now: allowed, in the pool, every entrance room at its slabs (or forced). */
    static bool KindCanCome(const DaiKnowledge &k, DaiThingModel m);
    DaiPosture Posture() const { return m_posture; }
    DaiTurn PostureSince() const { return m_posture_since; }
    int64_t PostureScore(DaiPosture p) const { return m_score[(size_t)p]; }
    /** Why the posture last changed (empty before the first change). */
    const char *PostureWhy() const { return m_why; }
    /** The last turn an enemy threatened the keeper's ground (0: never), and what it was. */
    DaiTurn LastThreat() const { return m_threat_turn; }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    void expansion(const DaiTurnCtx &c, DaiGoals &goals);
    void targets(const DaiTurnCtx &c);
    void attraction(const DaiTurnCtx &c, bool more);
    void programme(const DaiTurnCtx &c, DaiGoals &goals);
    /** The scores (0-1000) of every posture now, with a reason for each that is not 0. */
    void score(const DaiTurnCtx &c, const DaiGoals &goals, std::array<std::string, (size_t)DaiPosture::Count> *why);
    void posture(const DaiTurnCtx &c, const DaiGoals &goals);
    bool recover_trigger(const DaiTurnCtx &c) const;
    /** Notes this turn's threats to the keeper's ground (m_threat_turn); every turn. */
    void watch_threats(const DaiTurnCtx &c);
    bool defending(const DaiTurnCtx &c) const;
    void defend_goals(const DaiTurnCtx &c, DaiGoals &goals);
    bool programme_done(const DaiKnowledge &k) const;
    /** The current posture's factor on a goal kind's base weight (DaiFix). */
    DaiFix factor(DaiGoalKind kind) const;
    DaiTurn m_next_scan = 0;
    DaiTurn m_next_programme = 0;
    int64_t m_seen_creatures = -1, m_seen_rooms = -1;   /**< a change either way works the programme out again */
    std::array<int64_t, (size_t)DaiRoom::Count> m_target{};
    DaiPosture m_posture = DaiPosture::Opening;
    DaiTurn m_posture_since = 0;
    DaiTurn m_next_posture = 0;
    std::array<int64_t, (size_t)DaiPosture::Count> m_score{};
    int64_t m_army_peak = 0;                /**< the most creatures within recover_window (army losses) */
    DaiTurn m_army_peak_turn = 0;
    char m_why[96] = "";
    // Phase 4 (DIRC v4): the defend posture's trigger.
    DaiTurn m_threat_turn = 0;
    DaiTurn m_facts_seen = 0;              /**< the last inbox fact looked at */
    int64_t m_heart_seen = -1;              /**< the heart's health last turn (-1: not yet) */
    char m_threat_why[48] = "";
    bool m_opened = false;                  /**< opening has ended (a defend that interrupts it does not end it) */
    // Phase 5 (DIRC v5): raid and assault.
    void attack_goal(const DaiTurnCtx &c, DaiGoals &goals);
    DaiTurn m_attack_rest_until = 0;       /**< no raid or assault before this (an operation just ended) */
    PlayerNumber m_attack_rival = -1;       /**< the rival the Attack goal names */
    // P9 (DIRC v6): the finish after an Armageddon.
    DaiTurn m_finish_until = 0;            /**< an Armageddon's teleport seen: finishing may assault until this */
    bool m_finishing = false;               /**< the last score's assault is the finish (not saved: scored again) */
};

#endif
