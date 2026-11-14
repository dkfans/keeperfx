/** @file DaiOffence.h
 *  Offence (07 section 4): attack operations, one at a time, while the director wants one -- an Attack goal stands (key 0:
 *  any rival; 1 + r: rival r; the director posts it from P5.8, so until then no operation starts in play). Offence never
 *  fights: it asks Tactics through objectives and the other managers through service requests (02 C1). Every
 *  [offence] op_interval turns:
 *   - recon: the target is the best of DaiRivals' for the goal's rival; until it has a way in (and, for a heart, the heart
 *     is known) a Scout objective at it (Tactics' group walks there) and a Sight of Evil peek (DaiMagic); recon_timeout;
 *   - stage: until DaiRivals::Readiness says ready; stage_timeout;
 *   - breach: the Attack search's path from the keeper's own land to the target, planned again each look; the first
 *     earth run on it dug (a Dig request to Excavation for its far end), the first revealed wall of another keeper's
 *     opened (a DestroyWalls request to DaiMagic, level 1: it digs the slab out); done when the way is open and the slab
 *     before the rival's ground (the staging point, where drops can land) is the keeper's own; breach_timeout, or
 *     too many refused requests;
 *   - assault: an Assault objective at the target (staging, cta_attack_budget a 1000 turns, home_guard_share) whose
 *     report it reads: a heart target won when the rival is no keeper any more, a room or portal when its slab is not
 *     the rival's; withdrawn on a Lost report, on the enemy force above withdraw_ratio % of its own for withdraw_patience
 *     turns, or after assault_timeout;
 *   - exploit (07 section 8): after a heart falls the loser's rooms stay its own and claimable; one at a time, portals
 *     first, then a treasury, a library, the rest, a slab of one that touches the keeper's land is crept onto (a Creep
 *     request, creep_imps); the operation is won when none of the loser's rooms is left seen, or after exploit_turns
 *     (its claimed path turns neutral and the keeper's imps claim it, bringing more rooms within reach meanwhile);
 *   - withdraw: a Withdraw objective at the target for withdraw_turns, then the target cools down for retry_after turns.
 *  A fight at home (a Defence objective engaged) aborts recon, stage and breach, turns an assault into a withdrawal, and
 *  keeps a new operation from starting -- for withdraw_turns after one ended for it (such a fight comes and goes).
 *  Ending an operation withdraws its requests and drops its objectives: the serving managers cancel jobs whose request
 *  is gone, so their gold claims are released (the refund). The Attack goal's status follows the phase.
 *  An Attack goal whose count is DAI_ATTACK_RAIDS (the director's raid posture) starts no operation.
 *  Raids (07 section 4.4): while the goal stands, one at a time beside an operation (not while it assaults or
 *  withdraws, nor while the keeper fights at home), [offence] raid_interval / the personality's raid_appetite turns
 *  after the last; a Raid objective (key 2) of the first kind that has a target:
 *   - PortalSteal: a slab of the rival's portal beside the keeper's own land (the foothold, its staging point): a Creep
 *     request to Excavation (creep_imps imps) while Tactics' group escorts; won when the slab is the keeper's;
 *   - ImpHunt: an imp of the rival's seen within raid_border slabs of the keeper's land: the group attacks it; over
 *     when it is no longer seen;
 *   - Harass: a room of the rival's within raid_border slabs of the keeper's land (staging: its own floor nearest it):
 *     the banner on it (Tactics' gather and advance, half the attack budget) for harass_turns, then a Withdraw
 *     (key 2) for withdraw_turns;
 *  given up after raid_timeout, or when the request is refused.
 *  Diplomacy (07 section 5a), every [diplomacy] interval turns, for each other keeper in the game, from the knowledge
 *  only: its value as an ally -- a common threat (a third keeper, not already the keeper's ally, stronger than both:
 *  1.0 and up to 1.0 more by how much), weakness relief (up to 0.5 when the other is the stronger), less its threat
 *  (the hate it has earned, DaiRivals, 1.0 for every HATE_ALLY) and the lost prize (0.5 when it is the best target);
 *  the last war with it (it took the keeper's ground or came onto it, or an operation or raid went against it). One
 *  set_alliance gesture a look, with a message to every player, never on an ALLIANCE_LOCKED level:
 *   - propose: the value at ally_min_value or more for ally_dwell turns, no war for peace_after_war turns;
 *   - accept: the other has declared; the value at ally_min_value x (1 - loyalty / 2), no war for peace_after_war;
 *   - break: it withdrew, or a war since the alliance, or the value under ally_min_value x (1 - loyalty), or (mutual
 *     allies, the other at under half the keeper's force) treachery's chance from the keeper's random stream.
 *  A keeper the keeper has declared alliance with is no target of its operations or raids. */
#ifndef DAIMON_DAIOFFENCE_H
#define DAIMON_DAIOFFENCE_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>
#include <vector>

#include "daimon/DaiDigSearch.h"
#include "daimon/DaiRivals.h"
#include "daimon/IDaiManager.h"

enum class DaiOpPhase : uint8_t { None, Recon, Stage, Breach, Assault, Exploit, Withdraw, Count };
const char *DaiOpPhaseName(DaiOpPhase p);

class DaiOffence : public IDaiManager {
public:
    const char *GetName() const override { return "offence"; }
    DaiDomain GetDomain() const override { return DaiDomain::Offence; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;

    DaiOpPhase Phase() const { return m_phase; }
    DaiTurn PhaseSince() const { return m_phase_since; }
    const DaiRivalTarget &Target() const { return m_target; }
    DaiSlabXY Staging() const { return m_staging; }
    uint32_t AssaultObjective() const { return m_assault; }
    /** Why the last operation ended ("" while one runs), and how many were won and given up. */
    const char *LastEnd() const { return m_last_end; }
    /** The raid under way (its kind, None: none), its objective, when the next may start, how many have ended and why
     *  the last did. */
    DaiObjectiveSub RaidSub() const { return m_raid_sub; }
    uint32_t RaidObjective() const { return m_raid; }
    DaiTurn NextRaid() const { return m_raid_next; }
    int64_t Raids() const { return m_raids; }
    const char *RaidEnd() const { return m_raid_end; }
    /** Diplomacy: the other keeper's value as an ally at the last look, and when the keeper last fought it (0: never). */
    DaiFix AllyValue(PlayerNumber r) const { return ((r >= 0) && (r < DAI_MAX_PLAYERS)) ? m_ties[(size_t)r].value : 0; }
    DaiTurn LastWar(PlayerNumber r) const { return ((r >= 0) && (r < DAI_MAX_PLAYERS)) ? m_ties[(size_t)r].war : 0; }
    /** Hate (DaiRivals') that makes a keeper's threat as an ally 1.0. */
    static constexpr int64_t HATE_ALLY = 50;
    int64_t Won() const { return m_won; }
    int64_t GivenUp() const { return m_given_up; }
    /** The service requests the breach has standing. */
    const std::vector<uint32_t> &Requests() const { return m_requests; }
    /** Turns a target (by rival, kind and room) is left alone after an operation against it was given up. */
    DaiTurn CoolUntil(const DaiRivalTarget &t) const;
private:
    struct Cooled { PlayerNumber rival = -1; DaiTargetKind kind = DaiTargetKind::Heart; int64_t room_index = 0; DaiTurn until = 0; };
    void start(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiGoal &g);
    void recon(const DaiTurnCtx &c, DaiManagerWrites &w);
    void stage(const DaiTurnCtx &c, DaiManagerWrites &w);
    void breach(const DaiTurnCtx &c, DaiManagerWrites &w);
    void assault(const DaiTurnCtx &c, DaiManagerWrites &w);
    void withdraw(const DaiTurnCtx &c, DaiManagerWrites &w);
    void exploit(const DaiTurnCtx &c, DaiManagerWrites &w);
    void go(DaiOpPhase p, const DaiTurnCtx &c, const char *why);
    /** Ends the operation: requests withdrawn, objectives dropped; `cool`: the target left alone for retry_after. */
    void end(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why, bool won, bool cool);
    void begin_withdraw(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why);
    uint32_t ask(const DaiTurnCtx &c, DaiManagerWrites &w, DaiDomain to, DaiService s, DaiSlabXY at, int64_t amount);
    bool home_fight(const DaiTurnCtx &c) const;
    /** The target as DaiRivals has it now (its route and defence refreshed); false if it is no longer there. */
    bool refresh_target(const DaiTurnCtx &c);
    DaiRect target_area() const;
    void raids(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiGoal *g);
    bool start_raid(const DaiTurnCtx &c, DaiManagerWrites &w, PlayerNumber rival);
    bool try_raid(const DaiTurnCtx &c, DaiManagerWrites &w, PlayerNumber rival, int64_t border, bool steal);
    void follow_raid(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiGoal *g);
    void end_raid(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why);
    void diplomacy(const DaiTurnCtx &c, DaiManagerWrites &w);
    DaiFix ally_value(const DaiTurnCtx &c, PlayerNumber r) const;
    void declare(const DaiTurnCtx &c, DaiManagerWrites &w, PlayerNumber r, bool on, const char *why);

    DaiOpPhase m_phase = DaiOpPhase::None;
    DaiTurn m_phase_since = 0, m_next = 0;
    uint32_t m_goal = 0;                    /**< the Attack goal it serves */
    DaiRivalTarget m_target;
    DaiSlabXY m_staging{ -1, -1 };
    uint32_t m_scout = 0, m_assault = 0, m_withdraw = 0;    /**< its objectives */
    std::vector<uint32_t> m_requests;       /**< its standing service requests */
    int64_t m_refusals = 0;                 /**< breach requests refused this operation */
    int64_t m_breach_left = -1;             /**< breach: the fewest slabs left to open on the way so far (-1: not looked) */
    DaiTurn m_breach_moved = 0;            /**< breach: when that last fell (the timeout runs from it) */
    DaiTurn m_bad_since = 0;               /**< assault: the force ratio has been past withdraw_ratio since (0: not) */
    bool m_search_on = false;
    DaiDigSearch m_search;                  /**< breach: the Attack path */
    std::vector<Cooled> m_cooled;
    int64_t m_won = 0, m_given_up = 0;
    DaiTurn m_quiet_until = 0;             /**< none begun before this (an operation ended for a fight at home) */
    DaiSlabXY m_exploit_at{ -1, -1 };       /**< exploit: the loser's room slab being crept onto */
    // Diplomacy.
    struct Tie {
        DaiFix value = 0;
        DaiTurn above_since = 0;           /**< the value at ally_min_value or more since (0: not) */
        DaiTurn war = 0;                   /**< the last war with it */
        DaiTurn allied = 0;                /**< when the keeper declared alliance with it */
        int64_t hate = 0;                   /**< its hate at the last look */
        bool mutual = false;                /**< mutual allies at the last look */
    };
    std::array<Tie, DAI_MAX_PLAYERS> m_ties{};
    DaiTurn m_dip_next = 0;
    char m_last_end[32] = "";
    // The raid.
    DaiObjectiveSub m_raid_sub = DaiObjectiveSub::None;
    uint32_t m_raid = 0, m_raid_withdraw = 0, m_raid_request = 0;
    DaiTurn m_raid_since = 0, m_raid_next = 0;
    DaiSlabXY m_raid_at{ -1, -1 };
    PlayerNumber m_raid_rival = -1;
    DaiThingRef m_raid_imp;
    int64_t m_raids = 0;
    char m_raid_end[32] = "";
};

#endif
