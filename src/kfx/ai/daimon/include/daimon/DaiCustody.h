/** @file DaiCustody.h
 *  DaiCustody (08 sections 4-6, 17: the custody core, P6.2-P6.4): the keeper's prisoners, each given the plan worth most
 *  -- convert or interrogate on a rack (one torture: a break is either), a skeleton starved in prison, a ghost killed on
 *  a rack, held from its keeper, or dismissed -- valued on creature_force_score's scale and scaled by the personality's
 *  weights; re-planned every [custody] plan_interval turns and on a custody fact. Its actions, one hand trip at a time:
 *  a victim carried to a free rack (the best first), a prisoner off a rack back to prison when its plan no longer
 *  wants the rack, a held prisoner fed when hungry, one worth nothing dismissed. A victim to be converted or
 *  interrogated is healed below heal_victim_below % of its health, and sped up once past its kind's break time. The
 *  imprison tendency is on while a prison has prison_spare places free and the personality takes prisoners; a prison
 *  is asked for once it has fought, a torture chamber once a prisoner's best plan wants a rack. Gold and force compare
 *  through the keeper's own creatures (their force per gold of pay). With a temple pool (P6.5, 08 section 7.2), the
 *  sacrifice planner picks the next victim -- a prisoner (giving up its plan), one of its creatures (its force) or an
 *  imp, by the personality's `sacrifices` -- and a hand trip drops it on the pool; a temple is asked for when a
 *  sacrifice would be worth it or creatures want to pray. A graveyard (P6.6) once [custody] graveyard_bodies bodies have
 *  lain on its ground at once, with the personality's vampires: its imps carry bodies there and every bodies_for_vampire
 *  of them rotted raise a vampire. A jailbreak (P6.9, section 10): for a creature of its own held by another keeper,
 *  worth [custody] jailbreak_min_force by its force x rescue_value, a floor slab beside that prison is chosen and the free
 *  slab beside that nearest its land dug to (a claim route, Excavation's Dig service -- a claim route cannot end on
 *  another keeper's floor) and, once its land touches the floor slab, crept onto (Creep) -- a prison that
 *  touches the prisoner's keeper's land lets it escape (the engine's roll); a Raid objective (Jailbreak) has Tactics
 *  escort. Over when the creature is out, gone, or after jailbreak_timeout (then not again for jailbreak_retry). Discipline
 *  of its own creatures is DaiCreatures' (section 8). */
#ifndef DAIMON_DAICUSTODY_H
#define DAIMON_DAICUSTODY_H

#include "daimon_base_types.h"
#include <vector>

#include "daimon/DaiSacrificePlanner.h"
#include "daimon/IDaiManager.h"

enum class DaiCustodyPlan : uint8_t { None, Convert, Interrogate, Skeleton, Ghost, Hold, Dismiss, Count };
constexpr const char *DaiCustodyPlanName(DaiCustodyPlan p)
{
    switch (p) {
    case DaiCustodyPlan::Convert: return "convert";
    case DaiCustodyPlan::Interrogate: return "interrogate";
    case DaiCustodyPlan::Skeleton: return "skeleton";
    case DaiCustodyPlan::Ghost: return "ghost";
    case DaiCustodyPlan::Hold: return "hold";
    case DaiCustodyPlan::Dismiss: return "dismiss";
    default: return "none";
    }
}

/** A prisoner as Custody keeps it: its plan, the plan's value and since when, and its last heal and speed asked. */
struct DaiPrisonerRecord {
    DaiThingRef ref;
    DaiCustodyPlan plan = DaiCustodyPlan::None;
    int64_t value = 0;
    DaiTurn since = 0;
    DaiTurn healed = 0, sped = 0;
};

/** One prisoner's plan values (force score, weights applied); Custody picks the highest. */
struct DaiCustodyValues {
    int64_t v[(size_t)DaiCustodyPlan::Count] = {};
    DaiCustodyPlan Best() const;
};

class DaiCustody : public IDaiManager {
public:
    const char *GetName() const override { return "custody"; }
    DaiDomain GetDomain() const override { return DaiDomain::Custody; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;

    /** A prisoner's plan values (pure: tests and the tournament's notes). */
    static DaiCustodyValues Values(const DaiTurnCtx &c, const DaiPrisonerLook &p, bool prison_room_free);
    /** The keeper's own creatures' force for a gold piece of pay (DaiFix); 0 with none. */
    static int64_t ForcePerGold(const DaiKnowledge &k);
    /** The kind it pays most for, all its creatures of it together (0: none). */
    static DaiThingModel MostPaidKind(const DaiKnowledge &k);
    std::span<const DaiPrisonerRecord> Prisoners() const { return std::span<const DaiPrisonerRecord>(m_prisoners); }
    const DaiPrisonerRecord *Record(const DaiThingRef &r) const;
    /** Conversions and interrogations seen (custody facts), prisoners sent to the portal: the tournament's counts. */
    int64_t Converted() const { return m_converted; }
    int64_t Interrogated() const { return m_interrogated; }
    int64_t Dismissed() const { return m_dismissed; }
    int64_t Sacrificed() const { return m_sacrificed; }     /**< recipes fired (KF_Sacrifice) */
    int64_t Vampires() const { return m_vampires; }         /**< vampires risen from its graveyard (KF_VampireProgress) */
    int64_t Freed() const { return m_freed; }               /**< its creatures out of another's prison during a jailbreak */
    int64_t Punished() const { return m_punished; }         /**< recipes fired that punished it (never planned: 0 expected) */
    DaiSlabXY JailbreakAt() const { return m_jb_at; }       /**< the slab its jailbreak claims (-1: none) */
    static constexpr uint64_t KEY_JAIL = 3;
    /** The sacrifice the planner wants now, pool or no pool (pure: tests; with none, the temple ask). imps_per_kturn:
     *  how fast the keeper has been making imps lately (DaiFix), what cheaper imps would save on. */
    /** Prisoners worth more than bodies to its graveyard, the enemies it fights now (P9); true with no graveyard. */
    static bool PrisonersOverBodies(const DaiTurnCtx &c);
    static DaiSacrificeChoice SacrificePlan(const DaiTurnCtx &c, const std::vector<DaiPrisonerRecord> &prisoners,
                                            DaiFix imps_per_kturn = 0);
    /** The rate imps were made lately (DaiFix a 1000 turns): a moving average of the new imps seen. */
    DaiFix ImpsPerKturn() const { return m_imp_rate; }
    /** One sample of it: `made` imps in `dt` turns moves the average dt / (dt + IMP_TAU) of the way (pure: tests). */
    static DaiFix NextImpRate(DaiFix rate, bool first, int64_t made, int64_t dt);
    static constexpr int64_t IMP_SAMPLE = 1000, IMP_TAU = 10000;
    /** Job keys: one hand trip at a time; the tendency (Tactics' flee is key 0); heals and speeds by the prisoner. */
    static constexpr uint64_t KEY_HAND = 1, KEY_TENDENCY = 1, KEY_CAST = 1ull << 32;
private:
    void refresh(const DaiTurnCtx &c, DaiManagerWrites &w);
    void count_imps(const DaiTurnCtx &c);
    void plan_all(const DaiTurnCtx &c);
    void tendency(const DaiTurnCtx &c, DaiManagerWrites &w);
    void rooms(const DaiTurnCtx &c, DaiManagerWrites &w);
    void hand(const DaiTurnCtx &c, DaiManagerWrites &w);
    void casts(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool sacrifice(const DaiTurnCtx &c, DaiCustodyHandParams *hp);
    void jailbreak(const DaiTurnCtx &c, DaiManagerWrites &w);
    void end_jailbreak(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why);
    std::vector<DaiPrisonerRecord> m_prisoners;
    DaiTurn m_next_plan = 0;
    bool m_replan = false;
    bool m_fought = false;      /**< a fight of its own seen: a prison is worth having */
    int64_t m_converted = 0, m_interrogated = 0, m_dismissed = 0;   /**< custody facts and dismissals (the tournament) */
    int64_t m_sacrificed = 0;
    bool m_sacrifice_wanted = false;    /**< the last plan found a sacrifice worth making (a temple is asked for) */
    DaiTurn m_sacrifice_retry = 0;     /**< after a trip to the pool failed (the victim would not be picked up): none before */
    bool m_bodies = false;      /**< graveyard_bodies lay on its ground at once: a graveyard is worth having (P6.6) */
    int64_t m_vampires = 0;
    // The jailbreak under way (P6.9): whom, the slab beside the prison to claim, whose prison, since when; its request
    // (a dig, then a creep) and objective; not again before cool; how many creatures came out while it ran.
    DaiThingRef m_jb_who;
    DaiSlabXY m_jb_at{ -1, -1 };
    PlayerNumber m_jb_rival = -1;
    DaiTurn m_jb_since = 0, m_jb_cool = 0;
    uint32_t m_jb_request = 0, m_jb_objective = 0;
    bool m_jb_creep = false;
    int64_t m_freed = 0, m_punished = 0;
    // Imps made: the own imps last seen and the imps sacrificed then (not saved: after a load the next look is the
    // baseline), the new ones since the sample began less those sacrificed, and the moving average of the samples.
    std::vector<DaiThingRef> m_imps_seen;
    bool m_imps_baseline = false;
    int64_t m_imps_sacrificed_seen = 0;
    int64_t m_imps_new = 0;
    DaiTurn m_imp_sample = 0;
    DaiFix m_imp_rate = 0;
    int64_t m_imp_samples = 0;
};

#endif
