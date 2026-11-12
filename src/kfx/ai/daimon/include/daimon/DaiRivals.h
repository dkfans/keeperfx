/** @file DaiRivals.h
 *  The keeper's rivals as targets of an attack (07 section 3; Offence's, readable by every manager, 02 section 4.13c).
 *  From the knowledge only (what the keeper has seen, P5.1), every [offence] interval turns:
 *   - per rival keeper: its heart if seen; its force (KnownForce, in creature levels) and a growth guess from its lairs
 *     and training rooms seen; the hate it has earned -- ground and rooms it took from the keeper (KF_Loss facts, at
 *     once), its creatures seen on the keeper's ground -- halved every [offence] hate_halflife turns;
 *   - its targets: its heart (seen), each of its portals seen, and its least defended other room seen; each with a
 *     prize (the heart's: the rival removed; a room's: its slabs, a treasury's or library's twice; a portal's), the
 *     defence seen within [offence] defence_radius slabs (its creatures' levels, its doors by health, its traps, its
 *     guard posts; and [offence] response_share % of its estimated force, coming to defend it), and the cost of the way in
 *     (the dig search's Attack model from the keeper's own land, P5.2; searched again every [offence] route_interval);
 *   - each target's score: prize x own force / (defence x superiority_to_attack / 100) / route cost, as DaiFix. None
 *     with never_attack; none against a rival that has earned no hate with only_attack_attackers; first_target's kind
 *     counts double. Best(): the highest with a way in at attack_threshold or above.
 *  Readiness (07 section 3.3) for a target: the free fighters' force at superiority_to_attack of its defence with
 *  home_guard_share of it kept home, their mean level at attack_level_target, the forecast covering [offence]
 *  attack_runway pay days. Saved whole, the route search in progress included. */
#ifndef DAIMON_DAIRIVALS_H
#define DAIMON_DAIRIVALS_H

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "config_daimon.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiDigSearch.h"
#include "daimon/DaiForecast.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiPersonality.h"
#include "daimon/DaiTypes.h"

struct KeeperFact;

enum class DaiTargetKind : uint8_t { Heart, Room, Portal, Count };
const char *DaiTargetKindName(DaiTargetKind k);

struct DaiRivalTarget {
    PlayerNumber rival = -1;
    DaiTargetKind kind = DaiTargetKind::Heart;
    DaiSlabCoord x = 0, y = 0;          /**< the slab aimed at */
    RoomKind room = 0;                  /**< Room, Portal: the game's room kind */
    int64_t room_index = 0;             /**< Room, Portal: the game's room (with the kind, which target it is) */
    int64_t prize = 0;
    int64_t defence = 1;                /**< in creature levels, at least 1 */
    int64_t route_cost = -1;            /**< the Attack search's cost; -1 not searched yet; DAI_DIG_NEVER no way in */
    DaiTurn route_turn = 0;            /**< when it was searched */
    DaiFix score = 0;
};

struct DaiRivalEstimate {
    bool rival = false;                 /**< a rival keeper in the game now */
    bool heart_known = false;
    DaiSubtlCoord heart_x = 0, heart_y = 0;
    int64_t force = 0;                  /**< KnownForce, creature levels */
    int64_t growth = 0;                 /**< creatures its lairs and training rooms seen may add, in levels */
    int64_t hate = 0;                   /**< as at hate_turn (decayed lazily, Hate()) */
    DaiTurn hate_turn = 0;
};

/** Whether the keeper is ready to attack a target now (07 section 3.3), and why not. */
struct DaiReadiness {
    int64_t free_force = 0;             /**< healthy fighters in no fight, in levels */
    int64_t needed = 0;                 /**< superiority_to_attack % of the target's defence */
    int64_t sendable = 0;               /**< what free_force leaves after home_guard_share stays home */
    /** A pack's summon the spare gold pays for (BestSummon), in levels: counted toward the force -- Tactics casts it at
     *  the staging point while the force gathers (Phase 7), the keeper's own ground that a summon needs. */
    int64_t summon_force = 0;
    int64_t mean_level = 0;             /**< of the fighters that would go (the strongest, the home share left), x100 */
    bool force_ok = false, level_ok = false, runway_ok = false;
    /** How near the income's end is (DaiFix 0..1, Urgency()): the level, the runway and the overwhelming force asked
     *  for relax with it, to level 1, no runway and the force just enough when its side's gold is gone -- an attack
     *  taken a few pay days before the map is dry, with gold left for its spells. */
    DaiFix urgency = 0;
    int64_t level_wanted = 0;           /**< the mean level asked for, x100 (attack_level_target, relaxed) */
    int64_t runway_wanted = 0;          /**< the pay days asked for (attack_runway, relaxed) */
    /** The force alone decides (the level and the runway asks waived): sendable at overwhelm_share % of needed or more
     *  -- relaxed toward 100 % with the urgency. Level 124 at 60,000: leads of five to eight times the rival's army never
     *  attacked. */
    bool overwhelming = false;
    bool Ready() const { return force_ok && ((level_ok && runway_ok) || overwhelming); }
};

class DaiRivals {
public:
    /** The best summon (a pack's own power of the summon role) the keeper can cast now from `spare` gold: the strongest
     *  creature for the price; nullptr if none (`why` says why, when given). */
    static const DaiContentEntry *BestSummon(const DaiKnowledge &k, int64_t spare, int64_t *price, const char **why = nullptr);
    /** One turn: the hate from this turn's facts; the look every [offence] interval; a route search stepped. */
    void Update(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, const DaiPersonality &per,
                const KeeperFact *facts, int64_t nfacts, DaiTurn now);
    const DaiRivalEstimate &Rival(PlayerNumber r) const;
    /** Rival r's hate decayed to `now`. */
    int64_t Hate(PlayerNumber r, DaiTurn now, const DaimonGlobalConfig &cfg) const;
    /** Every target, the best score first. */
    std::span<const DaiRivalTarget> Targets() const { return std::span<const DaiRivalTarget>(m_targets); }
    /** The best scoring target with a way in and a score at the personality's attack_threshold (nullptr: none). */
    const DaiRivalTarget *Best() const;
    DaiTurn LastLook() const { return m_last_look; }
    bool Searching() const { return m_search_on; }
    /** The keeper's fighters' force in levels (imps apart), and of those healthy and in no fight. */
    static int64_t OwnLevels(const DaiKnowledge &k);
    static int64_t FreeLevels(const DaiKnowledge &k);
    static DaiReadiness Readiness(const DaiKnowledge &k, const DaiPersonality &per, const DaimonGlobalConfig &cfg,
                                  const DaiForecastOut &forecast, const DaiRivalTarget &t);
    /** The attack urgency (DaiFix 0..1, plan 13): 0 until the income's horizon is within [offence] urgency_turns
     *  turns, growing to 1 at none left -- the side's gold gone. 0 in the game's first urgency_grace_paydays pay days,
     *  with a gem held, the gold not known, no income yet or urgency_turns 0. */
    static DaiFix Urgency(const DaiKnowledge &k, const DaiPersonality &per, const DaiForecastOut &forecast);
    /** The force estimate against rival r (any rival: r < 0), plan 13 L1: its target with a way in and the least
     *  defence (a room or portal, else its heart) -- what stands there and response_share % of its force -- and whether the free fighters reach
     *  superiority_to_attack % of that (Readiness.force_ok). False, with *known false, when no such target is seen. */
    bool Favourable(const DaiKnowledge &k, const DaiPersonality &per, const DaimonGlobalConfig &cfg,
                    const DaiForecastOut &forecast, PlayerNumber r, bool *known) const;
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
    /** The target kind wanted of each rival this turn (-1: the personality's first_target), from the keeper's Attack
     *  goals (a lieutenant's attack objective, 14 R4). Set before Update; not saved (derived each turn). */
    void SetAims(const std::array<int64_t, DAI_MAX_PLAYERS> &aims) { m_aims = aims; }
private:
    void add_hate(PlayerNumber r, int64_t v, DaiTurn now, const DaimonGlobalConfig &cfg);
    void look(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, const DaiPersonality &per, DaiTurn now);
    void score(const DaiKnowledge &k, const DaimonGlobalConfig &cfg, const DaiPersonality &per, DaiTurn now);
    void routes(const DaiKnowledge &k, PlayerNumber p, const DaimonGlobalConfig &cfg, const DaiPersonality &per, DaiTurn now);

    std::array<DaiRivalEstimate, DAI_MAX_PLAYERS> m_rivals{};
    std::vector<DaiRivalTarget> m_targets;
    DaiFix m_threshold = DAI_FIX_ONE;   /**< attack_threshold when last scored */
    std::array<int64_t, DAI_MAX_PLAYERS> m_aims = make_no_aims();
    static std::array<int64_t, DAI_MAX_PLAYERS> make_no_aims() { std::array<int64_t, DAI_MAX_PLAYERS> a{}; a.fill(-1); return a; }
    DaiTurn m_last_look = 0, m_next_look = 0;
    // The route search in progress: for the target at m_searching (its rival, kind and room).
    bool m_search_on = false;
    DaiRivalTarget m_searching;
    DaiDigSearch m_search;
};

#endif
