/** @file DaiTactics.h
 *  The one decider in the keeper's fights (06 section 5): Defence (and later Offence) post objectives; Tactics keeps the
 *  engagements -- every battle its creatures are in, and contacts (enemies not yet fighting on its ground, near an
 *  objective's area, or on a predicted route) -- matches each to the objectives it touches (an unmatched battle on its
 *  ground serves an implicit HoldArea, or ProtectImps where imps are near), ranks them by stakes (the objective's
 *  weight times what is at risk) and shares out what a keeper has one of: the Call to Arms banner, the group, the free
 *  fighters and the gold. It writes each objective's report. Then its action menu (06 section 5.5), in order:
 *   1 evacuate imps, 3 reinforce, 6 lift the wounded -- keeper_reflexes' decision (kfx_ai, D8) on candidates from the
 *     knowledge, its drop spots Daimon's own (DaiFindDropSpot); one hand trip at a time;
 *   2 the heart: Hold Audience below hold_audience_below (Q3), else the banner at the heart;
 *   4 the Call to Arms banner at the engagement it was given (own ground, or an objective paying for it), down when
 *     none has it;
 *   5 casts: keeper_assist_decide on Tactics' own KeeperAssistState and the fight rules (DaiFightRules,
 *     DaiAssistWorld), noted on the state when the cast is made; the state is saved;
 *   7 recall: a fight past hopeless_ratio that no help can turn -- its fighters lifted home, its objective lost;
 *   8 the group (06 section 5a): formed of the free fighters nearest the use the allocation gave it, up to group_size
 *     (one without a barracks), sent to attack an enemy it sees there or to move and hold (a Scout: to the slab its
 *     creatures can reach nearest the target, 07 section 7); released when unused.
 *  The offensive half (07 section 4.3, P5.6), for the Assault or harassing Raid with a staging point of the highest
 *  stakes (a raid that steals a portal or hunts an imp is a use of the group alone, 4.4: it escorts, or attacks the imp):
 *   gather: the banner at the staging point (own ground, free) until [offence] gather_share % of the force it may send
 *     (its free fighters' but the home guard's) stands there, or gather_turns have passed; then
 *   advance: the banner onto the target (its upkeep the objective's budget: cta_use 2), the rival's heart once it is
 *     seen and in reach (heart focus);
 *   waves: the hand drops its free fighters still at home at the staging point, the strongest first, a hand-load a trip
 *     and at most waves_per_kturn trips a 1000 turns, never into the home guard;
 *   strike: once advanced the group attacks the heart (focus), else the rival's door nearest the staging point between
 *     it and the target, else the target room; ordered again when that point changes;
 *   casts: the offensive rule bank while an assault's fight outweighs every fight at home;
 *   withdraw: for a Withdraw objective, its creatures away from home near it lifted to a lair, one hand-load a trip; the
 *     banner and the group are no longer the operation's, so they come down and go; reported Done when none is left.
 *  The personality's cta_use limits the banner (never; own ground; anywhere an objective pays), and its flee_tendency is
 *  kept set on the creatures' panel (a set_tendency job, again a while after one fails). */
#ifndef DAIMON_DAITACTICS_H
#define DAIMON_DAITACTICS_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "keeper_assist.h"
#include "keeper_assist_state.h"
#include "daimon/DaiObjectives.h"
#include "daimon/IDaiManager.h"

struct DaiEngagement {
    uint64_t key = 0;                   /**< battle: 1 << 32 | the game's battle; contact: 2 << 32 | its slab */
    bool battle = false;
    DaiSlabCoord x = 0, y = 0;          /**< where (a battle: its own fighters' mean place; a contact: the enemy) */
    int64_t own_force = 0, enemy_force = 0, own_count = 0, enemy_count = 0;
    int64_t enemy_levels = 0;           /**< its enemy creatures' levels, imps apart ([defence] threat_force's measure) */
    bool own_ground = false;
    int64_t imps_near = 0;              /**< the keeper's imps within 3 slabs */
    uint32_t objective = 0;             /**< the objective it serves (0: an implicit one) */
    DaiObjectiveKind kind = DaiObjectiveKind::HoldArea;
    DaiFix stakes = 0;
    // What it was given this tick.
    int64_t deficit = 0;                /**< superiority x enemy - own force: > 0, it needs help */
    int64_t fighters = 0;               /**< free fighters' force sent to it, at most its deficit */
    int64_t gold = 0;                   /**< gold its casts and upkeep may spend */
    bool banner = false;                /**< the Call to Arms banner is its */
    bool group = false;                 /**< the group is its */
};

class DaiTactics : public IDaiManager {
public:
    const char *GetName() const override { return "tactics"; }
    DaiDomain GetDomain() const override { return DaiDomain::Tactics; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;

    std::span<const DaiEngagement> Engagements() const { return std::span<const DaiEngagement>(m_engagements); }
    /** The group's use: an engagement's key, or (for a non-combat objective: Intercept, Escort, Scout) 0 and the
     *  objective's id; both 0 when nothing has it. */
    uint64_t GroupEngagement() const { return m_group_key; }
    uint32_t GroupObjective() const { return m_group_objective; }
    /** What the group was last formed for (0: nothing), and when. */
    uint64_t GroupUse() const { return m_group_use; }
    DaiTurn GroupTurn() const { return m_group_turn; }
    uint64_t BannerEngagement() const { return m_banner_key; }
    /** The free fighters' force this tick, and what is kept home while an Assault or Raid runs. */
    int64_t FreeForce() const { return m_free_force; }
    int64_t HomeReserve() const { return m_home_reserve; }
    DaiTurn NextTick() const { return m_next; }
    /** Its casting state (06 section 7): the live rules, cooldowns and spend. */
    const KeeperAssistState &Assist() const { return m_assist; }
    /** Where it last put the banner (-1: not up by Tactics). */
    DaiSlabXY BannerAt() const { return m_banner_at; }
    /** The utility of each menu step's jobs: the arbiter runs the higher first within Tactics. */
    static DaiFix StepUtility(int step);
    /** The offensive objective it serves (0: none), whether its banner has left the staging point, since when. */
    uint32_t OffenceObjective() const { return m_off_id; }
    bool Advanced() const { return m_off_advanced; }
    DaiTurn OffenceSince() const { return m_off_since; }
    /** Where its banner and strike go now: the staging point while gathering, then the target or the heart (-1: none). */
    DaiSlabXY OffencePoint() const { return m_off_point; }
    bool HeartFocus() const { return m_off_heart.x >= 0; }
    /** The turns of the waves dropped in the last 1000 turns. */
    const std::vector<DaiTurn> &Waves() const { return m_waves; }
private:
    DaiTurn m_summon_logged = 0;     /**< the last "no summon" line (a log, not saved) */
    struct Tally { uint32_t id = 0; int64_t own_count = 0, enemy_count = 0; DaiFix ratio = 0; bool engaged = false; };
    void find_engagements(const DaiTurnCtx &c);
    void match(const DaiTurnCtx &c);
    void allocate(const DaiTurnCtx &c);
    void report(const DaiTurnCtx &c, DaiManagerWrites &w);
    void act(const DaiTurnCtx &c, DaiManagerWrites &w);
    void tendency(const DaiTurnCtx &c, DaiManagerWrites &w);
    void log_fights(const DaiTurnCtx &c);
    void group(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool reflexes(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool recall(const DaiTurnCtx &c, DaiManagerWrites &w);
    /** A map pack's own summon (Phase 7): cast on the keeper's own ground at a home fight short of force, with the
     *  gold spare for it -- the creature it makes joins the fight. */
    bool summon(const DaiTurnCtx &c, DaiManagerWrites &w);
    /** An assault's summon at its staging point: true once cast or none is to be had (not tried again). */
    bool summon_at_staging(const DaiTurnCtx &c, DaiManagerWrites &w, DaiSlabXY stg);
    void casts(const DaiTurnCtx &c, DaiManagerWrites &w);
    void offence(const DaiTurnCtx &c);
    bool waves(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool withdraw(const DaiTurnCtx &c, DaiManagerWrites &w);
    /** The group's order for the offensive objective: what to attack (or where to go) and the slab. */
    void strike(const DaiTurnCtx &c, const DaiObjective &o, DaiGroupParams &p, DaiSlabXY &at) const;
    /** The keeper's creatures off its own ground near the objective (a withdrawal's), nearest first. */
    std::vector<const DaiCreatureLook *> away(const DaiTurnCtx &c, const DaiObjective &o) const;

    DaiTurn m_next = 0;
    DaiTurn m_fight_logged = 0;        /**< log_fights' last line (logs only, not saved) */
    std::vector<DaiEngagement> m_engagements;
    uint64_t m_banner_key = 0;
    uint64_t m_group_key = 0;
    uint32_t m_group_objective = 0;
    int64_t m_free_force = 0, m_home_reserve = 0;
    std::vector<Tally> m_tally;         /**< per objective, what the last report saw */
    KeeperAssistState m_assist{};
    /** The assist candidates of a turn (scratch, not saved: a member, not a static, so keepers never share it). */
    std::array<KeeperAssistCandidate, KEEPER_ASSIST_MAX_CANDIDATES> m_cands{};
    std::array<int64_t, KEEPER_ASSIST_RULES> m_reflex_fire{};   /**< each hand rule's last firing turn */
    DaiSlabXY m_banner_at{ -1, -1 };
    DaiTurn m_audience_turn = 0;       /**< its last Hold Audience */
    DaiTurn m_tendency_after = 0;      /**< no set_tendency before this (one failed) */
    uint64_t m_group_use = 0;           /**< what the group was last formed for (an engagement key, or GROUP_OBJECTIVE | id) */
    DaiTurn m_group_turn = 0;          /**< when */
    std::vector<DaiThingRef> m_group_members;   /**< whom it was formed of (an order again keeps the group) */
    DaiSlabXY m_group_at{ -1, -1 };     /**< the offensive strike point it was last sent to */
    uint32_t m_off_id = 0;              /**< the offensive objective served */
    bool m_off_summoned = false;        /**< its summon cast at the staging point (or none to cast) */
    DaiTurn m_off_since = 0;           /**< gathering since */
    bool m_off_advanced = false;
    std::vector<DaiTurn> m_waves;      /**< the waves' turns, the last 1000 turns' */
    // Worked out again at every look, before they are used (not saved).
    DaiSlabXY m_off_point{ -1, -1 };
    DaiSlabXY m_off_heart{ -1, -1 };
};

#endif
